/*
 *
 * Player Stuff
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include "descent.h"
#include "input.h"
#include "network.h"
#include "marker.h"

tShipModifier shipModifiers [MAX_SHIP_TYPES] = {
	{{1.0f, 1.0f, 1.0f}},
	{{0.9f, 0.9f, 1.1f}},
	{{1.5f, 1.2f, 0.9f}}
};

//-------------------------------------------------------------------------
// reads a CPlayerShip structure from a CFile
 
void PlayerShipRead (CPlayerShip *ps, CFile& cf)
{
ps->nModel = cf.ReadInt ();
ps->nExplVClip = cf.ReadInt ();
ps->mass = cf.ReadFix ();
ps->drag = cf.ReadFix ();
ps->maxThrust = cf.ReadFix ();
ps->reverseThrust = cf.ReadFix ();
ps->brakes = cf.ReadFix ();
ps->wiggle = cf.ReadFix ();
ps->maxRotThrust = cf.ReadFix ();
for (int i = 0; i < N_PLAYER_GUNS; i++)
	cf.ReadVector (ps->gunPoints[i]);
}

//-------------------------------------------------------------------------

int EquippedPlayerGun (CObject *objP)
{
if (objP->info.nType == OBJ_PLAYER) {
		int		nPlayer = objP->info.nId;
		int		nWeapon = gameData.multiplayer.weaponStates [nPlayer].nPrimary;

	return (nWeapon || (gameData.multiplayer.weaponStates [nPlayer].nLaserLevel <= MAX_LASER_LEVEL)) ? nWeapon : SUPER_LASER_INDEX;
	}
return 0;
}

//-------------------------------------------------------------------------

static int nBombIds [] = {SMART_INDEX, MEGA_INDEX, EARTHSHAKER_INDEX};

int EquippedPlayerBomb (CObject *objP)
{
if (objP->info.nType == OBJ_PLAYER) {
		int		nPlayer = objP->info.nId;
		int		i, nWeapon = gameData.multiplayer.weaponStates [nPlayer].nSecondary;

	for (i = 0; i < (int) sizeofa (nBombIds); i++)
		if (nWeapon == nBombIds [i])
			return i + 1;
	}
return 0;
}

//-------------------------------------------------------------------------

static int nMissileIds [] = {CONCUSSION_INDEX, HOMING_INDEX, FLASHMSL_INDEX, GUIDED_INDEX, MERCURY_INDEX};

int EquippedPlayerMissile (CObject *objP, int *nMissiles)
{
if (objP->info.nType == OBJ_PLAYER) {
		int		nPlayer = objP->info.nId;
		int		i, nWeapon = gameData.multiplayer.weaponStates [nPlayer].nSecondary;

	for (i = 0; i < (int) sizeofa (nMissileIds); i++)
		if (nWeapon == nMissileIds [i]) {
			*nMissiles = gameData.multiplayer.weaponStates [nPlayer].nMissiles;
			return i + 1;
			}
	}
*nMissiles = 0;
return 0;
}

//-------------------------------------------------------------------------

#if 0
static inline int WIFireTicks (int nWeapon)
{
return 1000 * WI_fire_wait (nWeapon) / I2X (1);
}
#endif

//-------------------------------------------------------------------------

void UpdateFiringSounds (void)
{
	CWeaponState	*wsP = gameData.multiplayer.weaponStates;
	tFiringData		*fP;
	int				bGatling, bGatlingSound, i;

bGatlingSound = (gameOpts->UseHiresSound () == 2) && gameOpts->sound.bGatling;
for (i = 0; i < gameData.multiplayer.nPlayers; i++, wsP++) {
	if (!IsMultiGame || gameData.multiplayer.players [i].Connected ()) {
		bGatling = (wsP->nPrimary == VULCAN_INDEX) || (wsP->nPrimary == GAUSS_INDEX);
		fP = wsP->firing;
		if (bGatling && bGatlingSound && (fP->bSound == 1)) {
			audio.CreateObjectSound (-1, SOUNDCLASS_PLAYER, (short) gameData.multiplayer.players [i].nObject, 0, 
											 I2X (1), I2X (256), -1, -1, AddonSoundName (SND_ADDON_GATLING_SPIN), 0);
			fP->bSound = 0;
			}
		}
	}
}

//-------------------------------------------------------------------------

void UpdateFiringState (void)
{
	int	bGatling = (gameData.weapons.nPrimary == VULCAN_INDEX) || (gameData.weapons.nPrimary == GAUSS_INDEX);

if ((controls [0].firePrimaryState != 0) || (controls [0].firePrimaryDownCount != 0)) {
	if (gameData.weapons.firing [0].nStart <= 0) {
		gameData.weapons.firing [0].nStart = gameStates.app.nSDLTicks [0];
		if (bGatling) {
			if (EGI_FLAG (bGatlingSpeedUp, 1, 0, 0))
				gameData.weapons.firing [0].bSound = 1;
			else {
				gameData.weapons.firing [0].nStart -= GATLING_DELAY + 1;
				gameData.weapons.firing [0].bSound = 0;
				}
			}
		}
	gameData.weapons.firing [0].nDuration = gameStates.app.nSDLTicks [0] - gameData.weapons.firing [0].nStart;
	gameData.weapons.firing [0].nStop = 0;
	}
else if (gameData.weapons.firing [0].nDuration) {
	gameData.weapons.firing [0].nStop = gameStates.app.nSDLTicks [0];
	gameData.weapons.firing [0].nDuration = 
	gameData.weapons.firing [0].nStart = 0;
	}
else if (gameData.weapons.firing [0].nStop > 0) {
	if (gameStates.app.nSDLTicks [0] - gameData.weapons.firing [0].nStop >= GATLING_DELAY /*WIFireTicks (gameData.weapons.nPrimary) * 4 / 5*/) {
		gameData.weapons.firing [0].nStop = 0;
		}
	}
if ((controls [0].fireSecondaryState != 0) || (controls [0].fireSecondaryDownCount != 0)) {
	if (gameData.weapons.firing [1].nStart <= 0)
		gameData.weapons.firing [1].nStart = gameStates.app.nSDLTicks [0];
	gameData.weapons.firing [1].nDuration = gameStates.app.nSDLTicks [0] - gameData.weapons.firing [1].nStart;
	gameData.weapons.firing [1].nStop = 0;
	}
else if (gameData.weapons.firing [1].nDuration) {
	gameData.weapons.firing [1].nStop = gameStates.app.nSDLTicks [0];
	gameData.weapons.firing [1].nDuration = 
	gameData.weapons.firing [1].nStart = 0;
	}
}

//-------------------------------------------------------------------------

void UpdatePlayerWeaponInfo (void)
{
	int				i, bUpdate = 0;
	CWeaponState	*wsP = gameData.multiplayer.weaponStates + N_LOCALPLAYER;
	tFiringData		*fP;

if (gameStates.app.bPlayerIsDead)
	gameData.weapons.firing [0].nStart = 
	gameData.weapons.firing [0].nDuration = 
	gameData.weapons.firing [0].nStop = 
	gameData.weapons.firing [1].nStart = 
	gameData.weapons.firing [1].nDuration =
	gameData.weapons.firing [1].nStop = 0;
else
	UpdateFiringState ();
if (wsP->nPrimary != gameData.weapons.nPrimary) {
	wsP->nPrimary = gameData.weapons.nPrimary;
	bUpdate = 1;
	}
if (wsP->nSecondary != gameData.weapons.nSecondary) {
	wsP->nSecondary = gameData.weapons.nSecondary;
	bUpdate = 1;
	}
if (wsP->bQuadLasers != ((LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS) != 0)) {
	wsP->bQuadLasers = ((LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS) != 0);
	bUpdate = 1;
	}
for (i = 0, fP = wsP->firing; i < 2; i++, fP++) {
	if (fP->nStart != gameData.weapons.firing [i].nStart) {
		fP->nStart = gameData.weapons.firing [i].nStart;
		bUpdate = 1;
		}
	if (fP->nDuration != gameData.weapons.firing [i].nDuration) {
		fP->nDuration = gameData.weapons.firing [i].nDuration;
		bUpdate = 1;
		}
	if (fP->nStop != gameData.weapons.firing [i].nStop) {
		fP->nStop = gameData.weapons.firing [i].nStop;
		bUpdate = 1;
		}
	if (gameData.weapons.firing [i].bSound == 1) {
		fP->bSound = 1;
		gameData.weapons.firing [i].bSound = 0;
		}
	if (fP->bSpeedUp != EGI_FLAG (bGatlingSpeedUp, 1, 0, 0)) {
		fP->bSpeedUp = EGI_FLAG (bGatlingSpeedUp, 1, 0, 0);
		bUpdate = 1;
		}
	}
if (wsP->nMissiles != LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary]) {
	wsP->nMissiles = (char) LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary];
	bUpdate = 1;
	}
if (wsP->nLaserLevel != LOCALPLAYER.LaserLevel ()) {
	wsP->nLaserLevel = LOCALPLAYER.LaserLevel ();
	bUpdate = 1;
	}
if (wsP->bTripleFusion != gameData.weapons.bTripleFusion) {
	wsP->bTripleFusion = gameData.weapons.bTripleFusion;
	bUpdate = 1;
	}
if (wsP->nMslLaunchPos != (gameData.laser.nMissileGun & 3)) {
	wsP->nMslLaunchPos = gameData.laser.nMissileGun & 3;
	bUpdate = 1;
	}
if (wsP->xMslFireTime != gameData.missiles.xNextFireTime) {
	wsP->xMslFireTime = gameData.missiles.xNextFireTime;
	bUpdate = 1;
	}
if (bUpdate)
	MultiSendPlayerWeapons ();	
UpdateFiringSounds ();
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
	CObject	*markerP = markerManager.SpawnObject (-1);

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
	CObject	*markerP = markerManager.SpawnObject (nPlayer);

return markerP ? &markerP->info.position.vPos : &gameData.multiplayer.playerInit [nPlayer].position.vPos;
}

//------------------------------------------------------------------------------

CFixMatrix *PlayerSpawnOrient (int nPlayer)
{
	CObject	*markerP = markerManager.SpawnObject (nPlayer);

return markerP ? &markerP->info.position.mOrient : &gameData.multiplayer.playerInit [nPlayer].position.mOrient;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

int CPlayerData::Index (void)
{
int i = int (this - gameData.multiplayer.players);
return ((i < 0) || (i > MAX_PLAYERS)) ? 0 : i;
}

//-------------------------------------------------------------------------

bool CPlayerData::IsLocalPlayer (void)
{
return Index () == N_LOCALPLAYER;
}

//-------------------------------------------------------------------------

void CPlayerData::UpdateDeathTime (void) 
{
if (m_shield.Get () >= 0) {
	m_tDeath = 0;
	m_bExploded = 0;
	}
else if (!m_tDeath)
	m_tDeath = gameStates.app.nSDLTicks [0];
}

//-------------------------------------------------------------------------

fix CPlayerData::SetShield (fix s, bool bScale) 
{ 
if (m_shield.Set (s, bScale)) {
	if (OBJECTS.Buffer () && (nObject >= 0) && (IsLocalPlayer () || (nObject != LOCALPLAYER.nObject)))
		OBJECTS [nObject].SetShield (s); 
	MultiSendShield ();
	}
UpdateDeathTime ();
return shield;
}

//-------------------------------------------------------------------------

fix CPlayerData::SetEnergy (fix e, bool bScale) 
{ 
m_energy.Set (e, bScale);
return energy;
}

//-------------------------------------------------------------------------

void CPlayerData::SetObject (short n)
{
nObject = n;
}

//-------------------------------------------------------------------------

CObject* CPlayerData::Object (void)
{
return (nObject < 0) ? NULL : OBJECTS + nObject;
}

//-------------------------------------------------------------------------

bool CPlayerData::WaitingForExplosion (void) 
{ 
return m_tDeath && (gameStates.app.nSDLTicks [0] - m_tDeath < 30000) && !m_bExploded && Connected ();  
}

//-------------------------------------------------------------------------

bool CPlayerData::WaitingForWeaponInfo (void) 
{ 
return !m_tWeaponInfo || ((gameStates.app.nSDLTicks [0] - m_tWeaponInfo > 15000)) && (gameStates.app.nSDLTicks [0] - m_tWeaponInfo < 180000)) && !m_bExploded && Connected ();  
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

float CShipEnergy::Scale (void)
{
	ubyte nShip = gameData.multiplayer.weaponStates [m_index].nShip;

return (nShip < MAX_SHIP_TYPES) ? shipModifiers [nShip].a [m_type] : 1.0f;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
