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
#include "oglmatrix.h"

//------------------------------------------------------------------------------
//increment counter for checking if points bTransformed

void RenderStartFrame (void)
{
if (!++gameStates.render.nFrameCount)		//wrap!
	gameStates.render.nFrameCount = 1;
}

//------------------------------------------------------------------------------

static void ComputeShadowTransformation (int32_t nLight)
{

	static double biasData [16] = {0.5, 0.0, 0.0, 0.0,
											 0.0, 0.5, 0.0, 0.0,
											 0.0, 0.0, 0.5, 0.0,
											 0.5, 0.5, 0.5, 1.0};

	COGLMatrix	bias, modelView, projection;
	GLint matrixMode;

ogl.SetupTransform (1);
glGetIntegerv (GL_MATRIX_MODE, &matrixMode);
bias = biasData;
modelView.Get (GL_MODELVIEW_MATRIX);
projection.Get (GL_PROJECTION_MATRIX);
ogl.ResetTransform (1);
glMatrixMode (GL_TEXTURE);
glActiveTexture (GL_TEXTURE1 + nLight);
#if 1
bias.Set ();
projection.Mul ();
#else
projection.Set ();
#endif
modelView.Mul ();
transformation.SystemMatrix (nLight).Get (GL_TEXTURE_MATRIX);
glMatrixMode (matrixMode);
}

//------------------------------------------------------------------------------

void SetupRenderView (fix xStereoSeparation, int16_t *nStartSegP, int32_t bOglScale)
{
	int16_t		nStartSeg;
	bool			bPlayer = (gameData.objData.viewerP == LOCALOBJECT);
	CFixMatrix	mView = gameData.objData.viewerP->info.position.mOrient;
	fix			xZoom = gameStates.render.xZoom;


gameData.render.mine.viewer = gameData.objData.viewerP->info.position;
if (LOCALPLAYER.ObservedPlayer () == N_LOCALPLAYER)
	FLIGHTPATH.SetPos (NULL);

if (gameStates.render.cameras.bActive)
	nStartSeg = gameData.objData.viewerP->info.nSegment;
else {
	if (bPlayer) {
		if (xStereoSeparation)
			gameData.render.mine.viewer.vPos += gameData.objData.viewerP->info.position.mOrient.m.dir.r * xStereoSeparation;
#if 0 // is done in the game loop anyway
		if ((LOCALPLAYER.ObservedPlayer () == N_LOCALPLAYER) && !gameStates.render.nWindow [0])
			FLIGHTPATH.Update (gameData.objData.viewerP);
#endif
		if (gameStates.render.bRearView) { // no zoom, no head tracking
			mView.m.dir.f.Neg ();
			mView.m.dir.r.Neg ();
			xZoom = FixDiv (gameStates.render.xZoom, gameStates.zoom.nFactor);
			}
	#if DBG
		else if (gameStates.render.bChaseCam) { // no zoom, no head tracking
	#else
		else if (gameStates.render.bChaseCam && (!IsMultiGame || IsCoopGame || (EGI_FLAG (bEnableCheats, 0, 0, 0) && !COMPETITION))) {
	#endif
			FLIGHTPATH.GetViewPoint ();
			if (FLIGHTPATH.Tail ())
				mView = FLIGHTPATH.Tail ()->mOrient;
			else if (gameData.multiplayer.tAppearing [N_LOCALPLAYER][0] != 0) {
				mView.m.dir.f.Neg ();
				mView.m.dir.r.Neg ();
				}
			}
		else {
			if (transformation.m_info.bUsePlayerHeadAngles) {
				CFixMatrix mHead = CFixMatrix::Create (transformation.m_info.playerHeadAngles);
				mView = gameData.objData.viewerP->info.position.mOrient * mHead;
				}
			if (!IsMultiGame || gameStates.app.bHaveExtraGameInfo [1]) { // zoom?
				if (!(gameStates.zoom.nMinFactor = I2X (gameStates.render.glAspect)))
					gameStates.zoom.nMinFactor = I2X (1);
				gameStates.zoom.nMaxFactor = gameStates.zoom.nMinFactor * 5;
				HandleZoom ();
				xZoom = FixDiv (gameStates.render.xZoom, gameStates.zoom.nFactor);
				}
			}
		}
	if (!nStartSegP)
		nStartSeg = gameStates.render.nStartSeg; // re-use start segment
	else if (0 > (nStartSeg = FindSegByPos (gameData.render.mine.viewer.vPos, gameData.objData.viewerP->info.nSegment, 1, 0)))
		nStartSeg = gameData.objData.viewerP->info.nSegment;
	}
if (nStartSegP)
	*nStartSegP = nStartSeg;

SetupTransformation (transformation, gameData.render.mine.viewer.vPos, mView, xZoom, bOglScale, xStereoSeparation);
#if DBG
{
CFixVector p, m_pos = OBJECT (0)->FrontPosition ();
tScreenPos s;
transformation.TransformAndEncode (p, m_pos);
//SetupCanvasses ();
gameData.render.frame.Activate ("");
ProjectPoint (p, s, 0, 0);
gameData.render.frame.Deactivate ();
}
#endif
CCanvas::Current ()->Reactivate ();
if (gameStates.render.cameras.bActive && (gameStates.render.nShadowMap > 0))
	ComputeShadowTransformation (gameStates.render.nShadowMap - 1);
ogl.SetupTransform (1);
transformation.m_info.oglModelview.Get (GL_MODELVIEW_MATRIX);
ogl.ResetTransform (1);
}

//------------------------------------------------------------------------------

#if DBG
static int32_t bDbgFullBright = -1;
#endif

int32_t BeginRenderMine (int16_t nStartSeg, fix xStereoSeparation, int32_t nWindow)
{
PROF_START
#if DBG
if (bDbgFullBright >= 0)
	gameStates.render.bFullBright = bDbgFullBright;
else if ((gameStates.app.bEndLevelSequence == EL_FLYTHROUGH) || (gameStates.app.bEndLevelSequence == EL_LOOKBACK))
	gameStates.render.bFullBright = 2;
else if (automap.Active () && gameOpts->render.automap.bBright) 
	gameStates.render.bFullBright = 1;
else
	gameStates.render.bFullBright = 0;
#else
gameStates.render.bFullBright = (automap.Active () && gameOpts->render.automap.bBright)
#if MAX_SHADOWMAPS
										  || (gameStates.render.nShadowMap > 0)
#endif
										  ;
#endif
ogl.m_states.bStandardContrast = gameStates.app.bNostalgia || IsMultiGame || (ogl.m_states.nContrast == 8);
ogl.m_states.bScaleLight = EGI_FLAG (bShadows, 0, 1, 0) && (gameStates.render.nShadowPass < 3) && !FAST_SHADOWS;
gameStates.render.bUseCameras = USE_CAMERAS;

if (!nWindow)
	GetPlayerMslLock ();	// uses rendered object info from previous frame stored in windowRenderedData
if (!gameStates.render.cameras.bActive)
	windowRenderedData [nWindow].nObjects = 0;
ogl.m_states.fAlpha = FADE_LEVELS;
if (((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2) && (gameStates.render.nShadowBlurPass < 2)) || gameStates.render.nShadowMap) {
	if (!automap.Active ())
		RenderStartFrame ();
	}
if ((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2)) {
	ogl.SetTransform (1);
	BuildRenderSegList (nStartSeg, nWindow);		//fills in gameData.render.mine.renderSegList & gameData.render.mine.visibility [0].nSegments
	if ((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2)) {
		BuildRenderObjLists (gameData.render.mine.visibility [0].nSegments);
		if (xStereoSeparation <= 0)	// Do for left eye or zero.
			SetDynamicLight ();
		}
	ogl.SetTransform (0);
	lightManager.Transform (0, 1);
	}
PROF_END(ptAux);
return !gameStates.render.cameras.bActive && (gameData.objData.viewerP->info.nType != OBJ_ROBOT);
}

//------------------------------------------------------------------------------

void SetupMineRenderer (int32_t nWindow)
{
#if DBG
if (gameStates.app.bNostalgia) {
	gameOptions [1].render.debug.bWireFrame = 0;
	gameOptions [1].render.debug.bTextures = 1;
	gameOptions [1].render.debug.bObjects = 1;
	gameOptions [1].render.debug.bWalls = 1;
	gameOptions [1].render.debug.bDynamicLight = 1;
	}
#endif

#if 0
if (gameStates.app.bNostalgia > 1)
	gameStates.render.nLightingMethod =
	gameStates.render.bPerPixelLighting = 0;
else if (!(lightmapManager.HaveLightmaps ()))
	gameStates.render.bPerPixelLighting = 0;
else {
	if (gameStates.render.nLightingMethod == 2)
		gameStates.render.bPerPixelLighting = 2;
	else if ((gameStates.render.nLightingMethod == 1) && gameOpts->render.bUseLightmaps)
		gameStates.render.bPerPixelLighting = 1;
	else
		gameStates.render.bPerPixelLighting = 0;
	}
#endif

if ((nWindow == 0) && (gameStates.render.nShadowPass < 2)) {
	ogl.m_states.bDepthBuffer [0] =
	ogl.m_states.bDepthBuffer [1] = 0;
	ogl.SetDepthWrite (true);
	}
gameData.render.nUsedFaces =
gameData.render.nTotalFaces =
gameData.render.nTotalObjects =
gameData.render.nTotalSprites =
gameData.render.nTotalLights =
gameData.render.nMaxLights =
gameData.render.nStateChanges =
gameData.render.nShaderChanges = 0;
gameData.render.fBrightness = paletteManager.Brightness ();
if (ogl.IsAnaglyphDevice () && (!gameOpts->app.bExpertMode || gameOpts->render.stereo.bBrighten))
	gameData.render.fBrightness *= 1.25f;

SetFaceDrawer (-1);
gameData.render.vertColor.bNoShadow = !FAST_SHADOWS && (gameStates.render.nShadowPass == 4);
gameData.render.vertColor.bDarkness = IsMultiGame && gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [IsMultiGame].bDarkness;
gameStates.render.bApplyDynLight =
gameStates.render.bUseDynLight = SHOW_DYN_LIGHT;
if (!EGI_FLAG (bPowerupLights, 0, 0, 0))
	gameData.render.nPowerupFilter = 0;
else if (gameStates.render.bPerPixelLighting == 2)
	gameData.render.nPowerupFilter = 1;
else
	gameData.render.nPowerupFilter = 2;
gameStates.render.bDoCameras = extraGameInfo [0].bUseCameras &&
									    (!IsMultiGame || (gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [1].bUseCameras)) &&
										 !gameStates.render.cameras.bActive;
gameStates.render.bDoLightmaps = 0;
}

//------------------------------------------------------------------------------
// Always needs to be executed, since it resets the face list and sets segment visibility

void ComputeMineLighting (int16_t nStartSeg, fix xStereoSeparation, int32_t nWindow)
{
PROF_START
ogl.m_states.fLightRange = fLightRanges [IsMultiGame ? 1 : extraGameInfo [IsMultiGame].nLightRange];
if ((gameStates.render.nRenderPass <= 0) && (gameStates.render.nShadowPass < 2)) {
	gameData.render.mine.bSetAutomapVisited = BeginRenderMine (nStartSeg, xStereoSeparation, nWindow);

	if (!ogl.StereoSeparation () || gameStates.render.Dirty ()) {
		ResetFaceList ();
		gameStates.render.nThreads = gameStates.app.nThreads;
		lightManager.ResetSegmentLights ();
		lightManager.ResetIndex ();
		if ((gameStates.render.bPerPixelLighting == 2) || (CountRenderFaces () < 16) || (gameStates.app.nThreads < 2)
#	if !USE_OPENMP
			 || !RunRenderThreads (rtComputeFaceLight, gameStates.app.nThreads)
#	endif
			)
			{
			gameStates.render.nThreads = 1;
			if (gameStates.render.bTriangleMesh || !gameStates.render.bApplyDynLight || (gameData.render.mine.visibility [0].nSegments < gameData.segData.nSegments))
				ComputeFaceLight (0, gameData.render.mine.visibility [0].nSegments, 0);
			else if (gameStates.app.bEndLevelSequence < EL_OUTSIDE)
				ComputeFaceLight (0, FACES.nFaces, 0);
			else
				ComputeFaceLight (0, gameData.segData.nSegments, 0);
			}
#if USE_OPENMP //> 1
		else {
				int32_t	nStart, nEnd, nMax;

			if (gameStates.render.bTriangleMesh || !gameStates.render.bApplyDynLight || (gameData.render.mine.visibility [0].nSegments < gameData.segData.nSegments))
				nMax = gameData.render.mine.visibility [0].nSegments;
			else if (gameStates.app.bEndLevelSequence < EL_OUTSIDE)
				nMax = FACES.nFaces;
			else
				nMax = gameData.segData.nSegments;
			if (gameStates.app.nThreads & 1) {
#			pragma omp parallel for private (nStart, nEnd)
				for (int32_t i = 0; i < gameStates.app.nThreads; i++) {
					ComputeThreadRange (i, nMax, nStart, nEnd);
					ComputeFaceLight (nStart, nEnd, i);
					}
				}
			else {
				int32_t	nPivot = gameStates.app.nThreads / 2;
#				pragma omp parallel for private (nStart, nEnd)
				for (int32_t i = 0; i < gameStates.app.nThreads; i++) {
					if (i < nPivot) {
						ComputeThreadRange (i, tiRender.nMiddle, nStart, nEnd, nPivot);
						ComputeFaceLight (nStart, nEnd, i);
						}
					else {
						ComputeThreadRange (i - nPivot, nMax - tiRender.nMiddle, nStart, nEnd, nPivot);
						ComputeFaceLight (nStart + tiRender.nMiddle, nEnd + tiRender.nMiddle, i);
						}
					}
				}
			}
#endif //_OPENMP
		if ((gameStates.render.bPerPixelLighting == 2) && !gameData.app.nFrameCount)
			meshBuilder.BuildVBOs ();
		gameStates.render.bHeadlights = gameOpts->ogl.bHeadlight && lightManager.Headlights ().nLights && !(gameStates.render.bFullBright || automap.Active ());
		}
	transparencyRenderer.InitBuffer (gameData.render.zMin, gameData.render.zMax, nWindow);
	}
PROF_END(ptLighting)
}

//------------------------------------------------------------------------------
// eof
