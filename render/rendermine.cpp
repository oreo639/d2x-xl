/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "descent.h"
#include "error.h"
#include "gamefont.h"
#include "texmap.h"
#include "rendermine.h"
#include "fastrender.h"
#include "rendershadows.h"
#include "transprender.h"
#include "renderthreads.h"
#include "glare.h"
#include "radar.h"
#include "objrender.h"
#include "textures.h"
#include "screens.h"
#include "segpoint.h"
#include "texmerge.h"
#include "physics.h"
#include "segmath.h"
#include "light.h"
#include "dynlight.h"
#include "headlight.h"
#include "newdemo.h"
#include "automap.h"
#include "key.h"
#include "u_mem.h"
#include "kconfig.h"
#include "mouse.h"
#include "interp.h"
#include "ogl_lib.h"
#include "ogl_color.h"
#include "ogl_render.h"
#include "ogl_fastrender.h"
#include "cockpit.h"
#include "input.h"
#include "shadows.h"
#include "textdata.h"
#include "sparkeffect.h"
#include "createmesh.h"
#include "systemkeys.h"

// ------------------------------------------------------------------------------

#define CLEAR_WINDOW	0

int	nClearWindow = 0; //2	// 1 = Clear whole background tPortal, 2 = clear view portals into rest of world, 0 = no clear

void RenderSkyBox (int nWindow);

//------------------------------------------------------------------------------

extern int bLog;

CCanvas *reticleCanvas = NULL;

void _CDECL_ FreeReticleCanvas (void)
{
if (reticleCanvas) {
	PrintLog ("unloading reticle data\n");
	reticleCanvas->Destroy ();
	reticleCanvas = NULL;
	}
}

//------------------------------------------------------------------------------

//cycle the flashing light for when mine destroyed
void FlashFrame (void)
{
	static fixang flashAngle = 0;

if (automap.Display ())
	return;
if (!(gameData.reactor.bDestroyed || gameStates.gameplay.seismic.nMagnitude)) {
	gameStates.render.nFlashScale = I2X (1);
	return;
	}
if (gameStates.app.bEndLevelSequence)
	return;
if (paletteManager.BlueEffect () > 10)		//whiting out
	return;
//	flashAngle += FixMul(FLASH_CYCLE_RATE, gameData.time.xFrame);
if (gameStates.gameplay.seismic.nMagnitude) {
	fix xAddedFlash = abs(gameStates.gameplay.seismic.nMagnitude);
	if (xAddedFlash < I2X (1))
		xAddedFlash *= 16;
	flashAngle += (fixang) FixMul (gameStates.render.nFlashRate, FixMul(gameData.time.xFrame, xAddedFlash+I2X (1)));
	FixFastSinCos (flashAngle, &gameStates.render.nFlashScale, NULL);
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + I2X (3))/4;	//	gets in range 0.5 to 1.0
	}
else {
	flashAngle += (fixang) FixMul (gameStates.render.nFlashRate, gameData.time.xFrame);
	FixFastSinCos (flashAngle, &gameStates.render.nFlashScale, NULL);
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + I2X (1))/2;
	if (gameStates.app.nDifficultyLevel == 0)
		gameStates.render.nFlashScale = (gameStates.render.nFlashScale+I2X (3))/4;
	}
}

//------------------------------------------------------------------------------

void DoRenderObject (int nObject, int nWindow)
{
	CObject*					objP = OBJECTS + nObject;
	int						count = 0;

if (!(IsMultiGame || gameOpts->render.debug.bObjects))
	return;
Assert(nObject < LEVEL_OBJECTS);
#if 0
if (!(nWindow || gameStates.render.cameras.bActive) && (gameStates.render.nShadowPass < 2) &&
    (gameData.render.mine.bObjectRendered [nObject] == gameStates.render.nFrameFlipFlop))	//already rendered this...
	return;
#endif
if (gameData.demo.nState == ND_STATE_PLAYBACK) {
	if ((nDemoDoingLeft == 6 || nDemoDoingRight == 6) && objP->info.nType == OBJ_PLAYER) {
  		return;
		}
	}
if ((count++ > LEVEL_OBJECTS) || (objP->info.nNextInSeg == nObject)) {
	Int3();					// infinite loop detected
	objP->info.nNextInSeg = -1;		// won't this clean things up?
	return;					// get out of this infinite loop!
	}
if (RenderObject (objP, nWindow, 0)) {
	gameData.render.mine.bObjectRendered [nObject] = gameStates.render.nFrameFlipFlop;
	if (!gameStates.render.cameras.bActive) {
		tWindowRenderedData*	wrd = windowRenderedData + nWindow;
		int nType = objP->info.nType;
		if ((nType == OBJ_ROBOT) || (nType == OBJ_PLAYER) ||
			 ((nType == OBJ_WEAPON) && (WeaponIsPlayerMine (objP->info.nId) || (gameData.objs.bIsMissile [objP->info.nId] && EGI_FLAG (bKillMissiles, 0, 0, 0))))) {
			if (wrd->nObjects >= MAX_RENDERED_OBJECTS) {
				Int3();
				wrd->nObjects /= 2;
				}
			wrd->renderedObjects [wrd->nObjects++] = nObject;
			}
		}
	}
for (int i = objP->info.nAttachedObj; i != -1; i = objP->cType.explInfo.attached.nNext) {
	objP = OBJECTS + i;
	Assert (objP->info.nType == OBJ_FIREBALL);
	Assert (objP->info.controlType == CT_EXPLOSION);
	Assert (objP->info.nFlags & OF_ATTACHED);
	RenderObject (objP, nWindow, 1);
	}
}

//------------------------------------------------------------------------------

static tObjRenderListItem objRenderList [MAX_OBJECTS_D2X];

void QSortObjRenderList (int left, int right)
{
	int	l = left,
			r = right,
			m = objRenderList [(l + r) / 2].xDist;

do {
	while (objRenderList [l].xDist < m)
		l++;
	while (objRenderList [r].xDist > m)
		r--;
	if (l <= r) {
		if (l < r) {
			tObjRenderListItem h = objRenderList [l];
			objRenderList [l] = objRenderList [r];
			objRenderList [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	QSortObjRenderList (l, right);
if (left < r)
	QSortObjRenderList (left, r);
}

//------------------------------------------------------------------------------

int SortObjList (int nSegment)
{
	tObjRenderListItem*	pi;
	int						i, j;

if (nSegment < 0)
	nSegment = -nSegment - 1;
for (i = gameData.render.mine.renderObjs.ref [nSegment], j = 0; i >= 0; i = pi->nNextItem) {
	pi = gameData.render.mine.renderObjs.objs + i;
	objRenderList [j++] = *pi;
	}
#if 1
if (j > 1)
	QSortObjRenderList (0, j - 1);
#endif
return j;
}

//------------------------------------------------------------------------------

void RenderObjList (int nListPos, int nWindow)
{
PROF_START
	int i, j;
	int saveLinDepth = gameStates.render.detail.nMaxLinearDepth;

gameStates.render.detail.nMaxLinearDepth = gameStates.render.detail.nMaxLinearDepthObjects;
for (i = 0, j = SortObjList (gameData.render.mine.nSegRenderList [0][nListPos]); i < j; i++)
	DoRenderObject (objRenderList [i].nObject, nWindow);	// note link to above else
gameStates.render.detail.nMaxLinearDepth = saveLinDepth;
PROF_END(ptRenderObjects)
}

//------------------------------------------------------------------------------

void RenderSkyBoxObjects (void)
{
PROF_START
	short		i, nObject;
	short		*segNumP;

gameStates.render.nType = RENDER_TYPE_OBJECTS;
gameStates.render.nState = 1;
for (i = gameData.segs.skybox.ToS (), segNumP = gameData.segs.skybox.Buffer (); i; i--, segNumP++)
	for (nObject = SEGMENTS [*segNumP].m_objects; nObject != -1; nObject = OBJECTS [nObject].info.nNextInSeg)
		DoRenderObject (nObject, gameStates.render.nWindow);
PROF_END(ptRenderObjects)
}

//------------------------------------------------------------------------------

void RenderSkyBox (int nWindow)
{
PROF_START
if (gameStates.render.bHaveSkyBox && (!automap.Display () || gameOpts->render.automap.bSkybox)) {
	ogl.SetDepthWrite (true);
	RenderSkyBoxFaces ();
	RenderSkyBoxObjects ();
	}
PROF_END(ptRenderPass)
}

//------------------------------------------------------------------------------

static SDL_mutex* semaphore = NULL;

static sbyte bWaiting [MAX_THREADS];

//------------------------------------------------------------------------------

static inline int ObjectSegment (int i)
{
if (i >= gameData.render.mine.nRenderSegs)
	return -1;
short nSegment = gameData.render.mine.nSegRenderList [0][i];
if (nSegment < 0) {
	if (nSegment == -0x7fff)
		return -1;
	nSegment = -nSegment - 1;
	}
if (0 > gameData.render.mine.renderObjs.ref [nSegment])
	return -1;
return nSegment;
}

//------------------------------------------------------------------------------

void DoRenderMineObjects (int nThread)
{
	short nSegment;

for (int i = nThread; i < gameData.render.mine.nRenderSegs; i += gameStates.app.nThreads) {
	if (0 > (nSegment = ObjectSegment (i))) {
		bWaiting [nThread] = 1;
		while (bWaiting [nThread])
			; //G3_SLEEP (0);
		}
	else {
		if (gameStates.render.bApplyDynLight) {
			lightManager.SetNearestToSegment (nSegment, -1, 0, 1, nThread);
			lightManager.SetNearestStatic (nSegment, 1, 1, nThread);
			}
#if 0
#if USE_OPENMP > 1
#	pragma omp critical (objRender)
#elif !USE_OPENMP
SDL_mutexP (semaphore);
#endif
	{
#	if DBG
	if (lightManager.ThreadId (-1) != -1)
		nSegment = nSegment;
#	endif
	lightManager.SetThreadId (nThread);
	RenderObjList (i, gameStates.render.nWindow);
	lightManager.SetThreadId (-1);
	}
#if !USE_OPENMP
SDL_mutexV (semaphore);
#endif
#endif
		bWaiting [nThread] = 1;
		while (bWaiting [nThread])
			; //G3_SLEEP (0);
		if (gameStates.render.bApplyDynLight)
			lightManager.ResetNearestStatic (nSegment, nThread);
		}
	}	
bWaiting [nThread] = 2;
}

//------------------------------------------------------------------------------

void RenderMineObjects (int nType)
{
#if DBG
if (!gameOpts->render.debug.bObjects)
	return;
#endif
gameStates.render.nType = RENDER_TYPE_OBJECTS;
gameStates.render.nState = 1;
gameStates.render.bApplyDynLight = gameStates.render.bUseDynLight && gameOpts->ogl.bLightObjects;

#if USE_OPENMP > 1
#pragma omp parallel
{
#	pragma omp for 
	for (int i = 0; i < gameStates.app.nThreads; i++)
		DoRenderMineObjects (i);
}
#else
#	if !USE_OPENMP
if (!semaphore)
	semaphore = SDL_CreateMutex ();
memset (bWaiting, 1, sizeofa (bWaiting));
if (RunRenderThreads (-int (rtPolyModel) - 1)) {
	int h = (1 << gameStates.app.nThreads) - 1, i = 0, j, b;
	while (i < gameData.render.mine.nRenderSegs) {
		for (j = 0; j < gameStates.app.nThreads; j++)
			bWaiting [j] &= ~1;
		do {
			//G3_SLEEP (0);
			b = gameStates.app.nThreads;
			for (j = 0; j < gameStates.app.nThreads; j++)
				if (bWaiting [j])
					b--;
			} while (b);
		for (j = 0; j < gameStates.app.nThreads; j++, i++) {
			if (0 <= ObjectSegment (i)) {
				lightManager.SetThreadId (j);
				RenderObjList (i, gameStates.render.nWindow);
				lightManager.SetThreadId (-1);
				}
			}
		}
	}
else
#	endif
	DoRenderMineObjects (0);
#endif
gameStates.render.bApplyDynLight = (gameStates.render.nLightingMethod != 0);
gameStates.render.nState = 0;
}

//------------------------------------------------------------------------------

inline int RenderSegmentList (int nType)
{
PROF_START
gameStates.render.nType = nType;
if (!(EGI_FLAG (bShadows, 0, 1, 0) && FAST_SHADOWS && !gameOpts->render.shadows.bSoft && (gameStates.render.nShadowPass >= 2))) {
	BumpVisitedFlag ();
	RenderFaceList (nType);
	ogl.ClearError (0);
	}
RenderMineObjects (nType);
for (int i = 0; i < gameStates.app.nThreads; i++)
	lightManager.ResetAllUsed (1, i);
ogl.ClearError (0);
PROF_END(ptRenderPass)
return 1;
}

//------------------------------------------------------------------------------

void RenderEffects (int nWindow)
{
#if 1
	bool	bCreate = !gameOpts->render.n3DGlasses || (ogl.StereoSeparation () < 0) || nWindow || gameStates.render.cameras.bActive;
#else
	bool	bCreate = true; 
#endif
	int bLightning, bParticles, bSparks;
	PROF_START

#if UNIFY_THREADS
WaitForRenderThreads ();
#else
WaitForEffectsThread ();
#endif
if (automap.Display ()) {
	bLightning = gameOpts->render.automap.bLightning;
	bParticles = gameOpts->render.automap.bParticles;
	bSparks = gameOpts->render.automap.bSparks;
	}
else {
	bSparks = (gameOptions [0].render.nQuality > 0);
	bLightning = (!nWindow || gameOpts->render.lightning.bAuxViews) && 
					  (!gameStates.render.cameras.bActive || gameOpts->render.lightning.bMonitors);
	bParticles = (!nWindow || gameOpts->render.particles.bAuxViews) &&
					 (!gameStates.render.cameras.bActive || gameOpts->render.particles.bMonitors);
	}
if (bCreate) {
	if (bSparks) {
		SEM_ENTER (SEM_SPARKS)
		sparkManager.Render ();
		}
	if (bParticles) {
		SEM_ENTER (SEM_SMOKE)
		particleManager.Cleanup ();
		particleManager.Render ();
		}
	if (bLightning) {
		SEM_ENTER (SEM_LIGHTNING)
		lightningManager.Render ();
		}
	if (bLightning)
		SEM_LEAVE (SEM_LIGHTNING)
	}

transparencyRenderer.Render (nWindow);

if (bCreate) {
	if (bParticles)
		SEM_LEAVE (SEM_SMOKE)
	if (bSparks)
		SEM_LEAVE (SEM_SPARKS)
	}
PROF_END(ptEffects)
}

//------------------------------------------------------------------------------
//renders onto current canvas

extern int bLog;

void RenderMine (short nStartSeg, fix xStereoSeparation, int nWindow)
{
PROF_START
SetupMineRenderer ();
PROF_END(ptAux)
ComputeMineLighting (nStartSeg, xStereoSeparation, nWindow);
RenderSegmentList (RENDER_TYPE_GEOMETRY);	// render opaque geometry
if (!EGI_FLAG (bShadows, 0, 1, 0) || (gameStates.render.nShadowPass == 1)) {
	if (!gameStates.app.bNostalgia &&
		 (!automap.Display () || gameOpts->render.automap.bCoronas) && gameOpts->render.effects.bEnabled && gameOpts->render.coronas.bUse) 
		if (!nWindow && SetupCoronas ())
			RenderSegmentList (RENDER_TYPE_CORONAS);
	}
gameData.app.nMineRenderCount++;
PROF_END(ptRenderMine);
}

//------------------------------------------------------------------------------
// eof
