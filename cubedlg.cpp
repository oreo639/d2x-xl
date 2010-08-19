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

BEGIN_MESSAGE_MAP (CCubeTool, CToolDlg)
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

CCubeTool::CCubeTool (CPropertySheet *pParent)
	: CToolDlg (nLayout ? IDD_CUBEDATA2 : IDD_CUBEDATA, pParent)
{
Reset ();
}

                        /*--------------------------*/

void CCubeTool::Reset ()
{
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
if (GetMine ()) {
	m_nOwner = m_mine->Segments ()->owner;
	m_nGroup = m_mine->Segments ()->group;
	}
else {
	m_nOwner = -1;
	m_nGroup = -1;
	}
memset (m_nCoord, 0, sizeof (m_nCoord));
}

                        /*--------------------------*/

void CCubeTool::InitCBCubeNo ()
{
if (!GetMine ())
	return;
CComboBox *pcb = CBCubeNo ();
if (m_mine->SegCount () != pcb->GetCount ()) {
	pcb->ResetContent ();
	for (INT32 i = 0; i < m_mine->SegCount (); i++) {
		_itoa_s (i, message, sizeof (message), 10);
		pcb->AddString (message);
		}
	}
pcb->SetCurSel (m_nSegment);
}

                        /*--------------------------*/

BOOL CCubeTool::OnInitDialog ()
{
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

CToolDlg::OnInitDialog ();
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

void CCubeTool::DoDataExchange (CDataExchange *pDX)
{
if (!GetMine ())
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
//	DDV_MinMaxInt (pDX, (long) m_nCoord [i], -0x7fff, 0x7fff);
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

BOOL CCubeTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

                        /*--------------------------*/

bool CCubeTool::IsBotMaker (CDSegment *seg)
{
return 
	(seg->function == SEGMENT_FUNC_ROBOTMAKER) &&
	(seg->matcen_num >= 0) &&
	(seg->matcen_num < m_mine->GameInfo ().botgen.count);
}

                        /*--------------------------*/

bool CCubeTool::IsEquipMaker (CDSegment *seg)
{
return 
	(seg->function == SEGMENT_FUNC_EQUIPMAKER) &&
	(seg->matcen_num >= 0) &&
	(seg->matcen_num < m_mine->GameInfo ().equipgen.count);
}

                        /*--------------------------*/

void CCubeTool::EnableControls (BOOL bEnable)
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
// enable/disable "end of exit tunnel" button
EndOfExit ()->EnableWindow (seg->children [m_nSide] < 0);
// enable/disable add cube button
GetDlgItem (IDC_CUBE_ADD)->EnableWindow ((m_mine->SegCount () < MAX_SEGMENTS (m_mine)) &&
													  (m_mine->VertCount () < MAX_VERTICES (m_mine) - 4) &&
													  (seg->children [m_nSide] < 0));
GetDlgItem (IDC_CUBE_DEL)->EnableWindow (m_mine->SegCount () > 1);
// enable/disable add robot button
GetDlgItem (IDC_CUBE_ADDBOT)->EnableWindow ((IsBotMaker (seg) || IsEquipMaker (seg)) && (LBAvailBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_DELBOT)->EnableWindow ((IsBotMaker (seg) || IsEquipMaker (seg)) && (LBUsedBots ()->GetCount () > 0));
GetDlgItem (IDC_CUBE_WALLDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_TRIGGERDETAILS)->EnableWindow (LBTriggers ()->GetCount () > 0);
GetDlgItem (IDC_CUBE_ADD_REPAIRCEN)->EnableWindow (m_mine->IsD2File ());
GetDlgItem (IDC_CUBE_OWNER)->EnableWindow (m_mine->IsD2XLevel ());
GetDlgItem (IDC_CUBE_GROUP)->EnableWindow (m_mine->IsD2XLevel ());
}

                        /*--------------------------*/

void CCubeTool::OnSetCoord ()
{
if (!GetMine ())
	return;
UpdateData (TRUE);
theApp.SetModified (TRUE);
m_nVertex = m_mine->CurrSeg ()->verts[side_vert[m_mine->Current ()->nSide][m_mine->Current ()->nPoint]];
m_mine->Vertices (m_nVertex)->x = (FIX) (m_nCoord [0] * 0x10000L);
m_mine->Vertices (m_nVertex)->y = (FIX) (m_nCoord [1] * 0x10000L);
m_mine->Vertices (m_nVertex)->z = (FIX) (m_nCoord [2] * 0x10000L);
theApp.MineView ()->Refresh (false);
}

                        /*--------------------------*/

void CCubeTool::OnResetCoord ()
{
if (!GetMine ())
	return;
m_nVertex = m_mine->CurrSeg ()->verts[side_vert[m_mine->Current ()->nSide][m_mine->Current ()->nPoint]];
m_nCoord [0] = (double) m_mine->Vertices (m_nVertex)->x / 0x10000L;
m_nCoord [1] = (double) m_mine->Vertices (m_nVertex)->y / 0x10000L;
m_nCoord [2] = (double) m_mine->Vertices (m_nVertex)->z / 0x10000L;
UpdateData (FALSE);
theApp.MineView ()->Refresh (false);
}

                        /*--------------------------*/

void CCubeTool::OnProp (INT32 nProp)
{
if (!GetMine ())
	return;
theApp.SetModified (TRUE);
if (Prop (nProp)->GetCheck ())
	m_nProps |= 1 << nProp;
else
	m_nProps &= ~(1 << nProp);
m_mine->CurrSeg ()->props = m_nProps;
}

void CCubeTool::OnProp1 () { OnProp (0); }
void CCubeTool::OnProp2 () { OnProp (1); }
void CCubeTool::OnProp3 () { OnProp (2); }
void CCubeTool::OnProp4 () { OnProp (3); }
void CCubeTool::OnProp5 () { OnProp (4); }

                        /*--------------------------*/

void CCubeTool::OnSide (INT32 nSide)
{
if (!GetMine ())
	return;
m_mine->Current ()->nSide = m_nSide = nSide;
theApp.MineView ()->Refresh ();
}

void CCubeTool::OnSide1 () { OnSide (0); }
void CCubeTool::OnSide2 () { OnSide (1); }
void CCubeTool::OnSide3 () { OnSide (2); }
void CCubeTool::OnSide4 () { OnSide (3); }
void CCubeTool::OnSide5 () { OnSide (4); }
void CCubeTool::OnSide6 () { OnSide (5); }

                        /*--------------------------*/

void CCubeTool::OnPoint (INT32 nPoint)
{
if (!GetMine ())
	return;
m_mine->Current ()->nPoint = m_nPoint = nPoint;
theApp.MineView ()->Refresh ();
}

void CCubeTool::OnPoint1 () { OnPoint (0); }
void CCubeTool::OnPoint2 () { OnPoint (1); }
void CCubeTool::OnPoint3 () { OnPoint (2); }
void CCubeTool::OnPoint4 () { OnPoint (3); }

                        /*--------------------------*/

void CCubeTool::SetDefTexture (INT16 nTexture)
{
CDSegment *seg = m_mine->Segments () + m_nSegment;
if (m_bSetDefTexture = ((CButton *) GetDlgItem (IDC_CUBE_SETDEFTEXTURE))->GetCheck ()) {
	INT32 i;
	for (i = 0; i < 6; i++)
		if (seg->children [i] == -1)
			m_mine->SetTexture (m_nSegment, i, nTexture, 0);
	}
}

//------------------------------------------------------------------------
// CCubeTool - RefreshData
//------------------------------------------------------------------------

void CCubeTool::Refresh () 
{
if (!m_bInited)
	return;
if (!GetMine ())
	return;
InitCBCubeNo ();
OnResetCoord ();

INT32 h, i, j;

// update automatic data
m_mine->RenumberBotGens ();
m_mine->RenumberEquipGens ();
// update cube number combo box if number of cubes has changed
CDSegment *seg = m_mine->CurrSeg ();
m_bEndOfExit = (seg->children [m_mine->Current ()->nSide] == -2);
m_nSegment = m_mine->Current ()->nSegment;
m_nSide = m_mine->Current ()->nSide;
m_nPoint = m_mine->Current ()->nPoint;
m_nType = seg->function;
m_nDamage [0] = seg->damage [0];
m_nDamage [1] = seg->damage [1];
m_nProps = seg->props;
m_nOwner = seg->owner;
m_nGroup = seg->group;
//CBType ()->SetCurSel (m_nType);
SelectItemData (CBType (), m_nType);
OnResetCoord ();
  // show Triggers () that point at this cube
LBTriggers()->ResetContent();
CTrigger *trigger = m_mine->Triggers ();
INT32 trignum;
for (trignum = 0; trignum < m_mine->GameInfo ().triggers.count; trignum++, trigger++) {
	for (i = 0; i < trigger->count; i++) {
		if ((trigger [i] == CSideKey (m_nSegment, m_nSide)) {
			// find the wall with this trigger
			CWall *wall = m_mine->Walls ();
			INT32 wallnum;
			for (wallnum = 0; wallnum < m_mine->GameInfo ().walls.count ;wallnum++, wall++) {
				if (wall->trigger == trignum) 
					break;
				}
			if (wallnum < m_mine->GameInfo ().walls.count) {
				sprintf_s (message, sizeof (message),  "%d,%d", (INT32) wall->nSegment, (INT32) wall->nSide + 1);
				INT32 h = LBTriggers ()->AddString (message);
				LBTriggers ()->SetItemData (h, (long) wall->nSegment * 0x10000L + wall->nSide);
				}
			}
		}
	}
// show if this is cube/side is triggered by the control_center
CReactorTrigger* reactorTrigger = m_mine->ReactorTriggers ();
INT32 control;
for (control = 0; control < MAX_CONTROL_CENTER_TRIGGERS; control++, reactorTrigger++) {
	INT32 count = reactorTrigger->count;
	for (i = 0; i < count; i++) {
		if (reactorTrigger [i] == CSideKey (m_nSegment, m_nSide)) {
			LBTriggers ()->AddString ("Reactor");
			break;
			}
		}
	if (i < count) 
		break; // quit if for loop broke above
	}

// show "none" if there is no Triggers ()
if (!LBTriggers()->GetCount()) {
	LBTriggers()->AddString ("none");
	}

m_nLight = ((double) seg->static_light) / (24 * 327.68);

CListBox *plb [2] = { LBAvailBots (), LBUsedBots () };
if (IsBotMaker (seg)) {
	INT32 nMatCen = seg->matcen_num;
	// if # of items in list box totals to less than the number of robots
	//    if (LBAvailBots ()->GetCount() + LBAvailBots ()->GetCount() < MAX_ROBOT_IDS) {
	HINSTANCE hInst = AfxGetInstanceHandle ();
	char		szObj [80];
	INT32		objFlags [2];
	for (i = 0; i < 2; i++)
		objFlags [i] = m_mine->BotGens (nMatCen)->objFlags [i];
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
else if (IsEquipMaker (seg)) {
	INT32 nMatCen = seg->matcen_num;
	// if # of items in list box totals to less than the number of robots
	//    if (LBAvailBots ()->GetCount() + LBAvailBots ()->GetCount() < MAX_ROBOT_IDS) {
	HINSTANCE hInst = AfxGetInstanceHandle ();
	char		szObj [80];
	INT32		objFlags [2];
	for (i = 0; i < 2; i++)
		objFlags [i] = m_mine->EquipGens (nMatCen)->objFlags [i];
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
// CCubeTool - EndOfExitTunnel ()
//------------------------------------------------------------------------

void CCubeTool::OnEndOfExit ()
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
theApp.SetModified (TRUE);
if (m_bEndOfExit = EndOfExit ()->GetCheck ()) {
	seg->children [m_nSide] = -2;
	seg->child_bitmask |= (1 << m_nSide);
	}
else {
	seg->children[m_nSide] = -1;
	seg->child_bitmask &= ~(1 << m_nSide);
	}
}

//------------------------------------------------------------------------
// CCubeTool - Add Cube
//------------------------------------------------------------------------

void CCubeTool::OnAddCube () 
{
if (!GetMine ())
	return;
m_mine->AddSegment ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - Delete Cube
//------------------------------------------------------------------------

void CCubeTool::OnDeleteCube () 
{
if (!GetMine ())
	return;
m_mine->DeleteSegment (m_mine->Current ()->nSegment);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------

void CCubeTool::OnSetOwner ()
{
if (!GetMine ())
	return;

	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = m_mine->GotMarkedSegments ();


bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CDSegment *seg = m_mine->Segments ();
	for (INT16 nSegNum = 0; nSegNum < m_mine->SegCount (); nSegNum++, seg++)
		if (seg->wall_bitmask & MARKED_MASK)
			seg->owner = m_nOwner;
	}
else 					
	m_mine->CurrSeg ()->owner = m_nOwner;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
}

//------------------------------------------------------------------------

void CCubeTool::OnSetGroup ()
{
if (!GetMine ())
	return;

	BOOL	bChangeOk = TRUE;
	BOOL	bMarked = m_mine->GotMarkedSegments ();

bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
UpdateData (TRUE);
if (bMarked) {
	CDSegment *seg = m_mine->Segments ();
	for (INT16 nSegNum = 0; nSegNum < m_mine->SegCount (); nSegNum++, seg++)
		if (seg->wall_bitmask & MARKED_MASK)
			seg->group = m_nGroup;
	}
else 					
	m_mine->CurrSeg ()->group = m_nGroup;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
}

//------------------------------------------------------------------------
// CCubeTool - SpecialMsg
//------------------------------------------------------------------------

void CCubeTool::OnSetType ()
{
if (!GetMine ())
	return;

	BOOL			bChangeOk = TRUE;
	BOOL			bMarked = m_mine->GotMarkedSegments ();
	INT32			nSegNum, nMinSeg, nMaxSeg;
	CDSegment	*segP;

bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
theApp.MineView ()->DelayRefresh (true);
m_nLastCube = -1; //force Refresh() to rebuild all dialog data
UINT8 nType = UINT8 (CBType ()->GetItemData (CBType ()->GetCurSel ()));
if (bMarked) {
	nMinSeg = 0;
	nMaxSeg = m_mine->SegCount ();
	}
else {
	nMinSeg = INT32 (m_mine->CurrSeg () - m_mine->Segments ());
	nMaxSeg = nMinSeg + 1;
	}
segP = m_mine->Segments (nMinSeg);
for (nSegNum = nMinSeg; nSegNum < nMaxSeg; nSegNum++, segP++) {
	if (bMarked && !(segP->wall_bitmask & MARKED_MASK))
		continue;
	m_nType = segP->function;
	switch(nType) {
		// check to see if we are adding a robot maker
		case SEGMENT_FUNC_ROBOTMAKER:
			if (nType == m_nType)
				goto errorExit;
			if (!m_mine->AddRobotMaker (nSegNum, false, m_bSetDefTexture == 1)) {
				theApp.ResetModified (bUndo);
				goto funcExit;
				}
			break;

		// check to see if we are adding a fuel center
		case SEGMENT_FUNC_REPAIRCEN:
			if (m_mine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}

		case SEGMENT_FUNC_FUELCEN:
			if (nType == m_nType)
				continue;
			if (!m_mine->AddFuelCenter (nSegNum, nType, false, (nType == SEGMENT_FUNC_FUELCEN) && (m_bSetDefTexture == 1))) {
				theApp.ResetModified (bUndo);
				goto funcExit;
				}
			break;

		case SEGMENT_FUNC_CONTROLCEN:
			if (nType == m_nType)
				continue;
			if (!m_mine->AddReactor (nSegNum, false, m_bSetDefTexture == 1)) {
				theApp.ResetModified (bUndo);
				goto funcExit;
				}
			break;

		case SEGMENT_FUNC_GOAL_BLUE:
		case SEGMENT_FUNC_GOAL_RED:
			if (nType == m_nType)
				continue;
			if (!m_mine->AddGoalCube (nSegNum, false, m_bSetDefTexture == 1, nType, -1))
				goto errorExit;		
			break;

		case SEGMENT_FUNC_TEAM_BLUE:
		case SEGMENT_FUNC_TEAM_RED:
			if (m_mine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (nType == m_nType)
				continue;
			if (!m_mine->AddTeamCube (nSegNum, false, false, nType, -1))
				goto errorExit;		
			break;

		case SEGMENT_FUNC_SPEEDBOOST:
			if (m_mine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddSpeedBoostCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_SKYBOX:
			if (m_mine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddSkyboxCube (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_EQUIPMAKER:
			if (m_mine->IsStdLevel ()) {
				m_nType = nType;
				if (!bExpertMode)
					ErrorMsg ("Convert the level to a D2X-XL level to use this segment type.");
				break;
				}
			if (!m_mine->AddEquipMaker (nSegNum, false))
				goto errorExit;
			break;

		case SEGMENT_FUNC_NONE:
			m_mine->UndefineSegment (nSegNum);
			break;

		default:
			break;
		}
#if 1
	m_nType = nType;
#else
	if (m_nType == SEGMENT_FUNC_ROBOTMAKER) {
		// remove matcen
		INT32 nMatCens = (INT32) m_mine->GameInfo ().matcen.count;
		if (nMatCens > 0) {
			// fill in deleted matcen
			INT32 nDelMatCen = m_mine->CurrSeg ()->value;
			memcpy (m_mine->BotGens (nDelMatCen), m_mine->BotGens (nDelMatCen + 1), (nMatCens - 1 - nDelMatCen) * sizeof (CRobotMaker));
			m_mine->GameInfo ().matcen.count--;
			INT32 i;
			for (i = 0; i < 6; i++)
				m_mine->DeleteTriggerTargets (m_nSegment, i);
			}
		}
	else if (m_nType == SEGMENT_FUNC_FUELCEN) { //remove all fuel cell Walls ()
		INT16 nSegNum = m_mine->Current ()->nSegment;
		CDSegment *childseg, *seg = m_mine->CurrSeg ();
		CDSide *oppside, *side = m_mine->CurrSide ();
		CWall *wall;
		INT16 opp_segnum, opp_sidenum;
		for (INT16 sidenum = 0; sidenum < 6; sidenum++, side++) {
			if (seg->children [sidenum] < 0)	// assume no wall if no child segment at the current side
				continue;
			childseg = m_mine->Segments () + seg->children [sidenum];
			if (childseg->function == SEGMENT_FUNC_FUELCEN)	// don't delete if child segment is fuel center
				continue;
			// if there is a wall and it's a fuel cell delete it
			if ((wall = m_mine->GetWall (nSegNum, sidenum)) && 
				 (wall->type == WALL_ILLUSION) && (side->nBaseTex == (IsD1File ()) ? 322 : 333))
				m_mine->DeleteWall (side->nWall);
			// if there is a wall at the opposite side and it's a fuel cell delete it
			if (m_mine->GetOppositeSide (opp_segnum, opp_sidenum, nSegNum, sidenum) &&
				 (wall = m_mine->GetWall (nSegNum, sidenum)) && (wall->type == WALL_ILLUSION)) {
				oppside = m_mine->Segments (opp_segnum)->sides + opp_sidenum;
				if (oppside->nBaseTex == (IsD1File ()) ? 322 : 333)
					m_mine->DeleteWall (oppside->nWall);
				}
			}
		}
	// update "special"
	if (bChangeOk) {
		m_nType = nType;
		m_mine->CurrSeg ()->function = nType;
		if (nType == SEGMENT_FUNC_NONE)
			m_mine->CurrSeg ()->child_bitmask &= ~(1 << MAX_SIDES_PER_SEGMENT);
		else
			m_mine->CurrSeg ()->child_bitmask |= (1 << MAX_SIDES_PER_SEGMENT);
		}
#endif
	}

errorExit:

theApp.UnlockUndo ();
m_mine->AutoLinkExitToReactor ();
theApp.SetModified (TRUE);

funcExit:

theApp.MineView ()->DelayRefresh (false);
UpdateData (TRUE);
}

//------------------------------------------------------------------------
// CCubeTool - Cube Number Message
//------------------------------------------------------------------------

void CCubeTool::OnSetCube () 
{
if (!GetMine ())
	return;
m_mine->Current ()->nSegment = CBCubeNo ()->GetCurSel ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - LightMsg
//------------------------------------------------------------------------

void CCubeTool::OnLight () 
{
if (!GetMine ())
	return;
UpdateData (TRUE);
m_mine->CurrSeg ()->static_light = (FIX) (m_nLight * 24 * 327.68);
theApp.SetModified (TRUE);
}

                        /*--------------------------*/

void CCubeTool::OnDamage (INT32 i) 
{
if (!GetMine ())
	return;
UpdateData (TRUE);
m_mine->CurrSeg ()->damage [i] = m_nDamage [i];
theApp.SetModified (TRUE);
}

void CCubeTool::OnDamage0 () { OnDamage (0); }
void CCubeTool::OnDamage1 () { OnDamage (1); }

                        /*--------------------------*/

INT32 CCubeTool::FindBot (CListBox *plb, LPSTR pszObj)
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

INT32 CCubeTool::FindEquip (CListBox *plb, LPSTR pszObj)
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

void CCubeTool::AddBot ()
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
INT32 matcen = seg->matcen_num;
char szObj [80];
INT32 i = FindBot (LBAvailBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
m_mine->BotGens (matcen)->objFlags [i / 32] |= (1L << (i % 32));
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

void CCubeTool::AddEquip ()
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
INT32 matcen = seg->matcen_num;
char szObj [80];
INT32 i = FindEquip (LBAvailBots (), szObj);
if ((i < 0) || (i >= MAX_POWERUP_IDS2))
	return;
m_mine->EquipGens (matcen)->objFlags [i / 32] |= (1L << (i % 32));
INT32 h = LBAvailBots ()->GetCurSel ();
LBAvailBots ()->DeleteString (h);
LBAvailBots ()->SetCurSel (h);
h = LBUsedBots ()->AddString (szObj);
LBUsedBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - AddMsg
//------------------------------------------------------------------------

void CCubeTool::OnAddObj ()
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
if (IsBotMaker (seg))
	AddBot ();
else if (IsEquipMaker (seg))
	AddEquip ();
}

//------------------------------------------------------------------------
// CCubeTool - DelMsg
//------------------------------------------------------------------------

void CCubeTool::DeleteBot () 
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
INT32 matcen = seg->matcen_num;
char szObj [80];
INT32 i = FindBot (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
m_mine->BotGens (matcen)->objFlags [i / 32] &= ~(1L << (i % 32));
INT32 h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - DelMsg
//------------------------------------------------------------------------

void CCubeTool::DeleteEquip () 
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
INT32 matcen = seg->matcen_num;
char szObj [80];
INT32 i = FindEquip (LBUsedBots (), szObj);
if ((i < 0) || (i >= 64))
	return;
m_mine->EquipGens (matcen)->objFlags [i / 32] &= ~(1L << (i % 32));
INT32 h = LBUsedBots ()->GetCurSel ();
LBUsedBots ()->DeleteString (h);
LBUsedBots ()->SetCurSel (h);
h = LBAvailBots ()->AddString (szObj);
LBAvailBots ()->SetItemData (h, i);
theApp.SetModified (TRUE);
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - DelMsg
//------------------------------------------------------------------------

void CCubeTool::OnDeleteObj () 
{
if (!GetMine ())
	return;
CDSegment *seg = m_mine->CurrSeg ();
if (IsBotMaker (seg))
	DeleteBot ();
else if (IsEquipMaker (seg))
	DeleteEquip ();
}

//------------------------------------------------------------------------
// CCubeTool - OtherCubeMsg
//------------------------------------------------------------------------

void CCubeTool::OnOtherCube () 
{
theApp.MineView ()->SelectOtherCube ();
}

//------------------------------------------------------------------------
// CCubeTool - CubeButtonMsg
//------------------------------------------------------------------------

void CCubeTool::OnWallDetails () 
{
if (!GetMine ())
	return;
if (!LBTriggers ()->GetCount ())
	return;
INT32 i = LBTriggers ()->GetCurSel ();
if (i < 0)
	return;
long h = long (LBTriggers ()->GetItemData (i));
m_mine->Current ()->nSegment = (INT16) (h / 0x10000L);
m_mine->Current ()->nSide = (INT16) (h % 0x10000L);
theApp.ToolView ()->EditWall ();
theApp.MineView ()->Refresh ();
}

//------------------------------------------------------------------------
// CCubeTool - TriggerButtonMsg
//------------------------------------------------------------------------

void CCubeTool::OnTriggerDetails ()
{
if (!GetMine ())
	return;
if (!LBTriggers ()->GetCount ())
	return;
INT32 i = LBTriggers ()->GetCurSel ();
if ((i < 0) || (i >= LBTriggers ()->GetCount ()))
	return;
long h = long (LBTriggers ()->GetItemData (i));
m_mine->Other ()->segment = m_mine->Current ()->nSegment;
m_mine->Other ()->side = m_mine->Current ()->nSide;
m_mine->Current ()->nSegment = (INT16) (h / 0x10000L);
m_mine->Current ()->nSide = (INT16) (h % 0x10000L);
theApp.ToolView ()->EditTrigger ();
theApp.MineView ()->Refresh ();
}

                        /*--------------------------*/

void CCubeTool::OnAddBotGen ()
{
if (!GetMine ())
	return;
m_mine->AddRobotMaker ();
m_nLastCube = -1;
Refresh ();
}

                        /*--------------------------*/

void CCubeTool::OnAddEquipGen ()
{
if (!GetMine ())
	return;
m_mine->AddEquipMaker ();
m_nLastCube = -1;
Refresh ();
}

                        /*--------------------------*/

void CCubeTool::OnAddFuelCen ()
{
if (!GetMine ())
	return;
m_mine->AddFuelCenter ();
}

                        /*--------------------------*/

void CCubeTool::OnAddRepairCen ()
{
if (!GetMine ())
	return;
m_mine->AddFuelCenter (-1, SEGMENT_FUNC_REPAIRCEN);
}

                        /*--------------------------*/

void CCubeTool::OnAddControlCen ()
{
if (!GetMine ())
	return;
m_mine->AddReactor ();
}

                        /*--------------------------*/

void CCubeTool::OnSplitCube ()
{
if (!GetMine ())
	return;
m_mine->SplitSegment ();
}

                        /*--------------------------*/

//eof cubedlg.cpp