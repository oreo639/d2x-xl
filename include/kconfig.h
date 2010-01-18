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

#ifndef _KCONFIG_H
#define _KCONFIG_H

#include "config.h"
#include "gamestat.h"
#include "text.h"
#include "menu.h"

#define	D2X_KEYS		1

typedef struct control_info {
	fix pitchTime;
	fix verticalThrustTime;
	fix headingTime;
	fix sidewaysThrustTime;
	fix bankTime;
	fix forwardThrustTime;

	ubyte rearViewDownCount;
	ubyte rearViewDownState;

	ubyte firePrimaryDownCount;
	ubyte firePrimaryState;
	ubyte fireSecondaryState;
	ubyte fireSecondaryDownCount;
	ubyte fireFlareDownCount;

	ubyte dropBombDownCount;

	ubyte automapDownCount;
	ubyte automapState;

	//CAngleVector heading;
	//char oem_message [64];

	ubyte afterburnerState;
	ubyte cyclePrimaryCount;
	ubyte cycleSecondaryCount;
	ubyte headlightCount;
	ubyte toggleIconsCount;
	ubyte zoomDownCount;
	ubyte useCloakDownCount;
	ubyte useInvulDownCount;
	ubyte slowMotionCount;
	ubyte bulletTimeCount;
} __pack__ tControlInfo;

typedef struct ext_control_info {
	fix pitchTime;
	fix verticalThrustTime;
	fix headingTime;
	fix sidewaysThrustTime;
	fix bankTime;
	fix forwardThrustTime;

	ubyte rearViewDownCount;
	ubyte rearViewDownState;

	ubyte firePrimaryDownCount;
	ubyte firePrimaryState;
	ubyte fireSecondaryState;
	ubyte fireSecondaryDownCount;
	ubyte fireFlareDownCount;

	ubyte dropBombDownCount;

	ubyte automapDownCount;
	ubyte automapState;

	//CAngleVector heading;   	    // for version >=1.0
	//char oem_message [64];     	// for version >=1.0

	//CFixVector ship_pos           // for version >=2.0
	//CFixMatrix ship_orient        // for version >=2.0

	//ubyte cyclePrimaryCount     // for version >=3.0
	//ubyte cycleSecondaryCount   // for version >=3.0
	//ubyte afterburnerState       // for version >=3.0
	//ubyte headlightCount         // for version >=3.0

	// everything below this line is for version >=4.0

	//ubyte headlightState

	//int primaryWeaponFlags
	//int secondaryWeaponFlags
	//ubyte Primary_weapon_selected
	//ubyte Secondary_weapon_selected

	//CFixVector force_vector
	//CFixMatrix force_matrix
	//int joltinfo[3]
	//int x_vibrate_info[2]
	//int y_vibrate_info[2]

	//int x_vibrate_clear
	//int y_vibrate_clear

	//ubyte gameStatus;

	//ubyte keyboard[128];          // scan code array, not ascii
	//ubyte current_guidebot_command;

	//ubyte Reactor_blown

} ext_control_info;

typedef struct advanced_ext_control_info {
	fix pitchTime;
	fix verticalThrustTime;
	fix headingTime;
	fix sidewaysThrustTime;
	fix bankTime;
	fix forwardThrustTime;

	ubyte rearViewDownCount;
	ubyte rearViewDownState;

	ubyte firePrimaryDownCount;
	ubyte firePrimaryState;
	ubyte fireSecondaryState;
	ubyte fireSecondaryDownCount;
	ubyte fireFlareDownCount;

	ubyte dropBombDownCount;

	ubyte automapDownCount;
	ubyte automapState;

	// everything below this line is for version >=1.0

	CAngleVector heading;
	char oem_message [64];

	// everything below this line is for version >=2.0

	CFixVector ship_pos;
	CFixMatrix ship_orient;

	// everything below this line is for version >=3.0

	ubyte cyclePrimaryCount;
	ubyte cycleSecondaryCount;
	ubyte afterburnerState;
	ubyte headlightCount;

	// everything below this line is for version >=4.0

	int primaryWeaponFlags;
	int secondaryWeaponFlags;
	ubyte currentPrimary_weapon;
	ubyte currentSecondary_weapon;

	CFixVector force_vector;
	CFixMatrix force_matrix;
	int joltinfo[3];
	int x_vibrate_info[2];
	int y_vibrate_info[2];

	int x_vibrate_clear;
	int y_vibrate_clear;

	ubyte gameStatus;

	ubyte headlightState;
	ubyte current_guidebot_command;

	ubyte keyboard[128];    // scan code array, not ascii

	ubyte Reactor_blown;

} advanced_ext_control_info;


class CExternalControls {
	public:
		ubyte					m_bUse;
		ubyte					m_bEnable;
		ubyte					m_intno;
		ext_control_info*	m_info;
		char*					m_name;
		ubyte					m_version;

	public:
		CExternalControls () { memset (this, 0, sizeof (*this)); }
		~CExternalControls () { Destroy (); }
		void Init (int intno, int address);
		void Destroy (void);
		void Read (void);
};

extern CExternalControls externalControls;


typedef struct kcItem {
	short id;				// The id of this item
	short x, y;
	short w1;
	short w2;
	short u,d,l,r;
        //short text_num1;
   const char 	*text;
	int   textId;
	ubyte nType;
	ubyte value;		// what key,button,etc
} kcItem;


// added on 2/4/99 by Victor Rachels to add new keys menu
#define NUM_HOTKEY_CONTROLS	KcHotkeySize () //20
#define MAX_HOTKEY_CONTROLS	40

#define NUM_KEY_CONTROLS		KcKeyboardSize () //64
#define NUM_JOY_CONTROLS		KcJoystickSize () //66
#define NUM_SUPERJOY_CONTROLS	KcSuperJoySize () //66
#define NUM_MOUSE_CONTROLS		KcMouseSize () //31
#define MAX_CONTROLS				64		// there are actually 48, so this leaves room for more

typedef struct tControlSettings {
	ubyte		custom [CONTROL_MAX_TYPES][MAX_CONTROLS];
	ubyte		defaults [CONTROL_MAX_TYPES][MAX_CONTROLS];
	ubyte		d2xCustom [MAX_HOTKEY_CONTROLS];
	ubyte		d2xDefaults [MAX_HOTKEY_CONTROLS];
} __pack__ tControlSettings;

extern tControlSettings controlSettings;

extern char *control_text[CONTROL_MAX_TYPES];

void KCSetControls (int bGet);

// Tries to use vfx1 head tracking.
void kconfig_sense_init();

//set the cruise speed to zero

extern int kconfig_is_axes_used(int axis);

extern ubyte nExtGameStatus;
void KConfig(int n, const char *pszTitle);
void SetControlType (void);

extern ubyte system_keys [];

extern tControlSettings controlSettings;
extern kcItem kcKeyboard []; //NUM_KEY_CONTROLS];
extern ubyte kc_kbdFlags []; //NUM_KEY_CONTROLS];
extern kcItem kcJoystick []; //NUM_JOY_CONTROLS];
extern kcItem kcMouse [];	//NUM_MOUSE_CONTROLS];
extern kcItem kcHotkeys [];	//NUM_HOTKEY_CONTROLS];
extern kcItem kcSuperJoy []; //NUM_JOY_CONTROLS];

#define BT_NONE				-1
#define BT_KEY 				0
#define BT_MOUSE_BUTTON 	1
#define BT_MOUSE_AXIS		2
#define BT_JOY_BUTTON 		3
#define BT_JOY_AXIS			4
#define BT_INVERT				5

int KcKeyboardSize (void);
int KcMouseSize (void);
int KcJoystickSize (void);
int KcSuperJoySize (void);
int KcHotkeySize (void);

//------------------------------------------------------------------------------

class CControlConfig {
	public:
		void Run (int nType, const char* pszTitle);

	private:
		typedef ubyte kc_ctrlfuncType (void);
		typedef kc_ctrlfuncType *kc_ctrlfunc_ptr;

		typedef struct tItemPos {
			int	i, l, r, y;
			} tItemPos;

	private:
		kcItem*		m_items;
		int			m_nItems;
		int			m_nCurItem, m_nPrevItem;
		int			m_nChangeMode, m_nPrevChangeMode;
		int			m_nMouseState, m_nPrevMouseState;
		int			m_bTimeStopped;
		int			m_bRedraw;
		int			m_xOffs, m_yOffs;
		int			m_closeX, m_closeY, m_closeSize;
		int			m_startAxis [JOY_MAX_AXES];
		const char*	m_pszTitle;

	private:
		void Edit (kcItem* items, int nItems);
		int HandleControl (void);
		int HandleInput (void);
		void Quit (void);


		const char* MouseButtonText (int i);
		const char* MouseAxisText (int i);
		const char* YesNoText (int i);

		int IsAxisUsed (int axis);
		int GetItemHeight (kcItem *item);
		void DrawTitle (void);
		void DrawHeader (void);
		void DrawTable (void);
		void DrawScreen (void);
		void DrawQuestion (kcItem *item);
		void DrawItem (kcItem* item, int bIsCurrent, int bRedraw);
		inline void DrawItem (kcItem *item, int bIsCurrent) { DrawItem (item, bIsCurrent, MODERN_STYLE); }

		void ReadFCS (int raw_axis);

		int AssignControl (kcItem *item, int nType, ubyte code);
		ubyte KeyCtrlFunc (void);
		ubyte JoyBtnCtrlFunc (void);
		ubyte MouseBtnCtrlFunc (void);
		ubyte JoyAxisCtrlFunc (void);
		ubyte MouseAxisCtrlFunc (void);
		int ChangeControl (kcItem *item, int nType, kc_ctrlfunc_ptr ctrlfunc, const char *pszMsg);

		inline int ChangeKey (kcItem *item) { return ChangeControl (item, BT_KEY, &CControlConfig::KeyCtrlFunc, TXT_PRESS_NEW_KEY); }
		inline int ChangeJoyButton (kcItem *item) { return ChangeControl (item, BT_JOY_BUTTON, &CControlConfig::JoyBtnCtrlFunc, TXT_PRESS_NEW_JBUTTON); }
		inline int ChangeMouseButton (kcItem * item) { return ChangeControl (item, BT_MOUSE_BUTTON, &CControlConfig::MouseBtnCtrlFunc, TXT_PRESS_NEW_MBUTTON); }
		inline int ChangeJoyAxis (kcItem *item) { return ChangeControl (item, BT_JOY_AXIS, &CControlConfig::JoyAxisCtrlFunc, TXT_MOVE_NEW_JOY_AXIS); }
		inline int ChangeMouseAxis (kcItem * item) { return ChangeControl (item, BT_MOUSE_AXIS, &CControlConfig::MouseAxisCtrlFunc, TXT_MOVE_NEW_MSE_AXIS); }
		int ChangeInvert (kcItem * item);

		void QSortItemPos (tItemPos* pos, int left, int right);
		tItemPos* GetItemPos (kcItem* items, int nItems);
		int* GetItemRef (int nItems, tItemPos* pos);
		int FindItemAt (int x, int y);
		int FindNextItemLeft (int nItems, int nCurItem, tItemPos *pos, int *ref);
		int FindNextItemRight (int nItems, int nCurItem, tItemPos *pos, int *ref);
		int FindNextItemUp (int nItems, int nCurItem, tItemPos *pos, int *ref);
		int FindNextItemDown (int nItems, int nCurItem, tItemPos *pos, int *ref);
		void LinkKbdEntries (void);
		void LinkJoyEntries (void);
		void LinkMouseEntries (void);
		void LinkHotkeyEntries (void);
		void LinkTableEntries (int tableFlags);
};

//------------------------------------------------------------------------------


#endif /* _KCONFIG_H */
