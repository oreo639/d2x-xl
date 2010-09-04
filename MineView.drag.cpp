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

                        /*--------------------------*/
                        
BOOL CMineView::UpdateDragPos (void)
{
if ((theMine == null)) return FALSE;

if ((m_mouseState != eMouseStateInitDrag) && (m_mouseState != eMouseStateDrag))
	return FALSE;

	short nVert = sideVertTable [current.m_nSide] [current.m_nPoint];
	short v = current.Segment ()->m_info.verts [nVert];
	short x = m_viewPoints [v].x;
	short y = m_viewPoints [v].y;

if (m_mouseState == eMouseStateInitDrag) {
	SetMouseState (eMouseStateDrag);
// SetCapture ();
	m_highlightPos.x = x;
	m_highlightPos.y = y;
	m_lastDragPos = m_highlightPos;
	}
HighlightDrag (nVert, x, y);
	
//InvalidateRect (null, TRUE);
return TRUE;
}

                        /*--------------------------*/
                        
void CMineView::HighlightDrag (short nVert, long x, long y) 
{
CHECKMINE;

m_pDC->SelectObject((HBRUSH) GetStockObject (NULL_BRUSH));
//m_pDC->SetROP2 (R2_NOT);
m_pDC->SetROP2 (R2_NOT);
m_pDC->Ellipse (x-4, y-4, x+4, y+4);

CRect	rc (x, y, x, y);
int i;
for (i = 0; i < 3; i++) {
	m_pDC->MoveTo (x, y);
	short nVert2 = connectPointTable [nVert] [i];
	short x2 = m_viewPoints [current.Segment ()->m_info.verts [nVert2]].x;
	short y2 = m_viewPoints [current.Segment ()->m_info.verts [nVert2]].y;
   m_pDC->LineTo (x2, y2);
	if (rc.left > x2)
		rc.left = x2;
	if (rc.right < x2)
		rc.right = x2;
	if (rc.top > y2)
		rc.top = y2;
	if (rc.bottom < y2)
		rc.bottom = y2;
	}
m_pDC->SetROP2 (R2_COPYPEN);
rc.InflateRect (4, 4);
InvalidateRect (rc, FALSE);
UpdateWindow ();
}

                        /*--------------------------*/
                        
BOOL CMineView::DrawDragPos (void)
{
if ((theMine == null)) return FALSE;

if (m_mouseState != eMouseStateDrag)
	return FALSE;
if (m_lastMousePos == m_lastDragPos)
	return FALSE;

	short nVert;
	short x, y;
	int i;

nVert = sideVertTable [current.m_nSide] [current.m_nPoint];

// unhighlight last point and lines drawing
HighlightDrag (nVert, m_lastDragPos.x, m_lastDragPos.y);

// highlight the new position
HighlightDrag (nVert, m_lastMousePos.x, m_lastMousePos.y);
m_lastDragPos = m_lastMousePos;

m_pDC->SetROP2 (R2_NOT);
for (i = 0; i < theMine->VertCount (); i++) {
	x = m_viewPoints [i].x;
	y = m_viewPoints [i].y;
	if ((abs (x - m_lastMousePos.x) < 5) && (abs (y - m_lastMousePos.y) < 5)) {
		if ((x != m_highlightPos.x) || (y != m_highlightPos.y)) {
			if (m_highlightPos.x != -1)
				// erase last point
				m_pDC->Ellipse (m_highlightPos.x-8, m_highlightPos.y-8, m_highlightPos.x+8, m_highlightPos.y+8);
			// define and draw new point
			m_highlightPos.x = x;
			m_highlightPos.y = y;
			m_pDC->Ellipse (m_highlightPos.x-8, m_highlightPos.y-8, m_highlightPos.x+8, m_highlightPos.y+8);
			break;
			}
		}
	}
// if no point found near cursor
if ((i >= theMine->VertCount ()) && (m_highlightPos.x != -1))
	// erase last point
	m_pDC->Ellipse (m_highlightPos.x-8, m_highlightPos.y-8, m_highlightPos.x+8, m_highlightPos.y+8);
m_pDC->SetROP2 (R2_COPYPEN);
// define and draw new point
m_highlightPos.x = -1;
m_highlightPos.y = -1;
return TRUE;
}

                        /*--------------------------*/
                        
void CMineView::FinishDrag (void)
{
CHECKMINE;

//ReleaseCapture ();
	int		m_changesMade = 1;
	int		i, new_vert, count = 0;
	long		xPos,yPos;
	short		xPoint,yPoint;
	short		point1,vert1;
	short		point2,vert2;

xPos = m_releasePos.x;
yPos = m_releasePos.y;
point1 = sideVertTable [current.m_nSide] [current.m_nPoint];
vert1 = theMine->Segments (0) [current.m_nSegment].m_info.verts [point1];
// find point to merge with
for (i = 0; i < theMine->VertCount (); i++) {
	xPoint = m_viewPoints [i].x;
	yPoint = m_viewPoints [i].y;
	if (abs(xPos - xPoint) < 5 && abs(yPos - yPoint)<5) {
		count++;
		new_vert = i;
		}
	}
// if too many matches found
if ((count > 1) && 
	 (QueryMsg("It is not clear which point you want to snap to."
				  "Do you want to attach these points anyway?") == IDYES))
	count = 1;
if (count == 1) {
// make sure new vert is not one of the current cube's verts
	for (i=0;i<8;i++) {
		if (i!=point1) {
			vert2 = theMine->Segments (0) [current.m_nSegment].m_info.verts [i];
			if (new_vert == vert2) {
				ErrorMsg ("Cannot drop point onto another corner of the current cube.");
				break;
				}
			}
		}
	if (i==8 && new_vert!=vert1) {
	// make sure the new line lengths are close enough
		for (i=0;i<3;i++) {
			point2 = connectPointTable [point1] [i];
			vert2 = theMine->Segments (0) [current.m_nSegment].m_info.verts [point2];
			if (Distance (*theMine->Vertices (new_vert), *theMine->Vertices (vert2)) >= 1000.0) {
				ErrorMsg ("Cannot move this point so far away.");
				break;
				}
			}
		if (i==3) { //
			// replace origional vertex with new vertex
			theMine->Segments () [current.m_nSegment].m_info.verts [point1] = new_vert;
			// all unused vertices
			theMine->DeleteUnusedVertices ();
			theMine->FixChildren ();
			segmentManager.SetLinesToDraw ();
			}
		}	
	}
else {
	// no vertex found, just drop point along screen axii
	APOINT apoint;
	apoint.x = (short) xPos;
	apoint.y = (short) yPos;
	apoint.z = m_viewPoints [vert1].z;
	m_view.Unproject (*theMine->Vertices (vert1), apoint);
	}
Refresh ();
}

                        /*--------------------------*/


//eof