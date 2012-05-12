
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

BEGIN_MESSAGE_MAP (CSoundEffectTool, CEffectTabDlg)
	ON_WM_HSCROLL ()
	ON_BN_CLICKED (IDC_EFFECT_ENABLED, OnEdit)
	ON_EN_KILLFOCUS (IDC_SOUND_FILE, OnEdit)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------

BOOL CSoundEffectTool::OnInitDialog ()
{
if (!CEffectTabDlg::OnInitDialog ())
	return FALSE;
m_soundVolume.Init (this, IDC_SOUND_VOLUME_SLIDER, IDC_SOUND_VOLUME_SPINNER, -IDT_SOUND_VOLUME, 0, 10, 1.0, 10.0, 1, "%d%%");
m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------

void CSoundEffectTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;
CGameObject *objP = GetEffect (null, false);
EnableControls (objP != null);
if (objP) {
	objP->rType.soundInfo.bEnabled = DDX_Flag (pDX, IDC_EFFECT_ENABLED, objP->rType.soundInfo.bEnabled);
	DDX_Text (pDX, IDC_SOUND_FILE, objP->rType.soundInfo.szFilename, sizeof (objP->rType.soundInfo.szFilename));
	m_soundVolume.DoDataExchange (pDX, objP->rType.soundInfo.nVolume);
	}
}

//------------------------------------------------------------------------

void CSoundEffectTool::EnableControls (BOOL bEnable)
{
if (!(m_bInited && theMine))
	return;
CDlgHelpers::EnableControls (IDC_SOUND_FILE, IDC_SOUND_VOLUME_SLIDER, GetEffect (null, false) != null);
}

//------------------------------------------------------------------------

void CSoundEffectTool::Add (void)
{
if (!AddEffect ())
	return;
CGameObject *objP = current->Object ();
objP->Type () = OBJ_EFFECT;
objP->Id () = SOUND_ID;
objP->m_info.movementType = MT_NONE;
objP->m_info.controlType = CT_NONE;
objP->m_info.renderType = RT_SOUND;
*objP->rType.soundInfo.szFilename = '\0';
objP->rType.soundInfo.nVolume = 10;
Refresh ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CSoundEffectTool::Copy (void)
{
CGameObject *objP = GetEffect ();
if (!objP) 
	ErrorMsg ("No effect object currently selected");
else 
	m_sound = objP->rType.soundInfo;
}

//------------------------------------------------------------------------

void CSoundEffectTool::Paste (CGameObject* objP, bool bRefresh)
{
if (Valid () && (objP = GetEffect (objP))) {
	objP->rType.soundInfo = m_sound;
	if (bRefresh)
		Refresh ();
	}
}

//------------------------------------------------------------------------

void CSoundEffectTool::OnEdit (void)
{
UpdateData (TRUE);
}

//------------------------------------------------------------------------

void CSoundEffectTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
if (m_soundVolume.OnScroll (scrollCode, thumbPos, pScrollBar))
	UpdateData (TRUE);
else
	CEffectTabDlg::OnHScroll (scrollCode, thumbPos, pScrollBar);
}

//------------------------------------------------------------------------
//eof SoundEffectTool.cpp