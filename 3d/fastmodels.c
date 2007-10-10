/* $Id: interp.c, v 1.14 2003/03/19 19:21:34 btb Exp $ */
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
COPYRIGHT 1993-1998 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
static char rcsid [] = "$Id: interp.c, v 1.14 2003/03/19 19:21:34 btb Exp $";
#endif

#include <math.h>
#include <stdlib.h>
#include "error.h"

#include "inferno.h"
#include "interp.h"
#include "shadows.h"
#include "hitbox.h"
#include "globvars.h"
#include "gr.h"
#include "byteswap.h"
#include "u_mem.h"
#include "console.h"
#include "ogl_init.h"
#include "network.h"
#include "render.h"
#include "gameseg.h"
#include "lighting.h"
#include "lightning.h"

extern tFaceColor tMapColor;

#if DBG_SHADOWS
extern int bShadowTest;
extern int bFrontCap;
extern int bRearCap;
extern int bShadowVolume;
extern int bFrontFaces;
extern int bBackFaces;
extern int bSWCulling;
#endif
#if SHADOWS
extern int bZPass;
#endif

#define G3_DRAW_ARRAYS	1
#define G3_USE_VBOS		0


//------------------------------------------------------------------------------

#define G3_FREE(_p)	{if (_p) D2_FREE (_p);}

int G3FreeModelItems (tG3Model *pm)
{
G3_FREE (pm->pFaces);
G3_FREE (pm->pSubModels);
#if G3_USE_VBOS
if (gameStates.ogl.bHaveVBOs && pm->vboHandle)
	glDeleteBuffers (1, &pm->vboHandle);
else
#endif
	G3_FREE (pm->pFaceVerts);
G3_FREE (pm->pColor);
G3_FREE (pm->pVertNorms);
G3_FREE (pm->pTransVerts);
G3_FREE (pm->pVerts);
G3_FREE (pm->pVertBuf);
G3_FREE (pm->pIndex);
memset (pm, 0, sizeof (*pm));
return 0;
}

//------------------------------------------------------------------------------

void G3FreeAllPolyModelItems (void)
{
	int	i;

for (i = 0; i < MAX_POLYGON_MODELS; i++)
	G3FreeModelItems (gameData.models.g3Models + i);
POFFreeAllPolyModelItems ();
}

//------------------------------------------------------------------------------

bool G3CountModelItems (void *modelP, short *pnSubModels, short *pnVerts, short *pnFaces, short *pnFaceVerts)
{
	ubyte *p = modelP;

G3CheckAndSwap (modelP);
for (;;)
	switch (WORDVAL (p)) {
		case OP_EOF:
			return 1;

		case OP_DEFPOINTS: {
			int n = WORDVAL (p+2);
			(*pnVerts) += n;
			p += n * sizeof (vmsVector) + 4;
			break;
			}

		case OP_DEFP_START: {
			int n = WORDVAL (p+2);
			p += n * sizeof (vmsVector) + 8;
			(*pnVerts) += n;
			break;
			}

		case OP_FLATPOLY: {
			int nVerts = WORDVAL (p+2);
			(*pnFaces)++;
			(*pnFaceVerts) += nVerts;
			p += 30 + ((nVerts & ~ 1) + 1) * 2;
			break;
			}

		case OP_TMAPPOLY: {
			int nVerts = WORDVAL (p + 2);
			(*pnFaces)++;
			(*pnFaceVerts) += nVerts;
			p += 30 + ((nVerts & ~1) + 1) * 2 + nVerts * 12;
			break;
			}

		case OP_SORTNORM:
			G3CountModelItems (p + WORDVAL (p+28), pnSubModels, pnVerts, pnFaces, pnFaceVerts);
			G3CountModelItems (p + WORDVAL (p+30), pnSubModels, pnVerts, pnFaces, pnFaceVerts);
			p += 32;
			break;


		case OP_RODBM: {
			p += 36;
			break;
			}

		case OP_SUBCALL: {
			(*pnSubModels)++;
			G3CountModelItems (p + WORDVAL (p+16), pnSubModels, pnVerts, pnFaces, pnFaceVerts);
			p += 20;
			break;
			}

		case OP_GLOW:
			p += 4;
			break;

		default:
			Error ("invalid polygon model\n");
	}
return 1;
}

//------------------------------------------------------------------------------

tG3ModelFace *G3AddModelFace (tG3Model *pm, tG3SubModel *psm, tG3ModelFace *pf, vmsVector *pn, ubyte *p, 
										grsBitmap **modelBitmaps, tRgbaColorf *pObjColor)
{
	short				nVerts = WORDVAL (p+2);
	tG3ModelVertex	*pmv;
	short				*pfv;
	tUVL				*uvl;
	grsBitmap		*bmP;
	tRgbaColorf		baseColor;
	short				i, j;
	ushort			c;
	char				bTextured;

Assert (pf - pm->pFaces < pm->nFaces);
if (modelBitmaps && *modelBitmaps) {
	bTextured = 1;
	pf->nBitmap = WORDVAL (p+28);
	bmP = modelBitmaps [pf->nBitmap];
	if (pObjColor) {
		ubyte c = bmP->bmAvgColor;
		pObjColor->red = CPAL2Tr (gamePalette, c);
		pObjColor->green = CPAL2Tg (gamePalette, c);
		pObjColor->blue = CPAL2Tb (gamePalette, c);
		}
	baseColor.red = baseColor.green = baseColor.blue = baseColor.alpha = 1;
	i = (int) (bmP - gameData.pig.tex.bitmaps [0]);
	pf->bThruster = (i == 24) || ((i >= 1741) && (i <= 1745));
	}
else {
	bTextured = 0;
	pf->nBitmap = -1;
	c = WORDVAL (p + 28);
	baseColor.red = (float) PAL2RGBA (((c >> 10) & 31) << 1) / 255.0f;
	baseColor.green = (float) PAL2RGBA (((c >> 5) & 31) << 1) / 255.0f;
	baseColor.blue = (float) PAL2RGBA ((c & 31) << 1) / 255.0f;
	baseColor.alpha = 1;
	if (pObjColor)
		*pObjColor = baseColor;
	}
pf->vNormal = *pn;
pf->nIndex = pm->iFaceVert;
pmv = pm->pFaceVerts + pm->iFaceVert;
if (psm->nIndex < 0)
	psm->nIndex = pm->iFaceVert;
pf->nVerts = nVerts;
if (pf->bGlow = (nGlow >= 0))
	nGlow = -1;
uvl = (tUVL *) (p + 30 + (nVerts | 1) * 2);
for (i = nVerts, pfv = WORDPTR (p+30); i; i--, pmv++, pfv++, uvl++) {
	j = *pfv;
	Assert (j < pm->nVerts);
	Assert (pmv - pm->pFaceVerts < pm->nFaceVerts);
	VmVecIncf (&pm->pVertNorms [j].vNormal, &pmv->vertex);
	pm->pVertNorms [j].nVerts++;
	pmv->texCoord.v.u = f2fl (uvl->u);
	pmv->texCoord.v.v = f2fl (uvl->v);
	pmv->baseColor = baseColor;
	pmv->bTextured = bTextured;
	pmv->nIndex = j;
	}
pm->iFaceVert += nVerts;
pf++;
pm->iFace++;
psm->nFaces++;
return pf;
}

//------------------------------------------------------------------------------

bool G3GetModelItems (void *modelP, vmsAngVec *pAnimAngles, tG3Model *pm, int nThis, int nParent, 
							 grsBitmap **modelBitmaps, tRgbaColorf *pObjColor)
{
	ubyte				*p = modelP;
	tG3SubModel		*psm = pm->pSubModels + nThis;
	tG3ModelFace	*pf = pm->pFaces + pm->iFace;
	int				nChild;

G3CheckAndSwap (modelP);
nGlow = -1;
if (!psm->pFaces) {
	psm->pFaces = pf;
	psm->nIndex = -1;
	psm->nParent = nParent;
	}
for (;;)
	switch (WORDVAL (p)) {
		case OP_EOF:
			return 1;

		case OP_DEFPOINTS: {
			int i, n = WORDVAL (p+2);
			fVector *pfv = pm->pVerts;
			vmsVector *pv = VECPTR(p+4);
			for (i = n; i; i--)
				VmsVecToFloat (pfv++, pv++);
			p += n * sizeof (vmsVector) + 4;
			break;
			}

		case OP_DEFP_START: {
			int i, n = WORDVAL (p+2);
			int s = WORDVAL (p+4);
			fVector *pfv = pm->pVerts + s;
			vmsVector *pv = VECPTR(p+8);
			for (i = n; i; i--)
				VmsVecToFloat (pfv++, pv++);
			p += n * sizeof (vmsVector) + 8;
			break;
			}

		case OP_FLATPOLY: {
			int nVerts = WORDVAL (p+2);
			pf = G3AddModelFace (pm, psm, pf, VECPTR (p+16), p, NULL, pObjColor);
			p += 30 + (nVerts | 1) * 2;
			break;
			}

		case OP_TMAPPOLY: {
			int nVerts = WORDVAL (p + 2);
			pf = G3AddModelFace (pm, psm, pf, VECPTR (p+16), p, modelBitmaps, pObjColor);
			p += 30 + (nVerts | 1) * 2 + nVerts * 12;
			break;
			}

		case OP_SORTNORM:
			G3GetModelItems (p + WORDVAL (p+28), pAnimAngles, pm, nThis, nParent, modelBitmaps, pObjColor);
			pf = pm->pFaces + pm->iFace;
			G3GetModelItems (p + WORDVAL (p+30), pAnimAngles, pm, nThis, nParent, modelBitmaps, pObjColor);
			pf = pm->pFaces + pm->iFace;
			p += 32;
			break;

		case OP_RODBM:
			p+=36;
			break;

		case OP_SUBCALL: {
			nChild = ++pm->iSubModel;
			pm->pSubModels [nChild].vOffset = *VECPTR (p+4);
			pm->pSubModels [nChild].nAngles = WORDVAL (p+2);
			G3GetModelItems (p + WORDVAL (p+16), pAnimAngles, pm, nChild, nThis, modelBitmaps, pObjColor);
			pf = pm->pFaces + pm->iFace;
			p += 20;
			break;
			}

		case OP_GLOW:
			nGlow = WORDVAL (p+2);
			p += 4;
			break;

		default:
			Error ("invalid polygon model\n");
		}
return 1;
}

//------------------------------------------------------------------------------

inline int G3CmpFaces (tG3ModelFace *pf, tG3ModelFace *pm)
{
if (pf->nBitmap < pm->nBitmap)
	return -1;
if (pf->nBitmap > pm->nBitmap)
	return 1;
if (pf->nVerts < pm->nVerts)
	return -1;
if (pf->nVerts > pm->nVerts)
	return 1;
return 0;
}

//------------------------------------------------------------------------------

void G3SortFaces (tG3SubModel *psm, int left, int right)
{
	int				l = left,
						r = right;
	tG3ModelFace	m = psm->pFaces [(l + r) / 2];
			
do {
	while (G3CmpFaces (psm->pFaces + l, &m) < 0)
		l++;
	while (G3CmpFaces (psm->pFaces + r, &m) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			tG3ModelFace h = psm->pFaces [l];
			psm->pFaces [l] = psm->pFaces [r];
			psm->pFaces [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	G3SortFaces (psm, l, right);
if (left < r)
	G3SortFaces (psm, left, r);
}

//------------------------------------------------------------------------------

#define	G3_ALLOC(_buf,_count,_type,_fill) \
			if ((_buf) = (_type *) D2_ALLOC (_count * sizeof (_type))) \
				memset (_buf, (char) _fill, _count * sizeof (_type)); \
			else \
				return G3FreeModelItems (pm);

//------------------------------------------------------------------------------

void G3SortFaceVerts (tG3Model *pm, tG3SubModel *psm, tG3ModelVertex *psv)
{
	tG3ModelFace	*pf;
	tG3ModelVertex	*pmv = pm->pFaceVerts;
	int				i, j, nIndex = psm->nIndex;

psv += nIndex;
for (i = psm->nFaces, pf = psm->pFaces; i; i--, pf++, psv += j) {
	j = pf->nVerts;
	if (nIndex + j > pm->nFaceVerts)
		break;
	memcpy (psv, pmv + pf->nIndex, j * sizeof (tG3ModelVertex));
	pf->nIndex = nIndex;
	nIndex += j;
	}
}

//------------------------------------------------------------------------------

int G3BuildModel (int nModel, tPolyModel *pp, grsBitmap **modelBitmaps, tRgbaColorf *pObjColor)
{
	tG3Model			*pm = gameData.models.g3Models + nModel;
	tG3SubModel		*psm;
	tG3VertNorm		*pn;
	tG3ModelFace	*pf, *pfi, *pfj;
	tG3ModelVertex	*pmv, *pSortedVerts;
	fVector3			*pv;
	tTexCoord2f		*pt;
	tRgbaColorf		*pc;
	int				i, j;
	short				nId;

if (pm->bValid)
	return 1;
if (!pp->modelData)
	return 0;
pm->nSubModels = 1;
G3CountModelItems (pp->modelData, &pm->nSubModels, &pm->nVerts, &pm->nFaces, &pm->nFaceVerts);
G3_ALLOC (pm->pVerts, pm->nVerts, fVector, 0);
G3_ALLOC (pm->pTransVerts, pm->nVerts, fVector, 0);
G3_ALLOC (pm->pVertNorms, pm->nVerts, tG3VertNorm, 0);
G3_ALLOC (pm->pColor, pm->nVerts, tFaceColor, 0xff);
#if G3_USE_VBOS
if (gameStates.ogl.bHaveVBOs) {
	glGenBuffers (1, &pm->vboHandle);
	if (i = glGetError ()) {
		glGenBuffers (1, &pm->vboHandle);
		if (i = glGetError ()) {
#	ifdef _DEBUG
			HUDMessage (0, "glGenBuffers failed (%d)", i);
#	endif
			return G3FreeModelItems (pm);
			}
		}	
	glBindBuffer (GL_ARRAY_BUFFER_ARB, pm->vboHandle);
	if (i = glGetError ()) {
#	ifdef _DEBUG
		HUDMessage (0, "glBindBuffer failed (%d)", i);
#	endif
		return G3FreeModelItems (pm);
		}	
	glBufferData (GL_ARRAY_BUFFER, pm->nFaceVerts * sizeof (tRenderVertex), NULL, GL_DYNAMIC_DRAW_ARB);
	pm->pVertBuf = (char *) glMapBuffer (GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	}
else
#endif
	G3_ALLOC (pm->pVertBuf, pm->nFaceVerts, tG3RenderVertex, 0);
G3_ALLOC (pm->pFaceVerts, pm->nFaceVerts, tG3ModelVertex, 0);
G3_ALLOC (pm->pSubModels, pm->nSubModels, tG3SubModel, 0);
G3_ALLOC (pm->pFaces, pm->nFaces, tG3ModelFace, 0);
G3_ALLOC (pm->pIndex, pm->nFaceVerts, short, 0);
G3_ALLOC (pSortedVerts, pm->nFaceVerts, tG3ModelVertex, 0);
G3GetModelItems (pp->modelData, NULL, pm, 0, -1, modelBitmaps, pObjColor);
pm->fScale = 1;
for (i = 0, j = pm->nFaceVerts; i < j; i++)
	pm->pIndex [i] = i;
for (i = pm->nVerts, pn = pm->pVertNorms; i; i--, pn++) {
	Assert (pn - pm->pVertNorms < pm->nVerts);
	if (pn->nVerts)
		VmVecScalef (&pn->vNormal, &pn->vNormal, 1 / (float) pn->nVerts);
	}
for (i = pm->nFaces, pf = pm->pFaces; i; i--, pf++)
	for (j = pf->nVerts, pmv = pm->pFaceVerts + pf->nIndex; j; j--, pmv++) {
		pmv->vertex = pm->pVerts [pmv->nIndex];
		pmv->normal = pm->pVertNorms [pmv->nIndex].vNormal;
		}
for (i = pm->nSubModels, psm = pm->pSubModels; i; i--, psm++) {
	G3SortFaces (psm, 0, psm->nFaces - 1);
	G3SortFaceVerts (pm, psm, pSortedVerts);
	for (nId = 0, j = psm->nFaces - 1, pfi = psm->pFaces; j; j--) {
		pfi->nId = nId;
		pfj = pfi++;
		if (G3CmpFaces (pfi, pfj))
			nId++;
		}	
	pfi->nId = nId;
	}
pm->pVBVerts = (fVector3 *) pm->pVertBuf;
pm->pVBColor = (tRgbaColorf *) (pm->pVBVerts + pm->nFaceVerts);
pm->pVBTexCoord = (tTexCoord2f *) (pm->pVBColor + pm->nFaceVerts);
pv = pm->pVBVerts;
pt = pm->pVBTexCoord;
pc = pm->pVBColor;
pmv = pSortedVerts;
for (i = pm->nFaceVerts; i; i--, pt++, pc++, pv++, pmv++) {
	memcpy (pv, &pmv->vertex, sizeof (fVector3));
	*pc = pmv->baseColor;
	*pt = pmv->texCoord;
	}
memcpy (pm->pFaceVerts, pSortedVerts, pm->nFaceVerts * sizeof (tG3ModelVertex));
D2_FREE (pSortedVerts);
pm->bValid = 1;
#if G3_USE_VBOS
if (gameStates.ogl.bHaveVBOs) {
	glUnmapBuffer (GL_ARRAY_BUFFER_ARB);
	glBindBuffer (GL_ARRAY_BUFFER_ARB, 0);
	}
#endif
return -1;
}

//------------------------------------------------------------------------------

typedef struct tG3ThreadInfo {
	tG3Model		*pm;
	tThreadInfo	ti [2];
} tG3ThreadInfo;

static tG3ThreadInfo g3ti;

//------------------------------------------------------------------------------

void G3DynLightModel (tG3Model *pm, short iVerts, short nVerts, short iFaceVerts, short nFaceVerts)
{
	fVector			*pv;
	tG3ModelVertex	*pmv;
	tG3VertNorm		*pn;
	tFaceColor		*pc;
	float				fAlpha = (float) gameStates.render.grAlpha / (float) GR_ACTUAL_FADE_LEVELS;
	int				h, i;

if (!gameStates.render.bBrightObject)
	for (i = iVerts, pv = pm->pVerts + iVerts, pn = pm->pVertNorms + iVerts, pc = pm->pColor + iVerts; i < nVerts; i++, pv++, pn++, pc++)
		G3VertexColor (&pn->vNormal, pv, i, pc, NULL, 1, 0, 0);
for (i = iFaceVerts, h = iFaceVerts, pmv = pm->pFaceVerts + iFaceVerts; i < nFaceVerts; i++, h++, pmv++) {
	if (gameStates.render.bBrightObject)
		pm->pVBColor [h] = pmv->baseColor;
	else if (pmv->bTextured)
		pm->pVBColor [h] = pm->pColor [pmv->nIndex].color;
	else {
		pc = pm->pColor + pmv->nIndex;
		pm->pVBColor [h].red = pmv->baseColor.red *pc->color.red;
		pm->pVBColor [h].green = pmv->baseColor.green *pc->color.green;
		pm->pVBColor [h].blue = pmv->baseColor.blue *pc->color.blue;
		pm->pVBColor [h].alpha = pmv->baseColor.alpha;
		}
	}
}

//------------------------------------------------------------------------------

int _CDECL_ G3ModelLightThread (void *pThreadId)
{
	int	nId = *((int *) pThreadId);
	short	iVerts, nVerts, iFaceVerts, nFaceVerts;

do {
	while (!g3ti.ti [nId].bExec)
		G3_SLEEP (0);
	if (nId) {
		nVerts = g3ti.pm->nVerts;
		iVerts = nVerts / 2;
		nFaceVerts = g3ti.pm->nFaceVerts;
		iFaceVerts = nFaceVerts / 2;
		}
	else {
		iVerts = 0;
		nVerts = g3ti.pm->nVerts / 2;
		iFaceVerts = 0;
		nFaceVerts = g3ti.pm->nFaceVerts / 2;
		}
	G3DynLightModel (g3ti.pm, iVerts, nVerts, iFaceVerts, nFaceVerts);
	g3ti.ti [nId].bExec = 0;
	} while (!g3ti.ti [nId].bDone);
return 0;
}

//------------------------------------------------------------------------------

void G3StartModelLightThreads (void)
{
	int	i;

for (i = 0; i < 2; i++) {
	g3ti.ti [i].bDone = 0;
	g3ti.ti [i].bExec = 0;
	g3ti.ti [i].nId = i;
	g3ti.ti [i].pThread = SDL_CreateThread (G3ModelLightThread, &g3ti.ti [i].nId);
	}
}

//------------------------------------------------------------------------------

void G3EndModelLightThreads (void)
{
	int	i;

for (i = 0; i < 2; i++)
	g3ti.ti [i].bDone = 1;
G3_SLEEP (1);
#if 1
SDL_KillThread (g3ti.ti [0].pThread);
SDL_KillThread (g3ti.ti [1].pThread);
#else
SDL_WaitThread (g3ti.ti [0].pThread, NULL);
SDL_WaitThread (g3ti.ti [1].pThread, NULL);
#endif
}

//------------------------------------------------------------------------------

void G3LightModel (int nModel, fix xModelLight, fix *xGlowValues)
{
	tG3Model			*pm = gameData.models.g3Models + nModel;
	tG3ModelVertex	*pmv;
	tG3ModelFace	*pf;
	tRgbaColorf		baseColor, *colorP;
	float				fLight, fAlpha = (float) GrAlpha (); //(float) gameStates.render.grAlpha / (float) GR_ACTUAL_FADE_LEVELS;
	int				h, i, j, l;

if (!gameStates.render.bCloaked && SHOW_DYN_LIGHT && gameOpts->ogl.bLightObjects) {
	if (gameStates.app.bMultiThreaded) {
		g3ti.pm = pm;
		g3ti.ti [0].bExec = g3ti.ti [1].bExec = 1;
		while (g3ti.ti [0].bExec || g3ti.ti [1].bExec)
			G3_SLEEP (0);
		}
	else {
		G3DynLightModel (pm, 0, pm->nVerts, 0, pm->nFaceVerts);
		}
	}
else {
	if (!gameStates.render.bCloaked && gameData.objs.color.index)
		baseColor = gameData.objs.color.color;
	else
		baseColor.red = baseColor.green = baseColor.blue = 0;
	baseColor.alpha = fAlpha;
	for (i = pm->nFaces, pf = pm->pFaces; i; i--, pf++) {
		if (gameStates.render.bCloaked) 
			colorP = NULL;
		else {
			if (pf->nBitmap >= 0) {
				if (pf->bGlow) 
					l = xGlowValues [nGlow];
				else {
					l = -VmVecDot (&viewInfo.view [0].fVec, &pf->vNormal);
					l = f1_0 / 4 + 3 * l / 4;
					l = FixMul (l, xModelLight);
					}
				fLight = f2fl (l);
				colorP = &baseColor;
				}
			else
				colorP = NULL;
			}
		for (j = pf->nVerts, h = pf->nIndex, pmv = pm->pFaceVerts + pf->nIndex; j; j--, h++, pmv++) {
			if (colorP) {
				pm->pVBColor [h].red = colorP->red * fLight;
				pm->pVBColor [h].green = colorP->green * fLight;
				pm->pVBColor [h].blue = colorP->blue * fLight;
				pm->pVBColor [h].alpha = fAlpha;
				}
			else if (!gameStates.render.bCloaked) {
				pm->pVBColor [h] = pmv->baseColor;
				pm->pVBColor [h].alpha = fAlpha;
				}
			else
				pm->pVBColor [h] = baseColor;
			}
		}
	}
}

//------------------------------------------------------------------------------

void G3ScaleModel (int nModel)
{
	tG3Model			*pm = gameData.models.g3Models + nModel;
	float				fScale = gameData.models.nScale ? f2fl (gameData.models.nScale) : 1;
	int				i;
	fVector			*pv;
	tG3ModelVertex	*pmv;

if (pm->fScale == fScale)
	return;
fScale /= pm->fScale;
for (i = pm->nVerts, pv = pm->pVerts; i; i--, pv++) {
	pv->p.x *= fScale;
	pv->p.y *= fScale;
	pv->p.z *= fScale;
	}
for (i = pm->nFaceVerts, pmv = pm->pFaceVerts; i; i--, pmv++)
	pmv->vertex = pm->pVerts [pmv->nIndex];
pm->fScale *= fScale;
}

//------------------------------------------------------------------------------

void G3GetThrusterPos (short nModel, tG3ModelFace *pf, vmsVector *vOffset)
{
	tG3Model				*pm = gameData.models.g3Models + nModel;
	tG3ModelVertex		*pmv;
	fVector				v, vn, vo, vForward = {{0,0,1}};
	tModelThrusters	*mtP = gameData.models.thrusters + nModel;
	int					i, j;
	float					h, nSize;

if (mtP->nCount >= 2)
	return;
VmsVecToFloat (&vn, &pf->vNormal);
if (VmVecDotf (&vn, &vForward) > -F1_0 / 3)
	return;
for (i = 1, j = pf->nVerts, pmv = pm->pFaceVerts + pf->nIndex, v = pmv->vertex; i < j; i++)
	VmVecIncf (&v, &pmv [i].vertex);
v.p.x /= j;
v.p.y /= j;
v.p.z /= j;
v.p.z -= F1_0 / 8;
if (vOffset) {
	VmsVecToFloat (&vo, vOffset);
	VmVecIncf (&v, &vo);
	}
if (mtP->nCount && (v.p.x == mtP->vPos [0].p.x) && (v.p.y == mtP->vPos [0].p.y) && (v.p.z == mtP->vPos [0].p.z))
	return;
mtP->vPos [mtP->nCount].p.x = fl2f (v.p.x);
mtP->vPos [mtP->nCount].p.y = fl2f (v.p.y);
mtP->vPos [mtP->nCount].p.z = fl2f (v.p.z);
if (vOffset)
	VmVecDecf (&v, &vo);
mtP->vDir [mtP->nCount] = pf->vNormal;
VmVecNegate (mtP->vDir + mtP->nCount);
if (!mtP->nCount++) {
	for (i = 0, nSize = 1000000000; i < j; i++)
		if (nSize > (h = VmVecDistf (&v, &pmv [i].vertex)))
			nSize = h;
	mtP->fSize = nSize;// * 1.25f;
	}
}

//------------------------------------------------------------------------------

void G3RenderSubModel (tObject *objP, short nModel, short nSubModel, grsBitmap **modelBitmaps, vmsAngVec *pAnimAngles, vmsVector *vOffset)
{
	tG3Model			*pm = gameData.models.g3Models + nModel;
	tG3SubModel		*psm = pm->pSubModels + nSubModel;
	tG3ModelFace	*pf;
	grsBitmap		*bmP = NULL;
	vmsAngVec		*va = pAnimAngles ? pAnimAngles + psm->nAngles : &avZero;
	vmsVector		vo;
	int				i, j;
	short				nId, nFaceVerts, nVerts, nIndex, nBitmap = -1;

// set the translation
vo = psm->vOffset;
if (gameData.models.nScale)
	VmVecScale (&vo, gameData.models.nScale);
if (vOffset) {
	G3StartInstanceAngles (&vo, va);
	VmVecInc (&vo, vOffset);
	}
// render any dependent submodels
for (i = 0, j = pm->nSubModels, psm = pm->pSubModels; i < j; i++, psm++)
	if (psm->nParent == nSubModel)
		G3RenderSubModel (objP, nModel, i, modelBitmaps, pAnimAngles, &vo);
// render the faces
glDisable (GL_TEXTURE_2D);
for (psm = pm->pSubModels + nSubModel, i = psm->nFaces, pf = psm->pFaces; i; ) {
	if (!gameStates.render.bCloaked && (nBitmap != pf->nBitmap)) {
		if (0 <= (nBitmap = pf->nBitmap)) {
			bmP = modelBitmaps [nBitmap];
			glEnable (GL_TEXTURE_2D);
			bmP = BmOverride (bmP, -1);
			if (BM_FRAMES (bmP))
				bmP = BM_CURFRAME (bmP);
			if (OglBindBmTex (bmP, 1, 3))
				continue;
			OglTexWrap (bmP->glTexture, GL_REPEAT);
			}
		else {
			glDisable (GL_TEXTURE_2D);
			}
		}
	nIndex = pf->nIndex;
	if ((nFaceVerts = pf->nVerts) > 4) {
		if (pf->bThruster)
			G3GetThrusterPos (nModel, pf, vOffset);
		nVerts = nFaceVerts;
		pf++;
		i--;
		}
	else { 
		nId = pf->nId;
		nVerts = 0;
		do {
			if (pf->bThruster)
				G3GetThrusterPos (nModel, pf, vOffset);
			nVerts += nFaceVerts;
			pf++;
			i--;
			} while (i && (pf->nId == nId));
		}
#ifdef _DEBUG
	if (nIndex + nVerts > pm->nFaceVerts)
		break;
#endif
#	if 0//G3_USE_VBOS
if (gameStates.ogl.bHaveVBOs)
	nVerts /= nFaceVerts;
#	endif
#if G3_DRAW_ARRAYS
#	if G3_USE_VBOS
	if (gameStates.ogl.bHaveVBOs)
		glDrawArrays ((nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN, nIndex, nVerts);
	else
#	endif
#if 0
	if (glDrawRangeElements)
		glDrawRangeElements ((nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN, 
									nIndex, nIndex + nVerts - 1, nVerts, GL_UNSIGNED_SHORT, pm->pIndex + nIndex);

	else
		glDrawElements ((nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN, 
							 nVerts, GL_UNSIGNED_SHORT, pm->pIndex + nIndex);
#else
	glDrawArrays ((nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN, nIndex, nVerts);
#endif
#else
	{
	tG3ModelVertex	*pmv = pm->pFaceVerts + nIndex;
	glBegin ((nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN);
	for (j = nVerts; j; j--, pmv++) {
		glTexCoord2fv ((GLfloat *) &pmv->texCoord);
		glColor4fv ((GLfloat *) &pmv->renderColor);
		glVertex3fv ((GLfloat *) &pmv->vertex);
		}
	glEnd ();
	}
#endif
	}
if (vOffset)
	G3DoneInstance ();
}

//------------------------------------------------------------------------------

void G3RenderDamageLightnings (tObject *objP, short nModel, short nSubModel, vmsAngVec *pAnimAngles, vmsVector *vOffset)
{
	tG3Model			*pm = gameData.models.g3Models + nModel;
	tG3SubModel		*psm = pm->pSubModels + nSubModel;
	tG3ModelFace	*pf;
	grsBitmap		*bmP = NULL;
	vmsAngVec		*va = pAnimAngles ? pAnimAngles + psm->nAngles : &avZero;
	vmsVector		vo;
	int				i, j;

if (!(SHOW_LIGHTNINGS && gameOpts->render.lightnings.bDamage))
	return;
if (!objP || (ObjectDamage (objP) > 0.5f))
	return;
// set the translation
vo = psm->vOffset;
if (gameData.models.nScale)
	VmVecScale (&vo, gameData.models.nScale);
if (vOffset) {
	G3StartInstanceAngles (&vo, va);
	VmVecInc (&vo, vOffset);
	}
// render any dependent submodels
for (i = 0, j = pm->nSubModels, psm = pm->pSubModels; i < j; i++, psm++)
	if (psm->nParent == nSubModel)
		G3RenderDamageLightnings (objP, nModel, i, pAnimAngles, &vo);
// render the lightnings
for (psm = pm->pSubModels + nSubModel, i = psm->nFaces, pf = psm->pFaces; i; i--, pf++) 
	RenderDamageLightnings (objP, NULL, pm->pFaceVerts + pf->nIndex, pf->nVerts);
if (vOffset)
	G3DoneInstance ();
}

//------------------------------------------------------------------------------

#define G3_BUFFER_OFFSET(_i)	(GLvoid *) ((char *) NULL + (_i))

int G3RenderModel (tObject *objP, int nModel, tPolyModel *pp, grsBitmap **modelBitmaps, 
						 vmsAngVec *pAnimAngles, fix xModelLight, fix *xGlowValues, tRgbaColorf *pObjColor)
{
	tG3Model	*pm = gameData.models.g3Models + nModel;

	static int bFast = 1;

if (!bFast)
	return 0;
if (G3BuildModel (nModel, pp, modelBitmaps, pObjColor) <= 0)
	return 0;
#if G3_DRAW_ARRAYS
if (!G3EnableClientStates (1, 1, GL_TEXTURE0))
	return 0;
#	if G3_USE_VBOS
if (gameStates.ogl.bHaveVBOs) {
	int i;
	glBindBuffer (GL_ARRAY_BUFFER_ARB, pm->vboHandle);
	if (i = glGetError ()) {
		glBindBuffer (GL_ARRAY_BUFFER_ARB, pm->vboHandle);
		if (i = glGetError ())
			return 0;
		}
	pm->pVertBuf = (tG3RenderVertex *) glMapBuffer (GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	if (i = glGetError ())
		return 0;
	}
#endif
pm->pVBVerts = (fVector3 *) pm->pVertBuf;
pm->pVBColor = (tRgbaColorf *) (pm->pVBVerts + pm->nFaceVerts);
pm->pVBTexCoord = (tTexCoord2f *) (pm->pVBColor + pm->nFaceVerts);
//G3ScaleModel (nModel);
G3LightModel (nModel, xModelLight, xGlowValues);
#	if G3_USE_VBOS
if (gameStates.ogl.bHaveVBOs) {
	if (!glUnmapBuffer (GL_ARRAY_BUFFER_ARB))
		return 0;
	glVertexPointer (3, GL_FLOAT, 0, G3_BUFFER_OFFSET (0));
	glColorPointer (4, GL_FLOAT, 0, G3_BUFFER_OFFSET (pm->nFaceVerts * sizeof (fVector3)));
	glTexCoordPointer (2, GL_FLOAT, 0, G3_BUFFER_OFFSET (pm->nFaceVerts * ((sizeof (fVector3) + sizeof (tRgbaColorf)))));
	}
else 
#	endif
	{
	glTexCoordPointer (2, GL_FLOAT, 0, pm->pVBTexCoord);
	glColorPointer (4, GL_FLOAT, 0, pm->pVBColor);
	glVertexPointer (3, GL_FLOAT, 0, pm->pVBVerts);	
	}
#endif
G3RenderSubModel (objP, nModel, 0, modelBitmaps, pAnimAngles, NULL);
glDisable (GL_TEXTURE_2D);
#if G3_DRAW_ARRAYS
#	if G3_USE_VBOS
glBindBuffer (GL_ARRAY_BUFFER_ARB, 0);
#	endif
G3DisableClientStates (1, 1, -1);
#endif
G3RenderDamageLightnings (objP, nModel, 0, pAnimAngles, NULL);
return 1;
}

//------------------------------------------------------------------------------
//eof
