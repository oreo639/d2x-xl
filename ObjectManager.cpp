#include "mine.h"
#include "dle-xp.h"

CObjectManager objectManager;

// -----------------------------------------------------------------------------

CObjectManager::CObjectManager ()
{
SortObjects () = GetPrivateProfileInt ("DLE", "SortObjects", objectManager.SortObjects (), DLE.IniFile ());
}

// -----------------------------------------------------------------------------

CGameObject* CObjectManager::FindBySeg (short nSegment, short i)
{
for (CGameObject* objP = Object (i); i < Count (); i++)
	if (objP->m_info.nSegment == nSegment)
		return objP;
return null;
}

//------------------------------------------------------------------------

CGameObject* CObjectManager::FindBySig (short nSignature)
{
	CGameObject* objP = Object (0);

for (short i = Count (); i; i--, objP++)
	if (objP->m_info.signature == nSignature)
		return objP;
return null;
}

//------------------------------------------------------------------------

CGameObject* CObjectManager::FindRobot (short nId, short i)
{
for (CGameObject* objP = Object (i); i < Count (); i++, objP++)
	if (objP->Id () == nId)
		return objP;
return null;
}

//------------------------------------------------------------------------

static short sortObjType [MAX_OBJECT_TYPES] = {7, 8, 5, 4, 0, 2, 9, 3, 10, 6, 11, 12, 13, 14, 1, 16, 15, 17, 18, 19, 20};


int CObjectManager::Compare (CGameObject& pi, CGameObject& pm)
{
	short ti = sortObjType [pi.m_info.type];
	short tm = sortObjType [pm.m_info.type];

if (ti < tm)
	return -1;
if (ti > tm)
	return 1;
return (pi.m_info.id < pm.m_info.id) ? -1 : (pi.m_info.id > pm.m_info.id) ? 1 : 0;
}

//------------------------------------------------------------------------

void CObjectManager::Sort (short left, short right)
{
	CGameObject	median = m_objects [(left + right) / 2];
	short	l = left, r = right;

do {
	while (Compare (m_objects [l], median) < 0)
		l++;
	while (Compare (m_objects [r], median) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			Swap (m_objects [l], m_objects [r]);
			if (current->m_nObject == l)
				current->m_nObject = r;
			else if (current->m_nObject == r)
				current->m_nObject = l;
			if (other->m_nObject == l)
				other->m_nObject = r;
			else if (other->m_nObject == r)
				other->m_nObject = l;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	Sort (l, right);
if (left < r)
	Sort (left, r);
}

//------------------------------------------------------------------------
	
void CObjectManager::Sort (void)
{
	short	i = Count ();

if (m_bSortObjects && (i > 1)) {
	for (short j = 0; j < i; j++)
		Object (j)->m_info.signature = j;
	Sort (0, i - 1);
	SetIndex ();
	triggerManager.RenumberObjTriggers ();
	triggerManager.RenumberTargetObjs ();
	}
}

// -----------------------------------------------------------------------------

void CObjectManager::SetIndex (void)
{
for (int i = 0; i < Count (); i++)
	m_objects [i].Index () = i;
}

// -----------------------------------------------------------------------------

bool CObjectManager::HaveResources (void)
{
if (Count () < MAX_OBJECTS)
	return true;
ErrorMsg ("The maximum number of objects has already been reached.");
return false;
}

// -----------------------------------------------------------------------------

short CObjectManager::Add (bool bUndo)
{
if (!HaveResources ())
	return -1;
m_objects [Count ()].Clear ();
m_objects [Count ()].Backup (opAdd);
return Count ()++;
}

// -----------------------------------------------------------------------------
// copy_object ()
//
// Action - Copies the current object to a new object
//          If object is a player or coop, it chooses the next available id
//
// Parameters - char newType = new type of object
//
// Returns - true upon success
//
// -----------------------------------------------------------------------------

bool CObjectManager::Create (ubyte newType, short nSegment) 
{
	short ids [MAX_PLAYERS_D2X + MAX_COOP_PLAYERS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	CGameObject *objP, *currObjP;
	ubyte type;
	short nObject, id;

// If OBJ_NONE used, then make a copy of the current object type
// otherwise use the type passed as the parameter "newType"
if (!HaveResources ())
	return false;

type = (newType == OBJ_NONE) ? current->Object ()->m_info.type : newType;

// Make sure it is ok to add another object of this type
// Set the id if it's a player or coop
if ((type == OBJ_PLAYER) || (type == OBJ_COOP)) {
	objP = Object (0);
	for (nObject = Count (); nObject; nObject--, objP++)
		if (objP->Type () == type) {
			id = objP->Id ();
			if ((id >= 0) && (id < MAX_PLAYERS + MAX_COOP_PLAYERS))
				ids [id]++;
			}
	if (type == OBJ_PLAYER) {
		for (id = 0; (id <= MAX_PLAYERS) && ids [id]; id++)
				;// loop until 1st id with 0
		if (id > MAX_PLAYERS) {
				char szMsg [80];

			sprintf_s (szMsg, sizeof (szMsg), "There are already %d players in the mine", MAX_PLAYERS);
			ErrorMsg (szMsg);
			return false;
			}
		}
	else {
		for (id = MAX_PLAYERS; (id < MAX_PLAYERS + MAX_COOP_PLAYERS) && ids [id]; id++)
			;// loop until 1st id with 0
		if (id > MAX_PLAYERS + MAX_COOP_PLAYERS) {
				char szMsg [80];

			sprintf_s (szMsg, sizeof (szMsg), "There are already %d cooperative players in the mine", MAX_COOP_PLAYERS);
			ErrorMsg (szMsg);
			return false;
			}
		}
	}

// Now we can add the object
// Make a new object
undoManager.Begin (udObjects);
nObject = Add ();
if (nObject == 0) {
	Object (0)->Create (OBJ_PLAYER, (nSegment < 0) ? current->m_nSegment : nSegment);
	nObject = 0;
	}
else {
	// Make a copy of the current object
	objP = Object (nObject);
	currObjP = current->Object ();
	memcpy (objP, currObjP, sizeof (CGameObject));
	}
objP->m_info.flags = 0;                                      // new: 1/27/97
//objP->m_info.nSegment = current->m_nSegment;
// set object position in the center of the segment for now
CVertex center;
//objP->Position () = segmentManager.CalcCenter (center, current->m_nSegment);
//objP->m_location.lastPos = objP->Position ();
objP->Info ().nSegment = -1;
current->m_nObject = nObject;
Move (objP);
// set the id if new object is a player or a coop
if ((type == OBJ_PLAYER) || (type == OBJ_COOP))
	objP->Id () = (ubyte) id;
// set object data if new object being added
if (newType != OBJ_NONE)
	objP->Setup (objP->Type () = newType);
// set the contents to zero
objP->m_info.contents.type = 0;
objP->m_info.contents.id = 0;
objP->m_info.contents.count = 0;
Sort ();
objP->Backup ();
DLE.MineView ()->Refresh (false);
DLE.ToolView ()->ObjectTool ()->Refresh ();
undoManager.End ();
return true;
}

// -----------------------------------------------------------------------------

void CObjectManager::Delete (short nDelObj, bool bUndo)
{
if (Count () == 0) {
	if (!DLE.ExpertMode ())
		ErrorMsg ("There are no objects in the mine.");
	return;
	}
if (Count () == 1) {
	if (!DLE.ExpertMode ())
		ErrorMsg ("Cannot delete the last object.");
	return;
	}
if (nDelObj < 0)
	nDelObj = current->m_nObject;
if (nDelObj == Count ()) {
	if (!DLE.ExpertMode ())
		ErrorMsg ("Cannot delete the secret return.");
	return;
	}
undoManager.Begin (udObjects);
if (nDelObj < 0)
	nDelObj = current->m_nObject;
triggerManager.DeleteObjTriggers (nDelObj);
Object (nDelObj)->Backup (opDelete);
if (nDelObj < --Count ()) {
	for (int i = 0; i < Count (); i++)
		Object (i)->m_info.signature = i;
	memcpy (Object (nDelObj), Object (nDelObj + 1), (Count () - nDelObj) * sizeof (m_objects [0]));
	SetIndex ();
	triggerManager.RenumberObjTriggers ();
	triggerManager.RenumberTargetObjs ();
	}
if (selections [0].m_nObject >= Count ())
	selections [0].m_nObject = Count () - 1;
if (selections [1].m_nObject >= Count ())
	selections [1].m_nObject = Count () - 1;
undoManager.End ();
}

// -----------------------------------------------------------------------------

void CObjectManager::DeleteSegmentObjects (short nSegment)
{
for (short i = Count (); i >= 0; i--)
	if (Object (i)->m_info.nSegment == nSegment)
		Delete (i); 
	}

// -----------------------------------------------------------------------------

void CObjectManager::Move (CGameObject * objP, int nSegment)
{
#if 0
if (QueryMsg ("Are you sure you want to move the\n"
				 "current object to the current segment?\n") != IDYES)
	return;
#endif
undoManager.Begin (udObjects);
if (objP == null)
	objP = current->Object ();
if (Index (objP) == Count ())
	SecretSegment () = current->m_nSegment;
else {
	CVertex center;
	if (nSegment < 0)
		nSegment = current->m_nSegment;
	// bump position over if this is not the first object in the segment
	int i, count = 0;
	for (i = 0; i < Count (); i++)
		if (Object (i)->Info ().nSegment == nSegment) 
			count++;
	objP->Info ().nSegment = current->m_nSegment;
	objP->Position () = segmentManager.CalcCenter (center, nSegment);
	if (0 < count) {
		int dir = ((count - 1) % 6) / 2;
		//count -= 2 * dir;
		i = ((count % 6) & 1) ? 1 : -1;
		i *= count / 6 + 1;
		i *= 3;
		if (dir == 2) {
			objP->Position ().v.x += i;
			objP->m_location.lastPos.v.x += i;
			}
		else if (dir == 1) {
			objP->Position ().v.z += i;
			objP->m_location.lastPos.v.z += i;
			}
		else { //if (!dir || ((count - 1) / 6)) {
			objP->Position ().v.y += i;
			objP->m_location.lastPos.v.y += i;
			}
		objP->Info ().nSegment = nSegment;
		}
	DLE.MineView ()->Refresh (false);
	}
undoManager.End ();
}

// -----------------------------------------------------------------------------

void CObjectManager::UpdateSegments (short nOldSeg, short nNewSeg)
{
undoManager.Begin (udObjects);
for (int i = 0; i < Count (); i++)
	if (Object (i)->m_info.nSegment == nOldSeg)
		Object (i)->m_info.nSegment = nNewSeg;
undoManager.End ();
}

// -----------------------------------------------------------------------------

void CObjectManager::Read (CFileManager* fp)
{
if (m_info.Restore (fp)) {
	for (short i = 0; i < Count (); i++) {
		if (i < MAX_OBJECTS) {
			m_objects [i].Read (fp);
			m_objects [i].Index () = i;
			}
		else {
			CGameObject o;
			o.Read (fp);
			}	
		}
	}
}

// ----------------------------------------------------------------------------- 

void CObjectManager::Write (CFileManager* fp)
{
if (m_info.Setup (fp)) {
	m_info.size = 0x108;
	for (short i = 0; i < Count (); i++)
		m_objects [i].Write (fp);
	}
}

// ----------------------------------------------------------------------------- 

void CObjectManager::Clear (void)
{
for (short i = 0; i < Count (); i++)
	m_objects [i].Clear ();
}

// ----------------------------------------------------------------------------- 

void CObjectManager::SetCenter (CDoubleVector v)
{
for (short i = 0; i < Count (); i++)
	m_objects [i].m_location -= v;
}

// -----------------------------------------------------------------------------

void CObjectManager::DeleteD2X (void)
{
for (short i = Count () - 1; i >= 0; i--)
	if (m_objects [i].IsD2X ())
		Delete (i);
}

// -----------------------------------------------------------------------------

void CObjectManager::DeleteVertigo (void)
{
for (short i = Count () - 1; i >= 0; i--)
	if (m_objects [i].IsVertigo ())
		Delete (i);
}

// -----------------------------------------------------------------------------