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
#include "cockpit.h"
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
#include "postprocessing.h"

#if USE_OPENMP
#	define PERSISTENT_THREADS 1
#else
#	define PERSISTENT_THREADS 0
#endif

// ------------------------------------------------------------------------------

#define CLEAR_WINDOW	0

int32_t	nClearWindow = 0; //2	// 1 = Clear whole background tPortal, 2 = clear view portals into rest of world, 0 = no clear

void RenderSkyBox (int32_t nWindow);

//------------------------------------------------------------------------------

extern int32_t bLog;

CCanvas *reticleCanvas = NULL;

void _CDECL_ FreeReticleCanvas (void)
{
if (reticleCanvas) {
	PrintLog (0, "unloading reticle data\n");
	reticleCanvas->Destroy ();
	reticleCanvas = NULL;
	}
}

//------------------------------------------------------------------------------

//cycle the flashing light for when mine destroyed
void FlashFrame (void)
{
	static fixang flashAngle = 0;

if (automap.Active ())
	return;
if (!(gameData.reactor.bDestroyed || gameStates.gameplay.seismic.nMagnitude)) {
	gameStates.render.nFlashScale = 0;
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
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + I2X (3)) / 4;	//	gets in range 0.5 to 1.0
	}
else {
	flashAngle += (fixang) FixMul (gameStates.render.nFlashRate, gameData.time.xFrame);
	FixFastSinCos (flashAngle, &gameStates.render.nFlashScale, NULL);
	gameStates.render.nFlashScale = (gameStates.render.nFlashScale + I2X (1)) / 2;
	if (gameStates.app.nDifficultyLevel == 0)
		gameStates.render.nFlashScale = (gameStates.render.nFlashScale + I2X (3)) / 4;
	}
}

//------------------------------------------------------------------------------

void DoRenderObject (int32_t nObject, int32_t nWindow)
{
#if DBG
if (!(IsMultiGame || gameOpts->render.debug.bObjects))
	return;
#endif
#if 0 //DBG
if (gameData.render.mine.bObjectRendered [nObject] == gameStates.render.nFrameCount) 
	return;
#endif

CObject*	objP = gameData.Object (nObject);

#if DBG
if (nWindow && (objP->info.nType == OBJ_WEAPON))
	BRP;
#endif

if (gameData.demo.nState == ND_STATE_PLAYBACK) {
	if ((nDemoDoingLeft == 6 || nDemoDoingRight == 6) && objP->info.nType == OBJ_PLAYER) {
  		return;
		}
	}

if (RenderObject (objP, nWindow, 0)) {
#if 0 //DBG
	gameData.render.mine.bObjectRendered [nObject] = gameStates.render.nFrameCount;
#endif
	if (!gameStates.render.cameras.bActive) {
		tWindowRenderedData*	wrd = windowRenderedData + nWindow;
		int32_t nType = objP->info.nType;
		if ((nType == OBJ_ROBOT) || (nType == OBJ_PLAYER) ||
			 ((nType == OBJ_WEAPON) && (objP->IsPlayerMine () || (objP->IsMissile () && EGI_FLAG (bKillMissiles, 0, 0, 0))))) {
			if (wrd->nObjects >= MAX_RENDERED_OBJECTS) {
				Int3();
				wrd->nObjects /= 2;
				}
			wrd->renderedObjects [wrd->nObjects++] = nObject;
			}
		}
	}

for (int32_t i = objP->info.nAttachedObj; i != -1; i = objP->cType.explInfo.attached.nNext) {
	objP = gameData.Object (i);
	if (objP->info.nType != OBJ_FIREBALL) 
		break;
	if (objP->info.controlType != CT_EXPLOSION)
		break;
	if (!(objP->info.nFlags & OF_ATTACHED))
		break;
	RenderObject (objP, nWindow, 1);
	}
}

//------------------------------------------------------------------------------

static tObjRenderListItem objRenderList [MAX_OBJECTS_D2X];

void QSortObjRenderList (int32_t left, int32_t right)
{
	int32_t	l = left,
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

int32_t SortObjList (int32_t nSegment)
{
	tObjRenderListItem*	pi;
	int32_t						i, j;

if (nSegment < 0)
	nSegment = -nSegment - 1;
for (i = gameData.render.mine.objRenderList.ref [nSegment], j = 0; i >= 0; i = pi->nNextItem) {
	pi = gameData.render.mine.objRenderList.objs + i;
	objRenderList [j++] = *pi;
#if DBG
	if (gameData.Object (pi->nObject)->info.nSegment != nSegment)
		BRP;
#endif
	}
#if 1
if (j > 1)
	QSortObjRenderList (0, j - 1);
#endif
return j;
}

//------------------------------------------------------------------------------

static int32_t nDbgListPos = -1;

void RenderObjList (int32_t nListPos, int32_t nWindow)
{
#if DBG
if ((nListPos < 0) || (nListPos >= gameData.render.mine.nObjRenderSegs)) {
	//PrintLog (0, "invalid object render list index!\n");
	BRP;
	return;
	}
if ((gameData.render.mine.objRenderSegList [nListPos] < 0) || (gameData.render.mine.objRenderSegList [nListPos] >= gameData.segs.nSegments)) {
	//PrintLog (0, "invalid segment at object render list [%d]!\n", nListPos);
	BRP;
	return;
	}
#endif

PROF_START
	int32_t i, j;
	int32_t saveLinDepth = gameStates.render.detail.nMaxLinearDepth;

nDbgListPos = nListPos;
gameStates.render.detail.nMaxLinearDepth = gameStates.render.detail.nMaxLinearDepthObjects;
for (i = 0, j = SortObjList (gameData.render.mine.objRenderSegList [nListPos]); i < j; i++)
	DoRenderObject (objRenderList [i].nObject, nWindow);	// note link to above else
gameStates.render.detail.nMaxLinearDepth = saveLinDepth;
PROF_END(ptRenderObjects)
}

//------------------------------------------------------------------------------

void RenderSkyBoxObjects (void)
{
PROF_START
	int16_t		i, nObject;
	int16_t		*segNumP;

gameStates.render.nType = RENDER_TYPE_OBJECTS;
gameStates.render.nState = 1;
for (i = gameData.segs.skybox.ToS (), segNumP = gameData.segs.skybox.Buffer (); i; i--, segNumP++)
	for (nObject = gameData.Segment (*segNumP)->m_objects; nObject != -1; nObject = gameData.Object (nObject)->info.nNextInSeg)
		DoRenderObject (nObject, gameStates.render.nWindow [0]);
PROF_END(ptRenderObjects)
}

//------------------------------------------------------------------------------

void RenderSkyBox (int32_t nWindow)
{
PROF_START
if (gameStates.render.bHaveSkyBox && (!automap.Active () || gameOpts->render.automap.bSkybox)) {
	ogl.SetDepthWrite (true);
	RenderSkyBoxFaces ();
	}
PROF_END(ptRenderPass)
}

//------------------------------------------------------------------------------

void RenderObjectsST (void)
{
	int16_t nSegment;

for (int32_t i = 0; i < gameData.render.mine.nObjRenderSegs; i++) {
	nSegment = gameData.render.mine.objRenderSegList [i];
#if DBG
	if (nSegment == nDbgSeg)
		BRP;
#endif
	if (gameStates.render.bApplyDynLight) {
		lightManager.SetNearestToSegment (nSegment, -1, 0, 1, 0);
		//lightManager.ResetNearestStatic (nSegment, 0);
		lightManager.SetNearestStatic (nSegment, 1, 0);
		}
	RenderObjList (i, gameStates.render.nWindow [0]);
	if (gameStates.render.bApplyDynLight)
		lightManager.ResetNearestStatic (nSegment, 0);
	}	
}

//------------------------------------------------------------------------------
// The following pragmas keep the Visual C++ 9 optimizer from trying to inline
// the following thread function.

#ifdef _MSC_VER
#	pragma optimize("ga", off)
#	pragma auto_inline(off)
#endif

class CThreadedObjectRenderer {
	private:
		bool			m_bInited;
		SDL_Thread* m_threads [MAX_THREADS];
		int32_t			m_nThreadIds [MAX_THREADS];
		SDL_sem*		m_lightObjects [MAX_THREADS];
		SDL_mutex*	m_lightLock;
		SDL_sem*		m_lightDone;
		SDL_sem*		m_renderDone;
		int32_t			m_nRenderThreads;
		int32_t			m_nActiveThreads;

	public:
		CThreadedObjectRenderer ();
		~CThreadedObjectRenderer ();
		void Reset (void);
		void Create (void);
		void Destroy (void);
		void Render (void);
		int32_t Illuminate (void* nThreadP);
};

CThreadedObjectRenderer threadedObjectRenderer;

//------------------------------------------------------------------------------

int32_t _CDECL_ LightObjectsThread (void* nThreadP)
{
return threadedObjectRenderer.Illuminate (nThreadP);
}

//------------------------------------------------------------------------------

CThreadedObjectRenderer::CThreadedObjectRenderer () : m_bInited (false)
{
Reset ();
}

//------------------------------------------------------------------------------

CThreadedObjectRenderer::~CThreadedObjectRenderer () 
{
Destroy ();
}

//------------------------------------------------------------------------------

void CThreadedObjectRenderer::Reset (void) 
{
memset (m_threads, 0, sizeof (m_threads));
memset (m_nThreadIds, 0, sizeof (m_nThreadIds));
memset (m_lightObjects, 0, sizeof (m_lightObjects));
m_lightLock = NULL;
m_lightDone = NULL;
m_renderDone = NULL;
m_nRenderThreads = 0;
m_nActiveThreads = 0;
m_bInited = false;
}

//------------------------------------------------------------------------------

void CThreadedObjectRenderer::Destroy (void) 
{
if (m_bInited) {
	SDL_DestroyMutex (m_lightLock);
	SDL_DestroySemaphore (m_lightDone);
	SDL_DestroySemaphore (m_renderDone);
	for (int32_t i = 0; i < gameStates.app.nThreads; i++) {
		SDL_DestroySemaphore (m_lightObjects [i]);
		SDL_KillThread (m_threads [i]);
		}
	Reset ();
	}
}

//------------------------------------------------------------------------------

void CThreadedObjectRenderer::Create (void)
{
if (!m_bInited) {
	m_bInited = true;
	m_lightLock = SDL_CreateMutex ();
	m_lightDone = SDL_CreateSemaphore (0);
	m_renderDone = SDL_CreateSemaphore (0);
	for (int32_t i = 0; i < gameStates.app.nThreads; i++) {
		m_nThreadIds [i] = i;
		m_lightObjects [i] = SDL_CreateSemaphore (0);
#if PERSISTENT_THREADS
		m_threads [i] = SDL_CreateThread (LightObjectsThread, m_nThreadIds + i);
#endif
		}
	}
#if !PERSISTENT_THREADS
for (int32_t i = 0; i < m_nRenderThreads; i++)
	m_threads [i] = SDL_CreateThread (LightObjectsThread, m_nThreadIds + i);
#endif
m_nRenderThreads = Min (gameData.render.mine.nObjRenderSegs, gameStates.app.nThreads);
m_nActiveThreads = m_nRenderThreads;
}

//------------------------------------------------------------------------------
// LightObjectsThread computes the segment lighting for the object render process.
// It will set its semaphore to 1 after lighting for a segment has been computed to
// tell the render process that this segment's objects can be rendered, and waits
// for the render process to reset its semaphore before proceeding with the next 
// segment.
#if DBG
static int32_t nDbgThread = -1;
#endif

int32_t CThreadedObjectRenderer::Illuminate (void* nThreadP)
{
#if 1
	int32_t	nThread = *((int32_t*) nThreadP);
#endif
	int16_t nSegment;

#if PERSISTENT_THREADS
for (;;) {
	SDL_SemWait (m_lightObjects [nThread]);
#	if DBG
	if (!m_nActiveThreads)
		BRP;
#	endif
#endif
	for (int32_t i = nThread; i < gameData.render.mine.nObjRenderSegs; i += m_nRenderThreads) {
		nSegment = gameData.render.mine.objRenderSegList [i];
		if (gameStates.render.bApplyDynLight) {
			lightManager.SetNearestToSegment (nSegment, -1, 0, 1, nThread);
			lightManager.SetNearestStatic (nSegment, 1, nThread);
			}
		SDL_LockMutex (m_lightLock); // renderer must only be called by one thread at a time
#	if DBG
		if (lightManager.ThreadId (nThread) != nThread)
			BRP;
#	endif
		lightManager.SetThreadId (nThread);
		SDL_SemPost (m_lightDone); // tell the renderer it can render some objects
		SDL_SemWait (m_renderDone); // wait until the renderer is done
		SDL_UnlockMutex (m_lightLock);
		if (gameStates.render.bApplyDynLight)
			lightManager.ResetNearestStatic (nSegment, nThread);
		}	
#if PERSISTENT_THREADS
	SDL_LockMutex (m_lightLock); // active render thread count must only be decremented by one thread at a time
#	if DBG
	if (!m_nActiveThreads)
		BRP;
	else
#	endif
	if (!--m_nActiveThreads)
		SDL_SemPost (m_lightDone); // tell the renderer to quit rendering and continue program execution
	SDL_UnlockMutex (m_lightLock);
	}
#endif
return 1;
}

//------------------------------------------------------------------------------
// Main object rendering function. It must reside in the main process because only 
// the main process has a valid OpenGL context. It communicates
// with the object lighting threads via semaphores. If a lighting thread's semaphore
// is set, that thread has finished lighting calculations for its current segment
// and all objects in that segment can be rendered. The lighting thread waits for the
// semaphore to be reset by the render process after the objects have been rendered.
// Since the object render process needs to run full throttle, only gameStates.app.nThreads - 1
// lighting m_threads are executed.
// Note: SDL's semaphore/mutex handling doesn't really cut it here (and is too slow).

void CThreadedObjectRenderer::Render (void)
{
Create ();

#if DBG
lightManager.SetThreadId (-1);
#endif

int32_t i, nListPos [MAX_THREADS];
for (i = 0; i < MAX_THREADS; i++)
	nListPos [i] = i;

for (int32_t i = 0; i < m_nRenderThreads; i++) 
	SDL_SemPost (m_lightObjects [i]);

#if PERSISTENT_THREADS
for (;;) {
	SDL_SemWait (m_lightDone);
	if (!m_nActiveThreads)
		break;
	int32_t nThread = lightManager.ThreadId (-1);
#if DBG
	if (nThread < 0)
		BRP;
#endif
	RenderObjList (nListPos [nThread], gameStates.render.nWindow [0]);
#if DBG
	lightManager.SetThreadId (-1);
#endif
	nListPos [nThread] += m_nRenderThreads;
	SDL_SemPost (m_renderDone);
	}
#else
for (i = 0; i < gameStates.app.nThreads; i++) 
	SDL_WaitThread (m_threads [i], NULL);
#endif
lightManager.SetThreadId (-1);
}

#ifdef _MSC_VER
#	pragma optimize("ga", on)
#	pragma auto_inline(on)
#endif

//------------------------------------------------------------------------------

#if DBG

static inline int32_t VerifyObjectRenderSegment (int16_t nSegment)
{
for (int32_t i = 0; i < gameData.render.mine.nObjRenderSegs; i++)
	if (gameData.render.mine.objRenderSegList [i] == nSegment)
		return -1;
return nSegment;
}

#else

#	define VerifyObjectRenderSegment(_nSegment) (_nSegment)


#endif

//------------------------------------------------------------------------------

static int32_t ObjectRenderSegment (int32_t i)
{
if (i >= gameData.render.mine.visibility [0].nSegments)
	return -1;
int16_t nSegment = gameData.render.mine.visibility [0].segments [i];
if (nSegment < 0) {
	if (nSegment == -0x7fff)
		return -1;
	nSegment = -nSegment - 1;
	}
if (0 > gameData.render.mine.objRenderList.ref [nSegment])
	return -1;
if (!automap.Active ())
	return VerifyObjectRenderSegment (nSegment);
if (extraGameInfo [IsMultiGame].bPowerupsOnRadar && extraGameInfo [IsMultiGame].bRobotsOnRadar)
	return VerifyObjectRenderSegment (nSegment);

tObjRenderListItem* pi;

for (i = gameData.render.mine.objRenderList.ref [nSegment]; i >= 0; i = pi->nNextItem) {
	pi = gameData.render.mine.objRenderList.objs + i;
	int32_t nType = gameData.Object (pi->nObject)->info.nType;
	if (nType == OBJ_POWERUP) {
		if (extraGameInfo [IsMultiGame].bPowerupsOnRadar)
			return VerifyObjectRenderSegment (nSegment);
		}
	else if (nType == OBJ_ROBOT) {
		if (extraGameInfo [IsMultiGame].bRobotsOnRadar)
			return VerifyObjectRenderSegment (nSegment);
		}
	else
		return VerifyObjectRenderSegment (nSegment);
	}

return -1;
}

//------------------------------------------------------------------------------

void RenderMineObjects (int32_t nType)
{
#if DBG
if (!gameOpts->render.debug.bObjects)
	return;
#endif
if (nType != RENDER_TYPE_GEOMETRY)
	return;
if (automap.Active () && !(gameOpts->render.automap.bTextured & 1))
	return;

gameStates.render.nType = RENDER_TYPE_OBJECTS;
gameStates.render.nState = 1;
gameStates.render.bApplyDynLight = gameStates.render.bUseDynLight && gameOpts->ogl.bLightObjects && !gameStates.render.bFullBright;

	int32_t	i;
	int16_t nSegment;

gameData.render.mine.nObjRenderSegs = 0;
for (i = 0; i < gameData.render.mine.visibility [0].nSegments; i++)
	if (0 <= (nSegment = ObjectRenderSegment (i)))
		gameData.render.mine.objRenderSegList [gameData.render.mine.nObjRenderSegs++] = nSegment;

#if 0

RenderObjectsST ();

#else

if (!gameStates.app.bMultiThreaded || (gameStates.render.nShadowPass == 2) || (gameStates.app.nThreads < 2) || (gameData.render.mine.nObjRenderSegs < 2/*gameStates.app.nThreads - 1*/))
	RenderObjectsST ();
else
#if USE_OPENMP // > 1
	threadedObjectRenderer.Render ();
#else
	RenderObjectsST ();
#endif

#endif

gameStates.render.bApplyDynLight = (gameStates.render.nLightingMethod != 0);
gameStates.render.nState = 0;
}

//------------------------------------------------------------------------------

int32_t RenderSegmentList (int32_t nType)
{
PROF_START

gameStates.render.nType = nType;
#if MAX_SHADOWMAPS
#	if MAX_SHADOWMAPS > 0
if (gameStates.render.nShadowMap == 0) 
#endif
	{
#else
if (!(EGI_FLAG (bShadows, 0, 1, 0) && FAST_SHADOWS && !gameOpts->render.shadows.bSoft && (gameStates.render.nShadowPass >= 2))) {
#endif
	gameData.render.mine.visibility [0].BumpVisitedFlag ();
	RenderFaceList (nType);
	ogl.ClearError (0);
	}
#if MAX_SHADOWMAPS >= 0
RenderMineObjects (nType);
#endif
for (int32_t i = 0; i < gameStates.app.nThreads; i++)
	lightManager.ResetAllUsed (1, i);
ogl.ClearError (0);
PROF_END(ptRenderPass)
return 1;
}

//------------------------------------------------------------------------------

void RenderEffects (int32_t nWindow)
{
PROF_START

if (!ogl.StereoDevice () || (ogl.StereoSeparation () < 0) || nWindow || gameStates.render.cameras.bActive) {
	int32_t bLightning, bParticles, bSparks;

	if (gameStates.app.nThreads > 1) {
		while (!transparencyRenderer.Ready ())
			G3_SLEEP (0);
		}
	if (automap.Active ()) {
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

	if (bSparks) 
		sparkManager.Render ();
	if (bParticles) {
		//particleManager.Cleanup ();
		particleManager.Render ();
		}
	if (bLightning) 
		lightningManager.Render ();
	}
transparencyRenderer.Render (nWindow);
if (!nWindow) 
	postProcessManager.Prepare ();

PROF_END(ptEffects)
}

//------------------------------------------------------------------------------

int32_t bHave3DCockpit = -1;

void RenderCockpitModel (void)
{
	static int32_t bCockpit = 1;
	static float yOffset = 5.0f;

if (bCockpit && bHave3DCockpit && (gameStates.render.cockpit.nType == CM_FULL_COCKPIT)) {
	int32_t bFullBright = gameStates.render.bFullBright;
	gameStates.render.bFullBright = 1;
	ogl.SetTransform (1);
	CFixVector vOffset = /*gameData.Object (0)->Orientation ().m.dir.f * F2X (xOffset) +*/ gameData.Object (0)->Orientation ().m.dir.u * F2X (yOffset);
	gameData.Object (0)->Position () -= vOffset;
	gameData.render.scene.Activate ("RenderCockpitModel (scene)");
	gameData.models.vScale.Set (F2X (float (gameData.render.screen.Width ()) / float (gameData.render.screen.Height ()) * 0.75f), I2X (1), I2X (1));
	bHave3DCockpit = (G3RenderModel (gameData.Object (0), COCKPIT_MODEL, -1, NULL, gameData.models.textures, NULL, NULL, 0, NULL, NULL) > 0);
	gameData.models.vScale.Set (I2X (1), I2X (1), I2X (1));
	CCanvas::Current ()->SetViewport ();
	gameData.Object (0)->Position () += vOffset;
	ogl.SetTransform (0);
	gameStates.render.bFullBright = bFullBright;
	gameData.render.scene.Deactivate ();
	}
}

//------------------------------------------------------------------------------

void RenderSegmentOutline (void)
{
ogl.SetCullMode ((gameStates.render.bRearView < 0) ? GL_BACK : GL_FRONT);
glPolygonMode (GL_BACK, GL_LINE);       // Draw Backfacing Polygons As Wireframes
glLineWidth (4.0);         // Set The Line Width
ogl.SetDepthMode (GL_LEQUAL);            // Change The Depth Mode
glColor3f (0, 0, 0);   
RenderSegmentList (RENDER_TYPE_OUTLINE);
ogl.SetCullMode ((gameStates.render.bRearView < 0) ? GL_FRONT : GL_BACK);
glPolygonMode (GL_BACK, GL_FILL);       // Draw Backfacing Polygons As Wireframes
}

//------------------------------------------------------------------------------
//renders onto current canvas

void RenderMine (int16_t nStartSeg, fix xStereoSeparation, int32_t nWindow)
{
PROF_START
SetupMineRenderer (nWindow);
PROF_END(ptAux)
ComputeMineLighting (nStartSeg, xStereoSeparation, nWindow);
#if 0
++gameStates.render.bFullBright;
RenderSegmentList (RENDER_TYPE_ZCULL);	// render depth only
--gameStates.render.bFullBright;
#endif
#if 0 //DBG
RenderCockpitModel ();
#endif
#if 1
RenderSkyBoxObjects ();
RenderSegmentList (RENDER_TYPE_GEOMETRY);
//RenderSegmentOutline ();
#	if 1
if (!(EGI_FLAG (bShadows, 0, 1, 0) && (gameStates.render.nShadowMap > 0))) {
	if (!gameStates.app.bNostalgia &&
		 (!automap.Active () || gameOpts->render.automap.bCoronas) && gameOpts->render.effects.bEnabled && gameOpts->render.coronas.bUse) 
		if (!nWindow && SetupCoronas ())
			RenderSegmentList (RENDER_TYPE_CORONAS);
	}
#	endif
#endif
gameData.app.nMineRenderCount++;
PROF_END(ptRenderMine);
}

//------------------------------------------------------------------------------
// eof
