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
#include "glew.h"

#include <math.h>
#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE [] = __FILE__;
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
	ON_COMMAND(ID_SEL_PREV_TAB, OnSelectPrevTab)
	ON_COMMAND(ID_SEL_NEXT_TAB, OnSelectNextTab)
END_MESSAGE_MAP()

BOOL CMineView::PreCreateWindow(CREATESTRUCT& cs)
{
return CView::PreCreateWindow(cs);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

COLORREF CRenderData::PenColor (int nPen)
{
	static COLORREF penColors [] = {
		RGB (  0,  0,  0),
		RGB (255,255,255),
		RGB (255, 196, 0),
		RGB (255,   0, 0),
		RGB (255,   0, 0),
		RGB (128,128,128),
		RGB (160,160,160),
		RGB (  0,255,  0),
		RGB (  0,128,  0),
		RGB (  0,128,128),
		RGB (  0,  0,255),
		RGB (  0,128,255),
		RGB (  0,255,255),
		RGB (255,196,  0),
		RGB (255,128,  0),
		RGB (255,  0,255)
	};

return (nPen < penCount) ? penColors [nPen] : RGB (0, 0, 0);
}

//------------------------------------------------------------------------------

CRenderData::CRenderData ()
{
m_renderBuffer = 0;
m_depthBuffer = null;
m_bDepthTest = true;

for (int nWeight = 0; nWeight < 2; nWeight++)
	for (int nPen = 0; nPen < penCount; nPen++)
		m_pens [nWeight][nPen] = new CPen (PS_SOLID, nWeight + 1, PenColor (nPen));
}

//------------------------------------------------------------------------------

CRenderData::~CRenderData()
{
for (int nWeight = 0; nWeight < 2; nWeight++)
	for (int nPen = 0; nPen < penCount; nPen++)
		delete m_pens [nWeight][nPen];
}

//------------------------------------------------------------------------------
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

m_renderers [0] = new CRendererSW (m_renderData);
m_renderers [1] = new CRendererGL (m_renderData);
m_nRenderer = 0;
SetRenderer (1);

for (int i = eMouseStateIdle; i < eMouseStateCount; i++)
	m_hCursors [i] = LoadCursor ((nIdCursors [i] == IDC_ARROW) ? null: DLE.m_hInstance, nIdCursors [i]);
ViewObjectFlags () = eViewObjectsAll;
ViewMineFlags () = eViewMineLights | eViewMineWalls | eViewMineSpecial;
m_viewOption = eViewTextured;
m_nDelayRefresh = 0;
m_bHScroll = 
m_bVScroll = false;
m_xScrollRange =
m_yScrollRange = 0;
m_xScrollCenter =
m_yScrollCenter = 0;
m_nMineCenter = 2;
m_nViewDist = 0;
SetMineMoveRate (1.0);
SetViewMoveRate (1.0);
Reset ();
}

//------------------------------------------------------------------------------

void CMineView::Reset (void)
{
ViewWidth () = ViewHeight () = ViewDepth () = 0;	// force OnDraw to initialize these

tunnelMaker.Destroy ();
m_bUpdate = true;
m_mouseState  = 
m_lastMouseState = eMouseStateIdle;
m_selectMode = eSelectSide;
m_lastSegment = 0;

Renderer ().Reset ();
// calculate transformation m_view based on move, size, and spin
m_lightTimer =
m_selectTimer = -1;
m_nFrameRate = 100;
m_xRenderOffs = m_yRenderOffs = 0;
}

//------------------------------------------------------------------------------

CMineView::~CMineView()
{
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

//------------------------------------------------------------------------------

void CMineView::SetRenderer (int nRenderer) 
{ 
if (!m_renderer || (m_nRenderer != nRenderer)) {
	m_renderer = m_renderers [m_nRenderer = abs (nRenderer)];
	Reset ();
	FitToView ();
	Refresh ();
	}
}

//------------------------------------------------------------------------------

void CMineView::SetPerspective (int nPerspective) 
{ 
if (Renderer ().SetPerspective (nPerspective)) {
	FitToView ();
	Refresh ();
	DLE.ToolView ()->SettingsTool ()->m_nPerspective = nPerspective;
	}
}

//------------------------------------------------------------------------------

bool CMineView::UpdateScrollBars (void)
{
#if 1
return false;
#else
Project ();

	int		dx = m_maxViewPoint.m_screen.x - m_minViewPoint.m_screen.x + 1;
	int		dy = m_minViewPoint.m_screen.y - m_maxViewPoint.m_screen.y + 1;
	double	r;

bool bHScroll = m_bHScroll;
bool bVScroll = m_bVScroll;
int xScrollCenter = m_xScrollCenter;
int yScrollCenter = m_yScrollCenter;
int xScrollRange = m_xScrollRange;
int yScrollRange = m_yScrollRange;
m_bHScroll = (dx > ViewWidth ());
m_bVScroll = (dy > ViewHeight ());
ShowScrollBar (SB_HORZ, m_bHScroll);
ShowScrollBar (SB_VERT, m_bVScroll);
InitViewDimensions ();
Project ();
dx = m_maxViewPoint.m_screen.x - m_minViewPoint.m_screen.x + 1;
dy = m_minViewPoint.m_screen.y - m_maxViewPoint.m_screen.y + 1;
if (m_bHScroll) {
	if (xScrollRange > m_xScrollRange)
		SetScrollRange (SB_HORZ, 0, m_xScrollRange = dx - ViewWidth (), TRUE);
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
		SetScrollRange (SB_HORZ, 0, m_xScrollRange = dx - ViewWidth (), TRUE);
	}
else {
	m_xScrollRange = 0;
	m_xRenderOffs = 0;
	}
if (m_bVScroll) {
	if (yScrollRange > m_yScrollRange)
		SetScrollRange (SB_VERT, 0, m_yScrollRange = dy - ViewHeight (), TRUE);
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
		SetScrollRange (SB_VERT, 0, m_yScrollRange = dy - ViewHeight (), TRUE);
	}
else {
	m_yScrollRange = 0;
	m_yRenderOffs = 0;
	}
return (bHScroll != m_bHScroll) || (bVScroll != m_bVScroll);
#endif
}

//------------------------------------------------------------------------------

void CMineView::OnDraw (CDC* pViewDC)
{
CHECKMINE;

CDlcDoc* pDoc = GetDocument();
ASSERT_VALID(pDoc);
if (!pDoc) 
	return;

Renderer ().SetDC (pViewDC);
if (!m_nRenderer) {
	if (DrawRubberBox () || DrawDragPos ()) {
		m_bUpdate = false;
		return;
		}
	}

InitView (pViewDC);
if (m_bUpdate) {
	ClearView ();
	m_xRenderOffs = m_bHScroll ? GetScrollPos (SB_HORZ) - m_xScrollCenter: 0;
	m_yRenderOffs = m_bVScroll ? GetScrollPos (SB_VERT) - m_yScrollCenter: 0;

	BeginRender ();
	Project ();
	EndRender ();
	ShiftViewPoints ();
	switch (m_viewOption) {
		case eViewTextured:
			DrawSegmentsTextured ();
			break;

		case eViewTexturedWireFrame:
			DrawWireFrame (false);
			DrawSegmentsTextured ();
			break;

		case eViewWireFrameFull:
			DrawWireFrame (false);
			break;

		case eViewHideLines:
			DrawWireFrame (false); 
			break;

		case eViewNearbyCubeLines:
			DrawWireFrame (false);
			break;

		case eViewWireFrameSparse:
			DrawWireFrame (true);
			break;
		}
	}

DrawRubberBox ();
if (m_nRenderer)
	DrawDragPos ();
DrawHighlight ();
DrawMineCenter ();

Renderer ().EndRender (true);
Renderer ().SetDC (pViewDC);
m_bUpdate = false;
}

//------------------------------------------------------------------------------

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

	int i, j = lightManager.Count ();

#pragma omp parallel for
for (i = 0; i < j; i++) {
	CVariableLight *flP = lightManager.VariableLight (i);
	int delay = X2I (flP->m_info.delay * 1000);
	if (delay) {
		tLightTimer *ltP = lightManager.LightTimer (i);
		ltP->ticks += (1000 + m_nFrameRate / 2) / m_nFrameRate;
		if (ltP->ticks >= delay) {
			short h = ltP->ticks / delay;
			ltP->ticks %= delay;
			ltP->impulse = (ltP->impulse + h) % 32;
			}
		}
	}
}

//------------------------------------------------------------------------------

bool CMineView::SetLightStatus (void)
{
#if 0
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
#else
return lightManager.DeltaIndexCount () > 0;
#endif
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
	if (DLE.MineView ()->RenderVariableLights ()) {
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

//----------------------------------------------------------------------------

void CMineView::EnableDeltaShading (int bEnable, int nFrameRate, int bShowLightSource)
{
if (bEnable) {
	DLE.MineView ()->ViewMineFlags () |= eViewMineVariableLights;
	if ((nFrameRate >= 10) && (nFrameRate <= 100))
		m_nFrameRate = nFrameRate;
	//m_lightTimer = SetTimer (3, (UINT) (1000 / m_nFrameRate + m_nFrameRate / 2), null);
	m_lightTimer = SetTimer (3, (UINT) 33, null);
	lightManager.AnimInfo ().Clear ();
	}
else {
	DLE.MineView ()->ViewMineFlags () &= ~eViewMineVariableLights;
	if (m_lightTimer != -1) {
		KillTimer (m_lightTimer);
		m_lightTimer = -1;
		}
	}
}

//----------------------------------------------------------------------------

BOOL CMineView::SetWindowPos (const CWnd *pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
	CRect	rc;

	GetClientRect (rc);

if ((rc.Width () != cx) || (rc.Height () != cy))
	DLE.MainFrame ()->ResetPaneMode ();
return CView::SetWindowPos (pWndInsertAfter, x, y, cx, cy, nFlags);
}

//----------------------------------------------------------------------------

void CMineView::ResetView (bool bRefresh)
{
Renderer ().Release ();
if (bRefresh)
	Refresh (true);
}

//----------------------------------------------------------------------------

void CMineView::InitView (CDC *pViewDC)
{
m_bUpdate = (0 > Renderer ().Setup (this, pViewDC));
}

//----------------------------------------------------------------------------

void CMineView::ClearView (void)
{
Renderer ().ClearView ();
}

//--------------------------------------------------------------------------

bool CMineView::ViewObject (CGameObject *objP)
{
switch(objP->Type ()) {
	case OBJ_ROBOT:
		if (ViewObject (eViewObjectsRobots))
			return true;
		if (ViewObject (eViewObjectsKeys)) {
			int nId = objP->Contains (OBJ_POWERUP);
			if ((nId >= 0) && (powerupTypeTable [nId] == POWERUP_KEY_MASK))
				return true;
			nId = objP->TypeContains (OBJ_POWERUP);
			if ((nId >= 0) && (powerupTypeTable [nId] == POWERUP_KEY_MASK))
				return true;
			}
		if (ViewObject (eViewObjectsControlCenter) && objP->IsBoss ())
			return true;
		return false;

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

	case OBJ_REACTOR:
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
 
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
// CMineView message handlers

BOOL CMineView::OnEraseBkgnd(CDC* pDC)
{
//	return CView::OnEraseBkgnd(pDC);
	return TRUE;
}

//------------------------------------------------------------------------------

void CMineView::OnSize (UINT nType, int cx, int cy)
{
	CRect	rc;

GetClientRect(rc);
// define global screen variables (these must be redefined if window is sized)
m_viewCenter.x = (rc.right - rc.left) / 2;
m_viewCenter.y = (rc.bottom - rc.top) / 2;
//aspect_ratio = (Center ().y/7.0) / (Center ().x/10.0);
//aspect_ratio = (double) rc.Height () / (double) rc.Width ();
m_viewMax.x = 8 * rc.right;
m_viewMax.y = 8 * rc.bottom;
//if (DLE.MainFrame () && ((ViewWidth () != cx) || (ViewHeight () != cy)))
//	DLE.MainFrame ()->ResetPaneMode ();
CView::OnSize (nType, cx, cy);
m_bUpdate = true;
}

//------------------------------------------------------------------------------

void CMineView::SetViewOption (eViewOptions option)
{
m_viewOption = option;
Refresh ();
}

//------------------------------------------------------------------------------

void CMineView::ToggleViewMine (eMineViewFlags flag)
{
ViewMineFlags () ^= flag;
EnableDeltaShading ((ViewMineFlags () & eViewMineVariableLights) != 0, -1, -1);
Refresh ();
}

//------------------------------------------------------------------------------

void CMineView::ToggleViewObjects(eObjectViewFlags mask)
{
	ViewObjectFlags () ^= mask;
	Refresh ();
}

//------------------------------------------------------------------------------

void CMineView::SetViewMineFlags(uint mask)
{
	ViewMineFlags () = mask;
	Refresh ();
}

//------------------------------------------------------------------------------

void CMineView::SetViewObjectFlags (uint mask)
{
	ViewObjectFlags () = mask;
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

void CMineView::RecordMousePos (CPoint& mousePos, CPoint point)
{
mousePos.x = point.x;
mousePos.y = m_nRenderer ? ViewHeight () - point.y : point.y;
}

//------------------------------------------------------------------------------

void CMineView::OnMouseMove (UINT nFlags, CPoint point)
{
RecordMousePos (point, point);

	CPoint change = m_lastMousePos - point;

if (GetFocus () != this)
	SetFocus ();
if (change.x || change.y) {
	switch (m_mouseState) {
		case eMouseStateIdle:
			if (SelectMode (eSelectPoint) || SelectMode (eSelectLine) || SelectMode (eSelectSide) || SelectMode (eSelectSegment))
				Invalidate (FALSE);

#if 0
			if (Perspective ()) {
				double scale = Perspective () ? 300.0 : 200.0;
				Rotate('Y', Perspective () ? double (change.x) / scale : -double (change.x) / scale);
				Rotate('X', Perspective () ? double (change.y) / scale : -double (change.y) / scale);
				break;
				}
#endif
		case eMouseStatePan:
		case eMouseStateRotate:
#if 0
			if ((nFlags & (MK_SHIFT | MK_CONTROL)) == (MK_SHIFT | MK_CONTROL)) {
				if (change.y > 0)
					ZoomOut (1, true);
				else if (change.y < 0)
					ZoomIn (1, true);
				}	
			else if (nFlags & MK_SHIFT) {
				SetMouseState (eMouseStateRotate);
				double scale = Perspective () ? 300.0 : 200.0;
				Rotate('Y', -double (change.x) / scale);
				Rotate('X', (m_nRenderer && !Perspective ()) ? double (change.y) / scale : -double (change.y) / scale);
				}
			else if (nFlags & MK_CONTROL) {
				double scale = m_nRenderer ? 0.5 : 1.0;
				SetMouseState (eMouseStatePan);
				double d = double (change.x) * scale;
				if (d != 0.0)
					Pan ('X', Perspective () ? -d : d);
				d = double (change.y) * scale;
				if (d != 0.0)
					Pan ('Y', -d);
				}
#else
			if (nFlags & MK_CONTROL) {
				if (nFlags & MK_SHIFT) {
					SetMouseState (eMouseStateRotate);
					double scale = Perspective () ? 300.0 : 200.0;
					Rotate('Y', -double (change.x) / scale);
					Rotate('X', (m_nRenderer && !Perspective ()) ? double (change.y) / scale : -double (change.y) / scale);
					}
				else {
					double scale = m_nRenderer ? 0.5 : 1.0;
					SetMouseState (eMouseStatePan);
					double d = double (change.x) * scale;
					if (d != 0.0)
						Pan ('X', Perspective () ? -d : d);
					d = double (change.y) * scale;
					if (d != 0.0)
						Pan ('Y', -d);
					}
				}
#endif
			else if ((m_mouseState == eMouseStatePan) || (m_mouseState == eMouseStateRotate))
				SetMouseState (eMouseStateIdle);
			break;

		case eMouseStateButtonDown:
			if (nFlags & MK_CONTROL)
				SetMouseState (eMouseStateZoom);
			else {
				int v = vertexManager.Index (current->Vertex ());
				if ((abs (m_clickPos.x - vertexManager [v].m_screen.x) < 5) && 
					 (abs (m_clickPos.y - vertexManager [v].m_screen.y) < 5)) {
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
			if (Perspective ()) {
				if ((change.x > 0) || ((change.x == 0) && (change.y > 0))) {
					SetMouseState (eMouseStateZoomOut);
					ZoomOut (1, true);
					}
				else if ((change.x < 0) || ((change.x == 0) && (change.y < 0))) {
					SetMouseState (eMouseStateZoomIn);
					ZoomIn (1, true);
					}
				}
			else {
				static int nChange = 0;
				if ((m_lastMouseState != eMouseStateZoomOut) && (m_lastMouseState != eMouseStateZoomIn))
					nChange = 0;
				if ((change.x < 0) || ((change.x == 0) && (change.y < 0))) {
					SetMouseState (eMouseStateZoomIn);
					if (nChange > 0)
						nChange = 0;
					nChange += change.x ? change.x : change.y;
					if (nChange < -30) {
						nChange = 0;
						ZoomIn (1, true);
						}
					}
				else if ((change.x > 0) || ((change.x == 0) && (change.y > 0))) {
					SetMouseState (eMouseStateZoomOut);
					if (nChange < 0)
						nChange = 0;
					nChange += change.x ? change.x : change.y;
					if (nChange > 30) {
						nChange = 0;
						ZoomOut (1, true);
						}
					}
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
RecordMousePos (m_clickPos, point);
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
RecordMousePos (m_releasePos, point);
m_releaseState = nFlags;
if (m_mouseState == eMouseStateButtonDown)
	if (m_clickState & MK_CONTROL)
		ZoomIn ();
	else {
		SetMouseState (eMouseStateIdle);
		if (Perspective ())
			SelectCurrentElement (m_clickPos.x, m_clickPos.y, (m_clickState & MK_SHIFT) ? 1 : -1);
		else {
			if (m_clickState & MK_SHIFT)
				SelectCurrentSide (m_clickPos.x, m_clickPos.y);
			else
				SelectCurrentSegment (1, m_clickPos.x, m_clickPos.y);
			}
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
RecordMousePos (m_clickPos, point);
m_clickState = nFlags;
CView::OnRButtonDown (nFlags, point);
}

//------------------------------------------------------------------------------

void CMineView::OnRButtonUp (UINT nFlags, CPoint point)
{
RecordMousePos (m_releasePos, point);
m_releaseState = nFlags;
if (m_mouseState == eMouseStateButtonDown) {
	if (m_clickState & MK_CONTROL)
		ZoomOut ();
	else if (!Perspective () && !(m_clickState & MK_SHIFT)) {
		SetMouseState (eMouseStateIdle);
		SelectCurrentObject (m_clickPos.x, m_clickPos.y);
		}
	else {
		CMenu		contextMenu;
		CMenu*	tracker;
		contextMenu.LoadMenu (IDR_MINE_CONTEXT_MENU);
		ClientToScreen (&point);
		tracker = contextMenu.GetSubMenu (0); 
		tracker->CheckMenuItem ((UINT) theMine->SelectMode (), MF_BYPOSITION | MF_CHECKED);
		if (theMine->EditReference ())
			tracker->CheckMenuItem ((UINT) ID_EDIT_VIEWER_IS_REFERENCE, MF_BYCOMMAND | MF_CHECKED);
	   int nChoice = tracker->TrackPopupMenu (TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, point.x , point.y, AfxGetMainWnd ()); 
		contextMenu.DestroyMenu ();
		if (nChoice) {
			if ((nChoice >= ID_SEL_POINTMODE) && (nChoice <= ID_SEL_BLOCKMODE))
				SetSelectMode (nChoice - ID_SEL_POINTMODE);
			else if (nChoice == ID_EDIT_VIEWER_IS_REFERENCE)
				theMine->EditReference () = !theMine->EditReference ();
			else if (nChoice == ID_EDIT_QUICKCOPY)
				blockManager.QuickCopy ();
			else if (nChoice == ID_EDIT_QUICKPASTE)
				blockManager.QuickPaste ();
			else if (nChoice == ID_VIEW_COLLAPSE_EDGE)
				segmentManager.CollapseEdge ();
			else if (nChoice == ID_VIEW_CREATE_WEDGE)
				segmentManager.CreateWedge ();
			else if (nChoice == ID_VIEW_CREATE_PYRAMID)
				segmentManager.CreatePyramid ();
			else if (nChoice == ID_EDIT_UNDO)
				undoManager.Undo ();
			else if (nChoice == ID_EDIT_REDO)
				undoManager.Redo ();
			Refresh ();
			}
		}
	}
else if (m_mouseState == eMouseStateRubberBand)
   ResetRubberRect ();
SetMouseState (eMouseStateIdle);
CView::OnRButtonUp (nFlags, point);
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
CSegment	*segP = segmentManager.Segment (0);
for (int i = segmentManager.Count (); i; i--, segP++) {
	for (short j = 0; j < MAX_VERTICES_PER_SEGMENT; j++)
		if ((segP->m_info.vertexIds [j] == v) && Visible (segP))
			return true;
	}
return false;
}

//------------------------------------------------------------------------------

void CMineView::MarkRubberBandedVertices (void)
{
CHECKMINE;

for (int i = 0, j = vertexManager.Count (); i < j; i++) {
	CVertex& v = vertexManager [i];
	if (BETWEEN (m_clickPos.x, v.m_screen.x, m_releasePos.x) &&
		 BETWEEN (m_clickPos.y, v.m_screen.y, m_releasePos.y) &&
		 VertexVisible (i)) {
		if (m_clickState & MK_SHIFT)
			vertexManager.Status (i) &= ~MARKED_MASK;
		else
			vertexManager.Status (i) |= MARKED_MASK;
		m_bUpdate = true;
		}
	}
if (m_bUpdate) 
	segmentManager.UpdateMarked ();
Refresh ();
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

BOOL CMineView::DrawRubberBox (void)
{
if (theMine == null) return FALSE;

	static CRect	prevRect (0, 0, 0, 0);
      
if (m_mouseState != eMouseStateRubberBand)
	return FALSE;
if ((m_rubberRect.Width () || m_rubberRect.Height ())) {
      CPoint	rubberPoly [5];
   
	if (m_nRenderer) {
		glPushAttrib (GL_ENABLE_BIT);
		glLineStipple (1, 0x3333);  
		glEnable (GL_LINE_STIPPLE);
		Renderer ().SelectPen (penWhite + 1);
		}
	else {
		DC ()->SetROP2 (R2_XORPEN);
		Renderer ().SelectPen (penBlack + 1);
		}
   rubberPoly [0].x = m_rubberRect.left + RUBBER_BORDER;
   rubberPoly [0].y = m_rubberRect.top + RUBBER_BORDER;
   rubberPoly [1].x = m_rubberRect.right - RUBBER_BORDER;
   rubberPoly [1].y = m_rubberRect.top + RUBBER_BORDER;
   rubberPoly [2].x = m_rubberRect.right - RUBBER_BORDER;
   rubberPoly [2].y = m_rubberRect.bottom - RUBBER_BORDER;
   rubberPoly [3].x = m_rubberRect.left + RUBBER_BORDER;
   rubberPoly [3].y = m_rubberRect.bottom - RUBBER_BORDER;
   rubberPoly [4] = rubberPoly [0];
	Renderer ().BeginRender (true);
	Renderer ().PolyLine (rubberPoly, sizeof (rubberPoly) / sizeof (POINT));
	Renderer ().EndRender ();
	if (m_nRenderer)
		glPopAttrib ();
	else 
	   DC ()->SetROP2 (R2_COPYPEN);
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
	static short wheelDeltas [2] = {WHEEL_DELTA, WHEEL_DELTA / 10};

	CRect	rc;

GetWindowRect (rc);
if ((pt.x < rc.left) || (pt.x >= rc.right) || (pt.y < rc.top) || (pt.y >= rc.bottom))
	return DLE.TextureView ()->OnMouseWheel (nFlags, zDelta, pt);
if (zDelta > 0)
	ZoomIn (zDelta / wheelDeltas [Perspective ()]);
else
	ZoomOut (-zDelta / wheelDeltas [Perspective ()]);
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
		nPos -= ViewWidth ();
		break;
	case SB_PAGEDOWN:
		nPos += ViewWidth ();
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
		nPos -= ViewHeight ();
		break;
	case SB_PAGEDOWN:
		nPos += ViewHeight ();
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