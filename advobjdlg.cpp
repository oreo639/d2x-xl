// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include "afxpriv.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "mine.h"
#include "dle-xp.h"
#include "global.h"
#include "toolview.h"
#include "textures.h"

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CAdvObjTool, CToolDlg)
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

                        /*--------------------------*/

CAdvObjTool::CAdvObjTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_ADVOBJDATA2 : IDD_ADVOBJDATA, pParent)
{
}

                        /*--------------------------*/

BOOL CAdvObjTool::OnInitDialog ()
{
CToolDlg::OnInitDialog ();
m_bInited = true;
return TRUE;
}

                        /*--------------------------*/

void CAdvObjTool::DoDataExchange (CDataExchange * pDX)
{
DDX_Text (pDX, IDC_ADVOBJ_MASS, m_mass);
DDX_Text (pDX, IDC_ADVOBJ_DRAG, m_drag);
DDX_Text (pDX, IDC_ADVOBJ_BRAKES, m_brakes);
DDX_Text (pDX, IDC_ADVOBJ_TURNROLL, m_turnRoll);
DDX_Text (pDX, IDC_ADVOBJ_FLAGS, m_flags);
DDX_Text (pDX, IDC_ADVOBJ_SIZE, m_size);
DDX_Text (pDX, IDC_ADVOBJ_SHIELD, m_shields);
DDX_Text (pDX, IDC_ADVOBJ_VX, m_vx);
DDX_Text (pDX, IDC_ADVOBJ_VY, m_vy);
DDX_Text (pDX, IDC_ADVOBJ_VZ, m_vz);
DDX_Text (pDX, IDC_ADVOBJ_TX, m_tx);
DDX_Text (pDX, IDC_ADVOBJ_TY, m_ty);
DDX_Text (pDX, IDC_ADVOBJ_TZ, m_tz);
DDX_Text (pDX, IDC_ADVOBJ_RVX, m_rvx);
DDX_Text (pDX, IDC_ADVOBJ_RVY, m_rvy);
DDX_Text (pDX, IDC_ADVOBJ_RVZ, m_rvz);
DDX_Text (pDX, IDC_ADVOBJ_RTX, m_rtx);
DDX_Text (pDX, IDC_ADVOBJ_RTY, m_rty);
DDX_Text (pDX, IDC_ADVOBJ_RTZ, m_rtz);
DDX_Text (pDX, IDC_ADVOBJ_MODEL, m_model);
DDX_Text (pDX, IDC_ADVOBJ_FRAME, m_frame);
DDX_Text (pDX, IDC_ADVOBJ_FRAMENO, m_frameNo);
}
								
								/*--------------------------*/

void CAdvObjTool::OnAccept (void)
{
if (!(m_bInited && theMine))
	return;
UpdateData (TRUE);
CGameObject *pObj = theMine->CurrObj ();
pObj->mType.physInfo.mass = m_mass;
pObj->mType.physInfo.drag = m_drag;
pObj->mType.physInfo.brakes = m_brakes;
pObj->mType.physInfo.turnroll = m_turnRoll;
pObj->mType.physInfo.flags = m_flags;
pObj->mType.physInfo.velocity.x = m_vx;
pObj->mType.physInfo.velocity.y = m_vy;
pObj->mType.physInfo.velocity.z = m_vz;
pObj->mType.physInfo.thrust.x = m_tx;
pObj->mType.physInfo.thrust.y = m_ty;
pObj->mType.physInfo.thrust.z = m_tz;
pObj->mType.physInfo.rotvel.x = m_rvx;
pObj->mType.physInfo.rotvel.y = m_rvy;
pObj->mType.physInfo.rotvel.z = m_rvz;
pObj->mType.physInfo.rotthrust.x = m_rtx;
pObj->mType.physInfo.rotthrust.y = m_rty;
pObj->mType.physInfo.rotthrust.z = m_rtz;
}

								/*--------------------------*/

BOOL CAdvObjTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

								/*--------------------------*/

BOOL CAdvObjTool::OnKillActive ()
{
OnAccept ();
return CToolDlg::OnSetActive ();
}

								/*--------------------------*/

void CAdvObjTool::Refresh ()
{
if (!(m_bInited && theMine))
	return;
if (!theMine->GameInfo ().objects.count) {
	EnableControls (IDC_ADVOBJ_SIZE, IDC_ADVOBJ_RTZ, FALSE);
	return;
	}
CGameObject *pObj = theMine->CurrObj ();
EnableControls (IDC_ADVOBJ_SIZE, IDC_ADVOBJ_RTZ, TRUE);
m_size = pObj->size;
m_shields = pObj->shields;
switch (pObj->movement_type) {
	case MT_PHYSICS:	
		m_mass = pObj->mType.physInfo.mass;
		m_drag = pObj->mType.physInfo.drag;
		m_brakes = pObj->mType.physInfo.brakes;
		m_turnRoll = pObj->mType.physInfo.turnroll;
		m_flags = pObj->mType.physInfo.flags;
		m_vx = pObj->mType.physInfo.velocity.x;
		m_vy = pObj->mType.physInfo.velocity.y;
		m_vz = pObj->mType.physInfo.velocity.z;
		m_tx = pObj->mType.physInfo.thrust.x;
		m_ty = pObj->mType.physInfo.thrust.y;
		m_tz = pObj->mType.physInfo.thrust.z;
		m_rvx = pObj->mType.physInfo.rotvel.x;
		m_rvy = pObj->mType.physInfo.rotvel.y;
		m_rvz = pObj->mType.physInfo.rotvel.z;
		m_rtx = pObj->mType.physInfo.rotthrust.x;
		m_rty = pObj->mType.physInfo.rotthrust.y;
		m_rtz = pObj->mType.physInfo.rotthrust.z;
		break;
	case MT_SPINNING:
	case MT_NONE:
	default:
		EnableControls (IDC_ADVOBJ_MASS, IDC_ADVOBJ_RTZ, FALSE);
		m_mass = 0;
		m_drag = 0;
		m_brakes = 0;
		m_turnRoll = 0;
		m_flags = 0;
		m_vx = 0;
		m_vy = 0;
		m_vz = 0;
		m_tx = 0;
		m_ty = 0;
		m_tz = 0;
		m_rvx = 0;
		m_rvy = 0;
		m_rvz = 0;
		m_rtx = 0;
		m_rty = 0;
		m_rtz = 0;
		break;
	}

switch (pObj->render_type) {
	case RT_MORPH:
	case RT_POLYOBJ:
		m_model = pObj->rType.polyModelInfo.model_num;
		m_frame = 0;
		m_frameNo = 0;
		EnableControls (IDC_ADVOBJ_FRAME, IDC_ADVOBJ_FRAMENO, FALSE);
		break;
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
		m_model = pObj->rType.vClipInfo.vclip_num;
		m_frame = pObj->rType.vClipInfo.frametime;
		m_frameNo = pObj->rType.vClipInfo.framenum;
		break;
	case RT_LASER:
	case RT_NONE:
	default:
		EnableControls (IDC_ADVOBJ_MODEL, IDC_ADVOBJ_FRAMENO, FALSE);
		m_model = 0;
		m_frame = 0;
		m_frameNo = 0;
		break;
	}
UpdateData (FALSE);
}

                        /*--------------------------*/

//eof advObjTool.cpp