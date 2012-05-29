// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

extern short nDbgSeg, nDbgSide;
extern int nDbgVertex;
uint nDbgEdgeKey = 0xffffffff;

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

short CSegmentManager::TaggedCount (bool bSides, bool bCheck)
{
int nCount = 0; 
CSegment* segP = Segment (0);
short nSegments = Count ();
for (short i = 0; i < nSegments; i++, segP++) {
	if (segP->IsTagged () || (bSides && segP->IsTagged (short (-1)))) {
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

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// TODO: Beginning with the current side, walk through all adjacent sides and 
// mark all in plane

int CTaggingStrategy::Run (void) 
{
segmentManager.UnTagAll ();

m_sideList.Destroy ();
m_sideList.Create (segmentManager.VisibleSideCount ());

int nHead = 0;
int nTail = 1;
m_sideList [nHead] = CSideKey (*current);
current->Segment ()->Tag (current->m_nSide);

CAVLTree <CEdgeTreeNode, uint> edgeTree;
segmentManager.GatherEdges (edgeTree);

segmentManager.ComputeNormals (false);

CEdgeList edgeList;

double maxAngle = cos (Radians (22.5));

while (nHead < nTail) {
	CSideKey key = m_sideList [nHead++];
	CSegment* m_segP = segmentManager.Segment (key);
	CSide* m_sideP = segmentManager.Side (key);
	edgeList.Reset ();
	if (!m_segP->BuildEdgeList (edgeList, ubyte (key.m_nSide), true))
		continue;
	int nEdges = edgeList.Count ();
	for (int nEdge = 0; nEdge < nEdges; nEdge++) {
		ubyte side1, side2, i1, i2;
		edgeList.Get (nEdge, side1, side2, i1, i2);
		if (side1 > 5) {
			side1 = side2;
			if (side1 > 5)
				continue;
			}
		ushort v1 = m_segP->VertexId (i1);
		ushort v2 = m_segP->VertexId (i2);
		CEdgeTreeNode* node = edgeTree.Find ((v1 < v2) ? v1 + (uint (v2) << 16) : v2 + (uint (v1) << 16));
		if (!node)
			continue;
		CSLLIterator<CSideKey, CSideKey> iter (node->m_sides);
		for (iter.Begin (); *iter != iter.End (); iter++) {
			CSegment* m_childSegP = segmentManager.Segment (**iter);
			if (!m_childSegP->IsTagged (iter->m_nSide)) {
				CSide* m_childSideP = segmentManager.Side (**iter);
				double dot = Dot (m_sideP->Normal (), m_childSideP->Normal ());
				if (dot < maxAngle) 
					continue;
				m_childSegP->Tag (iter->m_nSide);
				m_sideList [nTail++] = **iter;
				}
			}
		}
	}
return nTail;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp