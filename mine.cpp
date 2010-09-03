// Copyright (c) 1998 Bryan Aamot, Brainware

#include "stdafx.h"
#include "dle-xp-res.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "cfile.h"
#include "textures.h"
#include "palette.h"
#include "dle-xp.h"
#include "robot.h"
#include "HogManager.h"
#include "palette.h"
#include "light.h"

#ifdef ALLOCATE_tPolyModelS
#undef ALLOCATE_tPolyModelS
#endif
#define ALLOCATE_tPolyModelS 0

#define ENABLE_TEXT_DUMP 0

CMine* theMine = null;

//==========================================================================
// CMine - CMine
//==========================================================================

CMine::CMine() 
{ 
//memset (this, 0, sizeof (*this));
Initialize ();
}

void CMine::Initialize (void)
{
VertCount () = 0;
SegCount () = 0;
#if 1
CLEAR (Segments ());
CLEAR (Objects ());
CLEAR (Vertices ());
CLEAR (Walls ());
CLEAR (Triggers ());
CLEAR (ObjTriggers ());
CLEAR (ReactorTriggers ());
CLEAR (BotGens ());
CLEAR (EquipGens ());
//CLEAR (Textures () [0]);
//CLEAR (Textures () [1]);
CLEAR (RobotInfo ());
#endif
m_levelVersion = 7;
m_fileType = RL2_FILE;
lightManager.Count () = 0;
Current () = &Current1 ();
*m_szBlockFile = '\0';
MineInfo ().objects.Reset ();
MineInfo ().walls.Reset ();
MineInfo ().doors.Reset ();
MineInfo ().triggers.Reset ();
MineInfo ().control.Reset ();
MineInfo ().botgen.Reset ();
MineInfo ().equipgen.Reset ();
MineInfo ().lightDeltaIndices.Reset ();
MineInfo ().lightDeltaValues.Reset ();
m_nNoLightDeltas = 2;
m_lightRenderDepth = MAX_LIGHT_DEPTH;
m_deltaLightRenderDepth = MAX_LIGHT_DEPTH;
m_bSortObjects = TRUE;
m_bVertigo = false;
m_pHxmExtraData = null;
m_nHxmExtraDataSize = 0;
m_bUseTexColors = false;
LoadDefaultLightAndColor ();
Reset ();
Default ();
//	strcpy (descentPath [1], "d:\\games\\descent\\d2\\");
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
Current () = &Current1 ();
current1.m_nSegment = DEFAULT_SEGMENT;
current1.m_nPoint = DEFAULT_POINT;
current1.m_nLine = DEFAULT_LINE;
current1.m_nSide = DEFAULT_SIDE;
current1.m_nObject = DEFAULT_OBJECT;
current2.m_nSegment = DEFAULT_SEGMENT;
current2.m_nPoint = DEFAULT_POINT;
current2.m_nLine = DEFAULT_LINE;
current2.m_nSide = DEFAULT_SIDE;
current2.m_nObject = DEFAULT_OBJECT;
DLE.ResetUndoBuffer ();
}


// -----------------------------------------------------------------------------
// Create()
//
// Action: makes a new level from the resource file
// -----------------------------------------------------------------------------

short CMine::CreateNewLevel (void)
{
CResource res;
byte *dataP = res.Load (IsD1File () ? IDR_NEW_RDL : IDR_NEW_RL2);
if (!dataP)
	return 0;
// copy dataP to a file

CFileManager::SplitPath ((m_fileType== RDL_FILE) ? descentPath [0] : missionPath, m_startFolder , null, null);
sprintf_s (message, sizeof (message),  (m_fileType== RDL_FILE) ? "%sNEW.RDL" : "%sNEW.RL2", m_startFolder );
ASSIGN (RobotInfo (), DefRobotInfo ());
CFileManager fp;
if (fp.Open (message, "wb")) 
	return 2;
size_t nBytes = fp.Write (dataP, sizeof (byte), (ushort) res.Size ());
fp.Close ();
if (nBytes != res.Size ())
	return 1;
NumObjTriggers () = 0;
return 0;
}


// -----------------------------------------------------------------------------
// CMine::Default()
// -----------------------------------------------------------------------------
void CMine::Default()
{
ClearMineData();

if (m_pHxmExtraData) {
	free (m_pHxmExtraData);
	m_pHxmExtraData = null;
	m_nHxmExtraDataSize = 0;
	}

CSegment& seg = *Segments (0);
CVertex *vert = Vertices (0);

seg.m_sides [0].m_info.nWall = NO_WALL;
seg.m_sides [0].m_info.nBaseTex = 0;
seg.m_sides [0].m_info.nOvlTex = 0;
seg.m_sides [0].m_info.uvls [0].u = 0;
seg.m_sides [0].m_info.uvls [0].v = 0;
seg.m_sides [0].m_info.uvls [0].l = 0x8000U;
seg.m_sides [0].m_info.uvls [1].u = 0;
seg.m_sides [0].m_info.uvls [1].v = 2047;
seg.m_sides [0].m_info.uvls [1].l = 511;
seg.m_sides [0].m_info.uvls [2].u = - 2048;
seg.m_sides [0].m_info.uvls [2].v = 2047;
seg.m_sides [0].m_info.uvls [2].l = 3833;
seg.m_sides [0].m_info.uvls [3].u = - 2048;
seg.m_sides [0].m_info.uvls [3].v = 0;
seg.m_sides [0].m_info.uvls [3].l = 0x8000U;

seg.m_sides [1].m_info.nWall = NO_WALL;
seg.m_sides [1].m_info.nBaseTex = 263;
seg.m_sides [1].m_info.nOvlTex = 264;
seg.m_sides [1].m_info.uvls [0].u = 0;
seg.m_sides [1].m_info.uvls [0].v = 0;
seg.m_sides [1].m_info.uvls [0].l = 0x8000U;
seg.m_sides [1].m_info.uvls [1].u = 0;
seg.m_sides [1].m_info.uvls [1].v = 2047;
seg.m_sides [1].m_info.uvls [1].l = 0x8000U;
seg.m_sides [1].m_info.uvls [2].u = - 2048;
seg.m_sides [1].m_info.uvls [2].v = 2047;
seg.m_sides [1].m_info.uvls [2].l = 0x8000U;
seg.m_sides [1].m_info.uvls [3].u = - 2048;
seg.m_sides [1].m_info.uvls [3].v = 0;
seg.m_sides [1].m_info.uvls [3].l = 0x8000U;

seg.m_sides [2].m_info.nWall = NO_WALL;
seg.m_sides [2].m_info.nBaseTex = 0;
seg.m_sides [2].m_info.nOvlTex = 0;
seg.m_sides [2].m_info.uvls [0].u = 0;
seg.m_sides [2].m_info.uvls [0].v = 0;
seg.m_sides [2].m_info.uvls [0].l = 0x8000U;
seg.m_sides [2].m_info.uvls [1].u = 0;
seg.m_sides [2].m_info.uvls [1].v = 2047;
seg.m_sides [2].m_info.uvls [1].l = 3836;
seg.m_sides [2].m_info.uvls [2].u = - 2048;
seg.m_sides [2].m_info.uvls [2].v = 2047;
seg.m_sides [2].m_info.uvls [2].l = 5126;
seg.m_sides [2].m_info.uvls [3].u = - 2048;
seg.m_sides [2].m_info.uvls [3].v = 0;
seg.m_sides [2].m_info.uvls [3].l = 0x8000U;

seg.m_sides [3].m_info.nWall = NO_WALL;
seg.m_sides [3].m_info.nBaseTex = 270;
seg.m_sides [3].m_info.nOvlTex = 0;
seg.m_sides [3].m_info.uvls [0].u = 0;
seg.m_sides [3].m_info.uvls [0].v = 0;
seg.m_sides [3].m_info.uvls [0].l = 11678;
seg.m_sides [3].m_info.uvls [1].u = 0;
seg.m_sides [3].m_info.uvls [1].v = 2047;
seg.m_sides [3].m_info.uvls [1].l = 12001;
seg.m_sides [3].m_info.uvls [2].u = - 2048;
seg.m_sides [3].m_info.uvls [2].v = 2047;
seg.m_sides [3].m_info.uvls [2].l = 12001;
seg.m_sides [3].m_info.uvls [3].u = - 2048;
seg.m_sides [3].m_info.uvls [3].v = 0;
seg.m_sides [3].m_info.uvls [3].l = 11678;

seg.m_sides [4].m_info.nWall = NO_WALL;
seg.m_sides [4].m_info.nBaseTex = 0;
seg.m_sides [4].m_info.nOvlTex = 0;
seg.m_sides [4].m_info.uvls [0].u = 0;
seg.m_sides [4].m_info.uvls [0].v = 0;
seg.m_sides [4].m_info.uvls [0].l = 0x8000U;
seg.m_sides [4].m_info.uvls [1].u = 0;
seg.m_sides [4].m_info.uvls [1].v = 2043;
seg.m_sides [4].m_info.uvls [1].l = 0x8000U;
seg.m_sides [4].m_info.uvls [2].u = - 2044;
seg.m_sides [4].m_info.uvls [2].v = 2045;
seg.m_sides [4].m_info.uvls [2].l = 0x8000U;
seg.m_sides [4].m_info.uvls [3].u = - 2043;
seg.m_sides [4].m_info.uvls [3].v = 0;
seg.m_sides [4].m_info.uvls [3].l = 0x8000U;

seg.m_sides [5].m_info.nWall = NO_WALL;
seg.m_sides [5].m_info.nBaseTex = 0;
seg.m_sides [5].m_info.nOvlTex = 0;
seg.m_sides [5].m_info.uvls [0].u = 0;
seg.m_sides [5].m_info.uvls [0].v = 0;
seg.m_sides [5].m_info.uvls [0].l = 24576;
seg.m_sides [5].m_info.uvls [1].u = 0;
seg.m_sides [5].m_info.uvls [1].v = 2048;
seg.m_sides [5].m_info.uvls [1].l = 24576;
seg.m_sides [5].m_info.uvls [2].u = - 2048;
seg.m_sides [5].m_info.uvls [2].v = 2048;
seg.m_sides [5].m_info.uvls [2].l = 24576;
seg.m_sides [5].m_info.uvls [3].u = - 2048;
seg.m_sides [5].m_info.uvls [3].v = 0;
seg.m_sides [5].m_info.uvls [3].l = 24576;

seg.SetChild (0, -1);
seg.SetChild (1, -1);
seg.SetChild (2, -1);
seg.SetChild (3, -1);
seg.SetChild (4, -1);
seg.SetChild (5, -1);

seg.m_info.verts [0] = 0;
seg.m_info.verts [1] = 1;
seg.m_info.verts [2] = 2;
seg.m_info.verts [3] = 3;
seg.m_info.verts [4] = 4;
seg.m_info.verts [5] = 5;
seg.m_info.verts [6] = 6;
seg.m_info.verts [7] = 7;

seg.m_info.function = 0;
seg.m_info.nMatCen = -1;
seg.m_info.value = -1;
seg.m_info.s2Flags = 0;
seg.m_info.staticLight = 263152L;
seg.m_info.childFlags = 0;
seg.m_info.wallFlags = 0;
seg.m_info.nIndex = 0;
seg.m_info.mapBitmask = 0;

vert [0] = CVertex (+10, +10, -10);
vert [1] = CVertex (+10, -10, -10);
vert [2] = CVertex (-10, -10, -10);
vert [3] = CVertex (-10, +10, -10);
vert [4] = CVertex (+10, +10, +10);
vert [5] = CVertex (+10, -10, +10);
vert [6] = CVertex (-10, -10, +10);
vert [7] = CVertex (-10, +10, +10);

SegCount () = 1;
VertCount () = 8;
}

// -----------------------------------------------------------------------------
// ClearMineData()
// -----------------------------------------------------------------------------

void CMine::ClearMineData() 
{
	short i;

// initialize Segments ()
CSegment *segP = Segments (0);
for (i = 0; i < MAX_SEGMENTS; i++, segP++)
	segP->m_info.wallFlags &= ~MARKED_MASK;
SegCount () = 0;
// initialize vertices
for (i = 0; i < MAX_VERTICES; i++) 
	vertexManager.Status (i) &= ~MARKED_MASK;
VertCount () = 0;
lightManager.Count () = 0;
// reset "howmany"
MineInfo ().objects.Reset ();
MineInfo ().walls.Reset ();
MineInfo ().doors.Reset ();
MineInfo ().triggers.Reset ();
MineInfo ().control.Reset ();
MineInfo ().botgen.Reset ();
MineInfo ().equipgen.Reset ();
MineInfo ().lightDeltaIndices.Reset ();
MineInfo ().lightDeltaValues.Reset ();
}

// ----------------------------------------------------------------------------------

void CMine::ClearGameItem (CGameItem* items, int nCount)
{
for (int i = 0; i < nCount; i++) {
	items->Clear ();
	items = items->Next ();
	}
}

// ----------------------------------------------------------------------------------
//eof mine.cpp