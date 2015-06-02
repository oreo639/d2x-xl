#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include "descent.h"
#include "error.h"
#include "text.h"
#include "u_mem.h"
#include "timer.h"
#include "findpath.h"
#include "segmath.h"
#include "network.h"
#include "network_lib.h"
#include "loadobjects.h"
#include "objeffects.h"
#include "fireball.h"
#include "dropobject.h"
#include "headlight.h"
#include "thrusterflames.h"

#if DBG
#	define	BASE_NET_DROP_DEPTH	2
#else
#	define	BASE_NET_DROP_DEPTH	8
#endif

//------------------------------------------------------------------------------

int32_t InitObjectCount (CObject *pObj)
{
ENTER (0, 0);
	int32_t	nFree, nTotal, i, j, bFree;
	int16_t	nType = pObj->info.nType;
	int16_t	id = pObj->info.nId;

nFree = nTotal = 0;
for (i = 0, pObj = gameData.objData.init.Buffer (); i < gameFileInfo.objects.count; i++, pObj++) {
	if ((pObj->info.nType != nType) || (pObj->info.nId != id))
		continue;
	nTotal++;
	for (bFree = 1, j = SEGMENT (pObj->info.nSegment)->m_objects; j != -1; j = OBJECT (j)->info.nNextInSeg)
		if ((OBJECT (j)->info.nType == nType) && (OBJECT (j)->info.nId == id)) {
			bFree = 0;
			break;
			}
	if (bFree)
		nFree++;
	}
RETVAL (nFree ? -nFree : nTotal)
}

//------------------------------------------------------------------------------

CObject *FindInitObject (CObject *pObj)
{
ENTER (0, 0);
	int32_t	h, i, j, bUsed,
			bUseFree,
			objCount = InitObjectCount (pObj);
	int16_t	nType = pObj->info.nType;
	int16_t	id = pObj->info.nId;

// due to OBJECTS being deleted from the CObject list when picked up and recreated when dropped,
// cannot determine exact respawn CSegment, so randomly chose one from all segments where powerups
// of this nType had initially been placed in the level.
if (!objCount)		//no OBJECTS of this nType had initially been placed in the mine.
	RETVAL (NULL)	//can happen with missile packs
if ((bUseFree = (objCount < 0)))
	objCount = -objCount;
h = Rand (objCount) + 1;
for (i = 0, pObj = gameData.objData.init.Buffer (); i < gameFileInfo.objects.count; i++, pObj++) {
	if ((pObj->info.nType != nType) || (pObj->info.nId != id))
		continue;
	// if the current CSegment does not contain a powerup of the nType being looked for,
	// return that CSegment
	if (bUseFree) {
		for (bUsed = 0, j = SEGMENT (pObj->info.nSegment)->m_objects; j != -1; j = OBJECT (j)->info.nNextInSeg)
			if ((OBJECT (j)->info.nType == nType) && (OBJECT (j)->info.nId == id)) {
				bUsed = 1;
				break;
				}
		if (bUsed)
			continue;
		}
	if (!--h)
		RETVAL (pObj)
	}
RETVAL (NULL)
}

// --------------------------------------------------------------------------------------------------------------------
//	Return true if there is a door here and it is openable
//	It is assumed that the player has all keys.
int32_t PlayerCanOpenDoor (CSegment *pSeg, int16_t nSide)
{
CWall* pWall = pSeg->Wall (nSide);
if (!pWall)
	return 0;						//	no CWall here.
int16_t wallType = pWall->nType;
//	Can't open locked doors.
if (( (wallType == WALL_DOOR) && (pWall->flags & WALL_DOOR_LOCKED)) || (wallType == WALL_CLOSED))
	return 0;
return 1;
}

// --------------------------------------------------------------------------------------------------------------------
//	Return a CSegment %i segments away from initial CSegment.
//	Returns -1 if can't find a CSegment that distance away.

// --------------------------------------------------------------------------------------------------------------------

static int32_t	segQueue [MAX_SEGMENTS_D2X];

int32_t PickConnectedSegment (CObject *pObj, int32_t nMaxDepth, int32_t *nDepthP)
{
ENTER (0, 0);
	int32_t		nCurDepth;
	int32_t		nStartSeg;
	int32_t		nHead, nTail;
	int16_t		i, j, nSideCount, nSide, nChild, sideList [6];
	CSegment*	pSeg;
	CWall*		pWall;
	uint8_t		bVisited [MAX_SEGMENTS_D2X];

if (!pObj)
	RETVAL (-1)
nStartSeg = OBJSEG (pObj);
nHead =
nTail = 0;
segQueue [nHead++] = nStartSeg;

memset (bVisited, 0, gameData.segData.nSegments);
bVisited [nStartSeg] = 1;

while (nTail != nHead) {
	nCurDepth = bVisited [segQueue [nTail]];
	if (nCurDepth >= nMaxDepth) {
		if (nDepthP)
			*nDepthP = nCurDepth;
		RETVAL (segQueue [nTail + Rand (nHead - nTail)])
		}
	pSeg = SEGMENT (segQueue [nTail++]);

	//	select sides randomly
	for (i = 0, nSideCount = 0; i < SEGMENT_SIDE_COUNT; i++)
		if (pSeg->Side (i)->FaceCount ())
			sideList [nSideCount++] = i;
	for (i = nSideCount; i; ) {
		j = Rand (i);
		nSide = sideList [j];
		if (j < --i)
			sideList [j] = sideList [i];
		if (0 > (nChild = pSeg->m_children [nSide]))
			continue;
		if (bVisited [nChild])
			continue;
		pWall = pSeg->Wall (nSide);
		if (pWall && !PlayerCanOpenDoor (pSeg, nSide))
			continue;
		segQueue [nHead++] = pSeg->m_children [nSide];
		bVisited [nChild] = nCurDepth + 1;
		}
	}
#if TRACE
console.printf (CON_DBG, "...failed at depth %i, returning -1\n", nCurDepth);
#endif
while ((nTail > 0) && (bVisited [segQueue [nTail - 1]] == nCurDepth))
	nTail--;
if (nDepthP)
	*nDepthP = nCurDepth + 1;
RETVAL (segQueue [nTail + Rand (nHead - nTail)])
}

//	------------------------------------------------------------------------------------------------------
//	Choose CSegment to drop a powerup in.
//	For all active net players, try to create a N CSegment path from the player.  If possible, return that
//	CSegment.  If not possible, try another player.  After a few tries, use a Random CSegment.
//	Don't drop if control center in CSegment.
int32_t ChooseDropSegment (CObject *pObj, int32_t *pbFixedPos, int32_t nDropState)
{
ENTER (0, 0);
	int32_t		nPlayer = 0;
	int16_t		nSegment = -1;
	int32_t		nDepth, nDropDepth;
	int32_t		count;
	int16_t		nPlayerSeg;
	CFixVector	tempv, *vPlayerPos;
	fix			nDist = 0;
	int32_t		bUseInitSgm =
					pObj &&
					(EGI_FLAG (bFixedRespawns, 0, 0, 0) ||
					 (EGI_FLAG (bEnhancedCTF, 0, 0, 0) &&
					 (pObj->info.nType == OBJ_POWERUP) && ((pObj->info.nId == POW_BLUEFLAG) || (pObj->info.nId == POW_REDFLAG))));
#if TRACE
console.printf (CON_DBG, "ChooseDropSegment:");
#endif
if (bUseInitSgm) {
	CObject *pInitObj = FindInitObject (pObj);
	if (pInitObj) {
		*pbFixedPos = 1;
		pObj->info.position.vPos = pInitObj->info.position.vPos;
		pObj->info.position.mOrient = pInitObj->info.position.mOrient;
		RETVAL (pInitObj->info.nSegment)
		}
	}
if (pbFixedPos)
	*pbFixedPos = 0;
nDepth = BASE_NET_DROP_DEPTH + Rand (BASE_NET_DROP_DEPTH * 2);
vPlayerPos = &LOCALOBJECT->info.position.vPos;
nPlayerSeg = LOCALOBJECT->info.nSegment;

if (gameStates.multi.nGameType != UDP_GAME)
	gameStates.app.SRand ();
while (nSegment == -1) {
	if (!IsMultiGame)
		nPlayer = N_LOCALPLAYER;
	else {	// chose drop segment at required minimum distance from some random player
		nPlayer = Rand (N_PLAYERS);
		count = 0;
		while ((count < N_PLAYERS) &&	// make sure player is not the local player or on his team
				 (!PLAYER (nPlayer).connected ||
				  (nPlayer == N_LOCALPLAYER) ||
				  ((gameData.appData.GameMode (GM_TEAM|GM_CAPTURE|GM_ENTROPY)) && (GetTeam (nPlayer) == GetTeam (N_LOCALPLAYER))))) {
			nPlayer = (nPlayer + 1) % N_PLAYERS;
			count++;
			}
		if (count == N_PLAYERS)
			nPlayer = N_LOCALPLAYER;
		}
	nSegment = PickConnectedSegment (OBJECT (PLAYER (nPlayer).nObject), nDepth, &nDropDepth);
#if 1
	if (nDropDepth < BASE_NET_DROP_DEPTH / 2)
		RETVAL (-1)
#endif
#if TRACE
	console.printf (CON_DBG, " %d", nSegment);
#endif
	if (nSegment == -1) {
		nDepth--;
		continue;
		}

	CSegment* pSeg = SEGMENT (nSegment);
	int32_t nSegFunc = pSeg->m_function;
	if (pSeg->HasBlockedProp () ||
		 (nSegFunc == SEGMENT_FUNC_REACTOR) ||
		 (nSegFunc == SEGMENT_FUNC_GOAL_BLUE) ||
		 (nSegFunc == SEGMENT_FUNC_GOAL_RED) ||
		 (nSegFunc == SEGMENT_FUNC_TEAM_BLUE) ||
		 (nSegFunc == SEGMENT_FUNC_TEAM_RED))
		nSegment = -1;
	else {	//don't drop in any children of control centers
		for (int32_t i = 0; i < SEGMENT_SIDE_COUNT; i++) {
			int32_t nChild = pSeg->m_children [i];
			if (IS_CHILD (nChild) && (SEGMENT (nChild)->m_function == SEGMENT_FUNC_REACTOR)) {
				nSegment = -1;
				break;
				}
			}
		}
	//bail if not far enough from original position
	if (nSegment > -1) {
		tempv = pSeg->Center ();
		nDist = simpleRouter [0].PathLength (*vPlayerPos, nPlayerSeg, tempv, nSegment, -1, WID_PASSABLE_FLAG, -1);
		if ((nDist < 0) || (nDist >= I2X (20) * nDepth))
			break;
		}
	nDepth--;
	}
#if TRACE
if (nSegment != -1)
	console.printf (CON_DBG, " dist=%x\n", nDist);
#endif
if (nSegment == -1) {
#if TRACE
	console.printf (1, "Warning: Unable to find a connected CSegment.  Picking a random one.\n");
#endif
	RETVAL (Rand (gameData.segData.nSegments))
	}
RETVAL (nSegment)
}

//	------------------------------------------------------------------------------------------------------

void DropPowerups (void)
{
ENTER (0, 0);
if (extraGameInfo [IsMultiGame].nSpawnDelay != 0) {
	int16_t	h = gameData.objData.nFirstDropped, i;
	while (h >= 0) {
		i = h;
		h = gameData.objData.dropInfo [i].nNextPowerup;
		if ((gameData.objData.dropInfo [i].nDropTime != 0x7FFFFFFF) && !MaybeDropNetPowerup (i, gameData.objData.dropInfo [i].nPowerupType, CHECK_DROP))
			break;
		}
	}
RETURN
}

//	------------------------------------------------------------------------------------------------------

void RespawnDestroyedWeapon (int16_t nObject)
{
ENTER (0, 0);
	int32_t	h = gameData.objData.nFirstDropped, i;

while (h >= 0) {
	i = h;
	h = gameData.objData.dropInfo [i].nNextPowerup;
	if ((gameData.objData.dropInfo [i].nObject == nObject) &&
		 (gameData.objData.dropInfo [i].nDropTime < 0)) {
		gameData.objData.dropInfo [i].nDropTime = 0;
		MaybeDropNetPowerup (i, gameData.objData.dropInfo [i].nPowerupType, CHECK_DROP);
		}
	}
RETURN
}

//	------------------------------------------------------------------------------------------------------

int32_t AddDropInfo (int16_t nObject, int16_t nPowerupType, int32_t nDropTime)
{
ENTER (0, 0);
	int32_t	h;

if (gameData.objData.nFreeDropped < 0)
	RETVAL (-1)
//AddPowerupInMine (nPowerupType);
h = gameData.objData.nFreeDropped;
gameData.objData.nFreeDropped = gameData.objData.dropInfo [h].nNextPowerup;
gameData.objData.dropInfo [h].nPrevPowerup = gameData.objData.nLastDropped;
gameData.objData.dropInfo [h].nNextPowerup = -1;
gameData.objData.dropInfo [h].nObject = nObject;
gameData.objData.dropInfo [h].nSignature = OBJECT (nObject)->Signature ();
gameData.objData.dropInfo [h].nPowerupType = nPowerupType;
gameData.objData.dropInfo [h].nDropTime = (nDropTime > 0) ? nDropTime : (extraGameInfo [IsMultiGame].nSpawnDelay <= 0) ? -1 : gameStates.app.nSDLTicks [0];
if (gameData.objData.nFirstDropped >= 0)
	gameData.objData.dropInfo [gameData.objData.nLastDropped].nNextPowerup = h;
else
	gameData.objData.nFirstDropped = h;
gameData.objData.nLastDropped = h;
gameData.objData.nDropped++;
RETVAL (h)
}

//	------------------------------------------------------------------------------------------------------

void DelDropInfo (int32_t h)
{
ENTER (0, 0);
	int32_t	i, j;

if (h < 0)
	RETURN;
RemovePowerupInMine (gameData.objData.dropInfo [h].nPowerupType);
i = gameData.objData.dropInfo [h].nPrevPowerup;
j = gameData.objData.dropInfo [h].nNextPowerup;
if (i < 0)
	gameData.objData.nFirstDropped = j;
else
	gameData.objData.dropInfo [i].nNextPowerup = j;
if (j < 0)
	gameData.objData.nLastDropped = i;
else
	gameData.objData.dropInfo [j].nPrevPowerup = i;
gameData.objData.dropInfo [h].nNextPowerup = gameData.objData.nFreeDropped;
gameData.objData.nFreeDropped = h;
gameData.objData.nDropped--;
RETURN
}

//	------------------------------------------------------------------------------------------------------

int32_t FindDropInfo (int32_t nSignature)
{
ENTER (0, 0);
	int16_t	i = gameData.objData.nFirstDropped;

while (i >= 0) {
	if (gameData.objData.dropInfo [i].nSignature == nSignature)
		RETVAL (i)
	i = gameData.objData.dropInfo [i].nNextPowerup;
	}
RETVAL (-1)
}

//	------------------------------------------------------------------------------------------------------
//	Drop cloak powerup if in a network game.
// nObject will contain a drop list index if MaybeDropNetPowerup is called with state CHECK_DROP

int32_t MaybeDropNetPowerup (int16_t nObject, int32_t nPowerupType, int32_t nDropState)
{
ENTER (0, 0);
if (EGI_FLAG (bImmortalPowerups, 0, 0, 0) || (IsMultiGame && !IsCoopGame)) {
#if 0
	if (IsNetworkGame && (nDropState < CHECK_DROP) && (nPowerupType >= 0)) {
		if (gameData.multiplayer.powerupsInMine [nPowerupType] >= gameData.multiplayer.maxPowerupsAllowed [nPowerupType])
			RETVAL (0)
		}
#endif
	if (gameData.reactorData.bDestroyed || gameStates.app.bEndLevelSequence)
		RETVAL (0)
	gameData.multigame.create.nCount = 0;
	
	if (IsMultiGame && (extraGameInfo [IsMultiGame].nSpawnDelay != 0)) {
		if (nDropState == CHECK_DROP) {
			if ((gameData.objData.dropInfo [nObject].nDropTime < 0) ||
				 (gameStates.app.nSDLTicks [0] - gameData.objData.dropInfo [nObject].nDropTime < extraGameInfo [IsMultiGame].nSpawnDelay))
				RETVAL (0)
			nDropState = EXEC_DROP;
			}
		else if (nDropState == INIT_DROP) {
			AddDropInfo (nObject, (int16_t) nPowerupType);
			RETVAL (0)
			}
		if (nDropState == EXEC_DROP) {
			DelDropInfo (nObject);
			}
		}
	else {
		if (IsMultiGame && (gameStates.multi.nGameType == UDP_GAME) && (nDropState == INIT_DROP) && OBJECT (nObject)->IsMissile ()) {
			//if (!(MultiPowerupIs4Pack (nPowerupType + 1) && MissingPowerups (nPowerupType + 1))) 
				AddDropInfo (nObject, nPowerupType, 0x7FFFFFFF); // respawn missiles only after their destruction
			RETVAL (0)
			}
		}

	if (0 > (nObject = PrepareObjectCreateEgg (OBJECT (LOCALPLAYER.nObject), 1, OBJ_POWERUP, nPowerupType, true)))
		RETVAL (0)

	CObject* pObj = OBJECT (nObject);
	int32_t bFixedPos = 0;
	int16_t nSegment = ChooseDropSegment (OBJECT (nObject), &bFixedPos, nDropState);
	if (0 > nSegment) {
		pObj->Die ();
		RETVAL (0)
		}
	pObj->mType.physInfo.velocity.SetZero ();

	CFixVector vNewPos;
	if (bFixedPos)
		vNewPos = pObj->info.position.vPos;
	else {
		CFixVector vOffset = SEGMENT (nSegment)->Center () - vNewPos;
		CFixVector::Normalize (vOffset);
		vNewPos = SEGMENT (nSegment)->RandomPoint ();
		vNewPos += vOffset * pObj->info.xSize;
		}
	nSegment = FindSegByPos (vNewPos, nSegment, 1, 0);
	MultiSendCreatePowerup (nPowerupType, nSegment, nObject, &vNewPos);
	if (!bFixedPos)
		pObj->info.position.vPos = vNewPos;
	pObj->RelinkToSeg (nSegment);
	CreateExplosion (pObj, nSegment, vNewPos, vNewPos, I2X (5), ANIM_POWERUP_DISAPPEARANCE);
	RETVAL (1)
	}
RETVAL (0)
}

//	------------------------------------------------------------------------------------------------------
//	Return true if current CSegment contains some CObject.
int32_t SegmentContainsObject (int32_t objType, int32_t obj_id, int32_t nSegment)
{
ENTER (0, 0);
if (nSegment == -1)
	RETVAL (0)
int32_t nObject = SEGMENT (nSegment)->m_objects;
while (nObject != -1) {
	CObject *pObj = OBJECT (nObject);
	if (!pObj)
		RETVAL (0)
	if ((pObj->Type () == objType) && (pObj->Id () == obj_id))
		RETVAL (1)
	else
		nObject = pObj->info.nNextInSeg;
	}
RETVAL (0)
}

//	------------------------------------------------------------------------------------------------------

int32_t ObjectNearbyAux (int32_t nSegment, int32_t objectType, int32_t object_id, int32_t depth)
{
ENTER (0, 0);
if (depth == 0)
	RETVAL (0)
if (SegmentContainsObject (objectType, object_id, nSegment))
	RETVAL (1)
CSegment* pSeg = SEGMENT (nSegment);
if (!pSeg)
	RETVAL (0)
for (int32_t i = 0; i < SEGMENT_SIDE_COUNT; i++) {
	int16_t nChildSeg = pSeg->m_children [i];
	if ((nChildSeg != -1) && ObjectNearbyAux (nChildSeg, objectType, object_id, depth-1))
		RETVAL (1)
	}
RETVAL (0)
}


//	------------------------------------------------------------------------------------------------------
//	Return true if some powerup is nearby (within 3 segments).
int32_t WeaponNearby (CObject *pObj, int32_t weapon_id)
{
return ObjectNearbyAux (pObj->info.nSegment, OBJ_POWERUP, weapon_id, 3);
}

//	------------------------------------------------------------------------------------------------------

void MaybeReplacePowerupWithEnergy (CObject *pDelObj)
{
ENTER (0, 0);
	int32_t	nWeapon = -1;

if (pDelObj->info.contains.nType != OBJ_POWERUP)
	RETURN

if (pDelObj->info.contains.nId == POW_CLOAK) {
	if (WeaponNearby (pDelObj, pDelObj->info.contains.nId)) {
#if TRACE
		console.printf (CON_DBG, "Bashing cloak into nothing because there's one nearby.\n");
#endif
		pDelObj->info.contains.nCount = 0;
	}
	RETURN
}
switch (pDelObj->info.contains.nId) {
	case POW_VULCAN:
		nWeapon = VULCAN_INDEX;
		break;
	case POW_GAUSS:
		nWeapon = GAUSS_INDEX;
		break;
	case POW_SPREADFIRE:
		nWeapon = SPREADFIRE_INDEX;
		break;
	case POW_PLASMA:
		nWeapon = PLASMA_INDEX;
		break;
	case POW_FUSION:
		nWeapon = FUSION_INDEX;
		break;
	case POW_HELIX:
		nWeapon = HELIX_INDEX;
		break;
	case POW_PHOENIX:
		nWeapon = PHOENIX_INDEX;
		break;
	case POW_OMEGA:
		nWeapon = OMEGA_INDEX;
		break;
	}

//	Don't drop vulcan ammo if player maxed out.
if (( (nWeapon == VULCAN_INDEX) || (pDelObj->info.contains.nId == POW_VULCAN_AMMO)) && (LOCALPLAYER.primaryAmmo [VULCAN_INDEX] >= VULCAN_AMMO_MAX))
	pDelObj->info.contains.nCount = 0;
else if (( (nWeapon == GAUSS_INDEX) || (pDelObj->info.contains.nId == POW_VULCAN_AMMO)) && (LOCALPLAYER.primaryAmmo [VULCAN_INDEX] >= VULCAN_AMMO_MAX))
	pDelObj->info.contains.nCount = 0;
else if (nWeapon != -1) {
	if ((PlayerHasWeapon (nWeapon, 0, -1, 1) & HAS_WEAPON_FLAG) || WeaponNearby (pDelObj, pDelObj->info.contains.nId)) {
		if (RandShort () > 16384) {
			pDelObj->info.contains.nType = OBJ_POWERUP;
			if (nWeapon == VULCAN_INDEX) {
				pDelObj->info.contains.nId = POW_VULCAN_AMMO;
				}
			else if (nWeapon == GAUSS_INDEX) {
				pDelObj->info.contains.nId = POW_VULCAN_AMMO;
				}
			else {
				pDelObj->info.contains.nId = POW_ENERGY;
				}
			}
		else {
			pDelObj->info.contains.nType = OBJ_POWERUP;
			pDelObj->info.contains.nId = POW_SHIELD_BOOST;
			}
		}
	}
else if (pDelObj->info.contains.nId == POW_QUADLASER)
	if ((LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS) || WeaponNearby (pDelObj, pDelObj->info.contains.nId)) {
		if (RandShort () > 16384) {
			pDelObj->info.contains.nType = OBJ_POWERUP;
			pDelObj->info.contains.nId = POW_ENERGY;
			}
		else {
			pDelObj->info.contains.nType = OBJ_POWERUP;
			pDelObj->info.contains.nId = POW_SHIELD_BOOST;
			}
		}

//	If this robot was gated in by the boss and it now contains energy, make it contain nothing,
//	else the room gets full of energy.
if ((pDelObj->info.nCreator == BOSS_GATE_PRODUCER_NUM) && (pDelObj->info.contains.nId == POW_ENERGY) && (pDelObj->info.contains.nType == OBJ_POWERUP)) {
#if TRACE
	console.printf (CON_DBG, "Converting energy powerup to nothing because robot %i gated in by boss.\n", OBJ_IDX (pDelObj));
#endif
	pDelObj->info.contains.nCount = 0;
	}

// Change multiplayer extra-lives into invulnerability
if (IsMultiGame && (pDelObj->info.contains.nId == POW_EXTRA_LIFE))
	pDelObj->info.contains.nId = POW_INVUL;
RETURN
}

//------------------------------------------------------------------------------

int32_t DropPowerup (uint8_t nType, uint8_t nId, int16_t owner, int32_t bDropExtras, const CFixVector& vInitVel, const CFixVector& vPos, int16_t nSegment, bool bLocal)
{
ENTER (0, 0);
	int16_t		nObject = -1;
	CObject*		pObj;
	CFixVector	vNewVel, vNewPos;
	fix			xOldMag, xNewMag;

switch (nType) {
	case OBJ_POWERUP:
		if (gameStates.app.bGameSuspended & SUSP_POWERUPS)
			RETVAL (-1)
		if (nId >= MAX_WEAPON_ID)
			RETVAL (-1)

		int32_t	nRandScale, nOffset;
		vNewVel = vInitVel;
		xOldMag = vInitVel.Mag ();

		//	We want powerups to move more in network mode.
		if (IsMultiGame && !gameData.appData.GameMode (GM_MULTI_ROBOTS)) {
			nRandScale = 4;
			//	extra life powerups are converted to invulnerability in multiplayer, for what is an extra life, anyway?
			if (nId == POW_EXTRA_LIFE)
				nId = POW_INVUL;
			}
		else
			nRandScale = 2;
		xNewMag = xOldMag + I2X (32);
		nOffset = 16384 * nRandScale;
		vNewVel.v.coord.x += FixMul (xNewMag, RandShort () * nRandScale - nOffset);
		vNewVel.v.coord.y += FixMul (xNewMag, RandShort () * nRandScale - nOffset);
		vNewVel.v.coord.z += FixMul (xNewMag, RandShort () * nRandScale - nOffset);
		// Give keys zero velocity so they can be tracked better in multi
		if (IsMultiGame && (((nId >= POW_KEY_BLUE) && (nId <= POW_KEY_GOLD)) || (nId == POW_MONSTERBALL)))
			vNewVel.SetZero ();
		vNewPos = vPos;

		if (IsMultiGame) {
			if (gameData.multigame.create.nCount >= MAX_NET_CREATE_OBJECTS) {
				RETVAL (-1)
				}
			if (IsNetworkGame && networkData.nStatus == NETSTAT_ENDLEVEL)
				RETVAL (-1)
			}
		nObject = CreatePowerup (nId, owner, nSegment, vNewPos, 0);
		pObj = OBJECT (nObject);
		if (!pObj) 
			RETVAL (-1)
		if (IsMultiGame) {
#if 0
			if ((gameStates.multi.nGameType == UDP_GAME) && !bLocal)
				MultiSendDropPowerup (nId, nSegment, nObject, &vNewPos, &vNewVel);
#endif
			gameData.multigame.create.nObjNums [gameData.multigame.create.nCount++] = nObject;
			}
		pObj->mType.physInfo.velocity = vNewVel;
		pObj->mType.physInfo.drag = 512;	//1024;
		pObj->mType.physInfo.mass = I2X (1);
		pObj->mType.physInfo.flags = PF_BOUNCES;
		pObj->rType.animationInfo.nClipIndex = gameData.objData.pwrUp.info [pObj->info.nId].nClipIndex;
		pObj->rType.animationInfo.xFrameTime = gameData.effectData.animations [0][pObj->rType.animationInfo.nClipIndex].xFrameTime;
		pObj->rType.animationInfo.nCurFrame = 0;

		switch (pObj->info.nId) {
			case POW_CONCUSSION_1:
			case POW_CONCUSSION_4:
			case POW_SHIELD_BOOST:
			case POW_ENERGY:
				pObj->SetLife ((RandShort () + I2X (3)) * 64);		//	Lives for 3 to 3.5 binary minutes (a binary minute is 64 seconds)
				if (IsMultiGame)
					pObj->SetLife (pObj->LifeLeft () / 2);
				break;
			default:
//						if (IsMultiGame)
//							pObj->SetLife ((RandShort () + I2X (3)) * 64);		//	Lives for 5 to 5.5 binary minutes (a binary minute is 64 seconds)
				break;
			}
		break;

	case OBJ_ROBOT: {
		vNewVel = vInitVel;
		xOldMag = vInitVel.Mag ();
		CFixVector::Normalize (vNewVel);
		vNewVel.v.coord.x += SRandShort () * 2;
		vNewVel.v.coord.y += SRandShort () * 2;
		vNewVel.v.coord.z += SRandShort () * 2;
		CFixVector::Normalize (vNewVel);
		vNewVel *= ((I2X (32) + xOldMag) * 2);
		vNewPos = vPos;
		if (0 > (nObject = CreateRobot (nId, nSegment, vNewPos)))
			RETVAL (nObject)
		if (IsMultiGame)
			gameData.multigame.create.nObjNums [gameData.multigame.create.nCount++] = nObject;
		pObj = OBJECT (nObject);
		tRobotInfo *pRobotInfo = ROBOTINFO (pObj);
		if (pRobotInfo) {
			//Set polygon-CObject-specific data
			pObj->rType.polyObjInfo.nModel = pRobotInfo->nModel;
			pObj->rType.polyObjInfo.nSubObjFlags = 0;
			//set Physics info
			pObj->mType.physInfo.velocity = vNewVel;
			pObj->mType.physInfo.mass = pRobotInfo->mass;
			pObj->mType.physInfo.drag = pRobotInfo->drag;
			pObj->mType.physInfo.flags |= (PF_LEVELLING);
			pObj->SetShield (pRobotInfo->strength);
			pObj->cType.aiInfo.behavior = AIB_NORMAL;
			gameData.aiData.localInfo [nObject].targetAwarenessType = WEAPON_ROBOT_COLLISION;
			gameData.aiData.localInfo [nObject].targetAwarenessTime = I2X (3);
			pObj->cType.aiInfo.CURRENT_STATE = AIS_LOCK;
			pObj->cType.aiInfo.GOAL_STATE = AIS_LOCK;
			pObj->cType.aiInfo.REMOTE_OWNER = -1;
			if (pObj->IsBoss ())
				gameData.bossData.Add (nObject);
			}

			// At JasenW's request, robots which contain robots sometimes drop shield.
			if (bDropExtras && (RandShort () > 16384)) {
				AddAllowedPowerup (POW_SHIELD_BOOST);
				DropPowerup (OBJ_POWERUP, POW_SHIELD_BOOST, -1, 0, vInitVel, vPos, nSegment);
				}
			}
		break;

	default:
		PrintLog (0, "Warning: Illegal object type %d in function DropPowerup.\n", nType);
	}
RETVAL (nObject)
}

// ----------------------------------------------------------------------------
// Returns created CObject number. If object dropped by player, set flag.

int32_t CObject::CreateEgg (bool bLocal, bool bUpdateLimits)
{
ENTER (0, 0);
	int32_t	i, nObject = -1;

if ((info.nType != OBJ_PLAYER) && (info.contains.nType == OBJ_POWERUP)) {
	if (IsMultiGame) {
		if (bUpdateLimits)
			AddAllowedPowerup (info.contains.nId, info.contains.nCount);
		}
	else {
		if (info.contains.nId == POW_SHIELD_BOOST) {
			if (LOCALPLAYER.Shield () >= I2X (100)) {
				if (RandShort () > 16384) {
					RETVAL (-1)
					}
				} 
			else if (LOCALPLAYER.Shield () >= I2X (150)) {
				if (RandShort () > 8192) {
					RETVAL (-1)
					}
				}
			}
		else if (info.contains.nId == POW_ENERGY) {
			if (LOCALPLAYER.Energy () >= I2X (100)) {
				if (RandShort () > 16384) {
					RETVAL (-1)
					}
				} 
			else if (LOCALPLAYER.Energy () >= I2X (150)) {
				if (RandShort () > 8192) {
					RETVAL (-1)
					}
				}
			}
		}
	}

for (i = info.contains.nCount; i; i--) {
	nObject = DropPowerup (info.contains.nType, uint8_t (info.contains.nId), Index (), i == 1, // drop extra powerups?
								  mType.physInfo.velocity, info.position.vPos, info.nSegment, bLocal);
	CObject *pObj = OBJECT (nObject);
	if (!pObj)
		RETVAL (-1)
	if (info.nType == OBJ_PLAYER) {
		if (info.nId == N_LOCALPLAYER)
			pObj->info.nFlags |= OF_PLAYER_DROPPED;
		}
	else if (info.nType == OBJ_ROBOT) {
		if (info.contains.nType == OBJ_POWERUP) {
			if ((info.contains.nId == POW_VULCAN) || (info.contains.nId == POW_GAUSS))
				pObj->cType.powerupInfo.nCount = VULCAN_WEAPON_AMMO_AMOUNT;
			else if (info.contains.nId == POW_OMEGA)
				pObj->cType.powerupInfo.nCount = MAX_OMEGA_CHARGE;
			}
		}
	}
RETVAL (nObject)
}

// -- extern int32_t Items_destroyed;

//	-------------------------------------------------------------------------------------------------------
//	Put count OBJECTS of nType nType (eg, powerup), id = id (eg, energy) into *pObj, then drop them! Yippee!
//	Returns created CObject number.
int32_t PrepareObjectCreateEgg (CObject *pObj, int32_t nCount, int32_t nType, int32_t nId, bool bLocal, bool bUpdateLimits)
{
ENTER (0, 0);
if (nCount <= 0)
	RETVAL (-1)
pObj->info.contains.nCount = nCount;
pObj->info.contains.nType = nType;
pObj->info.contains.nId = nId;
RETVAL (pObj->CreateEgg (bLocal, bUpdateLimits))
}

//------------------------------------------------------------------------------
//creates afterburner blobs behind the specified CObject
void DropAfterburnerBlobs (CObject *pObj, int32_t count, fix xSizeScale, fix xLifeTime, CObject *pParent, int32_t bThruster)
{
ENTER (0, 0);
	int16_t			i, nSegment, nThrusters;
	CObject			*pBlobObj;
	tThrusterInfo	ti;

nThrusters = thrusterFlames.CalcPos (pObj, &ti, 1);
for (i = 0; i < nThrusters; i++) {
	nSegment = FindSegByPos (ti.vPos [i], pObj->info.nSegment, 1, 0);
	if (nSegment == -1)
		continue;
	if (!(pBlobObj = CreateExplosion (nSegment, ti.vPos [i], xSizeScale, ANIM_AFTERBURNER_BLOB)))
		continue;
	if (xLifeTime != -1) {
		pBlobObj->rType.animationInfo.xTotalTime = xLifeTime;
		pBlobObj->rType.animationInfo.xFrameTime = FixMulDiv (gameData.effectData.animations [0][ANIM_AFTERBURNER_BLOB].xFrameTime,
																		  xLifeTime, pBlobObj->info.xLifeLeft);
		pBlobObj->SetLife (xLifeTime);
		}
	AddChildObjectP (pParent, pBlobObj);
	pBlobObj->info.renderType = RT_THRUSTER;
	if (bThruster)
		pBlobObj->mType.physInfo.flags |= PF_WIGGLE;
	}
RETURN
}

//	-----------------------------------------------------------------------------

int32_t MaybeDropPrimaryWeaponEgg (CObject *pPlayerObj, int32_t nWeapon)
{
ENTER (0, 0);
	int32_t nWeaponFlag = HAS_FLAG (nWeapon);

if (!(PLAYER (pPlayerObj->info.nId).primaryWeaponFlags & nWeaponFlag))
	RETVAL (-1)
if ((nWeapon == 4) && gameData.weaponData.bTripleFusion)
	gameData.weaponData.bTripleFusion = 0;
else if (gameStates.app.bHaveExtraGameInfo [IsMultiGame] && (extraGameInfo [IsMultiGame].loadout.nGuns & nWeaponFlag))
	RETVAL (-1)
RETVAL (PrepareObjectCreateEgg (pPlayerObj, 1, OBJ_POWERUP, primaryWeaponToPowerup [nWeapon]))
}

//	-----------------------------------------------------------------------------

void MaybeDropSecondaryWeaponEgg (CObject *pPlayerObj, int32_t nWeapon, int32_t count)
{
ENTER (0, 0);
	int32_t nWeaponFlag = HAS_FLAG (nWeapon);
	int32_t nPowerup = secondaryWeaponToPowerup [0][nWeapon];

if (PLAYER (pPlayerObj->info.nId).secondaryWeaponFlags & nWeaponFlag) {
	int32_t i, maxCount = ((EGI_FLAG (bDropAllMissiles, 0, 0, 0)) ? count : min(count, 3));

	for (i = 0; i < maxCount; i++)
		PrepareObjectCreateEgg (pPlayerObj, 1, OBJ_POWERUP, nPowerup);
	}
RETURN
}

//	-----------------------------------------------------------------------------

void MaybeDropDeviceEgg (CPlayerInfo *pPlayer, CObject *pPlayerObj, int32_t nDeviceFlag, int32_t nPowerupId)
{
ENTER (0, 0);
if ((PLAYER (pPlayerObj->info.nId).flags & nDeviceFlag) &&
	 !(gameStates.app.bHaveExtraGameInfo [IsMultiGame] && (extraGameInfo [IsMultiGame].loadout.nDevice & nDeviceFlag)))
	PrepareObjectCreateEgg (pPlayerObj, 1, OBJ_POWERUP, nPowerupId);
RETURN
}

//	-----------------------------------------------------------------------------

void DropMissile1or4 (CObject *pPlayerObj, int32_t nMissileIndex)
{
ENTER (0, 0);
	int32_t nMissiles, nPowerupId;

if (0 < (nMissiles = PLAYER (pPlayerObj->info.nId).secondaryAmmo [nMissileIndex])) {
#if DBG
	if (nMissiles > 40)
		BRP;
#endif
	nPowerupId = secondaryWeaponToPowerup [0][nMissileIndex];
	if ((nMissileIndex == CONCUSSION_INDEX) && (pPlayerObj->Id () == N_LOCALPLAYER))
		nMissiles -= gameData.multiplayer.weaponStates [pPlayerObj->Id ()].nBuiltinMissiles;	//player gets 4 concs anyway when respawning, so avoid them building up
	if (nMissiles > 0) {
		if (!(IsMultiGame || EGI_FLAG (bDropAllMissiles, 0, 0, 0)) && (nMissiles > 10))
			nMissiles = 10;
		PrepareObjectCreateEgg (pPlayerObj, nMissiles / 4, OBJ_POWERUP, nPowerupId + 1);
		PrepareObjectCreateEgg (pPlayerObj, nMissiles % 4, OBJ_POWERUP, nPowerupId);
		}
	}
RETURN
}

// -- int32_t	Items_destroyed = 0;

//	-----------------------------------------------------------------------------
//	If the player had mines, maybe arm up to 3 of them.

static void MaybeArmMines (CObject *pPlayerObj, CPlayerInfo* pPlayer, int32_t nType, int32_t nId)
{
ENTER (0, 0);
if (gameStates.multi.nGameType == UDP_GAME) {
	int32_t nAmmo = pPlayer->secondaryAmmo [nType];
	if (nAmmo <= 0)
		RETURN
	if (nAmmo > 4)
		nAmmo = 4;
	for (nAmmo = Rand (nAmmo); nAmmo; nAmmo--) {
		CFixVector vRandom = CFixVector::Random ();
		CFixVector vDropPos = pPlayerObj->info.position.vPos + vRandom;
		int16_t nNewSeg = FindSegByPos (vDropPos, pPlayerObj->info.nSegment, 1, 0);
		if (nNewSeg == -1)
			RETURN
		int16_t nObject = CreateNewWeapon (&vRandom, &vDropPos, nNewSeg, OBJ_IDX (pPlayerObj), nId, 0);
		if (!OBJECT (nObject))
			RETURN
	#if 0
		if (IsMultiGame && (gameStates.multi.nGameType == UDP_GAME))
			MultiSendCreateWeapon (nObject);
	#endif
  		}
	}
else {
	for (int32_t nThreshold = 30000; (pPlayer->secondaryAmmo [nType] % 4 == 1) && (RandShort () < nThreshold); nThreshold /= 2) {
		CFixVector vRandom = CFixVector::Random ();
		nThreshold /= 2;
		CFixVector vDropPos = pPlayerObj->info.position.vPos + vRandom;
		int16_t nNewSeg = FindSegByPos (vDropPos, pPlayerObj->info.nSegment, 1, 0);
		if (nNewSeg == -1)
			RETURN
		int16_t nObject = CreateNewWeapon (&vRandom, &vDropPos, nNewSeg, OBJ_IDX (pPlayerObj), nId, 0);
		if (!OBJECT (nObject))
			RETURN
	#if 0
		if (IsMultiGame && (gameStates.multi.nGameType == UDP_GAME))
			MultiSendCreateWeapon (nObject);
	#endif
  		}
	}
RETURN
}

//	-----------------------------------------------------------------------------

void DropPlayerEggs (CObject *pPlayerObj)
{
ENTER (0, 0);
if (pPlayerObj && ((pPlayerObj->info.nType == OBJ_PLAYER) || (pPlayerObj->info.nType == OBJ_GHOST))) {
	int32_t			nPlayer = pPlayerObj->info.nId;
	int16_t			nObject;
	int32_t			nVulcanAmmo = 0;
	CPlayerData*	pPlayer = gameData.multiplayer.players + nPlayer;
	int32_t			bResetLasers = !IsMultiGame || (nPlayer != N_LOCALPLAYER);

	// Seed the Random number generator so in net play the eggs will always drop the same way
	PrintLog (1, "dropping player equipment\n");
	if (IsMultiGame) {
		gameData.multigame.create.nCount = 0;
		gameStates.app.SRand ((gameStates.multi.nGameType == UDP_GAME) ? gameStates.app.nRandSeed : 5483L);
		}
	MaybeArmMines (pPlayerObj, pPlayer, SMARTMINE_INDEX, SMARTMINE_ID);
	if (IsMultiGame && !(IsHoardGame || IsEntropyGame))
		MaybeArmMines (pPlayerObj, pPlayer, PROXMINE_INDEX, PROXMINE_ID);

	//	If the player dies and he has powerful lasers, create the powerups here.
	if (pPlayer->LaserLevel (1)) {
		if (!IsBuiltinWeapon (SUPER_LASER_INDEX)) {
			PrepareObjectCreateEgg (pPlayerObj, pPlayer->LaserLevel (1), OBJ_POWERUP, POW_SUPERLASER);
			PrepareObjectCreateEgg (pPlayerObj, MAX_LASER_LEVEL, OBJ_POWERUP, POW_LASER);
			if (bResetLasers)
				pPlayer->SetSuperLaser (0);
			}
		}
	if (pPlayer->LaserLevel (0) > 0) {
		if (!(IsBuiltinWeapon (LASER_INDEX) || IsBuiltinWeapon (SUPER_LASER_INDEX))) {
			PrepareObjectCreateEgg (pPlayerObj, pPlayer->LaserLevel (0), OBJ_POWERUP, POW_LASER);	// Note: laserLevel = 0 for laser level 1.
			if (bResetLasers)
				pPlayer->SetStandardLaser (0);
			}
		}

	//	Drop quad laser if appropos
	MaybeDropDeviceEgg (pPlayer, pPlayerObj, PLAYER_FLAGS_QUAD_LASERS, POW_QUADLASER);
	MaybeDropDeviceEgg (pPlayer, pPlayerObj, PLAYER_FLAGS_CLOAKED, POW_CLOAK);
	while (pPlayer->nInvuls--)
		PrepareObjectCreateEgg (pPlayerObj, 1, OBJ_POWERUP, POW_INVUL);
	while (pPlayer->nCloaks--)
		PrepareObjectCreateEgg (pPlayerObj, 1, OBJ_POWERUP, POW_CLOAK);
	MaybeDropDeviceEgg (pPlayer, pPlayerObj, PLAYER_FLAGS_FULLMAP, POW_FULL_MAP);
	MaybeDropDeviceEgg (pPlayer, pPlayerObj, PLAYER_FLAGS_AFTERBURNER, POW_AFTERBURNER);
	MaybeDropDeviceEgg (pPlayer, pPlayerObj, PLAYER_FLAGS_AMMO_RACK, POW_AMMORACK);
	MaybeDropDeviceEgg (pPlayer, pPlayerObj, PLAYER_FLAGS_CONVERTER, POW_CONVERTER);
	if (!IsMultiGame) {
		MaybeDropDeviceEgg (pPlayer, pPlayerObj, PLAYER_FLAGS_SLOWMOTION, POW_SLOWMOTION);
		MaybeDropDeviceEgg (pPlayer, pPlayerObj, PLAYER_FLAGS_BULLETTIME, POW_BULLETTIME);
		}	
	if (PlayerHasHeadlight (nPlayer) && !(extraGameInfo [IsMultiGame].loadout.nDevice & PLAYER_FLAGS_HEADLIGHT) &&
		 !(gameStates.app.bHaveExtraGameInfo [1] && IsMultiGame && extraGameInfo [1].bDarkness))
		MaybeDropDeviceEgg (pPlayer, pPlayerObj, PLAYER_FLAGS_HEADLIGHT, POW_HEADLIGHT);
	// drop the other enemies flag if you have it

	pPlayer->nInvuls =
	pPlayer->nCloaks = 0;
	pPlayer->flags &= ~(PLAYER_FLAGS_INVULNERABLE | PLAYER_FLAGS_CLOAKED);
	if ((gameData.appData.GameMode (GM_CAPTURE)) && (pPlayer->flags & PLAYER_FLAGS_FLAG))
		PrepareObjectCreateEgg (pPlayerObj, 1, OBJ_POWERUP, (GetTeam (nPlayer) == TEAM_RED) ? POW_BLUEFLAG : POW_REDFLAG);

#if !DBG
	if (gameData.appData.GameMode (GM_HOARD | GM_ENTROPY))
#endif
	if (IsHoardGame || (IsEntropyGame && extraGameInfo [1].entropy.nVirusStability)) {
		// Drop hoard orbs

		int32_t maxCount, i;
#if TRACE
		console.printf (CON_DBG, "HOARD MODE: Dropping %d orbs \n", pPlayer->secondaryAmmo [PROXMINE_INDEX]);
#endif
		maxCount = pPlayer->secondaryAmmo [PROXMINE_INDEX];
		if (IsHoardGame && (maxCount > 12))
			maxCount = 12;
		for (i = 0; i < maxCount; i++)
			PrepareObjectCreateEgg (pPlayerObj, 1, OBJ_POWERUP, POW_HOARD_ORB);
		}

	//Drop the vulcan, gauss, and ammo
	nVulcanAmmo = pPlayer->primaryAmmo [VULCAN_INDEX] + gameData.multiplayer.weaponStates [nPlayer].nAmmoUsed;
	if (!IsMultiGame || gameStates.app.bHaveExtraGameInfo [1]) {
		int32_t nGunObjs [2] = {-1, -1};
		int32_t nGunIds [2] = {VULCAN_INDEX, GAUSS_INDEX};
		int32_t nGunAmmo [2] = {VULCAN_WEAPON_AMMO_AMOUNT, GAUSS_WEAPON_AMMO_AMOUNT};
		int32_t i;

		gameData.multiplayer.weaponStates [nPlayer].nAmmoUsed = 0;
		if (0 < (i = nVulcanAmmo / VULCAN_CLIP_CAPACITY - 1)) {	// drop ammo in excess of presupplied Vulcan/Gauss ammo as vulcan ammo packs
			PrepareObjectCreateEgg (pPlayerObj, i, OBJ_POWERUP, POW_VULCAN_AMMO);
			nVulcanAmmo -= i * VULCAN_CLIP_CAPACITY;
			if (nVulcanAmmo < 0)
				nVulcanAmmo = 0;
			}
		for (i = 0; i < 2; i++) {
			if (IsBuiltinWeapon (nGunIds [i]))
				nVulcanAmmo -= nGunAmmo [i];
			else if (pPlayer->primaryWeaponFlags & HAS_FLAG (nGunIds [i]))
				nGunObjs [i] = MaybeDropPrimaryWeaponEgg (pPlayerObj, nGunIds [i]);
			}
		if ((nGunObjs [0] >= 0) && (nGunObjs [1] >= 0))
			nVulcanAmmo /= 2;
		for (i = 0; i < 2; i++) {
			if (nGunObjs [i] >= 0)
				OBJECT (nGunObjs [i])->cType.powerupInfo.nCount = nVulcanAmmo;
			}
		}
	//	Drop the rest of the primary weapons
	MaybeDropPrimaryWeaponEgg (pPlayerObj, SPREADFIRE_INDEX);
	MaybeDropPrimaryWeaponEgg (pPlayerObj, PLASMA_INDEX);
	if (gameData.weaponData.bTripleFusion)
		MaybeDropPrimaryWeaponEgg (pPlayerObj, FUSION_INDEX);
	MaybeDropPrimaryWeaponEgg (pPlayerObj, FUSION_INDEX);
	MaybeDropPrimaryWeaponEgg (pPlayerObj, HELIX_INDEX);
	MaybeDropPrimaryWeaponEgg (pPlayerObj, PHOENIX_INDEX);
	nObject = MaybeDropPrimaryWeaponEgg (pPlayerObj, OMEGA_INDEX);
	if (nObject >= 0)
		OBJECT (nObject)->cType.powerupInfo.nCount =
			(pPlayerObj->info.nId == N_LOCALPLAYER) ? gameData.omegaData.xCharge [IsMultiGame] : DEFAULT_MAX_OMEGA_CHARGE;
	//	Drop the secondary weapons
	//	Note, proximity weapon only comes in packets of 4.  So drop n/2, but a max of 3 (handled inside maybe_drop..)  Make sense?
	if (!(gameData.appData.GameMode (GM_HOARD | GM_ENTROPY)))
		MaybeDropSecondaryWeaponEgg (pPlayerObj, PROXMINE_INDEX, (pPlayer->secondaryAmmo [PROXMINE_INDEX])/4);
	MaybeDropSecondaryWeaponEgg (pPlayerObj, SMART_INDEX, pPlayer->secondaryAmmo [SMART_INDEX]);
	MaybeDropSecondaryWeaponEgg (pPlayerObj, MEGA_INDEX, pPlayer->secondaryAmmo [MEGA_INDEX]);
	if (!IsEntropyGame)
		MaybeDropSecondaryWeaponEgg (pPlayerObj, SMARTMINE_INDEX, (pPlayer->secondaryAmmo [SMARTMINE_INDEX])/4);
	MaybeDropSecondaryWeaponEgg (pPlayerObj, EARTHSHAKER_INDEX, pPlayer->secondaryAmmo [EARTHSHAKER_INDEX]);
	//	Drop the player's missiles in packs of 1 and/or 4
	DropMissile1or4 (pPlayerObj, HOMING_INDEX);
	DropMissile1or4 (pPlayerObj, GUIDED_INDEX);
	DropMissile1or4 (pPlayerObj, CONCUSSION_INDEX);
	DropMissile1or4 (pPlayerObj, FLASHMSL_INDEX);
	DropMissile1or4 (pPlayerObj, MERCURY_INDEX);

		//	Always drop a shield and energy powerup.
	if (IsMultiGame && !gameStates.app.bChangingShip) {
		PrepareObjectCreateEgg (pPlayerObj, 1, OBJ_POWERUP, POW_SHIELD_BOOST);
		PrepareObjectCreateEgg (pPlayerObj, 1, OBJ_POWERUP, POW_ENERGY);
		}
	PrintLog (-1);
	}
RETURN
}

//	----------------------------------------------------------------------------
// Drop excess ammo when the ammo rack is stolen from the player

void DropExcessAmmo (void)
{
ENTER (0, 0);
for (int32_t nWeapon = CONCUSSION_INDEX; nWeapon <= EARTHSHAKER_INDEX; nWeapon++) {
	int32_t nExcess = MaxSecondaryAmmo (nWeapon) - LOCALPLAYER.secondaryAmmo [nWeapon];
	if (nExcess > 0) {
		if (nExcess >= 4)
			DropSecondaryWeapon (nWeapon, nExcess / 4, 1);
		DropSecondaryWeapon (nWeapon, nExcess % 4, 1);
		}
	}
int32_t nExcess = LOCALPLAYER.primaryAmmo [VULCAN_INDEX] - nMaxPrimaryAmmo [VULCAN_INDEX];
if (nExcess > 0) {
	int32_t nClips = (nExcess + VULCAN_CLIP_CAPACITY - 1) / VULCAN_CLIP_CAPACITY;
	LOCALPLAYER.primaryAmmo [VULCAN_INDEX] -= nClips * VULCAN_CLIP_CAPACITY;
	if (LOCALPLAYER.primaryAmmo [VULCAN_INDEX] < 0)
		LOCALPLAYER.primaryAmmo [VULCAN_INDEX] = 0;
	PrepareObjectCreateEgg (LOCALOBJECT, nClips, OBJ_POWERUP, POW_VULCAN_AMMO);
	}
RETURN
}

//	------------------------------------------------------------------------------------------------------

int32_t ReturnFlagHome (CObject *pObj)
{
ENTER (0, 0);
	CObject	*pInitObj;

if (gameStates.app.bHaveExtraGameInfo [1] && extraGameInfo [1].bEnhancedCTF) {
	if (SEGMENT (pObj->info.nSegment)->m_function == ((pObj->info.nId == POW_REDFLAG) ? SEGMENT_FUNC_GOAL_RED : SEGMENT_FUNC_GOAL_BLUE))
		RETVAL (pObj->info.nSegment)
	if ((pInitObj = FindInitObject (pObj))) {
		pObj->info.position.vPos = pInitObj->info.position.vPos;
		pObj->info.position.mOrient = pInitObj->info.position.mOrient;
		pObj->RelinkToSeg (pInitObj->info.nSegment);
		HUDInitMessage (TXT_FLAG_RETURN);
		audio.PlaySound (SOUND_DROP_WEAPON);
		MultiSendReturnFlagHome (pObj->Index ());
		}
	}
RETVAL (pObj->info.nSegment)
}

//------------------------------------------------------------------------------
//eof
