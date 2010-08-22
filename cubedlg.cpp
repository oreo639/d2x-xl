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
	ON_BN_CLICKED (IDC_CUBE_ADD_MATCEN, OnAddBotGen)
	ON_BN_CLICKED (IDC_CUBE_ADD_EQUIPMAKER, OnAddEquipGen)
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
	ON_BN_CLICKED (IDC_CUBE_ADDBOT, OnAddObj)
	ON_BN_CLICKED (IDC_CUBE_DELBOT, OnDeleteObj)
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
if (!theMine) return;

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
m_nOwner = theMine->Segments (0)->owner;
m_nGroup = theMine->Segments (0)->group;
memset (m_nCoord, 0, sizeof (m_nCoord));
}

                        /*--------------------------*/

void CSegmentTool::InitCBCubeNo ()
{
CComboBox *pcb = CBCubeNo ();
if (theMine->SegCount () != pcb->GetCount ()) {
	pcb->ResetContent ();
	for (INT32 i = 0; i < theMine->SegCount (); i++) {
		_itoa_s (i, message, sizeof (message), 10);
		pcb->AddString (message);
		}
	}
pcb->SetCurSel (m_nSegment);
}

                        /*--------------------------*/

BOOL CSegmentTool::OnInitDialog ()
{
if (!(theMine && CToolDlg::OnInitDialog ()))
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

INT32 h, i, j;
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
m_bInited = TRUE;
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
for (INT32	i = 0; i < 3; i++) {
	DDX_Double (pDX, IDC_CUBE_POINTX + i, m_nCoord [i]);
	if (m_nCoord [i] < -0x7fff)
		m_nCoord [i] = -0x7fff;
	else if (m_nCoord [i] > 0x7fff)
		m_nCoord [i] = 0x7fff;
//	DDV_MinMaxInt (pDX, (FIX) m_nCoord [i], -0x7fff, 0x7fff);
	}

INT32 i;

for (i = 0; i < 5; i++) {
	INT32 h = (m_nProps & (1 << i)) != 0;
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

bool CSegmentTool::IsBotMaker (CSegment *segP)
{
return 
	(segP->function == SEGMENT_FUNC_ROBOTMAKER) &&
	(segP->nMatCen >= 0) &&
	(segP->nMatCen < theMine->GameInfo ().botgen.count);
}

                        /*--------------------------*/

bool CSegmentTool::IsEquipMaker (CSegment *segP)
{
return 
	(segP->function == SEGMENT_FUNC_EQUIPMAKER) &&
	(segP->nMatCen >= 0) &&
	(segP->nMatCen < theMine->GameInfo ().equipgen.count);
}

                        /*--------------------------*/

void CSegmentTool::EnableControls (BOOL bEnable)
{
CSegment *segP = theMine->CurrSeg ();
// enable/disable "end of exit tunnel" button
EndOfExit ()->EnableWindow (segP->children [m_nSide] < 0);
// enable/disable add cube button
GetDlgItem (IDC_CUBE_ADD)->EnableWindow ((theMine->SegCount () < MAX_SEGMENTS) &&
													  (theMine->VertCount () < MAX_VERTICES - 4) &&
													  (segP->children [m_nSide] < 0));
GetDlgItem (IDC_CUBE_DEL)->EnableWindow (theMine->SegCount () > 1);
// enable/disable add robot button
GetDlgItem (IDC_CUBE_ADDBOT)->EnableWindow ((IsBotMaker (segP) || IsEquipMaker (segP)) && (LBAvailBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_DELBOT)->EnableWindow ((IsBotMaker (segP) || IsEquipMaker (segP)) && (LBUsedBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_WALLDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_TRIGGERDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_ADD_REPAIRCEN)->EnableWindow (theApp.IsD2File ());
GetDlgItem (IDC_CUBE_OWNER)->EnableWindow (theMine->IsD2XLevel ());
GetDlgItem (IDC_CUBE_GROUP)->EnableWindow (theMine->IsD2XLevel ());
}

                        /*--------------------------*/

void CSegmentTool::OnSetCoord ()
{
UpdateData (TRUE);
theApp.SetModified (TRUE);
m_nVertex = theMine->CurrSeg ()->verts[side_vert[theMine->Current ()->nSide][theMine->Current ()->nPoint]];
theMine->Vertices (m_nVertex)->x = (FIX) (m_nCoord [0] * 0x10000L);
theMine->Vertices (m_nVertex)->y = (FIX) (m_nCoord [1] * 0x10000L);
theMine->Vertices (m_nVertex)->z = (FIX) (m_nCoord [2] * 0x10000L);
theApp.MineView ()->Refresh (false);
}

                        /*--------------------------*/

void CSegmentTool::OnResetCoord ()
{
m_nVertex = theMine->CurrSeg ()->verts[side_vert[theMine->Current ()->nSide][theMine->Current ()->nPoint]];
m_nCoord [0] = (double) theMine->Vertices (m_nVertex)->x / 0x10000L;
m_nCoord [1] = (double) theMine->Vertices (m_nVertex)->y / 0x10000L;
m_nCoord [2] = (double) theMine->Vertices (m_nVertex)->z / 0x10000L;
UpdateData (FALSE);
theApp.MineView ()->Refresh (false);
}

                        /*--------------------------*/

void CSegmentTool::OnProp (INT32 nProp)
{
theApp.SetModified (TRUE);
if (Prop (nProp)->GetCheck ())
	m_nProps |= 1 << nProp;
else
	m_nProps &= ~(1 << nProp);
theMine->CurrSeg ()->props = m_nProps;
}

void CSegmentTool::OnProp1 () { OnProp (0); }
void CSegmentTool::OnProp2 () { OnProp (1); }
void CSegmentTool::OnProp3 () { OnProp (2); }
void CSegmentTool::OnProp4 () { OnProp (3); }
void CSegmentTool::OnProp5 () { OnProp (4); }

                        /*--------------------------*/

void CSegmentTool::OnSide (INT32 nSide)
{
theMine->Current ()->nSide = m_nSide = nSide;
theApp.MineView ()->Refresh ();
}

void CSegmentTool::OnSide1 () { OnSide (0); }
void CSegmentTool::OnSide2 () { OnSide (1); }
void CSegmentTool::OnSide3 () { OnSide (2); }
void CSegmentTool::OnSide4 () { OnSide (3); }
void CSegmentTool::OnSide5 () { OnSide (4); }
void CSegmentTool::OnSide6 () { OnSide (5); }

                        /*--------------------------*/

void CSegmentTool::OnPoint (INT32 nPoint)
{
theMine->Current ()->nPoint = m_nPoint = nPoint;
theApp.MineView ()->Refresh ();
}

void CSegmentTool::OnPoint1 () { OnPoint (0); }
void CSegmentTool::OnPoint2 () { OnPoint (1); }
void CSegmentTool::OnPoint3 () { OnPoint (2); }
void CSegmentTool::OnPoint4 () { OnPoint (3); }

                        /*--------------------------*/

void CSegmentTool::SetDefTexture (INT16 nTexture)
{
CSegment *segP = theMine->Segments (0) + m_nSegment;
if (m_bSetDefTexture = ((CButton *) GetDlgItem (IDC_CUBE_SETDEFTEXTURE))->GetCheck ()) {
	INT32 i;
	for (i = 0; i < 6; i++)
		if (segP->children [i] == -1)
			theMine->SetTexture (m_nSegment, i, nTexture, 0);
	}
}

//------------------------------------------------------------------------
// CSegmentTool - RefreshData
//------------------------------------------------------------------------

void CSegmentTool::Refresh () 
{
if (!(m_bInited && theMine))
	return;
InitCBCubeNo ();
OnResetCoord ();

INT32 h, i, j;

// update automatic data
theMine->RenumberBotGens ();
theMine->RenumberEquipGens ();
// update cube number combo box if number of cubes has changed
CSegment *segP = theMine->CurrSeg ();
m_bEndOfExit = (segP->children [theMine->Current ()->nSide] == -2);
m_nSegment = theMine->Current ()->nSegment;
m_nSide = theMine->Current ()->nSide;
m_nPoint = theMine->Current ()->nPoint;
m_nType = segP->function;
m_nDamage [0] = segP->damage [0];
m_nDamage [1] = segP->damage [1];
m_nProps = segP->props;
m_nOwner = segP->owner;
m_nGroup = segP->group;
//CBType ()->SetCurSel (m_nType);
SelectItemData (CBType (), m_nType);
OnResetCoord ();
  // show Triggers () that point at this cube
LBTriggers()->ResetContent();
CTrigger *trigP = theMine->Triggers (0);
INT32 nTrigger;
for (nTrigger = 0; nTrigger < theMine->GameInfo ().triggers.count; nTrigger++, trigP++) {
	for (i = 0; i < trigP->m_count; i++) {
		if (trigP->m_targets [i] == CSideKey (m_nSegment, m_nSide)) {
			// find the wallP with this trigP
			CWall *wallP = theMine->Walls (0);
			INT32 nWall;
			for (nWall = 0; nWall < theMine->GameInfo ().walls.count ;nWall++, wallP++) {
				if (wallP->nTrigger == nTrigger) 
					break;
				}
			if (nWall < theMine->GameInfo ().walls.count) {
				sprintf_s (message, sizeof (message),  "%d,%d", (INT32) wallP->m_nSegment, (INT32) wallP->m_nSide + 1);
				INT32 h = LBTriggers ()->AddString (message);
				LBTriggers ()->SetItemData (h, (FIX) wallP->m_nSegment * 0x10000L + wallP->m_nSide);
				}
			}
		}
	}
// show if this is cube/side is trigPed by the control_center
CReactorTrigger* reactorTrigger = theMine->ReactorTriggers (0);
INT32 control;
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

m_nLight = ((double) segP->static_light) / (24 * 327.68);

CListBox *plb [2] = { LBAvailBots (), LBUsedBots () };
if (IsBotMaker (segP)) {
	INT32 nMatCen = segP->nMatCen;
	// if # of items in list box totals to less than the number of robots
	//    if (LBAvailBots ()->GetCount() + LBAvailBots ()->GetCount() < MAX_ROBOT_IDS) {
	HINSTANCE hInst = AfxGetInstanceHandle ();
	char		szObj [80];
	INT32		objFlags [2];
	for (i = 0; i < 2; i++)
		objFlags [i] = theMine->BotGens (nMatCen)->objFlags [i];
	if ((m_nLastCube != m_nSegment) || (m_nLastSide != m_nSide)) {
		for (i = 0; i < 2; i++) {
			plb [i]->ResetContent ();
			for (j = 0; j < 64; j++) {
				if (i) {
					h = ((objFlags [j / 32] & (1L << (j % 32))) != 0);
					if (!h)	//only add flagged objects to 2nd list box
						continue;
					}
				LoadString (hInst, ROBOT_STRING_TABLE + j, szObj, sizeof (szObj));
				h = plb [i]->AddString (szObj);
				plb [i]->SetItemData (h, j);
				}
			plb [i]->SetCurSel (0);
			}
		}
	}
else if (IsEquipMaker (segP)) {
	INT32 nMatCen = segP->nMatCen;
	// if # of items in list box totals to less than the number of robots
	//    if (LBAvailBots ()->GetCount() + LBAvailBots ()->GetCount() < MAX_ROBOT_IDS) {
	HINSTANCE hInst = AfxGetInstanceHandle ();
	char		szObj [80];
	INT32		objFlags [2];
	for (i = 0; i < 2; i++)
		objFlags [i] = theMine->EquipGens (nMatCen)->objFlags [i];
	if ((m_nLastCube != m_nSegment) || (m_nLastSide != m_nSide)) {
		for (i = 0; i < 2; i++) {
			plb [i]->ResetContent ();
			for (j = 0; j < MAX_POWERUP_IDS2; j++) {
				if (i) {
					h = ((objFlags [j / 32] & (1L << (j % 32))) != 0);
					if (!h)	//only add flagged objects to 2nd list box
						continue;
					}
				LoadString (hInst, POWERUP_STRING_TABLE + j, szObj, sizeof (szObj));
				if (!strcmp (szObj, "(not used)"))
					continue;
				h = plb [i]->AddString (szObj);
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
CSegment *segP = theMine->CurrSeg ();
theApp.SetModified (TRUE);
if (m_bEndOfExit = EndOfExit ()->GetCheck ()) {
	segP->children [m_nSide] = -2;
	segP->childFlags |= (1 << m_nSide);
	}
else {
	segP->children[m_nSide] = -1;
	segP->childFlags &= ~(1 << m_nSide);
	}
}

//------------------------------------------------------------------------
// CSegmentTool - Add Cube
//------------------------------------------------------------------------

void CSegmentTool::OnAddCube () 
{
theMine->AddSegment ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - Delete Cube
//------------------------------------------------------------------------

void CSegmentTool::OnDeleteCube () 
{
theMine->DeleteSegment (theMine->Current ()->nSegment);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CSegmentTool::OnSetOwner ()
{
	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = theMine->GotMarkedSegments ();


bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CSegment *segP = theMine->Segments (0);
	for (INT16 nSegNum = 0; nSegNum < theMine->SegCount (); nSegNum++, segP++)
		if (segP->wallFlags & MARKED_MASK)
			segP->owner = m_nOwner;
	}
else 					
	theMine->CurrSeg ()->owner = m_nOwner;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
}

//------------------------------------------------------------------------

void CSegmentTool::OnSetGroup ()
{
	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = theMine->GotMarkedSegments ();

bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CSegment *segP = theMine->Segments (0);
	for (INT16 nSegNum = 0; nSegNum < theMine->SegCount (); nSegNum++, segP++)
		if (segP->wallFlags & MARKED_MASK)
			segP->group = m_nGroup;
	}
else 					
	theMine->CurrSeg ()->group = m_nGroup;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
}

//------------------------------------------------------------------------
// CSegmentTool - SpecialMsg
//------------------------------------------------------------------------

void CSegmentTool::OnSetType ()
{
	BOOL			bChangeOk = TRUE;
	BOOL			bMarked = theMine->GotMarkedSegments ();
	INT32			nSegNum, nMinSeg, nMaxSeg;
	CSegment	*segP;

bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
m_nLastCube = -1; //force Refresh() to rebuild all dialog data
UINT8 nType = UINT8 (CBType ()->GetItemData (CBType ()->GetCurSel ()));
if (bMarked) {
	nMinSeg = 0;
	nMaxSeg = theMine->SegCount ();
	}
else {
	nMinSeg = INT32 (theMine->CurrSeg () - theMine->Segments (0));
	nMaxSeg = nMinSeg + 1;
	}
segP = theMine->Segments (nMinSeg);
for (nSegNum = nMinSeg; nSegNum < nMaxSeg; nSegNum++, segP++) {
	if (bMarked && !(segP->wallFlags & MARKED_MASK))
		continue;
	m_nType = segP->function;
	switch(nType) {
		// check to see if we are adding a robot maker
		case SEGMENT_FUNC_ROBOTMAKER:
			if (nType == m_nType)
				goto errorExit;
			if (!theMine->AddRobotMaker (nSegNum, false, m_bSetDefTexture == 1)) {
				theApp.ResetModified (bUndo);
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
			if (!theMine->AddFuelCenter (nSegNum, nType, false, (nType == SEGMENT_FUNC_FUELCEN) && (m_bSetDefTexture == 1))) {
				theApp.ResetModified (bUndo);
				goto funcExit;
				}
			break;

		case SEGMENT_FUNC_CONTROLCEN:
			if (nType == m_nType)
				continue;
			if (!theMine->AddReactor (nSegNum, false, m_bSetDefTexture == 1)) {
				theApp.ResetModified (bUndo);
				goto funcExit;
				}
			break;

		case SEGMENT_FUNC_GOAL_BLUE:
		case SEGMENT_FUNC_GOAL_RED:
			if (nType == m_nType)
				continue;
			if (!theMine->AddGoalCube (nSegNum, false, m_bSetDefTexture == 1, nType, -1))
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
			if (!theMine->AddTeamCube (nSegNum, false, false, nType, -1))
				goto errorExit;		
			break;

		case SEGMENT_FUNC_SPEEDBOOST:
			if (theMine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!theMine->AddSpeedBoostCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_SKYBOX:
			if (theMine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!theMine->AddSkyboxCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_EQUIPMAKER:
			if (theMine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!theMine->AddEquipMaker (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_NONE:
			theMine->UndefineSegment (nSegNum);
			break;

		default:
			break;
		}
#if 1
	m_nType = nType;
#else
	if (m_nType == SEGMENT_FUNC_ROBOTMAKER) {
		// remove matcen
		INT32 nMatCens = (INT32) theMine->GameInfo ().matcen.count;
		if (nMatCens > 0) {
			// fill in deleted matcen
			INT32 nDelMatCen = theMine->CurrSeg ()->value;
			memcpy (theMine->BotGens (nDelMatCen), theMine->BotGens (nDelMatCen + 1), (nMatCens - 1 - nDelMatCen) * sizeof (CRobotMaker));
			theMine->GameInfo ().matcen.count--;
			INT32 i;
			for (i = 0; i < 6; i++)
				theMine->DeleteTriggerTargets (m_nSegment, i);
			}
		}
	else if (m_nType == SEGMENT_FUNC_FUELCEN) { //remove all fuel cell Walls ()
		INT16 nSegNum = theMine->Current ()->nSegment;
		CSegment *childseg, *segP = theMine->CurrSeg ();
		CSide *oppside, *sideP = theMine->CurrSide ();
		CWall *wallP;
		INT16 nOppSeg, nOppSide;
		for (INT16 nSide = 0; nSide < 6; nSide++, sideP++) {
			if (segP->children [nSide] < 0)	// assume no wall if no child segment at the current side
				continue;
			childseg = theMine->Segments (0) + segP->children [nSide];
			if (childseg->function == SEGMENT_FUNC_FUELCEN)	// don't delete if child segment is fuel center
				continue;
			// if there is a wall and it's a fuel cell delete it
			if ((wall = theMine->GetWall (nSegNum, nSide)) && 
				 (wallP->type == WALL_ILLUSION) && (sideP->nBaseTex == (IsD1File ()) ? 322 : 333))
				theMine->DeleteWall (sideP->nWall);
			// if there is a wall at the opposite side and it's a fuel cell delete it
			if (theMine->GetOppositeSide (nOppSeg, nOppSide, nSegNum, nSide) &&
				 (wall = theMine->GetWall (nSegNum, nSide)) && (wallP->type == WALL_ILLUSION)) {
				oppside = theMine->Segments (nOppSeg)->sides + nOppSide;
				if (oppsideP->nBaseTex == (IsD1File ()) ? 322 : 333)
					theMine->DeleteWall (oppsideP->nWall);
				}
			}
		}
	// update "special"
	if (bChangeOk) {
		m_nType = nType;
		theMine->CurrSeg ()->function = nType;
		if (nType == SEGMENT_FUNC_NONE)
			theMine->CurrSeg ()->childFlags &= ~(1 << MAX_SIDES_PER_SEGMENT);
		else
			theMine->CurrSeg ()->childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
		}
#endif
	}

errorExit:

theApp.UnlockUndo ();
theMine->AutoLinkExitToReactor ();
theApp.SetModified (TRUE);

funcExit:

theApp.MineView ()->DelayRefresh (false);
UpdateData (TRUE);
}

//------------------------------------------------------------------------
// CSegmentTool - Cube Number Message
//------------------------------------------------------------------------

void CSegmentTool::OnSetCube () 
{
theMine->Current ()->nSegment = CBCubeNo ()->GetCurSel ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - LightMsg
//------------------------------------------------------------------------

void CSegmentTool::OnLight () 
{
UpdateData (TRUE);
theMine->CurrSeg ()->static_light = (FIX) (m_nLight * 24 * 327.68);
theApp.SetModified (TRUE);
}

                        /*--------------------------*/

void CSegmentTool::OnDamage (INT32 i) 
{
UpdateData (TRUE);
theMine->CurrSeg ()->damage [i] = m_nDamage [i];
theApp.SetModified (TRUE);
}

void CSegmentTool::OnDamage0 () { OnDamage (0); }
void CSegmentTool::OnDamage1 () { OnDamage (1); }

                        /*--------------------------*/

INT32 CSegmentTool::FindBot (CListBox *plb, LPSTR pszObj)
{
	INT32 i, j;

i = plb->GetCurSel ();
if (i < 0)
	return -1;
j = INT32 (plb->GetItemData (i));
if (pszObj)
	LoadString (AfxGetInstanceHandle(), ROBOT_STRING_TABLE + j, pszObj, 80);
return j;
}

                        /*--------------------------*/

INT32 CSegmentTool::FindEquip (CListBox *plb, LPSTR pszObj)
{
	INT32 i, j;

i = plb->GetCurSel ();
if (i < 0)
	return -1;
j = INT32 (plb->GetItemData (i));
if (pszObj)
	LoadString (AfxGetInstanceHandle(), POWERUP_STRING_TABLE + j, pszObj, 80);
return j;
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------

void CSegmentTool::AddBot ()
{
CSegment *segP = theMine->CurrSeg ();
INT32 matcen = segP->nMatCen;
char szObj [80];
INT32 i = FindBot (LBAvailBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
theMine->BotGens (matcen)->objFlags [i / 32] |= (1L << (i % 32));
INT32 h = LBAvailBots ()->GetCurSel ();
LBAvailBots ()->DeleteString (h);
LBAvailBots ()->SetCurSel (h);
h = LBUsedBots ()->AddString (szObj);
LBUsedBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------

void CSegmentTool::AddEquip ()
{
CSegment *segP = theMine->CurrSeg ();
INT32 matcen = segP->nMatCen;
char szObj [80];
INT32 i = FindEquip (LBAvailBots (), szObj);
if ((i < 0) || (i >= MAX_POWERUP_IDS2))
	return;
theMine->EquipGens (matcen)->objFlags [i / 32] |= (1L << (i % 32));
INT32 h = LBAvailBots ()->GetCurSel ();
LBAvailBots ()->DeleteString (h);
LBAvailBots ()->SetCurSel (h);
h = LBUsedBots ()->AddString (szObj);
LBUsedBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - AddMsg
//------------------------------------------------------------------------

void CSegmentTool::OnAddObj ()
{
CSegment *segP = theMine->CurrSeg ();
if (IsBotMaker (segP))
	AddBot ();
else if (IsEquipMaker (segP))
	AddEquip ();
}

//------------------------------------------------------------------------
// CSegmentTool - DelMsg
//------------------------------------------------------------------------

void CSegmentTool::DeleteBot () 
{
CSegment *segP = theMine->CurrSeg ();
INT32 matcen = segP->nMatCen;
char szObj [80];
INT32 i = FindBot (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
theMine->BotGens (matcen)->objFlags [i / 32] &= ~(1L << (i % 32));
INT32 h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - DelMsg
//------------------------------------------------------------------------

void CSegmentTool::DeleteEquip () 
{
CSegment *segP = theMine->CurrSeg ();
INT32 matcen = segP->nMatCen;
char szObj [80];
INT32 i = FindEquip (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
theMine->EquipGens (matcen)->objFlags [i / 32] &= ~(1L << (i % 32));
INT32 h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - DelMsg
//------------------------------------------------------------------------

void CSegmentTool::OnDeleteObj () 
{
CSegment *segP = theMine->CurrSeg ();
if (IsBotMaker (segP))
	DeleteBot ();
else if (IsEquipMaker (segP))
	DeleteEquip ();
}

//------------------------------------------------------------------------
// CSegmentTool - OtherCubeMsg
//------------------------------------------------------------------------

void CSegmentTool::OnOtherCube () 
{
theApp.MineView ()->SelectOtherCube ();
}

//------------------------------------------------------------------------
// CSegmentTool - CubeButtonMsg
//------------------------------------------------------------------------

void CSegmentTool::OnWallDetails () 
{
if (!LBTriggers ()->GetCount ())
	return;
INT32 i = LBTriggers ()->GetCurSel ();
if (i < 0)
	return;
long h = long (LBTriggers ()->GetItemData (i));
theMine->Current ()->nSegment = (INT16) (h / 0x10000L);
theMine->Current ()->nSide = (INT16) (h % 0x10000L);
theApp.ToolView ()->EditWall ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CSegmentTool - TriggerButtonMsg
//------------------------------------------------------------------------

void CSegmentTool::OnTriggerDetails ()
{
if (!theMine) return;
if (!LBTriggers ()->GetCount ())
	return;
INT32 i = LBTriggers ()->GetCurSel ();
if ((i < 0) || (i >= LBTriggers ()->GetCount ()))
	return;
long h = long (LBTriggers ()->GetItemData (i));
theMine->Other ()->nSegment = theMine->Current ()->nSegment;
theMine->Other ()->nSide = theMine->Current ()->nSide;
theMine->Current ()->nSegment = (INT16) (h / 0x10000L);
theMine->Current ()->nSide = (INT16) (h % 0x10000L);
theApp.ToolView ()->EditTrigger ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CSegmentTool::OnAddBotGen ()
{
theMine->AddRobotMaker ();
m_nLastCube = -1;
Refresh ();
}

                        /*--------------------------*/

void CSegmentTool::OnAddEquipGen ()
{
theMine->AddEquipMaker ();
m_nLastCube = -1;
Refresh ();
}

                        /*--------------------------*/

void CSegmentTool::OnAddFuelCen ()
{
theMine->AddFuelCenter ();
}

                        /*--------------------------*/

void CSegmentTool::OnAddRepairCen ()
{
theMine->AddFuelCenter (-1, SEGMENT_FUNC_REPAIRCEN);
}

                        /*--------------------------*/

void CSegmentTool::OnAddControlCen ()
{
theMine->AddReactor ();
}

                        /*--------------------------*/

void CSegmentTool::OnSplitCube ()
{
theMine->SplitSegment ();
}

                        /*--------------------------*/

//eof cubedlg.cpp