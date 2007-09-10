#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <string.h>	// for memset
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "inferno.h"
#include "error.h"
#include "timer.h"
#include "u_mem.h"
#include "particles.h"
#include "lightning.h"
#include "laser.h"
#include "fireball.h"
#include "network.h"
#include "newdemo.h"
#include "render.h"
#include "object.h"
#include "objrender.h"
#include "objsmoke.h"
#include "automap.h"

//------------------------------------------------------------------------------

#ifdef _DEBUG

void KillObjectSmoke (int i)
{
if ((i >= 0) && (gameData.smoke.objects [i] >= 0)) {
	SetSmokeLife (gameData.smoke.objects [i], 0);
	gameData.smoke.objects [i] = -1;
	}
}

#endif

//------------------------------------------------------------------------------

void KillPlayerSmoke (int i)
{
KillObjectSmoke (gameData.multiplayer.players [i].nObject);
}

//------------------------------------------------------------------------------

void ResetPlayerSmoke (void)
{
	int	i;

for (i = 0; i < MAX_PLAYERS; i++)
	KillPlayerSmoke (i);
}

//------------------------------------------------------------------------------

void InitObjectSmoke (void)
{
memset (gameData.smoke.objects, 0xff, sizeof (*gameData.smoke.objects) * MAX_OBJECTS);
memset (gameData.smoke.objExplTime, 0xff, sizeof (*gameData.smoke.objExplTime) * MAX_OBJECTS);
}

//------------------------------------------------------------------------------

void ResetObjectSmoke (void)
{
	int	i;

for (i = 0; i < MAX_OBJECTS; i++)
	KillObjectSmoke (i);
InitObjectSmoke ();
}

//------------------------------------------------------------------------------

static inline int RandN (int n)
{
if (!n)
	return 0;
return (int) ((double) rand () * (double) n / (double) RAND_MAX);
}

//------------------------------------------------------------------------------

void CreateDamageExplosion (int h, int i)
{
if (EGI_FLAG (bDamageExplosions, 1, 0, 0) && 
	 (gameStates.app.nSDLTicks - gameData.smoke.objExplTime [i] > 100)) {
	gameData.smoke.objExplTime [i] = gameStates.app.nSDLTicks;
	if (!RandN (11 - h))
		CreateSmallFireballOnObject (gameData.objs.objects + i, F1_0, 1);
	}
}

//------------------------------------------------------------------------------
#if 0
void CreateThrusterFlames (tObject *objP)
{
	static int nThrusters = -1;

	vmsVector	pos, dir = objP->position.mOrient.fVec;
	int			d, j;
	tCloud		*pCloud;

VmVecNegate (&dir);
if (nThrusters < 0) {
	nThrusters = 
		CreateSmoke (&objP->position.vPos, &dir, objP->nSegment, 2, -2000, 20000,
						 gameOpts->render.smoke.bSyncSizes ? -1 : gameOpts->render.smoke.nSize [1],
						 2, -2000, PLR_PART_SPEED * 50, 5, OBJ_IDX (objP), NULL, 1);
	gameData.smoke.objects [OBJ_IDX (objP)] = nThrusters;
	}
else
	SetSmokeDir (nThrusters, &dir);
d = 8 * objP->size / 40;
for (j = 0; j < 2; j++)
	if (pCloud = GetCloud (nThrusters, j)) {
		VmVecScaleAdd (&pos, &objP->position.vPos, &objP->position.mOrient.fVec, -objP->size);
		VmVecScaleInc (&pos, &objP->position.mOrient.rVec, j ? d : -d);
		VmVecScaleInc (&pos, &objP->position.mOrient.uVec,  -objP->size / 25);
		SetCloudPos (pCloud, &pos);
		}
}
#endif
//------------------------------------------------------------------------------

extern tSmoke	smoke [];

void DoPlayerSmoke (tObject *objP, int i)
{
	int			h, j, d, nParts, nType;
	float			nScale;
	tCloud		*pCloud;
	vmsVector	vPos [2], fn, mn;
	static int	bForward = 1;
	static int	t0 = -1;


if (i < 0)
	i = objP->id;
if ((gameData.multiplayer.players [i].flags & PLAYER_FLAGS_CLOAKED) ||
	 (gameStates.render.automap.bDisplay && IsMultiGame && !AM_SHOW_PLAYERS)) {
	KillObjectSmoke (i);
	return;
	}
j = OBJ_IDX (objP);
#ifdef _DEBUG
if ((h = gameData.lightnings.objects [j]) >= 0) {
	t0 = gameStates.app.nSDLTicks;
	}
else if (gameStates.app.nSDLTicks - t0 > 2000) {
	tRgbaColorf color = {0.1f, 0.1f, 1.0f, 0.3f};
	gameData.lightnings.objects [j] = CreateLightning (
		30, 
		gameStates.app.bFreeCam ? &gameStates.app.playerPos.vPos : &objP->position.vPos,
		(1 || (rand () & 1)) ? NULL :
		gameStates.app.bFreeCam ? &gameStates.app.playerPos.mOrient.uVec : &objP->position.mOrient.uVec, 
		j, -30000, 2000, F1_0 * 60, F1_0 * 5, 0, 100, 10, 1, 3, 1, 1, &color);
	}
#endif
if (gameOpts->render.smoke.bDecreaseLag && (i == gameData.multiplayer.nLocalPlayer)) {
	fn = objP->position.mOrient.fVec;
	VmVecSub (&mn, &objP->position.vPos, &objP->vLastPos);
	VmVecNormalize (&fn);
	VmVecNormalize (&mn);
	d = VmVecDot (&fn, &mn);
	if (d >= -F1_0 / 2) 
		bForward = 1;
	else {
		if (bForward) {
			if ((h = gameData.smoke.objects [j]) >= 0) {
				KillObjectSmoke (i);
				DestroySmoke (h);
				}
			bForward = 0;
			nScale = 0;
			return;
			}
		}
	}
#if 0
if (EGI_FLAG (bThrusterFlames, 1, 1, 0)) {
	if ((a <= F1_0 / 4) && (a || !gameStates.input.bControlsSkipFrame))	//no thruster flames if moving backward
		DropAfterburnerBlobs (objP, 2, i2f (1), -1, gameData.objs.console, 1); //F1_0 / 4);
	}
#endif
if ((gameData.app.nGameMode & GM_NETWORK) && !gameData.multiplayer.players [i].connected)
	nParts = 0;
else if (objP->flags & (OF_SHOULD_BE_DEAD | OF_DESTROYED))
	nParts = 0;
else if ((i == gameData.multiplayer.nLocalPlayer) && (gameStates.app.bPlayerIsDead || (gameData.multiplayer.players [i].shields < 0)))
	nParts = 0;
else {
	h = f2ir (gameData.multiplayer.players [i].shields);
	nParts = 10 - h / 5;
	nScale = f2fl (objP->size);
	if (h <= 25)
		nScale /= 1.5;
	else if (h <= 50)
		nScale /= 2;
	else
		nScale /= 3;
	if (nParts <= 0) {
		nType = 2;
		//nParts = (gameStates.entropy.nTimeLastMoved < 0) ? 250 : 125;
		}
	else {
		CreateDamageExplosion (nParts, j);
		nType = (h > 25);
		nParts *= 25;
		nParts += 75;
		}
	nParts = (gameStates.entropy.nTimeLastMoved < 0) ? SHIP_MAX_PARTS * 2 : SHIP_MAX_PARTS;
	if (SHOW_SMOKE && nParts && gameOpts->render.smoke.bPlayers) {
		if (gameOpts->render.smoke.bSyncSizes) {
			nParts = -MAX_PARTICLES (nParts, gameOpts->render.smoke.nDens [0]);
			nScale = PARTICLE_SIZE (gameOpts->render.smoke.nSize [0], nScale);
			}
		else {
			nParts = -MAX_PARTICLES (nParts, gameOpts->render.smoke.nDens [1]);
			nScale = PARTICLE_SIZE (gameOpts->render.smoke.nSize [1], nScale);
			}
		if (!(objP->mType.physInfo.thrust.p.x ||
				objP->mType.physInfo.thrust.p.y ||
				objP->mType.physInfo.thrust.p.z)) {
			nParts /= 2;
			nScale /= 2;
			}
		if (0 > (h = gameData.smoke.objects [j])) {
			//LogErr ("creating tPlayer smoke\n");
			h = gameData.smoke.objects [j] = 
				CreateSmoke (&objP->position.vPos, NULL, objP->nSegment, 2, nParts, nScale,
								 gameOpts->render.smoke.nSize [1],
								 2, PLR_PART_LIFE / (nType + 1), PLR_PART_SPEED, nType, j, NULL, 1);
			}
		else {
			SetSmokeType (h, nType);
			SetSmokePartScale (h, -nScale);
			SetSmokeDensity (h, nParts, gameOpts->render.smoke.bSyncSizes ? -1 : gameOpts->render.smoke.nSize [1]);
			SetSmokeSpeed (gameData.smoke.objects [i], 
								(objP->mType.physInfo.velocity.p.x || objP->mType.physInfo.velocity.p.y || objP->mType.physInfo.velocity.p.z) ?
								PLR_PART_SPEED : PLR_PART_SPEED * 2);
			}
		CalcShipThrusterPos (objP, vPos);
		for (j = 0; j < 2; j++)
			if ((pCloud = GetCloud (h, j)))
				SetCloudPos (pCloud, vPos + j);
		return;
		}
	}
KillObjectSmoke (i);
}

//------------------------------------------------------------------------------

void DoRobotSmoke (tObject *objP)
{
	int			h = -1, i, nShields = 0, nParts;
	float			nScale;
	vmsVector	pos;

if (!SHOW_SMOKE)
	return;
i = OBJ_IDX (objP);
if (!gameOpts->render.smoke.bRobots) {
	if (gameData.smoke.objects [i] >= 0)
		KillObjectSmoke (i);
	return;
	}
if ((objP->shields < 0) || (objP->flags & (OF_SHOULD_BE_DEAD | OF_DESTROYED)))
	nParts = 0;
else {
	nShields = f2ir (RobotDefaultShields (objP));
	h = f2ir (objP->shields) * 100 / nShields;
	}
if (h < 0)
	return;	
nParts = 10 - h / 5;
if (nParts > 0) {
	if (nShields > 4000)
		nShields = 4000;
	else if (nShields < 1000)
		nShields = 1000;
	CreateDamageExplosion (nParts, i);
	//nParts *= nShields / 10;
	nParts = BOT_MAX_PARTS;
	nScale = (float) sqrt (8.0 / f2fl (objP->size));
	nScale *= 1.0f + h / 25.0f;
	if (!gameOpts->render.smoke.bSyncSizes) {
		nParts = -MAX_PARTICLES (nParts, gameOpts->render.smoke.nDens [2]);
		nScale = PARTICLE_SIZE (gameOpts->render.smoke.nSize [2], nScale);
		}
	if (gameData.smoke.objects [i] < 0) {
		//LogErr ("creating robot %d smoke\n", i);
		gameData.smoke.objects [i] = CreateSmoke (&objP->position.vPos, NULL, objP->nSegment, 1, nParts, nScale,
																gameOpts->render.smoke.bSyncSizes ? -1 : gameOpts->render.smoke.nSize [2],
																1, BOT_PART_LIFE, BOT_PART_SPEED, 0, i, NULL, 1);
		}
	else {
		SetSmokePartScale (gameData.smoke.objects [i], nScale);
		SetSmokeDensity (gameData.smoke.objects [i], nParts, gameOpts->render.smoke.bSyncSizes ? -1 : gameOpts->render.smoke.nSize [2]);
		SetSmokeSpeed (gameData.smoke.objects [i], 
							(objP->mType.physInfo.velocity.p.x || objP->mType.physInfo.velocity.p.y || objP->mType.physInfo.velocity.p.z) ?
							BOT_PART_SPEED : BOT_PART_SPEED * 2 / 3);
		}
	VmVecScaleAdd (&pos, &objP->position.vPos, &objP->position.mOrient.fVec, -objP->size / 2);
	SetSmokePos (gameData.smoke.objects [i], &pos);
	}
else 
	KillObjectSmoke (i);
}

//------------------------------------------------------------------------------

void DoReactorSmoke (tObject *objP)
{
	int			h = -1, i, nShields = 0, nParts;
	vmsVector	vDir, vPos;

if (!SHOW_SMOKE)
	return;
i = OBJ_IDX (objP);
if (!gameOpts->render.smoke.bRobots) {
	if (gameData.smoke.objects [i] >= 0)
		KillObjectSmoke (i);
	return;
	}
if ((objP->shields < 0) || (objP->flags & (OF_SHOULD_BE_DEAD | OF_DESTROYED)))
	nParts = 0;
else {
	nShields = f2ir (gameData.bots.info [gameStates.app.bD1Mission][objP->id].strength);
	h = nShields ? f2ir (objP->shields) * 100 / nShields : 0;
	}
if (h < 0)
	h = 0;	
nParts = 10 - h / 10;
if (nParts > 0) {
	nParts = REACTOR_MAX_PARTS;
	if (gameData.smoke.objects [i] < 0) {
		//LogErr ("creating robot %d smoke\n", i);
		gameData.smoke.objects [i] = CreateSmoke (&objP->position.vPos, NULL, objP->nSegment, 1, nParts, 3.0,
																-1, 1, BOT_PART_LIFE * 2, BOT_PART_SPEED, 0, i, NULL, 1);
		}
	else {
		SetSmokePartScale (gameData.smoke.objects [i], 0.5);
		SetSmokeDensity (gameData.smoke.objects [i], nParts, -1);
		vDir.p.x = d_rand () - F1_0 / 4;
		vDir.p.y = d_rand () - F1_0 / 4;
		vDir.p.z = d_rand () - F1_0 / 4;
		VmVecNormalize (&vDir);
		VmVecScaleAdd (&vPos, &objP->position.vPos, &vDir, -objP->size / 2);
		SetSmokePos (gameData.smoke.objects [i], &vPos);
		}
	}
else 
	KillObjectSmoke (i);
}

//------------------------------------------------------------------------------

void DoMissileSmoke (tObject *objP)
{
	int			nParts, nSpeed, nLife, i;
	float			nScale = 1.5f;
	vmsVector	pos;

if (!SHOW_SMOKE)
	return;
i = OBJ_IDX (objP);
if (!gameOpts->render.smoke.bMissiles) {
	if (gameData.smoke.objects [i] >= 0)
		KillObjectSmoke (i);
	return;
	}
if ((objP->shields < 0) || (objP->flags & (OF_SHOULD_BE_DEAD | OF_DESTROYED)))
	nParts = 0;
else {
	nSpeed = WI_speed (objP->id, gameStates.app.nDifficultyLevel);
	nLife = gameOpts->render.smoke.nLife [3] + 1;
#if 1
	nParts = (int) (MSL_MAX_PARTS * f2fl (nSpeed) / (40.0f * (4 - nLife)));
#else
	nParts = (objP->id == EARTHSHAKER_ID) ? 1500 : 
				(objP->id == MEGAMSL_ID) ? 1400 : 
				(objP->id == SMARTMSL_ID) ? 1300 : 
				1200;
#endif
	}
if (nParts) {
	if (gameData.smoke.objects [i] < 0) {
		if (!gameOpts->render.smoke.bSyncSizes) {
			nParts = -MAX_PARTICLES (nParts, gameOpts->render.smoke.nDens [3]);
			nScale = PARTICLE_SIZE (gameOpts->render.smoke.nSize [3], nScale);
			}
		gameData.smoke.objects [i] = CreateSmoke (&objP->position.vPos, NULL, objP->nSegment, 1, nParts, nScale,
																gameOpts->render.smoke.bSyncSizes ? -1 : gameOpts->render.smoke.nSize [3],
																1, nLife * MSL_PART_LIFE, MSL_PART_SPEED, 1, i, NULL, 1);
		}
	VmVecScaleAdd (&pos, &objP->position.vPos, &objP->position.mOrient.fVec, -objP->size);
	SetSmokePos (gameData.smoke.objects [i], &pos);
	}
else 
	KillObjectSmoke (i);
}

//------------------------------------------------------------------------------

void DoDebrisSmoke (tObject *objP)
{
	int			nParts, i;
	float			nScale = 2;
	vmsVector	pos;

if (!SHOW_SMOKE)
	return;
i = OBJ_IDX (objP);
if (!gameOpts->render.smoke.bDebris) {
	if (gameData.smoke.objects [i] >= 0)
		KillObjectSmoke (i);
	return;
	}
if ((objP->shields < 0) || (objP->flags & (OF_SHOULD_BE_DEAD | OF_DESTROYED)) ||
	 (gameOpts->render.nDebrisLife && (nDebrisLife [gameOpts->render.nDebrisLife] * F1_0 - objP->lifeleft > 10 * F1_0)))
	nParts = 0;
else 
	nParts = DEBRIS_MAX_PARTS;
if (nParts) {
	if (!gameOpts->render.smoke.bSyncSizes) {
		nParts = -MAX_PARTICLES (nParts, gameOpts->render.smoke.nDens [4]);
		nScale = PARTICLE_SIZE (gameOpts->render.smoke.nSize [4], nScale);
		}
	if (gameData.smoke.objects [i] < 0) {
		gameData.smoke.objects [i] = CreateSmoke (&objP->position.vPos, NULL, objP->nSegment, 1, nParts / 2,
																nScale, -1, 1, DEBRIS_PART_LIFE, DEBRIS_PART_SPEED, 2, i, NULL, 0);
		}
	VmVecScaleAdd (&pos, &objP->position.vPos, &objP->position.mOrient.fVec, -objP->size);
	SetSmokePos (gameData.smoke.objects [i], &pos);
	}
else 
	KillObjectSmoke (i);
}

//------------------------------------------------------------------------------

void DoStaticSmoke (tObject *objP)
{
	int			i, j;
	vmsVector	pos, offs, dir;

if (!SHOW_SMOKE)
	return;
i = (int) OBJ_IDX (objP);
if (!gameOpts->render.smoke.bStatic) {
	if (gameData.smoke.objects [i] >= 0)
		KillObjectSmoke (i);
	return;
	}
if (gameData.smoke.objects [i] < 0) {
	VmVecCopyScale (&dir, &objP->position.mOrient.fVec, objP->cType.smokeInfo.nSpeed * 2 * F1_0 / 55);
	gameData.smoke.objects [i] = CreateSmoke (&objP->position.vPos, &dir, 
															objP->nSegment, 1, -objP->cType.smokeInfo.nParts, 
															-PARTICLE_SIZE (objP->cType.smokeInfo.nSize [gameOpts->render.smoke.bDisperse], 2.0f), 
															-1, 3, STATIC_SMOKE_PART_LIFE * objP->cType.smokeInfo.nLife, 
															objP->cType.smokeInfo.nDrift, 2, i, NULL, 1);
	SetSmokeBrightness (gameData.smoke.objects [i], objP->cType.smokeInfo.nBrightness);
	}
i = objP->cType.smokeInfo.nDrift >> 4;
i += objP->cType.smokeInfo.nSize [0] >> 4;
i /= 2;
j = i - i / 2;
i /= 2;
offs.p.x = (F1_0 / 4 - d_rand ()) * (d_rand () % j + i);
offs.p.y = (F1_0 / 4 - d_rand ()) * (d_rand () % j + i);
offs.p.z = (F1_0 / 4 - d_rand ()) * (d_rand () % j + i);
VmVecAdd (&pos, &objP->position.vPos, &offs);
SetSmokePos (gameData.smoke.objects [i], &pos);
}

//------------------------------------------------------------------------------

void DoBombSmoke (tObject *objP)
{
	int			nParts, i;
	vmsVector	pos, offs;

if (gameStates.app.bNostalgia || !gameStates.app.bHaveExtraGameInfo [IsMultiGame])
	return;
i = OBJ_IDX (objP);
if ((objP->shields < 0) || (objP->flags & (OF_SHOULD_BE_DEAD | OF_DESTROYED)))
	nParts = 0;
else 
	nParts = -BOMB_MAX_PARTS;
if (nParts) {
	if (gameData.smoke.objects [i] < 0) {
		gameData.smoke.objects [i] = CreateSmoke (&objP->position.vPos, NULL, objP->nSegment, 1, nParts,
																-PARTICLE_SIZE (3, 0.5f), -1, 3, BOMB_PART_LIFE, BOMB_PART_SPEED, 1, i, NULL, 1);
		}
	offs.p.x = (F1_0 / 4 - d_rand ()) * ((d_rand () & 15) + 16);
	offs.p.y = (F1_0 / 4 - d_rand ()) * ((d_rand () & 15) + 16);
	offs.p.z = (F1_0 / 4 - d_rand ()) * ((d_rand () & 15) + 16);
	VmVecAdd (&pos, &objP->position.vPos, &offs);
	SetSmokePos (gameData.smoke.objects [i], &pos);
	}
else 
	KillObjectSmoke (i);
}

//------------------------------------------------------------------------------

void DoParticleTrail (tObject *objP)
{
	int			nParts, i;
	float			nScale;
	vmsVector	pos;
	tRgbaColord	c;

if (!SHOW_OBJ_FX && EGI_FLAG (bLightTrails, 1, 1, 0))
	return;
i = OBJ_IDX (objP);
if (!gameOpts->render.smoke.bPlasmaTrails) {
	if (gameData.smoke.objects [i] >= 0)
		KillObjectSmoke (i);
	return;
	}
#if 1
nParts = LASER_MAX_PARTS;
#else
nParts = gameData.weapons.info [objP->id].speed [0] / F1_0; 
#endif
c.red = (double) gameData.weapons.color [objP->id].red;
c.green = (double) gameData.weapons.color [objP->id].green;
c.blue = (double) gameData.weapons.color [objP->id].blue;
c.alpha = 0.5;
if (gameData.smoke.objects [i] < 0) {
	nScale = f2fl (objP->size);
	if (nScale >= 3)
		nScale = 1.5;
	else if (nScale >= 2)
		nScale = 2;
	else if (nScale >= 1)
		nScale = 2.5;
	else if (nScale >= 0.5f)
		nScale = 3;
	else
		nScale = 4;
	c.alpha = 0.1f + nScale / 10;
	gameData.smoke.objects [i] = CreateSmoke (&objP->position.vPos, NULL, objP->nSegment, 1, nParts, -PARTICLE_SIZE (gameOpts->render.smoke.nSize [0], nScale),
															gameOpts->render.smoke.bSyncSizes ? -1 : gameOpts->render.smoke.nSize [3],
															1, (gameOpts->render.smoke.nLife [3] + 1) * LASER_PART_LIFE, LASER_PART_SPEED, 3, i, &c, 0);
	}
VmVecScaleAdd (&pos, &objP->position.vPos, &objP->position.mOrient.fVec, -objP->size);
SetSmokePos (gameData.smoke.objects [i], &pos);
}

// -----------------------------------------------------------------------------

#if 0

#define SHRAPNEL_MAX_PARTS			400
#define SHRAPNEL_PART_LIFE			-1750
#define SHRAPNEL_PART_SPEED		10

int CreateShrapnels (tObject *parentObjP)
{
	tShrapnelData	*sdP;
	tShrapnel		*shrapnelP;
	vmsVector		vDir;
	float				fSize = f2fl (parentObjP->size), fSpeedScale, fPartScale;
	int				i, h = (int) (fSize * 5 / (1 << (4 - gameOpts->render.smoke.nShrapnels)));
	short				nObject;
	tObject			*objP;
	tRgbaColord		color = {1,1,1,0.5};

if (!SHOW_SMOKE)
	return 0;
if (!gameOpts->render.nExplShrapnels)
	return 0;
if (parentObjP->flags & OF_ARMAGEDDON)
	return 0;
if ((parentObjP->nType != OBJ_PLAYER) && (parentObjP->nType != OBJ_ROBOT))
	return 0;
nObject = CreateObject (OBJ_FIREBALL, 0, -1, parentObjP->nSegment, &parentObjP->position.vPos, &vmdIdentityMatrix, 
								1, CT_EXPLOSION, MT_NONE, RT_SHRAPNELS, 1);
if (nObject < 0)
	return 0;
objP = gameData.objs.objects + nObject;
objP->lifeleft = 0;
objP->cType.explInfo.nSpawnTime = -1;
objP->cType.explInfo.nDeleteObj = -1;
objP->cType.explInfo.nDeleteTime = -1;
sdP = gameData.objs.shrapnels + nObject;
h += d_rand () % h;
if (!(sdP->shrapnels = (tShrapnel *) D2_ALLOC (h * sizeof (tShrapnel))))
	return 0;
sdP->nShrapnels = h;
srand (gameStates.app.nSDLTicks);
fPartScale = (float) sqrt (fSize);
fSpeedScale = (float) sqrt (fPartScale);
for (i = h, shrapnelP = sdP->shrapnels; i; i--, shrapnelP++) {
	vDir.p.x = F1_0 / 4 - d_rand ();
	vDir.p.y = F1_0 / 4 - d_rand ();
	vDir.p.z = F1_0 / 4 - d_rand ();
	VmVecNormalize (&vDir);
	shrapnelP->vOffs = 
	shrapnelP->vDir = vDir;
	VmVecScaleAdd (&shrapnelP->vPos, &parentObjP->position.vPos, &vDir, parentObjP->size / 2 + rand () % (parentObjP->size / 2));
	shrapnelP->nTurn = 1;
	shrapnelP->xSpeed = (fix) ((3 * F1_0 / 2 + rand () % (F1_0 / 2)) * fPartScale);
	shrapnelP->xBaseSpeed = shrapnelP->xSpeed / 10;
	shrapnelP->xSpeed -= shrapnelP->xBaseSpeed;
	shrapnelP->xLife = 3 * F1_0 / 2 + d_rand ();
	if (objP->lifeleft < shrapnelP->xLife)
		objP->lifeleft = shrapnelP->xLife;
	shrapnelP->xTTL = (fix) (shrapnelP->xLife / 65.536); //3 * F1_0 / 2 + d_rand ();
	shrapnelP->xLife = shrapnelP->xTTL;
	shrapnelP->d = 0;
	shrapnelP->vStartPos = shrapnelP->vPos;
	shrapnelP->tUpdate = gameStates.app.nSDLTicks - 25;
	shrapnelP->nParts = (fix) (-SHRAPNEL_MAX_PARTS * fPartScale) + 50; // * shrapnelP->xTTL / MAX_SHRAPNEL_LIFE;
	shrapnelP->fScale = 45.0 / 80.0;
	shrapnelP->nSmoke = CreateSmoke (&shrapnelP->vPos, NULL, objP->nSegment, 1, shrapnelP->nParts,
											   -PARTICLE_SIZE (1, 4), -1, 1, /*shrapnelP->xTTL * 4000 /*/ SHRAPNEL_PART_LIFE, 
												SHRAPNEL_PART_SPEED, 1, 0x7fffffff, &color, 1);
	}
objP->lifeleft += objP->lifeleft;
objP->cType.explInfo.nSpawnTime = -1;
objP->cType.explInfo.nDeleteObj = -1;
objP->cType.explInfo.nDeleteTime = -1;
return 1;
}

// -----------------------------------------------------------------------------

void DestroyShrapnels (tObject *objP)
{
	tShrapnelData	*sdP = gameData.objs.shrapnels + OBJ_IDX (objP);

if (sdP->shrapnels) {
	int	i, h = sdP->nShrapnels;

	sdP->nShrapnels = 0;
	for (i = 0; i < h; i++)
		if (sdP->shrapnels [i].nSmoke >= 0)
			SetSmokeLife (sdP->shrapnels [i].nSmoke, 0);
	D2_FREE (sdP->shrapnels);
	sdP->shrapnels = 0;
	}
objP->lifeleft = -1;
}

// -----------------------------------------------------------------------------

void MoveShrapnel (tShrapnel *shrapnelP)
{
	fix			xSpeed = shrapnelP->xSpeed,
					xBaseSpeed = shrapnelP->xBaseSpeed;
	vmsVector	vOffs;
	time_t		nTicks;
	double		fParts = shrapnelP->nParts, 
					fPartScale = shrapnelP->fScale;
	int			d;

if ((nTicks = gameStates.app.nSDLTicks - shrapnelP->tUpdate) < 25)
	return;
SetSmokeDensity (shrapnelP->nSmoke, shrapnelP->nParts - 50, -1);
for (; nTicks >= 25; nTicks -= 25) {
	if (--(shrapnelP->nTurn))
		vOffs = shrapnelP->vOffs;
	else {
		shrapnelP->nTurn = ((shrapnelP->xTTL / 500) + 1) + rand () % 4;
		vOffs = shrapnelP->vDir;
		vOffs.p.x = (fix) ((double) vOffs.p.x * (2 * (double) d_rand () / 32767.0));
		vOffs.p.y = (fix) ((double) vOffs.p.y * (2 * (double) d_rand () / 32767.0));
		vOffs.p.z = (fix) ((double) vOffs.p.z * (2 * (double) d_rand () / 32767.0));
		VmVecNormalize (&vOffs);
		shrapnelP->vOffs = vOffs;
		}
	xSpeed -= (fix) (xSpeed * (1 - 0.85) / gameStates.gameplay.slowmo [0].fSpeed);
	VmVecScaleInc (&shrapnelP->vPos, &vOffs, (fix) ((xBaseSpeed + xSpeed) / gameStates.gameplay.slowmo [0].fSpeed));
	d = VmVecDist (&shrapnelP->vPos, &shrapnelP->vStartPos);
	if (d < shrapnelP->d)
		d = d;
	else
		shrapnelP->d = d;
	fParts -= fParts * (1 - 0.9125) / gameStates.gameplay.slowmo [0].fSpeed;
	}
shrapnelP->xSpeed = xSpeed;
shrapnelP->nParts = (int) (fParts + 0.5);
shrapnelP->fScale = fPartScale;
SetSmokePos (shrapnelP->nSmoke, &shrapnelP->vPos);
shrapnelP->tUpdate = gameStates.app.nSDLTicks - nTicks;
}

// -----------------------------------------------------------------------------

void DrawShrapnel (tShrapnel *shrapnelP)
{
if (shrapnelP->xTTL > 0) {
	if (LoadExplBlast ()) {
		fix	xSize = F1_0 / 2 + d_rand () % (F1_0 / 4);
		glDepthMask (0);
		G3DrawSprite (&shrapnelP->vPos, xSize, xSize, bmpExplBlast, NULL, f2fl (shrapnelP->xTTL) / f2fl (shrapnelP->xLife) / 2);
		glDepthMask (1);
		}
	}
}

// -----------------------------------------------------------------------------

void DrawShrapnels (tObject *objP)
{
	tShrapnelData	*sdP = gameData.objs.shrapnels + OBJ_IDX (objP);
	tShrapnel		*shrapnelP = sdP->shrapnels;
	int				h, i;

for (i = 0, h = sdP->nShrapnels; i < h; i++)
	DrawShrapnel (shrapnelP);
}

// -----------------------------------------------------------------------------

int UpdateShrapnels (tObject *objP)
{
	tShrapnelData	*sdP = gameData.objs.shrapnels + OBJ_IDX (objP);
	tShrapnel		*shrapnelP = sdP->shrapnels;
	int				h, i;

if (!gameStates.app.tick40fps.bTick)
	return 0;
if (objP->lifeleft > 0) {
	for (i = 0, h = sdP->nShrapnels; i < h; ) {
		if (shrapnelP->xTTL <= 0)
			continue;
		MoveShrapnel (shrapnelP);
		if (0 < (shrapnelP->xTTL -= (fix) (gameStates.app.tick40fps.nTime / gameStates.gameplay.slowmo [0].fSpeed))) {
			shrapnelP++;
			i++;
			}
		else {
			SetSmokeLife (shrapnelP->nSmoke, 0);
			shrapnelP->nSmoke = -1;
			if (i < --h)
				*shrapnelP = sdP->shrapnels [h];
			}
		}
	if (sdP->nShrapnels = h)
		return 1;
	}
DestroyShrapnels (objP);
return 0;
}

#else

// -----------------------------------------------------------------------------

#define SHRAPNEL_MAX_PARTS			500
#define SHRAPNEL_PART_LIFE			-1750
#define SHRAPNEL_PART_SPEED		10

static float fShrapnelScale [5] = {0, 5.0f / 3.0f, 2.5f, 10.0f / 3.0f, 5};

int CreateShrapnels (tObject *parentObjP)
{
	tShrapnelData	*sdP;
	tShrapnel		*shrapnelP;
	vmsVector		vDir;
	int				i, h = (int) (f2fl (parentObjP->size) * fShrapnelScale [gameOpts->render.nExplShrapnels] + 0.5);
	short				nObject;
	tObject			*objP;
	tRgbaColord		color = {1,1,1,0.5};

if (!SHOW_SMOKE)
	return 0;
if (!gameOpts->render.nExplShrapnels)
	return 0;
if (parentObjP->flags & OF_ARMAGEDDON)
	return 0;
if ((parentObjP->nType != OBJ_PLAYER) && (parentObjP->nType != OBJ_ROBOT))
	return 0;
nObject = CreateObject (OBJ_FIREBALL, 0, -1, parentObjP->nSegment, &parentObjP->position.vPos, &vmdIdentityMatrix, 
								1, CT_EXPLOSION, MT_NONE, RT_SHRAPNELS, 1);
if (nObject < 0)
	return 0;
objP = gameData.objs.objects + nObject;
objP->lifeleft = 0;
objP->cType.explInfo.nSpawnTime = -1;
objP->cType.explInfo.nDeleteObj = -1;
objP->cType.explInfo.nDeleteTime = -1;
sdP = gameData.objs.shrapnels + nObject;
h += d_rand () % h;
if (!(sdP->shrapnels = (tShrapnel *) D2_ALLOC (h * sizeof (tShrapnel))))
	return 0;
sdP->nShrapnels = h;
srand (gameStates.app.nSDLTicks);
for (i = h, shrapnelP = sdP->shrapnels; i; i--, shrapnelP++) {
	vDir.p.x = F1_0 /4 - d_rand () % (2 * F1_0);
	vDir.p.y = F1_0 /4 - d_rand () % (2 * F1_0);
	vDir.p.z = F1_0 /4 - d_rand () % (2 * F1_0);
	VmVecNormalize (&vDir);
	shrapnelP->vDir = vDir;
	VmVecScaleAdd (&shrapnelP->vPos, &parentObjP->position.vPos, &vDir, parentObjP->size / 4 + rand () % (parentObjP->size / 2));
	shrapnelP->nTurn = 1;
	shrapnelP->xSpeed = 3 * (F1_0 / 20 + rand () % (F1_0 / 20)) / 4;
	shrapnelP->xLife = 
	shrapnelP->xTTL = 3 * F1_0 / 2 + rand ();
	shrapnelP->tUpdate = gameStates.app.nSDLTicks;
	if (objP->lifeleft < shrapnelP->xLife)
		objP->lifeleft = shrapnelP->xLife;
	shrapnelP->nSmoke = CreateSmoke (&shrapnelP->vPos, NULL, objP->nSegment, 1, -SHRAPNEL_MAX_PARTS,
											   -PARTICLE_SIZE (1, 4), -1, 1, SHRAPNEL_PART_LIFE , SHRAPNEL_PART_SPEED, 1, 0x7fffffff, &color, 1);
	}
objP->lifeleft *= 2;
objP->cType.explInfo.nSpawnTime = -1;
objP->cType.explInfo.nDeleteObj = -1;
objP->cType.explInfo.nDeleteTime = -1;
return 1;
}

// -----------------------------------------------------------------------------

void DestroyShrapnels (tObject *objP)
{
	tShrapnelData	*sdP = gameData.objs.shrapnels + OBJ_IDX (objP);

if (sdP->shrapnels) {
	int	i, h = sdP->nShrapnels;

	sdP->nShrapnels = 0;
	for (i = 0; i < h; i++)
		if (sdP->shrapnels [i].nSmoke >= 0)
			SetSmokeLife (sdP->shrapnels [i].nSmoke, 0);
	D2_FREE (sdP->shrapnels);
	sdP->shrapnels = 0;
	}
objP->lifeleft = -1;
}

// -----------------------------------------------------------------------------

void MoveShrapnel (tShrapnel *shrapnelP)
{
	fix			xSpeed = FixDiv (shrapnelP->xSpeed, 25 * F1_0 / 1000);
	vmsVector	vOffs;
	time_t		nTicks;

if ((nTicks = gameStates.app.nSDLTicks - shrapnelP->tUpdate) < 25)
	return;
xSpeed = (fix) (xSpeed / gameStates.gameplay.slowmo [0].fSpeed);
for (; nTicks >= 25; nTicks -= 25) {
	if (--(shrapnelP->nTurn))
		vOffs = shrapnelP->vOffs;
	else {
		shrapnelP->nTurn = ((shrapnelP->xTTL > F1_0 / 2) ? 2 : 4) + d_rand () % 4;
		vOffs = shrapnelP->vDir;
		vOffs.p.x = (fix) (vOffs.p.x * (double) (rand () % F1_0) / 65536.0);
		vOffs.p.y = (fix) (vOffs.p.y * (double) (rand () % F1_0) / 65536.0);
		vOffs.p.z = (fix) (vOffs.p.z * (double) (rand () % F1_0) / 65536.0);
		VmVecNormalize (&vOffs);
		shrapnelP->vOffs = vOffs;
		}
	VmVecScale (&vOffs, xSpeed);
	VmVecInc (&shrapnelP->vPos, &vOffs);
	}
SetSmokePos (shrapnelP->nSmoke, &shrapnelP->vPos);
shrapnelP->tUpdate = gameStates.app.nSDLTicks - nTicks;
}

// -----------------------------------------------------------------------------

void DrawShrapnel (tShrapnel *shrapnelP)
{
if (shrapnelP->xTTL > 0) {
	if (LoadExplBlast ()) {
		fix	xSize = F1_0 / 2 + d_rand () % (F1_0 / 4);
		glDepthMask (0);
		G3DrawSprite (&shrapnelP->vPos, xSize, xSize, bmpExplBlast, NULL, f2fl (shrapnelP->xTTL) / f2fl (shrapnelP->xLife) / 2);
		glDepthMask (1);
		}
	}
}

// -----------------------------------------------------------------------------

void DrawShrapnels (tObject *objP)
{
	tShrapnelData	*sdP = gameData.objs.shrapnels + OBJ_IDX (objP);
	tShrapnel		*shrapnelP = sdP->shrapnels;
	int				h, i;

for (i = 0, h = sdP->nShrapnels; i < h; i++)
	DrawShrapnel (shrapnelP);
}

// -----------------------------------------------------------------------------

int UpdateShrapnels (tObject *objP)
{
	tShrapnelData	*sdP = gameData.objs.shrapnels + OBJ_IDX (objP);
	tShrapnel		*shrapnelP = sdP->shrapnels;
	int				h, i;

if (!gameStates.app.tick40fps.bTick)
	return 0;
if (objP->lifeleft > 0) {
	for (i = 0, h = sdP->nShrapnels; i < h; ) {
		if (shrapnelP->xTTL <= 0)
			continue;
		MoveShrapnel (shrapnelP);
		if (0 < (shrapnelP->xTTL -= (fix) (secs2f (gameStates.app.tick40fps.nTime) / gameStates.gameplay.slowmo [0].fSpeed))) {
			shrapnelP++;
			i++;
			}
		else {
			SetSmokeLife (shrapnelP->nSmoke, 0);
			shrapnelP->nSmoke = -1;
			if (i < --h)
				*shrapnelP = sdP->shrapnels [h];
			}
		}
	if (sdP->nShrapnels = h)
		return 1;
	}
DestroyShrapnels (objP);
return 0;
}

#endif

//------------------------------------------------------------------------------

int DoObjectSmoke (tObject *objP)
{
int t = objP->nType;
#if 0
if (extraGameInfo [0].bShadows && (gameStates.render.nShadowPass < 3))
	return;
#endif
if (t == OBJ_PLAYER)
	DoPlayerSmoke (objP, -1);
else if (t == OBJ_ROBOT)
	DoRobotSmoke (objP);
else if (t == OBJ_SMOKE)
	DoStaticSmoke (objP);
else if (t == OBJ_CNTRLCEN)
	DoReactorSmoke (objP);
else if (t == OBJ_WEAPON) {
	if (gameData.objs.bIsMissile [objP->id])
		DoMissileSmoke (objP);
	else if (!COMPETITION && EGI_FLAG (bSmokeGrenades, 0, 0, 0) && (objP->id == PROXMINE_ID))
		DoBombSmoke (objP);
	else if (gameData.objs.bIsWeapon [objP->id])
		DoParticleTrail (objP);
	else
		return 0;
	}
else if (t == OBJ_DEBRIS)
	DoDebrisSmoke (objP);
else
	return 0;
return 1;
}

//------------------------------------------------------------------------------

void PlayerSmokeFrame (void)
{
	int	i;

if (!gameOpts->render.smoke.bPlayers)
	return;
for (i = 0; i < gameData.multiplayer.nPlayers; i++)
	DoPlayerSmoke (gameData.objs.objects + gameData.multiplayer.players [i].nObject, i);
}

//------------------------------------------------------------------------------

void ObjectSmokeFrame (void)
{
	int		i;
	tObject	*objP;

if (!SHOW_SMOKE)
	return;
if (!gameOpts->render.smoke.bRobots)
	return;
for (i = 0, objP = gameData.objs.objects; i <= gameData.objs.nLastObject; i++, objP++)
	if (objP->nType == OBJ_NONE)
		KillObjectSmoke (i);
}

//------------------------------------------------------------------------------

void StaticSmokeFrame (void)
{
	tObject	*objP = gameData.objs.objects;
	int		i;

if (!SHOW_SMOKE)
	return;
for (i = gameData.objs.nLastObject + 1; i; i--, objP++)
	if (objP->nType == OBJ_SMOKE)
		DoStaticSmoke (objP);
}

//------------------------------------------------------------------------------

void DoSmokeFrame (void)
{
#if SHADOWS
if (gameStates.render.nShadowPass > 1)
	return;
#endif
#ifdef _DEBUG
if (!gameStates.render.bExternalView)
#else
if (!gameStates.render.bExternalView && (!IsMultiGame || IsCoopGame || EGI_FLAG (bEnableCheats, 0, 0, 0)))
#endif
	DoPlayerSmoke (gameData.objs.viewer, gameData.multiplayer.nLocalPlayer);
ObjectSmokeFrame ();
StaticSmokeFrame ();
//if (SHOW_SMOKE)
	UpdateSmoke ();
}

//------------------------------------------------------------------------------
