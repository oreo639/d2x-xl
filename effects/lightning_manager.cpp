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
#include "segmath.h"

#include "objsmoke.h"
#include "transprender.h"
#include "renderthreads.h"
#include "rendermine.h"
#include "error.h"
#include "light.h"
#include "dynlight.h"
#include "ogl_lib.h"
#include "automap.h"

CLightningManager lightningManager;
//------------------------------------------------------------------------------

CLightningManager::CLightningManager ()
{
m_objects = NULL;
m_lights = NULL;
m_nFirstLight = -1;
}

//------------------------------------------------------------------------------

CLightningManager::~CLightningManager ()
{
Shutdown (true);
}

//------------------------------------------------------------------------------

void CLightningManager::Init (void)
{
if (!(m_objects.Buffer () || m_objects.Create (LEVEL_OBJECTS))) {
	Shutdown (1);
	extraGameInfo [0].bUseLightning = 0;
	return;
	}
m_objects.Clear (0xff);
if (!(m_lights.Buffer () || m_lights.Create (2 * LEVEL_SEGMENTS))) {
	Shutdown (1);
	extraGameInfo [0].bUseLightning = 0;
	return;
	}
if (!m_emitters.Create (MAX_LIGHTNING_SYSTEMS)) {
	Shutdown (1);
	extraGameInfo [0].bUseLightning = 0;
	return;
	}
m_emitterList.Create (MAX_LIGHTNING_SYSTEMS);
int32_t i = 0;
int32_t nCurrent = m_emitters.FreeList ();
for (CLightningEmitter* pEmitter = m_emitters.GetFirst (nCurrent); pEmitter; pEmitter = m_emitters.GetNext (nCurrent))
	pEmitter->Init (i++);
m_nFirstLight = -1;
m_bDestroy = 0;
}

//------------------------------------------------------------------------------

class CLightningSettings {
	public:
		CFixVector	vPos;
		CFixVector	vEnd;
		CFixVector	vDelta;
		int32_t		nBolts;
		int32_t		nLife;
		int32_t		nFrames;
	};

//------------------------------------------------------------------------------

int32_t CLightningManager::Create (int32_t nBolts, CFixVector *vPos, CFixVector *vEnd, CFixVector *vDelta,
											  int16_t nObject, int32_t nLife, int32_t nDelay, int32_t nLength, int32_t nAmplitude,
											  char nAngle, int32_t nOffset, int16_t nNodes, int16_t nChildren, char nDepth, int16_t nFrames,
											  int16_t nSmoothe, char bClamp, char bGlow, char bSound, char bLight,
											  char nStyle, float nWidth, CFloatVector *pColor)
{
if (!(SHOW_LIGHTNING (1) && pColor))
	return -1;
if (!nBolts)
	return -1;
SEM_ENTER (SEM_LIGHTNING)
CLightningEmitter* pEmitter = m_emitters.Pop ();
if (!pEmitter)
	return -1;
// for random lightning, add a few extra nodes. These won't get rendered, but will cause the end of the lightning bolts to dance around a bit,
// making them look more natural
if (!(pEmitter->Create (nBolts, vPos, vEnd, vDelta, nObject, nLife, nDelay, nLength, nAmplitude,
							   nAngle, nOffset, nNodes, nChildren, nDepth, nFrames, nSmoothe, bClamp, bGlow, bSound, bLight,
							   nStyle, nWidth, pColor))) {
	m_emitters.Push (pEmitter->Id ());
	pEmitter->Destroy ();
	SEM_LEAVE (SEM_LIGHTNING)
	return -1;
	}
pEmitter->m_bValid = 1;
SEM_LEAVE (SEM_LIGHTNING)
return pEmitter->Id ();
}

//------------------------------------------------------------------------------

int32_t CLightningManager::Create (tLightningInfo& li, CFixVector *vPos, CFixVector *vEnd, CFixVector *vDelta, int16_t nObject)
{
CFloatVector color;

color.Red () = float (li.color.Red ()) / 255.0f;
color.Green () = float (li.color.Green ()) / 255.0f;
color.Blue () = float (li.color.Blue ()) / 255.0f;
color.Alpha () = float (li.color.Alpha ()) / 255.0f;

return Create (li.nBolts, 
					vPos, vEnd, vDelta, 
					nObject, 
					-abs (li.nLife), 
					li.nDelay, 
					(li.nLength > 0) ? I2X (li.nLength) : CFixVector::Dist (*vPos, *vEnd),
					(li.nAmplitude > 0) ? I2X (li.nAmplitude) : -I2X (li.nAmplitude) * gameOpts->render.lightning.nStyle, 
					li.nAngle, 
					I2X (li.nOffset), 
					(li.nNodes > 0) ? li.nNodes : -li.nNodes * gameOpts->render.lightning.nStyle, 
					li.nChildren, 
					li.nChildren > 0, 
					li.nFrames,
				   li.nSmoothe, 
					li.bClamp, 
					li.bGlow, 
					li.bSound, 
					1, 
					li.nStyle, 
					(float) li.nWidth, 
					&color);
}

//------------------------------------------------------------------------------

void CLightningManager::Destroy (CLightningEmitter* pEmitter, CLightning *pLightning)
{
if (pLightning)
	pLightning->Destroy ();
else
	pEmitter->m_bDestroy = 1;
}

//------------------------------------------------------------------------------

void CLightningManager::Cleanup (void)
{
SEM_ENTER (SEM_LIGHTNING)
int32_t nCurrent = -1;
for (CLightningEmitter* pEmitter = m_emitters.GetFirst (nCurrent), * pNext = NULL; pEmitter; pEmitter = pNext) {
	pNext = m_emitters.GetNext (nCurrent);
	if (pEmitter->m_bDestroy) {
		pEmitter->Destroy ();
		m_emitters.Push (pEmitter->Id ());
		}
	}
SEM_LEAVE (SEM_LIGHTNING)
}

//------------------------------------------------------------------------------

int32_t CLightningManager::Shutdown (bool bForce)
{
if (!bForce && (m_bDestroy >= 0))
	m_bDestroy = 1;
else {
	uint32_t bSem = gameData.app.semaphores [SEM_LIGHTNING];
	if (!bSem)
		SEM_ENTER (SEM_LIGHTNING)
	int32_t nCurrent = -1;
	for (CLightningEmitter* pEmitter = m_emitters.GetFirst (nCurrent), * pNext = NULL; pEmitter; pEmitter = pNext) {
		pNext = m_emitters.GetNext (nCurrent);
		Destroy (pEmitter, NULL);
		m_emitters.Push (pEmitter->Id ());
		}
	ResetLights (1);
	m_emitters.Destroy ();
	m_emitterList.Destroy ();
	m_objects.Destroy ();
	m_lights.Destroy ();
	if (!bSem)
		SEM_LEAVE (SEM_LIGHTNING)
	}
return 1;
}

//------------------------------------------------------------------------------

void CLightningManager::Move (int32_t i, CFixVector vNewPos, CFixVector vNewEnd, int16_t nSegment)
{
if (nSegment < 0)
	return;
if (SHOW_LIGHTNING (1))
	m_emitters [i].Move (vNewPos, vNewEnd, nSegment);
}

//------------------------------------------------------------------------------

void CLightningManager::Move (int32_t i, CFixVector vNewPos, int16_t nSegment)
{
if (nSegment < 0)
	return;
if (SHOW_LIGHTNING (1))
	m_emitters [i].Move (vNewPos, nSegment);
}

//------------------------------------------------------------------------------

void CLightningManager::MoveForObject (CObject* pObj)
{
	int32_t i = pObj->Index ();

if (m_objects [i] >= 0)
	Move (m_objects [i], OBJPOS (pObj)->vPos, OBJSEG (pObj));
}

//------------------------------------------------------------------------------

CFloatVector *CLightningManager::LightningColor (CObject* pObj)
{
if (pObj->info.nType == OBJ_ROBOT) {
	if (ROBOTINFO (pObj) && ROBOTINFO (pObj)->energyDrain) {
		static CFloatVector color = {{{1.0f, 0.8f, 0.3f, 0.2f}}};
		return &color;
		}
	}
else if ((pObj->info.nType == OBJ_PLAYER) && gameOpts->render.lightning.bPlayers) {
	if (gameData.FusionCharge (pObj->info.nId) > I2X (2)) {
		static CFloatVector color = {{{0.666f, 0.0f, 0.75f, 0.2f}}};
		return &color;
		}
	int32_t f = SEGMENT (pObj->info.nSegment)->m_function;
	if (f == SEGMENT_FUNC_FUELCENTER) {
		static CFloatVector color = {{{1.0f, 0.8f, 0.3f, 0.2f}}};
		return &color;
		}
	if (f == SEGMENT_FUNC_REPAIRCENTER) {
		static CFloatVector color = {{{0.1f, 0.3f, 1.0f, 0.2f}}};
		return &color;
		}
	}
return NULL;
}

//------------------------------------------------------------------------------

void CLightningManager::Update (void)
{
if (SHOW_LIGHTNING (1)) {

		CObject	*pObj;
		uint8_t		h;
		int32_t		i;

#if LIMIT_LIGHTNING_FPS
#	if LIGHTNING_SLOWMO
		static int32_t	t0 = 0;
		int32_t t = gameStates.app.nSDLTicks [0] - t0;

	if (t / gameStates.gameplay.slowmo [0].fSpeed < 25)
		return;
	t0 = gameStates.app.nSDLTicks [0] + 25 - int32_t (gameStates.gameplay.slowmo [0].fSpeed * 25);
#	else
	if (!gameStates.app.tick40fps.bTick)
		return 0;
#	endif
#endif
	for (i = 0, pObj = OBJECTS.Buffer (); i <= gameData.objData.nLastObject [1]; i++, pObj++) {
		if (gameData.objData.bWantEffect [i] & DESTROY_LIGHTNING) {
			gameData.objData.bWantEffect [i] &= ~DESTROY_LIGHTNING;
			DestroyForObject (pObj);
			}
		}
	int32_t nCurrent = -1;
#if USE_OPENMP // > 1
	if (gameStates.app.bMultiThreaded && m_emitterList.Buffer ()) {
		int32_t nSystems = 0;
		CLightningEmitter* pEmitter, * pNext;
		for (pEmitter = m_emitters.GetFirst (nCurrent), pNext = NULL; pEmitter; pEmitter = pNext) {
			m_emitterList [nSystems++] = pEmitter;
			pNext = m_emitters.GetNext (nCurrent);
			}
		if (nSystems > 0)
#	pragma omp parallel
			{
			int32_t nThread = omp_get_thread_num ();
#		pragma omp for private(pEmitter)
			for (int32_t i = 0; i < nSystems; i++) {
				pEmitter = m_emitterList [i];
				if (0 > pEmitter->Update (nThread))
					Destroy (pEmitter, NULL);
				}
			}
		}
	else 
#endif
		{
		for (CLightningEmitter* pEmitter = m_emitters.GetFirst (nCurrent), * pNext = NULL; pEmitter; pEmitter = pNext) {
			pNext = m_emitters.GetNext (nCurrent);
			if (0 > pEmitter->Update (0))
				Destroy (pEmitter, NULL);
			}
		}

	FORALL_OBJS (pObj) {
		i = pObj->Index ();
		h = gameData.objData.bWantEffect [i];
		if (h & EXPL_LIGHTNING) {
			if ((pObj->info.nType == OBJ_ROBOT) || (pObj->info.nType == OBJ_DEBRIS) || (pObj->info.nType == OBJ_REACTOR))
				CreateForBlowup (pObj);
#if DBG
			else if (pObj->info.nType != 255)
				PrintLog (1, "invalid effect requested\n");
#endif
			}
		else if (h & MISSILE_LIGHTNING) {
			if (pObj->IsMissile ())
				CreateForMissile (pObj);
#if DBG
			else if (pObj->info.nType != 255)
				PrintLog (1, "invalid effect requested\n");
#endif
			}
		else if (h & ROBOT_LIGHTNING) {
			if (pObj->info.nType == OBJ_ROBOT)
				CreateForRobot (pObj, LightningColor (pObj));
#if DBG
			else if (pObj->info.nType != 255)
				PrintLog (1, "invalid effect requested\n");
#endif
			}
		else if (h & PLAYER_LIGHTNING) {
			if (pObj->info.nType == OBJ_PLAYER)
				CreateForPlayer (pObj, LightningColor (pObj));
#if DBG
			else if (pObj->info.nType != 255)
				PrintLog (1, "invalid effect requested\n");
#endif
			}
		else if (h & MOVE_LIGHTNING) {
			if ((pObj->info.nType == OBJ_PLAYER) || (pObj->info.nType == OBJ_ROBOT))
				MoveForObject (pObj);
			}
		gameData.objData.bWantEffect [i] &= ~(PLAYER_LIGHTNING | ROBOT_LIGHTNING | MISSILE_LIGHTNING | EXPL_LIGHTNING | MOVE_LIGHTNING);
		}
	}
}

//------------------------------------------------------------------------------
#if 0
void MoveForObject (CObject* pObj)
{
SEM_ENTER (SEM_LIGHTNING)
MoveForObjectInternal (pObj);
SEM_LEAVE (SEM_LIGHTNING)
}
#endif
//------------------------------------------------------------------------------

void CLightningManager::DestroyForObject (CObject* pObj)
{
	int32_t i = pObj->Index ();

if (m_objects [i] >= 0) {
	Destroy (m_emitters + m_objects [i], NULL);
	m_objects [i] = -1;
	}
int32_t nCurrent = -1, nObject = pObj->Index ();
for (CLightningEmitter* pEmitter = m_emitters.GetFirst (nCurrent); pEmitter; pEmitter = m_emitters.GetNext (nCurrent))
	if (pEmitter->m_nObject == nObject)
		Destroy (pEmitter, NULL);
}

//------------------------------------------------------------------------------

void CLightningManager::DestroyForAllObjects (int32_t nType, int32_t nId)
{
	CObject* pObj;

FORALL_OBJS (pObj) {
	if ((pObj->info.nType == nType) && ((nId < 0) || (pObj->info.nId == nId)))
#if 1
		pObj->RequestEffects (DESTROY_LIGHTNING);
#else
		DestroyObjectLightnings (pObj);
#endif
	}
}

//------------------------------------------------------------------------------

void CLightningManager::DestroyForPlayers (void)
{
DestroyForAllObjects (OBJ_PLAYER, -1);
}

//------------------------------------------------------------------------------

void CLightningManager::DestroyForRobots (void)
{
DestroyForAllObjects (OBJ_ROBOT, -1);
}

//------------------------------------------------------------------------------

void CLightningManager::DestroyStatic (void)
{
DestroyForAllObjects (OBJ_EFFECT, LIGHTNING_ID);
}

//------------------------------------------------------------------------------

void CLightningManager::Render (void)
{
if (SHOW_LIGHTNING (1)) {
		int32_t bStencil = ogl.StencilOff ();

	int32_t nCurrent = -1;
#if USE_OPENMP > 1
	if (gameStates.app.bMultiThreaded && m_emitterList.Buffer ()) {
		CLightningEmitter* pEmitter;
		int32_t nSystems = 0;
		for (pEmitter = m_emitters.GetFirst (nCurrent); pEmitter; pEmitter = m_emitters.GetNext (nCurrent))
			if (!(pEmitter->m_nKey [0] | pEmitter->m_nKey [1]))
				m_emitterList [nSystems++] = pEmitter;
		if (nSystems) {
			if (nSystems < 2)
				m_emitterList [0]->Render (0, m_emitterList [0]->m_nBolts, 0);
			else
#	pragma omp parallel
				{
				int32_t nThread = omp_get_thread_num ();
#		pragma omp for private(pEmitter)
				for (int32_t i = 0; i < nSystems; i++) {
					pEmitter = m_emitterList [i];
					pEmitter->Render (0, pEmitter->m_nBolts, nThread);
					}
				}
			}
		}
	else 
#endif
		{
		for (CLightningEmitter* pEmitter = m_emitters.GetFirst (nCurrent); pEmitter; pEmitter = m_emitters.GetNext (nCurrent))
			if (!(pEmitter->m_nKey [0] | pEmitter->m_nKey [1]))
				pEmitter->Render (0, pEmitter->m_nBolts, 0);
		}
	ogl.StencilOn (bStencil);
	}
}

//------------------------------------------------------------------------------

void CLightningManager::Mute (void)
{
if (SHOW_LIGHTNING (1)) {
	int32_t nCurrent = -1;
	for (CLightningEmitter* pEmitter = m_emitters.GetFirst (nCurrent); pEmitter; pEmitter = m_emitters.GetNext (nCurrent))
		pEmitter->Mute ();
	}
}

//------------------------------------------------------------------------------

CFixVector *CLightningManager::FindTargetPos (CObject* pEmitter, int16_t nTarget)
{
	CObject* pObj;

if (!nTarget)
	return 0;
FORALL_EFFECT_OBJS (pObj) {
	if ((pObj != pEmitter) && (pObj->info.nId == LIGHTNING_ID) && (pObj->rType.lightningInfo.nId == nTarget))
		return &pObj->info.position.vPos;
	}
return NULL;
}

//------------------------------------------------------------------------------

int32_t CLightningManager::Enable (CObject* pObj)
{
int32_t h = m_objects [pObj->Index ()];
if (h < 0)
	return 0;
if (m_emitters [h].m_bValid)
	m_emitters [h].m_bValid = pObj->rType.lightningInfo.bEnabled ? 1 : -1;
return m_emitters [h].m_bValid;
}

//------------------------------------------------------------------------------

void CLightningManager::StaticFrame (void)
{
	CObject*		pObj;
	CFixVector*	vEnd, * vDelta, v;

if (!SHOW_LIGHTNING (1))
	return;
if (!gameOpts->render.lightning.bStatic)
	return;
FORALL_EFFECT_OBJS (pObj) {
	if (pObj->info.nId != LIGHTNING_ID)
		continue;
	int16_t nObject = pObj->Index ();
	if (m_objects [nObject] >= 0)
		continue;
	tLightningInfo& li = pObj->rType.lightningInfo;
	if (li.nBolts <= 0)
		continue;
	if (li.bRandom && !li.nAngle)
		vEnd = NULL;
	else if ((vEnd = FindTargetPos (pObj, li.nTarget)))
		li.nLength = CFixVector::Dist (pObj->info.position.vPos, *vEnd) / I2X (1);
	else {
		v = pObj->info.position.vPos + pObj->info.position.mOrient.m.dir.f * I2X (li.nLength);
		vEnd = &v;
		}
	vDelta = li.bInPlane ? &pObj->info.position.mOrient.m.dir.r : NULL;
	int32_t nHandle = Create (li, &pObj->info.position.vPos, vEnd, vDelta, nObject);
	if (nHandle >= 0) {
		m_objects [nObject] = nHandle;
		if (!pObj->rType.lightningInfo.bEnabled)
			m_emitters [nHandle].m_bValid = -1;
		}
	}
}

//------------------------------------------------------------------------------

void CLightningManager::DoFrame (void)
{
if (m_bDestroy) {
	m_bDestroy = -1;
	Shutdown (0);
	}
else {
	Update ();
	omegaLightning.Update (NULL, NULL);
	StaticFrame ();
	Cleanup ();
	}
}

//------------------------------------------------------------------------------

void CLightningManager::SetSegmentLight (int16_t nSegment, CFixVector *vPosP, CFloatVector *pColor)
{
if ((nSegment < 0) || (nSegment >= gameData.segData.nSegments))
	return;
else {
		tLightningLight	*pLight = m_lights + nSegment;

#if DBG
	if (nSegment == nDbgSeg)
		BRP;
#endif
	if (pLight->nFrame != gameData.app.nFrameCount) {
		memset (pLight, 0, sizeof (*pLight));
		pLight->nFrame = gameData.app.nFrameCount;
		pLight->nSegment = nSegment;
		pLight->nNext = m_nFirstLight;
		m_nFirstLight = nSegment;
		}
	pLight->nLights++;
	pLight->vPos += *vPosP;
	pLight->color.Red () += pColor->Red ();
	pLight->color.Green () += pColor->Green ();
	pLight->color.Blue () += pColor->Blue ();
	pLight->color.Alpha () += pColor->Alpha ();
	}
}

//------------------------------------------------------------------------------

void CLightningManager::ResetLights (int32_t bForce)
{
if ((SHOW_LIGHTNING (1) || bForce) && m_lights.Buffer ()) {
		tLightningLight	*pLight;
		int32_t				i;

	for (i = m_nFirstLight; i >= 0; ) {
		if ((i < 0) || (i >= LEVEL_SEGMENTS))
			break;
		pLight = m_lights + i;
		i = pLight->nNext;
		pLight->nLights = 0;
		pLight->nNext = -1;
		pLight->vPos.SetZero ();
		pLight->color.Red () =
		pLight->color.Green () =
		pLight->color.Blue () = 0;
		pLight->nBrightness = 0;
		if (pLight->nDynLight >= 0) {
			pLight->nDynLight = -1;
			}
		}
	m_nFirstLight = -1;
	lightManager.DeleteLightning ();
	}
}

//------------------------------------------------------------------------------

void CLightningManager::SetLights (void)
{
	int32_t nLights = 0;

ResetLights (0);
if (SHOW_LIGHTNING (1)) {
		tLightningLight*	pLight = NULL;
		int32_t				i, n, bDynLighting = gameStates.render.nLightingMethod;

	m_nFirstLight = -1;
	int32_t nCurrent = -1;

#if USE_OPENMP // > 1
	if (gameStates.app.bMultiThreaded && m_emitterList.Buffer ()) {
		CLightningEmitter* pEmitter;
		int32_t nSystems = 0;
		for (pEmitter = m_emitters.GetFirst (nCurrent); pEmitter; pEmitter = m_emitters.GetNext (nCurrent))
			m_emitterList [nSystems++] = pEmitter;
#	pragma omp parallel for private(pEmitter) reduction(+: nLights)
		for (int32_t i = 0; i < nSystems; i++) {
			pEmitter = m_emitterList [i];
			nLights += pEmitter->SetLight ();
			}
		}
	else 
#endif
		for (CLightningEmitter* pEmitter = m_emitters.GetFirst (nCurrent); pEmitter; pEmitter = m_emitters.GetNext (nCurrent))
			nLights += pEmitter->SetLight ();
	if (!nLights)
		return;
	nLights = 0;
	for (i = m_nFirstLight; i >= 0; i = pLight->nNext) {
		if ((i < 0) || (i >= LEVEL_SEGMENTS))
			continue;
		pLight = m_lights + i;
#if DBG
		if (pLight->nSegment == nDbgSeg)
			BRP;
#endif
		n = pLight->nLights;
		pLight->vPos.v.coord.x /= n;
		pLight->vPos.v.coord.y /= n;
		pLight->vPos.v.coord.z /= n;
		pLight->color.Red () /= n;
		pLight->color.Green () /= n;
		pLight->color.Blue () /= n;

#if 1
		pLight->nBrightness = F2X (sqrt (10 * (pLight->color.Red () + pLight->color.Green () + pLight->color.Blue ()) * pLight->color.Alpha ()));
#else
		if (gameStates.render.bPerPixelLighting == 2)
			pLight->nBrightness = F2X (sqrt (10 * (pLight->Red () + pLight->Green () + pLight->Blue ()) * pLight->Alpha ()));
		else
			pLight->nBrightness = F2X (10 * (pLight->Red () + pLight->Green () + pLight->Blue ()) * pLight->Alpha ());
#endif
		if (bDynLighting) {
			pLight->nDynLight = lightManager.Add (NULL, &pLight->color, pLight->nBrightness, pLight->nSegment, -1, -1, -1, &pLight->vPos);
			nLights++;
			}
		}
	}
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForExplosion (CObject* pObj, CFloatVector *pColor, int32_t nRods, int32_t nRad, int32_t nTTL)
{
if (SHOW_LIGHTNING (1) && gameOpts->render.lightning.bExplosions) {
	//m_objects [pObj->Index ()] =
		Create (
			nRods, &pObj->Position (), NULL, NULL, -pObj->Segment () - 1/*pObj->Index ()*/, nTTL, 0,
			nRad, I2X (4), 0, I2X (2), 50, 0, 1, 3, 1, 1, 0, 0, 1, -1, 3.0f, pColor);
	}
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForShaker (CObject* pObj)
{
static CFloatVector color = {{{0.1f, 0.1f, 0.8f, 0.2f}}};

CreateForExplosion (pObj, &color, 30, I2X (20), 750);
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForShakerMega (CObject* pObj)
{
static CFloatVector color = {{{0.1f, 0.1f, 0.6f, 0.2f}}};

CreateForExplosion (pObj, &color, 20, I2X (15), 750);
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForMega (CObject* pObj)
{
static CFloatVector color = {{{0.8f, 0.1f, 0.1f, 0.2f}}};

CreateForExplosion (pObj, &color, 30, I2X (15), 750);
}

//------------------------------------------------------------------------------

int32_t CLightningManager::CreateForMissile (CObject* pObj)
{
if (pObj->IsMissile ()) {
	if ((pObj->info.nId == EARTHSHAKER_ID) || (pObj->info.nId == EARTHSHAKER_ID))
		CreateForShaker (pObj);
	else if ((pObj->info.nId == EARTHSHAKER_MEGA_ID) || (pObj->info.nId == ROBOT_SHAKER_MEGA_ID))
		CreateForShakerMega (pObj);
	else if ((pObj->info.nId == MEGAMSL_ID) || (pObj->info.nId == ROBOT_MEGAMSL_ID))
		CreateForMega (pObj);
	else
		return 0;
	gameData.objData.bWantEffect [pObj->Index ()] &= ~DESTROY_LIGHTNING;
	return 1;
	}
return 0;
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForBlowup (CObject* pObj)
{
static CFloatVector color = {{{0.1f, 0.1f, 0.8f, 0.2f}}};

int32_t h = X2I (pObj->info.xSize) * 2;

CreateForExplosion (pObj, &color, h + Rand (h), h * (I2X (1) + I2X (1) / 2), 350);
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForTeleport (CObject* pObj, CFloatVector *pColor, float fRodScale)
{
if (SHOW_LIGHTNING (0) /*&& gameOpts->render.lightning.bExplosions*/) {
	int32_t h = X2I (pObj->info.xSize) * 4;

	Create (
		(int32_t) FRound ((h + Rand (h)) * fRodScale), &pObj->info.position.vPos, NULL, NULL, -pObj->info.nSegment - 1, 1000, 0,
		I2X (8), I2X (4), 0, I2X (2), 50, 0, 1, 3, 1, 1, -1, 0, 1, -1, 3.0f, pColor);
	}
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForPlayerTeleport (CObject* pObj)
{
#if 0
static CFloatVector color = {{{0.25f, 0.125f, 0.0f, 0.2f}}};
#else
static CFloatVector color = {{{0.0f, 0.125f, 0.25f, 0.2f}}};
#endif

CreateForTeleport (pObj, &color);
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForRobotTeleport (CObject* pObj)
{
static CFloatVector color = {{{0.25f, 0.0f, 0.125f, 0.2f}}};
CreateForTeleport (pObj, &color, 0.5f);
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForPowerupTeleport (CObject* pObj)
{
static CFloatVector color = {{{0.0f, 0.25f, 0.125f, 0.2f}}};
CreateForTeleport (pObj, &color);
}

//------------------------------------------------------------------------------

static tLightningInfo robotLightningInfo = {
	-5000, // nLife
	0, // nDelay
	0, // nLength
	-4, // nAmplitude
	0, // nOffset
	-1, // nWayPoint
	5, // nBolts
	-1, // nId
	-1, // nTarget
	100, // nNodes
	0, // nChildren
	3, // nFrames
	3, // nWidth
	0, // nAngle
	-1, // nStyle
	1, // nSmoothe
	1, // bClamp
	-1, // bGlow
	1, // bSound
	0, // bRandom
	0, // bInPlane
	1, // bEnabled
	0, // bDirection
	{(uint8_t) (255 * 0.9f), (uint8_t) (255 * 0.6f), (uint8_t) (255 * 0.6f), (uint8_t) (255 * 0.3f)} // color;
	};

void CLightningManager::CreateForRobot (CObject* pObj, CFloatVector *pColor)
{
if (SHOW_LIGHTNING (1) && gameOpts->render.lightning.bRobots && OBJECT_EXISTS (pObj)) {
		int32_t nObject = pObj->Index ();

	if (0 <= m_objects [nObject])
		MoveForObject (pObj);
	else {
		robotLightningInfo.color.Set (uint8_t (255 * pColor->v.color.r), uint8_t (255 * pColor->v.color.g), uint8_t (255 * pColor->v.color.b));
		int32_t h = lightningManager.Create (robotLightningInfo, &pObj->Position (), &OBJPOS (OBJECT (LOCALPLAYER.nObject))->vPos, NULL, nObject);
		if (h >= 0)
			m_objects [nObject] = h;
		}
	}
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForPlayer (CObject* pObj, CFloatVector *pColor)
{
if (SHOW_LIGHTNING (1) && gameOpts->render.lightning.bPlayers && OBJECT_EXISTS (pObj)) {
	int32_t h, nObject = pObj->Index ();

	if (0 <= m_objects [nObject])
		MoveForObject (pObj);
	else {
		int32_t s = pObj->info.xSize;
		int32_t i = X2I (s);
		h = Create (5 * i, &OBJPOS (pObj)->vPos, NULL, NULL, pObj->Index (), -250, 150,
						4 * s, s, 0, 2 * s, i * 20, (i + 1) / 2, 1, 3, 1, 1, -1, 1, 1, -1, 3.0f, pColor);
		if (h >= 0)
			m_objects [nObject] = h;
		}
	}
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForDamage (CObject* pObj, CFloatVector *pColor)
{
if (SHOW_LIGHTNING (1) && gameOpts->render.lightning.bDamage && OBJECT_EXISTS (pObj)) {
	int32_t i = pObj->Index ();
	int32_t n = X2IR (RobotDefaultShield (pObj));
	if (0 >= n)
		return;
	int32_t h = X2IR (pObj->info.xShield) * 100 / n;
	if ((h < 0) || (h >= 50))
		return;
	n = (5 - h / 10) * 2;
	if (0 <= (h = m_objects [i])) {
		if (m_emitters [h].m_nBolts == n) {
			MoveForObject (pObj);
			return;
			}
		Destroy (m_emitters + h, NULL);
		}
	h = Create (n, &pObj->info.position.vPos, NULL, NULL, pObj->Index (), -1000, 4000,
					pObj->info.xSize, pObj->info.xSize / 8, 0, 0, 20, 0, 1, 10, 1, 1, 0, 0, 0, -1, 3.0f, pColor);
	if (h >= 0)
		m_objects [i] = h;
	}
}

//------------------------------------------------------------------------------

int32_t CLightningManager::FindDamageLightning (int16_t nObject, int32_t *pKey)
{
int32_t nCurrent = -1;
for (CLightningEmitter* pEmitter = m_emitters.GetFirst (nCurrent); pEmitter; pEmitter = m_emitters.GetNext (nCurrent))
	if ((pEmitter->m_nObject == nObject) && (pEmitter->m_nKey [0] == pKey [0]) && (pEmitter->m_nKey [1] == pKey [1]))
		return pEmitter->Id ();
return -1;
}

//------------------------------------------------------------------------------

typedef union tPolyKey {
	int32_t	i [2];
	int16_t	s [4];
} tPolyKey;

int32_t CLightningManager::RenderForDamage (CObject* pObj, CRenderPoint **pointList, RenderModel::CVertex *pVertex, int32_t nVertices)
{
	CLightningEmitter*	pEmitter;
	CFloatVector			v, vPosf, vEndf, vNormf, vDeltaf;
	CFixVector				vPos, vEnd, vNorm, vDelta;
	int32_t					h, i, j, nLife = -1, bUpdate = 0;
	int16_t					nObject;
	tPolyKey					key;

	static int16_t	nLastObject = -1;
	static float	fDamage;
	static int32_t	nFrameFlipFlop = -1;

	static CFloatVector color = {{{0.2f, 0.2f, 1.0f, 1.0f}}};

if (!(SHOW_LIGHTNING (1) && gameOpts->render.lightning.bDamage))
	return -1;
if ((pObj->info.nType != OBJ_ROBOT) && (pObj->info.nType != OBJ_PLAYER))
	return -1;
if (nVertices < 3)
	return -1;
j = (nVertices > 4) ? 4 : nVertices;
h = (nVertices + 1) / 2;
// create a unique key for the lightning using the vertex key/index
if (pointList) {
	for (i = 0; i < j; i++)
		key.s [i] = pointList [i]->Key ();
	for (; i < 4; i++)
		key.s [i] = 0;
	}
else {
	for (i = 0; i < j; i++)
		key.s [i] = pVertex [i].m_nIndex;
	for (; i < 4; i++)
		key.s [i] = 0;
	}
i = FindDamageLightning (nObject = pObj->Index (), key.i);
if (i < 0) {
	// create new lightning stroke
	if ((nLastObject != nObject) || (nFrameFlipFlop != gameStates.render.nFrameFlipFlop)) {
		nLastObject = nObject;
		nFrameFlipFlop = gameStates.render.nFrameFlipFlop;
		fDamage = (0.5f - pObj->Damage ()) / 500.0f;
		}
#if 1
	if (RandDouble () > fDamage)
		return 0;
#endif
	if (pointList) {
		vPos = pointList [0]->WorldPos ();
		vEnd = pointList [1 + Rand (nVertices - 1)]->WorldPos ();
		vNorm = CFixVector::Normal (vPos, pointList [1]->WorldPos (), vEnd);
		vPos += vNorm * (I2X (1) / 64);
		vEnd += vNorm * (I2X (1) / 64);
		vDelta = CFixVector::Normal (vNorm, vPos, vEnd);
		h = CFixVector::Dist (vPos, vEnd);
		}
	else {
		memcpy (&vPosf, &pVertex->m_vertex, sizeof (CFloatVector3));
		memcpy (&vEndf, &pVertex [1 + Rand (nVertices - 1)].m_vertex, sizeof (CFloatVector3));
		memcpy (&v, &pVertex [1].m_vertex, sizeof (CFloatVector3));
		vNormf = CFloatVector::Normal (vPosf, v, vEndf);
		vPosf += vNormf * (1.0f / 64.0f);
		vEndf += vNormf * (1.0f / 64.0f);
		vDeltaf = CFloatVector::Normal (vNormf, vPosf, vEndf);
		h = F2X (CFloatVector::Dist (vPosf, vEndf));
		vPos.Assign (vPosf);
		vEnd.Assign (vEndf);
		}
	if (CFixVector::Dist (vPos, vEnd) < I2X (1) / 4)
		return -1;
	nLife = 1000 + Rand (2000);
	i = Create (1, &vPos, &vEnd, NULL /*&vDelta*/, nObject, nLife, 0,
					h, I2X (1) / 2, 0, 0, 20, 0, 1, 5, 0, 1, /*-1*/0, 0, 0, 1, 3.0f, &color);
	bUpdate = 1;
	}
if (i >= 0) {
	pEmitter = m_emitters + i;
	if (bUpdate) {
		pEmitter->m_nKey [0] = key.i [0];
		pEmitter->m_nKey [1] = key.i [1];
		}
	if (pEmitter->Lightning () && (pEmitter->m_nBolts = pEmitter->Lightning ()->Update (0, 0))) {
		if (nLife > 0)
			pEmitter->m_tUpdate = gameStates.app.nSDLTicks [0] + nLife;
		ogl.SetFaceCulling (false);
		pEmitter->Render (0, -1, -1);
		ogl.SetFaceCulling (true);
		return 1;
		}
	else {
		Destroy (m_emitters + i, NULL);
		}
	}
return 0;
}

//------------------------------------------------------------------------------
//eof
