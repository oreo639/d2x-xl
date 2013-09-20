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
#include "error.h"
#include "u_mem.h"
#include "text.h"
#include "gamefont.h"
#include "input.h"
#include "ogl_bitmap.h"
#include "ogl_render.h"
#include "ogl_hudstuff.h"
#include "slowmotion.h"
#include "network.h"
#include "cockpit.h"
#include "headlight.h"
#include "hudicons.h"

//	-----------------------------------------------------------------------------

#define NUM_INV_ITEMS			10
#define INV_ITEM_HEADLIGHT		2
#define INV_ITEM_QUADLASERS	5
#define INV_ITEM_CLOAK			6
#define INV_ITEM_INVUL			7
#define INV_ITEM_SLOWMOTION	8
#define INV_ITEM_BULLETTIME	9

CBitmap*	bmpInventory = NULL;
CBitmap	bmInvItems [NUM_INV_ITEMS];
CBitmap	bmObjTally [2];
CBitmap	bmGaugeIcons [3];

int bHaveInvBms = -1;
int bHaveObjTallyBms = -1;
int bHaveGaugeBms = -1;

CHUDIcons	hudIcons;

//	-----------------------------------------------------------------------------

int CHUDIcons::LoadIcons (const char* pszIcons [], CBitmap* icons, int nIcons, int& bHaveIcons)
{
	char	szFilename [FILENAME_LEN];
	CFile	cf;

if (bHaveIcons > -1)
	return bHaveIcons;
for (int i = 0; i < nIcons; i++) {
	icons [i].Destroy ();
	sprintf (szFilename, "%s/d2x-xl/%s", gameFolders.szTextureDir [2], pszIcons [i]);
	if (!cf.Exist (szFilename, "", 0))
		sprintf (szFilename, "%s/d2x-xl/%s", gameFolders.szTextureDir [0], pszIcons [i]);
	CTGA tga (&icons [i]);
	if (!tga.Read (szFilename, NULL, -1, 1.0, 0)) {
		while (i) {
			i--;
			icons [i].Destroy ();
			}
		return bHaveIcons = 0;
		}
	}
return bHaveIcons = 1;
}

//	-----------------------------------------------------------------------------

void CHUDIcons::DestroyIcons (CBitmap* icons, int nIcons, int& bHaveIcons)
{
if (bHaveObjTallyBms > 0) {
	for (int i = 0; i < nIcons; i++)
		icons [i].Destroy ();
	bHaveIcons = -1;
	}
}

//	-----------------------------------------------------------------------------

const char *pszObjTallyIcons [] = {"rboticon.tga", "pwupicon.tga"};

int CHUDIcons::LoadTallyIcons (void)
{
return LoadIcons (pszObjTallyIcons, bmObjTally, sizeofa (bmObjTally), bHaveObjTallyBms);
}

//	-----------------------------------------------------------------------------

void CHUDIcons::DestroyTallyIcons (void)
{
DestroyIcons (bmObjTally, sizeofa (bmObjTally), bHaveObjTallyBms);
}

//	-----------------------------------------------------------------------------

const char *pszGaugeIcons [] = {"shield-icon.tga", "energy-icon.tga", "afterburner-icon.tga"};

int CHUDIcons::LoadGaugeIcons (void)
{
return LoadIcons (pszGaugeIcons, bmGaugeIcons, sizeofa (bmGaugeIcons), bHaveGaugeBms);
}

//	-----------------------------------------------------------------------------

void CHUDIcons::DestroyGaugeIcons (void)
{
DestroyIcons (bmGaugeIcons, sizeofa (bmGaugeIcons), bHaveGaugeBms);
}

//	-----------------------------------------------------------------------------

CBitmap& CHUDIcons::GaugeIcon (int i)
{
return bmGaugeIcons [i];
}

//	-----------------------------------------------------------------------------

void CHUDIcons::DrawTally (void)
{
if (!gameOpts->render.cockpit.bObjectTally)
	return;
if (cockpit->Hide ())
	return;
if (ogl.IsOculusRift ())
	return;

	static int		objCounts [2] = {0, 0};
	static time_t	t0 = -1;
	static int		nIdTally [2] = {0, 0};
	time_t			t;

if (!IsMultiGame || IsCoopGame) {
	int	x, x0 = 0, y = 0, w, h, aw, i, bmW, bmH;
	char	szInfo [20];

	if (gameStates.render.cockpit.nType == CM_FULL_COCKPIT)
		y = 3 * m_nLineSpacing;
	else if (gameStates.render.cockpit.nType == CM_STATUS_BAR)
		y = 2 * m_nLineSpacing;
	else {//if (!cockpit->Always ()) {
		y = 2 * m_nLineSpacing;
		if (gameStates.render.fonts.bHires)
			y += m_nLineSpacing;
		}
	if (gameOpts->render.cockpit.bPlayerStats)
		y += 2 * m_nLineSpacing;

	x0 = gameData.render.scene.Width ();
	if ((extraGameInfo [0].nWeaponIcons >= 3) && (gameData.render.scene.Height () < 670))
		x0 -= HUD_LHX (20);
	cockpit->SetFontColor (GREEN_RGBA);
	t = gameStates.app.nSDLTicks [0];
	if (t - t0 > 333) {	//update 3 times per second
		t0 = t;
		for (i = 0; i < 2; i++) 
			objCounts [i] = ObjectCount (i ? OBJ_POWERUP : OBJ_ROBOT);
		}
	if (!cockpit->ShowTextGauges () && (LoadTallyIcons () > 0)) {
		for (i = 0; i < 2; i++) {
			bmH = bmObjTally [i].Width () / 2;
			bmW = bmObjTally [i].Height () / 2;
			x = x0 - bmW - HUD_LHX (2);
			bmObjTally [i].RenderScaled (cockpit->X (x), y, bmW, bmH, I2X (1), 0, NULL);
			sprintf (szInfo, "%d", objCounts [i]);
			fontManager.Current ()->StringSize (szInfo, w, h, aw);
			x -= w + HUD_LHY (2);
			nIdTally [i] = cockpit->DrawHUDText (nIdTally + i, x, y + (bmH - h) / 2, szInfo);
			y += bmH;
			}
		}
	else {
		//y = 6 + 3 * m_nLineSpacing;
		for (i = 0; i < 2; i++) {
			sprintf (szInfo, "%s: %5d", i ? "Powerups" : "Robots", objCounts [i]);
			fontManager.Current ()->StringSize (szInfo, w, h, aw);
			nIdTally [i] = cockpit->DrawHUDText (nIdTally + i, x0 - w - HUD_LHX (2), y, szInfo);
			y += m_nLineSpacing;
			}
		}
	}
}

//	-----------------------------------------------------------------------------

void CHUDIcons::ToggleWeaponIcons (void)
{
for (int i = 0; i < controls [0].toggleIconsCount; i++)
	if (gameStates.app.bNostalgia)
		extraGameInfo [0].nWeaponIcons = 0;
	else {
		extraGameInfo [0].nWeaponIcons = (extraGameInfo [0].nWeaponIcons + 1) % 5;
		if (!gameOpts->render.weaponIcons.bEquipment && (extraGameInfo [0].nWeaponIcons == 3))
			extraGameInfo [0].nWeaponIcons = 4;
		}
}

//	-----------------------------------------------------------------------------

static ubyte ammoType [2][10] = {{0, 1, 0, 0, 0, 0, 1, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

int CHUDIcons::GetWeaponState (int& bHave, int& bAvailable, int& bActive, int i, int j, int l)
{
if (i)
	bHave = (LOCALPLAYER.secondaryWeaponFlags & (1 << l));
else if (!l) {
	bHave = (LOCALPLAYER.LaserLevel ()  <= MAX_LASER_LEVEL);
	if (!bHave)
		return 0;
	}
else if (l == 5) {
	bHave = (LOCALPLAYER.LaserLevel () > MAX_LASER_LEVEL);
	if (!bHave)
		return 0;
	}
else {
	bHave = (LOCALPLAYER.primaryWeaponFlags & (1 << l));
	if (bHave && extraGameInfo [0].bSmartWeaponSwitch && ((l == 1) || (l == 2)) &&
			LOCALPLAYER.primaryWeaponFlags & (1 << (l + 5)))
		return 0;
	}

if (!bHave)
	bAvailable = 0;
else if (ammoType [i][l]) {
	int nAmmo = (i ? LOCALPLAYER.secondaryAmmo [l] : LOCALPLAYER.primaryAmmo [(l == 6) ? 1 : l]);
	bAvailable = (nAmmo > 0);
	}
else {
	bAvailable = (LOCALPLAYER.Energy () > gameData.weapons.info [l].xEnergyUsage);
	}
if (i && !bAvailable)
	bHave = 0;

if (!bHave)
	bActive = 0;
else if (i) {
	if (j < 8)
		bActive = (l == gameData.weapons.nSecondary);
	else
		bActive = (j == 8) == (bLastSecondaryWasSuper [PROXMINE_INDEX] != 0);
	}
else {
	if (l == 5)
		bActive = (bHave && (0 == gameData.weapons.nPrimary));
	else if (l)
		bActive = (l == gameData.weapons.nPrimary);
	else
		bActive = (bHave && (l == gameData.weapons.nPrimary));
	}

return 1;
}

//	-----------------------------------------------------------------------------

int CHUDIcons::GetAmmo (char* szAmmo, int i, int j, int l)
{
if (ammoType [i][l]) {
	int nAmmo = (i ? LOCALPLAYER.secondaryAmmo [l] : LOCALPLAYER.primaryAmmo [(l == 6) ? 1 : l]);
	if (!i && (l % 5 == 1)) {//Gauss/Vulcan
		nAmmo = X2I (nAmmo * (unsigned) VULCAN_AMMO_SCALE);
		if (nAmmo && (nAmmo < 1000)) {
			sprintf (szAmmo, ".%d", nAmmo / 100);
			return RED_RGBA;
			}
		else
			sprintf (szAmmo, "%d", nAmmo / 1000);
		}
	else
		sprintf (szAmmo, "%d", nAmmo);
	}
else {
	if (l == 0) {//Lasers
		sprintf (szAmmo, "%d", (LOCALPLAYER.LaserLevel () > MAX_LASER_LEVEL) ? MAX_LASER_LEVEL + 1 : LOCALPLAYER.LaserLevel () + 1);
		}
	else if ((l == 5) && (LOCALPLAYER.LaserLevel () > MAX_LASER_LEVEL)) {
		sprintf (szAmmo, "%d", LOCALPLAYER.LaserLevel () - MAX_LASER_LEVEL);
		}
	}
return GREEN_RGBA;
}

//	-----------------------------------------------------------------------------

int CHUDIcons::GetWeaponIndex (int i, int j, int& nMaxAutoSelect)
{
	static int nLvlMap [2][10] = {{9, 4, 8, 3, 7, 2, 6, 1, 5, 0}, {4, 3, 2, 1, 0, 4, 3, 2, 1, 0}};

if (gameOpts->render.weaponIcons.nSort && !gameStates.app.bD1Mission) {
	int l = nWeaponOrder [i][j];
	if (l == 255)
		nMaxAutoSelect = j;
	return nWeaponOrder [i][j + (j >= nMaxAutoSelect)];
	}
return nLvlMap [gameStates.app.bD1Mission][j];
}

//	-----------------------------------------------------------------------------

CBitmap* CHUDIcons::LoadWeaponIcon (int i, int l)
{
	CBitmap * bmP, * bmoP;

int m = i ? secondaryWeaponToWeaponInfo [l] : primaryWeaponToWeaponInfo [l];
if ((gameData.pig.tex.nHamFileVersion >= 3) && gameStates.video.nDisplayMode) {
	LoadTexture (gameData.weapons.info [m].hiresPicture.index, 0);
	bmP = gameData.pig.tex.bitmaps [0] + gameData.weapons.info [m].hiresPicture.index;
	}
else {
	bmP = gameData.pig.tex.bitmaps [0] + gameData.weapons.info [m].picture.index;
	LoadTexture (gameData.weapons.info [m].picture.index, 0);
	}

if ((bmoP = bmP->HasOverride ()))
	bmP = bmoP;
return bmP;
}

//	-----------------------------------------------------------------------------

void CHUDIcons::SetWeaponFillColor (int bHave, int bAvailable, float alpha)
{
if (bHave) {
	if (bAvailable)
		if (gameOpts->app.bColorblindFriendly)
			gameData.render.viewport.SetColorRGB (0, 192, 255, ubyte (alpha * 16));
		else
			gameData.render.viewport.SetColorRGB (255, 192, 0, ubyte (alpha * 16));
	else
		gameData.render.viewport.SetColorRGB (128, 0, 0, ubyte (alpha * 16));
	}
else
	gameData.render.viewport.SetColorRGB (64, 64, 64, (ubyte) (159 + alpha * 12));
}

//	-----------------------------------------------------------------------------

void CHUDIcons::SetWeaponFrameColor (int bHave, int bAvailable, int bActive, float alpha)
{
if (bActive)
	if (bAvailable)
		if (gameOpts->app.bColorblindFriendly)
			gameData.render.viewport.SetColorRGB (0, 192, 255, 255);
		else
			gameData.render.viewport.SetColorRGB (255, 192, 0, 255);
	else
		gameData.render.viewport.SetColorRGB (160, 0, 0, 255);
else if (bHave)
	if (bAvailable)
		gameData.render.viewport.SetColorRGB (0, 160, 0, 255);
	else
		gameData.render.viewport.SetColorRGB (96, 0, 0, 255);
else
	gameData.render.viewport.SetColorRGB (64, 64, 64, 255);
}

//	-----------------------------------------------------------------------------

#define ICON_SCALE	3

void CHUDIcons::DrawWeapons (void)
{
	int	nWeaponIcons = /*(gameStates.render.cockpit.nType == CM_STATUS_BAR) ? 3 :*/ extraGameInfo [0].nWeaponIcons;
	int	nIconScale = (gameOpts->render.weaponIcons.bSmall || (gameStates.render.cockpit.nType != CM_FULL_SCREEN)) ? 4 : 3;
	int	nIconPos = nWeaponIcons - 1;
	int	nHiliteColor = gameOpts->app.bColorblindFriendly;
	int	nMaxAutoSelect;
	int	nDmgIconWidth = 0;
	int	nOffsetSave = gameData.SetStereoOffsetType (STEREO_OFFSET_FIXED);
#if 0
								((nWeaponIcons == 2) 
								 && ((gameStates.app.nSDLTicks [0] - OBJECTS [LOCALPLAYER.nObject].TimeLastRepaired () > 3000) || 
								     gameData.objs.consoleP->CriticalDamage ()))) ? 32 : 0;
#endif
	int	ox = 6, 
			oy = 6, 
			x, dx, y = 0, dy = 0;
	//float	fLineWidth = (gameData.render.scene.Width () >= 1200) ? 2.0f : 1.0f;
	float	fLineWidth = float (gameData.render.scene.Width ()) / 640.0f;
	ubyte	alpha = gameOpts->render.weaponIcons.alpha;
	uint	nAmmoColor;
	char	szAmmo [10];

	static int	wIcon = 0, 
					hIcon = 0;
	static int	w = -1, 
					h = -1;
	static int bInitIcons = 1;
	static int nIdIcons [2][10] = {{0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0}};

if (gameOpts->render.weaponIcons.bShowAmmo) {
	fontManager.SetCurrent (SMALL_FONT);
	fontManager.SetColorRGBi (GREEN_RGBA, 1, 0, 0);
	}
dx = (int) (10 * m_xScale);
#if DBG
if (ogl.IsSideBySideDevice ()  || (nWeaponIcons < 3)) {
#else
if (ogl.IsOculusRift () || (nWeaponIcons < 3)) {
#endif
	dy = (gameData.render.frame.Height () - gameData.render.scene.Height ());
	y = 3 * gameData.render.frame.Height () / 4 - dy - 4 * oy;
	}
else if (nWeaponIcons < 3) {
	dy = (gameData.render.frame.Height () - gameData.render.scene.Height ());
	y = nIconPos ? gameData.render.frame.Height () - dy - oy : oy + hIcon + 12;
	}
if (extraGameInfo [0].nWeaponIcons == 1)
	y += gameData.render.scene.Top ();
else if (extraGameInfo [0].nWeaponIcons == 2) {
	y += gameData.render.scene.Top ();
	if (gameStates.render.cockpit.nType == CM_FULL_COCKPIT)
		y -= cockpit->LHX (10);
	}

for (int i = 0; i < 2; i++) {
	int n = (gameStates.app.bD1Mission) ? 5 : 10;
	nMaxAutoSelect = 255;
#if DBG
	if (ogl.IsSideBySideDevice ()  || (nWeaponIcons < 3)) {
#else
	if (ogl.IsOculusRift () || (nWeaponIcons < 3)) {
#endif
		if (!i) {
			int nBombType, bHaveBombs = cockpit->BombCount (nBombType) > 0;
			x = gameData.render.frame.Width () / 2 - (2 + bHaveBombs) * (wIcon + ox) / 2;
			}
		}
	else if (nWeaponIcons < 3) {
		x = gameData.render.frame.Width () / 2;
		if (i)
			x += dx + nDmgIconWidth;
		else
			x -= dx + wIcon + nDmgIconWidth;
		}
	else {
		int h = 0;
		y = (gameData.render.scene.Height () - h - n * (hIcon + oy)) / 2 + hIcon;
		x = i ? gameData.render.frame.Width () - wIcon - ox : ox;
		y += gameData.render.scene.Top ();
		}

	for (int j = 0; j < n; j++) {
		int bActive, bHave, bAvailable;
		int fw, fh, faw;
		int l = GetWeaponIndex (i, j, nMaxAutoSelect);
		CBitmap* bmP = LoadWeaponIcon (i, l);

		Assert (bmP != NULL);
		if (w < bmP->Width ())
			w = bmP->Width ();
		if (h < bmP->Height ())
			h = bmP->Height ();
		wIcon = (int) ((w + nIconScale - 1) / nIconScale * m_xScale);
		hIcon = (int) ((h + nIconScale - 1) / nIconScale * m_yScale);

		if (bInitIcons)
			continue;
		if (!GetWeaponState (bHave, bAvailable, bActive, i, j, l))
			continue;

#if DBG
		if (ogl.IsSideBySideDevice () && !bActive) 
#else
		if (ogl.IsOculusRift () && !bActive) 
#endif
			continue;

		cockpit->BitBlt (-1, nIconScale * (x + (w - bmP->Width ()) / (2 * nIconScale)), nIconScale * (y - hIcon), false, true, I2X (nIconScale), 0, bmP);

		nAmmoColor = GREEN_RGBA;
		*szAmmo = '\0';
		if (gameOpts->render.weaponIcons.bShowAmmo && bHave && bAvailable) {
			nAmmoColor = GetAmmo (szAmmo, i, j, l);
			fontManager.Current ()->StringSize (szAmmo, fw, fh, faw);
			}

#if DBG
		if (!ogl.IsSideBySideDevice ())
#else
		if (!ogl.IsOculusRift ()) 
#endif
			{
			SetWeaponFillColor (bHave, bAvailable, alpha);
			OglDrawFilledRect (cockpit->X (x - 1), y - hIcon - 1, cockpit->X (x + wIcon + 2), y + 2);
			SetWeaponFrameColor (bHave, bAvailable, bActive, alpha);
			glLineWidth ((bActive && bAvailable && gameOpts->render.weaponIcons.bBoldHighlight) ? fLineWidth + 2 : fLineWidth);
			OglDrawEmptyRect (cockpit->X (x - 1), y - hIcon - 1, cockpit->X (x + wIcon + 2), y + 2);
			}
		if (*szAmmo) {
			fontManager.SetColorRGBi (nAmmoColor, 1, 0, 0);
			nIdIcons [i][j] = GrString (x + wIcon + 2 - fw, y - fh, szAmmo, nIdIcons [i] + j);
			fontManager.SetColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
			}
		gameStates.render.grAlpha = 1.0f;

#if DBG
		if (ogl.IsSideBySideDevice ()  || (nWeaponIcons < 3))
#else
		if (ogl.IsOculusRift () || (nWeaponIcons < 3))
#endif
			x += wIcon + ox;
		else if (nWeaponIcons > 2)
			y += hIcon + oy;
		else {
			if (i)
				x += wIcon + ox;
			else
				x -= wIcon + ox;
			}
		}
	}
bInitIcons = 0;
gameData.SetStereoOffsetType (nOffsetSave);
}

//	-----------------------------------------------------------------------------

int CHUDIcons::LoadInventoryIcons (void)
{
	int		h, i;
	ubyte*	buffer;

if (!((bmpInventory = PiggyLoadBitmap ("inventry.bmp")) ||
	   (bmpInventory = PiggyLoadBitmap ("inventory.bmp"))))
	return bHaveInvBms = 0;
memset (bmInvItems, 0, sizeof (bmInvItems));
h = bmpInventory->Width () * bmpInventory->Width ();
buffer = bmpInventory->Buffer ();
CPalette* palette = paletteManager.Load ("groupa.256", NULL);
for (i = 0; i < NUM_INV_ITEMS; i++) {
	bmInvItems [i] = *bmpInventory;
	bmInvItems [i].SetName ("Inventory");
	bmInvItems [i].SetHeight (bmInvItems [i].Width ());
	bmInvItems [i].SetBuffer (buffer + h * i, 1, h);
	bmInvItems [i].SetTranspType (3);
	bmInvItems [i].ResetTexture ();
	bmInvItems [i].SetPalette (palette ? palette : paletteManager.Game ());
	}
return bHaveInvBms = 1;
}

//	-----------------------------------------------------------------------------

void CHUDIcons::DestroyInventoryIcons (void)
{
if (bmpInventory) {
	delete bmpInventory;
	bmpInventory = NULL;
	bHaveInvBms = -1;
	}
}

//	-----------------------------------------------------------------------------

int CHUDIcons::EquipmentActive (int bFlag)
{
switch (bFlag) {
	case PLAYER_FLAGS_AFTERBURNER:
		return (gameData.physics.xAfterburnerCharge && controls [0].afterburnerState);
	case PLAYER_FLAGS_CONVERTER:
		return gameStates.app.bUsingConverter;
	case PLAYER_FLAGS_HEADLIGHT:
		return HeadlightIsOn (-1) != 0;
	case PLAYER_FLAGS_FULLMAP:
		return 0;
	case PLAYER_FLAGS_AMMO_RACK:
		return 0;
	case PLAYER_FLAGS_QUAD_LASERS:
		return 0;
	case PLAYER_FLAGS_CLOAKED:
		return (LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED) != 0;
	case PLAYER_FLAGS_INVULNERABLE:
		return (LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) != 0;
	case PLAYER_FLAGS_SLOWMOTION:
		return SlowMotionActive ();
	case PLAYER_FLAGS_BULLETTIME:
		return BulletTimeActive ();
	}
return 0;
}

//	-----------------------------------------------------------------------------

void CHUDIcons::DrawInventory (void)
{
if (ogl.IsOculusRift ())
	return;

	CBitmap*	bmP;
	char		szCount [20];
	int		nIconScale = (gameOpts->render.weaponIcons.bSmall || (gameStates.render.cockpit.nType != CM_FULL_SCREEN)) ? 3 : 2;
	int		nIconPos = extraGameInfo [0].nWeaponIcons & 1;
	int		nHiliteColor = gameOpts->app.bColorblindFriendly;
	int		fw, fh, faw;
	int		j, n, firstItem, 
				oy = 6, 
				ox = 6, 
				x, y, dy;
	int		w = bmpInventory->Width (), 
				h = bmpInventory->Width ();
	int		wIcon = (int) ((w + nIconScale - 1) / nIconScale * m_xScale), 
				hIcon = (int) ((h + nIconScale - 1) / nIconScale * m_yScale);
	int		nDmgIconWidth = 0;
#if 0
									(nIconPos
									 && ((gameStates.app.nSDLTicks [0] - OBJECTS [LOCALPLAYER.nObject].TimeLastRepaired () > 3000) || 
									     gameData.objs.consoleP->CriticalDamage ()))) ? 80 : 0;
#endif
	float		fLineWidth = float (gameData.render.scene.Width ()) / 640.0f;
	ubyte		alpha = gameOpts->render.weaponIcons.alpha;

	static int nInvFlags [NUM_INV_ITEMS] = {
		PLAYER_FLAGS_AFTERBURNER, 
		PLAYER_FLAGS_CONVERTER, 
		PLAYER_FLAGS_HEADLIGHT, 
		PLAYER_FLAGS_FULLMAP, 
		PLAYER_FLAGS_AMMO_RACK, 
		PLAYER_FLAGS_QUAD_LASERS, 
		PLAYER_FLAGS_CLOAKED, 
		PLAYER_FLAGS_INVULNERABLE,
		PLAYER_FLAGS_SLOWMOTION,
		PLAYER_FLAGS_BULLETTIME
		};
	static int nEnergyType [NUM_INV_ITEMS] = {I2X (1), I2X (100), 0, I2X (1), 0, I2X (1), 0, 0, I2X (1), I2X (1)};
	static int nIdItems [NUM_INV_ITEMS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

dy = (gameData.render.frame.Height () - gameData.render.scene.Height ());
#if 1
	y = nIconPos ? gameData.render.frame.Height () - dy - oy : oy + hIcon + 12;
#else
if (gameStates.render.cockpit.nType != CM_STATUS_BAR) //(!cockpit->Always ())
	y = nIconPos ? gameData.render.frame.Height () - dy - oy : oy + hIcon + 12;
else
	y = oy + hIcon + 12;
#endif
n = (gameOpts->gameplay.bInventory && (!IsMultiGame || IsCoopGame)) ? NUM_INV_ITEMS : NUM_INV_ITEMS - 2;
firstItem = gameStates.app.bD1Mission ? INV_ITEM_QUADLASERS : 0;
x = (gameData.render.frame.Width () - (n - firstItem) * wIcon - (n - 1 - firstItem) * ox - nDmgIconWidth) / 2;
if ((gameStates.render.cockpit.nType == CM_FULL_COCKPIT) && (extraGameInfo [0].nWeaponIcons & 1))
	y -= cockpit->LHX (10);
y += gameData.render.scene.Top ();
for (j = firstItem; j < n; j++) {
	int bHave, bAvailable, bActive = EquipmentActive (nInvFlags [j]);
	bmP = bmInvItems + j;
	if (j == (n - firstItem + 1) / 2)
		x += nDmgIconWidth;
	cockpit->BitBlt (-1, nIconScale * (x + (w - bmP->Width ()) / (2 * nIconScale)), nIconScale * (y - hIcon), false, true, I2X (nIconScale), 0, bmP);
	//m = 9 - j;
	*szCount = '\0';
	if (j == INV_ITEM_HEADLIGHT)
		bHave = PlayerHasHeadlight (-1);
	else if (j == INV_ITEM_INVUL) {
		if ((bHave = (LOCALPLAYER.nInvuls > 0)))
			sprintf (szCount, "%d", LOCALPLAYER.nInvuls);
		else
			bHave = LOCALPLAYER.flags & nInvFlags [j];
		}
	else if (j == INV_ITEM_CLOAK) {
		if ((bHave = (LOCALPLAYER.nCloaks > 0)))
			sprintf (szCount, "%d", LOCALPLAYER.nCloaks);
		else
			bHave = LOCALPLAYER.flags & nInvFlags [j];
		}
	else
		bHave = LOCALPLAYER.flags & nInvFlags [j];
	bAvailable = (LOCALPLAYER.Energy () > nEnergyType [j]);
#if 1
	if (bHave) {
		if (bAvailable)
			if (bActive)
				if (nHiliteColor)
					gameData.render.viewport.SetColorRGB (0, 192, 255, ubyte (alpha * 16));
				else
					gameData.render.viewport.SetColorRGB (255, 192, 0, ubyte (alpha * 16));
			else
				gameData.render.viewport.SetColorRGB (128, 128, 0, ubyte (alpha * 16));
		else
			gameData.render.viewport.SetColorRGB (128, 0, 0, ubyte (alpha * 16));
		}
	else {
		gameData.render.viewport.SetColorRGB (64, 64, 64, (ubyte) (159 + alpha * 12));
		}
	OglDrawFilledRect (cockpit->X (x - 1), y - hIcon - 1, cockpit->X (x + wIcon + 2), y + 2);
	if (bHave)
		if (bAvailable)
			if (bActive)
				if (nHiliteColor)
					gameData.render.viewport.SetColorRGB (0, 192, 255, 255);
				else
					gameData.render.viewport.SetColorRGB (255, 192, 0, 255);
			else
				gameData.render.viewport.SetColorRGB (0, 160, 0, 255);
		else
			gameData.render.viewport.SetColorRGB (96, 0, 0, 255);
	else
		gameData.render.viewport.SetColorRGB (64, 64, 64, 255);
	glLineWidth ((bActive && gameOpts->render.weaponIcons.bBoldHighlight) ? 3 : fLineWidth);
	OglDrawEmptyRect (cockpit->X (x - 1), y - hIcon - 1, cockpit->X (x + wIcon + 2), y + 2);
	if (*szCount) {
		fontManager.Current ()->StringSize (szCount, fw, fh, faw);
		fontManager.SetColorRGBi (GREEN_RGBA, 1, 0, 0);
		nIdItems [j] = GrString (x + wIcon + 2 - fw, y - fh, szCount, nIdItems + j);
		fontManager.SetColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
		}
#endif
	gameStates.render.grAlpha = 1.0f;
	x += wIcon + ox;
	}
}

//	-----------------------------------------------------------------------------

void CHUDIcons::Render (void)
{
if (gameStates.app.bNostalgia)
	return;
if (gameStates.app.bEndLevelSequence)
	return;
if (gameStates.render.bRearView)
	return;
#if 0
if (gameData.render.frame.Left () || gameData.render.frame.Top ())
	return;	// render window has been shrunk
#endif
if ((gameOpts->render.cockpit.bHUD) || cockpit->ShowAlways ()) {
	m_nLineSpacing = cockpit->LineSpacing ();
	if (!(gameStates.render.bRearView || gameStates.render.bChaseCam || (gameStates.render.bFreeCam > 0)))
		DrawTally ();
	if (!gameStates.app.bDemoData && EGI_FLAG (nWeaponIcons, 1, 1, 0)) {
		m_xScale = cockpit->XScale () * HUD_ASPECT;
		m_yScale = cockpit->YScale ();
		cockpit->SetScales (m_xScale, m_yScale);
		DrawWeapons ();
		if (gameOpts->render.weaponIcons.bEquipment) {
			if (bHaveInvBms < 0)
				LoadInventoryIcons ();
			if (bHaveInvBms > 0)
				DrawInventory ();
			}
		m_xScale /= HUD_ASPECT;
		cockpit->SetScales (m_xScale, m_yScale);
		glLineWidth (1);
		}
	}
}

//	-----------------------------------------------------------------------------

void CHUDIcons::Destroy (void)
{
DestroyTallyIcons ();
DestroyInventoryIcons ();
}

//	-----------------------------------------------------------------------------
