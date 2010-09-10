//***************************************************************************
//
//  NAME:    GLOBAL.CPP
//  
//	PURPOSE:
//	
//	  BY:      Bryan Aamot
//	  
//		REV:
//		
//***************************************************************************

#include "stdafx.h"

#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"

MISSION_DATA missionData;
//HGLOBAL texture_handle [MAX_TEXTURES_D2]; // MAX_TEXTURES_D2
CPalette *m_currentPalette;
LPLOGPALETTE MyLogPalette;

char FileName[256]="";
char FilePath[256]="";
char SubFile[20]="";

bool bExpertMode = true;
bool bExtBlkFmt = false;

int preferences = 0;

//CVariableLight *variableLights=0;
//uint       N_robot_types = 0;
//CRobotInfo   *robotInfo;     // MAX_ROBOT_TYPES

#if ALLOCATE_POLYOBJECT
uint       N_robot_joints=0;
uint       N_polygon_models=0;
uint       N_object_bitmaps=0;
JOINTPOS     *Robot_joints;   // MAX_ROBOT_JOINTS
tPolyModel    *Polygon_models[MAX_POLYGON_MODELS];
uint       *Dying_modelnums;// N_POLYGON_MODELS_D2
uint       *Dead_modelnums; // N_POLYGON_MODELS_D2
ushort *ObjBitmaps;     // MAX_OBJ_BITMAPS
ushort *ObjBitmapPtrs;  // MAX_OBJ_BITMAPS
#endif

char skip[16] = "tomÅ";
int left;
int top;
int right;
int bottom;
int x_center;
int y_center;
int x_max;
int y_max;
double aspect_ratio;
int refresh_needed = 0;
int splash = 0;


int level_modified = 0;
//byte texture_modified[1+MAX_TEXTURES_D2/8];
int disable_saves = 1;

long ScrnMemMax = 0L;
int ScrnWidth = 0;
int ScrnHeight = 0;

// new data for Descent 2
int file_type = RL2_FILE;
short	     num_static_lights=0;

char		string [256];

byte object_list[MAX_OBJECT_NUMBER] = {
	OBJ_ROBOT,
	OBJ_HOSTAGE,
	OBJ_PLAYER,
	OBJ_WEAPON,
	OBJ_POWERUP,
	OBJ_CNTRLCEN,
	OBJ_COOP,
	OBJ_CAMBOT,
	OBJ_MONSTERBALL,
	OBJ_SMOKE,
	OBJ_EXPLOSION,
	OBJ_EFFECT
};

byte contentsList[MAX_CONTAINS_NUMBER] = {
	OBJ_ROBOT,
	OBJ_POWERUP
};

// the following array is used to select a list item by objP->Type ()
char objectSelection [MAX_OBJECT_TYPES] = {
	-1, -1, 0, 1, 2, 3, -1, 4, -1, 5, -1, -1, -1, -1, 6, -1, 7, 8, 9, 10, 11
};
char contentsSelection [MAX_OBJECT_TYPES] = {
	-1, -1, 0, -1, -1, -1, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

byte robotClip[MAX_ROBOT_IDS_TOTAL] = {
	0x00, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 
	0x0e, 0x10, 0x12, 0x13, 0x14, 0x15, 0x16, 0x18, 
	0x19, 0x1b, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 
	40, 41, 43, 44, 45, 46, 47, 49, 
	50, 51, 52, 53, 55, 56, 58, 60, 	// 50,  52,  53,  and 86 were guessed at but seem to work ok
	62, 64, 65, 67, 68, 69, 70, 71, 
	72, 73, 74, 75, 76, 77, 78, 80, 
	82, 83, 85, 86, 87, 88, 89, 90, 
	91, 92, 
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1  // vertigo clip numbers
};

// note1: 0 == not used,
// note2: 100 and 101 are flags but will appear as shields
//        in non multiplayer level
byte powerupClip [MAX_POWERUP_IDS_D2] = {
	36, 18, 19, 20, 24, 26, 25,  0,
	 0,  0, 34, 35, 51, 37, 38, 39,
	40, 41, 42, 43, 44, 45, 46, 47,
	 0, 49,  0,  0, 69, 70, 71, 72,
	77, 73, 74, 75, 76, 83, 78, 89,
	79, 90, 91, 81,102, 82,100,101
};

char *ai_options [MAX_AI_OPTIONS_D2] = {
	"Still", "Normal", "Get behind", "Drop Bombs", "Snipe", "Station", "Follow", "Static", "Smart Bombs"
};

int powerupSize [MAX_POWERUP_IDS_D2] = {
	0x28000, 0x30000, 0x28000, 0x40000, 0x30000, 0x30000, 0x30000, 0x30000, 
	0x30000, 0x30000, 0x28000, 0x30000, 0x30000, 0x40000, 0x40000, 0x40000, 
	0x40000, 0x30000, 0x28000, 0x30000, 0x28000, 0x30000, 0x1cccc, 0x20000, 
	0x30000, 0x29999, 0x30000, 0x30000, 0x40000, 0x40000, 0x40000, 0x40000, 
	0x40000, 0x40000, 0x40000, 0x40000, 0x48000, 0x30000, 0x28000, 0x28000, 
	0x30000, 0x30000, 0x40000, 0x40000, 0x40000, 0x40000, 0x38000, 0x38000
};


int robotSize [MAX_ROBOT_IDS_TOTAL] = {
	399147, 368925, 454202, 316909, 328097, 345407, 399147, 293412, 
	300998, 308541, 246493, 283415, 283415, 227232, 200000, 598958, 
	399147, 1597221, 290318, 345407, 323879, 339488, 294037, 1443273, 
	378417, 408340, 586422, 302295, 524232, 405281, 736493, 892216, 
	400000, 204718, 400000, 400000, 354534, 236192, 373267, 351215, 
	429512, 169251, 310419, 378181, 381597, 1101683, 853047, 423359, 
	402717, 289744, 187426, 361065, 994374, 758384, 429512, 408340, 
	289744, 408340, 289744, 400000, 402717, 169251, 1312272, 169251, 
	905234, 1014749, 
	374114, 318151, 377386, 492146, 257003, 403683,     // vertigo robots
	342424, 322628, 332831, 1217722, 907806, 378960    // vertigo robots
};

int robotShield [MAX_ROBOT_IDS_TOTAL] = {
	6553600, 6553600, 6553600, 1638400, 2293760, 6553600, 9830400, 16384000, 
	2293760, 16384000, 2293760, 2293760, 2000000, 9830400, 1310720, 26214400, 
	21299200, 131072000, 6553600, 3276800, 3276800, 4587520, 4587520, 327680000, 
	5570560, 5242880, 9830400, 2949120, 6553600, 6553600, 7864320, 196608000, 
	5000000, 45875200, 5000000, 5000000, 5242880, 786432, 1966080, 4587520, 
	9830400, 1310720, 29491200, 9830400, 11796480, 262144000, 262144000, 13107200, 
	7208960, 655360, 983040, 11141120, 294912000, 32768000, 7864320, 3932160, 
	4587520, 5242880, 4587520, 5000000, 7208960, 1310720, 196608000, 1310720, 
	294912000, 19660800, 
	6553600, 6553600, 6553600, 10485760, 4587520, 16384000,   // vertigo robots
	6553600, 7864320, 7864320, 180224000, 360448000, 9830400 // vertigo robots
};


short d2_switches [] = {414, 416, 418, -1};

short add_segment_mode = ORTHOGONAL;
char dlc_version[10] = "2.7a";
short serial_number   = 27;
char message [256];
char m_startFolder [256];
int grid = 0x2000L;   // 0.125
char descentPath [2][256] = {"\\programs\\d2\\data", "\\programs\\d2\\data"};
char missionPath [256]= "\\programs\\d2\\missions";
char player_profile [20] = "";

short nTrigger = 0;

tUVL defaultUVLs [4] = {
	{(ushort) 0x0000, (ushort)0x0000, (ushort)DEFAULT_LIGHTING},
	{(ushort) 0x0000, (ushort)0x0800, (ushort)DEFAULT_LIGHTING},
	{(ushort)-0x0800, (ushort)0x0800, (ushort)DEFAULT_LIGHTING},
	{(ushort)-0x0800, (ushort)0x0000, (ushort)DEFAULT_LIGHTING}
};

char *objectNameTable [MAX_OBJECT_TYPES] = {
	"Wall",
	"Fireball",
	"Robot",
	"Hostage",
	"Player",
	"Mine",
	"Camera",
	"Power Up",
	"Debris",
	"Reactor",
	"Flare",
	"Clutter",
	"Ghost",
	"Light",
	"CoOp",
	"Marker",
	"Camera",
	"Monsterball",
	"Smoke",
	"Explosion",
	"Effect"
};

byte powerupTypeTable [MAX_POWERUP_IDS_D2] = {
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_KEY_MASK,
		POWERUP_KEY_MASK,
		POWERUP_KEY_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_UNKNOWN_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_WEAPON_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK,
		POWERUP_POWERUP_MASK
};

char wall_type_names[6][10] = {
	"Normal",
		"Blastable",
		"Door",
		"Illusion",
		"Open",
		"Closed"
};

char wall_flag_names[8][14] = {
	"Blasted",
		"Door Closed",
		"(not used)",
		"Door Locked",
		"Door Auto",
		"Illusion Off",
		"(not used)",
		"(not used)"
};

char wall_state_names[4][8] = {
	"Closed",
		"Opening",
		"Open",
		"Closing"
};


APOINT *scrn;
short gx0,gy0,gz0;
double spinx,spiny,spinz;
double movex, movey, movez;
short max_x, min_x, max_y, min_y, max_z, min_z;
double sizex,sizey,sizez;
double angleRate = (double)PI / 16.f;
double moveRate = 1.0;
double depthPerception = 1000.0;

                        /*--------------------------*/

int renderXOffs = 0;
int renderYOffs = 0;


