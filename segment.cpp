// Segment.cpp

#include "stdafx.h"
#include "dle-xp-res.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "io.h"
#include "textures.h"
#include "palette.h"
#include "dle-xp.h"
#include "robot.h"
#include "io.h"

// -------------------------------------------------------------------------- 
// -------------------------------------------------------------------------- 

double CMine::CalcLength(CFixVector* center1, CFixVector* center2)
{
	CFixVector direction; 

	// calculate distance vector between the centers
	direction.x = center1->x - center2->x; 
	direction.y = center1->y - center2->y; 
	direction.z = center1->z - center2->z; 

	// calculate the length of the new cube
	return (sqrt((double)direction.x*(double)direction.x
		 +  (double)direction.y*(double)direction.y
		 +  (double)direction.z*(double)direction.z)); 
}

// ----------------------------------------------------------------- 
// ----------------------------------------------------------------- 
double CMine::dround_off(double value, double round) {
	if (value >= 0) {
		value += round/2; 
	} else {
		value -= round/2; 
	}
	return value; 
}

// -------------------------------------------------------------------------- 
// -------------------------------------------------------------------------- 

void CMine::DeleteSegmentWalls (INT16 nSegment)
{
	CSide *sideP = Segments (nSegment)->sides; 

INT32 i;
for (i = MAX_SIDES_PER_SEGMENT; i; i--, sideP++)
	if (sideP->nWall != NO_WALL)
		DeleteWall (sideP->nWall); 
}

// -------------------------------------------------------------------------- 
// -------------------------------------------------------------------------- 

void CMine::DeleteSegment(INT16 nDelSeg)
{
	CSegment			*segP, *delSegP, *childSegP; 
	CGameObject		*objP; 
	CTrigger			*trigP;
	UINT16			nSegment, real_segnum; 
	INT16				child; 
	INT16				i, j; 

if (SegCount () < 2)
	return; 
if (nDelSeg < 0)
	nDelSeg = Current ()->nSegment; 
if (nDelSeg < 0 || nDelSeg >= SegCount ()) 
	return; 

theApp.SetModified (TRUE);
theApp.LockUndo ();
delSegP = Segments (nDelSeg); 
UndefineSegment (nDelSeg);

// delete any flickering lights that use this segment
for (INT32 nSide = 0; nSide < 6; nSide++) {
	DeleteTriggerTargets (nDelSeg, nSide); 
	INT16 index = GetFlickeringLight(nDelSeg, nSide); 
	if (index != -1) {
		FlickerLightCount ()--; 
		// put last light in place of deleted light
		memcpy(FlickeringLights (index), FlickeringLights (FlickerLightCount ()), 
			sizeof (CFlickeringLight)); 
		}
	}

	// delete any Walls () within segment (if defined)
DeleteSegmentWalls (nDelSeg); 

// delete any Walls () on child Segments () that connect to this segment
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	child = delSegP->children [i]; 
	if (child >= 0 && child < SegCount ()) {
		INT16	oppSegNum, oppSideNum;
		GetOppositeSide (oppSegNum, oppSideNum, nDelSeg, i);
		if (Segments (oppSegNum)->sides [oppSideNum].nWall != NO_WALL)
			DeleteWall (Segments (oppSegNum)->sides [oppSideNum].nWall); 
			}
		}

	// delete any Objects () within segment
for (i = (UINT16)GameInfo ().objects.count - 1; i >= 0; i--) {
	if (Objects (i)->nSegment == nDelSeg) {
		DeleteObject(i); 
		}
	}
#if 0 // done by UndefineSegment ()
	// delete any robot centers with this
	for (i = (UINT16)GameInfo ().botgen.count - 1; i >= 0; i--) {
		nSegment = BotGens (i)->nSegment; 
		if (nSegment == nDelSeg) {
			INT32 nMatCens = --GameInfo ().botgen.count; 
			if (i < nMatCens)
			memcpy ((void *) BotGens (i), (void *) BotGens (nMatCens), sizeof (CRobotMaker)); 
			}
		}

	// delete any equipment centers with this
	for (i = (UINT16) GameInfo ().equipgen.count - 1; i >= 0; i--) {
		nSegment = EquipGens (i)->nSegment; 
		if (nSegment == nDelSeg) {
			GameInfo ().equipgen.count--; 
			memcpy ((void *) EquipGens (i), (void *) EquipGens (i + 1), 
					  (GameInfo ().equipgen.count - i) * sizeof (CRobotMaker)); 
			}
		}
#endif
	for (j = 0; j < GameInfo ().botgen.count; j++)
		if (BotGens (i)->nSegment > nDelSeg)
			BotGens (i)->nSegment--;
	for (j = 0; j < GameInfo ().equipgen.count; j++)
		if (EquipGens (i)->nSegment > nDelSeg)
			EquipGens (i)->nSegment--;
	// delete any control segP with this segment
	for (i = (UINT16)GameInfo ().control.count - 1; i >= 0; i--) {
		INT32 count = ReactorTriggers (i)->m_count; 
		for (j = count - 1; j > 0; j--) {
			if (ReactorTriggers (i)->Segment (j) == nDelSeg) {
				// move last segment into this spot
				ReactorTriggers (i)->Delete (j);
				}
			}
		}

	// update secret cube number if out of range now
	nSegment = (UINT16) SecretCubeNum (); 
	if (nSegment >= SegCount () || nSegment== nDelSeg)
		SecretCubeNum () = 0; 

	// update segment flags
	delSegP->wallFlags &= ~MARKED_MASK; 

	// unlink any children with this segment number
	for (nSegment = 0, segP = Segments (); nSegment < SegCount (); nSegment++, segP++) {
		for (child = 0; child < MAX_SIDES_PER_SEGMENT; child++) {
			if (segP->children [child]== nDelSeg) {

				// subtract by 1 if segment is above deleted segment
				Current ()->nSegment = nSegment; 
				if (nSegment > nDelSeg) 
					Current ()->nSegment--; 

				// remove child number and update child bitmask
				segP->children [child] = -1; 
				segP->childFlags &= ~(1 << child); 

				// define textures, (u, v) and light
				CSide *sideP = delSegP->sides + child;
				SetTexture (nSegment, child, sideP->nBaseTex, sideP->nOvlTex); 
				SetUV (nSegment, child, 0, 0, 0); 
				double scale = pTextures [m_fileType][sideP->nBaseTex].Scale (sideP->nBaseTex);
				for (i = 0; i < 4; i++) {
					segP->sides [child].uvls [i].u = (INT16) ((double) default_uvls [i].u / scale); 
					segP->sides [child].uvls [i].v = (INT16) ((double) default_uvls [i].v / scale); 
					segP->sides [child].uvls [i].l = delSegP->sides [child].uvls [i].l; 
				}
			}
		}
	}

	// move other Segments () to deleted segment location
	if (nDelSeg != SegCount ()-1) { // if this is not the last segment

		// mark each segment with it's real number
		real_segnum = 0; 
		for (nSegment = 0, segP = Segments (); nSegment < SegCount (); nSegment++, segP++)
			if(nDelSeg != nSegment)
				segP->nIndex = real_segnum++; 

		// replace all children with real numbers
		for (nSegment = 0, segP = Segments (); nSegment < SegCount (); nSegment++, segP++) {
			for (child = 0; child < MAX_SIDES_PER_SEGMENT; child++) {
				if (segP->childFlags & (1 << child)
					&& segP->children [child] >= 0 && segP->children [child] < SegCount ()) { // debug fix
					childSegP = &Segments () [segP->children [child]]; 
					segP->children [child] = childSegP->nIndex; 
				}
			}
		}

		// replace all wall segP numbers with real numbers
		for (i = 0; i < GameInfo ().walls.count; i++) {
			nSegment = (INT16) Walls (i)->m_nSegment; 
			if (nSegment < SegCount ()) {
				segP = Segments (nSegment); 
				Walls (i)->m_nSegment = segP->nIndex; 
				} 
			else {
				Walls (i)->m_nSegment = 0; // fix wall segment number
			}
		}

		// replace all trigger segP numbers with real numbers
		for (i = NumTriggers (), trigP = Triggers (); i; i--, trigP++) {
			for (j = 0; j < trigP->m_count; j++) {
				if (SegCount () > (nSegment = trigP->Segment (j)))
					trigP->Segment (j) = Segments (nSegment)->nIndex; 
				else {
					DeleteTargetFromTrigger (trigP, j, 0);
					j--;
					}
				}
			}

		// replace all trigger segP numbers with real numbers
		for (i = NumObjTriggers (), trigP = ObjTriggers (); i; i--, trigP++) {
			for (j = 0; j < trigP->m_count; j++) {
				if (SegCount () > (nSegment = trigP->Segment (j)))
					trigP->Segment (j) = Segments (nSegment)->nIndex; 
				else {
					DeleteTargetFromTrigger (trigP, j, 0);
					j--;
					}
				}
			}

		// replace all object segP numbers with real numbers
		for (i = 0; i < GameInfo ().objects.count; i++) {
			objP = Objects (i); 
			if (SegCount () > (nSegment = objP->nSegment))
				objP->nSegment = Segments (nSegment)->nIndex; 
			else
				objP->nSegment = 0; // fix object segment number
			}

		// replace robot centers segP numbers with real numbers
		for (i = 0; i < GameInfo ().botgen.count; i++) {
			if (SegCount () > (nSegment = BotGens (i)->nSegment))
				BotGens (i)->nSegment = Segments (nSegment)->nIndex; 
			else
				BotGens (i)->nSegment = 0; // fix robot center nSegment
			}

		// replace equipment centers segP numbers with real numbers
		for (i = 0; i < GameInfo ().equipgen.count; i++) {
			if (SegCount () > (nSegment = EquipGens (i)->nSegment))
				EquipGens (i)->nSegment = Segments (nSegment)->nIndex; 
			else
				EquipGens (i)->nSegment = 0; // fix robot center nSegment
			}

		// replace control segP numbers with real numbers
		for (i = 0; i < GameInfo ().control.count; i++) {
			for (j = 0; j < ReactorTriggers (i)->m_count; j++) {
				if (SegCount () > (nSegment = ReactorTriggers (i)->Segment (j)))
					ReactorTriggers (i)->Segment (j) = Segments (nSegment)->nIndex; 
				else 
					ReactorTriggers (i)->Segment (j) = 0; // fix control center segment number
			}
		}

		// replace flickering light segP numbers with real numbers
		for (i = 0; i < FlickerLightCount (); i++) {
			if (SegCount () > (nSegment = FlickeringLights (i)->m_nSegment))
				FlickeringLights (i)->m_nSegment = Segments (nSegment)->nIndex; 
			else 
				FlickeringLights (i)->m_nSegment = 0; // fix object segment number
			}

		// replace secret cubenum with real number
		if (SegCount () > (nSegment = (UINT16) SecretCubeNum ()))
			SecretCubeNum () = Segments (nSegment)->nIndex; 
		else
			SecretCubeNum () = 0; // fix secret cube number

		// move remaining Segments () down by 1
  }
#if 1
		if (INT32 segC = (--SegCount () - nDelSeg)) {
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
theApp.MineView ()->Refresh (false); 
theApp.ToolView ()->Refresh (); 
theApp.UnlockUndo ();
}



// -------------------------------------------------------------------------- 
// DeleteVertex()
//
// ACTION - Removes a vertex from the vertices array and updates all the
//	    Segments () vertices who's vertex is greater than the deleted vertex
// -------------------------------------------------------------------------- 

void CMine::DeleteVertex(INT16 nDeletedVert)
{
	INT16 nVertex, nSegment; 

theApp.SetModified (TRUE); 
// fill in gap in vertex array and status
memcpy (Vertices (nDeletedVert), Vertices (nDeletedVert + 1), (VertCount () - 1 - nDeletedVert) * sizeof (*Vertices ()));
// update anyone pointing to this vertex
CSegment *segP = Segments (0);
for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++)
	for (nVertex = 0; nVertex < 8; nVertex++)
		if (segP->verts [nVertex] > nDeletedVert)
			segP->verts [nVertex]--; 
// update number of vertices
VertCount ()--; 
}

// -------------------------------------------------------------------------- 
// DeleteUnusedVertices()
//
// ACTION - Deletes unused vertices
// -------------------------------------------------------------------------- 

void CMine::DeleteUnusedVertices (void)
{
	INT16 nVertex, nSegment, point; 

for (nVertex = 0; nVertex < VertCount (); nVertex++)
	VertStatus (nVertex) &= ~NEW_MASK; 
// mark all used verts
CSegment *segP = Segments (0);
for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++)
	for (point = 0; point < 8; point++)
		VertStatus (segP->verts [point]) |= NEW_MASK; 
for (nVertex = VertCount ()-1; nVertex >= 0; nVertex--)
	if (!(VertStatus (nVertex) & NEW_MASK))
		DeleteVertex(nVertex); 
}

// -------------------------------------------------------------------------- 
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
// -------------------------------------------------------------------------- 

void CMine::InitSegment (INT16 nSegment)
{
	CSegment	*segP = Segments (nSegment);
	INT16			nSide;

// define special, etc..
segP->owner = -1;
segP->group = -1;
segP->function = 0; 
segP->nMatCen = -1; 
segP->value = -1; 
segP->childFlags = 0;
// define Walls ()
segP->wallFlags = 0; // unmarked cube
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	segP->sides [nSide].nWall = NO_WALL; 
	segP->sides [nSide].nBaseTex =
	segP->sides [nSide].nOvlTex = 0; 
	INT32 i;
	for (i = 0; i < 4; i++)
		segP->sides [nSide].uvls [i].l = (UINT16) DEFAULT_LIGHTING; 
	SetUV (nSegment, nSide, 0, 0, 0); 
	}
segP->static_light = 0; 
MEMSET (segP->children, 0xFF, sizeof (segP->children));
}

// -------------------------------------------------------------------------- 

bool CMine::AddSegment ()
{
	CSegment *segP, *currSeg; 
	INT16 i, nNewSeg, nNewSide, nCurrSide = Current ()->nSide; 
	INT16 new_verts [4]; 
	INT16 nSegment, nSide; 

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return FALSE; 
	}

currSeg = Segments (Current ()->nSegment); 

if (SegCount () >= MAX_SEGMENTS) {
	ErrorMsg ("Cannot add a new cube because\nthe maximum number of cubes has been reached."); 
	return FALSE;
	}
if (SegCount () >= MAX_SEGMENTS) {
	ErrorMsg ("Cannot add a new cube because\nthe maximum number of vertices has been reached."); 
	return FALSE;
	}
if (currSeg->children [nCurrSide] >= 0) {
	ErrorMsg ("Can not add a new cube to a side\nwhich already has a cube attached."); 
	return FALSE;
	}

theApp.SetModified (TRUE); 
theApp.LockUndo ();
// get new verts
new_verts [0] = VertCount () + 0; 
new_verts [1] = VertCount () + 1; 
new_verts [2] = VertCount () + 2; 
new_verts [3] = VertCount () + 3; 

// get new segment
nNewSeg = SegCount (); 
segP = Segments (nNewSeg); 

// define vertices
DefineVertices (new_verts); 

// define vert numbers for common side
segP->verts [opp_side_vert [nCurrSide][0]] = currSeg->verts [side_vert [nCurrSide][0]]; 
segP->verts [opp_side_vert [nCurrSide][1]] = currSeg->verts [side_vert [nCurrSide][1]]; 
segP->verts [opp_side_vert [nCurrSide][2]] = currSeg->verts [side_vert [nCurrSide][2]]; 
segP->verts [opp_side_vert [nCurrSide][3]] = currSeg->verts [side_vert [nCurrSide][3]]; 

// define vert numbers for new side
segP->verts [side_vert [nCurrSide][0]] = new_verts [0]; 
segP->verts [side_vert [nCurrSide][1]] = new_verts [1]; 
segP->verts [side_vert [nCurrSide][2]] = new_verts [2]; 
segP->verts [side_vert [nCurrSide][3]] = new_verts [3]; 

InitSegment (nNewSeg);
// define children and special child
segP->childFlags = 1 << opp_side [nCurrSide]; /* only opposite side connects to current_segment */
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) /* no remaining children */
	segP->children [i] = (segP->childFlags & (1 << i)) ? Current ()->nSegment : -1;

// define textures
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	if (segP->children [nSide] < 0) {
		// if other segment does not have a child (therefore it has a texture)
		if (currSeg->children [nSide] < 0 && currSeg->function == SEGMENT_FUNC_NONE) {
			segP->sides [nSide].nBaseTex = currSeg->sides [nSide].nBaseTex; 
			segP->sides [nSide].nOvlTex = currSeg->sides [nSide].nOvlTex; 
			for (i = 0; i < 4; i++) 
				segP->sides [nSide].uvls [i].l = currSeg->sides [nSide].uvls [i].l; 
			} 
		}
	else {
		MEMSET (segP->sides [nSide].uvls, 0, sizeof (segP->sides [nSide].uvls));
		}
	}

// define static light
segP->static_light = currSeg->static_light; 

// delete flickering light if it exists
INT16 index = GetFlickeringLight (Current ()->nSegment, nCurrSide); 
if (index != -1) {
	FlickerLightCount ()--; 
	// put last light in place of deleted light
	memcpy( FlickeringLights (index), FlickeringLights (FlickerLightCount ()), sizeof (CFlickeringLight)); 
	}

// update current segment
currSeg->children [nCurrSide] = nNewSeg; 
currSeg->childFlags |= (1 << nCurrSide); 
currSeg->sides [nCurrSide].nBaseTex = 0; 
currSeg->sides [nCurrSide].nOvlTex = 0; 
MEMSET (currSeg->sides [nCurrSide].uvls, 0, sizeof (currSeg->sides [nCurrSide].uvls));
 
// update number of Segments () and vertices and clear vertexStatus
SegCount ()++;
for (int i = 0; i < 4; i++)
	Vertices (VertCount ()++)->m_status = 0;

// link the new segment with any touching Segments ()
CSegment *pSeg = Segments ();
CFixVector *vNewSeg = Vertices (Segments (nNewSeg)->verts [0]);
CFixVector *vSeg;
for (nSegment = 0; nSegment < SegCount (); nSegment++, pSeg++) {
	if (nSegment!= nNewSeg) {
		// first check to see if Segments () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		vSeg = Vertices (pSeg->verts [0]);
		if (labs (vNewSeg->x - vSeg->x) < 0xA00000L &&
			 labs (vNewSeg->y - vSeg->y) < 0xA00000L &&
			 labs (vNewSeg->z - vSeg->z) < 0xA00000L)
			for (nNewSide = 0; nNewSide < 6; nNewSide++)
				for (nSide = 0; nSide < 6; nSide++)
					LinkSegments(nNewSeg,nNewSide,nSegment,nSide,3*F1_0);
		}
	}
// auto align textures new segment
for (nNewSide = 0; nNewSide < 6; nNewSide++)
	AlignTextures (Current ()->nSegment, nNewSide, nNewSeg, TRUE, TRUE); 
// set current segment to new segment
Current ()->nSegment = nNewSeg; 
//		SetLinesToDraw(); 
theApp.MineView ()->Refresh (false); 
theApp.ToolView ()->Refresh (); 
theApp.UnlockUndo ();
return TRUE; 
}

// -------------------------------------------------------------------------- 
// DefineVertices()
//
//  ACTION - Calculates vertices when adding a new segment.
//
// -------------------------------------------------------------------------- 

#define CURRENT_POINT(a) ((Current ()->nPoint + (a))&0x03)

void CMine::DefineVertices (INT16 new_verts [4])
{
	CSegment *currSeg; 
	struct dvector A [8], B [8], C [8], D [8], E [8], a, b, c, d; 
	double angle1, angle2, angle3; 
	double length; 
	INT16 nVertex; 
	INT16 i; 
	CFixVector center, opp_center, orthog; 
	CFixVector *vert, new_center; 

	currSeg = Segments (Current ()->nSegment); 

	// METHOD 1: orthogonal with right angle on new side and standard cube side
// TODO:
//	INT32 add_segment_mode = ORTHOGONAL; 
	switch (add_segment_mode)
	{
		case(ORTHOGONAL):
		{
			CalcCenter(center, Current ()->nSegment, Current ()->nSide); 
			CalcCenter(opp_center, Current ()->nSegment, opp_side [Current ()->nSide]); 

			CalcOrthoVector(orthog, Current ()->nSegment, Current ()->nSide); 

			// set the length of the new cube to be one standard cube length
			length = 20; 

			// scale the vector
			orthog.x = (long) ((double) orthog.x * length); 
			orthog.y = (long) ((double) orthog.y * length); 
			orthog.z = (long) ((double) orthog.z * length); 

			// figure out new center
			new_center.x = center.x + orthog.x; 
			new_center.y = center.y + orthog.y; 
			new_center.z = center.z + orthog.z; 

			// new method: extend points 0 and 1 with orthog, then move point 0 toward point 1.
			double factor; 

			// point 0
			nVertex = currSeg->verts [side_vert [Current ()->nSide][CURRENT_POINT(0)]];
			vert = Vertices (nVertex);
			a.x = orthog.x + vert->x; 
			a.y = orthog.y + vert->y; 
			a.z = orthog.z + vert->z; 

			// point 1
			nVertex = currSeg->verts [side_vert [Current ()->nSide][CURRENT_POINT(1)]]; 
			vert = Vertices (nVertex);
			b.x = orthog.x + vert->x; 
			b.y = orthog.y + vert->y; 
			b.z = orthog.z + vert->z; 

			// center
			c.x = (a.x + b.x) / 2; 
			c.y = (a.y + b.y) / 2; 
			c.z = (a.z + b.z) / 2; 

			// vector from center to point0 and its length
			d.x = a.x - c.x; 
			d.y = a.y - c.y; 
			d.z = a.z - c.z; 
			length = sqrt(d.x*d.x + d.y*d.y + d.z*d.z); 

			// factor to mul
			if (length > 0) {
				factor = 10.0*F1_0 /length; 
			} else {
				factor = 1.0; 
			}

			// set point 0
			A [CURRENT_POINT(0)].x = (c.x + factor * d.x); 
			A [CURRENT_POINT(0)].y = (c.y + factor * d.y); 
			A [CURRENT_POINT(0)].z = (c.z + factor * d.z); 

			// set point 1
			A [CURRENT_POINT(1)].x = (c.x - factor * d.x); 
			A [CURRENT_POINT(1)].y = (c.y - factor * d.y); 
			A [CURRENT_POINT(1)].z = (c.z - factor * d.z); 

			// point 2 is orthogonal to the vector 01 and the orthog vector
			a.x = orthog.x; 
			a.y = orthog.y; 
			a.z = orthog.z; 
			b.x = A [CURRENT_POINT(0)].x - A [CURRENT_POINT(1)].x; 
			b.y = A [CURRENT_POINT(0)].y - A [CURRENT_POINT(1)].y; 
			b.z = A [CURRENT_POINT(0)].z - A [CURRENT_POINT(1)].z; 
			c.x = a.y*b.z - a.z*b.y; 
			c.y = a.z*b.x - a.x*b.z; 
			c.z = a.x*b.y - a.y*b.x; 
			// normalize the vector
			length = sqrt(c.x*c.x + c.y*c.y + c.z*c.z); 
			if (length>0) {
				c.x /= length; 
				c.y /= length; 
				c.z /= length; 
			}
			A [CURRENT_POINT(2)].x = A [CURRENT_POINT(1)].x + c.x * 20*F1_0; 
			A [CURRENT_POINT(2)].y = A [CURRENT_POINT(1)].y + c.y * 20*F1_0; 
			A [CURRENT_POINT(2)].z = A [CURRENT_POINT(1)].z + c.z * 20*F1_0; 

			A [CURRENT_POINT(3)].x = A [CURRENT_POINT(0)].x + c.x * 20*F1_0; 
			A [CURRENT_POINT(3)].y = A [CURRENT_POINT(0)].y + c.y * 20*F1_0; 
			A [CURRENT_POINT(3)].z = A [CURRENT_POINT(0)].z + c.z * 20*F1_0; 

			// now center the side along about the new_center
			a.x = (A [0].x + A [1].x + A [2].x + A [3].x)/4; 
			a.y = (A [0].y + A [1].y + A [2].y + A [3].y)/4; 
			a.z = (A [0].z + A [1].z + A [2].z + A [3].z)/4; 
			for (i = 0; i < 4; i++) {
				A [i].x += new_center.x - a.x; 
				A [i].y += new_center.y - a.y; 
				A [i].z += new_center.z - a.z; 
			}

			// set the new vertices
			for (i = 0; i < 4; i++) {
				//nVertex = currSeg->verts [side_vert [Current ()->nSide][i]]; 
				nVertex = new_verts [i];
				Vertices (nVertex)->x = (long) dround_off(A [i].x, 1.0); 
				Vertices (nVertex)->y = (long) dround_off(A [i].y, 1.0); 
				Vertices (nVertex)->z = (long) dround_off(A [i].z, 1.0); 
			}
		}
		break; 
		// METHOD 2: orghogonal with right angle on new side
		case(EXTEND):
		{
			CalcCenter(center, Current ()->nSegment, Current ()->nSide); 
			CalcCenter(opp_center, Current ()->nSegment, opp_side [Current ()->nSide]); 

			CalcOrthoVector(orthog, Current ()->nSegment, Current ()->nSide); 

			// calculate the length of the new cube
			length = CalcLength(&center, &opp_center) / 0x10000L; 

			// scale the vector
			orthog.x = (long) ((double) orthog.x * length); 
			orthog.y = (long) ((double) orthog.y * length); 
			orthog.z = (long) ((double) orthog.z * length); 

			// set the new vertices
			for (i = 0; i < 4; i++) {
				INT32 v1 = currSeg->verts [side_vert [Current ()->nSide][i]]; 
				INT32 v2 = new_verts [i];
				Vertices (v2)->x = orthog.x + Vertices (v1)->x; 
				Vertices (v2)->y = orthog.y + Vertices (v1)->y; 
				Vertices (v2)->z = orthog.z + Vertices (v1)->z; 
			}
		}
		break; 

		// METHOD 3: mirror relative to plane of side
		case(MIRROR):
		{
			// copy side's four points into A
			for (i = 0; i < 4; i++) {
				nVertex = currSeg->verts [side_vert [Current ()->nSide][i]]; 
				A [i].x = Vertices (nVertex)->x; 
				A [i].y = Vertices (nVertex)->y; 
				A [i].z = Vertices (nVertex)->z; 
				nVertex = currSeg->verts [opp_side_vert [Current ()->nSide][i]]; 
				A [i + 4].x = Vertices (nVertex)->x; 
				A [i + 4].y = Vertices (nVertex)->y; 
				A [i + 4].z = Vertices (nVertex)->z; 
			}

			// subtract point 0 from all points in A to form B points
			for (i = 0; i < 8; i++) {
				B [i].x = A [i].x - A [0].x; 
				B [i].y = A [i].y - A [0].y; 
				B [i].z = A [i].z - A [0].z; 
			}

			// calculate angle to put point 1 in x - y plane by spinning on x - axis
			// then rotate B points on x - axis to form C points.
			// check to see if on x - axis already
			//    if (B [1].z== B [1].y) {
			//      angle1 = PI/4; 
			//    } else {
			angle1 = atan3(B [1].z, B [1].y); 
			//    }
			for (i = 0; i < 8; i++) {
				C [i].x = B [i].x; 
				C [i].y = B [i].y * cos(angle1) + B [i].z * sin(angle1); 
				C [i].z = - B [i].y * sin(angle1) + B [i].z * cos(angle1); 
			}

			// calculate angle to put point 1 on x axis by spinning on z - axis
			// then rotate C points on z - axis to form D points
			// check to see if on z - axis already
			//    if (C [1].y== C [1].x) {
			//      angle2 = PI/4; 
			//    } else {
			angle2 = atan3(C [1].y, C [1].x); 
			//    }
			for (i = 0; i < 8; i++) {
				D [i].x = C [i].x * cos(angle2) + C [i].y * sin(angle2); 
				D [i].y = - C [i].x * sin(angle2) + C [i].y * cos(angle2); 
				D [i].z = C [i].z; 
			}

			// calculate angle to put point 2 in x - y plane by spinning on x - axis
			// the rotate D points on x - axis to form E points
			// check to see if on x - axis already
			//    if (D [2].z== D [2].y) {
			//      angle3 = PI/4; 
			//    } else {
			angle3 = atan3(D [2].z, D [2].y); 
			//    }
			for (i = 0; i < 8; i++) {
				E [i].x = D [i].x; 
				E [i].y = D [i].y * cos(angle3) + D [i].z * sin(angle3); 
				E [i].z = - D [i].y * sin(angle3) + D [i].z * cos(angle3); 
			}

			// now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
			// mirror new points on z axis
			for (i = 4; i < 8; i++) {
				E [i].z = - E [i].z; 
			}

			// now reverse rotations
			for (i = 4; i < 8; i++) {
				D [i].x = E [i].x; 
				D [i].y = E [i].y * cos(- angle3) + E [i].z * sin(- angle3); 
				D [i].z = - E [i].y * sin(- angle3) + E [i].z * cos(- angle3); 
			}
			for (i = 4; i < 8; i++) {
				C [i].x = D [i].x * cos(- angle2) + D [i].y * sin(- angle2); 
				C [i].y = - D [i].x * sin(- angle2) + D [i].y * cos(- angle2); 
				C [i].z = D [i].z; 
			}
			for (i = 4; i < 8; i++) {
				B [i].x = C [i].x; 
				B [i].y = C [i].y * cos(- angle1) + C [i].z * sin(- angle1); 
				B [i].z = - C [i].y * sin(- angle1) + C [i].z * cos(- angle1); 
			}
			// and translate back
			nVertex = currSeg->verts [side_vert [Current ()->nSide][0]]; 
			for (i = 4; i < 8; i++) {
				A [i].x = B [i].x + Vertices (nVertex)->x; 
				A [i].y = B [i].y + Vertices (nVertex)->y; 
				A [i].z = B [i].z + Vertices (nVertex)->z; 
			}

			for (i = 0; i < 4; i++) {
				INT32 nVertex = new_verts [i];
				Vertices (nVertex)->x = (long) dround_off(A [i + 4].x, 1.0); 
				Vertices (nVertex)->y = (long) dround_off(A [i + 4].y, 1.0); 
				Vertices (nVertex)->z = (long) dround_off(A [i + 4].z, 1.0); 
			}
		}
	}
}

// -------------------------------------------------------------------------- 
// LinkSegments()
//
//  Action - checks 2 Segments () and 2 sides to see if the vertices are identical
//           If they are, then the segment sides are linked and the vertices
//           are removed (sidenum1 is the extra side).
//
//  Change - no longer links if segment already has a child
//           no longer links Segments () if vert numbers are not in the right order
//
// -------------------------------------------------------------------------- 

bool CMine::LinkSegments (INT16 segnum1, INT16 sidenum1, INT16 segnum2, INT16 sidenum2, FIX margin)
{
	CSegment *seg1, *seg2; 
	INT16 i, j; 
	CFixVector v1 [4], v2 [4]; 
	INT16 fail;
	tVertMatch match [4]; 

	seg1 = Segments (segnum1); 
	seg2 = Segments (segnum2); 

// don't link to a segment which already has a child
if (seg1->children [sidenum1]!=-1 || seg2->children [sidenum2]!=-1)
	return FALSE; 

// copy vertices for comparison later (makes code more readable)
for (i = 0; i < 4; i++) {
	INT32 nVertex = seg1->verts [side_vert [sidenum1][i]];
	memcpy (v1 + i, Vertices (nVertex), sizeof (*Vertices ()));
/*
	v1 [i].x = Vertices (nVertex)->x; 
	v1 [i].y = Vertices (nVertex)->y; 
	v1 [i].z = Vertices (nVertex)->z; 
*/
	nVertex = seg2->verts [side_vert [sidenum2][i]];
	memcpy (v2 + i, Vertices (nVertex), sizeof (*Vertices ()));
/*
	v2 [i].x = Vertices (nVertex)->x; 
	v2 [i].y = Vertices (nVertex)->y; 
	v2 [i].z = Vertices (nVertex)->z; 
*/
	match [i].i =-1; 
}

// check to see if all 4 vertices match exactly one of each of the 4 other cube's vertices
fail = 0;   // assume test will pass for now
for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
		if (labs (v1 [i].x - v2 [j].x) < margin &&
			 labs (v1 [i].y - v2 [j].y) < margin &&
			 labs (v1 [i].z - v2 [j].z) < margin)
			if (match [j].i != -1) // if this vertex already matched another vertex then abort
				return FALSE; 
			else
				match [j].i = i;  // remember which vertex it matched
/*
for (i = 0; i < 4; i++)
	match [i] = -1;
for (i = 0; i < 4; i++)
	for (j = 0; j < 4; j++)
		if (labs (v1 [i].x - v2 [j].x) < margin &&
			 labs (v1 [i].y - v2 [j].y) < margin &&
			 labs (v1 [i].z - v2 [j].z) < margin)
			match [i] = j;  // remember which vertex it matched
*/
if (match [0].i == -1)
	return FALSE;
static INT32 matches [][4] = {{0,3,2,1},{1,0,3,2},{2,1,0,3},{3,2,1,0}};
for (i = 1; i < 4; i++)
	if (match [i].i != matches [match [0].i][i])
		return FALSE;
// make sure verts match in the correct order
/*
if ((match [0] == 0) && (match [1] != 3 || match [2] != 2 || match [3] != 1)) fail = 1; 
else if ((match [0] == 1) && (match [1] != 0 || match [2] != 3 || match [3] != 2)) fail = 1; 
else if ((match [0] == 2) && (match [1] != 1 || match [2] != 0 || match [3] != 3)) fail = 1; 
else if ((match [0] == 3) && (match [1] != 2 || match [2] != 1 || match [3] != 0)) fail = 1; 
*/
// if not failed and match found for each
LinkSides (segnum1, sidenum1, segnum2, sidenum2, match); 
return TRUE; 
}


// -------------------------------------------------------------------------- 
// LinkSides()
// -------------------------------------------------------------------------- 

void CMine::LinkSides (INT16 segnum1, INT16 sidenum1, INT16 segnum2, INT16 sidenum2, tVertMatch match [4]) 
{
	CSegment *seg1, *seg2; 
	seg1 = Segments (segnum1); 
	seg2 = Segments (segnum2); 
	INT32 i; 
	INT16 nSegment, nVertex, oldVertex, newVertex; 

	seg1->children [sidenum1] = segnum2; 
	seg1->childFlags |= (1 << sidenum1); 
	seg1->sides [sidenum1].nBaseTex = 0; 
	seg1->sides [sidenum1].nOvlTex = 0; 
	for (i = 0; i < 4; i++) {
		seg1->sides [sidenum1].uvls [i].u = 0; 
		seg1->sides [sidenum1].uvls [i].v = 0; 
		seg1->sides [sidenum1].uvls [i].l = 0; 
	}
	seg2->children [sidenum2] = segnum1; 
	seg2->childFlags |= (1 << sidenum2); 
	seg2->sides [sidenum2].nBaseTex = 0; 
	seg2->sides [sidenum2].nOvlTex = 0; 
	for (i = 0; i < 4; i++) {
		seg2->sides [sidenum2].uvls [i].u = 0; 
		seg2->sides [sidenum2].uvls [i].v = 0; 
		seg2->sides [sidenum2].uvls [i].l = 0; 
	}

	// merge vertices
	for (i = 0; i < 4; i++) {
		oldVertex = seg1->verts [side_vert [sidenum1][i]]; 
		newVertex = seg2->verts [side_vert [sidenum2][match [i].i]]; 

		// if either vert was marked, then mark the new vert
		VertStatus (newVertex) |= (VertStatus (oldVertex) & MARKED_MASK); 

		// update all Segments () that use this vertex
		if (oldVertex != newVertex) {
			CSegment *segP = Segments (0);
			for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++)
				for (nVertex = 0; nVertex < 8; nVertex++)
					if (segP->verts [nVertex] == oldVertex)
						segP->verts [nVertex] = newVertex; 
			// then delete the vertex
			DeleteVertex (oldVertex); 
		}
	}
}
// ------------------------------------------------------------------------- 
// calculate_segment_center()
// ------------------------------------------------------------------------- 

void CMine::CalcSegCenter(CFixVector& pos, INT16 nSegment) 
{
  INT16	*nVerts =Segments (nSegment)->verts; 
  CFixVector *vert;
  
MEMSET (&pos, 0, sizeof (pos));
INT32 i;
for (i = 0; i < 8; i++) {
	vert = Vertices (nVerts [i]);
	pos.x += vert->x;
	pos.y += vert->y;
	pos.z += vert->z;
	}
pos.x /= 8;
pos.y /= 8;
pos.z /= 8;
/*
  pos.x  = 
      (Vertices (verts [0])->x
       + Vertices (verts [1])->x
       + Vertices (verts [2])->x
       + Vertices (verts [3])->x
       + Vertices (verts [4])->x
       + Vertices (verts [5])->x
       + Vertices (verts [6])->x
       + Vertices (verts [7])->x)/8; 
  pos.y  = 
      (Vertices (verts [0])->y
       + Vertices (verts [1])->y
       + Vertices (verts [2])->y
       + Vertices (verts [3])->y
       + Vertices (verts [4])->y
       + Vertices (verts [5])->y
       + Vertices (verts [6])->y
       + Vertices (verts [7])->y)/8; 
  pos.z  = 
      (Vertices (verts [0])->z
       + Vertices (verts [1])->z
       + Vertices (verts [2])->z
       + Vertices (verts [3])->z
       + Vertices (verts [4])->z
       + Vertices (verts [5])->z
       + Vertices (verts [6])->z
       + Vertices (verts [7])->z)/8; 
*/
}

//========================================================================== 
// SideIsMarked
//========================================================================== 

bool CMine::SideIsMarked (INT16 nSegment, INT16 nSide)
{
GetCurrent (nSegment, nSide);
CSegment *segP = Segments (nSegment);
for (INT32 i = 0; i < 4; i++) {
	if (!(VertStatus (segP->verts [side_vert [nSide][i]]) & MARKED_MASK))
		return false;
	}
return true;
}

bool CMine::SegmentIsMarked (INT16 nSegment)
{
CSegment *segP = Segments (nSegment);
for (INT32 i = 0;  i < 8; i++)
	if (!(VertStatus (segP->verts [i]) & MARKED_MASK))
		return false;
return true;
}

//========================================================================== 
// MENU - Mark
//========================================================================== 
void CMine::Mark()
{
	bool	bCubeMark = false; 
	CSegment *segP = CurrSeg (); 
	INT32 i, p [8], n_points; 

switch (theApp.MineView ()->GetSelectMode ()) {
	case eSelectPoint:
		n_points = 1; 
		p [0] = segP->verts [side_vert [Current ()->nSide][Current ()->nPoint]]; 
		break; 
	case eSelectLine:
		n_points = 2; 
		p [0] = segP->verts [side_vert [Current ()->nSide][Current ()->nPoint]]; 
		p [1] = segP->verts [side_vert [Current ()->nSide][(Current ()->nPoint + 1)&3]]; 
		break; 
	case eSelectSide:
		n_points = 4; 
		for (i = 0; i < n_points; i++)
			p [i] = segP->verts [side_vert [Current ()->nSide][i]]; 
		break; 
	default:
		bCubeMark = true; 
	}

if (bCubeMark)
		MarkSegment (Current ()->nSegment); 
else {
	// set i to n_points if all verts are marked
	for (i = 0; i < n_points; i++)
		if (!(VertStatus (p [i]) & MARKED_MASK))
			break; 
		// if all verts are marked, then unmark them
	if (i== n_points)
		for (i = 0; i < n_points; i++)
			VertStatus (p [i]) &= ~MARKED_MASK; 
	else
		// otherwise mark all the points
		for (i = 0; i < n_points; i++)
			VertStatus (p [i]) |= MARKED_MASK; 
		UpdateMarkedCubes(); 
	}
theApp.MineView ()->Refresh (); 
}

// -------------------------------------------------------------------------- 
//			 mark_segment()
//
//  ACTION - Toggle marked bit of segment and mark/unmark vertices.
//
// -------------------------------------------------------------------------- 
void CMine::MarkSegment(INT16 nSegment)
{
  CSegment *segP = Segments (0) + nSegment; 

	segP->wallFlags ^= MARKED_MASK; /* flip marked bit */

	// update vertices's marked status
	// ..first clear all marked verts
	INT16 nVertex; 
	for (nVertex = 0; nVertex < MAX_VERTICES; nVertex++)
		VertStatus (nVertex) &= ~MARKED_MASK; 
	// ..then mark all verts for marked Segments ()
	for (nSegment = 0, segP = Segments (); nSegment < SegCount (); nSegment++, segP++)
		if (segP->wallFlags & MARKED_MASK)
			for (nVertex = 0; nVertex < 8; nVertex++)
				VertStatus (segP->verts [nVertex]) |= MARKED_MASK; 
}

// -------------------------------------------------------------------------- 
// update_marked_cubes()
// -------------------------------------------------------------------------- 
void CMine::UpdateMarkedCubes()
{
	CSegment *segP; 
	INT32 i; 
	// mark all cubes which have all 8 verts marked
	for (i = 0, segP = Segments (); i < SegCount (); i++, segP++)
		if ((VertStatus (segP->verts [0]) & MARKED_MASK) &&
			 (VertStatus (segP->verts [1]) & MARKED_MASK) &&
			 (VertStatus (segP->verts [2]) & MARKED_MASK) &&
			 (VertStatus (segP->verts [3]) & MARKED_MASK) &&
			 (VertStatus (segP->verts [4]) & MARKED_MASK) &&
			 (VertStatus (segP->verts [5]) & MARKED_MASK) &&
			 (VertStatus (segP->verts [6]) & MARKED_MASK) &&
			 (VertStatus (segP->verts [7]) & MARKED_MASK))
			segP->wallFlags |= MARKED_MASK; 
		else
			segP->wallFlags &= ~MARKED_MASK; 
}

//========================================================================== 
// MENU - Mark all cubes
//========================================================================== 

void CMine::MarkAll() 
{
	INT32 i; 

for (i = 0; i < SegCount (); i++) 
	Segments (i)->wallFlags |= MARKED_MASK; 
for (i = 0; i < VertCount (); i++) 
	VertStatus (i) |= MARKED_MASK; 
theApp.MineView ()->Refresh (); 
}

//========================================================================== 
// MENU - Unmark all cubes
//========================================================================== 
void CMine::UnmarkAll() {
	INT32 i; 
	CSegment *segP = Segments (0);
	for (i = 0; i < MAX_SEGMENTS; i++, segP++)
		segP->wallFlags &= ~MARKED_MASK; 
	UINT8& stat = VertStatus ();
	for (i = 0; i < MAX_VERTICES; i++, stat++)
		stat &= ~MARKED_MASK; 
	theApp.MineView ()->Refresh (); 
}

// -------------------------------------------------------------------------- 
// ResetSide()
//
// Action - sets side to have no child and a default texture
// -------------------------------------------------------------------------- 

void CMine::ResetSide (INT16 nSegment, INT16 nSide)
{
if (nSegment < 0 || nSegment >= SegCount ()) 
	return; 
theApp.SetModified (TRUE); 
theApp.LockUndo ();
CSegment *segP = Segments (0) + nSegment; 
segP->children [nSide] =-1; 
segP->childFlags &= ~(1 << nSide); 
CSide *sideP = segP->sides + nSide;
sideP->nBaseTex = 0; 
sideP->nOvlTex = 0; 
CUVL *uvls = sideP->uvls;
double scale = pTextures [m_fileType][sideP->nBaseTex].Scale (sideP->nBaseTex);
INT32 i;
for (i = 0; i < 4; i++, uvls++) {
	uvls->u = (INT16) (default_uvls [i].u / scale); 
	uvls->v = (INT16) (default_uvls [i].v / scale); 
	uvls->l = (UINT16) DEFAULT_LIGHTING; 
	}
theApp.UnlockUndo ();
}

// -------------------------------------------------------------------------- 
// unlink_child()
//
// Action - unlinks current cube's children which don't share all four points
//
// Note: 2nd parameter "nSide" is ignored
// -------------------------------------------------------------------------- 

void CMine::UnlinkChild (INT16 parent_segnum, INT16 nSide) 
{
  CSegment *parent_seg = Segments (parent_segnum); 

// loop on each side of the parent
//	INT32 nSide;
//  for (nSide = 0; nSide < 6; nSide++) {
INT32 child_segnum = parent_seg->children [nSide]; 
// does this side have a child?
if (child_segnum < 0 || child_segnum >= SegCount ())
	return;
CSegment *child_seg = Segments () + child_segnum; 
// yes, see if child has a side which points to the parent
INT32 child_sidenum;
for (child_sidenum = 0; child_sidenum < 6; child_sidenum++)
	if (child_seg->children [child_sidenum]== parent_segnum) break; 
// if we found the matching side
if (child_sidenum < 6) {
// define vert numbers for comparison
	INT16 pv [4], cv [4]; // (INT16 names given for clarity)
	INT32 i;
	for (i = 0; i < 4; i++) {
		pv [i] = parent_seg->verts [side_vert [nSide][i]]; // parent vert
		cv [i] = child_seg->verts [side_vert [child_sidenum][i]]; // child vert
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
		theApp.SetModified (TRUE); 
		theApp.LockUndo ();
		ResetSide (child_segnum, child_sidenum); 
		ResetSide (parent_segnum, nSide); 
		theApp.UnlockUndo ();
		}
	}
else {
	// if the child does not point to the parent, 
	// then just unlink the parent from the child
	ResetSide (parent_segnum, nSide); 
	}
}

// -------------------------------------------------------------------------- 

bool CMine::IsPointOfSide (CSegment *segP, INT32 nSide, INT32 pointnum)
{
	INT32	i;

for (i = 0; i < 4; i++)
	if (side_vert [nSide][i] == pointnum)
		return true;
return false;
}

// -------------------------------------------------------------------------- 

bool CMine::IsLineOfSide (CSegment *segP, INT32 nSide, INT32 linenum)
{
	INT32	i;

for (i = 0; i < 2; i++)
	if (!IsPointOfSide (segP, nSide, line_vert [linenum][i]))
		return false;
return true;
}

// -------------------------------------------------------------------------- 
//                          Splitpoints()
//
// Action - Splits one point shared between two cubes into two points.
//          New point is added to current cube, other cube is left alone.
//
// -------------------------------------------------------------------------- 

void CMine::SplitPoints () 
{
CSegment *segP; 
INT16 vert, nSegment, nVertex, nOppSeg, nOppSide; 
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
vert = segP->verts [side_vert [Current ()->nSide][Current ()->nPoint]]; 

// check to see if current point is shared by any other cubes
found = FALSE; 
segP = Segments ();
for (nSegment = 0; (nSegment < SegCount ()) && !found; nSegment++, segP++)
	if (nSegment != Current ()->nSegment)
		for (nVertex = 0; nVertex < 8; nVertex++)
			if (segP->verts [nVertex] == vert) {
				found = TRUE; 
				break; 
				}
if (!found) {
	ErrorMsg ("This point is not joined with any other point."); 
	return; 
	}

if (QueryMsg("Are you sure you want to unjoin this point?") != IDYES) 
	return; 

theApp.SetModified (TRUE); 
theApp.LockUndo ();
// create a new point (copy of other vertex)
memcpy (Vertices (VertCount ()), Vertices (vert), sizeof (*Vertices ()));
/*
Vertices (VertCount ()).x = Vertices (vert).x; 
Vertices (VertCount ()).y = Vertices (vert).y; 
Vertices (VertCount ()).z = Vertices (vert).z; 
*/
// replace existing point with new point
segP = Segments (Current ()->nSegment); 
segP->verts [side_vert [Current ()->nSide][Current ()->nPoint]] = VertCount (); 
segP->wallFlags &= ~MARKED_MASK; 

// update total number of vertices
VertStatus (VertCount ()++) = 0; 

INT32 nSide;
for (nSide = 0; nSide < 6; nSide++)
	if (IsPointOfSide (segP, nSide, segP->verts [side_vert [Current ()->nSide][Current ()->nPoint]]) &&
		 GetOppositeSide (nOppSeg, nOppSide, Current ()->nSegment, nSide)) {
		UnlinkChild (segP->children [nSide], opp_side [nSide]);
		UnlinkChild (Current ()->nSegment, nSide); 
		}

	UnlinkChild(Current ()->nSegment, nSide); 

SetLinesToDraw(); 
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
INFOMSG("A new point was made for the current point."); 
}

// -------------------------------------------------------------------------- 
//                         Splitlines()
//
// Action - Splits common lines of two cubes into two lines.
//
// -------------------------------------------------------------------------- 

void CMine::SplitLines() 
{
  CSegment *segP; 
  INT16 vert [2], nSegment, nVertex, linenum, nOppSeg, nOppSide, i; 
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
	linenum = side_line [Current ()->nSide][Current ()->nLine]; 
	vert [i] = Segments (Current ()->nSegment)->verts [line_vert [linenum][i]]; 
	// check to see if current points are shared by any other cubes
	found [i] = FALSE; 
	segP = Segments ();
	for (nSegment = 0; (nSegment < SegCount ()) && !found [i]; nSegment++, segP++) {
		if (nSegment != Current ()->nSegment) {
			for (nVertex = 0; nVertex < 8; nVertex++) {
				if (segP->verts [nVertex] == vert [i]) {
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
theApp.SetModified (TRUE); 
theApp.LockUndo ();
segP = Segments (Current ()->nSegment); 
// create a new points (copy of other vertices)
for (i = 0; i < 2; i++)
	if (found [i]) {
		memcpy (Vertices (VertCount ()), Vertices (vert [i]), sizeof (*Vertices ()));
		/*
		vertices [VertCount ()].x = vertices [vert [i]].x; 
		vertices [VertCount ()].y = vertices [vert [i]].y; 
		vertices [VertCount ()].z = vertices [vert [i]].z; 
		*/
		// replace existing points with new points
		linenum = side_line [Current ()->nSide][Current ()->nLine]; 
		segP->verts [line_vert [linenum][i]] = VertCount (); 
		segP->wallFlags &= ~MARKED_MASK; 
		// update total number of vertices
		VertStatus (VertCount ()++) = 0; 
		}
INT32 nSide;
for (nSide = 0; nSide < 6; nSide++) {
	if (IsLineOfSide (segP, nSide, linenum) && 
		 GetOppositeSide (nOppSeg, nOppSide, Current ()->nSegment, nSide)) {
		UnlinkChild (nOppSeg, nOppSide);
		UnlinkChild (Current ()->nSegment, nSide); 
		}
	}
SetLinesToDraw(); 
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
INFOMSG ("Two new points were made for the current line."); 
}

// -------------------------------------------------------------------------- 
//                       Splitsegments()
//
// ACTION - Splits a cube from all other points which share its coordinates
//
//  Changes - Added option to make thin side
// If solidify == 1, the side will keep any points it has in common with other
// sides, unless one or more of its vertices are already solitaire, in which
// case the side needs to get disconnected from its child anyway because that 
// constitutes an error in the level structure.
// -------------------------------------------------------------------------- 

void CMine::SplitSegments (INT32 solidify, INT32 nSide) 
{
  CSegment *segP; 
  INT32 vert [4], nSegment, nVertex, i, nFound = 0; 
  bool found [4]; 

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return; 
	}

segP = CurrSeg (); 
if (nSide < 0)
	nSide = Current ()->nSide;
INT32 child_segnum = segP->children [nSide]; 
if (child_segnum == -1) {
	ErrorMsg ("The current side is not connected to another cube"); 
	return; 
	}

for (i = 0; i < 4; i++)
	vert [i] = segP->verts [side_vert [nSide][i]]; 
	// check to see if current points are shared by any other cubes
for (nSegment = 0, segP = Segments (); nSegment < SegCount (); nSegment++, segP++)
	if (nSegment != Current ()->nSegment)
		for (i = 0, nFound = 0; i < 4; i++) {
			found [i] = FALSE;
			for (nVertex = 0; nVertex < 8; nVertex++)
				if (segP->verts [nVertex] == vert [i]) {
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

theApp.SetModified (TRUE); 
theApp.LockUndo ();
segP = Segments (Current ()->nSegment); 
if (nFound < 4)
	solidify = 0;
if (!solidify) {
	// create new points (copy of other vertices)
	for (i = 0; i < 4; i++) {
		if (found [i]) {
			memcpy (Vertices (VertCount ()), Vertices (vert [i]), sizeof (*Vertices ()));
			/*
			vertices [VertCount ()].x = vertices [vert [i]].x; 
			vertices [VertCount ()].y = vertices [vert [i]].y; 
			vertices [VertCount ()].z = vertices [vert [i]].z; 
			*/
			// replace existing points with new points
			segP->verts [side_vert [nSide][i]] = VertCount (); 
			segP->wallFlags &= ~MARKED_MASK; 

			// update total number of vertices
			VertStatus (VertCount ()++) = 0; 
			}
		}
	INT32 nSide;
	for (nSide = 0; nSide < 6; nSide++)
		if (nSide != opp_side [nSide])
			UnlinkChild (Current ()->nSegment, nSide); 
	SetLinesToDraw(); 
	INFOMSG (" Four new points were made for the current side."); 
	}
else {
	// does this side have a child?
	CSegment *child_seg = Segments (child_segnum); 
	// yes, see if child has a side which points to the parent
	INT32 child_sidenum;
	for (child_sidenum = 0; child_sidenum < 6; child_sidenum++)
		if (child_seg->children [child_sidenum]== Current ()->nSegment) 
			break; 
	// if we found the matching side
	if (child_sidenum < 6)
		ResetSide (child_segnum, child_sidenum); 
	ResetSide (Current ()->nSegment, Current ()->nSide); 
	SetLinesToDraw(); 
	}
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
}

// -------------------------------------------------------------------------- 
// Mine - Joinpoints
// -------------------------------------------------------------------------- 

void CMine::JoinPoints() 
{
  CSegment *seg1, *seg2; 
 double distance; //v1x, v1y, v1z, v2x, v2y, v2z; 
  INT32 vert1, vert2; 
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
	seg1 = Segments () + Current1 ().nSegment; 
	seg2 = Segments () + Current2 ().nSegment; 
	cur1 = &Current1 (); 
	cur2 = &Current2 (); 
	}
else {
	seg1 = Segments () + Current2 ().nSegment; 
	seg2 = Segments () + Current1 ().nSegment; 
	cur1 = &Current2 (); 
	cur2 = &Current1 (); 
	}
vert1 = seg1->verts [side_vert [cur1->nSide][cur1->nPoint]]; 
vert2 = seg2->verts [side_vert [cur2->nSide][cur2->nPoint]]; 
// make sure verts are different
if (vert1== vert2) {
	ErrorMsg ("These points are already joined."); 
	return; 
	}
// make sure there are distances are close enough
distance = CalcLength(Vertices (vert1), Vertices (vert2)); 
if (distance > JOIN_DISTANCE) {
	ErrorMsg ("Points are too far apart to join"); 
	return; 
	}
if (QueryMsg("Are you sure you want to join the current point\n"
				 "with the 'other' cube's current point?") != IDYES)
	return; 
theApp.SetModified (TRUE); 
theApp.LockUndo ();
// define vert numbers
seg1->verts [side_vert [cur1->nSide][cur1->nPoint]] = vert2; 
// delete any unused vertices
//  delete_unused_vertices(); 
FixChildren(); 
SetLinesToDraw(); 
theApp.MineView ()->Refresh ();
theApp.UnlockUndo ();
}

// -------------------------------------------------------------------------- 
// Mine - Joinlines
// -------------------------------------------------------------------------- 

void CMine::JoinLines() 
{
  CSegment *seg1, *seg2; 
  double v1x [2], v1y [2], v1z [2], v2x [2], v2y [2], v2z [2]; 
  double distance, min_radius; 
  INT32 v1, v2, vert1 [2], vert2 [2]; 
  INT16 match [2]; 
  INT16 i, j, linenum; 
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
	seg1 = Segments () + Current1 ().nSegment; 
	seg2 = Segments () + Current2 ().nSegment; 
	cur1 = &Current1 (); 
	cur2 = &Current2 (); 
	} 
else {
	seg1 = Segments () + Current2 ().nSegment; 
	seg2 = Segments () + Current1 ().nSegment; 
	cur1 = &Current2 (); 
	cur2 = &Current1 (); 
	}

for (i = 0; i < 2; i++) {
	linenum = side_line [cur1->nSide][cur1->nLine]; 
	v1 = vert1 [i] = seg1->verts [line_vert [linenum][i]]; 
	linenum = side_line [cur2->nSide][cur2->nLine]; 
	v2 = vert2 [i] = seg2->verts [line_vert [linenum][i]]; 
	v1x [i] = Vertices (v1)->x; 
	v1y [i] = Vertices (v1)->y; 
	v1z [i] = Vertices (v1)->z; 
	v2x [i] = Vertices (v2)->x; 
	v2y [i] = Vertices (v2)->y; 
	v2z [i] = Vertices (v2)->z; 
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
if (min_radius== JOIN_DISTANCE) {
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
theApp.SetModified (TRUE); 
theApp.LockUndo ();
// define vert numbers
for (i = 0; i < 2; i++) {
	linenum = side_line [cur1->nSide][cur1->nLine]; 
	seg1->verts [line_vert [linenum][i]] = vert2 [match [i]]; 
	}
FixChildren(); 
SetLinesToDraw(); 
theApp.MineView ()->Refresh ();
theApp.UnlockUndo ();
}


// -------------------------------------------------------------------------- 
//			  set_lines_to_draw()
//
//  ACTION - Determines which lines will be shown when drawing 3d image of
//           the theMine->  This helps speed up drawing by avoiding drawing lines
//           multiple times.
//
// -------------------------------------------------------------------------- 

void CMine::SetLinesToDraw()
{
  CSegment *segP; 
  INT16 nSegment, nSide; 

for (nSegment = SegCount (), segP = Segments (); nSegment; nSegment--, segP++) {
	segP->map_bitmask |= 0xFFF; 
	// if segment nSide has a child, clear bit for drawing line
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		if (segP->children [nSide] > -1) { // -1 = no child,  - 2 = outside of world
			segP->map_bitmask &= ~(1 << (side_line [nSide][0])); 
			segP->map_bitmask &= ~(1 << (side_line [nSide][1])); 
			segP->map_bitmask &= ~(1 << (side_line [nSide][2])); 
			segP->map_bitmask &= ~(1 << (side_line [nSide][3])); 
			}
		}
	}
}

// -------------------------------------------------------------------------- 
// FixChildren()
//
// Action - Updates linkage between current segment and all other Segments ()
// -------------------------------------------------------------------------- 

void CMine::FixChildren()
{
INT16 nNewSide, nSide, nSegment, nNewSeg; 

nNewSeg = Current ()->nSegment; 
nNewSide = Current ()->nSide; 
CSegment *pSeg = Segments (),
			*pNewSeg = Segments (nNewSeg);
CFixVector	*vSeg, 
			*vNewSeg = Vertices (pNewSeg->verts [0]);
for (nSegment = 0; nSegment < SegCount (); nSegment++, pSeg) {
	if (nSegment != nNewSeg) {
		// first check to see if Segments () are any where near each other
		// use x, y, and z coordinate of first point of each segment for comparison
		vSeg = Vertices (pSeg->verts [0]);
		if (fabs ((double) (vNewSeg->x - vSeg->x)) < 0xA00000L &&
		    fabs ((double) (vNewSeg->y - vSeg->y)) < 0xA00000L &&
		    fabs ((double) (vNewSeg->z - vSeg->z)) < 0xA00000L) {
			for (nSide = 0; nSide < 6; nSide++) {
				if (!LinkSegments(nNewSeg, nNewSide, nSegment, nSide, 3*F1_0)) {
					// if these Segments () were linked, then unlink them
					if (pNewSeg->children [nNewSide]== nSegment && pSeg->children [nSide]== nNewSeg) {
						pNewSeg->children [nNewSide] =-1; 
						pNewSeg->childFlags &= ~(1 << nNewSide); 
						pSeg->children [nSide] =-1; 
						pSeg->childFlags &= ~(1 << nSide); 
						}
					}
				}
			}
		}
	}
}

// -------------------------------------------------------------------------- 
//		       Joinsegments()
//
//  ACTION - Joins sides of current Segments ().  Finds closest corners.
//	     If sides use vertices with the same coordinates, these vertices
//	     are merged and the cube's are connected together.  Otherwise, a
//           new cube is added added.
//
//  Changes - Added option to solidifyally figure out "other cube"
// -------------------------------------------------------------------------- 

void CMine::JoinSegments(INT32 solidify)
{
	CSegment *segP; 
	CSegment *seg1, *seg2; 
	INT16 h, i, j, nSide, nNewSeg, nSegment; 
	CFixVector v1 [4], v2 [4]; 
	double radius, min_radius, max_radius, dx, dy, dz, totalRad, minTotalRad; 
	tVertMatch match [4]; 
	bool fail; 
	CSelection *cur1, *cur2, my_cube; 

if (m_bSplineActive) {
	ErrorMsg (spline_error_message); 
	return; 
	}

// figure out "other' cube
if (solidify) {
	if (Segments (Current ()->nSegment)->children [Current ()->nSide] != -1) {
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
#if 1
		memcpy (v1 + i, Vertices (seg1->verts [side_vert [cur1->nSide][i]]), sizeof (*Vertices ()));
#else
		INT32 nVertex = seg1->verts [side_vert [cur1->nSide][i]];
		v1 [i].x = Vertices (nVertex)->x; 
		v1 [i].y = Vertices (nVertex)->y; 
		v1 [i].z = Vertices (nVertex)->z; 
#endif
		}
	minTotalRad = 1e300;
	for (nSegment = 0, seg2 = Segments (); nSegment < SegCount (); nSegment++, seg2++) {
		if (nSegment== cur1->nSegment)
			continue; 
		for (nSide = 0; nSide < 6; nSide++) {
			fail = FALSE; 
			for (i = 0; i < 4; i++) {
#if 1
				memcpy (v2 + i, Vertices (seg2->verts[side_vert[nSide][i]]), sizeof (*Vertices ()));
#else
				INT32 nVertex = seg2->verts [side_vert [nSide][i]];
				v2 [i].x = Vertices (nVertex)->x; 
				v2 [i].y = Vertices (nVertex)->y; 
				v2 [i].z = Vertices (nVertex)->z; 
#endif
				}
			for (i = 0; i < 4; i++)
				match [i].b = 0; 
			for (i = 0; i < 4; i++) {
				match [i].i = -1; 
				match [i].d = 1e300;
				for (j = 0, h = -1; j < 4; j++) {
					if (match [j].b)
						continue;
					dx = (double) v1 [i].x - (double) v2 [j].x;
					dy = (double) v1 [i].y - (double) v2 [j].y;
					dz = (double) v1 [i].z - (double) v2 [j].z;
					radius = sqrt (dx * dx + dy * dy + dz * dz);
					if ((radius <= 10.0 * F1_0) && (radius < match [i].d)) {
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

#if 0
			if (fail)
				continue;
			for (i = 0; i < 4; i++)
				if (match [i].i == -1) {
					fail = TRUE; 
					break; 
					}
		// make sure there are matches for each and they are unique
		// Actually, if there's a match for each vertex, it must be unique here,
		// because if it wasn't, one entry in match[] must have been left out and thus be -1 [DM]
#	if 1
			for (i = 0; i < 3; i++)
				for (j = i + 1; j < 4; j++)
					if (match [i] == match [j]) {
						fail = TRUE;
						break;
						}
#	else
			if (match [0] == match [1]) fail = TRUE; 
			else if (match [0] == match [2]) fail = TRUE; 
			else if (match [0] == match [3]) fail = TRUE; 
			else if (match [1] == match [2]) fail = TRUE; 
			else if (match [1] == match [3]) fail = TRUE; 
			else if (match [2] == match [3]) fail = TRUE; 
#	endif
#endif
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
	memcpy (v1 + i, Vertices (seg1->verts [side_vert [cur1->nSide][i]]), sizeof (*Vertices ())); 
	memcpy (v2 + i, Vertices (seg2->verts [side_vert [cur2->nSide][i]]), sizeof (*Vertices ())); 
/*
	v1 [i].x = vertices [seg1->verts [side_vert [cur1->nSide][i]]].x; 
	v1 [i].y = vertices [seg1->verts [side_vert [cur1->nSide][i]]].y; 
	v1 [i].z = vertices [seg1->verts [side_vert [cur1->nSide][i]]].z; 
	v2 [i].x = vertices [seg2->verts [side_vert [cur2->nSide][i]]].x; 
	v2 [i].y = vertices [seg2->verts [side_vert [cur2->nSide][i]]].y; 
	v2 [i].z = vertices [seg2->verts [side_vert [cur2->nSide][i]]].z; 
*/
	match [i].i = -1; 
	}

// find closest for each point for each corner
for (i = 0; i < 4; i++) {
	min_radius = JOIN_DISTANCE; 
	for (j = 0; j < 4; j++) {
		dx = (double) v1 [i].x - (double) v2 [j].x;
		dy = (double) v1 [i].y - (double) v2 [j].y;
		dz = (double) v1 [i].z - (double) v2 [j].z;
		radius = sqrt(dx * dx + dy * dy + dz * dz);
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
	//    ErrorMsg ("Can't figure out how to attach these sides\n"
	//	     "because the closest point to each point\n"
	//	     "on the current side is not a unique point\n"
	//	     "on the other side."); 
	//    return; 
	// go method #2, use current points
	INT32 offset = (4 + cur1->nPoint - (3 - cur2->nPoint))%4; 
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
	radius = sqrt(((double)v1 [i].x - (double)v2 [j].x) * ((double)v1 [i].x - (double)v2 [j].x)
				 +  ((double)v1 [i].y - (double)v2 [j].y) * ((double)v1 [i].y - (double)v2 [j].y)
				 +  ((double)v1 [i].z - (double)v2 [j].z) * ((double)v1 [i].z - (double)v2 [j].z)); 
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
if (min_radius <= 5*F1_0) {
	theApp.SetModified (TRUE); 
	theApp.LockUndo ();
	LinkSides (cur1->nSegment, cur1->nSide, cur2->nSegment, cur2->nSide, match); 
	SetLinesToDraw(); 
	theApp.UnlockUndo ();
	theApp.MineView ()->Refresh ();
	return; 
	}

if (QueryMsg("Are you sure you want to create a new cube which\n"
				 "connects the current side with the 'other' side?\n\n"
				 "Hint: Make sure you have the current point of each cube\n"
				 "on the corners you to connected.\n"
				 "(the 'P' key selects the current point)") != IDYES)
	return; 

//  nNewSeg = first_free_segment(); 
//  if (nNewSeg== -1) {
nNewSeg = SegCount (); 
if (!(SegCount () < MAX_SEGMENTS)) {
	if (!bExpertMode)
		ErrorMsg ("The maximum number of Segments () has been reached.\n"
					"Cannot add any more Segments ()."); 
	return; 
	}
segP = Segments (nNewSeg); 

theApp.SetModified (TRUE); 
theApp.LockUndo ();
// define children and special child
// first clear all sides
segP->childFlags = 0; 
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  /* no remaining children */
	segP->children [i] =-1; 

// now define two sides:
// near side has opposite side number cube 1
segP->childFlags |= (1 << (opp_side [cur1->nSide])); 
segP->children [opp_side [cur1->nSide]] = cur1->nSegment; 
// far side has same side number as cube 1
segP->childFlags |= (1 << cur1->nSide); 
segP->children [cur1->nSide] = cur2->nSegment; 
segP->owner = -1;
segP->group = -1;
segP->function = 0; 
segP->nMatCen =-1; 
segP->value =-1; 

// define vert numbers
for (i = 0; i < 4; i++) {
	segP->verts [opp_side_vert [cur1->nSide][i]] = seg1->verts [side_vert [cur1->nSide][i]]; 
	segP->verts [side_vert [cur1->nSide][i]] = seg2->verts [side_vert [cur2->nSide][match [i].i]]; 
	}

// define Walls ()
segP->wallFlags = 0; // unmarked
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++)
	segP->sides [nSide].nWall = NO_WALL; 

// define sides
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	if (segP->children [nSide]==-1) {
		SetTexture (nNewSeg, nSide, seg1->sides [cur1->nSide].nBaseTex, seg1->sides [cur1->nSide].nOvlTex); 
//        for (i = 0; i < 4; i++) {
//	  segP->sides [nSide].uvls [i].u = seg1->sides [cur1->nSide].uvls [i].u; 
//	  segP->sides [nSide].uvls [i].v = seg1->sides [cur1->nSide].uvls [i].v; 
//	  segP->sides [nSide].uvls [i].l = seg1->sides [cur1->nSide].uvls [i].l; 
//        }
		SetUV (nNewSeg, nSide, 0, 0, 0); 
		}
	else {
		SetTexture (nNewSeg, nSide, 0, 0); 
		for (i = 0; i < 4; i++) {
			segP->sides [nSide].uvls [i].u = 0; 
			segP->sides [nSide].uvls [i].v = 0; 
			segP->sides [nSide].uvls [i].l = 0; 
			}
		}
	}

// define static light
segP->static_light = seg1->static_light; 

// update cur segment
seg1->children [cur1->nSide] = nNewSeg; 
seg1->childFlags |= (1 << cur1->nSide); 
SetTexture (cur1->nSegment, cur1->nSide, 0, 0); 
for (i = 0; i < 4; i++) {
	seg1->sides [cur1->nSide].uvls [i].u = 0; 
	seg1->sides [cur1->nSide].uvls [i].v = 0; 
	seg1->sides [cur1->nSide].uvls [i].l = 0; 
	}
seg2->children [cur2->nSide] = nNewSeg; 
seg2->childFlags |= (1 << cur2->nSide); 
SetTexture (cur2->nSegment, cur2->nSide, 0, 0); 
for (i = 0; i < 4; i++) {
	seg2->sides [cur2->nSide].uvls [i].u = 0; 
	seg2->sides [cur2->nSide].uvls [i].v = 0; 
	seg2->sides [cur2->nSide].uvls [i].l = 0; 
	}

// update number of Segments () and vertices
SegCount ()++; 
theApp.UnlockUndo ();
SetLinesToDraw(); 
theApp.MineView ()->Refresh ();
}

// ------------------------------------------------------------------------ 

void CMine::LoadSideTextures (INT16 nSegment, INT16 nSide)
{
GetCurrent (nSegment, nSide);
CSide	*sideP = Segments (nSegment)->sides + nSide;
pTextures [m_fileType][sideP->nBaseTex].Read (sideP->nBaseTex);
if ((sideP->nOvlTex & 0x3fff) > 0)
	pTextures [m_fileType][sideP->nOvlTex & 0x3fff].Read (sideP->nOvlTex & 0x3fff);
}

// ------------------------------------------------------------------------ 
// Mine - SetUV ()
// ------------------------------------------------------------------------ 

void CMine::SetUV (INT16 nSegment, INT16 nSide, INT16 x, INT16 y, double dummy)
{
	struct vector {
		double x, y, z; 
		}; 

	struct vector A [4], B [4], C [4], D [4], E [4]; 
	INT32 i, nVertex; 
	double angle; 

// for testing, x is used to tell how far to convert vector
// 0, 1, 2, 3 represent B, C, D, E coordinate transformations

// copy side's four points into A
INT32 h = sizeof (*Vertices ());
for (i = 0; i < 4; i++) {
	nVertex = Segments (nSegment)->verts [side_vert [nSide][i]]; 
	A [i].x = Vertices (nVertex)->x; 
	A [i].y = Vertices (nVertex)->y; 
	A [i].z = Vertices (nVertex)->z; 
	}

// subtract point 0 from all points in A to form B points
for (i = 0; i < 4; i++) {
	B [i].x = A [i].x - A [0].x; 
	B [i].y = A [i].y - A [0].y; 
	B [i].z = A [i].z - A [0].z; 
	}

// calculate angle to put point 1 in x - y plane by spinning on x - axis
// then rotate B points on x - axis to form C points.
// check to see if on x - axis already
angle = atan3(B [1].z, B [1].y); 
for (i = 0; i < 4; i++) {
	C [i].x = B [i].x; 
	C [i].y = B [i].y * cos(angle) + B [i].z * sin(angle); 
	C [i].z = -B [i].y * sin(angle) + B [i].z * cos(angle); 
	}

#if UV_DEBUG
if (abs((INT32)C [1].z) != 0) {
	sprintf_s (message, sizeof (message),  "SetUV: point 1 not in x/y plane\n(%f); angle = %f", (float)C [1].z, (float)angle); 
	DEBUGMSG (message); 
	}
#endif

// calculate angle to put point 1 on x axis by spinning on z - axis
// then rotate C points on z - axis to form D points
// check to see if on z - axis already
angle = atan3(C [1].y, C [1].x); 
for (i = 0; i < 4; i++) {
	D [i].x = C [i].x * cos(angle) + C [i].y * sin(angle); 
	D [i].y = -C [i].x * sin(angle) + C [i].y * cos(angle); 
	D [i].z = C [i].z; 
	}
#if UV_DEBUG
if (abs((INT32)D [1].y) != 0) {
	DEBUGMSG (" SetUV: Point 1 not in x axis"); 
	}
#endif

// calculate angle to put point 2 in x - y plane by spinning on x - axis
// the rotate D points on x - axis to form E points
// check to see if on x - axis already
angle = atan3(D [2].z, D [2].y); 
for (i = 0; i < 4; i++) {
	E [i].x = D [i].x; 
	E [i].y = D [i].y * cos(angle) + D [i].z * sin(angle); 
	E [i].z = -D [i].y * sin(angle) + D [i].z * cos(angle); 
	}

// now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
// set v to x axis and u to negative u axis to match default (u, v)
// (remember to scale by dividing by 640)
CSide *sideP = Segments (nSegment)->sides + nSide;
CUVL *uvls = sideP->uvls;
#if UV_DEBUG
switch (x) {
	case 0:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (B [i].x/640); 
			uvls [i].u = - (B [i].y/640); 
			}
		break; 
	case 1:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (C [i].x/640); 
			uvls [i].u = 0x400/10 - (C [i].y/640); 
			}
		break; 
	case 2:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (D [i].x/640); 
			uvls [i].u = 2*0x400/10 - (D [i].y/640); 
			}
		break; 
	case 3:
		for (i = 0; i < 4; i++) {
			uvls [i].v = (E [i].x/640); 
			uvls [i].u = 3*0x400/10 - (E [i].y/640); 
			}
	break; 
	}
#else
theApp.SetModified (TRUE); 
LoadSideTextures (nSegment, nSide);
double scale = 1.0; //pTextures [m_fileType][sideP->nBaseTex].Scale (sideP->nBaseTex);
for (i = 0; i < 4; i++, uvls++) {
	uvls->v = (INT16) ((y + (E [i].x / 640)) / scale); 
	uvls->u = (INT16) ((x - (E [i].y / 640)) / scale); 
	}
#endif
}

                        /* -------------------------- */

bool CMine::GotMarkedSides ()
{
INT32	nSegment, nSide; 

for (nSegment = 0; nSegment < SegCount (); nSegment++)
	for (nSide = 0; nSide < 6; nSide++)
		if (SideIsMarked (nSegment, nSide))
			return true;
return false; 
}

                        /* -------------------------- */

INT16 CMine::MarkedSegmentCount (bool bCheck)
{
	INT32	nSegment, nCount; 
	CSegment *segP = Segments (0);
for (nSegment = SegCount (), nCount = 0; nSegment; nSegment--, segP++)
	if (segP->wallFlags & MARKED_MASK)
		if (bCheck)
			return 1; 
		else
			++nCount; 
return nCount; 
}

                        /* -------------------------- */

INT32 CMine::IsWall (INT16 nSegment, INT16 nSide)
{
GetCurrent (nSegment, nSide); 
return (Segments (nSegment)->children [nSide]== -1) ||
		 (Segments (nSegment)->sides [nSide].nWall < GameInfo ().walls.count); 
}

                        /* -------------------------- */

INT32 CMine::ScrollSpeed (UINT16 texture, INT32 *x, INT32 *y)
{
if (IsD1File ())
	return 0;
*x = 0; 
*y = 0; 
switch (texture) {
	case 399: *x = - 2; break; 
	case 400: *y = - 8; break; 
	case 402: *x = - 4; break; 
	case 405: *y = - 2; break; 
	case 406: *y = - 4; break; 
	case 407: *y = - 2; break; 
	case 348: *x = - 2; *y = - 2; break; 
	case 349: *x = - 2; *y = - 2; break; 
	case 350: *x = + 2; *y = - 2; break; 
	case 401: *y = - 8; break; 
	case 408: *y = - 2; break; 
	default:
		return 0; 
	}
return 1; 
}

                        /* -------------------------- */

INT32 CMine::AlignTextures (INT16 start_segment, INT16 start_side, INT16 only_child, BOOL bAlign1st, BOOL bAlign2nd, char bAlignedSides)
{
	CSegment *segP = Segments (start_segment); 
	CSegment *childSeg; 
	CSide *sideP = segP->sides + start_side; 
	CSide *childSide; 

	INT32 return_code =-1; 
	INT16 i; 
	INT16 nSide, childnum, linenum; 
	INT16 point0, point1, vert0, vert1; 
	INT16 childs_side, childs_line; 
	INT16 childs_point0, childs_point1, childs_vert0, childs_vert1; 
	INT16 u0, v0; 
	double sangle, cangle; 
	double angle, length; 
	static INT32 side_child [6][4] = {
		{4, 3, 5, 1}, //{5, 1, 4, 3}, 
		{2, 4, 0, 5}, //{5, 0, 4, 2}, 
		{5, 3, 4, 1}, //{5, 3, 4, 1}, 
		{0, 4, 2, 5}, //{5, 0, 4, 2}, 
		{2, 3, 0, 1}, //{2, 3, 0, 1}, 
		{0, 3, 2, 1} //{2, 3, 0, 1}
		}; 

theApp.SetModified (TRUE);
theApp.LockUndo ();
for (linenum = 0; linenum < 4; linenum++) {
	// find vert numbers for the line's two end points
	point0 = line_vert [side_line [start_side][linenum]][0]; 
	point1 = line_vert [side_line [start_side][linenum]][1]; 
	vert0  = segP->verts [point0]; 
	vert1  = segP->verts [point1]; 
	// check child for this line
	if (start_segment == only_child) {
		nSide = start_side;
		childnum = start_segment;
		}
	else {
		nSide = side_child [start_side][linenum]; 
		childnum = segP->children [nSide]; 
		}
	childSeg = Segments (childnum); 
	if ((childnum < 0) || ((only_child != -1) && (childnum != only_child)))
		continue;
	// figure out which side of child shares two points w/ start_side
	for (childs_side = 0; childs_side < 6; childs_side++) {
		if ((start_segment == only_child) && (childs_side == start_side))
			continue;
		if (bAlignedSides & (1 << childs_side))
			continue;
		// ignore children of different textures (or no texture)
		if (!IsWall (childnum, childs_side))
			continue;
		if (childSeg->sides [childs_side].nBaseTex != sideP->nBaseTex)
			continue;
		for (childs_line = 0; childs_line < 4; childs_line++) {
			// find vert numbers for the line's two end points
			childs_point0 = line_vert [side_line [childs_side][childs_line]][0]; 
			childs_point1 = line_vert [side_line [childs_side][childs_line]][1]; 
			childs_vert0  = childSeg->verts [childs_point0]; 
			childs_vert1  = childSeg->verts [childs_point1]; 
			// if points of child's line== corresponding points of parent
			if (!((childs_vert0 == vert1 && childs_vert1 == vert0) ||
					(childs_vert0 == vert0 && childs_vert1 == vert1)))
				continue;
			// now we know the child's side & line which touches the parent
			// child:  childnum, childs_side, childs_line, childs_point0, childs_point1
			// parent: start_segment, start_side, linenum, point0, point1
			childSide = childSeg->sides + childs_side; 
			if (bAlign1st) {
				// now translate all the childs (u, v) coords so child_point1 is at zero
				u0 = childSide->uvls [(childs_line + 1)&3].u; 
				v0 = childSide->uvls [(childs_line + 1)&3].v; 
				for (i = 0; i < 4; i++) {
					childSide->uvls [i].u -= u0; 
					childSide->uvls [i].v -= v0; 
					}
				// find difference between parent point0 and child point1
				u0 = childSide->uvls [(childs_line + 1)&3].u - sideP->uvls [linenum].u; 
				v0 = childSide->uvls [(childs_line + 1)&3].v - sideP->uvls [linenum].v; 
				// find the angle formed by the two lines
				sangle = atan3(sideP->uvls [(linenum + 1)&3].v - sideP->uvls [linenum].v, 
				sideP->uvls [(linenum + 1)&3].u - sideP->uvls [linenum].u); 
				cangle = atan3(childSide->uvls [childs_line].v - childSide->uvls [(childs_line + 1)&3].v, 
									childSide->uvls [childs_line].u - childSide->uvls [(childs_line + 1)&3].u); 
				// now rotate childs (u, v) coords around child_point1 (cangle - sangle)
				for (i = 0; i < 4; i++) {
					angle = atan3(childSide->uvls [i].v, childSide->uvls [i].u); 
					length = sqrt((double)childSide->uvls [i].u*(double)childSide->uvls [i].u +
									  (double)childSide->uvls [i].v*(double)childSide->uvls [i].v); 
					angle -= (cangle - sangle); 
					childSide->uvls [i].u = (INT16)(length*cos(angle)); 
					childSide->uvls [i].v = (INT16)(length*sin(angle)); 
					}
				// now translate all the childs (u, v) coords to parent point0
				for (i = 0; i < 4; i++) {
					childSide->uvls [i].u -= u0; 
					childSide->uvls [i].v -= v0; 
					}
				// modulo points by 0x800 (== 64 pixels)
				u0 = childSide->uvls [0].u/0x800; 
				v0 = childSide->uvls [0].v/0x800; 
				for (i = 0; i < 4; i++) {
					childSide->uvls [i].u -= u0*0x800; 
					childSide->uvls [i].v -= v0*0x800; 
					}
				if (only_child != -1)
					return_code = childs_side; 
				}
			if (bAlign2nd && sideP->nOvlTex && childSide->nOvlTex) {
				INT32 r;
				switch (sideP->nOvlTex & 0xC000) {
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
				INT32 h = (INT32) (fabs (angle) * 180.0 / PI / 90 + 0.5); 
//				h +=(childs_line + linenum + 2) % 4; //(childs_line > linenum) ? childs_line - linenum : linenum - childs_line;
				h = (h + r) % 4;
				childSide->nOvlTex &= ~0xC000;
				switch (h) {
					case 0:
						break;
					case 1:
						childSide->nOvlTex |= 0xC000;
						break;
					case 2:
						childSide->nOvlTex |= 0x8000;
						break;
					case 3:
						childSide->nOvlTex |= 0x4000;
						break;
					}
				}
			break;
			}
		}
	}
theApp.UnlockUndo ();
return return_code; 
}

// --------------------------------------------------------------------------- 
// get_opposing_side()
//
// Action - figures out childs nSegment and side for a given side
// Returns - TRUE on success
// --------------------------------------------------------------------------- 

bool CMine::GetOppositeSide (INT16& nOppSeg, INT16& nOppSide, INT16 nSegment, INT16 nSide)
{
  INT16 childseg, childside; 

nOppSeg = 0; 
nOppSide = 0; 
GetCurrent (nSegment, nSide); 
if (nSegment < 0 || nSegment >= SegCount ())
	return false; 
if (nSide < 0 || nSide >= 6)
	return false; 
childseg =Segments (nSegment)->children [nSide]; 
if (childseg < 0 || childseg >= SegCount ())
	return false; 
for (childside = 0; childside < 6; childside++) {
	if (Segments () [childseg].children [childside]== nSegment) {
		nOppSeg = childseg; 
		nOppSide = childside; 
		return true; 
		}
	}
return false; 
}

                        /* -------------------------- */

CSide *CMine::OppSide () 
{
INT16 nOppSeg, nOppSide;
if (!GetOppositeSide (nOppSeg, nOppSide))
	return NULL;
return Segments (nOppSeg)->sides + nOppSide;
}

                        /* -------------------------- */

bool CMine::SetTexture (INT16 nSegment, INT16 nSide, INT16 nTexture, INT16 tmapnum2)
{
	bool bUndo, bChange = false;
	CDTexture pTx [2];

bUndo = theApp.SetModified (TRUE); 
theApp.LockUndo (); 
GetCurrent (nSegment, nSide); 
if (tmapnum2 == nTexture)
   tmapnum2 = 0; 
CSide *sideP = Segments (nSegment)->sides + nSide; 
if ((nTexture >= 0) && (nTexture != sideP->nBaseTex)) {
	sideP->nBaseTex = nTexture; 
	if (nTexture == (sideP->nOvlTex & 0x3fff)) {
		sideP->nOvlTex = 0; 
		}
	bChange = true; 
	}
if (tmapnum2 >= 0) {
	if (tmapnum2 == sideP->nBaseTex)
		tmapnum2 = 0; 
	if (tmapnum2) {
		sideP->nOvlTex &= ~(0x3fff);	//preserve light settings
		sideP->nOvlTex |= tmapnum2; 
		}
	else
		sideP->nOvlTex = 0; 
	bChange = true; 
	}
if (!bChange) {
	theApp.ResetModified (bUndo);
	return false;
	}
pTextures [m_fileType][sideP->nBaseTex].Read (sideP->nBaseTex);
pTextures [m_fileType][sideP->nOvlTex & 0x3fff].Read (sideP->nOvlTex & 0x3fff);
#if 0
if (((sideP->nOvlTex & 0x3fff) > 0) &&
    (pTextures [m_fileType][sideP->nBaseTex].m_size != 
	  pTextures [m_fileType][sideP->nOvlTex & 0x3fff].m_size))
	sideP->nOvlTex = 0;
#endif
if ((IsLight (sideP->nBaseTex)== -1) && (IsLight (sideP->nOvlTex & 0x3fff)== -1))
	DeleteFlickeringLight (nSegment, nSide); 
if (!WallClipFromTexture (nSegment, nSide))
	CheckForDoor (nSegment, nSide); 
theApp.UnlockUndo (); 
sprintf_s (message, sizeof (message), "side has textures %d, %d", sideP->nBaseTex & 0x3fff, sideP->nOvlTex & 0x3fff); 
INFOMSG (message); 
return true;
}

                        /* -------------------------- */

void CMine::RenumberBotGens () 
{
	INT32			i, nMatCens, value, nSegment; 
	CSegment	*segP; 

// number "matcen"
nMatCens = 0; 
for (i = 0; i < GameInfo ().botgen.count; i++) {
	nSegment = BotGens (i)->nSegment; 
	if (nSegment >= 0) {
		segP = Segments () + nSegment; 
		segP->value = i; 
		if (segP->function== SEGMENT_FUNC_ROBOTMAKER)
			segP->nMatCen = nMatCens++; 
		}
	}

// number "value"
value = 0; 
for (i = 0, segP = Segments (); i < SegCount (); i++, segP++)
	if (segP->function== SEGMENT_FUNC_NONE)
		segP->value = 0; 
	else
		segP->value = value++; 
}

                        /* -------------------------- */

void CMine::RenumberEquipGens () 
{
	INT32			i, nMatCens, value, nSegment; 
	CSegment	*segP; 

// number "matcen"
nMatCens = 0; 
for (i = 0; i < GameInfo ().equipgen.count; i++) {
	nSegment = EquipGens (i)->nSegment; 
	if (nSegment >= 0) {
		segP = Segments () + nSegment; 
		segP->value = i; 
		if (segP->function== SEGMENT_FUNC_EQUIPMAKER)
			segP->nMatCen = nMatCens++; 
		}
	}

// number "value"
value = 0; 
for (i = 0, segP = Segments (); i < SegCount (); i++, segP++)
	if (segP->function== SEGMENT_FUNC_NONE)
		segP->value = 0; 
	else
		segP->value = value++; 
}

                        /* -------------------------- */

void CMine::CopyOtherCube ()
{
	bool bUndo, bChange = false;

if (Current1 ().nSegment == Current2 ().nSegment)
	return; 
INT16 nSegment = Current ()->nSegment; 
CSegment *otherSeg = OtherSeg (); 
bUndo = theApp.SetModified (TRUE); 
theApp.LockUndo ();
INT32 nSide;
for (nSide = 0; nSide < 6; nSide++)
	if (SetTexture (nSegment, nSide, 
						 otherSeg->sides [nSide].nBaseTex, 
						 otherSeg->sides [nSide].nOvlTex))
		bChange = true;
if (!bChange)
	theApp.ResetModified (bUndo);
else {
	theApp.UnlockUndo ();
	theApp.MineView ()->Refresh (); 
	}
}

                        /* -------------------------- */

bool CMine::SplitSegment ()
{
	CSegment	*centerSegP = CurrSeg (), *segP, *childSegP;
	INT16			nCenterSeg = INT16 (centerSegP - Segments ());
	INT16			nSegment, childSegNum;
	INT16			nSide, oppSideNum, childSideNum;
	INT16			vertNum, nWall;
	CFixVector		segCenter, *v, *segVert, *centerSegVert;
	bool			bVertDone [8], bUndo;
	INT32			h, i, j, k;
	INT16			oppSides [6] = {2,3,0,1,5,4};

if (SegCount () >= MAX_SEGMENTS - 6) {
	ErrorMsg ("Cannot split this cube because\nthe maximum number of cubes would be exceeded."); 
	return false;
	}
bUndo = theApp.SetModified (TRUE); 
theApp.LockUndo ();
h = VertCount ();
#if 0
// isolate segment
for (nSide = 0; nSide < 6; nSide++) {
	if (segP->children [nSide] < 0)
		continue;
	for (vertNum = 0; vertNum < 4; vertNum++, h++)
		*Vertices (h) = *Vertices (segP->verts [side_vert [nSide][vertNum]]);
	}
VertCount () = h;
#endif
// compute segment center
MEMSET (&segCenter, 0, sizeof (segCenter));
for (i = 0; i < 8; i++) {
	v = Vertices (centerSegP->verts [i]);
	segCenter.x += v->x;
	segCenter.y += v->y;
	segCenter.z += v->z;
	}
segCenter.x /= 8;
segCenter.y /= 8;
segCenter.z /= 8;
// add center segment
// compute center segment vertices
MEMSET (bVertDone, 0, sizeof (bVertDone));
for (nSide = 0; nSide < 6; nSide++) {
	for (vertNum = 0; vertNum < 4; vertNum++) {
		j = side_vert [nSide][vertNum];
		if (bVertDone [j])
			continue;
		bVertDone [j] = true;
		centerSegVert = Vertices (centerSegP->verts [j]);
		segVert = Vertices (h + j);
		segVert->x = (centerSegVert->x + segCenter.x) / 2;
		segVert->y = (centerSegVert->y + segCenter.y) / 2;
		segVert->z = (centerSegVert->z + segCenter.z) / 2;
		//centerSegP->verts [j] = h + j;
		}
	}
VertCount () = h + 8;
#if 1
// create the surrounding segments
for (nSegment = SegCount (), nSide = 0; nSide < 6; nSegment++, nSide++) {
	segP = Segments (nSegment);
	oppSideNum = oppSides [nSide];
	for (vertNum = 0; vertNum < 4; vertNum++) {
		i = side_vert [nSide][vertNum];
		segP->verts [i] = centerSegP->verts [i];
#if 0
		j = side_vert [oppSideNum][vertNum];
		segP->verts [j] = h + i;
#else
		if ((nSide & 1) || (nSide >= 4)) {
			i = line_vert [side_line [nSide][0]][0];
			j = line_vert [side_line [oppSideNum][2]][0];
			segP->verts [j] = h + i;
			i = line_vert [side_line [nSide][0]][1];
			j = line_vert [side_line [oppSideNum][2]][1];
			segP->verts [j] = h + i;
			i = line_vert [side_line [nSide][2]][0];
			j = line_vert [side_line [oppSideNum][0]][0];
			segP->verts [j] = h + i;
			i = line_vert [side_line [nSide][2]][1];
			j = line_vert [side_line [oppSideNum][0]][1];
			segP->verts [j] = h + i;
			}
		else {
			i = line_vert [side_line [nSide][0]][0];
			j = line_vert [side_line [oppSideNum][2]][1];
			segP->verts [j] = h + i;
			i = line_vert [side_line [nSide][0]][1];
			j = line_vert [side_line [oppSideNum][2]][0];
			segP->verts [j] = h + i;
			i = line_vert [side_line [nSide][2]][0];
			j = line_vert [side_line [oppSideNum][0]][1];
			segP->verts [j] = h + i;
			i = line_vert [side_line [nSide][2]][1];
			j = line_vert [side_line [oppSideNum][0]][0];
			segP->verts [j] = h + i;
			}
#endif
		}
	InitSegment (nSegment);
	if ((segP->children [nSide] = centerSegP->children [nSide]) > -1) {
		segP->childFlags |= (1 << nSide);
		for (childSegP = Segments (segP->children [nSide]), childSideNum = 0;
			  childSideNum < 6; 
			  childSideNum++)
			if (childSegP->children [childSideNum] == nCenterSeg) {
				childSegP->children [childSideNum] = nSegment;
				break;
				}
			}
	segP->children [oppSideNum] = nCenterSeg;
	segP->childFlags |= (1 << oppSideNum);
	centerSegP->children [nSide] = nSegment;
	centerSegP->childFlags |= (1 << nSide);
	nWall = centerSegP->sides [nSide].nWall;
	segP->sides [nSide].nWall = nWall;
	if ((nWall >= 0) && (nWall != NO_WALL)) {
		Walls (nWall)->m_nSegment = nSegment;
		centerSegP->sides [nSide].nWall = NO_WALL;
		}
	}
// relocate center segment vertex indices
MEMSET (bVertDone, 0, sizeof (bVertDone));
for (nSide = 0; nSide < 6; nSide++) {
	for (vertNum = 0; vertNum < 4; vertNum++) {
		j = side_vert [nSide][vertNum];
		if (bVertDone [j])
			continue;
		bVertDone [j] = true;
		centerSegP->verts [j] = h + j;
		}
	}
// join adjacent sides of the segments surrounding the center segment
#if 1
for (nSegment = 0, segP = Segments (SegCount ()); nSegment < 5; nSegment++, segP++) {
	for (nSide = 0; nSide < 6; nSide++) {
		if (segP->children [nSide] >= 0)
			continue;
		for (childSegNum = nSegment + 1, childSegP = Segments (SegCount () + childSegNum); 
			  childSegNum < 6; 
			  childSegNum++, childSegP++) {
			for (childSideNum = 0; childSideNum < 6; childSideNum++) {
				if (childSegP->children [childSideNum] >= 0)
					continue;
				h = 0;
				for (i = 0; i < 4; i++) {
					k = segP->verts [side_vert [nSide][i]];
					for (j = 0; j < 4; j++) {
						if (k == childSegP->verts [side_vert [childSideNum][j]]) {
							h++;
							break;
							}
						}
					}
				if (h == 4) {
					segP->children [nSide] = SegCount () + childSegNum;
					segP->childFlags |= (1 << nSide);
					childSegP->children [childSideNum] = SegCount () + nSegment;
					childSegP->childFlags |= (1 << childSideNum);
					break;
					}
				}
			}
		}
	}
#endif
SegCount () += 6;
#endif
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return true;
}

//------------------------------------------------------------------------------

static UINT8 segFuncFromType [] = {
	SEGMENT_FUNC_NONE,
	SEGMENT_FUNC_FUELCEN,
	SEGMENT_FUNC_REPAIRCEN,
	SEGMENT_FUNC_CONTROLCEN,
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

static UINT8 segPropsFromType [] = {
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

void CSegment::Upgrade (void)
{
props = segPropsFromType [function];
function = segFuncFromType [function];
damage [0] =
damage [1] = 0;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

UINT8 CSegment::ReadWalls (FILE* fp, int nLevelVersion)
{
	UINT8 wallFlags = UINT8 (read_INT8 (fp));
	int	i;

for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	if (wallFlags &= (1 << i)) 
		sides [i].nWall = (nLevelVersion >= 13) ? read_INT16 (fp) : INT16 (read_INT8 (fp));
return wallFlags;
}

// ------------------------------------------------------------------------

void CSegment::ReadExtras (FILE* fp, int nLevelType, int nLevelVersion, bool bExtras)
{
if (bExtras) {
	function = read_INT8 (fp);
	nMatCen = read_INT8 (fp);
	value = read_INT8 (fp);
	read_INT8 (fp);
	}
else {
	function = 0;
	nMatCen = -1;
	value = 0;
	}
s2_flags = 0;  // d1 doesn't use this number, so zero it
if (nLevelType == 2) {
	props = read_INT8 (fp);
	if (nLevelVersion < 20)
		Upgrade ();
	else {
		damage [0] = read_INT16 (fp);
		damage [1] = read_INT16 (fp);
		}
	}
static_light = read_FIX (fp);
}

// ------------------------------------------------------------------------

INT32 CSegment::Read (FILE* fp, int nLevelType, int nLevelVersion)
{
	int	i;

if (nLevelType == 2) {
	owner = read_INT8 (fp);
	group = read_INT8 (fp);
	}
else {
	owner = -1;
	group = -1;
	}
// read in child mask (1 byte)
childFlags = UINT8 (read_INT8 (fp));

// read 0 to 6 children (0 to 12 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
	children [i] = (childFlags & (1 << i)) ? read_INT16 (fp) : -1;

// read vertex numbers (16 bytes)
for (int i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	verts [i] = read_INT16 (fp);

if (nLevelVersion == 0)
	ReadExtras (0, nLevelType, nLevelVersion, (childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

// read the wall bit mask
wallFlags = UINT8 (read_INT8 (fp));

// read in wall numbers (0 to 6 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	sides [i].nWall = (wallFlags & (1 << i)) 
							? (nLevelVersion < 13) 
								? UINT16 (read_INT8 (fp)) 
								: UINT16 (read_INT16 (fp)) 
							: NO_WALL;

// read in textures and uvls (0 to 60 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  
	sides [i].Read (fp, (children [i] == -1) || ((wallFlags & (1 << i)) != 0));
return 1;
}

// ------------------------------------------------------------------------

UINT8 CSegment::WriteWalls (FILE* fp, int nLevelVersion)
{
	UINT8 wallFlags = 0;
	int	i;

for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if(sides [i].nWall < theMine->GameInfo ().walls.count) 
		wallFlags |= (1 << i);
	}
write_INT8 (wallFlags, fp);

// write wall numbers
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if (wallFlags & (1 << i)) {
		if (nLevelVersion >= 13)
			write_INT16 (sides [i].nWall, fp);
		else
			write_INT8 (INT8 (sides [i].nWall), fp);
		}
	}
return wallFlags;
}

// ------------------------------------------------------------------------

void CSegment::WriteExtras (FILE* fp, int nLevelType, bool bExtras)
{
if (bExtras) {
	write_INT8 (function, fp);
	write_INT8 (nMatCen, fp);
	write_INT8 (value, fp);
	write_INT8 (s2_flags, fp);
	}
if (nLevelType == 2) {
	write_INT8 (props, fp);
	write_INT16 (damage [0], fp);
	write_INT16 (damage [1], fp);
	}
write_FIX (static_light, fp);
}

// ------------------------------------------------------------------------

void CSegment::Write (FILE* fp, int nLevelType, int nLevelVersion)
{
	int	i;

if (nLevelType == 2) {
	write_INT8 (owner, fp);
	write_INT8 (group, fp);
	}

#if 1
childFlags = 0;
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	if(children [i] != -1) {
		childFlags |= (1 << i);
		}
	}
if (nLevelType == 0) {
	if (function != 0) { // if this is a special cube
		childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
		}
	}
#endif
write_INT8 (childFlags, fp);

// write children numbers (0 to 6 bytes)
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	if (childFlags & (1 << i)) 
		write_INT16 (children [i], fp);

// write vertex numbers (16 bytes)
for (i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	write_INT16 (verts [i], fp);

// write special info (0 to 4 bytes)
if ((function == SEGMENT_FUNC_ROBOTMAKER) && (nMatCen == -1)) {
	function = SEGMENT_FUNC_NONE;
	value = 0;
	childFlags &= ~(1 << MAX_SIDES_PER_SEGMENT);
	}
if (nLevelType == 0)
	WriteExtras (fp, nLevelType, (childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

// calculate wall bit mask
UINT8 wallFlags = WriteWalls (fp, nLevelVersion);
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  
	if ((children [i] == -1) || (wallFlags & (1 << i))) 
		sides [i].Write (fp);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

INT32 CSide::Read (FILE* fp, bool bTextured)
{
if (bTextured) {
	nBaseTex = read_INT16 (fp);
	if (nBaseTex & 0x8000) {
		nBaseTex &= ~0x8000;
		nOvlTex = read_INT16 (fp);
		if ((nOvlTex & 0x1FFF) == 0)
			nOvlTex = 0;
		}
	else
		nOvlTex = 0;
	for (int i = 0; i < 4; i++)
		uvls [i].Read (fp);
	}
else {
	nBaseTex = 0;
	nOvlTex = 0;
	for (int i = 0; i < 4; i++)
		uvls [i].Clear ();
	}
return 1;
}

// ------------------------------------------------------------------------

void CSide::Write (FILE* fp)
{
if (nOvlTex == 0)
	write_INT16 (nBaseTex, fp);
else {
	write_INT16 (nBaseTex | 0x8000, fp);
	write_INT16 (nOvlTex, fp);
	}
for (int i = 0; i < 4; i++)
	uvls [i].Write (fp);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

INT32 CLightDeltaValue::Read (FILE *fp, INT32 version, bool bFlag)
{
m_nSegment = read_INT16 (fp);
m_nSide = INT16 (read_INT8 (fp));
for (int i = 0; i < 4; i++)
	vert_light [i] = read_INT8 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CLightDeltaValue::Write (FILE *fp, INT32 version, bool bFlag)
{
write_INT16 (m_nSegment, fp);
write_INT8 (INT8 (m_nSide), fp);
for (int i = 0; i < 4; i++)
	write_INT8 (vert_light [i], fp);
}

// ------------------------------------------------------------------------

INT32 CLightDeltaIndex::Read (FILE *fp, INT32 version, bool bD2X)
{
m_nSegment = read_INT16 (fp);
UINT16 h = read_INT16 (fp);
if (bD2X) {
	m_nSide = h & 3;
	count = h >> 3;
	}
else {
	m_nSide = h % 256;
	count = h / 256;
	}
index = read_INT16 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CLightDeltaIndex::Write (FILE *fp, INT32 version, bool bD2X)
{
write_INT16 (m_nSegment, fp);
if (bD2X)
	write_INT16 (bD2X ? (m_nSide & 3) | (count << 3) : (m_nSide % 256 + count * 256), fp);
write_INT16 (index, fp);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

INT32 CRobotMaker::Read (FILE *fp, INT32 version, bool bFlag)
{
objFlags [0] = read_INT32 (fp);
if (theApp.IsD2File ())
	objFlags [1] = read_INT32 (fp);
hitPoints = read_FIX (fp);
interval = read_FIX (fp);
nSegment = read_INT16 (fp);
nFuelCen = read_INT16 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CRobotMaker::Write (FILE *fp, INT32 version, bool bFlag)
{
write_INT32 (objFlags [0], fp);
if (theApp.IsD2File ())
	write_INT32 (objFlags [1], fp);
write_FIX (hitPoints, fp);
write_FIX (interval, fp);
write_INT16 (nSegment, fp);
write_INT16 (nFuelCen, fp);
}

// ------------------------------------------------------------------------
//eof segment.cpp