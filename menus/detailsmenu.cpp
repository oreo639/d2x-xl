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
#include <string.h>

#include "menu.h"
#include "descent.h"
#include "ipx.h"
#include "key.h"
#include "iff.h"
#include "u_mem.h"
#include "error.h"
#include "screens.h"
#include "joy.h"
#include "slew.h"
#include "args.h"
#include "hogfile.h"
#include "newdemo.h"
#include "timer.h"
#include "text.h"
#include "gamefont.h"
#include "menu.h"
#include "network.h"
#include "network_lib.h"
#include "netmenu.h"
#include "game.h"
#include "scores.h"
#include "joydefs.h"
#include "playerprofile.h"
#include "kconfig.h"
#include "credits.h"
#include "texmap.h"
#include "savegame.h"
#include "movie.h"
#include "game.h"
#include "cockpit.h"
#include "strutil.h"
#include "reorder.h"
#include "rendermine.h"
#include "light.h"
#include "lightmap.h"
#include "autodl.h"
#include "tracker.h"
#include "omega.h"
#include "lightning.h"
#include "vers_id.h"
#include "input.h"
#include "collide.h"
#include "objrender.h"
#include "sparkeffect.h"
#include "renderthreads.h"
#include "soundthreads.h"
#include "menubackground.h"

// -----------------------------------------------------------------------------
// Set detail level based stuff.
// Note: Highest detail level (gameStates.render.detail.nLevel == NUM_DETAIL_LEVELS-1) is custom detail level.
int32_t InitDetailLevels (int32_t nDetailLevel)
{
if ((nDetailLevel < 0) || (nDetailLevel >= NUM_DETAIL_LEVELS))
	nDetailLevel = NUM_DETAIL_LEVELS - 1;
if (nDetailLevel < NUM_DETAIL_LEVELS - 1) {
	gameStates.render.detail.nRenderDepth = detailData.renderDepths [nDetailLevel];
	gameStates.render.detail.nMaxPerspectiveDepth = detailData.maxPerspectiveDepths [nDetailLevel];
	gameStates.render.detail.nMaxLinearDepth = detailData.maxLinearDepths [nDetailLevel];
	gameStates.render.detail.nMaxLinearDepthObjects = detailData.maxLinearDepthObjects [nDetailLevel];
	gameStates.render.detail.nMaxDebrisObjects = detailData.maxDebrisObjects [nDetailLevel];
	gameStates.render.detail.nMaxObjectsOnScreenDetailed = detailData.maxObjsOnScreenDetailed [nDetailLevel];
	gameData.models.nSimpleModelThresholdScale = detailData.simpleModelThresholdScales [nDetailLevel];
	audio.SetMaxChannels (detailData.nSoundChannels [nDetailLevel]);
	//      Set custom menu defaults.
	gameStates.render.detail.nObjectComplexity = nDetailLevel;
	gameStates.render.detail.nWallRenderDepth = nDetailLevel;
	gameStates.render.detail.nObjectDetail = nDetailLevel;
	gameStates.render.detail.nWallDetail = nDetailLevel;
	gameStates.render.detail.nDebrisAmount = nDetailLevel;
	gameStates.sound.nSoundChannels = nDetailLevel;
	gameStates.render.detail.nLevel = nDetailLevel;
	}
return nDetailLevel;
}

// -----------------------------------------------------------------------------

void DetailLevelMenu (void)
{
	int32_t		i, choice = gameStates.app.nDetailLevel;
	CMenu		m (8);

	char szMenuDetails [5][20];

for (i = 0; i < 5; i++) {
	sprintf (szMenuDetails [i], "%d. %s", i + 1, MENU_DETAIL_TEXT (i));
	m.AddMenu ("", szMenuDetails [i], 0, HTX_ONLINE_MANUAL);
	}
m.AddText ("", "");
m.AddMenu ("", MENU_DETAIL_TEXT (5), KEY_C, HTX_ONLINE_MANUAL);
m.AddCheck ("hires movies", TXT_HIRES_MOVIES, gameOpts->movies.bHires, KEY_S, HTX_ONLINE_MANUAL);
i = m.Menu (NULL, TXT_DETAIL_LEVEL, NULL, &choice);
if (i > -1) {
	switch (choice) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			gameStates.app.nDetailLevel = InitDetailLevels (choice);
			break;
		default:
			gameStates.app.nDetailLevel = 5;
			CustomDetailsMenu ();
			break;
		}
	}
gameOpts->movies.bHires = m [7].Value ();
}

// -----------------------------------------------------------------------------

int32_t CustomDetailsCallback (CMenu& m, int32_t& nLastKey, int32_t nCurItem, int32_t nState)
{
if (nState)
	return nCurItem;

	nCurItem = nCurItem;

gameStates.render.detail.nObjectComplexity = m [0].Value ();
gameStates.render.detail.nObjectDetail = m [1].Value ();
gameStates.render.detail.nWallDetail = m [2].Value ();
gameStates.render.detail.nWallRenderDepth = m [3].Value ();
gameStates.render.detail.nDebrisAmount = m [4].Value ();
if (!gameStates.app.bGameRunning)
	gameStates.sound.nSoundChannels = m [5].Value ();
return nCurItem;
}

// -----------------------------------------------------------------------------

void SetMaxCustomDetails (void)
{
gameStates.render.detail.nRenderDepth = detailData.renderDepths [DIFFICULTY_LEVEL_COUNT - 1];
gameStates.render.detail.nMaxPerspectiveDepth = detailData.maxPerspectiveDepths [DIFFICULTY_LEVEL_COUNT - 1];
gameStates.render.detail.nMaxLinearDepth = detailData.maxLinearDepths [DIFFICULTY_LEVEL_COUNT - 1];
gameStates.render.detail.nMaxDebrisObjects = detailData.maxDebrisObjects [DIFFICULTY_LEVEL_COUNT - 1];
gameStates.render.detail.nMaxObjectsOnScreenDetailed = detailData.maxObjsOnScreenDetailed [DIFFICULTY_LEVEL_COUNT - 1];
gameData.models.nSimpleModelThresholdScale = detailData.simpleModelThresholdScales [DIFFICULTY_LEVEL_COUNT - 1];
gameStates.render.detail.nMaxLinearDepthObjects = detailData.maxLinearDepthObjects [DIFFICULTY_LEVEL_COUNT - 1];
}

// -----------------------------------------------------------------------------

void InitCustomDetails (void)
{
gameStates.render.detail.nRenderDepth = detailData.renderDepths [gameStates.render.detail.nWallRenderDepth];
gameStates.render.detail.nMaxPerspectiveDepth = detailData.maxPerspectiveDepths [gameStates.render.detail.nWallDetail];
gameStates.render.detail.nMaxLinearDepth = detailData.maxLinearDepths [gameStates.render.detail.nWallDetail];
gameStates.render.detail.nMaxDebrisObjects = detailData.maxDebrisObjects [gameStates.render.detail.nDebrisAmount];
gameStates.render.detail.nMaxObjectsOnScreenDetailed = detailData.maxObjsOnScreenDetailed [gameStates.render.detail.nObjectComplexity];
gameData.models.nSimpleModelThresholdScale = detailData.simpleModelThresholdScales [gameStates.render.detail.nObjectComplexity];
gameStates.render.detail.nMaxLinearDepthObjects = detailData.maxLinearDepthObjects [gameStates.render.detail.nObjectDetail];
audio.SetMaxChannels (detailData.nSoundChannels [gameStates.sound.nSoundChannels]);
}

#define	DL_MAX	10

// -----------------------------------------------------------------------------

void CustomDetailsMenu (void)
{
	int32_t	i;
	CMenu m;

	static int32_t choice = 0;

do {
	m.Destroy ();
	m.Create (DL_MAX);
	m.AddSlider ("object complexity", TXT_OBJ_COMPLEXITY, gameStates.render.detail.nObjectComplexity, 0, DIFFICULTY_LEVEL_COUNT-1, 0, HTX_ONLINE_MANUAL);
	m.AddSlider ("object detail", TXT_OBJ_DETAIL, gameStates.render.detail.nObjectDetail, 0, DIFFICULTY_LEVEL_COUNT-1, 0, HTX_ONLINE_MANUAL);
	m.AddSlider ("wall detail", TXT_WALL_DETAIL, gameStates.render.detail.nWallDetail, 0, DIFFICULTY_LEVEL_COUNT-1, 0, HTX_ONLINE_MANUAL);
	m.AddSlider ("render depth", TXT_WALL_RENDER_DEPTH, gameStates.render.detail.nWallRenderDepth, 0, DIFFICULTY_LEVEL_COUNT-1, 0, HTX_ONLINE_MANUAL);
	m.AddSlider ("debris amount", TXT_DEBRIS_AMOUNT, gameStates.render.detail.nDebrisAmount, 0, DIFFICULTY_LEVEL_COUNT-1, 0, HTX_ONLINE_MANUAL);
	if (!gameStates.app.bGameRunning)
		m.AddSlider ("sound channels", TXT_SOUND_CHANNELS, gameStates.sound.nSoundChannels, 0, sizeofa (detailData.nSoundChannels) - 1, 0, HTX_ONLINE_MANUAL);
	m.AddText ("", TXT_LO_HI, 0);
	i = m.Menu (NULL, TXT_DETAIL_CUSTOM, CustomDetailsCallback, &choice);
} while (i > -1);
InitCustomDetails ();
}

//------------------------------------------------------------------------------
//eof
