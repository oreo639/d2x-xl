// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"

//==========================================================================
// SetDefaultTexture
//==========================================================================

bool CMine::SetDefaultTexture (INT16 nTexture, INT16 walltype)
{
if (nTexture < 0)
	return true;
INT16 nSegment = Current ()->nSegment;
INT16 nOppSeg, nOppSide;
CSegment *segP = Segments (nSegment);
CSide *sideP = segP->sides;
double scale = pTextures [m_fileType][nTexture].Scale (nTexture);
segP->child_bitmask |= (1 << MAX_SIDES_PER_SEGMENT);
// set textures
for (INT16 nSide = 0; nSide < 6; nSide++, sideP++) {
	if (segP->children [nSide] == -1) {
		SetTexture (nSegment, nSide, nTexture, 0);
	/*
		sideP->uvls [0].l = 0x8000;
		sideP->uvls [1].l = 0x8000;
		sideP->uvls [2].l = 0x8000;
		sideP->uvls [3].l = 0x8000;
	*/
		INT32 i;
		for (i = 0; i < 4; i++) {
			sideP->uvls [i].u = (INT16) ((double) default_uvls [i].u / scale);
			sideP->uvls [i].v = (INT16) ((double) default_uvls [i].v / scale);
			sideP->uvls [i].l = default_uvls [i].l;
			}
		SetUV (nSegment, nSide, 0, 0, 0);
		}
	else if (nTexture >= 0) {
		if (walltype >= 0) {
			if ((GameInfo ().walls.count < MAX_WALLS (this)) &&
				 (Segments (nSegment)->sides [nSide].nWall >= GameInfo ().walls.count))
				AddWall (nSegment, nSide, (UINT8) walltype, 0, KEY_NONE, -1, -1); // illusion
			else
				return false;
			if ((GameInfo ().walls.count < MAX_WALLS (this)) &&
				 GetOppositeSide (nOppSeg, nOppSide, nSegment, nSide) &&
				 (Segments (nOppSeg)->sides [nOppSide].nWall >= GameInfo ().walls.count))
				AddWall (nOppSeg, nOppSide, (UINT8) walltype, 0, KEY_NONE, -1, -1); // illusion
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

void CMine::UndefineSegment (INT16 nSegment)
{
	CSegment *segP = (nSegment < 0) ? CurrSeg () : Segments (nSegment);

nSegment = INT16 (segP - Segments ());
if (segP->function == SEGMENT_FUNC_ROBOTMAKER) {
	// remove matcen
	INT32 nMatCens = (INT32) GameInfo ().botgen.count;
	if (nMatCens > 0) {
		// fill in deleted matcen
		INT32 nDelMatCen = segP->nMatCen;
		if ((nDelMatCen >= 0) && (nDelMatCen < --nMatCens)) {
			memcpy (BotGens (nDelMatCen), BotGens (nMatCens), sizeof (CRobotMaker));
			BotGens (nDelMatCen)->nFuelCen = nDelMatCen;
			segP->nMatCen = -1;
			}
		GameInfo ().botgen.count--;
		INT32 i;
		for (i = 0; i < 6; i++)
			DeleteTriggerTargets (nSegment, i);
		CSegment *s;
		for (i = SegCount (), s = Segments (); i; i--, s++)
			if ((segP->function == SEGMENT_FUNC_ROBOTMAKER) && (s->nMatCen == nMatCens)) {
				s->nMatCen = nDelMatCen;
				break;
				}
		}
	segP->nMatCen = -1;
	}
if (segP->function == SEGMENT_FUNC_EQUIPMAKER) {
	// remove matcen
	INT32 nMatCens = (INT32) GameInfo ().equipgen.count;
	if (nMatCens > 0) {
		// fill in deleted matcen
		INT32 nDelMatCen = segP->nMatCen;
		if ((nDelMatCen >= 0) && (nDelMatCen < --nMatCens)) {
			memcpy (EquipGens (nDelMatCen), EquipGens (nMatCens), sizeof (CRobotMaker));
			EquipGens (nDelMatCen)->nFuelCen = nDelMatCen;
			segP->nMatCen = -1;
			}
		GameInfo ().equipgen.count--;
		INT32 i;
		for (i = 0; i < 6; i++)
			DeleteTriggerTargets (nSegment, i);
		CSegment *s;
		nDelMatCen += (INT32) GameInfo ().botgen.count;
		for (i = SegCount (), s = Segments (); i; i--, s++)
			if ((s->function == SEGMENT_FUNC_EQUIPMAKER) && (s->nMatCen == nMatCens)) {
				s->nMatCen = nDelMatCen;
				break;
			}
		}
	segP->nMatCen = -1;
	}
else if (segP->function == SEGMENT_FUNC_FUELCEN) { //remove all fuel cell walls
	CSegment *childseg;
	CSide *oppside, *sideP = segP->sides;
	CWall *wallP;
	INT16 nOppSeg, nOppSide;
	for (INT16 nSide = 0; nSide < 6; nSide++, sideP++) {
		if (segP->children [nSide] < 0)	// assume no wall if no child segment at the current side
			continue;
		childseg = Segments (segP->children [nSide]);
		if (childseg->function == SEGMENT_FUNC_FUELCEN)	// don't delete if child segment is fuel center
			continue;
		// if there is a wall and it's a fuel cell delete it
		if ((wall = GetWall (nSegment, nSide)) && 
			 (wallP->type == WALL_ILLUSION) && (sideP->nBaseTex == (IsD1File () ? 322 : 333)))
			DeleteWall (sideP->nWall);
		// if there is a wall at the opposite side and it's a fuel cell delete it
		if (GetOppositeSide (nOppSeg, nOppSide, nSegment, nSide) &&
			 (wall = GetWall (nSegment, nSide)) && (wallP->type == WALL_ILLUSION)) {
			oppside = Segments (nOppSeg)->sides + nOppSide;
			if (oppsideP->nBaseTex == (IsD1File () ? 322 : 333))
				DeleteWall (oppsideP->nWall);
			}
		}
	}
segP->child_bitmask &= ~(1 << MAX_SIDES_PER_SEGMENT);
segP->function = SEGMENT_FUNC_NONE;
}

//==========================================================================
// DefineSegment
//==========================================================================

bool CMine::DefineSegment (INT16 nSegment, UINT8 type, INT16 nTexture, INT16 walltype)
{
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
UndefineSegment (nSegment);
CSegment *segP = (nSegment < 0) ? CurrSeg () : Segments (nSegment);
segP->function = type;
segP->child_bitmask |= (1 << MAX_SIDES_PER_SEGMENT);
SetDefaultTexture (nTexture, walltype);
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================
// MENU - Add_Reactor
//==========================================================================

bool CMine::AddReactor (INT16 nSegment, bool bCreate, bool bSetDefTextures) 
{
#if 0
for (segP = Segments (), i = SegCount (); i; i--, segP++)
	if (segP->function == SEGMENT_FUNC_CONTROLCEN) {
		if (!bExpertMode)
			ErrorMsg ("There is already a reactor in this mine.");
		return false;
		}
#endif
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, SEGMENT_FUNC_CONTROLCEN, bSetDefTextures ? (IsD1File ()) ? 10 : 357 : -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
if (bCreate) {
	if (!CopyObject (OBJ_CNTRLCEN, nSegment)) {
		theApp.ResetModified (bUndo);
		return false; 
		}	
	CurrObj ()->id = (IsD1File ()) ? 0 : 2;
	AutoLinkExitToReactor ();
	}
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================
// MENU - Add_EquipMaker
//==========================================================================

bool CMine::AddEquipMaker (INT16 nSegment, bool bCreate, bool bSetDefTextures) 
{
INT32 n_matcen = (INT32) GameInfo ().equipgen.count;
if (n_matcen >= MAX_NUM_MATCENS (this)) {
    ErrorMsg ("Maximum number of equipment makers reached");
	 return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.MineView ()->DelayRefresh (true);
if (!DefineSegment (nSegment, SEGMENT_FUNC_EQUIPMAKER, -1)) {
	theApp.ResetModified (bUndo);
	theApp.MineView ()->DelayRefresh (false);
	return false; 
	}	
EquipGens (n_matcen)->objFlags [0] = 0;
EquipGens (n_matcen)->objFlags [1] = 0;
EquipGens (n_matcen)->hitPoints = 0;
EquipGens (n_matcen)->interval = 0;
EquipGens (n_matcen)->nSegment = nSegment;
EquipGens (n_matcen)->nFuelCen = n_matcen;
Segments (Current ()->nSegment)->value = 
Segments (Current ()->nSegment)->nMatCen = n_matcen;
GameInfo ().equipgen.count++;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================
// MENU - Add_RobotMaker
//==========================================================================

bool CMine::AddRobotMaker (INT16 nSegment, bool bCreate, bool bSetDefTextures) 
{
INT32 n_matcen = (INT32) GameInfo ().botgen.count;
if (n_matcen >= MAX_NUM_MATCENS (this)) {
    ErrorMsg ("Maximum number of robot makers reached");
	 return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.MineView ()->DelayRefresh (true);
if (!DefineSegment (nSegment, SEGMENT_FUNC_ROBOTMAKER,  bSetDefTextures ? (IsD1File ()) ? 339 : 361 : -1)) {
	theApp.ResetModified (bUndo);
	theApp.MineView ()->DelayRefresh (false);
	return false; 
	}	
BotGens (n_matcen)->objFlags [0] = 8;
BotGens (n_matcen)->objFlags [1] = 0;
BotGens (n_matcen)->hitPoints = 0;
BotGens (n_matcen)->interval = 0;
BotGens (n_matcen)->nSegment = nSegment;
BotGens (n_matcen)->nFuelCen = n_matcen;
Segments (Current ()->nSegment)->value = 
Segments (Current ()->nSegment)->nMatCen = n_matcen;
GameInfo ().botgen.count++;
theApp.UnlockUndo ();
theApp.MineView ()->DelayRefresh (false);
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddGoalCube (INT16 nSegment, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture) 
{
if (IsD1File ()) {
	if (!bExpertMode)
		ErrorMsg ("Flag goals are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, nType, bSetDefTextures ? nTexture : -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddTeamCube (INT16 nSegment, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture) 
{
if (IsD1File ()) {
	if (!bExpertMode)
		ErrorMsg ("Team start positions are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, nType, bSetDefTextures ? nTexture : -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddSkyboxCube (INT16 nSegment, bool bCreate) 
{
if (IsD1File ()) {
	if (!bExpertMode)
		ErrorMsg ("Blocked cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, SEGMENT_FUNC_SKYBOX, -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

bool CMine::AddSpeedBoostCube (INT16 nSegment, bool bCreate) 
{
if (IsD1File ()) {
	if (!bExpertMode)
		ErrorMsg ("Speed boost cubes are not available in Descent 1.");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}
if (!DefineSegment (nSegment, SEGMENT_FUNC_SPEEDBOOST, -1)) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//==========================================================================

INT32 CMine::FuelCenterCount (void)
{
INT32 n_fuelcen = 0;
CSegment *segP = Segments ();
INT32 i;
for (i = 0; i < SegCount (); i++, segP++)
	if ((segP->function == SEGMENT_FUNC_FUELCEN) || (segP->function == SEGMENT_FUNC_REPAIRCEN))
		n_fuelcen++;
return n_fuelcen;
}

//==========================================================================
// MENU - Add_FuelCenter
//==========================================================================

bool CMine::AddFuelCenter (INT16 nSegment, UINT8 nType, bool bCreate, bool bSetDefTextures) 
{
// count number of fuel centers
INT32 n_fuelcen = FuelCenterCount ();
CSegment *segP = Segments ();
if (n_fuelcen >= MAX_NUM_FUELCENS (this)) {
	ErrorMsg ("Maximum number of fuel centers reached.");
	return false;
	}
if ((IsD1File ()) && (nType == SEGMENT_FUNC_REPAIRCEN)) {
	if (!bExpertMode)
		ErrorMsg ("Repair centers are not available in Descent 1.");
	return false;
	}
INT32 last_segment = Current ()->nSegment;
bool bUndo = theApp.SetModified (TRUE);
if (bCreate && !AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
INT32 new_segment = Current ()->nSegment;
Current ()->nSegment = last_segment;
if (bSetDefTextures && (nType == SEGMENT_FUNC_FUELCEN) && (GameInfo ().walls.count < MAX_WALLS (this)))
	AddWall (Current ()->nSegment, Current ()->nSide, WALL_ILLUSION, 0, KEY_NONE, -1, -1); // illusion
Current ()->nSegment = new_segment;
if (!((nType == SEGMENT_FUNC_FUELCEN) ?
	   DefineSegment (nSegment, nType,  bSetDefTextures ? ((IsD1File ()) ? 322 : 333) : -1, WALL_ILLUSION) :
	   DefineSegment (nSegment, nType,  bSetDefTextures ? 433 : -1, -1)) //use the blue goal texture for repair centers
	) {
	theApp.ResetModified (bUndo);
	return false; 
	}	
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//---------------------------------------------------------------------------
// TMainWindow - add_door()
//
// Action - Adds a wall to both sides of the current side
//---------------------------------------------------------------------------

bool CMine::AddDoor (UINT8 type, UINT8 flags, UINT8 keys, INT8 nClip, INT16 nTexture) 
{
  INT16 	nOppSeg, nOppSide;
  UINT16 nWall;

nWall = CurrSide ()->nWall;
if (nWall < GameInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (GameInfo ().walls.count + 1 >= MAX_WALLS (this)) {
	ErrorMsg ("Maximum number of Walls reached");
	return false;
	}
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
// add a door to the current segment/side
if (AddWall (Current ()->nSegment, Current ()->nSide, type, flags, keys, nClip, nTexture)) {
	// add a door to the opposite segment/side
	if (GetOppositeSide (nOppSeg, nOppSide, Current ()->nSegment, Current ()->nSide) &&
		 AddWall (nOppSeg, nOppSide, type, flags, keys, nClip, nTexture)) {
		theApp.UnlockUndo ();
		theApp.MineView ()->Refresh ();
		return true;
		}
	}
theApp.ResetModified (bUndo);
return false;
}

//==========================================================================
// MENU - Add Auto Door
//==========================================================================

bool CMine::AddAutoDoor (INT8 nClip, INT16 nTexture) 
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

bool CMine::AddExit (INT16 type) 
{

UINT16 nWall = Segments (Current ()->nSegment)->sides [Current ()->nSide].nWall;
if (nWall < GameInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (GameInfo ().walls.count >= MAX_WALLS (this) - 1) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
if (GameInfo ().triggers.count >= MAX_TRIGGERS (this) - 1) {
	ErrorMsg ("Maximum number of triggers reached");
	return false;
	}
// make a new wall and a new trigger
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (AddWall (Current ()->nSegment, Current ()->nSide, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
// set clip number and texture
	Walls () [GameInfo ().walls.count-1].nClip = 10;
	SetTexture (Current ()->nSegment, Current ()->nSide, 0, (IsD1File ()) ? 444 : 508);
	AddTrigger (GameInfo ().walls.count - 1, type);
// add a new wall and trigger to the opposite segment/side
	INT16 nOppSeg, nOppSide;
	if (GetOppositeSide (nOppSeg, nOppSide, Current ()->nSegment, Current ()->nSide) &&
		AddWall (nOppSeg, nOppSide, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
		// set clip number and texture
		Walls () [GameInfo ().walls.count - 1].nClip = 10;
		SetTexture (nOppSeg, nOppSide, 0, (IsD1File ()) ? 444 : 508);
		AutoLinkExitToReactor();
		theApp.UnlockUndo ();
		theApp.MineView ()->Refresh ();
		return true;
		}
	}
theApp.ResetModified (bUndo);
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
if (GameInfo ().walls.count >= MAX_WALLS (this)) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
if (GameInfo ().triggers.count >= MAX_TRIGGERS (this) - 1) {
	ErrorMsg ("Maximum number of triggers reached");
	return false;
	}
INT32 last_segment = Current ()->nSegment;
bool bUndo = theApp.SetModified (true);
theApp.LockUndo ();
if (!AddSegment ()) {
	theApp.ResetModified (bUndo);
	return false;
	}
INT32 new_segment = Current ()->nSegment;
Current ()->nSegment = last_segment;
if (AddWall (Current ()->nSegment, Current ()->nSide, WALL_ILLUSION, 0, KEY_NONE, -1, -1)) {
	AddTrigger (GameInfo ().walls.count - 1, TT_SECRET_EXIT);
	SecretCubeNum () = Current ()->nSegment;
	SetDefaultTexture (426, -1);
	Current ()->nSegment = new_segment;
	SetDefaultTexture (426, -1);
	theApp.MineView ()->Refresh ();
	theApp.UnlockUndo ();
	return true;
	}
theApp.ResetModified (bUndo);
return false;
}



bool CMine::GetTriggerResources (UINT16& nWall)
{
nWall = CurrSide ()->nWall;
if (nWall < GameInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (GameInfo ().walls.count >= MAX_WALLS (this) - 1) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
if (GameInfo ().triggers.count >= MAX_TRIGGERS (this) - 1) {
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

bool CMine::AutoAddTrigger (INT16 wall_type, UINT16 wall_flags, UINT16 trigger_type) 
{
UINT16 nWall;
if (!GetTriggerResources (nWall))
	return false;
// make a new wall and a new trigger
bool bUndo = theApp.SetModified (TRUE);
theApp.LockUndo ();
if (AddWall (Current ()->nSegment, Current ()->nSide, (UINT8) wall_type, wall_flags, KEY_NONE, -1, -1) &&
	 AddTrigger (GameInfo ().walls.count - 1, trigger_type)) {
	INT16 trignum = GameInfo ().triggers.count - 1;
	// set link to trigger target
	Triggers () [trignum].count = 1;
	Triggers () [trignum].targets [0].nSegment = Other ()->nSegment;
	Triggers () [trignum].targets [0].nSide = Other ()->nSide;
	theApp.UnlockUndo ();
	theApp.MineView ()->Refresh ();
	return true;
	}
theApp.ResetModified (bUndo);
return false;
}



bool CMine::AddDoorTrigger (INT16 wall_type, UINT16 wall_flags, UINT16 trigger_type) 
{
CSegment* otherSegP = OtherSeg ();
UINT16 nWall = otherSegP->sides [Other ()->nSide].nWall;
if (nWall >= GameInfo ().walls.count) {
	ErrorMsg ("Other cube's side is not on a wall.\n\n"
				"Hint: Select a wall using the 'other cube' and\n"
				"select a trigger location using the 'current cube'.");
	return false;
	}
// automatically change the trigger type to open if not a door
if (Walls (nWall)->type != WALL_DOOR)
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
CSegment *otherSegP = OtherSeg ();
if (otherSegP->function != SEGMENT_FUNC_ROBOTMAKER) {
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
