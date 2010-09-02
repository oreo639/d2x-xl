// Segment.cpp

#include "stdafx.h"
#include "dle-xp-res.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "cfile.h"
#include "texturemanager.h"
#include "palette.h"
#include "dle-xp.h"
#include "robot.h"
#include "cfile.h"

CSegmentManager segmentManager;

// -----------------------------------------------------------------------------

CWall* CSegmentManager::Wall (short nSegment, short nSide)
{
current.Get (nSegment, nSide);
return wallManager.Walls (Segments (nSegment)->m_sides [nSide].m_info.nWall);
}

// -----------------------------------------------------------------------------

void CSegmentManager::GetTextures (short nSegment, short nSide, short& nBaseTex, short& nOvlTex)
{
current.Get (nSegment, nSide);
GetSide (nSegment, nSide)->GetTextures (nBaseTex, nOvlTex);
}

// -----------------------------------------------------------------------------

CDoubleVector CSegmentManager::CalcSideNormal (short nSegment, short nSide)
{
current.Get (nSegment, nSide);

	short* segVertP = Segments (nSegment)->m_info.verts;
	byte*	sideVertP = &sideVertTable [nSide][0];
	CDoubleVector	v;

return -Normal (*Vertices (segVertP [sideVertP [0]]), *Vertices (segVertP [sideVertP [1]]), *Vertices (segVertP [sideVertP [3]]));
}

// -----------------------------------------------------------------------------

CDoubleVector CSegmentManager::CalcSideCenter (short nSegment, short nSide)
{
current.Get (nSegment, nSide);

	short*	segVertP = Segments (nSegment)->m_info.verts;
	byte*		sideVertP = &sideVertTable [nSide][0];
	CDoubleVector	v;

for (int i = 0; i < 4; i++)
	v += *Vertices (segVertP [sideVertP [i]]);
v /= 4.0;
return v;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::DeleteWalls (short nSegment)
{
	CSide *sideP = Segments (nSegment)->m_sides; 

for (int i = MAX_SIDES_PER_SEGMENT; i; i--, sideP++)
	wallManager.Delete (sideP [i].m_info.nWall);
}

// -----------------------------------------------------------------------------

void CSegmentManager::UpdateWalls (short nOldWall, short nNewWall)
{
	CSegment* segP = m_segments;

for (int i = 0; i < m_nSegments; i++, segP++) {
	CSide* sideP = segP->m_sides;
	for (int j = 0; j < 6; 
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::UpdateVertex (short nOldVert, short nNewVert)
{
CSegment *segP = Segments (0);
for (nSegment = SegCount (); nSegment; nSegment--, segP++) {
	for (nVertex = 0; nVertex < 8; nVertex++) {
		short* vertP = segP->m_info.verts;
		if (vertP [nVertex] == nOldVert)
			vertP [nVertex] == nNewVert;
		}
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::Delete (short nDelSeg)
{
	CSegment			*segP, *delSegP, *childSegP; 
	CGameObject		*objP; 
	CTrigger			*trigP;
	ushort			nSegment, real_segnum; 
	short				child; 
	short				i, j; 

if (SegCount () < 2)
	return; 
if (nDelSeg < 0)
	nDelSeg = Current ()->nSegment; 
if (nDelSeg < 0 || nDelSeg >= SegCount ()) 
	return; 

undoManager.SetModified (TRUE);
undoManager.Lock ();
delSegP = Segments (nDelSeg); 
UndefineSegment (nDelSeg);

// delete any flickering lights that use this segment
for (int nSide = 0; nSide < 6; nSide++) {
	DeleteTriggerTargets (nDelSeg, nSide); 
	short index = GetFlickeringLight (nDelSeg, nSide); 
	if (index != -1) {
		FlickerLightCount ()--; 
		// put last light in place of deleted light
		memcpy(FlickeringLights (index), FlickeringLights (FlickerLightCount ()), sizeof (CFlickeringLight)); 
		}
	}

	// delete any Walls () within segment (if defined)
DeleteWalls (nDelSeg); 

// delete any Walls () on child Segments () that connect to this segment
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	child = delSegP->GetChild (i); 
	if (child >= 0 && child < SegCount ()) {
		short	oppSegNum, oppSideNum;
		GetOppositeSide (oppSegNum, oppSideNum, nDelSeg, i);
		if (Segments (oppSegNum)->m_sides [oppSideNum].m_info.nWall != NO_WALL)
			DeleteWall (Segments (oppSegNum)->m_sides [oppSideNum].m_info.nWall); 
			}
		}

	// delete any Objects () within segment
for (i = (ushort)MineInfo ().objects.count - 1; i >= 0; i--) {
	if (Objects (i)->m_info.nSegment == nDelSeg) {
		DeleteObject(i); 
		}
	}
#if 0 // done by UndefineSegment ()
	// delete any robot centers with this
	for (i = (ushort)MineInfo ().botgen.count - 1; i >= 0; i--) {
		nSegment = BotGens (i)->m_info.nSegment; 
		if (nSegment == nDelSeg) {
			int nMatCens = --MineInfo ().botgen.count; 
			if (i < nMatCens)
			memcpy ((void *) BotGens (i), (void *) BotGens (nMatCens), sizeof (CRobotMaker)); 
			}
		}

	// delete any equipment centers with this
	for (i = (ushort) MineInfo ().equipgen.count - 1; i >= 0; i--) {
		nSegment = EquipGens (i)->m_info.nSegment; 
		if (nSegment == nDelSeg) {
			MineInfo ().equipgen.count--; 
			memcpy ((void *) EquipGens (i), (void *) EquipGens (i + 1), 
					  (MineInfo ().equipgen.count - i) * sizeof (CRobotMaker)); 
			}
		}
#endif
	for (i = 0; i < MineInfo ().botgen.count; i++)
		if (BotGens (i)->m_info.nSegment > nDelSeg)
			BotGens (i)->m_info.nSegment--;
	for (i = 0; i < MineInfo ().equipgen.count; i++)
		if (EquipGens (i)->m_info.nSegment > nDelSeg)
			EquipGens (i)->m_info.nSegment--;
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
	nSegment = (ushort) SecretCubeNum (); 
	if (nSegment >= SegCount () || nSegment== nDelSeg)
		SecretCubeNum () = 0; 

	// update segment flags
	delSegP->m_info.wallFlags &= ~MARKED_MASK; 

	// unlink any children with this segment number
	CTexture* texP = textureManager.Textures (m_fileType);
	for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++) {
		for (child = 0; child < MAX_SIDES_PER_SEGMENT; child++) {
			if (segP->GetChild (child) == nDelSeg) {

				// subtract by 1 if segment is above deleted segment
				Current ()->nSegment = nSegment; 
				if (nSegment > nDelSeg) 
					Current ()->nSegment--; 

				// remove child number and update child bitmask
				segP->SetChild (child, -1); 

				// define textures, (u, v) and light
				CSide *sideP = delSegP->m_sides + child;
				SetTextures (nSegment, child, sideP->m_info.nBaseTex, sideP->m_info.nOvlTex); 
				Segments (nSegment)->SetUV (child, 0, 0); 
				double scale = texP [sideP->m_info.nBaseTex].Scale (sideP->m_info.nBaseTex);
				for (i = 0; i < 4; i++) {
					//segP->m_sides [child].m_info.uvls [i].u = (short) ((double) defaultUVLs [i].u / scale); 
					//segP->m_sides [child].m_info.uvls [i].v = (short) ((double) defaultUVLs [i].v / scale); 
					segP->m_sides [child].m_info.uvls [i].l = delSegP->m_sides [child].m_info.uvls [i].l; 
				}
			}
		}
	}

	// move other Segments () to deleted segment location
	if (nDelSeg != SegCount ()-1) { // if this is not the last segment

		// mark each segment with it's real number
		real_segnum = 0; 
		for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++)
			if(nDelSeg != nSegment)
				segP->m_info.nIndex = real_segnum++; 

		// replace all children with real numbers
		for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++) {
			for (child = 0; child < MAX_SIDES_PER_SEGMENT; child++) {
				if (segP->m_info.childFlags & (1 << child)
					&& segP->GetChild (child) >= 0 && segP->GetChild (child) < SegCount ()) { // debug int
					childSegP = Segments (segP->GetChild (child)); 
					segP->SetChild (child, childSegP->m_info.nIndex); 
				}
			}
		}

		// replace all wall segP numbers with real numbers
		for (i = 0; i < MineInfo ().walls.count; i++) {
			nSegment = (short) Walls (i)->m_nSegment; 
			if (nSegment < SegCount ()) {
				segP = Segments (nSegment); 
				Walls (i)->m_nSegment = segP->m_info.nIndex; 
				} 
			else {
				Walls (i)->m_nSegment = 0; // int wall segment number
			}
		}

		// replace all trigger segP numbers with real numbers
		for (i = NumTriggers (), trigP = Triggers (0); i; i--, trigP++) {
			for (j = 0; j < trigP->m_count; j++) {
				if (SegCount () > (nSegment = trigP->Segment (j)))
					trigP->Segment (j) = Segments (nSegment)->m_info.nIndex; 
				else {
					DeleteTargetFromTrigger (trigP, j, 0);
					j--;
					}
				}
			}

		// replace all trigger segP numbers with real numbers
		for (i = NumObjTriggers (), trigP = ObjTriggers (0); i; i--, trigP++) {
			for (j = 0; j < trigP->m_count; j++) {
				if (SegCount () > (nSegment = trigP->Segment (j)))
					trigP->Segment (j) = Segments (nSegment)->m_info.nIndex; 
				else {
					DeleteTargetFromTrigger (trigP, j, 0);
					j--;
					}
				}
			}

		// replace all object segP numbers with real numbers
		for (i = 0; i < MineInfo ().objects.count; i++) {
			objP = Objects (i); 
			if (SegCount () > (nSegment = objP->m_info.nSegment))
				objP->m_info.nSegment = Segments (nSegment)->m_info.nIndex; 
			else
				objP->m_info.nSegment = 0; // int object segment number
			}

		// replace robot centers segP numbers with real numbers
		for (i = 0; i < MineInfo ().botgen.count; i++) {
			if (SegCount () > (nSegment = BotGens (i)->m_info.nSegment))
				BotGens (i)->m_info.nSegment = Segments (nSegment)->m_info.nIndex; 
			else
				BotGens (i)->m_info.nSegment = 0; // int robot center nSegment
			}

		// replace equipment centers segP numbers with real numbers
		for (i = 0; i < MineInfo ().equipgen.count; i++) {
			if (SegCount () > (nSegment = EquipGens (i)->m_info.nSegment))
				EquipGens (i)->m_info.nSegment = Segments (nSegment)->m_info.nIndex; 
			else
				EquipGens (i)->m_info.nSegment = 0; // int robot center nSegment
			}

		// replace control segP numbers with real numbers
		for (i = 0; i < MineInfo ().control.count; i++) {
			for (j = 0; j < ReactorTriggers (i)->m_count; j++) {
				if (SegCount () > (nSegment = ReactorTriggers (i)->Segment (j)))
					ReactorTriggers (i)->Segment (j) = Segments (nSegment)->m_info.nIndex; 
				else 
					ReactorTriggers (i)->Segment (j) = 0; // int control center segment number
			}
		}

		// replace flickering light segP numbers with real numbers
		for (i = 0; i < FlickerLightCount (); i++) {
			if (SegCount () > (nSegment = FlickeringLights (i)->m_nSegment))
				FlickeringLights (i)->m_nSegment = Segments (nSegment)->m_info.nIndex; 
			else 
				FlickeringLights (i)->m_nSegment = 0; // int object segment number
			}

		// replace secret cubenum with real number
		if (SegCount () > (nSegment = (ushort) SecretCubeNum ()))
			SecretCubeNum () = Segments (nSegment)->m_info.nIndex; 
		else
			SecretCubeNum () = 0; // int secret cube number

		// move remaining Segments () down by 1
  }
#if 1
		if (int segC = (--SegCount () - nDelSeg)) {
			memcpy (Segments (nDelSeg), Segments (nDelSeg + 1), segC * sizeof (CSegment));
			memcpy (LightColors (nDelSeg), LightColors (nDelSeg + 1), segC * 6 * sizeof (CColor));
			}
#else
		for (nSegment = nDelSeg; nSegment < (SegCount ()-1); nSegment++) {
			segP = Segments (nSegment); 
			childSegP = Segments (nSegment + 1); 
			memcpy(segP, childSegP, sizeof (CSegment)); 
			}
  SegCount ()-- ; 
#endif


  // delete all unused vertices
  DeleteUnusedVertices(); 

  // make sure current segment numbers are valid
  if (Current1 ().nSegment >= SegCount ()) Current1 ().nSegment--; 
  if (Current2 ().nSegment >= SegCount ()) Current2 ().nSegment--; 
  if (Current1 ().nSegment < 0) Current1 ().nSegment = 0; 
  if (Current2 ().nSegment < 0) Current2 ().nSegment = 0; 
//DLE.MineView ()->Refresh (false); 
//DLE.ToolView ()->Refresh (); 
undoManager.Unlock ();
}



// ----------------------------------------------------------------------------- 
//	AddSegment()
//
//  ACTION - Add new segment at the end. solidifyally joins to adjacent
//           Segments () if sides are identical.  Uses other current cube for
//           textures.
//
//  Returns - TRUE on success
//
//  Changes - Now auto aligns u, v numbers based on parent textures
//
//  NEW - If there is a flickering light on the current side of this segment, 
//        it is deleted.
//
//        If cube is special (fuel center, robot maker, etc..) then textures
//        are set to default texture.
// ----------------------------------------------------------------------------- 

void CSegmentManager::InitSegment (short nSegment)
{
Segments (nSegment)->Setup ();
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::Add (void)
{
	CSegment *newSegP, *curSegP; 
	short i, nNewSeg, nNewSide, ncurrent.Side = Current ()->nSide; 
	short newVerts [4]; 
	short nSegment, nSide; 

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return FALSE; 
	}

curSegP = Segments (Current ()->nSegment); 

if (SegCount () >= MAX_SEGMENTS) {
	ErrorMsg ("Cannot add a new cube because\nthe maximum number of cubes has been reached."); 
	return FALSE;
	}
if (SegCount () >= MAX_SEGMENTS) {
	ErrorMsg ("Cannot add a new cube because\nthe maximum number of vertices has been reached."); 
	return FALSE;
	}
if (curSegP->GetChild (ncurrent.Side) >= 0) {
	ErrorMsg ("Can not add a new cube to a side\nwhich already has a cube attached."); 
	return FALSE;
	}

undoManager.SetModified (TRUE); 
undoManager.Lock ();
// get new verts
newVerts [0] = VertCount (); 
newVerts [1] = newVerts [0] + 1; 
newVerts [2] = newVerts [0] + 2; 
newVerts [3] = newVerts [0] + 3; 

// get new segment
nNewSeg = SegCount (); 
newSegP = Segments (nNewSeg); 

// define vertices
DefineVertices (newVerts); 

// define vert numbers for common side
newSegP->m_info.verts [oppSideVertTable [ncurrent.Side][0]] = curSegP->m_info.verts [sideVertTable [ncurrent.Side][0]]; 
newSegP->m_info.verts [oppSideVertTable [ncurrent.Side][1]] = curSegP->m_info.verts [sideVertTable [ncurrent.Side][1]]; 
newSegP->m_info.verts [oppSideVertTable [ncurrent.Side][2]] = curSegP->m_info.verts [sideVertTable [ncurrent.Side][2]]; 
newSegP->m_info.verts [oppSideVertTable [ncurrent.Side][3]] = curSegP->m_info.verts [sideVertTable [ncurrent.Side][3]]; 

// define vert numbers for new side
newSegP->m_info.verts [sideVertTable [ncurrent.Side][0]] = newVerts [0]; 
newSegP->m_info.verts [sideVertTable [ncurrent.Side][1]] = newVerts [1]; 
newSegP->m_info.verts [sideVertTable [ncurrent.Side][2]] = newVerts [2]; 
newSegP->m_info.verts [sideVertTable [ncurrent.Side][3]] = newVerts [3]; 

InitSegment (nNewSeg);
// define children and special child
newSegP->m_info.childFlags = 1 << oppSideTable [ncurrent.Side]; /* only opposite side connects to current_segment */
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) /* no remaining children */
	newSegP->SetChild (i, (newSegP->m_info.childFlags & (1 << i)) ? Current ()->nSegment : -1);

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

// delete flickering light if it exists
short index = GetFlickeringLight (Current ()->nSegment, ncurrent.Side); 
if (index != -1) {
	FlickerLightCount ()--; 
	// put last light in place of deleted light
	memcpy( FlickeringLights (index), FlickeringLights (FlickerLightCount ()), sizeof (CFlickeringLight)); 
	}

// update current segment
curSegP->SetChild (ncurrent.Side, nNewSeg); 
curSegP->m_sides [ncurrent.Side].m_info.nBaseTex = 0; 
curSegP->m_sides [ncurrent.Side].m_info.nOvlTex = 0; 
memset (curSegP->m_sides [ncurrent.Side].m_info.uvls, 0, sizeof (curSegP->m_sides [ncurrent.Side].m_info.uvls));
 
// update number of Segments () and vertices and clear vertexStatus
SegCount ()++;
for (int i = 0; i < 4; i++)
	Vertices (VertCount ()++)->m_status = 0;

// link the new segment with any touching Segments ()
CVertex *vNewSeg = Vertices (newSegP->m_info.verts [0]);
CVertex *vSeg;
for (nSegment = 0; nSegment < SegCount (); nSegment++) {
	if (nSegment != nNewSeg) {
		// first check to see if Segments () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		vSeg = Vertices (Segments (nSegment)->m_info.verts [0]);
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
	AlignTextures (Current ()->nSegment, nNewSide, nNewSeg, TRUE, TRUE); 
// set current segment to new segment
Current ()->nSegment = nNewSeg; 
//		SetLinesToDraw(); 
//DLE.MineView ()->Refresh (false); 
//DLE.ToolView ()->Refresh (); 
undoManager.Unlock ();
return TRUE; 
}

// ----------------------------------------------------------------------------- 
// DefineVertices()
//
//  ACTION - Calculates vertices when adding a new segment.
//
// ----------------------------------------------------------------------------- 

#define CURRENT_POINT(a) ((Current ()->nPoint + (a))&0x03)

void CSegmentManager::DefineVertices (short newVerts [4])
{
	CSegment*		curSegP; 
	CDoubleVector	A [8], B [8], C [8], D [8], E [8], a, b, c, d, v; 
	double			length; 
	short				nVertex; 
	short				i, points [4]; 
	CDoubleVector	center, oppCenter, newCenter, orthog; 

curSegP = Segments (Current ()->nSegment); 
for (i = 0; i < 4; i++)
	points [i] = CURRENT_POINT(i);
	// METHOD 1: orthogonal with right angle on new side and standard cube side
// TODO:
//	int add_segment_mode = ORTHOGONAL; 
switch (m_nAddMode) {
	case (ORTHOGONAL):
		{
		center = CalcSideCenter (Current ()->nSegment, Current ()->nSide); 
		oppCenter = CalcSideCenter (Current ()->nSegment, oppSideTable [Current ()->nSide]); 
		orthog = CalcSideNormal (Current ()->nSegment, Current ()->nSide); 
		// set the length of the new cube to be one standard cube length
		// scale the vector
		orthog *= 20; 
		// figure out new center
		newCenter = center + orthog; 
		// new method: extend points 0 and 1 with orthog, then move point 0 toward point 1.
		// point 0
		a = orthog + *Vertices (curSegP->m_info.verts [sideVertTable [Current ()->nSide][CURRENT_POINT(0)]]); 
		// point 1
		b = orthog + *Vertices (curSegP->m_info.verts [sideVertTable [Current ()->nSide][CURRENT_POINT(1)]]); 
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
			//nVertex = curSegP->m_info.verts [sideVertTable [Current ()->nSide][i]]; 
			nVertex = newVerts [i];
			*Vertices (nVertex) = A [i]; 
			}
		}
	break; 

	// METHOD 2: orghogonal with right angle on new side
	case (EXTEND):
		{
		center = CalcSideCenter (Current ()->nSegment, Current ()->nSide); 
		oppCenter = CalcSideCenter (Current ()->nSegment, oppSideTable [Current ()->nSide]); 
		orthog = CalcSideNormal (Current ()->nSegment, Current ()->nSide); 
		// calculate the length of the new cube
		orthog *= Distance (center, oppCenter); 
		// set the new vertices
		for (i = 0; i < 4; i++) {
			CDoubleVector v = *Vertices (curSegP->m_info.verts [sideVertTable [Current ()->nSide][i]]);
			v += orthog;
			*Vertices (newVerts [i]) = v; 
			}
		}
	break; 

	// METHOD 3: mirror relative to plane of side
	case(MIRROR):
		{
		// copy side's four points into A
		short nSide = Current ()->nSide;
		for (i = 0; i < 4; i++) {
			A [i] = *Vertices (curSegP->m_info.verts [sideVertTable [nSide][i]]); 
			A [i + 4] = *Vertices (curSegP->m_info.verts [oppSideVertTable [nSide][i]]); 
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
		nVertex = curSegP->m_info.verts [sideVertTable [Current ()->nSide][0]]; 
		for (i = 4; i < 8; i++) 
			A [i] = B [i] + *Vertices (nVertex); 

		for (i = 0; i < 4; i++)
			*Vertices (newVerts [i]) = A [i + 4]; 
		}
	}
}

// ----------------------------------------------------------------------------- 
// LinkSegments()
//
//  Action - checks 2 Segments () and 2 sides to see if the vertices are identical
//           If they are, then the segment sides are linked and the vertices
//           are removed (nSide1 is the extra side).
//
//  Change - no longer links if segment already has a child
//           no longer links Segments () if vert numbers are not in the right order
//
// ----------------------------------------------------------------------------- 

bool CSegmentManager::Link (short nSegment1, short nSide1, short nSegment2, short nSide2, double margin)
{
	CSegment		* seg1, * seg2; 
	short			i, j; 
	CVertex		v1 [4], v2 [4]; 
	short			fail;
	tVertMatch	match [4]; 

	seg1 = Segments (nSegment1); 
	seg2 = Segments (nSegment2); 

// don't link to a segment which already has a child
if (seg1->GetChild (nSide1) !=-1 || seg2->GetChild (nSide2) != -1)
	return FALSE; 

// copy vertices for comparison later (makes code more readable)
for (i = 0; i < 4; i++) {
	v1 [i] = *Vertices (seg1->m_info.verts [sideVertTable [nSide1][i]]);
	v2 [i] = *Vertices (seg2->m_info.verts [sideVertTable [nSide2][i]]);
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
	CSegment*	seg1 = Segments (nSegment1); 
	CSegment*	seg2 = Segments (nSegment2); 
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
	VertStatus (newVertex) |= (VertStatus (oldVertex) & MARKED_MASK); 

	// update all Segments () that use this vertex
	if (oldVertex != newVertex) {
		CSegment *segP = Segments (0);
		for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++)
			for (nVertex = 0; nVertex < 8; nVertex++)
				if (segP->m_info.verts [nVertex] == oldVertex)
					segP->m_info.verts [nVertex] = newVertex; 
		// then delete the vertex
		DeleteVertex (oldVertex); 
		}
	}
}

// ------------------------------------------------------------------------- 
// calculate_segment_center()
// ------------------------------------------------------------------------- 

void CSegmentManager::CalcSegCenter (CVertex& pos, short nSegment) 
{
  short	*nVerts = Segments (nSegment)->m_info.verts; 
  
pos.Clear ();
for (int i = 0; i < 8; i++)
	pos += *Vertices (nVerts [i]);
pos /= 8.0;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::SideIsMarked (short nSegment, short nSide)
{
current.Get (nSegment, nSide);
CSegment *segP = Segments (nSegment);
for (int i = 0; i < 4; i++) {
	if (!(VertStatus (segP->m_info.verts [sideVertTable [nSide][i]]) & MARKED_MASK))
		return false;
	}
return true;
}

bool CSegmentManager::SegmentIsMarked (short nSegment)
{
CSegment *segP = Segments (nSegment);
for (int i = 0;  i < 8; i++)
	if (!(VertStatus (segP->m_info.verts [i]) & MARKED_MASK))
		return false;
return true;
}

// ----------------------------------------------------------------------------- 
//			 mark_segment()
//
//  ACTION - Toggle marked bit of segment and mark/unmark vertices.
//
// ----------------------------------------------------------------------------- 

void CSegmentManager::Mark (short nSegment)
{
  CSegment *segP = Segments (nSegment); 

segP->m_info.wallFlags ^= MARKED_MASK; /* flip marked bit */

// update vertices's marked status
// ..first clear all marked verts
short nVertex; 
for (nVertex = 0; nVertex < MAX_VERTICES; nVertex++)
	VertStatus (nVertex) &= ~MARKED_MASK; 
// ..then mark all verts for marked Segments ()
for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++)
	if (segP->m_info.wallFlags & MARKED_MASK)
		for (nVertex = 0; nVertex < 8; nVertex++)
			VertStatus (segP->m_info.verts [nVertex]) |= MARKED_MASK; 
}

// ----------------------------------------------------------------------------- 
// update_marked_cubes()
// ----------------------------------------------------------------------------- 

void CSegmentManager::UpdateMarked (void)
{
	CSegment *segP = Segments (0); 
	int i; 
// mark all cubes which have all 8 verts marked
for (int i = 0; i < SegCount (); i++, segP++)
	if ((VertStatus (segP->m_info.verts [0]) & MARKED_MASK) &&
		 (VertStatus (segP->m_info.verts [1]) & MARKED_MASK) &&
		 (VertStatus (segP->m_info.verts [2]) & MARKED_MASK) &&
		 (VertStatus (segP->m_info.verts [3]) & MARKED_MASK) &&
		 (VertStatus (segP->m_info.verts [4]) & MARKED_MASK) &&
		 (VertStatus (segP->m_info.verts [5]) & MARKED_MASK) &&
		 (VertStatus (segP->m_info.verts [6]) & MARKED_MASK) &&
		 (VertStatus (segP->m_info.verts [7]) & MARKED_MASK))
		segP->m_info.wallFlags |= MARKED_MASK; 
	else
		segP->m_info.wallFlags &= ~MARKED_MASK; 
}

//========================================================================== 
// MENU - Mark all cubes
//========================================================================== 

void CSegmentManager::MarkAll (void) 
{
	int i; 

for (i = 0; i < SegCount (); i++) 
	Segments (i)->m_info.wallFlags |= MARKED_MASK; 
for (i = 0; i < VertCount (); i++) 
	VertStatus (i) |= MARKED_MASK; 
//DLE.MineView ()->Refresh (); 
}

//========================================================================== 
// MENU - Unmark all cubes
//========================================================================== 

void CSegmentManager::UnmarkAll (void) 
{
	short i; 

CSegment *segP = Segments (0);
for (i = 0; i < MAX_SEGMENTS; i++, segP++)
	segP->m_info.wallFlags &= ~MARKED_MASK; 
byte& stat = VertStatus ();
for (i = 0; i < MAX_VERTICES; i++, stat++)
	stat &= ~MARKED_MASK; 
//	DLE.MineView ()->Refresh (); 
}

// ----------------------------------------------------------------------------- 
// ResetSide()
//
// Action - sets side to have no child and a default texture
// ----------------------------------------------------------------------------- 

void CSegmentManager::ResetSide (short nSegment, short nSide)
{
if (nSegment < 0 || nSegment >= SegCount ()) 
	return; 
undoManager.SetModified (TRUE); 
undoManager.Lock ();
CSegment *segP = Segments (nSegment); 
segP->SetChild (nSide, -1); 
segP->m_info.childFlags &= ~(1 << nSide); 
CSide *sideP = segP->m_sides + nSide;
sideP->m_info.nBaseTex = 0; 
sideP->m_info.nOvlTex = 0; 
CUVL *uvls = sideP->m_info.uvls;
double scale = textureManager.Textures (m_fileType, sideP->m_info.nBaseTex)->Scale (sideP->m_info.nBaseTex);
int i;
for (i = 0; i < 4; i++, uvls++) {
	uvls->u = (short) (defaultUVLs [i].u / scale); 
	uvls->v = (short) (defaultUVLs [i].v / scale); 
	uvls->l = (ushort) DEFAULT_LIGHTING; 
	}
undoManager.Unlock ();
}

// ----------------------------------------------------------------------------- 
// unlink_child()
//
// Action - unlinks current cube's children which don't share all four points
//
// Note: 2nd parameter "nSide" is ignored
// ----------------------------------------------------------------------------- 

void CSegmentManager::UnlinkChild (short nParentSeg, short nSide) 
{
  CSegment *parentSegP = Segments (nParentSeg); 

// loop on each side of the parent
//	int nSide;
//  for (nSide = 0; nSide < 6; nSide++) {
int nChildSeg = parentSegP->GetChild (nSide); 
// does this side have a child?
if (nChildSeg < 0 || nChildSeg >= SegCount ())
	return;
CSegment *child_seg = Segments (nChildSeg); 
// yes, see if child has a side which points to the parent
int nChildSide;
for (nChildSide = 0; nChildSide < 6; nChildSide++)
	if (child_seg->GetChild (nChildSide) == nParentSeg) break; 
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
		undoManager.SetModified (TRUE); 
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
	int	i;

for (i = 0; i < 4; i++)
	if (sideVertTable [nSide][i] == nPoint)
		return true;
return false;
}

// ----------------------------------------------------------------------------- 

bool CSegmentManager::IsLineOfSide (CSegment *segP, int nSide, int nLine)
{
	int	i;

for (i = 0; i < 2; i++)
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
short vert, nSegment, nVertex, nOppSeg, nOppSide; 
bool found; 

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return; 
	}
if (VertCount () > (MAX_VERTICES - 1)) {
	ErrorMsg ("Cannot unjoin these points because the\n"
				"maximum number of points is reached."); 
	return; 
	}

segP = Segments (Current ()->nSegment); 
vert = segP->m_info.verts [sideVertTable [Current ()->nSide][Current ()->nPoint]]; 

// check to see if current point is shared by any other cubes
found = FALSE; 
segP = Segments (0);
for (nSegment = 0; (nSegment < SegCount ()) && !found; nSegment++, segP++)
	if (nSegment != Current ()->nSegment)
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

undoManager.SetModified (TRUE); 
undoManager.Lock ();
// create a new point (copy of other vertex)
memcpy (Vertices (VertCount ()), Vertices (vert), sizeof (*Vertices (0)));
/*
Vertices (VertCount ()).x = Vertices (vert).x; 
Vertices (VertCount ()).y = Vertices (vert).y; 
Vertices (VertCount ()).z = Vertices (vert).z; 
*/
// replace existing point with new point
segP = Segments (Current ()->nSegment); 
segP->m_info.verts [sideVertTable [Current ()->nSide][Current ()->nPoint]] = VertCount (); 
segP->m_info.wallFlags &= ~MARKED_MASK; 

// update total number of vertices
VertStatus (VertCount ()++) = 0; 

int nSide;
for (nSide = 0; nSide < 6; nSide++)
	if (IsPointOfSide (segP, nSide, segP->m_info.verts [sideVertTable [Current ()->nSide][Current ()->nPoint]]) &&
		 GetOppositeSide (nOppSeg, nOppSide, Current ()->nSegment, nSide)) {
		UnlinkChild (segP->GetChild (nSide), oppSideTable [nSide]);
		UnlinkChild (Current ()->nSegment, nSide); 
		}

	UnlinkChild(Current ()->nSegment, nSide); 

SetLinesToDraw(); 
undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
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
  short vert [2], nSegment, nVertex, nLine, nOppSeg, nOppSide, i; 
  bool found [2]; 

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return; 
	}
if (VertCount () > (MAX_VERTICES - 2)) {
	if (!bExpertMode)
		ErrorMsg ("Cannot unjoin these lines because\nthere are not enought points left."); 
	return; 
	}

segP = Segments (Current ()->nSegment); 
for (i = 0; i < 2; i++) {
	nLine = sideLineTable [Current ()->nSide][Current ()->nLine]; 
	vert [i] = Segments (Current ()->nSegment)->m_info.verts [lineVertTable [nLine][i]]; 
	// check to see if current points are shared by any other cubes
	found [i] = FALSE; 
	segP = Segments (0);
	for (nSegment = 0; (nSegment < SegCount ()) && !found [i]; nSegment++, segP++) {
		if (nSegment != Current ()->nSegment) {
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
undoManager.SetModified (TRUE); 
undoManager.Lock ();
segP = Segments (Current ()->nSegment); 
// create a new points (copy of other vertices)
for (i = 0; i < 2; i++)
	if (found [i]) {
		memcpy (Vertices (VertCount ()), Vertices (vert [i]), sizeof (*Vertices (0)));
		/*
		vertices [VertCount ()].x = vertices [vert [i]].x; 
		vertices [VertCount ()].y = vertices [vert [i]].y; 
		vertices [VertCount ()].z = vertices [vert [i]].z; 
		*/
		// replace existing points with new points
		nLine = sideLineTable [Current ()->nSide][Current ()->nLine]; 
		segP->m_info.verts [lineVertTable [nLine][i]] = VertCount (); 
		segP->m_info.wallFlags &= ~MARKED_MASK; 
		// update total number of vertices
		VertStatus (VertCount ()++) = 0; 
		}
int nSide;
for (nSide = 0; nSide < 6; nSide++) {
	if (IsLineOfSide (segP, nSide, nLine) && 
		 GetOppositeSide (nOppSeg, nOppSide, Current ()->nSegment, nSide)) {
		UnlinkChild (nOppSeg, nOppSide);
		UnlinkChild (Current ()->nSegment, nSide); 
		}
	}
SetLinesToDraw(); 
undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
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

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return; 
	}

segP = current.Segment (); 
if (nSide < 0)
	nSide = Current ()->nSide;
int nChildSeg = segP->GetChild (nSide); 
if (nChildSeg == -1) {
	ErrorMsg ("The current side is not connected to another cube"); 
	return; 
	}

for (i = 0; i < 4; i++)
	vert [i] = segP->m_info.verts [sideVertTable [nSide][i]]; 
	// check to see if current points are shared by any other cubes
for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++)
	if (nSegment != Current ()->nSegment)
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

if (!solidify && (VertCount () > (MAX_VERTICES - nFound))) {
	ErrorMsg ("Cannot unjoin this side because\nthere are not enough vertices left."); 
	return; 
	}

if (QueryMsg ("Are you sure you want to unjoin this side?") != IDYES)
	return; 

undoManager.SetModified (TRUE); 
undoManager.Lock ();
segP = Segments (Current ()->nSegment); 
if (nFound < 4)
	solidify = 0;
if (!solidify) {
	// create new points (copy of other vertices)
	for (i = 0; i < 4; i++) {
		if (found [i]) {
			memcpy (Vertices (VertCount ()), Vertices (vert [i]), sizeof (*Vertices (0)));
			/*
			vertices [VertCount ()].x = vertices [vert [i]].x; 
			vertices [VertCount ()].y = vertices [vert [i]].y; 
			vertices [VertCount ()].z = vertices [vert [i]].z; 
			*/
			// replace existing points with new points
			segP->m_info.verts [sideVertTable [nSide][i]] = VertCount (); 
			segP->m_info.wallFlags &= ~MARKED_MASK; 

			// update total number of vertices
			VertStatus (VertCount ()++) = 0; 
			}
		}
	int nSide;
	for (nSide = 0; nSide < 6; nSide++)
		if (nSide != oppSideTable [nSide])
			UnlinkChild (Current ()->nSegment, nSide); 
	SetLinesToDraw(); 
	INFOMSG (" Four new points were made for the current side."); 
	}
else {
	// does this side have a child?
	CSegment *childSegP = Segments (nChildSeg); 
	// yes, see if child has a side which points to the parent
	int nChildSide;
	for (nChildSide = 0; nChildSide < 6; nChildSide++)
		if (childSegP->GetChild (nChildSide) == Current ()->nSegment) 
			break; 
	// if we found the matching side
	if (nChildSide < 6)
		ResetSide (nChildSeg, nChildSide); 
	ResetSide (Current ()->nSegment, Current ()->nSide); 
	SetLinesToDraw(); 
	}
undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
}

// ----------------------------------------------------------------------------- 
// Mine - Joinpoints
// ----------------------------------------------------------------------------- 

void CSegmentManager::JoinPoints (void) 
{
  CSegment *seg1, *seg2; 
 double distance; //v1x, v1y, v1z, v2x, v2y, v2z; 
  int vert1, vert2; 
  CSelection *cur1, *cur2; 

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return; 
	}
if (Current1 ().nSegment== Current2 ().nSegment) {
	ErrorMsg ("You cannot joint two points on the same cube.\n\n"
				"Hint: The two golden circles represent the current point, \n"
				"and the 'other' cube's current point.  Press 'P' to change the\n"
				"Current () point or press the space bar to switch to the other cube."); 
	return;
	}

if (Current () == &Current1 ()) {
	seg1 = Segments (Current1 ().nSegment); 
	seg2 = Segments (Current2 ().nSegment); 
	cur1 = &Current1 (); 
	cur2 = &Current2 (); 
	}
else {
	seg1 = Segments (Current2 ().nSegment); 
	seg2 = Segments (Current1 ().nSegment); 
	cur1 = &Current2 (); 
	cur2 = &Current1 (); 
	}
vert1 = seg1->m_info.verts [sideVertTable [cur1->nSide][cur1->nPoint]]; 
vert2 = seg2->m_info.verts [sideVertTable [cur2->nSide][cur2->nPoint]]; 
// make sure verts are different
if (vert1== vert2) {
	ErrorMsg ("These points are already joined."); 
	return; 
	}
// make sure there are distances are close enough
distance = Distance (*Vertices (vert1), *Vertices (vert2)); 
if (distance > JOIN_DISTANCE) {
	ErrorMsg ("Points are too far apart to join"); 
	return; 
	}
if (QueryMsg("Are you sure you want to join the current point\n"
				 "with the 'other' cube's current point?") != IDYES)
	return; 
undoManager.SetModified (TRUE); 
undoManager.Lock ();
// define vert numbers
seg1->m_info.verts [sideVertTable [cur1->nSide][cur1->nPoint]] = vert2; 
// delete any unused vertices
//  delete_unused_vertices(); 
FixChildren(); 
SetLinesToDraw(); 
//DLE.MineView ()->Refresh ();
undoManager.Unlock ();
}

// ----------------------------------------------------------------------------- 
// Mine - Joinlines
// ----------------------------------------------------------------------------- 

void CSegmentManager::JoinLines (void) 
{
  CSegment *seg1, *seg2; 
  double v1x [2], v1y [2], v1z [2], v2x [2], v2y [2], v2z [2]; 
  double distance, min_radius; 
  int v1, v2, vert1 [2], vert2 [2]; 
  short match [2]; 
  short i, j, nLine; 
  bool fail; 
  CSelection *cur1, *cur2; 

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return; 
	}

if (Current1 ().nSegment == Current2 ().nSegment) {
	ErrorMsg ("You cannot joint two lines on the same cube.\n\n"
				"Hint: The two green lines represent the current line, \n"
				"and the 'other' cube's current line.  Press 'L' to change\n"
				"the current line or press the space bar to switch to the other cube."); 
	return;
	}

if (Current ()== &Current1 ()) {
	seg1 = Segments (Current1 ().nSegment); 
	seg2 = Segments (Current2 ().nSegment); 
	cur1 = &Current1 (); 
	cur2 = &Current2 (); 
	} 
else {
	seg1 = Segments (Current2 ().nSegment); 
	seg2 = Segments (Current1 ().nSegment); 
	cur1 = &Current2 (); 
	cur2 = &Current1 (); 
	}

for (i = 0; i < 2; i++) {
	nLine = sideLineTable [cur1->nSide][cur1->nLine]; 
	v1 = vert1 [i] = seg1->m_info.verts [lineVertTable [nLine][i]]; 
	nLine = sideLineTable [cur2->nSide][cur2->nLine]; 
	v2 = vert2 [i] = seg2->m_info.verts [lineVertTable [nLine][i]]; 
	v1x [i] = Vertices (v1)->v.x; 
	v1y [i] = Vertices (v1)->v.y; 
	v1z [i] = Vertices (v1)->v.z; 
	v2x [i] = Vertices (v2)->v.x; 
	v2y [i] = Vertices (v2)->v.y; 
	v2z [i] = Vertices (v2)->v.z; 
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
	min_radius = JOIN_DISTANCE; 
	for (j = 0; j < 2; j++) {
		distance = sqrt((v1x [i] - v2x [j]) * (v1x [i] - v2x [j])
					+ (v1y [i] - v2y [j]) * (v1y [i] - v2y [j])
					+ (v1z [i] - v2z [j]) * (v1z [i] - v2z [j])); 
		if (distance < min_radius) {
			min_radius = distance; 
			match [i] = j;  // remember which vertex it matched
			}
		}
	}

// make sure there are distances are close enough
if (min_radius == JOIN_DISTANCE) {
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
undoManager.SetModified (TRUE); 
undoManager.Lock ();
// define vert numbers
for (i = 0; i < 2; i++) {
	nLine = sideLineTable [cur1->nSide][cur1->nLine]; 
	seg1->m_info.verts [lineVertTable [nLine][i]] = vert2 [match [i]]; 
	}
FixChildren(); 
SetLinesToDraw(); 
//DLE.MineView ()->Refresh ();
undoManager.Unlock ();
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

for (nSegment = SegCount (), segP = Segments (0); nSegment; nSegment--, segP++) {
	segP->m_info.mapBitmask |= 0xFFF; 
	// if segment nSide has a child, clear bit for drawing line
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		if (segP->GetChild (nSide) > -1) { // -1 = no child,  - 2 = outside of world
			segP->m_info.mapBitmask &= ~(1 << (sideLineTable [nSide][0])); 
			segP->m_info.mapBitmask &= ~(1 << (sideLineTable [nSide][1])); 
			segP->m_info.mapBitmask &= ~(1 << (sideLineTable [nSide][2])); 
			segP->m_info.mapBitmask &= ~(1 << (sideLineTable [nSide][3])); 
			}
		}
	}
}

// ----------------------------------------------------------------------------- 
// FixChildren()
//
// Action - Updates linkage between current segment and all other Segments ()
// ----------------------------------------------------------------------------- 

void CSegmentManager::FixChildren (void)
{
short nNewSide, nSide, nSegment, nNewSeg; 

nNewSeg = Current ()->nSegment; 
nNewSide = Current ()->nSide; 
CSegment *segP = Segments (0),
			*newSegP = Segments (nNewSeg);
CVertex	*vSeg, 
			*vNewSeg = Vertices (newSegP->m_info.verts [0]);
for (nSegment = 0; nSegment < SegCount (); nSegment++, segP) {
	if (nSegment != nNewSeg) {
		// first check to see if Segments () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		vSeg = Vertices (segP->m_info.verts [0]);
		if (fabs ((double) (vNewSeg->v.x - vSeg->v.x)) < 10.0 &&
		    fabs ((double) (vNewSeg->v.y - vSeg->v.y)) < 10.0 &&
		    fabs ((double) (vNewSeg->v.z - vSeg->v.z)) < 10.0) {
			for (nSide = 0; nSide < 6; nSide++) {
				if (!Link (nNewSeg, nNewSide, nSegment, nSide, 3)) {
					// if these Segments () were linked, then unlink them
					if ((newSegP->GetChild (nNewSide) == nSegment) && (segP->GetChild (nSide) == nNewSeg)) {
						newSegP->SetChild (nNewSide, -1); 
						segP->SetChild (nSide, -1); 
						}
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------- 
//		       Joinsegments()
//
//  ACTION - Joins sides of current Segments ().  Finds closest corners.
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
	double radius, min_radius, max_radius, totalRad, minTotalRad; 
	tVertMatch match [4]; 
	bool fail; 
	CSelection *cur1, *cur2, my_cube; 

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return; 
	}

// figure out "other' cube
if (solidify) {
	if (Segments (Current ()->nSegment)->GetChild (Current ()->nSide) != -1) {
		if (!bExpertMode)
			ErrorMsg ("The current side is already joined to another cube"); 
		return; 
		}
	cur1 = Current (); 
	cur2 = &my_cube; 
	my_cube.nSegment = -1;
	// find first cube (other than this cube) which shares all 4 points
	// of the current side (points must be < 5.0 away)
	seg1 = Segments (cur1->nSegment); 
	for (i = 0; i < 4; i++) {
		memcpy (&v1 [i], Vertices (seg1->m_info.verts [sideVertTable [cur1->nSide][i]]), sizeof (CVertex));
		}
	minTotalRad = 1e300;
	for (nSegment = 0, seg2 = Segments (0); nSegment < SegCount (); nSegment++, seg2++) {
		if (nSegment== cur1->nSegment)
			continue; 
		for (nSide = 0; nSide < 6; nSide++) {
			fail = FALSE; 
			for (i = 0; i < 4; i++) {
				memcpy (&v2 [i], Vertices (seg2->m_info.verts[sideVertTable[nSide][i]]), sizeof (CVertex));
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
				my_cube.nSegment = nSegment; 
				my_cube.nSide = nSide; 
				my_cube.nPoint = 0; // should not be used
			// force break from loops
				if (minTotalRad == 0) {
					nSide = 6; 
					nSegment = SegCount (); 
					}
				}
			}
		}
	if (my_cube.nSegment < 0) {
		if (!bExpertMode)
			ErrorMsg ("Could not find another cube whose side is within\n"
						"10.0 units from the current side"); 
		return; 
		}
	}
else
	if (Current ()== &Current1 ()) {
		cur1 = &Current1 (); 
		cur2 = &Current2 (); 
		}
	else {
		cur1 = &Current2 (); 
		cur2 = &Current1 (); 
		}

if (cur1->nSegment == cur2->nSegment) {
	if (!bExpertMode)
		ErrorMsg ("You cannot joint two sides on the same cube.\n\n"
					"Hint: The two red squares represent the current side, \n"
					"and the 'other' cube's current side.  Press 'S' to change\n"
					"the current side or press the space bar to switch to the other cube."); 
	return; 
	}

seg1 = Segments (cur1->nSegment); 
seg2 = Segments (cur2->nSegment); 

// figure out matching corners to join to.
// get coordinates for calulaction and set match = none
for (i = 0; i < 4; i++) {
	memcpy (&v1 [i], Vertices (seg1->m_info.verts [sideVertTable [cur1->nSide][i]]), sizeof (CVertex)); 
	memcpy (&v2 [i], Vertices (seg2->m_info.verts [sideVertTable [cur2->nSide][i]]), sizeof (CVertex)); 
	match [i].i = -1; 
	}

// find closest for each point for each corner
for (i = 0; i < 4; i++) {
	min_radius = JOIN_DISTANCE; 
	for (j = 0; j < 4; j++) {
		radius = Distance (v1 [i], v2 [j]);
		if (radius < min_radius) {
			min_radius = radius; 
			match [i].i = j;  // remember which vertex it matched
			}
		}
	}

fail = FALSE; 
for (i = 0; i < 4; i++)
	if (match [i].i == -1) {
		fail = TRUE; 
		break; 
	}

// make sure there are matches for each and they are unique
if (match [0].i == match [1].i) fail = TRUE; 
else if (match [0].i == match [2].i) fail = TRUE; 
else if (match [0].i == match [3].i) fail = TRUE; 
else if (match [1].i == match [2].i) fail = TRUE; 
else if (match [1].i == match [3].i) fail = TRUE; 
else if (match [2].i == match [3].i) fail = TRUE; 

if (fail) {
	int offset = (4 + cur1->nPoint - (3 - cur2->nPoint))%4; 
	match [0].i = (offset + 3) % 4; 
	match [1].i = (offset + 2) % 4; 
	match [2].i = (offset + 1) % 4; 
	match [3].i = (offset + 0) % 4; 
	}

// determine min and max distances
min_radius = JOIN_DISTANCE; 
max_radius = 0; 
for (i = 0; i < 4; i++) {
	j = match [i].i; 
	radius = Distance (v1 [i], v2 [j]);
	min_radius = min(min_radius, radius); 
	max_radius = max(max_radius, radius); 
	}

// make sure there are distances are close enough
if (max_radius >= JOIN_DISTANCE) {
	if (!bExpertMode)
		ErrorMsg ("Sides are too far apart to join.\n\n"
					"Hint: Cubes should not exceed 200 in any dimension\n"
					"or they will distort when viewed from close up."); 
	return; 
	}

// if Segments () are too close to put a new segment between them, 
// then solidifyally link them together without asking
if (min_radius <= 5) {
	undoManager.SetModified (TRUE); 
	undoManager.Lock ();
	LinkSides (cur1->nSegment, cur1->nSide, cur2->nSegment, cur2->nSide, match); 
	SetLinesToDraw(); 
	undoManager.Unlock ();
	//DLE.MineView ()->Refresh ();
	return; 
	}

if (QueryMsg("Are you sure you want to create a new cube which\n"
				 "connects the current side with the 'other' side?\n\n"
				 "Hint: Make sure you have the current point of each cube\n"
				 "on the corners you to connected.\n"
				 "(the 'P' key selects the current point)") != IDYES)
	return; 

nNewSeg = SegCount (); 
if (!(SegCount () < MAX_SEGMENTS)) {
	if (!bExpertMode)
		ErrorMsg ("The maximum number of Segments () has been reached.\n"
					"Cannot add any more Segments ()."); 
	return; 
	}
segP = Segments (nNewSeg); 

undoManager.SetModified (TRUE); 
undoManager.Lock ();
// define children and special child
// first clear all sides
segP->m_info.childFlags = 0; 
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  /* no remaining children */
	segP->SetChild (i, -1); 

// now define two sides:
// near side has opposite side number cube 1
segP->SetChild (oppSideTable [cur1->nSide], cur1->nSegment); 
// far side has same side number as cube 1
segP->SetChild (cur1->nSide, cur2->nSegment); 
segP->m_info.owner = -1;
segP->m_info.group = -1;
segP->m_info.function = 0; 
segP->m_info.nMatCen =-1; 
segP->m_info.value =-1; 

// define vert numbers
for (i = 0; i < 4; i++) {
	segP->m_info.verts [oppSideVertTable [cur1->nSide][i]] = seg1->m_info.verts [sideVertTable [cur1->nSide][i]]; 
	segP->m_info.verts [sideVertTable [cur1->nSide][i]] = seg2->m_info.verts [sideVertTable [cur2->nSide][match [i].i]]; 
	}

// define Walls ()
segP->m_info.wallFlags = 0; // unmarked
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++)
	segP->m_sides [nSide].m_info.nWall = NO_WALL; 

// define sides
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	if (segP->GetChild (nSide) == -1) {
		SetTextures (nNewSeg, nSide, seg1->m_sides [cur1->nSide].m_info.nBaseTex, seg1->m_sides [cur1->nSide].m_info.nOvlTex); 
		Segments (nNewSeg)->SetUV (nSide, 0, 0); 
		}
	else {
		SetTextures (nNewSeg, nSide, 0, 0); 
		for (i = 0; i < 4; i++) {
			segP->m_sides [nSide].m_info.uvls [i].u = 0; 
			segP->m_sides [nSide].m_info.uvls [i].v = 0; 
			segP->m_sides [nSide].m_info.uvls [i].l = 0; 
			}
		}
	}

// define static light
segP->m_info.staticLight = seg1->m_info.staticLight; 

// update cur segment
seg1->SetChild (cur1->nSide, nNewSeg); 
SetTextures (cur1->nSegment, cur1->nSide, 0, 0); 
for (i = 0; i < 4; i++) {
	seg1->m_sides [cur1->nSide].m_info.uvls [i].u = 0; 
	seg1->m_sides [cur1->nSide].m_info.uvls [i].v = 0; 
	seg1->m_sides [cur1->nSide].m_info.uvls [i].l = 0; 
	}
seg2->SetChild (cur2->nSide, nNewSeg); 
SetTextures (cur2->nSegment, cur2->nSide, 0, 0); 
for (i = 0; i < 4; i++) {
	seg2->m_sides [cur2->nSide].m_info.uvls [i].u = 0; 
	seg2->m_sides [cur2->nSide].m_info.uvls [i].v = 0; 
	seg2->m_sides [cur2->nSide].m_info.uvls [i].l = 0; 
	}

// update number of Segments () and vertices
SegCount ()++; 
undoManager.Unlock ();
SetLinesToDraw(); 
//DLE.MineView ()->Refresh ();
}

// ------------------------------------------------------------------------ 

bool CSegmentManager::HaveMarkedSides ()
{
int	nSegment, nSide; 

for (nSegment = 0; nSegment < SegCount (); nSegment++)
	for (nSide = 0; nSide < 6; nSide++)
		if (SideIsMarked (nSegment, nSide))
			return true;
return false; 
}

// ------------------------------------------------------------------------ 

short CSegmentManager::MarkedCount (bool bCheck)
{
	int	nSegment, nCount; 
	CSegment *segP = Segments (0);
for (nSegment = SegCount (), nCount = 0; nSegment; nSegment--, segP++)
	if (segP->m_info.wallFlags & MARKED_MASK)
		if (bCheck)
			return 1; 
		else
			++nCount; 
return nCount; 
}

// ------------------------------------------------------------------------ 


int CSegmentManager::IsWall (short nSegment, short nSide)
{
current.Get (nSegment, nSide); 
return (Segments (nSegment)->GetChild (nSide)== -1) ||
		 (Segments (nSegment)->m_sides [nSide].m_info.nWall < MineInfo ().walls.count); 
}


                        /* -------------------------- */

int CSegmentManager::AlignTextures (short nStartSeg, short nStartSide, short nOnlyChildSeg, BOOL bAlign1st, BOOL bAlign2nd, char bAlignedSides)
{
	CSegment*	segP = Segments (nStartSeg); 
	CSegment*	childSegP; 
	CSide*		sideP = segP->m_sides + nStartSide; 
	CSide*		childSideP; 

	int			return_code = -1; 
	short			i; 
	short			nSide, nChildSeg, nLine; 
	short			point0, point1, vert0, vert1; 
	short			nChildSide, nChildLine; 
	short			nChildPoint0, nChildPoint1, nChildVert0, nChildVert1; 
	short			u0, v0; 
	double		sangle, cangle, angle, length; 

	static int sideChildTable [6][4] = {
		{4, 3, 5, 1}, //{5, 1, 4, 3}, 
		{2, 4, 0, 5}, //{5, 0, 4, 2}, 
		{5, 3, 4, 1}, //{5, 3, 4, 1}, 
		{0, 4, 2, 5}, //{5, 0, 4, 2}, 
		{2, 3, 0, 1}, //{2, 3, 0, 1}, 
		{0, 3, 2, 1} //{2, 3, 0, 1}
		}; 

undoManager.SetModified (TRUE);
undoManager.Lock ();
for (nLine = 0; nLine < 4; nLine++) {
	// find vert numbers for the line's two end points
	point0 = lineVertTable [sideLineTable [nStartSide][nLine]][0]; 
	point1 = lineVertTable [sideLineTable [nStartSide][nLine]][1]; 
	vert0  = segP->m_info.verts [point0]; 
	vert1  = segP->m_info.verts [point1]; 
	// check child for this line
	if (nStartSeg == nOnlyChildSeg) {
		nSide = nStartSide;
		nChildSeg = nStartSeg;
		}
	else {
		nSide = sideChildTable [nStartSide][nLine]; 
		nChildSeg = segP->GetChild (nSide); 
		}
	if ((nChildSeg < 0) || ((nOnlyChildSeg != -1) && (nChildSeg != nOnlyChildSeg)))
		continue;
	childSegP = Segments (nChildSeg); 
	// figure out which side of child shares two points w/ nStartSide
	for (nChildSide = 0; nChildSide < 6; nChildSide++) {
		if ((nStartSeg == nOnlyChildSeg) && (nChildSide == nStartSide))
			continue;
		if (bAlignedSides & (1 << nChildSide))
			continue;
		// ignore children of different textures (or no texture)
		if (!IsWall (nChildSeg, nChildSide))
			continue;
		if (childSegP->m_sides [nChildSide].m_info.nBaseTex != sideP->m_info.nBaseTex)
			continue;
		for (nChildLine = 0; nChildLine < 4; nChildLine++) {
			// find vert numbers for the line's two end points
			nChildPoint0 = lineVertTable [sideLineTable [nChildSide][nChildLine]][0]; 
			nChildPoint1 = lineVertTable [sideLineTable [nChildSide][nChildLine]][1]; 
			nChildVert0  = childSegP->m_info.verts [nChildPoint0]; 
			nChildVert1  = childSegP->m_info.verts [nChildPoint1]; 
			// if points of child's line== corresponding points of parent
			if (!((nChildVert0 == vert1 && nChildVert1 == vert0) ||
					(nChildVert0 == vert0 && nChildVert1 == vert1)))
				continue;
			// now we know the child's side & line which touches the parent
			// child:  nChildSeg, nChildSide, nChildLine, nChildPoint0, nChildPoint1
			// parent: nStartSeg, nStartSide, nLine, point0, point1
			childSideP = childSegP->m_sides + nChildSide; 
			if (bAlign1st) {
				// now translate all the childs (u, v) coords so child_point1 is at zero
				u0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].u; 
				v0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].v; 
				for (i = 0; i < 4; i++) {
					childSideP->m_info.uvls [i].u -= u0; 
					childSideP->m_info.uvls [i].v -= v0; 
					}
				// find difference between parent point0 and child point1
				u0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].u - sideP->m_info.uvls [nLine].u; 
				v0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].v - sideP->m_info.uvls [nLine].v; 
				// find the angle formed by the two lines
				sangle = atan3(sideP->m_info.uvls [(nLine + 1) % 4].v - sideP->m_info.uvls [nLine].v, 
									sideP->m_info.uvls [(nLine + 1) % 4].u - sideP->m_info.uvls [nLine].u); 
				cangle = atan3(childSideP->m_info.uvls [nChildLine].v - childSideP->m_info.uvls [(nChildLine + 1) % 4].v, 
									childSideP->m_info.uvls [nChildLine].u - childSideP->m_info.uvls [(nChildLine + 1) % 4].u); 
				// now rotate childs (u, v) coords around child_point1 (cangle - sangle)
				for (i = 0; i < 4; i++) {
					angle = atan3(childSideP->m_info.uvls [i].v, childSideP->m_info.uvls [i].u); 
					length = sqrt((double) childSideP->m_info.uvls [i].u * (double) childSideP->m_info.uvls [i].u +
									  (double) childSideP->m_info.uvls [i].v * (double) childSideP->m_info.uvls [i].v); 
					angle -= (cangle - sangle); 
					childSideP->m_info.uvls [i].u = (short)(length * cos (angle)); 
					childSideP->m_info.uvls [i].v = (short)(length * sin (angle)); 
					}
				// now translate all the childs (u, v) coords to parent point0
				for (i = 0; i < 4; i++) {
					childSideP->m_info.uvls [i].u -= u0; 
					childSideP->m_info.uvls [i].v -= v0; 
					}
				// modulo points by 0x800 (== 64 pixels)
				u0 = childSideP->m_info.uvls [0].u / 0x800; 
				v0 = childSideP->m_info.uvls [0].v / 0x800; 
				for (i = 0; i < 4; i++) {
					childSideP->m_info.uvls [i].u -= u0*0x800; 
					childSideP->m_info.uvls [i].v -= v0*0x800; 
					}
				if (nOnlyChildSeg != -1)
					return_code = nChildSide; 
				}
			if (bAlign2nd && sideP->m_info.nOvlTex && childSideP->m_info.nOvlTex) {
				int r;
				switch (sideP->m_info.nOvlTex & 0xC000) {
					case 0:
						r = 0;
						break;
					case 0xC000:
						r = 1;
						break;
					case 0x8000:
						r = 2;
						break;
					case 0x4000:
						r = 3;
						break;
					}
				int h = (int) (Degrees (fabs (angle)) / 90 + 0.5); 
//				h +=(nChildLine + nLine + 2) % 4; //(nChildLine > nLine) ? nChildLine - nLine : nLine - nChildLine;
				h = (h + r) % 4;
				childSideP->m_info.nOvlTex &= ~0xC000;
				switch (h) {
					case 0:
						break;
					case 1:
						childSideP->m_info.nOvlTex |= 0xC000;
						break;
					case 2:
						childSideP->m_info.nOvlTex |= 0x8000;
						break;
					case 3:
						childSideP->m_info.nOvlTex |= 0x4000;
						break;
					}
				}
			break;
			}
		}
	}
undoManager.Unlock ();
return return_code; 
}

// ------------------------------------------------------------------------------ 
// get_opposing_side()
//
// Action - figures out childs nSegment and side for a given side
// Returns - TRUE on success
// ------------------------------------------------------------------------------ 

bool CSegmentManager::GetOppositeSide (short& nOppSeg, short& nOppSide, short nSegment, short nSide)
{
  short nChildSeg, nChildSide; 

nOppSeg = 0; 
nOppSide = 0; 
current.Get (nSegment, nSide); 
if (nSegment < 0 || nSegment >= SegCount ())
	return false; 
if (nSide < 0 || nSide >= 6)
	return false; 
nChildSeg = Segments (nSegment)->GetChild (nSide); 
if (nChildSeg < 0 || nChildSeg >= SegCount ())
	return false; 
for (nChildSide = 0; nChildSide < 6; nChildSide++) {
	if (Segments (nChildSeg)->GetChild (nChildSide) == nSegment) {
		nOppSeg = nChildSeg; 
		nOppSide = nChildSide; 
		return true; 
		}
	}
return false; 
}

// ------------------------------------------------------------------------------ 

CSide* CSegmentManager::GetOppositeSide (short nSegment, short nSide)
{
	short nOppSeg, nOppSide;

return GetOppositeSide (nOppSeg, nOppSide, nSegment, nSide) ? GetSide (nOppSeg, nOppSide) : null;
}

// ------------------------------------------------------------------------------ 

                        /* -------------------------- */

CSide* CSegmentManager::OppSide (void) 
{
short nOppSeg, nOppSide;
if (!GetOppositeSide (nOppSeg, nOppSide))
	return null;
return Segments (nOppSeg)->m_sides + nOppSide;
}

                        /* -------------------------- */

bool CSegmentManager::SetTextures (short nSegment, short nSide, short nBaseTex, short nOvlTex)
{
	bool bUndo, bChange = false;

bUndo = undoManager.SetModified (TRUE); 
undoManager.Lock (); 
current.Get (nSegment, nSide); 
CSide *sideP = Segments (nSegment)->m_sides + nSide; 
bChange = sideP->SetTextures (nBaseTex, nOvlTex);
if (!bChange) {
	undoManager.ResetModified (bUndo);
	return false;
	}
if ((IsLight (sideP->m_info.nBaseTex) == -1) && (IsLight (sideP->m_info.nOvlTex & 0x3fff) == -1))
	DeleteFlickeringLight (nSegment, nSide); 
if (!WallClipFromTexture (nSegment, nSide))
	CheckForDoor (nSegment, nSide); 
undoManager.Unlock (); 
sprintf_s (message, sizeof (message), "side has textures %d, %d", sideP->m_info.nBaseTex & 0x3fff, sideP->m_info.nOvlTex & 0x3fff); 
INFOMSG (message); 
return true;
}

                        /* -------------------------- */

void CSegmentManager::RenumberBotGens (void) 
{
	int			i, nMatCens, value, nSegment; 
	CSegment	*segP; 

// number "matcen"
nMatCens = 0; 
for (i = 0; i < MineInfo ().botgen.count; i++) {
	nSegment = BotGens (i)->m_info.nSegment; 
	if (nSegment >= 0) {
		segP = Segments (nSegment); 
		segP->m_info.value = i; 
		if (segP->m_info.function== SEGMENT_FUNC_ROBOTMAKER)
			segP->m_info.nMatCen = nMatCens++; 
		}
	}

// number "value"
value = 0; 
for (i = 0, segP = Segments (0); i < SegCount (); i++, segP++)
	if (segP->m_info.function== SEGMENT_FUNC_NONE)
		segP->m_info.value = 0; 
	else
		segP->m_info.value = value++; 
}

                        /* -------------------------- */

void CSegmentManager::RenumberEquipGens (void) 
{
	int		i, nMatCens, value, nSegment; 
	CSegment	*segP; 

// number "matcen"
nMatCens = 0; 
for (i = 0; i < MineInfo ().equipgen.count; i++) {
	nSegment = EquipGens (i)->m_info.nSegment; 
	if (nSegment >= 0) {
		segP = Segments (nSegment); 
		segP->m_info.value = i; 
		if (segP->m_info.function== SEGMENT_FUNC_EQUIPMAKER)
			segP->m_info.nMatCen = nMatCens++; 
		}
	}

// number "value"
value = 0; 
for (i = 0, segP = Segments (0); i < SegCount (); i++, segP++)
	if (segP->m_info.function== SEGMENT_FUNC_NONE)
		segP->m_info.value = 0; 
	else
		segP->m_info.value = value++; 
}

                        /* -------------------------- */

void CSegmentManager::Copyother.Segmentment ()
{
	bool bUndo, bChange = false;

if (Current1 ().nSegment == Current2 ().nSegment)
	return; 
short nSegment = Current ()->nSegment; 
CSegment *otherSeg = other.Segment (); 
bUndo = undoManager.SetModified (TRUE); 
undoManager.Lock ();
int nSide;
for (nSide = 0; nSide < 6; nSide++)
	if (SetTextures (nSegment, nSide, otherSeg->m_sides [nSide].m_info.nBaseTex, otherSeg->m_sides [nSide].m_info.nOvlTex))
		bChange = true;
if (!bChange)
	undoManager.ResetModified (bUndo);
else {
	undoManager.Unlock ();
	//DLE.MineView ()->Refresh (); 
	}
}

                        /* -------------------------- */

bool CSegmentManager::SplitSegment (void)
{
	CSegment*	centerSegP = current.Segment (), *segP, *childSegP;
	short			nCenterSeg = short (centerSegP - Segments (0));
	short			nSegment, nChildSeg;
	short			nSide, nOppSide, nChildSide;
	short			vertNum, nWall;
	CVertex		segCenter, *segVert, *centerSegVert;
	bool			bVertDone [8], bUndo;
	int			h, i, j, k;
	short			oppSides [6] = {2,3,0,1,5,4};

if (SegCount () >= MAX_SEGMENTS - 6) {
	ErrorMsg ("Cannot split this cube because\nthe maximum number of cubes would be exceeded."); 
	return false;
	}
bUndo = undoManager.SetModified (TRUE); 
undoManager.Lock ();
h = VertCount ();
#if 0
// isolate segment
for (nSide = 0; nSide < 6; nSide++) {
	if (segP->GetChild (nSide) < 0)
		continue;
	for (vertNum = 0; vertNum < 4; vertNum++, h++)
		*Vertices (h) = *Vertices (segP->m_info.verts [sideVertTable [nSide][vertNum]]);
	}
VertCount () = h;
#endif
// compute segment center
segCenter.Clear ();
for (i = 0; i < 8; i++)
	segCenter += *Vertices (centerSegP->m_info.verts [i]);
segCenter /= 8.0;
// add center segment
// compute center segment vertices
memset (bVertDone, 0, sizeof (bVertDone));
for (nSide = 0; nSide < 6; nSide++) {
	for (vertNum = 0; vertNum < 4; vertNum++) {
		j = sideVertTable [nSide][vertNum];
		if (bVertDone [j])
			continue;
		bVertDone [j] = true;
		centerSegVert = Vertices (centerSegP->m_info.verts [j]);
		segVert = Vertices (h + j);
		*segVert = Average (*centerSegVert, segCenter);
		//centerSegP->m_info.verts [j] = h + j;
		}
	}
VertCount () = h + 8;
#if 1
// create the surrounding segments
for (nSegment = SegCount (), nSide = 0; nSide < 6; nSegment++, nSide++) {
	segP = Segments (nSegment);
	nOppSide = oppSides [nSide];
	for (vertNum = 0; vertNum < 4; vertNum++) {
		i = sideVertTable [nSide][vertNum];
		segP->m_info.verts [i] = centerSegP->m_info.verts [i];
#if 0
		j = sideVertTable [nOppSide][vertNum];
		segP->m_info.verts [j] = h + i;
#else
		if ((nSide & 1) || (nSide >= 4)) {
			i = lineVertTable [sideLineTable [nSide][0]][0];
			j = lineVertTable [sideLineTable [nOppSide][2]][0];
			segP->m_info.verts [j] = h + i;
			i = lineVertTable [sideLineTable [nSide][0]][1];
			j = lineVertTable [sideLineTable [nOppSide][2]][1];
			segP->m_info.verts [j] = h + i;
			i = lineVertTable [sideLineTable [nSide][2]][0];
			j = lineVertTable [sideLineTable [nOppSide][0]][0];
			segP->m_info.verts [j] = h + i;
			i = lineVertTable [sideLineTable [nSide][2]][1];
			j = lineVertTable [sideLineTable [nOppSide][0]][1];
			segP->m_info.verts [j] = h + i;
			}
		else {
			i = lineVertTable [sideLineTable [nSide][0]][0];
			j = lineVertTable [sideLineTable [nOppSide][2]][1];
			segP->m_info.verts [j] = h + i;
			i = lineVertTable [sideLineTable [nSide][0]][1];
			j = lineVertTable [sideLineTable [nOppSide][2]][0];
			segP->m_info.verts [j] = h + i;
			i = lineVertTable [sideLineTable [nSide][2]][0];
			j = lineVertTable [sideLineTable [nOppSide][0]][1];
			segP->m_info.verts [j] = h + i;
			i = lineVertTable [sideLineTable [nSide][2]][1];
			j = lineVertTable [sideLineTable [nOppSide][0]][0];
			segP->m_info.verts [j] = h + i;
			}
#endif
		}
	InitSegment (nSegment);
	if ((segP->SetChild (nSide, centerSegP->GetChild (nSide))) > -1) {
		for (childSegP = Segments (segP->GetChild (nSide)), nChildSide = 0; nChildSide < 6; nChildSide++)
			if (childSegP->GetChild (nChildSide) == nCenterSeg) {
				childSegP->SetChild (nChildSide, nSegment);
				break;
				}
			}
	segP->SetChild (nOppSide, nCenterSeg);
	centerSegP->SetChild (nSide, nSegment);
	nWall = centerSegP->m_sides [nSide].m_info.nWall;
	segP->m_sides [nSide].m_info.nWall = nWall;
	if ((nWall >= 0) && (nWall != NO_WALL)) {
		Walls (nWall)->m_nSegment = nSegment;
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
		centerSegP->m_info.verts [j] = h + j;
		}
	}
// join adjacent sides of the segments surrounding the center segment
#if 1
for (nSegment = 0, segP = Segments (SegCount ()); nSegment < 5; nSegment++, segP++) {
	for (nSide = 0; nSide < 6; nSide++) {
		if (segP->GetChild (nSide) >= 0)
			continue;
		for (nChildSeg = nSegment + 1, childSegP = Segments (SegCount () + nChildSeg); 
			  nChildSeg < 6; 
			  nChildSeg++, childSegP++) {
			for (nChildSide = 0; nChildSide < 6; nChildSide++) {
				if (childSegP->GetChild (nChildSide)  >= 0)
					continue;
				h = 0;
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
					segP->SetChild (nSide, SegCount () + nChildSeg);
					childSegP->SetChild (nChildSide, SegCount () + nSegment);
					break;
					}
				}
			}
		}
	}
#endif
SegCount () += 6;
#endif
undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
return true;
}

// ----------------------------------------------------------------------------- 
// DeleteVertex()
//
// ACTION - Removes a vertex from the vertices array and updates all the
//	    Segments () vertices who's vertex is greater than the deleted vertex
// ----------------------------------------------------------------------------- 

void CSegmentManager::DeleteVertex (short nDeletedVert)
{
	short nVertex, nSegment; 

undoManager.SetModified (TRUE); 
// fill in gap in vertex array and status
memcpy (Vertices (nDeletedVert), Vertices (nDeletedVert + 1), (VertCount () - 1 - nDeletedVert) * sizeof (*Vertices (0)));
// update anyone pointing to this vertex
CSegment *segP = Segments (0);
for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++)
	for (nVertex = 0; nVertex < 8; nVertex++)
		if (segP->m_info.verts [nVertex] > nDeletedVert)
			segP->m_info.verts [nVertex]--; 
// update number of vertices
VertCount ()--; 
}

// ----------------------------------------------------------------------------- 
// DeleteUnusedVertices()
//
// ACTION - Deletes unused vertices
// ----------------------------------------------------------------------------- 

void CSegmentManager::DeleteUnusedVertices (void)
{
	short nVertex, nSegment, point; 

for (nVertex = 0; nVertex < VertCount (); nVertex++)
	VertStatus (nVertex) &= ~NEW_MASK; 
// mark all used verts
CSegment *segP = Segments (0);
for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++)
	for (point = 0; point < 8; point++)
		VertStatus (segP->m_info.verts [point]) |= NEW_MASK; 
for (nVertex = VertCount () - 1; nVertex >= 0; nVertex--)
	if (!(VertStatus (nVertex) & NEW_MASK))
		DeleteVertex(nVertex); 
}

// ------------------------------------------------------------------------
//eof segmentmanager.cpp