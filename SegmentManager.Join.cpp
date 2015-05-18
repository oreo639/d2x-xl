// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

extern short nDbgSeg, nDbgSide;
extern int nDbgVertex;

// ----------------------------------------------------------------------------- 
// LinkSegments()
//
//  Action - checks 2 Segment () and 2 sides to see if the vertices are identical
//           If they are, then the segment sides are linked and the vertices
//           are removed (nSide1 is the extra side).
//
//  Change - no longer links if segment already has a child
//           no longer links Segment () if vert numbers are not in the right order
//
// ----------------------------------------------------------------------------- 

bool CSegmentManager::Link (short nSegment1, short nSide1, short nSegment2, short nSide2, double margin)
{
	short			i, j, n; 
	CVertex		v1 [4], v2 [4]; 
	tVertMatch	match [4]; 

CSegment* seg1 = Segment (nSegment1); 
CSegment* seg2 = Segment (nSegment2); 

// don't link to a segment which already has a child
if ((seg1->ChildId (nSide1) != -1) || (seg2->ChildId (nSide2) != -1))
	return false; 

CSide* side1 = seg1->Side (nSide1);
CSide* side2 = seg2->Side (nSide2);

if (side1->Shape () != side2->Shape ())
	return false;
if (side1->Shape () > SIDE_SHAPE_TRIANGLE)
	return false;

n = side1->VertexCount ();
// copy vertices for comparison later (makes code more readable)
for (i = 0; i < n; i++) {
	v1 [i] = *vertexManager.Vertex (seg1->m_info.vertexIds [side1->VertexIdIndex (i)]);
	v2 [i] = *vertexManager.Vertex (seg2->m_info.vertexIds [side2->VertexIdIndex (i)]);
	match [i].v = -1; 
	}

// check to see if all 4 vertices match exactly one of each of the 4 other segment's vertices
for (i = 0; i < n; i++) {
	for (j = 0; j < n; j++) {
		if ((fabs (v1 [i].v.x - v2 [j].v.x) < margin) &&
			 (fabs (v1 [i].v.y - v2 [j].v.y) < margin) &&
			 (fabs (v1 [i].v.z - v2 [j].v.z) < margin)) {
			if (match [j].v != -1) // if this vertex already matched another vertex then abort
				return false; 
			else
				match [j].v = i;  // remember which vertex it matched
			}
		}
	}
if (match [0].v == -1)
	return false;

static int matches [][4] = {{0,3,2,1},{1,0,3,2},{2,1,0,3},{3,2,1,0}};

for (i = 1; i < n; i++) {
	if (match [i].v != matches [match [0].v][i])
		return false;
	}
// make sure verts match in the correct order
// if not failed and match found for each
LinkSides (nSegment1, nSide1, nSegment2, nSide2, match); 
return true; 
}


// ----------------------------------------------------------------------------- 
// LinkSides()
// ----------------------------------------------------------------------------- 

void CSegmentManager::LinkSides (short nSegment1, short nSide1, short nSegment2, short nSide2, tVertMatch match [4]) 
{
	CSegment*	seg1 = Segment (nSegment1); 
	CSegment*	seg2 = Segment (nSegment2); 
	CSide*		side1 = seg1->Side (nSide1);
	CSide*		side2 = seg2->Side (nSide2);
	ushort		nVertex, oldVertex, newVertex; 
	int			i, n = side1->VertexCount (); 

undoManager.Begin (__FUNCTION__, udSegments);
seg1->SetChild (nSide1, nSegment2); 
if (!side1->Wall ()) {
	side1->ResetTextures ();
	}
seg2->SetChild (nSide2, nSegment1); 
if (!side2->Wall ()) {
	side2->ResetTextures ();
	}

// merge vertices
for (i = 0; i < n; i++) {
	oldVertex = seg1->m_info.vertexIds [side1->VertexIdIndex (i)]; 
	newVertex = seg2->m_info.vertexIds [side2->VertexIdIndex (match [i].v)]; 

	// if either vert was marked, then mark the new vert
	vertexManager.Status (newVertex) |= (vertexManager.Status (oldVertex) & TAGGED_MASK); 

	// update all Segment () that use this vertex
	if (oldVertex != newVertex) {
		int nSegments = segmentManager.Count ();
		CSegment* pSegment = segmentManager.Segment (0);
		for (int nSegment = 0; nSegment < nSegments; nSegment++, pSegment++) {
			for (nVertex = 0; nVertex < 8; nVertex++)
				if (pSegment->m_info.vertexIds [nVertex] == oldVertex)
					pSegment->m_info.vertexIds [nVertex] = newVertex; 
			}
		// then delete the vertex
		vertexManager.Delete (oldVertex); 
		}
	}
undoManager.End (__FUNCTION__);
}

// ----------------------------------------------------------------------------- 
// Mine - Joinpoints
// ----------------------------------------------------------------------------- 

void CSegmentManager::JoinPoints (void) 
{
	CSegment *seg1, *seg2; 
	double distance; //v1x, v1y, v1z, v2x, v2y, v2z; 
	int vert1, vert2; 
	CSideKey key, otherKey;

if (tunnelMaker.Active ()) 
	return; 

if (selections [0].m_nSegment== selections [1].m_nSegment) {
	ErrorMsg ("You cannot joint two points on the same segment.\n\n"
				 "Hint: The two golden circles represent the current point, \n"
				 "and the 'other' segment's current point.  Press 'P' to change the\n"
				 "current point or press the space bar to switch to the other segment."); 
	return;
	}

other = &selections [*current == selections [0]];
seg1 = Segment (current->SegmentId ()); 
seg2 = Segment (other->m_nSegment); 
vert1 = seg1->m_info.vertexIds [current->Side ()->VertexIdIndex (current->Point ())]; 
vert2 = seg2->m_info.vertexIds [other->Side ()->VertexIdIndex (other->Point ())]; 
// make sure verts are different
if (vert1== vert2) {
	ErrorMsg ("These points are already joined."); 
	return; 
	}
// make sure there are distances are close enough
distance = Distance (*vertexManager.Vertex (vert1), *vertexManager.Vertex (vert2)); 
if (distance > MAX_JOIN_DISTANCE) {
	ErrorMsg ("Points are too far apart to join"); 
	return; 
	}
if (QueryMsg("Are you sure you want to join the current point\n"
				 "with the 'other' segment's current point?") != IDYES)
	return; 
undoManager.Begin (__FUNCTION__, udSegments);
// define vert numbers
seg1->m_info.vertexIds [current->Side ()->VertexIdIndex (current->Point ())] = vert2; 
// delete any unused vertices
//  vertexManager.DeleteUnused (); 
FixChildren (); 
undoManager.End (__FUNCTION__);
SetLinesToDraw (); 
DLE.MineView ()->Refresh ();
}

// ----------------------------------------------------------------------------- 
// Mine - Joinlines
// ----------------------------------------------------------------------------- 

void CSegmentManager::JoinLines (void) 
{
  CSegment *seg1, *seg2; 
  double		distance, minRadius; 
  ushort		i1 [2], i2 [2];
  CVertex*	v1 [2], *v2 [2]; 
  short		match [2]; 
  short		i, j; 

if (tunnelMaker.Active ())
	return; 

if (selections [0].m_nSegment == selections [1].m_nSegment) {
	ErrorMsg ("You cannot joint two lines on the same segment.\n\n"
				"Hint: The two green lines represent the current line, \n"
				"and the 'other' segment's current line.  Press 'L' to change\n"
				"the current line or press the space bar to switch to the other segment."); 
	return;
	}

other = &selections [*current == selections [0]];
seg1 = Segment (current->SegmentId ()); 
seg2 = Segment (other->m_nSegment); 

for (i = 0; i < 2; i++) {
	i1 [i] = seg1->VertexId (current->SideId (), current->Edge () + i);
	i2 [i] = seg2->VertexId (other->m_nSide, other->Edge () + i);
	v1 [i] = seg1->Vertex (current->SideId (), current->Edge () + i);
	v2 [i] = seg2->Vertex (other->m_nSide, other->Edge () + i);
	match [i] = -1; 
	}

// make sure verts are different
if ((i1 [0] == i2 [0]) || (i1 [0] == i2 [1]) || (i1 [1] == i2 [0]) || (i1 [1] == i2 [1])) {
	ErrorMsg ("Some or all of these points are already joined."); 
	return; 
	}

// find closest for each point for each corner
for (i = 0; i < 2; i++) {
	minRadius = MAX_JOIN_DISTANCE; 
	for (j = 0; j < 2; j++) {
		distance = Distance (*v1 [i], *v2 [j]);
		if (distance < minRadius) {
			minRadius = distance; 
			match [i] = j;  // remember which vertex it matched
			}
		}
	}

// make sure there are distances are close enough
if ((match [0] < 0) || (match [1] < 0)) {
	ErrorMsg ("Lines are too far apart to join"); 
	return; 
	}

if (QueryMsg("Are you sure you want to join the current line\n"
				 "with the 'other' segment's current line?") != IDYES)
	return; 

// make sure there are matches for each and they are unique
if (match [0] == match [1]) {
	match [0] = 1; 
	match [1] = 0; 
	}
undoManager.Begin (__FUNCTION__, udSegments);
// define vert numbers
for (i = 0; i < 2; i++) 
	seg1->SetVertexId (current->SideId (), current->Edge () + i, i2 [match [i]]); 
FixChildren (); 
undoManager.End (__FUNCTION__);
SetLinesToDraw (); 
DLE.MineView ()->Refresh ();
}

// ----------------------------------------------------------------------------- 
// ----------------------------------------------------------------------------- 
// ----------------------------------------------------------------------------- 

class CSideMatcher {
	public:
		CSideKey		m_keys [2];
		CSideKey		m_bestKey;
		CVertex*		m_vertices [2][4];
		short			m_nVertices;
		tVertMatch	m_match [4];
		tVertMatch	m_bestMatch [4];
		short			m_nPoint;
		double		m_minRadius;

		bool SetSide (int nSide, CSideKey key) {
			m_keys [nSide] = key;
			CSegment* pSegment = segmentManager.Segment (key.m_nSegment);
			short nVertices = pSegment->Side (key.m_nSide)->VertexCount ();
			if (nSide == 0) {
				m_nVertices = nVertices;
				m_minRadius = 1e30;
				m_bestKey = CSideKey (-1, -1);
				}
			else if (m_nVertices != nVertices)
				return false;
			for (int i = 0; i < m_nVertices; i++) 
				m_vertices [nSide][i] = pSegment->Vertex (key.m_nSide, i);
			return true;
			}

		short Match (double maxRadius = 1e30) {
			bool fail = false;
			int nMatches = 0;
			for (short direction = 0; direction < 2; direction++) {
				for (short offset = 0; offset < m_nVertices; offset++) { // try all orientations of current side towards candidate side
					short i;
					// Check that no pair of points in the candidate join is too far apart
					for (i = 0; i < m_nVertices; i++) {
						short j = (offset + (direction ? m_nVertices - i : i)) % m_nVertices;
						m_match [i].v = j; 
						m_match [i].d = Distance (*m_vertices [0][i], *m_vertices [1][j]);
						if (m_match [i].d > maxRadius)
							break;
						}
					if (i < m_nVertices)
						continue;
					// Check if each line segment intersects an opposing side. If it does, skip, this would create a degenerate segment
					for (i = 0; i < m_nVertices; i++) {
						if (DoesLineFromPointIntersectSides (i))
							break;
						}
					if (i < m_nVertices)
						continue;
					double r = 0.0;
					for (i = 0; i < m_nVertices; i++)
						r += m_match [i].d * m_match [i].d;
					if (m_minRadius > r) {
						m_minRadius = r;
						m_bestKey = m_keys [1]; 
						memcpy (m_bestMatch, m_match, sizeof (m_match));
						m_nPoint = offset;
						if (m_minRadius == 0.0) 
							return -1;
						++nMatches;
						}
					}
				}
			return nMatches;
			}

		bool GetBestMatch (CSideKey& key, short& point, tVertMatch* match) {
			if (m_bestKey.m_nSegment < 0)
				return false;
			key = m_bestKey;
			point = m_nPoint;
			memcpy (match, m_bestMatch, sizeof (m_bestMatch));
			return true;
			}

		double MinDistance (void) {
			double d = 1e30;
			for (int i = 0; i < m_nVertices; i++)
				if (d > m_bestMatch [i].d)
					d = m_bestMatch [i].d;
			return d;
			}

		double MaxDistance (void) {
			double d = 0.0;
			for (int i = 0; i < m_nVertices; i++)
				if (d < m_bestMatch [i].d)
					d = m_bestMatch [i].d;
			return d;
			}

		bool DoesLineFromPointIntersectSides (short point) {
			if (m_nVertices <= 2)
				return false;
			if (m_match [point].v < 0 || m_match [point].v >= m_nVertices)
				return false;
			short adjacent1 = (point + 1) % m_nVertices;
			if (m_match [adjacent1].v < 0 || m_match [adjacent1].v >= m_nVertices)
				return false;
			short opposite = (point + 2) % m_nVertices;
			if (m_match [opposite].v < 0 || m_match [opposite].v >= m_nVertices)
				return false;

			// Not a perfectly accurate calculation of the triangles we will be generating, but close enough to avoid twisted cubes
			CDoubleVector intersection;
			// Side 1 triangle 1
			CDoubleVector vPlane = *m_vertices [1][m_match [adjacent1].v] - *m_vertices [0][adjacent1];
			CDoubleVector vNormal = CrossProduct (*m_vertices [0][opposite] - *m_vertices [0][adjacent1], vPlane);
			vNormal.Normalize ();
			if (PlaneLineIntersection (intersection, m_vertices [0][adjacent1], &vNormal, m_vertices [0][point], m_vertices [1][m_match [point].v]) > 0)
				return true;
			// Side 1 triangle 2
			vPlane = *m_vertices [1][m_match [opposite].v] - *m_vertices [0][opposite];
			vNormal = CrossProduct (*m_vertices [0][opposite] - *m_vertices [0][adjacent1], vPlane);
			vNormal.Normalize ();
			if (PlaneLineIntersection (intersection, m_vertices [0][opposite], &vNormal, m_vertices [0][point], m_vertices [1][m_match [point].v]) > 0)
				return true;

			// Triangular cross-sections don't have the extra side
			if (m_nVertices > 3) {
				short adjacent2 = (point + 3) % m_nVertices;
				if (m_match [adjacent2].v < 0 || m_match [adjacent2].v >= m_nVertices)
					return false;
				// Side 2 triangle 1
				vNormal = CrossProduct (*m_vertices [0][adjacent2] - *m_vertices [0][opposite], vPlane);
				vNormal.Normalize ();
				if (PlaneLineIntersection (intersection, m_vertices [0][opposite], &vNormal, m_vertices [0][point], m_vertices [1][m_match [point].v]) > 0)
					return true;
				// Side 2 triangle 2
				vPlane = *m_vertices [1][m_match [adjacent2].v] - *m_vertices [0][adjacent2];
				vNormal = CrossProduct (*m_vertices [0][adjacent2] - *m_vertices [0][opposite], vPlane);
				vNormal.Normalize ();
				if (PlaneLineIntersection (intersection, m_vertices [0][adjacent2], &vNormal, m_vertices [0][point], m_vertices [1][m_match [point].v]) > 0)
					return true;
				}

			return false;
			}
	};


// ----------------------------------------------------------------------------- 
// ----------------------------------------------------------------------------- 
// ----------------------------------------------------------------------------- 

bool CSegmentManager::FindNearbySide (CSideKey thisKey, CSideKey& otherKey, short& thisPoint, short& otherPoint, tVertMatch* match)
{
thisPoint = (thisKey == *current) ? current->Point () : 0;
otherKey.m_nSegment = -1;
// find first segment (other than this segment) which shares all points
// of the current side (points must be < 5.0 away)

	CSideMatcher sideMatcher;

	sideMatcher.SetSide (0, thisKey);

int nSegments = segmentManager.Count ();
for (int nSegment = 0; nSegment < nSegments; nSegment++) {
	if (nSegment == thisKey.m_nSegment)
		continue; 
	for (short nSide = 0; nSide < 6; nSide++) {
#ifdef _DEBUG
		if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (nSide == nDbgSide)))
			nDbgSeg = nDbgSeg;
#endif
		// Skip sides that are already joined to another segment
		if (Segment (nSegment)->ChildId (nSide) != -1)
			continue;
		if (!sideMatcher.SetSide (1, CSideKey (nSegment, nSide)))
			continue;
		short nMatches = sideMatcher.Match (10.0);
		if (nMatches < 0) 
			return sideMatcher.GetBestMatch (otherKey, otherPoint, match);
		}
	}

if (sideMatcher.GetBestMatch (otherKey, otherPoint, match))
	return true;
if (!DLE.ExpertMode ())
	ErrorMsg ("Could not find any other segment's side that is within\n10.0 units from the current side"); 
return false; 
}

// ----------------------------------------------------------------------------- 
//	Join current side with other side (!bFind) or with closest side of any other 
// segment (bFind).
// ----------------------------------------------------------------------------- 

void CSegmentManager::Join (CSideKey thisKey, bool bFind)
{
	CSideMatcher sideMatcher;
	tVertMatch	match [4]; 
	CSideKey		otherKey;
	short			thisPoint, otherPoint;
	bool			bJoin;

if (tunnelMaker.Active ()) 
	return; 
if (Segment (thisKey.m_nSegment)->ChildId (thisKey.m_nSide) != -1) {
	if (!DLE.ExpertMode ())
		ErrorMsg ("The current side is already joined to another segment"); 
	return; 
	}

// figure out 'other' segment
if (bFind) {
	if (!FindNearbySide (thisKey, otherKey, thisPoint, otherPoint, match))
		return;
	bJoin = true;
	}
else {
	otherKey = *other; 
	otherPoint = other->Point ();
	thisPoint = current->Point ();
	if (thisKey.m_nSegment == otherKey.m_nSegment) {
		if (!DLE.ExpertMode ())
			ErrorMsg ("You cannot join two sides on the same segment.\n\n"
						 "Hint: The two red squares represent the current side, \n"
						 "and the 'other' segment's current side.  Press 'S' to change\n"
						 "the current side or press the space bar to switch to the other segment."); 
		return; 
		}
	else if (Segment (otherKey)->ChildId (otherKey.m_nSide) != -1) {
		if (!DLE.ExpertMode ())
			ErrorMsg ("The selected side of the 'other' segment\n"
						"is already joined to another segment.");
		return;
		}

	sideMatcher.SetSide (0, thisKey);
	sideMatcher.SetSide (1, otherKey);
	sideMatcher.Match (double (MAX_JOIN_DISTANCE));
	if (!sideMatcher.GetBestMatch (otherKey, otherPoint, match)) {
		if (!DLE.ExpertMode ())
			ErrorMsg ("Sides are too far apart to join.\n\n"
						 "Hint: Segment edge lengths should not exceed 400 unit in any dimension\n"
						 "or they will distort when viewed from close up."); 
		return; 
		}
	bJoin = (sideMatcher.MinDistance () < 1.0) || (sideMatcher.MaxDistance () <= 5.0);
	}

// if segments are too close to put a new segment between them, then link them together without asking
if (bJoin) {
	undoManager.Begin (__FUNCTION__, udSegments | udVertices);
	LinkSides (thisKey.m_nSegment, thisKey.m_nSide, otherKey.m_nSegment, otherKey.m_nSide, match); 
	undoManager.End (__FUNCTION__);
	SetLinesToDraw (); 
	DLE.MineView ()->Refresh ();
	return; 
	}

if (QueryMsg ("Do you want to create a new segment which\n"
				  "connects the current side with the 'other' side?\n\n"
				  "Hint: Make sure you have the current point of each segment\n"
				  "on the corners you want to be connected.\n"
				  "(the 'P' key selects the current point)") != IDYES)
	return; 

undoManager.Begin (__FUNCTION__, udSegments);

thisKey.m_nSegment = Create (thisKey, EXTEND);
if (thisKey.m_nSegment < 0) {
	if (!DLE.ExpertMode ())
		ErrorMsg ("The maximum number of segments has been reached.\nCannot add any more segments."); 
	undoManager.Unroll (__FUNCTION__);
	return;
	}

// We can now re-use the old Match call results, they are more accurate when calculated from the start segment

LinkSides (thisKey.m_nSegment, thisKey.m_nSide, otherKey.m_nSegment, otherKey.m_nSide, match); 
undoManager.End (__FUNCTION__);
SetLinesToDraw(); 
DLE.MineView ()->Refresh ();
}

// ----------------------------------------------------------------------------- 
// FixChildren()
//
// Action - Updates linkage between current segment and all other Segment ()
// ----------------------------------------------------------------------------- 

void CSegmentManager::FixChildren (void)
{
short nNewSeg = current->SegmentId (); 
short nNewSide = current->SideId (); 

CSegment*	pNewSeg = Segment (nNewSeg);
CVertex*		vNewSeg = vertexManager.Vertex (pNewSeg->m_info.vertexIds [0]);

undoManager.Begin (__FUNCTION__, udSegments);
int nSegments = segmentManager.Count ();
for (int nSegment = 0; nSegment < nSegments; nSegment++) {
	if (nSegment != nNewSeg) {
		CSegment* pSegment = segmentManager.Segment (nSegment);
		// first check to see if Segment () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		CVertex* vSeg = vertexManager.Vertex (pSegment->m_info.vertexIds [0]);
		if (fabs ((double) (vNewSeg->v.x - vSeg->v.x)) < 160.0 &&
		    fabs ((double) (vNewSeg->v.y - vSeg->v.y)) < 160.0 &&
		    fabs ((double) (vNewSeg->v.z - vSeg->v.z)) < 160.0) {
			for (short nSide = 0; nSide < 6; nSide++) {
				if (!Link (nNewSeg, nNewSide, nSegment, nSide, 3)) {
					// if these Segment () were linked, then unlink them
					if ((pNewSeg->ChildId (nNewSide) == nSegment) && (pSegment->ChildId (nSide) == nNewSeg)) {
						pNewSeg->SetChild (nNewSide, -1); 
						pSegment->SetChild (nSide, -1); 
						}
					}
				}
			}
		}
	}
undoManager.End (__FUNCTION__);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp