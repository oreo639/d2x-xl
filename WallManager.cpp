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
CWall* wallP = GetWall (nWall);
wallP->Setup (nSegment, nSide, nWall, (byte) type, nClip, nTexture, false);
wallP->m_info.flags = flags;
wallP->m_info.keys = keys;
// update number of Walls () in mine
Count ()++;
undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
return wallP;
}

//--------------------------------------------------------------------------

CWall* CWallManager::GetOppositeWall (short nSegment, short nSide)
{
CSide* sideP = segmentManager.GetOppositeSide (nSegment, nSide);
return (sideP == null) ? null : sideP->GetWall ();
}

//------------------------------------------------------------------------------
// Mine - delete wall
//------------------------------------------------------------------------------

void CWallManager::Delete (short nDelWall) 
{
CWall* delWallP = GetWall (nDelWall);
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
CWall* oppWallP = segmentManager.GetOppositeWall (delWallP->m_nSegment, delWallP->m_nSide);
if (oppWallP != null) 
	oppWallP->m_info.linkedWall = -1;

triggerManager.DeleteTargets (delWallP->m_nSegment, delWallP->m_nSide);
segmentManager.GetSide (delWallP->m_nSegment, delWallP->m_nSide)->SetWall (NO_WALL);
if (nDelWall < --Count ()) { // move last wall in list to position of deleted wall
	CWall* lastWallP = m_walls + m_nCount;
	segmentManager.GetSide (lastWallP->m_nSegment, lastWallP->m_nSide)->SetWall (nDelWall); // make last wall's side point to new wall position
	*delWallP = *lastWallP;
	}

undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
triggerManager.UpdateReactor ();
}

//------------------------------------------------------------------------------

CWall *CWallManager::FindBySide (CSideKey key, int i)
{
for (; i < m_nCount; i++)
	if (m_walls [i] == key)
		return &m_walls [i];
return null;
}

//------------------------------------------------------------------------------

CWall* CWallManager::FindByTrigger (short nTrigger, int i)
{
for (; i < m_nCount; i++)
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
CWall *wallP = segmentManager.GetWall (nSegment, nSide);

if (!(wallP && wallP->IsDoor ()))
	return true;

short nBaseTex, nOvlTex;

segmentManager.GetTextures (nSegment, nSide, nBaseTex, nOvlTex);

return (wallP->SetClip (nOvlTex) >= 0) || (wallP->SetClip (nBaseTex) >= 0);
}

//------------------------------------------------------------------------------
//eof wallmanager.cpp