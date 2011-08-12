// dlcView.cpp : implementation of the CMineView class
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

#define UV_FACTOR ((double)640.0/(double)0x10000L)

/////////////////////////////////////////////////////////////////////////////
// CToolView

static int rotMasks [4] = {0x0000, 0xC000, 0x8000, 0x4000};

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
RefreshAlignWnd ();
m_alignWnd.InvalidateRect (null);
m_alignWnd.UpdateWindow ();
DLE.MineView ()->Refresh (false);
}

//------------------------------------------------------------------------------

void CTextureTool::RefreshAlignWnd (void) 
{
CHECKMINE;

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
// each side has 4 children (ordered by side's line number)
	static int sideChildTable[6][4] = {
		{4,3,5,1},//{5,1,4,3},
		{2,4,0,5},//{5,0,4,2},
		{5,3,4,1},//{5,3,4,1},
		{0,4,2,5},//{5,0,4,2},
		{2,3,0,1},//{2,3,0,1},
		{0,3,2,1} //{2,3,0,1}
		};


// read scroll bar
offset.x = (int)(m_zoom * (double) HScrollAlign ()->GetScrollPos ()) + m_centerPt.x;
offset.y = (int)(m_zoom * (double) VScrollAlign ()->GetScrollPos ()) + m_centerPt.y;
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
nSide = current->m_nSide;
nLine = current->m_nLine;

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
	for (i = 0; i < 4; i++) {
		x = offset.x + (int)(m_zoom * (double)sideP->m_info.uvls[i].u / 64.0);
		y = offset.y + (int)(m_zoom * (double)sideP->m_info.uvls[i].v / 64.0);
		m_apts [i].x = x;
		m_apts [i].y = y;
		if (i == 0) {
			m_minPt.x = m_maxPt.x = x;
			m_minPt.y = m_maxPt.y = y;
			}
		else {
			m_minPt.x = min(m_minPt.x,x);
			m_maxPt.x = max(m_maxPt.x,x);
			m_minPt.y = min(m_minPt.y,y);
			m_maxPt.y = max(m_maxPt.y,y);
			}
		}
	m_minPt.x = max(m_minPt.x, minRect.x);
	m_maxPt.x = min(m_maxPt.x, maxRect.x);
	m_minPt.y = max(m_minPt.y, minRect.y);
	m_maxPt.y = min(m_maxPt.y, maxRect.y);

	if (m_bShowChildren) {
		short nChildSide, nChild, nChildLine;
		int point0, point1, vert0, vert1;
		short iChildSide, iChildLine;
		short childPoint0, childPoint1, childVert0, childVert1;
		int x0, y0;
		POINT child_pts[4];

		// draw all sides (u,v)
		pDC->SelectObject (hPenGrid);
		for (nChildLine = 0; nChildLine < 4; nChildLine++) {
			// find vert numbers for the line's two end points
			point0 = lineVertTable [sideLineTable [nSide][nChildLine]][0];
			point1 = lineVertTable [sideLineTable [nSide][nChildLine]][1];
			vert0  = segP->m_info.verts [point0];
			vert1  = segP->m_info.verts [point1];

			// check child for this line 
			nChildSide = sideChildTable[nSide][nChildLine];
			nChild = segP->Child (nChildSide);
			if (nChild > -1) {
				childSegP = segmentManager.Segment (nChild);
				// figure out which side of child shares two points w/ current->side
				for (iChildSide = 0; iChildSide < 6; iChildSide++) {
					// ignore children of different textures (or no texture)
					CSide *childSideP = childSegP->m_sides + iChildSide;
					if (segmentManager.IsWall (CSideKey (nChild, iChildSide)) &&
						 (childSideP->m_info.nBaseTex == sideP->m_info.nBaseTex)) {
						for (iChildLine=0;iChildLine<4;iChildLine++) {
							// find vert numbers for the line's two end points
							childPoint0 = lineVertTable [sideLineTable [iChildSide][iChildLine]][0];
							childPoint1 = lineVertTable [sideLineTable [iChildSide][iChildLine]][1];
							childVert0  = childSegP->m_info.verts [childPoint0];
							childVert1  = childSegP->m_info.verts [childPoint1];
							// if both points of line == either point of parent
							if ((childVert0 == vert0 && childVert1 == vert1) ||
								 (childVert0 == vert1 && childVert1 == vert0)) {

								// now we know the child's side & line which touches the parent
								// so, we need to translate the child's points by even increments
								// ..of the texture size in order to make it line up on the screen
								// start by copying points into an array
								for (i = 0; i < 4; i++) {
									x = offset.x + (int)(m_zoom*(double)childSideP->m_info.uvls[i].u/64.0);
									y = offset.y + (int)(m_zoom*(double)childSideP->m_info.uvls[i].v/64.0);
									child_pts[i].x = x;
									child_pts[i].y = y;
									}
								// now, calculate offset
								uv = (iChildLine+1)&3;
								x0 = child_pts[uv].x - m_apts [nChildLine].x;
								y0 = child_pts[uv].y - m_apts [nChildLine].y;
								x0 = round_int(x0,(int)(32.0*m_zoom));
								y0 = round_int(y0,(int)(32.0*m_zoom));
								// translate child points
								for (i=0;i<4;i++) {
									child_pts[i].x -= x0;
									child_pts[i].y -= y0;
									}
								// draw child (u,v)
								pDC->SelectObject (hPenCurrentPoint); // color = cyan
								pDC->MoveTo (child_pts[3].x,child_pts[3].y);
								for (i = 0; i < 4; i++) {
									pDC->LineTo (child_pts[i].x,child_pts[i].y);
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
	x = m_apts [current->m_nPoint].x;
	y = m_apts [current->m_nPoint].y;
	pDC->Ellipse((int) (x-4*m_zoom), (int) (y-4*m_zoom), (int) (x+4*m_zoom), (int) (y+4*m_zoom));
	// fill in texture
	DrawAlignment (pDC);
	pDC->SelectObject (hRgn);
	// draw CUVL
	pDC->SelectObject (hPenCurrentSide);
	pDC->MoveTo (m_apts [3].x, m_apts [3].y);
	for (i = 0; i < 4; i++)
		pDC->LineTo (m_apts [i].x, m_apts [i].y);
	// highlight current line
	pDC->SelectObject(hPenCurrentLine);
	pDC->MoveTo (m_apts [nLine].x, m_apts [nLine].y);
	pDC->LineTo (m_apts [(nLine+1)&3].x, m_apts [(nLine+1)&3].y);
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

	CPalette		*oldPalette;
	CRgn			hRgn;
	int			h, i, j, x, y;
	POINT			offset;
	CSide*		sideP = current->Side ();
	CTexture		tex (textureManager.m_bmBuf);
	ushort		scale;

offset.x = (int) (m_zoom * (double) HScrollAlign ()->GetScrollPos ()) + m_centerPt.x - 128;
offset.y = (int) (m_zoom * (double) VScrollAlign ()->GetScrollPos ()) + m_centerPt.y - 128;

memset (tex.Buffer (), 0, sizeof (textureManager.m_bmBuf));
if (textureManager.BlendTextures (sideP->m_info.nBaseTex, sideP->m_info.nOvlTex, &tex, 0, 0)) {
	DEBUGMSG (" Texture tool: Texture not found (textureManager.BlendTextures failed)");
	return;
	}
oldPalette = pDC->SelectPalette (paletteManager.Render (), FALSE);
pDC->RealizePalette();
hRgn.CreatePolygonRgn (m_apts, sizeof (m_apts) / sizeof (POINT), ALTERNATE);
pDC->SelectObject (&hRgn);
scale = min (tex.Width (), tex.m_info.height) / 64;
for (x = m_minPt.x; x < m_maxPt.x; x++) {
	for (y = m_minPt.y; y < m_maxPt.y; y++) {
		i = ((int) ((((x - offset.x) * 2) / m_zoom)) & 63) * scale;
		j = ((int) ((((y - offset.y) * 2) / m_zoom)) & 63) * scale;
		pDC->SetPixel (x, y, h = tex.Buffer () [(tex.Width () - j) * tex.Width () + i].ColorRef ());
		}
	}
DeleteObject(hRgn);
pDC->SelectPalette (oldPalette, FALSE);
}


//------------------------------------------------------------------------------

void CTextureTool::OnAlignX ()
{
UpdateData (TRUE);

	int i,	delta;
	CSide	*sideP = current->Side ();

if (delta = (int) (sideP->m_info.uvls [current->m_nPoint].u - m_alignX / UV_FACTOR)) {
	UpdateData (TRUE);
	undoManager.Begin (udSegments);
	switch (DLE.MineView ()->GetSelectMode ()) {
		case POINT_MODE:
			sideP->m_info.uvls[current->m_nPoint].u -= delta;
			break;
		case LINE_MODE:
			sideP->m_info.uvls[current->m_nLine].u -= delta;
			sideP->m_info.uvls[(current->m_nLine+1)&3].u -= delta;
			break;
		default:
			for (i = 0; i < 4; i++)
				sideP->m_info.uvls[i].u -= delta;
		}  
	undoManager.End ();
	UpdateAlignWnd ();
	}
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignY ()
{
UpdateData (TRUE);

	int i, delta;
	CSide	*sideP = current->Side ();

if (delta = (int) (sideP->m_info.uvls [current->m_nPoint].v - m_alignY / UV_FACTOR)) {
	UpdateData (TRUE);
	undoManager.Begin (udSegments);
	switch (DLE.MineView ()->GetSelectMode ()) {
		case POINT_MODE:
			sideP->m_info.uvls[current->m_nPoint].v -= delta;
			break;
		case LINE_MODE:
			sideP->m_info.uvls[current->m_nLine].v -= delta;
			sideP->m_info.uvls[(current->m_nLine+1)&3].v -= delta;
			break;
		default:
			for (i = 0; i < 4; i++)
				sideP->m_info.uvls[i].v -= delta;
		}  
	undoManager.End ();
	UpdateAlignWnd ();
	}
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignRot ()
{
UpdateData (TRUE);
  
	double delta,dx,dy,angle;
	CSide	*sideP = current->Side ();

dx = sideP->m_info.uvls[1].u - sideP->m_info.uvls[0].u;
dy = sideP->m_info.uvls[1].v - sideP->m_info.uvls[0].v;
angle = (dx || dy) ? atan3 (dy,dx) - M_PI_2 : 0;
delta = angle - m_alignAngle * PI / 180.0;
RotateUV (delta, FALSE);
}

//------------------------------------------------------------------------------

void CTextureTool::RefreshAlignment ()
{
CSide * sideP = current->Side ();

m_alignX = (double) sideP->m_info.uvls [current->m_nPoint].u * UV_FACTOR;
m_alignY = (double) sideP->m_info.uvls [current->m_nPoint].v * UV_FACTOR;

double dx = sideP->m_info.uvls [1].u - sideP->m_info.uvls [0].u;
double dy = sideP->m_info.uvls [1].v - sideP->m_info.uvls [0].v;
m_alignAngle = ((dx || dy) ? atan3 (dy,dx) - M_PI_2 : 0) * 180.0 / M_PI;
if (m_alignAngle < 0)
	m_alignAngle += 360.0;
else if (m_alignAngle > 360)
	m_alignAngle -= 360.0;
int h = sideP->m_info.nOvlTex & 0xC000;
for (m_alignRot2nd = 0; m_alignRot2nd < 4; m_alignRot2nd++)
	if (rotMasks [m_alignRot2nd] == h)
		break;
}

//------------------------------------------------------------------------------

void CTextureTool::RotateUV (double angle, bool bUpdate)
{
	int		i;
	double	x, y, a, radius;
	CSide	*	sideP = current->Side ();

UpdateData (TRUE);
undoManager.Begin (udSegments);
for (i = 0; i < 4; i++) {
	// convert to polar coordinates
	x = sideP->m_info.uvls[i].u;
	y = sideP->m_info.uvls[i].v;
	if (x || y) {
		radius = sqrt(x*x + y*y);
		a = atan3 (y,x) - angle;			// add rotation
		// convert back to rectangular coordinates
		x = radius * cos (a);
		y = radius * sin (a);
		sideP->m_info.uvls[i].u = (short) x;
		sideP->m_info.uvls[i].v = (short) y;
		}
	}
if (bUpdate)
	m_alignAngle -= angle * 180.0 / PI;
UpdateData (FALSE);
undoManager.End ();
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::HFlip (void)
{
	CSide	*sideP = current->Side ();
	short		h, i, l;

UpdateData (TRUE);
undoManager.Begin (udSegments);
switch (DLE.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		break;
	case LINE_MODE:
		l = current->m_nLine;
		h = sideP->m_info.uvls [l].u;
		sideP->m_info.uvls [l].u = sideP->m_info.uvls [(l + 1) & 3].u;
		sideP->m_info.uvls [(l + 1) & 3].u = h;
		break;
	default:
		for (i = 0; i < 2; i++) {
			h = sideP->m_info.uvls[i].u;
			sideP->m_info.uvls[i].u = sideP->m_info.uvls[i + 2].u;
			sideP->m_info.uvls[i + 2].u = h;
			}
	}
UpdateData (FALSE);
undoManager.End ();
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::VFlip (void)
{
	CSide	*sideP = current->Side ();
	short		h, i, l;

UpdateData (TRUE);
undoManager.Begin (udSegments);
switch (DLE.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		break;
	case LINE_MODE:
		l = current->m_nLine;
		h = sideP->m_info.uvls [l].v;
		sideP->m_info.uvls [l].v = sideP->m_info.uvls [(l + 1) & 3].v;
		sideP->m_info.uvls [(l + 1) & 3].v = h;
		break;
	default:
		for (i = 0; i < 2; i++) {
			h = sideP->m_info.uvls[i].v;
			sideP->m_info.uvls[i].v = sideP->m_info.uvls[i + 2].v;
			sideP->m_info.uvls[i + 2].v = h;
			}
	}
UpdateData (FALSE);
undoManager.End ();
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::HAlign (int dir)
{
	int		i;
	CSide	*sideP = current->Side ();
	double	delta = moveRate * (0x0800 / 8) / m_zoom * dir;

UpdateData (TRUE);
undoManager.Begin (udSegments);
switch (DLE.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		sideP->m_info.uvls[current->m_nPoint].u += (short) delta;
		break;
	case LINE_MODE:
		sideP->m_info.uvls[current->m_nLine].u += (short) delta;
		sideP->m_info.uvls[(current->m_nLine+1)&3].u += (short) delta;
		break;
	default:
		for (i=0;i<4;i++)
			sideP->m_info.uvls[i].u += (short) delta;
	}
m_alignX = (double) sideP->m_info.uvls [current->m_nPoint].u * UV_FACTOR;
UpdateData (FALSE);
undoManager.End ();
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::VAlign (int dir)
{
	int		i;
	CSide	*sideP = current->Side ();
	double	delta = moveRate * (0x0800 / 8) / m_zoom * dir;

UpdateData (TRUE);
undoManager.Begin (udSegments);
switch (DLE.MineView ()->GetSelectMode ()) {
	case POINT_MODE:
		sideP->m_info.uvls[current->m_nPoint].v += (short) delta;
		break;
	case LINE_MODE:
		sideP->m_info.uvls[current->m_nLine].v += (short) delta;
		sideP->m_info.uvls[(current->m_nLine+1)&3].v += (short) delta;
		break;
	default:
		for (i=0;i<4;i++)
			sideP->m_info.uvls[i].v += (short) delta;
	}
m_alignY = (double)sideP->m_info.uvls[current->m_nPoint].v * UV_FACTOR;
UpdateData (FALSE);
undoManager.End ();
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
RotateUV (angleRate);
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignRotRight (void)
{
RotateUV (-angleRate);
}

//------------------------------------------------------------------------------

void CTextureTool::OnHShrink ()
{
	int		i = current->m_nPoint;
	CSide	*sideP = current->Side ();
	double	delta = moveRate * 256 / m_zoom ;

UpdateData (TRUE);
undoManager.Begin (udSegments);
sideP->m_info.uvls [0].u -= (short) delta;
sideP->m_info.uvls[1].u -= (short) delta;
sideP->m_info.uvls [2].u += (short) delta;
sideP->m_info.uvls[3].u += (short) delta;
undoManager.End ();
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnVShrink ()
{
	int		i = current->m_nPoint;
	CSide	*sideP = current->Side ();
	double	delta = moveRate * 256 / m_zoom;

UpdateData (TRUE);
undoManager.Begin (udSegments);
sideP->m_info.uvls [0].v += (short) delta;
sideP->m_info.uvls[3].v += (short) delta;
sideP->m_info.uvls [1].v -= (short) delta;
sideP->m_info.uvls[2].v -= (short) delta;
undoManager.End ();
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignReset ()
{
UpdateData (TRUE);
undoManager.Begin (udSegments);
current->Segment ()->SetUV (current->m_nSide, 0, 0);
m_alignX = 0;
m_alignY = 0;
m_alignAngle = 0;
Rot2nd (0);
UpdateData (FALSE);
undoManager.End ();
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignResetMarked ()
{	
	short nWalls = wallManager.WallCount ();
	BOOL bModified = FALSE;

UpdateData (TRUE);
undoManager.Begin (udSegments);
for (CSegmentIterator si; si; si++) {
	CSegment* segP = &(*si);
	short nSegment = si.Index ();
	for (short nSide = 0; nSide < 6; nSide++) {
		if (segmentManager.IsMarked (CSideKey (nSegment, nSide))) {
			if ((segP->Child (nSide) == -1) || (segP->m_sides [nSide].m_info.nWall < nWalls)) {
				segP->m_sides [nSide].m_info.nOvlTex &= 0x3fff; // rotate 0
				segP->SetUV (nSide,0,0);
				bModified = TRUE;
				}
			}
		}
	}
undoManager.End ();
DLE.MineView ()->Refresh (false);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignStretch2Fit ()
{
	CSide*		sideP = current->Side ();
	uint		scale = 1; //textureManager.Textures () [m_fileType][sideP->m_info.nBaseTex].Scale (sideP->m_info.nBaseTex);
	CSegment*	segP;
	short			nSegment, nSide;
	int			i;

UpdateData (TRUE);
undoManager.Begin (udSegments);
if (!segmentManager.HaveMarkedSides ()) {
	for (i = 0; i < 4; i++) {
		sideP->m_info.uvls [i].u = defaultUVLs [i].u / scale;
		sideP->m_info.uvls [i].v = defaultUVLs [i].v / scale;
		}
	}
else {
	for (nSegment = 0, segP = segmentManager.Segment (0); nSegment < segmentManager.Count (); nSegment++, segP++) {
		for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++) {
			if (segmentManager.IsMarked (CSideKey (nSegment, nSide))) {
				for (i = 0; i < 4; i++) {
					sideP->m_info.uvls [i].u = defaultUVLs [i].u / scale;
					sideP->m_info.uvls [i].v = defaultUVLs [i].v / scale;
					}
				}
			}
		}
	}
undoManager.End ();
DLE.MineView ()->Refresh (false);
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::AlignChildren (short nSegment, short nSide, bool bStart, bool bMarked)
{
// set all segment sides as not aligned yet
if (bStart) {
	CSegment *segP = segmentManager.Segment (0);
	int i;
	for (i = segmentManager.Count (); i; i--, segP++)
		segP->Index () = (bMarked && !segP->IsMarked ()) ? -1 : 0; // all four sides not aligned yet
	}
// mark current side as aligned
segmentManager.Segment (nSegment)->Index () = 1;
// call recursive function which aligns one at a time
AlignChildTextures (nSegment, nSide, MAX_SEGMENTS);
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignAll (void)
{
// set all segment sides as not aligned yet
	CSegment	*currSeg = current->Segment (),
					*segP = segmentManager.Segment (0);
	CSide		*sideP = current->Side (),
					*childSideP;
	short			nSegment, 
					nSide = current->m_nSide,
					nChildLine = 3;
	double		sAngle, cAngle, angle, length; 

UpdateData (TRUE);
undoManager.Begin (udSegments);
bool bAll = segmentManager.HaveMarkedSegments ();
for (nSegment = 0, segP = segmentManager.Segment (0); nSegment < segmentManager.Count (); nSegment++, segP++)
	 segP->Index () = 0;
for (nSegment = 0, segP = segmentManager.Segment (0); nSegment < segmentManager.Count (); nSegment++, segP++) {
	if (segP->Index ())
		continue;
	childSideP = segP->m_sides + nSide;
	if (m_bUse1st && (sideP->m_info.nBaseTex != childSideP->m_info.nBaseTex))
		continue;
	if (m_bUse2nd && (sideP->m_info.nOvlTex != childSideP->m_info.nOvlTex))
		continue;
	if (!(bAll || segmentManager.IsMarked (CSideKey (nSegment, nSide))))
		continue;
	if (nSegment != current->m_nSegment) {
		segmentManager.Segment (nSegment)->SetUV (nSide, 0, 0);
		sAngle = atan3 (sideP->m_info.uvls [(nChildLine + 1) & 3].v - sideP->m_info.uvls [nChildLine].v, 
							 sideP->m_info.uvls [(nChildLine + 1) & 3].u - sideP->m_info.uvls [nChildLine].u); 
		cAngle = atan3 (childSideP->m_info.uvls [nChildLine].v - childSideP->m_info.uvls [(nChildLine + 1) & 3].v, 
							 childSideP->m_info.uvls [nChildLine].u - childSideP->m_info.uvls [(nChildLine + 1) & 3].u); 
		// now rotate childs (u, v) coords around child_point1 (cAngle - sAngle)
		int i;
		for (i = 0; i < 4; i++) {
			angle = atan3 (childSideP->m_info.uvls [i].v, childSideP->m_info.uvls [i].u); 
			length = sqrt ((double)childSideP->m_info.uvls [i].u * (double) childSideP->m_info.uvls [i].u +
								(double)childSideP->m_info.uvls [i].v * (double) childSideP->m_info.uvls [i].v); 
			angle -= (cAngle - sAngle); 
			childSideP->m_info.uvls [i].u = (short) (length * cos (angle)); 
			childSideP->m_info.uvls [i].v = (short) (length * sin (angle)); 
			}
		}
	AlignChildren (nSegment, nSide, false, false);
	}
undoManager.End ();
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

void CTextureTool::OnAlignChildren ()
{
// set all segment sides as not aligned yet
UpdateData (TRUE);
undoManager.Begin (udSegments);
if (!segmentManager.HaveMarkedSegments ())
	// call recursive function which aligns one at a time
	AlignChildren (current->m_nSegment, current->m_nSide, true, false);
else {	// use all marked sides as alignment source
	bool bStart = true;
	if (current->Segment ()->IsMarked ()) {
		AlignChildren (current->m_nSegment, current->m_nSide, true, true);
		bStart = false;
		}
	for (short nSegment = 0; nSegment < segmentManager.Count (); nSegment++, bStart = false)
		for (short nSide = 0; nSide < 6; nSide++)
			if (segmentManager.IsMarked (CSideKey (nSegment, nSide))) 
				AlignChildren (nSegment, nSide, bStart, true);
	}
undoManager.End ();
UpdateAlignWnd ();
}

//------------------------------------------------------------------------------

static const int sideChildTable[6][4] = {
  {4,3,5,1},
  {2,4,0,5},
  {5,3,4,1},
  {0,4,2,5},
  {2,3,0,1},
  {0,3,2,1}
};

void CTextureTool::AlignChildTextures (int nSegment, int nSide, int nDepth)  
{
	char			bAlignedSides = 0;

if (nDepth <= 0)
	return;
if ((nSegment < 0) || (nSegment >= segmentManager.Count ()))
	return;
CSegment* segP = segmentManager.Segment (nSegment);
if (segP->Index () < 0)
	return;

	short			*childList = new short [segmentManager.Count ()];
	short			pos = 0, tos = 0;

if (!childList)
	return;

childList [tos++] = nSegment;
segP->Index () = nSide;

if (m_bIgnorePlane) {
	CSide* sideP = segP->m_sides + nSide;
	short nBaseTex = sideP->m_info.nBaseTex;
	bAlignedSides = 1 << nSide;
	int h = segmentManager.AlignTextures (nSegment, nSide, nSegment, m_bUse1st, m_bUse2nd, bAlignedSides);
	for (short nLine = 0; nLine < 4; nLine++) {
		short nChildSide = sideChildTable[nSide][nLine];
		if (!(bAlignedSides & (1 << nChildSide))) {
			bAlignedSides |= (1 << nChildSide);
			CSide* childSideP = segP->m_sides + nChildSide;
			if (childSideP->m_info.nBaseTex == nBaseTex)
				segmentManager.AlignTextures (nSegment, nChildSide, nSegment, m_bUse1st, m_bUse2nd, bAlignedSides);
			}
		}
	if (h >= 0) {
		CSide* childSideP = segP->m_sides;
		for (short nChildSide = 0; nChildSide < 6; nChildSide++, childSideP++) {
			if (childSideP->BaseTex () == nBaseTex) {
				for (short nLine = 0; nLine < 4; nLine++) {
					short nChildSeg = segP->Child (sideChildTable[nChildSide][nLine]);
					if ((nChildSeg < 0) || (nChildSeg >= segmentManager.Count ()))
						continue;
					CSegment* childSegP = segmentManager.Segment (nChildSeg);
					if (childSegP->Index () != 0)
						continue;
					h = segmentManager.AlignTextures (nSegment, nChildSide, nChildSeg, m_bUse1st, m_bUse2nd, 0);
					childSegP->Index () = h + 1;
					}
				}
			}
		}
	segP->Index () = -1;
	--nDepth;
	CSide* childSideP = segP->m_sides;
	for (short nSide = 0; nSide < 6; nSide++, childSideP++) {
//			if (childSideP->m_info.nBaseTex != sideP->m_info.nBaseTex)
//				continue;
		for (short nLine = 0; nLine < 4; nLine++) {
			short nChildSeg = segP->Child (sideChildTable[nSide][nLine]);
			if ((nChildSeg < 0) || (nChildSeg >= segmentManager.Count ()))
				continue;
			CSegment* childSegP = segmentManager.Segment (nChildSeg);
			short nChildSide = childSegP->Index () - 1;
			if (nChildSide < 0)
				continue;
			AlignChildTextures (nChildSeg, nChildSide, nDepth);
			}
		}
	}
else {
	while (pos < tos) {
		short nSegment = childList [pos++];
		CSegment* segP = segmentManager.Segment (nSegment);
		short nSide = segP->Index ();
		segP->Index () = -1;
		for (short nLine = 0; nLine < 4; nLine++) {
			if (nSide < 0)
				continue;
			short nChildSeg = segP->Child (sideChildTable[nSide][nLine]);
			if ((nChildSeg < 0) || (nChildSeg >= segmentManager.Count ()))
				continue;
			CSegment* childSegP = segmentManager.Segment (nChildSeg);
			if (childSegP->Index ())
				continue;
			//if (bMarked && !childSegP->IsMarked ())
			//	continue;
			childSegP->Index () = segmentManager.AlignTextures (nSegment, nSide, nChildSeg, m_bUse1st, m_bUse2nd, 0);
			if (childSegP->Index () >= 0)
				childList [tos++] = nChildSeg;
			}
/*
		for (nLine = 0; nLine < 4; nLine++) {
			nChildSeg = segP->Child (sideChildTable[nSide][nLine]);
			if ((nChildSeg < 0) || (nChildSeg >= segmentManager.Count ()))
				continue;
			childSeg = segmentManager.Segment (nChildSeg);
			nChildSide = childSeg->Index ();
			if (nChildSide < 0)
				continue;
			AlignChildTextures (nChildSeg, nChildSide, --nDepth);
			}
*/
		}
	}
delete childList;
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
 
if ((sideP->m_info.nOvlTex & 0x1fff) && ((sideP->m_info.nOvlTex & 0xc000) != rotMasks [iAngle])) {
	undoManager.Begin (udSegments);
	sideP->m_info.nOvlTex &= ~0xc000;
   sideP->m_info.nOvlTex |= rotMasks [iAngle];
	m_alignRot2nd = iAngle;
	UpdateData (FALSE);
	undoManager.End ();
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
		
//eof aligndlg.cpp