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
#ifdef TACTILE
#	include "tactile.h"
#endif

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
int i = 0;
int nCurrent = m_emitters.FreeList ();
for (CLightningEmitter* emitterP = m_emitters.GetFirst (nCurrent); emitterP; emitterP = m_emitters.GetNext (nCurrent))
	emitterP->Init (i++);
m_nFirstLight = -1;
m_bDestroy = 0;
}

//------------------------------------------------------------------------------

int CLightningManager::Create (int nBolts, CFixVector *vPos, CFixVector *vEnd, CFixVector *vDelta,
										 short nObject, int nLife, int nDelay, int nLength, int nAmplitude,
										 char nAngle, int nOffset, short nNodes, short nChildren, char nDepth, short nSteps,
										 short nSmoothe, char bClamp, char bGlow, char bSound, char bLight,
										 char nStyle, tRgbaColorf *colorP)
{
if (!(SHOW_LIGHTNING && colorP))
	return -1;
if (!nBolts)
	return -1;
SEM_ENTER (SEM_LIGHTNING)
CLightningEmitter* emitterP = m_emitters.Pop ();
if (!emitterP)
	return -1;
if (!(emitterP->Create (nBolts, vPos, vEnd, vDelta, nObject, nLife, nDelay, nLength, nAmplitude,
							   nAngle, nOffset, nNodes, nChildren, nDepth, nSteps, nSmoothe, bClamp, bGlow, bSound, bLight,
							   nStyle, colorP))) {
	m_emitters.Push (emitterP->Id ());
	emitterP->Destroy ();
	SEM_LEAVE (SEM_LIGHTNING)
	return -1;
	}
emitterP->m_bValid = 1;
SEM_LEAVE (SEM_LIGHTNING)
return emitterP->Id ();
}

//------------------------------------------------------------------------------

void CLightningManager::Destroy (CLightningEmitter* emitterP, CLightning *lightningP)
{
if (lightningP)
	lightningP->Destroy ();
else
	emitterP->m_bDestroy = 1;
}

//------------------------------------------------------------------------------

void CLightningManager::Cleanup (void)
{
SEM_ENTER (SEM_LIGHTNING)
int nCurrent = -1;
for (CLightningEmitter* emitterP = m_emitters.GetFirst (nCurrent), * nextP = NULL; emitterP; emitterP = nextP) {
	nextP = m_emitters.GetNext (nCurrent);
	if (emitterP->m_bDestroy) {
		emitterP->Destroy ();
		m_emitters.Push (emitterP->Id ());
		}
	}
SEM_LEAVE (SEM_LIGHTNING)
}

//------------------------------------------------------------------------------

int CLightningManager::Shutdown (bool bForce)
{
if (!bForce && (m_bDestroy >= 0))
	m_bDestroy = 1;
else {
	uint bSem = gameData.app.semaphores [SEM_LIGHTNING];
	if (!bSem)
		SEM_ENTER (SEM_LIGHTNING)
	int nCurrent = -1;
	for (CLightningEmitter* emitterP = m_emitters.GetFirst (nCurrent), * nextP = NULL; emitterP; emitterP = nextP) {
		nextP = m_emitters.GetNext (nCurrent);
		Destroy (emitterP, NULL);
		m_emitters.Push (emitterP->Id ());
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

void CLightningManager::Move (int i, CFixVector vNewPos, CFixVector vNewEnd, short nSegment)
{
if (nSegment < 0)
	return;
if (SHOW_LIGHTNING)
	m_emitters [i].Move (vNewPos, vNewEnd, nSegment, 0);
}

//------------------------------------------------------------------------------

void CLightningManager::Move (int i, CFixVector vNewPos, short nSegment)
{
if (nSegment < 0)
	return;
if (SHOW_LIGHTNING)
	m_emitters [i].Move (vNewPos, nSegment, 0);
}

//------------------------------------------------------------------------------

void CLightningManager::MoveForObject (CObject* objP)
{
	int i = objP->Index ();

if (m_objects [i] >= 0)
	Move (m_objects [i], OBJPOS (objP)->vPos, OBJSEG (objP));
}

//------------------------------------------------------------------------------

tRgbaColorf *CLightningManager::LightningColor (CObject* objP)
{
if (objP->info.nType == OBJ_ROBOT) {
	if (ROBOTINFO (objP->info.nId).energyDrain) {
		static tRgbaColorf color = {1.0f, 0.8f, 0.3f, 0.2f};
		return &color;
		}
	}
else if ((objP->info.nType == OBJ_PLAYER) && gameOpts->render.lightning.bPlayers) {
	if (gameData.FusionCharge (objP->info.nId) > I2X (2)) {
		static tRgbaColorf color = {0.666f, 0.0f, 0.75f, 0.2f};
		return &color;
		}
	int s = SEGMENTS [objP->info.nSegment].m_function;
	if (s == SEGMENT_FUNC_FUELCEN) {
		static tRgbaColorf color = {1.0f, 0.8f, 0.3f, 0.2f};
		return &color;
		}
	if (s == SEGMENT_FUNC_REPAIRCEN) {
		static tRgbaColorf color = {0.3f, 0.5f, 0.1f, 0.2f};
		return &color;
		}
	}
return NULL;
}

//------------------------------------------------------------------------------

void CLightningManager::Update (void)
{
if (SHOW_LIGHTNING) {

		CObject	*objP;
		ubyte		h;
		int		i;

#if LIMIT_LIGHTNING_FPS
#	if LIGHTNING_SLOWMO
		static int	t0 = 0;
		int t = gameStates.app.nSDLTicks [0] - t0;

	if (t / gameStates.gameplay.slowmo [0].fSpeed < 25)
		return;
	t0 = gameStates.app.nSDLTicks [0] + 25 - int (gameStates.gameplay.slowmo [0].fSpeed * 25);
#	else
	if (!gameStates.app.tick40fps.bTick)
		return 0;
#	endif
#endif
	for (i = 0, objP = OBJECTS.Buffer (); i <= gameData.objs.nLastObject [1]; i++, objP++) {
		if (gameData.objs.bWantEffect [i] & DESTROY_LIGHTNING) {
			gameData.objs.bWantEffect [i] &= ~DESTROY_LIGHTNING;
			DestroyForObject (objP);
			}
		}
	int nCurrent = -1;
#if USE_OPENMP > 1
	if (m_emitterList.Buffer ()) {
		int nSystems = 0;
		CLightningEmitter* emitterP, * nextP;
		for (emitterP = m_emitters.GetFirst (nCurrent), nextP = NULL; emitterP; emitterP = nextP) {
			m_emitterList [nSystems++] = emitterP;
			nextP = m_emitters.GetNext (nCurrent);
			}
#	pragma omp parallel
			{
			int nThread = omp_get_thread_num ();
#		pragma omp for private(emitterP)
			for (int i = 0; i < nSystems; i++) {
				emitterP = m_emitterList [i];
				if (0 > emitterP->Update (nThread))
					Destroy (emitterP, NULL);
				}
			}
		}
	else 
#endif
		{
		for (CLightningEmitter* emitterP = m_emitters.GetFirst (nCurrent), * nextP = NULL; emitterP; emitterP = nextP) {
			nextP = m_emitters.GetNext (nCurrent);
			if (0 > emitterP->Update (0))
				Destroy (emitterP, NULL);
			}
		}

	FORALL_OBJS (objP, i) {
		i = objP->Index ();
		h = gameData.objs.bWantEffect [i];
		if (h & EXPL_LIGHTNING) {
			if ((objP->info.nType == OBJ_ROBOT) || (objP->info.nType == OBJ_REACTOR))
				CreateForBlowup (objP);
			else if (objP->info.nType != 255)
				PrintLog ("invalid effect requested\n");
			}
		else if (h & MISSILE_LIGHTNING) {
			if ((objP->info.nType == OBJ_WEAPON) || gameData.objs.bIsMissile [objP->info.nId])
				CreateForMissile (objP);
			else if (objP->info.nType != 255)
				PrintLog ("invalid effect requested\n");
			}
		else if (h & ROBOT_LIGHTNING) {
			if (objP->info.nType == OBJ_ROBOT)
				CreateForRobot (objP, LightningColor (objP));
			else if (objP->info.nType != 255)
				PrintLog ("invalid effect requested\n");
			}
		else if (h & PLAYER_LIGHTNING) {
			if (objP->info.nType == OBJ_PLAYER)
				CreateForPlayer (objP, LightningColor (objP));
			else if (objP->info.nType != 255)
				PrintLog ("invalid effect requested\n");
			}
		else if (h & MOVE_LIGHTNING) {
			if ((objP->info.nType == OBJ_PLAYER) || (objP->info.nType == OBJ_ROBOT))
				MoveForObject (objP);
			}
		gameData.objs.bWantEffect [i] &= ~(PLAYER_LIGHTNING | ROBOT_LIGHTNING | MISSILE_LIGHTNING | EXPL_LIGHTNING | MOVE_LIGHTNING);
		}
	}
}

//------------------------------------------------------------------------------
#if 0
void MoveForObject (CObject* objP)
{
SEM_ENTER (SEM_LIGHTNING)
MoveForObjectInternal (objP);
SEM_LEAVE (SEM_LIGHTNING)
}
#endif
//------------------------------------------------------------------------------

void CLightningManager::DestroyForObject (CObject* objP)
{
	int i = objP->Index ();

if (m_objects [i] >= 0) {
	Destroy (m_emitters + m_objects [i], NULL);
	m_objects [i] = -1;
	}
}

//------------------------------------------------------------------------------

void CLightningManager::DestroyForAllObjects (int nType, int nId)
{
	CObject	*objP;
	//int		i;

FORALL_OBJS (objP, i) {
	if ((objP->info.nType == nType) && ((nId < 0) || (objP->info.nId == nId)))
#if 1
		objP->RequestEffects (DESTROY_LIGHTNING);
#else
		DestroyObjectLightnings (objP);
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
if (SHOW_LIGHTNING) {
		int bStencil = ogl.StencilOff ();

	int nCurrent = -1;
#if USE_OPENMP > 1
	if (m_emitterList.Buffer ()) {
		CLightningEmitter* emitterP;
		int nSystems = 0;
		for (emitterP = m_emitters.GetFirst (nCurrent); emitterP; emitterP = m_emitters.GetNext (nCurrent))
			if (!(emitterP->m_nKey [0] | emitterP->m_nKey [1]))
				m_emitterList [nSystems++] = emitterP;
#	pragma omp parallel
			{
			int nThread = omp_get_thread_num ();
#		pragma omp for private(emitterP)
			for (int i = 0; i < nSystems; i++) {
				emitterP = m_emitterList [i];
				emitterP->Render (0, emitterP->m_nBolts, nThread);
				}
			}
		}
	else 
#endif
		{
		for (CLightningEmitter* emitterP = m_emitters.GetFirst (nCurrent); emitterP; emitterP = m_emitters.GetNext (nCurrent))
			if (!(emitterP->m_nKey [0] | emitterP->m_nKey [1]))
				emitterP->Render (0, emitterP->m_nBolts, 0);
		}
	ogl.StencilOn (bStencil);
	}
}

//------------------------------------------------------------------------------

void CLightningManager::Mute (void)
{
if (SHOW_LIGHTNING) {
	int nCurrent = -1;
	for (CLightningEmitter* emitterP = m_emitters.GetFirst (nCurrent); emitterP; emitterP = m_emitters.GetNext (nCurrent))
		emitterP->Mute ();
	}
}

//------------------------------------------------------------------------------

CFixVector *CLightningManager::FindTargetPos (CObject* emitterP, short nTarget)
{
	//int		i;
	CObject*	objP;

if (!nTarget)
	return 0;
FORALL_EFFECT_OBJS (objP, i) {
	if ((objP != emitterP) && (objP->info.nId == LIGHTNING_ID) && (objP->rType.lightningInfo.nId == nTarget))
		return &objP->info.position.vPos;
	}
return NULL;
}

//------------------------------------------------------------------------------

int CLightningManager::Enable (CObject* objP)
{
int h = m_objects [objP->Index ()];
if ((h >= 0) && m_emitters [h].m_bValid)
	m_emitters [h].m_bValid = objP->rType.lightningInfo.bEnabled ? 1 : -1;
return m_emitters [h].m_bValid;
}

//------------------------------------------------------------------------------

void CLightningManager::StaticFrame (void)
{
	int				h, i;
	CObject			*objP;
	CFixVector		*vEnd, *vDelta, v;
	tLightningInfo	*pli;
	tRgbaColorf		color;

if (!SHOW_LIGHTNING)
	return;
if (!gameOpts->render.lightning.bStatic)
	return;
FORALL_EFFECT_OBJS (objP, i) {
	if (objP->info.nId != LIGHTNING_ID)
		continue;
	i = objP->Index ();
	if (m_objects [i] >= 0)
		continue;
	pli = &objP->rType.lightningInfo;
	if (pli->nBolts <= 0)
		continue;
	if (pli->bRandom && !pli->nAngle)
		vEnd = NULL;
	else if ((vEnd = FindTargetPos (objP, pli->nTarget)))
		pli->nLength = CFixVector::Dist (objP->info.position.vPos, *vEnd) / I2X (1);
	else {
		v = objP->info.position.vPos + objP->info.position.mOrient.FVec () * I2X (pli->nLength);
		vEnd = &v;
		}
	color.red = float (pli->color.red) / 255.0f;
	color.green = float (pli->color.green) / 255.0f;
	color.blue = float (pli->color.blue) / 255.0f;
	color.alpha = float (pli->color.alpha) / 255.0f;
	vDelta = pli->bInPlane ? &objP->info.position.mOrient.RVec () : NULL;
	h = Create (pli->nBolts, &objP->info.position.vPos, vEnd, vDelta, i, -abs (pli->nLife), pli->nDelay, I2X (pli->nLength),
				   I2X (pli->nAmplitude), pli->nAngle, I2X (pli->nOffset), pli->nNodes, pli->nChildren, pli->nChildren > 0, pli->nSteps,
				   pli->nSmoothe, pli->bClamp, pli->bGlow, pli->bSound, 1, pli->nStyle, &color);
	if (h >= 0) {
		m_objects [i] = h;
		if (!objP->rType.lightningInfo.bEnabled)
			m_emitters [h].m_bValid = -1;
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

void CLightningManager::SetSegmentLight (short nSegment, CFixVector *vPosP, tRgbaColorf *colorP)
{
if ((nSegment < 0) || (nSegment >= gameData.segs.nSegments))
	return;
else {
		tLightningLight	*llP = m_lights + nSegment;

#if DBG
	if (nSegment == nDbgSeg)
		nDbgSeg = nDbgSeg;
#endif
	if (llP->nFrame != gameData.app.nFrameCount) {
		memset (llP, 0, sizeof (*llP));
		llP->nFrame = gameData.app.nFrameCount;
		llP->nSegment = nSegment;
		llP->nNext = m_nFirstLight;
		m_nFirstLight = nSegment;
		}
	llP->nLights++;
	llP->vPos += *vPosP;
	llP->color.red += colorP->red;
	llP->color.green += colorP->green;
	llP->color.blue += colorP->blue;
	llP->color.alpha += colorP->alpha;
	}
}

//------------------------------------------------------------------------------

void CLightningManager::ResetLights (int bForce)
{
if ((SHOW_LIGHTNING || bForce) && m_lights.Buffer ()) {
		tLightningLight	*llP;
		int					i;

	for (i = m_nFirstLight; i >= 0; ) {
		if ((i < 0) || (i >= LEVEL_SEGMENTS))
			break;
		llP = m_lights + i;
		i = llP->nNext;
		llP->nLights = 0;
		llP->nNext = -1;
		llP->vPos.SetZero ();
		llP->color.red =
		llP->color.green =
		llP->color.blue = 0;
		llP->nBrightness = 0;
		if (llP->nDynLight >= 0) {
			llP->nDynLight = -1;
			}
		}
	m_nFirstLight = -1;
	lightManager.DeleteLightnings ();
	}
}

//------------------------------------------------------------------------------

void CLightningManager::SetLights (void)
{
	int nLights = 0;

ResetLights (0);
if (SHOW_LIGHTNING) {
		tLightningLight	*llP = NULL;
		int					i, n, bDynLighting = gameStates.render.nLightingMethod;

	m_nFirstLight = -1;
	int nCurrent = -1;

#if USE_OPENMP > 1
	if (m_emitterList.Buffer ()) {
		CLightningEmitter* emitterP;
		int nSystems = 0;
		for (emitterP = m_emitters.GetFirst (nCurrent); emitterP; emitterP = m_emitters.GetNext (nCurrent))
			m_emitterList [nSystems++] = emitterP;
#	pragma omp parallel
			{
			int nThread = omp_get_thread_num ();
#		pragma omp for private(emitterP) reduction(+: nLights)
			for (int i = 0; i < nSystems; i++) {
				emitterP = m_emitterList [i];
				nLights += emitterP->SetLight ();
				}
			}
		}
	else 
#endif
		for (CLightningEmitter* emitterP = m_emitters.GetFirst (nCurrent); emitterP; emitterP = m_emitters.GetNext (nCurrent))
			nLights += emitterP->SetLight ();
	if (!nLights)
		return;
	nLights = 0;
	for (i = m_nFirstLight; i >= 0; i = llP->nNext) {
		if ((i < 0) || (i >= LEVEL_SEGMENTS))
			continue;
		llP = m_lights + i;
#if DBG
		if (llP->nSegment == nDbgSeg)
			nDbgSeg = nDbgSeg;
#endif
		n = llP->nLights;
		llP->vPos [X] /= n;
		llP->vPos [Y] /= n;
		llP->vPos [Z] /= n;
		llP->color.red /= n;
		llP->color.green /= n;
		llP->color.blue /= n;

#if 1
		llP->nBrightness = F2X (sqrt (10 * (llP->color.red + llP->color.green + llP->color.blue) * llP->color.alpha));
#else
		if (gameStates.render.bPerPixelLighting == 2)
			llP->nBrightness = F2X (sqrt (10 * (llP->color.red + llP->color.green + llP->color.blue) * llP->color.alpha));
		else
			llP->nBrightness = F2X (10 * (llP->color.red + llP->color.green + llP->color.blue) * llP->color.alpha);
#endif
		if (bDynLighting) {
			llP->nDynLight = lightManager.Add (NULL, &llP->color, llP->nBrightness, llP->nSegment, -1, -1, -1, &llP->vPos);
			nLights++;
			}
		}
	}
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForExplosion (CObject* objP, tRgbaColorf *colorP, int nRods, int nRad, int nTTL)
{
if (SHOW_LIGHTNING && gameOpts->render.lightning.bExplosions) {
	//m_objects [objP->Index ()] =
		Create (
			nRods, &objP->info.position.vPos, NULL, NULL, objP->Index (), nTTL, 0,
			nRad, I2X (4), 0, I2X (2), 50, 0, 1, 3, 1, 1, 0, 0, 1, -1, colorP);
	}
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForShaker (CObject* objP)
{
static tRgbaColorf color = {0.1f, 0.1f, 0.8f, 0.2f};

CreateForExplosion (objP, &color, 30, I2X (20), 750);
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForShakerMega (CObject* objP)
{
static tRgbaColorf color = {0.1f, 0.1f, 0.6f, 0.2f};

CreateForExplosion (objP, &color, 20, I2X (15), 750);
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForMega (CObject* objP)
{
static tRgbaColorf color = {0.8f, 0.1f, 0.1f, 0.2f};

CreateForExplosion (objP, &color, 30, I2X (15), 750);
}

//------------------------------------------------------------------------------

int CLightningManager::CreateForMissile (CObject* objP)
{
if (gameData.objs.bIsMissile [objP->info.nId]) {
	if ((objP->info.nId == EARTHSHAKER_ID) || (objP->info.nId == EARTHSHAKER_ID))
		CreateForShaker (objP);
	else if ((objP->info.nId == EARTHSHAKER_MEGA_ID) || (objP->info.nId == ROBOT_SHAKER_MEGA_ID))
		CreateForShakerMega (objP);
	else if ((objP->info.nId == MEGAMSL_ID) || (objP->info.nId == ROBOT_MEGAMSL_ID))
		CreateForMega (objP);
	else
		return 0;
	return 1;
	}
return 0;
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForBlowup (CObject* objP)
{
static tRgbaColorf color = {0.1f, 0.1f, 0.8f, 0.2f};

int h = X2I (objP->info.xSize) * 2;

CreateForExplosion (objP, &color, h + rand () % h, h * (I2X (1) + I2X (1) / 2), 350);
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForRobot (CObject* objP, tRgbaColorf *colorP)
{
if (SHOW_LIGHTNING && gameOpts->render.lightning.bRobots && OBJECT_EXISTS (objP)) {
		int h, i = objP->Index ();

	if (0 <= m_objects [i])
		MoveForObject (objP);
	else {
		h = Create (2 * objP->info.xSize / I2X (1), &objP->info.position.vPos, NULL, NULL, objP->Index (), -1000, 100,
						objP->info.xSize, objP->info.xSize / 8, 0, 0, 25, 3, 1, 3, 1, 1, 0, 0, 1, 0, colorP);
		if (h >= 0)
			m_objects [i] = h;
		}
	}
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForPlayer (CObject* objP, tRgbaColorf *colorP)
{
if (SHOW_LIGHTNING && gameOpts->render.lightning.bPlayers && OBJECT_EXISTS (objP)) {
	int h, nObject = objP->Index ();

	if (0 <= m_objects [nObject])
		MoveForObject (objP);
	else {
		int s = objP->info.xSize;
		int i = X2I (s);
		h = Create (5 * i, &OBJPOS (objP)->vPos, NULL, NULL, objP->Index (), -250, 150,
						4 * s, s, 0, 2 * s, i * 20, (i + 1) / 2, 1, 3, 1, 1, -1, 1, 1, 1, colorP);
		if (h >= 0)
			m_objects [nObject] = h;
		}
	}
}

//------------------------------------------------------------------------------

void CLightningManager::CreateForDamage (CObject* objP, tRgbaColorf *colorP)
{
if (SHOW_LIGHTNING && gameOpts->render.lightning.bDamage && OBJECT_EXISTS (objP)) {
		int h, n, i = objP->Index ();

	n = X2IR (RobotDefaultShield (objP));
	h = X2IR (objP->info.xShield) * 100 / n;
	if ((h < 0) || (h >= 50))
		return;
	n = (5 - h / 10) * 2;
	if (0 <= (h = m_objects [i])) {
		if (m_emitters [h].m_nBolts == n) {
			MoveForObject (objP);
			return;
			}
		Destroy (m_emitters + h, NULL);
		}
	h = Create (n, &objP->info.position.vPos, NULL, NULL, objP->Index (), -1000, 4000,
					objP->info.xSize, objP->info.xSize / 8, 0, 0, 20, 0, 1, 10, 1, 1, 0, 0, 0, -1, colorP);
	if (h >= 0)
		m_objects [i] = h;
	}
}

//------------------------------------------------------------------------------

int CLightningManager::FindDamageLightning (short nObject, int *pKey)
{
int nCurrent = -1;
for (CLightningEmitter* emitterP = m_emitters.GetFirst (nCurrent); emitterP; emitterP = m_emitters.GetNext (nCurrent))
	if ((emitterP->m_nObject == nObject) && (emitterP->m_nKey [0] == pKey [0]) && (emitterP->m_nKey [1] == pKey [1]))
		return emitterP->Id ();
return -1;
}

//------------------------------------------------------------------------------

typedef union tPolyKey {
	int	i [2];
	short	s [4];
} tPolyKey;

int CLightningManager::RenderForDamage (CObject* objP, g3sPoint **pointList, RenderModel::CVertex *vertP, int nVertices)
{
	CLightningEmitter*	emitterP;
	CFloatVector			v, vPosf, vEndf, vNormf, vDeltaf;
	CFixVector				vPos, vEnd, vNorm, vDelta;
	int						h, i, j, bUpdate = 0;
	short						nObject;
	tPolyKey					key;

	static short	nLastObject = -1;
	static float	fDamage;
	static int		nFrameFlipFlop = -1;

	static tRgbaColorf color = {0.2f, 0.2f, 1.0f, 1.0f};

if (!(SHOW_LIGHTNING && gameOpts->render.lightning.bDamage))
	return -1;
if ((objP->info.nType != OBJ_ROBOT) && (objP->info.nType != OBJ_PLAYER))
	return -1;
if (nVertices < 3)
	return -1;
j = (nVertices > 4) ? 4 : nVertices;
h = (nVertices + 1) / 2;
// create a unique key for the lightning using the vertex key/index
if (pointList) {
	for (i = 0; i < j; i++)
		key.s [i] = pointList [i]->p3_key;
	for (; i < 4; i++)
		key.s [i] = 0;
	}
else {
	for (i = 0; i < j; i++)
		key.s [i] = vertP [i].m_nIndex;
	for (; i < 4; i++)
		key.s [i] = 0;
	}
i = FindDamageLightning (nObject = objP->Index (), key.i);
if (i < 0) {
	// create new lightning stroke
	if ((nLastObject != nObject) || (nFrameFlipFlop != gameStates.render.nFrameFlipFlop)) {
		nLastObject = nObject;
		nFrameFlipFlop = gameStates.render.nFrameFlipFlop;
		fDamage = (0.5f - objP->Damage ()) / 250.0f;
		}
#if 1
	if (dbl_rand () > fDamage)
		return 0;
#endif
	if (pointList) {
		vPos = pointList [0]->p3_src;
		vEnd = pointList [1 + d_rand () % (nVertices - 1)]->p3_vec;
		vNorm = CFixVector::Normal (vPos, pointList [1]->p3_vec, vEnd);
		vPos += vNorm * (I2X (1) / 64);
		vEnd += vNorm * (I2X (1) / 64);
		vDelta = CFixVector::Normal (vNorm, vPos, vEnd);
		h = CFixVector::Dist (vPos, vEnd);
		}
	else {
		memcpy (&vPosf, &vertP->m_vertex, sizeof (CFloatVector3));
		memcpy (&vEndf, &vertP [1 + d_rand () % (nVertices - 1)].m_vertex, sizeof (CFloatVector3));
		memcpy (&v, &vertP [1].m_vertex, sizeof (CFloatVector3));
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
	i = Create (1, &vPos, &vEnd, NULL /*&vDelta*/, nObject, 1000 + d_rand () % 2000, 0,
					h, I2X (1) / 2, 0, 0, 20, 0, 1, 5, 0, 1, -1, 0, 0, 1, &color);
	bUpdate = 1;
	}
if (i >= 0) {
	emitterP = m_emitters + i;
	if (bUpdate) {
		emitterP->m_nKey [0] = key.i [0];
		emitterP->m_nKey [1] = key.i [1];
		}
	if (emitterP->Lightning () && (emitterP->m_nBolts = emitterP->Lightning ()->Update (0, 0))) {
		ogl.SetFaceCulling (false);
		emitterP->Render (0, -1, -1);
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
