/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <string.h>	// for memset
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "inferno.h"
#include "stdlib.h"
#include "texmap.h"
#include "key.h"
#include "gameseg.h"
#include "textures.h"
#include "lightning.h"
#include "objsmoke.h"
#include "physics.h"
#include "slew.h"
#include "render.h"
#include "fireball.h"
#include "error.h"
#include "endlevel.h"
#include "timer.h"
#include "gameseg.h"
#include "collide.h"
#include "dynlight.h"
#include "interp.h"
#include "newdemo.h"
#include "gauges.h"
#include "text.h"
#include "sphere.h"
#include "input.h"
#include "automap.h"
#include "u_mem.h"
#include "entropy.h"
#include "objrender.h"
#include "dropobject.h"
#include "marker.h"
#include "hiresmodels.h"
#include "loadgame.h"
#include "multi.h"
#ifdef TACTILE
#	include "tactile.h"
#endif
#ifdef EDITOR
#	include "editor/editor.h"
#endif

#ifdef _3DFX
#include "3dfx_des.h"
#endif

#define LIMIT_PHYSICS_FPS	0

void NDRecordGuidedEnd (void);
void DetachChildObjects (CObject *parent);
void DetachFromParent (CObject *sub);
int FreeObjectSlots (int nRequested);

//info on the various types of OBJECTS
#if DBG
CObject	*dbgObjP = NULL;
#endif

#ifndef fabsf
#	define fabsf(_f)	(float) fabs (_f)
#endif

int dbgObjInstances = 0;

#define	DEG90		 (I2X (1) / 4)
#define	DEG45		 (I2X (1) / 8)
#define	DEG1		 (I2X (1) / (4 * 90))

//------------------------------------------------------------------------------

int bPrintObjectInfo = 0;

tWindowRenderedData windowRenderedData [MAX_RENDERED_WINDOWS];

#if DBG
char	szObjectTypeNames [MAX_OBJECT_TYPES][10] = {
	"WALL     ",
	"FIREBALL ",
	"ROBOT    ",
	"HOSTAGE  ",
	"PLAYER   ",
	"WEAPON   ",
	"CAMERA   ",
	"POWERUP  ",
	"DEBRIS   ",
	"CNTRLCEN ",
	"FLARE    ",
	"CLUTTER  ",
	"GHOST    ",
	"LIGHT    ",
	"COOP     ",
	"MARKER   ",
	"CAMBOT   ",
	"M-BALL   ",
	"SMOKE    ",
	"EXPLOSION",
	"EFFECT   "
};
#endif

// -----------------------------------------------------------------------------

float CObject::Damage (void)
{
	float	fDmg;
	fix	xMaxShields;

if (info.nType == OBJ_PLAYER)
	fDmg = X2F (gameData.multiplayer.players [info.nId].shields) / 100;
else if (info.nType == OBJ_ROBOT) {
	xMaxShields = RobotDefaultShields (this);
	fDmg = X2F (info.xShields) / X2F (xMaxShields);
#if 0
	if (gameData.bots.info [0][info.nId].companion)
		fDmg /= 2;
#endif
	}
else if (info.nType == OBJ_REACTOR)
	fDmg = X2F (info.xShields) / X2F (ReactorStrength ());
else if ((info.nType == 255) || (info.nFlags & (OF_EXPLODING | OF_SHOULD_BE_DEAD | OF_DESTROYED | OF_ARMAGEDDON)))
	fDmg = 0.0f;
else
	fDmg = 1.0f;
return (fDmg > 1.0f) ? 1.0f : (fDmg < 0.0f) ? 0.0f : fDmg;
}

//------------------------------------------------------------------------------

#if DBG
//set viewerP CObject to next CObject in array
void ObjectGotoNextViewer (void)
{
	//int 		i;
	CObject	*objP;

FORALL_OBJS (objP, i) {
	if (objP->info.nType != OBJ_NONE) {
		gameData.objs.viewerP = objP;
		return;
		}
	}
Error ("Couldn't find a viewerP CObject!");
}

//------------------------------------------------------------------------------
//set viewerP CObject to next CObject in array
void ObjectGotoPrevViewer (void)
{
	CObject	*objP;
	int 		nStartObj = 0;
	//int		i;

nStartObj = OBJ_IDX (gameData.objs.viewerP);		//get viewerP CObject number
FORALL_OBJS (objP, i) {
	if (--nStartObj < 0)
		nStartObj = gameData.objs.nLastObject [0];
	if (OBJECTS [nStartObj].info.nType != OBJ_NONE) {
		gameData.objs.viewerP = OBJECTS + nStartObj;
		return;
		}
	}
Error ("Couldn't find a viewerP CObject!");
}
#endif

//------------------------------------------------------------------------------

CObject *ObjFindFirstOfType (int nType)
{
	//int		i;
	CObject	*objP;

FORALL_OBJS (objP, i)
	if (objP->info.nType == nType)
		return (objP);
return reinterpret_cast<CObject*> (NULL);
}

//------------------------------------------------------------------------------

int ObjReturnNumOfType (int nType)
{
	int		count = 0;
	//int		i;
	CObject	*objP;

FORALL_OBJS (objP, i)
	if (objP->info.nType == nType)
		count++;
return count;
}

//------------------------------------------------------------------------------

int ObjReturnNumOfTypeAndId (int nType, int id)
{
	int		count = 0;
	CObject	*objP;
	//int		i;

FORALL_OBJS (objP, i)
	if ((objP->info.nType == nType) && (objP->info.nId == id))
	 count++;
 return count;
 }

//------------------------------------------------------------------------------

void ResetPlayerObject (void)
{
	int i;

//Init physics
gameData.objs.consoleP->mType.physInfo.velocity.SetZero ();
gameData.objs.consoleP->mType.physInfo.thrust.SetZero ();
gameData.objs.consoleP->mType.physInfo.rotVel.SetZero ();
gameData.objs.consoleP->mType.physInfo.rotThrust.SetZero ();
gameData.objs.consoleP->mType.physInfo.brakes = gameData.objs.consoleP->mType.physInfo.turnRoll = 0;
gameData.objs.consoleP->mType.physInfo.mass = gameData.pig.ship.player->mass;
gameData.objs.consoleP->mType.physInfo.drag = gameData.pig.ship.player->drag;
gameData.objs.consoleP->mType.physInfo.flags |= PF_TURNROLL | PF_LEVELLING | PF_WIGGLE | PF_USES_THRUST;
//Init render info
gameData.objs.consoleP->info.renderType = RT_POLYOBJ;
gameData.objs.consoleP->rType.polyObjInfo.nModel = gameData.pig.ship.player->nModel;		//what model is this?
gameData.objs.consoleP->rType.polyObjInfo.nSubObjFlags = 0;		//zero the flags
gameData.objs.consoleP->rType.polyObjInfo.nTexOverride = -1;		//no tmap override!
for (i = 0; i < MAX_SUBMODELS; i++)
	gameData.objs.consoleP->rType.polyObjInfo.animAngles[i].SetZero ();
// Clear misc
gameData.objs.consoleP->info.nFlags = 0;
}

//------------------------------------------------------------------------------

//sets up gameData.multiplayer.nLocalPlayer & gameData.objs.consoleP
void InitMultiPlayerObject (void)
{
Assert ((gameData.multiplayer.nLocalPlayer >= 0) && (gameData.multiplayer.nLocalPlayer < MAX_PLAYERS));
if (gameData.multiplayer.nLocalPlayer != 0) {
	gameData.multiplayer.players [0] = LOCALPLAYER;
	gameData.multiplayer.nLocalPlayer = 0;
	}
LOCALPLAYER.nObject = 0;
LOCALPLAYER.nInvuls =
LOCALPLAYER.nCloaks = 0;
gameData.objs.consoleP = OBJECTS + LOCALPLAYER.nObject;
gameData.objs.consoleP->SetType (OBJ_PLAYER);
gameData.objs.consoleP->info.nId = gameData.multiplayer.nLocalPlayer;
gameData.objs.consoleP->info.controlType = CT_FLYING;
gameData.objs.consoleP->info.movementType = MT_PHYSICS;
gameStates.entropy.nTimeLastMoved = -1;
}

//------------------------------------------------------------------------------
//make object0 the CPlayerData, setting all relevant fields
void InitPlayerObject (void)
{
gameData.objs.consoleP->SetType (OBJ_PLAYER, false);
gameData.objs.consoleP->info.nId = 0;					//no sub-types for CPlayerData
gameData.objs.consoleP->info.nSignature = 0;			//CPlayerData has zero, others start at 1
gameData.objs.consoleP->info.xSize = gameData.models.polyModels [0][gameData.pig.ship.player->nModel].Rad ();
gameData.objs.consoleP->info.controlType = CT_SLEW;			//default is CPlayerData slewing
gameData.objs.consoleP->info.movementType = MT_PHYSICS;		//change this sometime
gameData.objs.consoleP->info.xLifeLeft = IMMORTAL_TIME;
gameData.objs.consoleP->info.nAttachedObj = -1;
ResetPlayerObject ();
InitMultiPlayerObject ();
}

//------------------------------------------------------------------------------

void CObject::Init (void)
{
info.nType = OBJ_NONE;
cType.explInfo.attached.nNext =
cType.explInfo.attached.nPrev =
cType.explInfo.attached.nParent =
info.nAttachedObj = -1;
info.nFlags = 0;
ResetSgmLinks ();
ResetLinks ();
m_shots.nObject = -1;
m_shots.nSignature = -1;
m_xCreationTime =
m_xTimeLastHit = 0;
m_vStartVel.SetZero ();
}

//------------------------------------------------------------------------------
//sets up the free list, init player data etc.
void InitObjects (void)
{
CollideInit ();
ResetSegObjLists ();
gameData.objs.lists.Init ();
gameData.objs.InitFreeList ();
for (int i = 0; i < LEVEL_OBJECTS; i++)
	OBJECTS [i].Init ();
gameData.objs.consoleP =
gameData.objs.viewerP = OBJECTS.Buffer ();
InitPlayerObject ();
gameData.objs.consoleP->LinkToSeg (0);	//put in the world in segment 0
//gameData.objs.consoleP->Link ();
gameData.objs.nObjects = 1;				//just the player
gameData.objs.nLastObject [0] = 0;
}

//------------------------------------------------------------------------------
//after calling InitObject (), the network code has grabbed specific
//object slots without allocating them.  Go through the objects and build
//the free list, then set the appropriate globals
void SpecialResetObjects (void)
{
	CObject	*objP;
	int		i;

gameData.objs.nObjects = LEVEL_OBJECTS;
gameData.objs.nLastObject [0] = 0;
gameData.objs.lists.Init ();
Assert (OBJECTS [0].info.nType != OBJ_NONE);		//0 should be used
for (objP = OBJECTS.Buffer () + LEVEL_OBJECTS, i = LEVEL_OBJECTS; i; ) {
	objP--, i--;
#if DBG
	if (i == nDbgObj) {
		nDbgObj = nDbgObj;
		if (objP->info.nType != OBJ_NONE)
			dbgObjInstances++;
		}
#endif
	objP->InitLinks ();
	if (objP->info.nType == OBJ_NONE)
		gameData.objs.freeList [--gameData.objs.nObjects] = i;
	else {
		if (i > gameData.objs.nLastObject [0])
			gameData.objs.nLastObject [0] = i;
		objP->Link ();
		}
	}
}

//------------------------------------------------------------------------------
#if DBG
int IsObjectInSeg (int nSegment, int objn)
{
	short nObject, count = 0;

for (nObject = SEGMENTS [nSegment].m_objects; nObject != -1; nObject = OBJECTS [nObject].info.nNextInSeg) {
	if (count > LEVEL_OBJECTS) {
		Int3 ();
		return count;
		}
	if (nObject==objn)
		count++;
	}
 return count;
}

//------------------------------------------------------------------------------

int SearchAllSegsForObject (int nObject)
{
	int i;
	int count = 0;

	for (i = 0; i <= gameData.segs.nLastSegment; i++) {
		count += IsObjectInSeg (i, nObject);
	}
	return count;
}

//------------------------------------------------------------------------------

void JohnsObjUnlink (int nSegment, int nObject)
{
	CObject  *objP = OBJECTS + nObject;

Assert (nObject != -1);
if (objP->info.nPrevInSeg == -1)
	SEGMENTS [nSegment].m_objects = objP->info.nNextInSeg;
else
	OBJECTS [objP->info.nPrevInSeg].info.nNextInSeg = objP->info.nNextInSeg;
if (objP->info.nNextInSeg != -1)
	OBJECTS [objP->info.nNextInSeg].info.nPrevInSeg = objP->info.nPrevInSeg;
}

//------------------------------------------------------------------------------

void RemoveAllObjectsBut (int nSegment, int nObject)
{
	int i;

for (i = 0; i <= gameData.segs.nLastSegment; i++)
	if (nSegment != i)
		if (IsObjectInSeg (i, nObject))
			JohnsObjUnlink (i, nObject);
}

//------------------------------------------------------------------------------

int CheckDuplicateObjects (void)
{
	int 		count = 0;
	CObject	*objP;
	//int		i;

FORALL_OBJS (objP, i) {
	if (objP->info.nType != OBJ_NONE) {
		count = SearchAllSegsForObject (objP->Index ());
		if (count > 1) {
#if DBG
#	if TRACE
			console.printf (1, "Object %d is in %d segments!\n", objP->Index (), count);
#	endif
			Int3 ();
#endif
			RemoveAllObjectsBut (objP->info.nSegment, objP->Index ());
			return count;
			}
		}
	}
	return count;
}

//------------------------------------------------------------------------------

void ListSegObjects (int nSegment)
{
	int nObject, count = 0;

for (nObject = SEGMENTS [nSegment].m_objects; nObject != -1; nObject = OBJECTS [nObject].info.nNextInSeg) {
	count++;
	if (count > LEVEL_OBJECTS)  {
		Int3 ();
		return;
		}
	}
return;
}
#endif

//------------------------------------------------------------------------------

void ResetSegObjLists (void)
{
for (int i = 0; i < LEVEL_SEGMENTS; i++)
	SEGMENTS [i].m_objects = -1;
}

//------------------------------------------------------------------------------

void LinkAllObjsToSegs (void)
{
	CObject	*objP;
	//int		i;

FORALL_OBJS (objP, i)
	objP->LinkToSeg (objP->info.nSegment);
}

//------------------------------------------------------------------------------

void RelinkAllObjsToSegs (void)
{
ResetSegObjLists ();
LinkAllObjsToSegs ();
}

//------------------------------------------------------------------------------

bool CheckSegObjList (CObject *objP, short nObject, short nFirstObj)
{
if (objP->info.nNextInSeg == nFirstObj)
	return false;
if ((nObject == nFirstObj) && (objP->info.nPrevInSeg >= 0))
	return false;
if (((objP->info.nNextInSeg >= 0) && (OBJECTS [objP->info.nNextInSeg].info.nPrevInSeg != nObject)) ||
	 ((objP->info.nPrevInSeg >= 0) && (OBJECTS [objP->info.nPrevInSeg].info.nNextInSeg != nObject))) 
	return false;
return true;
}

//------------------------------------------------------------------------------

CObject::CObject ()
{
SetPrev (NULL);
SetNext (NULL);
}

//------------------------------------------------------------------------------

CObject::~CObject ()
{
if (Prev ())
	Prev ()->SetNext (Next ());
if (Next ())
	Next ()->SetPrev (Prev ());
SetPrev (NULL);
SetNext (NULL);
}

//------------------------------------------------------------------------------

void CObject::UnlinkFromSeg (void)
{
if (info.nPrevInSeg == -1)
	SEGMENTS [info.nSegment].m_objects = info.nNextInSeg;
else
	OBJECTS [info.nPrevInSeg].info.nNextInSeg = info.nNextInSeg;
if (info.nNextInSeg != -1)
	OBJECTS [info.nNextInSeg].info.nPrevInSeg = info.nPrevInSeg;
info.nSegment = info.nNextInSeg = info.nPrevInSeg = -1;
}

//------------------------------------------------------------------------------

void CObject::ToBaseObject (tBaseObject *objP)
{
//objP->info = *this;
}

//------------------------------------------------------------------------------

#if 0

void CRobotObject::ToBaseObject (tBaseObject *objP)
{
objP->info = *CObject::GetInfo ();
objP->mType.physInfo = *CPhysicsInfo::GetInfo ();
objP->cType.aiInfo = *CAIStaticInfo::GetInfo ();
objP->rType.polyObjInfo = *CPolyObjInfo::GetInfo ();
}

//------------------------------------------------------------------------------

void CPowerupObject::ToBaseObject (tBaseObject *objP)
{
objP->info = *CObject::GetInfo ();
objP->mType.physInfo = *CPhysicsInfo::GetInfo ();
objP->rType.polyObjInfo = *CPolyObjInfo::GetInfo ();
}

//------------------------------------------------------------------------------

void CWeaponObject::ToBaseObject (tBaseObject *objP)
{
objP->info = *CObject::GetInfo ();
objP->mType.physInfo = *CPhysicsInfo::GetInfo ();
objP->rType.polyObjInfo = *CPolyObjInfo::GetInfo ();
}

//------------------------------------------------------------------------------

void CLightObject::ToBaseObject (tBaseObject *objP)
{
objP->info = *CObject::GetInfo ();
objP->cType.lightInfo = *CObjLightInfo::GetInfo ();
}

//------------------------------------------------------------------------------

void CLightningObject::ToBaseObject (tBaseObject *objP)
{
objP->info = *CObject::GetInfo ();
objP->rType.lightningInfo = *CLightningInfo::GetInfo ();
}

//------------------------------------------------------------------------------

void CParticleObject::ToBaseObject (tBaseObject *objP)
{
objP->info = *CObject::GetInfo ();
objP->rType.particleInfo = *CSmokeInfo::GetInfo ();
}

#endif

//------------------------------------------------------------------------------

int InsertObject (int nObject)
{
for (int i = gameData.objs.nObjects; i < LEVEL_OBJECTS; i++)
	if (gameData.objs.freeList [i] == nObject) {
		gameData.objs.freeList [i] = gameData.objs.freeList [gameData.objs.nObjects++];
		if (nObject > gameData.objs.nLastObject [0]) {
			gameData.objs.nLastObject [0] = nObject;
			if (gameData.objs.nLastObject [1] < gameData.objs.nLastObject [0])
				gameData.objs.nLastObject [1] = gameData.objs.nLastObject [0];
		}
	return 1;
	}
return 0;
}

//------------------------------------------------------------------------------

int nUnusedObjectsSlots;

//returns the number of a free object, updating gameData.objs.nLastObject [0].
//Generally, CObject::Create() should be called to get an CObject, since it
//fills in important fields and does the linking.
//returns -1 if no free objects
int AllocObject (void)
{
	CObject *objP;
	int		nObject;

if (gameData.objs.nObjects >= LEVEL_OBJECTS - 2) {
	FreeObjectSlots (LEVEL_OBJECTS - 10);
	CleanupObjects ();
	}
if (gameData.objs.nObjects >= LEVEL_OBJECTS)
	return -1;
nObject = gameData.objs.freeList [gameData.objs.nObjects++];
#if DBG
if (nObject == nDbgObj) {
	PrintLog ("allocating object #%d\n", nObject);
	nDbgObj = nDbgObj;
	if (dbgObjInstances++ > 0)
		nDbgObj = nDbgObj;
	}
#endif
if (nObject > gameData.objs.nLastObject [0]) {
	gameData.objs.nLastObject [0] = nObject;
	if (gameData.objs.nLastObject [1] < gameData.objs.nLastObject [0])
		gameData.objs.nLastObject [1] = gameData.objs.nLastObject [0];
	}
objP = OBJECTS + nObject;
#if DBG
if (objP->info.nType != OBJ_NONE)
	objP = objP;
#endif
memset (objP, 0, sizeof (*objP));
objP->info.nType = OBJ_NONE;
objP->SetLinkedType (OBJ_NONE);
objP->info.nAttachedObj =
objP->cType.explInfo.attached.nNext =
objP->cType.explInfo.attached.nPrev =
objP->cType.explInfo.attached.nParent = -1;
return nObject;
}

//------------------------------------------------------------------------------

#if DBG

bool CObject::IsInList (tObjListRef& ref, int nLink)
{
	CObject	*listObjP;

for (listObjP = ref.head; listObjP; listObjP = listObjP->m_links [nLink].next)
	if (listObjP == this)
		return true;
return false;
}

#endif

//------------------------------------------------------------------------------

void CObject::Link (tObjListRef& ref, int nLink)
{
	CObjListLink& link = m_links [nLink];

#if DBG
if (IsInList (ref, nLink)) {
	//PrintLog ("object %d, type %d, link %d is already linked\n", objP - gameData.objs.objects, objP->info.nType);
	return;
	}
if (this - gameData.objs.objects == nDbgObj)
	nDbgObj = nDbgObj;
#endif
link.prev = ref.tail;
if (ref.tail)
	ref.tail->m_links [nLink].next = this;
else
	ref.head = this;
ref.tail = this;
ref.nObjects++;
}

//------------------------------------------------------------------------------

void CObject::Unlink (tObjListRef& ref, int nLink)
{
	CObjListLink& link = m_links [nLink];

if (link.prev || link.next) {
	if (ref.nObjects <= 0)
		return;
	if (link.next)
		link.next->m_links [nLink].prev = link.prev;
	else
		ref.tail = link.prev;
	if (link.prev)
		link.prev->m_links [nLink].next = link.next;
	else
		ref.head = link.next;
	ref.nObjects--;
	}
else if ((ref.head == this) && (ref.tail == this))
		ref.head = ref.tail = NULL;
else if (ref.head == this) { //this actually means the list is corrupted
	if (!ref.tail)
		ref.head = NULL;
	else
		for (ref.head = ref.tail; ref.head->m_links [nLink].prev; ref.head = ref.head->m_links [nLink].prev)
			;
	}
else if (ref.tail == this) { //this actually means the list is corrupted
	if (!ref.head)
		ref.head = NULL;
	else
		for (ref.tail = ref.head; ref.tail->m_links [nLink].next; ref.tail = ref.tail->m_links [nLink].next)
			;
	}
link.prev = link.next = NULL;
#if DBG
if (IsInList (ref, nLink)) {
	PrintLog ("object %d, type %d, link %d is still linked\n", this - gameData.objs.objects, info.nType);
	return;
	}
CObject *objP;
if (objP = ref.head) {
	if (objP->Links (nLink).next == objP)
		nDbgObj = nDbgObj;
	if (objP->Links (nLink).prev == objP)
		nDbgObj = nDbgObj;
	}
#endif
}

//------------------------------------------------------------------------------

void CObject::Link (void)
{
	ubyte nType = info.nType;

#if DBG
if (this - gameData.objs.objects == nDbgObj) {
	nDbgObj = nDbgObj;
	//PrintLog ("linking object #%d, type %d\n", objP - gameData.objs.objects, nType);
	}
#endif
Unlink (true);
m_nLinkedType = nType;
Link (gameData.objs.lists.all, 0);
if (nType == OBJ_PLAYER)
	Link (gameData.objs.lists.players, 1);
else if (nType == OBJ_ROBOT)
	Link (gameData.objs.lists.robots, 1);
else if (nType != OBJ_REACTOR) {
	if (nType == OBJ_WEAPON)
		Link (gameData.objs.lists.weapons, 1);
	else if (nType == OBJ_POWERUP)
		Link (gameData.objs.lists.powerups, 1);
	else if (nType == OBJ_EFFECT)
		Link (gameData.objs.lists.effects, 1);
	else if (nType == OBJ_LIGHT)
		Link (gameData.objs.lists.lights, 1);
	else
		m_links [1].prev = m_links [1].next = NULL;
#ifdef _DEBUG
	if (nType == OBJ_CAMBOT)
		nType = nType;
#endif
	Link (gameData.objs.lists.statics, 2);
	return;
	}
Link (gameData.objs.lists.actors, 2);
}

//------------------------------------------------------------------------------

void CObject::Unlink (bool bForce)
{
	ubyte nType = m_nLinkedType;

if (bForce || (nType != OBJ_NONE)) {
#if DBG
	if (this - gameData.objs.objects == nDbgObj) {
		nDbgObj = nDbgObj;
		//PrintLog ("unlinking object #%d, type %d\n", objP - gameData.objs.objects, nType);
		}
#endif
	m_nLinkedType = OBJ_NONE;
	Unlink (gameData.objs.lists.all, 0);
	if (nType == OBJ_PLAYER)
		Unlink (gameData.objs.lists.players, 1);
	else if (nType == OBJ_ROBOT)
		Unlink (gameData.objs.lists.robots, 1);
	else if (nType != OBJ_REACTOR) {
		if (nType == OBJ_WEAPON)
			Unlink (gameData.objs.lists.weapons, 1);
		else if (nType == OBJ_POWERUP)
			Unlink (gameData.objs.lists.powerups, 1);
		else if (nType == OBJ_EFFECT)
			Unlink (gameData.objs.lists.effects, 1);
		else if (nType == OBJ_LIGHT)
			Unlink (gameData.objs.lists.lights, 1);
		Unlink (gameData.objs.lists.statics, 2);
		return;
		}
	Unlink (gameData.objs.lists.actors, 2);
	}
}

//------------------------------------------------------------------------------

void CObject::SetType (ubyte nNewType, bool bLink)
{
if (info.nType != nNewType) {
	if (bLink)
		Unlink ();
	info.nType = nNewType;
	if (bLink)
		Link ();
	}
}

//------------------------------------------------------------------------------
//frees up an CObject.  Generally, ReleaseObject () should be called to get
//rid of an CObject.  This function deallocates the CObject entry after
//the CObject has been unlinked
void FreeObject (int nObject)
{
	CObject	*objP = OBJECTS + nObject;

#if DBG
if (nObject == nDbgObj) {
	//PrintLog ("freeing object #%d\n", nObject);
	nDbgObj = nDbgObj;
	if (dbgObjInstances > 0)
		dbgObjInstances--;
	else
		nDbgObj = nDbgObj;
	}
#endif
objP->Unlink ();
DelObjChildrenN (nObject);
DelObjChildN (nObject);
gameData.objs.bWantEffect [nObject] = 0;
#if 1
OBJECTS [nObject].RequestEffects (DESTROY_SMOKE | DESTROY_LIGHTNINGS);
#else
SEM_ENTER (SEM_SMOKE)
KillObjectSmoke (nObject);
SEM_LEAVE (SEM_SMOKE)
SEM_ENTER (SEM_LIGHTNINGS)
lightningManager.DestroyForObject (OBJECTS + nObject);
SEM_LEAVE (SEM_LIGHTNINGS)
#endif
lightManager.Delete (-1, -1, nObject);
gameData.objs.freeList [--gameData.objs.nObjects] = nObject;
Assert (gameData.objs.nObjects >= 0);
if (nObject == gameData.objs.nLastObject [0])
	while (OBJECTS [--gameData.objs.nLastObject [0]].info.nType == OBJ_NONE);
#if DBG
if (dbgObjP && (OBJ_IDX (dbgObjP) == nObject))
	dbgObjP = NULL;
#endif
}

//-----------------------------------------------------------------------------

#if defined(_WIN32) && !DBG
typedef int __fastcall tFreeFilter (CObject *objP);
#else
typedef int tFreeFilter (CObject *objP);
#endif
typedef tFreeFilter *pFreeFilter;


int FreeDebrisFilter (CObject *objP)
{
return objP->info.nType == OBJ_DEBRIS;
}


int FreeFireballFilter (CObject *objP)
{
return (objP->info.nType == OBJ_FIREBALL) && (objP->cType.explInfo.nDeleteObj == -1);
}



int FreeFlareFilter (CObject *objP)
{
return (objP->info.nType == OBJ_WEAPON) && (objP->info.nId == FLARE_ID);
}



int FreeWeaponFilter (CObject *objP)
{
return (objP->info.nType == OBJ_WEAPON);
}

//-----------------------------------------------------------------------------

int FreeCandidates (int *candidateP, int *nCandidateP, int nToFree, pFreeFilter filterP)
{
	CObject	*objP;
	int		h = *nCandidateP, i;

for (i = 0; i < h; ) {
	objP = OBJECTS + candidateP [i];
	if (!filterP (objP))
		i++;
	else {
		if (i < --h)
			candidateP [i] = candidateP [h];
		objP->Die ();
		if (!--nToFree)
			return 0;
		}
	}
*nCandidateP = h;
return nToFree;
}

//-----------------------------------------------------------------------------
//	Scan the CObject list, freeing down to nRequested OBJECTS
//	Returns number of slots freed.
int FreeObjectSlots (int nRequested)
{
	int		i, nCandidates = 0;
	int		candidates [MAX_OBJECTS_D2X];
	int		nAlreadyFree, nToFree, nOrgNumToFree;

nAlreadyFree = LEVEL_OBJECTS - gameData.objs.nLastObject [0] - 1;

if (LEVEL_OBJECTS - nAlreadyFree < nRequested)
	return 0;

for (i = 0; i <= gameData.objs.nLastObject [0]; i++) {
	if (OBJECTS [i].info.nFlags & OF_SHOULD_BE_DEAD) {
		nAlreadyFree++;
		if (LEVEL_OBJECTS - nAlreadyFree < nRequested)
			return nAlreadyFree;
		}
	else
		switch (OBJECTS [i].info.nType) {
			case OBJ_NONE:
				nAlreadyFree++;
				if (LEVEL_OBJECTS - nAlreadyFree < nRequested)
					return 0;
				break;
			case OBJ_FIREBALL:
			case OBJ_EXPLOSION:
			case OBJ_WEAPON:
			case OBJ_DEBRIS:
				candidates [nCandidates++] = i;
				break;
#if 1
			default:
#else
			case OBJ_FLARE:
			case OBJ_ROBOT:
			case OBJ_HOSTAGE:
			case OBJ_PLAYER:
			case OBJ_REACTOR:
			case OBJ_CLUTTER:
			case OBJ_GHOST:
			case OBJ_LIGHT:
			case OBJ_CAMERA:
			case OBJ_POWERUP:
			case OBJ_MONSTERBALL:
			case OBJ_SMOKE:
			case OBJ_EXPLOSION:
			case OBJ_EFFECT:
#endif
				break;
			}
	}

nToFree = LEVEL_OBJECTS - nRequested - nAlreadyFree;
nOrgNumToFree = nToFree;
if (nToFree > nCandidates) {
#if TRACE
	console.printf (1, "Warning: Asked to free %i objects when there's only %i available.\n", nToFree, nCandidates);
#endif
	nToFree = nCandidates;
	}
if (!(nToFree = FreeCandidates (candidates, &nCandidates, nToFree, FreeDebrisFilter)))
	return nOrgNumToFree;
if (!(nToFree = FreeCandidates (candidates, &nCandidates, nToFree, FreeFireballFilter)))
	return nOrgNumToFree;
if (!(nToFree = FreeCandidates (candidates, &nCandidates, nToFree, FreeFlareFilter)))
	return nOrgNumToFree;
if (!(nToFree = FreeCandidates (candidates, &nCandidates, nToFree, FreeWeaponFilter)))
	return nOrgNumToFree;
return nOrgNumToFree - nToFree;
}

//------------------------------------------------------------------------------

bool CObject::IsLinkedToSeg (short nSegment)
{
	short nObject = OBJ_IDX (this);

for (short h = SEGMENTS [nSegment].m_objects; h >= 0; h = OBJECTS [h].info.nNextInSeg)
	if (nObject == h)
		return true;
return false;
}

//-----------------------------------------------------------------------------

void CObject::LinkToSeg (int nSegment)
{
if ((nSegment < 0) || (nSegment >= gameData.segs.nSegments)) {
	nSegment = FindSegByPos (*GetPos (), 0, 0, 0);
	if (nSegment < 0)
		return;
	}
SetSegment (nSegment);
#if DBG
if (IsLinkedToSeg (nSegment))
	UnlinkFromSeg ();
#else
if (SEGMENTS [nSegment].m_objects == Index ())
	return;
#endif
SetNextInSeg (SEGMENTS [nSegment].m_objects);
#if DBG
if ((info.nNextInSeg < -1) || (info.nNextInSeg >= LEVEL_OBJECTS))
	SetNextInSeg (-1);
#endif
SetPrevInSeg (-1);
SEGMENTS [nSegment].m_objects = Index ();
if (info.nNextInSeg != -1)
	OBJECTS [info.nNextInSeg].SetPrevInSeg (Index ());
}

//--------------------------------------------------------------------
//when an CObject has moved into a new CSegment, this function unlinks it
//from its old CSegment, and links it into the new CSegment
void CObject::RelinkToSeg (int nNewSeg)
{
#if DBG
	short nObject = OBJ_IDX (this);
if ((nObject < 0) || (nObject > gameData.objs.nLastObject [0])) {
	PrintLog ("invalid object in RelinkObjToSeg\r\n");
	return;
	}
if ((nNewSeg < 0) || (nNewSeg > gameData.segs.nLastSegment)) {
	PrintLog ("invalid segment in RelinkObjToSeg\r\n");
	return;
	}
#endif
UnlinkFromSeg ();
LinkToSeg (nNewSeg);
#if DBG
#if TRACE
if (SEGMENTS [info.nSegment].Masks (info.position.vPos, 0).m_center)
	console.printf (1, "CObject::RelinkToSeg violates seg masks.\n");
#endif
#endif
}

//------------------------------------------------------------------------------

void CObject::Initialize (ubyte nType, ubyte nId, short nCreator, short nSegment, const CFixVector& vPos,
								  const CFixMatrix& mOrient, fix xSize, ubyte cType, ubyte mType, ubyte rType)
{
SetSignature (gameData.objs.nNextSignature++);
SetType (nType);
SetId (nId);
SetLastPos (vPos);
SetSize (xSize);
SetCreator ((sbyte) nCreator);
SetOrient (&mOrient);
SetControlType (cType);
SetMovementType (mType);
SetRenderType (rType);
SetContainsType (-1);
SetLifeLeft (
	 ((gameData.app.nGameMode & GM_ENTROPY) &&  (nType == OBJ_POWERUP) &&  (nId == POW_HOARD_ORB) &&  (extraGameInfo [1].entropy.nVirusLifespan > 0)) ?
		I2X (extraGameInfo [1].entropy.nVirusLifespan) : IMMORTAL_TIME);
SetAttachedObj (-1);
SetShields (I2X (20));
SetSegment (-1);					//set to zero by memset, above
LinkToSeg (nSegment);
}

//-----------------------------------------------------------------------------

int CObject::Create (ubyte nType, ubyte nId, short nCreator, short nSegment, 
							const CFixVector& vPos, const CFixMatrix& mOrient,
							fix xSize, ubyte cType, ubyte mType, ubyte rType)
{
#if DBG
if (nType == OBJ_WEAPON) {
	nType = nType;
	if ((nCreator >= 0) && (OBJECTS [nCreator].info.nType == OBJ_ROBOT))
		nType = nType;
	if (nId == FLARE_ID)
		nType = nType;
	if (gameData.objs.bIsMissile [(int) nId])
		nType = nType;
	}
else if (nType == OBJ_ROBOT) {
	if (ROBOTINFO ((int) nId).bossFlag && (BOSS_COUNT >= MAX_BOSS_COUNT))
		return -1;
	}
else if (nType == OBJ_HOSTAGE)
	nType = nType;
else if (nType == OBJ_FIREBALL)
	nType = nType;
else if (nType == OBJ_REACTOR)
	nType = nType;
else if (nType == OBJ_DEBRIS)
	nType = nType;
else if (nType == OBJ_MARKER)
	nType = nType;
else if (nType == OBJ_PLAYER)
	nType = nType;
else if (nType == OBJ_POWERUP)
	nType = nType;
#endif

SetSegment (FindSegByPos (vPos, nSegment, 1, 0));
if ((Segment () < 0) || (Segment () > gameData.segs.nLastSegment))
	return -1;

if (nType == OBJ_DEBRIS) {
	if (gameData.objs.nDebris >= gameStates.render.detail.nMaxDebrisObjects)
		return -1;
	}

// Zero out object structure to keep weird bugs from happening in uninitialized fields.
m_nId = OBJ_IDX (this);
SetSignature (gameData.objs.nNextSignature++);
SetType (nType);
SetId (nId);
SetLastPos (vPos);
SetPos (&vPos);
SetSize (xSize);
SetCreator ((sbyte) nCreator);
SetOrient (&mOrient);
SetControlType (cType);
SetMovementType (mType);
SetRenderType (rType);
SetContainsType (-1);
SetLifeLeft (
	((gameData.app.nGameMode & GM_ENTROPY) && (nType == OBJ_POWERUP) && (nId == POW_HOARD_ORB) && (extraGameInfo [1].entropy.nVirusLifespan > 0)) ?
	I2X (extraGameInfo [1].entropy.nVirusLifespan) : IMMORTAL_TIME);
SetAttachedObj (-1);
m_xCreationTime = gameData.time.xGame;
#if 0
if (GetControlType () == CT_POWERUP)
	CPowerupInfo::SetCount (1);
// Init physics info for this CObject
if (GetMovementType () == MT_PHYSICS)
	m_vStartVel.SetZero ();
if (GetRenderType () == RT_POLYOBJ)
	CPolyObjInfo::SetTexOverride (-1);

if (GetType () == OBJ_WEAPON) {
	CPhysicsInfo::SetFlags (CPhysInfo.GetFlags () | WI_persistent (m_info.nId) * PF_PERSISTENT);
	CLaserInfo::SetCreationTime (gameData.time.xGame);
	CLaserInfo::SetLastHitObj (0);
	CLaserInfo::SetScale (I2X (1));
	}
else if (GetType () == OBJ_DEBRIS)
	gameData.objs.nDebris++;
if (GetControlType () == CT_POWERUP)
	CPowerupInfo::SetCreationTime (gameData.time.xGame);
else if (GetControlType () == CT_EXPLOSION) {
	CAttachedInfo::SetPrev (-1);
	CAttachedInfo::SetNext (-1);
	CAttachedInfo::SetParent (-1);
	}
#endif
Link ();
LinkToSeg (nSegment);
return m_nId;
}

//-----------------------------------------------------------------------------
//initialize a new CObject.  adds to the list for the given CSegment
//note that nSegment is really just a suggestion, since this routine actually
//searches for the correct CSegment
//returns the CObject number

int CreateObject (ubyte nType, ubyte nId, short nCreator, short nSegment, const CFixVector& vPos, const CFixMatrix& mOrient,
						fix xSize, ubyte cType, ubyte mType, ubyte rType)
{
	short		nObject;
	CObject	*objP;

#if DBG
if (nType == OBJ_WEAPON) {
	nType = nType;
	if ((nCreator >= 0) && (OBJECTS [nCreator].info.nType == OBJ_ROBOT))
		nType = nType;
	if (nId == FLARE_ID)
		nType = nType;
	if (gameData.objs.bIsMissile [(int) nId])
		nType = nType;
	}
else if (nType == OBJ_ROBOT) {
	if (ROBOTINFO ((int) nId).bossFlag && (BOSS_COUNT >= MAX_BOSS_COUNT))
		return -1;
	}
else if (nType == OBJ_HOSTAGE)
	nType = nType;
else if (nType == OBJ_FIREBALL)
	nType = nType;
else if (nType == OBJ_REACTOR)
	nType = nType;
else if (nType == OBJ_DEBRIS)
	nType = nType;
else if (nType == OBJ_MARKER)
	nType = nType;
else if (nType == OBJ_PLAYER)
	nType = nType;
else if (nType == OBJ_POWERUP)
	nType = nType;
#endif

//if (GetSegMasks (vPos, nSegment, 0).m_center))
if (nSegment < -1)
	nSegment = -nSegment - 2;
else
	nSegment = FindSegByPos (vPos, nSegment, 1, 0);
if ((nSegment < 0) || (nSegment > gameData.segs.nLastSegment))
	return -1;

if (nType == OBJ_DEBRIS) {
	if (gameData.objs.nDebris >= gameStates.render.detail.nMaxDebrisObjects)
		return -1;
	}

// Find next free object
if (0 > (nObject = AllocObject ()))
	return -1;
objP = OBJECTS + nObject;
objP->SetId (nObject);
// Zero out object structure to keep weird bugs from happening in uninitialized fields.
objP->info.nSignature = gameData.objs.nNextSignature++;
objP->info.nType = nType;
objP->info.nId = nId;
objP->info.vLastPos = 
objP->info.position.vPos = vPos;
objP->info.xSize = xSize;
objP->info.nCreator = (sbyte) nCreator;
objP->info.position.mOrient = mOrient;
objP->info.controlType = cType;
objP->info.movementType = mType;
objP->info.renderType = rType;
objP->info.contains.nType = -1;
objP->info.xLifeLeft = 
	((gameData.app.nGameMode & GM_ENTROPY) && (nType == OBJ_POWERUP) && (nId == POW_HOARD_ORB) && (extraGameInfo [1].entropy.nVirusLifespan > 0)) ?
	I2X (extraGameInfo [1].entropy.nVirusLifespan) : IMMORTAL_TIME;
objP->info.nAttachedObj = -1;
if (objP->info.controlType == CT_POWERUP)
	objP->cType.powerupInfo.nCount = 1;

// Init physics info for this CObject
if (objP->info.movementType == MT_PHYSICS)
	objP->SetStartVel ((CFixVector*) &CFixVector::ZERO);
if (objP->info.renderType == RT_POLYOBJ)
	objP->rType.polyObjInfo.nTexOverride = -1;
objP->SetCreationTime (gameData.time.xGame);

if (objP->info.nType == OBJ_WEAPON) {
	Assert (objP->info.controlType == CT_WEAPON);
	objP->mType.physInfo.flags |= WI_persistent (objP->info.nId) * PF_PERSISTENT;
	objP->cType.laserInfo.xCreationTime = gameData.time.xGame;
	objP->cType.laserInfo.nLastHitObj = 0;
	objP->cType.laserInfo.xScale = I2X (1);
	}
else if (objP->info.nType == OBJ_DEBRIS)
	gameData.objs.nDebris++;
if (objP->info.controlType == CT_POWERUP)
	objP->cType.powerupInfo.xCreationTime = gameData.time.xGame;
else if (objP->info.controlType == CT_EXPLOSION)
	objP->cType.explInfo.attached.nNext =
	objP->cType.explInfo.attached.nPrev =
	objP->cType.explInfo.attached.nParent = -1;

objP->Link ();
objP->LinkToSeg (nSegment);
return nObject;
}

//------------------------------------------------------------------------------

int CloneObject (CObject *objP)
{
	short		nObject, nSegment;
	int		nSignature;
	CObject	*cloneP;
	
if (0 > (nObject = AllocObject ()))
	return -1;
cloneP = OBJECTS + nObject;
nSignature = cloneP->info.nSignature;
memcpy (cloneP, objP, sizeof (CObject));
cloneP->info.nSignature = nSignature;
cloneP->info.nCreator = -1;
cloneP->mType.physInfo.thrust.SetZero ();
cloneP->SetCreationTime (gameData.time.xGame);
nSegment = objP->info.nSegment;
cloneP->info.nSegment = 
cloneP->info.nPrevInSeg = 
cloneP->info.nNextInSeg = -1;
cloneP->InitLinks ();
cloneP->SetLinkedType (OBJ_NONE);
objP->Link ();
objP->LinkToSeg (nSegment);
return nObject;
}

//------------------------------------------------------------------------------

int CreateRobot (ubyte nId, short nSegment, const CFixVector& vPos)
{
return CreateObject (OBJ_ROBOT, nId, -1, nSegment, vPos, CFixMatrix::IDENTITY, gameData.models.polyModels [0][ROBOTINFO (nId).nModel].Rad (), 
							CT_AI, MT_PHYSICS, RT_POLYOBJ);
}

//------------------------------------------------------------------------------

int CreatePowerup (ubyte nId, short nCreator, short nSegment, const CFixVector& vPos, int bIgnoreLimits)
{
if (!bIgnoreLimits && TooManyPowerups ((int) nId)) {
#if 0//def _DEBUG
	HUDInitMessage ("%c%c%c%cDiscarding excess %s!", 1, 127 + 128, 64 + 128, 128, pszPowerup [nId]);
	TooManyPowerups (nId);
#endif
	return -2;
	}
short nObject = CreateObject (OBJ_POWERUP, nId, nCreator, nSegment, vPos, CFixMatrix::IDENTITY, gameData.objs.pwrUp.info [nId].size, 
										CT_POWERUP, MT_PHYSICS, RT_POWERUP);
if ((nObject >= 0) && IsMultiGame && PowerupClass (nId)) {
	gameData.multiplayer.powerupsInMine [(int) nId]++;
	if (MultiPowerupIs4Pack (nId))
		gameData.multiplayer.powerupsInMine [(int) nId - 1] += 4;
	}
return nObject;
}

//------------------------------------------------------------------------------

int CreateWeapon (ubyte nId, short nCreator, short nSegment, const CFixVector& vPos, fix xSize, ubyte rType)
{
if (rType == 255) {
	switch (gameData.weapons.info [nId].renderType) {
		case WEAPON_RENDER_BLOB:
			rType = RT_LASER;			// Render as a laser even if blob (see render code above for explanation)
			xSize = gameData.weapons.info [nId].blob_size;
			break;
		case WEAPON_RENDER_POLYMODEL:
			xSize = 0;	//	Filled in below.
			rType = RT_POLYOBJ;
			break;
		case WEAPON_RENDER_NONE:
			rType = RT_NONE;
			xSize = I2X (1);
			break;
		case WEAPON_RENDER_VCLIP:
			rType = RT_WEAPON_VCLIP;
			xSize = gameData.weapons.info [nId].blob_size;
			break;
		default:
			Error ("Invalid weapon render nType in CreateNewWeapon\n");
			return -1;
		}
	}
return CreateObject (OBJ_WEAPON, nId, nCreator, nSegment, vPos, CFixMatrix::IDENTITY, xSize, CT_WEAPON, MT_PHYSICS, rType); 
}

//------------------------------------------------------------------------------

int CreateFireball (ubyte nId, short nSegment, const CFixVector& vPos, fix xSize, ubyte rType)
{
return CreateObject (OBJ_FIREBALL, nId, -1, nSegment, vPos, CFixMatrix::IDENTITY, xSize, CT_EXPLOSION, MT_NONE, rType);
}

//------------------------------------------------------------------------------

int CreateDebris (CObject *parentP, short nSubModel)
{
return CreateObject (OBJ_DEBRIS, 0, -1, parentP->info.nSegment, parentP->info.position.vPos, parentP->info.position.mOrient,
							gameData.models.polyModels [0][parentP->rType.polyObjInfo.nModel].SubModels ().rads [nSubModel],
							CT_DEBRIS, MT_PHYSICS, RT_POLYOBJ);
}

//------------------------------------------------------------------------------

int CreateCamera (CObject *parentP)
{
return CreateObject (OBJ_CAMERA, 0, -1, parentP->info.nSegment, parentP->info.position.vPos, parentP->info.position.mOrient, 0, 
							CT_NONE, MT_NONE, RT_NONE);
}

//------------------------------------------------------------------------------

int CreateLight (ubyte nId, short nSegment, const CFixVector& vPos)
{
return CreateObject (OBJ_LIGHT, nId, -1, nSegment, vPos, CFixMatrix::IDENTITY, 0, CT_LIGHT, MT_NONE, RT_NONE);
}

//------------------------------------------------------------------------------

#ifdef EDITOR
//create a copy of an CObject. returns new CObject number
int ObjectCreateCopy (int nObject, CFixVector *new_pos, int nNewSegnum)
{
	CObject *objP;
	int newObjNum = AllocObject ();

// Find next D2_FREE CObject
if (newObjNum == -1)
	return -1;
objP = OBJECTS + newObjNum;
*objP = OBJECTS [nObject];
objP->info.position.vPos = objP->info.vLastPos = *new_pos;
objP->info.nNextInSeg = objP->info.nPrevInSeg = objP->info.nSegment = -1;
objP->Link ();
objP->LinkToSeg (nNewSegnum);
objP->info.nSignature = gameData.objs.nNextSignature++;
//we probably should initialize sub-structures here
return newObjNum;
}
#endif

//------------------------------------------------------------------------------

//remove CObject from the world
void ReleaseObject (short nObject)
{
if (!nObject)
	return;

	int nParent;
	CObject *objP = OBJECTS + nObject;

if (objP->info.nType == OBJ_WEAPON) {
	RespawnDestroyedWeapon (nObject);
	if (objP->info.nId == GUIDEDMSL_ID) {
		nParent = OBJECTS [objP->cType.laserInfo.parent.nObject].info.nId;
		if (nParent != gameData.multiplayer.nLocalPlayer)
			gameData.objs.guidedMissile [nParent].objP = NULL;
		else if (gameData.demo.nState == ND_STATE_RECORDING)
			NDRecordGuidedEnd ();
		}
	}
if (objP == gameData.objs.viewerP)		//deleting the viewerP?
	gameData.objs.viewerP = gameData.objs.consoleP;						//..make the CPlayerData the viewerP
if (objP->info.nFlags & OF_ATTACHED)		//detach this from CObject
	DetachFromParent (objP);
if (objP->info.nAttachedObj != -1)		//detach all OBJECTS from this
	DetachChildObjects (objP);
if (objP->info.nType == OBJ_DEBRIS)
	gameData.objs.nDebris--;
OBJECTS [nObject].UnlinkFromSeg ();
Assert (OBJECTS [0].info.nNextInSeg != 0);
if ((objP->info.nType == OBJ_ROBOT) || (objP->info.nType == OBJ_REACTOR))
	ExecObjTriggers (nObject, 0);
objP->info.nType = OBJ_NONE;		//unused!
objP->info.nSignature = -1;
objP->info.nSegment = -1;				// zero it!
FreeObject (nObject);
SpawnLeftoverPowerups (nObject);
}

//------------------------------------------------------------------------------

#define	DEATH_SEQUENCE_EXPLODE_TIME	 (I2X (2))

CObject	*viewerSaveP;
int		nPlayerFlagsSave;
fix		xCameraToPlayerDistGoal=I2X (4);
ubyte		nControlTypeSave, nRenderTypeSave;

//------------------------------------------------------------------------------

void DeadPlayerEnd (void)
{
if (!gameStates.app.bPlayerIsDead)
	return;
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordRestoreCockpit ();
gameStates.app.bPlayerIsDead = 0;
gameStates.app.bPlayerExploded = 0;
if (gameData.objs.deadPlayerCamera) {
	ReleaseObject (OBJ_IDX (gameData.objs.deadPlayerCamera));
	gameData.objs.deadPlayerCamera = NULL;
	}
SelectCockpit (gameStates.render.cockpit.nModeSave);
gameStates.render.cockpit.nModeSave = -1;
gameData.objs.viewerP = viewerSaveP;
gameData.objs.consoleP->SetType (OBJ_PLAYER);
gameData.objs.consoleP->info.nFlags = nPlayerFlagsSave;
Assert ((nControlTypeSave == CT_FLYING) || (nControlTypeSave == CT_SLEW));
gameData.objs.consoleP->info.controlType = nControlTypeSave;
gameData.objs.consoleP->info.renderType = nRenderTypeSave;
LOCALPLAYER.flags &= ~PLAYER_FLAGS_INVULNERABLE;
gameStates.app.bPlayerEggsDropped = 0;
}

//------------------------------------------------------------------------------

//	Camera is less than size of CPlayerData away from
void SetCameraPos (CFixVector *vCameraPos, CObject *objP)
{
	CFixVector	vPlayerCameraOffs = *vCameraPos - objP->info.position.vPos;
	int			count = 0;
	fix			xCameraPlayerDist;
	fix			xFarScale;

xCameraPlayerDist = vPlayerCameraOffs.Mag();
if (xCameraPlayerDist < xCameraToPlayerDistGoal) { // 2*objP->info.xSize) {
	//	Camera is too close to CPlayerData CObject, so move it away.
	tFVIQuery	fq;
	tFVIData		hit_data;
	CFixVector	local_p1;

	if (vPlayerCameraOffs.IsZero())
		vPlayerCameraOffs[X] += I2X (1)/16;

	hit_data.hit.nType = HIT_WALL;
	xFarScale = I2X (1);

	while ((hit_data.hit.nType != HIT_NONE) && (count++ < 6)) {
		CFixVector	closer_p1;
		CFixVector::Normalize(vPlayerCameraOffs);
		vPlayerCameraOffs *= xCameraToPlayerDistGoal;

		fq.p0 = &objP->info.position.vPos;
		closer_p1 = objP->info.position.vPos + vPlayerCameraOffs;	//	This is the actual point we want to put the camera at.
		vPlayerCameraOffs *= xFarScale;				//	...but find a point 50% further away...
		local_p1 = objP->info.position.vPos + vPlayerCameraOffs;		//	...so we won't have to do as many cuts.

		fq.p1					= &local_p1;
		fq.startSeg			= objP->info.nSegment;
		fq.radP0				=
		fq.radP1				= 0;
		fq.thisObjNum		= objP->Index ();
		fq.ignoreObjList	= NULL;
		fq.flags				= 0;
		FindVectorIntersection (&fq, &hit_data);

		if (hit_data.hit.nType == HIT_NONE)
			*vCameraPos = closer_p1;
		else {
			vPlayerCameraOffs = CFixVector::Random();
			xFarScale = I2X (3) / 2;
			}
		}
	}
}

//------------------------------------------------------------------------------

extern int nProximityDropped, nSmartminesDropped;

void DeadPlayerFrame (void)
{
	fix			xTimeDead, h;
	CFixVector	fVec;

if (gameStates.app.bPlayerIsDead) {
	xTimeDead = gameData.time.xGame - gameStates.app.nPlayerTimeOfDeath;

	//	If unable to create camera at time of death, create now.
	if (!gameData.objs.deadPlayerCamera) {
		CObject *playerP = OBJECTS + LOCALPLAYER.nObject;
		int nObject = CreateCamera (playerP);
		if (nObject != -1)
			gameData.objs.viewerP = gameData.objs.deadPlayerCamera = OBJECTS + nObject;
		else
			Int3 ();
		}
	h = DEATH_SEQUENCE_EXPLODE_TIME - xTimeDead;
	h = max (0, h);
	gameData.objs.consoleP->mType.physInfo.rotVel = CFixVector::Create(h / 4, h / 2, h / 3);
	xCameraToPlayerDistGoal = min (xTimeDead * 8, I2X (20)) + gameData.objs.consoleP->info.xSize;
	SetCameraPos (&gameData.objs.deadPlayerCamera->info.position.vPos, gameData.objs.consoleP);
	fVec = gameData.objs.consoleP->info.position.vPos - gameData.objs.deadPlayerCamera->info.position.vPos;
	gameData.objs.deadPlayerCamera->info.position.mOrient = CFixMatrix::CreateF(fVec);

	if (xTimeDead > DEATH_SEQUENCE_EXPLODE_TIME) {
		if (!gameStates.app.bPlayerExploded) {
		if (LOCALPLAYER.hostages.nOnBoard > 1)
			HUDInitMessage (TXT_SHIP_DESTROYED_2, LOCALPLAYER.hostages.nOnBoard);
		else if (LOCALPLAYER.hostages.nOnBoard == 1)
			HUDInitMessage (TXT_SHIP_DESTROYED_1);
		else
			HUDInitMessage (TXT_SHIP_DESTROYED_0);

#ifdef TACTILE
			if (TactileStick)
				ClearForces ();
#endif
			gameStates.app.bPlayerExploded = 1;
			if (gameData.app.nGameMode & GM_NETWORK) {
				AdjustMineSpawn ();
				MultiCapObjects ();
				}
			DropPlayerEggs (gameData.objs.consoleP);
			gameStates.app.bPlayerEggsDropped = 1;
			if (IsMultiGame)
				MultiSendPlayerExplode (MULTI_PLAYER_EXPLODE);
			gameData.objs.consoleP->ExplodeBadassPlayer ();
			//is this next line needed, given the badass call above?
			gameData.objs.consoleP->Explode (0);
			gameData.objs.consoleP->info.nFlags &= ~OF_SHOULD_BE_DEAD;		//don't really kill CPlayerData
			gameData.objs.consoleP->info.renderType = RT_NONE;				//..just make him disappear
			gameData.objs.consoleP->SetType (OBJ_GHOST);						//..and kill intersections
			LOCALPLAYER.flags &= ~PLAYER_FLAGS_HEADLIGHT_ON;
#if 0
			if (gameOpts->gameplay.bFastRespawn)
				gameStates.app.bDeathSequenceAborted = 1;
#endif
			}
		}
	else {
		if (d_rand () < gameData.time.xFrame * 4) {
			if (gameData.app.nGameMode & GM_MULTI)
				MultiSendCreateExplosion (gameData.multiplayer.nLocalPlayer);
			CreateSmallFireballOnObject (gameData.objs.consoleP, I2X (1), 1);
			}
		}
	if (gameStates.app.bDeathSequenceAborted) { //xTimeDead > DEATH_SEQUENCE_LENGTH) {
		StopPlayerMovement ();
		gameStates.app.bEnterGame = 2;
		if (!gameStates.app.bPlayerEggsDropped) {
			if (gameData.app.nGameMode & GM_NETWORK) {
				AdjustMineSpawn ();
				MultiCapObjects ();
				}
			DropPlayerEggs (gameData.objs.consoleP);
			gameStates.app.bPlayerEggsDropped = 1;
			if (gameData.app.nGameMode & GM_MULTI)
				MultiSendPlayerExplode (MULTI_PLAYER_EXPLODE);
			}
		DoPlayerDead ();		//kill_playerP ();
		}
	}
}

//------------------------------------------------------------------------------

void AdjustMineSpawn (void)
{
if (!(gameData.app.nGameMode & GM_NETWORK))
	return;  // No need for this function in any other mode
if (!(gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY)))
	LOCALPLAYER.secondaryAmmo [PROXMINE_INDEX]+=nProximityDropped;
if (!(gameData.app.nGameMode & GM_ENTROPY))
	LOCALPLAYER.secondaryAmmo [SMARTMINE_INDEX]+=nSmartminesDropped;
nProximityDropped = 0;
nSmartminesDropped = 0;
}

//------------------------------------------------------------------------------

void CleanupObjects (void)
{
	CObject	*objP, *nextObjP = NULL;
	int		nLocalDeadPlayerObj = -1;

for (objP = gameData.objs.lists.all.head; objP; objP = nextObjP) {
	nextObjP = objP->Links (0).next;
	if (objP->info.nType == OBJ_NONE)
		continue;
	if (!(objP->info.nFlags & OF_SHOULD_BE_DEAD))
		continue;
	Assert ((objP->info.nType != OBJ_FIREBALL) || (objP->cType.explInfo.nDeleteTime == -1));
	if (objP->info.nType != OBJ_PLAYER)
		ReleaseObject (objP->Index ());
	else {
		if (objP->info.nId == gameData.multiplayer.nLocalPlayer) {
			if (nLocalDeadPlayerObj == -1) {
				StartPlayerDeathSequence (objP);
				nLocalDeadPlayerObj = objP->Index ();
				}
			else
				Int3 ();
			}
		}
	}
}

//--------------------------------------------------------------------
//reset CObject's movement info
//--------------------------------------------------------------------

void StopObjectMovement (CObject *objP)
{
Controls [0].headingTime = 0;
Controls [0].pitchTime = 0;
Controls [0].bankTime = 0;
Controls [0].verticalThrustTime = 0;
Controls [0].sidewaysThrustTime = 0;
Controls [0].forwardThrustTime = 0;
objP->mType.physInfo.rotThrust.SetZero ();
objP->mType.physInfo.thrust.SetZero ();
objP->mType.physInfo.velocity.SetZero ();
objP->mType.physInfo.rotVel.SetZero ();
}

//--------------------------------------------------------------------

void StopPlayerMovement (void)
{
if (!gameData.objs.speedBoost [OBJ_IDX (gameData.objs.consoleP)].bBoosted) {
	StopObjectMovement (OBJECTS + LOCALPLAYER.nObject);
	memset (&gameData.physics.playerThrust, 0, sizeof (gameData.physics.playerThrust));
//	gameData.time.xFrame = I2X (1);
	gameData.objs.speedBoost [OBJ_IDX (gameData.objs.consoleP)].bBoosted = 0;
	}
}

//	-----------------------------------------------------------------------------------------------------------

int ObjectSoundClass (CObject *objP)
{
	short nType = objP->info.nType;

if (nType == OBJ_PLAYER)
	return SOUNDCLASS_PLAYER;
if (nType == OBJ_ROBOT)
	return SOUNDCLASS_ROBOT;
if (nType == OBJ_WEAPON)
	return gameData.objs.bIsMissile [objP->info.nId] ? SOUNDCLASS_MISSILE : SOUNDCLASS_GENERIC;
return SOUNDCLASS_GENERIC;
}

//------------------------------------------------------------------------------

int ObjectCount (int nType)
{
	int		h = 0;
	//int		i;
	CObject	*objP = OBJECTS.Buffer ();

FORALL_OBJS (objP, i)
	if (objP->info.nType == nType)
		h++;
return h;
}

//------------------------------------------------------------------------------
//called after load. Takes number of objects, and objects should be
//compressed.  resets free list, marks unused objects as unused
void ResetObjects (int nObjects)
{
	CObject	*objP = OBJECTS.Buffer ();

if (!objP)
	return;

gameData.objs.nObjects = nObjects;

	int	i;

for (i = 0; i < nObjects; i++, objP++)
	if (objP->info.nType != OBJ_DEBRIS)
		objP->rType.polyObjInfo.nSubObjFlags = 0;
for (; i < LEVEL_OBJECTS; i++, objP++) {
	gameData.objs.freeList [i] = i;
	objP->info.nType = OBJ_NONE;
	objP->info.nSegment =
	objP->cType.explInfo.attached.nNext =
	objP->cType.explInfo.attached.nPrev =
	objP->cType.explInfo.attached.nParent =
	objP->info.nAttachedObj = -1;
	objP->rType.polyObjInfo.nSubObjFlags = 0;
	objP->info.nFlags = 0;
	}
gameData.objs.nLastObject [0] = gameData.objs.nObjects - 1;
gameData.objs.nDebris = 0;
}

//------------------------------------------------------------------------------
//Tries to find a CSegment for an CObject, using FindSegByPos ()
int CObject::FindSegment (void)
{
return FindSegByPos (info.position.vPos, info.nSegment, 1, 0);
}

//------------------------------------------------------------------------------
//If an CObject is in a CSegment, set its nSegment field and make sure it's
//properly linked.  If not in any CSegment, returns 0, else 1.
//callers should generally use FindVectorIntersection ()
int UpdateObjectSeg (CObject * objP, bool bMove)
{
	int nNewSeg;

#if DBG
if (objP->Index () == nDbgObj)
	nDbgObj = nDbgObj;
#endif
if (0 > (nNewSeg = objP->FindSegment ())) {
	if (!bMove)
		return 0;
	nNewSeg = FindClosestSeg (objP->info.position.vPos);
	CSegment* segP = SEGMENTS + nNewSeg;
	CFixVector vOffset = objP->info.position.vPos - segP->Center ();
	CFixVector::Normalize (vOffset);
	objP->info.position.vPos = segP->Center () + vOffset * segP->MinRad ();
	}
if (nNewSeg != objP->info.nSegment)
	objP->RelinkToSeg (nNewSeg);
return 1;
}

//------------------------------------------------------------------------------
//go through all OBJECTS and make sure they have the correct CSegment numbers
void FixObjectSegs (void)
{
	CObject	*objP;
	//int		i;

FORALL_OBJS (objP, i) {
	if ((objP->info.nType == OBJ_NONE) || (objP->info.nType == OBJ_CAMBOT) || (objP->info.nType == OBJ_EFFECT))
		continue;
	if (UpdateObjectSeg (objP))
		continue;
	CSegment* segP = SEGMENTS + objP->info.nSegment;
	fix xScale = segP->MinRad () - objP->info.xSize;
	if (xScale < 0)
		objP->info.position.vPos = segP->Center ();
	else {
		CFixVector	vCenter, vOffset;
		vCenter = segP->Center ();
		vOffset = objP->info.position.vPos - vCenter;
		CFixVector::Normalize (vOffset);
		objP->info.position.vPos = vCenter + vOffset * xScale;
		}
	}
}

//------------------------------------------------------------------------------
//go through all OBJECTS and make sure they have the correct size
void FixObjectSizes (void)
{
	//int 		i;
	CObject	*objP = OBJECTS.Buffer ();

FORALL_ROBOT_OBJS (objP, i)
	objP->info.xSize = gameData.models.polyModels [0][objP->rType.polyObjInfo.nModel].Rad ();
}

//------------------------------------------------------------------------------

//delete OBJECTS, such as weapons & explosions, that shouldn't stay between levels
//	Changed by MK on 10/15/94, don't remove proximity bombs.
//if bClearAll is set, clear even proximity bombs
void ClearTransientObjects (int bClearAll)
{
	CObject *objP, *nextObjP;

for (objP = gameData.objs.lists.weapons.head; objP; objP = nextObjP) {
	nextObjP = objP->Links (1).next;
	if ((!(gameData.weapons.info [objP->info.nId].flags & WIF_PLACABLE) &&
		  (bClearAll || ((objP->info.nId != PROXMINE_ID) && (objP->info.nId != SMARTMINE_ID)))) ||
			objP->info.nType == OBJ_FIREBALL ||
			objP->info.nType == OBJ_DEBRIS ||
			((objP->info.nType != OBJ_NONE) && (objP->info.nFlags & OF_EXPLODING))) {

#if DBG
#	if TRACE
		if (objP->info.xLifeLeft > I2X (2))
			console.printf (CON_DBG, "Note: Clearing CObject %d (nType=%d, id=%d) with lifeleft=%x\n",
							objP->Index (), objP->info.nType, objP->info.nId, objP->info.xLifeLeft);
#	endif
#endif
		ReleaseObject (objP->Index ());
		}
	#if DBG
#	if TRACE
		else if ((objP->info.nType != OBJ_NONE) && (objP->info.xLifeLeft < I2X (2)))
		console.printf (CON_DBG, "Note: NOT clearing CObject %d (nType=%d, id=%d) with lifeleft=%x\n",
						objP->Index (), objP->info.nType, objP->info.nId,	objP->info.xLifeLeft);
#	endif
#endif
	}
}

//------------------------------------------------------------------------------
//attaches an CObject, such as a fireball, to another CObject, such as a robot
void AttachObject (CObject *parentObjP, CObject *childObjP)
{
Assert (childObjP->info.nType == OBJ_FIREBALL);
Assert (childObjP->info.controlType == CT_EXPLOSION);
Assert (childObjP->cType.explInfo.attached.nNext==-1);
Assert (childObjP->cType.explInfo.attached.nPrev==-1);
Assert (parentObjP->info.nAttachedObj == -1 ||
		  OBJECTS [parentObjP->info.nAttachedObj].cType.explInfo.attached.nPrev==-1);
childObjP->cType.explInfo.attached.nNext = parentObjP->info.nAttachedObj;
if (childObjP->cType.explInfo.attached.nNext != -1)
	OBJECTS [childObjP->cType.explInfo.attached.nNext].cType.explInfo.attached.nPrev = OBJ_IDX (childObjP);
parentObjP->info.nAttachedObj = OBJ_IDX (childObjP);
childObjP->cType.explInfo.attached.nParent = OBJ_IDX (parentObjP);
childObjP->info.nFlags |= OF_ATTACHED;
Assert (childObjP->cType.explInfo.attached.nNext != OBJ_IDX (childObjP));
Assert (childObjP->cType.explInfo.attached.nPrev != OBJ_IDX (childObjP));
}

//------------------------------------------------------------------------------
//dettaches one CObject
void DetachFromParent (CObject *objP)
{
if ((OBJECTS [objP->cType.explInfo.attached.nParent].info.nType != OBJ_NONE) &&
	 (OBJECTS [objP->cType.explInfo.attached.nParent].info.nAttachedObj != -1)) {
	if (objP->cType.explInfo.attached.nNext != -1) {
		OBJECTS [objP->cType.explInfo.attached.nNext].cType.explInfo.attached.nPrev = objP->cType.explInfo.attached.nPrev;
		}
	if (objP->cType.explInfo.attached.nPrev != -1) {
		OBJECTS [objP->cType.explInfo.attached.nPrev].cType.explInfo.attached.nNext =
			objP->cType.explInfo.attached.nNext;
		}
	else {
		OBJECTS [objP->cType.explInfo.attached.nParent].info.nAttachedObj = objP->cType.explInfo.attached.nNext;
		}
	}
objP->cType.explInfo.attached.nNext =
objP->cType.explInfo.attached.nPrev =
objP->cType.explInfo.attached.nParent = -1;
objP->info.nFlags &= ~OF_ATTACHED;
}

//------------------------------------------------------------------------------
//dettaches all OBJECTS from this CObject
void DetachChildObjects (CObject *parentP)
{
while (parentP->info.nAttachedObj != -1)
	DetachFromParent (OBJECTS + parentP->info.nAttachedObj);
}

//------------------------------------------------------------------------------
//creates a marker CObject in the world.  returns the CObject number
int DropMarkerObject (CFixVector& vPos, short nSegment, CFixMatrix& orient, ubyte nMarker)
{
	short nObject;

Assert (gameData.models.nMarkerModel != -1);
nObject = CreateObject (OBJ_MARKER, nMarker, -1, nSegment, vPos, orient,
								gameData.models.polyModels [0][gameData.models.nMarkerModel].Rad (), CT_NONE, MT_NONE, RT_POLYOBJ);
if (nObject >= 0) {
	CObject *objP = OBJECTS + nObject;
	objP->rType.polyObjInfo.nModel = gameData.models.nMarkerModel;
	objP->mType.spinRate = objP->info.position.mOrient.UVec () * (I2X (1) / 2);
	//	MK, 10/16/95: Using lifeleft to make it flash, thus able to trim lightlevel from all OBJECTS.
	objP->info.xLifeLeft = IMMORTAL_TIME;
	}
return nObject;
}

//------------------------------------------------------------------------------

void ResetChildObjects (void)
{
	int	i;

for (i = 0; i < LEVEL_OBJECTS; i++) {
	gameData.objs.childObjs [i].objIndex = -1;
	gameData.objs.childObjs [i].nextObj = i + 1;
	}
gameData.objs.childObjs [i - 1].nextObj = -1;
gameData.objs.nChildFreeList = 0;
gameData.objs.firstChild.Clear (0xff);
gameData.objs.parentObjs.Clear (0xff);
}

//------------------------------------------------------------------------------

int CheckChildList (int nParent)
{
	int h, i, j;

if (gameData.objs.firstChild [nParent] == gameData.objs.nChildFreeList)
	return 0;
for (h = 0, i = gameData.objs.firstChild [nParent]; i >= 0; i = j, h++) {
	j = gameData.objs.childObjs [i].nextObj;
	if (j == i)
		return 0;
	if (h > gameData.objs.nLastObject [0])
		return 0;
	}
return 1;
}

//------------------------------------------------------------------------------

int AddChildObjectN (int nParent, int nChild)
{
	int	h, i;

if (gameData.objs.nChildFreeList < 0)
	return 0;
h = gameData.objs.firstChild [nParent];
i = gameData.objs.firstChild [nParent] = gameData.objs.nChildFreeList;
gameData.objs.nChildFreeList = gameData.objs.childObjs [gameData.objs.nChildFreeList].nextObj;
gameData.objs.childObjs [i].nextObj = h;
gameData.objs.childObjs [i].objIndex = nChild;
gameData.objs.parentObjs [nChild] = nParent;
CheckChildList (nParent);
return 1;
}

//------------------------------------------------------------------------------

int AddChildObjectP (CObject *pParent, CObject *pChild)
{
return pParent ? AddChildObjectN (OBJ_IDX (pParent), OBJ_IDX (pChild)) : 0;
//return (pParent->nType == OBJ_PLAYER) ? AddChildObjectN (OBJ_IDX (pParent), OBJ_IDX (pChild)) : 0;
}

//------------------------------------------------------------------------------

int DelObjChildrenN (int nParent)
{
	int	i, j;

for (i = gameData.objs.firstChild [nParent]; i >= 0; i = j) {
	j = gameData.objs.childObjs [i].nextObj;
	gameData.objs.childObjs [i].nextObj = gameData.objs.nChildFreeList;
	gameData.objs.childObjs [i].objIndex = -1;
	gameData.objs.nChildFreeList = i;
	}
return 1;
}

//------------------------------------------------------------------------------

int DelObjChildrenP (CObject *pParent)
{
return DelObjChildrenN (OBJ_IDX (pParent));
}

//------------------------------------------------------------------------------

int DelObjChildN (int nChild)
{
	int	nParent, h = -1, i, j;

if (0 > (nParent = gameData.objs.parentObjs [nChild]))
	return 0;
for (i = gameData.objs.firstChild [nParent]; i >= 0; i = j) {
	j = gameData.objs.childObjs [i].nextObj;
	if (gameData.objs.childObjs [i].objIndex == nChild) {
		if (h < 0)
			gameData.objs.firstChild [nParent] = j;
		else
			gameData.objs.childObjs [h].nextObj = j;
		gameData.objs.childObjs [i].nextObj = gameData.objs.nChildFreeList;
		gameData.objs.childObjs [i].objIndex = -1;
		gameData.objs.nChildFreeList = i;
		CheckChildList (nParent);
		return 1;
		}
	h = i;
	}
return 0;
}

//------------------------------------------------------------------------------

int DelObjChildP (CObject *pChild)
{
return DelObjChildN (OBJ_IDX (pChild));
}

//------------------------------------------------------------------------------

tObjectRef *GetChildObjN (short nParent, tObjectRef *pChildRef)
{
	int i = pChildRef ? pChildRef->nextObj : gameData.objs.firstChild [nParent];

return (i < 0) ? NULL : gameData.objs.childObjs + i;
}

//------------------------------------------------------------------------------

tObjectRef *GetChildObjP (CObject *pParent, tObjectRef *pChildRef)
{
return GetChildObjN (OBJ_IDX (pParent), pChildRef);
}

//------------------------------------------------------------------------------

int CountPlayerObjects (int nPlayer, int nType, int nId)
{
	int		h = 0;
	//int		i;
	CObject	*objP;

FORALL_OBJS (objP, i) 
	if ((objP->info.nType == nType) && (objP->info.nId == nId) &&
		 (objP->cType.laserInfo.parent.nType == OBJ_PLAYER) &&
		 (OBJECTS [objP->cType.laserInfo.parent.nObject].info.nId == nPlayer))
	h++;
return h;
}

//------------------------------------------------------------------------------

void GetPlayerSpawn (int nSpawnPos, CObject *objP)
{
	CObject	*markerP = SpawnMarkerObject (-1);

if (markerP) {
	objP->info.position = markerP->info.position;
 	objP->RelinkToSeg (markerP->info.nSegment);
	}
else {
	if ((gameData.multiplayer.playerInit [nSpawnPos].nSegment < 0) || 
		 (gameData.multiplayer.playerInit [nSpawnPos].nSegment >= gameData.segs.nSegments))
		GameStartInitNetworkPlayers ();
	objP->info.position = gameData.multiplayer.playerInit [nSpawnPos].position;
 	objP->RelinkToSeg (gameData.multiplayer.playerInit [nSpawnPos].nSegment);
	}
}

//------------------------------------------------------------------------------

CFixVector* PlayerSpawnPos (int nPlayer)
{
	CObject	*markerP = SpawnMarkerObject (nPlayer);

return markerP ? &markerP->info.position.vPos : &gameData.multiplayer.playerInit [nPlayer].position.vPos;
}

//------------------------------------------------------------------------------

CFixMatrix *PlayerSpawnOrient (int nPlayer)
{
	CObject	*markerP = SpawnMarkerObject (nPlayer);

return markerP ? &markerP->info.position.mOrient : &gameData.multiplayer.playerInit [nPlayer].position.mOrient;
}

//------------------------------------------------------------------------------

CFixMatrix *ObjectView (CObject *objP)
{
	tObjectViewData	*viewP = gameData.objs.viewData + objP->Index ();

if (viewP->nFrame != gameData.objs.nFrameCount) {
	viewP->mView = OBJPOS (objP)->mOrient.Transpose();
	viewP->nFrame = gameStates.render.nFrameCount;
	}
return &viewP->mView;
}

//------------------------------------------------------------------------------

#define BUILD_ALL_MODELS 0

void PrepareModels (void)
{
	int      h, i, j;
	CObject	o, *objP = OBJECTS.Buffer ();
	const char		*pszHires;

if (!RENDERPATH)
	return;
if (!gameData.objs.nLastObject [0])
	return;
PrintLog ("   building optimized polygon model data\n");
gameStates.render.nType = 1;
gameStates.render.nShadowPass = 1;
gameStates.render.bBuildModels = 1;
h = 0;
#if !BUILD_ALL_MODELS
for (i = 0; i <= gameData.objs.nLastObject [0]; i++, objP++) {
	if ((objP->info.nSegment >= 0) && (objP->info.nType != 255) && (objP->info.renderType == RT_POLYOBJ) &&
		 !G3HaveModel (objP->rType.polyObjInfo.nModel)) {
		PrintLog ("      building model %d\n", objP->rType.polyObjInfo.nModel);
#if DBG
		if (objP->rType.polyObjInfo.nModel == nDbgModel)
			nDbgModel = nDbgModel;
#endif
		if (DrawPolygonObject (objP, 1, 0))
			h++;
		}
	}
#endif
memset (&o, 0, sizeof (o));
o.info.nType = OBJ_WEAPON;
o.info.position = OBJECTS [0].info.position;
o.rType.polyObjInfo.nTexOverride = -1;
PrintLog ("   building optimized replacement model data\n");
#if BUILD_ALL_MODELS
j = 0;
for (i = 0; i < MAX_POLYGON_MODELS; i++) {
	o.rType.polyObjInfo.nModel = i; //replacementModels [i].nModel;
#else
j = ReplacementModelCount ();
for (i = 0; i < j; i++)
	if ((pszHires = replacementModels [i].pszHires) && (strstr (pszHires, "laser") == pszHires))
		break;
for (tReplacementModel *rmP = replacementModels + i; i < j; i++, rmP++) {
#endif
	if ((pszHires = rmP->pszHires)) {
		if (strstr (pszHires, "pminepack") == pszHires)
			o.info.nType = OBJ_POWERUP;
		else if (strstr (pszHires, "hostage") == pszHires)
			o.info.nType = OBJ_HOSTAGE;
		}
	o.info.nId = (ubyte) rmP->nId;
#if DBG
	if (o.info.nId == nDbgObjId)
		nDbgObjId = nDbgObjId;
#endif
	o.rType.polyObjInfo.nModel = rmP->nModel;
	if (!G3HaveModel (o.rType.polyObjInfo.nModel)) {
#if DBG
		if (o.rType.polyObjInfo.nModel == nDbgModel)
			nDbgModel = nDbgModel;
#endif
		PrintLog ("      building model %d (%s)\n", o.rType.polyObjInfo.nModel, pszHires ? pszHires : "n/a");
		if (DrawPolygonObject (&o, 1, 0))
			h++;
		}
	if (o.info.nType == OBJ_HOSTAGE)
		o.info.nType = OBJ_POWERUP;
	}
gameStates.render.bBuildModels = 0;
PrintLog ("   saving optimized polygon model data\n", h);
SaveModelData ();
PrintLog ("   finished building optimized polygon model data (%d models converted)\n", h);
}

//------------------------------------------------------------------------------

void CObject::CreateSound (short nSound)
{
audio.CreateSegmentSound (nSound, info.nSegment, 0, info.position.vPos);
}

//------------------------------------------------------------------------------

int CObject::OpenableDoorsInSegment (void)
{
return SEGMENTS [info.nSegment].HasOpenableDoor ();
}

//------------------------------------------------------------------------------

int CObject::Index (void)
{ 
return OBJ_IDX (this); 
}

//------------------------------------------------------------------------------

void CObject::LoadState (CFile& cf)
{
info.nSignature = cf.ReadInt ();      
info.nType = (ubyte) cf.ReadByte (); 
info.nId = (ubyte) cf.ReadByte ();
info.nNextInSeg = cf.ReadShort ();
info.nPrevInSeg = cf.ReadShort ();
info.controlType = (ubyte) cf.ReadByte ();
info.movementType = (ubyte) cf.ReadByte ();
info.renderType = (ubyte) cf.ReadByte ();
info.nFlags = (ubyte) cf.ReadByte ();
info.nSegment = cf.ReadShort ();
info.nAttachedObj = cf.ReadShort ();
cf.ReadVector (info.position.vPos);     
cf.ReadMatrix (info.position.mOrient);  
info.xSize = cf.ReadFix (); 
info.xShields = cf.ReadFix ();
cf.ReadVector (info.vLastPos);  
info.contains.nType = cf.ReadByte (); 
info.contains.nId = cf.ReadByte ();   
info.contains.nCount = cf.ReadByte ();
info.nCreator = cf.ReadByte ();
info.xLifeLeft = cf.ReadFix ();   
if (info.movementType == MT_PHYSICS) {
	cf.ReadVector (mType.physInfo.velocity);   
	cf.ReadVector (mType.physInfo.thrust);     
	mType.physInfo.mass = cf.ReadFix ();       
	mType.physInfo.drag = cf.ReadFix ();       
	mType.physInfo.brakes = cf.ReadFix ();     
	cf.ReadVector (mType.physInfo.rotVel);     
	cf.ReadVector (mType.physInfo.rotThrust);  
	mType.physInfo.turnRoll = cf.ReadFixAng ();   
	mType.physInfo.flags = (ushort) cf.ReadShort ();      
	}
else if (info.movementType == MT_SPINNING) {
	cf.ReadVector (mType.spinRate);  
	}
switch (info.controlType) {
	case CT_WEAPON:
		cType.laserInfo.parent.nType = cf.ReadShort ();
		cType.laserInfo.parent.nObject = cf.ReadShort ();
		cType.laserInfo.parent.nSignature = cf.ReadInt ();
		cType.laserInfo.xCreationTime = cf.ReadFix ();
		cType.laserInfo.nLastHitObj = cf.ReadShort ();
		if (cType.laserInfo.nLastHitObj < 0)
			cType.laserInfo.nLastHitObj = 0;
		else {
			gameData.objs.nHitObjects [Index () * MAX_HIT_OBJECTS] = cType.laserInfo.nLastHitObj;
			cType.laserInfo.nLastHitObj = 1;
			}
		cType.laserInfo.nHomingTarget = cf.ReadShort ();
		cType.laserInfo.xScale = cf.ReadFix ();
		break;

	case CT_EXPLOSION:
		cType.explInfo.nSpawnTime = cf.ReadFix ();
		cType.explInfo.nDeleteTime = cf.ReadFix ();
		cType.explInfo.nDeleteObj = cf.ReadShort ();
		cType.explInfo.attached.nParent = cf.ReadShort ();
		cType.explInfo.attached.nPrev = cf.ReadShort ();
		cType.explInfo.attached.nNext = cf.ReadShort ();
		break;

	case CT_AI:
		cType.aiInfo.behavior = (ubyte) cf.ReadByte ();
		cf.Read (cType.aiInfo.flags, 1, MAX_AI_FLAGS);
		cType.aiInfo.nHideSegment = cf.ReadShort ();
		cType.aiInfo.nHideIndex = cf.ReadShort ();
		cType.aiInfo.nPathLength = cf.ReadShort ();
		cType.aiInfo.nCurPathIndex = cf.ReadByte ();
		cType.aiInfo.bDyingSoundPlaying = cf.ReadByte ();
		cType.aiInfo.nDangerLaser = cf.ReadShort ();
		cType.aiInfo.nDangerLaserSig = cf.ReadInt ();
		cType.aiInfo.xDyingStartTime = cf.ReadFix ();
		break;

	case CT_LIGHT:
		cType.lightInfo.intensity = cf.ReadFix ();
		break;

	case CT_POWERUP:
		cType.powerupInfo.nCount = cf.ReadInt ();
		cType.powerupInfo.xCreationTime = cf.ReadFix ();
		cType.powerupInfo.nFlags = cf.ReadInt ();
		break;
	}
switch (info.renderType) {
	case RT_MORPH:
	case RT_POLYOBJ: {
		int i;
		rType.polyObjInfo.nModel = cf.ReadInt ();
		for (i = 0; i < MAX_SUBMODELS; i++)
			cf.ReadAngVec (rType.polyObjInfo.animAngles [i]);
		rType.polyObjInfo.nSubObjFlags = cf.ReadInt ();
		rType.polyObjInfo.nTexOverride = cf.ReadInt ();
		rType.polyObjInfo.nAltTextures = cf.ReadInt ();
		break;
		}
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
	case RT_THRUSTER:
		rType.vClipInfo.nClipIndex = cf.ReadInt ();
		rType.vClipInfo.xFrameTime = cf.ReadFix ();
		rType.vClipInfo.nCurFrame = cf.ReadByte ();
		break;

	case RT_LASER:
		break;
	}
}

//------------------------------------------------------------------------------

void CObject::SaveState (CFile& cf)
{
cf.WriteInt (info.nSignature);      
cf.WriteByte ((sbyte) info.nType); 
cf.WriteByte ((sbyte) info.nId);
cf.WriteShort (info.nNextInSeg);
cf.WriteShort (info.nPrevInSeg);
cf.WriteByte ((sbyte) info.controlType);
cf.WriteByte ((sbyte) info.movementType);
cf.WriteByte ((sbyte) info.renderType);
cf.WriteByte ((sbyte) info.nFlags);
cf.WriteShort (info.nSegment);
cf.WriteShort (info.nAttachedObj);
cf.WriteVector (OBJPOS (this)->vPos);     
cf.WriteMatrix (OBJPOS (this)->mOrient);  
cf.WriteFix (info.xSize); 
cf.WriteFix (info.xShields);
cf.WriteVector (info.vLastPos);  
cf.WriteByte (info.contains.nType); 
cf.WriteByte (info.contains.nId);   
cf.WriteByte (info.contains.nCount);
cf.WriteByte (info.nCreator);
cf.WriteFix (info.xLifeLeft);   
if (info.movementType == MT_PHYSICS) {
	cf.WriteVector (mType.physInfo.velocity);   
	cf.WriteVector (mType.physInfo.thrust);     
	cf.WriteFix (mType.physInfo.mass);       
	cf.WriteFix (mType.physInfo.drag);       
	cf.WriteFix (mType.physInfo.brakes);     
	cf.WriteVector (mType.physInfo.rotVel);     
	cf.WriteVector (mType.physInfo.rotThrust);  
	cf.WriteFixAng (mType.physInfo.turnRoll);   
	cf.WriteShort ((short) mType.physInfo.flags);      
	}
else if (info.movementType == MT_SPINNING) {
	cf.WriteVector(mType.spinRate);  
	}
switch (info.controlType) {
	case CT_WEAPON:
		cf.WriteShort (cType.laserInfo.parent.nType);
		cf.WriteShort (cType.laserInfo.parent.nObject);
		cf.WriteInt (cType.laserInfo.parent.nSignature);
		cf.WriteFix (cType.laserInfo.xCreationTime);
		if (cType.laserInfo.nLastHitObj)
			cf.WriteShort (gameData.objs.nHitObjects [Index () * MAX_HIT_OBJECTS + cType.laserInfo.nLastHitObj - 1]);
		else
			cf.WriteShort (-1);
		cf.WriteShort (cType.laserInfo.nHomingTarget);
		cf.WriteFix (cType.laserInfo.xScale);
		break;

	case CT_EXPLOSION:
		cf.WriteFix (cType.explInfo.nSpawnTime);
		cf.WriteFix (cType.explInfo.nDeleteTime);
		cf.WriteShort (cType.explInfo.nDeleteObj);
		cf.WriteShort (cType.explInfo.attached.nParent);
		cf.WriteShort (cType.explInfo.attached.nPrev);
		cf.WriteShort (cType.explInfo.attached.nNext);
		break;

	case CT_AI:
		cf.WriteByte ((sbyte) cType.aiInfo.behavior);
		cf.Write (cType.aiInfo.flags, 1, MAX_AI_FLAGS);
		cf.WriteShort (cType.aiInfo.nHideSegment);
		cf.WriteShort (cType.aiInfo.nHideIndex);
		cf.WriteShort (cType.aiInfo.nPathLength);
		cf.WriteByte (cType.aiInfo.nCurPathIndex);
		cf.WriteByte (cType.aiInfo.bDyingSoundPlaying);
		cf.WriteShort (cType.aiInfo.nDangerLaser);
		cf.WriteInt (cType.aiInfo.nDangerLaserSig);
		cf.WriteFix (cType.aiInfo.xDyingStartTime);
		break;

	case CT_LIGHT:
		cf.WriteFix (cType.lightInfo.intensity);
		break;

	case CT_POWERUP:
		cf.WriteInt (cType.powerupInfo.nCount);
		cf.WriteFix (cType.powerupInfo.xCreationTime);
		cf.WriteInt (cType.powerupInfo.nFlags);
		break;
	}
switch (info.renderType) {
	case RT_MORPH:
	case RT_POLYOBJ: {
		int i;
		cf.WriteInt (rType.polyObjInfo.nModel);
		for (i = 0; i < MAX_SUBMODELS; i++)
			cf.WriteAngVec (rType.polyObjInfo.animAngles [i]);
		cf.WriteInt (rType.polyObjInfo.nSubObjFlags);
		cf.WriteInt (rType.polyObjInfo.nTexOverride);
		cf.WriteInt (rType.polyObjInfo.nAltTextures);
		break;
		}
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
	case RT_THRUSTER:
		cf.WriteInt (rType.vClipInfo.nClipIndex);
		cf.WriteFix (rType.vClipInfo.xFrameTime);
		cf.WriteByte (rType.vClipInfo.nCurFrame);
		break;

	case RT_LASER:
		break;
	}
}

//------------------------------------------------------------------------------
//eof
