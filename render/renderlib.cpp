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
#include "u_mem.h"
#include "error.h"
#include "segmath.h"
#include "network.h"
#include "light.h"
#include "dynlight.h"
#include "lightmap.h"
#include "ogl_color.h"
#include "ogl_render.h"
#include "ogl_hudstuff.h"
#include "renderlib.h"
#include "renderthreads.h"
#include "cameras.h"
#include "menubackground.h"

//------------------------------------------------------------------------------

#define SW_CULLING 1

//------------------------------------------------------------------------------

int	bOutLineMode = 0,
		bShowOnlyCurSide = 0;

//------------------------------------------------------------------------------

int FaceIsVisible (short nSegment, short nSide)
{
#if SW_CULLING
CSegment *segP = SEGMENTS + nSegment;
CSide *sideP = segP->m_sides + nSide;
CFixVector v;
if (sideP->m_nType == SIDE_IS_QUAD) {
	v = gameData.render.mine.viewer.vPos - segP->SideCenter (nSide); //gameData.segs.vertices + segP->m_verts [sideVertIndex [nSide][0]]);
	return CFixVector::Dot (sideP->m_normals [0], v) >= 0;
	}
v = gameData.render.mine.viewer.vPos - VERTICES [sideP->m_vertices [(sideP->m_nType == SIDE_IS_TRI_02) ? 0 : 3]];
return (CFixVector::Dot (sideP->m_normals [0], v) >= 0) || (CFixVector::Dot (sideP->m_normals [1], v) >= 0);
#else
return 1;
#endif
}

//------------------------------------------------------------------------------

void RotateTexCoord2f (tTexCoord2f& dest, tTexCoord2f& src, ubyte nOrient)
{
if (nOrient == 1) {
	dest.v.u = 1.0f - src.v.v;
	dest.v.v = src.v.u;
	}
else if (nOrient == 2) {
	dest.v.u = 1.0f - src.v.u;
	dest.v.v = 1.0f - src.v.v;
	}
else if (nOrient == 3) {
	dest.v.u = src.v.v;
	dest.v.v = 1.0f - src.v.u;
	}
else {
	dest.v.u = src.v.u;
	dest.v.v = src.v.v;
	}
}

//------------------------------------------------------------------------------

int ToggleOutlineMode (void)
{
return bOutLineMode = !bOutLineMode;
}

//------------------------------------------------------------------------------

int ToggleShowOnlyCurSide (void)
{
return bShowOnlyCurSide = !bShowOnlyCurSide;
}

//------------------------------------------------------------------------------

void DrawOutline (int nVertices, g3sPoint **pointList)
{
	int i;
	GLint depthFunc;
	g3sPoint center, Normal;
	CFixVector n;
	CFloatVector *nf;

#if 1 //!DBG
if (gameStates.render.bQueryOcclusion) {
	tRgbaColorf outlineColor = {1, 1, 0, -1};
	G3DrawPolyAlpha (nVertices, pointList, &outlineColor, 1, -1);
	return;
	}
#endif

glGetIntegerv (GL_DEPTH_FUNC, &depthFunc);
ogl.SetDepthMode (GL_ALWAYS);
CCanvas::Current ()->SetColorRGB (255, 255, 255, 255);
center.p3_vec.SetZero ();
for (i = 0; i < nVertices; i++) {
	G3DrawLine (pointList [i], pointList [(i + 1) % nVertices]);
	center.p3_vec += pointList [i]->p3_vec;
	nf = &pointList [i]->p3_normal.vNormal;
/*
	n[X] = (fix) (nf->x() * 65536.0f);
	n[Y] = (fix) (nf->y() * 65536.0f);
	n[Z] = (fix) (nf->z() * 65536.0f);
*/
	n.Assign (*nf);
	transformation.Rotate(n, n, 0);
	Normal.p3_vec = pointList[i]->p3_vec + n * (I2X (10));
	G3DrawLine (pointList [i], &Normal);
	}
#if 0
VmVecNormal (&Normal.p3_vec,
				 &pointList [0]->p3_vec,
				 &pointList [1]->p3_vec,
				 &pointList [2]->p3_vec);
VmVecInc (&Normal.p3_vec, &center.p3_vec);
VmVecScale (&Normal.p3_vec, I2X (10));
G3DrawLine (&center, &Normal);
#endif
ogl.SetDepthMode (depthFunc);
}

// ----------------------------------------------------------------------------

char IsColoredSegFace (short nSegment, short nSide)
{
	CSegment*	segP = SEGMENTS + nSegment;
	short			nConnSeg;
	int			owner;

if ((gameData.app.nGameMode & GM_ENTROPY) && (extraGameInfo [1].entropy.nOverrideTextures == 2) &&
	 ((owner = segP->m_owner) > 0)) {
	nConnSeg = segP->m_children [nSide];
	if ((nConnSeg < 0) || (SEGMENTS [nConnSeg].m_owner != owner))
		return (owner == 1) ? 2 : 1;
	}
nConnSeg = segP->m_children [nSide];
if ((nConnSeg >= 0) && (segP->m_function == SEGMENTS [nConnSeg].m_function))
	return 0;
if (segP->HasWaterProp ())
	return 3;
if (segP->HasLavaProp ())
	return 4;
return 0;
}

// ----------------------------------------------------------------------------

tRgbaColorf segmentColors [4] = {
	 {0.5f, 0, 0, 0.333f},
	 {0, 0, 0.5f, 0.333f},
	 {0, 1.0f / 16.0f, 0.5f, 0.333f},
	 {0.5f, 0, 0, 0.333f}};

tRgbaColorf *ColoredSegmentColor (int nSegment, int nSide, char nColor)
{
	CSegment*	segP = SEGMENTS + nSegment;
	short			nConnSeg;
	int			owner;

if (nColor > 0)
	nColor--;
else {
	if ((gameData.app.nGameMode & GM_ENTROPY) && (extraGameInfo [1].entropy.nOverrideTextures == 2) &&
		((owner = segP->m_owner) > 0)) {
		nConnSeg = segP->m_children [nSide];
		if ((nConnSeg >= 0) && (SEGMENTS [nConnSeg].m_owner == owner))
			return NULL;
		nColor = (owner == 1);
		}
	if (segP->HasWaterProp ())
		nColor = 2;
	else if (segP->HasLavaProp ())
		nColor = 3;
	else
		return NULL;
	nConnSeg = segP->m_children [nSide];
	if (nConnSeg >= 0) {
		if (segP->m_function == SEGMENTS [nConnSeg].m_function)
			return NULL;
		if (IS_WALL (segP->WallNum (nSide)))
			return NULL;
		}
	}
return segmentColors + nColor;
}

//------------------------------------------------------------------------------
// If any color component > 1, scale all components down so that the greatest == 1.

static inline void ScaleColor (tFaceColor *colorP, float l)
{
	float m = colorP->color.red;

if (m < colorP->color.green)
	m = colorP->color.green;
if (m < colorP->color.blue)
	m = colorP->color.blue;
if (m > l) {
	m = l / m;
	colorP->color.red *= m;
	colorP->color.green *= m;
	colorP->color.blue *= m;
	}
}

//------------------------------------------------------------------------------

int SetVertexColor (int nVertex, tFaceColor *colorP)
{
#if DBG
if (nVertex == nDbgVertex)
	nVertex = nVertex;
#endif
if (gameStates.render.bAmbientColor) {
	colorP->color.red += gameData.render.color.ambient [nVertex].color.red;
	colorP->color.green += gameData.render.color.ambient [nVertex].color.green;
	colorP->color.blue += gameData.render.color.ambient [nVertex].color.blue;
	}
#if 0
else
	memset (colorP, 0, sizeof (*colorP));
#endif
return 1;
}

//------------------------------------------------------------------------------

int SetVertexColors (tFaceProps *propsP)
{
if (SHOW_DYN_LIGHT) {
	// set material properties specific for certain textures here
	lightManager.SetMaterial (propsP->segNum, propsP->sideNum, -1);
	return 0;
	}
memset (vertColors, 0, sizeof (vertColors));
if (gameStates.render.bAmbientColor) {
	int i, j = propsP->nVertices;
	for (i = 0; i < j; i++)
		SetVertexColor (propsP->vp [i], vertColors + i);
	}
else
	memset (vertColors, 0, sizeof (vertColors));
return 1;
}

//------------------------------------------------------------------------------

fix SetVertexLight (int nSegment, int nSide, int nVertex, tFaceColor *colorP, fix light)
{
	tRgbColorf	*pdc;
	fix			dynLight;
	float			fl, dl, hl;

//the tUVL struct has static light already in it
//scale static light for destruction effect
if (EGI_FLAG (bDarkness, 0, 0, 0))
	light = 0;
else {
#if LMAP_LIGHTADJUST
	if (USE_LIGHTMAPS) {
		else {
			light = I2X (1) / 2 + gameData.render.lights.segDeltas [nSegment * 6 + nSide];
			if (light < 0)
				light = 0;
			}
		}
#endif
	if (gameData.reactor.bDestroyed || gameStates.gameplay.seismic.nMagnitude)	//make lights flash
		light = FixMul (gameStates.render.nFlashScale, light);
	}
//add in dynamic light (from explosions, etc.)
dynLight = gameData.render.lights.dynamicLight [nVertex];
fl = X2F (light);
dl = X2F (dynLight);
light += dynLight;
#if DBG
if (nVertex == nDbgVertex)
	nVertex = nVertex;
#endif
if (gameStates.app.bHaveExtraGameInfo [IsMultiGame]) {
	if (gameData.render.lights.bGotDynColor [nVertex]) {
		pdc = gameData.render.lights.dynamicColor + nVertex;
		if (gameOpts->render.color.bMix) {
			if (gameOpts->render.color.nLevel) {
				if (gameStates.render.bAmbientColor) {
					if ((fl != 0) && gameData.render.color.vertBright [nVertex]) {
						hl = fl / gameData.render.color.vertBright [nVertex];
						colorP->color.red = colorP->color.red * hl + pdc->red * dl;
						colorP->color.green = colorP->color.green * hl + pdc->green * dl;
						colorP->color.blue = colorP->color.blue * hl + pdc->blue * dl;
						ScaleColor (colorP, fl + dl);
						}
					else {
						colorP->color.red = pdc->red * dl;
						colorP->color.green = pdc->green * dl;
						colorP->color.blue = pdc->blue * dl;
						ScaleColor (colorP, dl);
						}
					}
				else {
					colorP->color.red = fl + pdc->red * dl;
					colorP->color.green = fl + pdc->green * dl;
					colorP->color.blue = fl + pdc->blue * dl;
					ScaleColor (colorP, fl + dl);
					}
				}
			else {
				colorP->color.red =
				colorP->color.green =
				colorP->color.blue = fl + dl;
				}
			if (gameOpts->render.color.bCap) {
				if (colorP->color.red > 1.0)
					colorP->color.red = 1.0;
				if (colorP->color.green > 1.0)
					colorP->color.green = 1.0;
				if (colorP->color.blue > 1.0)
					colorP->color.blue = 1.0;
				}
			}
		else {
			float dl = X2F (light);
			dl = (float) pow (dl, 1.0f / 3.0f);
			colorP->color.red = pdc->red * dl;
			colorP->color.green = pdc->green * dl;
			colorP->color.blue = pdc->blue * dl;
			}
		}
	else {
		ScaleColor (colorP, fl + dl);
		}
	}
else {
	ScaleColor (colorP, fl + dl);
	}
colorP->color.red *= gameData.render.fBrightness;
colorP->color.green *= gameData.render.fBrightness;
colorP->color.blue *= gameData.render.fBrightness;
light = fix (light * gameData.render.fBrightness);
//saturate at max value
if (light > MAX_LIGHT)
	light = MAX_LIGHT;
return light;
}

//------------------------------------------------------------------------------

int SetFaceLight (tFaceProps *propsP)
{
	int	i;

if (SHOW_DYN_LIGHT)
	return 0;
for (i = 0; i < propsP->nVertices; i++) {
	propsP->uvls [i].l = SetVertexLight (propsP->segNum, propsP->sideNum, propsP->vp [i], vertColors + i, propsP->uvls [i].l);
	vertColors [i].index = -1;
	}
return 1;
}

//------------------------------------------------------------------------------

#if 0

static inline int IsLava (tFaceProps *propsP)
{
	short	nTexture = SEGMENTS [propsP->segNum].m_sides [propsP->sideNum].m_nBaseTex;

return (nTexture == 378) || ((nTexture >= 404) && (nTexture <= 409));
}

//------------------------------------------------------------------------------

static inline int IsWater (tFaceProps *propsP)
{
	short	nTexture = SEGMENTS [propsP->segNum].m_sides [propsP->sideNum].m_nBaseTex;

return ((nTexture >= 399) && (nTexture <= 403));
}

//------------------------------------------------------------------------------

static inline int IsWaterOrLava (tFaceProps *propsP)
{
	short	nTexture = SEGMENTS [propsP->segNum].m_sides [propsP->sideNum].m_nBaseTex;

return (nTexture == 378) || ((nTexture >= 399) && (nTexture <= 409));
}

#endif

//------------------------------------------------------------------------------

int IsTransparentTexture (short nTexture)
{
return !gameStates.app.bD1Mission &&
		 ((nTexture == 378) ||
		  (nTexture == 353) ||
		  (nTexture == 420) ||
		  (nTexture == 432) ||
		  ((nTexture >= 399) && (nTexture <= 409)));
}

//------------------------------------------------------------------------------

float WallAlpha (short nSegment, short nSide, short nWall, ubyte widFlags, int bIsMonitor, ubyte bAdditive,
					  tRgbaColorf *colorP, int *nColor, ubyte *bTextured, ubyte *bCloaked, ubyte* bTransparent)
{
	static tRgbaColorf cloakColor = {1, 1, 1, 0};

	CWall	*wallP;
	float fAlpha, fMaxColor;
	short	c;

if (!IS_WALL (nWall))
	return 1;
#if DBG
if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif
if (!(wallP = WALLS + nWall))
	return 1;
if (SHOW_DYN_LIGHT) {
	*bTransparent = (wallP->state == WALL_DOOR_CLOAKING) || (wallP->state == WALL_DOOR_DECLOAKING);
	*bCloaked = !*bTransparent && ((widFlags & WID_CLOAKED_FLAG) != 0);
	}
else {
	*bTransparent = 0;
	*bCloaked = (wallP->state == WALL_DOOR_CLOAKING) || (wallP->state == WALL_DOOR_DECLOAKING) || ((widFlags & WID_CLOAKED_FLAG) != 0);
	}
if (*bCloaked || *bTransparent || (widFlags & WID_TRANSPARENT_FLAG)) {
	if (bIsMonitor)
		return 1;
	c = wallP->cloakValue;
	if (*bCloaked || *bTransparent) {
		*colorP = cloakColor;
		*nColor = 1;
		*bTextured = !*bCloaked;
		colorP->alpha = (c >= FADE_LEVELS) ? 0 : 1.0f - float (c) / float (FADE_LEVELS);
		if (*bTransparent)
			colorP->red =
			colorP->green =
			colorP->blue = colorP->alpha;
#if DBG
		if (colorP->alpha < 1)
			return colorP->alpha;
#endif
		return colorP->alpha;
		}
	if (!gameOpts->render.color.bWalls)
		c = 0;
	if (WALLS [nWall].hps)
		fAlpha = (float) fabs ((1.0f - (float) WALLS [nWall].hps / ((float) I2X (100))));
	else if (IsMultiGame && gameStates.app.bHaveExtraGameInfo [1])
		fAlpha = COMPETITION ? 0.5f : (float) (FADE_LEVELS - extraGameInfo [1].grWallTransparency) / (float) FADE_LEVELS;
	else
		fAlpha = 1.0f - extraGameInfo [0].grWallTransparency / (float) FADE_LEVELS;
	if (fAlpha < 1) {
		//fAlpha = (float) sqrt (fAlpha);
		paletteManager.Game ()->ToRgbaf ((ubyte) c, *colorP);
		if (bAdditive) {
			colorP->red /= fAlpha;
			colorP->green /= fAlpha;
			colorP->blue /= fAlpha;
			}
		fMaxColor = colorP->red;
		if (fMaxColor < colorP->green)
			fMaxColor = colorP->green;
		if (fMaxColor < colorP->blue)
			fMaxColor = colorP->blue;
		if (fMaxColor > 1) {
			colorP->red /= fMaxColor;
			colorP->green /= fMaxColor;
			colorP->blue /= fMaxColor;
			}
		*bTextured = 0;
		*nColor = 1;
		}
	return colorP->alpha = fAlpha;
	}
if (gameStates.app.bD2XLevel) {
	c = wallP->cloakValue;
	return colorP->alpha = (c && (c < FADE_LEVELS)) ? (float) (FADE_LEVELS - c) / (float) FADE_LEVELS : 1;
	}
if (gameOpts->render.effects.bAutoTransparency && IsTransparentTexture (SEGMENTS [nSegment].m_sides [nSide].m_nBaseTex))
	return colorP->alpha = 0.8f;
return colorP->alpha = 1;
}

//------------------------------------------------------------------------------

int IsMonitorFace (short nSegment, short nSide, int bForce)
{
return (bForce || gameStates.render.bDoCameras) ? cameraManager.GetFaceCamera (nSegment * 6 + nSide) : -1;
}

//------------------------------------------------------------------------------

int SetupMonitorFace (short nSegment, short nSide, short nCamera, CSegFace *faceP)
{
	CCamera		*cameraP = cameraManager.Camera (nCamera);
	int			bHaveMonitorBg, bIsTeleCam = cameraP->GetTeleport ();
#if !DBG
	int			i;
#endif
#if RENDER2TEXTURE
	int			bCamBufAvail = cameraP->HaveBuffer (1) == 1;
#else
	int			bCamBufAvail = 0;
#endif

if (!gameStates.render.bDoCameras)
	return 0;
bHaveMonitorBg = cameraP->Valid () && /*!cameraP->bShadowMap &&*/
					  (cameraP->Texture () || bCamBufAvail) &&
					  (!bIsTeleCam || EGI_FLAG (bTeleporterCams, 0, 1, 0));
if (bHaveMonitorBg) {
	cameraP->Align (faceP, NULL, FACES.texCoord + faceP->m_info.nIndex, FACES.vertices + faceP->m_info.nIndex);
	if (bIsTeleCam) {
#if DBG
		faceP->bmBot = cameraP;
		gameStates.render.grAlpha = 1.0f;
#else
		faceP->bmTop = cameraP;
		for (i = 0; i < 4; i++)
			gameData.render.color.vertices [faceP->m_info.index [i]].color.alpha = 0.7f;
#endif
		}
	else if (/*gameOpts->render.cameras.bFitToWall ||*/ (faceP->m_info.nOvlTex == 0) || !faceP->bmBot)
		faceP->bmBot = cameraP;
	else
		faceP->bmTop = cameraP;
	faceP->texCoordP = cameraP->TexCoord ();
	}
faceP->m_info.bTeleport = bIsTeleCam;
cameraP->SetVisible (1);
return bHaveMonitorBg || gameOpts->render.cameras.bFitToWall;
}

//------------------------------------------------------------------------------

void AdjustVertexColor (CBitmap *bmP, tFaceColor *colorP, fix xLight)
{
	float l = (bmP && (bmP->Flags () & BM_FLAG_NO_LIGHTING)) ? 1.0f : X2F (xLight);
	float s = 1.0f;

if (ogl.m_states.bScaleLight)
	s *= gameStates.render.bHeadlightOn ? 0.4f : 0.3f;
if (!colorP->index || !gameStates.render.bAmbientColor || (gameStates.app.bEndLevelSequence >= EL_OUTSIDE)) {
	colorP->color.red =
	colorP->color.green =
	colorP->color.blue = l * s;
	}
else if (s != 1) {
	colorP->color.red *= s;
	colorP->color.green *= s;
	colorP->color.blue *= s;
	}
colorP->color.alpha = 1;
}

// -----------------------------------------------------------------------------------
//Given a list of point numbers, rotate any that haven't been bRotated this frame
//cc.ccAnd and cc.ccOr will contain the position/orientation of the face that is determined
//by the vertices passed relative to the viewer

g3sPoint *RotateVertex (int i)
{
g3sPoint *p = gameData.segs.points + i;
if (gameData.render.mine.nRotatedLast [i] != gameStates.render.nFrameCount) {
	G3TransformAndEncodePoint (p, gameData.segs.vertices [i]);
#if TRANSP_DEPTH_HASH
	fix d = p->p3_vec.Mag ();
	if (gameData.render.zMin > d)
		gameData.render.zMin = d;
	if (gameData.render.zMax < d)
		gameData.render.zMax = d;
#else
	if (gameData.render.zMax < p->p3_vec [Z])
		gameData.render.zMax = p->p3_vec [Z];
#endif
	if (!ogl.m_states.bUseTransform) {
		gameData.segs.fVertices [i][X] = X2F (p->p3_vec [X]);
		gameData.segs.fVertices [i][Y] = X2F (p->p3_vec [Y]);
		gameData.segs.fVertices [i][Z] = X2F (p->p3_vec [Z]);
		}
	p->p3_index = i;
	gameData.render.mine.nRotatedLast [i] = gameStates.render.nFrameCount;
	}
return p;
}

// -----------------------------------------------------------------------------------
//Given a list of point numbers, rotate any that haven't been bRotated this frame
//cc.ccAnd and cc.ccOr will contain the position/orientation of the face that is determined
//by the vertices passed relative to the viewer

g3sCodes RotateVertexList (int nVertices, short* vertexIndexP)
{
	int			i;
	g3sPoint		*p;
	g3sCodes		cc = {0, 0xff};

for (i = 0; i < nVertices; i++) {
	p = RotateVertex (vertexIndexP [i]);
	cc.ccAnd &= p->p3_codes;
	cc.ccOr |= p->p3_codes;
	}
	return cc;
}

// -----------------------------------------------------------------------------------
//Given a lit of point numbers, project any that haven't been projected
void ProjectVertexList (int nVertices, short *vertexIndexP)
{
	int i, j;

for (i = 0; i < nVertices; i++) {
	j = vertexIndexP [i];
	if (!(gameData.segs.points [j].p3_flags & PF_PROJECTED))
		G3ProjectPoint (&gameData.segs.points [j]);
	}
}

//------------------------------------------------------------------------------

void RotateSideNorms (void)
{
	int			i, j;
	CSide			*sideP;

for (i = 0; i < gameData.segs.nSegments; i++)
	for (j = 6, sideP = SEGMENTS [i].m_sides; j; j--, sideP++) {
		transformation.Rotate (sideP->m_rotNorms [0], sideP->m_normals [0], 0);
		transformation.Rotate (sideP->m_rotNorms [1], sideP->m_normals [1], 0);
		}
}

//------------------------------------------------------------------------------

#if USE_SEGRADS

void TransformSideCenters (void)
{
	int	i;

for (i = 0; i < gameData.segs.nSegments; i++)
	transformation.Transform (gameData.segs.segCenters [1] + i, gameData.segs.segCenters [0] + i, 0);
}

#endif

//------------------------------------------------------------------------------

ubyte BumpVisitedFlag (void)
{
#if USE_OPENMP > 1
#	pragma omp critical
#endif
	{
	if (!++gameData.render.mine.nVisited) {
		gameData.render.mine.bVisited.Clear (0);
		gameData.render.mine.nVisited = 1;
		}
	}
return gameData.render.mine.nVisited;
}

//------------------------------------------------------------------------------

ubyte BumpProcessedFlag (void)
{
#if USE_OPENMP > 1
#	pragma omp critical
#endif
	{
	if (!++gameData.render.mine.nProcessed) {
		gameData.render.mine.bProcessed.Clear (0);
		gameData.render.mine.nProcessed = 1;
		}
	}
return gameData.render.mine.nProcessed;
}

//------------------------------------------------------------------------------

ubyte BumpVisibleFlag (void)
{
#if USE_OPENMP > 1
#	pragma omp critical
#endif
	{
	if (!++gameData.render.mine.nVisible) {
		gameData.render.mine.bVisible.Clear (0);
		gameData.render.mine.nVisible = 1;
		}
	}
return gameData.render.mine.nVisible;
}

// ----------------------------------------------------------------------------

int SegmentMayBeVisible (short nStartSeg, short nRadius, int nMaxDist, int nThread)
{
if (gameData.render.mine.bVisible [nStartSeg] == gameData.render.mine.nVisible)
	return 1;

	CSegment*	segP;
	int			nSegment, nChildSeg, nChild, h, i, j;
	ubyte			bVisited [MAX_THREADS][MAX_SEGMENTS_D2X];
	ubyte*		visitedP = bVisited [nThread];
	short*		segListP = gameData.render.mine.segRenderList [0].Buffer ();

segListP [0] = nStartSeg;
gameData.render.mine.bProcessed [nStartSeg] = BumpProcessedFlag ();
if (nMaxDist < 0)
	nMaxDist = nRadius * I2X (20);
memset (visitedP, 0, gameData.segs.nSegments);
visitedP [nStartSeg] = 1;
for (i = 0, j = 1; nRadius; nRadius--) {
	for (h = i, i = j; h < i; h++) {
		nSegment = segListP [h];
		if ((gameData.render.mine.bVisible [nSegment] == gameData.render.mine.nVisible) &&
			 (!nMaxDist || (CFixVector::Dist (SEGMENTS [nStartSeg].Center (), SEGMENTS [nSegment].Center ()) <= nMaxDist)))
			return 1;
		segP = SEGMENTS + nSegment;
		for (nChild = 0; nChild < 6; nChild++) {
			nChildSeg = segP->m_children [nChild];
			if ((nChildSeg >= 0) && !visitedP [nChildSeg] && (segP->IsDoorWay (nChild, NULL) & WID_RENDPAST_FLAG)) {
				segListP [j++] = nChildSeg;
				visitedP [nChildSeg] = 1;
				}
			}
		}
	}
return 0;
}

//------------------------------------------------------------------------------
// eof
