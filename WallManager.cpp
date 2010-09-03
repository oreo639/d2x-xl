// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>

#include "SegmentManager.h"
#include "ObjectManager.h"
#include "WallManager.h"
#include "TextureManager.h"
#include "UndoManager.h"

CWallManager wallManager;

//------------------------------------------------------------------------------

bool CWallManager::HaveResources (void)
{
CWall* wallP = segmentManager.GetWall ();
if (wallP != null) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (Count () >= MAX_WALLS - 1) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
}

//------------------------------------------------------------------------------
// Mine - add wall
//
// Returns - TRUE on success
//
// Note: nClip & nTexture are used for call to DefineWall only.
//------------------------------------------------------------------------------

CWall* CWallManager::Add (short nSegment, short nSide, short type, ushort flags, byte keys, char nClip, short nTexture) 
{
if (!HaveResources ())
	return null;

current.Get (nSegment, nSide);

CSegment *segP = segmentManager.GetSegment (nSegment);
CSide* sideP = segmentManager.GetSide (nSegment, nSide);

// if wall is an overlay, make sure there is no child
short nChild = segP->GetChild (nSide);
if (type < 0)
	type = (nChild == -1) ? WALL_OVERLAY : WALL_OPEN;

if (type == WALL_OVERLAY) {
	if (nChild != -1) {
		ErrorMsg ("Switches can only be put on solid sides.");
		return null;
		}
	}
else {
	// otherwise make sure there is a child
	if (nChild == -1) {
		ErrorMsg ("This side must be attached to an other cube before a wall can be added.");
		return null;
		}
	}

ushort nWall = Count ();

// link wall to segment/side
undoManager.SetModified (true);
undoManager.Lock ();
sideP->SetWall (nWall);
CWall* wallP = GetWall (nWall);
wallP->Setup (nSegment, nSide, nWall, (byte) type, nClip, nTexture, false);
wallP->m_info.flags = flags;
wallP->m_info.keys = keys;
// update number of Walls () in mine
Count ()++;
undoManager.Unlock ();
////DLE.MineView ()->Refresh ();
return wallP;
}

//--------------------------------------------------------------------------

CWall* CWallManager::GetOppositeWall (short nSegment, short nSide)
{
CSide* sideP = segmentManager.GetOppositeSide (nSegment, nSide);
return (sideP == null) ? null : sideP->GetWall ();
}

//------------------------------------------------------------------------------

void CWallManager::Delete (short nDelWall) 
{
if (nDelWall == NO_WALL)
	return;
CWall* delWallP = GetWall (nDelWall);
if (delWallP == null) {
	delWallP = current.Wall ();
	nDelWall = Index (delWallP);
	}

if (delWallP == null)
	return;
// if trigger exists, remove it as well
triggerManager.Delete (delWallP->m_info.nTrigger);
undoManager.SetModified (true);
undoManager.Lock ();
// remove references to the deleted wall
CWall* oppWallP = segmentManager.GetOppositeWall (delWallP->m_nSegment, delWallP->m_nSide);
if (oppWallP != null) 
	oppWallP->m_info.linkedWall = -1;

triggerManager.DeleteTargets (delWallP->m_nSegment, delWallP->m_nSide);
segmentManager.GetSide (delWallP->m_nSegment, delWallP->m_nSide)->SetWall (NO_WALL);
if (nDelWall < --Count ()) { // move last wall in list to position of deleted wall
	CWall* lastWallP = GetWall (Count ());
	segmentManager.GetSide (lastWallP->m_nSegment, lastWallP->m_nSide)->SetWall (nDelWall); // make last wall's side point to new wall position
	*delWallP = *lastWallP;
	}

undoManager.Unlock ();
////DLE.MineView ()->Refresh ();
triggerManager.UpdateReactor ();
}

//------------------------------------------------------------------------------

CWall *CWallManager::FindBySide (CSideKey key, int i)
{
for (; i < Count (); i++)
	if (m_walls [i] == key)
		return &m_walls [i];
return null;
}

//------------------------------------------------------------------------------

CWall* CWallManager::FindByTrigger (short nTrigger, int i)
{
for (; i < Count (); i++)
	if (m_walls [i].m_info.nTrigger == nTrigger)
		return &m_walls [i];
return null;
}

//------------------------------------------------------------------------------

void CWallManager::UpdateTrigger (short nOldTrigger, short nNewTrigger)
{
	CWall* wallP = FindByTrigger (nOldTrigger);

if (wallP != null)
	wallP->SetTrigger (nNewTrigger);
}

//------------------------------------------------------------------------------

bool CWallManager::ClipFromTexture (short nSegment, short nSide)
{
CWall* wallP = segmentManager.GetWall (nSegment, nSide);

if (!(wallP && wallP->IsDoor ()))
	return true;

short nBaseTex, nOvlTex;

segmentManager.GetTextures (nSegment, nSide, nBaseTex, nOvlTex);

return (wallP->SetClip (nOvlTex) >= 0) || (wallP->SetClip (nBaseTex) >= 0);
}

//------------------------------------------------------------------------------

bool CWallManager::AddDoor (byte type, byte flags, byte keys, char nClip, short nTexture) 
{
short nWall = current.Side ()->m_info.nWall;
if (nWall < Count ()) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (Count () + 1 >= MAX_WALLS) {
	ErrorMsg ("Maximum number of Walls reached");
	return false;
	}
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
// add a door to the current segment/side
if (Add (current.m_nSegment, current.m_nSide, type, flags, keys, nClip, nTexture)) {
	// add a door to the opposite segment/side
	short nOppSeg, nOppSide;
	if (segmentManager.GetOppositeSide (current, nOppSeg, nOppSide) &&
		 Add (nOppSeg, nOppSide, type, flags, keys, nClip, nTexture)) {
		undoManager.Unlock ();
		//DLE.MineView ()->Refresh ();
		return true;
		}
	}
undoManager.ResetModified (bUndo);
return false;
}

//------------------------------------------------------------------------------

bool CWallManager::AddAutoDoor (char nClip, short nTexture) 
{
return AddDoor (WALL_DOOR, WALL_DOOR_AUTO, KEY_NONE, nClip,  nTexture);
}

//------------------------------------------------------------------------------

bool CWallManager::AddPrisonDoor (void) 
{
return AddDoor (WALL_BLASTABLE, 0, 0, -1, -1);
}

//------------------------------------------------------------------------------

bool CWallManager::AddGuideBotDoor (void) 
{
if (theMine->IsD1File ()) {
	ErrorMsg ("Guide bot doors are not available in Descent 1");
	return false;
  }
return AddDoor (WALL_BLASTABLE, 0, 0, 46, -1);
}

//------------------------------------------------------------------------------

bool CWallManager::AddFuelCell (void) 
{
return AddDoor (WALL_ILLUSION, 0, 0, -1, theMine->IsD1File () ? 328 : 353);
}

//------------------------------------------------------------------------------

bool CWallManager::AddIllusion (void) 
{
return AddDoor (WALL_ILLUSION, 0, 0, -1, 0);
}

//------------------------------------------------------------------------------

bool CWallManager::AddForceField (void) 
{
if (theMine->IsD1File ()) {
	ErrorMsg ("Force fields are not supported in Descent 1");
   return false;
	}
return AddDoor (WALL_CLOSED, 0, 0, -1, 420);
}

//------------------------------------------------------------------------------

bool CWallManager::AddFan (void)
{
return AddDoor (WALL_CLOSED, 0, 0, -2, theMine->IsD1File () ? 325 : 354);
}

//------------------------------------------------------------------------------

bool CWallManager::AddWaterFall (void)
{
if (theMine->IsD1File ()) {
	ErrorMsg ("Water falls are not supported in Descent 1");
   return false;
	}
return AddDoor (WALL_ILLUSION, 0, 0, -1, 401);
}

//------------------------------------------------------------------------------

bool CWallManager::AddLavaFall (void) 
{
if (theMine->IsD1File ()) {
	ErrorMsg ("Lava falls are not supported in Descent 1");
   return false;
	}
return AddDoor (WALL_ILLUSION, 0, 0, -1, 408);
}

//------------------------------------------------------------------------------

bool CWallManager::AddGrate (void) 
{
return AddDoor (WALL_CLOSED, 0, 0, -2, theMine->IsD1File () ? 246 : 321);
}

//------------------------------------------------------------------------------

bool CWallManager::AddNormalExit (void) 
{
return AddExit (TT_EXIT);
}

//------------------------------------------------------------------------------

bool CWallManager::AddExit (short type) 
{

if (MineInfo ().triggers.count >= MAX_TRIGGERS - 1) {
	ErrorMsg ("Maximum number of triggers reached");
	return false;
	}
// make a new wall and a new trigger
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
if (AddWall (current.m_nSegment, current.m_nSide, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
// set clip number and texture
	Walls () [Count ()-1].m_info.nClip = 10;
	SetTexture (current.m_nSegment, current.m_nSide, 0, (IsD1File ()) ? 444 : 508);
	AddTrigger (Count () - 1, type);
// add a new wall and trigger to the opposite segment/side
	short nOppSeg, nOppSide;
	if (GetOppositeSide (nOppSeg, nOppSide, current.m_nSegment, current.m_nSide) &&
		AddWall (nOppSeg, nOppSide, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
		// set clip number and texture
		Walls () [Count () - 1].m_info.nClip = 10;
		SetTexture (nOppSeg, nOppSide, 0, (IsD1File ()) ? 444 : 508);
		AutoUpdateReactor();
		undoManager.Unlock ();
		//DLE.MineView ()->Refresh ();
		return true;
		}
	}
undoManager.ResetModified (bUndo);
return false;
}

//------------------------------------------------------------------------------

bool CWallManager::AddSecretExit (void) 
{
if (theMine->IsD1File ()) {
    AddExit (TT_SECRET_EXIT);
	 return false;
	}
if (Count () >= MAX_WALLS) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
if (triggerManager.Count () >= MAX_TRIGGERS - 1) {
	ErrorMsg ("Maximum number of triggers reached");
	return false;
	}
int last_segment = current.m_nSegment;
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
if (!AddSegment ()) {
	undoManager.ResetModified (bUndo);
	return false;
	}
int new_segment = current.m_nSegment;
current.m_nSegment = last_segment;
if (AddWall (current.m_nSegment, current.m_nSide, WALL_ILLUSION, 0, KEY_NONE, -1, -1)) {
	AddTrigger (Count () - 1, TT_SECRET_EXIT);
	SecretCubeNum () = current.m_nSegment;
	SetDefaultTexture (426, -1);
	current.m_nSegment = new_segment;
	SetDefaultTexture (426, -1);
	//DLE.MineView ()->Refresh ();
	undoManager.Unlock ();
	return true;
	}
undoManager.ResetModified (bUndo);
return false;
}

// ----------------------------------------------------------------------------- 

void Read (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
for (int = 0; i < Count (); i++)
	m_walls [i].Read (fp, info, nFileVersion);
}

// ----------------------------------------------------------------------------- 

void Write (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
for (int = 0; i < Count (); i++)
	m_walls [i].Write (fp, info, nFileVersion);
}

// ----------------------------------------------------------------------------- 

void Clear (void)
{
for (int = 0; i < Count (); i++)
	m_walls [i].Clear ();
}

//------------------------------------------------------------------------------
//eof wallmanager.cpp