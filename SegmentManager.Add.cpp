#include "mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

short Add (void) 
{ 
return (Count () < MAX_SEGMENTS) ? Count ()++ : -1; 
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Create (void)
{
	CSegment *newSegP, *curSegP; 
	short i, nNewSeg, nNewSide, nCurSide = current.m_nSide; 
	short newVerts [4]; 
	short nSegment, nSide; 

if (tunnelMaker.Active ()) {
	ErrorMsg (spline_error_message); 
	return FALSE; 
	}

curSegP = GetSegment (current.m_nSegment); 

if (Count () >= MAX_SEGMENTS) {
	ErrorMsg ("Cannot add a new cube because\nthe maximum number of cubes has been reached."); 
	return FALSE;
	}
if (vertexManager.Count () >= MAX_VERTICES) {
	ErrorMsg ("Cannot add a new cube because\nthe maximum number of vertices has been reached."); 
	return FALSE;
	}
if (curSegP->GetChild (nCurSide) >= 0) {
	ErrorMsg ("Can not add a new cube to a side\nwhich already has a cube attached."); 
	return FALSE;
	}

undoManager.SetModified (true); 
undoManager.Lock ();
// get new verts
newVerts [0] = VertCount (); 
newVerts [1] = newVerts [0] + 1; 
newVerts [2] = newVerts [0] + 2; 
newVerts [3] = newVerts [0] + 3; 

// get new segment
nNewSeg = Count (); 
newSegP = GetSegment (nNewSeg); 

// define vertices
DefineVertices (newVerts); 

// define vert numbers for common side
newSegP->m_info.verts [oppSideVertTable [nCurSide][0]] = curSegP->m_info.verts [sideVertTable [nCurSide][0]]; 
newSegP->m_info.verts [oppSideVertTable [nCurSide][1]] = curSegP->m_info.verts [sideVertTable [nCurSide][1]]; 
newSegP->m_info.verts [oppSideVertTable [nCurSide][2]] = curSegP->m_info.verts [sideVertTable [nCurSide][2]]; 
newSegP->m_info.verts [oppSideVertTable [nCurSide][3]] = curSegP->m_info.verts [sideVertTable [nCurSide][3]]; 

// define vert numbers for new side
newSegP->m_info.verts [sideVertTable [nCurSide][0]] = newVerts [0]; 
newSegP->m_info.verts [sideVertTable [nCurSide][1]] = newVerts [1]; 
newSegP->m_info.verts [sideVertTable [nCurSide][2]] = newVerts [2]; 
newSegP->m_info.verts [sideVertTable [nCurSide][3]] = newVerts [3]; 

newSegP->Setup ();
// define children and special child
newSegP->m_info.childFlags = 1 << oppSideTable [nCurSide]; /* only opposite side connects to current_segment */
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) /* no remaining children */
	newSegP->SetChild (i, (newSegP->m_info.childFlags & (1 << i)) ? current.m_nSegment : -1);

// define textures
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	if (newSegP->GetChild (nSide) < 0) {
		// if other segment does not have a child (therefore it has a texture)
		if ((curSegP->GetChild (nSide) < 0) && (curSegP->m_info.function == SEGMENT_FUNC_NONE)) {
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
short index = GetVariableLight (current.m_nSegment, nCurSide); 
if (index != -1) {
	lightManager.Count ()--; 
	// put last light in place of deleted light
	memcpy( VariableLights (index), VariableLights (lightManager.Count ()), sizeof (CVariableLight)); 
	}

// update current segment
curSegP->SetChild (nCurSide, nNewSeg); 
curSegP->m_sides [nCurSide].m_info.nBaseTex = 0; 
curSegP->m_sides [nCurSide].m_info.nOvlTex = 0; 
memset (curSegP->m_sides [nCurSide].m_info.uvls, 0, sizeof (curSegP->m_sides [nCurSide].m_info.uvls));
 
// update number of GetSegment () and vertices and clear vertexStatus
Count ()++;
for (int i = 0; i < 4; i++)
	vertexManager.GetVertex (VertCount ()++)->m_status = 0;

// link the new segment with any touching GetSegment ()
CVertex *vNewSeg = vertexManager.GetVertex (newSegP->m_info.verts [0]);
CVertex *vSeg;
for (nSegment = 0; nSegment < Count (); nSegment++) {
	if (nSegment != nNewSeg) {
		// first check to see if GetSegment () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		vSeg = vertexManager.GetVertex (GetSegment (nSegment)->m_info.verts [0]);
		if (fabs (vNewSeg->v.x - vSeg->v.x) < 10.0 &&
			 fabs (vNewSeg->v.y - vSeg->v.y) < 10.0 &&
			 fabs (vNewSeg->v.z - vSeg->v.z) < 10.0)
			for (nNewSide = 0; nNewSide < 6; nNewSide++)
				for (nSide = 0; nSide < 6; nSide++)
					LinkSegments (nNewSeg, nNewSide, nSegment, nSide, 3);
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

bool CSegmentManager::CreateReactor (short nSegment, bool bCreate, bool bSetDefTextures) 
{
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
if (bCreate && !Create ()) {
	undoManager.ResetModified (bUndo);
	return false; 
	}
if (!Define (nSegment, SEGMENT_FUNC_CONTROLCEN, bSetDefTextures ? (theMine->IsD1File () ? 10 : 357 : -1)) {
	undoManager.ResetModified (bUndo);
	return false; 
	}	
if (bCreate) {
	if (!objectManager.Copy (OBJ_CNTRLCEN, nSegment)) {
		undoManager.ResetModified (bUndo);
		return false; 
		}	
	current.Object ()->m_info.id = theMine->IsD1File () ? 0 : 2;
	triggerManager.UpdateReactor ();
	}
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Create (short nSegment, short nFunction, bool bCreate = true, short nTexture = -1, char* szError = null)
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
if (!DefineSegment (nSegment, nFunction, -1)) {
	undoManager.ResetModified (bUndo);
	DLE.MineView ()->DelayRefresh (false);
	return false; 
	}	
undoManager.Unlock ();
return true;
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateEquipMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
{
int nMatCen = (int) theMine->Info ().equipGen.count;
if (nMatCen >= MAX_ROBOT_MAKERS) {
    ErrorMsg ("Maximum number of equipment makers reached");
	 return false;
	}
if (!Create (nSegment, SEGMENT_FUNC_EQUIPMAKER, bCreate))
	return false;
GetEquipGen (nMatCen)->Setup (nSegment, nMatCen, 0);
GetSegment (current.m_nSegment)->m_info.value = 
GetSegment (current.m_nSegment)->m_info.nMatCen = nMatCen;
theMine->Info ().equipGen.count++;
return true;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateRobotMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
{
int nMatCen = (int) theMine->Info ().botGen.count;
if (nMatCen >= MAX_ROBOT_MAKERS) {
    ErrorMsg ("Maximum number of robot makers reached");
	 return false;
	}
if (!Create (nSegment, SEGMENT_FUNC_ROBOTMAKER, bCreate))
	return false;
GetBotGen (nMatCen)->Setup (nSegment, nMatCen, 8);
GetSegment (current.m_nSegment)->m_info.value = 
GetSegment (current.m_nSegment)->m_info.nMatCen = nMatCen;
theMine->Info ().botGen.count++;
undoManager.Unlock ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->Refresh ();
return true;
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
CSegment *segP = GetSegment (0);
for (int i = SegCount (); i > 0; i--, segP++)
	if ((segP->m_info.function == SEGMENT_FUNC_FUELCEN) || (segP->m_info.function == SEGMENT_FUNC_REPAIRCEN))
		nFuelCens++;
return nFuelCens;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::CreateFuelCenter (short nSegment, byte nType, bool bCreate, bool bSetDefTextures) 
{
// count number of fuel centers
int nFuelCen = FuelCenterCount ();
CSegment *segP = Segments (0);
if (nFuelCen >= MAX_NUM_FUELCENS) {
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
		if (wallManager.Create (current.m_nSegment, current.m_nSide, WALL_ILLUSION, 0, KEY_NONE, -1, -1)) {
			short nOppSeg, nOppSide;
			if (GetOppositeSide (current, nOppSeg, nOppSide)
				wallManager.Create (nOppSeg, nOppSide, WALL_ILLUSION, 0, KEY_NONE, -1, -1);
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

curSegP = GetSegment (current.m_nSegment); 
for (i = 0; i < 4; i++)
	points [i] = CURRENT_POINT(i);
	// METHOD 1: orthogonal with right angle on new side and standard cube side
// TODO:
//	int add_segment_mode = ORTHOGONAL; 
switch (m_nAddMode) {
	case (ORTHOGONAL):
		{
		center = CalcCenter (current.m_nSegment, current.m_nSide); 
		oppCenter = CalcSideCenter (current.m_nSegment, oppSideTable [current.m_nSide]); 
		orthog = CalcSideNormal (current.m_nSegment, current.m_nSide); 
		// set the length of the new cube to be one standard cube length
		// scale the vector
		orthog *= 20; 
		// figure out new center
		newCenter = center + orthog; 
		// new method: extend points 0 and 1 with orthog, then move point 0 toward point 1.
		// point 0
		a = orthog + *vertexManager.GetVertex (curSegP->m_info.verts [sideVertTable [current.m_nSide][CURRENT_POINT(0)]]); 
		// point 1
		b = orthog + *vertexManager.GetVertex (curSegP->m_info.verts [sideVertTable [current.m_nSide][CURRENT_POINT(1)]]); 
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
			*vertexManager.GetVertex (nVertex) = A [i]; 
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
			CDoubleVector v = *vertexManager.GetVertex (curSegP->m_info.verts [sideVertTable [current.m_nSide][i]]);
			v += orthog;
			*vertexManager.GetVertex (newVerts [i]) = v; 
			}
		}
	break; 

	// METHOD 3: mirror relative to plane of side
	case(MIRROR):
		{
		// copy side's four points into A
		short nSide = current.m_nSide;
		for (i = 0; i < 4; i++) {
			A [i] = *vertexManager.GetVertex (curSegP->m_info.verts [sideVertTable [nSide][i]]); 
			A [i + 4] = *vertexManager.GetVertex (curSegP->m_info.verts [oppSideVertTable [nSide][i]]); 
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
			A [i] = B [i] + *vertexManager.GetVertex (nVertex); 

		for (i = 0; i < 4; i++)
			*vertexManager.GetVertex (newVerts [i]) = A [i + 4]; 
		}
	}
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::SetDefaultTexture (short nTexture)
{
if (nTexture < 0)
	return true;
short nSegment = current.m_nSegment;
short nOppSeg, nOppSide;
CSegment *segP = GetSegment (nSegment);
CSide *sideP = segP->m_sides;
double scale = textureManager.Textures (m_fileType, nTexture)->Scale (nTexture);
segP->m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
// set textures
for (short nSide = 0; nSide < 6; nSide++, sideP++) {
	if (segP->GetChild (nSide) == -1) {
		SetTextures (nSegment, nSide, nTexture, 0);
		int i;
		for (i = 0; i < 4; i++) {
			sideP->m_info.uvls [i].u = (short) ((double) defaultUVLs [i].u / scale);
			sideP->m_info.uvls [i].v = (short) ((double) defaultUVLs [i].v / scale);
			sideP->m_info.uvls [i].l = defaultUVLs [i].l;
			}
		GetSegment (nSegment)->SetUV (nSide, 0, 0);
		}
	}
return true;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Define (short nSegment, byte type, short nTexture)
{
bool bUndo = undoManager.SetModified (true);
undoManager.Lock ();
Undefine (nSegment);
CSegment *segP = (nSegment < 0) ? current.Segment () : GetSegment (nSegment);
segP->m_info.function = type;
segP->m_info.childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
SetDefaultTexture (nTexture);
undoManager.Unlock ();
DLE.MineView ()->Refresh ();
return true;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::Undefine (short nSegment)
{
	CSegment *segP = (nSegment < 0) ? current.Segment () : GetSegment (nSegment);

nSegment = short (segP - Segments (0));
if (segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) {
	// remove matcen
	int nMatCens = (int) theMine->Info ().botGen.count;
	if (nMatCens > 0) {
		// fill in deleted matcen
		int nDelMatCen = segP->m_info.nMatCen;
		if ((nDelMatCen >= 0) && (nDelMatCen < --nMatCens)) {
			memcpy (GetBotGen (nDelMatCen), GetBotGen (nMatCens), sizeof (CRobotMaker));
			GetBotGen (nDelMatCen)->m_info.nFuelCen = nDelMatCen;
			segP->m_info.nMatCen = -1;
			}
		theMine->Info ().botGen.count--;
		int i;
		for (int i = 0; i < 6; i++)
			DeleteTriggerTargets (nSegment, i);
		CSegment *segP = Segments (0);
		for (i = SegCount (); i; i--, s++)
			if ((segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) && (segP->m_info.nMatCen == nMatCens)) {
				segP->m_info.nMatCen = nDelMatCen;
				break;
				}
		}
	segP->m_info.nMatCen = -1;
	}
else if (segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) {
	// remove matcen
	int nMatCens = (int) theMine->Info ().equipGen.count;
	if (nMatCens > 0) {
		// fill in deleted matcen
		int nDelMatCen = segP->m_info.nMatCen;
		if ((nDelMatCen >= 0) && (nDelMatCen < --nMatCens)) {
			memcpy (GetEquipGen (nDelMatCen), GetEquipGen (nMatCens), sizeof (CRobotMaker));
			GetEquipGen (nDelMatCen)->m_info.nFuelCen = nDelMatCen;
			segP->m_info.nMatCen = -1;
			}
		theMine->Info ().equipGen.count--;
		int i;
		for (int i = 0; i < 6; i++)
			DeleteTriggerTargets (nSegment, i);
		CSegment *segP = Segments (0);
		nDelMatCen += (int) theMine->Info ().botGen.count;
		for (i = SegCount (); i; i--, s++)
			if ((segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER) && (s->m_info.nMatCen == nMatCens)) {
				segP->m_info.nMatCen = nDelMatCen;
				break;
			}
		}
	segP->m_info.nMatCen = -1;
	}
else if (segP->m_info.function == SEGMENT_FUNC_FUELCEN) { //remove all fuel cell walls
	CSegment *childSegP;
	CSide *oppSideP, *sideP = segP->m_sides;
	CWall *wallP;
	short nOppSeg, nOppSide;
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		if (segP->GetChild (nSide) < 0)	// assume no wall if no child segment at the current side
			continue;
		childSegP = GetSegment (segP->GetChild (nSide));
		if (childSegP->m_info.function == SEGMENT_FUNC_FUELCEN)	// don't delete if child segment is fuel center
			continue;
		// if there is a wall and it's a fuel cell delete it
		if ((wallP = GetWall (nSegment, nSide)) && 
			 (wallP->m_info.type == WALL_ILLUSION) && (sideP->m_info.nBaseTex == (theMine->IsD1File () ? 322 : 333)))
			wallManager.Delete (sideP->m_info.nWall);
		// if there is a wall at the opposite side and it's a fuel cell delete it
		if (GetOppositeSide (CSideKey (nSegment, nSide), nOppSeg, nOppSide) &&
			 (wallP = GetWall (nSegment, nSide)) && (wallP->m_info.type == WALL_ILLUSION)) {
			oppSideP = Segments (nOppSeg)->m_sides + nOppSide;
			if (oppSideP->m_info.nBaseTex == (theMine->IsD1File () ? 322 : 333))
				DeleteWall (oppSideP->m_info.nWall);
			}
		}
	}
segP->m_info.childFlags &= ~(1 << MAX_SIDES_PER_SEGMENT);
segP->m_info.function = SEGMENT_FUNC_NONE;
}

// ----------------------------------------------------------------------------- 

