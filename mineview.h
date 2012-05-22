// dlcView.h : interface of the CMineView class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __mineview_h
#define __mineview_h

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxcview.h"
#include "DlcDoc.h"
#include "Matrix.h"
#include "PolyModel.h"
#include "Textures.h"
#include "renderer.h"
#include "glew.h"

// -----------------------------------------------------------------------------

enum eViewOptions {
	eViewWireFrameFull = 0,
	eViewHideLines,
	eViewNearbyCubeLines,
	eViewWireFrameSparse,
	eViewTextured,
	eViewTexturedWireFrame
	};

// -----------------------------------------------------------------------------

enum eSelectModes {
	eSelectPoint	= POINT_MODE,
	eSelectLine		= LINE_MODE,
	eSelectSide		= SIDE_MODE,
	eSelectSegment	= SEGMENT_MODE,
	eSelectObject	= OBJECT_MODE,
	eSelectBlock	= BLOCK_MODE
	};

// -----------------------------------------------------------------------------

enum eMouseStates
{
	eMouseStateIdle,
	eMouseStateButtonDown,
	eMouseStateSelect,
	eMouseStatePan,
	eMouseStateRotate,
	eMouseStateZoom,
	eMouseStateZoomIn,
	eMouseStateZoomOut,
	eMouseStateInitDrag,
	eMouseStateDrag,
	eMouseStateRubberBand,
	eMouseStateCount	//must always be last tag
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CViewPoint {
	public:
		ubyte				m_bTransformed :1;
		ubyte				m_bProjected :1;
		CDoubleVector	m_view;
		CLongVector		m_screen;

	public:
		CViewPoint () : m_bTransformed (0), m_bProjected (0) {}
		void Transform (void);
		void Project (void);
		inline void Reset (void) { m_bTransformed = m_bProjected = 0; }
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CMineView : public CView
{
protected: // create from serialization only
	CMineView();
	DECLARE_DYNCREATE(CMineView)

	CSplitterWnd*	m_pSplitter;

	CRenderData		m_renderData;
	// member variables
	bool 				m_bUpdate;
	bool 				m_bUpdateCursor;
	bool 				m_bDelayRefresh;
	int 				m_nDelayRefresh;
	uint				m_viewOption;
	uint				m_selectMode;
	HCURSOR			m_hCursors [eMouseStateCount];

	int 				m_mouseState;
	int 				m_lastMouseState;
	CPoint			m_lastMousePos;
	CPoint			m_clickPos, 
						m_releasePos,
						m_lastDragPos,
						m_highlightPos;
	UINT				m_clickState,
						m_releaseState;
	short				m_lastSegment;
	CRect				m_rubberRect;
	UINT_PTR			m_lightTimer;
	UINT_PTR			m_selectTimer;
	int 				m_nFrameRate;
	int				m_nShowSelectionCandidates;
	int				m_bEnableQuickSelection;
	bool 				m_bHScroll,
						m_bVScroll;
	int 				m_xScrollRange,
						m_yScrollRange;
	int 				m_xScrollCenter,
						m_yScrollCenter;
	int 				m_xRenderOffs,
						m_yRenderOffs;
	int 				m_nViewDist;
	int 				m_nMineCenter;
	int				m_nEditReference;

	CPoint			m_viewCenter;
	CPoint			m_viewMax;

	int 				m_x0, m_x1, m_y;
	double			m_z0, m_z1;

	CRenderer*		m_renderers [2];
	CRenderer*		m_renderer;
	int				m_nRenderer;

// Attributes
public:
	CDlcDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMineView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CGameObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMineView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	inline void SetViewDist (int nViewDist) {
		if (m_nViewDist != nViewDist) {
			m_nViewDist = nViewDist;
			Refresh ();
			}
		}
	inline int ViewDist (void) {
		return (m_nViewDist <= 10) ? m_nViewDist : 
		(m_nViewDist < 20) ? 10 + 2 * (m_nViewDist - 10) : 30 + 3 * (m_nViewDist - 20);
		}

	inline bool Visible (CSegment *segP) {
		if ((segP->m_info.function == SEGMENT_FUNC_SKYBOX) && !ViewFlag (eViewMineSkyBox))
			return false;
		if (!m_nViewDist)
			return true;
		return (segP->Index () >= 0) && (segP->Index () <= ViewDist ()); 
		}

	inline void SetEditReference (int nReference) { m_nEditReference = nReference; }
	inline int GetEditReference (void) { return Perspective () && m_nEditReference; }
	inline int EnableQuickSelection (void) { return m_bEnableQuickSelection; }
	inline int ShowSelectionCandidates (void) { return m_nShowSelectionCandidates; }
	void DrawMineCenter (void);
	bool VertexVisible (int v);
	void ComputeViewLimits (CRect* pRC = null);
	void ShiftViewPoints ();
	// drawing functions
	void InitView (CDC* pViewDC);
	bool UpdateScrollBars (void);
	void ClearView();
	void ResetView (bool bRefresh = false);
	bool InitViewDimensions (void);
	void DrawWireFrame (bool bSparse);
	void DrawSegmentsTextured (void);
	void DrawMarkedSegments (short bClear = 0);
	void DrawSegment (CSegment *segP, bool bSparse);
	void DrawSegment (short nSegment,short nSide, short linenum, short pointnum, short bClear = 0);
	void DrawSegmentPartial (CSegment *segP);
	void DrawSegmentWireFrame (CSegment *segP, bool bSparse = false, char bTunnel = 0);
	void DrawSparseSegmentWireFrame (CSegment *segP);
	void RenderSegmentWireFrame (CSegment *segP, bool bSparse);
	void DrawSegmentPoints (CSegment *segP);

	void DrawCurrentSegment (CSegment *segP, bool bSparse);
	void DrawLine (CSegment *segP, short vert1, short vert2);

	void DrawWalls (void);
	void DrawLights (void);
	void DrawOctagon(short nSide, short nSegment);
	void DrawObject (short objnum, short bClear = 0);
	void DrawObjects (short bClear = 0);
	bool DrawSelectablePoint (void);
	bool DrawSelectableEdge (void);
	bool DrawSelectableSides (void);
	bool DrawSelectableSegments (void);
	void DrawHighlight (short bClear = 0);
	void DrawTunnel (void);
	bool SelectWireFramePen (CSegment* segP);
	void SelectWallPen (CWall* wallP);
	void SelectObjectPen (CGameObject* objP, short bClear);

	// view control functions
	int FitToView (void);
	void TogglePerspective (void);
	void Rotate (char direction, double angle);
	void AlignSide ();
	void MarkVisibleVerts (bool bReset = false);
	void CenterOnMine ();
	void CenterOnSegment ();
	void CenterOnObject ();
	void SetViewOption (eViewOptions option);
	void ToggleViewMine (eMineViewFlags flag);
	void ToggleViewObjects (eObjectViewFlags mask);
	void SetViewMineFlags (uint mask);
	void SetViewObjectFlags (uint mask);
	void SetSelectMode (uint mode);
	void CalcSegDist (void);
	bool InRange (short *pv, short i);

	void NextPoint (int dir = 1);
	void PrevPoint ();
	void NextLine (int dir = 1);
	void PrevLine ();
	void NextSide (int dir = 1);
	void PrevSide ();
	void NextSegment (int dir = 1);
	void PrevSegment ();
	void SegmentForward (int dir = 1);
	void SegmentBackwards ();
	void SelectOtherSegment ();
	bool SelectOtherSide ();
	void NextObject (int dir = 1);
	void PrevObject ();
	void NextSegmentElement (int dir = 1);
	void PrevSegmentElement ();
	void HiliteTarget (void);

	void Refresh (bool bAll = true);
	void EnableDeltaShading (int bEnable, int nFrameRate, int bShowLightSource);
	void AdvanceLightTick (void);
	bool SetLightStatus (void);
	void Invalidate (BOOL bErase);
	void InvalidateRect (LPCRECT lpRect, BOOL bErase);

	bool ViewObject (CGameObject *objP);
	inline bool ViewObject (uint flag = 0) { return flag ? ((ViewObjectFlags () & flag) != 0) : (ViewObjectFlags () != 0); }
	inline bool ViewFlag (uint flag = 0) { return flag ? (ViewMineFlags () & flag) != 0 : (ViewMineFlags () != 0); }
	inline bool ViewOption (uint option) { return m_viewOption == option; }
	inline bool SelectMode (uint mode) { return m_selectMode == mode; }
	inline uint GetMineViewFlags () { return ViewMineFlags (); }
	inline uint GetObjectViewFlags () { return ViewObjectFlags (); }
	inline uint GetViewOptions () { return m_viewOption; }
	inline uint GetSelectMode () { return m_selectMode; }
	inline int& MineCenter (void) { return m_nMineCenter; }
	inline void DelayRefresh (bool bDelay) {
		if (bDelay)
			m_nDelayRefresh++;
		else if (m_nDelayRefresh > 0)
			m_nDelayRefresh--;
		}
	inline bool DelayRefresh (void) { return m_nDelayRefresh > 0; }
	inline CPoint& ViewCenter (void) { return m_viewCenter; }
	inline CPoint& ViewMax (void) { return m_viewMax; }

	void SetMouseState (int newMouseState);
	inline bool MouseState (int nMouseState) { return m_mouseState == nMouseState; }
	void RecordMousePos (CPoint& mousePos, CPoint point);
	BOOL SetCursor (HCURSOR hCursor);
//	void UpdateCursor (void);

	inline void Wrap (short& v, short delta, short min, short max) {
		v += delta;
		if (v > max)
			v = min;
		else if (v < min)
			v = max;
		}

	int FindNearestVertex (long xMouse, long yMouse);
	bool SelectCurrentSegment (long xMouse, long yMouse, int bAdd = 0);
	bool SelectCurrentSide (long xMouse, long yMouse, int bAdd = 0);
	bool SelectCurrentLine (long xMouse, long yMouse, int bAdd = 0);
	bool SelectCurrentPoint (long xMouse, long yMouse, int bAdd = 0);
	void SelectCurrentObject (long xMouse, long yMouse);
	bool SelectCurrentElement (long xMouse, long yMouse, int bAdd);
	void RefreshObject(short old_object, short new_object);
	void MarkRubberBandedVertices (void);
	BOOL DrawRubberBox ();
	void UpdateRubberRect (CPoint pt);
	void ResetRubberRect ();
	BOOL UpdateDragPos ();
	void HighlightDrag (short nVert, long x, long y);
	BOOL DrawDragPos (void);
	void FinishDrag (void);

	BOOL SetWindowPos (const CWnd *pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags);

	CRenderer& Renderer (void) { return *m_renderer; }
	void SetRenderer (int nRenderer);
	void SetPerspective (int nPerspective);

	inline int Perspective (void) { return Renderer ().Perspective (); }

	inline void Zoom (int nSteps, double zoom) { 
		Renderer ().Zoom (nSteps, zoom); 
		Refresh (false);
		}
	inline int Project (CRect* pRC = null, bool bCheckBehind = false) { return Renderer ().Project (pRC, bCheckBehind); } 
	inline void DrawFaceTextured (CFaceListEntry& fle) { Renderer ().DrawFaceTextured (fle); } 
	inline int FaceIsVisible (CSegment* segP, CSide* sideP) { return Renderer ().FaceIsVisible (segP, sideP); }
	inline void BeginRender (bool bOrtho = false) { Renderer ().BeginRender (bOrtho); }
	inline void EndRender (bool bSwapBuffers = false) { Renderer ().EndRender (bSwapBuffers); } 
	inline int ZoomIn (int nSteps = 1, bool bSlow = false) { 
		if (!Renderer ().ZoomIn (nSteps, bSlow))
			return 0;
		Refresh ();
		return 1;
		} 
	inline int ZoomOut (int nSteps = 1, bool bSlow = false) { 
		if (!Renderer ().ZoomOut (nSteps, bSlow))
			return 0;
		Refresh ();
		return 1;
		}
	inline CViewMatrix* ViewMatrix (void) { return Renderer ().ViewMatrix (); } 
	inline void SetCenter (CVertex v, int nType) { 
		Renderer ().SetCenter (v, nType); 
		//FitToView ();
		}

	inline void Pan (char direction, double offset) { 
		Renderer ().Pan (direction, offset); 
		Refresh (false);
		}
	inline void Reset (void);

	inline CDoubleVector& Center (void) { return m_renderData.m_vCenter; }
	inline CDoubleVector& Translation (void) { return m_renderData.m_vTranslate; }
	inline CDoubleVector& Scale (void) { return m_renderData.m_vScale; }
	inline CDoubleVector& Rotation (void) { return m_renderData.m_vRotate; }
	inline int& ViewWidth (void) { return m_renderData.m_viewWidth; }
	inline int& ViewHeight (void) { return m_renderData.m_viewHeight; }
	inline int& ViewDepth (void) { return m_renderData.m_viewDepth; }
	inline double DepthScale (void) { return ViewMatrix ()->DepthScale (); }
	//inline void SetDepthScale (double scale) { return ViewMatrix ()->SetDepthScale (scale); }
	inline void SetDepthScale (int i) { return ViewMatrix ()->SetDepthScale (i); }
	inline int DepthPerception (void) { return ViewMatrix ()->DepthPerception (); }
	inline CBGR* RenderBuffer (void) { return m_renderData.m_renderBuffer; }
	inline depthType* DepthBuffer (void) { return m_renderData.m_depthBuffer; }
	inline CBGR& RenderBuffer (int i) { return m_renderData.m_renderBuffer [i]; }
	inline depthType& DepthBuffer (int i) { return m_renderData.m_depthBuffer [i]; }
	inline void SetDepthBuffer (depthType* buffer) { m_renderData.m_depthBuffer = buffer; }
	inline CPen* Pen (ePenColor nPen, int nWeight = 1) { return (nPen < penCount) ? m_renderData.m_pens [nWeight > 1][nPen] : null; }
	inline CVertex& MinViewPoint (void) { return m_renderData.m_minViewPoint; }
	inline CVertex& MaxViewPoint (void) { return m_renderData.m_maxViewPoint; }
	inline bool IgnoreDepth (void) { return m_renderData.m_bIgnoreDepth; }
	inline void SetIgnoreDepth (bool bIgnoreDepth) { m_renderData.m_bIgnoreDepth = bIgnoreDepth; }
	inline bool DepthTest (void) { return m_renderData.m_bDepthTest; }
	inline void SetDepthTest (bool bDepthTest) { m_renderData.m_bDepthTest = bDepthTest; }
	inline int RenderIllumination (void) { return (m_renderData.m_viewMineFlags & eViewMineIllumination) != 0; }
	inline int RenderVariableLights (void) { return (m_renderData.m_viewMineFlags & eViewMineVariableLights) != 0; }
	inline ubyte Alpha (void) { return m_renderData.m_alpha; }
	inline void SetAlpha (ubyte alpha) { m_renderData.m_alpha = alpha; }
	inline uint& ViewMineFlags (void) { return m_renderData.m_viewMineFlags; }
	inline uint& ViewObjectFlags (void) { return m_renderData.m_viewObjectFlags; }
	inline double MineMoveRate (void) { return m_renderData.m_moveRate [0]; }
	inline double ViewMoveRate (void) { return m_renderData.m_moveRate [1]; }
	inline void SetMineMoveRate (double moveRate) { m_renderData.m_moveRate [0] = moveRate; }
	inline void SetViewMoveRate (double moveRate) { m_renderData.m_moveRate [1] = moveRate; }
	inline void GetMoveRates (double* moveRates) { 
		moveRates [0] = m_renderData.m_moveRate [0], 
		moveRates [1] = m_renderData.m_moveRate [1]; 
		}
	inline void SetMoveRates (double* moveRates) { 
		m_renderData.m_moveRate [0] = Clamp (moveRates [0], 0.001, 1000.0), 
		m_renderData.m_moveRate [1] = Clamp (moveRates [1], 0.001, 1000.0);
		}

	inline CDC* DC (void) { return Renderer ().DC (); }

private:
	short FindSelectedTexturedSide (long xMouse, long yMouse, short& nSide);

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CMineView)
	afx_msg void OnDestroy ();
	afx_msg void OnTimer (UINT_PTR nIdEvent);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnPaint ();
	afx_msg BOOL OnMouseWheel (UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnSelectPrevTab ();
	afx_msg void OnSelectNextTab ();
	afx_msg void OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
	afx_msg void OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in dlcView.cpp
inline CDlcDoc* CMineView::GetDocument()
   { return (CDlcDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif //__mineview_h
