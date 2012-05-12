
#include "stdafx.h"
#include "afxpriv.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>

#include "mine.h"
#include "dle-xp.h"
#include "toolview.h"
#include "textures.h"

//------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CAdvancedObjTool, CObjectTabDlg)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_SIZE, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_SHIELD, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_MASS, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_DRAG, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_BRAKES, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_TURNROLL, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_FLAGS, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_VX , OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_TX , OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_RVX, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_VY , OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_TY , OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_RVY, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_VZ , OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_TZ , OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_RVZ, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_RTX, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_RTY, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_RTZ, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_MODEL, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_FRAME, OnAccept)
	ON_EN_KILLFOCUS (IDC_ADVOBJ_FRAMENO, OnAccept)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------------

BOOL CAdvancedObjTool::OnInitDialog ()
{
if (!CObjectTabDlg::OnInitDialog ())
	return FALSE;
m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------------

void CAdvancedObjTool::DoDataExchange (CDataExchange * pDX)
{
if (!HaveData (pDX)) 
	return;
DDX_Text (pDX, IDC_ADVOBJ_MASS, m_mass);
DDX_Text (pDX, IDC_ADVOBJ_DRAG, m_drag);
DDX_Text (pDX, IDC_ADVOBJ_BRAKES, m_brakes);
DDX_Text (pDX, IDC_ADVOBJ_TURNROLL, m_turnRoll);
DDX_Text (pDX, IDC_ADVOBJ_FLAGS, m_flags);
DDX_Text (pDX, IDC_ADVOBJ_SIZE, m_size);
DDX_Text (pDX, IDC_ADVOBJ_SHIELD, m_shields);
DDX_Text (pDX, IDC_ADVOBJ_VX, m_velocity.x);
DDX_Text (pDX, IDC_ADVOBJ_VY, m_velocity.y);
DDX_Text (pDX, IDC_ADVOBJ_VZ, m_velocity.z);
DDX_Text (pDX, IDC_ADVOBJ_TX, m_thrust.x);
DDX_Text (pDX, IDC_ADVOBJ_TY, m_thrust.y);
DDX_Text (pDX, IDC_ADVOBJ_TZ, m_thrust.z);
DDX_Text (pDX, IDC_ADVOBJ_RVX, m_rotVel.x);
DDX_Text (pDX, IDC_ADVOBJ_RVY, m_rotVel.y);
DDX_Text (pDX, IDC_ADVOBJ_RVZ, m_rotVel.z);
DDX_Text (pDX, IDC_ADVOBJ_RTX, m_rotThrust.x);
DDX_Text (pDX, IDC_ADVOBJ_RTY, m_rotThrust.y);
DDX_Text (pDX, IDC_ADVOBJ_RTZ, m_rotThrust.z);
DDX_Text (pDX, IDC_ADVOBJ_MODEL, m_model);
DDX_Text (pDX, IDC_ADVOBJ_FRAME, m_frame);
DDX_Text (pDX, IDC_ADVOBJ_FRAMENO, m_frameNo);
}
								
								/*--------------------------*/

void CAdvancedObjTool::OnAccept (void)
{
if (!(m_bInited && theMine))
	return;
UpdateData (TRUE);
CGameObject *objP = current->Object ();
objP->mType.physInfo.mass = m_mass;
objP->mType.physInfo.drag = m_drag;
objP->mType.physInfo.brakes = m_brakes;
objP->mType.physInfo.turnRoll = m_turnRoll;
objP->mType.physInfo.flags = m_flags;
objP->mType.physInfo.velocity.x = m_velocity.x;
objP->mType.physInfo.velocity.y = m_velocity.y;
objP->mType.physInfo.velocity.z = m_velocity.z;
objP->mType.physInfo.thrust.x = m_thrust.x;
objP->mType.physInfo.thrust.y = m_thrust.y;
objP->mType.physInfo.thrust.z = m_thrust.z;
objP->mType.physInfo.rotVel.x = m_rotVel.x;
objP->mType.physInfo.rotVel.y = m_rotVel.y;
objP->mType.physInfo.rotVel.z = m_rotVel.z;
objP->mType.physInfo.rotThrust.x = m_rotThrust.x;
objP->mType.physInfo.rotThrust.y = m_rotThrust.y;
objP->mType.physInfo.rotThrust.z = m_rotThrust.z;
}

								/*--------------------------*/

BOOL CAdvancedObjTool::OnSetActive ()
{
Refresh ();
return CObjectTabDlg::OnSetActive ();
}

								/*--------------------------*/

BOOL CAdvancedObjTool::OnKillActive ()
{
OnAccept ();
return CObjectTabDlg::OnSetActive ();
}

								/*--------------------------*/

bool CAdvancedObjTool::Refresh (void)
{
if (!(m_bInited && theMine))
	return false;
if (!objectManager.Count ()) {
	CDlgHelpers::EnableControls (IDC_ADVOBJ_SIZE, IDC_ADVOBJ_RTZ, FALSE);
	return false;
	}
CDlgHelpers::EnableControls (IDC_ADVOBJ_SIZE, IDC_ADVOBJ_FRAMENO, TRUE);

CGameObject *objP = current->Object ();
m_size = objP->m_info.size;
m_shields = objP->m_info.shields;
switch (objP->m_info.movementType) {
	case MT_PHYSICS:	
		m_mass = objP->mType.physInfo.mass;
		m_drag = objP->mType.physInfo.drag;
		m_brakes = objP->mType.physInfo.brakes;
		m_turnRoll = objP->mType.physInfo.turnRoll;
		m_flags = objP->mType.physInfo.flags;
		m_velocity.x = objP->mType.physInfo.velocity.x;
		m_velocity.y = objP->mType.physInfo.velocity.y;
		m_velocity.z = objP->mType.physInfo.velocity.z;
		m_thrust.x = objP->mType.physInfo.thrust.x;
		m_thrust.y = objP->mType.physInfo.thrust.y;
		m_thrust.z = objP->mType.physInfo.thrust.z;
		m_rotVel.x = objP->mType.physInfo.rotVel.x;
		m_rotVel.y = objP->mType.physInfo.rotVel.y;
		m_rotVel.z = objP->mType.physInfo.rotVel.z;
		m_rotThrust.x = objP->mType.physInfo.rotThrust.x;
		m_rotThrust.y = objP->mType.physInfo.rotThrust.y;
		m_rotThrust.z = objP->mType.physInfo.rotThrust.z;
		break;

	case MT_SPINNING:
	case MT_NONE:
	default:
		CDlgHelpers::EnableControls (IDC_ADVOBJ_MASS, IDC_ADVOBJ_RTZ, FALSE);
		m_mass = 0;
		m_drag = 0;
		m_brakes = 0;
		m_turnRoll = 0;
		m_flags = 0;
		m_velocity.x = 0;
		m_velocity.y = 0;
		m_velocity.z = 0;
		m_thrust.x = 0;
		m_thrust.y = 0;
		m_thrust.z = 0;
		m_rotVel.x = 0;
		m_rotVel.y = 0;
		m_rotVel.z = 0;
		m_rotThrust.x = 0;
		m_rotThrust.y = 0;
		m_rotThrust.z = 0;
		break;
	}

switch (objP->m_info.renderType) {
	case RT_MORPH:
	case RT_POLYOBJ:
		m_model = objP->rType.polyModelInfo.nModel;
		m_frame = 0;
		m_frameNo = 0;
		CDlgHelpers::EnableControls (IDC_ADVOBJ_FRAME, IDC_ADVOBJ_FRAMENO, FALSE);
		break;

	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
		m_model = objP->rType.animationInfo.nAnimation;
		m_frame = objP->rType.animationInfo.nFrameTime;
		m_frameNo = objP->rType.animationInfo.nFrame;
		break;

	case RT_LASER:
	case RT_NONE:
	default:
		CDlgHelpers::EnableControls (IDC_ADVOBJ_MODEL, IDC_ADVOBJ_FRAMENO, FALSE);
		m_model = 0;
		m_frame = 0;
		m_frameNo = 0;
		break;
	}
UpdateData (FALSE);
return true;
}

//------------------------------------------------------------------------------

//eof advObjTool.cpp