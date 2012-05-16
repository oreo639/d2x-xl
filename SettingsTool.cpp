// dlcView.cpp : implementation of the CMineView class
//

#include "stdafx.h"
#include "afxpriv.h"
#include "dle-xp.h"

#include "dlcDoc.h"
#include "mineview.h"
#include "toolview.h"

#include "PaletteManager.h"
#include "TextureManager.h"
#include "global.h"
#include "FileManager.h"
#include "ObjectManager.h"

#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	MAX_VIEWDIST	30

/////////////////////////////////////////////////////////////////////////////
// CToolView

BEGIN_MESSAGE_MAP(CSettingsTool, CToolDlg)
	ON_WM_HSCROLL ()
	ON_WM_VSCROLL ()
	ON_BN_CLICKED (IDC_PREFS_BROWSE_D1PIG, OnOpenD1PIG)
	ON_BN_CLICKED (IDC_PREFS_BROWSE_D2PIG, OnOpenD2PIG)
	ON_BN_CLICKED (IDC_PREFS_BROWSE_MISSIONS, OnOpenMissions)
	ON_BN_CLICKED (IDC_PREFS_VIEW_MINE_NONE, OnViewMineNone)
	ON_BN_CLICKED (IDC_PREFS_VIEW_MINE_ALL, OnViewMineAll)
	ON_BN_CLICKED (IDC_PREFS_VIEW_OBJECTS_NONE, OnViewObjectsNone)
	ON_BN_CLICKED (IDC_PREFS_VIEW_OBJECTS_ALL, OnViewObjectsAll)
	ON_BN_CLICKED (IDC_PREFS_VIEW_PLAYERS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_ROBOTS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_WEAPONS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_POWERUPS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_KEYS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_HOSTAGES, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_REACTOR, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_EFFECTS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_WALLS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_SPECIAL, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_LIGHTS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_SHADING, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_DELTALIGHTS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_HIDEMARKED, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_ALLTEXTURES, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_SKYBOX, OnOK)
	ON_BN_CLICKED (IDC_PREFS_VIEW_DEPTHTEST, OnOK)
	ON_BN_CLICKED (IDC_PREFS_DEPTH_OFF, OnOK)
	ON_BN_CLICKED (IDC_PREFS_DEPTH_LOW, OnOK)
	ON_BN_CLICKED (IDC_PREFS_DEPTH_MEDIUM, OnOK)
	ON_BN_CLICKED (IDC_PREFS_DEPTH_HIGH, OnOK)
	ON_BN_CLICKED (IDC_PREFS_LAYOUT0, OnLayout0)
	ON_BN_CLICKED (IDC_PREFS_LAYOUT1, OnLayout1)
	ON_BN_CLICKED (IDC_PREFS_LAYOUT2, OnLayout2)
	ON_BN_CLICKED (IDC_PREFS_LAYOUT3, OnLayout3)
	ON_BN_CLICKED (IDC_PREFS_RENDERER_1ST_PERSON, OnOK)
	ON_BN_CLICKED (IDC_PREFS_RENDERER_3RD_PERSON, OnOK)
	//ON_BN_CLICKED (IDC_PREFS_RENDERER_PERSPECTIVE, OnOK)
	ON_BN_CLICKED (IDC_PREFS_EXPERTMODE, OnOK)
	ON_BN_CLICKED (IDC_PREFS_SPLASHSCREEN, OnOK)
	ON_BN_CLICKED (IDC_PREFS_USETEXCOLORS, OnOK)
	ON_BN_CLICKED (IDC_PREFS_UNDO, OnOK)
	ON_EN_KILLFOCUS (IDC_PREFS_MOVERATE, OnOK)
	ON_EN_KILLFOCUS (IDC_PREFS_VIEW_MOVERATE, OnOK)
	ON_EN_KILLFOCUS (IDC_PREFS_BROWSE_D1PIG, OnOK)
	ON_EN_KILLFOCUS (IDC_PREFS_BROWSE_D2PIG, OnOK)
	ON_EN_KILLFOCUS (IDC_PREFS_BROWSE_MISSIONS, OnOK)
	ON_CBN_SELCHANGE (IDC_PREFS_MINECENTER, OnSetMineCenter)
//	ON_EN_UPDATE (IDC_PREFS_MOVERATE, OnOK)
END_MESSAGE_MAP()

//------------------------------------------------------------------------------

void CSettingsTool::CompletePath (LPSTR pszPath, LPSTR pszFile, LPSTR pszExt)
{
_strlwr_s (pszPath, 256);
if (*pszPath && !strstr (pszPath, pszExt)) {
	if (pszPath [strlen (pszPath) - 1] != '\\')
		strcat_s (pszPath, 256, "\\");
	strcat_s (pszPath, 256, pszFile);
	}
}

//------------------------------------------------------------------------------

CSettingsTool::CSettingsTool (CPropertySheet *pParent)
	: CToolDlg (IDD_PREFSDATA_HORZ + nLayout, pParent)
{
m_depthPerception = 2;

m_rotateRates [8] = (double) PI / 4.0; // 45 deg
m_rotateRates [7] = (double) PI / 6.0; // 30 deg
m_rotateRates [6] = (double) PI / 8.0;
m_rotateRates [5] = (double) PI / 12.0; // 15 deg
m_rotateRates [4] = (double) PI / 16.0;
m_rotateRates [3] = (double) PI / 24.0; // 7.5 deg
m_rotateRates [2] = (double) PI / 32.0;
m_rotateRates [1] = (double) PI / 64.0;
m_rotateRates [0] = (double) PI / 128.0;
m_iRotateRate = 4;
m_bDepthTest = 1;
m_nRenderer = -1;
m_nPerspective = 0;

LoadAppSettings (true);

DLE.ExpertMode () = m_bExpertMode;
DLE.SplashScreen () = m_bSplashScreen;
m_bApplyFaceLightSettingsGlobally = 1; // on by default; changes only valid per session and not persistent!

m_bNoRefresh = false;
m_bInvalid = false;
}

//------------------------------------------------------------------------------

CSettingsTool::~CSettingsTool ()
{
SaveAppSettings ();
}

//------------------------------------------------------------------------------

char* __cdecl FormatRotateRate (char* szValue, int nValue)
{
	static char* szRotateRates [] = {"1.40625°", "2.8125°", "5.625°", "7.5°", "11.25°", "15°", "22.5°", "30°", "45°"};

strcpy (szValue, szRotateRates [nValue]);
return szValue;
}

//------------------------------------------------------------------------------

BOOL CSettingsTool::OnInitDialog ()
{
if (!CToolDlg::OnInitDialog ())
   return FALSE;
m_btnBrowseD1PIG.AutoLoad (IDC_PREFS_BROWSE_D1PIG, this);
m_btnBrowseD2PIG.AutoLoad (IDC_PREFS_BROWSE_D2PIG, this);
m_btnBrowseMissions.AutoLoad (IDC_PREFS_BROWSE_MISSIONS, this);
#if 1
// first get the default values
GetAppSettings (false);
LoadAppSettings (false);
//m_mineViewFlags = DLE.MineView ()->GetMineViewFlags ();
//m_objViewFlags = DLE.MineView ()->GetObjectViewFlags ();
//m_texViewFlags = DLE.TextureView ()->GetViewFlags ();
//m_bDepthTest = DLE.MineView ()->DepthTest ();
//m_bSortObjects = objectManager ().SortObjects ();

//m_bDepthTest = GetPrivateProfileInt ("DLE-XP", "DepthTest", m_bDepthTest, DLE.IniFile ());
//m_mineViewFlags = GetPrivateProfileInt ("DLE-XP", "MineViewFlags", m_mineViewFlags, DLE.IniFile ());
//m_objViewFlags = GetPrivateProfileInt ("DLE-XP", "ObjViewFlags", m_objViewFlags, DLE.IniFile ());
//m_texViewFlags = GetPrivateProfileInt ("DLE-XP", "TexViewFlags", m_texViewFlags, DLE.IniFile ());
//m_nMaxUndo = GetPrivateProfileInt ("DLE-XP", "MaxUndo", DLE_MAX_UNDOS, DLE.IniFile ());

m_rotateRate.Init (this, IDC_PREFS_ROTATE_RATE_SLIDER, IDC_PREFS_ROTATE_RATE_SPINNER, -IDT_PREFS_ROTATE_RATE, 0, 8);
m_rotateRate.SetFormatter (FormatRotateRate);
InitSlider (IDC_PREFS_VIEWDIST, 0, MAX_VIEWDIST);
for (int i = 0; i <= MAX_VIEWDIST; i++)
	ViewDistSlider ()->SetTic (i);
CComboBox *pcb = CBMineCenter ();
pcb->AddString ("None");
pcb->AddString ("Crosshairs");
pcb->AddString ("Globe");
m_bInvalid = false;
SetAppSettings ();
#else
m_bInvalid = false;
#endif
m_bInited = true;
return TRUE;
}

//------------------------------------------------------------------------------

void CSettingsTool::Refresh (void)
{
if (::IsWindow (m_hWnd) && !m_bNoRefresh) {
	OnCancel ();
/*
	m_mineViewFlags = DLE.MineView ()->GetMineViewFlags ();
	m_objViewFlags = DLE.MineView ()->GetObjectViewFlags ();
	m_texViewFlags = DLE.TextureView ()->GetViewFlags ();
	UpdateData (FALSE);
*/
	}
}

//------------------------------------------------------------------------------

void CSettingsTool::DoDataExchange (CDataExchange * pDX)
{
if (!HaveData (pDX)) 
	return;

	int	h, i;

DDX_Text (pDX, IDC_PREFS_PATH_D1PIG, m_d1Folder, sizeof (m_d1Folder));
DDX_Text (pDX, IDC_PREFS_PATH_D2PIG, m_d2Folder, sizeof (m_d2Folder));
DDX_Text (pDX, IDC_PREFS_PATH_MISSIONS, m_missionFolder, sizeof (m_missionFolder));
DDX_Radio (pDX, IDC_PREFS_DEPTH_OFF, m_depthPerception);
m_rotateRate.DoDataExchange (pDX, m_iRotateRate);
#if 0
sprintf (szMoveRate, "%1.3f", m_moveRate [0]);
DDX_Text (pDX, IDC_PREFS_MOVERATE, szMoveRate, sizeof (szMoveRate));
m_moveRate [0] = atof (szMoveRate);
#else
DDX_Text (pDX, IDC_PREFS_MOVERATE, m_moveRate [0]);
DDX_Text (pDX, IDC_PREFS_VIEW_MOVERATE, m_moveRate [1]);
#endif
for (i = 0; i <= IDC_PREFS_VIEW_SKYBOX - IDC_PREFS_VIEW_WALLS; i++) {
	h = ((m_mineViewFlags & (1 << i)) != 0);
	DDX_Check (pDX, IDC_PREFS_VIEW_WALLS + i, h);
	if (h)
		m_mineViewFlags |= (1 << i);
	else
		m_mineViewFlags &= ~(1 << i);
	}
if (!pDX->m_bSaveAndValidate) {
	m_mineViewFlags = DLE.MineView ()->GetMineViewFlags ();
	m_objViewFlags = DLE.MineView ()->GetObjectViewFlags ();
	m_texViewFlags = DLE.TextureView ()->GetViewFlags ();
	}
h = ((m_texViewFlags & eViewMineUsedTextures) != 0);
DDX_Check (pDX, IDC_PREFS_VIEW_ALLTEXTURES, h);
m_texViewFlags = h ? eViewMineUsedTextures : 0;
for (i = 0; i <= IDC_PREFS_VIEW_EFFECTS - IDC_PREFS_VIEW_ROBOTS; i++) {
	h = ((m_objViewFlags & (1 << i)) != 0);
	DDX_Check (pDX, IDC_PREFS_VIEW_ROBOTS + i, h);
	if (h)
		m_objViewFlags |= (1 << i);
	else
		m_objViewFlags &= ~(1 << i);
	}
DDX_Check (pDX, IDC_PREFS_VIEW_DEPTHTEST, m_bDepthTest);
DDX_Check (pDX, IDC_PREFS_EXPERTMODE, m_bExpertMode);
DDX_Check (pDX, IDC_PREFS_SPLASHSCREEN, m_bSplashScreen);
DDX_Text (pDX, IDC_PREFS_UNDO, m_nMaxUndo);
h = bSingleToolPane ? 3 : nLayout;
DDX_Radio (pDX, IDC_PREFS_LAYOUT0, h);
bSingleToolPane = (h == 3);
nLayout = bSingleToolPane ? 1 : h;
DDX_Radio (pDX, IDC_PREFS_RENDERER_3RD_PERSON, m_nPerspective);
DDX_Check (pDX, IDC_PREFS_USETEXCOLORS, m_bApplyFaceLightSettingsGlobally);
if (pDX->m_bSaveAndValidate)
	m_nMineCenter = CBMineCenter ()->GetCurSel ();
else {
	char	szViewDist [10];
	if (m_nViewDist)
		sprintf_s (szViewDist, sizeof (szViewDist), "%d", DLE.MineView ()->ViewDist ());
	else
		strcpy_s (szViewDist, sizeof (szViewDist), "all");
	((CWnd *) GetDlgItem (IDC_PREFS_VIEWDIST_TEXT))->SetWindowText (szViewDist);
	ViewDistSlider ()->SetPos (m_nViewDist);
	CBMineCenter ()->SetCurSel (m_nMineCenter);
	}
DDX_Text (pDX, IDC_PREFS_PLAYER, szPlayerProfile, sizeof (szPlayerProfile));
}

								/*--------------------------*/

void CSettingsTool::OnSetMineCenter (void)
{
UpdateData (TRUE);
m_bNoRefresh = true;
SetAppSettings (-1);
m_bNoRefresh = false;
}

								/*--------------------------*/

BOOL CSettingsTool::OnSetActive ()
{
Refresh ();
return CToolDlg::OnSetActive ();
}

//------------------------------------------------------------------------------

bool CSettingsTool::BrowseFile (LPSTR fileType, LPSTR fileName, LPSTR fileExt, BOOL bOpen)
{
   char        s [256];
   int         nResult;
   char		   pn [256];

strcpy_s (pn, sizeof (pn), fileName);
sprintf_s (s, sizeof (s), "%s (%s)|%s|all files (*.*)|*.*||", fileType, fileExt, fileExt);
CFileDialog d (bOpen, fileExt, pn, 0, s, this);
d.m_ofn.hInstance = AfxGetInstanceHandle ();
d.m_ofn.lpstrInitialDir = pn;
if ((nResult = int (d.DoModal ())) != IDOK)
	return false;
strcpy_s (fileName, 256, d.m_ofn.lpstrFile);
return true;
}

//------------------------------------------------------------------------------

void CSettingsTool::WritePrivateProfileInt (LPSTR szKey, int nValue)
{
	char	szValue [20];

sprintf_s (szValue, sizeof (szValue), "%d", nValue);
WritePrivateProfileString ("DLE-XP", szKey, szValue, DLE.IniFile ());
}

//------------------------------------------------------------------------------

void CSettingsTool::WritePrivateProfileDouble (LPSTR szKey, double nValue)
{
	char	szValue [100];

sprintf_s (szValue, sizeof (szValue), "%1.3f", nValue);
WritePrivateProfileString ("DLE-XP", szKey, szValue, DLE.IniFile ());
}

//------------------------------------------------------------------------------
// copy application settings to local storage

void CSettingsTool::GetAppSettings (bool bGetFolders)
{
if (bGetFolders) {
	strcpy_s (m_d1Folder, sizeof (m_d1Folder), descentFolder [0]);
	strcpy_s (m_d2Folder, sizeof (m_d2Folder), descentFolder [1]);
	strcpy_s (m_missionFolder, sizeof (m_missionFolder), missionFolder);
	}

m_bDepthTest = DLE.MineView ()->DepthTest ();
m_mineViewFlags = DLE.MineView ()->GetMineViewFlags ();
m_objViewFlags = DLE.MineView ()->GetObjectViewFlags ();
m_texViewFlags = DLE.TextureView ()->GetViewFlags ();
m_nMaxUndo = undoManager.MaxSize ();
m_depthPerception = DLE.MineView ()->DepthPerception ();
m_iRotateRate = 4;
if (theMine) {
	int i = 8;
	for (; i > 0; i--)
		if (theMine->RotateRate () >= m_rotateRates [i]) {
			break;
			}
	m_iRotateRate = i;
	}
DLE.MineView ()->GetMoveRates (m_moveRate);
m_nViewDist = DLE.MineView ()->ViewDist ();
m_bApplyFaceLightSettingsGlobally = lightManager.ApplyFaceLightSettingsGlobally ();
m_bSortObjects = objectManager.SortObjects ();
m_bExpertMode = DLE.ExpertMode ();
m_bSplashScreen = DLE.SplashScreen ();
}

//------------------------------------------------------------------------------

void CSettingsTool::LoadAppSettings (bool bInitialize)
{
if (bInitialize) {
	GetPrivateProfileString ("DLE-XP", "DescentDirectory", descentFolder [0], descentFolder [0], sizeof (descentFolder [0]), DLE.IniFile ());
	strcpy_s (m_d1Folder, sizeof (m_d1Folder), descentFolder [0]);
	CompletePath (m_d1Folder, "descent.pig", ".pig");
	GetPrivateProfileString ("DLE-XP", "Descent2Directory", descentFolder [1], descentFolder [1], sizeof (descentFolder [1]), DLE.IniFile ());
	strcpy_s (m_d2Folder, sizeof (m_d2Folder), descentFolder [1]);
	CompletePath (m_d2Folder, "groupa.pig", ".pig");
	GetPrivateProfileString ("DLE-XP", "LevelsDirectory", missionFolder, missionFolder, sizeof (missionFolder), DLE.IniFile ());
	char* ps = strrchr (missionFolder, '\\');
	if (ps)
		*ps = '\0';
	strcpy_s (m_missionFolder, sizeof (m_missionFolder), missionFolder);
	//CompletePath (m_missionFolder, "descent2.hog", ".hog");
	GetPrivateProfileString ("DLE-XP", "PlayerProfile", szPlayerProfile, szPlayerProfile, sizeof (szPlayerProfile), DLE.IniFile ());
	strcpy (modFolder, descentFolder [1]);
	ps = strstr (modFolder, "data");
	if (ps)
		strcpy (ps, "mods");
	else
		strcat (modFolder, "mods");

	m_depthPerception = GetPrivateProfileInt ("DLE-XP", "DepthPerception", 2, DLE.IniFile ());
	m_iRotateRate = GetPrivateProfileInt ("DLE-XP", "RotateRate", 4, DLE.IniFile ());

	char	szMoveRate [100];

	GetPrivateProfileString ("DLE-XP", "MoveRate", "1", szMoveRate, sizeof (szMoveRate), DLE.IniFile ());
	m_moveRate [0] = Clamp ((double) atof (szMoveRate), 0.001, 1000.0);
	GetPrivateProfileString ("DLE-XP", "ViewMoveRate", "1", szMoveRate, sizeof (szMoveRate), DLE.IniFile ());
	m_moveRate [1] = Clamp ((double) atof (szMoveRate), 0.001, 1000.0);
	}

#if 0
*descentFolder [0] =
*descentFolder [1] = '\0';
#endif

m_bExpertMode = GetPrivateProfileInt ("DLE-XP", "ExpertMode", 1, DLE.IniFile ());
m_bSplashScreen = GetPrivateProfileInt ("DLE-XP", "SplashScreen", 1, DLE.IniFile ());
m_mineViewFlags = GetPrivateProfileInt ("DLE-XP", "MineViewFlags", m_mineViewFlags, DLE.IniFile ());
m_objViewFlags = GetPrivateProfileInt ("DLE-XP", "ObjViewFlags", m_objViewFlags, DLE.IniFile ());
m_texViewFlags = GetPrivateProfileInt ("DLE-XP", "TexViewFlags", m_texViewFlags, DLE.IniFile ());
//m_bApplyFaceLightSettingsGlobally = GetPrivateProfileInt ("DLE-XP", "ApplyFaceLightSettingsGlobally", m_bApplyFaceLightSettingsGlobally, DLE.IniFile ());
m_bSortObjects = GetPrivateProfileInt ("DLE-XP", "SortObjects", m_bSortObjects, DLE.IniFile ());
m_bDepthTest = GetPrivateProfileInt ("DLE-XP", "DepthTest", m_bDepthTest, DLE.IniFile ());
m_nViewDist = GetPrivateProfileInt ("DLE-XP", "ViewDistance", 0, DLE.IniFile ());
//m_nRenderer = GetPrivateProfileInt ("DLE-XP", "Renderer", 0, DLE.IniFile ());
m_nPerspective = GetPrivateProfileInt ("DLE-XP", "Perspective", 0, DLE.IniFile ());
m_nMineCenter = GetPrivateProfileInt ("DLE-XP", "MineCenter", 0, DLE.IniFile ());
m_nMaxUndo = GetPrivateProfileInt ("DLE-XP", "MaxUndo", DLE_MAX_UNDOS, DLE.IniFile ());
}

//------------------------------------------------------------------------------

void CSettingsTool::SaveAppSettings (bool bSaveFolders)
{
	char	szMoveRate [100];

GetAppSettings (bSaveFolders);
if (bSaveFolders) {
	WritePrivateProfileString ("DLE-XP", "DescentDirectory", descentFolder [0], DLE.IniFile ());
	WritePrivateProfileString ("DLE-XP", "Descent2Directory", descentFolder [1], DLE.IniFile ());
	WritePrivateProfileString ("DLE-XP", "levelsDirectory", missionFolder, DLE.IniFile ());
	}
WritePrivateProfileString ("DLE-XP", "PlayerProfile", szPlayerProfile, DLE.IniFile ());
//WritePrivateProfileInt ("Renderer", m_nRenderer);
WritePrivateProfileInt ("Perspective", m_nPerspective);
WritePrivateProfileInt ("DepthPerception", m_depthPerception);
WritePrivateProfileInt ("RotateRate", m_iRotateRate);
sprintf_s (szMoveRate, sizeof (szMoveRate), "%1.3f", m_moveRate [0]);
WritePrivateProfileDouble ("MoveRate", m_moveRate [0]);
sprintf_s (szMoveRate, sizeof (szMoveRate), "%1.3f", m_moveRate [1]);
WritePrivateProfileDouble ("ViewMoveRate", m_moveRate [1]);
WritePrivateProfileInt ("ExpertMode", m_bExpertMode);
WritePrivateProfileInt ("SplashScreen", m_bSplashScreen);
WritePrivateProfileInt ("DepthTest", m_bDepthTest);
WritePrivateProfileInt ("MineViewFlags", m_mineViewFlags);
WritePrivateProfileInt ("ObjViewFlags", m_objViewFlags);
WritePrivateProfileInt ("TexViewFlags", m_texViewFlags);
//WritePrivateProfileInt ("ApplyFaceLightSettingsGlobally", m_bApplyFaceLightSettingsGlobally);
WritePrivateProfileInt ("SortObjects", m_bSortObjects);
WritePrivateProfileInt ("ViewDistance", m_nViewDist);
WritePrivateProfileInt ("MineCenter", m_nMineCenter);
WritePrivateProfileInt ("MaxUndo", m_nMaxUndo);
WritePrivateProfileInt ("TextureFilter", DLE.TextureView ()->TextureFilter ());
}

//------------------------------------------------------------------------------

void CSettingsTool::ReloadTextures (int nVersion)
{
paletteManager.Reload ();
if (textureManager.Reload (nVersion)) {
	if (m_bInited) {
		DLE.TextureView ()->Setup ();
		DLE.TextureView ()->Refresh ();
		}
	}
else if (m_bInited)
	DLE.ToolView ()->SetActive (11);
if (m_bInited)
	DLE.MineView ()->ResetView (true);
}

//------------------------------------------------------------------------------

void CSettingsTool::SetPerspective (int nPerspective) 
{ 
m_nPerspective = nPerspective; 
DLE.MineView ()->SetPerspective (m_nPerspective);
}

//------------------------------------------------------------------------------

void CSettingsTool::TogglePerspective (void)
{
SetPerspective (!m_nPerspective);
}

//------------------------------------------------------------------------------
// copy local settings to application

void CSettingsTool::SetAppSettings (int bUpdate)
{
CHECKMINE;
if (m_bInvalid)
	return;

_strlwr_s (m_d1Folder, sizeof (m_d1Folder));
if (strcmp (descentFolder [0], m_d1Folder)) {
	strcpy_s (descentFolder [0], sizeof (descentFolder [0]), m_d1Folder);
	WritePrivateProfileString ("DLE-XP", "DescentDirectory", descentFolder [0], DLE.IniFile ());
	ReloadTextures (0);
	}

_strlwr_s (m_d2Folder, sizeof (m_d2Folder));
if (strcmp (descentFolder [1], m_d2Folder)) {
	bool	bChangePig = true;
	if (textureManager.HasCustomTextures () &&
		 (QueryMsg ("Changing the pig file will affect the custom textures\n"
						"in this level because of the change in palette.\n"
						"(Reload the level to int custom texture appeareance.)\n\n"
						"Are you sure you want to do this?") != IDOK))
		bChangePig = false;
	if (bChangePig) {
		strcpy_s (descentFolder [1], sizeof (descentFolder [1]), m_d2Folder);
		WritePrivateProfileString ("DLE-XP", "Descent2Directory", descentFolder [1], DLE.IniFile ());
		ReloadTextures (1);
		}
	}

_strlwr_s (m_missionFolder, sizeof (m_missionFolder));
if (strcmp (missionFolder, m_missionFolder)) {
	strcpy_s (missionFolder, sizeof (missionFolder), m_missionFolder);
	WritePrivateProfileString ("DLE-XP", "levelsDirectory", missionFolder, DLE.IniFile ());
	if (*missionFolder)
		::SetCurrentDirectory (missionFolder);
	}

if (!bUpdate)
	DLE.MineView ()->DelayRefresh (true);
else if (bUpdate < 0) {
	DLE.MineView ()->SetRenderer (m_nRenderer);
	m_nRenderer = abs (m_nRenderer);
	DLE.MineView ()->SetPerspective (m_nPerspective);
	DLE.MineView ()->SetViewDist (m_nViewDist);
	DLE.MineView ()->SetDepthTest (m_bDepthTest != 0);
	DLE.MineView ()->SetViewMineFlags (m_mineViewFlags);
	DLE.MineView ()->SetViewObjectFlags (m_objViewFlags);
	DLE.TextureView ()->SetViewFlags (m_texViewFlags);
	DLE.MineView ()->SetDepthScale (m_depthPerception);
	DLE.MineView ()->MineCenter () = m_nMineCenter;
	}
if (!bUpdate) {
	DLE.MineView ()->DelayRefresh (false);
	DLE.MineView ()->Refresh (false);
	}
if (theMine)
	theMine->RotateRate () = m_rotateRates [m_iRotateRate];
DLE.MineView ()->SetDepthScale (m_depthPerception);
DLE.MineView ()->SetMoveRates (m_moveRate);
DLE.ExpertMode () = m_bExpertMode;
lightManager.ApplyFaceLightSettingsGlobally () = m_bApplyFaceLightSettingsGlobally;
DLE.SplashScreen () = m_bSplashScreen;
objectManager.SortObjects () = m_bSortObjects;
undoManager.SetMaxSize (m_nMaxUndo);
if (bUpdate < 1)
	SaveAppSettings (false);
#ifdef _DEBUG
if (m_bInited)
	DLE.ToolView ()->SetActive (11);
#endif
}

//------------------------------------------------------------------------------

void CSettingsTool::OnOK (void)
{
UpdateData (TRUE);
m_bNoRefresh = true;
SetAppSettings (-1);
m_bNoRefresh = false;
Refresh ();
}

//------------------------------------------------------------------------------

void CSettingsTool::FreeTextureHandles (bool bDeleteAll)
{
//textureManager.Release (bDeleteAll);
DLE.TextureView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CSettingsTool::OnCancel (void)
{
GetAppSettings ();
UpdateData (FALSE);
}

//------------------------------------------------------------------------------

void CSettingsTool::OnOpenD1PIG (void)
{
if (BrowseFile ("Descent 1 PIG", m_d1Folder, "*.pig", TRUE)) {
	UpdateData (FALSE);
	OnOK ();
	}
}

//------------------------------------------------------------------------------

void CSettingsTool::OnOpenD2PIG (void)
{
if (BrowseFile ("Descent 2 PIG", m_d2Folder, "*.pig", TRUE)) {
	UpdateData (FALSE);
	OnOK ();
	}
}

//------------------------------------------------------------------------------

void CSettingsTool::OnOpenMissions (void)
{
if (BrowseFile ("Descent mission file", m_missionFolder, "*.hog", TRUE)) {
	UpdateData (FALSE);
	OnOK ();
	}
}

//------------------------------------------------------------------------------

void CSettingsTool::OnViewObjectsNone (void)
{
m_objViewFlags = eViewObjectsNone;
DLE.MineView ()->SetViewObjectFlags (m_objViewFlags);
UpdateData (FALSE);
OnOK ();
}

//------------------------------------------------------------------------------

void CSettingsTool::OnViewObjectsAll (void)
{
m_objViewFlags = eViewObjectsAll;
DLE.MineView ()->SetViewObjectFlags (m_objViewFlags);
UpdateData (FALSE);
OnOK ();
}

//------------------------------------------------------------------------------

void CSettingsTool::OnViewMineNone (void)
{
m_mineViewFlags &= ~(eViewMineLights | eViewMineSpecial | eViewMineWalls | eViewMineVariableLights);
DLE.MineView ()->SetViewMineFlags (m_mineViewFlags);
UpdateData (FALSE);
OnOK ();
}

//------------------------------------------------------------------------------

void CSettingsTool::OnViewMineAll (void)
{
m_mineViewFlags |= (eViewMineLights | eViewMineSpecial | eViewMineWalls | eViewMineVariableLights);
DLE.MineView ()->SetViewMineFlags (m_mineViewFlags);
UpdateData (FALSE);
OnOK ();
}

//------------------------------------------------------------------------------

void CSettingsTool::SetLayout (int nLayout)
{
if (nLayout == 3)
	WritePrivateProfileInt ("Layout", 1);
else
	WritePrivateProfileInt ("Layout", nLayout);
WritePrivateProfileInt ("SingleToolPane", nLayout == 3);
}

//------------------------------------------------------------------------------

void CSettingsTool::OnLayout0 (void) { SetLayout (0); }

void CSettingsTool::OnLayout1 (void) { SetLayout (1); }

void CSettingsTool::OnLayout2 (void) { SetLayout (2); }

void CSettingsTool::OnLayout3 (void) { SetLayout (3); }

//------------------------------------------------------------------------------

void CSettingsTool::OnRenderer (int nRenderer)
{
DLE.MineView ()->SetRenderer (m_nRenderer = nRenderer);
UpdateData (FALSE);
OnOK ();
}

void CSettingsTool::OnRendererSW (void) { OnRenderer (0); }

void CSettingsTool::OnRendererGL (void) { OnRenderer (1); }

//------------------------------------------------------------------------------

void CSettingsTool::OnScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
if (m_rotateRate.OnScroll (scrollCode, thumbPos, pScrollBar)) {
	 m_iRotateRate = m_rotateRate.GetValue ();
	if (theMine)
		theMine->RotateRate () = m_rotateRates [m_iRotateRate];
	}
else if ((void*) pScrollBar != (void*) ViewDistSlider ()) 
	pScrollBar->SetScrollPos (pScrollBar->GetScrollPos (), TRUE);
else {
	int nPos = ViewDistSlider ()->GetPos ();
	switch (scrollCode) {
		case SB_LINEUP:
			nPos--;
			break;
		case SB_LINEDOWN:
			nPos++;
			break;
		case SB_PAGEUP:
			nPos -= 10;
			break;
		case SB_PAGEDOWN:
			nPos += 10;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			nPos = thumbPos;
			break;
		case SB_ENDSCROLL:
			return;
		}
	if (nPos < 0)
		nPos = 0;
	else if (nPos > MAX_VIEWDIST)
		nPos = MAX_VIEWDIST;
	DLE.MineView ()->SetViewDist (m_nViewDist = nPos);
	}
}

//------------------------------------------------------------------------------

void CSettingsTool::OnHScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
OnScroll (scrollCode, thumbPos, pScrollBar);
}

//------------------------------------------------------------------------------

void CSettingsTool::OnVScroll (UINT scrollCode, UINT thumbPos, CScrollBar *pScrollBar)
{
OnScroll (scrollCode, thumbPos, pScrollBar);
}

//------------------------------------------------------------------------------

//eof prefsdlg.cpp