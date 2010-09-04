// Segment.cpp

#include "stdafx.h"
#include "dle-xp-res.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "cfile.h"
#include "TriggerManager.h"
#include "WallManager.h"
#include "SegmentManager.h"
#include "TextureManager.h"
#include "palette.h"
#include "dle-xp.h"
#include "robot.h"
#include "cfile.h"

CSegmentManager segmentManager;

// ------------------------------------------------------------------------- 

void CSegmentManager::CalcSegCenter (CVertex& pos, short nSegment) 
{
  short	*nVerts = Segment (nSegment)->m_info.verts; 
  
pos.Clear ();
for (int i = 0; i < 8; i++)
	pos += *vertexManager.Vertex (nVerts [i]);
pos /= 8.0;
}

// -----------------------------------------------------------------------------

CDoubleVector CSegmentManager::CalcSideCenter (short nSegment, short nSide)
{
current.Get (nSegment, nSide);

	short*	segVertP = Segment (nSegment)->m_info.verts;
	byte*		sideVertP = &sideVertTable [nSide][0];
	CDoubleVector	v;

for (int i = 0; i < 4; i++)
	v += *vertexManager.Vertex (segVertP [sideVertP [i]]);
v /= 4.0;
return v;
}

// -----------------------------------------------------------------------------

CDoubleVector CSegmentManager::CalcSideNormal (short nSegment, short nSide)
{
current.Get (nSegment, nSide);

short* segVertP = Segment (nSegment)->m_info.verts;
byte*	sideVertP = &sideVertTable [nSide][0];

return -Normal (*vertexManager.Vertex (segVertP [sideVertP [0]]), 
					 *vertexManager.Vertex (segVertP [sideVertP [1]]), 
					 *vertexManager.Vertex (segVertP [sideVertP [3]]));
}

// ------------------------------------------------------------------------------ 
// get_opposing_side()
//
// Action - figures out childs nSegment and side for a given side
// Returns - TRUE on success
// ------------------------------------------------------------------------------ 

bool CSegmentManager::OppositeSide (CSideKey& opp, short nSegment, short nSide)
{
  short nChildSeg, nChildSide; 

current.Get (nSegment, nSide); 
#ifdef _DEBUG
if (nSegment < 0 || nSegment >= Count ())
	return false; 
if (nSide < 0 || nSide >= 6)
	return false; 
#endif
nChildSeg = Segment (nSegment)->Child (nSide); 
if (nChildSeg < 0 || nChildSeg >= Count ())
	return false; 
for (nChildSide = 0; nChildSide < 6; nChildSide++) {
	if (Segment (nChildSeg)->Child (nChildSide) == nSegment) {
		opp.m_nSegment = nChildSeg; 
		opp.m_nSide = nChildSide; 
		return true; 
		}
	}
return false; 
}

// -----------------------------------------------------------------------------

CSide* CSegmentManager::OppositeSide (short nSegment, short nSide)
{
current.Get (nSegment, nSide);
CSideKey opp;
return OppositeSide (opp, nSegment, nSide) ? Side (nOppSeg, nOppSide) : null;
}

// -----------------------------------------------------------------------------

int CSegmentManager::IsWall (short nSegment, short nSide)
{
current.Get (nSegment, nSide); 
return (Segment (nSegment)->Child (nSide)== -1) ||
		 (Segment (nSegment)->m_sides [nSide].m_info.nWall < MineInfo ().walls.count); 
}

// -----------------------------------------------------------------------------

CWall* CSegmentManager::Wall (short nSegment, short nSide)
{
current.Get (nSegment, nSide);
return wallManager.Wall (Segment (nSegment)->m_sides [nSide].m_info.nWall);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::DeleteWalls (short nSegment)
{
	CSide *sideP = Segment (nSegment)->m_sides; 

for (int i = MAX_SIDES_PER_SEGMENT; i; i--, sideP++)
	wallManager.Delete (sideP [i].m_info.nWall);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::UpdateVertex (short nOldVert, short nNewVert)
{
CSegment *segP = Segment (0);

for (nSegment = Count (); nSegment; nSegment--, segP++) {
	short* vertP = segP->m_info.verts;
	for (nVertex = 0; nVertex < 8; nVertex++) {
		if (vertP [nVertex] == nOldVert)
			vertP [nVertex] == nNewVert;
		}
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::UpdateWall (short nOldWall, short nNewWall)
{
CSegment *segP = Segments (0);
CSide *sideP;

for (int i = SegCount (); i; i--, segP++)
	for (int j = 0, sideP = segP->m_sides; j < 6; j++, sideP++)
		if (sideP->m_info.nWall >= nOldWall)
			sideP->m_info.nWall = nNewWall;
}

// ----------------------------------------------------------------------------- 
//	AddSegment()
//
//  ACTION - Add new segment at the end. solidifyally joins to adjacent
//           Segment () if sides are identical.  Uses other current cube for
//           textures.
//
//  Returns - TRUE on success
//
//  Changes - Now auto aligns u, v numbers based on parent textures
//
//  NEW - If there is a variable light on the current side of this segment, 
//        it is deleted.
//
//        If cube is special (fuel center, robot maker, etc..) then textures
//        are set to default texture.
// ----------------------------------------------------------------------------- 

void CSegmentManager::InitSegment (short nSegment)
{
Segment (nSegment)->Setup ();
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::IsMarked (CSideKey key)
{
current.Get (key);
CSegment *segP = Segment (key.m_nSegment);
for (int i = 0; i < 4; i++) {
	if (!(vertexManager.Status (segP->m_info.verts [sideVertTable [key.m_nSide][i]]) & MARKED_MASK))
		return false;
	}
return true;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::IsMarked (short nSegment)
{
CSegment *segP = Segment (nSegment);
for (int i = 0;  i < 8; i++)
	if (!(vertexManager.Status (segP->m_info.verts [i]) & MARKED_MASK))
		return false;
return true;
}

// ----------------------------------------------------------------------------- 
//			 mark_segment()
//
//  ACTION - Toggle marked bit of segment and mark/unmark vertices.
//
// ----------------------------------------------------------------------------- 

void CSegmentManager::Mark (short nSegment)
{
  CSegment *segP = Segment (nSegment); 

segP->m_info.wallFlags ^= MARKED_MASK; /* flip marked bit */

// update vertices's marked status
// ..first clear all marked verts
// ..then mark all verts for marked Segment ()
for (nSegment = 0, segP = Segment (0); nSegment < Count (); nSegment++, segP++)
	if (segP->m_info.wallFlags & MARKED_MASK)
		for (nVertex = 0; nVertex < 8; nVertex++)
			vertexManager.Status (segP->m_info.verts [nVertex]) |= MARKED_MASK; 
}

// -----------------------------------------------------------------------------

void CSegmentManager::UpdateMarked (void)
{
	CSegment *segP = Segment (0); 
	int i; 
// mark all cubes which have all 8 verts marked
for (int i = 0; i < Count (); i++, segP++)
	if ((vertexManager.Status (segP->m_info.verts [0]) & MARKED_MASK) &&
		 (vertexManager.Status (segP->m_info.verts [1]) & MARKED_MASK) &&
		 (vertexManager.Status (segP->m_info.verts [2]) & MARKED_MASK) &&
		 (vertexManager.Status (segP->m_info.verts [3]) & MARKED_MASK) &&
		 (vertexManager.Status (segP->m_info.verts [4]) & MARKED_MASK) &&
		 (vertexManager.Status (segP->m_info.verts [5]) & MARKED_MASK) &&
		 (vertexManager.Status (segP->m_info.verts [6]) & MARKED_MASK) &&
		 (vertexManager.Status (segP->m_info.verts [7]) & MARKED_MASK))
		segP->m_info.wallFlags |= MARKED_MASK; 
	else
		segP->m_info.wallFlags &= ~MARKED_MASK; 
}

// -----------------------------------------------------------------------------

void CSegmentManager::MarkAll (void) 
{
	int i; 

for (i = 0; i < Count (); i++) 
	Segment (i)->m_info.wallFlags |= MARKED_MASK; 
for (i = 0; i < vertexManager.Count (); i++) 
	vertexManager.Status (i) |= MARKED_MASK; 
DLE.MineView ()->Refresh (); 
}

// -----------------------------------------------------------------------------

void CSegmentManager::UnmarkAll (void) 
{
	short i; 

CSegment *segP = Segment (0);
for (i = 0; i < MAX_SEGMENTS; i++, segP++)
	segP->m_info.wallFlags &= ~MARKED_MASK; 
byte& stat = vertexManager.Status ();
for (i = 0; i < MAX_VERTICES; i++, stat++)
	stat &= ~MARKED_MASK; 
//	DLE.MineView ()->Refresh (); 
}

// ------------------------------------------------------------------------ 

bool CSegmentManager::HaveMarkedSides (void)
{
int	nSegment, nSide; 

for (nSegment = 0; nSegment < Count (); nSegment++)
	for (nSide = 0; nSide < 6; nSide++)
		if (SideIsMarked (nSegment, nSide))
			return true;
return false; 
}

// ------------------------------------------------------------------------ 

short CSegmentManager::MarkedCount (bool bCheck)
{
	int	nSegment, nCount; 
	CSegment *segP = Segment (0);
for (nSegment = Count (), nCount = 0; nSegment; nSegment--, segP++)
	if (segP->m_info.wallFlags & MARKED_MASK)
		if (bCheck)
			return 1; 
		else
			++nCount; 
return nCount; 
}

// -----------------------------------------------------------------------------
// ResetSide()
//
// Action - sets side to have no child and a default texture
// ----------------------------------------------------------------------------- 

void CSegmentManager::ResetSide (short nSegment, short nSide)
{
if (nSegment < 0 || nSegment >= Count ()) 
	return; 
undoManager.SetModified (true); 
undoManager.Lock ();
Segment (nSegment)->ResetSide (nSide); 
undoManager.Unlock ();
}

// ----------------------------------------------------------------------------- 
//			  set_lines_to_draw()
//
//  ACTION - Determines which lines will be shown when drawing 3d image of
//           the theMine->  This helps speed up drawing by avoiding drawing lines
//           multiple times.
//
// ----------------------------------------------------------------------------- 

void CSegmentManager::SetLinesToDraw (void)
{
  CSegment *segP; 
  short nSegment, nSide; 

for (nSegment = Count (), segP = Segment (0); nSegment; nSegment--, segP++) {
	segP->m_info.mapBitmask |= 0xFFF; 
	// if segment nSide has a child, clear bit for drawing line
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		if (segP->Child (nSide) > -1) { // -1 = no child,  - 2 = outside of world
			segP->m_info.mapBitmask &= ~(1 << (sideLineTable [nSide][0])); 
			segP->m_info.mapBitmask &= ~(1 << (sideLineTable [nSide][1])); 
			segP->m_info.mapBitmask &= ~(1 << (sideLineTable [nSide][2])); 
			segP->m_info.mapBitmask &= ~(1 << (sideLineTable [nSide][3])); 
			}
		}
	}
}

// ------------------------------------------------------------------------ 

int CSegmentManager::AlignTextures (short nStartSeg, short nStartSide, short nOnlyChildSeg, bool bAlign1st, bool bAlign2nd, char bAlignedSides)
{
	CSegment*	segP = Segment (nStartSeg); 
	CSegment*	childSegP; 
	CSide*		sideP = segP->m_sides + nStartSide; 
	CSide*		childSideP; 

	int			return_code = -1; 
	short			i; 
	short			nSide, nChildSeg, nLine; 
	short			point0, point1, vert0, vert1; 
	short			nChildSide, nChildLine; 
	short			nChildPoint0, nChildPoint1, nChildVert0, nChildVert1; 
	short			u0, v0; 
	double		sangle, cangle, angle, length; 

	static int sideChildTable [6][4] = {
		{4, 3, 5, 1}, //{5, 1, 4, 3}, 
		{2, 4, 0, 5}, //{5, 0, 4, 2}, 
		{5, 3, 4, 1}, //{5, 3, 4, 1}, 
		{0, 4, 2, 5}, //{5, 0, 4, 2}, 
		{2, 3, 0, 1}, //{2, 3, 0, 1}, 
		{0, 3, 2, 1} //{2, 3, 0, 1}
		}; 

undoManager.SetModified (true);
undoManager.Lock ();
for (nLine = 0; nLine < 4; nLine++) {
	// find vert numbers for the line's two end points
	point0 = lineVertTable [sideLineTable [nStartSide][nLine]][0]; 
	point1 = lineVertTable [sideLineTable [nStartSide][nLine]][1]; 
	vert0  = segP->m_info.verts [point0]; 
	vert1  = segP->m_info.verts [point1]; 
	// check child for this line
	if (nStartSeg == nOnlyChildSeg) {
		nSide = nStartSide;
		nChildSeg = nStartSeg;
		}
	else {
		nSide = sideChildTable [nStartSide][nLine]; 
		nChildSeg = segP->Child (nSide); 
		}
	if ((nChildSeg < 0) || ((nOnlyChildSeg != -1) && (nChildSeg != nOnlyChildSeg)))
		continue;
	childSegP = Segment (nChildSeg); 
	// figure out which side of child shares two points w/ nStartSide
	for (nChildSide = 0; nChildSide < 6; nChildSide++) {
		if ((nStartSeg == nOnlyChildSeg) && (nChildSide == nStartSide))
			continue;
		if (bAlignedSides & (1 << nChildSide))
			continue;
		// ignore children of different textures (or no texture)
		if (!IsWall (nChildSeg, nChildSide))
			continue;
		if (childSegP->m_sides [nChildSide].m_info.nBaseTex != sideP->m_info.nBaseTex)
			continue;
		for (nChildLine = 0; nChildLine < 4; nChildLine++) {
			// find vert numbers for the line's two end points
			nChildPoint0 = lineVertTable [sideLineTable [nChildSide][nChildLine]][0]; 
			nChildPoint1 = lineVertTable [sideLineTable [nChildSide][nChildLine]][1]; 
			nChildVert0  = childSegP->m_info.verts [nChildPoint0]; 
			nChildVert1  = childSegP->m_info.verts [nChildPoint1]; 
			// if points of child's line== corresponding points of parent
			if (!((nChildVert0 == vert1 && nChildVert1 == vert0) ||
					(nChildVert0 == vert0 && nChildVert1 == vert1)))
				continue;
			// now we know the child's side & line which touches the parent
			// child:  nChildSeg, nChildSide, nChildLine, nChildPoint0, nChildPoint1
			// parent: nStartSeg, nStartSide, nLine, point0, point1
			childSideP = childSegP->m_sides + nChildSide; 
			if (bAlign1st) {
				// now translate all the childs (u, v) coords so child_point1 is at zero
				u0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].u; 
				v0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].v; 
				for (i = 0; i < 4; i++) {
					childSideP->m_info.uvls [i].u -= u0; 
					childSideP->m_info.uvls [i].v -= v0; 
					}
				// find difference between parent point0 and child point1
				u0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].u - sideP->m_info.uvls [nLine].u; 
				v0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].v - sideP->m_info.uvls [nLine].v; 
				// find the angle formed by the two lines
				sangle = atan3(sideP->m_info.uvls [(nLine + 1) % 4].v - sideP->m_info.uvls [nLine].v, 
									sideP->m_info.uvls [(nLine + 1) % 4].u - sideP->m_info.uvls [nLine].u); 
				cangle = atan3(childSideP->m_info.uvls [nChildLine].v - childSideP->m_info.uvls [(nChildLine + 1) % 4].v, 
									childSideP->m_info.uvls [nChildLine].u - childSideP->m_info.uvls [(nChildLine + 1) % 4].u); 
				// now rotate childs (u, v) coords around child_point1 (cangle - sangle)
				for (i = 0; i < 4; i++) {
					angle = atan3(childSideP->m_info.uvls [i].v, childSideP->m_info.uvls [i].u); 
					length = sqrt((double) childSideP->m_info.uvls [i].u * (double) childSideP->m_info.uvls [i].u +
									  (double) childSideP->m_info.uvls [i].v * (double) childSideP->m_info.uvls [i].v); 
					angle -= (cangle - sangle); 
					childSideP->m_info.uvls [i].u = (short)(length * cos (angle)); 
					childSideP->m_info.uvls [i].v = (short)(length * sin (angle)); 
					}
				// now translate all the childs (u, v) coords to parent point0
				for (i = 0; i < 4; i++) {
					childSideP->m_info.uvls [i].u -= u0; 
					childSideP->m_info.uvls [i].v -= v0; 
					}
				// modulo points by 0x800 (== 64 pixels)
				u0 = childSideP->m_info.uvls [0].u / 0x800; 
				v0 = childSideP->m_info.uvls [0].v / 0x800; 
				for (i = 0; i < 4; i++) {
					childSideP->m_info.uvls [i].u -= u0*0x800; 
					childSideP->m_info.uvls [i].v -= v0*0x800; 
					}
				if (nOnlyChildSeg != -1)
					return_code = nChildSide; 
				}
			if (bAlign2nd && sideP->m_info.nOvlTex && childSideP->m_info.nOvlTex) {
				int r;
				switch (sideP->m_info.nOvlTex & 0xC000) {
					case 0:
						r = 0;
						break;
					case 0xC000:
						r = 1;
						break;
					case 0x8000:
						r = 2;
						break;
					case 0x4000:
						r = 3;
						break;
					}
				int h = (int) (Degrees (fabs (angle)) / 90 + 0.5); 
//				h +=(nChildLine + nLine + 2) % 4; //(nChildLine > nLine) ? nChildLine - nLine : nLine - nChildLine;
				h = (h + r) % 4;
				childSideP->m_info.nOvlTex &= ~0xC000;
				switch (h) {
					case 0:
						break;
					case 1:
						childSideP->m_info.nOvlTex |= 0xC000;
						break;
					case 2:
						childSideP->m_info.nOvlTex |= 0x8000;
						break;
					case 3:
						childSideP->m_info.nOvlTex |= 0x4000;
						break;
					}
				}
			break;
			}
		}
	}
undoManager.Unlock ();
return return_code; 
}

// -----------------------------------------------------------------------------

void CSegmentManager::Textures (CSideKey key, short& nBaseTex, short& nOvlTex)
{
current.Get (key);
Side (key)->GetTextures (nBaseTex, nOvlTex);
}

// -----------------------------------------------------------------------------

bool CSegmentManager::SetTextures (CSideKey key, short nBaseTex, short nOvlTex)
{
	bool bUndo, bChange = false;

bUndo = undoManager.SetModified (true); 
undoManager.Lock (); 
current.Get (key); 
CSide *sideP = Side (key); 
bChange = sideP->SetTextures (nBaseTex, nOvlTex);
if (!bChange) {
	undoManager.ResetModified (bUndo);
	return false;
	}
if ((IsLight (sideP->m_info.nBaseTex) == -1) && (IsLight (sideP->m_info.nOvlTex & 0x3fff) == -1))
	DeleteVariableLight (key); 
if (!wallManager.ClipFromTexture (key))
	wallManager.CheckForDoor (key); 
undoManager.Unlock (); 
sprintf_s (message, sizeof (message), "side has textures %d, %d", sideP->m_info.nBaseTex & 0x3fff, sideP->m_info.nOvlTex & 0x3fff); 
INFOMSG (message); 
return true;
}

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberRobotMakers (void) 
{
	int		i, nMatCens, value, nSegment; 
	CSegment	*segP; 

// number "matcen"
nMatCens = 0; 
for (i = 0; i < MineInfo ().botGen.count; i++) {
	nSegment = RobotMakers (i)->m_info.nSegment; 
	if (nSegment >= 0) {
		segP = Segment (nSegment); 
		segP->m_info.value = i; 
		if (segP->m_info.function== SEGMENT_FUNC_ROBOTMAKER)
			segP->m_info.nMatCen = nMatCens++; 
		}
	}

// number "value"
value = 0; 
for (i = 0, segP = Segment (0); i < Count (); i++, segP++)
	if (segP->m_info.function== SEGMENT_FUNC_NONE)
		segP->m_info.value = 0; 
	else
		segP->m_info.value = value++; 
}

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberEquipGens (void) 
{
	int		i, nMatCens, value, nSegment; 
	CSegment	*segP; 

// number "matcen"
nMatCens = 0; 
for (i = 0; i < MineInfo ().equipGen.count; i++) {
	nSegment = EquipMakers (i)->m_info.nSegment; 
	if (nSegment >= 0) {
		segP = Segment (nSegment); 
		segP->m_info.value = i; 
		if (segP->m_info.function== SEGMENT_FUNC_EQUIPMAKER)
			segP->m_info.nMatCen = nMatCens++; 
		}
	}

// number "value"
value = 0; 
for (i = 0, segP = Segment (0); i < Count (); i++, segP++)
	if (segP->m_info.function== SEGMENT_FUNC_NONE)
		segP->m_info.value = 0; 
	else
		segP->m_info.value = value++; 
}

                        /* -------------------------- */

void CSegmentManager::CopyOtherSegment (void)
{
	bool bUndo, bChange = false;

if (selections [0].m_nSegment == selections [1].m_nSegment)
	return; 
short nSegment = current.m_nSegment; 
CSegment *otherSeg = other.Segment (); 
bUndo = undoManager.SetModified (true); 
undoManager.Lock ();
int nSide;
for (nSide = 0; nSide < 6; nSide++)
	if (SetTextures (nSegment, nSide, otherSeg->m_sides [nSide].m_info.nBaseTex, otherSeg->m_sides [nSide].m_info.nOvlTex))
		bChange = true;
if (!bChange)
	undoManager.ResetModified (bUndo);
else {
	undoManager.Unlock ();
	DLE.MineView ()->Refresh (); 
	}
}

// ----------------------------------------------------------------------------- 

CSegment* CSegmentManager::FindRobotMaker (short i = 0)
{
	CSegment* segP = Segment (i);

for (; i < Count (); i++, segP++)
	if (segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER)
		return segP;
return null;
}

// -----------------------------------------------------------------------------

void CSegmentManager::SetLight (double fLight, bool bAll, bool bDynSegLights)
{
	long nLight = (int) (fLight * 65536); //24.0 * 327.68);

UndoManager.SetModified (true);

fLight /= 100.0;
CSegment *segP = Segment (0);
for (short nSegment = Count (); nSegment; nSegment--, segP++) {
	if (bAll || (segP->m_info.wallFlags & MARKED_MASK)) {
		if (!bDynSegLights)
			segP->m_info.staticLight = nLight;
		else {
			int l = 0;
			int c = 0;
			CSide* sideP = segP->m_sides;
			for (short nSide = 0; nSide < 6; nSide++) {
				for (short nCorner = 0; nCorner < 4; nCorner++) {
					ushort h = (ushort) sideP [nSide].m_info.uvls [nCorner].l;
					if (h || !sideP->IsVisible ()) {
						l += h;
						c++;
						}
					}
				}
			segP->m_info.staticLight = (int) (c ? fLight * ((double) l / (double) c) * 2 : nLight);
			}
		}
	}
}

// ----------------------------------------------------------------------------- 

void ReadSegments (CFileManager& fp, int nFileVersion)
{
if (m_segmentInfo.offset >= 0) {
	for (int i = 0; i < Count (); i++)
		m_segments [i].Read (fp, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void WriteSegments (CFileManager& fp, int nFileVersion)
{
if (Count () == 0)
	m_segmentInfo.offset = -1;
else {
	m_segmentInfo.offset = fp.Tell ();
	for (int = 0; i < Count (); i++)
		m_segments [i].Write (fp, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void ReadMatCens (CFileManager& fp, int nFileVersion, int nClass)
{
if (m_matCenInfo [nClass].offset >= 0) {
	for (int i = 0; i < Count (nClass); i++)
		m_matCens [nClass][i].Read (fp, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void WriteMatCens (CFileManager& fp, int nFileVersion, int nClass)
{
if (m_matCenInfo [nClass].count == 0)
	m_matCenInfo [nClass].offset = -1;
else {
	m_matCenInfo [nClass].offset = fp.Tell ();
	for (int = 0; i < Count (nClass); i++)
		m_matCens [nClass][i].Write (fp, info, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void ReadRobotMakers (CFileManager& fp, int nFileVersion)
{
ReadMatCens (fp, nFileVersion, 1);
}

// ----------------------------------------------------------------------------- 

void WriteRobotMakers (CFileManager& fp, int nFileVersion)
{
WriteMatCens (fp, nFileVersion, 1);
}

// ----------------------------------------------------------------------------- 

void ReadEquipMakers (CFileManager& fp, int nFileVersion)
{
ReadMatCens (fp, nFileVersion, 1);
}

// ----------------------------------------------------------------------------- 

void WriteEquipMakers (CFileManager& fp, int nFileVersion)
{
WriteMatCens (fp, nFileVersion, 1);
}

// ----------------------------------------------------------------------------- 

void Clear (void)
{
for (int i = 0; i < Count (); i++)
	m_segments [i].Clear ();
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp