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
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include "descent.h"

//-----------------------------------------------------------------------------

int FusionBump (void)
{
if (gameData.fusion.xAutoFireTime) {
	if (gameData.weapons.nPrimary != FUSION_INDEX)
		gameData.fusion.xAutoFireTime = 0;
	else if (gameData.time.xGame + gameData.fusion.xFrameTime / 2 >= gameData.fusion.xAutoFireTime) {
		gameData.fusion.xAutoFireTime = 0;
		gameData.laser.nGlobalFiringCount = 1;
		}
	else {
		static time_t t0 = 0;

		time_t t = gameStates.app.nSDLTicks;
		if (t - t0 < 30)
			return 0;
		t0 = t;
		gameData.laser.nGlobalFiringCount = 0;
		gameData.objs.consoleP->RandomBump (I2X (1) / 8, (gameData.fusion.xCharge > I2X (2)) ? gameData.fusion.xCharge * 4 : I2X (4));
		}
	}
return 1;
}

//	-----------------------------------------------------------------------------
//	Fire Laser:  Registers a laser fire, and performs special stuff for the fusion
//				    cannon.
void ChargeFusion (void)
{
if ((LOCALPLAYER.energy < I2X (2)) && (gameData.fusion.xAutoFireTime == 0)) {
	gameData.laser.nGlobalFiringCount = 0;
	}
else {
	gameData.fusion.xFrameTime += gameData.time.xFrame;
	if (!gameData.fusion.xCharge)
		LOCALPLAYER.energy -= I2X (2);
	fix h = (gameData.fusion.xFrameTime <= LOCALPLAYER.energy) ? gameData.fusion.xFrameTime : LOCALPLAYER.energy;
	gameData.fusion.xCharge += h;
	LOCALPLAYER.energy -= h;
	if (LOCALPLAYER.energy > 0) 
		gameData.fusion.xAutoFireTime = gameData.time.xGame + gameData.fusion.xFrameTime / 2 + 1;
	else {
		LOCALPLAYER.energy = 0;
		gameData.fusion.xAutoFireTime = gameData.time.xGame - 1;	//	Fire now!
		}
	if (gameStates.limitFPS.bFusion && !gameStates.app.tick40fps.bTick)
		return;

	float fScale = float (gameData.fusion.xCharge >> 11) / 64.0f;
	tRgbaColorf* colorP = gameData.weapons.color + FUSION_ID;

	if (gameData.fusion.xCharge < I2X (2)) 
		paletteManager.BumpEffect (colorP->red * fScale, colorP->green * fScale, colorP->blue * fScale);
	else 
		paletteManager.BumpEffect (colorP->blue * fScale, colorP->red * fScale, colorP->green * fScale);
	if (gameData.time.xGame < gameData.fusion.xLastSoundTime)		//gametime has wrapped
		gameData.fusion.xNextSoundTime = gameData.fusion.xLastSoundTime = gameData.time.xGame;
	if (gameData.fusion.xNextSoundTime < gameData.time.xGame) {
		if (gameData.fusion.xCharge > I2X (2)) {
			audio.PlaySound (11);
			gameData.objs.consoleP->ApplyDamageToPlayer (gameData.objs.consoleP, d_rand () * 4);
			}
		else {
			CreateAwarenessEvent (gameData.objs.consoleP, WEAPON_ROBOT_COLLISION);
			audio.PlaySound (SOUND_FUSION_WARMUP);
			if (gameData.app.nGameMode & GM_MULTI)
				MultiSendPlaySound (SOUND_FUSION_WARMUP, I2X (1));
				}
		gameData.fusion.xLastSoundTime = gameData.time.xGame;
		gameData.fusion.xNextSoundTime = gameData.time.xGame + I2X (1) / 8 + d_rand () / 4;
		}
	gameData.fusion.xFrameTime = 0;
	}
}

//-----------------------------------------------------------------------------
//eof
