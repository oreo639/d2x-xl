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
#include "FileManager.h"
#include "ModelManager.h"

#include <math.h>
#include <time.h>

extern short nDbgSeg, nDbgSide;
extern int nDbgVertex;

#define MODEL_DISPLAY_LIMIT 300.0 // max. distance from viewer at which 3D models and icons are rendered instead of angled arrows

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

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
			short nChild = segI->ChildId (nSide);
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

//------------------------------------------------------------------------------

void CMineView::DrawMineCenter (void)
{
if (Renderer ().Perspective ())
	return;

	CViewMatrix*	viewMatrix = ViewMatrix ();

if (m_nMineCenter == 1) {
	Renderer ().SelectPen (penWhite + 1);
	int nScale = int (20.0 * Scale ().v.y) + 1;
	CVertex v = viewMatrix->Origin ();
	viewMatrix->Transform (v.m_view, v);
	v.Project (viewMatrix);
	Renderer ().BeginRender (true);
	Renderer ().MoveTo (v.m_screen.x, v.m_screen.y - nScale);
	Renderer ().LineTo (v.m_screen.x, v.m_screen.y + nScale);
	Renderer ().MoveTo (v.m_screen.x - nScale, v.m_screen.y);
	Renderer ().LineTo (v.m_screen.x + nScale, v.m_screen.y);
	Renderer ().EndRender ();
	}
else if (m_nMineCenter == 2) {
	// draw a globe
	// 5 circles around each axis at angles of 30, 60, 90, 120, and 150
	// each circle has 18 points
	CVertex			v, center, circles [3][120 / 30 + 1][360 / 15 + 1];

	int h, i, j, m, n;

	Renderer ().BeginRender ();
	for (i = -60, m = 0; i <= 60; i += 30, m++) {
		for (j = 0, n = 0; j <= 360; j += 15, n++) {
			double sini = sin (Radians (i));
			double cosi = cos (Radians (i));
			double sinj = sin (Radians (j));
			double cosj = cos (Radians (j));
			double scale = 5 * cosi;

			v.Set (scale * cosj, scale * sinj, 5 * sini);
			v += viewMatrix->Origin ();
			if (!m_nRenderer)
				v -= viewMatrix->m_data [0].m_translate;
			viewMatrix->Transform (v.m_view, v);
			v.Project (viewMatrix);
			circles [0][m][n] = v;

			v.Set (scale * cosj, 5 * sini, scale * sinj);
			v += viewMatrix->Origin ();
			if (!m_nRenderer)
				v -= viewMatrix->m_data [0].m_translate;
			viewMatrix->Transform (v.m_view, v);
			v.Project (viewMatrix);
			circles [1][m][n] = v;

			v.Set (5 * sini, scale * cosj, scale * sinj);
			v += viewMatrix->Origin ();
			if (!m_nRenderer)
				v -= viewMatrix->m_data [0].m_translate;
			viewMatrix->Transform (v.m_view, v);
			v.Project (viewMatrix);
			circles [2][m][n] = v;
			}
		}

	center = ViewMatrix ()->Origin ();
	viewMatrix->Transform (center.m_view, center);
	Renderer ().EndRender ();

	ePenColor penColors [3] = {penBoldGreen, penBoldGray, penBoldGold};

	Renderer ().BeginRender (true);
	for (h = 0; h < 3; h++) {
		Renderer ().SelectPen (penColors [h] + 1);
		for (i = -60, m = 0; i <= 60; i += 30, m++) {
			for (j = 0, n = 0; j <= 360; j += 15, n++) {
				CVertex v = circles [h][m][n];
				if (j == 0)
					Renderer ().MoveTo (v.m_screen.x, v.m_screen.y);
				else if (m_nRenderer ? (v.m_view.v.z < center.m_view.v.z) : (v.m_screen.z <= 0))
					Renderer ().LineTo (v.m_screen.x, v.m_screen.y);
				else 
					Renderer ().MoveTo (v.m_screen.x, v.m_screen.y);
				}
			}
		}
	Renderer ().EndRender ();
	}
}
//----------------------------------------------------------------------------
// DrawWireFrame
//----------------------------------------------------------------------------

void CMineView::DrawWireFrame (bool bSparse)
{
CHECKMINE;

CalcSegDist ();
Renderer ().BeginRender (Renderer ().Type () == 0);
Renderer ().SelectPen (penGray + 1);
CSegment* segP = segmentManager.Segment (0);
short segCount = segmentManager.Count ();
for (short nSegment = 0; nSegment < segCount; nSegment++, segP++) 
	DrawSegmentWireFrame (segP, bSparse);
Renderer ().EndRender ();
if (Visible (current->Segment ())) {
	DrawCurrentSegment (current->Segment (), bSparse);
	}
}

//----------------------------------------------------------------------------

static CFaceListEntry faceRenderList [SEGMENT_LIMIT * 6];

void SortFaces (int left, int right)
{
	CFaceListEntry m = faceRenderList [(left + right) / 2];
	int l = left, r = right;

do {
	while ((faceRenderList [l].m_zMax > m.m_zMax) || ((faceRenderList [l].m_zMax == m.m_zMax) && (faceRenderList [l].m_zMin > m.m_zMin)))
		l++;
	while ((faceRenderList [r].m_zMax < m.m_zMax) || ((faceRenderList [l].m_zMax == m.m_zMax) && (faceRenderList [l].m_zMin < m.m_zMin)))
		r--;
	if (l <= r) {
		if (l < r)
			Swap (faceRenderList [l], faceRenderList [r]);
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	SortFaces (l, right);
if (left < r)
	SortFaces (left, r);
}

//----------------------------------------------------------------------------

void CMineView::DrawSegmentsTextured (void)
{
CHECKMINE;
if (!textureManager.Available ())
	return;

// Draw Segments ()
short segCount = segmentManager.Count ();
int faceCount = 0;
CSegment* segP = segmentManager.Segment (0);
for (short nSegment = 0; nSegment < segCount; nSegment++, segP++) {
	if (!Visible (segP))
		continue;

	CSide* sideP = segP->Side (0);
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
#ifdef _DEBUG
		if ((nSegment == nDbgSeg) && ((nDbgSide < 0) || (nSide == nDbgSide)))
			nDbgSeg = nDbgSeg;
#endif
		if (segP->m_info.bTunnel)
			continue;
		if (segP->ChildId (nSide) != -1) { // not a solid side
			CWall* wallP = sideP->Wall ();
			if (wallP == null) // no wall either
				continue;
			if (wallP->Type () == WALL_OPEN) // invisible wall
				continue;
			if ((wallP->Type () == WALL_CLOAKED) && (wallP->Info ().cloakValue == 0)) // invisible cloaked wall
				continue;
			}

		if (!FaceIsVisible (segP, sideP))
			continue;
		if (sideP->Shape () > SIDE_SHAPE_TRIANGLE)
			continue;

		long zMin = -LONG_MIN, zMax = LONG_MIN;
		for (ushort nVertex = 0; nVertex < 4; nVertex++) {
			long z = vertexManager [segP->m_info.vertexIds [sideP->VertexIdIndex (nVertex)]].m_screen.z;
			if (zMin > z)
				zMin = z;
			if (zMax < z)
				zMax = z;
			}

		faceRenderList [faceCount].m_nSegment = nSegment;
		faceRenderList [faceCount].m_nSide = nSide;
		faceRenderList [faceCount].m_zMin = zMin;
		faceRenderList [faceCount].m_zMax = zMax;
		CWall* wallP = sideP->Wall ();
		faceRenderList [faceCount].m_bTransparent = textureManager.Texture (sideP->BaseTex ())->Transparent () || ((wallP != null) && (wallP->Alpha () < 255));
		++faceCount;
		}
	}

if (!faceCount)
	return;
if (faceCount > 1) 
	SortFaces (0, faceCount - 1);
CalcSegDist ();

Renderer ().RenderFaces (faceRenderList, faceCount, m_bSelectTexturedSides);
}

//--------------------------------------------------------------------------
// DrawSegment ()
//--------------------------------------------------------------------------

bool CMineView::InRange (short *pv, short i)
{
for (; i; i--, pv++) {
	if ((*pv <= MAX_VERTEX) && !vertexManager [*pv].InRange (m_viewMax.x, m_viewMax.y, Renderer ().Type ()))
		return false;
	}
return true;
}

//--------------------------------------------------------------------------

void CMineView::DrawSegment (short nSegment, short nSide, short nLine, short nPoint, short bClearIt) 
{
CHECKMINE;

	CSegment *segP = segmentManager.Segment (nSegment);
	CSide* sideP = segP->Side (nSide);
	short xMax = ViewWidth () * 2;
	short yMax = ViewHeight () * 2;

	if (!Visible (segP))
		return;

if (bClearIt) {
	Renderer ().SelectObject ((HBRUSH) GetStockObject (NULL_BRUSH));
	Renderer ().SelectPen (penBlack + 1); // BLACK
	int nVert = segP->m_info.vertexIds [sideP->VertexIdIndex (nPoint)];
	if (vertexManager [nVert].InRange (xMax, yMax, Renderer ().Type ()))
		Renderer ().Ellipse (vertexManager [nVert], 4, 4);
	if (segP->IsMarked ()) {
		Renderer ().SelectPen (penGold + 1);
		DrawSegmentWireFrame (segP);
		} 
	else {
		if (m_viewOption == eViewWireFrameSparse) {
			Renderer ().SelectPen (penBlack + 1);
			DrawSegmentWireFrame (segP);  
			Renderer ().SelectPen (penGray + 1);
			DrawSegmentPartial (segP); 
			}
		if ((m_viewOption == eViewWireFrameFull) || (m_viewOption == eViewNearbyCubeLines) || (m_viewOption == eViewTextured)) {
			Renderer ().SelectPen (penGray + 1); 
			DrawSegmentWireFrame (segP);
			}
		if (m_viewOption == eViewHideLines) {
			Renderer ().SelectPen (penBlack + 1);  
			DrawSegmentWireFrame (segP);  
			}
		}
	if (m_viewOption == eViewNearbyCubeLines) {
		Renderer ().SelectPen (penBlack + 1);  
		DrawSegmentWireFrame (segP);  
		Renderer ().SelectPen (penWhite + 1); 
		DrawSegmentPoints (segP);  
		}
	} 
else {
	if (segP->IsMarked ())
		Renderer ().SelectPen (penBoldGold + 1);
	else 
		Renderer ().SelectPen ((nSegment == current->m_nSegment) ? SelectMode (eSelectSegment) ? penBoldRed + 1 : penWhite + 1 : penBoldGray + 1);   
	if (m_viewOption == eViewWireFrameSparse)
		DrawSegmentPartial (segP);
	else
		DrawSegmentWireFrame (segP);

	int i;
	if (!Renderer ().Ortho ())
		i = 4;
	else {
		for (i = 0; i < 4; i++)
			if (!vertexManager [segP->m_info.vertexIds [sideP->VertexIdIndex (i)]].InRange (xMax, yMax, Renderer ().Type ()))
				break;
		}
	if (i == 4) {
		Renderer ().SelectPen ((nSegment == current->m_nSegment) ? SelectMode (eSelectSide) ? penBoldRed + 1 : penBoldGreen + 1 : penBoldDkGreen + 1);  
		short nVertices = sideP->VertexCount ();
		for (i = 0; i < nVertices; i++)
			DrawLine (segP, sideP->VertexIdIndex (i), sideP->VertexIdIndex (i + 1));
		}

	//if (!Renderer ().Ortho ()) 
		{
		short i1 = sideP->VertexIdIndex (nLine);
		short i2 = sideP->VertexIdIndex (nLine + 1);
		if (!Renderer ().Ortho () ||
			 (vertexManager [segP->m_info.vertexIds [i1]].InRange (xMax, yMax, Renderer ().Type ()) &&
		     vertexManager [segP->m_info.vertexIds [i2]].InRange (xMax, yMax, Renderer ().Type ()))) 
			{
			Renderer ().SelectPen ((nSegment == current->m_nSegment) ? SelectMode (eSelectLine) ? penBoldRed + 1 : penBoldGold + 1 : penDkCyan + 1);  
			DrawLine (segP, i1, i2);
			}
		}

#if 0
	if (Renderer ().Ortho () && !Perspective ()) {
		Renderer ().EndRender ();
		Renderer ().BeginRender (true);
		}
#endif
	Renderer ().SelectObject ((HBRUSH) GetStockObject (NULL_BRUSH));
	Renderer ().SelectPen ((nSegment == current->m_nSegment) ? SelectMode (eSelectPoint) ? penBoldRed + 1 : penBoldGold + 1 : penBoldDkCyan + 1); 
	i = segP->m_info.vertexIds [sideP->VertexIdIndex (nPoint)];
	if (vertexManager [i].InRange (xMax, yMax, Renderer ().Type ()))
		Renderer ().Ellipse (vertexManager [i], 4, 4);
	}
}

//--------------------------------------------------------------------------

void CMineView::DrawSegmentPartial (CSegment *segP) 
{
RenderSegmentWireFrame (segP, true);
}

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

void CMineView::DrawSparseSegmentWireFrame (CSegment *segP)
{
for (short nSide = 0; nSide < 6; nSide++) {
	if (segP->ChildId (nSide) >= 0)
		continue;
	CSide* sideP = segP->Side (nSide);
	
	CPoint side [4], line [2], vert;
	int i, j, nCommonVerts;

	for (i = 0; i < 4; i++) {
		side [i].x = vertexManager [segP->m_info.vertexIds [sideP->VertexIdIndex (i)]].m_screen.x; 
		side [i].y = vertexManager [segP->m_info.vertexIds [sideP->VertexIdIndex (i)]].m_screen.y; 
		}
	CDoubleVector a,b;
	a.v.x = (double) (side [1].x - side [0].x);
	a.v.y = (double) (side [1].y - side [0].y);
	b.v.x = (double) (side [3].x - side [0].x);
	b.v.y = (double) (side [3].y - side [0].y);
	if (a.v.x * b.v.y < a.v.y * b.v.x)
		Renderer ().SelectPen (penWhite + 1);
	else
		Renderer ().SelectPen (penGray + 1);
	// draw each line of the current side separately
	// only draw if there is no childP segment of the current segment with a common side
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 2; j++)
			line [j] = side [(i+j)%4];

		// check childP segments
		nCommonVerts = 0;
		for (short nChildSeg = 0; (nChildSeg < 6) && (nCommonVerts < 2); nChildSeg++) {
			if (segP->ChildId (nChildSeg) < 0)
				continue;
			CSegment* childSegP = segmentManager.Segment (segP->ChildId (nChildSeg));
			// walk through childP segment's sides
			nCommonVerts = 0;
			for (short nChildSide = 0; (nChildSide < 6) && (nCommonVerts < 2); nChildSide++) {
				// check current childP segment side for common line
				// i.e. check each line for common vertices with the parent line
				nCommonVerts = 0;
				for (short nChildVertex = 0; (nChildVertex < 4) && (nCommonVerts < 2); nChildVertex++) {
					CVertex& v = *childSegP->Vertex (nChildSide, nChildVertex);
					int h;
					for (h = 0; h < 2; h++) {
						if ((line [h].x == v.m_screen.x) && (line [h].y == v.m_screen.y)) {
							++nCommonVerts;
							break;
							}
						}
					}
				}
			}
		if (nCommonVerts < 2)
			Renderer ().PolyLine (line, 2);
		}
	}
}

//--------------------------------------------------------------------------

void CMineView::RenderSegmentWireFrame (CSegment *segP, bool bSparse)
{
	int bOrtho = Renderer ().Ortho ();

if (bOrtho) {
	if (!Visible (segP))
		return;
	}
else if (!bSparse) {
	if ((segP == current->Segment ()) || (segP == other->Segment ()))
		glDisable (GL_DEPTH_TEST);
	else
		glEnable (GL_DEPTH_TEST);
	glLineWidth (ViewOption (eViewTexturedWireFrame) ? 3.0f : 2.0f);
	}

	CEdgeList	edgeList;
	ushort*	vertexIds = segP->m_info.vertexIds;
	short		xMax = ViewWidth (),
				yMax = ViewHeight ();
	int		nType = Renderer ().Type ();

for (int i = 0, j = segP->BuildEdgeList (edgeList, bSparse); i < j; i++) {
	ubyte i1, i2, side1, side2;
	edgeList.Get (i, side1, side2, i1, i2);
	CVertex& v1 = vertexManager [vertexIds [i1]];
	CVertex& v2 = vertexManager [vertexIds [i2]];
	if (!bOrtho) {
		Renderer ().MoveTo (v1);
		Renderer ().LineTo (v2);
		}
	else if (v1.InRange (xMax, yMax, nType) && v2.InRange (xMax, yMax, nType)) {
		Renderer ().MoveTo (v1.m_screen.x, v1.m_screen.y);
		Renderer ().LineTo (v2.m_screen.x, v2.m_screen.y);
		}
	}
}

//--------------------------------------------------------------------------

void CMineView::DrawSegmentWireFrame (CSegment *segP, bool bSparse, char bTunnel)
{
CHECKMINE;

if (!Visible (segP))
	return;
if (segP->m_info.bTunnel != bTunnel)
	return;

if (bSparse || Renderer ().Ortho ()) {
	short xMax = ViewWidth ();
	short yMax = ViewHeight ();
	ushort* vertexIds = segP->m_info.vertexIds;
	for (int i = 0; i < 8; i++, vertexIds++) {
		int v = *vertexIds;
		if ((v <= MAX_VERTEX) && !vertexManager [v].InRange (xMax, yMax, Renderer ().Type ()))
			return;
		}
	}

if (bSparse)
	DrawSparseSegmentWireFrame (segP);
else
	RenderSegmentWireFrame (segP, false);
}

//--------------------------------------------------------------------------
//                        draw_segmentPoints()
//--------------------------------------------------------------------------

void CMineView::DrawSegmentPoints (CSegment *segP)
{
CHECKMINE;

	ushort*	vertexIds = segP->m_info.vertexIds;
	int		h;

Renderer ().SelectPen (penGray + 1);

for (int i = 0; i < 8; i++, vertexIds++)
	if (MAX_VERTEX >= (h = *vertexIds))
		Renderer ().Ellipse (vertexManager [h].m_screen.x, vertexManager [h].m_screen.y, 2, 2);
}

//--------------------------------------------------------------------------

bool CMineView::SelectWireFramePen (CSegment* segP)
{
if (segP->IsMarked ()) {
	Renderer ().SelectPen (SelectMode (eSelectBlock) ? penRed + 1 : penGold + 1);
	return true;
	}

if (ViewFlag (eViewMineSpecial) && !(m_viewOption == eViewTextured)) {
	switch (segP->m_info.function) {
		case SEGMENT_FUNC_PRODUCER:
		case SEGMENT_FUNC_SPEEDBOOST:
			Renderer ().SelectPen (penYellow + 1);
			return true;
		case SEGMENT_FUNC_REACTOR:
			Renderer ().SelectPen (penOrange + 1);
			return true;
		case SEGMENT_FUNC_REPAIRCEN:
			Renderer ().SelectPen (penLtBlue + 1);
			return true;
		case SEGMENT_FUNC_ROBOTMAKER:
		case SEGMENT_FUNC_EQUIPMAKER:
			Renderer ().SelectPen (penMagenta + 1);
			return true;
		case SEGMENT_FUNC_GOAL_BLUE:
		case SEGMENT_FUNC_TEAM_BLUE:
			Renderer ().SelectPen (penBlue + 1);
			return true;
		case SEGMENT_FUNC_GOAL_RED:
		case SEGMENT_FUNC_TEAM_RED:
			Renderer ().SelectPen (penRed + 1);
			return true;
		default:
			if (segP->m_info.props & SEGMENT_PROP_WATER)
				Renderer ().SelectPen (penMedBlue + 1);
			else if (segP->m_info.props & SEGMENT_PROP_LAVA)
				Renderer ().SelectPen (penMedRed + 1);
			else
				return false;
		return true;
		}
	}
return false;
}

//--------------------------------------------------------------------------

void CMineView::DrawMarkedSegments (short bClear) 
{
CHECKMINE;

	CSegment*	segP;
	short			xMax = ViewWidth ();
	short			yMax = ViewHeight ();
	short			i;

Renderer ().BeginRender (Renderer ().Type () == 0);
if (!Renderer ().Ortho ())
	glDepthFunc (GL_LEQUAL);
if (!bClear) {
	for (i = 0; i < segmentManager.Count (); i++) {
		if (SelectWireFramePen (segP = segmentManager.Segment (i)))
			DrawSegmentWireFrame (segP);
		}
	}
Renderer ().EndRender ();
Renderer ().BeginRender (true);
// draw a square around all marked points
Renderer ().SelectObject ((HBRUSH)GetStockObject (NULL_BRUSH));
if (bClear)
	Renderer ().SelectPen (penBlack + 1);
else if (SelectMode (eSelectBlock)) // && edit_mode != EDIT_OFF) {
	Renderer ().SelectPen (penRed + 1);
else
	Renderer ().SelectPen (penGold + 1);
for (i = 0; i < vertexManager.Count (); i++)
	if ((vertexManager.Status (i) & MARKED_MASK) && vertexManager [i].InRange (xMax, yMax, Renderer ().Type ())) 
		Renderer ().Rectangle (vertexManager [i], 5, 5);
Renderer ().EndRender ();
}

//--------------------------------------------------------------------------

void CMineView::DrawCurrentSegment (CSegment *segP, bool bSparse)
{
CHECKMINE;

	short xMax = ViewWidth ();
	short yMax = ViewHeight ();
	short nSide = current->m_nSide;
	short nLine = current->m_nPoint;
	short	nPoint = current->m_nPoint;
	CSide* sideP = segP->Side (nSide);
	short nVertices = sideP->VertexCount ();

if (Renderer ().Type ()) {
	BeginRender ();
	glDisable (GL_DEPTH_TEST);
	}
else {
	for (int i = 0; i < nVertices; i++)
		if (!vertexManager [segP->m_info.vertexIds [sideP->VertexIdIndex (i)]].InRange (xMax, yMax, Renderer ().Type ()))
			return;
	BeginRender (true);
	}

Renderer ().SelectPen ((m_selectMode == SIDE_MODE) ? penRed + 1 : penGreen + 1);
for (int i = 0; i < nVertices; i++)
	DrawLine (segP, sideP->VertexIdIndex (i), sideP->VertexIdIndex (i + 1));
// draw current line
Renderer ().SelectPen ((m_selectMode == SIDE_MODE) ? penRed + 1 : penGold + 1);
DrawLine (segP, sideP->VertexIdIndex (nLine), sideP->VertexIdIndex (nLine + 1));
// draw a circle around the current point
Renderer ().SelectObject ((HBRUSH) GetStockObject (NULL_BRUSH));
Renderer ().SelectPen ((m_selectMode == SIDE_MODE) ? penRed + 1 : penGold + 1);
Renderer ().Ellipse (vertexManager [segP->m_info.vertexIds [sideP->VertexIdIndex (nPoint)]], 4, 4);
EndRender ();
}

//--------------------------------------------------------------------------

void CMineView::DrawLine (CSegment* segP, short i1, short i2) 
{
CHECKMINE;
if (Renderer ().Ortho ()) {
	Renderer ().MoveTo (vertexManager [segP->m_info.vertexIds [i1]].m_screen.x, vertexManager [segP->m_info.vertexIds [i1]].m_screen.y);
	Renderer ().LineTo (vertexManager [segP->m_info.vertexIds [i2]].m_screen.x, vertexManager [segP->m_info.vertexIds [i2]].m_screen.y);
	}
else {
	Renderer ().MoveTo (vertexManager [segP->m_info.vertexIds [i1]]);
	Renderer ().LineTo (vertexManager [segP->m_info.vertexIds [i2]]);
	}
}
//--------------------------------------------------------------------------

void CMineView::SelectWallPen (CWall* wallP)
{
switch (wallP->Type ()) {
	case WALL_NORMAL:
		Renderer ().SelectPen (penLtGray + 1);
		break;
	case WALL_BLASTABLE:
		Renderer ().SelectPen (penLtGray + 1);
		break;
	case WALL_DOOR:
		switch(wallP->Info ().keys) {
			case KEY_NONE:
				Renderer ().SelectPen (penLtGray + 1);
				break;
			case KEY_BLUE:
				Renderer ().SelectPen (penBlue + 1);
				break;
			case KEY_RED:
				Renderer ().SelectPen (penRed + 1);
				break;
			case KEY_GOLD:
				Renderer ().SelectPen (penYellow + 1);
				break;
			default:
				Renderer ().SelectPen (penGray + 1);
			}
		break;
	case WALL_ILLUSION:
		Renderer ().SelectPen (penLtGray + 1);
		break;
	case WALL_OPEN:
		Renderer ().SelectPen (penLtGray + 1);
		break;
	case WALL_CLOSED:
		Renderer ().SelectPen (penLtGray + 1);
		break;
	default:
		Renderer ().SelectPen (penLtGray + 1);
	}
}

//--------------------------------------------------------------------------

void CMineView::DrawWalls (void) 
{
CHECKMINE;

	CWall*		walls = wallManager.Wall (0);
	CSegment*	segments = segmentManager.Segment (0);
	CSegment*	segP;
	CSide*		sideP;
	short			i,j;
	short			xMax = ViewWidth () * 2;
	short			yMax = ViewHeight () * 2;

for (i = 0; i < wallManager.WallCount (); i++) {
	if (walls [i].m_nSegment > segmentManager.Count ())
		continue;
	segP = segments + (int) walls [i].m_nSegment;
	if (!Visible (segP))
		continue;
	short nSide = walls [i].m_nSide;
	sideP = segP->Side (nSide);
	for (j = 0; j < 4; j++) {
		CVertex& v = vertexManager [segP->m_info.vertexIds [sideP->VertexIdIndex (j)]];
		if (IN_RANGE (v.m_screen.x, xMax) && IN_RANGE (v.m_screen.y, yMax))
			break;
		}
	if (j < 4) {
			CVertex center, normal, vector;
			CVertex arrowStartPoint, arrowEndPoint, arrow1Point, arrow2Point;

		center = segmentManager.CalcSideCenter (walls [i]);
		normal = segmentManager.CalcSideNormal (walls [i]);
		vector = center - normal;
		Renderer ().BeginRender ();
		vector.Transform (ViewMatrix ());
		vector.Project (ViewMatrix ());

		if (walls [i].Info ().nTrigger != NO_TRIGGER) {
			// calculate arrow points
			arrowStartPoint = center - (normal * 3);
			arrowStartPoint.Transform (ViewMatrix ());
			arrowStartPoint.Project (ViewMatrix ());
			arrowEndPoint = center + (normal * 3);
			arrowEndPoint.Transform (ViewMatrix ());
			arrowEndPoint.Project (ViewMatrix ());
			// direction toward center of line 0 from center
			CVertex vector = Average (*segP->Vertex (nSide, 0), *segP->Vertex (nSide, 1));
			vector -= center;
			vector.Normalize ();

			arrow1Point = (normal * 2);
			arrow1Point += center;
			arrow1Point += vector;
			arrow1Point.Transform (ViewMatrix ());
			arrow1Point.Project (ViewMatrix ());
			vector *= 2;
			arrow2Point = arrow1Point - vector;
			arrow2Point.Transform (ViewMatrix ());
			arrow2Point.Project (ViewMatrix ());
			}
		Renderer ().EndRender ();

		if (Renderer ().Type ()) {
			Renderer ().BeginRender (false);
			glEnable (GL_DEPTH_TEST);
			SelectWallPen (&walls [i]);
			glLineWidth (ViewOption (eViewTexturedWireFrame) ? 4.0f : 3.0f);
			for (j = 0; j < sideP->VertexCount (); j++) {
				Renderer ().MoveTo (vector);
				Renderer ().LineTo (*segP->Vertex (nSide, j));
				}
			if (walls [i].Info ().nTrigger != NO_TRIGGER) {
				// draw arrow
				Renderer ().MoveTo (arrowStartPoint);
				Renderer ().LineTo (arrowEndPoint);
				Renderer ().LineTo (arrow1Point);
				Renderer ().MoveTo (arrowEndPoint);
				Renderer ().LineTo (arrow2Point);
				}
			}
		else {
			Renderer ().BeginRender (true);
			SelectWallPen (&walls [i]);
			for (j = 0; j < sideP->VertexCount (); j++) {
				Renderer ().MoveTo (vector.m_screen.x, vector.m_screen.y);
				CVertex& v = *segP->Vertex (nSide, j);
				Renderer ().LineTo (v.m_screen.x, v.m_screen.y);
				}
			if (walls [i].Info ().nTrigger != NO_TRIGGER) {
				// draw arrow
				Renderer ().MoveTo (arrowStartPoint.m_screen.x, arrowStartPoint.m_screen.y);
				Renderer ().LineTo (arrowEndPoint.m_screen.x, arrowEndPoint.m_screen.y);
				Renderer ().LineTo (arrow1Point.m_screen.x, arrow1Point.m_screen.y);
				Renderer ().MoveTo (arrowEndPoint.m_screen.x, arrowEndPoint.m_screen.y);
				Renderer ().LineTo (arrow2Point.m_screen.x, arrow2Point.m_screen.y);
				}
			}
		Renderer ().EndRender ();
		}
	}
}
//--------------------------------------------------------------------------

void CMineView::DrawLights (void) 
{
CHECKMINE;

// now show variable lights
CViewMatrix* viewMatrix = Renderer ().ViewMatrix ();
CVariableLight* flP = lightManager.VariableLight (0);

// find variable light from
Renderer ().BeginRender ();
flP = lightManager.VariableLight (0);
for (INT i = 0; i < lightManager.Count (); i++, flP++)
	if (Visible (segmentManager.Segment (flP->m_nSegment)))
	   DrawOctagon (flP->m_nSide, flP->m_nSegment);
Renderer ().EndRender ();
}

//--------------------------------------------------------------------------

void CMineView::DrawOctagon (short nSide, short nSegment) 
{
CHECKMINE;

	short xMax = ViewWidth ();
	short yMax = ViewHeight ();

if (nSegment >= 0 && nSegment <= segmentManager.Count () && nSide >= 0 && nSide <= 5) {
	CSegment* segP = segmentManager.Segment (nSegment);
	CSide* sideP = segP->Side (nSide);
	short nVertices = sideP->VertexCount ();
	if (nVertices < 3)
		return;


	CVertex vCenter = segP->ComputeCenter (nSide);
	if (!vCenter.InRange (xMax, yMax, m_nRenderer)) 
		return;

	if (ViewMatrix ()->Distance (vCenter) <= MODEL_DISPLAY_LIMIT) {
		sideP->ComputeNormals (segP->m_info.vertexIds, segP->ComputeCenter ());
		vCenter += sideP->Normal ();
		Renderer ().Sprite (&textureManager.Icon (LIGHT_ICON), vCenter, 5.0, 5.0, false);
		}
	else {
		sideP->ComputeNormals (segP->m_info.vertexIds, segP->ComputeCenter (true), true);
		CVertex vertices [4];
		short nVertices = sideP->VertexCount ();
#if 1
		CDoubleVector vOffset = sideP->Normal ();
		for (short i = 0; i < nVertices; i++)
			vertices [i].m_view = segP->Vertex (nSide, i)->m_view + vOffset;
#else
		for (short i = 0; i < nVertices; i++)
			vertices [i] = *segP->Vertex (nSide, i);
		CDoubleVector vNormal = Normal (vertices [0], vertices [1], vertices [2]); // = sideP->Normal ();
		vertices [0] -= vCenter;
		vertices [0].Normalize ();
		vertices [3] = vCenter + vNormal;
		vertices [1] = Normal (vCenter, vertices [0], vertices [3]);
		vertices [0] *= 50.0;
		vertices [1] *= 50.0;
		vertices [2] = vCenter - vertices [0];
		vertices [3] = vCenter - vertices [1];
		vertices [0] += vCenter;
		vertices [1] += vCenter;
		CDoubleVector vOffset = vNormal * 10.0;
		for (short i = 0; i < nVertices; i++) {
			vertices [i].Transform (Renderer ().ViewMatrix ());
			vertices [i].m_view += vOffset;
			}
#endif
		glDisable (GL_CULL_FACE);
		Renderer ().TexturedPolygon (&textureManager.Icon (CIRCLE_ICON), null, null, vertices, nVertices, null);
		glEnable (GL_CULL_FACE);
		}
	}
}

//----------------------------------------------------------------------------

void CMineView::DrawTunnel (void) 
{
	int h, i, j;
	CVertex points [4];
	CRenderer& renderer = DLE.MineView ()->Renderer ();

renderer.SelectPen (penRed + 1);
Renderer ().SelectObject ((HBRUSH)GetStockObject (NULL_BRUSH));
tunnelMaker.ComputeTunnel ();
renderer.BeginRender ();
points [1].Transform (ViewMatrix ());
points [1].Project (ViewMatrix ());
if (IN_RANGE (points [1].m_screen.x, m_viewMax.x) && IN_RANGE (points [1].m_screen.y, m_viewMax.y)) {
	points [0].Transform (ViewMatrix ());
	points [0].Project (ViewMatrix ());
	if (IN_RANGE (points [0].m_screen.x, m_viewMax.x) && IN_RANGE (points [0].m_screen.y, m_viewMax.y)){
		renderer.BeginRender (true);
		renderer.MoveTo (points [0].m_screen.x, points [0].m_screen.y);
		renderer.LineTo (points [1].m_screen.x, points [1].m_screen.y);
		renderer.Ellipse (points [1], 4, 4);
		renderer.EndRender ();
		renderer.BeginRender ();
		}
	}
points [2].Transform (ViewMatrix ());
points [2].Project (ViewMatrix ());
if (IN_RANGE (points [2].m_screen.x, m_viewMax.x) && IN_RANGE (points [2].m_screen.y, m_viewMax.y)){
	points [2].Transform (ViewMatrix ());
	points [2].Project (ViewMatrix ());
	if (IN_RANGE (points [3].m_screen.x, m_viewMax.x) && IN_RANGE (points [3].m_screen.y, m_viewMax.y)){
		renderer.BeginRender (true);
		renderer.MoveTo (points [3].m_screen.x, points [3].m_screen.y);
		renderer.LineTo (points [2].m_screen.x, points [2].m_screen.y);
		renderer.Ellipse (points [2].m_screen.x - 4, points [2].m_screen.y - 4, points [2].m_screen.x+4,  points [2].m_screen.y+4);
		renderer.EndRender ();
		renderer.BeginRender ();
		}
	}
j = MAX_VERTICES;
for (h = tunnelMaker.Length () * 4, i = 0; i < h; i++) {
	vertexManager [--j].Transform (ViewMatrix ());
	vertexManager [--j].Project (ViewMatrix ());
	}
CSegment *segP = segmentManager.Segment (SEGMENT_LIMIT - 1);
renderer.BeginRender (renderer.Type () == 0);
renderer.SelectPen (penBlue + 1);
for (i = 0; i < tunnelMaker.Length (); i++, segP--)
	DrawSegmentWireFrame (segP, false, 1);
renderer.EndRender ();
}

//--------------------------------------------------------------------------
//			  DrawObject ()
//
// Changed: 0=normal,1=gray,2=black
//        if (nObject == (objectManager.Count ()
//        then its a secret return point)
//--------------------------------------------------------------------------

void CMineView::SelectObjectPen (CGameObject* objP, short bClear) 
{
if (bClear == 2)
	Renderer ().SelectPen (penBlack + 1);
else {
	if ((m_selectMode == OBJECT_MODE) && (objP == current->Object ()))
		Renderer ().SelectPen (penRed + 1); // RED
	else {
		switch(objP->Type ()) {
			case OBJ_ROBOT: 
			case OBJ_CAMBOT: 
			case OBJ_EXPLOSION:
			case OBJ_MONSTERBALL:
				Renderer ().SelectPen (penMagenta + 1);
				break;
			case OBJ_SMOKE:
			case OBJ_EFFECT:
				Renderer ().SelectPen (penBoldGreen + 1);
				break;
			case OBJ_HOSTAGE: 
				Renderer ().SelectPen (penBlue + 1);
				break;
			case OBJ_PLAYER: 
				Renderer ().SelectPen (penGold + 1);
				break;
			case OBJ_WEAPON: 
				Renderer ().SelectPen (penDkGreen + 1);
				break;
			case OBJ_POWERUP: 
				Renderer ().SelectPen (penOrange + 1);
				break;
			case OBJ_REACTOR: 
				Renderer ().SelectPen (penLtGray + 1);
				break;
			case OBJ_COOP: 
				Renderer ().SelectPen (penGold + 1);
				break;
			default:
				Renderer ().SelectPen (penGreen + 1);
			}
		}
	}
}

//--------------------------------------------------------------------------

void CMineView::DrawObject (short nObject, short bClear) 
{
CHECKMINE;

	CGameObject*	objP;
	CGameObject		tempObj;

if (nObject >= 0 && nObject < objectManager.Count ()) {
	objP = objectManager.Object (nObject);
	if (!Visible (segmentManager.Segment (objP->m_info.nSegment)))
		return;
	}
else {
	// secret return
	objP = &tempObj;
	objP->Type () = -1;
	// theMine->secret_orient = Objects () [0]->orient;
	objP->m_location.orient.m.rVec = -objectManager.SecretOrient ().m.rVec;
	objP->m_location.orient.m.uVec =  objectManager.SecretOrient ().m.fVec;
	objP->m_location.orient.m.fVec =  objectManager.SecretOrient ().m.uVec;
	// objP->m_location.orient =  theMine->secret_orient;
	ushort nSegment = (ushort)objectManager.SecretSegment ();
	if (nSegment >= segmentManager.Count ())
		nSegment = 0;
	if (!Visible (segmentManager.Segment (nSegment)))
		return;
	CVertex center;
	objP->Position () = segmentManager.CalcCenter (center, nSegment); // define objP->position
	}

double d = (ViewOption (eViewTexturedWireFrame) || ViewOption (eViewTextured)) ? ViewMatrix ()->Distance (objP->Position ()) : 1e30;
if (textureManager.Available (1) && objP->HasPolyModel () && modelManager.Setup (objP, m_renderer, DC ()) && ((nObject == current->m_nObject) || (d <= MODEL_DISPLAY_LIMIT))) {
	SelectObjectPen (objP, bClear);
	if (objP->DrawArrow (Renderer (), -1)) { // only render if fully visible (check using the arrow representation)
		if (bClear)
			Renderer ().SelectPen (penBlack + 1);
		Renderer ().SelectObject ((HBRUSH) GetStockObject (BLACK_BRUSH));
		modelManager.Draw ();
		}
	}
else {
	if ((d > MODEL_DISPLAY_LIMIT) || !objP->DrawSprite (Renderer ()) || (nObject == current->m_nObject)) {
		SelectObjectPen (objP, bClear);
		objP->DrawArrow (Renderer (), (nObject == current->m_nObject));
		}
	}

if ((nObject == current->m_nObject) || (nObject == other->m_nObject))
	objP->DrawHighlight (Renderer (), (nObject == current->m_nObject));
}

//--------------------------------------------------------------------------

void CMineView::DrawObjects (short bClear) 
{
CHECKMINE;

if (!ViewObject ())
	return;

int i, j;

Renderer ().BeginRender ();
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
Renderer ().EndRender ();
}

//--------------------------------------------------------------------------

void CMineView::DrawHighlight (short bClear) 
{
CHECKMINE;

	short	currSide, currPoint;
//	short i;
//	RECT rect;

if (segmentManager.Count ()== 0) 
	return;

// draw Objects ()
if (!bClear) {
	DrawObjects (bClear);
//	if (/*!(preferences & PREFS_HIDE_MARKED_BLOCKS) ||*/ SelectMode (eSelectBlock))
	DrawMarkedSegments (bClear);
  }

Renderer ().BeginRender (Renderer ().Type () == 0);
// draw highlighted Segments () (other first, then current)
if (*current == selections [0]) {
	if (selections [0].m_nSegment != selections [1].m_nSegment)
		DrawSegment (selections [1].m_nSegment, selections [1].m_nSide, selections [1].m_nLine, selections [1].m_nPoint, bClear);
	DrawSegment (selections [0].m_nSegment, selections [0].m_nSide, selections [0].m_nLine, selections [0].m_nPoint, bClear);
	}
else {
	if (selections [0].m_nSegment != selections [1].m_nSegment)
		DrawSegment (selections [0].m_nSegment, selections [0].m_nSide, selections [0].m_nLine, selections [0].m_nPoint, bClear);
	DrawSegment (selections [1].m_nSegment, selections [1].m_nSide, selections [1].m_nLine, selections [1].m_nPoint, bClear);
	}
Renderer ().EndRender ();

if (ViewFlag (eViewMineWalls))
	DrawWalls ();
if (ViewFlag (eViewMineLights))
	DrawLights ();

tunnelMaker.Draw (Renderer (), Pen (penRed), Pen (penBlue), ViewMatrix ());

*message = '\0';
#if 0
if (preferences & PREFS_SHOW_POINT_COORDINATES) 
	{
   strcat_s (message, sizeof (message), "  point (x, y,z): (");
   short vertex = segmentManager.Segment (0) [current->m_nSegment].m_info.vertexIds [sideVertexTable [current->m_nSide][current->m_nPoint]];
	char	szCoord [20];
	sprintf_s (szCoord, sizeof (szCoord), "%1.4f,%1.4f,%1.4f)", 
				  vertexManager.Vertex (vertex)->v.x, vertexManager.Vertex (vertex)->v.y, vertexManager.Vertex (vertex)->v.z);
	strcat_s (message, sizeof (message), szCoord);
	}
else 
#endif
	{
   // calculate segment size (length between center point of opposing sides)
	strcat_s (message, sizeof (message), "  segment size: ");
	CDoubleVector center1, center2;
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
strcat_s (message, sizeof (message), ",  segment:");
_itoa_s (current->m_nSegment, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " side:");
_itoa_s ((currSide = current->m_nSide) + 1, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " point:");
_itoa_s (currPoint = current->m_nPoint, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " vertex:");
_itoa_s (current->Segment ()->m_info.vertexIds [current->Side ()->VertexIdIndex (currPoint)], message + strlen (message), sizeof (message) - strlen (message), 10);

strcat_s (message, sizeof (message), ",  textures:");
strcat_s (message, sizeof (message), " 1st:");
_itoa_s (current->Side ()->BaseTex (), message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " 2nd:");
_itoa_s (current->Side ()->OvlTex (0), message + strlen (message), sizeof (message) - strlen (message), 10);

strcat_s (message, sizeof (message), ",  zoom:");
double zoom = log (10 * Scale ().v.x) / log (zoomScales [m_nRenderer]);
if (zoom > 0) 
	zoom += 0.5;
else
	zoom -= 0.5;
sprintf_s (message + strlen (message), sizeof (message) - strlen (message),  "%1.2f", zoom);
STATUSMSG (message);
}

//------------------------------------------------------------------------------


//eof