// Copyright (C) 1997 Bryan Aamot
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

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CWallTool, CTexToolDlg)
	ON_WM_HSCROLL ()
	ON_BN_CLICKED (IDC_WALL_ADD, OnAddWall)
	ON_BN_CLICKED (IDC_WALL_DELETE, OnDeleteWall)
	ON_BN_CLICKED (IDC_WALL_DELETEALL, OnDeleteWallAll)
	ON_BN_CLICKED (IDC_WALL_OTHERSIDE, OnOtherSide)
	ON_BN_CLICKED (IDC_WALL_LOCK, OnLock)
	ON_BN_CLICKED (IDC_WALL_NOKEY, OnNoKey)
	ON_BN_CLICKED (IDC_WALL_BLUEKEY, OnBlueKey)
	ON_BN_CLICKED (IDC_WALL_GOLDKEY, OnGoldKey)
	ON_BN_CLICKED (IDC_WALL_REDKEY, OnRedKey)
	ON_BN_CLICKED (IDC_WALL_BLASTED, OnBlasted)
	ON_BN_CLICKED (IDC_WALL_DOOROPEN, OnDoorOpen)
	ON_BN_CLICKED (IDC_WALL_DOORLOCKED, OnDoorLocked)
	ON_BN_CLICKED (IDC_WALL_DOORAUTO, OnDoorAuto)
	ON_BN_CLICKED (IDC_WALL_ILLUSIONOFF, OnIllusionOff)
	ON_BN_CLICKED (IDC_WALL_SWITCH, OnSwitch)
	ON_BN_CLICKED (IDC_WALL_BUDDYPROOF, OnBuddyProof)
	ON_BN_CLICKED (IDC_WALL_RENDER_ADDITIVE, OnRenderAdditive)
	ON_BN_CLICKED (IDC_WALL_IGNORE_MARKER, OnIgnoreMarker)
	ON_BN_CLICKED (IDC_WALL_ADD_DOOR_NORMAL, OnAddDoorNormal)
	ON_BN_CLICKED (IDC_WALL_ADD_DOOR_EXIT, OnAddDoorExit)
	ON_BN_CLICKED (IDC_WALL_ADD_DOOR_SECRETEXIT, OnAddDoorSecretExit)
	ON_BN_CLICKED (IDC_WALL_ADD_DOOR_PRISON, OnAddDoorPrison)
	ON_BN_CLICKED (IDC_WALL_ADD_DOOR_GUIDEBOT, OnAddDoorGuideBot)
	ON_BN_CLICKED (IDC_WALL_ADD_WALL_FUELCELL, OnAddWallFuelCell)
	ON_BN_CLICKED (IDC_WALL_ADD_WALL_ILLUSION, OnAddWallIllusion)
	ON_BN_CLICKED (IDC_WALL_ADD_WALL_FORCEFIELD, OnAddWallForceField)
	ON_BN_CLICKED (IDC_WALL_ADD_WALL_FAN, OnAddWallFan)
	ON_BN_CLICKED (IDC_WALL_ADD_WALL_GRATE, OnAddWallGrate)
	ON_BN_CLICKED (IDC_WALL_ADD_WALL_WATERFALL, OnAddWallWaterfall)
	ON_BN_CLICKED (IDC_WALL_ADD_WALL_LAVAFALL, OnAddWallLavafall)
	ON_BN_CLICKED (IDC_WALL_BOTHSIDES, OnBothSides)
	ON_BN_CLICKED (IDC_WALL_FLYTHROUGH, OnStrength)
	ON_CBN_SELCHANGE (IDC_WALL_WALLNO, OnSetWall)
	ON_CBN_SELCHANGE (IDC_WALL_TYPE, OnSetType)
	ON_CBN_SELCHANGE (IDC_WALL_CLIPNO, OnSetClip)
	ON_EN_KILLFOCUS (IDC_WALL_STRENGTH, OnStrength)
	ON_EN_KILLFOCUS (IDC_WALL_CLOAK, OnCloak)
	ON_EN_UPDATE (IDC_WALL_STRENGTH, OnStrength)
#ifdef RELEASE
	ON_EN_UPDATE (IDC_WALL_CLOAK, OnCloak)
#endif
END_MESSAGE_MAP ()

//------------------------------------------------------------------------
// DIALOG - CWallTool (constructor)
//------------------------------------------------------------------------

CWallTool::CWallTool (CPropertySheet *pParent)
	: CTexToolDlg (nLayout ? IDD_WALLDATA2 : IDD_WALLDATA, pParent, IDC_WALL_SHOW, 5, RGB (0,0,0))
{
m_defWall.Clear ();
m_defWall.m_info.type = WALL_DOOR;
m_defWall.m_info.flags = WALL_DOOR_AUTO;
m_defWall.m_info.keys = KEY_NONE;
m_defWall.m_info.nClip = -1;
m_defWall.m_info.cloakValue = 16; //50%
m_defDoorTexture = -1;
m_defTexture = -1;
m_defOvlTexture = 414;
m_nType = 0;
memcpy (&m_defDoor, &m_defWall, sizeof (m_defDoor));
m_bBothSides = FALSE;
m_bLock = false;
m_bDelayRefresh = false;
Reset ();
}

                        /*--------------------------*/

CWallTool::~CWallTool ()
{
}

                        /*--------------------------*/

void CWallTool::Reset ()
{
m_nSegment = 0;
m_nSide = 1;
m_nTrigger = 0;
m_nWall [0] = -1;
m_nWall [1] = -1;
m_nClip = 0;
m_nStrength = 0;
m_nCloak = 0;
m_bFlyThrough = 0;
memset (m_bKeys, 0, sizeof (m_bKeys));
memset (m_bFlags, 0, sizeof (m_bFlags));
*m_szMsg = '\0';
}

                        /*--------------------------*/

BOOL CWallTool::TextureIsVisible ()
{
return m_pWall [0] != NULL;
}

//------------------------------------------------------------------------
// CWallTool - SetupWindow
//------------------------------------------------------------------------

BOOL CWallTool::OnInitDialog ()
{
	static char* pszWallTypes [] = {
		"Normal",
		"Blastable",
		"Door",
		"Illusion",
		"Open",
		"Close",
		"Overlay",
		"Cloaked",
		"Transparent"
		};

	CComboBox *pcb;

CTexToolDlg::OnInitDialog ();
InitCBWallNo ();

pcb = CBType ();
pcb->ResetContent ();

INT32 h, i, j = sizeof (pszWallTypes) /  sizeof (*pszWallTypes);
for (i = 0; i < j; i++) {
	h = pcb->AddString (pszWallTypes [i]);
	pcb->SetItemData (h, i);
	}	

pcb = CBClipNo ();
pcb->ResetContent ();
j = (theApp.IsD2File ()) ? D2_NUM_OF_CLIPS : NUM_OF_CLIPS;
for (i = 0; i < j; i++) {
	sprintf_s (m_szMsg, sizeof (m_szMsg), i ? "door%02d" : "wall%02d", doorClipTable [i]);
	pcb->AddString (m_szMsg);
	}
InitSlider (IDC_WALL_TRANSPARENCY, 0, 10);
for (i = 0; i <= 10; i++)
	SlCtrl (IDC_WALL_TRANSPARENCY)->SetTic (i);
*m_szMsg = '\0';
m_bInited = true;
return TRUE;
}

                        /*--------------------------*/

void CWallTool::InitCBWallNo ()
{
CComboBox *pcb = CBWallNo ();
pcb->ResetContent ();
INT32 i;
for (i = 0; i < theMine->GameInfo ().walls.count; i++) {
	_itoa_s (i, message, sizeof (message), 10);
	pcb->AddString (message);
	}
pcb->SetCurSel (m_nWall [0]);
}

                        /*--------------------------*/

void CWallTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;
DDX_Text (pDX, IDC_WALL_CUBE, m_nSegment);
DDX_Text (pDX, IDC_WALL_SIDE, m_nSide);
DDX_Text (pDX, IDC_WALL_TRIGGER, m_nTrigger);
DDX_CBIndex (pDX, IDC_WALL_WALLNO, m_nWall [0]);
//DDX_CBIndex (pDX, IDC_WALL_TYPE, m_nType);
SelectItemData (CBType (), m_nType);
DDX_CBIndex (pDX, IDC_WALL_CLIPNO, m_nClip);
DDX_Double (pDX, IDC_WALL_STRENGTH, m_nStrength, -100, 100, "%3.1f");
DDX_Double (pDX, IDC_WALL_CLOAK, m_nCloak, 0, 100, "%3.1f");
INT32 i;
for (i = 0; i < 4; i++)
	DDX_Check (pDX, IDC_WALL_NOKEY + i, m_bKeys [i]);
for (i = 0; i < MAX_WALL_FLAGS; i++)
	DDX_Check (pDX, IDC_WALL_BLASTED + i, m_bFlags [i]);
DDX_Check (pDX, IDC_WALL_FLYTHROUGH, m_bFlyThrough);
DDX_Text (pDX, IDC_WALL_MSG, m_szMsg, sizeof (m_szMsg));
char szTransparency [20];
INT32 t = (INT32) (((m_nType == WALL_TRANSPARENT) ? m_nStrength : m_nCloak) + 5) / 10;
DDX_Slider (pDX, IDC_WALL_TRANSPARENCY, t);
sprintf_s (szTransparency, sizeof (szTransparency), "transp: %d%c", t * 10, '%');
SetDlgItemText (IDC_WALL_TRANSPARENCY_TEXT, szTransparency);
}

                        /*--------------------------*/

void CWallTool::EnableControls (BOOL bEnable)
{
INT32 i;
for (i = IDC_WALL_WALLNO + 1; i <= IDC_WALL_FLYTHROUGH; i++)
	GetDlgItem (i)->EnableWindow (bEnable);
}

                        /*--------------------------*/

BOOL CWallTool::OnSetActive ()
{
Refresh ();
GetWalls ();
return CTexToolDlg::OnSetActive ();
}

                        /*--------------------------*/

BOOL CWallTool::OnKillActive ()
{
Refresh ();
return CTexToolDlg::OnKillActive ();
}

                        /*--------------------------*/

void CWallTool::OnLock ()
{
m_bLock = !m_bLock;
GetDlgItem (IDC_WALL_LOCK)->SetWindowText (m_bLock ? "&unlock" : "&lock");
}

//------------------------------------------------------------------------
// CWallTool - RefreshData
//------------------------------------------------------------------------

void CWallTool::Refresh ()
{
if (m_bDelayRefresh)
	return;
if (!(m_bInited && theMine))
	return;

InitCBWallNo ();
if (!(m_pWall [0] = theMine->FindWall ())) {
	strcpy_s (m_szMsg, sizeof (m_szMsg), "No wall for current side");
	EnableControls (FALSE);
	if (theMine->CurrSeg ()->Child (theMine->Current ()->nSide) >= 0)
		CToolDlg::EnableControls (IDC_WALL_ADD_DOOR_NORMAL, IDC_WALL_ADD_WALL_LAVAFALL, TRUE);
	GetDlgItem (IDC_WALL_ADD)->EnableWindow (TRUE);
	GetDlgItem (IDC_WALL_TYPE)->EnableWindow (TRUE);
	//CBType ()->SetCurSel (m_nType);
	SelectItemData (CBType (), m_nType);
	CBClipNo ()->SetCurSel (-1);
	Reset ();
	} 
else {
    // enable all
	EnableControls (TRUE);
	GetDlgItem (IDC_WALL_ADD)->EnableWindow (FALSE);
   if ((theApp.IsD2File ()) && (m_pWall [0]->m_info.type == WALL_TRANSPARENT))
		GetDlgItem (IDC_WALL_STRENGTH)->EnableWindow (FALSE);
	else {
		GetDlgItem (IDC_WALL_FLYTHROUGH)->EnableWindow (FALSE);
		}
   if ((theApp.IsD1File ()) || (m_pWall [0]->m_info.type == WALL_TRANSPARENT))
		GetDlgItem (IDC_WALL_CLOAK)->EnableWindow (FALSE);

    // enable buddy proof and switch checkboxes only if d2 level
	if (theApp.IsD1File ()) {
		INT32 i;
		for (i = 0; i < 2; i++)
			GetDlgItem (IDC_WALL_SWITCH + i)->EnableWindow (FALSE);
		}
	// update wall data
	if (m_pWall [0]->m_info.nTrigger == NO_TRIGGER)
		sprintf_s (m_szMsg, sizeof (m_szMsg), "cube = %ld, side = %ld, no trigger", m_pWall [0]->m_nSegment, m_pWall [0]->m_nSide);
	else
		sprintf_s (m_szMsg, sizeof (m_szMsg), "cube = %ld, side = %ld, trigger= %d", m_pWall [0]->m_nSegment, m_pWall [0]->m_nSide, (INT32)m_pWall [0]->m_info.nTrigger);

	m_nWall [0] = INT32 (m_pWall [0] - theMine->Walls (0));
	GetOtherWall ();
	m_nSegment = m_pWall [0]->m_nSegment;
	m_nSide = m_pWall [0]->m_nSide + 1;
	m_nTrigger = (m_pWall [0]->m_info.nTrigger < theMine->GameInfo ().triggers.count) ? m_pWall [0]->m_info.nTrigger : -1;
	m_nType = m_pWall [0]->m_info.type;
	m_nClip = m_pWall [0]->m_info.nClip;
	m_nStrength = ((double) m_pWall [0]->m_info.hps) / F1_0;
	if (m_bFlyThrough = (m_nStrength < 0))
		m_nStrength = -m_nStrength;
	m_nCloak = ((double) (m_pWall [0]->m_info.cloakValue % 32)) * 100.0 / 31.0;
	CBWallNo ()->SetCurSel (m_nWall [0]);
	//CBType ()->SetCurSel (m_nType);
	SelectItemData (CBType (), m_nType);
	CBClipNo ()->EnableWindow ((m_nType == WALL_BLASTABLE) || (m_nType == WALL_DOOR));
	// select list box index for clip
	INT32 i;
	for (i = 0; i < D2_NUM_OF_CLIPS; i++)
		if (clipList [i] == m_nClip)
			break;
	m_nClip = i;
	CBClipNo ()->SetCurSel ((i < D2_NUM_OF_CLIPS) ? i : 0);
	for (i = 0; i < MAX_WALL_FLAGS; i++)
		m_bFlags [i] = ((m_pWall [0]->m_info.flags & wall_flags [i]) != 0);
	for (i = 0; i < 4; i++)
		m_bKeys [i] = ((m_pWall [0]->m_info.keys & (1 << i)) != 0);
	if (!m_bLock) {
		m_defWall = *m_pWall [0];
		i = theMine->Segments (m_defWall.m_nSegment)->m_sides [m_defWall.m_nSide].m_info.nBaseTex;
		if (m_defWall.m_info.type == WALL_CLOAKED)
			m_defOvlTexture = i;
		else
			m_defTexture = i;
		}
	if (m_nType == WALL_DOOR) {
		memcpy (&m_defDoor, &m_defWall, sizeof (m_defDoor));
		m_defDoorTexture = m_defTexture;
		}
   }
GetDlgItem (IDC_WALL_BOTHSIDES)->EnableWindow (TRUE);
GetDlgItem (IDC_WALL_OTHERSIDE)->EnableWindow (TRUE);
GetDlgItem (IDC_WALL_WALLNO)->EnableWindow (theMine->GameInfo ().walls.count > 0);
CTexToolDlg::Refresh ();
CToolDlg::EnableControls (IDC_WALL_DELETEALL, IDC_WALL_DELETEALL, theMine->GameInfo ().walls.count > 0);
CBWallNo ()->SetCurSel (m_nWall [0]);
UpdateData (FALSE);
}

//------------------------------------------------------------------------
// CWallTool - Add Wall
//------------------------------------------------------------------------

void CWallTool::OnAddWall ()
{

CWall *wallP;
CSegment *segP [2];
CSide *sideP [2];
INT16 nSegment [2]; 
INT16 nSide [2];

bool bRefresh = false;

m_bDelayRefresh = true;
segP [0] = theMine->CurrSeg ();
sideP [0] = theMine->CurrSide ();
nSegment [0] = theMine->Current ()->nSegment;
nSide [0] = theMine->Current ()->nSide;
if (theMine->GetOppositeSide (nSegment [1], nSide [1], nSegment [0], nSide [0])) {
	segP [1] = theMine->Segments (nSegment [1]);
	sideP [1] = segP [1]->m_sides + nSide [1];
	}

for (BOOL bSide = FALSE; bSide <= m_bBothSides; bSide++)
	if (sideP [bSide]->m_info.nWall < theMine->GameInfo ().walls.count)
		ErrorMsg ("There is already a wall at that side of the current cube.");
	else if (theMine->GameInfo ().walls.count >= MAX_WALLS)
		ErrorMsg ("The maximum number of walls is already reached.");
	else {
		if ((theApp.IsD2File ()) && (segP [bSide]->Child (nSide [bSide]) == -1))
			theMine->AddWall (-1, -1, WALL_OVERLAY, 0, KEY_NONE, -2, m_defOvlTexture);
		else if (wallP = theMine->AddWall (nSegment [bSide], nSide [bSide], m_defWall.m_info.type, m_defWall.m_info.flags, 
													m_defWall.m_info.keys, m_defWall.m_info.nClip, m_defTexture)) {
			if (wallP->m_info.type == m_defWall.m_info.type) {
				wallP->m_info.hps = m_defWall.m_info.hps;
				wallP->m_info.cloakValue = m_defWall.m_info.cloakValue;
				}
			else if (wallP->m_info.type == WALL_CLOAKED) {
				wallP->m_info.hps = 0;
				wallP->m_info.cloakValue = 16;
				}
			else {
				wallP->m_info.hps = 0;
				wallP->m_info.cloakValue = 31;
				}
			}
			// update main window
		bRefresh = true;
		}
m_bDelayRefresh = false;
if (bRefresh) {
	theApp.MineView ()->Refresh ();
	Refresh ();
	}
}

//------------------------------------------------------------------------
// CWallTool - Delete Wall
//------------------------------------------------------------------------

void CWallTool::OnDeleteWall () 
{
	bool bRefresh = false;
	INT16 nWall;

GetWalls ();
for (BOOL bSide = FALSE; bSide <= m_bBothSides; bSide++) {
	nWall = m_nWall [bSide];
	if (bSide && (m_nWall [1] > m_nWall [0]))
		nWall--;
	if (nWall >= 0) {
		m_bDelayRefresh = true;
		theMine->DeleteWall ((UINT16) nWall);
		m_bDelayRefresh = false;
		bRefresh = true;
		}
	else if (!bExpertMode)
		if (theMine->GameInfo ().walls.count == 0)
			ErrorMsg ("There are no walls in this mine.");
		else
			ErrorMsg ("There is no wall at this side of the current cube.");
	}
if (bRefresh) {
	theApp.MineView ()->Refresh ();
	Refresh ();
	}
}

//------------------------------------------------------------------------
// CWallTool - Delete WallAll
//------------------------------------------------------------------------

void CWallTool::OnDeleteWallAll () 
{
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
CSegment *segP = theMine->Segments (0);
CSide *sideP;
bool bAll = (theMine->MarkedSegmentCount (true) == 0);
INT32 i, j, nDeleted = 0;
for (i = theMine->SegCount (); i; i--, segP++) {
	sideP = segP->m_sides;
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++, sideP++) {
		if (sideP->m_info.nWall >= MAX_WALLS)
			continue;
		if (bAll || theMine->SideIsMarked (i, j)) {
			theMine->DeleteWall (sideP->m_info.nWall);
			nDeleted++;
			}
		}
	}
theApp.MineView ()->DelayRefresh (false);
if (nDeleted) {
	theApp.UnlockUndo ();
	theApp.MineView ()->Refresh ();
	Refresh ();
	}
else
	theApp.ResetModified (bUndo);
}

//------------------------------------------------------------------------
// CWallTool - Other Side
//------------------------------------------------------------------------

void CWallTool::OnOtherSide () 
{
theApp.MineView ()->SelectOtherSide ();
}

                        /*--------------------------*/

CWall *CWallTool::GetOtherWall (void)
{
INT16 nOppSeg, nOppSide;

if (!theMine->GetOppositeSide (nOppSeg, nOppSide))
	return m_pWall [1] = NULL;
m_nWall [1] = theMine->Segments (nOppSeg)->m_sides [nOppSide].m_info.nWall;
return m_pWall [1] = (m_nWall [1] < theMine->GameInfo ().walls.count ? theMine->Walls (m_nWall [1]) : NULL);
}

                        /*--------------------------*/

bool CWallTool::GetWalls ()
{
m_nWall [0] = CBWallNo ()->GetCurSel ();
if (m_nWall [0] < 0) {
	m_nWall [1] = -1;
	m_pWall [0] =
	m_pWall [1] = NULL;
	return false;
	}
m_pWall [0] = theMine->Walls (m_nWall [0]);
theMine->SetCurrent (m_pWall [0]->m_nSegment, m_pWall [0]->m_nSide);
m_nTrigger = m_pWall [0]->m_info.nTrigger;
GetOtherWall ();
return true;
}

                        /*--------------------------*/

void CWallTool::OnSetWall ()
{
if (GetWalls ())
	theApp.MineView ()->Refresh ();
Refresh ();
}

                        /*--------------------------*/

void CWallTool::OnSetType ()
{
	CSegment	*segP [2];
	CSide		*sideP [2];
	CWall		*wallP;
	INT16			nSegment [2], nSide [2];
	INT32			nType;

GetWalls ();
nType = INT32 (CBType ()->GetItemData (CBType ()->GetCurSel ()));
if ((nType > WALL_CLOSED) && theApp.IsD1File ()) 
	return;
if ((nType > WALL_CLOAKED) && (theMine->IsStdLevel ())) 
	return;

m_defWall.m_info.type = m_nType = nType;
/*
m_nWall [0] = CBWallNo ()->GetCurSel ();
m_pWall [0] = theMine->Walls (m_nWall [0]);
*/
segP [0] = theMine->CurrSeg ();
sideP [0] = theMine->CurrSide ();
nSegment [0] = theMine->Current ()->nSegment;
nSide [0] = theMine->Current ()->nSide;
if (theMine->GetOppositeSide (nSegment [1], nSide [1], nSegment [0], nSide [0])) {
	segP [1] = theMine->Segments (nSegment [1]);
	sideP [1] = segP [1]->m_sides + nSide [1];
	}
for (BOOL bSide = FALSE; bSide <= m_bBothSides; bSide++)
	if ((wallP = m_pWall [bSide]) && sideP [bSide]) {
		INT16 nBaseTex  = sideP [bSide]->m_info.nBaseTex;
		INT16 nOvlTex = sideP [bSide]->m_info.nOvlTex;
		theMine->DefineWall (nSegment [bSide], nSide [bSide], m_nWall [bSide], m_nType, m_pWall [0]->m_info.nClip, -1, true);
		if ((wallP->m_info.type == WALL_OPEN) || (wallP->m_info.type == WALL_CLOSED))
			theMine->SetTexture (wallP->m_nSegment, wallP->m_nSide, nBaseTex, nOvlTex);
//		else if ((wallP->m_info.type == WALL_CLOAKED) || (wallP->m_info.type == WALL_TRANSPARENT))
//			wallP->m_info.cloakValue = m_defWall.cloakValue;
		}
theApp.MineView ()->Refresh ();
Refresh ();
}

                        /*--------------------------*/

void CWallTool::OnSetClip ()
{
	INT32		nClip;
	CWall	*wallP;
/*
m_nWall [0] = CBWallNo ()->GetCurSel ();
m_pWall [0] = theMine->Walls () + m_nWall [0];
*/
GetWalls ();
m_nClip = CBClipNo ()->GetCurSel ();
for (BOOL bSide = FALSE; bSide <= m_bBothSides; bSide++)
	if (wallP = m_pWall [bSide])
		if ((wallP->m_info.type == WALL_BLASTABLE) || (wallP->m_info.type == WALL_DOOR)) {
			if (m_nWall [bSide] < theMine->GameInfo ().walls.count) {
				theApp.SetModified (TRUE);
				theApp.LockUndo ();
				nClip = clipList [m_nClip];
				wallP->m_info.nClip = nClip;
				// define door textures based on clip number
				if (wallP->m_info.nClip >= 0)
					theMine->SetWallTextures (m_nWall [bSide], m_defTexture);
				theApp.UnlockUndo ();
				theApp.MineView ()->Refresh ();
				Refresh ();
				}
			}
		else
			wallP->m_info.nClip = -1;
}

                        /*--------------------------*/

void CWallTool::OnKey (INT32 i) 
{
GetWalls ();
memset (m_bKeys, 0, sizeof (m_bKeys));
m_bKeys [i] = TRUE;
for (BOOL bSide = FALSE; bSide <= m_bBothSides; bSide++)
	if (m_pWall [bSide]) {
		theApp.SetModified (TRUE);
		m_pWall [bSide]->m_info.keys = (1 << i);
		Refresh ();
		}
}

void CWallTool::OnFlag (INT32 i) 
{
GetWalls ();
m_bFlags [i] = BtnCtrl (IDC_WALL_BLASTED + i)->GetCheck ();
for (BOOL bSide = FALSE; bSide <= m_bBothSides; bSide++)
	if (m_pWall [bSide]) {
		theApp.SetModified (TRUE);
		if (m_bFlags [i])
			m_pWall [bSide]->m_info.flags |= wall_flags [i];
		else
			m_pWall [bSide]->m_info.flags &= ~wall_flags [i];
		Refresh ();
		}
}

void CWallTool::OnNoKey () { OnKey (0); }
void CWallTool::OnBlueKey () { OnKey (1); }
void CWallTool::OnGoldKey () { OnKey (2); }
void CWallTool::OnRedKey () { OnKey (3); }

void CWallTool::OnBlasted () { OnFlag (0); }
void CWallTool::OnDoorOpen () { OnFlag (1); }
void CWallTool::OnDoorLocked () { OnFlag (2); }
void CWallTool::OnDoorAuto () { OnFlag (3); }
void CWallTool::OnIllusionOff () { OnFlag (4); }
void CWallTool::OnSwitch () { OnFlag (5); }
void CWallTool::OnBuddyProof () { OnFlag (6); }
void CWallTool::OnRenderAdditive () { OnFlag (7); }
void CWallTool::OnIgnoreMarker () { OnFlag (8); }

                        /*--------------------------*/

void CWallTool::OnStrength ()
{
for (BOOL bSide = FALSE; bSide <= m_bBothSides; bSide++)
	if (m_pWall [bSide]) {
		UpdateData (TRUE);
		theApp.SetModified (TRUE);
		m_pWall [bSide]->m_info.hps = (FIX) m_nStrength * F1_0;
		if ((m_pWall [bSide]->m_info.type == WALL_TRANSPARENT) && m_bFlyThrough)
			m_pWall [bSide]->m_info.hps = -m_pWall [bSide]->m_info.hps;
		}
}

                        /*--------------------------*/

void CWallTool::OnCloak ()
{
for (BOOL bSide = FALSE; bSide <= m_bBothSides; bSide++)
	if (m_pWall [bSide]) {
		UpdateData (TRUE);
		theApp.SetModified (TRUE);
		m_defWall.m_info.cloakValue =
		m_pWall [bSide]->m_info.cloakValue = (INT8) (m_nCloak * 31.0 / 100.0) % 32;
		}
	else
		INFOMSG ("wall not found");
}

                        /*--------------------------*/

void CWallTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
	INT32	nPos = pScrollBar->GetScrollPos ();
	CRect	rc;

if (!m_pWall [0])
	return;
if (pScrollBar == TransparencySlider ()) {
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= 10;
			break;
		case SB_PAGEDOWN:
			nPos += 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > 10)
		nPos = 10;
	if  (m_pWall [0]->m_info.type == WALL_TRANSPARENT) {
		m_nStrength = nPos * 10;
		UpdateData (FALSE);
		OnStrength ();
		}
	else {
		m_nCloak = nPos * 10;
		UpdateData (FALSE);
		OnCloak ();
		}
//	pScrollBar->SetScrollPos (nPos, TRUE);
	}
pScrollBar->SetScrollPos (nPos, TRUE);
}

                        /*--------------------------*/

void CWallTool::OnAddDoorNormal ()
{
theMine->AddAutoDoor (m_defDoor.m_info.nClip, m_defDoorTexture);
}

void CWallTool::OnAddDoorExit ()
{
theMine->AddNormalExit ();
}

void CWallTool::OnAddDoorSecretExit ()
{
theMine->AddSecretExit ();
}

void CWallTool::OnAddDoorPrison ()
{
theMine->AddPrisonDoor ();
}

void CWallTool::OnAddDoorGuideBot ()
{
theMine->AddGuideBotDoor ();
}

void CWallTool::OnAddWallFuelCell ()
{
theMine->AddFuelCell ();
}

void CWallTool::OnAddWallIllusion ()
{
theMine->AddIllusionaryWall ();
}

void CWallTool::OnAddWallForceField ()
{
theMine->AddForceField ();
}

void CWallTool::OnAddWallFan ()
{
theMine->AddFan ();
}

void CWallTool::OnAddWallGrate ()
{
theMine->AddGrate ();
}

void CWallTool::OnAddWallWaterfall ()
{
theMine->AddWaterFall ();
}

void CWallTool::OnAddWallLavafall ()
{
theMine->AddLavaFall ();
}

void CWallTool::OnBothSides ()
{
m_bBothSides = BtnCtrl (IDC_WALL_BOTHSIDES)->GetCheck ();
}

//eof walldlg.cpp