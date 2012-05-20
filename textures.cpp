#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "FileManager.h"
#include "dle-xp-res.h"
#include "global.h"
#include "PaletteManager.h"
#include "TextureManager.h"
#include "dle-xp.h"
#include "glew.h"

short nDbgTexture = -1;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void RgbFromIndex (int nIndex, PALETTEENTRY& rgb)
{
CBGR* color = paletteManager.Current (nIndex);
if (color != null) {
	rgb.peRed = color->r;
	rgb.peGreen = color->g;
	rgb.peBlue = color->b;
	rgb.peFlags = 0;
	}
}

// -------------------------------------------------------------------------- 

bool PaintTexture (CWnd *wndP, int bkColor, int nSegment, int nSide, int nBaseTex, int nOvlTex, int xOffset, int yOffset)
{
if (theMine == null) 
	return false;
if (!wndP->m_hWnd)
	return false;

	static int nOffset [2] = {0, 0};

	CDC			*pDC = wndP->GetDC ();

if (!pDC)
	return false;

	CBitmap			bmTexture;
	CFileManager	fp;
	char				szFile [256];
	BITMAP			bm;
	CDC				memDC;
	CSegment*		segP;
	CSide*			sideP;
	short				nWall;
	bool				bShowTexture = true, bDescent1 = DLE.IsD1File ();
	char*				path = bDescent1 ? descentFolder [0] : descentFolder [1];

CRect	rc;
wndP->GetClientRect (rc);

if (nBaseTex < 0) {
	segP = (nSegment < 0) ? current->Segment () : segmentManager.Segment (nSegment);
	sideP = (nSide < 0) ? current->Side () : segP->m_sides + nSide;
	int nSide = current->m_nSide;
	nBaseTex = sideP->BaseTex ();
	nOvlTex = sideP->OvlTex (0);
	if (segP->ChildId (nSide) == -1)
		bShowTexture = TRUE;
	else {
		nWall = sideP->m_info.nWall;
		bShowTexture = (nWall < wallManager.WallCount ());
		}
	}
if ((nBaseTex < 0) || (nBaseTex >= textureManager.MaxTextures () + 10))
	bShowTexture = false;
if ((nOvlTex < 0) || (nOvlTex >= textureManager.MaxTextures ()))	// this allows to suppress bitmap display by 
	bShowTexture = false;									// passing 0 for texture 1 and -1 for nOvlTex

if (bShowTexture) {
	// check pig file
	if (nOffset [bDescent1] == 0) {
		strcpy_s (szFile, sizeof (szFile), (bDescent1) ? descentFolder [0] : descentFolder [1]);
		if (!fp.Open (szFile, "rb"))
			nOffset [bDescent1] = -1;  // pig file not found
		else {
			fp.Seek (0, SEEK_SET);
			nOffset [bDescent1] = fp.ReadInt32 ();  // determine type of pig file
			fp.Close ();
			}
		}
	if (nOffset [bDescent1] > 0x10000L) {  // pig file type is v1.4a or descent 2 type
		CTexture tex (textureManager.m_bmBuf);
		if (textureManager.BlendTextures (nBaseTex, nOvlTex, &tex, xOffset, yOffset))
			DEBUGMSG (" Texture renderer: Texture not found (textureManager.BlendTextures failed)");
		//CPalette *pOldPalette = pDC->SelectPalette (paletteManager.Render (), FALSE);
		//pDC->RealizePalette ();
#if 0
		if (pDC->GetDeviceCaps (RASTERCAPS) & RC_DIBTODEV) {
			BITMAPINFO* bmi = paletteManager.BMI ();
			bmi->bmiHeader.biWidth = 
			bmi->bmiHeader.biHeight = tex.Width ();
			bmi->bmiHeader.biBitCount = 32;
			bmi->bmiHeader.biSizeImage = ((((bmi->bmiHeader.biWidth * bmi->bmiHeader.biBitCount) + 31) & ~31) >> 3) * bmi->bmiHeader.biHeight;
			StretchDIBits (pDC->m_hDC, 0, 0, rc.Width (), rc.Height (), 0, 0, tex.Width (), tex.Width (),
					        	(void *) textureManager.m_bmBuf, bmi, DIB_RGB_COLORS, SRCCOPY);
			}
		else 
#endif
			{
			double scale = tex.Scale ();
			int w = tex.Width (0);
			if (scale == 1.0) {
				for (int i = 0, y = w; y > 0; y--)
					for (int x = 0; x < w; x++)
						pDC->SetPixel ((int) x, (int) y, textureManager.m_bmBuf [i++].ColorRef ());
				}
			else {
				for (int y = w - 1; y >= 0; y = (int) (y - scale))
					for (int x = 0; x < w; x = (int) (x + scale))
						pDC->SetPixel ((int) (x / scale), (int) (y / scale), textureManager.m_bmBuf [y * w + x].ColorRef ());
				}
			}
		//pDC->SelectPalette (pOldPalette, FALSE);
		}
	else {
		HGDIOBJ hgdiobj1;
		bmTexture.LoadBitmap ((nOffset [bDescent1] < 0) ? "NO_PIG_BITMAP" : "WRONG_PIG_BITMAP");
		bmTexture.GetObject (sizeof (BITMAP), &bm);
		memDC.CreateCompatibleDC (pDC);
		hgdiobj1 = memDC.SelectObject (bmTexture);
		pDC->StretchBlt (0, 0, rc.Width (), rc.Height (), &memDC, 0, 0, 64, 64, SRCCOPY);
		memDC.SelectObject (hgdiobj1);
		DeleteObject (bmTexture);
//		DeleteDC (memDC.m_hDC);
		}
	}
else if (bkColor < 0) {
	HGDIOBJ hgdiobj;
	// set bitmap
	bmTexture.LoadBitmap ("NO_TEXTURE_BITMAP");
	bmTexture.GetObject (sizeof (BITMAP), &bm);
	memDC.CreateCompatibleDC (pDC);
	hgdiobj = memDC.SelectObject (bmTexture);
	pDC->StretchBlt (0, 0, rc.Width (), rc.Height (), &memDC, 0, 0, 64, 64, SRCCOPY);
	memDC.SelectObject (hgdiobj);
	DeleteObject (bmTexture);
//	DeleteDC (memDC.m_hDC);
	}
else if (bkColor >= 0)
	pDC->FillSolidRect (&rc, (COLORREF) bkColor);
wndP->ReleaseDC (pDC);
wndP->InvalidateRect (null, TRUE);
wndP->UpdateWindow ();
return bShowTexture;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTexture::ComputeIndex (ubyte* palIndex)
{
	CBGR* palette = paletteManager.Current ();

#ifndef _DEBUG
#	pragma omp parallel for
#endif
for (int y = 0; y < (int) m_info.height; y++) {
	int i = y * m_info.width;
	int k = Size () - Width () - i;
	for (int x = 0; x < (int) m_info.width; x++) {
		palIndex [k + x] = paletteManager.ClosestColor (m_data [i + x]);
		}
	}
}

//------------------------------------------------------------------------------

bool CTexture::Allocate (int nSize)
{
if ((m_data != null) && (m_info.bufSize != nSize))
	Release ();
if (m_data == null) {
#ifdef _DEBUG
	if (nSize > 4096 * 4096 * 4)
		m_data = null;
	else
#endif
	m_data = new CBGRA [nSize];
	m_info.bufSize = m_data ? nSize : 0;
	}
return (m_data != null);
}

//------------------------------------------------------------------------

void CTexture::Release (void) 
{
GLRelease ();
if (!m_info.bExtData) {
	if (m_data != null) {
		delete m_data;
		m_data = null;
		}
	}
bool bFrame = m_info.bFrame;
Clear ();
m_info.bFrame = bFrame;
}

//------------------------------------------------------------------------

void CTexture::Load (int nId) 
{
	CResource res;	

ubyte* dataP = res.Load (nId);
if (!dataP)
	return;
tTgaHeader* h = (tTgaHeader*) dataP;
int nSize = int (h->width) * int (h->height);
if (res.Size () < sizeof (tTgaHeader) + nSize * sizeof (CBGRA))
	return;
if (!Allocate (nSize))
	return;
memcpy (m_data, dataP + sizeof (tTgaHeader), nSize * sizeof (m_data [0]));
m_info.width = h->width;
m_info.height = h->height;
m_info.nFormat = 1;
#ifdef _DEBUG
for (dataP += sizeof (tTgaHeader), nSize *= 4; nSize; nSize--, dataP++)
	if (*dataP)
		break;
#endif
}

//------------------------------------------------------------------------

#define RLE_CODE			0xE0
#define NOT_RLE_CODE		31
#define IS_RLE_CODE(x)	(((x) & RLE_CODE) == RLE_CODE)

void CTexture::Load (CFileManager& fp, CPigTexture& info) 
{
	ubyte	rowSize [4096];
	ubyte	rowBuf [4096], *rowPtr;
	ubyte	palIndex, runLength;
	int	width, height;
	bool	bCenter = m_info.bufSize == info.BufSize ();
	CBGR*	palette = paletteManager.Current ();

memcpy (m_info.szName, info.name, sizeof (info.name));
m_info.szName [sizeof (m_info.szName) - 1] = '\0';
m_info.width = info.width;
m_info.height = info.height;
if (bCenter) {
	width = Width ();
	height = Height ();
	}
else {
	width = info.width;
	height = info.height;
	}
m_info.bValid = 1;
m_info.bTransparent = false;
memset (m_data, 0, m_info.bufSize * sizeof (m_data [0]));

m_info.xOffset = (width - info.width) / 2;
m_info.yOffset = (height - info.height) / 2;
if (m_info.xOffset || m_info.yOffset)
	m_info.bValid = 1;

if (m_info.nFormat) {
	tRGBA color;
#if 1
	uint h = info.width * (info.height - 1);
	for (uint i = info.height; i; i--) {
		for (uint j = info.width; j; j--, h++) {
			fp.Read (&color, sizeof (color), 1);
			m_data [h] = color; // different data types -> type conversion!
			if (color.a < 255)
				m_info.bTransparent = true;
			}
		h -= 2 * info.width;
		}
#else
	for (uint i = 0, h = Size (); i < h; i++) {
		fp.Read (&color, sizeof (color), 1);
		m_data [i] = color;
		if (color.a < 255)
			m_info.bTransparent = true;
		}
#endif
	//texP->m_info.bValid = TGA2Bitmap (texP->m_data, texP->m_data, (int) pigTexInfo.width, (int) pigTexInfo.height);
	}
else if (info.flags & 0x08) {
	int nSize = fp.ReadInt32 ();
	fp.ReadBytes (rowSize, info.height);
	int nRow = 0;
	for (int y = info.height - 1; y >= 0; y--) {
		fp.ReadBytes (rowBuf, rowSize [nRow++]);
		rowPtr = rowBuf;
		int h = (y + m_info.yOffset) * width + m_info.xOffset;
		for (int x = 0; x < info.width; ) {
			palIndex = *rowPtr++;
			if (IS_RLE_CODE (palIndex)) {
				runLength = palIndex & ~RLE_CODE;
				if (runLength > info.width - x)
					runLength = info.width - x;
				palIndex = *rowPtr++;
				CBGR color = palette [palIndex];
				ubyte alpha = (palIndex < 254) ? 255 : 0;
				for (int j = 0; j < runLength; j++, x++, h++) {
					m_data [h] = color;
					m_data [h].a = alpha;
					m_info.bTransparent = alpha < 255;
					}
				}
			else {
				m_data [h] = palette [palIndex];
				if (palIndex >= 254) {
					m_data [h].a = 0;
					m_info.bTransparent = true;
					}
				x++, h++;
				}
			}
		}
	}
else {
	for (int y = info.height - 1; y >= 0; y--) {
		int h = (y + m_info.yOffset) * width + m_info.xOffset;
		for (int x = 0; x < info.width; x++, h++) {
			palIndex = fp.ReadUByte ();
			m_data [h] = palette [palIndex];
			if (palIndex >= 254) {
				m_data [h].a = 0;
				m_info.bTransparent = true;
				}
			}
		}
	}
}

//------------------------------------------------------------------------

int CTexture::Load (CFileManager& fp, short nTexture, int nVersion) 
{
if (m_info.bCustom)
	return 0;

#ifdef _DEBUG
if (nTexture == nDbgTexture)
	nDbgTexture = nDbgTexture;
#endif
CPigTexture* infoP = textureManager.Info (nVersion, -nTexture - 1);
if (!infoP)
	return 0;
// bitmaps > 64x64 will not be used by DLE and will therefore not get centered
if ((infoP->width > 64) || (infoP->height > 64))
	return 1;
// bitmaps < 64x64 may be needed for rendering items in the level;
// these will be centered in a 64x64 pixel buffer
// BufSize() computes the buffer size so that it is big enough for a square image
// the side length of which is a multiple of 2 that is big enough to contain the original image
int nSize = infoP->BufSize ();
if (m_data && (m_info.bufSize == nSize))
	return 0; // already loaded
m_info.nFormat = 0;
if (!Allocate (nSize)) 
	return 1;
fp.Seek (textureManager.m_nOffsets [nVersion] + infoP->offset, SEEK_SET);
Load (fp, *infoP);
return 0;
}

//------------------------------------------------------------------------------

bool CTexture::LoadTGA (CFileManager& fp)
{
	tTgaHeader	tgaHeader;
	char			imgIdent [255];
	tBGRA			color;

fp.Read (&tgaHeader, sizeof (tgaHeader), 1);
int h = tgaHeader.width * tgaHeader.height;
if (h > 4096 * 4096) {
	ErrorMsg ("Image too large.");
	return false;
	}
if (!Allocate (h))
	return false;
m_info.width = tgaHeader.width;
m_info.height = tgaHeader.height;
if (tgaHeader.identSize)
	fp.Read (imgIdent, tgaHeader.identSize, 1);
int s = (tgaHeader.bits == 32) ? 4 : 3;
color.a = 255;
CBGRA* buffer = Buffer ();
// textures are getting reversed here for easier rendering!
if (tgaHeader.yStart == 0) { 
	h = m_info.width * (m_info.height - 1);
	for (int i = m_info.height; i; i--) {
		for (int j = m_info.width; j; j--, h++) {
			fp.Read (&color, s, 1);
			buffer [h] = color;
			}
		h -= 2 * m_info.width;
		}
	}
else {
	h = 0;
	for (int i = m_info.height; i; i--) {
		for (int j = m_info.width; j; j--, h++) {
			fp.Read (&color, s, 1);
			buffer [h] = color;
			}
		}
	}
if (DLE.IsD2XLevel ())
	m_info.nFormat = 1;
#if 0
else {
	if (!ToBitmap ())
		return false;
	m_info.nFormat = 0;
	}
#endif
return true;
}

//------------------------------------------------------------------------------

bool CTexture::LoadTGA (char* pszFile)
{
	CFileManager fp;

if (!fp.Open (pszFile, "rb"))
	return false;
bool bSuccess = LoadTGA (fp);
fp.Close ();
return bSuccess;
}

//------------------------------------------------------------------------

int CTexture::Reload (void) 
{
CFileManager* fp = textureManager.OpenPigFile (textureManager.Version ());
if (fp == null)
	return 1;
Release ();
int i = Load (*fp, textureManager.Index (this), textureManager.Version ());
delete fp;
return i;
}

//------------------------------------------------------------------------

double CTexture::Scale (short nTexture)
{
//if (!m_info.width)
//	if (nTexture < 0)
//		return 1.0;
//	else
//		Load (nTexture);
return m_info.width ? (double) m_info.width / 64.0 : 1.0;
}

//	-----------------------------------------------------------------------------

#if 1

#define BPP 4

int CTexture::Shrink (int xFactor, int yFactor)
{
	int		xSrc, ySrc, xMax, yMax, xDest, yDest, x, y, w, h, bSuperTransp;
	CBGRA		* dataP, * srcP, * destP;
	CBGRA		superTranspKey;
	double	cSum [4], nFactor2, nSuperTransp;

if (!m_data)
	return 0;
if ((xFactor < 1) || (yFactor < 1))
	return 0;
if ((xFactor == 1) && (yFactor == 1))
	return 1;
superTranspKey = paletteManager.SuperTranspKey ();
superTranspKey.a = 0;
w = Width ();
h = Height ();
xMax = w / xFactor;
yMax = h / yFactor;
nFactor2 = xFactor * yFactor;
if (!(dataP = new CBGRA [xMax * yMax]))
	return 0;
destP = dataP;

for (yDest = 0; yDest < yMax; yDest++) {
	for (xDest = 0; xDest < xMax; xDest++, destP++) {
		memset (&cSum, 0, sizeof (cSum));
		ySrc = yDest * yFactor;
		nSuperTransp = 0;
		for (y = yFactor; y; ySrc++, y--) {
			xSrc = xDest * xFactor;
			srcP = m_data + (ySrc * w + xSrc);
			for (x = xFactor; x; xSrc++, x--, srcP++) {
				bSuperTransp = *srcP == superTranspKey;
				if (bSuperTransp)
					nSuperTransp++;
				else {
					cSum [0] += srcP->r;
					cSum [1] += srcP->g;
					cSum [2] += srcP->b;
					}
				}
			}
		if (nSuperTransp >= nFactor2 / 2)
			*destP = superTranspKey;
		else {
			destP->r = (ubyte) (cSum [0] / (nFactor2 - nSuperTransp) + 0.5);
			destP->g = (ubyte) (cSum [1] / (nFactor2 - nSuperTransp) + 0.5);
			destP->b = (ubyte) (cSum [2] / (nFactor2 - nSuperTransp) + 0.5);
			}
		}
	}
delete m_data;
m_data = dataP;
m_info.width = xMax;
m_info.height = yMax;
return 1;
}

#endif

//------------------------------------------------------------------------

ubyte* CTexture::ToBitmap (bool bShrink)
{
if (bShrink && !Shrink (Width () / 64, Height () / 64))
	return 0;

CBGRA* colorP = Buffer ();
CBGR* palette = paletteManager.Current ();
int h = Size ();
ubyte* palIndex = new ubyte [h];
if (!palIndex)
	return null;
#if 1
ComputeIndex (palIndex);
#else
#ifndef _DEBUG
#	pragma omp parallel for
#endif
for (int i = 0; i < h; i++) 
	palIndex [i] = palette [paletteManager.ClosestColor (m_data [i])];
#endif
return palIndex;
}

//------------------------------------------------------------------------

GLuint CTexture::GLBind (GLuint nTMU, GLuint nMode)
{
glActiveTexture (nTMU);
glClientActiveTexture (nTMU);
glEnable (GL_TEXTURE_2D);
if (m_info.glHandle)
	glBindTexture (GL_TEXTURE_2D, m_info.glHandle); 
else {
	glGenTextures (1, &m_info.glHandle); 
	if (!m_info.glHandle)
		return 0;
	glBindTexture (GL_TEXTURE_2D, m_info.glHandle); 
	// don't use height: Hires animations have all frames vertically stacked in one texture!
	glTexImage2D (GL_TEXTURE_2D, 0, 4, Width (), Width (), 0, GL_BGRA, GL_UNSIGNED_BYTE, Buffer ()); 
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, nMode);
return m_info.glHandle;
}

//------------------------------------------------------------------------

void CTexture::GLRelease (void)
{
if (m_info.glHandle) {
	glDeleteTextures (1, &m_info.glHandle);
	m_info.glHandle = 0;
	}
}

//------------------------------------------------------------------------------

void CTexture::DrawLine (POINT pt0, POINT pt1, CBGRA color) 
{
CHECKMINE;

	int i, x, y;
	int dx = pt1.x - pt0.x;
	int dy = pt1.y - pt0.y;

	int xInc, yInc;
	double scale;
	int nStep = 0;

if (dx > 0)
	xInc = 1;
else {
	xInc = -1;
	dx = -dx;
	}
if (dy > 0)
	yInc = 1;
else {
	yInc = -1;
	dy = -dy;
	}
scale = Scale ();
xInc = (int) ((double) xInc * scale);
yInc = (int) ((double) yInc * scale);

x = pt0.x;
y = pt0.y;

#if 0	//most universal
int xStep = 0, yStep = 0;
int dd = (dx >= dy) ? dx: dy;
for (i = dd + 1; i; i--) {
	*Buffer (y * texP->m_info.width + x) = color;
	yStep += dy;
	if (yStep >= dx) {
		y += yInc;
		yStep = dx ? yStep % dx: 0;
		}
	xStep += dx;
	if (xStep >= dy) {
		x += xInc;
		xStep = dy ? xStep % dy: 0;
		}
	}
#else //0; faster
if (dx >= dy) {
	for (i = dx + 1; i; i--, x += xInc) {
		*Buffer (y * m_info.width + x) = color;
		nStep += dy;
		if (nStep >= dx) {
			y += yInc;
			nStep -= dx;
			}
		}
	}
else {
	for (i = dy + 1; i; i--, y += yInc) {
		*Buffer (y * m_info.width + x) = color;
		nStep += dx;
		if (nStep >= dy) {
			x += xInc;
			nStep -= dy;
			}
		}
	}
#endif //0
}

//------------------------------------------------------------------------------

void CTexture::DrawAnimDirArrows (short nTexture)
{
	int sx, sy;
	int bScroll = textureManager.ScrollSpeed (nTexture, &sx, &sy);

if (!bScroll)
	return;

	POINT *pointP;
	static POINT ptp0 [4] = {{54,32},{12,32},{42,42},{42,22}};
	static POINT pt0n [4] = {{32,12},{32,54},{42,22},{22,22}};
	static POINT ptn0 [4] = {{12,32},{54,32},{22,22},{22,42}};
	static POINT pt0p [4] = {{32,54},{32,12},{22,42},{42,42}};
	static POINT ptpn [4] = {{54,12},{12,54},{54,22},{42,12}};
	static POINT ptnn [4] = {{12,12},{54,54},{22,12},{12,22}};
	static POINT ptnp [4] = {{12,54},{54,12},{12,42},{22,54}};
	static POINT ptpp [4] = {{54,54},{12,12},{42,54},{54,42}};

if (sx >0 && sy==0) pointP = ptp0;
else if (sx >0 && sy >0) pointP = ptpp;
else if (sx==0 && sy >0) pointP = pt0p;
else if (sx <0 && sy >0) pointP = ptnp;
else if (sx <0 && sy==0) pointP = ptn0;
else if (sx <0 && sy <0) pointP = ptnn;
else if (sx==0 && sy <0) pointP = pt0n;
else if (sx >0 && sy <0) pointP = ptpn;

CBGRA color (255, 255, 255, 255);

DrawLine (pointP [0], pointP [1], color);
DrawLine (pointP [0], pointP [2], color);
DrawLine (pointP [0], pointP [3], color);
}

//------------------------------------------------------------------------

rgbaColorf& CTexture::AverageColor (rgbaColorf& color)
{
if (Flat ())
	color = m_info.averageColor;
else {
	CBGRA* bufP = Buffer ();
	color.r = color.g = color.b = color.a = 0.0f;
	if (bufP) {
		int bufSize = Width (0) * Height (0);
		for (int i = bufSize; i; i--, bufP++) {
			color.r += bufP->r;
			color.g += bufP->g;
			color.b += bufP->b;
			color.a += bufP->a;
			}
		color.r /= float (bufSize);
		color.g /= float (bufSize);
		color.b /= float (bufSize);
		color.a /= float (bufSize);
		}
	}
return color;
}

//------------------------------------------------------------------------
//eof textures.cpp