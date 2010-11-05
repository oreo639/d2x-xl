// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __mainfrm_h
#define __mainfrm_h

#define EDITBAR	0

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "mineview.h"
#include "textureview.h"
#include "toolview.h"

extern int nLayout;

#if EDITBAR == 0

class CEditTool : public CDialog {
	public:
		CExtBitmapButton	m_btns [10];
		UINT_PTR	m_nTimer;
		WPARAM	m_nEditFunc;
		UINT		m_nTimerDelay;

		CEditTool ();
		void QuitEditFunc (void);
		virtual BOOL OnInitDialog ();
		afx_msg void OnEditGeo0 ();
		afx_msg void OnEditGeo1 ();
		afx_msg void OnEditGeo2 ();
		afx_msg void OnEditGeo3 ();
		afx_msg void OnEditGeo4 ();
		afx_msg void OnEditGeo5 ();
		afx_msg void OnEditGeo6 ();
		afx_msg void OnEditGeo7 ();
		afx_msg void OnEditGeo8 ();
		afx_msg void OnEditGeo9 ();
		bool EditGeo0 (void);
		bool EditGeo1 (void);
		bool EditGeo2 (void);
		bool EditGeo3 (void);
		bool EditGeo4 (void);
		bool EditGeo5 (void);
		bool EditGeo6 (void);
		bool EditGeo7 (void);
		bool EditGeo8 (void);
		bool EditGeo9 (void);
		afx_msg void OnSetFocus (CWnd* pOldWnd);
		afx_msg void OnKillFocus (CWnd* pNewWnd);
		afx_msg void OnTimer (UINT_PTR nIdEvent);
		afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnMouseMove (UINT nFlags, CPoint point);
		virtual BOOL OnNotify (WPARAM wParam, LPARAM lParam, LRESULT *pResult);

		DECLARE_MESSAGE_MAP ()
};

#endif

class CDlcSplitterWnd : public CSplitterWnd 
{
	public:
		int	m_texPaneHeight;
		CDlcSplitterWnd () : CSplitterWnd ()
			{ m_texPaneHeight = -1; }
		virtual void RecalcLayout (int nToolMode = 0, int nTextureMode = 0);
};


class CExtToolBar : public CToolBar
{
	public:
		int		m_nId;
		UINT		m_nState;
		int		m_nPos;
		UINT_PTR	m_nTimer;
		UINT		m_nTimerDelay;

		CExtToolBar ();
		afx_msg void OnLButtonUp (UINT nFlags, CPoint point);
		afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
		afx_msg void OnMouseMove (UINT nFlags, CPoint point);
		afx_msg void OnTimer (UINT_PTR nIdEvent);
		void Notify (UINT nMsg);
		int Width ();

		DECLARE_MESSAGE_MAP ()
};

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:
	CMineView* GetMineView();
	CTextureView* GetTextureView();
	CToolView* GetToolView();


// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:  // control bar embedded members
	CStatusBar			m_statusBar;
	CExtToolBar			m_toolBar;
#if EDITBAR
	CExtToolBar			m_editBar;
#else
	CEditTool			*m_pEditTool;
#endif
	int					m_bEditorTB;
	CSplitterWnd		m_splitter1;
	CDlcSplitterWnd	m_splitter2;
	CMineView			*m_mineView;
	CTextureView		*m_textureView;
	CToolView			*m_toolView;
	int					m_paneMode;
	int					m_toolMode;
	int					m_textureMode;
	int					m_mineZoom;
	int					m_bShowCtrlBar;
	int					m_bRecalcBarLayout;
	int					m_texPaneWidth;
	CProgressCtrl		m_progress;

public:
	BOOL OnBarCheck(UINT nID);
	inline CMineView *MineView ()
		{ return m_mineView; }
	inline CTextureView *TextureView ()
		{ return m_textureView; }
	inline CToolView *ToolView ()
		{ return m_toolView; }
	inline CDlcDoc *GetDocument ()
		{ return MineView ()->GetDocument (); }
	inline CWnd *TexturePane ()
		{ return nLayout ? m_splitter2.GetPane (1,0) : m_splitter1.GetPane (0,0); }
	inline CWnd *MinePane ()
		{ return m_splitter2.GetPane (0,0); }
	inline CWnd *ToolPane ()
		{ return nLayout ? m_splitter1.GetPane (0,0) : m_splitter2.GetPane (1,0); }
	virtual void RecalcLayout (int nToolMode = 0, int nTextureMode = 0);
	inline void StatusMsg (const char *pszMsg)
		{ m_statusBar.SetPaneText (1, pszMsg); }
	inline void InfoMsg (const char *pszMsg)
		{ m_statusBar.SetPaneText (2, pszMsg); }
	inline void InsModeMsg (const char *pszMsg)
		{ m_statusBar.SetPaneText (3, pszMsg); }
	inline void SelModeMsg (const char *pszMsg)
		{ m_statusBar.SetPaneText (4, pszMsg); }
	CProgressCtrl& Progress ()
		{ return m_progress; }
	bool InitProgress (int nMax);
	void DebugMsg (const char *);
	void FixToolBars ();
	void ShowTools (void);
	void ResetPaneMode ();
	void ResetMineZoom ();
	void SetSelectMode (eSelectModes mode);
	void UpdateSelectButtons (eSelectModes mode);
	void SetInsertMode (short mode);
	void UpdateInsModeButtons (short mode);
	int CreateToolBars (bool bToolBar = true, bool bEditBar = true);
	int CreateStatusBar ();
	void DockToolBars (bool bToolBar, bool bEditBar);

	void ConvertMine (int nVersion);

	bool EditGeoFwd (void);
	bool EditGeoUp (void);
	bool EditGeoBack (void);
	bool EditGeoLeft (void);
	bool EditGeoGrow (void);
	bool EditGeoRight (void);
	bool EditGeoRotLeft (void);
	bool EditGeoDown (void);
	bool EditGeoRotRight (void);
	bool EditGeoShrink (void);
// Generated message map functions
public:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose ();
	afx_msg void OnSize (UINT nType, int cx, int cy);
	afx_msg BOOL OnCreateClient (LPCREATESTRUCT lpcs, CCreateContext * pContext);
	afx_msg void OnExtBlkFmt ();
	afx_msg void OnUpdateExtBlkFmt ();
	afx_msg void OnConvertToStandard ();
	afx_msg void OnConvertToVertigo ();
	afx_msg void OnConvertToD2X ();
	afx_msg void OnEditorToolbar ();
	afx_msg void OnCheckMine ();
	afx_msg void OnUndo ();
	afx_msg void OnRedo ();
	afx_msg void OnEditGeoFwd ();
	afx_msg void OnEditGeoUp ();
	afx_msg void OnEditGeoBack ();
	afx_msg void OnEditGeoLeft ();
	afx_msg void OnEditGeoGrow ();
	afx_msg void OnEditGeoRight ();
	afx_msg void OnEditGeoRotLeft ();
	afx_msg void OnEditGeoDown ();
	afx_msg void OnEditGeoRotRight ();
	afx_msg void OnEditGeoShrink ();
	afx_msg void OnInsModeNormal ();
	afx_msg void OnInsModeExtend ();
	afx_msg void OnInsModeMirror ();
	afx_msg void OnToggleInsMode ();
	afx_msg void OnEditMark ();
	afx_msg void OnEditMarkAll ();
	afx_msg void OnEditUnmarkAll ();
	afx_msg void OnRedraw ();
	afx_msg void OnToggleViews ();
	afx_msg void OnToggleTexturePane();
	afx_msg void OnEditTexture();
	afx_msg void OnEditWall();
	afx_msg void OnEditCube();
	afx_msg void OnEditObject();
	afx_msg void OnEditEffect();
	afx_msg void OnEditLight();
	afx_msg void OnEditPrefs();
	afx_msg void OnEditTrigger();
	afx_msg void OnEditMission();
	afx_msg void OnEditReactor();
	afx_msg void OnTunnelGenerator();
	afx_msg void OnTxtFilters();
	afx_msg void OnViewZoomin();
	afx_msg void OnViewZoomout();
	afx_msg void OnViewFitToView();
	afx_msg void OnViewAlignSideRotation();
	afx_msg void OnViewAlllines();
	afx_msg void OnViewUsedTextures();
	afx_msg void OnViewCenterEntireMine();
	afx_msg void OnViewCenterOnCurrentSegment();
	afx_msg void OnViewCenterOnCurrentObject();
	afx_msg void OnViewHidelines();
	afx_msg void OnViewLights();
	afx_msg void OnViewNearbySegmentLines();
	afx_msg void OnViewObjectsAll();
	afx_msg void OnViewObjectsReactor();
	afx_msg void OnViewObjectsHostages();
	afx_msg void OnViewObjectsKeys();
	afx_msg void OnViewObjectsNone();
	afx_msg void OnViewObjectsPlayers();
	afx_msg void OnViewObjectsPowerups();
	afx_msg void OnViewObjectsRobots();
	afx_msg void OnViewObjectsWeapons();
	afx_msg void OnViewPanDown();
	afx_msg void OnViewPanIn();
	afx_msg void OnViewPanLeft();
	afx_msg void OnViewPanOut();
	afx_msg void OnViewPanRight();
	afx_msg void OnViewPanUp();
	afx_msg void OnViewPartialLines();
	afx_msg void OnViewRotateClockwise();
	afx_msg void OnViewRotateCounterclockwise();
	afx_msg void OnViewRotateHorizontallyLeft();
	afx_msg void OnViewRotateHorizontallyRight();
	afx_msg void OnViewRotateVerticallyDown();
	afx_msg void OnViewRotateVerticallyUp();
	afx_msg void OnViewShading();
	afx_msg void OnViewDeltaLights();
	afx_msg void OnViewSpecial();
	afx_msg void OnViewTextured();
	afx_msg void OnViewWalls();
	afx_msg void OnJoinPoints ();
	afx_msg void OnJoinLines ();
	afx_msg void OnJoinSides ();
	afx_msg void OnJoinCurrentSide ();
	afx_msg void OnSplitPoints ();
	afx_msg void OnSplitLines ();
	afx_msg void OnSplitSides ();
	afx_msg void OnSplitCurrentSide ();
	afx_msg void OnSelectPrevTab ();
	afx_msg void OnSelectNextTab ();
	afx_msg void OnSelectPointMode ();
	afx_msg void OnSelectLineMode ();
	afx_msg void OnSelectSideMode ();
	afx_msg void OnSelectCubeMode ();
	afx_msg void OnSelectObjectMode ();
	afx_msg void OnSelectBlockMode ();
	afx_msg void OnSelNextPoint ();
	afx_msg void OnSelPrevPoint ();
	afx_msg void OnSelNextLine ();
	afx_msg void OnSelPrevLine ();
	afx_msg void OnSelNextSide ();
	afx_msg void OnSelPrevSide ();
	afx_msg void OnSelNextCubeElem ();
	afx_msg void OnSelPrevCubeElem ();
	afx_msg void OnSelNextCube ();
	afx_msg void OnSelPrevCube ();
	afx_msg void OnSelNextObject ();
	afx_msg void OnSelPrevObject ();
	afx_msg void OnSelFwdCube ();
	afx_msg void OnSelBackCube ();
	afx_msg void OnSelOtherSegment ();
	afx_msg void OnSelOtherSide ();
	afx_msg void OnIncSpline ();
	afx_msg void OnDecSpline ();
	afx_msg void OnUpdateToggleViews (CCmdUI* pCmdUI);
	afx_msg void OnUpdateToggleTexPane(CCmdUI* pCmdUI);
	afx_msg void OnUpdateExtBlkFmt (CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditorToolbar (CCmdUI* pCmdUI);
	afx_msg void OnUpdateInsModeNormal (CCmdUI* pCmdUI);
	afx_msg void OnUpdateInsModeExtend (CCmdUI* pCmdUI);
	afx_msg void OnUpdateInsModeMirror (CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewUsedTextures(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewWalls(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewSpecial(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewShading(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewDeltaLights(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewPartiallines(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectsWeapons(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectsRobots(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectsPowerups(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectsPlayers(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectsNoobjects(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectsKeys(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectsHostages(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectsControlcenter(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewObjectsAllobjects(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewNearbySegmentLines(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewLights(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewHidelines(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewAlllines(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewTexturemapped(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#if !defined(_M_IA64) && !defined(_M_AMD64)
#	define INIFILE ".\\dle-xp.ini"
#else
#	define INIFILE ".\\dle-xp.ini"
#endif

#endif //__mainframe_h
