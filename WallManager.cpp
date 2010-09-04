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

CWallManager wallManager;

//------------------------------------------------------------------------------

bool CWallManager::HaveResources (void)
{
CWall* wallP = segmentManager.Wall ();
if (wallP != null) {
	ErrorMsg ("There is already a wall on this side");
	return false;
	}
if (Count () >= MAX_WALLS - 1) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
return true;
}

//------------------------------------------------------------------------------

short CWallManager::Add (void) 
{ 
return HaveResources () ? Count ()++ : NO_WALL; 
}

//------------------------------------------------------------------------------
// Mine - add wall
//
// Returns - TRUE on success
//
// Note: nClip & nTexture are used for call to DefineWall only.
//------------------------------------------------------------------------------

CWall* CWallManager::Create (CSideKey key, short type, ushort flags, byte keys, char nClip, short nTexture) 
{
if (!HaveResources ())
	return null;

current.Get (key);

CSegment *segP = segmentManager.Segment (key);
CSide* sideP = segmentManager.Side (key);

// if wall is an overlay, make sure there is no child
short nChild = segP->GetChild (key.m_nSide);
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
CWall* wallP = Wall (nWall);
wallP->Setup (key, nWall, (byte) type, nClip, nTexture, false);
wallP->m_nIndex = nWall;
wallP->m_info.flags = flags;
wallP->m_info.keys = keys;
// update number of Walls () in mine
Count ()++;
undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
return wallP;
}

//--------------------------------------------------------------------------

CWall* CWallManager::OppositeWall (short nSegment, short nSide)
{
CSide* sideP = segmentManager.OppositeSide (CSideKey (nSegment, nSide));
return (sideP == null) ? null : sideP->Wall ();
}

//------------------------------------------------------------------------------

void CWallManager::Delete (short nDelWall) 
{
if (nDelWall == NO_WALL)
	return;
CWall* delWallP = Wall (nDelWall);
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
CWall* oppWallP = segmentManager.OppositeWall (*delWallP);
if (oppWallP != null) 
	oppWallP->m_info.linkedWall = -1;

triggerManager.DeleteTargets (delWallP->m_nSegment, delWallP->m_nSide);
segmentManager.Side (*delWallP)->SetWall (NO_WALL);
if (nDelWall < --Count ()) { // move last wall in list to position of deleted wall
	CWall* lastWallP = Wall (Count ());
	segmentManager.Side (*lastWallP)->SetWall (nDelWall); // make last wall's side point to new wall position
	*delWallP = *lastWallP;
	delWallP->m_nIndex = Index (delWallP);
	}

undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
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
CWall* wallP = segmentManager.Wall (CSideKey (nSegment, nSide));

if (!(wallP && wallP->IsDoor ()))
	return true;

short nBaseTex, nOvlTex;

segmentManager.Textures (CSideKey (nSegment, nSide), nBaseTex, nOvlTex);

return (wallP->SetClip (nOvlTex) >= 0) || (wallP->SetClip (nBaseTex) >= 0);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateDoor (byte type, byte flags, byte keys, char nClip, short nTexture) 
{
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
// add a door to the current segment/side
if (Create (current, type, flags, keys, nClip, nTexture)) {
	// add a door to the opposite segment/side
	CSideKey opp;
	if (segmentManager.OppositeSide (opp) && Create (opp, type, flags, keys, nClip, nTexture)) {
		undoManager.Unlock ();
		DLE.MineView ()->Refresh ();
		return true;
		}
	}
undoManager.ResetModified (bUndo);
return false;
}

//------------------------------------------------------------------------------

bool CWallManager::CreateAutoDoor (char nClip, short nTexture) 
{
return CreateDoor (WALL_DOOR, WALL_DOOR_AUTO, KEY_NONE, nClip,  nTexture);
}

//------------------------------------------------------------------------------

bool CWallManager::CreatePrisonDoor (void) 
{
return CreateDoor (WALL_BLASTABLE, 0, 0, -1, -1);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateGuideBotDoor (void) 
{
if (DLE.IsD1File ()) {
	ErrorMsg ("Guide bot doors are not available in Descent 1");
	return false;
  }
return CreateDoor (WALL_BLASTABLE, 0, 0, 46, -1);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateFuelCell (void) 
{
return CreateDoor (WALL_ILLUSION, 0, 0, -1, DLE.IsD1File () ? 328 : 353);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateIllusion (void) 
{
return CreateDoor (WALL_ILLUSION, 0, 0, -1, 0);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateForceField (void) 
{
if (DLE.IsD1File ()) {
	ErrorMsg ("Force fields are not supported in Descent 1");
   return false;
	}
return CreateDoor (WALL_CLOSED, 0, 0, -1, 420);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateFan (void)
{
return CreateDoor (WALL_CLOSED, 0, 0, -2, DLE.IsD1File () ? 325 : 354);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateWaterFall (void)
{
if (DLE.IsD1File ()) {
	ErrorMsg ("Water falls are not supported in Descent 1");
   return false;
	}
return CreateDoor (WALL_ILLUSION, 0, 0, -1, 401);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateLavaFall (void) 
{
if (DLE.IsD1File ()) {
	ErrorMsg ("Lava falls are not supported in Descent 1");
   return false;
	}
return CreateDoor (WALL_ILLUSION, 0, 0, -1, 408);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateGrate (void) 
{
return CreateDoor (WALL_CLOSED, 0, 0, -2, DLE.IsD1File () ? 246 : 321);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateNormalExit (void) 
{
return CreateExit (TT_EXIT);
}

//------------------------------------------------------------------------------

bool CWallManager::CreateExit (short type) 
{
if (!triggerManager.HaveResources ())
	return false;
// make a new wall and a new trigger
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
if (Create (current, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
// set clip number and texture
	Wall (Count ()- 1)->m_info.nClip = 10;
	segmentManager.SetTextures (current.m_nSegment, current.m_nSide, 0, DLE.IsD1File () ? 444 : 508);
	triggerManager.Create (Count () - 1, type);
// add a new wall and trigger to the opposite segment/side
	CSideKey opp;
	if (segmentManager.OppositeSide (opp) && Create (opp, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
		// set clip number and texture
		Wall (Count () - 1)->m_info.nClip = 10;
		segmentManager.SetTextures (opp.m_nSegment, opp.m_nSide, 0, DLE.IsD1File () ? 444 : 508);
		triggerManager.UpdateReactor ();
		undoManager.Unlock ();
		DLE.MineView ()->Refresh ();
		return true;
		}
	}
undoManager.ResetModified (bUndo);
return false;
}

//------------------------------------------------------------------------------

bool CWallManager::CreateSecretExit (void) 
{
if (DLE.IsD1File ()) {
    CreateExit (TT_SECRET_EXIT);
	 return false;
	}
if (!triggerManager.HaveResources ())
	return false;

int nLastSeg = current.m_nSegment;
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
if (!segmentManager.Create ()) {
	undoManager.ResetModified (bUndo);
	return false;
	}
int nNewSeg = current.m_nSegment;
current.m_nSegment = nLastSeg;
if (Create (current, WALL_ILLUSION, 0, KEY_NONE, -1, -1)) {
	triggerManager.Create (Count () - 1, TT_SECRET_EXIT);
	theMine->SecretSegment () = current.m_nSegment;
	segmentManager.SetDefaultTexture (426, -1);
	current.m_nSegment = nNewSeg;
	segmentManager.SetDefaultTexture (426, -1);
	DLE.MineView ()->Refresh ();
	undoManager.Unlock ();
	return true;
	}
undoManager.ResetModified (bUndo);
return false;
}

// ----------------------------------------------------------------------------- 

void CWallManager::SetIndex (void)
{
for (short i = 0; i < Count (); i++)
	m_walls [i].m_nIndex = i;
}

// ----------------------------------------------------------------------------- 

void CWallManager::Read (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
Count () = info.count;
for (short i = 0; i < Count (); i++) {
	m_walls [i].Read (fp, nFileVersion);
	m_walls [i].m_nIndex = i;
	}
}

// ----------------------------------------------------------------------------- 

void CWallManager::Write (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
info.count = Count ();
info.offset = fp.Tell ();
for (short i = 0; i < Count (); i++)
	m_walls [i].Write (fp, nFileVersion);
}

// ----------------------------------------------------------------------------- 

void CWallManager::Clear (void)
{
for (short i = 0; i < Count (); i++)
	m_walls [i].Clear ();
}

//------------------------------------------------------------------------------
//eof wallmanager.cpp