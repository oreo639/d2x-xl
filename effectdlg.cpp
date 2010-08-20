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

BEGIN_MESSAGE_MAP (CEffectTool, CToolDlg)
	ON_WM_HSCROLL ()
#if 1
	ON_CBN_SELCHANGE (IDC_SMOKE_LIFE, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_SIZE, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_DENSITY, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_SPEED, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_DRIFT, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_BRIGHTNESS, OnEdit)
	ON_CBN_SELCHANGE (IDC_SMOKE_TYPE, OnEdit)
	ON_EN_KILLFOCUS (IDC_SMOKE_SIDE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_ID, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_TARGET, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_BOLTS, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_NODES, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_CHILDREN, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_LIFE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_DELAY, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_LENGTH, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_AMPLITUDE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_ANGLE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_OFFSET, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_SPEED, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_RED, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_GREEN, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_BLUE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_ALPHA, OnEdit)
	ON_EN_KILLFOCUS (IDC_SOUND_FILE, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_SMOOTHE, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_CLAMP, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_SOUND, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_PLASMA, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_RANDOM, OnEdit)
#endif
	ON_BN_CLICKED (IDC_SMOKE_ADD, OnAddSmoke)
	ON_BN_CLICKED (IDC_LIGHTNING_ADD, OnAddLightning)
	ON_BN_CLICKED (IDC_SOUND_ADD, OnAddSound)
	ON_BN_CLICKED (IDC_EFFECT_ENABLED, OnEdit)
	ON_BN_CLICKED (IDC_EFFECT_DELETE, OnDelete)
	ON_BN_CLICKED (IDC_EFFECT_COPY, OnCopy)
	ON_BN_CLICKED (IDC_EFFECT_PASTE, OnPaste)
	ON_BN_CLICKED (IDC_EFFECT_PASTE_ALL, OnPasteAll)
	ON_CBN_SELCHANGE (IDC_EFFECT_OBJECTS, OnSetObject)
	ON_CBN_SELCHANGE (IDC_LIGHTNING_STYLE, OnSetStyle)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------
// DIALOG - CEffectTool (constructor)
//------------------------------------------------------------------------

CEffectTool::CEffectTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_EFFECTDATA2 : IDD_EFFECTDATA, pParent)
{
Reset ();
}

								/*--------------------------*/

CEffectTool::~CEffectTool ()
{
}

								/*--------------------------*/

void CEffectTool::Reset ()
{
m_nBufferId = -1;
}

                        /*--------------------------*/

void CEffectTool::LoadEffectList () 
{
	HINSTANCE	hInst = AfxGetApp()->m_hInstance;
	CComboBox	*cbEffects = CBEffects ();
	char			szEffect [100];
	INT32			index, curSel = 0;

cbEffects->ResetContent ();
CGameObject *curObj = theMine->CurrObj (),
			*objP = theMine->Objects (0);
INT32 i;
for (i = 0; i < theMine->GameInfo ().objects.count; i++, objP++) {
	if (objP->type != OBJ_EFFECT)
		continue;
	if (objP == curObj)
		curSel = i;
	if (objP->id == SMOKE_ID)
		sprintf_s (szEffect, sizeof (szEffect), "Smoke");
	else if (objP->id == LIGHTNING_ID)
		sprintf_s (szEffect, sizeof (szEffect), "Lightning %d (%d)", objP->rType.lightningInfo.nId, i);
	else if (objP->id == SOUND_ID)
		sprintf_s (szEffect, sizeof (szEffect), "Sound (%d)", i);
	else
		continue;
	index = cbEffects->AddString (szEffect);
	cbEffects->SetItemData (index, i);
	}
SelectItemData (cbEffects, curSel);
}

                        /*--------------------------*/

BOOL CEffectTool::OnInitDialog ()
{
if (!(theMine && CToolDlg::OnInitDialog ()))
	return FALSE;
INT32 i, nId;
for (nId = IDC_SMOKE_LIFE; nId <= IDC_SMOKE_BRIGHTNESS; nId++) {
	InitSlider (nId, 1, 10);
	for (i = 1; i <= 10; i++)
		SlCtrl (nId)->SetTic (i);
	}
InitSlider (IDC_SOUND_VOLUME, 1, 10);
for (i = 1; i <= 10; i++)
	SlCtrl (IDC_SOUND_VOLUME)->SetTic (i);
CComboBox *pcb = CBStyle ();
pcb->AddString ("automatic");
pcb->AddString ("erratic");
pcb->AddString ("jaggy");
pcb->AddString ("smoothe");
pcb->SetCurSel (0);
pcb = CBType ();
pcb->AddString ("Smoke");
pcb->AddString ("Spray");
pcb->AddString ("Bubbles");
pcb->AddString ("Fire");
pcb->AddString ("Waterfall");
pcb->SetCurSel (0);
m_bInited = TRUE;
return TRUE;
}

								/*--------------------------*/

void CEffectTool::DoDataExchange (CDataExchange *pDX)
{
if (!(m_bInited && theMine))
	return;
CGameObject *objP = theMine->CurrObj ();
if (objP->type != OBJ_EFFECT)
	return;
if (objP->id == SMOKE_ID) {
	objP->rType.smokeInfo.bEnabled = DDX_Flag (pDX, IDC_EFFECT_ENABLED, objP->rType.smokeInfo.bEnabled);
	DDX_Slider (pDX, IDC_SMOKE_LIFE, objP->rType.smokeInfo.nLife);
	DDX_Slider (pDX, IDC_SMOKE_SIZE, objP->rType.smokeInfo.nSize [0]);
	DDX_Slider (pDX, IDC_SMOKE_DENSITY, objP->rType.smokeInfo.nParts);
	DDX_Slider (pDX, IDC_SMOKE_SPEED, objP->rType.smokeInfo.nSpeed);
	DDX_Slider (pDX, IDC_SMOKE_DRIFT, objP->rType.smokeInfo.nDrift);
	DDX_Slider (pDX, IDC_SMOKE_BRIGHTNESS, objP->rType.smokeInfo.nBrightness);
	INT32 i;
	for (i = 0; i < 4; i++)
		DDX_Text (pDX, IDC_SMOKE_RED + i, objP->rType.smokeInfo.color [i]);
	objP->rType.smokeInfo.nSide = (char) DDX_Int (pDX, IDC_SMOKE_SIDE, (INT32) objP->rType.smokeInfo.nSide);
	if (pDX->m_bSaveAndValidate)
		objP->rType.smokeInfo.nType = CBType ()->GetCurSel ();
	else
		CBType ()->SetCurSel (objP->rType.smokeInfo.nType);
	}
else if (objP->id == LIGHTNING_ID) {
	objP->rType.lightningInfo.nId = DDX_Int (pDX, IDC_LIGHTNING_ID, objP->rType.lightningInfo.nId);
	objP->rType.lightningInfo.nTarget = DDX_Int (pDX, IDC_LIGHTNING_TARGET, objP->rType.lightningInfo.nTarget);
	objP->rType.lightningInfo.nLightnings = DDX_Int (pDX, IDC_LIGHTNING_BOLTS, objP->rType.lightningInfo.nLightnings);
	objP->rType.lightningInfo.nNodes = DDX_Int (pDX, IDC_LIGHTNING_NODES, objP->rType.lightningInfo.nNodes);
	objP->rType.lightningInfo.nChildren = DDX_Int (pDX, IDC_LIGHTNING_CHILDREN, objP->rType.lightningInfo.nChildren);
	objP->rType.lightningInfo.nLife = DDX_Int (pDX, IDC_LIGHTNING_LIFE, objP->rType.lightningInfo.nLife);
	objP->rType.lightningInfo.nDelay = DDX_Int (pDX, IDC_LIGHTNING_DELAY, objP->rType.lightningInfo.nDelay);
	objP->rType.lightningInfo.nLength = DDX_Int (pDX, IDC_LIGHTNING_LENGTH, objP->rType.lightningInfo.nLength);
	objP->rType.lightningInfo.nAmplitude = DDX_Int (pDX, IDC_LIGHTNING_AMPLITUDE, objP->rType.lightningInfo.nAmplitude);
	objP->rType.lightningInfo.nSteps = DDX_Int (pDX, IDC_LIGHTNING_SPEED, objP->rType.lightningInfo.nSteps);
	objP->rType.lightningInfo.nAngle = DDX_Int (pDX, IDC_LIGHTNING_ANGLE, objP->rType.lightningInfo.nAngle);
	objP->rType.lightningInfo.nOffset = DDX_Int (pDX, IDC_LIGHTNING_OFFSET, objP->rType.lightningInfo.nOffset);
	INT32 i;
	for (i = 0; i < 4; i++)
		DDX_Text (pDX, IDC_LIGHTNING_RED + i, objP->rType.lightningInfo.color [i]);
	objP->rType.lightningInfo.bEnabled = DDX_Flag (pDX, IDC_EFFECT_ENABLED, objP->rType.lightningInfo.bEnabled);
	objP->rType.lightningInfo.nSmoothe = DDX_Flag (pDX, IDC_LIGHTNING_SMOOTHE, objP->rType.lightningInfo.nSmoothe);
	objP->rType.lightningInfo.bClamp = DDX_Flag (pDX, IDC_LIGHTNING_CLAMP, objP->rType.lightningInfo.bClamp);
	objP->rType.lightningInfo.bSound = DDX_Flag (pDX, IDC_LIGHTNING_SOUND, objP->rType.lightningInfo.bSound);
	objP->rType.lightningInfo.bPlasma = DDX_Flag (pDX, IDC_LIGHTNING_PLASMA, objP->rType.lightningInfo.bPlasma);
	objP->rType.lightningInfo.bRandom = DDX_Flag (pDX, IDC_LIGHTNING_RANDOM, objP->rType.lightningInfo.bRandom);
	objP->rType.lightningInfo.bInPlane = DDX_Flag (pDX, IDC_LIGHTNING_INPLANE, objP->rType.lightningInfo.bInPlane);
	if (pDX->m_bSaveAndValidate)
		objP->rType.lightningInfo.nStyle = CBStyle ()->GetCurSel () - 1;
	else {
		CBStyle ()->SetCurSel (objP->rType.lightningInfo.nStyle + 1);
		HiliteTarget ();
		}
	}
else if (objP->id == SOUND_ID) {
	objP->rType.soundInfo.bEnabled = DDX_Flag (pDX, IDC_EFFECT_ENABLED, objP->rType.soundInfo.bEnabled);
	DDX_Text (pDX, IDC_SOUND_FILE, objP->rType.soundInfo.szFilename, sizeof (objP->rType.soundInfo.szFilename));
	DDX_Slider (pDX, IDC_SOUND_VOLUME, objP->rType.soundInfo.nVolume);
	}
}

								/*--------------------------*/

BOOL CEffectTool::OnSetActive ()
{
Refresh ();
return TRUE; //CTexToolDlg::OnSetActive ();
}

								/*--------------------------*/

BOOL CEffectTool::OnKillActive ()
{
Refresh ();
return CToolDlg::OnKillActive ();
}

								/*--------------------------*/

void CEffectTool::EnableControls (BOOL bEnable)
{
if (!(m_bInited && theMine))
	return;
CGameObject *objP = theMine->CurrObj ();
CToolDlg::EnableControls (IDC_SMOKE_LIFE, IDC_SMOKE_BRIGHTNESS, (objP->type == OBJ_EFFECT) && (objP->id == SMOKE_ID));
CToolDlg::EnableControls (IDC_LIGHTNING_ID, IDC_LIGHTNING_RANDOM, (objP->type == OBJ_EFFECT) && (objP->id == LIGHTNING_ID));
CToolDlg::EnableControls (IDC_SOUND_FILE, IDC_SOUND_VOLUME, (objP->type == OBJ_EFFECT) && (objP->id == SOUND_ID));
}

//------------------------------------------------------------------------
// CEffectTool - RefreshData
//------------------------------------------------------------------------

void CEffectTool::Refresh ()
{
if (!(m_bInited && theMine))
	return;

EnableControls (true);
LoadEffectList ();
UpdateData (FALSE);
}

//------------------------------------------------------------------------
// CEffectTool - Add Effect
//------------------------------------------------------------------------

bool CEffectTool::AddEffect ()
{
if (theMine->GameInfo ().objects.count >= MAX_OBJECTS) {
	ErrorMsg ("Maximum numbers of objects reached");
	return false;
	}
UpdateData (TRUE);
theMine->CopyObject (OBJ_EFFECT);
return true;
}

//------------------------------------------------------------------------

void CEffectTool::OnAddSmoke ()
{
if (!AddEffect ())
	return;
CGameObject *objP = theMine->CurrObj ();
objP->type = OBJ_EFFECT;
objP->id = SMOKE_ID;
objP->render_type = RT_SMOKE;
memset (&objP->rType.smokeInfo, 0, sizeof (objP->rType.smokeInfo));
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnAddLightning ()
{
if (!AddEffect ())
	return;
CGameObject *objP = theMine->CurrObj ();
objP->type = OBJ_EFFECT;
objP->id = LIGHTNING_ID;
objP->render_type = RT_LIGHTNING;
memset (&objP->rType.lightningInfo, 0, sizeof (objP->rType.lightningInfo));
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnAddSound ()
{
if (!AddEffect ())
	return;
CGameObject *objP = theMine->CurrObj ();
objP->type = OBJ_EFFECT;
objP->id = SOUND_ID;
objP->render_type = RT_SOUND;
*objP->rType.soundInfo.szFilename = '\0';
objP->rType.soundInfo.nVolume = 10;
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnDelete ()
{
if (theMine->Current ()->nObject == theMine->GameInfo ().objects.count) {
	ErrorMsg ("Cannot delete the secret return.");
	return;
	}
if (theMine->GameInfo ().objects.count == 1) {
	ErrorMsg ("Cannot delete the last object");
	return;
	}
if (theMine->CurrObj ()->type != OBJ_EFFECT) {
	ErrorMsg ("No effect object currently selected");
	return;
	}
if (QueryMsg ("Are you sure you want to delete this object?") == IDYES) {
	theMine->DeleteObject ();
	Refresh ();
	theApp.MineView ()->Refresh (false);
	}
}

//------------------------------------------------------------------------

void CEffectTool::OnCopy ()
{
CGameObject *objP = theMine->CurrObj ();
if (objP->type != OBJ_EFFECT) {
	ErrorMsg ("No effect object currently selected");
	return;
	}
m_nBufferId = objP->id;
if (m_nBufferId == SMOKE_ID)
	m_smoke = objP->rType.smokeInfo;
else if (m_nBufferId == LIGHTNING_ID)
	m_lightning = objP->rType.lightningInfo;
else if (m_nBufferId == SOUND_ID)
	m_sound = objP->rType.soundInfo;
}

//------------------------------------------------------------------------

void CEffectTool::OnPaste ()
{
CGameObject *objP = theMine->CurrObj ();
if (objP->type != OBJ_EFFECT) {
	ErrorMsg ("No effect object currently selected");
	return;
	}
if (objP->id != m_nBufferId) {
	ErrorMsg ("No effect data of that type currently available (copy data first)");
	return;
	}
if (objP->id == SMOKE_ID)
	objP->rType.smokeInfo = m_smoke;
else if (objP->id == LIGHTNING_ID)
	objP->rType.lightningInfo = m_lightning;
else if (objP->id == SOUND_ID)
	objP->rType.soundInfo = m_sound;
Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnPasteAll ()
{
if (m_nBufferId < 0) {
	ErrorMsg ("No effect data currently available (copy data first)");
	return;
	}
CGameObject *objP = theMine->Objects (0);
boolean bAll = !theMine->GotMarkedSegments ();

INT32 i;
for (i = theMine->ObjCount (); i; i--, objP++)
	if ((objP->type == OBJ_EFFECT) && (objP->id == m_nBufferId) && (bAll || theMine->SegmentIsMarked (objP->nSegment)))
		if (m_nBufferId == SMOKE_ID)
			objP->rType.smokeInfo = m_smoke;
		else if (m_nBufferId == LIGHTNING_ID)
			objP->rType.lightningInfo = m_lightning;
		else if (m_nBufferId == SOUND_ID)
			objP->rType.soundInfo = m_sound;
Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnSetObject ()
{
INT16 nOld = theMine->Current ()->nObject;
INT16 nNew = INT16 (CBEffects ()->GetItemData (CBEffects ()->GetCurSel ()));
if (nOld != nNew) {
	UpdateData (TRUE);
	theApp.MineView ()->RefreshObject (nOld, nNew);
	HiliteTarget ();
	}
//Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnSetStyle ()
{
CGameObject *objP = theMine->CurrObj ();
objP->rType.lightningInfo.nStyle = CBStyle ()->GetCurSel () - 1;
//Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::HiliteTarget (void)
{
#if 0
	INT32 i, nTarget;

CGameObject *objP = theMine->CurrObj ();
if ((objP->type != OBJ_EFFECT) || (objP->id != LIGHTNING_ID))
	return;
theMine->Other ()->nObject = theMine->Current ()->nObject;
if (nTarget = objP->rType.lightningInfo.nTarget)
	for (i = 0, objP = theMine->Objects (0); i < theMine->GameInfo ().objects.count; i++, objP++)
		if ((objP->type == OBJ_EFFECT) && (objP->id == LIGHTNING_ID) && (objP->rType.lightningInfo.nId == nTarget)) {
			theMine->Other ()->nObject = i;
			break;
			return;
			}
theApp.MineView ()->Refresh ();
#endif
}
 
//------------------------------------------------------------------------

void CEffectTool::OnEdit (void)
{
UpdateData (TRUE);
}

                       /*--------------------------*/

bool CEffectTool::FindSlider (CScrollBar *pScrollBar)
{
for (INT32 i = IDC_SMOKE_LIFE; i <= IDC_SMOKE_BRIGHTNESS; i++)
	if (pScrollBar == (CScrollBar *) GetDlgItem (i))
		return true;
if (pScrollBar == (CScrollBar *) GetDlgItem (IDC_SOUND_VOLUME))
	return true;
return false;
}

								/*--------------------------*/

void CEffectTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
if (FindSlider (pScrollBar))
	UpdateData (TRUE);
}

//eof effectdlg.cpp