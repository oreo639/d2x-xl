 // dlcView.cpp : implementation of the CMineView class
//

#include <math.h>
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
static char THIS_FILE [] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolView


                        /*--------------------------*/

void CTextureTool::AnimateLight (void)
{
LightButton (m_nHighlight)->SetState (0);
m_nHighlight++;
m_nHighlight &= 0x1f; // 0..31

CButton *pb = LightButton (m_nHighlight);
pb->SetState (1);

CRect rc;
pb->GetClientRect (rc);
rc.InflateRect (-3, -4);
rc.left += 2;
rc.right++;
CDC *pDC = pb->GetDC ();
COLORREF	color = pb->GetCheck () ? RGB (0,0,0) : RGB (196,196,196);
CBrush br;
br.CreateSolidBrush (color);
CBrush *pOldBrush = pDC->SelectObject (&br);
CPen pen;
pen.CreatePen (PS_SOLID, 1, color);
CPen *pOldPen = pDC->SelectObject (&pen);
pDC->Ellipse (&rc);
pDC->SelectObject (pOldPen);
pDC->SelectObject (pOldBrush);
pb->ReleaseDC (pDC);
UpdateColorCtrl (
	&m_lightWnd, 
	pb->GetCheck () ? 
		(m_nColorIndex > 0) ?
			RGB (m_rgbColor.peRed, m_rgbColor.peGreen, m_rgbColor.peBlue) :
			RGB (255,196,0) : 
		RGB (0,0,0));
}

                        /*--------------------------*/

void CTextureTool::ToggleLight (INT32 i)
{
CButton *pb = LightButton (i - 1);
pb->SetCheck (!pb->GetCheck ());
SetLightString ();
}

                        /*--------------------------*/

void CTextureTool::UpdateLight (void)
{
	bool bUndo, bChange = false;

if (m_iLight < 0)
	return;
if (m_iLight >= MAX_FLICKERING_LIGHTS)
	return;

UINT32 nLightMask = 0;
for (INT32 i = 0; i < 32; i++)
	if (m_szLight [i] == '1')
		nLightMask |= (1 << i);
long nDelay = (m_nLightDelay * F1_0 /*- F0_5*/) / 1000;

CFlickeringLight* flP = theMine->FlickeringLights (m_iLight);
if ((flP->m_info.mask != nLightMask) || (flP->m_info.delay != nDelay)) {
	bUndo = theApp.SetModified (TRUE);
	theApp.LockUndo ();
	flP->m_info.mask = nLightMask;
	flP->m_info.delay = nDelay;
	theApp.UnlockUndo ();
	}
m_nLightDelay = (1000 * nDelay + F0_5) / F1_0;
}

                        /*--------------------------*/

bool CTextureTool::SetLightDelay (INT32 nSpeed)
{
if (nSpeed < 0)
	return false;
if (m_nLightTimer)
	KillTimer (m_nLightTimer);
if ((m_iLight >= 0) && nSpeed) {
	m_nLightDelay = nSpeed;
	m_nLightTimer = SetTimer (2, m_nLightDelay, NULL);
	((CSliderCtrl *) TimerSlider ())->SetPos (20 - (m_nLightDelay + 25) / 50);
	}
else {
	m_nLightDelay = 0;
	m_nLightTimer = -1;
	}
m_nLightTime = m_nLightDelay / 1000.0;
UpdateLight ();
UpdateData (FALSE);
return true;
}

                        /*--------------------------*/

void CTextureTool::SetLightString (void)
{
	static char cLight [2] = {'0', '1'};
	char szLight [33];

INT32 i;
for (i = 0; i < 32; i++)
	szLight [i] = cLight [LightButton (i)->GetCheck ()];
szLight [32] = '\0';
if (strcmp (szLight, m_szLight)) {
	strcpy_s (m_szLight, sizeof (m_szLight), szLight);
	UpdateLight ();
	UpdateData (FALSE);
	}
}
		
                        /*--------------------------*/

void CTextureTool::SetLightButtons (LPSTR szLight, INT32 nSpeed)
{
	bool	bDefault = false;

if (szLight) {
	if (szLight != m_szLight)
		strcpy_s (m_szLight, sizeof (m_szLight), szLight);
	}
else
	UpdateData (TRUE);
INT32 i;
for (i = 0; i < 32; i++) {
	if (!bDefault && (m_szLight [i] == '\0'))
		bDefault = true;
	if (bDefault)
		m_szLight [i] = '0';
	LightButton (i)->SetCheck (m_szLight [i] == '1');
	}
m_szLight [32] = '\0';
if (!SetLightDelay (nSpeed)) {
	UpdateLight ();
	UpdateData (FALSE);
	}
}

                        /*--------------------------*/

void CTextureTool::EnableLightControls (BOOL bEnable)
{
INT32 i;
for (i = IDC_TEXLIGHT_OFF; i <= IDC_TEXLIGHT_TIMER; i++)
	GetDlgItem (i)->EnableWindow (bEnable);
}

                        /*--------------------------*/

void CTextureTool::UpdateLightWnd (void)
{
CHECKMINE;

CWall *wallP = theMine->CurrWall ();
if (!SideHasLight ()) {
	if (m_bLightEnabled)
		EnableLightControls (m_bLightEnabled = FALSE);
	if (theMine->IsD2XLevel ())
		theMine->CurrLightColor ()->Clear ();
	}
else {
	if (!m_bLightEnabled)
		EnableLightControls (m_bLightEnabled = TRUE);
	if (theMine->IsD2XLevel ()) {
		CColor *plc = theMine->CurrLightColor ();
		if (!plc->m_info.index) {	// set light color to white for new lights
			plc->m_info.index = 255;
			plc->m_info.color.r =
			plc->m_info.color.g =
			plc->m_info.color.b = 1.0;
			}
		}
	}
m_iLight = theMine->GetFlickeringLight ();
if (m_iLight < 0) {
	OnLightOff ();
	return;
	}

long nLightMask = theMine->FlickeringLights (m_iLight)->m_info.mask;
INT32 i;
for (i = 0; i < 32; i++)
	m_szLight [i] = (nLightMask & (1 << i)) ? '1' : '0';
m_szLight [32] = '\0';
SetLightButtons (m_szLight, (INT32) (((1000 * theMine->FlickeringLights (m_iLight)->m_info.delay + F0_5) / F1_0)));
}

                        /*--------------------------*/

void CTextureTool::OnLightEdit ()
{
if (m_iLight < 0)
	UpdateData (FALSE);
else {
	UpdateData (TRUE);
	SetLightButtons ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnLightTimerEdit ()
{
if (m_iLight < 0)
	UpdateData (FALSE);
else {
	UpdateData (TRUE);
	SetLightDelay ((INT32) (1000 * m_nLightTime));
	}
}

                        /*--------------------------*/

void CTextureTool::OnAddLight ()
{
if (m_iLight >= 0)
	INFOMSG (" There is already a flickering light.")
else if (0 <= (m_iLight = theMine->AddFlickeringLight (-1, -1, 0xAAAAAAAAL, F1_0 / 4))) {
	UpdateLightWnd ();
	theApp.MineView ()->Refresh ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnDeleteLight ()
{
if (m_iLight < 0)
	INFOMSG (" There is no flickering light.")
else if (theMine->DeleteFlickeringLight ()) {
	m_iLight = -1;
	UpdateLightWnd ();
	theApp.MineView ()->Refresh ();
	}
}

                        /*--------------------------*/

void CTextureTool::OnLightOff () { SetLightButtons ("", 1000); }
void CTextureTool::OnLightOn () { SetLightButtons ("11111111111111111111111111111111", 1000); }
void CTextureTool::OnLightStrobe4 () { SetLightButtons ("10000000100000001000000010000000", 250); }
void CTextureTool::OnLightStrobe8 () { SetLightButtons ("10001000100010001000100010001000", 250); }
void CTextureTool::OnLightFlicker () { SetLightButtons ("11111111000000111100010011011110", 100); }
void CTextureTool::OnLightDefault () { SetLightButtons ("10101010101010101010101010101010", 250); }

                        /*--------------------------*/

void CTextureTool::OnLight1 () { ToggleLight (1); }
void CTextureTool::OnLight2 () { ToggleLight (2); }
void CTextureTool::OnLight3 () { ToggleLight (3); }
void CTextureTool::OnLight4 () { ToggleLight (4); }
void CTextureTool::OnLight5 () { ToggleLight (5); }
void CTextureTool::OnLight6 () { ToggleLight (6); }
void CTextureTool::OnLight7 () { ToggleLight (7); }
void CTextureTool::OnLight8 () { ToggleLight (8); }
void CTextureTool::OnLight9 () { ToggleLight (9); }
void CTextureTool::OnLight10 () { ToggleLight (10); }
void CTextureTool::OnLight11 () { ToggleLight (11); }
void CTextureTool::OnLight12 () { ToggleLight (12); }
void CTextureTool::OnLight13 () { ToggleLight (13); }
void CTextureTool::OnLight14 () { ToggleLight (14); }
void CTextureTool::OnLight15 () { ToggleLight (15); }
void CTextureTool::OnLight16 () { ToggleLight (16); }
void CTextureTool::OnLight17 () { ToggleLight (17); }
void CTextureTool::OnLight18 () { ToggleLight (18); }
void CTextureTool::OnLight19 () { ToggleLight (19); }
void CTextureTool::OnLight20 () { ToggleLight (20); }
void CTextureTool::OnLight21 () { ToggleLight (21); }
void CTextureTool::OnLight22 () { ToggleLight (22); }
void CTextureTool::OnLight23 () { ToggleLight (23); }
void CTextureTool::OnLight24 () { ToggleLight (24); }
void CTextureTool::OnLight25 () { ToggleLight (25); }
void CTextureTool::OnLight26 () { ToggleLight (26); }
void CTextureTool::OnLight27 () { ToggleLight (27); }
void CTextureTool::OnLight28 () { ToggleLight (28); }
void CTextureTool::OnLight29 () { ToggleLight (29); }
void CTextureTool::OnLight30 () { ToggleLight (30); }
void CTextureTool::OnLight31 () { ToggleLight (31); }
void CTextureTool::OnLight32 () { ToggleLight (32); }

                        /*--------------------------*/

void CTextureTool::SetWallColor (void)
{
if (theMine->UseTexColors ()) {
	INT16			nSegment, nSide;
	INT16			nBaseTex = theMine->CurrSide ()->m_info.nBaseTex;
	CSegment*	segP = theMine->Segments (0);
	CSide*		sideP;
	CWall			*wallP;
	bool			bAll = !theMine->GotMarkedSides ();

	for (nSegment = 0; nSegment < theMine->SegCount (); nSegment++, segP++) {
		for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++) {
			if (sideP->m_info.nWall < 0)
				continue;
			wallP = theMine->Walls (sideP->m_info.nWall);
			if (wallP->m_info.type != WALL_TRANSPARENT)
				continue;
			if (!(bAll || theMine->SideIsMarked (nSegment, nSide)))
				continue;
			if (sideP->m_info.nBaseTex != nBaseTex)
				continue;
			wallP->m_info.cloakValue = m_nColorIndex;
			}
		}
	}
}

                        /*--------------------------*/

void CTextureTool::OnLButtonDown (UINT nFlags, CPoint point)
{
	CRect		rcPal;

if (/*(theMine->IsD2XLevel ()) &&*/ SideHasLight ()) {
	GetCtrlClientRect (&m_paletteWnd, rcPal);
	if (PtInRect (rcPal, point)) {
		point.x -= rcPal.left;
		point.y -= rcPal.top;
		if (m_paletteWnd.SelectColor (point, m_nColorIndex, &m_rgbColor)) {
			CWall *wallP = theMine->CurrWall ();
			if (wallP && (wallP->m_info.type == WALL_TRANSPARENT)) {
				wallP->m_info.cloakValue = m_nColorIndex;
				SetWallColor ();
				}
			CColor *psc = theMine->CurrLightColor ();
			if (psc->m_info.index = m_nColorIndex) {
				psc->m_info.color.r = (double) m_rgbColor.peRed / 255.0;
				psc->m_info.color.g = (double) m_rgbColor.peGreen / 255.0;
				psc->m_info.color.b = (double) m_rgbColor.peBlue / 255.0;
				}
			else {
				psc->m_info.color.r =
				psc->m_info.color.g =
				psc->m_info.color.b = 1.0;
				}
			//if (!wallP || (wallP->m_info.type != WALL_TRANSPARENT)) 
				{
				theMine->SetTexColor (theMine->CurrSide ()->m_info.nBaseTex, psc);
				theMine->SetTexColor (theMine->CurrSide ()->m_info.nOvlTex, psc);
				}
			UpdateData (FALSE);
			UpdatePaletteWnd ();
			}
		}
	}
}

                        /*--------------------------*/
		
void CTextureTool::ChooseRGBColor (void)
{
	CHOOSECOLOR	cc;

memset (&cc, 0, sizeof (cc));
cc.lStructSize = sizeof (cc);
cc.hwndOwner = theApp.MainFrame ()->m_hWnd;
cc.rgbResult = RGB (m_rgbColor.peRed, m_rgbColor.peGreen, m_rgbColor.peBlue);
cc.lpCustColors = m_custColors;
cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT | CC_SHOWHELP;
if (ChooseColor (&cc)) {
	CColor *psc = theMine->CurrLightColor ();
	psc->m_info.index = m_nColorIndex = 255;
	m_rgbColor.peBlue = ((BYTE) (cc.rgbResult >> 16)) & 0xFF;
	m_rgbColor.peGreen = ((BYTE) (cc.rgbResult >> 8)) & 0xFF;
	m_rgbColor.peRed = ((BYTE) cc.rgbResult) & 0xFF;
	psc->m_info.color.r = (double) m_rgbColor.peRed / 255.0;
	psc->m_info.color.g = (double) m_rgbColor.peGreen / 255.0;
	psc->m_info.color.b = (double) m_rgbColor.peBlue / 255.0;
	theMine->SetTexColor (theMine->CurrSide ()->m_info.nBaseTex, psc);
	theMine->SetTexColor (theMine->CurrSide ()->m_info.nOvlTex, psc);
	UpdatePaletteWnd ();
	}
}

                        /*--------------------------*/
		
		//eof texturedlg.cpp