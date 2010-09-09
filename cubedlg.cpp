// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
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

                        /*--------------------------*/

BEGIN_MESSAGE_MAP (CSegmentTool, CToolDlg)
	ON_BN_CLICKED (IDC_CUBE_SETCOORD, OnSetCoord)
	ON_BN_CLICKED (IDC_CUBE_RESETCOORD, OnResetCoord)
	ON_BN_CLICKED (IDC_CUBE_ADD, OnAddCube)
	ON_BN_CLICKED (IDC_CUBE_ADD_MATCEN, OnAddRobotMaker)
	ON_BN_CLICKED (IDC_CUBE_ADD_EQUIPMAKER, OnAddEquipMaker)
	ON_BN_CLICKED (IDC_CUBE_ADD_FUELCEN, OnAddFuelCen)
	ON_BN_CLICKED (IDC_CUBE_ADD_REPAIRCEN, OnAddRepairCen)
	ON_BN_CLICKED (IDC_CUBE_ADD_CONTROLCEN, OnAddControlCen)
	ON_BN_CLICKED (IDC_CUBE_SPLIT, OnSplitCube)
	ON_BN_CLICKED (IDC_CUBE_DEL, OnDeleteCube)
	ON_BN_CLICKED (IDC_CUBE_OTHER, OnOtherCube)
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
	ON_CBN_SELCHANGE (IDC_CUBE_CUBENO, OnSetCube)
	ON_CBN_SELCHANGE (IDC_CUBE_TYPE, OnSetType)
	ON_CBN_SELCHANGE (IDC_CUBE_OWNER, OnSetOwner)
	ON_EN_KILLFOCUS (IDC_CUBE_LIGHT, OnLight)
	ON_EN_KILLFOCUS (IDC_CUBE_SHIELD_DAMAGE, OnDamage0)
	ON_EN_KILLFOCUS (IDC_CUBE_ENERGY_DAMAGE, OnDamage1)
	ON_EN_KILLFOCUS (IDC_CUBE_GROUP, OnSetGroup)
	ON_EN_UPDATE (IDC_CUBE_LIGHT, OnLight)
	ON_LBN_DBLCLK (IDC_CUBE_TRIGGERS, OnTriggerDetails)
END_MESSAGE_MAP ()

                        /*--------------------------*/

CSegmentTool::CSegmentTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_CUBEDATA2 : IDD_CUBEDATA, pParent)
{
Reset ();
}

                        /*--------------------------*/

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
m_nLastCube =
m_nLastSide = -1;
m_bSetDefTexture = 0;
m_nOwner = segmentManager.Segment (0)->m_info.owner;
m_nGroup = segmentManager.Segment (0)->m_info.group;
memset (m_nCoord, 0, sizeof (m_nCoord));
}

                        /*--------------------------*/

void CSegmentTool::InitCBCubeNo (void)
{
CHECKMINE;
CComboBox *pcb = CBCubeNo ();
if (segmentManager.Count () != pcb->GetCount ()) {
	pcb->ResetContent ();
	for (int i = 0; i < segmentManager.Count (); i++) {
		_itoa_s (i, message, sizeof (message), 10);
		pcb->AddString (message);
		}
	}
pcb->SetCurSel (m_nSegment);
}

                        /*--------------------------*/

BOOL CSegmentTool::OnInitDialog ()
{
if (!CToolDlg::OnInitDialog ())
	return FALSE;

	static char* pszCubeFuncs [] = {
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
for (j = sizeof (pszCubeFuncs) / sizeof (*pszCubeFuncs), i = 0; i < j; i++) {
	h = pcb->AddString (pszCubeFuncs [i]);
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

                        /*--------------------------*/

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


                        /*--------------------------*/

BOOL CSegmentTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

                        /*--------------------------*/

bool CSegmentTool::IsRobotMaker (CSegment *segP)
{
return 
	(segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) &&
	(segP->m_info.nMatCen >= 0) &&
	(segP->m_info.nMatCen < theMine->Info ().botGen.count);
}

                        /*--------------------------*/

bool CSegmentTool::IsEquipMaker (CSegment *segP)
{
return 
	(segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) &&
	(segP->m_info.nMatCen >= 0) &&
	(segP->m_info.nMatCen < theMine->Info ().equipGen.count);
}

                        /*--------------------------*/

void CSegmentTool::EnableControls (BOOL bEnable)
{
CHECKMINE;
CSegment *segP = current.Segment ();
// enable/disable "end of exit tunnel" button
EndOfExit ()->EnableWindow (segP->Child (m_nSide) < 0);
// enable/disable add cube button
GetDlgItem (IDC_CUBE_ADD)->EnableWindow ((segmentManager.Count () < MAX_SEGMENTS) &&
													  (theMine->vertexManager.Count () < MAX_VERTICES - 4) &&
													  (segP->Child (m_nSide) < 0));
GetDlgItem (IDC_CUBE_DEL)->EnableWindow (segmentManager.Count () > 1);
// enable/disable add robot button
GetDlgItem (IDC_CUBE_ADDBOT)->EnableWindow ((IsRobotMaker (segP) || IsEquipMaker (segP)) && (LBAvailBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_DELBOT)->EnableWindow ((IsRobotMaker (segP) || IsEquipMaker (segP)) && (LBUsedBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_WALLDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_TRIGGERDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_ADD_REPAIRCEN)->EnableWindow (DLE.IsD2File ());
GetDlgItem (IDC_CUBE_OWNER)->EnableWindow (theMine->IsD2XLevel ());
GetDlgItem (IDC_CUBE_GROUP)->EnableWindow (theMine->IsD2XLevel ());
}

                        /*--------------------------*/

void CSegmentTool::OnSetCoord (void)
{
CHECKMINE;
UpdateData (TRUE);
undoManager.Begin (udVertices);
current.Segment ()->Vertex (m_info.verts [sideVertTable[current.m_nSide][current.m_nPoint]])->Set (m_nCoord [0], m_nCoord [1], m_nCoord [2]);
undoManager.End ();
DLE.MineView ()->Refresh (false);
}

                        /*--------------------------*/

void CSegmentTool::OnResetCoord (void)
{
CHECKMINE;
CVertex* vertP = current.Segment ()->Vertex (m_info.verts [sideVertTable[current.m_nSide][current.m_nPoint]]);
m_nCoord [0] = vertP->v.x;
m_nCoord [1] = vertP->v.y;
m_nCoord [2] = vertP->v.z;
UpdateData (FALSE);
DLE.MineView ()->Refresh (false);
}

                        /*--------------------------*/

void CSegmentTool::OnProp (int nProp)
{
CHECKMINE;
undoManager.Begin (udSegments);
if (Prop (nProp)->GetCheck ())
	m_nProps |= 1 << nProp;
else
	m_nProps &= ~(1 << nProp);
current.Segment ()->m_info.props = m_nProps;
undoManager.End ();
}

void CSegmentTool::OnProp1 () { OnProp (0); }
void CSegmentTool::OnProp2 () { OnProp (1); }
void CSegmentTool::OnProp3 () { OnProp (2); }
void CSegmentTool::OnProp4 () { OnProp (3); }
void CSegmentTool::OnProp5 () { OnProp (4); }

                        /*--------------------------*/

void CSegmentTool::OnSide (int nSide)
{
CHECKMINE;
current.m_nSide = m_nSide = nSide;
DLE.MineView ()->Refresh ();
}

void CSegmentTool::OnSide1 () { OnSide (0); }
void CSegmentTool::OnSide2 () { OnSide (1); }
void CSegmentTool::OnSide3 () { OnSide (2); }
void CSegmentTool::OnSide4 () { OnSide (3); }
void CSegmentTool::OnSide5 () { OnSide (4); }
void CSegmentTool::OnSide6 () { OnSide (5); }

                        /*--------------------------*/

void CSegmentTool::OnPoint (int nPoint)
{
CHECKMINE;
current.m_nPoint = m_nPoint = nPoint;
DLE.MineView ()->Refresh ();
}

void CSegmentTool::OnPoint1 () { OnPoint (0); }
void CSegmentTool::OnPoint2 () { OnPoint (1); }
void CSegmentTool::OnPoint3 () { OnPoint (2); }
void CSegmentTool::OnPoint4 () { OnPoint (3); }

                        /*--------------------------*/

void CSegmentTool::SetDefTexture (short nTexture)
{
CSegment *segP = segmentManager.Segment (m_nSegment);
if (m_bSetDefTexture = ((CButton *) GetDlgItem (IDC_CUBE_SETDEFTEXTURE))->GetCheck ()) {
	int i;
	for (i = 0; i < 6; i++)
		if (segP->Child (i) == -1)
			segmentManager.SetTextures (m_nSegment, i, nTexture, 0);
	}
}

//------------------------------------------------------------------------
// CSegmentTool - RefreshData
//------------------------------------------------------------------------

void CSegmentTool::Refresh (void) 
{
if (!(m_bInited && theMine))
	return;
InitCBCubeNo ();
OnResetCoord ();

int h, i, j;

// update automatic data
theMine->RenumberBotGens ();
theMine->RenumberEquipGens ();
// update cube number combo box if number of cubes has changed
CSegment *segP = current.Segment ();
m_bEndOfExit = (segP->Child (current.m_nSide) == -2);
m_nSegment = current.m_nSegment;
m_nSide = current.m_nSide;
m_nPoint = current.m_nPoint;
m_nType = segP->m_info.function;
m_nDamage [0] = segP->m_info.damage [0];
m_nDamage [1] = segP->m_info.damage [1];
m_nProps = segP->m_info.props;
m_nOwner = segP->m_info.owner;
m_nGroup = segP->m_info.group;
//CBType ()->SetCurSel (m_nType);
SelectItemData (CBType (), m_nType);
OnResetCoord ();
  // show Triggers () that point at this cube
LBTriggers()->ResetContent();
CTrigger *trigP = wallManager.Trigger (0);
int nTrigger;
for (nTrigger = 0; nTrigger < theMine->Info ().triggers.count; nTrigger++, trigP++) {
	for (i = 0; i < trigP->m_count; i++) {
		if (trigP->m_targets [i] == CSideKey (m_nSegment, m_nSide)) {
			// find the wallP with this trigP
			CWall *wallP = wallManager.Wall (0);
			int nWall;
			for (nWall = 0; nWall < theMine->Info ().walls.count ;nWall++, wallP++) {
				if (wallP->m_info.nTrigger == nTrigger) 
					break;
				}
			if (nWall < theMine->Info ().walls.count) {
				sprintf_s (message, sizeof (message),  "%d,%d", (int) wallP->m_nSegment, (int) wallP->m_nSide + 1);
				int h = LBTriggers ()->AddString (message);
				LBTriggers ()->SetItemData (h, (int) wallP->m_nSegment * 0x10000L + wallP->m_nSide);
				}
			}
		}
	}
// show if this is cube/side is trigPed by the control_center
CReactorTrigger* reactorTrigger = theMine->ReactorTriggers (0);
int control;
for (control = 0; control < MAX_REACTOR_TRIGGERS; control++, reactorTrigger++) {
	if (-1 < (reactorTrigger->Find (m_nSegment, m_nSide))) {
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
	if ((m_nLastCube != m_nSegment) || (m_nLastSide != m_nSide)) {
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
		objFlags [i] = theMine->EquipMakers (nMatCen)->m_info.objFlags [i];
	if ((m_nLastCube != m_nSegment) || (m_nLastSide != m_nSide)) {
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
m_nLastCube = m_nSegment;
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
CSegment *segP = current.Segment ();
undoManager.Begin (udSegments);
m_bEndOfExit = EndOfExit ()->GetCheck ();
segP->SetChild (m_nSide, m_bEndOfExit ? -2 : -1);
undoManager.End ();
}

//------------------------------------------------------------------------
// CSegmentTool - Add Cube
//------------------------------------------------------------------------

void CSegmentTool::OnAddCube () 
{
CHECKMINE;
theMine->AddSegment ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - Delete Cube
//------------------------------------------------------------------------

void CSegmentTool::OnDeleteCube () 
{
CHECKMINE;
theMine->DeleteSegment (current.m_nSegment);
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CSegmentTool::OnSetOwner (void)
{
CHECKMINE;

	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = theMine->GotMarkedSegments ();


undoManager.Begin (udSegments);
DLE.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CSegment *segP = segmentManager.Segment (0);
	for (short nSegNum = 0; nSegNum < segmentManager.Count (); nSegNum++, segP++)
		if (segP->Marked ())
			segP->m_info.owner = m_nOwner;
	}
else 					
	current.Segment ()->m_info.owner = m_nOwner;
undoManager.End ();
DLE.MineView ()->DelayRefresh (false);
}

//------------------------------------------------------------------------

void CSegmentTool::OnSetGroup (void)
{
CHECKMINE;

	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = theMine->GotMarkedSegments ();

undoManager.Begin (udSegments);
DLE.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CSegment *segP = segmentManager.Segment (0);
	for (short nSegNum = 0; nSegNum < segmentManager.Count (); nSegNum++, segP++)
		if (segP->Marked ())
			segP->m_info.group = m_nGroup;
	}
else 					
	current.Segment ()->m_info.group = m_nGroup;
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
	BOOL		bMarked = theMine->GotMarkedSegments ();
	int		nSegNum, nMinSeg, nMaxSeg;

DLE.MineView ()->DelayRefresh (true);
m_nLastCube = -1; //force Refresh() to rebuild all dialog data
byte nType = byte (CBType ()->GetItemData (CBType ()->GetCurSel ()));
if (bMarked) {
	nMinSeg = 0;
	nMaxSeg = segmentManager.Count ();
	}
else {
	nMinSeg = int (current.Segment () - segmentManager.Segment (0));
	nMaxSeg = nMinSeg + 1;
	}
undoManager.Begin (udSegments);
CSegment* segP = segmentManager.Segment (nMinSeg);
for (nSegNum = nMinSeg; nSegNum < nMaxSeg; nSegNum++, segP++) {
	if (bMarked && !segP->Marked ())
		continue;
	m_nType = segP->m_info.function;
	switch(nType) {
		// check to see if we are adding a robot maker
		case SEGMENT_FUNC_ROBOTMAKER:
			if (nType == m_nType)
				goto errorExit;
			if ((theMine == null)->AddRobotMaker (nSegNum, false, m_bSetDefTexture == 1)) {
				undoManager.Unroll ();
				goto funcExit;
				}
			break;

		// check to see if we are adding a fuel center
		case SEGMENT_FUNC_REPAIRCEN:
			if (theMine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}

		case SEGMENT_FUNC_FUELCEN:
			if (nType == m_nType)
				continue;
			if ((theMine == null)->AddFuelCenter (nSegNum, nType, false, (nType == SEGMENT_FUNC_FUELCEN) && (m_bSetDefTexture == 1))) {
				undoManager.Unroll ();
				goto funcExit;
				}
			break;

		case SEGMENT_FUNC_REACTOR:
			if (nType == m_nType)
				continue;
			if ((theMine == null)->AddReactor (nSegNum, false, m_bSetDefTexture == 1)) {
				undoManager.Unroll ();
				goto funcExit;
				}
			break;

		case SEGMENT_FUNC_GOAL_BLUE:
		case SEGMENT_FUNC_GOAL_RED:
			if (nType == m_nType)
				continue;
			if ((theMine == null)->AddGoalCube (nSegNum, false, m_bSetDefTexture == 1, nType, -1))
				goto errorExit;		
			break;

		case SEGMENT_FUNC_TEAM_BLUE:
		case SEGMENT_FUNC_TEAM_RED:
			if (theMine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (nType == m_nType)
				continue;
			if ((theMine == null)->AddTeamCube (nSegNum, false, false, nType, -1))
				goto errorExit;		
			break;

		case SEGMENT_FUNC_SPEEDBOOST:
			if (theMine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if ((theMine == null)->AddSpeedBoostCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_SKYBOX:
			if (theMine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if ((theMine == null)->AddSkyboxCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_EQUIPMAKER:
			if (theMine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if ((theMine == null)->AddEquipMaker (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_NONE:
			theMine->UndefineSegment (nSegNum);
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
// CSegmentTool - Cube Number Message
//------------------------------------------------------------------------

void CSegmentTool::OnSetCube () 
{
CHECKMINE;
current.m_nSegment = CBCubeNo ()->GetCurSel ();
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
current.Segment ()->m_info.staticLight = (int) (m_nLight * 24 * 327.68);
undoManager.End ();
}

                        /*--------------------------*/

void CSegmentTool::OnDamage (int i) 
{
CHECKMINE;
UpdateData (TRUE);
undoManager.Begin (udSegments);
current.Segment ()->m_info.damage [i] = m_nDamage [i];
undoManager.End ();
}

void CSegmentTool::OnDamage0 () { OnDamage (0); }
void CSegmentTool::OnDamage1 () { OnDamage (1); }

                        /*--------------------------*/

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

                        /*--------------------------*/

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
CSegment *segP = current.Segment ();
char szObj [80];
int i = FindBot (LBAvailBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
undoManager.Begin (udRobotMaker);
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
CSegment *segP = current.Segment ();
char szObj [80];
int i = FindEquip (LBAvailBots (), szObj);
if ((i < 0) || (i >= MAX_POWERUP_IDS_D2))
	return;
undoManager.Begin (udEquipMaker);
theMine->EquipMakers (segP->m_info.nMatCen)->m_info.objFlags [i / 32] |= (1L << (i % 32));
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
CSegment *segP = current.Segment ();
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
CSegment *segP = current.Segment ();
char szObj [80];
int i = FindBot (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
undoManager.Begin (udRobotMaker);
segmentManager.RobotMaker (segP->m_info.nMatCen)->m_info.objFlags [i / 32] &= ~(1L << (i % 32));
undoManager.End ();
int h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
undoManager.Begin (true);
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - DelMsg
//------------------------------------------------------------------------

void CSegmentTool::DeleteEquip () 
{
CHECKMINE;
CSegment *segP = current.Segment ();
char szObj [80];
int i = FindEquip (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
undoManager.Begin (udEquipMaker);
theMine->EquipMakers (segP->m_info.nMatCen)->m_info.objFlags [i / 32] &= ~(1L << (i % 32));
undoManager.End ();
int h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
undoManager.Begin (true);
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - DelMsg
//------------------------------------------------------------------------

void CSegmentTool::OnDeleteMatCenter () 
{
CHECKMINE;
CSegment *segP = current.Segment ();
if (IsRobotMaker (segP))
	DeleteRobotMaker ();
else if (IsEquipMaker (segP))
	DeleteEquipMaker ();
}

//------------------------------------------------------------------------
// CSegmentTool - OtherCubeMsg
//------------------------------------------------------------------------

void CSegmentTool::OnOtherCube () 
{
DLE.MineView ()->SelectOtherSegment ();
}

//------------------------------------------------------------------------
// CSegmentTool - CubeButtonMsg
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
current.m_nSegment = (short) (h / 0x10000L);
current.m_nSide = (short) (h % 0x10000L);
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
other.m_nSegment = current.m_nSegment;
other.m_nSide = current.m_nSide;
current.m_nSegment = (short) (h / 0x10000L);
current.m_nSide = (short) (h % 0x10000L);
DLE.ToolView ()->EditTrigger ();
DLE.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CSegmentTool::OnAddBotGen ()
{
CHECKMINE;
theMine->AddRobotMaker ();
m_nLastCube = -1;
Refresh ();
}

                        /*--------------------------*/

void CSegmentTool::OnAddEquipGen ()
{
CHECKMINE;
theMine->AddEquipMaker ();
m_nLastCube = -1;
Refresh ();
}

                        /*--------------------------*/

void CSegmentTool::OnAddFuelCen ()
{
CHECKMINE;
theMine->AddFuelCenter ();
}

                        /*--------------------------*/

void CSegmentTool::OnAddRepairCen ()
{
CHECKMINE;
theMine->AddFuelCenter (-1, SEGMENT_FUNC_REPAIRCEN);
}

                        /*--------------------------*/

void CSegmentTool::OnAddControlCen ()
{
CHECKMINE;
theMine->AddReactor ();
}

                        /*--------------------------*/

void CSegmentTool::OnSplitCube ()
{
CHECKMINE;
theMine->SplitSegment ();
}

                        /*--------------------------*/

//eof cubedlg.cpp