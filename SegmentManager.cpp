// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

CSegmentManager segmentManager;

// ------------------------------------------------------------------------- 

void CSegmentManager::CalcCenter (CVertex& pos, short nSegment) 
{
pos.Clear ();
CSegment* segP = Segment (nSegment);
for (int i = 0; i < 8; i++)
	pos += *segP->Vertex (i);
pos /= 8.0;
}

// -----------------------------------------------------------------------------

CDoubleVector CSegmentManager::CalcSideCenter (CSideKey key)
{
current.Get (key);

	CSegment*	segP = Segment (key.m_nSegment);
	byte*		sideVertP = &sideVertTable [key.m_nSide][0];
	CDoubleVector	v;

for (int i = 0; i < 4; i++)
	v += *segP->Vertex (sideVertP [i]);
v /= 4.0;
return v;
}

// -----------------------------------------------------------------------------

CDoubleVector CSegmentManager::CalcSideNormal (CSideKey key)
{
current.Get (key);
CSegment* segP = Segment (key.m_nSegment);
byte*	sideVertP = &sideVertTable [key.m_nSide][0];
return -Normal (*segP->Vertex (sideVertP [0]), *segP->Vertex (sideVertP [1]), *segP->Vertex (sideVertP [3]));
}

// ------------------------------------------------------------------------------ 
// get_opposing_side()
//
// Action - figures out childs nSegment and side for a given side
// Returns - TRUE on success
// ------------------------------------------------------------------------------ 

CSide* CSegmentManager::OppositeSide (CSideKey key, CSideKey& opp)
{
current.Get (key); 
#ifdef _DEBUG
if (key.m_nSegment < 0 || key.m_nSegment >= Count ())
	return false; 
if (key.m_nSide < 0 || key.m_nSide >= 6)
	return false; 
#endif
short nChildSeg = Segment (key.m_nSegment)->Child (key.m_nSide); 
if (nChildSeg < 0 || nChildSeg >= Count ())
	return false; 
for (short nChildSide = 0; nChildSide < 6; nChildSide++) {
	if (Segment (nChildSeg)->Child (nChildSide) == key.m_nSegment) {
		opp.m_nSegment = nChildSeg; 
		opp.m_nSide = nChildSide; 
		return Side (opp); 
		}
	}
return null; 
}

// -----------------------------------------------------------------------------

int CSegmentManager::IsWall (CSideKey key)
{
current.Get (key); 
return (Segment (key.m_nSegment)->Child (key.m_nSide) == -1) || (Wall (key) != null);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::DeleteWalls (short nSegment)
{
	CSide *sideP = Segment (nSegment)->m_sides; 

for (int i = MAX_SIDES_PER_SEGMENT; i; i--, sideP++)
	wallManager.Delete (sideP [i].m_info.nWall);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::UpdateVertices (short nOldVert, short nNewVert)
{
CSegment *segP = Segment (0);

for (CSegmentIterator si; si; si++) {
	ushort* vertP = si->m_info.verts;
	for (short nVertex = 0; nVertex < 8; nVertex++) {
		if (vertP [nVertex] == nOldVert)
			vertP [nVertex] = nNewVert;
		}
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::UpdateWalls (short nOldWall, short nNewWall)
{
CSegment *segP = Segment (0);
for (int i = Count (); i; i--, segP++) {
	CSide* sideP = segP->m_sides;
	for (int j = 0; j < 6; j++, sideP++)
		if (sideP->m_info.nWall >= nOldWall)
			sideP->m_info.nWall = nNewWall;
	}
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
undoManager.Begin (true); 
undoManager.Begin ();
Segment (nSegment)->Reset (nSide); 
undoManager.End ();
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

// ----------------------------------------------------------------------------- 

void CSegmentManager::CopyOtherSegment (void)
{
	bool bUndo, bChange = false;

if (selections [0].m_nSegment == selections [1].m_nSegment)
	return; 
short nSegment = current.m_nSegment; 
CSegment *otherSeg = other.Segment (); 
bUndo = undoManager.Begin (true); 
undoManager.Begin ();
for (int nSide = 0; nSide < 6; nSide++)
	if (SetTextures (CSideKey (nSegment, nSide), otherSeg->m_sides [nSide].m_info.nBaseTex, otherSeg->m_sides [nSide].m_info.nOvlTex))
		bChange = true;
if (!bChange)
	undoManager.Unroll ();
else {
	undoManager.End ();
	DLE.MineView ()->Refresh (); 
	}
}

// -----------------------------------------------------------------------------

int CSegmentManager::Fix (void)
{
int errFlags = 0;

for (CSegmentIterator si; si; si++) {
	CSegment *segP = &(*si);
	for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		CSide& side = segP->m_sides [nSide];
		if ((side.m_info.nWall >= wallManager.Count ()) || !side.Wall ()->Used ()) {
			side.m_info.nWall = NO_WALL;
			errFlags |= 1;
			}
		if ((segP->Child (nSide) < -2) || (segP->Child (nSide) > Count ())) {
			segP->SetChild (nSide, -1);
			errFlags |= 2;
			}
		}
	for (short nVertex = 0; nVertex < MAX_VERTICES_PER_SEGMENT; nVertex++) {
		if ((segP->m_info.verts [nVertex] < 0) || (segP->m_info.verts [nVertex] >= vertexManager.Count ())) {
			segP->m_info.verts [nVertex] = 0;  // this will cause a bad looking picture
			errFlags |= 4;
			}
		}
	}
return errFlags;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//eof segmentmanager.cpp