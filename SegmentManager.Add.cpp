#include "mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

short CSegmentManager::Add (void) 
{ 
if (Count () >= MAX_SEGMENTS)
	return -1;
m_segments [m_free.Get ()].Clear ();
return Count ()++; 
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Create (void)
{
	CSegment *newSegP, *curSegP; 
	int		i;
	short		nNewSeg, nNewSide, nCurSide = current.m_nSide; 
	short		newVerts [4]; 
	short		nSegment, nSide; 

if (tunnelMaker.Active ()) {
	ErrorMsg (spline_error_message); 
	return FALSE; 
	}

curSegP = Segment (current.m_nSegment); 

if (Count () >= MAX_SEGMENTS) {
	ErrorMsg ("Cannot add a new cube because\nthe maximum number of cubes has been reached."); 
	return FALSE;
	}
if (vertexManager.Count () >= MAX_VERTICES) {
	ErrorMsg ("Cannot add a new cube because\nthe maximum number of vertices has been reached."); 
	return FALSE;
	}
if (curSegP->Child (nCurSide) >= 0) {
	ErrorMsg ("Can not add a new cube to a side\nwhich already has a cube attached."); 
	return FALSE;
	}

undoManager.SetModified (true); 
undoManager.Lock ();
// get new segment
nNewSeg = Count (); 
newSegP = Segment (nNewSeg); 

// define vert numbers for common side
for (i = 0; i < 4; i++) {
	newSegP->m_info.verts [oppSideVertTable [nCurSide][i]] = curSegP->m_info.verts [sideVertTable [nCurSide][i]]; 
	// define vert numbers for new side
	newVerts [i] = vertexManager.Add ();
	newSegP->m_info.verts [sideVertTable [nCurSide][i]] = newVerts [i]; 
	}

ComputeVertices (newVerts); 
newSegP->Setup ();
// define children and special child
newSegP->m_info.childFlags = 1 << oppSideTable [nCurSide]; /* only opposite side connects to current_segment */
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) /* no remaining children */
	newSegP->SetChild (i, (newSegP->m_info.childFlags & (1 << i)) ? current.m_nSegment : -1);

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
short index = lightManager.GetVariableLight (current.m_nSegment, nCurSide); 
if (index != -1) {
	lightManager.Count ()--; 
	// put last light in place of deleted light
	memcpy (lightManager.VariableLight (index), lightManager.VariableLight (lightManager.Count ()), sizeof (CVariableLight)); 
	}

// update current segment
curSegP->SetChild (nCurSide, nNewSeg); 
curSegP->m_sides [nCurSide].m_info.nBaseTex = 0; 
curSegP->m_sides [nCurSide].m_info.nOvlTex = 0; 
memset (curSegP->m_sides [nCurSide].m_info.uvls, 0, sizeof (curSegP->m_sides [nCurSide].m_info.uvls));
 
// update number of Segment () and vertices and clear vertexStatus
Count ()++;
for (int i = 0; i < 4; i++)
	vertexManager.Add ();

// link the new segment with any touching Segment ()
CVertex *vNewSeg = vertexManager.Vertex (newSegP->m_info.verts [0]);
CVertex *vSeg;
for (nSegment = 0; nSegment < Count (); nSegment++) {
	if (nSegment != nNewSeg) {
		// first check to see if Segment () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		vSeg = vertexManager.Vertex (Segment (nSegment)->m_info.verts [0]);
		if (fabs (vNewSeg->v.x - vSeg->v.x) < 10.0 &&
			 fabs (vNewSeg->v.y - vSeg->v.y) < 10.0 &&
			 fabs (vNewSeg->v.z - vSeg->v.z) < 10.0)
			for (nNewSide = 0; nNewSide < 6; nNewSide++)
				for (nSide = 0; nSide < 6; nSide++)
					Link (nNewSeg, nNewSide, nSegment, nSide, 3);
		}
	}
// auto align textures new segment
for (nNewSide = 0; nNewSide < 6; nNewSide++)
	AlignTextures (current.m_nSegment, nNewSide, nNewSeg, TRUE, TRUE); 
// set current segment to new segment
current.m_nSegment = nNewSeg; 
//		SetLinesToDraw(); 
DLE.MineView ()->Refresh (false); 
DLE.ToolView ()->Refresh (); 
undoManager.Unlock ();
return TRUE; 
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Create (short nSegment, bool bCreate, byte nFunction, short nTexture, char* szError)
{
if ((szError != null) && theMine->IsD1File ()) {
	if (!bExpertMode)
		ErrorMsg (szError);
	return false;
	}

bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
if (bCreate && !Create ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
DLE.MineView ()->DelayRefresh (true);
if (!Define (nSegment, nFunction, -1)) {
	undoManager.ResetModified (bUndo);
	DLE.MineView ()->DelayRefresh (false);
	return false; 
	}	
undoManager.Unlock ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
return true;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateMatCen (short nSegment, bool bCreate, byte nType, bool bSetDefTextures, 
												CMatCenter* matCens, CMineItemInfo& info, char* szError) 
{
if (info.count >= MAX_MATCENS) {
    ErrorMsg ("Maximum number of equipment makers reached");
	 return false;
	}
if (!Create (nSegment, bCreate, nType))
	return false;
matCens [info.count].Setup (nSegment, info.count, 0);
current.Segment ()->m_info.value = 
current.Segment ()->m_info.nMatCen = info.count++;
return true;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateEquipMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
{
return CreateMatCen (nSegment, bCreate, SEGMENT_FUNC_EQUIPMAKER, bSetDefTextures, EquipMaker (0), m_matCenInfo [1], 
							"Maximum number of equipment makers reached");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateRobotMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
{
return CreateMatCen (nSegment, bCreate, SEGMENT_FUNC_ROBOTMAKER, bSetDefTextures, RobotMaker (0), m_matCenInfo [1], 
							"Maximum number of robot makers reached");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateReactor (short nSegment, bool bCreate, bool bSetDefTextures) 
{
return Create (nSegment, bCreate, SEGMENT_FUNC_REACTOR, bSetDefTextures ? theMine->IsD1File () ? 10 : 357 : -1, "Flag goals are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateGoal (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture) 
{
return Create (nSegment, bCreate, nType, bSetDefTextures ? nTexture : -1, "Flag goals are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateTeam (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture) 
{
return Create (nSegment, bCreate, nType, bSetDefTextures ? nTexture : -1, "Team start positions are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateSkybox (short nSegment, bool bCreate) 
{
return Create (nSegment, bCreate, SEGMENT_FUNC_SKYBOX, -1, "Skyboxes are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateSpeedBoost (short nSegment, bool bCreate) 
{
return Create (nSegment, bCreate, SEGMENT_FUNC_SPEEDBOOST, -1, "Speed boost cubes are not available in Descent 1.");
}

// ----------------------------------------------------------------------------- 

int CSegmentManager::FuelCenterCount (void)
{
int nFuelCens = 0;
CSegment *segP = Segment (0);
for (int i = Count (); i > 0; i--, segP++)
	if ((segP->m_info.function == SEGMENT_FUNC_FUELCEN) || (segP->m_info.function == SEGMENT_FUNC_REPAIRCEN))
		nFuelCens++;
return nFuelCens;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateFuelCenter (short nSegment, byte nType, bool bCreate, bool bSetDefTextures) 
{
// count number of fuel centers
int nFuelCen = FuelCenterCount ();
CSegment *segP = Segment (0);
if (nFuelCen >= MAX_NUM_RECHARGERS) {
	ErrorMsg ("Maximum number of fuel centers reached.");
	return false;
	}


if (nType == SEGMENT_FUNC_FUELCEN) {
	short nLastSeg = current.m_nSegment;
	if (!Create (nSegment, bCreate, nType, bSetDefTextures ? theMine->IsD1File () ? 322 : 333 : -1))
		return false;
	if (bSetDefTextures) {
		short nNewSeg = current.m_nSegment;
		current.m_nSegment = nLastSeg;
		if (wallManager.Create (current, WALL_ILLUSION, 0, KEY_NONE, -1, -1)) {
			CSideKey opp;
			if (OppositeSide (opp))
				wallManager.Create (opp, WALL_ILLUSION, 0, KEY_NONE, -1, -1);
			}
		current.m_nSegment = nNewSeg;
		}
	}
else if (nType == SEGMENT_FUNC_REPAIRCEN) {
	if (!Create (nSegment, bCreate, nType, bSetDefTextures ? 433 : -1, "Repair centers are not available in Descent 1."))
		return false;
	}
else
	return false;
return true;
}

// ----------------------------------------------------------------------------- 
// Calculate vertices when adding a new segment.

#define CURRENT_POINT(a) ((current.m_nPoint + (a))&0x03)

void CSegmentManager::ComputeVertices (short newVerts [4])
{
	CSegment*		curSegP; 
	CDoubleVector	A [8], B [8], C [8], D [8], E [8], a, b, c, d, v; 
	double			length; 
	short				nVertex; 
	short				i, points [4]; 
	CDoubleVector	center, oppCenter, newCenter, orthog; 

curSegP = Segment (current.m_nSegment); 
for (i = 0; i < 4; i++)
	points [i] = CURRENT_POINT(i);
	// METHOD 1: orthogonal with right angle on new side and standard cube side
// TODO:
//	int add_segment_mode = ORTHOGONAL; 
switch (m_nAddMode) {
	case (ORTHOGONAL):
		{
		center = CalcSideCenter (current.m_nSegment, current.m_nSide); 
		oppCenter = CalcSideCenter (current.m_nSegment, oppSideTable [current.m_nSide]); 
		orthog = CalcSideNormal (current.m_nSegment, current.m_nSide); 
		// set the length of the new cube to be one standard cube length
		// scale the vector
		orthog *= 20; 
		// figure out new center
		newCenter = center + orthog; 
		// new method: extend points 0 and 1 with orthog, then move point 0 toward point 1.
		// point 0
		a = orthog + *vertexManager.Vertex (curSegP->m_info.verts [sideVertTable [current.m_nSide][CURRENT_POINT(0)]]); 
		// point 1
		b = orthog + *vertexManager.Vertex (curSegP->m_info.verts [sideVertTable [current.m_nSide][CURRENT_POINT(1)]]); 
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
		// point 2 is orthogonal to the vector 01 and the orthog vector
		c = -CrossProduct (A [points [0]] - A [points [1]], orthog);
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
			//nVertex = curSegP->m_info.verts [sideVertTable [current.m_nSide][i]]; 
			nVertex = newVerts [i];
			*vertexManager.Vertex (nVertex) = A [i]; 
			}
		}
	break; 

	// METHOD 2: orghogonal with right angle on new side
	case (EXTEND):
		{
		center = CalcSideCenter (current.m_nSegment, current.m_nSide); 
		oppCenter = CalcSideCenter (current.m_nSegment, oppSideTable [current.m_nSide]); 
		orthog = CalcSideNormal (current.m_nSegment, current.m_nSide); 
		// calculate the length of the new cube
		orthog *= Distance (center, oppCenter); 
		// set the new vertices
		for (i = 0; i < 4; i++) {
			CDoubleVector v = *vertexManager.Vertex (curSegP->m_info.verts [sideVertTable [current.m_nSide][i]]);
			v += orthog;
			*vertexManager.Vertex (newVerts [i]) = v; 
			}
		}
	break; 

	// METHOD 3: mirror relative to plane of side
	case(MIRROR):
		{
		// copy side's four points into A
		short nSide = current.m_nSide;
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
		nVertex = curSegP->m_info.verts [sideVertTable [current.m_nSide][0]]; 
		for (i = 4; i < 8; i++) 
			A [i] = B [i] + *vertexManager.Vertex (nVertex); 

		for (i = 0; i < 4; i++)
			*vertexManager.Vertex (newVerts [i]) = A [i + 4]; 
		}
	}
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::SetDefaultTexture (short nTexture)
{
if (nTexture < 0)
	return true;
short nSegment = current.m_nSegment;

CSegment *segP = Segment (nSegment);
double scale = textureManager.Textures (m_fileType, nTexture)->Scale (nTexture);
segP->m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
// set textures
CSide *sideP = segP->m_sides;
for (short nSide = 0; nSide < 6; nSide++, sideP++) {
	if (segP->Child (nSide) == -1) {
		SetTextures (CSideKey (nSegment, nSide), nTexture, 0);
		int i;
		for (i = 0; i < 4; i++) {
			sideP->m_info.uvls [i].u = (short) ((double) defaultUVLs [i].u / scale);
			sideP->m_info.uvls [i].v = (short) ((double) defaultUVLs [i].v / scale);
			sideP->m_info.uvls [i].l = defaultUVLs [i].l;
			}
		Segment (nSegment)->SetUV (nSide, 0, 0);
		}
	}
return true;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Define (short nSegment, byte nFunction, short nTexture)
{
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
Undefine (nSegment);
CSegment *segP = (nSegment < 0) ? current.Segment () : Segment (nSegment);
segP->m_info.function = nFunction;
segP->m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
SetDefaultTexture (nTexture);
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::RemoveMatCen (CSegment* segP, CMatCenter* matCens, CMineItemInfo& info)
{
if (info.count > 0) {
	// fill in deleted matcen
	int nDelMatCen = segP->m_info.nMatCen;
	if ((nDelMatCen >= 0) && (nDelMatCen < --info.count)) {
		// copy last matCen in list to deleted matCen's position
		memcpy (&matCens [nDelMatCen], &matCens [info.count], sizeof (matCens [0]));
		matCens [nDelMatCen].m_info.nFuelCen = nDelMatCen;
		segP->m_info.nMatCen = -1;
		// point owner of relocated matCen to that matCen's new position
		CSegment *segP = Segment (0);
		for (short nSegment = Count (); nSegment; nSegment--, segP++)
			if ((segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) && (segP->m_info.nMatCen == info.count)) {
				segP->m_info.nMatCen = nDelMatCen;
				break;
				}
		}
	// remove matCen from all robot maker triggers targetting it
	CSideKey key = (-Index (segP) - 1, 0); 
	for (int nClass = 0; nClass < 2; nClass++) {
		CTrigger* trigP = triggerManager.Trigger (0, nClass);
		for (int nTrigger = triggerManager.Count (nClass); nTrigger; nTrigger--, trigP++)
			if (trigP->m_info.type == TT_MATCEN)
				trigP->Delete (key);
		}
	}
segP->m_info.nMatCen = -1;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::Undefine (short nSegment)
{
	CSegment *segP = (nSegment < 0) ? current.Segment () : Segment (nSegment);

nSegment = short (segP - Segment (0));
if (segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER)
	RemoveMatCen (segP, RobotMaker (0), theMine->Info ().botGen);
else if (segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) 
	RemoveMatCen (segP, EquipMaker (0), theMine->Info ().equipGen);
else if (segP->m_info.function == SEGMENT_FUNC_FUELCEN) { //remove all fuel cell walls
	CSegment *childSegP;
	CSide *oppSideP, *sideP = segP->m_sides;
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		if (segP->Child (nSide) < 0)	// assume no wall if no child segment at the current side
			continue;
		childSegP = Segment (segP->Child (nSide));
		if (childSegP->m_info.function == SEGMENT_FUNC_FUELCEN)	// don't delete if child segment is fuel center
			continue;
		// if there is a wall and it's a fuel cell delete it
		CWall *wallP = Wall (CSideKey (nSegment, nSide));
		if ((wallP != null) && (wallP->m_info.type == WALL_ILLUSION) && (sideP->m_info.nBaseTex == (theMine->IsD1File () ? 322 : 333)))
			wallManager.Delete (sideP->m_info.nWall);
		// if there is a wall at the opposite side and it's a fuel cell delete it
		CSideKey key (nSegment, nSide);
		CSideKey opp;
		if (OppositeSide (key, opp) &&
			 (wallP = Wall (key)) && (wallP->m_info.type == WALL_ILLUSION)) {
			oppSideP = Side (opp);
			if (oppSideP->m_info.nBaseTex == (theMine->IsD1File () ? 322 : 333))
				wallManager.Delete (oppSideP->m_info.nWall);
			}
		}
	}
segP->m_info.childFlags &= ~(1 << MAX_SIDES_PER_SEGMENT);
segP->m_info.function = SEGMENT_FUNC_NONE;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::Delete (short nDelSeg)
{
	CSegment			*segP, *delSegP, *childSegP; 
	CGameObject		*objP; 
	CTrigger			*trigP;
	ushort			nSegment, nRealSeg; 
	short				child; 
	short				i, j; 

if (Count () < 2)
	return; 
if (nDelSeg < 0)
	nDelSeg = current.m_nSegment; 
if (nDelSeg < 0 || nDelSeg >= Count ()) 
	return; 

undoManager.SetModified (true);
undoManager.Lock ();
delSegP = Segment (nDelSeg); 
UndefineSegment (nDelSeg);

// delete any variable lights that use this segment
for (int nSide = 0; nSide < 6; nSide++) {
	CSideKey key (nDelSeg, nSide);
	triggerManager.DeleteTargets (key); 
	lightManager.DeleteVariableLight (key);
	}

	// delete any Walls () within segment (if defined)
DeleteWalls (nDelSeg); 

// delete any Walls () on child Segment () that connect to this segment
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	nChild = delSegP->Child (i); 
	if (nChild >= 0 && nChild < Count ()) {
		CSideKey opp, key (nDelSeg, i);
		if (OppositeSide (key, opp)
			wallManager.Delete (Side (opp)->m_info.nWall); 
		}

	// delete any Objects () within segment
for (i = objectManager.Count (); i >= 0; i--) {
	if (Objects (i)->m_info.nSegment == nDelSeg) {
		objectManager.Delete (i); 
		}
	}
	for (i = 0; i < RobotMakerCount (); i++)
		if (RobotMaker (i)->m_info.nSegment > nDelSeg)
			RobotMaker (i)->m_info.nSegment--;
	for (i = 0; i < MineInfo ().equipGen.count; i++)
		if (EquipMaker (i)->m_info.nSegment > nDelSeg)
			EquipMaker (i)->m_info.nSegment--;
	// delete any control segP with this segment
	for (j = (ushort)MineInfo ().control.count - 1; j >= 0; j--) {
		int count = ReactorTriggers (i)->m_count;
		for (j = count - 1; j > 0; j--) {
			if (ReactorTriggers (i)->Segment (j) == nDelSeg) {
				// move last segment into this spot
				ReactorTriggers (i)->Delete (j);
				}
			}
		}

	// update secret cube number if out of range now
	nSegment = (ushort) SecretSegment (); 
	if (nSegment >= Count () || nSegment == nDelSeg)
		SecretSegment () = 0; 

	// update segment flags
	delSegP->m_info.wallFlags &= ~MARKED_MASK; 

	// unlink any children with this segment number
	CTexture* texP = textureManager.Textures (m_fileType);
	for (nSegment = 0, segP = Segment (0); nSegment < Count (); nSegment++, segP++) {
		for (nChild = 0; nChild < MAX_SIDES_PER_SEGMENT; nChild++) {
			if (segP->Child (nChild) == nDelSeg) {

				// subtract by 1 if segment is above deleted segment
				current.m_nSegment = nSegment; 
				if (nSegment > nDelSeg) 
					current.m_nSegment--; 

				// remove nChild number and update nChild bitmask
				segP->SetChild (nChild, -1); 

				// define textures, (u, v) and light
				CSide *sideP = delSegP->m_sides + nChild;
				SetTextures (CSideKey (nSegment, nChild), sideP->m_info.nBaseTex, sideP->m_info.nOvlTex); 
				Segment (nSegment)->SetUV (nChild, 0, 0); 
				double scale = texP [sideP->m_info.nBaseTex].Scale (sideP->m_info.nBaseTex);
				for (i = 0; i < 4; i++) {
					//segP->m_sides [nChild].m_info.uvls [i].u = (short) ((double) defaultUVLs [i].u / scale); 
					//segP->m_sides [nChild].m_info.uvls [i].v = (short) ((double) defaultUVLs [i].v / scale); 
					segP->m_sides [nChild].m_info.uvls [i].l = delSegP->m_sides [nChild].m_info.uvls [i].l; 
				}
			}
		}
	}

	// move other Segment () to deleted segment location
	if (nDelSeg != Count ()-1) { // if this is not the last segment

		// mark each segment with it's real number
		nRealSeg = 0; 
		for (nSegment = 0, segP = Segment (0); nSegment < Count (); nSegment++, segP++)
			if(nDelSeg != nSegment)
				segP->m_info.nIndex = nRealSeg++; 

		// replace all children with real numbers
		for (nSegment = 0, segP = Segment (0); nSegment < Count (); nSegment++, segP++) {
			for (nChild = 0; nChild < MAX_SIDES_PER_SEGMENT; nChild++) {
				if (segP->m_info.childFlags & (1 << nChild)
					&& segP->Child (nChild) >= 0 && segP->Child (nChild) < Count ()) { // debug int
					childSegP = Segment (segP->Child (nChild)); 
					segP->SetChild (nChild, childSegP->m_info.nIndex); 
				}
			}
		}

		// replace all wall segP numbers with real numbers
		for (i = 0; i < MineInfo ().walls.count; i++) {
			nSegment = (short) Walls (i)->m_nSegment; 
			if (nSegment < Count ()) {
				segP = Segment (nSegment); 
				Walls (i)->m_nSegment = segP->m_info.nIndex; 
				} 
			else {
				Walls (i)->m_nSegment = 0; // int wall segment number
			}
		}

		// replace all trigger segP numbers with real numbers
		for (i = NumGeoTriggers (), trigP = Triggers (0); i; i--, trigP++) {
			for (j = 0; j < trigP->m_count; j++) {
				if (Count () > (nSegment = trigP->Segment (j)))
					trigP->Segment (j) = Segment (nSegment)->m_info.nIndex; 
				else {
					DeleteTargetFromTrigger (trigP, j, 0);
					j--;
					}
				}
			}

		// replace all trigger segP numbers with real numbers
		for (i = NumObjTriggers (), trigP = ObjTriggers (0); i; i--, trigP++) {
			for (j = 0; j < trigP->m_count; j++) {
				if (Count () > (nSegment = trigP->Segment (j)))
					trigP->Segment (j) = Segment (nSegment)->m_info.nIndex; 
				else {
					DeleteTargetFromTrigger (trigP, j, 0);
					j--;
					}
				}
			}

		// replace all object segP numbers with real numbers
		for (i = 0; i < MineInfo ().objects.count; i++) {
			objP = Objects (i); 
			if (Count () > (nSegment = objP->m_info.nSegment))
				objP->m_info.nSegment = Segment (nSegment)->m_info.nIndex; 
			else
				objP->m_info.nSegment = 0; // int object segment number
			}

		// replace robot centers segP numbers with real numbers
		for (i = 0; i < MineInfo ().botGen.count; i++) {
			if (Count () > (nSegment = RobotMaker (i)->m_info.nSegment))
				RobotMaker (i)->m_info.nSegment = Segment (nSegment)->m_info.nIndex; 
			else
				RobotMaker (i)->m_info.nSegment = 0; // int robot center nSegment
			}

		// replace equipment centers segP numbers with real numbers
		for (i = 0; i < MineInfo ().equipGen.count; i++) {
			if (Count () > (nSegment = EquipMaker (i)->m_info.nSegment))
				EquipMaker (i)->m_info.nSegment = Segment (nSegment)->m_info.nIndex; 
			else
				EquipMaker (i)->m_info.nSegment = 0; // int robot center nSegment
			}

		// replace control segP numbers with real numbers
		for (i = 0; i < MineInfo ().control.count; i++) {
			for (j = 0; j < ReactorTriggers (i)->m_count; j++) {
				if (Count () > (nSegment = ReactorTriggers (i)->Segment (j)))
					ReactorTriggers (i)->Segment (j) = Segment (nSegment)->m_info.nIndex; 
				else 
					ReactorTriggers (i)->Segment (j) = 0; // int control center segment number
			}
		}

		// replace variable light segP numbers with real numbers
		for (i = 0; i < lightManager.Count (); i++) {
			if (Count () > (nSegment = VariableLights (i)->m_nSegment))
				VariableLights (i)->m_nSegment = Segment (nSegment)->m_info.nIndex; 
			else 
				VariableLights (i)->m_nSegment = 0; // int object segment number
			}

		// replace secret cubenum with real number
		if (Count () > (nSegment = (ushort) SecretSegment ()))
			SecretSegment () = Segment (nSegment)->m_info.nIndex; 
		else
			SecretSegment () = 0; // int secret cube number

		// move remaining Segment () down by 1
  }
#if 1
		if (int segC = (--Count () - nDelSeg)) {
			memcpy (Segment (nDelSeg), Segment (nDelSeg + 1), segC * sizeof (CSegment));
			memcpy (LightColors (nDelSeg), LightColors (nDelSeg + 1), segC * 6 * sizeof (CColor));
			}
#else
		for (nSegment = nDelSeg; nSegment < (Count ()-1); nSegment++) {
			segP = Segment (nSegment); 
			childSegP = Segment (nSegment + 1); 
			memcpy(segP, childSegP, sizeof (CSegment)); 
			}
  Count ()-- ; 
#endif


  // delete all unused vertices
  vertexManager.DeleteUnused (); 

  // make sure current segment numbers are valid
  if (selections [0].m_nSegment >= Count ()) 
	  selections [0].m_nSegment--; 
  if (selections [1].m_nSegment >= Count ()) 
	  selections [1].m_nSegment--; 
  if (selections [0].m_nSegment < 0) 
	  selections [0].m_nSegment = 0; 
  if (selections [1].m_nSegment < 0) 
	  selections [1].m_nSegment = 0; 
DLE.MineView ()->Refresh (false); 
DLE.ToolView ()->Refresh (); 
undoManager.Unlock ();
}

// ----------------------------------------------------------------------------- 

