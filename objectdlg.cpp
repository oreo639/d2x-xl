// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include "afxpriv.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "mine.h"
#include "dle-xp.h"
#include "global.h"
#include "toolview.h"
#include "textures.h"

typedef struct tSliderData {
	INT32	nId;
	INT32	nMin;
	INT32	nMax;
	long	nFactor;
	char	**pszLabels;
} tSliderData;

// list box tables
INT32 model_num_list [N_D2_ROBOT_TYPES] = {
  0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x0f, 0x11,
  0x13, 0x14, 0x15, 0x16, 0x17, 0x19, 0x1a, 0x1c, 0x1d, 0x1f,
  0x21, 0x23, 0x25, 0x27, 0x28, 0x29, 0x2b, 0x2c, 0x2d, 0x2e,
  0x2f, 0x31, 0x32, 0x33, 0x34, 0x36, 0x37, 0x38, 0x3a, 0x3c,
  0x3e, 0x40, 0x41, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x50, 0x52, 0x53, 0x55, 0x56,
  0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c
};

#define MAX_EXP2_VCLIP_NUM_TABLE 4
UINT8 exp2_vclip_num_table [MAX_EXP2_VCLIP_NUM_TABLE] = {
	0x00, 0x03, 0x07, 0x3c
	};

#define MAX_WEAPON_TYPE_TABLE 30
UINT8 weapon_type_table [MAX_WEAPON_TYPE_TABLE] = {
	0x00, 0x05, 0x06, 0x0a, 0x0b, 0x0e, 0x11, 0x14, 0x15, 0x16,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
	0x2e, 0x30, 0x32, 0x34, 0x35, 0x37, 0x39, 0x3a, 0x3c, 0x3d
	};
/*
 1: 5, 6, 10, 11, 14, 17, 20, 21, 22,
10: 24, 25, 26, 27, 28, 41, 42, 43, 44, 45,
20: 46, 48, 50, 52, 53, 55, 57, 58, 60, 61
*/
#define MAX_BEHAVIOR_TABLE 8
UINT8 behavior_table [MAX_BEHAVIOR_TABLE] = {
	AIB_STILL, AIB_NORMAL, AIB_GET_BEHIND, AIB_RUN_FROM, AIB_FOLLOW_PATH, AIB_STATION, AIB_SNIPE
	};

char *szSkills [5] = {
	"Trainee", 
	"Rookie", 
	"Hotshot", 
	"Ace", 
	"Insane"
};

char *pszExplosionIds [] = {"small explosion", "medium explosion", "big explosion", "huge explosion", "red blast"};
INT32 nExplosionIds [] = {7, 58, 0, 60, 106};

INT32 powerupIdStrXlat [MAX_POWERUP_IDS2];

                        /*--------------------------*/

static tSliderData sliderData [] = {
	{IDC_OBJ_SCORE, 0, 600, 50, NULL},
	{IDC_OBJ_STRENGTH, 13, 20, 1, NULL},
	{IDC_OBJ_MASS, 10, 20, 1, NULL},
	{IDC_OBJ_DRAG, 1, 13, -F1_0 / 100, NULL},
	{IDC_OBJ_EBLOBS, 0, 100, 1, NULL},
	{IDC_OBJ_LIGHT, 0, 10, 1, NULL},
	{IDC_OBJ_GLOW, 0, 12, 1, NULL},
	{IDC_OBJ_AIM, 2, 4, -0x40, NULL},
	{IDC_OBJ_SKILL, 0, 4, 1, szSkills},
	{IDC_OBJ_FOV, -10, 10, -F1_0 / 10, NULL},
	{IDC_OBJ_FIREWAIT1, 1, 35, -F1_0 / 5, NULL},
	{IDC_OBJ_FIREWAIT2, 1, 35, -F1_0 / 5, NULL},
	{IDC_OBJ_TURNTIME, 0, 10, -F1_0 / 10, NULL},
	{IDC_OBJ_MAXSPEED, 0, 140, -F1_0, NULL},
	{IDC_OBJ_FIRESPEED, 1, 18, 1, NULL},
	{IDC_OBJ_EVADESPEED, 0, 6, 1, NULL},
	{IDC_OBJ_CIRCLEDIST, 0, 0, -F1_0, NULL},
	{IDC_OBJ_DEATHROLL, 0, 10, 1, NULL},
	{IDC_OBJ_EXPLSIZE, 0, 100, 1, NULL},
	{IDC_OBJ_CONT_COUNT, 0, 100, 1, NULL},
	{IDC_OBJ_CONT_PROB, 0, 16, 1, NULL}
	};

                        /*--------------------------*/

FIX fix_exp(INT32 x);
INT32 fix_log(FIX x);

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CObjectTool, CToolDlg)
	ON_WM_HSCROLL ()
	ON_WM_PAINT ()
	ON_BN_CLICKED (IDC_OBJ_ADD, OnAdd)
	ON_BN_CLICKED (IDC_OBJ_DELETE, OnDelete)
	ON_BN_CLICKED (IDC_OBJ_DELETEALL, OnDeleteAll)
	ON_BN_CLICKED (IDC_OBJ_MOVE, OnMove)
	ON_BN_CLICKED (IDC_OBJ_RESET, OnReset)
	ON_BN_CLICKED (IDC_OBJ_DEFAULT, OnDefault)
	ON_BN_CLICKED (IDC_OBJ_ADVANCED, OnAdvanced)
	ON_BN_CLICKED (IDC_OBJ_AI_KAMIKAZE, OnAIKamikaze)
	ON_BN_CLICKED (IDC_OBJ_AI_COMPANION, OnAICompanion)
	ON_BN_CLICKED (IDC_OBJ_AI_THIEF, OnAIThief)
	ON_BN_CLICKED (IDC_OBJ_AI_SMARTBLOBS, OnAISmartBlobs)
	ON_BN_CLICKED (IDC_OBJ_AI_PURSUE, OnAIPursue)
	ON_BN_CLICKED (IDC_OBJ_AI_CHARGE, OnAICharge)
	ON_BN_CLICKED (IDC_OBJ_AI_EDRAIN, OnAIEDrain)
	ON_BN_CLICKED (IDC_OBJ_AI_ENDSLEVEL, OnAIEndsLevel)
	ON_BN_CLICKED (IDC_OBJ_BRIGHT, OnBright)
	ON_BN_CLICKED (IDC_OBJ_CLOAKED, OnCloaked)
	ON_BN_CLICKED (IDC_OBJ_MULTIPLAYER, OnMultiplayer)
	ON_BN_CLICKED (IDC_OBJ_SORT, OnSort)
	ON_CBN_SELCHANGE (IDC_OBJ_OBJNO, OnSetObject)
	ON_CBN_SELCHANGE (IDC_OBJ_TYPE, OnSetObjType)
	ON_CBN_SELCHANGE (IDC_OBJ_ID, OnSetObjId)
	ON_CBN_SELCHANGE (IDC_OBJ_TEXTURE, OnSetTexture)
	ON_CBN_SELCHANGE (IDC_OBJ_SPAWN_TYPE, OnSetSpawnType)
	ON_CBN_SELCHANGE (IDC_OBJ_CONT_TYPE, OnSetContType)
	ON_CBN_SELCHANGE (IDC_OBJ_SPAWN_ID, OnSetSpawnId)
	ON_CBN_SELCHANGE (IDC_OBJ_CONT_TYPE, OnSetContType)
	ON_CBN_SELCHANGE (IDC_OBJ_CONT_ID, OnSetContId)
	ON_CBN_SELCHANGE (IDC_OBJ_AI, OnSetObjAI)
	ON_CBN_SELCHANGE (IDC_OBJ_CLASS_AI, OnSetObjClassAI)
	ON_CBN_SELCHANGE (IDC_OBJ_AI_BOSSTYPE, OnAIBossType)
	ON_CBN_SELCHANGE (IDC_OBJ_TEXTURE, OnSetTexture)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_EXPLODE, OnSetSoundExplode)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_ATTACK, OnSetSoundAttack)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_SEE, OnSetSoundSee)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_CLAW, OnSetSoundClaw)
	ON_CBN_SELCHANGE (IDC_OBJ_SOUND_DEATH, OnSetSoundDeath)
	ON_CBN_SELCHANGE (IDC_OBJ_WEAPON1, OnSetWeapon1)
	ON_CBN_SELCHANGE (IDC_OBJ_WEAPON2, OnSetWeapon2)
	ON_EN_KILLFOCUS (IDC_OBJ_SPAWN_QTY, OnSetSpawnQty)
	ON_EN_UPDATE (IDC_OBJ_SPAWN_QTY, OnSetSpawnQty)
END_MESSAGE_MAP ()

                        /*--------------------------*/

CObjectTool::CObjectTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_OBJECTDATA2: IDD_OBJECTDATA, pParent)
{
//Reset ();
}

                        /*--------------------------*/

CObjectTool::~CObjectTool ()
{
if (m_bInited) {
	m_showObjWnd.DestroyWindow ();
	m_showSpawnWnd.DestroyWindow ();
	m_showTextureWnd.DestroyWindow ();
	}
}

                        /*--------------------------*/

void CObjectTool::UpdateSliders (INT32 i)
{
CWnd *pWnd;
char szPos [10];
char *pszPos;
INT32 min = (i < 0) ? 0: i;
INT32 max = (i < 0) ? sizeof (sliderData) / sizeof (tSliderData): i + 1;
INT32 nPos;
tSliderData *psd = sliderData + min;
for (i = min; i < max; i++, psd++) {
	pWnd = GetDlgItem (psd->nId);
	nPos = ((CSliderCtrl *) pWnd)->GetPos ();
	if (psd->pszLabels)
		pszPos = psd->pszLabels [nPos];
	else {
		if (psd->nFactor > 0)
			nPos *= psd->nFactor;
		sprintf_s (szPos, sizeof (szPos), "%d", (INT32) nPos);
		pszPos = szPos;
		}
	AfxSetWindowText (GetDlgItem (psd->nId + 1)->GetSafeHwnd (), pszPos);
	}
}

                        /*--------------------------*/

void CObjectTool::InitSliders ()
{
INT32 h = sizeof (sliderData) / sizeof (tSliderData);
tSliderData *psd = sliderData;
INT32 i;
for (i = 0; i < h; i++, psd++) {
	InitSlider (psd->nId, psd->nMin, psd->nMax);
	((CSliderCtrl *) GetDlgItem (psd->nId))->SetPos (psd->nMin);
	}
}

                        /*--------------------------*/

void CObjectTool::CBInit (CComboBox *pcb, char** pszNames, UINT8 *pIndex, UINT8 *pItemData, INT32 nMax, INT32 nType, bool bAddNone)
{
	INT32 h, j, l;
	HINSTANCE hInst;
	char szLabel [100];
	char *pszLabel;
	DWORD nErr;
	
if (nType & 1) {
	hInst = AfxGetApp()->m_hInstance;
	pszLabel = szLabel;
	}
else if (nType == 2)
	pszLabel = szLabel;
pcb->ResetContent ();
if (bAddNone) {
	j = pcb->AddString ("(none)");
	pcb->SetItemData (j, -1);
	}
INT32 i;
for (i = 0; i < nMax; i++) {
	switch (nType) {
		case 0:
			h = pIndex ? pIndex [i]: i;
			sprintf_s (szLabel, sizeof (szLabel), "%s", pszNames [h]);
			pszLabel = szLabel;
//			pszLabel = pszNames [h];
			break;
		case 1:
			sprintf_s (szLabel, sizeof (szLabel), "%d: ", i);
			l = INT32 (strlen (szLabel));
			LoadString (hInst, INT32 (pszNames) + i, szLabel + l, sizeof (szLabel) - l);
			h = i;
			break;
		case 2:
			sprintf_s (szLabel, sizeof (szLabel), "%s %d", (char *) pszNames, i);
			h = pIndex ? pIndex [i]: i;
			break;
		case 3:
			LoadString (hInst, INT32 (pszNames) + i, szLabel, sizeof (szLabel));
			nErr = GetLastError ();
			h = i;
			break;
		default:
			return;
		}
	if (!strstr (pszLabel, "(not used)")) {
		j = pcb->AddString (pszLabel);
		pcb->SetItemData (j, pItemData ? pItemData [h]: h);
		}
	}
pcb->SetCurSel (0);
}

                        /*--------------------------*/

double CObjectTool::SliderFactor (INT32 nId)
{
INT32 h = sizeof (sliderData) / sizeof (tSliderData);
tSliderData *psd = sliderData;
INT32 i;
for (i = 0; i < h; i++, psd++)
	if (psd->nId == nId)
		return (double) ((psd->nFactor < 0) ? -(psd->nFactor): psd->nFactor);
return 1.0;
}

                        /*--------------------------*/

char *pszBossTypes [] = {"none", "Boss 1", "Boss 2", "Red Fatty", "Water Boss", "Fire Boss", "Ice Boss", "Alien 1", "Alien 2", "Vertigo", "Red Guard", NULL};

BOOL CObjectTool::OnInitDialog ()
{
if (!CToolDlg::OnInitDialog ())
	return FALSE;
CreateImgWnd (&m_showObjWnd, IDC_OBJ_SHOW);
CreateImgWnd (&m_showSpawnWnd, IDC_OBJ_SHOW_SPAWN);
CreateImgWnd (&m_showTextureWnd, IDC_OBJ_SHOW_TEXTURE);
InitSliders ();
UpdateSliders ();
CBInit (CBObjType (), (char**) object_names, object_list, NULL, MAX_OBJECT_NUMBER);
CBInit (CBSpawnType (), (char**) object_names, contentsList, NULL, MAX_CONTAINS_NUMBER, 0, true);
CBInit (CBObjAI (), (char**) ai_options, NULL, behavior_table, (theApp.IsD1File ()) ? MAX_D1_AI_OPTIONS: MAX_D2_AI_OPTIONS);
CBInit (CBObjClassAI (), (char**) ai_options, NULL, behavior_table, (theApp.IsD1File ()) ? MAX_D1_AI_OPTIONS: MAX_D2_AI_OPTIONS);

INT16 nTextures = (theApp.IsD1File ()) ? MAX_D1_TEXTURES: MAX_D2_TEXTURES;
INT16 i, j;
char sz [100], **psz;
HINSTANCE hInst = AfxGetApp()->m_hInstance;
CComboBox *pcb = CBObjTexture ();
pcb->AddString ("(none)");
for (i = 0; i < nTextures; i++) {
	LoadString (hInst, texture_resource + i, sz, sizeof (sz));
	if (!strstr((char *) sz, "frame")) {
		INT32 index = pcb->AddString (sz);
		pcb->SetItemData(index++, i);
		}
	}
for (i = j = 0; i < MAX_POWERUP_IDS; i++) {
	LoadString (hInst, POWERUP_STRING_TABLE + i, sz, sizeof (sz));
	//if (strcmp (sz, "(not used)"))
		powerupIdStrXlat [j++] = i;
	}
pcb = CBBossType ();
pcb->ResetContent ();
for (psz = pszBossTypes; *psz; psz++) {
	INT32 index = pcb->AddString (*psz);
	pcb->SetItemData(index++, (INT32) (psz - pszBossTypes));
	}
CGameObject *objP = theMine->CurrObj ();
//CBInit (CBObjProps (), (char **) ROBOT_STRING_TABLE, NULL, NULL, ROBOT_IDS2, 1);
//SelectItemData (CBObjProps (), (objP->m_info.type == OBJ_ROBOT) && (objP->m_info.id < N_D2_ROBOT_TYPES) ? objP->m_info.id: -1);
CBInit (CBExplType (), (char **) "explosion", NULL, exp2_vclip_num_table, MAX_EXP2_VCLIP_NUM_TABLE, 2);
CBInit (CBWeapon1 (), (char **) 7000, NULL, NULL, MAX_WEAPON_TYPES, 3, true);
CBInit (CBWeapon2 (), (char **) 7000, NULL, NULL, MAX_WEAPON_TYPES, 3, true);
i = CBContType ()->AddString ("Robot");
CBContType ()->SetItemData (i, OBJ_ROBOT);
i = CBContType ()->AddString ("Powerup");
CBContType ()->SetItemData (i, OBJ_POWERUP);
i = CBContType ()->AddString ("(none)");
CBContType ()->SetItemData (i, -1);
// setup sound list boxes
char szSound [100];
for (i = 0; i < 196; i++) {
	LoadString (hInst, 6000 + i, szSound,sizeof (szSound));
	// INT32 nSound = (szSound [0] - '0') * 100 + (szSound [1] - '0') * 10 + (szSound [2] - '0');
	INT32 nSound = atoi (szSound);
	INT32 index = CBSoundExpl ()->AddString (szSound + 3);
	CBSoundExpl ()->SetItemData (index, nSound);
	index = CBSoundSee ()->AddString (szSound + 3);
	CBSoundSee ()->SetItemData (index, nSound);
	index = CBSoundAttack ()->AddString (szSound + 3);
	CBSoundAttack ()->SetItemData (index, nSound);
	index = CBSoundClaw ()->AddString (szSound + 3);
	CBSoundClaw ()->SetItemData (index, nSound);
	index = CBSoundDeath ()->AddString (szSound + 3);
	CBSoundDeath ()->SetItemData (index, nSound);
	}
Refresh ();
m_bInited = true;
return TRUE;
}

                        /*--------------------------*/

void CObjectTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;
DDX_Text (pDX, IDC_OBJ_SPAWN_QTY, m_nSpawnQty);
DDX_Text (pDX, IDC_OBJ_INFO, m_szInfo, sizeof (m_szInfo));
if (!pDX->m_bSaveAndValidate) {
	char szCount [4];

	sprintf_s (szCount, sizeof (szCount), "%d", ObjOfAKindCount ());
	AfxSetWindowText (GetDlgItem (IDT_OBJ_COUNT)->GetSafeHwnd (), szCount);
	}
DDX_Check (pDX, IDC_OBJ_SORT, theMine->m_bSortObjects);
}

                        /*--------------------------*/

BOOL CObjectTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

                        /*--------------------------*/

void CObjectTool::Reset ()
{
//m_nSpawnQty = 0;
}

                        /*--------------------------*/

void CObjectTool::EnableControls (BOOL bEnable)
{
CToolDlg::EnableControls (IDC_OBJ_OBJNO, IDT_OBJ_CONT_PROB, bEnable);
}

                        /*--------------------------*/

INT32 CObjectTool::GetSliderData (CScrollBar *pScrollBar)
{
INT32 h = sizeof (sliderData) / sizeof (tSliderData);
INT32 i;
for (i = 0; i < h; i++)
	if (pScrollBar == (CScrollBar *) GetDlgItem (sliderData [i].nId))
		return i;
return -1;
}

                        /*--------------------------*/

void CObjectTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
INT32 i = GetSliderData (pScrollBar);
if (i < 0)
	return;
tSliderData *psd = sliderData + i;
INT32 nPos = pScrollBar->GetScrollPos ();
switch (scrollCode) {
	case SB_LINEUP:
		nPos++;
		break;
	case SB_LINEDOWN:
		nPos--;
		break;
	case SB_PAGEUP:
		nPos += (psd->nMax - psd->nMin) / 4;
		break;
	case SB_PAGEDOWN:
		nPos -= (psd->nMax - psd->nMin) / 4;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nPos = thumbPos;
		break;
	case SB_ENDSCROLL:
		return;
	}
if (nPos < psd->nMin)
	nPos = psd->nMin;
else if (nPos > psd->nMax)
	nPos = psd->nMax;
pScrollBar->SetScrollPos (nPos);
if (pScrollBar == SBCtrl (IDC_OBJ_SKILL))
	Refresh ();
else
	UpdateSliders (i);
UpdateRobot ();
}

                        /*--------------------------*/

void CObjectTool::Refresh ()
{
if (!(m_bInited && theMine))
	return;
HINSTANCE hInst = AfxGetApp()->m_hInstance;

INT16 type;

// update object list box
CBObjNo ()->ResetContent ();
CGameObject *objP = theMine->Objects (0);
INT32 i;
for (i = 0; i < theMine->GameInfo ().objects.count; i++, objP++) {
	switch(objP->m_info.type) {
		case OBJ_ROBOT: /* an evil enemy */
			LoadString (hInst, ROBOT_STRING_TABLE + objP->m_info.id, string, sizeof (string));
			break;
		case OBJ_HOSTAGE: // a hostage you need to rescue
			sprintf_s (string, sizeof (string), "Hostage");
			break;
		case OBJ_PLAYER: // the player on the console
			sprintf_s (string, sizeof (string), "Player #%d", objP->m_info.id + 1);
			break;
		case OBJ_WEAPON: //
			strcpy_s (string, sizeof (string), "Red Mine");
			break;
		case OBJ_POWERUP: // a powerup you can pick up
			LoadString (hInst, POWERUP_STRING_TABLE + powerupIdStrXlat [objP->m_info.id], string, sizeof (string));
			break;
		case OBJ_CNTRLCEN: // a control center */
			sprintf_s (string, sizeof (string), "Reactor");
			break;
		case OBJ_COOP: // a cooperative player object
			sprintf_s (string, sizeof (string), "Coop Player #%d", objP->m_info.id + 1);
			break;
		case OBJ_CAMBOT: // a camera */
			sprintf_s (string, sizeof (string), "Camera");
			break;
		case OBJ_MONSTERBALL: // a camera */
			sprintf_s (string, sizeof (string), "Monsterball");
			break;
		case OBJ_EXPLOSION:
			sprintf_s (string, sizeof (string), "Explosion");
			break;
		case OBJ_SMOKE: 
			sprintf_s (string, sizeof (string), "Smoke");
			break;
		case OBJ_EFFECT:
			sprintf_s (string, sizeof (string), "Effect");
			break;
		default:
			*string = '\0';
	}
	sprintf_s (message, sizeof (message), (i < 10) ? "%3d: %s": "%d: %s", i, string);
	CBObjNo ()->AddString (message);
	}
// add secret object to list
for (i = 0; i < theMine->GameInfo ().triggers.count; i++)
	if (theMine->Triggers (i)->m_info.type == TT_SECRET_EXIT) {
		CBObjNo ()->AddString ("secret object");
		break;
		}
// select current object
CBObjNo ()->SetCurSel (theMine->Current ()->nObject);

// if secret object, disable everything but the "move" button
// and the object list, then return
if (theMine->Current ()->nObject == theMine->GameInfo ().objects.count) {
	CToolDlg::EnableControls (IDC_OBJ_OBJNO, IDC_OBJ_SPAWN_QTY, FALSE);
	CBObjNo ()->EnableWindow (TRUE);
	BtnCtrl (IDC_OBJ_MOVE)->EnableWindow (TRUE);

	strcpy_s (m_szInfo, sizeof (m_szInfo), "Secret Level Return");
	CBObjType ()->SetCurSel (-1);
	CBObjId ()->SetCurSel (-1);
	CBObjAI ()->SetCurSel (-1);
	CBObjTexture ()->SetCurSel (-1);
	CBSpawnType ()->SetCurSel (-1);
	CBSpawnId ()->SetCurSel (-1);
	CBObjClassAI ()->SetCurSel (-1);

	CDC *pDC = m_showObjWnd.GetDC ();
	if (pDC) {
		CRect rc;
		m_showObjWnd.GetClientRect (rc);
		pDC->FillSolidRect (&rc, IMG_BKCOLOR);
		m_showObjWnd.ReleaseDC (pDC);
		CToolDlg::EnableControls (IDC_OBJ_DELETEALL, IDC_OBJ_DELETEALL, theMine->GameInfo ().objects.count > 0);
		UpdateData (FALSE);
		}
	return;
	}

// otherwise (non-secret object), setup the rest of the
// dialog.
objP = theMine->CurrObj ();
sprintf_s (m_szInfo, sizeof (m_szInfo), "cube %d", objP->m_info.nSegment);
if (/*(objectSelection [objP->m_info.type] == 0) &&*/ theMine->RobotInfo (objP->m_info.id)->m_info.bCustom)
	strcat_s (m_szInfo, sizeof (m_szInfo), "\r\nmodified");

CBObjType ()->SetCurSel (objectSelection [objP->m_info.type]);
SetObjectId (CBObjId (), objP->m_info.type, objP->m_info.id);

// ungray most buttons and combo boxes
CToolDlg::EnableControls (IDC_OBJ_OBJNO, IDC_OBJ_SPAWN_QTY, TRUE);
CToolDlg::EnableControls (IDC_OBJ_MULTIPLAYER, IDC_OBJ_MULTIPLAYER, TRUE);

// gray contains and behavior if not a robot type object
if (objP->m_info.type != OBJ_ROBOT) {
//	CBObjProps ()->EnableWindow (FALSE);
	CToolDlg::EnableControls (IDC_OBJ_SPAWN_TYPE, IDC_OBJ_SPAWN_QTY, FALSE);
	CToolDlg::EnableControls (IDC_OBJ_BRIGHT, IDT_OBJ_CONT_PROB, FALSE);
	if (objP->m_info.type != OBJ_POWERUP)
		CToolDlg::EnableControls (IDC_OBJ_MULTIPLAYER, IDC_OBJ_MULTIPLAYER, FALSE);
	for (i = IDC_OBJ_SOUND_EXPLODE; i <= IDC_OBJ_SOUND_DEATH; i++)
		CBCtrl (i)->SetCurSel (-1);
	}
else {
	CToolDlg::EnableControls (IDC_OBJ_BRIGHT, IDT_OBJ_CONT_PROB, TRUE);
	for (i = IDC_OBJ_SOUND_EXPLODE; i <= IDC_OBJ_SOUND_DEATH; i++)
		CBCtrl (i)->SetCurSel (0);
	}

// gray texture override if not a poly object
if (objP->m_info.renderType != RT_POLYOBJ)
	CBObjTexture ()->EnableWindow (FALSE);

// gray edit if this is an RDL file
if (theApp.IsD1File ())
	CToolDlg::EnableControls (IDC_OBJ_BRIGHT, IDT_OBJ_CONT_PROB, FALSE);

// set contains data
type = (objP->m_info.contents.type == -1) ? MAX_CONTAINS_NUMBER : contentsSelection [objP->m_info.contents.type];
//if (type == -1)
//	type = MAX_CONTAINS_NUMBER;

CBSpawnType ()->SetCurSel (type + 1);
BtnCtrl (IDC_OBJ_MULTIPLAYER)->SetCheck (theMine->CurrObj ()->m_info.multiplayer);
theMine->CurrObj ()->m_info.multiplayer = BtnCtrl (IDC_OBJ_MULTIPLAYER)->GetCheck ();
//SelectItemData (CBSpawnType (), type);
SetObjectId (CBSpawnId (), objP->m_info.contents.type, objP->m_info.contents.id, 1);
m_nSpawnQty = objP->m_info.contents.count;
//SelectItemData (CBObjProps (), (objP->m_info.type == OBJ_ROBOT) && (objP->m_info.id < N_D2_ROBOT_TYPES) ? objP->m_info.id: -1);
if ((objP->m_info.type == OBJ_ROBOT) || (objP->m_info.type == OBJ_CAMBOT)) {
	INT32 index =
		((objP->cType.aiInfo.behavior == AIB_RUN_FROM) && (objP->cType.aiInfo.flags [4] & 0x02)) ? // smart bomb flag
		8 : objP->cType.aiInfo.behavior - 0x80;
	CBObjAI ()->SetCurSel (index);
	}
else
	CBObjAI ()->SetCurSel (1); // Normal
m_bEndsLevel = (objP->m_info.type == OBJ_ROBOT) && (theMine->RobotInfo (objP->m_info.id)->m_info.bossFlag > 0);
RefreshRobot ();
UpdateSliders ();
DrawObjectImages ();
SetTextureOverride ();
CToolDlg::EnableControls (IDC_OBJ_DELETEALL, IDC_OBJ_DELETEALL, theMine->GameInfo ().objects.count > 0);
UpdateData (FALSE);
theApp.MineView ()->Refresh (FALSE);
}

                        /*--------------------------*/

void CObjectTool::RefreshRobot ()
{
  INT32 i,j, nType;
  CRobotInfo rInfo;

  // get selection
if ((nType = object_list [CBObjType ()->GetCurSel ()]) != OBJ_ROBOT) {
	CBContId ()->SetCurSel (-1);
	CBWeapon1 ()->SetCurSel (-1);
	CBWeapon2 ()->SetCurSel (-1);
	CBSoundExpl ()->SetCurSel (-1);
	CBSoundSee ()->SetCurSel (-1);
	CBSoundAttack ()->SetCurSel (-1);
	CBSoundClaw ()->SetCurSel (-1);
	CBSoundDeath ()->SetCurSel (-1);
	CBObjClassAI ()->SetCurSel (-1);
	CBExplType ()->SetCurSel (-1);
	CBContType ()->SetCurSel (-1);
	// update check boxes
	BtnCtrl (IDC_OBJ_AI_KAMIKAZE)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_COMPANION)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_THIEF)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_SMARTBLOBS)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_PURSUE)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_CHARGE)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_EDRAIN)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_AI_ENDSLEVEL)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_BRIGHT)->SetCheck (FALSE);
	BtnCtrl (IDC_OBJ_CLOAKED)->SetCheck (FALSE);
	if (nType != OBJ_POWERUP)
		BtnCtrl (IDC_OBJ_MULTIPLAYER)->SetCheck (FALSE);
	// update scroll bars
	SlCtrl (IDC_OBJ_SCORE)->SetPos (0);
	SlCtrl (IDC_OBJ_STRENGTH)->SetPos (0);
	SlCtrl (IDC_OBJ_MASS)->SetPos (0);
	SlCtrl (IDC_OBJ_DRAG)->SetPos (0);
	SlCtrl (IDC_OBJ_EBLOBS)->SetPos (0);
	SlCtrl (IDC_OBJ_LIGHT)->SetPos (0);
	SlCtrl (IDC_OBJ_GLOW)->SetPos (0);
	SlCtrl (IDC_OBJ_AIM)->SetPos (0);
	SlCtrl (IDC_OBJ_FOV)->SetPos (0);
	SlCtrl (IDC_OBJ_FIREWAIT1)->SetPos (0);
	SlCtrl (IDC_OBJ_FIREWAIT2)->SetPos (0);
	SlCtrl (IDC_OBJ_TURNTIME)->SetPos (0);
	SlCtrl (IDC_OBJ_MAXSPEED)->SetPos (0);
	SlCtrl (IDC_OBJ_CIRCLEDIST)->SetPos (0);
	SlCtrl (IDC_OBJ_FIRESPEED)->SetPos (0);
	SlCtrl (IDC_OBJ_EVADESPEED)->SetPos (0);
	SlCtrl (IDC_OBJ_DEATHROLL)->SetPos (0);
	SlCtrl (IDC_OBJ_EXPLSIZE)->SetPos (0);
	SlCtrl (IDC_OBJ_CONT_PROB)->SetPos (0);
	SlCtrl (IDC_OBJ_CONT_COUNT)->SetPos (0);
	return;
	}
i = INT32 (CBObjId ()->GetItemData (CBObjId ()->GetCurSel ()));
rInfo = *theMine->RobotInfo (i);
j = SlCtrl (IDC_OBJ_SKILL)->GetPos ();
CBContId ()->ResetContent ();
switch (rInfo.m_info.contents.type) {
	case OBJ_ROBOT: /* an evil enemy */
		CBInit (CBContId (), (char **) ROBOT_STRING_TABLE, NULL, NULL, ROBOT_IDS2, 1, true);
		break;
	case OBJ_POWERUP: // a powerup you can pick up
		CBInit (CBContId (), (char **) POWERUP_STRING_TABLE, NULL, NULL, MAX_POWERUP_IDS, 1, true);
		break;
	}
// update list boxes
SelectItemData (CBContId (), (INT32) rInfo.m_info.contents.id);
SelectItemData (CBWeapon1 (), (INT32) (rInfo.m_info.weaponType [0] ? rInfo.m_info.weaponType [0] : -1));
SelectItemData (CBWeapon2 (), (INT32) rInfo.m_info.weaponType [1]);
SelectItemData (CBSoundExpl (), (INT32) rInfo.m_info.expl [1].nSound);
SelectItemData (CBSoundSee (), (INT32) rInfo.m_info.sounds.see);
SelectItemData (CBSoundAttack (), (INT32) rInfo.m_info.sounds.attack);
SelectItemData (CBSoundClaw (), (INT32) rInfo.m_info.sounds.claw);
SelectItemData (CBSoundDeath (), (INT32) rInfo.m_info.deathRollSound);
SelectItemData (CBObjClassAI (), (INT32) rInfo.m_info.behavior);
SelectItemData (CBExplType (), (INT32) rInfo.m_info.expl [1].nClip);
SelectItemData (CBContType (), (INT32) rInfo.m_info.contents.type);
INT8 bossFlag = (rInfo.m_info.bossFlag < 0) ? -rInfo.m_info.bossFlag : rInfo.m_info.bossFlag;
SelectItemData (CBBossType (), (INT32) (bossFlag < 21) ? bossFlag : bossFlag - 18);
// update check boxes
BtnCtrl (IDC_OBJ_AI_KAMIKAZE)->SetCheck (rInfo.m_info.kamikaze);
BtnCtrl (IDC_OBJ_AI_COMPANION)->SetCheck (rInfo.m_info.companion);
BtnCtrl (IDC_OBJ_AI_THIEF)->SetCheck (rInfo.m_info.thief);
BtnCtrl (IDC_OBJ_AI_SMARTBLOBS)->SetCheck (rInfo.m_info.smartBlobs);
BtnCtrl (IDC_OBJ_AI_PURSUE)->SetCheck (rInfo.m_info.pursuit);
BtnCtrl (IDC_OBJ_AI_CHARGE)->SetCheck (rInfo.m_info.attackType);
BtnCtrl (IDC_OBJ_AI_EDRAIN)->SetCheck (m_bEndsLevel);
BtnCtrl (IDC_OBJ_AI_ENDSLEVEL)->SetCheck (m_bEndsLevel);
BtnCtrl (IDC_OBJ_BRIGHT)->SetCheck (rInfo.m_info.lighting);
BtnCtrl (IDC_OBJ_CLOAKED)->SetCheck (rInfo.m_info.cloakType);

// update scroll bars
SlCtrl (IDC_OBJ_SCORE)->SetPos ((INT32) (rInfo.m_info.scoreValue / SliderFactor (IDC_OBJ_SCORE)));
SlCtrl (IDC_OBJ_STRENGTH)->SetPos (fix_log (rInfo.m_info.strength));
SlCtrl (IDC_OBJ_MASS)->SetPos (fix_log (rInfo.m_info.mass));
SlCtrl (IDC_OBJ_DRAG)->SetPos ((INT32) (rInfo.m_info.drag / SliderFactor (IDC_OBJ_DRAG)));
SlCtrl (IDC_OBJ_EBLOBS)->SetPos ((INT32) (rInfo.m_info.energyBlobs / SliderFactor (IDC_OBJ_EBLOBS)));
SlCtrl (IDC_OBJ_LIGHT)->SetPos ((INT32) (rInfo.m_info.lightCast / SliderFactor (IDC_OBJ_LIGHT)));
SlCtrl (IDC_OBJ_GLOW)->SetPos ((INT32) (rInfo.m_info.glow / SliderFactor (IDC_OBJ_GLOW)));
SlCtrl (IDC_OBJ_AIM)->SetPos ((INT32) ((rInfo.m_info.aim + 1) / SliderFactor (IDC_OBJ_AIM)));
SlCtrl (IDC_OBJ_FOV)->SetPos ((INT32) (rInfo.m_info.combat [j].fieldOfView / SliderFactor (IDC_OBJ_FOV)));
SlCtrl (IDC_OBJ_FIREWAIT1)->SetPos ((INT32) (rInfo.m_info.combat [j].firingWait [0] / SliderFactor (IDC_OBJ_FIREWAIT1)));
SlCtrl (IDC_OBJ_FIREWAIT2)->SetPos ((INT32) (rInfo.m_info.combat [j].firingWait [1] / SliderFactor (IDC_OBJ_FIREWAIT2)));
SlCtrl (IDC_OBJ_TURNTIME)->SetPos ((INT32) (rInfo.m_info.combat [j].turnTime / SliderFactor (IDC_OBJ_TURNTIME)));
SlCtrl (IDC_OBJ_MAXSPEED)->SetPos ((INT32) (rInfo.m_info.combat [j].maxSpeed / SliderFactor (IDC_OBJ_MAXSPEED)));
SlCtrl (IDC_OBJ_CIRCLEDIST)->SetPos ((INT32) (rInfo.m_info.combat [j].circleDistance / SliderFactor (IDC_OBJ_CIRCLEDIST)));
SlCtrl (IDC_OBJ_FIRESPEED)->SetPos ((INT32) (rInfo.m_info.combat [j].rapidFire / SliderFactor (IDC_OBJ_FIRESPEED)));
SlCtrl (IDC_OBJ_EVADESPEED)->SetPos ((INT32) (rInfo.m_info.combat [j].evadeSpeed / SliderFactor (IDC_OBJ_EVADESPEED)));
SlCtrl (IDC_OBJ_DEATHROLL)->SetPos ((INT32) (rInfo.m_info.deathRoll / SliderFactor (IDC_OBJ_DEATHROLL)));
SlCtrl (IDC_OBJ_EXPLSIZE)->SetPos ((INT32) (rInfo.m_info.badass / SliderFactor (IDC_OBJ_EXPLSIZE)));
SlCtrl (IDC_OBJ_CONT_PROB)->SetPos ((INT32) (rInfo.m_info.contents.prob / SliderFactor (IDC_OBJ_CONT_PROB)));
SlCtrl (IDC_OBJ_CONT_COUNT)->SetPos ((INT32) (rInfo.m_info.contents.count / SliderFactor (IDC_OBJ_CONT_COUNT)));
}
  
                        /*--------------------------*/

void CObjectTool::UpdateRobot ()
{
  INT32 i,j;
  CRobotInfo rInfo;

  // get selection
i = INT32 (CBObjId ()->GetItemData (CBObjId ()->GetCurSel ()));
if (i < 0 || i >= ROBOT_IDS2)
	i = 0;
j = SlCtrl (IDC_OBJ_SKILL)->GetPos ();
rInfo = *theMine->RobotInfo (i);
theApp.SetModified (TRUE);
rInfo.m_info.bCustom |= 1;
rInfo.m_info.scoreValue = (INT32) (SlCtrl (IDC_OBJ_SCORE)->GetPos () * SliderFactor (IDC_OBJ_SCORE));
rInfo.m_info.strength = (INT32) fix_exp (SlCtrl (IDC_OBJ_STRENGTH)->GetPos ());
rInfo.m_info.mass = (INT32) fix_exp (SlCtrl (IDC_OBJ_MASS)->GetPos ());
rInfo.m_info.drag = (INT32) (SlCtrl (IDC_OBJ_DRAG)->GetPos () * SliderFactor (IDC_OBJ_DRAG));
rInfo.m_info.energyBlobs = (INT32) (SlCtrl (IDC_OBJ_EBLOBS)->GetPos ()  * SliderFactor (IDC_OBJ_EBLOBS));
rInfo.m_info.lightCast = (INT32) (SlCtrl (IDC_OBJ_LIGHT)->GetPos () * SliderFactor (IDC_OBJ_LIGHT));
rInfo.m_info.glow = (INT32) (SlCtrl (IDC_OBJ_GLOW)->GetPos () * SliderFactor (IDC_OBJ_GLOW));
rInfo.m_info.aim = (INT32) ((SlCtrl (IDC_OBJ_AIM)->GetPos ()) * SliderFactor (IDC_OBJ_AIM)) - 1;
rInfo.m_info.combat [j].fieldOfView = (INT32) (SlCtrl (IDC_OBJ_FOV)->GetPos () * SliderFactor (IDC_OBJ_FOV));
rInfo.m_info.combat [j].firingWait [0] = (INT32) (SlCtrl (IDC_OBJ_FIREWAIT1)->GetPos () * SliderFactor (IDC_OBJ_FIREWAIT1));
rInfo.m_info.combat [j].firingWait [1] = (INT32) (SlCtrl (IDC_OBJ_FIREWAIT2)->GetPos () * SliderFactor (IDC_OBJ_FIREWAIT2));
rInfo.m_info.combat [j].turnTime = (INT32) (SlCtrl (IDC_OBJ_TURNTIME)->GetPos () * SliderFactor (IDC_OBJ_TURNTIME));
rInfo.m_info.combat [j].maxSpeed = (INT32) (SlCtrl (IDC_OBJ_MAXSPEED)->GetPos () * SliderFactor (IDC_OBJ_MAXSPEED));
rInfo.m_info.combat [j].circleDistance = (INT32) (SlCtrl (IDC_OBJ_CIRCLEDIST)->GetPos () * SliderFactor (IDC_OBJ_CIRCLEDIST));
rInfo.m_info.combat [j].rapidFire = (INT32) (SlCtrl (IDC_OBJ_FIRESPEED)->GetPos () * SliderFactor (IDC_OBJ_FIRESPEED));
rInfo.m_info.combat [j].evadeSpeed = (INT32) (SlCtrl (IDC_OBJ_EVADESPEED)->GetPos () * SliderFactor (IDC_OBJ_EVADESPEED));
rInfo.m_info.deathRoll = (INT32) (SlCtrl (IDC_OBJ_DEATHROLL)->GetPos () * SliderFactor (IDC_OBJ_DEATHROLL));
rInfo.m_info.badass = (INT32) (SlCtrl (IDC_OBJ_EXPLSIZE)->GetPos () * SliderFactor (IDC_OBJ_EXPLSIZE));
rInfo.m_info.contents.prob = (INT32) (SlCtrl (IDC_OBJ_CONT_PROB)->GetPos () * SliderFactor (IDC_OBJ_CONT_PROB));
rInfo.m_info.contents.count = (INT32) (SlCtrl (IDC_OBJ_CONT_COUNT)->GetPos () * SliderFactor (IDC_OBJ_CONT_COUNT));

rInfo.m_info.kamikaze = BtnCtrl (IDC_OBJ_AI_KAMIKAZE)->GetCheck ();
rInfo.m_info.companion = BtnCtrl (IDC_OBJ_AI_COMPANION)->GetCheck ();
rInfo.m_info.thief = BtnCtrl (IDC_OBJ_AI_THIEF)->GetCheck ();
rInfo.m_info.smartBlobs = BtnCtrl (IDC_OBJ_AI_SMARTBLOBS)->GetCheck ();
rInfo.m_info.pursuit = BtnCtrl (IDC_OBJ_AI_PURSUE)->GetCheck ();
rInfo.m_info.attackType = BtnCtrl (IDC_OBJ_AI_CHARGE)->GetCheck ();
rInfo.m_info.drainEnergy = BtnCtrl (IDC_OBJ_AI_EDRAIN)->GetCheck ();
m_bEndsLevel = BtnCtrl (IDC_OBJ_AI_ENDSLEVEL)->GetCheck ();
rInfo.m_info.lighting = BtnCtrl (IDC_OBJ_BRIGHT)->GetCheck ();
rInfo.m_info.cloakType = BtnCtrl (IDC_OBJ_CLOAKED)->GetCheck ();

// get list box changes
INT32 index;
if (0 <= (index = CBBossType ()->GetCurSel ())) {
	rInfo.m_info.bossFlag = (UINT8) CBBossType ()->GetItemData (index);
	if ((rInfo.m_info.bossFlag = (UINT8) CBBossType ()->GetItemData (index)) > 2)
			rInfo.m_info.bossFlag += 18;
	if (!m_bEndsLevel)
		rInfo.m_info.bossFlag = -rInfo.m_info.bossFlag;
	}
if (0 <= (index = CBWeapon1 ()->GetCurSel ())) {
	rInfo.m_info.weaponType [0] = (UINT8) CBWeapon1 ()->GetItemData (index);
	if (rInfo.m_info.weaponType [0] < 0)
		rInfo.m_info.weaponType [0] = 0;
	}
if (0 <= (index = CBWeapon2 ()->GetCurSel ()))
	rInfo.m_info.weaponType [1] = (UINT8) CBWeapon2 ()->GetItemData (index);
if (0 <= (index = CBSoundExpl ()->GetCurSel ()))
	rInfo.m_info.expl [1].nSound = (UINT8) CBSoundExpl ()->GetItemData (index);
if (0 <= (index = CBSoundSee ()->GetCurSel ()))
	rInfo.m_info.sounds.see = (UINT8) CBSoundSee ()->GetItemData (index);
if (0 <= (index = CBSoundAttack ()->GetCurSel ()))
	rInfo.m_info.sounds.attack = (UINT8) CBSoundAttack ()->GetItemData (index);
if (0 <= (index = CBSoundClaw ()->GetCurSel ()))
	rInfo.m_info.sounds.claw = (UINT8) CBSoundClaw ()->GetItemData (index);
if (0 <= (index = CBSoundDeath ()->GetCurSel ()))
	rInfo.m_info.deathRollSound = (UINT8) CBSoundDeath ()->GetItemData (index);
if (0 <= (index = CBObjClassAI ()->GetCurSel ()))
	rInfo.m_info.behavior = (UINT8) CBObjClassAI ()->GetItemData (index);
if (0 <= (index = CBExplType ()->GetCurSel ()))
	rInfo.m_info.expl [1].nClip = (UINT8) CBExplType ()->GetItemData (index);
if (0 <= (index = CBContType ()->GetCurSel ()))
	rInfo.m_info.contents.type = (UINT8) CBContType ()->GetItemData (index);
if (0 <= (index = CBContId ()->GetCurSel ()) - 1)
	rInfo.m_info.contents.id = (UINT8) CBContId ()->GetItemData (index);
*theMine->RobotInfo (i) = rInfo;
}

//------------------------------------------------------------------------
// CObjectTool - SetTextureOverride
//------------------------------------------------------------------------

void CObjectTool::SetTextureOverride ()
{
CGameObject *objP = theMine->CurrObj ();
#if 0
CRect rc;
m_showTextureWnd.GetClientRect (&rc);
pDC->FillSolidRect (&rc, IMG_BKCOLOR);
#endif
INT16 tnum = 0, tnum2 = -1;

if (objP->m_info.renderType != RT_POLYOBJ)
	CBObjTexture ()->SetCurSel (0);
else {
	tnum = (INT16) theMine->CurrObj ()->rType.polyModelInfo.tmap_override;
	if ((tnum < 0) || (tnum >= ((theApp.IsD1File ()) ? MAX_D1_TEXTURES : MAX_D2_TEXTURES))) {
		CBObjTexture ()->SetCurSel (0);
		tnum = 0;	// -> force PaintTexture to clear the texture display window
		}
	else {
		tnum2 = 0;
#if 0
		// texture is overrides, select index in list box
		CDC *pDC = m_showTextureWnd.GetDC ();
		if (!pDC)
			return;
		HINSTANCE hInst = AfxGetApp()->m_hInstance;
		LoadString (hInst,texture_resource + tnum, message, sizeof (message));
		CBObjTexture ()->SelectString (-1, message);
		// and show bitmap
		CTexture tx (bmBuf);
		if (!DefineTexture(tnum,0,&tx,0,0)) {
			CPalette * pOldPalette = pDC->SelectPalette(m_currentPalette, FALSE);
			pDC->RealizePalette ();
			BITMAPINFO *bmi;
			bmi = MakeBitmap ();
			CRect rc;
			m_showTextureWnd.GetClientRect (rc);
			StretchDIBits (pDC->m_hDC, 0, 0, rc.Width (), rc.Height (), 0, 0, tx.width, tx.height,
								(void *) bmBuf, bmi, DIB_RGB_COLORS,SRCCOPY);
			pDC->SelectPalette(pOldPalette, FALSE);
			}
#endif
		}
	}
#if 0
m_showTextureWnd.ReleaseDC (pDC);
m_showTextureWnd.InvalidateRect (NULL, TRUE);
m_showTextureWnd.UpdateWindow ();
#else
PaintTexture (&m_showTextureWnd, IMG_BKCOLOR, -1, -1, tnum, tnum2);
#endif
}

//------------------------------------------------------------------------
// CObjectTool - DrawObjectImage
//------------------------------------------------------------------------

void CObjectTool::DrawObjectImages () 
{
CGameObject *objP = theMine->CurrObj ();
theMine->DrawObject (&m_showObjWnd, objP->m_info.type, objP->m_info.id);
theMine->DrawObject (&m_showSpawnWnd, objP->m_info.contents.type, objP->m_info.contents.id);
}


//------------------------------------------------------------------------
// CObjectTool - Set Object Id Message
//------------------------------------------------------------------------

INT32 bbb = 1;

void CObjectTool::SetObjectId (CComboBox *pcb, INT16 type, INT16 id, INT16 flag) 
{
	char str [40];
	INT32 h, i, j;
	INT16 max_robot_ids = flag 
								 ? theApp.IsD1File () 
									? ROBOT_IDS1 
									: 64 
								 : theApp.IsD1File () 
									? ROBOT_IDS1 
									: ROBOT_IDS2;

pcb->ResetContent ();
HINSTANCE hInst = AfxGetApp ()->m_hInstance;
switch(type) {
	case OBJ_ROBOT: /* an evil enemy */
		for (i = 0; i < max_robot_ids; i++) {
			sprintf_s (string, sizeof (string), (i < 10) ? "%3d: ": "%d: ", i);
			h = INT32 (strlen (string));
			LoadString (hInst, ROBOT_STRING_TABLE + i, string + h, sizeof (string) - h);
			if (!strcmp (string, "(not used)"))
				continue;
			h = CBAddString (pcb, string);
			pcb->SetItemData (h, i);
			}
		if (id < 0 || id >= max_robot_ids) {
			sprintf_s (message, sizeof (message), " ObjectTool: Unknown robot id (%d)", id);
			DEBUGMSG (message);
			}
		SelectItemData (pcb, id); // if out of range, nothing is selected
		break;

	case OBJ_HOSTAGE: // a hostage you need to rescue
		for (i = 0; i <= 1; i++) {
			sprintf_s (str, sizeof (str), "%d", i);
			h = pcb->AddString (str);
			pcb->SetItemData (h, i);
			}
		SelectItemData (pcb, id);
		break;

	case OBJ_PLAYER: // the player on the console
		for (i = 0; i < MAX_PLAYERS; i++) {
			sprintf_s (str, sizeof (str), (i < 9) ? "%3d" : "%d", i + 1);
			h = pcb->AddString (str);
			pcb->SetItemData (h, i);
			}
		SelectItemData (pcb, id);
		break;

	case OBJ_WEAPON: //
		h = pcb->AddString ("Mine");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_CAMBOT: //
		h = pcb->AddString ("Camera");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_MONSTERBALL: //
		h = pcb->AddString ("Monsterball");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_EXPLOSION: //
		for (i = 0; i < 5; i++) {
			h = pcb->AddString (pszExplosionIds [i]);
			pcb->SetItemData (h, nExplosionIds [i]);
			}
		SelectItemData (pcb, id); // if out of range, nothing is selected
		break;

	case OBJ_SMOKE: //
		h = pcb->AddString ("Smoke");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_EFFECT: //
		h = pcb->AddString ("Effect");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
		break;

	case OBJ_POWERUP: // a powerup you can pick up
		INT32 xlat [100];
		memset (xlat, 0xff, sizeof (xlat));
		h = pcb->AddString ("(none)");
		pcb->SetItemData (h, -1);
		for (i = 0; i < MAX_POWERUP_IDS; i++) {
			j = powerupIdStrXlat [i];
			LoadString (hInst, POWERUP_STRING_TABLE + j, string, sizeof (string));
			if (!strcmp (string, "(not used)"))
				continue;
			h = pcb->AddString (string);
			xlat [i] = j;
			pcb->SetItemData (h, j);
			}
#if 0//def _DEBUG // hack to fix bogus powerup ids
		CGameObject *objP;
		for (i = 0, objP = theMine->Objects (0); i < theMine->ObjCount (); i++, objP++)
			if ((objP->m_info.type == OBJ_POWERUP) && (xlat [objP->m_info.id] == -1)) {
				for (i = 0, objP = theMine->Objects (0); i < theMine->ObjCount (); i++, objP++)
					if (objP->m_info.type == OBJ_POWERUP)
						objP->m_info.id = xlat [objP->m_info.id]; 
				break;
				}
#endif
		SelectItemData (pcb, id);
		break;

	case OBJ_CNTRLCEN: // a control center */
		if (theApp.IsD1File ()) {
			for ( i = 0; i <= 25; i++) { //??? not sure of max
				sprintf_s (str, sizeof (str), "%d", i);
				h = pcb->AddString (str);
				pcb->SetItemData (h, i);
				}
			}
		else {
			for (i = 1; i <= 6; i++) {
				sprintf_s (str, sizeof (str), "%d", i);
				h = pcb->AddString (str);
				pcb->SetItemData (h, i);
				}
			}
		SelectItemData (pcb, id);
		break;

	case OBJ_COOP: // a cooperative player object
		for (i = MAX_PLAYERS; i < MAX_PLAYERS + MAX_COOP_PLAYERS; i++) {
			sprintf_s (str, sizeof (str), "%d", i);
			h = pcb->AddString (str);
			pcb->SetItemData (h, i - MAX_PLAYERS);
			}
		SelectItemData (pcb, id - MAX_PLAYERS);
		break;

	default:
		h = pcb->AddString ("(none)");
		pcb->SetItemData (0, h);
		pcb->SetCurSel (0);
	break;
	}
}

//------------------------------------------------------------------------
// CObjectTool - WMDrawItem
//------------------------------------------------------------------------

void CObjectTool::OnPaint ()
{
CToolDlg::OnPaint ();
DrawObjectImages ();
SetTextureOverride ();
}

//------------------------------------------------------------------------
// CObjectTool - Add Object Message
//------------------------------------------------------------------------

void CObjectTool::OnAdd () 
{
if (theMine->Current ()->nObject == theMine->GameInfo ().objects.count) {
	ErrorMsg ("Cannot add another secret return.");
	return;
 }

if (theMine->GameInfo ().objects.count >= MAX_OBJECTS) {
	ErrorMsg ("Maximum numbers of objects reached");
	return;
	}
theMine->CopyObject (OBJ_NONE);
Refresh ();
}

//------------------------------------------------------------------------
// CObjectTool - Delete Object Message
//------------------------------------------------------------------------

void CObjectTool::OnDelete ()
{
if (theMine->Current ()->nObject == theMine->GameInfo ().objects.count) {
	ErrorMsg ("Cannot delete the secret return.");
	return;
	}
if (theMine->GameInfo ().objects.count == 1) {
	ErrorMsg ("Cannot delete the last object");
	return;
	}
if (QueryMsg ("Are you sure you want to delete this object?") == IDYES) {
	theMine->DeleteObject ();
	Refresh ();
	theApp.MineView ()->Refresh (false);
	}
}

//------------------------------------------------------------------------
// CObjectTool - Delete All (Marked) Triggers
//------------------------------------------------------------------------

void CObjectTool::OnDeleteAll () 
{
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
CGameObject *objP = theMine->CurrObj ();
INT32 nType = objP->m_info.type;
INT32 nId = objP->m_info.id;
objP = theMine->Objects (0);
bool bAll = (theMine->MarkedSegmentCount (true) == 0);
INT32 nDeleted = 0;
for (INT32 h = theMine->GameInfo ().objects.count, i = 0; i < h; ) {
	if ((objP->m_info.type == nType) && (objP->m_info.id == nId) && (bAll || (theMine->Segments (objP->m_info.nSegment)->m_info.wallFlags &= MARKED_MASK))) {
		theMine->DeleteObject (i);
		nDeleted++;
		h--;
		}
	else
		i++, objP++;
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
// CObjectTool - Reset Object Message
//------------------------------------------------------------------------

void CObjectTool::OnReset () 
{
CDoubleMatrix* orient;

theApp.SetModified (TRUE);
theApp.LockUndo ();
if (theMine->Current ()->nObject == theMine->GameInfo ().objects.count) {
	orient = &theMine->SecretOrient ();
	orient->Set (1, 0, 0, 0, 0, 1, 0, 1, 0);
} else {
	orient = &theMine->CurrObj ()->m_info.orient;
	orient->Set (1, 0, 0, 1, 0, 0, 0, 0, 1);
	}
theApp.UnlockUndo ();
Refresh ();
theApp.MineView ()->Refresh (false);
}

//------------------------------------------------------------------------
// CObjectTool - AdvancedMsg
//------------------------------------------------------------------------

void CObjectTool::OnAdvanced () 
{
}


//------------------------------------------------------------------------
// CObjectTool - Move Object Message
//------------------------------------------------------------------------

void CObjectTool::OnMove ()
{
#if 0
if (QueryMsg ("Are you sure you want to move the\n"
				 "current object to the current cube?\n") != IDYES)
	return;
#endif
theApp.SetModified (TRUE);
if (theMine->Current ()->nObject == theMine->GameInfo ().objects.count)
	theMine->SecretCubeNum () = theMine->Current ()->nSegment;
else {
	CGameObject *objP = theMine->CurrObj ();
	theMine->CalcSegCenter (objP->m_info.pos, theMine->Current ()->nSegment);
	// bump position over if this is not the first object in the cube
	INT32 i, count = 0;
	for (i = 0; i < theMine->GameInfo ().objects.count;i++)
		if (theMine->Objects (i)->m_info.nSegment == theMine->Current ()->nSegment)
			count++;
	objP->m_info.pos.v.y += count * 2 * F1_0;
	objP->m_info.lastPos.v.y += count * 2 * F1_0;
	objP->m_info.nSegment = theMine->Current ()->nSegment;
	Refresh ();
	theApp.MineView ()->Refresh (false);
	}
}

//------------------------------------------------------------------------
// CObjectTool - Object Number Message
//------------------------------------------------------------------------

void CObjectTool::OnSetObject ()
{
INT16 old_object = theMine->Current ()->nObject;
INT16 new_object = CBObjNo ()->GetCurSel ();
theApp.MineView ()->RefreshObject (old_object, new_object);
//Refresh ();
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

bool CObjectTool::SetPlayerId (CGameObject *objP, INT32 objType, INT32 *ids, INT32 numIds, char *pszError)
{
CGameObject *o = theMine->Objects (0);
INT32		i, n = 0;

for (i = theMine->ObjCount (); i && (n < numIds); i--, o++)
	if (o->m_info.type == objType)
		ids [n++] = -1;
if (n == numIds) {
	ErrorMsg (pszError);
	return false;
	}
for (i = 0; i < numIds; i++)
	if (ids [i] >= 0) {
		objP->m_info.id = ids [i];
		break;
		}
return true;
}

//------------------------------------------------------------------------
// CObjectTool - Type ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnSetObjType () 
{
CGameObject *objP = theMine->CurrObj ();
INT32 selection = object_list [CBObjType ()->GetCurSel ()];
if (theApp.IsD1File () && (selection == OBJ_WEAPON)) {
	ErrorMsg ("You can not use this type of object in a Descent 1 level");
	return;
	}
if ((selection == OBJ_SMOKE) || (selection == OBJ_EFFECT)) {
	ErrorMsg ("You can use the effects tool to create and edit this type of object");
	return;
	}
// set id
INT32 playerIds [16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
INT32 coopIds [3];
for (INT32 i = 0; i < 3; i++)
	coopIds [i] = MAX_PLAYERS + i;

switch (selection) {
	case OBJ_PLAYER:
		if (!SetPlayerId (objP, selection, playerIds, MAX_PLAYERS, "Only 8/16 players allowed."))
			return;
		break;

	case OBJ_COOP:
		if (!SetPlayerId (objP, selection, coopIds, 3, "Only 3 coop players allowed."))
			return;
		break;

	case OBJ_WEAPON:
		objP->m_info.id = SMALLMINE_ID;
		break;

	case OBJ_CNTRLCEN:
		objP->m_info.id = theApp.IsD1File () ? 0: 2;
		break;

	case OBJ_EXPLOSION:
		objP->m_info.id = 0;
		break;

	default:
		objP->m_info.id = 0;
	}
objP->m_info.type = selection;
SetObjectId (CBObjId (), selection, 0);
theMine->SetObjectData (objP->m_info.type);
Refresh ();
theApp.MineView ()->Refresh (false);
}

//------------------------------------------------------------------------
// CObjectTool - IdMsg Message
//
// This routine resets the size, shield, vclip if the id changes.
//------------------------------------------------------------------------

INT32 CObjectTool::GetObjectsOfAKind (INT32 nType, CGameObject *objList [])
{
	INT32 i, nObjects = 0;
	CGameObject *objP;

for (i = theMine->GameInfo ().objects.count, objP = theMine->Objects (0); i; i--, objP++)
	if (objP->m_info.type == nType)
		objList [nObjects++] = objP;
return nObjects;
}

//------------------------------------------------------------------------

void CObjectTool::SetNewObjId (CGameObject *objP, INT32 nType, INT32 nId, INT32 nMaxId)
{
if (nId = objP->m_info.id)
	return;

	INT32 nObjects = ObjOfAKindCount (nType);

CGameObject **objList = new CGameObject* [nObjects];
GetObjectsOfAKind (nType, objList);
if ((nMaxId > 0) && (nId >= nMaxId)) {
	nId = nMaxId;
	if (nId == objP->m_info.id)
		return;
	}
// find object that currently has id nCurSel and swap ids
INT32 i;
for (i = 0; i < nObjects; i++)
	if (objList [i]->m_info.id == nId) {
		objList [i]->m_info.id = objP->m_info.id;
		break;
		}
objP->m_info.id = nId;
delete objList;
}

//------------------------------------------------------------------------

void CObjectTool::OnSetObjId ()
{
	INT32	id;

CGameObject *objP = theMine->CurrObj ();
CComboBox *pcb = CBObjId ();
INT32 nCurSel = INT32 (pcb->GetItemData (pcb->GetCurSel ()));

theApp.SetModified (TRUE);
theApp.LockUndo ();
switch (objP->m_info.type) {
	case OBJ_PLAYER:
		SetNewObjId (objP, OBJ_PLAYER, nCurSel, MAX_PLAYERS);
		break;

	case OBJ_COOP:
		SetNewObjId (objP, OBJ_COOP, nCurSel + MAX_PLAYERS, 3);
		break;

	case OBJ_WEAPON:
		objP->m_info.id = SMALLMINE_ID;
		break;

	case OBJ_CNTRLCEN:
		objP->m_info.id = nCurSel; // + (IsD2File ());
		objP->rType.vClipInfo.vclip_num = nCurSel;
		break;

	default:
		objP->m_info.id = nCurSel;
	}

switch (objP->m_info.type) {
	case OBJ_POWERUP:
		id = (objP->m_info.id < MAX_POWERUP_IDS_D2) ? objP->m_info.id : POW_AMMORACK;
		objP->m_info.size = powerupSize [id];
		objP->m_info.shields = DEFAULT_SHIELD;
		objP->rType.vClipInfo.vclip_num = powerupClip [id];
		break;

	case OBJ_ROBOT:
		objP->m_info.size = robotSize [objP->m_info.id];
		objP->m_info.shields = robot_shield [objP->m_info.id];
		objP->rType.polyModelInfo.nModel = robotClip [objP->m_info.id];
		break;

	case OBJ_CNTRLCEN:
		objP->m_info.size = REACTOR_SIZE;
		objP->m_info.shields = REACTOR_SHIELD;
		if (theApp.IsD1File ())
			objP->rType.polyModelInfo.nModel = REACTOR_CLIP_NUMBER;
		else {
			INT32 model;
			switch(objP->m_info.id) {
				case 1: model = 95; break;
				case 2: model = 97; break;
				case 3: model = 99; break;
				case 4: model = 101; break;
				case 5: model = 103; break;
				case 6: model = 105; break;
				default: model = 97; break; // level 1's reactor
				}
			objP->rType.polyModelInfo.nModel = model;
		}
		break;

	case OBJ_PLAYER:
		objP->m_info.size = PLAYER_SIZE;
		objP->m_info.shields = DEFAULT_SHIELD;
		objP->rType.polyModelInfo.nModel = PLAYER_CLIP_NUMBER;
		break;

	case OBJ_WEAPON:
		objP->m_info.size = WEAPON_SIZE;
		objP->m_info.shields = WEAPON_SHIELD;
		objP->rType.polyModelInfo.nModel = MINE_CLIP_NUMBER;
		break;

	case OBJ_COOP:
		objP->m_info.size = PLAYER_SIZE;
		objP->m_info.shields = DEFAULT_SHIELD;
		objP->rType.polyModelInfo.nModel = COOP_CLIP_NUMBER;
		break;

	case OBJ_HOSTAGE:
		objP->m_info.size = PLAYER_SIZE;
		objP->m_info.shields = DEFAULT_SHIELD;
		objP->rType.vClipInfo.vclip_num = HOSTAGE_CLIP_NUMBER;
		break;
	}
theMine->SortObjects ();
SelectItemData (pcb, objP->m_info.id);
theApp.UnlockUndo ();
Refresh ();
}

//------------------------------------------------------------------------
// CObjectTool - ContainsQtyMsg
//------------------------------------------------------------------------

void CObjectTool::OnSetSpawnQty ()
{
UpdateData (TRUE);
theApp.SetModified (TRUE);
theMine->CurrObj ()->m_info.contents.count = m_nSpawnQty;
Refresh ();
}

//------------------------------------------------------------------------
// CObjectTool - Container Type ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnSetSpawnType () 
{
CGameObject *objP = theMine->CurrObj ();
INT32 selection;
theApp.SetModified (TRUE);
theApp.UnlockUndo ();
INT32 i = CBSpawnType ()->GetCurSel () - 1;
if ((i < 0) || (i == MAX_CONTAINS_NUMBER)) {
	objP->m_info.contents.count = 0;
	objP->m_info.contents.type = -1;
	objP->m_info.contents.id = -1;
	}
else {
	objP->m_info.contents.type = 
	selection = contentsList [i];
	SetObjectId (CBSpawnId (),selection,0,1);
	UpdateData (TRUE);
	if (m_nSpawnQty < 1) {
		m_nSpawnQty = 1;
		UpdateData (FALSE);
		}
	OnSetSpawnQty ();
	OnSetSpawnId ();
	}
theApp.LockUndo ();
}

//------------------------------------------------------------------------
// CObjectTool - ContainsIdMsg
//------------------------------------------------------------------------

void CObjectTool::OnSetSpawnId () 
{
CGameObject *objP = theMine->CurrObj ();

theApp.SetModified (TRUE);
if (objP->m_info.contents.count < -1)
	objP->m_info.contents.count = -1;
INT32 i = CBSpawnType ()->GetCurSel () - 1;
if ((i > -1) && (objP->m_info.contents.count > 0)) {
	objP->m_info.contents.type = contentsList [i];
	objP->m_info.contents.id = (INT8) CBSpawnId ()->GetItemData (CBSpawnId ()->GetCurSel ());
	}
else {
	objP->m_info.contents.type = -1;
	objP->m_info.contents.id = -1;
	}
Refresh ();
theApp.SetModified (TRUE);
}

//------------------------------------------------------------------------
// CObjectTool - Options ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnSetObjAI ()
{
theApp.SetModified (TRUE);
CGameObject *objP = theMine->CurrObj ();
if ((objP->m_info.type == OBJ_ROBOT) || (objP->m_info.type == OBJ_CAMBOT)) {
 	INT32 index = CBObjAI ()->GetCurSel ();
	if (index == 8) {
		index = AIB_RUN_FROM;
		objP->cType.aiInfo.flags [4] |= 2; // smart bomb flag
		}
	else {
		index += 0x80;
		objP->cType.aiInfo.flags [4] &= ~2;
		}
	objP->cType.aiInfo.behavior = index;
	}
else
	CBObjAI ()->SetCurSel (1); // Normal
Refresh ();
theApp.SetModified (TRUE);
}

//------------------------------------------------------------------------
// CObjectTool - Texture ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnSetTexture ()
{
CGameObject *objP = theMine->CurrObj ();

if (objP->m_info.renderType == RT_POLYOBJ) {
	theApp.SetModified (TRUE);
	INT32 index = CBObjTexture ()->GetCurSel ();
	objP->rType.polyModelInfo.tmap_override = 
		(index > 0) ? (INT16)CBObjTexture ()->GetItemData (index): -1;
	Refresh ();
	}
}

//------------------------------------------------------------------------
// CObjectTool - Texture ComboBox Message
//------------------------------------------------------------------------

void CObjectTool::OnDefault ()
{
if (object_list [CBObjType ()->GetCurSel ()] != OBJ_ROBOT)
	return;
INT32 i = INT32 (CBObjId ()->GetItemData (CBObjId ()->GetCurSel ()));
memcpy (theMine->RobotInfo (i), theMine->DefRobotInfo (i), sizeof (tRobotInfo));
Refresh ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundExplode ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundSee ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundAttack ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundClaw ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetSoundDeath ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetObjClassAI ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetWeapon1 ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetWeapon2 ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetContType ()
{
INT32 i = CBContType ()->GetCurSel ();
if (0 > i)
	return;
INT32 j = INT32 (CBObjId ()->GetItemData (CBObjId ()->GetCurSel ()));
CRobotInfo *rInfo = theMine->RobotInfo (j);
rInfo->m_info.contents.type = (UINT8) CBContType ()->GetItemData (i);
RefreshRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSetContId ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIKamikaze ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAICompanion ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIThief ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAISmartBlobs ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIPursue ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAICharge ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIEDrain ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIEndsLevel ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnAIBossType ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnBright ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnCloaked ()
{
UpdateRobot ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnMultiplayer ()
{
theMine->CurrObj ()->m_info.multiplayer = BtnCtrl (IDC_OBJ_MULTIPLAYER)->GetCheck ();
Refresh ();
}

                        /*--------------------------*/

afx_msg void CObjectTool::OnSort ()
{
if (theMine->m_bSortObjects = BtnCtrl (IDC_OBJ_SORT)->GetCheck ()) {
	theMine->SortObjects ();
	Refresh ();
	}
}

                        /*--------------------------*/

INT32 CObjectTool::ObjOfAKindCount (INT32 nType, INT32 nId)
{
if (nType < 0)
	nType = theMine->CurrObj ()->m_info.type;
if (nId < 0)
	nId =  theMine->CurrObj ()->m_info.id;
INT32 nCount = 0;
CGameObject *objP = theMine->Objects (0);
INT32 i;
for (i = theMine->GameInfo ().objects.count; i; i--, objP++)
	if ((objP->m_info.type == nType) && ((objP->m_info.type == OBJ_PLAYER) || (objP->m_info.type == OBJ_COOP) || (objP->m_info.id == nId))) 
		nCount++;
return nCount;
}

                        /*--------------------------*/

INT32 fix_log(FIX x) 
{
return (x >= 1) ? (INT32) (log ((double) x) + 0.5): 0; // round (assume value is positive)
}

                        /*--------------------------*/

FIX fix_exp(INT32 x) 
{
return (x >= 0 && x <= 21) ? (FIX) (exp ((double) x) + 0.5): 1; // round (assume value is positive)
}

                        /*--------------------------*/

//eof objectdlg.cpp