
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

BEGIN_MESSAGE_MAP (CWayPointTool, CEffectTabDlg)
	ON_WM_HSCROLL ()
	ON_EN_KILLFOCUS (IDC_WAYPOINT_ID, OnEdit)
	ON_EN_KILLFOCUS (IDC_WAYPOINT_SUCC, OnEdit)
	ON_EN_KILLFOCUS (IDC_WAYPOINT_SPEED, OnEdit)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------

BOOL CWayPointTool::OnInitDialog ()
{
if (!CDialog::OnInitDialog ())
	return FALSE;
m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------

void CWayPointTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;
CGameObject *objP = GetEffect (null, false);
EnableControls (objP != null);
if (objP) {
	objP->cType.wayPointInfo.nId = DDX_Int (pDX, IDC_WAYPOINT_ID, objP->cType.wayPointInfo.nId);
	objP->cType.wayPointInfo.nSuccessor = DDX_Int (pDX, IDC_WAYPOINT_SUCC, objP->cType.wayPointInfo.nSuccessor);
	objP->cType.wayPointInfo.nSpeed = DDX_Int (pDX, IDC_WAYPOINT_SPEED, objP->cType.wayPointInfo.nSpeed);
	}
}

//------------------------------------------------------------------------

void CWayPointTool::EnableControls (BOOL bEnable)
{
if (!(m_bInited && theMine))
	return;
CDlgHelpers::EnableControls (IDC_WAYPOINT_ID, IDC_WAYPOINT_SPEED, GetEffect (null, false) != null);
}

//------------------------------------------------------------------------

void CWayPointTool::Add (void)
{
if (!AddEffect ())
	return;
CGameObject *objP = current->Object ();
objP->Type () = OBJ_EFFECT;
objP->Id () = WAYPOINT_ID;
objP->m_info.movementType = MT_NONE;
objP->m_info.controlType = CT_WAYPOINT;
objP->m_info.renderType = RT_NONE;
objP->cType.wayPointInfo.nId = 0;
objP->cType.wayPointInfo.nSuccessor = 0;
objP->cType.wayPointInfo.nSpeed = 20;
Refresh ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CWayPointTool::Copy (void)
{
CGameObject *objP = GetEffect ();
if (objP) {
	m_wayPoint = objP->cType.wayPointInfo;
	m_bValid = 1;
	}
}

//------------------------------------------------------------------------

void CWayPointTool::Paste (CGameObject* objP, bool bRefresh)
{
if (Valid () && (objP = GetEffect (objP))) {
	objP->cType.wayPointInfo = m_wayPoint;
	if (bRefresh)
		Refresh ();
	}
}

//------------------------------------------------------------------------

void CWayPointTool::OnEdit (void)
{
UpdateData (TRUE);
}

//------------------------------------------------------------------------
//eof WayPointTool.cpp