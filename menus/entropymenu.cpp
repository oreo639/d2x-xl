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
#include "menubackground.h"
#include "netmenu.h"

#define LHX(x)      (gameStates.menus.bHires?2* (x):x)
#define LHY(y)      (gameStates.menus.bHires? (24* (y))/10:y)

//------------------------------------------------------------------------------

void NetworkEntropyToggleOptions (void)
{
	CMenu	m (12);
	int	optPlrHand;

m.AddCheck (TXT_ENT_HANDICAP, extraGameInfo [0].entropy.bPlayerHandicap, KEY_H, HTX_ONLINE_MANUAL, "player handicap");
m.AddCheck (TXT_ENT_CONQWARN, extraGameInfo [0].entropy.bDoCaptureWarning, KEY_W, HTX_ONLINE_MANUAL, "capture warning");
m.AddCheck (TXT_ENT_REVERT, extraGameInfo [0].entropy.bRevertRooms, KEY_R, HTX_ONLINE_MANUAL, "revert rooms");
m.AddText ("");
m.AddText (TXT_ENT_VIRSTAB, -1, "virus stability");
m.AddRadio (TXT_ENT_VIRSTAB_DROP, 0, KEY_D);
m.AddRadio (TXT_ENT_VIRSTAB_ENEMY, 0, KEY_T);
m.AddRadio (TXT_ENT_VIRSTAB_TOUCH, 0, KEY_L);
m.AddRadio (TXT_ENT_VIRSTAB_NEVER, 0, KEY_N);
m [m.IndexOf ("virus stability") + extraGameInfo [0].entropy.nVirusStability].Value () = 1;

m.Menu (NULL, TXT_ENT_TOGGLES, NULL, 0);

extraGameInfo [0].entropy.bRevertRooms = m ["revert rooms"]->Value ();
extraGameInfo [0].entropy.bDoCaptureWarning = m ["capture warning"]->Value ();
extraGameInfo [0].entropy.bPlayerHandicap = m ["player handicap"]->Value ();
for (extraGameInfo [0].entropy.nVirusStability = 0; 
	  extraGameInfo [0].entropy.nVirusStability < 3; 
	  extraGameInfo [0].entropy.nVirusStability++)
	if (m [m.IndexOf ("virus stability") + extraGameInfo [0].entropy.nVirusStability].Value ())
		break;
}

//------------------------------------------------------------------------------

void NetworkEntropyTextureOptions (void)
{
	CMenu	m (7);

m.AddRadio (TXT_ENT_TEX_KEEP, 0, KEY_K, HTX_ONLINE_MANUAL, "texture override");
m.AddRadio (TXT_ENT_TEX_OVERRIDE, 0, KEY_O, HTX_ONLINE_MANUAL);
m.AddRadio (TXT_ENT_TEX_COLOR, 0, KEY_C, HTX_ONLINE_MANUAL);
m [optOvrTex + extraGameInfo [0].entropy.nOverrideTextures]->Value () = 1;
m.AddText ("");
m.AddCheck (TXT_ENT_TEX_BRIGHTEN, extraGameInfo [0].entropy.bBrightenRooms, KEY_B, HTX_ONLINE_MANUAL, "brighten rooms");

m.Menu (NULL, TXT_ENT_TEXTURES, NULL, 0);

extraGameInfo [0].entropy.bBrightenRooms = m ["brighten rooms"]->Value ();
for (extraGameInfo [0].entropy.nOverrideTextures = 0; 
	  extraGameInfo [0].entropy.nOverrideTextures < 3; 
	  extraGameInfo [0].entropy.nOverrideTextures++)
	if (m [m.IndexOf ("texture override") + extraGameInfo [0].entropy.nOverrideTextures].Value ())
		break;
}

//------------------------------------------------------------------------------

void NetworkEntropyOptions (void)
{
	CMenu		m (25);
	int		i, optTogglesMenu, optTextureMenu;
	char		szCapVirLim [10], szCapTimLim [10], szMaxVirCap [10], szBumpVirCap [10], 
				szBashVirCap [10], szVirGenTim [10], szVirLife [10], 
				szEnergyFill [10], szShieldFill [10], szShieldDmg [10];

m.AddInput (TXT_ENT_VIRUS_THRESHOLD, szCapVirLim, extraGameInfo [0].entropy.nCaptureVirusThreshold, 3, HTX_ONLINE_MANUAL, "capture virus threshold");
m.AddInput (TXT_ENT_CAPTURE_TIME, szCapTimLim, extraGameInfo [0].entropy.nCaptureTimeThreshold, 3, HTX_ONLINE_MANUAL, "capture time threshold");
m.AddInput (TXT_ENT_MAXCAP, szMaxVirCap, extraGameInfo [0].entropy.nMaxVirusCapacity, 6, HTX_ONLINE_MANUAL, "virus capacity");
m.AddInput (TXT_ENT_BUMPCAP, szBumpVirCap, extraGameInfo [0].entropy.nBumpVirusCapacity, 3, HTX_ONLINE_MANUAL, "virus capacity increase"); // cap. increase when scoring a kill
m.AddInput (TXT_ENT_BASHCAP, szBashVirCap, extraGameInfo [0].entropy.nBashVirusCapacity, 3, HTX_ONLINE_MANUAL, "virus capacity decrease"); // cap. decrease when being killed
m.AddInput (TXT_ENT_GENTIME, szVirGenTim, extraGameInfo [0].entropy.nVirusGenTime, 3, HTX_ONLINE_MANUAL, "virus creation delay");
m.AddInput (TXT_ENT_VIRLIFE, szVirLife, extraGameInfo [0].entropy.nVirusLifespan, 3, HTX_ONLINE_MANUAL, "virus lifetime");
m.AddInput (TXT_ENT_EFILL, szEnergyFill, extraGameInfo [0].entropy.nEnergyFillRate, 3, HTX_ONLINE_MANUAL, "refuel rate");
m.AddInput (TXT_ENT_SFILL, szShieldFill, extraGameInfo [0].entropy.nShieldFillRate, 3, HTX_ONLINE_MANUAL, "repair rate");
m.AddInput (TXT_ENT_DMGRATE, szShieldDmg, extraGameInfo [0].entropy.nShieldDamageRate, 3, HTX_ONLINE_MANUAL, "damage rate");
m.AddText ("");
m.AddMenu (TXT_ENT_TGLMENU, KEY_E, "toggle options");
m.AddMenu (TXT_ENT_TEXMENU, KEY_T, "texture options");

for (;;) {
	i = m.Menu (NULL, "Entropy Options", NULL, 0);
	if (i == optTogglesMenu)
		NetworkEntropyToggleOptions ();
	else if (i == optTextureMenu)
		NetworkEntropyTextureOptions ();
	else
		break;
	}
extraGameInfo [0].entropy.nCaptureVirusThreshold = (char) atol (m ["capture virus threshold"]->Text ());
extraGameInfo [0].entropy.nCaptureTimeThreshold = (char) atol (m ["capture time threshold"]->Text ());
extraGameInfo [0].entropy.nMaxVirusCapacity = (ushort) atol (m ["virus capacity"]->Text ());
extraGameInfo [0].entropy.nBumpVirusCapacity = (char) atol (m ["virus capacity increase"]->Text ());
extraGameInfo [0].entropy.nBashVirusCapacity = (char) atol (m ["virus capacity decrease"]->Text ());
extraGameInfo [0].entropy.nVirusGenTime = (char) atol (m ["virus creation time"]->Text ());
extraGameInfo [0].entropy.nVirusLifespan = (char) atol (m ["virus lifetime"]->Text ());
extraGameInfo [0].entropy.nEnergyFillRate = (ushort) atol (m ["refule rate"]->Text ());
extraGameInfo [0].entropy.nShieldFillRate = (ushort) atol (m ["repair rate"]->Text ());
extraGameInfo [0].entropy.nShieldDamageRate = (ushort) atol (m ["damage rage"]->Text ());
}

//------------------------------------------------------------------------------

