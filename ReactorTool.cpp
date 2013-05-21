
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

BEGIN_MESSAGE_MAP (CReactorTool, CToolDlg)
	ON_BN_CLICKED (IDC_REACTOR_ADDTGT, OnAddTarget)
	ON_BN_CLICKED (IDC_REACTOR_DELTGT, OnDeleteTarget)
	ON_BN_CLICKED (IDC_REACTOR_ADDWALLTGT, OnAddWallTarget)
	ON_BN_CLICKED (IDC_REACTOR_DELWALLTGT, OnDeleteWallTarget)
	ON_EN_KILLFOCUS (IDC_REACTOR_COUNTDOWN, OnCountDown)
	ON_EN_KILLFOCUS (IDC_REACTOR_SECRETRETURN, OnSecretReturn)
	ON_EN_UPDATE (IDC_REACTOR_COUNTDOWN, OnCountDown)
	ON_EN_UPDATE (IDC_REACTOR_SECRETRETURN, OnSecretReturn)
	ON_LBN_SELCHANGE (IDC_REACTOR_TARGETLIST, OnSetTarget)
	ON_LBN_DBLCLK (IDC_REACTOR_TARGETLIST, OnSetTarget)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------
// DIALOG - CReactorTool (constructor)
//------------------------------------------------------------------------

CReactorTool::CReactorTool (CPropertySheet *pParent)
	: CToolDlg (IDD_REACTORDATA_HORZ + nLayout, pParent)
{
m_triggerP = null;
Reset ();
}

								/*--------------------------*/

void CReactorTool::Reset ()
{
m_nCountDown = 30;
m_nSecretReturn = 0;
m_nTrigger = 0;
m_targets = 0;
m_iTarget = -1;
*m_szTarget = '\0';
}

//------------------------------------------------------------------------------

BOOL CReactorTool::OnInitDialog ()
{
if (!CToolDlg::OnInitDialog ())
	return FALSE;
// Descent only uses the first control center, #0
m_bInited = true;
return TRUE;
}

								/*--------------------------*/

void CReactorTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;
DDX_Text (pDX, IDC_REACTOR_COUNTDOWN, m_nCountDown);
DDX_Text (pDX, IDC_REACTOR_SECRETRETURN, m_nSecretReturn);
DDX_Text (pDX, IDC_REACTOR_TARGET, m_szTarget, sizeof (m_szTarget));
}

								/*--------------------------*/

BOOL CReactorTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

								/*--------------------------*/

void CReactorTool::EnableControls (BOOL bEnable)
{
CToolDlg::EnableControls (IDC_REACTOR_COUNTDOWN, IDC_REACTOR_SECRETRETURN, bEnable);
//int i;
//for (i = IDC_TRIGGER_TRIGGER_NO; i <= IDC_TRIGGER_PASTE; i++)
//	GetDlgItem (i)->EnableWindow (bEnable);
}

								/*--------------------------*/

void CReactorTool::InitLBTargets ()
{
CListBox *plb = LBTargets ();
m_iTarget = plb->GetCurSel ();
plb->ResetContent ();
if (m_triggerP) {
	m_targets = m_triggerP->Count ();
	int i;
	for (i = 0; i < m_targets ; i++) {
		sprintf_s (m_szTarget, sizeof (m_szTarget), "   %d, %d", m_triggerP->Segment (i), m_triggerP->Side (i) + 1);
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

//------------------------------------------------------------------------
// CReactorTool - RefreshData
//------------------------------------------------------------------------

void CReactorTool::Refresh ()
{
if (!(m_bInited && theMine))
	return;
EnableControls (DLE.IsD2File ());
m_triggerP = triggerManager.ReactorTrigger (m_nTrigger);
m_nCountDown = triggerManager.ReactorTime ();
m_nSecretReturn = objectManager.SecretSegment ();
InitLBTargets ();
OnSetTarget ();
UpdateData (FALSE);
}

//------------------------------------------------------------------------
// CReactorTool - TrigValueMsg
//------------------------------------------------------------------------

void CReactorTool::OnCountDown () 
{
char szVal [5];
::GetWindowText (GetDlgItem (IDC_REACTOR_COUNTDOWN)->m_hWnd, szVal, sizeof (szVal));
if (!*szVal)
	return;
UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udTriggers);
triggerManager.ReactorTime () = m_nCountDown;
undoManager.End (__FUNCTION__);
}

//------------------------------------------------------------------------
// CReactorTool - TrigTimeMsg
//------------------------------------------------------------------------

void CReactorTool::OnSecretReturn () 
{
char szVal [5];
::GetWindowText (GetDlgItem (IDC_REACTOR_SECRETRETURN)->m_hWnd, szVal, sizeof (szVal));
if (!*szVal)
	return;
UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udObjects);
objectManager.SecretSegment () = m_nSecretReturn;
undoManager.End (__FUNCTION__);
}

//------------------------------------------------------------------------
// CReactorTool - Add segment/side to trigger list
//------------------------------------------------------------------------

void CReactorTool::AddTarget (short nSegment, short nSide) 
{
m_targets = m_triggerP->Count ();
if (m_targets >= MAX_TRIGGER_TARGETS) {
	DEBUGMSG (" Reactor tool: No more targets possible for this trigger.");
	return;
	}
if (FindTarget (nSegment, nSide) >= 0) {
	DEBUGMSG (" Reactor tool: Trigger already has this target.");
	return;
	}
undoManager.Begin (__FUNCTION__, udTriggers);
m_triggerP->Add (nSegment, nSide - 1);
undoManager.End (__FUNCTION__);
sprintf_s (m_szTarget, sizeof (m_szTarget), "   %d,%d", nSegment, nSide);
LBTargets ()->AddString (m_szTarget);
LBTargets ()->SetCurSel (m_targets++);
*m_szTarget = '\0';
Refresh ();
}


//------------------------------------------------------------------------------

void CReactorTool::OnAddTarget () 
{
int nSegment, nSide;
UpdateData (TRUE);
sscanf_s (m_szTarget, "%d,%d", &nSegment, &nSide);
if ((nSegment < 0) || (nSegment >= segmentManager.Count ()) || (nSide < 1) || (nSide > 6))
	return;
AddTarget (nSegment, nSide);
}

//------------------------------------------------------------------------------

void CReactorTool::OnAddWallTarget ()
{
other = &selections [!current->Index ()];
int i = FindTarget (other->m_nSegment, other->m_nSide);
if (i >= 0)
	return;
LBTargets ()->SetCurSel (i);
AddTarget (other->m_nSegment, other->m_nSide + 1);
}

//------------------------------------------------------------------------
// CReactorTool - Delete segment/side
//------------------------------------------------------------------------

void CReactorTool::OnDeleteTarget ()
{
m_iTarget = LBTargets ()->GetCurSel ();
if ((m_iTarget < 0) || (m_iTarget >= MAX_TRIGGER_TARGETS))
	return;
undoManager.Begin (__FUNCTION__, udTriggers);
m_targets = m_triggerP->Delete (m_iTarget);
undoManager.End (__FUNCTION__);
LBTargets ()->DeleteString (m_iTarget);
if (m_iTarget >= LBTargets ()->GetCount ())
	m_iTarget--;
LBTargets ()->SetCurSel (m_iTarget);
Refresh ();
}

//------------------------------------------------------------------------------

int CReactorTool::FindTarget (short nSegment, short nSide)
{
return m_triggerP->Find (nSegment, nSide);
}

//------------------------------------------------------------------------------

void CReactorTool::OnDeleteWallTarget ()
{
other = &selections [!current->Index ()];
int i = FindTarget (other->m_nSegment, other->m_nSide);
if (i < 0) {
	DEBUGMSG (" Reactor tool: Trigger doesn't target other segment's current side.");
	return;
	}
LBTargets ()->SetCurSel (i);
OnDeleteTarget ();
}

//------------------------------------------------------------------------
// CReactorTool - Cube/Side list box message
//
// sets "other segment" to selected item
//------------------------------------------------------------------------

void CReactorTool::OnSetTarget () 
{
// get affected segment/side list box index
m_iTarget = LBTargets ()->GetCurSel ();
// if selected and within range, then set "other" segment/side
if ((m_iTarget < 0) || (m_iTarget >= MAX_TRIGGER_TARGETS) || (m_iTarget >= m_triggerP->Count ()))
	return;

short nSegment = m_triggerP->Segment (m_iTarget);
if ((nSegment < 0) || (nSegment >= segmentManager.Count ()))
	 return;
short nSide = m_triggerP->Side (m_iTarget);
if ((nSide < 0) || (nSide > 5))
	return;
if ((current->SegmentId () == nSegment) && (current->SideId () == nSide))
	return;
*((CSideKey *) other) = (*m_triggerP) [m_iTarget];
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

//eof reactordlg.cpp