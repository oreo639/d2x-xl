// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>

#include "mine.h"
#include "dle-xp.h"

//------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CSegmentTool, CToolDlg)
	ON_BN_CLICKED (IDC_CUBE_SETCOORD, OnSetCoord)
	ON_BN_CLICKED (IDC_CUBE_RESETCOORD, OnResetCoord)
	ON_BN_CLICKED (IDC_CUBE_ADD, OnAddSegment)
	ON_BN_CLICKED (IDC_CUBE_ADD_MATCEN, OnAddRobotMaker)
	ON_BN_CLICKED (IDC_CUBE_ADD_EQUIPMAKER, OnAddEquipMaker)
	ON_BN_CLICKED (IDC_CUBE_ADD_FUELCEN, OnAddFuelCenter)
	ON_BN_CLICKED (IDC_CUBE_ADD_REPAIRCEN, OnAddRepairCenter)
	ON_BN_CLICKED (IDC_CUBE_ADD_CONTROLCEN, OnAddReactor)
	ON_BN_CLICKED (IDC_CUBE_SPLIT, OnSplitSegment)
	ON_BN_CLICKED (IDC_CUBE_DEL, OnDeleteSegment)
	ON_BN_CLICKED (IDC_CUBE_OTHER, OnOtherSegment)
	ON_BN_CLICKED (IDC_CUBE_ENDOFEXIT, OnEndOfExit)
	ON_BN_CLICKED (IDC_CUBE_WATER, OnProp1)
	ON_BN_CLICKED (IDC_CUBE_LAVA, OnProp2)
	ON_BN_CLICKED (IDC_CUBE_BLOCKED, OnProp3)
	ON_BN_CLICKED (IDC_CUBE_NODAMAGE, OnProp4)
	ON_BN_CLICKED (IDC_CUBE_OUTDOORS, OnProp5)
	ON_BN_CLICKED (IDC_CUBE_SIDE1, OnSide1)
	ON_BN_CLICKED (IDC_CUBE_SIDE2, OnSide2)
	ON_BN_CLICKED (IDC_CUBE_SIDE3, OnSide3)
	ON_BN_CLICKED (IDC_CUBE_SIDE4, OnSide4)
	ON_BN_CLICKED (IDC_CUBE_SIDE5, OnSide5)
	ON_BN_CLICKED (IDC_CUBE_SIDE6, OnSide6)
	ON_BN_CLICKED (IDC_CUBE_POINT1, OnPoint1)
	ON_BN_CLICKED (IDC_CUBE_POINT2, OnPoint2)
	ON_BN_CLICKED (IDC_CUBE_POINT3, OnPoint3)
	ON_BN_CLICKED (IDC_CUBE_POINT4, OnPoint4)
	ON_BN_CLICKED (IDC_CUBE_ADDBOT, OnAddMatCenter)
	ON_BN_CLICKED (IDC_CUBE_DELBOT, OnDeleteMatCenter)
	ON_BN_CLICKED (IDC_CUBE_TRIGGERDETAILS, OnTriggerDetails)
	ON_BN_CLICKED (IDC_CUBE_WALLDETAILS, OnWallDetails)
	ON_CBN_SELCHANGE (IDC_CUBE_CUBENO, OnSetSegment)
	ON_CBN_SELCHANGE (IDC_CUBE_TYPE, OnSetType)
	ON_CBN_SELCHANGE (IDC_CUBE_OWNER, OnSetOwner)
	ON_EN_KILLFOCUS (IDC_CUBE_LIGHT, OnLight)
	ON_EN_KILLFOCUS (IDC_CUBE_SHIELD_DAMAGE, OnDamage0)
	ON_EN_KILLFOCUS (IDC_CUBE_ENERGY_DAMAGE, OnDamage1)
	ON_EN_KILLFOCUS (IDC_CUBE_GROUP, OnSetGroup)
	ON_EN_UPDATE (IDC_CUBE_LIGHT, OnLight)
	ON_LBN_DBLCLK (IDC_CUBE_TRIGGERS, OnTriggerDetails)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------------

CSegmentTool::CSegmentTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_CUBEDATA2 : IDD_CUBEDATA, pParent)
{
Reset ();
}

//------------------------------------------------------------------------------

void CSegmentTool::Reset ()
{
CHECKMINE;

m_nSegment =
m_nSide =
m_nPoint = 0;
m_nType = 0;
m_nProps = 0;
m_nVertex = 0;
m_bEndOfExit = 0;
m_nDamage [0] =
m_nDamage [1] = 0;
m_nLight = 0;
m_nLastSegment =
m_nLastSide = -1;
m_bSetDefTexture = 0;
m_nOwner = segmentManager.Segment (0)->m_info.owner;
m_nGroup = segmentManager.Segment (0)->m_info.group;
memset (m_nCoord, 0, sizeof (m_nCoord));
}

//------------------------------------------------------------------------------

void CSegmentTool::InitCBSegmentNo (void)
{
CHECKMINE;
CComboBox *pcb = CBSegmentNo ();
if (segmentManager.Count () != pcb->GetCount ()) {
	pcb->ResetContent ();
	for (int i = 0; i < segmentManager.Count (); i++) {
		_itoa_s (i, message, sizeof (message), 10);
		pcb->AddString (message);
		}
	}
pcb->SetCurSel (m_nSegment);
}

//------------------------------------------------------------------------------

BOOL CSegmentTool::OnInitDialog ()
{
if (!CToolDlg::OnInitDialog ())
	return FALSE;

	static char* pszSegmentFuncs [] = {
		"None",
		"Fuel Center",
		"Repair Center",
		"Reactor",
		"Robot Maker",
		"Blue Goal",
		"Red Goal",
		"Blue Team",
		"Red Team",
		"Speed Boost",
		"Sky Box",
		"Equip Maker"
		};

CComboBox *pcb = CBType ();
pcb->ResetContent ();

int h, i, j;
for (j = sizeof (pszSegmentFuncs) / sizeof (*pszSegmentFuncs), i = 0; i < j; i++) {
	h = pcb->AddString (pszSegmentFuncs [i]);
	pcb->SetItemData (h, i);
	}
pcb = CBOwner ();
pcb->ResetContent ();

pcb->AddString ("Neutral");
pcb->AddString ("Unowned");
pcb->AddString ("Blue Team");
pcb->AddString ("Red Team");
m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------------

void CSegmentTool::DoDataExchange (CDataExchange *pDX)
{
if (!HaveData (pDX)) 
	return;

DDX_CBIndex (pDX, IDC_CUBE_CUBENO, m_nSegment);
//DDX_CBIndex (pDX, IDC_CUBE_TYPE, m_nType);
SelectItemData (CBType (), m_nType);
DDX_Double (pDX, IDC_CUBE_LIGHT, m_nLight);
DDX_Radio (pDX, IDC_CUBE_SIDE1, m_nSide);
DDX_Radio (pDX, IDC_CUBE_POINT1, m_nPoint);
for (int	i = 0; i < 3; i++) {
	DDX_Double (pDX, IDC_CUBE_POINTX + i, m_nCoord [i]);
	if (m_nCoord [i] < -0x7fff)
		m_nCoord [i] = -0x7fff;
	else if (m_nCoord [i] > 0x7fff)
		m_nCoord [i] = 0x7fff;
//	DDV_MinMaxInt (pDX, (int) m_nCoord [i], -0x7fff, 0x7fff);
	}

int i;

for (i = 0; i < 5; i++) {
	int h = (m_nProps & (1 << i)) != 0;
	DDX_Check (pDX, IDC_CUBE_WATER + i, h);
	if (h)
		m_nProps |= (1 << i);
	else
		m_nProps &= ~(1 << i);
	}

DDX_Check (pDX, IDC_CUBE_ENDOFEXIT, m_bEndOfExit);
DDX_Check (pDX, IDC_CUBE_SETDEFTEXTURE, m_bSetDefTexture);
++m_nOwner;
DDX_CBIndex (pDX, IDC_CUBE_OWNER, m_nOwner);
m_nOwner--;
DDX_Text (pDX, IDC_CUBE_GROUP, m_nGroup);
if (m_nGroup < -1)
	m_nGroup = -1;
else if (m_nGroup > 127)
	m_nGroup = 127;
for (i = 0; i < 2; i++)
	DDX_Text (pDX, IDC_CUBE_SHIELD_DAMAGE + i, m_nDamage [i]);

//DDV_MinMaxInt (pDX, m_nGroup, -1, 127);
}


//------------------------------------------------------------------------------

BOOL CSegmentTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

//------------------------------------------------------------------------------

bool CSegmentTool::IsRobotMaker (CSegment *segP)
{
return 
	(segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) &&
	(segP->m_info.nMatCen >= 0) &&
	(segP->m_info.nMatCen < segmentManager.RobotMakerCount ());
}

//------------------------------------------------------------------------------

bool CSegmentTool::IsEquipMaker (CSegment *segP)
{
return 
	(segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) &&
	(segP->m_info.nMatCen >= 0) &&
	(segP->m_info.nMatCen < segmentManager.EquipMakerCount ());
}

//------------------------------------------------------------------------------

void CSegmentTool::EnableControls (BOOL bEnable)
{
CHECKMINE;
CSegment *segP = current->Segment ();
// enable/disable "end of exit tunnel" button
EndOfExit ()->EnableWindow (segP->Child (m_nSide) < 0);
// enable/disable add segment button
GetDlgItem (IDC_CUBE_ADD)->EnableWindow ((segmentManager.Count () < MAX_SEGMENTS) &&
													  (vertexManager.Count () < MAX_VERTICES - 4) &&
													  (segP->Child (m_nSide) < 0));
GetDlgItem (IDC_CUBE_DEL)->EnableWindow (segmentManager.Count () > 1);
// enable/disable add robot button
GetDlgItem (IDC_CUBE_ADDBOT)->EnableWindow ((IsRobotMaker (segP) || IsEquipMaker (segP)) && (LBAvailBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_DELBOT)->EnableWindow ((IsRobotMaker (segP) || IsEquipMaker (segP)) && (LBUsedBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_WALLDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_TRIGGERDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_ADD_REPAIRCEN)->EnableWindow (DLE.IsD2File ());
GetDlgItem (IDC_CUBE_OWNER)->EnableWindow (DLE.IsD2XLevel ());
GetDlgItem (IDC_CUBE_GROUP)->EnableWindow (DLE.IsD2XLevel ());
}

//------------------------------------------------------------------------------

void CSegmentTool::OnSetCoord (void)
{
CHECKMINE;
UpdateData (TRUE);
undoManager.Begin (udVertices);
current->Segment ()->Vertex (current->Segment ()->m_info.verts [sideVertTable[current->m_nSide][current->m_nPoint]])->Set (m_nCoord [0], m_nCoord [1], m_nCoord [2]);
undoManager.End ();
DLE.MineView ()->Refresh (false);
}

//------------------------------------------------------------------------------

void CSegmentTool::OnResetCoord (void)
{
CHECKMINE;
CVertex* vertP = current->Segment ()->Vertex (sideVertTable [current->m_nSide][current->m_nPoint]);
m_nCoord [0] = vertP->v.x;
m_nCoord [1] = vertP->v.y;
m_nCoord [2] = vertP->v.z;
UpdateData (FALSE);
DLE.MineView ()->Refresh (false);
}

//------------------------------------------------------------------------------

void CSegmentTool::OnProp (int nProp)
{
CHECKMINE;
if (Prop (nProp)->GetCheck ())
	m_nProps |= 1 << nProp;
else
	m_nProps &= ~(1 << nProp);

BOOL bMarked = segmentManager.HaveMarkedSegments ();

undoManager.Begin (udSegments);
DLE.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CSegment *segP = segmentManager.Segment (0);
	for (short nSegNum = 0; nSegNum < segmentManager.Count (); nSegNum++, segP++)
		if (segP->IsMarked ())
			segP->m_info.props = m_nProps;
	}
else 					
	current->Segment ()->m_info.props = m_nProps;
DLE.MineView ()->DelayRefresh (false);
undoManager.End ();
}

void CSegmentTool::OnProp1 () { OnProp (0); }
void CSegmentTool::OnProp2 () { OnProp (1); }
void CSegmentTool::OnProp3 () { OnProp (2); }
void CSegmentTool::OnProp4 () { OnProp (3); }
void CSegmentTool::OnProp5 () { OnProp (4); }

//------------------------------------------------------------------------------

void CSegmentTool::OnSide (int nSide)
{
CHECKMINE;
current->m_nSide = m_nSide = nSide;
DLE.MineView ()->Refresh ();
}

void CSegmentTool::OnSide1 () { OnSide (0); }
void CSegmentTool::OnSide2 () { OnSide (1); }
void CSegmentTool::OnSide3 () { OnSide (2); }
void CSegmentTool::OnSide4 () { OnSide (3); }
void CSegmentTool::OnSide5 () { OnSide (4); }
void CSegmentTool::OnSide6 () { OnSide (5); }

//------------------------------------------------------------------------------

void CSegmentTool::OnPoint (int nPoint)
{
CHECKMINE;
current->m_nPoint = m_nPoint = nPoint;
DLE.MineView ()->Refresh ();
}

void CSegmentTool::OnPoint1 () { OnPoint (0); }
void CSegmentTool::OnPoint2 () { OnPoint (1); }
void CSegmentTool::OnPoint3 () { OnPoint (2); }
void CSegmentTool::OnPoint4 () { OnPoint (3); }

//------------------------------------------------------------------------------

void CSegmentTool::SetDefTexture (short nTexture)
{
CSegment *segP = segmentManager.Segment (m_nSegment);
if (m_bSetDefTexture = ((CButton *) GetDlgItem (IDC_CUBE_SETDEFTEXTURE))->GetCheck ()) {
	int i;
	for (i = 0; i < 6; i++)
		if (segP->Child (i) == -1)
			segmentManager.SetTextures (CSideKey (m_nSegment, i), nTexture, 0);
	}
}

//------------------------------------------------------------------------
// CSegmentTool - RefreshData
//------------------------------------------------------------------------

void CSegmentTool::Refresh (void) 
{
if (!(m_bInited && theMine))
	return;
InitCBSegmentNo ();
OnResetCoord ();

int h, i, j;

// update automatic data
segmentManager.RenumberRobotMakers ();
segmentManager.RenumberEquipMakers ();
// update segment number combo box if number of segments has changed
CSegment *segP = current->Segment ();
m_bEndOfExit = (segP->Child (current->m_nSide) == -2);
m_nSegment = current->m_nSegment;
m_nSide = current->m_nSide;
m_nPoint = current->m_nPoint;
m_nType = segP->m_info.function;
m_nDamage [0] = segP->m_info.damage [0];
m_nDamage [1] = segP->m_info.damage [1];
m_nProps = segP->m_info.props;
m_nOwner = segP->m_info.owner;
m_nGroup = segP->m_info.group;
//CBType ()->SetCurSel (m_nType);
SelectItemData (CBType (), m_nType);
OnResetCoord ();
  // show Triggers () that point at this segment
LBTriggers()->ResetContent();
CTrigger *trigP = triggerManager.Trigger (0);
int nTrigger;
for (nTrigger = 0; nTrigger < triggerManager.WallTriggerCount (); nTrigger++, trigP++) {
	for (i = 0; i < trigP->Count (); i++) {
		if ((*trigP) [i] == CSideKey (m_nSegment, m_nSide)) {
			// find the wallP with this trigP
			CWall *wallP = wallManager.Wall (0);
			int nWall;
			for (nWall = 0; nWall < wallManager.WallCount (); nWall++, wallP++) {
				if (wallP->Info ().nTrigger == nTrigger) 
					break;
				}
			if (nWall < wallManager.WallCount ()) {
				sprintf_s (message, sizeof (message),  "%d,%d", (int) wallP->m_nSegment, (int) wallP->m_nSide + 1);
				int h = LBTriggers ()->AddString (message);
				LBTriggers ()->SetItemData (h, (int) wallP->m_nSegment * 0x10000L + wallP->m_nSide);
				}
			}
		}
	}
// show if this is segment/side is trigPed by the control_center
CReactorTrigger* reactorTrigger = triggerManager.ReactorTrigger (0);
for (short nTrigger = 0; nTrigger < MAX_REACTOR_TRIGGERS; nTrigger++, reactorTrigger++) {
	if (-1 < (reactorTrigger->Find (CSideKey (m_nSegment, m_nSide)))) {
		LBTriggers ()->AddString ("Reactor");
		break;
		}
	}

// show "none" if there is no Triggers ()
if (!LBTriggers()->GetCount()) {
	LBTriggers()->AddString ("none");
	}

m_nLight = ((double) segP->m_info.staticLight) / (24 * 327.68);

CListBox *plb [2] = { LBAvailBots (), LBUsedBots () };
if (IsRobotMaker (segP)) {
	int nMatCen = segP->m_info.nMatCen;
	// if # of items in list box totals to less than the number of robots
	//    if (LBAvailBots ()->GetCount() + LBAvailBots ()->GetCount() < MAX_ROBOT_IDS) {
	
	int objFlags [2];
	CStringResource res;

	for (i = 0; i < 2; i++)
		objFlags [i] = segmentManager.RobotMaker (nMatCen)->m_info.objFlags [i];
	if ((m_nLastSegment != m_nSegment) || (m_nLastSide != m_nSide)) {
		for (i = 0; i < 2; i++) {
			plb [i]->ResetContent ();
			for (j = 0; j < 64; j++) {
				if (i) {
					h = ((objFlags [j / 32] & (1L << (j % 32))) != 0);
					if (!h)	//only add flagged objects to 2nd list box
						continue;
					}
				res.Load (ROBOT_STRING_TABLE + j);
				h = plb [i]->AddString (res.Value ());
				plb [i]->SetItemData (h, j);
				}
			plb [i]->SetCurSel (0);
			}
		}
	}
else if (IsEquipMaker (segP)) {
	int nMatCen = segP->m_info.nMatCen;
	// if # of items in list box totals to less than the number of robots
	//    if (LBAvailBots ()->GetCount() + LBAvailBots ()->GetCount() < MAX_ROBOT_IDS) {
	int objFlags [2];
	CStringResource res;

	for (i = 0; i < 2; i++)
		objFlags [i] = segmentManager.EquipMaker (nMatCen)->m_info.objFlags [i];
	if ((m_nLastSegment != m_nSegment) || (m_nLastSide != m_nSide)) {
		for (i = 0; i < 2; i++) {
			plb [i]->ResetContent ();
			for (j = 0; j < MAX_POWERUP_IDS_D2; j++) {
				if (i) {
					h = ((objFlags [j / 32] & (1L << (j % 32))) != 0);
					if (!h)	//only add flagged objects to 2nd list box
						continue;
					}
				res.Load (POWERUP_STRING_TABLE + j);
				if (!strcmp (res.Value (), "(not used)"))
					continue;
				h = plb [i]->AddString (res.Value ());
				plb [i]->SetItemData (h, j);
				}
			plb [i]->SetCurSel (0);
			}
		}
	}
else {
	sprintf_s (message, sizeof (message), "n/a");
	for (i = 0; i < 2; i++) {
		plb [i]->ResetContent();
		plb [i]->AddString(message);
		}
	}
m_nLastSegment = m_nSegment;
m_nLastSide = m_nSide;
EnableControls (TRUE);
UpdateData (FALSE);
}


//------------------------------------------------------------------------
// CSegmentTool - EndOfExitTunnel ()
//------------------------------------------------------------------------

void CSegmentTool::OnEndOfExit ()
{
CHECKMINE;
CSegment *segP = current->Segment ();
undoManager.Begin (udSegments);
m_bEndOfExit = EndOfExit ()->GetCheck ();
segP->SetChild (m_nSide, m_bEndOfExit ? -2 : -1);
undoManager.End ();
}

//------------------------------------------------------------------------
// CSegmentTool - Add Segment
//------------------------------------------------------------------------

void CSegmentTool::OnAddSegment () 
{
CHECKMINE;
segmentManager.Create ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - Delete Segment
//------------------------------------------------------------------------

void CSegmentTool::OnDeleteSegment () 
{
CHECKMINE;
segmentManager.Delete (current->m_nSegment);
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CSegmentTool::OnSetOwner (void)
{
CHECKMINE;

	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = segmentManager.HaveMarkedSegments ();

undoManager.Begin (udSegments);
DLE.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CSegment *segP = segmentManager.Segment (0);
	for (short nSegNum = 0; nSegNum < segmentManager.Count (); nSegNum++, segP++)
		if (segP->IsMarked ())
			segP->m_info.owner = m_nOwner;
	}
else 					
	current->Segment ()->m_info.owner = m_nOwner;
undoManager.End ();
DLE.MineView ()->DelayRefresh (false);
}

//------------------------------------------------------------------------

void CSegmentTool::OnSetGroup (void)
{
CHECKMINE;

	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = segmentManager.HaveMarkedSegments ();

undoManager.Begin (udSegments);
DLE.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CSegment *segP = segmentManager.Segment (0);
	for (short nSegNum = 0; nSegNum < segmentManager.Count (); nSegNum++, segP++)
		if (segP->IsMarked ())
			segP->m_info.group = m_nGroup;
	}
else 					
	current->Segment ()->m_info.group = m_nGroup;
undoManager.End ();
DLE.MineView ()->DelayRefresh (false);
}

//------------------------------------------------------------------------
// CSegmentTool - SpecialMsg
//------------------------------------------------------------------------

void CSegmentTool::OnSetType (void)
{
CHECKMINE;

	BOOL		bChangeOk = TRUE;
	BOOL		bMarked = segmentManager.HaveMarkedSegments ();
	int		nSegNum, nMinSeg, nMaxSeg;

DLE.MineView ()->DelayRefresh (true);
m_nLastSegment = -1; //force Refresh() to rebuild all dialog data
byte nType = byte (CBType ()->GetItemData (CBType ()->GetCurSel ()));
if (bMarked) {
	nMinSeg = 0;
	nMaxSeg = segmentManager.Count ();
	}
else {
	nMinSeg = int (current->Segment () - segmentManager.Segment (0));
	nMaxSeg = nMinSeg + 1;
	}
undoManager.Begin (udSegments);
CSegment* segP = segmentManager.Segment (nMinSeg);
for (nSegNum = nMinSeg; nSegNum < nMaxSeg; nSegNum++, segP++) {
	if (bMarked && !segP->IsMarked ())
		continue;
	m_nType = segP->m_info.function;
	switch(nType) {
		// check to see if we are adding a robot maker
		case SEGMENT_FUNC_ROBOTMAKER:
			if (nType == m_nType)
				goto errorExit;
			if (!segmentManager.CreateRobotMaker (nSegNum, false, m_bSetDefTexture == 1)) {
				undoManager.Unroll ();
				goto funcExit;
				}
			break;

		// check to see if we are adding a fuel center
		case SEGMENT_FUNC_REPAIRCEN:
			if (DLE.IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}

		case SEGMENT_FUNC_FUELCEN:
			if (nType == m_nType)
				continue;
			if (!segmentManager.CreateFuelCenter (nSegNum, nType, false, (nType == SEGMENT_FUNC_FUELCEN) && (m_bSetDefTexture == 1))) {
				undoManager.Unroll ();
				goto funcExit;
				}
			break;

		case SEGMENT_FUNC_REACTOR:
			if (nType == m_nType)
				continue;
			if (!segmentManager.CreateReactor (nSegNum, false, m_bSetDefTexture == 1)) {
				undoManager.Unroll ();
				goto funcExit;
				}
			break;

		case SEGMENT_FUNC_GOAL_BLUE:
		case SEGMENT_FUNC_GOAL_RED:
			if (nType == m_nType)
				continue;
			if (!segmentManager.CreateGoal (nSegNum, false, m_bSetDefTexture == 1, nType, -1))
				goto errorExit;		
			break;

		case SEGMENT_FUNC_TEAM_BLUE:
		case SEGMENT_FUNC_TEAM_RED:
			if (DLE.IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (nType == m_nType)
				continue;
			if (!segmentManager.CreateTeam (nSegNum, false, false, nType, -1))
				goto errorExit;		
			break;

		case SEGMENT_FUNC_SPEEDBOOST:
			if (DLE.IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!segmentManager.CreateSpeedBoost (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_SKYBOX:
			if (DLE.IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!segmentManager.CreateSkybox (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_EQUIPMAKER:
			if (DLE.IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!segmentManager.CreateEquipMaker (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_NONE:
			segmentManager.Undefine (nSegNum);
			break;

		default:
			break;
		}
	m_nType = nType;
	}

errorExit:

undoManager.End ();
triggerManager.UpdateReactor ();

funcExit:

DLE.MineView ()->DelayRefresh (false);
UpdateData (TRUE);
}

//------------------------------------------------------------------------
// CSegmentTool - Segment Number Message
//------------------------------------------------------------------------

void CSegmentTool::OnSetSegment () 
{
CHECKMINE;
current->m_nSegment = CBSegmentNo ()->GetCurSel ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - LightMsg
//------------------------------------------------------------------------

void CSegmentTool::OnLight () 
{
CHECKMINE;
UpdateData (TRUE);
undoManager.Begin (udSegments);
current->Segment ()->m_info.staticLight = (int) (m_nLight * 24 * 327.68);
undoManager.End ();
}

//------------------------------------------------------------------------------

void CSegmentTool::OnDamage (int i) 
{
CHECKMINE;
UpdateData (TRUE);
undoManager.Begin (udSegments);
current->Segment ()->m_info.damage [i] = m_nDamage [i];
undoManager.End ();
}

void CSegmentTool::OnDamage0 () { OnDamage (0); }
void CSegmentTool::OnDamage1 () { OnDamage (1); }

//------------------------------------------------------------------------------

int CSegmentTool::FindRobotMaker (CListBox *plb, LPSTR pszObj)
{
	int i, j;

i = plb->GetCurSel ();
if (i < 0)
	return -1;
j = int (plb->GetItemData (i));
if (pszObj)
	LoadString (AfxGetInstanceHandle(), ROBOT_STRING_TABLE + j, pszObj, 80);
return j;
}

//------------------------------------------------------------------------------

int CSegmentTool::FindEquipMaker (CListBox *plb, LPSTR pszObj)
{
	int i, j;

i = plb->GetCurSel ();
if (i < 0)
	return -1;
j = int (plb->GetItemData (i));
if (pszObj)
	LoadString (AfxGetInstanceHandle(), POWERUP_STRING_TABLE + j, pszObj, 80);
return j;
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------

void CSegmentTool::AddRobotMaker ()
{
CHECKMINE;
CSegment *segP = current->Segment ();
char szObj [80];
int i = FindRobotMaker (LBAvailBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
undoManager.Begin (udMatCenters);
segmentManager.RobotMaker (segP->m_info.nMatCen)->m_info.objFlags [i / 32] |= (1L << (i % 32));
undoManager.End ();
int h = LBAvailBots ()->GetCurSel ();
LBAvailBots ()->DeleteString (h);
LBAvailBots ()->SetCurSel (h);
h = LBUsedBots ()->AddString (szObj);
LBUsedBots ()->SetItemData (h, i);
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------

void CSegmentTool::AddEquipMaker ()
{
CHECKMINE;
CSegment *segP = current->Segment ();
char szObj [80];
int i = FindEquipMaker (LBAvailBots (), szObj);
if ((i < 0) || (i >= MAX_POWERUP_IDS_D2))
	return;
undoManager.Begin (udMatCenters);
segmentManager.EquipMaker (segP->m_info.nMatCen)->m_info.objFlags [i / 32] |= (1L << (i % 32));
undoManager.End ();
int h = LBAvailBots ()->GetCurSel ();
LBAvailBots ()->DeleteString (h);
LBAvailBots ()->SetCurSel (h);
h = LBUsedBots ()->AddString (szObj);
LBUsedBots ()->SetItemData (h, i);
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - AddMsg
//------------------------------------------------------------------------

void CSegmentTool::OnAddMatCenter ()
{
CHECKMINE;
CSegment *segP = current->Segment ();
if (IsRobotMaker (segP))
	AddRobotMaker ();
else if (IsEquipMaker (segP))
	AddEquipMaker ();
}

//------------------------------------------------------------------------
// CSegmentTool - DelMsg
//------------------------------------------------------------------------

void CSegmentTool::DeleteRobotMaker () 
{
CHECKMINE;
CSegment *segP = current->Segment ();
char szObj [80];
int i = FindRobotMaker (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
undoManager.Begin (udMatCenters);
segmentManager.RobotMaker (segP->m_info.nMatCen)->m_info.objFlags [i / 32] &= ~(1L << (i % 32));
undoManager.End ();
int h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - DelMsg
//------------------------------------------------------------------------

void CSegmentTool::DeleteEquipMaker () 
{
CHECKMINE;
CSegment *segP = current->Segment ();
char szObj [80];
int i = FindEquipMaker (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
undoManager.Begin (udMatCenters);
segmentManager.EquipMaker (segP->m_info.nMatCen)->m_info.objFlags [i / 32] &= ~(1L << (i % 32));
undoManager.End ();
int h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - DelMsg
//------------------------------------------------------------------------

void CSegmentTool::OnDeleteMatCenter () 
{
CHECKMINE;
CSegment *segP = current->Segment ();
if (IsRobotMaker (segP))
	DeleteRobotMaker ();
else if (IsEquipMaker (segP))
	DeleteEquipMaker ();
}

//------------------------------------------------------------------------
// CSegmentTool - OtherSegmentMsg
//------------------------------------------------------------------------

void CSegmentTool::OnOtherSegment () 
{
DLE.MineView ()->SelectOtherSegment ();
}

//------------------------------------------------------------------------
// CSegmentTool - SegmentButtonMsg
//------------------------------------------------------------------------

void CSegmentTool::OnWallDetails () 
{
CHECKMINE;
if (!LBTriggers ()->GetCount ())
	return;
int i = LBTriggers ()->GetCurSel ();
if (i < 0)
	return;
long h = long (LBTriggers ()->GetItemData (i));
current->m_nSegment = (short) (h / 0x10000L);
current->m_nSide = (short) (h % 0x10000L);
DLE.ToolView ()->EditWall ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - TriggerButtonMsg
//------------------------------------------------------------------------

void CSegmentTool::OnTriggerDetails ()
{
CHECKMINE;
if (!LBTriggers ()->GetCount ())
	return;
int i = LBTriggers ()->GetCurSel ();
if ((i < 0) || (i >= LBTriggers ()->GetCount ()))
	return;
long h = long (LBTriggers ()->GetItemData (i));
other->m_nSegment = current->m_nSegment;
other->m_nSide = current->m_nSide;
current->m_nSegment = (short) (h / 0x10000L);
current->m_nSide = (short) (h % 0x10000L);
DLE.ToolView ()->EditTrigger ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CSegmentTool::OnAddRobotMaker ()
{
CHECKMINE;
AddRobotMaker ();
m_nLastSegment = -1;
Refresh ();
}

//------------------------------------------------------------------------------

void CSegmentTool::OnAddEquipMaker ()
{
CHECKMINE;
AddEquipMaker ();
m_nLastSegment = -1;
Refresh ();
}

//------------------------------------------------------------------------------

void CSegmentTool::OnAddFuelCenter ()
{
CHECKMINE;
segmentManager.CreateFuelCenter ();
}

//------------------------------------------------------------------------------

void CSegmentTool::OnAddRepairCenter ()
{
CHECKMINE;
segmentManager.CreateFuelCenter (-1, SEGMENT_FUNC_REPAIRCEN);
}

//------------------------------------------------------------------------------

void CSegmentTool::OnAddReactor ()
{
CHECKMINE;
segmentManager.CreateReactor ();
}

//------------------------------------------------------------------------------

void CSegmentTool::OnSplitSegment ()
{
CHECKMINE;
segmentManager.Split ();
}

//------------------------------------------------------------------------------

//eof segmenttool.cpp