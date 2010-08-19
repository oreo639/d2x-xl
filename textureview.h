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

                        /*--------------------------*/

class CTextureView : public CWnd {
	public:
// structs
		DECLARE_DYNCREATE(CTextureView)

		INT32			*m_pTextures;
		INT32			m_nTextures [2];
		CPen			*m_penCyan;
		CSize			m_iconSize;
		CSize			m_iconSpace;
		CSize			m_viewSpace;
		INT32			m_nRows [2];
		UINT32		m_viewFlags;
		BOOL			m_bShowAll;
		bool			m_bDelayRefresh;
		INT16			m_mapTxtToView [MAX_D2_TEXTURES];
		INT16			m_mapViewToTxt [MAX_D2_TEXTURES];
		UINT32		m_nTxtFilter;

		CTextureView ();
		~CTextureView ();
		void Reset ();
		void QSortTxtMap (INT16 left, INT16 right);
		void CrtTxtMap (void);
		UINT32 TextureFilter (INT16 nTxt);
		INT16 TexFilterIndex (INT16 nTxt);
		INT32 QCmpTxtFilters (INT32 nTxt, INT32 mTxt, UINT32 mf, UINT32 mf2);
//		CTreeView	m_treeView;
//		afx_msg INT32 CTextureView::OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg BOOL OnEraseBkgnd (CDC* pDC);
		afx_msg void OnPaint ();
		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
		afx_msg void OnVScroll(UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar);
		afx_msg BOOL OnMouseWheel (UINT nFlags, INT16 zDelta, CPoint pt);
		void Setup ();
		void RecalcLayout ();
		void FilterTextures(UINT8 *show_texture,BOOL show_all_textures);
		INT32 TextureIndex(INT16 nBaseTex);
		INT32 PickTexture(CPoint &point,INT16 &nBaseTex);
//		INT32 ReadTextureFromFile(INT16 texture_number,UINT8 *bitmap_array);
//		INT32 ReadPog(FILE *file);
		void Refresh (bool bRepaint = true);
		void ToggleViewFlag(eMineViewFlags flag) {
			m_viewFlags ^= flag;
			Refresh ();
			}
		void SetViewFlags (UINT32 flags) {
			if (m_viewFlags != flags) {
				m_viewFlags = flags; 
				Refresh ();
				}
			}
		inline UINT32 GetViewFlags ()
			{ return m_viewFlags; }
		inline UINT32& TextureFilter (void)
			{ return m_nTxtFilter; }
		inline bool ShowAll ()
			{ return m_bShowAll && (m_nTxtFilter == 0xFFFFFFFF); }
	DECLARE_MESSAGE_MAP()
};
                        
#endif //__textureview_h
