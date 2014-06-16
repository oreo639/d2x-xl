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
#include "findfile.h"
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
#include "mouse.h"
#include "collide.h"
#include "objrender.h"
#include "sparkeffect.h"
#include "renderthreads.h"
#include "soundthreads.h"
#include "menubackground.h"
#include "songs.h"

//------------------------------------------------------------------------------ 

#define LHX(x) (gameStates.menus.bHires? 2 * (x) : x)
#define LHY(y) (gameStates.menus.bHires? (24 * (y)) / 10 : y)

//------------------------------------------------------------------------------ 
// Creates a message box with a brief notification in the message box' subtitle
// The message box offers several choices to pick from (like 'yes' / 'no' / 'maybe'

int _CDECL_ InfoBox (const char* pszTitle, pMenuCallback callback, int nWallpaper, int nChoices, ...)
{
	int				i;
	char*				format, * s;
	va_list			args;
	char				szSubTitle [MSGBOX_TEXT_SIZE];
	CMenu	mm;

if (!mm.Create (5))
	return - 1;

va_start (args, nChoices);
for (i = 0; i < nChoices; i++) {
	s = va_arg (args, char *);
	mm.AddMenu ("msgbox", s, - 1);
	}
format = va_arg (args, char*);
vsprintf (szSubTitle, format, args);
va_end (args);
Assert (strlen (szSubTitle) < MSGBOX_TEXT_SIZE);
return mm.Menu (pszTitle, szSubTitle, callback, NULL, BG_SUBMENU, nWallpaper);
}

//------------------------------------------------------------------------------ 
// Creates a message box with a longer text placed in static text menu entries
// of the message box's underlying menu
// The text box doesn't offer choices to pick from, only an 'ok' button
// since it's purpose is to display a longer text (e.g. a help text)

int _CDECL_ TextBox (const char* pszTitle, int nWallpaper, int nChoices, ...)
{
	int				h, i, l, bTiny, nInMenu;
	char				*format, *s;
	va_list			args;
	char				nm_text [MSGBOX_TEXT_SIZE];
	CMenu	mm;


if (!(nChoices && mm.Create (10)))
	return -1;
if ((bTiny = (nChoices < 0)))
	nChoices = -nChoices;
va_start (args, nChoices);
for (i = l = 0; i < nChoices; i++) {
	s = va_arg (args, char* );
	h = (int) strlen (s);
	if (l + h > MSGBOX_TEXT_SIZE)
		break;
	l += h;
	if (!(bTiny || i))
		mm.AddMenu ("msgbox", s, - 1);
	else {
		mm.AddText ("", s);
		mm.Item (i).m_bUnavailable = 1;
		}
	if (bTiny)
		mm.Item (i).m_bCentered = (i != 0);
	}
if (!bTiny) {
	format = va_arg (args, char* );
	vsprintf (nm_text, format, args);
	va_end (args);
	}
nInMenu = gameStates.menus.nInMenu;
gameStates.menus.nInMenu = 0;
i = bTiny 
	 ? mm.Menu (NULL, pszTitle, NULL, NULL, BG_SUBMENU, nWallpaper, LHX (340), -1, 1)
	 : mm.Menu (pszTitle, nm_text, NULL, NULL);
gameStates.menus.nInMenu = nInMenu;
return i;
}

//------------------------------------------------------------------------------ 
//added on 10/14/98 by Victor Rachels to attempt a fixedwidth font messagebox
int _CDECL_ FixedFontMsgBox (char* pszTitle, int nChoices, ...)
{
	int				i;
	char*				format;
	va_list			args;
	char*				s;
	char				szSubTitle [MSGBOX_TEXT_SIZE];
	CMenu	mm;

if (!mm.Create (5))
	return -1;

va_start (args, nChoices);
for (i = 0; i < nChoices; i++) {
	s = va_arg (args, char*);
	mm.AddMenu ("msgbox", s);
	}
format = va_arg (args, char* );
vsprintf (szSubTitle, format, args);
va_end (args);
Assert (strlen (szSubTitle) < MSGBOX_TEXT_SIZE);
return mm.FixedFontMenu (pszTitle, szSubTitle, NULL, NULL);
}
//end this section addition - Victor Rachels

//------------------------------------------------------------------------------
//eof
