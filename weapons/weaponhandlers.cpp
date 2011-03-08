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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "descent.h"
#include "objrender.h"
#include "lightning.h"
#include "trackobject.h"
#include "omega.h"
#include "segpoint.h"
#include "error.h"
#include "key.h"
#include "texmap.h"
#include "textures.h"
#include "rendermine.h"
#include "fireball.h"
#include "newdemo.h"
#include "timer.h"
#include "physics.h"
#include "segmath.h"
#include "input.h"
#include "dropobject.h"
#include "lightcluster.h"
#include "visibility.h"

#ifdef TACTILE
#include "tactile.h"
#endif

//	--------------------------------------------------------------------------------------------------

#if defined(_WIN32) && !DBG
typedef int ( __fastcall * pWeaponHandler) (CObject *, int, int&, int);
#else
typedef int (* pWeaponHandler) (CObject *, int, int&, int);
#endif

//-------------------------------------------

int LaserHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
	ubyte	nLaser = (nLevel <= MAX_LASER_LEVEL) ? LASER_ID + nLevel : SUPERLASER_ID + (nLevel - MAX_LASER_LEVEL - 1);
	short	nLightObj = lightClusterManager.Create (objP);
	short nFired = 0;

gameData.laser.nOffset = (I2X (2) * (d_rand () % 8)) / 8;
if (0 <= LaserPlayerFire (objP, nLaser, 0, 1, 0, nLightObj))
	nFired++;
if (0 <= LaserPlayerFire (objP, nLaser, 1, 0, 0, nLightObj))
	nFired++;
if (nFlags & LASER_QUAD) {
	nFired += 2;
	//	hideous system to make quad laser 1.5x powerful as Normal laser, make every other quad laser bolt bHarmless
	if (0 <= LaserPlayerFire (objP, nLaser, 2, 0, 0, nLightObj))
		nFired++;
	if (0 <= LaserPlayerFire (objP, nLaser, 3, 0, 0, nLightObj))
		nFired++;
	}
if (!nFired && (nLightObj >= 0))
	OBJECTS [nLightObj].Die ();
return nFired ? nRoundsPerShot : 0;
}

//-------------------------------------------

int VulcanHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
#	define VULCAN_SPREAD	(d_rand ()/8 - 32767/16)

	int			bGatlingSound = gameStates.app.bHaveExtraGameInfo [IsMultiGame] &&
										 (gameOpts->UseHiresSound () == 2) && gameOpts->sound.bGatling;
	tFiringData *fP = gameData.multiplayer.weaponStates [objP->info.nId].firing;
	short			nFired = 0;

if (bGatlingSound && (fP->nDuration <= GATLING_DELAY))
	return 0;
//	Only make sound for 1/4 of vulcan bullets.
if (0 <= LaserPlayerFireSpread (objP, VULCAN_ID, 6, VULCAN_SPREAD, VULCAN_SPREAD, 1, 0, -1))
	nFired++;
if (nRoundsPerShot > 1) {
	if (0 <= LaserPlayerFireSpread (objP, VULCAN_ID, 6, VULCAN_SPREAD, VULCAN_SPREAD, 0, 0, -1))
	nFired++;
	if (nRoundsPerShot > 2)
		if (0 <= LaserPlayerFireSpread (objP, VULCAN_ID, 6, VULCAN_SPREAD, VULCAN_SPREAD, 0, 0, -1))
	nFired++;
	}
return nFired ? nRoundsPerShot : 0;
}

//-------------------------------------------

int SpreadfireHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
	short	nLightObj = lightClusterManager.Create (objP);
	short	nFired = 0;

if (nFlags & LASER_SPREADFIRE_TOGGLED) {
	if (0 <= LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, I2X (1) / 16, 0, 0, 0, nLightObj))
		nFired++;
	if (0 <= LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, -I2X (1) / 16, 0, 0, 0, nLightObj))
		nFired++;
	}
else {
	if (0 <= LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, 0, I2X (1) / 16, 0, 0, nLightObj))
		nFired++;
	if (0 <= LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, 0, -I2X (1) / 16, 0, 0, nLightObj))
		nFired++;
	}
if (0 <= LaserPlayerFireSpread (objP, SPREADFIRE_ID, 6, 0, 0, 1, 0, nLightObj))
	nFired++;
if (!nFired && (nLightObj >= 0))
	OBJECTS [nLightObj].Die ();
return nFired ? nRoundsPerShot : 0;
}

//-------------------------------------------

int PlasmaHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
	short	nLightObj = lightClusterManager.Create (objP);
	short	nFired = 0;

if (0 <= LaserPlayerFire (objP, PLASMA_ID, 0, 1, 0, nLightObj))
	nFired++;
if (0 <= LaserPlayerFire (objP, PLASMA_ID, 1, 0, 0, nLightObj))
	nFired++;
if (!nFired && (nLightObj >= 0))
	OBJECTS [nLightObj].Die ();
if (nRoundsPerShot > 1) {
	nLightObj = lightClusterManager.Create (objP);
	if (0 <= FireWeaponDelayedWithSpread (objP, PLASMA_ID, 0, 0, 0, gameData.time.xFrame / 2, 1, 0, nLightObj))
		nFired++;
	if (0 <= FireWeaponDelayedWithSpread (objP, PLASMA_ID, 1, 0, 0, gameData.time.xFrame / 2, 0, 0, nLightObj))
		nFired++;
	if (!nFired && (nLightObj >= 0))
		OBJECTS [nLightObj].Die ();
	}
return nFired ? nRoundsPerShot : 0;
}

//-------------------------------------------

int FusionHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
	CFixVector	vForce;
	short			nLightObj = lightClusterManager.Create (objP);
	short			nFired = 0;

if (0 <= LaserPlayerFire (objP, FUSION_ID, 0, 1, 0, nLightObj))
	nFired++;
if (0 <= LaserPlayerFire (objP, FUSION_ID, 1, 1, 0, nLightObj))
	nFired++;
if (EGI_FLAG (bTripleFusion, 0, 0, 0) && gameData.multiplayer.weaponStates [objP->info.nId].bTripleFusion)
	if (0 <= LaserPlayerFire (objP, FUSION_ID, 6, 1, 0, nLightObj))
		nFired++;
nFlags = sbyte (gameData.FusionCharge () >> 12);
gameData.SetFusionCharge (0);
if (!nFired) {
	if (nLightObj >= 0)
		OBJECTS [nLightObj].Die ();
	return 0;
	}
vForce.v.c.x = -(objP->info.position.mOrient.m.v.f.v.c.x << 7);
vForce.v.c.y = -(objP->info.position.mOrient.m.v.f.v.c.y << 7);
vForce.v.c.z = -(objP->info.position.mOrient.m.v.f.v.c.z << 7);
objP->ApplyForce (vForce);
vForce.v.c.x = (vForce.v.c.x >> 4) + d_rand () - 16384;
vForce.v.c.y = (vForce.v.c.y >> 4) + d_rand () - 16384;
vForce.v.c.z = (vForce.v.c.z >> 4) + d_rand () - 16384;
objP->ApplyRotForce (vForce);
return nRoundsPerShot;
}

//-------------------------------------------

int SuperlaserHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
	ubyte nSuperLevel = 3;		//make some new kind of laser eventually
	short	nLightObj = lightClusterManager.Create (objP);
	short	nFired = 0;

if (0 <= LaserPlayerFire (objP, nSuperLevel, 0, 1, 0, nLightObj))
	nFired++;
if (0 <= LaserPlayerFire (objP, nSuperLevel, 1, 0, 0, nLightObj))
	nFired++;

if (nFlags & LASER_QUAD) {
	//	hideous system to make quad laser 1.5x powerful as Normal laser, make every other quad laser bolt bHarmless
	if (0 <= LaserPlayerFire (objP, nSuperLevel, 2, 0, 0, nLightObj))
		nFired++;
	if (0 <= LaserPlayerFire (objP, nSuperLevel, 3, 0, 0, nLightObj))
		nFired++;
	}
if (!nFired && (nLightObj >= 0))
	OBJECTS [nLightObj].Die ();
return nFired ? nRoundsPerShot : 0;
}

//-------------------------------------------

int GaussHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
#	define GAUSS_SPREAD		(VULCAN_SPREAD / 5)

	int			bGatlingSound = gameStates.app.bHaveExtraGameInfo [IsMultiGame] &&
										 (gameOpts->UseHiresSound () == 2) && gameOpts->sound.bGatling;
	tFiringData *fP = gameData.multiplayer.weaponStates [objP->info.nId].firing;
	short			nFired = 0;

if (bGatlingSound && (fP->nDuration <= GATLING_DELAY))
	return 0;
//	Only make sound for 1/4 of vulcan bullets.
if (0 <= LaserPlayerFireSpread (objP, GAUSS_ID, 6, GAUSS_SPREAD, GAUSS_SPREAD,
										  (objP->info.nId != gameData.multiplayer.nLocalPlayer) || (gameData.laser.xNextFireTime > gameData.time.xGame), 0, -1))
	nFired++;
if (nRoundsPerShot > 1) {
	if (0 <= LaserPlayerFireSpread (objP, GAUSS_ID, 6, GAUSS_SPREAD, GAUSS_SPREAD, 0, 0, -1))
		nFired++;
	if (nRoundsPerShot > 2)
		if (0 <= LaserPlayerFireSpread (objP, GAUSS_ID, 6, GAUSS_SPREAD, GAUSS_SPREAD, 0, 0, -1))
			nFired++;
	}
return nFired ? nRoundsPerShot : 0;
}

//-------------------------------------------

int HelixHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
	typedef struct tSpread {
		fix	r, u;
		} tSpread;

	static tSpread spreadTable [8] = {
	 {I2X (1) / 16, 0},
	 {I2X (1) / 17, I2X (1) / 42},
	 {I2X (1) / 22, I2X (1) / 22},
	 {I2X (1) / 42, I2X (1) / 17},
	 {0, I2X (1) / 16},
	 {-I2X (1) / 42, I2X (1) / 17},
	 {-I2X (1) / 22, I2X (1) / 22},
	 {-I2X (1) / 17, I2X (1) / 42}
		};

	tSpread	spread = spreadTable [(nFlags >> LASER_HELIX_SHIFT) & LASER_HELIX_MASK];
	short		nLightObj = lightClusterManager.Create (objP);
	short		nFired = 0;

if (0 <= LaserPlayerFireSpread (objP, HELIX_ID, 6,  0,  0, 1, 0, nLightObj))
	nFired++;
if (0 <= LaserPlayerFireSpread (objP, HELIX_ID, 6,  spread.r,  spread.u, 0, 0, nLightObj))
	nFired++;
if (0 <= LaserPlayerFireSpread (objP, HELIX_ID, 6, -spread.r, -spread.u, 0, 0, nLightObj))
	nFired++;
if (0 <= LaserPlayerFireSpread (objP, HELIX_ID, 6,  spread.r * 2,  spread.u * 2, 0, 0, nLightObj))
	nFired++;
if (0 <= LaserPlayerFireSpread (objP, HELIX_ID, 6, -spread.r * 2, -spread.u * 2, 0, 0, nLightObj))
	nFired++;
if (!nFired && (nLightObj >= 0))
	OBJECTS [nLightObj].Die ();
return nFired ? nRoundsPerShot : 0;
}

//-------------------------------------------

int PhoenixHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
	short	nLightObj = lightClusterManager.Create (objP);
	short	nFired = 0;

if (0 <= LaserPlayerFire (objP, PHOENIX_ID, 0, 1, 0, nLightObj))
	nFired++;;
if (0 <= LaserPlayerFire (objP, PHOENIX_ID, 1, 0, 0, nLightObj))
	nFired++;
if (!nFired && (nLightObj >= 0))
	OBJECTS [nLightObj].Die ();
if (nRoundsPerShot > 1) {
	nLightObj = lightClusterManager.Create (objP);
	if (0 <= FireWeaponDelayedWithSpread (objP, PHOENIX_ID, 0, 0, 0, gameData.time.xFrame / 2, 1, 0, nLightObj))
	nFired++;
	if (0 <= FireWeaponDelayedWithSpread (objP, PHOENIX_ID, 1, 0, 0, gameData.time.xFrame / 2, 0, 0, nLightObj))
	nFired++;
	if (!nFired && (nLightObj >= 0))
		OBJECTS [nLightObj].Die ();
	}
return nFired ? nRoundsPerShot : 0;
}

//-------------------------------------------

int OmegaHandler (CObject *objP, int nLevel, int& nFlags, int nRoundsPerShot)
{
LaserPlayerFire (objP, OMEGA_ID, 6, 1, 0, -1);
return nRoundsPerShot;
}

//-------------------------------------------

pWeaponHandler weaponHandlers [] = {
	LaserHandler,
	VulcanHandler,
	SpreadfireHandler,
	PlasmaHandler,
	FusionHandler,
	SuperlaserHandler,
	GaussHandler,
	HelixHandler,
	PhoenixHandler,
	OmegaHandler
	};



//	--------------------------------------------------------------------------------------------------
//	Object "nObject" fires weapon "weapon_num" of level "level". (Right now (9/24/94) level is used only for nType 0 laser.
//	Flags are the CPlayerData flags.  For network mode, set to 0.
//	It is assumed that this is a CPlayerData CObject (as in multiplayer), and therefore the gun positions are known.
//	Returns number of times a weapon was fired.  This is typically 1, but might be more for low frame rates.
//	More than one shot is fired with a pseudo-delay so that players on show machines can fire (for themselves
//	or other players) often enough for things like the vulcan cannon.

int FireWeapon (short nObject, ubyte nWeapon, int nLevel, int& nFlags, int nRoundsPerShot)
{
if (nWeapon > OMEGA_INDEX) {
	gameData.weapons.nPrimary = 0;
	nRoundsPerShot = 0;
	}
else {
	gameData.multigame.laser.nFired [0] = 0;
	nRoundsPerShot = weaponHandlers [nWeapon] (OBJECTS + nObject, nLevel, nFlags, nRoundsPerShot);
	}
if (IsMultiGame && (nObject == LOCALPLAYER.nObject)) {
	gameData.multigame.laser.bFired = nRoundsPerShot;
	gameData.multigame.laser.nGun = nWeapon;
	gameData.multigame.laser.nFlags = nFlags;
	gameData.multigame.laser.nLevel = nLevel;
	}
return nRoundsPerShot;
}

//	-------------------------------------------------------------------------------------------
// eof
