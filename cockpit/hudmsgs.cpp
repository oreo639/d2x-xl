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
#include <stdlib.h>

#include "descent.h"
#include "u_mem.h"
#include "strutil.h"
#include "cockpit.h"
#include "error.h"
#include "newdemo.h"
#include "gamefont.h"
#include "screens.h"
#include "text.h"
#include "args.h"
#include "ogl_render.h"

void CopyBackgroundRect (int32_t left,int32_t top,int32_t right,int32_t bot);
char szDisplayedBackgroundMsg [2][HUD_MESSAGE_LENGTH] = {"",""};

int32_t nLastMsgYCrd = -1;
int32_t nLastMsgHeight = 6;
int32_t bMSGPlayerMsgs = 0;
int32_t bNoMsgRedundancy = 0;
int32_t nModexHUDMsgs;

// ----------------------------------------------------------------------------

void ClearBackgroundMessages (void)
{
#if 0 
	// Obsolete. You cannot shrink the game's output area below the screen or window size anymore in D2X-XL. 
	// That feature has been removed because today's hardware is powerful enough to render Descent,
	// and since you can run the program in windowed and fullscreen mode at lower than native screen resolution.
if (((gameStates.render.cockpit.nType == CM_STATUS_BAR) || (gameStates.render.cockpit.nType == CM_FULL_SCREEN)) && 
	  (nLastMsgYCrd != -1) && (gameData.render.scene.Top () >= 6)) {
	gameData.render.frame.Activate ("ClearBackgroundMessages (frame)");
	CopyBackgroundRect (0, nLastMsgYCrd, CCanvas::Current ()->Width (), nLastMsgYCrd + nLastMsgHeight - 1);
	gameData.render.frame.Deactivate ();
	nLastMsgYCrd = -1;
	}
#endif
szDisplayedBackgroundMsg [0][0] = 0;
}

//	-----------------------------------------------------------------------------

void HUDClearMessages (void)
{
	int32_t i, j;
	CHUDMessage	*pMsgs;

for (j = 2, pMsgs = gameData.hud.msgs; j; j--, pMsgs++) {
	pMsgs->nMessages = 0;
	pMsgs->nFirst = 
	pMsgs->nLast = 0;
	pMsgs->xTimer = 0;
	ClearBackgroundMessages ();
	for (i = 0; i < HUD_MAX_MSGS; i++)
#if 1
		*pMsgs->szMsgs [i] = '\0';
#else
		sprintf (pMsgs->szMsgs [i], TXT_SLAGEL);
#endif
	}
}


// ----------------------------------------------------------------------------

void HUDRenderMessages (uint8_t nType)
{
if (gameStates.app.bSaveScreenShot)
	return;

	int32_t		h, i, n, w, y, aw, yStart;
	CHUDMessage *pMsgs = gameData.hud.msgs + nType;

if ((pMsgs->nMessages < 0) || (pMsgs->nMessages > HUD_MAX_MSGS))
	return; // Get Rob!
if ((pMsgs->nMessages < 1) && (nModexHUDMsgs == 0))
	return;
pMsgs->xTimer -= gameData.time.xFrame;
if (pMsgs->xTimer < 0) {
	// Timer expired... get rid of oldest pszMsg...
	if (pMsgs->nLast != pMsgs->nFirst) {
		int32_t	temp;

		//&pMsgs->szMsgs.szMsg [pMsgs->nFirst][0] is deing deleted...;
		pMsgs->nFirst = (pMsgs->nFirst + 1) % HUD_MAX_MSGS;
		pMsgs->xTimer = I2X (2);
		if (!--pMsgs->nMessages)
			nModexHUDMsgs = 2;
		temp = nLastMsgYCrd;
		ClearBackgroundMessages ();			//	If in status bar mode and no messages, then erase.
		if (nModexHUDMsgs)
			nLastMsgYCrd = temp;
	}
}

if (pMsgs->nMessages > 0) {
	if (pMsgs->nColor == (uint32_t) -1)
		pMsgs->nColor = GREEN_RGBA;

#if 0 
	// Obsolete. You cannot shrink the game's output area below the screen or window size anymore in D2X-XL. 
	// That feature has been removed because today's hardware is powerful enough to render Descent,
	// and since you can run the program in windowed and fullscreen mode at lower than native screen resolution.
	if (((gameStates.render.cockpit.nType == CM_STATUS_BAR) || (gameStates.render.cockpit.nType == CM_FULL_SCREEN)) && (gameData.render.scene.Top () >= (gameData.render.screen.Height () / 8))) {
		// Only display the most recent pszMsg in this mode
		int32_t nMsg = (pMsgs->nFirst + pMsgs->nMessages-1) % HUD_MAX_MSGS;
		char* pszMsg = pMsgs->szMsgs [nMsg];

		if (strcmp (szDisplayedBackgroundMsg [0], pszMsg)) {
			int32_t ycrd = /*CCanvas::Current ()->Top () -*/ (SMALL_FONT->Height () + 2);
			if (ycrd < 0)
				ycrd = 0;
			fontManager.SetCurrent (SMALL_FONT);
			fontManager.Current ()->StringSize (pszMsg, w, h, aw);
			ClearBackgroundMessages ();
			if (pMsgs->nColor == (uint32_t) -1)
				pMsgs->nColor = GREEN_RGBA;
			fontManager.SetColorRGBi (pMsgs->nColor, 1, 0, 0);
			pMsgs->nMsgIds [nMsg] = GrPrintF (pMsgs->nMsgIds + nMsg, (CCanvas::Current ()->Width ()-w) / 2, ycrd, pszMsg);
			strcpy (szDisplayedBackgroundMsg [0], pszMsg);
			nLastMsgYCrd = ycrd;
			nLastMsgHeight = h;
			}
		} 
	else 
#endif // obsolete
		{
		fontManager.SetCurrent (SMALL_FONT);
		if ((gameStates.render.cockpit.nType == CM_FULL_SCREEN) || 
			 (gameStates.render.cockpit.nType == CM_LETTERBOX)) {
			yStart = SMALL_FONT->Height () / 2;
			}
		else
			yStart = SMALL_FONT->Height () / 2;
		if (gameOpts->render.cockpit.bGuidedInMainView) {
			if (gameData.objData.HasGuidedMissile (N_LOCALPLAYER))
				yStart += SMALL_FONT->Height () + 3;
			}

		for (i = 0, y = yStart; i < pMsgs->nMessages; i++) {
			n = (pMsgs->nFirst + i) % HUD_MAX_MSGS;
			if ((n < 0) || (n >= HUD_MAX_MSGS))
				return; // Get Rob!!
			if (!strcmp (pMsgs->szMsgs [n], "This is a bug."))
				return; // Get Rob!!
			fontManager.Current ()->StringSize (pMsgs->szMsgs [n], w, h, aw);
			fontManager.SetColorRGBi (pMsgs->nColor, 1, 0, 0);
			y = yStart + i * (h + 1);
			if (nType)
				y += ((2 * HUD_MAX_MSGS - 1) * (h + 1)) / 2;
#if 1
			GrString ((CCanvas::Current ()->Width ()-w)/2, y, pMsgs->szMsgs [n]);
#else
			pMsgs->nMsgIds [n] = GrString ((CCanvas::Current ()->Width ()-w)/2, y, pMsgs->szMsgs [n], pMsgs->nMsgIds + n);
#endif
			if (!gameOpts->render.cockpit.bSplitHUDMsgs) 
				y += h + 1;
			}
		}
	}
#if 0
else if (CurrentGameScreen ()->Mode () == BM_MODEX) {
	if (nModexHUDMsgs) {
		int32_t temp = nLastMsgYCrd;
		nModexHUDMsgs--;
		ClearBackgroundMessages ();			//	If in status bar mode and no messages, then erase.
		nLastMsgYCrd = temp;
		}
	}
#endif
fontManager.SetCurrent (GAME_FONT);
}

// ----------------------------------------------------------------------------
//	Writes a pszMsg on the HUD and checks its timer.
void HUDRenderMessageFrame (void)
{
HUDRenderMessages (0);
if (gameOpts->render.cockpit.bSplitHUDMsgs)
	HUDRenderMessages (1);
}

//------------------------------------------------------------------------------
// Call to flash a message on the HUD.  Returns true if message drawn.
// (pszMsg might not be drawn if previous pszMsg was same)
int32_t HUDInitMessageVA (uint8_t nType, const char * format, va_list args)
{
	CHUDMessage *pMsgs = gameData.hud.msgs + (gameOpts->render.cockpit.bSplitHUDMsgs ? nType : 0);
	int32_t		temp;
	char			*pszMsg = NULL, 
					*pszLastMsg = NULL;
	char			con_message [HUD_MESSAGE_LENGTH + 3];

#if DBG
if (!gameOpts->render.cockpit.bHUDMsgs)
#else
if (!gameOpts->render.cockpit.bHUDMsgs || cockpit->Hide ())
#endif
	return 0;
nModexHUDMsgs = 2;
if ((pMsgs->nLast < 0) || (pMsgs->nLast >= HUD_MAX_MSGS))
	return 0; // Get Rob!!
pszMsg = pMsgs->szMsgs [pMsgs->nLast];
vsprintf (pszMsg, format, args);
if (strlen (pszMsg) >= HUD_MESSAGE_LENGTH) {
	PrintLog (0, "HUD message is too long. Limit is %i characters.\n", HUD_MESSAGE_LENGTH);
	return 0;
	}
// Produce a colorised version and send it to the console
con_message [0] = CC_COLOR;
if (pMsgs->nColor != (uint32_t) -1) {
	con_message [1] = (char) RGBA_RED (pMsgs->nColor) / 2 + 128;
	con_message [2] = (char) RGBA_GREEN (pMsgs->nColor) / 2 + 128;
	con_message [3] = (char) RGBA_BLUE (pMsgs->nColor) / 2 + 128;
	}
else if (nType) {
	con_message [1] = (char) (127 + 128);
	con_message [2] = (char) (95 + 128);
	con_message [3] = (char) (0 + 128);
	}
else {
	con_message [1] = (char) (0 + 128);
	con_message [3] = (char) (0 + 128);
	con_message [2] = (char) (63 + 128);
	}
con_message [4] = '\0';
strcat (con_message, pszMsg);
#if TRACE	
console.printf (CON_NORMAL, "%s\n", con_message);
#endif
// Added by Leighton
if (IsMultiGame) {
	if (gameOpts->multi.bNoRedundancy && !strnicmp ("You already", pszMsg, 11))
		return 0;
	if (!gameData.hud.bPlayerMessage && FindArg ("-PlayerMessages"))
		return 0;
	}
if (pMsgs->nMessages > 1) {
	pszLastMsg = pMsgs->szMsgs [((pMsgs->nLast - 1) ? pMsgs->nLast : HUD_MAX_MSGS) - 2];
	if (pszLastMsg && (!strcmp (pszLastMsg, pszMsg))) {
		pMsgs->xTimer = I2X (3);		// 1 second per 5 characters
		return 0;	// ignore since it is the same as the last one
		}
	}
if (pMsgs->nMessages > 0)
	pszLastMsg = pMsgs->szMsgs [(pMsgs->nLast ? pMsgs->nLast : HUD_MAX_MSGS) - 1];
temp = (pMsgs->nLast + 1) % HUD_MAX_MSGS;
if (temp == pMsgs->nFirst) { // If too many messages, remove oldest pszMsg to make room
	pMsgs->nFirst = (pMsgs->nFirst + 1) % HUD_MAX_MSGS;
	pMsgs->nMessages--;
	}
if (pszLastMsg && (!strcmp (pszLastMsg, pszMsg))) {
	pMsgs->xTimer = I2X (3);		// 1 second per 5 characters
	return 0;	// ignore since it is the same as the last one
	}
pMsgs->nLast = temp;
// Check if memory has been overwritten at this point.
if (gameData.demo.nState == ND_STATE_RECORDING)
	NDRecordHUDMessage (pszMsg);
pMsgs->xTimer = I2X (3);		// 1 second per 5 characters
pMsgs->nMessages++;
return 1;
}

//------------------------------------------------------------------------------

int32_t _CDECL_ HUDInitMessage (const char *format, ...)
{
	int32_t ret = 0;

if (gameOpts->render.cockpit.bHUDMsgs) {
	va_list args;

	va_start (args, format);
	ret = HUDInitMessageVA (0, format, args);
	va_end (args);
	}
return ret;
}

//------------------------------------------------------------------------------

void PlayerDeadMessage (void)
{
if (gameOpts->render.cockpit.bHUDMsgs && LOCALPLAYER.m_bExploded) {
	CHUDMessage	*pMsgs = gameData.hud.msgs;

   if (LOCALPLAYER.lives < 2) {
      int32_t x, y, w, h, aw;
      fontManager.SetCurrent (HUGE_FONT);
      fontManager.Current ()->StringSize (TXT_GAME_OVER, w, h, aw);
      w += 20;
      h += 8;
      x = (CCanvas::Current ()->Width () - w) / 2;
      y = (CCanvas::Current ()->Height () - h) / 2;
      gameStates.render.grAlpha =	 (2 * 7);
      CCanvas::Current ()->SetColorRGB (0, 0, 0, 255);
      OglDrawFilledRect (x, y, x + w, y + h);
      gameStates.render.grAlpha = 1.0f;
      GrString (0x8000, (CCanvas::Current ()->Height () - CCanvas::Current ()->Font ()->Height ()) / 2 + h / 8, TXT_GAME_OVER);
#if 0
      // Automatically exit death after 10 secs
      if (gameData.time.xGame > gameStates.app.nPlayerTimeOfDeath + I2X (10)) {
               SetFunctionMode (FMODE_MENU);
               gameData.app.SetGameMode (GM_GAME_OVER);
               __asm int32_t 3; longjmp (gameExitPoint, 1);        // Exit out of game loop
	      }
#endif
	   }
   fontManager.SetCurrent (GAME_FONT);
   if (pMsgs->nColor == (uint32_t) -1)
      pMsgs->nColor = RGBA_PAL2 (0, 28, 0);
	fontManager.SetColorRGBi (pMsgs->nColor, 1, 0, 0);
   GrString (0x8000, CCanvas::Current ()->Height ()- (CCanvas::Current ()->Font ()->Height () + 3), TXT_PRESS_ANY_KEY);
	}
}

//------------------------------------------------------------------------------
// void say_afterburner_status (void)
// {
// 	if (LOCALPLAYER.flags & PLAYER_FLAGS_AFTERBURNER)
// 		InitHUDMessage ("Afterburner engaged.");
// 	else
// 		InitHUDMessage ("Afterburner disengaged.");
// }

void _CDECL_ HUDMessage (int32_t nClass, const char *format, ...)
{
if (gameOpts->render.cockpit.bHUDMsgs && 
#if !DBG
	 !cockpit->Hide () &&
#endif
	 (!bNoMsgRedundancy || (nClass & MSGC_NOREDUNDANCY)) &&
	 (!bMSGPlayerMsgs || !IsMultiGame || (nClass & MSGC_PLAYERMESSAGES))) {
		va_list vp;

	va_start (vp, format);
	HUDInitMessageVA (0, format, vp);
	va_end (vp);
	}
}

//------------------------------------------------------------------------------

void _CDECL_ HUDPlayerMessage (const char *format, ...)
{
	va_list vp;

if (gameOpts->render.cockpit.bHUDMsgs && !cockpit->Hide ()) {
	va_start (vp, format);
	HUDInitMessageVA (1, format, vp);
	va_end (vp);
	}
}

//------------------------------------------------------------------------------
//eof
