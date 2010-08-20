// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "toolview.h"

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CTriggerTool, CTexToolDlg)
	ON_WM_PAINT ()
	ON_WM_HSCROLL ()
	ON_BN_CLICKED (IDC_TRIGGER_ADD, OnAddTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_DELETE, OnDeleteTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_DELETEALL, OnDeleteTriggerAll)
	ON_BN_CLICKED (IDC_TRIGGER_ADDTGT, OnAddTarget)
	ON_BN_CLICKED (IDC_TRIGGER_DELTGT, OnDeleteTarget)
	ON_BN_CLICKED (IDC_TRIGGER_ADDWALLTGT, OnAddWallTarget)
	ON_BN_CLICKED (IDC_TRIGGER_ADDOBJTGT, OnAddObjTarget)
	ON_BN_CLICKED (IDC_TRIGGER_COPY, OnCopyTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_PASTE, OnPasteTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_STANDARD, OnStandardTrigger)
	ON_BN_CLICKED (IDC_TRIGGER_OBJECT, OnObjectTrigger)

	ON_BN_CLICKED (IDC_TRIGGER_NOMESSAGE, OnD2Flag1)
	ON_BN_CLICKED (IDC_TRIGGER_ONESHOT, OnD2Flag2)
	ON_BN_CLICKED (IDC_TRIGGER_PERMANENT, OnD2Flag3)
	ON_BN_CLICKED (IDC_TRIGGER_ALTERNATE, OnD2Flag4)
	ON_BN_CLICKED (IDC_TRIGGER_SET_ORIENT, OnD2Flag5)
	ON_BN_CLICKED (IDC_TRIGGER_AUTOPLAY, OnD2Flag6)
	ON_BN_CLICKED (IDC_TRIGGER_SILENT, OnD2Flag7)

	ON_BN_CLICKED (IDC_TRIGGER_CONTROLDOORS, OnD1Flag1)
	ON_BN_CLICKED (IDC_TRIGGER_SHIELDDRAIN, OnD1Flag2)
	ON_BN_CLICKED (IDC_TRIGGER_ENERGYDRAIN, OnD1Flag3)
	ON_BN_CLICKED (IDC_TRIGGER_ENDLEVEL, OnD1Flag4)
	ON_BN_CLICKED (IDC_TRIGGER_SECRETEXIT, OnD1Flag5)
	ON_BN_CLICKED (IDC_TRIGGER_ACTIVE, OnD1Flag6)
	ON_BN_CLICKED (IDC_TRIGGER_ONESHOTD1, OnD1Flag7)
	ON_BN_CLICKED (IDC_TRIGGER_ROBOTMAKER, OnD1Flag8)
	ON_BN_CLICKED (IDC_TRIGGER_ILLUSIONOFF, OnD1Flag9)
	ON_BN_CLICKED (IDC_TRIGGER_ILLUSIONON, OnD1Flag10)
	ON_BN_CLICKED (IDC_TRIGGER_OPENWALL, OnD1Flag11)
	ON_BN_CLICKED (IDC_TRIGGER_CLOSEWALL, OnD1Flag12)

	ON_BN_CLICKED (IDC_TRIGGER_ADD_OPENDOOR, OnAddOpenDoor)
	ON_BN_CLICKED (IDC_TRIGGER_ADD_ROBOTMAKER, OnAddRobotMaker)
	ON_BN_CLICKED (IDC_TRIGGER_ADD_SHIELDDRAIN, OnAddShieldDrain)
	ON_BN_CLICKED (IDC_TRIGGER_ADD_ENERGYDRAIN, OnAddEnergyDrain)
	ON_BN_CLICKED (IDC_TRIGGER_ADD_CONTROLPANEL, OnAddControlPanel)
	ON_CBN_SELCHANGE (IDC_TRIGGER_TRIGGERNO, OnSetTrigger)
	ON_CBN_SELCHANGE (IDC_TRIGGER_D2TYPE, OnSetType)
	ON_CBN_SELCHANGE (IDC_TRIGGER_TARGETLIST, OnSetTarget)
	ON_CBN_SELCHANGE (IDC_TRIGGER_TEXTURE1, OnSelect1st)
	ON_CBN_SELCHANGE (IDC_TRIGGER_TEXTURE2, OnSelect2nd)
	ON_EN_KILLFOCUS (IDC_TRIGGER_STRENGTH, OnStrength)
	ON_EN_KILLFOCUS (IDC_TRIGGER_TIME, OnTime)
	//ON_EN_UPDATE (IDC_TRIGGER_STRENGTH, OnStrength)
	//ON_EN_UPDATE (IDC_TRIGGER_TIME, OnTime)
END_MESSAGE_MAP ()

static INT16 triggerFlagsD1 [MAX_TRIGGER_FLAGS] = {
	TRIGGER_CONTROL_DOORS,
	TRIGGER_SHIELD_DAMAGE,
	TRIGGER_ENERGY_DRAIN,
	TRIGGER_EXIT,
	TRIGGER_SECRET_EXIT,
	TRIGGER_ON,
	TRIGGER_ONE_SHOT,
	TRIGGER_MATCEN,
	TRIGGER_ILLUSION_OFF,
	TRIGGER_ILLUSION_ON,
	TRIGGER_OPEN_WALL,
	TRIGGER_CLOSE_WALL
	};

static INT32 d2FlagXlat [] = {0, 1, 3, 4, 5, 7, 6};

//------------------------------------------------------------------------
// DIALOG - CTriggerTool (constructor)
//------------------------------------------------------------------------

CTriggerTool::CTriggerTool (CPropertySheet *pParent)
	: CTexToolDlg (nLayout ? IDD_TRIGGERDATA2 : IDD_TRIGGERDATA, pParent, IDC_TRIGGER_SHOW, 6, RGB (0,0,0), true)
{
Reset ();
}

								/*--------------------------*/

CTriggerTool::~CTriggerTool ()
{
if (m_bInited) {
	m_showObjWnd.DestroyWindow ();
	m_showTexWnd.DestroyWindow ();
	}
}

                        /*--------------------------*/

void CTriggerTool::LoadTextureListBoxes () 
{
	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	char			name [80];
	INT32			nTextures, iTexture, index;
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();

INT16 texture1 = Texture1 ();
INT16 texture2 = Texture2 ();

if ((texture1 < 0) || (texture1 >= MAX_TEXTURES))
	texture1 = 0;
if ((texture2 < 0) || (texture2 >= MAX_TEXTURES))
	texture2 = 0;

cbTexture1->ResetContent ();
cbTexture2->ResetContent ();
index = cbTexture1->AddString ("(none)");
texture_resource = (theApp.IsD1File ()) ? D1_TEXTURE_STRING_TABLE : D2_TEXTURE_STRING_TABLE;
nTextures = (theApp.IsD1File ()) ? MAX_D1_TEXTURES : MAX_D2_TEXTURES;
for (iTexture = 0; iTexture < nTextures; iTexture++) {
#if 0
	if (iTexture >= 910)
		sprintf (name, "xtra #%d", iTexture);
	else
#endif
		LoadString (hInst, texture_resource + iTexture, name, sizeof (name));
	if (!strstr ((char *) name, "frame")) {
		index = cbTexture1->AddString (name);
		cbTexture1->SetItemData (index, iTexture);
		if (texture1 == iTexture)
			cbTexture1->SetCurSel (index);
		index = cbTexture2->AddString (iTexture ? name : "(none)");
		if (texture2 == iTexture)
			cbTexture2->SetCurSel (index);
		cbTexture2->SetItemData (index, iTexture);
		}
	}
}

								/*--------------------------*/

void CTriggerTool::Reset ()
{
m_nTrigger = 
m_nStdTrigger = 
m_nObjTrigger = -1;
m_nClass = 0;
m_nType = 0;
m_nStrength = 0;
m_nTime = 0;
m_bAutoAddWall = 1;
m_targets = 0;
m_iTarget = -1;
m_nSliderValue = 10;
m_bFindTrigger = true;
memset (m_bD1Flags, 0, sizeof (m_bD1Flags));
memset (m_bD2Flags, 0, sizeof (m_bD2Flags));
*m_szTarget = '\0';
}

                        /*--------------------------*/

typedef struct tTriggerData {
	char	*pszName;
	INT32	nType;
} tTriggerData;

static tTriggerData triggerData [] = {
	{"open door", TT_OPEN_DOOR},
	{"close door", TT_CLOSE_DOOR},
	{"make robots", TT_MATCEN},
	{"exit", TT_EXIT},
	{"secret exit", TT_SECRET_EXIT},
	{"illusion off", TT_ILLUSION_OFF},
	{"illusion on", TT_ILLUSION_ON},
	{"unlock door", TT_UNLOCK_DOOR},
	{"lock door", TT_LOCK_DOOR},
	{"open wall", TT_OPEN_WALL},
	{"close wall", TT_CLOSE_WALL},
	{"illusory wall", TT_ILLUSORY_WALL},
	{"light off", TT_LIGHT_OFF},
	{"light on", TT_LIGHT_ON},
	{"teleport", TT_TELEPORT},
	{"speed boost", TT_SPEEDBOOST},
	{"camera", TT_CAMERA},
	{"damage shields", TT_SHIELD_DAMAGE_D2},
	{"drain energy", TT_ENERGY_DRAIN_D2},
	{"change texture", TT_CHANGE_TEXTURE},
	{"countdown", TT_COUNTDOWN},
	{"spawn bot", TT_SPAWN_BOT},
	{"set spawn", TT_SET_SPAWN},
	{"message", TT_MESSAGE},
	{"sound", TT_SOUND},
	{"master", TT_MASTER},
	{"enable", TT_ENABLE_TRIGGER},
	{"disable", TT_DISABLE_TRIGGER}
	};


BOOL CTriggerTool::OnInitDialog ()
{
CTexToolDlg::OnInitDialog ();
CreateImgWnd (&m_showObjWnd, IDC_TRIGGER_SHOW_OBJ);
CreateImgWnd (&m_showTexWnd, IDC_TRIGGER_SHOW_TEXTURE);
CComboBox *pcb = CBType ();
pcb->ResetContent();
if (theApp.IsD2File ()) {
	INT32 h, i, j = sizeof (triggerData) / sizeof (tTriggerData);
	for (i = 0; i < j; i++) {
		h = pcb->AddString (triggerData [i].pszName);
		pcb->SetItemData (h, triggerData [i].nType);
		}
	}
else
	pcb->AddString ("n/a");
pcb->SetCurSel (0);
InitSlider (IDC_TRIGGER_SLIDER, 1, 10);
INT32 i;
for (i = 1; i <= 10; i++)
	SlCtrl (IDC_TRIGGER_SLIDER)->SetTic (i);
LoadTextureListBoxes ();
m_bInited = TRUE;
return TRUE;
}

								/*--------------------------*/

bool CTriggerTool::TriggerHasSlider (void)
{
return 
	(m_nType == TT_SPEEDBOOST) || 
	(m_nType == TT_TELEPORT) ||
	(m_nType == TT_SPAWN_BOT);
}

								/*--------------------------*/

void CTriggerTool::DoDataExchange (CDataExchange *pDX)
{
	static char *pszSmokeParams [] = {"life", "speed", "density", "volume", "drift", "", "", "brightness"};

if (!(m_bInited && theMine))
	return;
DDX_CBIndex (pDX, IDC_TRIGGER_TRIGGERNO, m_nTrigger);
DDX_CBIndex (pDX, IDC_TRIGGER_D2TYPE, m_nType);
if (pDX->m_bSaveAndValidate)
	m_nType = INT32 (CBType ()->GetItemData (CBType ()->GetCurSel ()));
else
	SelectItemData (CBType (), m_nType);
DDX_Text (pDX, IDC_TRIGGER_TIME, m_nTime);
INT32 i;
for (i = 0; i < 2; i++)
	DDX_Check (pDX, IDC_TRIGGER_NOMESSAGE + i, m_bD2Flags [i]);
for (i = 2; i < 7; i++)
	DDX_Check (pDX, IDC_TRIGGER_NOMESSAGE + i, m_bD2Flags [d2FlagXlat [i]]);
for (i = 0; i < MAX_TRIGGER_FLAGS; i++)
	DDX_Check (pDX, IDC_TRIGGER_CONTROLDOORS + i, m_bD1Flags [i]);
if (TriggerHasSlider () || (m_nType == TT_SHIELD_DAMAGE_D2) || (m_nType == TT_ENERGY_DRAIN_D2))
	DDX_Double (pDX, IDC_TRIGGER_STRENGTH, m_nStrength, -1000, 1000, "%3.1f");
else if ((m_nType == TT_MESSAGE) || (m_nType == TT_SOUND))
	DDX_Double (pDX, IDC_TRIGGER_STRENGTH, m_nStrength, 0, 1000, "%1.0f");
else if ((theApp.IsD1File ()) && (m_bD1Flags [1] || m_bD1Flags [2]))	// D1 shield/energy drain
	DDX_Double (pDX, IDC_TRIGGER_STRENGTH, m_nStrength, -10000, 10000, "%1.0f");
else
	DDX_Double (pDX, IDC_TRIGGER_STRENGTH, m_nStrength, 0, 64, "%1.0f");
DDX_Text (pDX, IDC_TRIGGER_TARGET, m_szTarget, sizeof (m_szTarget));
DDX_Check (pDX, IDC_TRIGGER_AUTOADDWALL, m_bAutoAddWall);
char szLabel [40];
if (m_nType == TT_SPEEDBOOST) {
	DDX_Slider (pDX, IDC_TRIGGER_SLIDER, m_nSliderValue);
	sprintf_s (szLabel, sizeof (szLabel), "boost: %d%c", m_nSliderValue * 10, '%');
	}
else if ((m_nType == TT_TELEPORT) || (m_nType == TT_SPAWN_BOT)) {
	DDX_Slider (pDX, IDC_TRIGGER_SLIDER, m_nSliderValue);
	sprintf_s (szLabel, sizeof (szLabel), "damage: %d%c", m_nSliderValue * 10, '%');
	}
else
	strcpy_s (szLabel, sizeof (szLabel), "n/a");
SetDlgItemText (IDC_TRIGGER_SLIDER_TEXT, szLabel);
DDX_Radio (pDX, IDC_TRIGGER_STANDARD, m_nClass);
if (m_nType == TT_MESSAGE) {
	strcpy_s (szLabel, sizeof (szLabel), "msg #");
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT, szLabel);
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT2, "");
	}
else if (m_nType == TT_EXIT) {
	strcpy_s (szLabel, sizeof (szLabel), "dest:");
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT, szLabel);
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT2, "");
	}
else if (m_nType == TT_SOUND) {
	strcpy_s (szLabel, sizeof (szLabel), "sound #");
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT, szLabel);
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT2, "");
	}
else if (m_nType == TT_MASTER) {
	strcpy_s (szLabel, sizeof (szLabel), "semaphore:");
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT, szLabel);
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT2, "");
	}
else {
	strcpy_s (szLabel, sizeof (szLabel), "strength:");
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT, szLabel);
	SetDlgItemText (IDC_TRIGGER_STRENGTH_TEXT2, "%");
	}
}

								/*--------------------------*/

BOOL CTriggerTool::OnSetActive ()
{
Refresh ();
return TRUE; //CTexToolDlg::OnSetActive ();
}

								/*--------------------------*/

BOOL CTriggerTool::OnKillActive ()
{
Refresh ();
return CTexToolDlg::OnKillActive ();
}

								/*--------------------------*/

void CTriggerTool::EnableControls (BOOL bEnable)
{
CToolDlg::EnableControls (IDC_TRIGGER_STANDARD, IDC_TRIGGER_OBJECT, theMine->LevelVersion () >= 12);
CToolDlg::EnableControls (IDC_TRIGGER_SHOW_OBJ, IDC_TRIGGER_SHOW_OBJ, theMine->LevelVersion () >= 12);
CToolDlg::EnableControls (IDC_TRIGGER_TRIGGERNO + 1, IDC_TRIGGER_ADD_CONTROLPANEL, bEnable);
CToolDlg::EnableControls (IDC_TRIGGER_SLIDER, IDC_TRIGGER_SLIDER, bEnable && TriggerHasSlider ());
CToolDlg::EnableControls (IDC_TRIGGER_STRENGTH, IDC_TRIGGER_STRENGTH, bEnable && (m_nType != TT_SPEEDBOOST) && (m_nType != TT_CHANGE_TEXTURE));
CToolDlg::EnableControls (IDC_TRIGGER_SHOW_TEXTURE, IDC_TRIGGER_TEXTURE2, bEnable && (m_nType == TT_CHANGE_TEXTURE));
//INT32 i;
//for (i = IDC_TRIGGER_TRIGGER_NO; i <= IDC_TRIGGER_PASTE; i++)
//	GetDlgItem (i)->EnableWindow (bEnable);
}

								/*--------------------------*/

INT32 CTriggerTool::NumTriggers ()
{
return m_nClass ? theMine->NumObjTriggers () : theMine->GameInfo ().triggers.count;
}

								/*--------------------------*/

void CTriggerTool::InitCBTriggerNo ()
{
CComboBox *pcb = CBTriggerNo ();
pcb->ResetContent ();
INT32 i, j = NumTriggers ();
for (i = 0; i < j; i++) {
	_itoa_s (i, message, sizeof (message), 10);
	pcb->AddString (message);
	}
pcb->SetCurSel (m_nTrigger);
}

								/*--------------------------*/

void CTriggerTool::InitLBTargets ()
{
CListBox *plb = LBTargets ();
m_iTarget = plb->GetCurSel ();
plb->ResetContent ();
if (m_pTrigger) {
	m_targets = m_pTrigger->m_count;
	INT32 i;
	for (i = 0; i < m_targets ; i++) {
		sprintf_s (m_szTarget, sizeof (m_szTarget), "   %d, %d", m_pTrigger->Segment (i), m_pTrigger->Side (i) + 1);
		plb->AddString (m_szTarget);
		}
	if ((m_iTarget < 0) || (m_iTarget >= m_targets))
		m_iTarget = 0;
	*m_szTarget = '\0';
	}
else
	m_targets =
	m_iTarget = 0;
plb->SetCurSel (m_iTarget);
}

								/*--------------------------*/

void CTriggerTool::SetTriggerPtr (void)
{
if (m_nTrigger == -1) {
	m_pTrigger = NULL;
	m_nStdTrigger = 
	m_nObjTrigger = -1;
	ClearObjWindow ();
	}	
else if (m_nClass) {
	m_pTrigger = theMine->ObjTriggers (m_nTrigger);
	DrawObjectImage ();
	}
else {
	m_pTrigger = theMine->Triggers (m_nTrigger);
	ClearObjWindow ();
	}
}

//------------------------------------------------------------------------

void CTriggerTool::ClearObjWindow (void)
{
CDC *pDC = m_showObjWnd.GetDC ();
if (pDC) {
	CRect rc;
	m_showObjWnd.GetClientRect (rc);
	pDC->FillSolidRect (&rc, IMG_BKCOLOR);
	m_showObjWnd.ReleaseDC (pDC);
	}
}

//------------------------------------------------------------------------

void CTriggerTool::DrawObjectImage ()
{
if (m_nClass) {
	CGameObject *objP = theMine->CurrObj ();
	if ((objP->type == OBJ_ROBOT) || (objP->type == OBJ_CAMBOT) || (objP->type == OBJ_MONSTERBALL) || (objP->type == OBJ_SMOKE))
		theMine->DrawObject (&m_showObjWnd, objP->type, objP->id);
	}
}

//------------------------------------------------------------------------

void CTriggerTool::OnPaint ()
{
CToolDlg::OnPaint ();
DrawObjectImage ();
}

//------------------------------------------------------------------------

bool CTriggerTool::FindTrigger (INT16 &nTrigger)
{
if (!m_bFindTrigger)
	nTrigger = m_nTrigger;
else {
	if (m_nClass) {
		if (theMine->Current ()->nObject == theMine->ObjTriggers (m_nTrigger)->nObject)
			return false;
		for (INT32 i = 0, j = theMine->NumObjTriggers (); j; j--, i++) {
			if (theMine->Current ()->nObject == theMine->ObjTriggers (i)->nObject) {
				m_nTrigger = i;
				return false;
				}
			}
		m_nTrigger = -1;
		return false;
		}
	else {
		// use current side's trigger
		UINT16 nWall = theMine->FindTriggerWall (&nTrigger);
		m_nTrigger = (nTrigger == NO_TRIGGER) ? -1 : nTrigger;
		// if found, proceed
		if ((m_nTrigger == -1) || (nWall >= theMine->GameInfo ().walls.count))
			return false;
		}
	}
return true;
}

//------------------------------------------------------------------------
// CTriggerTool - RefreshData
//------------------------------------------------------------------------

void CTriggerTool::Refresh ()
{
if (!(m_bInited && theMine))
	return;

	INT32			i;
	INT16			nTrigger;
	CComboBox	*cbTexture1 = CBTexture1 ();
	CComboBox	*cbTexture2 = CBTexture2 ();
	CSide		*sideP;

FindTrigger (nTrigger);
InitCBTriggerNo ();
if (m_nClass || (m_nTrigger == -1)) {
	SetTriggerPtr ();
	EnableControls (FALSE);
	CToolDlg::EnableControls (IDC_TRIGGER_STANDARD, IDC_TRIGGER_OBJECT, TRUE);
	CToolDlg::EnableControls (IDC_TRIGGER_ADD_OPENDOOR, IDC_TRIGGER_ADD_CONTROLPANEL, TRUE);
	if (theApp.IsD2File ())
		CToolDlg::EnableControls (IDC_TRIGGER_ADD_SHIELDDRAIN, IDC_TRIGGER_ADD_ENERGYDRAIN, FALSE);
	GetDlgItem (IDC_TRIGGER_ADD)->EnableWindow (TRUE);
	GetDlgItem (IDC_TRIGGER_D2TYPE)->EnableWindow (TRUE);
	InitLBTargets ();
	ClearObjWindow ();
	}
if (m_nTrigger != -1) {
	SetTriggerPtr ();
	m_nType = m_pTrigger->type;
	if (m_nType != TT_CHANGE_TEXTURE) {
		cbTexture1->SetCurSel (cbTexture1->SelectString (-1, "(none)"));  // unselect if string not found
		cbTexture2->SetCurSel (cbTexture2->SelectString (-1, "(none)"));  // unselect if string not found
		}
	else {
		LoadString (hInst, texture_resource + Texture1 (), message, sizeof (message));
		cbTexture1->SetCurSel (cbTexture1->SelectString (-1, message));  // unselect if string not found
		if (Texture2 ()) {
			LoadString (hInst, texture_resource + Texture2 (), message, sizeof (message));
			cbTexture2->SetCurSel (cbTexture2->SelectString (-1, message));  // unselect if string not found
			}
		else
			cbTexture2->SetCurSel (cbTexture2->SelectString (-1, "(none)"));  // unselect if string not found
		}

	EnableControls (TRUE);
	if (!m_nClass)
		GetDlgItem (IDC_TRIGGER_ADD)->EnableWindow (FALSE);
	m_nTime = m_pTrigger->time;
	m_targets = m_pTrigger->m_count;
	InitLBTargets ();
	//TriggerCubeSideList ();
	// if D2 file, use trigger.type
	if (theApp.IsD2File ()) {
		SelectItemData (CBType (), m_nType);
		CToolDlg::EnableControls (IDC_TRIGGER_CONTROLDOORS, IDC_TRIGGER_CLOSEWALL, FALSE);
		CToolDlg::EnableControls (IDC_TRIGGER_ADD_SHIELDDRAIN, IDC_TRIGGER_ADD_ENERGYDRAIN, FALSE);
		m_bD2Flags [0] = ((m_pTrigger->flags & TF_NO_MESSAGE) != 0);
		m_bD2Flags [1] = ((m_pTrigger->flags & TF_ONE_SHOT) != 0);
		m_bD2Flags [2] = 0;
		m_bD2Flags [3] = ((m_pTrigger->flags & TF_PERMANENT) != 0);
		m_bD2Flags [4] = ((m_pTrigger->flags & TF_ALTERNATE) != 0);
		m_bD2Flags [5] = ((m_pTrigger->flags & TF_SET_ORIENT) != 0);
		m_bD2Flags [6] = ((m_pTrigger->flags & TF_SILENT) != 0);
		m_bD2Flags [7] = ((m_pTrigger->flags & TF_AUTOPLAY) != 0);
		if (m_nType == TT_SPEEDBOOST)
			m_nSliderValue = m_pTrigger->value;
		if (m_nType == TT_TELEPORT)
			m_nSliderValue = m_pTrigger->value;
		if (m_nType == TT_SPAWN_BOT)
			m_nSliderValue = m_pTrigger->value;
		else if (m_nType != TT_CHANGE_TEXTURE)
			m_nStrength = (double) m_pTrigger->value / F1_0;
		}
	else {
		CBType ()->EnableWindow (FALSE);
		CToolDlg::EnableControls (IDC_TRIGGER_NOMESSAGE, IDC_TRIGGER_ONESHOT, FALSE);
		for (i = 0; i < MAX_TRIGGER_FLAGS; i++)
			m_bD1Flags [i] = ((m_pTrigger->flags & triggerFlagsD1 [i]) != 0);
		m_nStrength = (double) m_pTrigger->value / F1_0;
		}
	OnSetTarget ();
	}
CToolDlg::EnableControls (IDC_TRIGGER_TRIGGERNO, IDC_TRIGGER_TRIGGERNO, NumTriggers () > 0);
CToolDlg::EnableControls (IDC_TRIGGER_DELETEALL, IDC_TRIGGER_DELETEALL, NumTriggers () > 0);
sideP = theMine->OtherSide ();
CTexToolDlg::Refresh (sideP->nBaseTex, sideP->nOvlTex, 1);
if ((m_nTrigger >= 0) && (m_nType == TT_CHANGE_TEXTURE))
	PaintTexture (&m_showTexWnd, RGB (128,128,128), -1, -1, Texture1 (), Texture2 ());
else
	PaintTexture (&m_showTexWnd, RGB (128,128,128), -1, -1, MAX_TEXTURES);
UpdateData (FALSE);
}

//------------------------------------------------------------------------

void CTriggerTool::OnStandardTrigger (void)
{
m_nObjTrigger = m_nTrigger;
m_nClass = 0;
UpdateData (FALSE);
m_pTrigger = m_pStdTrigger;
m_nTrigger = m_nStdTrigger;
Refresh ();
}

//------------------------------------------------------------------------

void CTriggerTool::OnObjectTrigger (void)
{
m_nStdTrigger = m_nTrigger;
m_pStdTrigger = m_pTrigger;
m_nClass = 1;
UpdateData (FALSE);
m_pTrigger = theMine->ObjTriggers (m_nTrigger);
m_nTrigger = m_nObjTrigger;
Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - Add Trigger
//------------------------------------------------------------------------

void CTriggerTool::OnAddTrigger ()
{
//m_nTrigger = nTrigger;
m_bAutoAddWall = ((CButton *) GetDlgItem (IDC_TRIGGER_AUTOADDWALL))->GetCheck ();
if (m_nClass) {
	m_pTrigger = theMine->AddObjTrigger (-1, m_nType);
	m_nTrigger = m_pTrigger ? INT32 (m_pTrigger - theMine->ObjTriggers (0)) : -1; 
	}
else {
	m_pTrigger = theMine->AddTrigger (-1, m_nType, (BOOL) m_bAutoAddWall /*TT_OPEN_DOOR*/);
	m_nTrigger = m_pTrigger ? INT32 (m_pTrigger - theMine->Triggers (0)) : -1;
	}
// Redraw trigger window
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - Delete Trigger
//------------------------------------------------------------------------

void CTriggerTool::OnDeleteTrigger () 
{
// check to see if trigger already exists on wall
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nClass)
	theMine->DeleteObjTrigger (m_nTrigger);
else
	theMine->DeleteTrigger (m_nTrigger);
// Redraw trigger window
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - Delete All (Marked) Triggers
//------------------------------------------------------------------------

void CTriggerTool::OnDeleteTriggerAll () 
{
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
CSegment *segP = theMine->Segments (0);
CSide *sideP;
bool bAll = (theMine->MarkedSegmentCount (true) == 0);
INT32 i, j, nDeleted = 0;
for (i = theMine->SegCount (); i; i--, segP++) {
	sideP = segP->sides;
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++, sideP++) {
		if (sideP->nWall >= MAX_WALLS)
			continue;
		CWall *wallP = theMine->Walls (sideP->nWall);
		if (wallP->nTrigger >= NumTriggers ())
			continue;
		if (bAll || theMine->SideIsMarked (i, j)) {
			theMine->DeleteTrigger (wallP->nTrigger);
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
// CTriggerTool - TrigNumberMsg
//------------------------------------------------------------------------

void CTriggerTool::OnSetTrigger ()
{
UINT16 nWall;
CWall *wallP;

// find first wall with this trigger
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if ((m_nTrigger == -1) || (m_nTrigger >= NumTriggers ()))
	return;
if (m_nClass) {
	theMine->Current ()->nObject = theMine->ObjTriggers (m_nTrigger)->nObject;
	}
else {
	for (nWall = 0, wallP = theMine->Walls (0); nWall < theMine->GameInfo ().walls.count; nWall++, wallP++)
		if (wallP->nTrigger == m_nTrigger)
			break;
	if (nWall >= theMine->GameInfo ().walls.count) {
		EnableControls (FALSE);
		GetDlgItem (IDC_TRIGGER_DELETE)->EnableWindow (TRUE);
		return;
		}
	if ((wallP->m_nSegment >= theMine->SegCount ()) || (wallP->m_nSegment < 0) || 
		 (wallP->m_nSide < 0) || (wallP->m_nSide > 5)) {
		EnableControls (FALSE);
		GetDlgItem (IDC_TRIGGER_DELETE)->EnableWindow (TRUE);
		return;
		}
	if ((theMine->Current ()->nSegment != wallP->m_nSegment) ||
		 (theMine->Current ()->nSide != wallP->m_nSide)) {
		theMine->SetCurrent (wallP->m_nSegment, wallP->m_nSide);
		}
	}
SetTriggerPtr ();
m_bFindTrigger = false;
Refresh ();
m_bFindTrigger = true;
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - TrigTypeMsg
//------------------------------------------------------------------------

void CTriggerTool::OnSetType ()
{
INT32 nType = INT32 (CBType ()->GetItemData (CBType ()->GetCurSel ()));
if ((nType == TT_SMOKE_BRIGHTNESS) || ((nType >= TT_SMOKE_LIFE) && (nType <= TT_SMOKE_DRIFT))) {
	ErrorMsg ("This trigger type is not supported any more.\nYou can use the effects tool to edit smoke emitters.");
	return;
	}
if ((nType >= TT_TELEPORT) && (theMine->IsStdLevel ())) {
	SelectItemData (CBType (), m_nType);
	return;
	}
m_nType = nType;
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
theApp.SetModified (TRUE);
m_pTrigger->type = m_nType;
Refresh ();
}

//------------------------------------------------------------------------
// CTriggerTool - TrigValueMsg
//------------------------------------------------------------------------

void CTriggerTool::OnStrength () 
{
UpdateData (TRUE);
if ((m_nTrigger == -1) || (m_nType == TT_SPEEDBOOST) || (m_nType == TT_CHANGE_TEXTURE))
	return;
SetTriggerPtr ();
theApp.SetModified (TRUE);
UpdateData (FALSE);
m_pTrigger->value = (INT32) (m_nStrength * F1_0);
}

//------------------------------------------------------------------------
// CTriggerTool - TrigTimeMsg
//------------------------------------------------------------------------

void CTriggerTool::OnTime () 
{
UpdateData (TRUE);
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
theApp.SetModified (TRUE);
m_pTrigger->time = m_nTime;
}

//------------------------------------------------------------------------
// CTriggerTool - TriggerFlags0Msg
//------------------------------------------------------------------------

bool CTriggerTool::OnD1Flag (INT32 i, INT32 j)
{
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return false;
SetTriggerPtr ();
theApp.SetModified (TRUE);
if ((m_bD1Flags [i] = !m_bD1Flags [i]))
//if ((m_bD1Flags [i] = ((CButton *) GetDlgItem (IDC_TRIGGER_CONTROLDOORS + j))->GetCheck ()))
	m_pTrigger->flags |= triggerFlagsD1 [i];
else
	m_pTrigger->flags &= ~triggerFlagsD1 [i];
((CButton *) GetDlgItem (IDC_TRIGGER_CONTROLDOORS + i))->SetCheck (m_bD1Flags [i]);
if (m_bD1Flags [i] && (j >= 0)) {
	m_bD1Flags [j] = 0;
	m_pTrigger->flags &= ~triggerFlagsD1 [j];
	((CButton *) GetDlgItem (IDC_TRIGGER_CONTROLDOORS + j))->SetCheck (0);
	}
UpdateData (FALSE);
return m_bD1Flags [i] != 0;
}

                        /*--------------------------*/

void CTriggerTool::OnD2Flag (INT32 i, INT32 j)
{
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
theApp.SetModified (TRUE);
j = d2FlagXlat [i];
INT32 h = 1 << j;
m_pTrigger->flags ^= h;
m_bD2Flags [j] = ((m_pTrigger->flags & h) != 0);
((CButton *) GetDlgItem (IDC_TRIGGER_NOMESSAGE + i))->SetCheck (m_bD2Flags [j]);
UpdateData (FALSE);
}

//------------------------------------------------------------------------
// CTriggerTool - TriggerFlags1Msg
//------------------------------------------------------------------------

void CTriggerTool::OnD1Flag1 () { OnD1Flag (0); }
void CTriggerTool::OnD1Flag2 () { OnD1Flag (1); }
void CTriggerTool::OnD1Flag3 () { OnD1Flag (2); }
void CTriggerTool::OnD1Flag4 () { OnD1Flag (3, 4); }
void CTriggerTool::OnD1Flag5 () { OnD1Flag (4, 3); }
void CTriggerTool::OnD1Flag6 () { OnD1Flag (5); }
void CTriggerTool::OnD1Flag7 () { OnD1Flag (6); }
void CTriggerTool::OnD1Flag8 () { OnD1Flag (7); }
void CTriggerTool::OnD1Flag9 () { OnD1Flag (8, 9); }
void CTriggerTool::OnD1Flag10 () { OnD1Flag (9, 8); }
void CTriggerTool::OnD1Flag11 () { OnD1Flag (10, 11); }
void CTriggerTool::OnD1Flag12 () { OnD1Flag (11, 10); }

void CTriggerTool::OnD2Flag1 () { OnD2Flag (0); }
void CTriggerTool::OnD2Flag2 () { OnD2Flag (1); }
// caution: 4 is TF_DISABLED in Descent 2 - do not use here!
void CTriggerTool::OnD2Flag3 () { OnD2Flag (2); }
void CTriggerTool::OnD2Flag4 () { OnD2Flag (3); }
void CTriggerTool::OnD2Flag5 () { OnD2Flag (4); }
void CTriggerTool::OnD2Flag6 () { OnD2Flag (5); }
void CTriggerTool::OnD2Flag7 () { OnD2Flag (6); }

//------------------------------------------------------------------------
// CTriggerTool - Add cube/side to trigger list
//------------------------------------------------------------------------

void CTriggerTool::AddTarget (INT16 nSegment, INT16 nSide) 
{
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
m_targets = m_pTrigger->m_count;
if (m_targets >= MAX_TRIGGER_TARGETS) {
	DEBUGMSG (" Trigger tool: No more targets possible for this trigger.");
	return;
	}
if (FindTarget (nSegment, nSide) > -1) {
	DEBUGMSG (" Trigger tool: Trigger already has this target.");
	return;
	}
theApp.SetModified (TRUE);
m_pTrigger->Add (nSegment, nSide + 1);
sprintf_s (m_szTarget, sizeof (m_szTarget), "   %d,%d", nSegment, nSide);
LBTargets ()->AddString (m_szTarget);
LBTargets ()->SetCurSel (m_targets++);
*m_szTarget = '\0';
Refresh ();
}


                        /*--------------------------*/

void CTriggerTool::OnAddTarget () 
{
INT32 nSegment, nSide;
UpdateData (TRUE);
sscanf_s (m_szTarget, "%d, %d", &nSegment, &nSide);
if (nSegment < 0)
	return;
if (nSide < 0)
	nSide = 0;
else if (nSide > 6)
	nSide = 6;
if (nSegment > ((nSide == 0) ? theMine->ObjCount () : theMine->SegCount ()))
	return;
AddTarget (nSegment, nSide);
}

                        /*--------------------------*/

void CTriggerTool::OnAddWallTarget ()
{
CSelection *other = (theMine->Current () == &theMine->Current1 ()) ? &theMine->Current2 () : &theMine->Current1 ();
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
if ((theApp.IsD1File ()) ? 
	 (m_pTrigger->flags & TRIGGER_MATCEN) != 0 : 
	 (m_pTrigger->type == TT_MATCEN) && 
	 (theMine->Segments (other->nSegment)->function != SEGMENT_FUNC_ROBOTMAKER)) {
	DEBUGMSG (" Trigger tool: Target is no robot maker");
	return;
	}
INT32 i = FindTarget (other->nSegment, other->nSide);
if (i > -1)
	return;
AddTarget (other->nSegment, other->nSide + 1);
}

                        /*--------------------------*/

void CTriggerTool::OnAddObjTarget ()
{
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
SetTriggerPtr ();
AddTarget (theMine->Current ()->nObject, 0);
}

//------------------------------------------------------------------------
// CTriggerTool - Delete cube/side
//------------------------------------------------------------------------

void CTriggerTool::OnDeleteTarget ()
{
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
m_iTarget = LBTargets ()->GetCurSel ();
if ((m_iTarget < 0) || (m_iTarget >= MAX_TRIGGER_TARGETS))
	return;
theApp.SetModified (TRUE);
SetTriggerPtr ();
m_targets = m_pTrigger->Delete (m_iTarget);
LBTargets ()->DeleteString (m_iTarget);
if (m_iTarget >= LBTargets ()->GetCount ())
	m_iTarget--;
LBTargets ()->SetCurSel (m_iTarget);
Refresh ();
}

                        /*--------------------------*/

INT32 CTriggerTool::FindTarget (INT16 nSegment, INT16 nSide)
{
return m_pTrigger->Find (nSegment, nSide);
}

//------------------------------------------------------------------------
// CTriggerTool - Cube/Side list box message
//
// sets "other cube" to selected item
//------------------------------------------------------------------------

void CTriggerTool::OnSetTarget () 
{
INT16 nTrigger;
if (!FindTrigger (nTrigger))
	return;
SetTriggerPtr ();
// get affected cube/side list box index
m_iTarget = LBTargets ()->GetCurSel ();
// if selected and within range, then set "other" cube/side
if ((m_iTarget < 0) || (m_iTarget >= MAX_TRIGGER_TARGETS) || (m_iTarget >= m_pTrigger->m_count))
	return;
INT16 nSegment = m_pTrigger->Segment (m_iTarget);
if ((nSegment < 0) || (nSegment >= theMine->SegCount ()))
	 return;
INT16 nSide = m_pTrigger->Side (m_iTarget);
if ((nSide < 0) || (nSide > 5))
	return;

CSelection *other = theMine->Other ();
if ((theMine->Current ()->nSegment == nSegment) && (theMine->Current ()->nSide == nSide))
	return;
other->nSegment = m_pTrigger->Segment (m_iTarget);
other->nSide = m_pTrigger->Side (m_iTarget);
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CTriggerTool::OnCopyTrigger ()
{
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
m_defTrigger = theMine->Triggers () [m_nTrigger];
}

                        /*--------------------------*/

void CTriggerTool::OnPasteTrigger ()
{
m_nTrigger = CBTriggerNo ()->GetCurSel ();
if (m_nTrigger == -1)
	return;
theApp.SetModified (TRUE);
theMine->Triggers () [m_nTrigger] = m_defTrigger;
Refresh ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddOpenDoor ()
{
theMine->AddOpenDoorTrigger ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddRobotMaker ()
{
theMine->AddRobotMakerTrigger ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddShieldDrain ()
{
theMine->AddShieldTrigger ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddEnergyDrain ()
{
theMine->AddEnergyTrigger ();
}

                        /*--------------------------*/

afx_msg void CTriggerTool::OnAddControlPanel ()
{
theMine->AddUnlockTrigger ();
}

                        /*--------------------------*/

BOOL CTriggerTool::TextureIsVisible ()
{
return !m_nClass && (m_pTrigger != NULL) && (m_iTarget >= 0) && (m_iTarget < m_pTrigger->m_count);
}

                        /*--------------------------*/

void CTriggerTool::SelectTexture (INT32 nIdC, bool bFirst)
{
	CSide		*sideP = theMine->CurrSide ();
	CComboBox	*pcb = bFirst ? CBTexture1 () : CBTexture2 ();
	INT32			index = pcb->GetCurSel ();
	
if (index <= 0)
	SetTexture (0, 0);
else {
	INT16 texture = (INT16) pcb->GetItemData (index);
	if (bFirst)
		SetTexture (texture, -1);
	else
		SetTexture (-1, texture);
	}
Refresh ();
}

                        /*--------------------------*/

void CTriggerTool::OnSelect1st () 
{
SelectTexture (IDC_TRIGGER_TEXTURE1, true);
}

                        /*--------------------------*/

void CTriggerTool::OnSelect2nd () 
{
SelectTexture (IDC_TRIGGER_TEXTURE2, false);
}

                        /*--------------------------*/

void CTriggerTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
	INT32	nPos = pScrollBar->GetScrollPos ();
	CRect	rc;

if (!m_pTrigger || !TriggerHasSlider ())
	return;
if (pScrollBar == SpeedBoostSlider ()) {
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= 1;
			break;
		case SB_PAGEDOWN:
			nPos += 1;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 1)
		nPos = 1;
	else if (nPos > 10)
		nPos = 10;
	m_nSliderValue = m_pTrigger->value = nPos;
	UpdateData (FALSE);
#if 0
	pScrollBar->SetScrollPos (nPos, TRUE);
	if (!(nPos = pScrollBar->GetScrollPos ())) {
		INT32	h, i, j;
		pScrollBar->GetScrollRange (&i, &j);
		h = i;
		h = j;
		}
#endif
	}
}

                        /*--------------------------*/


//eof triggerdlg.cpp