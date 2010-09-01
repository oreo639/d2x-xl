// Copyright (C) 1997 Bryan Aamot

#ifndef __global_h
#define __global_h

#include "types.h"
#include "segment.h"
#include "object.h"

extern int m_changesMade;
extern bool bExpertMode;
extern bool bExtBlkFmt;

extern MISSION_DATA	missionData;
//extern HGLOBAL texture_handle[MAX_TEXTURES_D2]; // MAX_TEXTURES_D2
; // MAX_TEXTURES_D2
extern int left,right,top,bottom;
extern LPLOGPALETTE MyLogPalette;
extern short d2_blastable_lights [];
extern short d2_switches [];

extern int bEnableDeltaShading;

extern char FileName[256];
extern char FilePath[256];
extern char SubFile[20];

// robot related globals
extern int preferences;
//extern CFlickeringLight *flickeringLights;
extern LIGHT_TIMER lightTimers [MAX_FLICKERING_LIGHTS];
extern LIGHT_STATUS lightStatus [MAX_SEGMENTS3][MAX_SIDES_PER_SEGMENT];
extern uint        N_robot_types;
//extern CRobotInfo   *robotInfo;
#if ALLOCATE_POLYOBJECT
extern uint        N_robot_joints;
extern uint        N_polygon_models;
extern uint        N_object_bitmaps;
extern JOINTPOS     *Robot_joints;
extern tPolyModel    *Polygon_models[MAX_POLYGON_MODELS];
extern uint       *Dying_modelnums;
extern uint       *Dead_modelnums;
extern ushort *ObjBitmaps;
extern ushort *ObjBitmapPtrs;
#endif

extern char skip[16];
extern int left;
extern int top;
extern int right;
extern int bottom;
extern int x_center;
extern int y_center;
extern int x_max;
extern int y_max;
extern double aspect_ratio;
extern int refresh_needed;
extern int splash;



extern int level_modified;
//extern byte texture_modified[1+MAX_TEXTURES_D2/8];
extern int disable_saves;
extern byte clipList [D2_NUM_OF_CLIPS];
extern ushort wall_flags[9];
extern long ScrnMemMax;
extern int ScrnWidth;
extern int ScrnHeight;
extern int m_fileType;
extern CLightDeltaIndex *lightDeltaIndices;
extern CLightDeltaValue *lightDeltaValues;
extern short num_static_lights;
extern char string [256];
extern short nSplineSeg1, nSplineSeg2, nSplineSide1, nSplineSide2;
extern char spline_error_message [];
extern CVertex splinePoints [MAX_SPLINES];
extern CVertex points [4];
extern short n_splines;
//extern short spline_active;
extern byte object_list [MAX_OBJECT_NUMBER];
extern byte contentsList [MAX_CONTAINS_NUMBER];
extern char objectSelection [MAX_OBJECT_TYPES];
extern char contentsSelection [MAX_OBJECT_TYPES];
extern byte robotClip [MAX_ROBOT_IDS_TOTAL];
extern byte powerupClip [MAX_POWERUP_IDS2];
extern char *ai_options [MAX_D2_AI_OPTIONS];
extern fix powerupSize [MAX_POWERUP_IDS2];
extern fix robotSize [MAX_ROBOT_IDS_TOTAL];
extern fix robot_shield [MAX_ROBOT_IDS_TOTAL];
extern TEXTURE_LIGHT d1_texture_light[NUM_LIGHTS_D1];
extern TEXTURE_LIGHT d2_texture_light[NUM_LIGHTS_D2];
extern long lightMap [MAX_TEXTURES_D2];
extern short add_segment_mode;
extern char dlc_version[10];
extern char testers_name[20];
extern short serial_number;
extern char message[256];
extern char m_startFolder [256];
extern fix grid;
extern char descentPath [2][256];
extern char missionPath [256];
extern char player_profile [20];
extern short nTrigger;
//extern short number_of_textures;
#if 0
extern short  show_lines;
extern short  show_lights;
extern short  show_walls;
extern short  show_special;
extern short  show_objects;
extern short  show_powerups;
#endif
//extern CUBE  current1;
//extern CUBE  current2;
//extern CUBE  *current;
#if 0
extern short select_mode;
#endif
extern short edit_mode;
extern tUVL   default_uvls[4];
extern char  *object_names[MAX_OBJECT_TYPES];
extern byte powerup_types[MAX_POWERUP_IDS2];
extern char  wall_type_names[6][10];
extern char  wall_flag_names[8][14];
extern char  wall_state_names[4][8];
extern APOINT *scrn; //MAX_VERTICES
extern short gx0, gy0, gz0;
extern double spinx,spiny,spinz;
extern double movex, movey, movez;
extern short max_x, min_x, max_y, min_y, max_z, min_z;
extern double sizex,sizey,sizez;
extern double angleRate;
extern double moveRate;
//extern double M[4][4];  /* 4x4 matrix used in coordinate transformation */
//extern double IM[4][4]; /* inverse matrix of M[4][4] */
extern double depthPerception;

extern byte sideVertTable[6][4];
extern byte oppSideTable[6];
extern byte oppSideVertTable[6][4];
extern byte lineVertTable[12][2];
extern byte sideLineTable[6][4];
extern byte connectPointTable[8][3];
extern char pointSideTable[8][3];
extern char pointCornerTable[8][3];

extern byte doorClipTable [D2_NUM_OF_CLIPS];

/* debug data */
extern uint debug_offset2;
extern uint debug_offset1;
/* External Prototypes */
extern void cleanup(short error);

extern short map_mask[4];

extern int renderXOffs;
extern int renderYOffs;

#endif //__global_h4