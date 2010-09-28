// Copyright (c) 1998 Bryan Aamot, Brainware

#include "stdafx.h"
#include "dle-xp-res.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "FileManager.h"
#include "textures.h"
#include "PaletteManager.h"
#include "dle-xp.h"
#include "robot.h"
#include "HogManager.h"
#include "PaletteManager.h"
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
vertexManager.Count () = 0;
segmentManager.Count () = 0;
segmentManager.RobotMakerCount () = 0;
segmentManager.EquipMakerCount () = 0;
wallManager.WallCount () = 0;
wallManager.DoorCount () = 0;
triggerManager.WallTriggerCount () = 0;
triggerManager.ObjTriggerCount () = 0;
triggerManager.ReactorTriggerCount () = 0;
objectManager.Count () = 0;
m_levelVersion = 7;
m_fileType = RL2_FILE;
lightManager.Count () = 0;
*m_szBlockFile = '\0';
segmentManager.ResetInfo ();
wallManager.ResetInfo ();
triggerManager.ResetInfo ();
lightManager.ResetInfo ();
objectManager.ResetInfo ();
m_nNoLightDeltas = 2;
objectManager.SortObjects () = true;
m_bVertigo = false;
m_pHxmExtraData = null;
m_nHxmExtraDataSize = 0;
lightManager.UseTexColors () = false;
lightManager.LoadDefaults ();
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
selections [0].Index () = 0;
selections [1].Index () = 1;
current = &selections [0];
other = &selections [1];
selections [0].m_nSegment = DEFAULT_SEGMENT;
selections [0].m_nPoint = DEFAULT_POINT;
selections [0].m_nLine = DEFAULT_LINE;
selections [0].m_nSide = DEFAULT_SIDE;
selections [0].m_nObject = DEFAULT_OBJECT;
selections [1].m_nSegment = DEFAULT_SEGMENT;
selections [1].m_nPoint = DEFAULT_POINT;
selections [1].m_nLine = DEFAULT_LINE;
selections [1].m_nSide = DEFAULT_SIDE;
selections [1].m_nObject = DEFAULT_OBJECT;
undoManager.Reset ();
}


// -----------------------------------------------------------------------------
// Create()
//
// Action: makes a new level from the resource file
// -----------------------------------------------------------------------------

short CMine::CreateNewLevel (CMemoryFile& fp)
{
CResource res;
// copy data to a file
triggerManager.ObjTriggerCount () = 0;
byte* dataP = res.Load (IsD1File () ? IDR_NEW_RDL : IDR_NEW_RL2);
return !fp.Load (dataP, res.Size ());
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

CSegment& seg = *segmentManager.Segment (0);

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
seg.m_info.flags = 0;
seg.m_info.staticLight = 263152L;
seg.m_info.childFlags = 0;
seg.m_info.wallFlags = 0;
seg.Index () = 0;
seg.m_info.mapBitmask = 0;

CVertex *vertP = vertexManager.Vertex (0);

vertP [0] = CVertex (+10, +10, -10);
vertP [1] = CVertex (+10, -10, -10);
vertP [2] = CVertex (-10, -10, -10);
vertP [3] = CVertex (-10, +10, -10);
vertP [4] = CVertex (+10, +10, +10);
vertP [5] = CVertex (+10, -10, +10);
vertP [6] = CVertex (-10, -10, +10);
vertP [7] = CVertex (-10, +10, +10);

segmentManager.Count () = 1;
vertexManager.Count () = 8;
}

// -----------------------------------------------------------------------------
// ClearMineData()
// -----------------------------------------------------------------------------

void CMine::ClearMineData () 
{
	short i;

// initialize Segments ()
CSegment *segP = segmentManager.Segment (0);
for (i = 0; i < MAX_SEGMENTS; i++, segP++)
	segP->Unmark ();
segmentManager.Count () = 0;
// initialize vertices
for (i = 0; i < MAX_VERTICES; i++) 
	vertexManager.Status (i) &= ~MARKED_MASK;
vertexManager.Count () = 0;
lightManager.Count () = 0;
// reset "howmany"
segmentManager.ResetInfo ();
wallManager.ResetInfo ();
triggerManager.ResetInfo ();
lightManager.ResetInfo ();
objectManager.ResetInfo ();
}

// ----------------------------------------------------------------------------------
//eof mine.cpp