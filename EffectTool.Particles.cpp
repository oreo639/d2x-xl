
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

BEGIN_MESSAGE_MAP (CParticleEffectTool, CEffectTabDlg)
	ON_WM_PAINT ()
	ON_WM_HSCROLL ()
	ON_WM_VSCROLL ()
	ON_BN_CLICKED (IDC_PARTICLE_SELECT_COLOR, OnSelectColor)
	ON_BN_CLICKED (IDC_EFFECT_ENABLED, OnEdit)

	ON_EN_KILLFOCUS (IDC_PARTICLE_LIFE, OnEdit)
	ON_EN_KILLFOCUS (IDC_PARTICLE_SIZE, OnEdit)
	ON_EN_KILLFOCUS (IDC_PARTICLE_DENSITY, OnEdit)
	ON_EN_KILLFOCUS (IDC_PARTICLE_SPEED, OnEdit)
	ON_EN_KILLFOCUS (IDC_PARTICLE_DRIFT, OnEdit)
	ON_EN_KILLFOCUS (IDC_PARTICLE_BRIGHTNESS, OnEdit)
	ON_EN_KILLFOCUS (IDC_PARTICLE_RED, OnEdit)
	ON_EN_KILLFOCUS (IDC_PARTICLE_GREEN, OnEdit)
	ON_EN_KILLFOCUS (IDC_PARTICLE_BLUE, OnEdit)
	ON_EN_KILLFOCUS (IDC_PARTICLE_ALPHA, OnEdit)

	ON_CBN_SELCHANGE (IDC_PARTICLE_TYPE, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_LIFE, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_SIZE, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_DENSITY, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_SPEED, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_DRIFT, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_BRIGHTNESS, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_RED, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_GREEN, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_BLUE, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_ALPHA, OnEdit)
	//ON_EN_CHANGE (IDC_PARTICLE_SIDE, OnEdit)
	//ON_EN_KILLFOCUS (IDC_PARTICLE_TYPE, OnEdit)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------------

BOOL CParticleEffectTool::OnInitDialog ()
{
if (!CEffectTabDlg::OnInitDialog ())
	return FALSE;

CComboBox* pcb = CBType ();
pcb->AddString ("Smoke");
pcb->AddString ("Spray");
pcb->AddString ("Bubbles");
pcb->AddString ("Fire");
pcb->AddString ("Waterfall");
pcb->AddString ("Rain");
pcb->AddString ("Snow");
pcb->SetCurSel (0);

for (int i = 0, nId = IDC_PARTICLE_LIFE_SLIDER; i < 6; i++, nId += 3)
	m_data [i].Init (this, nId, nId + 1, nId + 2, 0, 10, 1.0, 1.0, 1);
for (int i = 6, nId = IDC_PARTICLE_RED_SLIDER; i < 10; i++, nId += 3)
	m_data [i].Init (this, nId, nId + 1, nId + 2, 0, 255, 1.0, 1.0, 1);
m_data [10].Init (this, 0, IDC_PARTICLE_SIDE_SPINNER, IDC_PARTICLE_SIDE, 0, 5, 1.0, 1.0, 1);

CreateColorCtrl (&m_colorWnd, IDC_PARTICLE_COLOR);

m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------

void CParticleEffectTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;
CGameObject *objP = GetEffect (null, false);
EnableControls (objP != null);
if (objP) {
	objP->rType.particleInfo.bEnabled = DDX_Flag (pDX, IDC_EFFECT_ENABLED, objP->rType.particleInfo.bEnabled);
	m_data [0].DoDataExchange (pDX, objP->rType.particleInfo.nLife);
	m_data [1].DoDataExchange (pDX, objP->rType.particleInfo.nSize [0]);
	m_data [2].DoDataExchange (pDX, objP->rType.particleInfo.nParts);
	m_data [3].DoDataExchange (pDX, objP->rType.particleInfo.nSpeed);
	m_data [4].DoDataExchange (pDX, objP->rType.particleInfo.nDrift);
	m_data [5].DoDataExchange (pDX, objP->rType.particleInfo.nBrightness);
	for (int i = 0; i < 4; i++)
		m_data [6 + i].DoDataExchange (pDX, objP->rType.particleInfo.color [i]);
	m_data [10].DoDataExchange (pDX, objP->rType.particleInfo.nSide);
	if (pDX->m_bSaveAndValidate)
		objP->rType.particleInfo.nType = CBType ()->GetCurSel ();
	else
		CBType ()->SetCurSel (objP->rType.particleInfo.nType);
	}
}

//------------------------------------------------------------------------

void CParticleEffectTool::EnableControls (BOOL bEnable)
{
if (!(m_bInited && theMine))
	return;
CDlgHelpers::EnableControls (IDC_PARTICLE_LIFE, IDC_PARTICLE_SELECT_COLOR, GetEffect (null, false) != null);
}

//------------------------------------------------------------------------

void CParticleEffectTool::Add (void)
{
if (!AddEffect ())
	return;
CGameObject *objP = current->Object ();
objP->Type () = OBJ_EFFECT;
objP->Id () = PARTICLE_ID;
objP->m_info.movementType = MT_NONE;
objP->m_info.controlType = CT_NONE;
objP->m_info.renderType = RT_PARTICLE;
memset (&objP->rType.particleInfo, 0, sizeof (objP->rType.particleInfo));
Refresh ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CParticleEffectTool::Copy (void)
{
CGameObject *objP = GetEffect ();
if (!objP)
	ErrorMsg ("No effect object currently selected");
else {
	m_particles = objP->rType.particleInfo;
	m_bValid = 1;
	}
}

//------------------------------------------------------------------------

void CParticleEffectTool::Paste (CGameObject* objP, bool bRefresh)
{
if (Valid () && (objP = GetEffect (objP))) {
	objP->rType.particleInfo = m_particles;
	if (bRefresh)
		Refresh ();
	}
}

//------------------------------------------------------------------------

bool CParticleEffectTool::Refresh (void)
{
UpdateColorCtrl ();
UpdateData (FALSE);
return true;
}

//------------------------------------------------------------------------

void CParticleEffectTool::OnSelectColor ()
{
CGameObject *objP = GetEffect (null, false);
if (objP && CDlgHelpers::SelectColor (BYTE (objP->rType.particleInfo.color [0]), BYTE (objP->rType.particleInfo.color [1]), BYTE (objP->rType.particleInfo.color [2])))
	UpdateColorCtrl ();
}

//------------------------------------------------------------------------

void CParticleEffectTool::OnEdit (void)
{
UpdateData (TRUE);
}

//------------------------------------------------------------------------

bool CParticleEffectTool::OnExtSlider (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
for (int i = 0; i < 11; i++) {
	if (m_data [i].OnScroll (scrollCode, thumbPos, pScrollBar)) {
		UpdateData (TRUE);
		UpdateColorCtrl ();
		return true;
		}
	}
return false;
}

//------------------------------------------------------------------------

void CParticleEffectTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
if (!OnExtSlider (scrollCode, thumbPos, pScrollBar))
	CEffectTabDlg::OnHScroll (scrollCode, thumbPos, pScrollBar);
}

//------------------------------------------------------------------------

void CParticleEffectTool::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
if (!OnExtSlider (scrollCode, thumbPos, pScrollBar))
	CEffectTabDlg::OnVScroll (scrollCode, thumbPos, pScrollBar);
}

//------------------------------------------------------------------------

void CParticleEffectTool::UpdateColorCtrl (void)
{
CGameObject *objP = GetEffect (null, false);
CDlgHelpers::UpdateColorCtrl (&m_colorWnd, objP ? RGB (BYTE (objP->rType.particleInfo.color [0]), BYTE (objP->rType.particleInfo.color [1]), BYTE (objP->rType.particleInfo.color [2])) : RGB (0,0,0));
if (objP) {
	for (int i = 0; i < 4; i++)
		m_data [6 + i].SetValue (objP->rType.particleInfo.color [i]);
	}
}

//------------------------------------------------------------------------

void CParticleEffectTool::OnPaint ()
{
if (theMine) {
	CEffectTabDlg::OnPaint ();
	UpdateColorCtrl ();
	}
}

//------------------------------------------------------------------------
//eof ParticleEffectTool.cpp