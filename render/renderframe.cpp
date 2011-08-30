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

/*
 *
 * Stuff for rendering the HUD
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "descent.h"
#include "error.h"
#include "rendermine.h"
#include "screens.h"
#include "console.h"
#include "cockpit.h"
#include "gamefont.h"
#include "newdemo.h"
#include "text.h"
#include "textdata.h"
#include "gr.h"
#include "ogl_render.h"
#include "endlevel.h"
#include "playerprofile.h"
#include "automap.h"
#include "gamepal.h"
#include "visibility.h"
#include "lightning.h"
#include "rendershadows.h"
#include "transprender.h"
#include "radar.h"
#include "menubackground.h"
#include "addon_bitmaps.h"
#include "createmesh.h"

#define bSavingMovieFrames 0

void StartLightingFrame (CObject *viewer);

uint	nClearWindowColor = 0;

//------------------------------------------------------------------------------

static inline bool GuidedMissileActive (void)
{
CObject *gmObjP = gameData.objs.guidedMissile [gameData.multiplayer.nLocalPlayer].objP;
return gmObjP &&
		 (gmObjP->info.nType == OBJ_WEAPON) &&
		 (gmObjP->info.nId == GUIDEDMSL_ID) &&
		 (gmObjP->info.nSignature == gameData.objs.guidedMissile [gameData.multiplayer.nLocalPlayer].nSignature);
}

//------------------------------------------------------------------------------

//draw a crosshair for the guided missile
void DrawGuidedCrosshair (fix xStereoSeparation)
{
CCanvas::Current ()->SetColorRGBi (RGB_PAL (0, 31, 0));
int w = CCanvas::Current ()->Width () >> 5;
if (w < 5)
	w = 5;
int h = I2X (w) / screen.Aspect ();
int x = CCanvas::Current ()->Width () / 2;
if (xStereoSeparation) {
	ogl.ColorMask (1,1,1,1,1);
	x -= int (float (x / 32) * X2F (xStereoSeparation));
	}
int y = CCanvas::Current ()->Height () / 2;
glLineWidth (float (screen.Width ()) / 640.0f);
#if 1
	w /= 2;
	h /= 2;
	OglDrawLine (x - w, y, x + w, y);
	OglDrawLine (x, y - h, x, y + h);
#else
	OglDrawLine (I2X (x-w/2), I2X (y), I2X (x+w/2), I2X (y));
	OglDrawLine (I2X (x), I2X (y-h/2), I2X (x), I2X (y+h/2));
#endif
glLineWidth (1.0f);
if (xStereoSeparation) {
	ogl.ColorMask (1,1,1,1,0);
	}
}

//------------------------------------------------------------------------------

void DrawScope (void)
{
if (scope.Load ()) {
	float sh = float (screen.Height ());
	float ch = float (CCanvas::Current ()->Height ());
	float w = 0.25f * float (CCanvas::Current ()->Width ()) / ch;
	float y = 1.0f - float (CCanvas::Current ()->Top ()) / sh;
	float h = ch / sh;

	ogl.SetTexturing (true);
	ogl.SetBlending (true);
	ogl.SetBlendMode (OGL_BLEND_ALPHA);
	ogl.SetDepthTest (false);
	if (scope.Bind (1))
		return;
	scope.Texture ()->Wrap (GL_REPEAT);
	glColor3f (1.0f, 1.0f, 1.0f);
	glBegin (GL_QUADS);
	glTexCoord2f (0.5f - w, 0.25f);
	glVertex2f (0, y);
	glTexCoord2f (0.5f + w, 0.25f);
	glVertex2f (1, y);
	glTexCoord2f (0.5f + w, 0.75f);
	glVertex2f (1, y - h);
	glTexCoord2f (0.5f - w, 0.75f);
	glVertex2f (0, y - h);
	glEnd ();
	ogl.BindTexture (0);
	ogl.SetDepthTest (true);
	ogl.SetTexturing (false);
	glPopMatrix ();
	}
}

//------------------------------------------------------------------------------

//draw a crosshair for the zoom
void DrawZoomCrosshair (void)
{
DrawScope ();

	static tSinCosf sinCos [124];
	static int bInitSinCos = 1;

if (bInitSinCos) {
	ComputeSinCosTable (sizeofa (sinCos), sinCos);
	bInitSinCos = 0;
	}

	int bHaveTarget = TargetInLineOfFire ();
	int sh = screen.Height ();
	int ch = CCanvas::Current ()->Height ();
	int cw = CCanvas::Current ()->Width ();
	int h = ch >> 2;
	int w = X2I (h * screen.Aspect ());
	int x = cw / 2;
	int y = ch / 2;
	int left = x - w, right = x + w, top = y - h, bottom = y + h;
	float xStep = float (2 * w + 1) / 12.0f;
	//float yStep = float (2 * h + 1) / 12.0f;
	float xScale = float (w + (w >> 1)) / float (cw);
	float yScale = float (h + (h >> 1)) / float (sh);
	float	x1, y1;
	int	i;

w >>= 4;
h >>= 4;
//w += w >> 1;
//h += h >> 1;

glLineWidth (float (cw) / 640.0f);

if (bHaveTarget)
	CCanvas::Current ()->SetColorRGBi (RGBA_PAL (39, 0, 0, 128));
else
	CCanvas::Current ()->SetColorRGBi (RGBA_PAL (0, 39, 0, 128));
for (i = 0, x1 = float (left); i < 11; i++) {
	x1 += xStep;
	if (i != 5)
		OglDrawLine (int (x1 + 0.5f), y - h, int (x1 + 0.5f), y + h);
	}

for (i = 0, y1 = float (top); i < 11; i++) {
	y1 += xStep;
	if (i != 5)
		OglDrawLine (x - w, int (y1 + 0.5f), x + w, int (y1 + 0.5f));
	}

if (bHaveTarget)
	CCanvas::Current ()->SetColorRGBi (RGBA_PAL (63, 0, 0, 160));
else
	CCanvas::Current ()->SetColorRGBi (RGBA_PAL (0, 63, 0, 160));

glLineWidth (float (floor (2 * float (cw) / 640.0f)));

glPushMatrix ();
ogl.SetLineSmooth (true);
glTranslatef (0.5f, 1.0f - float (CCanvas::Current ()->Top () + y) / float (screen.Height ()), 0.0f);
glScalef (xScale, yScale, 1.0f);
#if 0
float fh = 2.0f * float (h) / float (sh);
float fw = 2.0f * float (w) / float (cw);
#endif
glBegin (GL_LINES);
glVertex2f (0.0f, -1.0f);
glVertex2f (0.0f, -1.125f);
glVertex2f (sinCos [15].fCos, sinCos [15].fSin);
glVertex2f (sinCos [15].fCos * 1.0625f, sinCos [15].fSin * 1.0625f);
glVertex2f (0.0f, 1.0f);
glVertex2f (0.0f, 1.125f);
glVertex2f (sinCos [46].fCos, sinCos [46].fSin);
glVertex2f (sinCos [46].fCos * 1.0625f, sinCos [46].fSin * 1.0625f);
glVertex2f (-1.125f, 0.0f);
glVertex2f (-1.0f, 0.0f);
glVertex2f (sinCos [77].fCos, sinCos [77].fSin);
glVertex2f (sinCos [77].fCos * 1.0625f, sinCos [77].fSin * 1.0625f);
glVertex2f (1.125f, 0.0f);
glVertex2f (1.0f, 0.0f);
glVertex2f (sinCos [108].fCos, sinCos [108].fSin);
glVertex2f (sinCos [108].fCos * 1.0625f, sinCos [108].fSin * 1.0625f);
glEnd ();
glPopMatrix ();

w += w >> 1;
h += h >> 1;
w <<= 1;
h <<= 1;

glLineWidth (float (cw) / 640.0f);

OglDrawLine (left, y, right, y);
OglDrawLine (x, top, x, bottom);
OglDrawLine (left, y - h, left, y + h);
OglDrawLine (right, y - h, right, y + h);
OglDrawLine (x - w, top, x + w, top);
OglDrawLine (x - w, bottom, x + w, bottom);
glLineWidth (1.0f);

w >>= 1;
h >>= 1;
w -= w >> 1;
h -= h >> 1;
glLineWidth (float (floor (2 * float (cw) / 640.0f)));
#if 1
//float xScale = float (w << 5) / float (cw);
//float yScale = float (h << 5) / float (screen.Height ());
OglDrawEllipse (sizeofa (sinCos), GL_LINE_LOOP, xScale, 0.5f, yScale, 1.0f - float (CCanvas::Current ()->Top () + y) / float (screen.Height ()), sinCos);
#else
glPushMatrix ();
ogl.SetLineSmooth (true);
if (bHaveTarget)
	glColor4f (1.0f, 0.0f, 0.0f, 0.25f);
else
	glColor4f (0.0f, 1.0f, 0.0f, 0.25f);
glTranslatef (0.5f, 0.5f, 0.5f);
glScalef (float (w << 5) / float (cw), float (h << 5) / float (ch), 0.1f);
OglDrawEllipse (sizeofa (sinCos), GL_LINE_LOOP, 1.0f, 0, 1.0f, 0, sinCos);
ogl.SetLineSmooth (false);
glPopMatrix ();
#endif

char	szZoom [20];
int	r, aw;
if (extraGameInfo [IsMultiGame].nZoomMode == 2)
	r = int (100.0f * gameStates.zoom.nFactor / float (gameStates.zoom.nMinFactor));
else {
	float s = float (pow (float (gameStates.zoom.nMaxFactor) / float (gameStates.zoom.nMinFactor), 0.25f));
	fix f = gameStates.zoom.nMinFactor;
	for (r = 1; f < fix (gameStates.zoom.nFactor); r++)
		f = fix (float (f) * s + 0.5f);
	r *= 100;
	}
sprintf (szZoom, "X %d.%02d", r / 100, r % 100);
fontManager.Current ()->StringSize (szZoom, w, h, aw);
if (bHaveTarget)
	fontManager.SetColorRGBi (RED_RGBA, 1, 0, 0);
else
	fontManager.SetColorRGBi (GREEN_RGBA, 1, 0, 0);
GrPrintF (NULL, x - w  / 2, bottom + h, szZoom);
}

//------------------------------------------------------------------------------

int RenderMissileView (void)
{
	CObject	*objP = NULL;

if (GuidedMslView (&objP)) {
	if (gameOpts->render.cockpit.bGuidedInMainView) {
		gameStates.render.nRenderingType = 6 + (1 << 4);
		cockpit->RenderWindow (1, gameData.objs.viewerP, 0, WBUMSL, "SHIP");
		}
	else {
		gameStates.render.nRenderingType = 1+ (1 << 4);
		cockpit->RenderWindow (1, objP, 0, WBU_GUIDED, "GUIDED");
	   }
	return 1;
	}
else {
	if (objP) {		//used to be active
		if (!gameOpts->render.cockpit.bGuidedInMainView)
			cockpit->RenderWindow (1, NULL, 0, WBU_STATIC, NULL);
		gameData.objs.guidedMissile [gameData.multiplayer.nLocalPlayer].objP = NULL;
		}
	if (gameData.objs.missileViewerP && !gameStates.render.bChaseCam) {		//do missile view
		static int mslViewerSig = -1;
		if (mslViewerSig == -1)
			mslViewerSig = gameData.objs.missileViewerP->info.nSignature;
		if (gameOpts->render.cockpit.bMissileView &&
			 (gameData.objs.missileViewerP->info.nType != OBJ_NONE) &&
			 (gameData.objs.missileViewerP->info.nSignature == mslViewerSig)) {
			//HUDMessage (0, "missile view");
  			gameStates.render.nRenderingType = 2 + (1 << 4);
			cockpit->RenderWindow (1, gameData.objs.missileViewerP, 0, WBUMSL, "MISSILE");
			return 1;
			}
		else {
			gameData.objs.missileViewerP = NULL;
			mslViewerSig = -1;
			gameStates.render.nRenderingType = 255;
			cockpit->RenderWindow (1, NULL, 0, WBU_STATIC, NULL);
			}
		}
	}
return 0;
}

//------------------------------------------------------------------------------

void FlashMine (void)
{
if (gameOpts->app.bEpilepticFriendly ||
	 !(/*extraGameInfo [0].bFlickerLights &&*/ gameStates.render.nFlashScale && (gameStates.render.nFlashScale != I2X (1))))
	return;

ogl.SetBlendMode (OGL_BLEND_ALPHA);
glColor4f (0, 0, 0, /*1.0f -*/ 3 * X2F (gameStates.render.nFlashScale) / 4);
ogl.SetTexturing (false);
ogl.SetDepthTest (false);
ogl.RenderScreenQuad ();
ogl.SetDepthTest (true);
}

//------------------------------------------------------------------------------

void Draw2DFrameElements (void)
{
//if (gameStates.render.bRenderIndirect > 0)
	ogl.SetDrawBuffer (GL_BACK, 0);
fix xStereoSeparation = ogl.StereoSeparation ();
ogl.SetStereoSeparation (0);
ogl.ColorMask (1,1,1,1,0);
//SetBlendMode (OGL_BLEND_ALPHA);
if (gameStates.app.bGameRunning && !automap.Display ()) {
	PROF_START
	cockpit->Render (!(gameOpts->render.cockpit.bGuidedInMainView && GuidedMissileActive ()), 0);
	PROF_END(ptCockpit)
	}
paletteManager.RenderEffect ();
console.Draw ();
FlashMine ();
ogl.SetStereoSeparation (xStereoSeparation);
}

//------------------------------------------------------------------------------

void FlushFrame (fix xStereoSeparation)
{
if (!(gameOpts->render.stereo.nGlasses && xStereoSeparation)) {	//no stereo or shutter glasses
	if (gameStates.render.bRenderIndirect <= 0)
		Draw2DFrameElements ();
	ogl.SwapBuffers (0, 0);
	}
else {
	int i = ogl.Enhance3D ();
	if (i < 0)
		Draw2DFrameElements ();
	if (i) {
		if (xStereoSeparation > 0) {
#if 0
			if (gameStates.menus.nInMenu) {
				gameStates.render.bRenderIndirect = 0;
				Draw2DFrameElements ();
				gameStates.render.bRenderIndirect = 1;
				}
			else
#endif
				ogl.SwapBuffers (0, 0);
			}
		}
	else {
		if (xStereoSeparation < 0) {
			//glFlush ();
			ogl.ColorMask (1,1,1,1,0);
			}
		else {
			//glFlush ();
			ogl.ColorMask (1,1,1,1,0);
			Draw2DFrameElements ();
			ogl.SwapBuffers (0, 0);
			}
		}
	}
}

//------------------------------------------------------------------------------

#if MAX_SHADOWMAPS

static int RenderShadowMap (CDynLight* lightP, int nLight, fix xStereoSeparation)
{
if (!lightP)
	return 0;

	CCamera* cameraP = cameraManager.ShadowMap (nLight);

if (!(cameraP || (cameraP = cameraManager.AddShadowMap (nLight, lightP)))) 
	return 0;

if (cameraP->HaveBuffer (0))
	cameraP->Setup (cameraP->Id (), lightP->info.nSegment, lightP->info.nSide, -1, -1, (lightP->info.nObject < 0) ? NULL : OBJECTS + lightP->info.nObject, 0);
else if (!cameraP->Create (cameraManager.Count () - 1, lightP->info.nSegment, lightP->info.nSide, -1, -1, (lightP->info.nObject < 0) ? NULL : OBJECTS + lightP->info.nObject, 1, 0)) {
	cameraManager.DestroyShadowMap (nLight);
	return 0;
	}
CCamera* current = cameraManager [cameraManager.Current ()];
cameraP = cameraManager.ShadowMap (nLight);
cameraManager.SetCurrent (cameraP);
cameraP->Render ();
cameraManager.SetCurrent (current);
return 1;
}

//------------------------------------------------------------------------------

static void RenderShadowMaps (fix xStereoSeparation)
{
if (EGI_FLAG (bShadows, 0, 1, 0)) {
	lightManager.ResetActive (1, 0);
	short nSegment = OBJSEG (gameData.objs.viewerP);
	lightManager.ResetNearestStatic (nSegment, 1);
	lightManager.SetNearestStatic (nSegment, 1, 1);
	CDynLightIndex* sliP = &lightManager.Index (0,1);
	CActiveDynLight* activeLightsP = lightManager.Active (1) + sliP->nFirst;
	int nLights = 0, h = (sliP->nActive < abs (MAX_SHADOWMAPS)) ? sliP->nActive : abs (MAX_SHADOWMAPS);
	for (gameStates.render.nShadowMap = 1; gameStates.render.nShadowMap <= h; gameStates.render.nShadowMap++) 
		nLights += RenderShadowMap (lightManager.GetActive (activeLightsP, 1), nLights, xStereoSeparation);
	lightManager.SetLightCount (nLights, 2);
	gameStates.render.nShadowMap = 0;
	}
}

#endif

//------------------------------------------------------------------------------

extern CBitmap bmBackground;

void RenderFrame (fix xStereoSeparation, int nWindow)
{
	short nStartSeg;
	fix	nEyeOffsetSave = gameStates.render.xStereoSeparation;

gameStates.render.nWindow = nWindow;
gameStates.render.xStereoSeparation = xStereoSeparation;
if (gameStates.app.bEndLevelSequence) {
	RenderEndLevelFrame (xStereoSeparation, nWindow);
	gameData.app.nFrameCount++;
	gameStates.render.xStereoSeparation = nEyeOffsetSave;
	return;
	}
if ((gameData.demo.nState == ND_STATE_RECORDING) && (xStereoSeparation >= 0)) {
   if (!gameStates.render.nRenderingType)
   	NDRecordStartFrame (gameData.app.nFrameCount, gameData.time.xFrame);
   if (gameStates.render.nRenderingType != 255)
   	NDRecordViewerObject (gameData.objs.viewerP);
	}

StartLightingFrame (gameData.objs.viewerP);		//this is for ugly light-smoothing hack
ogl.m_states.bEnableScissor = !gameStates.render.cameras.bActive && nWindow;
if (!nWindow)
	gameData.render.dAspect = (double) CCanvas::Current ()->Width () / (double) CCanvas::Current ()->Height ();

{
PROF_START
G3StartFrame (0, !(nWindow || gameStates.render.cameras.bActive), xStereoSeparation);
SetRenderView (xStereoSeparation, &nStartSeg, 1);
transformation.ComputeFrustum ();
#if MAX_SHADOWMAPS
if (!(nWindow || gameStates.render.cameras.bActive)) {
	ogl.SetupTransform (1);
	transformation.SystemMatrix (-1).Get (GL_MODELVIEW_MATRIX, true); // inverse
	transformation.SystemMatrix (-2).Get (GL_PROJECTION_MATRIX, true); 
	transformation.SystemMatrix (-3).Get (GL_PROJECTION_MATRIX, false);
	ogl.ResetTransform (1);
	glPushMatrix ();
	transformation.SystemMatrix (-1).Set ();
	transformation.SystemMatrix (-2).Mul ();
	transformation.SystemMatrix (-3).Get (GL_MODELVIEW_MATRIX, false); // inverse (modelview * projection)
	glPopMatrix ();
	}
#endif
PROF_END(ptAux)
}

#if 0 //DBG
if (gameStates.render.nShadowMap) {
	G3EndFrame (nWindow);
	gameStates.render.xStereoSeparation = nEyeOffsetSave;
	return;
	}
#endif

if (0 > (gameStates.render.nStartSeg = nStartSeg)) {
	G3EndFrame (nWindow);
	gameStates.render.xStereoSeparation = nEyeOffsetSave;
	return;
	}

#if DBG
if (bShowOnlyCurSide)
	CCanvas::Current ()->Clear (nClearWindowColor);
#endif

#if MAX_SHADOWMAPS
RenderMine (nStartSeg, xStereoSeparation, nWindow);
#else
if (!ogl.m_features.bStencilBuffer.Available ())
	extraGameInfo [0].bShadows =
	extraGameInfo [1].bShadows = 0;
if (SHOW_SHADOWS &&
	 !(nWindow || gameStates.render.cameras.bActive || automap.Display ())) {
	if (!gameStates.render.nShadowMap) {
		gameStates.render.nShadowPass = 1;
#if SOFT_SHADOWS
		if (gameOpts->render.shadows.bSoft = 1)
			gameStates.render.nShadowBlurPass = 1;
#endif
		ogl.StartFrame (0, 0, xStereoSeparation);
#if SOFT_SHADOWS
		ogl.Viewport (CCanvas::Current ()->props.x, CCanvas::Current ()->props.y, 128, 128);
#endif
		RenderMine (nStartSeg, xStereoSeparation, nWindow);
		PROF_START
		RenderFastShadows (xStereoSeparation, nWindow, nStartSeg);
		PROF_END(ptEffects)
		if (FAST_SHADOWS)
			;//RenderFastShadows (xStereoSeparation, nWindow, nStartSeg);
		else {
			PROF_START
			RenderNeatShadows (xStereoSeparation, nWindow, nStartSeg);
			PROF_END(ptEffects)
			}
#if SOFT_SHADOWS
		if (gameOpts->render.shadows.bSoft) {
			PROF_START
			CreateShadowTexture ();
			PROF_END(ptEffects)
			gameStates.render.nShadowBlurPass = 2;
			gameStates.render.nShadowPass = 0;
			ogl.StartFrame (0, 1, xStereoSeparation);
			SetRenderView (xStereoSeparation, &nStartSeg, 1);
			RenderMine (nStartSeg, xStereoSeparation, nWindow);
			RenderShadowTexture ();
			}
#endif
		nWindow = 0;
		}
	}
else {
	if (gameStates.render.nRenderPass < 0)
		RenderMine (nStartSeg, xStereoSeparation, nWindow);
	else {
		for (gameStates.render.nRenderPass = 0; gameStates.render.nRenderPass < 2; gameStates.render.nRenderPass++) {
			ogl.StartFrame (0, 1, xStereoSeparation);
			RenderMine (nStartSeg, xStereoSeparation, nWindow);
			}
		}
	}
ogl.StencilOff ();
#endif
RenderSkyBox (nWindow);
RenderEffects (nWindow);
if (!(nWindow || gameStates.render.cameras.bActive || gameStates.app.bEndLevelSequence || GuidedInMainView ())) {
	radar.Render ();
	}
#if 0
if (transformation.m_info.bUsePlayerHeadAngles)
	Draw3DReticle (xStereoSeparation);
#endif
gameStates.render.nShadowPass = 0;
//PrintLog (1, "G3EndFrame\n");
G3EndFrame (nWindow);
if (nWindow)
	ogl.SetStereoSeparation (gameStates.render.xStereoSeparation = nEyeOffsetSave);
if (!ShowGameMessage (gameData.messages, -1, -1))
	ShowGameMessage (gameData.messages + 1, -1, -1);
}

//------------------------------------------------------------------------------

void RenderMonoFrame (fix xStereoSeparation = 0)
{
	CCanvas		frameWindow;
	int			bExtraInfo = 1;

#if MAX_SHADOWMAPS
RenderShadowMaps (xStereoSeparation);
#endif
gameStates.render.vr.buffers.screenPages [0].SetupPane (
	&frameWindow,
	gameStates.render.vr.buffers.subRender [0].Left (),
	gameStates.render.vr.buffers.subRender [0].Top (),
	gameStates.render.vr.buffers.subRender [0].Width (),
	gameStates.render.vr.buffers.subRender [0].Height ());

CCanvas::SetCurrent (&gameStates.render.vr.buffers.subRender [0]);

if (xStereoSeparation <= 0) {
	PROF_START
	SEM_ENTER (SEM_LIGHTNING)
	bool bRetry;
	do {
		bRetry = false;
		try {
			lightningManager.SetLights ();
			}
		catch(...) {
			bRetry = true;
			}
		} while (bRetry);
	SEM_LEAVE (SEM_LIGHTNING)
	PROF_END(ptLighting)
	}

if (gameOpts->render.cockpit.bGuidedInMainView && GuidedMissileActive ()) {
	int w, h, aw;
	const char *msg = "Guided Missile View";
	CObject *viewerSave = gameData.objs.viewerP;

   if (gameStates.render.cockpit.nType == CM_FULL_COCKPIT) {
		gameStates.render.cockpit.bBigWindowSwitch = 1;
		gameStates.render.cockpit.bRedraw = 1;
		cockpit->Activate (CM_STATUS_BAR);
		return;
		}
  	gameData.objs.viewerP = gameData.objs.guidedMissile [gameData.multiplayer.nLocalPlayer].objP;
	UpdateRenderedData (0, gameData.objs.viewerP, 0, 0);
	if ((xStereoSeparation <= 0) && cameraManager.Render ())
		CCanvas::SetCurrent (&gameStates.render.vr.buffers.subRender [0]);
	RenderFrame (xStereoSeparation, 0);
	if (xStereoSeparation <= 0)
  		WakeupRenderedObjects (gameData.objs.viewerP, 0);
	gameData.objs.viewerP = viewerSave;
	fontManager.SetCurrent (GAME_FONT);    //GAME_FONT);
	fontManager.SetColorRGBi (RED_RGBA, 1, 0, 0);
	fontManager.Current ()->StringSize (msg, w, h, aw);
	GrPrintF (NULL, (CCanvas::Current ()->Width () - w) / 2, 3, msg);
	//DrawGuidedCrosshair (xStereoSeparation);
	HUDRenderMessageFrame ();
	bExtraInfo = 0;
	}
else {
	if (gameStates.render.cockpit.bBigWindowSwitch) {
		gameStates.render.cockpit.bRedraw = 1;
		cockpit->Activate (CM_FULL_COCKPIT);
		gameStates.render.cockpit.bBigWindowSwitch = 0;
		return;
		}
	UpdateRenderedData (0, gameData.objs.viewerP, gameStates.render.bRearView, 0);
	if ((xStereoSeparation <= 0) && cameraManager.Render ())
		CCanvas::SetCurrent (&gameStates.render.vr.buffers.subRender [0]);
	RenderFrame (xStereoSeparation, 0);
	}
CCanvas::SetCurrent (&gameStates.render.vr.buffers.subRender [0]);
FlushFrame (xStereoSeparation);
}

//------------------------------------------------------------------------------

#define WINDOW_W_DELTA	 ((gameData.render.window.wMax / 16) & ~1)	//24	//20
#define WINDOW_H_DELTA	 ((gameData.render.window.hMax / 16) & ~1)	//12	//10

#define WINDOW_MIN_W		 ((gameData.render.window.wMax * 10) / 22)	//160
#define WINDOW_MIN_H		 ((gameData.render.window.hMax * 10) / 22)

void GrowWindow (void)
{
StopTime ();
if (gameStates.render.cockpit.nType == CM_FULL_COCKPIT) {
	gameData.render.window.h = gameData.render.window.hMax;
	gameData.render.window.w = gameData.render.window.wMax;
	cockpit->Toggle ();
	HUDInitMessage (TXT_COCKPIT_F3);
	StartTime (0);
	return;
	}

if (gameStates.render.cockpit.nType != CM_STATUS_BAR && (gameStates.render.vr.nScreenFlags & VRF_ALLOW_COCKPIT)) {
	StartTime (0);
	return;
	}

if (gameData.render.window.h>=gameData.render.window.hMax || gameData.render.window.w>=gameData.render.window.wMax) {
	//gameData.render.window.w = gameData.render.window.wMax;
	//gameData.render.window[HA] = gameData.render.window.hMax;
	cockpit->Activate (CM_FULL_SCREEN);
	}
else {
	//int x, y;
	gameData.render.window.w += WINDOW_W_DELTA;
	gameData.render.window.h += WINDOW_H_DELTA;
	if (gameData.render.window.h > gameData.render.window.hMax)
		gameData.render.window.h = gameData.render.window.hMax;
	if (gameData.render.window.w > gameData.render.window.wMax)
		gameData.render.window.w = gameData.render.window.wMax;
	gameData.render.window.x = (gameData.render.window.wMax - gameData.render.window.w)/2;
	gameData.render.window.y = (gameData.render.window.hMax - gameData.render.window.h)/2;
	GameInitRenderSubBuffers (gameData.render.window.x, gameData.render.window.y, gameData.render.window.w, gameData.render.window.h);
	}
HUDClearMessages ();	//	@mk, 11/11/94
SavePlayerProfile ();
StartTime (0);
}

//------------------------------------------------------------------------------

extern CBitmap bmBackground;

void CopyBackgroundRect (int left, int top, int right, int bot)
{
if (right < left || bot < top)
	return;

	int x, y;
	int tileLeft, tileRight, tileTop, tileBot;
	int xOffs, yOffs;
	int xDest, yDest;

tileLeft = left / bmBackground.Width ();
tileRight = right / bmBackground.Width ();
tileTop = top / bmBackground.Height ();
tileBot = bot / bmBackground.Height ();

yOffs = top % bmBackground.Height ();
yDest = top;

for (y = tileTop;y <= tileBot; y++) {
	xOffs = left % bmBackground.Width ();
	xDest = left;
	int h = min (bot - yDest + 1, bmBackground.Height () - yOffs);
	for (x = tileLeft; x <= tileRight; x++) {
		int w = min (right - xDest + 1, bmBackground.Width () - xOffs);
		bmBackground.Blit (CCanvas::Current (), xDest, yDest, w, h, xOffs, yOffs, 1);
		xOffs = 0;
		xDest += w;
		}
	yOffs = 0;
	yDest += h;
	}
}

//------------------------------------------------------------------------------
//fills int the background surrounding the 3d window
void FillBackground (void)
{
if (gameData.render.window.x || gameData.render.window.y) {
	CCanvas::Push ();
	CCanvas::SetCurrent (CurrentGameScreen ());
	CViewport viewport = ogl.m_states.viewport [0];
	ogl.m_states.viewport [0] = CViewport (0, 0, CCanvas::Current ()->Width (), CCanvas::Current ()->Height ());
	ogl.SetDepthWrite (false);
	bmBackground.Render (CCanvas::Current (), 0, 0, CCanvas::Current ()->Width (), CCanvas::Current ()->Height (), 0, 0, -bmBackground.Width (), -bmBackground.Height ());
	ogl.SetDepthWrite (true);
	CCanvas::Pop ();
	ogl.m_states.viewport [0] = viewport;
	}
}

//------------------------------------------------------------------------------

void ShrinkWindow (void)
{
StopTime ();
if (gameStates.render.cockpit.nType == CM_FULL_COCKPIT && (gameStates.render.vr.nScreenFlags & VRF_ALLOW_COCKPIT)) {
	gameData.render.window.h = gameData.render.window.hMax;
	gameData.render.window.w = gameData.render.window.wMax;
	//!!ToggleCockpit ();
	gameStates.render.cockpit.nNextType = CM_FULL_COCKPIT;
	cockpit->Activate (CM_STATUS_BAR);
//		ShrinkWindow ();
//		ShrinkWindow ();
	HUDInitMessage (TXT_COCKPIT_F3);
	SavePlayerProfile ();
	StartTime (0);
	return;
	}

if (gameStates.render.cockpit.nType == CM_FULL_SCREEN && (gameStates.render.vr.nScreenFlags & VRF_ALLOW_COCKPIT)) {
	//gameData.render.window.w = gameData.render.window.wMax;
	//gameData.render.window[HA] = gameData.render.window.hMax;
	cockpit->Activate (CM_STATUS_BAR);
	SavePlayerProfile ();
	StartTime (0);
	return;
	}

if (gameStates.render.cockpit.nType != CM_STATUS_BAR && (gameStates.render.vr.nScreenFlags & VRF_ALLOW_COCKPIT)) {
	StartTime (0);
	return;
	}

#if TRACE
console.printf (CON_DBG, "Cockpit mode=%d\n", gameStates.render.cockpit.nType);
#endif
if (gameData.render.window.w > WINDOW_MIN_W) {
	//int x, y;
   gameData.render.window.w -= WINDOW_W_DELTA;
	gameData.render.window.h -= WINDOW_H_DELTA;
#if TRACE
  console.printf (CON_DBG, "NewW=%d NewH=%d VW=%d maxH=%d\n", gameData.render.window.w, gameData.render.window.h, gameData.render.window.wMax, gameData.render.window.hMax);
#endif
	if (gameData.render.window.w < WINDOW_MIN_W)
		gameData.render.window.w = WINDOW_MIN_W;
	if (gameData.render.window.h < WINDOW_MIN_H)
		gameData.render.window.h = WINDOW_MIN_H;
	gameData.render.window.x = (gameData.render.window.wMax - gameData.render.window.w) / 2;
	gameData.render.window.y = (gameData.render.window.hMax - gameData.render.window.h) / 2;
	GameInitRenderSubBuffers (gameData.render.window.x, gameData.render.window.y, gameData.render.window.w, gameData.render.window.h);
	HUDClearMessages ();
	SavePlayerProfile ();
	}
StartTime (0);
}

//------------------------------------------------------------------------------

void UpdateSlidingFaces (void)
{
	CSegFace		*faceP;
	short			h, k, nOffset;
	tTexCoord2f	*texCoordP, *ovlTexCoordP;
	tUVL			*uvlP;

for (faceP = FACES.slidingFaces; faceP; faceP = faceP->nextSlidingFace) {
#if DBG
	if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
		faceP = faceP;
#endif
	texCoordP = FACES.texCoord + faceP->m_info.nIndex;
	ovlTexCoordP = FACES.ovlTexCoord + faceP->m_info.nIndex;
	uvlP = SEGMENTS [faceP->m_info.nSegment].m_sides [faceP->m_info.nSide].m_uvls;
	nOffset = faceP->m_info.nType == SIDE_IS_TRI_13;
	if (gameStates.render.bTriangleMesh) {
		static short nTriVerts [2][6] = {{0,1,2,0,2,3},{0,1,3,1,2,3}};
		for (h = 0; h < 6; h++) {
			k = nTriVerts [nOffset][h];
			texCoordP [h].v.u = X2F (uvlP [k].u);
			texCoordP [h].v.v = X2F (uvlP [k].v);
			RotateTexCoord2f (ovlTexCoordP [h], texCoordP [h], faceP->m_info.nOvlOrient);
			}
		}
	else {
		for (h = 0; h < 4; h++) {
			texCoordP [h].v.u = X2F (uvlP [(h + nOffset) % 4].u);
			texCoordP [h].v.v = X2F (uvlP [(h + nOffset) % 4].v);
			RotateTexCoord2f (ovlTexCoordP [h], texCoordP [h], faceP->m_info.nOvlOrient);
			}
		}
	}
}

//------------------------------------------------------------------------------

void GameRenderFrame (void)
{
if (gameData.segs.nFaceKeys < 0)
	meshBuilder.ComputeFaceKeys ();
PROF_START
UpdateSlidingFaces ();
PROF_END(ptAux);
SetScreenMode (SCREEN_GAME);
if (!ogl.Enhance3D () || !(gameData.app.nFrameCount & 1)) {
	cockpit->PlayHomingWarning ();
	FillBackground ();
	transparencyRenderer.Reset ();
	}
if (!gameOpts->render.stereo.nGlasses)
	RenderMonoFrame ();
else {
	if (gameOpts->render.stereo.xSeparation == 0)
		gameOpts->render.stereo.xSeparation = 3 * I2X (1) / 4;
	fix xStereoSeparation = automap.Display () ? 2 * gameOpts->render.stereo.xSeparation : gameOpts->render.stereo.xSeparation;
	if (gameStates.menus.nInMenu && ogl.Enhance3D ()) {
		RenderMonoFrame ((gameData.app.nFrameCount & 1) ? xStereoSeparation : -xStereoSeparation);
		}
	else {
		RenderMonoFrame (-xStereoSeparation);
		RenderMonoFrame (xStereoSeparation);
		}
	}
//StopTime ();
//if (!gameStates.menus.nInMenu)
//	paletteManager.EnableEffect ();
//StartTime (0);
gameData.app.nFrameCount++;
PROF_END (ptRenderFrame)
}

//------------------------------------------------------------------------------
//eof

