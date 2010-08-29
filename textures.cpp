#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "io.h"
#include "dle-xp-res.h"
#include "global.h"
#include "palette.h"
#include "textures.h"
#include "dle-xp.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

void RgbFromIndex (int nIndex, PALETTEENTRY *pRGB)
{
if (pRGB) {
	HRSRC hFind = FindResource (hInst, PaletteResource (), "RC_DATA");
	if (hFind) {
		HGLOBAL hPalette = LoadResource (hInst, hFind);
		byte *palette = ((byte *) LockResource (hPalette)) + 3 * nIndex;
		pRGB->peRed = (*palette++)<<2;
		pRGB->peGreen = (*palette++)<<2;
		pRGB->peBlue = (*palette)<<2;
		pRGB->peFlags = 0;
		}
	}
}

//------------------------------------------------------------------------

BITMAPINFO *MakeBitmap (void) 
{
	BITMAPINFO *bmi;
	BITMAPINFOHEADER *bi;
	short i;
	HRSRC hFind;
	HGLOBAL hPalette;
	byte *palette;

	typedef struct tagMyBMI {
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];
		} MyBMI;

static MyBMI my_bmi;

	// point bitmap info to structure
	bmi = (BITMAPINFO *)&my_bmi;

	// point the info to the bitmap header structure
	bi = &bmi->bmiHeader;

	// define bit map info header elements
	bi->biSize          = sizeof (BITMAPINFOHEADER);
	bi->biWidth         = 64;
	bi->biHeight        = 64;
	bi->biPlanes        = 1;
	bi->biBitCount      = 8;
	bi->biCompression   = BI_RGB;
	bi->biSizeImage     = 0;
	bi->biXPelsPerMeter = 0;
	bi->biYPelsPerMeter = 0;
	bi->biClrUsed       = 0;
	bi->biClrImportant  = 0;

	// define d1 palette from resource
	hFind = FindResource (hInst, PaletteResource (), "RC_DATA");
	if (!hFind) {
		DEBUGMSG (" Bitmap creation: Palette resource not found.");
		return NULL;
		}
	hPalette = LoadResource (hInst, hFind);
	palette = (byte *) LockResource (hPalette);
#if 0
	for (i=0;i<256;i++) {
	  bmi->bmiColors[i].rgbRed      = *palette++;
	  bmi->bmiColors[i].rgbGreen    = *palette++;
	  bmi->bmiColors[i].rgbBlue     = *palette++;
	  bmi->bmiColors[i].rgbReserved = 0;palette++;
	}
#else
	for (i=0;i<256;i++) {
	  bmi->bmiColors[i].rgbRed    = (*palette++)<<2;
	  bmi->bmiColors[i].rgbGreen  = (*palette++)<<2;
	  bmi->bmiColors[i].rgbBlue   = (*palette++)<<2;
	  bmi->bmiColors[i].rgbReserved = 0;
	}
#endif
	FreeResource (hPalette);
  return bmi;
}

// -------------------------------------------------------------------------- 

bool PaintTexture (CWnd *wndP, int bkColor, int nSegment, int nSide, int nBaseTex, int nOvlTex, int xOffset, int yOffset)
{
if (!theMine) 
	return false;

	static int nOffset [2] = {0, 0};

	CDC			*pDC = wndP->GetDC ();

if (!pDC)
	return false;

	HINSTANCE	hInst = AfxGetApp ()->m_hInstance;
	CBitmap		bmTexture;
	FILE			*fp = NULL;
	char			szFile [256];
	BITMAP		bm;
	CDC			memDC;
	CSegment*	segP;
	CSide*		sideP;
	short			nWall;
	bool			bShowTexture = true, bDescent1 = DLE.IsD1File ();
	char			*path = bDescent1 ? descent_path : descent2_path;

CRect	rc;
wndP->GetClientRect (rc);

if (nBaseTex < 0) {
	segP = (nSegment < 0) ? theMine->CurrSeg () : theMine->Segments (nSegment);
	sideP = (nSide < 0) ? theMine->CurrSide () : segP->m_sides + nSide;
	int nSide = theMine->Current ()->nSide;
	nBaseTex = sideP->m_info.nBaseTex;
	nOvlTex = sideP->m_info.nOvlTex & 0x1fff;
	if (segP->Child (nSide) == -1)
		bShowTexture = TRUE;
	else {
		nWall = sideP->m_info.nWall;
		bShowTexture = (nWall < theMine->GameInfo ().walls.count);
		}
	}
if ((nBaseTex < 0) || (nBaseTex >= textureManager.MaxTextures () + 10))
	bShowTexture = false;
if ((nOvlTex < 0) || (nOvlTex >= textureManager.MaxTextures ()))	// this allows to suppress bitmap display by 
	bShowTexture = false;									// passing 0 for texture 1 and -1 for nOvlTex

if (bShowTexture) {
	// check pig file
	if (nOffset [bDescent1] == 0) {
		strcpy_s (szFile, sizeof (szFile), (bDescent1) ? descent_path : descent2_path);
		if (fopen_s (&fp, szFile, "rb"))
			nOffset [bDescent1] = -1;  // pig file not found
		else {
			fseek (fp, 0, SEEK_SET);
			nOffset [bDescent1] = ReadInt32 (fp);  // determine type of pig file
			fclose (fp);
			}
		}
	if (nOffset [bDescent1] > 0x10000L) {  // pig file type is v1.4a or descent 2 type
		CTexture	tex (textureManager.bmBuf);
		if (textureManager.Define (nBaseTex, nOvlTex, &tex, xOffset, yOffset))
			DEBUGMSG (" Texture renderer: Texture not found (textureManager.Define failed)");
		CPalette *pOldPalette = pDC->SelectPalette (theMine->m_currentPalette, FALSE);
		pDC->RealizePalette ();
		int caps = pDC->GetDeviceCaps (RASTERCAPS);
		if (caps & RC_DIBTODEV) {
			BITMAPINFO *bmi = MakeBitmap ();
			if (!bmi)
				return false;
			bmi->bmiHeader.biWidth = 
			bmi->bmiHeader.biHeight = tex.m_info.width;
			StretchDIBits (pDC->m_hDC, 0, 0, rc.Width (), rc.Height (), 0, 0, tex.m_info.width, tex.m_info.width,
					        	 (void *) textureManager.bmBuf, bmi, DIB_RGB_COLORS, SRCCOPY);
			}
		else {
			double scale = tex.Scale ();
			uint x, y;
			for (x = 0; x < tex.m_info.width; x = (int) (x + scale))
				for (y = 0; y < tex.m_info.width; y = (int) (y + scale))
					pDC->SetPixel ((short) (x / scale), (short) (y / scale), PALETTEINDEX (textureManager.bmBuf [y * tex.m_info.width + x]));
			}
		pDC->SelectPalette (pOldPalette, FALSE);
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
wndP->InvalidateRect (NULL, TRUE);
wndP->UpdateWindow ();
return bShowTexture;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

bool CTexture::Allocate (int nSize, int nTexture)
{
if (m_info.bmDataP && ((m_info.width * m_info.height != nSize)))
	Release ();
if (m_info.bmDataP == NULL)
	m_info.bmDataP = new byte [nSize];
return (m_info.bmDataP != NULL);
}

//------------------------------------------------------------------------

void CTexture::Load (FILE* fp, CPigTexture& info) 
{
	byte	rowSize [4096];
	byte	rowBuf [4096], *rowPtr;
	byte	byteVal, runLength, runValue;

m_info.nFormat = 0;
if (info.flags & 0x08) {
	int nSize = ReadInt32 (fp);
	ReadBytes (rowSize, info.height, fp);
	int nRow = 0;
	for (int y = info.height - 1; y >= 0; y--) {
		fread (rowBuf, rowSize [nRow++], 1, fp);
		rowPtr = rowBuf;
			for (int x = 0; x < info.width; ) {
			byteVal = *rowPtr++;
			if ((byteVal & 0xe0) == 0xe0) {
				runLength = byteVal & 0x1f;
				runValue = *rowPtr++;
				for (int j = 0; j < runLength; j++) {
					if (x < info.width) {
						m_info.bmDataP [y * info.width + x] = runValue;
						x++;
						}
					}
				}
			else {
				m_info.bmDataP [y * info.width + x] = byteVal;
				x++;
				}
			}
		}
	}
else {
	for (int y = info.height - 1; y >= 0; y--) {
		fread (m_info.bmDataP + y * info.width, info.width, 1, fp);
		}
	}
m_info.width = info.width;
m_info.height = info.height;
m_info.size = info.BufSize ();
m_info.bValid = 1;
}

//------------------------------------------------------------------------

int CTexture::Load (short nTexture, int nVersion, FILE* fp) 
{
if (m_info.bModified)
	return 0;

if (nVersion < 0)
	nVersion = DLE.IsD1File () ? 0 : 1;

bool	bLocalFile = (fp == NULL);
	
if (!(fp || textureManager.HaveInfo (nVersion)))
	fp = textureManager.OpenPigFile (nVersion);

if (nTexture == 326)
	nTexture = nTexture;
CPigTexture& info = textureManager.LoadInfo (fp, nVersion, nTexture);
int nSize = info.BufSize ();
if (m_info.bmDataP && ((m_info.width * m_info.height == nSize)))
	return 0; // already loaded
if (!fp)
	fp = textureManager.OpenPigFile (nVersion);
if (!Allocate (nSize, nTexture)) {
	if (bLocalFile)
		fclose (fp);	
	return 1;
	}
fseek (fp, textureManager.nOffsets [nVersion] + info.offset, SEEK_SET);
Load (fp, info);
if (bLocalFile)
	fclose (fp);
return 0;
}

//------------------------------------------------------------------------

double CTexture::Scale (short nTexture)
{
if (!m_info.width)
	if (nTexture < 0)
		return 1.0;
	else
		Load (nTexture);
return m_info.width ? (double) m_info.width / 64.0 : 1.0;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

void CTextureManager::Setup (void)
{
header [0] = CPigHeader (0);
header [1] = CPigHeader (1);
LoadIndex (0);
LoadIndex (1);
LoadTextures (0);
LoadTextures (1);
}

//------------------------------------------------------------------------

void CTextureManager::Release (bool bDeleteModified) 
{
// free any textures that have been buffered
for (int i = 0; i < 2; i++) {
	CTexture* texP = &textures [i][0];
	for (int j = MaxTextures (i); j; j--, texP++)
		if (bDeleteModified || !texP->m_info.bModified)
			texP->Release ();
	}
pExtraTexture p;
while (extraTextures) {
	p = extraTextures;
	extraTextures = p->pNext;
	delete p;
	}
}

//------------------------------------------------------------------------

FILE* CTextureManager::OpenPigFile (int nVersion)
{
	FILE* fp = NULL;
	char	filename [256];

strcpy_s (filename, sizeof (filename), nVersion ? descent2_path : descent_path);
if (!strstr (filename, ".pig"))
	strcat_s (filename, sizeof (filename), "groupa.pig");
if (fopen_s (&fp, filename, "rb")) {
	DEBUGMSG (" Reading texture: Texture file not found.");
	return NULL;
	}
uint nOffset = ReadUInt32 (fp);
if (nOffset == 0x47495050) // 'PPIG' Descent 2 type
	nOffset = 0;
else if (nOffset < 0x10000)
	nOffset = 0;
fseek (fp, nOffset, SEEK_SET);
return fp;
}

//------------------------------------------------------------------------

int CTextureManager::MaxTextures (int nVersion)
{
return ((nVersion < 0) ? DLE.IsD2File () : nVersion) ? MAX_D2_TEXTURES : MAX_D1_TEXTURES;
}

//------------------------------------------------------------------------

bool CTextureManager::HasCustomTextures (void) 
{
CTexture* texP = &textures [DLE.FileType ()][0];

for (int i = MaxTextures (DLE.FileType ()); i; i--, texP++)
	if (texP->m_info.bModified)
		return true;
return false;
}

//------------------------------------------------------------------------

int CTextureManager::CountCustomTextures (void) 
{
	int			count = 0;
	CTexture*	texP = &textures [DLE.FileType ()][0];

for (int i = MaxTextures (); i; i--, texP++)
	if (texP->m_info.bModified)
		count++;
return count;
}

//------------------------------------------------------------------------------

bool CTextureManager::Check (int nTexture)
{
if ((nTexture >= 0) && (nTexture < MaxTextures ()))
	return true;
sprintf_s (message, sizeof (message), "Reading texture: Texture #%d out of range.", nTexture);
DEBUGMSG (message);
return false;
}

//------------------------------------------------------------------------------

void CTextureManager::Load (ushort nBaseTex, ushort nOvlTex)
{
if (Check (nBaseTex)) {
   textures [(int)DLE.FileType ()] [nBaseTex].Load (nBaseTex);
   if (Check (nOvlTex & 0x1FFF) && ((nOvlTex & 0x1FFF) != 0)) {
       Check ((ushort)(nOvlTex & 0x1FFF));
       textures [(int)DLE.FileType ()] [nOvlTex].Load (nBaseTex);
		}
   }
}

//------------------------------------------------------------------------------

int CTextureManager::LoadIndex (int nVersion)
{
HINSTANCE hInst = AfxGetInstanceHandle ();
HRSRC hRes = FindResource (hInst, MAKEINTRESOURCE (nVersion ? IDR_TEXTURE2_DAT : IDR_TEXTURE_DAT), "RC_DATA");
if (!hRes) {
	DEBUGMSG (" Reading texture: Texture index not found.");
	return 1;
	}
HGLOBAL hGlobal = LoadResource (hInst, hRes);
if (!hGlobal) {
	DEBUGMSG (" Reading texture: Could not load texture index.");
	return 2;
	}
// first long is number of textures
ushort* indexP = (ushort *) LockResource (hGlobal);
nTextures [nVersion] = *((uint*) indexP);
indexP += 2;
if (!(index [nVersion] = new ushort [nTextures [nVersion]])) {
	FreeResource (hGlobal);
	DEBUGMSG (" Reading texture: Could not allocate texture index.");
	return 3;
	}
for (uint i = 0; i < nTextures [nVersion]; i++)
	index [nVersion][i] = (*indexP++) - 1;
FreeResource (hGlobal);
return 0;
}

//------------------------------------------------------------------------------

CPigTexture& CTextureManager::LoadInfo (FILE* fp, int nVersion, short nTexture)
{
if (info [nVersion] == NULL) {
	header [nVersion].Read (fp);
	info [nVersion] = new CPigTexture [header [nVersion].nTextures];
	for (int i = 0; i < header [nVersion].nTextures; i++)
		{
		if (i == 1554)
			i = i;
		info [nVersion][i].Read (fp, nVersion);
		nOffsets [nVersion] = ftell (fp);
		}
	}
return info [nVersion][index [nVersion][nTexture]];
}

//------------------------------------------------------------------------------

void CTextureManager::LoadTextures (int nVersion)
{
header [nVersion] = CPigHeader (nVersion);
LoadIndex (nVersion);
#if USE_DYN_ARRAYS
textures [nVersion].Create (MaxTextures (nVersion));
#else
textures [nVersion] = new CTexture [MaxTextures (nVersion)];
#endif

	FILE* fp = OpenPigFile (nVersion);

for (int i = 0, j = MaxTextures (nVersion); i < j; i++)
	textures [nVersion][i].Load (i, nVersion, fp);
fclose (fp);
}

//------------------------------------------------------------------------------
// textureManager.Define ()
//
// ACTION - Defines data with texture.  If bitmap_handle != 0,
//          then data is copied from the global data.  Otherwise,
//          the pig file defines bmBuf and then global memory
//          is allocated and the data is copied to the global memory.
//          The next time that texture is used, the handle will be defined.
//------------------------------------------------------------------------------

int CTextureManager::Define (short nBaseTex, short nOvlTex, CTexture *destTexP, int x0, int y0) 
{
	typedef struct tFrac {
		int	c, d;
	} tFrac;

	byte			*ptr;
	short			nTextures [2], mode, w, h;
	int			i, x, y, y1, offs, s;
	tFrac			scale, scale2;
	int			rc; // return code
	CTexture*	texP [2];
	byte			*bmBufP = destTexP->m_info.bmDataP;
	byte			c;
	int			fileType = DLE.FileType ();

	
nTextures [0] = nBaseTex;
nTextures [1] = nOvlTex & 0x3fff;
mode = nOvlTex & 0xC000;
for (i = 0; i < 2; i++) {
#if 0	
	ASSERT (textures [i] < MAX_TEXTURES);
#endif
	if ((nTextures [i] < 0) || (nTextures [i] >= MAX_TEXTURES))
		nTextures [i] = 0;
	// buffer textures if not already buffered
	texP [i] = &textures [fileType][nTextures [i]];
	if (!(texP [i]->m_info.bmDataP && texP [i]->m_info.bValid))
		if (rc = texP [i]->Load (nTextures [i]))
			return rc;
	}
	
	// Define bmBufP based on texture numbers and rotation
destTexP->m_info.width = texP [0]->m_info.width;
destTexP->m_info.height = texP [0]->m_info.height;
destTexP->m_info.size = texP [0]->m_info.size;
destTexP->m_info.bValid = 1;
ptr = texP [0]->m_info.bmDataP;
if (ptr) {
	// if not rotated, then copy directly
	if (x0 == 0 && y0 == 0) 
		memcpy (bmBufP, ptr, texP [0]->m_info.size);
	else {
		// otherwise, copy bit by bit
		w = texP [0]->m_info.width;
#if 1
		int	l1 = y0 * w + x0;
		int	l2 = texP [0]->m_info.size - l1;
		memcpy (bmBufP, ptr + l1, l2);
		memcpy (bmBufP + l2, ptr, l1);
#else
		byte		*dest = bmBufP;
		h = w;//texP [0]->m_info.height;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				*dest++ = ptr [(((y - y0 + h) % h) * w) + ((x - x0 + w) % w)];
#endif
		}
	}

// Overlay texture 2 if present

if (nTextures [1] == 0)
	return 0;
if (!(ptr = texP [1]->m_info.bmDataP))
	return 0;
if (texP [0]->m_info.width == texP [1]->m_info.width)
	scale.c = scale.d = 1;
else if (texP [0]->m_info.width < texP [1]->m_info.width) {
	scale.c = texP [1]->m_info.width / texP [0]->m_info.width;
	scale.d = 1;
	}
else if (texP [0]->m_info.width > texP [1]->m_info.width) {
	scale.d = texP [0]->m_info.width / texP [1]->m_info.width;
	scale.c = 1;
	}
scale2.c = scale.c * scale.c;
scale2.d = scale.d * scale.d;
offs = 0;
w = texP [1]->m_info.width / scale.c * scale.d;
h = w;//texP [1]->m_info.height / scale.c * scale.d;
s = (texP [1]->m_info.width * texP [1]->m_info.width)/*texP [1]->m_info.size*/ / scale2.c * scale2.d;
if (!(x0 || y0)) {
	byte *dest, *dest2;
	if (mode == 0x0000) {
		dest = bmBufP;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++, dest++) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest = c;
				}
		}
	else if (mode == (short) 0x4000) {
		dest = bmBufP + h - 1;
		for (y = 0; y < h; y++, dest--)
			for (x = 0, dest2 = dest; x < w; x++, dest2 += w) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest2 = c;
				}
		}
	else if (mode == (short) 0x8000) {
		dest = bmBufP + s - 1;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++, dest--) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest = c;
				}
		}
	else if (mode == (short) 0xC000) {
		dest = bmBufP + (h - 1) * w;
		for (y = 0; y < h; y++, dest++)
			for (x = 0, dest2 = dest; x < w; x++, dest2 -= w) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest2 = c;
				}
		}
	} 
else {
	if (mode == 0x0000) {
		for (y = 0; y < h; y++) {
			y1 = ((y + y0) % h) * w;
			for (x = 0; x < w; x++) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBufP [y1 + (x + x0) % w] = c;
				}
			}
		}
	else if (mode == (short) 0x4000) {
		for (y = h - 1; y >= 0; y--)
			for (x = 0; x < w; x++) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBufP [((x + y0) % h) * w + (y + x0) % w] = c;
				}
		}
	else if (mode == (short) 0x8000) {
		for (y = h - 1; y >= 0; y--) {
			y1 = ((y + y0) % h) * w;
			for (x = w - 1; x >= 0; x--) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBufP [y1 + (x + x0) % w] = c;
				}
			}
		}
	else if (mode == (short) 0xC000) {
		for (y = 0; y < h; y++)
			for (x = w - 1; x >= 0; x--) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBufP [((x + y0) % h) * w + (y + x0) % w] = c;
				}
			}
	}
return 0;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//eof textures.cpp