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


                        /*--------------------------*/

void CMineView::SetViewPoints (CRect *pRC, bool bSetViewInfo)
{
if (!theMine) return;

	CRect		rc (LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	long		i, 
				x, y, z,
				minX = LONG_MAX, minY = LONG_MAX, minZ = LONG_MAX,
				maxX = LONG_MIN, maxY = LONG_MIN, maxZ = LONG_MIN;

#if 0 //OGL_RENDERING
m_view.Calculate (0,0,0); // let OpenGL do the translation
#else
m_view.Calculate (m_move.v.x, m_move.v.y, m_move.v.z);
#endif
InitViewDimensions ();
if (bSetViewInfo)
	m_view.SetViewInfo (m_depthPerception, m_viewWidth, m_viewHeight);
i = theMine->VertCount ();
APOINT *a = m_viewPoints + i;
CVertex* verts = theMine->Vertices (i);
for (; i--; ) {
	m_view.Project (*(--verts), *(--a));
	x = a->x;
	y = a->y;
	z = a->z;
	if (rc.left > x) {
		rc.left = x;
		m_minVPIdx.x = (INT16) i;
		}
	if (rc.right < x) {
		rc.right = x;
		m_maxVPIdx.x = (INT16) i;
		}
	if (rc.top > y) {
		rc.top = y;
		m_minVPIdx.y = (INT16) i;
		}
	if (rc.bottom < y) {
		rc.bottom = y;
		m_maxVPIdx.y = (INT16) i;
		}
	if (minZ > z) {
		minZ = z;
		m_minVPIdx.z = (INT16) i;
		}
	if (maxZ < z) {
		maxZ = z;
		m_maxVPIdx.z = (INT16) i;
		}
	}
#if OGL_RENDERING 
//flip mine over for OpenGL
for (i = theMine->VertCount (), a = m_viewPoints; i--; a++) {
	a->y = rc.top + rc.bottom - a->
		y;
	a->z = minZ + maxZ - a->z;
	}
#endif
x = rc.Width ();
y = rc.Height ();
if (pRC)
	*pRC = rc;
m_minViewPoint.x = (INT16) rc.left;
m_minViewPoint.y = (INT16) rc.bottom;
m_minViewPoint.z = (INT16) minZ;
m_maxViewPoint.x = (INT16) rc.right;
m_maxViewPoint.y = (INT16) rc.top;
m_maxViewPoint.z = (INT16) maxZ;
}

//----------------------------------------------------------------------------
// CalcSegDist
//----------------------------------------------------------------------------

void CMineView::CalcSegDist (void)
{
CHECKMINE;

	INT32			h, i, j, c, nDist, segNum = theMine->SegCount (), sideNum;
	CSegment	*segI, *segJ;

	static INT16 segRef [MAX_SEGMENTS3];

for (i = segNum, segI = theMine->Segments (0); i; i--, segI++)
	segI->m_info.nIndex = -1;
segRef [0] = theMine->Current ()->nSegment;	
theMine->CurrSeg ()->m_info.nIndex = 0;
i = 1;
h = j = 0;
for (nDist = 1; (j < segNum) && (h < i); nDist++) {
	for (h = i; j < h; j++) {
		segI = theMine->Segments (segRef [j]);
		for (sideNum = 0; sideNum < 6; sideNum++) {
			c = segI->Child (sideNum);
			if (c < 0) 
				continue;
			segJ = theMine->Segments (c);
			if (segJ->m_info.nIndex != -1)
				continue;
			segJ->m_info.nIndex = nDist;
			segRef [i++] = c;
			}
		}
	}
}

                        /*--------------------------*/

void CMineView::DrawMineCenter (CDC *pViewDC)
{
if (m_nMineCenter == 1) {
	m_pDC->SelectObject(GetStockObject (WHITE_PEN));
	m_pDC->MoveTo (x_center, y_center - (INT32) (10.0 * m_size.v.y) + 1);
	m_pDC->LineTo (x_center, y_center + (INT32) (10.0 * m_size.v.y) + 1);
	m_pDC->MoveTo (x_center - (INT32) (10.0 * m_size.v.x) + 1, y_center);
	m_pDC->LineTo (x_center + (INT32) (10.0 * m_size.v.x) + 1, y_center);
	}
else if (m_nMineCenter == 2) {
	// draw a globe
	// 5 circles around each axis at angles of 30, 60, 90, 120, and 150
	// each circle has 18 points
	CVertex circle;
	APOINT pt;

	m_pDC->SelectObject (m_penCyan);
	INT32 i, j;
	for (i = -60; i <= 60; i += 30) {
		for (j = 0; j <= 360; j += 15) {
			double scale = (5 * cos (Radians (i)));
			circle.Set (scale * cos (Radians (j)), scale * sin (Radians (j)), 5 * sin (Radians (i)));
			circle -= m_view.m_move [0];
			m_view.Project (circle, pt);
			if (j == 0)
				m_pDC->MoveTo (pt.x,pt.y);
			else if (pt.z <= 0)
				m_pDC->LineTo (pt.x,pt.y);
			else 
				m_pDC->MoveTo (pt.x,pt.y);
			}
		}
	m_pDC->SelectObject (m_penGreen);
	for (i = -60; i <= 60; i += 30) {
		for (j = 0; j <= 360; j += 15) {
			double scale = (5 * cos (Radians (i)));
			circle.Set (scale * cos (Radians (j)), 5 * sin (Radians (i)), scale * sin (Radians (j)));
			circle -= m_view.m_move [0];
			m_view.Project (circle, pt);
			if (j == 0)
				m_pDC->MoveTo (pt.x,pt.y);
			else if (pt.z <= 0)
				m_pDC->LineTo (pt.x,pt.y);
			else 
				m_pDC->MoveTo (pt.x,pt.y);
			}
		}
	m_pDC->SelectObject (m_penGray);
	for (i = -60; i <= 60; i += 30) {
		for (j = 0; j <= 360; j += 15) {
			double scale = (5 * cos (Radians (i)));
			circle.Set (5 * sin (Radians (i)), scale * cos (Radians (j)), scale * sin (Radians (j)));
			circle -= m_view.m_move [0];
			m_view.Project (circle, pt);
			if (j == 0)
				m_pDC->MoveTo (pt.x,pt.y);
			else if (pt.z <= 0)
				m_pDC->LineTo (pt.x,pt.y);
			else 
				m_pDC->MoveTo (pt.x,pt.y);
			}
		}
	}
}
//----------------------------------------------------------------------------
// DrawWireFrame
//----------------------------------------------------------------------------

void CMineView::DrawWireFrame (bool bPartial)
{
CHECKMINE;

	INT32			nSegment;
	CSegment	*segP;

CalcSegDist ();
m_pDC->SelectObject(m_penGray);
for (nSegment=0, segP = theMine->Segments (0);nSegment<theMine->SegCount ();nSegment++, segP++) {
	if (!Visible (segP))
		continue;
	DrawCube (segP, bPartial);
	if (nSegment == m_Current->nSegment) {
		DrawCurrentCube (segP, bPartial);
		m_pDC->SelectObject (m_penGray);
		}
	}
}

//----------------------------------------------------------------------------
// DrawTextureMappedCubes
//----------------------------------------------------------------------------

typedef struct tSegZOrder {
	INT32		zMax;
	INT16		iSeg;
} tSegZOrder;

typedef tSegZOrder *pSegZOrder;

static tSegZOrder szo [MAX_SEGMENTS3];

void QSortCubes (INT16 left, INT16 right)
{
	INT32		m = szo [(left + right) / 2].zMax;
	tSegZOrder	h;
	INT16	l = left, r = right;

do {
	while (szo [l].zMax > m)
		l++;
	while (szo [r].zMax < m)
		r--;
	if (l <= r) {
		if (l < r) {
			h = szo [l];
			szo [l] = szo [r];
			szo [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortCubes (l, right);
if (left < r)
	QSortCubes (left, r);
}

//----------------------------------------------------------------------------

void CMineView::DrawTextureMappedCubes (void)
{
CHECKMINE;

	UINT32 nSegment;
	INT16	 iVertex;
	INT32	 z, zMax;
	CSegment *segP;

	// Get shading table data
UINT8* light_index = 0;
if (m_viewMineFlags & eViewMineShading && (light_index = PalettePtr ()))
	light_index += 256*5; // skip 3-byte palette + 1st 2 light tables

// Draw Segments ()
for (nSegment = 0, segP = theMine->Segments (0); nSegment < theMine->SegCount (); nSegment++, segP++) {
	for (iVertex = 0, zMax = LONG_MIN; iVertex < MAX_VERTICES_PER_SEGMENT; iVertex++)
		if (zMax < (z = m_viewPoints [segP->m_info.verts [iVertex]].z))
			zMax = z;
	szo [nSegment].iSeg = nSegment;
	szo [nSegment].zMax = zMax;
	}
QSortCubes (0, theMine->SegCount () - 1);
CalcSegDist ();
for (nSegment = 0; nSegment < theMine->SegCount (); nSegment++) {
	segP = theMine->Segments (szo [nSegment].iSeg);
	if (Visible (segP))
	 	DrawCubeTextured (segP, light_index);
	}
}

//--------------------------------------------------------------------------
// DrawCube()
//--------------------------------------------------------------------------
#define IN_RANGE(value,absolute_range) ((-absolute_range <= value) && (value <= absolute_range))

bool CMineView::InRange (INT16 *pv, INT16 i)
{
	INT32	v;

for (; i; i--, pv++) {
	v = *pv;
	if (!(IN_RANGE (m_viewPoints [v].x, x_max) &&
			IN_RANGE (m_viewPoints [v].y, y_max)))
		return false;
	}
return true;
}

								/*-----------------------*/

void CMineView::DrawCube (CSegment *segP, bool bPartial)
{
DrawCubeQuick (segP, bPartial);
}

void CMineView::DrawCube (INT16 nSegment,INT16 nSide, INT16 linenum, INT16 pointnum, INT16 clear_it) 
{
CHECKMINE;

	CSegment *segP = theMine->Segments (nSegment);
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (!Visible (segP))
		return;

	// clear segment and point
	if (clear_it) {
		m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
		m_pDC->SelectObject (GetStockObject(BLACK_PEN)); // BLACK
		INT32 nVert = segP->m_info.verts [sideVertTable [nSide] [pointnum]];
		if (IN_RANGE (m_viewPoints [nVert].x,x_max) &&
			 IN_RANGE (m_viewPoints [nVert].y,y_max)) {
			m_pDC->Ellipse(m_viewPoints [nVert].x - 4,
			m_viewPoints [nVert].y - 4,
			m_viewPoints [nVert].x + 4,
			m_viewPoints [nVert].y + 4);
			}
		if (segP->m_info.wallFlags & MARKED_MASK) {
			m_pDC->SelectObject (m_penCyan);
			DrawCubeQuick (segP);
			} 
		else {
			if (m_viewOption == eViewPartialLines) {
				m_pDC->SelectObject (GetStockObject(BLACK_PEN));  // BLACK
				DrawCubeQuick (segP);   // clear all cube lines
				m_pDC->SelectObject (m_penGray); // GRAY
				DrawCubePartial (segP); // then redraw the ones we need
				}
			if ((m_viewOption == eViewAllLines) || 
				 (m_viewOption == eViewNearbyCubeLines) || 
				 (m_viewOption == eViewTextureMapped)) {
				m_pDC->SelectObject (m_penGray); // GRAY
				DrawCubeQuick (segP);
				}
			if (m_viewOption == eViewHideLines) {
				m_pDC->SelectObject (GetStockObject(BLACK_PEN));  // BLACK
				DrawCubeQuick (segP);   // clear all cube lines
				}
			}
			if (m_viewOption == eViewNearbyCubeLines) {
			m_pDC->SelectObject (GetStockObject(BLACK_PEN));  // BLACK
			DrawCubeQuick (segP);   // clear all cube lines
			m_pDC->SelectObject (GetStockObject(WHITE_PEN)); // WHITE
			DrawCubePoints (segP);  // then draw the points
			}
		} 
	else {
		if (segP->m_info.wallFlags & MARKED_MASK)
			m_pDC->SelectObject (m_penHiCyan);
		else if (nSegment == theMine->Current ()->nSegment)
			if (SelectMode (eSelectCube)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);         // RED
			else
				m_pDC->SelectObject (GetStockObject (WHITE_PEN)); // WHITE
			else
				m_pDC->SelectObject (m_penHiGray);        // LIGHT_GRAY
		if (m_viewOption == eViewPartialLines)
			DrawCubePartial (segP); // then redraw the ones we need
		else
			DrawCubeQuick (segP);
		}

	// draw current side
	// must draw in same order as segment to avoid leftover pixels on screen
	if (!clear_it) {
		if (nSegment == theMine->Current ()->nSegment)
			if (SelectMode (eSelectSide)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);        // RED
			else
				m_pDC->SelectObject (m_penHiGreen); // GREEN
		else
			m_pDC->SelectObject (m_penHiDkGreen);         // DARK_GREEN
		if (IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [0]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [0]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [1]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [1]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [2]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [2]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [3]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [3]]].y,y_max))
			 DrawLine (segP,sideVertTable [nSide] [0],sideVertTable [nSide] [1]);
			 DrawLine (segP,sideVertTable [nSide] [1],sideVertTable [nSide] [2]);
			 DrawLine (segP,sideVertTable [nSide] [2],sideVertTable [nSide] [3]);
			 DrawLine (segP,sideVertTable [nSide] [3],sideVertTable [nSide] [0]);

		// draw current line
		// must draw in same order as segment to avoid leftover pixels on screen
		if (nSegment == theMine->Current ()->nSegment)
			if (SelectMode (eSelectLine)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);  // RED
			else 
				m_pDC->SelectObject (m_penHiCyan);  // BLUE/CYAN
		else
			m_pDC->SelectObject (m_penDkCyan);  // BLUE/CYAN
		if (IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [0]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [0]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [1]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [1]]].y,y_max))
			DrawLine (segP,
						 lineVertTable [sideLineTable [nSide] [linenum]] [0],
						 lineVertTable [sideLineTable [nSide] [linenum]] [1]);
		}

	// draw a circle around the current point
	if (!clear_it) {
		m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
		if (nSegment == theMine->Current ()->nSegment)
			if (SelectMode (eSelectPoint)) //  && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed); // RED
			else
				m_pDC->SelectObject (m_penHiCyan); // CYAN
		else
			m_pDC->SelectObject (m_penHiDkCyan); // CYAN
		if (IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y,y_max))
			m_pDC->Ellipse(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x - 4,
								m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y - 4,
								m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x + 4,
								m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y + 4);
		}
}

//--------------------------------------------------------------------------
//			 draw_partial_segment()
//--------------------------------------------------------------------------

void CMineView::DrawCubePartial (CSegment *segP) 
{
CHECKMINE;

  INT16 line;
  INT16 vert0,vert1;

if (!Visible (segP))
	return;
for (line=0;line<12;line++) {
	if (segP->m_info.mapBitmask & (1<<line)) {
      if (IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [line] [0]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [line] [0]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [line] [1]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [line] [1]]].y,y_max)) {
			vert0 = lineVertTable [line] [0];
			vert1 = lineVertTable [line] [1];
			if (vert1>vert0) {
				m_pDC->MoveTo (m_viewPoints [segP->m_info.verts [vert0]].x, m_viewPoints [segP->m_info.verts [vert0]].y);
				m_pDC->LineTo (m_viewPoints [segP->m_info.verts [vert1]].x, m_viewPoints [segP->m_info.verts [vert1]].y);
				}
			else {
				m_pDC->MoveTo (m_viewPoints [segP->m_info.verts [vert1]].x, m_viewPoints [segP->m_info.verts [vert1]].y);
				m_pDC->LineTo (m_viewPoints [segP->m_info.verts [vert0]].x, m_viewPoints [segP->m_info.verts [vert0]].y);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
// DrawCube()
//--------------------------------------------------------------------------

void QSortLineRef (POINT *lineRef, INT16 left, INT16 right)
{
	INT32		m = lineRef [(left + right) / 2].y;
	INT16	l = left, r = right;
do {
	while (lineRef [l].y < m)
		l++;
	while (lineRef [r].y > m)
		r--;
	if (l <= r) {
		if (l < r) {
			POINT h = lineRef [l];
			lineRef [l] = lineRef [r];
			lineRef [r] = h;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortLineRef (lineRef, l, right);
if (left < r)
	QSortLineRef (lineRef, left, r);
}

//--------------------------------------------------------------------------

void CMineView::DrawCubeQuick	(CSegment *segP, bool bPartial)
{
CHECKMINE;

if (!Visible (segP))
	return;

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;
	INT32	chSegI, chSideI, chVertI, i, j, commonVerts;
	CSegment	*childP;
	INT16 *pv = segP->m_info.verts;

for (i = 0; i < 8; i++, pv++) {
	INT32	v = *pv;
	if (!(IN_RANGE (m_viewPoints [v].x, x_max) &&
			IN_RANGE (m_viewPoints [v].y, y_max)))
		return;
	}
if (bPartial) {
	UINT32 nSide;
	for (nSide=0; nSide<6; nSide++) {
		if (segP->Child (nSide) >= 0)
			continue;
		
		POINT	side [4], line [2], vert;
		for (i = 0; i < 4; i++) {
			side [i].x = m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [i]]].x; 
			side [i].y = m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [i]]].y; 
			}
		CDoubleVector a,b;
		a.v.x = (double) (side [1].x - side [0].x);
		a.v.y = (double) (side [1].y - side [0].y);
		b.v.x = (double) (side [3].x - side [0].x);
		b.v.y = (double) (side [3].y - side [0].y);
		if (a.v.x * b.v.y < a.v.y * b.v.x)
			m_pDC->SelectObject((HPEN)GetStockObject(WHITE_PEN));
		else
			m_pDC->SelectObject(m_penGray);
		// draw each line of the current side separately
		// only draw if there is no childP cube of the current cube with a common side
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 2; j++)
				line [j] = side [(i+j)%4];

			// check childP cubes
			commonVerts = 0;
			for (chSegI = 0; (chSegI < 6) && (commonVerts < 2); chSegI++) {
				if (segP->child (chSegI) < 0)
					continue;
				childP = theMine->Segments (segP->Child (chSegI));
				// walk through childP cube's sides
				commonVerts = 0;
				for (chSideI = 0; (chSideI < 6) && (commonVerts < 2); chSideI++) {
					// check current childP cube side for common line
					// i.e. check each line for common vertices with the parent line
					for (commonVerts = 0, chVertI = 0; (chVertI < 4) && (commonVerts < 2); chVertI++) {
						vert.x = m_viewPoints [childP->m_info.verts [sideVertTable [chSideI] [chVertI]]].x;
						vert.y = m_viewPoints [childP->m_info.verts [sideVertTable [chSideI] [chVertI]]].y;
						INT32 h;
						for (h = 0; h < 2; h++) {
							if ((line [h].x == vert.x) && (line [h].y == vert.y)) {
								++commonVerts;
								break;
								}
							}
						}
					}
				}
			if (commonVerts < 2)
				m_pDC->Polyline (line, 2);
			}
		}
	}
else {	//!bPartial
	POINT	lines [12][2];
	POINT lineRef [12];
/*
	static	INT32 poly1 [] = {4,0,1,2,3};
	static	INT32 poly2 [] = {3,0,3,7};
	static	INT32 poly3 [] = {5,0,4,5,6,7};
	static	INT32 poly4 [] = {2,4,7};
	static	INT32 poly5 [] = {2,2,6};
	static	INT32 poly6 [] = {2,1,5};
	static	INT32* polys [] = {poly1, poly2, poly3, poly4, poly5, poly6};
*/
	static	INT32 points [] = {0,1,1,2,2,3,3,0,0,4,4,5,5,6,6,7,7,4,3,7,2,6,1,5,-1};
	INT32		i, j, k, v, l;

	for (i = 0;; i++) {
		k = points [i];
		if (0 > k)
			break;
		v = segP->m_info.verts [k];
		l = i/2;
		j = i&1;
		if (!j)
			lineRef [l].y = LONG_MIN;
		lines [l][j].x = m_viewPoints [v].x;
		lines [l][j].y = m_viewPoints [v].y;
		lineRef [l].x = l;
		if (lineRef [l].y < m_viewPoints [v].z)
			lineRef [l].y = m_viewPoints [v].z;
		}
	QSortLineRef (lineRef, 0, 11);
	for (i = 0; i < 12; i++)
		m_pDC->Polyline (lines [lineRef [i].x], 2);
	}
}

//--------------------------------------------------------------------------
// draw_line()
//
// works for all glAngle
//--------------------------------------------------------------------------

void CMineView::DrawLine (CTexture *pTx, POINT pt0, POINT pt1, UINT8 color) 
{
CHECKMINE;

	INT32 i,x,y;
	INT32 dx = pt1.x - pt0.x;
	INT32 dy = pt1.y - pt0.y;

#if 1
	INT32 xInc, yInc;
	double scale;
	INT32 nStep = 0;

if (dx > 0)
	xInc = 1;
else {
	xInc = -1;
	dx = -dx;
	}
if (dy > 0)
	yInc = 1;
else {
	yInc = -1;
	dy = -dy;
	}
scale = pTx->Scale ();
xInc = (INT32) ((double) xInc * scale);
yInc = (INT32) ((double) yInc * scale);

x = pt0.x;
y = pt0.y;

#if 0	//most universal
INT32 xStep = 0, yStep = 0;
INT32 dd = (dx >= dy) ? dx: dy;
for (i = dd + 1; i; i--) {
	pTx->m_info.bmDataP [y*pTx->m_info.width+x] = color;
	yStep += dy;
	if (yStep >= dx) {
		y += yInc;
		yStep = dx ? yStep % dx: 0;
		}
	xStep += dx;
	if (xStep >= dy) {
		x += xInc;
		xStep = dy ? xStep % dy: 0;
		}
	}
#else //0; faster
if (dx >= dy) {
	for (i = dx + 1; i; i--, x += xInc) {
		pTx->m_info.bmDataP [y*pTx->m_info.width+x] = color;
		nStep += dy;
		if (nStep >= dx) {
			y += yInc;
			nStep -= dx;
			}
		}
	}
else {
	for (i = dy + 1; i; i--, y += yInc) {
		pTx->m_info.bmDataP [y*pTx->m_info.width+x] = color;
		nStep += dx;
		if (nStep >= dy) {
			x += xInc;
			nStep -= dy;
			}
		}
	}
#endif //0
#else //0
if (dx == 0) {
	x = pt0.x;
	if (dy>0)
		for (y=pt0.y;y<=pt1.y;y++)
			pTx->m_info.bmDataP [y*pTx->m_info.width+x] = color;
	else
		for (y=pt0.y;y>=pt1.y;y--)
			pTx->m_info.bmDataP [y*pTx->m_info.width+x] = color;
	return;
	}

if (dy == 0) {
	y = pt0.y;
	if (dx > 0)
		for (x=pt0.x;x<=pt1.x;x++)
			bitmapBuffer [y*64+x] = color;
	else
		for (x=pt0.x;x>=pt1.x;x--)
			bitmapBuffer [y*64+x] = color;
	return;
	}

if (dx > 0)
	if (dy > 0)
		for (y=pt0.y,x=pt0.x;y<=pt1.y,x<=pt1.x;y++,x++)
			bitmapBuffer [y*64+x] = color;
	else
		for (y=pt0.y,x=pt0.x;y>=pt1.y,x<=pt1.x;y--,x++)
			bitmapBuffer [y*64+x] = color;
else
	if (dy > 0)
		for (y=pt0.y,x=pt0.x;y<=pt1.y,x>=pt1.x;y++,x--)
			bitmapBuffer [y*64+x] = color;
	else
		for (y=pt0.y,x=pt0.x;y>=pt1.y,x>=pt1.x;y--,x--)
			bitmapBuffer [y*64+x] = color;
#endif //0
}

//--------------------------------------------------------------------------
// DrawAnimDirArrows()
//--------------------------------------------------------------------------

void CMineView::DrawAnimDirArrows (INT16 texture1, CTexture *pTx)
{
	INT32 sx,sy;
	INT32 bScroll = theMine->ScrollSpeed (texture1, &sx, &sy);

if (!bScroll)
	return;

	POINT *pt;
	static POINT ptp0 [4] = {{54,32},{12,32},{42,42},{42,22}};
	static POINT pt0n [4] = {{32,12},{32,54},{42,22},{22,22}};
	static POINT ptn0 [4] = {{12,32},{54,32},{22,22},{22,42}};
	static POINT pt0p [4] = {{32,54},{32,12},{22,42},{42,42}};
	static POINT ptpn [4] = {{54,12},{12,54},{54,22},{42,12}};
	static POINT ptnn [4] = {{12,12},{54,54},{22,12},{12,22}};
	static POINT ptnp [4] = {{12,54},{54,12},{12,42},{22,54}};
	static POINT ptpp [4] = {{54,54},{12,12},{42,54},{54,42}};

if (sx >0 && sy==0) pt = ptp0;
else if (sx >0 && sy >0) pt = ptpp;
else if (sx==0 && sy >0) pt = pt0p;
else if (sx <0 && sy >0) pt = ptnp;
else if (sx <0 && sy==0) pt = ptn0;
else if (sx <0 && sy <0) pt = ptnn;
else if (sx==0 && sy <0) pt = pt0n;
else if (sx >0 && sy <0) pt = ptpn;

DrawLine (pTx, pt [0], pt [1], 1);
DrawLine (pTx, pt [0], pt [2], 1);
DrawLine (pTx, pt [0], pt [3], 1);
}

//--------------------------------------------------------------------------
// DrawCubeTextured()
//--------------------------------------------------------------------------

void CMineView::DrawCubeTextured(CSegment *segP, UINT8* light_index) 
{
CHECKMINE;

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (IN_RANGE(m_viewPoints [segP->m_info.verts [0]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [0]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [1]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [1]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [2]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [2]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [3]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [3]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [4]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [4]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [5]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [5]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [6]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [6]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [7]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [7]].y,y_max)   )
	{

		CTexture tex (bmBuf);
		UINT8 *pm_viewPointsMem = (UINT8 *)m_pvBits;
		UINT16 width = m_viewWidth;
		UINT16 height = m_viewHeight;
		UINT16 rowOffset = (m_viewWidth + 3) & ~3;
		UINT16 nSide = 5;
		CWall *wallP;
		UINT16 nWall = NO_WALL;

		for (nSide = 0; nSide < 6; nSide++) {
			wallP = ((nWall = segP->m_sides [nSide].m_info.nWall) == NO_WALL) ? NULL : theMine->Walls () + nWall;
			if ((segP->Child (nSide) == -1) ||
				(wallP && (wallP->m_info.type != WALL_OPEN) && ((wallP->m_info.type != WALL_CLOAKED) || wallP->m_info.cloakValue))
				)
			{
				APOINT& p0 = m_viewPoints [segP->m_info.verts [sideVertTable [nSide][0]]];
				APOINT& p1 = m_viewPoints [segP->m_info.verts [sideVertTable [nSide][1]]];
				APOINT& p3 = m_viewPoints [segP->m_info.verts [sideVertTable [nSide][3]]];

				CDoubleVector a,b;
				a.v.x = (double) (p1.x - p0.x);
				a.v.y = (double) (p1.y - p0.y);
				b.v.x = (double) (p3.x - p0.x);
				b.v.y = (double) (p3.y - p0.y);
				if (a.v.x * b.v.y > a.v.y * b.v.x) {
					INT16 texture1 = segP->m_sides [nSide].m_info.nBaseTex;
					INT16 texture2 = segP->m_sides [nSide].m_info.nOvlTex;
					if (!DefineTexture (texture1, texture2, &tex, 0, 0)) {
						DrawAnimDirArrows (texture1, &tex);
						TextureMap (segP, nSide, tex.m_info.bmDataP, tex.m_info.width, tex.m_info.height, 
									   light_index, pm_viewPointsMem, m_viewPoints, width, height, rowOffset);
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
//                        draw_segmentPoints()
//--------------------------------------------------------------------------

void CMineView::DrawCubePoints (CSegment *segP)
{
CHECKMINE;

	INT16		*pv = segP->m_info.verts;
	COLORREF	color = RGB (128,128,128);
	INT32		h, i;

for (i = 0; i < 8; i++, pv++) {
	h = *pv;
	m_pDC->SetPixel (m_viewPoints [h].x, m_viewPoints [h].y, color);
	}
#if 0
pDC->SetPixel (m_viewPoints [segP.verts [0]].x, m_viewPoints [segP.verts [0]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [segP.verts [1]].x, m_viewPoints [segP.verts [1]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [segP.verts [2]].x, m_viewPoints [segP.verts [2]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [segP.verts [3]].x, m_viewPoints [segP.verts [3]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [segP.verts [4]].x, m_viewPoints [segP.verts [4]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [segP.verts [5]].x, m_viewPoints [segP.verts [5]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [segP.verts [6]].x, m_viewPoints [segP.verts [6]].y,RGB(128,128,128));
pDC->SetPixel (m_viewPoints [segP.verts [7]].x, m_viewPoints [segP.verts [7]].y,RGB(128,128,128));
#endif
}

//--------------------------------------------------------------------------
//			draw_marked_segments()
//--------------------------------------------------------------------------

void CMineView::DrawMarkedCubes (INT16 clear_it) 
{
CHECKMINE;

	CSegment	*segP;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;
	INT16 i;

	// draw marked/special Segments () and Walls ()
if (!clear_it) {
	for (i = 0; i < theMine->SegCount (); i++) {
		segP = theMine->Segments (i);
		if (segP->m_info.wallFlags & MARKED_MASK) {
			m_pDC->SelectObject (SelectMode (eSelectBlock) ? m_penRed: m_penCyan);
			DrawCubeQuick (segP);
			}
		else {
			//    if (show_special) {
			if (ViewFlag (eViewMineSpecial) && !(m_viewOption == eViewTextureMapped) ) {
				switch(segP->m_info.function) {
				case SEGMENT_FUNC_FUELCEN:
				case SEGMENT_FUNC_SPEEDBOOST:
					m_pDC->SelectObject (m_penYellow);
					DrawCubeQuick (segP);
					break;
				case SEGMENT_FUNC_CONTROLCEN:
					m_pDC->SelectObject (m_penOrange);
					DrawCubeQuick (segP);
					break;
				case SEGMENT_FUNC_REPAIRCEN:
					m_pDC->SelectObject (m_penLtBlue);
					DrawCubeQuick (segP);
					break;
				case SEGMENT_FUNC_ROBOTMAKER:
				case SEGMENT_FUNC_EQUIPMAKER:
					m_pDC->SelectObject (m_penMagenta);
					DrawCubeQuick (segP);
					break;
				case SEGMENT_FUNC_GOAL_BLUE:
				case SEGMENT_FUNC_TEAM_BLUE:
					m_pDC->SelectObject (m_penBlue);
					DrawCubeQuick (segP);
					break;
				case SEGMENT_FUNC_GOAL_RED:
				case SEGMENT_FUNC_TEAM_RED:
					m_pDC->SelectObject (m_penRed);
					DrawCubeQuick (segP);
					break;
				default:
					if (segP->m_info.props & SEGMENT_PROP_WATER)
						m_pDC->SelectObject (m_penMedBlue);
					else if (segP->m_info.props & SEGMENT_PROP_LAVA)
						m_pDC->SelectObject (m_penMedRed);
					else
						break;
					DrawCubeQuick (segP);
					break;
					}
				}
			}
		}
	}

// draw a square around all marked points
m_pDC->SelectObject((HBRUSH)GetStockObject(NULL_BRUSH));
if (clear_it)
	m_pDC->SelectObject(GetStockObject(BLACK_PEN));
else if (SelectMode (eSelectBlock)) // && edit_mode != EDIT_OFF) {
	m_pDC->SelectObject (m_penRed);
else
	m_pDC->SelectObject (m_penCyan);
for (i=0;i<theMine->VertCount ();i++)
	if (theMine->VertStatus (i) & MARKED_MASK)
		if (IN_RANGE(m_viewPoints [i].x,x_max) && IN_RANGE(m_viewPoints [i].y,y_max))
			m_pDC->Rectangle(m_viewPoints [i].x - 4, m_viewPoints [i].y - 4, m_viewPoints [i].x + 4, m_viewPoints [i].y + 4);
}

//--------------------------------------------------------------------------
// DrawCurrentCube()
//--------------------------------------------------------------------------

void CMineView::DrawCurrentCube(CSegment *segP, bool bPartial)
{
CHECKMINE;

	INT16 nSide = m_Current->nSide;
	INT16 linenum = m_Current->nPoint;
	INT16 pointnum = m_Current->nPoint;

	if (segP->m_info.wallFlags & MARKED_MASK) {
		m_pDC->SelectObject(m_penCyan);
	}
	else
	{
		if (m_selectMode == CUBE_MODE)
		{
			m_pDC->SelectObject(m_penRed);
		} else {
			m_pDC->SelectObject(GetStockObject(WHITE_PEN));
		}
	}

	// draw current side
	// must draw in same order as segment to avoid leftover pixels on screen
	if (m_selectMode == SIDE_MODE)
		m_pDC->SelectObject(m_penRed);
	else
		m_pDC->SelectObject(m_penGreen);

// Select this pen if this is the "other current" cube
//	m_pDC->SelectObject(m_penDkGreen);

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [0]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [0]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [1]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [1]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [2]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [2]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [3]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [3]]].y,y_max)) {

		DrawLine(segP, sideVertTable [nSide] [0], sideVertTable [nSide] [1]);
		DrawLine(segP, sideVertTable [nSide] [1], sideVertTable [nSide] [2]);
		DrawLine(segP, sideVertTable [nSide] [2], sideVertTable [nSide] [3]);
		DrawLine(segP, sideVertTable [nSide] [3], sideVertTable [nSide] [0]);
	}

	// draw current line
	// must draw in same order as segment to avoid leftover pixels on screen
	if (m_selectMode == LINE_MODE) { // && edit_mode != EDIT_OFF) {
		m_pDC->SelectObject(m_penRed);  // RED
	} else {
		m_pDC->SelectObject(m_penCyan);  // BLUE/CYAN
	}

	if (IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [0]]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [0]]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [1]]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [1]]].y,y_max)   ) {

		DrawLine(segP, lineVertTable [sideLineTable [nSide] [linenum]] [0],
			lineVertTable [sideLineTable [nSide] [linenum]] [1]);
	}

	// draw a circle around the current point
	m_pDC->SelectObject((HBRUSH)GetStockObject(NULL_BRUSH));
	if (m_selectMode == POINT_MODE) { //  && edit_mode != EDIT_OFF) {
		m_pDC->SelectObject(m_penRed); // RED
	} else {
		m_pDC->SelectObject(m_penCyan); // CYAN
	}

	if (IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y,y_max)     ) {

		m_pDC->Ellipse(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x - 4,
			m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y - 4,
			m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x + 4,
			m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y + 4);
	}
}

//--------------------------------------------------------------------------
// DrawLine ()
//
// Action - draws a line starting with lowest vert
//--------------------------------------------------------------------------

void CMineView::DrawLine(CSegment *segP,INT16 vert1,INT16 vert2) 
{
CHECKMINE;
if (vert2 > vert1) {
	m_pDC->MoveTo(m_viewPoints [segP->m_info.verts [vert1]].x, m_viewPoints [segP->m_info.verts [vert1]].y);
	m_pDC->LineTo(m_viewPoints [segP->m_info.verts [vert2]].x, m_viewPoints [segP->m_info.verts [vert2]].y);
	} 
else {
	m_pDC->MoveTo(m_viewPoints [segP->m_info.verts [vert2]].x, m_viewPoints [segP->m_info.verts [vert2]].y);
	m_pDC->LineTo(m_viewPoints [segP->m_info.verts [vert1]].x, m_viewPoints [segP->m_info.verts [vert1]].y);
	}
}
//--------------------------------------------------------------------------
// DrawWalls()
//--------------------------------------------------------------------------

void CMineView::DrawWalls(void) 
{
CHECKMINE;

	CWall		*walls = theMine->Walls (0);
	CSegment	*segments = theMine->Segments (0);
	CVertex	*vertices = theMine->Vertices (0);
	CSegment	*segP;
	INT16 i,j;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

for (i=0;i<theMine->GameInfo ().walls.count;i++) {
	if (walls [i].m_nSegment > theMine->SegCount ())
		continue;
	segP = segments + (INT32)walls [i].m_nSegment;
	if (!Visible (segP))
		continue;
	switch (walls [i].m_info.type) {
		case WALL_NORMAL:
			m_pDC->SelectObject(m_penLtGray);
			break;
		case WALL_BLASTABLE:
			m_pDC->SelectObject(m_penLtGray);
			break;
		case WALL_DOOR:
			switch(walls [i].m_info.keys) {
				case KEY_NONE:
					m_pDC->SelectObject(m_penLtGray);
					break;
				case KEY_BLUE:
					m_pDC->SelectObject(m_penBlue);
					break;
				case KEY_RED:
					m_pDC->SelectObject(m_penRed);
					break;
				case KEY_GOLD:
					m_pDC->SelectObject(m_penYellow);
					break;
				default:
					m_pDC->SelectObject(m_penGray);
				}
			break;
		case WALL_ILLUSION:
			m_pDC->SelectObject(m_penLtGray);
			break;
		case WALL_OPEN:
			m_pDC->SelectObject(m_penLtGray);
			break;
		case WALL_CLOSED:
			m_pDC->SelectObject(m_penLtGray);
			break;
		default:
			m_pDC->SelectObject(m_penLtGray);
		}
	j = walls [i].m_nSide;
	if (IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [j][0]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [j][0]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [j][1]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [j][1]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [j][2]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [j][2]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [j][3]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->m_info.verts [sideVertTable [j][3]]].y,y_max)) {

			CVertex	center, orthog, vector;
			APOINT	point;

		center = theMine->CalcSideCenter (walls [i].m_nSegment, walls [i].m_nSide);
		orthog = theMine->CalcSideNormal (walls [i].m_nSegment, walls [i].m_nSide);
		vector = center - orthog;
		m_view.Project (vector, point);
		for (j = 0; j < 4; j++) {
			m_pDC->MoveTo (point.x,point.y);
			m_pDC->LineTo (m_viewPoints [segP->m_info.verts [sideVertTable [walls [i].m_nSide] [j]]].x,
			m_viewPoints [segP->m_info.verts [sideVertTable [walls [i].m_nSide] [j]]].y);
			}
		if (walls [i].m_info.nTrigger != NO_TRIGGER) {
				APOINT arrowStartPoint,arrowEndPoint,arrow1Point,arrow2Point;
				CVertex fin;

			// calculate arrow points
			vector = center - (orthog * 3);
			m_view.Project (vector, arrowStartPoint);
			vector = center + (orthog * 3);
			m_view.Project (vector, arrowEndPoint);

			// direction toward center of line 0 from center
			UINT8 *svp = &sideVertTable [walls [i].m_nSide][0];
			vector = Average (vertices [segP->m_info.verts [svp [0]]], vertices [segP->m_info.verts [svp [1]]]);
			vector -= center;
			vector.Normalize ();

			fin = (orthog * 2);
			fin += center;
			fin += vector;
			m_view.Project (fin, arrow1Point);
			fin -= vector * 2;
			m_view.Project (fin, arrow2Point);

			// draw arrow
			m_pDC->MoveTo (arrowStartPoint.x, arrowStartPoint.y);
			m_pDC->LineTo (arrowEndPoint.x, arrowEndPoint.y);
			m_pDC->LineTo (arrow1Point.x, arrow1Point.y);
			m_pDC->MoveTo (arrowEndPoint.x, arrowEndPoint.y);
			m_pDC->LineTo (arrow2Point.x, arrow2Point.y);
			}
		}
	}
}
//--------------------------------------------------------------------------
//			  draw_lights()
//--------------------------------------------------------------------------

void CMineView::DrawLights (void) 
{
CHECKMINE;

if (!m_pDC) return;

  // now show flickering lights
  m_pDC->SelectObject(m_penYellow);

  // find flickering light from
CFlickeringLight* flP = theMine->FlickeringLights (0);
for (INT i = 0; i < theMine->FlickerLightCount (); i++, flP++)
	if (Visible (theMine->Segments (flP->m_nSegment)))
	   DrawOctagon(flP->m_nSide, flP->m_nSegment);
}

//------------------------------------------------------------------------
// DrawOctagon()
//------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMineView::DrawOctagon(INT16 nSide, INT16 nSegment) 
{
CHECKMINE;

	CSegment *segP;
	INT16 j;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

if (nSegment >=0 && nSegment <=theMine->SegCount () && nSide>=0 && nSide<=5 ) {
	POINT corners [4],center,line_centers [4],diamond [4],fortyfive [4];
	segP = theMine->Segments (0) + nSegment;
	for (j=0;j<4;j++) {
		corners [j].x = m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [j]]].x;
		corners [j].y = m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [j]]].y;
		}
	if (IN_RANGE(corners [0].x,x_max) && IN_RANGE(corners [0].y,y_max) &&
		 IN_RANGE(corners [1].x,x_max) && IN_RANGE(corners [1].y,y_max) &&
		 IN_RANGE(corners [2].x,x_max) && IN_RANGE(corners [2].y,y_max) &&
		 IN_RANGE(corners [3].x,x_max) && IN_RANGE(corners [3].y,y_max)) {
		center.x = (corners [0].x + corners [1].x + corners [2].x + corners [3].x)>>2;
		center.y = (corners [0].y + corners [1].y + corners [2].y + corners [3].y)>>2;
		for (j = 0; j < 4; j++) {
			INT32 k = (j+1) & 0x03;
			line_centers [j].x = (corners [j].x + corners [k].x) >> 1;
			line_centers [j].y = (corners [j].y + corners [k].y) >> 1;
			diamond [j].x = (line_centers [j].x + center.x) >> 1;
			diamond [j].y = (line_centers [j].y + center.y) >> 1;
			fortyfive [j].x = ((corners [j].x-center.x)*7)/20 + center.x;
			fortyfive [j].y = ((corners [j].y-center.y)*7)/20 + center.y;
			}
		// draw octagon
		m_pDC->MoveTo(diamond [3].x,diamond [3].y);
		for (j = 0; j < 4; j++) {
			m_pDC->LineTo(fortyfive [j].x,fortyfive [j].y);
			m_pDC->LineTo(diamond [j].x,diamond [j].y);
			}
		}
	}
}

//----------------------------------------------------------------------------
// draw_spline()
//----------------------------------------------------------------------------

void CMineView::DrawSpline (void) 
{
	INT32 h, i, j;

//  SelectObject(hdc, hrgnAll);
m_pDC->SelectObject (m_penRed);
m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
theMine->CalcSpline ();
APOINT point;
m_view.Project (points [1], point);
if (IN_RANGE(point.x, x_max) && IN_RANGE(point.y, y_max)){
	m_view.Project (points [0], point);
	if (IN_RANGE(point.x, x_max) && IN_RANGE(point.y, y_max)){
		m_pDC->MoveTo (point.x, point.y);
		m_view.Project (points [1], point);
		m_pDC->LineTo (point.x, point.y);
		m_pDC->Ellipse (point.x - 4, point.y - 4, point.x+4,  point.y+4);
		}
	}
m_view.Project (points [2], point);
if (IN_RANGE(point.x, x_max) && IN_RANGE(point.y, y_max)){
	m_view.Project (points [3], point);
	if (IN_RANGE(point.x, x_max) && IN_RANGE(point.y, y_max)){
		m_pDC->MoveTo (point.x, point.y);
		m_view.Project (points [2], point);
		m_pDC->LineTo (point.x, point.y);
		m_pDC->Ellipse (point.x - 4, point.y - 4, point.x+4,  point.y+4);
		}
	}
m_pDC->SelectObject (m_penBlue);
j = MAX_VERTICES;
for (h = n_splines * 4, i = 0; i < h; i++)
	m_view.Project (*theMine->Vertices (--j), m_viewPoints [j]);
CSegment *segP = theMine->Segments (MAX_SEGMENTS - 1);
for (i = 0; i < n_splines; i++, segP--)
	DrawCubeQuick (segP);
}

//--------------------------------------------------------------------------
//			  DrawObject()
//
// Changed: 0=normal,1=gray,2=black
//        if (objnum == (GameInfo ().objects.count
//        then its a secret return point)
//--------------------------------------------------------------------------

void TransformModelPoint (CVertex& dest, APOINT &src, CDoubleMatrix &orient, CVertex offs)
{
CDoubleVector v (src.x, src.y, src.z);
dest = orient * v;
dest += offs;
}


void CMineView::DrawObject(INT16 objnum,INT16 clear_it) 
{
CHECKMINE;

	INT16				poly;
	CGameObject*	objP;
	CVertex			pt [MAX_POLY];
	APOINT			poly_draw [MAX_POLY];
	APOINT			object_shape [MAX_POLY] = {
		{ 0,  4, -4},
		{ 0,  0, -4},
		{ 0,  0,  4},
		{-2,  0,  2},
		{ 2,  0,  2},
		{ 0,  0,  4}
		};
	CGameObject temp_obj;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

//  m_pDC->SelectObject(hrgnBackground);
if (objnum >=0 && objnum < theMine->GameInfo ().objects.count) {
	objP = theMine->Objects (objnum);
	if (!Visible (theMine->Segments (objP->m_info.nSegment)))
		return;
	}
else {
	// secret return
	objP = &temp_obj;
	objP->m_info.type = -1;
	// theMine->secret_orient = Objects () [0]->orient;
	objP->m_location.orient.rVec = -theMine->SecretOrient ().rVec;
	objP->m_location.orient.uVec =  theMine->SecretOrient ().fVec;
	objP->m_location.orient.fVec =  theMine->SecretOrient ().uVec;
	// objP->m_location.orient =  theMine->secret_orient;
	UINT16 nSegment = (UINT16)theMine->SecretCubeNum ();
	if (nSegment >= theMine->SegCount ())
		nSegment = 0;
	if (!Visible (theMine->Segments (nSegment)))
		return;
	theMine->CalcSegCenter (objP->m_location.pos, nSegment); // define objP->position
	}

switch (clear_it) {
	case 0: // normal
	case 1: // gray
		if (m_selectMode == OBJECT_MODE && objnum == theMine->Current ()->nObject) 
			m_pDC->SelectObject(m_penRed); // RED
		else {
			switch(objP->m_info.type) {
				case OBJ_ROBOT: /* an evil enemy */
				case OBJ_CAMBOT: /* an evil enemy */
				case OBJ_EXPLOSION:
				case OBJ_MONSTERBALL:
					m_pDC->SelectObject(m_penMagenta);
					break;
				case OBJ_SMOKE:
				case OBJ_EFFECT:
					m_pDC->SelectObject(m_penHiGreen);
					break;
				case OBJ_HOSTAGE: /* a hostage you need to rescue */
					m_pDC->SelectObject(m_penBlue);
					break;
				case OBJ_PLAYER: /* the player on the console */
					m_pDC->SelectObject(m_penCyan);
					break;
				case OBJ_WEAPON: // exploding mine
					m_pDC->SelectObject(m_penDkGreen);
					break;
				case OBJ_POWERUP: /* a powerup you can pick up */
					m_pDC->SelectObject(m_penOrange);
					break;
				case OBJ_CNTRLCEN: /* the control center */
					m_pDC->SelectObject(m_penLtGray);
					break;
				case OBJ_COOP: /* a cooperative player object */
					m_pDC->SelectObject(m_penCyan);
					break;
				default:
					m_pDC->SelectObject(m_penGreen);
				}
			}
		break;
	case 2: // black
		m_pDC->SelectObject(GetStockObject(BLACK_PEN));
		break;
	}

// rotate object shape using object's orient matrix
// then translate object
//CBRK (objnum == 45);
for (poly = 0; poly < MAX_POLY; poly++) {
	::TransformModelPoint (pt [poly], object_shape [poly], objP->m_location.orient, objP->m_location.pos);
	m_view.Project (pt [poly], poly_draw [poly]);
	}

// figure out world coordinates

INT32 i;
for (i = 0; i < 6; i++)
	if (!(IN_RANGE (poly_draw [i].x, x_max) &&
			IN_RANGE (poly_draw [i].y, y_max)))
		return;

if ((theApp.IsD2File ()) &&
	 (objnum == theMine->Current ()->nObject) &&
	 (objP->m_info.type != OBJ_CAMBOT) && (objP->m_info.type != OBJ_MONSTERBALL) && 
	 (objP->m_info.type != OBJ_EXPLOSION) && (objP->m_info.type != OBJ_SMOKE) && (objP->m_info.type != OBJ_EFFECT) &&
	 (objP->m_info.renderType == RT_POLYOBJ) &&
	 !SetupModel(objP)) {
	if (clear_it)
		m_pDC->SelectObject(GetStockObject(BLACK_PEN));
	m_pDC->SelectObject((HBRUSH)GetStockObject(BLACK_BRUSH));
	DrawModel();
	}
else {
	m_pDC->MoveTo (poly_draw [0].x,poly_draw [0].y);
	for (poly = 0; poly < 6; poly++)
		m_pDC->LineTo (poly_draw [poly].x, poly_draw [poly].y);
	if (objnum == theMine->Current ()->nObject) {
		INT32 dx,dy;
		for (dx = -1; dx < 2; dx++) {
			for (dy = -1; dy < 2; dy++) {
				m_pDC->MoveTo (poly_draw [0].x+dx,poly_draw [0].y+dy);
				for (poly = 0; poly < 6; poly++)
					m_pDC->LineTo (poly_draw [poly].x + dx, poly_draw [poly].y + dy);
				}
			}
		}
	}
if ((objnum == theMine->Current ()->nObject) || (objnum == theMine->Other ()->nObject)) {
	CPen     pen, *pOldPen;
	INT32		d;

	pt [0] =
	pt [1] =
	pt [2] = objP->m_location.pos;
	pt [1].v.x -= objP->m_info.size;
	pt [2].v.x += objP->m_info.size;
	m_view.Project (pt [0], poly_draw [0]);
	m_view.Push ();
	m_view.Unrotate ();
	m_view.Project (pt [1], poly_draw [1]);
	m_view.Project (pt [2], poly_draw [2]);
	m_view.Pop ();
	d = (poly_draw [2].x - poly_draw [1].x);
	if (d < 24)
		d = 24;
	pen.CreatePen (PS_SOLID, 2, (objnum == theMine->Current ()->nObject) ? RGB (255,0,0) : RGB (255,208,0));
	pOldPen = m_pDC->SelectObject (&pen);
	m_pDC->SelectObject ((HBRUSH)GetStockObject(HOLLOW_BRUSH));
	m_pDC->Ellipse (poly_draw [0].x - d, poly_draw [0].y - d, poly_draw [0].x + d, poly_draw [0].y + d);
	m_pDC->SelectObject (pOldPen);
	}
}

//--------------------------------------------------------------------------
//			  DrawObjects()
//--------------------------------------------------------------------------

void CMineView::DrawObjects (INT16 clear_it) 
{
CHECKMINE;

if (!ViewObject ())
	return;

INT32 i, j;
if (theApp.IsD2File ()) {
	// see if there is a secret exit trigger
	for(i = 0; i < theMine->GameInfo ().triggers.count; i++)
	if (theMine->Triggers (i)->m_info.type == TT_SECRET_EXIT) {
		DrawObject ((INT16)theMine->GameInfo ().objects.count, 0);
		break; // only draw one secret exit
		}
	}
HiliteTarget ();
CGameObject *objP = theMine->Objects (0);
for (i = theMine->GameInfo ().objects.count, j = 0; i; i--, j++, objP++)
	if (ViewObject (objP))
		DrawObject (j, 0);
}

//--------------------------------------------------------------------------
//			  draw_highlight()
//--------------------------------------------------------------------------

void CMineView::DrawHighlight(INT16 clear_it) 
{
CHECKMINE;

	INT16	currSide, currPoint;
//	INT16 i;
//	RECT rect;

if (theMine->SegCount ()==0) 
	return;

// draw Objects ()
if (!clear_it) {
	DrawObjects (clear_it);
//	if (/*!(preferences & PREFS_HIDE_MARKED_BLOCKS) ||*/ SelectMode (eSelectBlock))
	DrawMarkedCubes(clear_it);
  }

// draw highlighted Segments () (other first, then current)
if (theMine->Current () == &theMine->Current1 ()) {
	if (theMine->Current1 ().nSegment != theMine->Current2 ().nSegment)
		DrawCube (theMine->Current2 ().nSegment, theMine->Current2 ().nSide, theMine->Current2 ().nLine, theMine->Current2 ().nPoint,clear_it);
	DrawCube (theMine->Current1 ().nSegment, theMine->Current1 ().nSide, theMine->Current1 ().nLine, theMine->Current1 ().nPoint,clear_it);
	}
else {
	if (theMine->Current1 ().nSegment != theMine->Current2 ().nSegment)
		DrawCube (theMine->Current1 ().nSegment, theMine->Current1 ().nSide, theMine->Current1 ().nLine, theMine->Current1 ().nPoint,clear_it);
	DrawCube (theMine->Current2 ().nSegment, theMine->Current2 ().nSide, theMine->Current2 ().nLine, theMine->Current2 ().nPoint,clear_it);
	}

// draw Walls ()
if (ViewFlag (eViewMineWalls))
	DrawWalls ();

// draw lights
if (ViewFlag (eViewMineLights))
	  DrawLights ();

// draw spline
if (theMine->m_bSplineActive)
	DrawSpline ();

*message = '\0';
if (preferences & PREFS_SHOW_POINT_COORDINATES) {
   strcat_s (message, sizeof (message), "  point (x,y,z): (");
   INT16 vertex = theMine->Segments (0) [theMine->Current ()->nSegment].m_info.verts [sideVertTable [theMine->Current ()->nSide][theMine->Current ()->nPoint]];
	char	szCoord [20];
	sprintf_s (szCoord, sizeof (szCoord), "%1.4f,%1.4f,%1.4f)", 
				  theMine->Vertices (vertex)->v.x, theMine->Vertices (vertex)->v.y, theMine->Vertices (vertex)->v.z);
	strcat_s (message, sizeof (message), szCoord);
	}
else {
   // calculate cube size (length between center point of opposing sides)
	strcat_s (message, sizeof (message), "  cube size: ");
	CDoubleVector center1,center2;
   double length;
   center1 = theMine->CalcSideCenter (theMine->Current ()->nSegment, 0);
	center2 = theMine->CalcSideCenter (theMine->Current ()->nSegment, 2);
   length = Distance (center1, center2);
	sprintf_s (message + strlen (message), sizeof (message) - strlen (message), "%.1f", (double) length);
	strcat_s (message, sizeof (message), " x ");
   center1 = theMine->CalcSideCenter (theMine->Current ()->nSegment, 1);
   center2 = theMine->CalcSideCenter (theMine->Current ()->nSegment, 3);
   length = Distance (center1, center2);
   sprintf_s (message + strlen (message), sizeof (message) - strlen (message), "%.1f", (double) length);
	strcat_s (message, sizeof (message), " x ");
   center1 = theMine->CalcSideCenter (theMine->Current ()->nSegment, 4);
   center2 = theMine->CalcSideCenter (theMine->Current ()->nSegment, 5);
   length = Distance (center1, center2);
	sprintf_s (message + strlen (message), sizeof (message) - strlen (message), "%.1f", (double) length);
	}
strcat_s (message, sizeof (message), ",  cube:");
_itoa_s (theMine->Current ()->nSegment, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " side:");
_itoa_s ((currSide = theMine->Current ()->nSide) + 1, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " point:");
_itoa_s (currPoint = theMine->Current ()->nPoint, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " vertex:");
_itoa_s (theMine->CurrSeg ()->m_info.verts [sideVertTable [currSide][currPoint]], message + strlen (message), sizeof (message) - strlen (message), 10);

strcat_s (message, sizeof (message), ",  textures:");
strcat_s (message, sizeof (message), " 1st:");
_itoa_s (theMine->CurrSide ()->m_info.nBaseTex, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " 2nd:");
_itoa_s (theMine->CurrSide ()->m_info.nOvlTex & 0x3fff, message + strlen (message), sizeof (message) - strlen (message), 10);

strcat_s (message, sizeof (message), ",  zoom:");
double zoom_factor = log (10 * m_size.v.x) / log (1.2);
if (zoom_factor > 0) 
	zoom_factor += 0.5;
else
	zoom_factor -= 0.5;
sprintf_s (message + strlen (message), sizeof (message) - strlen (message),  "%1.2f", zoom_factor);
STATUSMSG (message);
}

                        /*--------------------------*/


//eof