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

char skip[16] = "tom≈";
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



short d2_switches [] = {414, 416, 418, -1};

short add_segment_mode = ORTHOGONAL;
char dlc_version[10] = "2.7a";
short serial_number   = 27;
char message [256];
char m_startFolder [256];
//int grid = 0x2000L;   // 0.125
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


