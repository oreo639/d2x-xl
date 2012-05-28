// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

bool CSegmentManager::IsTagged (CSideKey key)
{
return Segment (key.m_nSegment)->IsTagged (key.m_nSide);
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::IsTagged (short nSegment)
{
CSegment *segP = Segment (nSegment);
for (int i = 0;  i < 8; i++)
	if (!(vertexManager.Status (segP->m_info.vertexIds [i]) & TAGGED_MASK))
		return false;
return true;
}

// ----------------------------------------------------------------------------- 
//			 mark_segment()
//
//  ACTION - Toggle marked bit of segment and tag/untag vertices.
//
// ----------------------------------------------------------------------------- 

void CSegmentManager::Tag (short nSegment)
{
  CSegment *segP = Segment (nSegment); 

segP->Tag (); /* flip marked bit */

// update vertices's marked status
// ..first clear all marked verts
// ..then mark all verts for marked Segment ()
int nSegments = segmentManager.Count ();
for (int si = 0; si < nSegments; si++) {
	CSegment* segP = segmentManager.Segment (si);
	if (segP->IsTagged ())
		for (ushort nVertex = 0; nVertex < 8; nVertex++)
			segP->Vertex (nVertex)->Status () |= TAGGED_MASK; 
	}
}

// -----------------------------------------------------------------------------

void CSegmentManager::UpdateTagged (void)
{
// mark all cubes which have all 8 verts marked
CSegment* segP = Segment (0);
short nSegments = Count ();

for (short i = 0; i < nSegments; i++, segP++) {
	ushort* vertexIds = segP->m_info.vertexIds;
	short nTagged = 0, nVertices = 0;
	short j = 0;
	for (; j < 8; j++) {
		if ((vertexIds [j] <= MAX_VERTEX) && !(vertexManager.Status (vertexIds [j]) & TAGGED_MASK))
			break;
		}
	if (j < 8)
		segP->UnTag (); 
	else
		segP->Tag (); 
	}
}

// -----------------------------------------------------------------------------

void CSegmentManager::TagAll (ubyte mask) 
{
CSegment* segP = Segment (0);
short nSegments = Count ();
for (short i = 0; i < nSegments; i++, segP++) {
	segP->Tag (mask); 
	for (short j = 0; j < 8; j++)
		if (segP->VertexId (j) <= MAX_VERTEX)
			segP->Vertex (j)->Status () |= mask;
	}
DLE.MineView ()->Refresh (); 
}

// -----------------------------------------------------------------------------

void CSegmentManager::UnTagAll (ubyte mask) 
{
CSegment* segP = Segment (0);
short nSegments = Count ();
for (short i = 0; i < nSegments; i++, segP++) {
	segP->UnTag (mask); 
	for (short j = 0; j < 6; j++)
		Side (j)->UnTag (mask);
	for (short j = 0; j < 8; j++)
		if (segP->VertexId (j) <= MAX_VERTEX)
			segP->Vertex (j)->Status () &= ~mask;
	}
DLE.MineView ()->Refresh (); 
}

// ------------------------------------------------------------------------ 

bool CSegmentManager::HaveTaggedSides (void)
{
int nSegments = segmentManager.Count ();
for (int nSegment = 0; nSegment < nSegments; nSegment++) {
	CSide* sideP = segmentManager.Segment (nSegment)->m_sides;
	for (short nSide = 0; nSide < 6; nSide++)
		if (IsTagged (CSideKey (nSegment, nSide)))
			return true;
	}
return false; 
}

// ------------------------------------------------------------------------ 

short CSegmentManager::TaggedCount (bool bCheck)
{
int nCount = 0; 
CSegment* segP = Segment (0);
short nSegments = Count ();
for (short i = 0; i < nSegments; i++, segP++) {
	if (segP->IsTagged ()) {
		if (bCheck)
			return 1; 
		++nCount; 
		}
	}
return nCount; 
}

// ------------------------------------------------------------------------

void CSegmentManager::TagSelected (void)
{
	bool	bSegTag = false; 
	CSegment *segP = current->Segment (); 
	int i, p [8], nPoints; 

switch (DLE.MineView ()->GetSelectMode ()) {
	case eSelectPoint:
		nPoints = 1; 
		p [0] = current->VertexId (); 
		break; 

	case eSelectLine:
		nPoints = 2; 
		p [0] = segP->VertexId (current->Side ()->VertexIdIndex (current->m_nPoint)); 
		p [1] = segP->VertexId (current->Side ()->VertexIdIndex (current->m_nPoint + 1)); 
		break; 

	case eSelectSide:
		nPoints = current->Side ()->VertexCount (); 
		for (i = 0; i < nPoints; i++)
			p [i] = segP->VertexId (current->Side ()->VertexIdIndex (i)); 
		break; 

	case eSelectSegment:
	default:
		nPoints = 8; 
		for (i = 0; i < nPoints; i++)
			p [i] = segP->VertexId (i); 
		break; 

	//default:
	//	bSegTag = true; 
	}

if (bSegTag)
	Tag (current->m_nSegment); 
else {
	// set i to nPoints if all verts are marked
	for (i = 0; i < nPoints; i++)
		if (!(vertexManager.Status (p [i]) & TAGGED_MASK)) 
			break; 
		// if all verts are tagged, then untag them
	if (i == nPoints) {
		for (i = 0; i < nPoints; i++)
			vertexManager.Status (p [i]) &= ~TAGGED_MASK; 
		}
	else {
		// otherwise tag all the points
		for (i = 0; i < nPoints; i++)
			vertexManager.Status (p [i]) |= TAGGED_MASK; 
		}
	UpdateTagged (); 
	}
DLE.MineView ()->Refresh (); 
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp