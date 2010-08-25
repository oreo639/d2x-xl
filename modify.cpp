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
  CFixVector		center, oppCenter;
  CDoubleVector	v;
  INT32				i;
/* calculate center of current side */

for (i = 0; i < 4; i++) {
	INT32 nVertex = Segments (Current ()->nSegment)->m_info.verts [sideVertTable [Current ()->nSide][i]];
   center += *Vertices (nVertex);
	 }
center /= 4.0;

// calculate center of opposite of current side
for (i = 0; i < 4; i++) {
	INT32 nVertex = Segments (Current ()->nSegment)->m_info.verts [oppSideVertTable [Current ()->nSide][i]];
   oppCenter += *Vertices (nVertex);
	}
oppCenter /= 4.0;

center -= oppCenter;
// normalize vector
v = CDoubleVector (center);

// normalize direction
radius = v.Mag ();

if (radius > 0.1)
	v /= radius;
else
	v = CalcSideNormal (Current ()->nSegment, Current ()->nSide);

// move on x, y, and z
 theApp.SetModified (TRUE);
 theApp.LockUndo ();
 v *= moveRate;
 MoveOn (v);
 theApp.UnlockUndo ();
 return true;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoBack (void) 
{
  CFixVector	center, oppCenter;
  INT32			i;

/* calculate center of current side */
for (i = 0; i < 4; i++) {
	INT32 nVertex = Segments (Current ()->nSegment)->m_info.verts [sideVertTable [Current ()->nSide][i]];
	center += *Vertices (nVertex);
	}
center /= 4.0;

// calculate center of oppisite current side
for (i = 0; i < 4; i++) {
	INT32 nVertex = Segments (Current ()->nSegment)->m_info.verts [oppSideVertTable [Current ()->nSide][i]];
	oppCenter += *Vertices (nVertex);
	}
oppCenter /= 4.0;

// normalize vector
CDoubleVector v (center - oppCenter);

// make sure distance is positive to prevent
// cube from turning inside out
// defines line orthogonal to a side at a point
	UINT8 sideNormalTable [6][4] = {
		{8,6,1,3},
		{0,5,7,2},
		{3,1,6,8},
		{2,7,5,0},
		{4,9,10,11},
		{11,10,9,4}
		};
	CSegment *segP;
	bool		okToMove = true;

segP = Segments (0) + Current ()->nSegment;
UINT8* sideNormalP = sideNormalTable [Current ()->nSide];
switch (m_selectMode) {
	case POINT_MODE:
		if (Distance (*Vertices (segP->m_info.verts [lineVertTable [sideNormalP [Current ()->nPoint]][0]]), 
						  *Vertices (segP->m_info.verts [lineVertTable [sideNormalP [Current ()->nPoint]][1]]))
			 - moveRate < 0.25) 
			okToMove = false;
		break;

	case LINE_MODE:
		for (i = 0; i < 2; i++) {
			if (Distance (*Vertices (segP->m_info.verts [lineVertTable [sideNormalP [(Current ()->nLine + i) % 4]][0]]), 
							  *Vertices (segP->m_info.verts [lineVertTable [sideNormalP [(Current ()->nLine + i) % 4]][1]]))
				 - moveRate < 0.25) 
				okToMove = false;
			}
	break;

	case SIDE_MODE:
		for (i = 0; i < 4; i++) {
			if (Distance (*Vertices (segP->m_info.verts [lineVertTable [sideNormalP [i]][0]]), 
							  *Vertices (segP->m_info.verts [lineVertTable [sideNormalP [i]][1]]))
				 - moveRate < 0.25) 
			okToMove = false;
			}
		break;
	}
if (!okToMove) {
	ErrorMsg ("Too small to move in that direction");
	return false;
	}

double radius = v.Mag ();
if (radius - moveRate < 0.25) {
	if (m_selectMode == POINT_MODE || m_selectMode == LINE_MODE || m_selectMode == SIDE_MODE) {
		ErrorMsg ("Cannot make cube any smaller\n"
		"Cube must be greater or equal to 1.0 units wide.");
		return false;
		}
	}
else {
	// normalize direction
	if (radius > 0.1) 
		v /= radius;
	else 
		v = CalcSideNormal (Current ()->nSegment,Current ()->nSide);
	// move on x, y, and z
	theApp.SetModified (TRUE);
	theApp.LockUndo ();
	v *= -moveRate;
	MoveOn (v);
	theApp.UnlockUndo ();
	}
theApp.SetModified (TRUE);
return true;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoRotRight (void)
{
return SpinSelection (angleRate);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoRotLeft (void)
{
return SpinSelection (-angleRate);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoUp (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (angleRate,FALSE) : MovePoints (1,0);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoDown (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (-angleRate, FALSE) : MovePoints (0,1);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoRight (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (angleRate,TRUE) : MovePoints(3,0);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

bool CMine::EditGeoLeft (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (-angleRate,TRUE) : MovePoints(0,3);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
bool CMine::EditGeoGrow (void) 
{
return ResizeItem (moveRate);
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
bool CMine::EditGeoShrink (void) 
{
return ResizeItem (-moveRate);
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
	INT32			nSegment = Current ()->nSegment;
	INT32			nSide = Current ()->nSide;
	CSegment*	segP = Segments (nSegment);
	CVertex		center, oppCenter;
	INT32			i, pts [4];

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
		oppCenter = Average (*Vertices (segP->m_info.verts [oppSideVertTable [nSide][pts [0]]]),
									*Vertices (segP->m_info.verts [oppSideVertTable [nSide][pts [1]]]));
		// calculate center opp side line 2
		center = Average (*Vertices (segP->m_info.verts [oppSideVertTable [nSide][pts [2]]]),
								*Vertices (segP->m_info.verts [oppSideVertTable [nSide][pts [3]]]));
		// rotate points around a line
		for (i = 0; i < 4; i++)
			Vertices (segP->m_info.verts [sideVertTable [nSide][i]])->Rotate (center, oppCenter, angle);
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
//			ResizeItem()
//
// need to prevent reduction through zero
// absolute value of shorts line to size must be greater
// then incremental value if inc is negetive
//
//***************************************************************************

bool CMine::ResizeItem (double delta) 
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
		point [0] = lineVertTable [sideLineTable [Current ()->nSide][Current ()->nLine]][0];
		point [1] = lineVertTable [sideLineTable [Current ()->nSide][Current ()->nLine]][1];
		return ResizeLine (segP, point [0], point [1], delta);

	case SIDE_MODE:
		theApp.SetModified (TRUE);
		theApp.LockUndo ();
		for (i = 0; i < 4; i++)
			point [i] = sideVertTable [Current ()->nSide][i];
		// enlarge the diagonals
		result = ResizeLine (segP, point [0], point [2], (INT32) (delta*sqrt(2.0))) &&
				   ResizeLine (segP, point [1], point [3], (INT32) (delta*sqrt(2.0)));
		theApp.UnlockUndo ();
		return result;

	case CUBE_MODE:
		// enlarge the diagonals
		theApp.SetModified (TRUE);
		theApp.LockUndo ();
		result = ResizeLine (segP, 0, 6, (INT32) (delta*sqrt(3.0))) &&
				   ResizeLine (segP, 1, 7, (INT32) (delta*sqrt(3.0))) &&
					ResizeLine (segP, 2, 4, (INT32) (delta*sqrt(3.0))) &&
					ResizeLine (segP, 3, 5, (INT32) (delta*sqrt(3.0)));
		theApp.UnlockUndo ();
		return result;

	case OBJECT_MODE:
		return false;

	case BLOCK_MODE:
		theApp.SetModified (TRUE);
		CVertex	max_pt (-0x7fffffffL, -0x7fffffffL, -0x7fffffffL), 
					min_pt (0x7fffffffL, 0x7fffffffL, 0x7fffffffL), 
					center;
		CVertex* verts = Vertices (0);
		for (i = VertCount (), j = 0; j < i; j++, verts++)
			if (verts->m_status & MARKED_MASK) {
				max_pt = Max (max_pt, *verts);
				min_pt = Min (min_pt, *verts);
				}
		center = Average (max_pt, min_pt);
		double scale = ((double)(20*F1_0) + (double)delta)/(double)(20*F1_0);
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

bool CMine::MovePoints (INT32 pt0, INT32 pt1) 
{
	CDoubleVector	delta;
	double			length;
	INT32				i;
	CSegment*		segP = Segments (Current ()->nSegment);
	UINT8*			sideVertP = sideVertTable [Current ()->nSide];
	INT16				p0 = sideVertP [CURRENT_POINT(pt0)];
	INT16				p1 = sideVertP [CURRENT_POINT(pt1)];

delta = *Vertices (segP->m_info.verts [p1]) - *Vertices (segP->m_info.verts [p0]);
length = delta.Mag ();
if (length < 1.0) 
	delta.Set (moveRate, 0, 0);
else
	delta *= ((double) moveRate / length);

switch (m_selectMode){
	case POINT_MODE:
		*Vertices (segP->m_info.verts [p0]) += delta;
		theApp.SetModified (TRUE);
		break;

	case LINE_MODE:
		*Vertices (segP->m_info.verts [p0]) += delta;
		*Vertices (segP->m_info.verts [p1]) += delta;
		theApp.SetModified (TRUE);
		break;

	case SIDE_MODE:
		for (i = 0; i < 4; i++)
			*Vertices (segP->m_info.verts [sideVertP [i]]) += delta;
		theApp.SetModified (TRUE);
		break;

	case CUBE_MODE:
		for (i = 0; i < 8; i++) 
			*Vertices (segP->m_info.verts [i]) += delta;
		theApp.SetModified (TRUE);
		break;

	case OBJECT_MODE:
		CurrObj ()->m_location.pos += delta;
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
//     				ResizeLine()
//
// prevent lines from being bigger than 8*20 and less than 3
//--------------------------------------------------------------------------

bool CMine::ResizeLine (CSegment *segP, INT32 point0, INT32 point1, double delta) 
{
#if 0
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
#endif
CVertex	*v1 = Vertices (segP->m_info.verts [point0]),
			*v2 = Vertices (segP->m_info.verts [point1]);
double	radius;
CDoubleVector	v (*v1 - *v2);
// figure out direction to modify line
// normalize direction
radius = v.Mag ();
if (radius > (double) F1_0 - delta) 
	return false;
if ((delta < 0.0) && (radius <= -delta * 3.0)) 
	return false;
if (radius == 0.0)
	return false;
v *= delta / radius;
// update vertices
CFixVector h (v);
*v1 += h;
*v2 -= h;
return true;
}

/***************************************************************************
				MoveOn()
***************************************************************************/

bool CMine::MoveOn (CFixVector delta) 
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
		*Vertices (segP->m_info.verts [sideVertTable [nSide][nPoint]]) += delta;
		break;

	case LINE_MODE:
		*Vertices (segP->m_info.verts [lineVertTable [sideLineTable [nSide][nLine]][0]]) += delta;
		*Vertices (segP->m_info.verts [lineVertTable [sideLineTable [nSide][nLine]][1]]) += delta;
		break;

	case SIDE_MODE:
		for (i = 0; i < 4; i++)
			*Vertices (segP->m_info.verts [sideVertTable [nSide][i]]) += delta;
		break;

	case CUBE_MODE:
		for (i = 0; i < 8; i++)
			*Vertices (segP->m_info.verts [i]) += delta;
		for (i = 0; i < GameInfo ().objects.count; i++)
			if (Objects (i)->m_info.nSegment == nSegment)
				Objects (i)->m_location.pos += delta;
		break;

	case OBJECT_MODE:
		CurrObj ()->m_location.pos += delta;
		break;
	break;

	case BLOCK_MODE:
		CGameObject *objP = Objects (0);
		for (i = 0; i < MAX_VERTICES; i++)
			if (VertStatus (i) & MARKED_MASK)
				*Vertices (i) += delta;
		for (i = GameInfo ().objects.count; i; i--, objP++)
			if (objP->m_info.nSegment >= 0)
				if (Segments (objP->m_info.nSegment)->m_info.wallFlags & MARKED_MASK)
					objP->m_location.pos += delta;
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
	INT32				nSegment = Current ()->nSegment;
	INT32				nSide = Current ()->nSide;
	CSegment*		segP = Segments (nSegment);
	CGameObject*	objP;
	CDoubleMatrix* orient;
	CVertex			center, oppCenter, normal;
	INT16				i;

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
			center += *Vertices (segP->m_info.verts [sideVertTable [nSide][i]]);
			}
		center /= 4.0;
		// calculate orthogonal vector from lines which intersect point 0
		//       |x  y  z |
		// AxB = |ax ay az| = x(aybz-azby), y(azbx-axbz), z(axby-aybx)
		//       |bx by bz|
		normal = CalcSideNormal ();
		// normalize the vector
		// set sign (since vert numbers for most sides don't follow right-handed convention)
		if ((nSide < 2) || (nSide > 4))
			normal = -normal;
		// set opposite center
		oppCenter = center + normal;
		/* rotate points around a line */
		for (i = 0; i < 4; i++)
			Vertices (segP->m_info.verts [sideVertTable [nSide][i]])->Rotate (center, oppCenter, angle);
		break;

	case CUBE_MODE:	// spin cube around the center of the cube using screen's perspective
		// calculate center of current cube
		theApp.SetModified (TRUE);
		center.Clear ();
		for (i = 0; i < 8; i++) 
			center += *Vertices (segP->m_info.verts [i]);
		center /= 8.0;
		// calculate center of oppisite current side
		oppCenter.Clear ();
		for (i = 0; i < 4; i++) 
			oppCenter += *Vertices (segP->m_info.verts [oppSideVertTable [nSide][i]]);
		oppCenter /= 4.0;
		// rotate points about a point
		for (i = 0; i < 8; i++)
			Vertices (segP->m_info.verts [i])->Rotate (center, oppCenter, angle);
		break;

	case OBJECT_MODE:	// spin object vector
		theApp.SetModified (TRUE);
		orient = (Current ()->nObject == GameInfo ().objects.count) ? &SecretOrient () : &CurrObj ()->m_location.orient;
		switch (nSide) {
			case 0:
				orient->Rotate (angle, 'x');
				break;
			case 2:
				orient->Rotate (-angle, 'x');
				break;
			case 1:
				orient->Rotate (-angle, 'y');
				break;
			case 3:
				orient->Rotate (angle, 'y');
				break;
			case 4:
				orient->Rotate (angle, 'z');
				break;
			case 5:
				orient->Rotate (-angle, 'z');
				break;
			}
		break;

	case BLOCK_MODE:
		theApp.SetModified (TRUE);
		// calculate center of current cube
		center.Clear ();
		for (i = 0; i < 8; i++) {
			center += *Vertices (segP->m_info.verts [i]);
			}
		center /= 8.0;
		// calculate center of oppisite current side
		oppCenter.Clear ();
		for (i = 0; i < 4; i++) {
			oppCenter += *Vertices (segP->m_info.verts [oppSideVertTable [nSide][i]]);
			}
		oppCenter /= 4.0;
		// rotate points about a point
		for (i=0;i<VertCount ();i++)
			if (VertStatus (i) & MARKED_MASK)
				Vertices (i)->Rotate (center, oppCenter, angle);
		// rotate Objects () within marked cubes
		objP = Objects (0);
		for (i = GameInfo ().objects.count; i; i--, objP++)
			if (Segments (objP->m_info.nSegment)->m_info.wallFlags & MARKED_MASK)
				objP->m_location.pos.Rotate (center, oppCenter, angle);
		break;
	}
return true;
}

// eof modify.cpp