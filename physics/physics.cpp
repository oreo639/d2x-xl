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

#include <stdio.h>
#include <stdlib.h>

#include "descent.h"
#include "joy.h"
#include "error.h"
#include "physics.h"
#include "key.h"
#include "collide.h"
#include "timer.h"
#include "network.h"
#include "segmath.h"
#include "kconfig.h"
#include "automap.h"
#ifdef FORCE_FEEDBACK
#	include "tactile.h"
#endif

//Global variables for physics system

#define DBG_PHYSICS 0

#if DBG_PHYSICS
static int32_t bNewPhysCode = 1;
#endif

#define UNSTICK_OBJS		1

#define ROLL_RATE 		0x2000
#define DAMP_ANG 			0x400                  //min angle to bank

#define TURNROLL_SCALE	 (0x4ec4/2)

#define MAX_OBJECT_VEL (I2X (100) + extraGameInfo [IsMultiGame].nSpeedScale * I2X (100) / 4)

#define BUMP_HACK	1	//if defined, bump CPlayerData when he gets stuck
#define DAMPEN_KICKBACK 0 // if defined, ship will not bounce endlessly back from walls while having thrust

int32_t bFloorLeveling = 0;

fix CheckVectorObjectCollision (CHitData& hitData, CFixVector *p0, CFixVector *p1, fix rad, CObject *pThisObj, CObject *pOtherObj, int32_t nCollisionModel, bool bCheckVisibility);

//	-----------------------------------------------------------------------------

#if DBG

#define CATCH_OBJ(_objP,_cond) {if (((_objP) == dbgObjP) && (_cond)) CatchDbgObj (_cond);}

int32_t CatchDbgObj (int32_t cond)
{
if (cond)
	return 1;
return 0;
}

#else

#define CATCH_OBJ(_objP,_cond)

#endif

//	-----------------------------------------------------------------------------

static int32_t FindBestAlignedSide (int16_t nSegment, CFloatVector3& vPos, CFixVector& vDir, float& maxDot, int32_t nDepth)
{
ENTER (0, 0);
if (nSegment < 0)
	RETVAL (-1)
if (nDepth > 2)
	RETVAL (-1)

	CSegment*		pSeg = SEGMENT (nSegment);
	CSide*			pSide = pSeg->Side (0);
	int32_t				nBestSide = -1;
// bank CPlayerData according to CSegment orientation
//find CSide of CSegment that CPlayerData is most aligned with
for (int32_t nSide = 0; nSide < SEGMENT_SIDE_COUNT; nSide++, pSide++) {
	if (!pSide->FaceCount ())
		continue;
	if (pSide->Shape () > SIDE_SHAPE_TRIANGLE)
		continue;
	float dist = DistToFace (vPos, nSegment, nSide);
	if (dist > 60.0f)
		continue;
	if (pSeg->IsSolid (nSide)) {
		float dot = (float) CFixVector::Dot (pSide->m_normals [2], vDir) / dist;
		if (maxDot < dot) {
			maxDot = dot; 
			nBestSide = (nSegment << 3) | nSide;
			}
		}
	else {
		int32_t h = FindBestAlignedSide (pSeg->ChildId (nSide), vPos, vDir, maxDot, nDepth + 1);
		if (h >= 0)
			nBestSide = h;
		}
	}
RETVAL (nBestSide)
}

//	-----------------------------------------------------------------------------

void DoPhysicsAlignObject (CObject * pObj)
{
ENTER (0, 0);
if (!pObj->mType.physInfo.rotThrust.IsZero ())
	RETURN

	CFixVector		desiredUpVec;
	float				maxDot = -1.0f;
	CFloatVector3	vPos;

vPos.Assign (pObj->info.position.vPos);
int32_t nBestSide = FindBestAlignedSide (pObj->Segment (), vPos, pObj->info.position.mOrient.m.dir.u, maxDot, 0);
if (nBestSide < 0)
	RETURN
CSegment* pSeg = SEGMENT (nBestSide >> 3);
nBestSide &= 7;
// bank CPlayerData according to CSegment orientation
//find CSide of CSegment that CPlayerData is most aligned with
if (gameOpts->gameplay.nAutoLeveling == 1)	// old way: used floor's Normal as upvec
	desiredUpVec = pSeg->m_sides [3].FaceCount () ? pSeg->m_sides [3].m_normals [2] : pSeg->m_sides [nBestSide].m_normals [2];
else if (gameOpts->gameplay.nAutoLeveling == 2) {	 // new CPlayerData leveling code: use Normal of CSide closest to our up vec
	CSide* pSide = pSeg->m_sides + nBestSide;
	desiredUpVec = pSide->m_normals [2];
	}
else if (gameOpts->gameplay.nAutoLeveling == 3)	// mine's up vector
	desiredUpVec = (*PlayerSpawnOrient (N_LOCALPLAYER)).m.dir.u;
else
	RETURN
if (labs (CFixVector::Dot (desiredUpVec, pObj->info.position.mOrient.m.dir.f)) < I2X (1)/2) {
	CAngleVector turnAngles;

	CFixMatrix m = CFixMatrix::CreateFU(pObj->info.position.mOrient.m.dir.f, desiredUpVec);
//	m = CFixMatrix::CreateFU(pObj->info.position.mOrient.m.v.f, &desiredUpVec, NULL);
	fixang deltaAngle = CFixVector::DeltaAngle(pObj->info.position.mOrient.m.dir.u, m.m.dir.u, &pObj->info.position.mOrient.m.dir.f);
	deltaAngle += pObj->mType.physInfo.turnRoll;
	if (abs (deltaAngle) > /*DAMP_ANG*/1) {
		CFixMatrix mRotate, new_pm;

		fixang rollAngle = (fixang) FixMul (gameData.physicsData.xTime, ROLL_RATE);
		if (abs (deltaAngle) < rollAngle)
			rollAngle = deltaAngle;
		else if (deltaAngle < 0)
			rollAngle = -rollAngle;
		turnAngles.v.coord.p = turnAngles.v.coord.h = 0;
		turnAngles.v.coord.b = rollAngle;
		mRotate = CFixMatrix::Create(turnAngles);
		new_pm = pObj->info.position.mOrient * mRotate;
		pObj->info.position.mOrient = new_pm;
		}
#if 0
	else
		gameOpts->gameplay.nAutoLeveling = 0;
#endif
	}
RETURN
}

//	-----------------------------------------------------------------------------

void CObject::SetTurnRoll (void)
{
//if (!gameStates.app.bD1Mission)
{
	fixang desired_bank = (fixang) -FixMul (mType.physInfo.rotVel.v.coord.y, TURNROLL_SCALE);
	if (mType.physInfo.turnRoll != desired_bank) {
		fixang deltaAngle, max_roll;
		max_roll = (fixang) FixMul (ROLL_RATE, gameData.physicsData.xTime);
		deltaAngle = desired_bank - mType.physInfo.turnRoll;
		if (labs (deltaAngle) < max_roll)
			max_roll = deltaAngle;
		else
			if (deltaAngle < 0)
				max_roll = -max_roll;
		mType.physInfo.turnRoll += max_roll;
		}
	}
}

#if DBG
#define EXTRADBG 1		//no extra debug when NDEBUG is on
#endif

#ifdef EXTRADBG
CObject *debugObjP=NULL;
#endif

#if DBG
int32_t	nTotalRetries=0, nTotalSims=0;
int32_t	bDontMoveAIObjects=0;
#endif

#define FT (I2X (1)/64)

//	-----------------------------------------------------------------------------
// add rotational velocity & acceleration

int32_t CObject::DoPhysicsSimRot (void)
{
ENTER (0, 0);
	CAngleVector	turnAngles;
	CFixMatrix		mRotate, mNewOrient;

if (gameData.physicsData.xTime <= 0)
	RETVAL (0)
if (mType.physInfo.rotVel.IsZero () && mType.physInfo.rotThrust.IsZero ())
	RETVAL (0)
if (mType.physInfo.drag) {
	CFixVector	accel;
	int32_t		nTries = gameData.physicsData.xTime / FT;
	fix			r = gameData.physicsData.xTime % FT;
	fix			k = FixDiv (r, FT);
	fix			xDrag = (mType.physInfo.drag * 5) / 2;
	fix			xScale = I2X (1) - xDrag;

	if (this == gameData.objData.pConsole)
		xDrag = EGI_FLAG (nDrag, 0, 0, 0) * xDrag / 10;
	if (mType.physInfo.flags & PF_USES_THRUST) {
		accel = mType.physInfo.rotThrust * FixDiv (I2X (1), mType.physInfo.mass);
		while (nTries--) {
			mType.physInfo.rotVel += accel;
			mType.physInfo.rotVel *= xScale;
			}
		//do linear scale on remaining bit of time
		mType.physInfo.rotVel += accel * k;
		if (xDrag)
			mType.physInfo.rotVel *= (I2X (1) - FixMul (k, xDrag));
		}
	else if (xDrag && !(mType.physInfo.flags & PF_FREE_SPINNING)) {
		fix xTotalDrag = I2X (1);
		while (nTries--)
			xTotalDrag = FixMul (xTotalDrag, xScale);
		//do linear scale on remaining bit of time
		xTotalDrag = FixMul (xTotalDrag, I2X (1) - FixMul (k, xDrag));
		mType.physInfo.rotVel *= xTotalDrag;
		}
	}
//now rotate CObject
//unrotate CObject for bank caused by turn
if (mType.physInfo.turnRoll) {
	CFixMatrix mOrient;

	turnAngles.v.coord.p = turnAngles.v.coord.h = 0;
	turnAngles.v.coord.b = -mType.physInfo.turnRoll;
	mRotate = CFixMatrix::Create (turnAngles);
	mOrient = info.position.mOrient * mRotate;
	info.position.mOrient = mOrient;
	}

fix t = OBSERVING ? gameData.physicsData.xTime * 2 : gameData.physicsData.xTime;
turnAngles.v.coord.p = fixang (FixMul (mType.physInfo.rotVel.v.coord.x, t));
turnAngles.v.coord.h = fixang (FixMul (mType.physInfo.rotVel.v.coord.y, t));
turnAngles.v.coord.b = fixang (FixMul (mType.physInfo.rotVel.v.coord.z, t));
if (!IsMultiGame) {
	int32_t i = (this != gameData.objData.pConsole) ? 0 : 1;
	float fSpeed = gameStates.gameplay.slowmo [i].fSpeed;
	if (fSpeed != 1) {
		turnAngles.v.coord.p = fixang (turnAngles.v.coord.p / fSpeed);
		turnAngles.v.coord.h = fixang (turnAngles.v.coord.h / fSpeed);
		turnAngles.v.coord.b = fixang (turnAngles.v.coord.b / fSpeed);
		}
	}
mRotate = CFixMatrix::Create (turnAngles);
mNewOrient = info.position.mOrient * mRotate;
info.position.mOrient = mNewOrient;
if (mType.physInfo.flags & PF_TURNROLL)
	SetTurnRoll ();
//re-rotate object for bank caused by turn
if (mType.physInfo.turnRoll) {
	CFixMatrix m;

	turnAngles.v.coord.p = turnAngles.v.coord.h = 0;
	turnAngles.v.coord.b = mType.physInfo.turnRoll;
	mRotate = CFixMatrix::Create(turnAngles);
	m = info.position.mOrient * mRotate;
	info.position.mOrient = m;
	}
info.position.mOrient.CheckAndFix ();
RETVAL (1)
}

//	-----------------------------------------------------------------------------

void CObject::DoBumpHack (void)
{
ENTER (0, 0);
	CFixVector vCenter, vBump;

//bump CPlayerData a little towards vCenter of CSegment to unstick
CSegment* pSeg = SEGMENT (info.nSegment);
vCenter = pSeg->Center ();
//don't bump CPlayerData towards center of reactor CSegment
CFixVector::NormalizedDir (vBump, vCenter, info.position.vPos);
if (pSeg->m_function == SEGMENT_FUNC_REACTOR)
	vBump.Neg ();
info.position.vPos += vBump * (info.xSize / 5);
//if moving away from seg, might move out of seg, so update
if (pSeg->m_function == SEGMENT_FUNC_REACTOR)
	UpdateObjectSeg (this);
RETURN
}

//	-----------------------------------------------------------------------------

int32_t CObject::Bounce (CHitResult hitResult, float fOffs, fix *pxSideDists)
{
ENTER (0, 0);
#if 1
	CFloatVector3 pos;
	pos.Assign (Position ());
	float intrusion = X2F (ModelRadius (0)) - DistToFace (pos, hitResult.nSideSegment, (uint8_t) hitResult.nSide);
	if (intrusion < 0)
		RETVAL (0)
#if 0 // slow down
	if (intrusion > 1.0f)
		intrusion = 1.0f;
#endif
	Position () += hitResult.vNormal * F2X (intrusion);
	int16_t nSegment = FindSegByPos (Position (), info.nSegment, 1, 0);
	if ((nSegment < 0) || (nSegment > gameData.segData.nSegments)) {
		Position () = info.vLastPos;
		nSegment = FindSegByPos (Position (), info.nSegment, 1, 0);
		}
	if ((nSegment < 0) || (nSegment > gameData.segData.nSegments) || (nSegment == info.nSegment))
		RETVAL (0)
	RelinkToSeg (nSegment);
	RETVAL (1)

#else
	fix	xSideDist, xSideDists [6];

if (!pxSideDists) {
	SEGMENT (hitResult.nSideSegment)->GetSideDists (info.position.vPos, xSideDists, 1);
	pxSideDists = xSideDists;
	}
xSideDist = pxSideDists [hitResult.nSide];
if (xSideDist < info.xSize - info.xSize / 100) {
	float r;
	xSideDist = info.xSize - xSideDist;
	r = ((float) xSideDist / (float) info.xSize) * X2F (info.xSize);
	info.position.vPos.v.coord.x += (fix) ((float) hitResult.vNormal.v.coord.x * fOffs);
	info.position.vPos.v.coord.y += (fix) ((float) hitResult.vNormal.v.coord.y * fOffs);
	info.position.vPos.v.coord.z += (fix) ((float) hitResult.vNormal.v.coord.z * fOffs);
	int16_t nSegment = FindSegByPos (info.position.vPos, info.nSegment, 1, 0);
	if ((nSegment < 0) || (nSegment > gameData.segData.nSegments)) {
		info.position.vPos = info.vLastPos;
		nSegment = FindSegByPos (info.position.vPos, info.nSegment, 1, 0);
		}
	if ((nSegment < 0) || (nSegment > gameData.segData.nSegments) || (nSegment == info.nSegment))
		RETVAL (0)
	RelinkToSeg (nSegment);
	RETVAL (1)
	}
#endif
RETVAL (0)
}

//	-----------------------------------------------------------------------------

void CObject::Unstick (void)
{
ENTER (0, 0);
if (info.nType == OBJ_PLAYER) {
	if ((info.nId == N_LOCALPLAYER) && (gameStates.app.cheats.bPhysics == 0xBADA55))
		RETURN
	}
else {
#if UNSTICK_OBJS < 2
	if ((info.nType != OBJ_MONSTERBALL) && (info.nType != OBJ_ROBOT))
		RETURN
#else
	if (info.nType != OBJ_ROBOT)
		RETURN
#endif
	}
CHitQuery hitQuery (0, &info.position.vPos, &info.position.vPos, info.nSegment, Index (), 0, info.xSize);
CHitResult hitResult;
int32_t fviResult = FindHitpoint (hitQuery, hitResult, -1);
if (fviResult == HIT_WALL)
#if 1
#	if 0
	DoBumpHack ();
#	else
	Bounce (hitResult, 0.1f, NULL);
#	endif
#else
	Bounce (hi, X2F (info.xSize - VmVecDist (&info.position.vPos, &hi.vPoint)) /*0.25f*/, NULL);
#endif
RETURN
}

//	-----------------------------------------------------------------------------

void UpdateStats (CObject *pObj, int32_t nHitType)
{
ENTER (0, 0);
	int32_t	i;

if (!nHitType)
	RETURN
if (pObj->info.nType != OBJ_WEAPON)
	RETURN
if (pObj->cType.laserInfo.parent.nObject != OBJ_IDX (gameData.objData.pConsole))
	RETURN
switch (pObj->info.nId) {
	case FUSION_ID:
		if (nHitType == HIT_OBJECT)
			RETURN
		if (pObj->cType.laserInfo.nLastHitObj > 0)
			nHitType = HIT_OBJECT;
	case LASER_ID:
	case LASER_ID + 1:
	case LASER_ID + 2:
	case LASER_ID + 3:
	case VULCAN_ID:
	case SPREADFIRE_ID:
	case PLASMA_ID:
	case SUPERLASER_ID:
	case SUPERLASER_ID + 1:
	case GAUSS_ID:
	case HELIX_ID:
	case PHOENIX_ID:
		i = 0;
		break;
	case CONCUSSION_ID:
	case FLASHMSL_ID:
	case GUIDEDMSL_ID:
	case MERCURYMSL_ID:
		i = 1;
		break;
	default:
		RETURN
	}
if (nHitType == HIT_WALL) {
	gameData.statsData.player [0].nMisses [i]++;
	gameData.statsData.player [1].nMisses [i]++;
	}
else if (nHitType == HIT_OBJECT) {
	gameData.statsData.player [0].nHits [i]++;
	gameData.statsData.player [1].nHits [i]++;
	}
else
	RETURN
RETURN;
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------

void CPhysSimData::Setup (void)
{
xOldSimTime =
xMovedTime =
xMovedDist =
xAttemptedDist = 0;

vNewPos.SetZero ();
vHitPos.SetZero ();
vOffset.SetZero ();
vMoved.SetZero ();

bUseHitbox = 
bStopped = 
bBounced = 
bIgnoreObjFlag = 0;
nTries = 0;

if ((bInitialize = (gameData.physicsData.xTime < 0)))
	gameData.physicsData.xTime = I2X (1);
xSimTime = gameData.physicsData.xTime;
gameData.physicsData.nSegments = 0;
speedBoost = gameData.objData.speedBoost [nObject];
bSpeedBoost = speedBoost.bBoosted;

pObj = OBJECT (nObject);
bGetPhysSegs = (pObj->Type () == OBJ_PLAYER) || (pObj->Type () == OBJ_ROBOT);
velocity = pObj->Velocity ();
#if DBG
if (!velocity.IsZero ())
	BRP;
#endif

vOldPos = vStartPos = pObj->Position ();
nOldSeg = nStartSeg = pObj->Segment ();
#if DBG
bUseHitbox = (pObj->Type () == OBJ_PLAYER) && CollisionModel () && UseHitbox (pObj);
#else
bUseHitbox = 0;
#endif
bScaleSpeed = !(gameStates.app.bNostalgia || bInitialize) && (pObj->IsMissile () && (pObj->Id () != EARTHSHAKER_MEGA_ID) && (pObj->Id () != ROBOT_SHAKER_MEGA_ID)) ;

if (extraGameInfo [IsMultiGame].bFluidPhysics) {
	if (SEGMENT (nStartSeg)->HasWaterProp ())
		xTimeScale = 75;
	else if (SEGMENT (nStartSeg)->HasLavaProp ())
		xTimeScale = 66;
	else
		xTimeScale = 100;
	}
else
	xTimeScale = 100;
xTimeScale += extraGameInfo [IsMultiGame].nSpeedScale * xTimeScale / 4;
}

//	-----------------------------------------------------------------------------

void CPhysSimData::GetPhysSegs (void)
{
ENTER (0, 0);
if (bGetPhysSegs) {
	if (gameData.physicsData.nSegments && (gameData.physicsData.segments [gameData.physicsData.nSegments-1] == hitResult.segList [0]))
		gameData.physicsData.nSegments--;
	int32_t i = MAX_FVI_SEGS - gameData.physicsData.nSegments - 1;
	if (i > 0) {
		if (i > hitResult.nSegments)
			i = hitResult.nSegments;
		if (i > 0)
			memcpy (gameData.physicsData.segments + gameData.physicsData.nSegments, hitResult.segList, i * sizeof (gameData.physicsData.segments [0]));
		gameData.physicsData.nSegments += i;
		}
	}
RETURN
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------

int32_t CObject::UpdateOffset (CPhysSimData& simData)
{
ENTER (0, 0);
	float fScale = simData.bScaleSpeed ? MissileSpeedScale (this) : 1.0f;

if (fScale < 1.0f) {
	CFixVector vStartVel = StartVel ();
	CFixVector::Normalize (vStartVel);
	fix xDot = CFixVector::Dot (info.position.mOrient.m.dir.f, vStartVel);
	simData.vOffset = Velocity () /*simData.velocity*/ + StartVel ();
	simData.vOffset *= F2X (fScale * fScale);
	simData.vOffset += StartVel () * ((xDot > 0) ? -xDot : xDot);
	simData.vOffset *= FixMulDiv (simData.xSimTime, simData.xTimeScale, 100);
	}
else
	simData.vOffset = Velocity () /*simData.velocity*/ * FixMulDiv (simData.xSimTime, simData.xTimeScale, 100);
if (!(IsMultiGame || simData.bInitialize)) {
	int32_t i = (this != gameData.objData.pConsole) ? 0 : 1;
	if (gameStates.gameplay.slowmo [i].fSpeed != 1) {
		simData.vOffset *= (fix) (I2X (1) / gameStates.gameplay.slowmo [i].fSpeed);
		}
	}
RETVAL (!simData.vOffset.IsZero ());
}

//	-----------------------------------------------------------------------------

void CObject::SetupHitQuery (CHitQuery& hitQuery, int32_t nFlags, CFixVector* vNewPos)
{
hitQuery.p0 = &info.position.vPos;
hitQuery.p1 = vNewPos ? vNewPos : &info.position.vPos;
hitQuery.nSegment = info.nSegment;
hitQuery.radP0 = 
hitQuery.radP1 = info.xSize;
hitQuery.nObject = Index ();
hitQuery.bIgnoreObjFlag = gameData.physicsData.bIgnoreObjFlag;
hitQuery.flags = nFlags;
}

//	-----------------------------------------------------------------------------

int32_t CObject::HandleWallCollision (CPhysSimData& simData)
{
ENTER (0, 0);
if (gameStates.render.bHaveSkyBox && (info.nType == OBJ_WEAPON) && (simData.hitResult.nSegment >= 0)) {
	if (SEGMENT (simData.hitResult.nSegment)->m_function == SEGMENT_FUNC_SKYBOX) { // allow missiles and shots to leave the level and enter a skybox
		int16_t nConnSeg = SEGMENT (simData.hitResult.nSegment)->m_children [simData.hitResult.nSide];
		if ((nConnSeg < 0) && (info.xLifeLeft > I2X (1))) {	//leaving the mine
			UpdateLife (0);
			info.nFlags |= OF_SHOULD_BE_DEAD;
			}
		simData.hitResult.nType = HIT_NONE;
		}
	else if (SEGMENT (simData.hitResult.nSideSegment)->CheckForTranspPixel (simData.hitResult.vPoint, simData.hitResult.nSide, simData.hitResult.nFace)) {
		int16_t nNewSeg = FindSegByPos (simData.vNewPos, gameData.segData.skybox [0], 1, 1);
		if ((nNewSeg >= 0) && (SEGMENT (nNewSeg)->m_function == SEGMENT_FUNC_SKYBOX)) {
			simData.hitResult.nSegment = nNewSeg;
			simData.hitResult.nType = HIT_NONE;
			}
		}
	}
RETVAL (1)
}

//	-----------------------------------------------------------------------------

int32_t CObject::HandleObjectCollision (CPhysSimData& simData)
{
return 1;
}

//	-----------------------------------------------------------------------------

int32_t CObject::HandleBadCollision (CPhysSimData& simData) // hit point outside of level
{
ENTER (0, 0);
#if DBG
static int32_t nBadP0 = 0;
HUDMessage (0, "BAD P0 %d", nBadP0++);
if (info.position.vPos != simData.vOldPos)
	BRP;
#endif
memset (&simData.hitResult, 0, sizeof (simData.hitResult));
simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult, -1);
simData.hitQuery.nSegment = FindSegByPos (simData.vNewPos, info.nSegment, 1, 0);
if ((simData.hitQuery.nSegment < 0) || (simData.hitQuery.nSegment == info.nSegment)) {
	info.position.vPos = simData.vOldPos;
	RETVAL (0);
	}
simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult, -1);
if (simData.hitResult.nType == HIT_BAD_P0) {
	info.position.vPos = simData.vOldPos;
	RETVAL (0);
	}
RETVAL (1);
}

//	-----------------------------------------------------------------------------

void CObject::ComputeMovedTime (CPhysSimData& simData)
{
ENTER (0, 0);
simData.vMoved = info.position.vPos - simData.vOldPos;
simData.xMovedDist = simData.vMoved.Mag ();
simData.xAttemptedDist = simData.vOffset.Mag ();
simData.xSimTime = FixMulDiv (simData.xSimTime, simData.xAttemptedDist - simData.xMovedDist, simData.xAttemptedDist);
simData.xMovedTime = simData.xOldSimTime - simData.xSimTime;
if ((simData.xSimTime < 0) || (simData.xSimTime > simData.xOldSimTime)) {
	simData.xSimTime = simData.xOldSimTime;
	simData.xMovedTime = 0;
	}
RETURN
}

//	-----------------------------------------------------------------------------

void CObject::UnstickFromWall (CPhysSimData& simData, CFixVector& vOldVel)
{
ENTER (0, 0);
if (simData.xMovedTime) {
	if (vOldVel.IsZero ()) {
		simData.vOffset = simData.hitResult.vPoint - OBJPOS (this)->vPos;
		fix l = CFixVector::Normalize (simData.vOffset);
		simData.vOffset *= (l - info.xSize);
		}	
	int32_t nSideMask = 3 << (simData.hitResult.nSide * 2);
	CFixVector vTestPos = simData.vOldPos;
	CFixVector vOffset = simData.vMoved;
	simData.vNewPos = simData.vOldPos;
	for (int32_t i = 0, s = I2X (1); (i < 8) || (s < 0); i++) {
		vOffset /= I2X (2);
		if (vOffset.IsZero ())
			break;
		vTestPos += vOffset * s;
		int32_t mask = SEGMENT (simData.hitResult.nSideSegment)->Masks (vTestPos, simData.hitQuery.radP1).m_face;
		s = (mask & nSideMask) ? -I2X (1) : I2X (1);
		if (s > 0)
			simData.vNewPos = vTestPos;
		}
	if (CFixVector::Dist (info.position.vPos, simData.vNewPos) < simData.xMovedDist) {
		info.position.vPos = simData.vNewPos;
		ComputeMovedTime (simData);
		}
	}
RETURN
}

//	-----------------------------------------------------------------------------

int32_t CObject::ProcessWallCollision (CPhysSimData& simData)
{
ENTER (0, 0);

fix xWallPart = (gameData.collisionData.hitResult.nNormals ? CFixVector::Dot (simData.vMoved, simData.hitResult.vNormal) / gameData.collisionData.hitResult.nNormals : 0);

//if (xWallPart < 0) 
	{
	fix xHitSpeed;
	if ((simData.xMovedTime > 0) && ((xHitSpeed = -FixDiv (xWallPart, simData.xMovedTime)) > 0))
		CollideObjectAndWall (xHitSpeed, simData.hitResult.nSideSegment, simData.hitResult.nSide, simData.hitResult.vPoint);
	else if ((info.nType == OBJ_WEAPON) && !simData.xMovedDist) 
		RETVAL (-1)
	else
		ScrapeOnWall (simData.hitResult.nSideSegment, simData.hitResult.nSide, simData.hitResult.vPoint);
	}

if (info.nFlags & OF_SHOULD_BE_DEAD)
	RETVAL (-1)

if (info.nType == OBJ_DEBRIS)
	RETVAL (0)

int32_t bForceFieldBounce = (gameData.pigData.tex.pTexMapInfo [SEGMENT (simData.hitResult.nSideSegment)->m_sides [simData.hitResult.nSide].m_nBaseTex].flags & TMI_FORCE_FIELD) != 0;
if (!bForceFieldBounce && (mType.physInfo.flags & PF_STICK)) {		//stop moving
	AddStuckObject (this, simData.hitResult.nSideSegment, simData.hitResult.nSide);
	Velocity ().SetZero ();
	simData.bStopped = 1;
	RETVAL (0)
	}

// slide object along wall
int32_t bCheckVel = 0;
//We're constrained by a wall, so subtract wall part from velocity vector

xWallPart = CFixVector::Dot (simData.hitResult.vNormal, Velocity () /*simData.velocity*/);
if ((xWallPart < 0) && (bForceFieldBounce || (mType.physInfo.flags & PF_BOUNCES))) {		//bounce off CWall
	CFixVector vVelNorm = Velocity ();
	CFixVector::Normalize (vVelNorm);
	CFixVector vMoveNorm = *simData.hitQuery.p1 - *simData.hitQuery.p0;
	CFixVector::Normalize (vMoveNorm);
	fix xVelDot = CFixVector::Dot (vVelNorm, vMoveNorm);
	xVelDot = CFixVector::Dot (Orientation ().m.dir.f, vMoveNorm);
	xVelDot = CFixVector::Dot (Orientation ().m.dir.f, simData.hitResult.vNormal);
	xWallPart *= 2;	//Subtract out wall part twice to achieve bounce
	if (bForceFieldBounce) {
		bCheckVel = 1;				//check for max velocity
		if (info.nType == OBJ_PLAYER)
			xWallPart *= 2;		//CPlayerData bounce twice as much
		}
#if 1
	if (mType.physInfo.flags & PF_BOUNCES_TWICE) {
		if (mType.physInfo.flags & PF_BOUNCED_ONCE)
			mType.physInfo.flags &= ~(PF_BOUNCES | PF_BOUNCED_ONCE | PF_BOUNCES_TWICE);
		else
			mType.physInfo.flags |= PF_BOUNCED_ONCE;
		}
#else
	if (mType.physInfo.flags & PF_BOUNCES_TWICE) {
		mType.physInfo.flags &= ~PF_BOUNCES_TWICE;
		mType.physInfo.flags |= PF_BOUNCED_ONCE;
		}
	else if (mType.physInfo.flags & PF_BOUNCES)
		mType.physInfo.flags &= ~(PF_BOUNCES | PF_BOUNCED_ONCE);
#endif
	simData.bBounced = 1;		//this CObject simData.bBounced
	Velocity () /*simData.velocity*/ -= simData.hitResult.vNormal * xWallPart;
	}
else {
#if DAMPEN_KICKBACK
	if ((simData.xMovedDist < simData.xAttemptedDist) && (info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) {
		CFixVector vVelNorm = Velocity () /*simData.velocity*/;
		CFixVector::Normalize (vVelNorm);
		CFixVector vReflect = simData.hitResult.vNormal * xWallPart;
		CFixVector::Normalize (vReflect);
		if (CFixVector::Dot (vReflect, vVelNorm) > I2X (1) / 2)
			Velocity () /*simData.velocity*/ *= fix ((float) simData.xMovedDist / (float) simData.xAttemptedDist);
		Velocity () /*simData.velocity*/ -= vReflect;
		}
	else
#endif
	Velocity () /*simData.velocity*/ -= simData.hitResult.vNormal * xWallPart;
	}
if (bCheckVel) {
	fix vel = Velocity () /*simData.velocity*/.Mag ();
	if (vel > MAX_OBJECT_VEL)
		Velocity () /*simData.velocity*/ *= (FixDiv (MAX_OBJECT_VEL, vel));
	}
if (simData.bBounced && (info.nType == OBJ_WEAPON)) {
	info.position.mOrient = CFixMatrix::CreateFU (Velocity (), info.position.mOrient.m.dir.u);
	SetOrigin (simData.hitResult.vPoint);
	}
RETVAL (1)
}

//	-----------------------------------------------------------------------------

void CObject::UnstickFromObject (CPhysSimData& simData, CFixVector& vOldVel)
{
ENTER (0, 0);
	CObject* pHitObj = OBJECT (simData.hitResult.nObject);

if (vOldVel.IsZero ()) {
	simData.vOffset = OBJPOS (pHitObj)->vPos - OBJPOS (this)->vPos;
	CFixVector::Normalize (simData.vOffset);
	simData.vOffset *= info.xSize;
	}	
info.position.vPos = simData.vOldPos;
int32_t bMoved = (info.position.vPos != simData.vNewPos);
CFixVector vTestPos = simData.vNewPos;
CFixVector vOffset = simData.vOffset;
for (int32_t i = 0, s = -I2X (1); (i < 8) || (s < 0); i++) {
	vOffset /= I2X (2);
	if (vOffset.IsZero ())
		break;
	vTestPos += vOffset * s;
	if (!bMoved)
		info.position.vPos = vTestPos;
	s = CheckVectorObjectCollision (simData.hitResult, &info.position.vPos, &vTestPos, info.xSize, this, pHitObj, -1, false) ? -I2X (1) : I2X (1);
	if (s > 0)
		simData.vNewPos = vTestPos;
	}
info.position.vPos = simData.vNewPos;
RETURN
}

//	-----------------------------------------------------------------------------

int32_t CObject::ProcessObjectCollision (CPhysSimData& simData)
{
ENTER (0, 0);
CObject* pHitObj = OBJECT (simData.hitResult.nObject);
if (!pHitObj)
	RETVAL (-1)
CFixVector vOldVel = Velocity () /*simData.velocity*/;
if (!pHitObj->IsPowerup () && (CollisionModel () || pHitObj->IsStatic ())) {
	CollideTwoObjects (this, pHitObj, simData.hitResult.vPoint, &simData.hitResult.vNormal);
#if 1 // unstick objects
	UnstickFromObject (simData, vOldVel);
#endif
	}
else {
	CFixVector& pos0 = pHitObj->info.position.vPos;
	CFixVector& pos1 = info.position.vPos;
	fix size0 = OBJECT (simData.hitResult.nObject)->info.xSize;
	fix size1 = info.xSize;
	//	Calculate the hit point between the two objects.
	simData.hitResult.vPoint = pos1 - pos0;
	simData.hitResult.vPoint *= FixDiv (size0, size0 + size1);
	simData.hitResult.vPoint += pos0;
	CollideTwoObjects (this, pHitObj, simData.hitResult.vPoint);
	}
if (simData.bSpeedBoost && (this == gameData.objData.pConsole))
	Velocity () /*simData.velocity*/ = vOldVel;
// Let object continue its movement
if (info.nFlags & OF_SHOULD_BE_DEAD)
	RETVAL (-1)
if ((mType.physInfo.flags & PF_PERSISTENT) || (vOldVel == Velocity () /*simData.velocity*/)) {
	if (pHitObj->Type () == OBJ_POWERUP) 
		simData.nTries--;
	pHitObj->Ignore (simData.hitQuery.bIgnoreObjFlag);
	RETVAL (1)
	}
RETVAL (0)
}

//	-----------------------------------------------------------------------------

int32_t CObject::ProcessBadCollision (CPhysSimData& simData) // hit point outside of level
{
return 1;
}

//	-----------------------------------------------------------------------------

void CObject::ProcessDrag (CPhysSimData& simData)
{
ENTER (0, 0);
int32_t nTries = simData.xSimTime / FT;
fix xDrag = mType.physInfo.drag;
fix r = simData.xSimTime % FT;
fix k = FixDiv (r, FT);

if (this == gameData.objData.pConsole)
	xDrag = EGI_FLAG (nDrag, 0, 0, 0) * xDrag / 10;

fix d = I2X (1) - xDrag;

if (mType.physInfo.flags & PF_USES_THRUST) {
	CFixVector accel = mType.physInfo.thrust * FixDiv (I2X (1), mType.physInfo.mass);
	fix a = !accel.IsZero ();
	if (simData.bSpeedBoost && !(a || gameStates.input.bControlsSkipFrame))
		Velocity () /*simData.velocity*/ = simData.speedBoost.vVel;
	else {
		if (a) {
			while (nTries--) {
				Velocity () /*simData.velocity*/ += accel;
				Velocity () /*simData.velocity*/ *= d;
				}
			}
		else {
			while (nTries--) {
				Velocity () /*simData.velocity*/ *= d;
				}
		}
		//do linear scale on remaining bit of time
		Velocity () /*simData.velocity*/ += accel * k;
		if (xDrag)
			Velocity () /*simData.velocity*/ *= (I2X (1) - FixMul (k, xDrag));
		if (simData.bSpeedBoost) {
			if (Velocity () /*simData.velocity*/.v.coord.x < simData.speedBoost.vMinVel.v.coord.x)
				Velocity () /*simData.velocity*/.v.coord.x = simData.speedBoost.vMinVel.v.coord.x;
			else if (Velocity () /*simData.velocity*/.v.coord.x > simData.speedBoost.vMaxVel.v.coord.x)
				Velocity () /*simData.velocity*/.v.coord.x = simData.speedBoost.vMaxVel.v.coord.x;
			if (Velocity () /*simData.velocity*/.v.coord.y < simData.speedBoost.vMinVel.v.coord.y)
				Velocity () /*simData.velocity*/.v.coord.y = simData.speedBoost.vMinVel.v.coord.y;
			else if (Velocity () /*simData.velocity*/.v.coord.y > simData.speedBoost.vMaxVel.v.coord.y)
				Velocity () /*simData.velocity*/.v.coord.y = simData.speedBoost.vMaxVel.v.coord.y;
			if (Velocity () /*simData.velocity*/.v.coord.z < simData.speedBoost.vMinVel.v.coord.z)
				Velocity () /*simData.velocity*/.v.coord.z = simData.speedBoost.vMinVel.v.coord.z;
			else if (Velocity () /*simData.velocity*/.v.coord.z > simData.speedBoost.vMaxVel.v.coord.z)
				Velocity () /*simData.velocity*/.v.coord.z = simData.speedBoost.vMaxVel.v.coord.z;
			}
		}
	}
else if (xDrag) {
	fix xTotalDrag = I2X (1);
	while (nTries--)
		xTotalDrag = FixMul (xTotalDrag, d);
	//do linear scale on remaining bit of time
	xTotalDrag = FixMul (xTotalDrag, I2X (1)-FixMul (k, xDrag));
	Velocity () /*simData.velocity*/ *= xTotalDrag;
	}
RETURN
}

//	-----------------------------------------------------------------------------

int32_t CObject::ProcessOffset (CPhysSimData& simData)
{
ENTER (0, 0);
// update object's position and segment number
#if DBG
if ((Index () == nDbgObj) && (info.xSize / 2 < CFixVector::Dist (info.vLastPos, simData.hitResult.vPoint)))
	BRP;
#endif
info.position.vPos = simData.hitResult.vPoint;
if (simData.hitResult.nSegment != info.nSegment)
	RelinkToSeg (simData.hitResult.nSegment);
//if start point not in segment, move object to center of segment
if (SEGMENT (info.nSegment)->Masks (info.position.vPos, info.xSize).m_center) {	//object stuck
	int32_t n = FindSegment ();
	if (n == -1) {
		if (simData.bGetPhysSegs)
			n = FindSegByPos (info.vLastPos, info.nSegment, 1, 0);
		if (n == -1) {
			info.position.vPos = info.vLastPos;
			RelinkToSeg (info.nSegment);
			}
		else {
			CFixVector vCenter = SEGMENT (info.nSegment)->Center ();
			vCenter -= info.position.vPos;
			if (vCenter.Mag() > I2X (1)) {
				CFixVector::Normalize (vCenter);
				vCenter /= 10;
				}
			info.position.vPos -= vCenter;
			}
		if (info.nType == OBJ_WEAPON) {
			Die ();
			RETVAL (0)
			}
		}
	}
//simData.xOldSimTime = simData.xSimTime;
//ComputeMovedTime (simData);
RETVAL (1)
}

//	-----------------------------------------------------------------------------

void CObject::FixPosition (CPhysSimData& simData)
{
ENTER (0, 0);
if (info.controlType == CT_AI) {
	//	pass retry attempts info to AI.
	if (simData.nTries > 0)
		gameData.aiData.localInfo [simData.nObject].nRetryCount = simData.nTries - 1;
	}
	// If the ship has thrust, but the velocity is zero or the current position equals the start position
	// stored when entering this function, it has been stopped forcefully by something, so bounce it back to
	// avoid that the ship gets driven into the obstacle (most likely a wall, as that doesn't give in ;)
	if (((simData.hitResult.nType == HIT_WALL) || (simData.hitResult.nType == HIT_BAD_P0)) && !(simData.bSpeedBoost || simData.bStopped || simData.bBounced)) {	//Set velocity from actual movement
		simData.vMoved = info.position.vPos - simData.vStartPos;
		simData.vMoved *= (FixMulDiv (FixDiv (I2X (1), gameData.physicsData.xTime), simData.xTimeScale, 100));
		if (!simData.bSpeedBoost)
			Velocity () /*simData.velocity*/ = simData.vMoved;
		if ((this == gameData.objData.pConsole) && simData.vMoved.IsZero () && !mType.physInfo.thrust.IsZero ())
			DoBumpHack ();
		}

	if (mType.physInfo.flags & PF_LEVELLING)
		DoPhysicsAlignObject (this);
	//hack to keep player from going through closed doors
	if (((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) && (info.nSegment != simData.nStartSeg) && (gameStates.app.cheats.bPhysics != 0xBADA55)) {
		int32_t nSide = SEGMENT (info.nSegment)->ConnectedSide (SEGMENT (simData.nStartSeg));
		if (nSide != -1) {
			if (!(SEGMENT (simData.nStartSeg)->IsPassable (nSide, (info.nType == OBJ_PLAYER) ? this : NULL) & WID_PASSABLE_FLAG)) {
				//bump object back
				CSide* pSide = SEGMENT (simData.nStartSeg)->m_sides + nSide;
				if (simData.nStartSeg == -1)
					Error ("simData.nStartSeg == -1 in physics");
				fix dist = simData.vStartPos.DistToPlane (pSide->m_normals [0], gameData.segData.vertices [pSide->m_nMinVertex [0]]);
				info.position.vPos = simData.vStartPos + pSide->m_normals [0] * (info.xSize - dist);
				UpdateObjectSeg (this);
				}
			}
		}

//if end point not in segment, move object to last pos, or segment center
if (((Index () != LOCALPLAYER.nObject) || !automap.Active ()) && (info.nSegment >= 0) && SEGMENT (info.nSegment)->Masks (info.position.vPos, 0).m_center) {
	if (FindSegment () == -1) {
		int32_t n;

		if (((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) &&
			 (n = FindSegByPos (info.vLastPos, info.nSegment, 1, 0)) != -1) {
			info.position.vPos = info.vLastPos;
			RelinkToSeg (n);
			}
		else {
			info.position.vPos = SEGMENT (info.nSegment)->Center ();
			info.position.vPos.v.coord.x += simData.nObject;
			}
		if (info.nType == OBJ_WEAPON)
			Die ();
		}
	}
RETURN
}

//	-----------------------------------------------------------------------------

int32_t CObject::UpdateSimTime (CPhysSimData& simData)
{
ENTER (0, 0);
simData.xOldSimTime = simData.xSimTime;
simData.vMoved = info.position.vPos - simData.vOldPos;
CFixVector vMoveNormal = simData.vMoved;
simData.xMovedDist = CFixVector::Normalize (vMoveNormal);
if ((simData.hitResult.nType == HIT_WALL) && (CFixVector::Dot (vMoveNormal, simData.vOffset) < 0)) {		//moved backwards
	//don't change position or simData.xSimTime
	info.position.vPos = simData.vOldPos;
	if (simData.nOldSeg != simData.hitResult.nSegment)
		RelinkToSeg (simData.nOldSeg);
	if (simData.bSpeedBoost) {
		info.position.vPos = simData.vStartPos;
		SetSpeedBoostVelocity (simData.nObject, -1, -1, -1, -1, -1, &simData.vStartPos, &simData.speedBoost.vDest, 0);
		simData.vOffset = simData.speedBoost.vVel * simData.xSimTime;
		simData.bUpdateOffset = 0;
		RETVAL (0)
		}
#if 0 //DBG // unstick object from wall
	UnstickFromWall (simData, Velocity () /*simData.velocity*/);
#endif
	simData.xMovedTime = 0;
	}
else {
	simData.xAttemptedDist = simData.vOffset.Mag ();
	simData.xSimTime = FixMulDiv (simData.xSimTime, simData.xAttemptedDist - simData.xMovedDist, simData.xAttemptedDist);
	simData.xMovedTime = simData.xOldSimTime - simData.xSimTime;
	if ((simData.xSimTime < 0) || (simData.xSimTime > simData.xOldSimTime)) {
		simData.xSimTime = simData.xOldSimTime;
		simData.xMovedTime = 0;
		}
	}
RETVAL (1)
}

//	-----------------------------------------------------------------------------

//Simulate a physics CObject for this frame

void CObject::FinishPhysicsSim (CPhysSimData& simData)
{
//Velocity () /*simData.velocity*/ /= I2X (extraGameInfo [IsMultiGame].nSpeedScale + 2) / 2;
//Velocity () = simData.velocity;
}

//	-----------------------------------------------------------------------------

#if DBG

void CheckObjPos (void)
{
if (nDbgObj != -1) {
	CObject* pObj = OBJECT (nDbgObj);
	static int32_t factor = 2;
	fix d = CFixVector::Dist (pObj->info.vLastPos, pObj->Position ());
	if (pObj->info.xSize / factor  < d)
		BRP;
	}
}

#endif

//	-----------------------------------------------------------------------------

//Simulate a physics CObject for this frame

void CObject::DoPhysicsSim (void)
{
ENTER (0, 0);
if (IsPowerup () && (gameStates.app.bGameSuspended & SUSP_POWERUPS))
	RETURN

#if DBG_PHYSICS
if (!(bNewPhysCode & 1)) {
	DoPhysicsSimOld ();
	RETURN
	}
#endif
PROF_START

	CPhysSimData simData (OBJ_IDX (this)), simData2 (OBJ_IDX (this)); // must be called after initializing gameData.physicsData.xTime! Will call simData.Setup ()!

CFixMatrix mSaveOrient = info.position.mOrient;
if (DoPhysicsSimRot () && ((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) && CollisionModel ()) {
	++gameData.physicsData.bIgnoreObjFlag;
	SetupHitQuery (simData.hitQuery, FQ_CHECK_OBJS | FQ_IGNORE_POWERUPS | ((info.nType == OBJ_WEAPON) ? FQ_TRANSPOINT : 0));
	if (FindHitpoint (simData.hitQuery, simData.hitResult, -1) != HIT_NONE)
		info.position.mOrient = mSaveOrient;
	}

#if 0 //DBG
// allow tracking object movement back two frames
static CPhysSimData simData3(0);
static CFixVector vLastLastPos;
static CFixVector vLastVel, vLastLastVel;
static int32_t nLastSeg = -1, nLastLastSeg = -1;
if (Index () == nDbgObj) {
	BRP;
	memcpy (&simData3, &simData2, sizeof (simData));
	vLastLastPos = info.vLastPos;
	SetLastPos (Position ());
	vLastLastVel = vLastVel;
	vLastVel = Velocity ();
	nLastLastSeg = nLastSeg;
	nLastSeg = info.nSegment;
	}
#endif
#if DBG
if (IsWeapon ())
	BRP;
#endif
if (Velocity () .IsZero ()) {
#	if UNSTICK_OBJS
	Unstick ();
#	endif
	if (IsWeapon ()) {// actually this indicates a bug, but a workaround is needed. Alternatively, the weapon object could (and probably should) be killed
#	if 1
		CreateWeaponSpeed (this, true);
		if (Velocity ().IsZero ()) {
			Die ();
			RETURN
			}
#	else
		Die ();
		RETURN
#	endif
		}
	else {
		if (this == gameData.objData.pConsole)
			gameData.objData.speedBoost [simData.nObject].bBoosted = simData.bSpeedBoost = 0;
#if 1
		if (mType.physInfo.thrust.IsZero ())
			RETURN
#endif
		}
	}
PROF_END(ptPhysics)

#if DBG
if (Index () == nDbgObj) {
	BRP;
	if (!Velocity () /*simData.velocity*/.IsZero ())
		BRP;
	else
		RETURN
	HUDMessage (0, "%1.2f", X2F (Velocity ().Mag ()));
	}
#endif

PROF_CONT
ProcessDrag (simData);
if (Velocity ().IsZero ())
	RETURN

#if DBG
if ((nDbgSeg >= 0) && (info.nSegment == nDbgSeg))
	BRP;
#endif

#if 0// DBG
redoPhysSim:
#endif

simData.nTries = 0;
memcpy (&simData2, &simData, sizeof (simData));
++gameData.physicsData.bIgnoreObjFlag;

int32_t bRetry;

for (;;) {	//Move the object
	if (!simData.bUpdateOffset)
		simData.bUpdateOffset = 1;
	else if (!UpdateOffset (simData))
		break;

	if ((Index () == LOCALPLAYER.nObject) && /*OBSERVING*/automap.Active ()) {
		if (!IsMultiGame || !IsTeamGame || IsCoopGame) {
			info.position.vPos += simData.vOffset;
			info.position.vPos += simData.vOffset;
			}
		FinishPhysicsSim (simData);
		PROF_END(ptPhysics)
		RETURN
		}	

	do {
		bRetry = -1;

		//	If retry count is getting large, then we are trying to do something stupid.
		if (++simData.nTries > 3) {
			if (info.nType != OBJ_PLAYER)
				break;
			if (simData.nTries > 8) {
				if (simData.bSpeedBoost)
					simData.bSpeedBoost = 0;
				break;
				}
			}
		bRetry = 0;

		simData.vOldPos = info.position.vPos;			
		simData.nOldSeg = info.nSegment;
		simData.vNewPos = info.position.vPos + simData.vOffset;

#if DBG
		if (Index () == nDbgObj)
			BRP;
#endif
		SetupHitQuery (simData.hitQuery, FQ_CHECK_OBJS | ((info.nType == OBJ_WEAPON) ? FQ_TRANSPOINT : 0) | (simData.bGetPhysSegs ? FQ_GET_SEGLIST : 0), &simData.vNewPos);
		simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult, -1);
		if ((Type () == 5) && (Id () == 6) && (simData.hitResult.nType == 1) && (nDbgSeg >= 0) && ((simData.hitResult.nSegment == nDbgSeg)|| (simData.hitResult.nSideSegment == nDbgSeg))) {
			SetupHitQuery (simData.hitQuery, FQ_CHECK_OBJS | ((info.nType == OBJ_WEAPON) ? FQ_TRANSPOINT : 0) | (simData.bGetPhysSegs ? FQ_GET_SEGLIST : 0), &simData.vNewPos);
			simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult, -1);
			}
		UpdateStats (this, simData.hitResult.nType);

		if (simData.hitResult.nType == HIT_BAD_P0) {
			if (!HandleBadCollision (simData))
				break;
			}
		else if (simData.hitResult.nType == HIT_WALL) {
			if (!HandleWallCollision (simData))
				break;
			}
		else if (simData.hitResult.nType == HIT_OBJECT) {
			if (OBJECT (simData.hitResult.nObject)->IsPlayerMine ())
				simData.nTries--;
			//else if (OBJECT (simData.hitResult.nObject)->IsPowerup ())
			//	simData.hitResult.vPoint = simData.vNewPos;
			}
		else
			simData.hitResult.vPoint = simData.vNewPos;

		simData.GetPhysSegs ();
		if (simData.hitResult.nSegment == -1) {		
			if (info.nType == OBJ_WEAPON)
				Die ();
			FinishPhysicsSim (simData);
			PROF_END(ptPhysics)
			RETURN
			}
		if (!ProcessOffset (simData)) {
			FinishPhysicsSim (simData);
			PROF_END(ptPhysics)
			RETURN
			}
		} while (!UpdateSimTime (simData));

	if (bRetry < 0)
		break;
	if (simData.hitResult.nType == HIT_WALL) {
		bRetry = ProcessWallCollision (simData);
		}
	else if (simData.hitResult.nType == HIT_OBJECT) 
		bRetry = ProcessObjectCollision (simData);
	if (bRetry < 0) {
		FinishPhysicsSim (simData);
		PROF_END(ptPhysics)
		RETURN
		}
	if (!bRetry) 
		break;
	}
if ((Type () == 5) && (Id () == 6) && (Segment () == 694) && (0 > CFixVector::Dot (Velocity (), SEGMENT (nDbgSeg)->Side (nDbgSide)->Normal (2))))
	PrintLog (0);
if ((Type () == 5) && (Id () == 6) && (Segment () == nDbgSeg) && (0 > CFixVector::Dot (Velocity (), SEGMENT (nDbgSeg)->Side (nDbgSide)->Normal (2))))
	PrintLog (0);

FixPosition (simData);
FinishPhysicsSim (simData);
if (CriticalHit ())
	RandomBump (I2X (1), I2X (8), true);

#if 0 //DBG
if (Index () == nDbgObj) {
	static int factor = 2;
	static int bSound = 1;
	fix d = CFixVector::Dist (info.vLastPos, Position ());
	if (info.xSize / factor  < d) {
		if (bSound)
			audio.PlaySound (SOUND_HUD_MESSAGE);
		else {
			Position () = vLastLastPos;
			Velocity () = vLastLastVel;
			RelinkToSeg (nLastLastSeg);
			memcpy (&simData, &simData3, sizeof (simData));
			goto redoPhysSim;
			}
		}
	}
#endif
PROF_END(ptPhysics)
RETURN
}

//	----------------------------------------------------------------
//Applies an instantaneous force on an CObject, resulting in an instantaneous
//change in velocity.

void CObject::ApplyForce (CFixVector vForce)
{
ENTER (0, 0);
	//	Put in by MK on 2/13/96 for force getting applied to Omega blobs, which have 0 mass,
	//	in collision with crazy reactor robot thing on d2levf-s.
if (mType.physInfo.mass == 0)
	RETURN
if (info.movementType != MT_PHYSICS)
	RETURN
if ((automap.Active () && (this == gameData.objData.pConsole)) || SPECTATOR (this))
	RETURN
#ifdef FORCE_FEEDBACK
  if (TactileStick && (obj == OBJECT (LOCALPLAYER.nObject)))
	Tactile_apply_force (vForce, &info.position.mOrient);
#endif
//Add in acceleration due to force
if (!gameData.objData.speedBoost [OBJ_IDX (this)].bBoosted || (this != gameData.objData.pConsole)) {
#if DBG
	fix xScale = FixDiv (I2X (1), mType.physInfo.mass);
	vForce *= xScale;
	fix m1 = vForce.Mag ();
	fix m2 = Velocity ().Mag ();
	Velocity () += vForce;
#else
	vForce *= 1.0f / X2F (mType.physInfo.mass);
	Velocity () += vForce;
#	if DBG
if (Index () == nDbgObj) {
	fix xMag = Velocity ().Mag ();
	BRP;
	}
#	endif
#endif
	}
RETURN
}

//	----------------------------------------------------------------
//	Do *dest = *delta unless:
//				*delta is pretty small
//		and	they are of different signs.
void PhysicsSetRotVelAndSaturate (fix *dest, fix delta)
{
*dest = (((delta ^ *dest) >= 0) || (abs (delta) <= I2X (1) / 8)) ? delta : delta / 4;
}

//	------------------------------------------------------------------------------------------------------
//	Note: This is the old AITurnTowardsVector code.
//	ApplyRotForce used to call AITurnTowardsVector until I fixed it, which broke ApplyRotForce.
void CObject::TurnTowardsVector (CFixVector vGoal, fix rate)
{
ENTER (0, 0);
	CAngleVector	dest_angles, cur_angles;
	fix				delta_p, delta_h;
	CFixVector&		pvRotVel = mType.physInfo.rotVel;

// Make this CObject turn towards the vGoal.  Changes orientation, doesn't change direction of movement.
// If no one moves, will be facing vGoal in 1 second.

//	Detect null vector.
if (automap.Active () && (this == gameData.objData.pConsole))
	RETURN
if (vGoal.IsZero ())
	RETURN
//	Make morph OBJECTS turn more slowly.
if (info.controlType == CT_MORPH)
	rate *= 2;

dest_angles = vGoal.ToAnglesVec ();
cur_angles = info.position.mOrient.m.dir.f.ToAnglesVec ();
delta_p = (dest_angles.v.coord.p - cur_angles.v.coord.p);
delta_h = (dest_angles.v.coord.h - cur_angles.v.coord.h);
if (delta_p > I2X (1)/2)
	delta_p = dest_angles.v.coord.p - cur_angles.v.coord.p - I2X (1);
if (delta_p < -I2X (1)/2)
	delta_p = dest_angles.v.coord.p - cur_angles.v.coord.p + I2X (1);
if (delta_h > I2X (1)/2)
	delta_h = dest_angles.v.coord.h - cur_angles.v.coord.h - I2X (1);
if (delta_h < -I2X (1)/2)
	delta_h = dest_angles.v.coord.h - cur_angles.v.coord.h + I2X (1);
delta_p = FixDiv (delta_p, rate);
delta_h = FixDiv (delta_h, rate);
if (abs (delta_p) < I2X (1)/16) delta_p *= 4;
if (abs (delta_h) < I2X (1)/16) delta_h *= 4;
if (!IsMultiGame) {
	int32_t i = (this != gameData.objData.pConsole) ? 0 : 1;
	if (gameStates.gameplay.slowmo [i].fSpeed != 1) {
		delta_p = (fix) (delta_p / gameStates.gameplay.slowmo [i].fSpeed);
		delta_h = (fix) (delta_h / gameStates.gameplay.slowmo [i].fSpeed);
		}
	}
PhysicsSetRotVelAndSaturate (&pvRotVel.v.coord.x, delta_p);
PhysicsSetRotVelAndSaturate (&pvRotVel.v.coord.y, delta_h);
pvRotVel.v.coord.z = 0;
RETURN
}

//	-----------------------------------------------------------------------------
//	Applies an instantaneous whack on an CObject, resulting in an instantaneous
//	change in orientation.
void CObject::ApplyRotForce (CFixVector vForce)
{
ENTER (0, 0);
	fix	xRate, xMag;

if (info.movementType != MT_PHYSICS)
	RETURN
xMag = vForce.Mag() / 8;
if (xMag < I2X (1) / 256)
	xRate = I2X (4);
else if (xMag < mType.physInfo.mass >> 14)
	xRate = I2X (4);
else {
	xRate = FixDiv (mType.physInfo.mass, xMag);
	if (info.nType == OBJ_ROBOT) {
		if (xRate < I2X (1)/4)
			xRate = I2X (1)/4;
		//	Changed by mk, 10/24/95, claw guys should not slow down when attacking!
		tRobotInfo* pRobotInfo = ROBOTINFO (info.nId);
		if (!pRobotInfo || !(pRobotInfo->thief || pRobotInfo->attackType)) {
			if (cType.aiInfo.SKIP_AI_COUNT * gameData.physicsData.xTime < I2X (3)/4) {
				fix	xTime = FixDiv (I2X (1), 8 * gameData.physicsData.xTime);
				int32_t	nTime = X2I (xTime);
				if ((RandShort () * 2) < (xTime & 0xffff))
					nTime++;
				cType.aiInfo.SKIP_AI_COUNT += nTime;
				}
			}
		}
	else {
		if (xRate < I2X (1) / 2)
			xRate = I2X (1) / 2;
		}
	}
//	Turn amount inversely proportional to mass.  Third parameter is seconds to do 360 turn.
TurnTowardsVector (vForce, xRate);
RETURN
}

//	-----------------------------------------------------------------------------------------------------------
//this routine will set the thrust for an CObject to a value that will
// (hopefully) maintain the CObject's current velocity
void CObject::SetThrustFromVelocity (void)
{
fix k = FixMulDiv (Mass (), Drag (), (I2X (1) - Drag ()));
Thrust () = Velocity () * k;
}

//	-----------------------------------------------------------------------------------------------------------
//Simulate a physics CObject for this frame

#if DBG_PHYSICS

void CObject::DoPhysicsSimOld (void)
{
ENTER (0, 0);
if ((Type () == OBJ_POWERUP) && (gameStates.app.bGameSuspended & SUSP_POWERUPS))
	RETURN

	CPhysSimData simData (OBJ_IDX (this)); // must be called after initializing gameData.physicsData.xTime! Will call simData.Setup ()!

Assert (info.nType != OBJ_NONE);
Assert (info.movementType == MT_PHYSICS);
#if DBG
if (Index () == nDbgObj)
	BRP;
if (bDontMoveAIObjects)
	if (info.controlType == CT_AI)
		RETURN
	if (info.nType == OBJ_DEBRIS)
		BRP;
#endif
if (simData.bInitialize)
	gameData.physicsData.xTime = I2X (1);
CATCH_OBJ (this, Velocity ().v.coord.y == 0);
gameData.physicsData.nSegments = 0;

CFixMatrix mSaveOrient = info.position.mOrient;
++gameData.physicsData.bIgnoreObjFlag;
if (DoPhysicsSimRot () && ((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) && CollisionModel ()) {
	simData.hitQuery.bIgnoreObjFlag = gameData.physicsData.bIgnoreObjFlag;
	simData.hitQuery.p0 =
	simData.hitQuery.p1 = &info.position.vPos;
	simData.hitQuery.nSegment = info.nSegment;
	simData.hitQuery.radP0 = 
	simData.hitQuery.radP1 = info.xSize;
	simData.hitQuery.nObject = simData.nObject;
	simData.hitQuery.flags = FQ_CHECK_OBJS | FQ_IGNORE_POWERUPS;

	if (info.nType == OBJ_WEAPON)
		simData.hitQuery.flags |= FQ_TRANSPOINT;

	memset (&simData.hitResult, 0, sizeof (simData.hitResult));
#if DBG
	if (Index () == nDbgObj)
		BRP;
#endif
	if (FindHitpoint (simData.hitQuery, simData.hitResult, -1) != HIT_NONE)
		info.position.mOrient = mSaveOrient;
	}

if (Velocity ().IsZero ()) {
#	if UNSTICK_OBJS
	Unstick ();
#	endif
	if (this == gameData.objData.pConsole)
		gameData.objData.speedBoost [simData.nObject].bBoosted = simData.speedBoost.bBoosted = 0;
#if 1
	if (mType.physInfo.thrust.IsZero ())
		RETURN
#endif
	}

#if DBG
if (Index () == nDbgObj) {
	BRP;
	if (!Velocity ().IsZero ())
		BRP;
	HUDMessage (0, "%1.2f", X2F (Velocity ().Mag ()));
	}
#endif

//Assert (mType.physInfo.brakes == 0);		//brakes not used anymore?
//if uses thrust, cannot have zero xDrag
//Assert (!(mType.physInfo.flags & PF_USES_THRUST) || mType.physInfo.drag);
//do thrust & xDrag
if (bNewPhysCode & 2)
	ProcessDrag (simData);
else {
	if (mType.physInfo.drag) {
		CFixVector accel, &vel = Velocity ();
		int32_t			nTries = simData.xSimTime / FT;
		fix			xDrag = mType.physInfo.drag;
		fix			r = simData.xSimTime % FT;
		fix			k = FixDiv (r, FT);
		fix			a;

		if (this == gameData.objData.pConsole)
			xDrag = EGI_FLAG (nDrag, 0, 0, 0) * xDrag / 10;

		fix		d = I2X (1) - xDrag;

		if (mType.physInfo.flags & PF_USES_THRUST) {
			accel = mType.physInfo.thrust * FixDiv (I2X (1), mType.physInfo.mass);
			a = !accel.IsZero ();
			if (simData.bSpeedBoost && !(a || gameStates.input.bControlsSkipFrame))
				vel = simData.speedBoost.vVel;
			else {
				if (a) {
					while (nTries--) {
						vel += accel;
						vel *= d;
						}
					}
				else {
					while (nTries--) {
						vel *= d;
						}
				}
				//do linear scale on remaining bit of time
				vel += accel * k;
				if (xDrag)
					vel *= (I2X (1) - FixMul (k, xDrag));
				if (simData.bSpeedBoost) {
					if (vel.v.coord.x < simData.speedBoost.vMinVel.v.coord.x)
						vel.v.coord.x = simData.speedBoost.vMinVel.v.coord.x;
					else if (vel.v.coord.x > simData.speedBoost.vMaxVel.v.coord.x)
						vel.v.coord.x = simData.speedBoost.vMaxVel.v.coord.x;
					if (vel.v.coord.y < simData.speedBoost.vMinVel.v.coord.y)
						vel.v.coord.y = simData.speedBoost.vMinVel.v.coord.y;
					else if (vel.v.coord.y > simData.speedBoost.vMaxVel.v.coord.y)
						vel.v.coord.y = simData.speedBoost.vMaxVel.v.coord.y;
					if (vel.v.coord.z < simData.speedBoost.vMinVel.v.coord.z)
						vel.v.coord.z = simData.speedBoost.vMinVel.v.coord.z;
					else if (vel.v.coord.z > simData.speedBoost.vMaxVel.v.coord.z)
						vel.v.coord.z = simData.speedBoost.vMaxVel.v.coord.z;
					}
				}
			}
		else if (xDrag) {
			fix xTotalDrag = I2X (1);
			while (nTries--)
				xTotalDrag = FixMul (xTotalDrag, d);
			//do linear scale on remaining bit of time
			xTotalDrag = FixMul (xTotalDrag, I2X (1)-FixMul (k, xDrag));
			Velocity () *= xTotalDrag;
			}
		}
	}
//moveIt:

#if DBG
if ((nDbgSeg >= 0) && (info.nSegment == nDbgSeg))
	BRP;
#endif

if (extraGameInfo [IsMultiGame].bFluidPhysics) {
	if (SEGMENT (info.nSegment)->HasWaterProp ())
		simData.xTimeScale = 75;
	else if (SEGMENT (info.nSegment)->HasLavaProp ())
		simData.xTimeScale = 66;
	else
		simData.xTimeScale = 100;
	}
else
	simData.xTimeScale = 100;

simData.nTries = 0;

int32_t bRetry;

do {	//Move the object
	bRetry = 0;

	if (bNewPhysCode & 4) {
		if (!UpdateOffset (simData))
			break;
		}
	else {
		float fScale = !(gameStates.app.bNostalgia || simData.bInitialize) && (IsMissile () && (info.nId != EARTHSHAKER_MEGA_ID) && (info.nId != ROBOT_SHAKER_MEGA_ID)) 
							? MissileSpeedScale (this) 
							: 1;
		if (fScale < 1) {
			CFixVector vStartVel = StartVel ();
			CFixVector::Normalize (vStartVel);
			fix xDot = CFixVector::Dot (info.position.mOrient.m.dir.f, vStartVel);
			simData.vOffset = Velocity () + StartVel ();
			simData.vOffset *= F2X (fScale * fScale);
			simData.vOffset += StartVel () * ((xDot > 0) ? -xDot : xDot);
			simData.vOffset *= FixMulDiv (simData.xSimTime, simData.xTimeScale, 100);
			}
		else
			simData.vOffset = Velocity () * FixMulDiv (simData.xSimTime, simData.xTimeScale, 100);
		if (!(IsMultiGame || simData.bInitialize)) {
			int32_t i = (this != gameData.objData.pConsole) ? 0 : 1;
			if (gameStates.gameplay.slowmo [i].fSpeed != 1) {
				simData.vOffset *= (fix) (I2X (1) / gameStates.gameplay.slowmo [i].fSpeed);
				}
			}
		if (simData.vOffset.IsZero ())
			break;
		}

retryMove:

	//	If retry count is getting large, then we are trying to do something stupid.
	if (++simData.nTries > 3) {
		if (info.nType != OBJ_PLAYER)
			break;
		if (simData.nTries > 8) {
			if (simData.speedBoost.bBoosted)
				simData.speedBoost.bBoosted = 0;
			break;
			}
		}

	if (bNewPhysCode & 8) {
		simData.vOldPos = info.position.vPos;			
		simData.nOldSeg = info.nSegment;
		simData.vNewPos = info.position.vPos + simData.vOffset;

		SetupHitQuery (simData.hitQuery, FQ_CHECK_OBJS | ((info.nType == OBJ_WEAPON) ? FQ_TRANSPOINT : 0) | (simData.bGetPhysSegs ? FQ_GET_SEGLIST : 0), &simData.vNewPos);
		simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult, -1);
		UpdateStats (this, simData.hitResult.nType);
		}
	else {
		simData.vNewPos = info.position.vPos + simData.vOffset;
		simData.hitQuery.bIgnoreObjFlag = gameData.physicsData.bIgnoreObjFlag;
		simData.hitQuery.p0 = &info.position.vPos;
		simData.hitQuery.nSegment = info.nSegment;
		simData.hitQuery.p1 = &simData.vNewPos;
		simData.hitQuery.radP0 = 
		simData.hitQuery.radP1 = info.xSize;
		simData.hitQuery.nObject = simData.nObject;
		simData.hitQuery.flags = FQ_CHECK_OBJS;

		if (info.nType == OBJ_WEAPON)
			simData.hitQuery.flags |= FQ_TRANSPOINT;
		if (simData.bGetPhysSegs)
			simData.hitQuery.flags |= FQ_GET_SEGLIST;
		memset (&simData.hitResult, 0, sizeof (simData.hitResult));
	#if DBG
		if (info.nType == OBJ_POWERUP)
			BRP;
	#endif
		simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult, -1);
		UpdateStats (this, simData.hitResult.nType);
		simData.vOldPos = info.position.vPos;			//save the CObject's position
		simData.nOldSeg = info.nSegment;
		}

	if (simData.hitResult.nType == HIT_BAD_P0) {
		if (bNewPhysCode & 16) {
			if (!HandleBadCollision (simData))
				break;
			}
		else {
#if DBG
			static int32_t nBadP0 = 0;
			HUDMessage (0, "BAD P0 %d", nBadP0++);
#endif
			memset (&simData.hitResult, 0, sizeof (simData.hitResult));
			simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult, -1);
			simData.hitQuery.nSegment = FindSegByPos (simData.vNewPos, info.nSegment, 1, 0);
			if ((simData.hitQuery.nSegment < 0) || (simData.hitQuery.nSegment == info.nSegment)) {
				info.position.vPos = simData.vOldPos;
				break;
				}
			simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult, -1);
			if (simData.hitResult.nType == HIT_BAD_P0) {
				info.position.vPos = simData.vOldPos;
				break;
				}
			}
		}
	else if (simData.hitResult.nType == HIT_WALL) {
		if (bNewPhysCode & 32) {
			if (!HandleWallCollision (simData))
				break;
			}
		else {
			if (gameStates.render.bHaveSkyBox && (info.nType == OBJ_WEAPON) && (simData.hitResult.nSegment >= 0)) {
				if (SEGMENT (simData.hitResult.nSegment)->m_function == SEGMENT_FUNC_SKYBOX) {
					int16_t nConnSeg = SEGMENT (simData.hitResult.nSegment)->m_children [simData.hitResult.nSide];
					if ((nConnSeg < 0) && (info.xLifeLeft > I2X (1))) {	//leaving the mine
						UpdateLife (0);
						info.nFlags |= OF_SHOULD_BE_DEAD;
						}
					simData.hitResult.nType = HIT_NONE;
					}
				else if (SEGMENT (simData.hitResult.nSideSegment)->CheckForTranspPixel (simData.hitResult.vPoint, simData.hitResult.nSide, simData.hitResult.nFace)) {
					int16_t nNewSeg = FindSegByPos (simData.vNewPos, gameData.segData.skybox [0], 1, 1);
					if ((nNewSeg >= 0) && (SEGMENT (nNewSeg)->m_function == SEGMENT_FUNC_SKYBOX)) {
						simData.hitResult.nSegment = nNewSeg;
						simData.hitResult.nType = HIT_NONE;
						}
					}
				}
			}
		}
	else if (simData.hitResult.nType == HIT_OBJECT) {
		if (bNewPhysCode & 64) {
			if (!HandleObjectCollision (simData))
				break;
			}
		else {
			CObject	*pHitObj = OBJECT (simData.hitResult.nObject);
			if (pHitObj->IsPlayerMine ())
				simData.nTries--;
			}
		}

	if (bNewPhysCode & 128) {
		simData.GetPhysSegs ();
		if (simData.hitResult.nSegment == -1) {		//some sort of horrible error
			if (info.nType == OBJ_WEAPON)
				Die ();
			break;
			}
		if (!ProcessOffset (simData))
			RETURN
		}
	else {
		//RegisterHit (simData.hitResult.vPoint);
		if (simData.bGetPhysSegs) {
			if (gameData.physicsData.nSegments && (gameData.physicsData.segments [gameData.physicsData.nSegments-1] == simData.hitResult.segList [0]))
				gameData.physicsData.nSegments--;
			int32_t i = MAX_FVI_SEGS - gameData.physicsData.nSegments - 1;
			if (i > 0) {
				if (i > simData.hitResult.nSegments)
					i = simData.hitResult.nSegments;
				if (i > 0)
					memcpy (gameData.physicsData.segments + gameData.physicsData.nSegments, simData.hitResult.segList, i * sizeof (gameData.physicsData.segments [0]));
				gameData.physicsData.nSegments += i;
				}
			}
		if (simData.hitResult.nSegment == -1) {		//some sort of horrible error
			if (info.nType == OBJ_WEAPON)
				Die ();
			break;
			}
		// update CObject's position and CSegment number
		info.position.vPos = simData.hitResult.vPoint;
		if (simData.hitResult.nSegment != info.nSegment)
			RelinkToSeg (simData.hitResult.nSegment);
		//if start point not in segment, move object to center of segment
		if (SEGMENT (info.nSegment)->Masks (info.position.vPos, 0).m_center) {	//object stuck
			int32_t n = FindSegment ();
			if (n == -1) {
				if (simData.bGetPhysSegs)
					n = FindSegByPos (info.vLastPos, info.nSegment, 1, 0);
				if (n == -1) {
					info.position.vPos = info.vLastPos;
					RelinkToSeg (info.nSegment);
					}
				else {
					CFixVector vCenter;
					vCenter = SEGMENT (info.nSegment)->Center ();
					vCenter -= info.position.vPos;
					if (vCenter.Mag() > I2X (1)) {
						CFixVector::Normalize (vCenter);
						vCenter /= 10;
						}
					info.position.vPos -= vCenter;
					}
				if (info.nType == OBJ_WEAPON) {
					Die ();
					RETURN
					}
				}
			}
		}
		//calulate new sim time
	if (bNewPhysCode & 256) {
		if (!UpdateSimTime (simData))
			goto retryMove;
		}
	else {
		simData.xOldSimTime = simData.xSimTime;
		simData.vMoved = info.position.vPos - simData.vOldPos;
		CFixVector vMoveNormal = simData.vMoved;
		simData.xMovedDist = CFixVector::Normalize (vMoveNormal);
		if ((simData.hitResult.nType == HIT_WALL) && (CFixVector::Dot (vMoveNormal, simData.vOffset) < 0)) {		//moved backwards
			//don't change position or simData.xSimTime
			info.position.vPos = simData.vOldPos;
			if (simData.nOldSeg != simData.hitResult.nSegment)
				RelinkToSeg (simData.nOldSeg);
			if (simData.bSpeedBoost) {
				info.position.vPos = simData.vStartPos;
				SetSpeedBoostVelocity (simData.nObject, -1, -1, -1, -1, -1, &simData.vStartPos, &simData.speedBoost.vDest, 0);
				simData.vOffset = simData.speedBoost.vVel * simData.xSimTime;
				goto retryMove;
				}
			simData.xMovedTime = 0;
			}
		else {
			simData.xAttemptedDist = simData.vOffset.Mag ();
			simData.xSimTime = FixMulDiv (simData.xSimTime, simData.xAttemptedDist - simData.xMovedDist, simData.xAttemptedDist);
			simData.xMovedTime = simData.xOldSimTime - simData.xSimTime;
			if ((simData.xSimTime < 0) || (simData.xSimTime > simData.xOldSimTime)) {
				simData.xSimTime = simData.xOldSimTime;
				simData.xMovedTime = 0;
				}
			}
		}

	if (simData.hitResult.nType == HIT_WALL) {
		if (bNewPhysCode & 512)
			bRetry = ProcessWallCollision (simData);
		else {
			fix xHitSpeed, xWallPart;
			// Find hit speed

			xWallPart = gameData.collisionData.hitResult.nNormals ? CFixVector::Dot (simData.vMoved, simData.hitResult.vNormal) / gameData.collisionData.hitResult.nNormals : 0;
			if (xWallPart && (simData.xMovedTime > 0) && ((xHitSpeed = -FixDiv (xWallPart, simData.xMovedTime)) > 0)) {
				CollideObjectAndWall (xHitSpeed, simData.hitResult.nSideSegment, simData.hitResult.nSide, simData.hitResult.vPoint);
				}
			else if ((info.nType == OBJ_WEAPON) && simData.vMoved.IsZero ()) 
				Die ();
			else
				ScrapeOnWall (simData.hitResult.nSideSegment, simData.hitResult.nSide, simData.hitResult.vPoint);
#if UNSTICK_OBJS == 3
			fix	xSideDists [6];
			SEGMENT (simData.hitResult.nSideSegment)->GetSideDists (&info.position.vPos, xSideDists);
			bRetry = BounceObject (this, simData.hitResult, 0.1f, xSideDists);
#else
			bRetry = 0;
#endif
			if (!(info.nFlags & OF_SHOULD_BE_DEAD) && (info.nType != OBJ_DEBRIS)) {
				int32_t bForceFieldBounce;		//bounce off a forcefield

				///Assert (gameStates.app.cheats.bBouncingWeapons || ((mType.physInfo.flags & (PF_STICK | PF_BOUNCES)) != (PF_STICK | PF_BOUNCES)));	//can't be bounce and stick
				bForceFieldBounce = (gameData.pigData.tex.pTexMapInfo [SEGMENT (simData.hitResult.nSideSegment)->m_sides [simData.hitResult.nSide].m_nBaseTex].flags & TMI_FORCE_FIELD);
				if (!bForceFieldBounce && (mType.physInfo.flags & PF_STICK)) {		//stop moving
					AddStuckObject (this, simData.hitResult.nSideSegment, simData.hitResult.nSide);
					Velocity ().SetZero ();
					simData.bStopped = 1;
					bRetry = 0;
					}
				else {				// Slide CObject along CWall
					int32_t bCheckVel = 0;
					//We're constrained by a wall, so subtract wall part from velocity vector

					xWallPart = CFixVector::Dot (simData.hitResult.vNormal, Velocity ());
					if (bForceFieldBounce || (mType.physInfo.flags & PF_BOUNCES)) {		//bounce off CWall
						xWallPart *= 2;	//Subtract out wall part twice to achieve bounce
						if (bForceFieldBounce) {
							bCheckVel = 1;				//check for max velocity
							if (info.nType == OBJ_PLAYER)
								xWallPart *= 2;		//CPlayerData bounce twice as much
							}
						if ((mType.physInfo.flags & (PF_BOUNCES | PF_BOUNCES_TWICE)) == (PF_BOUNCES | PF_BOUNCES_TWICE)) {
							//Assert (mType.physInfo.flags & PF_BOUNCES);
							if (mType.physInfo.flags & PF_BOUNCED_ONCE)
								mType.physInfo.flags &= ~(PF_BOUNCES | PF_BOUNCED_ONCE | PF_BOUNCES_TWICE);
							else
								mType.physInfo.flags |= PF_BOUNCED_ONCE;
							}
						simData.bBounced = 1;		//this CObject simData.bBounced
						}
					Velocity () += simData.hitResult.vNormal * (-xWallPart);
					if (bCheckVel) {
						fix vel = Velocity ().Mag();
						if (vel > MAX_OBJECT_VEL)
							Velocity () *= (FixDiv (MAX_OBJECT_VEL, vel));
						}
					if (simData.bBounced && (info.nType == OBJ_WEAPON)) {
						info.position.mOrient = CFixMatrix::CreateFU (Velocity (), info.position.mOrient.m.dir.u);
						SetOrigin (simData.hitResult.vPoint);
						}
					bRetry = 1;
					}
				}
			}
		}
	else if (simData.hitResult.nType == HIT_OBJECT) {
		if (bNewPhysCode & 1024)
			bRetry = ProcessObjectCollision (simData);
		else {
			CObject* pHitObj = OBJECT (simData.hitResult.nObject);
			//	Calculate the hit point between the two objects.
			CFixVector vHitPos = Position () - pHitObj->Position ();
			vHitPos *= FixDiv (pHitObj->info.xSize, pHitObj->info.xSize + info.xSize);
#if DBG
			int32_t l = vHitPos.Mag ();
			l = CFixVector::Dist (pHitObj->Position (), simData.hitResult.vPoint);
#endif
			vHitPos += pHitObj->Position ();
			CFixVector vOldVel = Velocity ();
			//if (!(SPECTATOR (this) || SPECTATOR (OBJECT (simData.hitResult.nObject))))
				CollideTwoObjects (this, OBJECT (simData.hitResult.nObject), vHitPos);
			if (simData.speedBoost.bBoosted && (this == gameData.objData.pConsole))
				Velocity () = vOldVel;
			// Let object continue its movement
			if (!(info.nFlags & OF_SHOULD_BE_DEAD)) {
				if ((mType.physInfo.flags & PF_PERSISTENT) || (vOldVel == Velocity ())) {
					if (OBJECT (simData.hitResult.nObject)->info.nType == OBJ_POWERUP)
						simData.nTries--;
					OBJECT (simData.hitResult.nObject)->Ignore (simData.hitQuery.bIgnoreObjFlag);
					bRetry = 1;
					}
				}
			}
		}
	else if (simData.hitResult.nType == HIT_NONE) {
#ifdef FORCE_FEEDBACK
		if (TactileStick && (this == gameData.objData.pConsole) && !(FrameCount & 15))
			Tactile_Xvibrate_clear ();
#endif
		}
#if DBG
	else if (simData.hitResult.nType == HIT_BAD_P0) {
		Int3 ();		// Unexpected collision nType: start point not in specified CSegment.
		}
	else {
		Int3 ();		// Unknown collision nType returned from FindHitpoint!!
		break;
		}
#endif
	} while (bRetry);

if (bNewPhysCode & 2048)
	FixPosition (simData);
else {
//	Pass retry attempts info to AI.
	if (info.controlType == CT_AI) {
		Assert (simData.nObject >= 0);
		if (simData.nTries > 0)
			gameData.aiData.localInfo [simData.nObject].nRetryCount = simData.nTries - 1;
		}
		// If the ship has thrust, but the velocity is zero or the current position equals the start position
		// stored when entering this function, it has been stopped forcefully by something, so bounce it back to
		// avoid that the ship gets driven into the obstacle (most likely a wall, as that doesn't give in ;)
		if (((simData.hitResult.nType == HIT_WALL) || (simData.hitResult.nType == HIT_BAD_P0)) && !(simData.speedBoost.bBoosted || simData.bStopped || simData.bBounced)) {	//Set velocity from actual movement
			fix s = FixMulDiv (FixDiv (I2X (1), gameData.physicsData.xTime), simData.xTimeScale, 100);

			CFixVector vMoved = info.position.vPos - simData.vStartPos;
			s = vMoved.Mag();
			vMoved *= (FixMulDiv (FixDiv (I2X (1), gameData.physicsData.xTime), simData.xTimeScale, 100));
			if (!simData.bSpeedBoost)
				Velocity () = vMoved;
			if ((this == gameData.objData.pConsole) && vMoved.IsZero () && !mType.physInfo.thrust.IsZero ())
				DoBumpHack ();
			}

		if (mType.physInfo.flags & PF_LEVELLING)
			DoPhysicsAlignObject (this);
		//hack to keep CPlayerData from going through closed doors
		if (((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) && (info.nSegment != simData.nStartSeg) &&
			 (gameStates.app.cheats.bPhysics != 0xBADA55)) {
			int32_t nSide = SEGMENT (info.nSegment)->ConnectedSide (SEGMENT (simData.nStartSeg));
			if (nSide != -1) {
				if (!(SEGMENT (simData.nStartSeg)->IsPassable (nSide, (info.nType == OBJ_PLAYER) ? this : NULL) & WID_PASSABLE_FLAG)) {
					//bump object back
					CSide* pSide = SEGMENT (simData.nStartSeg)->m_sides + nSide;
					if (simData.nStartSeg == -1)
						Error ("simData.nStartSeg == -1 in physics");
					fix dist = simData.vStartPos.DistToPlane (pSide->m_normals [0], gameData.segData.vertices [pSide->m_nMinVertex [0]]);
					info.position.vPos = simData.vStartPos + pSide->m_normals [0] * (info.xSize - dist);
					UpdateObjectSeg (this);
					}
				}
			}

	//if end point not in segment, move object to last pos, or segment center
	if ((info.nSegment >= 0) && SEGMENT (info.nSegment)->Masks (info.position.vPos, 0).m_center) {
		if (FindSegment () == -1) {
			int32_t n;

			if (((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) &&
				 (n = FindSegByPos (info.vLastPos, info.nSegment, 1, 0)) != -1) {
				info.position.vPos = info.vLastPos;
				RelinkToSeg (n);
				}
			else {
				info.position.vPos = SEGMENT (info.nSegment)->Center ();
				info.position.vPos.v.coord.x += simData.nObject;
				}
			if (info.nType == OBJ_WEAPON)
				Die ();
			}
		}
	}

if (CriticalHit ())
	RandomBump (I2X (1), I2X (8), true);
CATCH_OBJ (this, Velocity ().v.coord.y == 0);
#if DBG
if (Index () == nDbgObj) {
	BRP;
	HUDMessage (0, "%1.2f", X2F (Velocity ().Mag ()));
	}
#endif

#if UNSTICK_OBJS
Unstick ();
#endif
RETURN
}

#endif //DBG

//	-----------------------------------------------------------------------------
