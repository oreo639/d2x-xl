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
#include "error.h"
#include "newdemo.h"
#include "gamefont.h"
#include "text.h"
#include "network.h"
#include "network_lib.h"
#include "rendermine.h"
#include "transprender.h"
#include "gamepal.h"
#include "ogl_lib.h"
#include "ogl_render.h"
#include "ogl_bitmap.h"
#include "ogl_hudstuff.h"
#include "playerprofile.h"
#include "cockpit.h"
#include "hud_defs.h"
#include "statusbar.h"
#include "slowmotion.h"
#include "automap.h"
#include "gr.h"

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------

CGenericCockpit::CGenericCockpit ()
{
m_save.Create (10);
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------

int32_t CGenericCockpit::X (int32_t x, bool bForce)
{
#if 0
return x;
#else
return gameData.X (x, bForce);
#endif
}

//------------------------------------------------------------------------------

void CCockpit::GetHostageWindowCoords (int32_t& x, int32_t& y, int32_t& w, int32_t& h)
{
x = SECONDARY_W_BOX_LEFT;
y = SECONDARY_W_BOX_TOP;
w = SECONDARY_W_BOX_RIGHT - SECONDARY_W_BOX_LEFT + 1;
h = SECONDARY_W_BOX_BOT - SECONDARY_W_BOX_TOP + 1;
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawRecording (void)
{
CGenericCockpit::DrawRecording ((CCanvas::Current ()->Height () > 240) ? 80 : 30);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawCountdown (void)
{
CGenericCockpit::DrawCountdown (SMALL_FONT->Height () * 4);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawCruise (void)
{
CGenericCockpit::DrawCruise (3, CCanvas::Current ()->Height () - m_info.nLineSpacing * (IsMultiGame ? 11 : 6));
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawLives (void)
{
hudCockpit.SetColor (WHITE_RGBA);
hudCockpit.DrawLives ();
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawScore (void)
{
hudCockpit.DrawScore ();
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawAddedScore (void)
{
hudCockpit.SetLineSpacing (m_info.nLineSpacing);
hudCockpit.DrawAddedScore ();
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawOrbs (void)
{
CGenericCockpit::DrawOrbs (4 * m_info.fontWidth, 2 * m_info.nLineSpacing);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawFlag (void)
{
CGenericCockpit::DrawFlag (4 * m_info.fontWidth, 2 * m_info.nLineSpacing);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawHomingWarning (void)
{
m_info.bLastHomingWarningDrawn [0] = (LOCALPLAYER.homingObjectDist >= 0) && (gameData.timeData.xGame & 0x4000);
BitBlt (m_info.bLastHomingWarningDrawn [0] ? GAUGE_HOMING_WARNING_ON : GAUGE_HOMING_WARNING_OFF, 
		  HOMING_WARNING_X, HOMING_WARNING_Y);
}

//	-----------------------------------------------------------------------------

void CCockpit::ClearBombCount (int32_t bgColor)
{
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawBombCount (void)
{
CGenericCockpit::DrawBombCount (BOMB_COUNT_X, BOMB_COUNT_Y, BLACK_RGBA, 1);
}

//	-----------------------------------------------------------------------------

int32_t CCockpit::DrawBombCount (int32_t& nIdBombCount, int32_t x, int32_t y, int32_t nColor, char* pszBombCount)
{
fontManager.SetColorRGBi (nColor, 1, 0, 1);
return PrintF (&nIdBombCount, -(ScaleX (x) + WidthPad (pszBombCount)), -(ScaleY (y) + HeightPad ()), pszBombCount, nIdBombCount);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawPrimaryAmmoInfo (int32_t ammoCount)
{
DrawAmmoInfo (ScaleX (PRIMARY_AMMO_X), ScaleY (PRIMARY_AMMO_Y), ammoCount, 1);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawSecondaryAmmoInfo (int32_t ammoCount)
{
DrawAmmoInfo (ScaleX (SECONDARY_AMMO_X), ScaleY (SECONDARY_AMMO_Y), ammoCount, 0);
}

//	-----------------------------------------------------------------------------

static inline int32_t NumDispX (int32_t val)
{
int32_t x = ((val > 99) ? 7 : (val > 9) ? 11 : 15);
if (!gameStates.video.nDisplayMode)
	x /= 2;
return x + NUMERICAL_GAUGE_X;
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawShieldText (void)
{
	static int32_t nIdShield = 0;

	char szShield [20];

#if 0
CBitmap* pBm = BitBlt (GAUGE_NUMERICAL, NUMERICAL_GAUGE_X, NUMERICAL_GAUGE_Y);
#else
PageInGauge (GAUGE_NUMERICAL);
CBitmap* pBm = gameData.pigData.tex.bitmaps [0] + GaugeIndex (GAUGE_NUMERICAL);
#endif
fontManager.SetColorRGBi (RGBA_PAL2 (14, 14, 23), 1, 0, 0);
sprintf (szShield, "%d", (int32_t) FRound (m_info.nShield * LOCALPLAYER.ShieldScale ()));
int32_t w, h, aw;
fontManager.SetScale (floor (float (CCanvas::Current ()->Width ()) / 640.0f));
fontManager.Current ()->StringSize (szShield, w, h, aw);
nIdShield = PrintF (&nIdShield, -(ScaleX (NUMERICAL_GAUGE_X + pBm->Width () / 2) - w / 2), 
						  NUMERICAL_GAUGE_Y + (gameStates.video.nDisplayMode ? 36 : 16) + HeightPad (), szShield);
fontManager.SetScale (1.0f);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawEnergyText (void)
{
	static int32_t nIdEnergy = 0;

	char szEnergy [20];

#if 0
CBitmap* pBm = BitBlt (GAUGE_NUMERICAL, NUMERICAL_GAUGE_X, NUMERICAL_GAUGE_Y);
#else
PageInGauge (GAUGE_NUMERICAL);
CBitmap* pBm = gameData.pigData.tex.bitmaps [0] + GaugeIndex (GAUGE_NUMERICAL);
#endif
fontManager.SetColorRGBi (RGBA_PAL2 (25, 18, 6), 1, 0, 0);
sprintf (szEnergy, "%d", (int32_t) FRound (m_info.nEnergy * LOCALPLAYER.EnergyScale ()));
int32_t w, h, aw;
fontManager.SetScale (floor (float (CCanvas::Current ()->Width ()) / 640.0f));
fontManager.Current ()->StringSize (szEnergy, w, h, aw);
nIdEnergy = PrintF (&nIdEnergy, -(ScaleX (NUMERICAL_GAUGE_X + pBm->Width () / 2) - w / 2), 
						  NUMERICAL_GAUGE_Y + (gameStates.video.nDisplayMode ? 5 : 2), szEnergy);
fontManager.SetScale (1.0f);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawEnergyBar (void)
{
// values taken directly from the bitmap
#define ENERGY_GAUGE_TOP_LEFT		20
#define ENERGY_GAUGE_BOT_LEFT		0
#define ENERGY_GAUGE_BOT_WIDTH	126

if (m_info.nEnergy) {	
	BitBlt (GAUGE_ENERGY_LEFT, LEFT_ENERGY_GAUGE_X, LEFT_ENERGY_GAUGE_Y);
	BitBlt (GAUGE_ENERGY_RIGHT, RIGHT_ENERGY_GAUGE_X, RIGHT_ENERGY_GAUGE_Y);
	if (m_info.nEnergy < 100) {	// erase part of gauge corresponding to energy loss
		float fScale = float (100 - m_info.nEnergy) / 100.0f;
		ogl.SetBlendMode (OGL_BLEND_ALPHA);

			{
			int32_t x [4] = {ENERGY_GAUGE_TOP_LEFT, LEFT_ENERGY_GAUGE_W, ENERGY_GAUGE_BOT_LEFT + ENERGY_GAUGE_BOT_WIDTH, ENERGY_GAUGE_BOT_LEFT};
			int32_t y [4] = {0, 0, LEFT_ENERGY_GAUGE_H, LEFT_ENERGY_GAUGE_H};

			x [1] = x [0] + int32_t (fScale * (x [1] - x [0]));
			x [2] = x [3] + int32_t (fScale * (x [2] - x [3]));
			for (int32_t i = 0; i < 4; i++) {
				x [i] = ScaleX (LEFT_ENERGY_GAUGE_X + x [i]);
				y [i] = ScaleY (LEFT_ENERGY_GAUGE_Y + y [i]);
				}
			OglDrawFilledPoly (x, y, 4, gaugeFadeColors [0], 1);
#if 0
			x [0] = x [1];
			x [3] = x [2];
			x [1] += (ScaleX (LEFT_ENERGY_GAUGE_X + LEFT_ENERGY_GAUGE_W) - x [1]) / 2;
			x [2] += (ScaleX (LEFT_ENERGY_GAUGE_X + ENERGY_GAUGE_BOT_LEFT + ENERGY_GAUGE_BOT_WIDTH) - x [2]) / 2;
			ogl.SetBlending (true);
			OglDrawFilledPoly (x, y, 4, gaugeFadeColors [0], 4);
#endif
			}

			{
			int32_t x [4] = {0, LEFT_ENERGY_GAUGE_W - ENERGY_GAUGE_TOP_LEFT, LEFT_ENERGY_GAUGE_W - ENERGY_GAUGE_BOT_LEFT, LEFT_ENERGY_GAUGE_W - ENERGY_GAUGE_BOT_WIDTH};
			int32_t y [4] = {0, 0, LEFT_ENERGY_GAUGE_H, LEFT_ENERGY_GAUGE_H};

			x [0] = x [1] - int32_t (fScale * (x [1] - x [0]));
			x [3] = x [2] - int32_t (fScale * (x [2] - x [3]));
			for (int32_t i = 0; i < 4; i++) {
				x [i] = ScaleX (RIGHT_ENERGY_GAUGE_X + x [i]);
				y [i] = ScaleY (RIGHT_ENERGY_GAUGE_Y + y [i]);
				}
			OglDrawFilledPoly (x, y, 4, gaugeFadeColors [0], 1);
#if 0
			x [1] = x [0];
			x [2] = x [3];
			x [0] = ScaleX (RIGHT_ENERGY_GAUGE_X);
			x [0] += (x [1] - x [0]) / 2;
			x [3] = ScaleX (RIGHT_ENERGY_GAUGE_X + LEFT_ENERGY_GAUGE_W - ENERGY_GAUGE_BOT_WIDTH);
			x [3] += (x [2] - x [3]) / 2;
			ogl.SetBlending (true);
			OglDrawFilledPoly (x, y, 4, gaugeFadeColors [1], 4);
#endif
			}
		ogl.SetBlending (false);
		}
	}
}

//	-----------------------------------------------------------------------------

uint8_t afterburnerBarTable [AFTERBURNER_GAUGE_H_L * 2] = {
			3, 11,
			3, 11,
			3, 11,
			3, 11,
			3, 11,
			3, 11,
			2, 11,
			2, 10,
			2, 10,
			2, 10,
			2, 10,
			2, 10,
			2, 10,
			1, 10,
			1, 10,
			1, 10,
			1, 9,
			1, 9,
			1, 9,
			1, 9,
			0, 9,
			0, 9,
			0, 8,
			0, 8,
			0, 8,
			0, 8,
			1, 8,
			2, 8,
			3, 8,
			4, 8,
			5, 8,
			6, 7,
};

uint8_t afterburnerBarTableHires [AFTERBURNER_GAUGE_H_H*2] = {
	5, 20,
	5, 20,
	5, 19,
	5, 19,
	5, 19,
	5, 19,
	4, 19,
	4, 19,
	4, 19,
	4, 19,

	4, 19,
	4, 18,
	4, 18,
	4, 18,
	4, 18,
	3, 18,
	3, 18,
	3, 18,
	3, 18,
	3, 18,

	3, 18,
	3, 17,
	3, 17,
	2, 17,
	2, 17,
	2, 17,
	2, 17,
	2, 17,
	2, 17,
	2, 17,

	2, 17,
	2, 16,
	2, 16,
	1, 16,
	1, 16,
	1, 16,
	1, 16,
	1, 16,
	1, 16,
	1, 16,

	1, 16,
	1, 15,
	1, 15,
	1, 15,
	0, 15,
	0, 15,
	0, 15,
	0, 15,
	0, 15,
	0, 15,

	0, 14,
	0, 14,
	0, 14,
	1, 14,
	2, 14,
	3, 14,
	4, 14,
	5, 14,
	6, 13,
	7, 13,

	8, 13,
	9, 13,
	10, 13,
	11, 13,
	12, 13
};

//	-----------------------------------------------------------------------------

void CCockpit::DrawAfterburnerBar (void)
{
#if 1
if (!(LOCALPLAYER.flags & PLAYER_FLAGS_AFTERBURNER))
	return;		//don't draw if don't have
if (!gameData.physicsData.xAfterburnerCharge)
	return;
#endif
//CCanvas::Current ()->SetColorRGB (255, 255, 255, 255);
BitBlt (GAUGE_AFTERBURNER, AFTERBURNER_GAUGE_X, AFTERBURNER_GAUGE_Y);
int32_t yMax = FixMul (I2X (1) - gameData.physicsData.xAfterburnerCharge, AFTERBURNER_GAUGE_H);
if (yMax) {
	int32_t		x [4], y [4];
	uint8_t*	tableP = gameStates.video.nDisplayMode ? afterburnerBarTableHires : afterburnerBarTable;

	y [0] = y [1] = ScaleY (AFTERBURNER_GAUGE_Y);
	y [3] = ScaleY (AFTERBURNER_GAUGE_Y + yMax) - 1;
	x [1] = ScaleX (AFTERBURNER_GAUGE_X + tableP [0]);
	x [0] = ScaleX (AFTERBURNER_GAUGE_X + tableP [1] + 1);
	x [2] = x [1];
	y [2] = 0;
	for (int32_t i = 1; i < yMax - 1; i++)
		if (x [2] >= tableP [2 * i]) {
			x [2] = tableP [2 * i];
			y [2] = i;
			}
	x [2] = ScaleX (AFTERBURNER_GAUGE_X + x [2] + 1);
	y [2] = ScaleY (AFTERBURNER_GAUGE_Y + y [2]);
	x [3] = ScaleX (AFTERBURNER_GAUGE_X + tableP [2 * yMax - 1] + 1);
	gameStates.render.grAlpha = 1.0f;
	ogl.SetBlending (true);
	ogl.SetBlendMode (OGL_BLEND_ALPHA);
	OglDrawFilledPoly (x, y, 4, gaugeFadeColors [0], 1);
	ogl.SetBlending (false);
	}
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawShieldBar (void)
{
if (!(LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) || (m_info.tInvul <= 0))
	BitBlt (GAUGE_SHIELDS + 9 - ((m_info.nShield >= 100) ? 9 : (m_info.nShield / 10)), SHIELD_GAUGE_X, SHIELD_GAUGE_Y);
}

//	-----------------------------------------------------------------------------

typedef struct tKeyGaugeInfo {
	int32_t	nFlag, nGaugeOn, nGaugeOff, x [2], y [2];
} tKeyGaugeInfo;

static tKeyGaugeInfo keyGaugeInfo [] = {
	{PLAYER_FLAGS_BLUE_KEY, GAUGE_BLUE_KEY, GAUGE_BLUE_KEY_OFF, {GAUGE_BLUE_KEY_X_L, GAUGE_BLUE_KEY_X_H}, {GAUGE_BLUE_KEY_Y_L, GAUGE_BLUE_KEY_Y_H}},
	{PLAYER_FLAGS_GOLD_KEY, GAUGE_GOLD_KEY, GAUGE_GOLD_KEY_OFF, {GAUGE_GOLD_KEY_X_L, GAUGE_GOLD_KEY_X_H}, {GAUGE_GOLD_KEY_Y_L, GAUGE_GOLD_KEY_Y_H}},
	{PLAYER_FLAGS_RED_KEY, GAUGE_RED_KEY, GAUGE_RED_KEY_OFF, {GAUGE_RED_KEY_X_L, GAUGE_RED_KEY_X_H}, {GAUGE_RED_KEY_Y_L, GAUGE_RED_KEY_Y_H}},
	};

void CCockpit::DrawKeys (void)
{
int32_t bHires = gameStates.video.nDisplayMode != 0;
for (int32_t i = 0; i < 3; i++)
	BitBlt ((LOCALPLAYER.flags & keyGaugeInfo [i].nFlag) ? keyGaugeInfo [i].nGaugeOn : keyGaugeInfo [i].nGaugeOff, keyGaugeInfo [i].x [bHires], keyGaugeInfo [i].y [bHires]);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawWeaponInfo (int32_t nWeaponType, int32_t nWeaponId, int32_t laserLevel)
{
	int32_t nIndex;

if (nWeaponType == 0) {
	nIndex = primaryWeaponToWeaponInfo [nWeaponId];
	if (nIndex == LASER_ID && laserLevel > MAX_LASER_LEVEL)
		nIndex = SUPERLASER_ID;
	CGenericCockpit::DrawWeaponInfo (nWeaponType, nIndex,
		hudWindowAreas + COCKPIT_PRIMARY_BOX,
		PRIMARY_W_PIC_X, PRIMARY_W_PIC_Y,
		PRIMARY_WEAPON_NAMES_SHORT (nWeaponId),
		PRIMARY_W_TEXT_X, PRIMARY_W_TEXT_Y, 0);
		}
else {
	nIndex = secondaryWeaponToWeaponInfo [nWeaponId];
	CGenericCockpit::DrawWeaponInfo (nWeaponType, nIndex,
		hudWindowAreas + COCKPIT_SECONDARY_BOX,
		SECONDARY_W_PIC_X, SECONDARY_W_PIC_Y,
		SECONDARY_WEAPON_NAMES_SHORT (nWeaponId),
		SECONDARY_W_TEXT_X, SECONDARY_W_TEXT_Y, 0);
	}
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawKillList (void)
{
CGenericCockpit::DrawKillList (53, CCanvas::Current ()->Height () - LHX (6));
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawStatic (int32_t nWindow)
{
CGenericCockpit::DrawStatic (nWindow, COCKPIT_PRIMARY_BOX);
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawPlayerShip (void)
{
CGenericCockpit::DrawPlayerShip (m_info.bCloak, m_history [0].bCloak, SHIP_GAUGE_X, SHIP_GAUGE_Y);
}

//	-----------------------------------------------------------------------------

//	Draws invulnerable ship, or maybe the flashing ship, depending on invulnerability time left.
void CCockpit::DrawInvul (void)
{
	static fix time = 0;

if ((LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) &&
	 ((m_info.tInvul > I2X (4)) || ((m_info.tInvul > 0) && (gameData.timeData.xGame & 0x8000)))) {
	BitBlt (GAUGE_INVULNERABLE + m_info.nInvulnerableFrame, SHIELD_GAUGE_X, SHIELD_GAUGE_Y);
	time += gameData.timeData.xFrame;
	while (time > INV_FRAME_TIME) {
		time -= INV_FRAME_TIME;
		if (++m_info.nInvulnerableFrame == N_INVULNERABLE_FRAMES)
			m_info.nInvulnerableFrame = 0;
		}
	}
}

//	-----------------------------------------------------------------------------

void CCockpit::DrawCockpit (bool bAlphaTest)
{
CGenericCockpit::DrawCockpit (m_info.nCockpit, 0, bAlphaTest);
}

//	-----------------------------------------------------------------------------

void CCockpit::SetupWindow (int32_t nWindow)
{
tGaugeBox* pHudArea = hudWindowAreas + COCKPIT_PRIMARY_BOX + nWindow;
gameData.renderData.window.Setup (&gameData.renderData.frame, 
										gameData.renderData.frame.Left (false) + ScaleX (pHudArea->left),
										gameData.renderData.frame.Top (false) + ScaleY (pHudArea->top),
										ScaleX (pHudArea->right - pHudArea->left + 1), ScaleY (pHudArea->bot - pHudArea->top + 1));
gameData.renderData.window.Activate ("HUD Window (window)", &gameData.renderData.frame);
}

//	-----------------------------------------------------------------------------

bool CCockpit::Setup (bool bScene, bool bRebuild)
{
if (bRebuild && !m_info.bRebuild)
	return true;
m_info.bRebuild = false;
if (!CGenericCockpit::Setup (bScene, bRebuild))
	return false;
*((CViewport*) &gameData.renderData.scene) += CViewport (0, 0, 0, -gameData.renderData.frame.Height (false) / 3);
if (bScene)
	gameData.renderData.scene.Activate ("CCockpit::Setup (scene)");
else
	gameData.renderData.frame.Activate ("CCockpit::Setup (frame)");
return true;
}

//	-----------------------------------------------------------------------------

void CCockpit::Toggle (void)
{
CGenericCockpit::Activate (CM_STATUS_BAR, true);
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------

bool CRearView::Setup (bool bScene, bool bRebuild)
{
if (bRebuild && !m_info.bRebuild)
	return true;
if (!CGenericCockpit::Setup (bScene, bRebuild))
	return false;
*((CViewport*) &gameData.renderData.scene) += CViewport (0, 0, 0, -gameData.renderData.frame.Height (false) / 3);
if (bScene)
	gameData.renderData.scene.Activate ("CCockpit::Setup (scene)");
else
	gameData.renderData.frame.Activate ("CCockpit::Setup (frame)");
//*Canvas () += CViewport (0, 0, 0, -gameData.renderData.frame.Height (false) / 3);
//Canvas ()->Activate ("CRearView::Setup");
return true;
}

//	-----------------------------------------------------------------------------

