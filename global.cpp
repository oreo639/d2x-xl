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
LIGHT_TIMER lightTimers [MAX_VARIABLE_LIGHTS];
LIGHT_STATUS lightStatus [SEGMENT_LIMIT][MAX_SIDES_PER_SEGMENT];
uint       N_robot_types=0;
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
short		nSplineSeg1, nSplineSeg2, nSplineSide1, nSplineSide2;
char		spline_error_message []  =  "You must exit spline creation before preforming this function";
CVertex	splinePoints [MAX_SPLINES];
CVertex	points [4];
short		n_splines = 0;
short		spline_length1 = 20, spline_length2 = 20;
short		spline_active = 0;

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

// the following array is used to select a list item by objP->m_info.type
char objectSelection[MAX_OBJECT_TYPES] = {
	-1,-1,0,1,2,3,-1,4,-1,5,-1,-1,-1,-1,6,-1,7,8,9,10,11
};
char contentsSelection[MAX_OBJECT_TYPES] = {
	-1,-1,0,-1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

byte robotClip[MAX_ROBOT_IDS_TOTAL] = {
	0x00,0x02,0x03,0x05,0x07,0x09,0x0b,0x0d,
	0x0e,0x10,0x12,0x13,0x14,0x15,0x16,0x18,
	0x19,0x1b,0x1c,0x1e,0x20,0x22,0x24,0x26,
	40,41,43,44,45,46,47,49,
	50,51,52,53,55,56,58,60,	// 50, 52, 53, and 86 were guessed at but seem to work ok
	62,64,65,67,68,69,70,71,
	72,73,74,75,76,77,78,80,
	82,83,85,86,87,88,89,90,
	91,92,
	0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,0xb0,0xb1  // vertigo clip numbers
};

// note1: 0 == not used,
// note2: 100 and 101 are flags but will appear as shields
//        in non multiplayer level
byte powerupClip[MAX_POWERUP_IDS_D2] = {
	36, 18, 19, 20, 24, 26, 25,  0,
		0,  0, 34, 35, 51, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 46, 47,
		0, 49,  0,  0, 69, 70, 71, 72,
		77, 73, 74, 75, 76, 83, 78, 89,
		79, 90, 91, 81,102, 82,100,101
};

char *ai_options[MAX_AI_OPTIONS_D2] = {
	"Still", "Normal", "Get behind", "Drop Bombs", "Snipe", "Station", "Follow", "Static", "Smart Bombs"
};

int powerupSize[MAX_POWERUP_IDS_D2] = {
		0x28000L,0x30000L,0x28000L,0x40000L,0x30000L,0x30000L,0x30000L,0x30000L,
		0x30000L,0x30000L,0x28000L,0x30000L,0x30000L,0x40000L,0x40000L,0x40000L,
		0x40000L,0x30000L,0x28000L,0x30000L,0x28000L,0x30000L,0x1ccccL,0x20000L,
		0x30000L,0x29999L,0x30000L,0x30000L,0x40000L,0x40000L,0x40000L,0x40000L,
		0x40000L,0x40000L,0x40000L,0x40000L,0x48000L,0x30000L,0x28000L,0x28000L,
		0x30000L,0x30000L,0x40000L,0x40000L,0x40000L,0x40000L,0x38000L,0x38000L
};


int robotSize [MAX_ROBOT_IDS_TOTAL] = {
		399147L,368925L,454202L,316909L,328097L,345407L,399147L,293412L,
		300998L,308541L,246493L,283415L,283415L,227232L,200000L,598958L,
		399147L,1597221L,290318L,345407L,323879L,339488L,294037L,1443273L,
		378417L,408340L,586422L,302295L,524232L,405281L,736493L,892216L,
		400000L,204718L,400000L,400000L,354534L,236192L,373267L,351215L,
		429512L,169251L,310419L,378181L,381597L,1101683L,853047L,423359L,
		402717L,289744L,187426L,361065L,994374L,758384L,429512L,408340L,
		289744L,408340L,289744L,400000L,402717L,169251L,1312272L,169251L,
		905234L,1014749L,
		374114L,318151L,377386L,492146L,257003L,403683L,    // vertigo robots
		342424L,322628L,332831L,1217722L,907806L,378960L    // vertigo robots
};

int robot_shield [MAX_ROBOT_IDS_TOTAL] = {
	6553600L,6553600L,6553600L,1638400L,2293760L,6553600L,9830400L,16384000L,
		2293760L,16384000L,2293760L,2293760L,2000000L,9830400L,1310720L,26214400L,
		21299200L,131072000L,6553600L,3276800L,3276800L,4587520L,4587520L,327680000L,
		5570560L,5242880L,9830400L,2949120L,6553600L,6553600L,7864320L,196608000L,
		5000000L,45875200L,5000000L,5000000L,5242880L,786432L,1966080L,4587520L,
		9830400L,1310720L,29491200L,9830400L,11796480L,262144000L,262144000L,13107200L,
		7208960L,655360L,983040L,11141120L,294912000L,32768000L,7864320L,3932160L,
		4587520L,5242880L,4587520L,5000000L,7208960L,1310720L,196608000L,1310720L,
		294912000L,19660800L,
		6553600L,6553600L,6553600L,10485760L,4587520L,16384000L,  // vertigo robots
		6553600L,7864320L,7864320L,180224000L,360448000L,9830400L // vertigo robots
};


short d2_switches [] = {414, 416, 418, -1};

short add_segment_mode = ORTHOGONAL;
char dlc_version[10] = "2.7a";
short serial_number   = 27;
char message[256];
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


