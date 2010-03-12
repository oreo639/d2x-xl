#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "descent.h"
#include "error.h"
#include "u_mem.h"
#include "light.h"
#include "dynlight.h"
#include "lightmap.h"
#include "automap.h"
#include "texmerge.h"
#include "ogl_lib.h"
#include "ogl_color.h"
#include "ogl_fastrender.h"
#include "glare.h"
#include "rendermine.h"
#include "renderthreads.h"
#include "gpgpu_lighting.h"
#include "fastrender.h"
#include "ogl_shader.h"

#define LOAD_BITMAPS	0

//------------------------------------------------------------------------------

void ResetFaceList (void)
{
PROF_START
int* tails = gameData.render.faceIndex.tails.Buffer (),
	* usedKeys = gameData.render.faceIndex.usedKeys.Buffer ();
for (int i = 0; i < gameData.render.faceIndex.nUsedKeys; i++) 
	tails [usedKeys [i]] = -1;
gameData.render.faceIndex.nUsedKeys = 0;
PROF_END(ptFaceList)
}

//------------------------------------------------------------------------------
// AddFaceListItem creates lists of faces with the same texture
// The lists themselves are kept in gameData.render.faceList
// gameData.render.faceIndex holds the root index of each texture's face list

int AddFaceListItem (CSegFace *faceP, int nThread)
{
#if 1
if (faceP->m_info.nFrame == gameData.app.nMineRenderCount)
	return 0;
#endif
#if DBG
if (faceP - FACES.faces >= gameData.segs.nFaces)
	return 0;
#endif

#ifdef _OPENMP
#	pragma omp critical
#endif
{
PROF_START
int nKey = faceP->m_info.nKey;
int i = gameData.render.nUsedFaces++;
int j = gameData.render.faceIndex.tails [nKey];
if (j < 0) {
	gameData.render.faceIndex.usedKeys [gameData.render.faceIndex.nUsedKeys++] = nKey;
	gameData.render.faceIndex.roots [nKey] = i;
	}
else {
#if DBG
	if (!i)
		i = i;
#endif
	gameData.render.faceList [j].nNextItem = i;
	}
gameData.render.faceList [i].nNextItem = -1;
gameData.render.faceList [i].faceP = faceP;
gameData.render.faceIndex.tails [nKey] = i;
faceP->m_info.nTransparent = -1;
faceP->m_info.nColored = -1;
faceP->m_info.nFrame = gameData.app.nFrameCount;
PROF_END(ptFaceList)
}
return 1;
}

//------------------------------------------------------------------------------

void LoadFaceBitmaps (CSegment *segP, CSegFace *faceP)
{
	CSide	*sideP = segP->m_sides + faceP->m_info.nSide;
	short	nFrame = sideP->m_nFrame;

if (faceP->m_info.nCamera >= 0) {
	if (SetupMonitorFace (faceP->m_info.nSegment, faceP->m_info.nSide, faceP->m_info.nCamera, faceP)) 
		return;
	faceP->m_info.nCamera = -1;
	}
#if DBG
if (FACE_IDX (faceP) == nDbgFace)
	nDbgFace = nDbgFace;
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif
if ((faceP->m_info.nBaseTex < 0) || !faceP->m_info.bTextured)
	return;
if (faceP->m_info.bOverlay)
	faceP->m_info.nBaseTex = sideP->m_nOvlTex;
else {
	faceP->m_info.nBaseTex = sideP->m_nBaseTex;
	if (!faceP->m_info.bSplit)
		faceP->m_info.nOvlTex = sideP->m_nOvlTex;
	}
if (gameOpts->ogl.bGlTexMerge) {
	faceP->bmBot = LoadFaceBitmap (faceP->m_info.nBaseTex, nFrame);
	if (nFrame)
		nFrame = nFrame;
	if (faceP->m_info.nOvlTex)
		faceP->bmTop = LoadFaceBitmap ((short) (faceP->m_info.nOvlTex), nFrame);
	else
		faceP->bmTop = NULL;
	}
else {
	if (faceP->m_info.nOvlTex != 0) {
		faceP->bmBot = TexMergeGetCachedBitmap (faceP->m_info.nBaseTex, faceP->m_info.nOvlTex, faceP->m_info.nOvlOrient);
		if (faceP->bmBot)
			faceP->bmBot->SetupTexture (1, 1);
#if DBG
		else
			faceP->bmBot = TexMergeGetCachedBitmap (faceP->m_info.nBaseTex, faceP->m_info.nOvlTex, faceP->m_info.nOvlOrient);
#endif
		faceP->bmTop = NULL;
		}
	else {
		faceP->bmBot = gameData.pig.tex.bitmapP + gameData.pig.tex.bmIndexP [faceP->m_info.nBaseTex].index;
		LoadBitmap (gameData.pig.tex.bmIndexP [faceP->m_info.nBaseTex].index, gameStates.app.bD1Mission);
		}
	}
}

//------------------------------------------------------------------------------

bool RenderFaceDepth (CSegment *segP, CSegFace *faceP)
{
#if LOAD_BITMAPS
if (!(faceP->m_info.widFlags & WID_RENDER_FLAG))
	return false;
LoadFaceBitmaps (segP, faceP);
#endif
if (!faceP->bmBot)
	return false;
RenderDepth (faceP, faceP->bmBot, faceP->bmTop);
return true;
}

//------------------------------------------------------------------------------

bool RenderStaticLights (CSegment *segP, CSegFace *faceP)
{
#if LOAD_BITMAPS
if (!(faceP->m_info.widFlags & WID_RENDER_FLAG))
	return false;
LoadFaceBitmaps (segP, faceP);
#endif
if (!faceP->bmBot)
	return false;
RenderLightmaps (faceP, faceP->bmBot, faceP->bmTop);
return true;
}

//------------------------------------------------------------------------------

bool RenderDynamicLights (CSegment *segP, CSegFace *faceP)
{
#if LOAD_BITMAPS
if (!(faceP->m_info.widFlags & WID_RENDER_FLAG))
	return false;
LoadFaceBitmaps (segP, faceP);
#endif
if (!faceP->bmBot)
	return false;
RenderLights (faceP, faceP->bmBot, faceP->bmTop);
return true;
}

//------------------------------------------------------------------------------

bool RenderPlayerLights (CSegment *segP, CSegFace *faceP)
{
#if LOAD_BITMAPS
if (!(faceP->m_info.widFlags & WID_RENDER_FLAG))
	return false;
LoadFaceBitmaps (segP, faceP);
#endif
if (!faceP->bmBot)
	return false;
RenderHeadlights (faceP, faceP->bmBot, faceP->bmTop);
return true;
}

//------------------------------------------------------------------------------

bool RenderFaceColor (CSegment *segP, CSegFace *faceP)
{
#if LOAD_BITMAPS
if (!(faceP->m_info.widFlags & WID_RENDER_FLAG))
	return false;
LoadFaceBitmaps (segP, faceP);
#endif
if (!faceP->bmBot)
	return false;
RenderColor (faceP, faceP->bmBot, faceP->bmTop);
return true;
}

//------------------------------------------------------------------------------

bool RenderStaticFace (CSegment *segP, CSegFace *faceP)
{
if (IS_WALL (faceP->m_info.nWall))
	return false;
#if LOAD_BITMAPS
if (!(faceP->m_info.widFlags & WID_RENDER_FLAG))
	return false;
LoadFaceBitmaps (segP, faceP);
#endif
if (!faceP->bmBot)
	return false;
RenderFace (faceP, faceP->bmBot, faceP->bmTop, (faceP->m_info.nCamera < 0) || faceP->m_info.bTeleport, faceP->m_info.bTextured);
return true;
}

//------------------------------------------------------------------------------

bool RenderDynamicFace (CSegment *segP, CSegFace *faceP)
{
if (!IS_WALL (faceP->m_info.nWall))
	return false;
#if LOAD_BITMAPS
if (!(faceP->m_info.widFlags & WID_RENDER_FLAG))
	return false;
LoadFaceBitmaps (segP, faceP);
#endif
RenderFace (faceP, faceP->bmBot, faceP->bmTop, (faceP->m_info.nCamera < 0) || faceP->m_info.bTeleport, faceP->m_info.bTextured);
return true;
}

//------------------------------------------------------------------------------

bool RenderColoredFace (CSegment *segP, CSegFace *faceP)
{
#if LOAD_BITMAPS
if (!(faceP->m_info.widFlags & WID_RENDER_FLAG))
	return false;
#endif
if (faceP->bmBot)
	return false;
short nSegment = faceP->m_info.nSegment;
short special = SEGMENTS [nSegment].m_nType;
if ((special < SEGMENT_IS_WATER) || (special > SEGMENT_IS_TEAM_RED) || 
	 (SEGMENTS [nSegment].m_group < 0) || (SEGMENTS [nSegment].m_owner < 1))
	return false;
if (!gameData.app.nFrameCount)
	gameData.render.nColoredFaces++;
RenderFace (faceP, faceP->bmBot, faceP->bmTop, (faceP->m_info.nCamera < 0) || faceP->m_info.bTeleport, faceP->m_info.bTextured);
return true;
}

//------------------------------------------------------------------------------

bool RenderCoronaFace (CSegment *segP, CSegFace *faceP)
{
#if LOAD_BITMAPS
if (!(faceP->m_info.widFlags & WID_RENDER_FLAG))
	return false;
#endif
if (!faceP->m_info.nCorona)
	return false;
#if DBG
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif
glareRenderer.Render (faceP->m_info.nSegment, faceP->m_info.nSide, 1, faceP->m_info.fRads [0]);
return true;
}

//------------------------------------------------------------------------------

bool RenderSkyBoxFace (CSegment *segP, CSegFace *faceP)
{
LoadFaceBitmaps (segP, faceP);
RenderFace (faceP, faceP->bmBot, faceP->bmTop, 1, 1);
return true;
}

//------------------------------------------------------------------------------

#if defined(_WIN32) && !DBG
typedef bool (__fastcall * pRenderHandler) (CSegment *segP, CSegFace *faceP);
#else
typedef bool (* pRenderHandler) (CSegment *segP, CSegFace *faceP);
#endif

static pRenderHandler renderHandlers [] = {
	RenderStaticLights, 
	RenderDynamicLights, 
	RenderPlayerLights,
	RenderFaceDepth, 
	RenderFaceColor,
	RenderStaticFace, 
	RenderDynamicFace, 
	RenderColoredFace, 
	RenderCoronaFace, 
	RenderSkyBoxFace
	};



static inline bool RenderMineFace (CSegment *segP, CSegFace *faceP, int nType)
{
#if LOAD_BITMAPS
if (!faceP->m_info.bVisible)
	return false;
#endif
#if DBG
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif
return renderHandlers [nType] (segP, faceP);
}

//------------------------------------------------------------------------------

typedef struct tFaceRef {
	short		nSegment;
	CSegFace	*faceP;
	} tFaceRef;

static tFaceRef faceRef [2][MAX_SEGMENTS_D2X * 6];

int QCmpFaces (CSegFace *fp, CSegFace *mp)
{
if (!fp->m_info.bOverlay && mp->m_info.bOverlay)
	return -1;
if (fp->m_info.bOverlay && !mp->m_info.bOverlay)
	return 1;
if (!fp->m_info.bTextured && mp->m_info.bTextured)
	return -1;
if (fp->m_info.bTextured && !mp->m_info.bTextured)
	return 1;
if (!fp->m_info.nOvlTex && mp->m_info.nOvlTex)
	return -1;
if (fp->m_info.nOvlTex && !mp->m_info.nOvlTex)
	return 1;
if (fp->m_info.nBaseTex < mp->m_info.nBaseTex)
	return -1;
if (fp->m_info.nBaseTex > mp->m_info.nBaseTex)
	return -1;
if (fp->m_info.nOvlTex < mp->m_info.nOvlTex)
	return -1;
if (fp->m_info.nOvlTex > mp->m_info.nOvlTex)
	return -1;
return 0;
}

//------------------------------------------------------------------------------

void QSortFaces (int left, int right)
{
	int		l = left,
				r = right;
	tFaceRef	*pf = faceRef [0];
	CSegFace	m = *pf [(l + r) / 2].faceP;

do {
	while (QCmpFaces (pf [l].faceP, &m) < 0)
		l++;
	while (QCmpFaces (pf [r].faceP, &m) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			tFaceRef h = pf [l];
			pf [l] = pf [r];
			pf [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortFaces (l, right);
if (left < r)
	QSortFaces (left, r);
}

//------------------------------------------------------------------------------

#if 1

#define SortFaces()	0

#else

int SortFaces (void)
{
	tSegFaces	*segFaceP;
	CSegFace		*faceP;
	tFaceRef		*ph, *pi, *pj;
	int			h, i, j;
	short			nSegment;

for (h = i = 0, ph = faceRef [0]; i < gameData.render.mine.nRenderSegs; i++) {
	if (0 > (nSegment = gameData.render.mine.nSegRenderList [0][i]))
		continue;
	segFaceP = SEGFACES + nSegment;
	for (j = segFaceP->nFaces, faceP = segFaceP->faceP; j; j--, faceP++, h++, ph++) {
		ph->nSegment = nSegment;
		ph->faceP = faceP;
		}
	}
tiRender.nFaces = h;
if (h > 1) {
	if (RunRenderThreads (rtSortFaces)) {
		for (i = h / 2, j = h - i, ph = faceRef [1], pi = faceRef [0], pj = pi + h / 2; h; h--) {
			if (i && (!j || (QCmpFaces (pi->faceP, pj->faceP) <= 0))) {
				*ph++ = *pi++;
				i--;
				}
			else if (j) {
				*ph++ = *pj++;
				j--;
				}
			}
		}
	else 
		QSortFaces (0, h - 1);
	}
return tiRender.nFaces;
}

#endif

//------------------------------------------------------------------------------

int BeginRenderFaces (int nType, int bDepthOnly)
{
	int	//bVBO = 0,
			bLightmaps = lightmapManager.HaveLightmaps () && (nType == RENDER_LIGHTMAPS),
			bColor = !gameStates.render.bFullBright && (nType == RENDER_LIGHTMAPS), //RENDER_COLOR),
			bTexCoord = (nType != RENDER_COLOR) && (nType != RENDER_LIGHTS),
			bNormals = (nType == RENDER_LIGHTMAPS) || (nType == RENDER_LIGHTS) || (nType == RENDER_HEADLIGHTS); //(nType == RENDER_COLOR);

gameData.threads.vertColor.data.bDarkness = 0;
gameStates.render.nType = nType;
gameStates.render.history.bOverlay = -1;
gameStates.render.history.bColored = 1;
gameStates.render.history.nBlendMode = -1;
gameStates.render.history.bmBot = 
gameStates.render.history.bmTop =
gameStates.render.history.bmMask = NULL;
gameStates.render.bQueryCoronas = 0;
ogl.ResetClientStates ();
shaderManager.Deploy (-1);
ogl.SetFaceCulling (true);
CTexture::Wrap (GL_REPEAT);

if (nType == RENDER_DEPTH) {
	ogl.ColorMask (0,0,0,0,0);
	ogl.SetDepthWrite (true);
	ogl.SetDepthMode (GL_LESS);
	ogl.SetBlendMode (GL_ONE, GL_ZERO);
	}
else if (nType == RENDER_LIGHTMAPS) {
#if 1
	if (!SetupColorShader ())
		return 0;
#endif
	ogl.SetDepthMode (GL_EQUAL); 
	ogl.SetBlendMode (GL_ONE, GL_ZERO);
	ogl.SetDepthWrite (false);
	}
else if (nType == RENDER_COLOR) {
	ogl.SetDepthMode (GL_EQUAL); 
	ogl.SetBlendMode (GL_ONE, GL_ONE);
	ogl.SetDepthWrite (false);
	}
else if (nType == RENDER_HEADLIGHTS) {
	if (0 > lightManager.Headlights ().SetupShader ())
		return 0;
	ogl.SetDepthMode (GL_EQUAL); 
	ogl.SetBlendMode (GL_ONE, GL_ONE);
	ogl.SetDepthWrite (false);
	}
else if (nType == RENDER_LIGHTS) {
	if (0 > LoadPerPixelLightingShader ())
		return 0;
	ogl.SetDepthMode (GL_EQUAL); 
	ogl.SetBlendMode (GL_ONE, GL_ONE);
	ogl.SetDepthWrite (false);
	ogl.EnableLighting (1);
	for (int i = 0; i < 8; i++)
		glEnable (GL_LIGHT0 + i);
	ogl.SetLighting (false);
	}
else if (nType == RENDER_CORONAS) {
	if (glareRenderer.Style ())
		glareRenderer.LoadShader (10);
	return 0;
	}
else {
	if (!gameStates.render.bPerPixelLighting || gameStates.render.bFullBright) {
		ogl.SetDepthMode (GL_LEQUAL); 
		ogl.SetBlendMode (GL_ONE, GL_ZERO);
		}
	else if ((nType == RENDER_STATIC_FACES) || (nType == RENDER_DYNAMIC_FACES) || (nType == RENDER_COLORED_FACES)) {
		ogl.SetDepthMode (GL_EQUAL); 
		ogl.SetBlendMode (GL_DST_COLOR, GL_ZERO);
		ogl.SetDepthWrite (false);
		}
	else if (nType == RENDER_CORONAS) {
		ogl.SetDepthMode (GL_LEQUAL); 
		ogl.SetBlendMode (GL_ONE, GL_ONE);
		ogl.SetDepthWrite (false);
		}
	else {
		ogl.SetDepthMode (GL_LEQUAL); 
		ogl.SetBlendMode (GL_ONE, GL_ZERO);
		ogl.SetDepthWrite (false);
		}
	}

ogl.SetupTransform (1);
#if GEOMETRY_VBOS
if (bVBO) {
	ogl.EnableClientStates (!bDepthOnly, bColor, bNormals, GL_TEXTURE0);
	if (bNormals)
		OglNormalPointer (GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iNormals));
	if (!bDepthOnly) {
		if (bLightmaps)
			OglTexCoordPointer (2, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iLMapTexCoord));
		else
			OglTexCoordPointer (2, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iTexCoord));
		OglColorPointer (4, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iColor));
		}
	OglVertexPointer (3, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iVertices));
	if (bLightmaps) {
		ogl.EnableClientStates (1, 1, bNormals, GL_TEXTURE1);
		OglTexCoordPointer (2, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iTexCoord));
		OglColorPointer (4, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iColor));
		OglVertexPointer (3, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iVertices));
		}
	ogl.EnableClientStates (1, 1, 0, GL_TEXTURE1 + bLightmaps);
	OglTexCoordPointer (2, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iOvlTexCoord));
	OglColorPointer (4, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iColor));
	OglVertexPointer (3, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iVertices));
	ogl.EnableClientStates (1, 1, 0, GL_TEXTURE2 + bLightmaps);
	OglTexCoordPointer (2, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iOvlTexCoord));
	OglColorPointer (4, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iColor));
	OglVertexPointer (3, GL_FLOAT, 0, G3_BUFFER_OFFSET (FACES.iVertices));
	if (FACES.vboIndexHandle)
		glBindBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, FACES.vboIndexHandle);
	}	
else 
#endif
	{
	if ((nType == RENDER_DEPTH) || (nType >= RENDER_STATIC_FACES)) {
		ogl.EnableClientStates (1, bColor, bNormals, GL_TEXTURE1);
		OglTexCoordPointer (2, GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.ovlTexCoord.Buffer ()));
		if (bColor)
			OglColorPointer (4, GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.color.Buffer ()));
		OglVertexPointer (3, GL_FLOAT, 0, reinterpret_cast<const GLvoid*> (FACES.vertices.Buffer ()));
		if (nType < RENDER_CORONAS) {
			ogl.EnableClientStates (1, bColor, 0, GL_TEXTURE2);
			OglTexCoordPointer (2, GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.ovlTexCoord.Buffer ()));
			if (bColor)
				OglColorPointer (4, GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.color.Buffer ()));
			OglVertexPointer (3, GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.vertices.Buffer ()));
			}
		}

	ogl.EnableClientStates (bTexCoord, bColor, bNormals, GL_TEXTURE0);
	if (bNormals)
		OglNormalPointer (GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.normals.Buffer ()));
	if (bTexCoord) {
		if (bLightmaps)
			OglTexCoordPointer (2, GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.lMapTexCoord.Buffer ()));
		else
			OglTexCoordPointer (2, GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.texCoord.Buffer ()));
		}
	if (bColor)
		OglColorPointer (4, GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.color.Buffer ()));
	OglVertexPointer (3, GL_FLOAT, 0, reinterpret_cast<const GLvoid *> (FACES.vertices.Buffer ()));
	}
glColor3f (1,1,1);
ogl.ClearError (0);
return 1;
}

//------------------------------------------------------------------------------

void EndRenderFaces (int bDepthOnly)
{
#if 1
FlushFaceBuffer (1);
#endif
ogl.ResetClientStates ();
shaderManager.Deploy (-1);
if (gameStates.render.nType != RENDER_CORONAS) {
	if (gameStates.render.bPerPixelLighting == 2)
		ogl.DisableLighting ();
	ogl.ResetTransform (1);
	if (FACES.vboDataHandle)
		glBindBufferARB (GL_ARRAY_BUFFER_ARB, 0);
	}
else if (glareRenderer.Style () == 2)
	glareRenderer.UnloadShader ();
else if (ogl.m_states.bOcclusionQuery && gameData.render.lights.nCoronas && !gameStates.render.bQueryCoronas && (glareRenderer.Style () == 1))
	glDeleteQueries (gameData.render.lights.nCoronas, gameData.render.lights.coronaQueries.Buffer ());
ogl.SetBlendMode (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
ogl.SetDepthWrite (true);
ogl.SetDepthTest (true);
ogl.SetDepthMode (GL_LEQUAL);
ogl.ClearError (0);
}

//------------------------------------------------------------------------------

void RenderSkyBoxFaces (void)
{
	tSegFaces*	segFaceP;
	CSegFace*	faceP;
	short*		segP;
	int			i, j, nSegment, bFullBright = gameStates.render.bFullBright;

if (gameStates.render.bHaveSkyBox) {
	ogl.SetDepthWrite (true);
	gameStates.render.bFullBright = 1;
	BeginRenderFaces (RENDER_SKYBOX, 0);
	for (i = gameData.segs.skybox.ToS (), segP = gameData.segs.skybox.Buffer (); i; i--, segP++) {
		nSegment = *segP;
		segFaceP = SEGFACES + nSegment;
		for (j = segFaceP->nFaces, faceP = segFaceP->faceP; j; j--, faceP++) {
			if (!(faceP->m_info.bVisible = !FaceIsCulled (nSegment, faceP->m_info.nSide)))
				continue;
			RenderMineFace (SEGMENTS + nSegment, faceP, RENDER_SKYBOX);
			}
		}
	gameStates.render.bFullBright = bFullBright;
	EndRenderFaces (0);
	}
}

//------------------------------------------------------------------------------

static inline int VisitSegment (short nSegment, int bAutomap)
{
if (nSegment < 0)
	return 0;
if (bAutomap) {
	if (automap.Display ()) {
		if (!(automap.m_bFull || automap.m_visible [nSegment]))
			return 0;
		if (!gameOpts->render.automap.bSkybox && (SEGMENTS [nSegment].m_nType == SEGMENT_IS_SKYBOX))
			return 0;
		}
	else
		automap.m_visited [0][nSegment] = gameData.render.mine.bSetAutomapVisited;
	}
if (VISITED (nSegment))
	return 0;
VISIT (nSegment);
return 1;
}

//------------------------------------------------------------------------------

static inline int FaceIsTransparent (CSegFace *faceP, CBitmap *bmBot, CBitmap *bmTop)
{
if (!bmBot)
	return faceP->m_info.nTransparent = (faceP->m_info.color.alpha < 1.0f);
if (faceP->m_info.bTransparent || faceP->m_info.bAdditive)
	return 1;
if (bmBot->Flags () & BM_FLAG_SEE_THRU)
	return faceP->m_info.nTransparent = 0;
if (!(bmBot->Flags () & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT)))
	return 0;
if (!bmTop)
	return 1;
if (bmTop->Flags () & BM_FLAG_SEE_THRU)
	return 0;
if (bmTop->Flags () & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT))
	return 1;
return 0;
}

//------------------------------------------------------------------------------

static inline int FaceIsColored (CSegFace *faceP)
{
return !automap.Display () || automap.m_visited [0][faceP->m_info.nSegment] || !gameOpts->render.automap.bGrayOut;
}

//------------------------------------------------------------------------------

short BuildFaceLists (void)
{
	tFaceListItem*	fliP = gameData.render.faceList.Buffer ();
	CSegFace*		faceP;
	int				i, j, nFaces = 0, nSegment = -1;

#if 1
if (automap.Display ())
	gameData.render.faceIndex.usedKeys.SortAscending (0, gameData.render.faceIndex.nUsedKeys - 1);
#endif
gameData.render.nRenderFaces [0] = 
gameData.render.nRenderFaces [1] = 0;
for (i = 0; i < gameData.render.faceIndex.nUsedKeys; i++) {
	for (j = gameData.render.faceIndex.roots [gameData.render.faceIndex.usedKeys [i]]; j >= 0; j = fliP [j].nNextItem) {
		faceP = fliP [j].faceP;
#if DBG
		if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
			nDbgSeg = nDbgSeg;
#endif
		if (faceP->m_info.bVisible) {
			LoadFaceBitmaps (SEGMENTS + faceP->m_info.nSegment, faceP);
			faceP->m_info.nTransparent = FaceIsTransparent (faceP, faceP->bmBot, faceP->bmTop);
			faceP->m_info.nColored = FaceIsColored (faceP);
			gameData.render.renderFaces [faceP->m_info.nTransparent][gameData.render.nRenderFaces [faceP->m_info.nTransparent]++] = faceP;
			}
		}
	}
return nFaces;
}

//------------------------------------------------------------------------------

#if 1

short RenderFaceList (int nType)
{
	CSegFace**	flP = gameData.render.renderFaces [gameStates.render.bTransparency].Buffer ();
	CSegFace*	faceP;
	int			i, j, nFaces = 0, nSegment = -1;
	int			bAutomap = (nType <= RENDER_STATIC_FACES);

for (i = 0, j = gameData.render.nRenderFaces [gameStates.render.bTransparency]; i < j; i++) {
	faceP = flP [i];
	if (nSegment != faceP->m_info.nSegment) {
		nSegment = faceP->m_info.nSegment;
#if DBG
		if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
			nDbgSeg = nDbgSeg;
#endif
		if (gameStates.render.nType == RENDER_DEPTH)
			VisitSegment (nSegment, bAutomap);
		else if (gameStates.render.nType == RENDER_LIGHTS)
			lightManager.Index (0)[0].nActive = -1;
		}
	if (RenderMineFace (SEGMENTS + nSegment, faceP, nType))
		nFaces++;
	}
return nFaces;
}

#else

short RenderFaceList (int nType)
{
	tFaceListItem*	fliP = gameData.render.faceList.Buffer ();
	CSegFace*		faceP;
	int				i, j, nFaces = 0, nSegment = -1;
	int				bAutomap = (nType <= RENDER_STATIC_FACES);

#if 1
if (automap.Display ())
	flx.usedKeys.SortAscending (0, flx.nUsedKeys - 1);
#endif
for (i = 0; i < gameData.render.faceIndex.nUsedKeys; i++) {
	for (j = gameData.render.faceIndex.roots [flx.usedKeys [i]]; j >= 0; j = fliP->nNextItem) {
		faceP = fliP [j].faceP;
		if (nSegment != faceP->m_info.nSegment) {
			nSegment = faceP->m_info.nSegment;
#if DBG
			if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
				nDbgSeg = nDbgSeg;
#endif
			if (gameStates.render.nType == RENDER_DEPTH)
				VisitSegment (nSegment, bAutomap);
			else if (gameStates.render.nType == RENDER_LIGHTS)
				lightManager.Index (0)[0].nActive = -1;
			}
		if (RenderMineFace (SEGMENTS + nSegment, faceP, nType))
			nFaces++;
		}
	}
return nFaces;
}

#endif

//------------------------------------------------------------------------------

int SetupCoronaFaces (void)
{
	tSegFaces*	segFaceP;
	CSegFace*	faceP;
	int			i, j, nSegment;

if (!(gameOpts->render.effects.bEnabled && glareRenderer.Style ()))
	return 0;
gameData.render.lights.nCoronas = 0;
for (i = 0; i < gameData.render.mine.nRenderSegs; i++) {
	if (0 > (nSegment = gameData.render.mine.nSegRenderList [0][i]))
		continue;
	if ((SEGMENTS [nSegment].m_nType == SEGMENT_IS_SKYBOX) ||
		 (SEGMENTS [nSegment].m_nType == SEGMENT_IS_OUTDOOR))
		continue;
	segFaceP = SEGFACES + nSegment;
#if DBG
	if (nSegment == nDbgSeg)
		nSegment = nSegment;
#endif
	for (j = segFaceP->nFaces, faceP = segFaceP->faceP; j; j--, faceP++)
		if (faceP->m_info.bVisible && faceP->m_info.bIsLight && (faceP->m_info.nCamera < 0) &&
			 glareRenderer.FaceHasCorona (nSegment, faceP->m_info.nSide, NULL, NULL))
			faceP->m_info.nCorona = ++gameData.render.lights.nCoronas;
		else
			faceP->m_info.nCorona = 0;
	}
return gameData.render.lights.nCoronas;
}

//------------------------------------------------------------------------------

static short RenderSegmentFaces (int nType, short nSegment, int bAutomap)
{
if (nSegment < 0)
	return 0;

	tSegFaces	*segFaceP = SEGFACES + nSegment;
	CSegFace		*faceP;
	short			nFaces = 0;
	int			i;

#if DBG
if (nSegment == nDbgSeg)
	nSegment = nSegment;
#endif
if (!VisitSegment (nSegment, bAutomap))
	return 0;
#if DBG
if (nSegment == nDbgSeg)
	nSegment = nSegment;
#endif
if (gameStates.render.bPerPixelLighting == 2)
	lightManager.Index (0)[0].nActive = -1;
for (i = segFaceP->nFaces, faceP = segFaceP->faceP; i; i--, faceP++) {
#if DBG
	if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
		nSegment = nSegment;
#endif
	if (faceP->m_info.bVisible && RenderMineFace (SEGMENTS + nSegment, faceP, nType))
		nFaces++;
	}
return nFaces;
}

//------------------------------------------------------------------------------

short RenderSegments (int nType)
{
	int	i, nFaces = 0, bAutomap = (nType <= RENDER_STATIC_FACES);

if (nType >= RENDER_DYNAMIC_FACES) {
	// render mine segment by segment
	if (gameData.render.mine.nRenderSegs == gameData.segs.nSegments) {
		CSegFace *faceP = FACES.faces.Buffer ();
		for (i = gameData.segs.nFaces; i; i--, faceP++)
			if (RenderMineFace (SEGMENTS + faceP->m_info.nSegment, faceP, nType))
				nFaces++;
		}
	else {
		for (i = gameData.render.mine.nRenderSegs; i; )
			nFaces += RenderSegmentFaces (nType, gameData.render.mine.nSegRenderList [0][--i], bAutomap);
		}
	}
else {
	// render mine by pre-sorted textures
	nFaces = RenderFaceList (nType);
	}
return nFaces;
}

//------------------------------------------------------------------------------

void RenderHeadlights (int nType)
{
#if 0
if (gameStates.render.bPerPixelLighting && gameStates.render.bHeadlights) {
	ogl.SetBlendMode (GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	faceRenderFunc = lightmapManager.HaveLightmaps () ? RenderHeadlightsPP : RenderHeadlightsVL;
	RenderSegments (nType, 0, 1);
	SetFaceDrawer (-1);
	}
#endif
}

//------------------------------------------------------------------------------

int SetupCoronas (int nType)
{
SetupCoronaFaces ();
return 0;
}

//------------------------------------------------------------------------------

int SetupDepthBuffer (int nType)
{
BeginRenderFaces (nType, 1);
RenderSegments (nType);
EndRenderFaces (nType);
return SortFaces ();
}

//------------------------------------------------------------------------------

void RenderFaceList (int nType, int bFrontToBack)
{
if (nType > RENDER_OBJECTS) {	//back to front
	BeginRenderFaces (nType, 0);
	RenderSegments (nType);
	}
else {	//front to back
	if ((nType == RENDER_STATIC_FACES) && !gameStates.render.nWindow)
		SetupCoronas (nType);
	BeginRenderFaces (nType, 0);
	ogl.ColorMask (1,1,1,1,1);
	gameData.render.mine.nVisited++;
	RenderSegments (nType);
	ogl.SetDepthWrite (true);
	RenderHeadlights (nType);
	}
EndRenderFaces (0);
}

//------------------------------------------------------------------------------
// eof
