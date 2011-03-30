#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

#define PATCH12

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#	include <winsock.h>
#else
#	include <sys/socket.h>
#endif

#include "descent.h"
#include "strutil.h"
#include "args.h"
#include "timer.h"
#include "ipx.h"
#include "ipx_udp.h"
#include "menu.h"
#include "key.h"
#include "error.h"
#include "network.h"
#include "network_lib.h"
#include "menu.h"
#include "text.h"
#include "byteswap.h"
#include "netmisc.h"
#include "kconfig.h"
#include "autodl.h"
#include "tracker.h"
#include "gamefont.h"
#include "IP2Country.h"
#include "menubackground.h"

#define LHX(x)      (gameStates.menus.bHires ? 2 * (x) : x)
#define LHY(y)      (gameStates.menus.bHires? (24* (y)) / 10 : y)

#define AGI	activeNetGames [choice]
#define AXI activeExtraGameInfo [choice]

/* the following are the possible packet identificators.
 * they are stored in the "nType" field of the packet structs.
 * they are offset 4 bytes from the beginning of the raw IPX data
 * because of the "driver's" ipx_packetnum (see linuxnet.c).
 */

void SetFunctionMode (int);
extern ubyte ipx_MyAddress [10];

//------------------------------------------------------------------------------

#if 1
extern CNetGameInfo activeNetGames [];
extern tExtraGameInfo activeExtraGameInfo [];
#endif

char szHighlight [] = {1, (char) 255, (char) 192, (char) 128, 0};

#define FLAGTEXT(_b)	((_b) ? TXT_ON : TXT_OFF)

#define	INITFLAGS(_t) \
		 {sprintf (mTexts [opt], _t); strcat (mTexts [opt], szHighlight); j = 0;}

#define	ADDFLAG(_f,_t) \
	if (_f) {if (j) strcat (mTexts [opt], ", "); strcat (mTexts [opt], _t); if (++j == 5) {opt++; INITFLAGS ("   ")}; }

//------------------------------------------------------------------------------

void ShowNetGameInfo (int choice)
{
	CMenu	m (30);
   char	mTexts [30][200];
	int	i, j, nInMenu, opt = 0;

#if !DBG
if (choice >= networkData.nActiveGames)
	return;
#endif
memset (mTexts, 0, sizeof (mTexts));
sprintf (mTexts [opt], TXT_NGI_GAME, szHighlight, AGI.m_info.szGameName); 
opt++;
sprintf (mTexts [opt], TXT_NGI_MISSION, szHighlight, AGI.m_info.szMissionTitle); 
opt++;
sprintf (mTexts [opt], TXT_NGI_LEVEL, szHighlight, AGI.m_info.nLevel); 
opt++;
sprintf (mTexts [opt], TXT_NGI_SKILL, szHighlight, MENU_DIFFICULTY_TEXT (AGI.m_info.difficulty)); 
opt++;
opt++;
#if !DBG
if (!*AXI.szGameName) {
	sprintf (mTexts [opt], "Gamehost is not using D2X-XL or running in pure mode");
	opt++;
	}
else 
#endif
	{
	if (AXI.bShadows || AXI.bUseParticles || (!AXI.bCompetition && AXI.bUseLightning)) {
		INITFLAGS ("Graphics Fx: "); 
		ADDFLAG (AXI.bShadows, "Shadows");
		ADDFLAG (AXI.bUseParticles, "Smoke");
		if (!AXI.bCompetition)
			ADDFLAG (AXI.bUseLightning, "Lightning");
		}
	else
		strcpy (mTexts [opt], "Graphics Fx: None");
	opt++;
	if (!AXI.bCompetition && (AXI.bLightTrails || AXI.bTracers)) {
		INITFLAGS ("Weapon Fx: ");
		ADDFLAG (AXI.bLightTrails, "Light trails");
		ADDFLAG (AXI.bTracers, "Tracers");
		}
	else
		sprintf (mTexts [opt], "Weapon Fx: None");
	opt++;
	if (!AXI.bCompetition && (AXI.bDamageExplosions || AXI.bPlayerShield)) {
		INITFLAGS ("Ship Fx: ");
		ADDFLAG (AXI.bPlayerShield, "Shield");
		ADDFLAG (AXI.bDamageExplosions, "Damage");
		ADDFLAG (AXI.bShowWeapons, "Weapons");
		ADDFLAG (AXI.bGatlingSpeedUp, "Gatling speedup");
		}
	else
		sprintf (mTexts [opt], "Ship Fx: None");
	opt++;
	if (AXI.nWeaponIcons || (!AXI.bCompetition && (AXI.bTargetIndicators || AXI.bDamageIndicators))) {
		INITFLAGS ("HUD extensions: ");
		ADDFLAG (AXI.nWeaponIcons != 0, "Icons");
		ADDFLAG (!AXI.bCompetition && AXI.bTargetIndicators, "Tgt indicators");
		ADDFLAG (!AXI.bCompetition && AXI.bDamageIndicators, "Dmg indicators");
		ADDFLAG (!AXI.bCompetition && AXI.bMslLockIndicators, "Trk indicators");
		}
	else
		strcat (mTexts [opt], "HUD extensions: None");
	opt++;
	if (!AXI.bCompetition && AXI.bRadarEnabled) {
		INITFLAGS ("Radar: ");
		ADDFLAG ((AGI.m_info.gameFlags & NETGAME_FLAG_SHOW_MAP) != 0, "Players");
		ADDFLAG (AXI.nRadar, "Radar");
		ADDFLAG (AXI.bPowerupsOnRadar, "Powerups");
		ADDFLAG (AXI.bRobotsOnRadar, "Robots");
		}
	else
		strcat (mTexts [opt], "Radar: off");
	opt++;
	if (!AXI.bCompetition && (AXI.bMouseLook || AXI.bFastPitch)) {
		INITFLAGS ("Controls ext.: ");
		ADDFLAG (AXI.bMouseLook, "mouselook");
		ADDFLAG (AXI.bFastPitch, "fast pitch");
		}
	else
		strcat (mTexts [opt], "Controls ext.: None");
	opt++;
	if (!AXI.bCompetition && 
		 (AXI.bDualMissileLaunch || !AXI.bFriendlyFire || AXI.bInhibitSuicide || 
		  AXI.bEnableCheats || AXI.bDarkness || (AXI.nFusionRamp != 2))) {
		INITFLAGS ("Gameplay ext.: ");
		ADDFLAG (AXI.bEnableCheats, "Cheats");
		ADDFLAG (AXI.bDarkness, "Darkness");
		ADDFLAG (AXI.bSmokeGrenades, "Smoke Grens");
		ADDFLAG (AXI.bDualMissileLaunch, "Dual Msls");
		ADDFLAG (AXI.nFusionRamp != 2, "Fusion ramp");
		ADDFLAG (!AXI.bFriendlyFire, "no FF");
		ADDFLAG (AXI.bInhibitSuicide, "no suicide");
		ADDFLAG (AXI.bKillMissiles, "shoot msls");
		ADDFLAG (AXI.bTripleFusion, "tri fusion");
		ADDFLAG (AXI.bEnhancedShakers, "enh shakers");
		ADDFLAG (AXI.nHitboxes, "hit boxes");
		}
	else
		strcat (mTexts [opt], "Gameplay ext.: None");
	opt++;
	}
for (i = 0; i < opt; i++)
	m.AddText (reinterpret_cast<char*> (mTexts + i));
bAlreadyShowingInfo = 1;
nInMenu = gameStates.menus.nInMenu;
gameStates.menus.nInMenu = 0;
gameStates.menus.bNoBackground = 0;
m.TinyMenu (NULL, TXT_NETGAME_INFO);
gameStates.menus.bNoBackground = 0;
gameStates.app.bGameRunning = 0;
gameStates.menus.nInMenu = nInMenu;
bAlreadyShowingInfo = 0;
 }

//------------------------------------------------------------------------------

#undef AGI
#undef AXI

#define AGI netGame.m_info
#define AXI	extraGameInfo [0]

static int GraphicsFxCompMode (void)
{
return (!AXI.bCompetition && AXI.bUseLightning) ? 2 : (AXI.bShadows || AXI.bUseParticles) ? 1 : 0;
}

//------------------------------------------------------------------------------

static int WeaponFxCompMode (void)
{
return (!AXI.bCompetition && (AXI.bLightTrails || AXI.bTracers)) ? 2 : 0;
}

//------------------------------------------------------------------------------

static int ShipFxCompMode (void)
{
return (!AXI.bCompetition && (AXI.bPlayerShield || AXI.bDamageExplosions || AXI.bShowWeapons || AXI.bGatlingSpeedUp)) ? 2 : 0;
}

//------------------------------------------------------------------------------

static int HUDCompMode (void)
{
return (!AXI.bCompetition && (AXI.bTargetIndicators || AXI.bDamageIndicators || AXI.bMslLockIndicators)) ? 2 : (AXI.nWeaponIcons != 0) ? 1 : 0;
}

//------------------------------------------------------------------------------

static int RadarCompMode (void)
{
return (!AXI.bCompetition && AXI.bRadarEnabled) &&
		  (((AGI.gameFlags & NETGAME_FLAG_SHOW_MAP) != 0) || AXI.bPowerupsOnRadar || AXI.bRobotsOnRadar || (AXI.nRadar != 0)) ? 2 : 0;
}

//------------------------------------------------------------------------------

static int ControlsCompMode (void)
{
return (!AXI.bCompetition && (AXI.bMouseLook || AXI.bFastPitch)) ? 2 : 0;
}

//------------------------------------------------------------------------------

static int GameplayCompMode (void)
{
return (!AXI.bCompetition && (AXI.bEnableCheats || AXI.bDarkness || AXI.bSmokeGrenades || (AXI.nFusionRamp != 2) || !AXI.bFriendlyFire ||
										 AXI.bInhibitSuicide || AXI.bKillMissiles || AXI.bTripleFusion || AXI.bEnhancedShakers || AXI.nHitboxes)) ? 2 : 0;
}

//------------------------------------------------------------------------------
// 2: no competition mode
// 1: competition mode, some uncritical D2X-XL extensions enabled
// 0: full competition mode

static int CompetitionMode (void)
{
return GraphicsFxCompMode () | WeaponFxCompMode () | ShipFxCompMode () | HUDCompMode () | RadarCompMode () | ControlsCompMode() | GameplayCompMode ();
}

//------------------------------------------------------------------------------

char* XMLGameInfo (void)
{
	static char xmlGameInfo [UDP_PAYLOAD_SIZE];

	static const char* szGameType [2][9] = {
		{"Anarchy", "Anarchy", "Anarchy", "Coop", "CTF", "Hoard", "Hoard", "Monsterball", "Entropy"},
		{"Anarchy", "Anarchy", "Anarchy", "Coop", "CTF+", "Hoard", "Hoard", "Monsterball", "Entropy"}
		};
	static const char* szGameState [] = {"open", "closed", "restricted"};
	static const char* szCompMode [] = {"none", "basic", "critical"};

	int nExtensions;

sprintf (xmlGameInfo, "<?xml version=\"1.0\"?>\n<GameInfo>\n  <Descent>\n");

sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <Host Name=\"%s\" />\n",
			 gameData.multiplayer.players [gameData.multiplayer.nLocalPlayer].callsign);

sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <Mission Type=\"%s\" Name=\"%s\" Level=\"%d\" \n",
			 gameStates.app.bD1Mission ? "D1" : gameStates.app.bD2XLevel ? "D2X" : "D2", AGI.szMissionTitle, AGI.nLevel);
if (IsCoopGame || (mpParams.nGameMode & NETGAME_ROBOT_ANARCHY))
	sprintf (xmlGameInfo + strlen (xmlGameInfo), "Difficulty=\"%d\" ", AGI.difficulty);
sprintf (xmlGameInfo + strlen (xmlGameInfo), "/>\n");

sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <Player Current=\"%d\" Max=\"%d\" />\n",
			 AGI.nNumPlayers, AGI.nMaxPlayers);

sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <Mode Type=\"%s\" Team=\"%d\" Robots=\"%d\" />\n",
			szGameType [(int) extraGameInfo [0].bEnhancedCTF][(int) mpParams.nGameMode],
			(mpParams.nGameMode != NETGAME_ANARCHY) && (mpParams.nGameMode != NETGAME_ROBOT_ANARCHY) && (mpParams.nGameMode != NETGAME_HOARD),
			(mpParams.nGameMode == NETGAME_ROBOT_ANARCHY) || (mpParams.nGameMode == NETGAME_COOPERATIVE));

sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <Status>%s</Status>\n",
			(AGI.nNumPlayers == AGI.nMaxPlayers)
			? "full"
			: (networkData.nStatus == NETSTAT_STARTING)
				? "forming"
				: szGameState [mpParams.nGameAccess]);
strcat (xmlGameInfo, "  </Descent>\n");

sprintf (xmlGameInfo + strlen (xmlGameInfo), "  <Server Type=\"D2X-XL\" Version=\"%s\" Extensions=\"%s\" />\n",
			 VERSION, szCompMode [nExtensions = CompetitionMode ()]);

if (nExtensions) {
	sprintf (xmlGameInfo + strlen (xmlGameInfo), "  <Extensions>\n");

	sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <GraphicsFx Shadows=\"%d\" Smoke=\"%d\" Lightning=\"%d\" />\n",
				AXI.bShadows,
				AXI.bUseParticles,
				!AXI.bCompetition && AXI.bUseLightning);

	sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <WeaponFx LightTrails=\"%d\" Tracers=\"%d\" />\n",
				!AXI.bCompetition && AXI.bLightTrails,
				!AXI.bCompetition && AXI.bTracers);

	sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <ShipFx Shield=\"%d\" Damage=\"%d\" Weapons=\"%d\" GatlingSpeedup=\"%d\" />\n",
				!AXI.bCompetition && AXI.bPlayerShield,
				!AXI.bCompetition && AXI.bDamageExplosions,
				!AXI.bCompetition && AXI.bShowWeapons,
				!AXI.bCompetition && AXI.bGatlingSpeedUp);

	sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <HUD Icons=\"%d\" TargetInd=\"%d\" DamageInd=\"%d\" LockInd=\"%d\" />\n",
				AXI.nWeaponIcons != 0,
				!AXI.bCompetition && AXI.bTargetIndicators,
				!AXI.bCompetition && AXI.bDamageIndicators,
				!AXI.bCompetition && AXI.bMslLockIndicators);

	sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <Radar Players=\"%d\" Powerups=\"%d\" Robots=\"%d\"  HUD=\"%d\" />\n",
				!AXI.bCompetition && AXI.bRadarEnabled && ((AGI.gameFlags & NETGAME_FLAG_SHOW_MAP) != 0),
				!AXI.bCompetition && AXI.bRadarEnabled && AXI.bPowerupsOnRadar,
				!AXI.bCompetition && AXI.bRadarEnabled && AXI.bRobotsOnRadar,
				!AXI.bCompetition && AXI.bRadarEnabled && (AXI.nRadar != 0));

	sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <Controls MouseLook=\"%d\" FastPitch=\"%d\" />\n",
				!AXI.bCompetition && AXI.bMouseLook,
				!AXI.bCompetition && AXI.bFastPitch);

	sprintf (xmlGameInfo + strlen (xmlGameInfo), "    <GamePlay Cheats=\"%d\" Darkness=\"%d\" SmokeGrenades=\"%d\" FusionRamp=\"%d\" FriendlyFire=\"%d\" Suicide=\"%d\" KillMissiles=\"%d\" TriFusion=\"%d\" BetterShakers=\"%d\" HitBoxes=\"%d\" />\n",
				!AXI.bCompetition && AXI.bEnableCheats,
				!AXI.bCompetition && AXI.bDarkness,
				!AXI.bCompetition && AXI.bSmokeGrenades,
				!AXI.bCompetition && (AXI.nFusionRamp != 2),
				AXI.bCompetition || AXI.bFriendlyFire,
				AXI.bCompetition || !AXI.bInhibitSuicide,
				!AXI.bCompetition && AXI.bKillMissiles,
				!AXI.bCompetition && AXI.bTripleFusion,
				!AXI.bCompetition && AXI.bEnhancedShakers,
				!AXI.bCompetition && AXI.nHitboxes);
				//!AXI.bCompetition && AXI.bDualMissileLaunch,

	strcat (xmlGameInfo, "  </Extensions>\n");
	}
strcat (xmlGameInfo, "</GameInfo>\n");

PrintLog ("\nXML game info:\n\n");
PrintLog (xmlGameInfo);
PrintLog ("\n");
return xmlGameInfo;
}

//------------------------------------------------------------------------------

char* XMLGameStatus (void)
{
	static char xmlGameStatus [UDP_PAYLOAD_SIZE];

sprintf (xmlGameStatus, "<?xml version=\"1.0\"?>\n<GameStatus>\n  <Descent>\n");
sprintf (xmlGameStatus + strlen (xmlGameStatus), "    <PlayerCount=%d>\n", gameData.multiplayer.nPlayers);
for (int i = 0; i < gameData.multiplayer.nPlayers; i++) {
	sprintf (xmlGameStatus + strlen (xmlGameStatus), "    <Player%d ping=");
	if (pingStats [i].ping < 0)
		strcat (xmlGameStatus, "n/a");
	else
		sprintf (xmlGameStatus + strlen (xmlGameStatus), "\"%d\"", pingStats [i].ping);
	sprintf (xmlGameStatus + strlen (xmlGameStatus), " score=\"%d\" kills=\"%d\" deaths=\"%d\" country=\"%s\"/>\n", 
				gameData.multiplayer.players [i].score,
				gameData.multiplayer.players [i].netKillsTotal,
				gameData.multiplayer.players [i].netKilledTotal,
				CountryFromIP (*((int*) gameData.multiplayer.players [i].netAddress)));
	}
strcat (xmlGameStatus, "  </Descent>\n</GameStatus>\n");
PrintLog ("\nXML game status:\n\n");
PrintLog (xmlGameStatus);
PrintLog ("\n");
return xmlGameStatus;
}

//------------------------------------------------------------------------------

void HandleXMLGameStatusRequest (void)
{
if (networkData.xmlGameStatusRequestTime <= 0) {
	networkData.xmlGameStatusRequestTime = SDL_GetTicks ();
	for (int i = 0; i < gameData.multiplayer.nPlayers; i++) {
		pingStats [i].ping = -1;
		pingStats [i].launchTime = -networkData.xmlGameStatusRequestTime; // negative value suppresses display of returned ping on HUD
		NetworkSendPing (i);
		}
	}
else if (SDL_GetTicks () - networkData.xmlGameStatusRequestTime > 1500) {
	}
}

//------------------------------------------------------------------------------
