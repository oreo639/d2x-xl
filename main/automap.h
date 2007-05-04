/* $Id: automap.h,v 1.4 2003/11/15 00:36:54 btb Exp $ */
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

#ifndef _AUTOMAP_H
#define _AUTOMAP_H

#include "player.h"

extern void DoAutomap(int key_code, int bRadar);
extern void AutomapClearVisited();
extern ubyte bAutomapVisited[MAX_SEGMENTS_D2X];
void DropBuddyMarker(tObject *objp);

#define AM_RENDER_PLAYERS			(!IsMultiGame || (gameData.app.nGameMode & (GM_TEAM | GM_MULTI_COOP)) || (netGame.gameFlags & NETGAME_FLAG_SHOW_MAP))
#define AM_RENDER_PLAYER(_i)		(!IsMultiGame || \
											 (gameData.app.nGameMode & GM_MULTI_COOP) || \
											 (netGame.gameFlags & NETGAME_FLAG_SHOW_MAP) || \
											 (GetTeam (gameData.multiplayer.nLocalPlayer) == GetTeam (_i)))
#define AM_RENDER_ROBOTS			EGI_FLAG (bRobotsOnRadar, 0, 1, 0)
#define AM_RENDER_POWERUPS			(EGI_FLAG (bPowerupsOnRadar, 0, 1, 0) && !IsMultiGame)
#endif
