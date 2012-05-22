// dlcView.cpp: implementation of the CMineView class
//

#include "stdafx.h"
#include "winuser.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"

#include "PaletteManager.h"
#include "textures.h"
#include "global.h"
#include "FileManager.h"

#include <math.h>
#include <time.h>

double zoomScales [2] = {1.2, 1.1};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CMineView::ShiftViewPoints (void)
{
if (!(m_xRenderOffs && m_yRenderOffs))
	return;
for (int i = 0, j = vertexManager.Count (); i < j; i++) {
	CVertex& v = vertexManager [i];
	v.m_screen.x += m_xRenderOffs;
	v.m_screen.y += m_yRenderOffs;
	}
}

//------------------------------------------------------------------------------

void CMineView::TogglePerspective (void)
{
SetPerspective (!Perspective ());
}

//------------------------------------------------------------------------------

int CMineView::FitToView (void)
{
#if 0 //def _DEBUG
return 1;
#endif
if (theMine == null) 
	return 1;
if (!Renderer ().CanFitToView ()) {
	if (segmentManager.Count () == 1)
		DLE.MineView ()->Pan ('Z', 10.0);
	return 1;
	}
//if (m_glRendering)
//	return 1;

	CRect			rc (LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	double		zoomX, zoomY, zoom;
	long			dx, dy;
	double		moveRates [2];

DelayRefresh (true);
GetMoveRates (moveRates);
SetViewMoveRate (1.0);
SetMineMoveRate (1.0);
Renderer ().Setup (this, DC ());
Renderer ().Reset ();
ViewMatrix ()->Setup (Translation (), Scale (), Rotation ());
//CenterOnMine ();
Project (&rc);
ViewMatrix ()->SetViewInfo (ViewWidth (), ViewHeight ());
MarkVisibleVerts ();
Project (&rc);
CRect	crc;
GetClientRect (crc);
crc.InflateRect (-4, -4);
zoomX = (double) rc.Width () / (double) crc.Width ();
zoomY = (double) rc.Height () / (double) crc.Height ();
zoom = (zoomX < zoomY) ? zoomX : zoomY;
Zoom (-1, zoom);
for (;;) {
	if (!Project (&rc))
		return 1;
	if ((rc.Width () >= crc.Width ()) || (rc.Height () >= crc.Height ()))
		break;
	Zoom (-1, 1.0 / zoomScales [1]);
	}

if (m_nRenderer) {
	if ((rc.Width () > crc.Width ()) || (rc.Height () > crc.Height ())) {
		int	nSteps = -1;
		int	dx0, dy0;
		CRect rcPrev;

		for (;;) {
			rcPrev = rc;
			Zoom (nSteps, zoomScales [1]);
			int nProjected = Project (&rc, true);
			if (!nProjected)
				return 1;
			dx0 = rc.Width () - crc.Width ();
			dy0 = rc.Height () - crc.Height ();
			if ((dx0 <= 0) && (dy0 <= 0))
				break;
			if (nProjected < 0)
				continue;
			dx = rcPrev.Width () - rc.Width ();
			dy = rcPrev.Height () - rc.Height ();
			if ((dx > 0) && (dy > 0)) {
				float sx = float (dx0) / float (dx);
				float sy = float (dy0) / float (dy);
				nSteps = (dx0 < 0) ? -int (sy) : (dy0 < 0) ? -int (sx) : (sx < sy) ? -int (sx) : -int (sy);
				if (nSteps == 0x80000000)
					nSteps = -1;
				if (nSteps > -1)
					nSteps = -1;
				}
			}
		}
	}
else {
	while ((rc.Width () > crc.Width ()) || (rc.Height () > crc.Height ())) {
		Zoom (-1, zoomScales [1]);
		if (!Project (&rc))
			return 1;
		}
	}

dy = (crc.Height () - rc.Height ()) / 2;
while (rc.top - dy > 0) {
	Pan ('Y', -1);
	if (!Project (&rc))
		return 1;
	dy = (crc.Height () - rc.Height ()) / 2;
	}
if (rc.top < dy)
	while (rc.top - dy < 0) {
		Pan ('Y', 1);
		if (!Project (&rc))
			return 1;
		dy = (crc.Height () - rc.Height ()) / 2;
		}
else
	while (rc.bottom + dy > crc.bottom) {
		Pan ('Y', -1);
		if (!Project (&rc))
			return 1;
		}
dx = (crc.Width () - rc.Width ()) / 2;
if (rc.left < dx)
	while (rc.left - dx < 0) {
		Pan ('X', -1);
		if (!Project (&rc))
			return 1;
		dx = (crc.Width () - rc.Width ()) / 2;
		}
else
	while (rc.right + dx > crc.right) {
		Pan ('X', +1);
		if (!Project (&rc))
			return 1;
		dx = (crc.Width () - rc.Width ()) / 2;
		}
if (segmentManager.Count () == 1)
	DLE.MineView ()->Pan ('Z', -80.0);
Renderer ().FitToView ();
SetMoveRates (moveRates);
MarkVisibleVerts (true);
DelayRefresh (false);
Refresh ();
return 1;
}

//------------------------------------------------------------------------------

void CMineView::Rotate (char direction, double angle)
{
	static double a = 0;

int i = direction - 'X';
if ((i < 0) || (i > 2))
	return;
angle *= DLE.MineView ()->ViewMoveRate ();
ViewMatrix ()->Rotate (direction, -2 * angle); // * ((double) DLE.MineView ()->MineMoveRate () / 0x10000L));
a += 2 * angle;// * PI;
if (a < -360)
	a += 360;
else if (a > 360)
	a -= 360;
sprintf_s (message, sizeof (message), "ROTATE (%1.2f�)", a);
INFOMSG (message);
Refresh (false);
}

//------------------------------------------------------------------------------

void CMineView::AlignSide()
{
}
//------------------------------------------------------------------------------

void CMineView::MarkVisibleVerts (bool bReset)
{
	int			h, i;
	CSegment*	segP;

segP = segmentManager.Segment (0);
for (i = 0, h = segmentManager.Count (); i < h; i++, segP++) {
	ubyte status = bReset ? 0 : Visible (segP) ? 1 : 255;
	for (int j = 0; j < 8; j++)
		if (segP->m_info.vertexIds [j] <= MAX_VERTEX)
			vertexManager.Status (segP->m_info.vertexIds [j]) = status;
	}
}

//------------------------------------------------------------------------------

void CMineView::CenterOnMine (void)
{
CHECKMINE;

//	CDlcDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

	CVertex*		vertP;
	CVertex		vMin (0x7fffffff, 0x7fffffff, 0x7fffffff), vMax (-0x7fffffff, -0x7fffffff, -0x7fffffff);

MarkVisibleVerts ();
vertP = vertexManager.Vertex (0);
for (int i = 0, h = vertexManager.Count (); i < h; i++, vertP++) {
	if (vertexManager.Status (i)) {
		vMin = Min (vMin, *vertP);
		vMax = Max (vMax, *vertP);
		}
	}
Rotation ().Set (M_PI / 4.0, M_PI / 4.0, 0.0);
CDoubleVector v = vMax - vMin;
int maxVal = int (max (max (v.v.x, v.v.y), v.v.z) / 20);
double factor;
if (maxVal < 2)      
	factor = 14;
else if (maxVal < 4) 
	factor = 10;
else if (maxVal < 8) 
	factor = 8;
else if (maxVal < 12) 
	factor = 5;
else if (maxVal < 16) 
	factor = 3;
else if (maxVal < 32) 
	factor = 2;
else 
	factor = 1;
factor = 0.1 * pow (zoomScales [0], (double) factor);
Scale ().Set (factor, factor, factor);
ViewMatrix ()->Setup (Translation (), Scale (), Rotation ());
MarkVisibleVerts (true);
SetCenter (CDoubleVector (Average (vMin, vMax)), 0);
Refresh (false);
}

//------------------------------------------------------------------------------

void CMineView::CenterOnObject (void)
{
CHECKMINE;

CDlcDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
if (!pDoc) 
	return;

SetCenter (current->Object ()->Position (), 2);
Refresh (false);
}

//------------------------------------------------------------------------------

void CMineView::CenterOnSegment (void)
{
CHECKMINE;

SetCenter (current->Segment ()->Center (), 1);
Refresh (false);
}

//------------------------------------------------------------------------------

void CMineView::AlignViewerWithSide (void)
{
CSegment* segP = current->Segment ();
CSide* sideP = current->Side ();
sideP->ComputeNormals (segP->m_info.vertexIds, segP->ComputeCenter ());
CDoubleMatrix r;
r.m.fVec = sideP->Normal (2);
r.m.fVec.Negate ();
short nSide = current->m_nSide;
short nEdge = current->m_nEdge;
r.m.rVec = *segP->Vertex (nSide, nEdge + 1) - *segP->Vertex (nSide, nEdge);
r.m.rVec.Normalize ();
Rotation () = r.Angles ();
ViewMatrix ()->Setup (Translation (), Scale (), Rotation ());
Invalidate (FALSE);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CRenderer::Zoom (int nSteps, double zoom)
{
if (nSteps < 0)
	zoom = 1.0 / zoom;
else if (nSteps > 1)
	zoom = pow (zoom, nSteps);
Scale ().v.z *= zoom;
ViewMatrix ()->Scale (zoom);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------


//eof