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
#include <stdarg.h>

#include "descent.h"
#include "screens.h"
#include "physics.h"
#include "error.h"
#include "newdemo.h"
#include "gamefont.h"
#include "text.h"
#include "network.h"
#include "input.h"
#include "ogl_bitmap.h"
#include "ogl_hudstuff.h"
#include "ogl_render.h"
#include "cockpit.h"
#include "hud_defs.h"
#include "hudmsgs.h"
#include "statusbar.h"

//	-----------------------------------------------------------------------------

CBitmap* CStatusBar::StretchBlt (int32_t nGauge, int32_t x, int32_t y, double xScale, double yScale, int32_t scale, int32_t orient)
{
	CBitmap* pBm = NULL;

if (nGauge >= 0) {
	PageInGauge (nGauge);
	CBitmap* pBm = gameData.pigData.tex.bitmaps [0] + GaugeIndex (nGauge);
	if (pBm)
		pBm->RenderScaled (ScaleX (x), ScaleY (y), 
								 ScaleX ((int32_t) DRound (pBm->Width () * xScale)), ScaleY ((int32_t) DRound (pBm->Height () * yScale)), 
								 scale, orient, NULL);
	}
return pBm;
}

//	-----------------------------------------------------------------------------
//fills in the coords of the hostage video window
void CStatusBar::GetHostageWindowCoords (int32_t& x, int32_t& y, int32_t& w, int32_t& h)
{
x = SB_SECONDARY_W_BOX_LEFT;
y = SB_SECONDARY_W_BOX_TOP;
w = SB_SECONDARY_W_BOX_RIGHT - SB_SECONDARY_W_BOX_LEFT + 1;
h = SB_SECONDARY_W_BOX_BOT - SB_SECONDARY_W_BOX_TOP + 1;
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawRecording (void)
{
CGenericCockpit::DrawRecording (0);
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawCountdown (void)
{
CGenericCockpit::DrawCountdown (SMALL_FONT->Height () * 6);
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawCruise (void)
{
CGenericCockpit::DrawCruise (22, ScaleY (m_info.nLineSpacing * 22));
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawScore (void)
{	                                                                                                                                                                                                                                                             
	char	szScore [20];
	int32_t	w, h, aw;

	static int32_t nIdLabel = 0, nIdScore = 0;

fontManager.SetCurrent (GAME_FONT);
SetFontScale ((float) floor (float (CCanvas::Current ()->Width ()) / 640.0f));
strcpy (szScore, (IsMultiGame && !IsCoopGame) ? TXT_KILLS : TXT_SCORE);
strcat (szScore, ":");
fontManager.Current ()->StringSize (szScore, w, h, aw);
SetFontColor (MEDGREEN_RGBA);
nIdLabel = DrawHUDText (&nIdLabel, ScaleX (SB_SCORE_LABEL_X), ScaleY (SB_SCORE_Y), szScore);
sprintf (szScore, "%5d", (IsMultiGame && !IsCoopGame) ? LOCALPLAYER.netKillsTotal : LOCALPLAYER.score);
fontManager.Current ()->StringSize (szScore, w, h, aw);
SetFontColor ((IsMultiGame && !IsCoopGame) ? MEDGREEN_RGBA : GREEN_RGBA);
nIdScore = DrawHUDText (&nIdScore, ScaleX (SB_SCORE_RIGHT- w) - LHY (2), ScaleY (SB_SCORE_Y), szScore);
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawAddedScore (void)
{
if (IsMultiGame && !IsCoopGame) 
	return;

	int32_t	nScore, nTime;

if (!(nScore = cockpit->AddedScore (0)))
	return;

	int32_t	x, w, h, aw, color;
	char	szScore [32];

	static int32_t nIdTotalScore = 0;

cockpit->SetScoreTime (nTime = cockpit->ScoreTime () - gameData.timeData.xFrame);
if (nTime > 0) {
	color = X2I (nTime * 20) + 10;
	if (color < 10) 
		color = 10;
	else if (color > 31) 
		color = 31;
	color = color - (color % 4);	//	Only allowing colors 12, 16, 20, 24, 28 speeds up gr_getcolor, improves caching
	if (gameStates.app.cheats.bEnabled)
		sprintf (szScore, "%s", TXT_CHEATER);
	else
		sprintf (szScore, "%5d", nScore);
	fontManager.SetCurrent (GAME_FONT);
	fontManager.Current ()->StringSize (szScore, w, h, aw);
	x = SB_SCORE_ADDED_RIGHT - w - LHY (2);
	SetFontColor (RGBA_PAL2 (0, color, 0));
	SetFontScale (floor (float (CCanvas::Current ()->Width ()) / 640.0f));
	nIdTotalScore = DrawHUDText (&nIdTotalScore, x, SB_SCORE_ADDED_Y, szScore);
	} 
#if 1
else {
	//erase old score
	cockpit->SetScoreTime (0);
	cockpit->SetAddedScore (0, 0);
	}
#endif
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawOrbs (void)
{
CGenericCockpit::DrawOrbs (m_info.fontWidth, m_info.nLineSpacing);
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawFlag (void)
{
CGenericCockpit::DrawFlag (5 * m_info.nLineSpacing, m_info.nLineSpacing * (gameStates.render.fonts.bHires + 1));
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawHomingWarning (void)
{
hudCockpit.DrawHomingWarning ();
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawPrimaryAmmoInfo (int32_t ammoCount)
{
DrawAmmoInfo (ScaleX (SB_PRIMARY_AMMO_X), ScaleY (SB_PRIMARY_AMMO_Y), ammoCount, 1);
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawSecondaryAmmoInfo (int32_t ammoCount)
{
DrawAmmoInfo (ScaleX (SB_SECONDARY_AMMO_X), ScaleY (SB_SECONDARY_AMMO_Y), ammoCount, 0);
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawLives (void)
{
	static int32_t nIdLives [2] = {0, 0}, nIdKilled = 0;
  
	char		szLives [20];
	int32_t		w, h, aw;

SetFontScale ((float) floor (float (CCanvas::Current ()->Width ()) / 640.0f));
SetFontColor (MEDGREEN_RGBA);
strcpy (szLives, IsMultiGame ? TXT_DEATHS : TXT_LIVES);
fontManager.Current ()->StringSize (szLives, w, h, aw);
nIdLives [0] = DrawHUDText (&nIdLives [0], ScaleX (SB_LIVES_LABEL_X), ScaleY (SB_LIVES_LABEL_Y + HeightPad ()), szLives);

if (IsMultiGame) {
	static int32_t lastX [4] = {SB_SCORE_RIGHT_L, SB_SCORE_RIGHT_L, SB_SCORE_RIGHT_H, SB_SCORE_RIGHT_H};

	char	szKilled [20];
	int32_t	x = SB_LIVES_X, 
			y = (ScaleY (SB_LIVES_Y + 1) + HeightPad ());

	sprintf (szKilled, "%5d", LOCALPLAYER.netKilledTotal);
	fontManager.Current ()->StringSize (szKilled, w, h, aw);
	CCanvas::Current ()->SetColorRGBi (RGB_PAL (0, 0, 0));
	Rect (lastX [(gameStates.video.nDisplayMode ? 2 : 0) + 0], y + 1, SB_SCORE_RIGHT, y + GAME_FONT->Height ());
	SetFontColor (MEDGREEN_RGBA);
	x = SB_SCORE_RIGHT - w - 2;	
	nIdKilled = DrawHUDText (&nIdKilled, ScaleX (x), y + 1, szKilled);
	lastX [(gameStates.video.nDisplayMode ? 2 : 0) + 0] = x;
	}
else if (LOCALPLAYER.lives > 1) {
	int32_t y = ScaleY (SB_LIVES_Y + HeightPad ());
	SetFontColor (MEDGREEN_RGBA);
	CBitmap* pBm = BitBlt (GAUGE_LIVES, SB_LIVES_X, SB_LIVES_Y);
	nIdLives [1] = DrawHUDText (&nIdLives [1], ScaleX (SB_LIVES_X + pBm->Width () + m_info.fontWidth), y, " x %d", LOCALPLAYER.lives - 1);
	}
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawEnergyText (void)
{
	static int32_t nIdEnergy = 0;

	int32_t w, h, aw;
	char szEnergy [20];

SetFontScale ((float) floor (float (CCanvas::Current ()->Width ()) / 640.0f));
sprintf (szEnergy, "%d", (int32_t) DRound (m_info.nEnergy * LOCALPLAYER.EnergyScale ()));
fontManager.Current ()->StringSize (szEnergy, w, h, aw);
SetFontColor (RGBA_PAL2 (25, 18, 6));
nIdEnergy = DrawHUDText (&nIdEnergy, 
								 ScaleX (SB_ENERGY_GAUGE_X + (SB_ENERGY_GAUGE_W - w) / 2), 
								 ScaleY (SB_ENERGY_GAUGE_Y + SB_ENERGY_GAUGE_H - m_info.nLineSpacing) + HeightPad (), 
								 "%d", (int32_t) FRound (m_info.nEnergy * LOCALPLAYER.EnergyScale ()));
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawEnergyBar (void)
{
if (gameStates.app.bD1Mission)
	StretchBlt (SB_GAUGE_ENERGY, SB_ENERGY_GAUGE_X, SB_ENERGY_GAUGE_Y, 1.0, 
					double (SB_ENERGY_GAUGE_H) / double (SB_ENERGY_GAUGE_H - SB_AFTERBURNER_GAUGE_H));
else
	BitBlt (SB_GAUGE_ENERGY, SB_ENERGY_GAUGE_X, SB_ENERGY_GAUGE_Y);
int32_t nEraseHeight = (100 - m_info.nEnergy) * SB_ENERGY_GAUGE_H / 100;
if (nEraseHeight > 0) {
	CCanvas::Current ()->SetColorRGBi (BLACK_RGBA);
	ogl.SetBlending (false);
	Rect (
		SB_ENERGY_GAUGE_X, 
		SB_ENERGY_GAUGE_Y, 
		SB_ENERGY_GAUGE_X + SB_ENERGY_GAUGE_W, 
		SB_ENERGY_GAUGE_Y + nEraseHeight);
	ogl.SetBlending (true);
	}
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawAfterburnerText (void)
{
if (gameStates.app.bD1Mission)
	return;

	static int32_t nIdAfterBurner = 0;

	char szAB [3] = "AB";

SetFontScale ((float) floor (float (CCanvas::Current ()->Width ()) / 640.0f));
if (LOCALPLAYER.flags & PLAYER_FLAGS_AFTERBURNER)
	SetFontColor (RGBA_PAL2 (45, 21, 0));
else 
	SetFontColor (RGBA_PAL2 (12, 12, 12));

int32_t w, h, aw;
fontManager.Current ()->StringSize (szAB, w, h, aw);
nIdAfterBurner = DrawHUDText (&nIdAfterBurner, 
										ScaleX (SB_AFTERBURNER_GAUGE_X + (SB_AFTERBURNER_GAUGE_W - w) / 2), 
										ScaleY (SB_AFTERBURNER_GAUGE_Y + SB_AFTERBURNER_GAUGE_H - m_info.nLineSpacing) + HeightPad (), 
										"AB");
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawAfterburnerBar (void)
{
if (gameStates.app.bD1Mission)
	return;

BitBlt (SB_GAUGE_AFTERBURNER, SB_AFTERBURNER_GAUGE_X, SB_AFTERBURNER_GAUGE_Y);
int32_t nEraseHeight = FixMul ((I2X (1) - gameData.physicsData.xAfterburnerCharge), SB_AFTERBURNER_GAUGE_H);

if (nEraseHeight > 0) {
	ogl.SetBlending (false);
	Rect (SB_AFTERBURNER_GAUGE_X, SB_AFTERBURNER_GAUGE_Y, 
			SB_AFTERBURNER_GAUGE_X + SB_AFTERBURNER_GAUGE_W - 1, SB_AFTERBURNER_GAUGE_Y + nEraseHeight - 1);
	ogl.SetBlending (true);
	}
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawShieldText (void)
{
	static int32_t nIdShield = 0;

	int32_t w, h, aw;
	char szShield [20];

SetFontScale ((float) floor (float (CCanvas::Current ()->Width ()) / 640.0f));
//LoadTexture (gameData.pigData.tex.cockpitBmIndex [gameStates.render.cockpit.nType + (gameStates.video.nDisplayMode ? gameData.modelData.nCockpits / 2 : 0)].index, 0);
SetFontColor (BLACK_RGBA);
Rect (SB_SHIELD_NUM_X, SB_SHIELD_NUM_Y, SB_SHIELD_NUM_X + (gameStates.video.nDisplayMode ? 27 : 13), SB_SHIELD_NUM_Y + m_info.fontHeight);
sprintf (szShield, "%d", (int32_t) FRound (m_info.nShield * LOCALPLAYER.ShieldScale ()));
fontManager.Current ()->StringSize (szShield, w, h, aw);
SetFontColor (RGBA_PAL2 (14, 14, 23));
nIdShield = DrawHUDText (&nIdShield, 
							  ScaleX (SB_SHIELD_NUM_X + (SB_SHIELD_NUM_W - w) / 2), 
							  ScaleY (SB_SHIELD_NUM_Y) + HeightPad (), 
							  "%d", (int32_t) FRound (m_info.nShield * LOCALPLAYER.ShieldScale ()));
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawShieldBar (void)
{
if (!(LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) || (m_info.tInvul <= 0)) {
	BitBlt (GAUGE_SHIELDS + 9 - ((m_info.nShield >= 100) ? 9 : (m_info.nShield / 10)), SB_SHIELD_GAUGE_X, SB_SHIELD_GAUGE_Y);
	}
}

//	-----------------------------------------------------------------------------

typedef struct tKeyGaugeInfo {
	int32_t	nFlag, nGaugeOn, nGaugeOff, x [2], y [2];
} tKeyGaugeInfo;

static tKeyGaugeInfo keyGaugeInfo [] = {
	{PLAYER_FLAGS_BLUE_KEY, SB_GAUGE_BLUE_KEY, SB_GAUGE_BLUE_KEY_OFF, {SB_GAUGE_KEYS_X_L, SB_GAUGE_KEYS_X_H}, {SB_GAUGE_BLUE_KEY_Y_L, SB_GAUGE_BLUE_KEY_Y_H}},
	{PLAYER_FLAGS_GOLD_KEY, SB_GAUGE_GOLD_KEY, SB_GAUGE_GOLD_KEY_OFF, {SB_GAUGE_KEYS_X_L, SB_GAUGE_KEYS_X_H}, {SB_GAUGE_GOLD_KEY_Y_L, SB_GAUGE_GOLD_KEY_Y_H}},
	{PLAYER_FLAGS_RED_KEY, SB_GAUGE_RED_KEY, SB_GAUGE_RED_KEY_OFF, {SB_GAUGE_KEYS_X_L, SB_GAUGE_KEYS_X_H}, {SB_GAUGE_RED_KEY_Y_L, SB_GAUGE_RED_KEY_Y_H}},
	};

void CStatusBar::DrawKeys (void)
{
int32_t bHires = gameStates.video.nDisplayMode != 0;
for (int32_t i = 0; i < 3; i++)
	BitBlt ((LOCALPLAYER.flags & keyGaugeInfo [i].nFlag) ? keyGaugeInfo [i].nGaugeOn : keyGaugeInfo [i].nGaugeOff, keyGaugeInfo [i].x [bHires], keyGaugeInfo [i].y [bHires]);
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawPlayerShip (void)
{
CGenericCockpit::DrawPlayerShip (m_info.bCloak, m_history [0].bCloak, SB_SHIP_GAUGE_X, SB_SHIP_GAUGE_Y);
}

//	-----------------------------------------------------------------------------

//	Draws invulnerable ship, or maybe the flashing ship, depending on invulnerability time left.
void CStatusBar::DrawInvul (void)
{
	static fix time = 0;

if ((LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) &&
	 ((m_info.tInvul > I2X (4)) || ((m_info.tInvul > 0) && (gameData.timeData.xGame & 0x8000)))) {
	BitBlt (GAUGE_INVULNERABLE + m_info.nInvulnerableFrame, SB_SHIELD_GAUGE_X, SB_SHIELD_GAUGE_Y);
	time += gameData.timeData.xFrame;
	while (time > INV_FRAME_TIME) {
		time -= INV_FRAME_TIME;
		if (++m_info.nInvulnerableFrame == N_INVULNERABLE_FRAMES)
			m_info.nInvulnerableFrame = 0;
		}
	}
}

//	-----------------------------------------------------------------------------

void CStatusBar::ClearBombCount (int32_t bgColor)
{
CCanvas::Current ()->SetColorRGBi (bgColor);
if (!gameStates.video.nDisplayMode) {
	Rect (169, 189, 189, 196);
	CCanvas::Current ()->SetColorRGBi (RGB_PAL (128, 128, 128));
	OglDrawLine (ScaleX (168), ScaleY (189), ScaleX (189), ScaleY (189));
	}
else {
	OglDrawFilledRect (ScaleX (338), ScaleY (453), ScaleX (378), ScaleY (470));
	CCanvas::Current ()->SetColorRGBi (RGB_PAL (128, 128, 128));
	OglDrawLine (ScaleX (336), ScaleY (453), ScaleX (378), ScaleY (453));
	}
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawBombCount (void)
{
CGenericCockpit::DrawBombCount (ScaleX (SB_BOMB_COUNT_X), ScaleY (SB_BOMB_COUNT_Y), BLACK_RGBA, 1);
}

//	-----------------------------------------------------------------------------

int32_t CStatusBar::DrawBombCount (int32_t& nIdBombCount, int32_t x, int32_t y, int32_t nColor, char* pszBombCount)
{
SetFontColor (nColor);
int32_t nId = DrawHUDText (&nIdBombCount, x, y, pszBombCount, nIdBombCount);
return nId;
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawStatic (int32_t nWindow)
{
CGenericCockpit::DrawStatic (nWindow, SB_PRIMARY_BOX);
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawWeaponInfo (int32_t nWeaponType, int32_t nWeaponId, int32_t laserLevel)
{
	int32_t nIndex;

if (nWeaponType == 0) {
	nIndex = primaryWeaponToWeaponInfo [nWeaponId];
	if (nIndex == LASER_ID && laserLevel > MAX_LASER_LEVEL)
		nIndex = SUPERLASER_ID;
	CGenericCockpit::DrawWeaponInfo (nWeaponType, nIndex,
												hudWindowAreas + SB_PRIMARY_BOX,
												SB_PRIMARY_W_PIC_X, SB_PRIMARY_W_PIC_Y,
												PRIMARY_WEAPON_NAMES_SHORT (nWeaponId),
												SB_PRIMARY_W_TEXT_X, SB_PRIMARY_W_TEXT_Y, 0);
		}
else {
	nIndex = secondaryWeaponToWeaponInfo [nWeaponId];
	CGenericCockpit::DrawWeaponInfo (nWeaponType, nIndex,
												hudWindowAreas + SB_SECONDARY_BOX,
												SB_SECONDARY_W_PIC_X, SB_SECONDARY_W_PIC_Y,
												SECONDARY_WEAPON_NAMES_SHORT (nWeaponId),
												SB_SECONDARY_W_TEXT_X, SB_SECONDARY_W_TEXT_Y, 0);
	}
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawKillList (void)
{
CGenericCockpit::DrawKillList (60, CCanvas::Current ()->Height ());
}

//	-----------------------------------------------------------------------------

void CStatusBar::DrawCockpit (bool bAlphaTest)
{
CGenericCockpit::DrawCockpit (CM_STATUS_BAR + m_info.nCockpit, gameData.renderData.scene.Height (), bAlphaTest);
}

//	-----------------------------------------------------------------------------

bool CStatusBar::Setup (bool bScene, bool bRebuild)
{
if (bRebuild && !m_info.bRebuild)
	return true;
m_info.bRebuild = false;
if (!CGenericCockpit::Setup (bScene, bRebuild))
	return false;

int32_t h = gameData.pigData.tex.bitmaps [0][gameData.pigData.tex.cockpitBmIndex [CM_STATUS_BAR + (gameStates.video.nDisplayMode ? (gameData.modelData.nCockpits / 2) : 0)].index].Height ();
if (gameStates.app.bDemoData)
	h *= 2;
if (gameData.renderData.screen.Height () > 480)
	h = (int32_t) ((double) h * (double) gameData.renderData.screen.Height () / 480.0);
*((CViewport*) &gameData.renderData.scene) += CViewport (0, 0, 0, -h);

if (bScene)
	gameData.renderData.scene.Activate ("CStatusBar::Setup (scene)");
else
	gameData.renderData.frame.Activate ("CStatusBar::Setup (frame)");
return true;
}

//	-----------------------------------------------------------------------------

void CStatusBar::SetupWindow (int32_t nWindow)
{
tGaugeBox* pHudArea = hudWindowAreas + SB_PRIMARY_BOX + nWindow;
gameData.renderData.window.Setup (&gameData.renderData.frame, 
										gameData.renderData.frame.Left (false) + ScaleX (pHudArea->left),
										gameData.renderData.frame.Top (false) + ScaleY (pHudArea->top),
										ScaleX (pHudArea->right - pHudArea->left + 1), ScaleY (pHudArea->bot - pHudArea->top + 1));
gameData.renderData.window.Activate ("CStatusBar::SetupWindow (window)", &gameData.renderData.frame);
}

//	-----------------------------------------------------------------------------

void CStatusBar::Toggle (void)
{
CGenericCockpit::Activate ((gameStates.render.cockpit.nNextType < 0) ? CM_FULL_SCREEN : gameStates.render.cockpit.nNextType, true);
}

//	-----------------------------------------------------------------------------
