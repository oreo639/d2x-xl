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
#include "gamecntl.h"
#include "menu.h"
#include "network.h"
#include "network_lib.h"
#include "netmenu.h"
#include "scores.h"
#include "joydefs.h"
#include "playerprofile.h"
#include "kconfig.h"
#include "credits.h"
#include "texmap.h"
#include "savegame.h"
#include "movie.h"
#include "gamepal.h"
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

//------------------------------------------------------------------------------

#define WEAPON_ICONS 0

typedef struct tWindowOpts {
	int	nPos;
	int	nSize;
	int	nAlign;
	int	nZoom;
	int	nType [2];
} tWindowOpts;

typedef struct tRadarOpts {
	int	nPos;
	int	nSize;
	int	nRange;
} tRadarOpts;

static struct {
	tWindowOpts	windows;
	tRadarOpts	radar;
	int			nTgtInd;
} cockpitOpts;

static int nWindowPos, nWindowAlign, nTgtInd;

#if WEAPON_ICONS
static int	optWeaponIcons, bShowWeaponIcons, optIconAlpha;
#endif

static const char* szWindowType [7];
static const char* szWindowSize [4];
static const char* szWindowPos [2];
static const char* szWindowAlign [3];
static const char* szTgtInd [3];
static const char* szRadarSize [3];
static const char* szRadarRange [5];

static int nWinFuncs, winFuncs [CV_FUNC_COUNT];

//------------------------------------------------------------------------------

int CockpitOptionsCallback (CMenu& menu, int& key, int nCurItem, int nState)
{
if (nState)
	return nCurItem;

	CMenuItem*	m;
	int			v;

#if WEAPON_ICONS
mat = menu + optWeaponIcons;
dir = mat->m_value;
if (dir != bShowWeaponIcons) {
	bShowWeaponIcons = dir;
	key = -2;
	return nCurItem;
	}
#endif

for (int i = 0; i < 2; i++) {
	m = menu + cockpitOpts.windows.nType [i];
	v = winFuncs [m->m_value];
	if (v != gameStates.render.cockpit.n3DView [i]) {
		gameStates.render.cockpit.n3DView [i] = v;
		sprintf (m->m_text, GT (1163 + i), szWindowType [m->m_value]);
		m->m_bRebuild = 1;
		}
	}

m = menu + cockpitOpts.windows.nSize;
v = m->m_value;
if (gameOpts->render.cockpit.nWindowSize != v) {
	gameOpts->render.cockpit.nWindowSize = v;
	m->SetText (szWindowSize [v]);
	sprintf (m->m_text, TXT_AUXWIN_SIZE, szWindowSize [v]);
	m->m_bRebuild = 1;
	}

m = menu + cockpitOpts.windows.nZoom;
v = m->m_value;
if (gameOpts->render.cockpit.nWindowZoom != v) {
	gameOpts->render.cockpit.nWindowZoom = v;
	sprintf (m->m_text, TXT_AUXWIN_ZOOM, gameOpts->render.cockpit.nWindowZoom + 1);
	m->m_bRebuild = 1;
	}

m = menu + cockpitOpts.windows.nPos;
v = m->m_value;
if (nWindowPos != v) {
	nWindowPos = v;
	sprintf (m->m_text, TXT_AUXWIN_POSITION, szWindowPos [v]);
	m->m_bRebuild = 1;
	}

m = menu + cockpitOpts.windows.nAlign;
v = m->m_value;
if (nWindowAlign != v) {
	nWindowAlign = v;
	sprintf (m->m_text, TXT_AUXWIN_ALIGNMENT, szWindowAlign [v]);
	m->m_bRebuild = 1;
	}

m = menu + cockpitOpts.radar.nPos;
v = m->m_value;
if (gameOpts->render.cockpit.nRadarPos != v) {
	gameOpts->render.cockpit.nRadarPos = v;
	sprintf (m->m_text, TXT_RADAR_POSITION, szWindowPos [v]);
	m->m_bRebuild = 1;
	}

m = menu + cockpitOpts.radar.nSize;
v = m->m_value;
if (gameOpts->render.cockpit.nRadarSize != v) {
	gameOpts->render.cockpit.nRadarSize = v;
	sprintf (m->m_text, TXT_RADAR_SIZE, szRadarSize [v]);
	m->m_bRebuild = 1;
	}

m = menu + cockpitOpts.radar.nRange;
v = m->m_value;
if (gameOpts->render.cockpit.nRadarRange != v) {
	gameOpts->render.cockpit.nRadarRange = v;
	sprintf (m->m_text, TXT_RADAR_RANGE, szRadarRange [v]);
	m->m_bRebuild = 1;
	}

m = menu + cockpitOpts.nTgtInd;
v = m->m_value;
if (nTgtInd != v) {
	nTgtInd = v;
	sprintf (m->m_text, TXT_TARGET_INDICATORS, szTgtInd [v]);
	m->m_bRebuild = 1;
	}

return nCurItem;
}

//------------------------------------------------------------------------------

static void InitStrings (void)
{
	static bool bInitialized = false;

if (bInitialized)
	return;
bInitialized = true;

szWindowSize [0] = TXT_AUXWIN_SMALL;
szWindowSize [1] = TXT_AUXWIN_MEDIUM;
szWindowSize [2] = TXT_AUXWIN_LARGE;
szWindowSize [3] = TXT_AUXWIN_HUGE;

szWindowPos [0] = TXT_POS_BOTTOM;
szWindowPos [1] = TXT_POS_TOP;

szWindowAlign [0] = TXT_ALIGN_CORNERS;
szWindowAlign [1] = TXT_ALIGN_MIDDLE;
szWindowAlign [2] = TXT_ALIGN_CENTER;

szRadarSize [0] = TXT_SMALL;
szRadarSize [1] = TXT_MEDIUM;
szRadarSize [2] = TXT_LARGE;

szRadarRange [0] = TXT_OFF;
szRadarRange [1] = TXT_SHORT;
szRadarRange [2] = TXT_MEDIUM;
szRadarRange [3] = TXT_LONG;
szRadarRange [4] = TXT_EXTREME;

szTgtInd [0] = TXT_NONE;
szTgtInd [1] = TXT_MISSILES;
szTgtInd [2] = TXT_FULL;

}

//------------------------------------------------------------------------------

void DefaultCockpitSettings (void);

void CockpitOptionsMenu (void)
{
	static int choice = 0;

	CMenu m;
	int	i;
	int	optTextGauges, optHUD, optReticle, optMissiles, optObjTally, optPosition, optAlignment, optZoomType = -1;

	char	szSlider [50];

nWinFuncs = GatherWindowFunctions (winFuncs);
for (i = 0; i < nWinFuncs; i++)
	szWindowType [i] = GT (1156 + winFuncs [i]);

InitStrings ();

nWindowPos = gameOpts->render.cockpit.nWindowPos / 3;
nWindowAlign = gameOpts->render.cockpit.nWindowPos % 3;

optPosition = optAlignment = cockpitOpts.windows.nSize = cockpitOpts.windows.nZoom = optTextGauges = -1;
#if WEAPON_ICONS
int optIconPos = -1;
optWeaponIcons = -1;
bShowWeaponIcons = (extraGameInfo [0].nWeaponIcons != 0);
#endif

nTgtInd = extraGameInfo [0].bMslLockIndicators ? extraGameInfo [0].bTargetIndicators ? 2 : 1 : 0;

do {
	m.Destroy ();
	m.Create (20);

	optHUD = m.AddCheck (TXT_SHOW_HUD, gameOpts->render.cockpit.bHUD, KEY_U, HTX_CPIT_SHOWHUD);
	optReticle = m.AddCheck (TXT_SHOW_RETICLE, gameOpts->render.cockpit.bReticle, KEY_S, HTX_CPIT_SHOWRETICLE);
	optMissiles = m.AddCheck (TXT_MISSILE_VIEW, gameOpts->render.cockpit.bMissileView, KEY_M, HTX_CPIT_MSLVIEW);
	optTextGauges = m.AddCheck (TXT_SHOW_GFXGAUGES, !gameOpts->render.cockpit.bTextGauges, KEY_G, HTX_CPIT_GFXGAUGES);
	optObjTally = m.AddCheck (TXT_OBJECT_TALLY, gameOpts->render.cockpit.bObjectTally, KEY_T, HTX_CPIT_OBJTALLY);
	optZoomType = m.AddCheck (TXT_ZOOM_SMOOTH, extraGameInfo [IsMultiGame].nZoomMode - 1, KEY_O, HTX_GPLAY_ZOOMSMOOTH);
#if 0
	cockpitOpts.nTgtInd = mat.AddCheck (TXT_TARGET_INDICATORS, extraGameInfo [0].bTargetIndicators, KEY_T, HTX_CPIT_TGTIND);
#else
	m.AddText ("", 0);
	sprintf (szSlider, TXT_TARGET_INDICATORS, szTgtInd [nTgtInd]);
	cockpitOpts.nTgtInd = m.AddSlider (szSlider, nTgtInd, 0, 2, KEY_T, HTX_CPIT_TGTIND);
#endif
#if WEAPON_ICONS
	mat.AddText ("", 0);
	optWeaponIcons = mat.AddCheck (TXT_SHOW_WEAPONICONS, bShowWeaponIcons, KEY_W, HTX_CPIT_WPNICONS);
	if (bShowWeaponIcons) {
		optIconPos = mat.AddRadio (TXT_WPNICONS_TOP, 0, KEY_I, HTX_CPIT_ICONPOS);
		mat.AddRadio (TXT_WPNICONS_BTM, 0, KEY_I, HTX_CPIT_ICONPOS);
		mat.AddRadio (TXT_WPNICONS_LRB, 0, KEY_I, HTX_CPIT_ICONPOS);
		mat.AddRadio (TXT_WPNICONS_LRT, 0, KEY_I, HTX_CPIT_ICONPOS);
		mat [optIconPos + NMCLAMP (extraGameInfo [0].nWeaponIcons - 1, 0, 3)].m_value = 1;
		}
	else
		optIconPos = -1;
#endif
	m.AddText ("", 0);
	i = m.AddText (TXT_COCKPIT_WINDOWS, 0);
	m [i].m_bCentered = 1;
	m.AddText ("", 0);

	for (i = 0; i < 2; i++) {
		sprintf (szSlider, GT (1163 + i), szWindowType [gameStates.render.cockpit.n3DView [i]]);
		cockpitOpts.windows.nType [i] = m.AddSlider (szSlider, gameStates.render.cockpit.n3DView [0], 0, nWinFuncs - 1, i ? KEY_R : KEY_L, HTX_CPIT_WINTYPE);
		}

	sprintf (szSlider, TXT_AUXWIN_SIZE, szWindowSize [gameOpts->render.cockpit.nWindowSize]);
	cockpitOpts.windows.nSize = m.AddSlider (szSlider, gameOpts->render.cockpit.nWindowSize, 0, 3, KEY_I, HTX_CPIT_WINSIZE);

	sprintf (szSlider, TXT_AUXWIN_ZOOM, gameOpts->render.cockpit.nWindowZoom + 1);
	cockpitOpts.windows.nZoom = m.AddSlider (szSlider, gameOpts->render.cockpit.nWindowZoom, 0, 3, KEY_Z, HTX_CPIT_WINZOOM);

	sprintf (szSlider, TXT_AUXWIN_POSITION, szWindowPos [nWindowPos]);
	cockpitOpts.windows.nPos = m.AddSlider (szSlider, nWindowPos, 0, 1, KEY_P, HTX_AUXWIN_POSITION);

	sprintf (szSlider, TXT_AUXWIN_ALIGNMENT, szWindowAlign [nWindowAlign]);
	cockpitOpts.windows.nAlign = m.AddSlider (szSlider, nWindowAlign, 0, 2, KEY_A, HTX_AUXWIN_ALIGNMENT);

	m.AddText ("", 0);

	sprintf (szSlider, TXT_RADAR_POSITION, szWindowPos [gameOpts->render.cockpit.nRadarPos]);
	cockpitOpts.radar.nPos = m.AddSlider (szSlider, gameOpts->render.cockpit.nRadarPos, 0, 1, KEY_O, HTX_CPIT_RADARPOS);

	sprintf (szSlider, TXT_RADAR_SIZE, szRadarSize [gameOpts->render.cockpit.nRadarSize]);
	cockpitOpts.radar.nSize = m.AddSlider (szSlider, gameOpts->render.cockpit.nRadarSize, 0, 2, KEY_O, HTX_CPIT_RADARSIZE);

	sprintf (szSlider, TXT_RADAR_RANGE, szRadarRange [gameOpts->render.cockpit.nRadarRange]);
	cockpitOpts.radar.nRange = m.AddSlider (szSlider, gameOpts->render.cockpit.nRadarRange, 0, 4, KEY_R, HTX_CPIT_RADARRANGE);

	do {
		i = m.Menu (NULL, TXT_COCKPIT_OPTS, &CockpitOptionsCallback, &choice);
		if (i < 0)
			break;
#if WEAPON_ICONS
		if (bShowWeaponIcons && (optIconPos >= 0)) {
			for (int j = 0; j < 4; j++)
				if (mat [optIconPos + j].m_value) {
					extraGameInfo [0].nWeaponIcons = j + 1;
					break;
					}
				}
#endif
	} while (i >= 0);

	GET_VAL (gameOpts->render.cockpit.bReticle, optReticle);
	GET_VAL (gameOpts->render.cockpit.bHUD, optHUD);
	GET_VAL (gameOpts->render.cockpit.bMissileView, optMissiles);
	GET_VAL (gameOpts->render.cockpit.bObjectTally, optObjTally);
	//GET_VAL (extraGameInfo [0].bTargetIndicators, cockpitOpts.nTgtInd);
	gameOpts->render.cockpit.bTextGauges = !m [optTextGauges].m_value;
	gameOpts->render.cockpit.nWindowPos = nWindowPos * 3 + nWindowAlign;
//if (gameOpts->app.bExpertMode)
	extraGameInfo [IsMultiGame].nZoomMode = m [optZoomType].m_value + 1;
#if WEAPON_ICONS
	if (bShowWeaponIcons) {
		if (optIconPos >= 0) {
			for (int j = 0; j < 4; j++)
				if (mat [optIconPos + j].m_value) {
					extraGameInfo [0].nWeaponIcons = j + 1;
					break;
					}
				}
		GET_VAL (gameOpts->render.weaponIcons.alpha, optIconAlpha);
		}
#endif
	} while (i == -2);

extraGameInfo [0].bMslLockIndicators = (nTgtInd > 0);
extraGameInfo [0].bTargetIndicators = (nTgtInd > 1);

DefaultCockpitSettings ();
}

//------------------------------------------------------------------------------
//eof
