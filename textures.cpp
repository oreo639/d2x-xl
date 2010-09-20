#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "FileManager.h"
#include "dle-xp-res.h"
#include "global.h"
#include "PaletteManager.h"
#include "TextureManager.h"
#include "dle-xp.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

void RgbFromIndex (int nIndex, PALETTEENTRY& rgb)
{
CResource res;
COLORREF* color = paletteManager.Current (nIndex);
if (color != null) {
	rgb.peRed = GetRValue (*color);
	rgb.peGreen = GetGValue (*color);
	rgb.peBlue = GetBValue (*color);
	rgb.peFlags = 0;
	}
}

// -------------------------------------------------------------------------- 

bool PaintTexture (CWnd *wndP, int bkColor, int nSegment, int nSide, int nBaseTex, int nOvlTex, int xOffset, int yOffset)
{
if (theMine == null) 
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
	char*				path = bDescent1 ? descentPath [0] : descentPath [1];

CRect	rc;
wndP->GetClientRect (rc);

if (nBaseTex < 0) {
	segP = (nSegment < 0) ? current->Segment () : segmentManager.Segment (nSegment);
	sideP = (nSide < 0) ? current->Side () : segP->m_sides + nSide;
	int nSide = current->m_nSide;
	nBaseTex = sideP->m_info.nBaseTex;
	nOvlTex = sideP->m_info.nOvlTex & 0x1fff;
	if (segP->Child (nSide) == -1)
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
		strcpy_s (szFile, sizeof (szFile), (bDescent1) ? descentPath [0] : descentPath [1]);
		if (fp.Open (szFile, "rb"))
			nOffset [bDescent1] = -1;  // pig file not found
		else {
			fp.Seek (0, SEEK_SET);
			nOffset [bDescent1] = fp.ReadInt32 ();  // determine type of pig file
			fp.Close ();
			}
		}
	if (nOffset [bDescent1] > 0x10000L) {  // pig file type is v1.4a or descent 2 type
		CTexture	tex (textureManager.m_bmBuf);
		if (textureManager.Define (nBaseTex, nOvlTex, &tex, xOffset, yOffset))
			DEBUGMSG (" Texture renderer: Texture not found (textureManager.Define failed)");
		//CPalette *pOldPalette = pDC->SelectPalette (paletteManager.Render (), FALSE);
		//pDC->RealizePalette ();
#if 0
		if (pDC->GetDeviceCaps (RASTERCAPS) & RC_DIBTODEV) {
			BITMAPINFO* bmi = paletteManager.BMI ();
			bmi->bmiHeader.biWidth = 
			bmi->bmiHeader.biHeight = tex.m_info.width;
			bmi->bmiHeader.biSizeImage = ((((bmi->bmiHeader.biWidth * bmi->bmiHeader.biBitCount) + 31) & ~31) >> 3) * bmi->bmiHeader.biHeight;
			StretchDIBits (pDC->m_hDC, 0, 0, rc.Width (), rc.Height (), 0, 0, tex.m_info.width, tex.m_info.width,
					        	(void *) textureManager.m_bmBuf, bmi, DIB_RGB_COLORS, SRCCOPY);
			}
		else 
#endif
			{
			double scale = tex.Scale ();
			uint x, y;
			for (x = 0; x < tex.m_info.width; x = (int) (x + scale))
				for (y = 0; y < tex.m_info.width; y = (int) (y + scale))
					pDC->SetPixel ((short) (x / scale), (short) (y / scale), /*PALETTEINDEX*/ (textureManager.m_bmBuf [y * tex.m_info.width + x]));
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

bool CTexture::Allocate (int nSize, int nTexture)
{
if ((m_info.bmData != null) && ((m_info.width * m_info.height != nSize)))
	Release ();
if (m_info.bmData == null)
	m_info.bmData = new COLORREF [nSize];
if ((m_info.nFormat == 0) && (m_info.bmIndex == null))
	m_info.bmIndex = new byte [nSize];
return (m_info.bmData != null) && ((m_info.nFormat == 0) || (m_info.bmIndex != null));
}

//------------------------------------------------------------------------

void CTexture::Release (void) 
{
if (!m_info.bExtData) {
	if (m_info.bmData) {
		delete m_info.bmData;
		m_info.bmData = null;
		}
	if (m_info.bmIndex) {
		delete m_info.bmIndex;
		m_info.bmIndex = null;
		}
	}
bool bFrame = m_info.bFrame;
Clear ();
m_info.bFrame = bFrame;
}

//------------------------------------------------------------------------

#define RLE_CODE			0xE0
#define NOT_RLE_CODE		31
#define IS_RLE_CODE(x)	(((x) & RLE_CODE) == RLE_CODE)

void CTexture::Load (CFileManager& fp, CPigTexture& info) 
{
	byte			rowSize [4096];
	byte			rowBuf [4096], *rowPtr;
	byte			palIndex, runLength;
	COLORREF*	palette = paletteManager.Current ();

m_info.width = info.width;
m_info.height = info.height;
m_info.size = info.BufSize ();
m_info.bValid = 1;
if (m_info.nFormat) {
	tRGBA color;
	for (uint i = 0; i < m_info.size; i++) {
		fp.Read (&color, sizeof (color), 1);
		m_info.bmData [i] = RGB (color.r, color.g, color.b);
		}
	//texP->m_info.bValid = TGA2Bitmap (texP->m_info.bmData, texP->m_info.bmData, (int) pigTexInfo.width, (int) pigTexInfo.height);
	}
else if (info.flags & 0x08) {
	int nSize = fp.ReadInt32 ();
	fp.ReadBytes (rowSize, info.height);
	int nRow = 0;
	for (int y = info.height - 1; y >= 0; y--) {
		fp.ReadBytes (rowBuf, rowSize [nRow++]);
		rowPtr = rowBuf;
			for (int x = 0; x < info.width; ) {
			palIndex = *rowPtr++;
			if (IS_RLE_CODE (palIndex)) {
				runLength = palIndex & ~RLE_CODE;
				palIndex = *rowPtr++;
				COLORREF color = palette [palIndex];
				for (int j = 0; j < runLength; j++) {
					if (x < info.width) {
						int h = y * info.width + x++;
						m_info.bmIndex [h] = palIndex;
						m_info.bmData [h] = color;
						}
					}
				}
			else {
				int h = y * info.width + x++;
				m_info.bmIndex [h] = palIndex;
				m_info.bmData [h] = palette [palIndex];
				}
			}
		}
	}
else {
	for (int y = info.height - 1; y >= 0; y--) {
		for (int x = 0; x < info.width; x++) {
			int h = y * info.width + x;
			palIndex = fp.ReadByte ();
			m_info.bmIndex [h] = palIndex;
			m_info.bmData [h] = palette [palIndex];
			}
		}
	}
}

//------------------------------------------------------------------------

int CTexture::Load (CFileManager& fp, short nTexture, int nVersion) 
{
if (m_info.bCustom)
	return 0;

if (nVersion < 0)
	nVersion = DLE.IsD1File () ? 0 : 1;

CPigTexture& info = textureManager.Info (nVersion, nTexture);
int nSize = info.BufSize ();
if (m_info.bmData && ((m_info.width * m_info.height == nSize)))
	return 0; // already loaded
m_info.nFormat = 0;
if (!Allocate (nSize, nTexture)) 
	return 1;
fp.Seek (textureManager.m_nOffsets [nVersion] + info.offset, SEEK_SET);
Load (fp, info);
m_info.bFrame = (strstr (textureManager.m_names [nVersion][nTexture], "frame") != null);
return 0;
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

//------------------------------------------------------------------------

//eof textures.cpp