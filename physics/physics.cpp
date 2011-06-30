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
#ifdef TACTILE
#	include "tactile.h"
#endif

//Global variables for physics system

#define UNSTICK_OBJS		1

#define ROLL_RATE 		0x2000
#define DAMP_ANG 			0x400                  //min angle to bank

#define TURNROLL_SCALE	 (0x4ec4/2)

#define MAX_OBJECT_VEL I2X (100)

#define BUMP_HACK	1	//if defined, bump CPlayerData when he gets stuck

int bFloorLeveling = 0;

fix CheckVectorObjectCollision (CHitData& hitData, CFixVector *p0, CFixVector *p1, fix rad, CObject *thisObjP, CObject *otherObjP, bool bCheckVisibility);

//	-----------------------------------------------------------------------------

#if DBG

#define CATCH_OBJ(_objP,_cond) {if (((_objP) == dbgObjP) && (_cond)) CatchDbgObj (_cond);}

int CatchDbgObj (int cond)
{
if (cond)
	return 1;
return 0;
}

#else

#define CATCH_OBJ(_objP,_cond)

#endif

//	-----------------------------------------------------------------------------

void DoPhysicsAlignObject (CObject * objP)
{
	CFixVector	desiredUpVec;
	fixang		delta_ang, roll_ang;
	//CFixVector forvec = {0, 0, I2X (1)};
	CFixMatrix	temp_matrix;
	fix			dot, maxDot = -I2X (1);
	int			i, nBestSide;

nBestSide = 0;
// bank CPlayerData according to CSegment orientation
//find CSide of CSegment that CPlayerData is most aligned with
for (i = 0; i < 6; i++) {
	dot = CFixVector::Dot (SEGMENTS [objP->info.nSegment].m_sides [i].m_normals [0], objP->info.position.mOrient.m.dir.u);
	if (maxDot < dot) {
		maxDot = dot; 
		nBestSide = i;
		}
	}
if (gameOpts->gameplay.nAutoLeveling == 1)	// old way: used floor's Normal as upvec
	desiredUpVec = SEGMENTS [objP->info.nSegment].m_sides [3].m_normals [0];
else if (gameOpts->gameplay.nAutoLeveling == 2) {	 // new CPlayerData leveling code: use Normal of CSide closest to our up vec
	CSide* sideP = SEGMENTS [objP->info.nSegment].m_sides + nBestSide;
	if (sideP->FaceCount () == 2) {
		desiredUpVec = sideP->m_normals [2];
		CFixVector::Normalize (desiredUpVec);
		}
	else
		desiredUpVec = SEGMENTS [objP->info.nSegment].m_sides [nBestSide].m_normals [0];
	}
else if (gameOpts->gameplay.nAutoLeveling == 3)	// mine's up vector
	desiredUpVec = (*PlayerSpawnOrient (gameData.multiplayer.nLocalPlayer)).m.dir.u;
else
	return;
if (labs (CFixVector::Dot (desiredUpVec, objP->info.position.mOrient.m.dir.f)) < I2X (1)/2) {
	fixang save_delta_ang;
	CAngleVector turnAngles;

	temp_matrix = CFixMatrix::CreateFU(objP->info.position.mOrient.m.dir.f, desiredUpVec);
//	temp_matrix = CFixMatrix::CreateFU(objP->info.position.mOrient.m.v.f, &desiredUpVec, NULL);
	save_delta_ang =
	delta_ang = CFixVector::DeltaAngle(objP->info.position.mOrient.m.dir.u, temp_matrix.m.dir.u, &objP->info.position.mOrient.m.dir.f);
	delta_ang += objP->mType.physInfo.turnRoll;
	if (abs (delta_ang) > DAMP_ANG) {
		CFixMatrix mRotate, new_pm;

		roll_ang = (fixang) FixMul (gameData.physics.xTime, ROLL_RATE);
		if (abs (delta_ang) < roll_ang)
			roll_ang = delta_ang;
		else if (delta_ang < 0)
			roll_ang = -roll_ang;
		turnAngles.v.coord.p = turnAngles.v.coord.h = 0;
		turnAngles.v.coord.b = roll_ang;
		mRotate = CFixMatrix::Create(turnAngles);
		new_pm = objP->info.position.mOrient * mRotate;
		objP->info.position.mOrient = new_pm;
		}
#if 0
	else
		gameOpts->gameplay.nAutoLeveling = 0;
#endif
	}
}

//	-----------------------------------------------------------------------------

void CObject::SetTurnRoll (void)
{
//if (!gameStates.app.bD1Mission)
{
	fixang desired_bank = (fixang) -FixMul (mType.physInfo.rotVel.v.coord.y, TURNROLL_SCALE);
	if (mType.physInfo.turnRoll != desired_bank) {
		fixang delta_ang, max_roll;
		max_roll = (fixang) FixMul (ROLL_RATE, gameData.physics.xTime);
		delta_ang = desired_bank - mType.physInfo.turnRoll;
		if (labs (delta_ang) < max_roll)
			max_roll = delta_ang;
		else
			if (delta_ang < 0)
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
int	nTotalRetries=0, nTotalSims=0;
int	bDontMoveAIObjects=0;
#endif

#define FT (I2X (1)/64)

//	-----------------------------------------------------------------------------
// add rotational velocity & acceleration

int CObject::DoPhysicsSimRot (void)
{
	CAngleVector	turnAngles;
	CFixMatrix		mRotate, mNewOrient;

if (gameData.physics.xTime <= 0)
	return 0;
if (mType.physInfo.rotVel.IsZero () && mType.physInfo.rotThrust.IsZero ())
	return 0;
if (mType.physInfo.drag) {
	CFixVector	accel;
	int			nTries = gameData.physics.xTime / FT;
	fix			r = gameData.physics.xTime % FT;
	fix			k = FixDiv (r, FT);
	fix			xDrag = (mType.physInfo.drag * 5) / 2;
	fix			xScale = I2X (1) - xDrag;

	if (this == gameData.objs.consoleP)
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
turnAngles.v.coord.p = fixang (FixMul (mType.physInfo.rotVel.v.coord.x, gameData.physics.xTime));
turnAngles.v.coord.h = fixang (FixMul (mType.physInfo.rotVel.v.coord.y, gameData.physics.xTime));
turnAngles.v.coord.b = fixang (FixMul (mType.physInfo.rotVel.v.coord.z, gameData.physics.xTime));
if (!IsMultiGame) {
	int i = (this != gameData.objs.consoleP) ? 0 : 1;
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
return 1;
}

//	-----------------------------------------------------------------------------

void DoBumpHack (CObject *objP)
{
	CFixVector vCenter, vBump;

//bump CPlayerData a little towards vCenter of CSegment to unstick
vCenter = SEGMENTS [objP->info.nSegment].Center ();
//don't bump CPlayerData towards center of reactor CSegment
CFixVector::NormalizedDir (vBump, vCenter, objP->info.position.vPos);
if (SEGMENTS [objP->info.nSegment].m_function == SEGMENT_FUNC_CONTROLCEN)
	vBump.Neg ();
objP->info.position.vPos += vBump * (objP->info.xSize / 5);
//if moving away from seg, might move out of seg, so update
if (SEGMENTS [objP->info.nSegment].m_function == SEGMENT_FUNC_CONTROLCEN)
	UpdateObjectSeg (objP);
}

//	-----------------------------------------------------------------------------

#if 1 //UNSTICK_OBJS

int BounceObject (CObject *objP, CHitResult hitResult, float fOffs, fix *pxSideDists)
{
	fix	xSideDist, xSideDists [6];
	short	nSegment;

if (!pxSideDists) {
	SEGMENTS [hitResult.nSideSegment].GetSideDists (objP->info.position.vPos, xSideDists, 1);
	pxSideDists = xSideDists;
	}
xSideDist = pxSideDists [hitResult.nSide];
if (/*(0 <= xSideDist) && */
	 (xSideDist < objP->info.xSize - objP->info.xSize / 100)) {
#if 0
	objP->info.position.vPos = objP->info.vLastPos;
#else
#	if 1
	float r;
	xSideDist = objP->info.xSize - xSideDist;
	r = ((float) xSideDist / (float) objP->info.xSize) * X2F (objP->info.xSize);
#	endif
	objP->info.position.vPos.v.coord.x += (fix) ((float) hitResult.vNormal.v.coord.x * fOffs);
	objP->info.position.vPos.v.coord.y += (fix) ((float) hitResult.vNormal.v.coord.y * fOffs);
	objP->info.position.vPos.v.coord.z += (fix) ((float) hitResult.vNormal.v.coord.z * fOffs);
#endif
	nSegment = FindSegByPos (objP->info.position.vPos, objP->info.nSegment, 1, 0);
	if ((nSegment < 0) || (nSegment > gameData.segs.nSegments)) {
		objP->info.position.vPos = objP->info.vLastPos;
		nSegment = FindSegByPos (objP->info.position.vPos, objP->info.nSegment, 1, 0);
		}
	if ((nSegment < 0) || (nSegment > gameData.segs.nSegments) || (nSegment == objP->info.nSegment))
		return 0;
	objP->RelinkToSeg (nSegment);
#if DBG
	if (objP->info.nType == OBJ_PLAYER)
		HUDMessage (0, "PENETRATING WALL (%d, %1.4f)", objP->info.xSize - pxSideDists [hitResult.nSide], r);
#endif
	return 1;
	}
return 0;
}

//	-----------------------------------------------------------------------------

void UnstickObject (CObject *objP)
{
if ((objP->info.nType == OBJ_PLAYER) &&
	 (objP->info.nId == gameData.multiplayer.nLocalPlayer) &&
	 (gameStates.app.cheats.bPhysics == 0xBADA55))
	return;
if (objP->info.nType == OBJ_WEAPON) 
	return;
#if UNSTICK_OBJS < 2
if (objP->info.nType != OBJ_MONSTERBALL)
	return;
#endif
CHitQuery hitQuery (0, &objP->info.position.vPos, &objP->info.position.vPos, objP->info.nSegment, objP->Index (), 0, objP->info.xSize);
CHitResult hitResult;
int fviResult = FindHitpoint (hitQuery, hitResult);
if (fviResult == HIT_WALL)
#if 1
#	if 0
	DoBumpHack (objP);
#	else
	BounceObject (objP, hitResult, 0.1f, NULL);
#	endif
#else
	BounceObject (objP, hi, X2F (objP->info.xSize - VmVecDist (&objP->info.position.vPos, &hi.hit.vPoint)) /*0.25f*/, NULL);
#endif
}

#endif

//	-----------------------------------------------------------------------------

void UpdateStats (CObject *objP, int nHitType)
{
	int	i;

if (!nHitType)
	return;
if (objP->info.nType != OBJ_WEAPON)
	return;
if (objP->cType.laserInfo.parent.nObject != OBJ_IDX (gameData.objs.consoleP))
	return;
switch (objP->info.nId) {
	case FUSION_ID:
		if (nHitType == HIT_OBJECT)
			return;
		if (objP->cType.laserInfo.nLastHitObj > 0)
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
		return;
	}
if (nHitType == HIT_WALL) {
	gameData.stats.player [0].nMisses [i]++;
	gameData.stats.player [1].nMisses [i]++;
	}
else if (nHitType == HIT_OBJECT) {
	gameData.stats.player [0].nHits [i]++;
	gameData.stats.player [1].nHits [i]++;
	}
else
	return;
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------

void CPhysSimData::Setup (void)
{
if ((bInitialize = (gameData.physics.xTime < 0)))
	gameData.physics.xTime = I2X (1);
xSimTime = gameData.physics.xTime;
gameData.physics.nSegments = 0;
speedBoost = gameData.objs.speedBoost [nObject];
bSpeedBoost = speedBoost.bBoosted;
bScaleSpeed = !(gameStates.app.bNostalgia || bInitialize) && (IS_MISSILE (objP) && (objP->Id () != EARTHSHAKER_MEGA_ID) && (objP->Id () != ROBOT_SHAKER_MEGA_ID)) ;

objP = &OBJECTS [nObject];
bGetPhysSegs = (objP->Type () == OBJ_PLAYER) || (objP->Type () == OBJ_ROBOT);
vStartPos = objP->Position ();
nStartSeg = objP->Segment ();
bUseHitbox = (objP->Type () == OBJ_PLAYER) && CollisionModel () && UseHitbox (objP);

if (extraGameInfo [IsMultiGame].bFluidPhysics) {
	if (SEGMENTS [nStartSeg].HasWaterProp ())
		xTimeScale = 75;
	else if (SEGMENTS [nStartSeg].HasLavaProp ())
		xTimeScale = 66;
	else
		xTimeScale = 100;
	}
else
	xTimeScale = 100;
}

//	-----------------------------------------------------------------------------

void CPhysSimData::GetPhysSegs (void)
{
if (bGetPhysSegs) {
	if (gameData.physics.nSegments && (gameData.physics.segments [gameData.physics.nSegments-1] == hitResult.segList [0]))
		gameData.physics.nSegments--;
	int i = MAX_FVI_SEGS - gameData.physics.nSegments - 1;
	if (i > 0) {
		if (i > hitResult.nSegments)
			i = hitResult.nSegments;
		if (i > 0)
			memcpy (gameData.physics.segments + gameData.physics.nSegments, hitResult.segList, i * sizeof (gameData.physics.segments [0]));
		gameData.physics.nSegments += i;
		}
	}
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------

int CObject::UpdateOffset (CPhysSimData& simData)
{
	float fScale = simData.bScaleSpeed ? MissileSpeedScale (this) : 1.0f;

if (fScale < 1.0f) {
	CFixVector vStartVel = StartVel ();
	CFixVector::Normalize (vStartVel);
	fix xDot = CFixVector::Dot (info.position.mOrient.m.dir.f, vStartVel);
	simData.vOffset = mType.physInfo.velocity + StartVel ();
	simData.vOffset *= F2X (fScale * fScale);
	simData.vOffset += StartVel () * ((xDot > 0) ? -xDot : xDot);
	simData.vOffset *= FixMulDiv (simData.xSimTime, simData.xTimeScale, 100);
	}
else
	simData.vOffset = mType.physInfo.velocity * FixMulDiv (simData.xSimTime, simData.xTimeScale, 100);
if (!(IsMultiGame || simData.bInitialize)) {
	int i = (this != gameData.objs.consoleP) ? 0 : 1;
	if (gameStates.gameplay.slowmo [i].fSpeed != 1) {
		simData.vOffset *= (fix) (I2X (1) / gameStates.gameplay.slowmo [i].fSpeed);
		}
	}
return !(simData.nTries && !simData.hitResult.nType && simData.vOffset.IsZero ());
}

//	-----------------------------------------------------------------------------

void CObject::SetupHitQuery (CHitQuery& hitQuery, int nFlags)
{
gameData.physics.ignoreObjs [0] = -1;
hitQuery.p0 = 
hitQuery.p1 = &info.position.vPos;
hitQuery.nSegment = info.nSegment;
hitQuery.radP0 = 
hitQuery.radP1 = info.xSize;
hitQuery.nObject = Index ();
hitQuery.ignoreObjList = gameData.physics.ignoreObjs.Buffer ();
hitQuery.flags = nFlags;
}

//	-----------------------------------------------------------------------------

int CObject::HandleWallCollision (CPhysSimData& simData)
{
if (gameStates.render.bHaveSkyBox && (info.nType == OBJ_WEAPON) && (simData.hitResult.nSegment >= 0)) {
	if (SEGMENTS [simData.hitResult.nSegment].m_function == SEGMENT_FUNC_SKYBOX) {
		short nConnSeg = SEGMENTS [simData.hitResult.nSegment].m_children [simData.hitResult.nSide];
		if ((nConnSeg < 0) && (info.xLifeLeft > I2X (1))) {	//leaving the mine
			info.xLifeLeft = 0;
			info.nFlags |= OF_SHOULD_BE_DEAD;
			}
		simData.hitResult.nType = HIT_NONE;
		}
	else if (SEGMENTS [simData.hitResult.nSideSegment].CheckForTranspPixel (simData.hitResult.vPoint, simData.hitResult.nSide, simData.hitResult.nFace)) {
		short nNewSeg = FindSegByPos (simData.vNewPos, gameData.segs.skybox [0], 1, 1);
		if ((nNewSeg >= 0) && (SEGMENTS [nNewSeg].m_function == SEGMENT_FUNC_SKYBOX)) {
			simData.hitResult.nSegment = nNewSeg;
			simData.hitResult.nType = HIT_NONE;
			}
		}
	}
return 1;
}

//	-----------------------------------------------------------------------------

int CObject::HandleObjectCollision (CPhysSimData& simData)
{
return 1;
}

//	-----------------------------------------------------------------------------

int CObject::HandleBadCollision (CPhysSimData& simData) // hit point outside of level
{
#if DBG
static int nBadP0 = 0;
HUDMessage (0, "BAD P0 %d", nBadP0++);
if (info.position.vPos != simData.vOldPos)
	nDbgSeg = nDbgSeg;
#endif
memset (&simData.hitResult, 0, sizeof (simData.hitResult));
simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult);
simData.hitQuery.nSegment = FindSegByPos (simData.vNewPos, info.nSegment, 1, 0);
if ((simData.hitQuery.nSegment < 0) || (simData.hitQuery.nSegment == info.nSegment)) {
	info.position.vPos = simData.vOldPos;
	return 0;
	}
simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult);
if (simData.hitResult.nType == HIT_BAD_P0) {
	info.position.vPos = simData.vOldPos;
	return 0;
	}
return 1;
}

//	-----------------------------------------------------------------------------

int CObject::ProcessWallCollision (CPhysSimData& simData)
{
if (CFixVector::Dot (simData.vMoved, simData.vOffset) < 0) {		//moved backwards
#if DBG
	CFixVector vi;
	FindPointLineIntersection (vi, simData.vOldPos, simData.vNewPos, simData.hitResult.vPoint, -1);

	CFixVector v0 = simData.vOldPos;
	simData.hitQuery.p0 = &v0;
	simData.hitQuery.nSegment = info.nSegment;
	simData.hitQuery.p1 = &simData.vNewPos;
	simData.hitQuery.radP0 = 
	simData.hitQuery.radP1 = info.xSize;
	simData.hitQuery.nObject = simData.nObject;
	simData.hitQuery.ignoreObjList = gameData.physics.ignoreObjs.Buffer ();
	simData.hitQuery.flags = FQ_CHECK_OBJS;
	if (info.nType == OBJ_WEAPON)
		simData.hitQuery.flags |= FQ_TRANSPOINT;
	if ((simData.bGetPhysSegs = ((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT))))
		simData.hitQuery.flags |= FQ_GET_SEGLIST;
	CHitResult hitResult2;
	FindHitpoint (simData.hitQuery, hitResult2);
#endif
	//don't change position or simData.xSimTime
	info.position.vPos = simData.vOldPos;
	if (simData.nOldSeg != simData.hitResult.nSegment)
		RelinkToSeg (simData.nOldSeg);
	if (simData.bSpeedBoost) {
		info.position.vPos = simData.vStartPos;
		SetSpeedBoostVelocity (Index (), -1, -1, -1, -1, -1, &simData.vStartPos, &simData.speedBoost.vDest, 0);
		simData.vOffset = simData.speedBoost.vVel * simData.xSimTime;
		simData.bUpdateOffset = 0;
		return 1;
		}
	simData.xMovedTime = 0;
	}

fix xWallPart = gameData.collisions.hitResult.nNormals ? CFixVector::Dot (simData.vMoved, simData.hitResult.vNormal) / gameData.collisions.hitResult.nNormals : 0;
fix xHitSpeed;

if (xWallPart && (simData.xMovedTime > 0) && ((xHitSpeed = -FixDiv (xWallPart, simData.xMovedTime)) > 0))
	CollideObjectAndWall (xHitSpeed, simData.hitResult.nSideSegment, simData.hitResult.nSide, simData.hitResult.vPoint);
else if ((info.nType == OBJ_WEAPON) && simData.vMoved.IsZero ()) 
	return -1;
ScrapeOnWall (simData.hitResult.nSideSegment, simData.hitResult.nSide, simData.hitResult.vPoint);

#if UNSTICK_OBJS == 2
fix	xSideDists [6];
SEGMENTS [simData.hitResult.nSideSegment].GetSideDists (&info.position.vPos, xSideDists);
int bRetry = BounceObject (this, simData.hitResult, 0.1f, xSideDists);
#else
int bRetry = 0;
#endif

if (info.nType != OBJ_DEBRIS) {
	int bForceFieldBounce = (gameData.pig.tex.tMapInfoP [SEGMENTS [simData.hitResult.nSideSegment].m_sides [simData.hitResult.nSide].m_nBaseTex].flags & TMI_FORCE_FIELD);
	if (!bForceFieldBounce && (mType.physInfo.flags & PF_STICK)) {		//stop moving
		AddStuckObject (this, simData.hitResult.nSideSegment, simData.hitResult.nSide);
		mType.physInfo.velocity.SetZero ();
		simData.bStopped = 1;
		bRetry = 0;
		}
	else {				// Slide CObject along CWall
		int bCheckVel = 0;
		//We're constrained by a wall, so subtract wall part from velocity vector

		xWallPart = CFixVector::Dot (simData.hitResult.vNormal, mType.physInfo.velocity);
		if (bForceFieldBounce || (mType.physInfo.flags & PF_BOUNCE)) {		//bounce off CWall
			xWallPart *= 2;	//Subtract out wall part twice to achieve bounce
			if (bForceFieldBounce) {
				bCheckVel = 1;				//check for max velocity
				if (info.nType == OBJ_PLAYER)
					xWallPart *= 2;		//CPlayerData bounce twice as much
				}
			if ((mType.physInfo.flags & (PF_BOUNCE | PF_BOUNCES_TWICE)) == (PF_BOUNCE | PF_BOUNCES_TWICE)) {
				//Assert (mType.physInfo.flags & PF_BOUNCE);
				if (mType.physInfo.flags & PF_HAS_BOUNCED)
					mType.physInfo.flags &= ~(PF_BOUNCE | PF_HAS_BOUNCED | PF_BOUNCES_TWICE);
				else
					mType.physInfo.flags |= PF_HAS_BOUNCED;
				}
			simData.bBounced = 1;		//this CObject simData.bBounced
			}
		mType.physInfo.velocity += simData.hitResult.vNormal * (-xWallPart);
		if (bCheckVel) {
			fix vel = mType.physInfo.velocity.Mag ();
			if (vel > MAX_OBJECT_VEL)
				mType.physInfo.velocity *= (FixDiv (MAX_OBJECT_VEL, vel));
			}
		if (simData.bBounced && (info.nType == OBJ_WEAPON)) {
			info.position.mOrient = CFixMatrix::CreateFU (mType.physInfo.velocity, info.position.mOrient.m.dir.u);
			SetOrigin (simData.hitResult.vPoint);
			}
		bRetry = 1;
		}
	}
return bRetry;
}

//	-----------------------------------------------------------------------------

int CObject::ProcessObjectCollision (CPhysSimData& simData)
{
Assert (simData.nObject != -1);
CObject* hitObjP = OBJECTS + simData.nObject;
//if (!(SPECTATOR (this) || SPECTATOR (OBJECTS + simData.nObject)))
CFixVector vOldVel = mType.physInfo.velocity;
if (CollisionModel () || hitObjP->IsStatic ()) {
	CollideTwoObjects (this, hitObjP, simData.hitResult.vPoint, &simData.hitResult.vNormal);
#if 1
	if (vOldVel.IsZero ()) {
		simData.vOffset = OBJPOS (hitObjP)->vPos - OBJPOS (this)->vPos;
		CFixVector::Normalize (simData.vOffset);
		simData.vOffset *= info.xSize;
		}	
	info.position.vPos = simData.vOldPos;
	for (int i = 0, s = -I2X (1); (i < 8) || (s < 0); i++) {
		simData.vOffset /= I2X (2);
		if (simData.vOffset.IsZero ())
			break;
		if (info.position.vPos != simData.vNewPos)
			simData.vNewPos += simData.vOffset * s;
		else {
			simData.vNewPos += simData.vOffset * s;
			info.position.vPos = simData.vNewPos;
			}
		s = CheckVectorObjectCollision (simData.hitResult, &info.position.vPos, &simData.vNewPos, info.xSize, this, hitObjP, false) ? -I2X (1) : I2X (1);
		}
	info.position.vPos = simData.vNewPos;
#endif
	}
else {
	simData.hitResult.vPoint = info.position.vPos - OBJECTS [simData.nObject].info.position.vPos;
	CollideTwoObjects (this, hitObjP, simData.hitResult.vPoint);
	}
if (simData.bSpeedBoost && (this == gameData.objs.consoleP))
	mType.physInfo.velocity = vOldVel;
// Let object continue its movement
if (info.nFlags & OF_SHOULD_BE_DEAD)
	return -1;
if ((mType.physInfo.flags & PF_PERSISTENT) || (vOldVel == mType.physInfo.velocity)) {
	if (OBJECTS [simData.nObject].info.nType == OBJ_POWERUP) 
		simData.nTries--;
	gameData.physics.ignoreObjs [simData.nIgnoredObjs++] = simData.nObject;
	return 1;
	}
return 0;
}

//	-----------------------------------------------------------------------------

int CObject::ProcessBadCollision (CPhysSimData& simData) // hit point outside of level
{
return 1;
}

//	-----------------------------------------------------------------------------

void CObject::ProcessDrag (CPhysSimData& simData)
{
int nTries = simData.xSimTime / FT;
fix xDrag = mType.physInfo.drag;
fix r = simData.xSimTime % FT;
fix k = FixDiv (r, FT);

if (this == gameData.objs.consoleP)
	xDrag = EGI_FLAG (nDrag, 0, 0, 0) * xDrag / 10;

fix d = I2X (1) - xDrag;

if (mType.physInfo.flags & PF_USES_THRUST) {
	CFixVector accel = mType.physInfo.thrust * FixDiv (I2X (1), mType.physInfo.mass);
	fix a = !accel.IsZero ();
	if (simData.bSpeedBoost && !(a || gameStates.input.bControlsSkipFrame))
		mType.physInfo.velocity = simData.speedBoost.vVel;
	else {
		if (a) {
			while (nTries--) {
				mType.physInfo.velocity += accel;
				mType.physInfo.velocity *= d;
				}
			}
		else {
			while (nTries--) {
				mType.physInfo.velocity *= d;
				}
		}
		//do linear scale on remaining bit of time
		mType.physInfo.velocity += accel * k;
		if (xDrag)
			mType.physInfo.velocity *= (I2X (1) - FixMul (k, xDrag));
		if (simData.bSpeedBoost) {
			if (mType.physInfo.velocity.v.coord.x < simData.speedBoost.vMinVel.v.coord.x)
				mType.physInfo.velocity.v.coord.x = simData.speedBoost.vMinVel.v.coord.x;
			else if (mType.physInfo.velocity.v.coord.x > simData.speedBoost.vMaxVel.v.coord.x)
				mType.physInfo.velocity.v.coord.x = simData.speedBoost.vMaxVel.v.coord.x;
			if (mType.physInfo.velocity.v.coord.y < simData.speedBoost.vMinVel.v.coord.y)
				mType.physInfo.velocity.v.coord.y = simData.speedBoost.vMinVel.v.coord.y;
			else if (mType.physInfo.velocity.v.coord.y > simData.speedBoost.vMaxVel.v.coord.y)
				mType.physInfo.velocity.v.coord.y = simData.speedBoost.vMaxVel.v.coord.y;
			if (mType.physInfo.velocity.v.coord.z < simData.speedBoost.vMinVel.v.coord.z)
				mType.physInfo.velocity.v.coord.z = simData.speedBoost.vMinVel.v.coord.z;
			else if (mType.physInfo.velocity.v.coord.z > simData.speedBoost.vMaxVel.v.coord.z)
				mType.physInfo.velocity.v.coord.z = simData.speedBoost.vMaxVel.v.coord.z;
			}
		}
	}
else if (xDrag) {
	fix xTotalDrag = I2X (1);
	while (nTries--)
		xTotalDrag = FixMul (xTotalDrag, d);
	//do linear scale on remaining bit of time
	xTotalDrag = FixMul (xTotalDrag, I2X (1)-FixMul (k, xDrag));
	mType.physInfo.velocity *= xTotalDrag;
	}
}

//	-----------------------------------------------------------------------------

int CObject::ProcessOffset (CPhysSimData& simData)
{
// update CObject's position and CSegment number
info.position.vPos = simData.hitResult.vPoint;
if (simData.hitResult.nSegment != info.nSegment)
	RelinkToSeg (simData.hitResult.nSegment);
//if start point not in segment, move object to center of segment
if (SEGMENTS [info.nSegment].Masks (info.position.vPos, 0).m_center) {	//object stuck
	int n = FindSegment ();
	if (n == -1) {
		if (simData.bGetPhysSegs)
			n = FindSegByPos (info.vLastPos, info.nSegment, 1, 0);
		if (n == -1) {
			info.position.vPos = info.vLastPos;
			RelinkToSeg (info.nSegment);
			}
		else {
			CFixVector vCenter;
			vCenter = SEGMENTS [info.nSegment].Center ();
			vCenter -= info.position.vPos;
			if (vCenter.Mag() > I2X (1)) {
				CFixVector::Normalize (vCenter);
				vCenter /= 10;
				}
			info.position.vPos -= vCenter;
			}
		if (info.nType == OBJ_WEAPON) {
			Die ();
			return 0;
			}
		}
	}

simData.xOldSimTime = simData.xSimTime;
simData.vMoved = info.position.vPos - simData.vOldPos;
fix actualDist = simData.vMoved.Mag ();
fix attemptedDist = simData.vOffset.Mag ();
simData.xSimTime = FixMulDiv (simData.xSimTime, attemptedDist - actualDist, attemptedDist);
simData.xMovedTime = simData.xOldSimTime - simData.xSimTime;
if ((simData.xSimTime < 0) || (simData.xSimTime > simData.xOldSimTime)) {
	simData.xSimTime = simData.xOldSimTime;
	simData.xMovedTime = 0;
	}
return 1;
}

//	-----------------------------------------------------------------------------

//Simulate a physics CObject for this frame

void CObject::DoPhysicsSim (void)
{
if ((Type () == OBJ_POWERUP) && (gameStates.app.bGameSuspended & SUSP_POWERUPS))
	return;

	CPhysSimData simData (OBJ_IDX (this)); // must be called after initializing gameData.physics.xTime! Will call simData.Setup ()!

CFixMatrix mSaveOrient = info.position.mOrient;
if (DoPhysicsSimRot () && ((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) && CollisionModel ()) {
	SetupHitQuery (simData.hitQuery, FQ_CHECK_OBJS | FQ_IGNORE_POWERUPS | ((info.nType == OBJ_WEAPON) ? FQ_TRANSPOINT : 0));
	if (FindHitpoint (simData.hitQuery, simData.hitResult) != HIT_NONE)
		info.position.mOrient = mSaveOrient;
	}

if (mType.physInfo.velocity.IsZero ()) {
#	if UNSTICK_OBJS
	UnstickObject (this);
#	endif
	if (this == gameData.objs.consoleP)
		gameData.objs.speedBoost [simData.nObject].bBoosted = simData.bSpeedBoost = 0;
#if 0
	if (mType.physInfo.thrust.IsZero ())
		return;
#endif
	}

#if DBG
if (Index () == nDbgObj) {
	nDbgObj = nDbgObj;
	if (!mType.physInfo.velocity.IsZero ())
		nDbgObj = nDbgObj;
	HUDMessage (0, "%1.2f", X2F (mType.physInfo.velocity.Mag ()));
	}
#endif

ProcessDrag (simData);

#if DBG
if ((nDbgSeg >= 0) && (info.nSegment == nDbgSeg))
	nDbgSeg = nDbgSeg;
#endif

int bRetry;

simData.nTries = 0;
do {	//Move the object
	if (!simData.bUpdateOffset)
		simData.bUpdateOffset = 1;
	else if (!UpdateOffset (simData))
		break;

	bRetry = 0;

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

	simData.vOldPos = info.position.vPos;			
	simData.nOldSeg = info.nSegment;
	simData.vNewPos = info.position.vPos + simData.vOffset;

	SetupHitQuery (simData.hitQuery, FQ_CHECK_OBJS | ((info.nType == OBJ_WEAPON) ? FQ_TRANSPOINT : 0) | (simData.bGetPhysSegs ? FQ_GET_SEGLIST : 0));
	simData.hitResult.nType = FindHitpoint (simData.hitQuery, simData.hitResult);
	UpdateStats (this, simData.hitResult.nType);

	if (simData.hitResult.nType == HIT_BAD_P0) {
		if (!HandleBadCollision (simData))
			break;
		}
	else if (simData.hitResult.nType == HIT_OBJECT) {
		if (OBJECTS [simData.hitResult.nObject].IsPlayerMine ())
			simData.nTries--;
		}

	simData.GetPhysSegs ();
	if (simData.hitResult.nSegment == -1) {		//some sort of horrible error
		if (info.nType == OBJ_WEAPON)
			Die ();
		break;
		}

	Assert ((simData.hitResult.nType != HIT_WALL) || ((simData.hitResult.nSideSegment > -1) && (simData.hitResult.nSideSegment <= gameData.segs.nLastSegment)));

	if (!ProcessOffset (simData))
		return;

	if (simData.hitResult.nType == HIT_WALL) {
		bRetry = ProcessWallCollision (simData);
		}
	//calulate new sim time
	else if (simData.hitResult.nType == HIT_OBJECT) {
		bRetry = ProcessObjectCollision (simData);
		}
	if (bRetry < 0)
		return;
	} while (bRetry);

//	Pass retry attempts info to AI.
if (info.controlType == CT_AI) {
	Assert (simData.nObject >= 0);
	if (simData.nTries > 0)
		gameData.ai.localInfo [simData.nObject].nRetryCount = simData.nTries - 1;
	}
	// If the ship has thrust, but the velocity is zero or the current position equals the start position
	// stored when entering this function, it has been stopped forcefully by something, so bounce it back to
	// avoid that the ship gets driven into the obstacle (most likely a wall, as that doesn't give in ;)
	if (((simData.hitResult.nType == HIT_WALL) || (simData.hitResult.nType == HIT_BAD_P0)) && !(simData.bSpeedBoost || simData.bStopped || simData.bBounced)) {	//Set velocity from actual movement
		fix s = FixMulDiv (FixDiv (I2X (1), gameData.physics.xTime), simData.xTimeScale, 100);

		simData.vMoved = info.position.vPos - simData.vStartPos;
		s = simData.vMoved.Mag();
		simData.vMoved *= (FixMulDiv (FixDiv (I2X (1), gameData.physics.xTime), simData.xTimeScale, 100));
		if (!simData.bSpeedBoost)
			mType.physInfo.velocity = simData.vMoved;
		if ((this == gameData.objs.consoleP) && simData.vMoved.IsZero () && !mType.physInfo.thrust.IsZero ())
			DoBumpHack (this);
		}

	if (mType.physInfo.flags & PF_LEVELLING)
		DoPhysicsAlignObject (this);
	//hack to keep CPlayerData from going through closed doors
	if (((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) && (info.nSegment != simData.nStartSeg) &&
		 (gameStates.app.cheats.bPhysics != 0xBADA55)) {
		int nSide = SEGMENTS [info.nSegment].ConnectedSide (SEGMENTS + simData.nStartSeg);
		if (nSide != -1) {
			if (!(SEGMENTS [simData.nStartSeg].IsDoorWay (nSide, (info.nType == OBJ_PLAYER) ? this : NULL) & WID_PASSABLE_FLAG)) {
				//bump object back
				CSide* sideP = SEGMENTS [simData.nStartSeg].m_sides + nSide;
				if (simData.nStartSeg == -1)
					Error ("simData.nStartSeg == -1 in physics");
				fix dist = simData.vStartPos.DistToPlane (sideP->m_normals [0], gameData.segs.vertices [sideP->m_nMinVertex [0]]);
				info.position.vPos = simData.vStartPos + sideP->m_normals [0] * (info.xSize - dist);
				UpdateObjectSeg (this);
				}
			}
		}

//if end point not in segment, move object to last pos, or segment center
if ((info.nSegment >= 0) && SEGMENTS [info.nSegment].Masks (info.position.vPos, 0).m_center) {
	if (FindSegment () == -1) {
		int n;

		if (((info.nType == OBJ_PLAYER) || (info.nType == OBJ_ROBOT)) &&
			 (n = FindSegByPos (info.vLastPos, info.nSegment, 1, 0)) != -1) {
			info.position.vPos = info.vLastPos;
			OBJECTS [simData.nObject].RelinkToSeg (n);
			}
		else {
			info.position.vPos = SEGMENTS [info.nSegment].Center ();
			info.position.vPos.v.coord.x += simData.nObject;
			}
		if (info.nType == OBJ_WEAPON)
			Die ();
		}
	}

if (CriticalHit ())
	RandomBump (I2X (1), I2X (8), true);
CATCH_OBJ (this, mType.physInfo.velocity.v.coord.y == 0);
#if DBG
if (Index () == nDbgObj) {
	nDbgObj = nDbgObj;
	HUDMessage (0, "%1.2f", X2F (mType.physInfo.velocity.Mag ()));
	}
#endif

#if UNSTICK_OBJS
UnstickObject (this);
#endif
}

//	----------------------------------------------------------------
//Applies an instantaneous force on an CObject, resulting in an instantaneous
//change in velocity.

void CObject::ApplyForce (CFixVector vForce)
{
	//	Put in by MK on 2/13/96 for force getting applied to Omega blobs, which have 0 mass,
	//	in collision with crazy reactor robot thing on d2levf-s.
if (mType.physInfo.mass == 0)
	return;
if (info.movementType != MT_PHYSICS)
	return;
if ((automap.Display () && (this == gameData.objs.consoleP)) || SPECTATOR (this))
	return;
#ifdef TACTILE
  if (TactileStick && (obj == OBJECTS + LOCALPLAYER.nObject))
	Tactile_apply_force (vForce, &info.position.mOrient);
#endif
//Add in acceleration due to force
if (!gameData.objs.speedBoost [OBJ_IDX (this)].bBoosted || (this != gameData.objs.consoleP)) {
#if 0 //DBG
	fix xScale = FixDiv (I2X (1), mType.physInfo.mass);
	vForce *= xScale;
	mType.physInfo.velocity += vForce;
#else
	mType.physInfo.velocity += (vForce * FixDiv (I2X (1), mType.physInfo.mass));
#	if DBG
if (Index () == nDbgObj) {
	fix xMag = mType.physInfo.velocity.Mag ();
	nDbgObj = nDbgObj;
	}
#	endif
#endif
	}
}

//	----------------------------------------------------------------
//	Do *dest = *delta unless:
//				*delta is pretty small
//		and	they are of different signs.
void PhysicsSetRotVelAndSaturate (fix *dest, fix delta)
{
if ((delta ^ *dest) < 0) {
	if (abs (delta) < I2X (1)/8) {
		*dest = delta/4;
		}
	else
		*dest = delta;
		}
else {
	*dest = delta;
	}
}

//	------------------------------------------------------------------------------------------------------
//	Note: This is the old AITurnTowardsVector code.
//	ApplyRotForce used to call AITurnTowardsVector until I fixed it, which broke ApplyRotForce.
void CObject::TurnTowardsVector (CFixVector vGoal, fix rate)
{
	CAngleVector	dest_angles, cur_angles;
	fix				delta_p, delta_h;
	CFixVector&		pvRotVel = mType.physInfo.rotVel;

// Make this CObject turn towards the vGoal.  Changes orientation, doesn't change direction of movement.
// If no one moves, will be facing vGoal in 1 second.

//	Detect null vector.
if (automap.Display () && (this == gameData.objs.consoleP))
	return;
if (vGoal.IsZero ())
	return;
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
	int i = (this != gameData.objs.consoleP) ? 0 : 1;
	if (gameStates.gameplay.slowmo [i].fSpeed != 1) {
		delta_p = (fix) (delta_p / gameStates.gameplay.slowmo [i].fSpeed);
		delta_h = (fix) (delta_h / gameStates.gameplay.slowmo [i].fSpeed);
		}
	}
PhysicsSetRotVelAndSaturate (&pvRotVel.v.coord.x, delta_p);
PhysicsSetRotVelAndSaturate (&pvRotVel.v.coord.y, delta_h);
pvRotVel.v.coord.z = 0;
}

//	-----------------------------------------------------------------------------
//	Applies an instantaneous whack on an CObject, resulting in an instantaneous
//	change in orientation.
void CObject::ApplyRotForce (CFixVector vForce)
{
	fix	xRate, xMag;

if (info.movementType != MT_PHYSICS)
	return;
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
		if (!(ROBOTINFO (info.nId).thief || ROBOTINFO (info.nId).attackType)) {
			if (cType.aiInfo.SKIP_AI_COUNT * gameData.physics.xTime < I2X (3)/4) {
				fix	xTime = FixDiv (I2X (1), 8 * gameData.physics.xTime);
				int	nTime = X2I (xTime);
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
}


//this routine will set the thrust for an CObject to a value that will
// (hopefully) maintain the CObject's current velocity
void CObject::SetThrustFromVelocity (void)
{
fix k = FixMulDiv (mType.physInfo.mass, mType.physInfo.drag, (I2X (1) - mType.physInfo.drag));
mType.physInfo.thrust = mType.physInfo.velocity * k;
}

//	-----------------------------------------------------------------------------
