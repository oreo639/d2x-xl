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

#include "descent.h"
#include "stdlib.h"
#include "texmap.h"
#include "key.h"
#include "segmath.h"
#include "textures.h"
#include "lightning.h"
#include "objsmoke.h"
#include "physics.h"
#include "slew.h"
#include "rendermine.h"
#include "fireball.h"
#include "error.h"
#include "endlevel.h"
#include "timer.h"
#include "segmath.h"
#include "collide.h"
#include "dynlight.h"
#include "interp.h"
#include "newdemo.h"
#include "cockpit.h"
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

int32_t MultiPowerupIs4Pack (int32_t);

//------------------------------------------------------------------------------

void CObject::SetCreationTime (fix xCreationTime) 
{ 
m_xCreationTime = ((xCreationTime < 0) ? gameData.time.xGame : xCreationTime); 
}

//------------------------------------------------------------------------------

fix CObject::LifeTime (void) 
{ 
return gameData.time.xGame - m_xCreationTime; 
}

//------------------------------------------------------------------------------

void CObject::Initialize (uint8_t nType, uint8_t nId, int16_t nCreator, int16_t nSegment, const CFixVector& vPos,
								  const CFixMatrix& mOrient, fix xSize, uint8_t cType, uint8_t mType, uint8_t rType)
{
SetSignature (gameData.objs.nNextSignature++);
SetType (nType);
SetKey (nId);
SetLastPos (vPos);
SetSize (xSize);
SetCreator ((int8_t) nCreator);
SetOrient (&mOrient);
SetControlType (cType);
SetMovementType (mType);
SetRenderType (rType);
SetContainsType (-1);
SetLifeLeft (
	 (IsEntropyGame &&  (nType == OBJ_POWERUP) &&  (nId == POW_HOARD_ORB) &&  (extraGameInfo [1].entropy.nVirusLifespan > 0)) ?
		I2X (extraGameInfo [1].entropy.nVirusLifespan) : IMMORTAL_TIME);
SetAttachedObj (-1);
SetShield (I2X (20));
SetSegment (-1);					//set to zero by memset, above
LinkToSeg (nSegment);
}

//-----------------------------------------------------------------------------

int32_t CObject::Create (uint8_t nType, uint8_t nId, int16_t nCreator, int16_t nSegment,
							const CFixVector& vPos, const CFixMatrix& mOrient,
							fix xSize, uint8_t cType, uint8_t mType, uint8_t rType)
{
#if DBG
if (nType == OBJ_WEAPON) {
	nType = nType;
	if ((nCreator >= 0) && (OBJECTS [nCreator].info.nType == OBJ_ROBOT))
		nType = nType;
	if (nId == FLARE_ID)
		nType = nType;
	if (IsMissile ((int32_t) nId))
		nType = nType;
	}
else if (nType == OBJ_ROBOT) {
#if 0
	if (ROBOTINFO ((int32_t) nId).bossFlag && (BOSS_COUNT >= MAX_BOSS_COUNT))
		return -1;
#endif
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
	if (!gameOpts->render.effects.nShrapnels && (gameData.objs.nDebris >= gameStates.render.detail.nMaxDebrisObjects))
		return -1;
	}

// Zero out object structure to keep weird bugs from happening in uninitialized fields.
SetKey (OBJ_IDX (this));
SetSignature (gameData.objs.nNextSignature++);
SetType (nType);
SetKey (nId);
SetLastPos (vPos);
SetPos (&vPos);
SetSize (xSize);
SetCreator ((int8_t) nCreator);
SetOrient (&mOrient);
SetControlType (cType);
SetMovementType (mType);
SetRenderType (rType);
SetContainsType (-1);
SetLifeLeft (
	(IsEntropyGame && (nType == OBJ_POWERUP) && (nId == POW_HOARD_ORB) && (extraGameInfo [1].entropy.nVirusLifespan > 0)) ?
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
return Key ();
}

//-----------------------------------------------------------------------------
//initialize a new CObject.  adds to the list for the given CSegment
//note that nSegment is really just a suggestion, since this routine actually
//searches for the correct CSegment
//returns the CObject number

int32_t CreateObject (uint8_t nType, uint8_t nId, int16_t nCreator, int16_t nSegment, const CFixVector& vPos, const CFixMatrix& mOrient, fix xSize, uint8_t cType, uint8_t mType, uint8_t rType)
{
	int16_t		nObject;
	CObject	*objP;

#if DBG
if (nType == OBJ_WEAPON) {
	BRP;
	if ((nCreator >= 0) && (OBJECTS [nCreator].info.nType == OBJ_ROBOT)) {
		BRP;
		if ((nDbgSeg >= 0) && (nSegment == nDbgSeg))
			BRP;
		}
	if (nId == FLARE_ID)
		BRP;
	if (nId == EARTHSHAKER_MEGA_ID)
		BRP;
	if (CObject::IsMissile ((int32_t) nId))
		BRP;
	else
		BRP;
	}
else if (nType == OBJ_ROBOT) {
	BRP;
#if 0
	if (ROBOTINFO ((int32_t) nId).bossFlag && (BOSS_COUNT >= MAX_BOSS_COUNT))
		return -1;
#endif
	}
else if (nType == OBJ_HOSTAGE)
	BRP;
else if (nType == OBJ_FIREBALL)
	BRP;
else if (nType == OBJ_REACTOR)
	BRP;
else if (nType == OBJ_DEBRIS)
	BRP;
else if (nType == OBJ_MARKER)
	BRP;
else if (nType == OBJ_PLAYER)
	BRP;
else if (nType == OBJ_POWERUP) {
	BRP;
	if (nId == POW_MONSTERBALL)
		nId = nId;
	if (nId == 27) // unknown powerup type
		nId = nId;
	}
#endif

//if (GetSegMasks (vPos, nSegment, 0).m_center))
if (nSegment < -1)
	nSegment = -nSegment - 2;
else
	nSegment = FindSegByPos (vPos, nSegment, 1, 0);
if ((nSegment < 0) || (nSegment > gameData.segs.nLastSegment))
	return -1;

if (nType == OBJ_DEBRIS) {
	if (!gameOpts->render.effects.nShrapnels && (gameData.objs.nDebris >= gameStates.render.detail.nMaxDebrisObjects))
		return -1;
	}

// Find next free object
if (0 > (nObject = AllocObject ()))
	return -1;
objP = OBJECTS + nObject;
objP->SetKey (nObject);
objP->SetCreationTime ();
// Zero out object structure to keep weird bugs from happening in uninitialized fields.
objP->info.nSignature = gameData.objs.nNextSignature++;
objP->info.nType = nType;
objP->info.nId = nId;
objP->info.vLastPos =
objP->info.position.vPos = vPos;
objP->SetOrigin (vPos);
objP->SetSize (xSize);
objP->info.nCreator = int8_t (nCreator);
objP->SetLife (IMMORTAL_TIME);
if (IsMultiGame && IsEntropyGame && (nType == OBJ_POWERUP) && (nId == POW_ENTROPY_VIRUS)) {
	if ((nCreator >= 0) && (OBJECTS [nCreator].info.nType == OBJ_PLAYER))
		objP->info.nCreator = int8_t (GetTeam (OBJECTS [nCreator].info.nId) + 1);
	if (extraGameInfo [1].entropy.nVirusLifespan > 0)
		objP->SetLife (I2X (extraGameInfo [1].entropy.nVirusLifespan));
	}
objP->info.position.mOrient = mOrient;
objP->info.controlType = cType;
objP->info.movementType = mType;
objP->info.renderType = rType;
objP->info.contains.nType = -1;
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

objP->Arm ();
objP->Link ();
objP->LinkToSeg (nSegment);
objP->StopSync ();

memset (&objP->HitInfo (), 0, sizeof (CObjHitInfo));
#if 1
if (IsMultiGame && IsCoopGame && 
	 (nType == OBJ_WEAPON) && CObject::IsMissile (int16_t (nId)) && (nCreator >= 0) && (OBJECTS [nCreator].info.nType == OBJ_PLAYER)) {
	extern char powerupToObject [MAX_POWERUP_TYPES];

	for (int32_t i = 0; i < MAX_POWERUP_TYPES; i++) {
		if (powerupToObject [i] == nId)
			gameData.multiplayer.maxPowerupsAllowed [i]--;
		}
	}
#endif
OBJECTS [nObject].ResetDamage ();
OBJECTS [nObject].SetTarget (NULL);
return nObject;
}

//------------------------------------------------------------------------------

int32_t CloneObject (CObject *objP)
{
	int16_t	nObject, nSegment;
	int32_t	nSignature;
	CObject*	cloneP;

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
#if OBJ_LIST_TYPE == 1
cloneP->InitLinks ();
cloneP->SetLinkedType (OBJ_NONE);
#endif
objP->Link ();
objP->LinkToSeg (nSegment);
return nObject;
}

//------------------------------------------------------------------------------

int32_t CreateRobot (uint8_t nId, int16_t nSegment, const CFixVector& vPos)
{
if (nId >= gameData.bots.nTypes [gameStates.app.bD1Mission]) {
	PrintLog (0, "Trying to create non-existent robot (type %d)\n", nId);
	return -1;
	}
return CreateObject (OBJ_ROBOT, nId, -1, nSegment, vPos, CFixMatrix::IDENTITY, gameData.models.polyModels [0][ROBOTINFO (nId).nModel].Rad (),CT_AI, MT_PHYSICS, RT_POLYOBJ);
}

//------------------------------------------------------------------------------

int32_t PowerupsInMine (int32_t nPowerup)
{
int32_t nCount = 0;
if (gameStates.multi.nGameType == UDP_GAME) {
	nCount = PowerupsOnShips (nPowerup);
	if (MultiPowerupIs4Pack (nPowerup))
		nCount /= 4;
	}
nCount += gameData.multiplayer.powerupsInMine [nPowerup];
if (nPowerup == POW_VULCAN_AMMO) {
	int32_t	nAmmo = 0;
	int32_t	i;
	CObject* objP;
	FORALL_POWERUP_OBJS (objP) {
		if ((objP->Id () == POW_VULCAN) || (objP->Id () == POW_GAUSS))
			nAmmo += objP->cType.powerupInfo.nCount;
		}
	nCount += (nAmmo + VULCAN_CLIP_CAPACITY - 1) / VULCAN_CLIP_CAPACITY;
	}
else if ((nPowerup == POW_PROXMINE) || (nPowerup == POW_SMARTMINE)) {
	int32_t nMines = 0;
	int32_t	nId = (nPowerup == POW_PROXMINE) ? PROXMINE_ID : SMARTMINE_ID;
	int32_t	i;
	CObject* objP;
	FORALL_WEAPON_OBJS (objP) {
		if (objP->Id () == nId)
			nMines++;
		}
	nCount += (nMines + 3) / 4;
	}
return nCount;
}

//-----------------------------------------------------------------------------

#if DBG
int32_t nDbgPowerup = -1;
#endif

void AddAllowedPowerup (int32_t nPowerup, int32_t nCount)
{
if (nCount && powerupFilter [nPowerup]) {
#if DBG
	if (nPowerup == nDbgPowerup)
		BRP;
#endif
	if (MultiPowerupIs4Pack (nPowerup))
		gameData.multiplayer.maxPowerupsAllowed [nPowerup - 1] += 4 * nCount;
	gameData.multiplayer.maxPowerupsAllowed [nPowerup] += nCount;
	if ((nPowerup == POW_VULCAN) || (nPowerup == POW_GAUSS))
		gameData.multiplayer.maxPowerupsAllowed [POW_VULCAN_AMMO] += 2 * nCount;
	}
}

//-----------------------------------------------------------------------------

void RemoveAllowedPowerup (int32_t nPowerup)
{
if (MultiPowerupIs4Pack (nPowerup))
	gameData.multiplayer.maxPowerupsAllowed [nPowerup - 1] -= 4;
gameData.multiplayer.maxPowerupsAllowed [nPowerup]--;
if ((nPowerup == POW_VULCAN) || (nPowerup == POW_GAUSS))
	gameData.multiplayer.maxPowerupsAllowed [POW_VULCAN_AMMO] -= 2;
}

//-----------------------------------------------------------------------------

void AddPowerupInMine (int32_t nPowerup, bool bIncreaseLimit)
{
#if DBG
if (nPowerup == nDbgPowerup)
	BRP;
#endif
if (MultiPowerupIs4Pack (nPowerup))
	gameData.multiplayer.powerupsInMine [nPowerup - 1] += 4;
gameData.multiplayer.powerupsInMine [nPowerup]++;
if (bIncreaseLimit)
	AddAllowedPowerup (nPowerup);
}

//-----------------------------------------------------------------------------

void RemovePowerupInMine (int32_t nPowerup)
{
#if DBG
if (nPowerup == nDbgPowerup)
	BRP;
#endif
if (gameData.multiplayer.powerupsInMine [nPowerup] > 0) {
	gameData.multiplayer.powerupsInMine [nPowerup]--;
	if (MultiPowerupIs4Pack (nPowerup)) {
		if (gameData.multiplayer.powerupsInMine [--nPowerup] < 4)
			gameData.multiplayer.powerupsInMine [nPowerup] = 0;
		else
			gameData.multiplayer.powerupsInMine [nPowerup] -= 4;
		}
	}
}

//------------------------------------------------------------------------------

int32_t MissingPowerups (int32_t nPowerup, int32_t bBreakDown)
{
return gameData.multiplayer.maxPowerupsAllowed [nPowerup] - PowerupsInMine (nPowerup);
}

//------------------------------------------------------------------------------

static inline int32_t TooManyPowerups (int32_t nPowerup)
{
#if DBG
if (nPowerup == nDbgPowerup)
	BRP;
#endif
if (!IsMultiGame)
	return 0;
if (!PowerupClass (nPowerup))
	return 0;
if (PowerupsInMine (nPowerup) < gameData.multiplayer.maxPowerupsAllowed [nPowerup])
	return 0;
return 1;
}

//------------------------------------------------------------------------------

int32_t CreatePowerup (uint8_t nId, int16_t nCreator, int16_t nSegment, const CFixVector& vPos, int32_t bIgnoreLimits, bool bForce)
{
if (gameStates.app.bGameSuspended & SUSP_POWERUPS)
	return -1;
if (nId >= MAX_POWERUP_TYPES) {
	PrintLog (0, "Trying to create non-existent powerup (type %d)\n", nId);
	return -1;
	}
if (!bIgnoreLimits && TooManyPowerups ((int32_t) nId)) {
#if 1 //DBG
	PrintLog (0, "Deleting excess powerup %d (in mine: %d, on ships: %d, max: %d)\n", 
				 nId, gameData.multiplayer.powerupsInMine [nId], PowerupsOnShips (nId), gameData.multiplayer.maxPowerupsAllowed [nId]);
	//HUDInitMessage ("%c%c%c%cDiscarding excess %s!", 1, 127 + 128, 64 + 128, 128, pszPowerup [nId]);
	TooManyPowerups (nId);
#endif
	return -2;
	}
if (gameStates.gameplay.bMineMineCheat && !bForce && (CObject::IsEquipment (nId) < 2))
	return -1;
int16_t nObject = CreateObject (OBJ_POWERUP, nId, nCreator, nSegment, vPos, CFixMatrix::IDENTITY, gameData.objs.pwrUp.info [nId].size, CT_POWERUP, MT_PHYSICS, RT_POWERUP);
if ((nObject >= 0) && IsMultiGame && PowerupClass (nId)) 
	AddPowerupInMine ((int32_t) nId);
return nObject;
}

//------------------------------------------------------------------------------

int32_t CreateWeapon (uint8_t nId, int16_t nCreator, int16_t nSegment, const CFixVector& vPos, fix xSize, uint8_t rType)
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

int32_t CreateFireball (uint8_t nId, int16_t nSegment, const CFixVector& vPos, fix xSize, uint8_t rType)
{
return CreateObject (OBJ_FIREBALL, nId, -1, nSegment, vPos, CFixMatrix::IDENTITY, xSize, CT_EXPLOSION, MT_NONE, rType);
}

//------------------------------------------------------------------------------

int32_t CreateDebris (CObject *parentP, int16_t nSubModel)
{
return CreateObject (OBJ_DEBRIS, 0, -1, parentP->info.nSegment, parentP->info.position.vPos, parentP->info.position.mOrient,
							gameData.models.polyModels [0][parentP->ModelId ()].SubModels ().rads [nSubModel],
							CT_DEBRIS, MT_PHYSICS, RT_POLYOBJ);
}

//------------------------------------------------------------------------------

int32_t CreateCamera (CObject *parentP)
{
return CreateObject (OBJ_CAMERA, 0, -1, parentP->info.nSegment, parentP->info.position.vPos, parentP->info.position.mOrient, 0,
							CT_NONE, MT_NONE, RT_NONE);
}

//------------------------------------------------------------------------------

int32_t CreateLight (uint8_t nId, int16_t nSegment, const CFixVector& vPos)
{
return CreateObject (OBJ_LIGHT, nId, -1, nSegment, vPos, CFixMatrix::IDENTITY, 0, CT_LIGHT, MT_NONE, RT_NONE);
}

//------------------------------------------------------------------------------

void CreateSmallFireballOnObject (CObject *objP, fix size_scale, int32_t bSound)
{
	fix			size;
	CFixVector	vPos, vRand;
	int16_t			nSegment;

vPos = objP->info.position.vPos;
vRand = CFixVector::Random();
vRand *= (objP->info.xSize / 2);
vPos += vRand;
size = FixMul (size_scale, I2X (1) / 2 + RandShort () * 4 / 2);
nSegment = FindSegByPos (vPos, objP->info.nSegment, 1, 0);
if (nSegment != -1) {
	CObject *explObjP = CreateExplosion (nSegment, vPos, size, VCLIP_SMALL_EXPLOSION);
	if (!explObjP)
		return;
	AttachObject (objP, explObjP);
	if (bSound || (RandShort () < 8192)) {
		fix vol = I2X (1) / 2;
		if (objP->info.nType == OBJ_ROBOT)
			vol *= 2;
		audio.CreateObjectSound (SOUND_EXPLODING_WALL, SOUNDCLASS_EXPLOSION, objP->Index (), 0, vol);
		}
	}
}

//------------------------------------------------------------------------------


void CreateVClipOnObject (CObject *objP, fix xScale, uint8_t nVClip)
{
	fix			xSize;
	CFixVector	vPos, vRand;
	int16_t			nSegment;

vPos = objP->info.position.vPos;
vRand = CFixVector::Random();
vRand *= (objP->info.xSize / 2);
vPos += vRand;
xSize = FixMul (xScale, I2X (1) + RandShort ()*4);
nSegment = FindSegByPos (vPos, objP->info.nSegment, 1, 0);
if (nSegment != -1) {
	CObject *explObjP = CreateExplosion (nSegment, vPos, xSize, nVClip);
	if (!explObjP)
		return;

	explObjP->info.movementType = MT_PHYSICS;
	explObjP->mType.physInfo.velocity = objP->mType.physInfo.velocity * (I2X (1) / 2);
	}
}

//------------------------------------------------------------------------------
//creates a marker CObject in the world.  returns the CObject number
int32_t DropMarkerObject (CFixVector& vPos, int16_t nSegment, CFixMatrix& orient, uint8_t nMarker)
{
	int16_t nObject;

Assert (gameData.models.nMarkerModel != -1);
nObject = CreateObject (OBJ_MARKER, nMarker, -1, nSegment, vPos, orient,
								gameData.models.polyModels [0][gameData.models.nMarkerModel].Rad (), CT_NONE, MT_NONE, RT_POLYOBJ);
if (nObject >= 0) {
	CObject *objP = OBJECTS + nObject;
	objP->rType.polyObjInfo.nModel = gameData.models.nMarkerModel;
	objP->mType.spinRate = objP->info.position.mOrient.m.dir.u * (I2X (1) / 2);
	//	MK, 10/16/95: Using lifeleft to make it flash, thus able to trim lightlevel from all OBJECTS.
	objP->SetLife (IMMORTAL_TIME);
	}
return nObject;
}

//------------------------------------------------------------------------------

//remove CObject from the world
void ReleaseObject (int16_t nObject)
{
if ((nObject <= 0) || (nObject >= LEVEL_OBJECTS))
	return;

	int32_t nParent;
	CObject *objP = OBJECTS + nObject;

if (objP->info.nType == OBJ_WEAPON) {
	if (gameData.demo.nVcrState != ND_STATE_PLAYBACK)
		RespawnDestroyedWeapon (nObject);
	if (objP->info.nId == GUIDEDMSL_ID) {
		nParent = OBJECTS [objP->cType.laserInfo.parent.nObject].info.nId;
		if (nParent != N_LOCALPLAYER)
			gameData.objs.guidedMissile [nParent].objP = NULL;
		else if (gameData.demo.nState == ND_STATE_RECORDING)
			NDRecordGuidedEnd ();
		}
	}
if (objP == gameData.objs.viewerP)		//deleting the viewerP?
	gameData.objs.viewerP = gameData.objs.consoleP;						//..make the player the viewerP
if (objP->info.nFlags & OF_ATTACHED)		//detach this from CObject
	DetachFromParent (objP);
if (objP->info.nAttachedObj != -1)		//detach all OBJECTS from this
	DetachChildObjects (objP);
if (objP->info.nType == OBJ_DEBRIS)
	gameData.objs.nDebris--;
OBJECTS [nObject].UnlinkFromSeg ();
Assert (OBJECTS [0].info.nNextInSeg != 0);
if ((objP->info.nType == OBJ_ROBOT) ||
	 (objP->info.nType == OBJ_DEBRIS) ||	// exploded robot
	 (objP->info.nType == OBJ_REACTOR) ||
	 (objP->info.nType == OBJ_POWERUP) ||
	 (objP->info.nType == OBJ_HOSTAGE))
	ExecObjTriggers (nObject, 0);
objP->info.nType = OBJ_NONE;		//unused!
objP->info.nSignature = -1;
objP->info.nSegment = -1;				// zero it!
try {
	FreeObject (nObject);
	}
catch (...) {
	PrintLog (0, "Error freeing an object\n");
	}
SpawnLeftoverPowerups (nObject);
}

//------------------------------------------------------------------------------
//eof
