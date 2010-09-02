#include "triggermanager.h"

CTriggerManager triggerManager;

//------------------------------------------------------------------------

int CTriggerManager::CmpObjTriggers (CTrigger *pi, CTrigger *pm)
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

//------------------------------------------------------------------------

void CTriggerManager::SortObjTriggers (short left, short right)
{
	CTrigger median = *ObjTriggers ((left + right) / 2);
	short	l = left, r = right;

do {
	while (QCmpObjTriggers (ObjTriggers (l), &median) < 0)
		l++;
	while (QCmpObjTriggers (ObjTriggers (r), &median) > 0)
		r--;
	if (l <= r) {
		if (l < r)
			Swap (*ObjTriggers (l), *ObjTriggers (r));
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

//------------------------------------------------------------------------

void CTriggerManager::SortObjTriggers (void)
{
	int	h = NumObjTriggers ();

if (h > 1) {
	for (ushort i = 0; i < h; i++)
		ObjTriggers (i)->m_info.nIndex = i;
	SortObjTriggers (0, h - 1);
	}
}

//------------------------------------------------------------------------

CTrigger *CTriggerManager::Add (ushort nWall, short type, bool bAddWall) 
{
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
CSegment* segP = segmentManager.GetSegment ();
CSide* sideP = segmentManager.GetSide ();
CWall* wallP = sideP->GetWall ();
if (sideP->GetWall ()) {
	ErrorMsg ("There is already a trigger on this side");
	return null;
	}
if (Count () >= MAX_TRIGGERS) {
	ErrorMsg ("The maximum number of triggers has been reached.");
	return null;
	}
// if no wall at current side, try to add a wall of proper type
bool bUndo = DLE.SetModified (TRUE);
DLE.LockUndo ();
if (current.Side ()->m_info.nWall >= MineInfo ().walls.count) {
	if (bAddWall) {
		if (MineInfo ().walls.count >= MAX_WALLS) {
			ErrorMsg ("Cannot add a wall to this side,\nsince the maximum number of walls is already reached.");
			return null;
			}
		nSegment = nSide = -1;
		current.Get (nSegment, nSide);
		if (!wallManager.Add (-1, -1, (Segments (nSegment)->GetChild (nSide) < 0) ? WALL_OVERLAY : defWallTypes [type], 0, 0, -1, defWallTextures [type])) {
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

void CTriggerManager::DeleteTrigger (short nTrigger) 
{
	short	i, nSegment, nSide, nWall;

if (nTrigger < 0) {
	nWall = current.Segment ()->m_sides [Current ()->nSide].m_info.nWall;
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

int CTriggerManager::DeleteTargetFromTrigger (CTrigger *trigger, short linknum, bool bAutoDeleteTrigger)
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


int CTriggerManager::DeleteTargetFromTrigger (short nTrigger, short linknum, bool bAutoDeleteTrigger)
{
return DeleteTargetFromTrigger (Triggers (nTrigger), linknum, bAutoDeleteTrigger);
}


bool CTriggerManager::DeleteTriggerTarget (CTrigger* trigP, short nSegment, short nSide, bool bAutoDeleteTrigger) 
{
int j;
for (j = 0; j < trigP->m_count; j++)
	if ((trigP->m_targets [j] == CSideKey (nSegment, nSide)))
		return DeleteTargetFromTrigger (trigP, j, bAutoDeleteTrigger) == 0;
return false;
}


void CTriggerManager::DeleteTriggerTargets (short nSegment, short nSide) 
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

short CTriggerManager::FindWall (short& nTrigger, short nSegment, short nSide)
{
current.Get (nSegment, nSide);
CWall *wallP = Walls (0);
int nWall;
for (nWall = MineInfo ().walls.count; nWall; nWall--, wallP++) {
	if ((wallP->m_nSegment == nSegment) && (wallP->m_nSide == nSide)) {
		nTrigger = wallP->m_info.nTrigger;
		return short (wallP - Walls (0));
		}
	}
nTrigger = NO_TRIGGER;
return MineInfo ().walls.count;
}

//------------------------------------------------------------------------

short CTriggerManager::FindTriggerWall (short nTrigger)
{
CWall *wallP = Walls (0);
int nWall;
for (nWall = MineInfo ().walls.count; nWall; nWall--, wallP++)
	if (wallP->m_info.nTrigger == nTrigger)
		return short (wallP - Walls (0));
return MineInfo ().walls.count;
}

//------------------------------------------------------------------------

short CTriggerManager::FindTriggerObject (short& nTrigger)
{
	short nObject = Current ()->nObject;

for (int i = 0; i < NumObjTriggers (); i++)
	if (ObjTriggers (i)->m_info.nObject == nObject) {
		nTrigger = i;
		return nObject;
		}
nTrigger = NO_TRIGGER;
return -1;
}

//------------------------------------------------------------------------
// Mine - FindTrigger
//------------------------------------------------------------------------

short CTriggerManager::FindTarget (short nTrigger, short nSegment, short nSide)
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

void CTriggerManager::LinkExitToReactor (void) 
{
  short 		nTarget;
  CSideKey	key;
  ushort		nWall;
  char 		nTrigger;
  bool 		found;

  CReacto1rTrigger *reactorTrigger = ReactorTriggers (0);	// only one reactor trigger per level

DLE.SetModified (TRUE);
DLE.LockUndo ();
// remove items from list that do not point to a wall
for (nTarget = 0; nTarget < reactorTrigger->m_count; nTarget++) {
	if (!wallManager.Find (reactorTrigger->m_targets [nTarget]))
		reactorTrigger->Delete (nTarget);
	}
// add any exits to target list that are not already in it
for (nWall = 0; nWall < wallManager.Count (); nWall++) {
	CTrigger* trigP = Triggers (Walls (nWall)->m_info.nTrigger);
	if (trigP == null)
		continue;
	if (!trigP->IsExit ())
		continue;
	if (reactorTrigger->Find (*Walls (nWall)))
		continue;
	nTarget = reactorTrigger->Add (face);
	}
DLE.UnlockUndo ();
}

//------------------------------------------------------------------------

CTrigger *CTriggerManager::AddObjTrigger (short nObject, short type) 
{
if (nObject < 0)
	nObject = Current ()->nObject;
if ((Objects (nObject)->m_info.type != OBJ_ROBOT) && 
	 (Objects (nObject)->m_info.type != OBJ_CAMBOT) &&
	 (Objects (nObject)->m_info.type != OBJ_POWERUP) &&
	 (Objects (nObject)->m_info.type != OBJ_HOSTAGE) &&
	 (Objects (nObject)->m_info.type != OBJ_CNTRLCEN)) {
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
ObjTriggers (nTrigger)->m_info.nObject = nObject;
NumObjTriggers ()++;
DLE.UnlockUndo ();
SortObjTriggers ();
for (ushort i = NumObjTriggers (); i; )
	if (ObjTriggers (--i)->m_info.nIndex == nTrigger)
		return ObjTriggers (i);
return ObjTriggers (nTrigger);
}

//------------------------------------------------------------------------

bool CTriggerManager::ObjTriggerIsInList (short nTrigger)
{
return true;
}

//------------------------------------------------------------------------

void CTriggerManager::DeleteObjTrigger (short nTrigger) 
{
if ((nTrigger < 0) || (nTrigger >= NumObjTriggers ()))
	return;
if (nTrigger < --NumObjTriggers ())
	*ObjTriggers (nTrigger) = *ObjTriggers (NumObjTriggers ());
}

//------------------------------------------------------------------------

void CTriggerManager::DeleteObjTriggers (short objnum) 
{
	short i = NumObjTriggers ();
	
while (i)
	if (ObjTriggers (--i)->m_info.nObject == objnum)
		DeleteObjTrigger (i);
}

//------------------------------------------------------------------------

short CTriggerManager::FindObjTriggerTarget (short nTrigger, short nSegment, short nSide)
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
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

void CTriggerManager::DeleteTargets (triggerList triggers, short nTriggers, short nSegment, short nSide) 
{
int i;

for (i = 0; nTriggers; i++)
	if (DeleteTriggerTarget (Triggers (i), nSegment, nSide))
		i--;

for (i = 0; i < NumObjTriggers (); i++)
	if (DeleteTriggerTarget (ObjTriggers (i), nSegment, nSide, false)) {
		DeleteObjTrigger (i);
		i--;
		}
}

