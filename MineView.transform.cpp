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
#include "render.h"
#include "cfile.h"

#include <math.h>
#include <time.h>

                        /*--------------------------*/

static double zoomScales [2] = {1.2, 1.033};

int CMineView::ZoomFactor (int nSteps, double min, double max)
{
double zoom;
int i;

for (zoom = log(10*m_size.v.x), i = 0; i < nSteps; i++) {
	zoom /= log (1.2);
	if ((zoom < min) || (zoom > max))
		return i;
	}
return nSteps; //(int) ((zoom > 0) ? zoom + 0.5: zoom - 0.5);
}

                        /*--------------------------*/

void CMineView::Zoom (int nSteps, double zoom)
{
for (; nSteps; nSteps--) {
//	m_size.v.x *= zoom;
//	m_size.v.y *= zoom;
	m_size.v.z *= zoom;
	m_view.Scale (1.0 / zoom);
	}
Refresh (false);
}

                        /*--------------------------*/

int CMineView::ZoomIn (int nSteps, bool bSlow)
{
if (nSteps = ZoomFactor (nSteps, -100, 25))
	Zoom (nSteps, zoomScales [bSlow]);
else
	INFOMSG("Already at maximum zoom")
/*
		ErrorMsg ("Already at maximum zoom\n\n"
					"Hint: Try using the 'A' and 'Z' keys to\n"
					"move in forward and backwards.");
*/
return nSteps;
}

                        /*--------------------------*/

int CMineView::ZoomOut(int nSteps, bool bSlow)
{
if (nSteps = ZoomFactor (nSteps, -5, 100))
	Zoom (nSteps, 1.0 / zoomScales [bSlow]);
else
	INFOMSG("Already at minimum zoom")
return nSteps;
}

                        /*--------------------------*/

void CMineView::ShiftViewPoints ()
{
if (!(m_xRenderOffs && m_yRenderOffs))
	return;

	APOINT *a = m_viewPoints;

int i;
for (i = theMine->vertexManager.Count (); i; i--, a++) {
	a->x += m_xRenderOffs;
	a->y += m_yRenderOffs;
	}
}

                        /*--------------------------*/

int CMineView::FitToView (void)
{
if (theMine == null) return 1;

	CRect			rc (LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	double		zoomX, zoomY, zoom;
	int			dx, dy;

DelayRefresh (true);
//CenterMine ();
//SetViewPoints (&rc);
m_move.Clear ();
m_view.SetViewInfo (10000, m_viewWidth, m_viewHeight);
MarkVisibleVerts ();
SetViewPoints (&rc, false);
CRect	crc;
GetClientRect (crc);
crc.InflateRect (-4, -4);
zoomX = (double) crc.Width () / (double) rc.Width ();
zoomY = (double) crc.Height () / (double) rc.Height ();
zoom = (zoomX < zoomY) ? zoomX: zoomY;
Zoom (1, zoom);
for (;;) {
	m_view.SetViewInfo (depthPerception, m_viewWidth, m_viewHeight);
	SetViewPoints (&rc);
	if ((rc.Width () <= crc.Width ()) && (rc.Height () <= crc.Height ()))
		break;
	Zoom (1, 0.95);
	}

dy = (crc.Height () - rc.Height ()) / 2;
while (rc.top - dy > 0) {
	Pan ('Y', -1);
	SetViewPoints (&rc);
	}
if (rc.top < dy)
	while (rc.top - dy < 0) {
		Pan ('Y', 1);
		SetViewPoints (&rc);
		}
else
	while (rc.bottom + dy > crc.bottom) {
		Pan ('Y', -1);
		SetViewPoints (&rc);
		}
dx = (crc.Width () - rc.Width ()) / 2;
if (rc.left < dx)
	while (rc.left - dx < 0) {
		Pan ('X', -1);
		SetViewPoints (&rc);
		}
else
	while (rc.right + dx > crc.right) {
		Pan ('X', +1);
		SetViewPoints (&rc);
		}
MarkVisibleVerts (true);
DelayRefresh (false);
Refresh ();
return 1;
}

                        /*--------------------------*/

void CMineView::Rotate (char direction, double angle)
{
	static double a = 0;
int i = direction - 'X';
if ((i < 0) || (i > 2))
	return;
#if 0 //OGL_RENDERING
angle *= 5 * 180.0 / PI;
glAngle [i] += angle;
if (glAngle [i] <= -360)
	glAngle [i] += 360;
else if (a >= 360)
	glAngle [i] -= 360;
sprintf_s (message, sizeof (message), "ROTATE (%1.2f°)", glAngle [i]);
INFOMSG (message);
glRotated (glAngle [i], glRotMat [i][0], glRotMat [i][1], glRotMat [i][2]);
#else
m_view.Rotate (direction, 2 * angle); // * ((double) moveRate / 0x10000L));
a += 2 * angle;// * PI;
if (a < -360)
	a += 360;
else if (a > 360)
	a -= 360;
sprintf_s (message, sizeof (message), "ROTATE (%1.2f°)", a);
INFOMSG (message);
#endif
Refresh (false);
}

                        /*--------------------------*/

void CMineView::Pan (char direction, int value)
{
if (!value)
	return;
int i = direction - 'X';
if ((i < 0) || (i > 2))
	i = 1;
#if 0 //OGL_RENDERING
glPan [i] += (double) value * 1.9;
glTranslated (glPan [0], glPan [1], glPan [2]);
#else
# if 0
if (i == 1)
	m_movex -= value;
else if (i == 2)
	m_movey -= value;
else if (i == 2)
	m_movez -= value;
# else
m_move -= CDoubleVector (m_view.m_invMat [0].rVec [i], m_view.m_invMat [0].uVec [i], m_view.m_invMat [0].fVec [i]) * value;
# endif
#endif
Refresh (false);
}

                        /*--------------------------*/

void CMineView::AlignSide()
{
}
                        /*--------------------------*/

void CMineView::MarkVisibleVerts (bool bReset)
{
	int			h, i;
	CSegment*	segP;

segP = theMine->Segments (0);
for (i = 0, h = segmentManager.Count (); i < h; i++, segP++) {
	byte status = bReset ? 0 : Visible (segP) ? 1 : 255;
	for (int j = 0; j < 8; j++)
		theMine->vertexManager.Status (segP->m_info.verts [j]) = status;
	}
}

                        /*--------------------------*/

void CMineView::CenterMine()
{
CHECKMINE;

//	CDlcDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

	CVertex*		vertP;
	CVertex		vMin (0x7fffffff, 0x7fffffff, 0x7fffffff), vMax (-0x7fffffff, -0x7fffffff, -0x7fffffff);

MarkVisibleVerts ();
vertP = theMine->Vertices (0);
for (int i = 0, h = theMine->vertexManager.Count (); i < h; i++, vertP++) {
	if (theMine->vertexManager.Status (i)) {
		vMin = Min (vMin, *vertP);
		vMax = Max (vMax, *vertP);
		}
	}
m_spin.Set (M_PI / 4.0, M_PI / 4.0, 0.0);
m_move = CDoubleVector (Average (vMin, vMax));
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
factor = 0.1 * pow (1.2, (double) factor);
m_size.Set (factor, factor, factor);
m_view.Set (m_move, m_size, m_spin);
MarkVisibleVerts (true);
Refresh (false);
}

                        /*--------------------------*/

void CMineView::CenterCube (void)
{
CHECKMINE;

	CSegment& seg = theMine->Segments (0) [m_Current->nSegment];
	CVertex *vMine = theMine->Vertices (0);
	short *vSeg = seg.m_info.verts;

m_move = (vMine [vSeg [0]] +
			 vMine [vSeg [1]] +
			 vMine [vSeg [2]] +
			 vMine [vSeg [3]] +
			 vMine [vSeg [4]] +
			 vMine [vSeg [5]] +
			 vMine [vSeg [6]] +
			 vMine [vSeg [7]]);
m_move /= -8.0;
Refresh (false);
}

                        /*--------------------------*/

void CMineView::CenterObject()
{
CHECKMINE;

CDlcDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
if (!pDoc) return;

m_move = -theMine->Objects (m_Current->nObject)->m_location.pos;
Refresh (false);
}


                        /*--------------------------*/


//eof