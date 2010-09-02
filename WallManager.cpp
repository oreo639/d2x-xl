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
#include "cfile.h"
#include "texturemanager.h"

CWallManager wallManager;

//------------------------------------------------------------------------------
// Mine - add wall
//
// Returns - TRUE on success
//
// Note: nClip & nTexture are used for call to DefineWall only.
//------------------------------------------------------------------------------

CWall *CWallManager::Add (short nSegment, short nSide, short type, ushort flags, byte keys, char nClip, short nTexture) 
{
current.Get (nSegment, nSide);

ushort nWall;
CSegment *segP = segmentManager.GetSegment (nSegment);
CSide* sideP = segmentManager.GetSide (nSegment, nSide);

// if wall is an overlay, make sure there is no child
if (type < 0)
	type = (segP->GetChild (nSide) == -1) ? WALL_OVERLAY : WALL_OPEN;
	if ((type == WALL_OVERLAY) && (segP->GetChild (nSide) != -1)) {
		ErrorMsg ("Switches can only be put on solid sides.");
		return null;
		}
	}
else {
	// otherwise make sure there is a child
	if (segP->GetChild (nSide) < 0) {
		ErrorMsg ("This side must be attached to an other cube before a wall can be added.");
		return null;
		}
	}

if (sideP->GetWall () != null) {
	ErrorMsg ("There is already a wall on this side.");
	return null;
	}

if ((nWall = Count ()) >= MAX_WALLS) {
	ErrorMsg ("Maximum number of Walls () reached");
	return null;
	}

// link wall to segment/side
DLE.SetModified (TRUE);
DLE.LockUndo ();
sideP->m_info.nWall = nWall;
CWall* wallP = Walls (nWall);
wallP->(nSegment, nSide, nWall, (byte) type, nClip, nTexture, false);
wallP->m_info.flags = flags;
wallP->m_info.keys = keys;
// update number of Walls () in mine
MineInfo ().walls.count++;
DLE.UnlockUndo ();
DLE.MineView ()->Refresh ();
return wallP;
}

//--------------------------------------------------------------------------

bool CWallManager::GetOppositeWall (short& nOppWall, short nSegment, short nSide)
{
	short nOppSeg, nOppSide;

if (!GetOppositeSide (nOppSeg, nOppSide, nSegment, nSide))
	return false;
nOppWall = Segments (nOppSeg)->m_sides [nOppSide].m_info.nWall;
return true;
}

//------------------------------------------------------------------------------
// Mine - delete wall
//------------------------------------------------------------------------------

void CWallManager::Delete (ushort nWall) 
{
	short nTrigger;
	short nSegment, nSide, nOppSeg, nOppSide;
	CSegment *segP;
	CSide *sideP;

if (nWall < 0)
	nWall = current.Side ()->m_info.nWall;

CWall* delWallP = Walls (nWall);

if (delWallP == null)
	return;
// if trigger exists, remove it as well
triggerManager.Delete (delWallP->m_info.nTrigger);
DLE.SetModified (TRUE);
DLE.LockUndo ();
// remove references to the deleted wall
CSide* sideP = segmentManager.OppositeSide (delWallP->m_nSegment, delWallP->m_nSide);
if (sideP != null) {
	short nOppWall = Segments (nOppSeg)->m_sides [nOppSide].m_info.nWall;
	if ((nOppWall >= 0) && (nOppWall < MineInfo ().walls.count))
		Walls (nOppWall)->m_info.linkedWall = -1;
	}
// update all Segments () that point to Walls () higher than deleted one
// and unlink all Segments () that point to deleted wall
for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++)
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++)
		if (sideP->m_info.nWall >= MineInfo ().walls.count)
			sideP->m_info.nWall = NO_WALL;
		else if (sideP->m_info.nWall > nWall)
			sideP->m_info.nWall--;
		else if (sideP->m_info.nWall == nWall) {
			sideP->m_info.nWall = NO_WALL;
			DeleteTriggerTargets (nSegment, nSide); //delete this wall from all Triggers () that target it
			}
// move remaining Walls () in place of deleted wall
// for (i = nWall; i < MineInfo ().walls.count - 1; i++)
if (nWall < --MineInfo ().walls.count)
	memcpy (delWallP, Walls (nWall + 1), (MineInfo ().walls.count - nWall) * sizeof (CWall));
// update number of Walls () in mine
DLE.UnlockUndo ();
DLE.MineView ()->Refresh ();
LinkExitToReactor();
}

//------------------------------------------------------------------------------

int CWallManager::FindClip (CWall *wallP, short nTexture)
{
	char *ps, *pszName = textureManager.Name (nTexture);

if (!strcmp (pszName, "wall01 - anim"))
	return wallP->m_info.nClip = 0;
if (ps = strstr (pszName, "door")) {
	int i, nDoor = atol (ps + 4);
	for (i = 1; i < NUM_OF_CLIPS_D2; i++)
		if (nDoor == doorClipTable [i]) {
			wallP->m_info.nClip = clipList [i];
			DLE.SetModified (TRUE);
			DLE.MineView ()->Refresh ();
			return i;
			}
	}
return -1;
}

//------------------------------------------------------------------------------

CWall *CWallManager::FindSide (CSideKey key, int i = 0)
{
for (; i < m_nWalls; i++)
	if (m_walls [i] == key)
		return &m_walls [i];
return null;
}

//------------------------------------------------------------------------------

CWall *CWallManager::FindTrigger (short nTrigger, int i = 0)
{
for (; i < m_nWalls; i++)
	if (m_walls [i].m_nTrigger == nTrigger)
		return &m_walls [i];
return null;
}

//------------------------------------------------------------------------------

bool CWallManager::ClipFromTexture (short nSegment, short nSide)
{
CWall *wallP = segmentManager.Wall (nSegment, nSide);

if (!wallP || ((wallP->m_info.type != WALL_DOOR) && (wallP->m_info.type != WALL_BLASTABLE)))
	return true;

short nBaseTex, nOvlTex;

segmentManager.GetTextures (nSegment, nSide, nBaseTex, nOvlTex);

return (FindClip (wallP, nOvlTex) >= 0) || (FindClip (wallP, nBaseTex) >= 0);
}

//------------------------------------------------------------------------
// CheckForDoor()
//------------------------------------------------------------------------

void CWallManager::CheckForDoor (short nSegment, short nSide) 
{
if (!bExpertMode) {
	CWall* wallP = segmentManager.GetWall (nSegment, nSide);
	if ((wallP != null) && wallP->IsDoor ())
		ErrorMsg ("Changing the texture of a door only affects\n"
					"how the door will look before it is opened.\n"
					"You can use this trick to hide a door\n"
					"until it is used for the first time.\n\n"
					"Hint: To change the door animation,\n"
					"select \"Wall edit...\" from the Tools\n"
					"menu and change the clip number.");
	}
}

// ------------------------------------------------------------------------


//eof wall.cpp