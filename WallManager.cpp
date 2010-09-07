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
if (m_free.Empty ()) {
	ErrorMsg ("Maximum number of walls reached");
	return false;
	}
return true;
}

//------------------------------------------------------------------------------

short CWallManager::Add (void) 
{ 
if (!HaveResources ())
	return NO_WALL;
int nWall = --m_free;
m_walls [nWall].Clear ();
m_walls [nWall].Backup (opAdd);
WallCount ()++;
return (short) nWall;
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
short nChild = segP->Child (key.m_nSide);
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

ushort nWall = WallCount ();

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
WallCount ()++;
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
delWallP->Backup (opDelete);
// if trigger exists, remove it as well
triggerManager.Delete (delWallP->m_info.nTrigger);
undoManager.SetModified (true);
undoManager.Lock ();
// remove references to the deleted wall
CWall* oppWallP = segmentManager.OppositeWall (*delWallP);
if (oppWallP != null) 
	oppWallP->m_info.linkedWall = -1;

triggerManager.DeleteTargets (*delWallP);
segmentManager.Side (*delWallP)->SetWall (NO_WALL);
m_free += (int) nDelWall;
WallCount ()--;

undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
triggerManager.UpdateReactor ();
}

//------------------------------------------------------------------------------

CWall *CWallManager::FindBySide (CSideKey key, int i)
{
for (CWallIterator wi; wi; wi++)
	if (*wi == key)
		return &(*wi);
return null;
}

//------------------------------------------------------------------------------

CWall* CWallManager::FindByTrigger (short nTrigger, int i)
{
for (CWallIterator wi; wi; wi++)
	if (wi->m_info.nTrigger == nTrigger)
		return &(*wi);
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

bool CWallManager::ClipFromTexture (CSideKey key)
{
CWall* wallP = segmentManager.Wall (key);

if (!(wallP && wallP->IsDoor ()))
	return true;

short nBaseTex, nOvlTex;

segmentManager.Textures (key, nBaseTex, nOvlTex);

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
	Wall (WallCount ()- 1)->m_info.nClip = 10;
	segmentManager.SetTextures (current, 0, DLE.IsD1File () ? 444 : 508);
	triggerManager.Create (WallCount () - 1, type);
// add a new wall and trigger to the opposite segment/side
	CSideKey opp;
	if (segmentManager.OppositeSide (opp) && Create (opp, WALL_DOOR, WALL_DOOR_LOCKED, KEY_NONE, -1, -1)) {
		// set clip number and texture
		Wall (WallCount () - 1)->m_info.nClip = 10;
		segmentManager.SetTextures (opp, 0, DLE.IsD1File () ? 444 : 508);
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
	triggerManager.Create (WallCount () - 1, TT_SECRET_EXIT);
	theMine->SecretSegment () = current.m_nSegment;
	segmentManager.SetDefaultTexture (426);
	current.m_nSegment = nNewSeg;
	segmentManager.SetDefaultTexture (426);
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
int j = 0;
for (CWallIterator wi; wi; wi++)
	wi->m_nIndex = j++;
}

// ----------------------------------------------------------------------------- 

void CWallManager::ReadWalls (CFileManager& fp, int nFileVersion)
{
if (m_info [0].offset >= 0) {
	fp.Seek (m_info [0].offset);
	m_free.Reset ();
	for (short i = 0; i < WallCount (); i++) {
		if (i < MAX_WALLS) {
			CWall* wallP = Wall (Add ());
			wallP->Read (fp, nFileVersion);
			wallP->m_nIndex = i;
			}
		else {
			CWall w;
			w.Read (fp, nFileVersion);
			}
		}
	if (WallCount () > MAX_WALLS)
		WallCount () = MAX_WALLS;
	}
}

// ----------------------------------------------------------------------------- 

void CWallManager::WriteWalls (CFileManager& fp, int nFileVersion)
{
if (WallCount () == 0)
	m_info [0].offset = -1;
else {
	m_info [0].size = 24;
	m_info [0].offset = fp.Tell ();
	for (CWallIterator wi; wi; wi++)
		wi->Write (fp, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void CWallManager::ReadDoors (CFileManager& fp, int nFileVersion)
{
if (m_info [1].offset >= 0) {
	fp.Seek (m_info [1].offset);
	for (short i = 0; i < DoorCount (); i++) {
		if (i < MAX_DOORS) {
			m_doors [i].Read (fp, nFileVersion);
			m_doors [i].m_nIndex = i;
			}
		else {
			CDoor d;
			d.Read (fp, nFileVersion);
			}
		}
	if (DoorCount () > MAX_DOORS)
		DoorCount () = MAX_DOORS;
	}
}

// ----------------------------------------------------------------------------- 

void CWallManager::WriteDoors (CFileManager& fp, int nFileVersion)
{
if (DoorCount () == 0)
	m_info [1].offset = -1;
else {
	m_info [1].size = 16;
	m_info [1].offset = fp.Tell ();
	for (short i = 0; i < WallCount (); i++)
		m_doors [i].Write (fp, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void CWallManager::Clear (void)
{
for (short i = 0; i < WallCount (); i++)
	m_walls [i].Clear ();
for (short i = 0; i < DoorCount (); i++)
	m_doors [i].Clear ();
}

//------------------------------------------------------------------------------

void CWallManager::Read (CFileManager& fp, int nFileVersion)
{
ReadWalls (fp, nFileVersion);
ReadDoors (fp, nFileVersion);
}

//------------------------------------------------------------------------------

void CWallManager::Write (CFileManager& fp, int nFileVersion)
{
WriteWalls (fp, nFileVersion);
WriteDoors (fp, nFileVersion);
}

//------------------------------------------------------------------------------
// put up a warning if changing a door's texture

void CWallManager::CheckForDoor (CSideKey key) 
{
if (bExpertMode)
	return;

current.Get (key);
CWall* wallP = segmentManager.Wall (key);

if (!wallP)
 return;
if (!wallP->IsDoor ())
	return;

ErrorMsg ("Changing the texture of a door only affects\n"
			 "how the door will look before it is opened.\n"
			 "You can use this trick to hide a door\n"
			 "until it is used for the first time.\n\n"
			 "Hint: To change the door animation,\n"
			 "select \"Wall edit...\" from the Tools\n"
			 "menu and change the clip number.");
}

//------------------------------------------------------------------------------

int CWallManager::Fix (void)
{
int errFlags = 0;
CWall *wallP = Wall (0);
for (short nWall = WallCount (); nWall > 0; nWall--, wallP++) {
	// check nSegment
	if (wallP->m_nSegment < 0 || wallP->m_nSegment > segmentManager.Count ()) {
		wallP->m_nSegment = 0;
		errFlags |= 16;
		}
	if ((wallP->m_nSide < 0) || (wallP->m_nSide > 5)) {
		wallP->m_nSide = 0;
		errFlags |= 16;
		}
	}
return errFlags;
}

//------------------------------------------------------------------------------
//eof wallmanager.cpp