
#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>

#include "mine.h"
#include "dle-xp.h"
#include "toolview.h"

//------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CLightningEffectTool, CEffectTabDlg)
	ON_WM_PAINT ()
	ON_WM_HSCROLL ()
	ON_WM_VSCROLL ()
	ON_BN_CLICKED (IDC_EFFECT_ENABLED, OnEdit)

	ON_EN_KILLFOCUS (IDC_LIGHTNING_ID, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_TARGET, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_BOLTS, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_NODES, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_CHILDREN, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_LIFE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_DELAY, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_LENGTH, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_WIDTH, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_AMPLITUDE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_OFFSET, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_SPEED, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_ANGLE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_WAYPOINT, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_RED, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_GREEN, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_BLUE, OnEdit)
	ON_EN_KILLFOCUS (IDC_LIGHTNING_ALPHA, OnEdit)

	//ON_EN_CHANGE (IDC_LIGHTNING_ID, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_TARGET, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_BOLTS, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_NODES, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_CHILDREN, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_LIFE, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_DELAY, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_LENGTH, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_WIDTH, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_AMPLITUDE, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_OFFSET, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_SPEED, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_ANGLE, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_WAYPOINT, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_RED, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_GREEN, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_BLUE, OnEdit)
	//ON_EN_CHANGE (IDC_LIGHTNING_ALPHA, OnEdit)

	ON_BN_CLICKED (IDC_LIGHTNING_SMOOTHE, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_CLAMP, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_SOUND, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_PLASMA, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_RANDOM, OnEdit)
	ON_BN_CLICKED (IDC_LIGHTNING_SELECT_COLOR, OnSelectColor)
	ON_CBN_SELCHANGE (IDC_LIGHTNING_STYLE, OnEdit) //OnSetStyle)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------------

BOOL CLightningEffectTool::OnInitDialog ()
{
if (!CEffectTabDlg::OnInitDialog ())
	return FALSE;
CComboBox *pcb = CBStyle ();
pcb->AddString ("automatic");
pcb->AddString ("erratic");
pcb->AddString ("jaggy");
pcb->AddString ("smoothe");
pcb->SetCurSel (0);

m_data [0].Init (this, 0, IDC_LIGHTNING_BOLTS_SPINNER, IDC_LIGHTNING_BOLTS, 1, 1000);
m_data [1].Init (this, 0, IDC_LIGHTNING_NODES_SPINNER, IDC_LIGHTNING_NODES, 1, 1000);
m_data [2].Init (this, 0, IDC_LIGHTNING_CHILDREN_SPINNER, IDC_LIGHTNING_CHILDREN, 0, 1000);
m_data [3].Init (this, 0, IDC_LIGHTNING_LIFE_SPINNER, IDC_LIGHTNING_LIFE, 1, 1000000000, 50.0);
m_data [4].Init (this, 0, IDC_LIGHTNING_DELAY_SPINNER, IDC_LIGHTNING_DELAY, 1, 1000000000, 50.0);
m_data [5].Init (this, 0, IDC_LIGHTNING_LENGTH_SPINNER, IDC_LIGHTNING_LENGTH, 1, 1000);
m_data [6].Init (this, 0, IDC_LIGHTNING_WIDTH_SPINNER, IDC_LIGHTNING_WIDTH, 1, 100);
m_data [7].Init (this, 0, IDC_LIGHTNING_AMPLITUDE_SPINNER, IDC_LIGHTNING_AMPLITUDE, 1, 100);
m_data [8].Init (this, 0, IDC_LIGHTNING_SPEED_SPINNER, IDC_LIGHTNING_SPEED, 1, 100);
m_data [9].Init (this, 0, IDC_LIGHTNING_ANGLE_SPINNER, IDC_LIGHTNING_ANGLE, 0, 360);
m_data [10].Init (this, 0, IDC_LIGHTNING_OFFSET_SPINNER, IDC_LIGHTNING_OFFSET, 1, 1000);
for (int i = 11, nId = IDC_LIGHTNING_RED_SLIDER; i < 15; i++, nId += 3)
	m_data [i].Init (this, nId, nId + 1, nId + 2, 0, 255, 1.0, 1.0, 1);

CreateColorCtrl (&m_colorWnd, IDC_LIGHTNING_COLOR);

m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------------

void CLightningEffectTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;
CGameObject *objP = GetEffect (null, false);
EnableControls (objP != null);
if (objP) {
	objP->rType.lightningInfo.nId = DDX_Int (pDX, IDC_LIGHTNING_ID, objP->rType.lightningInfo.nId);
	objP->rType.lightningInfo.nTarget = DDX_Int (pDX, IDC_LIGHTNING_TARGET, objP->rType.lightningInfo.nTarget);
	objP->rType.lightningInfo.nWayPoint = DDX_Int (pDX, IDC_LIGHTNING_WAYPOINT, objP->rType.lightningInfo.nWayPoint);
	m_data [0].DoDataExchange (pDX, objP->rType.lightningInfo.nBolts);
	m_data [1].DoDataExchange (pDX, objP->rType.lightningInfo.nNodes);
	m_data [2].DoDataExchange (pDX, objP->rType.lightningInfo.nChildren);
	m_data [3].DoDataExchange (pDX, objP->rType.lightningInfo.nLife);
	m_data [4].DoDataExchange (pDX, objP->rType.lightningInfo.nDelay);
	m_data [5].DoDataExchange (pDX, objP->rType.lightningInfo.nLength);
	m_data [6].DoDataExchange (pDX, objP->rType.lightningInfo.nWidth);
	m_data [7].DoDataExchange (pDX, objP->rType.lightningInfo.nAmplitude);
	m_data [8].DoDataExchange (pDX, objP->rType.lightningInfo.nFrames);
	m_data [9].DoDataExchange (pDX, objP->rType.lightningInfo.nAngle);
	m_data [10].DoDataExchange (pDX, objP->rType.lightningInfo.nOffset);
	for (int i = 0; i < 4; i++)
		m_data [11 + i].DoDataExchange (pDX, objP->rType.lightningInfo.color [i]);
	objP->rType.lightningInfo.nSmoothe = DDX_Flag (pDX, IDC_LIGHTNING_SMOOTHE, objP->rType.lightningInfo.nSmoothe);
	objP->rType.lightningInfo.bClamp = DDX_Flag (pDX, IDC_LIGHTNING_CLAMP, objP->rType.lightningInfo.bClamp);
	objP->rType.lightningInfo.bSound = DDX_Flag (pDX, IDC_LIGHTNING_SOUND, objP->rType.lightningInfo.bSound);
	objP->rType.lightningInfo.bPlasma = DDX_Flag (pDX, IDC_LIGHTNING_PLASMA, objP->rType.lightningInfo.bPlasma);
	objP->rType.lightningInfo.bRandom = DDX_Flag (pDX, IDC_LIGHTNING_RANDOM, objP->rType.lightningInfo.bRandom);
	objP->rType.lightningInfo.bInPlane = DDX_Flag (pDX, IDC_LIGHTNING_INPLANE, objP->rType.lightningInfo.bInPlane);
	objP->rType.lightningInfo.bEnabled = DDX_Flag (pDX, IDC_EFFECT_ENABLED, objP->rType.lightningInfo.bEnabled);
	if (pDX->m_bSaveAndValidate)
		objP->rType.lightningInfo.nStyle = ubyte (CBStyle ()->GetCurSel () - 1);
	else {
		CBStyle ()->SetCurSel (ubyte (objP->rType.lightningInfo.nStyle + 1));
		HiliteTarget ();
		}
	}
}

//------------------------------------------------------------------------

void CLightningEffectTool::EnableControls (BOOL bEnable)
{
if (!(m_bInited && theMine))
	return;
CDlgHelpers::EnableControls (IDC_LIGHTNING_ID, IDC_LIGHTNING_SELECT_COLOR, GetEffect (null, false) != null);
}

//------------------------------------------------------------------------

void CLightningEffectTool::Add (void)
{
if (!AddEffect ())
	return;
CGameObject *objP = current->Object ();
objP->Type () = OBJ_EFFECT;
objP->Id () = LIGHTNING_ID;
objP->m_info.movementType = MT_NONE;
objP->m_info.controlType = CT_NONE;
objP->m_info.renderType = RT_LIGHTNING;
memset (&objP->rType.lightningInfo, 0, sizeof (objP->rType.lightningInfo));
objP->rType.lightningInfo.nWayPoint = -1;
Refresh ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CLightningEffectTool::Copy (void)
{
CGameObject *objP = GetEffect ();
if (objP) {
	m_lightning = objP->rType.lightningInfo;
	m_bValid = 1;
	}
}

//------------------------------------------------------------------------

void CLightningEffectTool::Paste (CGameObject* objP, bool bRefresh)
{
if (Valid () && (objP = GetEffect (objP))) {
	objP->rType.lightningInfo = m_lightning;
	if (bRefresh)
		Refresh ();
	}
}

//------------------------------------------------------------------------

bool CLightningEffectTool::Refresh (void)
{
UpdateColorCtrl ();
UpdateData (FALSE);
return true;
}

//------------------------------------------------------------------------

void CLightningEffectTool::OnSelectColor ()
{
CGameObject *objP = GetEffect (null, false);
if (objP && CDlgHelpers::SelectColor (BYTE (objP->rType.lightningInfo.color [0]), BYTE (objP->rType.lightningInfo.color [1]), BYTE (objP->rType.lightningInfo.color [2])))
	UpdateColorCtrl (true);
}

//------------------------------------------------------------------------

void CLightningEffectTool::OnSetStyle ()
{
CGameObject *objP = GetEffect ();
if (objP)
	objP->rType.lightningInfo.nStyle = CBStyle ()->GetCurSel () - 1;
//Refresh ();
}

//------------------------------------------------------------------------

void CLightningEffectTool::HiliteTarget (void)
{
#if 0
	int i, nTarget;

CGameObject *objP = current->Object ();
if ((objP->Type () != OBJ_EFFECT) || (objP->Id () != LIGHTNING_ID))
	return;
other->m_nObject = current->m_nObject;
if (nTarget = objP->rType.lightningInfo.nTarget)
	for (i = 0, objP = objectManager.Object (0); i < objectManager.Count (); i++, objP++)
		if ((objP->Type () == OBJ_EFFECT) && (objP->Id () == LIGHTNING_ID) && (objP->rType.lightningInfo.nId == nTarget)) {
			other->m_nObject = i;
			break;
			return;
			}
DLE.MineView ()->Refresh ();
#endif
}
 
//------------------------------------------------------------------------

void CLightningEffectTool::OnEdit (void)
{
UpdateData (TRUE);
}

//------------------------------------------------------------------------

bool CLightningEffectTool::OnExtSlider (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
for (int i = 0; i < 15; i++) {
	if (m_data [i].OnScroll (scrollCode, thumbPos, pScrollBar)) {
		UpdateColorCtrl ();
		UpdateData (TRUE);
		return true;
		}
	}
return false;
}

//------------------------------------------------------------------------

void CLightningEffectTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
if (OnExtSlider (scrollCode, thumbPos, pScrollBar))
	UpdateData (TRUE);
else
	CEffectTabDlg::OnHScroll (scrollCode, thumbPos, pScrollBar);
}

//------------------------------------------------------------------------

void CLightningEffectTool::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
if (OnExtSlider (scrollCode, thumbPos, pScrollBar))
	UpdateData (TRUE);
else
	CEffectTabDlg::OnVScroll (scrollCode, thumbPos, pScrollBar);
}

//------------------------------------------------------------------------

void CLightningEffectTool::UpdateColorCtrl (bool bSave)
{
CGameObject *objP = GetEffect (null, false);
CDlgHelpers::UpdateColorCtrl (&m_colorWnd, objP ? RGB (BYTE (objP->rType.lightningInfo.color [0]), BYTE (objP->rType.lightningInfo.color [1]), BYTE (objP->rType.lightningInfo.color [2])) : RGB (0,0,0));
if (bSave && (objP != NULL)) {
	for (int i = 0; i < 3; i++)
		m_data [11 + i].SetValue (objP->rType.lightningInfo.color [i]);
	}
}

//------------------------------------------------------------------------

void CLightningEffectTool::OnPaint ()
{
if (theMine) {
	CEffectTabDlg::OnPaint ();
	UpdateColorCtrl ();
	}
}

//------------------------------------------------------------------------
//eof LightningEffectTool.cpp