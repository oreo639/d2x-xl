
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

BEGIN_MESSAGE_MAP (CEffectTool, CToolDlg)
	ON_BN_CLICKED (IDC_EFFECT_ADD, OnAdd)
	ON_BN_CLICKED (IDC_EFFECT_DELETE, OnDelete)
	ON_BN_CLICKED (IDC_EFFECT_COPY, OnCopy)
	ON_BN_CLICKED (IDC_EFFECT_PASTE, OnPaste)
	ON_BN_CLICKED (IDC_EFFECT_PASTE_ALL, OnPasteAll)
	ON_CBN_SELCHANGE (IDC_EFFECT_OBJECTS, OnSetObject)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------
// DIALOG - CEffectTool (constructor)
//------------------------------------------------------------------------

CEffectTool::CEffectTool (CPropertySheet *pParent)
	: CToolDlg (IDD_EFFECTDATA_HORZ + nLayout, pParent)
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
}

//------------------------------------------------------------------------------

void CEffectTool::LoadEffectList (void) 
{
CHECKMINE;

	CComboBox	*cbEffects = CBEffects ();
	char			szEffect [100];
	int			index, curSel = 0;

cbEffects->ResetContent ();
CGameObject *curObj = current->Object (),
				*objP = objectManager.Object (0);

for (int i = 0; i < objectManager.Count (); i++, objP++) {
	if (objP->Type () != OBJ_EFFECT)
		continue;
	if (objP == curObj)
		curSel = i;
	if (objP->Id () == PARTICLE_ID)
		sprintf_s (szEffect, sizeof (szEffect), "Particles (%d)", i);
	else if (objP->Id () == LIGHTNING_ID)
		sprintf_s (szEffect, sizeof (szEffect), "Lightning %d (%d)", objP->rType.lightningInfo.nId, i);
	else if (objP->Id () == SOUND_ID)
		sprintf_s (szEffect, sizeof (szEffect), "Sound (%d)", i);
	else if (objP->Id () == WAYPOINT_ID)
		sprintf_s (szEffect, sizeof (szEffect), "Waypoint (%d)", i);
	else
		continue;
	index = cbEffects->AddString (szEffect);
	cbEffects->SetItemData (index, i);
	}
SelectItemData (cbEffects, curSel);
}

//------------------------------------------------------------------------------

BOOL CEffectTool::OnInitDialog ()
{
if (!CToolDlg::OnInitDialog ())
	return FALSE;
m_effectTools.Init (this);
if (nLayout) {
	m_effectTools.Add (new CParticleEffectTool (IDD_EFFECT_PARTICLE_VERT, this), IDD_EFFECT_PARTICLE_VERT, "Particles");
	m_effectTools.Add (new CLightningEffectTool (IDD_EFFECT_LIGHTNING_VERT, this), IDD_EFFECT_LIGHTNING_VERT, "Lightning");
	m_effectTools.Add (new CSoundEffectTool (IDD_EFFECT_SOUND_VERT, this), IDD_EFFECT_SOUND_VERT, "Sounds");
	m_effectTools.Add (new CWayPointTool (IDD_EFFECT_WAYPOINT_VERT, this), IDD_EFFECT_WAYPOINT_VERT, "Way points");
	}
else {
	m_effectTools.Add (new CParticleEffectTool (IDD_EFFECT_PARTICLE_HORZ, this), IDD_EFFECT_PARTICLE_HORZ, "Particles");
	m_effectTools.Add (new CLightningEffectTool (IDD_EFFECT_LIGHTNING_HORZ, this), IDD_EFFECT_LIGHTNING_HORZ, "Lightning");
	m_effectTools.Add (new CSoundEffectTool (IDD_EFFECT_SOUND_HORZ, this), IDD_EFFECT_SOUND_HORZ, "Sounds");
	m_effectTools.Add (new CWayPointTool (IDD_EFFECT_WAYPOINT_HORZ, this), IDD_EFFECT_WAYPOINT_HORZ, "Way points");
	}
m_effectTools.Setup ();
m_bInited = true;
return TRUE;
}

								/*--------------------------*/

void CEffectTool::DoDataExchange (CDataExchange *pDX)
{
DDX_Control (pDX, IDC_EFFECT_TAB, m_effectTools);
if (!HaveData (pDX)) 
	return;
CGameObject *objP = current->Object ();
if (objP->Type () != OBJ_EFFECT)
	return;
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

//------------------------------------------------------------------------
// CEffectTool - RefreshData
//------------------------------------------------------------------------

void CEffectTool::Refresh ()
{
if (!(m_bInited && theMine))
	return;

LoadEffectList ();
if (current->Object ()->Type () == OBJ_EFFECT)
	m_effectTools.Update (current->Object ()->Id ());
Current ()->Refresh ();
UpdateData (FALSE);
}

//------------------------------------------------------------------------
// CEffectTool - Add Effect
//------------------------------------------------------------------------

void CEffectTool::OnAdd ()
{
Current ()->Add ();
}

//------------------------------------------------------------------------

void CEffectTool::OnDelete ()
{
Current ()->Delete ();
Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnCopy ()
{
if (Current ()->GetEffect ())
	Current ()->Copy ();
else
	ErrorMsg ("No effect object currently selected");
}

//------------------------------------------------------------------------

void CEffectTool::OnPaste ()
{
if (!Current ()->GetEffect ())
	ErrorMsg ("No effect object currently selected");
else if (!Current ()->Valid ())
	ErrorMsg ("No effect data of that type currently available (copy data first)");
	Current ()->Paste ();
}

//------------------------------------------------------------------------

void CEffectTool::OnPasteAll ()
{
if (!Current ()->Valid ())
	ErrorMsg ("No effect data of that type currently available (copy data first)");
else {
	CGameObject *objP = objectManager.Object (0);
	for (int i = objectManager.Count (); i; i--, objP++)
		Current ()->Paste (objP, false);
	}
Refresh ();
}

//------------------------------------------------------------------------

void CEffectTool::OnSetObject ()
{
short nOld = current->ObjectId ();
short nNew = short (CBEffects ()->GetItemData (CBEffects ()->GetCurSel ()));
if (nOld != nNew) {
	UpdateData (TRUE);
	DLE.MineView ()->RefreshObject (nOld, nNew);
	if (current->Object ()->Type () == OBJ_EFFECT)
		m_effectTools.Update (current->Object ()->Id ());
	}
//Refresh ();
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CEffectTabDlg::AddEffect (void)
{
if (objectManager.Count () >= MAX_OBJECTS) {
	ErrorMsg ("Maximum numbers of objects reached");
	return false;
	}
UpdateData (TRUE);
objectManager.Create (OBJ_EFFECT);
return true;
}

//------------------------------------------------------------------------

void CEffectTabDlg::DeleteEffect (void)
{
if (current->ObjectId () == objectManager.Count ()) {
	ErrorMsg ("Cannot delete the secret return.");
	return;
	}
if (objectManager.Count () == 1) {
	ErrorMsg ("Cannot delete the last object");
	return;
	}
if (current->Object ()->m_info.type != OBJ_EFFECT) {
	ErrorMsg ("No effect object currently selected");
	return;
	}
if (QueryMsg ("Are you sure you want to delete this object?") == IDYES) {
	objectManager.Delete ();
	Refresh ();
	DLE.MineView ()->Refresh (false);
	}
}

//------------------------------------------------------------------------

CGameObject* CEffectTabDlg::GetEffect (CGameObject* objP, ubyte nId, char* szType)
{
if (!objP)
	objP = current->Object ();
if ((objP->Type () == OBJ_EFFECT) && (objP->Id () == nId)) 
	return objP;
if (szType) {
	char szMsg [100];
	sprintf_s (szMsg, sizeof (szMsg), "no %s object currently selected", szType);
	ErrorMsg (szMsg);
	}
return null;
}

//------------------------------------------------------------------------

bool CEffectTabDlg::Refresh (void)
{
if (!(m_bInited && theMine))
	return false;
EnableControls (true);
UpdateData (FALSE);
return true;
}

//------------------------------------------------------------------------

BOOL CEffectTabDlg::OnSetActive ()
{
EnableWindow (TRUE);
Refresh ();
return TRUE; 
}

//------------------------------------------------------------------------

BOOL CEffectTabDlg::OnKillActive ()
{
Refresh ();
return TRUE;
}

//------------------------------------------------------------------------

#if 0
BOOL CEffectTabDlg::PreTranslateMessage (MSG* pMsg)
{
if (pMsg->message == WM_KEYDOWN) {
	if (pMsg->wParam == VK_RETURN) {
		GetParent ()->PostMessage (WM_KEYDOWN, VK_RETURN, 0);
		return TRUE;
		}
	if (pMsg->wParam == VK_ESCAPE) {
		GetParent ()->PostMessage (WM_KEYDOWN, VK_ESCAPE, 0);
		return TRUE;
		}
	}
return CDialog::PreTranslateMessage (pMsg);
} 
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//eof effectdlg.cpp