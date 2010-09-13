// dlcView.cpp: implementation of the CMineView class
//

#include "stdafx.h"
#include "winuser.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"

#include "PaletteManager.h"
#include "TextureManager.h"
#include "global.h"
#include "render.h"
#include "FileManager.h"

#include <math.h>
#include <time.h>


                        /*--------------------------*/

void CMineView::SetViewPoints (CRect *pRC, bool bSetViewInfo)
{
CHECKMINE;

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
i = vertexManager.Count ();
APOINT *a = m_viewPoints + i;
CVertex* vertP = vertexManager.Vertex (i);
for (; i--; ) {
	if ((--vertP)->Status () == 255)
		continue;
	m_view.Project (*(vertP), *(--a));
	x = a->x;
	y = a->y;
	z = a->z;
	if (rc.left > x) {
		rc.left = x;
		m_minVPIdx.x = (short) i;
		}
	if (rc.right < x) {
		rc.right = x;
		m_maxVPIdx.x = (short) i;
		}
	if (rc.top > y) {
		rc.top = y;
		m_minVPIdx.y = (short) i;
		}
	if (rc.bottom < y) {
		rc.bottom = y;
		m_maxVPIdx.y = (short) i;
		}
	if (minZ > z) {
		minZ = z;
		m_minVPIdx.z = (short) i;
		}
	if (maxZ < z) {
		maxZ = z;
		m_maxVPIdx.z = (short) i;
		}
	}
#if OGL_RENDERING 
//flip mine over for OpenGL
for (i = vertexManager.Count (), a = m_viewPoints; i--; a++) {
	a->y = rc.top + rc.bottom - a->
		y;
	a->z = minZ + maxZ - a->z;
	}
#endif
x = rc.Width ();
y = rc.Height ();
if (pRC)
	*pRC = rc;
m_minViewPoint.x = (short) rc.left;
m_minViewPoint.y = (short) rc.bottom;
m_minViewPoint.z = (short) minZ;
m_maxViewPoint.x = (short) rc.right;
m_maxViewPoint.y = (short) rc.top;
m_maxViewPoint.z = (short) maxZ;
}

//----------------------------------------------------------------------------
// CalcSegDist
//----------------------------------------------------------------------------

void CMineView::CalcSegDist (void)
{
CHECKMINE;

	static short segRef [SEGMENT_LIMIT];

for (CSegmentIterator si; si; si++)
	si->Index () = -1;
segRef [0] = current->m_nSegment;	
current->Segment ()->Index () = 0;

int i = 1, h = 0, j = 0;
int segCount = segmentManager.Count ();

for (short nDist = 1; (j < segCount) && (h < i); nDist++) {
	for (h = i; j < h; j++) {
		CSegment* segI = segmentManager.Segment (segRef [j]);
		for (short nSide = 0; nSide < 6; nSide++) {
			short nChild = segI->Child (nSide);
			if (nChild < 0) 
				continue;
			CSegment* segJ = segmentManager.Segment (nChild);
			if (segJ->Index () != -1)
				continue;
			segJ->Index () = nDist;
			segRef [i++] = nChild;
			}
		}
	}
}

                        /*--------------------------*/

void CMineView::DrawMineCenter (CDC *pViewDC)
{
if (m_nMineCenter == 1) {
	m_pDC->SelectObject(GetStockObject (WHITE_PEN));
	m_pDC->MoveTo (x_center, y_center - (int) (10.0 * m_size.v.y) + 1);
	m_pDC->LineTo (x_center, y_center + (int) (10.0 * m_size.v.y) + 1);
	m_pDC->MoveTo (x_center - (int) (10.0 * m_size.v.x) + 1, y_center);
	m_pDC->LineTo (x_center + (int) (10.0 * m_size.v.x) + 1, y_center);
	}
else if (m_nMineCenter == 2) {
	// draw a globe
	// 5 circles around each axis at angles of 30, 60, 90, 120, and 150
	// each circle has 18 points
	CVertex circle;
	APOINT pt;

	m_pDC->SelectObject (m_penCyan);
	int i, j;
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

CalcSegDist ();
m_pDC->SelectObject(m_penGray);
for (CSegmentIterator si; si; si++) {
	CSegment* segP = &(*si);
	if (!Visible (segP))
		continue;
	DrawSegment (segP, bPartial);
	if (si.Index () == current->m_nSegment) {
		DrawCurrentSegment (segP, bPartial);
		m_pDC->SelectObject (m_penGray);
		}
	}
}

//----------------------------------------------------------------------------
// DrawSegmentsTextured
//----------------------------------------------------------------------------

typedef struct tSegZOrder {
	int		zMax;
	short		iSeg;
} tSegZOrder;

typedef tSegZOrder *pSegZOrder;

static tSegZOrder szo [SEGMENT_LIMIT];

void QSortCubes (short left, short right)
{
	int		m = szo [(left + right) / 2].zMax;
	tSegZOrder	h;
	short	l = left, r = right;

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

void CMineView::DrawSegmentsTextured (void)
{
CHECKMINE;

	short		nSegment;
	short		iVertex;
	int		h, z, zMax;

	// Get shading table data
byte* light_index = 0;
if (m_viewMineFlags & eViewMineShading && (light_index = paletteManager.Current ()))
	light_index += 256*5; // skip 3-byte palette + 1st 2 light tables

// Draw Segments ()
h = segmentManager.Count ();
#pragma omp parallel
{
#	pragma omp for
for (nSegment = 0; nSegment < h; nSegment++) {
	CSegment* segP = segmentManager.Segment (nSegment);
	for (iVertex = 0, zMax = LONG_MIN; iVertex < MAX_VERTICES_PER_SEGMENT; iVertex++)
		if (zMax < (z = m_viewPoints [segP->m_info.verts [iVertex]].z))
			zMax = z;
	szo [nSegment].iSeg = nSegment;
	szo [nSegment].zMax = zMax;
	}
} // omp parallel
QSortCubes (0, segmentManager.Count () - 1);
CalcSegDist ();
for (nSegment = 0; nSegment < h; nSegment++) {
	CSegment* segP = segmentManager.Segment (szo [nSegment].iSeg);
	if (Visible (segP))
	 	DrawSegmentTextured (segP, light_index);
	}
}

//--------------------------------------------------------------------------
// DrawSegment()
//--------------------------------------------------------------------------

bool CMineView::InRange (short *pv, short i)
{
	int	v;

for (; i; i--, pv++) {
	v = *pv;
	if (!(IN_RANGE (m_viewPoints [v].x, x_max) &&
			IN_RANGE (m_viewPoints [v].y, y_max)))
		return false;
	}
return true;
}

								/*-----------------------*/

void CMineView::DrawSegment (CSegment *segP, bool bPartial)
{
DrawSegmentQuick (segP, bPartial);
}

void CMineView::DrawSegment (short nSegment,short nSide, short linenum, short pointnum, short clear_it) 
{
CHECKMINE;

	CSegment *segP = segmentManager.Segment (nSegment);
	short x_max = m_viewWidth * 2;
	short y_max = m_viewHeight * 2;

	if (!Visible (segP))
		return;

	// clear segment and point
	if (clear_it) {
		m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
		m_pDC->SelectObject (GetStockObject(BLACK_PEN)); // BLACK
		int nVert = segP->m_info.verts [sideVertTable [nSide] [pointnum]];
		if (IN_RANGE (m_viewPoints [nVert].x,x_max) &&
			 IN_RANGE (m_viewPoints [nVert].y,y_max)) {
			m_pDC->Ellipse(m_viewPoints [nVert].x - 4,
			m_viewPoints [nVert].y - 4,
			m_viewPoints [nVert].x + 4,
			m_viewPoints [nVert].y + 4);
			}
		if (segP->m_info.wallFlags & MARKED_MASK) {
			m_pDC->SelectObject (m_penCyan);
			DrawSegmentQuick (segP);
			} 
		else {
			if (m_viewOption == eViewPartialLines) {
				m_pDC->SelectObject (GetStockObject(BLACK_PEN));  // BLACK
				DrawSegmentQuick (segP);   // clear all cube lines
				m_pDC->SelectObject (m_penGray); // GRAY
				DrawSegmentPartial (segP); // then redraw the ones we need
				}
			if ((m_viewOption == eViewAllLines) || 
				 (m_viewOption == eViewNearbyCubeLines) || 
				 (m_viewOption == eViewTextureMapped)) {
				m_pDC->SelectObject (m_penGray); // GRAY
				DrawSegmentQuick (segP);
				}
			if (m_viewOption == eViewHideLines) {
				m_pDC->SelectObject (GetStockObject(BLACK_PEN));  // BLACK
				DrawSegmentQuick (segP);   // clear all cube lines
				}
			}
			if (m_viewOption == eViewNearbyCubeLines) {
			m_pDC->SelectObject (GetStockObject(BLACK_PEN));  // BLACK
			DrawSegmentQuick (segP);   // clear all cube lines
			m_pDC->SelectObject (GetStockObject(WHITE_PEN)); // WHITE
			DrawCubePoints (segP);  // then draw the points
			}
		} 
	else {
		if (segP->m_info.wallFlags & MARKED_MASK)
			m_pDC->SelectObject (m_penHiCyan);
		else if (nSegment == current->m_nSegment)
			if (SelectMode (eSelectCube)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);         // RED
			else
				m_pDC->SelectObject (GetStockObject (WHITE_PEN)); // WHITE
			else
				m_pDC->SelectObject (m_penHiGray);        // LIGHT_GRAY
		if (m_viewOption == eViewPartialLines)
			DrawSegmentPartial (segP); // then redraw the ones we need
		else
			DrawSegmentQuick (segP);
		}

	// draw current side
	// must draw in same order as segment to avoid leftover pixels on screen
	if (!clear_it) {
		if (nSegment == current->m_nSegment)
			if (SelectMode (eSelectSide)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);        // RED
			else
				m_pDC->SelectObject (m_penHiGreen); // GREEN
		else
			m_pDC->SelectObject (m_penHiDkGreen);         // DARK_GREEN
		if (IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [0]]].x,x_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [0]]].y,y_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [1]]].x,x_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [1]]].y,y_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [2]]].x,x_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [2]]].y,y_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [3]]].x,x_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [3]]].y,y_max))
			 DrawLine (segP,sideVertTable [nSide] [0],sideVertTable [nSide] [1]);
			 DrawLine (segP,sideVertTable [nSide] [1],sideVertTable [nSide] [2]);
			 DrawLine (segP,sideVertTable [nSide] [2],sideVertTable [nSide] [3]);
			 DrawLine (segP,sideVertTable [nSide] [3],sideVertTable [nSide] [0]);

		// draw current line
		// must draw in same order as segment to avoid leftover pixels on screen
		if (nSegment == current->m_nSegment)
			if (SelectMode (eSelectLine)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);  // RED
			else 
				m_pDC->SelectObject (m_penHiCyan);  // BLUE/CYAN
		else
			m_pDC->SelectObject (m_penDkCyan);  // BLUE/CYAN
		if (IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [0]]].x,x_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [0]]].y,y_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [1]]].x,x_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [1]]].y,y_max))
			DrawLine (segP,
						 lineVertTable [sideLineTable [nSide] [linenum]] [0],
						 lineVertTable [sideLineTable [nSide] [linenum]] [1]);
		}

	// draw a circle around the current point
	if (!clear_it) {
		m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
		if (nSegment == current->m_nSegment)
			if (SelectMode (eSelectPoint)) //  && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed); // RED
			else
				m_pDC->SelectObject (m_penHiCyan); // CYAN
		else
			m_pDC->SelectObject (m_penHiDkCyan); // CYAN
		if (IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x,x_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y,y_max))
			m_pDC->Ellipse(m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x - 4,
								m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y - 4,
								m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x + 4,
								m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y + 4);
		}
}

//--------------------------------------------------------------------------
//			 draw_partial_segment()
//--------------------------------------------------------------------------

void CMineView::DrawSegmentPartial (CSegment *segP) 
{
CHECKMINE;

if (!Visible (segP))
	return;
for (short nLine = 0; nLine < 12; nLine++) {
	if (segP->m_info.mapBitmask & (1<<nLine)) {
      if (IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [nLine] [0]]].x,x_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [nLine] [0]]].y,y_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [nLine] [1]]].x,x_max) &&
			 IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [nLine] [1]]].y,y_max)) {
			short vert0 = lineVertTable [nLine][0];
			short vert1 = lineVertTable [nLine][1];
			if (vert1 > vert0) {
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
// DrawSegment()
//--------------------------------------------------------------------------

void QSortLineRef (POINT *lineRef, short left, short right)
{
	int		m = lineRef [(left + right) / 2].y;
	short	l = left, r = right;
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

void CMineView::DrawSegmentQuick	(CSegment *segP, bool bPartial)
{
CHECKMINE;

if (!Visible (segP))
	return;

	short x_max = m_viewWidth * 2;
	short y_max = m_viewHeight * 2;
	int	chSegI, chSideI, chVertI, i, j, commonVerts;
	CSegment	*childP;
	ushort *pv = segP->m_info.verts;

for (i = 0; i < 8; i++, pv++) {
	int	v = *pv;
	if (!(IN_RANGE (m_viewPoints [v].x, x_max) &&
			IN_RANGE (m_viewPoints [v].y, y_max)))
		return;
	}
if (bPartial) {
	for (short nSide = 0; nSide < 6; nSide++) {
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
				if (segP->Child (chSegI) < 0)
					continue;
				childP = segmentManager.Segment (segP->Child (chSegI));
				// walk through childP cube's sides
				commonVerts = 0;
				for (chSideI = 0; (chSideI < 6) && (commonVerts < 2); chSideI++) {
					// check current childP cube side for common line
					// i.e. check each line for common vertices with the parent line
					for (commonVerts = 0, chVertI = 0; (chVertI < 4) && (commonVerts < 2); chVertI++) {
						vert.x = m_viewPoints [childP->m_info.verts [sideVertTable [chSideI] [chVertI]]].x;
						vert.y = m_viewPoints [childP->m_info.verts [sideVertTable [chSideI] [chVertI]]].y;
						int h;
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
	static	int poly1 [] = {4,0,1,2,3};
	static	int poly2 [] = {3,0,3,7};
	static	int poly3 [] = {5,0,4,5,6,7};
	static	int poly4 [] = {2,4,7};
	static	int poly5 [] = {2,2,6};
	static	int poly6 [] = {2,1,5};
	static	int* polys [] = {poly1, poly2, poly3, poly4, poly5, poly6};
*/
	static	int points [] = {0,1,1,2,2,3,3,0,0,4,4,5,5,6,6,7,7,4,3,7,2,6,1,5,-1};
	int		i, j, k, v, l;

	for (i = 0;; i++) {
		k = points [i];
		if (0 > k)
			break;
		v = segP->m_info.verts [k];
		l = i / 2;
		j = i & 1;
		if (j == 0)
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

void CMineView::DrawLine (CTexture *pTx, POINT pt0, POINT pt1, byte color) 
{
CHECKMINE;

	int i,x,y;
	int dx = pt1.x - pt0.x;
	int dy = pt1.y - pt0.y;

#if 1
	int xInc, yInc;
	double scale;
	int nStep = 0;

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
xInc = (int) ((double) xInc * scale);
yInc = (int) ((double) yInc * scale);

x = pt0.x;
y = pt0.y;

#if 0	//most universal
int xStep = 0, yStep = 0;
int dd = (dx >= dy) ? dx: dy;
for (i = dd + 1; i; i--) {
	pTx->m_info.bmData [y*pTx->m_info.width+x] = color;
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
		pTx->m_info.bmData [y*pTx->m_info.width+x] = color;
		nStep += dy;
		if (nStep >= dx) {
			y += yInc;
			nStep -= dx;
			}
		}
	}
else {
	for (i = dy + 1; i; i--, y += yInc) {
		pTx->m_info.bmData [y*pTx->m_info.width+x] = color;
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
			pTx->m_info.bmData [y*pTx->m_info.width+x] = color;
	else
		for (y=pt0.y;y>=pt1.y;y--)
			pTx->m_info.bmData [y*pTx->m_info.width+x] = color;
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

void CMineView::DrawAnimDirArrows (short texture1, CTexture *pTx)
{
	int sx,sy;
	int bScroll = textureManager.ScrollSpeed (texture1, &sx, &sy);

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
// DrawSegmentTextured()
//--------------------------------------------------------------------------

void CMineView::DrawSegmentTextured(CSegment *segP, byte* light_index) 
{
CHECKMINE;

	short x_max = m_viewWidth * 2;
	short y_max = m_viewHeight * 2;

	if (IN_RANGE (m_viewPoints [segP->m_info.verts [0]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [0]].y,y_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [1]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [1]].y,y_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [2]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [2]].y,y_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [3]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [3]].y,y_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [4]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [4]].y,y_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [5]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [5]].y,y_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [6]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [6]].y,y_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [7]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [7]].y,y_max)   )
	{

		CTexture tex (textureManager.m_bmBuf);
		byte* screenP = (byte *)m_pvBits;
		ushort width = m_viewWidth;
		ushort height = m_viewHeight;
		ushort rowOffset = (m_viewWidth + 3) & ~3;

		CSide* sideP = segP->m_sides;
		for (short nSide = 0; nSide < 6; nSide++, sideP++) {
			if (segP->Child (nSide) != -1) { // not a solid side
				CWall* wallP = sideP->Wall ();
				if (wallP == null) // no wall either
					continue;
				if (wallP->Type () == WALL_OPEN) // invisible wall
					continue;
				if ((wallP->Type () == WALL_CLOAKED) && (wallP->Info ().cloakValue == 0)) // invisible cloaked wall
					continue;
				}
			APOINT& p0 = m_viewPoints [segP->m_info.verts [sideVertTable [nSide][0]]];
			APOINT& p1 = m_viewPoints [segP->m_info.verts [sideVertTable [nSide][1]]];
			APOINT& p3 = m_viewPoints [segP->m_info.verts [sideVertTable [nSide][3]]];

			CDoubleVector	a ((double) (p1.x - p0.x), (double) (p1.y - p0.y), 0.0), 
								b ((double) (p3.x - p0.x), (double) (p3.y - p0.y), 0.0);
			if (a.v.x * b.v.y > a.v.y * b.v.x) {
				if (!textureManager.Define (sideP->BaseTex (), sideP->OvlTex (), &tex, 0, 0)) {
					DrawAnimDirArrows (sideP->BaseTex (), &tex);
					RenderFace (segP, nSide, tex.m_info.bmData, tex.m_info.width, tex.m_info.height, 
								   light_index, screenP, m_viewPoints, width, height, rowOffset);
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

	ushort	*pv = segP->m_info.verts;
	COLORREF	color = RGB (128,128,128);
	int		h, i;

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

void CMineView::DrawMarkedSegments (short clear_it) 
{
CHECKMINE;

	CSegment	*segP;
	short x_max = m_viewWidth * 2;
	short y_max = m_viewHeight * 2;
	short i;

	// draw marked/special Segments () and Walls ()
if (!clear_it) {
	for (i = 0; i < segmentManager.Count (); i++) {
		segP = segmentManager.Segment (i);
		if (segP->m_info.wallFlags & MARKED_MASK) {
			m_pDC->SelectObject (SelectMode (eSelectBlock) ? m_penRed: m_penCyan);
			DrawSegmentQuick (segP);
			}
		else {
			//    if (show_special) {
			if (ViewFlag (eViewMineSpecial) && !(m_viewOption == eViewTextureMapped) ) {
				switch(segP->m_info.function) {
				case SEGMENT_FUNC_FUELCEN:
				case SEGMENT_FUNC_SPEEDBOOST:
					m_pDC->SelectObject (m_penYellow);
					DrawSegmentQuick (segP);
					break;
				case SEGMENT_FUNC_REACTOR:
					m_pDC->SelectObject (m_penOrange);
					DrawSegmentQuick (segP);
					break;
				case SEGMENT_FUNC_REPAIRCEN:
					m_pDC->SelectObject (m_penLtBlue);
					DrawSegmentQuick (segP);
					break;
				case SEGMENT_FUNC_ROBOTMAKER:
				case SEGMENT_FUNC_EQUIPMAKER:
					m_pDC->SelectObject (m_penMagenta);
					DrawSegmentQuick (segP);
					break;
				case SEGMENT_FUNC_GOAL_BLUE:
				case SEGMENT_FUNC_TEAM_BLUE:
					m_pDC->SelectObject (m_penBlue);
					DrawSegmentQuick (segP);
					break;
				case SEGMENT_FUNC_GOAL_RED:
				case SEGMENT_FUNC_TEAM_RED:
					m_pDC->SelectObject (m_penRed);
					DrawSegmentQuick (segP);
					break;
				default:
					if (segP->m_info.props & SEGMENT_PROP_WATER)
						m_pDC->SelectObject (m_penMedBlue);
					else if (segP->m_info.props & SEGMENT_PROP_LAVA)
						m_pDC->SelectObject (m_penMedRed);
					else
						break;
					DrawSegmentQuick (segP);
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
for (i=0;i<vertexManager.Count ();i++)
	if (vertexManager.Status (i) & MARKED_MASK)
		if (IN_RANGE (m_viewPoints [i].x,x_max) && IN_RANGE (m_viewPoints [i].y,y_max))
			m_pDC->Rectangle(m_viewPoints [i].x - 4, m_viewPoints [i].y - 4, m_viewPoints [i].x + 4, m_viewPoints [i].y + 4);
}

//--------------------------------------------------------------------------
// DrawCurrentSegment()
//--------------------------------------------------------------------------

void CMineView::DrawCurrentSegment(CSegment *segP, bool bPartial)
{
CHECKMINE;

	short nSide = current->m_nSide;
	short linenum = current->m_nPoint;
	short pointnum = current->m_nPoint;

	if (segP->m_info.wallFlags & MARKED_MASK) {
		m_pDC->SelectObject(m_penCyan);
	}
	else
	{
		if (m_selectMode == SEGMENT_MODE)
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

	short x_max = m_viewWidth * 2;
	short y_max = m_viewHeight * 2;

	if (IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [0]]].x,x_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [0]]].y,y_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [1]]].x,x_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [1]]].y,y_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [2]]].x,x_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [2]]].y,y_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [3]]].x,x_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [3]]].y,y_max)) {

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

	if (IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [0]]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [0]]].y,y_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [1]]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [lineVertTable [sideLineTable [nSide] [linenum]] [1]]].y,y_max)   ) {

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

	if (IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].x,x_max) &&
		IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [pointnum]]].y,y_max)     ) {

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

void CMineView::DrawLine(CSegment *segP,short vert1,short vert2) 
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

	CWall		*walls = wallManager.Wall (0);
	CSegment	*segments = segmentManager.Segment (0);
	CVertex	*vertices = vertexManager.Vertex (0);
	CSegment	*segP;
	short i,j;
	short x_max = m_viewWidth * 2;
	short y_max = m_viewHeight * 2;

for (i = 0; i < wallManager.WallCount (); i++) {
	if (walls [i].m_nSegment > segmentManager.Count ())
		continue;
	segP = segments + (int)walls [i].m_nSegment;
	if (!Visible (segP))
		continue;
	switch (walls [i].Type ()) {
		case WALL_NORMAL:
			m_pDC->SelectObject(m_penLtGray);
			break;
		case WALL_BLASTABLE:
			m_pDC->SelectObject(m_penLtGray);
			break;
		case WALL_DOOR:
			switch(walls [i].Info ().keys) {
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
	if (IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [j][0]]].x,x_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [j][0]]].y,y_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [j][1]]].x,x_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [j][1]]].y,y_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [j][2]]].x,x_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [j][2]]].y,y_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [j][3]]].x,x_max) &&
		 IN_RANGE (m_viewPoints [segP->m_info.verts [sideVertTable [j][3]]].y,y_max)) {

			CVertex	center, orthog, vector;
			APOINT	point;

		center = segmentManager.CalcSideCenter (walls [i]);
		orthog = segmentManager.CalcSideNormal (walls [i]);
		vector = center - orthog;
		m_view.Project (vector, point);
		for (j = 0; j < 4; j++) {
			m_pDC->MoveTo (point.x,point.y);
			m_pDC->LineTo (m_viewPoints [segP->m_info.verts [sideVertTable [walls [i].m_nSide] [j]]].x,
			m_viewPoints [segP->m_info.verts [sideVertTable [walls [i].m_nSide] [j]]].y);
			}
		if (walls [i].Info ().nTrigger != NO_TRIGGER) {
				APOINT arrowStartPoint,arrowEndPoint,arrow1Point,arrow2Point;
				CVertex fin;

			// calculate arrow points
			vector = center - (orthog * 3);
			m_view.Project (vector, arrowStartPoint);
			vector = center + (orthog * 3);
			m_view.Project (vector, arrowEndPoint);

			// direction toward center of line 0 from center
			byte *svp = &sideVertTable [walls [i].m_nSide][0];
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

  // now show variable lights
  m_pDC->SelectObject(m_penYellow);

  // find variable light from
CVariableLight* flP = lightManager.VariableLight (0);
for (INT i = 0; i < lightManager.Count (); i++, flP++)
	if (Visible (segmentManager.Segment (flP->m_nSegment)))
	   DrawOctagon(flP->m_nSide, flP->m_nSegment);
}

//------------------------------------------------------------------------
// DrawOctagon()
//------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMineView::DrawOctagon(short nSide, short nSegment) 
{
CHECKMINE;

	CSegment *segP;
	short j;
	short x_max = m_viewWidth * 2;
	short y_max = m_viewHeight * 2;

if (nSegment >=0 && nSegment <=segmentManager.Count () && nSide>=0 && nSide<=5 ) {
	POINT corners [4],center,line_centers [4],diamond [4],fortyfive [4];
	segP = segmentManager.Segment (nSegment);
	for (j=0;j<4;j++) {
		corners [j].x = m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [j]]].x;
		corners [j].y = m_viewPoints [segP->m_info.verts [sideVertTable [nSide] [j]]].y;
		}
	if (IN_RANGE (corners [0].x,x_max) && IN_RANGE (corners [0].y,y_max) &&
		 IN_RANGE (corners [1].x,x_max) && IN_RANGE (corners [1].y,y_max) &&
		 IN_RANGE (corners [2].x,x_max) && IN_RANGE (corners [2].y,y_max) &&
		 IN_RANGE (corners [3].x,x_max) && IN_RANGE (corners [3].y,y_max)) {
		center.x = (corners [0].x + corners [1].x + corners [2].x + corners [3].x)>>2;
		center.y = (corners [0].y + corners [1].y + corners [2].y + corners [3].y)>>2;
		for (j = 0; j < 4; j++) {
			int k = (j+1) & 0x03;
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

void CMineView::DrawTunnel (void) 
{
	int h, i, j;
	CVertex points [4];

//  SelectObject(hdc, hrgnAll);
m_pDC->SelectObject (m_penRed);
m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
tunnelMaker.ComputeTunnel ();
APOINT point;
m_view.Project (points [1], point);
if (IN_RANGE (point.x, x_max) && IN_RANGE (point.y, y_max)){
	m_view.Project (points [0], point);
	if (IN_RANGE (point.x, x_max) && IN_RANGE (point.y, y_max)){
		m_pDC->MoveTo (point.x, point.y);
		m_view.Project (points [1], point);
		m_pDC->LineTo (point.x, point.y);
		m_pDC->Ellipse (point.x - 4, point.y - 4, point.x+4,  point.y+4);
		}
	}
m_view.Project (points [2], point);
if (IN_RANGE (point.x, x_max) && IN_RANGE (point.y, y_max)){
	m_view.Project (points [3], point);
	if (IN_RANGE (point.x, x_max) && IN_RANGE (point.y, y_max)){
		m_pDC->MoveTo (point.x, point.y);
		m_view.Project (points [2], point);
		m_pDC->LineTo (point.x, point.y);
		m_pDC->Ellipse (point.x - 4, point.y - 4, point.x+4,  point.y+4);
		}
	}
m_pDC->SelectObject (m_penBlue);
j = MAX_VERTICES;
for (h = tunnelMaker.Length () * 4, i = 0; i < h; i++)
	m_view.Project (*vertexManager.Vertex (--j), m_viewPoints [j]);
CSegment *segP = segmentManager.Segment (MAX_SEGMENTS - 1);
for (i = 0; i < tunnelMaker.Length (); i++, segP--)
	DrawSegmentQuick (segP);
}

//--------------------------------------------------------------------------
//			  DrawObject()
//
// Changed: 0=normal,1=gray,2=black
//        if (nObject == (objectManager.Count ()
//        then its a secret return point)
//--------------------------------------------------------------------------

void TransformModelPoint (CVertex& dest, APOINT &src, CDoubleMatrix &orient, CVertex offs)
{
CDoubleVector v (src.x, src.y, src.z);
dest = orient * v;
dest += offs;
}


void CMineView::DrawObject(short nObject,short clear_it) 
{
CHECKMINE;

	short				poly;
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
	short x_max = m_viewWidth * 2;
	short y_max = m_viewHeight * 2;

//  m_pDC->SelectObject(hrgnBackground);
if (nObject >=0 && nObject < objectManager.Count ()) {
	objP = objectManager.Object (nObject);
	if (!Visible (segmentManager.Segment (objP->m_info.nSegment)))
		return;
	}
else {
	// secret return
	objP = &temp_obj;
	objP->Type () = -1;
	// theMine->secret_orient = Objects () [0]->orient;
	objP->m_location.orient.rVec = -objectManager.SecretOrient ().rVec;
	objP->m_location.orient.uVec =  objectManager.SecretOrient ().fVec;
	objP->m_location.orient.fVec =  objectManager.SecretOrient ().uVec;
	// objP->m_location.orient =  theMine->secret_orient;
	ushort nSegment = (ushort)objectManager.SecretSegment ();
	if (nSegment >= segmentManager.Count ())
		nSegment = 0;
	if (!Visible (segmentManager.Segment (nSegment)))
		return;
	CVertex center;
	objP->Position () = segmentManager.CalcCenter (center, nSegment); // define objP->position
	}

switch (clear_it) {
	case 0: // normal
	case 1: // gray
		if (m_selectMode == OBJECT_MODE && nObject == current->m_nObject) 
			m_pDC->SelectObject(m_penRed); // RED
		else {
			switch(objP->Type ()) {
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
//CBRK (nObject == 45);
for (poly = 0; poly < MAX_POLY; poly++) {
	::TransformModelPoint (pt [poly], object_shape [poly], objP->m_location.orient, objP->Position ());
	m_view.Project (pt [poly], poly_draw [poly]);
	}

// figure out world coordinates

int i;
for (i = 0; i < 6; i++)
	if (!(IN_RANGE (poly_draw [i].x, x_max) &&
			IN_RANGE (poly_draw [i].y, y_max)))
		return;

if ((DLE.IsD2File ()) &&
	 (nObject == current->m_nObject) &&
	 (objP->Type () != OBJ_CAMBOT) && (objP->Type () != OBJ_MONSTERBALL) && 
	 (objP->Type () != OBJ_EXPLOSION) && (objP->Type () != OBJ_SMOKE) && (objP->Type () != OBJ_EFFECT) &&
	 (objP->m_info.renderType == RT_POLYOBJ) &&
	 !modelRenderer.Setup (objP, &m_view, m_pDC)) {
	if (clear_it)
		m_pDC->SelectObject (GetStockObject(BLACK_PEN));
	m_pDC->SelectObject ((HBRUSH)GetStockObject(BLACK_BRUSH));
	modelRenderer.Draw ();
	}
else {
	m_pDC->MoveTo (poly_draw [0].x,poly_draw [0].y);
	for (poly = 0; poly < 6; poly++)
		m_pDC->LineTo (poly_draw [poly].x, poly_draw [poly].y);
	if (nObject == current->m_nObject) {
		int dx,dy;
		for (dx = -1; dx < 2; dx++) {
			for (dy = -1; dy < 2; dy++) {
				m_pDC->MoveTo (poly_draw [0].x+dx,poly_draw [0].y+dy);
				for (poly = 0; poly < 6; poly++)
					m_pDC->LineTo (poly_draw [poly].x + dx, poly_draw [poly].y + dy);
				}
			}
		}
	}
if ((nObject == current->m_nObject) || (nObject == other->m_nObject)) {
	CPen     pen, *pOldPen;
	int		d;

	pt [0] =
	pt [1] =
	pt [2] = objP->Position ();
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
	pen.CreatePen (PS_SOLID, 2, (nObject == current->m_nObject) ? RGB (255,0,0) : RGB (255,208,0));
	pOldPen = m_pDC->SelectObject (&pen);
	m_pDC->SelectObject ((HBRUSH)GetStockObject(HOLLOW_BRUSH));
	m_pDC->Ellipse (poly_draw [0].x - d, poly_draw [0].y - d, poly_draw [0].x + d, poly_draw [0].y + d);
	m_pDC->SelectObject (pOldPen);
	}
}

//--------------------------------------------------------------------------
//			  DrawObjects()
//--------------------------------------------------------------------------

void CMineView::DrawObjects (short clear_it) 
{
CHECKMINE;

if (!ViewObject ())
	return;

int i, j;
if (DLE.IsD2File ()) {
	// see if there is a secret exit trigger
	for(i = 0; i < triggerManager.WallTriggerCount (); i++)
	if (triggerManager.Trigger (i)->Type () == TT_SECRET_EXIT) {
		DrawObject ((short)objectManager.Count (), 0);
		break; // only draw one secret exit
		}
	}
HiliteTarget ();
CGameObject *objP = objectManager.Object (0);
for (i = objectManager.Count (), j = 0; i; i--, j++, objP++)
	if (ViewObject (objP))
		DrawObject (j, 0);
}

//--------------------------------------------------------------------------
//			  draw_highlight()
//--------------------------------------------------------------------------

void CMineView::DrawHighlight(short clear_it) 
{
CHECKMINE;

	short	currSide, currPoint;
//	short i;
//	RECT rect;

if (segmentManager.Count ()==0) 
	return;

// draw Objects ()
if (!clear_it) {
	DrawObjects (clear_it);
//	if (/*!(preferences & PREFS_HIDE_MARKED_BLOCKS) ||*/ SelectMode (eSelectBlock))
	DrawMarkedSegments(clear_it);
  }

// draw highlighted Segments () (other first, then current)
if (*current == selections [0]) {
	if (selections [0].m_nSegment != selections [1].m_nSegment)
		DrawSegment (selections [1].m_nSegment, selections [1].m_nSide, selections [1].m_nLine, selections [1].m_nPoint,clear_it);
	DrawSegment (selections [0].m_nSegment, selections [0].m_nSide, selections [0].m_nLine, selections [0].m_nPoint,clear_it);
	}
else {
	if (selections [0].m_nSegment != selections [1].m_nSegment)
		DrawSegment (selections [0].m_nSegment, selections [0].m_nSide, selections [0].m_nLine, selections [0].m_nPoint,clear_it);
	DrawSegment (selections [1].m_nSegment, selections [1].m_nSide, selections [1].m_nLine, selections [1].m_nPoint,clear_it);
	}

// draw Walls ()
if (ViewFlag (eViewMineWalls))
	DrawWalls ();

// draw lights
if (ViewFlag (eViewMineLights))
	  DrawLights ();

tunnelMaker.Draw (m_pDC, m_penRed, m_penBlue, m_view);

*message = '\0';
if (preferences & PREFS_SHOW_POINT_COORDINATES) {
   strcat_s (message, sizeof (message), "  point (x,y,z): (");
   short vertex = segmentManager.Segment (0) [current->m_nSegment].m_info.verts [sideVertTable [current->m_nSide][current->m_nPoint]];
	char	szCoord [20];
	sprintf_s (szCoord, sizeof (szCoord), "%1.4f,%1.4f,%1.4f)", 
				  vertexManager.Vertex (vertex)->v.x, vertexManager.Vertex (vertex)->v.y, vertexManager.Vertex (vertex)->v.z);
	strcat_s (message, sizeof (message), szCoord);
	}
else {
   // calculate cube size (length between center point of opposing sides)
	strcat_s (message, sizeof (message), "  cube size: ");
	CDoubleVector center1,center2;
   double length;
   center1 = segmentManager.CalcSideCenter (CSideKey (current->m_nSegment, 0));
	center2 = segmentManager.CalcSideCenter (CSideKey (current->m_nSegment, 2));
   length = Distance (center1, center2);
	sprintf_s (message + strlen (message), sizeof (message) - strlen (message), "%.1f", (double) length);
	strcat_s (message, sizeof (message), " x ");
   center1 = segmentManager.CalcSideCenter (CSideKey (current->m_nSegment, 1));
   center2 = segmentManager.CalcSideCenter (CSideKey (current->m_nSegment, 3));
   length = Distance (center1, center2);
   sprintf_s (message + strlen (message), sizeof (message) - strlen (message), "%.1f", (double) length);
	strcat_s (message, sizeof (message), " x ");
   center1 = segmentManager.CalcSideCenter (CSideKey (current->m_nSegment, 4));
   center2 = segmentManager.CalcSideCenter (CSideKey (current->m_nSegment, 5));
   length = Distance (center1, center2);
	sprintf_s (message + strlen (message), sizeof (message) - strlen (message), "%.1f", (double) length);
	}
strcat_s (message, sizeof (message), ",  cube:");
_itoa_s (current->m_nSegment, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " side:");
_itoa_s ((currSide = current->m_nSide) + 1, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " point:");
_itoa_s (currPoint = current->m_nPoint, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " vertex:");
_itoa_s (current->Segment ()->m_info.verts [sideVertTable [currSide][currPoint]], message + strlen (message), sizeof (message) - strlen (message), 10);

strcat_s (message, sizeof (message), ",  textures:");
strcat_s (message, sizeof (message), " 1st:");
_itoa_s (current->Side ()->m_info.nBaseTex, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " 2nd:");
_itoa_s (current->Side ()->m_info.nOvlTex & 0x3fff, message + strlen (message), sizeof (message) - strlen (message), 10);

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