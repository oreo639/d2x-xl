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

CWall* CWallManager::Add (short nSegment, short nSide, short type, ushort flags, byte keys, char nClip, short nTexture) 
{
current.Get (nSegment, nSide);

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

ushort nWall = Count ();

if (nWall >= MAX_WALLS) {
	ErrorMsg ("Maximum number of Walls () reached");
	return null;
	}

// link wall to segment/side
undoManager.SetModified (TRUE);
undoManager.Lock ();
sideP->SetWall (nWall);
CWall* wallP = Walls (nWall);
wallP->(nSegment, nSide, nWall, (byte) type, nClip, nTexture, false);
wallP->m_info.flags = flags;
wallP->m_info.keys = keys;
// update number of Walls () in mine
MineInfo ().walls.count++;
undoManager.Unlock ();
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

void CWallManager::Delete (short nDelWall) 
{
	short nTrigger;
	short nSegment, nSide, nOppSeg, nOppSide;
	CSegment *segP;
	CSide *sideP;

CWall* delWallP = Walls (nDelWall);
if (delWallP == null) {
	delWallP = current.Wall ();
	nDelWall = Index (delWallP);
	}

if (delWallP == null)
	return;
// if trigger exists, remove it as well
triggerManager.Delete (delWallP->m_info.nTrigger);
undoManager.SetModified (TRUE);
undoManager.Lock ();
// remove references to the deleted wall
CWall* oppWallP = segmentManager.OppositeWall (delWallP->m_nSegment, delWallP->m_nSide);
if (oppWallP != null) 
	oppWallP->m_info.linkedWall = -1;

triggerManager.DeleteTargets (delWallP->m_nSegment, delWallP->m_nSide);
segmentManager.GetSide (delWallP->m_nSegment, delWallP->m_nSide)->SetWall (NO_WALL);
if (nDelWall < --Count ()) {
	CWall* moveWallP = m_walls + m_nCount;
	segmentManager.GetSide (moveWallP->m_nSegment, moveWallP->m_nSide)->SetWall (nDelWall);
	*delWallP = *moveWallP;
	}

for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++)
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++)
		if (sideP->m_info.nDelWall >= MineInfo ().walls.count)
			sideP->m_info.nDelWall = NO_WALL;
		else if (sideP->m_info.nDelWall > nDelWall)
			sideP->m_info.nDelWall--;
		else if (sideP->m_info.nDelWall == nDelWall) {
			sideP->m_info.nDelWall = NO_WALL;
			DeleteTriggerTargets (nSegment, nSide); //delete this wall from all Triggers () that target it
			}
// move remaining Walls () in place of deleted wall
// for (i = nDelWall; i < MineInfo ().walls.count - 1; i++)
	memcpy (delWallP, Walls (nDelWall + 1), (MineInfo ().walls.count - nDelWall) * sizeof (CWall));
// update number of Walls () in mine
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
LinkExitToReactor();
}

//------------------------------------------------------------------------------

CWall *CWallManager::FindBySide (CSideKey key, int i = 0)
{
for (; i < m_nCount; i++)
	if (m_walls [i] == key)
		return &m_walls [i];
return null;
}

//------------------------------------------------------------------------------

CWall* CWallManager::FindByTrigger (short nTrigger, int i = 0)
{
for (; i < m_nCount; i++)
	if (m_walls [i].m_nTrigger == nTrigger)
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
CWall *wallP = segmentManager.Wall (nSegment, nSide);

if (!(wallP && wallP->IsDoor ()))
	return true;

short nBaseTex, nOvlTex;

segmentManager.GetTextures (nSegment, nSide, nBaseTex, nOvlTex);

return (wallP->SetClip (nOvlTex) >= 0) || (wallP->SetClip (nBaseTex) >= 0);
}

//------------------------------------------------------------------------------
// CheckForDoor()
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
//eof wallmanager.cpp