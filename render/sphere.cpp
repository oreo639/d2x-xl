// -----------------------------------------------------------------------------

#include <math.h>

#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

#include "descent.h"
#include "error.h"
#include "network.h"
#include "sphere.h"
#include "rendermine.h"
#include "maths.h"
#include "u_mem.h"
#include "glare.h"
#include "../effects/glow.h"
#include "ogl_lib.h"
#include "ogl_shader.h"
#include "objeffects.h"
#include "objrender.h"
#include "transprender.h"
#include "oof.h"
#include "addon_bitmaps.h"

#define ADDITIVE_SPHERE_BLENDING 1
#define MAX_SPHERE_RINGS 256

#if !RINGED_SPHERE

// TODO: Create a c-tor for the two tables

OOF::CTriangle baseSphereOcta [8] = {
 {{{-1,0,1},{1,0,1},{0,1,0}},{0,0,0}},
 {{{1,0,1},{1,0,-1},{0,1,0}},{0,0,0}},
 {{{1,0,-1},{-1,0,-1},{0,1,0}},{0,0,0}},
 {{{-1,0,-1},{-1,0,1},{0,1,0}},{0,0,0}},
 {{{1,0,1},{-1,0,1},{0,-1,0}},{0,0,0}},
 {{{1,0,-1},{1,0,1},{0,-1,0}},{0,0,0}},
 {{{-1,0,-1},{1,0,-1},{0,-1,0}},{0,0,0}},
 {{{-1,0,1},{-1,0,-1},{0,-1,0}},{0,0,0}}
};

OOF::CQuad baseSphereCube [6] = {
 {{{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}},{0,0,0}},
 {{{1,-1,1},{1,-1,-1},{1,1,-1},{1,1,1}},{0,0,0}},
 {{{1,-1,-1},{-1,-1,-1},{-1,1,-1},{1,1,-1}},{0,0,0}},
 {{{-1,-1,1},{-1,1,1},{-1,1,-1},{-1,-1,-1}},{0,0,0}},
 {{{-1,1,1},{1,1,1},{1,1,-1},{-1,1,-1}},{0,0,0}},
 {{{-1,-1,-1},{1,-1,-1},{1,-1,1},{-1,-1,1}},{0,0,0}}
};

#endif

// -----------------------------------------------------------------------------

const char *pszSphereFS =
	"uniform sampler2D sphereTex;\r\n" \
	"uniform vec4 vHit [3];\r\n" \
	"uniform vec3 fRad;\r\n" \
	"varying vec3 vertPos;\r\n" \
	"void main() {\r\n" \
	"vec3 scale;\r\n" \
	"scale.x = 1.0 - clamp (length (vertPos - vec3 (vHit [0])) * fRad.x, 0.0, 1.0);\r\n" \
	"scale.y = 1.0 - clamp (length (vertPos - vec3 (vHit [1])) * fRad.y, 0.0, 1.0);\r\n" \
	"scale.z = 1.0 - clamp (length (vertPos - vec3 (vHit [2])) * fRad.z, 0.0, 1.0);\r\n" \
	"gl_FragColor = texture2D (sphereTex, gl_TexCoord [0].xy) * gl_Color * max (scale.x, max (scale.y, scale.z));\r\n" \
	"}"
	;

// -----------------------------------------------------------------------------

const char *pszSphereVS =
	"varying vec3 vertPos;\r\n" \
	"void main() {\r\n" \
	"	gl_TexCoord [0] = gl_MultiTexCoord0;\r\n" \
	"	gl_Position = ftransform();\r\n" \
   "	gl_FrontColor = gl_Color;\r\n" \
	"	vertPos = vec3 (gl_Vertex);\r\n" \
	"	}"
	;

int32_t sphereShaderProg = -1;

// -----------------------------------------------------------------------------

int32_t CreateSphereShader (void)
{
if (!(ogl.m_features.bShaders && ogl.m_features.bPerPixelLighting.Available ())) {
	gameStates.render.bPerPixelLighting = 0;
	return 0;
	}
if (sphereShaderProg < 0) {
	PrintLog (1, "building sphere shader program\n");
	if (!shaderManager.Build (sphereShaderProg, pszSphereFS, pszSphereVS)) {
		ogl.m_features.bPerPixelLighting.Available (0);
		gameStates.render.bPerPixelLighting = 0;
		PrintLog (-1);
		return -1;
		}
	PrintLog (-1);
	}
return 1;
}

// -----------------------------------------------------------------------------

void ResetSphereShaders (void)
{
//sphereShaderProg = -1;
}

// -----------------------------------------------------------------------------

void UnloadSphereShader (void)
{
shaderManager.Deploy (-1);
}

// -----------------------------------------------------------------------------

int32_t SetupSphereShader (CObject* objP, float alpha)
{
	int32_t	nHits = 0;

PROF_START
if (CreateSphereShader () < 1) {
	PROF_END(ptShaderStates)
	return 0;
	}

	CObjHitInfo	hitInfo = objP->HitInfo ();
	float fSize = 1.0f + 2.0f / X2F (objP->Size ());
	float fScale [3];
	CFloatVector vHitf [3];

	tObjTransformation *posP = OBJPOS (objP);
	CFixMatrix m;
	CFixVector vPos;

if (!ogl.UseTransform ()) {
	fSize *= X2F (objP->Size ());
	ogl.SetupTransform (0);
	m = CFixMatrix::IDENTITY;
	transformation.Begin (*PolyObjPos (objP, &vPos), m); 
	}
else {
	m = posP->mOrient;
	m.Transpose (m);
	m = m.Inverse ();
	}
for (int32_t i = 0; i < 3; i++) {
	int32_t dt = gameStates.app.nSDLTicks [0] - int32_t (hitInfo.t [i]);
	if (dt < SHIELD_EFFECT_TIME) {
		float h = (fSize * float (cos (sqrt (float (dt) / float (SHIELD_EFFECT_TIME)) * PI / 2)));
		if (h > 1.0f / 1e6f) {
			fScale [i] = 1.0f / h;
			vHitf [i].v.coord.w = 0.0f;
			if (ogl.UseTransform ()) {
				vHitf [i].Assign (m * hitInfo.v [i]);
				CFloatVector::Normalize (vHitf [i]);
				}
			else {
				vHitf [i].Assign (hitInfo.v [i]);
				transformation.Transform (vHitf [i], vHitf [i]);
				}
			nHits++;
			}
		else {
			fScale [i] = 1e6f;
			vHitf [i].SetZero ();
			}
		}
	else {
		fScale [i] = 1e6f;
		vHitf [i].SetZero ();
		}
	}
if (!ogl.UseTransform ()) {
	transformation.End ();
	ogl.ResetTransform (1);
	}

if (!nHits)
	return 0;

GLhandleARB shaderProg = GLhandleARB (shaderManager.Deploy (sphereShaderProg));
if (shaderProg) {
	if (shaderManager.Rebuild (shaderProg))
		/*nothing*/;
		glUniform1i (glGetUniformLocation (shaderProg, "sphereTex"), 0);
	//if (shaderProg) 
		{
		glUniform4fv (glGetUniformLocation (shaderProg, "vHit"), 3, reinterpret_cast<GLfloat*> (vHitf));
		glUniform3fv (glGetUniformLocation (shaderProg, "fRad"), 1, reinterpret_cast<GLfloat*> (fScale));
		}
	}
ogl.ClearError (0);
PROF_END(ptShaderStates)
return shaderManager.Current ();
}

// -----------------------------------------------------------------------------

void CSphereData::Init (void)
{
#if !RINGED_SPHERE
m_nTessDepth = 0;
m_nFaces = 0;
m_nFaceNodes = 4; //tesselate using quads
#endif
m_pulseP = NULL;
m_nFrame = 0;
}

// -----------------------------------------------------------------------------

#if !RINGED_SPHERE

CFloatVector *OOF_TriangleCenter (OOF::CTriangle *pt)
{
pt->coord = (pt->p [0] + pt->p [1] + pt->p [2]) / 3.0f;
return &pt->coord;
}

// -----------------------------------------------------------------------------

static int32_t SplitTriangle (OOF::CTriangle *pDest, OOF::CTriangle *pSrc)
{
	int32_t	i, j;
	CFloatVector	coord = pSrc->coord;
	CFloatVector	h [6];

for (i = 0; i < 3; i++)
	h [2 * i] = pSrc->p [i];
for (i = 1; i < 6; i += 2)
	h [i] = h [i - 1] + h [(i + 1) % 6];
for (i = 0; i < 6; i++, pDest++) {
	pDest->p [0] = h [i];
	pDest->p [1] = h [(i + 1) % 6];
	pDest->p [2] = coord;
	for (j = 0; j < 3; j++)
		CFloatVector::Normalize (pDest->p [j]);
	OOF_TriangleCenter (pDest);
	}
return 1;
}

// -----------------------------------------------------------------------------

static int32_t TesselateSphereTri (OOF::CTriangle *pDest, OOF::CTriangle *pSrc, int32_t nFaces)
{
	int32_t	i;

for (i = 0; i < nFaces; i++, pDest += 6, pSrc++)
	SplitTriangle (pDest, pSrc);
return 1;
}

// -----------------------------------------------------------------------------

static int32_t BuildSphereTri (OOF::CTriangle **buf, int32_t *pnFaces, int32_t nTessDepth)
{
    int32_t		i, j, nFaces = 0;
	 float	l;

l = (float) sqrt (2.0f) / 2.0f;
for (i = 0; i < 8; i++) {
	for (j = 0; j < 3; j++) {
		buf [0][i].p [j] = baseSphereOcta [i].p [j];
		buf [0][i].p [j] *= l;
		}
	OOF_TriangleCenter (buf [0] + i);
	}
nFaces = 8;
for (i = 0, j = 1; i < nTessDepth; i++, nFaces *= 6, j = !j) {
	TesselateSphereTri (buf [j], buf [!j], nFaces);
	}
*pnFaces = nFaces;
return !j;
}

// -----------------------------------------------------------------------------

static CFloatVector *OOF_QuadCenter (OOF::CQuad *pt)
{
pt->coord = (pt->p [0] + pt->p [1] + pt->p [2] + pt->p [3]) / 4.0f;
return &pt->coord;
}

// -----------------------------------------------------------------------------

static int32_t SplitQuad (OOF::CQuad *pDest, OOF::CQuad *pSrc)
{
	int32_t	i, j;
	CFloatVector	coord = pSrc->coord;
	CFloatVector	h [8];

for (i = 0; i < 4; i++)
	h [2 * i] = pSrc->p [i];
for (i = 1; i < 8; i += 2)
	h [i] = h [i - 1] + h [(i + 1) % 8];
for (i = 0; i < 8; i += 2, pDest++) {
	pDest->p [0] = h [i ? i - 1 : 7];
	pDest->p [1] = h [i];
	pDest->p [2] = h [(i + 1) % 8];
	pDest->p [3] = coord;
	for (j = 0; j < 4; j++)
		CFloatVector::Normalize (pDest->p [j]);
	OOF_QuadCenter (pDest);
	}
return 1;
}

// -----------------------------------------------------------------------------

static int32_t TesselateSphereQuad (OOF::CQuad *pDest, OOF::CQuad *pSrc, int32_t nFaces)
{
	int32_t	i;

for (i = 0; i < nFaces; i++, pDest += 4, pSrc++)
	SplitQuad (pDest, pSrc);
return 1;
}

// -----------------------------------------------------------------------------

static int32_t BuildSphereQuad (OOF::CQuad **buf, int32_t *pnFaces, int32_t nTessDepth)
{
    int32_t		i, j, nFaces;
	 float	l;

l = (float) sqrt (2.0f) / 2.0f;
for (i = 0; i < 6; i++) {
	for (j = 0; j < 4; j++) {
		buf [0][i].p [j] = baseSphereCube [i].p [j];
		buf [0][i].p [j] *= l;
		}
	OOF_QuadCenter (buf [0] + i);
	}
nFaces = 6;
for (i = 0, j = 1; i < nTessDepth; i++, nFaces *= 4, j = !j) {
	TesselateSphereQuad (buf [j], buf [!j], nFaces);
	}
*pnFaces = nFaces;
return !j;
}

// -----------------------------------------------------------------------------

int32_t TesselateSphere (void)
{
	int32_t			nFaces, i, j;
	CFloatVector	*buf [2];

PrintLog (1, "Creating shield sphere\n");
if (m_nFaceNodes == 3) {
	nFaces = 8;
	j = 6;
	}
else {
	nFaces = 6;
	j = 4;
	}
for (i = 0; i < m_nTessDepth; i++)
	nFaces *= j;
for (i = 0; i < 2; i++) {
	if (!(buf [i] = new CFloatVector [nFaces * (m_nFaceNodes + 1)])) {
		if (i)
			delete[] buf [i - 1];
		PrintLog (-1);
		return -1;
		}
	}
j = (m_nFaceNodes == 3) ?
	 BuildSphereTri (reinterpret_cast<OOF::CTriangle **> (buf), &nFaces, m_nTessDepth) :
	 BuildSphereQuad (reinterpret_cast<OOF::CQuad **> (buf), &nFaces, m_nTessDepth);
delete[] buf [!j];
if (!m_texCoord.Create (nFaces * m_nFaceNodes)) {
	delete[] buf [j];
	PrintLog (-1);
	return -1;
	}
m_vertices.SetBuffer (buf [j]);
PrintLog (-1);
return nFaces;
}

// -----------------------------------------------------------------------------

OOF::CTriangle *RotateSphere (CFloatVector *rotSphereP, CFloatVector *vPosP, float xScale, float yScale, float zScale)
{
	CFloatMatrix	mat;
	CFloatVector	h, dir, p,
					*vertP = m_vertices.Buffer (),
					*s = rotSphereP;
	int32_t			nFaces;

OOF_MatVms2Oof (&mat, transformation.m_info.view[0]);
OOF_VecVms2Oof (&p, transformation.m_info.coord);
for (nFaces = m_nFaces * (m_nFaceNodes + 1); nFaces; nFaces--, vertP++, rotSphereP++) {
	dir = *vertP;
	dir.x *= xScale;
	dir.y *= yScale;
	dir.z *= zScale;
	rotSphereP = mat * (h = dir - p);
	}
return (OOF::CTriangle *) s;
}

// -----------------------------------------------------------------------------

OOF::CTriangle *SortSphere (OOF::CTriangle *sphereP, int32_t left, int32_t right)
{
	int32_t	l = left,
			r = right;
	float	mat = sphereP [(l + r) / 2].coord.z;

do {
	while (sphereP [l].coord.z < mat)
		l++;
	while (sphereP [r].coord.z > mat)
		r--;
	if (l <= r) {
		if (l < r) {
			OOF::CTriangle h = sphereP [l];
			sphereP [l] = sphereP [r];
			sphereP [r] = h;
			}
		}
	++l;
	--r;
	} while (l <= r);
if (right > l)
   Sort (sphereP, l, right);
if (r > left)
   Sort (sphereP, left, r);
return sphereP;
}

#endif //!RINGED_SPHERE

// -----------------------------------------------------------------------------

void CSphere::Pulsate (void)
{
if (m_pulseP) {
	static time_t	t0 = 0;
	if (gameStates.app.nSDLTicks [0] - t0 > 25) {
		t0 = gameStates.app.nSDLTicks [0];
		m_pulseP->fScale += m_pulseP->fDir;
		if (m_pulseP->fScale > 1.0f) {
			m_pulseP->fScale = 1.0f;
			m_pulseP->fDir = -m_pulseP->fSpeed;
			}
		else if (m_pulseP->fScale < m_pulseP->fMin) {
			m_pulseP->fScale = m_pulseP->fMin;
			m_pulseP->fDir = m_pulseP->fSpeed;
			}
		}
	}
}

// -----------------------------------------------------------------------------

void CSphere::Animate (CBitmap* bmP)
{
#if 1
if (bmP && (bmP == shield.Bitmap ())) {
	static time_t t0 = 0;
	if ((gameStates.app.nSDLTicks [0] - t0 > 40) && bmP->CurFrame ()) {
		t0 = gameStates.app.nSDLTicks [0];
		bmP->NextFrame ();
		}
	}
#endif
}

// -----------------------------------------------------------------------------

int32_t CSphere::InitSurface (float red, float green, float blue, float alpha, CBitmap *bmP, float fScale)
{
	int32_t	bTextured = bmP != NULL;

fScale = m_pulseP ? m_pulseP->fScale : 1.0f;
ogl.ResetClientStates (0);
if (bmP) {
	Animate (bmP);
	ogl.EnableClientStates (bTextured, 0, 0, GL_TEXTURE0);
	if (bmP->CurFrame ())
		bmP = bmP->CurFrame ();
	if (bmP->Bind (1))
		bmP = NULL;
	else {
		if (!bTextured)
			bTextured = -1;
		}
	}
if (!bmP) {
	ogl.SetTexturing (false);
	bTextured = 0;
	alpha /= 2;
	ogl.EnableClientStates (0, 0, 0, GL_TEXTURE0);
	}
if (alpha < 0)
	alpha = (float) (1.0f - gameStates.render.grAlpha / (float) FADE_LEVELS);
if (alpha < 1.0f) {
#if ADDITIVE_SPHERE_BLENDING
	fScale *= coronaIntensities [gameOpts->render.coronas.nObjIntensity];
#endif
	if (m_pulseP && m_pulseP->fScale) {
		red *= fScale;
		green *= fScale;
		blue *= fScale;
		}
	}
ogl.SetDepthWrite (false);
m_bmP = bmP;
m_color.Set (red, green, blue, alpha);
return bTextured;
}

// -----------------------------------------------------------------------------

#if RINGED_SPHERE

#define UV_SCALE	3.0f

int32_t CSphere::Create (int32_t nRings, int32_t nTiles)
{
	int32_t			h, i, j;
	float				t1, t2, t3, a, sint1, cost1, sint2, cost2, sint3, cost3;
	tSphereVertex*	svP;

if (nRings > MAX_SPHERE_RINGS)
	nRings = MAX_SPHERE_RINGS;
if ((m_nRings == nRings) && (m_nTiles == nTiles) && (m_vertices.Buffer () != NULL))
	return 1;

m_nRings =
m_nTiles = 
m_nVertices = 0;
m_vertices.Destroy ();
h = nRings * (nRings + 1);
if (!m_vertices.Create (h))
	return 0;
m_nRings = nRings;
m_nTiles = nTiles;
m_nVertices = h;
h = nRings / 2;
a = float (2 * PI / nRings);
svP = m_vertices.Buffer ();
for (j = 0; j < h; j++) {
	t1 = float (j * a - PI / 2);
	t2 = t1 + a;
	sint1 = float (sin (t1));
	cost1 = float (cos (t1));
	sint2 = float (sin (t2));
	cost2 = float (cos (t2));
	for (i = 0; i <= nRings; i++) {
		t3 = i * a;
		sint3 = float (sin (t3));
		cost3 = float (cos (t3));
		svP->vPos.v.coord.x = cost2 * cost3;
		svP->vPos.v.coord.y = sint2;
		svP->vPos.v.coord.z = cost2 * sint3;
		svP->uv.v.u =(1.0f - float (i) / nRings) * nTiles * UV_SCALE;
		svP->uv.v.v = (float (2 * j + 2) / nRings) * nTiles * UV_SCALE;
		svP++;
		svP->vPos.v.coord.x = cost1 * cost3;
		svP->vPos.v.coord.y = sint1;
		svP->vPos.v.coord.z = cost1 * sint3;
		svP->uv.v.u = (1.0f - float (i) / nRings) * nTiles * UV_SCALE;
		svP->uv.v.v = (float (2 * j) / nRings) * nTiles * UV_SCALE;
		svP++;
		}
	}
return 1;
}

// -----------------------------------------------------------------------------

void CSphere::RenderRing (int32_t nOffset, int32_t nItems, int32_t bTextured, int32_t nPrimitive)
{
ogl.EnableClientStates (bTextured, 0, 0, GL_TEXTURE0);
if (bTextured && !m_bmP->Bind (1))
	OglTexCoordPointer (2, GL_FLOAT, sizeof (tSphereVertex), reinterpret_cast<GLfloat*> (&m_vertices [nOffset * nItems].uv));
OglVertexPointer (3, GL_FLOAT, sizeof (tSphereVertex), reinterpret_cast<GLfloat*> (&m_vertices [nOffset * nItems].vPos));
glColor4fv ((GLfloat*) m_color.v.vec);
OglDrawArrays (nPrimitive, 0, nItems);
}

// -----------------------------------------------------------------------------

void CSphere::RenderRing (CFloatVector *vertexP, tTexCoord2f *texCoordP, int32_t nItems, int32_t bTextured, int32_t nPrimitive)
{
ogl.EnableClientStates (bTextured, 0, 0, GL_TEXTURE0);
if (bTextured && !m_bmP->Bind (1))
	OglTexCoordPointer (2, GL_FLOAT, 0, texCoordP);
OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), vertexP);
glColor4fv ((GLfloat*) m_color.v.vec);
OglDrawArrays (nPrimitive, 0, nItems);
}

// -----------------------------------------------------------------------------

void CSphere::RenderRings (float fRadius, int32_t nRings, float red, float green, float blue, float alpha, int32_t bTextured, int32_t nTiles)
{
	int32_t			nCull, h, i, j, nQuads;
	CFloatVector	p [2 * MAX_SPHERE_RINGS + 2];
	tTexCoord2f		tc [2 * MAX_SPHERE_RINGS + 2];
	tSphereVertex*	svP [2];

if (nRings > MAX_SPHERE_RINGS)
	nRings = MAX_SPHERE_RINGS;
if (!Create (nRings, nTiles))
	return;
h = nRings / 2;
nQuads = 2 * nRings + 2;

ogl.EnableClientStates (bTextured, 0, 0, GL_TEXTURE0);
if (ogl.UseTransform ()) {
	glScalef (fRadius, fRadius, fRadius);
	for (nCull = 0; nCull < 2; nCull++) {
		svP [0] = svP [1] = m_vertices.Buffer ();
		ogl.SetCullMode (nCull ? GL_FRONT : GL_BACK);
		for (i = 0; i < h; i++) {
			RenderRing (i, nQuads, bTextured, GL_QUAD_STRIP);
#if 0
			if (!bTextured) {
				for (i = 0; i < nQuads; i++, svP [1]++) {
					p [i] = svP [1]->vPos;
					if (bTextured) {
						tc [i].dir.u = svP [1]->uv.dir.u * nTiles;
						tc [i].dir.dir = svP [1]->uv.dir.dir * nTiles;
						}
					}
				glLineWidth (2);
				RenderSphereRing (p, tc, nQuads, 0, GL_LINE_STRIP);
				glLineWidth (1);
				}
#endif
			}
		}
	}
else {
	for (nCull = 0; nCull < 2; nCull++) {
		ogl.SetCullMode (nCull ? GL_FRONT : GL_BACK);
		svP [0] = svP [1] = &m_vertices [0];
		for (j = 0; j < h; j++) {
			for (i = 0; i < nQuads; i++, svP [0]++) {
				p [i] = svP [0]->vPos * fRadius;
				transformation.Transform (p [i], p [i], 0);
				if (bTextured)
					tc [i] = svP [0]->uv;
				}
			RenderRing (p, tc, nQuads, bTextured, GL_QUAD_STRIP);
#if 0
			if (!bTextured) {
				for (i = 0; i < nQuads; i++, svP [1]++) {
					p [i] = svP [1]->vPos;
					VmVecScale (p + i, p + i, fRadius);
					transformation.Transform (p + i, p + i, 0);
					if (bTextured) {
						tc [i].dir.u = svP [1]->uv.dir.u * nTiles;
						tc [i].dir.dir = svP [1]->uv.dir.dir * nTiles;
						}
					}
				glLineWidth (2);
				RenderSphereRing (p, tc, nQuads, 0, GL_LINE_STRIP);
				glLineWidth (1);
				}
#endif
			}
		}
	}
ogl.DisableClientStates (bTextured, 0, 0, GL_TEXTURE0);
OglCullFace (0);
}

// -----------------------------------------------------------------------------

#else //!RINGED_SPHERE

int32_t CSphere::RenderTesselated (CFloatVector *vPosP, float xScale, float yScale, float zScale,
										 float red, float green, float blue, float alpha, CBitmap *bmP)
{
	int32_t			i, j, nFaces = m_nFaces;
	CFloatVector *ps,
					*sphereP = m_sphere,
					*rotSphereP = new CFloatVector [nFaces * (m_nFaceNodes + 1)];

if (!rotSphereP)
	return -1;
#	if 1
sphereP = reinterpret_cast<CFloatVector*> (Rotate (rotSphereP, vPosP, xScale, yScale, zScale));
#	else
sphereP = reinterpret_cast<CFloatVector*> (Sort (Rotate (rotSphereP, vPosP, nFaces, xScale, yScale, zScale), 0, nFaces - 1));
#	endif
if (m_nFaceNodes == 3) {
	glBegin (GL_LINES);
	for (j = nFaces, ps = sphereP; j; j--, ps++)
		for (i = 0; i < 3; i++, ps++)
			glVertex3fv (reinterpret_cast<GLfloat*> (ps));
	glEnd ();
	if (bmP)
		glColor4f (red, green, blue, 1.0f);
	else
		glColor4f (red, green, blue, alpha);
	glBegin (GL_TRIANGLES);
	for (j = nFaces, ps = sphereP; j; j--, ps++)
		for (i = 0; i < 3; i++, ps++) {
			glVertex3fv (reinterpret_cast<GLfloat*> (ps));
			}
	glEnd ();
	}
else {
	glBegin (GL_LINES);
	for (j = nFaces, ps = sphereP; j; j--, ps++)
		for (i = 0; i < 4; i++, ps++) {
			glVertex3fv (reinterpret_cast<GLfloat*> (ps));
			}
	glEnd ();
	if (bTextured)
		glColor4f (fScale, fScale, fScale, 1.0f);
	else
		glColor4f (red, green, blue, alpha);
	glBegin (GL_QUADS);
	for (j = nFaces, ps = sphereP; j; j--, ps++)
		for (i = 0; i < 4; i++, ps++) {
			if (bTextured)
				glTexCoord2f (fTexCoord [i][0], fTexCoord [i][1]);
			glVertex3fv (reinterpret_cast<GLfloat*> (ps));
			}
	glEnd ();
	}
delete[] rotSphereP;
}

#endif

// -----------------------------------------------------------------------------

int32_t CSphere::Render (CObject* objP, CFloatVector *vPosP, float xScale, float yScale, float zScale,
								 float red, float green, float blue, float alpha, CBitmap *bmP, int32_t nTiles, char bAdditive)
{
	float	fScale = 1.0f;
	int32_t	bTextured = 0;
#if 0 //DBG
	int32_t	bEffect = 0;
#else
	int32_t	bAppearing = objP->Appearing ();
	int32_t	bEffect = (objP->info.nType == OBJ_PLAYER) || (objP->info.nType == OBJ_ROBOT);
	int32_t	bGlow = /*!bAppearing &&*/ (bAdditive != 0) && glowRenderer.Available (GLOW_SHIELDS);
#endif

CFixVector vPos;
PolyObjPos (objP, &vPos);
if (bAppearing) {
	float scale = objP->AppearanceScale ();
	red *= scale;
	green *= scale;
	blue *= scale;
	}
#if !RINGED_SPHERE
if (m_nFaceNodes == 3)
	bmP = NULL;
else
#endif
Pulsate ();
if (bGlow) {
	glowRenderer.Begin (GLOW_SHIELDS, 2, false, 0.85f);
	if (!glowRenderer.SetViewport (GLOW_SHIELDS, vPos, 4 * xScale / 3)) {
		glowRenderer.Done (GLOW_SHIELDS);
		ogl.SetDepthMode (GL_LEQUAL);
		return 0;
		}
	ogl.SetBlendMode (OGL_BLEND_REPLACE);
	ogl.SetDepthMode (GL_ALWAYS);
	}
else {
	ogl.SetBlendMode (bAdditive);
	ogl.SetDepthMode (GL_LEQUAL);
	}
#if RINGED_SPHERE
ogl.SetTransform (1);
if (bAppearing) {
	UnloadSphereShader ();
	float scale = objP->AppearanceScale ();
	scale = Min (1.0f, (float) pow (1.0f - scale, 0.25f));
#if 1
	xScale *= scale;
	yScale *= scale;
	zScale *= scale;
#endif
	}
else if (!bEffect)
	UnloadSphereShader ();
else if (gameOpts->render.bUseShaders && ogl.m_features.bShaders.Available ()) {
	if (!SetupSphereShader (objP, alpha)) {
		if (bGlow)
			glowRenderer.Done (GLOW_SHIELDS);
		return 0;
		}
	}

bTextured = InitSurface (red, green, blue, bEffect ? 1.0f : alpha, bmP, fScale);

//ogl.SetupTransform (0);
tObjTransformation *posP = OBJPOS (objP);
transformation.Begin (vPos, posP->mOrient);
RenderRings (xScale, 32, red, green, blue, alpha, bTextured, nTiles);
transformation.End ();
//ogl.ResetTransform (0);
ogl.SetTransform (0);
if (bGlow) 
#if 0
	glowRenderer.Done (GLOW_SHIELDS);
#else
	glowRenderer.End ();
#endif
#else
RenderTesselated (vPosP, xScale, yScale, zScale, red, green, blue, alpha, bmP);
#endif //RINGED_SPHERE
ogl.SetDepthWrite (true);
//ogl.SetDepthMode (GL_LEQUAL);
return 1;
}

// -----------------------------------------------------------------------------

void CSphere::Destroy (void)
{
m_vertices.Destroy ();
Init ();
}

// -----------------------------------------------------------------------------

void CSphere::SetPulse (CPulseData* pulseP)
{
m_pulseP = pulseP;
}

// -----------------------------------------------------------------------------

void CSphere::SetupPulse (float fSpeed, float fMin)
{
m_pulse.fScale =
m_pulse.fMin = fMin;
m_pulse.fSpeed =
m_pulse.fDir = fSpeed;
}

// -----------------------------------------------------------------------------

void SetupSpherePulse (CPulseData *pulseP, float fSpeed, float fMin)
{
pulseP->fScale =
pulseP->fMin = fMin;
pulseP->fSpeed =
pulseP->fDir = fSpeed;
}

// -----------------------------------------------------------------------------

int32_t CreateShieldSphere (void)
{
if (!shield.Load ())
	return 0;
#if RINGED_SPHERE
//gameData.render.shield.Destroy ();
gameData.render.shield.Create (32, 1);
#else
if (gameData.render.shield.nTessDepth != gameOpts->render.textures.nQuality + 2) {
	gameData.render.shield.Destroy ();
	gameData.render.shield.nTessDepth = gameOpts->render.textures.nQuality + 2;
	}
if (!gameData.render.shield.sphereP)
	gameData.render.shield.nFaces = gameData.render.shield.Create ();
#endif
gameData.render.shield.SetupPulse (0.02f, 0.5f);
gameData.render.shield.SetPulse (gameData.render.shield.Pulse ());
return 1;
}

// -----------------------------------------------------------------------------

int32_t DrawShieldSphere (CObject *objP, float red, float green, float blue, float alpha, char bAdditive, fix nSize)
{
if (!CreateShieldSphere ())
	return 0;
#if !RINGED_SPHERE
if (gameData.render.shield.nFaces > 0)
#endif
 {
	if (!nSize) {
#if 0 //DBG
		nSize = objP->info.xSize;
#else
		if (objP->rType.polyObjInfo.nModel < 0) 
			nSize = objP->info.xSize;
		else {
			CPolyModel* modelP = GetPolyModel (objP, NULL, objP->ModelId (), 0);
			nSize = modelP ? modelP->Rad () : objP->info.xSize;
			}
#endif
		}
	float r = X2F (nSize);
	if (gameStates.render.nType == RENDER_TYPE_TRANSPARENCY)
		gameData.render.shield.Render (objP, NULL, r, r, r, red, green, blue, alpha, shield.Bitmap (), 1, bAdditive);
	else
		transparencyRenderer.AddSphere (riSphereShield, red, green, blue, alpha, objP, bAdditive, nSize);
	}
return 1;
}

// -----------------------------------------------------------------------------

void DrawMonsterball (CObject *objP, float red, float green, float blue, float alpha)
{
#if !RINGED_SPHERE
if (!gameData.render.monsterball.sphereP) {
	gameData.render.monsterball.nTessDepth = 3;
	gameData.render.monsterball.nFaces = gameData.render.monsterball.Create ();
	}
if (gameData.render.monsterball.nFaces > 0)
#endif
	{
	if (gameStates.render.nType != RENDER_TYPE_TRANSPARENCY)
		transparencyRenderer.AddSphere (riMonsterball, red, green, blue, alpha, objP, 0);
	else {
		float r = X2F (objP->info.xSize);
		CFloatVector p;
		p.SetZero ();
		gameData.render.monsterball.Render (objP, &p, r, r, r, red, green, blue, gameData.hoard.monsterball.bm.Buffer () ? 1.0f : alpha,
														&gameData.hoard.monsterball.bm, 4, 0);
		ogl.ResetTransform (1);
		ogl.SetTransform (0);
		}
	}
}

// -----------------------------------------------------------------------------

void DestroyShieldSphere (void)
{
gameData.render.shield.Destroy ();
}

// -----------------------------------------------------------------------------

void DestroyMonsterball (void)
{
gameData.render.monsterball.Destroy ();
}

// -----------------------------------------------------------------------------

void InitSpheres (void)
{
PrintLog (1, "creating spheres\n");
CreateShieldSphere ();
PrintLog (-1);
}

// -----------------------------------------------------------------------------
