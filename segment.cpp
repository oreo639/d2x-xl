// Segment.cpp

#include "types.h"
#include "mine.h"
#include "dle-xp-res.h"
#include "dle-xp.h"

extern short nDbgSeg, nDbgSide;
extern int nDbgVertex;

// -----------------------------------------------------------------------------
// define points for a given side 
ubyte sideVertexTable [6][4] = {
	{7,6,2,3},
	{0,4,7,3},
	{0,1,5,4},
	{2,6,5,1},
	{4,5,6,7},
	{3,2,1,0} 
};

// define opposite side of a given side 
ubyte oppSideTable [6] = {2,3,0,1,5,4};

// define points for the opposite side of a given side 
ubyte oppSideVertexTable [6][4] = {
	{4,5,1,0},
	{1,5,6,2},
	{3,2,6,7},
	{3,7,4,0},
	{0,1,2,3},
	{7,6,5,4} 
};

// each side has 4 children (ordered by side's line number)
int sideChildTable[6][4] = {
  {4,3,5,1},
  {2,4,0,5},
  {5,3,4,1},
  {0,4,2,5},
  {2,3,0,1},
  {0,3,2,1}
};

// define 2 points for a given line 
ubyte edgeVertexTable [12][2] = {
	{0,1},
	{1,2},
	{2,3},
	{3,0},
	{0,4},
	{4,5},
	{5,6},
	{6,7},
	{7,4},
	{1,5},
	{2,6},
	{3,7}
};

// edges connected to a vertex
ubyte vertexEdgeTable [8][3] = {
	{0,4,5},
	{0,1,9},
	{1,2,10},
	{2,3,11},
	{4,5,8},
	{5,6,9},
	{6,7,10},
	{7,8,11}
};

// edges for a given side 
ubyte sideEdgeTable [6][4] = {
	{7,10,2,11},
	{4,8,11,3},
	{0,9,5,4},
	{10,6,9,1},
	{5,6,7,8},
	{2,1,0,3} 
};

// sides adjacent to given edge
ubyte edgeSideTable [12][2] = { 
	{2,5},
	{3,5},
	{0,5},
	{1,5},
	{1,2},
	{2,4},
	{4,5},
	{0,4},
	{1,4},
	{2,3},
	{0,3},
	{0,1}
};

// the three adjacent points of a segment for each corner of the segment 
ubyte adjacentPointTable [8][3] = {
	{1,3,4},
	{2,0,5},
	{3,1,6},
	{0,2,7},
	{7,5,0},
	{4,6,1},
	{5,7,2},
	{6,4,3}
};

// side numbers for each point (3 sides touch each point)
ubyte adjacentSideTable [8][3] = {
    {1,2,5},
    {2,3,5},
    {0,3,5},
    {0,1,5},
    {1,2,4},
    {2,3,4},
    {0,3,4},
    {0,1,4}
};

// CUVL corner for adjacentSideTable above
ubyte pointCornerTable [8][3] = {
    {0,0,3},
    {1,3,2},
    {2,0,1},
    {3,3,0},
    {1,3,0},
    {2,2,1},
    {1,1,2},
    {0,2,3} 
};

// -----------------------------------------------------------------------------

static ubyte segFuncFromType [] = {
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_PRODUCER,
	SEGMENT_FUNC_REPAIRCEN,
	SEGMENT_FUNC_REACTOR,
	SEGMENT_FUNC_ROBOTMAKER,
	SEGMENT_FUNC_GOAL_BLUE,
	SEGMENT_FUNC_GOAL_RED,
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_TEAM_BLUE,
	SEGMENT_FUNC_TEAM_RED,
	SEGMENT_FUNC_SPEEDBOOST,
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_SKYBOX,
	SEGMENT_FUNC_EQUIPMAKER,
	SEGMENT_FUNC_NONE
	};

static ubyte segPropsFromType [] = {
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_WATER,
	SEGMENT_PROP_LAVA,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_BLOCKED,
	SEGMENT_PROP_NODAMAGE,
	SEGMENT_PROP_BLOCKED,
	SEGMENT_PROP_NONE,
	SEGMENT_PROP_OUTDOORS
	};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CSegment::Upgrade (void)
{
if ((m_info.function == 2) && DLE.LevelType () < 2) // standard level repair center type - not supported by standard Descent
	m_info.function = 0;
m_info.props = segPropsFromType [m_info.function];
#ifdef _DEBUG
if (m_info.props)
	m_info.props = m_info.props;
#endif
m_info.function = segFuncFromType [m_info.function];
m_info.damage [0] =
m_info.damage [1] = 0;
}

// -----------------------------------------------------------------------------

ubyte CSegment::ReadWalls (CFileManager* fp)
{
	ubyte wallFlags = ubyte (fp->ReadSByte ());
	int	i;

for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	if (m_info.wallFlags & (1 << i)) 
		m_sides [i].m_info.nWall = (DLE.LevelVersion () >= 13) ? fp->ReadUInt16 () : (ushort) fp->ReadUByte ();
return wallFlags;
}

// -----------------------------------------------------------------------------

void CSegment::ReadExtras (CFileManager* fp, bool bExtras)
{
if (bExtras) {
	m_info.function = fp->ReadSByte ();
	if ((DLE.LevelType () == 2) && (DLE.LevelVersion () > 23)) {
		m_info.nProducer = fp->ReadInt16 ();
		m_info.value = fp->ReadInt16 ();
		}
	else {
		m_info.nProducer = fp->ReadSByte ();
		m_info.value = fp->ReadSByte ();
		}
	fp->ReadSByte ();
	}
else {
	m_info.function = 0;
	m_info.nProducer = -1;
	m_info.value = 0;
	}
if (DLE.LevelType ()) {
	if (DLE.LevelVersion () < 20)
		Upgrade ();
	else {
		m_info.props = fp->ReadSByte ();
		m_info.damage [0] = fp->ReadInt16 ();
		m_info.damage [1] = fp->ReadInt16 ();
		}
	}
m_info.staticLight = (DLE.LevelType () == 0) ? (int) fp->ReadInt16 () : fp->ReadInt32 ();
}

// -----------------------------------------------------------------------------

void CSegment::Read (CFileManager* fp)
{
	int	i;

if (DLE.LevelType () == 2) {
	m_info.owner = fp->ReadSByte ();
	m_info.group = fp->ReadSByte ();
	}
else {
	m_info.owner = -1;
	m_info.group = -1;
	}

for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
	m_sides [i].Reset (i);
// read in child mask (1 ubyte)
m_info.childFlags = ubyte (fp->ReadSByte ());

// read 0 to 6 children (0 to 12 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
	m_sides [i].m_info.nChild = (m_info.childFlags & (1 << i)) ? fp->ReadInt16 () : -1;

// read vertex numbers (16 bytes)
m_nShape = 0;
for (i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	if (MAX_VERTEX < (m_info.vertexIds [i] = fp->ReadUInt16 ()))
		m_nShape++;

if (DLE.LevelType () == 0)
	ReadExtras (fp, (m_info.childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

// read the wall bit mask
m_info.wallFlags = ubyte (fp->ReadSByte ());

// read in wall numbers (0 to 6 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	m_sides [i].m_info.nWall = (m_info.wallFlags & (1 << i)) 
										? (DLE.LevelVersion () < 13) 
											? (ushort) fp->ReadUByte () 
											: fp->ReadUInt16 ()
										: NO_WALL;
	}

// read in textures and uvls (0 to 60 bytes)
m_nShape = 0;
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	m_sides [i].Read (fp, (ChildId (i) == -1) || ((m_info.wallFlags & (1 << i)) != 0));
	if (m_sides [i].Shape ())
		m_nShape++;
	}
}

// -----------------------------------------------------------------------------

ubyte CSegment::WriteWalls (CFileManager* fp)
{
	int	i;

m_info.wallFlags = 0;
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if (m_sides [i].m_info.nWall != NO_WALL) 
		m_info.wallFlags |= (1 << i);
	}
fp->Write (m_info.wallFlags);

// write wall numbers
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if (m_info.wallFlags & (1 << i)) {
		CWall _const_ * wallP = m_sides [i].Wall ();
		if (DLE.LevelVersion () >= 13)
			fp->WriteUInt16 ((ushort) const_cast<CWall*>(wallP)->Index ());
		else
			fp->WriteSByte ((sbyte) const_cast<CWall*>(wallP)->Index ());
		}
	}
return m_info.wallFlags;
}

// -----------------------------------------------------------------------------

void CSegment::WriteExtras (CFileManager* fp, bool bExtras)
{
if (bExtras) {
	fp->Write (m_info.function);
	if (DLE.LevelType () == 2) {
		fp->Write (m_info.nProducer);
		fp->Write (m_info.value);
		}
	else {
		sbyte h = (sbyte) m_info.nProducer;
		fp->Write (h);
		h = (sbyte) m_info.value;
		fp->Write (h);
		}
	fp->WriteByte (0); // s2Flags
	}
if (DLE.LevelType () == 2) {
	fp->Write (m_info.props);
	fp->Write (m_info.damage [0]);
	fp->Write (m_info.damage [1]);
	}
if (DLE.LevelType () == 0)
	fp->WriteInt16 ((short) m_info.staticLight);
else
	fp->Write (m_info.staticLight);
}

// -----------------------------------------------------------------------------

void CSegment::Write (CFileManager* fp)
{
	int	i;

if (DLE.LevelType () == 2) {
	fp->Write (m_info.owner);
	fp->Write (m_info.group);
	}

#if 1
m_info.childFlags = 0;
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if (ChildId (i) != -1) {
		m_info.childFlags |= (1 << i);
		}
	}
if (DLE.LevelType () == 0) {
	if (m_info.function != 0) { // if this is a special segment
		m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
		}
	}
#endif
fp->Write (m_info.childFlags);

// write children numbers (0 to 6 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	if (m_info.childFlags & (1 << i)) 
		fp->WriteInt16 (ChildId (i));

// write vertex numbers (16 bytes)
for (i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	fp->WriteUInt16 (m_info.vertexIds [i]);

// write special info (0 to 4 bytes)
if ((m_info.function == SEGMENT_FUNC_ROBOTMAKER) && (m_info.nProducer == -1)) {
	m_info.function = SEGMENT_FUNC_NONE;
	m_info.value = 0;
	m_info.childFlags &= ~(1 << MAX_SIDES_PER_SEGMENT);
	}
if (DLE.LevelType () == 0)
	WriteExtras (fp, (m_info.childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

// calculate wall bit mask
WriteWalls (fp);
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  
	m_sides [i].Write (fp, (ChildId (i) == -1) || (m_info.wallFlags & (1 << i)));
}

// -----------------------------------------------------------------------------

void CSegment::Setup (void)
{
Reset ();
}

// -----------------------------------------------------------------------------

void CSegment::SetUV (short nSide, double x, double y)
{
	CDoubleVector	A [4], C1 [3], C2 [3], D [4];
	CDoubleVector*	R;
	int				i; 
	double			angle; 
	CSide*			sideP = Side (nSide);

for (i = 0; i < sideP->VertexCount(); i++)
	A [i] = CDoubleVector (*vertexManager.Vertex (m_info.vertexIds [sideP->VertexIdIndex (i)])); 

switch (sideP->Shape ()) {
	case SIDE_SHAPE_TRIANGLE:
		{
			CDoubleVector B1 [] = {A [0], A [1], A [2]};
			GetTriangleUVs (C1, B1);
		}
		R = C1;
		break;

	case SIDE_SHAPE_RECTANGLE:
		if (Dot (A [3] - A [0], Normal (A [0], A [1], A [2])) > 0) {
			// Split on 0,2
			CDoubleVector B1 [] = {A [0], A [1], A [2]};
			CDoubleVector B2 [] = {A [0], A [2], A [3]};
			GetTriangleUVs (C1, B1);
			GetTriangleUVs (C2, B2);

			// Rotate triangle C2 UVs onto C1 UVs so points 0 and 2 match (only need to calculate last point)
			angle = -atan3 (-C1 [2].v.x, C1 [2].v.y);
			C2 [2].Rotate (C2 [0], CDoubleVector (0, 0, 1), angle);

			// Combine triangles
			D [0] = C1 [0], D [1] = C1 [1], D [2] = C1 [2], D [3] = C2 [2];
			}
		else {
			// Split on 1,3
			CDoubleVector B1 [] = {A [0], A [1], A [3]};
			CDoubleVector B2 [] = {A [3], A [1], A [2]};
			GetTriangleUVs (C1, B1);
			GetTriangleUVs (C2, B2);

			// Rotate and translate triangle C2 UVs onto C1 UVs so points 1 and 3 match
			angle = atan3 (C1 [1].v.x - C1 [2].v.x, C1 [1].v.y - C1 [2].v.y);
			C2 [2].Rotate (C2 [0], CDoubleVector (0, 0, 1), angle);
			C2 [2] += C1 [2];

			// Combine triangles
			D [0] = C1 [0], D [1] = C1 [1], D [2] = C2 [2], D [3] = C1 [2];
			}
		R = D;
		break;

	default:
		// No reset operation defined, just return
		return;
	}

// Scale, translate, store UV values
CUVL *uvls = m_sides [nSide].m_info.uvls;
undoManager.Begin (udSegments); 
m_sides [nSide].LoadTextures ();
for (i = 0; i < sideP->VertexCount (); i++, uvls++) {
	uvls->u = (x + R [i].v.x) / 20.0; 
	uvls->v = (y + R [i].v.y) / 20.0; 
	}
undoManager.End ();
}

// -----------------------------------------------------------------------------

void CSegment::GetTriangleUVs (CDoubleVector (&triangleUVs) [3], const CDoubleVector (&triangleVecs) [3])
{
	CDoubleVector vec1, vec2, proj, perp;

vec1 = triangleVecs [1] - triangleVecs [0];
vec2 = triangleVecs [2] - triangleVecs [0];
proj = vec1;
if (proj.Mag () == 0) // degenerate face but let's still handle it
	proj = CDoubleVector (0, 1, 0);
proj.Normalize ();
proj *= Dot (proj, vec2);
perp = vec2 - proj;
triangleUVs [0] = CDoubleVector (0, 0, 0);
triangleUVs [1] = CDoubleVector (0, vec1.Mag (), 0);
triangleUVs [2] = CDoubleVector (-perp.Mag (), proj.Mag (), 0);
}

// -----------------------------------------------------------------------------

short CSegment::SetChild (short nSide, short nSegment) 
{
m_sides [nSide].m_info.nChild = nSegment;
if (nSegment == -1)
	m_info.childFlags &= ~(1 << nSide);
else
	m_info.childFlags |= (1 << nSide);
return nSegment;
}

// -----------------------------------------------------------------------------

bool CSegment::ReplaceChild (short nOldSeg, short nNewSeg) 
{
for (short nSide = 0; nSide < 6; nSide++) {
	if (m_sides [nSide].m_info.nChild == nOldSeg) {
		SetChild (nSide, nNewSeg);
		return true;
		}
	}
return false;
}

// -----------------------------------------------------------------------------

short CSegment::CommonVertices (short nOtherSeg, short nMaxVertices, ushort* vertices)
{
	CSegment* otherP = segmentManager.Segment (nOtherSeg);
	short nCommon = 0;

for (short i = 0; i < 8; i++) {
	ushort nVertex = VertexId (i);
	for (short j = 0; j < 8; j++) {
		if (nVertex == otherP->VertexId (j)) {
			if (vertices)
				vertices [nCommon] = nVertex;
			if (++nCommon == nMaxVertices)
				return nCommon;
			break;
			}
		}
	}
return nCommon;
}

// -----------------------------------------------------------------------------

short CSegment::FindSide (short nSide, short nVertices, ushort* vertices)
{
for (; nSide < 6; nSide++) {
	short nCommon = 0;
	for (int i = 0; i < nVertices; i++) {
		for (int j = 0; j < 4; j++) {
			if (VertexId (Side (nSide)->VertexIdIndex (j)) == vertices [i]) {
				if (++nCommon == nVertices)
					return nSide;
				}
			}
		}
	}		 
return -1;
}

// -----------------------------------------------------------------------------

short CSegment::CommonSide (short nSide, ushort* vertices)
{
ubyte* vertexIdIndex = Side (nSide)->m_vertexIdIndex;
short nCommon = 0;
for (short i = 0; i < 4; i++) {
	ushort nVertex = vertices [i];
	short j = 0;
	for (; j < 4; j++) {
		if (m_info.vertexIds [vertexIdIndex [j]] == nVertex) {
			if (++nCommon == 4)
				return 1;
			break;
			}
		}
	if (j == 4)
		return 0;
	}	
return 0;
}

// -----------------------------------------------------------------------------

short CSegment::CommonSides (short nOtherSeg, short& nOtherSide)
{
ushort vertices [4];
if (CommonVertices (nOtherSeg, 4, vertices) < 4)
	return -1;

short nSide = 0;
for (; nSide < 6; nSide++) 
	if (CommonSide (nSide, vertices))
		break;
if (nSide == 6)
	return -1;

CSegment* otherP = segmentManager.Segment (nOtherSeg);

for (nOtherSide = 0; nOtherSide < 6; nOtherSide++)
	if (otherP->CommonSide (nOtherSide, vertices))
		break;
if (nOtherSide == 6)
	return -1;
return nSide;
}

// -----------------------------------------------------------------------------
// return the index of the vertex index nIndex in segments's vertex index table

short CSegment::VertexIndex (short nVertexId)
{
for (int i = 0; i < 8; i++)
	if (m_info.vertexIds [i] == nVertexId)
		return 1;
return -1;
}

// -----------------------------------------------------------------------------
// return the index of the vertex index nIndex in side's nSide vertex index index

short CSegment::SideVertexIndex (short nSide, ubyte nPoint)
{
return m_sides [nSide].FindVertexIdIndex (nPoint);
}

// -----------------------------------------------------------------------------

CVertex _const_ * CSegment::Vertex (ushort nVertex) _const_
{
return vertexManager.Vertex (m_info.vertexIds [nVertex]);
}

// -----------------------------------------------------------------------------

CVertex _const_ * CSegment::Vertex (ushort nSide, short nIndex) _const_
{
return vertexManager.Vertex (VertexId (nSide, nIndex));
}

// -----------------------------------------------------------------------------

CSegment* CSegment::Child (short nSide) _const_ 
{ 
return (short (m_sides [nSide].m_info.nChild) < 0) ? null : segmentManager.Segment (m_sides [nSide].m_info.nChild); 
}

// -----------------------------------------------------------------------------

int CSegment::UpdateVertexId (ushort nOldId, ushort nNewId)
{
for (int i = 0; i < 8; i++)
	if (m_info.vertexIds [i] == nOldId) {
		m_info.vertexIds [i] = nNewId;
		return i;
		}
return -1;
}

// -----------------------------------------------------------------------------

void CSegment::UpdateChildren (short nOldChild, short nNewChild)
{
for (short nSide = 0; nSide < 6; nSide++)
	if (m_sides [nSide].UpdateChild (nOldChild, nNewChild))	// no two sides can have the same child
		return;
}

// -----------------------------------------------------------------------------

void CSegment::Reset (short nSide)
{
if (nSide >= 0) {
	SetChild (nSide, -1); 
	m_info.childFlags &= ~(1 << nSide); 
	m_sides [nSide].Reset (-1);
	}
else {
	m_info.staticLight = 0; 
	m_info.group = -1;
	m_info.owner = -1;
	m_info.function = 0;
	m_info.value = -1;
	m_info.nProducer = -1;
	m_info.childFlags = 0;
	m_info.wallFlags = 0;
	m_info.flags = 0;
	for (int i = 0; i < 8; i++)
		m_info.vertexIds [i] = i;
	m_nShape = SIDE_SHAPE_RECTANGLE;
	for (nSide = 0; nSide < 6; nSide++) {
		SetChild (nSide, -1); 
		m_info.childFlags &= ~(1 << nSide); 
		m_sides [nSide].Reset (nSide);
		}
	}	
}

// -----------------------------------------------------------------------------

bool CSegment::HasVertex (ushort nVertex)
{
for (int i = 0; i < 8; i++)
	if (m_info.vertexIds [i] == nVertex)
		return true;
return false;
}

// -----------------------------------------------------------------------------

int CSegment::HasVertex (short nSide, ubyte nIndex)
{
return m_sides [nSide].HasVertex (nIndex);
}

// -----------------------------------------------------------------------------

bool CSegment::HasEdge (short nSide, ushort nVertex1, ushort nVertex2)
{
	CSide*	sideP = Side (nSide);
	int		nVertices = sideP->VertexCount ();

if (nVertices < 2)
	return false;

if (nVertices == 2) {
	ushort v1 = m_info.vertexIds [sideP->VertexIdIndex (0)],
			 v2 = m_info.vertexIds [sideP->m_vertexIdIndex [1]];
	return ((v1 == nVertex1) && (v2 == nVertex2)) || ((v2 == nVertex1) && (v1 == nVertex2));
	}

ushort v1, v2 = m_info.vertexIds [sideP->VertexIdIndex (0)];
for (int i = 1; i <= nVertices; i++) {
	v1 = v2;
	v2 = m_info.vertexIds [sideP->m_vertexIdIndex [i % nVertices]];
	if (((v1 == nVertex1) && (v2 == nVertex2)) || ((v2 == nVertex1) && (v1 == nVertex2)))
		return true;
	}
return false;
}

// -----------------------------------------------------------------------------

CGameItem* CSegment::Copy (CGameItem* destP)
{
if (destP != null)
	*dynamic_cast<CSegment*> (destP) = *this;
return destP;
}

// -----------------------------------------------------------------------------

CGameItem* CSegment::Clone (void)
{
return Copy (new CSegment);	// only make a copy if modified
}

// -----------------------------------------------------------------------------

void CSegment::Backup (eEditType editType)
{
Id () = undoManager.Backup (this, editType);
}

// -----------------------------------------------------------------------------

void CSegment::Undo (void)
{
switch (EditType ()) {
	case opAdd:
		segmentManager.Remove (Index ());
		break;
	case opDelete:
		segmentManager.Add ();
		// fall through
	case opModify:
		*Parent () = *this;
		break;
	}
}

// -----------------------------------------------------------------------------

void CSegment::Redo (void)
{
switch (EditType ()) {
	case opDelete:
		segmentManager.Remove (Index ());
		break;
	case opAdd:
		segmentManager.Add ();
		// fall through
	case opModify:
		*Parent () = *this;
		break;
	}
}

// -----------------------------------------------------------------------------

short CSegment::AdjacentSide (short nIgnoreSide, ushort* nEdgeVerts)
{
for (short nSide = 0; nSide < 6; nSide++)
	if ((nSide != nIgnoreSide) && (m_sides [nSide].m_nShape <= SIDE_SHAPE_TRIANGLE) && HasEdge (nSide, nEdgeVerts [0], nEdgeVerts [1]))
		return nSide;
return -1;
}

// -----------------------------------------------------------------------------

void CSegment::TagVertices (ubyte mask, short nSide)
{
if (nSide < 0) {
	for (int i = 0; i < 8; i++)
		if (m_info.vertexIds [i] <= MAX_VERTEX)
			vertexManager [m_info.vertexIds [i]].Tag (mask);
	}
else {
	CSide* sideP = Side (nSide);
	for (int i = 0, j = sideP->VertexCount (); i < j; i++)
		vertexManager [m_info.vertexIds [sideP->VertexIdIndex (i)]].Tag (mask);
	}
}

// -----------------------------------------------------------------------------

void CSegment::UnTagVertices (ubyte mask, short nSide)
{
if (nSide < 0) {
	for (int i = 0; i < 8; i++)
		if (m_info.vertexIds [i] <= MAX_VERTEX)
			vertexManager [m_info.vertexIds [i]].UnTag (mask);
	}
else {
	CSide* sideP = Side (nSide);
	for (int i = 0, j = sideP->VertexCount (); i < j; i++)
		vertexManager [m_info.vertexIds [sideP->VertexIdIndex (i)]].UnTag (mask);
	}
}

// -----------------------------------------------------------------------------

void CSegment::Tag (ubyte mask)
{
for (int i = 0; i < 6; i++)
	Side (i)->Tag (mask);
for (int i = 0; i < 8; i++)
	if (VertexId (i) <= MAX_VERTEX)
		Vertex (i)->Tag (mask);
}

// -----------------------------------------------------------------------------

void CSegment::UnTag (ubyte mask)
{
for (int i = 0; i < 6; i++)
	Side (i)->UnTag (mask);
for (int i = 0; i < 8; i++)
	if (VertexId (i) <= MAX_VERTEX)
		Vertex (i)->UnTag (mask);
}

// -----------------------------------------------------------------------------

void CSegment::ToggleTag (ubyte mask)
{
for (int i = 0; i < 6; i++)
	Side (i)->ToggleTag (mask);
for (int i = 0; i < 8; i++)
	if (VertexId (i) <= MAX_VERTEX)
		Vertex (i)->ToggleTag (mask);
}

// -----------------------------------------------------------------------------

void CSegment::Tag (short nSide, ubyte mask)
{
Side (nSide)->Tag (mask);
for (int i = 0, j = Side (nSide)->VertexCount (); i < j; i++)
	Vertex (nSide, i)->Tag (mask);
}

// -----------------------------------------------------------------------------

void CSegment::UnTag (short nSide, ubyte mask)
{
Side (nSide)->UnTag (mask);
for (int i = 0, j = Side (nSide)->VertexCount (); i < j; i++)
	Vertex (nSide, i)->UnTag (mask);
}

// -----------------------------------------------------------------------------

void CSegment::ToggleTag (short nSide, ubyte mask)
{
Side (nSide)->ToggleTag (mask);
for (int i = 0, j = Side (nSide)->VertexCount (); i < j; i++)
	Vertex (nSide, i)->ToggleTag (mask);
}

// -----------------------------------------------------------------------------

bool CSegment::IsTagged (short nSide, ubyte mask)
{
if (nSide >= 0)
	return Side (nSide)->IsTagged (mask);
for (nSide = 0; nSide < 6; nSide++)
	if (Side (nSide)->IsTagged (mask))
		return true;
return false;
}

// -----------------------------------------------------------------------------

bool CSegment::GetEdgeVertices (short nSide, short nLine, ushort& v1, ushort& v2)
{
	short i1, i2;

if (!Side (nSide)->GetEdgeIndices (nLine, i1, i2))
	return false;
v1 = m_info.vertexIds [i1];
v2 = m_info.vertexIds [i2];
return true;
}

// -----------------------------------------------------------------------------

ubyte CSegment::OppSideVertex (short nSide, short nIndex)
{
for (int i = 0; i < 4; i++) {
	if (sideVertexTable [nSide][i] == nIndex) 
		return oppSideVertexTable [nSide][i];
	}
return 0xff;
}

// -----------------------------------------------------------------------------

#if 0

double CSegment::MinEdgeLength (void)
{
	CEdgeList edgeList;

	ubyte		v1, v2, s1, s2;
	double	l, minLength = 1e30;
	int		nEdges = BuildEdgeList (edgeList);

for (int i = 0; i < nEdges; i++) {
	edgeList.Get (i, s1, s2, v1, v2);
	l = Distance (Vertex (VertexId (v1)), Vertex (VertexId (v2)));
	if (minLength > l)
		minLength = l;
	}
return minLength;
}

#endif

// -----------------------------------------------------------------------------

void CSegment::CreateOppVertexIndex (short nSide, ubyte* oppVertexIndex)
{
	CEdgeList edgeList;

	int	nEdges = BuildEdgeList (edgeList);
	int	nVertices = m_sides [nSide].VertexCount ();
	short	nOppSide = oppSideTable [nSide];
	ubyte* vertexIndex = m_sides [nSide].m_vertexIdIndex;
	int	i, j;
	ubyte	h, v, v1, v2, s1, s2;

for (i = 0; i < nVertices; i++) {
	oppVertexIndex [i] = 0xff;
	h = vertexIndex [i];
	// look for an edge that connects opp and far side and that shares the point we just assigned a vertex to. 
	// The other point of that edge is the corresponding point of origin side.
	for (j = 0; j < nEdges; j++) {
		edgeList.Get (j, s1, s2, v1, v2);
		if (h == v1)
			v = v2;
		else if (h == v2)
			v = v1;
		else
			continue;
		if ((s1 == nSide) || (s1 == nOppSide) || (s2 == nSide) || (s2 == nOppSide))
			continue;
		oppVertexIndex [i] = v; 
		break;
		}
	if (j == nEdges) {
		for (j = 0; j < 6; j++) {
			if (m_sides [j].m_nShape > SIDE_SHAPE_EDGE) {
				oppVertexIndex [i] = m_sides [j].m_vertexIdIndex [0];
				break;
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------

void CSegment::ShiftVertices (short nSide)
{
	CSide& side = m_sides [nSide];
	ushort vertices [4];
	int j = side.VertexCount ();

for (int i = 0; i < j; i++)
	vertices [(i + 1) % j] = m_info.vertexIds [side.m_vertexIdIndex [i]];
for (int i = 0; i < j; i++)
	m_info.vertexIds [side.m_vertexIdIndex [i]] = vertices [i];
}

// -----------------------------------------------------------------------------
// all vertex id table entries with index > nIndex have been move down once in
// the table. Update the sides' vertex id indices accordingly.

void CSegment::UpdateVertexIdIndex (ubyte nIndex)
{
CSide* sideP = Side (0);
for (int i = 0; i < 6; i++, sideP++) {
	int h = sideP->VertexCount ();
	for (int j = 0; j < h; j++) {
		if (sideP->m_vertexIdIndex [j] > nIndex)
			sideP->m_vertexIdIndex [j]--;
		}
	}
}

// -----------------------------------------------------------------------------

int CSegment::BuildEdgeList (CEdgeList& edgeList, ubyte nSide, bool bSparse)
{
	CSide* sideP = Side (nSide);

if (bSparse && !sideP->IsVisible ())
	return 0; // only gather edges that are not shared with another segment
short nVertices = sideP->VertexCount ();
if (nVertices < 2)
	return 0;
if (nVertices == 2)
	edgeList.Add (nSide, sideP->VertexIdIndex (0), sideP->VertexIdIndex (1));
else {
	ubyte v1, v2 = sideP->VertexIdIndex (0);
	for (short nVertex = 1; nVertex <= nVertices; nVertex++) {
		v1 = v2;
		v2 = sideP->VertexIdIndex (nVertex);
		edgeList.Add (nSide, v1, v2);
		}
	}
return 1;
}

// -----------------------------------------------------------------------------

int CSegment::BuildEdgeList (CEdgeList& edgeList, bool bSparse)
{
edgeList.Reset ();
for (ubyte nSide = 0; nSide < 6; nSide++) 
	BuildEdgeList (edgeList, nSide, bSparse);
return edgeList.Count ();
}

// -----------------------------------------------------------------------------

CVertex& CSegment::ComputeCenter (bool bView)
{
int nVertices = 0;
m_vCenter.Clear ();
for (int i = 0; i < 8; i++) {
	if (m_info.vertexIds [i] <= MAX_VERTEX) {
		nVertices++;
		m_vCenter += bView ? vertexManager [m_info.vertexIds [i]].m_view : vertexManager [m_info.vertexIds [i]];
		}
	}
m_vCenter /= double (nVertices);
return m_vCenter;
}

// -----------------------------------------------------------------------------

CVertex& CSegment::ComputeCenter (short nSide)
{
CSide* sideP = Side (nSide);
CVertex& vCenter = sideP->m_vCenter;
vCenter.Clear ();
vCenter.m_view.Clear ();
vCenter.m_screen.x = vCenter.m_screen.y = vCenter.m_screen.z = 0;
short j = sideP->VertexCount ();
for (short i = 0; i < j; i++) {
	CVertex* v = Vertex (nSide, i);
	vCenter += *v;
	vCenter.m_view += v->m_view;
	vCenter.m_screen += v->m_screen;
	}
vCenter /= double (j);
vCenter.m_view /= double (j);
vCenter.m_screen /= j;
return vCenter;
}

// -----------------------------------------------------------------------------

void CSegment::ComputeNormals (short nSide, bool bView)
{
if (nSide >= 0)
	Side (nSide)->ComputeNormals (m_info.vertexIds, ComputeCenter (bView));
else {
	ComputeCenter (bView);
	for (nSide = 0; nSide < 6; nSide++)
		Side (nSide)->ComputeNormals (m_info.vertexIds, m_vCenter);
	}
}

// -----------------------------------------------------------------------------

void CSegment::UpdateTexCoords (ushort nVertexId, bool bMove, short nIgnoreSide)
{
for (ubyte i = 0; i < 8; i++) {
	if (m_info.vertexIds [i] == nVertexId) {
		for (short nSide = 0; nSide < 6; nSide++) {
#ifdef _DEBUG
			if (nSide == nDbgSide)
				nDbgSide = nDbgSide;
#endif
			if (nSide != nIgnoreSide)
				m_sides [nSide].FindAndUpdateTexCoords (i, m_info.vertexIds, bMove);
			}
		return;
		}
	}
}

//--------------------------------------------------------------------------

short CSegment::IsSelected (CRect& viewport, long xMouse, long yMouse, short nSide, bool bSegments) 
{
ComputeCenter ();
CViewMatrix* viewMatrix = DLE.MineView ()->ViewMatrix ();
if (bSegments) {
	CVertex& center = Center ();
	center.Transform (viewMatrix);
	center.Project (viewMatrix);
	if ((center.m_screen.x  < 0) || (center.m_screen.y < 0) || (center.m_screen.x >= viewport.right) || (center.m_screen.y >= viewport.bottom) || (center.m_view.v.z < 0.0)) 
		return -1;
	}
short nSegment = segmentManager.Index (this);
#ifdef _DEBUG
if (nSegment == nDbgSeg)
	nDbgSeg = nDbgSeg;
#endif
CFrustum* frustum = DLE.MineView ()->Renderer ().Frustum ();
CSide* sideP = Side (nSide);
for (; nSide < 6; nSide++, sideP++) {
	if (frustum && !frustum->Contains (nSegment, nSide))
		continue;
	sideP->SetParent (nSegment);
	if (!sideP->IsSelected (viewport, xMouse, yMouse, m_info.vertexIds))
		continue;
	if (!bSegments) {
		ComputeCenter (nSide);
		CVertex& center = sideP->Center ();
		if (!sideP->IsVisible ()) {
			sideP->ComputeNormals (m_info.vertexIds, Center ());
			CVertex normal;
			center += sideP->Normal (2) * 2.0;
			center.Transform (viewMatrix);
			center.Project (viewMatrix);
			}
		if ((center.m_screen.x  < 0) || (center.m_screen.y < 0) || (center.m_screen.x >= viewport.right) || (center.m_screen.y >= viewport.bottom) || (center.m_view.v.z < 0.0)) 
			continue;
		}
	return nSide;
	}
return -1;
}

// -----------------------------------------------------------------------------

short CSegment::Edge (short nSide, ushort v1, ushort v2)
{
for (short i = 0; i < 8; i++) {
	if (m_info.vertexIds [i] == v1) {
		short j = (i + 1) % 8;
		if (m_info.vertexIds [j] != v2) {
			j = (i - 1 + 8) % 8;
			if (m_info.vertexIds [j] != v2)
				return -1;
			}
		return Side (nSide)->Edge (i, j);
		}
	}
return -1;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CObjectProducer::Read (CFileManager* fp, bool bFlag)
{
m_info.objFlags [0] = fp->ReadInt32 ();
if (DLE.IsD2File ())
	m_info.objFlags [1] = fp->ReadInt32 ();
m_info.hitPoints = fp->ReadInt32 ();
m_info.interval = fp->ReadInt32 ();
m_info.nSegment = fp->ReadInt16 ();
m_info.nProducer = fp->ReadInt16 ();
}

// -----------------------------------------------------------------------------

void CObjectProducer::Write (CFileManager* fp, bool bFlag)
{
fp->Write (m_info.objFlags [0]);
if (DLE.IsD2File ())
	fp->Write (m_info.objFlags [1]);
fp->Write (m_info.hitPoints);
fp->Write (m_info.interval);
fp->Write (m_info.nSegment);
fp->Write (m_info.nProducer);
}

// -----------------------------------------------------------------------------
// make a copy of this segment for the undo manager
// if segment was modified, make a copy of the current segment
// if segment was added or deleted, just make a new CGameItem instance and 
// mark the operation there

CGameItem* CObjectProducer::Clone (void)
{
CObjectProducer* cloneP = new CObjectProducer;	// only make a copy if modified
if (cloneP != null)
	*cloneP = *this;
return cloneP;
}

// -----------------------------------------------------------------------------

CGameItem* CObjectProducer::Copy (CGameItem* destP)
{
if (destP != null)
	*dynamic_cast<CObjectProducer*> (destP) = *this;
return destP;
}

// -----------------------------------------------------------------------------

void CObjectProducer::Backup (eEditType editType)
{
Id () = undoManager.Backup (this, editType);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int CEdgeList::Add (ubyte nSide, ubyte v1, ubyte v2)
{
ushort nEdge = (v1 < v2) ? ushort (v1) + (ushort (v2) << 8) : ushort (v2) + (ushort (v1) << 8);
for (int i = 0; i < m_nEdges; i++)
	if (m_edgeList [i].m_nEdge == nEdge) {
		if (m_edgeList [i].m_nSides == 2)
			return -1;
		m_edgeList [i].m_sides [1] = nSide;
		m_edgeList [i].m_nSides = 2;
		return i;
		}
m_edgeList [m_nEdges].m_nEdge = nEdge;
m_edgeList [m_nEdges].m_sides [0] = nSide;
m_edgeList [m_nEdges].m_nSides = 1;
return ++m_nEdges;
}

// -----------------------------------------------------------------------------

int CEdgeList::Find (int i, ubyte& side1, ubyte& side2, ubyte v1, ubyte v2)
{ 
ushort nEdge = (v1 < v2) ? ushort (v1) + (ushort (v2) << 8) : ushort (v2) + (ushort (v1) << 8);
for (; i < m_nEdges; i++) {
	if (m_edgeList [i].m_nEdge == nEdge) {
		side1 = m_edgeList [i].m_sides [0];
		side2 = m_edgeList [i].m_sides [1];
		return i;
		}
	}
return -1;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//eof segment.cpp