// Copyright (c) 1998 Bryan Aamot, Brainware

#include "stdafx.h"
#include "dle-xp-res.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "io.h"
#include "textures.h"
#include "palette.h"
#include "dle-xp.h"
#include "robot.h"
#include "hogmanager.h"
#include "light.h"

#ifdef ALLOCATE_POLYMODELS
#undef ALLOCATE_POLYMODELS
#endif
#define ALLOCATE_POLYMODELS 0

#define ENABLE_TEXT_DUMP 0

CMine* theMine = NULL;

//==========================================================================
// CMine - CMine
//==========================================================================

#define CLEAR(_b)	memset (_b, 0, sizeof (_b))

CMine::CMine() 
{ 
Initialize();
}

void CMine::Initialize (void)
{
VertCount () = 0;
SegCount () = 0;
#if 0
Segments ().Clear ();
Vertices ().Clear ();
Walls ().Clear ();
Triggers ().Clear ();
ObjTriggers ().Clear ();
ReactorTriggers ().Clear ();
VertexColors ().Clear ();
BotGens ().Clear ();
EquipGens ().Clear ();
#else
CLEAR (Segments ());
CLEAR (Vertices ());
CLEAR (Walls ());
CLEAR (Triggers ());
CLEAR (ObjTriggers ());
CLEAR (ReactorTriggers ());
CLEAR (VertexColors ());
CLEAR (BotGens ());
CLEAR (EquipGens ());
#endif
m_levelVersion = 7;
m_fileType = RL2_FILE;
m_dlcLogPalette = 0;
m_currentPalette = NULL;
FlickerLightCount () = 0;
Current () = &Current1 ();
*m_szBlockFile = '\0';
GameInfo ().objects.Reset ();
GameInfo ().walls.Reset ();
GameInfo ().doors.Reset ();
GameInfo ().triggers.Reset ();
GameInfo ().control.Reset ();
GameInfo ().botgen.Reset ();
GameInfo ().equipgen.Reset ();
GameInfo ().lightDeltaIndices.Reset ();
GameInfo ().lightDeltaValues.Reset ();
m_nNoLightDeltas = 2;
m_lightRenderDepth = MAX_LIGHT_DEPTH;
m_deltaLightRenderDepth = MAX_LIGHT_DEPTH;
#if 0
RobotInfo ().Clear ();
#else
memset (RobotInfo (), 0, sizeof (RobotInfo ()));
#endif
//	LoadPalette ();
m_bSortObjects = TRUE;
m_bVertigo = false;
m_pHxmExtraData = NULL;
m_nHxmExtraDataSize = 0;
m_bUseTexColors = false;
LoadDefaultLightAndColor ();
Default ();
//	strcpy (descent2_path, "d:\\games\\descent\\d2\\");
}

//==========================================================================
// CMine - ~CMine
//==========================================================================

CMine::~CMine()
{
Default ();
}

//==========================================================================
// CMine - init_data()
//==========================================================================

void CMine::Reset (void)
{
Current1 ().nSegment = DEFAULT_SEGMENT;
Current1 ().nPoint = DEFAULT_POINT;
Current1 ().nLine = DEFAULT_LINE;
Current1 ().nSide = DEFAULT_SIDE;
Current1 ().nObject = DEFAULT_OBJECT;
Current2 ().nSegment = DEFAULT_SEGMENT;
Current2 ().nPoint = DEFAULT_POINT;
Current2 ().nLine = DEFAULT_LINE;
Current2 ().nSide = DEFAULT_SIDE;
Current2 ().nObject = DEFAULT_OBJECT;
theApp.ResetUndoBuffer ();
}

void CMine::ConvertWallNum (UINT16 wNumOld, UINT16 wNumNew)
{
CSegment *segP = Segments (0);
CSide *sideP;
INT32 i, j;

for (i = SegCount (); i; i--, segP++)
	for (j = 0, sideP = segP->sides; j < 6; j++, sideP++)
		if (sideP->nWall >= wNumOld)
			sideP->nWall = wNumNew;
}


// ------------------------------------------------------------------------
// Create()
//
// Action: makes a new level from the resource file
// ------------------------------------------------------------------------

INT16 CMine::CreateNewLevel (void)
{
HGLOBAL hGlobal;
UINT32 nResSize;
UINT8 *data = LoadDataResource (MAKEINTRESOURCE ((IsD1File ()) ? IDR_NEW_RDL : IDR_NEW_RL2), hGlobal, nResSize);
if (!data)
	return 0;
// copy data to a file

FSplit ((m_fileType== RDL_FILE) ? descent_path : levels_path, m_startFolder , NULL, NULL);
sprintf_s (message, sizeof (message),  (m_fileType== RDL_FILE) ? "%sNEW.RDL" : "%sNEW.RL2", m_startFolder );
#if 0
RobotInfo () = DefRobotInfo ();
#else
memcpy (RobotInfo (), DefRobotInfo (), sizeof (RobotInfo ()));
#endif
texture_resource = (IsD1File ()) ? D1_TEXTURE_STRING_TABLE : D2_TEXTURE_STRING_TABLE;
FILE *file;
fopen_s (&file, message, "wb");
if (file) {
	size_t nBytes = fwrite(data, sizeof (UINT8), (UINT16)nResSize, file);
	fclose (file);
	FreeResource (hGlobal);
	if (nBytes != nResSize)
		return 1;
	NumObjTriggers () = 0;
	return 0;
	} 
else {
	FreeResource (hGlobal);
	return 2;
	}
}


// ------------------------------------------------------------------------
// CMine::Default()
// ------------------------------------------------------------------------
void CMine::Default()
{
ClearMineData();

if (m_pHxmExtraData) {
	free (m_pHxmExtraData);
	m_pHxmExtraData = NULL;
	m_nHxmExtraDataSize = 0;
	}

CSegment& segP = *Segments (0);
CVertex *vert = Vertices (0);

segP.sides [0].nWall = NO_WALL;
segP.sides [0].nBaseTex = 0;
segP.sides [0].nOvlTex = 0;
segP.sides [0].uvls [0].u = 0;
segP.sides [0].uvls [0].v = 0;
segP.sides [0].uvls [0].l = 0x8000U;
segP.sides [0].uvls [1].u = 0;
segP.sides [0].uvls [1].v = 2047;
segP.sides [0].uvls [1].l = 511;
segP.sides [0].uvls [2].u = - 2048;
segP.sides [0].uvls [2].v = 2047;
segP.sides [0].uvls [2].l = 3833;
segP.sides [0].uvls [3].u = - 2048;
segP.sides [0].uvls [3].v = 0;
segP.sides [0].uvls [3].l = 0x8000U;

segP.sides [1].nWall = NO_WALL;
segP.sides [1].nBaseTex = 263;
segP.sides [1].nOvlTex = 264;
segP.sides [1].uvls [0].u = 0;
segP.sides [1].uvls [0].v = 0;
segP.sides [1].uvls [0].l = 0x8000U;
segP.sides [1].uvls [1].u = 0;
segP.sides [1].uvls [1].v = 2047;
segP.sides [1].uvls [1].l = 0x8000U;
segP.sides [1].uvls [2].u = - 2048;
segP.sides [1].uvls [2].v = 2047;
segP.sides [1].uvls [2].l = 0x8000U;
segP.sides [1].uvls [3].u = - 2048;
segP.sides [1].uvls [3].v = 0;
segP.sides [1].uvls [3].l = 0x8000U;

segP.sides [2].nWall = NO_WALL;
segP.sides [2].nBaseTex = 0;
segP.sides [2].nOvlTex = 0;
segP.sides [2].uvls [0].u = 0;
segP.sides [2].uvls [0].v = 0;
segP.sides [2].uvls [0].l = 0x8000U;
segP.sides [2].uvls [1].u = 0;
segP.sides [2].uvls [1].v = 2047;
segP.sides [2].uvls [1].l = 3836;
segP.sides [2].uvls [2].u = - 2048;
segP.sides [2].uvls [2].v = 2047;
segP.sides [2].uvls [2].l = 5126;
segP.sides [2].uvls [3].u = - 2048;
segP.sides [2].uvls [3].v = 0;
segP.sides [2].uvls [3].l = 0x8000U;

segP.sides [3].nWall = NO_WALL;
segP.sides [3].nBaseTex = 270;
segP.sides [3].nOvlTex = 0;
segP.sides [3].uvls [0].u = 0;
segP.sides [3].uvls [0].v = 0;
segP.sides [3].uvls [0].l = 11678;
segP.sides [3].uvls [1].u = 0;
segP.sides [3].uvls [1].v = 2047;
segP.sides [3].uvls [1].l = 12001;
segP.sides [3].uvls [2].u = - 2048;
segP.sides [3].uvls [2].v = 2047;
segP.sides [3].uvls [2].l = 12001;
segP.sides [3].uvls [3].u = - 2048;
segP.sides [3].uvls [3].v = 0;
segP.sides [3].uvls [3].l = 11678;

segP.sides [4].nWall = NO_WALL;
segP.sides [4].nBaseTex = 0;
segP.sides [4].nOvlTex = 0;
segP.sides [4].uvls [0].u = 0;
segP.sides [4].uvls [0].v = 0;
segP.sides [4].uvls [0].l = 0x8000U;
segP.sides [4].uvls [1].u = 0;
segP.sides [4].uvls [1].v = 2043;
segP.sides [4].uvls [1].l = 0x8000U;
segP.sides [4].uvls [2].u = - 2044;
segP.sides [4].uvls [2].v = 2045;
segP.sides [4].uvls [2].l = 0x8000U;
segP.sides [4].uvls [3].u = - 2043;
segP.sides [4].uvls [3].v = 0;
segP.sides [4].uvls [3].l = 0x8000U;

segP.sides [5].nWall = NO_WALL;
segP.sides [5].nBaseTex = 0;
segP.sides [5].nOvlTex = 0;
segP.sides [5].uvls [0].u = 0;
segP.sides [5].uvls [0].v = 0;
segP.sides [5].uvls [0].l = 24576;
segP.sides [5].uvls [1].u = 0;
segP.sides [5].uvls [1].v = 2048;
segP.sides [5].uvls [1].l = 24576;
segP.sides [5].uvls [2].u = - 2048;
segP.sides [5].uvls [2].v = 2048;
segP.sides [5].uvls [2].l = 24576;
segP.sides [5].uvls [3].u = - 2048;
segP.sides [5].uvls [3].v = 0;
segP.sides [5].uvls [3].l = 24576;

segP.children [0] =-1;
segP.children [1] =-1;
segP.children [2] =-1;
segP.children [3] =-1;
segP.children [4] =-1;
segP.children [5] =-1;

segP.verts [0] = 0;
segP.verts [1] = 1;
segP.verts [2] = 2;
segP.verts [3] = 3;
segP.verts [4] = 4;
segP.verts [5] = 5;
segP.verts [6] = 6;
segP.verts [7] = 7;

segP.function = 0;
segP.nMatCen =-1;
segP.value =-1;
segP.s2_flags = 0;
segP.static_light = 263152L;
segP.childFlags = 0;
segP.wallFlags = 0;
segP.nIndex = 0;
segP.map_bitmask = 0;

vert [0] = CVertex (10 * F1_0, 10 * F1_0, -10 * F1_0);
vert [0].v.x = 10*F1_0;
vert [0].v.y = 10*F1_0;
vert [0].v.z = - 10*F1_0;
vert [1].v.x = 10*F1_0;
vert [1].v.y = - 10*F1_0;
vert [1].v.z = - 10*F1_0;
vert [2].v.x = - 10*F1_0;
vert [2].v.y = - 10*F1_0;
vert [2].v.z = - 10*F1_0;
vert [3].v.x = - 10*F1_0;
vert [3].v.y = 10*F1_0;
vert [3].v.z = - 10*F1_0;
vert [4].v.x = 10*F1_0;
vert [4].v.y = 10*F1_0;
vert [4].v.z = 10*F1_0;
vert [5].v.x = 10*F1_0;
vert [5].v.y = - 10*F1_0;
vert [5].v.z = 10*F1_0;
vert [6].v.x = - 10*F1_0;
vert [6].v.y = - 10*F1_0;
vert [6].v.z = 10*F1_0;
vert [7].v.x = - 10*F1_0;
vert [7].v.y = 10*F1_0;
vert [7].v.z = 10*F1_0;

SegCount () = 1;
VertCount () = 8;
}

// ------------------------------------------------------------------------
// ClearMineData()
// ------------------------------------------------------------------------

void CMine::ClearMineData() 
{
	INT16 i;

// initialize Segments ()
CSegment *segP = Segments (0);
for (i = 0; i < MAX_SEGMENTS; i++, segP++)
	segP->wallFlags &= ~MARKED_MASK;
SegCount () = 0;
// initialize vertices
for (i = 0; i < MAX_VERTICES; i++) 
	VertStatus (i) &= ~MARKED_MASK;
VertCount () = 0;
FlickerLightCount () = 0;
// reset "howmany"
GameInfo ().objects.Reset ();
GameInfo ().walls.Reset ();
GameInfo ().doors.Reset ();
GameInfo ().triggers.Reset ();
GameInfo ().control.Reset ();
GameInfo ().botgen.Reset ();
GameInfo ().equipgen.Reset ();
GameInfo ().lightDeltaIndices.Reset ();
GameInfo ().lightDeltaValues.Reset ();
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

CFixVector CMine::CalcSideNormal (INT16 nSegment, INT16 nSide)
{
if (nSegment < 0)
	nSegment = Current ()->nSegment;
if (nSide < 0)
	nSegment = Current ()->nSide;

	INT16*			sideIndexP = Segments (nSegment)->verts;
	UINT8*			sideVertP = &side_vert [nSide][0];
	CDoubleVector	v;

v = Normal (CDoubleVector (CFixVector (*Vertices (sideIndexP [sideVertP [0]]))), 
			   CDoubleVector (CFixVector (*Vertices (sideIndexP [sideVertP [1]]))), 
			   CDoubleVector (CFixVector (*Vertices (sideIndexP [sideVertP [3]]))));
return CFixVector (v);
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

CFixVector CMine::CalcSideCenter (INT16 nSegment, INT16 nSide)
{
	INT16*		sideIndexP = Segments (nSegment)->verts;
	UINT8*		sideVertP = &side_vert [nSide][0];
	CFixVector	v;

for (INT32 i = 0; i < 4; i++)
	v += *Vertices (sideIndexP [sideVertP [i]]);
v /= 4;
return v;
}

// --------------------------------------------------------------------------
//eof mine.cpp