#include "mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Full (void) 
{ 
return Count () >= SEGMENT_LIMIT; 
}

// ----------------------------------------------------------------------------- 

short CSegmentManager::Add (void) 
{ 
#if USE_FREELIST

if (m_free.Empty ())
	return -1;
int nSegment = --m_free;
m_segments [nSegment].Clear ();
return (short) nSegment; 

#else //USE_FREELIST

if (Count () >= SEGMENT_LIMIT)
	return -1;
m_segments [Count ()].Reset ();
return Count ()++;

#endif //USE_FREELIST
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::Remove (short nDelSeg)
{
#if USE_FREELIST
m_free += nDelSeg;
Count ()--;
#else
if (nDelSeg < --Count ()) {
	// move the last segment in the segment list to the deleted segment's position
	if (current->m_nSegment == Count ())
		current->m_nSegment = nDelSeg;
	if (other->m_nSegment == Count ())
		other->m_nSegment = nDelSeg;
	*Segment (nDelSeg) = *Segment (Count ());
	// update all trigger targets pointing at the moved segment
	triggerManager.UpdateTargets (Count (), nDelSeg);
	objectManager.UpdateSegments (Count (), nDelSeg);
	// update all walls inside the moved segment
	CSide* sideP = Segment (nDelSeg)->Side (0);
	for (int i = 0; i < 6; i++, sideP++) {
		CSegment* segP = sideP->Child ();
		if (segP != null)
			segP->UpdateChildren (Count (), nDelSeg);
		CWall* wallP = sideP->Wall ();
		if (wallP != null)
			wallP->m_nSegment = nDelSeg;
		}
	}
#endif
}

// ----------------------------------------------------------------------------- 

short CSegmentManager::Create (int addMode)
{
	int i;

if (tunnelMaker.Active ())
	return -1; 

if (Full ()) {
	ErrorMsg ("Cannot add a new segment because\nthe maximum number of segments has been reached."); 
	return -1;
	}
if (vertexManager.Full ()) {
	ErrorMsg ("Cannot add a new segment because\nthe maximum number of vertices has been reached."); 
	return -1;
	}
if ((Count () == MAX_SEGMENTS - 1) && (QueryMsg ("Adding more segments will exceed\nthe maximum segment count for this level type.\nAre you sure you want to continue?") != IDYES)) {
	return -1;
	}

CSegment* segP = Segment (current->m_nSegment); 

short	nCurSide = current->m_nSide; 
if (segP->ChildId (nCurSide) >= 0) {
	ErrorMsg ("Can not add a new segment to a side\nwhich already has a segment attached."); 
	return -1;
	}

if (current->Side ()->VertexCount () < 3) {
	ErrorMsg ("Can not add a new segment to this side."); 
	undoManager.Unroll ();
	return -1;
	}

m_bCreating = true;
undoManager.Begin (udSegments); 
// get new segment

if (addMode < 0)
	addMode = m_nAddMode;

short nSides [2] = {current->m_nSide, oppSideTable [current->m_nSide]};

if ((addMode == MIRROR) && (segP->Side (nSides [0])->Shape () != segP->Side (nSides [1])->Shape ())) // mirror not possible!
	addMode = EXTEND;

short nNewSeg = Add (); 
CSegment* newSegP = Segment (nNewSeg); 

ushort newVerts [4]; 
vertexManager.Add (newVerts, current->Side ()->VertexCount ());

short nVertices = ComputeVertices (newVerts, addMode); 

CSide* orgSideP = segP->Side (nSides [0]);
CSide* farSideP = newSegP->Side (nSides [0]);
CSide* oppSideP = newSegP->Side (nSides [1]);
short nOppVertices = oppSideP->VertexCount ();

newSegP->Setup ();

if (nVertices == 3) { //create a wedge shaped segment when adding to a triangular segment side
	short nSide = 0;
	for (; nSide < 6; nSide++)
		if ((nSide != nSides [0]) && (nSide != nSides [1]) && segP->Side (nSide)->Shape ())
			break;
	CSide* sideP = newSegP->Side (nSide);
	short nEdge = 0, i = sideP->m_vertexIdIndex [0], v1, v2 = newSegP->m_info.vertexIds [i];
	for (++nEdge; nEdge <= 4; nEdge++) {
		v1 = v2;
		i = sideP->m_vertexIdIndex [nEdge % 4];
		v2 = newSegP->m_info.vertexIds [i];
		if (newSegP->HasEdge (nSides [0], v1, v2) || newSegP->HasEdge (nSides [1], v1, v2))
			break;
		}
	--nEdge;
	segmentManager.CollapseEdge (nNewSeg, nSide, nEdge % sideP->VertexCount (), false);
	if (nEdge < sideP->VertexCount ())
		nEdge++;
	segmentManager.CollapseEdge (nNewSeg, nSide, nEdge % sideP->VertexCount (), false);
	}

#ifdef _DEBUG
for (i = 0; i < 8; i++)
	if (newSegP->m_info.vertexIds [i] < MAX_VERTEX)
		newSegP->m_info.vertexIds [i] = MAX_VERTEX;
#endif

// Definitions:
//   origin segment: The segment to which a new segment is added
//   origin side: The origin segment's side from which the new segment shall extend
//   opposite side: The side of new segment that forms the back side of the origin side
//   far side: The side of the new segment opposing the base side
//   origin vertices: Vertices of the origin side
// This function assigns the origin vertices to the base side and derives
// the far side's vertices from these (by adding an offset to them)
// The following code assigns vertices to the new segment by looking at the edges of 
// the segment connecting opp side and far side

ubyte oppVertexIdIndex [4], * vertexIdIndex = farSideP->m_vertexIdIndex;
newSegP->CreateOppVertexIndex (nSides [0], oppVertexIdIndex);

#if 0

for (short i = 0; i < nVertices; i++) {
	// define vertex numbers for new side
	ubyte h = farSideP->m_vertexIdIndex [i];
	newSegP->m_info.vertexIds [h] = newVerts [i]; 

	// look for an edge that connects opp and far side and that shares the point we just assigned a vertex to. 
	// The other point of that edge is the corresponding point of origin side.
	ubyte v, v1, v2, s1, s2;
	for (short j = 0; j < nEdges; j++) {
		edgeList.Get (j, s1, s2, v1, v2);
		if (v1 == h)
			v = v2;
		else if (v2 == h)
			v = v1;
		else
			continue;
		if ((s1 == nSides [0]) || (s1 == nSides [1]) || (s2 == nSides [0]) || (s2 == nSides [1]))
			continue;
		newSegP->m_info.vertexIds [v] = segP->m_info.vertexIds [orgSideP->m_vertexIdIndex [i]]; 
		break;
		}
	}

#else

for (int i = 0; i < nVertices; i++) {
	// define vertex numbers for new side
	newSegP->m_info.vertexIds [vertexIdIndex [i]] = newVerts [i]; 
	newSegP->m_info.vertexIds [oppVertexIdIndex [i]] = segP->m_info.vertexIds [orgSideP->m_vertexIdIndex [i]]; 
	}

#endif

// define children and special child
newSegP->m_info.childFlags = 1 << oppSideTable [nCurSide]; // only opposite side connects to current segment
for (int i = 0; i < MAX_SIDES_PER_SEGMENT; i++) { // no remaining children
	newSegP->SetChild (i, (newSegP->m_info.childFlags & (1 << i)) ? current->m_nSegment : -1);
	newSegP->Side (i)->DetectShape ();
	}

// define textures
for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	if (newSegP->ChildId (nSide) < 0) {
		// if other segment does not have a child (therefore it has a texture)
		if ((segP->ChildId (nSide) < 0) && (segP->m_info.function == SEGMENT_FUNC_NONE)) {
			newSegP->m_sides [nSide].SetTextures (segP->m_sides [nSide].BaseTex (), segP->m_sides [nSide].OvlTex ());
			for (i = 0; i < nVertices; i++) {
				if (addMode != ORTHOGONAL)
					memcpy (newSegP->m_sides [nSide].m_info.uvls, segP->m_sides [nSide].m_info.uvls, sizeof (newSegP->m_sides [nSide].m_info.uvls));
				else
					newSegP->m_sides [nSide].m_info.uvls [i].l = segP->m_sides [nSide].m_info.uvls [i].l; 
				}
			} 
		}
	else {
		newSegP->m_sides [nSide].ResetTextures ();
		}
	}

// define static light
newSegP->m_info.staticLight = segP->m_info.staticLight; 

// delete variable light if it exists
lightManager.DeleteVariableLight (CSideKey (current->m_nSegment, nCurSide)); 

// update current segment
segP->SetChild (nCurSide, nNewSeg); 
orgSideP->SetTextures (0, 0);
memset (orgSideP->m_info.uvls, 0, sizeof (orgSideP->m_info.uvls));
 
// link the new segment with any touching Segment ()
#if 1
CVertex *vNewSeg = vertexManager.Vertex (newSegP->m_info.vertexIds [0]);
int nSegments = Count ();
segP = Segment (0); 
for (int i = 0; i < nSegments; i++, segP++) {
	if (i != nNewSeg) {
		// first check to see if Segment () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		CVertex *vSeg = vertexManager.Vertex (segP->m_info.vertexIds [0]);
		if (fabs (vNewSeg->v.x - vSeg->v.x) < 160.0 &&
			 fabs (vNewSeg->v.y - vSeg->v.y) < 160.0 &&
			 fabs (vNewSeg->v.z - vSeg->v.z) < 160.0)
			for (short nNewSide = 0; nNewSide < 6; nNewSide++)
				for (short nSide = 0; nSide < 6; nSide++)
					Link (nNewSeg, nNewSide, i, nSide, 3);
		}
	}
#endif
// auto align textures new segment
for (short nNewSide = 0; nNewSide < 6; nNewSide++)
	AlignTextures (current->m_nSegment, nNewSide, nNewSeg, nNewSide, true, true); 
// set current segment to new segment
current->m_nSegment = nNewSeg; 
current->Segment ()->Backup (opAdd);
//		SetLinesToDraw(); 
DLE.MineView ()->Refresh (false); 
DLE.ToolView ()->Refresh (); 
undoManager.End ();
m_bCreating = false;
return nNewSeg; 
}

// ----------------------------------------------------------------------------- 

short CSegmentManager::Create (short nSegment, bool bCreate, ubyte nFunction, short nTexture, char* szError)
{
if ((szError != null) && DLE.IsD1File ()) {
	if (!DLE.ExpertMode ())
		ErrorMsg (szError);
	return 0;
	}

undoManager.Begin (udSegments);
if (bCreate) {
	if (current->Child () >= 0) {
		undoManager.End ();
		return -1;
		}
	nSegment = Create ();
	if (nSegment < 0) {
		Remove (nSegment);
		undoManager.End ();
		return -1; 
		}
	}	
DLE.MineView ()->DelayRefresh (true);
m_bCreating = true;
if (!Define (nSegment, nFunction, -1)) {
	if (bCreate)
		Remove (nSegment);
	undoManager.End ();
	DLE.MineView ()->DelayRefresh (false);
	m_bCreating = false;
	return -1; 
	}	
Segment (nSegment)->Backup ();
m_bCreating = false;
undoManager.End ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
return nSegment;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateProducer (short nSegment, bool bCreate, ubyte nType, bool bSetDefTextures, 
												CObjectProducer* producers, CMineItemInfo& info, char* szError) 
{
if (info.count >= MAX_MATCENS) {
    ErrorMsg (szError);
	 return false;
	}
undoManager.Begin (udSegments);
if (0 > (nSegment = Create (nSegment, bCreate, nType))) {
	undoManager.End ();
	return false;
	}
producers [info.count].Setup (nSegment, info.count, 0);
Segment (nSegment)->m_info.value = 
Segment (nSegment)->m_info.nProducer = info.count++;
Segment (nSegment)->Backup (); // overwrite backup
undoManager.End ();
return true;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateEquipMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
{
if (!DLE.IsD2XFile ()) {
	ErrorMsg ("Equipment makers are only available in D2X-XL levels.");
	return false;
	}
return CreateProducer (nSegment, bCreate, SEGMENT_FUNC_EQUIPMAKER, bSetDefTextures, EquipMaker (0), m_producerInfo [1], 
							"Maximum number of equipment makers reached");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateRobotMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
{
return CreateProducer (nSegment, bCreate, SEGMENT_FUNC_ROBOTMAKER, bSetDefTextures, RobotMaker (0), m_producerInfo [0], 
							"Maximum number of robot makers reached");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateReactor (short nSegment, bool bCreate, bool bSetDefTextures) 
{
return 0 <= Create (nSegment, bCreate, SEGMENT_FUNC_REACTOR, bSetDefTextures ? DLE.IsD1File () ? 10 : 357 : -1);
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateGoal (short nSegment, bool bCreate, bool bSetDefTextures, ubyte nType, short nTexture) 
{
return 0 <= Create (nSegment, bCreate, nType, bSetDefTextures ? nTexture : -1, "Flag goals are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateTeam (short nSegment, bool bCreate, bool bSetDefTextures, ubyte nType, short nTexture) 
{
return 0 <= Create (nSegment, bCreate, nType, bSetDefTextures ? nTexture : -1, "Team start positions are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateSkybox (short nSegment, bool bCreate) 
{
return 0 <= Create (nSegment, bCreate, SEGMENT_FUNC_SKYBOX, -1, "Skyboxes are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateSpeedBoost (short nSegment, bool bCreate) 
{
return 0 <= Create (nSegment, bCreate, SEGMENT_FUNC_SPEEDBOOST, -1, "Speed boost segments are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

int CSegmentManager::ProducerCount (void)
{
int nProducers = 0;
for (CSegmentIterator si; si; si++) 
	if ((si->m_info.function == SEGMENT_FUNC_PRODUCER) || (si->m_info.function == SEGMENT_FUNC_REPAIRCEN))
		nProducers++;
return nProducers;
}

// ----------------------------------------------------------------------------- 

short CSegmentManager::CreateProducer (short nSegment, ubyte nType, bool bCreate, bool bSetDefTextures) 
{
// count number of fuel centers
int nProducer = ProducerCount ();
if (nProducer >= MAX_NUM_RECHARGERS) {
	ErrorMsg ("Maximum number of fuel centers reached.");
	return 0;
	}

CSegment *segP = Segment (0);

undoManager.Begin (udSegments);
if (nType == SEGMENT_FUNC_REPAIRCEN)
	nSegment = Create (nSegment, bCreate, nType, bSetDefTextures ? 433 : -1, "Repair centers are not available in Descent 1.");
else {
	short nLastSeg = current->m_nSegment;
	nSegment = Create (nSegment, bCreate, nType, bSetDefTextures ? DLE.IsD1File () ? 322 : 333 : -1);
	if (nSegment < 0) {
		undoManager.End ();
		return -1;
		}
	if (bSetDefTextures) { // add energy spark walls to fuel center sides
		current->m_nSegment = nLastSeg;
		if (wallManager.Create (*current, WALL_ILLUSION, 0, KEY_NONE, -1, -1) != null) {
			CSideKey back;
			if (BackSide (back))
				wallManager.Create (back, WALL_ILLUSION, 0, KEY_NONE, -1, -1);
			}
		Segment (nSegment)->Backup ();
		current->m_nSegment = nSegment;
		}
	}
undoManager.End ();
return nSegment;
}

// ----------------------------------------------------------------------------- 

short CSegmentManager::ComputeVerticesOrtho (ushort newVerts [4])
{
	CSegment*		segP = current->Segment (); 
	CSide*			sideP = current->Side ();
	short				nVertices = sideP->VertexCount ();

	CDoubleVector	A [4]; 
	short				i, points [4]; 

for (i = 0; i < 4; i++)
	points [i] = (current->m_nPoint + i) % 4;
	// METHOD 1: orthogonal with right angle on new side and standard segment side
// TODO:
//	int add_segment_mode = ORTHOGONAL; 
CDoubleVector center = CalcSideCenter (*current); 
CDoubleVector oppCenter = CalcSideCenter (CSideKey (current->m_nSegment, oppSideTable [current->m_nSide])); 
CDoubleVector vNormal = CalcSideNormal (*current); 
// set the length of the new segment to be one standard segment length
// scale the vector
vNormal *= 20.0; // 20 is edge length of a standard segment
// figure out new center
CDoubleVector newCenter = center + vNormal; 
// new method: extend points 0 and 1 with vNormal, then move point 0 toward point 1.
// point 0
ushort i1, i2;
segP->GetEdgeVertices (current->m_nSide, current->m_nPoint, i1, i2);
CDoubleVector a = vNormal + *vertexManager.Vertex (i1); 
// point 1
CDoubleVector b = vNormal + *vertexManager.Vertex (i2); 
// center
CDoubleVector c = Average (a, b);
// vector from center to point0 and its length
CDoubleVector d = a - c; 
double length = d.Mag (); 
// factor to mul
double factor = (length > 0) ? 10.0 / length : 1.0; 
// set point 0
d *= factor;
A [points [0]] = c + d; 
// set point 1
A [points [1]] = c - d; 
// point 2 is orthogonal to the vector 01 and the vNormal vector
c = -CrossProduct (A [points [0]] - A [points [1]], vNormal);
c.Normalize ();
// normalize the vector
A [points [2]] = A [points [1]] + (c * 20); 
A [points [3]] = A [points [0]] + (c * 20); 
if (nVertices == 3) {
	A [points [2]] += A [points [3]]; 
	A [points [2]] *= 0.5;
	}
a = A [0];
for (i = 1; i < nVertices; i++)
	a += A [i]; 
a /= double (nVertices);
// now center the side along about the newCenter
for (i = 0; i < nVertices; i++)
	A [i] += (newCenter - a); 
// set the new vertices
for (i = 0; i < nVertices; i++) {
	ushort nVertex = newVerts [i];
	*vertexManager.Vertex (nVertex) = A [i]; 
	vertexManager.Vertex (nVertex)->Backup (); // update the newly added vertex instead of creating a new backup
	}
return nVertices;
}

// ----------------------------------------------------------------------------- 

short CSegmentManager::ComputeVerticesExtend (ushort newVerts [4])
{
	CSegment*		segP = current->Segment (); 
	CSide*			sideP = current->Side ();
	short				nVertices = sideP->VertexCount ();

CDoubleVector center = CalcSideCenter (*current); 
CDoubleVector oppCenter = CalcSideCenter (CSideKey (current->m_nSegment, oppSideTable [current->m_nSide])); 
CDoubleVector vNormal = CalcSideNormal (*current); 
// calculate the length of the new segment
vNormal *= Distance (center, oppCenter); 
// set the new vertices
for (int i = 0; i < nVertices; i++) {
	ushort nVertex = newVerts [i];
	*vertexManager.Vertex (nVertex) = *segP->Vertex (sideP->m_vertexIdIndex [i]) + vNormal; 
	vertexManager.Vertex (nVertex)->Backup (); // do not create a new backup right after adding this vertex
	}
return nVertices;
}

// ----------------------------------------------------------------------------- 

short CSegmentManager::ComputeVerticesMirror (ushort newVerts [4])
{
	CDoubleVector	A [8], B [8], C [8], D [8], E [8]; 
	short				i; 
	CDoubleVector	center, oppCenter, newCenter, vNormal; 

	CSegment*		segP = current->Segment (); 
	CSide*			sideP = current->Side ();
	CSide*			oppSideP = current->OppositeSide ();
	short				nVertices = sideP->VertexCount ();
	ubyte				oppVertexIdIndex [4];

	current->Segment ()->CreateOppVertexIndex (current->m_nSide, oppVertexIdIndex);

// copy side's four points into A
short nSide = current->m_nSide;
for (i = 0; i < nVertices; i++) {
	A [i] = *segP->Vertex (sideP->m_vertexIdIndex [i]); 
	A [i + nVertices] = *segP->Vertex (oppVertexIdIndex [i]); 
	}

// subtract point 0 from all points in A to form B points
for (i = 0; i < 2 * nVertices; i++)
	B [i] = A [i] - A [0]; 

// calculate angle to put point 1 in x - y plane by spinning on x - axis
// then rotate B points on x - axis to form C points.
// check to see if on x - axis already
double angle1 = atan3 (B [1].v.z, B [1].v.y); 
for (i = 0; i < 2 * nVertices; i++)
	C [i].Set (B [i].v.x, B [i].v.y * cos (angle1) + B [i].v.z * sin (angle1), B [i].v.z * cos (angle1) - B [i].v.y * sin (angle1)); 
// calculate angle to put point 1 on x axis by spinning on z - axis
// then rotate C points on z - axis to form D points
// check to see if on z - axis already
double angle2 = atan3 (C [1].v.y, C [1].v.x); 
for (i = 0; i < 2 * nVertices; i++)
	D [i].Set (C [i].v.x * cos (angle2) + C [i].v.y * sin (angle2), C [i].v.y * cos (angle2) - C [i].v.x * sin (angle2), C [i].v.z); 

// calculate angle to put point 2 in x - y plane by spinning on x - axis
// the rotate D points on x - axis to form E points
// check to see if on x - axis already
double angle3 = atan3 (D [2].v.z, D [2].v.y); 
for (i = 0; i < 2 * nVertices; i++) 
	E [i].Set (D [i].v.x, D [i].v.y * cos (angle3) + D [i].v.z * sin (angle3), D [i].v.z * cos (angle3) - D [i].v.y * sin (angle3)); 

// now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
// mirror new points on z axis
for (i = nVertices; i < 2 * nVertices; i++)
	E [i].v.z = -E [i].v.z; 
// now reverse rotations
angle3 = -angle3;
for (i = nVertices; i < 2 * nVertices; i++) 
	D [i].Set (E [i].v.x, E [i].v.y * cos (angle3) + E [i].v.z * sin (angle3), E [i].v.z * cos (angle3) - E [i].v.y * sin (angle3)); 
angle2 = -angle2;
for (i = nVertices; i < 2 * nVertices; i++) 
	C [i].Set (D [i].v.x * cos (angle2) + D [i].v.y * sin (angle2), D [i].v.y * cos (angle2) - D [i].v.x * sin (angle2), D [i].v.z); 
angle1 = -angle1;
for (i = nVertices; i < 2 * nVertices; i++) 
	B [i].Set (C [i].v.x, C [i].v.y * cos (angle1) + C [i].v.z * sin (angle1), C [i].v.z * cos (angle1) - C [i].v.y * sin (angle1)); 

// and translate back
ushort nVertex = segP->m_info.vertexIds [sideP->m_vertexIdIndex [0]]; 
for (i = nVertices; i < 2 * nVertices; i++) 
	A [i] = B [i] + *vertexManager.Vertex (nVertex); 

for (i = 0; i < nVertices; i++) {
	nVertex = newVerts [i];
	*vertexManager.Vertex (nVertex) = A [i + nVertices]; 
	vertexManager.Vertex (nVertex)->Backup ();
	}
return nVertices;
}

// ----------------------------------------------------------------------------- 
// Calculate vertices when adding a new segment.

short CSegmentManager::ComputeVertices (ushort newVerts [4], int addMode)
{
	short	nVertices = current->Side ()->VertexCount ();

if (nVertices < 3)
	return 0;

if (addMode < 0)
	addMode = m_nAddMode;
if (addMode == ORTHOGONAL)
	nVertices = ComputeVerticesOrtho (newVerts);
else if (addMode == EXTEND)
	nVertices = ComputeVerticesExtend (newVerts);
else if (addMode == MIRROR)
	nVertices = ComputeVerticesMirror (newVerts);
else
	return 0;
for (int i = nVertices; i < 4; i++) 
	newVerts [i] = 0xffff;
return nVertices;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::SetDefaultTexture (short nTexture)
{
if (nTexture < 0)
	return true;

short nSegment = current->m_nSegment;
CSegment *segP = Segment (nSegment);

if (!m_bCreating)
	segP->Backup ();
double scale = textureManager.Scale (DLE.FileType (), nTexture);

undoManager.Begin (udSegments);
segP->m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
// set textures
CSide *sideP = segP->m_sides;
for (short nSide = 0; nSide < 6; nSide++, sideP++) {
	if (segP->ChildId (nSide) == -1) {
		SetTextures (CSideKey (nSegment, nSide), nTexture, 0);
		for (int i = 0; i < 4; i++) {
			sideP->m_info.uvls [i].u = (short) ((double) defaultUVLs [i].u / scale);
			sideP->m_info.uvls [i].v = (short) ((double) defaultUVLs [i].v / scale);
			sideP->m_info.uvls [i].l = defaultUVLs [i].l;
			}
		Segment (nSegment)->SetUV (nSide, 0, 0);
		}
	}
undoManager.End ();
return true;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Define (short nSegment, ubyte nFunction, short nTexture)
{
undoManager.Begin (udSegments);
CSegment *segP = (nSegment < 0) ? current->Segment () : Segment (nSegment);
if (!m_bCreating)
	segP->Backup ();
Undefine (Index (segP));
segP->m_info.function = nFunction;
segP->m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
SetDefaultTexture (nTexture);
undoManager.End ();
DLE.MineView ()->Refresh ();
return true;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::RemoveProducer (CSegment* segP, CObjectProducer* producers, CMineItemInfo& info, int nFunction)
{
if (info.count > 0) {
	// fill in deleted matcen
	int nDelProducer = segP->m_info.nProducer;
	if (nDelProducer >= 0) {
		// copy last producer in list to deleted producer's position
		undoManager.Begin (udSegments | udProducers);
		segP->m_info.nProducer = -1;
		segP->m_info.value = -1;
		if (nDelProducer < --info.count) {
			memcpy (&producers [nDelProducer], &producers [info.count], sizeof (producers [0]));
			producers [nDelProducer].m_info.nProducer = nDelProducer;
			// point owner of relocated producer to that producer's new position
			for (CSegmentIterator si; si; si++) {
				CSegment *segP = &(*si);
				if ((segP->m_info.function == nFunction) && (segP->m_info.nProducer == info.count)) {
					segP->Backup ();
					segP->m_info.nProducer = nDelProducer;
					break;
					}
				}
			}
		// remove producer from all robot maker triggers targetting it
		CSideKey key = (-Index (segP) - 1, 0); 
		for (int nClass = 0; nClass < 2; nClass++) {
			CTrigger* trigP = triggerManager.Trigger (0, nClass);
			for (CTriggerIterator ti (nClass); ti; ti++) {
				if (ti->Info ().type == TT_MATCEN)
					ti->Delete (key);
				}
			}
		undoManager.End ();
		}
	}
segP->m_info.nProducer = -1;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::Undefine (short nSegment)
{
	CSegment *segP = (nSegment < 0) ? current->Segment () : Segment (nSegment);

segP->Backup ();
nSegment = Index (segP);
if (segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER)
	RemoveProducer (segP, RobotMaker (0), m_producerInfo [0], SEGMENT_FUNC_ROBOTMAKER);
else if (segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) 
	RemoveProducer (segP, EquipMaker (0), m_producerInfo [1], SEGMENT_FUNC_EQUIPMAKER);
else if (segP->m_info.function == SEGMENT_FUNC_PRODUCER) { //remove all fuel cell walls
	undoManager.Begin (udSegments);
	CSide *sideP = segP->m_sides;
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		if (segP->ChildId (nSide) < 0)	// assume no wall if no child segment at the current side
			continue;
		CSegment* childSegP = Segment (segP->ChildId (nSide));
		if (childSegP->m_info.function == SEGMENT_FUNC_PRODUCER)	// don't delete if child segment is fuel center
			continue;
		// if there is a wall and it's a fuel cell delete it
		CSideKey key (nSegment, nSide);
		CWall *wallP = Wall (key);
		if ((wallP != null) && (wallP->Type () == WALL_ILLUSION) && (sideP->BaseTex () == (DLE.IsD1File () ? 322 : 333)))
			wallManager.Delete (sideP->m_info.nWall);
		// if there is a wall at the opposite side and it's a fuel cell delete it
		CSideKey back;
		if (BackSide (key, back)) {
			wallP = Wall (back);
			if ((wallP != null) && (wallP->Type () == WALL_ILLUSION)) {
				CSide* oppSideP = Side (back);
				if (oppSideP->BaseTex () == (DLE.IsD1File () ? 322 : 333))
					wallManager.Delete (oppSideP->m_info.nWall);
				}
			}
		}
	undoManager.End ();
	}
segP->m_info.childFlags &= ~(1 << MAX_SIDES_PER_SEGMENT);
segP->m_info.function = SEGMENT_FUNC_NONE;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::Delete (short nDelSeg, bool bDeleteVerts)
{
if (Count () < 2)
	return; 
if (nDelSeg < 0)
	nDelSeg = current->m_nSegment; 
if (nDelSeg < 0 || nDelSeg >= Count ()) 
	return; 
if (tunnelMaker.Active ())
	return;

undoManager.Begin (udSegments);
CSegment* delSegP = Segment (nDelSeg); 
current->Fix (nDelSeg);
other->Fix (nDelSeg);
delSegP->Backup (opDelete);
Undefine (nDelSeg);

// delete any variable lights that use this segment
for (int nSide = 0; nSide < 6; nSide++) {
	CSideKey key (nDelSeg, nSide);
	triggerManager.DeleteTargets (key); 
	lightManager.DeleteVariableLight (key);
	}

// delete any Walls () within segment (if defined)
DeleteWalls (nDelSeg); 

// delete any walls from adjacent segments' sides connecting to this segment
for (short i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if (delSegP->ChildId (i) >= 0) {
		CSideKey back, key (nDelSeg, i);
		if (BackSide (key, back))
			wallManager.Delete (Side (back)->m_info.nWall); 
		}
	}

	// delete any Objects () within segment
objectManager.DeleteSegmentObjects (nDelSeg);

// delete any control segP with this segment
for (short i = triggerManager.ReactorTriggerCount (); i > 0; )
	triggerManager.ReactorTrigger (--i)->Delete (CSideKey (-nDelSeg - 1));

	// update secret segment number if out of range now
	short nSegment = (short) objectManager.SecretSegment (); 
	if (nSegment >= Count () || nSegment == nDelSeg)
		objectManager.SecretSegment () = 0; 

	// update segment flags
	delSegP->Unmark (); 

	// unlink any children with this segment number
	int nSegments = segmentManager.Count ();
	for (int si = 0; si < nSegments; si++) {
		CSegment* segP = segmentManager.Segment (si);
		for (short nChild = 0; nChild < MAX_SIDES_PER_SEGMENT; nChild++) {
			if (segP->ChildId (nChild) == nDelSeg) {
				// remove nChild number and update nChild bitmask
				segP->SetChild (nChild, -1); 

				// define textures, (u, v) and light
				CSide *sideP = delSegP->m_sides + nChild;
				SetTextures (CSideKey (si, nChild), sideP->BaseTex (), sideP->OvlTex ()); 
				segP->SetUV (nChild, 0, 0); 
				for (short j = 0; j < 4; j++) {
					segP->Uvls (nChild) [j].l = delSegP->Uvls (nChild) [j].l; 
				}
			}
		}
	}
Remove (nDelSeg);

// delete all unused vertices
vertexManager.DeleteUnused (); 

// make sure current segment numbers are valid
DLE.MineView ()->Refresh (false); 
DLE.ToolView ()->Refresh (); 
undoManager.End ();
}

// ----------------------------------------------------------------------------- 

