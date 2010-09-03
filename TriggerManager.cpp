#include <afxwin.h>

#include "SegmentManager.h"
#include "ObjectManager.h"
#include "TriggerManager.h"
#include "UndoManager.h"

CTriggerManager triggerManager;

//------------------------------------------------------------------------------

int CTriggerManager::CmpObjTriggers (CTrigger& pi, CTrigger& pm)
{
	short i = pi.m_info.nObject;
	short m = pm.m_info.nObject;

if (i < m)
	return -1;
if (i > m)
	return 1;
i = pi.m_info.type;
m = pm.m_info.type;
return (i < m) ? -1 : (i > m) ? 1 : 0;
}

//------------------------------------------------------------------------------

void CTriggerManager::SortObjTriggers (short left, short right)
{
	CTrigger median = m_triggers [1][(left + right) / 2];
	short	l = left, r = right;

do {
	while (CmpObjTriggers (m_triggers [1][l], median) < 0)
		l++;
	while (CmpObjTriggers (m_triggers [1][r], median) > 0)
		r--;
	if (l <= r) {
		if (l < r)
			Swap (m_triggers [1][l], m_triggers [1][r]);
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
		GetObjTrigger (i)->m_nIndex = i;
	SortObjTriggers (0, h - 1);
	}
}

//------------------------------------------------------------------------------

void CTriggerManager::RenumberObjTriggers (void)
{
	CTrigger*	trigP = GetObjTrigger (0);
	int			i;

for (i = NumObjTriggers (); i; i--, trigP++)
	trigP->m_info.nObject = objectManager.Index (objectManager.FindBySig (trigP->m_info.nObject));
i = NumObjTriggers ();
while (i) {
	if (GetObjTrigger (--i)->m_info.nObject < 0)
		DeleteFromObject (i);
	}
SortObjTriggers ();
}

//------------------------------------------------------------------------------

void CTriggerManager::RenumberTargetObjs (void)
{
	CTrigger* trigP = GetTrigger (0);

for (int i = NumObjTriggers (); i; i--, trigP++) {
	CSideKey* targetP = trigP->m_targets;
	for (int j = 0; j < trigP->m_count; ) {
		if (targetP->m_nSide >= 0) 
			targetP++;
		else {
			CGameObject* objP = objectManager.FindBySig (targetP->m_nSegment);
			if (objP != null)
				(targetP++)->m_nSegment = objectManager.Index (objP);
			else 
				trigP->Delete (j);
			}
		}
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
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();

CWall* wallP = current.Wall ();

if (wallP == null) {
	if (bAddWall) {
		if (wallManager.Count () >= MAX_WALLS) {
			ErrorMsg ("Cannot add a wall for this trigger\nsince the maximum number of walls is already reached.");
			return null;
			}
		wallP = wallManager.Create (-1, -1, (current.Child () < 0) ? WALL_OVERLAY : defWallTypes [type], 0, 0, -1, defWallTextures [type]);
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
if (theMine->IsD1File ()) {
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
CTrigger* trigP = GetTrigger (nTrigger);
trigP->Setup (type, flags);
trigP->m_nIndex = nTrigger;
wallP->SetTrigger (nTrigger);
UpdateReactor ();
undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
return GetTrigger (nTrigger);
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

CTrigger* delTrigP = GetTrigger (nDelTrigger, 0);

if (delTrigP == null)
	return;

undoManager.SetModified (true);
undoManager.Lock ();

wallManager.UpdateTrigger (nDelTrigger, NO_TRIGGER);
if (nDelTrigger < --m_nCount [0]) {	
	// move the last trigger in the list to the deleted trigger's position
	wallManager.UpdateTrigger (m_nCount [0], nDelTrigger);
	*delTrigP = m_triggers [0][m_nCount [0]];
	delTrigP->m_nIndex = Index (delTrigP);
	}

undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
UpdateReactor ();
}

//------------------------------------------------------------------------------
// Mine - DeleteTrigger
//------------------------------------------------------------------------------

void CTriggerManager::DeleteTarget (CSideKey key, short nClass) 
{
CTrigger* trigP = &m_triggers [nClass][0];
for (int i = 0; i < m_nCount [nClass]; i++, trigP++)
	trigP->Delete (key);
}

//------------------------------------------------------------------------------
// Mine - FindTrigger
//------------------------------------------------------------------------------

short CTriggerManager::FindBySide (short& nTrigger, short nSegment, short nSide)
{
current.Get (nSegment, nSide);
CWall *wallP = wallManager.FindBySide (nSegment, nSide);
if (wallP != null) {
	nTrigger = wallP->m_info.nTrigger;
	return wallManager.Index (wallP);
	}
return wallManager.Count ();
}

//------------------------------------------------------------------------------------
// Mine - FindTrigger
//------------------------------------------------------------------------------------

CTrigger* CTriggerManager::FindByTarget (short nSegment, short nSide, short i)
{
	CTrigger *trigP = GetTrigger (i);
	CSideKey key = CSideKey (nSegment, nSide);

for (; i < m_nCount [0]; i++, trigP++) {
	int j = trigP->Find (key);
	if (j >= 0)
		return trigP;
	}
return null;
}

//------------------------------------------------------------------------------------
// UpdateReactor()
//
// Action - Updates control center Triggers () so that exit door opens
//          when the reactor blows up.  Removes any invalid cube/sides
//          from reactorTriggers if they exist.
//------------------------------------------------------------------------------------

void CTriggerManager::UpdateReactor (void) 
{
  CReactorTrigger *reactorTrigger = ReactorTriggers (0);	// only one reactor trigger per level

undoManager.SetModified (true);
undoManager.Lock ();
// remove items from list that do not point to a wall
for (short nTarget = 0; nTarget < reactorTrigger->m_count; nTarget++) {
	if (!wallManager.FindBySide (reactorTrigger->m_targets [nTarget]))
		reactorTrigger->Delete (nTarget);
	}
// add any exits to target list that are not already in it
for (short nWall = 0; nWall < wallManager.Count (); nWall++) {
	CWall* wallP = wallManager.GetWall (nWall);
	CTrigger* trigP = wallP->GetTrigger ();
	if (trigP == null)
		continue;
	if (!trigP->IsExit ())
		continue;
	if (reactorTrigger->Find (*wallP))
		continue;
	reactorTrigger->Add (*wallP);
	}
undoManager.Unlock ();
}

//------------------------------------------------------------------------------------

CTrigger* CTriggerManager::AddToObject (short nObject, short type) 
{
	CGameObject* objP = (nObject < 0) ? current.Object () : objectManager.GetObject (nObject);

if (objP == null) {
	ErrorMsg ("Couldn't find object to attach triggers to.");
	return null;
	}

if ((objP->m_info.type != OBJ_ROBOT) && 
	 (objP->m_info.type != OBJ_CAMBOT) &&
	 (objP->m_info.type != OBJ_POWERUP) &&
	 (objP->m_info.type != OBJ_HOSTAGE) &&
	 (objP->m_info.type != OBJ_CNTRLCEN)) {
	ErrorMsg ("Object triggers can only be attached to robots, reactors, hostages, powerups and cameras.");
	return null;
	}

if (NumObjTriggers () >= MAX_OBJ_TRIGGERS) {
	ErrorMsg ("The maximum number of object triggers has been reached.");
	return null;
	}
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
short nTrigger = NumObjTriggers ();
GetObjTrigger (nTrigger)->Setup (type, 0);
GetObjTrigger (nTrigger)->m_info.nObject = nObject;
NumObjTriggers ()++;
undoManager.Unlock ();
SortObjTriggers ();
for (ushort i = NumObjTriggers (); i; )
	if (GetObjTrigger (--i)->m_nIndex == nTrigger)
		return GetObjTrigger (i);
return GetObjTrigger (nTrigger);
}

//------------------------------------------------------------------------------

void CTriggerManager::DeleteFromObject (short nDelTrigger) 
{
if ((nDelTrigger < 0) || (nDelTrigger >= NumObjTriggers ()))
	return;
if (nDelTrigger < --NumObjTriggers ())
	*GetObjTrigger (nDelTrigger) = *GetObjTrigger (NumObjTriggers ());
}

//------------------------------------------------------------------------------

void CTriggerManager::DeleteObjTriggers (short nObject) 
{
	short i = NumObjTriggers ();
	
while (i)
	if (GetObjTrigger (--i)->m_info.nObject == nObject)
		DeleteFromObject (i);
}

// -----------------------------------------------------------------------------

void CTriggerManager::DeleteTargets (triggerList triggers, short nTriggers, short nSegment, short nSide) 
{
CSideKey key (nSegment, nSide);
int i;

for (i = 0; nTriggers; i++)
	if (GetTrigger (i)->Delete (key))
		i--;

for (i = NumObjTriggers (); i > 0; )
	if (GetObjTrigger (--i)->Delete (key) == 0) // no targets left
		DeleteFromObject (i);
}

// ----------------------------------------------------------------------------- 

void CTriggerManager::SetIndex (void)
{
for (short i = 0; i < Count (); i++)
	m_triggers [0][i].m_nIndex = i;
}

// -----------------------------------------------------------------------------

void CTriggerManager::Read (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
if (info.offset < 0)
	return;
Count (0) = info.count;
for (short i = 0; i < info.count; i++) {
	m_triggers [0][i].Read (fp, nFileVersion, false);
	m_triggers [0][i].m_nIndex = i;
	}

int bObjTriggersOk = 1;

if (nFileVersion >= 33) {
	NumObjTriggers () = fp.ReadInt32 ();
	for (short i = 0; i < NumObjTriggers (); i++) {
		m_triggers [1][i].Read (fp, nFileVersion, true);
		m_triggers [1][i].m_nIndex = i;
		}
	if (nFileVersion >= 40) {
		for (short i = 0; i < NumObjTriggers (); i++)
			m_triggers [1][i].m_info.nObject = fp.ReadInt16 ();
		}
	else {
		for (short i = 0; i < NumObjTriggers (); i++) {
			fp.ReadInt16 ();
			fp.ReadInt16 ();
			m_triggers [1][i].m_info.nObject = fp.ReadInt16 ();
			}
		if (nFileVersion < 36)
			fp.Seek (700 * sizeof (short), SEEK_CUR);
		else
			fp.Seek (2 * sizeof (short) * fp.ReadInt16 (), SEEK_CUR);
		}
	}
if (bObjTriggersOk && NumObjTriggers ())
	SortObjTriggers ();
else {
	NumObjTriggers () = 0;
	Clear (1);
	}
}

// -----------------------------------------------------------------------------

void CTriggerManager::Write (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
info.count = Count (0);
if (Count (0) + Count (1) == 0)
	info.offset = -1;
else {
	short i;

	info.offset = fp.Tell ();

	for (i = 0; i < Count (0); i++)
		m_triggers [0][i].Write (fp, nFileVersion, false);

	if (theMine->LevelVersion () >= 12) {
		fp.Write (NumObjTriggers ());
		if (NumObjTriggers () > 0) {
			SortObjTriggers ();
			for (i = 0; i < NumObjTriggers (); i++)
				GetObjTrigger (i)->Write (fp, nFileVersion, true);
			for (i = 0; i < NumObjTriggers (); i++)
				fp.WriteInt16 (GetObjTrigger (i)->m_info.nObject);
			}
		}
	}
}

// ----------------------------------------------------------------------------- 

void CTriggerManager::Clear (int nType)
{
for (int i = 0; i < Count (nType); i++)
	m_triggers [nType][i].Clear ();
}

// ----------------------------------------------------------------------------- 

void CTriggerManager::Clear (void)
{
Clear (0);
Clear (1);
}

// ----------------------------------------------------------------------------- 

bool CTriggerManager::HaveResources (void)
{
if (!wallManager.HaveResources ())
	return false;
if (Count (0) >= MAX_TRIGGERS - 1) {
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

bool CTriggerManager::AutoAddTrigger (short wallType, ushort wallFlags, ushort triggerType) 
{
if (!HaveResources ())
	return false;
// make a new wall and a new trigger
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
if (wallManager.Create (current.m_nSegment, current.m_nSide, (byte) wallType, wallFlags, KEY_NONE, -1, -1) &&
	 AddToWall (wallManager.Count (0) - 1, triggerType, false)) {
	GetTrigger (Count (0) - 1)->Add (other.m_nSegment, other.m_nSide);
	undoManager.Unlock ();
	//DLE.MineView ()->Refresh ();
	return true;
	}
undoManager.ResetModified (bUndo);
return false;
}

// -----------------------------------------------------------------------------

bool CTriggerManager::AddDoorTrigger (short wallType, ushort wallFlags, ushort triggerType) 
{
if (other.Wall () == null) {
	ErrorMsg ("Other cube's side is not on a wall.\n\n"
				"Hint: Select a wall using the 'other cube' and\n"
				"select a trigger location using the 'current cube'.");
	return false;
	}
// automatically change the trigger type to open if not a door
if (other.Wall ()->m_info.type != WALL_DOOR)
	triggerType = TT_OPEN_WALL;
return AutoAddTrigger (wallType, wallFlags, triggerType);
}

// -----------------------------------------------------------------------------

bool CTriggerManager::AddOpenDoor (void) 
{
return AddDoorTrigger (WALL_OPEN,0,TT_OPEN_DOOR);
}

// -----------------------------------------------------------------------------

bool CTriggerManager::AddRobotMaker (void) 
{
if (other.Segment ()->m_info.function != SEGMENT_FUNC_ROBOTMAKER) {
	ErrorMsg ("There is no robot maker cube selected.\n\n"
				"Hint: Select a robot maker cube using the 'other cube' and\n"
				"select a trigger location using the 'current cube'.");
	return false;
	}
return AutoAddTrigger (WALL_OPEN, 0, TT_MATCEN);
}

// -----------------------------------------------------------------------------

bool CTriggerManager::AddShieldDrain (void) 
{
if (theMine->IsD2File ()) {
	ErrorMsg ("Descent 2 does not support shield damage Triggers ()");
   return false;
	}
return AutoAddTrigger (WALL_OPEN, 0, TT_SHIELD_DAMAGE);
}

// -----------------------------------------------------------------------------

bool CTriggerManager::AddEnergyDrain (void) 
{
if (theMine->IsD2File ()) {
	ErrorMsg ("Descent 2 does not support energy drain Triggers ()");
   return false;
	}
return AutoAddTrigger (WALL_OPEN, 0, TT_ENERGY_DRAIN);
}

// -----------------------------------------------------------------------------

bool CTriggerManager::AddUnlock (void) 
{
if (theMine->IsD1File ()) {
   ErrorMsg ("Control Panels are not supported in Descent 1.");
   return false;
	}
return AddDoorTrigger (WALL_OVERLAY, WALL_WALL_SWITCH, TT_UNLOCK_DOOR);
}

// -----------------------------------------------------------------------------
