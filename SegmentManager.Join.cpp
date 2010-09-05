// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

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
	CSegment		* seg1, * seg2; 
	short			i, j; 
	CVertex		v1 [4], v2 [4]; 
	short			fail;
	tVertMatch	match [4]; 

	seg1 = Segment (nSegment1); 
	seg2 = Segment (nSegment2); 

// don't link to a segment which already has a child
if (seg1->Child (nSide1) !=-1 || seg2->Child (nSide2) != -1)
	return FALSE; 

// copy vertices for comparison later (makes code more readable)
for (i = 0; i < 4; i++) {
	v1 [i] = *vertexManager.Vertex (seg1->m_info.verts [sideVertTable [nSide1][i]]);
	v2 [i] = *vertexManager.Vertex (seg2->m_info.verts [sideVertTable [nSide2][i]]);
	match [i].i = -1; 
}

// check to see if all 4 vertices match exactly one of each of the 4 other cube's vertices
fail = 0;   // assume test will pass for now
for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
		if (fabs (v1 [i].v.x - v2 [j].v.x) < margin &&
			 fabs (v1 [i].v.y - v2 [j].v.y) < margin &&
			 fabs (v1 [i].v.z - v2 [j].v.z) < margin)
			if (match [j].i != -1) // if this vertex already matched another vertex then abort
				return false; 
			else
				match [j].i = i;  // remember which vertex it matched
if (match [0].i == -1)
	return FALSE;

static int matches [][4] = {{0,3,2,1},{1,0,3,2},{2,1,0,3},{3,2,1,0}};

for (i = 1; i < 4; i++)
	if (match [i].i != matches [match [0].i][i])
		return FALSE;
// make sure verts match in the correct order
// if not failed and match found for each
LinkSides (nSegment1, nSide1, nSegment2, nSide2, match); 
return TRUE; 
}


// ----------------------------------------------------------------------------- 
// LinkSides()
// ----------------------------------------------------------------------------- 

void CSegmentManager::LinkSides (short nSegment1, short nSide1, short nSegment2, short nSide2, tVertMatch match [4]) 
{
	CSegment*	seg1 = Segment (nSegment1); 
	CSegment*	seg2 = Segment (nSegment2); 
	short			nSegment, nVertex, oldVertex, newVertex; 
	int			i; 

seg1->SetChild (nSide1, nSegment2); 
seg1->m_sides [nSide1].m_info.nBaseTex = 0; 
seg1->m_sides [nSide1].m_info.nOvlTex = 0; 
for (i = 0; i < 4; i++) 
	seg1->m_sides [nSide1].m_info.uvls [i].Clear (); 
seg2->SetChild (nSide2, nSegment1); 
seg2->m_sides [nSide2].m_info.nBaseTex = 0; 
seg2->m_sides [nSide2].m_info.nOvlTex = 0; 
for (i = 0; i < 4; i++) 
	seg2->m_sides [nSide2].m_info.uvls [i].Clear (); 

// merge vertices
for (i = 0; i < 4; i++) {
	oldVertex = seg1->m_info.verts [sideVertTable [nSide1][i]]; 
	newVertex = seg2->m_info.verts [sideVertTable [nSide2][match [i].i]]; 

	// if either vert was marked, then mark the new vert
	vertexManager.Status (newVertex) |= (vertexManager.Status (oldVertex) & MARKED_MASK); 

	// update all Segment () that use this vertex
	if (oldVertex != newVertex) {
		CSegment *segP = Segment (0);
		for (nSegment = 0; nSegment < Count (); nSegment++, segP++)
			for (nVertex = 0; nVertex < 8; nVertex++)
				if (segP->m_info.verts [nVertex] == oldVertex)
					segP->m_info.verts [nVertex] = newVertex; 
		// then delete the vertex
		vertexManager.Delete (oldVertex); 
		}
	}
}


// ----------------------------------------------------------------------------- 
// Mine - Joinpoints
// ----------------------------------------------------------------------------- 

void CSegmentManager::JoinPoints (void) 
{
  CSegment *seg1, *seg2; 
 double distance; //v1x, v1y, v1z, v2x, v2y, v2z; 
  int vert1, vert2; 
  CSelection * cur1, * cur2; 

if (tunnelMaker.Active ()) {
	ErrorMsg (spline_error_message); 
	return; 
	}
if (selections [0].m_nSegment== selections [1].m_nSegment) {
	ErrorMsg ("You cannot joint two points on the same cube.\n\n"
				"Hint: The two golden circles represent the current point, \n"
				"and the 'other' cube's current point.  Press 'P' to change the\n"
				"current point or press the space bar to switch to the other cube."); 
	return;
	}

if (current == selections [0]) {
	cur1 = &selections [0]; 
	cur2 = &selections [1]; 
	}
else {
	cur1 = &selections [1]; 
	cur2 = &selections [0]; 
	}
seg1 = Segment (cur1->m_nSegment); 
seg2 = Segment (cur2->m_nSegment); 
vert1 = seg1->m_info.verts [sideVertTable [cur1->m_nSide][cur1->m_nPoint]]; 
vert2 = seg2->m_info.verts [sideVertTable [cur2->m_nSide][cur2->m_nPoint]]; 
// make sure verts are different
if (vert1== vert2) {
	ErrorMsg ("These points are already joined."); 
	return; 
	}
// make sure there are distances are close enough
distance = Distance (*vertexManager.Vertex (vert1), *vertexManager.Vertex (vert2)); 
if (distance > JOIN_DISTANCE) {
	ErrorMsg ("Points are too far apart to join"); 
	return; 
	}
if (QueryMsg("Are you sure you want to join the current point\n"
				 "with the 'other' cube's current point?") != IDYES)
	return; 
undoManager.SetModified (true); 
undoManager.Lock ();
// define vert numbers
seg1->m_info.verts [sideVertTable [cur1->m_nSide][cur1->m_nPoint]] = vert2; 
// delete any unused vertices
//  delete_unused_vertices(); 
FixChildren (); 
SetLinesToDraw (); 
DLE.MineView ()->Refresh ();
undoManager.Unlock ();
}

// ----------------------------------------------------------------------------- 
// Mine - Joinlines
// ----------------------------------------------------------------------------- 

void CSegmentManager::JoinLines (void) 
{
  CSegment *seg1, *seg2; 
  double v1x [2], v1y [2], v1z [2], v2x [2], v2y [2], v2z [2]; 
  double distance, minRadius; 
  int v1, v2, vert1 [2], vert2 [2]; 
  short match [2]; 
  short i, j, nLine; 
  bool fail; 
  CSelection *cur1, *cur2; 

if (tunnelMaker.Active ()) {
	ErrorMsg (spline_error_message); 
	return; 
	}

if (selections [0].m_nSegment == selections [1].m_nSegment) {
	ErrorMsg ("You cannot joint two lines on the same cube.\n\n"
				"Hint: The two green lines represent the current line, \n"
				"and the 'other' cube's current line.  Press 'L' to change\n"
				"the current line or press the space bar to switch to the other cube."); 
	return;
	}

if (current == selections [0]) {
	cur1 = &selections [0]; 
	cur2 = &selections [1]; 
	} 
else {
	cur1 = &selections [1]; 
	cur2 = &selections [0]; 
	}
seg1 = Segment (cur1->m_nSegment); 
seg2 = Segment (cur2->m_nSegment); 

for (i = 0; i < 2; i++) {
	nLine = sideLineTable [cur1->m_nSide][cur1->m_nLine]; 
	v1 = vert1 [i] = seg1->m_info.verts [lineVertTable [nLine][i]]; 
	nLine = sideLineTable [cur2->m_nSide][cur2->m_nLine]; 
	v2 = vert2 [i] = seg2->m_info.verts [lineVertTable [nLine][i]]; 
	v1x [i] = vertexManager.Vertex (v1)->v.x; 
	v1y [i] = vertexManager.Vertex (v1)->v.y; 
	v1z [i] = vertexManager.Vertex (v1)->v.z; 
	v2x [i] = vertexManager.Vertex (v2)->v.x; 
	v2y [i] = vertexManager.Vertex (v2)->v.y; 
	v2z [i] = vertexManager.Vertex (v2)->v.z; 
	match [i] =-1; 
	}

// make sure verts are different
if (vert1 [0]== vert2 [0] || vert1 [0]== vert2 [1] ||
	 vert1 [1]== vert2 [0] || vert1 [1]== vert2 [1]) {
	ErrorMsg ("Some or all of these points are already joined."); 
	return; 
	}

// find closest for each point for each corner
for (i = 0; i < 2; i++) {
	minRadius = JOIN_DISTANCE; 
	for (j = 0; j < 2; j++) {
		distance = sqrt((v1x [i] - v2x [j]) * (v1x [i] - v2x [j])
					+ (v1y [i] - v2y [j]) * (v1y [i] - v2y [j])
					+ (v1z [i] - v2z [j]) * (v1z [i] - v2z [j])); 
		if (distance < minRadius) {
			minRadius = distance; 
			match [i] = j;  // remember which vertex it matched
			}
		}
	}

// make sure there are distances are close enough
if (minRadius == JOIN_DISTANCE) {
	ErrorMsg ("Lines are too far apart to join"); 
	return; 
	}

if (QueryMsg("Are you sure you want to join the current line\n"
				 "with the 'other' cube's current line?") != IDYES)
	return; 
fail = FALSE; 
// make sure there are matches for each and they are unique
fail = (match [0] == match [1]);
if (fail) {
	match [0] = 1; 
	match [1] = 0; 
	}
undoManager.SetModified (true); 
undoManager.Lock ();
// define vert numbers
for (i = 0; i < 2; i++) {
	nLine = sideLineTable [cur1->m_nSide][cur1->m_nLine]; 
	seg1->m_info.verts [lineVertTable [nLine][i]] = vert2 [match [i]]; 
	}
FixChildren(); 
SetLinesToDraw(); 
DLE.MineView ()->Refresh ();
undoManager.Unlock ();
}

// ----------------------------------------------------------------------------- 
//		       Joinsegments()
//
//  ACTION - Joins sides of current Segment ().  Finds closest corners.
//	     If sides use vertices with the same coordinates, these vertices
//	     are merged and the cube's are connected together.  Otherwise, a
//           new cube is added added.
//
//  Changes - Added option to solidifyally figure out "other cube"
// ----------------------------------------------------------------------------- 

void CSegmentManager::Join (int solidify)
{
	CSegment *segP; 
	CSegment *seg1, *seg2; 
	short h, i, j, nSide, nNewSeg, nSegment; 
	CVertex v1 [4], v2 [4]; 
	double radius, minRadius, maxRadius, totalRad, minTotalRad; 
	tVertMatch match [4]; 
	bool fail; 
	CSelection *cur1, *cur2, mySeg; 

if (tunnelMaker.Active ()) {
	ErrorMsg (spline_error_message); 
	return; 
	}

// figure out "other' cube
if (solidify) {
	if (Segment (current.m_nSegment)->Child (current.m_nSide) != -1) {
		if (!bExpertMode)
			ErrorMsg ("The current side is already joined to another cube"); 
		return; 
		}
	cur1 = &current; 
	cur2 = &mySeg; 
	mySeg.m_nSegment = -1;
	// find first cube (other than this cube) which shares all 4 points
	// of the current side (points must be < 5.0 away)
	seg1 = Segment (cur1->m_nSegment); 
	for (i = 0; i < 4; i++) {
		memcpy (&v1 [i], vertexManager.Vertex (seg1->m_info.verts [sideVertTable [cur1->m_nSide][i]]), sizeof (CVertex));
		}
	minTotalRad = 1e300;
	for (nSegment = 0, seg2 = Segment (0); nSegment < Count (); nSegment++, seg2++) {
		if (nSegment== cur1->m_nSegment)
			continue; 
		for (nSide = 0; nSide < 6; nSide++) {
			fail = FALSE; 
			for (i = 0; i < 4; i++) {
				memcpy (&v2 [i], vertexManager.Vertex (seg2->m_info.verts[sideVertTable[nSide][i]]), sizeof (CVertex));
				}
			for (i = 0; i < 4; i++)
				match [i].b = 0; 
			for (i = 0; i < 4; i++) {
				match [i].i = -1; 
				match [i].d = 1e300;
				for (j = 0, h = -1; j < 4; j++) {
					if (match [j].b)
						continue;
					radius = Distance (v1 [i], v2 [j]);
					if ((radius <= 10.0) && (radius < match [i].d)) {
						h = j;  // remember which vertex it matched
						match [i].d = radius;
						}
					}
				if (h < 0) {
					fail = TRUE;
					break;
					}
				match [i].i = h;
				match [h].b = i;
				}
			if (fail)
				continue;
			totalRad = 0;
			for (i = 0; i < 4; i++)
				totalRad += match [i].d;
			if (minTotalRad > totalRad) {
				minTotalRad = totalRad;
				mySeg.m_nSegment = nSegment; 
				mySeg.m_nSide = nSide; 
				mySeg.m_nPoint = 0; // should not be used
			// force break from loops
				if (minTotalRad == 0) {
					nSide = 6; 
					nSegment = Count (); 
					}
				}
			}
		}
	if (mySeg.m_nSegment < 0) {
		if (!bExpertMode)
			ErrorMsg ("Could not find another cube whose side is within\n"
						"10.0 units from the current side"); 
		return; 
		}
	}
else
	if (current== selections [0]) {
		cur1 = &selections [0]; 
		cur2 = &selections [1]; 
		}
	else {
		cur1 = &selections [1]; 
		cur2 = &selections [0]; 
		}

if (cur1->m_nSegment == cur2->m_nSegment) {
	if (!bExpertMode)
		ErrorMsg ("You cannot joint two sides on the same cube.\n\n"
					"Hint: The two red squares represent the current side, \n"
					"and the 'other' cube's current side.  Press 'S' to change\n"
					"the current side or press the space bar to switch to the other cube."); 
	return; 
	}

seg1 = Segment (cur1->m_nSegment); 
seg2 = Segment (cur2->m_nSegment); 

// figure out matching corners to join to.
// get coordinates for calulaction and set match = none
for (i = 0; i < 4; i++) {
	memcpy (&v1 [i], vertexManager.Vertex (seg1->m_info.verts [sideVertTable [cur1->m_nSide][i]]), sizeof (CVertex)); 
	memcpy (&v2 [i], vertexManager.Vertex (seg2->m_info.verts [sideVertTable [cur2->m_nSide][i]]), sizeof (CVertex)); 
	match [i].i = -1; 
	}

// find closest for each point for each corner
for (i = 0; i < 4; i++) {
	minRadius = JOIN_DISTANCE; 
	for (j = 0; j < 4; j++) {
		radius = Distance (v1 [i], v2 [j]);
		if (radius < minRadius) {
			minRadius = radius; 
			match [i].i = j;  // remember which vertex it matched
			}
		}
	}

fail = false; 
for (i = 0; i < 4; i++)
	if (match [i].i == -1) {
		fail = true; 
		break; 
	}

// make sure there are matches for each and they are unique
if (!fail)
	fail = (match [0].i == match [1].i) ||
			 (match [0].i == match [2].i) ||
			 (match [0].i == match [3].i) ||
			 (match [1].i == match [2].i) ||
			 (match [1].i == match [3].i) ||
			 (match [2].i == match [3].i);

if (fail) {
	int offset = (4 + cur1->m_nPoint - (3 - cur2->m_nPoint)) % 4; 
	match [0].i = (offset + 3) % 4; 
	match [1].i = (offset + 2) % 4; 
	match [2].i = (offset + 1) % 4; 
	match [3].i = (offset + 0) % 4; 
	}

// determine min and max distances
minRadius = JOIN_DISTANCE; 
maxRadius = 0; 
for (i = 0; i < 4; i++) {
	j = match [i].i; 
	radius = Distance (v1 [i], v2 [j]);
	minRadius = min (minRadius, radius); 
	maxRadius = max (maxRadius, radius); 
	}

// make sure there are distances are close enough
if (maxRadius >= JOIN_DISTANCE) {
	if (!bExpertMode)
		ErrorMsg ("Sides are too far apart to join.\n\n"
					 "Hint: Cubes should not exceed 200 in any dimension\n"
					 "or they will distort when viewed from close up."); 
	return; 
	}

// if Segment () are too close to put a new segment between them, 
// then solidifyally link them together without asking
if (minRadius <= 5) {
	undoManager.SetModified (true); 
	undoManager.Lock ();
	LinkSides (cur1->m_nSegment, cur1->m_nSide, cur2->m_nSegment, cur2->m_nSide, match); 
	SetLinesToDraw(); 
	undoManager.Unlock ();
	DLE.MineView ()->Refresh ();
	return; 
	}

if (QueryMsg ("Are you sure you want to create a new segment which\n"
				  "connects the current side with the 'other' side?\n\n"
				  "Hint: Make sure you have the current point of each segment\n"
				  "on the corners you to connected.\n"
				  "(the 'P' key selects the current point)") != IDYES)
	return; 

nNewSeg = Count (); 
if (!(Count () < MAX_SEGMENTS)) {
	if (!bExpertMode)
		ErrorMsg ("The maximum number of Segment () has been reached.\n"
					"Cannot add any more Segment ()."); 
	return; 
	}
segP = Segment (nNewSeg); 

undoManager.SetModified (true); 
undoManager.Lock ();
// define children and special child
// first clear all sides
segP->m_info.childFlags = 0; 
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  /* no remaining children */
	segP->SetChild (i, -1); 

// now define two sides:
// near side has opposite side number cube 1
segP->SetChild (oppSideTable [cur1->m_nSide], cur1->m_nSegment); 
// far side has same side number as cube 1
segP->SetChild (cur1->m_nSide, cur2->m_nSegment); 
segP->m_info.owner = -1;
segP->m_info.group = -1;
segP->m_info.function = 0; 
segP->m_info.nMatCen =-1; 
segP->m_info.value =-1; 

// define vert numbers
for (i = 0; i < 4; i++) {
	segP->m_info.verts [oppSideVertTable [cur1->m_nSide][i]] = seg1->m_info.verts [sideVertTable [cur1->m_nSide][i]]; 
	segP->m_info.verts [sideVertTable [cur1->m_nSide][i]] = seg2->m_info.verts [sideVertTable [cur2->m_nSide][match [i].i]]; 
	}

// define Walls ()
segP->m_info.wallFlags = 0; // unmarked
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++)
	segP->m_sides [nSide].m_info.nWall = NO_WALL; 

// define sides
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	if (segP->Child (nSide) == -1) {
		SetTextures (CSideKey (nNewSeg, nSide), seg1->m_sides [cur1->m_nSide].m_info.nBaseTex, seg1->m_sides [cur1->m_nSide].m_info.nOvlTex); 
		Segment (nNewSeg)->SetUV (nSide, 0, 0); 
		}
	else {
		SetTextures (CSideKey (nNewSeg, nSide), 0, 0); 
		for (i = 0; i < 4; i++) 
			segP->m_sides [nSide].m_info.uvls [i].Clear (); 
		}
	}

// define static light
segP->m_info.staticLight = seg1->m_info.staticLight; 

// update cur segment
seg1->SetChild (cur1->m_nSide, nNewSeg); 
SetTextures (*cur1, 0, 0); 
for (i = 0; i < 4; i++) 
	seg1->m_sides [cur1->m_nSide].m_info.uvls [i].Clear (); 
seg2->SetChild (cur2->m_nSide, nNewSeg); 
SetTextures (CSideKey (cur2->m_nSegment, cur2->m_nSide), 0, 0); 
for (i = 0; i < 4; i++) 
	seg2->m_sides [cur2->m_nSide].m_info.uvls [i].Clear (); 

// update number of Segment () and vertices
Count ()++; 
undoManager.Unlock ();
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
short nNewSide, nSide, nSegment, nNewSeg; 

nNewSeg = current.m_nSegment; 
nNewSide = current.m_nSide; 
CSegment *segP = Segment (0),
			*newSegP = Segment (nNewSeg);
CVertex	*vSeg, 
			*vNewSeg = vertexManager.Vertex (newSegP->m_info.verts [0]);
for (nSegment = 0; nSegment < Count (); nSegment++, segP) {
	if (nSegment != nNewSeg) {
		// first check to see if Segment () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		vSeg = vertexManager.Vertex (segP->m_info.verts [0]);
		if (fabs ((double) (vNewSeg->v.x - vSeg->v.x)) < 10.0 &&
		    fabs ((double) (vNewSeg->v.y - vSeg->v.y)) < 10.0 &&
		    fabs ((double) (vNewSeg->v.z - vSeg->v.z)) < 10.0) {
			for (nSide = 0; nSide < 6; nSide++) {
				if (!Link (nNewSeg, nNewSide, nSegment, nSide, 3)) {
					// if these Segment () were linked, then unlink them
					if ((newSegP->Child (nNewSide) == nSegment) && (segP->Child (nSide) == nNewSeg)) {
						newSegP->SetChild (nNewSide, -1); 
						segP->SetChild (nSide, -1); 
						}
					}
				}
			}
		}
	}
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp