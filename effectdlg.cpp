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

GetMine ();

cbEffects->ResetContent ();
CGameObject *curObj = m_mine->CurrObj (),
			*objP = m_mine->Objects ();
INT32 i;
for (i = 0; i < m_mine->GameInfo ().objects.count; i++, objP++) {
	if (objP->type != OBJ_EFFECT)
		continue;
	if (objP == curObj)
		curSel = i;
	if (objP->id == SMOKE_ID)
		sprintf_s (szEffect, sizeof (szEffect), "Smoke");
	else if (objP->id == LIGHTNING_ID)
		sprintf_s (szEffect, sizeof (szEffect), "Lightning %d (%d)", objP->rtype.lightningInfo.nId, i);
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
CToolDlg::OnInitDialog ();
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
if (!m_bInited)
	return;
if (!GetMine ())
	return;
CGameObject *objP = m_mine->CurrObj ();
if (objP->type != OBJ_EFFECT)
	return;
if (objP->id == SMOKE_ID) {
	objP->rtype.smokeInfo.bEnabled = DDX_Flag (pDX, IDC_EFFECT_ENABLED, objP->rtype.smokeInfo.bEnabled);
	DDX_Slider (pDX, IDC_SMOKE_LIFE, objP->rtype.smokeInfo.nLife);
	DDX_Slider (pDX, IDC_SMOKE_SIZE, objP->rtype.smokeInfo.nSize [0]);
	DDX_Slider (pDX, IDC_SMOKE_DENSITY, objP->rtype.smokeInfo.nParts);
	DDX_Slider (pDX, IDC_SMOKE_SPEED, objP->rtype.smokeInfo.nSpeed);
	DDX_Slider (pDX, IDC_SMOKE_DRIFT, objP->rtype.smokeInfo.nDrift);
	DDX_Slider (pDX, IDC_SMOKE_BRIGHTNESS, objP->rtype.smokeInfo.nBrightness);
	INT32 i;
	for (i = 0; i < 4; i++)
		DDX_Text (pDX, IDC_SMOKE_RED + i, objP->rtype.smokeInfo.color [i]);
	objP->rtype.smokeInfo.nSide = (char) DDX_Int (pDX, IDC_SMOKE_SIDE, (INT32) objP->rtype.smokeInfo.nSide);
	if (pDX->m_bSaveAndValidate)
		objP->rtype.smokeInfo.nType = CBType ()->GetCurSel ();
	else
		CBType ()->SetCurSel (objP->rtype.smokeInfo.nType);
	}
else if (objP->id == LIGHTNING_ID) {
	objP->rtype.lightningInfo.nId = DDX_Int (pDX, IDC_LIGHTNING_ID, objP->rtype.lightningInfo.nId);
	objP->rtype.lightningInfo.nTarget = DDX_Int (pDX, IDC_LIGHTNING_TARGET, objP->rtype.lightningInfo.nTarget);
	objP->rtype.lightningInfo.nLightnings = DDX_Int (pDX, IDC_LIGHTNING_BOLTS, objP->rtype.lightningInfo.nLightnings);
	objP->rtype.lightningInfo.nNodes = DDX_Int (pDX, IDC_LIGHTNING_NODES, objP->rtype.lightningInfo.nNodes);
	objP->rtype.lightningInfo.nChildren = DDX_Int (pDX, IDC_LIGHTNING_CHILDREN, objP->rtype.lightningInfo.nChildren);
	objP->rtype.lightningInfo.nLife = DDX_Int (pDX, IDC_LIGHTNING_LIFE, objP->rtype.lightningInfo.nLife);
	objP->rtype.lightningInfo.nDelay = DDX_Int (pDX, IDC_LIGHTNING_DELAY, objP->rtype.lightningInfo.nDelay);
	objP->rtype.lightningInfo.nLength = DDX_Int (pDX, IDC_LIGHTNING_LENGTH, objP->rtype.lightningInfo.nLength);
	objP->rtype.lightningInfo.nAmplitude = DDX_Int (pDX, IDC_LIGHTNING_AMPLITUDE, objP->rtype.lightningInfo.nAmplitude);
	objP->rtype.lightningInfo.nSteps = DDX_Int (pDX, IDC_LIGHTNING_SPEED, objP->rtype.lightningInfo.nSteps);
	objP->rtype.lightningInfo.nAngle = DDX_Int (pDX, IDC_LIGHTNING_ANGLE, objP->rtype.lightningInfo.nAngle);
	objP->rtype.lightningInfo.nOffset = DDX_Int (pDX, IDC_LIGHTNING_OFFSET, objP->rtype.lightningInfo.nOffset);
	INT32 i;
	for (i = 0; i < 4; i++)
		DDX_Text (pDX, IDC_LIGHTNING_RED + i, objP->rtype.lightningInfo.color [i]);
	objP->rtype.lightningInfo.bEnabled = DDX_Flag (pDX, IDC_EFFECT_ENABLED, objP->rtype.lightningInfo.bEnabled);
	objP->rtype.lightningInfo.nSmoothe = DDX_Flag (pDX, IDC_LIGHTNING_SMOOTHE, objP->rtype.lightningInfo.nSmoothe);
	objP->rtype.lightningInfo.bClamp = DDX_Flag (pDX, IDC_LIGHTNING_CLAMP, objP->rtype.lightningInfo.bClamp);
	objP->rtype.lightningInfo.bSound = DDX_Flag (pDX, IDC_LIGHTNING_SOUND, objP->rtype.lightningInfo.bSound);
	objP->rtype.lightningInfo.bPlasma = DDX_Flag (pDX, IDC_LIGHTNING_PLASMA, objP->rtype.lightningInfo.bPlasma);
	objP->rtype.lightningInfo.bRandom = DDX_Flag (pDX, IDC_LIGHTNING_RANDOM, objP->rtype.lightningInfo.bRandom);
	objP->rtype.lightningInfo.bInPlane = DDX_Flag (pDX, IDC_LIGHTNING_INPLANE, objP->rtype.lightningInfo.bInPlane);
	if (pDX->m_bSaveAndValidate)
		objP->rtype.lightningInfo.nStyle = CBStyle ()->GetCurSel () - 1;
	else {
		CBStyle ()->SetCurSel (objP->rtype.lightningInfo.nStyle + 1);
		HiliteTarget ();
		}
	}
else if (objP->id == SOUND_ID) {
	objP->rtype.soundInfo.bEnabled = DDX_Flag (pDX, IDC_EFFECT_ENABLED, objP->rtype.soundInfo.bEnabled);
	DDX_Text (pDX, IDC_SOUND_FILE, objP->rtype.soundInfo.szFilename, sizeof (objP->rtype.soundInfo.szFilename));
	DDX_Slider (pDX, IDC_SOUND_VOLUME, objP->rtype.soundInfo.nVolume);
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
if (!m_bInited)
	return;
if (!GetMine ())
	return;
CGameObject *objP = m_mine->CurrObj ();
CToolDlg::EnableControls (IDC_SMOKE_LIFE, IDC_SMOKE_BRIGHTNESS, (objP->type == OBJ_EFFECT) && (objP->id == SMOKE_ID));
CToolDlg::EnableControls (IDC_LIGHTNING_ID, IDC_LIGHTNING_RANDOM, (objP->type == OBJ_EFFECT) && (objP->id == LIGHTNING_ID));
CToolDlg::EnableControls (IDC_SOUND_FILE, IDC_SOUND_VOLUME, (objP->type == OBJ_EFFECT) && (objP->id == SOUND_ID));
}

//------------------------------------------------------------------------
// CEffectTool - RefreshData
//------------------------------------------------------------------------

void CEffectTool::Refresh ()
{
if (!m_bInited)
	return;
if (!GetMine ())
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
if (m_mine->GameInfo ().objects.count >= MAX_OBJECTS (m_mine)) {
	ErrorMsg ("Maximum numbers of objects reached");
	return false;
	}
UpdateData (TRUE);
m_mine->CopyObject (OBJ_EFFECT);
return true;
}

//------------------------------------------------------------------------

void CEffectTool::OnAddSmoke ()
{
if (!GetMine ())
	return;
if (!AddEffect ())
	return;
CGameObject *objP = m_mine->CurrObj ();
objP->type = OBJ_EFFECT;
objP->id = SMOKE_ID;
objP->render_type = RT_SMOKE;
memset (&objP->rtype.smokeInfo, 0, sizeof (objP->rtype.smokeInfo));
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnAddLightning ()
{
if (!GetMine ())
	return;
if (!AddEffect ())
	return;
CGameObject *objP = m_mine->CurrObj ();
objP->type = OBJ_EFFECT;
objP->id = LIGHTNING_ID;
objP->render_type = RT_LIGHTNING;
memset (&objP->rtype.lightningInfo, 0, sizeof (objP->rtype.lightningInfo));
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnAddSound ()
{
if (!GetMine ())
	return;
if (!AddEffect ())
	return;
CGameObject *objP = m_mine->CurrObj ();
objP->type = OBJ_EFFECT;
objP->id = SOUND_ID;
objP->render_type = RT_SOUND;
*objP->rtype.soundInfo.szFilename = '\0';
objP->rtype.soundInfo.nVolume = 10;
Refresh ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnDelete ()
{
if (!GetMine ())
	return;
if (m_mine->Current ()->nObject == m_mine->GameInfo ().objects.count) {
	ErrorMsg ("Cannot delete the secret return.");
	return;
	}
if (m_mine->GameInfo ().objects.count == 1) {
	ErrorMsg ("Cannot delete the last object");
	return;
	}
if (m_mine->CurrObj ()->type != OBJ_EFFECT) {
	ErrorMsg ("No effect object currently selected");
	return;
	}
if (QueryMsg ("Are you sure you want to delete this object?") == IDYES) {
	m_mine->DeleteObject ();
	Refresh ();
	theApp.MineView ()->Refresh (false);
	}
}

//------------------------------------------------------------------------

void CEffectTool::OnCopy ()
{
if (!GetMine ())
	return;
CGameObject *objP = m_mine->CurrObj ();
if (objP->type != OBJ_EFFECT) {
	ErrorMsg ("No effect object currently selected");
	return;
	}
m_nBufferId = objP->id;
if (m_nBufferId == SMOKE_ID)
	m_smoke = objP->rtype.smokeInfo;
else if (m_nBufferId == LIGHTNING_ID)
	m_lightning = objP->rtype.lightningInfo;
else if (m_nBufferId == SOUND_ID)
	m_sound = objP->rtype.soundInfo;
}

//------------------------------------------------------------------------

void CEffectTool::OnPaste ()
{
if (!GetMine ())
	return;
CGameObject *objP = m_mine->CurrObj ();
if (objP->type != OBJ_EFFECT) {
	ErrorMsg ("No effect object currently selected");
	return;
	}
if (objP->id != m_nBufferId) {
	ErrorMsg ("No effect data of that type currently available (copy data first)");
	return;
	}
if (objP->id == SMOKE_ID)
	objP->rtype.smokeInfo = m_smoke;
else if (objP->id == LIGHTNING_ID)
	objP->rtype.lightningInfo = m_lightning;
else if (objP->id == SOUND_ID)
	objP->rtype.soundInfo = m_sound;
Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnPasteAll ()
{
if (m_nBufferId < 0) {
	ErrorMsg ("No effect data currently available (copy data first)");
	return;
	}
if (!GetMine ())
	return;
CGameObject *objP = m_mine->Objects ();
boolean bAll = !m_mine->GotMarkedSegments ();

INT32 i;
for (i = m_mine->ObjCount (); i; i--, objP++)
	if ((objP->type == OBJ_EFFECT) && (objP->id == m_nBufferId) && (bAll || m_mine->SegmentIsMarked (objP->nSegment)))
		if (m_nBufferId == SMOKE_ID)
			objP->rtype.smokeInfo = m_smoke;
		else if (m_nBufferId == LIGHTNING_ID)
			objP->rtype.lightningInfo = m_lightning;
		else if (m_nBufferId == SOUND_ID)
			objP->rtype.soundInfo = m_sound;
Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnSetObject ()
{
if (!GetMine ())
	return;
INT16 nOld = m_mine->Current ()->nObject;
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
if (!GetMine ())
	return;
CGameObject *objP = m_mine->CurrObj ();
objP->rtype.lightningInfo.nStyle = CBStyle ()->GetCurSel () - 1;
//Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::HiliteTarget (void)
{
#if 0
	INT32 i, nTarget;

if (!GetMine ())
	return;
CGameObject *objP = m_mine->CurrObj ();
if ((objP->type != OBJ_EFFECT) || (objP->id != LIGHTNING_ID))
	return;
m_mine->Other ()->nObject = m_mine->Current ()->nObject;
if (nTarget = objP->rtype.lightningInfo.nTarget)
	for (i = 0, objP = m_mine->Objects (); i < m_mine->GameInfo ().objects.count; i++, objP++)
		if ((objP->type == OBJ_EFFECT) && (objP->id == LIGHTNING_ID) && (objP->rtype.lightningInfo.nId == nTarget)) {
			m_mine->Other ()->nObject = i;
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