// Segment.cpp

#include "mine.h"
#include "dle-xp.h"
#include "AVLTree.h"

CSegmentManager segmentManager;

// ------------------------------------------------------------------------- 

CVertex& CSegmentManager::CalcCenter (CVertex& pos, short nSegment) 
{
pos.Clear ();
CSegment _const_ * segP = Segment (nSegment);
for (int i = 0; i < 8; i++)
	pos += *segP->Vertex (i);
pos /= 8.0;
return pos;
}

// -----------------------------------------------------------------------------

CDoubleVector CSegmentManager::CalcSideCenter (CSideKey key)
{
current->Get (key);

	CSegment _const_ * segP = Segment (key.m_nSegment);
	CSide _const_ * sideP = segP->Side (key.m_nSide);
	ushort* vertexIds = segP->m_info.vertexIds;
	ubyte* vertexIdIndex = sideP->m_vertexIdIndex;
	int n = sideP->VertexCount ();

CDoubleVector v = vertexManager [vertexIds [*vertexIdIndex]];
for (int i = 1; i < n; i++)
	v += vertexManager [vertexIds [*++vertexIdIndex]];
v /= double (n);
return v;
}

// -----------------------------------------------------------------------------

CDoubleVector CSegmentManager::CalcSideNormal (CSideKey key)
{
current->Get (key);

	CSegment _const_ * segP = Segment (key.m_nSegment);
	int n = segP->Side (key.m_nSide)->VertexCount ();

if (n < 3)
	return CDoubleVector (0.0, 0.0, 0.0);
return -Normal (*(segP->Vertex (key.m_nSide, 0)), *(segP->Vertex (key.m_nSide, 1)), *(segP->Vertex (key.m_nSide, n - 1)));
}

// ------------------------------------------------------------------------------ 
// get_opposing_side()
//
// Action - figures out childs nSegment and side for a given side
// Returns - TRUE on success
// ------------------------------------------------------------------------------ 

CSide _const_ * CSegmentManager::BackSide (CSideKey key, CSideKey& back)
{
current->Get (key); 
#ifdef _DEBUG
if (key.m_nSegment < 0 || key.m_nSegment >= Count ())
	return null; 
if (key.m_nSide < 0 || key.m_nSide >= 6)
	return null; 
#endif
short nChildSeg = Segment (key.m_nSegment)->ChildId (key.m_nSide); 
if (nChildSeg < 0 || nChildSeg >= Count ())
	return null; 
CSegment* childSegP = Segment (nChildSeg);
for (short nChildSide = 0; nChildSide < 6; nChildSide++) {
	if (Segment (nChildSeg)->ChildId (nChildSide) == key.m_nSegment) {
		back.m_nSegment = nChildSeg; 
		back.m_nSide = nChildSide; 
		return Side (back); 
		}
	}
return null; 
}

// -----------------------------------------------------------------------------

bool CSegmentManager::IsExit (short nSegment)
{
	CSideKey key (nSegment, 0);
	CSegment* segP = segmentManager.Segment (nSegment);

for (key.m_nSide = 0; key.m_nSide < 6; key.m_nSide++) {
	CWall* wallP = Wall (key);
	if (wallP) {
		int i = wallP->Index ();
		if (wallManager.IsExit (i))
			return true;
		i = wallManager.Index (wallManager.OppositeWall (*wallManager.Wall (i)));
		if (wallManager.IsExit (i))
			return true;
		}
	}
return false;
}

// -----------------------------------------------------------------------------

int CSegmentManager::IsWall (CSideKey key)
{
current->Get (key); 
return (Segment (key.m_nSegment)->ChildId (key.m_nSide) == -1) || (Wall (key) != null);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::DeleteWalls (short nSegment)
{
	CSide _const_ * sideP = Segment (nSegment)->m_sides; 

for (int i = MAX_SIDES_PER_SEGMENT; i; i--, sideP++)
	wallManager.Delete (sideP->m_info.nWall);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::UpdateVertices (short nOldVert, short nNewVert)
{
CSegment _const_ * segP = Segment (0);
int nSegments = segmentManager.Count ();
for (int nSegment = 0; nSegment < nSegments; nSegment++, segP++) {
	ushort* vertexIds = segP->m_info.vertexIds;
	for (ushort nVertex = 0; nVertex < 8; nVertex++) {
		if (vertexIds [nVertex] == nOldVert)
			vertexIds [nVertex] = nNewVert;
		}
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::UpdateWalls (short nOldWall, short nNewWall)
{
if (nOldWall != nNewWall) {
	CSegment _const_ * segP = Segment (0);
	for (int i = Count (); i; i--, segP++) {
		CSide* sideP = &segP->m_sides [0];
		for (int j = 0; j < 6; j++, sideP++)
			if ((sideP->m_info.nWall != NO_WALL) && (sideP->m_info.nWall >= nOldWall))
				sideP->m_info.nWall = nNewWall;
		}
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
undoManager.Begin (udSegments);
const_cast<CSegment*>(Segment (nSegment))->Reset (nSide); 
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
		if (segP->ChildId (nSide) > -1) { // -1 = no child,  - 2 = outside of world
			segP->m_info.mapBitmask &= ~(1 << (sideEdgeTable [nSide][0])); 
			segP->m_info.mapBitmask &= ~(1 << (sideEdgeTable [nSide][1])); 
			segP->m_info.mapBitmask &= ~(1 << (sideEdgeTable [nSide][2])); 
			segP->m_info.mapBitmask &= ~(1 << (sideEdgeTable [nSide][3])); 
			}
		}
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::CopyOtherSegment (void)
{
	bool bChange = false;

if (selections [0].m_nSegment == selections [1].m_nSegment)
	return; 
short nSegment = current->m_nSegment; 
CSegment* segP = current->Segment ();
CSegment* otherSegP = other->Segment (); 
undoManager.Begin (udSegments);
for (int nSide = 0; nSide < 6; nSide++)
	if (SetTextures (CSideKey (nSegment, nSide), otherSegP->m_sides [nSide].BaseTex (), otherSegP->m_sides [nSide].OvlTex ()))
		bChange = true;
undoManager.End ();
if (bChange)
	DLE.MineView ()->Refresh (); 
}

// -----------------------------------------------------------------------------

int CSegmentManager::Fix (void)
{
int errFlags = 0;

int nSegments = segmentManager.Count ();
for (int si = 0; si < nSegments; si++) {
	CSegment* segP = segmentManager.Segment (si);
	for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		CSide& side = segP->m_sides [nSide];
		if ((side.m_info.nWall != NO_WALL) && ((side.m_info.nWall >= wallManager.Count () || !side.Wall ()->Used ()))) {
			side.m_info.nWall = NO_WALL;
			errFlags |= 1;
			}
		if ((segP->ChildId (nSide) < -2) || (segP->ChildId (nSide) > Count ())) {
			segP->SetChild (nSide, -1);
			errFlags |= 2;
			}
		}
	for (ushort nVertex = 0; nVertex < MAX_VERTICES_PER_SEGMENT; nVertex++) {
		if ((segP->m_info.vertexIds [nVertex] <= MAX_VERTEX) && (segP->m_info.vertexIds [nVertex] >= vertexManager.Count ())) {
			segP->m_info.vertexIds [nVertex] = 0;  // this will cause a bad looking picture
			errFlags |= 4;
			}
		}
	}
return errFlags;
}

// -----------------------------------------------------------------------------

void CSegmentManager::DeleteD2X (void)
{
while (Count () > MAX_SEGMENTS_D2)
	Delete (Count () - 1);
for (int i = 0; i < Count (); i++) {
	if (m_segments [i].IsD2X ()) {
		Undefine (i);
		m_segments [i].m_info.props = 0;
		}
	}
}

// ----------------------------------------------------------------------------- 

int CSegmentManager::Overflow (int nSegments) 
{ 
if (nSegments < 0)
	nSegments = Count ();
return DLE.IsD1File () 
		 ? (nSegments > MAX_SEGMENTS_D1) 
		 : DLE.IsStdLevel () 
			? (nSegments > MAX_SEGMENTS_D2) 
			: (nSegments > SEGMENT_LIMIT)
				? -1
				: 0;
}

// -----------------------------------------------------------------------------

short CSegmentManager::FindByVertex (ushort nVertex, short nSegment)
{
CSegment* segP = Segment (nSegment);
for (short i = Count (); nSegment < i; nSegment++, segP++)
	if (segP->HasVertex (nVertex))
		return nSegment;
return -1;
}

// -----------------------------------------------------------------------------

bool CSegmentManager::VertexInUse (ushort nVertex, short nIgnoreSegment)
{
if (nIgnoreSegment < 0)
	nIgnoreSegment = current->m_nSegment;
for (short nSegment = 0; 0 <= (nSegment = FindByVertex (nVertex, nSegment)); nSegment++)
	if (nSegment != nIgnoreSegment)
		return true;
return false;
}

//------------------------------------------------------------------------------

void CSegmentManager::ComputeNormals (bool bAll, bool bView)
{
	short nSegments = segmentManager.Count ();
	
#pragma omp parallel for
for (short nSegment = 0; nSegment < nSegments; nSegment++) {
	CSegment* segP = Segment (nSegment);
	segP->ComputeCenter (bView);
	CSide* sideP = segP->Side (0);
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		if (sideP->Shape () > SIDE_SHAPE_TRIANGLE)
			continue;
		if (!(bAll || sideP->IsVisible ()))
			continue;
		sideP->ComputeNormals (segP->m_info.vertexIds, segP->m_vCenter, bView);
		}
	}
}

//------------------------------------------------------------------------------

CSegment* CSegmentManager::GatherSelectableSegments (CRect& viewport, long xMouse, long yMouse, bool bAllowSkyBox)
{
	short nSegments = segmentManager.Count ();

m_selectedSegments = null;
	
#pragma omp parallel for
for (short nSegment = 0; nSegment < nSegments; nSegment++) {
	CSegment* segP = Segment (nSegment);
	if ((segP->Function () == SEGMENT_FUNC_SKYBOX) && !bAllowSkyBox)
		continue;
	if (segP->IsSelected (viewport, xMouse, yMouse, 0, true))
#pragma omp critical
		{
		segP->Link (m_selectedSegments);
		m_selectedSegments = segP;
		}
	}
return m_selectedSegments;
}

// -----------------------------------------------------------------------------

CSide* CSegmentManager::GatherSelectableSides (CRect& viewport, long xMouse, long yMouse, bool bAllowSkyBox, bool bSegments)
{
	short nSegments = segmentManager.Count ();

m_selectedSegments = null;
m_selectedSides = null;
	
#ifdef NDEBUG
#	pragma omp parallel for
#endif
for (short nSegment = 0; nSegment < nSegments; nSegment++) {
	CSegment* segP = Segment (nSegment);
	if ((segP->Function () == SEGMENT_FUNC_SKYBOX) && !bAllowSkyBox)
		continue;
	bool bSegmentSelected = false;
	short nSide = 0;
	for (; nSide < 6; nSide++) {
		nSide = segP->IsSelected (viewport, xMouse, yMouse, nSide, bSegments);
		if (nSide < 0)
			break;
#ifdef NDEBUG
#	pragma omp critical
#endif
			{
			if (!bSegmentSelected) {
				bSegmentSelected = true;
				segP->Link (m_selectedSegments);
				m_selectedSegments = segP;
				}
			}
			CSide* sideP = segP->Side (nSide);
#ifdef NDEBUG
#	pragma omp critical
#endif
			{
			sideP->Link (m_selectedSides);
			m_selectedSides = sideP;
			}
		sideP->SetParent (nSegment);
		}
	}
return m_selectedSides;
}

// -----------------------------------------------------------------------------
// Collect all edges that belong to at least one "visible" side (i.e. a side
// with no connected segment or a visible, textured wall) and create a list 
// of all visible sides that are connect to each such edge. The edge key is 
// created out of the ids of the two vertices defining the edge: smaller vertex
// id in low word, greater vertex id in high word of a uint. 

void CSegmentManager::GatherEdges (CAVLTree <CEdgeTreeNode, uint>& edgeTree)
{
	CEdgeList	edgeList;
	CSegment*	segP = Segment (0);
	short			nSegments = Count ();

for (short nSegment = 0; nSegment < nSegments; nSegment++, segP++) {
	int nEdges = segP->BuildEdgeList (edgeList, true);
	for (int nEdge = 0; nEdge < nEdges; nEdge++) {
		ubyte side1, side2, i1, i2;
		edgeList.Get (nEdge, side1, side2, i1, i2);
		ushort v1 = segP->VertexId (side1, i1);
		ushort v2 = segP->VertexId (side1, i2);
		uint key = (v1 < v2) ? v1 + (uint (v2) << 16) : v2 + (uint (v1) << 16);
#ifdef _DEBUG
		if (key == 17236226)
			key = key;
#endif
		bool bVisible [2] = { (side1 < 6) && segP->Side (side1)->IsVisible (), (side2 < 6) && segP->Side (side2)->IsVisible () };
		if ((bVisible [0] || bVisible [1])) {
			CEdgeTreeNode node (key);
			CEdgeTreeNode*nodeP = edgeTree.Insert (node, key);
			if (nodeP) { // insert sides into side list of current edge
				if (bVisible [0])
					nodeP->Insert (nSegment, side1);
				if (bVisible [1])
					nodeP->Insert (nSegment, side2);
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------

uint CSegmentManager::VisibleSideCount (void)
{
	CSegment*	segP = Segment (0);
	short			nSegments = Count ();
	uint			nSides = 0;

for (short nSegment = 0; nSegment < nSegments; nSegment++, segP++)
	for (short nSide = 0; nSide < 6; nSide++)
		if (segP->Side (nSide)->IsVisible ())
			nSides++;
return nSides;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//eof segmentmanager.cpp