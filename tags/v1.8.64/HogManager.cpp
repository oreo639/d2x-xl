// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <math.h>
#include <io.h>
#include <string.h>
#include <errno.h>

#include "mine.h"
#include "dle-xp.h"
#include "TimeDate.h"

//------------------------------------------------------------------------------
// local globals
char *VERTIGO_HINT =
	"Vertigo levels allow you to use 12 robots in addition\n"
	"to the 66 standard Descent II robots.  But, these robots will\n"
	"not appear in the game unless you play the \"Descent 2: Vertigo\"\n"
	"mission after you start Descent II and before you play your level.\n";


char cut_paste_filename [256] = "";

//------------------------------------------------------------------------------
// local prototypes
int read_block_file(char *name,int option);
void strip_extension(char *str);

void DeleteSubFile (CFileManager& fp, long size, long offset, int num_entries, int delete_index);
void ImportSubFile ();
bool ExportSubFile (const char *pszSrc, const char *pszDest, long offset, long size);
void RenameSubFile ();
void DeleteLevelSubFiles(CFileManager& fp,char *name);

//------------------------------------------------------------------------------

static byte dataBuf [65536];

//------------------------------------------------------------------------------

int AddFileData (CListBox *plb, int index, int size, int offset, int fileno)
{
if (index == -1)
	index = plb->GetCurSel ();
if (index == -1)
	return -1;
tHogFileData *pfd = new tHogFileData;
if (!pfd)
	return -1;
pfd->m_size = size;
pfd->m_offs = offset;
pfd->m_fileno = fileno;
plb->SetItemDataPtr (index, (void *) pfd);
return index;
}

//------------------------------------------------------------------------------

void ClearFileList (CListBox *plb)
{
	int i, h = plb->GetCount ();
	tHogFileData *pfd;

for (i = 0; i < h; i++)
	if (pfd = (tHogFileData *) plb->GetItemDataPtr (i))
		delete pfd;
plb->ResetContent ();
}

//------------------------------------------------------------------------------

CInputDialog::CInputDialog (CWnd *pParentWnd, LPSTR pszTitle, LPSTR pszPrompt, LPSTR pszBuf, size_t nBufSize)
	: CDialog (IDD_INPDLG, pParentWnd)
{
m_pszTitle = pszTitle;
m_pszPrompt = pszPrompt;
m_pszBuf = pszBuf;
m_nBufSize = nBufSize;
}

//------------------------------------------------------------------------------

BOOL CInputDialog::OnInitDialog (void)
{
CDialog::OnInitDialog ();
SetWindowText (m_pszTitle);
return TRUE;
}

//------------------------------------------------------------------------------

void CInputDialog::DoDataExchange (CDataExchange * pDX)
{
DDX_Text (pDX, IDC_INPDLG_PROMPT, m_pszPrompt, int (strlen (m_pszPrompt)));
DDX_Text (pDX, IDC_INPDLG_BUF, m_pszBuf, int (m_nBufSize));
}

//------------------------------------------------------------------------------

void CInputDialog::OnOK (void)
{
UpdateData (TRUE);
CDialog::OnOK ();
}

//------------------------------------------------------------------------------

BEGIN_MESSAGE_MAP (CHogManager, CDialog)
	ON_BN_CLICKED (IDC_HOG_RENAME, OnRename)
	ON_BN_CLICKED (IDC_HOG_DELETE, OnDelete)
	ON_BN_CLICKED (IDC_HOG_IMPORT, OnImport)
	ON_BN_CLICKED (IDC_HOG_EXPORT, OnExport)
	ON_BN_CLICKED (IDC_HOG_FILTER, OnFilter)
	ON_LBN_SELCHANGE (IDC_HOG_FILES, OnSetFile)
	ON_LBN_DBLCLK (IDC_HOG_FILES, OnOK)
END_MESSAGE_MAP ()

//------------------------------------------------------------------------------

bool BrowseForFile (BOOL bOpen, LPSTR pszDefExt, LPSTR pszFile, LPSTR pszFilter, DWORD nFlags, CWnd *pParentWnd)
{
   INT_PTR     nResult;
   char		   szFile [256], szFolder [256], * ps;

if (*pszFile)
	strcpy_s (szFolder, sizeof (szFolder), pszFile);
else {
	strcpy_s (szFolder, sizeof (szFolder), missionPath);
	if ((ps = strstr (szFolder, "\\data")))
		ps [1] = '\0';
	if (!*pszDefExt) 
		strcat_s (szFolder, sizeof (szFolder), "missions\\*.*");
	else {
		strcat_s (szFolder, sizeof (szFolder), "missions\\*.");
		strcat_s (szFolder, sizeof (szFolder), pszDefExt);
		if ((ps = strchr (szFolder, ';'))) 
			*ps = '\0';
		}
	}
if (!(ps = strrchr (szFolder, '\\')))
	 strcpy_s (szFile, sizeof (szFile), szFolder);
else {
	ps [0] = '\0';
	strcpy_s (szFile, sizeof (szFile), ps + 1);
	}
CFileDialog d (bOpen, pszDefExt, szFile, nFlags, pszFilter, pParentWnd);
//d.m_ofn.hInstance = AfxGetInstanceHandle ();
//d.GetOFN ().lpstrInitialDir = szFolder;
d.m_ofn.lpstrInitialDir = szFolder;
if ((nResult = d.DoModal ()) != IDOK)
	return false;
strcpy_s (pszFile, 256, d.GetPathName ());
return true;
}

//------------------------------------------------------------------------------

CHogManager::CHogManager (CWnd *pParentWnd, LPSTR pszFile, LPSTR pszSubFile)
	: CDialog (IDD_HOGMANAGER, pParentWnd) 
{
m_bInited = false;
m_pszFile = pszFile;
m_pszSubFile = pszSubFile;
m_bShowAll = false;
}

//------------------------------------------------------------------------------

void CHogManager::EndDialog (int nResult)
{
if (m_bInited)
	ClearFileList ();
CDialog::EndDialog (nResult);
}

//------------------------------------------------------------------------------

void CHogManager::Reset (void)
{
/*
m_fileData.m_nFiles = 0;
int i;
for (i = 0; i < MAX_HOGFILES - 1; i++)
	m_fileData.m_size [i] = i + 1;
m_fileData.m_size [i] = -1;
m_fileData.m_nFreeList = 0;
*/
}

//------------------------------------------------------------------------------

BOOL CHogManager::OnInitDialog (void)
{
CDialog::OnInitDialog ();
if (ReadHogData ()) {
	m_bInited = true;
	return TRUE;
	}
EndDialog (0);
return FALSE;
}

//------------------------------------------------------------------------------

void CHogManager::DoDataExchange (CDataExchange * pDX)
{
	long size, offset;

GetFileData (-1, &size, &offset);
DDX_Text (pDX, IDC_HOG_SIZE, size);
DDX_Text (pDX, IDC_HOG_OFFSET, offset);
DDX_Check (pDX, IDC_HOG_FILTER, m_bShowAll);
}

//------------------------------------------------------------------------------

void CHogManager::OnCancel (void)
{
CDialog::OnCancel ();
}

//------------------------------------------------------------------------------

void CHogManager::OnFilter (void)
{
m_bShowAll = ((CButton *) GetDlgItem (IDC_HOG_FILTER))->GetCheck ();
if (!ReadHogData ())
	EndDialog (0);
}

//------------------------------------------------------------------------------

void CHogManager::OnSetFile (void)
{
UpdateData (FALSE);
}

//------------------------------------------------------------------------------

void CHogManager::ClearFileList (void)
{
::ClearFileList (LBFiles ());
}

//------------------------------------------------------------------------------

int CHogManager::DeleteFile (int index)
{
if (index == -1)
	index = LBFiles ()->GetCurSel ();
if (index == -1)
	return -1;
tHogFileData *pfd = (tHogFileData *) LBFiles ()->GetItemDataPtr (index);
if (pfd)
	delete pfd;
LBFiles ()->DeleteString (index);
return 0;
}

//------------------------------------------------------------------------------

int CHogManager::AddFile (LPSTR pszName, long length, long size, long offset, int fileno)
{
_strlwr_s (pszName, length);

	int index = LBFiles ()->AddString (pszName);

if (0 > AddFileData (index, size, offset, fileno))
	return -1;
LBFiles ()->SetCurSel (index);
return 0;
}

//------------------------------------------------------------------------------

int CHogManager::GetFileData (int index, long *size, long *offset)
{
if (index == -1)
	index = LBFiles ()->GetCurSel ();
if (index == -1)
	return -1;
tHogFileData *pfd = (tHogFileData *) LBFiles ()->GetItemDataPtr (index);
if (!pfd)
	return -1;
if (size)
	*size = pfd->m_size;
if (offset)
	*offset = pfd->m_offs;
return pfd->m_fileno;
}

//------------------------------------------------------------------------------

int CHogManager::AddFileData (int index, long size, long offset, int fileno)
{
return ::AddFileData (LBFiles (), index, size, offset, fileno);
}

//------------------------------------------------------------------------------

int CHogManager::FindFilename (LPSTR pszName)
{
	CListBox	*plb = LBFiles ();
	char szName [256];

int h, i;
for (h = plb->GetCount (), i = 0; i < h; i++) {
	plb->GetText (i, szName);
	if (!_strcmpi (szName, pszName))
		return i;
	}
return -1;
}

//------------------------------------------------------------------------------
// CHogManager - read hog data
//------------------------------------------------------------------------------

bool CHogManager::ReadHogData () 
{
ClearFileList ();
Reset ();
if (!::ReadHogData (m_pszFile, LBFiles (), m_bShowAll == 1, false))
	return false;
UpdateData (FALSE);
return true;
}

//------------------------------------------------------------------------------

long CHogManager::FindSubFile (CFileManager& fp, char* pszFile, char* pszSubFile, char* pszExt)
{
strcpy_s (message, sizeof (message), (pszSubFile == null) ? m_pszSubFile : pszSubFile);
char* p = strrchr (message, '.');
if (p == null) 
	return 0;

long size, offset;
int index = -1;

sprintf_s (p, 5, pszExt);
if (pszSubFile)
	FindFileData (pszFile, message, &size, &offset, TRUE, &fp);
else {
	index = FindFilename (message);
	if (index < 0)
		size = offset = -1;
	else
		GetFileData (index, &size, &offset);
	}
if ((size <= 0) && (offset < 0)) 
	return 0;
fp.Seek (sizeof (struct level_header) + offset, SEEK_SET);
return size;
}

//------------------------------------------------------------------------------
// CHogManager - load level
//------------------------------------------------------------------------------

bool CHogManager::LoadLevel (LPSTR pszFile, LPSTR pszSubFile) 
{
	CFileManager	fSrc;
	long				size, offset;
	int				index = -1;
	bool				funcRes = false;

if (!pszFile)
	pszFile = m_pszFile;
if (pszSubFile) {
	if (!FindFileData (pszFile, pszSubFile, &size, &offset))
		return false;
	strcpy_s (m_pszSubFile, 256, pszSubFile);
	}
else if (0 > (index = GetFileData (-1, &size, &offset)))
	goto errorExit;

if (fSrc.Open (pszFile, "rb")) {
	ErrorMsg ("Unable to load level.");
	goto errorExit;
	}
// set subfile name
fSrc.Seek (sizeof (struct level_header) + offset, SEEK_SET);
// skip mineDataOffset and gameDataOffset
if (theMine->LoadMineSigAndType (&fSrc))
	goto errorExit;
fSrc.ReadInt32 ();
fSrc.ReadInt32 ();
theMine->LoadPaletteName (&fSrc);
fSrc.Seek (sizeof (struct level_header) + offset, SEEK_SET);
m_level.Load (&fSrc, size);
//while (size > 0) {
//	int chunk = (size > sizeof (dataBuf)) ? sizeof (dataBuf) : size;
//	fSrc.Read (dataBuf, 1, chunk);
//	//if (!chunk)
//	//	break;
//	fTmp.Write (dataBuf, 1, chunk);
//	size -= chunk;
//	}

if (0 < (size = FindSubFile (fSrc, pszFile, pszSubFile, ".pal")))
	paletteManager.LoadCustom (&fSrc, size);
if (0 < (size = FindSubFile (fSrc, pszFile, pszSubFile, ".lgt")))
	lightManager.ReadLightMap (fSrc, size);
if (0 < (size = FindSubFile (fSrc, pszFile, pszSubFile, ".clr")))
	lightManager.ReadColorMap (fSrc);
paletteManager.Reload ();
textureManager.LoadTextures ();
if (0 < (size = FindSubFile (fSrc, pszFile, pszSubFile, ".pog")))
	textureManager.ReadPog (fSrc, size);
if (0 < (size = FindSubFile (fSrc, pszFile, pszSubFile, ".hxm"))) {
	robotManager.ReadHXM (fSrc, size);
	int nCustom = 0;
	for (int i = 0; i < (int) robotManager.Count (); i++)
		if (robotManager.RobotInfo (i)->Info ().bCustom)
			nCustom++;
	sprintf_s (message, sizeof (message)," Hog manager: %d custom robots read", nCustom);
	DEBUGMSG (message);
	}
funcRes = true;

errorExit:

return funcRes;
}

//------------------------------------------------------------------------------
// CHogManager::Ok()
//
// Saves fp to a temporary fp called dle_temp.rdl so editor can load it
//------------------------------------------------------------------------------

void CHogManager::OnOK () 
{
LBFiles ()->GetText (LBFiles ()->GetCurSel (), m_pszSubFile);
char *pszExt = strrchr ((char *) m_pszSubFile, '.');
if (pszExt && _strcmpi (pszExt,".rdl") && _strcmpi (pszExt,".rl2")) {
	ErrorMsg ("DLE-XP cannot process this file. To change the file,\n\n"
				 "export it and process it with the appropriate application.\n"
				 "To incorporate the changes in the HOG file,\n"
				 "import the modified file back into the HOG file.");
	return;
	}

LoadLevel ();
CDialog::OnOK ();
}

//------------------------------------------------------------------------------
// CHogManager::RenameMsg()
//------------------------------------------------------------------------------

void CHogManager::Rename (CFileManager& fp, int index, char* szNewName)
{
	long size, offset;
	level_header lh;
	int fileno = GetFileData (index, &size, &offset);

fp.Seek (offset, SEEK_SET);
if (!fp.Read (&lh, sizeof (lh), 1))
	ErrorMsg ("Cannot read from HOG file");
else {
	memset (lh.name, 0, sizeof (lh.name));
	szNewName [12] = '\0';
	_strlwr_s (szNewName, sizeof (lh.name));
	strncpy_s (lh.name, sizeof (lh.name), szNewName, 12);
	fp.Seek (offset, SEEK_SET);
	if (!fp.Write (&lh, sizeof (lh), 1))
		ErrorMsg ("Cannot write to HOG file");
	else {
		// update list box
		DeleteFile (index);
		AddFile (lh.name, sizeof (lh.name), size, offset, fileno);
		}
	}
}

//------------------------------------------------------------------------------

void CHogManager::OnRename (void)
{
	char szOldName [20], szNewName [20];
	int index = LBFiles ()->GetCurSel ();

if (index < 0)
	return;
LBFiles ()->GetText (index, szOldName);
strcpy_s (szNewName, sizeof (szNewName), szOldName);
CInputDialog dlg (this, "Rename file", "Enter new name:", (char *) szNewName, sizeof (szNewName));
if (dlg.DoModal () != IDOK)
	return;
if (FindFilename (szNewName) >= 0) {
	ErrorMsg ("A file with that name already exists\nin the HOG file.");
	return;
	}
CFileManager fp;
if (fp.Open (m_pszFile, "r+b")) {
	ErrorMsg ("Could not open HOG file.");
	return;
	}
Rename (fp, index, szNewName);

// if renamed file was a level file, rename all auxiliary files belonging to that level, too
char* p = strstr (szOldName, ".rl2");
if (p != null) {
		static char* subFileExts [] = {".pal", ".lgt", ".clr", ".pog", ".hxm"};

	for (int i = 0; i < sizeofa (subFileExts); i++) {
		if (0 < FindSubFile (fp, m_pszFile, szOldName, subFileExts [i])) {
			p = strchr (szOldName, '.');
			memcpy (p, subFileExts [i], sizeof (subFileExts [i]));
			index = LBFiles ()->FindStringExact (-1, szOldName);
			if (index >= 0) {
				p = strchr (szNewName, '.');
				memcpy (p, subFileExts [i], sizeof (subFileExts [i]));
				Rename (fp, index, szNewName);
				}
			}
		}
	p = strchr (szNewName, '.');
	memcpy (p, ".rl2", 5);
	index = LBFiles ()->FindStringExact (-1, szNewName);
	if (index >= 0)
		LBFiles ()->SetCurSel (index);
	}
fp.Close ();
}

//------------------------------------------------------------------------------
// CHogManager - ImportMsg
//
// Adds fp to the end of the list
//------------------------------------------------------------------------------

void CHogManager::OnImport (void) 
{
	long offset;

	char szFile[256] = "\0"; // buffer for fp name
	level_header lh;
	OPENFILENAME ofn;

memset(&ofn, 0, sizeof (OPENFILENAME));
ofn.lStructSize = sizeof (OPENFILENAME);
ofn.hwndOwner = GetSafeHwnd ();
ofn.lpstrFilter = "Descent Level\0*.rdl\0"
						"Descent 2 Level\0*.rl2\0"
						"Texture file\0*.pog\0"
						"Robot file\0*.hxm\0"
						"Lightmap file\0*.lgt\0"
						"Color file\0*.clr\0"
						"Palette file\0*.pal\0"
						"All Files\0*.*\0";
if (DLE.IsD1File ()) {
	ofn.nFilterIndex = 1;
	ofn.lpstrDefExt = "rdl";
	}
else {
	ofn.nFilterIndex = 2;
	ofn.lpstrDefExt = "rl2";
	}
ofn.lpstrFile= szFile;
ofn.nMaxFile = sizeof (szFile);
ofn.lpstrFileTitle = lh.name;
ofn.nMaxFileTitle = sizeof (lh.name);
ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
if (!GetOpenFileName (&ofn))
	return;

CFileManager fDest;
if (fDest.Open (m_pszFile, "ab")) {
	ErrorMsg ("Could not open destination HOG file for import.");
	return;
	}

CFileManager fSrc;
if (fSrc.Open (szFile, "rb")) {
	ErrorMsg ("Could not open source file for import.");
	return;
	}
fDest.Seek (0, SEEK_END);
offset = fDest.Tell ();
// write header
lh.size = fSrc.Length ();
_strlwr_s (lh.name, sizeof (lh.name));
fDest.Write (&lh, sizeof (level_header), 1);

	// write data (from source to HOG)
while (!fSrc.EoF ()) {
	size_t nBytes = fSrc.Read (dataBuf, 1, sizeof (dataBuf));
	if (nBytes <= 0)
		break;
	fDest.Write (dataBuf, 1, nBytes);
	}
fSrc.Close ();
fDest.Close ();
// update list boxes
AddFile (lh.name, sizeof (lh.name), lh.size, offset, LBFiles ()->GetCount ());
}

//------------------------------------------------------------------------------
// CHogManager - ExportMsg
//
// Exports selected item to a fp
//------------------------------------------------------------------------------

void CHogManager::OnExport (void) 
{
	char szFile[256] = "\0"; // buffer for fp name
	DWORD index;
	level_header lh;
	long size, offset;

// make sure there is an item selected
index = LBFiles ()->GetCurSel ();
if (index < 0) {
	ErrorMsg ("Please select a fp to export.");
	return;
	}
// get item name, size, and offset
LBFiles ()->GetText (index, lh.name);
GetFileData (index, &size, &offset);
lh.size = size;
offset += sizeof (level_header);
strcpy_s (szFile, sizeof (szFile), lh.name);
if (!BrowseForFile (FALSE, "", szFile, "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT,
						  DLE.MainFrame ()))
	return;
ExportSubFile (m_pszFile, szFile, offset, lh.size);
}

//------------------------------------------------------------------------------
// CHogManager - DeleteMsg
//
// Deletes an item from a HOG file using the following steps:
//
// 1) Creates a new HOG file which does not contain the fp selected
// 2) Deletes original HOG file
// 3) Renames new fp to original fp's name
//
//------------------------------------------------------------------------------

void CHogManager::OnDelete (void) 
{
	int delete_index;
	long size;
	long offset;
	int fileno;
	CFileManager fp;
	CListBox * plb = LBFiles ();

// make sure there is an item selected
delete_index = plb->GetCurSel ();
if (delete_index < 0) {
	ErrorMsg ("Please choose a fp to delete.");
	return;
	}
#if 1//ndef _DEBUG
if (!bExpertMode) {
	strcpy_s (message, sizeof (message), "Are you sure you want to delete '");
	plb->GetText (delete_index, message + strlen (message));
	strcat_s (message, sizeof (message), "'?\n(This operation will change the HOG file immediately)");
	if (QueryMsg (message) != IDYES)
		return;
	}
#endif
fileno = GetFileData (delete_index, &size, &offset);
int nFiles = plb->GetCount ();
// open hog fp for modification
if (fp.Open (m_pszFile, "r+b")) {
	ErrorMsg ("Could not open hog fp.");
	return;
	}
DeleteSubFile (fp, size + sizeof (struct level_header), offset, nFiles, fileno);
fp.Close ();
ReadHogData ();
LBFiles ()->SetCurSel ((delete_index < nFiles - 1) ? delete_index : nFiles - 1);
}

//------------------------------------------------------------------------------
// CHogManager - read hog data
//------------------------------------------------------------------------------

bool ReadHogData (LPSTR pszFile, CListBox *plb, bool bAllFiles, bool bOnlyLevels, bool bGetFileData) 
{
	struct level_header *level;
	char data [256];
	long position;
	byte index;
	CFileManager fp;
	int nFiles;

ClearFileList (plb);
if (fp.Open (pszFile, "rb")) {
	sprintf_s (message, sizeof (message), "Unable to open HOG file (%s)",pszFile);
	ErrorMsg (message);
	return false;
	}
fp.Read (data, 3, 1); // verify signature "DHF"
if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') {
	ErrorMsg ("This is not a Descent HOG file");
	return false;
	}
position = 3;
nFiles = 0;
while (!fp.EoF ()) {
	fp.Seek (position, SEEK_SET);
	if (fp.Read (data, sizeof (struct level_header), 1) != 1) 
		break;
	level = (struct level_header *) data;
	if (level->size < 0) {
		ErrorMsg ("Error reading HOG file");
		fp.Close ();
		return false;
		}
	level->name [sizeof (level->name) - 1] = 0; // null terminate in case its bad
	_strlwr_s (level->name, sizeof (level->name));
	if (bAllFiles 
		 || strstr (level->name, ".rdl") || strstr (level->name, ".rl2")
		 || (!bOnlyLevels && (strstr (level->name, ".pog") || 
									 strstr (level->name, ".hxm") || 
									 strstr (level->name, ".lgt") || 
									 strstr (level->name, ".clr") || 
									 strstr (level->name, ".pal")))) {
		int i = plb->AddString (level->name);
		if (bGetFileData && (0 > AddFileData (plb, i, level->size, position, nFiles))) {
			ErrorMsg ("Too many files in HOG file.");
			fp.Close ();
			return false;
			}
		plb->SetCurSel (i);
		nFiles++;
		}
	position += sizeof (struct level_header) + level->size;
	}
fp.Close ();
// select first level fp
for (index = 0; index < nFiles; index++) {
	message [0] = null;
	plb->GetText (index, message);
	_strlwr_s (message, sizeof (message));
	if (strstr (message, ".rdl")) 
		break;
	if (strstr (message, ".rl2")) 
		break;
	}
if (index == nFiles)
	index = 0;
plb->SetCurSel (index);
return true;
}

//------------------------------------------------------------------------------
// CHogManager - read hog data
//------------------------------------------------------------------------------

bool FindFileData (LPSTR pszFile, LPSTR pszSubFile, long *nSize, long *nPos, BOOL bVerbose, CFileManager* fp) 
{
	struct level_header *level;
	char data [256];
	long position;
	int nFiles;
	CFileManager _fp;

*nSize = -1;
*nPos = -1;
if (fp != null)
	fp->Seek (0);
else {
	fp = &_fp;
	if (fp->Open (pszFile, "rb")) {
		if (bVerbose) {
			sprintf_s (message, sizeof (message), "Unable to open HOG file (%s)\n(%s)", pszFile, strerror (errno));
			ErrorMsg (message);
			}
		return false;
		}
	}
fp->Read (data, 3, 1); // verify signature "DHF"
if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') {
	if (bVerbose)
		ErrorMsg ("This is not a Descent HOG file");
	return false;
	}
position = 3;
nFiles = 0;
while (!fp->EoF ()) {
	fp->Seek (position, SEEK_SET);
	if (fp->Read (data, sizeof (struct level_header), 1) != 1) 
		break;
	level = (struct level_header *) data;
	if (level->size > 100000000L || level->size < 0) {
		if (bVerbose)
			ErrorMsg ("Error reading HOG file");
		if (fp == &_fp)
			fp->Close ();
		return false;
		}
	level->name [sizeof (level->name) - 1] = 0; // null terminate in case its bad
	_strlwr_s (level->name, sizeof (level->name));
	if (!strcmp (pszSubFile, "*"))
		strcpy_s (pszSubFile, 256, level->name);
	if (!_strcmpi (level->name, pszSubFile)) {
		*nSize = level->size;
		*nPos = position;
		if (fp == &_fp)
			fp->Close ();
		return true;
		}
	position += sizeof (struct level_header) + level->size;
	}
if (fp == &_fp)
	fp->Close ();
return false;
}

//------------------------------------------------------------------------------
// extract_from_hog()
//------------------------------------------------------------------------------

bool ExportSubFile (const char *pszSrc, const char *pszDest, long offset, long size) 
{
CFileManager fSrc;
if (fSrc.Open (pszSrc, "rb")) {
	ErrorMsg ("Could not open HOG file.");
	return false;
	}
CFileManager fDest;
if (fDest.Open (pszDest, "wb")) {
	ErrorMsg ("Could not create export file.");
	return false;
	}
// seek to item's offset in HOG file
fSrc.Seek (offset, SEEK_SET);
// create file (from HOG to file)
while (size > 0) {
	size_t nBytes, n;
	n = (size > sizeof (dataBuf)) ? sizeof (dataBuf) : size_t (size);
	nBytes = fSrc.Read (dataBuf, 1, n);
	fDest.Write (dataBuf, 1, nBytes);
	size -= long (nBytes);
	if (nBytes != n)
		break;
	}
fDest.Close ();
fSrc.Close ();
return (size == 0);
}


//----------------------------------------------------------------------------------
// delete_sub_file()
//----------------------------------------------------------------------------------

void DeleteSubFile (CFileManager& fp, long size, long offset, int num_entries, int delete_index) 
{
int nBytes;
// as long as we are not deleting the last item
if (delete_index < num_entries - 1) {
	// get size of chunk to remove from the fp, then move everything
	// down by that amount.
	do {
		fp.Seek (offset + size, SEEK_SET);
		nBytes = fp.Read (dataBuf, 1, sizeof (dataBuf));
		if (nBytes <= 0)
			break;
		fp.Seek (offset, SEEK_SET);
		fp.Write (dataBuf, 1, nBytes);
		offset += nBytes;
		} while (nBytes > 0);
	}
// set the new size of the fp
_chsize (_fileno (fp.File ()), offset);
}

//--------------------------------------------------------------------------------
// strip_extension
//--------------------------------------------------------------------------------

void strip_extension(char *str) 
{
char *ext = strrchr(str,'.');
if (ext)
	*ext = '\0';
}

//--------------------------------------------------------------------------------
// DeleteLevelSubFiles()
//
// deletes sub-files with same base name from hog
//--------------------------------------------------------------------------------

#define MAX_REGNUM 6

void DeleteLevelSubFiles (CFileManager& fp, char *base) 
{
struct region {
	int index;
	int offset;
	int size;
	int files;
	};
	int regnum = 0;
	int delete_index = 0;
	int num_entries = 0;
	region reg [MAX_REGNUM] = {{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0}};

// figure out regions of the fp to delete (3 regions max)
long offset = 3;
long size;
delete_index = -1;
while(!fp.EoF ()) {
	level_header lh = {"",0};
	fp.Seek (offset, SEEK_SET); // skip "HOG"
	if (!fp.Read (&lh, sizeof (lh), 1)) 
		break;
	size = lh.size + sizeof (lh);
	if (regnum < MAX_REGNUM) {
		_strlwr_s (lh.name, sizeof (lh.name));
		LPSTR ext = strrchr (lh.name, '.');
		if (!ext)
			goto nextSubFile;
		if (_strcmpi (ext, ".rdl") &&
		    _strcmpi (ext, ".rl2") &&
		    _strcmpi (ext, ".hxm") &&
		    _strcmpi (ext, ".pog") &&
		    _strcmpi (ext, ".pal") &&
		    _strcmpi (ext, ".lgt") &&
		    _strcmpi (ext, ".clr"))
			goto nextSubFile;
		*ext = '\0';
		if (_strcmpi (base, lh.name))
			goto nextSubFile;
		// try to merge this with the last region
		if ((regnum > 0) && (delete_index == num_entries - 1)) {
			reg[regnum-1].size += size;
			reg[regnum-1].files++;
			num_entries--; // pretend that there is one less entry
			}
		else {
			reg[regnum].index = num_entries;
			reg[regnum].offset = offset;
			reg[regnum].size = size;
			reg[regnum].files++;
			regnum++;
			}
		delete_index = num_entries;
		}
nextSubFile:
	offset += size;
	num_entries++;
	}

// now delete matching regions
while (regnum > 0) {
	regnum--;
	DeleteSubFile (fp, reg [regnum].size, reg [regnum].offset, num_entries, reg [regnum].index);
	num_entries -= reg [regnum].files;
	}
fp.Close ();
}

#undef MAX_REGNUM

//------------------------------------------------------------------------------
// write_sub_file()

int WriteSubFile (CFileManager& fDest, char *szSrc, char *szLevel) 
{
	CFileManager	fSrc;
	size_t			nBytes;
	level_header	lh;

if (fSrc.Open (szSrc, "rb")) {
	sprintf_s (message, sizeof (message), "Unable to open temporary fp:\n%s",szSrc);
	ErrorMsg (message);
	return -1;
	}
// write szLevel (13 chars, null filled)
memset (&lh, 0, sizeof (lh));
strncpy_s (lh.name, sizeof (lh.name), szLevel, 12);
_strlwr_s (lh.name, sizeof (lh.name));
// calculate then write size
lh.size = fSrc.Size ();
//fclose (fSrc);
//fSrc = fopen (szSrc,"rb");
fDest.Write (&lh, sizeof (lh), 1);
// write data
for (int l = fSrc.Size (); l > 0; l -= sizeof (dataBuf)) {
	nBytes = fSrc.Read (dataBuf, 1, min (l, sizeof (dataBuf)));
	if (nBytes > 0)
		fDest.Write (dataBuf, 1, nBytes);
	}
fSrc.Close ();
return lh.size + sizeof (lh);
}

//------------------------------------------------------------------------------
// make_hog()
//
// Action - makes a HOG file which includes three files
//             1. the rdl fp to include (only one)
//             2. a briefing fp (called brief.txb)
//             3. an ending sequence (same name as hog w/ .txb extension)
//          also makes a mission file for this HOG file
//
// Changes - now saves rl2 files
//------------------------------------------------------------------------------

typedef int (* subFileWriter) (CFileManager&);

//--------------------------------------------------------------------------------

int WriteCustomFile (CFileManager&fp, const int nType, const char* szFolder, const char* szFile)
{
	static char* extensions [] = {".lgt", ".clr", ".pal", ".pog", ".hxm"};

	CFileManager fTmp;
	char szTmp [256], szDest [256];

CFileManager::SplitPath (szFolder, szTmp, null, null);
sprintf_s (szTmp, sizeof (szTmp), "%sdle_temp%s", szFolder, extensions [nType]);
if (fTmp.Open (szTmp, "wb"))
	return 1;
int i = 0;
switch (nType) {
	case 0:
		lightManager.WriteLightMap (fTmp);
		break;
	case 1:
		lightManager.WriteColorMap (fTmp);
		break;
	case 2:
		i = paletteManager.SaveCustom (&fTmp);
		break;
	case 3:
		i = textureManager.CreatePog (fTmp);
		break;
	case 4:
		i = robotManager.WriteHXM (fTmp);
		break;
	default:
		i = 1;
		break;
	}
fTmp.Close ();
if (!i) {
	sprintf_s (szDest, sizeof (szDest), "%s%s", szFile, extensions [nType]);
	i = WriteSubFile (fp, szTmp, szDest);
	}
CFileManager::Delete (szTmp);
return i;
}

//--------------------------------------------------------------------------------

void WriteCustomFiles (CFileManager& fp, char* szFolder, char* szFile, bool bCreate = false)
{
	static char* szPogQuery = "This level contains custom textures.\nWould you like save these textures into the HOG file?\n\nNote: You must use version 1.2 or higher of Descent2 to see\nthe textures when you play the game.";
	static char* szHxmQuery = "This level contains custom robot settings.\nWould you like save these changes into the HOG file?\n\nNote: You must use version 1.2 or higher of Descent2 for\nthe changes to take effect.";

if (lightManager.HasCustomLightMap ())
	WriteCustomFile (fp, 0, szFolder, szFile);
if (lightManager.HasCustomLightColors ())
	WriteCustomFile (fp, 1, szFolder, szFile);
if (paletteManager.Custom () != null)
	WriteCustomFile (fp, 2, szFolder, szFile);
if (textureManager.HasCustomTextures () && (!bCreate || bExpertMode || QueryMsg (szPogQuery) == IDYES)) 
	WriteCustomFile (fp, 3, szFolder, szFile);
if (robotManager.HasCustomRobots () && (!bCreate || bExpertMode || QueryMsg (szHxmQuery) == IDYES)) 
	WriteCustomFile (fp, 4, szFolder, szFile);
}

//--------------------------------------------------------------------------------

int MakeHog (char *rdlFilename, char *hogFilename, char*szSubFile, bool bSaveAs) 
{
	CFileManager	fp, fTmp;
	char				*pszNameStart, *pszNameEnd, *pszExtStart;
	char				szFolder [256], szFile [256], szExt [256], szTmp [256];
	int				custom_robots = 0;
	int				custom_textures = 0;

// create HOG file which contains szTmp.rdl, szTmp.txb, and dlebrief.txb");
if (fp.Open (hogFilename, "wb")) {
	sprintf_s (message, sizeof (message), "Unable to create HOG file:\n%s", hogFilename);
	ErrorMsg (message);
	return 1;
	}
// write fp type
fp.Write ("DHF", 1, 3); // starts with Descent Hog File
// get base szTmp w/o extension and w/o path
memset (szFile, 0, 13);
CFileManager::SplitPath (hogFilename, szFolder, szFile, szExt);
szFile [12] = 0;
pszNameStart = strrchr(hogFilename,'\\');
if (pszNameStart == null)
	pszNameStart = hogFilename;
else
	pszNameStart++; // move to just pass the backslash
strncpy_s (szFile, sizeof (szFile), pszNameStart, 12);
szFile[12] = null; // make sure it is null terminated
pszNameEnd = strrchr ((char *)szFile,'.');
if (!pszNameEnd)
	pszNameEnd = szFile + strlen ((char *)szFile);
memset (pszNameEnd, 0, 12 - int (pszNameEnd - szFile));
// write rdl file
if (*szSubFile) {
	CFileManager::SplitPath (szSubFile, null, szFile, null);
	for (pszExtStart = szSubFile; *pszExtStart && (*pszExtStart != '.'); pszExtStart++)
		;
	strncpy_s (szFile, sizeof (szFile), szSubFile, pszExtStart - szSubFile);
	szFile [pszExtStart - szSubFile] = '\0';
	}

sprintf_s (szTmp, sizeof (szTmp), DLE.IsD1File () ? "%s.rdl" : "%s.rl2", szFile);
WriteSubFile (fp, rdlFilename, szTmp);
CFileManager::Delete (szTmp);

WriteCustomFiles (fp, szFolder, szFile, true);

fp.Close ();
MakeMissionFile (hogFilename, szSubFile, custom_textures, custom_robots, bSaveAs);
return 0;
}

//==========================================================================
// MENU - Save
//==========================================================================

int SaveToHog (LPSTR szHogFile, LPSTR szSubFile, bool bSaveAs) 
{
	CFileManager	fTmp;
	char				szFolder [256], szFile [256], szTmp [256];
	char*				psz;

_strlwr_s (szHogFile, 256);
psz = strstr (szHogFile, "new.");
if (!*szSubFile || psz) { 
	CInputDialog dlg (DLE.MainFrame (), "Name mine", "Enter file name:", szSubFile, 9);
	if (dlg.DoModal () != IDOK)
		return 1;
	LPSTR ext = strrchr (szSubFile, '.');
	if (ext)
		*ext = '\0';
	psz = strrchr (szSubFile, '.');
	if (psz)
		*psz = '\0';
	psz = strstr (szHogFile, "new.");
	if (psz) {
		int l = psz - szHogFile;
		strcpy_s (psz, 256 - l, szSubFile);
		l += strlen (szSubFile);
		strcat_s (psz, 256 - l, ".hog");
		}
	strcat_s (szSubFile, 256, (DLE.IsD1File ()) ? ".rdl" : ".rl2");
	}
// if this HOG file only contains one rdl/rl2 fp total and
// it has the same name as the current level, and it has
// no other files (besides hxm or pog files), then
// allow quick save
CFileManager fp;
// See if another level with the same name exists
// and see if there are any other files here (ignore hxm and pog files)
CFileManager::SplitPath (szHogFile, szFolder, null, null);
int bOtherFilesFound = 0;
int bIdenticalLevelFound = 0;
if (fp.Open (szHogFile, "rb")) {
	strcat_s (szFolder, sizeof (szFolder), "dle_temp.rdl");
	theMine->Save (szFolder);
	return MakeHog (szFolder, szHogFile, szSubFile, true);
	}

fp.Seek (3,SEEK_SET); // skip "HOG"
while (!fp.EoF ()) {
	level_header lh = {"",0};
	if (!fp.Read (&lh, sizeof (lh), 1)) 
		break;
	_strlwr_s (lh.name, sizeof (lh.name));
	lh.name [sizeof (lh.name) - 1] = null; // null terminate
	if (!strstr ((char *) lh.name, ".hxm") &&     // not a custom robot fp
		 !strstr ((char *) lh.name, ".pog") &&     // not a custom texture fp
		 !strstr ((char *) lh.name, ".lgt") &&     // not an texture brightness table fp
		 !strstr ((char *) lh.name, ".clr") &&     // not an texture color table fp
		 !strstr ((char *) lh.name, ".pal") &&     // not a palette fp
		 _strcmpi((char *) lh.name, szSubFile)) // not the same level
		bOtherFilesFound = 1;
	if (!_strcmpi ((char *) lh.name, szSubFile)) // same level
		bIdenticalLevelFound = 1;
	fp.Seek (lh.size, SEEK_CUR);
	}
fp.Close ();

// if no other files found
// then simply do a quick save
int bQuickSave = 0;
if (!bOtherFilesFound)
	bQuickSave = 1;
else if (bIdenticalLevelFound) {
	if (QueryMsg ("Overwrite old level with same name?") != IDYES)
		return 1;
	}
else {
	// otherwise, if save fp was not found,
	// then ask user if they want to append to the HOG file
	if (QueryMsg ("Would you like to add the level to the end\n"
					  "of this HOG file?\n\n"
					  "(Press OK to append this level to the HOG file\n"
					  "or Cancel to overwrite the entire HOG file") == IDYES) {
		if (!bExpertMode)
			ErrorMsg ("Don't forget to add this level's name to the mission file.\n");
		}
	else
		bQuickSave = 1;
	}
if (bQuickSave) {
	strcat_s (szFolder, sizeof (szFolder), "dle_temp.rdl");
	theMine->Save (szFolder);
	return MakeHog (szFolder, szHogFile, szSubFile, bSaveAs);
//	MySetCaption (szHogFile);
	}

// determine base name
CFileManager::SplitPath (szSubFile, null, szFile, null);
szFile [8] = null;
_strlwr_s (szFile, sizeof (szFile));

if (fp.Open (szHogFile, "r+b")) {
	ErrorMsg ("Destination HOG file not found or inaccessible.");
	return 1;
	}
DeleteLevelSubFiles (fp, szFile);
fp.Close ();
// now append sub-files to the end of the HOG file

if (fp.Open (szHogFile, "ab")) {
	ErrorMsg ("Could not open destination HOG file for save.");
	return 1;
	}
fp.Seek (0, SEEK_END);
sprintf_s (szTmp, sizeof (szTmp), "%sdle_temp.rdl", szFolder);
theMine->Save (szTmp, true);
WriteSubFile (fp, szTmp, szSubFile);
CFileManager::Delete (szTmp);

WriteCustomFiles (fp, szFolder, szFile);

fp.Close ();
return 0;
}

//------------------------------------------------------------------------------
// write_mission_file()

static LPSTR szMissionName [] = {"name", "zname", "d2x-name", null};
static LPSTR szMissionInfo [] = {"editor", "build_time", "date", "revision", "author", "email", "web_site", "briefing", null};
static LPSTR szMissionType [] = {"type", null};
static LPSTR szMissionTypes [] = {"anarchy", "normal", null};
static LPSTR szMissionFlags [] = {"normal", "anarchy", "robo_anarchy", "coop", "capture_flag", "hoard", null};
static LPSTR szCustomFlags [] = {"custom_textures", "custom_robots", "custom_music", null};
static LPSTR szAuthorFlags [] = {"multi_author", "want_feedback", null};
static LPSTR szNumLevels [] = {"num_levels", null};
static LPSTR szNumSecrets [] = {"num_secrets", null};
static LPSTR szBool [] = {"no", "yes", null};

static LPSTR *szTags [] = {szMissionName, szMissionInfo, szMissionType, szMissionFlags, szCustomFlags, szAuthorFlags, szNumLevels, szNumSecrets};

//------------------------------------------------------------------------------

int atob (LPSTR psz, size_t nSize)
{
_strlwr_s (psz, nSize);
int i;
for (i = 0; i < 2; i++)
	if (!strcmp (psz, szBool [i]))
		return i;
return 0;
}

//------------------------------------------------------------------------------

int ReadMissionFile (char *pszFile) 
{
	FILE	*fMsn;
	char  szMsn [256];
	LPSTR	psz, *ppsz;
	char	szTag [20], szValue [80], szBuf [100];
	int	i, j, l;

strcpy_s (szMsn, sizeof (szMsn), pszFile);
char *pExt = strrchr (szMsn, '.');
if (pExt)
	*pExt = '\0';
strcat_s (szMsn, sizeof (szMsn), (DLE.IsD1File ()) ? ".msn" : ".mn2");
fopen_s (&fMsn, szMsn, "rt");
if (!fMsn) {
	DEBUGMSG (" Hog manager: Mission fp not found.");
	return -1;
	}
memset (missionData.comment, 0, sizeof (missionData.comment));
l = 0;
while (fgets (szBuf, sizeof (szBuf), fMsn)) {
	if ((psz = strstr (szBuf, "\r\n")) || (psz = strchr (szBuf, '\n')))	//replace cr/lf
		*psz = '\0';
	if (*szBuf == ';') {	//comment
		if (l) {
			strncpy_s (missionData.comment + l, sizeof (missionData.comment) - l, "\r\n", sizeof (missionData.comment) - l);
			l += 2;
			}
		strncpy_s (missionData.comment + l, sizeof (missionData.comment) - l, szBuf + 1, sizeof (missionData.comment) - l);
		l += int (strlen (szBuf + 1));
		continue;
		}
	if (!(psz = strchr (szBuf, '=')))	// otherwise need <tag> '=' <value> format
		continue;
	if (psz - szBuf >= sizeof (szTag))	// invalid keyword
		continue;
	for (i = -1; psz + i > szBuf; i--)	// remove blanks around '='
		if (psz [i] != ' ') {
			psz [++i] = '\0';
			strncpy_s (szTag, sizeof (szTag), szBuf, sizeof (szTag));
			szTag [sizeof (szTag) - 1] = '\0';
			break;
			}
	for (i = 1; psz [i]; i++)
		if (psz [i] != ' ') {
			strncpy_s (szValue, sizeof (szValue), psz + i, sizeof (szValue));
			szValue [sizeof (szValue) - 1] = '\0';
			break;
			}
	if (!(*szTag && *szValue))	// mustn't be empty
		continue;
	_strlwr_s (szTag, sizeof (szTag));	// find valid tag
	for (i = 0, j = -1; i < 8; i++)
		for (ppsz = szTags [i]; *ppsz; ppsz++)
			if (!strcmp (*ppsz, szTag)) {
				j = int (ppsz - szTags [i]);
				goto tagFound;
				}
	continue;
tagFound:
	switch (i) {
		case 0:
			strcpy_s (missionData.missionName, sizeof (missionData.missionName), szValue);
			break;
		case 1:
			strcpy_s (missionData.missionInfo [j], sizeof (missionData.missionInfo [j]), szValue);
			break;
		case 2:
			_strlwr_s (szValue, sizeof (szValue));
			for (j = 0; j < 2; j++)
				if (!strcmp (szValue, szMissionTypes [j]))
					missionData.missionType = j;
			break;
		case 3:
			missionData.missionFlags [j] = atob (szValue, sizeof (szValue));
			break;
		case 4:
			missionData.customFlags [j] = atob (szValue, sizeof (szValue));
			break;
		case 5:
			missionData.authorFlags [j] = atob (szValue, sizeof (szValue));
			break;
		case 6:
			missionData.numLevels = atol (szValue);
			for (i = 0; i < missionData.numLevels; i++) {
				fgets (missionData.levelList [i], sizeof (missionData.levelList [i]), fMsn);
				for (j = int (strlen (missionData.levelList [i])); --j; )
					if ((missionData.levelList [i][j] != '\r') &&
						 (missionData.levelList [i][j] != '\n'))
						break;
				missionData.levelList [i][j+1] = '\0';
				_strlwr_s (missionData.levelList [i], sizeof (missionData.levelList [i]));
				}
			break;
		case 7:
			missionData.numSecrets = atol (szValue);
			for (i = 0, j = missionData.numLevels; i < missionData.numSecrets; i++, j++) {
				fscanf_s (fMsn, "%s", missionData.levelList [j], sizeof (missionData.levelList [j]));
				_strlwr_s (missionData.levelList [i], sizeof (missionData.levelList [i]));
				}
			break;
		default:
			continue;
		}
	}
fclose (fMsn);
return 0;
}

//------------------------------------------------------------------------------

int WriteMissionFile (char *pszFile, int levelVersion, bool bSaveAs) 
{
	FILE	*fMsn;
	char  szMsn [256];
	int	i, j;

strcpy_s (szMsn, sizeof (szMsn), pszFile);
char *pExt = strrchr (szMsn, '.');
if (pExt)
	*pExt = '\0';
strcat_s (szMsn, sizeof (szMsn), (DLE.IsD1File ()) ? ".msn" : ".mn2");
if (bSaveAs) {
	fopen_s (&fMsn, szMsn, "rt");
	if (fMsn) {
		fclose (fMsn);
		if (AfxMessageBox ("A mission file with that name already exists.\nOverwrite mission file?", MB_YESNO) != IDYES)
			return -1;
		}
	}
// create mission file
fopen_s (&fMsn, szMsn, "wt");
if (!fMsn)
	return -1;
if (levelVersion >= 9)
	fprintf (fMsn, "d2x-name = %s\n", missionData.missionName);
else if (levelVersion >= 8)
	fprintf (fMsn, "zname = %s\n", missionData.missionName);
else
	fprintf (fMsn, "name = %s\n", missionData.missionName);
fprintf (fMsn, "type = %s\n", szMissionTypes [missionData.missionType]);
fprintf (fMsn, "num_levels = %d\n", missionData.numLevels);
for (i = 0; i < missionData.numLevels; i++)
	fprintf (fMsn, "%s\n", missionData.levelList [i]);
if (missionData.numSecrets) {
	fprintf (fMsn, "num_secrets = %d\n", missionData.numSecrets);
	for (j = 0; j < missionData.numSecrets; i++, j++)
		fprintf (fMsn, "%s\n", missionData.levelList [i]);
	}
for (i = 0; i < 8; i++)
	if (*missionData.missionInfo [i])
		fprintf (fMsn, "%s = %s\n", szMissionInfo [i], missionData.missionInfo [i]);
for (i = 0; i < 3; i++)
	fprintf (fMsn, "%s = %s\n", szCustomFlags [i], szBool [missionData.customFlags [i]]);
for (i = 0; i < 6; i++)
	fprintf (fMsn, "%s = %s\n", szMissionFlags [i], szBool [missionData.missionFlags [i]]);
for (i = 0; i < 2; i++)
	fprintf (fMsn, "%s = %s\n", szAuthorFlags [i], szBool [missionData.authorFlags [i]]);
if (*missionData.comment) {
	char *pi, *pj;
	for (pi = pj = missionData.comment; ; pj++)
		if (!*pj) {
			fprintf (fMsn, ";%s\n", pi);
			break;
			}
		else if ((pj [0] == '\r') && (pj [1] == '\n')) {
			*pj = '\0';
			fprintf (fMsn, ";%s\n", pi);
			*pj++ = '\r';
			pi = pj + 1;
			}
	}
fclose (fMsn);
return 0;
}

//------------------------------------------------------------------------------
// make_mission_file()

int MakeMissionFile (char *pszFile, char *pszSubFile, int bCustomTextures, int bCustomRobots, bool bSaveAs) 
{
	char	szBaseName [256];
	char	szTime [20];

//memset (&missionData, 0, sizeof (missionData));
CFileManager::SplitPath (pszSubFile, null, szBaseName, null);
if (!*missionData.missionName)
	strcpy_s (missionData.missionName, sizeof (missionData.missionName), szBaseName);
if (bSaveAs || !*missionData.missionName)
	do {
		CInputDialog dlg (DLE.MainFrame (), "Mission title", "Enter mission title:", missionData.missionName, sizeof (missionData.missionName));
		if (dlg.DoModal () != IDOK)
			return -1;
	} while (!*missionData.missionName);
missionData.missionType = 1;
missionData.numLevels = 1;
strcpy_s (missionData.levelList [0], sizeof (missionData.levelList [0]), pszSubFile);
if (!strchr (pszSubFile, '.'))
	strcat_s (missionData.levelList [0], sizeof (missionData.levelList [0]), DLE.IsD2File () ? ".rl2" : ".rdl");
missionData.numSecrets = 0;
strcpy_s (missionData.missionInfo [0], sizeof (missionData.levelList [0]), "DLE-XP");
strcpy_s (missionData.missionInfo [2], sizeof (missionData.levelList [2]), DateStr (szTime, sizeof (szTime), true));
if (bSaveAs)
	strcpy_s (missionData.missionInfo [3], sizeof (missionData.missionInfo [3]), "1.0");
missionData.customFlags [0] = bCustomTextures;
missionData.customFlags [1] = bCustomRobots;
return WriteMissionFile (pszFile, DLE.LevelVersion (), bSaveAs);
}

//------------------------------------------------------------------------------
// eof fp.cpp