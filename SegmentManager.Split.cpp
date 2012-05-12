// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 
// unlink_child()
//
// Action - unlinks current segment's children which don't share all four points
//
// Note: 2nd parameter "nSide" is ignored
// ----------------------------------------------------------------------------- 

void CSegmentManager::UnlinkChild (short nParentSeg, short nSide) 
{
  CSegment *parentSegP = Segment (nParentSeg); 

// loop on each side of the parent
int nChildSeg = parentSegP->ChildId (nSide); 
// does this side have a child?
if (nChildSeg < 0 || nChildSeg >= Count ())
	return;
CSegment *childSegP = Segment (nChildSeg); 
// yes, see if child has a side which points to the parent
int nChildSide;
for (nChildSide = 0; nChildSide < 6; nChildSide++)
	if (childSegP->ChildId (nChildSide) == nParentSeg) 
		break; 
// if we found the matching side
if (nChildSide < 6) {
// define vert numbers for comparison
	ushort* vertexIds = parentSegP->m_info.vertexIds;
	CSide* sideP = parentSegP->Side (nSide), * childSideP = childSegP->Side (nChildSide);
	int nVertices = sideP->VertexCount (), nMatch = 0;
	for (int i = 0; i < nVertices; i++) {
		ushort v = childSegP->VertexId (nChildSide, i); 
		for (int j = 0; j < nVertices; j++) {
			if (vertexIds [sideP->VertexIdIndex (j)] == v) {
				++nMatch;
				break;
				}
			}
		}
	if (nMatch != nVertices) { // they don't share all four points correctly, so unlink the child from the parent and unlink the parent from the child
		undoManager.Begin (udSegments);
		ResetSide (nChildSeg, nChildSide); 
		ResetSide (nParentSeg, nSide); 
		undoManager.End ();
		}
	}
else {
	// if the child does not point to the parent, 
	// then just unlink the parent from the child
	ResetSide (nParentSeg, nSide); 
	}
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::IsPointOfSide (CSegment *segP, int nSide, int nPoint)
{
CSide* sideP = segP->Side (nSide);
for (int i = 0, j = sideP->VertexCount (); i < j; i++)
	if (sideP->m_vertexIdIndex [i] == nPoint)
		return true;
return false;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::IsLineOfSide (CSegment *segP, int nSide, int nLine)
{
for (int i = 0; i < 2; i++)
	if (!IsPointOfSide (segP, nSide, edgeVertexTable [nLine][i]))
		return false;
return true;
}

// ----------------------------------------------------------------------------- 
//                          Splitpoints()
//
// Action - Splits one point shared between two cubes into two points.
//          New point is added to current segment, other segment is left alone.
//
// ----------------------------------------------------------------------------- 

int CSegmentManager::SeparatePoints (CSideKey key, int nVertexId, bool bVerbose) 
{
ushort nSegment, nVertex; 

if (tunnelMaker.Active ())
	return -1; 

current->Get (key);

if (vertexManager.Count () > (MAX_VERTICES - 1)) {
	if (bVerbose)
		ErrorMsg ("Cannot unjoin these points because the\nmaximum number of points is reached."); 
	return -1; 
	}

CSegment* segP = Segment (key.m_nSegment); 
if (nVertexId < 0)
	nVertexId = current->VertexId (); 

// check to see if current point is shared by any other cubes
bool found = false; 
segP = Segment (0);
for (nSegment = 0; (nSegment < Count ()) && !found; nSegment++, segP++)
	if (nSegment != key.m_nSegment)
		for (nVertex = 0; nVertex < 8; nVertex++)
			if (segP->m_info.vertexIds [nVertex] == nVertexId) {
				found = true; 
				break; 
				}
if (!found) {
	if (bVerbose)
		ErrorMsg ("This point is not joined with any other point."); 
	return nVertexId; 
	}

if (bVerbose && (QueryMsg ("Are you sure you want to unjoin this point?") != IDYES))
	return nVertexId; 

undoManager.Begin (udSegments | udVertices);
memcpy (vertexManager.Vertex (vertexManager.Count ()), vertexManager.Vertex (nVertexId), sizeof (*vertexManager.Vertex (0)));
// replace existing point with new point
segP = Segment (key.m_nSegment); 
int nPoint = segP->UpdateVertexId (nVertexId, vertexManager.Count ()); 
segP->Unmark (); 

// update total number of vertices
for (short nSide = 0; nSide < 6; nSide++) {
	CSideKey back;
	if (IsPointOfSide (segP, nSide, nPoint) && BackSide (CSideKey (key.m_nSegment, nSide), back)) {
		UnlinkChild (back.m_nSegment, back.m_nSide);
		UnlinkChild (key.m_nSegment, nSide); 
		}
	}	

vertexManager.Status (vertexManager.Count ()++) = 0; 

SetLinesToDraw (); 
undoManager.End ();
DLE.MineView ()->Refresh ();
if (bVerbose)
	INFOMSG ("A new point was made for the current point."); 
return vertexManager.Count () - 1;
}

// ----------------------------------------------------------------------------- 
//                         Splitlines()
//
// Action - Splits common lines of two cubes into two lines.
//
// ----------------------------------------------------------------------------- 

void CSegmentManager::SeparateLines (short nLine) 
{
if (tunnelMaker.Active ()) 
	return; 

if (vertexManager.Count () > (MAX_VERTICES - 2)) {
	if (!DLE.ExpertMode ())
		ErrorMsg ("Cannot unjoin these lines because\nthere are not enought points left."); 
	return; 
	}
if (nLine < 0)
	nLine = current->m_nLine;

	CSegment*	segP = current->Segment (); 
	CSide*		sideP = current->Side ();
	ubyte			vertexIdIndex [2] = {sideP->VertexIdIndex (nLine), sideP->VertexIdIndex (nLine + 1)};
	ushort		nEdgeVerts [2]; 
	bool			bShared [2]; 
	int			i;

for (i = 0; i < 2; i++) {
	nEdgeVerts [i] = segP->m_info.vertexIds [vertexIdIndex [i]];
	bShared [i] = VertexInUse (nEdgeVerts [i]); 
	}
if (!(bShared [0] && bShared [1])) {
	if (!DLE.ExpertMode ())
		ErrorMsg ("One or both of these points are not joined with any other points."); 
	return; 
	}

if (QueryMsg ("Are you sure you want to unjoin this line?") != IDYES)
	return; 

undoManager.Begin (udSegments | udVertices);
// create new points (copy of other vertices)
for (i = 0; i < 2; i++)
	if (bShared [i]) {
		memcpy (vertexManager.Vertex (vertexManager.Count ()), vertexManager.Vertex (nEdgeVerts [i]), sizeof (*vertexManager.Vertex (0)));
		// replace existing point with new point
		segP->m_info.vertexIds [vertexIdIndex [i]] = vertexManager.Count (); 
		segP->Unmark (); 
		vertexManager.Status (vertexManager.Count ()++) = 0; 
		}
// this code will unlink all adjacement segments that share this edge, too
for (short nSide = 0; nSide < 6; nSide++) {
	CSideKey back, key (current->m_nSegment, nSide);
	if (segP->HasEdge (nSide, nEdgeVerts [0], nEdgeVerts [1]) && BackSide (key, back)) {
		UnlinkChild (back.m_nSegment, back.m_nSide);
		UnlinkChild (current->m_nSegment, nSide); 
		}
	}

SetLinesToDraw(); 
undoManager.End ();
DLE.MineView ()->Refresh ();
INFOMSG ("Two new points were made for the current line."); 
}

// ----------------------------------------------------------------------------- 
//                       Splitsegments()
//
// ACTION - Splits a segment from all other points which share its coordinates
//
//  Changes - Added option to make thin side
// If solidify == 1, the side will keep any points it has in common with other
// sides, unless one or more of its vertices are already solitaire, in which
// case the side needs to get disconnected from its child anyway because that 
// constitutes an error in the level structure.
// ----------------------------------------------------------------------------- 

bool CSegmentManager::SeparateSegments (int bSolidify, int nSide, bool bVerbose) 
{
if (tunnelMaker.Active ())
	return false; 

	CSegment* segP = current->Segment (); 

int nChildSeg = segP->ChildId (nSide); 
if (nChildSeg == -1) {
	ErrorMsg ("The current side is not connected to another segment"); 
	return false; 
	}
if (nSide < 0)
	nSide = current->m_nSide;

	CSide*	sideP = segP->Side (nSide);
	int		nVertices = sideP->VertexCount ();
	ushort	nFaceVerts [4];
	bool		bShared [4];
	int		i, nShared = 0;

// check to see if current points are shared by any other cubes
for (i = 0; i < nVertices; i++) {
	nFaceVerts [i] = segP->m_info.vertexIds [sideP->VertexIdIndex (i)]; 
	if ((bShared [i] = VertexInUse (nFaceVerts [i])))
		++nShared;
	}
// If a side has one or more solitary points but has a child, there is 
// something wrong. However, nothing speaks against completely unjoining it.
// In fact, this does even cure the problem. So, no error message.
//	ErrorMsg ("One or more of these points are not joined with any other points."); 
//	return; 

if (!bSolidify && (vertexManager.Count () > (MAX_VERTICES - nShared))) {
	ErrorMsg ("Cannot unjoin this side because\nthere are not enough vertices left."); 
	return false; 
	}

if (bVerbose && (QueryMsg ("Are you sure you want to unjoin this side?") != IDYES))
	return false; 

undoManager.Begin (udSegments | udVertices);
segP = Segment (current->m_nSegment); 
if (nShared < nVertices)
	bSolidify = 0;
if (!bSolidify) {
	// create new points (copy of other vertices)
	for (i = 0; i < nVertices; i++) {
		if (bShared [i]) {
			memcpy (vertexManager.Vertex (vertexManager.Count ()), vertexManager.Vertex (nFaceVerts [i]), sizeof (*vertexManager.Vertex (0)));
			// replace existing points with new points
			segP->m_info.vertexIds [sideP->VertexIdIndex (i)] = vertexManager.Count (); 
			segP->Unmark (); 
			vertexManager.Status (vertexManager.Count ()++) = 0; 
			}
		}
	ubyte nOppSide = oppSideTable [nSide];
	for (int nSide = 0; nSide < 6; nSide++)
		if (nSide != nOppSide)
			UnlinkChild (current->m_nSegment, nSide); 
	SetLinesToDraw (); 
	INFOMSG (" Four new points were made for the current side."); 
	}
else {
	// does this side have a child?
	CSegment *childSegP = Segment (nChildSeg); 
	// yes, see if child has a side which points to the parent
	int nChildSide;
	for (nChildSide = 0; nChildSide < 6; nChildSide++)
		if (childSegP->ChildId (nChildSide) == current->m_nSegment) 
			break; 
	// if we bShared the matching side
	if (nChildSide < 6)
		ResetSide (nChildSeg, nChildSide); 
	ResetSide (current->m_nSegment, current->m_nSide); 
	SetLinesToDraw (); 
	}
undoManager.End ();
DLE.MineView ()->Refresh ();
return true;
}

// -----------------------------------------------------------------------------
// Splits a segment so that it contains a small segment inside that is surrounded
// by 6 (more or less) conical segments

bool CSegmentManager::SplitIn7 (void)
{
	CSegment*	centerSegP = current->Segment (), *segP, *childSegP;
	short			nCenterSeg = segmentManager.Index (centerSegP);
	short			nNewSegs [6];
	ushort		nNewVerts [8];
	CVertex		segCenter;
	bool			bVertDone [8];

if (centerSegP->m_nShape) {
	ErrorMsg ("Cannot split segments with triangular faces."); 
	return false;
	}

if (Count () >= MAX_SEGMENTS - 6) {
	ErrorMsg ("Cannot split this segment because\nthe maximum number of segments would be exceeded."); 
	return false;
	}
undoManager.Begin (udSegments | udVertices);
//h = vertexManager.Count ();
// compute segment center
vertexManager.Add (nNewVerts, 8);
CalcCenter (segCenter, Index (centerSegP));
// add center segment
// compute center segment vertices
memset (bVertDone, 0, sizeof (bVertDone));

CSide* sideP = centerSegP->Side (0);
for (short nSide = 0; nSide < 6; nSide++, sideP++) {
	for (ushort nVertex = 0; nVertex < 4; nVertex++) {
		short j = sideP->m_vertexIdIndex [nVertex];
		if (bVertDone [j])
			continue;
		bVertDone [j] = true;
		*vertexManager.Vertex (nNewVerts [j]) = Average (*centerSegP->Vertex (j), segCenter);
		}
	}

// create the surrounding segments
sideP = centerSegP->Side (0);
for (short nSide = 0; nSide < 6; nSide++) {
	nNewSegs [nSide] = Add ();
	short nSegment = nNewSegs [nSide];
	CSegment* segP = Segment (nSegment);
	short nOppSide = oppSideTable [nSide];
	for (ushort nVertex = 0; nVertex < 4; nVertex++) {
		ushort j, i = sideP->m_vertexIdIndex [nVertex];
		segP->m_info.vertexIds [i] = centerSegP->m_info.vertexIds [i];
		if ((nSide & 1) || (nSide >= 4)) {
			i = edgeVertexTable [sideEdgeTable [nSide][0]][0];
			j = edgeVertexTable [sideEdgeTable [nOppSide][2]][0];
			segP->m_info.vertexIds [j] = nNewVerts [i];
			i = edgeVertexTable [sideEdgeTable [nSide][0]][1];
			j = edgeVertexTable [sideEdgeTable [nOppSide][2]][1];
			segP->m_info.vertexIds [j] = nNewVerts [i];
			i = edgeVertexTable [sideEdgeTable [nSide][2]][0];
			j = edgeVertexTable [sideEdgeTable [nOppSide][0]][0];
			segP->m_info.vertexIds [j] = nNewVerts [i];
			i = edgeVertexTable [sideEdgeTable [nSide][2]][1];
			j = edgeVertexTable [sideEdgeTable [nOppSide][0]][1];
			segP->m_info.vertexIds [j] = nNewVerts [i];
			}
		else {
			i = edgeVertexTable [sideEdgeTable [nSide][0]][0];
			j = edgeVertexTable [sideEdgeTable [nOppSide][2]][1];
			segP->m_info.vertexIds [j] = nNewVerts [i];
			i = edgeVertexTable [sideEdgeTable [nSide][0]][1];
			j = edgeVertexTable [sideEdgeTable [nOppSide][2]][0];
			segP->m_info.vertexIds [j] = nNewVerts [i];
			i = edgeVertexTable [sideEdgeTable [nSide][2]][0];
			j = edgeVertexTable [sideEdgeTable [nOppSide][0]][1];
			segP->m_info.vertexIds [j] = nNewVerts [i];
			i = edgeVertexTable [sideEdgeTable [nSide][2]][1];
			j = edgeVertexTable [sideEdgeTable [nOppSide][0]][0];
			segP->m_info.vertexIds [j] = nNewVerts [i];
			}
		}
	segP->Setup ();
	if ((segP->SetChild (nSide, centerSegP->ChildId (nSide))) > -1) 
		Segment (segP->ChildId (nSide))->ReplaceChild (nCenterSeg, nSegment);
	segP->SetChild (nOppSide, nCenterSeg);
	centerSegP->SetChild (nSide, nSegment);
	CWall* wallP = centerSegP->m_sides [nSide].Wall ();
	if (wallP == null)
		segP->m_sides [nSide].m_info.nWall = NO_WALL;
	else {
		segP->m_sides [nSide].m_info.nWall = wallManager.Index (wallP);
		wallP->m_nSegment = nSegment;
		centerSegP->m_sides [nSide].m_info.nWall = NO_WALL;
		}
	}
// relocate center segment vertex indices
memset (bVertDone, 0, sizeof (bVertDone));
sideP = centerSegP->Side (0);
for (short nSide = 0; nSide < 6; nSide++) {
	for (ushort nVertex = 0; nVertex < 4; nVertex++) {
		ushort i = sideP->m_vertexIdIndex [nVertex];
		if (bVertDone [i])
			continue;
		bVertDone [i] = true;
		centerSegP->m_info.vertexIds [i] = nNewVerts [i];
		}
	}
// join adjacent sides of the segments surrounding the center segment
// don't process 6th segment as this is handled by processing the 1st one already
for (short nSegment = 0; nSegment < 5; nSegment++) {
	segP = Segment (nNewSegs [nSegment]);
	CSide* sideP = segP->Side (0);
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		if (segP->ChildId (nSide) >= 0)
			continue;
		for (short nChildSeg = nSegment + 1; nChildSeg < 6; nChildSeg++) {
			childSegP = Segment (nNewSegs [nChildSeg]);
			CSide* childSideP = childSegP->Side (0);
			for (short nChildSide = 0; nChildSide < 6; nChildSide++, childSideP++) {
				if (childSegP->ChildId (nChildSide) >= 0)
					continue;
				short h = 0;
				for (short i = 0; i < 4; i++) {
					ushort k = segP->m_info.vertexIds [sideP->VertexIdIndex (i)];
					for (short j = 0; j < 4; j++) {
						if (k == childSegP->m_info.vertexIds [childSideP->m_vertexIdIndex[j]]) {
							h++;
							break;
							}
						}
					}
				if (h == 4) {
					segP->SetChild (nSide, nNewSegs [nChildSeg]);
					childSegP->SetChild (nChildSide, nNewSegs [nSegment]);
					break;
					}
				}
			}
		}
	}

undoManager.End ();
DLE.MineView ()->Refresh ();
return true;
}

// -----------------------------------------------------------------------------

static inline short EdgeIndex (short nVertex1, short nVertex2)
{
for (short i = 0; i < 12; i++) {
	if ((edgeVertexTable [i][0] == nVertex1) && (edgeVertexTable [i][1] == nVertex2))
		return i + 1;
	if ((edgeVertexTable [i][1] == nVertex1) && (edgeVertexTable [i][0] == nVertex2))
		return -i - 1;
	}
return 0;
}

// -----------------------------------------------------------------------------

static inline short FindSidePoint (ubyte nPoint, ubyte* points)
{
for (short i = 0; i < 4; i++)
	if (points [i] == nPoint)
		return i;
return -1;
}

// -----------------------------------------------------------------------------
// search through all sides of a segment and look for the one having the three
// corners nPoint0, nPoint1 and nPoint2. Return the side's fourth corner's index.

static short FindCornerByPoints (ubyte nPoint0, ubyte nPoint1, ubyte nPoint2, short& nSide)
{
	ubyte refPoints [3] = { nPoint0, nPoint1, nPoint2 };

for (nSide = 0; nSide < 6; nSide++) {
	ubyte sidePoints [6];
	memcpy (sidePoints, sideVertexTable [nSide], 6 * sizeof (sidePoints [0]));
	short i, j;
	for (i = 0; i < 3; i++) {
		j = FindSidePoint (refPoints [i], sidePoints);
		if (j < 0)
			break; // not found
		sidePoints [j] = 255; // remove and mark as found
		}
	if (j >= 0) {
		for (i = 0; i < 4; i++)
			if (sidePoints [i] < 255)
				return (short) sidePoints [i];
		}
	}
return -1;
}

// -----------------------------------------------------------------------------
// Splits a segment into 8 smaller segments

bool CSegmentManager::SplitIn8 (CSegment* rootSegP)
{
if (!rootSegP)
	rootSegP = current->Segment ();

if (rootSegP->m_nShape) {
	ErrorMsg ("Cannot split wedge and pyramid shaped segments."); 
	return false;
	}

	CSegment		rootSeg = *rootSegP;
	short			nRootSeg = segmentManager.Index (rootSegP);
	short			nSplitSegs [8];
	ushort		nNewVerts [19]; // 0: root segment center, 1 - 6: side centers, 7 - 18: edge centers
	CVertex		segCenter;
	short			nOldSegments = Count ();

if (nOldSegments >= MAX_SEGMENTS - 7) {
	ErrorMsg ("Cannot split this segment because\nthe maximum number of segments would be exceeded."); 
	return false;
	}
undoManager.Begin (udSegments | udVertices | udWalls);
//h = vertexManager.Count ();
// compute segment center
if (!vertexManager.Add (nNewVerts, 19)) {
	undoManager.Unroll ();
	ErrorMsg ("Cannot split this segment because\nthe maximum number of vertices would be exceeded."); 
	return false;
	}
CalcCenter (segCenter, Index (rootSegP));
// compute side center vertices
*vertexManager.Vertex (nNewVerts [0]) = segCenter;
short j = 1;
for (short nSide = 0; nSide < 6; nSide++) {
	CVertex vCenter;
	for (ushort nVertex = 0; nVertex < 4; nVertex++) 
		vCenter += *rootSegP->Vertex (rootSegP->Side (nSide)->m_vertexIdIndex [nVertex]);
	*vertexManager.Vertex (nNewVerts [j++]) = vCenter / 4;
	}

CSegment* childSegP;
for (short nSide = 0; nSide < 6; nSide++)
	if ((childSegP = rootSegP->Child (nSide))) {
		childSegP->RemoveChild (nRootSeg);
		rootSegP->SetChild (nSide, -1);
		}

for (short nLine = 0; nLine < 12; nLine++)
	*vertexManager.Vertex (nNewVerts [j++]) = Average (*rootSegP->Vertex (edgeVertexTable [nLine][0]), *rootSegP->Vertex (edgeVertexTable [nLine][1]));
for (j = 0; j < 19; j++)
	vertexManager.Status (nNewVerts [j]) |= MARKED_MASK; 

// create the surrounding segments

// compute the vertices of the new segments. Start with the vertex of the corner a segment sits in.
// Use that corner to determine the adjacent vertices and compute three of the new segment's corners
// using the corner vertex and the adjacent vertices.
for (ubyte nCorner = 0; nCorner < 8; nCorner++) {
	nSplitSegs [nCorner] = (nCorner == 7) ? nRootSeg : Add ();
	short nSegment = nSplitSegs [nCorner];
	CSegment* segP = Segment (nSegment);
	ushort nBasePoint = rootSeg.VertexId (nCorner);
	memset (segP->m_info.vertexIds, 0xFF, sizeof (segP->m_info.vertexIds));

	ushort nVertex;
	for (short i = 0; i < 3; i++) {
		short nEdge = EdgeIndex (nCorner, adjacentPointTable [nCorner][i]);
		nVertex = (nEdge < 0) ? 0 : 1;
		nEdge = abs (nEdge) - 1;
		segP->SetVertexId (edgeVertexTable [nEdge][nVertex], nNewVerts [7 + nEdge]);
		if (!i)
			segP->SetVertexId (edgeVertexTable [nEdge][!nVertex], nBasePoint);
		}

	for (short i = 0; i < 2; i++) {
		for (short j = i + 1; j < 3; j++) {
			short nSide, nPoint = FindCornerByPoints (nCorner, adjacentPointTable [nCorner][i], adjacentPointTable [nCorner][j], nSide);
			segP->SetVertexId (nPoint, nNewVerts [1 + nSide]);
			}
		}

	short i;
	for (i = 0; i < 8; i++)
		if (segP->VertexId (i) == 0xFFFF) {
			segP->SetVertexId (i, nNewVerts [0]);
			break;
			}

	if (nSegment == nRootSeg) {
		for (short j = 0; j < 3; j++) 
			segP->Side (adjacentSideTable [i][j])->DeleteWall ();
		}
	else {
		segP->Setup ();
		segP->Function () = rootSeg.Function ();
		segP->Props () = rootSeg.Props ();
		}

	short nBaseTex, nOvlTex;
	for (short i = 0; i < 2; i++) {
		for (short j = i + 1; j < 3; j++) {
			short nSide, nPoint = FindCornerByPoints (nCorner, adjacentPointTable [nCorner][i], adjacentPointTable [nCorner][j], nSide);
			if ((nSegment != nRootSeg) && rootSeg.Side (nSide)->Wall () && !segP->Side (nSide)->Wall () && !wallManager.Full ()) {
				CSideKey key (nSegment, nSide);
				segP->Side (nSide)->m_info.nWall = wallManager.Add (false, &key);
				CWall* wallP = wallManager.Wall (segP->Side (nSide)->m_info.nWall);
				if (wallP) {
					wallP->Info () = rootSeg.Side (nSide)->Wall ()->Info ();
					wallP->Info ().nTrigger = NO_TRIGGER;
					*((CSideKey*) wallP) = key;
					}
				}
			rootSeg.Side (nSide)->GetTextures (nBaseTex, nOvlTex);
			segP->Side (nSide)->SetTextures (nBaseTex, nOvlTex);
			memcpy (segP->Uvls (nSide), rootSeg.Uvls (nSide), 4 * sizeof (CUVL));
			}
		}
	}


for (int i = 0; i < 7; i++) {
	CSegment* segP = Segment (nSplitSegs [i]);
	for (int j = i + 1; j < 8; j++) {
		short nOtherSide, nSide = segP->CommonSides (nSplitSegs [j], nOtherSide);
		if (0 <= nSide)
			segmentManager.Link (nSplitSegs [i], nSide, nSplitSegs [j], nOtherSide, 1e-3);
		}
	}

for (int i = 0; i < 8; i++) {
	CSegment* segP = Segment (nSplitSegs [i]);
	for (int j = 0; j < nOldSegments; j++) {
		ushort nVertices [4];
		short nCommon = segP->CommonVertices (j, 4, nVertices);
		if (!nCommon)
			continue;
		CSegment* otherSegP = Segment (j);
		bool bLinked = false;
		for (short nSide = 0; !bLinked && (0 <= (nSide = segP->FindSide (nSide, nCommon, nVertices))); nSide++)
			for (short nOtherSide = 0; !bLinked && (0 <= (nOtherSide = otherSegP->FindSide (nOtherSide, nCommon, nVertices))); nOtherSide++)
				bLinked = segmentManager.Link (nSplitSegs [i], nSide, j, nOtherSide, 1.0);
		}
	}

memset (&rootSeg, 0, sizeof (rootSeg)); // beware of the d'tor
undoManager.End ();
DLE.MineView ()->Refresh ();
return true;
}

// ------------------------------------------------------------------------

bool CSegmentManager::CollapseEdge (short nSegment, short nSide, short nEdge, bool bUpdateCoord)
{
#ifdef NDEBUG
if (!DLE.IsD2XLevel ()) 
	return false;
#endif

current->Get (nSegment, nSide);

	CSegment*	segP = Segment (nSegment);
	CSide*		sideP = segP->Side (nSide);
	ushort		nEdgeVerts [2]; // ids of the edge's vertices
	ubyte			nSegVerts [2]; // indices of the edge's vertices' ids in the segment's vertex id table
	short			nSideVerts [2]; // indices of the edge's vertices' ids in the side's vertex id index 

if (sideP->VertexCount() < 2)
	return false;
if (nEdge < 0)
	nEdge = current->m_nLine;

for (int i = 0; i < 2; i++) 
	nEdgeVerts [i] = segP->m_info.vertexIds [nSegVerts [i] = sideP->m_vertexIdIndex [(nEdge + i) % sideP->VertexCount ()]]; 

if (nSegVerts [0] == nSegVerts [1])
	return false;

short nSides [2] = {nSide, segP->AdjacentSide (nSide, nEdgeVerts)};
if ((segP->Side (nSides [0])->VertexCount () < 4) && (segP->Side (nSides [1])->VertexCount () < 4)) {
	if (!DLE.ExpertMode ())
		ErrorMsg ("Cannot further collapse this segment."); 
	return false;
	}

undoManager.Begin (udSegments | udVertices | udWalls);

if ((0 > (nEdgeVerts [0] = SeparatePoints (CSideKey (nSegment, nSide), nEdgeVerts [0], false))) ||
	 (0 > (nEdgeVerts [1] = SeparatePoints (CSideKey (nSegment, nSide), nEdgeVerts [1], false)))) {
	undoManager.Unroll ();
	return false;
	}

if ((sideP->VertexCount () == 3) && sideP->Wall ())
	sideP->DeleteWall ();

if (short (sideP->m_info.nChild) >= 0)
	segmentManager.Segment (sideP->m_info.nChild)->ReplaceChild (nSegment, -1);
// compute the new vertex
if (bUpdateCoord) {
	*vertexManager.Vertex (nEdgeVerts [0]) += *vertexManager.Vertex (nEdgeVerts [1]);
	*vertexManager.Vertex (nEdgeVerts [0]) /= 2;
	}

// remove the superfluous vertex from all sides adjacent to the edge and recompute the sides' texture coordinates
for (short h = 0; h < 2; h++) {
	short nSide = nSides [h];
	if (nSide < 0)
		continue;
	sideP = segP->Side (nSide);
	for (int i = 0; i < 2; i++)
		nSideVerts [i] = segP->SideVertexIndex (nSide, nSegVerts [i]);
#if 0
	if (nSideVerts [0] > nSideVerts [1])
		Swap (nSideVerts [0], nSideVerts [1]);
#endif
	if (nSideVerts [0] < 0)
		continue;
	CUVL uvl = sideP->m_info.uvls [nSideVerts [0]];
	uvl.u = (uvl.u + sideP->m_info.uvls [nSideVerts [1]].u) / 2;
	uvl.v = (uvl.v + sideP->m_info.uvls [nSideVerts [1]].v) / 2;
	uvl.l = (uvl.l + sideP->m_info.uvls [nSideVerts [1]].l) / 2;
	sideP->m_info.uvls [nSideVerts [0]] = uvl;
	sideP->SetShape (sideP->Shape () + 1);
	short h = sideP->VertexCount () - nSideVerts [1];
	if (h > 0) {
		memcpy (sideP->m_vertexIdIndex + nSideVerts [1], sideP->m_vertexIdIndex + nSideVerts [1] + 1, h);
		memcpy (sideP->m_info.uvls + nSideVerts [1], sideP->m_info.uvls + nSideVerts [1] + 1, h * sizeof (CUVL));
		}
	sideP->m_vertexIdIndex [sideP->VertexCount ()] = 0xff; //sideP->m_vertexIdIndex [nSideVerts [0]];
	}
sideP = segP->Side (0);
for (short nSide = 0; nSide < 6; nSide++, sideP++) {
	short nVertexCount = sideP->VertexCount ();
	//if ((nSide == nSides [0]) || (nSide == nSides [1]))
	//	continue;
	int j = 0;
	for (int i = 0; i < nVertexCount; i++) {
		if (sideP->m_vertexIdIndex [i] == nSegVerts [1]) 
			sideP->m_vertexIdIndex [i] = nSegVerts [0];
		if (sideP->m_vertexIdIndex [i] == nSegVerts [0]) {
			if (++j > 1) {
				if (i < --nVertexCount)
					memcpy (sideP->m_vertexIdIndex + i, sideP->m_vertexIdIndex + i + 1, nVertexCount - i);
				sideP->m_vertexIdIndex [nVertexCount] = 0xff;
				sideP->SetShape (sideP->Shape () + 1);
				break;
				}
			}
		}
	}

#if 1
segP->m_info.vertexIds [nSegVerts [1]] = 0xffff - nSegVerts [0]; // remember the vertex id index that has replaced this one => MAX_VERTICES must be <= 0xfff8!
#else
segP->UpdateVertexIdIndex (nSegVerts [1]);
if (nSegVerts [1] < 7)
	memcpy (segP->m_info.vertexIds + nSegVerts [1], segP->m_info.vertexIds + nSegVerts [1] + 1, (7 - nSegVerts [1]) * sizeof (segP->m_info.vertexIds [0]));
segP->m_info.vertexIds [7] = 0xffff;
#endif

if (bUpdateCoord)
	vertexManager.Delete (nEdgeVerts [1]);
undoManager.End ();
return true;
}

// ------------------------------------------------------------------------

bool CSegmentManager::CreateWedge (void)
{
#ifdef NDEBUG
if (!DLE.IsD2XLevel ()) 
	return false;
#endif

	CSegment* segP = current->Segment ();

if (segP->Shape () != SEGMENT_SHAPE_CUBE) {
	ErrorMsg ("Cannot turn this segment in a wedge.\nTry to collapse individual edges instead."); 
	return false;
	}

	CSide* sideP = current->Side ();
	short nSide = current->m_nSide, nOppSide = oppSideTable [current->m_nSide];

undoManager.Begin (udSegments | udVertices | udWalls);

short nLine = current->m_nLine + 1;
CollapseEdge (-1, -1, nLine % current->Side ()->VertexCount ());
if (nLine < current->Side ()->VertexCount ())
	++nLine;
CollapseEdge (-1, -1, nLine % current->Side ()->VertexCount ());
segP->SetShape (SEGMENT_SHAPE_WEDGE);
DLE.MineView ()->NextSide (1);
undoManager.End ();
return true;
}

// ------------------------------------------------------------------------

bool CSegmentManager::CreatePyramid (void)
{
#ifdef NDEBUG
if (!DLE.IsD2XLevel ()) 
	return false;
#endif

	CSegment* segP = current->Segment ();

if (segP->Shape () != SEGMENT_SHAPE_CUBE) {
	ErrorMsg ("Cannot turn this segment in a pyramid.\nTry to collapse individual edges instead."); 
	return false;
	}

	CSide* sideP = current->Side ();
	short nSide = current->m_nSide, nOppSide = oppSideTable [current->m_nSide];

undoManager.Begin (udSegments | udVertices | udWalls);

#if 0
for (int i = 0; i < 6; i++) {
	if (i == nOppSide)
		continue;
	if (segP->Child (i) < 0)
		continue;
	if (!SeparateSegments (i)) {
		undoManager.Unlock ();
		undoManager.Undo ();
		return false;
		}
	}
#endif

short nLine = current->m_nLine + 1;
CollapseEdge (-1, -1, nLine % current->Side ()->VertexCount ());
if (nLine < current->Side ()->VertexCount ())
	++nLine;
CollapseEdge (-1, -1, nLine % current->Side ()->VertexCount ());
CollapseEdge (-1, -1, 0);
segP->SetShape (SEGMENT_SHAPE_PYRAMID);
DLE.MineView ()->NextSide (1);
undoManager.End ();
return true;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp