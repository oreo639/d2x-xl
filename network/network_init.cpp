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

#ifndef _WIN32
#	include <arpa/inet.h>
#	include <netinet/in.h> /* for htons & co. */
#endif

#include "descent.h"
#include "ipx.h"
#include "error.h"
#include "network.h"
#include "network_lib.h"
#include "netmisc.h"
#include "hogfile.h"

#define SECURITY_CHECK	1

//-----------------------------------------------------------------------------

void ResetPingStats (void)
{
memset (pingStats, 0, sizeof (pingStats));
networkData.tLastPingStat = 0;
}

//------------------------------------------------------------------------------

void ResetNetworkData (void)
{
networkData.localAddress.Reset ();
networkData.serverAddress.Reset ();
networkData.nActiveGames = 0;
networkData.nLastActiveGames = 0;
networkData.nNamesInfoSecurity = 0;
networkData.nMinPPS = 0;
networkData.nNetLifeKills = 0;
networkData.nNetLifeKilled = 0;
networkData.bDebug = 0;
networkData.bActive = 0;
networkData.nStatus = 0;
networkData.bGamesChanged = 0;
networkData.nPortOffset = 0;
networkData.bAllowSocketChanges = 0;
networkData.nSecurityFlag = 0;
networkData.nSecurityNum = 0;
networkData.nJoinState = 0;
networkData.bNewGame = 0;       
networkData.bPlayerAdded = 0;   
networkData.bD2XData = 0;
networkData.nSecurityCheck = 0;
networkData.bPacketUrgent = 0;
networkData.nGameType = 0;
networkData.nTotalMissedPackets = 0;
networkData.nTotalPacketsGot = 0;
networkData.nMissedPackets = 0;
networkData.nConsistencyErrorCount = 0;
networkData.bSyncPackInited = 0;       
networkData.bWantPlayersInfo = 0;
networkData.bWaitingForPlayerInfo = 0;
networkData.nSegmentCheckSum = 0;
networkData.nStartWaitAllTime = 0;
memset (networkData.nLastPacketTime, 0, sizeof (networkData.nLastPacketTime));
networkData.xLastSendTime = 0;
networkData.xLastTimeoutCheck = 0;
networkData.xPingReturnTime = 0;
networkData.tLastPingStat = 0;
networkData.bWaitAllChoice = 0;
networkData.bShowPingStats = 0;
networkData.bHaveSync = 0;
networkData.nPrevFrame = 0;
networkData.bTraceFrames = 0;
networkData.nJoining = 0;
networkData.xmlGameInfoRequestTime = 0;
memset (&networkData.refuse, 0, sizeof (networkData.refuse));
memset (&networkData.pThislayer, 0, sizeof (networkData.pThislayer));
memset (networkData.syncInfo, 0, sizeof (networkData.syncInfo));
}

//------------------------------------------------------------------------------

void InitNetworkData (void)
{
ResetNetworkData ();
networkData.xmlGameInfoRequestTime = -1;
networkData.nActiveGames = 0;
networkData.nLastActiveGames = 0;
networkData.nMinPPS = DEFAULT_PPS;
networkData.nNamesInfoSecurity = -1;
networkData.nNetLifeKills = 0;
networkData.nNetLifeKilled = 0;
networkData.bDebug = 0;
networkData.bActive = 0;
networkData.nStatus = 0;
networkData.bGamesChanged = 0;
networkData.nPortOffset = 0;
networkData.bAllowSocketChanges = 1;
networkData.nSecurityFlag = NETSECURITY_OFF;
networkData.nSecurityNum = 0;
NetworkResetSyncStates ();
networkData.nJoinState = 0;					// Did WE rejoin this game?
networkData.bNewGame = 0;					// Is this the first level of a new game?
networkData.bPlayerAdded = 0;				// Is this a new CPlayerData or a returning CPlayerData?
networkData.bPacketUrgent = 0;
networkData.nGameType = 0;
networkData.nTotalMissedPackets = 0;
networkData.nTotalPacketsGot = 0;
networkData.bSyncPackInited = 1;		// Set to 1 if the networkData.syncPack is zeroed.
networkData.nSegmentCheckSum = 0;
networkData.bWantPlayersInfo = 0;
networkData.bWaitingForPlayerInfo = 0;
}

//------------------------------------------------------------------------------

void NetworkInit (void)
{
	int32_t nPlayerSave = N_LOCALPLAYER;

GameDisableCheats ();
gameStates.multi.bIWasKicked = 0;
gameStates.gameplay.bFinalBossIsDead = 0;
networkData.nNamesInfoSecurity = -1;
#ifdef NETPROFILING
OpenSendLog ();
OpenReceiveLog (); 
#endif
InitAddressFilter ();
InitPacketHandlers ();
gameData.multiplayer.maxPowerupsAllowed.Clear (0);
gameData.multiplayer.powerupsInMine.Clear (0);
networkData.nTotalMissedPackets = 0; 
networkData.nTotalPacketsGot = 0;
memset (&netGameInfo, 0, sizeof (netGameInfo));
memset (&netPlayers [0], 0, sizeof (netPlayers [0]));
networkData.pThislayer.nType = PID_REQUEST;
memcpy (networkData.pThislayer.player.callsign, LOCALPLAYER.callsign, CALLSIGN_LEN+1);
networkData.pThislayer.player.versionMajor = D2X_MAJOR;
networkData.pThislayer.player.versionMinor = D2X_MINOR | (IS_D2_OEM ? NETWORK_OEM : 3);
networkData.pThislayer.player.rank=GetMyNetRanking ();
if (gameStates.multi.nGameType >= IPX_GAME) {
	memcpy (networkData.pThislayer.player.network.Node (), IpxGetMyLocalAddress (), 6);
	networkData.pThislayer.player.network.SetNetwork (IpxGetMyServerAddress ());
	}
networkData.pThislayer.player.computerType = DOS;
N_LOCALPLAYER = nPlayerSave;         
MultiNewGame ();
networkData.bNewGame = 1;
gameData.reactorData.bDestroyed = 0;
NetworkFlush ();
netGameInfo.SetMinPPS (mpParams.nMinPPS);
memcpy (extraGameInfo + 2, extraGameInfo, sizeof (extraGameInfo [0]));
//networkThread.Start ();
}

//------------------------------------------------------------------------------

int32_t NetworkCreateMonitorVector (void)
{
	#define MAX_BLOWN_BITMAPS 100
	
	int32_t      blownBitmaps [MAX_BLOWN_BITMAPS];
	int32_t      nBlownBitmaps = 0;
	int32_t      nMonitor = 0;
	int32_t      vector = 0;
	CSegment *pSeg = SEGMENTS.Buffer ();
	CSide    *pSide;
	int32_t      h, i, j, k;
	int32_t      tm, ec;

for (i = 0; i < gameData.effectData.nEffects [gameStates.app.bD1Data]; i++) {
	if ((h = gameData.effectData.pEffect [i].destroyed.nTexture) > 0) {
		for (j = 0; j < nBlownBitmaps; j++)
			if (blownBitmaps [j] == h)
				break;
		if (j == nBlownBitmaps) {
			blownBitmaps [nBlownBitmaps++] = h;
			Assert (nBlownBitmaps < MAX_BLOWN_BITMAPS);
			}
		}
	}               
	
for (i = 0; i <= gameData.segData.nLastSegment; i++, pSeg++) {
	for (j = 0, pSide = pSeg->m_sides; j < SEGMENT_SIDE_COUNT; j++, pSide++) {
		if (!pSide->FaceCount ())
			continue;
		if ((tm = pSide->m_nOvlTex) != 0) {
			if (((ec = gameData.pigData.tex.pTexMapInfo [tm].nEffectClip) != -1) &&
					(gameData.effectData.pEffect[ec].destroyed.nTexture != -1)) {
				nMonitor++;
				//Assert (nMonitor < 32);
				}
			else {
				for (k = 0; k < nBlownBitmaps; k++) {
					if ((tm) == blownBitmaps [k]) {
						vector |= (1 << nMonitor);
						nMonitor++;
						//Assert (nMonitor < 32);
						break;
						}
					}
				}
			}
		}
	}
return vector;
}

//------------------------------------------------------------------------------

void InitMonsterballSettings (tMonsterballInfo *pMonsterBall)
{
	tMonsterballForce *pForce = pMonsterBall->forces;

// primary weapons
pForce [0].nWeaponId = LASER_ID; 
pForce [0].nForce = 10;
pForce [1].nWeaponId = LASER_ID + 1; 
pForce [1].nForce = 20;
pForce [2].nWeaponId = LASER_ID + 2; 
pForce [2].nForce = 30;
pForce [3].nWeaponId = LASER_ID + 3; 
pForce [3].nForce = 50;
pForce [4].nWeaponId = SPREADFIRE_ID; 
pForce [4].nForce = 100;
pForce [5].nWeaponId = VULCAN_ID; 
pForce [5].nForce = 20;
pForce [6].nWeaponId = PLASMA_ID; 
pForce [6].nForce = 100;
pForce [7].nWeaponId = FUSION_ID; 
pForce [7].nForce = 500;
// primary "super" weapons
pForce [8].nWeaponId = SUPERLASER_ID; 
pForce [8].nForce = 75;
pForce [9].nWeaponId = SUPERLASER_ID + 1; 
pForce [9].nForce = 100;
pForce [10].nWeaponId = HELIX_ID; 
pForce [10].nForce = 200;
pForce [11].nWeaponId = GAUSS_ID; 
pForce [11].nForce = 30;
pForce [12].nWeaponId = PHOENIX_ID; 
pForce [12].nForce = 200;
pForce [13].nWeaponId = OMEGA_ID; 
pForce [13].nForce = 1;
pForce [14].nWeaponId = FLARE_ID; 
pForce [14].nForce = 1;
// missiles
pForce [15].nWeaponId = CONCUSSION_ID; 
pForce [15].nForce = 500;
pForce [16].nWeaponId = HOMINGMSL_ID; 
pForce [16].nForce = 500;
pForce [17].nWeaponId = SMARTMSL_ID; 
pForce [17].nForce = 1000;
pForce [18].nWeaponId = MEGAMSL_ID; 
pForce [18].nForce = 5000;
// "super" missiles
pForce [19].nWeaponId = FLASHMSL_ID; 
pForce [19].nForce = 500;
pForce [20].nWeaponId = GUIDEDMSL_ID; 
pForce [20].nForce = 500;
pForce [21].nWeaponId = MERCURYMSL_ID; 
pForce [21].nForce = 300;
pForce [22].nWeaponId = EARTHSHAKER_ID; 
pForce [22].nForce = 10000;
pForce [23].nWeaponId = EARTHSHAKER_MEGA_ID; 
pForce [23].nForce = 2500;
// CPlayerData ships
pForce [24].nWeaponId = 255;
pForce [24].nForce = 4;
pMonsterBall->nBonus = 1;
pMonsterBall->nSizeMod = 7;	// that is actually shield orb size * 3, because it's divided by 2, thus allowing for half sizes
}

//------------------------------------------------------------------------------

void InitEntropySettings (int32_t i)
{
extraGameInfo [i].entropy.nEnergyFillRate = 25;
extraGameInfo [i].entropy.nShieldFillRate = 11;
extraGameInfo [i].entropy.nShieldDamageRate = 11;
extraGameInfo [i].entropy.nMaxVirusCapacity = 0; 
extraGameInfo [i].entropy.nBumpVirusCapacity = 2;
extraGameInfo [i].entropy.nBashVirusCapacity = 1; 
extraGameInfo [i].entropy.nVirusGenTime = 2; 
extraGameInfo [i].entropy.nVirusLifespan = 0; 
extraGameInfo [i].entropy.nVirusStability = 0;
extraGameInfo [i].entropy.nCaptureVirusThreshold = 1; 
extraGameInfo [i].entropy.nCaptureTimeThreshold = 1; 
extraGameInfo [i].entropy.bRevertRooms = 0;
extraGameInfo [i].entropy.bDoCaptureWarning = 0;
extraGameInfo [i].entropy.nOverrideTextures = 2;
extraGameInfo [i].entropy.bBrightenRooms = 0;
extraGameInfo [i].entropy.bPlayerHandicap = 0;
}

//------------------------------------------------------------------------------

void InitExtraGameInfo (void)
{
	int32_t	i;

for (i = 0; i < 2; i++) {
	extraGameInfo [i].nType = 0;
	extraGameInfo [i].bFriendlyFire = 1;
	extraGameInfo [i].bInhibitSuicide = 0;
	extraGameInfo [i].bFixedRespawns = 0;
	extraGameInfo [i].nSpawnDelay = 0;
	extraGameInfo [i].bEnhancedCTF = 0;
	extraGameInfo [i].bRadarEnabled = i ? 0 : 1;
	extraGameInfo [i].bPowerupsOnRadar = 0;
	extraGameInfo [i].nZoomMode = 0;
	extraGameInfo [i].bRobotsHitRobots = 0;
	extraGameInfo [i].bAutoDownload = 1;
	extraGameInfo [i].bAutoBalanceTeams = 0;
	extraGameInfo [i].bDualMissileLaunch = 0;
	extraGameInfo [i].bRobotsOnRadar = 0;
	extraGameInfo [i].grWallTransparency = (FADE_LEVELS * 10 + 3) / 6;
	extraGameInfo [i].nSpeedBoost = 10;
	extraGameInfo [i].bDropAllMissiles = 1;
	extraGameInfo [i].bImmortalPowerups = 0;
	extraGameInfo [i].bUseCameras = 1;
	extraGameInfo [i].nFusionRamp = 2;
	extraGameInfo [i].nOmegaRamp = 2;
	extraGameInfo [i].bWiggle = 1;
	extraGameInfo [i].bMultiBosses = 0;
	extraGameInfo [i].nBossCount [0] = 0;
	extraGameInfo [i].nBossCount [1] = 0;
	extraGameInfo [i].bSmartWeaponSwitch = 0;
	extraGameInfo [i].bFluidPhysics = 0;
	extraGameInfo [i].nWeaponDropMode = 1;
	extraGameInfo [i].bRotateLevels = 0;
	extraGameInfo [i].bDisableReactor = 0;
	extraGameInfo [i].bMouseLook = i ? 0 : 1;
	extraGameInfo [i].nWeaponIcons = 3;
	extraGameInfo [i].bSafeUDP = 0;
	extraGameInfo [i].bFastPitch = 2;
	extraGameInfo [i].bUseParticles = 1;
	extraGameInfo [i].bUseLightning = 1;
	extraGameInfo [i].bDamageExplosions = 1;
	extraGameInfo [i].bThrusterFlames = 1;
	extraGameInfo [i].bShadows = 1;
	extraGameInfo [i].nShieldEffect = gameOpts->render.effects.bShields;
	extraGameInfo [i].bTeleporterCams = 0;
	extraGameInfo [i].bDarkness = 0;
	extraGameInfo [i].bTeamDoors = 0;
	extraGameInfo [i].bEnableCheats = 0;
	extraGameInfo [i].bTargetIndicators = 0;
	extraGameInfo [i].bDamageIndicators = 0;
	extraGameInfo [i].bFriendlyIndicators = 0;
	extraGameInfo [i].bCloakedIndicators = 0;
	extraGameInfo [i].bMslLockIndicators = 0;
	extraGameInfo [i].bTagOnlyHitObjs = 0;
	extraGameInfo [i].bTowFlags = 1;
	extraGameInfo [i].bUseHitAngles = 0;
	extraGameInfo [i].bLightTrails = 0;
	extraGameInfo [i].bTracers = 0;
	extraGameInfo [i].bShockwaves = 0;
	extraGameInfo [i].bCompetition = 0;
	extraGameInfo [i].bPowerupLights = i;
	extraGameInfo [i].bBrightObjects = i;
	extraGameInfo [i].bCheckUDPPort = 1;
	extraGameInfo [i].bSmokeGrenades = 0;
	extraGameInfo [i].nWeaponTurnSpeed = 0;
	extraGameInfo [i].nMslStartSpeed = 0;
	extraGameInfo [i].nMaxSmokeGrenades = 2;
	extraGameInfo [i].nCoopPenalty = 0;
	extraGameInfo [i].bKillMissiles = 0;
	extraGameInfo [i].bTripleFusion = 0;
	extraGameInfo [i].bShowWeapons = 0;
	extraGameInfo [i].bAllowCustomWeapons = 1;
	extraGameInfo [i].bEnhancedShakers = 0;
	extraGameInfo [i].nHitboxes = 0;
	extraGameInfo [i].nRadar = 0;
	extraGameInfo [i].nDrag = 10;
	extraGameInfo [i].nSpotSize = 2 - i;
	extraGameInfo [i].nSpotStrength = 2 - i;
	extraGameInfo [i].nLightRange = 0;
	extraGameInfo [i].headlight.bAvailable = 1;
	extraGameInfo [i].headlight.bDrainPower = 1;
	extraGameInfo [i].timeout.nDisconnectPlayer = TIMEOUT_DISCONNECT;
	extraGameInfo [i].timeout.nKickPlayer = TIMEOUT_KICK;
	extraGameInfo [i].timeout.nKeepMessage = TIMEOUT_MSG_KEEP_COOP;
	extraGameInfo [i].timeout.nResendMessage = TIMEOUT_MSG_RESEND_COOP;
	InitEntropySettings (i);
	InitMonsterballSettings (&extraGameInfo [i].monsterball);
	}
}

//------------------------------------------------------------------------------

void NetworkSetTimeoutValues (void)
{
networkData.nCompetitionLevel = COMPETITION_LEVEL;
for (int32_t i = 0; i < 2; i++) {
	switch (networkData.nCompetitionLevel) {
		case 2:
			extraGameInfo [i].timeout.nKeepMessage = TIMEOUT_MSG_KEEP_COMP;
			extraGameInfo [i].timeout.nResendMessage = TIMEOUT_MSG_RESEND_COMP;
			break;

		case 1:
			extraGameInfo [i].timeout.nKeepMessage = TIMEOUT_MSG_KEEP_STD;
			extraGameInfo [i].timeout.nResendMessage = TIMEOUT_MSG_RESEND_STD;
			break;

		default:
			extraGameInfo [i].timeout.nKeepMessage = TIMEOUT_MSG_KEEP_COOP;
			extraGameInfo [i].timeout.nResendMessage = TIMEOUT_MSG_RESEND_COOP;
			break;
		}
	}
}

//------------------------------------------------------------------------------

int32_t InitAutoNetGame (void)
{
if (gameData.multiplayer.autoNG.bValid <= 0)
	return 0;
PrintLog (1, "Preparing automatic netgame launch\n");
if (gameData.multiplayer.autoNG.bHost) {
	if (!missionManager.FindByName (gameData.multiplayer.autoNG.szFile, -1)) {
		PrintLog (-1);
		return 0;
		}
	hogFileManager.UseMission (gameData.multiplayer.autoNG.szFile);
	strcpy (szAutoMission, gameData.multiplayer.autoNG.szMission);
	gameStates.app.bAutoRunMission = hogFileManager.AltFiles ().bInitialized;
	strncpy (mpParams.szGameName, gameData.multiplayer.autoNG.szName, sizeof (mpParams.szGameName));
	mpParams.nLevel = gameData.multiplayer.autoNG.nLevel;
	extraGameInfo [0].bEnhancedCTF = 0;
	switch (gameData.multiplayer.autoNG.uType) {
		case 0:
			mpParams.nGameMode = gameData.multiplayer.autoNG.bTeam ? NETGAME_TEAM_ANARCHY : NETGAME_ANARCHY;
			break;
		case 1:
			mpParams.nGameMode = NETGAME_COOPERATIVE;
			break;
		case 2:
			mpParams.nGameMode = NETGAME_CAPTURE_FLAG;
			break;
		case 3:
			mpParams.nGameMode = NETGAME_CAPTURE_FLAG;
			extraGameInfo [0].bEnhancedCTF = 1;
			break;
		case 4:
			if (HoardEquipped ())
				mpParams.nGameMode = gameData.multiplayer.autoNG.bTeam ? NETGAME_TEAM_HOARD : NETGAME_HOARD;
			else
				mpParams.nGameMode = gameData.multiplayer.autoNG.bTeam ? NETGAME_TEAM_ANARCHY : NETGAME_ANARCHY;
			break;
		case 5:
			mpParams.nGameMode = NETGAME_ENTROPY;
			break;
		case 6:
			mpParams.nGameMode = NETGAME_MONSTERBALL;
			break;
			}
	mpParams.nGameType = mpParams.nGameMode;
	}
else {
	networkData.serverAddress.SetServer (gameData.multiplayer.autoNG.ipAddr);
	mpParams.udpPorts [0] = gameData.multiplayer.autoNG.nPort;
	}
PrintLog (-1);
return 1;
}

//------------------------------------------------------------------------------

void LogExtraGameInfo (void)
{
if (!gameStates.app.bHaveExtraGameInfo [1])
	PrintLog (0, "No extra game info data available\n");
else {
	PrintLog (1, "extra game info data:\n");
	PrintLog (0, "bFriendlyFire: %d\n", extraGameInfo [1].bFriendlyFire);
	PrintLog (0, "bInhibitSuicide: %d\n", extraGameInfo [1].bInhibitSuicide);
	PrintLog (0, "bFixedRespawns: %d\n", extraGameInfo [1].bFixedRespawns);
	PrintLog (0, "nSpawnDelay: %d\n", extraGameInfo [1].nSpawnDelay);
	PrintLog (0, "bEnhancedCTF: %d\n", extraGameInfo [1].bEnhancedCTF);
	PrintLog (0, "bRadarEnabled: %d\n", extraGameInfo [1].bRadarEnabled);
	PrintLog (0, "bPowerupsOnRadar: %d\n", extraGameInfo [1].bPowerupsOnRadar);
	PrintLog (0, "nZoomMode: %d\n", extraGameInfo [1].nZoomMode);
	PrintLog (0, "bRobotsHitRobots: %d\n", extraGameInfo [1].bRobotsHitRobots);
	PrintLog (0, "bAutoDownload: %d\n", extraGameInfo [1].bAutoDownload);
	PrintLog (0, "bAutoBalanceTeams: %d\n", extraGameInfo [1].bAutoBalanceTeams);
	PrintLog (0, "bDualMissileLaunch: %d\n", extraGameInfo [1].bDualMissileLaunch);
	PrintLog (0, "bRobotsOnRadar: %d\n", extraGameInfo [1].bRobotsOnRadar);
	PrintLog (0, "grWallTransparency: %d\n", extraGameInfo [1].grWallTransparency);
	PrintLog (0, "nSpeedBoost: %d\n", extraGameInfo [1].nSpeedBoost);
	PrintLog (0, "bDropAllMissiles: %d\n", extraGameInfo [1].bDropAllMissiles);
	PrintLog (0, "bImmortalPowerups: %d\n", extraGameInfo [1].bImmortalPowerups);
	PrintLog (0, "bUseCameras: %d\n", extraGameInfo [1].bUseCameras);
	PrintLog (0, "nFusionRamp: %d\n", extraGameInfo [1].nFusionRamp);
	PrintLog (0, "nOmegaRamp: %d\n", extraGameInfo [1].nOmegaRamp);
	PrintLog (0, "bWiggle: %d\n", extraGameInfo [1].bWiggle);
	PrintLog (0, "bMultiBosses: %d\n", extraGameInfo [1].bMultiBosses);
	PrintLog (0, "nBossCount: %d\n", extraGameInfo [1].nBossCount);
	PrintLog (0, "bSmartWeaponSwitch: %d\n", extraGameInfo [1].bSmartWeaponSwitch);
	PrintLog (0, "bFluidPhysics: %d\n", extraGameInfo [1].bFluidPhysics);
	PrintLog (0, "nWeaponDropMode: %d\n", extraGameInfo [1].nWeaponDropMode);
	PrintLog (0, "bDarkness: %d\n", extraGameInfo [1].bDarkness);
	PrintLog (0, "bPowerupLights: %d\n", extraGameInfo [1].bPowerupLights);
	PrintLog (0, "bBrightObjects: %d\n", extraGameInfo [1].bBrightObjects);
	PrintLog (0, "bTeamDoors: %d\n", extraGameInfo [1].bTeamDoors);
	PrintLog (0, "bEnableCheats: %d\n", extraGameInfo [1].bEnableCheats);
	PrintLog (0, "bRotateLevels: %d\n", extraGameInfo [1].bRotateLevels);
	PrintLog (0, "bDisableReactor: %d\n", extraGameInfo [1].bDisableReactor);
	PrintLog (0, "bMouseLook: %d\n", extraGameInfo [1].bMouseLook);
	PrintLog (0, "nWeaponIcons: %d\n", extraGameInfo [1].nWeaponIcons);
	PrintLog (0, "bSafeUDP: %d\n", extraGameInfo [1].bSafeUDP);
	PrintLog (0, "bFastPitch: %d\n", extraGameInfo [1].bFastPitch);
	PrintLog (0, "bUseParticles: %d\n", extraGameInfo [1].bUseParticles);
	PrintLog (0, "bUseLightning: %d\n", extraGameInfo [1].bUseLightning);
	PrintLog (0, "bDamageExplosions: %d\n", extraGameInfo [1].bDamageExplosions);
	PrintLog (0, "bThrusterFlames: %d\n", extraGameInfo [1].bThrusterFlames);
	PrintLog (0, "bShadows: %d\n", extraGameInfo [1].bShadows);
	PrintLog (0, "nShieldEffect: %d\n", extraGameInfo [1].nShieldEffect);
	PrintLog (0, "bTeleporterCams: %d\n", extraGameInfo [1].bTeleporterCams);
	PrintLog (0, "bEnableCheats: %d\n", extraGameInfo [1].bEnableCheats);
	PrintLog (0, "bTargetIndicators: %d\n", extraGameInfo [1].bTargetIndicators);
	PrintLog (0, "bDamageIndicators: %d\n", extraGameInfo [1].bDamageIndicators);
	PrintLog (0, "bFriendlyIndicators: %d\n", extraGameInfo [1].bFriendlyIndicators);
	PrintLog (0, "bCloakedIndicators: %d\n", extraGameInfo [1].bCloakedIndicators);
	PrintLog (0, "bMslLockIndicators: %d\n", extraGameInfo [1].bMslLockIndicators);
	PrintLog (0, "bTagOnlyHitObjs: %d\n", extraGameInfo [1].bTagOnlyHitObjs);
	PrintLog (0, "bTowFlags: %d\n", extraGameInfo [1].bTowFlags);
	PrintLog (0, "bUseHitAngles: %d\n", extraGameInfo [1].bUseHitAngles);
	PrintLog (0, "bLightTrails: %d\n", extraGameInfo [1].bLightTrails);
	PrintLog (0, "bTracers: %d\n", extraGameInfo [1].bTracers);
	PrintLog (0, "bShockwaves: %d\n", extraGameInfo [1].bShockwaves);
	PrintLog (0, "bSmokeGrenades: %d\n", extraGameInfo [1].bSmokeGrenades);
	PrintLog (0, "nMaxSmokeGrenades: %d\n", extraGameInfo [1].nMaxSmokeGrenades);
	PrintLog (0, "bCompetition: %d\n", extraGameInfo [1].bCompetition);
	PrintLog (0, "bFlickerLights: %d\n", extraGameInfo [1].bFlickerLights);
	PrintLog (0, "bCheckUDPPort: %d\n", extraGameInfo [1].bCheckUDPPort);
	PrintLog (0, "bSmokeGrenades: %d\n", extraGameInfo [1].bSmokeGrenades);
	PrintLog (0, "nWeaponTurnSpeed: %d\n", extraGameInfo [1].nWeaponTurnSpeed);
	PrintLog (0, "nMslStartSpeed: %d\n", extraGameInfo [1].nMslStartSpeed);
	PrintLog (0, "nCoopPenalty: %d\n", (int32_t) nCoopPenalties [(int32_t) extraGameInfo [1].nCoopPenalty]);
	PrintLog (0, "bKillMissiles: %d\n", extraGameInfo [1].bKillMissiles);
	PrintLog (0, "bTripleFusion: %d\n", extraGameInfo [1].bTripleFusion);
	PrintLog (0, "bShowWeapons: %d\n", extraGameInfo [1].bShowWeapons);
	PrintLog (0, "bAllowCustomWeapons: %d\n", extraGameInfo [1].bAllowCustomWeapons);
	PrintLog (0, "bEnhancedShakers: %d\n", extraGameInfo [1].bEnhancedShakers);
	PrintLog (0, "nHitboxes: %d\n", extraGameInfo [1].nHitboxes);
	PrintLog (0, "nRadar: %d\n", extraGameInfo [1].nRadar);
	PrintLog (0, "nDrag: %d\n", extraGameInfo [1].nDrag);
	PrintLog (0, "nSpotSize: %d\n", extraGameInfo [1].nSpotSize);
	PrintLog (0, "nSpotStrength: %d\n", extraGameInfo [1].nSpotStrength);
	PrintLog (0, "nLightRange: %d\n", extraGameInfo [1].nLightRange);
	PrintLog (0, "nSpeedScale: %d.%d\n", 5 * (extraGameInfo [1].nSpeedScale + 2) / 10, 5 * (extraGameInfo [1].nSpeedScale + 2) % 10);
	PrintLog (0, "entropy info data:\n");
	PrintLog (0, "nEnergyFillRate: %d\n", extraGameInfo [1].entropy.nEnergyFillRate);
	PrintLog (0, "nShieldFillRate: %d\n", extraGameInfo [1].entropy.nShieldFillRate);
	PrintLog (0, "nShieldDamageRate: %d\n", extraGameInfo [1].entropy.nShieldDamageRate);
	PrintLog (0, "nMaxVirusCapacity: %d\n", extraGameInfo [1].entropy.nMaxVirusCapacity); 
	PrintLog (0, "nBumpVirusCapacity: %d\n", extraGameInfo [1].entropy.nBumpVirusCapacity);
	PrintLog (0, "nBashVirusCapacity: %d\n", extraGameInfo [1].entropy.nBashVirusCapacity); 
	PrintLog (0, "nVirusGenTime: %d\n", extraGameInfo [1].entropy.nVirusGenTime); 
	PrintLog (0, "nVirusLifespan: %d\n", extraGameInfo [1].entropy.nVirusLifespan); 
	PrintLog (0, "nVirusStability: %d\n", extraGameInfo [1].entropy.nVirusStability);
	PrintLog (0, "nCaptureVirusThreshold: %d\n", extraGameInfo [1].entropy.nCaptureVirusThreshold); 
	PrintLog (0, "nCaptureTimeThreshold: %d\n", extraGameInfo [1].entropy.nCaptureTimeThreshold); 
	PrintLog (0, "bRevertRooms: %d\n", extraGameInfo [1].entropy.bRevertRooms);
	PrintLog (0, "bDoCaptureWarning: %d\n", extraGameInfo [1].entropy.bDoCaptureWarning);
	PrintLog (0, "nOverrideTextures: %d\n", extraGameInfo [1].entropy.nOverrideTextures);
	PrintLog (0, "bBrightenRooms: %d\n", extraGameInfo [1].entropy.bBrightenRooms);
	PrintLog (0, "bPlayerHandicap: %d\n", extraGameInfo [1].entropy.bPlayerHandicap);
	PrintLog (0, "headlight.bAvailable: %d\n", extraGameInfo [1].headlight.bAvailable);
	PrintLog (0, "headlight.bDrainPower: %d\n", extraGameInfo [1].headlight.bDrainPower);
	PrintLog (-1);
	}
}

//------------------------------------------------------------------------------

