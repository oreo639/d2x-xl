// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

CSegmentManager segmentManager;

// ------------------------------------------------------------------------- 

void CSegmentManager::CalcCenter (CVertex& pos, short nSegment) 
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

void CSegmentManager::UpdateVertex (short nOldVert, short nNewVert)
{
CSegment *segP = Segment (0);

for (short nSegment = Count (); nSegment; nSegment--, segP++) {
	short* vertP = segP->m_info.verts;
	for (short nVertex = 0; nVertex < 8; nVertex++) {
		if (vertP [nVertex] == nOldVert)
			vertP [nVertex] = nNewVert;
		}
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::UpdateWall (short nOldWall, short nNewWall)
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
undoManager.SetModified (true); 
undoManager.Lock ();
Segment (nSegment)->Reset (nSide); 
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

// ----------------------------------------------------------------------------- 

void CSegmentManager::CopyOtherSegment (void)
{
	bool bUndo, bChange = false;

if (selections [0].m_nSegment == selections [1].m_nSegment)
	return; 
short nSegment = current.m_nSegment; 
CSegment *otherSeg = other.Segment (); 
bUndo = undoManager.SetModified (true); 
undoManager.Lock ();
for (int nSide = 0; nSide < 6; nSide++)
	if (SetTextures (CSideKey (nSegment, nSide), otherSeg->m_sides [nSide].m_info.nBaseTex, otherSeg->m_sides [nSide].m_info.nOvlTex))
		bChange = true;
if (!bChange)
	undoManager.ResetModified (bUndo);
else {
	undoManager.Unlock ();
	DLE.MineView ()->Refresh (); 
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//eof segmentmanager.cpp