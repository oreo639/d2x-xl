// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include "dle-xp-res.h"

#include <math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "FileManager.h"
#include "textures.h"
#include "PaletteManager.h"
#include "dle-xp.h"

#define CURRENT_POINT(a) ((current->m_nPoint + (a))&0x03)

//This dialog is always active,
//but is hidden or restored by user
//::ShowWindow(hwnd,SW_HIDE);
//::ShowWindow(hwnd,SW_RESTORE);

bool CMine::EditGeoFwd (void)
{
  double				radius;
  CVertex			center, oppCenter;
  CDoubleVector	v;
  int				i;
/* calculate center of current side */

for (i = 0; i < 4; i++) {
	int nVertex = current->Segment ()->Info ().verts [sideVertTable [current->m_nSide][i]];
   center += *vertexManager.Vertex (nVertex);
	 }
center /= 4.0;

// calculate center of opposite of current side
for (i = 0; i < 4; i++) {
	int nVertex = current->Segment ()->Info ().verts [oppSideVertTable [current->m_nSide][i]];
   oppCenter += *vertexManager.Vertex (nVertex);
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
	v = segmentManager.CalcSideNormal (*current);

// move on x, y, and z
 v *= moveRate;
 MoveOn (v);
 return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

bool CMine::EditGeoBack (void) 
{
  CVertex	center, oppCenter;
  int		i;

/* calculate center of current side */
for (i = 0; i < 4; i++) {
	int nVertex = current->Segment ()->Info ().verts [sideVertTable [current->m_nSide][i]];
	center += *vertexManager.Vertex (nVertex);
	}
center /= 4.0;

// calculate center of oppisite current side
for (i = 0; i < 4; i++) {
	int nVertex = current->Segment ()->Info ().verts [oppSideVertTable [current->m_nSide][i]];
	oppCenter += *vertexManager.Vertex (nVertex);
	}
oppCenter /= 4.0;

// normalize vector
CDoubleVector v (center - oppCenter);

// make sure distance is positive to prevent
// segment from turning inside out
// defines line orthogonal to a side at a point
	static byte sideNormalTable [6][4] = {
		{8,6,1,3},
		{0,5,7,2},
		{3,1,6,8},
		{2,7,5,0},
		{4,9,10,11},
		{11,10,9,4}
		};
	CSegment *segP;
	bool		okToMove = true;

segP = current->Segment ();
byte* sideNormalP = sideNormalTable [current->m_nSide];
switch (m_selectMode) {
	case POINT_MODE:
		if (Distance (*vertexManager.Vertex (segP->Info ().verts [lineVertTable [sideNormalP [current->m_nPoint]][0]]), 
						  *vertexManager.Vertex (segP->Info ().verts [lineVertTable [sideNormalP [current->m_nPoint]][1]]))
			 - moveRate < 0.25) 
			okToMove = false;
		break;

	case LINE_MODE:
		for (i = 0; i < 2; i++) {
			if (Distance (*vertexManager.Vertex (segP->Info ().verts [lineVertTable [sideNormalP [(current->m_nLine + i) % 4]][0]]), 
							  *vertexManager.Vertex (segP->Info ().verts [lineVertTable [sideNormalP [(current->m_nLine + i) % 4]][1]]))
				 - moveRate < 0.25) 
				okToMove = false;
			}
	break;

	case SIDE_MODE:
		for (i = 0; i < 4; i++) {
			if (Distance (*vertexManager.Vertex (segP->Info ().verts [lineVertTable [sideNormalP [i]][0]]), 
							  *vertexManager.Vertex (segP->Info ().verts [lineVertTable [sideNormalP [i]][1]]))
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
		ErrorMsg ("Cannot make segment any smaller\n"
		"Cube must be greater or equal to 1.0 units wide.");
		return false;
		}
	}
else {
	// normalize direction
	if (radius > 0.1) 
		v /= radius;
	else 
		v = segmentManager.CalcSideNormal (*current);
	// move on x, y, and z
	v *= -moveRate;
	MoveOn (v);
	}
return true;
}

//------------------------------------------------------------------------------

bool CMine::EditGeoRotRight (void)
{
return SpinSelection (angleRate);
}

//------------------------------------------------------------------------------

bool CMine::EditGeoRotLeft (void)
{
return SpinSelection (-angleRate);
}

//------------------------------------------------------------------------------

bool CMine::EditGeoUp (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (angleRate, false) : MovePoints (1,0);
}

//------------------------------------------------------------------------------

bool CMine::EditGeoDown (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (-angleRate, false) : MovePoints (0,1);
}

//------------------------------------------------------------------------------

bool CMine::EditGeoRight (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (angleRate,true) : MovePoints(3,0);
}

//------------------------------------------------------------------------------

bool CMine::EditGeoLeft (void) 
{
return (m_selectMode == SIDE_MODE) ? RotateSelection (-angleRate,true) : MovePoints(0,3);
}

//------------------------------------------------------------------------------

bool CMine::EditGeoGrow (void) 
{
return ResizeItem (moveRate);
}

//------------------------------------------------------------------------------

bool CMine::EditGeoShrink (void) 
{
return ResizeItem (-moveRate);
}

//------------------------------------------------------------------------------
//                    RotateSelection()
//
// ACTION - rotates a side about the opposite side.  The line is drawn
//          between the center points of lines 0 and 2.  If perpendicular
//          is true, then the lines 1 and 3 are used instead.
//------------------------------------------------------------------------------

bool CMine::RotateSelection (double angle, bool perpendicular) 
{
	int			nSegment = current->m_nSegment;
	int			nSide = current->m_nSide;
	CSegment*	segP = current->Segment ();
	CVertex		center, oppCenter;
	int			i, pts [4];

switch (m_selectMode){
	case POINT_MODE:
		ErrorMsg ("Cannot bend a point");
		return false;

	case LINE_MODE:
		ErrorMsg ("Cannot bend a line");
		return false;

	case SIDE_MODE:	// spin side around the opposite side
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
		oppCenter = Average (*vertexManager.Vertex (segP->Info ().verts [oppSideVertTable [nSide][pts [0]]]),
									*vertexManager.Vertex (segP->Info ().verts [oppSideVertTable [nSide][pts [1]]]));
		// calculate center opp side line 2
		center = Average (*vertexManager.Vertex (segP->Info ().verts [oppSideVertTable [nSide][pts [2]]]),
								*vertexManager.Vertex (segP->Info ().verts [oppSideVertTable [nSide][pts [3]]]));
		// rotate points around a line
		undoManager.Begin (udVertices);
		for (i = 0; i < 4; i++)
			vertexManager.Vertex (segP->Info ().verts [sideVertTable [nSide][i]])->Rotate (center, oppCenter, angle);
		undoManager.End ();	
		break;
	
	case SEGMENT_MODE:
		ErrorMsg ("Cannot bend a segment");
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
	int nSegment = current->m_nSegment;
	int nSide = current->m_nSide;
	CSegment *segP = current->Segment ();
	int i, j, point [4];
	bool result = false;

switch (m_selectMode) {
	case POINT_MODE:
		return false;

	case LINE_MODE:
		point [0] = lineVertTable [sideLineTable [current->m_nSide][current->m_nLine]][0];
		point [1] = lineVertTable [sideLineTable [current->m_nSide][current->m_nLine]][1];
		undoManager.Begin (udVertices);
		result = ResizeLine (segP, point [0], point [1], delta);
		undoManager.End ();
		return result;

	case SIDE_MODE:
		for (i = 0; i < 4; i++)
			point [i] = sideVertTable [current->m_nSide][i];
		// enlarge the diagonals
		undoManager.Begin (udVertices);
		result = ResizeLine (segP, point [0], point [2], (int) (delta*sqrt(2.0))) &&
				   ResizeLine (segP, point [1], point [3], (int) (delta*sqrt(2.0)));
		undoManager.End ();
		return result;

	case SEGMENT_MODE:
		// enlarge the diagonals
		undoManager.Begin (udVertices);
		result = ResizeLine (segP, 0, 6, (int) (delta*sqrt(3.0))) &&
				   ResizeLine (segP, 1, 7, (int) (delta*sqrt(3.0))) &&
					ResizeLine (segP, 2, 4, (int) (delta*sqrt(3.0))) &&
					ResizeLine (segP, 3, 5, (int) (delta*sqrt(3.0)));
		undoManager.End ();
		return result;

	case OBJECT_MODE:
		return false;

	case BLOCK_MODE:
		CVertex	max_pt (-0x7fffffffL, -0x7fffffffL, -0x7fffffffL), 
					min_pt (0x7fffffffL, 0x7fffffffL, 0x7fffffffL), 
					center;
		CVertex* vertP = vertexManager.Vertex (0);
		for (i = vertexManager.Count (), j = 0; j < i; j++, vertP++)
			if (vertP->Status () & MARKED_MASK) {
				max_pt = Max (max_pt, *vertP);
				min_pt = Min (min_pt, *vertP);
				}
		center = Average (max_pt, min_pt);
		undoManager.Begin (udVertices);
		vertP = vertexManager.Vertex (0);
		double scale = (20.0 + delta) / 20.0;
		for (i = vertexManager.Count (), j = 0; j < i; j++, vertP++)
			if (vertP->IsMarked ()) {
				*vertP -= center;
				*vertP *= scale;
				*vertP += center;
				}
		undoManager.End ();
		return true;
	}
return false;
}

//--------------------------------------------------------------------------------
// MovePoints()
//
// moves blocks, sides, segments, lines, and points in the direction
// of the current line.
//--------------------------------------------------------------------------------

bool CMine::MovePoints (int pt0, int pt1) 
{
	CDoubleVector	delta;
	double			length;
	int				i;
	CSegment*		segP = current->Segment ();
	byte*				sideVertP = sideVertTable [current->m_nSide];
	short				p0 = sideVertP [CURRENT_POINT(pt0)];
	short				p1 = sideVertP [CURRENT_POINT(pt1)];

delta = *vertexManager.Vertex (segP->Info ().verts [p1]) - *vertexManager.Vertex (segP->Info ().verts [p0]);
length = delta.Mag ();
if (length < 1.0) 
	delta.Set (moveRate, 0, 0);
else
	delta *= ((double) moveRate / length);

switch (m_selectMode){
	case POINT_MODE:
		undoManager.Begin (udVertices);
		*vertexManager.Vertex (segP->Info ().verts [p0]) += delta;
		undoManager.End ();
		break;

	case LINE_MODE:
		undoManager.Begin (udVertices);
		*vertexManager.Vertex (segP->Info ().verts [p0]) += delta;
		*vertexManager.Vertex (segP->Info ().verts [p1]) += delta;
		undoManager.End ();
		break;

	case SIDE_MODE:
		undoManager.Begin (udVertices);
		for (i = 0; i < 4; i++)
			*vertexManager.Vertex (segP->Info ().verts [sideVertP [i]]) += delta;
		undoManager.End ();
		break;

	case SEGMENT_MODE:
		undoManager.Begin (udVertices);
		for (i = 0; i < 8; i++) 
			*vertexManager.Vertex (segP->Info ().verts [i]) += delta;
		undoManager.End ();
		break;

	case OBJECT_MODE:
		undoManager.Begin (udObjects);
		current->Object ()->Position () += delta;
		undoManager.End ();
		break;

	case BLOCK_MODE:
		bool bMoved = false;
		undoManager.Begin (udVertices);
		for (i = 0; i < MAX_VERTICES; i++) {
			if (vertexManager.Status (i) & MARKED_MASK) {
				*vertexManager.Vertex (i) += delta;
				bMoved = true;
				}
			}
		undoManager.End ();
		break;
	}
return true;
}

//--------------------------------------------------------------------------------
//     				ResizeLine()
//
// prevent lines from being bigger than 8*20 and less than 3
//--------------------------------------------------------------------------------

bool CMine::ResizeLine (CSegment *segP, int point0, int point1, double delta) 
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
CVertex	*v1 = vertexManager.Vertex (segP->Info ().verts [point0]),
			*v2 = vertexManager.Vertex (segP->Info ().verts [point1]);
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
*v1 += v;
*v2 -= v;
return true;
}

/***************************************************************************
				MoveOn()
***************************************************************************/

bool CMine::MoveOn (CDoubleVector delta) 
{
int nSegment = current->m_nSegment;
int nSide = current->m_nSide;
int nPoint = current->m_nPoint;
int nLine = current->m_nLine;
CSegment *segP = current->Segment ();
CGameObject* objP;
short i;

switch (m_selectMode) {
	case POINT_MODE:
		undoManager.Begin (udVertices);
		*vertexManager.Vertex (segP->Info ().verts [sideVertTable [nSide][nPoint]]) += delta;
		undoManager.End ();
		break;

	case LINE_MODE:
		undoManager.Begin (udVertices);
		*vertexManager.Vertex (segP->Info ().verts [lineVertTable [sideLineTable [nSide][nLine]][0]]) += delta;
		*vertexManager.Vertex (segP->Info ().verts [lineVertTable [sideLineTable [nSide][nLine]][1]]) += delta;
		undoManager.End ();
		break;

	case SIDE_MODE:
		undoManager.Begin (udVertices);
		for (i = 0; i < 4; i++)
			*vertexManager.Vertex (segP->Info ().verts [sideVertTable [nSide][i]]) += delta;
		undoManager.End ();
		break;

	case SEGMENT_MODE:
		undoManager.Begin (udVertices | udObjects);
		for (i = 0; i < 8; i++)
			*vertexManager.Vertex (segP->Info ().verts [i]) += delta;
		objP = objectManager.Object (0);
		for (i = objectManager.Count (); i; i--, objP++)
			if (objP->m_info.nSegment == nSegment)
				objP->Position () += delta;
		undoManager.End ();
		break;

	case OBJECT_MODE:
		undoManager.Begin (udObjects);
		current->Object ()->Position () += delta;
		undoManager.End ();
		break;
	break;

	case BLOCK_MODE:
		undoManager.Begin (udObjects);
		CGameObject *objP = objectManager.Object (0);
		for (i = 0; i < MAX_VERTICES; i++)
			if (vertexManager.Status (i) & MARKED_MASK)
				*vertexManager.Vertex (i) += delta;
		for (i = objectManager.Count (); i; i--, objP++)
			if (objP->m_info.nSegment >= 0)
				if (objP->Segment ()->IsMarked ())
					objP->Position () += delta;
		undoManager.End ();
		break;
	}
return true;
}

/***************************************************************************
			    SpinSelection()

  ACTION - Spins a side, segment, or object the amount specified.


***************************************************************************/

bool CMine::SpinSelection (double angle) 
{
	int				nSegment = current->m_nSegment;
	int				nSide = current->m_nSide;
	CSegment*		segP = current->Segment ();
	CGameObject*	objP;
	CDoubleMatrix* orient;
	CVertex			center, oppCenter, normal;
	short				i;

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
		center.Clear ();
		for (i = 0; i < 4; i++) {
			center += *vertexManager.Vertex (segP->Info ().verts [sideVertTable [nSide][i]]);
			}
		center /= 4.0;
		// calculate orthogonal vector from lines which intersect point 0
		//       |x  y  z |
		// AxB = |ax ay az| = x(aybz-azby), y(azbx-axbz), z(axby-aybx)
		//       |bx by bz|
		normal = segmentManager.CalcSideNormal (*current);
		// normalize the vector
		// set sign (since vert numbers for most sides don't follow right-handed convention)
		if ((nSide < 2) || (nSide > 4))
			normal = -normal;
		// set opposite center
		oppCenter = center + normal;
		/* rotate points around a line */
		undoManager.Begin (udVertices);
		for (i = 0; i < 4; i++)
			vertexManager.Vertex (segP->Info ().verts [sideVertTable [nSide][i]])->Rotate (center, oppCenter, angle);
		undoManager.End ();
		break;

	case SEGMENT_MODE:	// spin segment around the center of the segment using screen's perspective
		// calculate center of current segment
		center.Clear ();
		for (i = 0; i < 8; i++) 
			center += *vertexManager.Vertex (segP->Info ().verts [i]);
		center /= 8.0;
		// calculate center of oppisite current side
		oppCenter.Clear ();
		for (i = 0; i < 4; i++) 
			oppCenter += *vertexManager.Vertex (segP->Info ().verts [oppSideVertTable [nSide][i]]);
		oppCenter /= 4.0;
		// rotate points about a point
		undoManager.Begin (udVertices);
		for (i = 0; i < 8; i++)
			vertexManager.Vertex (segP->Info ().verts [i])->Rotate (center, oppCenter, angle);
		undoManager.End ();
		break;

	case OBJECT_MODE:	// spin object vector
		undoManager.Begin (udObjects);
		orient = (current->m_nObject == objectManager.Count ()) ? &objectManager.SecretOrient () : &current->Object ()->Orient ();
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
		undoManager.End ();
		break;

	case BLOCK_MODE:
		// calculate center of current segment
		center.Clear ();
		for (i = 0; i < 8; i++) {
			center += *vertexManager.Vertex (segP->Info ().verts [i]);
			}
		center /= 8.0;
		// calculate center of oppisite current side
		oppCenter.Clear ();
		for (i = 0; i < 4; i++) {
			oppCenter += *vertexManager.Vertex (segP->Info ().verts [oppSideVertTable [nSide][i]]);
			}
		oppCenter /= 4.0;
		// rotate points about a point
		undoManager.Begin (udVertices | udObjects);
		for (i = 0; i < vertexManager.Count (); i++)
			if (vertexManager.Status (i) & MARKED_MASK)
				vertexManager.Vertex (i)->Rotate (center, oppCenter, angle);
		// rotate Objects () within marked segments
		objP = objectManager.Object (0);
		for (i = objectManager.Count (); i; i--, objP++)
			if (objP->Segment ()->IsMarked ())
				objP->Position ().Rotate (center, oppCenter, angle);
		undoManager.End ();
		break;
	}
return true;
}

// eof modify.cpp