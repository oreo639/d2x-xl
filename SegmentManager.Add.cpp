#include "mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Full (void) 
{ 
return Count () >= MAX_SEGMENTS; 
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

if (Count () >= MAX_SEGMENTS)
	return -1;
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

short CSegmentManager::Create (void)
{
	CSegment *newSegP, *curSegP; 
	int		i;
	short		nNewSeg, nNewSide, nCurSide = current->m_nSide; 
	ushort	newVerts [4]; 
	short		nSide; 

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

curSegP = Segment (current->m_nSegment); 

if (curSegP->Child (nCurSide) >= 0) {
	ErrorMsg ("Can not add a new segment to a side\nwhich already has a segment attached."); 
	return -1;
	}

undoManager.Begin (udSegments); 
// get new segment
m_bCreating = true;
nNewSeg = Add (); 
newSegP = Segment (nNewSeg); 

vertexManager.Add (newVerts, 4);
// define vert numbers for common side
for (i = 0; i < 4; i++) {
	newSegP->m_info.verts [oppSideVertTable [nCurSide][i]] = curSegP->m_info.verts [sideVertTable [nCurSide][i]]; 
	// define vert numbers for new side
	newSegP->m_info.verts [sideVertTable [nCurSide][i]] = newVerts [i]; 
	}

ComputeVertices (newVerts); 
newSegP->Setup ();
// define children and special child
newSegP->m_info.childFlags = 1 << oppSideTable [nCurSide]; /* only opposite side connects to current_segment */
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) /* no remaining children */
	newSegP->SetChild (i, (newSegP->m_info.childFlags & (1 << i)) ? current->m_nSegment : -1);

// define textures
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	if (newSegP->Child (nSide) < 0) {
		// if other segment does not have a child (therefore it has a texture)
		if ((curSegP->Child (nSide) < 0) && (curSegP->m_info.function == SEGMENT_FUNC_NONE)) {
			newSegP->m_sides [nSide].m_info.nBaseTex = curSegP->m_sides [nSide].m_info.nBaseTex; 
			newSegP->m_sides [nSide].m_info.nOvlTex = curSegP->m_sides [nSide].m_info.nOvlTex; 
			for (i = 0; i < 4; i++) 
				newSegP->m_sides [nSide].m_info.uvls [i].l = curSegP->m_sides [nSide].m_info.uvls [i].l; 
			} 
		}
	else {
		memset (newSegP->m_sides [nSide].m_info.uvls, 0, sizeof (newSegP->m_sides [nSide].m_info.uvls));
		}
	}

// define static light
newSegP->m_info.staticLight = curSegP->m_info.staticLight; 

// delete variable light if it exists
lightManager.DeleteVariableLight (CSideKey (current->m_nSegment, nCurSide)); 

// update current segment
curSegP->SetChild (nCurSide, nNewSeg); 
CSide* sideP = &curSegP->m_sides [nCurSide];
sideP->m_info.nBaseTex = 0; 
sideP->m_info.nOvlTex = 0; 
memset (sideP->m_info.uvls, 0, sizeof (sideP->m_info.uvls));
 
// link the new segment with any touching Segment ()
CVertex *vNewSeg = vertexManager.Vertex (newSegP->m_info.verts [0]);
for (CSegmentIterator si; si; si++) {
	if (si.Index () != nNewSeg) {
		// first check to see if Segment () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		CVertex *vSeg = vertexManager.Vertex (si->m_info.verts [0]);
		if (fabs (vNewSeg->v.x - vSeg->v.x) < 160.0 &&
			 fabs (vNewSeg->v.y - vSeg->v.y) < 160.0 &&
			 fabs (vNewSeg->v.z - vSeg->v.z) < 160.0)
			for (nNewSide = 0; nNewSide < 6; nNewSide++)
				for (nSide = 0; nSide < 6; nSide++)
					Link (nNewSeg, nNewSide, si.Index (), nSide, 3);
		}
	}
// auto align textures new segment
for (nNewSide = 0; nNewSide < 6; nNewSide++)
	AlignTextures (current->m_nSegment, nNewSide, nNewSeg, true, true); 
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

short CSegmentManager::Create (short nSegment, bool bCreate, byte nFunction, short nTexture, char* szError)
{
if ((szError != null) && DLE.IsD1File ()) {
	if (!bExpertMode)
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

bool CSegmentManager::CreateMatCen (short nSegment, bool bCreate, byte nType, bool bSetDefTextures, 
												 CMatCenter* matCens, CMineItemInfo& info, char* szError) 
{
if (info.count >= MAX_MATCENS) {
    ErrorMsg (szError);
	 return false;
	}
undoManager.Begin (udSegments);
if (0 > (nSegment = Create (nSegment, bCreate, nType)))
	return false;
matCens [info.count].Setup (nSegment, info.count, 0);
Segment (nSegment)->m_info.value = 
Segment (nSegment)->m_info.nMatCen = info.count++;
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
return CreateMatCen (nSegment, bCreate, SEGMENT_FUNC_EQUIPMAKER, bSetDefTextures, EquipMaker (0), m_matCenInfo [1], 
							"Maximum number of equipment makers reached");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateRobotMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
{
return CreateMatCen (nSegment, bCreate, SEGMENT_FUNC_ROBOTMAKER, bSetDefTextures, RobotMaker (0), m_matCenInfo [0], 
							"Maximum number of robot makers reached");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateReactor (short nSegment, bool bCreate, bool bSetDefTextures) 
{
return 0 <= Create (nSegment, bCreate, SEGMENT_FUNC_REACTOR, bSetDefTextures ? DLE.IsD1File () ? 10 : 357 : -1, "Flag goals are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateGoal (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture) 
{
return 0 <= Create (nSegment, bCreate, nType, bSetDefTextures ? nTexture : -1, "Flag goals are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateTeam (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture) 
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

int CSegmentManager::FuelCenterCount (void)
{
int nFuelCens = 0;
for (CSegmentIterator si; si; si++) 
	if ((si->m_info.function == SEGMENT_FUNC_FUELCEN) || (si->m_info.function == SEGMENT_FUNC_REPAIRCEN))
		nFuelCens++;
return nFuelCens;
}

// ----------------------------------------------------------------------------- 

short CSegmentManager::CreateFuelCenter (short nSegment, byte nType, bool bCreate, bool bSetDefTextures) 
{
// count number of fuel centers
int nFuelCen = FuelCenterCount ();
if (nFuelCen >= MAX_NUM_RECHARGERS) {
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
	if (nSegment < 0)
		return -1;
	if (bSetDefTextures) { // add energy spark walls to fuel center sides
		current->m_nSegment = nLastSeg;
		if (wallManager.Create (*current, WALL_ILLUSION, 0, KEY_NONE, -1, -1) != null) {
			CSideKey opp;
			if (OppositeSide (opp))
				wallManager.Create (opp, WALL_ILLUSION, 0, KEY_NONE, -1, -1);
			}
		Segment (nSegment)->Backup ();
		current->m_nSegment = nSegment;
		}
	}
undoManager.End ();
return nSegment;
}

// ----------------------------------------------------------------------------- 
// Calculate vertices when adding a new segment.

#define CURRENT_POINT(a) ((current->m_nPoint + (a))&0x03)

void CSegmentManager::ComputeVertices (ushort newVerts [4])
{
	CSegment*		curSegP; 
	CDoubleVector	A [8], B [8], C [8], D [8], E [8], a, b, c, d, v; 
	double			length; 
	ushort			nVertex; 
	short				i, points [4]; 
	CDoubleVector	center, oppCenter, newCenter, vNormal; 

curSegP = Segment (current->m_nSegment); 
for (i = 0; i < 4; i++)
	points [i] = CURRENT_POINT(i);
	// METHOD 1: orthogonal with right angle on new side and standard segment side
// TODO:
//	int add_segment_mode = ORTHOGONAL; 
switch (m_nAddMode) {
	case (ORTHOGONAL):
		{
		center = CalcSideCenter (*current); 
		oppCenter = CalcSideCenter (CSideKey (current->m_nSegment, oppSideTable [current->m_nSide])); 
		vNormal = CalcSideNormal (*current); 
		// set the length of the new segment to be one standard segment length
		// scale the vector
		vNormal *= 20; 
		// figure out new center
		newCenter = center + vNormal; 
		// new method: extend points 0 and 1 with vNormal, then move point 0 toward point 1.
		// point 0
		a = vNormal + *vertexManager.Vertex (curSegP->m_info.verts [sideVertTable [current->m_nSide][CURRENT_POINT(0)]]); 
		// point 1
		b = vNormal + *vertexManager.Vertex (curSegP->m_info.verts [sideVertTable [current->m_nSide][CURRENT_POINT(1)]]); 
		// center
		c = Average (a, b);
		// vector from center to point0 and its length
		d = a - c; 
		length = d.Mag (); 
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
		// now center the side along about the newCenter
		a = (A [0] + A [1] + A [2] + A [3]); 
		a /= 4;
		for (i = 0; i < 4; i++)
			A [i] += (newCenter - a); 
		// set the new vertices
		for (i = 0; i < 4; i++) {
			//nVertex = curSegP->m_info.verts [sideVertTable [current->m_nSide][i]]; 
			nVertex = newVerts [i];
			*vertexManager.Vertex (nVertex) = A [i]; 
			vertexManager.Vertex (nVertex)->Backup (); // update the newly added vertex instead of creating a new backup
			}
		}
	break; 

	// METHOD 2: orghogonal with right angle on new side
	case (EXTEND):
		{
		center = CalcSideCenter (*current); 
		oppCenter = CalcSideCenter (CSideKey (current->m_nSegment, oppSideTable [current->m_nSide])); 
		vNormal = CalcSideNormal (*current); 
		// calculate the length of the new segment
		vNormal *= Distance (center, oppCenter); 
		// set the new vertices
		for (i = 0; i < 4; i++) {
			nVertex = newVerts [i];
			*vertexManager.Vertex (nVertex) = *vertexManager.Vertex (curSegP->m_info.verts [sideVertTable [current->m_nSide][i]]) + vNormal; 
			vertexManager.Vertex (nVertex)->Backup (); // do not create a new backup right after adding this vertex
			}
		}
	break; 

	// METHOD 3: mirror relative to plane of side
	case(MIRROR):
		{
		// copy side's four points into A
		short nSide = current->m_nSide;
		for (i = 0; i < 4; i++) {
			A [i] = *vertexManager.Vertex (curSegP->m_info.verts [sideVertTable [nSide][i]]); 
			A [i + 4] = *vertexManager.Vertex (curSegP->m_info.verts [oppSideVertTable [nSide][i]]); 
			}

		// subtract point 0 from all points in A to form B points
		for (i = 0; i < 8; i++)
			B [i] = A [i] - A [0]; 

		// calculate angle to put point 1 in x - y plane by spinning on x - axis
		// then rotate B points on x - axis to form C points.
		// check to see if on x - axis already
		double angle1 = atan3 (B [1].v.z, B [1].v.y); 
		for (i = 0; i < 8; i++)
			C [i].Set (B [i].v.x, B [i].v.y * cos (angle1) + B [i].v.z * sin (angle1), B [i].v.z * cos (angle1) - B [i].v.y * sin (angle1)); 
		// calculate angle to put point 1 on x axis by spinning on z - axis
		// then rotate C points on z - axis to form D points
		// check to see if on z - axis already
		double angle2 = atan3 (C [1].v.y, C [1].v.x); 
		for (i = 0; i < 8; i++)
			D [i].Set (C [i].v.x * cos (angle2) + C [i].v.y * sin (angle2), C [i].v.y * cos (angle2) - C [i].v.x * sin (angle2), C [i].v.z); 

		// calculate angle to put point 2 in x - y plane by spinning on x - axis
		// the rotate D points on x - axis to form E points
		// check to see if on x - axis already
		double angle3 = atan3 (D [2].v.z, D [2].v.y); 
		for (i = 0; i < 8; i++) 
			E [i].Set (D [i].v.x, D [i].v.y * cos (angle3) + D [i].v.z * sin (angle3), D [i].v.z * cos (angle3) - D [i].v.y * sin (angle3)); 

		// now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
		// mirror new points on z axis
		for (i = 4; i < 8; i++)
			E [i].v.z = -E [i].v.z; 
		// now reverse rotations
		angle3 = -angle3;
		for (i = 4; i < 8; i++) 
			D [i].Set (E [i].v.x, E [i].v.y * cos (angle3) + E [i].v.z * sin (angle3), E [i].v.z * cos (angle3) - E [i].v.y * sin (angle3)); 
		angle2 = -angle2;
		for (i = 4; i < 8; i++) 
			C [i].Set (D [i].v.x * cos (angle2) + D [i].v.y * sin (angle2), D [i].v.y * cos (angle2) - D [i].v.x * sin (angle2), D [i].v.z); 
		angle1 = -angle1;
		for (i = 4; i < 8; i++) 
			B [i].Set (C [i].v.x, C [i].v.y * cos (angle1) + C [i].v.z * sin (angle1), C [i].v.z * cos (angle1) - C [i].v.y * sin (angle1)); 

		// and translate back
		nVertex = curSegP->m_info.verts [sideVertTable [current->m_nSide][0]]; 
		for (i = 4; i < 8; i++) 
			A [i] = B [i] + *vertexManager.Vertex (nVertex); 

		for (i = 0; i < 4; i++) {
			nVertex = newVerts [i];
			*vertexManager.Vertex (nVertex) = A [i + 4]; 
			vertexManager.Vertex (nVertex)->Backup ();
			}
		}
	}
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
double scale = textureManager.Textures (DLE.FileType (), nTexture)->Scale (nTexture);

undoManager.Begin (udSegments);
segP->m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
// set textures
CSide *sideP = segP->m_sides;
for (short nSide = 0; nSide < 6; nSide++, sideP++) {
	if (segP->Child (nSide) == -1) {
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

bool CSegmentManager::Define (short nSegment, byte nFunction, short nTexture)
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

void CSegmentManager::RemoveMatCenter (CSegment* segP, CMatCenter* matCens, CMineItemInfo& info, int nFunction)
{
if (info.count > 0) {
	// fill in deleted matcen
	int nDelMatCen = segP->m_info.nMatCen;
	if (nDelMatCen >= 0) {
		// copy last matCen in list to deleted matCen's position
		undoManager.Begin (udSegments | udMatCenters);
		segP->m_info.nMatCen = -1;
		segP->m_info.value = -1;
		if (nDelMatCen < --info.count) {
			memcpy (&matCens [nDelMatCen], &matCens [info.count], sizeof (matCens [0]));
			matCens [nDelMatCen].m_info.nFuelCen = nDelMatCen;
			// point owner of relocated matCen to that matCen's new position
			for (CSegmentIterator si; si; si++) {
				CSegment *segP = &(*si);
				if ((segP->m_info.function == nFunction) && (segP->m_info.nMatCen == info.count)) {
					segP->Backup ();
					segP->m_info.nMatCen = nDelMatCen;
					break;
					}
				}
			}
		// remove matCen from all robot maker triggers targetting it
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
segP->m_info.nMatCen = -1;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::Undefine (short nSegment)
{
	CSegment *segP = (nSegment < 0) ? current->Segment () : Segment (nSegment);

segP->Backup ();
nSegment = Index (segP);
if (segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER)
	RemoveMatCenter (segP, RobotMaker (0), m_matCenInfo [0], SEGMENT_FUNC_ROBOTMAKER);
else if (segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) 
	RemoveMatCenter (segP, EquipMaker (0), m_matCenInfo [1], SEGMENT_FUNC_EQUIPMAKER);
else if (segP->m_info.function == SEGMENT_FUNC_FUELCEN) { //remove all fuel cell walls
	undoManager.Begin (udSegments);
	CSide *sideP = segP->m_sides;
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		if (segP->Child (nSide) < 0)	// assume no wall if no child segment at the current side
			continue;
		CSegment* childSegP = Segment (segP->Child (nSide));
		if (childSegP->m_info.function == SEGMENT_FUNC_FUELCEN)	// don't delete if child segment is fuel center
			continue;
		// if there is a wall and it's a fuel cell delete it
		CSideKey key (nSegment, nSide);
		CWall *wallP = Wall (key);
		if ((wallP != null) && (wallP->Type () == WALL_ILLUSION) && (sideP->m_info.nBaseTex == (DLE.IsD1File () ? 322 : 333)))
			wallManager.Delete (sideP->m_info.nWall);
		// if there is a wall at the opposite side and it's a fuel cell delete it
		CSideKey opp;
		if (OppositeSide (key, opp)) {
			wallP = Wall (opp);
			if ((wallP != null) && (wallP->Type () == WALL_ILLUSION)) {
				CSide* oppSideP = Side (opp);
				if (oppSideP->m_info.nBaseTex == (DLE.IsD1File () ? 322 : 333))
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
	if (delSegP->Child (i) >= 0) {
		CSideKey opp, key (nDelSeg, i);
		if (OppositeSide (key, opp))
			wallManager.Delete (Side (opp)->m_info.nWall); 
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
	CTexture* texP = textureManager.Textures (DLE.FileType ());
	for (CSegmentIterator si; si; si++) {
		CSegment* segP = &(*si);
		for (short nChild = 0; nChild < MAX_SIDES_PER_SEGMENT; nChild++) {
			if (si->Child (nChild) == nDelSeg) {
				// remove nChild number and update nChild bitmask
				segP->SetChild (nChild, -1); 

				// define textures, (u, v) and light
				CSide *sideP = delSegP->m_sides + nChild;
				SetTextures (CSideKey (si.Index (), nChild), sideP->BaseTex (), sideP->OvlTex ()); 
				segP->SetUV (nChild, 0, 0); 
				double scale = texP [sideP->m_info.nBaseTex].Scale (sideP->m_info.nBaseTex);
				for (short j = 0; j < 4; j++) {
					//segP->m_sides [nChild].m_info.uvls [j].u = (short) ((double) defaultUVLs [j].u / scale); 
					//segP->m_sides [nChild].m_info.uvls [j].v = (short) ((double) defaultUVLs [j].v / scale); 
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

