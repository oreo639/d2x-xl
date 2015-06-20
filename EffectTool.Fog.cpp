
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
segmentManager.m_fogInfo [0].m_density = 12;
segmentManager.m_fogInfo [1].m_density = 5;
segmentManager.m_fogInfo [0].m_color.r = segmentManager.m_fogInfo [0].m_color.g = segmentManager.m_fogInfo [0].m_color.b = ubyte (255.0f * 0.7f);
segmentManager.m_fogInfo [1].m_color.r = segmentManager.m_fogInfo [1].m_color.g = segmentManager.m_fogInfo [1].m_color.b = ubyte (255.0f * 0.7f);
m_fog [0].transpSlider.Init (this, IDC_LIGHT_FOG_TRANSP_SLIDER, IDC_LIGHT_FOG_TRANSP_SPINNER, -IDC_LIGHT_FOG_TRANSP_TEXT, 0, 19, 1.0, 100.0 / 15.0, 1, "transp: %d%%");
m_fog [1].transpSlider.Init (this, IDC_DENSE_FOG_TRANSP_SLIDER, IDC_DENSE_FOG_TRANSP_SPINNER, -IDC_DENSE_FOG_TRANSP_TEXT, 0, 19, 1.0, 100.0 / 15.0, 1, "transp: %d%%");
CreateColorCtrl (&m_fog [0].colorWnd, IDC_LIGHT_FOG_COLOR);
CreateColorCtrl (&m_fog [1].colorWnd, IDC_DENSE_FOG_COLOR);
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
	m_fog [0].transpSlider.SetValue (segmentManager.m_fogInfo [0].m_density);
	m_fog [1].transpSlider.SetValue (segmentManager.m_fogInfo [1].m_density);
	UpdateColor (0);
	UpdateColor (1);
	}
else {
	segmentManager.m_fogInfo [0].m_density = m_fog [0].transpSlider.GetValue ();
	segmentManager.m_fogInfo [1].m_density = m_fog [1].transpSlider.GetValue ();
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

void CFogTool::OnPickLightFogColor () { PickColor (0); }
void CFogTool::OnPickDenseFogColor () { PickColor (1); }

//------------------------------------------------------------------------------

void CFogTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
if (m_fog [0].transpSlider.OnScroll (scrollCode, thumbPos, pScrollBar))
	UpdateTransparency (0, m_fog [0].transpSlider.GetValue ());
if (m_fog [1].transpSlider.OnScroll (scrollCode, thumbPos, pScrollBar))
	UpdateTransparency (1, m_fog [1].transpSlider.GetValue ());
else
	pScrollBar->SetScrollPos (thumbPos, TRUE);
}

//------------------------------------------------------------------------
//eof WayPointTool.cpp