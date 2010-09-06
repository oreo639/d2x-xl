// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 
// unlink_child()
//
// Action - unlinks current cube's children which don't share all four points
//
// Note: 2nd parameter "nSide" is ignored
// ----------------------------------------------------------------------------- 

void CSegmentManager::UnlinkChild (short nParentSeg, short nSide) 
{
  CSegment *parentSegP = Segment (nParentSeg); 

// loop on each side of the parent
//	int nSide;
//  for (nSide = 0; nSide < 6; nSide++) {
int nChildSeg = parentSegP->Child (nSide); 
// does this side have a child?
if (nChildSeg < 0 || nChildSeg >= Count ())
	return;
CSegment *child_seg = Segment (nChildSeg); 
// yes, see if child has a side which points to the parent
int nChildSide;
for (nChildSide = 0; nChildSide < 6; nChildSide++)
	if (child_seg->Child (nChildSide) == nParentSeg) break; 
// if we found the matching side
if (nChildSide < 6) {
// define vert numbers for comparison
	short pv [4], cv [4]; // (short names given for clarity)
	int i;
	for (i = 0; i < 4; i++) {
		pv [i] = parentSegP->m_info.verts [sideVertTable [nSide][i]]; // parent vert
		cv [i] = child_seg->m_info.verts [sideVertTable [nChildSide][i]]; // child vert
		}
	// if they share all four vertices..
	// note: assumes verts increase clockwise looking outward
	if ((pv [0]== cv [3] && pv [1]== cv [2] && pv [2]== cv [1] && pv [3]== cv [0]) ||
		 (pv [0]== cv [2] && pv [1]== cv [1] && pv [2]== cv [0] && pv [3]== cv [3]) ||
		 (pv [0]== cv [1] && pv [1]== cv [0] && pv [2]== cv [3] && pv [3]== cv [2]) ||
		 (pv [0]== cv [0] && pv [1]== cv [3] && pv [2]== cv [2] && pv [3]== cv [1]))
		; // they match, don't mess with them
	else {
		// otherwise, they don't share all four points correctly
		// so unlink the child from the parent
		// and unlink the parent from the child
		undoManager.SetModified (true); 
		undoManager.Lock ();
		ResetSide (nChildSeg, nChildSide); 
		ResetSide (nParentSeg, nSide); 
		undoManager.Unlock ();
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
for (int i = 0; i < 4; i++)
	if (sideVertTable [nSide][i] == nPoint)
		return true;
return false;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::IsLineOfSide (CSegment *segP, int nSide, int nLine)
{
for (int i = 0; i < 2; i++)
	if (!IsPointOfSide (segP, nSide, lineVertTable [nLine][i]))
		return false;
return true;
}

// ----------------------------------------------------------------------------- 
//                          Splitpoints()
//
// Action - Splits one point shared between two cubes into two points.
//          New point is added to current cube, other cube is left alone.
//
// ----------------------------------------------------------------------------- 

void CSegmentManager::SplitPoints (void) 
{
CSegment *segP; 
short vert, nSegment, nVertex; 
bool found; 

if (tunnelMaker.Active ())
	return; 
	
if (vertexManager.Count () > (MAX_VERTICES - 1)) {
	ErrorMsg ("Cannot unjoin these points because the\n"
				"maximum number of points is reached."); 
	return; 
	}

segP = Segment (current.m_nSegment); 
vert = segP->m_info.verts [sideVertTable [current.m_nSide][current.m_nPoint]]; 

// check to see if current point is shared by any other cubes
found = FALSE; 
segP = Segment (0);
for (nSegment = 0; (nSegment < Count ()) && !found; nSegment++, segP++)
	if (nSegment != current.m_nSegment)
		for (nVertex = 0; nVertex < 8; nVertex++)
			if (segP->m_info.verts [nVertex] == vert) {
				found = TRUE; 
				break; 
				}
if (!found) {
	ErrorMsg ("This point is not joined with any other point."); 
	return; 
	}

if (QueryMsg("Are you sure you want to unjoin this point?") != IDYES) 
	return; 

undoManager.SetModified (true); 
undoManager.Lock ();
// create a new point (copy of other vertex)
memcpy (vertexManager.Vertex (vertexManager.Count ()), vertexManager.Vertex (vert), sizeof (*vertexManager.Vertex (0)));
/*
vertexManager.Vertex (vertexManager.Count ()).x = vertexManager.Vertex (vert).x; 
vertexManager.Vertex (vertexManager.Count ()).y = vertexManager.Vertex (vert).y; 
vertexManager.Vertex (vertexManager.Count ()).z = vertexManager.Vertex (vert).z; 
*/
// replace existing point with new point
segP = Segment (current.m_nSegment); 
segP->m_info.verts [sideVertTable [current.m_nSide][current.m_nPoint]] = vertexManager.Count (); 
segP->m_info.wallFlags &= ~MARKED_MASK; 

// update total number of vertices
vertexManager.Status (vertexManager.Count ()++) = 0; 

for (short nSide = 0; nSide < 6; nSide++) {
	CSideKey opp, key (current.m_nSegment, nSide);
	if (IsPointOfSide (segP, nSide, segP->m_info.verts [sideVertTable [current.m_nSide][current.m_nPoint]]) && OppositeSide (key, opp)) {
		UnlinkChild (opp.m_nSegment, opp.m_nSide);
		UnlinkChild (current.m_nSegment, nSide); 
		}
	}	

SetLinesToDraw (); 
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
INFOMSG("A new point was made for the current point."); 
}

// ----------------------------------------------------------------------------- 
//                         Splitlines()
//
// Action - Splits common lines of two cubes into two lines.
//
// ----------------------------------------------------------------------------- 

void CSegmentManager::SplitLines (void) 
{
  CSegment *segP; 
  short vert [2], nSegment, nVertex, nLine, i; 
  bool found [2]; 

if (tunnelMaker.Active ()) 
	return; 

if (vertexManager.Count () > (MAX_VERTICES - 2)) {
	if (!bExpertMode)
		ErrorMsg ("Cannot unjoin these lines because\nthere are not enought points left."); 
	return; 
	}

segP = Segment (current.m_nSegment); 
for (i = 0; i < 2; i++) {
	nLine = sideLineTable [current.m_nSide][current.m_nLine]; 
	vert [i] = Segment (current.m_nSegment)->m_info.verts [lineVertTable [nLine][i]]; 
	// check to see if current points are shared by any other cubes
	found [i] = FALSE; 
	segP = Segment (0);
	for (nSegment = 0; (nSegment < Count ()) && !found [i]; nSegment++, segP++) {
		if (nSegment != current.m_nSegment) {
			for (nVertex = 0; nVertex < 8; nVertex++) {
				if (segP->m_info.verts [nVertex] == vert [i]) {
					found [i] = TRUE; 
					break; 
					}
				}
			}
		}
	}
if (!(found [0] && found [1])) {
	if (!bExpertMode)
		ErrorMsg ("One or both of these points are not joined with any other points."); 
	return; 
	}

if (QueryMsg ("Are you sure you want to unjoin this line?") != IDYES)
	return; 
undoManager.SetModified (true); 
undoManager.Lock ();
segP = Segment (current.m_nSegment); 
// create a new points (copy of other vertices)
for (i = 0; i < 2; i++)
	if (found [i]) {
		memcpy (vertexManager.Vertex (vertexManager.Count ()), vertexManager.Vertex (vert [i]), sizeof (*vertexManager.Vertex (0)));
		/*
		vertices [vertexManager.Count ()].x = vertices [vert [i]].x; 
		vertices [vertexManager.Count ()].y = vertices [vert [i]].y; 
		vertices [vertexManager.Count ()].z = vertices [vert [i]].z; 
		*/
		// replace existing points with new points
		nLine = sideLineTable [current.m_nSide][current.m_nLine]; 
		segP->m_info.verts [lineVertTable [nLine][i]] = vertexManager.Count (); 
		segP->m_info.wallFlags &= ~MARKED_MASK; 
		// update total number of vertices
		vertexManager.Status (vertexManager.Count ()++) = 0; 
		}
for (short nSide = 0; nSide < 6; nSide++) {
	CSideKey opp, key (current.m_nSegment, nSide);
	if (IsLineOfSide (segP, nSide, nLine) && OppositeSide (key, opp)) {
		UnlinkChild (opp.m_nSegment, opp.m_nSide);
		UnlinkChild (current.m_nSegment, nSide); 
		}
	}
SetLinesToDraw(); 
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
INFOMSG ("Two new points were made for the current line."); 
}

// ----------------------------------------------------------------------------- 
//                       Splitsegments()
//
// ACTION - Splits a cube from all other points which share its coordinates
//
//  Changes - Added option to make thin side
// If solidify == 1, the side will keep any points it has in common with other
// sides, unless one or more of its vertices are already solitaire, in which
// case the side needs to get disconnected from its child anyway because that 
// constitutes an error in the level structure.
// ----------------------------------------------------------------------------- 

void CSegmentManager::SplitSegments (int solidify, int nSide) 
{
  CSegment *segP; 
  int vert [4], nSegment, nVertex, i, nFound = 0; 
  bool found [4]; 

if (tunnelMaker.Active ())
	return; 

segP = current.Segment (); 
if (nSide < 0)
	nSide = current.m_nSide;
int nChildSeg = segP->Child (nSide); 
if (nChildSeg == -1) {
	ErrorMsg ("The current side is not connected to another cube"); 
	return; 
	}

for (i = 0; i < 4; i++)
	vert [i] = segP->m_info.verts [sideVertTable [nSide][i]]; 
	// check to see if current points are shared by any other cubes
for (nSegment = 0, segP = Segment (0); nSegment < Count (); nSegment++, segP++)
	if (nSegment != current.m_nSegment)
		for (i = 0, nFound = 0; i < 4; i++) {
			found [i] = FALSE;
			for (nVertex = 0; nVertex < 8; nVertex++)
				if (segP->m_info.verts [nVertex] == vert [i]) {
					found [i] = TRUE;
					if (++nFound == 4)
						goto found;
					}
			}
// If a side has one or more solitary points but has a child, there is 
// something wrong. However, nothing speaks against completely unjoining it.
// In fact, this does even cure the problem. So, no error message.
//	ErrorMsg ("One or more of these points are not joined with any other points."); 
//	return; 

found:

if (!solidify && (vertexManager.Count () > (MAX_VERTICES - nFound))) {
	ErrorMsg ("Cannot unjoin this side because\nthere are not enough vertices left."); 
	return; 
	}

if (QueryMsg ("Are you sure you want to unjoin this side?") != IDYES)
	return; 

undoManager.SetModified (true); 
undoManager.Lock ();
segP = Segment (current.m_nSegment); 
if (nFound < 4)
	solidify = 0;
if (!solidify) {
	// create new points (copy of other vertices)
	for (i = 0; i < 4; i++) {
		if (found [i]) {
			memcpy (vertexManager.Vertex (vertexManager.Count ()), vertexManager.Vertex (vert [i]), sizeof (*vertexManager.Vertex (0)));
			/*
			vertices [vertexManager.Count ()].x = vertices [vert [i]].x; 
			vertices [vertexManager.Count ()].y = vertices [vert [i]].y; 
			vertices [vertexManager.Count ()].z = vertices [vert [i]].z; 
			*/
			// replace existing points with new points
			segP->m_info.verts [sideVertTable [nSide][i]] = vertexManager.Count (); 
			segP->m_info.wallFlags &= ~MARKED_MASK; 

			// update total number of vertices
			vertexManager.Status (vertexManager.Count ()++) = 0; 
			}
		}
	int nSide;
	for (nSide = 0; nSide < 6; nSide++)
		if (nSide != oppSideTable [nSide])
			UnlinkChild (current.m_nSegment, nSide); 
	SetLinesToDraw(); 
	INFOMSG (" Four new points were made for the current side."); 
	}
else {
	// does this side have a child?
	CSegment *childSegP = Segment (nChildSeg); 
	// yes, see if child has a side which points to the parent
	int nChildSide;
	for (nChildSide = 0; nChildSide < 6; nChildSide++)
		if (childSegP->Child (nChildSide) == current.m_nSegment) 
			break; 
	// if we found the matching side
	if (nChildSide < 6)
		ResetSide (nChildSeg, nChildSide); 
	ResetSide (current.m_nSegment, current.m_nSide); 
	SetLinesToDraw(); 
	}
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
}

// -----------------------------------------------------------------------------

bool CSegmentManager::Split (void)
{
	CSegment*	centerSegP = current.Segment (), *segP, *childSegP;
	short			nCenterSeg = short (centerSegP - Segment (0));
	short			nSegment, nChildSeg;
	short			nSide, nOppSide, nChildSide;
	short			vertNum, nWall;
	CVertex		segCenter, *segVert, *centerSegVert;
	bool			bVertDone [8], bUndo;
	int			i, j, k;
	short			oppSides [6] = {2,3,0,1,5,4};

if (Count () >= MAX_SEGMENTS - 6) {
	ErrorMsg ("Cannot split this cube because\nthe maximum number of cubes would be exceeded."); 
	return false;
	}
bUndo = undoManager.SetModified (true); 
undoManager.Lock ();
//h = vertexManager.Count ();
// compute segment center
ushort nVertices [8];
vertexManager.Add (nVertices, 8);
CalcCenter (segCenter, Index (centerSegP));
// add center segment
// compute center segment vertices
memset (bVertDone, 0, sizeof (bVertDone));
for (nSide = 0; nSide < 6; nSide++) {
	for (vertNum = 0; vertNum < 4; vertNum++) {
		j = sideVertTable [nSide][vertNum];
		if (bVertDone [j])
			continue;
		bVertDone [j] = true;
		centerSegVert = vertexManager.Vertex (centerSegP->m_info.verts [j]);
		segVert = vertexManager.Vertex (nVertices [j]);
		*segVert = Average (*centerSegVert, segCenter);
		//centerSegP->m_info.verts [j] = h + j;
		}
	}

// create the surrounding segments
for (nSide = 0; nSide < 6; nSide++) {
	nSegment = Add ();
	segP = Segment (nSegment);
	nOppSide = oppSides [nSide];
	for (vertNum = 0; vertNum < 4; vertNum++) {
		i = sideVertTable [nSide][vertNum];
		segP->m_info.verts [i] = centerSegP->m_info.verts [i];
		if ((nSide & 1) || (nSide >= 4)) {
			i = lineVertTable [sideLineTable [nSide][0]][0];
			j = lineVertTable [sideLineTable [nOppSide][2]][0];
			segP->m_info.verts [j] = nVertices [i];
			i = lineVertTable [sideLineTable [nSide][0]][1];
			j = lineVertTable [sideLineTable [nOppSide][2]][1];
			segP->m_info.verts [j] = nVertices [i];
			i = lineVertTable [sideLineTable [nSide][2]][0];
			j = lineVertTable [sideLineTable [nOppSide][0]][0];
			segP->m_info.verts [j] = nVertices [i];
			i = lineVertTable [sideLineTable [nSide][2]][1];
			j = lineVertTable [sideLineTable [nOppSide][0]][1];
			segP->m_info.verts [j] = nVertices [i];
			}
		else {
			i = lineVertTable [sideLineTable [nSide][0]][0];
			j = lineVertTable [sideLineTable [nOppSide][2]][1];
			segP->m_info.verts [j] = nVertices [i];
			i = lineVertTable [sideLineTable [nSide][0]][1];
			j = lineVertTable [sideLineTable [nOppSide][2]][0];
			segP->m_info.verts [j] = nVertices [i];
			i = lineVertTable [sideLineTable [nSide][2]][0];
			j = lineVertTable [sideLineTable [nOppSide][0]][1];
			segP->m_info.verts [j] = nVertices [i];
			i = lineVertTable [sideLineTable [nSide][2]][1];
			j = lineVertTable [sideLineTable [nOppSide][0]][0];
			segP->m_info.verts [j] = nVertices [i];
			}
		}
	segP->Setup ();
	if ((segP->SetChild (nSide, centerSegP->Child (nSide))) > -1) {
		for (childSegP = Segment (segP->Child (nSide)), nChildSide = 0; nChildSide < 6; nChildSide++)
			if (childSegP->Child (nChildSide) == nCenterSeg) {
				childSegP->SetChild (nChildSide, nSegment);
				break;
				}
			}
	segP->SetChild (nOppSide, nCenterSeg);
	centerSegP->SetChild (nSide, nSegment);
	nWall = centerSegP->m_sides [nSide].m_info.nWall;
	segP->m_sides [nSide].m_info.nWall = nWall;
	if ((nWall >= 0) && (nWall != NO_WALL)) {
		wallManager.Wall (nWall)->m_nSegment = nSegment;
		centerSegP->m_sides [nSide].m_info.nWall = NO_WALL;
		}
	}
// relocate center segment vertex indices
memset (bVertDone, 0, sizeof (bVertDone));
for (nSide = 0; nSide < 6; nSide++) {
	for (vertNum = 0; vertNum < 4; vertNum++) {
		j = sideVertTable [nSide][vertNum];
		if (bVertDone [j])
			continue;
		bVertDone [j] = true;
		centerSegP->m_info.verts [j] = nVertices [j];
		}
	}
// join adjacent sides of the segments surrounding the center segment
for (nSegment = 0, segP = Segment (Count ()); nSegment < 5; nSegment++, segP++) {
	for (nSide = 0; nSide < 6; nSide++) {
		if (segP->Child (nSide) >= 0)
			continue;
		for (nChildSeg = nSegment + 1, childSegP = Segment (Count () + nChildSeg); 
			  nChildSeg < 6; 
			  nChildSeg++, childSegP++) {
			for (nChildSide = 0; nChildSide < 6; nChildSide++) {
				if (childSegP->Child (nChildSide)  >= 0)
					continue;
				int h = 0;
				for (i = 0; i < 4; i++) {
					k = segP->m_info.verts [sideVertTable [nSide][i]];
					for (j = 0; j < 4; j++) {
						if (k == childSegP->m_info.verts [sideVertTable [nChildSide][j]]) {
							h++;
							break;
							}
						}
					}
				if (h == 4) {
					segP->SetChild (nSide, Count () + nChildSeg);
					childSegP->SetChild (nChildSide, Count () + nSegment);
					break;
					}
				}
			}
		}
	}

undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp