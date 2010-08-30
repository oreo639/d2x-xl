// dlcView.cpp : implementation of the CMineView class
//

#include "stdafx.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"

#include "palette.h"
#include "texturemanager.h"
#include "global.h"
#include "render.h"
#include "cfile.h"
#include "light.h"
#include "toolview.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TEXTURE_SCALE	2

#define	SETBIT(_b,_i)	((_b) [(_i) >> 3] |= (1 << ((_i) & 7)))
#define	BITSET(_b,_i)	((_b) [(_i) >> 3] & (1 << ((_i) & 7)))

/////////////////////////////////////////////////////////////////////////////
// CMineView

IMPLEMENT_DYNCREATE(CTextureView, CWnd)
BEGIN_MESSAGE_MAP(CTextureView, CWnd)
//	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()



CTextureView::CTextureView () 
	: CWnd ()
{
m_pTextures = null;
m_nTextures [0] = 0;
m_nTextures [1] = 0;
m_penCyan = new CPen (PS_SOLID, 1, RGB (0,255,255));
int scale = TEXTURE_SCALE;
m_iconSize.cx = 64 / scale;
m_iconSize.cy = 64 / scale;
m_iconSpace.cx = m_iconSize.cx + 6;
m_iconSpace.cy = m_iconSize.cy + 6;
m_viewFlags = 0;
TextureFilter () = GetPrivateProfileInt ("DLE-XP", "TextureFilter", 0xFFFFFFFF, INIFILE);
m_bShowAll = TRUE;
m_bDelayRefresh = false;
};

//------------------------------------------------------------------------

CTextureView::~CTextureView ()
{
if (m_pTextures)
	free (m_pTextures);
if (m_penCyan)
	delete m_penCyan;
};

//------------------------------------------------------------------------

void CTextureView::Refresh (bool bRepaint) 
{
if (!theMine) return;
if (bRepaint) {
	InvalidateRect (null, TRUE);
	UpdateWindow ();
	}
if (!m_bDelayRefresh) {
	DLE.MineView ()->Refresh (false);
	DLE.ToolView ()->Refresh ();
	}
}

//------------------------------------------------------------------------

void CTextureView::Reset () 
{
CHECKMINE;
//textureManager.Release ();
Refresh ();
}

//------------------------------------------------------------------------

short CTextureView::TexFilterIndex (short nTxt)
{
	short	m, l = 0, r = TEX_FILTER_SIZE - 1;

do {
	m = (l + r) / 2;
	if (nTxt < TEXTURE_FILTERS [m].iTexture.nMin)
		r = m - 1;
	else if (nTxt > TEXTURE_FILTERS [m].iTexture.nMax)
		l = m + 1;
	else
		return m;
	}
while (l <= r);
return -1;
}

//------------------------------------------------------------------------

uint CTextureView::TextureFilter (short nTxt)
{
	short	m = TexFilterIndex (nTxt);

return (m < 0) ? 0 : TEXTURE_FILTERS [m].nFilter;
}

//------------------------------------------------------------------------

int CTextureView::QCmpTexFilters (int nTxt, int mTxt, uint mf, uint mf2)
{
	short	n = TexFilterIndex (nTxt);
	uint	nf = TEXTURE_FILTERS [n].nFilter,
			nf2 = TEXTURE_FILTERS [n].n2ndFilter;

//CBRK (((nf == TXT_DOOR) && (mf == TXT_SAND)) || ((mf == TXT_DOOR) && (nf == TXT_SAND)));
if (nf < mf)
	return -1;
else if (nf > mf)
	return 1;
else if (nf2 < mf2)
	return -1;
else if (nf2 > mf2)
	return 1;
else
	return (nTxt < mTxt) ? -1 : (nTxt > mTxt) ? 1 : 0;
}

//------------------------------------------------------------------------

void CTextureView::QSortTexMap (short left, short right)
{
	short		mTxt = m_mapViewToTex [(left + right) / 2];
	short		m = TexFilterIndex (mTxt);
	uint		mf, mf2;
	short		h, l = left, r = right;

mf = TEXTURE_FILTERS [m].nFilter;
mf2 = TEXTURE_FILTERS [m].n2ndFilter;
do {
	while (QCmpTexFilters (m_mapViewToTex [l], mTxt, mf, mf2) < 0)
		l++;
	while (QCmpTexFilters (m_mapViewToTex [r], mTxt, mf, mf2) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			h = m_mapViewToTex [l];
			m_mapViewToTex [l] = m_mapViewToTex [r];
			m_mapViewToTex [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortTexMap (l, right);
if (left < r)
	QSortTexMap (left, r);
}

//------------------------------------------------------------------------

void CTextureView::CreateTexMap (void)
{
QSortTexMap (0, m_nTextures [1] - 1);
for (int i = 0; i < m_nTextures [1]; i++)
	m_mapTexToView [m_mapViewToTex [i]] = i;
}

//------------------------------------------------------------------------
// CTextureView::EvVScroll()
//
// Action - Receives scroll movements messages and updates scroll position
//          Also calls refreshdata() to update the bitmap.
//------------------------------------------------------------------------

void CTextureView::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
UINT nPos = GetScrollPos (SB_VERT);
CRect rect;
GetClientRect(rect);

m_bDelayRefresh = true;
switch (scrollCode) {
	case SB_LINEUP:
		nPos--;
		break;
	case SB_LINEDOWN:
		nPos++;
		break;
	case SB_PAGEUP:
		nPos -= m_viewSpace.cy;
		break;
	case SB_PAGEDOWN:
		nPos += m_viewSpace.cy;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nPos = thumbPos;
		break;
	case SB_ENDSCROLL:
		m_bDelayRefresh = false;
		Refresh (false);
		return;
}
SetScrollPos (SB_VERT, nPos, TRUE);
Refresh ();
}

//------------------------------------------------------------------------
// CTextureView::EvLButtonDown()
//
// Action - Sets texture 1 of current side to texture under mouse pointer
//
// Notes - If shift is held down and object is poly type, then object's
//         texture is changed instead of cube's texture.
//------------------------------------------------------------------------

void CTextureView::OnLButtonDown(UINT nFlags, CPoint point)
{
if (!theMine) return;

	short nBaseTex;

if (PickTexture (point, nBaseTex))
	return;
if (nFlags & MK_SHIFT) {
	CGameObject *objP = theMine->Objects (theMine->Current ()->nObject);
   if (objP->m_info.renderType != RT_POLYOBJ) 
		return;
	objP->rType.polyModelInfo.tmap_override = nBaseTex;
  } 
else if (nFlags & MK_CONTROL) {
	DLE.ToolView ()->TriggerTool ()->SetTexture (nBaseTex, -1);
	DLE.ToolView ()->TriggerTool ()->Refresh ();
	}
else {
	theMine->SetTexture (-1, -1, nBaseTex, -1);
	Refresh ();
	}
DLE.SetModified (TRUE);
}

//------------------------------------------------------------------------

void CTextureView::OnRButtonDown(UINT nFlags, CPoint point)
{
if (!theMine) return;

	CSide *sideP = theMine->CurrSide ();
	short	nBaseTex;

if (PickTexture (point, nBaseTex))
	return;
if (nFlags & MK_CONTROL) {
	DLE.ToolView ()->TriggerTool ()->SetTexture (-1, nBaseTex);
	DLE.ToolView ()->TriggerTool ()->Refresh ();
	}
else {
	theMine->SetTexture (-1, -1, -1, nBaseTex);
	Refresh ();
	}
DLE.SetModified (TRUE);
}

//------------------------------------------------------------------------
// CTextureView::PickTexture()
//
// Action - calculates position of textures and matches it up with mouse.
// If a match is found, nBaseTex is defined and 0 is returned
//------------------------------------------------------------------------

int CTextureView::PickTexture(CPoint &point,short &nBaseTex) 
{
if (!theMine) 
	return 1;

//if (!m_pTextures)
//	return 0;

  nBaseTex = 0;  // set default in case value is used when function fails

  CRect rect;
  GetClientRect(rect);

//if (m_nRows [m_bShowAll] <= m_viewSpace.cy)
//	return 1;

UINT nPos = GetScrollPos (SB_VERT);
int nOffset = nPos * m_viewSpace.cx;
byte pFilter [1 + MAX_D2_TEXTURES / 8];

FilterTextures (pFilter, m_bShowAll);
int x = 0;
int y = 0;

x = point.x / m_iconSpace.cx;
y = point.y / m_iconSpace.cy;
int h = nOffset + y * m_viewSpace.cx + x + 1;
int i;
for (i = 0; i < m_nTextures [1]; i++) {
	if (BITSET (pFilter, i)) //pFilter [i / 8] & (1 << (i & 7)))
		if (!--h) {
			nBaseTex = m_mapViewToTex [i]; //m_pTextures [i];
			return 0; // return success
			}
	}
return 1; // return failure
}

//------------------------------------------------------------------------
// TextureIndex()
//
// looks up texture index number for nBaseTex
// returns 0 to m_nTextures-1 on success
// returns -1 on failure
//------------------------------------------------------------------------

int CTextureView::TextureIndex(short nBaseTex) 
{
return ((nBaseTex < 0) || (nBaseTex >= sizeof (m_mapTexToView) / sizeof (m_mapTexToView [0]))) ? 0 : m_mapTexToView [nBaseTex];
}

//------------------------------------------------------------------------
// FilterTexture()
//
// Determines which textures to display based on which have been used
//------------------------------------------------------------------------

void CTextureView::FilterTextures (byte *pFilter, BOOL bShowAll) 
{
if (bShowAll) {
	if (m_nTxtFilter == 0xFFFFFFFF)
		memset (pFilter, 0xFF, (MAX_D2_TEXTURES + 7) / 8);
	else {
		memset (pFilter, 0, (MAX_D2_TEXTURES + 7) / 8);
		m_nTextures [0] = 0;
		int i, f = m_nTxtFilter & ~TXT_MOVE;
		for (i = 0; i < m_nTextures [1]; i++) {
			int t = m_mapViewToTex [i];
			int j = TexFilterIndex (t);
			if ((TEXTURE_FILTERS [j].nFilter | TEXTURE_FILTERS [j].n2ndFilter) & f) {
				SETBIT (pFilter, i);
				m_nTextures [0]++;
				}
			}
		}
	}
else {
	ushort nSegment,nSide;
	CSegment *segP;

	memset (pFilter, 0, (MAX_D2_TEXTURES + 7) / 8);
	m_nTextures [0] = 0;
	for (nSegment = 0, segP = theMine->Segments (0); nSegment < theMine->SegCount (); nSegment++, segP++)
      for (nSide = 0;nSide < 6; nSide++) {
			ushort nWall = segP->m_sides[nSide].m_info.nWall;
			if ((segP->Child (nSide) == -1) ||
				 (nWall < theMine->GameInfo ().walls.count && 
				  theMine->Walls (nWall)->m_info.type != WALL_OPEN)) {
				int t = segP->m_sides [nSide].m_info.nBaseTex;
				int i = TextureIndex (t);
				int j = TexFilterIndex (t);
				if ((i >= 0) && !BITSET (pFilter, i) && 
					 ((TEXTURE_FILTERS [j].nFilter | TEXTURE_FILTERS [j].n2ndFilter) & m_nTxtFilter)) {
					SETBIT (pFilter, i);
					m_nTextures [0]++;
					}
//					pFilter[t/8] |= (1<<(t&7));
				t = segP->m_sides [nSide].m_info.nOvlTex & 0x3fff;
				i = TextureIndex (t);
				j = TexFilterIndex (t);
				if ((t > 0) && !BITSET (pFilter, i)) {
					SETBIT (pFilter, i);
//					pFilter[t/8] |= (1<<(t&7));
					m_nTextures [0]++;
					}
				}
			}
	}
}

								/*---------------------------*/

void CTextureView::RecalcLayout () 
{
CHECKMINE;

	CRect rect;
	GetClientRect(rect);

m_viewSpace.cx = rect.Width () / m_iconSpace.cx;
m_viewSpace.cy = rect.Height () / m_iconSpace.cy;

int nOffset = 0;
m_bShowAll = ((m_viewFlags & eViewMineUsedTextures) == 0);

if (!(m_viewSpace.cx && m_viewSpace.cy))
	return;

byte pFilter [(MAX_D2_TEXTURES + 7) / 8];
FilterTextures (pFilter, m_bShowAll);
m_nRows [ShowAll ()] = (m_nTextures [ShowAll ()] + m_viewSpace.cx - 1) / m_viewSpace.cx;

// read scroll position to get offset for bitmap tiles
SetScrollRange (SB_VERT, 0, m_nRows [m_bShowAll] - m_viewSpace.cy, TRUE);
// if there is enough lines to show all textures, then hide the scroll bar
if (m_nRows [ShowAll ()] <= m_viewSpace.cy) {
	ShowScrollBar (SB_VERT,FALSE);
	nOffset = 0;
	}
else {
// otherwise, calculate the number of rows to skip
	ShowScrollBar (SB_VERT,TRUE);
	UINT nPos = GetScrollPos (SB_VERT);
	int i, j = nPos * m_viewSpace.cx; 
	for (i = 0, nOffset = 0; (i < m_nTextures [1]); i++)
		if (ShowAll () || BITSET (pFilter, i)) {
			nOffset++;
			if (--j < 0)
				break;
			}
	}
SetScrollPos (SB_VERT, nOffset / m_viewSpace.cx, TRUE);
// figure out position of current texture
int nBaseTex = theMine->CurrSide ()->m_info.nBaseTex;
int nOvlTex = theMine->CurrSide ()->m_info.nOvlTex & 0x3fff; // strip rotation info
CTexture tex (textureManager.bmBuf);

CDC *pDC = GetDC();
if (!pDC) {
	ErrorMsg ("No device context for texture picker available");
	return;
	}
BITMAPINFO* bmi = MakeBitmap();

   // realize pallette for 256 color displays
CPalette *oldPalette = pDC->SelectPalette(theMine->m_currentPalette, FALSE);
pDC->RealizePalette();
pDC->SetStretchBltMode(STRETCH_DELETESCANS);
int x = 0;
int y = 0;
for (int i = 0; i < m_nTextures [1]; i++) {
	if (!ShowAll ()) {
		if (!BITSET (pFilter, i))
			continue;
			}
	if (nOffset &&	--nOffset)
		continue;
	if (!textureManager.Define (m_mapViewToTex [i], 0, &tex, 0, 0)) {
		bmi->bmiHeader.biWidth = tex.m_info.width;
		bmi->bmiHeader.biHeight = tex.m_info.width;
		StretchDIBits (*pDC, 3 + x * m_iconSpace.cx, 3 + y * m_iconSpace.cy, 
							m_iconSize.cx, m_iconSize.cy, 0, 0, tex.m_info.width, tex.m_info.width, 
							(void *)tex.m_info.bmDataP, bmi, DIB_RGB_COLORS, SRCCOPY);
		}
// pick color for box drawn around texture
	if (m_mapViewToTex [i] == nBaseTex)
		pDC->SelectObject (GetStockObject (WHITE_PEN));
	else if (i && (m_mapViewToTex [i] == nOvlTex)) // note: 0 means no texture
		pDC->SelectObject (m_penCyan);
	else
		pDC->SelectObject (GetStockObject (BLACK_PEN));
	pDC->MoveTo (1 + x * m_iconSpace.cx, 1 + y * m_iconSpace.cy);
	pDC->LineTo (4 + x * m_iconSpace.cx+m_iconSize.cx,1 + y * m_iconSpace.cy);
	pDC->LineTo (4 + x * m_iconSpace.cx+m_iconSize.cx,4 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (1 + x * m_iconSpace.cx, 4 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (1 + x * m_iconSpace.cx, 1 + y * m_iconSpace.cy);

// draw black boxes around and inside this box
//	  SelectObject(hdc, GetStockObject(BLACK_PEN));
	pDC->MoveTo (0 + x * m_iconSpace.cx, 0 + y * m_iconSpace.cy);
	pDC->LineTo (5 + x * m_iconSpace.cx+m_iconSize.cx,0 + y * m_iconSpace.cy);
	pDC->LineTo (5 + x * m_iconSpace.cx+m_iconSize.cx,5 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (0 + x * m_iconSpace.cx, 5 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (0 + x * m_iconSpace.cx, 0 + y * m_iconSpace.cy);
	pDC->MoveTo (2 + x * m_iconSpace.cx, 2 + y * m_iconSpace.cy);
	pDC->LineTo (3 + x * m_iconSpace.cx+m_iconSize.cx,2 + y * m_iconSpace.cy);
	pDC->LineTo (3 + x * m_iconSpace.cx+m_iconSize.cx,3 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (2 + x * m_iconSpace.cx, 3 + y * m_iconSpace.cy+m_iconSize.cy);
	pDC->LineTo (2 + x * m_iconSpace.cx, 2 + y * m_iconSpace.cy);
	if (++x >= m_viewSpace.cx) {
		x = 0;
		if (++y >= m_viewSpace.cy)
			break;
			}
		} 
pDC->SelectPalette(oldPalette, FALSE);
ReleaseDC(pDC);
}

								/*---------------------------*/

void CTextureView::Setup (void) 
{
  int nTextures, nFrames = 0;

  nTextures = textureManager.MaxTextures ();

  // calculate total number of textures
m_nTextures [1] = 0;
for (int i = 0; i < nTextures; i++) {
	if (textureManager.Texture (i)->m_info.bFrame)
		++nFrames;
	else
		m_mapViewToTex [m_nTextures [1]++] = i;
	}

// allocate memory for texture list
CreateTexMap ();
}

								/*---------------------------*/

afx_msg BOOL CTextureView::OnEraseBkgnd (CDC* pDC)
{
if (!theMine) 
	return FALSE;

   CRect    rc;
   CBrush   bkGnd, * pOldBrush;
   CPoint   pt (0,0);

ClientToScreen (&pt);
bkGnd.CreateStockObject (BLACK_BRUSH);
bkGnd.UnrealizeObject ();
pDC->SetBrushOrg (pt);
pOldBrush = pDC->SelectObject (&bkGnd);
GetClientRect (rc);
pDC->FillRect (&rc, &bkGnd);
pDC->SelectObject (pOldBrush);
bkGnd.DeleteObject ();
return 1;
}

								/*---------------------------*/

BOOL CTextureView::OnMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
	CRect	rc;

GetWindowRect (rc);
if ((pt.x < rc.left) || (pt.x >= rc.right) || (pt.y < rc.top) || (pt.y >= rc.bottom))
	return 1;

int nPos = GetScrollPos (SB_VERT) - zDelta / WHEEL_DELTA;

if (nPos >= m_nRows [m_bShowAll])
	nPos = m_nRows [m_bShowAll] - m_viewSpace.cy;
if (nPos < 0)
	nPos = 0;
SetScrollPos (SB_VERT, nPos, TRUE);
Refresh ();
return 0;
}

								/*---------------------------*/

afx_msg void CTextureView::OnPaint ()
{
	CRect	rc;
	CDC	*pDC;
	PAINTSTRUCT	ps;

if (!GetUpdateRect (rc))
	return;
pDC = BeginPaint (&ps);
RecalcLayout ();
EndPaint (&ps);
}
								/*---------------------------*/
