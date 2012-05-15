// dlc.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "dle-xp.h"

#include "MainFrame.h"
#include "dlcDoc.h"
#include "mineview.h"
#include <initguid.h>
#include "Dlc_i.c"
//#include "ComMine.h"
//#include "ComCube.h"
//#include "ComObj.h"
#include "global.h"
#include "TextureManager.h"
#include "FileManager.h"
#ifdef _OPENMP
#	include "omp.h"
#endif

int nLayout = 1;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	UINT_PTR m_nTimer;
	int		m_nTimeout;

	CAboutDlg(int m_nTimeout = 0);

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   virtual BOOL OnInitDialog ();
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	afx_msg void OnLButtonDown (UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown (UINT nFlags, CPoint point);
	afx_msg void OnTimer (UINT_PTR nIdEvent);
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg(int nTimeout) : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_nTimer = -1;
	m_nTimeout = nTimeout;
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
		ON_WM_LBUTTONDOWN ()
		ON_WM_RBUTTONDOWN ()
		ON_WM_TIMER ()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CDLE::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

BOOL CAboutDlg::OnInitDialog ()
{
CDialog::OnInitDialog ();
if (m_nTimeout)
	m_nTimer = SetTimer (4, (UINT) 100, null);
return TRUE;
}

void CAboutDlg::OnLButtonDown (UINT nFlags, CPoint point)
{
EndDialog (1);
}

void CAboutDlg::OnRButtonDown (UINT nFlags, CPoint point)
{
EndDialog (1);
}

void CAboutDlg::OnTimer (UINT_PTR nIdEvent)
{
if (m_nTimer == (int) nIdEvent) {
	m_nTimeout--;
	if (m_nTimeout <= 0) {
		KillTimer (m_nTimer);
		m_nTimer = -1;
		EndDialog (1);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDLE

BEGIN_MESSAGE_MAP(CDLE, CWinApp)
	//{{AFX_MSG_MAP(CDLE)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_NEWFILE, CWinApp::OnFileNew)
	ON_COMMAND(ID_OPENFILE, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDLE construction

CDLE::CDLE()
{
m_pDlcDoc = null;
m_bSplashScreen = 1;
m_bMaximized = false;
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDLE object

CDLE DLE;


// {3F315842-67AC-11d2-AE2A-00C0F03014A5}
static const CLSID clsid  = 
{ 0x3f315842, 0x67ac, 0x11d2, { 0xae, 0x2a, 0x0, 0xc0, 0xf0, 0x30, 0x14, 0xa5 } };

/////////////////////////////////////////////////////////////////////////////
// CDLE initialization

#ifdef _DEBUG

LPCTSTR PaletteResource (void);

void DLE_XP_invalid_parameter(
   const wchar_t * expression,
   const wchar_t * function, 
   const wchar_t * file, 
   uint line,
   uintptr_t pReserved)
{
errno = EINVAL;
}

#endif

//------------------------------------------------------------------------------

void SetupOpenMP (void)
{
#ifdef _OPENMP
int nThreads = omp_get_num_threads ();
if (nThreads < 2)
#pragma omp parallel 
	{
	nThreads = omp_get_max_threads ();
	}
if (nThreads > 4)
	omp_set_num_threads (4);
#endif
}

//------------------------------------------------------------------------------

BOOL CDLE::InitInstance()
{
#ifdef _DEBUG
	_set_invalid_parameter_handler (DLE_XP_invalid_parameter);
#endif
	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	if (!InitATL())
		return FALSE;

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
	// Change the registry key under which our settings are stored.
	// You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("DLE-XP"));

	LoadStdProfileSettings(10);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	m_pDlcDoc = new CDlcDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDlcDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CMineView));
	AddDocTemplate(m_pDlcDoc);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	//cmdInfo.m_nShellCommand = CCommandLineInfo::FileNew;
	if (!ProcessShellCommand (cmdInfo))
		return FALSE;

	// Dispatch commands specified on the command line
	SetupOpenMP ();
	if (theMine == null)
		theMine = new CMine;
	textureManager.Setup ();
	MineView ()->DelayRefresh (true);
	MainFrame ()->ToolView ()->Setup ();
	hogManager = new CHogManager (MainFrame ());
	theMine->Load ();
	if (!textureManager.Available ())
		ToolView ()->SetActive (11); // invoke preferences dialog
	TextureView ()->Setup ();
	MainFrame ()->SetSelectMode (eSelectSide);
	MainFrame ()->ShowWindow (SW_SHOW);
	MainFrame ()->GetWindowText (m_szCaption, sizeof (m_szCaption));
	MainFrame ()->FixToolBars ();
	LoadLayout ();
	MainFrame ()->ToolView ()->RecalcLayout (MainFrame ()->m_toolMode, MainFrame ()->m_textureMode);
	//MainFrame ()->ToolView ()->CalcToolSize ();
	MainFrame ()->UpdateWindow ();
	if (m_bSplashScreen || DEMO) {
		CAboutDlg aboutDlg (100);
		aboutDlg.DoModal ();
		}
	if (*cmdInfo.m_strFileName)
		GetDocument ()->OpenFile (false, cmdInfo.m_strFileName.GetBuffer (256), null /*"*"*/);
	if (ToolView ()->SettingsTool ())
		ToolView ()->SettingsTool ()->SetAppSettings (-1);
	SetCurrentDirectory (missionFolder);
	MineView ()->DelayRefresh (false);
	//DLE.GetDocument ()->SetModifiedFlag (1); // allow saving right away
	//MineView ()->Refresh ();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDLE commands



/////////////////////////////////////////////////////////////////////////////
// CDLE message handlers
	
CDLCModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
#if 0
	OBJECT_ENTRY(CLSID_ComMine, CComMine)
	OBJECT_ENTRY(CLSID_ComCube, CComCube)
	OBJECT_ENTRY(CLSID_ComObj, CComObj)
#endif
END_OBJECT_MAP()

//------------------------------------------------------------------------------

LONG CDLCModule::Unlock()
{
	AfxOleUnlockApp();
	return 0;
}

//------------------------------------------------------------------------------

LONG CDLCModule::Lock()
{
	AfxOleLockApp();
	return 1;
}

//------------------------------------------------------------------------------

int CDLE::ExitInstance()
{
if (m_bATLInited) {
	_Module.RevokeClassObjects();
	_Module.Term();
	}
return CWinApp::ExitInstance();
}

//------------------------------------------------------------------------------

CDocument* CDLE::OpenDocumentFile (LPCTSTR lpszFileName)
{
	int	nAction = IDOK;

ASSERT(m_pDocManager != null);
//	GetDocument ()->SetPathName ("(new document)");
return CWinApp::OpenDocumentFile (lpszFileName);
}

//------------------------------------------------------------------------------

BOOL CDLE::InitATL()
{
m_bATLInited = TRUE;
_Module.Init(ObjectMap, AfxGetInstanceHandle());
_Module.dwThreadID = GetCurrentThreadId();
return TRUE;
}

//------------------------------------------------------------------------------

void CDLE::WritePrivateProfileInt (LPSTR szKey, int nValue)
{
	char	szValue [20];

sprintf_s (szValue, sizeof (szValue), "%d", nValue);
WritePrivateProfileString ("DLE-XP", szKey, szValue, INIFILE);
}

//------------------------------------------------------------------------------

void CDLE::SaveLayout ()
{
CHECKMINE;

	CRect	rc;

MainFrame ()->GetWindowRect (rc);
if (rc.left < 0)
	rc.left = 0;
if (rc.top < 0)
	rc.top = 0;
WritePrivateProfileInt ("xWin", rc.left);
WritePrivateProfileInt ("yWin", rc.top);
WritePrivateProfileInt ("cxWin", rc.Width ());
WritePrivateProfileInt ("cyWin", rc.Height ());
//MainFrame ()->ScreenToClient (rc);
MainFrame ()->m_toolBar.GetWindowRect (rc);
WritePrivateProfileInt ("xMainTB", rc.left);
WritePrivateProfileInt ("yMainTB", rc.top);
WritePrivateProfileInt ("cxMainTB", rc.Width ());
WritePrivateProfileInt ("cyMainTB", rc.Height ());
#if EDITBAR
MainFrame ()->m_editBar.GetWindowRect (rc);
WritePrivateProfileInt ("xEditTB", rc.left);
WritePrivateProfileInt ("yEditTB", rc.top);
WritePrivateProfileInt ("cxEditTB", rc.Width ());
WritePrivateProfileInt ("cyEditTB", rc.Height ());
#endif
WritePrivateProfileInt ("ToolMode", MainFrame ()->m_toolMode);
WritePrivateProfileInt ("TextureMode", MainFrame ()->m_textureMode);
WritePrivateProfileInt ("AutoFixBugs", ToolView ()->DiagTool ()->m_bAutoFixBugs);
WritePrivateProfileInt ("SplashScreen", m_bSplashScreen);
}

//------------------------------------------------------------------------------

void CDLE::LoadLayout ()
{
CHECKMINE;

	CRect	rc, tbrc;
	UINT h = AFX_IDW_DOCKBAR_TOP;

MainFrame ()->m_toolMode = GetPrivateProfileInt ("DLE-XP", "ToolMode", 1, INIFILE);
MainFrame ()->m_textureMode = GetPrivateProfileInt ("DLE-XP", "TextureMode", 1, INIFILE);
MainFrame ()->m_paneMode = ((MainFrame ()->m_toolMode == 2) && (MainFrame ()->m_textureMode == 2)) ? 2 : 0;
if (ToolView ()->DiagTool ())
	ToolView ()->DiagTool ()->m_bAutoFixBugs = GetPrivateProfileInt ("DLE-XP", "AutoFixBugs", 1, INIFILE);
rc.left = GetPrivateProfileInt ("DLE-XP", "xWin", 0, INIFILE);
rc.top = GetPrivateProfileInt ("DLE-XP", "yWin", 0, INIFILE);
rc.right = rc.left + GetPrivateProfileInt ("DLE-XP", "cxWin", 0, INIFILE);
rc.bottom = rc.top + GetPrivateProfileInt ("DLE-XP", "cyWin", 0, INIFILE);
if ((rc.left >= rc.right) || (rc.top >= rc.bottom) || 
	 (rc.bottom < 0) || (rc.right < 0) ||
	 (rc.left >= GetSystemMetrics (SM_CXSCREEN)) || (rc.top >= GetSystemMetrics (SM_CYSCREEN))) {
	rc.left = rc.top = 0;
	rc.right = GetSystemMetrics (SM_CXSCREEN);
	rc.bottom = GetSystemMetrics (SM_CYSCREEN);
	}
if (!rc.left && !rc.top && 
	 (rc.right >= GetSystemMetrics (SM_CXSCREEN)) && 
	 (rc.bottom >= GetSystemMetrics (SM_CYSCREEN)))
 	MainFrame ()->ShowWindow (SW_SHOWMAXIMIZED);
else
	MainFrame ()->MoveWindow (&rc, TRUE);

#if 0
tbrc.left = GetPrivateProfileInt ("DLE-XP", "xMainTB", 0, INIFILE);
if (tbrc.left < 0)
	tbrc.left = 0;
tbrc.top = GetPrivateProfileInt ("DLE-XP", "yMainTB", 0, INIFILE);
tbrc.right = tbrc.left + GetPrivateProfileInt ("DLE-XP", "cxMainTB", 0, INIFILE);
tbrc.bottom = tbrc.top + GetPrivateProfileInt ("DLE-XP", "cyMainTB", 0, INIFILE);
	if (tbrc.Width () > tbrc.Height ())	//horizontal
	if (tbrc.bottom >= rc.bottom - GetSystemMetrics (SM_CYFRAME))
		h = AFX_IDW_DOCKBAR_BOTTOM;
	else
		h = AFX_IDW_DOCKBAR_TOP;
else //vertical
	if (tbrc.right >= rc.right - GetSystemMetrics (SM_CXFRAME))
		h = AFX_IDW_DOCKBAR_RIGHT;
	else
		h = AFX_IDW_DOCKBAR_LEFT;
if (tbrc.Width () && tbrc.Height ())
	MainFrame ()->DockControlBar (&MainFrame ()->m_toolBar, (UINT) h, &tbrc);
#endif

tbrc.left = GetPrivateProfileInt ("DLE-XP", "xEditTB", 0, INIFILE);
if (tbrc.left < 0)
	tbrc.left = 0;
tbrc.top = GetPrivateProfileInt ("DLE-XP", "yEditTB", 0, INIFILE);
#if EDITBAR
tbrc.right = tbrc.left + GetPrivateProfileInt ("DLE-XP", "cxEditTB", 0, INIFILE);
tbrc.bottom = tbrc.top + GetPrivateProfileInt ("DLE-XP", "cyEditTB", 0, INIFILE);
if (tbrc.Width () > tbrc.Height ())	//horizontal
	if (tbrc.bottom >= rc.bottom - GetSystemMetrics (SM_CYFRAME))
		h = AFX_IDW_DOCKBAR_BOTTOM;
	else if (!tbrc.top)
		h = AFX_IDW_DOCKBAR_TOP;
else //vertical
	if (tbrc.right >= rc.right - GetSystemMetrics (SM_CXFRAME))
		h = AFX_IDW_DOCKBAR_RIGHT;
	else if (!tbrc.left)
		h = AFX_IDW_DOCKBAR_LEFT;
if (tbrc.Width () && tbrc.Height ()) {
	CPoint p;
	p.x = tbrc.left;
	p.y = tbrc.top;
	MainFrame ()->FloatControlBar (&MainFrame ()->m_editBar, p, (UINT) h);
	}
#endif
m_bSplashScreen = GetPrivateProfileInt ("DLE-XP", "SplashScreen", 1, INIFILE);
}

//------------------------------------------------------------------------------

bool CDLE::MakeModFolders (char* szSubFolder)
{
	char szMission [256], szLevel [256];

strcpy (szMission, hogManager->MissionName ());
char* pszMission = strrchr (szMission, '.');
if (pszMission)
	*pszMission = '\0';
if (!((pszMission = strrchr (szMission, '\\')) || (pszMission = strrchr (szMission, ':'))))
	pszMission = szMission;
sprintf (m_modFolders [2], "%s\\%s\\", modFolder, pszMission ? pszMission + 1 : pszMission);
sprintf (m_modFolders [1], "%s%s", m_modFolders [2], szSubFolder);

strcpy (szLevel, hogManager->LevelName ());
char* pszName = strrchr (szLevel, '.');
if (pszName)
	*pszName = '\0';
if (!*szLevel)
	return false;

int nLevel = DLE.ToolView ()->MissionTool ()->LevelNumber (szLevel);
if (!nLevel)
	return false;
sprintf (m_modFolders [0], "%s%s\\level%02d", m_modFolders [2], szSubFolder, nLevel);
strcat (m_modFolders [2], pszMission);
return true;
}

//------------------------------------------------------------------------------

void ArrayError (const char* pszMsg)
{
pszMsg = pszMsg;
ErrorMsg (pszMsg);
}

//------------------------------------------------------------------------------
//eof dlc.cpp