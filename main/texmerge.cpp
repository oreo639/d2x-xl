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

#include <stdio.h>

#include "descent.h"
#include "error.h"
#include "u_mem.h"
#include "textures.h"
#include "rle.h"
#include "ogl_shader.h"
#include "ogl_render.h"

#define MAX_NUM_CACHE_BITMAPS 100

//static CBitmap * cache_bitmaps [MAX_NUM_CACHE_BITMAPS];                     

class CTextureCache {
	public:
		CBitmap	*bitmap;
		CBitmap	*bmBot;
		CBitmap	*bmTop;
		int32_t 	nOrient;
		int32_t	nLastFrameUsed;
};

CStaticArray < CTextureCache, MAX_NUM_CACHE_BITMAPS > texCache;

static int32_t nCacheEntries = 0;
static int32_t nCacheHits = 0;
static int32_t nCacheMisses = 0;

void MergeTextures (int32_t nType, CBitmap *bmBot, CBitmap *bmTop, CBitmap *bmDest, int32_t bSuperTransp);
void MergeTexturesNormal (int32_t nType, CBitmap *bmBot, CBitmap *bmTop, uint8_t *destDataP);
void _CDECL_ TexMergeClose (void);

//----------------------------------------------------------------------

int32_t TexMergeInit (int32_t nCacheSize)
{
	int32_t i;
	CTextureCache *pCache = &texCache [0];

nCacheEntries = ((nCacheSize > 0) && (nCacheSize <= MAX_NUM_CACHE_BITMAPS)) ? nCacheSize  : MAX_NUM_CACHE_BITMAPS;
for (i = 0; i < nCacheEntries; i++, pCache++) {
	pCache->nLastFrameUsed = -1;
	pCache->bmTop =
	pCache->bmBot =
	pCache->bitmap = NULL;
	pCache->nOrient = -1;
	}
atexit (TexMergeClose);
return 1;
}

//----------------------------------------------------------------------

void TexMergeFlush (void)
{
for (int32_t i = 0; i < nCacheEntries; i++) {
	texCache [i].nLastFrameUsed = -1;
	texCache [i].nOrient = -1;
	texCache [i].bmTop =
	texCache [i].bmBot = NULL;
	}
}

//-------------------------------------------------------------------------

void _CDECL_ TexMergeClose (void)
{
PrintLog (1, "shutting down merged textures cache\n");
TexMergeFlush ();
PrintLog (0, "deleting %d bitmaps\n", nCacheEntries);
for (int32_t i = 0; i < nCacheEntries; i++) {
	if (texCache [i].bitmap) {
		delete texCache [i].bitmap;
		texCache [i].bitmap = NULL;
		}
	}
nCacheEntries = 0;
PrintLog (-1);
}

//-------------------------------------------------------------------------
//--unused-- int32_t info_printed = 0;
CBitmap * TexMergeGetCachedBitmap (int32_t tMapBot, int32_t tMapTop, int32_t nOrient)
{
	CBitmap*			bmTop, * bmBot, * pBm;
	int32_t				i, nLowestFrame, nLRU;
	char				szName [20];
	CTextureCache*	pCache;

nLRU = 0;
nLowestFrame = texCache [0].nLastFrameUsed;
bmTop = gameData.pig.tex.bitmapP [gameData.pig.tex.bmIndexP [tMapTop].index].Override (-1);
bmBot = gameData.pig.tex.bitmapP [gameData.pig.tex.bmIndexP [tMapBot].index].Override (-1);

for (i = 0, pCache = &texCache [0]; i < nCacheEntries; i++, pCache++) {
#if 1//!DBG
	if ((pCache->nLastFrameUsed > -1) && 
		 (pCache->bmTop == bmTop) && 
		 (pCache->bmBot == bmBot) && 
		 (pCache->nOrient == nOrient) &&
		  pCache->bitmap) {
		nCacheHits++;
		pCache->nLastFrameUsed = gameData.app.nFrameCount;
		return pCache->bitmap;
	}
#endif
	if (pCache->nLastFrameUsed < nLowestFrame) {
		nLowestFrame = pCache->nLastFrameUsed;
		nLRU = i;
		}
	}
//---- Page out the LRU bitmap;
nCacheMisses++;
// Make sure the bitmaps are paged in...
gameData.pig.tex.bPageFlushed = 0;
LoadTexture (gameData.pig.tex.bmIndexP [tMapTop].index, 0, gameStates.app.bD1Mission);
LoadTexture (gameData.pig.tex.bmIndexP [tMapBot].index, 0, gameStates.app.bD1Mission);
if (gameData.pig.tex.bPageFlushed) {	// If cache got flushed, re-read 'em.
	gameData.pig.tex.bPageFlushed = 0;
	LoadTexture (gameData.pig.tex.bmIndexP [tMapTop].index, 0, gameStates.app.bD1Mission);
	LoadTexture (gameData.pig.tex.bmIndexP [tMapBot].index, 0, gameStates.app.bD1Mission);
	}
Assert (gameData.pig.tex.bPageFlushed == 0);

bmTop = gameData.pig.tex.bitmapP [gameData.pig.tex.bmIndexP [tMapTop].index].Override (-1);
bmBot = gameData.pig.tex.bitmapP [gameData.pig.tex.bmIndexP [tMapBot].index].Override (-1);
if (!bmTop->Palette ())
	bmTop->SetPalette (paletteManager.Game ());
if (!bmBot->Palette ())
	bmBot->SetPalette (paletteManager.Game ());
pCache = texCache + nLRU;
pBm = pCache->bitmap;
if (pBm)
	pBm->ReleaseTexture ();

// if necessary, allocate cache bitmap
// in any case make sure the cache bitmap has the proper size
if (!pBm ||
	(pBm->Width () != bmBot->Width ()) || 
	(pBm->Height () != bmBot->Height ())) {
	if (pBm)
		delete pBm;
	pBm = CBitmap::Create (0, bmBot->Width (), bmBot->Height (), 4);
	if (!pBm)
		return NULL;
	}
else
	pBm->SetFlags ((char) BM_FLAG_TGA);
if (!pBm->Buffer ())
	return NULL;
pBm->SetPalette (paletteManager.Game ());
if (!gameOpts->ogl.bGlTexMerge) {
	if (bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) {
		MergeTextures (nOrient, bmBot, bmTop, pBm, 1);
		pBm->AddFlags (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT);
		pBm->SetAvgColorIndex (bmTop->AvgColorIndex ());
		}
	else {
		MergeTextures (nOrient, bmBot, bmTop, pBm, 0);
		if (bmBot->Flags () & bmTop->Flags () & BM_FLAG_TRANSPARENT)
			pBm->AddFlags (BM_FLAG_TRANSPARENT);
		pBm->AddFlags (bmBot->Flags () & (~BM_FLAG_RLE));
		pBm->SetAvgColorIndex (bmBot->AvgColorIndex ());
		}
	}
pCache->bitmap = pBm;
pCache->bmTop = bmTop;
pCache->bmBot = bmBot;
pCache->nLastFrameUsed = gameData.app.nFrameCount;
pCache->nOrient = nOrient;
pBm->SetStatic (1);
pBm->SetTranspType ((pBm->Flags () & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT)) ? 3 : 0);
pBm->NeedSetup ();
pBm->SetStatic (1);
sprintf (szName, "Merged %3d,%3d", tMapBot, tMapTop);
pBm->SetName (szName);
return pBm;
}

//-------------------------------------------------------------------------

void MergeTexturesNormal (int32_t nType, CBitmap * bmBot, CBitmap * bmTop, uint8_t * destDataP)
{
	uint8_t * topDataP, *btmDataP;
	int32_t scale;

if (gameOpts->ogl.bGlTexMerge)
	return;
if (bmTop->Flags () & BM_FLAG_RLE)
	bmTop = rle_expand_texture(bmTop);
if (bmBot->Flags () & BM_FLAG_RLE)
	bmBot = rle_expand_texture(bmBot);
//	Assert(bmBot != bmTop);
topDataP = bmTop->Buffer ();
btmDataP = bmBot->Buffer ();
scale = bmBot->Width () / bmTop->Width ();
if (!scale)
	scale = 1;
if (scale > 1)
	scale = scale;
//	Assert(btmDataP != topDataP);
switch(nType) {
	case 0:
		// Normal
		GrMergeTextures (btmDataP, topDataP, destDataP, bmBot->Width (), bmBot->Height (), scale);
		break;
	case 1:
		GrMergeTextures1 (btmDataP, topDataP, destDataP, bmBot->Width (), bmBot->Height (), scale);
		break;
	case 2:
		GrMergeTextures2 (btmDataP, topDataP, destDataP, bmBot->Width (), bmBot->Height (), scale);
		break;
	case 3:
		GrMergeTextures3 (btmDataP, topDataP, destDataP, bmBot->Width (), bmBot->Height (), scale);
		break;
	}
}

//-------------------------------------------------------------------------

typedef struct frac {
	int32_t	c, d;
} frac;

inline tRGBA *C (uint8_t *pPalette, uint8_t *b, int32_t i, int32_t bpp, int32_t *pbST)
{
	static	tRGBA	c;
	int32_t bST;

if (bpp == 4) {
	c = reinterpret_cast<tRGBA*> (b) [i];
	if ((*pbST = ((c.r == 120) && (c.g == 88) && (c.b == 128))))
		c.a = 0;
	return &c;
	}
if (bpp == 3) {
	*((tRGB *) &c) = ((tRGB *) (b)) [i];
	if ((*pbST = ((c.r == 120) && (c.g == 88) && (c.b == 128))))
		c.a = 0;
	else
		c.a = 255;
	return &c;
	}
i = b [i];
bST = (i == SUPER_TRANSP_COLOR);
c.a = (bST || (i == TRANSPARENCY_COLOR)) ? 0 : 255;
if (c.a) {
	i *= 3;
	c.r = pPalette [i] * 4; 
	c.g = pPalette [i+1] * 4; 
	c.b = pPalette [i+2] * 4; 
	}
else if (bST) {
	c.r = 120;
	c.g = 88;
	c.b = 128;
	}
else
	c.r = c.g = c.b = 0;
*pbST = bST;
return &c;
}

//-------------------------------------------------------------------------

static inline int32_t TexScale (int32_t v, frac s)
{
return (v * s.c) / s.d;
}


static inline int32_t TexIdx (int32_t x, int32_t y, int32_t w, frac s)
{
return TexScale (y * w + x, s);
}


#define TOPSCALE(v)	TexScale (v, topScale)

//#define TOPIDX	TexIdx (x, y, tw, topScale)
#define BTMIDX	TexIdx (x, y, bw, btmScale)


void MergeTextures (int32_t nType, CBitmap* bmBot, CBitmap* bmTop, CBitmap *bmDest, int32_t bSuperTransp)
{
	tRGBA*	pColor;
	int32_t		i, x, y, bw, tw, th, dw, dh;
	int32_t		bTopBPP, bBtmBPP, bST = 0;
	frac		topScale, btmScale;
	tRGBA		*destDataP = reinterpret_cast<tRGBA*> (bmDest->Buffer ());

	uint8_t		*topDataP, *btmDataP, *topPalette, *btmPalette;

bmBot = bmBot->Override (-1);
bmTop = bmTop->Override (-1);
if (gameOpts->ogl.bGlTexMerge)
	return;
if (bmTop->Flags () & BM_FLAG_RLE)
	bmTop = rle_expand_texture (bmTop);

if (bmBot->Flags () & BM_FLAG_RLE)
	bmBot = rle_expand_texture (bmBot);

//	Assert(bmBot != bmTop);

topDataP = bmTop->Buffer ();
btmDataP = bmBot->Buffer ();
topPalette = bmTop->Palette ()->Raw ();
btmPalette = bmBot->Palette ()->Raw ();

bw = bmBot->Width ();
//h = bmBot->Height ();
th =
tw = bmTop->Width ();
dw =
dh = bmDest->Width ();
//th = bmTop->Height ();
#if 1
// square textures assumed here, so no test for h!
if (dw < tw) {
	topScale.c = tw / dw;
	topScale.d = 1;
	}
else  {
	topScale.c = 1;
	topScale.d = dw / tw;
	}
if (dw < bw) {
	btmScale.c = bw / dw;
	btmScale.d = 1;
	}
else {
	btmScale.c = dw / bw;
	btmScale.d = 1;
	}
#else
if (w > bmTop->Width ())
	w = h = bmBot->Width ();
scale.pColor = scale.d = 1;
#endif
bTopBPP = bmTop->BPP ();
bBtmBPP = bmBot->BPP ();
#if DBG
memset (destDataP, 253, bmDest->Width () * bmDest->Height () * 4);
#endif
switch (nType) {
	case 0:
		// Normal
		for (i = y = 0; y < dh; y++)
			for (x = 0; x < dw; x++, i++) {
				pColor = C (topPalette, topDataP, tw * TOPSCALE (y) + TOPSCALE (x), bTopBPP, &bST);
				if (!(bST || pColor->a))
					pColor = C (btmPalette, btmDataP, BTMIDX, bBtmBPP, &bST);
				destDataP [i] = *pColor;
			}
		break;
	case 1:
		// 
		for (i = y = 0; y < dh; y++)
			for (x = 0; x < dw; x++, i++) {
				pColor = C (topPalette, topDataP, tw * TOPSCALE (x) + th - 1 - TOPSCALE (y), bTopBPP, &bST);
				if (!(bST || pColor->a))
					pColor = C (btmPalette, btmDataP, BTMIDX, bBtmBPP, &bST);
				destDataP [i] = *pColor;
			}
		break;
	case 2:
		// Normal
		for (i = y = 0; y < dh; y++)
			for (x = 0; x < dw; x++, i++) {
				pColor = C (topPalette, topDataP, tw * (th - 1 - TOPSCALE (y)) + tw - 1 - TOPSCALE (x), bTopBPP, &bST);
				if (!(bST || pColor->a))
					pColor = C (btmPalette, btmDataP, BTMIDX, bBtmBPP, &bST);
				destDataP [i] = *pColor;
			}
		break;
	case 3:
		// Normal
		for (i = y = 0; y < dh; y++)
			for (x = 0; x < dw; x++, i++) {
				pColor = C (topPalette, topDataP, tw * (th - 1 - TOPSCALE (x)) + TOPSCALE (y), bTopBPP, &bST);
				if (!(bST || pColor->a))
					pColor = C (btmPalette, btmDataP, BTMIDX, bBtmBPP, &bST);
				destDataP [i] = *pColor;
			}
		break;
	}
}

//------------------------------------------------------------------------------

int32_t tmShaderProgs [6] = {-1,-1,-1,-1,-1,-1};

const char *texMergeFS [6] = {
	// grayscale
	"uniform sampler2D baseTex, decalTex;\r\n" \
	"uniform bool bMask;\r\n" \
	"void main(void){" \
	"vec4 decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"vec4 texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"vec4 color = vec4(vec3(mix(texColor,decalColor,decalColor.a)),min (1.0,(texColor.a+decalColor.a)))*gl_Color;\r\n" \
	"float l = (color.r + color.g + color.b) / 3.0;\r\n" \
	"gl_FragColor = vec4 (l, l, l, color.a);\r\n" \
   "}"
,
	"uniform sampler2D baseTex, decalTex, maskTex;\r\n" \
	"uniform bool bMask;\r\n" \
	"vec4 decalColor, texColor;\r\n" \
	"float fMask;\r\n" \
	"void main(void){" \
	"fMask = bMask ? texture2D(maskTex,gl_TexCoord [2].xy).r : 1.0;\r\n" \
	"decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"vec4 color = vec4(vec3(mix(texColor,decalColor,decalColor.a)),min (1.0,(texColor.a+decalColor.a)))*gl_Color;\r\n" \
	"float l = (color.r + color.g + color.b) / 3.0;\r\n" \
	"gl_FragColor = fMask * vec4 (l, l, l, color.a);\r\n" \
	"}"
,
	"uniform sampler2D baseTex, decalTex;\r\n" \
	"uniform bool bMask;\r\n" \
	"vec4 decalColor, texColor;\r\n" \
	"void main(void)" \
	"{decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"if((abs(decalColor.r-120.0/255.0)<8.0/255.0)&&(abs(decalColor.g-88.0/255.0)<8.0/255.0)&&(abs(decalColor.b-128.0/255.0)<8.0/255.0))discard;\r\n" \
	"texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"vec4 color = vec4(vec3(mix(texColor,decalColor,decalColor.a)),min (1.0,(texColor.a+decalColor.a)))*gl_Color;\r\n" \
	"float l = (color.r + color.g + color.b) / 3.0;\r\n" \
	"gl_FragColor = vec4 (l, l, l, color.a);\r\n" \
   "}"
,
	// colored
	"uniform sampler2D baseTex, decalTex;\r\n" \
	"uniform bool bMask;\r\n" \
	"void main(void){" \
	"vec4 decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"vec4 texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"gl_FragColor=vec4(vec3(mix(texColor,decalColor,decalColor.a)),min (1.0,(texColor.a+decalColor.a)))*gl_Color;\r\n" \
   "}"
,
	"uniform sampler2D baseTex, decalTex, maskTex;\r\n" \
	"uniform bool bMask;\r\n" \
	"uniform float bColored;\r\n" \
	"vec4 decalColor, texColor;\r\n" \
	"float fMask;\r\n" \
	"void main(void){" \
	"fMask = bMask ? texture2D(maskTex,gl_TexCoord [2].xy).r : 1.0;\r\n" \
	"decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"gl_FragColor = fMask * vec4(vec3(mix(texColor,decalColor,decalColor.a)),min (1.0,(texColor.a+decalColor.a)))*gl_Color;\r\n" \
	"}"
,
	"uniform sampler2D baseTex, decalTex;\r\n" \
	"uniform bool bMask;\r\n" \
	"vec4 decalColor, texColor;\r\n" \
	"void main(void)" \
	"{decalColor=texture2D(decalTex,gl_TexCoord [1].xy);\r\n" \
	"if((abs(decalColor.r-120.0/255.0)<8.0/255.0)&&(abs(decalColor.g-88.0/255.0)<8.0/255.0)&&(abs(decalColor.b-128.0/255.0)<8.0/255.0))discard;\r\n" \
	"texColor=texture2D(baseTex,gl_TexCoord [0].xy);\r\n" \
	"gl_FragColor=vec4(vec3(mix(texColor,decalColor,decalColor.a)),min (1.0,(texColor.a+decalColor.a)))*gl_Color;\r\n" \
   "}"
	};

const char *texMergeVS [3] = {
	"void main(void){" \
	"gl_TexCoord [0]=gl_MultiTexCoord0;"\
	"gl_TexCoord [1]=gl_MultiTexCoord1;"\
	"gl_Position=ftransform() /*gl_ModelViewProjectionMatrix * gl_Vertex*/;"\
	"gl_FrontColor=gl_Color;}"
,
	"void main(void){" \
	"gl_TexCoord [0]=gl_MultiTexCoord0;"\
	"gl_TexCoord [1]=gl_MultiTexCoord1;"\
	"gl_TexCoord [2]=gl_MultiTexCoord2;"\
	"gl_Position=ftransform() /*gl_ModelViewProjectionMatrix * gl_Vertex*/;"\
	"gl_FrontColor=gl_Color;}"
,
	"void main(void){" \
	"gl_TexCoord [0]=gl_MultiTexCoord0;"\
	"gl_TexCoord [1]=gl_MultiTexCoord1;"\
	"gl_Position=ftransform() /*gl_ModelViewProjectionMatrix * gl_Vertex*/;"\
	"gl_FrontColor=gl_Color;}"
	};

const char *texMergeFSData = 
	"uniform sampler2D baseTex, decalTex, maskTex;\r\n" \
	"uniform float grAlpha;";

//-------------------------------------------------------------------------

void InitTexMergeShaders (void)
{
	int32_t	i, b;

if (!(gameOpts->render.bUseShaders && ogl.m_features.bShaders))
	gameOpts->ogl.bGlTexMerge = 0;
else {
	PrintLog (0, "building texturing shader programs\n");
	for (i = 0; i < 6; i++) {
		b = shaderManager.Build (tmShaderProgs [i], texMergeFS [i], texMergeVS [i % 3]);
		if (i == 2)
			gameStates.render.textures.bHaveMaskShader = (b >= 0);
		else
			gameStates.render.textures.bGlTexMergeOk = (b >= 0);
		if (!gameStates.render.textures.bGlTexMergeOk) {
			while (i)
				shaderManager.Delete (tmShaderProgs [--i]);
			gameOpts->ogl.bGlTexMerge = 0;
			break;
			}
		}
	}
if (!gameOpts->ogl.bGlTexMerge) {
	ogl.m_states.bLowMemory = 0;
	ogl.m_features.bTextureCompression = 0;
	PrintLog (0, "+++++ OpenGL shader texture merging has been disabled! +++++\n");
	}
}

//------------------------------------------------------------------------------

int32_t SetupTexMergeShader (int32_t bColorKey, int32_t bColored, int32_t nType)
{
#if DBG
if (nType < 2)
	return -1;
#endif
#if 1
if (nType == 3)
	nType = 1;
#else
if ((nType == 3) && !gameStates.render.history.bmMask)
	nType = 2;
else
	nType = 1;
#endif

	int32_t nShader = nType + bColored * 3;

GLhandleARB shaderProg = GLhandleARB (shaderManager.Deploy (tmShaderProgs [nShader]));
if (!shaderProg)
	return -1;
shaderManager.Rebuild (shaderProg);

glUniform1i (glGetUniformLocation (shaderProg, "baseTex"), 0);
glUniform1i (glGetUniformLocation (shaderProg, "decalTex"), 1);
glUniform1i (glGetUniformLocation (shaderProg, "maskTex"), 2);
glUniform1i (glGetUniformLocation (shaderProg, "bMask"), (GLboolean) bColorKey);
return tmShaderProgs [nShader];
}

//------------------------------------------------------------------------------

