#include "SegmentManager.h"
#include "TriggerManager.h"
#include "UndoManager.h"

CTriggerManager triggerManager;

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

void CTriggerManager::SortObjTriggers (short left, short right)
{
	CTrigger median = *ObjTriggers ((left + right) / 2);
	short	l = left, r = right;

do {
	while (CmpObjTriggers (ObjTriggers (l), &median) < 0)
		l++;
	while (CmpObjTriggers (ObjTriggers (r), &median) > 0)
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
	SortObjTriggers (l, right);
if (left < r)
	SortObjTriggers (left, r);
}

//------------------------------------------------------------------------------

void CTriggerManager::SortObjTriggers (void)
{
	int	h = NumObjTriggers ();

if (h > 1) {
	for (ushort i = 0; i < h; i++)
		ObjTriggers (i)->m_info.nIndex = i;
	SortObjTriggers (0, h - 1);
	}
}

//------------------------------------------------------------------------------

CTrigger* CTriggerManager::AddToWall (short nWall, short type, bool bAddWall) 
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
if (segmentManager.GetTrigger () != null) {
	ErrorMsg ("There is already a trigger on this side");
	return null;
	}
if (Count (0) >= MAX_TRIGGERS) {
	ErrorMsg ("The maximum number of triggers has been reached.");
	return null;
	}
// if no wall at current side, try to add a wall of proper type
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();

CWall* wallP = current.Wall ();

if (wallP == null) {
	if (bAddWall) {
		if (MineInfo ().walls.count >= MAX_WALLS) {
			ErrorMsg ("Cannot add a wall to this side,\nsince the maximum number of walls is already reached.");
			return null;
			}
		wallP = wallManager.Add (-1, -1, (current.Segment ()->GetChild (nSide) < 0) ? WALL_OVERLAY : defWallTypes [type], 0, 0, -1, defWallTextures [type]);
		if (wallP == null) {
			ErrorMsg ("Cannot add a wall for this trigger.");
			undoManager.ResetModified (bUndo);
			return null;
			}
		}
	else {
		ErrorMsg ("You need to add a wall to this side before you can add a trigger.");
		return null;
		}
	}
// if D1, then convert type to flag value
ushort flags;
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

short nTrigger = Count (0)++;
CTrigger* trigP = Triggers (nTrigger);
// set new trigger data
trigP->Setup (type, flags);
// link trigger to the wall
wallP->SetTrigger (nTrigger);
// update number of Triggers ()
LinkExitToReactor();
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return Triggers (nTrigger);
}

//------------------------------------------------------------------------------
// Mine - DeleteTrigger
//------------------------------------------------------------------------------

void CTriggerManager::DeleteFromWall (short nDelTrigger) 
{
if (nDelTrigger == NO_TRIGGER)
	return;

if (nDelTrigger < 0) {
	CWall* wallP = current.Wall ();
	if (wallP == null)
		return;
	nDelTrigger = wallP->m_info.nTrigger;
	}

CTrigger* delTrigP = Triggers (nDelTrigger, 0);

if (delTrigP == null)
	return;
// update all Walls () who point to Triggers () higher than this one
// and unlink all Walls () who point to deleted trigger (should be only one wall)
undoManager.SetModified (TRUE);
undoManager.Lock ();
wallManager.UpdateTrigger (nDelTrigger, NO_TRIGGER);
if (nTrigger < --Count ()) {	
	// move the last trigger in the list to the deleted trigger's position
	wallManager.UpdateTrigger (m_nTriggers [0], nDelTrigger);
	*delTrigP = Trigger (m_nTriggers [0], 0);
	}

undoManager.Unlock ();
DLE.MineView ()->Refresh ();
AutoLinkExitToReactor();
}

//------------------------------------------------------------------------------
// Mine - DeleteTrigger
//------------------------------------------------------------------------------

void CTriggerManager::DeleteTarget (CSideKey key, short nClass) 
{
CTrigger* trigP = m_triggers [nClass];
for (int i = 0; i < m_nTriggers [nClass]; i++, trigP++)
	trigP->Delete (key);
}

//------------------------------------------------------------------------------
// Mine - FindTrigger
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------------
// Mine - FindTrigger
//------------------------------------------------------------------------------------

short CTriggerManager::FindTarget (short nTrigger, short nSegment, short nSide, short nClass = 0)
{
	CTrigger *trigP = Triggers (0);
	CSideKey key = CSideKey (nSegment, nSide);
	int i, j;

for (i = nTrigger; i < MineInfo ().triggers.count; i++, trigP++)
	if (-1 < (j = trigP->Find (key)))
		return i;
return -1;
}

//------------------------------------------------------------------------------------
// LinkExitToReactor()
//
// Action - Updates control center Triggers () so that exit door opens
//          when the reactor blows up.  Removes any invalid cube/sides
//          from reactorTriggers if they exist.
//------------------------------------------------------------------------------------

void CTriggerManager::LinkExitToReactor (void) 
{
  short 		nTarget;
  CSideKey	key;
  ushort		nWall;
  char 		nTrigger;
  bool 		found;

  CReactorTrigger *reactorTrigger = ReactorTriggers (0);	// only one reactor trigger per level

undoManager.SetModified (TRUE);
undoManager.Lock ();
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
undoManager.Unlock ();
}

//------------------------------------------------------------------------------------

CTrigger* CTriggerManager::AddToObject (short nObject, short type) 
{
	CObject* objP = (nObject < 0) ? current.Object () : objectManager.GetObject (nObject);

if (objP == null) {
	ErrorMsg ("Couldn't find object to attach triggers to.");
	return null;
	}

if ((objP->m_info.type != OBJ_ROBOT) && 
	  objP->m_info.type != OBJ_CAMBOT) &&
	  objP->m_info.type != OBJ_POWERUP) &&
	  objP->m_info.type != OBJ_HOSTAGE) &&
	  objP->m_info.type != OBJ_CNTRLCEN)) {
	ErrorMsg ("Object triggers can only be attached to robots, reactors, hostages, powerups and cameras.");
	return null;
	}

if (NumObjTriggers () >= MAX_OBJ_TRIGGERS) {
	ErrorMsg ("The maximum number of object triggers has been reached.");
	return null;
	}
bool bUndo = undoManager.SetModified (TRUE);
undoManager.Lock ();
short nTrigger = NumObjTriggers ();
ObjTriggers (nTrigger)->Setup (type, 0);
ObjTriggers (nTrigger)->m_info.nObject = nObject;
NumObjTriggers ()++;
undoManager.Unlock ();
SortObjTriggers ();
for (ushort i = NumObjTriggers (); i; )
	if (ObjTriggers (--i)->m_info.nIndex == nTrigger)
		return ObjTriggers (i);
return ObjTriggers (nTrigger);
}

//------------------------------------------------------------------------------

void CTriggerManager::DeleteFromObject (short nDelTrigger) 
{
if ((nDelTrigger < 0) || (nDelTrigger >= NumObjTriggers ()))
	return;
if (nDelTrigger < NumObjTriggers () - 1)
	*ObjTriggers (nDelTrigger) = *ObjTriggers (NumObjTriggers () - 1);
}

//------------------------------------------------------------------------------

void CTriggerManager::DeleteObjTriggers (short nObject) 
{
	short i = NumObjTriggers ();
	
while (i)
	if (ObjTriggers (--i)->m_info.nObject == nObject)
		DeleteObjTrigger (i);
}

//------------------------------------------------------------------------------

short CTriggerManager::FindObjTarget (short nTrigger, short nSegment, short nSide)
{
return FindTarget (nTrigger, nSegment, nSide, 1);
}

// -----------------------------------------------------------------------------

void CTriggerManager::DeleteTargets (triggerList triggers, short nTriggers, short nSegment, short nSide) 
{
CSideKey key (nSegment, nSide);
int i;

for (i = 0; nTriggers; i++)
	if (Triggers (i)->Delete (key))
		i--;

for (i = NumObjTriggers (); i > 0)
	if (ObjTriggers (--i)->Delete (key) == 0) // no targets left
		DeleteObjTrigger (i);
}

// -----------------------------------------------------------------------------

void Read (CFileManager& fp, CMineItemInfo& info)
{
if (info.nOffset < 0)
	return;
Count () = info.count;
for (short i = 0; i < info.count; i++)
	m_triggers [0][i].Read (fp);

int bObjTriggersOk = 1;
if (MineInfo ().fileInfo.version >= 33) {
	NumObjTriggers () = fp.ReadInt32 ();
	for (int i = 0; i < NumObjTriggers (); i++)
		ObjTriggers (i)->Read (fp, MineInfo ().fileInfo.version, true);
	if (MineInfo ().fileInfo.version >= 40) {
		for (int i = 0; i < NumObjTriggers (); i++)
			ObjTriggers (i)->m_info.nObject = fp.ReadInt16 ();
		}
	else {
		for (int i = 0; i < NumObjTriggers (); i++) {
			fp.ReadInt16 ();
			fp.ReadInt16 ();
			ObjTriggers (i)->m_info.nObject = fp.ReadInt16 ();
			}
		if (MineInfo ().fileInfo.version < 36)
			fp.Seek (700 * sizeof (short), SEEK_CUR);
		else
			fp.Seek (2 * sizeof (short) * fp.ReadInt16 (), SEEK_CUR);
		}
	}
if (bObjTriggersOk && NumObjTriggers ())
	SortObjTriggers ();
else {
	NumObjTriggers () = 0;
	CLEAR (ObjTriggers ());
	}
}

// -----------------------------------------------------------------------------

void Write (CFileManager& fp, CMineItemInfo& info)
{
info.count = Count (0);
if (Count (0) + Count (1) == 0)
	info.offset = -1;
else {
	info.offset = fp.Tell ();
	for (short i = 0; i < Count (); i++)
		m_triggers [0][i].Write (fp);
if (DLE.LevelVersion () >= 12) {
	fp.Write (NumObjTriggers ());
	if (NumObjTriggers () > 0) {
		SortObjTriggers ();
		for (i = 0; i < NumObjTriggers (); i++)
			ObjTriggers (i)->Write (fp, MineInfo ().fileInfo.version, true);
		for (i = 0; i < NumObjTriggers (); i++)
			fp.WriteInt16 (ObjTriggers (i)->m_info.nObject);
		}
	}
}

// -----------------------------------------------------------------------------
