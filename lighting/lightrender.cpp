#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

#include <math.h>
#include <stdio.h>
#include <string.h>	// for memset ()

#include "descent.h"
#include "error.h"
#include "u_mem.h"
#include "fix.h"
#include "vecmat.h"
#include "network.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_color.h"
#include "ogl_shader.h"
#include "findpath.h"
#include "segmath.h"
#include "endlevel.h"
#include "renderthreads.h"
#include "light.h"
#include "lightmap.h"
#include "headlight.h"
#include "dynlight.h"

#define SORT_ACTIVE_LIGHTS 0

//------------------------------------------------------------------------------

void CLightManager::Transform (int bStatic, int bVariable)
{
	int			i;
	CDynLight*	pl = &m_data.lights [0];

m_data.nLights [1] = 0;
for (i = 0; i < m_data.nLights [0]; i++, pl++) {
#if DBG
	if ((nDbgSeg >= 0) && (nDbgSeg == pl->info.nSegment) && ((nDbgSide < 0) || (nDbgSide == pl->info.nSide)))
		nDbgSeg = nDbgSeg;
	if (i == nDbgLight)
		nDbgLight = nDbgLight;
#endif
	pl->render.vPosf [0].Assign (pl->info.vPos);
	if (ogl.m_states.bUseTransform)
		pl->render.vPosf [1] = pl->render.vPosf [0];
	else {
		transformation.Transform (pl->render.vPosf [1], pl->render.vPosf [0], 0);
		pl->render.vPosf [1].v.coord.w = 1;
		}
	pl->render.vPosf [0].v.coord.w = 1;
	pl->render.nType = pl->info.nType;
	pl->render.bState = pl->info.bState && (pl->info.color.red + pl->info.color.green + pl->info.color.blue > 0.0);
	pl->render.bLightning = (pl->info.nObject < 0) && (pl->info.nSide < 0);
	for (int j = 0; j < gameStates.app.nThreads; j++)
		ResetUsed (pl, j);
	pl->render.bShadow =
	pl->render.bExclusive = 0;
	if (pl->render.bState) {
		if (!bStatic && (pl->info.nType == 1) && !pl->info.bVariable)
			pl->render.bState = 0;
		if (!bVariable && ((pl->info.nType > 1) || pl->info.bVariable))
			pl->render.bState = 0;
		}
	m_data.renderLights [m_data.nLights [1]++] = pl;
	}
m_headlights.Prepare ();
m_headlights.Update ();
m_headlights.Transform ();
}

//------------------------------------------------------------------------------

#if SORT_ACTIVE_LIGHTS

static void QSortDynamicLights (int left, int right, int nThread)
{
	CDynLight**	activeLightsP = m_data.active [nThread];
	int			l = left,
					r = right,
					mat = activeLightsP [(l + r) / 2]->xDistance;

do {
	while (activeLightsP [l]->xDistance < mat)
		l++;
	while (activeLightsP [r]->xDistance > mat)
		r--;
	if (l <= r) {
		if (l < r) {
			CDynLight* h = activeLightsP [l];
			activeLightsP [l] = activeLightsP [r];
			activeLightsP [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	QSortDynamicLights (l, right, nThread);
if (left < r)
	QSortDynamicLights (left, r, nThread);
}

#endif //SORT_ACTIVE_LIGHTS

//------------------------------------------------------------------------------
// SetActive puts lights to be activated in a jump table depending on their 
// distance to the lit object. If a jump table entry is already occupied, the 
// light will be moved back in the table to the next free entry.

int CLightManager::SetActive (CActiveDynLight* activeLightsP, CDynLight* prl, short nType, int nThread, bool bForce)
{
#if DBG
if ((nDbgSeg >= 0) && (prl->info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (prl->info.nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
if ((nDbgObj >= 0) && (prl->info.nObject == nDbgObj))
	nDbgObj = nDbgObj;
#endif

if (prl->render.bUsed [nThread])
	return 0;
fix xDist;
if (bForce || prl->info.bSpot) 
	xDist = 0;
else {
	xDist = (prl->render.xDistance / 2000 + 5) / 10;
	if (xDist >= MAX_OGL_LIGHTS)
		return 0;
	if (xDist < 0)
		xDist = 0;
	}
#if PREFER_GEOMETRY_LIGHTS
else if (prl->info.nSegment >= 0)
	xDist /= 2;
else if (!prl->info.bSpot)
	xDist += (MAX_OGL_LIGHTS - xDist) / 2;
#endif
activeLightsP += xDist;
while (activeLightsP->nType) {
	if (activeLightsP->pl == prl)
		return 0;
	if (++xDist >= MAX_OGL_LIGHTS)
		return 0;
	activeLightsP++;
	}

#if DBG
if ((nDbgSeg >= 0) && (prl->info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (prl->info.nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
if ((nDbgObj >= 0) && (prl->info.nObject == nDbgObj))
	nDbgObj = nDbgObj;
#endif

activeLightsP->nType = nType;
activeLightsP->pl = prl;
prl->render.activeLightsP [nThread] = activeLightsP;
prl->render.bUsed [nThread] = ubyte (nType);

CDynLightIndex* sliP = &m_data.index [0][nThread];

sliP->nActive++;
if (sliP->nFirst > xDist)
	sliP->nFirst = short (xDist);
if (sliP->nLast < xDist)
	sliP->nLast = short (xDist);
return 1;
}

//------------------------------------------------------------------------------

CDynLight* CLightManager::GetActive (CActiveDynLight* activeLightsP, int nThread)
{
	CDynLight*	prl = activeLightsP->pl;
#if 0
if (prl) {
	if (prl->render.bUsed [nThread] > 1)
		prl->render.bUsed [nThread] = 0;
	if (activeLightsP->nType > 1) {
		activeLightsP->nType = 0;
		activeLightsP->pl = NULL;
		m_data.index [0][nThread].nActive--;
		}
	}
#endif
if (prl == reinterpret_cast<CDynLight*> (0xffffffff))
	return NULL;
return prl;
}

//------------------------------------------------------------------------------

ubyte CLightManager::VariableVertexLights (int nVertex)
{
	short*		pnl = m_data.nearestVertLights + nVertex * MAX_NEAREST_LIGHTS;
	CDynLight*	pl;
	short			i, j;
	ubyte			h;

#if DBG
if (nVertex == nDbgVertex)
	nDbgVertex = nDbgVertex;
#endif
for (h = 0, i = MAX_NEAREST_LIGHTS; i; i--, pnl++) {
	if ((j = *pnl) < 0)
		break;
	if ((pl = RenderLights (j)))
		h += pl->info.bVariable;
	}
return h;
}

//------------------------------------------------------------------------------

void CLightManager::SetNearestToVertex (int nSegment, int nSide, int nVertex, CFixVector *vNormal, ubyte nType, int bStatic, int bVariable, int nThread)
{
if (bStatic || m_data.variableVertLights [nVertex]) {
	PROF_START
	short*				pnl = m_data.nearestVertLights + nVertex * MAX_NEAREST_LIGHTS;
	CDynLightIndex*	sliP = &m_data.index [0][nThread];
	short					i, j, nActiveLightI = sliP->nActive;
	CDynLight*			prl;
	CActiveDynLight*	activeLightsP = m_data.active [nThread].Buffer ();
	CFixVector			vVertex = gameData.segs.vertices [nVertex], vLightToVertex;
	fix					xLightDist, xMaxLightRange = /*(gameStates.render.bPerPixelLighting == 2) ? MAX_LIGHT_RANGE * 2 :*/ MAX_LIGHT_RANGE;

#if DBG
if (nVertex == nDbgVertex)
	nDbgVertex = nDbgVertex;
#endif
	sliP->iVertex = nActiveLightI;
	for (i = MAX_NEAREST_LIGHTS; i; i--, pnl++) {
		if ((j = *pnl) < 0)
			break;
#if DBG
		if (j >= m_data.nLights [1])
			break;
#endif
		if (!(prl = RenderLights (j)))
			continue;
#if DBG
		if ((nDbgSeg >= 0) && (prl->info.nSegment == nDbgSeg))
			nDbgSeg = nDbgSeg;
#endif
		if (prl->render.bUsed [nThread])
			continue;
#if DBG
		if ((nDbgSeg >= 0) && (prl->info.nSegment == nDbgSeg))
			nDbgSeg = nDbgSeg;
#endif
		if (gameData.threads.vertColor.data.bNoShadow && prl->render.bShadow)
			continue;
		if (prl->info.bVariable) {
			if (!(bVariable && prl->info.bOn))
				continue;
			}
		else {
			if (!bStatic)
				continue;
			}
		vLightToVertex = vVertex - prl->info.vPos;
		xLightDist = vLightToVertex.Mag ();
		if ((prl->info.bDiffuse [nThread] = prl->SeesPoint (vNormal, vLightToVertex / xLightDist)) || (nSegment < 0))
			prl->render.xDistance = (fix) (xLightDist / prl->info.fRange);
		else if (nSegment >= 0)
			prl->render.xDistance = simpleRouter.PathLength (vVertex, nSegment, prl->info.vPos, prl->info.nSegment, X2I (xMaxLightRange / 5), WID_RENDPAST_FLAG | WID_FLY_FLAG, 0);
		if (prl->info.nSegment >= 0)
			prl->render.xDistance -= SEGMENTS [prl->info.nSegment].AvgRad ();
		if (prl->render.xDistance > xMaxLightRange)
			continue;
		if (SetActive (activeLightsP, prl, 2, nThread)) {
			prl->render.nType = nType;
			//prl->render.bState = 1;
			}
		}
	PROF_END(ptVertexLighting)
	}
}

//------------------------------------------------------------------------------

int CLightManager::SetNearestToFace (CSegFace* faceP, int bTextured)
{
PROF_START
#if 0
	static		int nFrameCount = -1;
if ((faceP == prevFaceP) && (nFrameCount == gameData.app.nFrameCount))
	return m_data.index [0][0].nActive;

prevFaceP = faceP;
nFrameCount = gameData.app.nFrameCount;
#endif
	int			i;
	CFixVector	vNormal;
	CSide*		sideP = SEGMENTS [faceP->m_info.nSegment].m_sides + faceP->m_info.nSide;

#if DBG
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
if (faceP - FACES.faces == nDbgFace)
	nDbgFace = nDbgFace;
#endif
#if 1//!DBG
if (m_data.index [0][0].nActive < 0)
	lightManager.SetNearestToSegment (faceP->m_info.nSegment, faceP - FACES.faces, 0, 0, 0);	//only get light emitting objects here (variable geometry lights are caught in lightManager.SetNearestToVertex ())
else {
	m_data.index [0][0] = m_data.index [1][0];
	}
#else
lightManager.SetNearestToSegment (faceP->m_info.nSegment, faceP - FACES, 0, 0, 0);	//only get light emitting objects here (variable geometry lights are caught in lightManager.SetNearestToVertex ())
#endif
vNormal = sideP->m_normals [2];
#if 1
for (i = 0; i < 4; i++)
	lightManager.SetNearestToVertex (-1, -1, faceP->m_info.index [i], &vNormal, 0, 0, 1, 0);
#endif
PROF_END(ptPerPixelLighting)
return m_data.index [0][0].nActive;
}

//------------------------------------------------------------------------------

void CLightManager::SetNearestStatic (int nSegment, int bStatic, int nThread)
{
	static short nActiveLights [4] = {-1, -1, -1, -1};

if (gameStates.render.nLightingMethod) {
	short*				pnl = m_data.nearestSegLights + nSegment * MAX_NEAREST_LIGHTS;
	short					i, j;
	CDynLight*			prl;
	CActiveDynLight*	activeLightsP = m_data.active [nThread].Buffer ();
	fix					xMaxLightRange = SEGMENTS [nSegment].AvgRad () + (/*(gameStates.render.bPerPixelLighting == 2) ? MAX_LIGHT_RANGE * 2 :*/ MAX_LIGHT_RANGE);
	CFixVector			c = SEGMENTS [nSegment].Center ();

	//m_data.iStaticLights [nThread] = m_data.index [0][nThread].nActive;
	for (i = MAX_NEAREST_LIGHTS; i; i--, pnl++) {
		if ((j = *pnl) < 0)
			break;
		if (!(prl = RenderLights (j)))
			continue;
		if (gameData.threads.vertColor.data.bNoShadow && prl->render.bShadow)
			continue;
		if (prl->info.bVariable) {
			if (!prl->info.bOn)
				continue;
			}
		else {
			if (!bStatic)
				continue;
			}
		prl->render.xDistance = (fix) ((CFixVector::Dist (c, prl->info.vPos) /*- F2X (prl->info.fRad)*/) / (prl->info.fRange * max (prl->info.fRad, 1.0f)));
		if (prl->render.xDistance > xMaxLightRange)
			continue;
		prl->info.bDiffuse [nThread] = 1;
		SetActive (activeLightsP, prl, 3, nThread);
		}
	}
nActiveLights [nThread] = m_data.index [0][nThread].nActive;
}

//------------------------------------------------------------------------------
// Retrieve closest variable (from objects and destructible/flickering geometry) lights.
// If bVariable == 0, only retrieve light emitting objects (will be stored in light buffer
// after all geometry lights).

short CLightManager::SetNearestToSegment (int nSegment, int nFace, int bVariable, int nType, int nThread)
{
PROF_START
	CDynLightIndex*	sliP = &m_data.index [0][nThread];

#if DBG
	static int nPrevSeg = -1;

if ((nDbgSeg >= 0) && (nSegment == nDbgSeg))
	nDbgSeg = nDbgSeg;
#endif
if (gameStates.render.nLightingMethod) {
	ubyte						nType;
	short						i = m_data.nLights [1],
								nLightSeg;
	int						bSkipHeadlight = !gameStates.render.nState && ((gameStates.render.bPerPixelLighting == 2) || gameOpts->ogl.bHeadlight);
	fix						xMaxLightRange = SEGMENTS [nSegment].AvgRad () + (/*(gameStates.render.bPerPixelLighting == 2) ? MAX_LIGHT_RANGE * 2 :*/ MAX_LIGHT_RANGE);
	CDynLight*				prl;
	CFixVector				c;
	CActiveDynLight*		activeLightsP = m_data.active [nThread].Buffer ();

	c = SEGMENTS [nSegment].Center ();
	lightManager.ResetAllUsed (0, nThread);
	lightManager.ResetActive (nThread, 0);
	while (i) {
		if (!(prl = RenderLights (--i)))
			continue;
#if DBG
		if ((nDbgSeg >= 0) && (prl->info.nSegment == nDbgSeg))
			prl = prl;
#endif
		if (gameData.threads.vertColor.data.bNoShadow && prl->render.bShadow)
			continue;
#if DBG
		if ((nDbgSeg >= 0) && (prl->info.nSegment == nDbgSeg))
			prl = prl;
		if ((prl->info.nSegment >= 0) && (prl->info.nSide < 0))
			prl = prl;
#endif
		nType = prl->info.nType;
		if (nType == 3) {
			if (bSkipHeadlight)
				continue;
			}
		else {
			if (prl->info.bPowerup > gameData.render.nPowerupFilter)
				continue;
			if (nType < 2) {	// all light emitting objects scanned
				if (!bVariable)
					break;
				if (!(prl->info.bVariable && prl->info.bOn))
					continue;
				}
#if DBG
			if (prl->info.nObject >= 0)
				nDbgObj = nDbgObj;
#endif
			if ((nLightSeg = prl->info.nSegment) >= 0) 
				prl->info.bDiffuse [nThread] = (prl->info.nSide >= 0) ? gameData.segs.LightVis (nLightSeg, nSegment) : gameData.segs.SegVis (nLightSeg, nSegment);
			else if ((prl->info.nObject >= 0) && ((nLightSeg = OBJECTS [prl->info.nObject].info.nSegment) >= 0))
				prl->info.bDiffuse [nThread] = gameData.segs.SegVis (nLightSeg, nSegment);
			else
				continue;
			}
		prl->render.xDistance = (fix) ((float) CFixVector::Dist (c, prl->info.vPos) / (prl->info.fRange * max (prl->info.fRad, 1.0f)));
		if (prl->render.xDistance > xMaxLightRange)
			continue;
		prl->info.bDiffuse [nThread] = 1;
#if DBG
		if (SetActive (activeLightsP, prl, 1, nThread)) {
			if ((nSegment == nDbgSeg) && (nDbgObj >= 0) && (prl->info.nObject == nDbgObj))
				prl = prl;
			if (nFace < 0)
				prl->render.nTarget = -nSegment - 1;
			else
				prl->render.nTarget = nFace + 1;
			prl->render.nFrame = gameData.app.nFrameCount;
			}
#else
		SetActive (activeLightsP, prl, 1, nThread);
#endif
		}
	m_data.index [1][nThread] = *sliP;
#if DBG
	if ((nDbgSeg >= 0) && (nSegment == nDbgSeg))
		nDbgSeg = nDbgSeg;
#endif
	}
#if DBG
nPrevSeg = nSegment;
#endif
PROF_END(ptSegmentLighting)
return sliP->nActive;
}

//------------------------------------------------------------------------------

short CLightManager::SetNearestToPixel (short nSegment, short nSide, CFixVector *vNormal, CFixVector *vPixelPos, float fLightRad, int nThread)
{
#if DBG
if ((nDbgSeg >= 0) && (nSegment == nDbgSeg))
	nDbgSeg = nDbgSeg;
#endif
if (gameStates.render.nLightingMethod) {
	int						nLightSeg;
	short						i, n = m_data.nLights [1];
	fix						xLightDist, xMaxLightRange = F2X (fLightRad) + (/*(gameStates.render.bPerPixelLighting == 2) ? MAX_LIGHT_RANGE * 2 :*/ MAX_LIGHT_RANGE);
	CDynLight*				prl;
	CFixVector				vLightToPixel;
	CActiveDynLight*		activeLightsP = m_data.active [nThread].Buffer ();
	bool						bForce, bLight = Find (nSegment, nSide, -1) >= 0;

	ResetActive (nThread, 0);
	ResetAllUsed (0, nThread);
	for (i = 0; i < n; i++) {
		if (!(prl = RenderLights (i)))
			continue;
#if DBG
		if ((nDbgSeg >= 0) && (prl->info.nSegment == nDbgSeg))
			prl = prl;
#endif
		if (prl->info.nType)
			break;
		if (prl->info.bVariable)
			continue;
		if (bLight && prl->info.bAmbient && ((prl->info.nSegment != nSegment) || (prl->info.nSide != nSide)))
			continue;
#if DBG
		if ((nDbgSeg >= 0) && (prl->info.nSegment == nDbgSeg))
			prl = prl;
#endif
		vLightToPixel = *vPixelPos - prl->info.vPos;
		xLightDist = vLightToPixel.Mag ();
		nLightSeg = prl->info.nSegment;
#if 0
		if ((nLightSeg != nSegment) || (prl->info.nSide != nSide)) {
			vLightToPixel.p.x = FixDiv (vLightToPixel.p.x, xLightDist);
			vLightToPixel.p.y = FixDiv (vLightToPixel.p.y, xLightDist);
			vLightToPixel.p.z = FixDiv (vLightToPixel.p.z, xLightDist);
			if (VmVecDot (vNormal, &vLightToPixel) >= 32768)
				continue;
			}
#endif
#if DBG
		if ((nDbgSeg >= 0) && (nDbgSeg == nLightSeg))
			nDbgSeg = nDbgSeg;
#endif
		if (nLightSeg < 0)
			continue;
		prl->info.bDiffuse [nThread] = gameData.segs.LightVis (nLightSeg, nSegment);
		if (!(bForce = (prl->info.nSegment == nSegment) && (prl->info.nSide == nSide))) {
			prl->render.xDistance = (fix) ((xLightDist /*- F2X (prl->info.fRad)*/) / prl->info.fRange);
			if (prl->render.xDistance > xMaxLightRange)
				continue;
			if (prl->info.bDiffuse [nThread])
				prl->info.bDiffuse [nThread] = prl->SeesPoint (nSegment, nSide, vLightToPixel / xLightDist);
			if (!prl->info.bDiffuse [nThread]) {
				prl->render.xDistance = simpleRouter.PathLength (*vPixelPos, nSegment, prl->info.vPos, prl->info.nSegment, X2I (xMaxLightRange / 5), WID_RENDPAST_FLAG | WID_FLY_FLAG, 0);
				if (prl->render.xDistance > xMaxLightRange)
					continue;
				}
			}
		SetActive (activeLightsP, prl, 1, nThread, bForce);
		}
	}
return m_data.index [0][nThread].nActive;
}

//------------------------------------------------------------------------------

int CLightManager::SetNearestToSgmAvg (short nSegment, int nThread)
{
	int			i;
	CSegment		*segP = SEGMENTS + nSegment;

#if DBG
if (nSegment == nDbgSeg)
	nDbgSeg = nDbgSeg;
#endif
lightManager.SetNearestToSegment (nSegment, -1, 0, 0, nThread);	//only get light emitting objects here (variable geometry lights are caught in lightManager.SetNearestToVertex ())
#if 1
for (i = 0; i < 8; i++)
	lightManager.SetNearestToVertex (-1, -1, segP->m_verts [i], NULL, 0, 1, 1, 0);
#endif
return m_data.index [0][0].nActive;
}

//------------------------------------------------------------------------------

tFaceColor* CLightManager::AvgSgmColor (int nSegment, CFixVector *vPosP, int nThread)
{
	tFaceColor	c, *pvc, *psc = gameData.render.color.segments + nSegment;
	short			i, *pv;
	CFixVector	vCenter, vVertex;
	float			d, ds;

#if DBG
if (nSegment == nDbgSeg)
	nSegment = nSegment;
#endif
if (!vPosP && (psc->index == (char) (gameData.app.nFrameCount & 0xff)) && (psc->color.red + psc->color.green + psc->color.blue != 0))
	return psc;
#if DBG
if (nSegment == nDbgSeg)
	nSegment = nSegment;
#endif
nThread = ThreadId (nThread);
if (SEGMENTS [nSegment].m_function == SEGMENT_FUNC_SKYBOX) {
	psc->color.red = psc->color.green = psc->color.blue = psc->color.alpha = 1.0f;
	psc->index = 1;
	}
else if (gameStates.render.bPerPixelLighting) {
	psc->color.red =
	psc->color.green =
	psc->color.blue = 0;
	psc->color.alpha = 1.0f;
	if (SetNearestToSgmAvg (nSegment, nThread)) {
			CVertColorData	vcd;

		InitVertColorData (vcd);
		vcd.vertNorm.SetZero ();
		if (vPosP)
			vcd.vertPos.Assign (*vPosP);
		else {
			vCenter = SEGMENTS [nSegment].Center ();
			vcd.vertPos.Assign (vCenter);
			}
		vcd.vertPosP = &vcd.vertPos;
		vcd.fMatShininess = 4;
		G3AccumVertColor (-1, reinterpret_cast<CFloatVector3*> (psc), &vcd, 0);
		}
#if DBG
	if (psc->color.red + psc->color.green + psc->color.blue == 0)
		psc = psc;
#endif
	lightManager.ResetAllUsed (0, nThread);
	m_data.index [0][0].nActive = -1;
	}
else {
	if (vPosP) {
		vCenter = SEGMENTS [nSegment].Center ();
		//transformation.Transform (&vCenter, &vCenter);
		ds = 0.0f;
		}
	else
		ds = 1.0f;
	pv = SEGMENTS [nSegment].m_verts;
	c.color.red = c.color.green = c.color.blue = 0.0f;
	c.index = 0;
	for (i = 0; i < 8; i++, pv++) {
		pvc = gameData.render.color.vertices + *pv;
		if (vPosP) {
			vVertex = gameData.segs.vertices [*pv];
			//transformation.Transform (&vVertex, &vVertex);
			d = 2.0f - X2F (CFixVector::Dist(vVertex, *vPosP)) / X2F (CFixVector::Dist(vCenter, vVertex));
			c.color.red += pvc->color.red * d;
			c.color.green += pvc->color.green * d;
			c.color.blue += pvc->color.blue * d;
			ds += d;
			}
		else {
			c.color.red += pvc->color.red;
			c.color.green += pvc->color.green;
			c.color.blue += pvc->color.blue;
			}
		}
#if DBG
	if (nSegment == nDbgSeg)
		nSegment = nSegment;
#endif
	psc->color.red = c.color.red / 8.0f;
	psc->color.green = c.color.green / 8.0f;
	psc->color.blue = c.color.blue / 8.0f;
#if 0
	if (lightManager.SetNearestToSegment (nSegment, 1)) {
		CDynLight*		prl;
		float				fLightRange = fLightRanges [IsMultiGame ? 1 : extraGameInfo [IsMultiGame].nLightfRange];
		float				fLightDist, fAttenuation;
		CFloatVector	vPosf;
		if (vPosP)
			vPosf.Assign (vPosP);
		for (i = 0; i < m_data.nActiveLights; i++) {
			prl = m_data.active [i];
#if 1
			if (vPosP) {
				vVertex = gameData.segs.vertices [*pv];
				//transformation.Transform (&vVertex, &vVertex);
				fLightDist = VmVecDist (prl->render.vPosf, &vPosf) / fLightRange;
				fAttenuation = fLightDist / prl->info.fBrightness;
				VmVecScaleAdd (reinterpret_cast<CFloatVector*> (&coord.color), reinterpret_cast<CFloatVector*> (&coord.color, reinterpret_cast<CFloatVector*> (&prl->render.color, 1.0f / fAttenuation);
				}
			else
#endif
			 {
				VmVecInc (reinterpret_cast<CFloatVector*> (&psc->color), reinterpret_cast<CFloatVector*> (&prl->render.color);
				}
			}
		}
#endif
#if 0
	d = psc->color.red;
	if (d < psc->color.green)
		d = psc->color.green;
	if (d < psc->color.blue)
		d = psc->color.blue;
	if (d > 1.0f) {
		psc->color.red /= d;
		psc->color.green /= d;
		psc->color.blue /= d;
		}
#endif
	}
psc->index = (char) (gameData.app.nFrameCount & 0xff);
return psc;
}

//------------------------------------------------------------------------------

void CLightManager::ResetSegmentLights (void)
{
for (short i = 0; i < gameData.segs.nSegments; i++)
	gameData.render.color.segments [i].index = -1;
}

//------------------------------------------------------------------------------

void CLightManager::ResetNearestStatic (int nSegment, int nThread)
{
if (gameStates.render.nLightingMethod) {
	short*		pnl = m_data.nearestSegLights + nSegment * MAX_NEAREST_LIGHTS;
	short			i, j;
	CDynLight*	prl;

	for (i = MAX_NEAREST_LIGHTS /*gameStates.render.nMaxLightsPerFace*/; i; i--, pnl++) {
		if ((j = *pnl) < 0)
			break;
		if ((prl = RenderLights (j)) && (prl->render.bUsed [nThread] == 3))
			ResetUsed (prl, nThread);
		}
	}
}

//------------------------------------------------------------------------------

void CLightManager::ResetNearestToVertex (int nVertex, int nThread)
{
//if (gameStates.render.nLightingMethod)
 {
	short*		pnl = m_data.nearestVertLights + nVertex * MAX_NEAREST_LIGHTS;
	short			i, j;
	CDynLight*	prl;

#if DBG
	if (nVertex == nDbgVertex)
		nDbgVertex = nDbgVertex;
#endif
	for (i = MAX_NEAREST_LIGHTS; i; i--, pnl++) {
		if ((j = *pnl) < 0)
			break;
		if ((prl = RenderLights (j)) && (prl->render.bUsed [nThread] == 2))
			ResetUsed (prl, nThread);
		}
	}
}

//------------------------------------------------------------------------------

void CLightManager::ResetUsed (CDynLight* prl, int nThread)
{
	CActiveDynLight* activeLightsP = prl->render.activeLightsP [nThread];

if (activeLightsP) {
	prl->render.activeLightsP [nThread] = NULL;
	activeLightsP->pl = NULL;
	activeLightsP->nType = 0;
	}
prl->render.bUsed [nThread] = 0;
}

//------------------------------------------------------------------------------

void CLightManager::ResetAllUsed (int bVariable, int nThread)
{
	int			i = m_data.nLights [1];
	CDynLight*	prl;

if (bVariable) {
	while (i) {
		if ((prl = m_data.renderLights [--i])) {
			if (prl->info.nType < 2)
				break;
			ResetUsed (prl, nThread);
			}
		}
	}
else {
	while (i) {
		if ((prl = m_data.renderLights [--i])) {
			ResetUsed (prl, nThread);
			}
		}
	}
}

//------------------------------------------------------------------------------

void CLightManager::ResetActive (int nThread, int nActive)
{
	CDynLightIndex*	sliP = &m_data.index [0][nThread];
	int					h;

if (0 < (h = sliP->nLast - sliP->nFirst + 1))
	memset (m_data.active [nThread] + sliP->nFirst, 0, sizeof (CActiveDynLight) * h);
sliP->nActive = nActive;
sliP->nFirst = MAX_OGL_LIGHTS;
sliP->nLast = 0;
}

// ----------------------------------------------------------------------------------------------
//eof
