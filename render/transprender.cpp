		/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTTIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DETIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEATING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYTIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL TIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifndef _WIN32
#	include <unistd.h>
#endif

#include "descent.h"
#include "u_mem.h"
#include "error.h"
#include "ogl_lib.h"
#include "ogl_shader.h"
#include "ogl_fastrender.h"
#include "rendermine.h"
#include "gameseg.h"
#include "objrender.h"
#include "lightmap.h"
#include "lightning.h"
#include "sphere.h"
#include "glare.h"
#include "automap.h"
#include "transprender.h"
#include "renderthreads.h"

#define RENDER_TRANSPARENCY 1
#define RENDER_TRANSP_DECALS 1

#define TI_SPLIT_POLYS 0
#define TI_POLY_OFFSET 0
#define TI_POLY_CENTER 1

#if DBG
int nDbgPoly = -1, nDbgItem = -1;
#endif

CTransparencyRenderer transparencyRenderer;

#define LAZY_RESET 1

//------------------------------------------------------------------------------

static tTexCoord2f tcDefault [4] = {{{0,0}},{{1,0}},{{1,1}},{{0,1}}};

inline int CTransparencyRenderer::AllocBuffers (void)
{
if (m_data.depthBuffer.Buffer ())
	return 1;
if (!m_data.depthBuffer.Create (ITEM_DEPTHBUFFER_SIZE))
	return 0;
if (!m_data.itemLists.Create (ITEM_BUFFER_SIZE)) {
	m_data.depthBuffer.Destroy ();
	return 0;
	}
m_data.nFreeItems = 0;
ResetBuffers ();
return 1;
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::FreeBuffers (void)
{
m_data.itemLists.Destroy ();
m_data.depthBuffer.Destroy ();
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::ResetBuffers (void)
{
m_data.depthBuffer.Clear ();
memset (m_data.itemLists.Buffer (), 0, (ITEM_BUFFER_SIZE - m_data.nFreeItems) * sizeof (struct tTranspItem));
m_data.nFreeItems = ITEM_BUFFER_SIZE;
}


//------------------------------------------------------------------------------

void CTransparencyRenderer::InitBuffer (int zMin, int zMax)
{
#if LAZY_RESET
if (!gameOpts->render.n3DGlasses || (ogl.StereoSeparation () < 0)) 
#endif
	{
	m_data.zMin = 0;
	m_data.zMax = zMax - m_data.zMin;
	m_data.zScale = (double) (ITEM_DEPTHBUFFER_SIZE - 1) / (double) (zMax - m_data.zMin);
	if (m_data.zScale < 0)
		m_data.zScale = 1;
	else if (m_data.zScale > 1)
		m_data.zScale = 1;
	}
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::Add (tTranspItemType nType, void *itemData, int itemSize, CFixVector vPos, int nOffset, bool bClamp)
{
#if LAZY_RESET
if (gameOpts->render.n3DGlasses && (ogl.StereoSeparation () >= 0))
	return 0;
#endif
#if RENDER_TRANSPARENCY
	tTranspItem *ph, *pi, *pj, **pd;
	int			nDepth =	CFixVector::Dist (vPos, OBJPOS (gameData.objs.viewerP)->vPos) + I2X (nOffset);

if (nDepth < m_data.zMin) {
	if (!bClamp)
		return m_data.nFreeItems;
	nDepth = m_data.zMin;
	}
else if (nDepth > m_data.zMax) {
	if (!bClamp)
		return m_data.nFreeItems;
	nDepth = m_data.zMax;
	}

AllocBuffers ();
if (!m_data.nFreeItems)
	return 0;
nOffset = (int) ((double) (nDepth - m_data.zMin) * m_data.zScale);
if (nOffset >= ITEM_DEPTHBUFFER_SIZE)
	return 0;
pd = m_data.depthBuffer + nOffset;
// find the first particle to insert the new one *before* and place in pj; pi will be it's predecessor (NULL if to insert at list start)
ph = m_data.itemLists + --m_data.nFreeItems;
ph->nItem = m_data.nItems++;
ph->nType = nType;
ph->bRendered = 0;
ph->parentP = NULL;
ph->z = nDepth;
ph->bValid = true;
memcpy (&ph->item, itemData, itemSize);
for (pi = NULL, pj = *pd; pj && ((pj->z < nDepth) || ((pj->z == nDepth) && (pj->nType < nType))); pj = pj->pNextItem) {
#if DBG
	if (pj == pi)
		break; // loop
#endif
	pi = pj;
	}
if (pi) {
	ph->pNextItem = pi->pNextItem;
	pi->pNextItem = ph;
	}
else {
	ph->pNextItem = *pd;
	*pd = ph;
	}
if (m_data.nMinOffs > nOffset)
	m_data.nMinOffs = nOffset;
if (m_data.nMaxOffs < nOffset)
	m_data.nMaxOffs = nOffset;
return m_data.nFreeItems;
#else
return 0;
#endif
}

//------------------------------------------------------------------------------

#if TI_SPLIT_POLYS

int CTransparencyRenderer::SplitPoly (tTranspPoly *item, int nDepth)
{
	tTranspPoly		split [2];
	CFloatVector		vSplit;
	tRgbaColorf	color;
	int			i, l, i0, i1, i2, i3, nMinLen = 0x7fff, nMaxLen = 0;
	float			z, zMin, zMax, *c, *c0, *c1;

split [0] = split [1] = *item;
for (i = i0 = 0; i < split [0].nVertices; i++) {
	l = split [0].sideLength [i];
	if (nMaxLen < l) {
		nMaxLen = l;
		i0 = i;
		}
	if (nMinLen > l)
		nMinLen = l;
	}
if ((nDepth > 1) || !nMaxLen || (nMaxLen < 10) || ((nMaxLen <= 30) && ((split [0].nVertices == 3) || (nMaxLen <= nMinLen / 2 * 3)))) {
	for (i = 0, zMax = 0, zMin = 1e30f; i < split [0].nVertices; i++) {
		z = split [0].vertices [i][Z];
		if (zMax < z)
			zMax = z;
		if (zMin > z)
			zMin = z;
		}
#if TI_POLY_CENTER
	return Add (item->bmP ? riTexPoly : riFlatPoly, item, sizeof (*item), F2X (zMax), F2X ((zMax + zMin) / 2));
#else
	return Add (item->bmP ? riTexPoly : riFlatPoly, item, sizeof (*item), F2X (zMax), F2X (zMin));
#endif
	}
if (split [0].nVertices == 3) {
	i1 = (i0 + 1) % 3;
	vSplit = CFloatVector::Avg(split [0].vertices [i0], split [0].vertices [i1]);
	split [0].vertices [i0] =
	split [1].vertices [i1] = vSplit;
	split [0].sideLength [i0] =
	split [1].sideLength [i0] = nMaxLen / 2;
	if (split [0].bmP) {
		split [0].texCoord [i0].v.u =
		split [1].texCoord [i1].v.u = (split [0].texCoord [i1].v.u + split [0].texCoord [i0].v.u) / 2;
		split [0].texCoord [i0].v.v =
		split [1].texCoord [i1].v.v = (split [0].texCoord [i1].v.v + split [0].texCoord [i0].v.v) / 2;
		}
	if (split [0].nColors == 3) {
		for (i = 4, c = reinterpret_cast<float*> (&color, c0 = reinterpret_cast<float*> (split [0].color + i0), c1 = reinterpret_cast<float*> (split [0].color + i1); i; i--)
			*c++ = (*c0++ + *c1++) / 2;
		split [0].color [i0] =
		split [1].color [i1] = color;
		}
	}
else {
	i1 = (i0 + 1) % 4;
	i2 = (i0 + 2) % 4;
	i3 = (i1 + 2) % 4;
	vSplit = CFloatVector::Avg(split [0].vertices [i0], split [0].vertices [i1]);
	split [0].vertices [i1] =
	split [1].vertices [i0] = vSplit;
	vSplit = CFloatVector::Avg(split [0].vertices [i2], split [0].vertices [i3]);
	split [0].vertices [i2] =
	split [1].vertices [i3] = vSplit;
	if (split [0].bmP) {
		split [0].texCoord [i1].v.u =
		split [1].texCoord [i0].v.u = (split [0].texCoord [i1].v.u + split [0].texCoord [i0].v.u) / 2;
		split [0].texCoord [i1].v.v =
		split [1].texCoord [i0].v.v = (split [0].texCoord [i1].v.v + split [0].texCoord [i0].v.v) / 2;
		split [0].texCoord [i2].v.u =
		split [1].texCoord [i3].v.u = (split [0].texCoord [i3].v.u + split [0].texCoord [i2].v.u) / 2;
		split [0].texCoord [i2].v.v =
		split [1].texCoord [i3].v.v = (split [0].texCoord [i3].v.v + split [0].texCoord [i2].v.v) / 2;
		}
	if (split [0].nColors == 4) {
		for (i = 4, c = reinterpret_cast<float*> (&color, c0 = reinterpret_cast<float*> (split [0].color + i0), c1 = reinterpret_cast<float*> (split [0].color + i1); i; i--)
			*c++ = (*c0++ + *c1++) / 2;
		split [0].color [i1] =
		split [1].color [i0] = color;
		for (i = 4, c = reinterpret_cast<float*> (&color, c0 = reinterpret_cast<float*> (split [0].color + i2), c1 = reinterpret_cast<float*> (split [0].color + i3); i; i--)
			*c++ = (*c0++ + *c1++) / 2;
		split [0].color [i2] =
		split [1].color [i3] = color;
		}
	split [0].sideLength [i0] =
	split [1].sideLength [i0] =
	split [0].sideLength [i2] =
	split [1].sideLength [i2] = nMaxLen / 2;
	}
return SplitPoly (split, nDepth + 1) && SplitPoly (split + 1, nDepth + 1);
}

#endif

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddObject (CObject *objP)
{
	tTranspObject	item;
//	CFixVector		vPos;

if (objP->info.nType == 255)
	return 0;
item.objP = objP;
item.vScale = gameData.models.vScale;
//transformation.Transform (vPos, OBJPOS (objP)->vPos, 0);
return Add (tiObject, &item, sizeof (item), OBJPOS (objP)->vPos);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddPoly (CSegFace *faceP, tFaceTriangle *triP, CBitmap *bmP,
												CFloatVector *vertices, char nVertices, tTexCoord2f *texCoord, tRgbaColorf *color,
												tFaceColor *altColor, char nColors, char bDepthMask, int nPrimitive, int nWrap, int bAdditive,
												short nSegment)
{
	tTranspPoly	item;
	int			i;
	float			s = gameStates.render.grAlpha;
	//fix			zCenter;

item.faceP = faceP;
item.triP = triP;
item.bmP = bmP;
item.nVertices = nVertices;
item.nPrimitive = nPrimitive;
item.nWrap = nWrap;
item.bDepthMask = bDepthMask;
item.bAdditive = bAdditive;
item.nSegment = nSegment;
memcpy (item.texCoord, texCoord ? texCoord : tcDefault, nVertices * sizeof (tTexCoord2f));
if ((item.nColors = nColors)) {
	if (nColors < nVertices)
		nColors = 1;
	if (color) {
		memcpy (item.color, color, nColors * sizeof (tRgbaColorf));
		for (i = 0; i < nColors; i++)
			if (bAdditive)
				item.color [i].alpha = 1;
			else
				item.color [i].alpha *= s;
		}
	else if (altColor) {
		for (i = 0; i < nColors; i++) {
			item.color [i] = altColor [i].color;
			if (bAdditive)
				item.color [i].alpha = 1;
			else
				item.color [i].alpha *= s;
			}
		}
	else
		item.nColors = 0;
	}
memcpy (item.vertices, vertices, nVertices * sizeof (CFloatVector));
#if TI_SPLIT_POLYS
if (bDepthMask && m_data.bSplitPolys) {
	for (i = 0; i < nVertices; i++)
		item.sideLength [i] = (short) (CFloatVector::Dist(vertices [i], vertices [(i + 1) % nVertices]) + 0.5f);
	return SplitPoly (&item, 0);
	}
else
#endif
	{
#if 0
	CFloatVector v = item.vertices [0];
	for (i = 1; i < item.nVertices; i++) 
		v += item.vertices [i];
	v /= item.nVertices;
	CFixVector vPos;
	vPos.Assign (v);
	fix d = CFixVector::Dist (vPos, OBJPOS (gameData.objs.viewerP)->vPos);
	return Add (item.bmP ? tiTexPoly : tiFlatPoly, &item, sizeof (item), vPos, true);
#else
	return Add (item.bmP ? tiTexPoly : tiFlatPoly, &item, sizeof (item), 
					SEGMENTS [faceP->m_info.nSegment].Side (faceP->m_info.nSide)->Center (), 0, true);
#endif
	}
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddFaceTris (CSegFace *faceP)
{
	tFaceTriangle*	triP;
	CFloatVector	vertices [3];
	int				h, i, j, bAdditive = FaceIsAdditive (faceP);
	CBitmap*			bmP = faceP->m_info.bTextured ? /*faceP->bmTop ? faceP->bmTop :*/ faceP->bmBot : NULL;

if (bmP)
	bmP = bmP->Override (-1);
#if DBG
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	faceP = faceP;
#endif
triP = FACES.tris + faceP->m_info.nTriIndex;
for (h = faceP->m_info.nTris; h; h--, triP++) {
	for (i = 0, j = triP->nIndex; i < 3; i++, j++) {
#if 1
		transformation.Transform (vertices [i], *(reinterpret_cast<CFloatVector*> (FACES.vertices + j)), 0);
#else
		if (automap.Display ())
			transformation.Transform (vertices + i, gameData.segs.fVertices + triP->index [i], 0);
		else
			vertices [i].Assign (gameData.segs.points [triP->index [i]].p3_vec);
#endif
		}
	if (!AddPoly (faceP, triP, bmP, vertices, 3, FACES.texCoord + triP->nIndex,
					  FACES.color + triP->nIndex,
					  NULL, 3, (bmP != NULL) && !bAdditive, GL_TRIANGLES, GL_REPEAT,
					  bAdditive, faceP->m_info.nSegment))
		return 0;
	}
return 1;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddFaceQuads (CSegFace *faceP)
{
	CFloatVector	vertices [4];
	int				i, j, bAdditive = FaceIsAdditive (faceP);
	CBitmap*			bmP = faceP->m_info.bTextured ? /*faceP->bmTop ? faceP->bmTop :*/ faceP->bmBot : NULL;

if (bmP)
	bmP = bmP->Override (-1);
#if DBG
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	faceP = faceP;
#endif
for (i = 0, j = faceP->m_info.nIndex; i < 4; i++, j++) {
#if 1
	transformation.Transform (vertices [i], *(reinterpret_cast<CFloatVector*> (FACES.vertices + j)), 0);
#else
	if (automap.Display ())
		transformation.Transform(vertices [i], gameData.segs.fVertices [faceP->m_info.index [i]], 0);
	else
		vertices [i].Assign (gameData.segs.points [faceP->m_info.index [i]].p3_vec);
#endif
	}
return AddPoly (faceP, NULL, bmP,
					 vertices, 4, FACES.texCoord + faceP->m_info.nIndex,
					 FACES.color + faceP->m_info.nIndex,
					 NULL, 4, (bmP != NULL) && !bAdditive, GL_TRIANGLE_FAN, GL_REPEAT,
					 bAdditive, faceP->m_info.nSegment) > 0;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddSprite (CBitmap *bmP, const CFixVector& position, tRgbaColorf *color,
												  int nWidth, int nHeight, char nFrame, char bAdditive, float fSoftRad)
{
	tTranspSprite	item;
	CFixVector		vPos;

item.bmP = bmP;
if ((item.bColor = (color != NULL)))
	item.color = *color;
item.nWidth = nWidth;
item.nHeight = nHeight;
item.nFrame = nFrame;
item.bAdditive = bAdditive;
item.fSoftRad = fSoftRad;
//transformation.Transform (vPos, position, 0);
item.position.Assign (vPos);
return Add (tiSprite, &item, sizeof (item), vPos);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddSpark (const CFixVector& position, char nType, int nSize, char nFrame)
{
	tTranspSpark	item;
//	CFixVector		vPos;

item.nSize = nSize;
item.nFrame = nFrame;
item.nType = nType;
item.position.Assign (position);
//transformation.Transform (vPos, position, 0);
return Add (tiSpark, &item, sizeof (item), position);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddSphere (tTranspSphereType nType, float red, float green, float blue, float alpha, CObject *objP, fix nSize)
{
	tTranspSphere	item;
	//CFixVector		vPos;

item.nType = nType;
item.color.red = red;
item.color.green = green;
item.color.blue = blue;
item.color.alpha = alpha;
item.nSize = nSize;
item.objP = objP;
//transformation.Transform (vPos, objP->info.position.vPos, 0);
return Add (tiSphere, &item, sizeof (item), objP->info.position.vPos);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddParticle (CParticle *particle, float fBrightness, int nThread)
{
	tTranspParticle	item;
//	fix					z;

if ((particle->m_nType < 0) || (particle->m_nType >= PARTICLE_TYPES))
	return 0;
item.particle = particle;
item.fBrightness = fBrightness;
//particle->Transform (gameStates.render.bPerPixelLighting == 2);
return Add (tiParticle, &item, sizeof (item), particle->m_vPos, (gameOpts->render.effects.bSoftParticles & 1) ? -10 : 0);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddLightning (CLightning *lightningP, short nDepth)
{
	tTranspLightning	item;
	//CFixVector			vPos;

item.lightning = lightningP;
item.nDepth = nDepth;
#if 0
transformation.Transform (vPos, lightningP->m_vPos, 0);
z = vPos [Z];
transformation.Transform (vPos, lightningP->m_vEnd, 0);
if (z < vPos [Z])
	z = vPos [Z];
#endif
fix d1 = CFixVector::Dist (lightningP->m_vPos, OBJPOS (gameData.objs.viewerP)->vPos);
fix d2 = CFixVector::Dist (lightningP->m_vEnd, OBJPOS (gameData.objs.viewerP)->vPos);
if (d1 > d2)
	::Swap (d1, d2);
if (d1 > m_data.zMax)
	return 0;
if (d2 < m_data.zMin)
	return 0;
if (!Add (tiLightning, &item, sizeof (item), CFixVector::Avg (lightningP->m_vPos, lightningP->m_vEnd), true))
	return 0;
return 1;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddLightTrail (CBitmap *bmP, CFloatVector *vThruster, tTexCoord2f *tcThruster, CFloatVector *vFlame, tTexCoord2f *tcFlame, tRgbaColorf *colorP)
{
	tTranspLightTrail	item;
	int					i, j, iMin = 0;
	CFixVector			v;
	CFloatVector		vViewer;
	float					d, dMin = 1e30f;

item.bmP = bmP;
memcpy (item.vertices, vThruster, 4 * sizeof (CFloatVector));
memcpy (item.texCoord, tcThruster, 4 * sizeof (tTexCoord2f));
item.color = *colorP;
if ((item.bTrail = (vFlame != NULL))) {
	memcpy (item.vertices + 4, vFlame, 3 * sizeof (CFloatVector));
	memcpy (item.texCoord + 4, tcFlame, 3 * sizeof (tTexCoord2f));
	j = 7;
	}
else
	j = 4;
transformation.Transform (v, OBJPOS (gameData.objs.viewerP)->vPos);
vViewer.Assign (v);
for (i = 0; i < j; i++) {
	d = CFloatVector::Dist (item.vertices [i], vViewer);
	if (dMin > d) {
		dMin = d;
		iMin = i;
		}
	}
v.Assign (item.vertices [iMin]);
return Add (tiThruster, &item, sizeof (item), v);
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::DisableTMU (int nTMU, char bFull)
{
ogl.DisableClientStates (1, 1, 1, nTMU);
if (bFull) {
	ogl.BindTexture (0);
	ogl.SetTextureUsage (false);
	}
}

//------------------------------------------------------------------------------

inline void CTransparencyRenderer::ResetBitmaps (void)
{
m_data.bmP [0] =
m_data.bmP [1] =
m_data.bmP [2] = NULL;
m_data.bDecal = 0;
m_data.bTextured = 0;
m_data.nFrame = -1;
m_data.bUseLightmaps = 0;
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::SetDecalState (char bDecal, char bTexCoord, char bColor, char bUseLightmaps)
{
#if RENDER_TRANSP_DECALS
if (m_data.bDecal != bDecal) {
	if (bDecal) {
		if (bDecal == 2)
			ogl.EnableClientStates (bTexCoord, 0, 0, GL_TEXTURE2 + bUseLightmaps);	// mask
		ogl.EnableClientStates (bTexCoord, bColor, 0, GL_TEXTURE1 + bUseLightmaps);	// decal
		m_data.bDecal = bDecal;
		}
	else {
		if (m_data.bDecal == 2) {
			DisableTMU (GL_TEXTURE2 + bUseLightmaps, 1);
			m_data.bmP [2] = NULL;
			}
		DisableTMU (GL_TEXTURE1 + bUseLightmaps, 1);
		m_data.bmP [1] = NULL;
		m_data.bDecal = 0;
		}
	}
#endif
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::SetClientState (char bClientState, char bTexCoord, char bColor, char bUseLightmaps, char bDecal)
{
PROF_START
if (m_data.bUseLightmaps != bUseLightmaps) {
	ogl.ResetClientStates ();
	ResetBitmaps ();
	ogl.EnableClientStates (1, 1, bUseLightmaps, GL_TEXTURE0);
	ogl.SetTextureUsage (true);
	if ((m_data.bUseLightmaps = bUseLightmaps))
		ogl.EnableClientStates (1, 1, 0, GL_TEXTURE1);
	}

if (bClientState) {
	SetDecalState (bDecal, bTexCoord, bColor, bUseLightmaps);
	ogl.EnableClientStates (bTexCoord, bColor, !bUseLightmaps, GL_TEXTURE0 + bUseLightmaps);
	}
else {
	SetDecalState (0, bTexCoord, bColor, bUseLightmaps);
	DisableTMU (GL_TEXTURE0 + bUseLightmaps, 0);
	}
ogl.SelectTMU (GL_TEXTURE0);
PROF_END(ptRenderStates)
return 1;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::LoadImage (CBitmap *bmP, char nColors, char nFrame, int nWrap,
												  int bClientState, int nTransp, int bShader, int bUseLightmaps,
												  int bHaveDecal, int bDecal)
{
if (bmP) {
	if (bDecal || SetClientState (bClientState, 1, nColors > 1, bUseLightmaps, bHaveDecal) || (m_data.bTextured < 1)) {
		ogl.SelectTMU (GL_TEXTURE0 + bUseLightmaps + bDecal, true);
		m_data.bTextured = 1;
		}
	ogl.SetTextureUsage (true);
	if (bDecal == 1)
		bmP = bmP->Override (-1);
	if ((bmP != m_data.bmP [bDecal]) || (nFrame != m_data.nFrame) || (nWrap != m_data.nWrap)) {
		gameData.render.nStateChanges++;
		if (bmP) {
			if (bmP->Bind (1)) {
				m_data.bmP [bDecal] = NULL;
				return 0;
				}
			if (bDecal != 2)
				bmP = bmP->Override (nFrame);
			bmP->Texture ()->Wrap (nWrap);
			m_data.nWrap = nWrap;
			m_data.nFrame = nFrame;
			}
		else
			ogl.BindTexture (0);
		m_data.bmP [bDecal] = bmP;
		}
	}
else if (SetClientState (bClientState, 0, /*!m_data.bLightmaps &&*/ (nColors > 1), bUseLightmaps, 0) || m_data.bTextured) {
	if (m_data.bTextured) {
		ogl.BindTexture (0);
		ogl.SetTextureUsage (false);
		ResetBitmaps ();
		}
	}
if (!bShader)
	shaderManager.Deploy (-1);
return 1;
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::SetRenderPointers (int nTMU, int nIndex, int bDecal)
{
#if DBG
if (nIndex + 3 > int (FACES.vertices.Length ()))
	return;
#endif
ogl.SelectTMU (nTMU, true);
if (m_data.bTextured)
	OglTexCoordPointer (2, GL_FLOAT, 0, bDecal ? FACES.ovlTexCoord + nIndex : FACES.texCoord + nIndex);
OglVertexPointer (3, GL_FLOAT, 0, FACES.vertices + nIndex);
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderPoly (tTranspPoly *item)
{
PROF_START
	CSegFace*		faceP;
	tFaceTriangle*	triP;
	CBitmap*			bmBot = item->bmP, *bmTop = NULL, *mask;
	int				nIndex, bLightmaps, bDecal, bSoftBlend = 0, bAdditive = 0;

#if TI_POLY_OFFSET
if (!bmBot) {
	glEnable (GL_POLYGON_OFFSET_FILL);
	glPolygonOffset (1,1);
	glPolygonMode (GL_FRONT, GL_FILL);
	}
#endif

faceP = item->faceP;
triP = item->triP;
bLightmaps = m_data.bLightmaps && (faceP != NULL);
#if DBG
if (!bLightmaps)
	bLightmaps = bLightmaps;
if (faceP) {
	if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide))) {
#if 0
		if (triP) {
			if ((triP->nIndex - faceP->m_info.nIndex) / 3 < 22)
				return;
			if ((triP->nIndex - faceP->m_info.nIndex) / 3 > 23)
				return;
			}
#endif
		nDbgSeg = nDbgSeg;
		}
	}
else {
	}
#endif
ogl.SetDepthWrite (item->bDepthMask != 0);
if (!faceP) {
	bmTop = mask = NULL;
	bDecal = 0;
	bSoftBlend = (gameOpts->render.effects.bSoftParticles & 1) != 0;
	}
else {
	if ((bmTop = faceP->bmTop))
		bmTop = bmTop->Override (-1);
	if (bmTop && !(bmTop->Flags () & (BM_FLAG_SUPER_TRANSPARENT | BM_FLAG_TRANSPARENT | BM_FLAG_SEE_THRU))) {
		bmBot = bmTop;
		bmTop = mask = NULL;
		bDecal = -1;
		faceP->m_info.nRenderType = gameStates.render.history.nType = 1;
		}
	else {
		bDecal = bmTop != NULL;
		mask = (bDecal && ((bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0) && gameStates.render.textures.bHaveMaskShader) ? bmTop->Mask () : NULL;
		}
	if (!bmTop && m_data.bmP [1]) {
		DisableTMU (GL_TEXTURE1 + bLightmaps, 1);
		m_data.bmP [1] = NULL;
		}
	if (!mask && m_data.bmP [2]) {
		DisableTMU (GL_TEXTURE2 + bLightmaps, 1);
		m_data.bmP [2] = NULL;
		}
	}

if (LoadImage (bmBot, (bLightmaps || gameStates.render.bFullBright) ? 0 : item->nColors, -1, item->nWrap, 1, 3, (faceP != NULL) || bSoftBlend, bLightmaps, mask ? 2 : bDecal > 0, 0) &&
	 ((bDecal < 1) || LoadImage (bmTop, 0, -1, item->nWrap, 1, 3, 1, bLightmaps, 0, 1)) &&
	 (!mask || LoadImage (mask, 0, -1, item->nWrap, 1, 3, 1, bLightmaps, 0, 2))) {
	nIndex = triP ? triP->nIndex : faceP ? faceP->m_info.nIndex : 0;
	if (triP || faceP) {
		if (!bLightmaps) {
			ogl.SelectTMU (GL_TEXTURE0, true);
			ogl.EnableClientState (GL_NORMAL_ARRAY);
			OglNormalPointer (GL_FLOAT, 0, FACES.normals + nIndex);
			}
		if (bDecal > 0) {
			SetRenderPointers (GL_TEXTURE1 + bLightmaps, nIndex, 1);
			if (mask)
				SetRenderPointers (GL_TEXTURE2 + bLightmaps, nIndex, 1);
			}
		SetRenderPointers (GL_TEXTURE0 + bLightmaps, nIndex, bDecal < 0);
		}
	else {
		ogl.SelectTMU (GL_TEXTURE0, true);
		if (m_data.bTextured)
			OglTexCoordPointer (2, GL_FLOAT, 0, item->texCoord);
		OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), item->vertices);
		}
	ogl.SetupTransform (faceP != NULL);
	if (gameStates.render.bFullBright)
		glColor3d (1, 1, 1);
	else if (item->nColors > 1) {
		ogl.SelectTMU (GL_TEXTURE0, true);
		ogl.EnableClientState (GL_COLOR_ARRAY);
		if (faceP || triP)
			OglColorPointer (4, GL_FLOAT, 0, FACES.color + nIndex);
		else
			OglColorPointer (4, GL_FLOAT, 0, item->color);
		if (bLightmaps) {
			OglTexCoordPointer (2, GL_FLOAT, 0, FACES.lMapTexCoord + nIndex);
			ogl.EnableClientState (GL_NORMAL_ARRAY);
			OglNormalPointer (GL_FLOAT, 0, FACES.normals + nIndex);
			OglVertexPointer (3, GL_FLOAT, 0, FACES.vertices + nIndex);
			}
		}
	else if (item->nColors == 1)
		glColor4fv (reinterpret_cast<GLfloat*> (item->color));
	else
		glColor3d (1, 1, 1);
	ogl.SetBlendMode (bAdditive = item->bAdditive);
#if DBG
	if (faceP && (faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
		nDbgSeg = nDbgSeg;
#endif
	if (faceP && gameStates.render.bPerPixelLighting) {
		if (!faceP->m_info.bColored) {
			G3SetupGrayScaleShader ((int) faceP->m_info.nRenderType, &faceP->m_info.color);
			OglDrawArrays (item->nPrimitive, 0, item->nVertices);
			}
		else {
			if (gameStates.render.bPerPixelLighting == 1) {
				G3SetupLightmapShader (faceP, 0, (int) faceP->m_info.nRenderType, false);
				OglDrawArrays (item->nPrimitive, 0, item->nVertices);
				}
			else {
				ogl.m_states.iLight = 0;
				lightManager.Index (0)[0].nActive = -1;
				for (;;) {
					G3SetupPerPixelShader (faceP, 0, faceP->m_info.nRenderType, false);
					OglDrawArrays (item->nPrimitive, 0, item->nVertices);
					if ((ogl.m_states.iLight >= ogl.m_states.nLights) ||
						 (ogl.m_states.iLight >= gameStates.render.nMaxLightsPerFace))
						break;
					bAdditive = 2;
					ogl.SetBlendMode (2);
					ogl.SetDepthWrite (false);
					}
				}
			if (gameStates.render.bHeadlights) {
#	if DBG
				if (faceP) {
					if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
						nDbgSeg = nDbgSeg;
					}
				ogl.SetBlending (true);
#	endif
				lightManager.Headlights ().SetupShader (m_data.bTextured, 1, m_data.bTextured ? NULL : &faceP->m_info.color);
				if (bAdditive != 2) {
					bAdditive = 2;
					ogl.SetBlendMode (2);
					ogl.SetDepthWrite (false);
					}
				OglDrawArrays (item->nPrimitive, 0, item->nVertices);
				}
			}
		}
	else {
		if (bAdditive && !automap.Display ()) {
			if (!(bSoftBlend && glareRenderer.LoadShader (5)))
				shaderManager.Deploy (-1);
			}
		else
			G3SetupShader (faceP, 0, mask != NULL, bDecal > 0, bmBot != NULL,
								(item->nSegment < 0) || !automap.Display () || automap.m_visited [0][item->nSegment],
								m_data.bTextured ? NULL : faceP ? &faceP->m_info.color : item->color);
		OglDrawArrays (item->nPrimitive, 0, item->nVertices);
		}
	ogl.ResetTransform (faceP != NULL);
	if (faceP)
		gameData.render.nTotalFaces++;
	}
#if GL_FALLBACK
else if (LoadImage (bmBot, item->nColors, -1, item->nWrap, 0, 3, 1, lightmapManager.HaveLightmaps () && (faceP != NULL), 0, 0)) {
	if (item->bAdditive == 1) {
		shaderManager.Deploy (-1);
		ogl.SetBlendMode (1);
		}
	else if (item->bAdditive == 2) {
		shaderManager.Deploy (-1);
		ogl.SetBlendMode (2);
		}
	else {
		ogl.SetBlendMode (0);
		G3SetupShader (faceP, 0, 0, 0, bmBot != NULL,
							(item->nSegment < 0) || !automap.Display () || automap.m_visited [0][item->nSegment],
							bmBot ? NULL : item->color);
		}
	int i, j = item->nVertices;
	glBegin (item->nPrimitive);
	if (item->nColors > 1) {
		if (bmBot) {
			for (i = 0; i < j; i++) {
				glColor4fv (reinterpret_cast<GLfloat*> (item->color + i));
				glTexCoord2fv (reinterpret_cast<GLfloat*> (item->texCoord + i));
				glVertex3fv (reinterpret_cast<GLfloat*> (item->vertices + i));
				}
			}
		else {
			for (i = 0; i < j; i++) {
				glColor4fv (reinterpret_cast<GLfloat*> (item->color + i));
				glVertex3fv (reinterpret_cast<GLfloat*> (item->vertices + i));
				}
			}
		}
	else {
		if (item->nColors)
			glColor4fv (reinterpret_cast<GLfloat*> (item->color));
		else
			glColor3d (1, 1, 1);
		if (bmBot) {
			for (i = 0; i < j; i++) {
				glTexCoord2fv (reinterpret_cast<GLfloat*> (item->texCoord + i));
				glVertex3fv (reinterpret_cast<GLfloat*> (item->vertices + i));
				}
			}
		else {
			for (i = 0; i < j; i++) {
				glVertex3fv (reinterpret_cast<GLfloat*> (item->vertices + i));
				}
			}
		}
	glEnd ();
	}
#endif
#if TI_POLY_OFFSET
if (!bmBot) {
	glPolygonOffset (0,0);
	glDisable (GL_POLYGON_OFFSET_FILL);
	}
#endif
PROF_END(ptRenderFaces)
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderObject (tTranspObject *item)
{
shaderManager.Deploy (-1);
ogl.ResetClientStates ();
gameData.models.vScale = item->vScale;
gameStates.render.bDepthSort = -1;
DrawPolygonObject (item->objP, 1);
gameStates.render.bDepthSort = 1;
gameData.models.vScale.SetZero ();
ogl.ResetClientStates ();
ResetBitmaps ();
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderSprite (tTranspSprite *item)
{
	int bSoftBlend = ((gameOpts->render.effects.bSoftParticles & 1) != 0) && (item->fSoftRad > 0);

if (LoadImage (item->bmP, item->bColor, item->nFrame, GL_CLAMP, 0, 1, bSoftBlend, 0, 0, 0)) {
	CFloatVector	vPosf = item->position;

	if (item->bColor)
		glColor4fv (reinterpret_cast<GLfloat*> (&item->color));
	else
		glColor3f (1, 1, 1);
	ogl.SetBlending (true);
	ogl.SetBlendMode (item->bAdditive);
	if (!(bSoftBlend && glareRenderer.LoadShader (item->fSoftRad, item->bAdditive != 0)))
		shaderManager.Deploy (-1);
	item->bmP->SetColor ();
	ogl.RenderQuad (item->bmP, vPosf, X2F (item->nWidth), X2F (item->nHeight), 3);
	}
}

//------------------------------------------------------------------------------

typedef struct tSparkVertex {
	CFloatVector3		vPos;
	tTexCoord2f	texCoord;
} tSparkVertex;

#define SPARK_BUF_SIZE	1000

typedef struct tSparkBuffer {
	int				nSparks;
	tSparkVertex	info [SPARK_BUF_SIZE * 4];
} tSparkBuffer;

tSparkBuffer sparkBuffer;

//------------------------------------------------------------------------------

void CTransparencyRenderer::FlushSparkBuffer (void)
{
	int bSoftBlend = (gameOpts->render.effects.bSoftParticles & 2) != 0;

if (sparkBuffer.nSparks && LoadImage (bmpSparks, 0, -1, GL_CLAMP, 1, 1, bSoftBlend, 0, 0, 0)) {
	ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
#if 0
	ogl.SetTextureUsage (true);
	bmpSparks->Texture ()->Bind ();
#endif
	if (!(bSoftBlend && glareRenderer.LoadShader (3, 1)))
		shaderManager.Deploy (-1);
	ogl.SetBlendMode (1);
	glColor3f (1, 1, 1);
	OglTexCoordPointer (2, GL_FLOAT, sizeof (tSparkVertex), &sparkBuffer.info [0].texCoord);
	OglVertexPointer (3, GL_FLOAT, sizeof (tSparkVertex), &sparkBuffer.info [0].vPos);
	ogl.SetTransform (1);
	ogl.SetupTransform (0);
	OglDrawArrays (GL_QUADS, 0, 4 * sparkBuffer.nSparks);
	ogl.ResetTransform (1);
	ogl.SetTransform (0);
	ogl.SetBlendMode (0);
	ogl.SetDepthTest (true);
	m_data.bClientColor = 0;
	sparkBuffer.nSparks = 0;
	}
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderSpark (tTranspSpark *item)
{
if (sparkBuffer.nSparks >= SPARK_BUF_SIZE)
	FlushSparkBuffer ();

	tSparkVertex	*infoP = sparkBuffer.info + 4 * sparkBuffer.nSparks;
	CFloatVector	vPos = item->position;
	float				nSize = X2F (item->nSize);
	float				nCol = (float) (item->nFrame / 8);
	float				nRow = (float) (item->nFrame % 8);

if (!item->nType)
	nCol += 4;
infoP->vPos [X] = vPos [X] - nSize;
infoP->vPos [Y] = vPos [Y] + nSize;
infoP->vPos [Z] = vPos [Z];
infoP->texCoord.v.u = nCol / 8.0f;
infoP->texCoord.v.v = (nRow + 1) / 8.0f;
infoP++;
infoP->vPos [X] = vPos [X] + nSize;
infoP->vPos [Y] = vPos [Y] + nSize;
infoP->vPos [Z] = vPos [Z];
infoP->texCoord.v.u = (nCol + 1) / 8.0f;
infoP->texCoord.v.v = (nRow + 1) / 8.0f;
infoP++;
infoP->vPos [X] = vPos [X] + nSize;
infoP->vPos [Y] = vPos [Y] - nSize;
infoP->vPos [Z] = vPos [Z];
infoP->texCoord.v.u = (nCol + 1) / 8.0f;
infoP->texCoord.v.v = nRow / 8.0f;
infoP++;
infoP->vPos [X] = vPos [X] - nSize;
infoP->vPos [Y] = vPos [Y] - nSize;
infoP->vPos [Z] = vPos [Z];
infoP->texCoord.v.u = nCol / 8.0f;
infoP->texCoord.v.v = nRow / 8.0f;
sparkBuffer.nSparks++;
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderSphere (tTranspSphere *item)
{
ogl.ResetClientStates ();
shaderManager.Deploy (-1);
gameStates.render.bDepthSort = -1;
if (item->nType == riSphereShield)
	DrawShieldSphere (item->objP, item->color.red, item->color.green, item->color.blue, item->color.alpha, item->nSize);
else if (item->nType == riMonsterball)
	DrawMonsterball (item->objP, item->color.red, item->color.green, item->color.blue, item->color.alpha);
gameStates.render.bDepthSort = 1;
ResetBitmaps ();
shaderManager.Deploy (-1);
ogl.ResetClientStates ();
ogl.SetTextureUsage (false);
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderBullet (CParticle *pParticle)
{
	CObject	o;

memset (&o, 0, sizeof (o));
o.info.nType = OBJ_POWERUP;
o.info.position.vPos = pParticle->m_vPos;
o.info.position.mOrient = pParticle->m_mOrient;
if (0 <= (o.info.nSegment = FindSegByPos (o.info.position.vPos, pParticle->m_nSegment, 0, 0))) {
	lightManager.Index (0)[0].nActive = 0;
	o.info.renderType = RT_POLYOBJ;
	o.rType.polyObjInfo.nModel = BULLET_MODEL;
	o.rType.polyObjInfo.nTexOverride = -1;
	DrawPolygonObject (&o, 1);
	gameData.models.vScale.SetZero ();
	ogl.ResetClientStates ();
	ResetBitmaps ();
	}
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderParticle (tTranspParticle *item)
{
if (item->particle->m_nType == BULLET_PARTICLES)
	CTransparencyRenderer::RenderBullet (item->particle);
else {
	int bSoftSmoke = (gameOpts->render.effects.bSoftParticles & 4) != 0;

#if 0
	if (!bSoftSmoke || (gameStates.render.history.nShader != 999))
		shaderManager.Deploy (-1);
#endif
	if (m_data.nPrevType != tiParticle) {
		ogl.SetTextureUsage (true);
		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		particleManager.SetLastType (-1);
		m_data.bTextured = 1;
		}
	if (item->particle->Render (item->fBrightness) < 0) {
		SetClientState (0, 0, 0, 0, 0);
		ResetBitmaps ();
		}
	}
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderLightning (tTranspLightning *item)
{
if (m_data.nPrevType != m_data.nCurType) {
	ogl.ResetClientStates ();
	ResetBitmaps ();
	shaderManager.Deploy (-1);
	}
gameStates.render.bDepthSort = -1;
item->lightning->Render (item->nDepth, 0);
gameStates.render.bDepthSort = 1;
ogl.ResetClientStates ();
ResetBitmaps ();
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderLightTrail (tTranspLightTrail *item)
{
ogl.SetDepthWrite (true);
ogl.SetFaceCulling (false);
ogl.SetBlendMode (1);
glColor4fv (reinterpret_cast<GLfloat*> (&item->color));
if (LoadImage (item->bmP, 1, -1, GL_CLAMP, 1, 1, 0, 0, 0, 0)) {
	OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), item->vertices);
	OglTexCoordPointer (2, GL_FLOAT, 0, item->texCoord);
	if (item->bTrail)
		OglDrawArrays (GL_TRIANGLES, 4, 3);
	OglDrawArrays (GL_QUADS, 0, 4);
	}
#if GL_FALLBACK
else
if (LoadImage (item->bmP, 0, -1, GL_CLAMP, 0, 1, 0, 0, 0, 0)) {
	int i;
	if (item->bTrail) {
		glBegin (GL_TRIANGLES);
		for (int i = 0; i < 3; i++) {
			glTexCoord2fv (reinterpret_cast<GLfloat*> (item->texCoord + 4 + i));
			glVertex3fv (reinterpret_cast<GLfloat*> (item->vertices + 4 + i));
			}
		glEnd ();
		}
	glBegin (GL_QUADS);
	for (i = 0; i < 4; i++) {
		glTexCoord2fv (reinterpret_cast<GLfloat*> (item->texCoord + i));
		glVertex3fv (reinterpret_cast<GLfloat*> (item->vertices + i));
		}
	glEnd ();
	}
#endif
ogl.SetFaceCulling (true);
ogl.SetBlendMode (0);
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::FlushParticleBuffer (int nType)
{
if (particleManager.BufPtr () && ((nType < 0) || ((nType != tiParticle) && (particleManager.LastType () >= 0)))) {
	if (sparkBuffer.nSparks)
		FlushSparkBuffer ();
	if (particleManager.FlushBuffer (-1.0f)) {
		if (nType < 0)
			particleManager.CloseBuffer ();
		ogl.SetDepthTest (true);
		ResetBitmaps ();
		particleManager.SetLastType (-1);
		m_data.bClientColor = 1;
		m_data.bUseLightmaps = 0;
		}
	}
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::FlushBuffers (int nType)
{
if ((nType != tiSpark) && (nType != tiParticle))
	FlushSparkBuffer ();
FlushParticleBuffer (nType);
}

//------------------------------------------------------------------------------

tTranspItem* CTransparencyRenderer::SetParent (int nChild, int nParent)
{
if (nChild < 0)
	return NULL;
tTranspItem	*childP = m_data.itemLists + nChild;
if (!childP->bValid)
	return NULL;
if (nParent < 0)
	return NULL;
tTranspItem	*parentP = m_data.itemLists + nParent;
if (!parentP->bValid)
	return NULL;
childP->parentP = parentP;
return parentP;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::RenderItem (struct tTranspItem *pl)
{
if (!pl->bRendered) {
	//pl->bRendered = true;
	m_data.nPrevType = m_data.nCurType;
	m_data.nCurType = pl->nType;
#if DBG
	if (gameOpts->render.debug.bTextures && gameOpts->render.debug.bWalls)
#endif
	try {
		FlushBuffers (m_data.nCurType);
		ogl.SetBlendMode (0);
		ogl.SetDepthWrite (false);
		ogl.SetDepthMode (GL_LEQUAL);
		ogl.SetDepthTest (true);
		if ((m_data.nCurType == tiTexPoly) || (m_data.nCurType == tiFlatPoly)) {
			RenderPoly (&pl->item.poly);
			}
		else if (m_data.nCurType == tiObject) {
			RenderObject (&pl->item.object);
			}
		else if (m_data.nCurType == tiSprite) {
			RenderSprite (&pl->item.sprite);
			}
		else if (m_data.nCurType == tiSpark) {
			RenderSpark (&pl->item.spark);
			}
		else if (m_data.nCurType == tiSphere) {
			RenderSphere (&pl->item.sphere);
			}
		else if (m_data.nCurType == tiParticle) {
			if (m_data.bHaveParticles)
				RenderParticle (&pl->item.particle);
			}
		else if (m_data.nCurType == tiLightning) {
			RenderLightning (&pl->item.lightning);
			}
		else if (m_data.nCurType == tiThruster) {
			RenderLightTrail (&pl->item.thruster);
			}
		}
	catch(...) {
		PrintLog ("invalid transparent render item (type: %d)\n", m_data.nCurType);
		}
	if (pl->parentP)
		RenderItem (pl->parentP);
	}
return m_data.nCurType;
}

//------------------------------------------------------------------------------

extern int bLog;

void CTransparencyRenderer::Render (void)
{
#if RENDER_TRANSPARENCY
	struct tTranspItem	**pd, *pl, *pn;
	int						nItems, nDepth, bStencil;
	bool						bReset = !LAZY_RESET || (ogl.StereoSeparation () >= 0);

if (!(m_data.depthBuffer.Buffer () && (m_data.nFreeItems < ITEM_BUFFER_SIZE))) {
	return;
	}
PROF_START
gameStates.render.nType = 5;
shaderManager.Deploy (-1);
bStencil = ogl.StencilOff ();
ogl.ResetClientStates ();
ResetBitmaps ();
m_data.bTextured = -1;
m_data.bUseLightmaps = 0;
m_data.bDecal = 0;
m_data.bLightmaps = lightmapManager.HaveLightmaps ();
m_data.bSplitPolys = (gameStates.render.bPerPixelLighting != 2) && (gameStates.render.bSplitPolys > 0);
m_data.nWrap = 0;
m_data.nFrame = -1;
m_data.bmP [0] =
m_data.bmP [1] = NULL;
sparkBuffer.nSparks = 0;
ogl.DisableLighting ();
ogl.ResetClientStates ();
shaderManager.Deploy (-1);
pl = &m_data.itemLists [ITEM_BUFFER_SIZE - 1];
m_data.bHaveParticles = particleImageManager.LoadAll ();
ogl.SetBlendMode (0);
ogl.SetDepthMode (GL_LEQUAL);
ogl.SetFaceCulling (true);
particleManager.BeginRender (-1, 1);
m_data.nCurType = -1;
for (pd = m_data.depthBuffer + m_data.nMaxOffs, nItems = m_data.nItems; (pd >= m_data.depthBuffer.Buffer ()) && nItems; pd--) {
	if ((pl = *pd)) {
		if (bReset)
			*pd = NULL;
		nDepth = 0;
		do {
#if DBG
			if (pl->nItem == nDbgItem)
				nDbgItem = nDbgItem;
#endif
			nItems--;
			RenderItem (pl);
			pn = pl->pNextItem;
			if (bReset)
				pl->pNextItem = NULL;
			pl = pn;
			nDepth++;
			} while (pl);
		}
	}
#if DBG
if (bReset) {
	pl = m_data.itemLists.Buffer ();
	for (int i = m_data.itemLists.Length (); i; i--, pl++)
		if (pl->pNextItem)
			pl->pNextItem = NULL;
	}
#endif
FlushBuffers (-1);
particleManager.EndRender ();
shaderManager.Deploy (-1);
ogl.ResetClientStates ();
ogl.SetTextureUsage (false);
ogl.SetBlendMode (0);
ogl.SetDepthMode (GL_LEQUAL);
ogl.SetDepthWrite (true);
ogl.StencilOn (bStencil);
if (bReset) {
	m_data.nMinOffs = ITEM_DEPTHBUFFER_SIZE;
	m_data.nMaxOffs = 0;
	m_data.nFreeItems = ITEM_BUFFER_SIZE;
	}
PROF_END(ptTranspPolys)
#endif
}

//------------------------------------------------------------------------------
//eof
