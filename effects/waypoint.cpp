#include "descent.h"
#include "waypoint.h"

CWayPointManager wayPointManager;

// ---------------------------------------------------------------------------------

bool CWayPointManager::Setup (bool bAttach)
{
if (!Count ())
	return false;
if (!m_wayPoints.Create (m_nWayPoints))
	return false;
Gather ();
Renumber ();
LinkBack ();
if (bAttach)
	Attach ();
return true;
}

// ---------------------------------------------------------------------------------

void CWayPointManager::Destroy (void)
{
m_nWayPoints = 0;
m_wayPoints.Destroy ();
}

// ---------------------------------------------------------------------------------
// return the number of way point objects 

int32_t CWayPointManager::Count (void)
{
	CObject* pObj;

m_nWayPoints = 0;
FORALL_EFFECT_OBJS (pObj) {
	if (pObj->Id () == WAYPOINT_ID)
		++m_nWayPoints;
	}
return m_nWayPoints;
}

// ---------------------------------------------------------------------------------
// Find a way point object using its logical id (as assigned by the level author in DLE-XP)

CObject* CWayPointManager::Find (int32_t nId)
{
for (int32_t i = 0; i < m_nWayPoints; i++)
	if (m_wayPoints [i]->WayPointId () == nId)
		return m_wayPoints [i];
return NULL;
}

// ---------------------------------------------------------------------------------
// Return a pointer to the target object (end point) of a lightning effect if the effect has one

CObject* CWayPointManager::Target (CObject* pObj)
{
	int32_t nTarget = pObj->rType.lightningInfo.nTarget;

if (nTarget < 0)
	return NULL;

	CObject* pTarget;

FORALL_EFFECT_OBJS (pTarget) {
	if ((pTarget->Id () == LIGHTNING_ID) && (pTarget->rType.lightningInfo.nId == nTarget))
		return pTarget;
	}

return NULL;
}

// ---------------------------------------------------------------------------------
// Store references to all way point objects in contiguous vector

void CWayPointManager::Gather (void)
{
	CObject* pObj;
	int32_t i = 0;

FORALL_EFFECT_OBJS (pObj) {
	if (pObj->Id () == WAYPOINT_ID) 
		m_wayPoints [i++] = pObj;
	}
}

// ---------------------------------------------------------------------------------
// Map logical way point ids to way point reference indices (physical ids) in way 
// point reference vector so that 0 <= physical id < way point count

void CWayPointManager::Remap (int32_t& nId)
{
CObject* pObj = Find (nId);
#if DBG
if (pObj)
	nId = pObj->cType.wayPointInfo.nId [0];
else
	nId = -1;
#else
nId = pObj ? pObj->cType.wayPointInfo.nId [0] : -1;
#endif
}

// ---------------------------------------------------------------------------------
// Map logical way point ids of all way point successors and lightning effect objects 
// to indices in the waypoint manager's waypoint vector

void CWayPointManager::Renumber (void)
{
	CObject* pObj;

for (int32_t i = 0; i < m_nWayPoints; i++)
	m_wayPoints [i]->cType.wayPointInfo.nId [0] = i;

for (int32_t i = 0; i < m_nWayPoints; i++)
	Remap (m_wayPoints [i]->NextWayPoint ());

FORALL_EFFECT_OBJS (pObj) {
	if ((pObj->Id () == LIGHTNING_ID) && (*pObj->WayPoint () >= 0)) {
		CObject* pTarget = Target (pObj);
		if (pTarget && !Target (pTarget))
			pTarget->rType.lightningInfo.nTarget = pObj->rType.lightningInfo.nId;
		Remap (*pObj->WayPoint ());
		}
	}
}

// ---------------------------------------------------------------------------------
// Setup the predecessor ids for all way points

void CWayPointManager::LinkBack (void)
{
for (int32_t i = 0; i < m_nWayPoints; i++)
	if ((m_wayPoints [i]->NextWayPoint () >= 0) && (m_wayPoints [m_wayPoints [i]->NextWayPoint ()]->PrevWayPoint () < 0))
		m_wayPoints [m_wayPoints [i]->NextWayPoint ()]->PrevWayPoint () = i;
}

// ---------------------------------------------------------------------------------
// Set an object's position to it's current way point's position

void CWayPointManager::Attach (void)
{
	CObject* pObj;

FORALL_EFFECT_OBJS (pObj) {
	if (pObj->Id () == LIGHTNING_ID) {
		uint32_t i = (uint32_t) *pObj->WayPoint ();
		if (m_wayPoints.IsIndex (i))
			pObj->Position () = m_wayPoints [i]->Position ();
#if DBG
		else
			BRP;
#endif
		}
	}
}

// ---------------------------------------------------------------------------------
// Return reference to an object's current way point

CObject* CWayPointManager::Current (CObject* pObj)
{
return WayPoint ((uint32_t) *pObj->WayPoint ());
}

// ---------------------------------------------------------------------------------
// Return reference to an object's next way point

CObject* CWayPointManager::Successor (CObject* pObj)
{
return WayPoint ((uint32_t) Current (pObj)->cType.wayPointInfo.nSuccessor [(int32_t) pObj->rType.lightningInfo.bDirection]);
}

// ---------------------------------------------------------------------------------
// Attach an object to its next way point and repeat until the way point speed is > 0
// or the initial way point has been reached (i.e. a circle has occurred)

bool CWayPointManager::Hop (CObject* pObj)
{
	CObject* succ, * curr = Current (pObj);

for (;;) {
	succ = Successor (pObj);
	pObj->rType.lightningInfo.nWayPoint = succ->cType.wayPointInfo.nId [0];
	pObj->Position () = succ->Position ();
	if (succ->NextWayPoint () == succ->PrevWayPoint ())
		pObj->rType.lightningInfo.bDirection = !pObj->rType.lightningInfo.bDirection;
	if (succ == curr)
		return false; // avoid endless cycles
	if (succ->cType.wayPointInfo.nSpeed > 0)
		break;
	if (Target (pObj))
		pObj->StartSync ();
	}
return !pObj->Synchronize ();
}

// ---------------------------------------------------------------------------------
// Use zero speed way points to synchronize linked pairs of effect objects (e.g. a 
// lightning emitter and it's lightning end point).
// Check if moving effect object has a target effect (e.g. lightning end point) and 
// has hopped to the current way point from a zero speed way point.
// If so, check if the target effect has also hopped to its current way point from a
// zero speed way point. If so, end synchronizing the movement of the two effects
// by allowing them to move again. If not, halt the effect until the other effect 
// has made such a hop, too.

bool CWayPointManager::Synchronize (CObject* pObj)
{
if (!pObj->Synchronize ()) 
	return false;

	CObject* pTarget = Target (pObj);

if (!pTarget->Synchronize ()) 
	return true;

pObj->StopSync ();
pTarget->StopSync ();
return false;
}

// ---------------------------------------------------------------------------------
// Move an object towards its next way point. Move distance depends on the object 
// speed which is given by its current way point (i.e. the last way point it has 
// reached or passed). If a way point is reached or passed, make it or the first
// subsequent way point with a speed > 0 the current way point, and move the object
// for a distance depending on the new way point's speed and remainder of movement
// frame time.

void CWayPointManager::Move (CObject* pObj)
{
if (Synchronize (pObj))
	return;

	float fScale = 1.0f;

for (;;) {
	CObject* curr = Current (pObj);
	int32_t nSucc = curr->cType.wayPointInfo.nSuccessor [(int32_t) pObj->rType.lightningInfo.bDirection];
	if (!m_wayPoints.IsIndex (uint32_t (nSucc)))
		break;
	CObject* succ = m_wayPoints [nSucc];

		CFloatVector vMove, vLeft;

	vMove.Assign (succ->Position () - curr->Position ());
	CFloatVector::Normalize (vMove);
	float fMove = (float) curr->cType.wayPointInfo.nSpeed / 40.0f * fScale / gameStates.gameplay.slowmo [0].fSpeed;
	vMove *= fMove;
	vLeft.Assign (succ->Position () - pObj->Position ());
	float fLeft = vLeft.Mag ();
	if (fLeft > fMove) {
		pObj->Position () += vMove;
		return;
		}
	if (!Hop (pObj))
		return;
	fScale = 1.0f - fLeft / fMove;
	if (fScale < 1e-6)
		return;
	}
}

// ---------------------------------------------------------------------------------

void CWayPointManager::Update (void)
{
if (gameStates.app.tick40fps.bTick) {
	CObject* pObj;

	FORALL_EFFECT_OBJS (pObj) {
		if (pObj->Id () != LIGHTNING_ID) 
			continue;
		if (!pObj->WayPoint () || (*pObj->WayPoint () < 0))
			continue;
#if DBG
		if (!pObj->rType.lightningInfo.bEnabled)
			continue;
#endif
		if (m_wayPoints.Buffer ())
			Move (pObj);
		else
			*pObj->WayPoint () = -1;
		}
	}
}

// ---------------------------------------------------------------------------------

