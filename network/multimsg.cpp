#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "descent.h"
#include "u_mem.h"
#include "timer.h"
#include "strutil.h"
#include "network.h"
#include "error.h"
#include "byteswap.h"
#include "key.h"
#include "banlist.h"
#include "text.h"
#include "playerprofile.h"
#include "multimsg.h"
#include "network_lib.h"

//-----------------------------------------------------------------------------

fix xPingReturnTime;

extern int32_t WhoIsGameHost ();
extern char bNameReturning;

//-----------------------------------------------------------------------------

void MultiSendMessage (void)
{
	int32_t bufP = 0;

if (gameData.multigame.msg.nReceiver != -1) {
	gameData.multigame.msg.buf [bufP++] = (char)MULTI_MESSAGE;            
	gameData.multigame.msg.buf [bufP++] = (char)N_LOCALPLAYER;                       
	strncpy ((char*) gameData.multigame.msg.buf + bufP, gameData.multigame.msg.szMsg, MAX_MESSAGE_LEN); 
	bufP += MAX_MESSAGE_LEN;
	gameData.multigame.msg.buf [bufP-1] = '\0';
	MultiSendData (gameData.multigame.msg.buf, bufP, 0);
	gameData.multigame.msg.nReceiver = -1;
	}
}

//-----------------------------------------------------------------------------

void MultiDefineMacro (int32_t key)
{
int32_t nMsg = 0;
#if !DBG
if (!(gameOpts->multi.bUseMacros && IsMultiGame))
	return;
#endif
key &= (~KEY_SHIFTED);
switch (key) {
	case KEY_F9:
		nMsg = 1; 
		break;
	case KEY_F10:
		nMsg = 2; 
		break;
	case KEY_F11:
		nMsg = 3; 
		break;
	case KEY_F12:
		nMsg = 4; 
		break;
	default:
		Int3 ();
	}
if (nMsg)  
	MultiSendMsgStart ((char) nMsg);
}

//-----------------------------------------------------------------------------

char szFeedbackResult [200];

int32_t MultiMessageFeedback (void)
{
	int32_t bFound = 0;
	int32_t i, l;

char *colon = strrchr (gameData.multigame.msg.szMsg, ':');
if (!colon)
	return 0;
l = (int32_t) (colon - gameData.multigame.msg.szMsg);
if (!l || (l > CALLSIGN_LEN))
	return 0;
sprintf (szFeedbackResult, "%s ", TXT_MESSAGE_SENT_TO);
if (IsTeamGame && (atoi (gameData.multigame.msg.szMsg) > 0) && 
	 (atoi (gameData.multigame.msg.szMsg) < 3)) {
	sprintf (szFeedbackResult+strlen (szFeedbackResult), "%s '%s'", 
				TXT_TEAM, netGameInfo.m_info.szTeamName [atoi (gameData.multigame.msg.szMsg)-1]);
	bFound = 1;
	}
if (!bFound)
	if (IsTeamGame) {
		for (i = 0; i < gameData.multiplayer.nPlayers; i++) {
			if (!strnicmp (netGameInfo.m_info.szTeamName [i], gameData.multigame.msg.szMsg, l)) {
				if (bFound)
					strcat (szFeedbackResult, ", ");
				bFound++;
				if (!(bFound % 4))
					strcat (szFeedbackResult, "\n");
				sprintf (szFeedbackResult+strlen (szFeedbackResult), "%s '%s'", 
							TXT_TEAM, netGameInfo.m_info.szTeamName [i]);
				}
			}
		}
if (!bFound)
	for (i = 0; i < gameData.multiplayer.nPlayers; i++) {
		if ((!strnicmp (gameData.multiplayer.players [i].callsign, gameData.multigame.msg.szMsg, l)) && 
			(i != N_LOCALPLAYER) && 
			(gameData.multiplayer.players [i].connected)) {
			if (bFound)
				strcat (szFeedbackResult, ", ");
			bFound++;
			if (!(bFound % 4))
				strcat (szFeedbackResult, "\n");
			sprintf (szFeedbackResult+strlen (szFeedbackResult), "%s", 
						gameData.multiplayer.players [i].callsign);
			}
		}
if (!bFound)
	strcat (szFeedbackResult, TXT_NOBODY);
else
	strcat (szFeedbackResult, ".");
audio.PlaySound (SOUND_HUD_MESSAGE);
Assert (strlen (szFeedbackResult) < 200);
HUDInitMessage (szFeedbackResult);
return 1;
}

//-----------------------------------------------------------------------------

void MultiSendMacro (int32_t key)
{
if (!(gameOpts->multi.bUseMacros && IsMultiGame))
	return;
switch (key) {
	case KEY_F9:
		key = 0; 
		break;
	case KEY_F10:
		key = 1; 
		break;
	case KEY_F11:
		key = 2; 
		break;
	case KEY_F12:
		key = 3; 
		break;
	default:
		Int3 ();
	}
if (!gameData.multigame.msg.szMacro [key][0]) {
	HUDInitMessage (TXT_NO_MACRO);
	return;
	}
strcpy (gameData.multigame.msg.szMsg, gameData.multigame.msg.szMacro [key]);
gameData.multigame.msg.nReceiver = 100;
HUDInitMessage ("%s '%s'", TXT_SENDING, gameData.multigame.msg.szMsg);
MultiMessageFeedback ();
}


//-----------------------------------------------------------------------------

void MultiDoStartTyping (uint8_t* buf)
{
gameStates.multi.bPlayerIsTyping [int32_t (buf [1])] = 1;
}

//-----------------------------------------------------------------------------

void MultiDoQuitTyping (uint8_t* buf)
{
gameStates.multi.bPlayerIsTyping [int32_t (buf [1])] = 0;
}

//-----------------------------------------------------------------------------

void MultiSendTyping (void)
{
if (gameStates.multi.bPlayerIsTyping [N_LOCALPLAYER]) {
	if (gameStates.app.nSDLTicks [0] - gameData.multigame.nTypingTimeout > 1000) {
		gameData.multigame.nTypingTimeout = gameStates.app.nSDLTicks [0];
		gameData.multigame.msg.buf [0] = (char) MULTI_START_TYPING;
		gameData.multigame.msg.buf [1] = N_LOCALPLAYER; 
		gameData.multigame.msg.buf [2] = gameData.multigame.msg.bSending;
		MultiSendData (gameData.multigame.msg.buf, 3, 0);
		}
	}
}

//-----------------------------------------------------------------------------

void MultiSendMsgStart (char nMsg)
{
if (IsMultiGame) {
	if (nMsg > 0)
		gameData.multigame.msg.bDefining = nMsg;
	else
		gameData.multigame.msg.bSending = -nMsg;
	gameData.multigame.msg.nIndex = 0;
	gameData.multigame.msg.szMsg [gameData.multigame.msg.nIndex] = 0;
	gameStates.multi.bPlayerIsTyping [N_LOCALPLAYER] = 1;
	gameData.multigame.nTypingTimeout = 0;
	MultiSendTyping ();
	}
}

//-----------------------------------------------------------------------------

void MultiSendMsgQuit (void)
{
gameData.multigame.msg.bSending = 0;
gameData.multigame.msg.bDefining = 0;
gameData.multigame.msg.nIndex = 0;
gameStates.multi.bPlayerIsTyping [N_LOCALPLAYER] = 0;
gameData.multigame.msg.buf [0] = (char) MULTI_QUIT_TYPING;
gameData.multigame.msg.buf [1] = N_LOCALPLAYER; 
gameData.multigame.msg.buf [2] = 0;
MultiSendData (gameData.multigame.msg.buf, 3, 0);
}

//-----------------------------------------------------------------------------

int32_t KickPlayer (int32_t bBan)
{
	int32_t i, name_index = 5 - bBan;
	const char *pszKick = GT (589 + bBan);

if (strlen (gameData.multigame.msg.szMsg) > 5)
	while (gameData.multigame.msg.szMsg [name_index] == ' ')
		name_index++;

if (!IAmGameHost ()) {
	HUDInitMessage (TXT_KICK_RIGHTS, gameData.multiplayer.players [WhoIsGameHost ()].callsign, pszKick);
	MultiSendMsgQuit ();
	return 1;
	}
if (strlen (gameData.multigame.msg.szMsg) <= (size_t) name_index) {
	HUDInitMessage (TXT_KICK_NAME, pszKick);
	MultiSendMsgQuit ();
	return 1;
	}

if (gameData.multigame.msg.szMsg [name_index] == '#' && ::isdigit (gameData.multigame.msg.szMsg [name_index+1])) {
	int32_t players [MAX_PLAYERS];
	int32_t listpos = gameData.multigame.msg.szMsg [name_index+1] - '0';

	if (gameData.multigame.score.bShowList == 1 || gameData.multigame.score.bShowList == 2) {
		if (listpos == 0 || listpos  >= gameData.multiplayer.nPlayers) {
			HUDInitMessage (TXT_KICK_PLR, pszKick);
			MultiSendMsgQuit ();
			return 1;
			}
		MultiGetKillList (players);
		i = players [listpos];
		if ((i != N_LOCALPLAYER) && (gameData.multiplayer.players [i].connected))
			goto kick_player;
		}
	else 
		HUDInitMessage (TXT_KICK_NUMBER, pszKick);
	MultiSendMsgQuit ();
	return 1;
	}

for (i = 0; i < gameData.multiplayer.nPlayers; i++)
	if ((!strnicmp (gameData.multiplayer.players [i].callsign, &gameData.multigame.msg.szMsg [name_index], strlen (gameData.multigame.msg.szMsg)-name_index)) && (i != N_LOCALPLAYER) && (gameData.multiplayer.players [i].connected)) {
kick_player:;
		if (gameStates.multi.nGameType  >= IPX_GAME)
			NetworkDumpPlayer (netPlayers [0].m_info.players [i].network.Network (), netPlayers [0].m_info.players [i].network.Node (), DUMP_KICKED);

		HUDInitMessage (TXT_DUMPING, gameData.multiplayer.players [i].callsign);
		if (bBan)
			banList.Add (gameData.multiplayer.players [i].callsign);
		MultiSendMsgQuit ();
		return 1;
		}
return 0;
}

//-----------------------------------------------------------------------------

int32_t PingPlayer (int32_t i)
{
if (IsNetworkGame) {
	if (i >= 0) {
		pingStats [i].launchTime = SDL_GetTicks (); //TimerGetFixedSeconds ();
		NetworkSendPing ((uint8_t) i);
		MultiSendMsgQuit ();
		pingStats [i].sent++;
		}
	else {
		int32_t name_index = 5;
		if (strlen (gameData.multigame.msg.szMsg) > 5)
			while (gameData.multigame.msg.szMsg [name_index] == ' ')
				name_index++;
		if (strlen (gameData.multigame.msg.szMsg) <= (size_t) name_index) {
			HUDInitMessage (TXT_PING_NAME);
			return 1;
			}
		for (i = 0; i < gameData.multiplayer.nPlayers; i++) {
			if ((!strnicmp (gameData.multiplayer.players [i].callsign, &gameData.multigame.msg.szMsg [name_index], strlen (gameData.multigame.msg.szMsg)-name_index)) && 
				 (i != N_LOCALPLAYER) && (gameData.multiplayer.players [i].connected)) {
				pingStats [i].launchTime = SDL_GetTicks (); //TimerGetFixedSeconds ();
				NetworkSendPing ((uint8_t) i);
				HUDInitMessage (TXT_PINGING, gameData.multiplayer.players [i].callsign);
				MultiSendMsgQuit ();
				return 1;
				}
			}
		}
	}
else {// Modem/Serial ping
	pingStats [0].launchTime = SDL_GetTicks (); //TimerGetFixedSeconds ();
	MultiSendModemPing ();
	HUDInitMessage (TXT_PING_OTHER);
	MultiSendMsgQuit ();
	return 1;
	}
return 0;
}

//-----------------------------------------------------------------------------

int32_t HandicapPlayer (void)
{
	char *mytempbuf = gameData.multigame.msg.szMsg + 9;

gameStates.gameplay.xInitialShield [0] = atol (mytempbuf);
if (gameStates.gameplay.xInitialShield [0] < 10) {
	gameStates.gameplay.xInitialShield [0] = 10;
	sprintf (gameData.multigame.msg.szMsg, TXT_NEW_HANDICAP, LOCALPLAYER.callsign, gameStates.gameplay.xInitialShield [0]);
	}
else if (gameStates.gameplay.xInitialShield [0] > 100) {
	sprintf (gameData.multigame.msg.szMsg, TXT_CHEAT_ALERT, LOCALPLAYER.callsign);
	gameStates.gameplay.xInitialShield [0] = 100;
	}
else
	sprintf (gameData.multigame.msg.szMsg, TXT_NEW_HANDICAP, LOCALPLAYER.callsign, gameStates.gameplay.xInitialShield [0]);
HUDInitMessage (TXT_HANDICAP_ALERT, gameStates.gameplay.xInitialShield [0]);
gameStates.gameplay.xInitialShield [0] = I2X (gameStates.gameplay.xInitialShield [0]);
return 0;
}

//-----------------------------------------------------------------------------

int32_t MovePlayer (void)
{
	int32_t	i;

if (IsNetworkGame && IsTeamGame) {
	int32_t name_index = 5;
	if (strlen (gameData.multigame.msg.szMsg) > 5)
		while (gameData.multigame.msg.szMsg [name_index] == ' ')
			name_index++;

	if (!IAmGameHost ()) {
		HUDInitMessage (TXT_MOVE_RIGHTS, gameData.multiplayer.players [WhoIsGameHost ()].callsign);
		return 1;
		}
	if (strlen (gameData.multigame.msg.szMsg) <= (size_t) name_index) {
		HUDInitMessage (TXT_MOVE_NAME);
		return 1;
		}
	for (i = 0; i < gameData.multiplayer.nPlayers; i++)
		if ((!strnicmp (gameData.multiplayer.players [i].callsign, &gameData.multigame.msg.szMsg [name_index], strlen (gameData.multigame.msg.szMsg)-name_index)) && (gameData.multiplayer.players [i].connected)) {
			if ((gameData.app.GameMode (GM_CAPTURE)) && (gameData.multiplayer.players [i].flags & PLAYER_FLAGS_FLAG)) {
				HUDInitMessage (TXT_MOVE_FLAG);
				return 1;
				}
			SetTeam (i, -1);
			break;
		}
	}
return 0;
}

//-----------------------------------------------------------------------------

void MultiSendMsgEnd ()
{
gameData.multigame.msg.nReceiver = 100;
if (!strnicmp (gameData.multigame.msg.szMsg, TXT_NAMES_OFF, 6)) {
	bNameReturning = 1-bNameReturning;
	HUDInitMessage (TXT_NAMERET, bNameReturning? TXT_NR_ACTIVE : TXT_NR_DISABLED);
	}
else if (!strnicmp (gameData.multigame.msg.szMsg, TXT_HANDICAP, 9)) {
	if (HandicapPlayer ())
		return;
	}
else if (!strnicmp (gameData.multigame.msg.szMsg, TXT_BOMBS_OFF, 7))
	netGameInfo.m_info.DoSmartMine = 0;
else if (!(gameStates.render.cockpit.bShowPingStats || strnicmp (gameData.multigame.msg.szMsg, TXT_PING, 5))) {
	if (PingPlayer (-1))
		return;
	}
else if (!strnicmp (gameData.multigame.msg.szMsg, TXT_MOVE, 5)) {
	if (MovePlayer ())
		return;
	}
else if (!strnicmp (gameData.multigame.msg.szMsg, TXT_KICK, 5) && IsNetworkGame) {
	if (KickPlayer (0))
		return;
	}
else if (!strnicmp (gameData.multigame.msg.szMsg, TXT_BAN, 4) && IsNetworkGame) {
	if (KickPlayer (1))
		return;
	}
else
	HUDInitMessage ("%s '%s'", TXT_SENDING, gameData.multigame.msg.szMsg);
MultiSendMessage ();
MultiMessageFeedback ();
MultiSendMsgQuit ();
}

//-----------------------------------------------------------------------------

void MultiDefineMacroEnd ()
{
Assert (gameData.multigame.msg.bDefining > 0);
strcpy (gameData.multigame.msg.szMacro [gameData.multigame.msg.bDefining-1], gameData.multigame.msg.szMsg);
SavePlayerProfile ();
MultiSendMsgQuit ();
}

//-----------------------------------------------------------------------------

void MultiMsgInputSub (int32_t key)
{
switch (key) {
	case KEY_F8:
	case KEY_ESC:
		MultiSendMsgQuit ();
		GameFlushInputs ();
		break;

	case KEY_LEFT:
	case KEY_BACKSPACE:
	case KEY_PAD4:
		if (gameData.multigame.msg.nIndex > 0)
			gameData.multigame.msg.nIndex--;
		gameData.multigame.msg.szMsg [gameData.multigame.msg.nIndex] = 0;
		break;

	case KEY_ENTER:
		if (gameData.multigame.msg.bSending)
			MultiSendMsgEnd ();
		else if (gameData.multigame.msg.bDefining)
			MultiDefineMacroEnd ();
		GameFlushInputs ();
		break;

	default:
		if (key > 0) {
			int32_t ascii = KeyToASCII (key);
			if (ascii < 255) {
				if (gameData.multigame.msg.nIndex < MAX_MESSAGE_LEN-2) {
					gameData.multigame.msg.szMsg [gameData.multigame.msg.nIndex++] = ascii;
					gameData.multigame.msg.szMsg [gameData.multigame.msg.nIndex] = 0;
					}
				else if (gameData.multigame.msg.bSending) {
					int32_t i;
					char * ptext, *pcolon;
					ptext = NULL;
					gameData.multigame.msg.szMsg [gameData.multigame.msg.nIndex++] = ascii;
					gameData.multigame.msg.szMsg [gameData.multigame.msg.nIndex] = 0;
					for (i = gameData.multigame.msg.nIndex-1; i >= 0; i--) {
						if (gameData.multigame.msg.szMsg [i] == 32) {
							ptext = &gameData.multigame.msg.szMsg [i+1];
							gameData.multigame.msg.szMsg [i] = 0;
							break;
							}
						}
					MultiSendMsgEnd ();
					if (ptext) {
						gameData.multigame.msg.bSending = 1;
						pcolon = strchr (gameData.multigame.msg.szMsg, ':');
						if (pcolon)
							strcpy (pcolon+1, ptext);
						else
							strcpy (gameData.multigame.msg.szMsg, ptext);
						gameData.multigame.msg.nIndex = (int32_t) strlen (gameData.multigame.msg.szMsg);
						}
					}
				}
			}
	}
}

//-----------------------------------------------------------------------------

void MultiSendMsgDialog (void)
{
	CMenu	m (1);
	int32_t	choice;

if (!IsMultiGame)
	return;
gameData.multigame.msg.szMsg [0] = 0;             // Get rid of old contents
m.AddInput ("", gameData.multigame.msg.szMsg, MAX_MESSAGE_LEN - 1);
choice = m.Menu (NULL, TXT_SEND_MESSAGE);
if ((choice > -1) && (strlen (gameData.multigame.msg.szMsg) > 0)) {
	gameData.multigame.msg.nReceiver = 100;
	HUDInitMessage ("%s '%s'", TXT_SENDING, gameData.multigame.msg.szMsg);
	MultiMessageFeedback ();
	}
}

//-----------------------------------------------------------------------------

static int32_t IsTeamId (char *bufP, int32_t nLen)
{
	int32_t	i;

if (!IsTeamGame)
	return 0;
i = atoi (bufP);
if ((i >= 1) && (i <= 2))
	return 1;
for (i = 0; i < 2; i++)
	if (!strnicmp (netGameInfo.m_info.szTeamName [i], bufP, nLen))
		return 1;
return 0;
}

//-----------------------------------------------------------------------------

static int32_t IsMyTeamId (char *bufP, int32_t nLen)
{
	int32_t	i;

if (!IsTeamGame)
	return 0;
i = GetTeam (N_LOCALPLAYER);
if (i == atoi (bufP) - 1)
	return 1;
if (!strnicmp (netGameInfo.m_info.szTeamName [i], bufP, nLen))
	return 1;
return 0;
}

//-----------------------------------------------------------------------------

static int32_t IsPlayerId (char *bufP, int32_t nLen)
{
	int32_t	i;

for (i = 0; i < gameData.multiplayer.nPlayers; i++)
	if (!strnicmp (gameData.multiplayer.players [i].callsign, bufP, nLen))
		return 1;
return 0;
}

//-----------------------------------------------------------------------------

static int32_t IsMyPlayerId (char *bufP, int32_t nLen)
{
return strnicmp (LOCALPLAYER.callsign, bufP, nLen) == 0;
}

//-----------------------------------------------------------------------------

void MultiDoMsg (uint8_t* buf)
{
	char *colon;
	char *tilde, msgBuf [200];
	int32_t tloc, t, l;
	int32_t bufP = 2;

if ((tilde = strchr ((char*) buf + bufP, '$'))) { 
	tloc = (int32_t) (tilde - ((char*) buf + bufP));			
	if (tloc > 0)
		strncpy (msgBuf, (char*) buf + bufP, tloc);
	strcpy (msgBuf + tloc, LOCALPLAYER.callsign);
	strcpy (msgBuf + strlen (LOCALPLAYER.callsign) + tloc, (char*) buf + bufP + tloc + 1);
	strcpy ((char*) buf + bufP, msgBuf);
	}
if ((colon = strrchr ((char*) buf + bufP, ':'))) {	//message may be addressed to a certain team or CPlayerData
	l = (int32_t) (colon - ((char*) buf + bufP));
	if (l && (l <= CALLSIGN_LEN) &&
		 ((IsTeamId ((char*) buf + bufP, l) && !IsMyTeamId ((char*) buf + bufP, l)) ||
		  (IsPlayerId ((char*) buf + bufP, l) && !IsMyPlayerId ((char*) buf + bufP, l))))
		return;
	}
msgBuf [0] = (char) 1;
msgBuf [1] = (char) (127 + 128);
msgBuf [2] = (char) (95 + 128);
msgBuf [3] = (char) (0 + 128);
strcpy (msgBuf + 4, gameData.multiplayer.players [int32_t (buf [1])].callsign);
t = (int32_t) strlen (msgBuf);
msgBuf [t] = ':';
msgBuf [t+1] = ' ';
msgBuf [t+2] = (char) 1;
msgBuf [t+3] = (char) (95 + 128);
msgBuf [t+4] = (char) (63 + 128);
msgBuf [t+5] = (char) (0 + 128);
msgBuf [t+6] = (char) 0;
audio.PlaySound (SOUND_HUD_MESSAGE);
HUDPlayerMessage ("%s %s", msgBuf, buf + 2);
}

//-----------------------------------------------------------------------------
//eof
