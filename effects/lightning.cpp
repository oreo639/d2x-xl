#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <string.h>	// for memset
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#ifndef _WIN32
#	include <unistd.h>
#endif

#include "descent.h"
#include "PerlinNoise.h"
#include "SimplexNoise.h"
#include "segmath.h"

#include "objsmoke.h"
#include "transprender.h"
#include "renderthreads.h"
#include "rendermine.h"
#include "error.h"
#include "glow.h"
#include "light.h"
#include "dynlight.h"
#include "ogl_lib.h"
#include "automap.h"
#include "addon_bitmaps.h"

#define RENDER_LIGHTNING_OUTLINE 0

#if NOISE_TYPE
extern CSimplexNoise noiseX [MAX_THREADS], noiseY [MAX_THREADS];
#else
extern CPerlinNoise noiseX [MAX_THREADS], noiseY [MAX_THREADS];
#endif

#define LIMIT_FLASH_FPS			1
#define FLASH_SLOWMO				1
#define DEFAULT_PLASMA_WIDTH	3.0f
#define DEFAULT_CORE_WIDTH		3.0f

#define STYLE	(((m_nStyle < 0) || (gameOpts->render.lightning.nStyle < m_nStyle)) ? \
					 gameOpts->render.lightning.nStyle : m_nStyle)

//------------------------------------------------------------------------------

void RenderTestImage (void)
{
if (gameStates.render.cameras.bActive) {
	//glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
	//glClear (GL_COLOR_BUFFER_BIT);
	float vertices [4][4][2] = {
		{{0.0, 0.0}, {0.0, 0.5}, {0.5, 0.5}, {0.5, 0.0}},
		{{0.5, 0.0}, {0.5, 0.5}, {1.0, 0.5}, {1.0, 0.0}},
		{{0.0, 0.5}, {0.0, 1.0}, {0.5, 1.0}, {0.5, 0.5}},
		{{0.5, 0.5}, {0.5, 1.0}, {1.0, 1.0}, {1.0, 0.5}},
		};
	float colors [4][4] = {
		{1.0, 0.5, 0.0, 0.5},
		{0.0, 0.5, 1.0, 0.5},
		{1.0, 0.0, 0.5, 0.5},
		{0.0, 1.0, 0.5, 0.5}
	};


	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();//clear matrix
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();//clear matrix
	glOrtho (0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

	GLenum nBlendModes [2], nDepthMode = ogl.GetDepthMode ();
	ogl.GetBlendMode (nBlendModes [0], nBlendModes [1]);
	ogl.SetBlendMode (OGL_BLEND_REPLACE);
	ogl.SetDepthMode (GL_ALWAYS);

	ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
	ogl.SetTexturing (false);
	for (int32_t i = 0; i < 4; i++) {
		ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
		ogl.SetTexturing (false);
		glColor3fv (colors [i]);
		OglVertexPointer (2, GL_FLOAT, 0, vertices [i]);
		OglDrawArrays (GL_QUADS, 0, 4);
		}
	ogl.SetBlendMode (nBlendModes [0], nBlendModes [1]);
	ogl.SetDepthMode (nDepthMode);
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();
	}
}

//------------------------------------------------------------------------------

CFixVector *VmRandomVector (CFixVector *vRand)
{
	CFixVector	vr;

do {
	vr.v.coord.x = (90 - Rand (181)) * (I2X (1) / 90);
	vr.v.coord.y = (90 - Rand (181)) * (I2X (1) / 90);
	vr.v.coord.z = (90 - Rand (181)) * (I2X (1) / 90);
} while (vr.IsZero ());
CFixVector::Normalize (vr);
*vRand = vr;
return vRand;
}

//------------------------------------------------------------------------------

#define SIGN(_i)	(((_i) < 0) ? -1 : 1)

#define VECSIGN(_v)	(SIGN ((_v).v.c.x) * SIGN ((_v).v.c.y) * SIGN ((_v).v.c.z))

//------------------------------------------------------------------------------

CFixVector *DirectedRandomVector (CFixVector *vRand, CFixVector *vDir, int32_t nMinDot, int32_t nMaxDot)
{
	CFixVector	vr, vd = *vDir;

CFixVector::Normalize (vd);
#if 1
do {
	VmRandomVector (&vr);
	} while (CFixVector::Dot (vr, vd) > I2X (9) / 10);

vr = CFixVector::Normal (CFixVector::ZERO, vd, vr);
fix dot = nMinDot + Rand ((nMaxDot - nMinDot + 1) / 2) * 2;
double a = acos (X2D (dot));
if (fabs (a - PI * 0.5) > 1e-6) {
	vr *= F2X (tan (a));
	vr += vd;
	CFixVector::Normalize (vr);
	}
#if DBG
dot = CFixVector::Dot (vd, vr);
#endif
#else
fix nDot;
do {
	VmRandomVector (&vr);
	nDot = CFixVector::Dot (vr, vd);
	} while (((nDot > nMaxDot) || (nDot < nMinDot)) && (++i < 100));
#endif
*vRand = vr;
return vRand;
}

//------------------------------------------------------------------------------

int32_t CLightning::ComputeChildEnd (CFixVector *vPos, CFixVector *vEnd, CFixVector *vDir, CFixVector *vParentDir, int32_t nLength)
{
//nLength = 4 * nLength / 5 + int32_t (RandDouble () * nLength / 5);
DirectedRandomVector (vDir, vParentDir, I2X (85) / 100, I2X (95) / 100);
*vEnd = *vPos + *vDir * nLength;
return nLength;
}

//------------------------------------------------------------------------------

void CLightning::Setup (bool bInit)
{
	CFixVector		vPos, vDir, vRefDir, vDelta [2], v;
	int32_t			i = 0, l;

m_vPos = m_vBase;
if (m_bRandom) {
	if (!m_nAngle)
		VmRandomVector (&vDir);
	else {
		int32_t nMinDot = I2X (1) - I2X (m_nAngle) / 90;
		vRefDir = m_vRefEnd - m_vPos;
		CFixVector::Normalize (vRefDir);
		do {
			VmRandomVector (&vDir);
		} while (CFixVector::Dot (vRefDir, vDir) < nMinDot);
	}
	m_vEnd = m_vPos + vDir * m_nLength;
}
else {
	vDir = m_vEnd - m_vPos;
	CFixVector::Normalize (vDir);
	}
m_vDir = vDir;
if (m_nOffset) {
	i = m_nOffset / 2 + int32_t (RandDouble () * m_nOffset / 2);
	m_vPos += vDir * i;
	m_vEnd += vDir * i;
	}
vPos = m_vPos;
if (m_bInPlane && !m_vDelta.IsZero ()) {
	vDelta [0] = m_vDelta;
	vDelta [1].SetZero ();
	}
else {
	do {
		VmRandomVector (&vDelta [0]);
	} while (abs (CFixVector::Dot (vDir, vDelta [0])) > I2X (9) / 10);
	vDelta [1] = CFixVector::Normal (vPos, m_vEnd, vDelta [0]);
	v = vPos + vDelta [1];
	vDelta [0] = CFixVector::Normal (vPos, m_vEnd, v);
	}
vDir *= FixDiv (m_nLength, I2X (m_nNodes - 1));
m_nNodes = abs (m_nNodes);
m_iStep = 0;
if (m_parent) {
	i = m_parent->m_nChildren + 1;
	l = m_nNodes * m_parent->m_nLength / abs (m_parent->m_nNodes);
	m_nLength = ComputeChildEnd (&m_vPos, &m_vEnd, &m_vDir, &m_parent->m_vDir, l);// + 3 * l / (m_nNode + 1));
#if DBG
	if (m_nLength < 0)
		m_nLength = ComputeChildEnd (&m_vPos, &m_vEnd, &m_vDir, &m_parent->m_vDir, l);
#endif
	vDir = m_vDir * (m_nLength / (m_nNodes - 1));
	}
for (i = 0; i < m_nNodes; i++) {
	m_nodes [i].Setup (bInit, &vPos, vDelta);
	vPos += vDir;
	}
m_nFrames = -abs (m_nFrames);
}

//------------------------------------------------------------------------------

void CLightning::Init (CFixVector *vPos, CFixVector *vEnd, CFixVector *vDelta,
							  int16_t nObject, int32_t nLife, int32_t nDelay, int32_t nLength, int32_t nAmplitude,
							  char nAngle, int32_t nOffset, int16_t nNodes, int16_t nChildren, int16_t nFrames,
							  int16_t nSmoothe, char bClamp, char bGlow, char bLight,
							  char nStyle, float nWidth, CFloatVector *pColor, CLightning *pParent, int16_t nNode)
{
	int32_t	bRandom = (vEnd == NULL) || (nAngle > 0);

memset (this, 0, sizeof (*this));
if (OBJECT (nObject)) {
	m_nObject = -1;
	m_nSegment = -nObject - 1;
	}
else {
	m_nObject = nObject;
	}
m_bRandom = bRandom ? nNodes / 5 : 0;
m_parent = pParent;
m_nNode = nNode;
m_nNodes = nNodes + m_bRandom;
m_nChildren = (extraGameInfo [0].bUseLightning > 1) ? (nChildren < 0) ? (nNodes - m_bRandom) / 10 : nChildren : 0;
if (m_nChildren > m_nNodes)
	m_nChildren = m_nNodes;
if (vEnd) {
	m_vRefEnd = *vEnd;
	m_vEnd = *vEnd;
	}
if (vDelta)
	m_vDelta = *vDelta;
else
	m_vDelta.SetZero ();
m_bInPlane = !m_vDelta.IsZero ();
m_nLife = nLife;
m_nTTL = abs (nLife);
m_nLength = nLength;
if (bRandom) {
	m_nLength *= m_nNodes;
	m_nLength /= nNodes;
	}
#if DBG
if (m_nLength < 0)
	BRP;
#endif
m_nDelay = abs (nDelay) * 10;
m_nAmplitude = nAmplitude;
m_nAngle = nAngle;
m_nOffset = nOffset;
m_nFrames = -nFrames;
m_nSmoothe = nSmoothe;
m_bClamp = bClamp;
m_bGlow = bGlow;
m_iStep = 0;
m_color = *pColor;
m_vBase = *vPos;
m_bLight = bLight;
m_nStyle = nStyle;
m_width = nWidth;
}

//------------------------------------------------------------------------------

bool CLightning::Create (char nDepth, int32_t nThread)
{
if ((m_nObject >= 0) && (0 > (m_nSegment = OBJECT (m_nObject)->info.nSegment)))
	return NULL;
if (!m_nodes.Create (m_nNodes))
	return false;
if (nDepth && (m_bGlow > 0))
	m_bGlow = 0;
if (gameOpts->render.lightning.bGlow && m_bGlow) {
	int32_t h = ((m_bGlow > 0) ? 2 : 1) * (m_nNodes - 1) * 4;
	if (!m_plasmaTexCoord.Create (h))
		return false;
	if (!m_plasmaVerts.Create (h))
		return false;
	}
else
	m_bGlow = 0;
if (!m_coreVerts.Create ((m_nNodes + 3) * 4))
	return false;
m_nodes.Clear ();
if (m_bRandom) {
	m_nTTL = 3 * m_nTTL / 4 + int32_t (RandDouble () * m_nTTL / 2);
	m_nLength = 3 * m_nLength / 4 + int32_t (RandDouble () * m_nLength / 2);
	}
if (m_nAmplitude < 0)
	m_nAmplitude = m_nLength / 6;
Setup (true);
if ((extraGameInfo [0].bUseLightning > 1) && nDepth && m_nChildren) {
	int32_t nBranches = 0;
	double nProb = double (m_nChildren) / double (m_nNodes);
	int32_t nOffset = m_nNodes / m_nChildren / 2;
	for (int32_t nNode = 1 + (nOffset ? Rand (nOffset) : 0); (nNode < m_nNodes - nOffset - m_bRandom) && (nBranches < m_nChildren); nNode++) {
		if (RandDouble () <= nProb) {
			nBranches++;
			int32_t nChildNodes = m_nNodes - nNode;
			if (nChildNodes > 1) {
				int32_t h = 4 * nChildNodes / 5;
				nChildNodes = Max (2, h + Rand ((nChildNodes - h)));
				double scale = sqrt (double (nChildNodes) / double (m_nNodes));
				int32_t l = (int32_t) DRound (m_nLength * scale);
				int32_t n = (int32_t) DRound (m_nFrames * scale);
				if (n == 0)
					n = (m_nFrames < 0) ? -1 : 1;
				if (!m_nodes [nNode].CreateChild (&m_vEnd, &m_vDelta, m_nLife, l, (int32_t) DRound (m_nAmplitude * scale), m_nAngle,
															 nChildNodes, m_nChildren / 5, nDepth - 1, n, m_nSmoothe, m_bClamp, m_bGlow, m_bLight,
															 m_nStyle, m_width, &m_color, this, nNode, nThread))
					return false;
				}
			}
		}
	}
return true;
}

//------------------------------------------------------------------------------

void CLightning::DestroyNodes (void)
{
	CLightningNode	*pNode = m_nodes.Buffer ();

if (pNode) {
	int32_t i;
	for (i = abs (m_nNodes); i > 0; i--, pNode++)
		pNode->Destroy ();
	m_nodes.Destroy ();
	m_plasmaVerts.Destroy ();
	m_plasmaTexCoord.Destroy ();
	m_coreVerts.Destroy ();
	m_nNodes = 0;
	}
}

//------------------------------------------------------------------------------

void CLightning::Destroy (void)
{
DestroyNodes ();
}

//------------------------------------------------------------------------------

void CLightning::Smoothe (void)
{
	CLightningNode	*plh, *pfi, *pfj;
	int32_t			i, j;

for (i = m_nNodes - 1, j = 0, pfi = m_nodes.Buffer (), plh = NULL; j < i; j++) {
	pfj = plh;
	plh = pfi++;
	if (j) {
		plh->m_vNewPos.v.coord.x = pfj->m_vNewPos.v.coord.x / 4 + plh->m_vNewPos.v.coord.x / 2 + pfi->m_vNewPos.v.coord.x / 4;
		plh->m_vNewPos.v.coord.y = pfj->m_vNewPos.v.coord.y / 4 + plh->m_vNewPos.v.coord.y / 2 + pfi->m_vNewPos.v.coord.y / 4;
		plh->m_vNewPos.v.coord.z = pfj->m_vNewPos.v.coord.z / 4 + plh->m_vNewPos.v.coord.z / 2 + pfi->m_vNewPos.v.coord.z / 4;
		}
	}
}

//------------------------------------------------------------------------------

void CLightning::ComputeOffsets (void)
{
if (m_nNodes > 0)
	for (int32_t i = 1, j = m_nNodes - 1 - (m_bRandom == 0); i < j; i++)
		m_nodes [i].ComputeOffset (m_nFrames);
}

//------------------------------------------------------------------------------
// Make sure max. amplitude is reached every once in a while

void CLightning::Bump (void)
{
	CLightningNode	*pNode;
	int32_t			h, i, nDist, nAmplitude, nMaxDist = 0;
	CFixVector	vBase [2];

nAmplitude = m_nAmplitude;
vBase [0] = m_vPos;
vBase [1] = m_nodes [m_nNodes - 1].m_vPos;
for (i = m_nNodes - 1 - (m_bRandom == 0), pNode = m_nodes + 1; i > 0; i--, pNode++) {
	nDist = CFixVector::Dist (pNode->m_vNewPos, pNode->m_vPos);
	if (nMaxDist < nDist) {
		nMaxDist = nDist;
		h = i;
		}
	}
if ((h = nAmplitude - nMaxDist)) {
	if (m_nNodes > 0) {
		nMaxDist += (Rand (4) + 1) * h / 4;
		for (i = m_nNodes - 1 - (m_bRandom == 0), pNode = m_nodes + 1; i > 0; i--, pNode++)
			pNode->m_vOffs *= FixDiv (nAmplitude, nMaxDist);
		}
	}
}

//------------------------------------------------------------------------------
// The end point of Perlin generated lightning do not necessarily lay at
// the origin. This function rotates the lightning path so that it does.

void CLightning::Rotate (int32_t nFrames)
{
CFloatVector v0, v1, vBase;
v0.Assign (m_vEnd - m_vPos);
v1.Assign (m_nodes [m_nNodes - 1].m_vNewPos - m_vPos);
vBase.Assign (m_vPos);
float len0 = v0.Mag ();
float len1 = v1.Mag ();
len1 *= len1;
for (int32_t i = 1; i < m_nNodes; i++) {
	m_nodes [i].Rotate (v0, len0, v1, len1, vBase, nFrames);
	}
}

//------------------------------------------------------------------------------
// Paths of lightning with style 1 isn't affected by amplitude. This function scales
// these paths with the amplitude.

void CLightning::Scale (int32_t nFrames, int32_t nAmplitude)
{
	fix	nOffset, nMaxOffset = 0;

for (int32_t i = 1; i < m_nNodes; i++) {
	nOffset = m_nodes [i].Offset (m_vPos, m_vEnd);
	if (nMaxOffset < nOffset)
		nMaxOffset = nOffset;
	}

nAmplitude = 4 * nAmplitude / 5 + (Rand (nAmplitude / 5));
CFloatVector vStart, vEnd;
vStart.Assign (m_vPos);
vEnd.Assign (m_vEnd);
float scale = X2F (nAmplitude) / X2F (nMaxOffset);

for (int32_t i = 1; i < m_nNodes; i++)
	m_nodes [i].Scale (vStart, vEnd, scale, nFrames);

}

//------------------------------------------------------------------------------

#if NOISE_TYPE
static double ampScale = 1.5;
static double persistence = 0.5;
static int32_t octaves = 6;
#else
static double ampScale = 1.5;
static double persistence = 2.0 / 3.0;
static int32_t octaves = 6;
#endif

void CLightning::CreatePath (int32_t nDepth, int32_t nThread)
{
	CLightningNode*	plh, * pNode [2];
	int32_t				h, i, j, nFrames, nStyle, nSmoothe, bClamp, nMinDist, nAmplitude, bPrevOffs [2] = {0,0};
	CFixVector			vPos [2], vBase [2], vPrevOffs [2];

vBase [0] = vPos [0] = m_vPos;
vBase [1] = vPos [1] = m_vEnd;
nStyle = STYLE;
nFrames = m_nFrames;
nSmoothe = m_nSmoothe;
bClamp = m_bClamp;
nAmplitude = m_nAmplitude;
plh = m_nodes.Buffer ();
plh->m_vNewPos = plh->m_vPos;
plh->m_vOffs.SetZero ();
if ((nDepth > 1) || m_bRandom) {
	if (nStyle == 2) {
		noiseX [nThread].Setup (X2D (nAmplitude) * ampScale, persistence, octaves);
		noiseY [nThread].Setup (X2D (nAmplitude) * ampScale, persistence, octaves);
		for (i = 0; i < m_nNodes; i++)
			m_nodes [i].CreatePerlin (m_nNodes, i, nThread);
		Rotate (nFrames);
		}
	else if (nStyle == 1) {
		nMinDist = m_nLength / (m_nNodes - 1);
		for (i = m_nNodes - 1, plh = m_nodes + 1; i > 0; i--, plh++, bPrevOffs [0] = 1)
			*vPrevOffs = plh->CreateJaggy (vPos, vPos + 1, vBase, bPrevOffs [0] ? vPrevOffs : NULL, nFrames, nAmplitude, nMinDist, i, nSmoothe, bClamp);
		Scale (nFrames, nAmplitude);
		}
	else {
		int32_t bInPlane = m_bInPlane && (nDepth == 1); 
		if (bInPlane)
			nStyle = 0;
		nMinDist = m_nLength / (m_nNodes - 1);
		nAmplitude *= (nDepth == 1) ? 4 : 16;
		for (h = m_nNodes - 1, i = 0, plh = m_nodes + 1; i < h; i++, plh++) 
			plh->CreateErratic (vPos, vBase, nFrames, nAmplitude, 0, bInPlane, 1, i, h + 1, nSmoothe, bClamp);
		}
	}
else {
	plh = &m_nodes [m_nNodes - 1];
	plh->m_vNewPos = plh->m_vPos;
	plh->m_vOffs.SetZero ();
	if (nStyle == 2) {
		noiseX [nThread].Setup (X2D (nAmplitude) * ampScale, persistence, octaves);
		noiseY [nThread].Setup (X2D (nAmplitude) * ampScale, persistence, octaves);
		for (i = 0, plh = m_nodes.Buffer (); i < m_nNodes; i++, plh++)
			plh->CreatePerlin (m_nNodes, i, nThread);
		Rotate (nFrames);
		}
	else if (nStyle == 1) {
		for (i = m_nNodes - 1, j = 0, pNode [0] = m_nodes + 1, pNode [1] = &m_nodes [i - 1]; i > 0; i--, j = !j) {
			plh = pNode [j];
			vPrevOffs [j] = plh->CreateJaggy (vPos + j, vPos + !j, vBase, bPrevOffs [j] ? vPrevOffs + j : NULL, nFrames, nAmplitude, 0, i, nSmoothe, bClamp);
			bPrevOffs [j] = 1;
			if (pNode [1] <= pNode [0])
				break;
			if (j)
				pNode [1]--;
			else
				pNode [0]++;
			}
		Scale (nFrames, nAmplitude);
		}
	else {
		int32_t bInPlane = m_bInPlane && (nDepth == 1); 
		if (bInPlane)
			nStyle = 0;
		nAmplitude *= 3;
		for (h = m_nNodes - 1, i = j = 0, pNode [0] = m_nodes + 1, pNode [1] = &m_nodes [h - 1]; i < h; i++, j = !j) {
			plh = pNode [j];
			plh->CreateErratic (vPos + j, vBase, nFrames, nAmplitude, bInPlane, j, 0, i, h, nSmoothe, bClamp);
			if (pNode [1] <= pNode [0])
				break;
			if (j)
				pNode [1]--;
			else
				pNode [0]++;
			}
		}
	}
if (nStyle < 2) {
	Smoothe ();
	ComputeOffsets ();
#if 0
	Bump ();
#endif
	}
}

//------------------------------------------------------------------------------

void CLightning::Animate (int32_t nDepth, int32_t nThread)
{
	CLightningNode*	pNode;
	int32_t				j;
	bool					bInit;

m_nTTL -= gameStates.app.tick40fps.nTime;
if (m_nNodes > 0) {
	if ((bInit = (m_nFrames < 0)))
		m_nFrames = -m_nFrames;
	if (!m_iStep) {
		CreatePath (nDepth + 1, nThread);
		m_iStep = m_nFrames;
		}
	for (j = m_nNodes - 1 - (m_bRandom == 0), pNode = m_nodes + 1; j > 0; j--, pNode++)
		pNode->Animate (bInit, m_nSegment, nDepth, nThread);
	RenderSetup (nDepth, nThread);
	(m_iStep)--;
	}
}

//------------------------------------------------------------------------------

int32_t CLightning::SetLife (void)
{
	int32_t	h;

if (m_nTTL <= 0) {
	if (m_nLife < 0) {
		if ((m_nDelay > 1) && (0 > (m_nNodes = -m_nNodes))) {
			h = m_nDelay / 2;
			m_nTTL = h + int32_t (RandDouble () * h);
			}
		else {
			if (m_bRandom) {
				h = -m_nLife;
				m_nTTL = 3 * h / 4 + int32_t (RandDouble () * h / 2);
				Setup (0);
				}
			else {
				m_nTTL = -m_nLife;
				m_nNodes = abs (m_nNodes);
				Setup (0);
				}
			}
		}
	else {
		DestroyNodes ();
		return 0;
		}
	}
return 1;
}

//------------------------------------------------------------------------------

int32_t CLightning::Update (int32_t nDepth, int32_t nThread)
{
Animate (nDepth, nThread);
RenderSetup (nDepth, nThread);
return SetLife ();
}

//------------------------------------------------------------------------------

void CLightning::Move (CFixVector vNewPos, CFixVector vNewEnd, int16_t nSegment)
{
if (nSegment < 0)
	return;
if (!m_nodes.Buffer ())
	return;
if (m_nNodes < 0)
	return;
if ((vNewPos == m_vPos) && (vNewEnd == m_vEnd))
	return;

	fix					xNewLength = CFixVector::Dist (vNewEnd, vNewPos);
	float					fScale = X2F (xNewLength) / X2F (m_nLength);
	CLightningNode*	pNode = m_nodes.Buffer ();

for (int32_t i = m_nNodes; i; i--, pNode++) 
	pNode->Move (m_vPos, m_vEnd, vNewPos, vNewEnd, fScale, nSegment);
m_nodes [0].m_vPos =
m_vPos = m_vBase = vNewPos;
m_nodes [m_nNodes - 1].m_vPos =
m_vEnd = vNewEnd;
m_vDir = m_vEnd - m_vPos;
m_nLength = xNewLength;
#if DBG
if (m_nLength < 0)
	BRP;
#endif
m_nSegment = nSegment;
}

//------------------------------------------------------------------------------

void CLightning::Move (CFixVector vNewPos, int16_t nSegment)
{
Move (vNewPos, m_vEnd + (vNewPos - m_vPos), nSegment);
}

//------------------------------------------------------------------------------

inline int32_t CLightning::MayBeVisible (int32_t nThread)
{
if (m_nSegment >= 0)
	return SegmentMayBeVisible (m_nSegment, m_nLength / I2X (20), 3 * m_nLength / 2, nThread);
if (m_nObject >= 0)
	return (gameData.render.mine.bObjectRendered [m_nObject] == gameStates.render.nFrameFlipFlop);
return 1;
}

//------------------------------------------------------------------------------

#define LIGHTNING_VERT_ARRAYS 1

static tTexCoord2f plasmaTexCoord [3][4] = {
 {{{0,0.45f}},{{1,0.45f}},{{1,0.55f}},{{0,0.55f}}},
 {{{0,0.15f}},{{1,0.15f}},{{1,0.5f}},{{0,0.5f}}},
 {{{0,0.5f}},{{1,0.5f}},{{1,0.85f}},{{0,0.85f}}}
	};

//------------------------------------------------------------------------------

static inline int32_t GlowType (void)
{
return (gameOpts->render.lightning.bGlow || glowRenderer.Available (GLOW_LIGHTNING)) ? 1 : 0;
}

//------------------------------------------------------------------------------

float CLightning::ComputeAvgDist (CFloatVector3* pVertex, int32_t nVerts)
{
	CFloatVector3 v;
	float zMin = 1e30f, zMax = -1e30f;

while (nVerts-- > 0) {
	transformation.Transform (v, *pVertex++);
	if (zMin > v.v.coord.z)
		zMin = v.v.coord.z;
	if (zMax < v.v.coord.z)
		zMax = v.v.coord.z;
	}
return m_fAvgDist = (zMin + zMax) / 2.0f;
}

//------------------------------------------------------------------------------

float CLightning::ComputeDistScale (float zPivot)
{
#if 1
return m_fDistScale = pow (1.0f - m_fAvgDist / (float) ZRANGE, 50.0f);
#else
if (zPivot < 0.0f) {
	zPivot = -zPivot;
	if (m_fAvgDist <= zPivot)
		return m_fDistScale = sqrt ((zPivot - m_fAvgDist) / zPivot);
	}
else {
	if (m_fAvgDist <= zPivot)
		return m_fDistScale = 1.0f + sqrt ((zPivot - m_fAvgDist) /*/ zPivot * 10.0f*/);
	if (m_fAvgDist <= 4 * zPivot)
		return m_fDistScale = sqrt ((zPivot - m_fAvgDist * 0.25f) / zPivot);
	}
return m_fDistScale = 0.0f; //sqrt (((float) ZRANGE - zPivot - m_fAvgDist)) / ((float) ZRANGE - zPivot);
#endif
}

//------------------------------------------------------------------------------
// Compute billboards around each lightning segment using the normal of the plane
// spanned by the lightning segment and the vector from the camera (eye) position
// to one lightning coordinate
// vPosf: Coordinates of two subsequent lightning bolt segments

void CLightning::ComputeGlow (int32_t nDepth, int32_t nThread)
{

	CLightningNode*	pNode = m_nodes.Buffer ();

if (!pNode)
	return;

	CFloatVector*		srcP, * dstP, vEye, vn, vd, 
							vPos [2] = {CFloatVector::ZERO, CFloatVector::ZERO};
	tTexCoord2f*		pTexCoord;
	int32_t				h, i, j;
	bool					bGlow = !nDepth && (m_bGlow > 0) && gameOpts->render.lightning.bGlow; // && glowRenderer.Available (GLOW_LIGHTNING);
#if 0
	float					fWidth = bGlow ? 4.0f : (m_bGlow > 0) ? 2.0f : (m_bGlow < 0) ? (m_width / 16.0f) : (m_width / 8.0f);
#else
	float					fWidth = bGlow ? m_width / 2.0f : (m_bGlow > 0) ? (m_width / 4.0f) : (m_bGlow < 0) ? (m_width / 16.0f) : (m_width / 8.0f);
#endif
	
fWidth = m_width * ComputeDistScale (-100.0f) * 0.25f;
if (nThread < 0)
	vEye.SetZero ();
else
	vEye.Assign (gameData.render.mine.viewer.vPos);
dstP = m_plasmaVerts.Buffer ();
pTexCoord = m_plasmaTexCoord.Buffer ();
for (h = m_nNodes - 1 - m_bRandom, i = 0; i <= h; i++, pNode++) {
#if DBG
	if (i >= h - 1)
		BRP;
#endif
	vPos [0] = vPos [1];
	vPos [1].Assign (pNode->m_vPos);
	//if (nThread < 0)
	//	transformation.Transform (vPos [1], vPos [1]);
	if (i) {
		vn = CFloatVector::Normal (vPos [0], vPos [1], vEye);
		vn *= fWidth;
		if (i == 1) {
			vd = vPos [0] - vPos [1];
			vd *= m_width / 4.0f;
			vPos [0] += vd;
			}
		if (i == h) {
			vd = vPos [1] - vPos [0];
			vd = vd * m_width / 4.0f;
			vPos [1] += vd;
			}
		*dstP++ = vPos [0] + vn;
		*dstP++ = vPos [0] - vn;
		*dstP++ = vPos [1] - vn;
		*dstP++ = vPos [1] + vn;
		memcpy (pTexCoord, plasmaTexCoord [0], 4 * sizeof (tTexCoord2f));
		pTexCoord += 4;
		}
	}
memcpy (pTexCoord - 4, plasmaTexCoord [2], 4 * sizeof (tTexCoord2f));
memcpy (&m_plasmaTexCoord [0], plasmaTexCoord [1], 4 * sizeof (tTexCoord2f));

dstP = m_plasmaVerts.Buffer ();
for (h = 4 * (m_nNodes - 2 - m_bRandom), i = 2, j = 4; i < h; i += 4, j += 4) {
	dstP [i+1] = dstP [j] = CFloatVector::Avg (dstP [i+1], dstP [j]);
	dstP [i] = dstP [j+1] = CFloatVector::Avg (dstP [i], dstP [j+1]);
	}

if (bGlow) {
	int32_t h = 4 * (m_nNodes - 1 - m_bRandom);
	//for (j = 0; j < 2; j++) 
		{
		memcpy (pTexCoord, pTexCoord - h, h * sizeof (tTexCoord2f));
		pTexCoord += h;
		srcP = dstP;
		dstP += h;
		for (i = 0; i < h; i += 2) {
			vPos [0] = CFloatVector::Avg (srcP [i], srcP [i+1]);
			vPos [1] = srcP [i] - srcP [i+1];
			vPos [1] /= 8;
			dstP [i] = vPos [0] + vPos [1];
			dstP [i+1] = vPos [0] - vPos [1];
			}
#if 0
		m_plasmaVerts [j+1][0] += (m_plasmaVerts [j+1][2] - m_plasmaVerts [j+1][0]) / 4;
		m_plasmaVerts [j+1][1] += (m_plasmaVerts [j+1][3] - m_plasmaVerts [j+1][1]) / 4;
		m_plasmaVerts [j+1][h-2] += (m_plasmaVerts [j+1][h-2] - m_plasmaVerts [j+1][h-4]) / 4;
		m_plasmaVerts [j+1][h-3] += (m_plasmaVerts [j+1][h-3] - m_plasmaVerts [j+1][h-1]) / 4;
#endif
		}
	}
}

//------------------------------------------------------------------------------

void CLightning::RenderSetup (int32_t nDepth, int32_t nThread)
{
if ((GlowType () == 1) && m_bGlow && m_plasmaVerts.Buffer ()) {
	if (m_coreVerts.Buffer ())
		ComputeCore ();
	ComputeGlow (nDepth, nThread);
	}
else if (m_coreVerts.Buffer ())
	ComputeCore ();
else
	return;
if (extraGameInfo [0].bUseLightning > 1)
	for (int32_t i = 0; i < m_nNodes - m_bRandom; i++)
		if (m_nodes [i].GetChild ())
			m_nodes [i].GetChild ()->RenderSetup (nDepth + 1, nThread);
}

//------------------------------------------------------------------------------

void CLightning::RenderGlow (CFloatVector *pColor, int32_t nDepth, int32_t nThread)
{
if (!m_plasmaVerts.Buffer ())
	return;
#if RENDER_LIGHTNING_OUTLINE
	tTexCoord2f*	pTexCoord;
	CFloatVector*	pVertex;
#endif

OglTexCoordPointer (2, GL_FLOAT, 0, m_plasmaTexCoord.Buffer ());
OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), m_plasmaVerts.Buffer ());
ogl.SetBlendMode (OGL_BLEND_ADD);
if (nDepth || (m_bGlow < 1)) {
	glColor3fv (reinterpret_cast<GLfloat*> (pColor));
	OglDrawArrays (GL_QUADS, 0, 4 * (m_nNodes - 1 - m_bRandom));
	}
else {
	int32_t h = 4 * (m_nNodes - 1 - m_bRandom);
	glColor3f (pColor->Red () / 2.0f, pColor->Green () / 2.0f, pColor->Blue () / 2.0f);
	for (int32_t i = 1; i >= 0; i--) {
		OglDrawArrays (GL_QUADS, i * h, h);
#if RENDER_LIGHTNING_OUTLINE
		if (i != 1)
			continue;
		ogl.SetTexturing (false);
		glColor3f (1,1,1);
		pTexCoord = m_plasmaTexCoord.Buffer ();
		pVertex = m_plasmaVerts.Buffer ();
		for (int32_t i = 0; i < m_nNodes - 1; i++) {
#if 1
			OglDrawArrays (GL_LINE_LOOP, i * 4, 4);
#else
			glBegin (GL_LINE_LOOP);
			for (int32_t j = 0; j < 4; j++) {
				glTexCoord2fv (reinterpret_cast<GLfloat*> (pTexCoord++));
				glVertex3fv (reinterpret_cast<GLfloat*> (pVertex++));
				}
			glEnd ();
#endif
			}
#endif
		}
	}
ogl.DisableClientStates (1, 0, 0, GL_TEXTURE0);
}

//------------------------------------------------------------------------------

void CLightning::ComputeCore (void)
{
	CFloatVector3*	pVertex, * vPosf;
	int32_t				i;

vPosf = pVertex = &m_coreVerts [0];

for (i = 0; i < m_nNodes; i++, vPosf++)
	vPosf->Assign (m_nodes [i].m_vPos);

*vPosf = pVertex [0] - pVertex [1];
*vPosf /= 100.0f * vPosf->Mag ();
*vPosf += pVertex [0];
*++vPosf = pVertex [0];
i = m_nNodes - 1;
*++vPosf = pVertex [i];
*++vPosf = pVertex [i] - pVertex [i - 1];
*vPosf /= 100.0f * vPosf->Mag ();
*vPosf += pVertex [i];
ComputeAvgDist (m_coreVerts.Buffer (), m_nNodes);
}

//------------------------------------------------------------------------------

void CLightning::RenderCore (CFloatVector *pColor, int32_t nDepth, int32_t nThread)
{
#if 0 //DBG
RenderTestImage ();
return;
#endif
if (!m_coreVerts.Buffer ())
	return;
ogl.SetBlendMode (OGL_BLEND_ADD);
ogl.SetLineSmooth (true);
if (ogl.EnableClientStates (0, 0, 0, GL_TEXTURE0)) {
	ogl.SetTexturing (false);
	glColor4fv ((GLfloat*) pColor);
	GLfloat w = nDepth ? m_width / 2.0f : m_width; // DEFAULT_CORE_WIDTH : DEFAULT_CORE_WIDTH * 1.5f;
	//ComputeDistScale (100.0f);
	if (glowRenderer.Available (GLOW_LIGHTNING) && (m_fDistScale != 0.0f)) 
		w *= 2.0f * m_fDistScale;
	glLineWidth ((w > 1.0f) ? w : 1.0f);
	OglVertexPointer (3, GL_FLOAT, 0, m_coreVerts.Buffer ());
	OglDrawArrays (GL_LINE_STRIP, 0, m_nNodes - m_bRandom);
	ogl.DisableClientStates (0, 0, 0, -1);
	}
#if GL_FALLBACK
else {
	ogl.SelectTMU (GL_TEXTURE0);
	ogl.SetTexturing (false);
	glBegin (GL_LINE_STRIP);
	for (i = m_nNodes, vPosf = coreBuffer [nThread]; i; i--, vPosf++)
		glVertex3fv (reinterpret_cast<GLfloat*> (vPosf));
	glEnd ();
	}
#endif
glLineWidth (GLfloat (1));
ogl.SetLineSmooth (false);
ogl.ClearError (0);
}

//------------------------------------------------------------------------------

int32_t CLightning::SetupGlow (void)
{
if (/*m_bGlow &&*/ gameOpts->render.lightning.bGlow) {
	glowRenderer.Begin (GLOW_LIGHTNING, 2, false, 1.05f);
	ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
	ogl.SelectTMU (GL_TEXTURE0, true);
	ogl.SetTexturing (true);
	if (corona.Load () && !corona.Bitmap ()->Bind (1)) {
		corona.Texture ()->Wrap (GL_CLAMP);
		return 1;
		}
	}
glowRenderer.Begin (GLOW_LIGHTNING, 3, false, 1.1f);
ogl.DisableClientStates (1, 0, 0, GL_TEXTURE0);
return 0;
}

//------------------------------------------------------------------------------

#if 0 //!USE_OPENMP

static inline void WaitForRenderThread (int32_t nThread)
{
if (gameStates.app.bMultiThreaded && (nThread > 0)) {	//thread 1 will always render after thread 0
	tiRender.ti [1].bBlock = 0;
	while (tiRender.ti [0].bBlock)
		G3_SLEEP (0);
	}
}

#endif

//------------------------------------------------------------------------------

void CLightning::Draw (int32_t nDepth, int32_t nThread)
{
	int32_t				i, bGlow;
	CFloatVector		color;

if (!m_nodes.Buffer () || (m_nNodes <= 0) || (m_nFrames < 0))
	return;
#if 0 //!USE_OPENMP
if (gameStates.app.bMultiThreaded && (nThread > 0))
	tiRender.ti [nThread].bBlock = 1;
#endif
color = m_color;
if (m_nLife > 0) {
	if ((i = m_nLife - m_nTTL) < 250)
		color.Alpha () *= (float) i / 250.0f;
	else if (m_nTTL < m_nLife / 4)
		color.Alpha () *= (float) m_nTTL / (float) (m_nLife / 4);
	}
color.Red () *= (float) (0.9 + RandDouble () / 5);
color.Green () *= (float) (0.9 + RandDouble () / 5);
color.Blue () *= (float) (0.9 + RandDouble () / 5);
ComputeDistScale (100.0f);
if ((bGlow = (m_fDistScale > 0.0f) && SetupGlow ()) && glowRenderer.Available (GLOW_LIGHTNING))
	glBlendEquation (GL_MAX);
else
	color.Alpha () *= 1.5f;
if (nDepth)
	color.Alpha () /= 2;
#if 0 //!USE_OPENMP
WaitForRenderThread (nThread);
#endif
#if DBG
if (m_fDistScale > 1.0f)
	BRP;
#endif
if ((GlowType () == 1) && bGlow && m_bGlow && m_plasmaVerts.Buffer ()) {
	if (glowRenderer.SetViewport (GLOW_LIGHTNING, m_plasmaVerts.Buffer (), 4 * (m_nNodes - 1))) {
		RenderGlow (&color, nDepth, nThread);
		RenderCore (&color, nDepth, nThread);
		}
	}
else {
	if (!bGlow || glowRenderer.SetViewport (GLOW_LIGHTNING, m_coreVerts.Buffer (), m_nNodes))
		RenderCore (&color, nDepth, nThread);
	}
if (bGlow)
	glBlendEquation (GL_FUNC_ADD);
#if 0 //!USE_OPENMP
WaitForRenderThread (nThread);
#endif
if (extraGameInfo [0].bUseLightning > 1)
		for (i = 0; i < m_nNodes; i++)
			if (m_nodes [i].GetChild ())
				m_nodes [i].GetChild ()->Draw (nDepth + 1, nThread);
glowRenderer.Done (GLOW_LIGHTNING);
ogl.ClearError (0);
}

//------------------------------------------------------------------------------

void CLightning::Render (int32_t nDepth, int32_t nThread)
{
if ((gameStates.render.nType != RENDER_TYPE_TRANSPARENCY) && (nThread >= 0)) {	// not in transparency renderer
	if ((m_nNodes < 0) || (m_nFrames < 0))
		return;
	if (!MayBeVisible (nThread))
		return;
#if 0
	if (!gameOpts->render.stereo.nGlasses) 
		RenderSetup (0, nThread);
#endif
	transparencyRenderer.AddLightning (this, nDepth);
	if (extraGameInfo [0].bUseLightning > 1)
		for (int32_t i = 0; i < m_nNodes; i++)
			if (m_nodes [i].GetChild ())
				m_nodes [i].GetChild ()->Render (nDepth + 1, nThread);
	}
else {
	if (!nDepth)
		ogl.SetFaceCulling (false);
#if 0 //DBG
	nThread = -1;
#endif
	if (nThread >= 0)
		ogl.SetupTransform (1);
	Draw (0, nThread);
	if (nThread >= 0)
		ogl.ResetTransform (1);
	if (!nDepth)
		ogl.SetFaceCulling (true);
	glLineWidth (1);
	ogl.SetLineSmooth (false);
	ogl.SetBlendMode (OGL_BLEND_ALPHA);
	}
}

//------------------------------------------------------------------------------

int32_t CLightning::SetLight (void)
{
	int32_t		j, nLights = 0, nStride;
	double	h, nStep;

if (!m_bLight)
	return 0;
if (!m_nodes.Buffer ())
	return 0;
if (0 < (j = m_nNodes)) {
	if (!(nStride = (int32_t) DRound ((double (m_nLength) / I2X (20)))))
		nStride = 1;
	if (0 >= (nStep = double (j - 1) / double (nStride)))
		nStep = double ((int32_t) DRound (j));
#if DBG
	if (m_nSegment == nDbgSeg)
		BRP;
#endif
	for (h = nStep / 2; h < j; h += nStep) {
		if (!m_nodes [int32_t (h)].SetLight (m_nSegment, &m_color))
			break;
		nLights++;
		}
	}
return nLights;
}

//------------------------------------------------------------------------------
//eof
