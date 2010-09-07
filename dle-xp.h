// dle-xp.h : main header file for the DLC application
//

#ifndef __dlc_h
#define __dlc_h

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <windows.h>

#include "dle-xp-res.h"       // main symbols
#include "dlc_i.h"
#include "mainfrm.h"
#include "mine.h"

extern int nLayout;

/////////////////////////////////////////////////////////////////////////////
// CDlcApp:
// See dlc.cpp for the implementation of this class
//

class CDlcApp : public CWinApp
{
public:
	CDlcDocTemplate	*m_pDlcDoc;
	char					m_szCaption [256];
	char					m_szExtCaption [256];
	int					m_delayUndo;
	int					m_nModified;
	BOOL					m_bSplashScreen;
	bool					m_bMaximized;

	CDlcApp();
	inline CMainFrame *MainFrame ()
		{ return (CMainFrame *) m_pMainWnd; }
	inline CMineView *MineView ()
		{ CMainFrame *h; return (h = MainFrame ()) ? h->MineView () : null; }
	inline CTextureView *TextureView ()
		{ CMainFrame* h; return (h = MainFrame ()) ? h->TextureView () : null; }
	inline CToolView *ToolView ()
		{ CMainFrame* h; return (h = MainFrame ()) ? MainFrame ()->ToolView () : null; }
	inline CDlcDoc *GetDocument ()
		{ CMineView *h; return (h = MineView ()) ? h->GetDocument () : null; }
	inline int FileType (void) 
		{ return theMine ? theMine->FileType (): RL2_FILE; }
	inline int LevelVersion (void) 
		{ return theMine ? theMine->LevelVersion () : 7; }
	inline bool IsD1File (void) { return FileType () == RDL_FILE; }
	inline bool IsD2File (void) { return FileType () != RDL_FILE; }
	inline bool IsD2XFile (void) { return (FileType () != RDL_FILE) && (LevelVersion () >= 12); }
	inline CWnd *TexturePane ()
		{ return MainFrame ()->TexturePane (); }
	inline CWnd *MinePane ()
		{ return MainFrame ()->MinePane (); }
	inline CWnd *ToolPane ()
		{ return MainFrame ()->ToolPane (); }
	inline CSize& ToolSize ()
		{ return ToolView ()->ToolSize (); }
	bool SetModified (BOOL bModified);
	void Unroll (bool bRevertUndo);
	CDocument* CDlcApp::OpenDocumentFile(LPCTSTR lpszFileName);
	void WritePrivateProfileInt (LPSTR szKey, int nValue);
	void SaveLayout ();
	void LoadLayout ();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlcApp)
	public:
	virtual BOOL InitInstance();
		virtual int ExitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//COleTemplateServer m_server;
		// Server object for document creation

	//{{AFX_MSG(CDlcApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_bATLInited;
private:
	BOOL InitATL();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

extern CDlcApp	DLE;

#endif //__dlc_h
