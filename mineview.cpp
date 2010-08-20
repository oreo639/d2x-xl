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

#if OGL_RENDERING

# define OGL_MAPPED 1

static HDC		glHDC;
static double	glRotMat [3][3] = {{1,0,0},{0,1,0},{0,0,1}};
static double	glAngle [3] = {0,0,0};
static double	glPan [3] = {0,0,-500};
static double	uvMap [4][2] = {{0.0,0.0},{0.0,1.0},{1.0,1.0},{1.0,0.0}};
static UINT8	rgbBuf [64*64*4];
static GLuint	glHandles [910];
static UINT8	*glPalette = NULL;
static BOOL		glFitToView = FALSE;
static INT32		glMinZ = 1;
static INT32		glMaxZ = 1;
static bool		glInit = true;
#endif

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
Reset ();
}

                        /*--------------------------*/

void CMineView::Reset ()
{
m_viewWidth = m_viewHeight = m_viewDepth = 0;	// force OnDraw to initialize these
if (GetMine ())
	m_mine->SetSplineActive (false);
m_bUpdate = true;
m_mouseState  = 
m_lastMouseState = eMouseStateIdle;
m_selectMode = eSelectSide;
m_lastSegment = 0;

m_x0 = 0;
m_y0 = 0;
m_z0 = 0;
m_spinx = M_PI/4.0;
m_spiny = M_PI/4.0;
m_spinz = 0.0;
m_movex = 0.0f;
m_movey = 0.0f;
m_movez = 0.0f;
m_sizex = 1.0f;
m_sizey = 1.0f;
m_sizez = 1.0f;

// calculate transformation m_matrix based on move, size, and spin
m_matrix.Set(m_movex, m_movey, m_movez,
				 m_sizex, m_sizey, m_sizez,
				 m_spinx, m_spiny, m_spinz);
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
	m_pDC->MoveTo (x_center,y_center-(INT32)(10.0*m_sizey)+1);
	m_pDC->LineTo (x_center,y_center+(INT32)(10.0*m_sizey)+1);
	m_pDC->MoveTo (x_center-(INT32)(10.0*m_sizex/**aspect_ratio*/)+1,y_center);
	m_pDC->LineTo (x_center+(INT32)(10.0*m_sizex/**aspect_ratio*/)+1,y_center);
	}
if (m_nMineCenter == 2) {
	// draw a globe
	// 5 circles around each axis at angles of 30, 60, 90, 120, and 150
	// each circle has 18 points
	CFixVector circle;
	APOINT pt;

	m_pDC->SelectObject (m_penCyan);
	INT32 i, j;
	for (i=-60;i<=60;i+=30) {
		for (j=0;j<=360;j+=15) {
			double scale = (FIX)(5*F1_0*cos((double)i*(M_PI/180.0)));
			circle.z = (FIX)(5*F1_0*sin((double)i*(M_PI/180.0)));
			circle.x = (FIX)(scale*cos((double)j*(M_PI/180.0)) );
			circle.y = (FIX)(scale*sin((double)j*(M_PI/180.0)) );
			circle.x -= (FIX)(m_matrix.M[1][0] * F1_0);
			circle.y -= (FIX)(m_matrix.M[2][0] * F1_0);
			circle.z -= (FIX)(m_matrix.M[3][0] * F1_0);
			m_matrix.SetPoint (&circle,&pt);
			if (j==0)
				m_pDC->MoveTo (pt.x,pt.y);
			else if (pt.z <= 0)
				m_pDC->LineTo (pt.x,pt.y);
			else 
				m_pDC->MoveTo (pt.x,pt.y);
			}
		}
	m_pDC->SelectObject (m_penGreen);
	for (i=-60;i<=60;i+=30) {
		for (j=0;j<=360;j+=15) {
			double scale = (FIX)(5*F1_0*cos((double)i*(M_PI/180.0)));
			circle.y = (FIX)(5*F1_0*sin((double)i*(M_PI/180.0)));
			circle.z = (FIX)(scale*cos((double)j*(M_PI/180.0)) );
			circle.x = (FIX)(scale*sin((double)j*(M_PI/180.0)) );
			circle.x -= (FIX)(m_matrix.M[1][0] * F1_0);
			circle.y -= (FIX)(m_matrix.M[2][0] * F1_0);
			circle.z -= (FIX)(m_matrix.M[3][0] * F1_0);
			m_matrix.SetPoint (&circle,&pt);
			if (j==0)
				m_pDC->MoveTo (pt.x,pt.y);
			else if (pt.z <= 0)
				m_pDC->LineTo (pt.x,pt.y);
			else 
				m_pDC->MoveTo (pt.x,pt.y);
			}
		}
	m_pDC->SelectObject (m_penGray);
	for (i=-60;i<=60;i+=30) {
		for (j=0;j<=360;j+=15) {
			double scale = (FIX)(5*F1_0*cos((double)i*(M_PI/180.0)));
			circle.x    = (FIX)(5*F1_0*sin((double)i*(M_PI/180.0)));
			circle.y = (FIX)(scale*cos((double)j*(M_PI/180.0)) );
			circle.z = (FIX)(scale*sin((double)j*(M_PI/180.0)) );
			circle.x -= (FIX)(m_matrix.M[1][0] * F1_0);
			circle.y -= (FIX)(m_matrix.M[2][0] * F1_0);
			circle.z -= (FIX)(m_matrix.M[3][0] * F1_0);
			m_matrix.SetPoint (&circle,&pt);
			if (j==0)
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
	if (!(pDoc && pDoc->GetMine ())) 
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
		// Calculate m_matrix M based on IM and "move x,y,z"
		//m_matrix.Calculate(m_movex, m_movey, m_movez);

		// Set view m_matrix misc. information
		//m_matrix.SetViewInfo(m_depthPerception, m_viewWidth, m_viewHeight);
		SetViewPoints ();
		ShiftViewPoints ();
		// make a local copy the mine's selection
		m_Current = m_mine->Current ();

		// draw the level
		switch(m_viewOption)		{
			case eViewAllLines:
				DrawWireFrame(m_mine, false);
				break;

			case eViewHideLines:
				DrawWireFrame(m_mine, false); // temporary
				break;

			case eViewNearbyCubeLines:
				DrawWireFrame(m_mine, false); // temporary
				break;

			case eViewPartialLines:
				DrawWireFrame(m_mine, bPartial = true);
				break;

			case eViewTextureMapped:
				DrawTextureMappedCubes(m_mine);
				break;
			}
/*
	if (m_viewFlags & eViewMineWalls)
		DrawWalls (mine);
	if (m_viewFlags & eViewMineLights)
		DrawLights (mine);
	if (m_viewObjects)
		DrawObjects (mine, bPartial);
*/
		}
#endif
DrawRubberBox ();
DrawHighlight (m_mine);
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
	CMine *mine = GetMine ();
	LIGHT_TIMER *plt = lightTimers;
	CFlickeringLight *pfl = mine->FlickeringLights ();
	INT32 i, light_delay;

for (i = mine->FlickerLightCount (); i; i--, pfl++, plt++) {
	light_delay = (pfl->delay * 100 /*+ F0_5*/) / F1_0;
	if (light_delay) {
		if (++plt->ticks == light_delay) {
			plt->ticks = 0;
			plt->impulse = (plt->impulse + 1) % 32;
			}
		}
	else
		plt->impulse = (plt->impulse + 1) % 32;
	}
}

                        /*--------------------------*/
#ifdef _DEBUG
static INT32 qqq1 = -1, qqq2 = 0;
#endif

bool CMineView::SetLightStatus (void)
{
	CMine *mine = GetMine ();
	INT32 h, i, j;
	CLightDeltaIndex *pdli = mine->LightDeltaIndex ();
	LIGHT_TIMER *plt;
	CFlickeringLight *pfl = mine->FlickeringLights ();
	LIGHT_STATUS *pls;
	bool bChange = false;
	bool bD2XLights = (mine->LevelVersion () >= 15) && (mine->GameInfo ().fileinfo.version >= 34);
	INT16 nSrcSide, nSrcSeg, nSegment, nSide;

CLightDeltaValue *dll = mine->LightDeltaValues ();
if (!dll)
	return false;
// search delta light index to see if current side has a light
pls = lightStatus [0];
for (i = mine->SegCount (); i; i--)
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++, pls++)
		pls->bWasOn = pls->bIsOn;
for (h = 0; h < mine->GameInfo ().lightDeltaIndices.count; h++, pdli++) {
	if (bD2XLights) {
		nSrcSide = pdli->m_nSegment;
		nSrcSeg = pdli->m_nSide;
		i = pdli->count;
		}
	else {
		nSrcSide = pdli->m_nSegment;
		nSrcSeg = pdli->m_nSide;
		i = pdli->count;
		}	
	j = mine->GetFlickeringLight (nSrcSide, nSrcSeg);
	if (j < 0)
		continue;	//shouldn't happen here, as there is a delta light value, but you never know ...
	dll = mine->LightDeltaValues (pdli->index);
	for (; i; i--, dll++) {
		nSegment = dll->m_nSegment;
		nSide = dll->m_nSide;
		if (m_bShowLightSource) {
			if ((nSegment != nSrcSide) || (nSide != nSrcSeg)) 
				continue;
			if (0 > mine->GetFlickeringLight (nSegment, nSide))
				continue;
			}
		else if (((nSegment != nSrcSide) || (nSide != nSrcSeg)) &&
			 (0 <= mine->GetFlickeringLight (nSegment, nSide)))
			continue;
		pls = lightStatus [nSegment] + nSide;
		plt = lightTimers + j;
		pls->bIsOn = (pfl [j].mask & (1 << lightTimers [j].impulse)) != 0;
		if (pls->bWasOn != pls->bIsOn)
			bChange = true;
		}
	}
return bChange;
}

                        /*--------------------------*/

void CMineView::OnTimer (UINT_PTR nIdEvent)
{
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
	MEMSET (lightTimers, 0, sizeof (lightTimers));
	MEMSET (lightStatus, 0xff, sizeof (lightStatus));
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
	MEMSET(m_pvBits, 0, m_viewWidth * m_viewHeight * m_viewDepth);
else {
	CRect rect;
	GetClientRect(rect);
	FillRect(m_pDC->m_hDC, rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}
#endif
}

//----------------------------------------------------------------------------
// CalcSegDist
//----------------------------------------------------------------------------

void CMineView::CalcSegDist (CMine *mine)
{
	INT32			h, i, j, c, nDist, segNum = mine->SegCount (), sideNum;
	CSegment	*segI, *segJ;
	INT16			*segRef = new INT16 [segNum];

for (i = segNum, segI = mine->Segments (); i; i--, segI++)
	segI->nIndex = -1;
segRef [0] = mine->Current ()->nSegment;	
mine->CurrSeg ()->nIndex = 0;
i = 1;
h = j = 0;
for (nDist = 1; (j < segNum) && (h < i); nDist++) {
	for (h = i; j < h; j++) {
		segI = mine->Segments (segRef [j]);
		for (sideNum = 0; sideNum < 6; sideNum++) {
			c = segI->children [sideNum];
			if (c < 0) 
				continue;
			segJ = mine->Segments (c);
			if (segJ->nIndex != -1)
				continue;
			segJ->nIndex = nDist;
			segRef [i++] = c;
			}
		}
	}
delete segRef;
}

//----------------------------------------------------------------------------
// DrawWireFrame
//----------------------------------------------------------------------------

void CMineView::DrawWireFrame (CMine *mine, bool bPartial)
{
	INT32			nSegment;
	CSegment	*segP;

CalcSegDist (mine);
m_pDC->SelectObject(m_penGray);
for (nSegment=0, segP = mine->Segments ();nSegment<mine->SegCount ();nSegment++, segP++) {
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

void CMineView::DrawTextureMappedCubes (CMine *mine)
{
	UINT32 nSegment;
	INT16	 iVertex;
	INT32	 z, zMax;
	CSegment *segP;

	// Get shading table data
UINT8* light_index = 0;
if (m_viewMineFlags & eViewMineShading && (light_index = PalettePtr ()))
	light_index += 256*5; // skip 3-byte palette + 1st 2 light tables

// Draw Segments ()
for (nSegment = 0, segP = mine->Segments (); nSegment < mine->SegCount (); nSegment++, segP++) {
	for (iVertex = 0, zMax = LONG_MIN; iVertex < MAX_VERTICES_PER_SEGMENT; iVertex++)
		if (zMax < (z = m_viewPoints [segP->verts [iVertex]].z))
			zMax = z;
	szo [nSegment].iSeg = nSegment;
	szo [nSegment].zMax = zMax;
	}
QSortCubes (0, mine->SegCount () - 1);
CalcSegDist (mine);
for (nSegment = 0; nSegment < mine->SegCount (); nSegment++) {
	segP = mine->Segments (szo [nSegment].iSeg);
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

void CMineView::DrawCube (CMine *mine, INT16 nSegment,INT16 nSide, INT16 linenum, INT16 pointnum, INT16 clear_it) 
{
	CSegment *segP = mine->Segments (nSegment);
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (!Visible (segP))
		return;

	// clear segment and point
	if (clear_it) {
		m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
		m_pDC->SelectObject (GetStockObject(BLACK_PEN)); // BLACK
		INT32 nVert = segP->verts [side_vert [nSide] [pointnum]];
		if (IN_RANGE (m_viewPoints [nVert].x,x_max) &&
			 IN_RANGE (m_viewPoints [nVert].y,y_max)) {
			m_pDC->Ellipse(m_viewPoints [nVert].x - 4,
			m_viewPoints [nVert].y - 4,
			m_viewPoints [nVert].x + 4,
			m_viewPoints [nVert].y + 4);
			}
		if (segP->wallFlags & MARKED_MASK) {
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
		if (segP->wallFlags & MARKED_MASK)
			m_pDC->SelectObject (m_penHiCyan);
		else if (nSegment == mine->Current ()->nSegment)
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
		if (nSegment == mine->Current ()->nSegment)
			if (SelectMode (eSelectSide)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);        // RED
			else
				m_pDC->SelectObject (m_penHiGreen); // GREEN
		else
			m_pDC->SelectObject (m_penHiDkGreen);         // DARK_GREEN
		if (IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [0]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [0]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [1]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [1]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [2]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [2]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [3]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [3]]].y,y_max))
			 DrawLine (segP,side_vert [nSide] [0],side_vert [nSide] [1]);
			 DrawLine (segP,side_vert [nSide] [1],side_vert [nSide] [2]);
			 DrawLine (segP,side_vert [nSide] [2],side_vert [nSide] [3]);
			 DrawLine (segP,side_vert [nSide] [3],side_vert [nSide] [0]);

		// draw current line
		// must draw in same order as segment to avoid leftover pixels on screen
		if (nSegment == mine->Current ()->nSegment)
			if (SelectMode (eSelectLine)) // && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed);  // RED
			else 
				m_pDC->SelectObject (m_penHiCyan);  // BLUE/CYAN
		else
			m_pDC->SelectObject (m_penDkCyan);  // BLUE/CYAN
		if (IN_RANGE(m_viewPoints [segP->verts [line_vert [side_line [nSide] [linenum]] [0]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [line_vert [side_line [nSide] [linenum]] [0]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [line_vert [side_line [nSide] [linenum]] [1]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [line_vert [side_line [nSide] [linenum]] [1]]].y,y_max))
			DrawLine (segP,
						 line_vert [side_line [nSide] [linenum]] [0],
						 line_vert [side_line [nSide] [linenum]] [1]);
		}

	// draw a circle around the current point
	if (!clear_it) {
		m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
		if (nSegment == mine->Current ()->nSegment)
			if (SelectMode (eSelectPoint)) //  && edit_mode != EDIT_OFF) {
				m_pDC->SelectObject (m_penHiRed); // RED
			else
				m_pDC->SelectObject (m_penHiCyan); // CYAN
		else
			m_pDC->SelectObject (m_penHiDkCyan); // CYAN
		if (IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].y,y_max))
			m_pDC->Ellipse(m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].x - 4,
								m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].y - 4,
								m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].x + 4,
								m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].y + 4);
		}
}

//--------------------------------------------------------------------------
//			 draw_partial_segment()
//--------------------------------------------------------------------------

void CMineView::DrawCubePartial (CSegment *segP) {
  INT16 line;
  INT16 vert0,vert1;

if (!Visible (segP))
	return;
for (line=0;line<12;line++) {
	if (segP->map_bitmask & (1<<line)) {
      if (IN_RANGE(m_viewPoints [segP->verts [line_vert [line] [0]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [line_vert [line] [0]]].y,y_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [line_vert [line] [1]]].x,x_max) &&
			 IN_RANGE(m_viewPoints [segP->verts [line_vert [line] [1]]].y,y_max)) {
			vert0 = line_vert [line] [0];
			vert1 = line_vert [line] [1];
			if (vert1>vert0) {
				m_pDC->MoveTo (m_viewPoints [segP->verts [vert0]].x, m_viewPoints [segP->verts [vert0]].y);
				m_pDC->LineTo (m_viewPoints [segP->verts [vert1]].x, m_viewPoints [segP->verts [vert1]].y);
				}
			else {
				m_pDC->MoveTo (m_viewPoints [segP->verts [vert1]].x, m_viewPoints [segP->verts [vert1]].y);
				m_pDC->LineTo (m_viewPoints [segP->verts [vert0]].x, m_viewPoints [segP->verts [vert0]].y);
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



void CMineView::DrawCubeQuick	(CSegment *segP, bool bPartial)
{
if (!GetMine ())
	return;
if (!Visible (segP))
	return;

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;
	INT32	chSegI, chSideI, chVertI, i, j, commonVerts;
	CSegment	*child;
	INT16 *pv = segP->verts;

for (i = 0; i < 8; i++, pv++) {
	INT32	v = *pv;
	if (!(IN_RANGE (m_viewPoints [v].x, x_max) &&
			IN_RANGE (m_viewPoints [v].y, y_max)))
		return;
	}
if (bPartial) {
	UINT32 nSide;
	for (nSide=0; nSide<6; nSide++) {
		if (segP->children [nSide] >= 0)
			continue;
		
		POINT	side [4], line [2], vert;
		for (i = 0; i < 4; i++) {
			side [i].x = m_viewPoints [segP->verts [side_vert [nSide] [i]]].x; 
			side [i].y = m_viewPoints [segP->verts [side_vert [nSide] [i]]].y; 
			}
		CFixVector a,b;
		a.x = side [1].x - side [0].x;
		a.y = side [1].y - side [0].y;
		b.x = side [3].x - side [0].x;
		b.y = side [3].y - side [0].y;
		if (a.x*b.y < a.y*b.x)
			m_pDC->SelectObject((HPEN)GetStockObject(WHITE_PEN));
		else
			m_pDC->SelectObject(m_penGray);
		// draw each line of the current side separately
		// only draw if there is no child cube of the current cube with a common side
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 2; j++)
				line [j] = side [(i+j)%4];

			// check child cubes
			commonVerts = 0;
			for (chSegI = 0; (chSegI < 6) && (commonVerts < 2); chSegI++) {
				if (segP->children [chSegI] < 0)
					continue;
				child = m_mine->Segments (segP->children [chSegI]);
				// walk through child cube's sides
				commonVerts = 0;
				for (chSideI = 0; (chSideI < 6) && (commonVerts < 2); chSideI++) {
					// check current child cube side for common line
					// i.e. check each line for common vertices with the parent line
					for (commonVerts = 0, chVertI = 0; (chVertI < 4) && (commonVerts < 2); chVertI++) {
						vert.x = m_viewPoints [child->verts [side_vert [chSideI] [chVertI]]].x;
						vert.y = m_viewPoints [child->verts [side_vert [chSideI] [chVertI]]].y;
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
		v = segP->verts [k];
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

void CMineView::DrawLine (CDTexture *pTx, POINT pt0, POINT pt1, UINT8 color) 
{
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
	pTx->m_pDataBM [y*pTx->m_width+x] = color;
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
		pTx->m_pDataBM [y*pTx->m_width+x] = color;
		nStep += dy;
		if (nStep >= dx) {
			y += yInc;
			nStep -= dx;
			}
		}
	}
else {
	for (i = dy + 1; i; i--, y += yInc) {
		pTx->m_pDataBM [y*pTx->m_width+x] = color;
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
			pTx->m_pDataBM [y*pTx->m_width+x] = color;
	else
		for (y=pt0.y;y>=pt1.y;y--)
			pTx->m_pDataBM [y*pTx->m_width+x] = color;
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

void CMineView::DrawAnimDirArrows (INT16 texture1, CDTexture *pTx)
{
	INT32 sx,sy;
	INT32 bScroll = GetMine ()->ScrollSpeed (texture1, &sx, &sy);

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

	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (IN_RANGE(m_viewPoints [segP->verts [0]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [0]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->verts [1]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [1]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->verts [2]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [2]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->verts [3]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [3]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->verts [4]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [4]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->verts [5]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [5]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->verts [6]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [6]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->verts [7]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [7]].y,y_max)   )
	{

		INT32 resolution = 0;
		CDTexture tx (bmBuf);
		UINT8 *pm_viewPointsMem = (UINT8 *)m_pvBits;
		UINT16 width = m_viewWidth;
		UINT16 height = m_viewHeight;
		UINT16 rowOffset = (m_viewWidth + 3) & ~3;
		UINT16 nSide = 5;
		CWall *pWall;
		UINT16 nWall = NO_WALL (m_mine);

		for (nSide=0; nSide<6; nSide++) {
			pWall = ((nWall = segP->sides [nSide].nWall) == NO_WALL (m_mine)) ? NULL : ((CDlcDoc*) GetDocument ())->GetMine ()->Walls () + nWall;
			if ((segP->children [nSide] == -1) ||
				(pWall && (pWall->type != WALL_OPEN) && ((pWall->type != WALL_CLOAKED) || pWall->cloak_value))
				)
			{
				APOINT& p0 = m_viewPoints [segP->verts [side_vert [nSide] [0]]];
				APOINT& p1 = m_viewPoints [segP->verts [side_vert [nSide] [1]]];
				APOINT& p3 = m_viewPoints [segP->verts [side_vert [nSide] [3]]];

				CFixVector a,b;
				a.x = p1.x - p0.x;
				a.y = p1.y - p0.y;
				b.x = p3.x - p0.x;
				b.y = p3.y - p0.y;
				if (a.x*b.y > a.y*b.x) {
					INT16 texture1 = segP->sides [nSide].nBaseTex;
					INT16 texture2 = segP->sides [nSide].nOvlTex;
					if (!DefineTexture (texture1, texture2, &tx, 0, 0)) {
						DrawAnimDirArrows (texture1, &tx);
						TextureMap (resolution, segP, nSide, tx.m_pDataBM, tx.m_width, tx.m_height, 
									   light_index, pm_viewPointsMem, m_viewPoints, width, height, rowOffset);
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
//                        draw_segment_points()
//--------------------------------------------------------------------------

void CMineView::DrawCubePoints (CSegment *segP)
{
	INT16		*pv = segP->verts;
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

void CMineView::DrawMarkedCubes (CMine *mine, INT16 clear_it) 
{
	CSegment	*segP;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;
	INT16 i;

	// draw marked/special Segments () and Walls ()
	if (!clear_it) {
		for (i = 0; i < mine->SegCount (); i++) {
			segP = mine->Segments () + i;
			if (segP->wallFlags & MARKED_MASK) {
				m_pDC->SelectObject (SelectMode (eSelectBlock) ? m_penRed: m_penCyan);
				DrawCubeQuick (segP);
				}
			else {
				//    if (show_special) {
				if (ViewFlag (eViewMineSpecial) && !(m_viewOption == eViewTextureMapped) ) {
					switch(segP->function) {
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
						if (segP->props & SEGMENT_PROP_WATER)
							m_pDC->SelectObject (m_penMedBlue);
						else if (segP->props & SEGMENT_PROP_LAVA)
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
	for (i=0;i<mine->VertCount ();i++)
		if (mine->VertStatus (i) & MARKED_MASK)
			if (IN_RANGE(m_viewPoints [i].x,x_max) && IN_RANGE(m_viewPoints [i].y,y_max))
				m_pDC->Rectangle(m_viewPoints [i].x - 4, m_viewPoints [i].y - 4, m_viewPoints [i].x + 4, m_viewPoints [i].y + 4);
}

//--------------------------------------------------------------------------
// DrawCurrentCube()
//--------------------------------------------------------------------------

void CMineView::DrawCurrentCube(CSegment *segP, bool bPartial)
{
	INT16 nSide = m_Current->nSide;
	INT16 linenum = m_Current->nPoint;
	INT16 pointnum = m_Current->nPoint;

	if (segP->wallFlags & MARKED_MASK) {
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

	if (IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [0]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [0]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [1]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [1]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [2]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [2]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [3]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [3]]].y,y_max)) {

		DrawLine(segP, side_vert [nSide] [0], side_vert [nSide] [1]);
		DrawLine(segP, side_vert [nSide] [1], side_vert [nSide] [2]);
		DrawLine(segP, side_vert [nSide] [2], side_vert [nSide] [3]);
		DrawLine(segP, side_vert [nSide] [3], side_vert [nSide] [0]);
	}

	// draw current line
	// must draw in same order as segment to avoid leftover pixels on screen
	if (m_selectMode == LINE_MODE) { // && edit_mode != EDIT_OFF) {
		m_pDC->SelectObject(m_penRed);  // RED
	} else {
		m_pDC->SelectObject(m_penCyan);  // BLUE/CYAN
	}

	if (IN_RANGE(m_viewPoints [segP->verts [line_vert [side_line [nSide] [linenum]] [0]]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [line_vert [side_line [nSide] [linenum]] [0]]].y,y_max) &&
		IN_RANGE(m_viewPoints [segP->verts [line_vert [side_line [nSide] [linenum]] [1]]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [line_vert [side_line [nSide] [linenum]] [1]]].y,y_max)   ) {

		DrawLine(segP, line_vert [side_line [nSide] [linenum]] [0],
			line_vert [side_line [nSide] [linenum]] [1]);
	}

	// draw a circle around the current point
	m_pDC->SelectObject((HBRUSH)GetStockObject(NULL_BRUSH));
	if (m_selectMode == POINT_MODE) { //  && edit_mode != EDIT_OFF) {
		m_pDC->SelectObject(m_penRed); // RED
	} else {
		m_pDC->SelectObject(m_penCyan); // CYAN
	}

	if (IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].x,x_max) &&
		IN_RANGE(m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].y,y_max)     ) {

		m_pDC->Ellipse(m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].x - 4,
			m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].y - 4,
			m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].x + 4,
			m_viewPoints [segP->verts [side_vert [nSide] [pointnum]]].y + 4);
	}
}

//--------------------------------------------------------------------------
// DrawLine ()
//
// Action - draws a line starting with lowest vert
//--------------------------------------------------------------------------

void CMineView::DrawLine(CSegment *segP,INT16 vert1,INT16 vert2) 
{
	if (vert2 > vert1) {
		m_pDC->MoveTo(m_viewPoints [segP->verts [vert1]].x, m_viewPoints [segP->verts [vert1]].y);
		m_pDC->LineTo(m_viewPoints [segP->verts [vert2]].x, m_viewPoints [segP->verts [vert2]].y);
	} else {
		m_pDC->MoveTo(m_viewPoints [segP->verts [vert2]].x, m_viewPoints [segP->verts [vert2]].y);
		m_pDC->LineTo(m_viewPoints [segP->verts [vert1]].x, m_viewPoints [segP->verts [vert1]].y);
	}
}
//--------------------------------------------------------------------------
// DrawWalls()
//--------------------------------------------------------------------------

void CMineView::DrawWalls(CMine *mine) 
{
	CWall		*walls = mine->Walls ();
	CSegment	*segments = mine->Segments ();
	CFixVector	*vertices = mine->Vertices ();
	CSegment	*segP;
	INT16 i,j;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

for (i=0;i<mine->GameInfo ().walls.count;i++) {
	if (walls [i].m_nSegment > mine->SegCount ())
		continue;
	segP = segments + (INT32)walls [i].m_nSegment;
	if (!Visible (segP))
		continue;
	switch(walls [i].type) {
		case WALL_NORMAL:
			m_pDC->SelectObject(m_penLtGray);
			break;
		case WALL_BLASTABLE:
			m_pDC->SelectObject(m_penLtGray);
			break;
		case WALL_DOOR:
			switch(walls [i].keys) {
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
	if (IN_RANGE(m_viewPoints [segP->verts [side_vert [j][0]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [j][0]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [j][1]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [j][1]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [j][2]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [j][2]]].y,y_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [j][3]]].x,x_max) &&
		 IN_RANGE(m_viewPoints [segP->verts [side_vert [j][3]]].y,y_max)) {

			CFixVector center,orthog,vector;
			APOINT point;

		mine->CalcCenter (center, walls [i].m_nSegment, walls [i].m_nSide);
		mine->CalcOrthoVector(orthog, walls [i].m_nSegment, walls [i].m_nSide);
		vector.x = center.x - orthog.x;
		vector.y = center.y - orthog.y;
		vector.z = center.z - orthog.z;
		m_matrix.SetPoint(&vector,&point);
		for (j=0;j<4;j++) {
			m_pDC->MoveTo(point.x,point.y);
			m_pDC->LineTo(m_viewPoints [segP->verts [side_vert [walls [i].m_nSide] [j]]].x,
			m_viewPoints [segP->verts [side_vert [walls [i].m_nSide] [j]]].y);
			}
		if (walls [i].nTrigger != NO_TRIGGER) {
				APOINT arrowstart_point,arrowend_point,arrow1_point,arrow2_point;
				CFixVector fin;

			// calculate arrow points
			vector.x = center.x - 3*orthog.x;
			vector.y = center.y - 3*orthog.y;
			vector.z = center.z - 3*orthog.z;
			m_matrix.SetPoint(&vector,&arrowstart_point);
			vector.x = center.x + 3*orthog.x;
			vector.y = center.y + 3*orthog.y;
			vector.z = center.z + 3*orthog.z;
			m_matrix.SetPoint(&vector,&arrowend_point);

			// direction toward center of line 0 from center
			UINT8 *svp = &side_vert [walls [i].m_nSide][0];
			vector.x  = vertices [segP->verts [svp [0]]].x;
			vector.x += vertices [segP->verts [svp [1]]].x;
			vector.x /= 2;
			vector.y  = vertices [segP->verts [svp [0]]].y;
			vector.y += vertices [segP->verts [svp [1]]].y;
			vector.y /= 2;
			vector.z  = vertices [segP->verts [svp [0]]].z;
			vector.z += vertices [segP->verts [svp [1]]].z;
			vector.z /= 2;
			vector.x -= center.x;
			vector.y -= center.y;
			vector.z -= center.z;
			double length;
			length = my_sqrt((double)vector.x*(double)vector.x
								+ (double)vector.y*(double)vector.y
								+ (double)vector.z*(double)vector.z);
			length /= F1_0;
			if (length < 1.0) 
				length = 1.0;
			vector.x = (long) ((double) vector.x / length);
			vector.y = (long) ((double) vector.y / length);
			vector.z = (long) ((double) vector.z / length);

			fin.x = center.x + 2*orthog.x + vector.x;
			fin.y = center.y + 2*orthog.y + vector.y;
			fin.z = center.z + 2*orthog.z + vector.z;
			m_matrix.SetPoint(&fin,&arrow1_point);

			fin.x = center.x + 2*orthog.x - vector.x;
			fin.y = center.y + 2*orthog.y - vector.y;
			fin.z = center.z + 2*orthog.z - vector.z;
			m_matrix.SetPoint(&fin,&arrow2_point);

			// draw arrow
			m_pDC->MoveTo(arrowstart_point.x,arrowstart_point.y);
			m_pDC->LineTo(arrowend_point.x,arrowend_point.y);
			m_pDC->LineTo(arrow1_point.x,arrow1_point.y);
			m_pDC->MoveTo(arrowend_point.x,arrowend_point.y);
			m_pDC->LineTo(arrow2_point.x,arrow2_point.y);
			}
		}
	}
}
//--------------------------------------------------------------------------
//			  draw_lights()
//--------------------------------------------------------------------------

void CMineView::DrawLights (CMine *mine) 
{
  INT16 i;

  if (!m_pDC) return;

#if 0
  INT16 j, k;
  // first show lights that blow up (delta lights)
  SelectObject(m_pDC, m_penLtGray);

  if (IsD2File ()) {
    LightDeltaIndex () = (CLightDeltaIndex *)GlobalLock(hDLIndex ());
    if (LightDeltaIndex ()) {
      LightDeltaValues () = (LightDeltaValues () *)GlobalLock(hDeltaLights ());
      if (LightDeltaValues ()) {
	for (i=0;i<GameInfo ().lightDeltaIndices.count;i++) {
	  nSide = LightDeltaIndex () [i].nSide;
	  nSegment  = LightDeltaIndex () [i].nSegment;
	  if (!Visible (mine->Segments (nSegment))
		  continue;
	  draw_octagon(m_pDC,nSide,nSegment);
	  if (nSegment == current->segment && nSide == current->side) {
	    POINT light_source;
	    light_source = segment_center_xy(nSide,nSegment);
	    for (j=0;j<LightDeltaIndex () [i].count;j++) {
	      POINT light_dest;
	      INT32 index = LightDeltaIndex () [i].index+j;
	      nSide = LightDeltaValues () [index].nSide;
			nSegment  = LightDeltaValues () [index].nSegment;
	      segment *segP = Segments () [nSegment];
	      light_dest = segment_center_xy(nSide,nSegment);
			for (k=0;k<4;k++)  {
				POINT corner;
				UINT8 l = LightDeltaValues () [index].vert_light [k];
				l = min(0x1f,l);
				l <<= 3;
				m_pen m_penLight = CreatePen(PS_SOLID, 1, RGB(l,l,255-l));
				SelectObject(m_pDC, m_penLight);
				corner.x = m_viewPoints [segP->verts [side_vert [nSide] [k]]].x;
				corner.y = m_viewPoints [segP->verts [side_vert [nSide] [k]]].y;
				corner.x = (corner.x + light_dest.x)>>1;
				corner.y = (corner.y + light_dest.y)>>1;
				MoveTo(m_pDC,light_source.x,light_source.y);
				LineTo(m_pDC,corner.x,corner.y);
				SelectObject(m_pDC, m_penLtGray);
				DeleteObject(m_penLight);
	      }
	    }
	  }
	}
	GlobalUnlock(hDeltaLights ());
	  }
      GlobalUnlock(hDLIndex ());
    }
  }
#endif
  // now show flickering lights
  m_pDC->SelectObject(m_penYellow);

  // find flickering light from
CFlickeringLight* pfl = mine->FlickeringLights ();
for (i = 0; i < mine->FlickerLightCount (); i++, pfl++)
	if (Visible (mine->Segments (pfl->m_nSegment)))
	   DrawOctagon(mine, pfl->m_nSide, pfl->m_nSegment);
}

//------------------------------------------------------------------------
// DrawOctagon()
//------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMineView::DrawOctagon(CMine *mine, INT16 nSide, INT16 nSegment) 
{
	CSegment *segP;
	INT16 j;
	INT16 x_max = m_viewWidth * 2;
	INT16 y_max = m_viewHeight * 2;

	if (nSegment >=0 && nSegment <=mine->SegCount () && nSide>=0 && nSide<=5 ) {
	  POINT corners [4],center,line_centers [4],diamond [4],fortyfive [4];
	  segP = mine->Segments () + nSegment;
	  for (j=0;j<4;j++) {
	    corners [j].x = m_viewPoints [segP->verts [side_vert [nSide] [j]]].x;
	    corners [j].y = m_viewPoints [segP->verts [side_vert [nSide] [j]]].y;
	  }
	  if (IN_RANGE(corners [0].x,x_max) && IN_RANGE(corners [0].y,y_max) &&
		   IN_RANGE(corners [1].x,x_max) && IN_RANGE(corners [1].y,y_max) &&
	      IN_RANGE(corners [2].x,x_max) && IN_RANGE(corners [2].y,y_max) &&
	      IN_RANGE(corners [3].x,x_max) && IN_RANGE(corners [3].y,y_max)   ) {

	    center.x = (corners [0].x + corners [1].x + corners [2].x + corners [3].x)>>2;
	    center.y = (corners [0].y + corners [1].y + corners [2].y + corners [3].y)>>2;
		for (j=0;j<4;j++) {
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
	    for (j=0;j<4;j++) {
	      m_pDC->LineTo(fortyfive [j].x,fortyfive [j].y);
		  m_pDC->LineTo(diamond [j].x,diamond [j].y);
	    }
	  }
	}
}

//----------------------------------------------------------------------------
// draw_spline()
//----------------------------------------------------------------------------

void CMineView::DrawSpline (CMine *mine) 
{
	INT32 h, i, j;

//  SelectObject(hdc, hrgnAll);
m_pDC->SelectObject (m_penRed);
m_pDC->SelectObject ((HBRUSH)GetStockObject(NULL_BRUSH));
mine->CalcSpline ();
APOINT point;
m_matrix.SetPoint (&points[1],&point);
if (IN_RANGE(point.x,x_max) && IN_RANGE(point.y,y_max)){
	m_matrix.SetPoint (&points[0],&point);
	if (IN_RANGE(point.x,x_max) && IN_RANGE(point.y,y_max)){
		m_pDC->MoveTo (point.x,point.y);
		m_matrix.SetPoint (&points[1],&point);
		m_pDC->LineTo (point.x,point.y);
		m_pDC->Ellipse (point.x - 4,point.y - 4,point.x+4, point.y+4);
		}
	}
m_matrix.SetPoint (&points[2],&point);
if (IN_RANGE(point.x,x_max) && IN_RANGE(point.y,y_max)){
	m_matrix.SetPoint (&points[3],&point);
	if (IN_RANGE(point.x,x_max) && IN_RANGE(point.y,y_max)){
		m_pDC->MoveTo (point.x,point.y);
		m_matrix.SetPoint (&points[2],&point);
		m_pDC->LineTo (point.x,point.y);
		m_pDC->Ellipse (point.x - 4,point.y - 4,point.x+4, point.y+4);
		}
	}
m_pDC->SelectObject (m_penBlue);
j = MAX_VERTICES (mine);
for (h = n_splines * 4, i = 0; i < h; i++, j--)
	m_matrix.SetPoint (mine->Vertices (--j), m_viewPoints + j);
CSegment *segP = mine->Segments (MAX_SEGMENTS (mine) - 1);
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

void TransformModelPoint (CFixVector& dest, APOINT &src, CFixMatrix &orient, CFixVector offs)
{
dest.x = (orient.rvec.x * (FIX)src.x + orient.uvec.x * (FIX)src.y + orient.fvec.x * (FIX)src.z);
dest.y = (orient.rvec.y * (FIX)src.x +	orient.uvec.y * (FIX)src.y + orient.fvec.y * (FIX)src.z);
dest.z = (orient.rvec.z * (FIX)src.x + orient.uvec.z * (FIX)src.y + orient.fvec.z * (FIX)src.z);
dest.x += offs.x;
dest.y += offs.y;
dest.z += offs.z;
}


void CMineView::DrawObject(CMine *mine,INT16 objnum,INT16 clear_it) 
{
	INT16 poly;
	CGameObject *objP;
	CFixVector pt [MAX_POLY];
	APOINT poly_draw [MAX_POLY];
	APOINT object_shape [MAX_POLY] = {
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
if (objnum >=0 && objnum < mine->GameInfo ().objects.count) {
	objP = mine->Objects (objnum);
	if (!Visible (mine->Segments (objP->nSegment)))
		return;
	}
else {
	// secret return
	objP = &temp_obj;
	objP->type = -1;
	// mine->secret_orient = Objects () [0]->orient;
	objP->orient.rvec.x = -mine->SecretOrient ().rvec.x;
	objP->orient.rvec.y = -mine->SecretOrient ().rvec.y;
	objP->orient.rvec.z = -mine->SecretOrient ().rvec.z;
	objP->orient.uvec.x =  mine->SecretOrient ().fvec.x;
	objP->orient.uvec.y =  mine->SecretOrient ().fvec.y;
	objP->orient.uvec.z =  mine->SecretOrient ().fvec.z;
	objP->orient.fvec.x =  mine->SecretOrient ().uvec.x;
	objP->orient.fvec.y =  mine->SecretOrient ().uvec.y;
	objP->orient.fvec.z =  mine->SecretOrient ().uvec.z;
	// objP->orient =  mine->secret_orient;
	UINT16 nSegment = (UINT16)mine->SecretCubeNum ();
	if (nSegment >= mine->SegCount ())
		nSegment = 0;
	if (!Visible (mine->Segments (nSegment)))
		return;
	mine->CalcSegCenter(objP->pos,nSegment); // define objP->position
	}

switch (clear_it) {
	case 0: // normal
	case 1: // gray
		if (m_selectMode == OBJECT_MODE && objnum == mine->Current ()->nObject) 
			m_pDC->SelectObject(m_penRed); // RED
		else {
			switch(objP->type) {
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
	::TransformModelPoint (pt [poly], object_shape [poly], objP->orient, objP->pos);
	m_matrix.SetPoint (pt + poly, poly_draw + poly);
	}

// figure out world coordinates

INT32 i;
for (i = 0; i < 6; i++)
	if (!(IN_RANGE (poly_draw [i].x, x_max) &&
			IN_RANGE (poly_draw [i].y, y_max)))
		return;

if ((theApp.IsD2File ()) &&
	 (objnum == mine->Current ()->nObject) &&
	 (objP->type != OBJ_CAMBOT) && (objP->type != OBJ_MONSTERBALL) && 
	 (objP->type != OBJ_EXPLOSION) && (objP->type != OBJ_SMOKE) && (objP->type != OBJ_EFFECT) &&
	 (objP->render_type == RT_POLYOBJ) &&
	 !SetupModel(mine,objP)) {
	if (clear_it)
		m_pDC->SelectObject(GetStockObject(BLACK_PEN));
	m_pDC->SelectObject((HBRUSH)GetStockObject(BLACK_BRUSH));
	DrawModel();
	}
else {
	m_pDC->MoveTo (poly_draw [0].x,poly_draw [0].y);
	for (poly = 0; poly < 6; poly++)
		m_pDC->LineTo (poly_draw [poly].x, poly_draw [poly].y);
	if (objnum == mine->Current ()->nObject) {
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
if ((objnum == mine->Current ()->nObject) || (objnum == mine->Other ()->nObject)) {
	CPen     pen, *pOldPen;
	INT32		d;

	pt [0] =
	pt [1] =
	pt [2] = objP->pos;
	pt [1].x -= objP->size;
	pt [2].x += objP->size;
	m_matrix.SetPoint (pt, poly_draw);
	m_matrix.Push ();
	m_matrix.Unrotate ();
	m_matrix.SetPoint (pt + 1, poly_draw + 1);
	m_matrix.SetPoint (pt + 2, poly_draw + 2);
	m_matrix.Pop ();
	d = (poly_draw [2].x - poly_draw [1].x);
	if (d < 24)
		d = 24;
	pen.CreatePen (PS_SOLID, 2, (objnum == mine->Current ()->nObject) ? RGB (255,0,0) : RGB (255,208,0));
	pOldPen = m_pDC->SelectObject (&pen);
	m_pDC->SelectObject ((HBRUSH)GetStockObject(HOLLOW_BRUSH));
	m_pDC->Ellipse (poly_draw [0].x - d, poly_draw [0].y - d, poly_draw [0].x + d, poly_draw [0].y + d);
	m_pDC->SelectObject (pOldPen);
	}
}

//--------------------------------------------------------------------------
//			  DrawObjects()
//--------------------------------------------------------------------------

bool CMineView::ViewObject (CGameObject *objP)
{
switch(objP->type) {
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
		switch (powerup_types [objP->id]) {
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

void CMineView::HiliteTarget (CMine *mine)
{
	INT32 i, nTarget;

CGameObject *objP = mine->CurrObj ();
if ((objP->type != OBJ_EFFECT) || (objP->id != LIGHTNING_ID))
	return;
mine->Other ()->nObject = mine->Current ()->nObject;
if (nTarget = objP->rType.lightningInfo.nTarget)
	for (i = 0, objP = mine->Objects (); i < mine->GameInfo ().objects.count; i++, objP++)
		if ((objP->type == OBJ_EFFECT) && (objP->id == LIGHTNING_ID) && (objP->rType.lightningInfo.nId == nTarget)) {
			mine->Other ()->nObject = i;
			break;
			return;
			}
}
 
//--------------------------------------------------------------------------
//			  DrawObjects()
//--------------------------------------------------------------------------

void CMineView::DrawObjects (CMine *mine, INT16 clear_it) 
{
if (!ViewObject ())
	return;

INT32 i, j;
if (theApp.IsD2File ()) {
	// see if there is a secret exit trigger
	for(i = 0; i < mine->GameInfo ().triggers.count; i++)
	if (mine->Triggers (i)->type == TT_SECRET_EXIT) {
		DrawObject (mine, (INT16)mine->GameInfo ().objects.count, 0);
		break; // only draw one secret exit
		}
	}
HiliteTarget (mine);
CGameObject *objP = mine->Objects ();
for (i = mine->GameInfo ().objects.count, j = 0; i; i--, j++, objP++)
	if (ViewObject (objP))
		DrawObject (mine, j, 0);
}

//--------------------------------------------------------------------------
//			  draw_highlight()
//--------------------------------------------------------------------------

void CMineView::DrawHighlight(CMine *mine, INT16 clear_it) 
{
	INT16	currSide, currPoint;
//	INT16 i;
//	RECT rect;

if (mine->SegCount ()==0) 
	return;

// draw Objects ()
if (!clear_it) {
	DrawObjects (mine, clear_it);
//	if (/*!(preferences & PREFS_HIDE_MARKED_BLOCKS) ||*/ SelectMode (eSelectBlock))
		DrawMarkedCubes(mine, clear_it);
  }

// draw highlighted Segments () (other first, then current)
if (mine->Current () == &mine->Current1 ()) {
	if (mine->Current1 ().nSegment != mine->Current2 ().nSegment)
		DrawCube (mine, mine->Current2 ().nSegment, mine->Current2 ().nSide, mine->Current2 ().nLine, mine->Current2 ().nPoint,clear_it);
	DrawCube (mine, mine->Current1 ().nSegment, mine->Current1 ().nSide, mine->Current1 ().nLine, mine->Current1 ().nPoint,clear_it);
	}
else {
	if (mine->Current1 ().nSegment != mine->Current2 ().nSegment)
		DrawCube (mine, mine->Current1 ().nSegment, mine->Current1 ().nSide, mine->Current1 ().nLine, mine->Current1 ().nPoint,clear_it);
	DrawCube (mine, mine->Current2 ().nSegment, mine->Current2 ().nSide, mine->Current2 ().nLine, mine->Current2 ().nPoint,clear_it);
	}

// draw Walls ()
if (ViewFlag (eViewMineWalls))
	DrawWalls(mine);

// draw lights
if (ViewFlag (eViewMineLights))
	  DrawLights (mine);

// draw spline
if (mine->m_bSplineActive)
	DrawSpline (mine);

*message = '\0';
if (preferences & PREFS_SHOW_POINT_COORDINATES) {
   strcat_s (message, sizeof (message), "  point (x,y,z): (");
   INT16 vertex = mine->Segments () [mine->Current ()->nSegment].verts [side_vert [mine->Current ()->nSide] [mine->Current ()->nPoint]];
	char	szCoord [20];
	sprintf_s (szCoord, sizeof (szCoord), "%1.4f,%1.4f,%1.4f)", mine->Vertices (vertex)->x, mine->Vertices (vertex)->y, mine->Vertices (vertex)->z);
	strcat_s (message, sizeof (message), szCoord);
	}
else {
   // calculate cube size (length between center point of opposing sides)
	strcat_s (message, sizeof (message), "  cube size: ");
	CFixVector center1,center2;
   double length;
   mine->CalcCenter (center1, mine->Current ()->nSegment,0);
	mine->CalcCenter (center2, mine->Current ()->nSegment,2);
   length = mine->CalcLength(&center1,&center2) / F1_0;
	sprintf_s (message + strlen (message), sizeof (message) - strlen (message), "%.1f", (double) length);
	strcat_s (message, sizeof (message), " x ");
   mine->CalcCenter (center1, mine->Current ()->nSegment,1);
   mine->CalcCenter (center2, mine->Current ()->nSegment,3);
	length = mine->CalcLength(&center1,&center2) / F1_0;
   sprintf_s (message + strlen (message), sizeof (message) - strlen (message), "%.1f", (double) length);
	strcat_s (message, sizeof (message), " x ");
   mine->CalcCenter (center1, mine->Current ()->nSegment,4);
   mine->CalcCenter (center2, mine->Current ()->nSegment,5);
   length = mine->CalcLength(&center1,&center2) / F1_0;
	sprintf_s (message + strlen (message), sizeof (message) - strlen (message), "%.1f", (double) length);
	}
strcat_s (message, sizeof (message), ",  cube:");
_itoa_s (mine->Current ()->nSegment, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " side:");
_itoa_s ((currSide = mine->Current ()->nSide) + 1, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " point:");
_itoa_s (currPoint = mine->Current ()->nPoint, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " vertex:");
_itoa_s (mine->CurrSeg ()->verts [side_vert [currSide][currPoint]], message + strlen (message), sizeof (message) - strlen (message), 10);

strcat_s (message, sizeof (message), ",  textures:");
strcat_s (message, sizeof (message), " 1st:");
_itoa_s (mine->CurrSide ()->nBaseTex, message + strlen (message), sizeof (message) - strlen (message), 10);
strcat_s (message, sizeof (message), " 2nd:");
_itoa_s (mine->CurrSide ()->nOvlTex & 0x3fff, message + strlen (message), sizeof (message) - strlen (message), 10);

strcat_s (message, sizeof (message), ",  zoom:");
double zoom_factor = log (10 * m_sizex) / log (1.2);
if (zoom_factor > 0) 
	zoom_factor += 0.5;
else
	zoom_factor -= 0.5;
sprintf_s (message + strlen (message), sizeof (message) - strlen (message),  "%1.2f", zoom_factor);
STATUSMSG (message);
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

for (zoom = log(10*m_sizex), i = 0; i < nSteps; i++) {
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
//	m_sizex *= zoom;
//	m_sizey *= zoom;
	m_sizez *= zoom;
	m_matrix.Scale (1.0 / zoom);
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
if (!(m_xRenderOffs && m_yRenderOffs && GetMine ()))
	return;

	APOINT *a = m_viewPoints;

INT32 i;
for (i = m_mine->VertCount (); i; i--, a++) {
	a->x += m_xRenderOffs;
	a->y += m_yRenderOffs;
	}
}

                        /*--------------------------*/

void CMineView::SetViewPoints (CRect *pRC, bool bSetViewInfo)
{
if (!GetMine ())
	return;

	CRect		rc (LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	long		i, 
				x, y, z,
				minX = LONG_MAX, minY = LONG_MAX, minZ = LONG_MAX,
				maxX = LONG_MIN, maxY = LONG_MIN, maxZ = LONG_MIN;

#if 0 //OGL_RENDERING
m_matrix.Calculate (0,0,0); // let OpenGL do the translation
#else
m_matrix.Calculate (m_movex, m_movey, m_movez);
#endif
InitViewDimensions ();
if (bSetViewInfo)
	m_matrix.SetViewInfo (m_depthPerception, m_viewWidth, m_viewHeight);
i = m_mine->VertCount ();
APOINT *a = m_viewPoints + i;
CVertex* verts = m_mine->Vertices (i);
for (; i--; ) {
	m_matrix.SetPoint (--verts, --a);
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
for (i = m_mine->VertCount (), a = m_viewPoints; i--; a++) {
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
//if (!GetMine ())
//	return;

	CRect			rc (LONG_MAX, LONG_MAX, -LONG_MAX, -LONG_MAX);
	double		zoomX, zoomY, zoom;
	INT32			dx, dy;

DelayRefresh (true);
//CenterMine ();
//SetViewPoints (&rc);
m_movex = 0.0;
m_movey = 0.0;
m_movez = 0.0;
m_matrix.SetViewInfo (10000, m_viewWidth, m_viewHeight);
SetViewPoints (&rc, false);
CRect	crc;
GetClientRect (crc);
crc.InflateRect (-4, -4);
zoomX = (double) crc.Width () / (double) rc.Width ();
zoomY = (double) crc.Height () / (double) rc.Height ();
zoom = (zoomX < zoomY) ? zoomX: zoomY;
Zoom (1, zoom);
for (;;) {
	m_matrix.SetViewInfo (depth_perception, m_viewWidth, m_viewHeight);
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
sprintf_s (message, sizeof (message), "ROTATE (%1.2f�)", glAngle [i]);
INFOMSG (message);
glRotated (glAngle [i], glRotMat [i][0], glRotMat [i][1], glRotMat [i][2]);
#else
m_matrix.Rotate (direction, 2 * angle); // * ((double) move_rate / 0x10000L));
a += 2 * angle;// * PI;
if (a < -360)
	a += 360;
else if (a > 360)
	a -= 360;
sprintf_s (message, sizeof (message), "ROTATE (%1.2f�)", a);
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
	i = 0;
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
m_movex -= (double) value * m_matrix.IM [1][++i];  /* move view point */
m_movey -= (double) value * m_matrix.IM [2][i];
m_movez -= (double) value * m_matrix.IM [3][i];
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
//	CDlcDoc* pDoc = GetDocument();
//	ASSERT_VALID(pDoc);

	CFixVector *verts;
	if (!GetMine ())
		return;

	INT32 maxx = 0;
	INT32 minx = 0;
	INT32 maxy = 0;
	INT32 miny = 0;
	INT32 maxz = 0;
	INT32 minz = 0;

	INT32 i;
	verts = m_mine->Vertices ();
	for (i=0;i<m_mine->VertCount ();i++, verts++) {
		maxx = max(maxx, verts->x);
		minx = min(minx, verts->x);
		maxy = max(maxy, verts->y);
		miny = min(miny, verts->y);
		maxz = max(maxz, verts->z);
		minz = min(minz, verts->z);
	}
	INT16 max_x = (INT16)(maxx / F1_0);
	INT16 max_y = (INT16)(maxy / F1_0);
	INT16 max_z = (INT16)(maxz / F1_0);
	INT16 min_x = (INT16)(minx / F1_0);
	INT16 min_y = (INT16)(miny / F1_0);
	INT16 min_z = (INT16)(minz / F1_0);

	m_spinx = M_PI/4.f;
	m_spiny = M_PI/4.f;
	m_spinz = 0.0;
	m_movex = -(max_x + min_x)/2.f;
	m_movey = -(max_y + min_y)/2.f;
	m_movez = -(max_z + min_z)/2.f;
	INT32 factor;
	INT32 max_all = max(max(max_x-min_x, max_y-min_y), max_z-min_z)/20;
	if (max_all < 2)      factor = 14;
	else if (max_all < 4) factor = 10;
	else if (max_all < 8) factor = 8;
	else if (max_all < 12) factor = 5;
	else if (max_all < 16) factor = 3;
	else if (max_all < 32) factor = 2;
	else factor = 1;
	m_sizex = .1f * (double)pow(1.2,factor);
	m_sizey = m_sizex;
	m_sizez = m_sizex;
	m_matrix.Set(
		m_movex, m_movey, m_movez,
		m_sizex, m_sizey, m_sizez,
		m_spinx, m_spiny, m_spinz);

Refresh (false);
}

                        /*--------------------------*/

void CMineView::CenterCube()
{
if (!GetMine ())
	return;
	CSegment& segP = m_mine->Segments () [m_Current->nSegment];
	CFixVector *vMine = m_mine->Vertices ();
	INT16 *vSeg = segP.verts;

	m_movex = -((double)vMine [segP.verts [0]].x
		 +(double)vMine [vSeg [1]].x
		 +(double)vMine [vSeg [2]].x
		 +(double)vMine [vSeg [3]].x
		 +(double)vMine [vSeg [4]].x
		 +(double)vMine [vSeg [5]].x
		 +(double)vMine [vSeg [6]].x
		 +(double)vMine [vSeg [7]].x)/(double)0x80000L;
	m_movey = -((double)vMine [vSeg [0]].y
		 +(double)vMine [vSeg [1]].y
		 +(double)vMine [vSeg [2]].y
		 +(double)vMine [vSeg [3]].y
		 +(double)vMine [vSeg [4]].y
		 +(double)vMine [vSeg [5]].y
		 +(double)vMine [vSeg [6]].y
		 +(double)vMine [vSeg [7]].y)/(double)0x80000L;
	m_movez = -((double)vMine [vSeg [0]].z
		 +(double)vMine [vSeg [1]].z
		 +(double)vMine [vSeg [2]].z
		 +(double)vMine [vSeg [3]].z
		 +(double)vMine [vSeg [4]].z
		 +(double)vMine [vSeg [5]].z
		 +(double)vMine [vSeg [6]].z
		 +(double)vMine [vSeg [7]].z)/(double)0x80000L;

Refresh (false);
}

                        /*--------------------------*/

void CMineView::CenterObject()
{
	CDlcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;
	CMine *mine = pDoc->GetMine ();
	if (!mine) return;

	CGameObject& objP = mine->Objects () [m_Current->nObject];
	m_movex = (INT16)(-(objP.pos.x)/0x10000L);
	m_movey = (INT16)(-(objP.pos.y)/0x10000L);
	m_movez = (INT16)(-(objP.pos.z)/0x10000L);

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
GetMine ()->SetSelectMode ((INT16) mode);
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
				GetMine ();
				INT32 v = m_mine->CurrVert ();
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
for (i = m_mine->SegCount (), segP = m_mine->Segments (); i; i--, segP++)
	for (j = 0; j < MAX_VERTICES_PER_SEGMENT; j++)
		if ((segP->verts [j] == v) && Visible (segP))
			return true;
return false;
}

                        /*--------------------------*/

void CMineView::MarkRubberBandedVertices (void)
{
if (!GetMine ())
	return;

	APOINT	*pa = m_viewPoints;
	INT32		x, y;

INT32 i;
for (i = 0; i < m_mine->VertCount (); i++, pa++) {
	x = pa->x;
	y = pa->y;
	if (BETWEEN (m_clickPos.x, x, m_releasePos.x) &&
		 BETWEEN (m_clickPos.y, y, m_releasePos.y) &&
		 VertexVisible (i)) {
		if (m_clickState & MK_SHIFT)
			m_mine->VertStatus (i) &= ~MARKED_MASK;
		else
			m_mine->VertStatus (i) |= MARKED_MASK;
		m_bUpdate = true;
		}
	}
if (m_bUpdate) {
	m_mine->UpdateMarkedCubes ();
	Refresh ();
	}
}

//==========================================================================
//==========================================================================
void CMineView::RefreshObject(INT16 old_object, INT16 new_object) 
{
if (!GetMine ())
	return;

m_mine->Current ()->nObject = new_object;
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
if (!GetMine ())
	return;

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
	for(i=0;i<(INT16)m_mine->GameInfo ().triggers.count;i++)
		if (m_mine->Triggers (i)->type ==TT_SECRET_EXIT) {
			enable_secret = TRUE;
			break;
			}

for (i=0;i<=m_mine->GameInfo ().objects.count;i++) {
	BOOL drawable = FALSE;
	// define temp object type and position for secret object selection
	if (i == m_mine->GameInfo ().objects.count && theApp.IsD2File () && enable_secret) {
		objP = &temp_obj;
		objP->type = OBJ_PLAYER;
		// define objP->position
		CalcSegmentCenter(objP->pos, (UINT16)m_mine->SecretCubeNum ());
		}
	else
		objP = m_mine->Objects (i);
#if 0
	switch(objP->type) {
		case OBJ_WEAPON:
			if (ViewObject (eViewObjectsPowerups | eViewObjectsWeapons)) {
				drawable = TRUE;
				}
		case OBJ_POWERUP:
			if (ViewObject (powerup_types [objP->id])) {
				drawable = TRUE;
				}
			break;
		default:
			if(ViewObject (1<<objP->type))
				drawable = TRUE;
		}
	if (drawable) 
#else
	if (ViewObject (objP))
#endif
		{
		// translate object's position to screen coordinates
		m_matrix.SetPoint(&objP->pos, &pt);
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
i = m_mine->Current ()->nObject;
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
if (!GetMine ())
	return false;

  CSegment		*segP;
  CRect			rc;
//  extern INT16 xMouse,yMouse;
  INT16			cur_segment, next_segment;
  INT16			i, j;
  INT32				x, y;
  APOINT			sideVerts [4], mousePos;
  bool			bFound = false;

/* find next segment which is within the cursor position */
GetClientRect (rc);
next_segment = cur_segment = m_mine->Current ()->nSegment;
mousePos.x = (INT16) xMouse;
mousePos.y = (INT16) yMouse;
mousePos.z = 0;
do {
	wrap (&next_segment, direction, 0, m_mine->SegCount () - 1); /* point to next segment */
	segP = m_mine->Segments (next_segment);
	if (!Visible (segP))
		continue;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++)
			sideVerts [j] = m_viewPoints [segP->verts [side_vert [i][j]]];
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
		x = m_viewPoints [segP->verts [i]].x;
		if ((x < rc.left) || (x > rc.right)) {
			bOnScreen = false;
			break;
			}
		y = m_viewPoints [segP->verts [i]].y;
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
m_mine->Current ()->nSegment = next_segment;
theApp.ToolView ()->Refresh ();
Refresh ();
return true;
}

//-------------------------------------------------------------------------
// calculate_segment_center()
//-------------------------------------------------------------------------

void CMineView::CalcSegmentCenter(CFixVector& pos,INT16 nSegment) 
{
if (!GetMine ())
	return;

CSegment *segP = m_mine->Segments () + nSegment;
CFixVector *vMine = m_mine->Vertices ();
INT16 *vSeg = segP->verts;
pos.x  =
   (vMine [vSeg [0]].x
   +vMine [vSeg [1]].x
   +vMine [vSeg [2]].x
   +vMine [vSeg [3]].x
   +vMine [vSeg [4]].x
   +vMine [vSeg [5]].x
   +vMine [vSeg [6]].x
   +vMine [vSeg [7]].x)/8;
pos.y  =
   (vMine [vSeg [0]].y
   +vMine [vSeg [1]].y
   +vMine [vSeg [2]].y
   +vMine [vSeg [3]].y
   +vMine [vSeg [4]].y
   +vMine [vSeg [5]].y
   +vMine [vSeg [6]].y
   +vMine [vSeg [7]].y)/8;
pos.z  =
   (vMine [vSeg [0]].z
   +vMine [vSeg [1]].z
   +vMine [vSeg [2]].z
   +vMine [vSeg [3]].z
   +vMine [vSeg [4]].z
   +vMine [vSeg [5]].z
   +vMine [vSeg [6]].z
   +vMine [vSeg [7]].z)/8;
}

                        /*--------------------------*/
                        
BOOL CMineView::DrawRubberBox ()
{
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
ReleaseCapture ();
InvalidateRect (&m_rubberRect, FALSE);
UpdateWindow ();
m_rubberRect.left = m_rubberRect.right =
m_rubberRect.top = m_rubberRect.bottom = 0;
}

                        /*--------------------------*/
                        
BOOL CMineView::UpdateDragPos ()
{
if (!GetMine ())
	return FALSE;
if ((m_mouseState != eMouseStateInitDrag) && (m_mouseState != eMouseStateDrag))
	return FALSE;

	INT16 nVert = side_vert [m_mine->Current ()->nSide] [m_mine->Current ()->nPoint];
	INT16 v = m_mine->CurrSeg ()->verts [nVert];
	INT16 x = m_viewPoints [v].x;
	INT16 y = m_viewPoints [v].y;

if (m_mouseState == eMouseStateInitDrag) {
	SetMouseState (eMouseStateDrag);
// SetCapture ();
	m_highlightPos.x = x;
	m_highlightPos.y = y;
	m_lastDragPos = m_highlightPos;
	}
HighlightDrag (m_mine, nVert, x, y);
	
//InvalidateRect (NULL, TRUE);
return TRUE;
}

                        /*--------------------------*/
                        
void CMineView::HighlightDrag (CMine *mine, INT16 nVert, long x, long y) 
{
m_pDC->SelectObject((HBRUSH) GetStockObject (NULL_BRUSH));
//m_pDC->SetROP2 (R2_NOT);
m_pDC->SetROP2 (R2_NOT);
m_pDC->Ellipse (x-4, y-4, x+4, y+4);

CRect	rc (x, y, x, y);
INT32 i;
for (i = 0; i < 3; i++) {
	m_pDC->MoveTo (x, y);
	INT16 nVert2 = connect_points [nVert] [i];
	INT16 x2 = m_viewPoints [mine->CurrSeg ()->verts [nVert2]].x;
	INT16 y2 = m_viewPoints [mine->CurrSeg ()->verts [nVert2]].y;
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
if (!GetMine ())
	return FALSE;
if (m_mouseState != eMouseStateDrag)
	return FALSE;
if (m_lastMousePos == m_lastDragPos)
	return FALSE;

	INT16 nVert;
	INT16 x, y;
	INT32 i;

nVert = side_vert [m_mine->Current ()->nSide] [m_mine->Current ()->nPoint];

// unhighlight last point and lines drawing
HighlightDrag (m_mine, nVert, m_lastDragPos.x, m_lastDragPos.y);

// highlight the new position
HighlightDrag (m_mine, nVert, m_lastMousePos.x, m_lastMousePos.y);
m_lastDragPos = m_lastMousePos;

m_pDC->SetROP2 (R2_NOT);
for (i = 0; i < m_mine->VertCount (); i++) {
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
if ((i >= m_mine->VertCount ()) && (m_highlightPos.x != -1))
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
//ReleaseCapture ();
if (!GetMine ())
	return;

	INT32		m_changesMade = 1;
	INT32		i, new_vert, count = 0;
	long		xPos,yPos;
	INT16		xPoint,yPoint;
	INT16		point1,vert1;
	INT16		point2,vert2;

xPos = m_releasePos.x;
yPos = m_releasePos.y;
point1 = side_vert [m_mine->Current ()->nSide] [m_mine->Current ()->nPoint];
vert1 = m_mine->Segments () [m_mine->Current ()->nSegment].verts [point1];
// find point to merge with
for (i = 0; i < m_mine->VertCount (); i++) {
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
			vert2 = m_mine->Segments () [m_mine->Current ()->nSegment].verts [i];
			if (new_vert==vert2) {
				ErrorMsg ("Cannot drop point onto another corner of the current cube.");
				break;
				}
			}
		}
	if (i==8 && new_vert!=vert1) {
	// make sure the new line lengths are close enough
		for (i=0;i<3;i++) {
			point2 = connect_points [point1] [i];
			vert2 = m_mine->Segments () [m_mine->Current ()->nSegment].verts [point2];
			if (m_mine->CalcLength (m_mine->Vertices (new_vert), m_mine->Vertices (vert2)) >= 1000.0*(double)F1_0) {
				ErrorMsg ("Cannot move this point so far away.");
				break;
				}
			}
		if (i==3) { //
			// replace origional vertex with new vertex
			m_mine->Segments () [m_mine->Current ()->nSegment].verts [point1] = new_vert;
			// all unused vertices
			m_mine->DeleteUnusedVertices();
			m_mine->FixChildren();
			m_mine->SetLinesToDraw();
			}
		}	
	}
else {
	// no vertex found, just drop point along screen axii
	APOINT apoint;
	apoint.x = (INT16) xPos;
	apoint.y = (INT16) yPos;
	apoint.z = m_viewPoints [vert1].z;
	m_matrix.UnsetPoint(m_mine->Vertices (vert1), &apoint);
	}
Refresh ();
}


//==========================================================================
// MENU - NextPoint
//==========================================================================
void CMineView::NextPoint(INT32 dir) 
{
if (!GetMine ())
	return;

//if (!mine->SplineActive ())
//	DrawHighlight (mine, 1);
//if (m_selectMode==POINT_MODE)
wrap(&m_mine->Current ()->nPoint,dir,0,4-1);
m_mine->Current ()->nLine = m_mine->Current ()->nPoint;
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
if (!GetMine ())
	return;

wrap(&m_mine->Current ()->nSide,dir,0,6-1);
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
if (!GetMine ())
	return;

wrap(&m_mine->Current ()->nSide,dir,0,6-1);
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
if (!GetMine ())
	return;

wrap (&m_mine->Current ()->nLine, dir, 0, 4-1);
m_mine->Current ()->nPoint = m_mine->Current ()->nLine;
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
if (!GetMine ())
	return;

if (m_mine->SegCount () <= 0)
	return;

if (0) {//!ViewOption (eViewPartialLines)) {
	DrawHighlight (m_mine, 1);
	//if (m_selectMode == CUBE_MODE)
		wrap (&m_mine->Current ()->nSegment,dir,0, m_mine->SegCount () - 1);
	Refresh (true);
	//SetSelectMode (CUBE_MODE);
	DrawHighlight (m_mine, 0);
	}
else {
	//if (m_selectMode == CUBE_MODE)
		wrap (&m_mine->Current ()->nSegment, dir, 0, m_mine->SegCount () - 1);
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
if (!GetMine ())
	return;

	CSegment *segP,*childseg;
	INT16 child,nSide;
	bool bFwd = (dir == 1);

DrawHighlight (m_mine, 1);
segP = m_mine->Segments () + m_mine->Current ()->nSegment;
child = segP->children [bFwd ? m_mine->Current ()->nSide: opp_side [m_mine->Current ()->nSide]];
if (child <= -1) {
	// first try to find a non backwards route
	for (nSide=0;nSide<6;nSide++) {
		if (segP->children [nSide] != m_lastSegment && segP->children [nSide] > -1) {
			child = segP->children [nSide];
			m_mine->Current ()->nSide =  bFwd ? nSide: opp_side [nSide];
			break;
			}
		}
	// then settle for any way out
	if (nSide == 6) {
		for (nSide=0;nSide<6;nSide++) {
			if (segP->children [nSide] > -1) {
				child = segP->children [nSide];
				m_mine->Current ()->nSide = bFwd ? nSide: opp_side [nSide];
				break;
				}
			}			
		}
	}
if (child > -1) {
	childseg = m_mine->Segments () + child;
// try to select side which is in same direction as current side
	for (nSide=0;nSide<6;nSide++) {
		if (childseg->children [nSide] == m_mine->Current ()->nSegment) {
			m_mine->Current ()->nSide =  bFwd ? opp_side [nSide]: nSide;
			break;
			}
		}
	m_lastSegment = m_mine->Current ()->nSegment;
	if (0) {//!ViewOption (eViewPartialLines)) {
		// DrawHighlight (m_mine, 1);
		m_mine->Current ()->nSegment = child;
		// DrawHighlight (m_mine, 0);
		} 
	else {
		m_mine->Current ()->nSegment = child;
		Refresh (true);
		}
	}
DrawHighlight (m_mine, 0);
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
if (!GetMine ())
	return;

m_mine->Current () = (m_mine->Current () == &m_mine->Current1 ()) ? &m_mine->Current2 (): &m_mine->Current1 ();
Refresh (true);
//theApp.ToolView ()->CubeTool ()->Refresh ();
}



bool CMineView::SelectOtherSide () 
{
if (!GetMine ())
	return false;

INT16 nOppSeg, nOppSide;

if (!m_mine->GetOppositeSide (nOppSeg, nOppSide))
	return false;

m_mine->Current ()->nSegment = nOppSeg;
m_mine->Current ()->nSide = nOppSide;
Refresh (true);
//theApp.ToolView ()->CubeTool ()->Refresh ();
return true;
}

//==========================================================================
// MENU - NextObject
//==========================================================================

void CMineView::NextObject (INT32 dir) 
{
if (!GetMine ())
	return;

  INT16 old_object = m_mine->Current ()->nObject;
  INT16 new_object = m_mine->Current ()->nObject;

//  DrawHighlight (m_mine, 1);
if (m_mine->GameInfo ().objects.count > 1) {
//	if (m_selectMode == OBJECT_MODE)
		wrap(&new_object,dir,0, (INT16)m_mine->GameInfo ().objects.count - 1) ;
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

#if OGL_RENDERING

// code from NeHe productions tutorials

                        /*--------------------------*/

BOOL CMineView::GLInit (GLvoid)
{
glShadeModel (GL_SMOOTH);
glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
glClearDepth (1.0f);
glEnable (GL_DEPTH_TEST);
glDepthFunc (GL_LEQUAL);
glEnable (GL_ALPHA_TEST);
glAlphaFunc (GL_GEQUAL, 0.5);	
glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
MEMSET (glHandles, 0, sizeof (glHandles));		
GLFitToView ();
#if OGL_ORTHO == 0
SetViewPoints ();
glMinZ = m_minViewPoint.z;
glMaxZ = m_maxViewPoint.z;
double dx = abs (m_maxViewPoint.x - m_minViewPoint.x);
double dy = abs (m_maxViewPoint.y - m_minViewPoint.y);
double z = m_viewPoints [m_minVPIdx.x].z;
if (z > m_viewPoints [m_maxVPIdx.x].z)
	z = m_viewPoints [m_maxVPIdx.x].z;
dx = dx / 2 + z;
CRect rc;
GetClientRect (rc);
double aspect = (double) rc.Width () / (double) rc.Height ();
if (aspect > 0)
	aspect = 1.0 / aspect;
z = m_viewPoints [m_minVPIdx.y].z;
if (z > m_viewPoints [m_maxVPIdx.y].z)
	z = m_viewPoints [m_maxVPIdx.y].z;
dy = dy / 2 + z;
dx *= aspect;
//dy *= (double) rc.Width () / (double) rc.Height ();
double dd = (dx > dy) ? dx: dy;
if (dd > glMinZ)
	glMinZ = -dd;
#endif
glInit = true;
return TRUE;
}

                        /*--------------------------*/

BOOL CMineView::GLInitPalette (GLvoid)
{
return glPalette || (glPalette = PalettePtr ());
}

                        /*--------------------------*/

static double glCX = 2.0, glCY = 2.0, glCZ = 2.0;

GLvoid CMineView::GLFitToView (GLvoid)
{
FitToView ();
#if 1
	SetViewPoints ();
	glCX = m_maxViewPoint.x - m_minViewPoint.x;
	glCY = m_maxViewPoint.y - m_minViewPoint.y;
	glCZ = m_maxViewPoint.z - m_minViewPoint.z;
# if 1
	CRect rc;
	GetClientRect (rc);
	double aspect = (double) rc.Width () / (double) rc.Height ();
	if (aspect > 0)
		glCX *= aspect;
	else
		glCY *= aspect;
# else
	if (glCX < glCY)
		glCX = glCY;
	else
		glCX = glCY;
# endif
#else
CRect	rc;
SetViewPoints (&rc);

INT32 rad = rc.Width ();
if (rad < rc.Height ())
	rad = rc.Height ();
if (rad < (m_maxViewPoint.z - m_minViewPoint.z + 1))
	rad = (m_maxViewPoint.z - m_minViewPoint.z + 1);
INT32 xMid = (rc.left + rc.right) / 2;
INT32 yMid = (rc.bottom + rc.top) / 2;
double left = (double) xMid - rad;
double right = (double) xMid + rad;
double bottom = (double) yMid - rad;
double top = (double) yMid + rad;
GetClientRect (rc);
double aspect = (double) rc.Width () / (double) rc.Height ();
if (aspect < 1.0) {
	bottom /= aspect;
	top /= aspect;
	}
else {
	left /= aspect;
	right /= aspect;
	}
glMatrixMode (GL_PROJECTION);
glLoadIdentity ();
//glOrtho (left, right, bottom, top, 1.0, 1.0 + rad);
glMatrixMode (GL_MODELVIEW);
glLoadIdentity ();
gluLookAt (0.0, 0.0, 2.0 * rad, xMid, yMid, (m_maxViewPoint.z + m_minViewPoint.z) / 2, 0.0, 1.0, 0.0);
#endif
glFitToView = TRUE;
}

                        /*--------------------------*/

BOOL CMineView::GLResizeScene (GLvoid) 
{
if (!GLCreateWindow ())
	return FALSE;

CRect rc;
	
GetClientRect (rc);
//ClientToScreen (rc);
if (!(rc.Width () && rc.Height ()))
	return FALSE;
glViewport (rc.left, rc.top, rc.Width (), rc.Height ());
/*
if (glFitToView)
	glFitToView = FALSE;
else 
*/
	{
//	glMatrixMode (GL_PROJECTION);
//	glLoadIdentity ();
	//glOrtho (-0.5, 0.5, -0.5, 0.5, -1, 1);
	//gluPerspective (90.0f, (GLfloat) rc.Width () / (GLfloat) rc.Height (), 1.0f, 10000.0f);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
#if OGL_ORTHO
	glOrtho (-glCX / 2, glCX / 2, glCY / 2, -glCY / 2, -glCZ / 2, glCZ / 2);
#else
	//glOrtho (-1, 1, -1, 1, -1, 1);
	//glOrtho (0, 1, 0, 1, 0, 1);
	gluPerspective (90.0f, (GLfloat) rc.Width () / (GLfloat) rc.Height (), 1.0f, 10000.0f);
#endif
	//glOrtho (-1.0 / glCX, 1.0 / glCX, -1.0 / glCY, 1.0 / glCY, -1.0, 1.0);
	//glOrtho (0, rc.Width (), 0, rc.Height (), -1, 1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	}
/*
glMatrixMode (GL_PROJECTION);
glLoadIdentity ();
glMatrixMode (GL_MODELVIEW);
glLoadIdentity ();
*/
return TRUE;
}

                        /*--------------------------*/

void CMineView::GLCreateTexture (INT16 nTexture)
{
if (!GLInitPalette ())
	return;
nTexture &= 0x1FFF; 
if (!glHandles [nTexture]) {
	CDTexture tx (bmBuf);
	DefineTexture (nTexture, 0, &tx, 0, 0);
	DrawAnimDirArrows (nTexture, &tx);
	// create RGBA bitmap from source bitmap
	INT32 h, i, j;
	for (h = i = 0; i < 64*64; i++) {
		j = bmBuf [i];
		if (j < 254) {
			j *= 3;
			rgbBuf [h++] = glPalette [j++] << 2;
			rgbBuf [h++] = glPalette [j++] << 2;
			rgbBuf [h++] = glPalette [j++] << 2;
			rgbBuf [h++] = 255;
			}
		else {
			rgbBuf [h++] = 0;
			rgbBuf [h++] = 0;
			rgbBuf [h++] = 0;
			rgbBuf [h++] = 0;
			}
		}
	glGenTextures (1, glHandles + nTexture); 
	glEnable (GL_TEXTURE_2D);
	glBindTexture (GL_TEXTURE_2D, glHandles [nTexture]); 
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbBuf);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
}

                        /*--------------------------*/

void CMineView::GLRenderTexture (INT16 nSegment, INT16 nSide, INT16 nTexture)
{
	CSegment *segP = m_mine->Segments (nSegment);
	CSide *sideP = segP->sides + nSide;
	CUVL *uvls;
	double l;
#if OGL_MAPPED
	APOINT *a;
#else
	CFixVector *verts = m_mine->Vertices ();
	CFixVector *v;
#endif
	static INT32 rotOffs [4] = {0,3,2,1};
	INT32 h = rotOffs [(nTexture & 0xC000) >> 14];
	INT32 j = h;
	bool bShaded = (m_viewMineFlags & eViewMineShading) != 0;

//CBRK (nSegment == 57 && nSide == 3);
GLCreateTexture (nTexture);
glEnable (GL_TEXTURE_2D);
glBindTexture (GL_TEXTURE_2D, glHandles [nTexture & 0x1FFF]); 
glBegin (GL_TRIANGLE_FAN);
INT32 i;
for (i = 0; i < 4; i++) {
	uvls = sideP->uvls + j;
	l = (bShaded ? uvls->l: F1_0) / UV_FACTOR;
	glColor3d (l,l,l);
	switch (h) {
		case 0:
			glTexCoord2d (uvls->u / 2048.0, uvls->v / 2048.0); 
			break;
		case 1:
			glTexCoord2d (uvls->v / 2048.0, uvls->u / 2048.0); 
			break;
		case 2:
			glTexCoord2d (-uvls->u / 2048.0, -uvls->v / 2048.0); 
			break;
		case 3:
			glTexCoord2d (-uvls->v / 2048.0, -uvls->u / 2048.0); 
			break;
		}
#if OGL_MAPPED
	a = m_viewPoints + segP->verts [side_vert [nSide][j]];
	glVertex3f ((double) a->x, (double) a->y, (double) a->z);
#else
	v = verts + segP->verts [side_vert [nSide][i]];
	glVertex3f (f2fl (v->x), f2fl (v->y), f2fl (v->z));
#endif
	j = (j + 1) % 4;
	}
glEnd ();
}

                        /*--------------------------*/

void CMineView::GLRenderFace (INT16 nSegment, INT16 nSide)
{
	CSegment *segP = m_mine->Segments (nSegment);
	CSide *sideP = segP->sides + nSide;
	CFixVector *verts = m_mine->Vertices ();
	UINT16 nWall = segP->sides [nSide].nWall;

if (sideP->nBaseTex < 0)
	return;
CWall *pWall = (nWall == NO_WALL) ? NULL: ((CDlcDoc*) GetDocument ())->m_mine->Walls (nWall);
if ((segP->children [nSide] > -1) &&
	 (!pWall || (pWall->type == WALL_OPEN) || ((pWall->type == WALL_CLOAKED) && !pWall->cloak_value)))
	return;
#if OGL_MAPPED
APOINT& p0 = m_viewPoints [segP->verts [side_vert [nSide] [0]]];
APOINT& p1 = m_viewPoints [segP->verts [side_vert [nSide] [1]]];
APOINT& p3 = m_viewPoints [segP->verts [side_vert [nSide] [3]]];

CFixVector a,b;
a.x = p1.x - p0.x;
a.y = p1.y - p0.y;
b.x = p3.x - p0.x;
b.y = p3.y - p0.y;
if (a.x*b.y > a.y*b.x)
	return;
#else
CFixVector *p0 = verts + segP->verts [side_vert [nSide] [0]];
CFixVector *p1 = verts + segP->verts [side_vert [nSide] [1]];
CFixVector *p3 = verts + segP->verts [side_vert [nSide] [3]];

CFixVector a,b;
a.x = p1->x - p0->x;
a.y = p1->y - p0->y;
b.x = p3->x - p0->x;
b.y = p3->y - p0->y;
if (a.x*b.y > a.y*b.x)
	return;
#endif
GLRenderTexture (nSegment, nSide, sideP->nBaseTex);
if (sideP->nOvlTex)
	GLRenderTexture (nSegment, nSide, sideP->nOvlTex);
}

                        /*--------------------------*/

BOOL CMineView::GLRenderScene (GLvoid)	
{
if (!GLResizeScene ())
	return FALSE;
//glDrawBuffer (GL_BACK);
glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//glLoadIdentity ();
#if 1
if (glInit) {
	glInit = false;
	//m_matrix.Rotate ('Y', -(PI/2));
	GLFitToView ();
	}
CRect rc;
GetClientRect (rc);
double cx = (double) rc.Width () / 2;
double cy = (double) rc.Height () / 2;
glTranslated (-cx, -cy, glMinZ);
#else
glTranslated (glPan [0], glPan [1], glPan [2] -500);	
INT32 i;
for (i = 0; i < 3; i++)
	glRotated (glAngle [i], glRotMat [i][0], glRotMat [i][1], glRotMat [i][2]);
#endif
SetViewPoints ();
for (INT16 nSegment = m_mine->SegCount (); nSegment--; )
	for (INT16 nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++)
		GLRenderFace (nSegment, nSide);
return TRUE;
}

                        /*--------------------------*/

GLvoid CMineView::GLReset (GLvoid)
{
glDeleteTextures (910, glHandles);
MEMSET (glHandles, 0, sizeof (glHandles));
glPalette = NULL;
glInit = false;
}

                        /*--------------------------*/

GLvoid CMineView::GLKillWindow (GLvoid)
{
GLReset ();
if (m_glRC)	{
	if (!wglMakeCurrent (NULL,NULL))
		ErrorMsg ("OpenGL: Release of DC and RC failed.");
	if (!wglDeleteContext(m_glRC))
		ErrorMsg ("OpenGL: Release of rendering context failed.");
		m_glRC = NULL;
	}
if (m_glDC && !::ReleaseDC (m_hWnd, glHDC))	 {
	//ErrorMsg ("OpenGL: Release of device context failed.")
		;
	m_glDC=NULL;	
	}
#if 0
if (!UnregisterClass ("OpenGL",AfxGetInstance ())) {
	ErrorMsg ("OpenGL: Could Not Unregister Class.");
	}
#endif
}

                        /*--------------------------*/

BOOL CMineView::GLCreateWindow (CDC * pDC)
{
if (m_glDC)
	return TRUE;

	GLuint PixelFormat;

static PIXELFORMATDESCRIPTOR pfd = {
	sizeof (PIXELFORMATDESCRIPTOR),
	1,	
	PFD_DRAW_TO_WINDOW |	PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,
	32,
	0, 0, 0, 0, 0, 0,
	0,	
	0,
	0,
	0, 0, 0, 0,
	32,
	0,
	0,
	PFD_MAIN_PLANE,
	0,
	0, 0, 0
	};

if (!(m_glDC = GetDC ())) {
	GLKillWindow ();
	ErrorMsg ("OpenGL: Can't create device context.");
	return FALSE;
	}
glHDC = m_glDC->GetSafeHdc ();
if (!(PixelFormat = ChoosePixelFormat (glHDC, &pfd))) {
	GLKillWindow ();
	sprintf_s (message, sizeof (message), "OpenGL: Can't find a suitable pixel format. (%d)", GetLastError ());
	ErrorMsg (message);
	return FALSE;
	}
if(!SetPixelFormat(glHDC, PixelFormat, &pfd)) {
		GLKillWindow();
		sprintf_s (message, sizeof (message), "OpenGL: Can't set the pixel format (%d).", GetLastError ());
		ErrorMsg (message);
		return FALSE;
	}
if (!(m_glRC = wglCreateContext (glHDC))) {
	GLKillWindow ();
	sprintf_s (message, sizeof (message), "OpenGL: Can't create a rendering context (%d).", GetLastError ());
	ErrorMsg (message);
	return FALSE;
	}

if(!wglMakeCurrent (glHDC, m_glRC)) {
	GLKillWindow ();
	sprintf_s (message, sizeof (message), "OpenGL: Can't activate the rendering context (%d).", GetLastError ());
	ErrorMsg (message);
	return FALSE;
	}
if (!GLInit())	{
	GLKillWindow ();
	ErrorMsg ("OpenGL: Initialization failed.");
	return FALSE;
	}
return TRUE;
}

                        /*--------------------------*/

#endif //OGL_RENDERING

//eof