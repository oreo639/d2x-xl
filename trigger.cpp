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
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "cfile.h"

//------------------------------------------------------------------------
// Mine - add_trigger()
//
// Returns - TRUE on success
//
// Changes - now takes type instead of flag.
//------------------------------------------------------------------------------

void CTrigger::Setup (short type, short flags)
{
m_info.type = (char) type;
m_info.flags = (char) flags;
if (type == TT_SPEEDBOOST)
	m_info.value = 10;
else if ((type == TT_CHANGE_TEXTURE) || (type == TT_MASTER))
	m_info.value = 0;
else if ((type == TT_MESSAGE) || (type == TT_SOUND))
	m_info.value = 1;
else 	
	m_info.value = 5 * F1_0; // 5% shield or energy damage
m_info.time = -1;
m_count = 0;
for (int i = 0; i < MAX_TRIGGER_TARGETS; i++)
	m_targets [i].Clear ();
}

//------------------------------------------------------------------------

int CMine::QCmpObjTriggers (CTrigger *pi, CTrigger *pm)
{
	short i = pi->m_info.nObject;
	short m = pm->m_info.nObject;

if (i < m)
	return -1;
if (i > m)
	return 1;
i = pi->m_info.type;
m = pm->m_info.type;
return (i < m) ? -1 : (i > m) ? 1 : 0;
}


void CMine::QSortObjTriggers (short left, short right)
{
	CTrigger median = *ObjTriggers ((left + right) / 2);
	short	l = left, r = right;

do {
	while (QCmpObjTriggers (ObjTriggers (l), &median) < 0)
		l++;
	while (QCmpObjTriggers (ObjTriggers (r), &median) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			CTrigger o = *ObjTriggers (l);
			*ObjTriggers (l) = *ObjTriggers (r);
			*ObjTriggers (r) = o;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortObjTriggers (l, right);
if (left < r)
	QSortObjTriggers (left, r);
}


void CMine::SortObjTriggers (void)
{
	int	h;

if ((h = NumObjTriggers ()) > 1) {
	for (ushort i = 0; i < h; i++)
		ObjTriggers (i)->m_info.nIndex = i;
	QSortObjTriggers (0, h - 1);
	}
}

//------------------------------------------------------------------------

CTrigger *CMine::AddTrigger (ushort nWall, short type, BOOL bAutoAddWall) 
{
	short flags;
	short nSegment, nSide, nTrigger;
	static short defWallTypes [NUM_TRIGGER_TYPES] = {
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_ILLUSION, 
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, 
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_ILLUSION,
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN,
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN,
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN,
		WALL_OPEN, WALL_OPEN, WALL_OPEN, WALL_OPEN
		};
	static short defWallTextures [NUM_TRIGGER_TYPES] = {
		0, 0, 0, 0, 426, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 426,
		0, 0
		};

// check if there's already a trigger on the current side
nWall = FindTriggerWall (&nTrigger);
if (nTrigger != NO_TRIGGER) {
	ErrorMsg ("There is already a trigger on this side");
	return null;
	}
if (MineInfo ().triggers.count >= MAX_TRIGGERS) {
	ErrorMsg ("The maximum number of triggers has been reached.");
	return null;
	}
// if no wall at current side, try to add a wall of proper type
bool bUndo = DLE.SetModified (TRUE);
DLE.LockUndo ();
if (CurrSide ()->m_info.nWall >= MineInfo ().walls.count) {
	if (bAutoAddWall) {
		if (MineInfo ().walls.count >= MAX_WALLS) {
			ErrorMsg ("Cannot add a wall to this side,\nsince the maximum number of walls is already reached.");
			return null;
			}
		nSegment = nSide = -1;
		GetCurrent (nSegment, nSide);
		if (!AddWall (-1, -1, (Segments (nSegment)->Child (nSide) < 0) ? WALL_OVERLAY : defWallTypes [type], 0, 0, -1, defWallTextures [type])) {
			ErrorMsg ("Cannot add a wall for this trigger.");
			DLE.ResetModified (bUndo);
			return null;
			}
		}
	else {
		ErrorMsg ("You must add a wall to this side before you can add a trigger.");
		return null;
		}
	}
// if D1, then convert type to flag value
if (IsD1File ()) {
	switch(type) {
		case TT_OPEN_DOOR:
			flags = TRIGGER_CONTROL_DOORS;
			break;
		case TT_MATCEN:
			flags = TRIGGER_MATCEN;
			break;
		case TT_EXIT:
			flags = TRIGGER_EXIT;
			break;
		case TT_SECRET_EXIT:
			flags = TRIGGER_SECRET_EXIT;
			break;
		case TT_ILLUSION_OFF:
			flags = TRIGGER_ILLUSION_OFF;
			break;
		case TT_ILLUSION_ON:
			flags = TRIGGER_ILLUSION_ON;
			break;
		case TT_ENERGY_DRAIN:
			flags = TRIGGER_ENERGY_DRAIN;
			break;
		case TT_SHIELD_DAMAGE:
			flags = TRIGGER_SHIELD_DAMAGE;
			break;
		default:
			flags = 0;
		}
	type = 0;
	}
else
	flags = 0;

nTrigger = (ushort) MineInfo ().triggers.count;
// set new trigger data
Triggers (nTrigger)->Setup (type, flags);
// link trigger to the wall
Walls (nWall)->m_info.nTrigger = (byte) nTrigger;
// update number of Triggers ()
MineInfo ().triggers.count++;
AutoLinkExitToReactor();
DLE.UnlockUndo ();
DLE.MineView ()->Refresh ();
return Triggers (nTrigger);
}

//------------------------------------------------------------------------
// Mine - DeleteTrigger
//------------------------------------------------------------------------

void CMine::DeleteTrigger (short nTrigger) 
{
	short	i, nSegment, nSide, nWall;

if (nTrigger < 0) {
	nWall = CurrSeg ()->m_sides [Current ()->nSide].m_info.nWall;
	if (nWall >= MineInfo ().walls.count)
		return;
	nTrigger = Walls (nWall)->m_info.nTrigger;
	}
if (nTrigger >= MineInfo ().triggers.count)
	return;
// update all Walls () who point to Triggers () higher than this one
// and unlink all Walls () who point to deleted trigger (should be only one wall)
DLE.SetModified (TRUE);
DLE.LockUndo ();
CWall *wallP = Walls (0);
for (i = MineInfo ().walls.count; i; i--, wallP++)
	if ((wallP->m_info.nTrigger != NO_TRIGGER) && (wallP->m_info.nTrigger > nTrigger))
		wallP->m_info.nTrigger--;
	else if (wallP->m_info.nTrigger == nTrigger) {
		wallP->m_info.nTrigger = NO_TRIGGER;
		nSegment = wallP->m_nSegment;
		nSide = wallP->m_nSide;
		}
// remove trigger from array
//for (i=nTrigger;i<MineInfo ().triggers.count-1;i++)
// update number of Triggers ()
CTrigger *trigP = Triggers (0);
for (i = NumTriggers (); i; i--, trigP++)
	if (trigP->m_info.type >= TT_MASTER)
		DeleteTriggerTarget (trigP, nSegment, nSide, false);
if (nTrigger < --MineInfo ().triggers.count)
	memcpy(Triggers (nTrigger), Triggers (nTrigger + 1), (MineInfo ().triggers.count - nTrigger) * sizeof (CTrigger));
DLE.UnlockUndo ();
DLE.MineView ()->Refresh ();
AutoLinkExitToReactor();
}

//------------------------------------------------------------------------
// Mine - DeleteTrigger
//------------------------------------------------------------------------

int CMine::DeleteTargetFromTrigger (CTrigger *trigger, short linknum, bool bAutoDeleteTrigger)
{
if (!--trigger->m_count) {
	if (bAutoDeleteTrigger)
		DeleteTrigger ();
	return 0;
	}
if (linknum < trigger->m_count) {
	memcpy (trigger->m_targets + linknum, trigger->m_targets + linknum + 1, (trigger->m_count - linknum) * sizeof (trigger [0]));
	}
return trigger->m_count;
}


int CMine::DeleteTargetFromTrigger (short nTrigger, short linknum, bool bAutoDeleteTrigger)
{
return DeleteTargetFromTrigger (Triggers (nTrigger), linknum, bAutoDeleteTrigger);
}


bool CMine::DeleteTriggerTarget (CTrigger* trigP, short nSegment, short nSide, bool bAutoDeleteTrigger) 
{
int j;
for (j = 0; j < trigP->m_count; j++)
	if ((trigP->m_targets [j] == CSideKey (nSegment, nSide)))
		return DeleteTargetFromTrigger (trigP, j, bAutoDeleteTrigger) == 0;
return false;
}


void CMine::DeleteTriggerTargets (short nSegment, short nSide) 
{
int i;

for (i = 0; i < MineInfo ().triggers.count; i++)
	if (DeleteTriggerTarget (Triggers (i), nSegment, nSide))
		i--;

for (i = 0; i < NumObjTriggers (); i++)
	if (DeleteTriggerTarget (ObjTriggers (i), nSegment, nSide, false)) {
		DeleteObjTrigger (i);
		i--;
		}
}

//------------------------------------------------------------------------
// Mine - FindTrigger
//------------------------------------------------------------------------

short CMine::FindTriggerWall (short *nTrigger, short nSegment, short nSide)
{
GetCurrent (nSegment, nSide);
CWall *wallP = Walls (0);
int nWall;
for (nWall = MineInfo ().walls.count; nWall; nWall--, wallP++) {
	if ((wallP->m_nSegment == nSegment) && (wallP->m_nSide == nSide)) {
		*nTrigger = wallP->m_info.nTrigger;
		return short (wallP - Walls (0));
		}
	}
*nTrigger = NO_TRIGGER;
return MineInfo ().walls.count;
}

short CMine::FindTriggerWall (short nTrigger)
{
CWall *wallP = Walls (0);
int nWall;
for (nWall = MineInfo ().walls.count; nWall; nWall--, wallP++)
	if (wallP->m_info.nTrigger == nTrigger)
		return short (wallP - Walls (0));
return MineInfo ().walls.count;
}

short CMine::FindTriggerObject (short *nTrigger)
{
	short nObject = Current ()->nObject;

for (int i = 0; i < NumObjTriggers (); i++)
	if (ObjTriggers (i)->m_info.nObject == nObject) {
		*nTrigger = i;
		return nObject;
		}
*nTrigger = NO_TRIGGER;
return -1;
}

//------------------------------------------------------------------------
// Mine - FindTrigger
//------------------------------------------------------------------------

short CMine::FindTriggerTarget (short nTrigger, short nSegment, short nSide)
{
	CTrigger *trigP = Triggers (0);
	CSideKey key = CSideKey (nSegment, nSide);
	int i, j;

for (i = nTrigger; i < MineInfo ().triggers.count; i++, trigP++)
	if (-1 < (j = trigP->Find (key)))
		return i;
return -1;
}

//------------------------------------------------------------------------
// AutoLinkExitToReactor()
//
// Action - Updates control center Triggers () so that exit door opens
//          when the reactor blows up.  Removes any invalid cube/sides
//          from reactorTriggers if they exist.
//------------------------------------------------------------------------

void CMine::AutoLinkExitToReactor () 
{
  short 		linknum,control,count;
  CSideKey	face;
  ushort		nWall;
  char 		nTrigger;
  bool 		found;

  control = 0; // only 0 used by the game Descent
  CReactorTrigger *reactorTrigger = ReactorTriggers (control);

DLE.SetModified (TRUE);
DLE.LockUndo ();
// remove items from list that do not point to a wall
for (linknum = 0; linknum < reactorTrigger->m_count; linknum++) {
	count = reactorTrigger->m_count;
	face = reactorTrigger->m_targets [linknum];
	// search for Walls () that have a exit of type trigger
	found = FALSE;
	for (nWall = 0; nWall < MineInfo ().walls.count; nWall++) {
		if (*Walls (nWall) == face) {
		found = TRUE;
		break;
		}
	}
	if (!found) {
		reactorTrigger->Delete (linknum);
		}
	}

// add exit to list if not already in list
// search for Walls () that have a exit of type trigger
count =  reactorTrigger->m_count;
for (nWall = 0; nWall < MineInfo ().walls.count; nWall++) {
	nTrigger = Walls (nWall)->m_info.nTrigger;
	if (nTrigger >= 0 && nTrigger <MineInfo ().triggers.count) {
		if (IsD1File () 
			 ? Triggers (nTrigger)->m_info.flags & (TRIGGER_EXIT | TRIGGER_SECRET_EXIT) 
			 : Triggers (nTrigger)->m_info.type == TT_EXIT || Triggers (nTrigger)->m_info.type == TT_SECRET_EXIT) {
			// see if cube,side is already on the list
			face = *Walls (nWall);
			found = FALSE;
			for (linknum = 0; linknum < count; linknum++) {
				if (face == reactorTrigger->m_targets [linknum]) {
					found = TRUE;
					break;
					}
				}
			// if not already on the list, add it
			if (!found) {
				linknum = reactorTrigger->Add (face);
				}
			}
		}
	}
DLE.UnlockUndo ();
}

//------------------------------------------------------------------------

CTrigger *CMine::AddObjTrigger (short objnum, short type) 
{
if (objnum < 0)
	objnum = Current ()->nObject;
if ((Objects (objnum)->m_info.type != OBJ_ROBOT) && 
	 (Objects (objnum)->m_info.type != OBJ_CAMBOT) &&
	 (Objects (objnum)->m_info.type != OBJ_POWERUP) &&
	 (Objects (objnum)->m_info.type != OBJ_HOSTAGE) &&
	 (Objects (objnum)->m_info.type != OBJ_CNTRLCEN)) {
	ErrorMsg ("Object triggers can only be attached to robots, reactors, hostages, powerups and cameras.");
	return null;
	}
if (NumObjTriggers () >= MAX_OBJ_TRIGGERS) {
	ErrorMsg ("The maximum number of object triggers has been reached.");
	return null;
	}
bool bUndo = DLE.SetModified (TRUE);
DLE.LockUndo ();
short nTrigger = NumObjTriggers ();
ObjTriggers (nTrigger)->Setup (type, 0);
ObjTriggers (nTrigger)->m_info.nObject = objnum;
NumObjTriggers ()++;
DLE.UnlockUndo ();
SortObjTriggers ();
for (ushort i = NumObjTriggers (); i; )
	if (ObjTriggers (--i)->m_info.nIndex == nTrigger)
		return ObjTriggers (i);
return ObjTriggers (nTrigger);
}

//------------------------------------------------------------------------

bool CMine::ObjTriggerIsInList (short nTrigger)
{
return true;
}

//------------------------------------------------------------------------

void CMine::DeleteObjTrigger (short nTrigger) 
{
if ((nTrigger < 0) || (nTrigger >= NumObjTriggers ()))
	return;
if (nTrigger < --NumObjTriggers ())
	*ObjTriggers (nTrigger) = *ObjTriggers (NumObjTriggers ());
}

//------------------------------------------------------------------------

void CMine::DeleteObjTriggers (short objnum) 
{
	short i = NumObjTriggers ();
	
while (i)
	if (ObjTriggers (--i)->m_info.nObject == objnum)
		DeleteObjTrigger (i);
}

//------------------------------------------------------------------------

short CMine::FindObjTriggerTarget (short nTrigger, short nSegment, short nSide)
{
CTrigger *trigP = ObjTriggers (0);
CSideKey key = CSideKey (nSegment, nSide);
int i, j;

for (i = nTrigger; i < NumObjTriggers (); i++, trigP++)
	for (j = 0; j < trigP->m_count; j++)
		if (-1 < (i = trigP->Find (key)))
			return i;
return -1;
}

// ------------------------------------------------------------------------

int CTrigger::Read (CFileManager& fp, int version, bool bObjTrigger)
{
if (DLE.IsD2File ()) {
	m_info.type = fp.ReadByte ();
	m_info.flags = bObjTrigger ? fp.ReadInt16 () : (ushort) fp.ReadByte ();
	m_count = fp.ReadByte ();
	fp.ReadByte ();
	m_info.value = fp.ReadFix ();
	if ((DLE.LevelVersion () < 21) && (m_info.type == TT_EXIT))
		m_info.value = 0;
	if ((version < 39) && (m_info.type == TT_MASTER))
		m_info.value = 0;
	m_info.time = fp.ReadFix ();
	}
else {
	m_info.type = fp.ReadByte ();
	m_info.flags = fp.ReadInt16 ();
	m_info.value = fp.ReadFix ();
	m_info.time = fp.ReadFix ();
	fp.ReadByte (); //skip 8 bit value "link_num"
	m_count = char (fp.ReadInt16 ());
	if (m_count < 0)
		m_count = 0;
	else if (m_count > MAX_TRIGGER_TARGETS)
		m_count = MAX_TRIGGER_TARGETS;
	}
ReadTargets (fp);
//int	i;
//for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
//	m_targets [i].m_nSegment = fp.ReadInt16 ();
//for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
//	m_targets [i].m_nSide = fp.ReadInt16 ();
return 1;
}

// ------------------------------------------------------------------------

void CTrigger::Write (CFileManager& fp, int version, bool bObjTrigger)
{
if (DLE.IsD2File ()) {
	fp.Write (m_info.type);
	if (bObjTrigger)
		fp.Write (m_info.flags);
	else
		fp.WriteSByte ((sbyte) m_info.flags);
	fp.WriteSByte ((sbyte) m_count);
	fp.WriteByte (0);
	fp.Write (m_info.value);
	fp.Write (m_info.time);
	}
else {
	fp.Write (m_info.type);
	fp.Write (m_info.flags);
	fp.Write (m_info.value);
	fp.Write (m_info.time);
	fp.WriteSByte ((sbyte) m_count);
	fp.Write (m_count);
	}
WriteTargets (fp);
//int	i;
//for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
//	WriteInt16 (m_targets [i].m_nSegment, fp);
//for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
//	WriteInt16 (m_targets [i].m_nSide, fp);
}

//------------------------------------------------------------------------

int CReactorTrigger::Read (CFileManager& fp, int version, bool bFlag)
{
	int	i;

m_count = char (fp.ReadInt16 ());
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	m_targets [i].m_nSegment = fp.ReadInt16 ();
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	m_targets [i].m_nSide = fp.ReadInt16 ();
return 1;
}

//------------------------------------------------------------------------

void CReactorTrigger::Write (CFileManager& fp, int version, bool bFlag)
{
	int	i;

fp.Write (m_count);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	fp.Write (m_targets [i].m_nSegment);
for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
	fp.Write (m_targets [i].m_nSide);
}

//------------------------------------------------------------------------
//eof trigger.cpp