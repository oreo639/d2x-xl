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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "descent.h"
#include "multibot.h"
#include "network.h"
#include "network_lib.h"
#include "error.h"
#include "timer.h"
#include "text.h"
#include "cockpit.h"
#include "dropobject.h"
#include "fireball.h"
#include "physics.h" 
#include "byteswap.h"
#include "segmath.h"

int32_t MultiAddControlledRobot (int32_t nObject, int32_t agitation);
void MultiSendReleaseRobot (int32_t nObject);
CObject* MultiDeleteControlledRobot (int32_t nObject);
void MultiSendRobotPositionSub (int32_t nObject);
void DropStolenItems (CObject*);


//
// Code for controlling robots in multiplayer games
//

#define STANDARD_EXPL_DELAY	(I2X (1)/4)
#define MIN_CONTROL_TIME		(I2X (1))
#define ROBOT_TIMEOUT			(I2X (2))

#define MIN_TO_ADD				60

#define MAX_ROBOT_POWERUPS		((gameStates.multi.nGameType == UDP_GAME) ? 15 : 4)

#define MULTI_ROBOT_PRIORITY(nObject, nPlayer) (((nObject % 4) + nPlayer) % N_PLAYERS)

void MultiSendStolenItems (void);
int32_t MultiPowerupIsAllowed (int32_t);

//-----------------------------------------------------------------------------

#if DBG

CObject* MultiRobot (int32_t nObject, const char* pszFile, const int32_t nLine)
{
CObject *objP = OBJECT (nObject);
if (!objP) {
	if (*pszFile)
		PrintLog (0, "Invalid multiplayer object reference in %s (%d)\n", pszFile, nLine);
	else
		PrintLog (0, "Invalid multiplayer object reference\n");
	return NULL;
	}
if (!objP->IsRobot ()) {
	if (*pszFile)
		PrintLog (0, "Invalid multiplayer object type in %s (%d)\n", pszFile, nLine);
	else
		PrintLog (0, "Invalid multiplayer object type\n");
	return NULL;
	}
return objP;
}

#else

CObject* MultiRobot (int32_t nObject)
{
CObject *objP = OBJECT (nObject);
if (!objP) 
	return NULL;
if (!objP->IsRobot ()) 
	return NULL;
return objP;
}

#endif

//-----------------------------------------------------------------------------
// Determine whether or not I am allowed to move this robot.

int32_t MultiCanControlRobot (int32_t nObject, int32_t agitation)
{
CObject *objP = OBJECT (nObject);
if (!objP)
	return 0;
	// Claim robot if necessary.
if (LOCALPLAYER.m_bExploded)
	return 0;
if (objP->IsBoss ()) {
	int32_t i = gameData.bosses.Find (nObject);
	if ((i >= 0) && gameData.bosses [i].m_nDying == 1)
		return 0;
	return 1;
	}
int32_t nRemOwner = objP->cType.aiInfo.REMOTE_OWNER;
if (nRemOwner == N_LOCALPLAYER) { // Already my robot!
	int32_t nSlot = objP->cType.aiInfo.REMOTE_SLOT_NUM;
   if ((nSlot < 0) || (nSlot >= MAX_ROBOTS_CONTROLLED))
		return 0;
	if (gameData.multigame.robots.fired [nSlot])
		return 0;
	gameData.multigame.robots.agitation [nSlot] = agitation;
	gameData.multigame.robots.lastMsgTime [nSlot] = gameData.time.xGame;
	return 1;
	}
if ((nRemOwner != -1) || (agitation < MIN_TO_ADD)) {
	if (agitation == ROBOT_FIRE_AGITATION) // Special case for firing at non-player
		return 1; // Try to vFire at player even tho we're not in control!
	return 0;
	}
return MultiAddControlledRobot (nObject, agitation);
}

//-----------------------------------------------------------------------------

void MultiCheckRobotTimeout (void)
{
	static fix lastcheck = 0;
	int32_t i, nRemOwner;

if (gameData.time.xGame > lastcheck + I2X (1)) {
	lastcheck = gameData.time.xGame;
	for (i = 0; i < MAX_ROBOTS_CONTROLLED; i++) {
		if ((gameData.multigame.robots.controlled [i] != -1) && 
			 (gameData.multigame.robots.lastSendTime [i] + ROBOT_TIMEOUT < gameData.time.xGame)) {
			CObject* objP = MULTIROBOT (gameData.multigame.robots.controlled [i]);
			if (objP) {
				nRemOwner = objP->cType.aiInfo.REMOTE_OWNER;
				if (nRemOwner != N_LOCALPLAYER) {	
					gameData.multigame.robots.controlled [i] = -1;
					return;
					}
 				if (nRemOwner != N_LOCALPLAYER)
					return;
				if (gameData.multigame.robots.sendPending [i])
					MultiSendRobotPosition (gameData.multigame.robots.controlled [i], 1);
				MultiSendReleaseRobot (gameData.multigame.robots.controlled [i]);
				}
			}
		}
	}		
}

//-----------------------------------------------------------------------------

void MultiStripRobots (int32_t nPlayer)
{
	// Grab all robots away from a player 
	// (player died or exited the game)

	int32_t 	i;
	CObject*	objP;

if (gameData.app.GameMode (GM_MULTI_ROBOTS)) {
	if (nPlayer == N_LOCALPLAYER)
		for (i = 0; i < MAX_ROBOTS_CONTROLLED; i++)
			MultiDeleteControlledRobot (gameData.multigame.robots.controlled [i]);
	FORALL_ROBOT_OBJS (objP)
		if (objP->cType.aiInfo.REMOTE_OWNER == nPlayer) {
			objP->cType.aiInfo.REMOTE_OWNER = -1;
			objP->cType.aiInfo.REMOTE_SLOT_NUM = (nPlayer == N_LOCALPLAYER) ? 4 : 0;
	  		}
	}
// Note -- only call this with playernum == N_LOCALPLAYER if all other players
// already know that we are clearing house.  This does not send a release
// message for each controlled robot!!
}

//-----------------------------------------------------------------------------

void MultiDumpRobots (void)
{
// Dump robot control info for debug purposes
if (!gameData.app.GameMode (GM_MULTI_ROBOTS))
	return;
}

//-----------------------------------------------------------------------------
// Try to add a new robot to the controlled list, return 1 if added, 0 if not.

int32_t MultiAddControlledRobot (int32_t nObject, int32_t agitation)
{
CObject *objP = MULTIROBOT (nObject);
if (!objP)
	return 0;

	int32_t i;
	int32_t lowest_agitation = 0x7fffffff; // MAX POSITIVE INT
	int32_t lowest_agitated_bot = -1;
	int32_t first_freeRobot = -1;

if (ROBOTINFO (objP)->bossFlag) // this is a boss, so make sure he gets a slot
	agitation = (agitation * 3) + N_LOCALPLAYER;  
if (objP->cType.aiInfo.REMOTE_SLOT_NUM > 0) {
	objP->cType.aiInfo.REMOTE_SLOT_NUM -= 1;
	return 0;
	}
for (i = 0; i < MAX_ROBOTS_CONTROLLED; i++) {
	CObject* robotP = MULTIROBOT (gameData.multigame.robots.controlled [i]);
	if (!objP || !objP->IsRobot ()) {
		first_freeRobot = i;
		break;
		}
	if (gameData.multigame.robots.lastMsgTime [i] + ROBOT_TIMEOUT < gameData.time.xGame) {
		if (gameData.multigame.robots.sendPending [i])
			MultiSendRobotPosition (gameData.multigame.robots.controlled [i], 1);
		MultiSendReleaseRobot (gameData.multigame.robots.controlled [i]);
		first_freeRobot = i;
		break;
		}
	if ((gameData.multigame.robots.controlled [i] != -1) && 
		 (gameData.multigame.robots.agitation [i] < lowest_agitation) && 
		 (gameData.multigame.robots.controlledTime [i] + MIN_CONTROL_TIME < gameData.time.xGame)) {
			lowest_agitation = gameData.multigame.robots.agitation [i];
			lowest_agitated_bot = i;
		}
	}
if (first_freeRobot != -1)  // Room left for new robots
	i = first_freeRobot;
else if ((agitation > lowest_agitation)) {// && (lowest_agitation <= MAX_TO_DELETE)) // Replace some old robot with a more agitated one
	if (gameData.multigame.robots.sendPending [lowest_agitated_bot])
		MultiSendRobotPosition (gameData.multigame.robots.controlled [lowest_agitated_bot], 1);
	MultiSendReleaseRobot (gameData.multigame.robots.controlled [lowest_agitated_bot]);
	i = lowest_agitated_bot;
	}
else
	return 0; // Sorry, can't squeeze him in!
MultiSendClaimRobot (nObject);
gameData.multigame.robots.controlled [i] = nObject;
gameData.multigame.robots.agitation [i] = agitation;
objP->cType.aiInfo.REMOTE_OWNER = N_LOCALPLAYER;
objP->cType.aiInfo.REMOTE_SLOT_NUM = i;
gameData.multigame.robots.controlledTime [i] = gameData.time.xGame;
gameData.multigame.robots.lastSendTime [i] = gameData.multigame.robots.lastMsgTime [i] = gameData.time.xGame;
return 1;
}

//-----------------------------------------------------------------------------
// Delete robot CObject number nObject from list of controlled robots because it is dead

CObject* MultiDeleteControlledRobot (int32_t nObject)
{
CObject *objP = MULTIROBOT (nObject);
if (!objP)
	return NULL;

for (int32_t i = 0; i < MAX_ROBOTS_CONTROLLED; i++)
	if (gameData.multigame.robots.controlled [i] == nObject) {
		if (objP->cType.aiInfo.REMOTE_SLOT_NUM != i) 
			return NULL;
		objP->cType.aiInfo.REMOTE_OWNER = -1;
		objP->cType.aiInfo.REMOTE_SLOT_NUM = 0;
		gameData.multigame.robots.controlled [i] = -1;
		gameData.multigame.robots.sendPending [i] = 0;
		gameData.multigame.robots.fired [i] = 0;
		}
return objP;
}

//-----------------------------------------------------------------------------

void MultiSendClaimRobot (int32_t nObject)
{
CObject *objP = MULTIROBOT (nObject);
if (!objP)
	return;
// The AI tells us we should take control of this robot. 
int32_t bufP = 0;

gameData.multigame.msg.buf [bufP++] = (uint8_t) MULTI_ROBOT_CLAIM;
ADD_MSG_ID
gameData.multigame.msg.buf [bufP++] = N_LOCALPLAYER;
int16_t nRemoteObject = GetRemoteObjNum (nObject, reinterpret_cast<int8_t&> (gameData.multigame.msg.buf [bufP + 2]));
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, nRemoteObject);
bufP += 3;
SET_MSG_ID
MultiSendData (gameData.multigame.msg.buf, bufP, 2);
}

//-----------------------------------------------------------------------------

void MultiSendReleaseRobot (int32_t nObject)
{
CObject *objP = MultiDeleteControlledRobot (nObject);
if (!objP)
	return;

int32_t bufP = 0;

gameData.multigame.msg.buf [bufP++] = (uint8_t) MULTI_ROBOT_RELEASE;
ADD_MSG_ID
gameData.multigame.msg.buf [bufP++] = N_LOCALPLAYER;
int16_t nRemoteObj = GetRemoteObjNum (nObject, reinterpret_cast<int8_t&> (gameData.multigame.msg.buf [bufP + 2]));
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, nRemoteObj);
bufP += 3;
SET_MSG_ID
MultiSendData (gameData.multigame.msg.buf, bufP, 2);
}

//-----------------------------------------------------------------------------

int32_t MultiSendRobotFrame (int32_t sent)
{
	static int32_t last_sent = 0;

	int32_t i, sending;
	int32_t rval = 0;

for (i = 0; i < MAX_ROBOTS_CONTROLLED; i++) {
	sending = (last_sent + 1 + i) % MAX_ROBOTS_CONTROLLED;
	if ((gameData.multigame.robots.controlled [sending] != -1) && 
		 ((gameData.multigame.robots.sendPending [sending] > sent) || (gameData.multigame.robots.fired [sending] > sent))) {
		if (gameData.multigame.robots.sendPending [sending]) {
			gameData.multigame.robots.sendPending [sending] = 0;
			MultiSendRobotPositionSub (gameData.multigame.robots.controlled [sending]);
			}
		if (gameData.multigame.robots.fired [sending]) {
			gameData.multigame.robots.fired [sending] = 0;
			MultiSendData (reinterpret_cast<uint8_t*> (gameData.multigame.robots.fireBuf [sending]), 18, 1);
			}
		if (!IsNetworkGame)
			sent += 1;
		last_sent = sending;
		rval++;
		}
	}
Assert ((last_sent >= 0) && (last_sent <= MAX_ROBOTS_CONTROLLED));
return rval;
}

//-----------------------------------------------------------------------------

void MultiSendRobotPositionSub (int32_t nObject)
{
CObject *objP = MULTIROBOT (nObject);
if (!objP)
	return;

	int32_t bufP = 0;
	int16_t s;
#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)
	tShortPos sp;
#endif

gameData.multigame.msg.buf [bufP++] = MULTI_ROBOT_POSITION;  							
gameData.multigame.msg.buf [bufP++] = N_LOCALPLAYER;										
s = GetRemoteObjNum (nObject, reinterpret_cast<int8_t&> (gameData.multigame.msg.buf [bufP + 2]));
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, s);
bufP += 3;
#if ! (defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__))
CreateShortPos (reinterpret_cast<tShortPos*> (gameData.multigame.msg.buf + bufP), objP, 0);	
bufP += sizeof (tShortPos);
#else
CreateShortPos (&sp, objP, 1);
memcpy (gameData.multigame.msg.buf + bufP, reinterpret_cast<uint8_t*> (sp.orient), 9);
bufP += 9;
memcpy (gameData.multigame.msg.buf + bufP, reinterpret_cast<uint8_t*> (&sp.coord), 14);
bufP += 14;
#endif
MultiSendData (reinterpret_cast<uint8_t*> (gameData.multigame.msg.buf), bufP, 1);
}

//-----------------------------------------------------------------------------
// Send robot position to other player (s).  Includes a byte
// value describing whether or not they fired a weapon

void MultiSendRobotPosition (int32_t nObject, int32_t bForce)
{
	int32_t	i;

if (! IsMultiGame)
	return;
CObject *objP = MULTIROBOT (nObject);
if (!objP)
	return;
if (objP->cType.aiInfo.REMOTE_OWNER != N_LOCALPLAYER)
	return;
i = objP->cType.aiInfo.REMOTE_SLOT_NUM;
gameData.multigame.robots.lastSendTime [i] = gameData.time.xGame;
gameData.multigame.robots.sendPending [i] = 1+bForce;
if (bForce & IsNetworkGame)
	NetworkFlushData ();
return;
}

//-----------------------------------------------------------------------------

void MultiSendRobotFire (int32_t nObject, int32_t nGun, CFixVector *vFire)
{
CObject *objP = MULTIROBOT (nObject);
if (!objP)
	return;
	// Send robot vFire event
	int32_t bufP = 0;
	int16_t s;
#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)
	CFixVector vSwapped;
#endif

gameData.multigame.msg.buf [bufP++] = MULTI_ROBOT_FIRE;					
gameData.multigame.msg.buf [bufP++] = N_LOCALPLAYER;							
s = GetRemoteObjNum (nObject, reinterpret_cast<int8_t&> (gameData.multigame.msg.buf [bufP + 2]));
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, s);
bufP += 3;
gameData.multigame.msg.buf [bufP++] = nGun;								
#if ! (defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__))
memcpy (gameData.multigame.msg.buf + bufP, vFire, sizeof (CFixVector));         
bufP += sizeof (CFixVector); // 12
// --------------------------
//      Total = 18
#else
vSwapped.dir.coord.x = (fix) INTEL_INT ((int32_t) (*vFire).dir.coord.x);
vSwapped.dir.coord.y = (fix) INTEL_INT ((int32_t) (*vFire).dir.coord.y);
vSwapped.dir.coord.z = (fix) INTEL_INT ((int32_t) (*vFire).dir.coord.z);
memcpy (gameData.multigame.msg.buf + bufP, &vSwapped, sizeof (CFixVector)); 
bufP += sizeof (CFixVector);
#endif
if (objP->cType.aiInfo.REMOTE_OWNER == N_LOCALPLAYER) {
	int32_t slot = objP->cType.aiInfo.REMOTE_SLOT_NUM;
	if ((slot < 0) || (slot >= MAX_ROBOTS_CONTROLLED))
		return;
	if (gameData.multigame.robots.fired [slot] != 0)
		return;
	memcpy (gameData.multigame.robots.fireBuf [slot], gameData.multigame.msg.buf, bufP);
	gameData.multigame.robots.fired [slot] = 1;
	if (IsNetworkGame)
		NetworkFlushData ();
	}
else
	MultiSendData (gameData.multigame.msg.buf, bufP, 2); // Not our robot, send ASAP
}

//-----------------------------------------------------------------------------

void MultiSendRobotExplode (int32_t nObject, int32_t nKiller, char bIsThief)
{
	// Send robot explosion event to the other players

	int32_t	bufP = 0;
	int16_t nRemoteObj;

gameData.multigame.msg.buf [bufP++] = MULTI_ROBOT_EXPLODE;				
ADD_MSG_ID
gameData.multigame.msg.buf [bufP++] = N_LOCALPLAYER;							
nRemoteObj = int16_t (GetRemoteObjNum (nKiller, reinterpret_cast<int8_t&> (gameData.multigame.msg.buf [bufP + 2])));
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, nRemoteObj);                       
bufP += 3;
nRemoteObj = int16_t (GetRemoteObjNum (nObject, reinterpret_cast<int8_t&> (gameData.multigame.msg.buf [bufP + 2])));
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, nRemoteObj);                       
bufP += 3;
gameData.multigame.msg.buf [bufP++] = bIsThief;   
SET_MSG_ID
MultiSendData (gameData.multigame.msg.buf, bufP, 1);
MultiDeleteControlledRobot (nObject);
}

//-----------------------------------------------------------------------------

void MultiSendCreateRobot (int32_t station, int32_t nObject, int32_t nType)
{
	// Send create robot information

	int32_t bufP = 0;

gameData.multigame.msg.buf [bufP++] = MULTI_CREATE_ROBOT;					
ADD_MSG_ID
gameData.multigame.msg.buf [bufP++] = N_LOCALPLAYER;	
gameData.multigame.msg.buf [bufP++] = (int8_t) station;                         
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, nObject);                  
bufP += 2;
gameData.multigame.msg.buf [bufP++] = nType;								
SetLocalObjNumMapping (int16_t (nObject));
SET_MSG_ID
MultiSendData (gameData.multigame.msg.buf, bufP, 2);
}

//-----------------------------------------------------------------------------

void MultiSendBossActions (int32_t nBossObj, int32_t action, int32_t secondary, int32_t nObject)
{
CObject *objP = MULTIROBOT (nObject);
if (!objP)
	return;
	// Send special boss behavior information

	int32_t bufP = 0;

gameData.multigame.msg.buf [bufP++] = MULTI_BOSS_ACTIONS;					
gameData.multigame.msg.buf [bufP++] = N_LOCALPLAYER;
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, nBossObj);  // Which player is controlling the boss        
bufP += 2; // We won't network map this nObject since it's the boss
gameData.multigame.msg.buf [bufP++] = (int8_t)action;  // What is the boss doing?
gameData.multigame.msg.buf [bufP++] = (int8_t)secondary;  // More info for what he is doing
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, nObject);                  
bufP += 2; // Objnum of CObject created by gate-in action
if (action == 3) {
	PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, objP->info.nSegment); 
	bufP += 2; // Segment number CObject created in (for gate only)
	}
else 
	bufP += 2; // Dummy

if (action == 1) { // Teleport releases robot
	// Boss is up for grabs after teleporting
	MultiDeleteControlledRobot (nBossObj);
	}
MultiSendData (gameData.multigame.msg.buf, bufP, 1);
}
		
//-----------------------------------------------------------------------------
// Send create robot information

void MultiSendCreateRobotPowerups (CObject *delObjP)
{
	int32_t	bufP = 0, hBufP;
#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)
	CFixVector vSwapped;
#endif

gameData.multigame.msg.buf [bufP++] = MULTI_CREATE_ROBOT_POWERUPS;			
ADD_MSG_ID
gameData.multigame.msg.buf [bufP++] = N_LOCALPLAYER;			
hBufP = bufP++; // points to the buffer location where the number of powerups contained in the data packet is stored
gameData.multigame.msg.buf [bufP++] = delObjP->info.contains.nType; 				
gameData.multigame.msg.buf [bufP++] = delObjP->info.contains.nId;					
PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, delObjP->info.nSegment);
bufP += 2;
#if !(defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__))
memcpy (gameData.multigame.msg.buf + bufP, &delObjP->info.position.vPos, sizeof (CFixVector));
#else
vSwapped.dir.coord.x = (fix)INTEL_INT ((int32_t) delObjP->info.position.vPos.dir.coord.x);
vSwapped.dir.coord.y = (fix)INTEL_INT ((int32_t) delObjP->info.position.vPos.dir.coord.y);
vSwapped.dir.coord.z = (fix)INTEL_INT ((int32_t) delObjP->info.position.vPos.dir.coord.z);
memcpy (gameData.multigame.msg.buf + bufP, &vSwapped, sizeof (CFixVector));     
#endif
bufP += sizeof (CFixVector);
if (gameStates.multi.nGameType == UDP_GAME) {
	int8_t nOwner;
	int32_t nRemoteObj = GetRemoteObjNum (delObjP->Index (), nOwner);
	PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP, nRemoteObj);
	bufP += 2;
	PUT_INTEL_INT (gameData.multigame.msg.buf + bufP, gameStates.app.nRandSeed);
	bufP += 4;
	}

// successively send all robot powerups just created (their count is in gameData.multigame.create.nCount)
while (gameData.multigame.create.nCount > MAX_ROBOT_POWERUPS) {
	CObject *objP = OBJECT (gameData.multigame.create.nObjNums [--gameData.multigame.create.nCount]);
	if (objP)
		objP->Die ();
	}
gameData.multigame.msg.buf [hBufP] = (uint8_t) delObjP->info.contains.nCount;

int32_t i;
for (i = 0; i < gameData.multigame.create.nCount; i++) {
	PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP + 2 * i, gameData.multigame.create.nObjNums [i]); // bufP must always point to the start of the object data list here!
	SetLocalObjNumMapping (gameData.multigame.create.nObjNums [i]);
	}
for (; i < MAX_ROBOT_POWERUPS; i++) 
	PUT_INTEL_SHORT (gameData.multigame.msg.buf + bufP + 2 * i, -1); // bufP must always point to the start of the object data list here!
bufP += 2 * MAX_ROBOT_POWERUPS;
SET_MSG_ID
MultiSendData (gameData.multigame.msg.buf, bufP, 2);
gameData.multigame.create.nCount = 0;
}

//-----------------------------------------------------------------------------

void MultiDoClaimRobot (uint8_t* buf)
{
	int32_t bufP = 1;

CHECK_MSG_ID

uint8_t nPlayer = buf [bufP++];
int16_t nRemoteBot = GET_INTEL_SHORT (buf + bufP);
bufP += 2;
int16_t nRobot = GetLocalObjNum (nRemoteBot, (int8_t)buf [bufP]);

CObject* objP = MULTIROBOT (nRobot);
if (!objP)
	return;
if (objP->cType.aiInfo.REMOTE_OWNER != -1)
	if (MULTI_ROBOT_PRIORITY (nRemoteBot, nPlayer) <= MULTI_ROBOT_PRIORITY (nRemoteBot, objP->cType.aiInfo.REMOTE_OWNER))
		return;
// Perform the requested change
if (objP->cType.aiInfo.REMOTE_OWNER == N_LOCALPLAYER)
	MultiDeleteControlledRobot (nRobot);
objP->cType.aiInfo.REMOTE_OWNER = nPlayer;
objP->cType.aiInfo.REMOTE_SLOT_NUM = 0;
}

//-----------------------------------------------------------------------------

void MultiDoReleaseRobot (uint8_t* buf)
{
	int32_t bufP = 1;

CHECK_MSG_ID

uint8_t nPlayer = buf [bufP++];
int16_t nRemoteBot = GET_INTEL_SHORT (buf + bufP);
bufP += 2;
int16_t nRobot = GetLocalObjNum (nRemoteBot, (int8_t)buf [bufP]);
CObject* objP = MULTIROBOT (nRobot);
if (!objP)
	return;
if (objP->cType.aiInfo.REMOTE_OWNER != nPlayer)
	return;
// Perform the requested change
objP->cType.aiInfo.REMOTE_OWNER = -1;
objP->cType.aiInfo.REMOTE_SLOT_NUM = 0;
}

//-----------------------------------------------------------------------------
// Process robot movement sent by another player

void MultiDoRobotPosition (uint8_t* buf)
{
#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)
	tShortPos sp;
#endif
	int16_t nRobot, nRemoteBot;
	int32_t bufP = 1;
	uint8_t nPlayer = buf [bufP++];

nRemoteBot = GET_INTEL_SHORT (buf + bufP);
nRobot = GetLocalObjNum (nRemoteBot, (int8_t)buf [bufP+2]); 
bufP += 3;
CObject* objP = MULTIROBOT (nRobot);
if (!objP)
	return;
if ((objP->info.nType != OBJ_ROBOT) || (objP->info.nFlags & OF_EXPLODING)) 
	return;
if (objP->cType.aiInfo.REMOTE_OWNER != nPlayer) {
	if (objP->cType.aiInfo.REMOTE_OWNER != -1)
		return;
		// Robot claim packet must have gotten lost, let this player claim it.
	else if (objP->cType.aiInfo.REMOTE_SLOT_NUM > 3) {
		objP->cType.aiInfo.REMOTE_OWNER = nPlayer;
		objP->cType.aiInfo.REMOTE_SLOT_NUM = 0;
		}
	else
		objP->cType.aiInfo.REMOTE_SLOT_NUM++;
	}
objP->SetThrustFromVelocity (); // Try to smooth out movement
#if ! (defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__))
ExtractShortPos (objP, reinterpret_cast<tShortPos*> (buf+bufP), 0);
#else
memcpy (reinterpret_cast<uint8_t*>(sp.orient), reinterpret_cast<uint8_t*>(buf + bufP), 9);	
bufP += 9;
memcpy (reinterpret_cast<uint8_t*> (&(sp.coord)), reinterpret_cast<uint8_t*>(buf + bufP), 14);
ExtractShortPos (objP, &sp, 1);
#endif
}

//-----------------------------------------------------------------------------

void MultiDoRobotFire (uint8_t* buf)
{
	// Send robot vFire event
	int32_t		bufP = 2;
	int16_t		nRobot;
	int16_t		nRemoteBot;
	int32_t		nGun;
	CFixVector	vFire, vGunPoint;
	tRobotInfo	*robotP;

nRemoteBot = GET_INTEL_SHORT (buf + bufP);
nRobot = GetLocalObjNum (nRemoteBot, (int8_t)buf [bufP+2]); 
CObject* objP = MULTIROBOT (nRobot);
if (!objP)
	return;

bufP += 3;
nGun = (int8_t)buf [bufP++];                                      
memcpy (&vFire, buf+bufP, sizeof (CFixVector));
vFire.v.coord.x = (fix)INTEL_INT ((int32_t)vFire.v.coord.x);
vFire.v.coord.y = (fix)INTEL_INT ((int32_t)vFire.v.coord.y);
vFire.v.coord.z = (fix)INTEL_INT ((int32_t)vFire.v.coord.z);
if ((nRobot < 0) || (nRobot > gameData.objData.nLastObject [0]) || 
	 (objP->info.nType != OBJ_ROBOT) || 
	 (objP->info.nFlags & OF_EXPLODING))
	return;
// Do the firing
if (nGun == -1 || nGun==-2)
	// Drop proximity bombs
	vGunPoint = objP->info.position.vPos + vFire;
else 
	CalcGunPoint (&vGunPoint, objP, nGun);
robotP = ROBOTINFO (objP);
if (nGun == -1) 
	CreateNewWeaponSimple (&vFire, &vGunPoint, nRobot, PROXMINE_ID, 1);
else if (nGun == -2)
	CreateNewWeaponSimple (&vFire, &vGunPoint, nRobot, SMARTMINE_ID, 1);
else
	CreateNewWeaponSimple (&vFire, &vGunPoint, nRobot, robotP->nWeaponType, 1);
}

//-----------------------------------------------------------------------------

int32_t MultiDestroyRobot (CObject* robotP, char bIsThief)
{
	int16_t	nRobot = OBJ_IDX (robotP);

// Drop non-Random KEY powerups locally only!
if (IsCoopGame && (robotP->info.contains.nCount > 0) && (robotP->info.contains.nType == OBJ_POWERUP) && 
	 (robotP->info.contains.nId >= POW_KEY_BLUE) && (robotP->info.contains.nId <= POW_KEY_GOLD))
	robotP->CreateEgg ();
/*else*/ 
if ((robotP->cType.aiInfo.REMOTE_OWNER == -1) && IAmGameHost ()) 
	robotP->cType.aiInfo.REMOTE_OWNER = N_LOCALPLAYER;
if (robotP->cType.aiInfo.REMOTE_OWNER == N_LOCALPLAYER) {
	MultiDropRobotPowerups (robotP->Index ());
	robotP->cType.aiInfo.REMOTE_OWNER = -1;
	MultiDeleteControlledRobot (nRobot);
	}

tRobotInfo* botInfoP = ROBOTINFO (robotP);
if (bIsThief || botInfoP->thief)
	DropStolenItems (robotP);
if (botInfoP->bossFlag) {
	int32_t i = gameData.bosses.Find (robotP->Index ());
	if ((i >= 0) && gameData.bosses [i].m_nDying)
		return 0;
	StartBossDeathSequence (robotP);
	}
else if (botInfoP->bDeathRoll)
	StartRobotDeathSequence (robotP);
else {
	if (robotP->info.nId == SPECIAL_REACTOR_ROBOT)
		SpecialReactorStuff ();
	if (botInfoP->kamikaze)
		robotP->Explode (1);	//	Kamikaze, explode right away, IN YOUR FACE!
	else
		robotP->Explode (STANDARD_EXPL_DELAY);
   }
return 1;
}

//-----------------------------------------------------------------------------

CObject* MultiExplodeRobot (int32_t nRobot, int32_t nKiller, char bIsThief)
{
CObject* objP = MULTIROBOT (nRobot);
if (!objP)
	return NULL;
if (objP->info.nFlags & OF_EXPLODING) // Object already exploding
	return NULL;
if (objP->cType.aiInfo.xDyingStartTime > 0)	// death sequence initiated
	return NULL;
// Data seems valid, explode the sucker
NetworkResetObjSync (nRobot);
return MultiDestroyRobot (objP, bIsThief) ? objP : NULL;
}

//-----------------------------------------------------------------------------

void MultiDoRobotExplode (uint8_t* buf)
{
	// Explode robot controlled by other player
	int32_t bufP = 1;

CHECK_MSG_ID

bufP++; // skip player id
int16_t nRemoteKiller = GET_INTEL_SHORT (buf + bufP);
int16_t nKiller = GetLocalObjNum (nRemoteKiller, int32_t (buf [bufP+2])); 
bufP += 3;
int16_t nRemoteBot = GET_INTEL_SHORT (buf + bufP);
int32_t nRobot = GetLocalObjNum (nRemoteBot, int32_t (buf [bufP+2])); 
bufP += 3;
CObject* objP = MultiExplodeRobot (nRobot, nKiller, buf [bufP]);
if (!objP)
	return;
if (nKiller == LOCALPLAYER.nObject)
	cockpit->AddPointsToScore (ROBOTINFO (objP)->scoreValue);
}

//-----------------------------------------------------------------------------

void MultiDoCreateRobot (uint8_t* buf)
{
	tProducerInfo*	robotGenP;
	CFixVector		vObjPos, direction;
	CObject*			objP;
	int32_t			nPlayer;
	int32_t			nProducer;
	int16_t			nRemoteObj;
	uint8_t			nType;
	int32_t			bufP = 1;

CHECK_MSG_ID

nPlayer = buf [bufP++];
nProducer = buf [bufP++];
nRemoteObj = GET_INTEL_SHORT (buf + bufP);
bufP += 2;
nType = buf [bufP];
if ((nPlayer < 0) || (nRemoteObj < 0) || (nProducer < 0) || (nProducer >= gameData.producers.nProducers) || (nPlayer >= N_PLAYERS)) 
	return;
robotGenP = gameData.producers.producers + nProducer;
// Play effect and sound
vObjPos = SEGMENT (robotGenP->nSegment)->Center ();
objP = CreateExplosion ((int16_t) robotGenP->nSegment, vObjPos, I2X (10), ANIM_MORPHING_ROBOT);
if (objP)
	ExtractOrientFromSegment (&objP->info.position.mOrient, SEGMENT (robotGenP->nSegment));
if (gameData.effects.animations [0][ANIM_MORPHING_ROBOT].nSound > -1)
	audio.CreateSegmentSound (gameData.effects.animations [0][ANIM_MORPHING_ROBOT].nSound, (int16_t) robotGenP->nSegment, 0, vObjPos, 0, I2X (1));
// Set robot center flags, in case we become the master for the next one
robotGenP->bFlag = 0;
robotGenP->xCapacity -= gameData.producers.xEnergyToCreateOneRobot;
robotGenP->xTimer = 0;
if (! (objP = CreateMorphRobot (SEGMENT (robotGenP->nSegment), &vObjPos, nType)))
	return; // Cannot create CObject!
objP->info.nCreator = ((int16_t) (robotGenP - gameData.producers.producers.Buffer ())) | 0x80;
//	ExtractOrientFromSegment (&objP->info.position.mOrient, &SEGMENT (robotGenP->nSegment));
direction = gameData.objData.consoleP->info.position.vPos - objP->info.position.vPos;
objP->info.position.mOrient = CFixMatrix::CreateFU(direction, objP->info.position.mOrient.m.dir.u);
//objP->info.position.mOrient = CFixMatrix::CreateFU(direction, &objP->info.position.mOrient.m.v.u, NULL);
objP->MorphStart ();
SetObjNumMapping (objP->Index (), nRemoteObj, nPlayer);
Assert (objP->cType.aiInfo.REMOTE_OWNER == -1);
}

//-----------------------------------------------------------------------------

void MultiDoBossActions (uint8_t* buf)
{
	// Code to handle remote-controlled boss actions

	CObject	*bossObjP;
	int16_t		nBossObj, nBossIdx;
	int32_t		nPlayer;
	int32_t		action;
	int16_t		secondary;
	int16_t		nRemoteObj, nSegment;
	int32_t		bufP = 1;

nPlayer = buf [bufP++];
nBossObj = GET_INTEL_SHORT (buf + bufP);           
bufP += 2;
action = buf [bufP++];
secondary = buf [bufP++];
nRemoteObj = GET_INTEL_SHORT (buf + bufP);         
bufP += 2;
nSegment = GET_INTEL_SHORT (buf + bufP);                
bufP += 2;
bossObjP = OBJECT (nBossObj);
if (!bossObjP)
	return;
nBossIdx = gameData.bosses.Find (nBossObj);
if (nBossIdx < 0)
	return;
if (!bossObjP->IsBoss ())
	return;
switch (action)  {
	case 1: // Teleport
		{
		int16_t nTeleportSeg;

		CFixVector vBossDir;
		if ((secondary < 0) || (secondary > gameData.bosses [nBossIdx].m_nTeleportSegs)) 
			return;
		nTeleportSeg = gameData.bosses [nBossIdx].m_teleportSegs[secondary];
		if ((nTeleportSeg < 0) || (nTeleportSeg > gameData.segData.nLastSegment)) 
			return;
		bossObjP->info.position.vPos = SEGMENT (nTeleportSeg)->Center ();
		bossObjP->RelinkToSeg (nTeleportSeg);
		gameData.bosses [nBossIdx].m_nLastTeleportTime = gameData.time.xGame;
		vBossDir = PLAYEROBJECT (nPlayer)->info.position.vPos - bossObjP->info.position.vPos;
		bossObjP->info.position.mOrient = CFixMatrix::CreateF(vBossDir);

		audio.CreateSegmentSound (gameData.effects.animations [0][ANIM_MORPHING_ROBOT].nSound, nTeleportSeg, 0, bossObjP->info.position.vPos, 0, I2X (1));
		audio.DestroyObjectSound (OBJ_IDX (bossObjP));
		audio.CreateObjectSound (SOUND_BOSS_SHARE_SEE, SOUNDCLASS_ROBOT, OBJ_IDX (bossObjP), 1, I2X (1), I2X (512));	//	I2X (5)12 means play twice as loud
		gameData.ai.localInfo [OBJ_IDX (bossObjP)].nextPrimaryFire = 0;
		if (bossObjP->cType.aiInfo.REMOTE_OWNER == N_LOCALPLAYER) {
			MultiDeleteControlledRobot (nBossObj);
//			gameData.multigame.robots.controlled [bossObjP->cType.aiInfo.REMOTE_SLOT_NUM] = -1;
			}
		bossObjP->cType.aiInfo.REMOTE_OWNER = -1; // Boss is up for grabs again!
		bossObjP->cType.aiInfo.REMOTE_SLOT_NUM = 0; // Available immediately!
		}
		break;

	case 2: // Cloak
		gameData.bosses [nBossIdx].m_nHitTime = -I2X (10);
		gameData.bosses [nBossIdx].m_nCloakStartTime = gameData.time.xGame;
		gameData.bosses [nBossIdx].m_nCloakEndTime = gameData.time.xGame + gameData.bosses [nBossIdx].m_nCloakDuration;
		bossObjP->cType.aiInfo.CLOAKED = 1;
		break;

	case 3: // Gate in robots!
		// Do some validity checking
		if ((nRemoteObj >= LEVEL_OBJECTS) || (nRemoteObj < 0) || (nSegment < 0) || (nSegment > gameData.segData.nLastSegment)) 
			return;
		// Gate one in!
		if (GateInRobot (nBossObj, (uint8_t) secondary, nSegment))
			SetObjNumMapping (gameData.multigame.create.nObjNums [0], nRemoteObj, nPlayer);
		break;

	case 4: // Start effect
		RestartEffect (BOSS_ECLIP_NUM);
		break;

	case 5:	// Stop effect
		StopEffect (BOSS_ECLIP_NUM);
		break;

	default:
		BRP; // Illegal nType to boss actions
	}
}

//-----------------------------------------------------------------------------
// Code to drop remote-controlled robot powerups

void MultiDoCreateRobotPowerups (uint8_t* buf)
{
	CObject	delObjP;
	int32_t	nPlayer, nEggObj, i;
	int16_t	s;
	int32_t	bufP = 1;

CHECK_MSG_ID

nPlayer = buf [bufP++];		
delObjP.info.nType = OBJ_ROBOT;
delObjP.info.contains.nCount = buf [bufP++];					
delObjP.info.contains.nType = buf [bufP++];					
delObjP.info.contains.nId = buf [bufP++]; 					
delObjP.info.nSegment = GET_INTEL_SHORT (buf + bufP);            
bufP += 2;
memcpy (&delObjP.info.position.vPos, buf + bufP, sizeof (CFixVector));      
bufP += sizeof (CFixVector);
delObjP.mType.physInfo.velocity.SetZero ();
delObjP.info.position.vPos.v.coord.x = (fix) INTEL_INT ((int32_t) delObjP.info.position.vPos.v.coord.x);
delObjP.info.position.vPos.v.coord.y = (fix) INTEL_INT ((int32_t) delObjP.info.position.vPos.v.coord.y);
delObjP.info.position.vPos.v.coord.z = (fix) INTEL_INT ((int32_t) delObjP.info.position.vPos.v.coord.z);
if (gameStates.multi.nGameType != UDP_GAME)
	gameStates.app.nRandSeed = 1245L;
else {
	// Check whether that robot is still alive on this client (-> out of sync!) and destroy it if it is to re-sync
	// GetRemoteObjNum will return -1 if the corresponding local object does not exist
	// MultiExplodeRobot will handle the cases of the robot not existing or still existing but already about to being destroyed
	int8_t nOwner;
	MultiExplodeRobot (GetRemoteObjNum (GET_INTEL_SHORT (buf + bufP), nOwner), -1, 0); 
	bufP += 2;
	gameStates.app.nRandSeed = (uint32_t) GET_INTEL_INT (buf + bufP);
	bufP += 4;
	}
gameStates.app.SRand (gameStates.app.nRandSeed);
gameData.multigame.create.nCount = 0;
nEggObj = delObjP.CreateEgg ();
if (nEggObj == -1)
	return; // Object buffer full
#if DBG
if (gameData.multigame.create.nCount > MAX_ROBOT_POWERUPS)
	BRP;
#endif
while (gameData.multigame.create.nCount > MAX_ROBOT_POWERUPS) {
	CObject* objP = OBJECT (gameData.multigame.create.nObjNums [--gameData.multigame.create.nCount]);
	if (objP)
		objP->Die ();
	}
for (i = 0; i < gameData.multigame.create.nCount; i++) {
	CObject* objP = OBJECT (gameData.multigame.create.nObjNums [i]);
	if (objP) {
		s = GET_INTEL_SHORT (buf + bufP);
		if (s != -1)
			SetObjNumMapping ((int16_t)gameData.multigame.create.nObjNums [i], s, nPlayer);
		else
			objP->Die (); // Delete OBJECTS other guy didn't create one of
		}
	bufP += 2;
	}
}

//-----------------------------------------------------------------------------
// Code to handle dropped robot powerups in network mode ONLY!

void MultiDropRobotPowerups (int32_t nObject)
{
	CObject		*delObjP;
	int32_t		nEggObj = -1;
	tRobotInfo	*botInfoP; 

delObjP = MULTIROBOT (nObject);
if (!delObjP)
	return;
botInfoP = ROBOTINFO (delObjP);
if (!botInfoP)
	return;
gameData.multigame.create.nCount = 0;

if (delObjP->info.contains.nCount > 0) { 
	if (delObjP->info.contains.nCount > MAX_ROBOT_POWERUPS)
		delObjP->info.contains.nCount = MAX_ROBOT_POWERUPS;
	//	If dropping a weapon that the player has, drop energy instead, unless it's vulcan, in which case drop vulcan ammo.
	if (delObjP->info.contains.nType == OBJ_POWERUP) {
		MaybeReplacePowerupWithEnergy (delObjP);
		if (!MultiPowerupIsAllowed (delObjP->info.contains.nId))
			delObjP->info.contains.nId = POW_SHIELD_BOOST;
		// No key drops in non-coop games!
		if (!IsCoopGame) {
			if ((delObjP->info.contains.nId >= POW_KEY_BLUE) && (delObjP->info.contains.nId <= POW_KEY_GOLD))
				delObjP->info.contains.nCount = 0;
			}
		}
	if (gameStates.multi.nGameType != UDP_GAME)
		gameStates.app.SRand (1245L);
	else
		gameStates.app.SRand ();
	if (delObjP->info.contains.nCount > 0)
		nEggObj = delObjP->CreateEgg ();
	}
else if (delObjP->cType.aiInfo.REMOTE_OWNER == -1) // No Random goodies for robots we weren't in control of
	return;
else if (botInfoP->containsCount) {
	if (gameStates.multi.nGameType != UDP_GAME)
		gameStates.app.SRand ();
	if (Rand (15) + 1 <= botInfoP->containsProb) {
		delObjP->info.contains.nCount = Rand (botInfoP->containsCount) + 1; // create at least one
		if (delObjP->info.contains.nCount > MAX_ROBOT_POWERUPS)
			delObjP->info.contains.nCount = MAX_ROBOT_POWERUPS;
		delObjP->info.contains.nType = botInfoP->containsType;
		delObjP->info.contains.nId = botInfoP->containsId;
		if (delObjP->info.contains.nType == OBJ_POWERUP) {
			MaybeReplacePowerupWithEnergy (delObjP);
			if (!MultiPowerupIsAllowed (delObjP->info.contains.nId))
				delObjP->info.contains.nId = POW_SHIELD_BOOST;
			 }
		if (gameStates.multi.nGameType != UDP_GAME)
			gameStates.app.SRand (1245L);
		else
			gameStates.app.SRand ();
		if (delObjP->info.contains.nCount > 0)
			nEggObj = delObjP->CreateEgg (false, true);
		}
	}
if (nEggObj >= 0) // Transmit the object creation to the other players	 
	MultiSendCreateRobotPowerups (delObjP);
}

//	-----------------------------------------------------------------------------
//	Robot *robot got whacked by player player_num and requests permission to do something about it.
//	Note: This function will be called regardless of whether gameData.app.nGameMode is a multiplayer mode, so it
//	should quick-out if not in a multiplayer mode.  On the other hand, it only gets called when a
//	player or player weapon whacks a robot, so it happens rarely.
void MultiRobotRequestChange (CObject *robot, int32_t player_num)
{
	int32_t	slot, nRemoteObj;
	int8_t dummy;

if (!gameData.app.GameMode (GM_MULTI_ROBOTS))
	return;
slot = robot->cType.aiInfo.REMOTE_SLOT_NUM;
if ((slot < 0) || (slot >= MAX_ROBOTS_CONTROLLED))
	return;
nRemoteObj = GetRemoteObjNum (OBJ_IDX (robot), dummy);
if (nRemoteObj < 0)
	return;
if ((gameData.multigame.robots.agitation [slot] < 70) || 
	 (MULTI_ROBOT_PRIORITY (nRemoteObj, player_num) > 
	  MULTI_ROBOT_PRIORITY (nRemoteObj, N_LOCALPLAYER)) || 
	  (RandShort () > 0x4400)) {
	if (gameData.multigame.robots.sendPending [slot])
		MultiSendRobotPosition (gameData.multigame.robots.controlled [slot], -1);
	MultiSendReleaseRobot (gameData.multigame.robots.controlled [slot]);
	robot->cType.aiInfo.REMOTE_SLOT_NUM = 5;  // Hands-off period
	}
}

//-----------------------------------------------------------------------------
//eof
