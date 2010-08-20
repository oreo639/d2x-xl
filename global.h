// Copyright (C) 1997 Bryan Aamot

#ifndef __global_h
#define __global_h

#include "types.h"
#include "segment.h"

extern INT32 m_changesMade;
extern bool bExpertMode;
extern bool bExtBlkFmt;

extern MISSION_DATA	missionData;
//extern HGLOBAL texture_handle[MAX_D2_TEXTURES]; // MAX_D2_TEXTURES
; // MAX_D2_TEXTURES
extern HINSTANCE hInst;
extern INT32 left,right,top,bottom;
extern LPLOGPALETTE MyLogPalette;
extern INT16 d2_blastable_lights [];
extern INT16 d2_switches [];

extern INT32 bEnableDeltaShading;

extern char FileName[256];
extern char FilePath[256];
extern char SubFile[20];

// robot related globals
extern INT32 preferences;
//extern CFlickeringLight *flickeringLights;
extern LIGHT_TIMER lightTimers [MAX_FLICKERING_LIGHTS];
extern LIGHT_STATUS lightStatus [MAX_SEGMENTS3][MAX_SIDES_PER_SEGMENT];
extern UINT32        N_robot_types;
//extern ROBOT_INFO   *robotInfo;
#if ALLOCATE_POLYOBJECT
extern UINT32        N_robot_joints;
extern UINT32        N_polygon_models;
extern UINT32        N_object_bitmaps;
extern JOINTPOS     *Robot_joints;
extern POLYMODEL    *Polygon_models[MAX_POLYGON_MODELS];
extern UINT32       *Dying_modelnums;
extern UINT32       *Dead_modelnums;
extern UINT16 *ObjBitmaps;
extern UINT16 *ObjBitmapPtrs;
#endif

extern char skip[16];
extern INT32 left;
extern INT32 top;
extern INT32 right;
extern INT32 bottom;
extern INT32 x_center;
extern INT32 y_center;
extern INT32 x_max;
extern INT32 y_max;
extern double aspect_ratio;
extern INT32 refresh_needed;
extern INT32 splash;



extern INT32 level_modified;
//extern UINT8 texture_modified[1+MAX_D2_TEXTURES/8];
extern INT32 disable_saves;
extern UINT8 clipList [D2_NUM_OF_CLIPS];
extern UINT16 wall_flags[9];
extern INT32 texture_resource;
extern long ScrnMemMax;
extern INT32 ScrnWidth;
extern INT32 ScrnHeight;
extern INT32 m_fileType;
extern CLightDeltaIndex *lightDeltaIndices;
extern CLightDeltaValue *CLightDeltaValues;
extern INT16 num_static_lights;
extern char string[256];
extern INT16 nSplineSeg1,nSplineSeg2,nSplineSide1,nSplineSide2;
extern char spline_error_message[];
extern CFixVector spline_points[MAX_SPLINES];
extern CFixVector points[4];
extern INT16 n_splines;
//extern INT16 spline_active;
extern UINT8 object_list[MAX_OBJECT_NUMBER];
extern UINT8 contains_list[MAX_CONTAINS_NUMBER];
extern INT8 object_selection[MAX_OBJECT_TYPES];
extern INT8 contains_selection[MAX_OBJECT_TYPES];
extern UINT8 robot_clip[MAX_ROBOT_IDS_TOTAL];
extern UINT8 powerup_clip[MAX_POWERUP_IDS2];
extern char *ai_options[MAX_D2_AI_OPTIONS];
extern FIX powerup_size[MAX_POWERUP_IDS2];
extern FIX robot_size [MAX_ROBOT_IDS_TOTAL];
extern FIX robot_shield [MAX_ROBOT_IDS_TOTAL];
extern TEXTURE_LIGHT d1_texture_light[NUM_LIGHTS_D1];
extern TEXTURE_LIGHT d2_texture_light[NUM_LIGHTS_D2];
extern long lightMap [MAX_D2_TEXTURES];
extern INT16 add_segment_mode;
extern char dlc_version[10];
extern char testers_name[20];
extern INT16 serial_number;
extern char message[256];
extern char m_startFolder [256];
extern FIX grid;
extern char descent_path[256];
extern char descent2_path[256];
extern char levels_path[256];
extern char player_profile [20];
extern INT16 nTrigger;
//extern INT16 number_of_textures;
#if 0
extern INT16  show_lines;
extern INT16  show_lights;
extern INT16  show_walls;
extern INT16  show_special;
extern INT16  show_objects;
extern INT16  show_powerups;
#endif
//extern CUBE  current1;
//extern CUBE  current2;
//extern CUBE  *current;
#if 0
extern INT16 select_mode;
#endif
extern INT16 edit_mode;
extern CUVL   default_uvls[4];
extern char  *object_names[MAX_OBJECT_TYPES];
extern UINT8 powerup_types[MAX_POWERUP_IDS2];
extern char  wall_type_names[6][10];
extern char  wall_flag_names[8][14];
extern char  wall_state_names[4][8];
extern APOINT *scrn; //MAX_VERTICES
extern INT16 gx0, gy0, gz0;
extern double spinx,spiny,spinz;
extern double movex, movey, movez;
extern INT16 max_x, min_x, max_y, min_y, max_z, min_z;
extern double sizex,sizey,sizez;
extern double angle_rate;
extern FIX   move_rate;
extern double M[4][4];  /* 4x4 matrix used in coordinate transformation */
extern double IM[4][4]; /* inverse matrix of M[4][4] */
extern double depth_perception;

extern UINT8 side_vert[6][4];
extern UINT8 opp_side[6];
extern UINT8 opp_side_vert[6][4];
extern UINT8 line_vert[12][2];
extern UINT8 side_line[6][4];
extern UINT8 connect_points[8][3];
extern INT8 point_sides[8][3];
extern INT8 point_corners[8][3];

extern UINT8 clip_door_number [D2_NUM_OF_CLIPS];

/* debug data */
extern UINT32 debug_offset2;
extern UINT32 debug_offset1;
/* External Prototypes */
extern void cleanup(INT16 error);

extern INT16 map_mask[4];

extern INT32 renderXOffs;
extern INT32 renderYOffs;

extern UINT8 *pCustomPalette;

#define	TXT_GRAY_ROCK			1
#define	TXT_BROWN_ROCK			(1L << 1)
#define	TXT_RED_ROCK			(1L << 2)
#define	TXT_GREEN_ROCK			(1L << 3)
#define	TXT_YELLOW_ROCK		(1L << 4)
#define	TXT_BLUE_ROCK			(1L << 5)
#define	TXT_ICE					(1L << 6)
#define	TXT_STONES				(1L << 7)
#define	TXT_GRASS				(1L << 8)
#define	TXT_SAND					(1L << 9)
#define	TXT_LAVA					(1L << 10)
#define	TXT_WATER				(1L << 11)
#define	TXT_STEEL				(1L << 12)
#define	TXT_CONCRETE			(1L << 13)
#define	TXT_BRICK				(1L << 14)
#define	TXT_TARMAC				(1L << 15)
#define	TXT_WALL					(1L << 16)
#define	TXT_FLOOR				(1L << 17)
#define	TXT_CEILING				(1L << 18)
#define	TXT_GRATE				(1L << 19)
#define	TXT_FAN					(1L << 20)
#define	TXT_LIGHT				(1L << 21)
#define	TXT_ENERGY				(1L << 22)
#define	TXT_FORCEFIELD			(1L << 23)
#define	TXT_SIGN					(1L << 24)
#define	TXT_SWITCH				(1L << 25)
#define	TXT_TECH					(1L << 26)
#define	TXT_DOOR					(1L << 27)
#define	TXT_LABEL				(1L << 28)
#define	TXT_MONITOR				(1L << 29)
#define	TXT_STRIPES				(1L << 30)
#define	TXT_MOVE					(1L << 31)

#define  TXT_ROCK			(TXT_GRAY_ROCK | TXT_BROWN_ROCK | TXT_RED_ROCK | TXT_YELLOW_ROCK | TXT_GREEN_ROCK | TXT_BLUE_ROCK)
#define  TXT_NATURE		(TXT_ICE | TXT_STONES | TXT_GRASS | TXT_SAND | TXT_LAVA | TXT_WATER)
#define	TXT_BUILDING	(TXT_STEEL | TXT_CONCRETE | TXT_BRICK | TXT_TARMAC | TXT_WALL | TXT_FLOOR | TXT_CEILING)
#define	TXT_OTHER		(TXT_FAN | TXT_GRATE | TXT_DOOR)
#define	TXT_TECHMAT		(TXT_SWITCH | TXT_TECH | TXT_ENERGY | TXT_FORCEFIELD | TXT_LIGHT)
#define	TXT_SIGNS		(TXT_SIGN | TXT_LABEL | TXT_MONITOR | TXT_STRIPES)

typedef struct tTxtRange {
	INT16	nMin, nMax;
} tTxtRange;

typedef struct tTxtFilter {
	tTxtRange	iTexture;
	UINT32		nFilter;
	UINT32		n2ndFilter;
} tTxtFilter;

#define D1_TXT_FILTER_SIZE	154
#define D2_TXT_FILTER_SIZE	174

extern tTxtFilter txtFiltersD2 [D2_TXT_FILTER_SIZE];
extern tTxtFilter txtFiltersD1 [D1_TXT_FILTER_SIZE];

#define TEX_FILTER_SIZE	(theApp.IsD1File () ? D1_TXT_FILTER_SIZE : D2_TXT_FILTER_SIZE)
#define TEXTURE_FILTERS (theApp.IsD1File () ? txtFiltersD1 : txtFiltersD2)

#endif //__global_h4