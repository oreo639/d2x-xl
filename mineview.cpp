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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
#endif

#define OGL_ORTHO 0

static LPSTR szMouseStates [] = {
	"IDLE",
	"BUTTON DOWN",
	"SELECT",
	"PAN",
	"ROTATE",
	"ZOOM",
	"ZOOM IN",
	"ZOOM OUT",
	"INIT DRAG",
	"DRAG",
	"RUBBERBAND"
	};


#define BETWEEN(a,b,c) ((a<c) ? (a<b)&&(b<c): (c<b)&&(b<a))
#define  RUBBER_BORDER     1

#define f2fl(f)	(((double) f) / (double) 65536.0)
#define UV_FACTOR ((double)640.0/(double)0x10000L)

/////////////////////////////////////////////////////////////////////////////
// CMineView

IMPLEMENT_DYNCREATE(CMineView, CView)

BEGIN_MESSAGE_MAP(CMineView, CView)
	//{{AFX_MSG_MAP(CMineView)
	ON_WM_TIMER ()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	ON_WM_MOUSEWHEEL()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(ID_SEL_PREVTAB, OnSelectPrevTab)
	ON_COMMAND(ID_SEL_NEXTTAB, OnSelectNextTab)
END_MESSAGE_MAP()

BOOL CMineView::PreCreateWindow(CREATESTRUCT& cs)
{
return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMineView construction/destruction

CMineView::CMineView()
{
static LPSTR nIdCursors [eMouseStateCount] = {
	IDC_ARROW,
	IDC_ARROW,
	MAKEINTRESOURCE (IDC_CURSOR_PAN),
	MAKEINTRESOURCE (IDC_CURSOR_ROTATE),
	MAKEINTRESOURCE (IDC_CURSOR_ZOOM),
	MAKEINTRESOURCE (IDC_CURSOR_ZOOMIN),
	MAKEINTRESOURCE (IDC_CURSOR_ZOOMOUT),
	MAKEINTRESOURCE (IDC_CURSOR_DRAG),
	MAKEINTRESOURCE (IDC_CURSOR_DRAG),
	IDC_ARROW
	};

m_penCyan    = new CPen(PS_SOLID, 1, RGB(255,196,  0)); //ok, that's gold, but cyan doesn't work - palette problem?
//m_penCyan    = new CPen(PS_SOLID, 1, RGB(  0,255,255)); //doesn't work (rather green) - palette problem?
m_penRed     = new CPen(PS_SOLID, 1, RGB(255,  0,  0));
m_penMedRed  = new CPen(PS_SOLID, 1, RGB(255,  0,  0));
m_penGray    = new CPen(PS_SOLID, 1, RGB(128,128,128));
m_penLtGray  = new CPen(PS_SOLID, 1, RGB(160,160,160));
m_penGreen   = new CPen(PS_SOLID, 1, RGB(  0,255,  0));
m_penDkGreen = new CPen(PS_SOLID, 1, RGB(  0,128,  0));
m_penDkCyan  = new CPen(PS_SOLID, 1, RGB(  0,128,128));
m_penBlue	 = new CPen(PS_SOLID, 1, RGB(  0,  0,255));
m_penMedBlue = new CPen(PS_SOLID, 1, RGB(  0,160,255));
m_penLtBlue	 = new CPen(PS_SOLID, 1, RGB(  0,255,255));
m_penYellow  = new CPen(PS_SOLID, 1, RGB(255,196,  0));
m_penOrange  = new CPen(PS_SOLID, 1, RGB(255,128,  0));
m_penMagenta = new CPen(PS_SOLID, 1, RGB(255,  0,255));
m_penHiCyan  = new CPen(PS_SOLID, 2, RGB(255,196,  0)); //ok, that's gold, but cyan doesn't work - palette problem?
//m_penHiCyan    = new CPen(PS_SOLID, 2, RGB(  0,255,255)); //doesn't work (rather green) - palette problem?
m_penHiRed     = new CPen(PS_SOLID, 2, RGB(255,  0,  0));
m_penHiGray    = new CPen(PS_SOLID, 2, RGB(128,128,128));
m_penHiLtGray  = new CPen(PS_SOLID, 2, RGB(160,160,160));
m_penHiGreen   = new CPen(PS_SOLID, 2, RGB(  0,255,  0));
m_penHiDkGreen = new CPen(PS_SOLID, 2, RGB(  0,128,  0));
m_penHiDkCyan  = new CPen(PS_SOLID, 2, RGB(  0,128,128));
m_penHiBlue		= new CPen(PS_SOLID, 2, RGB(  0,  0,255));
m_penHiYellow  = new CPen(PS_SOLID, 2, RGB(255,196,  0));
m_penHiOrange  = new CPen(PS_SOLID, 2, RGB(255,128,  0));
m_penHiMagenta = new CPen(PS_SOLID, 2, RGB(255,  0,255));
INT32 i;
for (i = eMouseStateIdle; i < eMouseStateCount; i++)
	m_hCursors [i] = LoadCursor ((nIdCursors [i] == IDC_ARROW) ? NULL: theApp.m_hInstance, nIdCursors [i]);
m_viewObjectFlags = eViewObjectsAll;
m_viewMineFlags = eViewMineLights | eViewMineWalls | eViewMineSpecial;
m_viewOption = eViewTextureMapped;
m_nDelayRefresh = 0;
m_bHScroll = 
m_bVScroll = false;
m_xScrollRange =
m_yScrollRange = 0;
m_xScrollCenter =
m_yScrollCenter = 0;
m_nMineCenter = 2;
#if OGL_RENDERING
m_glRC = NULL;
m_glDC = NULL;
#endif
m_nViewDist = 0;
m_depthPerception = 10000.0f;
renderModel = NULL;
Reset ();
}

                        /*--------------------------*/

void CMineView::Reset (void)
{
m_viewWidth = m_viewHeight = m_viewDepth = 0;	// force OnDraw to initialize these
theMine->SetSplineActive (false);
m_bUpdate = true;
m_mouseState  = 
m_lastMouseState = eMouseStateIdle;
m_selectMode = eSelectSide;
m_lastSegment = 0;

m_center.Clear ();
m_spin.Set (M_PI / 4.0, M_PI / 4.0, 0.0);
m_move.Clear ();
m_size.Set (1.0, 1.0, 1.0);

// calculate transformation m_view based on move, size, and spin
m_view.Set(m_move, m_size, m_spin);
m_lightTimer =
m_selectTimer = -1;
m_nFrameRate = 100;
m_bShowLightSource = 0;
m_xRenderOffs = m_yRenderOffs = 0;
}

                        /*--------------------------*/

CMineView::~CMineView()
{
delete m_penCyan;
delete m_penRed;
delete m_penMedRed;
delete m_penGray;
delete m_penGreen;
delete m_penDkGreen;
delete m_penDkCyan;
delete m_penBlue;
delete m_penMedBlue;
delete m_penLtBlue;
delete m_penLtGray;
delete m_penYellow;
delete m_penOrange;
delete m_penMagenta;
delete m_penHiCyan;
delete m_penHiRed;
delete m_penHiGray;
delete m_penHiGreen;
delete m_penHiDkGreen;
delete m_penHiDkCyan;
delete m_penHiLtGray;
delete m_penHiYellow;
delete m_penHiOrange;
delete m_penHiMagenta;
}

                        /*--------------------------*/

void CMineView::OnDestroy ()
{
if (m_lightTimer != -1) {
	KillTimer (m_lightTimer);
	m_lightTimer = -1;
	}
if (m_selectTimer != -1) {
	KillTimer (m_selectTimer);
	m_selectTimer = -1;
	}
CView::OnDestroy ();
}

/////////////////////////////////////////////////////////////////////////////
// CMineView drawing

bool CMineView::UpdateScrollBars (void)
{
return false;
SetViewPoints ();

	INT32		dx = m_maxViewPoint.x - m_minViewPoint.x + 1;
	INT32		dy = m_minViewPoint.y - m_maxViewPoint.y + 1;
	double	r;

bool bHScroll = m_bHScroll;
bool bVScroll = m_bVScroll;
INT32 xScrollCenter = m_xScrollCenter;
INT32 yScrollCenter = m_yScrollCenter;
INT32 xScrollRange = m_xScrollRange;
INT32 yScrollRange = m_yScrollRange;
m_bHScroll = (dx > m_viewWidth);
m_bVScroll = (dy > m_viewHeight);
ShowScrollBar (SB_HORZ, m_bHScroll);
ShowScrollBar (SB_VERT, m_bVScroll);
InitViewDimensions ();
SetViewPoints ();
dx = m_maxViewPoint.x - m_minViewPoint.x + 1;
dy = m_minViewPoint.y - m_maxViewPoint.y + 1;
if (m_bHScroll) {
	if (xScrollRange > m_xScrollRange)
		SetScrollRange (SB_HORZ, 0, m_xScrollRange = dx - m_viewWidth, TRUE);
	if (!bHScroll)
		SetScrollPos (SB_HORZ, m_xScrollCenter = m_xScrollRange / 2, TRUE);
	else if (xScrollRange != m_xScrollRange) {
		r = (double) xScrollRange / (double) m_xScrollRange;
		INT32 nPos = GetScrollPos (SB_HORZ);
		nPos -= m_xScrollCenter;
		m_xScrollCenter = m_xScrollRange / 2;
		SetScrollPos (SB_HORZ, (UINT) (m_xScrollCenter + (double) nPos * r), TRUE);
		}
	if (xScrollRange < m_xScrollRange)
		SetScrollRange (SB_HORZ, 0, m_xScrollRange = dx - m_viewWidth, TRUE);
	}
else {
	m_xScrollRange = 0;
	m_xRenderOffs = 0;
	}
if (m_bVScroll) {
	if (yScrollRange > m_yScrollRange)
		SetScrollRange (SB_VERT, 0, m_yScrollRange = dy - m_viewHeight, TRUE);
	if (!bVScroll)
		SetScrollPos (SB_VERT, m_yScrollCenter = m_yScrollRange / 2, TRUE);
	else if (yScrollRange != m_yScrollRange) {
		r = (double) yScrollRange / (double) m_yScrollRange;
		INT32 nPos = GetScrollPos (SB_VERT);
		nPos -= m_yScrollCenter;
		m_yScrollCenter = m_yScrollRange / 2;
		SetScrollPos (SB_VERT, (UINT) (m_yScrollCenter + (double) nPos * r), TRUE);
		}
	if (yScrollRange < m_yScrollRange)
		SetScrollRange (SB_VERT, 0, m_yScrollRange = dy - m_viewHeight, TRUE);
	}
else {
	m_yScrollRange = 0;
	m_yRenderOffs = 0;
	}
return (bHScroll != m_bHScroll) || (bVScroll != m_bVScroll);
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
                        /*--------------------------*/

void CMineView::OnDraw (CDC* pViewDC)
{
CHECKMINE;

#if OGL_RENDERING
if (m_bUpdate) {
	GLRenderScene ();
	if (!SwapBuffers (glHDC)) {
		INT32 err = GetLastError ();
	//	m_glDC = GetDC ();
	//	glHDC = m_glDC->m_hDC;
		GLKillWindow ();
		GLRenderScene ();
		}
//	pViewDC->BitBlt (0,0, m_viewWidth, m_viewHeight, &m_DC, 0, 0, SRCCOPY);
	}
#else
	CDlcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) 
		return;
	bool bPartial = false;

	// Initialize/Reinitialize the View's device context
	// and other handy variables
#if 1
	m_pDC = pViewDC;
	if (DrawRubberBox () || DrawDragPos ()) {
#if 0
		if (m_DIB)
			pViewDC->BitBlt (0,0, m_viewWidth, m_viewHeight, &m_DC, 0, 0, SRCCOPY);
#endif
		m_bUpdate = false;
		return;
		}
#endif
	InitView (pViewDC);
	if (m_bUpdate) {
		// Clear the View
		ClearView();

		m_xRenderOffs = m_bHScroll ? GetScrollPos (SB_HORZ) - m_xScrollCenter: 0;
		m_yRenderOffs = m_bVScroll ? GetScrollPos (SB_VERT) - m_yScrollCenter: 0;
		// Calculate m_view M based on IM and "move x,y,z"
		//m_view.Calculate(m_movex, m_movey, m_movez);

		// Set view m_view misc. information
		//m_view.SetViewInfo(m_depthPerception, m_viewWidth, m_viewHeight);
		SetViewPoints ();
		ShiftViewPoints ();
		// make a local copy the mine's selection
		m_Current = theMine->Current ();

		// draw the level
		switch(m_viewOption)	{
			case eViewAllLines:
				DrawWireFrame(false);
				break;

			case eViewHideLines:
				DrawWireFrame(false); // temporary
				break;

			case eViewNearbyCubeLines:
				DrawWireFrame(false); // temporary
				break;

			case eViewPartialLines:
				DrawWireFrame(bPartial = true);
				break;

			case eViewTextureMapped:
				DrawTextureMappedCubes();
				break;
			}
/*
	if (m_viewFlags & eViewMineWalls)
		DrawWalls ();
	if (m_viewFlags & eViewMineLights)
		DrawLights ();
	if (m_viewObjects)
		DrawObjects (bPartial);
*/
		}
#endif
DrawRubberBox ();
DrawHighlight ();
DrawMineCenter (pViewDC);
// if we are using our own DC, then copy it to the display
if (m_DIB)
	pViewDC->BitBlt (0,0, m_viewWidth, m_viewHeight, &m_DC, 0, 0, SRCCOPY);
//CBRK (!m_bUpdate);
m_bUpdate = false;
}

								/*---------------------------*/

afx_msg void CMineView::OnPaint ()
{
CHECKMINE;

	CRect	rc;
	CDC	*pDC;
	PAINTSTRUCT	ps;

if (!GetUpdateRect (rc))
	return;
pDC = BeginPaint (&ps);
DrawRubberBox ();
EndPaint (&ps);
}

                        /*--------------------------*/

void CMineView::AdvanceLightTick (void)
{
CHECKMINE;

	LIGHT_TIMER *ltP = lightTimers;
	CFlickeringLight *flP = theMine->FlickeringLights (0);
	INT32 i, light_delay;

for (i = theMine->FlickerLightCount (); i; i--, flP++, ltP++) {
	light_delay = (flP->m_info.delay * 100 /*+ F0_5*/) / F1_0;
	if (light_delay) {
		if (++ltP->ticks == light_delay) {
			ltP->ticks = 0;
			ltP->impulse = (ltP->impulse + 1) % 32;
			}
		}
	else
		ltP->impulse = (ltP->impulse + 1) % 32;
	}
}

                        /*--------------------------*/
#ifdef _DEBUG
static INT32 qqq1 = -1, qqq2 = 0;
#endif

bool CMineView::SetLightStatus (void)
{
	INT32 h, i, j;
	CLightDeltaIndex *ldiP = theMine->LightDeltaIndex (0);
	CLightDeltaValue *ldvP;
	LIGHT_TIMER *ltP;
	CFlickeringLight *flP = theMine->FlickeringLights (0);
	LIGHT_STATUS *pls;
	bool bChange = false;
	bool bD2XLights = (theMine->LevelVersion () >= 15) && (theMine->GameInfo ().fileinfo.version >= 34);
	INT16 nSrcSide, nSrcSeg, nSegment, nSide;

// search delta light index to see if current side has a light
pls = lightStatus [0];
for (i = theMine->SegCount (); i; i--)
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++, pls++)
		pls->bWasOn = pls->bIsOn;
for (h = 0; h < theMine->GameInfo ().lightDeltaIndices.count; h++, ldiP++) {
	nSrcSide = ldiP->m_nSegment;
	nSrcSeg = ldiP->m_nSide;
	j = theMine->GetFlickeringLight (nSrcSide, nSrcSeg);
	if (j < 0)
		continue;	//shouldn't happen here, as there is a delta light value, but you never know ...
	if (j >= MAX_FLICKERING_LIGHTS)
		continue;	//shouldn't happen 
	ldvP = theMine->LightDeltaValues (ldiP->m_info.index);
	for (i = ldiP->m_info.count; i; i--, ldvP++) {
		nSegment = ldvP->m_nSegment;
		nSide = ldvP->m_nSide;
		if (m_bShowLightSource) {
			if ((nSegment != nSrcSeg) || (nSide != nSrcSide)) 
				continue;
			if (0 > theMine->GetFlickeringLight (nSegment, nSide))
				continue;
			}
		else {
			if (((nSegment != nSrcSeg) || (nSide != nSrcSide)) && (0 <= theMine->GetFlickeringLight (nSegment, nSide)))
				continue;
			}
		pls = lightStatus [nSegment] + nSide;
		ltP = lightTimers + j;
		pls->bIsOn = (flP [j].m_info.mask & (1 << lightTimers [j].impulse)) != 0;
		if (pls->bWasOn != pls->bIsOn)
			bChange = true;
		}
	}
return bChange;
}

                        /*--------------------------*/

void CMineView::OnTimer (UINT_PTR nIdEvent)
{
CHECKMINE;

if (nIdEvent == 4) {
	if (m_mouseState == eMouseStateButtonDown) {
		m_mouseState = eMouseStateSelect;
#ifdef _DEBUG
	  	INFOMSG (szMouseStates [m_mouseState]);
#endif
		}
	if (m_mouseState == eMouseStateSelect) {
		if (!SelectCurrentSegment (1, m_clickPos.x, m_clickPos.y)) {
			SetMouseState (eMouseStateRubberBand);
			//UpdateRubberRect (m_clickPos);
			}
		}
	}
else if (nIdEvent == 3) {
	if (bEnableDeltaShading) {
		AdvanceLightTick ();
		if (SetLightStatus ()) {
			m_bUpdate = TRUE;
			InvalidateRect (NULL, TRUE);
			UpdateWindow ();
			}
		}
	}
else 
	CView::OnTimer (nIdEvent);
}

								/*---------------------------*/

void CMineView::EnableDeltaShading (INT32 bEnable, INT32 nFrameRate, INT32 bShowLightSource)
{
if (bEnableDeltaShading = bEnable) {
	m_lightTimer = SetTimer (3, (UINT) (m_nFrameRate + 5) / 10, NULL);
	if ((nFrameRate >= 10) && (nFrameRate <= 100))
		m_nFrameRate = nFrameRate;
	if (bShowLightSource != -1)
		m_bShowLightSource = bShowLightSource;
	memset (lightTimers, 0, sizeof (lightTimers));
	memset (lightStatus, 0xff, sizeof (lightStatus));
	}
else if (m_lightTimer != -1) {
	KillTimer (m_lightTimer);
	m_lightTimer = -1;
	}
}

								/*---------------------------*/

BOOL CMineView::SetWindowPos(const CWnd *pWndInsertAfter, INT32 x, INT32 y, INT32 cx, INT32 cy, UINT nFlags)
{
	CRect	rc;

	GetClientRect (rc);

if ((rc.Width () != cx) || (rc.Height () != cy))
	theApp.MainFrame ()->ResetPaneMode ();
return CView::SetWindowPos (pWndInsertAfter, x, y, cx, cy, nFlags);
}



bool CMineView::InitViewDimensions (void)
{
	CRect	rc;

GetClientRect (rc);
INT32 width = (rc.Width () + 3) & ~3; // long word align
INT32 height = rc.Height ();
if ((m_viewWidth == width) && (m_viewHeight == height))
	return false;
m_viewWidth = width;
m_viewHeight = height;
return true;
}

//----------------------------------------------------------------------------
// InitView()
//----------------------------------------------------------------------------

void CMineView::ResetView (bool bRefresh)
{
#if OGL_RENDERING
GLKillWindow ();
#else
if (m_DIB) {
	DeleteObject(m_DIB);
	m_DIB = 0;
	}
if (m_DC.m_hDC)
	m_DC.DeleteDC();
if (bRefresh)
	Refresh (true);
#endif
}

//----------------------------------------------------------------------------
// InitView()
//----------------------------------------------------------------------------

void CMineView::InitView (CDC *pViewDC)
{
	// if all else fails, then return the original device context
#if OGL_RENDERING == 0
	m_pDC = pViewDC;
#endif

//	INT32 depth = m_pDC->GetDeviceCaps(BITSPIXEL) / 8;
	INT32 depth = 1; // force 8-bit DIB
#if 0
	// if view size is new, then reset dib and delete the current device context
	CRect Rect;
	GetClientRect(Rect);
	INT32 width = Rect.Width (); //(Rect.Width() + 3) & ~3; // long word align
	INT32 height = Rect.Height();

	if (width != m_viewWidth || height != m_viewHeight || depth != m_viewDepth) {
#else
	if (InitViewDimensions() || (depth != m_viewDepth)) {
#endif
		m_bUpdate = true;
#if OGL_RENDERING == 0
		ResetView ();
#endif
	}
#if 0
	m_viewWidth = width;
	m_viewHeight = height;
#endif
	m_viewDepth = depth;
#if OGL_RENDERING == 0
	if (UpdateScrollBars ()) {
		ResetView (!m_bUpdate);
		m_bUpdate = true;
		}
	if (!m_DIB) {
		if (!m_DC.m_hDC)
			m_DC.CreateCompatibleDC(pViewDC);
		if (m_DC.m_hDC) {
			typedef struct {
				BITMAPINFOHEADER    bmiHeader;
				RGBQUAD             bmiColors [256];
			} MYBITMAPINFO;

			// define the bitmap parameters
			MYBITMAPINFO mybmi;
			mybmi.bmiHeader.biSize              = sizeof (BITMAPINFOHEADER);
			mybmi.bmiHeader.biWidth             = m_viewWidth;
			mybmi.bmiHeader.biHeight            = m_viewHeight;
			mybmi.bmiHeader.biPlanes            = 1;
			mybmi.bmiHeader.biBitCount          = m_viewDepth*8;
			mybmi.bmiHeader.biCompression       = BI_RGB;
			mybmi.bmiHeader.biSizeImage         = 0;
			mybmi.bmiHeader.biXPelsPerMeter     = 1000;
			mybmi.bmiHeader.biYPelsPerMeter     = 1000;
			mybmi.bmiHeader.biClrUsed           = 256;
			mybmi.bmiHeader.biClrImportant      = 256;

			// copy the bitmap palette
			UINT8 *palette = PalettePtr ();
			if (palette) {
				INT32 i, j;
				for (i = j = 0; i < 256; i++) {
					mybmi.bmiColors [i].rgbRed   = palette [j++]<<2;
					mybmi.bmiColors [i].rgbGreen = palette [j++]<<2;
					mybmi.bmiColors [i].rgbBlue  = palette [j++]<<2;
					mybmi.bmiColors [i].rgbReserved = 0;
					}
				}
			m_DIB =::CreateDIBSection(m_DC.m_hDC, (BITMAPINFO *) &mybmi, DIB_RGB_COLORS, &m_pvBits, NULL, 0);
			if (m_DIB) {
				m_DC.SelectObject(m_DIB);
			}
		}
	}
	// if DIB exists, then use our own DC instead of the View DC
	if (m_DIB) {
		m_pDC = &m_DC;
	}
#endif
}

//----------------------------------------------------------------------------
// ClearView()
//
// TODO: only clear the dirty area defined by the clip region
//----------------------------------------------------------------------------

void CMineView::ClearView()
{
// clear the dib or the view
#if OGL_RENDERING
glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glLoadIdentity ();
#else
if (m_DIB)
	memset(m_pvBits, 0, m_viewWidth * m_viewHeight * m_viewDepth);
else {
	CRect rect;
	GetClientRect(rect);
	FillRect(m_pDC->m_hDC, rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
#endif
}

//--------------------------------------------------------------------------
//			  DrawObjects()
//--------------------------------------------------------------------------

bool CMineView::ViewObject (CGameObject *objP)
{
switch(objP->m_info.type) {
	case OBJ_ROBOT:
	case OBJ_CAMBOT:
	case OBJ_SMOKE:
	case OBJ_EXPLOSION:
	case OBJ_MONSTERBALL:
		return ViewObject (eViewObjectsRobots);
	case OBJ_EFFECT:
		return ViewObject (eViewObjectsEffects);
	case OBJ_HOSTAGE:
		return ViewObject (eViewObjectsHostages);
	case OBJ_PLAYER:
	case OBJ_COOP:
		return ViewObject (eViewObjectsPlayers);
	case OBJ_WEAPON:
		return ViewObject (eViewObjectsWeapons);
	case OBJ_POWERUP:
		switch (powerup_types [objP->m_info.id]) {
			case POWERUP_WEAPON_MASK:
				return ViewObject (eViewObjectsWeapons);
			case POWERUP_POWERUP_MASK:
				return ViewObject (eViewObjectsPowerups);
			case POWERUP_KEY_MASK:
				return ViewObject (eViewObjectsKeys);
			default:
				return false;
			}
	case OBJ_CNTRLCEN:
		return ViewObject (eViewObjectsControlCenter);
	}
return false;
}

//------------------------------------------------------------------------

void CMineView::HiliteTarget (void)
{
	INT32 i, nTarget;

CGameObject *objP = theMine->CurrObj ();
if ((objP->m_info.type != OBJ_EFFECT) || (objP->m_info.id != LIGHTNING_ID))
	return;
theMine->Other ()->nObject = theMine->Current ()->nObject;
if (nTarget = objP->rType.lightningInfo.nTarget)
	for (i = 0, objP = theMine->Objects (0); i < theMine->GameInfo ().objects.count; i++, objP++)
		if ((objP->m_info.type == OBJ_EFFECT) && (objP->m_info.id == LIGHTNING_ID) && (objP->rType.lightningInfo.nId == nTarget)) {
			theMine->Other ()->nObject = i;
			break;
			return;
			}
}
 
/////////////////////////////////////////////////////////////////////////////
// CMineView printing

BOOL CMineView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CMineView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CMineView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CMineView diagnostics

#ifdef _DEBUG
void CMineView::AssertValid() const
{
	CView::AssertValid();
}

void CMineView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDlcDoc* CMineView::GetDocument() // non-debug version is inline
{
	if (m_pDocument)
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDlcDoc)));
	return (CDlcDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMineView message handlers

BOOL CMineView::OnEraseBkgnd(CDC* pDC)
{
//	return CView::OnEraseBkgnd(pDC);
	return TRUE;
}

                        /*--------------------------*/

void CMineView::OnSize(UINT nType, INT32 cx, INT32 cy)
{
	CRect	rc;

GetClientRect(rc);
// define global screen variables (these must be redefined if window is sized)
left = rc.left;
top = rc.top;
right = rc.right;
bottom = rc.bottom;
x_center = (right-left)/2;
y_center = (bottom-top)/2;
//aspect_ratio = (y_center/7.0) / (x_center/10.0);
aspect_ratio = (double) rc.Height () / (double) rc.Width ();
x_max = 8*right;
y_max = 8*bottom;
//if (theApp.MainFrame () && ((m_viewWidth != cx) || (m_viewHeight != cy)))
//	theApp.MainFrame ()->ResetPaneMode ();
CView::OnSize (nType, cx, cy);
m_bUpdate = true;
}

                        /*--------------------------*/

static double zoomScales [2] = {1.2, 1.033};

INT32 CMineView::ZoomFactor (INT32 nSteps, double min, double max)
{
double zoom;
INT32 i;

for (zoom = log(10*m_size.v.x), i = 0; i < nSteps; i++) {
	zoom /= log (1.2);
	if ((zoom < min) || (zoom > max))
		return i;
	}
return nSteps; //(INT32) ((zoom > 0) ? zoom + 0.5: zoom - 0.5);
}

                        /*--------------------------*/

void CMineView::Zoom (INT32 nSteps, double zoom)
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

INT32 CMineView::ZoomIn (INT32 nSteps, bool bSlow)
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

INT32 CMineView::ZoomOut(INT32 nSteps, bool bSlow)
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

INT32 i;
for (i = theMine->VertCount (); i; i--, a++) {
	a->x += m_xRenderOffs;
	a->y += m_yRenderOffs;
	}
}

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

                        /*--------------------------*/

INT32 CMineView::FitToView (void)
{
if (!theMine) return 1;

	CRect			rc (LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	double		zoomX, zoomY, zoom;
	INT32			dx, dy;

DelayRefresh (true);
//CenterMine ();
//SetViewPoints (&rc);
m_move.Clear ();
m_view.SetViewInfo (10000, m_viewWidth, m_viewHeight);
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
DelayRefresh (false);
Refresh ();
return 1;
}

                        /*--------------------------*/

void CMineView::Rotate (char direction, double angle)
{
	static double a = 0;
INT32 i = direction - 'X';
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

void CMineView::Pan (char direction, INT32 value)
{
if (!value)
	return;
INT32 i = direction - 'X';
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

void CMineView::CenterMine()
{
if (!theMine) return;

//	CDlcDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

	CVertex *verts;
	CVertex	vMin (0x7fffffff, 0x7fffffff, 0x7fffffff), vMax (-0x7fffffff, -0x7fffffff, -0x7fffffff);
	INT32 i;

verts = theMine->Vertices (0);
for (i = 0; i < theMine->VertCount ();i++, verts++) {
	vMin = Min (vMin, *verts);
	vMax = Max (vMax, *verts);
	}
m_spin.Set (M_PI / 4.0, M_PI / 4.0, 0.0);
m_move = CDoubleVector (Average (vMin, vMax));
CDoubleVector v = vMax - vMin;
INT32 maxVal = INT32 (max (max (v.v.x, v.v.y), v.v.z) / 20);
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
Refresh (false);
}

                        /*--------------------------*/

void CMineView::CenterCube (void)
{
if (!theMine) return;

	CSegment& seg = theMine->Segments (0) [m_Current->nSegment];
	CVertex *vMine = theMine->Vertices (0);
	INT16 *vSeg = seg.m_info.verts;

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
if (!theMine) return;

CDlcDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
if (!pDoc) return;

m_move = -theMine->Objects (m_Current->nObject)->m_location.pos;
Refresh (false);
}

                        /*--------------------------*/

void CMineView::SetViewOption(eViewOptions option)
{
m_viewOption = option;
Refresh ();
}

                        /*--------------------------*/

void CMineView::ToggleViewMine (eMineViewFlags flag)
{
	m_viewMineFlags ^= flag;
	EnableDeltaShading ((m_viewMineFlags & eViewMineDeltaLights) != 0, -1, -1);
	Refresh ();
}

                        /*--------------------------*/

void CMineView::ToggleViewObjects(eObjectViewFlags mask)
{
	m_viewObjectFlags ^= mask;
	Refresh ();
}

                        /*--------------------------*/

void CMineView::SetViewMineFlags(UINT32 mask)
{
	m_viewMineFlags = mask;
	Refresh ();
}

                        /*--------------------------*/

void CMineView::SetViewObjectFlags(UINT32 mask)
{
	m_viewObjectFlags = mask;
	Refresh ();
}

                        /*--------------------------*/

void CMineView::SetSelectMode(UINT32 mode)
{
theMine->SetSelectMode ((INT16) mode);
theApp.MainFrame ()->UpdateSelectButtons ((eSelectModes) mode);
m_selectMode = mode; 
Refresh (false);
}

                        /*--------------------------*/

BOOL CMineView::OnSetCursor (CWnd* pWnd, UINT nHitTest, UINT message)
{
//if (m_bUpdateCursor) {
//::SetCursor (AfxGetApp()->LoadStandardCursor (nIdCursors [m_mouseState]));
//	return TRUE;
//	}
return CView::OnSetCursor (pWnd, nHitTest, message);
}

                        /*--------------------------*/

BOOL CMineView::SetCursor (HCURSOR hCursor)
{
if (!hCursor) // || (hCursor == m_hCursor))
   return FALSE;
::SetClassLong (GetSafeHwnd (), -12 /*INT32 (GCL_HCURSOR)*/, INT32 (hCursor));
return TRUE;
}
                        
                        /*--------------------------*/

void CMineView::SetMouseState (INT32 newMouseState)
{
if (newMouseState != m_mouseState) {
	m_lastMouseState = m_mouseState;
	m_mouseState = newMouseState;
	m_bUpdateCursor = true;
	SetCursor (m_hCursors [m_mouseState]);
	m_bUpdateCursor = false;
#ifdef _DEBUG
  	INFOMSG (szMouseStates [m_mouseState]);
#endif
	}
}

                        /*--------------------------*/

void CMineView::OnMouseMove (UINT nFlags, CPoint point)
{
	CPoint change = m_lastMousePos - point;

if (GetFocus () != this)
	SetFocus ();
if (change.x || change.y) {
	switch(m_mouseState) {
		case eMouseStateIdle:
		case eMouseStatePan:
		case eMouseStateRotate:
			// if Control Key down
			if (nFlags & MK_CONTROL) {
				// if Shift then rotate
				if (nFlags & MK_SHIFT) {
					SetMouseState (eMouseStateRotate);
					Rotate('Y', -((double) change.x) / 200.0);
					Rotate('X', ((double) change.y) / 200.0);
					}
				else {// else move
					SetMouseState (eMouseStatePan);
					if (change.x)
						Pan ('X', change.x);
					if (change.y)
	#if OGL_RENDERING
						Pan ('Y', change.y);
	#else
						Pan ('Y', -change.y);
	#endif
					}
				}
			else if ((m_mouseState == eMouseStatePan) || (m_mouseState == eMouseStateRotate))
				SetMouseState (eMouseStateIdle);
			break;

		case eMouseStateButtonDown:
			if (nFlags & MK_CONTROL)
				SetMouseState (eMouseStateZoom);
			else {
				INT32 v = theMine->CurrVert ();
				if ((abs (m_clickPos.x - m_viewPoints [v].x) < 5) && 
					 (abs (m_clickPos.y - m_viewPoints [v].y) < 5)) {
					SetMouseState (eMouseStateInitDrag);
					UpdateDragPos ();
					}
				else {
					SetMouseState (eMouseStateRubberBand);
					UpdateRubberRect (point);
					}
				}
			break;

		case eMouseStateSelect:
			m_clickPos = point;
			SetMouseState (eMouseStateRubberBand);
			UpdateRubberRect (point);
			break;

		case eMouseStateDrag:
			if (change.x || change.y)
				UpdateDragPos ();
			break;

		case eMouseStateZoom:
		case eMouseStateZoomIn:
		case eMouseStateZoomOut:
			if ((change.x > 0) || ((change.x == 0) && (change.y < 0))) {
				SetMouseState (eMouseStateZoomOut);
				ZoomOut (1, true);
				}
			else {
				SetMouseState (eMouseStateZoomIn);
				ZoomIn (1, true);
				}
			break;
			
		case eMouseStateRubberBand:
			UpdateRubberRect (point);
			break;
		}
	m_lastMousePos = point;
	}
//CView::OnMouseMove(nFlags, point);
}

                        /*--------------------------*/

void CMineView::OnLButtonDown (UINT nFlags, CPoint point)
{
SetMouseState (eMouseStateButtonDown);
m_clickPos = point;
m_clickState = nFlags;
m_selectTimer = SetTimer (4, 500U, NULL);
CView::OnLButtonDown (nFlags, point);
}

                        /*--------------------------*/

void CMineView::OnLButtonUp (UINT nFlags, CPoint point)
{
if (m_selectTimer != -1) {
	KillTimer (m_selectTimer);
	m_selectTimer = -1;
	}
m_releasePos = point;
m_releaseState = nFlags;
if (m_mouseState == eMouseStateButtonDown)
	if (m_clickState & MK_CONTROL)
		ZoomIn ();
	else {
		SetMouseState (eMouseStateIdle);
		SelectCurrentSegment (1, m_clickPos.x, m_clickPos.y);
		}
else if (m_mouseState == eMouseStateRubberBand) {
   ResetRubberRect ();
	MarkRubberBandedVertices ();
	}
else if (m_mouseState == eMouseStateDrag)
	FinishDrag ();
SetMouseState (eMouseStateIdle);
CView::OnLButtonUp (nFlags, point);
}

                        /*--------------------------*/

void CMineView::OnRButtonDown (UINT nFlags, CPoint point)
{
SetMouseState (eMouseStateButtonDown);
m_clickPos = point;
m_clickState = nFlags;
CView::OnRButtonDown(nFlags, point);
}

                        /*--------------------------*/

void CMineView::OnRButtonUp (UINT nFlags, CPoint point)
{
m_releasePos = point;
m_releaseState = nFlags;
if (m_mouseState == eMouseStateButtonDown)
	if (m_clickState & MK_CONTROL)
		ZoomOut ();
	else {
		SetMouseState (eMouseStateIdle);
		SelectCurrentObject (m_clickPos.x, m_clickPos.y);
		}
else if (m_mouseState == eMouseStateRubberBand)
   ResetRubberRect ();
SetMouseState (eMouseStateIdle);
CView::OnRButtonUp(nFlags, point);
}

                        /*--------------------------*/

void CMineView::Invalidate (BOOL bErase)
{
CWnd::Invalidate (bErase);
}

                        /*--------------------------*/

void CMineView::InvalidateRect (LPCRECT lpRect, BOOL bErase)
{
CWnd::InvalidateRect (lpRect, bErase);
}

                        /*--------------------------*/

void CMineView::Refresh (bool bAll)
{
if (!theMine) return;

	static bool bRefreshing = false;

if (!(bRefreshing || m_nDelayRefresh)) {
	bRefreshing = true;
	InvalidateRect (NULL, TRUE);
//	SetFocus ();
	if (bAll && (m_mouseState == eMouseStateIdle)) {
		theApp.ToolView ()->Refresh ();
		theApp.TextureView ()->Refresh ();
//		UpdateWindow ();
		}
	m_bUpdate = true;
	bRefreshing = false;
	}
}

                        /*--------------------------*/

void CMineView::OnUpdate (CView* pSender, LPARAM lHint, CGameObject* pHint)
{
//m_bUpdate = true;
//InvalidateRect(NULL);
Refresh ();
}

                        /*--------------------------*/

bool CMineView::VertexVisible (INT32 v)
{
	CSegment	*segP;
	INT32			i, j;

if (!m_nViewDist)
	return true;
for (i = theMine->SegCount (), segP = theMine->Segments (0); i; i--, segP++)
	for (j = 0; j < MAX_VERTICES_PER_SEGMENT; j++)
		if ((segP->m_info.verts [j] == v) && Visible (segP))
			return true;
return false;
}

                        /*--------------------------*/

void CMineView::MarkRubberBandedVertices (void)
{
if (!theMine) return;

	APOINT	*pa = m_viewPoints;
	INT32		x, y;

INT32 i;
for (i = 0; i < theMine->VertCount (); i++, pa++) {
	x = pa->x;
	y = pa->y;
	if (BETWEEN (m_clickPos.x, x, m_releasePos.x) &&
		 BETWEEN (m_clickPos.y, y, m_releasePos.y) &&
		 VertexVisible (i)) {
		if (m_clickState & MK_SHIFT)
			theMine->VertStatus (i) &= ~MARKED_MASK;
		else
			theMine->VertStatus (i) |= MARKED_MASK;
		m_bUpdate = true;
		}
	}
if (m_bUpdate) {
	theMine->UpdateMarkedCubes ();
	Refresh ();
	}
}

//==========================================================================
//==========================================================================
void CMineView::RefreshObject(INT16 old_object, INT16 new_object) 
{
theMine->Current ()->nObject = new_object;
theApp.ToolView ()->Refresh ();
Refresh (false);
}

//--------------------------------------------------------------------------
//		       select_current_object()
//
//  ACTION - finds object pointed to by mouse then draws.
//
//--------------------------------------------------------------------------

void CMineView::SelectCurrentObject (long xMouse, long yMouse) 
{
CGameObject *objP;
INT16 closest_object;
INT16 i;
double radius,closest_radius;
APOINT pt;
CGameObject temp_obj;

// default to object 0 but set radius very large
closest_object = 0;
closest_radius = 1.0E30;

// if there is a secret exit, then enable it in search
INT32 enable_secret = FALSE;
if (theApp.IsD2File ())
	for(i=0;i<(INT16)theMine->GameInfo ().triggers.count;i++)
		if (theMine->Triggers (i)->m_info.type ==TT_SECRET_EXIT) {
			enable_secret = TRUE;
			break;
			}

for (i=0;i<=theMine->GameInfo ().objects.count;i++) {
	BOOL drawable = FALSE;
	// define temp object type and position for secret object selection
	if (i == theMine->GameInfo ().objects.count && theApp.IsD2File () && enable_secret) {
		objP = &temp_obj;
		objP->m_info.type = OBJ_PLAYER;
		// define objP->position
		CalcSegmentCenter (objP->m_location.pos, (UINT16)theMine->SecretCubeNum ());
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

INT32 Side (APOINT &p0, APOINT &p1, APOINT &p2)
{
return ((INT32) p0.y - (INT32) p1.y) * ((INT32) p2.x - (INT32) p1.x)  - 
		 ((INT32) p0.x - (INT32) p1.x) * ((INT32) p2.y - (INT32) p1.y);
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

bool CMineView::SelectCurrentSegment (INT16 direction, long xMouse, long yMouse) 
{
  CSegment		*segP;
  CRect			rc;
//  extern INT16 xMouse,yMouse;
  INT16			cur_segment, next_segment;
  INT16			i, j;
  INT32			x, y;
  APOINT			sideVerts [4], mousePos;
  bool			bFound = false;

/* find next segment which is within the cursor position */
GetClientRect (rc);
next_segment = cur_segment = theMine->Current ()->nSegment;
mousePos.x = (INT16) xMouse;
mousePos.y = (INT16) yMouse;
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
theApp.ToolView ()->Refresh ();
Refresh ();
return true;
}

//-------------------------------------------------------------------------
// calculate_segment_center()
//-------------------------------------------------------------------------

void CMineView::CalcSegmentCenter (CVertex& pos, INT16 nSegment)
{
CSegment *segP = theMine->Segments (nSegment);
CVertex *vMine = theMine->Vertices (0);
INT16 *vSeg = segP->m_info.verts;
pos  =
   vMine [vSeg [0]] +
   vMine [vSeg [1]] +
   vMine [vSeg [2]] +
   vMine [vSeg [3]] +
   vMine [vSeg [4]] +
   vMine [vSeg [5]] +
   vMine [vSeg [6]] +
   vMine [vSeg [7]];
pos /= 8.0;
}

                        /*--------------------------*/
                        
BOOL CMineView::DrawRubberBox ()
{
if (!theMine) return FALSE;

	static CRect	prevRect (0, 0, 0, 0);
      
if (m_mouseState != eMouseStateRubberBand)
	return FALSE;
if ((m_rubberRect.Width () || m_rubberRect.Height ())) {
      CPen     pen (PS_DOT, 1, RGB (0,0,0));
      CPen *   pOldPen;
      POINT    rubberPoly [5];
   
   m_pDC->SetROP2 (R2_XORPEN);
   pOldPen = m_pDC->SelectObject (&pen);
   rubberPoly [0].x = m_rubberRect.left + RUBBER_BORDER;
   rubberPoly [0].y = m_rubberRect.top + RUBBER_BORDER;
   rubberPoly [1].x = m_rubberRect.right - RUBBER_BORDER;
   rubberPoly [1].y = m_rubberRect.top + RUBBER_BORDER;
   rubberPoly [2].x = m_rubberRect.right - RUBBER_BORDER;
   rubberPoly [2].y = m_rubberRect.bottom - RUBBER_BORDER;
   rubberPoly [3].x = m_rubberRect.left + RUBBER_BORDER;
   rubberPoly [3].y = m_rubberRect.bottom - RUBBER_BORDER;
   rubberPoly [4] = rubberPoly [0];
   m_pDC->Polyline (rubberPoly, sizeof (rubberPoly) / sizeof (POINT));
   m_pDC->SetROP2 (R2_COPYPEN);
   m_pDC->SelectObject (pOldPen);
   }
return TRUE;
}                        
                        
                        /*--------------------------*/
                        
void CMineView::UpdateRubberRect (CPoint pt)
{
if (!theMine) return;

if (m_mouseState == eMouseStateZoom)
	return;
if (m_mouseState == eMouseStateDrag)
	return;
if (m_mouseState == eMouseStateButtonDown)
   SetCapture ();
else {
   //InvalidateRect (&m_rubberRect, FALSE);
   //UpdateWindow ();
   }
CRect rc = m_rubberRect;
if (m_clickPos.x < pt.x) {
   rc.left = m_clickPos.x - RUBBER_BORDER;
   rc.right = pt.x + RUBBER_BORDER;
   }
else if (m_clickPos.x > pt.x) {
   rc.right = m_clickPos.x + RUBBER_BORDER;
   rc.left = pt.x - RUBBER_BORDER;
   }
if (m_clickPos.y < pt.y) {
   rc.top = m_clickPos.y - RUBBER_BORDER;
   rc.bottom = pt.y + RUBBER_BORDER;
   }
else if (m_clickPos.y > pt.y) {
   rc.bottom = m_clickPos.y + RUBBER_BORDER;
   rc.top = pt.y - RUBBER_BORDER;
   }
if (rc != m_rubberRect) {
	SetMouseState (eMouseStateRubberBand);
	InvalidateRect (&m_rubberRect, TRUE);
	UpdateWindow ();
	m_rubberRect = rc;
	InvalidateRect (&m_rubberRect, TRUE);
	//UpdateWindow ();
	}
}                        
                        
                        /*--------------------------*/
                        
void CMineView::ResetRubberRect ()
{
if (!theMine) return;

ReleaseCapture ();
InvalidateRect (&m_rubberRect, FALSE);
UpdateWindow ();
m_rubberRect.left = m_rubberRect.right =
m_rubberRect.top = m_rubberRect.bottom = 0;
}

                        /*--------------------------*/
                        
BOOL CMineView::UpdateDragPos (void)
{
if (!theMine) return FALSE;

if ((m_mouseState != eMouseStateInitDrag) && (m_mouseState != eMouseStateDrag))
	return FALSE;

	INT16 nVert = sideVertTable [theMine->Current ()->nSide] [theMine->Current ()->nPoint];
	INT16 v = theMine->CurrSeg ()->m_info.verts [nVert];
	INT16 x = m_viewPoints [v].x;
	INT16 y = m_viewPoints [v].y;

if (m_mouseState == eMouseStateInitDrag) {
	SetMouseState (eMouseStateDrag);
// SetCapture ();
	m_highlightPos.x = x;
	m_highlightPos.y = y;
	m_lastDragPos = m_highlightPos;
	}
HighlightDrag (nVert, x, y);
	
//InvalidateRect (NULL, TRUE);
return TRUE;
}

                        /*--------------------------*/
                        
void CMineView::HighlightDrag (INT16 nVert, long x, long y) 
{
if (!theMine) return;

m_pDC->SelectObject((HBRUSH) GetStockObject (NULL_BRUSH));
//m_pDC->SetROP2 (R2_NOT);
m_pDC->SetROP2 (R2_NOT);
m_pDC->Ellipse (x-4, y-4, x+4, y+4);

CRect	rc (x, y, x, y);
INT32 i;
for (i = 0; i < 3; i++) {
	m_pDC->MoveTo (x, y);
	INT16 nVert2 = connectPointTable [nVert] [i];
	INT16 x2 = m_viewPoints [theMine->CurrSeg ()->m_info.verts [nVert2]].x;
	INT16 y2 = m_viewPoints [theMine->CurrSeg ()->m_info.verts [nVert2]].y;
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
if (!theMine) return FALSE;

if (m_mouseState != eMouseStateDrag)
	return FALSE;
if (m_lastMousePos == m_lastDragPos)
	return FALSE;

	INT16 nVert;
	INT16 x, y;
	INT32 i;

nVert = sideVertTable [theMine->Current ()->nSide] [theMine->Current ()->nPoint];

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
if (!theMine) return;

//ReleaseCapture ();
	INT32		m_changesMade = 1;
	INT32		i, new_vert, count = 0;
	long		xPos,yPos;
	INT16		xPoint,yPoint;
	INT16		point1,vert1;
	INT16		point2,vert2;

xPos = m_releasePos.x;
yPos = m_releasePos.y;
point1 = sideVertTable [theMine->Current ()->nSide] [theMine->Current ()->nPoint];
vert1 = theMine->Segments (0) [theMine->Current ()->nSegment].m_info.verts [point1];
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
			vert2 = theMine->Segments (0) [theMine->Current ()->nSegment].m_info.verts [i];
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
			vert2 = theMine->Segments (0) [theMine->Current ()->nSegment].m_info.verts [point2];
			if (Distance (*theMine->Vertices (new_vert), *theMine->Vertices (vert2)) >= 1000.0) {
				ErrorMsg ("Cannot move this point so far away.");
				break;
				}
			}
		if (i==3) { //
			// replace origional vertex with new vertex
			theMine->Segments () [theMine->Current ()->nSegment].m_info.verts [point1] = new_vert;
			// all unused vertices
			theMine->DeleteUnusedVertices ();
			theMine->FixChildren ();
			theMine->SetLinesToDraw ();
			}
		}	
	}
else {
	// no vertex found, just drop point along screen axii
	APOINT apoint;
	apoint.x = (INT16) xPos;
	apoint.y = (INT16) yPos;
	apoint.z = m_viewPoints [vert1].z;
	m_view.Unproject (*theMine->Vertices (vert1), apoint);
	}
Refresh ();
}


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

								/*---------------------------*/

BOOL CMineView::OnMouseWheel (UINT nFlags, INT16 zDelta, CPoint pt)
{
	CRect	rc;

GetWindowRect (rc);
if ((pt.x < rc.left) || (pt.x >= rc.right) || (pt.y < rc.top) || (pt.y >= rc.bottom))
	return theApp.TextureView ()->OnMouseWheel (nFlags, zDelta, pt);
if (zDelta > 0)
	ZoomIn (zDelta / WHEEL_DELTA);
else
	ZoomOut (-zDelta / WHEEL_DELTA);
return 0;
}

                        /*--------------------------*/

void CMineView::OnSelectPrevTab ()
{
theApp.MainFrame ()->ShowTools ();
theApp.ToolView ()->PrevTab ();
}

                        /*--------------------------*/

void CMineView::OnSelectNextTab ()
{
theApp.MainFrame ()->ShowTools ();
theApp.ToolView ()->NextTab ();
}

                        /*--------------------------*/

void CMineView::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
UINT nPos = GetScrollPos (SB_HORZ);
CRect rect;
GetClientRect(rect);

m_bDelayRefresh = true;
switch (scrollCode) {
	case SB_LINEUP:
		nPos--;
		break;
	case SB_LINEDOWN:
		nPos++;
		break;
	case SB_PAGEUP:
		nPos -= m_viewWidth;
		break;
	case SB_PAGEDOWN:
		nPos += m_viewWidth;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nPos = thumbPos;
		break;
	case SB_ENDSCROLL:
		m_bDelayRefresh = false;
		Refresh (false);
		return;
}
SetScrollPos (SB_HORZ, nPos, TRUE);
Refresh ();
}

                        /*--------------------------*/

void CMineView::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
INT32 nPos = GetScrollPos (SB_VERT);
CRect rect;
GetClientRect(rect);

m_bDelayRefresh = true;
switch (scrollCode) {
	case SB_LINEUP:
		nPos--;
		break;
	case SB_LINEDOWN:
		nPos++;
		break;
	case SB_PAGEUP:
		nPos -= m_viewHeight;
		break;
	case SB_PAGEDOWN:
		nPos += m_viewHeight;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nPos = thumbPos;
		break;
	case SB_ENDSCROLL:
		m_bDelayRefresh = false;
		Refresh (false);
		return;
}
if (nPos < 0)
	nPos = 0;
else if (nPos >= m_yScrollRange)
	nPos = m_yScrollRange - 1;
SetScrollPos (SB_VERT, nPos, TRUE);
m_yRenderOffs = nPos - m_yScrollCenter;
Refresh ();
}

                        /*--------------------------*/


//eof