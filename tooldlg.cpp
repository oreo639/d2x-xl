// dlcView.cpp : implementation of the CMineView class
//

#include "stdafx.h"
#include "afxpriv.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"
#include "toolview.h"

#include "palette.h"
#include "textures.h"
#include "global.h"
#include "render.h"
#include "io.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolView

IMPLEMENT_DYNCREATE(CToolDlg, CPropertyPage)

BEGIN_MESSAGE_MAP(CToolDlg, CPropertyPage)
	ON_COMMAND(ID_SEL_PREVTAB, OnSelectPrevTab)
	ON_COMMAND(ID_SEL_NEXTTAB, OnSelectNextTab)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
   ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()

								/*---------------------------*/

CToolDlg::CToolDlg (UINT nIdTemplate, CPropertySheet *pParent)
	: CPropertyPage (nIdTemplate, 0)
{
m_pParent = pParent; 
m_bInited = false; 
EnableToolTips (true);
}

								/*---------------------------*/

void CToolDlg::DDX_Double (CDataExchange * pDX, INT32 nIDC, double& fVal, double min, double max, LPSTR pszFmt, LPSTR pszErrMsg)
{
   HWND  hWndCtrl = pDX->PrepareEditCtrl (nIDC);
	char	szVal [100];

if (pDX->m_bSaveAndValidate) {
   ::GetWindowText (hWndCtrl, szVal, sizeof (szVal));
	fVal = atof (szVal);
	if ((min < max) && ((fVal < min) || (fVal > max))) {
		if (fVal < min)
			fVal = min;
		else if (fVal > max)
			fVal = max;
		}
	}
else {
	sprintf_s (szVal, sizeof (szVal), pszFmt && *pszFmt ? pszFmt : "%1.2f", fVal);
   AfxSetWindowText (hWndCtrl, szVal);
	}
}

								/*---------------------------*/

INT32 CToolDlg::DDX_Int (CDataExchange * pDX, INT32 nIDC, INT32 i)
{
   HWND  hWndCtrl = pDX->PrepareEditCtrl (nIDC);
	char	szVal [100];

if (pDX->m_bSaveAndValidate) {
   ::GetWindowText (hWndCtrl, szVal, sizeof (szVal));
	return atoi (szVal);
	}
else {
	sprintf_s (szVal, sizeof (szVal), "%d", i);
   AfxSetWindowText (hWndCtrl, szVal);
	return i;
	}
}

								/*---------------------------*/

INT32 CToolDlg::DDX_Flag (CDataExchange * pDX, INT32 nIDC, INT32 i)
{
DDX_Check (pDX, nIDC, i);
return i;
}

                        /*--------------------------*/

void CToolDlg::InitSlider (INT32 nIdC, INT32 nMin, INT32 nMax) 
{
	CSliderCtrl	*ps;

if (ps = (CSliderCtrl *) GetDlgItem (nIdC)) {
	ps->SetRange (nMin, nMax, TRUE);
	ps->SetPos (nMin);
	}
}

                        /*--------------------------*/

void CToolDlg::DDX_Slider (CDataExchange * pDX, INT32 nIdC, INT32& nTic) 
{
	CSliderCtrl	*ps;

ps = (CSliderCtrl *) GetDlgItem (nIdC);
if (pDX->m_bSaveAndValidate)
	nTic = ps->GetPos ();
else
	ps->SetPos (nTic);
if (nTic != ps->GetPos ())
	nTic = 0;
}

                        /*--------------------------*/

INT32 CToolDlg::GetCheck (INT32 nIdC)
{
	CButton	*pb;

return (pb = (CButton *) GetDlgItem (nIdC)) ? pb->GetCheck () : FALSE;
}

								/*--------------------------*/

void CToolDlg::EnableControls (INT32 nIdFirst, INT32 nIdLast, BOOL bEnable)
{
CWnd *pWnd;
INT32 i;
for (i = nIdFirst; i <= nIdLast; i++)
	if (pWnd = GetDlgItem (i))
		pWnd->EnableWindow (bEnable);
}

                        /*--------------------------*/

void CToolDlg::CreateImgWnd (CWnd * pImgWnd, INT32 nIdC)
{
CWnd *pParentWnd = GetDlgItem (nIdC);
CRect rc;
pParentWnd->GetClientRect (rc);
pImgWnd->Create (NULL, NULL, WS_CHILD | WS_VISIBLE, rc, pParentWnd, 0);
}

                        /*--------------------------*/

void CToolDlg::OnSelectPrevTab ()
{
theApp.MainFrame ()->ShowTools ();
theApp.ToolView ()->PrevTab ();
}

                        /*--------------------------*/

void CToolDlg::OnSelectNextTab ()
{
theApp.MainFrame ()->ShowTools ();
theApp.ToolView ()->NextTab ();
}

                        /*--------------------------*/

INT32 CToolDlg::CBAddString (CComboBox *pcb, char *str)
{
	INT32	i = 0, m = 0, l = 0, r = pcb->GetCount () - 1;
	char	h [80], *hsz, *psz;

psz = isalpha (*str) ? str : strstr (str, ":") + 1;
while (l <= r) {
	m = (l + r) / 2;
	pcb->GetLBText (m, h);
	hsz = isalpha (*h) ? h: strstr (h, ":") + 1;
	i = strcmp (psz, hsz);
	if (i < 0)
		r = m - 1;
	else if (i > 0)
		l = m + 1;
	else
		break;
	}
if (i > 0)
	m++;
return pcb->InsertString (m, str);
}

                        /*--------------------------*/

void CToolDlg::SelectItemData (CComboBox *pcb, INT32 nItemData)
{
//if (nItemData >= 0) 
	{
	INT32 i, h = pcb->GetCount ();
	for (i = 0; i < h; i++)
		if (pcb->GetItemData (i) == (DWORD) nItemData) {
			pcb->SetCurSel (i);
			return;
			}
	}
pcb->SetCurSel (-1);
}

                        /*--------------------------*/

#if 0
INT32 CToolDlg::OnToolHitTest (CPoint point, TOOLINFO* pTI)
{
	CRect	rc;

GetClientRect (rc);
if (!rc.PtInRect (point))
	return -1;
return 1;
}
#endif

                        /*--------------------------*/

BOOL CToolDlg::OnToolTipNotify (UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
   // need to handle both ANSI and UNICODE versions of the message
TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
//CString strTipText;
char strTipText [100], *psz;
UINT nID = UINT (pNMHDR->idFrom);
if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
    pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND)) {
   // idFrom is actually the HWND of the tool
   nID = INT32 (::GetDlgCtrlID (HWND (UINT_PTR (nID))));
}
if (nID != 0) // will be zero on a separator
	if (!LoadString (AfxGetApp()->m_hInstance, nID, strTipText, sizeof (strTipText)))
		*strTipText = '\0';
	else if (psz = strchr (strTipText, '\n'))
		*psz = '\0';
//   strTipText.Format ("Control ID = %d", nID);
if (pNMHDR->code == TTN_NEEDTEXTA)
   lstrcpyn(pTTTA->szText, strTipText, sizeof (pTTTA->szText));
else
   _mbstowcsz(pTTTW->szText, strTipText, sizeof (pTTTW->szText));
*pResult = 0;
return TRUE;    // message was handled
}

                        /*--------------------------*/

IMPLEMENT_DYNCREATE(CTexToolDlg, CToolDlg)

BEGIN_MESSAGE_MAP(CTexToolDlg, CToolDlg)
	ON_WM_PAINT ()
	ON_WM_TIMER ()
END_MESSAGE_MAP()

								/*---------------------------*/

CTexToolDlg::CTexToolDlg (UINT nIdTemplate, CPropertySheet *pParent, 
								  INT32 nTexWndId, INT32 nTimerId, COLORREF bkColor,
								  bool bOtherSeg)
	: CToolDlg (nIdTemplate, pParent)
{
m_nTexWndId = nTexWndId;
m_nTimerId = nTimerId;
m_bkColor = bkColor;
m_nTimer = -1;
m_bOtherSeg = bOtherSeg;
}

                        /*--------------------------*/

CTexToolDlg::~CTexToolDlg ()
{
if (m_bInited) {
	if (IsWindow (m_textureWnd))
		m_textureWnd.DestroyWindow ();
	}
}

                        /*--------------------------*/

BOOL CTexToolDlg::OnInitDialog ()
{
CToolDlg::OnInitDialog ();
CreateImgWnd (&m_textureWnd, m_nTexWndId);
return TRUE;
}

                        /*--------------------------*/

BOOL CTexToolDlg::TextureIsVisible ()
{
return TRUE;
}

                        /*--------------------------*/

bool CTexToolDlg::Refresh (INT16 nBaseTex, INT16 nOvlTex, INT16 nVisible)
{
m_frame [0] = 0;
m_frame [1] = 0;
if (nVisible < 0)
	nVisible = (INT16) TextureIsVisible ();
if (nVisible > 0) {
	if (nBaseTex < 0) {
		INT16 nSegment = m_bOtherSeg ? theMine->Other ()->nSegment : theMine->Current ()->nSegment;
		INT16 nSide = m_bOtherSeg ? theMine->Other ()->nSide : theMine->Current ()->nSide;
		if (nVisible = theMine->IsWall (nSegment, nSide)) {
			CSide *sideP = m_bOtherSeg ? theMine->OtherSide () : theMine->CurrSide ();
			nBaseTex = sideP->nBaseTex;
			nOvlTex = sideP->nOvlTex & 0x3fff;
			}
		}
	}
if (nVisible > 0)
	return PaintTexture (&m_textureWnd, m_bkColor, -1, -1, nBaseTex, nOvlTex);
return PaintTexture (&m_textureWnd, m_bkColor, -1, -1, MAX_TEXTURES);
}

                        /*--------------------------*/

void CTexToolDlg::OnPaint ()
{
CToolDlg::OnPaint ();
if (TextureIsVisible ()) {
	CSide *sideP = m_bOtherSeg ? theMine->OtherSide () : theMine->CurrSide ();
	PaintTexture (&m_textureWnd, m_bkColor, -1, -1, sideP->nBaseTex, sideP->nOvlTex & 0x1fff);
	}
else
	PaintTexture (&m_textureWnd, m_bkColor, -1, -1, MAX_TEXTURES);
m_textureWnd.InvalidateRect (NULL);
m_textureWnd.UpdateWindow ();
}

                        /*--------------------------*/

BOOL CTexToolDlg::OnSetActive ()
{
if (m_nTimerId >= 0)
	m_nTimer = SetTimer ((UINT) m_nTimerId, 100U, NULL);
return CToolDlg::OnSetActive ();
}

                        /*--------------------------*/

BOOL CTexToolDlg::OnKillActive ()
{
if (m_nTimer >= 0) {
	KillTimer (m_nTimer);
	m_nTimer = -1;
	}
return CToolDlg::OnKillActive ();
}

                        /*--------------------------*/

	static INT32 scroll_offset_x = 0;
	static INT32 scroll_offset_y = 0;
	static INT32 old_x,old_y;

void CTexToolDlg::AnimateTexture (void)
{
if (!TextureIsVisible ())
	return;

	CSegment *segP = m_bOtherSeg ? theMine->OtherSeg () : theMine->CurrSeg ();

	UINT16 texture [2];
	INT32 bScroll;
	INT32 x,y;

	CSide	*sideP = m_bOtherSeg ? theMine->OtherSide () : theMine->CurrSide ();

texture [0] = sideP->nBaseTex & 0x3fff;
texture [1] = sideP->nOvlTex;

// if texture1 is a scrolling texture, then offset the textures and
// redraw them, then return
bScroll = theMine->ScrollSpeed (texture [0], &x, &y);
if (bScroll) {
	PaintTexture (&m_textureWnd, m_bkColor, -1, -1, texture [0], texture [1], scroll_offset_x, scroll_offset_y);
//	DrawTexture (texture [0], texture [1], scroll_offset_x, scroll_offset_y);
	if (old_x != x || old_y != y) {
		scroll_offset_x = 0;
		scroll_offset_y = 0;
		}
	old_x = x;
	old_y = y;
	scroll_offset_x += x;
	scroll_offset_y += y;
	scroll_offset_x &= 63;
	scroll_offset_y &= 63;
	return;
	}

scroll_offset_x = 0;
scroll_offset_y = 0;

// abort if this is not a wall
#ifndef _DEBUG
UINT16 nWall = sideP->nWall;
if (nWall >= theMine->GameInfo ().walls.count)
	return;

// abort if this wall is not a door
//if (theMine->Walls (nWall)->type != WALL_DOOR)
//	return;
#endif
	INT32 i;
	static INT32 hold_time [2] = {0,0};
	static INT32 inc [2]= {1,1}; // 1=forward or -1=backwards
	INT32 index [2];
	static UINT16 d1_anims [] = {
		371, 376, 387, 399, 413, 419, 424, 436, 444, 459,
		472, 486, 492, 500, 508, 515, 521, 529, 536, 543,
		550, 563, 570, 577, 584, 0
		};
// note: 584 is not an anim texture, but it is used to calculate
//       the number of m_frames in 577
// The 0 is used to end the search

	static UINT16 d2_anims [] = {
		435, 440, 451, 463, 477, 483, 488, 500, 508, 523,
		536, 550, 556, 564, 572, 579, 585, 593, 600, 608,
		615, 628, 635, 642, 649, 664, 672, 687, 702, 717,
		725, 731, 738, 745, 754, 763, 772, 780, 790, 806,
		817 ,827 ,838 ,849 ,858 ,863 ,871 ,886, 901 ,910,
		0
		};
// note: 910 is not an anim texture, but it is used to calculate
//       the number of m_frames in 901
// The 0 is used to end the search
	UINT16 *anim; // points to d1_anim or d2_anim depending on m_fileType

// first find out if one of the textures is animated
anim = (theApp.IsD1File ()) ? d1_anims : d2_anims;

for (i=0; i<2;i++)
	for (index [i] = 0; anim [index [i]]; index [i]++)
		if (texture [i] == anim [index [i]])
			break;

if (anim [index [0]] || anim [index [1]]) {
	// calculate new texture numbers
	for (i = 0; i < 2; i++) {
		if (anim [index [i]]) {
		// if hold time has not expired, then return
			if (hold_time [i] < 5)
				hold_time [i]++;
			else
				m_frame [i] += inc [i];
			if (m_frame [i] < 0) {
				m_frame [i] = 0;
				hold_time [i] = 0;
				inc [i] = 1;
				}
			if (anim [index [i]] + m_frame [i] >= anim [index [i] + 1]) {
				m_frame [i] = (anim [index [i] + 1] - anim [index [i]]) - 1;
				hold_time [i] = 0;
				inc [i] = -1;
				}
			texture [i] = anim [index [i]] + m_frame [i];
			}
		}
	PaintTexture (&m_textureWnd, m_bkColor, -1, -1, texture [0], texture [1]);
	}
}

                        /*--------------------------*/

void CTexToolDlg::OnTimer (UINT_PTR nIdEvent)
{
if (nIdEvent == (UINT) m_nTimerId)
	AnimateTexture ();
else 
	CToolDlg::OnTimer (nIdEvent);
}

                        /*--------------------------*/

void CToolDlg::GetCtrlClientRect (CWnd *pWnd, CRect& rc)
{
	CRect	rcc;
	INT32	dx, dy;

pWnd->GetClientRect (&rcc);
pWnd->GetWindowRect (rc);
dx = rc.Width () - rcc.Width ();
dy = rc.Height () - rcc.Height ();
ScreenToClient (rc);
rc.DeflateRect (dx / 2, dy / 2);
}

								/*---------------------------*/

IMPLEMENT_DYNCREATE (CExtBitmapButton, CBitmapButton)

BEGIN_MESSAGE_MAP(CExtBitmapButton, CBitmapButton)
	ON_WM_LBUTTONDOWN ()
	ON_WM_LBUTTONUP ()
	ON_WM_MOUSEMOVE ()
//	ON_WM_SETFOCUS ()
//	ON_WM_KILLFOCUS ()
//	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipNotify)
//	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()

								/*---------------------------*/

void CExtBitmapButton::Notify (UINT nMsg)
{
	NMHDR	nmHdr;

nmHdr.hwndFrom = m_hWnd;
nmHdr.idFrom = m_nId;
nmHdr.code = nMsg;
m_pParent->SendMessage (WM_NOTIFY, m_nId, (LPARAM) &nmHdr);
}

								/*---------------------------*/

void CExtBitmapButton::OnLButtonDown (UINT nFlags, CPoint point)
{
CBitmapButton::OnLButtonDown (nFlags, point);
Notify (m_nState = WM_LBUTTONDOWN);
}

								/*---------------------------*/

void CExtBitmapButton::OnLButtonUp (UINT nFlags, CPoint point)
{
CBitmapButton::OnLButtonUp (nFlags, point);
Notify (m_nState = WM_LBUTTONUP);
}

								/*---------------------------*/

afx_msg void CExtBitmapButton::OnMouseMove (UINT nFlags, CPoint point)
{
if (nFlags & MK_LBUTTON) {
	CRect	rc;

	GetClientRect (&rc);
	if ((point.x < 0) || (point.y < 0) || (point.x >= rc.right) || (point.y >= rc.bottom)) {
		if (m_nPos == 1) {
			Notify (WM_LBUTTONUP);
			m_nPos = 0;
			}
		}
	else {
		if (m_nPos != 1) {
			Notify (WM_LBUTTONDOWN);
			m_nPos = 1;
			}
		}
	}
CBitmapButton::OnMouseMove (nFlags, point);
}

								/*---------------------------*/

void CExtBitmapButton::OnSetFocus (CWnd* pOldWnd)
{
if (m_nState == WM_LBUTTONDOWN)
	Notify (WM_LBUTTONDOWN);
}

								/*---------------------------*/

void CExtBitmapButton::OnKillFocus (CWnd* pNewWnd)
{
if (m_nState == WM_LBUTTONDOWN)
	Notify (WM_LBUTTONUP);
}

								/*---------------------------*/

//eof tooldlg.cpp