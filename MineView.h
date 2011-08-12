// dlcView.h : interface of the CMineView class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __mineview_h
#define __mineview_h

#ifdef NDEBUG
# define OGL_RENDERING 0
#else
# define OGL_RENDERING 0
#endif

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxcview.h"
#include "DlcDoc.h"
#include "Matrix.h"
#include "PolyModel.h"
#include "Textures.h"

#if OGL_RENDERING
# include <gl\gl.h>
# include <gl\glu.h>
# include <gl\glaux.h>
#endif

// -----------------------------------------------------------------------------

enum eObjectViewFlags {
	eViewObjectsNone          = 0,
	eViewObjectsRobots        = (1 << 0),
	eViewObjectsPlayers		  = (1 << 1),
	eViewObjectsWeapons       = (1 << 2),
	eViewObjectsPowerups      = (1 << 3),
	eViewObjectsKeys          = (1 << 4),
	eViewObjectsHostages      = (1 << 5),
	eViewObjectsControlCenter = (1 << 6),
	eViewObjectsEffects       = (1 << 7),
	eViewObjectsAll           = 0xff
	};

// -----------------------------------------------------------------------------

enum eMineViewFlags {
	eViewMineWalls            = (1 << 0),
	eViewMineSpecial          = (1 << 1),
	eViewMineLights           = (1 << 2),
	eViewMineShading          = (1 << 3),
	eViewMineDeltaLights		  = (1 << 4),
	eViewMineUsedTextures	  = (1 << 5),
	eViewMineSkyBox			  = (1 << 6)
	};

// -----------------------------------------------------------------------------

enum eViewOptions {
	eViewAllLines = 0,
	eViewHideLines,
	eViewNearbyCubeLines,
	eViewPartialLines,
	eViewTextureMapped
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

class CFaceListEntry : public CSideKey {
	public:
		long	m_zMin, m_zMax;
		bool	m_bTransparent;
	};

// -----------------------------------------------------------------------------

#if 1
typedef float depthType;
#define MAX_DEPTH 1e30f
#else
typedef long depthType;
#define MAX_DEPTH 0x7FFFFFFF
#endif

class CMineView : public CView
{
protected: // create from serialization only
	CMineView();
	DECLARE_DYNCREATE(CMineView)

	CSplitterWnd	*m_pSplitter;
	// member variables
	int			m_viewHeight;	// in pixels
	int			m_viewWidth;	// in pixels
	int			m_viewDepth;	// in bytes
	HBITMAP		m_DIB;
	CBGR*			m_renderBuffer;
	depthType*	m_depthBuffer;
	CDC			m_DC;
	CDC			*m_pDC; // if all goes well, this is set to &m_DC
	bool			m_bUpdate;
	bool			m_bUpdateCursor;
	bool			m_bDelayRefresh;
	int			m_nDelayRefresh;
	uint			m_viewObjectFlags;
	uint			m_viewMineFlags;
	uint			m_viewOption;
	uint			m_selectMode;
	HCURSOR		m_hCursors [eMouseStateCount];

	CPen*			m_penCyan;
	CPen*			m_penRed;
	CPen*			m_penMedRed;
	CPen*			m_penGray;
	CPen*			m_penGreen;
	CPen*			m_penDkGreen;
	CPen*			m_penDkCyan;
	CPen*			m_penYellow;
	CPen*			m_penOrange;
	CPen*			m_penBlue;
	CPen*			m_penMedBlue;
	CPen*			m_penLtBlue;
	CPen*			m_penLtGray;
	CPen*			m_penMagenta;
	CPen*			m_penHiCyan;
	CPen*			m_penHiRed;
	CPen*			m_penHiGray;
	CPen*			m_penHiGreen;
	CPen*			m_penHiDkGreen;
	CPen*			m_penHiDkCyan;
	CPen*			m_penHiYellow;
	CPen*			m_penHiOrange;
	CPen*			m_penHiBlue;
	CPen*			m_penHiLtGray;
	CPen*			m_penHiMagenta;

	//short			m_x0;
	//short			m_y0;
	//short			m_z0;
	CDoubleVector	m_center;
	CDoubleVector	m_move;
	CDoubleVector	m_size;
	CDoubleVector	m_spin;
	double			m_depthPerception;
	CViewMatrix		m_view;

	CDoubleMatrix	m_mat, m_invMat;
	//double		M[4][4];  /* 4x4 matrix used in coordinate transformation */
	//double		IM[4][4]; /* inverse matrix of M[4][4] */
//	double		depthPerception;
	tLongVector		m_viewPoints [VERTEX_LIMIT];
	tLongVector		m_minViewPoint;
	tLongVector		m_maxViewPoint;
	tLongVector		m_minVPIdx;
	tLongVector		m_maxVPIdx;

	int			m_mouseState;
	int			m_lastMouseState;
	CPoint		m_lastMousePos;
	CPoint		m_clickPos, 
					m_releasePos,
					m_lastDragPos,
					m_highlightPos;
	UINT			m_clickState,
					m_releaseState;
	short			m_lastSegment;
	CRect			m_rubberRect;
	UINT_PTR		m_lightTimer;
	UINT_PTR		m_selectTimer;
	int			m_nFrameRate;
	int			m_bShowLightSource;
	bool			m_bHScroll,
					m_bVScroll;
	int			m_xScrollRange,
					m_yScrollRange;
	int			m_xScrollCenter,
					m_yScrollCenter;
	int			m_xRenderOffs,
					m_yRenderOffs;
	int			m_nViewDist;
	int			m_nMineCenter;

	bool			m_bIgnoreDepth;
	bool			m_bTestDepth;
	byte			m_alpha;

	tLongVector		m_screenCoord [4];
	tLongVector		m_texCoord [4];
	int			m_x0, m_x1, m_y;
	double		m_z0, m_z1;

#if OGL_RENDERING
	HGLRC           m_glRC; // Permanent Rendering Context
	CDC             *m_glDC; // Private GDI Device Context
#endif
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
	void Reset ();
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
	void DrawMineCenter (CDC *pViewDC);
	bool VertexVisible (int v);
	int SetViewPoints (CRect *pRC = null, bool bSetViewInfo = true);
	void ShiftViewPoints ();
	// drawing functions
	void	InitView(CDC* pViewDC);
	bool	UpdateScrollBars (void);
	void	ClearView();
	void	ResetView (bool bRefresh = false);
	bool	InitViewDimensions (void);
	void	DrawWireFrame (bool bPartial);
	void	DrawSegmentsTextured (void);
	void	DrawMarkedSegments (short clear_it = 0);
	void	DrawSegment (CSegment *segP, bool bPartial);
	void	DrawSegment (short nSegment,short nSide, short linenum, short pointnum, short clear_it = 0);
	void	DrawSegmentPartial (CSegment *segP);
	void	DrawSegmentQuick (CSegment *segP, bool bPartial = false, char bTunnel = 0);
	void	DrawFaceTextured (CFaceListEntry& fle);
	void	DrawCubePoints (CSegment *segP);

	void	DrawCurrentSegment (CSegment *segP, bool bPartial);
	void	DrawLine (CSegment *segP, short vert1, short vert2);
	void	DrawLine (CTexture *pTx, POINT pt0, POINT pt1, CBGRA color);
	void	DrawAnimDirArrows (short texture1, CTexture *pTx);

	void	DrawWalls (void);
	void	DrawLights (void);
	void	DrawOctagon(short nSide, short nSegment);
	void	DrawObject (short objnum, short clear_it = 0);
	void	DrawObjects (short clear_it = 0);
	void	DrawHighlight (short clear_it = 0);
	void  DrawTunnel (void);

	//void	ReadPolyModel (tPolyModel& polyModel, CFileManager* file);

	// view control functions
	void	Zoom (int nSteps, double zoom);
	int	ZoomFactor (int nSteps, double min, double max);
	int	ZoomIn (int nSteps = 1, bool bSlow = false);
	int	ZoomOut (int nSteps = 1, bool bSlow = false);
	int	FitToView (void);
	void	Rotate (char direction, double angle);
	void	Pan (char direction, int amount);
	void	AlignSide ();
	void MarkVisibleVerts (bool bReset = false);
	void	CenterMine ();
	void	CenterSegment ();
	void	CenterObject ();
	void	SetViewOption (eViewOptions option);
	void	ToggleViewMine (eMineViewFlags flag);
	void	ToggleViewObjects (eObjectViewFlags mask);
	void	SetViewMineFlags (uint mask);
	void	SetViewObjectFlags (uint mask);
	void	SetSelectMode (uint mode);
	void CalcSegDist (void);
	bool	InRange (short *pv, short i);

	void NextPoint (int dir = 1);
	void PrevPoint ();
	void NextLine (int dir = 1);
	void PrevLine ();
	void NextSide (int dir = 1);
	void PrevSide ();
	void NextSide2 (int dir = 1);
	void PrevSide2 ();
	void NextCube (int dir = 1);
	void PrevCube ();
	void ForwardCube (int dir = 1);
	void BackwardsCube ();
	void SelectOtherSegment ();
	bool SelectOtherSide ();
	void NextObject (int dir = 1);
	void PrevObject ();
	void NextCubeElement (int dir = 1);
	void PrevCubeElement ();
	void HiliteTarget (void);

	void Refresh (bool bAll = true);
	void EnableDeltaShading (int bEnable, int nFrameRate, int bShowLightSource);
	void AdvanceLightTick (void);
	bool SetLightStatus (void);
	void Invalidate (BOOL bErase);
	void InvalidateRect (LPCRECT lpRect, BOOL bErase);

	bool ViewObject (CGameObject *objP);
	inline bool ViewObject (uint flag = 0)
		{ return flag ? ((m_viewObjectFlags & flag) != 0) : (m_viewObjectFlags != 0); }
	inline bool ViewFlag (uint flag = 0)
		{ return flag ? (m_viewMineFlags & flag) != 0 : (m_viewMineFlags != 0); }
	inline bool ViewOption (uint option)
		{ return m_viewOption == option; }
	inline bool SelectMode (uint mode)
		{ return m_selectMode == mode; }
	inline uint GetMineViewFlags ()
		{ return m_viewMineFlags; }
	inline uint GetObjectViewFlags ()
		{ return m_viewObjectFlags; }
	inline uint GetViewOptions ()
		{ return m_viewOption; }
	inline uint GetSelectMode ()
		{ return m_selectMode; }
	inline int *MineCenter (void)
		{ return &m_nMineCenter; }
	inline double &DepthPerception (void)
		{ return m_depthPerception; }
	inline void DelayRefresh (bool bDelay) {
		if (bDelay)
			m_nDelayRefresh++;
		else if (m_nDelayRefresh > 0)
			m_nDelayRefresh--;
		}
	inline bool DelayRefresh (void) { return m_nDelayRefresh > 0; }
	inline bool DepthTest (void) { return m_bTestDepth; }
	inline void SetDepthTest (bool bDepthTest) { m_bTestDepth = bDepthTest; }

	void SetMouseState (int newMouseState);
	BOOL SetCursor (HCURSOR hCursor);
//	void UpdateCursor (void);

	inline void Wrap (short& v, short delta, short min, short max) {
		v += delta;
		if (v > max)
			v = min;
		else if (v < min)
			v = max;
		}

	bool SelectCurrentSegment(short direction,long mouse_x, long mouse_y);
	void SelectCurrentObject(long mouse_x, long mouse_y);
	void CalcSegmentCenter (CVertex& pos, short nSegment);
	void RefreshObject(short old_object, short new_object);
	void MarkRubberBandedVertices (void);
	BOOL DrawRubberBox ();
	void UpdateRubberRect (CPoint pt);
	void ResetRubberRect ();
	BOOL UpdateDragPos ();
	void HighlightDrag (short nVert, long x, long y);
	BOOL CMineView::DrawDragPos (void);
	void FinishDrag (void);

	BOOL SetWindowPos(const CWnd *pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags);

	void RenderFace (CSegment* segP, short nSide, CTexture& tex, ushort rowOffset, CBGRA* colorP = null);

	inline bool Blend (CBGR& dest, CBGRA& src, depthType& depth, depthType z, short brightness = 32767);

	inline depthType Z (CTexture& tex, tLongVector* a, int x, int y);

	inline double ZRange (int x0, int x1, int y, double& z);

#if OGL_RENDERING
	BOOL GLInit (GLvoid);
	BOOL GLInitPalette (GLvoid);
	GLvoid GLReset (GLvoid);
	GLvoid GLFitToView (GLvoid);
	BOOL GLResizeScene (GLvoid);
	BOOL GLRenderScene (GLvoid);
	void GLRenderFace (short nSegment, short nSide);
	void GLRenderTexture (short nSegment, short nSide, short nTexture);
	void GLCreateTexture (short nTexture);
	GLvoid GLKillWindow (GLvoid);
	BOOL GLCreateWindow (CDC *pDC = null);
#endif

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

#endif //__mineview_h
