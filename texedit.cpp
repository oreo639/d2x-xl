
#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <commdlg.h>
#include <math.h>

#include "mine.h"
#include "dle-xp.h"
#include "toolview.h"
#include "PaletteManager.h"
#include "TextureManager.h"

//------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CPaletteWnd, CWnd)
#if 0
	ON_WM_LBUTTONDOWN ()
	ON_WM_RBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
	ON_WM_RBUTTONUP ()
#endif
END_MESSAGE_MAP ()

//------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CTextureEdit, CDialog)
	ON_WM_PAINT ()
	ON_WM_MOUSEMOVE ()
	ON_WM_LBUTTONDOWN ()
	ON_WM_RBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
	ON_WM_RBUTTONUP ()
	ON_BN_CLICKED (IDC_TEXEDIT_DEFAULT, OnDefault)
	ON_BN_CLICKED (IDC_TEXEDIT_UNDO, OnUndo)
	ON_BN_CLICKED (IDC_TEXEDIT_LOAD, OnLoad)
	ON_BN_CLICKED (IDC_TEXEDIT_SAVE, OnSave)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------------

CPaletteWnd::CPaletteWnd ()
{
m_nWidth =
m_nHeight = 0;
m_pDC = null;
m_pOldPal = null;
}

//------------------------------------------------------------------------------

CPaletteWnd::~CPaletteWnd ()
{
}

//------------------------------------------------------------------------------

#define	MINRGB(rgb)	(((rgb)->peRed < (rgb)->peGreen) ? ((rgb)->peRed < (rgb)->peBlue) ? (rgb)->peRed : (rgb)->peBlue : ((rgb)->peGreen < (rgb)->peBlue) ? (rgb)->peGreen : (rgb)->peBlue)
#define	MAXRGB(rgb)	(((rgb)->peRed > (rgb)->peGreen) ? ((rgb)->peRed > (rgb)->peBlue) ? (rgb)->peRed : (rgb)->peBlue : ((rgb)->peGreen > (rgb)->peBlue) ? (rgb)->peGreen : (rgb)->peBlue)

#define sqr(v)	(((int)(v))*((int)(v)))

int CPaletteWnd::CmpColors (PALETTEENTRY *c, PALETTEENTRY *m)
{
int i = c->peRed + c->peGreen + c->peBlue; //Luminance (c->peRed, c->peGreen, c->peBlue);
int j = m->peRed + m->peGreen + m->peBlue; //Luminance (m->peRed, m->peGreen, m->peBlue);
if (i < j)
	return -1;
if (i > j)
	return 1;
if (c->peRed < m->peRed)
	return -1;
if (c->peRed > m->peRed)
	return 1;
if (c->peGreen < m->peGreen)
	return -1;
if (c->peGreen > m->peGreen)
	return 1;
if (c->peBlue < m->peBlue)
	return -1;
if (c->peBlue > m->peBlue)
	return 1;
return 0;
}

//------------------------------------------------------------------------------

void CPaletteWnd::SortPalette (int left, int right)
{
	int				l = left, 
						r = right;
	PALETTEENTRY	m = m_palColors [(l + r) / 2];

do {
	while (CmpColors (m_palColors + l, &m) < 0)
		l++;
	while (CmpColors (m_palColors + r, &m) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			PALETTEENTRY h = m_palColors [l];
			m_palColors [l] = h = m_palColors [r];
			m_palColors [r] = h;
			ubyte i = m_nSortedPalIdx [l];
			m_nSortedPalIdx [l] = m_nSortedPalIdx [r];
			m_nSortedPalIdx [r] = i;
			}
		l++;
		r--;
		}
	}
while (l <= r);
if (left < r)
	SortPalette (left, r);
if (l < right)
	SortPalette (l, right);
}

//------------------------------------------------------------------------------

void CPaletteWnd::CreatePalette ()
{
for (int i = 0; i < 256; i++) {
	m_nSortedPalIdx [i] = i;
	RgbFromIndex (i, m_palColors [i]);
	}
}
//------------------------------------------------------------------------------

void CPaletteWnd::Update ()
{
InvalidateRect (null);
UpdateWindow ();
}

//------------------------------------------------------------------------------

int CPaletteWnd::Create (CWnd *pParentWnd, int nWidth, int nHeight)
{
	CRect	rc;

m_pParentWnd = pParentWnd;
pParentWnd->GetClientRect (rc);
m_nWidth = nWidth;
m_nHeight = nHeight;
if (m_nWidth < 0)
	m_nWidth = rc.Width () / 8;
if (m_nHeight < 0) {
	m_nHeight = rc.Height () / 8;
	if (m_nWidth * m_nHeight > 256)
		m_nHeight = (256 + m_nWidth - 1) / m_nWidth;
	}
return CWnd::Create (null, null, WS_CHILD | WS_VISIBLE, rc, pParentWnd, 0);
}

//------------------------------------------------------------------------------

bool CPaletteWnd::SelectColor (CPoint& point, int& color, PALETTEENTRY *pRGB)
{
	CRect	rcPal;

GetClientRect (rcPal);
//ClientToScreen (rcPal);
// if over palette, redefine foreground color
if (PtInRect (rcPal, point)) {
	int x, y;
//	x = ((point.x - rcPal.left) >> 3)&127;
//	y = ((point.y - rcPal.top) >> 3)&31;
	x = (int) ((double) (point.x - rcPal.left) * ((double) m_nWidth / rcPal.Width ()));
	y = (int) ((double) (point.y - rcPal.top) * ((double) m_nHeight / rcPal.Height ()));
	int c = m_nWidth * y + x;
	if (c > 255)
		return false;
	color = m_nSortedPalIdx [c];
	if (pRGB)
		*pRGB = m_palColors [c];
	//RgbFromIndex (color, pRGB);
	return true;
	}
return false;
}

//------------------------------------------------------------------------------

void CPaletteWnd::SetPalettePixel (int x, int y) 
{
	CRect	rc;

GetClientRect (&rc);
int dx, dy;
for (dy = 0; dy < 8; dy++)
	for (dx = 0; dx < 8; dx++)
		m_pDC->SetPixel ((x << 3) + dx + rc.left, (y << 3) + dy + rc.top, /*PALETTEINDEX*/ (y * m_nWidth + x));
}

//------------------------------------------------------------------------------

void CPaletteWnd::DrawPalette (void) 
{
if (!BeginPaint ())
	return;
CreatePalette ();
//SortPalette (0, 255);
CRect rc;
GetClientRect (&rc);
ubyte *bmPalette = new ubyte [m_nWidth * m_nHeight];
int h, i, c, w, x, y;
for (c = 0, y = m_nHeight - 1; (y >= 0); y--) {
	for (x = 0, h = y * m_nWidth; x < m_nWidth; x++, h++) {
		if (!y)
			y = 0;
		bmPalette [h] = (c < 256) ? m_nSortedPalIdx [c++] : 0;
		}
	}
BITMAPINFO* bmi = paletteManager.BMI ();
bmi->bmiHeader.biWidth = m_nWidth;
bmi->bmiHeader.biHeight = m_nHeight;
bmi->bmiHeader.biBitCount = 8;
bmi->bmiHeader.biClrUsed = 0;
//CPalette *pOldPalette = m_pDC->SelectPalette (paletteManager.Render (), FALSE);
//m_pDC->RealizePalette ();
if (m_nWidth & 1)
	for (i = 0; i < m_nHeight; i++) {
		w = (i == m_nHeight - 1) ? 256 % m_nWidth : m_nWidth;
		StretchDIBits (m_pDC->m_hDC, 0, i * 8, w * 8, 8, 0, 0, w, 1, 
						   (void *) (bmPalette + (m_nHeight - i - 1) * m_nWidth), bmi, 
							DIB_RGB_COLORS, SRCCOPY);
		}
else
	StretchDIBits (m_pDC->m_hDC, 0, 0, m_nWidth * 8, m_nHeight * 8, 0, 0, m_nWidth, m_nHeight, 
					   (void *) bmPalette, bmi, DIB_RGB_COLORS, SRCCOPY);
//m_pDC->SelectPalette (pOldPalette, FALSE);
free (bmPalette);
EndPaint ();
}

//------------------------------------------------------------------------------

bool CPaletteWnd::BeginPaint ()
{
if (!IsWindow (m_hWnd))
	return false;
if (m_pDC)
	return false;
if (!(m_pDC = GetDC ()))
	 return false;
m_pOldPal = m_pDC->SelectPalette (paletteManager.Render (), FALSE);
m_pDC->RealizePalette ();
return true;
}

//------------------------------------------------------------------------------

void CPaletteWnd::EndPaint ()
{
if (m_pDC) {
	if (m_pOldPal) {
		m_pDC->SelectPalette (m_pOldPal, FALSE);
		m_pOldPal = null;
		}
	ReleaseDC (m_pDC);
	m_pDC = null;
	}
Update ();
}

//------------------------------------------------------------------------------
#if 0
void CPaletteWnd::OnLButtonDown (UINT nFlags, CPoint point)
{
m_pParentWnd->SendMessage (WM_LBUTTONDOWN, (WPARAM) nFlags, (LPARAM) point.x + (((LPARAM) point.y) << 16));
}

//------------------------------------------------------------------------------

void CPaletteWnd::OnRButtonDown (UINT nFlags, CPoint point)
{
m_pParentWnd->SendMessage (WM_RBUTTONDOWN, (WPARAM) nFlags, (LPARAM) point.x + (((LPARAM) point.y) << 16));
}

//------------------------------------------------------------------------------

void CPaletteWnd::OnLButtonUp (UINT nFlags, CPoint point)
{
m_pParentWnd->SendMessage (WM_LBUTTONUP, (WPARAM) nFlags, (LPARAM) point.x + (((LPARAM) point.y) << 16));
}

//------------------------------------------------------------------------------

void CPaletteWnd::OnRButtonUp (UINT nFlags, CPoint point)
{
m_pParentWnd->SendMessage (WM_RBUTTONUP, (WPARAM) nFlags, (LPARAM) point.x + (((LPARAM) point.y) << 16));
}
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CTextureEdit::CTextureEdit (int bOverlay, char* pszName, CWnd *pParent)
	: CDialog (IDD_EDITTEXTURE, pParent) 
{
*m_szColors = '\0';
m_pDC = null;
m_pPaintWnd = null;
m_pOldPal = null;
m_lBtnDown =
m_rBtnDown = false;
strcpy_s (m_szName, sizeof (m_szName), pszName);
_strlwr_s (m_szName, sizeof (m_szName));
m_bOverlay = bOverlay;
if (!*m_szDefExt)
	strcpy_s (m_szDefExt, sizeof (m_szDefExt), "bmp");
}

//------------------------------------------------------------------------------

CTextureEdit::~CTextureEdit ()
{
m_texture [0].Release ();
m_texture [1].Release ();
}

//------------------------------------------------------------------------------

BOOL CTextureEdit::OnInitDialog ()
{
if (!textureManager.Available ())
	return FALSE;

CDialog::OnInitDialog ();

	CWnd*	pWnd;
	CRect	rc;

pWnd = GetDlgItem (IDC_TEXEDIT_TEXTURE);
pWnd->GetClientRect (rc);
m_textureWnd.Create (null, null, WS_CHILD | WS_VISIBLE, rc, pWnd, 0);
pWnd = GetDlgItem (IDC_TEXEDIT_PALETTE);
pWnd->GetClientRect (rc);
m_paletteWnd.Create (pWnd, 32, 8);
pWnd = GetDlgItem (IDC_TEXEDIT_LAYERS);
pWnd->GetClientRect (rc);
m_layerWnd.Create (null, null, WS_CHILD | WS_VISIBLE, rc, pWnd, 0);
// set cursor styles for bitmap windows
SetCursor (LoadCursor (AfxGetInstanceHandle (), "PENCIL_CURSOR"));
//  PaletteButton->SetCursor(null, IDC_CROSS);
m_fgColor = 0; // black
m_bgColor = 1; // white
m_lBtnDown  = FALSE;
m_rBtnDown = FALSE;
m_bModified = FALSE;
m_iTexture = m_bOverlay ? current->Side ()->OvlTex (0) : current->Side ()->BaseTex ();
if (m_iTexture >= MAX_TEXTURES_D2)
	m_iTexture = 0;
m_texP = textureManager.Textures (DLE.FileType (), m_iTexture);
if ((m_texP->Buffer () == null) || !m_texP->m_info.bValid) {
	DEBUGMSG (" Texture tool: Invalid texture");
	EndDialog (IDCANCEL);
	}
else if (!m_texture [0].Copy (*m_texP)) {
	DEBUGMSG (" Texture tool: Not enough memory for texture editing");
	EndDialog (IDCANCEL);
	}
m_texture [1].Clear ();
if (!m_texture [1].Allocate (m_texP->Size ()))
	DEBUGMSG (" Texture tool: Not enough memory for undo function");
Backup ();
Refresh ();
m_nWidth = m_texP->Width ();
m_nHeight = m_texP->Height ();
return TRUE;
}

//------------------------------------------------------------------------------

void CTextureEdit::DoDataExchange (CDataExchange *pDX)
{
DDX_Text (pDX, IDC_TEXEDIT_COLORS, m_szColors, sizeof (m_szColors));
}

//------------------------------------------------------------------------------

void CTextureEdit::Backup (void)
{
if (m_texture [1].Buffer ())
	m_texture [1].Copy (m_texture [0]);
}

//------------------------------------------------------------------------------

bool CTextureEdit::PtInRect (CRect& rc, CPoint& pt)
{
return (pt.x >= rc.left) && (pt.x < rc.right) && (pt.y >= rc.top) && (pt.y < rc.bottom);
}

//------------------------------------------------------------------------------

void CTextureEdit::OnButtonDown (UINT nFlags, CPoint point, int& color)
{
	CRect	rcEdit, rcPal;

GetClientRect (&m_textureWnd, rcEdit);
GetClientRect (&m_paletteWnd, rcPal);
if (PtInRect (rcEdit, point)) {
	Backup ();
	ColorPoint (nFlags, point, color);
	}
else if (PtInRect (rcPal, point)) {
	point.x -= rcPal.left;
	point.y -= rcPal.top;
	if (m_paletteWnd.SelectColor (point, color))
		DrawLayers ();
	}
}

//------------------------------------------------------------------------------

void CTextureEdit::OnOK ()
{
if (m_bModified) {
	m_texP->Copy (m_texture [0]);
	m_texP->m_info.bCustom = true;
	}
CDialog::OnOK ();
}

//------------------------------------------------------------------------------

void CTextureEdit::OnLButtonDown (UINT nFlags, CPoint point)
{
m_lBtnDown = TRUE;
OnButtonDown (nFlags, point, m_fgColor);
}

//------------------------------------------------------------------------------

void CTextureEdit::OnRButtonDown (UINT nFlags, CPoint point)
{
m_rBtnDown = TRUE;
OnButtonDown (nFlags, point, m_bgColor);
}

//------------------------------------------------------------------------------

void CTextureEdit::OnLButtonUp (UINT nFlags, CPoint point)
{
m_lBtnDown = FALSE;
}

//------------------------------------------------------------------------------

void CTextureEdit::OnRButtonUp (UINT nFlags, CPoint point)
{
m_rBtnDown = FALSE;
}

//------------------------------------------------------------------------------

void CTextureEdit::OnMouseMove (UINT nFlags, CPoint point)
{
if (m_lBtnDown)
	ColorPoint (nFlags, point, m_fgColor);
else if (m_rBtnDown)
	ColorPoint (nFlags, point, m_bgColor);
}


//------------------------------------------------------------------------------

bool CTextureEdit::BeginPaint (CWnd *pWnd)
{
if (m_pDC)
	return false;
if (!(m_pDC = pWnd->GetDC ()))
	 return false;
m_pPaintWnd = pWnd;
m_pOldPal = m_pDC->SelectPalette (paletteManager.Render (), FALSE);
m_pDC->RealizePalette ();
return true;
}

//------------------------------------------------------------------------------

void CTextureEdit::EndPaint ()
{
if (m_pPaintWnd) {
	if (m_pDC) {
		if (m_pOldPal) {
			m_pDC->SelectPalette (m_pOldPal, FALSE);
			m_pOldPal = null;
			}
		m_pPaintWnd->ReleaseDC (m_pDC);
		m_pDC = null;
		}
	Update (m_pPaintWnd);
	m_pPaintWnd = null;
	}
}

//------------------------------------------------------------------------------

void CTextureEdit::GetClientRect (CWnd *pWnd, CRect& rc)
{
	CRect	rcc;
	int	dx, dy;

pWnd->GetClientRect (&rcc);
pWnd->GetWindowRect (rc);
dx = rc.Width () - rcc.Width ();
dy = rc.Height () - rcc.Height ();
ScreenToClient (rc);
rc.DeflateRect (dx / 2, dy / 2);
}

//------------------------------------------------------------------------------
// CTextureEdit - ColorPoint
//
// Action - Uses coordinates to determine which pixel mouse cursor is
// over.  If it is over the palette, the palette box will be updated with
// the new color.  If it is over the texture, the texture will be updated
// with the color.
// If control key is held down, color is defined by bitmap instead.
//------------------------------------------------------------------------------

void CTextureEdit::ColorPoint (UINT nFlags, CPoint& point, int& color) 
{
	CRect		rcEdit;

GetClientRect (&m_textureWnd, rcEdit);

if (m_texture [0].m_info.nFormat) {
	m_lBtnDown = m_rBtnDown = false;
	ErrorMsg ("Cannot edit TGA images.");
	}
else if (PtInRect (rcEdit, point)) {
	int x, y;
	m_bModified = TRUE;  // mark this as m_bModified
//	x = ((point.x - rcEdit.left) >> 2) & 63;
//	y = ((point.y - rcEdit.top) >> 2) & 63;
	x = (int) ((double) (point.x - rcEdit.left) * (double (m_nWidth) / rcEdit.Width ()));
	y = (int) ((double) (point.y - rcEdit.top) * (double (m_nHeight) / rcEdit.Height ()));
	if (nFlags & MK_CONTROL) {
		color = m_texture [0][m_nWidth * (m_nHeight - 1 - y) + x].ColorRef ();
		DrawLayers ();
		}
	else if (BeginPaint (&m_textureWnd)) {
		m_texture [0][m_nWidth * (m_nHeight - 1 - y) + x] = *paletteManager.Current (color);
		SetTexturePixel (x, y);
		EndPaint ();
		}
	}
}

//------------------------------------------------------------------------------

void CTextureEdit::OnPaint () //EvDrawItem(UINT, DRAWITEMSTRUCT &) 
{
CDialog::OnPaint ();
Refresh ();
}

//------------------------------------------------------------------------------

inline int Sqr (int i)
{
return i * i;
}

//------------------------------------------------------------------------------

inline int ColorDelta (RGBQUAD *bmPal, PALETTEENTRY *sysPal, int j)
{
sysPal += j;
return 
	Sqr (int (bmPal->rgbBlue) - int (sysPal->peBlue)) + 
	Sqr (int (bmPal->rgbGreen) - int (sysPal->peGreen)) + 
	Sqr (int (bmPal->rgbRed) - int (sysPal->peRed));
}

//------------------------------------------------------------------------------

bool CTextureEdit::LoadBitmap (CFileManager& fp)
{
	RGBQUAD* palette = null;
	PALETTEENTRY* sysPal = null;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bmih;
	ubyte palIndexTable [256];
	bool bFuncRes = false;
	uint x, y, width, paletteSize;

palette = new RGBQUAD [256];
if (palette == null) {
	ErrorMsg ("Not enough memory for palette.");
	goto errorExit;
	}

sysPal = new PALETTEENTRY [256];
if (sysPal == null) {
	ErrorMsg ("Not enough memory for palette.");
	goto errorExit;
	}

// read the header information
fp.Read (&bmfh, sizeof (bmfh), 1);
fp.Read (&bmih, sizeof (bmih), 1);

// handle exceptions
if (bmih.biClrUsed == 0)  
	bmih.biClrUsed = 256;
if (bmih.biHeight < 0) 
	bmih.biHeight = -bmih.biHeight;

// make sure it is a bitmap file
if (bmfh.bfType != 'B' + (((ushort) 'M') << 8) ) {
	ErrorMsg ("This is not a bitmap file.");
	goto errorExit;
	}

// make sure it is a 256 or 16 color bitmap
if (bmih.biBitCount != 8 && bmih.biBitCount != 4) {
	ErrorMsg ("DLE only reads 16 or 256 color bitmap files.\n\n"
	"Hint: Load this image into a paint program\n"
	"then save it as a 16 or 256 color *.bmp file.");
	goto errorExit;
	}

// make sure the data is not compressed
if (bmih.biCompression != BI_RGB) {
	ErrorMsg ("Cannot read compressed bitmap files.\n\n"
	"Hint: Try to load this image into a paint program\n"
	"then save it as a 256 color *.bmp file with the\n"
	"compression option off.");
	goto errorExit;
	}

// read palette
paletteSize = min ((int) bmih.biClrUsed, 256);
if (paletteSize == 0)
	paletteSize = 1 << bmih.biBitCount;
fp.Read (palette, sizeof (RGBQUAD), paletteSize);

// read the logical palette entries
paletteManager.Render ()->GetPaletteEntries (0, 256, sysPal);

// check color palette
int i;
for (i = 0; i < int (paletteSize); i++) {
	palIndexTable [i] = i;
	if ((palette [i].rgbRed != sysPal [i].peRed) ||
		 (palette [i].rgbGreen != sysPal [i].peGreen) ||
		 (palette [i].rgbBlue != sysPal [i].peBlue)) {
		break;
		}		
	}

if (i != int (paletteSize)) {
	if (!DLE.ExpertMode ())
		ErrorMsg ("The palette of this bitmap file is not exactly the\n"
					 "the same as the Descent palette. Therefore, some color\n"
					 "changes may occur.\n\n"
					 "Hint: If you want the palettes to match, then save one of\n"
					 "the Descent textures to a file and use it as a starting point.\n"
					 "If you plan to use transparencies, then you may want to start\n"
					 "with the texture called 'empty'.");
		for (i = 0; i < int (paletteSize); i++) {
		uint closestIndex = i;
		if ((palette [i].rgbRed != sysPal [i].peRed) ||
			 (palette [i].rgbGreen != sysPal [i].peGreen) ||
			 (palette [i].rgbBlue != sysPal [i].peBlue)) {
			uint closestDelta = 0x7fffffff;
			for (int j = 0; j < 255; j++) {
				uint delta = ColorDelta (palette + i, sysPal, j);
				if (delta < closestDelta) {
					closestIndex = j;
					if (!(closestDelta = delta))
						break;
					}
				}
			palette [i].rgbRed = sysPal [closestIndex].peRed;
			palette [i].rgbGreen = sysPal [closestIndex].peGreen;
			palette [i].rgbBlue = sysPal [closestIndex].peBlue;
			}
		palIndexTable [i] = closestIndex;
		}
	}

int x0, x1, y0, y1;
// if size is not 64 x 64, ask if they want to "size to fit"
if ((bmih.biWidth != m_nWidth) || (bmih.biHeight != m_nHeight)) {
	sprintf_s (message, sizeof (message), 
				  "The bitmap being loaded is a %d x %d image.\n"
				  "Do you want the image to be sized to fit\n"
				  "the current %d x %d texture size?\n\n"
				  "(press no to see another option)", 
				 (int) bmih.biWidth, (int) bmih.biHeight, 
				 (int) m_nWidth, (int) m_nHeight);
	switch (Query2Msg (message, MB_YESNOCANCEL)) {
		case IDYES:
			Backup();
			x0 = 0;
			y0 = 0;
			x1 = (int) bmih.biWidth + 1;
			y1 = (int) bmih.biHeight + 1;
			break;

		case IDNO:
			Backup();
			if (Query2Msg ("Would you like to center/tile the image?", MB_YESNO) == IDYES) {
				x0 = (int)(bmih.biWidth - m_nWidth) / 2;
				y0 = (int)(bmih.biHeight - m_nHeight) / 2;
				x1 = x0 + m_nWidth;
				y1 = y0 + m_nHeight;
				}
			else if ((bmih.biWidth > 1024) || (bmih.biHeight > 1024)) 
				goto errorExit;
			else {
				x0 = 0;
				y0 = 0;
				x1 = m_nWidth = (ushort) bmih.biWidth;
				y1 = m_nHeight = (ushort) bmih.biHeight;
				}
			break;

			default:
				goto errorExit;
		}
	}
else {
	x0 = 0;
	y0 = 0;
	x1 = m_nWidth;
	y1 = m_nHeight;
	}

// save bitmap for undo command
m_nSize = m_nWidth * m_nHeight;

// read data into bitmap
m_bModified = TRUE;  // mark this as m_bModified
width = (((int)(bmih.biWidth * bmih.biBitCount + 31) >> 3)) & ~3;
double mx, my;
mx = (x1 - x0) / (double) m_nWidth;
my = (y1 - y0) / (double) m_nHeight;
uint z = 0;
for (y = 0; y < m_nHeight; y++) {
	for (x = 0; x < m_nWidth; x++, z++) {
		int u = (int) (mx * x + x0);
		int v = (int) (my * y + y0);
		u %= (int) bmih.biWidth;         //  -width to width
		if (u < 0) 
			u += (int) bmih.biWidth;		//       0 to width
		v %= (int) bmih.biHeight;        // -height to height
		if (v < 0) 
			v += (int) bmih.biHeight;		//       0 to height
	
		ubyte palIndex, i;

		if (bmih.biBitCount == 4) {
			long offset = (int) v * (int) width + (int) u / 2;
			fp.Seek ((int) bmfh.bfOffBits + offset, SEEK_SET);
			fp.Read (&palIndex, 1, 1);
			if (!(u & 1))
			palIndex >>= 4;
			palIndex &= 0x0f;
			i = palIndexTable [palIndex];
			m_texture [0][z] = i;
			}
		else {
			fp.Seek ((int) bmfh.bfOffBits + (int) v *(int) width + (int) u, SEEK_SET);
			fp.Read (&palIndex, 1, 1);
			m_texture [0][z] = palette [palIndex];
			}
		if (i >= 254)
			m_texture [0][z].a = 0;
		}
	}

m_texture [0].m_info.nFormat = 0;
bFuncRes = true;

errorExit:

if (palette) 
	delete palette;
if (sysPal) 
	delete sysPal;
return bFuncRes;
}

//------------------------------------------------------------------------------

char	CTextureEdit::m_szDefExt [4] = "bmp";

void CTextureEdit::OnLoad () 
{
  OPENFILENAME ofn;
  char szFile[80] = "\0";
  CFileManager fp;
  bool bFuncRes;

  sprintf_s (szFile, sizeof (szFile), "*.%s", m_szDefExt);
  memset (&ofn, 0, sizeof (OPENFILENAME));
  ofn.lStructSize = sizeof (OPENFILENAME);
  ofn.hwndOwner = m_hWnd;
  ofn.lpstrFilter = "all files\0*.bmp;*.tga\0bitmap files\0*.bmp\0TGA files\0*.tga\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = szFile;
  ofn.lpstrDefExt = m_szDefExt;
  ofn.nMaxFile = sizeof (szFile);
  ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

if (GetOpenFileName (&ofn)) {
	if (strchr (ofn.lpstrFile, '.'))
		strncpy_s (m_szDefExt, sizeof (m_szDefExt), strchr (ofn.lpstrFile, '.') + 1, 3);
	if (!fp.Open (ofn.lpstrFile, "rb")) {
		ErrorMsg ("Could not open texture file.");
		goto errorExit;
		}
	Backup ();
	_strlwr_s (m_szDefExt, sizeof (m_szDefExt));
	if (!strcmp (m_szDefExt, "bmp"))
		bFuncRes = LoadBitmap (fp);
	else 
		bFuncRes = m_texture [0].LoadTGA (fp);
	if (bFuncRes) {
		Refresh ();
		m_nWidth = m_texture [0].m_info.width;
		m_nHeight = m_texture [0].m_info.height;
		m_nSize = m_texture [0].m_info.width * m_texture [0].m_info.height;		
		m_bModified = TRUE;
		}
	else
		OnUndo ();
	}

errorExit:

fp.Close ();
}

//------------------------------------------------------------------------------

void CTextureEdit::SaveBitmap (CFileManager& fp)
{
BITMAPFILEHEADER bmfh;

bmfh.bfType = 'B' + ('M' << 8);
bmfh.bfSize = sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER) + 256 * 4 + m_nSize;
bmfh.bfReserved1 = 0;
bmfh.bfReserved2 = 0;
bmfh.bfOffBits   = sizeof (BITMAPFILEHEADER) + sizeof (BITMAPINFOHEADER) + 256 * 4;

// define the bitmap header
BITMAPINFO* bmi = paletteManager.BMI ();
bmi->bmiHeader.biWidth = m_nWidth;
bmi->bmiHeader.biHeight = m_nHeight;

// write the headers
fp.Write (&bmfh, sizeof (BITMAPFILEHEADER), 1);
fp.Write (&bmi->bmiHeader, sizeof (BITMAPINFOHEADER), 1);

// write palette
fp.Write (bmi->bmiColors, sizeof (RGBQUAD), 256);

// save bitmap data
textureManager.WriteCustomTexture (fp, &m_texture [0]);
}

//------------------------------------------------------------------------------

void CTextureEdit::SaveTGA (CFileManager& fp)
{
	tTgaHeader	h;

memset (&h, 0, sizeof (h));
h.imageType = 2;
h.width = m_texture [0].Width ();
h.height = m_texture [0].Height ();
h.bits = 32;
fp.Write (&h, sizeof (h), 1);
int j = h.width * h.height;
for (int i = h.height; i; i--) {
	j -= h.width;
	fp.Write (&m_texture [0][j], sizeof (CBGRA), h.width);
	}
}

//------------------------------------------------------------------------------

void CTextureEdit::OnSave ()
{
OPENFILENAME ofn;
char szFile [256] = "\0";
CFileManager fp;

memset(&ofn, 0, sizeof (OPENFILENAME));

ofn.lStructSize = sizeof (OPENFILENAME);
ofn.hwndOwner = m_hWnd;
ofn.lpstrFilter = m_texture [0].m_info.nFormat ? "Truevision Targa\0*.tga\0" : "256 color Bitmap Files\0*.bmp\0";
ofn.nFilterIndex = 1;
ofn.lpstrFile= szFile;
ofn.lpstrDefExt = m_texture [0].m_info.nFormat ? "tga" : "bmp";
sprintf (szFile, "%s.%s", m_szName, ofn.lpstrDefExt);
ofn.nMaxFile = sizeof (szFile);
ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
if (GetSaveFileName (&ofn)) {
	_strlwr_s (ofn.lpstrFile, sizeof (szFile));
	if (!fp.Open (ofn.lpstrFile, "wb")) {
		ErrorMsg ("Could not create texture file.");
		return;
		}
	if (m_texture [0].m_info.nFormat)
		SaveTGA (fp);
	else
		SaveBitmap (fp);
	fp.Close ();
	}
}

//------------------------------------------------------------------------------

void CTextureEdit::OnUndo ()
{
if (m_texture [1].Buffer ()) {
	m_texture [0].Copy (m_texture [1]);
	Refresh ();
	}
}

void CTextureEdit::Update (CWnd *pWnd) 
{
pWnd->InvalidateRect (null);
pWnd->UpdateWindow ();
}

//------------------------------------------------------------------------------

void CTextureEdit::OnDefault (void) 
{
if (QueryMsg("Are you sure you want to restore this texture\n"
				 "back to its original texture\n") == IDYES) {
	Backup ();
	m_texP->m_info.bCustom = m_bModified = false;
	//m_texP->Load (m_iTexture);
	if (!m_texP->Reload ())
		m_texture [0].Copy (*m_texP);
	Refresh ();
	}
}

//------------------------------------------------------------------------------

void CTextureEdit::DrawTexture (void) 
{
if (!BeginPaint (&m_textureWnd))
	return;
m_pDC->SetStretchBltMode (STRETCH_DELETESCANS);
BITMAPINFO* bmi = paletteManager.BMI ();
bmi->bmiHeader.biWidth =
bmi->bmiHeader.biHeight = m_texture [0].Width ();
bmi->bmiHeader.biBitCount = 32;
//bmi->bmiHeader.biSizeImage = m_texture [0].BufSize ();
bmi->bmiHeader.biClrUsed = 0;
CRect	rc;
m_textureWnd.GetClientRect (&rc);
StretchDIBits (m_pDC->m_hDC, 0, 0, rc.right, rc.bottom, 0, 0, m_texture [0].Width (), m_texture [0].Width (),
					(void *) m_texture [0].Buffer (), bmi, DIB_RGB_COLORS, SRCCOPY);
EndPaint ();
}

//------------------------------------------------------------------------------

void CTextureEdit::DrawPalette (void) 
{
m_paletteWnd.DrawPalette ();
}

//------------------------------------------------------------------------------

void CTextureEdit::DrawLayers () 
{
if (!BeginPaint (&m_layerWnd))
	return;

CRect		rc;
m_layerWnd.GetClientRect (&rc);
rc.DeflateRect (10, 10);
rc.right -= 10;
rc.bottom -= 10;
m_pDC->FillSolidRect (&rc, PALETTEINDEX (m_bgColor));
rc.OffsetRect (10, 10);
m_pDC->FillSolidRect (&rc, PALETTEINDEX (m_fgColor));
EndPaint ();

// set message
char fg_color[30];
char bg_color[30];
switch(m_fgColor) {
	case 255: 
		strcpy_s (fg_color, sizeof (fg_color), "transparent"); 
		break;
	case 254: 
		strcpy_s (fg_color, sizeof (fg_color), "see thru"); 
		break;
	default : 
		sprintf_s (fg_color, sizeof (fg_color), "color %d", m_fgColor); 
		break;
	}
switch(m_bgColor) {
	case 255: 
		strcpy_s (bg_color, sizeof (bg_color), "transparent"); 
		break;
	case 254: 
		strcpy_s (bg_color, sizeof (bg_color), "see thru"); 
		break;
	default : 
		sprintf_s (bg_color, sizeof (bg_color), "color %d", m_bgColor); 
		break;
	}
sprintf_s (m_szColors, sizeof (m_szColors), "foreground = %s, background = %s.", fg_color, bg_color);
UpdateData (FALSE);
}

//------------------------------------------------------------------------------

void CTextureEdit::Refresh (void) 
{
DrawTexture ();
DrawPalette ();
DrawLayers ();
}

//------------------------------------------------------------------------------

void CTextureEdit::SetTexturePixel (int x, int y) 
{
	CRect		rc;
	int		cx, cy;
	double	xs, ys;
#ifdef _DEBUG
	CBGR&		rgb = m_texture [0][(63 - y) * 64 + x];
	int		color = rgb.ColorRef ();
#else
	int		color = m_texture [0][(63 - y) * 64 + x].ColorRef ();
#endif

m_textureWnd.GetClientRect (&rc);
cx = rc.Width ();
cy = rc.Height ();
xs = (double) cx / 64.0;
ys = (double) cy / 64.0;
x = rc.left + (int) ((double) x * xs);
y = rc.top + (int) ((double) y * ys);
int dx, dy;
xs /= 4.0;
ys /= 4.0;
for (dy = 0; dy < 4; dy++)
	for (dx = 0; dx < 4; dx++)
		m_pDC->SetPixel (x + (int) ((double) dx * xs), y + (int) ((double) dy * ys), color);
}

//------------------------------------------------------------------------------

void CTextureEdit::SetPalettePixel (int x, int y) 
{
	CRect	rc;

m_paletteWnd.GetClientRect (&rc);
int dx, dy;
for (dy = 0; dy < 8; dy++)
	for (dx = 0; dx < 8; dx++)
		m_pDC->SetPixel ((x << 3) + dx + rc.left, (y << 3) + dy + rc.top, PALETTEINDEX (y * 32 + x));
}

//------------------------------------------------------------------------------
