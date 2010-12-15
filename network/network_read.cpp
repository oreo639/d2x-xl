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
#	include <conf.h>
#endif

#ifndef _WIN32
#	include <arpa/inet.h>
#	include <netinet/in.h> /* for htons & co. */
#endif

#include "descent.h"
#include "byteswap.h"
#include "error.h"
#include "timer.h"
#include "strutil.h"
#include "ipx.h"
#include "network.h"
#include "network_lib.h"
#include "netmisc.h"
#include "text.h"
#include "newdemo.h"
#include "segmath.h"
#include "objeffects.h"
#include "physics.h"

//------------------------------------------------------------------------------

void NetworkReadEndLevelPacket (ubyte *dataP)
{
	// Special packet for end of level syncing
	int				nPlayer;
	CEndLevelInfo	eli (reinterpret_cast<tEndLevelInfo*> (dataP));

#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)
	int i, j;

for (i = 0; i < MAX_NUM_NET_PLAYERS; i++)
	for (j = 0; j < MAX_NUM_NET_PLAYERS; j++)
		*eli.ScoreMatrix (i, j) = INTEL_SHORT (*eli.ScoreMatrix (i, j));
*eli.Kills () = INTEL_SHORT (*eli.Kills ());
*eli.Killed () = INTEL_SHORT (*eli.Killed ());
#endif
nPlayer = *eli.Player ();
Assert (nPlayer != gameData.multiplayer.nLocalPlayer);
if (nPlayer >= gameData.multiplayer.nPlayers) {
	Int3 (); // weird, but it an happen in a coop restore game
	return; // if it happens in a coop restore, don't worry about it
	}
if ((networkData.nStatus == NETSTAT_PLAYING) && (*eli.Connected () != 0))
	return; // Only accept disconnect packets if we're not out of the level yet
gameData.multiplayer.players [nPlayer].connected = *eli.Connected ();
memcpy (&gameData.multigame.score.matrix [nPlayer][0], eli.ScoreMatrix (), MAX_NUM_NET_PLAYERS * sizeof (short));
gameData.multiplayer.players [nPlayer].netKillsTotal = *eli.Kills ();
gameData.multiplayer.players [nPlayer].netKilledTotal = *eli.Killed ();
if ((gameData.multiplayer.players [nPlayer].connected == 1) && (*eli.SecondsLeft () < gameData.reactor.countdown.nSecsLeft))
	gameData.reactor.countdown.nSecsLeft = *eli.SecondsLeft ();
ResetPlayerTimeout (nPlayer, -1);
}

//------------------------------------------------------------------------------

void NetworkReadEndLevelShortPacket (ubyte *dataP)
{
	// Special packet for end of level syncing

	int						nPlayer;
	tEndLevelInfoShort*	eli;

eli = reinterpret_cast<tEndLevelInfoShort*> (dataP);
nPlayer = eli->nPlayer;
Assert (nPlayer != gameData.multiplayer.nLocalPlayer);
if (nPlayer >= gameData.multiplayer.nPlayers) {
	Int3 (); // weird, but it can happen in a coop restore game
	return; // if it happens in a coop restore, don't worry about it
	}

if ((networkData.nStatus == NETSTAT_PLAYING) && (eli->connected != 0))
	return; // Only accept disconnect packets if we're not out of the level yet
gameData.multiplayer.players [nPlayer].connected = eli->connected;
if ((gameData.multiplayer.players [nPlayer].connected == 1) && (eli->secondsLeft < gameData.reactor.countdown.nSecsLeft))
	gameData.reactor.countdown.nSecsLeft = eli->secondsLeft;
ResetPlayerTimeout (nPlayer, -1);
}

//------------------------------------------------------------------------------

void NetworkProcessSyncPacket (CNetGameInfo * sp, int rsinit)
{
	int					i, j;
	char					szLocalCallSign [CALLSIGN_LEN+1];
	tNetPlayerInfo*	playerP;
#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)
	CNetGameInfo		tmp_info;

if ((gameStates.multi.nGameType >= IPX_GAME) && (sp != &netGame)) { // for macintosh -- get the values unpacked to our structure format
	ReceiveFullNetGamePacket (reinterpret_cast<ubyte*> (sp), &tmp_info);
	sp = &tmp_info;
	}
#endif

if (rsinit)
	playerInfoP = &netPlayers;
	// This function is now called by all people entering the netgame.
if (sp != &netGame) {
	char *p = reinterpret_cast<char*> (sp);
	ushort h;
	int i, j = (int) netGame.Size () - 1, s;
	for (i = 0, h = -1; i < j; i++, p++) {
		s = *reinterpret_cast<ushort*> (p);
		if (s == networkData.nSegmentCheckSum) {
			h = i;
			break;
			}
		else if (((s / 256) + (s % 256) * 256) == networkData.nSegmentCheckSum) {
			h = i;
			break;
			}
		}
	netGame = *sp;
	netPlayers = *playerInfoP;
	}
gameData.multiplayer.nPlayers = sp->m_info.nNumPlayers;
gameStates.app.nDifficultyLevel = sp->m_info.difficulty;
networkData.nStatus = sp->m_info.gameStatus;
//Assert (gameStates.app.nFunctionMode != FMODE_GAME);
// New code, 11/27
#if 1
console.printf (1, "netGame.m_info.checksum = %d, calculated checksum = %d.\n",
					 netGame.GetSegmentCheckSum (), networkData.nSegmentCheckSum);
#endif
if (netGame.GetSegmentCheckSum () != networkData.nSegmentCheckSum) {
	if (extraGameInfo [0].bAutoDownload)
		networkData.nStatus = NETSTAT_AUTODL;
	else {
		short nInMenu = gameStates.menus.nInMenu;
		gameStates.menus.nInMenu = 0;
		networkData.nStatus = NETSTAT_MENU;
		MsgBox (TXT_ERROR, NULL, 1, TXT_OK, TXT_NETLEVEL_MISMATCH);
		gameStates.menus.nInMenu = nInMenu;
		}
#if 1//!DBG
		return;
#endif
	}
// Discover my CPlayerData number
memcpy (szLocalCallSign, LOCALPLAYER.callsign, CALLSIGN_LEN+1);
gameData.multiplayer.nLocalPlayer = -1;
for (i = 0; i < MAX_NUM_NET_PLAYERS; i++)
	gameData.multiplayer.players [i].netKillsTotal = 0;

for (i = 0, playerP = playerInfoP->m_info.players; i < gameData.multiplayer.nPlayers; i++, playerP++) {
	if (!CmpLocalPlayer (&playerP->network, playerP->callsign, szLocalCallSign)) {
		if (gameData.multiplayer.nLocalPlayer != -1) {
			Int3 (); // Hey, we've found ourselves twice
			MsgBox (TXT_ERROR, NULL, 1, TXT_OK, TXT_DUPLICATE_PLAYERS);
			console.printf (CON_DBG, TXT_FOUND_TWICE);
			networkData.nStatus = NETSTAT_MENU;
			return;
			}
		ChangePlayerNumTo (i);
		}
	memcpy (gameData.multiplayer.players [i].callsign, playerP->callsign, CALLSIGN_LEN+1);
	if (gameStates.multi.nGameType >= IPX_GAME) {
#ifdef WORDS_NEED_ALIGNMENT
		uint server;
		memcpy (&server, playerP->network.ipx.server, 4);
		if (server != 0)
			IpxGetLocalTarget (
				reinterpret_cast<ubyte*> (&server),
				playerInfoP->m_info.players [i].network.ipx.node,
				gameData.multiplayer.players [i].netAddress);
#else // WORDS_NEED_ALIGNMENT
		if (*reinterpret_cast<uint*> (playerInfoP->m_info.players [i].network.ipx.server) != 0)
			IpxGetLocalTarget (
				playerInfoP->m_info.players [i].network.ipx.server,
				playerInfoP->m_info.players [i].network.ipx.node,
				gameData.multiplayer.players [i].netAddress);
#endif // WORDS_NEED_ALIGNMENT
		else
			memcpy (gameData.multiplayer.players [i].netAddress, playerInfoP->m_info.players [i].network.ipx.node, 6);
		}
	gameData.multiplayer.players [i].nPacketsGot = -1;                             // How many packets we got from them
	gameData.multiplayer.players [i].nPacketsSent = 0;                            // How many packets we sent to them
	gameData.multiplayer.players [i].connected = playerP->connected;
	gameData.multiplayer.players [i].netKillsTotal = *sp->PlayerKills (i);
	gameData.multiplayer.players [i].netKilledTotal = *sp->Killed (i);
	if (networkData.nJoinState || (i != gameData.multiplayer.nLocalPlayer))
		gameData.multiplayer.players [i].score = *sp->PlayerScore (i);
	for (j = 0; j < MAX_NUM_NET_PLAYERS; j++)
		gameData.multigame.score.matrix [i][j] = *sp->Kills (i, j);
	}

if (gameData.multiplayer.nLocalPlayer < 0) {
	MsgBox (TXT_ERROR, NULL, 1, TXT_OK, TXT_PLAYER_REJECTED);
	networkData.nStatus = NETSTAT_MENU;
	return;
	}
if (networkData.nJoinState) {
	for (i = 0; i < gameData.multiplayer.nPlayers; i++)
		gameData.multiplayer.players [i].netKilledTotal = *sp->Killed (i);
	NetworkProcessMonitorVector (sp->GetMonitorVector ());
	LOCALPLAYER.timeLevel = sp->GetLevelTime ();
	}
gameData.multigame.score.nTeam [0] = *sp->TeamKills (0);
gameData.multigame.score.nTeam [1] = *sp->TeamKills (1);
LOCALPLAYER.connected = CONNECT_PLAYING;
netPlayers.m_info.players [gameData.multiplayer.nLocalPlayer].connected = CONNECT_PLAYING;
netPlayers.m_info.players [gameData.multiplayer.nLocalPlayer].rank = GetMyNetRanking ();
if (!networkData.nJoinState) {
	int	j, bGotTeamSpawnPos = (IsTeamGame) && GotTeamSpawnPos ();
	for (i = 0; i < gameData.multiplayer.nPlayerPositions; i++) {
		if (bGotTeamSpawnPos) {
			j = TeamSpawnPos (i);
			if (j < 0)
				j = *netGame.Locations (i);
			}
		else
			j = *netGame.Locations (i);
		GetPlayerSpawn (j, OBJECTS + gameData.multiplayer.players [i].nObject);
		}
	}
OBJECTS [LOCALPLAYER.nObject].SetType (OBJ_PLAYER);
networkData.nStatus = (IAmGameHost () || (networkData.nJoinState >= 4)) ? NETSTAT_PLAYING : NETSTAT_WAITING;
SetFunctionMode (FMODE_GAME);
networkData.bHaveSync = 1;
MultiSortKillList ();
}

//------------------------------------------------------------------------------

void NetworkTrackPackets (int nPlayer, int nPackets)
{
	CPlayerData*	playerP = gameData.multiplayer.players + nPlayer;

if (playerP->nPacketsGot < 0) {
	playerP->nPacketsGot = nPackets;
	networkData.nTotalPacketsGot += nPackets;
	}
else {
	playerP->nPacketsGot++;
	networkData.nTotalPacketsGot++;
	}
ResetPlayerTimeout (nPlayer, -1);
if  (nPackets != playerP->nPacketsGot) {
	networkData.nMissedPackets = nPackets - playerP->nPacketsGot;
	if (nPackets - playerP->nPacketsGot > 0)
		networkData.nTotalMissedPackets += nPackets - playerP->nPacketsGot;
#if 1
	if (networkData.nMissedPackets > 0)
		console.printf (0,
			"Missed %d packets from player #%d (%d total)\n",
			nPackets-playerP->nPacketsGot,
			nPlayer,
			networkData.nMissedPackets);
	else
		console.printf (CON_DBG,
			"Got %d late packets from player #%d (%d total)\n",
			playerP->nPacketsGot-nPackets,
			nPlayer,
			networkData.nMissedPackets);
#endif
	playerP->nPacketsGot = nPackets;
	}
}

//------------------------------------------------------------------------------

void NetworkReadPDataLongPacket (tFrameInfoLong *pd)
{
	int		nPlayer;
	int		theirObjNum;
	CObject* objP = NULL;

// tFrameInfoLong should be aligned...for mac, make the necessary adjustments
#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)
if (gameStates.multi.nGameType >= IPX_GAME) {
	pd->nPackets = INTEL_INT (pd->nPackets);
	pd->objPos[X] = INTEL_INT (pd->objPos[X]);
	pd->objPos[Y] = INTEL_INT (pd->objPos[Y]);
	pd->objPos[Z] = INTEL_INT (pd->objPos[Z]);
	pd->objOrient.RVec ()[X] = (fix)INTEL_INT ((int)pd->objOrient.RVec ()[X]);
	pd->objOrient.RVec ()[Y] = (fix)INTEL_INT ((int)pd->objOrient.RVec ()[Y]);
	pd->objOrient.RVec ()[Z] = (fix)INTEL_INT ((int)pd->objOrient.RVec ()[Z]);
	pd->objOrient.UVec ()[X] = (fix)INTEL_INT ((int)pd->objOrient.UVec ()[X]);
	pd->objOrient.UVec ()[Y] = (fix)INTEL_INT ((int)pd->objOrient.UVec ()[Y]);
	pd->objOrient.UVec ()[Z] = (fix)INTEL_INT ((int)pd->objOrient.UVec ()[Z]);
	pd->objOrient.FVec ()[X] = (fix)INTEL_INT ((int)pd->objOrient.FVec ()[X]);
	pd->objOrient.FVec ()[Y] = (fix)INTEL_INT ((int)pd->objOrient.FVec ()[Y]);
	pd->objOrient.FVec ()[Z] = (fix)INTEL_INT ((int)pd->objOrient.FVec ()[Z]);
	pd->physVelocity[X] = (fix)INTEL_INT ((int)pd->physVelocity[X]);
	pd->physVelocity[Y] = (fix)INTEL_INT ((int)pd->physVelocity[Y]);
	pd->physVelocity[Z] = (fix)INTEL_INT ((int)pd->physVelocity[Z]);
	pd->physRotVel[X] = (fix)INTEL_INT ((int)pd->physRotVel[X]);
	pd->physRotVel[Y] = (fix)INTEL_INT ((int)pd->physRotVel[Y]);
	pd->physRotVel[Z] = (fix)INTEL_INT ((int)pd->physRotVel[Z]);
	pd->nObjSeg = INTEL_SHORT (pd->nObjSeg);
	pd->dataSize = INTEL_SHORT (pd->dataSize);
	}
#endif
nPlayer = pd->nPlayer;
theirObjNum = gameData.multiplayer.players [pd->nPlayer].nObject;
if (nPlayer < 0) {
	Int3 (); // This packet is bogus!!
	return;
	}
if ((networkData.sync [0].nPlayer != -1) && (nPlayer == networkData.sync [0].nPlayer))
	networkData.sync [0].nPlayer = -1;
if (!gameData.multigame.bQuitGame && (nPlayer >= gameData.multiplayer.nPlayers)) {
	if (networkData.nStatus != NETSTAT_WAITING) {
		Int3 (); // We missed an important packet!
		//NetworkConsistencyError ();
		}
	return;
	}
if (gameStates.app.bEndLevelSequence || (networkData.nStatus == NETSTAT_ENDLEVEL)) {
	int old_Endlevel_sequence = gameStates.app.bEndLevelSequence;
	gameStates.app.bEndLevelSequence = 1;
	if (pd->dataSize > 0)
		// pass pd->data to some parser function....
		MultiProcessBigData (reinterpret_cast<char*> (pd->data), pd->dataSize);
	gameStates.app.bEndLevelSequence = old_Endlevel_sequence;
	return;
	}
if ((sbyte)pd->nLevel != missionManager.nCurrentLevel) {
#if 1
	console.printf (CON_DBG, "Got frame packet from CPlayerData %d wrong level %d!\n", pd->nPlayer, pd->nLevel);
#endif
	return;
	}

objP = OBJECTS + theirObjNum;
NetworkTrackPackets (nPlayer, pd->nPackets);
//------------ Read the player's ship's object info ----------------------
objP->info.position.vPos = pd->objPos;
objP->info.position.mOrient = pd->objOrient;
objP->mType.physInfo.velocity = pd->physVelocity;
objP->mType.physInfo.rotVel = pd->physRotVel;
if ((objP->info.renderType != pd->objRenderType) && (pd->objRenderType == RT_POLYOBJ))
	MultiMakeGhostPlayer (nPlayer);
OBJECTS [theirObjNum].RelinkToSeg (pd->nObjSeg);
if (objP->info.movementType == MT_PHYSICS)
	objP->SetThrustFromVelocity ();
//------------ Welcome them back if reconnecting --------------
if (!gameData.multiplayer.players [nPlayer].connected) {
	gameData.multiplayer.players [nPlayer].connected = CONNECT_PLAYING;
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordMultiReconnect (nPlayer);
	MultiMakeGhostPlayer (nPlayer);
	OBJECTS [theirObjNum].CreateAppearanceEffect ();
	audio.PlaySound (SOUND_HUD_MESSAGE);
	ClipRank (reinterpret_cast<char*> (&netPlayers.m_info.players [nPlayer].rank));
	if (gameOpts->multi.bNoRankings)
		HUDInitMessage ("'%s' %s", gameData.multiplayer.players [nPlayer].callsign, TXT_REJOIN);
	else
		HUDInitMessage ("%s'%s' %s",
							 pszRankStrings [netPlayers.m_info.players [nPlayer].rank],
							 gameData.multiplayer.players [nPlayer].callsign, TXT_REJOIN);
	MultiSendScore ();
	}
//------------ Parse the extra dataP at the end ---------------
if (pd->dataSize > 0)
	// pass pd->data to some parser function....
	MultiProcessBigData (reinterpret_cast<char*> (pd->data), pd->dataSize);
}

//------------------------------------------------------------------------------

#if defined (WORDS_BIGENDIAN) || defined (__BIG_ENDIAN__)

void GetShortFrameInfo (ubyte *old_info, tFrameInfoShort *new_info)
{
	int bufI = 0;

NW_GET_BYTE (old_info, bufI, new_info->nType);
/* skip three for pad byte */
bufI += 3;
NW_GET_INT (old_info, bufI, new_info->nPackets);
NW_GET_BYTES (old_info, bufI, new_info->objPos.orient, 9);
NW_GET_SHORT (old_info, bufI, new_info->objPos.pos [0]);
NW_GET_SHORT (old_info, bufI, new_info->objPos.pos [1]);
NW_GET_SHORT (old_info, bufI, new_info->objPos.pos [2]);
NW_GET_SHORT (old_info, bufI, new_info->objPos.nSegment);
NW_GET_SHORT (old_info, bufI, new_info->objPos.vel [0]);
NW_GET_SHORT (old_info, bufI, new_info->objPos.vel [1]);
NW_GET_SHORT (old_info, bufI, new_info->objPos.vel [2]);
NW_GET_SHORT (old_info, bufI, new_info->dataSize);
NW_GET_BYTE (old_info, bufI, new_info->nPlayer);
NW_GET_BYTE (old_info, bufI, new_info->objRenderType);
NW_GET_BYTE (old_info, bufI, new_info->nLevel);
NW_GET_BYTES (old_info, bufI, new_info->data, new_info->dataSize);
}
#else

#define GetShortFrameInfo(old_info, new_info) \
	memcpy (new_info, old_info, sizeof (tFrameInfoShort))

#endif

//------------------------------------------------------------------------------

void NetworkReadPDataShortPacket (tFrameInfoShort *pd)
{
	int nPlayer;
	int nObject;
	CObject * objP = NULL;
	tFrameInfoShort new_pd;

// short frame info is not aligned because of tShortPos.  The mac
// will call totally hacked and gross function to fix this up.

if (gameStates.multi.nGameType >= IPX_GAME)
	GetShortFrameInfo (reinterpret_cast<ubyte*> (pd), &new_pd);
else
	memcpy (&new_pd, reinterpret_cast<ubyte*> (pd), sizeof (tFrameInfoShort));
nPlayer = new_pd.nPlayer;
nObject = gameData.multiplayer.players [new_pd.nPlayer].nObject;
if (nPlayer < 0) {
	Int3 (); // This packet is bogus!!
	return;
	}
if (!gameData.multigame.bQuitGame && (nPlayer >= gameData.multiplayer.nPlayers)) {
	if (networkData.nStatus != NETSTAT_WAITING) {
		Int3 (); // We missed an important packet!
		NetworkConsistencyError ();
		}
	return;
	}
if ((networkData.sync [0].nPlayer != -1) && (nPlayer == networkData.sync [0].nPlayer)) {
	// Hurray! Someone really really got in the game (I think).
   networkData.sync [0].nPlayer = -1;
	}

if (gameStates.app.bEndLevelSequence || (networkData.nStatus == NETSTAT_ENDLEVEL)) {
	int old_Endlevel_sequence = gameStates.app.bEndLevelSequence;
	gameStates.app.bEndLevelSequence = 1;
	if (new_pd.dataSize > 0) {
		// pass pd->data to some parser function....
		MultiProcessBigData (reinterpret_cast<char*> (new_pd.data), new_pd.dataSize);
		}
	gameStates.app.bEndLevelSequence = old_Endlevel_sequence;
	return;
	}
if ((sbyte) new_pd.nLevel != missionManager.nCurrentLevel) {
#if 1
	console.printf (CON_DBG, "Got frame packet from CPlayerData %d wrong level %d!\n", new_pd.nPlayer, new_pd.nLevel);
#endif
	return;
	}
objP = OBJECTS + nObject;
NetworkTrackPackets (nPlayer, new_pd.nPackets);
//------------ Read the player's ship's CObject info ----------------------
ExtractShortPos (objP, &new_pd.objPos, 0);
if ((objP->info.renderType != new_pd.objRenderType) && (new_pd.objRenderType == RT_POLYOBJ))
	MultiMakeGhostPlayer (nPlayer);
if (objP->info.movementType == MT_PHYSICS)
	objP->SetThrustFromVelocity ();
//------------ Welcome them back if reconnecting --------------
if (!gameData.multiplayer.players [nPlayer].connected) {
	gameData.multiplayer.players [nPlayer].connected = CONNECT_PLAYING;
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordMultiReconnect (nPlayer);
	MultiMakeGhostPlayer (nPlayer);
	OBJECTS [nObject].CreateAppearanceEffect ();
	audio.PlaySound (SOUND_HUD_MESSAGE);
	ClipRank (reinterpret_cast<char*> (&netPlayers.m_info.players [nPlayer].rank));
	if (gameOpts->multi.bNoRankings)
		HUDInitMessage ("'%s' %s", gameData.multiplayer.players [nPlayer].callsign, TXT_REJOIN);
	else
		HUDInitMessage ("%s'%s' %s",
							 pszRankStrings [netPlayers.m_info.players [nPlayer].rank],
							 gameData.multiplayer.players [nPlayer].callsign, TXT_REJOIN);
	MultiSendScore ();
	}
//------------ Parse the extra dataP at the end ---------------
if (new_pd.dataSize > 0) {
	// pass pd->data to some parser function....
	MultiProcessBigData (reinterpret_cast<char*> (new_pd.data), new_pd.dataSize);
	}
}

//------------------------------------------------------------------------------

int NetworkVerifyObjects (int nRemoteObj, int nLocalObjs)
{
return !gameStates.app.bHaveExtraGameInfo [1] && (nRemoteObj - nLocalObjs > 10) ? -1 : 0;
}

//------------------------------------------------------------------------------

int NetworkVerifyPlayers (void)
{
	int				i, j, t, bCoop = IsCoopGame;
	int				nPlayers, nPlayerObjs [MAX_PLAYERS], bHaveReactor = !bCoop;
	CObject*			objP;
	CPlayerData*	playerP;

for (j = 0, playerP = gameData.multiplayer.players; j < MAX_PLAYERS; j++, playerP++)
	nPlayerObjs [j] = playerP->connected ? playerP->nObject : -1;
#if 0
if (gameData.app.nGameMode & GM_MULTI_ROBOTS)
#endif
//	bHaveReactor = 1;	// multiplayer maps do not need a control center ...
nPlayers = 0;
FORALL_OBJS (objP, i) {
	i = objP->Index ();
	t = objP->info.nType;
	if (t == OBJ_GHOST) {
		for (j = 0; j < MAX_PLAYERS; j++) {
			if (nPlayerObjs [j] == i) {
				nPlayers++;
				break;
				}
			}
		}
	else if (t == OBJ_PLAYER) {
		if (!(i && bCoop))
			nPlayers++;
		}
	else if (t == OBJ_COOP) {
		if (bCoop)
			nPlayers++;
		}
	else if (bCoop) {
		if ((t == OBJ_REACTOR) || ((t == OBJ_ROBOT) && ROBOTINFO (objP->info.nId).bossFlag))
			bHaveReactor = 1;
		}
	if (nPlayers > gameData.multiplayer.nMaxPlayers)
		return 1;
	}
return !bHaveReactor;
}

//------------------------------------------------------------------------------

void NetworkAbortSync (void)
{
MsgBox (NULL, NULL, 1, TXT_OK, TXT_NET_SYNC_FAILED);
networkData.nStatus = NETSTAT_MENU;
}

//------------------------------------------------------------------------------

static int NetworkCheckMissingFrames (void)
{
if (networkData.nPrevFrame == networkData.sync [0].objs.nFrame - 1)
	return 1;
if (!networkData.nPrevFrame || (networkData.nPrevFrame >= networkData.sync [0].objs.nFrame))
	return 0;
networkData.sync [0].objs.missingFrames.nFrame = networkData.nPrevFrame + 1;
networkData.nJoinState = 2;
NetworkSendMissingObjFrames ();
return -1;
}

//------------------------------------------------------------------------------

inline bool ObjectIsLinked (CObject *objP, short nSegment)
{
if (nSegment != -1) {
	short nObject = objP->Index ();
	for (short i = SEGMENTS [nSegment].m_objects, j = -1; i >= 0; j = i, i = OBJECTS [i].info.nNextInSeg) {
		if (i == nObject) {
			objP->info.nPrevInSeg = j;
			return true;
			}
		}
	}
return false;
}

//------------------------------------------------------------------------------

void NetworkReadObjectPacket (ubyte *dataP)
{
	static int	nPlayer = 0;
	static int	nMode = 0;

	// Object from another net CPlayerData we need to sync with
	CObject		*objP;
	short			nObject, nRemoteObj;
	sbyte			nObjOwner;
	short			nSegment, i;

	int			nObjects = dataP [1];
	int			bufI;

networkData.nPrevFrame = networkData.sync [0].objs.nFrame;
if (gameStates.multi.nGameType == UDP_GAME) {
	bufI = 2;
	NW_GET_SHORT (dataP, bufI, networkData.sync [0].objs.nFrame);
	}
else {
	networkData.sync [0].objs.nFrame = dataP [2];
	bufI = 3;
	}
i = NetworkCheckMissingFrames ();
if (!i) {
	networkData.toSyncPoll = 0;
	return;
	}
else if (i < 0)
	return;
#if DBG
//PrintLog ("Receiving object packet %d (prev: %d)\n", networkData.nPrevFrame, networkData.sync [0].objs.nFrame);
#endif
 for (i = 0; i < nObjects; i++) {
	objP = NULL;
	NW_GET_SHORT (dataP, bufI, nObject);
	NW_GET_BYTE (dataP, bufI, nObjOwner);
	NW_GET_SHORT (dataP, bufI, nRemoteObj);
	if ((nObject == -1) || (nObject == -3)) {
		// Clear CObject array
		nPlayer = nObjOwner;
		nMode = 1;
		networkData.nPrevFrame = networkData.sync [0].objs.nFrame - 1;
		if (nObject == -3) {
			if (networkData.nJoinState != 2)
				return;
#if DBG
			PrintLog ("Receiving missing object packets\n");
#endif
			networkData.nJoinState = 3;
			}
		else {
			if (networkData.nJoinState)
				return;
			InitObjects (false);
			ChangePlayerNumTo (nPlayer);
			InitMultiPlayerObject (1);
			gameData.objs.nObjects = 0;
			networkData.nJoinState = 1;
			}
		networkData.sync [0].objs.missingFrames.nFrame = 0;
		}
	else if ((nObject == -2) || (nObject == -4)) {	// Special debug checksum marker for entire send
 		if (!nMode && NetworkVerifyObjects (nRemoteObj, gameData.objs.nObjects)) {
			NetworkAbortSync ();
			return;
			}
		gameData.objs.RebuildEffects ();
		networkData.sync [0].objs.nFrame = 0;
		nMode = 0;
		if (networkData.bHaveSync)
			networkData.nStatus = NETSTAT_PLAYING;
		networkData.nJoinState = 4;
		}
	else if (networkData.nJoinState & 1) {
#if 1
		console.printf (CON_DBG, "Got a type 3 object packet!\n");
#endif
		nObject = nRemoteObj;
		if (!InsertObject (nObject)) {
			if (networkData.nJoinState == 3) {
				FreeObject (nObject);
				if (!InsertObject (nObject))
					nObject = -1;
				}
			else
				nObject = AllocObject ();
			}
		if ((nObjOwner != nPlayer) && (nObjOwner != -1)) {
			if (nMode == 1)
				nMode = 0;
			else
				Int3 (); // SEE ROB
			}
		if (nObject != -1) {
			Assert (nObject < LEVEL_OBJECTS);
			objP = OBJECTS + nObject;
			objP->Unlink (true);
			while (ObjectIsLinked (objP, objP->info.nSegment))
				objP->UnlinkFromSeg ();
			NW_GET_BYTES (dataP, bufI, &objP->info, sizeof (tBaseObject));
			if (objP->info.nType != OBJ_NONE) {
#if defined(WORDS_BIGENDIAN) || defined(__BIG_ENDIAN__)
				if (gameStates.multi.nGameType >= IPX_GAME)
					SwapObject (objP);
#endif
				nSegment = objP->info.nSegment;
				PrintLog ("receiving object %d (type: %d, segment: %d)\n", nObject, objP->info.nType, nSegment);
				objP->ResetSgmLinks ();
				objP->ResetLinks ();
				objP->info.nAttachedObj = -1;
				objP->Link ();
				if (nSegment < 0)
					nSegment = FindSegByPos (objP->info.position.vPos, -1, 1, 0);
				if (!ObjectIsLinked (objP, nSegment))
					objP->LinkToSeg (nSegment);
				if ((objP->info.nType == OBJ_PLAYER) || (objP->info.nType == OBJ_GHOST))
					RemapLocalPlayerObject (nObject, nRemoteObj);
				if (nObjOwner == nPlayer)
					MapObjnumLocalToLocal (nObject);
				else if (nObjOwner != -1)
					MapObjnumLocalToRemote (nObject, nRemoteObj, nObjOwner);
				else
					gameData.multigame.nObjOwner [nObject] = -1;
				if (objP->Type () == OBJ_MONSTERBALL) {
					gameData.hoard.monsterballP = objP;
					gameData.hoard.nMonsterballSeg = nSegment;
					}
				}
			}
		} // For a standard onbject
	} // For each CObject in packet
//gameData.objs.nLastObject [0] = gameData.objs.nObjects - 1;
}

//------------------------------------------------------------------------------

