// TexAlignTool.cpp
//

#include <math.h>
#include "stdafx.h"
#include "afxpriv.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"
#include "toolview.h"

#include "PaletteManager.h"
#include "TextureManager.h"
#include "global.h"
#include "textures.h"
#include "FileManager.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

extern short nDbgSeg, nDbgSide;
extern int nDbgVertex;

/////////////////////////////////////////////////////////////////////////////
// CToolView

static int rotMasks [4] = {0, 3, 2, 1};

//------------------------------------------------------------------------------

int round_int (int value, int round) 
{
if (value >= 0)
	value += round/2;
else
	value -= round/2;
return (value / round) * round;
}

//------------------------------------------------------------------------------

void CTextureTool::UpdateAlignWnd (void)
{
CHECKMINE;
if (m_alignWnd.m_hWnd) {
	RefreshAlignWnd ();
	m_alignWnd.InvalidateRect (null);
	m_alignWnd.UpdateWindow ();
	DLE.MineView ()->Refresh (false);
	}
}

//------------------------------------------------------------------------------

void CTextureTool::RefreshAlignWnd (void) 
{
CHECKMINE;

if (!m_alignWnd.m_hWnd)
	return;

	int			x, y, i, uv, dx, dy;
	CSegment*	segP, * childSegP;
	CSide*		sideP;
	int			nSide,
					nLine;
	CPen			hPenAxis, 
					hPenGrid;
	CPen			hPenCurrentPoint, 
					hPenCurrentLine,
					hPenCurrentSide;
	CDC			*pDC;
	CPoint		offset;
	CRgn			hRgn;

// read scroll bar
offset.x = int (m_zoom * (double) HScrollAlign ()->GetScrollPos ()) + m_centerPt.x;
offset.y = int (m_zoom * (double) VScrollAlign ()->GetScrollPos ()) + m_centerPt.y;
UpdateData (TRUE);
/*
RefreshX();
RefreshY();
RefreshAngle();
RefreshChecks();
*/
// setup drawing area
POINT	minRect, maxRect;

minRect.x = 12;
minRect.y = 10;
maxRect.x = minRect.x + 166;
maxRect.y = minRect.y + 166;
m_centerPt.x = minRect.x + 166 / 2;
m_centerPt.y = minRect.y + 166 / 2;

segP = current->Segment ();
sideP = current->Side ();
nSide = current->SideId ();
nLine = current->Edge ();
short nEdges = sideP->VertexCount ();

// get device context handle
pDC = m_alignWnd.GetDC ();

// create brush, pen, and region handles
hPenAxis.CreatePen (PS_DOT, 1, RGB (192,192,192));
hPenGrid.CreatePen (PS_DOT, 1, RGB (128,128,128));
uint select_mode = DLE.MineView ()->GetSelectMode ();
hPenCurrentPoint.CreatePen (PS_SOLID, 1, (select_mode == POINT_MODE) ? RGB (255,0,0) : RGB(255,196,0)); // red
hPenCurrentLine.CreatePen (PS_SOLID, 1, (select_mode == LINE_MODE) ? RGB (255,0,0) : RGB(255,196,0)); // red
hPenCurrentSide.CreatePen (PS_SOLID, 1, (select_mode == LINE_MODE) ? RGB (255,0,0) : RGB(0,255,0)); // red
CRect rc;
m_alignWnd.GetClientRect (rc);
minRect.x = rc.left;
minRect.y = rc.top;
maxRect.x = rc.right;
maxRect.y = rc.bottom;
m_centerPt.x = rc.Width () / 2;
m_centerPt.y = rc.Height () / 2;
hRgn.CreateRectRgn (minRect.x, minRect.y, maxRect.x, maxRect.y);

// clear texture region
i = pDC->SelectObject (&hRgn);
CBrush	brBlack (RGB (0,0,0));
brBlack.GetSafeHandle ();
hRgn.GetSafeHandle ();
pDC->FillRgn (&hRgn, &brBlack);

// draw grid
pDC->SetBkMode (TRANSPARENT);
y = 16;
for (x = -32 * y; x < 32 * y; x += 32) {
	dx = (int) (m_zoom * x);
	dy = (int) (m_zoom * y * 32);
	pDC->SelectObject((x==0) ? hPenAxis : hPenGrid);
	pDC->MoveTo ((int) (offset.x + dx), (int) (offset.y - dy));
	pDC->LineTo ((int) (offset.x + dx), (int) (offset.y + dy));
	pDC->MoveTo ((int) (offset.x - dy), (int) (offset.y + dx));
	pDC->LineTo ((int) (offset.x + dy), (int) (offset.y + dx));
	}	

if (segmentManager.IsWall ()) {
	// define array of screen points for (u,v) coordinates
	for (i = 0; i < nEdges; i++) {
		x = offset.x + (int) Round (m_zoom * sideP->m_info.uvls [i].u * 32.0);
		y = offset.y + (int) Round (m_zoom * sideP->m_info.uvls [i].v * 32.0);
		m_apts [i].x = x;
		m_apts [i].y = y;
		if (i == 0) {
			m_minPt.x = m_maxPt.x = x;
			m_minPt.y = m_maxPt.y = y;
			}
		else {
			m_minPt.x = min (m_minPt.x,x);
			m_maxPt.x = max (m_maxPt.x,x);
			m_minPt.y = min (m_minPt.y,y);
			m_maxPt.y = max (m_maxPt.y,y);
			}
		}
	m_minPt.x = max (m_minPt.x, minRect.x);
	m_maxPt.x = min (m_maxPt.x, maxRect.x);
	m_minPt.y = max (m_minPt.y, minRect.y);
	m_maxPt.y = min (m_maxPt.y, maxRect.y);

	if (m_bShowChildren) {
		short nChild, nChildLine, iChildSide, iChildLine;
		ushort vert0, vert1, childVert0, childVert1;
		int	x0, y0;
		POINT childPoints [4];

		CEdgeList edgeList;
		segP->BuildEdgeList (edgeList);

		// draw all sides (u,v)
		pDC->SelectObject (hPenGrid);
		for (nChildLine = 0; nChildLine < nEdges; nChildLine++) {
			ubyte s1, s2, 
				  v1 = sideP->VertexIdIndex (nChildLine), 
				  v2 = sideP->VertexIdIndex (nChildLine + 1);
			if (0 > edgeList.Find (0, s1, s2, v1, v2))
				continue;
			vert0 = segP->VertexId (v1);
			vert1 = segP->VertexId (v2);
			nChild = segP->ChildId ((s1 == nSide) ? s2 : s1);
			if (nChild > -1) {
				childSegP = segmentManager.Segment (nChild);
				// figure out which side of child shares two points w/ current->side
				for (iChildSide = 0; iChildSide < 6; iChildSide++) {
					// ignore children of different textures (or no texture)
					CSide *childSideP = childSegP->m_sides + iChildSide;
					if (childSideP->Shape () > SIDE_SHAPE_TRIANGLE)
						continue;
					short nChildEdges = childSideP->VertexCount ();
					if (segmentManager.IsWall (CSideKey (nChild, iChildSide)) && (childSideP->BaseTex () == sideP->BaseTex ())) {
						for (iChildLine = 0; iChildLine < nChildEdges; iChildLine++) {
							childVert0 = childSegP->VertexId (iChildSide, iChildLine);
							childVert1 = childSegP->VertexId (iChildSide, iChildLine + 1);
							// if both points of line == either point of parent
							if ((childVert0 == vert0 && childVert1 == vert1) ||
								 (childVert0 == vert1 && childVert1 == vert0)) {

								// now we know the child's side & line which touches the parent
								// so, we need to translate the child's points by even increments
								// ..of the texture size in order to make it line up on the screen
								// start by copying points into an array
								for (i = 0; i < nChildEdges; i++) {
									x = offset.x + (int) Round (m_zoom * childSideP->m_info.uvls [i].u * 32.0);
									y = offset.y + (int) Round (m_zoom * childSideP->m_info.uvls [i].v * 32.0);
									childPoints [i].x = x;
									childPoints [i].y = y;
									}
								// now, calculate offset
								uv = (iChildLine + 1) % nChildEdges;
								x0 = childPoints [uv].x - m_apts [nChildLine].x;
								y0 = childPoints [uv].y - m_apts [nChildLine].y;
								x0 = round_int (x0, int (32.0 * m_zoom));
								y0 = round_int (y0, int (32.0 * m_zoom));
								// translate child points
								for (i = 0; i < nChildEdges; i++) {
									childPoints [i].x -= x0;
									childPoints [i].y -= y0;
									}
								// draw child (u,v)
								pDC->SelectObject (hPenCurrentPoint); // color = cyan
								pDC->MoveTo (childPoints [nChildEdges - 1].x,childPoints [nChildEdges - 1].y);
								for (i = 0; i < nChildEdges; i++) {
									pDC->LineTo (childPoints [i].x, childPoints [i].y);
									}
								}
							}
						}
					}
				}
			}
		}

	// highlight current point
	pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
	pDC->SelectObject (hPenCurrentPoint);
	x = m_apts [current->Point ()].x;
	y = m_apts [current->Point ()].y;
	pDC->Ellipse((int) (x - 4 * m_zoom), (int) (y - 4 * m_zoom), (int) (x + 4 * m_zoom), (int) (y + 4 * m_zoom));
	// fill in texture
	DrawAlignment (pDC);
	pDC->SelectObject (hRgn);
	// draw CUVL
	pDC->SelectObject (hPenCurrentSide);
	pDC->MoveTo (m_apts [nEdges - 1].x, m_apts [nEdges - 1].y);
	for (i = 0; i < nEdges; i++)
		pDC->LineTo (m_apts [i].x, m_apts [i].y);
	// highlight current line
	pDC->SelectObject(hPenCurrentLine);
	pDC->MoveTo (m_apts [nLine].x, m_apts [nLine].y);
	pDC->LineTo (m_apts [(nLine + 1) % nEdges].x, m_apts [(nLine + 1) % nEdges].y);
	}

// release dc
m_alignWnd.ReleaseDC (pDC);
// delete Objects ()
DeleteObject(hRgn);
DeleteObject(hPenCurrentSide);
DeleteObject(hPenCurrentLine);
DeleteObject(hPenCurrentPoint);
DeleteObject(hPenAxis);
DeleteObject(hPenGrid);
}

//------------------------------------------------------------------------------

void CTextureTool::DrawAlignment (CDC *pDC)
{
CHECKMINE;
if (!m_bShowTexture)
	return;

	CPalette*	oldPalette;
	CRgn			hRgn;
	int			h, i, j, x, y;
	POINT			offset;
	CSide*		sideP = current->Side ();
	CTexture		tex (textureManager.m_bmBuf);
	ushort		scale;

offset.x = (int) (m_zoom * (double) HScrollAlign ()->GetScrollPos ()) + m_centerPt.x - 128;
offset.y = (int) (m_zoom * (double) VScrollAlign ()->GetScrollPos ()) + m_centerPt.y - 128;

memset (tex.Buffer (), 0, sizeof (textureManager.m_bmBuf));
if (textureManager.BlendTextures (sideP->BaseTex (), sideP->OvlTex (), &tex, 0, 0)) {
	DEBUGMSG (" Texture tool: Texture not found (textureManager.BlendTextures failed)");
	return;
	}
oldPalette = pDC->SelectPalette (paletteManager.Render (), FALSE);
pDC->RealizePalette();
hRgn.CreatePolygonRgn (m_apts, sideP->VertexCount (), ALTERNATE);
pDC->SelectObject (&hRgn);
uint w = tex.Width (0);
scale = w / 64;
for (x = m_minPt.x; x < m_maxPt.x; x++) {
	for (y = m_minPt.y; y < m_maxPt.y; y++) {
		i = ((int) ((((x - offset.x) * 2) / m_zoom)) % 64) * scale;
		j = ((int) ((((y - offset.y) * 2) / m_zoom)) % 64) * scale;
		pDC->SetPixel (x, y, h = tex.Buffer ((w - j) * w + i)->ColorRef ());
		}
	}
DeleteObject(hRgn);
pDC->SelectPalette (oldPalette, FALSE);
}


//------------------------------------------------------------------------------

void CTextureTool::OnAlignX ()
{
UpdateData (TRUE);

	CSide*	sideP = current->Side ();
	double	delta = sideP->m_info.uvls [current->Point ()].u - m_alignX / 20.0;

if (delta) {
	UpdateData (TRUE);
	undoManager.Begin (__FUNCTION__, udSegments, true);
	switch (DLE.MineView ()->GetSelectMode ()) {
		case POINT_MODE:
			sideP->m_info.uvls [current->Point ()].u -= delta;
			break;
		case LINE_MODE:
			sideP->m_info.uvls [current->Edge ()].u -= delta;
			sideP->m_info.uvls [(current->Edge () + 1) % sideP->VertexCount ()].u -= delta;
			break;
		default:
			for (int i = 0; i < sideP->VertexCount (); i++)
				sideP->m_info.uvls [i].u -= delta;
		}  
	undoManager.End (__FUNCTION__);
	UpdateAlignWnd ();
	}
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignY ()
{
UpdateData (TRUE);

	CSide*	sideP = current->Side ();
	double	delta = sideP->m_info.uvls [current->Point ()].v - m_alignY / 20.0;

if (delta) {
	UpdateData (TRUE);
	undoManager.Begin (__FUNCTION__, udSegments, true);
	switch (DLE.MineView ()->GetSelectMode ()) {
		case POINT_MODE:
			sideP->m_info.uvls [current->Point ()].v -= delta;
			break;
		case LINE_MODE:
			sideP->m_info.uvls [current->Edge ()].v -= delta;
			sideP->m_info.uvls [(current->Edge () + 1) % sideP->VertexCount ()].v -= delta;
			break;
		default:
			for (int i = 0; i < sideP->VertexCount (); i++)
				sideP->m_info.uvls [i].v -= delta;
		}  
	undoManager.End (__FUNCTION__);
	UpdateAlignWnd ();
	}
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignRot ()
{
UpdateData (TRUE);
  
	double	delta, dx, dy, angle;
	CSide*	sideP = current->Side ();

dx = sideP->m_info.uvls [1].u - sideP->m_info.uvls [0].u;
dy = sideP->m_info.uvls [1].v - sideP->m_info.uvls [0].v;
angle = (dx || dy) ? atan3 (dy, dx) - M_PI_2 : 0;
delta = angle - m_alignAngle * PI / 180.0;
RotateUV (delta, FALSE);
}

//------------------------------------------------------------------------------

void CTextureTool::RefreshAlignment ()
{
	CSide* sideP = current->Side ();

m_alignX = sideP->m_info.uvls [current->Point ()].u * 20.0;
m_alignY = sideP->m_info.uvls [current->Point ()].v * 20.0;

double dx = sideP->m_info.uvls [1].u - sideP->m_info.uvls [0].u;
double dy = sideP->m_info.uvls [1].v - sideP->m_info.uvls [0].v;
m_alignAngle = ((dx || dy) ? atan3 (dy,dx) - M_PI_2 : 0) * 180.0 / M_PI;
if (m_alignAngle < 0)
	m_alignAngle += 360.0;
else if (m_alignAngle > 360.9)
	m_alignAngle -= 360.0;
int h = sideP->OvlAlignment ();
for (m_alignRot2nd = 0; m_alignRot2nd < 4; m_alignRot2nd++)
	if (rotMasks [m_alignRot2nd] == h)
		break;
}

//------------------------------------------------------------------------------

void CTextureTool::RotateUV (double angle, bool bUpdate)
{
	CSide*	sideP = current->Side ();

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments, true);
for (int i = 0; i < sideP->VertexCount (); i++) 
	sideP->Uvls (i)->Rotate (angle);
if (bUpdate)
	m_alignAngle -= angle * 180.0 / PI;
UpdateData (FALSE);
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::HFlip (void)
{
	CSide*	sideP = current->Side ();

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
switch (DLE.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		break;
	case LINE_MODE:
		Swap (sideP->m_info.uvls [current->Edge ()].u, sideP->m_info.uvls [(current->Edge () + 1) % sideP->VertexCount ()].u);
		break;
	default:
		for (int i = 0; i < 2; i++) 
			Swap (sideP->m_info.uvls [i].u, sideP->m_info.uvls [i + 2].u);
	}
UpdateData (FALSE);
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::VFlip (void)
{
	CSide*	sideP = current->Side ();

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
switch (DLE.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		break;
	case LINE_MODE:
		Swap (sideP->m_info.uvls [current->Edge ()].v, sideP->m_info.uvls [(current->Edge () + 1) % sideP->VertexCount ()].v);
		break;
	default:
		for (int i = 0; i < 2; i++) 
			Swap (sideP->m_info.uvls [i].v, sideP->m_info.uvls [i + 2].v);
	}
UpdateData (FALSE);
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::HAlign (int dir)
{
	CSide*	sideP = current->Side ();
	double	delta = DLE.MineView ()->MineMoveRate () / 8.0 / m_zoom * dir;

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
switch (DLE.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		sideP->m_info.uvls [current->Point ()].u += delta;
		break;
	case LINE_MODE:
		sideP->m_info.uvls [current->Edge ()].u += delta;
		sideP->m_info.uvls [(current->Edge () + 1) % sideP->VertexCount ()].u += delta;
		break;
	default:
		for (int i = 0; i < sideP->VertexCount (); i++)
			sideP->m_info.uvls [i].u += delta;
	}
m_alignX = (double) sideP->m_info.uvls [current->Point ()].u * 20.0;
UpdateData (FALSE);
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::VAlign (int dir)
{
	CSide*	sideP = current->Side ();
	double	delta = DLE.MineView ()->MineMoveRate () / 8.0 / m_zoom * dir;

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
switch (DLE.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		sideP->m_info.uvls [current->Point ()].v += delta;
		break;
	case LINE_MODE:
		sideP->m_info.uvls [current->Edge ()].v += delta;
		sideP->m_info.uvls [(current->Edge () + 1) % sideP->VertexCount ()].v += delta;
		break;
	default:
		for (int i = 0; i < sideP->VertexCount (); i++)
			sideP->m_info.uvls [i].v += delta;
	}
m_alignY = (double)sideP->m_info.uvls [current->Point ()].v * 20.0;
UpdateData (FALSE);
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnHFlip (void)
{
HFlip ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnVFlip (void)
{
VFlip ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignLeft (void)
{
HAlign (-1);
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignRight (void)
{
HAlign (1);
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignUp (void)
{
VAlign (-1);
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignDown (void)
{
VAlign (1);
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignRotLeft (void)
{
RotateUV (theMine->RotateRate ());
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignRotRight (void)
{
RotateUV (-theMine->RotateRate ());
}

//------------------------------------------------------------------------------

void CTextureTool::OnHShrink ()
{
	CSide		*sideP = current->Side ();
	int		h = sideP->VertexCount ();
	double	delta = DLE.MineView ()->MineMoveRate () / 8.0 / m_zoom;

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
sideP->m_info.uvls [0].u -= delta;
if (h > 1) {
	sideP->m_info.uvls [1].u -= delta;
	if (h > 2) {
		sideP->m_info.uvls [2].u += delta;
		if (h > 3) 
			sideP->m_info.uvls [3].u += delta;
		}
	}
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnVShrink ()
{
	CSide*	sideP = current->Side ();
	int		h = sideP->VertexCount ();
	double	delta = DLE.MineView ()->MineMoveRate () / 8.0 / m_zoom;

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
sideP->m_info.uvls [0].v += delta;
if (h > 1) {
	sideP->m_info.uvls [3].v += delta;
	if (h > 2) {
		sideP->m_info.uvls [1].v -= delta;
		if (h > 3) 
			sideP->m_info.uvls [2].v -= delta;
		}
	}
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignReset ()
{
UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
current->Segment ()->SetUV (current->SideId (), 0.0, 0.0);
m_alignX = 0.0;
m_alignY = 0.0;
m_alignAngle = 0.0;
Rot2nd (0);
UpdateData (FALSE);
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignResetTagged ()
{	
	short nWalls = wallManager.WallCount ();
	BOOL bModified = FALSE;

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
CSegment* segP = segmentManager.Segment (0);
int nSegments = segmentManager.Count ();
for (int nSegment = 0; nSegment < nSegments; nSegment++, segP++) {
	CSide* sideP = segP->Side (0);
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		if (sideP->Shape () > SIDE_SHAPE_TRIANGLE) 
			continue;
		if (segmentManager.IsTagged (CSideKey (nSegment, nSide))) {
			if ((segP->ChildId (nSide) == -1) || (segP->m_sides [nSide].m_info.nWall < nWalls)) {
				segP->m_sides [nSide].m_info.nOvlTex &= TEXTURE_MASK; // rotate 0
				segP->SetUV (nSide, 0.0, 0.0);
				bModified = TRUE;
				}
			}
		}
	}
undoManager.End (__FUNCTION__);
DLE.MineView ()->Refresh (false);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignStretch2Fit ()
{
	CSide*		sideP = current->Side ();
	CSegment*	segP;
	short			nSegment, nSide;
	int			i;

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
if (!segmentManager.HasTaggedSides ()) {
	for (i = 0; i < sideP->VertexCount (); i++) {
		sideP->m_info.uvls [i].u = defaultUVLs [i].u;
		sideP->m_info.uvls [i].v = defaultUVLs [i].v;
		}
	}
else {
	for (nSegment = 0, segP = segmentManager.Segment (0); nSegment < segmentManager.Count (); nSegment++, segP++) {
		for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++) {
			if (segmentManager.IsTagged (CSideKey (nSegment, nSide))) {
				for (i = 0; i < sideP->VertexCount (); i++) {
					sideP->m_info.uvls [i].u = defaultUVLs [i].u;
					sideP->m_info.uvls [i].v = defaultUVLs [i].v;
					}
				}
			}
		}
	}
undoManager.End (__FUNCTION__);
DLE.MineView ()->Refresh (false);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignAll (void)
{
// set all segment sides as not aligned yet
	CSegment	* currSeg = current->Segment (),
				* segP = segmentManager.Segment (0);
	CSide		* sideP = current->Side (),
				*childSideP;
	short		nSegment, 
				nSide = current->SideId (),
				nChildLine = 3;
	double	sAngle, cAngle, angle; 

UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
bool bAll = !segmentManager.HasTaggedSegments (true);
for (nSegment = 0, segP = segmentManager.Segment (0); nSegment < segmentManager.Count (); nSegment++, segP++)
	 segP->Index () = 0;
for (nSegment = 0, segP = segmentManager.Segment (0); nSegment < segmentManager.Count (); nSegment++, segP++) {
	if (segP->Index ())
		continue;
	childSideP = segP->m_sides + nSide;
	if (childSideP->Shape () > SIDE_SHAPE_TRIANGLE)
		continue;
	if (m_bUse1st && (sideP->BaseTex () != childSideP->BaseTex ()))
		continue;
	if (m_bUse2nd && (sideP->OvlTex (0) != childSideP->OvlTex (0)))
		continue;
	if (!(bAll || segmentManager.IsTagged (CSideKey (nSegment, nSide))))
		continue;
	if (nSegment != current->SegmentId ()) {
		segmentManager.Segment (nSegment)->SetUV (nSide, 0, 0);
		sAngle = atan3 (sideP->m_info.uvls [(nChildLine + 1) % sideP->VertexCount ()].v - sideP->m_info.uvls [nChildLine].v, 
							 sideP->m_info.uvls [(nChildLine + 1) % sideP->VertexCount ()].u - sideP->m_info.uvls [nChildLine].u); 
		cAngle = atan3 (childSideP->m_info.uvls [nChildLine].v - childSideP->m_info.uvls [(nChildLine + 1) % sideP->VertexCount ()].v, 
							 childSideP->m_info.uvls [nChildLine].u - childSideP->m_info.uvls [(nChildLine + 1) % sideP->VertexCount ()].u); 
		// now rotate childs (u, v) coords around child_point1 (cAngle - sAngle)
		angle = cAngle - sAngle;
		for (int i = 0; i < childSideP->VertexCount (); i++) 
			childSideP->m_info.uvls [i].Rotate (angle); 
		}
	segmentManager.AlignTextures (nSegment, nSide, m_bUse1st, m_bUse2nd, m_bIgnorePlane, false, false);
	}
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignChildren ()
{
// set all segment sides as not aligned yet
UpdateData (TRUE);
undoManager.Begin (__FUNCTION__, udSegments);
// the alignment function will take care of only aligning tagged sides (provided they are all connected)
segmentManager.AlignTextures (current->SegmentId (), current->SideId (), m_bUse1st, m_bUse2nd, m_bIgnorePlane, true, false);
undoManager.End (__FUNCTION__);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnZoomIn ()
{
if (m_zoom < 16.0) {
	m_zoom *= 2.0;
	UpdateAlignWnd ();
	}
}

//------------------------------------------------------------------------------

void CTextureTool::OnZoomOut ()
{
if (m_zoom > 1.0/16.0) {
	m_zoom /= 2.0;
	UpdateAlignWnd ();
	}
}

//------------------------------------------------------------------------------

void CTextureTool::Rot2nd (int iAngle)
{
	CSide *sideP = current->Side ();
 
if ((sideP->OvlTex (0)) && ((sideP->OvlAlignment ()) != rotMasks [iAngle])) {
	undoManager.Begin (__FUNCTION__, udSegments);
	sideP->m_info.nOvlTex &= TEXTURE_MASK;
   sideP->m_info.nOvlTex |= (rotMasks [iAngle] << 14);
	m_alignRot2nd = iAngle;
	UpdateData (FALSE);
	undoManager.End (__FUNCTION__);
	UpdateAlignWnd ();
	}
}

//------------------------------------------------------------------------------

void CTextureTool::OnRot2nd0 ()
{
Rot2nd (0);
}

//------------------------------------------------------------------------------

void CTextureTool::OnRot2nd90 ()
{
Rot2nd (1);
}

//------------------------------------------------------------------------------

void CTextureTool::OnRot2nd180 ()
{
Rot2nd (2);
}

//------------------------------------------------------------------------------

void CTextureTool::OnRot2nd270 ()
{
Rot2nd (3);
}

//------------------------------------------------------------------------------
		
void CTextureTool::OnAlignIgnorePlane ()
{
m_bIgnorePlane = !m_bIgnorePlane;
Refresh ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignIgnoreWalls ()
{
m_bIgnoreWalls = !m_bIgnoreWalls;
Refresh ();
}

//------------------------------------------------------------------------------

//eof TexAlignTool.cpp