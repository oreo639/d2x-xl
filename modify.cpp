// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include "dle-xp-res.h"

#include <math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "io.h"
#include "textures.h"
#include "palette.h"
#include "dle-xp.h"

#define CURRENT_POINT(a) ((Current ()->nPoint + (a))&0x03)

//This dialog is always active,
//but is hidden or restored by user
//::ShowWindow(hwnd,SW_HIDE);
//::ShowWindow(hwnd,SW_RESTORE);

bool CMine::EditGeoFwd (void)
{
  double				radius;
  CFixVector		center, opp_center;
  CDoubleVector	v;
  INT32				i;
/* calculate center of current side */

 for (i = 0; i < 4; i++) {
	 INT32 nVertex = Segments (Current ()->nSegment)->verts [side_vert [Current ()->nSide][i]];
   center += *Vertices (nVertex);
 }
center /= FIX (4);

// calculate center of opposite of current side
for (i = 0; i < 4; i++) {
	INT32 nVertex = Segments (Current ()->nSegment)->verts [opp_side_vert [Current ()->nSide][i]];
   opp_center += *Vertices (nVertex);
	}
opp_center /= FIX (4);
center -= opp_center;
// normalize vector
v = CDoubleVector (center);

// normalize direction
radius = v.Mag ();

if (radius > (F1_0 / 10)) {
	v /= radius;
	}
else {
	CFixVector direction;
	direction = CalcSideNormal (Current ()->nSegment, Current ()->nSide);
	v = CDoubleVector (direction) / (double) F1_0;
	}

// move on x, y, and z
 theApp.SetModified (TRUE);
 theApp.LockUndo ();
 MoveOn('X', (INT32) (v.v.x * move_rate));
 MoveOn('Y', (INT32) (v.v.y * move_rate));
 MoveOn('Z', (INT32) (v.v.z * move_rate));
 theApp.UnlockUndo ();
 return true;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoBack (void) 
{
  CFixVector		center, opp_center;
  CDoubleVector	v;
  double				radius;
  INT32				i;

/* calculate center of current side */
for (i = 0; i < 4; i++) {
	INT32 nVertex = Segments (Current ()->nSegment)->verts [side_vert [Current ()->nSide][i]];
	center += *Vertices (nVertex);
	}
center >>= 2;

// calculate center of oppisite current side
for (i = 0; i < 4; i++) {
	INT32 nVertex = Segments (Current ()->nSegment)->verts [opp_side_vert [Current ()->nSide][i]];
	opp_center += *Vertices (nVertex);
	}
opp_center >>= 2;

// normalize vector
v = CDoubleVecto (center - opp_center);

// make sure distance is positive to prevent
// cube from turning inside out
// defines line orthogonal to a side at a point
	UINT8 orthog_line [6][4] = {
		{8,6,1,3},
		{0,5,7,2},
		{3,1,6,8},
		{2,7,5,0},
		{4,9,10,11},
		{11,10,9,4}
		};
	CSegment *segP;
	INT16 point0,point1;
	CFixVector* vector0,*vector1;
	bool ok_to_move;

ok_to_move = TRUE;
segP = Segments (0) + Current ()->nSegment;
switch (m_selectMode) {
	case POINT_MODE:
		point0 = line_vert [orthog_line [Current ()->nSide][Current ()->nPoint]][0];
		point1 = line_vert [orthog_line [Current ()->nSide][Current ()->nPoint]][1];
		vector0 = Vertices (segP->verts [point0]);
		vector1 = Vertices (segP->verts [point1]);
		if (CalcLength(vector0,vector1) - move_rate < F1_0 / 4) {
		ok_to_move = FALSE;
		}
		break;

	case LINE_MODE:
		for (i=0;i<2;i++) {
			point0 = line_vert [orthog_line [Current ()->nSide][(Current ()->nLine+i)%4]][0];
			point1 = line_vert [orthog_line [Current ()->nSide][(Current ()->nLine+i)%4]][1];
			vector0 = Vertices (segP->verts [point0]);
			vector1 = Vertices (segP->verts [point1]);
			if (CalcLength(vector0,vector1) - move_rate < F1_0 / 4) {
			ok_to_move = FALSE;
			}
		}
	break;

	case SIDE_MODE:
		for (i = 0; i < 4; i++) {
			point0 = line_vert [orthog_line [Current ()->nSide][i]][0];
			point1 = line_vert [orthog_line [Current ()->nSide][i]][1];
			vector0 = Vertices (segP->verts [point0]);
			vector1 = Vertices (segP->verts [point1]);
			if (CalcLength(vector0,vector1) - move_rate < F1_0 / 4) {
			ok_to_move = FALSE;
			}
		}
		break;
	}
if (!ok_to_move) {
	ErrorMsg ("Too small to move in that direction");
	return false;
	}

radius = sqrt(x*x + y*y + z*z);
if ((radius-move_rate) < F1_0 / 4) {
	if (m_selectMode == POINT_MODE || m_selectMode == LINE_MODE || m_selectMode == SIDE_MODE) {
		ErrorMsg ("Cannot make cube any smaller\n"
		"Cube must be greater or equal to 1.0 units wide.");
		return false;
		}
	}
else {
	// normalize direction
	if (radius > (F1_0/10)) {
		v /= radius;
		} 
	else {
		CFixVector direction;
		direction = CalcSideNormal(Current ()->nSegment,Current ()->nSide);
		v = CDoubleVector (direction);
		v /= (double)F1_0;
		}
	// move on x, y, and z
	theApp.SetModified (TRUE);
	theApp.LockUndo ();
	MoveOn('X',(INT32) (-v.v.x * move_rate));
	MoveOn('Y',(INT32) (-v.v.y * move_rate));
	MoveOn('Z',(INT32) (-v.v.z * move_rate));
	theApp.UnlockUndo ();
	}
theApp.SetModified (TRUE);
return true;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoRotRight (void)
{
return SpinSelection (angle_rate);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoRotLeft (void)
{
return SpinSelection (-angle_rate);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoUp (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (angle_rate,FALSE) : MovePoints (1,0);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoDown (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (-angle_rate, FALSE) : MovePoints (0,1);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoRight (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection(angle_rate,TRUE) : MovePoints(3,0);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoLeft (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection(-angle_rate,TRUE) : MovePoints(0,3);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
bool CMine::EditGeoGrow (void) 
{
return SizeItem (move_rate);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
bool CMine::EditGeoShrink (void) 
{
return SizeItem (-move_rate);
}

//------------------------------------------------------------------------
//                    RotateSelection()
//
// ACTION - rotates a side about the opposite side.  The line is drawn
//          between the center points of lines 0 and 2.  If perpendicular
//          is TRUE, then the lines 1 and 3 are used instead.
//------------------------------------------------------------------------

bool CMine::RotateSelection (double angle, bool perpendicular) 
{
INT32 nSegment = Current ()->nSegment;
INT32 nSide = Current ()->nSide;
CSegment *segP = Segments (nSegment);
CFixVector center,opp_center;
INT32 i,pts [4];

switch (m_selectMode){
	case POINT_MODE:
		ErrorMsg ("Cannot bend a point");
		return false;

	case LINE_MODE:
		ErrorMsg ("Cannot bend a line");
		return false;

	case SIDE_MODE:	// spin side around the opposite side
		theApp.SetModified (TRUE);
		theApp.LockUndo ();
		if (perpendicular) { // use lines 0 and 2
			pts [0] = 1;
			pts [1] = 2;
			pts [2] = 3;
			pts [3] = 0;
			} 
		else {             // use lines 1 and 3
			pts [0] = 0;
			pts [1] = 1;
			pts [2] = 2;
			pts [3] = 3;
			}
		// calculate center opp side line 0
		opp_center = Average (Vertices (segP->verts [opp_side_vert [nSide][pts [0]]]),
									 Vertices (segP->verts [opp_side_vert [nSide][pts [1]]]));
		// calculate center opp side line 2
		center = Average (Vertices (segP->verts [opp_side_vert [nSide][pts [2]]]),
								Vertices (segP->verts [opp_side_vert [nSide][pts [3]]]));
		// rotate points around a line
		for (i = 0; i < 4; i++)
			RotateVertex (Vertices (segP->verts [side_vert [nSide][i]]),
							  &center, &opp_center, angle);
		theApp.UnlockUndo ();	
		break;
	
	case CUBE_MODE:
		ErrorMsg ("Cannot bend a cube");
		return false;
	
	case OBJECT_MODE:
		ErrorMsg ("Cannot bend a object");
		return false;

	case BLOCK_MODE:
		ErrorMsg ("Cannot bend a block");
		return false;
	}
return true;
}

//***************************************************************************
//			SizeItem()
//
// need to prevent reduction through zero
// absolute value of shorts line to size must be greater
// then incremental value if inc is negetive
//
//***************************************************************************

bool CMine::SizeItem (INT32 inc) 
{
	INT32 nSegment = Current ()->nSegment;
	INT32 nSide = Current ()->nSide;
	CSegment *segP = Segments (nSegment);
	INT32 i, j, point [4];
	bool result = false;

switch (m_selectMode) {
	case POINT_MODE:
		return false;

	case LINE_MODE:
		point [0] = line_vert [side_line [Current ()->nSide][Current ()->nLine]][0];
		point [1] = line_vert [side_line [Current ()->nSide][Current ()->nLine]][1];
		return SizeLine (segP,point [0],point [1],inc);

	case SIDE_MODE:
		theApp.SetModified (TRUE);
		theApp.LockUndo ();
		for (i = 0; i < 4; i++)
			point [i] = side_vert [Current ()->nSide][i];
		// enlarge the diagonals
		result = SizeLine(segP,point [0],point [2],(INT32) (inc*sqrt(2.0))) &&
				   SizeLine(segP,point [1],point [3],(INT32) (inc*sqrt(2.0)));
		theApp.UnlockUndo ();
		return result;

	case CUBE_MODE:
		// enlarge the diagonals
		theApp.SetModified (TRUE);
		theApp.LockUndo ();
		result = SizeLine(segP,0,6,(INT32) (inc*sqrt(3.0))) &&
				   SizeLine(segP,1,7,(INT32) (inc*sqrt(3.0))) &&
					SizeLine(segP,2,4,(INT32) (inc*sqrt(3.0))) &&
					SizeLine(segP,3,5,(INT32) (inc*sqrt(3.0)));
		theApp.UnlockUndo ();
		return result;

	case OBJECT_MODE:
		return false;

	case BLOCK_MODE:
		theApp.SetModified (TRUE);
		CFixVector max_pt, min_pt, center;
		CVertex* verts;
		max_pt.Set (-0x7fffffffL, -0x7fffffffL, -0x7fffffffL);
		min_pt.Set (0x7fffffffL, 0x7fffffffL, 0x7fffffffL);
		verts = Vertices (0);
		j = 0;
		for (i = VertCount (), j = 0; j < i; j++, verts++)
			if (verts->m_status & MARKED_MASK) {
				max_pt = Max (max_pt, *verts);
				min_pt = Min (min_pt, *verts);
				}
		center = Average (max_pt + min_pt);
		double scale = ((double)(20*F1_0) + (double)inc)/(double)(20*F1_0);
		verts = Vertices (0);
		for (i = VertCount (), j = 0; j < i; j++, verts++)
			if (verts->m_status & MARKED_MASK) {
				*verts -= center;
				*verts *= scale;
				*verts += center;
				}
	return true;
	}
return false;
}

//--------------------------------------------------------------------------
// MovePoints()
//
// moves blocks, sides, cubes, lines, and points in the direction
// of the current line.
//--------------------------------------------------------------------------

bool CMine::MovePoints(INT32 pt0, INT32 pt1) 
{
	CFixVector* vector0, * vector1, delta;
	INT32			point0, point1;
	double		length;
	INT32			point;
	INT32			i;
	CVertex*		vertP;

point0  = side_vert [Current ()->nSide][CURRENT_POINT(pt0)];
point1  = side_vert [Current ()->nSide][CURRENT_POINT(pt1)];
vector0 = Vertices (Segments (Current ()->nSegment)->verts [point0]);
vector1 = Vertices (Segments (Current ()->nSegment)->verts [point1]);
length  = CalcLength(vector0,vector1);
if (length >= F1_0) {
	delta = *vector1 - *vector0;
	CDoubleVector d (delta);
	d *= (double)move_rate / length;
	delta = CFixVector (d);
	} 
else {
	delta.Set (move_rate, 0, 0);
	}

switch (m_selectMode){
	case POINT_MODE:
		point = side_vert [Current ()->nSide][CURRENT_POINT(0)];
		*Vertices (Segments (Current ()->nSegment)->verts [point]) += delta;
		theApp.SetModified (TRUE);
		break;

	case LINE_MODE:
		point = side_vert [Current ()->nSide][CURRENT_POINT(0)];
		*Vertices (Segments (Current ()->nSegment)->verts [point]) += delta;
		point = side_vert [Current ()->nSide][CURRENT_POINT(1)];
		*Vertices (Segments (Current ()->nSegment)->verts [point]) += delta;
		theApp.SetModified (TRUE);
		break;

	case SIDE_MODE:
		for (i = 0; i < 4; i++) {
			point = side_vert [Current ()->nSide][i];
			*Vertices (Segments (Current ()->nSegment)->verts [point]) += delta;
			}
		theApp.SetModified (TRUE);
		break;

	case CUBE_MODE:
		for (i = 0; i < 8; i++) {
			*Vertices (Segments (Current ()->nSegment)->verts [i]) += delta;
			}
		theApp.SetModified (TRUE);
		break;

	case OBJECT_MODE:
		CurrObj ()->pos += delta;
		theApp.SetModified (TRUE);
		break;

	case BLOCK_MODE:
		bool bMoved = false;
		for (i = 0; i < MAX_VERTICES; i++) {
			if (VertStatus (i) & MARKED_MASK) {
				*Vertices (i) += delta;
				bMoved = true;
				}
			}
		theApp.SetModified (bMoved);
		break;
	}
return true;
}

//--------------------------------------------------------------------------
//     				SizeLine()
//
// prevent lines from being bigger than 8*20 and less than 3
//--------------------------------------------------------------------------

bool CMine::SizeLine (CSegment *segP,INT32 point0,INT32 point1,INT32 inc) 
{

// round a bit
if (inc > 0)
	if (inc & 0xf)
		inc++;
	else if (inc & 1)
		inc--;
else
	if (inc & 0xf)
		inc--;
	else if (inc & 1)
		inc++;

CFixVector* v1 = Vertices (segP->verts [point0]),
			  *v2 = Vertices (segP->verts [point1]);
double			radius;
CDoubleVector	v (v1 - v2);
// figure out direction to modify line
// normalize direction
radius = v.Mag ();
if (radius > (double) F1_0* (double) F1_0 - inc) 
	return false;
if ((inc < 0) && (radius <= (double)-inc*3)) 
	return false;
if (radius == 0) 
	return false;
v /= radius;
// multiply by increment value
v *= inc;
// update vertices
v1 += CFixVector (v);
v2 -= CFixVector (v);
return true;
}

/***************************************************************************
				MoveOn()
***************************************************************************/

bool CMine::MoveOn (char axis,INT32 inc) 
{
INT32 nSegment = Current ()->nSegment;
INT32 nSide = Current ()->nSide;
INT32 nPoint = Current ()->nPoint;
INT32 nLine = Current ()->nLine;
CSegment *segP = Segments (nSegment);
INT16 i;

theApp.SetModified (TRUE);
switch (m_selectMode) {
	case POINT_MODE:
		switch (axis) {
			case 'X':
				Vertices (segP->verts [side_vert [nSide][nPoint]])->x += inc;
				break;
			case 'Y':
				Vertices (segP->verts [side_vert [nSide][nPoint]])->y += inc;
				break;
			case 'Z':
				Vertices (segP->verts [side_vert [nSide][nPoint]])->z += inc;
				break;
			}
		break;

	case LINE_MODE:
		switch (axis) {
			case 'X':
				Vertices (segP->verts [line_vert [side_line [nSide][nLine]][0]])->x += inc;
				Vertices (segP->verts [line_vert [side_line [nSide][nLine]][1]])->x += inc;
				break;
			case 'Y':
				Vertices (segP->verts [line_vert [side_line [nSide][nLine]][0]])->y += inc;
				Vertices (segP->verts [line_vert [side_line [nSide][nLine]][1]])->y += inc;
				break;
			case 'Z':
				Vertices (segP->verts [line_vert [side_line [nSide][nLine]][0]])->z += inc;
				Vertices (segP->verts [line_vert [side_line [nSide][nLine]][1]])->z += inc;
				break;
			}
		break;

	case SIDE_MODE:
		switch (axis) {
			case 'X':
			for (i = 0; i < 4; i++)
				Vertices (segP->verts [side_vert [nSide][i]])->x += inc;
			break;
		case 'Y':
			for (i = 0; i < 4; i++)
				Vertices (segP->verts [side_vert [nSide][i]])->y += inc;
			break;
		case 'Z':
			for (i = 0; i < 4; i++)
				Vertices (segP->verts [side_vert [nSide][i]])->z += inc;
			break;
		}
		break;

	case CUBE_MODE:
		switch (axis) {
			case 'X':
				for (i = 0; i < 8; i++)
					Vertices (segP->verts [i])->x += inc;
				for (i = 0; i < GameInfo ().objects.count; i++)
					if (Objects (i)->nSegment == nSegment)
						Objects (i)->pos.x += inc;
				break;
			case 'Y':
				for (i = 0; i < 8; i++)
					Vertices (segP->verts [i])->y += inc;
				for (i = 0; i < GameInfo ().objects.count; i++) 
					if (Objects (i)->nSegment == nSegment)
						Objects (i)->pos.y += inc;
				break;
			case 'Z':
				for (i = 0; i < 8; i++)
					Vertices (segP->verts [i])->z += inc;
				for (i = 0; i < GameInfo ().objects.count; i++) 
					if (Objects (i)->nSegment == nSegment) 
						Objects (i)->pos.z += inc;
				break;
			}
	break;

	case OBJECT_MODE:
		switch (axis) {
			case 'X':
				CurrObj ()->pos.x += inc;
				break;
			case 'Y':
				CurrObj ()->pos.y += inc;
				break;
			case 'Z':
				CurrObj ()->pos.z += inc;
				break;
		}
	break;

	case BLOCK_MODE:
		CGameObject *objP = Objects (0);
		switch (axis) {
			case 'X':
				for (i = 0; i < MAX_VERTICES; i++)
					if (VertStatus (i) & MARKED_MASK)
						Vertices (i)->x += inc;
				for (i = GameInfo ().objects.count; i; i--, objP++)
					if (objP->nSegment >= 0)
						if (Segments (objP->nSegment)->wallFlags & MARKED_MASK)
							objP->pos.x += inc;
				break;
			case 'Y':
				for (i = 0; i < MAX_VERTICES; i++)
					if (VertStatus (i) & MARKED_MASK)
						Vertices (i)->y += inc;
				for (i = GameInfo ().objects.count; i; i--, objP++)
					if (objP->nSegment >= 0)
						if (Segments (objP->nSegment)->wallFlags & MARKED_MASK)
							objP->pos.y += inc;
				break;
			case 'Z':
				for (i = 0; i < MAX_VERTICES; i++)
					if (VertStatus (i) & MARKED_MASK)
						Vertices (i)->z += inc;
				for (i = GameInfo ().objects.count; i; i--, objP++)
					if (objP->nSegment >= 0)
						if (Segments (objP->nSegment)->wallFlags & MARKED_MASK)
							objP->pos.z += inc;
				break;
		}
	break;
	}
return true;
}

/***************************************************************************
			    SpinSelection()

  ACTION - Spins a side, cube, or object the amount specified.


***************************************************************************/

bool CMine::SpinSelection (double angle) 
{
	INT32 nSegment = Current ()->nSegment;
	INT32 nSide = Current ()->nSide;
	CSegment *segP = Segments (nSegment);
	CGameObject *objP;
	CFixVector center,opp_center;
	INT16 i;

#ifdef SPIN_RELATIVE
	double xspin,yspin,zspin;
	CFixVector rel [3];
#endif

/* calculate segment pointer */
switch (m_selectMode) {
	case POINT_MODE:
		ErrorMsg ("Cannot spin a point");
		return false;
	
	case LINE_MODE:
		ErrorMsg ("Cannot spin a line");
		return false;
	
	case SIDE_MODE: // spin side around its center in the plane of the side
		// calculate center of current side
		theApp.SetModified (TRUE);
		center.Clear ();
		for (i = 0; i < 4; i++) {
			center += *Vertices (segP->verts [side_vert [nSide][i]]);
			}
		center.x >>= 2;
		// calculate orthogonal vector from lines which intersect point 0
		//       |x  y  z |
		// AxB = |ax ay az| = x(aybz-azby), y(azbx-axbz), z(axby-aybx)
		//       |bx by bz|
		n = CalcSideNormal ();
		// normalize the vector
		// set sign (since vert numbers for most sides don't follow right-handed convention)
		if (nSide!=1 && nSide!=5) {
			n = -n;
			}
		// set opposite center
		opp_center = center + CFixVector (n * F1_0);
		/* rotate points around a line */
		for (i = 0; i < 4; i++)
			RotateVertex(Vertices (segP->verts [side_vert [nSide][i]]), &center,&opp_center,angle);
		break;


	case CUBE_MODE:	// spin cube around the center of the cube using screen's perspective
		// calculate center of current cube
		theApp.SetModified (TRUE);
		center.Clear ();
		for (i = 0; i < 8; i++) {
			center += *Vertices (segP->verts [i]);
			}
		center >>= 3;
		// calculate center of oppisite current side
		opp_center.Clear ();
		for (i = 0; i < 4; i++) {
			opp_center += *Vertices (segP->verts [opp_side_vert [nSide][i]]);
			}
		opp_center >>= 2;
		// rotate points about a point
		for (i = 0; i < 8; i++)
			RotateVertex(Vertices (segP->verts [i]),&center,&opp_center,angle);
		break;

	case OBJECT_MODE:	// spin object vector
		theApp.SetModified (TRUE);
		CFixMatrix *orient;
		orient = (Current ()->nObject == GameInfo ().objects.count) ? &SecretOrient () : &CurrObj ()->orient;
		switch (nSide) {
			case 0:
				RotateVmsMatrix(orient,angle,'x');
				break;
			case 2:
				RotateVmsMatrix(orient,-angle,'x');
				break;
			case 1:
				RotateVmsMatrix(orient,-angle,'y');
				break;
			case 3:
				RotateVmsMatrix(orient,angle,'y');
				break;
			case 4:
				RotateVmsMatrix(orient,angle,'z');
				break;
			case 5:
				RotateVmsMatrix(orient,-angle,'z');
				break;
			}
#ifdef SPIN_RELATIVE
		// calculate angles to spin the side into the x-y plane
		// use points 0,1, and 2 of the side
		// make point0 the origin
		// and get coordinates of points 1 and 2 relative to point 0
		for (i=0;i<3;i++) {
			rel [i].x = vertices [segP->verts [side_vert [nSide][i]]].x - vertices [segP->verts [side_vert [nSide][0]]].x;
			rel [i].y = vertices [segP->verts [side_vert [nSide][i]]].y - vertices [segP->verts [side_vert [nSide][0]]].y;
			rel [i].z = vertices [segP->verts [side_vert [nSide][i]]].z - vertices [segP->verts [side_vert [nSide][0]]].z;
			}
		// calculate z-axis spin angle to rotate point1 so it lies in x-y plane
		zspin = (rel [1].x==rel [1].y) ? PI/4 : atan2(rel [1].y,rel [1].x);
		// spin all 3 points on z axis
		for (i=0;i<3;i++)
			RotateVmsVector(&rel [i],zspin,'z');
		// calculate y-axis spin angle to rotate point1 so it lies on x axis
		yspin = (rel [1].z==rel [1].x) ? PI/4 : atan2(rel [1].z,rel [1].x);
		// spin points 1 and 2 on y axis (don't need to spin point 0 since it is at 0,0,0)
		for (i=1;i<=2;i++)
			RotateVmsVector(&rel [i],yspin,'y');
		// calculate x-axis spin angle to rotate point2 so it lies in x-y plane
		xspin = (rel [2].z==rel [2].y) ? PI/4 : atan2(rel [2].z,rel [2].y);
		// spin points 2 on x axis (don't need to spin point 1 since it is on the x-axis
		RotateVmsVector(&rel [2],xspin,'x');
		RotateVmsMatrix(&objP->orient,zspin,'z');
		RotateVmsMatrix(&objP->orient,yspin,'y');
		RotateVmsMatrix(&objP->orient,xspin,'x');
		RotateVmsMatrix(&objP->orient,-xspin,'x');
		RotateVmsMatrix(&objP->orient,-yspin,'y');
		RotateVmsMatrix(&objP->orient,-zspin,'z');
#endif //SPIN_RELATIVE
		break;

	case BLOCK_MODE:
		theApp.SetModified (TRUE);
		// calculate center of current cube
		center.Clear ();
		for (i = 0; i < 8; i++) {
			center += *Vertices (segP->verts [i]);
			}
		center >>= 3;
		// calculate center of oppisite current side
		opp_center.Clear ();
		for (i = 0; i < 4; i++) {
			opp_center += *Vertices (segP->verts [opp_side_vert [nSide][i]]);
			}
		opp_center.x >>= 2;
		// rotate points about a point
		for (i=0;i<VertCount ();i++)
			if (VertStatus (i) & MARKED_MASK)
				RotateVertex(Vertices (i), &center, &opp_center, angle);
		// rotate Objects () within marked cubes
		objP = Objects (0);
		for (i = GameInfo ().objects.count; i; i--, objP++)
			if (Segments (objP->nSegment)->wallFlags & MARKED_MASK)
				RotateVertex (&objP->pos, &center, &opp_center, angle);
		break;
	}
return true;
}


//-----------------------------------------------------------------------------
// RotateVmsVector
//-----------------------------------------------------------------------------

  class vms {
  public:
    FIX x,y,z;
    vms() {x=y=z=0;}
    vms(FIX x0,FIX y0,FIX z0) {
      x=x0;y=y0;z=z0;
    }
    friend vms operator +(vms v1, vms v2) {
      return vms(v1.x+v2.x,v1.y+v2.y,v1.z+v2.z);
    }
  };

 void CMine::RotateVmsVector (CFixVector* vector,double angle,char axis) 
{
  CFixVector n;

  vms m,vertP;
  vertP.x = vector->x;
  vertP.y = vector->y;
  vertP.z = vector->z;
  switch(axis) {
    case 'x':
      m.x = (long) (vertP.x*cos(angle) + vertP.y*sin(angle));
      m.y = (long) (-vertP.x*sin(angle) + vertP.y*cos(angle));
      vertP = m;
      vector->x = vertP.x;
      vector->y = vertP.y;
      vector->z = vertP.z;
//      vector->x = n.x;
//      vector->y = n.y;
      break;
    case 'y':
      n.x = (long) (vector->x*cos(angle) - vector->z*sin(angle));
      n.z = (long) (vector->x*sin(angle) + vector->z*cos(angle));
      vector->x = n.x;
      vector->z = n.z;
      break;
    case 'z':
      n.y = (long) (vector->y*cos(angle) + vector->z*sin(angle));
      n.z = (long) (-vector->y*sin(angle) + vector->z*cos(angle));
      vector->y = n.y;
      vector->z = n.z;
      break;
  }
}

//-----------------------------------------------------------------------------
// RotateVmsMatrix
//-----------------------------------------------------------------------------

void CMine::RotateVmsMatrix (CFixMatrix *matrix,double angle,char axis) 
{
  CFixMatrix new_vms;
  double cosx,sinx;

  cosx = cos(angle);
  sinx = sin(angle);
      switch (axis) {
	case 'x':
// spin x
//	1	0	0
//	0	cos	sin
//	0	-sin	cos
//
	  new_vms.uvec.x = (long) (matrix->uvec.x * cosx + matrix->fvec.x * sinx);
	  new_vms.uvec.y = (long) (matrix->uvec.y * cosx + matrix->fvec.y * sinx);
	  new_vms.uvec.z = (long) (matrix->uvec.z * cosx + matrix->fvec.z * sinx);
	  new_vms.fvec.x = (long) (-matrix->uvec.x * sinx + matrix->fvec.x * cosx);
	  new_vms.fvec.y = (long) (-matrix->uvec.y * sinx + matrix->fvec.y * cosx);
	  new_vms.fvec.z = (long) (-matrix->uvec.z * sinx + matrix->fvec.z * cosx);
	  matrix->uvec.x = new_vms.uvec.x;
	  matrix->uvec.y = new_vms.uvec.y;
	  matrix->uvec.z = new_vms.uvec.z;
	  matrix->fvec.x = new_vms.fvec.x;
	  matrix->fvec.y = new_vms.fvec.y;
	  matrix->fvec.z = new_vms.fvec.z;
	  break;
	case 'y':
// spin y
//	cos	0	-sin
//	0	1	0
//	sin	0	cos
//
	  new_vms.rvec.x = (long) (matrix->rvec.x * cosx - matrix->fvec.x * sinx);
	  new_vms.rvec.y = (long) (matrix->rvec.y * cosx - matrix->fvec.y * sinx);
	  new_vms.rvec.z = (long) (matrix->rvec.z * cosx - matrix->fvec.z * sinx);
	  new_vms.fvec.x = (long) (matrix->rvec.x * sinx + matrix->fvec.x * cosx);
	  new_vms.fvec.y = (long) (matrix->rvec.y * sinx + matrix->fvec.y * cosx);
	  new_vms.fvec.z = (long) (matrix->rvec.z * sinx + matrix->fvec.z * cosx);
	  matrix->rvec.x = new_vms.rvec.x;
	  matrix->rvec.y = new_vms.rvec.y;
	  matrix->rvec.z = new_vms.rvec.z;
	  matrix->fvec.x = new_vms.fvec.x;
	  matrix->fvec.y = new_vms.fvec.y;
	  matrix->fvec.z = new_vms.fvec.z;
	  break;
	case 'z':
// spin z
//	cos	sin	0
//	-sin	cos	0
//	0	0	1
//
	  new_vms.rvec.x = (long) (matrix->rvec.x * cosx + matrix->uvec.x * sinx);
	  new_vms.rvec.y = (long) (matrix->rvec.y * cosx + matrix->uvec.y * sinx);
	  new_vms.rvec.z = (long) (matrix->rvec.z * cosx + matrix->uvec.z * sinx);
	  new_vms.uvec.x = (long) (-matrix->rvec.x * sinx + matrix->uvec.x * cosx);
	  new_vms.uvec.y = (long) (-matrix->rvec.y * sinx + matrix->uvec.y * cosx);
	  new_vms.uvec.z = (long) (-matrix->rvec.z * sinx + matrix->uvec.z * cosx);
	  matrix->rvec.x = new_vms.rvec.x;
	  matrix->rvec.y = new_vms.rvec.y;
	  matrix->rvec.z = new_vms.rvec.z;
	  matrix->uvec.x = new_vms.uvec.x;
	  matrix->uvec.y = new_vms.uvec.y;
	  matrix->uvec.z = new_vms.uvec.z;
	  break;
      }
}


/***************************************************************************
                           RotateVertex

  ACTION - Rotates a vertex around a center point perpendicular to direction
		   vector.

***************************************************************************/

void CMine::RotateVertex(CFixVector* vertex, CFixVector* origin, CFixVector* normal, double angle) 
{

  double z_spin,y_spin;
  double vx,vy,vz,nx,ny,nz;
  double x1,y1,z1,x2,y2,z2,x3,y3,z3;

  // translate coordanites to origin
  vx = vertex->x - origin->x;
  vy = vertex->y - origin->y;
  vz = vertex->z - origin->z;
  nx = normal->x - origin->x;
  ny = normal->y - origin->y;
  nz = normal->z - origin->z;

  // calculate angles to normalize direction
  // spin on z axis to get into the x-z plane
  if (ny==nx) {
    z_spin = PI/4;
  } else {
    z_spin = atan2(ny,nx);
  }
  x1 =   nx*cos(z_spin) + ny*sin(z_spin);
//  y1 = - nx*sin(z_spin) + ny*cos(z_spin); /* this should equal 0 */
  z1 = nz;

  // spin on y to get on the x axis
  if (z1==x1) {
    y_spin = PI/4;
  } else {
    y_spin = -atan2(z1,x1);
  }
//  x2 =   x1*cos(y_spin) - z1*sin(y_spin);
//  y2 =   y1;                               // this should equal 0
//  z2 =   x1*sin(y_spin) + z1*cos(y_spin);  // this should equal 0
//  if (y2!=0 || x2!=0) {
//    sprintf_s (message, sizeof (message), "Rotation error (y and z should be zero):"
//                      "(x,y,z) = (%f,%f,%f)",(double)x2,(double)y2,(double)z2);
//    DEBUGMSG(message);
//  }

  // normalize vertex (spin on z then y)
  x1 =   vx*cos(z_spin) + vy*sin(z_spin);
  y1 = - vx*sin(z_spin) + vy*cos(z_spin);
  z1 =   vz;

  x2 =   x1*cos(y_spin) - z1*sin(y_spin);
  y2 =   y1;
  z2 =   x1*sin(y_spin) + z1*cos(y_spin);

  // spin x
  x3 =   x2;
  y3 =   y2*cos(angle)  + z2*sin(angle);
  z3 = - y2*sin(angle)  + z2*cos(angle);

  // spin back in negative direction (y first then z)
  x2 =   x3*cos(-y_spin) - z3*sin(-y_spin);
  y2 =   y3;
  z2 =   x3*sin(-y_spin) + z3*cos(-y_spin);

  x1 =   x2*cos(-z_spin) + y2*sin(-z_spin);
  y1 = - x2*sin(-z_spin) + y2*cos(-z_spin) ;
  z1 =   z2;

  // translate back
  vertex->x = (long) (x1 + origin->x);
  vertex->y = (long) (y1 + origin->y);
  vertex->z = (long) (z2 + origin->z);

  // round off values
//  round_off(&vertex->x,grid);
//  round_off(&vertex->y,grid);
//  round_off(&vertex->z,grid);
}

// eof modify.cpp