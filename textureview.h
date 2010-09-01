// dlcView.h : interface of the CMineView class
//
/////////////////////////////////////////////////////////////////////////////

#ifndef __textureview_h
#define __textureview_h

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "afxcview.h"
#include "DlcDoc.h"
#include "Matrix.h"
#include "mineview.h"
#include "poly.h"
#include "texturefilter.h"

                        /*--------------------------*/

class CTextureView : public CWnd {
	public:
// structs
		DECLARE_DYNCREATE(CTextureView)

		int				*m_pTextures;
		int				m_nTextures [2];
		CPen				*m_penCyan;
		CSize				m_iconSize;
		CSize				m_iconSpace;
		CSize				m_viewSpace;
		int				m_nRows [2];
		uint				m_viewFlags;
		BOOL				m_bShowAll;
		bool				m_bDelayRefresh;
		CTextureFilter	m_filter;

		CTextureView ();
		~CTextureView ();
		void Reset ();
		void QSortTexMap (short left, short right);
		void CreateTexMap (void);
		int QCmpTexFilters (int nTxt, int mTxt, uint mf, uint mf2);
//		CTreeView	m_treeView;
//		afx_msg int CTextureView::OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg BOOL OnEraseBkgnd (CDC* pDC);
		afx_msg void OnPaint ();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnVScroll(UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		afx_msg BOOL OnMouseWheel (UINT nFlags, short zDelta, CPoint pt);
		void Setup ();
		void RecalcLayout ();
		int PickTexture(CPoint &point,short &nBaseTex);
//		int ReadTextureFromFile(short texture_number,byte *bitmap_array);
//		int ReadPog(CFileManager& file);
		void Refresh (bool bRepaint = true);
		void ToggleViewFlag(eMineViewFlags flag) {
			m_viewFlags ^= flag;
			Refresh ();
			}
		void SetViewFlags (uint flags) {
			if (m_viewFlags != flags) {
				m_viewFlags = flags; 
				Refresh ();
				}
			}
		inline uint GetViewFlags ()
			{ return m_viewFlags; }
		inline bool ShowAll ()
			{ return m_bShowAll && (m_filter.Filter () == 0xFFFFFFFF); }
	DECLARE_MESSAGE_MAP()
};
                        
#endif //__textureview_h
