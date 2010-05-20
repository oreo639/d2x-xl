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
#include "scores.h"
#include "joydefs.h"
#include "playsave.h"
#include "kconfig.h"
#include "credits.h"
#include "texmap.h"
#include "state.h"
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

#if VR_NONE
#   undef VR_NONE			//undef if != 0
#endif
#ifndef VR_NONE
#   define VR_NONE 0		//make sure VR_NONE is defined and 0 here
#endif

//------------------------------------------------------------------------------

static struct {
	int	nWideScreen;
	int	nCustom;
} screenResOpts;

//------------------------------------------------------------------------------

static int ScreenResMenuItemToMode (int menuItem)
{
	int j;

if ((screenResOpts.nWideScreen >= 0) && (menuItem > screenResOpts.nWideScreen))
	menuItem--;
for (j = 0; j < NUM_DISPLAY_MODES; j++)
	if (displayModeInfo [j].bAvailable)
		if (--menuItem < 0)
			break;
return j;
}

//------------------------------------------------------------------------------

static int ScreenResModeToMenuItem (int mode)
{
	int item = 0;

for (int j = 0; j < mode; j++)
	if (displayModeInfo [j].bAvailable)
		item++;
if ((screenResOpts.nWideScreen >= 0) && (mode >= screenResOpts.nWideScreen))
	item++;
return item;
}

//------------------------------------------------------------------------------

int ScreenResCallback (CMenu& menu, int& nKey, int nCurItem, int nState)
{
if (nState)
	return nCurItem;

	int	i, j;

if (menu [screenResOpts.nCustom].m_value != (gameStates.video.nDisplayMode == CUSTOM_DISPLAY_MODE)) 
	nKey = -2;
for (i = 0; i < screenResOpts.nCustom; i++)
	if (menu [i].m_value) {
		j = ScreenResMenuItemToMode (i);
		if ((j < NUM_DISPLAY_MODES) && (j != gameStates.video.nDisplayMode)) {
			//SetDisplayMode (j, 0);
			nKey = -2;
			}
		break;
		}
return nCurItem;
}

//------------------------------------------------------------------------------

int SwitchDisplayMode (int dir)
{
	int	i, h = NUM_DISPLAY_MODES;

for (i = 0; i < h; i++)
	displayModeInfo [i].bAvailable =
		 ((i < 2) || gameStates.menus.bHiresAvailable) && GrVideoModeOK (displayModeInfo [i].dim);
i = gameStates.video.nDisplayMode;
do {
	i += dir;
	if (i < 0)
		i = 0;
	else if (i >= h)
		i = h - 1;
	if (displayModeInfo [i].bAvailable) {
		SetDisplayMode (i, 0);
		return 1;
		}
	} while (i && (i < h - 1) && (i != gameStates.video.nDisplayMode));
return 0;
}

//------------------------------------------------------------------------------

void ScreenResMenu (void)
{
#	define N_SCREENRES_ITEMS (NUM_DISPLAY_MODES + 4)

	CMenu		m;
	int		choice;
	int		i, j, key, nCustWOpt, nCustHOpt, nCustW, nCustH, bStdRes;
	char		szMode [40];
	char		cShortCut, szCustX [5], szCustY [5];

if ((gameStates.video.nDisplayMode == -1) || (gameStates.render.vr.nRenderMode != VR_NONE)) {				//special VR mode
	MsgBox (TXT_SORRY, NULL, 1, TXT_OK, 
			"You may not change screen\nresolution when VR modes enabled.");
	return;
	}

do {
	screenResOpts.nWideScreen = -1;
	cShortCut = '1';
	m.Destroy ();
	m.Create (N_SCREENRES_ITEMS);
	for (i = 0, j = NUM_DISPLAY_MODES - 1; i < j; i++) {
		if (!(displayModeInfo [i].bAvailable = 
				 ((i < 2) || gameStates.menus.bHiresAvailable) && GrVideoModeOK (displayModeInfo [i].dim)))
				continue;
		if (displayModeInfo [i].bWideScreen && !displayModeInfo [i-1].bWideScreen) {
			m.AddText (TXT_WIDESCREEN_RES, 0);
			m.NewGroup (-1);
			if (screenResOpts.nWideScreen < 0)
				screenResOpts.nWideScreen = m.ToS () - 1;
			}
		if ((displayModeInfo [i].w == 640) && (displayModeInfo [i].h == 480))
			sprintf (szMode, "%c. %dx%d (NTSC)", cShortCut, displayModeInfo [i].w, displayModeInfo [i].h);
		else if ((displayModeInfo [i].w == 720) && (displayModeInfo [i].h == 576))
			sprintf (szMode, "%c. %dx%d (PAL/SECAM)", cShortCut, displayModeInfo [i].w, displayModeInfo [i].h);
		else if ((displayModeInfo [i].w == 1280) && (displayModeInfo [i].h == 720))
			sprintf (szMode, "%c. %dx%d (720p)", cShortCut, displayModeInfo [i].w, displayModeInfo [i].h);
		else if ((displayModeInfo [i].w == 1920) && (displayModeInfo [i].h == 1080))
			sprintf (szMode, "%c. %dx%d (1080p)", cShortCut, displayModeInfo [i].w, displayModeInfo [i].h);
		else
			sprintf (szMode, "%c. %dx%d", cShortCut, displayModeInfo [i].w, displayModeInfo [i].h);
		if (cShortCut == '9')
			cShortCut = 'A';
		else
			cShortCut++;
		m.AddRadio (szMode, 0, -1);
		}

	screenResOpts.nCustom = m.AddRadio (TXT_CUSTOM_SCRRES, 0, KEY_U, HTX_CUSTOM_SCRRES);
	*szCustX = *szCustY = '\0';
	if (displayModeInfo [CUSTOM_DISPLAY_MODE].w)
		sprintf (szCustX, "%d", displayModeInfo [CUSTOM_DISPLAY_MODE].w);
	if (displayModeInfo [CUSTOM_DISPLAY_MODE].h)
		sprintf (szCustY, "%d", displayModeInfo [CUSTOM_DISPLAY_MODE].h);
	nCustWOpt = m.AddInput (szCustX, 4, NULL);
	nCustHOpt = m.AddInput (szCustY, 4, NULL);

	choice = ScreenResModeToMenuItem (gameStates.video.nDisplayMode);
	m [choice].m_value = 1;

	key = m.Menu (NULL, TXT_SELECT_SCRMODE, ScreenResCallback, &choice);
	if (key == -1)
		return;
	bStdRes = 0;
	if (m [screenResOpts.nCustom].m_value) {
		key = -2;
		if ((nCustWOpt > 0) && (nCustHOpt > 0) &&
			 (0 < (nCustW = atoi (szCustX))) && (0 < (nCustH = atoi (szCustY)))) {
			i = CUSTOM_DISPLAY_MODE;
			if (SetCustomDisplayMode (nCustW, nCustH))
				key = 0;
			else
				MsgBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_ERROR_SCRMODE);
			}
		else
			continue;
		}
	else 
		{
		for (i = 0; i <= screenResOpts.nCustom; i++)
			if (m [i].m_value) {
				bStdRes = 1;
				i = ScreenResMenuItemToMode(i);
				break;
				}
			if (!bStdRes)
				continue;
		}
	if (((i > 1) && !gameStates.menus.bHiresAvailable) || !GrVideoModeOK (displayModeInfo [i].dim)) {
		MsgBox (TXT_SORRY, NULL, 1, TXT_OK, TXT_ERROR_SCRMODE);
		return;
		}
	if (i != gameStates.video.nDisplayMode) {
		SetDisplayMode (i, 0);
		SetScreenMode (SCREEN_MENU);
		if (bStdRes)
			return;
		}
	}
	while (key == -2);
}

//------------------------------------------------------------------------------
//eof
