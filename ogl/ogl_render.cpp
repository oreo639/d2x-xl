/*
 *
 * Graphics support functions for OpenGL.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef _WIN32
#	include <windows.h>
#	include <stddef.h>
#	include <io.h>
#endif
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef __macosx__
# include <stdlib.h>
# include <SDL/SDL.h>
#else
# include <SDL.h>
#endif

#include "descent.h"
#include "error.h"
#include "maths.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_color.h"
#include "ogl_shader.h"
#include "ogl_hudstuff.h"
#include "ogl_render.h"
#include "ogl_tmu.h"
#include "light.h"
#include "dynlight.h"
#include "lightmap.h"
#include "texmerge.h"
#include "glare.h"
#include "renderlib.h"
#include "transprender.h"

//------------------------------------------------------------------------------

#define OGL_CLEANUP			1
#define USE_VERTNORMS		1
#define G3_DRAW_ARRAYS		0
#define G3_MULTI_TEXTURE	0

//------------------------------------------------------------------------------

GLhandleARB	lmProg = (GLhandleARB) 0;
GLhandleARB	activeShaderProg = (GLhandleARB) 0;

tTexPolyMultiDrawer	*fpDrawTexPolyMulti = NULL;

//------------------------------------------------------------------------------

inline void SetTexCoord (tUVL *uvlList, int nOrient, int bMulti, tTexCoord2f *texCoord, int bMask)
{
	float u1, v1;

if (nOrient == 1) {
	u1 = 1.0f - X2F (uvlList->v);
	v1 = X2F (uvlList->u);
	}
else if (nOrient == 2) {
	u1 = 1.0f - X2F (uvlList->u);
	v1 = 1.0f - X2F (uvlList->v);
	}
else if (nOrient == 3) {
	u1 = X2F (uvlList->v);
	v1 = 1.0f - X2F (uvlList->u);
	}
else {
	u1 = X2F (uvlList->u);
	v1 = X2F (uvlList->v);
	}
if (texCoord) {
	texCoord->v.u = u1;
	texCoord->v.v = v1;
	}
else {
#if OGL_MULTI_TEXTURING
	if (bMulti) {
		glMultiTexCoord2f (GL_TEXTURE1, u1, v1);
		if (bMask)
			glMultiTexCoord2f (GL_TEXTURE2, u1, v1);
	}
else
#endif
		glTexCoord2f (u1, v1);
	}
}

//------------------------------------------------------------------------------

inline int G3BindTex (CBitmap *bmP, GLint nTexId, GLhandleARB lmProg, char *pszTexId,
						    pInitTMU initTMU, int bShaderVar, int bVertexArrays)
{
if (bmP || (nTexId >= 0)) {
	initTMU (bVertexArrays);
	if (nTexId >= 0)
		ogl.BindTexture (nTexId);
	else {
		if (bmP->Bind (1))
			return 1;
		bmP->Texture ()->Wrap (GL_REPEAT);
		}
	if (bShaderVar)
		glUniform1i (glGetUniformLocation (lmProg, pszTexId), 0);
	}
return 0;
}

//------------------------------------------------------------------------------

int G3DrawLine (g3sPoint *p0, g3sPoint *p1)
{
if (ogl.SizeVertexBuffer (2)) {
	ogl.SetTextureUsage (false);
	OglCanvasColor (&CCanvas::Current ()->Color ());
	ogl.VertexBuffer () [0].Assign (p0->p3_vec);
	ogl.VertexBuffer () [1].Assign (p1->p3_vec);
	ogl.FlushBuffers (GL_LINES, 2, 3);
	if (CCanvas::Current ()->Color ().rgb)
		ogl.SetBlending (false);
	}
return 1;
}

//------------------------------------------------------------------------------

void OglDrawEllipse (int nSides, int nType, float xsc, float xo, float ysc, float yo, tSinCosf *sinCosP)
{
	int		i;
	double	ang;

glPushMatrix ();
ogl.SetLineSmooth (true);
glTranslatef (xo, yo, 0.0f);
glScalef (xsc, ysc, 1.0f);
if (nType == GL_LINES) {	// implies a dashed circle
	if (ogl.SizeVertexBuffer (nSides * 2)) {
		if (sinCosP) {
			for (i = 0; i < nSides; i++, sinCosP++) {
				ogl.VertexBuffer () [i][X] = sinCosP->fCos;
				ogl.VertexBuffer () [i][Y] = sinCosP->fSin;
				i++, sinCosP++;
				ogl.VertexBuffer () [i][X] = sinCosP->fCos;
				ogl.VertexBuffer () [i][Y] = sinCosP->fSin;
				}
			}
		else {
			for (i = 0; i < nSides; i++) {
				ang = 2.0 * Pi * i / nSides;
				ogl.VertexBuffer () [i][X] = float (cos (ang));
				ogl.VertexBuffer () [i][Y] = float (sin (ang));
				i++;
				ang = 2.0 * Pi * i / nSides;
				ogl.VertexBuffer () [i][X] = float (cos (ang));
				ogl.VertexBuffer () [i][Y] = float (sin (ang));
				}
			}
		ogl.FlushBuffers (GL_LINES, nSides * 2, 2);
		}
	}
else {
	if (sinCosP) {
		ogl.EnableClientStates (0, 0, 0, GL_TEXTURE0);
		OglVertexPointer (2, GL_FLOAT, 2 * sizeof (float), reinterpret_cast<GLfloat*> (sinCosP));
		OglDrawArrays (nType, 0, nSides);
		ogl.DisableClientStates (0, 0, 0, GL_TEXTURE0);
		}
	else {
		if (ogl.SizeVertexBuffer (nSides)) {
			for (i = 0; i < nSides; i++) {
				ang = 2.0 * Pi * i / nSides;
				ogl.VertexBuffer () [i][X] = float (cos (ang));
				ogl.VertexBuffer () [i][Y] = float (sin (ang));
				}
			ogl.FlushBuffers (nType, nSides, 2);
			}
		}
	}
ogl.SetLineSmooth (false);
glPopMatrix ();
}

//------------------------------------------------------------------------------

void OglDrawCircle (int nSides, int nType)
{
	int		i;
	double	ang;

if (ogl.SizeVertexBuffer (nSides)) {
	for (i = 0; i < nSides; i++) {
		ang = 2.0 * Pi * i / nSides;
		ogl.VertexBuffer () [i][X] = float (cos (ang));
		ogl.VertexBuffer () [i][Y] = float (sin (ang));
		}
	ogl.FlushBuffers (nType, nSides, 2);
	}
}

//------------------------------------------------------------------------------

int G3DrawSphere (g3sPoint *pnt, fix rad, int bBigSphere)
{
	double r;

ogl.SetTextureUsage (false);
OglCanvasColor (&CCanvas::Current ()->Color ());
glPushMatrix ();
glTranslatef (X2F (pnt->p3_vec [X]), X2F (pnt->p3_vec [Y]), X2F (pnt->p3_vec [Z]));
r = X2F (rad);
glScaled (r, r, r);
if (bBigSphere) {
#if 1
	OglDrawCircle (20, GL_POLYGON);
#else
	if (hBigSphere)
		glCallList (hBigSphere);
	else
		hBigSphere = CircleListInit (20, GL_POLYGON, GL_COMPILE_AND_EXECUTE);
#endif
	}
else {
	if (hSmallSphere)
		glCallList (hSmallSphere);
	else
		hSmallSphere = CircleListInit (12, GL_POLYGON, GL_COMPILE_AND_EXECUTE);
	}
glPopMatrix ();
if (CCanvas::Current ()->Color ().rgb)
	ogl.SetBlending (false);
return 0;
}

//------------------------------------------------------------------------------

int G3DrawSphere3D (g3sPoint *p0, int nSides, int rad)
{
	tCanvasColor	c = CCanvas::Current ()->Color ();
	g3sPoint			p = *p0;
	int				i;
	CFloatVector	v;
	float				x, y, z, r;
	float				ang;

if (ogl.SizeVertexBuffer (nSides + 1)) {
	ogl.SetTextureUsage (false);
	OglCanvasColor (&CCanvas::Current ()->Color ());
	x = X2F (p.p3_vec [X]);
	y = X2F (p.p3_vec [Y]);
	z = X2F (p.p3_vec [Z]);
	r = X2F (rad);
	v [Z] = z;
	for (i = 0; i <= nSides; i++) {
		ang = 2.0f * float (Pi * (i % nSides) / nSides);
		v [X] = x + float (cos (ang) * r);
		v [Y] = y + float (sin (ang) * r);
		ogl.VertexBuffer () [i] = v;
		}
	ogl.FlushBuffers (GL_POLYGON, nSides + 1);
	if (c.rgb)
		ogl.SetBlending (false);
	}
return 1;
}

//------------------------------------------------------------------------------

int G3DrawCircle3D (g3sPoint *p0, int nSides, int rad)
{
	g3sPoint			p = *p0;
	int				i, j;
	CFloatVector	v;
	float				x, y, r;
	float				ang;

if (ogl.SizeVertexBuffer (2 * (nSides + 1))) {
	ogl.SetTextureUsage (false);
	OglCanvasColor (&CCanvas::Current ()->Color ());
	x = X2F (p.p3_vec [X]);
	y = X2F (p.p3_vec [Y]);
	v[Z] = X2F (p.p3_vec [Z]);
	r = X2F (rad);
	for (i = 0; i <= nSides; i++) {
		for (j = i; j <= i + 1; j++) {
			ang = 2.0f * (float) Pi * (j % nSides) / nSides;
			v [X] = x + (float) cos (ang) * r;
			v [Y] = y + (float) sin (ang) * r;
			ogl.VertexBuffer () [i] = v;
			}
		}
	ogl.FlushBuffers (GL_LINES, 2 * (nSides + 1), 2);
	if (CCanvas::Current ()->Color ().rgb)
		ogl.SetBlending (false);
	}
return 1;
}

//------------------------------------------------------------------------------

int GrUCircle (fix xc1, fix yc1, fix r1)
{//dunno if this really works, radar doesn't seem to.. hm..
ogl.SetTextureUsage (false);
//	glPointSize (X2F (rad);
OglCanvasColor (&CCanvas::Current ()->Color ());
glPushMatrix ();
glTranslatef ((X2F (xc1) + CCanvas::Current ()->Left ()) / (float) ogl.m_states.nLastW,
				  1.0f - (X2F (yc1) + CCanvas::Current ()->Top ()) / (float) ogl.m_states.nLastH, 0);
glScalef (X2F (r1), X2F (r1), X2F (r1));
if (r1<=I2X (5)){
	if (!circleh5)
		circleh5 = CircleListInit (5, GL_LINE_LOOP, GL_COMPILE_AND_EXECUTE);
	else
		glCallList (circleh5);
	}
else{
	if (!circleh10)
		circleh10 = CircleListInit (10, GL_LINE_LOOP, GL_COMPILE_AND_EXECUTE);
	else
		glCallList (circleh10);
}
glPopMatrix ();
if (CCanvas::Current ()->Color ().rgb)
	ogl.SetBlending (false);
return 0;
}

//------------------------------------------------------------------------------

int G3DrawWhitePoly (int nVertices, g3sPoint **pointList)
{
	int			i;
	g3sPoint*	p;

if (ogl.SizeVertexBuffer (nVertices)) {
	ogl.SetTextureUsage (false);
	ogl.SetBlending (false);
	glColor4d (1.0, 1.0, 1.0, 1.0);
	for (i = 0; i < nVertices; i++) {
		p = pointList [i];
		if (p->p3_index < 0)
			ogl.VertexBuffer () [i].Assign ((*pointList)->p3_vec);
		else
			ogl.VertexBuffer () [i] = gameData.render.vertP [p->p3_index];
		}
	ogl.FlushBuffers (GL_TRIANGLE_FAN, nVertices);
	}
return 0;
}

//------------------------------------------------------------------------------

int G3DrawPoly (int nVertices, g3sPoint **pointList)
{
	int			i;
	g3sPoint*	p;

if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (nVertices, pointList);
	return 0;
	}
if (ogl.SizeVertexBuffer (nVertices)) {
	ogl.SetTextureUsage (false);
	OglCanvasColor (&CCanvas::Current ()->Color ());
	for (i = 0; i < nVertices; i++) {
		p = pointList [i];
		if (p->p3_index < 0)
			ogl.VertexBuffer () [i].Assign ((*pointList)->p3_vec);
		else
			ogl.VertexBuffer () [i] = gameData.render.vertP [p->p3_index];
		}
	ogl.FlushBuffers (GL_TRIANGLE_FAN, nVertices);
	if (CCanvas::Current ()->Color ().rgb || (gameStates.render.grAlpha < 1.0f))
		ogl.SetBlending (false);
	}
return 0;
}

//------------------------------------------------------------------------------

int G3DrawPolyAlpha (int nVertices, g3sPoint **pointList, tRgbaColorf *color, char bDepthMask, short nSegment)
{
if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (nVertices, pointList);
	return 0;
	}
if (color->alpha < 0)
	color->alpha = gameStates.render.grAlpha;
CFloatVector	vertices [8];

for (int i = 0; i < nVertices; i++)
	vertices [i] = gameData.render.vertP [pointList [i]->p3_index];
transparencyRenderer.AddPoly (NULL, NULL, NULL, vertices, nVertices, NULL, color, NULL, 1, bDepthMask, GL_TRIANGLE_FAN, GL_REPEAT, 0, nSegment);
return 0;
}

//------------------------------------------------------------------------------

void gr_upoly_tmap (int nverts, int *vert )
{
#if TRACE
console.printf (CON_DBG, "gr_upoly_tmap: unhandled\n");//should never get called
#endif
}

//------------------------------------------------------------------------------

void DrawTexPolyFlat (CBitmap *bmP, int nVertices, g3sPoint **vertlist)
{
#if TRACE
console.printf (CON_DBG, "DrawTexPolyFlat: unhandled\n");//should never get called
#endif
}

//------------------------------------------------------------------------------

int G3DrawTexPolyFlat (
	int			nVertices,
	g3sPoint**	pointList,
	tUVL*			uvlList,
	tUVL*			uvlLMap,
	CBitmap*		bmBot,
	CBitmap*		bmTop,
	tLightmap*	lightmap,
	CFixVector*	pvNormal,
	int			orient,
	int			bBlend,
	short			nSegment)
{
	int			i;
	g3sPoint*	p;

if (ogl.SizeVertexBuffer (nVertices)) {
	if (FAST_SHADOWS) {
		if (bBlend)
			ogl.SetBlendMode (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			ogl.SetBlending (false);
		}
	else {
		if (gameStates.render.nShadowPass == 3) {
			ogl.SetBlending (true);
			ogl.SetBlendMode (GL_ONE, GL_ONE);
			}
		}
	ogl.SelectTMU (GL_TEXTURE0);
	ogl.SetTextureUsage (false);
	glColor4d (0, 0, 0, gameStates.render.grAlpha);
	for (i = 0; i < nVertices; i++) {
		p = pointList [i];
		if (p->p3_index < 0)
			ogl.VertexBuffer () [i].Assign ((*pointList)->p3_vec);
		else
			ogl.VertexBuffer () [i] = gameData.render.vertP [p->p3_index];
		}
	ogl.FlushBuffers (GL_TRIANGLE_FAN, nVertices);
	}
return 0;
}

//------------------------------------------------------------------------------

#define	G3VERTPOS(_dest,_src) \
			if ((_src)->p3_index < 0) \
				(_dest).Assign ((_src)->p3_vec); \
			else \
				_dest = gameData.render.vertP [(_src)->p3_index];

#define	G3VERTPOS3(_dest,_src) \
			if ((_src)->p3_index < 0) \
				(_dest).Assign ((_src)->p3_vec); \
			else \
				_dest = gameData.render.vertP [(_src)->p3_index];

//------------------------------------------------------------------------------

int G3DrawTexPolyMulti (
	int			nVertices,
	g3sPoint**	pointList,
	tUVL*			uvlList,
	tUVL*			uvlLMap,
	CBitmap*		bmBot,
	CBitmap*		bmTop,
	tLightmap*	lightmap,
	CFixVector*	pvNormal,
	int			orient,
	int			bBlend,
	short			nSegment)
{
	int			i, nShader, nFrame;
	int			bShaderMerge = 0,
					bSuperTransp = 0;
	int			bLight = 1,
					bDynLight = gameStates.render.bApplyDynLight && (gameStates.app.bEndLevelSequence < EL_OUTSIDE),
					bDepthSort,
					bResetColor = 0,
					bOverlay = 0;
	tFaceColor	*pc;
	CBitmap		*bmP = NULL, *mask = NULL;
	g3sPoint		*pl, **ppl;
#if USE_VERTNORMS
	CFloatVector	vNormal, vVertPos;
#endif
#if G3_DRAW_ARRAYS
	int			bVertexArrays = gameData.render.vertP != NULL;
#else
	int			bVertexArrays = 0;
#endif

if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (nVertices, pointList);
	return 0;
	}
if (!bmBot)
	return 1;
r_tpolyc++;
if (FAST_SHADOWS) {
	if (!bBlend)
		ogl.SetBlending (false);
#if 0
	else
		ogl.SetBlendMode (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
	}
else {
	if (gameStates.render.nShadowPass == 1)
		bLight = !bDynLight;
	else if (gameStates.render.nShadowPass == 3) {
		ogl.SetBlending (true);
		ogl.SetBlendMode (GL_ONE, GL_ONE);
		}
	}
ogl.SetDepthMode (GL_LEQUAL);
bmBot = bmBot->Override (-1);
bDepthSort = (!bmTop && 
				  ((gameStates.render.grAlpha < 1.0f) ||
				   (bmBot->Flags () & (BM_FLAG_TRANSPARENT | BM_FLAG_SEE_THRU | BM_FLAG_TGA)) == (BM_FLAG_TRANSPARENT | BM_FLAG_TGA)));
if (bmTop && (bmTop = bmTop->Override (-1)) && bmTop->Frames ()) {
	nFrame = (int) (bmTop->CurFrame () - bmTop->Frames ());
	bmP = bmTop;
	bmTop = bmTop->CurFrame ();
	}
else
	nFrame = -1;
if (bmTop) {
	if (nFrame < 0)
      bSuperTransp = (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
	else
		bSuperTransp = (bmP->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0;
	bShaderMerge = bSuperTransp && ogl.m_states.bGlTexMerge;
	bOverlay = !bShaderMerge;
	}
else
	bOverlay = -1;
#if G3_DRAW_ARRAYS
retry:
#endif
if (bShaderMerge) {
	mask = gameStates.render.textures.bHaveMaskShader ? bmTop->Mask () : NULL;
	nShader = bSuperTransp ? mask ? 2 : 1 : 0;
	activeShaderProg = GLhandleARB (abs (int (shaderManager.Deploy (tmShaderProgs [nShader]))));
	INIT_TMU (InitTMU0, GL_TEXTURE0, bmBot, lightmapManager.Buffer (), bVertexArrays, 0);
	glUniform1i (glGetUniformLocation (activeShaderProg, "btmTex"), 0);
	INIT_TMU (InitTMU1, GL_TEXTURE1, bmTop, lightmapManager.Buffer (), bVertexArrays, 0);
	glUniform1i (glGetUniformLocation (activeShaderProg, "topTex"), 1);
	if (mask) {
#if DBG
		InitTMU2 (bVertexArrays);
		G3_BIND (GL_TEXTURE2, mask, lightmapManager.Buffer (), bVertexArrays);
#else
		INIT_TMU (InitTMU2, GL_TEXTURE2, mask, lightmapManager.Buffer (), bVertexArrays, 0);
#endif
		glUniform1i (glGetUniformLocation (activeShaderProg, "maskTex"), 2);
		}
	glUniform1f (glGetUniformLocation (activeShaderProg, "grAlpha"), gameStates.render.grAlpha);
	}
else if (!bDepthSort) {
	if (bmBot == gameData.endLevel.satellite.bmP) {
		ogl.SelectTMU (GL_TEXTURE0);
		ogl.SetTextureUsage (true);
		}
	else
		InitTMU0 (bVertexArrays);
	if (bmBot->Bind (1))
		return 1;
	bmBot = bmBot->CurFrame (-1);
	bmBot->Texture ()->Wrap ((bmBot == bmpDeadzone) ? GL_CLAMP : GL_REPEAT);
	}

if (!bDepthSort) {
	if (SHOW_DYN_LIGHT) {
#if USE_VERTNORMS
		if (pvNormal) {
			vNormal.Assign (*pvNormal);
			transformation.Rotate(vNormal, vNormal, 0);
			}
	else
			G3CalcNormal (pointList, &vNormal);
#else
			G3Normal (pointList, pvNormal);
#endif
		}
	if (gameStates.render.bFullBright) {
		glColor3f (1,1,1);
		bLight = 0;
		}
	else if (!gameStates.render.nRenderPass)
		bLight = 0;
	else if (!bLight)
		glColor3i (0,0,0);
	if (!bLight)
		bDynLight = 0;
	ogl.m_states.bDynObjLight = bDynLight;
	}

ogl.m_states.fAlpha = gameStates.render.grAlpha;
if (bVertexArrays || bDepthSort) {
		CFloatVector	vertices [8];
		tFaceColor		vertColors [8];
		tTexCoord2f		texCoord [2][8];
		int				vertIndex [8];
		//int				colorIndex [8];

	for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
		pl = *ppl;
		vertIndex [i] = pl->p3_index;
		//colorIndex [i] = i;
		if (pl->p3_index < 0)
			vertices[i].Assign (pl->p3_vec);
		else
			vertices [i] = gameData.render.vertP [pl->p3_index];
		texCoord [0][i].v.u = X2F (uvlList [i].u);
		texCoord [0][i].v.v = X2F (uvlList [i].v);
		SetTexCoord (uvlList + i, orient, 1, texCoord [1] + i, 0);
		G3VERTPOS (vVertPos, pl);
		if (bDynLight)
			G3VertexColor (G3GetNormal (pl, &vNormal), vVertPos.XYZ (), vertIndex [i], vertColors + i, NULL,
								gameStates.render.nState ? X2F (uvlList [i].l) : 1, 0, 0);
		else if (bLight)
			SetTMapColor (uvlList + i, i, bmBot, !bOverlay, &vertColors [i].color);
		}
	bmBot->SetupTexture (1, 0);
	transparencyRenderer.AddPoly (NULL, NULL, bmBot, vertices, nVertices, texCoord [0], NULL, vertColors, nVertices, 1, GL_TRIANGLE_FAN, GL_REPEAT, 0, nSegment);
	return 0;
	}
#if G3_DRAW_ARRAYS
if (bVertexArrays) {
	if (!ogl.EnableClientStates (1, 1, 0, GL_TEXTURE0)) {
		bVertexArrays = 0;
		goto retry;
		}
	OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), vertices);
//	OglIndexPointer (GL_INT, 0, colorIndex);
	OglTexCoordPointer (2, GL_FLOAT, sizeof (tTexCoord3f), texCoord [0]);
	if (bLight)
		OglColorPointer (4, GL_FLOAT, sizeof (tFaceColor), vertColors);
	if (bmTop && !bOverlay) {
		if (!ogl.EnableClientStates (1, 1, 0, GL_TEXTURE1)) {
			ogl.DisableClientStates (1, 1, 0, GL_TEXTURE0);
			bVertexArrays = 0;
			goto retry;
			}
		OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), vertices);
		if (bLight)
			OglColorPointer (4, GL_FLOAT, sizeof (tFaceColor), vertColors);
//		OglIndexPointer (GL_INT, 0, colorIndex);
		OglTexCoordPointer (2, GL_FLOAT, sizeof (tTexCoord3f), texCoord [1]);
		}
	OglDrawArrays (GL_TRIANGLE_FAN, 0, nVertices);
	ogl.DisableClientStates (1, 1, 0, GL_TEXTURE0);
	if (bmTop && !bOverlay)
		ogl.DisableClientStates (GL_TEXTURE1);
	}
#endif
else if (ogl.SizeBuffers (nVertices)) {
	tFaceColor	faceColor;

	if (bDynLight) {
		if (bOverlay) {
			for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
				pl = *ppl;
				G3VERTPOS (vVertPos, pl);
				G3VertexColor (G3GetNormal (pl, &vNormal), vVertPos.XYZ (), pl->p3_index, NULL, &faceColor,
									gameStates.render.nState ? X2F (uvlList [i].l) : 1, 0, 0);
				ogl.ColorBuffer () [i] = faceColor.color;
				ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
				ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
				ogl.VertexBuffer () [i] = vVertPos;
				}
			}
		else {
			for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
				pl = *ppl;
				G3VERTPOS (vVertPos, pl);
				G3VertexColor (G3GetNormal (pl, &vNormal), vVertPos.XYZ (), pl->p3_index, NULL, &faceColor, 1, 0, 0);
				ogl.ColorBuffer () [i] = faceColor.color;
				ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
				ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
				SetTexCoord (uvlList + i, orient, 1, ogl.TexCoordBuffer (1) + i, mask != NULL);
				ogl.VertexBuffer () [i] = vVertPos;
				}
			}
		}
	else if (bLight) {
		if (bOverlay) {
			for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
				if (gameStates.render.nState || gameStates.app.bEndLevelSequence)
					SetTMapColor (uvlList + i, i, bmBot, 1, ogl.ColorBuffer () + i);
				else {
					pc = gameData.render.color.vertices + (*ppl)->p3_index;
					glColor3fv (reinterpret_cast<GLfloat*> (&pc->color));
					}
				ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
				ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
				OglVertex3f (*ppl, ogl.VertexBuffer () + i);
				}
			}
		else {
			bResetColor = (bOverlay != 1);
			for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
				if (gameStates.render.nState)
					SetTMapColor (uvlList + i, i, bmBot, 1, ogl.ColorBuffer () + i);
				else {
					pc = gameData.render.color.vertices + (*ppl)->p3_index;
					ogl.ColorBuffer () [i] = pc->color;
					}
				ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
				ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
				SetTexCoord (uvlList + i, orient, 1, ogl.TexCoordBuffer (1) + i, mask != NULL);
				OglVertex3f (*ppl, ogl.VertexBuffer () + i);
				}
			}
		}
	else {
		if (bOverlay) {
			for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
				ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
				ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
				OglVertex3f (*ppl, ogl.VertexBuffer () + i);
				}
			}
		else {
			for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
				ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
				ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
				SetTexCoord (uvlList + i, orient, 1, ogl.TexCoordBuffer (1) + i, mask != NULL);
				OglVertex3f (*ppl, ogl.VertexBuffer () + i);
				}
			}
		}
	if (bOverlay) {
		ogl.EnableClientStates (1, 1, 0, 1);
		OglColorPointer (4, GL_FLOAT, 0, &ogl.ColorBuffer ());
		OglTexCoordPointer (2, GL_FLOAT, 0, ogl.TexCoordBuffer (1).Buffer ());
		ogl.EnableClientStates (1, 0, 0, 2);
		OglTexCoordPointer (2, GL_FLOAT, 0, ogl.TexCoordBuffer (1).Buffer ());
		}
	ogl.FlushBuffers (GL_TRIANGLE_FAN, nVertices, 1, bDynLight || bLight);
	if (bOverlay) {
		ogl.DisableClientStates (1, 0, 0, 2);
		ogl.DisableClientStates (1, 1, 0, 1);
		ogl.SelectTMU (0);
		}
	}
if ((bOverlay > 0) && ogl.SizeBuffers (nVertices)) {
	tFaceColor	faceColor;

	ogl.SelectTMU (GL_TEXTURE0);
	ogl.SetTextureUsage (true);
	if (bmTop->Bind (1))
		return 1;
	bmTop = bmTop->CurFrame (-1);
	bmTop->Texture ()->Wrap (GL_REPEAT);

	if (bDynLight) {
		for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
			vVertPos.Assign ((*ppl)->p3_vec);
			G3VertexColor (G3GetNormal (*ppl, &vNormal), vVertPos.XYZ (), (*ppl)->p3_index, NULL, &faceColor, 1, 0, 0);
			ogl.ColorBuffer () [i] = faceColor.color;
			SetTexCoord (uvlList + i, orient, 0, ogl.TexCoordBuffer () + i, mask != NULL);
			OglVertex3f (*ppl, ogl.VertexBuffer () + i);
			}
		}
	else if (bLight) {
		for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
			SetTMapColor (uvlList + i, i, bmTop, 1, ogl.ColorBuffer () + i);
			SetTexCoord (uvlList + i, orient, 0, ogl.TexCoordBuffer () + i, mask != NULL);
			OglVertex3f (*ppl, ogl.VertexBuffer () + i);
			}
		}
	else {
		for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
			SetTexCoord (uvlList + i, orient, 0, ogl.TexCoordBuffer () + i, mask != NULL);
			OglVertex3f (*ppl, ogl.VertexBuffer () + i);
			}
		}
	ogl.FlushBuffers (GL_TRIANGLE_FAN, nVertices, 1, bDynLight || bLight);

	ogl.SetDepthMode (GL_LESS);
#if OGL_CLEANUP
	ogl.BindTexture (0);
	ogl.SetTextureUsage (false);
#endif
	}
else if (bShaderMerge) {
#if OGL_CLEANUP
	ogl.SelectTMU (GL_TEXTURE1, bVertexArrays != 0);
	ogl.BindTexture (0);
	ogl.SetTextureUsage (false); // Disable the 2nd texture
#endif
	activeShaderProg = 0;
	shaderManager.Deploy (-1);
	}
ogl.SelectTMU (GL_TEXTURE0, bVertexArrays != 0);
ogl.BindTexture (0);
ogl.SetTextureUsage (false);
tMapColor.index =
lightColor.index = 0;
if (!bBlend)
	ogl.SetBlending (true);
return 0;
}

//------------------------------------------------------------------------------

int G3DrawTexPolyLightmap (
	int			nVertices,
	g3sPoint**	pointList,
	tUVL*			uvlList,
	tUVL*			uvlLMap,
	CBitmap*		bmBot,
	CBitmap*		bmTop,
	tLightmap*	lightmap,
	CFixVector*	pvNormal,
	int			orient,
	int			bBlend,
	short			nSegment)
{
	int			i, nFrame, bShaderMerge;
	CBitmap	*bmP = NULL;
	g3sPoint		**ppl;

if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (nVertices, pointList);
	return 0;
	}
if (!bmBot)
	return 1;
r_tpolyc++;
//if (gameStates.render.nShadowPass != 3)
	ogl.SetDepthMode (GL_LEQUAL);
if (FAST_SHADOWS) {
	if (bBlend)
		ogl.SetBlendMode (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	else
		ogl.SetBlending (false);
	}
else {
	if (gameStates.render.nShadowPass == 3) {
		ogl.SetBlending (true);
		ogl.SetBlendMode (GL_ONE, GL_ONE);
		}
	}
ogl.SetDepthMode (GL_LEQUAL);
bmBot = bmBot->Override (-1);
if (bmTop && (bmTop = bmTop->Override (-1)) && bmTop->Frames ()) {
	nFrame = (int) (bmTop->CurFrame () - bmTop->Frames ());
	bmP = bmTop;
	bmTop = bmTop->CurFrame ();
	}
else
	nFrame = -1;
if (!lightmap) //lightmapping enabled
	return G3DrawTexPolyMulti (nVertices, pointList, uvlList, uvlLMap, bmBot, bmTop, lightmap, pvNormal, orient, bBlend, nSegment);
// chose shaders depending on whether overlay bitmap present or not
if ((bShaderMerge = bmTop && gameOpts->ogl.bGlTexMerge)) {
	lmProg = lmShaderProgs [(bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0];
	glUseProgramObject (lmProg);
	}
InitTMU0 (0);	// use render pipeline 0 for bottom texture
if (bmBot->Bind (1))
	return 1;
bmBot = bmBot->CurFrame (-1);
bmBot->Texture ()->Wrap (GL_REPEAT);
if (bShaderMerge)
	glUniform1i (glGetUniformLocation (lmProg, "btmTex"), 0);
if (bmTop) { // use render pipeline 1 for overlay texture
	InitTMU1 (0);
	if (bmTop->Bind (1))
		return 1;
	bmTop = bmTop->CurFrame (-1);
	bmTop->Texture ()->Wrap (GL_REPEAT);
	glUniform1i (glGetUniformLocation (lmProg, "topTex"), 1);
	}
// use render pipeline 2 for lightmap texture
if (ogl.SizeBuffers (nVertices)) {
	InitTMU2 (0);
	//ogl.BindTexture (lightmap->handle);
	if (bShaderMerge)
		glUniform1i (glGetUniformLocation (lmProg, "lMapTex"), 2);
	ppl = pointList;
	if (gameStates.render.bFullBright)
		glColor3d (1,1,1);
	for (i = 0; i < nVertices; i++, ppl++) {
		if (!gameStates.render.bFullBright)
			SetTMapColor (uvlList + i, i, bmBot, 1, ogl.ColorBuffer () + i);
		ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
		ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
		if (bmTop)
			SetTexCoord (uvlList + i, orient, 1, ogl.TexCoordBuffer (1) + i, 0);
		glMultiTexCoord2f (GL_TEXTURE2, X2F (uvlLMap [i].u), X2F (uvlLMap [i].v));
		OglVertex3f (*ppl, ogl.VertexBuffer () + i);
		}
	ogl.EnableClientStates (1, 1, 0, 1);
	OglColorPointer (4, GL_FLOAT, 0, &ogl.ColorBuffer ());
	OglTexCoordPointer (2, GL_FLOAT, 0, ogl.TexCoordBuffer (1).Buffer ());
	ogl.EnableClientStates (1, 0, 0, 2);
	OglTexCoordPointer (2, GL_FLOAT, 0, &ogl.TexCoordBuffer ());
	ogl.FlushBuffers (GL_TRIANGLE_FAN, nVertices, 1, !gameStates.render.bFullBright);
	ogl.DisableClientStates (1, 0, 0, 2);
	ogl.DisableClientStates (1, 1, 0, 1);
	ogl.SelectTMU (0);
	}
ExitTMU (0);
if (bShaderMerge)
	glUseProgramObject (lmProg = 0);
return 0;
}

//------------------------------------------------------------------------------

int G3DrawTexPolySimple (
	int			nVertices,
	g3sPoint		**pointList,
	tUVL			*uvlList,
	CBitmap	*bmP,
	CFixVector	*pvNormal,
	int			bBlend)
{
	int			i;
	int			bLight = 1,
					bDynLight = gameStates.render.bApplyDynLight && !gameStates.app.bEndLevelSequence;
	g3sPoint		*pl, **ppl;
#if USE_VERTNORMS
	CFloatVector		vNormal, vVertPos;
#endif

if (gameStates.render.nShadowBlurPass == 1) {
	G3DrawWhitePoly (nVertices, pointList);
	return 0;
	}
r_tpolyc++;
if (FAST_SHADOWS) {
	if (!bBlend)
		ogl.SetBlending (false);
#if 0
	else
		ogl.SetBlendMode (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
	}
else {
	if (gameStates.render.nShadowPass == 1)
		bLight = !bDynLight;
	else if (gameStates.render.nShadowPass == 3) {
		ogl.SetBlending (true);
		ogl.SetBlendMode (GL_ONE, GL_ONE);
		}
	}
ogl.SetDepthMode (GL_LEQUAL);
bmP = bmP->Override (-1);
if (bmP == gameData.endLevel.satellite.bmP) {
	ogl.SelectTMU (GL_TEXTURE0);
	ogl.SetTextureUsage (true);
	}
else
	InitTMU0 (0);
if (bmP->Bind (1))
	return 1;
if (bmP == bmpDeadzone)
	bmP->Texture ()->Wrap (GL_CLAMP);
else
	bmP->Texture ()->Wrap (GL_REPEAT);

if (SHOW_DYN_LIGHT) {
#if USE_VERTNORMS
	if (pvNormal)
		vNormal.Assign (*pvNormal);
else
		G3CalcNormal (pointList, &vNormal);
#else
		G3Normal (pointList, pvNormal);
#endif
	}
if (gameStates.render.bFullBright) {
	glColor3d (1,1,1);
	bLight = 0;
	}
else if (!gameStates.render.nRenderPass)
	bLight = 0;
else if (!bLight)
	glColor3i (0,0,0);
if (!bLight)
	bDynLight = 0;
ogl.m_states.bDynObjLight = bDynLight;
ogl.m_states.fAlpha = gameStates.render.grAlpha;
if (ogl.SizeBuffers (nVertices)) {
	if (bDynLight) {
		tFaceColor	faceColor;
		for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
			pl = *ppl;
			G3VERTPOS (vVertPos, pl);
			G3VertexColor (G3GetNormal (pl, &vNormal), vVertPos.XYZ (), pl->p3_index, NULL, &faceColor, 1, 0, 0);
			ogl.ColorBuffer () [i] = faceColor.color;
			ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
			ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
			ogl.VertexBuffer () [i] = vVertPos;
			}
		}
	else if (bLight) {
		for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
			SetTMapColor (uvlList + i, i, bmP, 1, ogl.ColorBuffer () + i);
			ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
			ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
			OglVertex3f (*ppl, ogl.VertexBuffer () + i);
			}
		}
	else {
		for (i = 0, ppl = pointList; i < nVertices; i++, ppl++) {
			ogl.TexCoordBuffer () [i].v.u = X2F (uvlList [i].u);
			ogl.TexCoordBuffer () [i].v.v = X2F (uvlList [i].v);
			OglVertex3f (*ppl, ogl.VertexBuffer () + i);
			}
		}
	ogl.FlushBuffers (GL_TRIANGLE_FAN, nVertices);
	}
ogl.SetTextureUsage (false);
tMapColor.index =
lightColor.index = 0;
if (!bBlend)
	ogl.SetBlending (true);
return 0;
}

//------------------------------------------------------------------------------

int COGL::BindBitmap (CBitmap* bmP, int nFrame, int nWrap)
{
if (bmP) {
	ogl.SetTextureUsage (true);
	if (!bmP->IsBound ()) {
		if (bmP->Bind (1))
			return 0;
		bmP = bmP->Override (-1);
		if (bmP->Frames ())
			bmP = bmP->Frames () + nFrame;
		bmP->Texture ()->Wrap (nWrap);
		}
	}
else if (!(ogl.DrawBuffer () && ogl.DrawBuffer ()->IsBound ()))
	ogl.SetTextureUsage (false);
return 1;
}

//------------------------------------------------------------------------------

int COGL::BindBuffers (CFloatVector *vertexP, int nVertices, int nDimensions,
							  tTexCoord2f *texCoordP, 
							  tRgbaColorf *colorP, int nColors,
							   CBitmap *bmP,
							  int nTMU)
{
if (!ogl.EnableClientStates (m_data.bClientTexCoord = texCoordP != NULL, m_data.bClientColor = ((colorP != NULL) && (nColors == nVertices)), 0, nTMU))
	return 0;
if (texCoordP)
	OglTexCoordPointer (2, GL_FLOAT, sizeof (tTexCoord2f), texCoordP);
if (colorP) {
	if (nColors == nVertices)
		OglColorPointer (4, GL_FLOAT, sizeof (tRgbaColorf), colorP);
	else
		glColor4fv (reinterpret_cast<GLfloat*> (colorP));
	}
OglVertexPointer (nDimensions, GL_FLOAT, sizeof (CFloatVector), vertexP);
return 1;
}

//------------------------------------------------------------------------------

void COGL::ReleaseBuffers (void)
{
ogl.DisableClientStates (m_data.bClientTexCoord, m_data.bClientColor, 0);
}

//------------------------------------------------------------------------------

int COGL::RenderArrays (int nPrimitive, 
							   CFloatVector *vertexP, int nVertices, int nDimensions,
							   tTexCoord2f *texCoordP, 
							   tRgbaColorf *colorP, int nColors, 
							   CBitmap *bmP, int nFrame, int nWrap)
{
if (!BindBitmap (bmP, nFrame, nWrap))
	return 0;
if (BindBuffers (vertexP, nVertices, nDimensions, texCoordP, colorP, nColors, bmP, GL_TEXTURE0)) {
	OglDrawArrays (nPrimitive, 0, nVertices);
	ogl.ReleaseBuffers ();
	}
#if GL_FALLBACK
else {
	int i = nVertices;
	glBegin (nPrimitive);
	if (colorP && (nColors == nVertices)) {
		if (bmP) {
			for (i = 0; i < nVertices; i++) {
				glColor4fv (reinterpret_cast<GLfloat*> (colorP + i));
				glVertex3fv (reinterpret_cast<GLfloat*> (vertexP + i));
				glTexCoord2fv (reinterpret_cast<GLfloat*> (texCoordP + i));
				}
			}
		else {
			for (i = 0; i < nVertices; i++) {
				glColor4fv (reinterpret_cast<GLfloat*> (colorP + i));
				glVertex3fv (reinterpret_cast<GLfloat*> (vertexP + i));
				}
			}
		}
	else {
		if (colorP)
			glColor4fv (reinterpret_cast<GLfloat*> (colorP));
		else
			glColor3d (1, 1, 1);
		if (bmP) {
			for (i = 0; i < nVertices; i++) {
				glVertex3fv (reinterpret_cast<GLfloat*> (vertexP + i));
				glTexCoord2fv (reinterpret_cast<GLfloat*> (texCoordP + i));
				}
			}
		else {
			for (i = 0; i < nVertices; i++) {
				glVertex3fv (reinterpret_cast<GLfloat*> (vertexP + i));
				}
			}
		}
	glEnd ();
	}
#endif
return 1;
}

//------------------------------------------------------------------------------

int COGL::RenderQuad (CBitmap* bmP, CFloatVector* vertexP, int nDimensions, tTexCoord2f* texCoordP, tRgbaColorf* colorP, int nColors, int nWrap)
{
if (!bmP)
	ogl.RenderArrays (GL_QUADS, vertexP, 4, nDimensions, texCoordP, colorP, nColors, bmP, 0, GL_CLAMP);
else if (texCoordP)
	RenderArrays (GL_QUADS, vertexP, 4, nDimensions, texCoordP, colorP, nColors, bmP, 0, nWrap);
else {
	GLfloat			u = bmP->Texture ()->U ();
	GLfloat			v = bmP->Texture ()->V ();
	tTexCoord2f		texCoords [4] = {{{0,0}},{{u,0}},{{u,v}},{{0,v}}};

	RenderArrays (GL_QUADS, vertexP, 4, nDimensions, texCoords, colorP, nColors, bmP, 0, nWrap);
	}
return 0;
}

//------------------------------------------------------------------------------

int COGL::RenderQuad (CBitmap* bmP, CFloatVector& vPosf, float width, float height, int nDimensions, int nWrap)
{
CFloatVector verts [4];
verts [0][X] =
verts [3][X] = vPosf [X] - width;
verts [1][X] =
verts [2][X] = vPosf [X] + width;
verts [0][Y] =
verts [1][Y] = vPosf [Y] + height;
verts [2][Y] =
verts [3][Y] = vPosf [Y] - height;
if (nDimensions == 3)
 verts [0][Z] =
 verts [1][Z] =
 verts [2][Z] =
 verts [3][Z] = vPosf [Z];
int nColors = 0;
tRgbaColorf* colorP = bmP ? bmP->GetColor (&nColors) : NULL;
return RenderQuad (bmP, verts, nDimensions, bmP ?  bmP->GetTexCoord () : NULL, colorP, nColors, nWrap);
}

//------------------------------------------------------------------------------

int COGL::RenderBitmap (CBitmap* bmP, const CFixVector& vPos, fix xWidth, fix xHeight, tRgbaColorf* colorP, float alpha, int bAdditive)
{
	CFloatVector	vPosf;
	tRgbaColorf		color = {1, 1, 1, alpha};

SelectTMU (GL_TEXTURE0);
SetBlendMode (bAdditive);
vPosf.Assign (vPos);
transformation.Transform (vPosf, vPosf, 0);
if (gameStates.render.nShadowBlurPass == 1)
	RenderQuad (NULL, vPosf, X2F (xWidth), X2F (xHeight), 2);
else {
	bmP->SetColor (colorP ? colorP : &color);
	RenderQuad (bmP, vPosf, X2F (xWidth), X2F (xHeight));
	}
SetBlendMode (0);
return 0;
}

//------------------------------------------------------------------------------

int COGL::RenderSprite (CBitmap* bmP, const CFixVector& vPos,
								fix xWidth,	fix xHeight,
								float alpha,
								int bAdditive, 
								float fSoftRad)
{
	CFixVector		pv, v1;
	tRgbaColorf*	colorP = bmP->GetColor ();

if (alpha < 1.0f) {
	tRgbaColorf	color;
	if (!colorP) {
		color.red =
		color.green =
		color.blue = 1;
		color.alpha = alpha;
		colorP = &color;
		}
	transparencyRenderer.AddSprite (bmP, vPos, colorP, xWidth, xHeight, 0, bAdditive, fSoftRad);
	}
else {
	ogl.SelectTMU (GL_TEXTURE0);
	v1 = vPos - transformation.m_info.pos;
	pv = transformation.m_info.view [0] * v1;
	CFloatVector vPosf;
	vPosf.Assign (pv);
	if (gameStates.render.nShadowBlurPass == 1) {
		glColor4f (1,1,1,1);
		ogl.RenderQuad (NULL, vPosf, X2F (xWidth), X2F (xHeight), 3);
		}
	else {
		if (!colorP)
			glColor4f (1, 1, 1, alpha);
		SetBlendMode (bAdditive);
		ogl.SetDepthWrite (false);
		ogl.RenderQuad (bmP, vPosf, X2F (xWidth), X2F (xHeight), 3);
		ogl.SetDepthWrite (true);
		SetBlendMode (0);
		}
	}
return 0;
}

//------------------------------------------------------------------------------

void COGL::RenderScreenQuad (int bTextured)
{
	static tTexCoord2f texCoord [4] = {{{0,0}},{{0,1}},{{1,1}},{{1,0}}};

CFloatVector verts [4];
verts [0][X] =
verts [1][X] = 0;
verts [2][X] =
verts [3][X] = 1;
verts [0][Y] =
verts [3][Y] = 0;
verts [1][Y] =
verts [2][Y] = 1;
RenderQuad (NULL, verts, 2, bTextured ? texCoord : NULL);
}

//------------------------------------------------------------------------------

bool COglBuffers::SizeVertices (int nVerts)
{
if (int (vertices.Length ()) >= nVerts)
	return true;
vertices.Destroy ();
return vertices.Create (nVerts) != NULL;
}

bool COglBuffers::SizeColor (int nVerts)
{
if (int (color.Length ()) >= nVerts)
	return true;
color.Destroy ();
return color.Create (nVerts) != NULL;
}

bool COglBuffers::SizeTexCoord (int nVerts)
{
if ((int (texCoord [0].Length ()) >= nVerts) && (int (texCoord [1].Length ()) >= nVerts))
	return true;
texCoord [0].Destroy ();
texCoord [1].Destroy ();
return ((texCoord [0].Create (nVerts) != NULL) && (texCoord [1].Create (nVerts) != NULL));
}

bool COglBuffers::SizeBuffers (int nVerts)
{
return SizeVertices (nVerts) && SizeColor (nVerts) && SizeTexCoord (nVerts);
}


//------------------------------------------------------------------------------

void COglBuffers::Flush (GLenum nPrimitive, int nVerts, int nDimensions, int bTextured, int bColored)
{
if (nVerts > 0)
	m_nVertices = nVerts;
if (vertices.Buffer () && m_nVertices) {
	if (bTextured && texCoord [0].Buffer ()) {
		ogl.EnableClientState (GL_TEXTURE_COORD_ARRAY);
		OglTexCoordPointer (2, GL_FLOAT, 0, texCoord [0].Buffer ());
		}
	if (bColored && color.Buffer ()) {
		ogl.EnableClientState (GL_COLOR_ARRAY);
		OglColorPointer (4, GL_FLOAT, 0, color.Buffer ());
		}
	ogl.EnableClientState (GL_VERTEX_ARRAY);
	OglVertexPointer (nDimensions, GL_FLOAT, sizeof (CFloatVector), vertices.Buffer ());
	OglDrawArrays (nPrimitive, 0, m_nVertices);
	ogl.DisableClientStates (bTextured, bColored, 0);
	}
}

//------------------------------------------------------------------------------
