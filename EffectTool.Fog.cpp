
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

//------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CFogTool, CEffectTabDlg)
	ON_WM_HSCROLL ()
	ON_BN_CLICKED (IDC_LIGHT_FOG_PICK_COLOR, OnPickLightFogColor)
	ON_BN_CLICKED (IDC_DENSE_FOG_PICK_COLOR, OnPickDenseFogColor)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------

BOOL CFogTool::OnInitDialog ()
{
if (!CDialog::OnInitDialog ())
	return FALSE;
for (int i = 0; i < NUM_FOG_TYPES; i++) {
	m_fog [0].transpSlider.Init (this, IDC_WATER_FOG_TRANSP_SLIDER + i, IDC_WATER_FOG_TRANSP_SPINNER + i, -(IDC_WATER_FOG_TRANSP_TEXT + i), 0, 19, 1.0, 100.0 / 15.0, 1, "transp: %d%%");
	CreateColorCtrl (&m_fog [i].colorWnd, IDC_WATER_FOG_COLOR + i);
	}
m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------

BOOL CFogTool::OnSetActive ()
{
EnableControls (FALSE);
UpdateData (FALSE);
return TRUE; 
}

//------------------------------------------------------------------------

BOOL CFogTool::OnKillActive ()
{
EnableControls (TRUE);
return TRUE;
}

//------------------------------------------------------------------------

void CFogTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;
if (!pDX->m_bSaveAndValidate) {
	for (int i = 0; i < NUM_FOG_TYPES; i++) {
		m_fog [i].transpSlider.SetValue (segmentManager.m_fogInfo [i].m_density);
		UpdateColor (i);
		}
	}
else {
	for (int i = 0; i < NUM_FOG_TYPES; i++)
		segmentManager.m_fogInfo [i].m_density = m_fog [i].transpSlider.GetValue ();
	}	
}

//------------------------------------------------------------------------

void CFogTool::EnableControls (BOOL bEnable)
{
if (!(m_bInited && theMine))
	return;
CDlgHelpers::EnableControls (IDC_EFFECT_COPY, IDC_EFFECT_ENABLED, false);
}

//------------------------------------------------------------------------------

void CFogTool::UpdateTransparency (int nFogType, int nValue)
{
segmentManager.m_fogInfo [nFogType].m_density = nValue;
}

//------------------------------------------------------------------------------
		
void CFogTool::UpdateColor (int nFogType)
{
UpdateColorCtrl (&m_fog [nFogType].colorWnd, RGB (segmentManager.m_fogInfo [nFogType].m_color.r, segmentManager.m_fogInfo [nFogType].m_color.g, segmentManager.m_fogInfo [nFogType].m_color.b));
}

//------------------------------------------------------------------------------
		
void CFogTool::PickColor (int nFogType)
{
if (CDlgHelpers::SelectColor (segmentManager.m_fogInfo [nFogType].m_color.r, segmentManager.m_fogInfo [nFogType].m_color.g, segmentManager.m_fogInfo [nFogType].m_color.b)) 
	UpdateColor (nFogType);
}

void CFogTool::OnPickWaterFogColor () { PickColor (0); }
void CFogTool::OnPickLavaFogColor  () { PickColor (1); }
void CFogTool::OnPickLightFogColor () { PickColor (2); }
void CFogTool::OnPickDenseFogColor () { PickColor (3); }

//------------------------------------------------------------------------------

void CFogTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
for (int i = 0; i < 3; i++) {
	if (m_fog [i].transpSlider.OnScroll (scrollCode, thumbPos, pScrollBar)) {
		UpdateTransparency (i, m_fog [i].transpSlider.GetValue ());
		return;
		}
	}
pScrollBar->SetScrollPos (thumbPos, TRUE);
}

//------------------------------------------------------------------------
//eof WayPointTool.cpp