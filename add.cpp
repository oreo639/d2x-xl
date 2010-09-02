// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "texturemanager.h"

//==========================================================================
// SetDefaultTexture
//==========================================================================

bool CMine::SetDefaultTexture (short nTexture, short walltype)
{
if (nTexture < 0)
	return true;
short nSegment = Current ()->nSegment;
short nOppSeg, nOppSide;
CSegment *segP = Segments (nSegment);
CSide *sideP = segP->m_sides;
double scale = textureManager.Textures (m_fileType, nTexture)->Scale (nTexture);
segP->m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
// set textures
for (short nSide = 0; nSide < 6; nSide++, sideP++) {
	if (segP->GetChild (nSide) == -1) {
		SetTexture (nSegment, nSide, nTexture, 0);
	/*
		sideP->m_info.uvls [0].l = 0x8000;
		sideP->m_info.uvls [1].l = 0x8000;
		sideP->m_info.uvls [2].l = 0x8000;
		sideP->m_info.uvls [3].l = 0x8000;
	*/
		int i;
		for (i = 0; i < 4; i++) {
			sideP->m_info.uvls [i].u = (short) ((double) defaultUVLs [i].u / scale);
			sideP->m_info.uvls [i].v = (short) ((double) defaultUVLs [i].v / scale);
			sideP->m_info.uvls [i].l = defaultUVLs [i].l;
			}
		Segments (nSegment)->SetUV (nSide, 0, 0);
		}
	else if (nTexture >= 0) {
		if (walltype >= 0) {
			if ((MineInfo ().walls.count < MAX_WALLS) &&
				 (Segments (nSegment)->m_sides [nSide].m_info.nWall >= MineInfo ().walls.count))
				AddWall (nSegment, nSide, (byte) walltype, 0, KEY_NONE, -1, -1); // illusion
			else
				return false;
			if ((MineInfo ().walls.count < MAX_WALLS) &&
				 GetOppositeSide (nOppSeg, nOppSide, nSegment, nSide) &&
				 (Segments (nOppSeg)->m_sides [nOppSide].m_info.nWall >= MineInfo ().walls.count))
				AddWall (nOppSeg, nOppSide, (byte) walltype, 0, KEY_NONE, -1, -1); // illusion
			else
				return false;
			}
		}
	}
return true;
}

//==========================================================================
// UndefineSegment
//==========================================================================

void CMine::UndefineSegment (short nSegment)
{
	CSegment *segP = (nSegment < 0) ? current.Segment () : Segments (nSegment);

nSegment = short (segP - Segments (0));
if (segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) {
	// remove matcen
	int nMatCens = (int) MineInfo ().botgen.count;
	if (nMatCens > 0) {
		// fill in deleted matcen
		int nDelMatCen = segP->m_info.nMatCen;
		if ((nDelMatCen >= 0) && (nDelMatCen < --nMatCens)) {
			memcpy (BotGens (nDelMatCen), BotGens (nMatCens), sizeof (CRobotMaker));
			BotGens (nDelMatCen)->m_info.nFuelCen = nDelMatCen;
			segP->m_info.nMatCen = -1;
			}
		MineInfo ().botgen.count--;
		int i;
		for (i = 0; i < 6; i++)
			DeleteTriggerTargets (nSegment, i);
		CSegment *s;
		for (i = SegCount (), s = Segments (0); i; i--, s++)
			if ((segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) && (s->m_info.nMatCen == nMatCens)) {
				s->m_info.nMatCen = nDelMatCen;
				break;
				}
		}
	segP->m_info.nMatCen = -1;
	}
if (segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) {
	// remove matcen
	int nMatCens = (int) MineInfo ().equipgen.count;
	if (nMatCens > 0) {
		// fill in deleted matcen
		int nDelMatCen = segP->m_info.nMatCen;
		if ((nDelMatCen >= 0) && (nDelMatCen < --nMatCens)) {
			memcpy (EquipGens (nDelMatCen), EquipGens (nMatCens), sizeof (CRobotMaker));
			EquipGens (nDelMatCen)->m_info.nFuelCen = nDelMatCen;
			segP->m_info.nMatCen = -1;
			}
		MineInfo ().equipgen.count--;
		int i;
		for (i = 0; i < 6; i++)
			DeleteTriggerTargets (nSegment, i);
		CSegment *s;
		nDelMatCen += (int) MineInfo ().botgen.count;
		for (i = SegCount (), s = Segments (0); i; i--, s++)
			if ((s->m_info.function == SEGMENT_FUNC_EQUIPMAKER) && (s->m_info.nMatCen == nMatCens)) {
				s->m_info.nMatCen = nDelMatCen;
				break;
			}
		}
	segP->m_info.nMatCen = -1;
	}
else if (segP->m_info.function == SEGMENT_FUNC_FUELCEN) { //remove all fuel cell walls
	CSegment *childSegP;
	CSide *oppSideP, *sideP = segP->m_sides;
	CWall *wallP;
	short nOppSeg, nOppSide;
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		if (segP->GetChild (nSide) < 0)	// assume no wall if no child segment at the current side
			continue;
		childSegP = Segments (segP->GetChild (nSide));
		if (childSegP->m_info.function == SEGMENT_FUNC_FUELCEN)	// don't delete if child segment is fuel center
			continue;
		// if there is a wall and it's a fuel cell delete it
		if ((wallP = GetWall (nSegment, nSide)) && 
			 (wallP->m_info.type == WALL_ILLUSION) && (sideP->m_info.nBaseTex == (IsD1File () ? 322 : 333)))
			DeleteWall (sideP->m_info.nWall);
		// if there is a wall at the opposite side and it's a fuel cell delete it
		if (GetOppositeSide (nOppSeg, nOppSide, nSegment, nSide) &&
			 (wallP = GetWall (nSegment, nSide)) && (wallP->m_info.type == WALL_ILLUSION)) {
			oppSideP = Segments (nOppSeg)->m_sides + nOppSide;
			if (oppSideP->m_info.nBaseTex == (IsD1File () ? 322 : 333))
				DeleteWall (oppSideP->m_info.nWall);
			}
		}
	}
segP->m_info.childFlags &= ~(1 << MAX_SIDES_PER_SEGMENT);
segP->m_info.function = SEGMENT_FUNC_NONE;
}

//==========================================================================
// DefineSegment
//==========================================================================

bool CMine::DefineSegment (short nSegment, byte type, short nTexture, short walltype)
{
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
UndefineSegment (nSegment);
CSegment *segP = (nSegment < 0) ? current.Segment () : Segments (nSegment);
segP->m_info.function = type;
segP->m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
SetDefaultTexture (nTexture, walltype);
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

//==========================================================================
// MENU - Add_Reactor
//==========================================================================

bool CMine::AddReactor (short nSegment, bool bCreate, bool bSetDefTextures) 
{
#if 0
for (segP = Segments (0), i = SegCount (); i; i--, segP++)
	if (segP->m_info.function == SEGMENT_FUNC_CONTROLCEN) {
		if (!bExpertMode)
			ErrorMsg ("There is already a reactor in this mine.");
		return false;
		}
#endif
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
if (bCreate && !AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, SEGMENT_FUNC_CONTROLCEN, bSetDefTextures ? (IsD1File ()) ? 10 : 357 : -1)) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
if (bCreate) {
	if (!CopyObject (OBJ_CNTRLCEN, nSegment)) {
		undoManager.ResetModified (bUndo);
		return false; 
		}	
	CurrObj ()->m_info.id = (IsD1File ()) ? 0 : 2;
	AutoLinkExitToReactor ();
	}
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

//==========================================================================
// MENU - Add_EquipMaker
//==========================================================================

bool CMine::AddEquipMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
{
int n_matcen = (int) MineInfo ().equipgen.count;
if (n_matcen >= MAX_ROBOT_MAKERS) {
    ErrorMsg ("Maximum number of equipment makers reached");
	 return false;
	}
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
if (bCreate && !AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
DLE.MineView ()->DelayRefresh (true);
if (!DefineSegment (nSegment, SEGMENT_FUNC_EQUIPMAKER, -1)) {
	undoManager.ResetModified (bUndo);
	DLE.MineView ()->DelayRefresh (false);
	return false; 
	}	
EquipGens (n_matcen)->m_info.objFlags [0] = 0;
EquipGens (n_matcen)->m_info.objFlags [1] = 0;
EquipGens (n_matcen)->m_info.hitPoints = 0;
EquipGens (n_matcen)->m_info.interval = 0;
EquipGens (n_matcen)->m_info.nSegment = nSegment;
EquipGens (n_matcen)->m_info.nFuelCen = n_matcen;
Segments (Current ()->nSegment)->m_info.value = 
Segments (Current ()->nSegment)->m_info.nMatCen = n_matcen;
MineInfo ().equipgen.count++;
undoManager.Unlock ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
return true;
}

//==========================================================================
// MENU - Add_RobotMaker
//==========================================================================

bool CMine::AddRobotMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
{
int n_matcen = (int) MineInfo ().botgen.count;
if (n_matcen >= MAX_ROBOT_MAKERS) {
    ErrorMsg ("Maximum number of robot makers reached");
	 return false;
	}
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
if (bCreate && !AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
DLE.MineView ()->DelayRefresh (true);
if (!DefineSegment (nSegment, SEGMENT_FUNC_ROBOTMAKER,  bSetDefTextures ? (IsD1File ()) ? 339 : 361 : -1)) {
	undoManager.ResetModified (bUndo);
	DLE.MineView ()->DelayRefresh (false);
	return false; 
	}	
BotGens (n_matcen)->m_info.objFlags [0] = 8;
BotGens (n_matcen)->m_info.objFlags [1] = 0;
BotGens (n_matcen)->m_info.hitPoints = 0;
BotGens (n_matcen)->m_info.interval = 0;
BotGens (n_matcen)->m_info.nSegment = nSegment;
BotGens (n_matcen)->m_info.nFuelCen = n_matcen;
Segments (Current ()->nSegment)->m_info.value = 
Segments (Current ()->nSegment)->m_info.nMatCen = n_matcen;
MineInfo ().botgen.count++;
undoManager.Unlock ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddGoalCube (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture) 
{
if (IsD1File ()) {
	if (!bExpertMode)
		ErrorMsg ("Flag goals are not available in Descent 1.");
	return false;
	}
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
if (bCreate && !AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, nType, bSetDefTextures ? nTexture : -1)) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddTeamCube (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture) 
{
if (IsD1File ()) {
	if (!bExpertMode)
		ErrorMsg ("Team start positions are not available in Descent 1.");
	return false;
	}
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
if (bCreate && !AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, nType, bSetDefTextures ? nTexture : -1)) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddSkyboxCube (short nSegment, bool bCreate) 
{
if (IsD1File ()) {
	if (!bExpertMode)
		ErrorMsg ("Blocked cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
if (bCreate && !AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, SEGMENT_FUNC_SKYBOX, -1)) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddSpeedBoostCube (short nSegment, bool bCreate) 
{
if (IsD1File ()) {
	if (!bExpertMode)
		ErrorMsg ("Speed boost cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
if (bCreate && !AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, SEGMENT_FUNC_SPEEDBOOST, -1)) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

//==========================================================================

int CMine::FuelCenterCount (void)
{
int n_fuelcen = 0;
CSegment *segP = Segments (0);
int i;
for (i = 0; i < SegCount (); i++, segP++)
	if ((segP->m_info.function == SEGMENT_FUNC_FUELCEN) || (segP->m_info.function == SEGMENT_FUNC_REPAIRCEN))
		n_fuelcen++;
return n_fuelcen;
}

//==========================================================================
// MENU - Add_FuelCenter
//==========================================================================

bool CMine::AddFuelCenter (short nSegment, byte nType, bool bCreate, bool bSetDefTextures) 
{
// count number of fuel centers
int n_fuelcen = FuelCenterCount ();
CSegment *segP = Segments (0);
if (n_fuelcen >= MAX_NUM_FUELCENS) {
	ErrorMsg ("Maximum number of fuel centers reached.");
	return false;
	}
if ((IsD1File ()) && (nType == SEGMENT_FUNC_REPAIRCEN)) {
	if (!bExpertMode)
		ErrorMsg ("Repair centers are not available in Descent 1.");
	return false;
	}
int last_segment = Current ()->nSegment;
bool bUndo = undoManager.SetModified (TRUE);
if (bCreate && !AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
int new_segment = Current ()->nSegment;
Current ()->nSegment = last_segment;
if (bSetDefTextures && (nType == SEGMENT_FUNC_FUELCEN) && (MineInfo ().walls.count < MAX_WALLS))
	AddWall (Current ()->nSegment, Current ()->nSide, WALL_ILLUSION, 0, KEY_NONE, -1, -1); // illusion
Current ()->nSegment = new_segment;
if (!((nType == SEGMENT_FUNC_FUELCEN) ?
	   DefineSegment (nSegment, nType,  bSetDefTextures ? ((IsD1File ()) ? 322 : 333) : -1, WALL_ILLUSION) :
	   DefineSegment (nSegment, nType,  bSetDefTextures ? 433 : -1, -1)) //use the blue goal texture for repair centers
	) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

//---------------------------------------------------------------------------
// TMainWindow - add_door()
//
// Action - Adds a wall to both sides of the current side
//---------------------------------------------------------------------------

bool CMine::AddDoor (byte type, byte flags, byte keys, char nClip, short nTexture) 
{
  short 	nOppSeg, nOppSide;
  ushort nWall;

nWall = current.Side ()->m_info.nWall;
if (nWall < MineInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (MineInfo ().walls.count + 1 >= MAX_WALLS) {
	ErrorMsg ("Maximum number of Walls reached");
	return false;
	}
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
// add a door to the current segment/side
if (AddWall (Current ()->nSegment, Current ()->nSide, type, flags, keys, nClip, nTexture)) {
	// add a door to the opposite segment/side
	if (GetOppositeSide (nOppSeg, nOppSide, Current ()->nSegment, Current ()->nSide) &&
		 AddWall (nOppSeg, nOppSide, type, flags, keys, nClip, nTexture)) {
		undoManager.Unlock ();
		DLE.MineView ()->Refresh ();
		return true;
		}
	}
undoManager.ResetModified (bUndo);
return false;
}

//==========================================================================
// MENU - Add Auto Door
//==========================================================================

bool CMine::AddAutoDoor (char nClip, short nTexture) 
{
return AddDoor (WALL_DOOR, WALL_DOOR_AUTO, KEY_NONE, nClip,  nTexture);
}

//==========================================================================
// MENU - Add Prison Door
//==========================================================================

bool CMine::AddPrisonDoor () 
{
return AddDoor (WALL_BLASTABLE, 0, 0, -1, -1);
}

//==========================================================================
// MENU - Add Guide bot door
//==========================================================================

bool CMine::AddGuideBotDoor() 
{
if (IsD1File ()) {
	ErrorMsg ("Guide bot doors are not allowed in Descent 1");
	return false;
  }
return AddDoor (WALL_BLASTABLE, 0, 0, 46, -1);
}

//==========================================================================
// MENU - Add Fuel Cell
//==========================================================================

bool CMine::AddFuelCell () 
{
return AddDoor (WALL_ILLUSION, 0, 0, -1, (IsD1File ()) ? 328 : 353);
}

//==========================================================================
// MENU - Add Illusionary Wall
//==========================================================================

bool CMine::AddIllusionaryWall () 
{
return AddDoor (WALL_ILLUSION, 0, 0, -1, 0);
}

//==========================================================================
// MENU - Add ForceField
//==========================================================================

bool CMine::AddForceField () 
{
if (IsD1File ()) {
	ErrorMsg ("Force fields are not supported in Descent 1");
   return false;
	}
return AddDoor (WALL_CLOSED, 0, 0, -1, 420);
}

//==========================================================================
// MENU - Add Fan
//
// note: -2 used for clip to so texture will be used for nOvlTex
//==========================================================================

bool CMine::AddFan ()
{
return AddDoor (WALL_CLOSED, 0, 0, -2, (IsD1File ()) ? 325 : 354);
}

//==========================================================================
// MENU - Add Water Fall
//==========================================================================

bool CMine::AddWaterFall ()
{
if (IsD1File ()) {
	ErrorMsg ("Water falls are not supported in Descent 1");
   return false;
	}
return AddDoor (WALL_ILLUSION, 0, 0, -1, 401);
}

//==========================================================================
// MENU - Add Lava Fall
//==========================================================================

bool CMine::AddLavaFall() 
{
if (IsD1File ()) {
	ErrorMsg ("Lava falls are not supported in Descent 1");
   return false;
	}
  // key of 5 selects lava fall
return AddDoor (WALL_ILLUSION, 0, 0, -1, 408);
}

//==========================================================================
// MENU - Add Grate
//
// note: -2 used for clip to so texture will be used for nOvlTex
//==========================================================================

bool CMine::AddGrate() 
{
return AddDoor (WALL_CLOSED, 0, 0, -2, (IsD1File ()) ? 246 : 321);
}

//==========================================================================
// MENU - Add Exit
//==========================================================================

bool CMine::AddNormalExit() 
{
return AddExit (TT_EXIT);
}

//--------------------------------------------------------------------------
// add_exit()
//
// Action - adds a wall and a exit trigger to the current segment/side
//--------------------------------------------------------------------------

bool CMine::AddExit (short type) 
{

ushort nWall = Segments (Current ()->nSegment)->m_sides [Current ()->nSide].m_info.nWall;
if (nWall < MineInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (MineInfo ().walls.count >= MAX_WALLS - 1) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
if (MineInfo ().triggers.count >= MAX_TRIGGERS - 1) {
	ErrorMsg ("Maximum number of triggers reached");
	return false;
	}
// make a new wall and a new trigger
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
if (AddWall (Current ()->nSegment, Current ()->nSide, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
// set clip number and texture
	Walls () [MineInfo ().walls.count-1].m_info.nClip = 10;
	SetTexture (Current ()->nSegment, Current ()->nSide, 0, (IsD1File ()) ? 444 : 508);
	AddTrigger (MineInfo ().walls.count - 1, type);
// add a new wall and trigger to the opposite segment/side
	short nOppSeg, nOppSide;
	if (GetOppositeSide (nOppSeg, nOppSide, Current ()->nSegment, Current ()->nSide) &&
		AddWall (nOppSeg, nOppSide, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
		// set clip number and texture
		Walls () [MineInfo ().walls.count - 1].m_info.nClip = 10;
		SetTexture (nOppSeg, nOppSide, 0, (IsD1File ()) ? 444 : 508);
		AutoLinkExitToReactor();
		undoManager.Unlock ();
		DLE.MineView ()->Refresh ();
		return true;
		}
	}
undoManager.ResetModified (bUndo);
return false;
}

//==========================================================================
// MENU - Add secret exit
//==========================================================================

bool CMine::AddSecretExit () 
{
if (IsD1File ()) {
    AddExit (TT_SECRET_EXIT);
	 return false;
	}
if (MineInfo ().walls.count >= MAX_WALLS) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
if (MineInfo ().triggers.count >= MAX_TRIGGERS - 1) {
	ErrorMsg ("Maximum number of triggers reached");
	return false;
	}
int last_segment = Current ()->nSegment;
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
if (!AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false;
	}
int new_segment = Current ()->nSegment;
Current ()->nSegment = last_segment;
if (AddWall (Current ()->nSegment, Current ()->nSide, WALL_ILLUSION, 0, KEY_NONE, -1, -1)) {
	AddTrigger (MineInfo ().walls.count - 1, TT_SECRET_EXIT);
	SecretCubeNum () = Current ()->nSegment;
	SetDefaultTexture (426, -1);
	Current ()->nSegment = new_segment;
	SetDefaultTexture (426, -1);
	DLE.MineView ()->Refresh ();
	undoManager.Unlock ();
	return true;
	}
undoManager.ResetModified (bUndo);
return false;
}



bool CMine::GetTriggerResources (ushort& nWall)
{
nWall = current.Side ()->m_info.nWall;
if (nWall < MineInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (MineInfo ().walls.count >= MAX_WALLS - 1) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
if (MineInfo ().triggers.count >= MAX_TRIGGERS - 1) {
	ErrorMsg ("Maximum number of triggers reached");
	return false;
	}
return true;
}

//--------------------------------------------------------------------------
// AddDoorTrigger
//
// Action - checks other cube's side to see if there is a door there,
//          then adds a trigger for that door
//--------------------------------------------------------------------------

bool CMine::AutoAddTrigger (short wall_type, ushort wall_flags, ushort trigger_type) 
{
ushort nWall;
if (!GetTriggerResources (nWall))
	return false;
// make a new wall and a new trigger
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
if (AddWall (Current ()->nSegment, Current ()->nSide, (byte) wall_type, wall_flags, KEY_NONE, -1, -1) &&
	 AddTrigger (MineInfo ().walls.count - 1, trigger_type)) {
	short nTrigger = MineInfo ().triggers.count - 1;
	// set link to trigger target
	Triggers (nTrigger)->m_count = 1;
	Triggers (nTrigger)->m_targets [0].m_nSegment = Other ()->nSegment;
	Triggers (nTrigger)->m_targets [0].m_nSide = Other ()->nSide;
	undoManager.Unlock ();
	DLE.MineView ()->Refresh ();
	return true;
	}
undoManager.ResetModified (bUndo);
return false;
}



bool CMine::AddDoorTrigger (short wall_type, ushort wall_flags, ushort trigger_type) 
{
CSegment* otherSegP = other.Segment ();
ushort nWall = otherSegP->m_sides [Other ()->nSide].m_info.nWall;
if (nWall >= MineInfo ().walls.count) {
	ErrorMsg ("Other cube's side is not on a wall.\n\n"
				"Hint: Select a wall using the 'other cube' and\n"
				"select a trigger location using the 'current cube'.");
	return false;
	}
// automatically change the trigger type to open if not a door
if (Walls (nWall)->m_info.type != WALL_DOOR)
	trigger_type = TT_OPEN_WALL;
return AutoAddTrigger (wall_type, wall_flags, trigger_type);
}

//==========================================================================
// MENU - Add Door Trigger
//==========================================================================

bool CMine::AddOpenDoorTrigger() 
{
return AddDoorTrigger (WALL_OPEN,0,TT_OPEN_DOOR);
}

//==========================================================================
// MENU - Add Robot Maker Trigger
//==========================================================================

bool CMine::AddRobotMakerTrigger () 
{
CSegment *otherSegP = other.Segment ();
if (otherSegP->m_info.function != SEGMENT_FUNC_ROBOTMAKER) {
	ErrorMsg ("There is no robot maker cube selected.\n\n"
				"Hint: Select a robot maker cube using the 'other cube' and\n"
				"select a trigger location using the 'current cube'.");
	return false;
	}
return AutoAddTrigger (WALL_OPEN, 0, TT_MATCEN);
}

//==========================================================================
// MENU -
//==========================================================================

bool CMine::AddShieldTrigger() 
{
if (IsD2File ()) {
	ErrorMsg ("Descent 2 does not support shield damage Triggers ()");
   return false;
	}
return AutoAddTrigger (WALL_OPEN, 0, TT_SHIELD_DAMAGE);
}

//==========================================================================
// MENU - Add Energy Drain Trigger
//==========================================================================
bool CMine::AddEnergyTrigger() 
{
if (IsD2File ()) {
	ErrorMsg ("Descent 2 does not support energy drain Triggers ()");
   return false;
	}
return AutoAddTrigger (WALL_OPEN, 0, TT_ENERGY_DRAIN);
}

//==========================================================================
// MENU - Unlock Trigger
//==========================================================================

bool CMine::AddUnlockTrigger() 
{
if (IsD1File ()) {
   ErrorMsg ("Control Panels are not supported in Descent 1.");
   return false;
	}
return AddDoorTrigger (WALL_OVERLAY, WALL_WALL_SWITCH, TT_UNLOCK_DOOR);
}
