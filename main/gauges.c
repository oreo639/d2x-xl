/* $Id: gauges.c, v 1.10 2003/10/11 09:28:38 btb Exp $ */
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

#include "hudmsg.h"

#include "inferno.h"
#include "game.h"
#include "screens.h"
#include "gauges.h"
#include "physics.h"
#include "error.h"

#include "menu.h"			// For the font.
#include "mono.h"
#include "collide.h"
#include "newdemo.h"
#include "player.h"
#include "gamefont.h"
#include "bm.h"
#include "text.h"
#include "powerup.h"
#include "sounds.h"
#include "multi.h"
#include "network.h"
#include "endlevel.h"
#include "cntrlcen.h"
#include "controls.h"
#include "weapon.h"
#include "globvars.h"

#include "wall.h"
#include "text.h"
#include "render.h"
#include "piggy.h"
#include "laser.h"
#include "grdef.h"
#include "gr.h"
#include "gamepal.h"
#include "input.h"
#include "object.h"
#include "ogl_init.h"

#define SHOW_PLAYER_IP		0

void DrawAmmoInfo (int x, int y, int ammoCount, int primary);
extern void DrawGuidedCrosshair (void);
extern void DoAutomap (int key_code, int bRadar);

double cmScaleX, cmScaleY;

#define HUD_ASPECT	((double) grdCurScreen->sc_h / (double) grdCurScreen->sc_w / 0.75)

tBitmapIndex Gauges [MAX_GAUGE_BMS];   // Array of all gauge bitmaps.
tBitmapIndex Gauges_hires [MAX_GAUGE_BMS];   // hires gauges

grs_canvas *Canv_LeftEnergyGauge;
grs_canvas *Canv_AfterburnerGauge;
grs_canvas *Canv_SBEnergyGauge;
grs_canvas *Canv_SBAfterburnerGauge;
grs_canvas *Canv_RightEnergyGauge;
grs_canvas *Canv_NumericalGauge;

#define NUM_INV_ITEMS			8
#define INV_ITEM_QUADLASERS	5
#define INV_ITEM_CLOAK			6
#define INV_ITEM_INVUL			7

grsBitmap	*bmpInventory = NULL;
grsBitmap	bmInvItems [NUM_INV_ITEMS];
grsBitmap	bmObjTally [2];

int bHaveInvBms = -1;
int bHaveObjTallyBms = -1;

//Flags for gauges/hud stuff
//bitmap numbers for gauges

#define GAUGE_SHIELDS			0		//0..9, in decreasing order (100%, 90%...0%)

#define GAUGE_INVULNERABLE		10		//10..19
#define N_INVULNERABLE_FRAMES	10

#define GAUGE_AFTERBURNER   	20
#define GAUGE_ENERGY_LEFT		21
#define GAUGE_ENERGY_RIGHT		22
#define GAUGE_NUMERICAL			23

#define GAUGE_BLUE_KEY			24
#define GAUGE_GOLD_KEY			25
#define GAUGE_RED_KEY			26
#define GAUGE_BLUE_KEY_OFF		27
#define GAUGE_GOLD_KEY_OFF		28
#define GAUGE_RED_KEY_OFF		29

#define SB_GAUGE_BLUE_KEY		30
#define SB_GAUGE_GOLD_KEY		31
#define SB_GAUGE_RED_KEY		32
#define SB_GAUGE_BLUE_KEY_OFF	33
#define SB_GAUGE_GOLD_KEY_OFF	34
#define SB_GAUGE_RED_KEY_OFF	35

#define SB_GAUGE_ENERGY			36

#define GAUGE_LIVES				37	

#define GAUGE_SHIPS				38
#define GAUGE_SHIPS_LAST		45

#define RETICLE_CROSS			46
#define RETICLE_PRIMARY			48
#define RETICLE_SECONDARY		51
#define RETICLE_LAST				55

#define GAUGE_HOMING_WARNING_ON	56
#define GAUGE_HOMING_WARNING_OFF	57

#define SML_RETICLE_CROSS		58
#define SML_RETICLE_PRIMARY	60
#define SML_RETICLE_SECONDARY	63
#define SML_RETICLE_LAST		67

#define KEY_ICON_BLUE			68
#define KEY_ICON_YELLOW			69
#define KEY_ICON_RED				70

#define SB_GAUGE_AFTERBURNER	71

#define FLAG_ICON_RED			72
#define FLAG_ICON_BLUE			73


/* Use static inline function under GCC to avoid CR/LF issues */
#ifdef __GNUC__
#define PAGE_IN_GAUGE(x) _page_in_gauge (x)
static inline void _page_in_gauge (int x)
{
PIGGY_PAGE_IN (gameStates.render.fonts.bHires ? Gauges_hires [x] : Gauges [x], 0);
}

#else
#define PAGE_IN_GAUGE(x)	PIGGY_PAGE_IN (gameStates.render.fonts.bHires ? Gauges_hires [x] : Gauges [x], 0);	

#endif

#define GET_GAUGE_INDEX(x)	 (gameStates.render.fonts.bHires?Gauges_hires [x].index:Gauges [x].index)

//change MAX_GAUGE_BMS when adding gauges

//Coordinats for gauges

#define GAUGE_BLUE_KEY_X_L		272
#define GAUGE_BLUE_KEY_Y_L		152
#define GAUGE_BLUE_KEY_X_H		535
#define GAUGE_BLUE_KEY_Y_H		374
#define GAUGE_BLUE_KEY_X		 (gameStates.video.nDisplayMode?GAUGE_BLUE_KEY_X_H:GAUGE_BLUE_KEY_X_L)
#define GAUGE_BLUE_KEY_Y		 (gameStates.video.nDisplayMode?GAUGE_BLUE_KEY_Y_H:GAUGE_BLUE_KEY_Y_L)

#define GAUGE_GOLD_KEY_X_L		273
#define GAUGE_GOLD_KEY_Y_L		162
#define GAUGE_GOLD_KEY_X_H		537
#define GAUGE_GOLD_KEY_Y_H		395
#define GAUGE_GOLD_KEY_X		 (gameStates.video.nDisplayMode?GAUGE_GOLD_KEY_X_H:GAUGE_GOLD_KEY_X_L)
#define GAUGE_GOLD_KEY_Y		 (gameStates.video.nDisplayMode?GAUGE_GOLD_KEY_Y_H:GAUGE_GOLD_KEY_Y_L)

#define GAUGE_RED_KEY_X_L		274
#define GAUGE_RED_KEY_Y_L		172
#define GAUGE_RED_KEY_X_H		539
#define GAUGE_RED_KEY_Y_H		416
#define GAUGE_RED_KEY_X			 (gameStates.video.nDisplayMode?GAUGE_RED_KEY_X_H:GAUGE_RED_KEY_X_L)
#define GAUGE_RED_KEY_Y			 (gameStates.video.nDisplayMode?GAUGE_RED_KEY_Y_H:GAUGE_RED_KEY_Y_L)

// status bar keys

#define SB_GAUGE_KEYS_X_L	   11
#define SB_GAUGE_KEYS_X_H		26
#define SB_GAUGE_KEYS_X			 (gameStates.video.nDisplayMode?SB_GAUGE_KEYS_X_H:SB_GAUGE_KEYS_X_L)

#define SB_GAUGE_BLUE_KEY_Y_L		153
#define SB_GAUGE_GOLD_KEY_Y_L		169
#define SB_GAUGE_RED_KEY_Y_L		185

#define SB_GAUGE_BLUE_KEY_Y_H		390
#define SB_GAUGE_GOLD_KEY_Y_H		422
#define SB_GAUGE_RED_KEY_Y_H		454

#define SB_GAUGE_BLUE_KEY_Y	 (gameStates.video.nDisplayMode?SB_GAUGE_BLUE_KEY_Y_H:SB_GAUGE_BLUE_KEY_Y_L)
#define SB_GAUGE_GOLD_KEY_Y	 (gameStates.video.nDisplayMode?SB_GAUGE_GOLD_KEY_Y_H:SB_GAUGE_GOLD_KEY_Y_L)
#define SB_GAUGE_RED_KEY_Y		 (gameStates.video.nDisplayMode?SB_GAUGE_RED_KEY_Y_H:SB_GAUGE_RED_KEY_Y_L)

// cockpit enery gauges

#define LEFT_ENERGY_GAUGE_X_L 	70
#define LEFT_ENERGY_GAUGE_Y_L 	131
#define LEFT_ENERGY_GAUGE_W_L 	64
#define LEFT_ENERGY_GAUGE_H_L 	8

#define LEFT_ENERGY_GAUGE_X_H 	137
#define LEFT_ENERGY_GAUGE_Y_H 	314
#define LEFT_ENERGY_GAUGE_W_H 	133
#define LEFT_ENERGY_GAUGE_H_H 	21

#define LEFT_ENERGY_GAUGE_X 	 (gameStates.video.nDisplayMode?LEFT_ENERGY_GAUGE_X_H:LEFT_ENERGY_GAUGE_X_L)
#define LEFT_ENERGY_GAUGE_Y 	 (gameStates.video.nDisplayMode?LEFT_ENERGY_GAUGE_Y_H:LEFT_ENERGY_GAUGE_Y_L)
#define LEFT_ENERGY_GAUGE_W 	 (gameStates.video.nDisplayMode?LEFT_ENERGY_GAUGE_W_H:LEFT_ENERGY_GAUGE_W_L)
#define LEFT_ENERGY_GAUGE_H 	 (gameStates.video.nDisplayMode?LEFT_ENERGY_GAUGE_H_H:LEFT_ENERGY_GAUGE_H_L)

#define RIGHT_ENERGY_GAUGE_X 	 (gameStates.video.nDisplayMode?380:190)
#define RIGHT_ENERGY_GAUGE_Y 	 (gameStates.video.nDisplayMode?314:131)
#define RIGHT_ENERGY_GAUGE_W 	 (gameStates.video.nDisplayMode?133:64)
#define RIGHT_ENERGY_GAUGE_H 	 (gameStates.video.nDisplayMode?21:8)

// cockpit afterburner gauge

#define AFTERBURNER_GAUGE_X_L	45-1
#define AFTERBURNER_GAUGE_Y_L	158
#define AFTERBURNER_GAUGE_W_L	12
#define AFTERBURNER_GAUGE_H_L	32

#define AFTERBURNER_GAUGE_X_H	88
#define AFTERBURNER_GAUGE_Y_H	377
#define AFTERBURNER_GAUGE_W_H	21
#define AFTERBURNER_GAUGE_H_H	65

#define AFTERBURNER_GAUGE_X	 (gameStates.video.nDisplayMode?AFTERBURNER_GAUGE_X_H:AFTERBURNER_GAUGE_X_L)
#define AFTERBURNER_GAUGE_Y	 (gameStates.video.nDisplayMode?AFTERBURNER_GAUGE_Y_H:AFTERBURNER_GAUGE_Y_L)
#define AFTERBURNER_GAUGE_W	 (gameStates.video.nDisplayMode?AFTERBURNER_GAUGE_W_H:AFTERBURNER_GAUGE_W_L)
#define AFTERBURNER_GAUGE_H	 (gameStates.video.nDisplayMode?AFTERBURNER_GAUGE_H_H:AFTERBURNER_GAUGE_H_L)

// sb afterburner gauge

#define SB_AFTERBURNER_GAUGE_X 		 (gameStates.video.nDisplayMode?196:98)
#define SB_AFTERBURNER_GAUGE_Y 		 (gameStates.video.nDisplayMode?446:184)
#define SB_AFTERBURNER_GAUGE_W 		 (gameStates.video.nDisplayMode?33:16)
#define SB_AFTERBURNER_GAUGE_H 		 (gameStates.video.nDisplayMode?29:13)

// sb energy gauge

#define SB_ENERGY_GAUGE_X 		 (gameStates.video.nDisplayMode?196:98)
#define SB_ENERGY_GAUGE_Y 		 (gameStates.video.nDisplayMode?382: (155-2))
#define SB_ENERGY_GAUGE_W 		 (gameStates.video.nDisplayMode?32:16)
#define SB_ENERGY_GAUGE_H 		 ((gameStates.video.nDisplayMode?60:29) + (gameStates.app.bD1Mission ? SB_AFTERBURNER_GAUGE_H : 0))

#define SB_ENERGY_NUM_X 		 (SB_ENERGY_GAUGE_X+ (gameStates.video.nDisplayMode?4:2))
#define SB_ENERGY_NUM_Y 		 (gameStates.video.nDisplayMode?457:175)

#define SHIELD_GAUGE_X 			 (gameStates.video.nDisplayMode?292:146)
#define SHIELD_GAUGE_Y			 (gameStates.video.nDisplayMode?374:155)
#define SHIELD_GAUGE_W 			 (gameStates.video.nDisplayMode?70:35)
#define SHIELD_GAUGE_H			 (gameStates.video.nDisplayMode?77:32)

#define SHIP_GAUGE_X 			 (SHIELD_GAUGE_X+ (gameStates.video.nDisplayMode?11:5))
#define SHIP_GAUGE_Y				 (SHIELD_GAUGE_Y+ (gameStates.video.nDisplayMode?10:5))

#define SB_SHIELD_GAUGE_X 		 (gameStates.video.nDisplayMode?247:123)		//139
#define SB_SHIELD_GAUGE_Y 		 (gameStates.video.nDisplayMode?395:163)

#define SB_SHIP_GAUGE_X 		 (SB_SHIELD_GAUGE_X+ (gameStates.video.nDisplayMode?11:5))
#define SB_SHIP_GAUGE_Y 		 (SB_SHIELD_GAUGE_Y+ (gameStates.video.nDisplayMode?10:5))

#define SB_SHIELD_NUM_X 		 (SB_SHIELD_GAUGE_X+ (gameStates.video.nDisplayMode?21:12))	//151
#define SB_SHIELD_NUM_Y 		 (SB_SHIELD_GAUGE_Y- (gameStates.video.nDisplayMode?16:8))			//156 -- MWA used to be hard coded to 156

#define NUMERICAL_GAUGE_X		 (gameStates.video.nDisplayMode?308:154)
#define NUMERICAL_GAUGE_Y		 (gameStates.video.nDisplayMode?316:130)
#define NUMERICAL_GAUGE_W		 (gameStates.video.nDisplayMode?38:19)
#define NUMERICAL_GAUGE_H		 (gameStates.video.nDisplayMode?55:22)

#define PRIMARY_W_PIC_X			 (gameStates.video.nDisplayMode? (135-10):64)
#define PRIMARY_W_PIC_Y			 (gameStates.video.nDisplayMode?370:154)
#define PRIMARY_W_TEXT_X		 (gameStates.video.nDisplayMode?182:87)
#define PRIMARY_W_TEXT_Y		 (gameStates.video.nDisplayMode?400:157)
#define PRIMARY_AMMO_X			 (gameStates.video.nDisplayMode?186: (96-3))
#define PRIMARY_AMMO_Y			 (gameStates.video.nDisplayMode?420:171)

#define SECONDARY_W_PIC_X		 (gameStates.video.nDisplayMode?466:234)
#define SECONDARY_W_PIC_Y		 (gameStates.video.nDisplayMode?374:154)
#define SECONDARY_W_TEXT_X		 (gameStates.video.nDisplayMode?413:207)
#define SECONDARY_W_TEXT_Y		 (gameStates.video.nDisplayMode?378:157)
#define SECONDARY_AMMO_X		 (gameStates.video.nDisplayMode?428:213)
#define SECONDARY_AMMO_Y		 (gameStates.video.nDisplayMode?407:171)

#define SB_LIVES_X				 (gameStates.video.nDisplayMode? (550-10-3):266)
#define SB_LIVES_Y				 (gameStates.video.nDisplayMode?450-3:185)
#define SB_LIVES_LABEL_X		 (gameStates.video.nDisplayMode?475:237)
#define SB_LIVES_LABEL_Y		 (SB_LIVES_Y+1)

#define SB_SCORE_RIGHT_L		301
#define SB_SCORE_RIGHT_H		 (605+8)
#define SB_SCORE_RIGHT			 (gameStates.video.nDisplayMode?SB_SCORE_RIGHT_H:SB_SCORE_RIGHT_L)

#define SB_SCORE_Y				 (gameStates.video.nDisplayMode?398:158)
#define SB_SCORE_LABEL_X		 (gameStates.video.nDisplayMode?475:237)

#define SB_SCORE_ADDED_RIGHT	 (gameStates.video.nDisplayMode?SB_SCORE_RIGHT_H:SB_SCORE_RIGHT_L)
#define SB_SCORE_ADDED_Y		 (gameStates.video.nDisplayMode?413:165)

#define HOMING_WARNING_X		 (gameStates.video.nDisplayMode?14:7)
#define HOMING_WARNING_Y		 (gameStates.video.nDisplayMode?415:171)

#define BOMB_COUNT_X			 (gameStates.video.nDisplayMode?546:275)
#define BOMB_COUNT_Y			 (gameStates.video.nDisplayMode?445:186)

#define SB_BOMB_COUNT_X			 (gameStates.video.nDisplayMode?342:171)
#define SB_BOMB_COUNT_Y			 (gameStates.video.nDisplayMode?458:191)

#define LHX(x)      (gameStates.menus.bHires?2* (x):x)
#define LHY(y)      (gameStates.menus.bHires? (24* (y))/10:y)

static int score_display [2];
static fix scoreTime;

static int old_score [2]			= {-1, -1};
static int old_energy [2]			= {-1, -1};
static int old_shields [2]			= {-1, -1};
static uint oldFlags [2]			= {(uint) -1, (uint) -1};
static int old_weapon [2][2]		= {{-1, -1}, {-1, -1}};
static int old_ammoCount [2][2]	= {{-1, -1}, {-1, -1}};
static int Old_Omega_charge [2]	= {-1, -1};
static int old_laserLevel [2]		= {-1, -1};
static int old_cloak [2]			= {0, 0};
static int old_lives [2]			= {-1, -1};
static fix old_afterburner [2]	= {-1, -1};
static int oldBombcount [2]		= {0, 0};

static int invulnerable_frame = 0;

static int nCloakFadeState;		//0=steady, -1 fading out, 1 fading in 

#define WS_SET				0		//in correct state
#define WS_FADING_OUT	1
#define WS_FADING_IN		2

int weapon_box_user [2]={WBU_WEAPON, WBU_WEAPON};		//see WBU_ constants in gauges.h
int weapon_boxStates [2];
fix weapon_box_fadeValues [2];

#define FADE_SCALE	 (2*i2f (GR_ACTUAL_FADE_LEVELS)/REARM_TIME)		// fade out and back in REARM_TIME, in fade levels per seconds (int)

typedef struct span {
	sbyte l, r;
} span;

//store delta x values from left of box
span weapon_window_left [] = {		//first span 67, 151
		{8, 51}, 
		{6, 53}, 
		{5, 54}, 
		{4-1, 53+2}, 
		{4-1, 53+3}, 
		{4-1, 53+3}, 
		{4-2, 53+3}, 
		{4-2, 53+3}, 
		{3-1, 53+3}, 
		{3-1, 53+3}, 
		{3-1, 53+3}, 
		{3-1, 53+3}, 
		{3-1, 53+3}, 
		{3-1, 53+3}, 
		{3-1, 53+3}, 
		{3-2, 53+3}, 
		{2-1, 53+3}, 
		{2-1, 53+3}, 
		{2-1, 53+3}, 
		{2-1, 53+3}, 
		{2-1, 53+3}, 
		{2-1, 53+3}, 
		{2-1, 53+3}, 
		{2-1, 53+3}, 
		{1-1, 53+3}, 
		{1-1, 53+2}, 
		{1-1, 53+2}, 
		{1-1, 53+2}, 
		{1-1, 53+2}, 
		{1-1, 53+2}, 
		{1-1, 53+2}, 
		{1-1, 53+2}, 
		{0, 53+2}, 
		{0, 53+2}, 
		{0, 53+2}, 
		{0, 53+2}, 
		{0, 52+3}, 
		{1-1, 52+2}, 
		{2-2, 51+3}, 
		{3-2, 51+2}, 
		{4-2, 50+2}, 
		{5-2, 50}, 
		{5-2+2, 50-2}, 
	};


//store delta x values from left of box
span weapon_window_right [] = {		//first span 207, 154
		{208-202, 255-202}, 
		{206-202, 257-202}, 
		{205-202, 258-202}, 
		{204-202, 259-202}, 
		{203-202, 260-202}, 
		{203-202, 260-202}, 
		{203-202, 260-202}, 
		{203-202, 260-202}, 
		{203-202, 260-202}, 
		{203-202, 261-202}, 
		{203-202, 261-202}, 
		{203-202, 261-202}, 
		{203-202, 261-202}, 
		{203-202, 261-202}, 
		{203-202, 261-202}, 
		{203-202, 261-202}, 
		{203-202, 261-202}, 
		{203-202, 261-202}, 
		{203-202, 262-202}, 
		{203-202, 262-202}, 
		{203-202, 262-202}, 
		{203-202, 262-202}, 
		{203-202, 262-202}, 
		{203-202, 262-202}, 
		{203-202, 262-202}, 
		{203-202, 262-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{204-202, 263-202}, 
		{205-202, 263-202}, 
		{206-202, 262-202}, 
		{207-202, 261-202}, 
		{208-202, 260-202}, 
		{211-202, 255-202}, 
	};

//store delta x values from left of box
span weapon_window_left_hires [] = {		//first span 67, 154
	{20, 110}, 
	{18, 113}, 
	{16, 114}, 
	{15, 116}, 
	{14, 117}, 
	{13, 118}, 
	{12, 119}, 
	{11, 119}, 
	{10, 120}, 
	{10, 120}, 
	{9, 121}, 
	{8, 121}, 
	{8, 121}, 
	{8, 122}, 
	{7, 122}, 
	{7, 122}, 
	{7, 122}, 
	{7, 122}, 
	{7, 122}, 
	{6, 122}, 
	{6, 122}, 
	{6, 122}, 
	{6, 122}, 
	{6, 122}, 
	{6, 122}, 
	{6, 122}, 
	{6, 122}, 
	{6, 122}, 
	{6, 122}, 
	{5, 122}, 
	{5, 122}, 
	{5, 122}, 
	{5, 122}, 
	{5, 121}, 
	{5, 121}, 
	{5, 121}, 
	{5, 121}, 
	{5, 121}, 
	{5, 121}, 
	{4, 121}, 
	{4, 121}, 
	{4, 121}, 
	{4, 121}, 
	{4, 121}, 
	{4, 121}, 
	{4, 121}, 
	{4, 121}, 
	{4, 121}, 
	{4, 121}, 
	{3, 121}, 
	{3, 121}, 
	{3, 120}, 
	{3, 120}, 
	{3, 120}, 
	{3, 120}, 
	{3, 120}, 
	{3, 120}, 
	{3, 120}, 
	{3, 120}, 
	{3, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{2, 120}, 
	{1, 120}, 
	{1, 120}, 
	{1, 119}, 
	{1, 119}, 
	{1, 119}, 
	{1, 119}, 
	{1, 119}, 
	{1, 119}, 
	{1, 119}, 
	{1, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 118}, 
	{0, 118}, 
	{0, 118}, 
	{0, 117}, 
	{0, 117}, 
	{0, 117}, 
	{1, 116}, 
	{1, 116}, 
	{2, 115}, 
	{2, 114}, 
	{3, 113}, 
	{4, 112}, 
	{5, 111}, 
	{5, 110}, 
	{7, 109}, 
	{9, 107}, 
	{10, 105}, 
	{12, 102}, 
};


//store delta x values from left of box
span weapon_window_right_hires [] = {		//first span 207, 154
	{12, 105}, 
	{9, 107}, 
	{8, 109}, 
	{6, 110}, 
	{5, 111}, 
	{4, 112}, 
	{3, 113}, 
	{3, 114}, 
	{2, 115}, 
	{2, 115}, 
	{1, 116}, 
	{1, 117}, 
	{1, 117}, 
	{0, 117}, 
	{0, 118}, 
	{0, 118}, 
	{0, 118}, 
	{0, 118}, 
	{0, 118}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 119}, 
	{0, 120}, 
	{0, 120}, 
	{0, 120}, 
	{0, 120}, 
	{1, 120}, 
	{1, 120}, 
	{1, 120}, 
	{1, 120}, 
	{1, 120}, 
	{1, 120}, 
	{1, 121}, 
	{1, 121}, 
	{1, 121}, 
	{1, 121}, 
	{1, 121}, 
	{1, 121}, 
	{1, 121}, 
	{1, 121}, 
	{1, 121}, 
	{1, 121}, 
	{1, 122}, 
	{1, 122}, 
	{2, 122}, 
	{2, 122}, 
	{2, 122}, 
	{2, 122}, 
	{2, 122}, 
	{2, 122}, 
	{2, 122}, 
	{2, 122}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 123}, 
	{2, 124}, 
	{2, 124}, 
	{3, 124}, 
	{3, 124}, 
	{3, 124}, 
	{3, 124}, 
	{3, 124}, 
	{3, 124}, 
	{3, 124}, 
	{3, 125}, 
	{3, 125}, 
	{3, 125}, 
	{3, 125}, 
	{3, 125}, 
	{3, 125}, 
	{3, 125}, 
	{3, 125}, 
	{4, 125}, 
	{4, 125}, 
	{4, 125}, 
	{5, 125}, 
	{5, 125}, 
	{5, 125}, 
	{6, 125}, 
	{6, 124}, 
	{7, 123}, 
	{8, 123}, 
	{9, 122}, 
	{10, 121}, 
	{11, 120}, 
	{12, 120}, 
	{13, 118}, 
	{15, 117}, 
	{18, 115}, 
	{20, 114}, 
};

											
#define N_LEFT_WINDOW_SPANS  (sizeof (weapon_window_left)/sizeof (*weapon_window_left))
#define N_RIGHT_WINDOW_SPANS (sizeof (weapon_window_right)/sizeof (*weapon_window_right))

#define N_LEFT_WINDOW_SPANS_H  (sizeof (weapon_window_left_hires)/sizeof (*weapon_window_left_hires))
#define N_RIGHT_WINDOW_SPANS_H (sizeof (weapon_window_right_hires)/sizeof (*weapon_window_right_hires))

// defining box boundries for weapon pictures

#define PRIMARY_W_BOX_LEFT_L		63
#define PRIMARY_W_BOX_TOP_L		151		//154
#define PRIMARY_W_BOX_RIGHT_L		 (PRIMARY_W_BOX_LEFT_L+58)
#define PRIMARY_W_BOX_BOT_L		 (PRIMARY_W_BOX_TOP_L+N_LEFT_WINDOW_SPANS-1)
											
#define PRIMARY_W_BOX_LEFT_H		121
#define PRIMARY_W_BOX_TOP_H		364
#define PRIMARY_W_BOX_RIGHT_H		242
#define PRIMARY_W_BOX_BOT_H		 (PRIMARY_W_BOX_TOP_H+N_LEFT_WINDOW_SPANS_H-1)		//470
											
#define PRIMARY_W_BOX_LEFT		 (gameStates.video.nDisplayMode?PRIMARY_W_BOX_LEFT_H:PRIMARY_W_BOX_LEFT_L)
#define PRIMARY_W_BOX_TOP		 (gameStates.video.nDisplayMode?PRIMARY_W_BOX_TOP_H:PRIMARY_W_BOX_TOP_L)
#define PRIMARY_W_BOX_RIGHT	 (gameStates.video.nDisplayMode?PRIMARY_W_BOX_RIGHT_H:PRIMARY_W_BOX_RIGHT_L)
#define PRIMARY_W_BOX_BOT		 (gameStates.video.nDisplayMode?PRIMARY_W_BOX_BOT_H:PRIMARY_W_BOX_BOT_L)

#define SECONDARY_W_BOX_LEFT_L	202	//207
#define SECONDARY_W_BOX_TOP_L		151
#define SECONDARY_W_BOX_RIGHT_L	263	// (SECONDARY_W_BOX_LEFT+54)
#define SECONDARY_W_BOX_BOT_L		 (SECONDARY_W_BOX_TOP_L+N_RIGHT_WINDOW_SPANS-1)

#define SECONDARY_W_BOX_LEFT_H	404
#define SECONDARY_W_BOX_TOP_H		363
#define SECONDARY_W_BOX_RIGHT_H	529
#define SECONDARY_W_BOX_BOT_H		 (SECONDARY_W_BOX_TOP_H+N_RIGHT_WINDOW_SPANS_H-1)		//470

#define SECONDARY_W_BOX_LEFT	 (gameStates.video.nDisplayMode?SECONDARY_W_BOX_LEFT_H:SECONDARY_W_BOX_LEFT_L)
#define SECONDARY_W_BOX_TOP	 (gameStates.video.nDisplayMode?SECONDARY_W_BOX_TOP_H:SECONDARY_W_BOX_TOP_L)
#define SECONDARY_W_BOX_RIGHT	 (gameStates.video.nDisplayMode?SECONDARY_W_BOX_RIGHT_H:SECONDARY_W_BOX_RIGHT_L)
#define SECONDARY_W_BOX_BOT	 (gameStates.video.nDisplayMode?SECONDARY_W_BOX_BOT_H:SECONDARY_W_BOX_BOT_L)

#define SB_PRIMARY_W_BOX_LEFT_L		34		//50
#define SB_PRIMARY_W_BOX_TOP_L		153
#define SB_PRIMARY_W_BOX_RIGHT_L		 (SB_PRIMARY_W_BOX_LEFT_L+53+2)
#define SB_PRIMARY_W_BOX_BOT_L		 (195+1)
											
#define SB_PRIMARY_W_BOX_LEFT_H		68
#define SB_PRIMARY_W_BOX_TOP_H		381
#define SB_PRIMARY_W_BOX_RIGHT_H		179
#define SB_PRIMARY_W_BOX_BOT_H		473
											
#define SB_PRIMARY_W_BOX_LEFT		 (gameStates.video.nDisplayMode?SB_PRIMARY_W_BOX_LEFT_H:SB_PRIMARY_W_BOX_LEFT_L)
#define SB_PRIMARY_W_BOX_TOP		 (gameStates.video.nDisplayMode?SB_PRIMARY_W_BOX_TOP_H:SB_PRIMARY_W_BOX_TOP_L)
#define SB_PRIMARY_W_BOX_RIGHT	 (gameStates.video.nDisplayMode?SB_PRIMARY_W_BOX_RIGHT_H:SB_PRIMARY_W_BOX_RIGHT_L)
#define SB_PRIMARY_W_BOX_BOT		 (gameStates.video.nDisplayMode?SB_PRIMARY_W_BOX_BOT_H:SB_PRIMARY_W_BOX_BOT_L)
											
#define SB_SECONDARY_W_BOX_LEFT_L	169
#define SB_SECONDARY_W_BOX_TOP_L		153
#define SB_SECONDARY_W_BOX_RIGHT_L	 (SB_SECONDARY_W_BOX_LEFT_L+54+1)
#define SB_SECONDARY_W_BOX_BOT_L		 (153+43)

#define SB_SECONDARY_W_BOX_LEFT_H	338
#define SB_SECONDARY_W_BOX_TOP_H		381
#define SB_SECONDARY_W_BOX_RIGHT_H	449
#define SB_SECONDARY_W_BOX_BOT_H		473

#define SB_SECONDARY_W_BOX_LEFT	 (gameStates.video.nDisplayMode?SB_SECONDARY_W_BOX_LEFT_H:SB_SECONDARY_W_BOX_LEFT_L)	//210
#define SB_SECONDARY_W_BOX_TOP	 (gameStates.video.nDisplayMode?SB_SECONDARY_W_BOX_TOP_H:SB_SECONDARY_W_BOX_TOP_L)
#define SB_SECONDARY_W_BOX_RIGHT	 (gameStates.video.nDisplayMode?SB_SECONDARY_W_BOX_RIGHT_H:SB_SECONDARY_W_BOX_RIGHT_L)
#define SB_SECONDARY_W_BOX_BOT	 (gameStates.video.nDisplayMode?SB_SECONDARY_W_BOX_BOT_H:SB_SECONDARY_W_BOX_BOT_L)

#define SB_PRIMARY_W_PIC_X			 (SB_PRIMARY_W_BOX_LEFT+1)	//51
#define SB_PRIMARY_W_PIC_Y			 (gameStates.video.nDisplayMode?382:154)
#define SB_PRIMARY_W_TEXT_X		 (SB_PRIMARY_W_BOX_LEFT+ (gameStates.video.nDisplayMode?50:24))	// (51+23)
#define SB_PRIMARY_W_TEXT_Y		 (gameStates.video.nDisplayMode?390:157)
#define SB_PRIMARY_AMMO_X			 (SB_PRIMARY_W_BOX_LEFT+ (gameStates.video.nDisplayMode? (38+20):30))	// ((SB_PRIMARY_W_BOX_LEFT+33)-3)	// (51+32)
#define SB_PRIMARY_AMMO_Y			 (gameStates.video.nDisplayMode?410:171)

#define SB_SECONDARY_W_PIC_X		 (gameStates.video.nDisplayMode?385: (SB_SECONDARY_W_BOX_LEFT+29))	// (212+27)
#define SB_SECONDARY_W_PIC_Y		 (gameStates.video.nDisplayMode?382:154)
#define SB_SECONDARY_W_TEXT_X		 (SB_SECONDARY_W_BOX_LEFT+2)	//212
#define SB_SECONDARY_W_TEXT_Y		 (gameStates.video.nDisplayMode?389:157)
#define SB_SECONDARY_AMMO_X		 (SB_SECONDARY_W_BOX_LEFT+ (gameStates.video.nDisplayMode? (14-4):11))	// (212+9)
#define SB_SECONDARY_AMMO_Y		 (gameStates.video.nDisplayMode?414:171)

typedef struct gauge_box {
	int left, top;
	int right, bot;		//maximal box
	span *spanlist;	//list of left, right spans for copy
} gauge_box;

gauge_box gauge_boxes [] = {

// primary left/right low res
		{PRIMARY_W_BOX_LEFT_L, PRIMARY_W_BOX_TOP_L, PRIMARY_W_BOX_RIGHT_L, PRIMARY_W_BOX_BOT_L, weapon_window_left}, 
		{SECONDARY_W_BOX_LEFT_L, SECONDARY_W_BOX_TOP_L, SECONDARY_W_BOX_RIGHT_L, SECONDARY_W_BOX_BOT_L, weapon_window_right}, 

//sb left/right low res
		{SB_PRIMARY_W_BOX_LEFT_L, SB_PRIMARY_W_BOX_TOP_L, SB_PRIMARY_W_BOX_RIGHT_L, SB_PRIMARY_W_BOX_BOT_L, NULL}, 
		{SB_SECONDARY_W_BOX_LEFT_L, SB_SECONDARY_W_BOX_TOP_L, SB_SECONDARY_W_BOX_RIGHT_L, SB_SECONDARY_W_BOX_BOT_L, NULL}, 

// primary left/right hires
		{PRIMARY_W_BOX_LEFT_H, PRIMARY_W_BOX_TOP_H, PRIMARY_W_BOX_RIGHT_H, PRIMARY_W_BOX_BOT_H, weapon_window_left_hires}, 
		{SECONDARY_W_BOX_LEFT_H, SECONDARY_W_BOX_TOP_H, SECONDARY_W_BOX_RIGHT_H, SECONDARY_W_BOX_BOT_H, weapon_window_right_hires}, 

// sb left/right hires
		{SB_PRIMARY_W_BOX_LEFT_H, SB_PRIMARY_W_BOX_TOP_H, SB_PRIMARY_W_BOX_RIGHT_H, SB_PRIMARY_W_BOX_BOT_H, NULL}, 
		{SB_SECONDARY_W_BOX_LEFT_H, SB_SECONDARY_W_BOX_TOP_H, SB_SECONDARY_W_BOX_RIGHT_H, SB_SECONDARY_W_BOX_BOT_H, NULL}, 
	};

// these macros refer to arrays above

#define COCKPIT_PRIMARY_BOX		 (!gameStates.video.nDisplayMode?0:4)
#define COCKPIT_SECONDARY_BOX		 (!gameStates.video.nDisplayMode?1:5)
#define SB_PRIMARY_BOX				 (!gameStates.video.nDisplayMode?2:6)
#define SB_SECONDARY_BOX			 (!gameStates.video.nDisplayMode?3:7)

static double xScale, yScale;

//	-----------------------------------------------------------------------------

#define HUD_SCALE(v, s)	 ((int) ((double) (v) * (s) + 0.5))
#define HUD_SCALE_X(v)	HUD_SCALE (v, cmScaleX)
#define HUD_SCALE_Y(v)	HUD_SCALE (v, cmScaleY)

//	-----------------------------------------------------------------------------

inline void HUDBitBlt (int x, int y, grsBitmap *bmP, int scale, int orient)
{
OglUBitMapMC (
	 (x < 0) ? -x : HUD_SCALE_X (x), 
	 (y < 0) ? -y : HUD_SCALE_Y (y), 
	HUD_SCALE_X (bmP->bm_props.w), 
	HUD_SCALE_Y (bmP->bm_props.h), 
	bmP, 
	NULL, 
	scale, 
	orient
	);
}

//	-----------------------------------------------------------------------------

inline void HUDStretchBlt (int x, int y, grsBitmap *bmP, int scale, int orient, double xScale, double yScale)
{
OglUBitMapMC (
	 (x < 0) ? -x : HUD_SCALE_X (x), 
	 (y < 0) ? -y : HUD_SCALE_Y (y), 
	HUD_SCALE_X ((int) (bmP->bm_props.w * xScale + 0.5)), 
	HUD_SCALE_Y ((int) (bmP->bm_props.h * yScale + 0.5)), 
	bmP, 
	NULL, 
	scale, 
	orient
	);
}

//	-----------------------------------------------------------------------------

inline void HUDRect (int left, int top, int width, int height)
{
GrRect (HUD_SCALE_X (left), HUD_SCALE_Y (top), HUD_SCALE_X (width), HUD_SCALE_Y (height));
}

//	-----------------------------------------------------------------------------

inline void HUDUScanLine (int left, int right, int y)
{
gr_uscanline (HUD_SCALE_X (left), HUD_SCALE_X (right), HUD_SCALE_Y (y));
}

//	-----------------------------------------------------------------------------

void _CDECL_ HUDPrintF (int x, int y, char *pszFmt, ...)
{
	static char szBuf [1000];
	va_list args;

	va_start (args, pszFmt);
	vsprintf (szBuf, pszFmt, args);
	GrString (HUD_SCALE_X (x), HUD_SCALE_Y (y), szBuf);
}

//	-----------------------------------------------------------------------------

//copy a box from the off-screen buffer to the visible page

void CopyGaugeBox (gauge_box *box, grsBitmap *bm)
{
#if 0
if (box->spanlist) {
	int n_spans = box->bot-box->top+1;
	int cnt, y;

for (cnt=0, y=box->top;cnt<n_spans;cnt++, y++) {
	GrBmUBitBlt (box->spanlist [cnt].r-box->spanlist [cnt].l+1, 1, 
				box->left+box->spanlist [cnt].l, y, box->left+box->spanlist [cnt].l, y, bm, &grdCurCanv->cv_bitmap);
	 	}
 	}
else {
	GrBmUBitBlt (box->right-box->left+1, box->bot-box->top+1, 
					box->left, box->top, box->left, box->top, 
					bm, &grdCurCanv->cv_bitmap);
	}
#endif
}

//	-----------------------------------------------------------------------------
//fills in the coords of the hostage video window
void GetHostageWindowCoords (int *x, int *y, int *w, int *h)
{
	if (gameStates.render.cockpit.nMode == CM_STATUS_BAR) {
		*x = SB_SECONDARY_W_BOX_LEFT;
		*y = SB_SECONDARY_W_BOX_TOP;
		*w = SB_SECONDARY_W_BOX_RIGHT - SB_SECONDARY_W_BOX_LEFT + 1;
		*h = SB_SECONDARY_W_BOX_BOT - SB_SECONDARY_W_BOX_TOP + 1;
	}
	else {
		*x = SECONDARY_W_BOX_LEFT;
		*y = SECONDARY_W_BOX_TOP;
		*w = SECONDARY_W_BOX_RIGHT - SECONDARY_W_BOX_LEFT + 1;
		*h = SECONDARY_W_BOX_BOT - SECONDARY_W_BOX_TOP + 1;
	}

}

//	-----------------------------------------------------------------------------
//these should be in gr.h 
#define cv_w  cv_bitmap.bm_props.w
#define cv_h  cv_bitmap.bm_props.h

extern fix ThisLevelTime;

void HUDShowScore ()
{
	char	score_str [20];
	int	w, h, aw;

if (!SHOW_HUD)
	return;
if ((gameData.hud.msgs [0].nMessages > 0) && 
	 (strlen (gameData.hud.msgs [0].szMsgs [gameData.hud.msgs [0].nFirst]) > 38))
	return;
GrSetCurFont (GAME_FONT);
if ((IsMultiGame && !IsCoopGame))
	sprintf (score_str, "%s: %5d", TXT_KILLS, LOCALPLAYER.netKillsTotal);
else
	sprintf (score_str, "%s: %5d", TXT_SCORE, LOCALPLAYER.score);
GrGetStringSize (score_str, &w, &h, &aw);
GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
GrPrintF (grdCurCanv->cv_w-w-LHX (2), 3, score_str);
}

//	-----------------------------------------------------------------------------

void HUDShowTimerCount ()
 {
	char	score_str [20];
	int	w, h, aw, i;
	fix timevar=0;

if (!SHOW_HUD)
	return;
if ((gameData.hud.msgs [0].nMessages > 0) && (strlen (gameData.hud.msgs [0].szMsgs [gameData.hud.msgs [0].nFirst]) > 38))
	return;

if ((gameData.app.nGameMode & GM_NETWORK) && netGame.xPlayTimeAllowed) {
	timevar=i2f (netGame.xPlayTimeAllowed*5*60);
	i=f2i (timevar-ThisLevelTime);
	i++;
	sprintf (score_str, "T - %5d", i);
	GrGetStringSize (score_str, &w, &h, &aw);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	if (i>-1 && !gameData.reactor.bDestroyed)
		GrPrintF (grdCurCanv->cv_w-w-LHX (10), LHX (11), score_str);
	}
}

//	-----------------------------------------------------------------------------
//y offset between lines on HUD
int nLineSpacing;

void HUDShowScoreAdded ()
{
	int	color;
	int	w, h, aw;
	char	score_str [20];

if (!SHOW_HUD)
	return;
if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP)) 
	return;
if (score_display [0] == 0)
	return;
GrSetCurFont (GAME_FONT);
scoreTime -= gameData.time.xFrame;
if (scoreTime > 0) {
	color = f2i (scoreTime * 20) + 12;
	if (color < 10) 
		color = 12;
	else if (color > 31) 
		color = 30;
	color = color - (color % 4);	//	Only allowing colors 12, 16, 20, 24, 28 speeds up gr_getcolor, improves caching
	if (gameStates.app.cheats.bEnabled)
		sprintf (score_str, "%s", TXT_CHEATER);
	else
		sprintf (score_str, "%5d", score_display [0]);
	GrGetStringSize (score_str, &w, &h, &aw);
	GrSetFontColorRGBi (RGBA_PAL2 (0, color, 0), 1, 0, 0);
	GrPrintF (grdCurCanv->cv_w-w-LHX (2+10), nLineSpacing+4, score_str);
	}
else {
	scoreTime = 0;
	score_display [0] = 0;
	}	
}

//	-----------------------------------------------------------------------------

void SBShowScore ()
{	                                                                                                                                                                                                                                                             
	char	score_str [20];
	int x, y;
	int	w, h, aw;
	static int last_x [4]={SB_SCORE_RIGHT_L, SB_SCORE_RIGHT_L, SB_SCORE_RIGHT_H, SB_SCORE_RIGHT_H};
	int redraw_score;

if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP)) 
	redraw_score = -99;
else
	redraw_score = -1;

if (old_score [gameStates.render.vr.nCurrentPage]==redraw_score) {
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);

	if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP)) {
			HUDPrintF (SB_SCORE_LABEL_X, SB_SCORE_Y, "%s:", TXT_KILLS);
		}
	else {
		HUDPrintF (SB_SCORE_LABEL_X, SB_SCORE_Y, "%s:", TXT_SCORE);
		}
	}

GrSetCurFont (GAME_FONT);
if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP)) 
	sprintf (score_str, "%5d", LOCALPLAYER.netKillsTotal);
else
	sprintf (score_str, "%5d", LOCALPLAYER.score);
GrGetStringSize (score_str, &w, &h, &aw);

x = SB_SCORE_RIGHT-w-LHX (2);
y = SB_SCORE_Y;

//erase old score
GrSetColorRGBi (RGBA_PAL (0, 0, 0));
HUDRect (last_x [ (gameStates.video.nDisplayMode?2:0)+gameStates.render.vr.nCurrentPage], y, SB_SCORE_RIGHT, y+GAME_FONT->ft_h);

if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP)) 
	GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
else
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);	

HUDPrintF (x, y, score_str);

last_x [ (gameStates.video.nDisplayMode?2:0)+gameStates.render.vr.nCurrentPage] = x;
}

//	-----------------------------------------------------------------------------

void SBShowScoreAdded ()
{
	int	color;
	int w, h, aw;
	char	score_str [32];
	int x;
	static int last_x [4]={SB_SCORE_RIGHT_L, SB_SCORE_RIGHT_L, SB_SCORE_RIGHT_H, SB_SCORE_RIGHT_H};
	static	int last_score_display [2] = {-1, -1};
   int frc=0;
	
	if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP)) 
		return;

	if (score_display [gameStates.render.vr.nCurrentPage] == 0)
		return;

GrSetCurFont (GAME_FONT);
scoreTime -= gameData.time.xFrame;
if (scoreTime > 0) {
	if (score_display [gameStates.render.vr.nCurrentPage] != last_score_display [gameStates.render.vr.nCurrentPage] || frc) {
		GrSetColorRGBi (RGBA_PAL (0, 0, 0));
		GrRect (last_x [ (gameStates.video.nDisplayMode?2:0)+gameStates.render.vr.nCurrentPage], SB_SCORE_ADDED_Y, SB_SCORE_ADDED_RIGHT, SB_SCORE_ADDED_Y+GAME_FONT->ft_h);

		last_score_display [gameStates.render.vr.nCurrentPage] = score_display [gameStates.render.vr.nCurrentPage];
		}
	color = f2i (scoreTime * 20) + 10;
	if (color < 10) 
		color = 10;
	else if (color > 31) 
		color = 31;

	if (gameStates.app.cheats.bEnabled)
		sprintf (score_str, "%s", TXT_CHEATER);
	else
		sprintf (score_str, "%5d", score_display [gameStates.render.vr.nCurrentPage]);
	GrGetStringSize (score_str, &w, &h, &aw);
	x = SB_SCORE_ADDED_RIGHT-w-LHX (2);
	GrSetFontColorRGBi (RGBA_PAL2 (0, color, 0), 1, 0, 0);
	GrPrintF (x, SB_SCORE_ADDED_Y, score_str);
	last_x [ (gameStates.video.nDisplayMode?2:0)+gameStates.render.vr.nCurrentPage] = x;
	} 
else {
	//erase old score
	GrSetColorRGBi (RGBA_PAL (0, 0, 0));
	GrRect (last_x [ (gameStates.video.nDisplayMode?2:0)+gameStates.render.vr.nCurrentPage], SB_SCORE_ADDED_Y, SB_SCORE_ADDED_RIGHT, SB_SCORE_ADDED_Y+GAME_FONT->ft_h);
	scoreTime = 0;
	score_display [gameStates.render.vr.nCurrentPage] = 0;
	}
}

fix	Last_warning_beepTime [2] = {0, 0};		//	Time we last played homing missile warning beep.

//	-----------------------------------------------------------------------------

void PlayHomingWarning (void)
{
	fix	beep_delay;

	if (gameStates.app.bEndLevelSequence || gameStates.app.bPlayerIsDead)
		return;

	if (LOCALPLAYER.homingObjectDist >= 0) {
		beep_delay = LOCALPLAYER.homingObjectDist/128;
		if (beep_delay > F1_0)
			beep_delay = F1_0;
		else if (beep_delay < F1_0/8)
			beep_delay = F1_0/8;

		if (Last_warning_beepTime [gameStates.render.vr.nCurrentPage] > gameData.time.xGame)
			Last_warning_beepTime [gameStates.render.vr.nCurrentPage] = 0;

		if (gameData.time.xGame - Last_warning_beepTime [gameStates.render.vr.nCurrentPage] > beep_delay/2) {
			DigiPlaySample (SOUND_HOMING_WARNING, F1_0);
			Last_warning_beepTime [gameStates.render.vr.nCurrentPage] = gameData.time.xGame;
		}
	}
}

int	bLastHomingWarningShown [2]={-1, -1};

//	-----------------------------------------------------------------------------

void ShowHomingWarning (void)
{
if ((gameStates.render.cockpit.nMode == CM_STATUS_BAR) || (gameStates.app.bEndLevelSequence)) {
	if (bLastHomingWarningShown [gameStates.render.vr.nCurrentPage] == 1) {
		PAGE_IN_GAUGE (GAUGE_HOMING_WARNING_OFF);
		HUDBitBlt (
			HOMING_WARNING_X, HOMING_WARNING_Y, 
			gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_HOMING_WARNING_OFF), F1_0, 0);
		bLastHomingWarningShown [gameStates.render.vr.nCurrentPage] = 0;
		}
	return;
}
GrSetCurrentCanvas (GetCurrentGameScreen ());
if (LOCALPLAYER.homingObjectDist >= 0) {
	if (gameData.time.xGame & 0x4000) {
		PAGE_IN_GAUGE (GAUGE_HOMING_WARNING_ON);
		HUDBitBlt (
			HOMING_WARNING_X, HOMING_WARNING_Y, 
			gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_HOMING_WARNING_ON), F1_0, 0);
			bLastHomingWarningShown [gameStates.render.vr.nCurrentPage] = 1;
		}
	else {
		PAGE_IN_GAUGE (GAUGE_HOMING_WARNING_OFF);
		HUDBitBlt (
			HOMING_WARNING_X, HOMING_WARNING_Y, 
			gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_HOMING_WARNING_OFF), F1_0, 0);
		bLastHomingWarningShown [gameStates.render.vr.nCurrentPage] = 0;
		}
	}
else {
	PAGE_IN_GAUGE (GAUGE_HOMING_WARNING_OFF);
	HUDBitBlt (
		HOMING_WARNING_X, HOMING_WARNING_Y, 
		gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_HOMING_WARNING_OFF), F1_0, 0);
	bLastHomingWarningShown [gameStates.render.vr.nCurrentPage] = 0;
	}
}

//	-----------------------------------------------------------------------------

#define MAX_SHOWN_LIVES 4

extern int Game_window_y;
extern int SW_y [2];

void HUDShowHomingWarning (void)
{
if (!SHOW_HUD)
	return;
if (LOCALPLAYER.homingObjectDist >= 0) {
	if (gameData.time.xGame & 0x4000) {
		int x=0x8000, y=grdCurCanv->cv_h-nLineSpacing;
		if (weapon_box_user [0] != WBU_WEAPON || weapon_box_user [1] != WBU_WEAPON) {
			int wy = (weapon_box_user [0] != WBU_WEAPON)?SW_y [0]:SW_y [1];
			y = min (y, (wy - nLineSpacing - Game_window_y));
			}
		GrSetCurFont (GAME_FONT);
		GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
		GrPrintF (x, y, TXT_LOCK);
		}
	}
}

//	-----------------------------------------------------------------------------

void HUDShowKeys (void)
{
	int y = 3*nLineSpacing;
	int dx = GAME_FONT->ft_w+GAME_FONT->ft_w/2;

if (!SHOW_HUD)
	return;
if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP))
	return;
if (LOCALPLAYER.flags & PLAYER_FLAGS_BLUE_KEY) {
	PAGE_IN_GAUGE (KEY_ICON_BLUE);
	GrUBitmapM (2, y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (KEY_ICON_BLUE)]);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_GOLD_KEY) {
	PAGE_IN_GAUGE (KEY_ICON_YELLOW);
	GrUBitmapM (2+dx, y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (KEY_ICON_YELLOW)]);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_RED_KEY) {
	PAGE_IN_GAUGE (KEY_ICON_RED);
	GrUBitmapM (2+2*dx, y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (KEY_ICON_RED)]);
	}
}

//	-----------------------------------------------------------------------------

char *pszObjTallyIcons [] = {"louguard.tga", "shldorb.tga"};

int LoadObjTallyIcons (void)
{
	int	i;

if (bHaveObjTallyBms > -1)
	return bHaveObjTallyBms;
memset (bmObjTally, 0, sizeof (bmObjTally));
for (i = 0; i < 2; i++)
	if (!ReadTGA (pszObjTallyIcons [i], gameFolders.szDataDir, bmObjTally + i, -1, 1.0, 0, 0)) {
		while (i)
			GrFreeBitmapData (bmObjTally + --i);
		return bHaveObjTallyBms = 0;
		}
return bHaveObjTallyBms = 1;
}

//	-----------------------------------------------------------------------------

void FreeObjTallyIcons (void)
{
	int	i;

if (bHaveObjTallyBms > 0) {
	for (i = 0; i < 2; i++)
		GrFreeBitmapData (bmObjTally + i);
	memset (bmObjTally, 0, sizeof (bmObjTally));
	bHaveObjTallyBms = -1;
	}
}

//	-----------------------------------------------------------------------------

void HUDShowObjTally (void)
{
	static int		objCounts [2] = {0, 0};
	static time_t	t0 = -1;
	time_t			t;

if (!gameOpts->render.cockpit.bObjectTally)
	return;
if (!SHOW_HUD)
	return;
if (!IsMultiGame || IsCoopGame) {
	int	x, x0 = 0, y = 0, w, h, aw, i, bmW, bmH;
	char	szInfo [20];

	if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT)
		y = 3 * nLineSpacing;
	else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
		y = 2 * nLineSpacing;
	else {//if (!SHOW_COCKPIT) {
		y = 2 * nLineSpacing;
		if (gameStates.render.fonts.bHires)
			y += nLineSpacing;
		}
	if (gameOpts->render.cockpit.bPlayerStats)
		y += 2 * nLineSpacing;

	x0 = grdCurCanv->cv_w;
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	t = gameStates.app.nSDLTicks;
	if (t - t0 > 333) {	//update 3 times per second
		t0 = t;
		for (i = 0; i < 2; i++) 
			objCounts [i] = ObjectCount (i ? OBJ_POWERUP : OBJ_ROBOT);
		}
	if (!gameOpts->render.cockpit.bTextGauges && (LoadObjTallyIcons () > 0)) {
		for (i = 0; i < 2; i++) {
			bmH = bmObjTally [i].bm_props.h / 2;
			bmW = bmObjTally [i].bm_props.w / 2;
			x = x0 - bmW - LHX (2);
			OglUBitMapMC (x, y, bmW, bmH, bmObjTally + i, NULL, F1_0, 0);
			sprintf (szInfo, "%d", objCounts [i]);
			GrGetStringSize (szInfo, &w, &h, &aw);
			x -= w + LHY (2);
			GrPrintF (x, y + (bmH - h) / 2, szInfo);
			y += bmH;
			}
		}
	else {
		y = 6 + 2 * nLineSpacing;
		for (i = 0; i < 2; i++) {
			sprintf (szInfo, "%s: %5d", i ? "Powerups" : "Robots", objCounts [i]);
			GrGetStringSize (szInfo, &w, &h, &aw);
			GrPrintF (grdCurCanv->cv_w - w - LHX (2), y, szInfo);
			y += nLineSpacing;
			}
		}
	}
}

//	-----------------------------------------------------------------------------

void HUDShowPlayerStats (void)
{
	int		h, w, aw, y;
	double	p [3], s [3];
	char		szStats [50];

if (!gameOpts->render.cockpit.bPlayerStats)
	return;
if (!SHOW_HUD)
	return;
if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT)
	y = 3 * nLineSpacing;
else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
	y = 2 * nLineSpacing;
else {//if (!SHOW_COCKPIT) {
	y = 2 * nLineSpacing;
	if (gameStates.render.fonts.bHires)
		y += nLineSpacing;
	}
GrSetFontColorRGBi (ORANGE_RGBA, 1, 0, 0);
y = 6 + 2 * nLineSpacing;
h = (gameData.stats.nDisplayMode - 1) / 2;
if ((gameData.stats.nDisplayMode - 1) % 2 == 0) {
	sprintf (szStats, "%s%d-%d %d-%d %d-%d", 
				h ? "T:" : "", 
				gameData.stats.player [h].nHits [0],
				gameData.stats.player [h].nMisses [0],
				gameData.stats.player [h].nHits [1],
				gameData.stats.player [h].nMisses [1],
				gameData.stats.player [h].nHits [0] + gameData.stats.player [h].nHits [1],
				gameData.stats.player [h].nMisses [0] + gameData.stats.player [h].nMisses [1]);
	}
else {
	s [0] = gameData.stats.player [h].nHits [0] + gameData.stats.player [h].nMisses [0];
	s [1] = gameData.stats.player [h].nHits [1] + gameData.stats.player [h].nMisses [1];
	s [2] = s [0] + s [1];
	p [0] = s [0] ? (gameData.stats.player [h].nHits [0] / s [0]) * 100 : 0;
	p [1] = s [1] ? (gameData.stats.player [h].nHits [1] / s [1]) * 100 : 0;
	p [2] = s [2] ? ((gameData.stats.player [h].nHits [0] + gameData.stats.player [h].nHits [1]) / s [2]) * 100 : 0;
	sprintf (szStats, "%s%1.1f%c %1.1f%c %1.1f%c", h ? "T:" : "", p [0], '%', p [1], '%', p [2], '%');
	}
GrGetStringSize (szStats, &w, &h, &aw);
GrString (grdCurCanv->cv_w - w - LHX (2), y, szStats);
}

//	-----------------------------------------------------------------------------

void HUDShowOrbs (void)
{
if (!SHOW_HUD)
	return;
if (gameData.app.nGameMode & (GM_HOARD | GM_ENTROPY)) {
	int x = 0, y = 0;
	grsBitmap *bmP = NULL;

	if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) {
		y = 2*nLineSpacing;
		x = 4*GAME_FONT->ft_w;
		}
	else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR) {
		y = nLineSpacing;
		x = GAME_FONT->ft_w;
		}
	else if ((gameStates.render.cockpit.nMode == CM_FULL_SCREEN) || 
				(gameStates.render.cockpit.nMode == CM_LETTERBOX)) {
//			y = 5*nLineSpacing;
		y = nLineSpacing;
		x = GAME_FONT->ft_w;
		if (gameStates.render.fonts.bHires)
			y += nLineSpacing;
		}
	else
		Int3 ();		//what sort of cockpit?

	bmP = &gameData.hoard.icon [gameStates.render.fonts.bHires].bm;
	GrUBitmapM (x, y, bmP);

	x+=bmP->bm_props.w+bmP->bm_props.w/2;
	y+= (gameStates.render.fonts.bHires?2:1);
	if (gameData.app.nGameMode & GM_ENTROPY) {
		char	szInfo [20];
		int	w, h, aw;
		if (LOCALPLAYER.secondaryAmmo [PROXIMITY_INDEX] >= extraGameInfo [1].entropy.nCaptureVirusLimit)
			GrSetFontColorRGBi (ORANGE_RGBA, 1, 0, 0);
		else
			GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
		sprintf (szInfo, 
			"x %d [%d]", 
			LOCALPLAYER.secondaryAmmo [PROXIMITY_INDEX], 
			LOCALPLAYER.secondaryAmmo [SMART_MINE_INDEX]);
		GrPrintF (x, y, szInfo);
		if (gameStates.entropy.bConquering) {
			int t = (extraGameInfo [1].entropy.nCaptureTimeLimit * 1000) - 
					   (gameStates.app.nSDLTicks - gameStates.entropy.nTimeLastMoved);

			if (t < 0)
				t = 0;
			GrGetStringSize (szInfo, &w, &h, &aw);
			x += w;
			GrSetFontColorRGBi (RED_RGBA, 1, 0, 0);
			sprintf (szInfo, " %d.%d", t / 1000, (t % 1000) / 100);
			GrPrintF (x, y, szInfo);
			}
		}
	else
		GrPrintF (x, y, "x %d", LOCALPLAYER.secondaryAmmo [PROXIMITY_INDEX]);
	}
}

//	-----------------------------------------------------------------------------

void HUDShowFlag (void)
{
if (!SHOW_HUD)
	return;
if ((gameData.app.nGameMode & GM_CAPTURE) && (LOCALPLAYER.flags & PLAYER_FLAGS_FLAG)) {
	int x = 0, y = 0, icon;

	if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) {
		y = 2*nLineSpacing;
		x = 4*GAME_FONT->ft_w;
		}
	else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR) {
		y = nLineSpacing;
		x = GAME_FONT->ft_w;
		}
	else if ((gameStates.render.cockpit.nMode == CM_FULL_SCREEN) || 
				 (gameStates.render.cockpit.nMode == CM_LETTERBOX)) {
		y = 5*nLineSpacing;
		x = GAME_FONT->ft_w;
		if (gameStates.render.fonts.bHires)
			y += nLineSpacing;
		}
	else
		Int3 ();		//what sort of cockpit?
	icon = (GetTeam (gameData.multiplayer.nLocalPlayer) == TEAM_BLUE) ? FLAG_ICON_RED : FLAG_ICON_BLUE;
	PAGE_IN_GAUGE (icon);
	GrUBitmapM (x, y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (icon));
	}
}

//	-----------------------------------------------------------------------------

inline int HUDShowFlashGauge (int h, int *bFlash, int tToggle)
{
	time_t t = gameStates.app.nSDLTicks;
	int b = *bFlash;

if (gameStates.app.bPlayerIsDead || 
	 gameStates.app.bPlayerExploded || 
	 !gameOpts->render.cockpit.bFlashGauges)
	b = 0;
else if (b == 2) {
	if (h > 20)
		b = 0;
	else if (h > 10)
		b = 1;
	}
else if (b == 1) {
	if (h > 20)
		b = 0;
	else if (h <= 10)
		b = 2;
	}
else {
	if (h <= 10)
		b = 2;
	else if (h <= 20)
		b = 1;
	else
		b = 0;
	tToggle = -1;
	}
*bFlash = b;
return (int) ((b && (tToggle <= t)) ? t + 300 / b : 0);
}

//	-----------------------------------------------------------------------------

void HUDShowEnergy (void)
{
	int h, y;

	//GrSetCurrentCanvas (&gameStates.render.vr.buffers.subRender [0]);	//render off-screen
if (!SHOW_HUD)
	return;
h = LOCALPLAYER.energy ? f2ir (LOCALPLAYER.energy) : 0;
if (gameOpts->render.cockpit.bTextGauges) {
	y = grdCurCanv->cv_h - ((gameData.app.nGameMode & GM_MULTI) ? 5 : 1) * nLineSpacing;
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	GrPrintF (2, y, "%s: %i", TXT_ENERGY, h);
	}
else {
	static int		bFlash = 0, bShow = 1;
	static time_t	tToggle;
	time_t			t;
	ubyte				c;

	if ((t = HUDShowFlashGauge (h, &bFlash, (int) tToggle))) {
		tToggle = t;
		bShow = !bShow;
		}
	y = grdCurCanv->cv_h - (int) ((((gameData.app.nGameMode & GM_MULTI) ? 5 : 1) * nLineSpacing - 1) * yScale);
	GrSetColorRGB (255, 255, (ubyte) ((h > 100) ? 255 : 0), 255);
	GrUBox (6, y, 6 + (int) (100 * xScale), y + (int) (9 * yScale));
	if (bFlash) {
		if (!bShow)
			goto skipGauge;
		h = 100;
		}
	c = (h > 100) ? 224 : 224;
	GrSetColorRGB (c, c, (ubyte) ((h > 100) ? c : 0), 128);
	GrURect (6, y, 6 + (int) (((h > 100) ? h - 100 : h) * xScale), y + (int) (9 * yScale));
	}

skipGauge:

if (gameData.demo.nState==ND_STATE_RECORDING) {
	int energy = f2ir (LOCALPLAYER.energy);

	if (energy != old_energy [gameStates.render.vr.nCurrentPage]) {
		NDRecordPlayerEnergy (old_energy [gameStates.render.vr.nCurrentPage], energy);
		old_energy [gameStates.render.vr.nCurrentPage] = energy;
	 	}
	}
}

//	-----------------------------------------------------------------------------

void HUDShowAfterburner (void)
{
	int h, y;

if (!SHOW_HUD)
	return;
if (!(LOCALPLAYER.flags & PLAYER_FLAGS_AFTERBURNER))
	return;		//don't draw if don't have
h = FixMul (xAfterburnerCharge, 100);
if (gameOpts->render.cockpit.bTextGauges) {
	y = grdCurCanv->cv_h - ((gameData.app.nGameMode & GM_MULTI) ? 8 : 3) * nLineSpacing;
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	GrPrintF (2, y, TXT_HUD_BURN, h);
	}
else {
	y = grdCurCanv->cv_h - (int) ((((gameData.app.nGameMode & GM_MULTI) ? 8 : 3) * nLineSpacing - 1) * yScale);
	GrSetColorRGB (255, 0, 0, 255);
	GrUBox (6, y, 6 + (int) (100 * xScale), y + (int) (9 * yScale));
	GrSetColorRGB (224, 0, 0, 128);
	GrURect (6, y, 6 + (int) (h * xScale), y + (int) (9 * yScale));
	}
if (gameData.demo.nState==ND_STATE_RECORDING) {
	if (xAfterburnerCharge != old_afterburner [gameStates.render.vr.nCurrentPage]) {
		NDRecordPlayerAfterburner (old_afterburner [gameStates.render.vr.nCurrentPage], xAfterburnerCharge);
		old_afterburner [gameStates.render.vr.nCurrentPage] = xAfterburnerCharge;
	 	}
	}
}
#if 0
char *d2_very_shortSecondary_weapon_names [] =
		{"Flash", "Guided", "SmrtMine", "Mercury", "Shaker"};
#endif
#define SECONDARY_WEAPON_NAMES_VERY_SHORT(nWeaponNum) 				\
	 ((nWeaponNum <= MEGA_INDEX)?GAMETEXT (541 + nWeaponNum):	\
	 GT (636+nWeaponNum-FLASHMSL_INDEX))

//return which bomb will be dropped next time the bomb key is pressed
extern int ArmedBomb ();

//	-----------------------------------------------------------------------------

void ShowBombCount (int x, int y, int bg_color, int always_show)
{
	int bomb, count, countx;
	char txt [5], *t;

if (gameData.app.nGameMode & GM_ENTROPY)
	return;
bomb = ArmedBomb ();
if (!bomb)
	return;
if ((gameData.app.nGameMode & GM_HOARD) && (bomb == PROXIMITY_INDEX))
	return;
count = LOCALPLAYER.secondaryAmmo [bomb];
#ifdef _DEBUG
count = min (count, 99);	//only have room for 2 digits - cheating give 200
#endif
countx = (bomb==PROXIMITY_INDEX)?count:-count;
if (always_show && count == 0)		//no bombs, draw nothing on HUD
	return;
if (!always_show && countx == oldBombcount [gameStates.render.vr.nCurrentPage])
	return;
// I hate doing this off of hard coded coords!!!!
if (gameStates.render.cockpit.nMode == CM_STATUS_BAR) {		//draw background
	GrSetColorRGBi (bg_color);
	if (!gameStates.video.nDisplayMode) {
		HUDRect (169, 189, 189, 196);
		GrSetColorRGBi (RGBA_PAL (0, 0, 0));
		GrScanLine (168, 189, 189);
		} 
	else {
		GrRect (HUD_SCALE_X (338), HUD_SCALE_Y (453), HUD_SCALE_X (378), HUD_SCALE_Y (470));
		GrSetColorRGBi (RGBA_PAL (0, 0, 0));
		GrScanLine (HUD_SCALE_X (336), HUD_SCALE_X (378), HUD_SCALE_Y (453));
		}
	}
if (count)
	GrSetFontColorRGBi (
		(bomb == PROXIMITY_INDEX) ? RGBA_PAL2 (55, 0, 0) : RGBA_PAL2 (59, 59, 21), 1, 
		bg_color, bg_color != -1);
else if (bg_color != -1)
	GrSetFontColorRGBi (bg_color, 1, bg_color, 1);	//erase by drawing in background color
sprintf (txt, "B:%02d", count);
while ((t=strchr (txt, '1')) != NULL)
	*t = '\x84';	//convert to wide '1'
if (SHOW_COCKPIT)
	HUDPrintF (x, y, txt);
else
	GrString (x, y, txt);
oldBombcount [gameStates.render.vr.nCurrentPage] = countx;
}

//	-----------------------------------------------------------------------------

void DrawPrimaryAmmoInfo (int ammoCount)
{
	if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
		DrawAmmoInfo (SB_PRIMARY_AMMO_X, SB_PRIMARY_AMMO_Y, ammoCount, 1);
	else
		DrawAmmoInfo (PRIMARY_AMMO_X, PRIMARY_AMMO_Y, ammoCount, 1);
}

//	-----------------------------------------------------------------------------

void HUDToggleWeaponIcons (void)
{
	int	i;

for (i = 0; i < Controls [0].toggleIconsCount; i++)
	if (gameStates.app.bNostalgia)
		extraGameInfo [0].nWeaponIcons = 0;
	else {
		extraGameInfo [0].nWeaponIcons = (extraGameInfo [0].nWeaponIcons + 1) % 5;
		if (!gameOpts->render.weaponIcons.bEquipment && (extraGameInfo [0].nWeaponIcons == 3))
			extraGameInfo [0].nWeaponIcons = 4;
		}
}

//	-----------------------------------------------------------------------------

#define ICON_SCALE	3

void HUDShowWeaponIcons (void)
{
	grsBitmap	*bmP;
	int	nWeaponIcons = (gameStates.render.cockpit.nMode == CM_STATUS_BAR) ? 3 : extraGameInfo [0].nWeaponIcons;
	int	nIconScale = (gameOpts->render.weaponIcons.bSmall || (gameStates.render.cockpit.nMode != CM_FULL_SCREEN)) ? 4 : 3;
	int	nIconPos = nWeaponIcons - 1;
	int	nMaxAutoSelect;
	int	fw, fh, faw, 
			i, j, ll, n, 
			ox = 6, 
			oy = 6, 
			x, dx, y = 0, dy = 0;
	ubyte	alpha = gameOpts->render.weaponIcons.alpha;
	unsigned int nAmmoColor;
	char	szAmmo [10];
	int	nLvlMap [2][10] = {{9, 4, 8, 3, 7, 2, 6, 1, 5, 0}, {4, 3, 2, 1, 0, 4, 3, 2, 1, 0}};
	static int	wIcon = 0, 
					hIcon = 0;
	static int	w = -1, 
					h = -1;
	static ubyte ammoType [2][10] = {{0, 1, 0, 0, 0, 0, 1, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};
	static int bInitIcons = 1;

ll = LOCALPLAYER.laserLevel;
if (gameOpts->render.weaponIcons.bShowAmmo) {
	GrSetCurFont (SMALL_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	}
dx = (int) (10 * cmScaleX);
if (nWeaponIcons < 3) {
#if 0
	if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT) {
#endif
		dy = (grdCurScreen->sc_h - grdCurCanv->cv_bitmap.bm_props.h);
		y = nIconPos ? grdCurScreen->sc_h - dy - oy : oy + hIcon + 12;
#if 0
		}
	else {
		y = (2 - gameStates.app.bD1Mission) * (oy + hIcon) + 12;
		nIconPos = 1;
		}
#endif
	}
for (i = 0; i < 2; i++) {
	n = (gameStates.app.bD1Mission) ? 5 : 10;
	nMaxAutoSelect = 255;
	if (nWeaponIcons > 2) {
		int h;
		if (gameStates.render.cockpit.nMode != CM_STATUS_BAR)
			h = 0;
		else {
#ifdef _DEBUG
			h = gameStates.render.cockpit.nMode + (gameStates.video.nDisplayMode ? gameData.models.nCockpits / 2 : 0);
			h = gameData.pig.tex.cockpitBmIndex [h].index;
			h = gameData.pig.tex.bitmaps [0][h].bm_props.h;
#else
			h = gameData.pig.tex.bitmaps [0][gameData.pig.tex.cockpitBmIndex [gameStates.render.cockpit.nMode + (gameStates.video.nDisplayMode ? gameData.models.nCockpits / 2 : 0)].index].bm_props.h;
#endif
			}
		y = (grdCurCanv->cv_bitmap.bm_props.h - h - n * (hIcon + oy)) / 2 + hIcon;
		x = i ? grdCurScreen->sc_w - wIcon - ox : ox;
		}
	else {
		x = grdCurScreen->sc_w / 2;
		if (i)
			x += dx;
		else
			x -= dx + wIcon;
		}
	for (j = 0; j < n; j++) {
		int bArmed, bHave, bLoaded, l, m;

		if (gameOpts->render.weaponIcons.nSort && !gameStates.app.bD1Mission) {
			l = nWeaponOrder [i][j];
			if (l == 255)
				nMaxAutoSelect = j;
			l = nWeaponOrder [i][j + (j >= nMaxAutoSelect)];
			}
		else
			l = nLvlMap [gameStates.app.bD1Mission][j];
		m = i ? secondaryWeaponToWeaponInfo [l] : primaryWeaponToWeaponInfo [l];
		if ((gameData.pig.tex.nHamFileVersion >= 3) && gameStates.video.nDisplayMode) {
			bmP = gameData.pig.tex.bitmaps [0] + gameData.weapons.info [m].hires_picture.index;
			PIGGY_PAGE_IN (gameData.weapons.info [m].hires_picture, 0);
			}
		else {
			bmP = gameData.pig.tex.bitmaps [0] + gameData.weapons.info [m].picture.index;
			PIGGY_PAGE_IN (gameData.weapons.info [m].picture, 0);
			}
		Assert (bmP != NULL);
		if (w < bmP->bm_props.w)
			w = bmP->bm_props.w;
		if (h < bmP->bm_props.h)
			h = bmP->bm_props.h;
		wIcon = (int) ((w + nIconScale - 1) / nIconScale * cmScaleX);
		hIcon = (int) ((h + nIconScale - 1) / nIconScale * cmScaleY);
		if (bInitIcons)
			continue;
		if (i)
			bHave = (LOCALPLAYER.secondaryWeaponFlags & (1 << l));
		else if (!l) {
			bHave = (ll <= MAX_LASER_LEVEL);
			if (!bHave)
				continue;
			}
		else if (l == 5) {
			bHave = (ll > MAX_LASER_LEVEL);
			if (!bHave)
				continue;
			}
		else {
			bHave = (LOCALPLAYER.primaryWeaponFlags & (1 << l));
			if (bHave && extraGameInfo [0].bSmartWeaponSwitch && ((l == 1) || (l == 2)) &&
				 LOCALPLAYER.primaryWeaponFlags & (1 << (l + 5)))
				continue;
			}
		HUDBitBlt (nIconScale * - (x + (w - bmP->bm_props.w) / (2 * nIconScale)), 
					  nIconScale * - (y - hIcon), bmP, nIconScale * F1_0, 0);
		*szAmmo = '\0';
		nAmmoColor = GREEN_RGBA;
		if (ammoType [i][l]) {
			int nAmmo = (i ? LOCALPLAYER.secondaryAmmo [l] : LOCALPLAYER.primaryAmmo [(l == 6) ? 1 : l]);
			bLoaded = (nAmmo > 0);
			if (bHave) {
				if (bLoaded && gameOpts->render.weaponIcons.bShowAmmo) {
					if (!i && (l % 5 == 1)) {//Gauss/Vulcan
						nAmmo = f2i (nAmmo * (unsigned) VULCAN_AMMO_SCALE);
#if 0
						sprintf (szAmmo, "%d.%d", nAmmo / 1000, (nAmmo % 1000) / 100);
#else
						if (nAmmo && (nAmmo < 1000)) {
							sprintf (szAmmo, ".%d", nAmmo / 100);
							nAmmoColor = RED_RGBA;
							}
						else
							sprintf (szAmmo, "%d", nAmmo / 1000);
#endif
						}
					else
						sprintf (szAmmo, "%d", nAmmo);
					GrGetStringSize (szAmmo, &fw, &fh, &faw);
					}
				}
			}
		else {
			bLoaded = (LOCALPLAYER.energy > gameData.weapons.info [l].energy_usage);
			if (l == 0) {//Lasers
				sprintf (szAmmo, "%d", (ll > MAX_LASER_LEVEL) ? MAX_LASER_LEVEL + 1 : ll + 1);
				GrGetStringSize (szAmmo, &fw, &fh, &faw);
				}
			else if ((l == 5) && (ll > MAX_LASER_LEVEL)) {
				sprintf (szAmmo, "%d", ll - MAX_LASER_LEVEL);
				GrGetStringSize (szAmmo, &fw, &fh, &faw);
				}
			}
		if (i && !bLoaded)
			bHave = 0;
		if (bHave) {
			//gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS * 2 / 3;
			if (bLoaded)
				GrSetColorRGB (128, 128, 0, (ubyte) (alpha * 16));
			else
				GrSetColorRGB (128, 0, 0, (ubyte) (alpha * 16));
			}
		else {
			//gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS * 2 / 7;
			GrSetColorRGB (64, 64, 64, (ubyte) (159 + alpha * 12));
			}
		GrURect (x - 1, y - hIcon - 1, x + wIcon + 2, y + 2);
		if (i) {
			if (j < 8)
				bArmed = (l == gameData.weapons.nSecondary);
			else
				bArmed = (j == 8) == (bLastSecondaryWasSuper [PROXIMITY_INDEX] != 0);
			}
		else {
			if (l == 5)
				bArmed = (bHave && (0 == gameData.weapons.nPrimary));
			else if (l)
				bArmed = (l == gameData.weapons.nPrimary);
			else
				bArmed = (bHave && (l == gameData.weapons.nPrimary));
			}
		if (bArmed)
			if (bLoaded)
				GrSetColorRGB (255, 192, 0, 255);
			else
				GrSetColorRGB (160, 0, 0, 255);
		else if (bHave)
			if (bLoaded)
				GrSetColorRGB (0, 160, 0, 255);
			else
				GrSetColorRGB (96, 0, 0, 255);
		else
			GrSetColorRGB (64, 64, 64, 255);
		GrUBox (x - 1, y - hIcon - 1, x + wIcon + 2, y + 2);
		if (*szAmmo) {
			GrSetFontColorRGBi (nAmmoColor, 1, 0, 0);
			GrString (x + wIcon + 2 - fw, y - fh, szAmmo);
			GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
			}
		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
		if (nWeaponIcons > 2)
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
}

//	-----------------------------------------------------------------------------

int LoadInventoryIcons (void)
{
	int	h, i;

if (!((bmpInventory = PiggyLoadBitmap ("inventry.bmp")) ||
	   (bmpInventory = PiggyLoadBitmap ("inventory.bmp"))))
	return bHaveInvBms = 0;
memset (bmInvItems, 0, sizeof (bmInvItems));
h = bmpInventory->bm_props.w * bmpInventory->bm_props.w;
for (i = 0; i < NUM_INV_ITEMS; i++) {
	bmInvItems [i] = *bmpInventory;
	bmInvItems [i].bm_props.h = bmInvItems [i].bm_props.w;
	bmInvItems [i].bm_texBuf += h * i;
	bmInvItems [i].bm_palette = gamePalette;
	}
return bHaveInvBms = 1;
}

//	-----------------------------------------------------------------------------

void FreeInventoryIcons (void)
{
if (bmpInventory) {
	GrFreeBitmap (bmpInventory);
	bmpInventory = NULL;
	bHaveInvBms = -1;
	}
}

//	-----------------------------------------------------------------------------

int HUDEquipmentActive (int bFlag)
{
switch (bFlag) {
	case PLAYER_FLAGS_AFTERBURNER:
		return (xAfterburnerCharge && Controls [0].afterburnerState);
	case PLAYER_FLAGS_CONVERTER:
		return gameStates.app.bUsingConverter;
	case PLAYER_FLAGS_HEADLIGHT:
		return (LOCALPLAYER.flags & PLAYER_FLAGS_HEADLIGHT_ON) != 0;
	case PLAYER_FLAGS_MAP_ALL:
		return 0;
	case PLAYER_FLAGS_AMMO_RACK:
		return 0;
	case PLAYER_FLAGS_QUAD_LASERS:
		return 0;
	case PLAYER_FLAGS_CLOAKED:
		return (LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED) != 0;
	case PLAYER_FLAGS_INVULNERABLE:
		return (LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) != 0;
	}
return 0;
}

//	-----------------------------------------------------------------------------

void HUDShowInventoryIcons (void)
{
	grsBitmap	*bm;
	char	szCount [4];
	int nIconScale = (gameOpts->render.weaponIcons.bSmall || (gameStates.render.cockpit.nMode != CM_FULL_SCREEN)) ? 3 : 2;
	int nIconPos = extraGameInfo [0].nWeaponIcons & 1;
	int	fw, fh, faw;
	int	j, n, firstItem, 
			oy = 6, 
			ox = 6, 
			x, y, dy;
	int	w = bmpInventory->bm_props.w, 
			h = bmpInventory->bm_props.w;
	int	wIcon = (int) ((w + nIconScale - 1) / nIconScale * cmScaleX), 
			hIcon = (int) ((h + nIconScale - 1) / nIconScale * cmScaleY);
	ubyte	alpha = gameOpts->render.weaponIcons.alpha;
	static int nInvFlags [NUM_INV_ITEMS] = {
		PLAYER_FLAGS_AFTERBURNER, 
		PLAYER_FLAGS_CONVERTER, 
		PLAYER_FLAGS_HEADLIGHT, 
		PLAYER_FLAGS_MAP_ALL, 
		PLAYER_FLAGS_AMMO_RACK, 
		PLAYER_FLAGS_QUAD_LASERS, 
		PLAYER_FLAGS_CLOAKED, 
		PLAYER_FLAGS_INVULNERABLE
		};
	static int nEnergyType [NUM_INV_ITEMS] = {F1_0, 100 * F1_0, 0, F1_0, 0, F1_0, 0, 0};

dy = (grdCurScreen->sc_h - grdCurCanv->cv_bitmap.bm_props.h);
if (gameStates.render.cockpit.nMode != CM_STATUS_BAR) //(!SHOW_COCKPIT)
	y = nIconPos ? grdCurScreen->sc_h - dy - oy : oy + hIcon + 12;
else
	y = oy + hIcon + 12;
n = (gameOpts->gameplay.bInventory && (!IsMultiGame || IsCoopGame)) ? NUM_INV_ITEMS : NUM_INV_ITEMS - 2;
firstItem = gameStates.app.bD1Mission ? INV_ITEM_QUADLASERS : 0;
x = (grdCurScreen->sc_w - (n - firstItem) * wIcon - (n - 1 - firstItem) * ox) / 2;
for (j = firstItem; j < n; j++) {
	int bHave, bArmed, bActive = HUDEquipmentActive (nInvFlags [j]);
	bm = bmInvItems + j;
	HUDBitBlt (nIconScale * - (x + (w - bm->bm_props.w) / (2 * nIconScale)), nIconScale * - (y - hIcon), bm, nIconScale * F1_0, 0);
	//m = 9 - j;
	*szCount = '\0';
	if (j == INV_ITEM_INVUL) {
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
	bArmed = (LOCALPLAYER.energy > nEnergyType [j]);
	if (bHave) {
		//gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS * 2 / 3;
		if (bArmed)
			if (bActive)
				GrSetColorRGB (255, 208, 0, (ubyte) (alpha * 16));
			else
				GrSetColorRGB (128, 128, 0, (ubyte) (alpha * 16));
		else
			GrSetColorRGB (128, 0, 0, (ubyte) (alpha * 16));
		}
	else {
		//gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS * 2 / 7;
		GrSetColorRGB (64, 64, 64, (ubyte) (159 + alpha * 12));
		}
	GrURect (x - 1, y - hIcon - 1, x + wIcon + 2, y + 2);
	if (bHave)
		if (bArmed)
			if (bActive)
				GrSetColorRGB (255, 208, 0, 255);
			else
				GrSetColorRGB (0, 160, 0, 255);
		else
			GrSetColorRGB (96, 0, 0, 255);
	else
		GrSetColorRGB (64, 64, 64, 255);
	GrUBox (x - 1, y - hIcon - 1, x + wIcon + 2, y + 2);
	if (*szCount) {
		GrGetStringSize (szCount, &fw, &fh, &faw);
		GrString (x + wIcon + 2 - fw, y - fh, szCount);
		}
	gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
	x += wIcon + ox;
	}
}

//	-----------------------------------------------------------------------------

void HUDShowIcons (void)
{
if (gameStates.app.bNostalgia)
	return;
HUDToggleWeaponIcons ();
if (gameOpts->render.cockpit.bHUD || SHOW_COCKPIT) {
	HUDShowPlayerStats ();
	HUDShowObjTally ();
	if (!gameStates.app.bDemoData && EGI_FLAG (nWeaponIcons, 0, 1, 0)) {
		cmScaleX *= HUD_ASPECT;
		HUDShowWeaponIcons ();
		if (gameOpts->render.weaponIcons.bEquipment) {
			if (bHaveInvBms < 0)
				LoadInventoryIcons ();
			if (bHaveInvBms > 0)
				HUDShowInventoryIcons ();
			}
		cmScaleX /= HUD_ASPECT;
		}
	}
}

//	-----------------------------------------------------------------------------

//convert '1' characters to special wide ones
#define convert_1s(s) {char *p=s; while ((p = strchr (p, '1'))) *p = (char)132;}

void HUDShowWeapons (void)
{
	int	w, h, aw;
	int	y;
	char	*weapon_name;
	char	weapon_str [32];

if (!SHOW_HUD)
	return;
#if 0
HUDShowIcons ();
#endif
//	GrSetCurrentCanvas (&gameStates.render.vr.buffers.subRender [0]);	//render off-screen
GrSetCurFont (GAME_FONT);
GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
y = grdCurCanv->cv_h;
if (gameData.app.nGameMode & GM_MULTI)
	y -= 4*nLineSpacing;
weapon_name = PRIMARY_WEAPON_NAMES_SHORT (gameData.weapons.nPrimary);
switch (gameData.weapons.nPrimary) {
	case LASER_INDEX:
		if (LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS)
			sprintf (weapon_str, "%s %s %i", TXT_QUAD, weapon_name, LOCALPLAYER.laserLevel+1);
		else
			sprintf (weapon_str, "%s %i", weapon_name, LOCALPLAYER.laserLevel+1);
		break;

	case SUPER_LASER_INDEX:	
		Int3 (); 
		break;	//no such thing as super laser

	case VULCAN_INDEX:		
	case GAUSS_INDEX:			
		sprintf (weapon_str, "%s: %i", weapon_name, f2i ((unsigned) LOCALPLAYER.primaryAmmo [VULCAN_INDEX] * (unsigned) VULCAN_AMMO_SCALE)); 
		convert_1s (weapon_str);
		break;

	case SPREADFIRE_INDEX:
	case PLASMA_INDEX:
	case FUSION_INDEX:
	case HELIX_INDEX:
	case PHOENIX_INDEX:
		strcpy (weapon_str, weapon_name);
		break;

	case OMEGA_INDEX:
		sprintf (weapon_str, "%s: %03i", weapon_name, gameData.laser.xOmegaCharge * 100/MAX_OMEGA_CHARGE);
		convert_1s (weapon_str);
		break;

	default:						
		Int3 ();	
		weapon_str [0] = 0;	
		break;
	}

GrGetStringSize (weapon_str, &w, &h, &aw);
GrPrintF (grdCurCanv->cv_bitmap.bm_props.w-5-w, y-2*nLineSpacing, weapon_str);

if (gameData.weapons.nPrimary == VULCAN_INDEX) {
	if (LOCALPLAYER.primaryAmmo [gameData.weapons.nPrimary] != old_ammoCount [0][gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState == ND_STATE_RECORDING)
			NDRecordPrimaryAmmo (old_ammoCount [0][gameStates.render.vr.nCurrentPage], LOCALPLAYER.primaryAmmo [gameData.weapons.nPrimary]);
		old_ammoCount [0][gameStates.render.vr.nCurrentPage] = LOCALPLAYER.primaryAmmo [gameData.weapons.nPrimary];
		}
	}

if (gameData.weapons.nPrimary == OMEGA_INDEX) {
	if (gameData.laser.xOmegaCharge != Old_Omega_charge [gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState == ND_STATE_RECORDING)
			NDRecordPrimaryAmmo (Old_Omega_charge [gameStates.render.vr.nCurrentPage], gameData.laser.xOmegaCharge);
		Old_Omega_charge [gameStates.render.vr.nCurrentPage] = gameData.laser.xOmegaCharge;
		}
	}

weapon_name = SECONDARY_WEAPON_NAMES_VERY_SHORT (gameData.weapons.nSecondary);
sprintf (weapon_str, "%s %d", weapon_name, LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary]);
GrGetStringSize (weapon_str, &w, &h, &aw);
GrPrintF (grdCurCanv->cv_bitmap.bm_props.w-5-w, y-nLineSpacing, weapon_str);

if (LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary] != old_ammoCount [1][gameStates.render.vr.nCurrentPage]) {
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordSecondaryAmmo (old_ammoCount [1][gameStates.render.vr.nCurrentPage], LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary]);
	old_ammoCount [1][gameStates.render.vr.nCurrentPage] = LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary];
	}
ShowBombCount (grdCurCanv->cv_bitmap.bm_props.w- (3*GAME_FONT->ft_w+ (gameStates.render.fonts.bHires?0:2)), y-3*nLineSpacing, -1, 1);
}

//	-----------------------------------------------------------------------------

void HUDShowCloakInvul (void)
{
if (!SHOW_HUD)
	return;
GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);

if (LOCALPLAYER.flags & PLAYER_FLAGS_CLOAKED) {
	int	y = grdCurCanv->cv_h;

	if (gameData.app.nGameMode & GM_MULTI)
		y -= 7*nLineSpacing;
	else
		y -= 4*nLineSpacing;
	if ((LOCALPLAYER.cloakTime+CLOAK_TIME_MAX - gameData.time.xGame > F1_0*3) || (gameData.time.xGame & 0x8000))
		GrPrintF (2, y, "%s", TXT_CLOAKED);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) {
	int	y = grdCurCanv->cv_h;

	if (gameData.app.nGameMode & GM_MULTI)
		y -= 10*nLineSpacing;
	else
		y -= 5*nLineSpacing;
	if (((LOCALPLAYER.invulnerableTime + INVULNERABLE_TIME_MAX - gameData.time.xGame) > F1_0*4) || (gameData.time.xGame & 0x8000))
		GrPrintF (2, y, "%s", TXT_INVULNERABLE);
	}
}

//	-----------------------------------------------------------------------------

void HUDShowShield (void)
{
	int	h, y;

if (!SHOW_HUD)
	return;
//	GrSetCurrentCanvas (&gameStates.render.vr.buffers.subRender [0]);	//render off-screen

h = (LOCALPLAYER.shields >= 0) ? f2ir (LOCALPLAYER.shields) : 0; 
if (gameOpts->render.cockpit.bTextGauges) {
	y = grdCurCanv->cv_h - ((gameData.app.nGameMode & GM_MULTI) ? 6 : 2) * nLineSpacing;
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	GrPrintF (2, y, "%s: %i", TXT_SHIELD, h);
	}
else {
	static int		bShow = 1;
	static time_t	tToggle = 0, nBeep = -1;
	time_t			t = gameStates.app.nSDLTicks;
	int				bLastFlash = gameStates.gameplay.nShieldFlash;

	if ((t = HUDShowFlashGauge (h, &gameStates.gameplay.nShieldFlash, (int) tToggle))) {
		tToggle = t;
		bShow = !bShow;
		}
	y = grdCurCanv->cv_h - (int) ((((gameData.app.nGameMode & GM_MULTI) ? 6 : 2) * nLineSpacing - 1) * yScale);
	GrSetColorRGB (0, (ubyte) ((h > 100) ? 255 : 64), 255, 255);
	GrUBox (6, y, 6 + (int) (100 * xScale), y + (int) (9 * yScale));
	if (gameStates.gameplay.nShieldFlash) {
		if (gameOpts->gameplay.bShieldWarning) {
			if ((nBeep < 0) || (bLastFlash != gameStates.gameplay.nShieldFlash)) {
				if (nBeep >= 0)
					DigiStopSound ((int) nBeep);
				nBeep = DigiStartSound (DigiXlatSound (SOUND_HUD_MESSAGE), F1_0 * 2 / 3, 0xFFFF / 2, 
												  -1, -1, -1, -1, (gameStates.gameplay.nShieldFlash == 1) ? F1_0 * 3 / 4 : F1_0 / 2, NULL);
				}
			}
		else if (nBeep >= 0) {
			DigiStopSound ((int) nBeep);
			nBeep = -1;
			}
		if (!bShow)
			goto skipGauge;
		h = 100;
		}
	else {
		if (nBeep >= 0) {
			DigiStopSound ((int) nBeep);
			nBeep = -1;
			}
		}
	GrSetColorRGB (0, (ubyte) ((h > 100) ? 224 : 64), 224, 128);
	GrURect (6, y, 6 + (int) (((h > 100) ? h - 100 : h) * xScale), y + (int) (9 * yScale));
	}

skipGauge:

if (gameData.demo.nState==ND_STATE_RECORDING) {
	int shields = f2ir (LOCALPLAYER.shields);

	if (shields != old_shields [gameStates.render.vr.nCurrentPage]) {		// Draw the shield gauge
		NDRecordPlayerShields (old_shields [gameStates.render.vr.nCurrentPage], shields);
		old_shields [gameStates.render.vr.nCurrentPage] = shields;
		}
	}
}

//	-----------------------------------------------------------------------------

//draw the icons for number of lives
void HUDShowLives ()
{
if (!SHOW_HUD)
	return;
if ((gameData.hud.msgs [0].nMessages > 0) && (strlen (gameData.hud.msgs [0].szMsgs [gameData.hud.msgs [0].nFirst]) > 38))
	return;
	if (gameData.app.nGameMode & GM_MULTI) {
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	GrPrintF (10, 3, "%s: %d", TXT_DEATHS, LOCALPLAYER.netKilledTotal);
	} 
else if (LOCALPLAYER.lives > 1)  {
	grsBitmap *bm;
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
	PAGE_IN_GAUGE (GAUGE_LIVES);
	bm = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_LIVES);
	GrUBitmapM (10, 3, bm);
	GrPrintF (10+bm->bm_props.w+bm->bm_props.w/2, 4, "x %d", LOCALPLAYER.lives-1);
	}
}

//	-----------------------------------------------------------------------------

void SBShowLives ()
{
	int x, y;
	grsBitmap * bm = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_LIVES);
   int frc=0;
	x = SB_LIVES_X;
	y = SB_LIVES_Y;
  
if ((old_lives [gameStates.render.vr.nCurrentPage] == -1) || frc) {
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
	if (gameData.app.nGameMode & GM_MULTI) {
		HUDPrintF (SB_LIVES_LABEL_X, SB_LIVES_LABEL_Y, "%s:", TXT_DEATHS);
		}
	else {
		HUDPrintF (SB_LIVES_LABEL_X, SB_LIVES_LABEL_Y, "%s:", TXT_LIVES);
		}
	}
if (gameData.app.nGameMode & GM_MULTI) {
	char killed_str [20];
	int w, h, aw;
	static int last_x [4] = {SB_SCORE_RIGHT_L, SB_SCORE_RIGHT_L, SB_SCORE_RIGHT_H, SB_SCORE_RIGHT_H};
	int x;

		sprintf (killed_str, "%5d", LOCALPLAYER.netKilledTotal);
		GrGetStringSize (killed_str, &w, &h, &aw);
		GrSetColorRGBi (RGBA_PAL (0, 0, 0));
		HUDRect (last_x [ (gameStates.video.nDisplayMode?2:0)+gameStates.render.vr.nCurrentPage], y+1, SB_SCORE_RIGHT, y+GAME_FONT->ft_h);
		GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
		x = SB_SCORE_RIGHT-w-2;		
		HUDPrintF (x, y+1, killed_str);
		last_x [ (gameStates.video.nDisplayMode?2:0)+gameStates.render.vr.nCurrentPage] = x;
		return;
	}
if (frc || (old_lives [gameStates.render.vr.nCurrentPage] == -1) || 
	 LOCALPLAYER.lives != old_lives [gameStates.render.vr.nCurrentPage]) {
//erase old icons
	GrSetColorRGBi (RGBA_PAL (0, 0, 0));
   
	HUDRect (x, y, SB_SCORE_RIGHT, y+bm->bm_props.h);
	HUDRect (x, y, SB_SCORE_RIGHT, y+bm->bm_props.h);
	if (LOCALPLAYER.lives-1 > 0) {
		GrSetCurFont (GAME_FONT);
		GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
		PAGE_IN_GAUGE (GAUGE_LIVES);
		HUDBitBlt (x, y, bm, F1_0, 0);
		HUDPrintF (x+bm->bm_props.w+GAME_FONT->ft_w, y, "x %d", LOCALPLAYER.lives-1);
		}
	}
}

//	-----------------------------------------------------------------------------

#ifdef PIGGY_USE_PAGING
extern int Piggy_bitmap_cache_next;
#endif

void ShowTime ()
{
	int secs = f2i (LOCALPLAYER.timeLevel) % 60;
	int mins = f2i (LOCALPLAYER.timeLevel) / 60;

GrSetCurFont (GAME_FONT);
GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
GrPrintF (grdCurCanv->cv_w-4*GAME_FONT->ft_w, grdCurCanv->cv_h-4*nLineSpacing, "%d:%02d", mins, secs);

//@@#ifdef PIGGY_USE_PAGING
//@@	{
//@@		char text [25];
//@@		int w, h, aw;
//@@		sprintf (text, "%d KB", Piggy_bitmap_cache_next/1024);
//@@		GrGetStringSize (text, &w, &h, &aw);	
//@@		GrPrintF (grdCurCanv->cv_w-10-w, grdCurCanv->cv_h/2, text);
//@@	}
//@@#endif

}

//	-----------------------------------------------------------------------------

#define EXTRA_SHIP_SCORE	50000		//get new ship every this many points

void AddPointsToScore (int points) 
{
	int prev_score;

	scoreTime += f1_0*2;
	score_display [0] += points;
	score_display [1] += points;
	if (scoreTime > f1_0*4) scoreTime = f1_0*4;
	if (points == 0 || gameStates.app.cheats.bEnabled)
		return;
	if ((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP))
		return;
	prev_score=LOCALPLAYER.score;
	LOCALPLAYER.score += points;
	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordPlayerScore (points);
	if (gameData.app.nGameMode & GM_MULTI_COOP)
		MultiSendScore ();
	if (gameData.app.nGameMode & GM_MULTI)
		return;
	if (LOCALPLAYER.score/EXTRA_SHIP_SCORE != prev_score/EXTRA_SHIP_SCORE) {
		short snd;
		LOCALPLAYER.lives += LOCALPLAYER.score/EXTRA_SHIP_SCORE - prev_score/EXTRA_SHIP_SCORE;
		PowerupBasic (20, 20, 20, 0, TXT_EXTRA_LIFE);
		if ((snd=gameData.objs.pwrUp.info [POW_EXTRA_LIFE].hitSound) > -1)
			DigiPlaySample (snd, F1_0);
	}
}

//	-----------------------------------------------------------------------------

void AddBonusPointsToScore (int points) 
{
	int prev_score;

	if (points == 0 || gameStates.app.cheats.bEnabled)
		return;

	prev_score=LOCALPLAYER.score;

	LOCALPLAYER.score += points;


	if (gameData.demo.nState == ND_STATE_RECORDING)
		NDRecordPlayerScore (points);

	if (gameData.app.nGameMode & GM_MULTI)
		return;

	if (LOCALPLAYER.score/EXTRA_SHIP_SCORE != prev_score/EXTRA_SHIP_SCORE) {
		short snd;
		LOCALPLAYER.lives += LOCALPLAYER.score/EXTRA_SHIP_SCORE - prev_score/EXTRA_SHIP_SCORE;
		if ((snd=gameData.objs.pwrUp.info [POW_EXTRA_LIFE].hitSound) > -1)
			DigiPlaySample (snd, F1_0);
	}
}

//	-----------------------------------------------------------------------------

#include "key.h"

static int bHaveGaugeCanvases = 0;

void InitGaugeCanvases ()
{
if (!bHaveGaugeCanvases && gamePalette) {
	bHaveGaugeCanvases = 1;
	PAGE_IN_GAUGE (SB_GAUGE_ENERGY);
	PAGE_IN_GAUGE (GAUGE_AFTERBURNER);
	Canv_LeftEnergyGauge = GrCreateCanvas (LEFT_ENERGY_GAUGE_W, LEFT_ENERGY_GAUGE_H);
	Canv_SBEnergyGauge = GrCreateCanvas (SB_ENERGY_GAUGE_W, SB_ENERGY_GAUGE_H);
	Canv_SBAfterburnerGauge = GrCreateCanvas (SB_AFTERBURNER_GAUGE_W, SB_AFTERBURNER_GAUGE_H);
	Canv_RightEnergyGauge = GrCreateCanvas (RIGHT_ENERGY_GAUGE_W, RIGHT_ENERGY_GAUGE_H);
	Canv_NumericalGauge = GrCreateCanvas (NUMERICAL_GAUGE_W, NUMERICAL_GAUGE_H);
	Canv_AfterburnerGauge = GrCreateCanvas (AFTERBURNER_GAUGE_W, AFTERBURNER_GAUGE_H);
	}
}

//	-----------------------------------------------------------------------------

void CloseGaugeCanvases ()
{
if (bHaveGaugeCanvases) {
	GrFreeCanvas (Canv_LeftEnergyGauge);
	GrFreeCanvas (Canv_SBEnergyGauge);
	GrFreeCanvas (Canv_SBAfterburnerGauge);
	GrFreeCanvas (Canv_RightEnergyGauge);
	GrFreeCanvas (Canv_NumericalGauge);
	GrFreeCanvas (Canv_AfterburnerGauge);
	bHaveGaugeCanvases = 0;
	}
}

//	-----------------------------------------------------------------------------

void InitGauges ()
{
	int i;

	//draw_gauges_on 	= 1;

	for (i=0; i<2; i++)	{
		if (((gameData.app.nGameMode & GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP)) || ((gameData.demo.nState == ND_STATE_PLAYBACK) && (gameData.demo.nGameMode & GM_MULTI) && !(gameData.demo.nGameMode & GM_MULTI_COOP))) 
			old_score [i] = -99;
		else
			old_score [i]			= -1;
		old_energy [i]			= -1;
		old_shields [i]			= -1;
		oldFlags [i]			= -1;
		old_cloak [i]			= -1;
		old_lives [i]			= -1;
		old_afterburner [i]	= -1;
		oldBombcount [i]		= 0;
		old_laserLevel [i]	= 0;
	
		old_weapon [0][i] = old_weapon [1][i] = -1;
		old_ammoCount [0][i] = old_ammoCount [1][i] = -1;
		Old_Omega_charge [i] = -1;
	}

	nCloakFadeState = 0;

	weapon_box_user [0] = weapon_box_user [1] = WBU_WEAPON;
}

//	-----------------------------------------------------------------------------

void DrawEnergyBar (int energy)
{
	grsBitmap *bm;
	int energy0;
	int x1, x2, y, yMax, i;
	int h0 = HUD_SCALE_X (LEFT_ENERGY_GAUGE_H - 1);
	int h1 = HUD_SCALE_Y (LEFT_ENERGY_GAUGE_H / 4);
	int h2 = HUD_SCALE_Y ((LEFT_ENERGY_GAUGE_H * 3) / 4);
	int w1 = HUD_SCALE_X (LEFT_ENERGY_GAUGE_W - 1);
	int w2 = HUD_SCALE_X (LEFT_ENERGY_GAUGE_W - 2);
	int w3 = HUD_SCALE_X (LEFT_ENERGY_GAUGE_W - 3);
	double eBarScale = (100.0 - (double) energy) * cmScaleX * 0.15 / (double) HUD_SCALE_Y (LEFT_ENERGY_GAUGE_H);

// Draw left energy bar
PAGE_IN_GAUGE (GAUGE_ENERGY_LEFT);
bm = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_ENERGY_LEFT);
HUDBitBlt (LEFT_ENERGY_GAUGE_X, LEFT_ENERGY_GAUGE_Y, bm, F1_0, 0);
#ifdef _DEBUG
	GrSetColorRGBi (RGBA_PAL (255, 255, 255));
#else
	GrSetColorRGBi (RGBA_PAL (0, 0, 0));
#endif
	//energy0 = (gameStates.video.nDisplayMode ? 125 - (energy * 125) / 100 : 61 - (energy * 61) / 100);
	energy0 = HUD_SCALE_X (112);
	energy0 = energy0 - (energy * energy0) / 100;
	//energy0 = HUD_SCALE_X (energy0);
	if (energy < 100) {
		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
		for (i = 0; i < LEFT_ENERGY_GAUGE_H; i++) {
			yMax = HUD_SCALE_Y (i + 1);
			for (y = i; y <= yMax; y++) {
				x1 = h0 - y;
				x2 = x1 + energy0 + (int) ((double) y * eBarScale);
				if (y < h1) {
					if (x2 > w1) 
						x2 = w1;
					}
				else if (y < h2) {
					if (x2 > w2)
						x2 = w2;
					}
				else {
					if (x2 > w3) 
						x2 = w3;
					}
				if (x2 > x1)
					gr_uscanline (
						HUD_SCALE_X (LEFT_ENERGY_GAUGE_X) + x1, 
						HUD_SCALE_X (LEFT_ENERGY_GAUGE_X) + x2, 
						HUD_SCALE_Y (LEFT_ENERGY_GAUGE_Y) + y); 
				}
			}
		}
	GrSetCurrentCanvas (GetCurrentGameScreen ());
	// Draw right energy bar
//	GrSetCurrentCanvas (Canv_RightEnergyGauge);
	PAGE_IN_GAUGE (GAUGE_ENERGY_RIGHT);
	bm = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_ENERGY_RIGHT);
//	GrUBitmapM (0, 0, bm);
//	GrUBitmapM (RIGHT_ENERGY_GAUGE_X, RIGHT_ENERGY_GAUGE_Y, bm);
	HUDBitBlt (RIGHT_ENERGY_GAUGE_X, RIGHT_ENERGY_GAUGE_Y, bm, F1_0, 0);
#ifdef _DEBUG
	GrSetColorRGBi (RGBA_PAL (255, 255, 255));
#else
	GrSetColorRGBi (RGBA_PAL (0, 0, 0));
#endif
	h0 = HUD_SCALE_X (RIGHT_ENERGY_GAUGE_W - RIGHT_ENERGY_GAUGE_H);
	w1 = HUD_SCALE_X (1);
	w2 = HUD_SCALE_X (2);
	if (energy < 100) {
		yMax = HUD_SCALE_Y (RIGHT_ENERGY_GAUGE_H);
		for (i = 0; i < RIGHT_ENERGY_GAUGE_H; i++) {
			yMax = HUD_SCALE_Y (i + 1);
			for (y = i; y <= yMax; y++) {
				x2 = h0 + y;
				x1 = x2 - energy0 - (int) ((double) y * eBarScale);
				if (y < h1) {
					if (x1 < 0) 
						x1 = 0;
					}
				else if (y < h2) {
					if (x1 < w1) 
						x1 = w1;
					}
				else {
					if (x1 < w2) 
						x1 = w2;
					}
				if (x2 > x1) 
					gr_uscanline (
						HUD_SCALE_X (RIGHT_ENERGY_GAUGE_X) + x1, 
						HUD_SCALE_X (RIGHT_ENERGY_GAUGE_X) + x2, 
						HUD_SCALE_Y (RIGHT_ENERGY_GAUGE_Y) + y); 
				}
			}
		}
GrSetCurrentCanvas (GetCurrentGameScreen ());
}

//	-----------------------------------------------------------------------------

ubyte afterburner_bar_table [AFTERBURNER_GAUGE_H_L*2] = {
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

ubyte afterburner_bar_table_hires [AFTERBURNER_GAUGE_H_H*2] = {
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

void DrawAfterburnerBar (int afterburner)
{
	int not_afterburner;
	int i, j, y, yMax;
	grsBitmap *bm;
	ubyte *pabt = gameStates.video.nDisplayMode ? afterburner_bar_table_hires : afterburner_bar_table;

// Draw afterburner bar
//GrSetCurrentCanvas (Canv_AfterburnerGauge);
PAGE_IN_GAUGE (GAUGE_AFTERBURNER);
bm =  gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_AFTERBURNER);
//	GrUBitmapM (0, 0, bm);
//	GrUBitmapM (AFTERBURNER_GAUGE_X, AFTERBURNER_GAUGE_Y, bm);
HUDBitBlt (AFTERBURNER_GAUGE_X, AFTERBURNER_GAUGE_Y, bm, F1_0, 0);
GrSetColorRGB (0, 0, 0, 255);
not_afterburner = FixMul (f1_0 - afterburner, AFTERBURNER_GAUGE_H);
gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
yMax = HUD_SCALE_Y (not_afterburner);
for (y = 0; y < not_afterburner; y++) {
	for (i = HUD_SCALE_Y (y), j = HUD_SCALE_Y (y + 1); i < j; i++) {
		gr_uscanline (
			HUD_SCALE_X (AFTERBURNER_GAUGE_X + pabt [y * 2]), 
			HUD_SCALE_X (AFTERBURNER_GAUGE_X + pabt [y * 2 + 1] + 1), 
			HUD_SCALE_Y (AFTERBURNER_GAUGE_Y) + i);
		}
	}
GrSetCurrentCanvas (GetCurrentGameScreen ());
}

//	-----------------------------------------------------------------------------

void DrawShieldBar (int shield)
{
	int bm_num = shield>=100?9: (shield / 10);

PAGE_IN_GAUGE (GAUGE_SHIELDS+9-bm_num	);
HUDBitBlt (SHIELD_GAUGE_X, SHIELD_GAUGE_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_SHIELDS+9-bm_num), F1_0, 0);
}

#define CLOAK_FADE_WAIT_TIME  0x400

//	-----------------------------------------------------------------------------

void DrawPlayerShip (int nCloakState, int nOldCloakState, int x, int y)
{
	static fix xCloakFadeTimer=0;
	static int nCloakFadeValue=GR_ACTUAL_FADE_LEVELS-1;
	static int refade = 0;
	grsBitmap *bm = NULL;

	if (gameData.app.nGameMode & GM_TEAM)	{
		PAGE_IN_GAUGE (GAUGE_SHIPS+GetTeam (gameData.multiplayer.nLocalPlayer));
		bm =gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_SHIPS+GetTeam (gameData.multiplayer.nLocalPlayer));
	} else {
		PAGE_IN_GAUGE (GAUGE_SHIPS+gameData.multiplayer.nLocalPlayer);
		bm = gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_SHIPS+gameData.multiplayer.nLocalPlayer);
	}
	

	if (nOldCloakState==-1 && nCloakState)
			nCloakFadeValue=0;

	if (!nCloakState) {
		nCloakFadeValue=GR_ACTUAL_FADE_LEVELS-1;
		nCloakFadeState = 0;
	}

	if (nCloakState==1 && nOldCloakState==0)
		nCloakFadeState = -1;
	//else if (nCloakState==0 && nOldCloakState==1)
	//	nCloakFadeState = 1;

	if (nCloakState==nOldCloakState)		//doing "about-to-uncloak" effect
		if (nCloakFadeState==0)
			nCloakFadeState = 2;
	

	if (nCloakFadeState)
		xCloakFadeTimer -= gameData.time.xFrame;

	while (nCloakFadeState && xCloakFadeTimer < 0) {
		xCloakFadeTimer += CLOAK_FADE_WAIT_TIME;
		nCloakFadeValue += nCloakFadeState;
		if (nCloakFadeValue >= GR_ACTUAL_FADE_LEVELS-1) {
			nCloakFadeValue = GR_ACTUAL_FADE_LEVELS-1;
			if (nCloakFadeState == 2 && nCloakState)
				nCloakFadeState = -2;
			else
				nCloakFadeState = 0;
		}
		else if (nCloakFadeValue <= 0) {
			nCloakFadeValue = 0;
			if (nCloakFadeState == -2)
				nCloakFadeState = 2;
			else
				nCloakFadeState = 0;
		}
	}

//	To fade out both pages in a paged mode.
	if (refade) 
		refade = 0;
	else if (nCloakState && nOldCloakState && !nCloakFadeState && !refade) {
		nCloakFadeState = -1;
		refade = 1;
	}
	if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT) {
		GrSetCurrentCanvas (&gameStates.render.vr.buffers.render [0]);
	}
		if (nCloakFadeValue >= GR_ACTUAL_FADE_LEVELS - 1)
			//gr_ubitmap (x, y, bm);
		HUDBitBlt (x, y, bm, F1_0, 0);

//		gameStates.render.grAlpha = nCloakFadeValue;
//		GrRect (x, y, x+bm->bm_props.w-1, y+bm->bm_props.h-1);
//		gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
	if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT) {
		GrSetCurrentCanvas (GetCurrentGameScreen ());
	}

	GrBmUBitBltM (
		 (int) (bm->bm_props.w * cmScaleX), (int) (bm->bm_props.h * cmScaleY), 
		 (int) (x * cmScaleX), (int) (y * cmScaleY), x, y, 
		&gameStates.render.vr.buffers.render [0].cv_bitmap, &grdCurCanv->cv_bitmap);
}

#define INV_FRAME_TIME	 (f1_0/10)		//how long for each frame

//	-----------------------------------------------------------------------------

inline int NumDispX (int val)
{
int x = ((val > 99) ? 7 : (val > 9) ? 11 : 15);
if (!gameStates.video.nDisplayMode)
	x /= 2;
return x + NUMERICAL_GAUGE_X;
}

void DrawNumericalDisplay (int shield, int energy)
{
	int dx = NUMERICAL_GAUGE_X, 
		 dy = NUMERICAL_GAUGE_Y;

	if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT)
		GrSetCurrentCanvas (Canv_NumericalGauge);
	PAGE_IN_GAUGE (GAUGE_NUMERICAL);
//	gr_ubitmap (dx, dy, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_NUMERICAL));
	HUDBitBlt (dx, dy, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_NUMERICAL), F1_0, 0);
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (RGBA_PAL2 (14, 14, 23), 1, 0, 0);
	HUDPrintF (NumDispX (shield), dy + (gameStates.video.nDisplayMode ? 36 : 16), "%d", shield);
	GrSetFontColorRGBi (RGBA_PAL2 (25, 18, 6), 1, 0, 0);
	HUDPrintF (NumDispX (energy), dy + (gameStates.video.nDisplayMode ? 5 : 2), "%d", energy);
	
	if (gameStates.render.cockpit.nMode != CM_FULL_COCKPIT) {
		GrSetCurrentCanvas (GetCurrentGameScreen ());
		HUDBitBlt (NUMERICAL_GAUGE_X, NUMERICAL_GAUGE_Y, &Canv_NumericalGauge->cv_bitmap, F1_0, 0);
	}
}


//	-----------------------------------------------------------------------------

void DrawKeys ()
{
GrSetCurrentCanvas (GetCurrentGameScreen ());

if (LOCALPLAYER.flags & PLAYER_FLAGS_BLUE_KEY)	{
	PAGE_IN_GAUGE (GAUGE_BLUE_KEY);
	HUDBitBlt (GAUGE_BLUE_KEY_X, GAUGE_BLUE_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_BLUE_KEY), F1_0, 0);
	} 
else {
	PAGE_IN_GAUGE (GAUGE_BLUE_KEY_OFF);
	HUDBitBlt (GAUGE_BLUE_KEY_X, GAUGE_BLUE_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_BLUE_KEY_OFF), F1_0, 0);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_GOLD_KEY)	{
	PAGE_IN_GAUGE (GAUGE_GOLD_KEY);
	HUDBitBlt (GAUGE_GOLD_KEY_X, GAUGE_GOLD_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_GOLD_KEY), F1_0, 0);
	} 
else {
	PAGE_IN_GAUGE (GAUGE_GOLD_KEY_OFF);
	HUDBitBlt (GAUGE_GOLD_KEY_X, GAUGE_GOLD_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_GOLD_KEY_OFF), F1_0, 0);
	}
if (LOCALPLAYER.flags & PLAYER_FLAGS_RED_KEY)	{
	PAGE_IN_GAUGE (GAUGE_RED_KEY);
	HUDBitBlt (GAUGE_RED_KEY_X,  GAUGE_RED_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_RED_KEY), F1_0, 0);
	} 
else {
	PAGE_IN_GAUGE (GAUGE_RED_KEY_OFF);
	HUDBitBlt (GAUGE_RED_KEY_X,  GAUGE_RED_KEY_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (GAUGE_RED_KEY_OFF), F1_0, 0);
	}
}

//	-----------------------------------------------------------------------------

void DrawWeaponInfoSub (int info_index, gauge_box *box, int pic_x, int pic_y, char *name, int text_x, 
								  int text_y, int orient)
{
	grsBitmap *bmP;
	char *p;

	//clear the window
#if 0
	if (gameStates.render.cockpit.nMode != CM_FULL_SCREEN) {
		GrSetColorRGBi (RGBA_PAL (0, 0, 0));
		GrRect ((int) (box->left * cmScaleX), (int) (box->top * cmScaleY), 
				  (int) (box->right * cmScaleX), (int) (box->bot * cmScaleY));
		}
#endif
	if ((gameData.pig.tex.nHamFileVersion >= 3) // !SHAREWARE
		&& gameStates.video.nDisplayMode) {
		bmP = gameData.pig.tex.bitmaps [0] + gameData.weapons.info [info_index].hires_picture.index;
		PIGGY_PAGE_IN (gameData.weapons.info [info_index].hires_picture, 0);
		}
	else {
		bmP=gameData.pig.tex.bitmaps [0] + gameData.weapons.info [info_index].picture.index;
		PIGGY_PAGE_IN (gameData.weapons.info [info_index].picture, 0);
		}
	Assert (bmP != NULL);

	HUDBitBlt (pic_x, pic_y, bmP, (gameStates.render.cockpit.nMode == CM_FULL_SCREEN) ? 2 * F1_0 : F1_0, orient);
	if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN)
		return;
	GrSetFontColorRGBi (MEDGREEN_RGBA, 1, 0, 0);
	if ((p=strchr (name, '\n'))!=NULL) {
		*p=0;
		HUDPrintF (text_x, text_y, name);
		HUDPrintF (text_x, text_y+grdCurCanv->cv_font->ft_h+1, p+1);
		*p='\n';
		}
	else {
		HUDPrintF (text_x, text_y, name);
	}	

	//	For laser, show level and quadness
	if (info_index == LASER_ID || info_index == SUPERLASER_ID) {
		char	temp_str [7];

		sprintf (temp_str, "%s: 0", TXT_LVL);

		temp_str [5] = LOCALPLAYER.laserLevel+1 + '0';

		HUDPrintF (text_x, text_y+nLineSpacing, temp_str);
		if (LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS) {
			strcpy (temp_str, TXT_QUAD);
			HUDPrintF (text_x, text_y+2*nLineSpacing, temp_str);
		}
	}
}

//	-----------------------------------------------------------------------------

void DrawWeaponInfo (int nWeaponType, int nWeaponNum, int laserLevel)
{
	int info_index;

if (nWeaponType == 0) {
	info_index = primaryWeaponToWeaponInfo [nWeaponNum];

	if (info_index == LASER_ID && laserLevel > MAX_LASER_LEVEL)
		info_index = SUPERLASER_ID;

	if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
		DrawWeaponInfoSub (info_index, 
			gauge_boxes + SB_PRIMARY_BOX, 
			SB_PRIMARY_W_PIC_X, SB_PRIMARY_W_PIC_Y, 
			PRIMARY_WEAPON_NAMES_SHORT (nWeaponNum), 
			SB_PRIMARY_W_TEXT_X, SB_PRIMARY_W_TEXT_Y, 0);
#if 0
	else if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN)
		DrawWeaponInfoSub (info_index, 
			gauge_boxes + SB_PRIMARY_BOX, 
			SB_PRIMARY_W_PIC_X, SB_PRIMARY_W_PIC_Y, 
			PRIMARY_WEAPON_NAMES_SHORT (nWeaponNum), 
			SB_PRIMARY_W_TEXT_X, SB_PRIMARY_W_TEXT_Y, 3);
#endif
	else
		DrawWeaponInfoSub (info_index, 
			gauge_boxes + COCKPIT_PRIMARY_BOX, 
			PRIMARY_W_PIC_X, PRIMARY_W_PIC_Y, 
			PRIMARY_WEAPON_NAMES_SHORT (nWeaponNum), 
			PRIMARY_W_TEXT_X, PRIMARY_W_TEXT_Y, 0);
		}
else {
	info_index = secondaryWeaponToWeaponInfo [nWeaponNum];

	if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
		DrawWeaponInfoSub (info_index, 
			gauge_boxes + SB_SECONDARY_BOX, 
			SB_SECONDARY_W_PIC_X, SB_SECONDARY_W_PIC_Y, 
			SECONDARY_WEAPON_NAMES_SHORT (nWeaponNum), 
			SB_SECONDARY_W_TEXT_X, SB_SECONDARY_W_TEXT_Y, 0);
#if 0
	else if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN)
		DrawWeaponInfoSub (info_index, 
			gauge_boxes + COCKPIT_SECONDARY_BOX, 
			SECONDARY_W_PIC_X, SECONDARY_W_PIC_Y, 
			SECONDARY_WEAPON_NAMES_SHORT (nWeaponNum), 
			SECONDARY_W_TEXT_X, SECONDARY_W_TEXT_Y, 1);
#endif
	else
		DrawWeaponInfoSub (info_index, 
			gauge_boxes + COCKPIT_SECONDARY_BOX, 
			SECONDARY_W_PIC_X, SECONDARY_W_PIC_Y, 
			SECONDARY_WEAPON_NAMES_SHORT (nWeaponNum), 
			SECONDARY_W_TEXT_X, SECONDARY_W_TEXT_Y, 0);
	}
}

//	-----------------------------------------------------------------------------

void DrawAmmoInfo (int x, int y, int ammoCount, int primary)
{
	int w;
	char str [16];

if (primary)
	w = (grdCurCanv->cv_font->ft_w*7)/2;
else
	w = (grdCurCanv->cv_font->ft_w*5)/2;
GrSetColorRGBi (RGBA_PAL (0, 0, 0));
GrRect (HUD_SCALE_X (x), HUD_SCALE_Y (y), HUD_SCALE_X (x+w), HUD_SCALE_Y (y + grdCurCanv->cv_font->ft_h));
GrSetFontColorRGBi (MEDRED_RGBA, 1, 0, 0);
sprintf (str, "%03d", ammoCount);
convert_1s (str);
HUDPrintF (x, y, str);
GrRect (HUD_SCALE_X (x), HUD_SCALE_Y (y), HUD_SCALE_X (x+w), HUD_SCALE_Y (y + grdCurCanv->cv_font->ft_h));
HUDPrintF (x, y, str);
}

//	-----------------------------------------------------------------------------

void DrawSecondaryAmmoInfo (int ammoCount)
{
if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
	DrawAmmoInfo (SB_SECONDARY_AMMO_X, SB_SECONDARY_AMMO_Y, ammoCount, 0);
else
	DrawAmmoInfo (SECONDARY_AMMO_X, SECONDARY_AMMO_Y, ammoCount, 0);
}

//	-----------------------------------------------------------------------------

//returns true if drew picture
int DrawWeaponBox (int nWeaponType, int nWeaponNum)
{
	int bDrew = 0;
	int bLaserLevelChanged;

GrSetCurrentCanvas (&gameStates.render.vr.buffers.render [0]);
GrSetCurFont (GAME_FONT);
bLaserLevelChanged = ((nWeaponType == 0) && 
								(nWeaponNum == LASER_INDEX) && 
								((LOCALPLAYER.laserLevel != old_laserLevel [gameStates.render.vr.nCurrentPage])));

if ((nWeaponNum != old_weapon [nWeaponType][gameStates.render.vr.nCurrentPage] || bLaserLevelChanged) && weapon_boxStates [nWeaponType] == WS_SET) {
	weapon_boxStates [nWeaponType] = WS_FADING_OUT;
	weapon_box_fadeValues [nWeaponType]=i2f (GR_ACTUAL_FADE_LEVELS-1);
	}
	
if ((old_weapon [nWeaponType][gameStates.render.vr.nCurrentPage] == -1) || 1/* (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT)*/) {
	DrawWeaponInfo (nWeaponType, nWeaponNum, LOCALPLAYER.laserLevel);
	old_weapon [nWeaponType][gameStates.render.vr.nCurrentPage] = nWeaponNum;
	old_ammoCount [nWeaponType][gameStates.render.vr.nCurrentPage] = -1;
	Old_Omega_charge [gameStates.render.vr.nCurrentPage] = -1;
	old_laserLevel [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.laserLevel;
	bDrew = 1;
	weapon_boxStates [nWeaponType] = WS_SET;
	}

if (weapon_boxStates [nWeaponType] == WS_FADING_OUT) {
	DrawWeaponInfo (nWeaponType, old_weapon [nWeaponType][gameStates.render.vr.nCurrentPage], old_laserLevel [gameStates.render.vr.nCurrentPage]);
	old_ammoCount [nWeaponType][gameStates.render.vr.nCurrentPage] = -1;
	Old_Omega_charge [gameStates.render.vr.nCurrentPage] = -1;
	bDrew = 1;
	weapon_box_fadeValues [nWeaponType] -= gameData.time.xFrame * FADE_SCALE;
	if (weapon_box_fadeValues [nWeaponType] <= 0) {
		weapon_boxStates [nWeaponType] = WS_FADING_IN;
		old_weapon [nWeaponType][gameStates.render.vr.nCurrentPage] = nWeaponNum;
		old_weapon [nWeaponType][!gameStates.render.vr.nCurrentPage] = nWeaponNum;
		old_laserLevel [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.laserLevel;
		old_laserLevel [!gameStates.render.vr.nCurrentPage] = LOCALPLAYER.laserLevel;
		weapon_box_fadeValues [nWeaponType] = 0;
		}
	}
else if (weapon_boxStates [nWeaponType] == WS_FADING_IN) {
	if (nWeaponNum != old_weapon [nWeaponType][gameStates.render.vr.nCurrentPage]) {
		weapon_boxStates [nWeaponType] = WS_FADING_OUT;
		}
	else {
		DrawWeaponInfo (nWeaponType, nWeaponNum, LOCALPLAYER.laserLevel);
		old_ammoCount [nWeaponType][gameStates.render.vr.nCurrentPage] = -1;
		Old_Omega_charge [gameStates.render.vr.nCurrentPage] = -1;
		bDrew=1;
		weapon_box_fadeValues [nWeaponType] += gameData.time.xFrame * FADE_SCALE;
		if (weapon_box_fadeValues [nWeaponType] >= i2f (GR_ACTUAL_FADE_LEVELS-1)) {
			weapon_boxStates [nWeaponType] = WS_SET;
			old_weapon [nWeaponType][!gameStates.render.vr.nCurrentPage] = -1;		//force redraw (at full fade-in) of other page
			}
		}
	}

if (weapon_boxStates [nWeaponType] != WS_SET) {		//fade gauge
	int fadeValue = f2i (weapon_box_fadeValues [nWeaponType]);
	int boxofs = (gameStates.render.cockpit.nMode == CM_STATUS_BAR) ? SB_PRIMARY_BOX : COCKPIT_PRIMARY_BOX;
	gameStates.render.grAlpha = (float) fadeValue;
	GrRect (gauge_boxes [boxofs + nWeaponType].left, 
			  gauge_boxes [boxofs + nWeaponType].top, 
			  gauge_boxes [boxofs + nWeaponType].right, 
			  gauge_boxes [boxofs + nWeaponType].bot);
	gameStates.render.grAlpha = GR_ACTUAL_FADE_LEVELS;
	}
GrSetCurrentCanvas (GetCurrentGameScreen ());
return bDrew;
}

//	-----------------------------------------------------------------------------

fix staticTime [2];

void DrawStatic (int win)
{
	tVideoClip *vc = gameData.eff.vClips [0] + VCLIP_MONITOR_STATIC;
	grsBitmap *bmp;
	int framenum;
	int boxofs = (gameStates.render.cockpit.nMode==CM_STATUS_BAR)?SB_PRIMARY_BOX:COCKPIT_PRIMARY_BOX;
	int x, y;

	staticTime [win] += gameData.time.xFrame;
	if (staticTime [win] >= vc->xTotalTime) {
		weapon_box_user [win] = WBU_WEAPON;
		return;
		}
	framenum = staticTime [win] * vc->nFrameCount / vc->xTotalTime;
	PIGGY_PAGE_IN (vc->frames [framenum], 0);
	bmp = gameData.pig.tex.bitmaps [0] + vc->frames [framenum].index;
	GrSetCurrentCanvas (&gameStates.render.vr.buffers.render [0]);
	for (x=gauge_boxes [boxofs+win].left;x<gauge_boxes [boxofs+win].right;x+=bmp->bm_props.w)
		for (y=gauge_boxes [boxofs+win].top;y<gauge_boxes [boxofs+win].bot;y+=bmp->bm_props.h)
			/*GrBitmap*/HUDBitBlt (x, y, bmp, F1_0, 0);
	GrSetCurrentCanvas (GetCurrentGameScreen ());
	CopyGaugeBox (&gauge_boxes [boxofs+win], &gameStates.render.vr.buffers.render [0].cv_bitmap);
}

//	-----------------------------------------------------------------------------

void DrawWeaponBoxes ()
{
	int boxofs = (gameStates.render.cockpit.nMode==CM_STATUS_BAR)?SB_PRIMARY_BOX:COCKPIT_PRIMARY_BOX;
	int drew;

	if (weapon_box_user [0] == WBU_WEAPON) {
		drew = DrawWeaponBox (0, gameData.weapons.nPrimary);
		if (drew) 
			CopyGaugeBox (gauge_boxes+boxofs, &gameStates.render.vr.buffers.render [0].cv_bitmap);

		if (weapon_boxStates [0] == WS_SET) {
			if ((gameData.weapons.nPrimary == VULCAN_INDEX) || (gameData.weapons.nPrimary == GAUSS_INDEX)) {
				if (LOCALPLAYER.primaryAmmo [VULCAN_INDEX] != old_ammoCount [0][gameStates.render.vr.nCurrentPage]) {
					if (gameData.demo.nState == ND_STATE_RECORDING)
						NDRecordPrimaryAmmo (old_ammoCount [0][gameStates.render.vr.nCurrentPage], LOCALPLAYER.primaryAmmo [VULCAN_INDEX]);
					DrawPrimaryAmmoInfo (f2i ((unsigned) VULCAN_AMMO_SCALE * (unsigned) LOCALPLAYER.primaryAmmo [VULCAN_INDEX]));
					old_ammoCount [0][gameStates.render.vr.nCurrentPage] = LOCALPLAYER.primaryAmmo [VULCAN_INDEX];
				}
			}

			if (gameData.weapons.nPrimary == OMEGA_INDEX) {
				if (gameData.laser.xOmegaCharge != Old_Omega_charge [gameStates.render.vr.nCurrentPage]) {
					if (gameData.demo.nState == ND_STATE_RECORDING)
						NDRecordPrimaryAmmo (Old_Omega_charge [gameStates.render.vr.nCurrentPage], gameData.laser.xOmegaCharge);
					DrawPrimaryAmmoInfo (gameData.laser.xOmegaCharge * 100/MAX_OMEGA_CHARGE);
					Old_Omega_charge [gameStates.render.vr.nCurrentPage] = gameData.laser.xOmegaCharge;
				}
			}
		}
	}
	else if (weapon_box_user [0] == WBU_STATIC)
		DrawStatic (0);

	if (weapon_box_user [1] == WBU_WEAPON) {
		drew = DrawWeaponBox (1, gameData.weapons.nSecondary);
		if (drew)
			CopyGaugeBox (&gauge_boxes [boxofs+1], &gameStates.render.vr.buffers.render [0].cv_bitmap);

		if (weapon_boxStates [1] == WS_SET)
			if (LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary] != old_ammoCount [1][gameStates.render.vr.nCurrentPage]) {
				oldBombcount [gameStates.render.vr.nCurrentPage] = 0x7fff;	//force redraw
				if (gameData.demo.nState == ND_STATE_RECORDING)
					NDRecordSecondaryAmmo (old_ammoCount [1][gameStates.render.vr.nCurrentPage], LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary]);
				DrawSecondaryAmmoInfo (LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary]);
				old_ammoCount [1][gameStates.render.vr.nCurrentPage] = LOCALPLAYER.secondaryAmmo [gameData.weapons.nSecondary];
			}
	}
	else if (weapon_box_user [1] == WBU_STATIC)
		DrawStatic (1);
}

//	-----------------------------------------------------------------------------

void SBDrawEnergyBar (energy)
{
	int erase_height, w, h, aw;
	char energy_str [20];

//	GrSetCurrentCanvas (Canv_SBEnergyGauge);

	PAGE_IN_GAUGE (SB_GAUGE_ENERGY);
	if (gameStates.app.bD1Mission)
	HUDStretchBlt (SB_ENERGY_GAUGE_X, SB_ENERGY_GAUGE_Y, 
			         gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (SB_GAUGE_ENERGY), F1_0, 0, 
						1.0, (double) SB_ENERGY_GAUGE_H / (double) (SB_ENERGY_GAUGE_H - SB_AFTERBURNER_GAUGE_H));
	else
	HUDBitBlt (SB_ENERGY_GAUGE_X, SB_ENERGY_GAUGE_Y, 
									  gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (SB_GAUGE_ENERGY), F1_0, 0);
	erase_height = (100 - energy) * SB_ENERGY_GAUGE_H / 100;
	if (erase_height > 0) {
		GrSetColorRGBi (BLACK_RGBA);
		glDisable (GL_BLEND);
		HUDRect (
			SB_ENERGY_GAUGE_X, 
			SB_ENERGY_GAUGE_Y, 
			SB_ENERGY_GAUGE_X+SB_ENERGY_GAUGE_W, 
			SB_ENERGY_GAUGE_Y+erase_height);
		glEnable (GL_BLEND);
	}
GrSetCurrentCanvas (GetCurrentGameScreen ());
//draw numbers
sprintf (energy_str, "%d", energy);
GrGetStringSize (energy_str, &w, &h, &aw);
GrSetFontColorRGBi (RGBA_PAL2 (25, 18, 6), 1, 0, 0);
HUDPrintF (SB_ENERGY_GAUGE_X + ((SB_ENERGY_GAUGE_W - w)/2), SB_ENERGY_GAUGE_Y + SB_ENERGY_GAUGE_H - GAME_FONT->ft_h - (GAME_FONT->ft_h / 4), "%d", energy);
OglFreeBmTexture (&Canv_SBEnergyGauge->cv_bitmap);
}

//	-----------------------------------------------------------------------------

void SBDrawAfterburner ()
{
	int erase_height, w, h, aw;
	char ab_str [3] = "AB";
//	static int b = 1;

	if (gameStates.app.bD1Mission)
		return;
//	GrSetCurrentCanvas (Canv_SBAfterburnerGauge);
	PAGE_IN_GAUGE (SB_GAUGE_AFTERBURNER);
	HUDBitBlt (SB_AFTERBURNER_GAUGE_X, SB_AFTERBURNER_GAUGE_Y, gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (SB_GAUGE_AFTERBURNER), F1_0, 0);
	erase_height = FixMul ((f1_0 - xAfterburnerCharge), SB_AFTERBURNER_GAUGE_H);
//	HUDMessage (0, "AB: %d", erase_height);

	if (erase_height > 0) {
		GrSetColorRGBi (BLACK_RGBA);
		glDisable (GL_BLEND);
		HUDRect (
			SB_AFTERBURNER_GAUGE_X, 
			SB_AFTERBURNER_GAUGE_Y, 
			SB_AFTERBURNER_GAUGE_X+SB_AFTERBURNER_GAUGE_W-1, 
			SB_AFTERBURNER_GAUGE_Y+erase_height-1);
		glEnable (GL_BLEND);
	}
GrSetCurrentCanvas (GetCurrentGameScreen ());
//draw legend
if (LOCALPLAYER.flags & PLAYER_FLAGS_AFTERBURNER)
	GrSetFontColorRGBi (RGBA_PAL2 (45, 0, 0), 1, 0, 0);
else 
	GrSetFontColorRGBi (RGBA_PAL2 (12, 12, 12), 1, 0, 0);

GrGetStringSize (ab_str, &w, &h, &aw);
HUDPrintF (SB_AFTERBURNER_GAUGE_X + ((SB_AFTERBURNER_GAUGE_W - w)/2), SB_AFTERBURNER_GAUGE_Y+SB_AFTERBURNER_GAUGE_H-GAME_FONT->ft_h - (GAME_FONT->ft_h / 4), "AB");
OglFreeBmTexture (&Canv_SBAfterburnerGauge->cv_bitmap);
}

//	-----------------------------------------------------------------------------

void SBDrawShieldNum (int shield)
{
	//draw numbers

	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (RGBA_PAL2 (14, 14, 23), 1, 0, 0);

//erase old one
PIGGY_PAGE_IN (gameData.pig.tex.cockpitBmIndex [gameStates.render.cockpit.nMode+ (gameStates.video.nDisplayMode? (gameData.models.nCockpits/2):0)], 0);
HUDRect (SB_SHIELD_NUM_X, SB_SHIELD_NUM_Y, SB_SHIELD_NUM_X+ (gameStates.video.nDisplayMode?27:13), SB_SHIELD_NUM_Y+GAME_FONT->ft_h);
HUDPrintF ((shield>99)?SB_SHIELD_NUM_X: ((shield>9)?SB_SHIELD_NUM_X+2:SB_SHIELD_NUM_X+4), SB_SHIELD_NUM_Y, "%d", shield);
}

//	-----------------------------------------------------------------------------

void SBDrawShieldBar (int shield)
{
	int bm_num = shield>=100?9: (shield / 10);

GrSetCurrentCanvas (GetCurrentGameScreen ());
PAGE_IN_GAUGE (GAUGE_SHIELDS+9-bm_num);
HUDBitBlt (SB_SHIELD_GAUGE_X, SB_SHIELD_GAUGE_Y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (GAUGE_SHIELDS+9-bm_num)], F1_0, 0);
}

//	-----------------------------------------------------------------------------

void SBDrawKeys ()
{
	grsBitmap * bm;
	int flags = LOCALPLAYER.flags;

GrSetCurrentCanvas (GetCurrentGameScreen ());
bm = &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX ((flags&PLAYER_FLAGS_BLUE_KEY)?SB_GAUGE_BLUE_KEY:SB_GAUGE_BLUE_KEY_OFF)];
PAGE_IN_GAUGE ((flags&PLAYER_FLAGS_BLUE_KEY)?SB_GAUGE_BLUE_KEY:SB_GAUGE_BLUE_KEY_OFF);
HUDBitBlt (SB_GAUGE_KEYS_X, SB_GAUGE_BLUE_KEY_Y, bm, F1_0, 0);
bm = &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX ((flags&PLAYER_FLAGS_GOLD_KEY)?SB_GAUGE_GOLD_KEY:SB_GAUGE_GOLD_KEY_OFF)];
PAGE_IN_GAUGE ((flags&PLAYER_FLAGS_GOLD_KEY)?SB_GAUGE_GOLD_KEY:SB_GAUGE_GOLD_KEY_OFF);
HUDBitBlt (SB_GAUGE_KEYS_X, SB_GAUGE_GOLD_KEY_Y, bm, F1_0, 0);
bm = &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX ((flags&PLAYER_FLAGS_RED_KEY)?SB_GAUGE_RED_KEY:SB_GAUGE_RED_KEY_OFF)];
PAGE_IN_GAUGE ((flags&PLAYER_FLAGS_RED_KEY)?SB_GAUGE_RED_KEY:SB_GAUGE_RED_KEY_OFF);
HUDBitBlt (SB_GAUGE_KEYS_X, SB_GAUGE_RED_KEY_Y, bm , F1_0, 0);
}

//	-----------------------------------------------------------------------------

//	Draws invulnerable ship, or maybe the flashing ship, depending on invulnerability time left.
void DrawInvulnerableShip ()
{
	static fix time=0;

GrSetCurrentCanvas (GetCurrentGameScreen ());
if (((LOCALPLAYER.invulnerableTime + INVULNERABLE_TIME_MAX - gameData.time.xGame) > F1_0*4) || (gameData.time.xGame & 0x8000)) {
	if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)	{
		PAGE_IN_GAUGE (GAUGE_INVULNERABLE+invulnerable_frame);
		HUDBitBlt (SB_SHIELD_GAUGE_X, SB_SHIELD_GAUGE_Y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (GAUGE_INVULNERABLE+invulnerable_frame)], F1_0, 0);
		} 
	else {
		PAGE_IN_GAUGE (GAUGE_INVULNERABLE+invulnerable_frame);
		HUDBitBlt (SHIELD_GAUGE_X, SHIELD_GAUGE_Y, &gameData.pig.tex.bitmaps [0][GET_GAUGE_INDEX (GAUGE_INVULNERABLE+invulnerable_frame)], F1_0, 0);
		}
	time += gameData.time.xFrame;
	while (time > INV_FRAME_TIME) {
		time -= INV_FRAME_TIME;
		if (++invulnerable_frame == N_INVULNERABLE_FRAMES)
			invulnerable_frame=0;
		}
	}
else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
	SBDrawShieldBar (f2ir (LOCALPLAYER.shields));
else
	DrawShieldBar (f2ir (LOCALPLAYER.shields));
}

//	-----------------------------------------------------------------------------

extern int nMissileGun;
extern int AllowedToFireLaser (void);
extern int AllowedToFireMissile (void);

rgb playerColors [] = {
	{15, 15, 23}, 
	{27, 0, 0}, 
	{0, 23, 0}, 
	{30, 11, 31}, 
	{31, 16, 0}, 
	{24, 17, 6}, 
	{14, 21, 12}, 
	{29, 29, 0}};

extern int max_window_w;

typedef struct {
	sbyte x, y;
} xy;

//offsets for reticle parts: high-big  high-sml  low-big  low-sml
xy cross_offsets [4] = 		{{-8, -5}, 	{-4, -2}, 	{-4, -2}, {-2, -1}};
xy primary_offsets [4] = 	{{-30, 14}, {-16, 6}, 	{-15, 6}, {-8, 2}};
xy secondary_offsets [4] =	{{-24, 2}, 	{-12, 0}, {-12, 1}, {-6, -2}};

void OglDrawMouseIndicator (void);

//draw the reticle
void ShowReticle (int force_big_one)
{
	int x, y;
	int bLaserReady, bMissileReady, bLaserAmmo, bMissileAmmo;
	int nCrossBm, nPrimaryBm, nSecondaryBm;
	int bHiresReticle, bSmallReticle, ofs, nGaugeIndex;

if ((gameData.demo.nState==ND_STATE_PLAYBACK) && gameData.demo.bFlyingGuided) {
	DrawGuidedCrosshair ();
	return;
	}

x = grdCurCanv->cv_w / 2;
y = grdCurCanv->cv_h / ((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? 2 : 2);
bLaserReady = AllowedToFireLaser ();
bMissileReady = AllowedToFireMissile ();
bLaserAmmo = PlayerHasWeapon (gameData.weapons.nPrimary, 0, -1);
bMissileAmmo = PlayerHasWeapon (gameData.weapons.nSecondary, 1, -1);
nPrimaryBm = (bLaserReady && bLaserAmmo==HAS_ALL);
nSecondaryBm = (bMissileReady && bMissileAmmo==HAS_ALL);
if (nPrimaryBm && (gameData.weapons.nPrimary==LASER_INDEX) && 
		(LOCALPLAYER.flags & PLAYER_FLAGS_QUAD_LASERS))
	nPrimaryBm++;

if (secondaryWeaponToGunNum [gameData.weapons.nSecondary]==7)
	nSecondaryBm += 3;		//now value is 0, 1 or 3, 4
else if (nSecondaryBm && !(nMissileGun&1))
		nSecondaryBm++;

nCrossBm = ((nPrimaryBm > 0) || (nSecondaryBm > 0));

Assert (nPrimaryBm <= 2);
Assert (nSecondaryBm <= 4);
Assert (nCrossBm <= 1);
#ifdef _DEBUG
if (gameStates.render.bExternalView)
#else	
if (gameStates.render.bExternalView && (!IsMultiGame || EGI_FLAG (bEnableCheats, 0, 0, 0)))
#endif	
	return;
cmScaleX *= HUD_ASPECT;
if (gameStates.ogl.nReticle==2 || (gameStates.ogl.nReticle && grdCurCanv->cv_bitmap.bm_props.w > 320)){               
   OglDrawReticle (nCrossBm, nPrimaryBm, nSecondaryBm);
  } 
else {
	bHiresReticle = (gameStates.render.fonts.bHires != 0);
	bSmallReticle = !(grdCurCanv->cv_bitmap.bm_props.w * 3 > max_window_w*2 || force_big_one);
	ofs = (bHiresReticle ? 0 : 2) + bSmallReticle;

	nGaugeIndex = (bSmallReticle ? SML_RETICLE_CROSS : RETICLE_CROSS) + nCrossBm;
	PAGE_IN_GAUGE (nGaugeIndex);
	HUDBitBlt (
		- (x + HUD_SCALE_X (cross_offsets [ofs].x)), 
		- (y + HUD_SCALE_Y (cross_offsets [ofs].y)), 
		gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (nGaugeIndex), F1_0, 0);
	nGaugeIndex = (bSmallReticle ? SML_RETICLE_PRIMARY : RETICLE_PRIMARY) + nPrimaryBm;
	PAGE_IN_GAUGE (nGaugeIndex);
	HUDBitBlt (
		- (x + HUD_SCALE_X (primary_offsets [ofs].x)), 
		- (y + HUD_SCALE_Y (primary_offsets [ofs].y)), 
		gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (nGaugeIndex), F1_0, 0);
	nGaugeIndex = (bSmallReticle ? SML_RETICLE_SECONDARY : RETICLE_SECONDARY) + nSecondaryBm;
	PAGE_IN_GAUGE (nGaugeIndex);
	HUDBitBlt (
		- (x + HUD_SCALE_X (secondary_offsets [ofs].x)), 
		- (y + HUD_SCALE_Y (secondary_offsets [ofs].y)), 
		gameData.pig.tex.bitmaps [0] + GET_GAUGE_INDEX (nGaugeIndex), F1_0, 0);
  }
if (!gameStates.app.bNostalgia && gameOpts->input.bJoyMouse && gameOpts->render.cockpit.bMouseIndicator)
	OglDrawMouseIndicator ();
cmScaleX /= HUD_ASPECT;
}

//	-----------------------------------------------------------------------------

void HUDShowKillList ()
{
	int n_players, player_list [MAX_NUM_NET_PLAYERS];
	int n_left, i, xo = 0, x0, x1, y, save_y, fth, l;
	int t = gameStates.app.nSDLTicks;
	int bGetPing = gameStates.render.cockpit.bShowPingStats && (!networkData.tLastPingStat || (t - networkData.tLastPingStat >= 1000));
	static int faw = -1;

if (!SHOW_HUD)
	return;
// ugly hack since placement of netgame players and kills is based off of
// menuhires (which is always 1 for mac).  This throws off placement of
// players in pixel double mode.
if (bGetPing)
	networkData.tLastPingStat = t;
if (gameData.multigame.kills.xShowListTimer > 0) {
	gameData.multigame.kills.xShowListTimer -= gameData.time.xFrame;
	if (gameData.multigame.kills.xShowListTimer < 0)
		gameData.multigame.kills.bShowList = 0;
	}
GrSetCurFont (GAME_FONT);
n_players = (gameData.multigame.kills.bShowList == 3) ? 2 : MultiGetKillList (player_list);
n_left = (n_players <= 4) ? n_players : (n_players+1)/2;
//If font size changes, this code might not work right anymore 
//Assert (GAME_FONT->ft_h==5 && GAME_FONT->ft_w==7);
fth = GAME_FONT->ft_h;
x0 = LHX (1); 
x1 = (gameData.app.nGameMode & GM_MULTI_COOP) ? LHX (31) : LHX (43);
save_y = y = grdCurCanv->cv_h - n_left* (fth+1);
if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) {
	save_y = y -= LHX (6);
	x1 = (gameData.app.nGameMode & GM_MULTI_COOP) ? LHX (33) : LHX (43);
	}
if (gameStates.render.cockpit.bShowPingStats) {
	if (faw < 0)
		faw = get_fontTotal_width (GAME_FONT) / (GAME_FONT->ft_maxchar - GAME_FONT->ft_minchar + 1);
		if (gameData.multigame.kills.bShowList == 2)
			xo = faw*24;//was +25;
		else if (gameData.app.nGameMode & GM_MULTI_COOP)
			xo = faw*14;//was +30;
		else
			xo = faw*8; //was +20;
	}	
for (i = 0; i < n_players; i++) {
	int nPlayer;
	char name [80], teamInd [2] = {127, 0};
	int sw, sh, aw, indent =0;

	if (i>=n_left) {
		x0 = grdCurCanv->cv_w - ((gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) ? LHX (53) : LHX (60));
		x1 = grdCurCanv->cv_w - ((gameData.app.nGameMode & GM_MULTI_COOP) ? LHX (27) : LHX (15));
		if (gameStates.render.cockpit.bShowPingStats) {
			x0 -= xo + 6 * faw;
			x1 -= xo + 6 * faw;
			}
		if (i==n_left)
			y = save_y;
		if (netGame.KillGoal || netGame.xPlayTimeAllowed) 
			x1-=LHX (18);
		}
	else if (netGame.KillGoal || netGame.xPlayTimeAllowed) 
		 x1 = LHX (43) - LHX (18);
	nPlayer = (gameData.multigame.kills.bShowList == 3) ? i : player_list [i];
	if ((gameData.multigame.kills.bShowList == 1) || (gameData.multigame.kills.bShowList == 2)) {
		int color;

		if (gameData.multiplayer.players [nPlayer].connected != 1)
			GrSetFontColorRGBi (RGBA_PAL2 (12, 12, 12), 1, 0, 0);
		else {
			if (gameData.app.nGameMode & GM_TEAM)
				color = GetTeam (nPlayer);
			else
				color = nPlayer;
			GrSetFontColorRGBi (RGBA_PAL2 (playerColors [color].r, playerColors [color].g, playerColors [color].b), 1, 0, 0);
			}
		}	
	else 
		GrSetFontColorRGBi (RGBA_PAL2 (playerColors [nPlayer].r, playerColors [nPlayer].g, playerColors [nPlayer].b), 1, 0, 0);
	if (gameData.multigame.kills.bShowList == 3) {
		if (GetTeam (gameData.multiplayer.nLocalPlayer) == i) {
#if 0//def _DEBUG
			sprintf (name, "%c%-8s %d.%d.%d.%d:%d", 
						teamInd [0], netGame.team_name [i], 
						netPlayers.players [i].network.ipx.node [0], 
						netPlayers.players [i].network.ipx.node [1], 
						netPlayers.players [i].network.ipx.node [2], 
						netPlayers.players [i].network.ipx.node [3], 
						netPlayers.players [i].network.ipx.node [5] +
						 (unsigned) netPlayers.players [i].network.ipx.node [4] * 256);
#else
			sprintf (name, "%c%s", teamInd [0], netGame.team_name [i]);
#endif
			indent = 0;
			}
		else {
#if SHOW_PLAYER_IP
			sprintf (name, "%-8s %d.%d.%d.%d:%d", 
						netGame.team_name [i], 
						netPlayers.players [i].network.ipx.node [0], 
						netPlayers.players [i].network.ipx.node [1], 
						netPlayers.players [i].network.ipx.node [2], 
						netPlayers.players [i].network.ipx.node [3], 
						netPlayers.players [i].network.ipx.node [5] +
						 (unsigned) netPlayers.players [i].network.ipx.node [4] * 256);
#else
			strcpy (name, netGame.team_name [i]);
#endif
			GrGetStringSize (teamInd, &indent, &sh, &aw);
			}
		}
	else
#if 0//def _DEBUG
		sprintf (name, "%-8s %d.%d.%d.%d:%d", 
					gameData.multiplayer.players [nPlayer].callsign, 
					netPlayers.players [nPlayer].network.ipx.node [0], 
					netPlayers.players [nPlayer].network.ipx.node [1], 
					netPlayers.players [nPlayer].network.ipx.node [2], 
					netPlayers.players [nPlayer].network.ipx.node [3], 
					netPlayers.players [nPlayer].network.ipx.node [5] +
					 (unsigned) netPlayers.players [nPlayer].network.ipx.node [4] * 256);
#else
		strcpy (name, gameData.multiplayer.players [nPlayer].callsign);	// Note link to above if!!
#endif
#if 0//def _DEBUG
	x1 += LHX (100);
#endif
	for (l = (int) strlen (name); l;) {
		GrGetStringSize (name, &sw, &sh, &aw);
		if (sw <= x1 - x0 - LHX (2))
			break;
		name [--l] = '\0';
		}
	GrPrintF (x0+indent, y, "%s", name);

	if (gameData.multigame.kills.bShowList==2) {
		if (gameData.multiplayer.players [nPlayer].netKilledTotal + gameData.multiplayer.players [nPlayer].netKillsTotal <= 0)
			GrPrintF (x1, y, TXT_NOT_AVAIL);
		else
			GrPrintF (x1, y, "%d%%", 
				 (int) ((double) gameData.multiplayer.players [nPlayer].netKillsTotal /
						 ((double) gameData.multiplayer.players [nPlayer].netKilledTotal +
						  (double) gameData.multiplayer.players [nPlayer].netKillsTotal) * 100.0));		
		}
	else if (gameData.multigame.kills.bShowList == 3) {
		if (gameData.app.nGameMode & GM_ENTROPY)
			GrPrintF (x1, y, "%3d [%d/%d]", 
						 gameData.multigame.kills.nTeam [i], gameData.entropy.nTeamRooms [i + 1], 
						 gameData.entropy.nTotalRooms);
		else
			GrPrintF (x1, y, "%3d", gameData.multigame.kills.nTeam [i]);
		}
	else if (gameData.app.nGameMode & GM_MULTI_COOP)
		GrPrintF (x1, y, "%-6d", gameData.multiplayer.players [nPlayer].score);
   else if (netGame.xPlayTimeAllowed || netGame.KillGoal)
      GrPrintF (x1, y, "%3d (%d)", 
					 gameData.multiplayer.players [nPlayer].netKillsTotal, 
					 gameData.multiplayer.players [nPlayer].nKillGoalCount);
   else
		GrPrintF (x1, y, "%3d", gameData.multiplayer.players [nPlayer].netKillsTotal);
	if (gameStates.render.cockpit.bShowPingStats && (nPlayer != gameData.multiplayer.nLocalPlayer)) {
		if (bGetPing)
			PingPlayer (nPlayer);
		if (pingStats [nPlayer].sent) {
#if 0//def _DEBUG
			GrPrintF (x1 + xo, y, "%lu %d %d", 
						  pingStats [nPlayer].ping, 
						  pingStats [nPlayer].sent, 
						  pingStats [nPlayer].received);
#else
			GrPrintF (x1 + xo, y, "%lu %i%%", pingStats [nPlayer].ping, 
						 100 - ((pingStats [nPlayer].received * 100) / pingStats [nPlayer].sent));
#endif
			}
		}
	y += fth+1;
	}
}

//	-----------------------------------------------------------------------------

#ifdef _DEBUG
extern int Saving_movie_frames;
#else
#define Saving_movie_frames 0
#endif

//returns true if viewer can see tObject
//	-----------------------------------------------------------------------------

int CanSeeObject (int nObject, int bCheckObjs)
{
	tVFIQuery fq;
	int nHitType;
	tFVIData hit_data;

	//see if we can see this tPlayer

fq.p0 = &gameData.objs.viewer->position.vPos;
fq.p1 = &gameData.objs.objects [nObject].position.vPos;
fq.radP0 = 
fq.radP1 = 0;
fq.thisObjNum = gameStates.render.cameras.bActive ? -1 : OBJ_IDX (gameData.objs.viewer);
fq.flags = bCheckObjs ? FQ_CHECK_OBJS | FQ_TRANSWALL : FQ_TRANSWALL;
fq.startSeg = gameData.objs.viewer->nSegment;
fq.ignoreObjList = NULL;
nHitType = FindVectorIntersection (&fq, &hit_data);
return bCheckObjs ? (nHitType == HIT_OBJECT) && (hit_data.hit.nObject == nObject) : (nHitType != HIT_WALL);
}

//	-----------------------------------------------------------------------------

//show names of teammates & players carrying flags
void ShowHUDNames ()
{
	int bHasFlag, bShowName, bShowTeamNames, bShowAllNames, bShowFlags, nObject, nTeam;
	int p;
	rgb *colorP;
	static int nCurColor = 1, tColorChange = 0;
	static rgb typingColors [2] = {
		{63, 0, 0}, 
		{63, 63, 0}
	};
	char s [CALLSIGN_LEN+10];
	int w, h, aw;
	int x1, y1;
	int nColor;

bShowAllNames = ((gameData.demo.nState == ND_STATE_PLAYBACK) || 
						 (netGame.ShowAllNames && gameData.multigame.bShowReticleName));
bShowTeamNames = (gameData.multigame.bShowReticleName &&
						 ((gameData.app.nGameMode & GM_MULTI_COOP) || (gameData.app.nGameMode & GM_TEAM)));
bShowFlags = (gameData.app.nGameMode & (GM_CAPTURE | GM_HOARD | GM_ENTROPY));

nTeam = GetTeam (gameData.multiplayer.nLocalPlayer);
for (p = 0; p < gameData.multiplayer.nPlayers; p++) {	//check all players

	bShowName = (gameStates.multi.bPlayerIsTyping [p] || (bShowAllNames && !(gameData.multiplayer.players [p].flags & PLAYER_FLAGS_CLOAKED)) || (bShowTeamNames && GetTeam (p)==nTeam));
	bHasFlag = (gameData.multiplayer.players [p].connected && gameData.multiplayer.players [p].flags & PLAYER_FLAGS_FLAG);

	if (gameData.demo.nState != ND_STATE_PLAYBACK) 
		nObject = gameData.multiplayer.players [p].nObject;
	else {
		//if this is a demo, the nObject in the tPlayer struct is wrong, 
		//so we search the tObject list for the nObject
		for (nObject = 0;nObject <= gameData.objs.nLastObject; nObject++)
			if (gameData.objs.objects [nObject].nType==OBJ_PLAYER && 
				 gameData.objs.objects [nObject].id == p)
				break;
		if (nObject > gameData.objs.nLastObject)		//not in list, thus not visible
			bShowName = !bHasFlag;				//..so don't show name
		}

	if ((bShowName || bHasFlag) && CanSeeObject (nObject, 1)) {
		g3sPoint vPlayerPos;
		G3TransformAndEncodePoint (&vPlayerPos, &gameData.objs.objects [nObject].position.vPos);
		if (vPlayerPos.p3_codes == 0) {	//on screen
			G3ProjectPoint (&vPlayerPos);
			if (!(vPlayerPos.p3Flags & PF_OVERFLOW)) {
				fix x = vPlayerPos.p3_sx;
				fix y = vPlayerPos.p3_sy;
				if (bShowName) {				// Draw callsign on HUD
					if (gameStates.multi.bPlayerIsTyping [p]) {
						int t = gameStates.app.nSDLTicks;

						if (t - tColorChange > 333) {
							tColorChange = t;
							nCurColor = !nCurColor;
							}
						colorP = typingColors + nCurColor;
						}
					else {
						nColor = (gameData.app.nGameMode & GM_TEAM)? GetTeam (p) : p;
						colorP = playerColors + nColor;
						}

					sprintf (s, "%s", gameStates.multi.bPlayerIsTyping [p] ? TXT_TYPING : gameData.multiplayer.players [p].callsign);
					GrGetStringSize (s, &w, &h, &aw);
					GrSetFontColorRGBi (RGBA_PAL2 (colorP->r, colorP->g, colorP->b), 1, 0, 0);
					x1 = f2i (x)-w/2;
					y1 = f2i (y)-h/2;
					GrString (x1, y1, s);
				}
	
				if (bHasFlag && (gameStates.app.bNostalgia || !(EGI_FLAG (bTargetIndicators, 0, 1, 0) || EGI_FLAG (bTowFlags, 0, 1, 0)))) {// Draw box on HUD
					fix dy = -FixMulDiv (gameData.objs.objects [nObject].size, i2f (grdCurCanv->cv_h)/2, vPlayerPos.p3_z);
//					fix dy = -FixMulDiv (FixMul (gameData.objs.objects [nObject].size, viewInfo.scale.y), i2f (grdCurCanv->cv_h)/2, vPlayerPos.p3_z);
					fix dx = FixMul (dy, grdCurScreen->sc_aspect);
					fix w = dx/4;
					fix h = dy/4;
					if (gameData.app.nGameMode & (GM_CAPTURE | GM_ENTROPY))
						GrSetColorRGBi ((GetTeam (p) == TEAM_BLUE) ? MEDRED_RGBA :  MEDBLUE_RGBA);
					else if (gameData.app.nGameMode & GM_HOARD) {
						if (gameData.app.nGameMode & GM_TEAM)
							GrSetColorRGBi ((GetTeam (p) == TEAM_RED) ? MEDRED_RGBA :  MEDBLUE_RGBA);
						else
							GrSetColorRGBi (MEDGREEN_RGBA);
						}
					GrLine (x+dx-w, y-dy, x+dx, y-dy);
					GrLine (x+dx, y-dy, x+dx, y-dy+h);
					GrLine (x-dx, y-dy, x-dx+w, y-dy);
					GrLine (x-dx, y-dy, x-dx, y-dy+h);
					GrLine (x+dx-w, y+dy, x+dx, y+dy);
					GrLine (x+dx, y+dy, x+dx, y+dy-h);
					GrLine (x-dx, y+dy, x-dx+w, y+dy);
					GrLine (x-dx, y+dy, x-dx, y+dy-h);
					}
				}
			}
		}
	}
}

//	-----------------------------------------------------------------------------

//draw all the things on the HUD
void DrawHUD ()
{

if (!SHOW_HUD)
	return;
if (gameStates.render.cockpit.nMode==CM_STATUS_BAR) {
	gameStates.render.cockpit.nLastDrawn [0] =
	gameStates.render.cockpit.nLastDrawn [1] = -1;
	InitGauges ();
//	vr_reset_display ();
  }
cmScaleX = (grdCurScreen->sc_w <= 640) ? 1 : (double) grdCurScreen->sc_w / 640.0;
cmScaleY = (grdCurScreen->sc_h <= 480) ? 1 : (double) grdCurScreen->sc_h / 480.0;
nLineSpacing = GAME_FONT->ft_h + GAME_FONT->ft_h/4;

	//	Show score so long as not in rearview
if (!(gameStates.render.bRearView || Saving_movie_frames || (gameStates.render.cockpit.nMode == CM_REAR_VIEW) || (gameStates.render.cockpit.nMode == CM_STATUS_BAR))) {
	HUDShowScore ();
	if (scoreTime)
		HUDShowScoreAdded ();
	}

if (!(gameStates.render.bRearView || Saving_movie_frames || (gameStates.render.cockpit.nMode == CM_REAR_VIEW)))
	HUDShowTimerCount ();

//	Show other stuff if not in rearview or letterbox.
if (!(gameStates.render.bRearView || (gameStates.render.cockpit.nMode == CM_REAR_VIEW))) {// && gameStates.render.cockpit.nMode!=CM_LETTERBOX) {
	if (gameStates.render.cockpit.nMode==CM_STATUS_BAR || gameStates.render.cockpit.nMode==CM_FULL_SCREEN)
		HUDShowHomingWarning ();

	if ((gameStates.render.cockpit.nMode==CM_FULL_SCREEN) || (gameStates.render.cockpit.nMode==CM_LETTERBOX)) {
		if (!gameOpts->render.cockpit.bTextGauges) {
			if (gameOpts->render.cockpit.bScaleGauges) {
				xScale = (double) grdCurCanv->cv_h / 480.0;
				yScale = (double) grdCurCanv->cv_h / 640.0;
				}
			else
				xScale = yScale = 1;
			}
		HUDShowEnergy ();
		HUDShowShield ();
		HUDShowAfterburner ();
		HUDShowWeapons ();
		if (!Saving_movie_frames)
			HUDShowKeys ();
		HUDShowCloakInvul ();

		if ((gameData.demo.nState==ND_STATE_RECORDING) && (LOCALPLAYER.flags != oldFlags [gameStates.render.vr.nCurrentPage])) {
			NDRecordPlayerFlags (oldFlags [gameStates.render.vr.nCurrentPage], LOCALPLAYER.flags);
			oldFlags [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.flags;
			}
		}
#ifdef _DEBUG
	if (!(gameData.app.nGameMode&GM_MULTI && gameData.multigame.kills.bShowList) && !Saving_movie_frames)
		ShowTime ();
#endif
	if (gameOpts->render.cockpit.bReticle && !gameStates.app.bPlayerIsDead /*&& gameStates.render.cockpit.nMode != CM_LETTERBOX*/ && (!bUsePlayerHeadAngles))
		ShowReticle (0);

	ShowHUDNames ();
	if (gameStates.render.cockpit.nMode != CM_REAR_VIEW) {
		HUDShowFlag ();
		HUDShowOrbs ();
		}
	if (!Saving_movie_frames)
		HUDRenderMessageFrame ();

	if (gameStates.render.cockpit.nMode!=CM_STATUS_BAR && !Saving_movie_frames)
		HUDShowLives ();

	if (gameData.app.nGameMode&GM_MULTI && gameData.multigame.kills.bShowList)
		HUDShowKillList ();
#if 0
	DrawWeaponBoxes ();
#endif
	return;
	}

if (gameStates.render.bRearView && gameStates.render.cockpit.nMode!=CM_REAR_VIEW) {
	HUDRenderMessageFrame ();
	GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	if (gameData.demo.nState == ND_STATE_PLAYBACK)
		GrPrintF (0x8000, grdCurCanv->cv_h-14, TXT_REAR_VIEW);
	else
		GrPrintF (0x8000, grdCurCanv->cv_h-10, TXT_REAR_VIEW);
	}
}

//	-----------------------------------------------------------------------------

//print out some tPlayer statistics
void RenderGauges ()
{
	static int old_display_mode = 0;
	int energy = f2ir (LOCALPLAYER.energy);
	int shields = f2ir (LOCALPLAYER.shields);
	int cloak = ((LOCALPLAYER.flags&PLAYER_FLAGS_CLOAKED) != 0);
	int frc=0;

if (!SHOW_HUD)
	return;

cmScaleX = (grdCurScreen->sc_w <= 640) ? 1 : (double) grdCurScreen->sc_w / 640.0;
cmScaleY = (grdCurScreen->sc_h <= 480) ? 1 : (double) grdCurScreen->sc_h / 480.0;

Assert (gameStates.render.cockpit.nMode==CM_FULL_COCKPIT || gameStates.render.cockpit.nMode==CM_STATUS_BAR);
// check to see if our display mode has changed since last render time --
// if so, then we need to make new gauge canvases.
if (old_display_mode != gameStates.video.nDisplayMode) {
	CloseGaugeCanvases ();
	InitGaugeCanvases ();
	old_display_mode = gameStates.video.nDisplayMode;
	}
if (shields < 0) 
	shields = 0;
GrSetCurrentCanvas (GetCurrentGameScreen ());
GrSetCurFont (GAME_FONT);
if (gameData.demo.nState == ND_STATE_RECORDING)
	if (LOCALPLAYER.homingObjectDist >= 0)
		NDRecordHomingDistance (LOCALPLAYER.homingObjectDist);

DrawWeaponBoxes ();
if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT) {
	if (energy != old_energy [gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState==ND_STATE_RECORDING) {
			NDRecordPlayerEnergy (old_energy [gameStates.render.vr.nCurrentPage], energy);
			}
		}
	DrawEnergyBar (energy);
	DrawNumericalDisplay (shields, energy);
	old_energy [gameStates.render.vr.nCurrentPage] = energy;

	if (xAfterburnerCharge != old_afterburner [gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState==ND_STATE_RECORDING) {
			NDRecordPlayerAfterburner (old_afterburner [gameStates.render.vr.nCurrentPage], xAfterburnerCharge);
			}
		}
	DrawAfterburnerBar (xAfterburnerCharge);
	old_afterburner [gameStates.render.vr.nCurrentPage] = xAfterburnerCharge;

	if (LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) {
		DrawNumericalDisplay (shields, energy);
		DrawInvulnerableShip ();
		old_shields [gameStates.render.vr.nCurrentPage] = shields ^ 1;
		}
	else {
		if (shields != old_shields [gameStates.render.vr.nCurrentPage]) {		// Draw the shield gauge
			if (gameData.demo.nState==ND_STATE_RECORDING) {
				NDRecordPlayerShields (old_shields [gameStates.render.vr.nCurrentPage], shields);
				}
			}
		DrawShieldBar (shields);
		DrawNumericalDisplay (shields, energy);
		old_shields [gameStates.render.vr.nCurrentPage] = shields;
		}

	if (LOCALPLAYER.flags != oldFlags [gameStates.render.vr.nCurrentPage]) {
		if (gameData.demo.nState==ND_STATE_RECORDING)
			NDRecordPlayerFlags (oldFlags [gameStates.render.vr.nCurrentPage], LOCALPLAYER.flags);
		oldFlags [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.flags;
		}
	DrawKeys ();
	ShowHomingWarning ();
	ShowBombCount (BOMB_COUNT_X, BOMB_COUNT_Y, BLACK_RGBA, gameStates.render.cockpit.nMode == CM_FULL_COCKPIT);
	}
else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR) {
	if (energy != old_energy [gameStates.render.vr.nCurrentPage] || frc)  {
		if (gameData.demo.nState==ND_STATE_RECORDING) {
			NDRecordPlayerEnergy (old_energy [gameStates.render.vr.nCurrentPage], energy);
			}
		SBDrawEnergyBar (energy);
		old_energy [gameStates.render.vr.nCurrentPage] = energy;
		}
	if (xAfterburnerCharge != old_afterburner [gameStates.render.vr.nCurrentPage] || frc) {
		if (gameData.demo.nState==ND_STATE_RECORDING) {
			NDRecordPlayerAfterburner (old_afterburner [gameStates.render.vr.nCurrentPage], xAfterburnerCharge);
			}
		SBDrawAfterburner ();
		old_afterburner [gameStates.render.vr.nCurrentPage] = xAfterburnerCharge;
		}
	if (LOCALPLAYER.flags & PLAYER_FLAGS_INVULNERABLE) {
		DrawInvulnerableShip ();
		old_shields [gameStates.render.vr.nCurrentPage] = shields ^ 1;
		SBDrawShieldNum (shields);
		} 
	else {
		if (shields != old_shields [gameStates.render.vr.nCurrentPage] || frc) {		// Draw the shield gauge
			if (gameData.demo.nState==ND_STATE_RECORDING) {
				NDRecordPlayerShields (old_shields [gameStates.render.vr.nCurrentPage], shields);
				}
			SBDrawShieldBar (shields);
			old_shields [gameStates.render.vr.nCurrentPage] = shields;
			SBDrawShieldNum (shields);
			}
		}
	if (LOCALPLAYER.flags != oldFlags [gameStates.render.vr.nCurrentPage] || frc) {
		if (gameData.demo.nState==ND_STATE_RECORDING)
			NDRecordPlayerFlags (oldFlags [gameStates.render.vr.nCurrentPage], LOCALPLAYER.flags);
		SBDrawKeys ();
		oldFlags [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.flags;
		}
	if (IsMultiGame && !IsCoopGame) {
		if (LOCALPLAYER.netKilledTotal != old_lives [gameStates.render.vr.nCurrentPage] || frc) {
			SBShowLives ();
			old_lives [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.netKilledTotal;
			}
		}
	else {
		if (LOCALPLAYER.lives != old_lives [gameStates.render.vr.nCurrentPage] || frc) {
			SBShowLives ();
			old_lives [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.lives;
			}
		}
	if ((gameData.app.nGameMode&GM_MULTI) && !(gameData.app.nGameMode & GM_MULTI_COOP)) {
		if (LOCALPLAYER.netKillsTotal != old_score [gameStates.render.vr.nCurrentPage] || frc) {
			SBShowScore ();
			old_score [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.netKillsTotal;
			}
		}
	else {
		if (LOCALPLAYER.score != old_score [gameStates.render.vr.nCurrentPage] || frc) {
			SBShowScore ();
			old_score [gameStates.render.vr.nCurrentPage] = LOCALPLAYER.score;
			}
		//if (scoreTime)
		SBShowScoreAdded ();
		}
	ShowBombCount (SB_BOMB_COUNT_X, SB_BOMB_COUNT_Y, BLACK_RGBA, 0);
	}
if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT)
	DrawPlayerShip (cloak, old_cloak [gameStates.render.vr.nCurrentPage], SHIP_GAUGE_X, SHIP_GAUGE_Y);
else if (frc || (cloak != old_cloak [gameStates.render.vr.nCurrentPage]) || nCloakFadeState || 
	 (cloak && gameData.time.xGame>LOCALPLAYER.cloakTime+CLOAK_TIME_MAX-i2f (3))) {
	DrawPlayerShip (cloak, old_cloak [gameStates.render.vr.nCurrentPage], SB_SHIP_GAUGE_X, SB_SHIP_GAUGE_Y);
	old_cloak [gameStates.render.vr.nCurrentPage]=cloak;
	}
}

//	---------------------------------------------------------------------------------------------------------
//	Call when picked up a laser powerup.
//	If laser is active, set old_weapon [0] to -1 to force redraw.
void UpdateLaserWeaponInfo (void)
{
	if (old_weapon [0][gameStates.render.vr.nCurrentPage] == 0)
		if (!(LOCALPLAYER.laserLevel > MAX_LASER_LEVEL && old_laserLevel [gameStates.render.vr.nCurrentPage] <= MAX_LASER_LEVEL))
			old_weapon [0][gameStates.render.vr.nCurrentPage] = -1;
}

extern int Game_window_y;
void FillBackground (void);

int SW_drawn [2], SW_x [2], SW_y [2], SW_w [2], SW_h [2];

//	---------------------------------------------------------------------------------------------------------
//draws a 3d view into one of the cockpit windows.  win is 0 for left, 
//1 for right.  viewer is tObject.  NULL tObject means give up window
//user is one of the WBU_ constants.  If rearViewFlag is set, show a
//rear view.  If label is non-NULL, print the label at the top of the
//window.

int cockpitWindowScale [4] = {6, 5, 4, 3};

void DoCockpitWindowView (int win, tObject *viewer, int rearViewFlag, int user, char *label)
{
	grs_canvas window_canv;
	static grs_canvas overlap_canv;

	tObject *viewer_save = gameData.objs.viewer;
	grs_canvas *save_canv = grdCurCanv;
	static int overlap_dirty [2]={0, 0};
	int boxnum;
	static int window_x, window_y;
	gauge_box *box;
	int rearView_save = gameStates.render.bRearView;
	int w, h, dx;
	fix nZoomSave;

if (!SHOW_HUD)
	return;
box = NULL;
if (!viewer) {								//this user is done
	Assert (user == WBU_WEAPON || user == WBU_STATIC);
	if (user == WBU_STATIC && weapon_box_user [win] != WBU_STATIC)
		staticTime [win] = 0;
	if (weapon_box_user [win] == WBU_WEAPON || weapon_box_user [win] == WBU_STATIC)
		return;		//already set
	weapon_box_user [win] = user;
	if (overlap_dirty [win]) {
		GrSetCurrentCanvas (&gameStates.render.vr.buffers.screenPages [gameStates.render.vr.nCurrentPage]);
		FillBackground ();
		overlap_dirty [win] = 0;
		}
	return;
	}
UpdateRenderedData (win+1, viewer, rearViewFlag, user);
weapon_box_user [win] = user;						//say who's using window
gameData.objs.viewer = viewer;
gameStates.render.bRearView = rearViewFlag;

if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN)	{
	w = (int) (gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.w / cockpitWindowScale [gameOpts->render.cockpit.nWindowSize] * HUD_ASPECT);			// hmm.  I could probably do the sub_buffer assigment for all macines, but I aint gonna chance it
	h = i2f (w) / grdCurScreen->sc_aspect;
	dx = (win==0)?- (w+ (w/10)): (w/10);
	switch (gameOpts->render.cockpit.nWindowPos) {
		case 0:
			window_x = win ? 
				gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.w-w-h/10 :
				h / 10;
			window_y = gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.h-h- (h/10);
			break;
		case 1:
			window_x = win ? 
				gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.w/3*2-w/3 :
				gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.w/3-2*w/3;
			window_y = gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.h-h- (h/10);
			break;
		case 2:	// only makes sense if there's only one cockpit window
			window_x = gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.w/2-w/2;
			window_y = gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.h-h- (h/10);
			break;
		case 3:
			window_x = win ? 
				gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.w-w-h/10 :
				h / 10;
			window_y = h/10;
			break;
		case 4:
			window_x = win ? 
				gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.w/3*2-w/3 :
				gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.w/3-2*w/3;
			window_y = h/10;
			break;
		case 5:	// only makes sense if there's only one cockpit window
			window_x = gameStates.render.vr.buffers.render [0].cv_bitmap.bm_props.w/2-w/2;
			window_y = h/10;
			break;
		}
	if ((gameOpts->render.cockpit.nWindowPos < 3) && 
			extraGameInfo [0].nWeaponIcons &&
			(extraGameInfo [0].nWeaponIcons - gameOpts->render.weaponIcons.bEquipment < 3))
			window_y -= (int) ((gameOpts->render.weaponIcons.bSmall ? 20.0 : 30.0) * (double) grdCurCanv->cv_h / 480.0);


	//copy these vars so stereo code can get at them
	SW_drawn [win]=1; 
	SW_x [win] = window_x; 
	SW_y [win] = window_y; 
	SW_w [win] = w; 
	SW_h [win] = h; 

	GrInitSubCanvas (&window_canv, &gameStates.render.vr.buffers.render [0], window_x, window_y, w, h);
	}
else {
	if (gameStates.render.cockpit.nMode == CM_FULL_COCKPIT)
		boxnum = (COCKPIT_PRIMARY_BOX)+win;
	else if (gameStates.render.cockpit.nMode == CM_STATUS_BAR)
		boxnum = (SB_PRIMARY_BOX)+win;
	else
		goto abort;
	box = gauge_boxes + boxnum;
		GrInitSubCanvas (
			&window_canv, gameStates.render.vr.buffers.render, 
			HUD_SCALE_X (box->left), 
			HUD_SCALE_Y (box->top), 
			HUD_SCALE_X (box->right-box->left+1), 
			HUD_SCALE_Y (box->bot-box->top+1));
	}

GrSetCurrentCanvas (&window_canv);
nZoomSave = gameStates.render.nZoomFactor;
gameStates.render.nZoomFactor = F1_0 * (gameOpts->render.cockpit.nWindowZoom + 1);					//the tPlayer's zoom factor
if ((user == WBU_RADAR_TOPDOWN) || (user == WBU_RADAR_HEADSUP)) {
	gameStates.render.bTopDownRadar = (user == WBU_RADAR_TOPDOWN);
	if (!(gameData.app.nGameMode & GM_MULTI) || (netGame.gameFlags & NETGAME_FLAG_SHOW_MAP))
		DoAutomap (0, 1);
	else
		RenderFrame (0, win+1);
	}
else
	RenderFrame (0, win+1);
gameStates.render.nZoomFactor = nZoomSave;					//the tPlayer's zoom factor
//	HACK!If guided missile, wake up robots as necessary.
if (viewer->nType == OBJ_WEAPON) {
	// -- Used to require to be GUIDED -- if (viewer->id == GUIDEDMSL_ID)
	WakeupRenderedObjects (viewer, win+1);
	}
if (label) {
		GrSetCurFont (GAME_FONT);
	GrSetFontColorRGBi (GREEN_RGBA, 1, 0, 0);
	GrPrintF (0x8000, 2, label);
	}
if (user == WBU_GUIDED) {
	DrawGuidedCrosshair ();
	}
if (gameStates.render.cockpit.nMode == CM_FULL_SCREEN) {
	int small_window_bottom, big_window_bottom, extra_part_h;
	
	GrSetColorRGBi (RGBA_PAL (0, 0, 32));
	GrUBox (0, 0, grdCurCanv->cv_bitmap.bm_props.w-1, grdCurCanv->cv_bitmap.bm_props.h);

	//if the window only partially overlaps the big 3d window, copy
	//the extra part to the visible screen
	big_window_bottom = Game_window_y + Game_window_h - 1;
	if (window_y > big_window_bottom) {
		//the small window is completely outside the big 3d window, so
		//copy it to the visible screen
		if (gameStates.render.vr.nScreenFlags & VRF_USE_PAGING)
			GrSetCurrentCanvas (&gameStates.render.vr.buffers.screenPages [!gameStates.render.vr.nCurrentPage]);
		else
			GrSetCurrentCanvas (GetCurrentGameScreen ());
		GrBitmap (window_x, window_y, &window_canv.cv_bitmap);
		overlap_dirty [win] = 1;
		}
	else {
		small_window_bottom = window_y + window_canv.cv_bitmap.bm_props.h - 1;
		extra_part_h = small_window_bottom - big_window_bottom;
		if (extra_part_h > 0) {
			GrInitSubCanvas (&overlap_canv, &window_canv, 0, window_canv.cv_bitmap.bm_props.h-extra_part_h, window_canv.cv_bitmap.bm_props.w, extra_part_h);
			if (gameStates.render.vr.nScreenFlags & VRF_USE_PAGING)
				GrSetCurrentCanvas (&gameStates.render.vr.buffers.screenPages [!gameStates.render.vr.nCurrentPage]);
			else
				GrSetCurrentCanvas (GetCurrentGameScreen ());
				GrBitmap (window_x, big_window_bottom+1, &overlap_canv.cv_bitmap);
			overlap_dirty [win] = 1;
			}
		}
	}
else {
	GrSetCurrentCanvas (GetCurrentGameScreen ());
	CopyGaugeBox (box, &gameStates.render.vr.buffers.render [0].cv_bitmap);
	}
//force redraw when done
old_weapon [win][gameStates.render.vr.nCurrentPage] = old_ammoCount [win][gameStates.render.vr.nCurrentPage] = -1;

abort:;

gameData.objs.viewer = viewer_save;
GrSetCurrentCanvas (save_canv);
gameStates.render.bRearView = rearView_save;
}

//	-----------------------------------------------------------------------------

