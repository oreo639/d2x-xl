// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

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
segP = Segment (0);
for (short nSegment = Count (); nSegment; nSegment--, segP++)
	if (segP->m_info.wallFlags & MARKED_MASK)
		for (short nVertex = 0; nVertex < 8; nVertex++)
			vertexManager.Status (segP->m_info.verts [nVertex]) |= MARKED_MASK; 
}

// -----------------------------------------------------------------------------

void CSegmentManager::UpdateMarked (void)
{
	CSegment *segP = Segment (0); 

// mark all cubes which have all 8 verts marked
for (int i = 0; i < Count (); i++, segP++) {
	short* vertP = segP->m_info.verts;
	if ((vertexManager.Status (vertP [0]) & MARKED_MASK) &&
		 (vertexManager.Status (vertP [1]) & MARKED_MASK) &&
		 (vertexManager.Status (vertP [2]) & MARKED_MASK) &&
		 (vertexManager.Status (vertP [3]) & MARKED_MASK) &&
		 (vertexManager.Status (vertP [4]) & MARKED_MASK) &&
		 (vertexManager.Status (vertP [5]) & MARKED_MASK) &&
		 (vertexManager.Status (vertP [6]) & MARKED_MASK) &&
		 (vertexManager.Status (vertP [7]) & MARKED_MASK))
		segP->m_info.wallFlags |= MARKED_MASK; 
	else
		segP->m_info.wallFlags &= ~MARKED_MASK; 
	}
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
CSegment* segP = Segment (0);
for (short nSegment = Count (); nSegment; nSegment--, segP++) {
	CSide* sideP = segP->m_sides;
	for (short nSide = 0; nSide < 6; nSide++)
		if (IsMarked (CSideKey (nSegment, nSide)))
			return true;
	}
return false; 
}

// ------------------------------------------------------------------------ 

short CSegmentManager::MarkedCount (bool bCheck)
{
int nCount = 0; 
CSegment *segP = Segment (0);
for (short nSegment = Count (); nSegment; nSegment--, segP++)
	if (segP->m_info.wallFlags & MARKED_MASK)
		if (bCheck)
			return 1; 
		else
			++nCount; 
return nCount; 
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp