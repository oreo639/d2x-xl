// dlcView.cpp: implementation of the CMineView class
//

#include "stdafx.h"
//#include "winuser.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"

#include "PaletteManager.h"
#include "textures.h"
#include "global.h"
#include "FileManager.h"

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
for (int i = eMouseStateIdle; i < eMouseStateCount; i++)
	m_hCursors [i] = LoadCursor ((nIdCursors [i] == IDC_ARROW) ? null: DLE.m_hInstance, nIdCursors [i]);
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
m_glRC = null;
m_glDC = null;
#endif
m_nViewDist = 0;
m_depthPerception = 10000.0f;
m_depthBuffer = null;
m_overdrawFilter = null;
Reset ();
}

//------------------------------------------------------------------------------

void CMineView::Reset (void)
{
m_viewWidth = m_viewHeight = m_viewDepth = 0;	// force OnDraw to initialize these
tunnelMaker.Destroy ();
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

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

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

	int		dx = m_maxViewPoint.x - m_minViewPoint.x + 1;
	int		dy = m_minViewPoint.y - m_maxViewPoint.y + 1;
	double	r;

bool bHScroll = m_bHScroll;
bool bVScroll = m_bVScroll;
int xScrollCenter = m_xScrollCenter;
int yScrollCenter = m_yScrollCenter;
int xScrollRange = m_xScrollRange;
int yScrollRange = m_yScrollRange;
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
		int nPos = GetScrollPos (SB_HORZ);
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
		int nPos = GetScrollPos (SB_VERT);
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

//------------------------------------------------------------------------------

void CMineView::OnDraw (CDC* pViewDC)
{
CHECKMINE;

#if OGL_RENDERING
if (m_bUpdate) {
	GLRenderScene ();
	if (!SwapBuffers (glHDC)) {
		int err = GetLastError ();
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
		// draw the level
		switch(m_viewOption)	{
			case eViewTextureMapped:
				if (textureManager.Available ()) {
					DrawSegmentsTextured ();
					break;
					}
				// otherwise fall through
			case eViewAllLines:
				DrawWireFrame (false);
				break;

			case eViewHideLines:
				DrawWireFrame (false); 
				break;

			case eViewNearbyCubeLines:
				DrawWireFrame (false);
				break;

			case eViewPartialLines:
				DrawWireFrame (bPartial = true);
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

//------------------------------------------------------------------------------

void CMineView::AdvanceLightTick (void)
{
CHECKMINE;

	tLightTimer *ltP = lightManager.LightTimer (0);
	CVariableLight *flP = lightManager.VariableLight (0);
	int i, delay;

for (i = lightManager.Count (); i; i--, flP++, ltP++) {
	delay = (flP->m_info.delay * 100 /*+ F0_5*/) / F1_0;
	if (delay) {
		if (++ltP->ticks == delay) {
			ltP->ticks = 0;
			ltP->impulse = (ltP->impulse + 1) % 32;
			}
		}
	else
		ltP->impulse = (ltP->impulse + 1) % 32;
	}
}

//------------------------------------------------------------------------------

bool CMineView::SetLightStatus (void)
{
	int h, i, j;
	CLightDeltaIndex *ldiP = lightManager.LightDeltaIndex (0);
	CLightDeltaValue *ldvP;
	tLightTimer *ltP;
	CVariableLight *flP = lightManager.VariableLight (0);
	tLightStatus *pls;
	bool bChange = false;
	bool bD2XLights = (DLE.LevelVersion () >= 15) && (theMine->Info ().fileInfo.version >= 34);
	short nSrcSide, nSrcSeg, nSegment, nSide;

// search delta light index to see if current side has a light
pls = lightManager.LightStatus (0);
for (i = segmentManager.Count (); i; i--)
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++, pls++)
		pls->bWasOn = pls->bIsOn;
for (h = 0; h < lightManager.DeltaIndexCount (); h++, ldiP++) {
	nSrcSide = ldiP->m_nSegment;
	nSrcSeg = ldiP->m_nSide;
	j = lightManager.VariableLight (CSideKey (nSrcSide, nSrcSeg));
	if (j < 0)
		continue;	//shouldn't happen here, as there is a delta light value, but you never know ...
	if (j >= MAX_VARIABLE_LIGHTS)
		continue;	//shouldn't happen 
	ldvP = lightManager.LightDeltaValue (ldiP->m_info.index);
	for (i = ldiP->m_info.count; i; i--, ldvP++) {
		nSegment = ldvP->m_nSegment;
		nSide = ldvP->m_nSide;
		if (m_bShowLightSource) {
			if ((nSegment != nSrcSeg) || (nSide != nSrcSide)) 
				continue;
			if (0 > lightManager.VariableLight (CSideKey (nSegment, nSide)))
				continue;
			}
		else {
			if (((nSegment != nSrcSeg) || (nSide != nSrcSide)) && (0 <= lightManager.VariableLight (CSideKey (nSegment, nSide))))
				continue;
			}
		pls = lightManager.LightStatus (nSegment, nSide);
		ltP = lightManager.LightTimer (j);
		pls->bIsOn = (flP [j].m_info.mask & (1 << lightManager.LightTimer (j)->impulse)) != 0;
		if (pls->bWasOn != pls->bIsOn)
			bChange = true;
		}
	}
return bChange;
}

//------------------------------------------------------------------------------

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
			InvalidateRect (null, TRUE);
			UpdateWindow ();
			}
		}
	}
else 
	CView::OnTimer (nIdEvent);
}

								/*---------------------------*/

void CMineView::EnableDeltaShading (int bEnable, int nFrameRate, int bShowLightSource)
{
if (bEnableDeltaShading = bEnable) {
	m_lightTimer = SetTimer (3, (UINT) (m_nFrameRate + 5) / 10, null);
	if ((nFrameRate >= 10) && (nFrameRate <= 100))
		m_nFrameRate = nFrameRate;
	if (bShowLightSource != -1)
		m_bShowLightSource = bShowLightSource;
	lightManager.AnimInfo ().Clear ();
	}
else if (m_lightTimer != -1) {
	KillTimer (m_lightTimer);
	m_lightTimer = -1;
	}
}

								/*---------------------------*/

BOOL CMineView::SetWindowPos(const CWnd *pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
	CRect	rc;

	GetClientRect (rc);

if ((rc.Width () != cx) || (rc.Height () != cy))
	DLE.MainFrame ()->ResetPaneMode ();
return CView::SetWindowPos (pWndInsertAfter, x, y, cx, cy, nFlags);
}



bool CMineView::InitViewDimensions (void)
{
	CRect	rc;

GetClientRect (rc);
int width = (rc.Width () + 3) & ~3; // long word align
int height = rc.Height ();
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

	//int depth = m_pDC->GetDeviceCaps(BITSPIXEL) / 8;
	int depth = 3; // force 24-bit DIB
	if (InitViewDimensions() || (depth != m_viewDepth)) {
		m_bUpdate = true;
#if OGL_RENDERING == 0
		ResetView ();
#endif
	}
	m_viewDepth = depth;
#if OGL_RENDERING == 0
	if (UpdateScrollBars ()) {
		ResetView (!m_bUpdate);
		m_bUpdate = true;
		}
	if (m_DIB == 0) {
		if (m_DC.m_hDC == 0)
			m_DC.CreateCompatibleDC (pViewDC);
		if (m_DC.m_hDC) {
			BITMAPINFO bmi = {{sizeof (BITMAPINFOHEADER), m_viewWidth, -m_viewHeight, 1, m_viewDepth * 8, BI_RGB, 0, 0, 0, 0, 0}, {255,255,255,0}};
		m_DIB = ::CreateDIBSection (NULL, (BITMAPINFO *) &bmi, DIB_RGB_COLORS, (void**) &m_renderBuffer, null, 0);
		if (m_depthBuffer != null) 
			delete m_depthBuffer;
		if (m_overdrawFilter != null)
			delete m_overdrawFilter;
		m_depthBuffer = new depthType [m_viewWidth * m_viewHeight];
		}
	}
	// if DIB exists, then use our own DC instead of the View DC
if (m_DIB != 0) {
	m_pDC = &m_DC;
	m_DC.SelectObject (m_DIB);
	if (m_depthBuffer != null) {
		for (int i = m_viewWidth * m_viewHeight; i > 0; )
		m_depthBuffer [--i] = MAX_DEPTH;
		}
	}
#endif
}

//----------------------------------------------------------------------------
// ClearView()
//
// TODO: only clear the dirty area defined by the clip region
//----------------------------------------------------------------------------

void CMineView::ClearView (void)
{
// clear the dib or the view
#if OGL_RENDERING
glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
glLoadIdentity ();
#else
if (m_DIB)
	memset (m_renderBuffer, 0, m_viewWidth * m_viewHeight * m_viewDepth);
else {
	CRect rect;
	GetClientRect (rect);
	FillRect (m_pDC->m_hDC, rect, (HBRUSH) GetStockObject (BLACK_BRUSH));
	}
#endif
}

//--------------------------------------------------------------------------
//			  DrawObjects()
//--------------------------------------------------------------------------

bool CMineView::ViewObject (CGameObject *objP)
{
switch(objP->Type ()) {
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
		switch (powerupTypeTable [objP->Id ()]) {
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
	int i, nTarget;

CGameObject *objP = current->Object ();
if ((objP->Type () != OBJ_EFFECT) || (objP->Id () != LIGHTNING_ID))
	return;
other->m_nObject = current->m_nObject;
if (nTarget = objP->rType.lightningInfo.nTarget) {
	CGameObject* objP = objectManager.Object (0);
	for (i = objectManager.Count (); i; i--, objP++) {
		if ((objP->Type () == OBJ_EFFECT) && (objP->Id () == LIGHTNING_ID) && (objP->rType.lightningInfo.nId == nTarget)) {
			other->m_nObject = i;
			break;
			return;
			}
		}
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

//------------------------------------------------------------------------------

void CMineView::OnSize(UINT nType, int cx, int cy)
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
//if (DLE.MainFrame () && ((m_viewWidth != cx) || (m_viewHeight != cy)))
//	DLE.MainFrame ()->ResetPaneMode ();
CView::OnSize (nType, cx, cy);
m_bUpdate = true;
}

//------------------------------------------------------------------------------

void CMineView::SetViewOption(eViewOptions option)
{
m_viewOption = option;
Refresh ();
}

//------------------------------------------------------------------------------

void CMineView::ToggleViewMine (eMineViewFlags flag)
{
	m_viewMineFlags ^= flag;
	EnableDeltaShading ((m_viewMineFlags & eViewMineDeltaLights) != 0, -1, -1);
	Refresh ();
}

//------------------------------------------------------------------------------

void CMineView::ToggleViewObjects(eObjectViewFlags mask)
{
	m_viewObjectFlags ^= mask;
	Refresh ();
}

//------------------------------------------------------------------------------

void CMineView::SetViewMineFlags(uint mask)
{
	m_viewMineFlags = mask;
	Refresh ();
}

//------------------------------------------------------------------------------

void CMineView::SetViewObjectFlags(uint mask)
{
	m_viewObjectFlags = mask;
	Refresh ();
}

//------------------------------------------------------------------------------

void CMineView::SetSelectMode (uint mode)
{
theMine->SetSelectMode ((short) mode);
DLE.MainFrame ()->UpdateSelectButtons ((eSelectModes) mode);
m_selectMode = mode; 
Refresh (false);
}

//------------------------------------------------------------------------------

BOOL CMineView::OnSetCursor (CWnd* pWnd, UINT nHitTest, UINT message)
{
//if (m_bUpdateCursor) {
//::SetCursor (AfxGetApp()->LoadStandardCursor (nIdCursors [m_mouseState]));
//	return TRUE;
//	}
return CView::OnSetCursor (pWnd, nHitTest, message);
}

//------------------------------------------------------------------------------

BOOL CMineView::SetCursor (HCURSOR hCursor)
{
if (!hCursor) // || (hCursor == m_hCursor))
   return FALSE;
::SetClassLong (GetSafeHwnd (), -12 /*int (GCL_HCURSOR)*/, int (hCursor));
return TRUE;
}
                        
//------------------------------------------------------------------------------

void CMineView::SetMouseState (int newMouseState)
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

//------------------------------------------------------------------------------

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
				int v = vertexManager.Index (current->Vertex ());
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

//------------------------------------------------------------------------------

void CMineView::OnLButtonDown (UINT nFlags, CPoint point)
{
SetMouseState (eMouseStateButtonDown);
m_clickPos = point;
m_clickState = nFlags;
m_selectTimer = SetTimer (4, 500U, null);
CView::OnLButtonDown (nFlags, point);
}

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

void CMineView::OnRButtonDown (UINT nFlags, CPoint point)
{
SetMouseState (eMouseStateButtonDown);
m_clickPos = point;
m_clickState = nFlags;
CView::OnRButtonDown(nFlags, point);
}

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

void CMineView::Invalidate (BOOL bErase)
{
CWnd::Invalidate (bErase);
}

//------------------------------------------------------------------------------

void CMineView::InvalidateRect (LPCRECT lpRect, BOOL bErase)
{
CWnd::InvalidateRect (lpRect, bErase);
}

//------------------------------------------------------------------------------

void CMineView::Refresh (bool bAll)
{
CHECKMINE;

	static bool bRefreshing = false;

if (!(bRefreshing || m_nDelayRefresh)) {
	bRefreshing = true;
	InvalidateRect (null, TRUE);
//	SetFocus ();
	if (bAll && (m_mouseState == eMouseStateIdle)) {
		DLE.ToolView ()->Refresh ();
		DLE.TextureView ()->Refresh ();
//		UpdateWindow ();
		}
	m_bUpdate = true;
	bRefreshing = false;
	}
}

//------------------------------------------------------------------------------

void CMineView::OnUpdate (CView* pSender, LPARAM lHint, CGameObject* pHint)
{
//m_bUpdate = true;
//InvalidateRect(null);
Refresh ();
}

//------------------------------------------------------------------------------

bool CMineView::VertexVisible (int v)
{
	CSegment	*segP;

if (!m_nViewDist)
	return true;
for (CSegmentIterator si; si; si++) {
	segP = &(*si);
	for (short i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
		if ((segP->m_info.verts [i] == v) && Visible (segP))
			return true;
	}
return false;
}

//------------------------------------------------------------------------------

void CMineView::MarkRubberBandedVertices (void)
{
CHECKMINE;

	APOINT	*pa = m_viewPoints;
	int		x, y;

for (int i = 0, j = vertexManager.Count (); i < j; i++, pa++) {
	x = pa->x;
	y = pa->y;
	if (BETWEEN (m_clickPos.x, x, m_releasePos.x) &&
		 BETWEEN (m_clickPos.y, y, m_releasePos.y) &&
		 VertexVisible (i)) {
		if (m_clickState & MK_SHIFT)
			vertexManager.Status (i) &= ~MARKED_MASK;
		else
			vertexManager.Status (i) |= MARKED_MASK;
		m_bUpdate = true;
		}
	}
if (m_bUpdate) {
	segmentManager.UpdateMarked ();
	Refresh ();
	}
}

//==========================================================================
//==========================================================================

void CMineView::RefreshObject(short old_object, short new_object) 
{
current->m_nObject = new_object;
DLE.ToolView ()->Refresh ();
Refresh (false);
}

//-------------------------------------------------------------------------
// calculate_segment_center()
//-------------------------------------------------------------------------

void CMineView::CalcSegmentCenter (CVertex& pos, short nSegment)
{
CSegment *segP = segmentManager.Segment (nSegment);
CVertex *vMine = vertexManager.Vertex (0);
ushort *vSeg = segP->m_info.verts;
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

//------------------------------------------------------------------------------
                        
BOOL CMineView::DrawRubberBox ()
{
if (theMine == null) return FALSE;

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
                        
//------------------------------------------------------------------------------
                        
void CMineView::UpdateRubberRect (CPoint pt)
{
CHECKMINE;

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
                        
//------------------------------------------------------------------------------
                        
void CMineView::ResetRubberRect ()
{
CHECKMINE;

ReleaseCapture ();
InvalidateRect (&m_rubberRect, FALSE);
UpdateWindow ();
m_rubberRect.left = m_rubberRect.right =
m_rubberRect.top = m_rubberRect.bottom = 0;
}

								/*---------------------------*/

BOOL CMineView::OnMouseWheel (UINT nFlags, short zDelta, CPoint pt)
{
	CRect	rc;

GetWindowRect (rc);
if ((pt.x < rc.left) || (pt.x >= rc.right) || (pt.y < rc.top) || (pt.y >= rc.bottom))
	return DLE.TextureView ()->OnMouseWheel (nFlags, zDelta, pt);
if (zDelta > 0)
	ZoomIn (zDelta / WHEEL_DELTA);
else
	ZoomOut (-zDelta / WHEEL_DELTA);
return 0;
}

//------------------------------------------------------------------------------

void CMineView::OnSelectPrevTab ()
{
DLE.MainFrame ()->ShowTools ();
DLE.ToolView ()->PrevTab ();
}

//------------------------------------------------------------------------------

void CMineView::OnSelectNextTab ()
{
DLE.MainFrame ()->ShowTools ();
DLE.ToolView ()->NextTab ();
}

//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------

void CMineView::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
int nPos = GetScrollPos (SB_VERT);
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

//------------------------------------------------------------------------------


//eof