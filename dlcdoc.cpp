// dlcDoc.cpp : implementation of the CDlcDoc class
//

#include <process.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "stdafx.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "global.h"
#include "parser.h"
#include "FileManager.h"
#include "HogManager.h"
#include "light.h"
#include "textures.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CSaveFileDlg : public CDialog {
	public:
		char	m_name [256];
		LPSTR	m_lpszName;
		int	m_type;
		int	*m_pType;

		CSaveFileDlg (CWnd *pParentWnd)
			: CDialog (IDD_SAVELEVEL, pParentWnd) {};
		afx_msg void OnNo ()
			{ EndDialog (IDNO); }
	
	DECLARE_MESSAGE_MAP ()
	};

BEGIN_MESSAGE_MAP (CSaveFileDlg, CDialog)
	ON_BN_CLICKED (IDNO, OnNo)
END_MESSAGE_MAP ()

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc

IMPLEMENT_DYNCREATE(CDlcDoc, CDocument)

BEGIN_MESSAGE_MAP(CDlcDoc, CDocument)
	//{{AFX_MSG_MAP(CDlcDoc)
	ON_COMMAND(ID_OPENFILE, OnOpenFile)
	ON_COMMAND(ID_SAVEFILE, OnSaveFile)
	ON_COMMAND(ID_SAVEFILE_AS, OnSaveFileAs)
	ON_COMMAND(ID_RUN_LEVEL, OnRunLevel)
	ON_COMMAND(ID_INSERT_CUBE, OnInsertSegment)
	ON_COMMAND(ID_INSERT_CUBE_REACTOR, OnInsertSegReactor)
	ON_COMMAND(ID_INSERT_CUBE_ROBOTMAKER, OnInsertSegRobotMaker)
	ON_COMMAND(ID_INSERT_CUBE_FUELCENTER, OnInsertSegFuelCenter)
	ON_COMMAND(ID_INSERT_CUBE_REPAIRCENTER, OnInsertSegRepairCenter)
	ON_COMMAND(ID_INSERT_OBJECT_PLAYERCOPY, OnInsertObjectPlayerCopy)
	ON_COMMAND(ID_INSERT_OBJECT_PLAYER, OnInsertObjectPlayer)
	ON_COMMAND(ID_INSERT_OBJECT_ROBOT, OnInsertObjectRobot)
	ON_COMMAND(ID_INSERT_OBJECT_WEAPON, OnInsertObjectWeapon)
	ON_COMMAND(ID_INSERT_OBJECT_POWERUP, OnInsertObjectPowerup)
	ON_COMMAND(ID_INSERT_OBJECT_GUIDEBOT, OnInsertObjectGuideBot)
	ON_COMMAND(ID_INSERT_OBJECT_COOPPLAYER, OnInsertObjectCoopPlayer)
	ON_COMMAND(ID_INSERT_OBJECT_REACTOR, OnInsertObjectReactor)
	ON_COMMAND(ID_INSERT_DOOR_NORMAL, OnInsertDoorNormal)
	ON_COMMAND(ID_INSERT_DOOR_PRISON, OnInsertDoorPrison)
	ON_COMMAND(ID_INSERT_DOOR_SECRETEXIT, OnInsertDoorSecretExit)
	ON_COMMAND(ID_INSERT_DOOR_EXIT, OnInsertDoorExit)
	ON_COMMAND(ID_INSERT_DOOR_GUIDEBOTDOOR, OnInsertDoorGuideBot)
	ON_COMMAND(ID_INSERT_TRIGGER_OPENDOOR, OnInsertTriggerOpenDoor)
	ON_COMMAND(ID_INSERT_TRIGGER_ROBOTMAKER, OnInsertTriggerRobotMaker)
	ON_COMMAND(ID_INSERT_TRIGGER_SHIELDDRAIN, OnInsertTriggerShieldDrain)
	ON_COMMAND(ID_INSERT_TRIGGER_ENERGYDRAIN, OnInsertTriggerEnergyDrain)
	ON_COMMAND(ID_INSERT_TRIGGER_CONTROLPANEL, OnInsertTriggerControlPanel)
	ON_COMMAND(ID_INSERT_WALL_FUELCELLS, OnInsertWallFuelCells)
	ON_COMMAND(ID_INSERT_WALL_ILLUSION, OnInsertWallIllusion)
	ON_COMMAND(ID_INSERT_WALL_FORCEFIELD, OnInsertWallForceField)
	ON_COMMAND(ID_INSERT_WALL_FAN, OnInsertWallFan)
	ON_COMMAND(ID_INSERT_WALL_GRATE, OnInsertWallGrate)
	ON_COMMAND(ID_INSERT_WALL_WATERFALL, OnInsertWallWaterfall)
	ON_COMMAND(ID_INSERT_WALL_LAVAFALL, OnInsertWallLavafall)
	ON_COMMAND(ID_DELETE_CUBE, OnDeleteSegment)
	ON_COMMAND(ID_DELETE_OBJECT, OnDeleteObject)
	ON_COMMAND(ID_DELETE_WALL, OnDeleteWall)
	ON_COMMAND(ID_DELETE_TRIGGER, OnDeleteTrigger)
#if 0
	ON_COMMAND(ID_FILE_TEST, OnFileTest)
#endif
	ON_COMMAND(ID_EDIT_CUT, OnCutBlock)
	ON_COMMAND(ID_EDIT_COPY, OnCopyBlock)
	ON_COMMAND(ID_EDIT_QUICKCOPY, OnQuickCopyBlock)
	ON_COMMAND(ID_EDIT_PASTE, OnPasteBlock)
	ON_COMMAND(ID_EDIT_QUICKPASTE, OnQuickPasteBlock)
	ON_COMMAND(ID_EDIT_DELETEBLOCK, OnDeleteBlock)
	ON_COMMAND(ID_EDIT_COPYOTHERCUBESTEXTURES, OnCopyOtherCube)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CDlcDoc, CDocument)
	//{{AFX_DISPATCH_MAP(CAutoDoc)
	DISP_PROPERTY(CDlcDoc, "MemberLong1", Member1, VT_I4)
	DISP_FUNCTION(CDlcDoc, "TestLong", Test, VT_I4, VTS_NONE)
	DISP_FUNCTION(CDlcDoc, "SetMyText", SetMyText, VT_EMPTY, VTS_BSTR)
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IAuto to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {3F315844-67AC-11d2-AE2A-00C0F03014A5}
static const IID IID_Idlc =
{ 0x3f315844, 0x67ac, 0x11d2, { 0xae, 0x2a, 0x0, 0xc0, 0xf0, 0x30, 0x14, 0xa5 } };

BEGIN_INTERFACE_MAP(CDlcDoc, CDocument)
	INTERFACE_PART(CDlcDoc, IID_Idlc, Dispatch)
END_INTERFACE_MAP()

                        /*--------------------------*/

class CNewFileDlg : public CDialog {
	public:
		char	m_name [256];
		LPSTR	m_lpszName;
		int	m_type;
		int	*m_pType;

		CNewFileDlg (CWnd *pParentWnd, LPSTR lpszName, int *pType)
			: CDialog (IDD_NEWLEVEL, pParentWnd) {
			strcpy_s (m_name, sizeof (m_name), m_lpszName = lpszName);
			m_type = *(m_pType = pType);
			}
      virtual BOOL OnInitDialog () {
			CDialog::OnInitDialog ();
			if (theMine->m_bVertigo)
				GetDlgItem (IDC_D2VLEVEL)->EnableWindow (FALSE);
			return TRUE;
			}
		virtual void DoDataExchange (CDataExchange * pDX) { 
			DDX_Text (pDX, IDC_LEVELNAME, m_name, sizeof (m_name)); 
			DDX_Radio (pDX, IDC_D1LEVEL, m_type);
			}
		void OnOK (void) {
			UpdateData (TRUE);
			strcpy_s (m_lpszName, 256, m_name); 
			*m_pType = m_type;
			EndDialog (IDOK);
			}
	};

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc construction/destruction

CDlcDoc::CDlcDoc()
{
//theMine = new CMine;
//theMine->Initialize ();
//theMine->Default ();
m_bInitDocument = true;
*m_szFile = '\0';
*m_szSubFile = '\0';
memset (&missionData, 0, sizeof (missionData));
}

CDlcDoc::~CDlcDoc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc serialization

void CDlcDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc diagnostics

#ifdef _DEBUG
void CDlcDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDlcDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDlcDoc commands

BOOL CDlcDoc::OnNewDocument()
{
if (!CDocument::OnNewDocument())
	return FALSE;
*m_szFile = '\0';
*m_szSubFile = '\0';

if (theMine) {
#if 0
	if (m_bInitDocument) {
		m_bInitDocument = false;
		theMine->Load ();
		}
	else
#endif
		CreateNewLevel ();
	}
return TRUE;
}

                        /*--------------------------*/

void CDlcDoc::CreateNewLevel ()
{
char	newLevelName [256];
int	newFileType = 1;
strcpy_s (newLevelName, sizeof (newLevelName), "(untitled)");

CNewFileDlg	d (DLE.MainFrame (), newLevelName, &newFileType);
if (d.DoModal () == IDOK) {
	theMine->Default ();
	DLE.MineView ()->Reset ();
	DLE.TextureView ()->Reset ();
	DLE.ToolView ()->Reset ();
//		InitRobotData();

	*m_szFile = '\0';
	theMine->SetFileType (newFileType);
	switch (newFileType) {
		case 0:
			theMine->SetFileType (0);
			break;
		case 1:
		case 2:
		case 3:
			theMine->SetFileType (1);
			break;
		}
	theMine->Load ();
	switch (newFileType) {
		case 0:
			theMine->SetLevelVersion (1);
			break;
		case 1:
			theMine->SetLevelVersion (7);
			break;
		case 2:
			theMine->SetLevelVersion (8);
			break;
		case 3:
			theMine->UpdateLevelVersion ();
			segmentManager.UpdateWalls (MAX_WALLS_D2, WALL_LIMIT
				);
		}
	*m_szSubFile = '\0';
	strcpy_s (theMine->LevelName (), theMine->LevelNameSize (), newLevelName);
	theMine->Reset ();
	segmentManager.SetLinesToDraw ();
	DLE.MineView ()->ResetView (true);
	memset (&missionData, 0, sizeof (missionData));
	lightManager.CreateLightMap ();
	DLE.TextureView ()->Setup ();
	DLE.ToolView ()->TextureTool ()->LoadTextureListBoxes ();
	DLE.ToolView ()->MissionTool ()->Refresh ();
	}
}

                        /*--------------------------*/

bool CDlcDoc::BrowseForFile (LPSTR pszFile, BOOL bOpen)
{
return ::BrowseForFile (bOpen, "hog;rl2;rdl", pszFile, 
								"all levels (*.hog;*.rl2;*.rdl)|*.hog;*.rl2;*.rdl"
								"|mission files (*.hog)|*.hog|"
								"|Descent 2 level (*.rl2)|*.rl2|"
								"Descent 1 level (*.rdl)|*.rdl||", 
								0, DLE.MainFrame ());
}

                        /*--------------------------*/

void CDlcDoc::UpdateCaption ()
{
SetPathName (m_szFile);
}

                        /*--------------------------*/

bool CDlcDoc::SaveIfModified (void)
{
	INT_PTR	nAction;

if (!IsModified ())
	return true;

CSaveFileDlg d (DLE.MainFrame ());

nAction = d.DoModal (); //AfxMessageBox ("\nThe mine has been modified.\n\nClick 'Yes' to load another mine and loose all changes,\n'No' to save changes before loading another mine,\nor 'Cancel' to keep this mine and return without saving.", MB_YESNOCANCEL | MB_ICONEXCLAMATION);
if (nAction == IDCANCEL)
	return false;
undoManager.Begin (FALSE);
// buggy for new mine; int required
if (nAction == IDNO) 
	SaveFile (*GetPathName () == '\0');
return true;
}

                        /*--------------------------*/

BOOL CDlcDoc::OpenFile (bool bBrowseForFile, LPSTR pszFile, LPSTR pszSubFile) 
{
	int err = 0;
	char szFile [256], szSubFile [256];

if (bEnableDeltaShading)
	DLE.ToolView ()->LightTool ()->OnShowDelta ();
if (!SaveIfModified ())
	return FALSE;
if (bBrowseForFile && !BrowseForFile (m_szFile, TRUE))
	return FALSE;
if (DLE.ToolView () && DLE.ToolView ()->DiagTool ())
	DLE.ToolView ()->DiagTool ()->Reset ();
if (!pszFile)
	pszFile = m_szFile;
if (!pszSubFile)
	pszSubFile = m_szSubFile;
_strlwr_s (pszFile, 256);
strcpy_s (szFile, sizeof (szFile), pszFile);
strcpy_s (szSubFile, sizeof (szSubFile), pszSubFile);
lightManager.CreateLightMap ();
if (strstr (pszFile, ".hog")) {
	CHogManager	hm (DLE.MainFrame (), szFile, szSubFile);
	if (pszSubFile != m_szSubFile) {
		if (!hm.LoadLevel (szFile, szSubFile))
			return FALSE;
		}
	else {
		if (hm.DoModal () != IDOK)
			return FALSE;
		}
	if (pszFile != m_szFile)
		strcpy_s (m_szFile, sizeof (m_szFile), szFile);
	strcpy_s (m_szSubFile, sizeof (m_szSubFile), szSubFile);
	CFileManager::SplitPath (pszFile, m_startFolder , null, null);
	sprintf_s (m_szTmpFile, sizeof (m_szTmpFile), "%sdle_temp.rdl", m_startFolder);
	err = theMine->Load (m_szTmpFile, true);
	CFileManager::Delete (m_szTmpFile);
	memset (&missionData, 0, sizeof (missionData));
	ReadMissionFile (m_szFile);
	}
else {
		char szExt [256];

	err = theMine->Load (pszFile);
	CFileManager::SplitPath (pszFile, null, pszSubFile, szExt);
	strcat_s (pszSubFile, 256, szExt);
	}
theMine->Reset ();
DLE.TextureView ()->Setup ();
DLE.MineView ()->DelayRefresh (true);
DLE.MineView ()->Reset ();
DLE.MineView ()->FitToView ();
DLE.TextureView ()->Refresh ();
DLE.ToolView ()->MissionTool ()->Refresh ();
DLE.MineView ()->DelayRefresh (false);
DLE.MineView ()->ResetView (true);
DLE.MainFrame ()->UpdateSelectButtons ((enum eSelectModes) DLE.MineView ()->GetSelectMode ());
//UpdateAllViews (null);
if (!err) {
	UpdateCaption ();
	AfxGetApp ()->AddToRecentFileList (m_szFile);
	}
//textureManager.CountCustomTextures ();
DLE.ToolView ()->TextureTool ()->LoadTextureListBoxes ();
return (err == 0);
}

                        /*--------------------------*/

//BOOL CDlcDoc::OnSaveDocument (LPCTSTR lpszPathName) 
bool CDlcDoc::SaveFile (bool bSaveAs) 
{
	int err = 0;

DLE.ToolView ()->Refresh ();
//textureManager.CountCustomTextures ();
if (bEnableDeltaShading)
	DLE.ToolView ()->LightTool ()->OnShowDelta ();
if (!*m_szFile) {
	char	szMissions [256];
	CFileManager::SplitPath ((DLE.IsD1File ()) ? descentPath [0] : missionPath, szMissions, null, null);
//	strcpy_s (m_szFile, sizeof (m_szFile), (DLE.IsD1File ()) ? "new.rdl" : "new.rl2");
	sprintf_s (m_szFile, sizeof (m_szFile), "%s%s.hog", szMissions, *m_szSubFile ? m_szSubFile : "new");
	}
if (bSaveAs && !BrowseForFile (m_szFile, FALSE))
	return false;
if (strstr (m_szFile, ".hog"))
	err = SaveToHog (m_szFile, m_szSubFile, bSaveAs);
else
	err = theMine->Save (m_szFile);
SetModifiedFlag (err != 0);
if (!err) {
	UpdateCaption ();
	AfxGetApp ()->AddToRecentFileList (m_szFile);
	}
return (err == 0);
}

                        /*--------------------------*/

BOOL CDlcDoc::OnOpenDocument (LPCTSTR lpszPathName) 
{
strcpy_s (m_szFile, sizeof (m_szFile), lpszPathName);
return OpenFile (false);
}

                        /*--------------------------*/

BOOL CDlcDoc::OnSaveDocument (LPCTSTR lpszPathName) 
{
strcpy_s (m_szFile, sizeof (m_szFile), lpszPathName);
return SaveFile (false);
}

                        /*--------------------------*/

void CDlcDoc::OnOpenFile () 
{
OpenFile ();
}

                        /*--------------------------*/

void CDlcDoc::OnSaveFile () 
{
SaveFile (false);
}

                        /*--------------------------*/

void CDlcDoc::OnSaveFileAs () 
{
SaveFile (true);
}

                        /*--------------------------*/

void CDlcDoc::OnRunLevel () 
{
SaveFile (false);
char *h, *p = strstr (m_szFile, "missions\\");
if (p) {
	char	szProg [255], szHogFile [255], szMission [255];

	strcpy_s (szProg, sizeof (szProg), descentPath [1]);
	if (h = strstr (szProg, "data"))
		*h = '\0';
	int i;
	for (i = int (strlen (szProg)); i && szProg [i - 1] != '\\'; i--)
		;
	szProg [i] = '\0';
	_chdir (szProg);
#ifdef _DEBUG
	strcat_s (szProg, sizeof (szProg), "d2x-xl-dbg.exe");
#else
	strcat_s (szProg, sizeof (szProg), "d2x-xl.exe");
#endif
	sprintf_s (szHogFile, sizeof (szHogFile), "\"%s\"", p + strlen ("missions\\"));
	sprintf_s (szMission, sizeof (szMission), "\"%s\"", m_szSubFile);
	intptr_t j = _spawnl (_P_WAIT, szProg, szProg, 
								 *player_profile ? "-player" : "", player_profile, 
								 "-auto_hogfile", szHogFile, 
								 "-auto_mission", szMission, 
								 null);
	if (j < 0)
		j = errno;
	}
}

                        /*--------------------------*/

void CDlcDoc::OnInsertSegment() 
{
segmentManager.Add ();
}

void CDlcDoc::OnDeleteSegment() 
{
segmentManager.Delete ();
}

void CDlcDoc::OnInsertSegReactor ()
{
segmentManager.CreateReactor ();
}

void CDlcDoc::OnInsertSegRobotMaker ()
{
segmentManager.CreateRobotMaker ();
}

void CDlcDoc::OnInsertSegFuelCenter ()
{
segmentManager.CreateFuelCenter ();
}

void CDlcDoc::OnInsertSegRepairCenter ()
{
segmentManager.CreateFuelCenter (-1, SEGMENT_FUNC_REPAIRCEN);
}

void CDlcDoc::OnInsertDoorNormal ()
{
wallManager.CreateAutoDoor ();
}

void CDlcDoc::OnInsertDoorPrison ()
{
wallManager.CreatePrisonDoor ();
}

void CDlcDoc::OnInsertDoorGuideBot ()
{
wallManager.CreateGuideBotDoor ();
}

void CDlcDoc::OnInsertDoorExit ()
{
wallManager.CreateNormalExit ();
}

void CDlcDoc::OnInsertDoorSecretExit ()
{
wallManager.CreateSecretExit ();
}

void CDlcDoc::OnInsertTriggerOpenDoor ()
{
triggerManager.AddOpenDoor ();
}

void CDlcDoc::OnInsertTriggerRobotMaker ()
{
triggerManager.AddRobotMaker ();
}

void CDlcDoc::OnInsertTriggerShieldDrain ()
{
triggerManager.AddShieldDrain ();
}

void CDlcDoc::OnInsertTriggerEnergyDrain ()
{
triggerManager.AddEnergyDrain ();
}

void CDlcDoc::OnInsertTriggerControlPanel ()
{
triggerManager.AddUnlock ();
}

void CDlcDoc::OnInsertWallFuelCells ()
{
wallManager.CreateFuelCell ();
}

void CDlcDoc::OnInsertWallIllusion ()
{
wallManager.CreateIllusion ();
}

void CDlcDoc::OnInsertWallForceField ()
{
wallManager.CreateForceField ();
}

void CDlcDoc::OnInsertWallFan ()
{
wallManager.CreateFan ();
}

void CDlcDoc::OnInsertWallGrate ()
{
wallManager.CreateGrate ();
}

void CDlcDoc::OnInsertWallWaterfall ()
{
wallManager.CreateWaterFall ();
}

void CDlcDoc::OnInsertWallLavafall ()
{
wallManager.CreateLavaFall ();
}

void CDlcDoc::OnInsertObjectPlayer ()
{
objectManager.Create (OBJ_PLAYER);
}

void CDlcDoc::OnInsertObjectCoopPlayer ()
{
objectManager.Create (OBJ_COOP);
}

void CDlcDoc::OnInsertObjectPlayerCopy ()
{

	objectManager.Create (OBJ_NONE);
}

void CDlcDoc::OnInsertObjectRobot ()
{
if (objectManager.Create (OBJ_ROBOT)) {
	current.Object ()->Id () = 3; // class 1 drone
	current.Object ()->Setup (current.Object ()->Type ());
	}
}

void CDlcDoc::OnInsertObjectWeapon ()
{
if (objectManager.Create (OBJ_WEAPON)) {
	current.Object ()->Id () = 3; // laser
	current.Object ()->Setup (current.Object ()->Type ());
	}
}

void CDlcDoc::OnInsertObjectPowerup ()
{
if (objectManager.Create (OBJ_POWERUP)) {
	current.Object ()->Id () = 1; // energy boost
	current.Object ()->Setup (current.Object ()->Type ());
	}
}

void CDlcDoc::OnInsertObjectGuideBot ()
{
if (objectManager.Create (OBJ_ROBOT)) {
	current.Object ()->Id () = 33; // guide bot
	current.Object ()->Setup (current.Object ()->Type ());
	}
}

void CDlcDoc::OnInsertObjectReactor ()
{
if (objectManager.Create (OBJ_CNTRLCEN)) {
	current.Object ()->Id () = 2; // standard reactor
	current.Object ()->Setup (current.Object ()->Type ());
	}
}

void CDlcDoc::OnDeleteObject ()
{
if ((QueryMsg ("Are you sure you want to delete this object?") == IDYES))
	theMine->DeleteObject ();
}

void CDlcDoc::OnDeleteWall ()
{
wallManager.Delete (current.m_nWall);
}

void CDlcDoc::OnDeleteTrigger ()
{
triggerManager.DeleteFromWall ();
}

void CDlcDoc::OnCutBlock ()
{
blockManager.Cut ();
}

void CDlcDoc::OnCopyBlock ()
{
blockManager.Copy();
}

void CDlcDoc::OnQuickCopyBlock ()
{
blockManager.Copy("dle_temp.blx");
}

void CDlcDoc::OnPasteBlock ()
{
blockManager.Paste();
}

void CDlcDoc::OnQuickPasteBlock ()
{
blockManager.QuickPaste();
}

void CDlcDoc::OnDeleteBlock ()
{
blockManager.Delete();
}

void CDlcDoc::OnCopyOtherCube ()
{
segmentManager.CopyOtherSegment ();
}


long CDlcDoc::Test() 
{
	// TODO: Add your dispatch handler code here

	return 0;
}

void CDlcDoc::SetMyText(LPCTSTR string) 
{
//	 m_Text.Format(string);
}

#if 0
void CDlcDoc::OnFileTest() 
{
	CParser parser();
	
	parser.RunScript();
	
}
#endif
