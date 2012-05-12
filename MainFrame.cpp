
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "dle-xp.h"

#include "MainFrame.h"
#include "global.h"
#include "toolview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//------------------------------------------------------------------------------
// CMainFrame

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_SEPARATOR
/*
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
*/
	};

#define TOOLBAR_STYLE   WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_GRIPPER | CBRS_SIZE_DYNAMIC
#define DOCKING_STYLE	CBRS_ALIGN_ANY //| CBRS_FLOAT_MULTI

//------------------------------------------------------------------------------
// CMainFrame construction/destruction

CMainFrame::CMainFrame ()
{
	// TODO: add member initialization code here
m_mineView = null;
m_textureView = null;
m_toolView = null;
m_paneMode = 0;
m_bShowCtrlBar = 1;
m_bRecalcBarLayout = 0;
m_texPaneWidth = -1;
m_toolMode = 1;
m_textureMode = 1;
m_mineZoom = 0;
nLayout = GetPrivateProfileInt ("DLE-XP", "Layout", 0, INIFILE);
#if EDITBAR
m_bEditorTB = 1;
#else
m_bEditorTB = 0;
m_pEditTool = null;
#endif

}

//------------------------------------------------------------------------------

CMainFrame::~CMainFrame ()
{
}

//------------------------------------------------------------------------------

void CMainFrame::OnClose ()
{
#if EDITBAR == 0
if (m_bEditorTB)
	OnEditorToolbar ();
#endif
DLE.SaveLayout ();
if (ToolView ()->SettingsTool ())
	ToolView ()->SettingsTool ()->SaveAppSettings ();
ToolView ()->Destroy ();
CFrameWnd::OnClose ();
}

//------------------------------------------------------------------------------

void CMainFrame::OnSize (UINT nType, int cx, int cy)
{
CFrameWnd::OnSize (nType, cx, cy);
if (ToolView ())
	ToolView ()->RecalcLayout (m_toolMode, m_textureMode);
RecalcLayout (m_toolMode, m_textureMode);
}

//------------------------------------------------------------------------------

int CMainFrame::CreateToolBars (bool bToolBar, bool bEditBar)
{
EnableDocking (DOCKING_STYLE);
if (bToolBar && !IsWindow (m_toolBar.m_hWnd)) {
	if (!m_toolBar.CreateEx (this, TBSTYLE_FLAT, TOOLBAR_STYLE) || 
		 !m_toolBar.LoadToolBar (IDR_MAINFRAME)) {
		TRACE0 ("Failed to create toolbar\n");
		return -1;      // fail to create
		}
	m_toolBar.EnableDocking (DOCKING_STYLE);
	DockControlBar (&m_toolBar);
	}
#if EDITBAR
if (bEditBar && !IsWindow (m_editBar.m_hWnd)) {
	if (!m_editBar.CreateEx (this, TBSTYLE_FLAT, TOOLBAR_STYLE) ||
		 !m_editBar.LoadToolBar (IDR_EDIT_TOOLBAR)) {
		TRACE0 ("Failed to create edit toolbar\n");
		return -1;      // fail to create
		}
	m_editBar.EnableDocking (DOCKING_STYLE);
	//DockControlBar (&m_editBar);
	}
#endif
return 0;
}

//------------------------------------------------------------------------------

void CMainFrame::DockToolBars (bool bToolBar, bool bEditBar)
{
if (bToolBar) {
	m_toolBar.EnableDocking (DOCKING_STYLE);
	DockControlBar (&m_toolBar);
	}
#if EDITBAR
if (bEditBar) {
	m_editBar.EnableDocking (DOCKING_STYLE);
	DockControlBar (&m_editBar);
	}
#endif
}

//------------------------------------------------------------------------------

int CMainFrame::CreateStatusBar ()
{
	UINT nId, nStyle;
	int cxWidth;

if (!m_statusBar.Create (this) || 
	 !m_statusBar.SetIndicators (indicators, sizeof (indicators)/sizeof (UINT))) {
	TRACE0 ("Failed to create status bar\n");
	return -1;      // fail to create
	}
m_statusBar.GetPaneInfo (0, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (0, nId, SBPS_NORMAL, 50);
m_statusBar.GetPaneInfo (1, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (1, nId, SBPS_NORMAL, 600);
m_statusBar.GetPaneInfo (2, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (2, nId, SBPS_STRETCH | SBPS_NORMAL, cxWidth);
m_statusBar.GetPaneInfo (3, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (3, nId, SBPS_NORMAL, 75);
m_statusBar.GetPaneInfo (4, nId, nStyle, cxWidth);
m_statusBar.SetPaneInfo (4, nId, SBPS_NORMAL, 75);
return 0;
}

//------------------------------------------------------------------------------

int CMainFrame::OnCreate (LPCREATESTRUCT lpCreateStruct)
{
if (CFrameWnd::OnCreate (lpCreateStruct) == -1)
	return -1;
if (CreateToolBars ())
	return -1;
if (CreateStatusBar ())
	return -1;
UpdateInsModeButtons (segmentManager.AddMode ());
return 0;
}

//------------------------------------------------------------------------------

BOOL CMainFrame::OnBarCheck (UINT nID)
{
BOOL bCheck = CFrameWnd::OnBarCheck (nID);
CControlBar* pBar = GetControlBar (nID);
m_bShowCtrlBar = pBar && ( (pBar->GetStyle () & WS_VISIBLE) != 0);
#if EDITBAR
m_editBar.ShowWindow ( (m_bShowCtrlBar && m_bEditorTB) ? SW_SHOW : SW_HIDE);
#endif
if (m_bShowCtrlBar) {
	if (m_bRecalcBarLayout)
	RecalcLayout (1);
	if (!CreateToolBars ())
		FixToolBars ();
	}
else {
	CRect rc;
	if (nLayout == 2)
		m_splitter1.GetPane (0,1)->GetWindowRect (rc);
	else
		m_splitter2.GetPane (1,0)->GetWindowRect (rc);
	m_bRecalcBarLayout = (DLE.ToolSize ().cy == rc.Height () + 10);
	}
return bCheck;
}

//------------------------------------------------------------------------------

void CMainFrame::FixToolBars (void)
{
m_toolBar.ShowWindow (SW_SHOW);
#if EDITBAR
	CRect	rc1, rc2;
	CPoint p;

m_toolBar.GetWindowRect (rc1);
GetClientRect (rc2);
rc2.left = rc2.right - rc1.Height ();
p.x = rc2.left;
p.y = rc2.top;
m_editBar.ShowWindow (SW_SHOW);
FloatControlBar (&m_editBar, p, (UINT) AFX_IDW_DOCKBAR_RIGHT);
#endif
}

//------------------------------------------------------------------------------

CMineView* CMainFrame::GetMineView () 
{
CView* view = null;
CDocument* doc = GetActiveDocument ();

if (doc) {
	POSITION pos = doc->GetFirstViewPosition ();
	while (pos != null) {
		view = (CMineView *)doc->GetNextView (pos);
		if (MineView ()->IsKindOf (RUNTIME_CLASS (CMineView))) break;
		}
	ASSERT (MineView ()->IsKindOf (RUNTIME_CLASS (CMineView)));
	}
return (CMineView *)view;
}


CTextureView* CMainFrame::GetTextureView () 
{
return TextureView ();
}


CToolView* CMainFrame::GetToolView () 
{
return ToolView ();
}


BOOL CMainFrame::PreCreateWindow (CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

return CFrameWnd::PreCreateWindow (cs);
}

//------------------------------------------------------------------------------

#define CX_TOOLS		354
#define CY_TOOLS		150
#define CX_TEXTURES	170
#define CY_TEXTURES	155

BOOL CMainFrame::OnCreateClient (LPCREATESTRUCT lpcs, CCreateContext * pContext)
{
	struct	CCreateContext context = *pContext;
	CRect		rc;

if (!m_splitter1.CreateStatic (this, 1, 2, WS_CHILD | WS_VISIBLE | WS_BORDER))
	return FALSE;
GetClientRect (rc);
rc.InflateRect (-2, -2);
if (nLayout == 1) {
	context.m_pNewViewClass = RUNTIME_CLASS (CToolView);
	m_splitter1.CreateView (0, 0, RUNTIME_CLASS (CToolView), CSize (CX_TOOLS, rc.Height ()), &context);
	m_splitter1.SetRowInfo (0, rc.Height (), 16);
	m_splitter1.SetColumnInfo (0, CX_TOOLS, 16);
	m_splitter1.SetColumnInfo (1, rc.Width () - CX_TOOLS, 16);
	m_splitter2.CreateStatic (&m_splitter1, 2, 1, WS_CHILD | WS_VISIBLE | WS_BORDER, m_splitter1.IdFromRowCol (0, 1));
	context.m_pNewViewClass = RUNTIME_CLASS (CMineView);
	m_splitter2.CreateView (0, 0, RUNTIME_CLASS (CMineView), CSize (rc.Width (), rc.Height ()), &context);
	context.m_pNewViewClass = RUNTIME_CLASS (CTextureView);
	m_splitter2.CreateView (1, 0, RUNTIME_CLASS (CTextureView), CSize (rc.Width (), 0), &context);
	m_splitter2.SetRowInfo (0, rc.Height () - CY_TEXTURES, 16);
	m_splitter2.SetRowInfo (1, CY_TEXTURES, 16);
	m_splitter2.SetColumnInfo (0, rc.Width (), 0);
	SetActiveView ( (CView*) m_splitter2.GetPane (0,0)); 
	m_toolView = (CToolView *) m_splitter1.GetPane (0,0);
	m_mineView = (CMineView *) m_splitter2.GetPane (0,0);
	m_textureView = (CTextureView *) m_splitter2.GetPane (1,0);
	}
else {
	context.m_pNewViewClass = RUNTIME_CLASS (CTextureView);
	m_splitter1.CreateView (0, 0, RUNTIME_CLASS (CTextureView), CSize (CX_TEXTURES, rc.Height ()), &context);
	m_splitter1.SetRowInfo (0, rc.Height (), 16);
	m_splitter1.SetColumnInfo (0, CX_TEXTURES, 16);
	m_splitter1.SetColumnInfo (1, rc.Width () - CX_TEXTURES, 16);
	if (nLayout == 2) {
		context.m_pNewViewClass = RUNTIME_CLASS (CMineView);
		m_splitter1.CreateView (0, 1, RUNTIME_CLASS (CMineView), CSize (rc.Width (), rc.Height ()), &context);
		m_textureView = (CTextureView *) m_splitter1.GetPane (0,0);
		m_mineView = (CMineView *) m_splitter1.GetPane (0,1);
		context.m_pNewViewClass = RUNTIME_CLASS (CToolView);
		m_toolView = new CToolView ();
		//CRect rc (0,0,0,0);
		//m_toolView->Create (RUNTIME_CLASS (CToolView)->m_lpszClassName, "", 0, rc, this, 0, &context);
		}
	else {
		m_splitter2.CreateStatic (&m_splitter1, 2, 1, WS_CHILD | WS_VISIBLE | WS_BORDER, m_splitter1.IdFromRowCol (0, 1));
		context.m_pNewViewClass = RUNTIME_CLASS (CMineView);
		m_splitter2.CreateView (0, 0, RUNTIME_CLASS (CMineView), CSize (rc.Width (), rc.Height ()), &context);
		context.m_pNewViewClass = RUNTIME_CLASS (CToolView);
		m_splitter2.CreateView (1, 0, RUNTIME_CLASS (CToolView), CSize (rc.Width (), 0), &context);
		m_splitter2.SetRowInfo (0, rc.Height () - CY_TOOLS, 16);
		m_splitter2.SetRowInfo (1, CY_TOOLS, 0);
		m_splitter2.SetColumnInfo (0, rc.Width (), 16);
		SetActiveView ( (CView*)m_splitter2.GetPane (0,0)); 
		m_textureView = (CTextureView *) m_splitter1.GetPane (0,0);
		m_mineView = (CMineView *) m_splitter2.GetPane (0,0);
		m_toolView = (CToolView *) m_splitter2.GetPane (1,0);
		}
	}
return TRUE;
}

//------------------------------------------------------------------------------
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid () const
{
	CFrameWnd::AssertValid ();
}

void CMainFrame::Dump (CDumpContext& dc) const
{
	CFrameWnd::Dump (dc);
}

#endif //_DEBUG

//------------------------------------------------------------------------------

void CMainFrame::ConvertMine (int nVersion) 
{
	CConvertDlg	d;

if (DLE.IsD1File ()) {
	if (nVersion == 0) {
		ErrorMsg ("This mine is already a Descent 1 level.");
		return;
		}

	if (d.DoModal () == IDOK) {
		MineView ()->Refresh ();
		if (DLE.ExpertMode ())
			INFOMSG (" Mine converted to a Descent 2 level")
		else
			ErrorMsg ("Mine converted to a Descent 2 level.");
		segmentManager.UpdateWalls (MAX_WALLS_D1 + 1, MAX_WALLS_D2 + 1);
		triggerManager.ObjTriggerCount () = 0;
		}
	return;
	}

if (nVersion == 0) {
	theMine->ConvertD2toD1 ();
	segmentManager.UpdateWalls (WALL_LIMIT, MAX_WALLS_D2 + 1);
	segmentManager.UpdateWalls (MAX_WALLS_D2 + 1, MAX_WALLS_D1 + 1);
	triggerManager.ObjTriggerCount () = 0;
	DLE.ToolView ()->EditLight ();
	DLE.ToolView ()->LightTool ()->OnOK ();
	DLE.ToolView ()->EditDiag ();
	DLE.ToolView ()->DiagTool ()->OnCheckMine ();
	if (DLE.ExpertMode ())
		INFOMSG (" Mine converted to a Descent 1 level")
	return;
	}

if (nVersion == 1) {
	if (DLE.LevelVersion () != 7) {	
		AdjustMine (1);
		theMine->SetLevelVersion (7);
		if (DLE.ExpertMode ())
			INFOMSG (" Mine converted to a Standard level")
		else
			ErrorMsg ("Mine converted to a Standard level.\n\nAll Vertigo and D2X-XL level elements have been removed.");
		segmentManager.UpdateWalls (WALL_LIMIT, MAX_WALLS_D2 + 1);
		triggerManager.ObjTriggerCount () = 0;
		}
	return;
	}

if (nVersion == 2) {
	if (DLE.LevelVersion () < 8) {
		AdjustMine (2);
		theMine->SetLevelVersion (8);
		if (DLE.ExpertMode ())
			INFOMSG (" Mine converted to a Vertigo level")
		else
			ErrorMsg ("Mine converted to a Vertigo level.\n\nAll D2X-XL level elements have been removed.");
		segmentManager.UpdateWalls (WALL_LIMIT, MAX_WALLS_D2 + 1);
		triggerManager.ObjTriggerCount () = 0;
		}
	return;
	}

if (nVersion == 3) {
	if (DLE.LevelVersion () < LEVEL_VERSION) {
		theMine->SetLevelVersion (LEVEL_VERSION);
		if (DLE.ExpertMode ())
			INFOMSG (" Mine converted to a D2X-XL level")
		else
			ErrorMsg ("Mine converted to a D2X-XL level.");
		segmentManager.UpdateWalls (MAX_WALLS_D2 + 1, WALL_LIMIT);
		triggerManager.ObjTriggerCount () = 0;
		}
	}
}

//------------------------------------------------------------------------------

void CMainFrame::OnConvertToD1 () 
{
ConvertMine (0);
}


void CMainFrame::OnConvertToStandard () 
{
ConvertMine (1);
}


void CMainFrame::OnConvertToVertigo () 
{
ConvertMine (2);
}


void CMainFrame::OnConvertToD2X () 
{
ConvertMine (3);
}

//------------------------------------------------------------------------------

void CMainFrame::RecalcLayout (int nToolMode, int nTextureMode)
{
	CRect	rc, rc0, rc1;

if (!IsWindow (m_splitter1) || ( (nLayout < 2) && !IsWindow (m_splitter2)))
	return;
if (nLayout < 2)
	m_splitter2.RecalcLayout (nToolMode, nTextureMode);
if (nLayout != 0) {
	if (nTextureMode)
		m_textureMode = nTextureMode;
	if (nToolMode /*&& (nLayout < 2)*/) {
		if (!MinePane ())
			return;
		m_splitter1.GetClientRect (rc);
		MinePane ()->GetWindowRect (rc0);
		if (nLayout == 2)
			rc1.left = rc1.right = rc1.top = rc1.bottom = 0;
		else if (!ToolPane ())
			return;
		else
			ToolPane ()->GetWindowRect (rc1);
		if (rc.Width () <= 0)
			return;
		int cxIdeal = rc.Width () - rc1.Width () - 9 - 2 * GetSystemMetrics (SM_CYBORDER);
		if (cxIdeal <= 0)
			return;
		if (rc.Width () - rc1.Width () < rc0.Width ())
			m_splitter1.SetColumnInfo (0, cxIdeal, 0);
		else if ( (nToolMode == 1) || (rc1.Width () > DLE.ToolSize ().cx)) {
			m_splitter1.SetColumnInfo (1, rc.Width () - DLE.ToolSize ().cx, 0);
			m_splitter1.SetColumnInfo (0, DLE.ToolSize ().cx, 0);
			}
		else if (nToolMode == 2) {
			m_splitter1.SetColumnInfo (0, 0, 0);
			m_splitter1.SetColumnInfo (1, rc.Width () - 9, 0);
			}
		m_splitter1.RecalcLayout ();
		m_toolMode = nToolMode;
		}
	}
else {
	if (nToolMode && (nLayout < 2))
		m_toolMode = nToolMode;
	if (nTextureMode) {
			int	nWidth = (nTextureMode == 1) ? (m_texPaneWidth < 0) ? CX_TEXTURES : m_texPaneWidth : 0;

		if (nTextureMode == 2) {	//hide
			TexturePane ()->GetWindowRect (rc);
			m_texPaneWidth = rc.Width ();
			}
		m_splitter1.GetClientRect (rc);
		m_splitter1.SetColumnInfo (0, nWidth, 0);
		m_splitter1.SetColumnInfo (1, rc.Width () - nWidth - /*nWidth **/ 9, 0);
		m_splitter1.RecalcLayout ();
		m_textureMode = nTextureMode;
		}
	}
CFrameWnd::RecalcLayout ();
}

//------------------------------------------------------------------------------

void C2ndSplitterWnd::RecalcLayout (int nToolMode, int nTextureMode)
{
if (DLE.MainFrame () && DLE.ToolView ()) {
	CRect	rc, rc0, rc1;
	GetClientRect (rc);
	GetPane (0,0)->GetWindowRect (rc0);
	GetPane (1,0)->GetWindowRect (rc1);
	if (nLayout != 0) {
		if (nTextureMode) {
				int	nHeight = (nTextureMode == 1) ? (m_texPaneHeight < 0) ? CY_TEXTURES : m_texPaneHeight : 0;

			if (nTextureMode == 2)	//hide
				m_texPaneHeight = rc1.Height ();
			SetRowInfo (0, rc.Height () - nHeight - m_cySplitter - 2 * GetSystemMetrics (SM_CYBORDER), 0);
			SetRowInfo (1, nHeight, 0);
			}
		}
	else {
		if (rc.Height () - rc1.Height () < rc0.Height ())
			SetRowInfo (0, rc.Height () - rc1.Height () - m_cySplitter - 2 * GetSystemMetrics (SM_CYBORDER), 0);
		else if ( (nToolMode == 1) || (rc1.Height () > DLE.ToolSize ().cy)) {
			SetRowInfo (0, rc.Height () - DLE.ToolSize ().cy, 0);
			SetRowInfo (1, DLE.ToolSize ().cy, 0);
			}
		else if (nToolMode == 2) {
			SetRowInfo (0, rc.Height (), 0);
			SetRowInfo (1, 0, 0);
			}
		}
	}
CSplitterWnd::RecalcLayout ();
}

//------------------------------------------------------------------------------

void CMainFrame::DebugMsg (const char *pszMsg)
{
m_statusBar.SetPaneText (2, pszMsg); 
if (! (DLE.ToolView () && DLE.ToolView ()->DiagTool ()))
	return;
if (!DLE.ToolView ()->DiagTool ()->Inited ()) {
	int i = DLE.ToolView ()->m_pTools->GetActiveIndex ();
	DLE.ToolView ()->EditDiag ();
	DLE.ToolView ()->SetActive (i);
	}
if (!DLE.ToolView ()->DiagTool ()->Inited ())
	return;
DLE.ToolView ()->DiagTool ()->AddMessage (pszMsg, 100);
}

//------------------------------------------------------------------------------

bool CMainFrame::InitProgress (int nMax)
{
CRect rc;
m_statusBar.GetItemRect (2, &rc);
if (!m_progress.Create (WS_CHILD | WS_VISIBLE, rc, &m_statusBar, 1))
	return false;
m_progress.SetRange32 (0, nMax);
m_progress.SetStep (1);
return true;
}


//------------------------------------------------------------------------------

//eof MainFrame.cpp