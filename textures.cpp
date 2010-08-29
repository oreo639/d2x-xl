#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "cfile.h"
#include "dle-xp-res.h"
#include "global.h"
#include "palette.h"
#include "textures.h"
#include "dle-xp.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

void RgbFromIndex (int nIndex, PALETTEENTRY& rgb)
{
CResource res;
byte* palette;
if (palette = res.Load (PaletteResource ())) {
	palette += 3 * nIndex;
	rgb.peRed = palette [0] << 2;
	rgb.peGreen = palette [1] << 2;
	rgb.peBlue = palette [2] << 2;
	rgb.peFlags = 0;
	}
}

//------------------------------------------------------------------------

BITMAPINFO *MakeBitmap (void) 
{
	typedef struct tagMyBMI {
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];
		} MyBMI;

	static MyBMI my_bmi;

	// point bitmap info to structure
BITMAPINFO* bmi = (BITMAPINFO *)&my_bmi;
// point the info to the bitmap header structure
BITMAPINFOHEADER& bi = bmi->bmiHeader;
// define bit map info header elements
bi.biSize          = sizeof (BITMAPINFOHEADER);
bi.biWidth         = 64;
bi.biHeight        = 64;
bi.biPlanes        = 1;
bi.biBitCount      = 8;
bi.biCompression   = BI_RGB;
bi.biSizeImage     = 0;
bi.biXPelsPerMeter = 0;
bi.biYPelsPerMeter = 0;
bi.biClrUsed       = 0;
bi.biClrImportant  = 0;

// define d1 palette from resource
CResource res;
byte* palette;
if (!(palette = res.Load (PaletteResource ())))
	return null;
for (int i = 0; i < 256; i++) {
	bmi->bmiColors [i].rgbRed = (*palette++) << 2;
	bmi->bmiColors [i].rgbGreen = (*palette++) << 2;
	bmi->bmiColors [i].rgbBlue = (*palette++) << 2;
	bmi->bmiColors [i].rgbReserved = 0;
	}
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

	HINSTANCE		hInst = AfxGetApp ()->m_hInstance;
	CBitmap			bmTexture;
	CFileManager	fp;
	char				szFile [256];
	BITMAP			bm;
	CDC				memDC;
	CSegment*		segP;
	CSide*			sideP;
	short				nWall;
	bool				bShowTexture = true, bDescent1 = DLE.IsD1File ();
	char*				path = bDescent1 ? descent_path : descent2_path;

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
		if (fp.Open (szFile, "rb"))
			nOffset [bDescent1] = -1;  // pig file not found
		else {
			fp.Seek (0, SEEK_SET);
			nOffset [bDescent1] = fp.ReadInt32 ();  // determine type of pig file
			fp.Close ();
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
wndP->InvalidateRect (null, TRUE);
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
if (m_info.bmDataP == null)
	m_info.bmDataP = new byte [nSize];
return (m_info.bmDataP != null);
}

//------------------------------------------------------------------------

void CTexture::Load (CFileManager& fp, CPigTexture& info) 
{
	byte	rowSize [4096];
	byte	rowBuf [4096], *rowPtr;
	byte	byteVal, runLength, runValue;

m_info.nFormat = 0;
if (info.flags & 0x08) {
	int nSize = fp.ReadInt32 ();
	fp.ReadBytes (rowSize, info.height);
	int nRow = 0;
	for (int y = info.height - 1; y >= 0; y--) {
		fp.ReadBytes (rowBuf, rowSize [nRow++]);
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
		fp.Read (m_info.bmDataP + y * info.width, info.width, 1);
		}
	}
m_info.width = info.width;
m_info.height = info.height;
m_info.size = info.BufSize ();
m_info.bValid = 1;
}

//------------------------------------------------------------------------

int CTexture::Load (short nTexture, int nVersion, CFileManager* fp) 
{
if (m_info.bModified)
	return 0;

if (nVersion < 0)
	nVersion = DLE.IsD1File () ? 0 : 1;

bool	bLocalFile = (fp == null);
	
if (!(fp || textureManager.HaveInfo (nVersion)))
	fp = textureManager.OpenPigFile (nVersion);

CPigTexture& info = textureManager.LoadInfo (*fp, nVersion, nTexture);
int nSize = info.BufSize ();
if (m_info.bmDataP && ((m_info.width * m_info.height == nSize)))
	return 0; // already loaded
if (!fp)
	fp = textureManager.OpenPigFile (nVersion);
if (!Allocate (nSize, nTexture)) {
	if (bLocalFile)
		fp->Close ();	
	return 1;
	}
fp->Seek (textureManager.nOffsets [nVersion] + info.offset, SEEK_SET);
Load (*fp, info);
if (bLocalFile)
	fp->Close ();
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

//eof textures.cpp