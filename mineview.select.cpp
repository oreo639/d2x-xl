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
#include "io.h"

#include <math.h>
#include <time.h>

//==========================================================================
// MENU - NextPoint
//==========================================================================
void CMineView::NextPoint(INT32 dir) 
{
//if (!theMine->SplineActive ())
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
void CMineView::NextSide (INT32 dir) 
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
void CMineView::NextSide2 (INT32 dir)
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

void CMineView::NextLine (INT32 dir) 
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

void CMineView::NextCube (INT32 dir) 
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

void CMineView::ForwardCube (INT32 dir) 
{
	CSegment *segP,*childseg;
	INT16 child,nSide;
	bool bFwd = (dir == 1);

DrawHighlight (1);
segP = theMine->Segments (0) + theMine->Current ()->nSegment;
child = segP->m_info.children [bFwd ? theMine->Current ()->nSide: oppSideTable [theMine->Current ()->nSide]];
if (child <= -1) {
	// first try to find a non backwards route
	for (nSide = 0; nSide < 6; nSide++) {
		if (segP->m_info.children [nSide] != m_lastSegment && segP->m_info.children [nSide] > -1) {
			child = segP->m_info.children [nSide];
			theMine->Current ()->nSide =  bFwd ? nSide: oppSideTable [nSide];
			break;
			}
		}
	// then settle for any way out
	if (nSide == 6) {
		for (nSide = 0; nSide < 6; nSide++) {
			if (segP->m_info.children [nSide] > -1) {
				child = segP->m_info.children [nSide];
				theMine->Current ()->nSide = bFwd ? nSide: oppSideTable [nSide];
				break;
				}
			}			
		}
	}
if (child > -1) {
	childseg = theMine->Segments (0) + child;
// try to select side which is in same direction as current side
	for (nSide=0;nSide<6;nSide++) {
		if (childseg->m_info.children [nSide] == theMine->Current ()->nSegment) {
			theMine->Current ()->nSide =  bFwd ? oppSideTable [nSide]: nSide;
			break;
			}
		}
	m_lastSegment = theMine->Current ()->nSegment;
	if (0) {//!ViewOption (eViewPartialLines)) {
		// DrawHighlight (1);
		theMine->Current ()->nSegment = child;
		// DrawHighlight (0);
		} 
	else {
		theMine->Current ()->nSegment = child;
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
//theApp.ToolView ()->CubeTool ()->Refresh ();
}



bool CMineView::SelectOtherSide () 
{
INT16 nOppSeg, nOppSide;

if (!theMine->GetOppositeSide (nOppSeg, nOppSide))
	return false;

theMine->Current ()->nSegment = nOppSeg;
theMine->Current ()->nSide = nOppSide;
Refresh (true);
//theApp.ToolView ()->CubeTool ()->Refresh ();
return true;
}

//==========================================================================
// MENU - NextObject
//==========================================================================

void CMineView::NextObject (INT32 dir) 
{
  INT16 old_object = theMine->Current ()->nObject;
  INT16 new_object = theMine->Current ()->nObject;

//  DrawHighlight (1);
if (theMine->GameInfo ().objects.count > 1) {
//	if (m_selectMode == OBJECT_MODE)
		wrap(&new_object,dir,0, (INT16)theMine->GameInfo ().objects.count - 1) ;
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

void CMineView::NextCubeElement (INT32 dir)
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