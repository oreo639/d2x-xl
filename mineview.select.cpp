// dlcView.cpp: implementation of the CMineView class
//

#include "stdafx.h"
#include "winuser.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"

#include "palette.h"
#include "textures.h"
#include "global.h"
#include "render.h"
#include "cfile.h"

#include <math.h>
#include <time.h>

//--------------------------------------------------------------------------
//		       select_current_object()
//
//  ACTION - finds object pointed to by mouse then draws.
//
//--------------------------------------------------------------------------

void CMineView::SelectCurrentObject (long xMouse, long yMouse) 
{
CGameObject *objP;
short closest_object;
short i;
double radius,closest_radius;
APOINT pt;
CGameObject temp_obj;

// default to object 0 but set radius very large
closest_object = 0;
closest_radius = 1.0E30;

// if there is a secret exit, then enable it in search
int enable_secret = FALSE;
if (DLE.IsD2File ())
	for(i=0;i<(short)theMine->MineInfo ().triggers.count;i++)
		if (theMine->Triggers (i)->m_info.type ==TT_SECRET_EXIT) {
			enable_secret = TRUE;
			break;
			}

for (i = 0; i <= theMine->MineInfo ().objects.count; i++) {
	BOOL drawable = FALSE;
	// define temp object type and position for secret object selection
	if (i == theMine->MineInfo ().objects.count && DLE.IsD2File () && enable_secret) {
		objP = &temp_obj;
		objP->m_info.type = OBJ_PLAYER;
		// define objP->position
		CalcSegmentCenter (objP->m_location.pos, (ushort)theMine->SecretCubeNum ());
		}
	else
		objP = theMine->Objects (i);
#if 0
	switch(objP->m_info.type) {
		case OBJ_WEAPON:
			if (ViewObject (eViewObjectsPowerups | eViewObjectsWeapons)) {
				drawable = TRUE;
				}
		case OBJ_POWERUP:
			if (ViewObject (powerup_types [objP->m_info.id])) {
				drawable = TRUE;
				}
			break;
		default:
			if(ViewObject (1<<objP->m_info.type))
				drawable = TRUE;
		}
	if (drawable) 
#else
	if (ViewObject (objP))
#endif
		{
		// translate object's position to screen coordinates
		m_view.Project (objP->m_location.pos, pt);
		// calculate radius^2 (don't bother to take square root)
		double dx = (double)pt.x - (double)xMouse;
		double dy = (double)pt.y - (double)yMouse;
		radius = dx * dx + dy * dy;
	// check to see if this object is closer than the closest so far
		if (radius < closest_radius) {
			closest_object = i;
			closest_radius = radius;
			}
		}
	}

// unhighlight current object and select next object
i = theMine->Current ()->nObject;
RefreshObject(i, closest_object);
}

//--------------------------------------------------------------------------
//		       select_current_segment()
//
//  ACTION - finds segment pointed to by mouse then draws.  Segment must have
//         all points in screen region.
//
//  INPUT  - direction: must be a 1 or a -1
//
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------

int Side (APOINT &p0, APOINT &p1, APOINT &p2)
{
return ((int) p0.y - (int) p1.y) * ((int) p2.x - (int) p1.x)  - 
		 ((int) p0.x - (int) p1.x) * ((int) p2.y - (int) p1.y);
}

//--------------------------------------------------------------------------

bool PointInTriangle (APOINT &p, APOINT &a, APOINT &b, APOINT &c)
{
__int64 fab = Side (p, a, b);
__int64 fbc = Side (p, b, c);
__int64 fca = Side (p, c, a);
return (fab * fbc > 0) && (fca * fbc > 0);
}

//--------------------------------------------------------------------------

bool CMineView::SelectCurrentSegment (short direction, long xMouse, long yMouse) 
{
  CSegment		*segP;
  CRect			rc;
//  extern short xMouse,yMouse;
  short			cur_segment, next_segment;
  short			i, j;
  int			x, y;
  APOINT			sideVerts [4], mousePos;
  bool			bFound = false;

/* find next segment which is within the cursor position */
GetClientRect (rc);
next_segment = cur_segment = theMine->Current ()->nSegment;
mousePos.x = (short) xMouse;
mousePos.y = (short) yMouse;
mousePos.z = 0;
do {
	wrap (&next_segment, direction, 0, theMine->SegCount () - 1); /* point to next segment */
	segP = theMine->Segments (next_segment);
	if (!Visible (segP))
		continue;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++)
			sideVerts [j] = m_viewPoints [segP->m_info.verts [sideVertTable [i][j]]];
		for (j = 0; j < 4; j++) {
			x = sideVerts [j].x;
			y = sideVerts [j].y;
			// allow segment selection if just one of its vertices is visible
			if ((x >= rc.left) && (x <= rc.right) && (y >= rc.top) || (y <= rc.bottom)) {
				sideVerts [j].z = 0;
				if (PointInTriangle (mousePos, sideVerts [0], sideVerts [1], sideVerts [2])) {
					bFound = true;
					goto foundSeg;
					}	
				if (PointInTriangle (mousePos, sideVerts [0], sideVerts [2], sideVerts [3])) {
					bFound = true;
					goto foundSeg;
					}	
				}
			}
		}
#if 0
	xMin = yMin = 0x7fffffff;
	xMax = yMax = -0x7fffffff;
	bOnScreen = true;
	for (i = 0; i < 8; i++) {
		x = m_viewPoints [segP->m_info.verts [i]].x;
		if ((x < rc.left) || (x > rc.right)) {
			bOnScreen = false;
			break;
			}
		y = m_viewPoints [segP->m_info.verts [i]].y;
		if ((y < rc.top) || (y > rc.bottom)) {
			bOnScreen = false;
			break;
			}
		if (xMin > x)
			xMin = x;
		if (xMax < x)
			xMax = x;
		if (yMin > y)
			yMin = y;
		if (yMax < y)
			yMax = y;
		}
	if (!bOnScreen)
		continue;
	if ((xMouse >= xMin) && (xMouse <= xMax) && (yMouse >= yMin) && (yMouse <= yMax))
		break;
#endif
	}
while (next_segment != cur_segment);

foundSeg:

if (!bFound)
	return false;
theMine->Current ()->nSegment = next_segment;
DLE.ToolView ()->Refresh ();
Refresh ();
return true;
}

//==========================================================================
// MENU - NextPoint
//==========================================================================
void CMineView::NextPoint(int dir) 
{
//if ((theMine == null)->SplineActive ())
//	DrawHighlight (1);
//if (m_selectMode==POINT_MODE)
wrap(&theMine->Current ()->nPoint,dir,0,4-1);
theMine->Current ()->nLine = theMine->Current ()->nPoint;
Refresh ();
//SetSelectMode (POINT_MODE);
}

//==========================================================================
// MENU - PreviousPoint
//==========================================================================

void CMineView::PrevPoint()
{
NextPoint (-1);
}

//==========================================================================
// MENU - NextSide
//==========================================================================
void CMineView::NextSide (int dir) 
{
wrap(&theMine->Current ()->nSide,dir,0,6-1);
Refresh (true);
//SetSelectMode (SIDE_MODE);
}

//==========================================================================
// MENU - PreviousSide
//==========================================================================
void CMineView::PrevSide () 
{
NextSide (-1);
}

//==========================================================================
// MENU - NextSide2 (same except doesn't change mode)
//==========================================================================
void CMineView::NextSide2 (int dir)
{
wrap(&theMine->Current ()->nSide,dir,0,6-1);
Refresh ();
}

void CMineView::PrevSide2 ()
{
NextSide2 (-1);
}

//==========================================================================
// MENU - NextLine
//==========================================================================

void CMineView::NextLine (int dir) 
{
wrap (&theMine->Current ()->nLine, dir, 0, 4-1);
theMine->Current ()->nPoint = theMine->Current ()->nLine;
Refresh ();
//SetSelectMode (LINE_MODE);
}

//==========================================================================
// MENU - PreviousLine
//==========================================================================

void CMineView::PrevLine () 
{
NextLine (-1);
}

//==========================================================================
// MENU - NextCube
//==========================================================================

void CMineView::NextCube (int dir) 
{
if (theMine->SegCount () <= 0)
	return;

if (0) {//!ViewOption (eViewPartialLines)) {
	DrawHighlight (1);
	//if (m_selectMode == CUBE_MODE)
		wrap (&theMine->Current ()->nSegment,dir,0, theMine->SegCount () - 1);
	Refresh (true);
	//SetSelectMode (CUBE_MODE);
	DrawHighlight (0);
	}
else {
	//if (m_selectMode == CUBE_MODE)
		wrap (&theMine->Current ()->nSegment, dir, 0, theMine->SegCount () - 1);
	Refresh (true);
	//SetSelectMode (CUBE_MODE);
	}
}

//==========================================================================
// MENU - PreviousCube
//==========================================================================

void CMineView::PrevCube () 
{
NextCube (-1);
}

//==========================================================================
// MENU - Forward_Cube
//
// ACTION - If child exists, this routine sets current_segment to child segP
//
// Changes - Smart side selection added (v0.8)
//         Smart side selection done before moving (instead of after) (v0.9)
//==========================================================================

void CMineView::ForwardCube (int dir) 
{
	CSegment *segP,*childSegP;
	short nChild, nSide;
	bool bFwd = (dir == 1);

DrawHighlight (1);
segP = theMine->Segments (theMine->Current ()->nSegment);
nChild = segP->Child (bFwd ? theMine->Current ()->nSide: oppSideTable [theMine->Current ()->nSide]);
if (nChild <= -1) {
	// first try to find a non backwards route
	for (nSide = 0; nSide < 6; nSide++) {
		if (segP->Child (nSide) != m_lastSegment && segP->Child (nSide) > -1) {
			nChild = segP->Child (nSide);
			theMine->Current ()->nSide =  bFwd ? nSide: oppSideTable [nSide];
			break;
			}
		}
	// then settle for any way out
	if (nSide == 6) {
		for (nSide = 0; nSide < 6; nSide++) {
			if (segP->Child (nSide) > -1) {
				nChild = segP->Child (nSide);
				theMine->Current ()->nSide = bFwd ? nSide: oppSideTable [nSide];
				break;
				}
			}			
		}
	}
if (nChild > -1) {
	childSegP = theMine->Segments (nChild);
// try to select side which is in same direction as current side
	for (nSide=0;nSide<6;nSide++) {
		if (childSegP->Child (nSide) == theMine->Current ()->nSegment) {
			theMine->Current ()->nSide =  bFwd ? oppSideTable [nSide]: nSide;
			break;
			}
		}
	m_lastSegment = theMine->Current ()->nSegment;
	if (0) {//!ViewOption (eViewPartialLines)) {
		// DrawHighlight (1);
		theMine->Current ()->nSegment = nChild;
		// DrawHighlight (0);
		} 
	else {
		theMine->Current ()->nSegment = nChild;
		Refresh (true);
		}
	}
DrawHighlight (0);
}

//==========================================================================
// MENU - Backwards_Cube
//==========================================================================

void CMineView::BackwardsCube () 
{
ForwardCube (-1);
}

//==========================================================================
// MENU - Other_Cube
//==========================================================================

void CMineView::SelectOtherCube () 
{
theMine->Current () = (theMine->Current () == &theMine->Current1 ()) ? &theMine->Current2 (): &theMine->Current1 ();
Refresh (true);
//DLE.ToolView ()->CubeTool ()->Refresh ();
}



bool CMineView::SelectOtherSide () 
{
short nOppSeg, nOppSide;

if ((theMine == null)->GetOppositeSide (nOppSeg, nOppSide))
	return false;

theMine->Current ()->nSegment = nOppSeg;
theMine->Current ()->nSide = nOppSide;
Refresh (true);
//DLE.ToolView ()->CubeTool ()->Refresh ();
return true;
}

//==========================================================================
// MENU - NextObject
//==========================================================================

void CMineView::NextObject (int dir) 
{
  short old_object = theMine->Current ()->nObject;
  short new_object = theMine->Current ()->nObject;

//  DrawHighlight (1);
if (theMine->MineInfo ().objects.count > 1) {
//	if (m_selectMode == OBJECT_MODE)
		wrap(&new_object,dir,0, (short)theMine->MineInfo ().objects.count - 1) ;
	Refresh (true);
	}
//SetSelectMode (OBJECT_MODE);
RefreshObject (old_object, new_object);
}

//==========================================================================
// MENU - PreviousObject
//==========================================================================

void CMineView::PrevObject() 
{
NextObject (-1);
}

//==========================================================================
// NextElement
//==========================================================================

void CMineView::NextCubeElement (int dir)
{
switch (m_selectMode) {
	case eSelectPoint:
		NextPoint (dir);
		break;
	case eSelectLine:
		NextLine (dir);
		break;
	default:
		NextSide (dir);
		break;
	}
}

                        /*--------------------------*/

void CMineView::PrevCubeElement ()
{
NextCubeElement (-1);
}

                        /*--------------------------*/


//eof