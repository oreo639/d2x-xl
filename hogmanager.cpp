// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <math.h>
#include <io.h>
#include <string.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "robot.h"
#include "textures.h"
#include "robot.h"
#include "dle-xp-res.h"
#include "io.h"
#include "hogmanager.h"
#include "light.h"
#include "palette.h"

// local globals
char *VERTIGO_HINT =
	"Vertigo levels allow you to use 12 robots in addition\n"
	"to the 66 standard Descent II robots.  But, these robots will\n"
	"not appear in the game unless you play the \"Descent 2: Vertigo\"\n"
	"mission after you start Descent II and before you play your level.\n";


char cut_paste_filename [256] = "";

// local prototypes
INT32 read_block_file(char *name,INT32 option);
void strip_extension(char *str);

void DeleteSubFile (FILE *file, long size, long offset, INT32 num_entries, INT32 delete_index);
void ImportSubFile ();
bool ExportSubFile (const char *pszSrc, const char *pszDest, long offset, long size);
void RenameSubFile ();
void DeleteLevelSubFiles(FILE *file,char *name);

                         /*--------------------------*/

static UINT8 dataBuf [65536];

                         /*--------------------------*/

INT32 AddFileData (CListBox *plb, INT32 index, INT32 size, INT32 offset, INT32 fileno)
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

                         /*--------------------------*/

void ClearFileList (CListBox *plb)
{
	INT32 i, h = plb->GetCount ();
	tHogFileData *pfd;

for (i = 0; i < h; i++)
	if (pfd = (tHogFileData *) plb->GetItemDataPtr (i))
		delete pfd;
plb->ResetContent ();
}

                         /*--------------------------*/

CInputDialog::CInputDialog (CWnd *pParentWnd, LPSTR pszTitle, LPSTR pszPrompt, LPSTR pszBuf, size_t nBufSize)
	: CDialog (IDD_INPDLG, pParentWnd)
{
m_pszTitle = pszTitle;
m_pszPrompt = pszPrompt;
m_pszBuf = pszBuf;
m_nBufSize = nBufSize;
}

                         /*--------------------------*/

BOOL CInputDialog::OnInitDialog (void)
{
CDialog::OnInitDialog ();
SetWindowText (m_pszTitle);
return TRUE;
}

                         /*--------------------------*/

void CInputDialog::DoDataExchange (CDataExchange * pDX)
{
DDX_Text (pDX, IDC_INPDLG_PROMPT, m_pszPrompt, INT32 (strlen (m_pszPrompt)));
DDX_Text (pDX, IDC_INPDLG_BUF, m_pszBuf, INT32 (m_nBufSize));
}

                         /*--------------------------*/

void CInputDialog::OnOK (void)
{
UpdateData (TRUE);
CDialog::OnOK ();
}

                         /*--------------------------*/

BEGIN_MESSAGE_MAP (CHogManager, CDialog)
	ON_BN_CLICKED (IDC_HOG_RENAME, OnRename)
	ON_BN_CLICKED (IDC_HOG_DELETE, OnDelete)
	ON_BN_CLICKED (IDC_HOG_IMPORT, OnImport)
	ON_BN_CLICKED (IDC_HOG_EXPORT, OnExport)
	ON_BN_CLICKED (IDC_HOG_FILTER, OnFilter)
	ON_LBN_SELCHANGE (IDC_HOG_FILES, OnSetFile)
	ON_LBN_DBLCLK (IDC_HOG_FILES, OnOK)
END_MESSAGE_MAP ()

                        /*--------------------------*/

bool BrowseForFile (BOOL bOpen, LPSTR pszDefExt, LPSTR pszFile, LPSTR pszFilter, DWORD nFlags, CWnd *pParentWnd)
{
   INT_PTR     nResult;
   char		   szFile [256], szFolder [256], * ps;

if (*pszFile)
	strcpy_s (szFolder, sizeof (szFolder), pszFile);
else {
	strcpy_s (szFolder, sizeof (szFolder), levels_path);
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

                         /*--------------------------*/

CHogManager::CHogManager (CWnd *pParentWnd, LPSTR pszFile, LPSTR pszSubFile)
	: CDialog (IDD_HOGMANAGER, pParentWnd) 
{
m_bInited = false;
m_pszFile = pszFile;
m_pszSubFile = pszSubFile;
m_bShowAll = false;
}

                         /*--------------------------*/

void CHogManager::EndDialog (INT32 nResult)
{
if (m_bInited)
	ClearFileList ();
CDialog::EndDialog (nResult);
}

                         /*--------------------------*/

void CHogManager::Reset (void)
{
/*
m_fileData.m_nFiles = 0;
INT32 i;
for (i = 0; i < MAX_HOGFILES - 1; i++)
	m_fileData.m_size [i] = i + 1;
m_fileData.m_size [i] = -1;
m_fileData.m_nFreeList = 0;
*/
}

                         /*--------------------------*/

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

                         /*--------------------------*/

void CHogManager::DoDataExchange (CDataExchange * pDX)
{
	long size, offset;

GetFileData (-1, &size, &offset);
DDX_Text (pDX, IDC_HOG_SIZE, size);
DDX_Text (pDX, IDC_HOG_OFFSET, offset);
DDX_Check (pDX, IDC_HOG_FILTER, m_bShowAll);
}

                         /*--------------------------*/

void CHogManager::OnCancel (void)
{
CDialog::OnCancel ();
}

                         /*--------------------------*/

void CHogManager::OnFilter (void)
{
m_bShowAll = ((CButton *) GetDlgItem (IDC_HOG_FILTER))->GetCheck ();
if (!ReadHogData ())
	EndDialog (0);
}

                         /*--------------------------*/

void CHogManager::OnSetFile (void)
{
UpdateData (FALSE);
}

                         /*--------------------------*/

void CHogManager::ClearFileList (void)
{
::ClearFileList (LBFiles ());
}

                         /*--------------------------*/

INT32 CHogManager::DeleteFile (INT32 index)
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

                         /*--------------------------*/

INT32 CHogManager::AddFile (LPSTR pszName, long size, long offset, INT32 fileno)
{
_strlwr_s (pszName, 256);

	INT32 index = LBFiles ()->AddString (pszName);

if (0 > AddFileData (index, size, offset, fileno))
	return -1;
LBFiles ()->SetCurSel (index);
return 0;
}

                         /*--------------------------*/

INT32 CHogManager::GetFileData (INT32 index, long *size, long *offset)
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

                         /*--------------------------*/

INT32 CHogManager::AddFileData (INT32 index, long size, long offset, INT32 fileno)
{
return ::AddFileData (LBFiles (), index, size, offset, fileno);
}

                         /*--------------------------*/

INT32 CHogManager::FindFilename (LPSTR pszName)
{
	CListBox	*plb = LBFiles ();
	char szName [256];

INT32 h, i;
for (h = plb->GetCount (), i = 0; i < h; i++) {
	plb->GetText (i, szName);
	if (!_strcmpi (szName, pszName))
		return i;
	}
return -1;
}

//------------------------------------------------------------------------
// CHogManager - read hog data
//------------------------------------------------------------------------

bool CHogManager::ReadHogData () 
{
ClearFileList ();
Reset ();
if (!::ReadHogData (m_pszFile, LBFiles (), m_bShowAll == 1, false))
	return false;
UpdateData (FALSE);
return true;
}

//------------------------------------------------------------------------
// CHogManager - load level
//------------------------------------------------------------------------

bool CHogManager::LoadLevel (LPSTR pszFile, LPSTR pszSubFile) 
{
	FILE*		fTmp = NULL, *fSrc = NULL;
	long		size,offset;
	char		szTmp[256];
	INT32		chunk;
	char*		pszExt;
	INT32		index = -1;
	bool		funcRes = false;

if (!pszFile)
	pszFile = m_pszFile;
if (pszSubFile) {
	if (!FindFileData (pszFile, pszSubFile, &size, &offset))
		return false;
	strcpy_s (m_pszSubFile, 256, pszSubFile);
	}
else if (0 > (index = GetFileData (-1, &size, &offset)))
	goto errorExit;
FreeTextureHandles ();


FSplit (pszFile, szTmp, NULL, NULL);
strcat_s (szTmp, sizeof (szTmp), "dle_temp.rdl");
fopen_s (&fTmp, szTmp, "wb");
fopen_s (&fSrc, pszFile, "rb");
// copy level to a temporary fTmp
if (!(fTmp && fSrc)) {
	ErrorMsg ("Unable to create temporary DLE-XP work file.");
	goto errorExit;
	}
// set subfile name
fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
size_t fPos = ftell (fSrc);
if (theMine->LoadMineSigAndType (fSrc))
	goto errorExit;
fseek (fSrc, long (fPos), SEEK_SET);
while (size > 0) {
	chunk = (size > sizeof (dataBuf)) ? sizeof (dataBuf) : size;
	fread (dataBuf, 1, chunk, fSrc);
	if (!chunk)
		break;
	fwrite(dataBuf, 1, chunk, fTmp);
	size -= chunk;
	}

// read custom palette if one exists
strcpy_s (message, sizeof (message), m_pszSubFile);
pszExt = strrchr (message, '.');
if (pszExt) {
	sprintf_s (pszExt, 5, ".pal");
	if (pszSubFile)
		FindFileData (pszFile, message, &size, &offset);
	else {
		index = FindFilename (message);
		if (index < 0)
			size = offset = -1;
		else
			GetFileData (index, &size, &offset);
		}
	if ((size > 0) || (offset >= 0)) {
		fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
		INT32 h = ftell (fSrc);
		ReadCustomPalette (fSrc, size);
		h = ftell (fSrc) - h;
		}
	}
// read custom light values if a lightmap file exists
strcpy_s (message, sizeof (message), m_pszSubFile);
pszExt = strrchr (message, '.');
if (pszExt) {
	sprintf_s (pszExt, 5, ".lgt");
	if (pszSubFile)
		FindFileData (pszFile, message, &size, &offset);
	else {
		index = FindFilename (message);
		if (index < 0)
			size = offset = -1;
		else
			GetFileData (index, &size, &offset);
		}
	if ((size < 0) || (offset < 0))
		CreateLightMap ();
	else {
		fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
		INT32 h = ftell (fSrc);
		ReadLightMap (fSrc, size);
		h = ftell (fSrc) - h;
		}
	}

//memset (theMine->TexColors (), 0, sizeof (theMine->MineData ().texColors));
sprintf_s (pszExt, 5, ".clr");
if (pszSubFile)
	FindFileData (pszFile, message, &size, &offset);
else {
	index = FindFilename (message);
	if (index < 0)
		size = offset = -1;
	else
		GetFileData (index, &size, &offset);
	}
if ((size >= 0) && (offset >= 0)) {
	fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
	INT32 h = ftell (fSrc);
	theMine->ReadColorMap (fSrc);
	h = ftell (fSrc) - h;
	}
// read custom textures if a pog file exists
strcpy_s (message, sizeof (message), m_pszSubFile);
pszExt = strrchr (message, '.');
if (pszExt) {
	sprintf_s (pszExt, 5, ".pog");
	if (pszSubFile)
		FindFileData (pszFile, message, &size, &offset);
	else {
		index = FindFilename (message);
		if (index < 0)
			size = offset = -1;
		else
			GetFileData (index, &size, &offset);
		}
	if ((size > 0) && (offset >= 0)) {
		fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
		INT32 h = ftell (fSrc);
		ReadPog (fSrc, size);
		h = ftell (fSrc) - h;
		}
	}
// read custom robot info if hxm fTmp exists
strcpy_s (message, sizeof (message), m_pszSubFile);
pszExt = strrchr (message, '.');
if (pszExt) {
	sprintf_s (pszExt, 5, ".hxm");
	if (pszSubFile)
		FindFileData (pszFile, message, &size, &offset);
	else {
		index = FindFilename (message);
		if (index < 0)
			size = offset = -1;
		else
			GetFileData (index, &size, &offset);
		}
	if ((size >= 0) && (offset >= 0)) {
		fseek (fSrc, sizeof (struct level_header) + offset, SEEK_SET);
		theMine->ReadHxmFile (fSrc, size);
		INT32 i, count;
		for (i = 0, count = 0; i < (INT32) N_robot_types;i++)
			if (theMine->RobotInfo (i)->m_info.bCustom)
				count++;
		sprintf_s (message, sizeof (message)," Hog manager: %d custom robots read", count);
		DEBUGMSG (message);
		}
	}
funcRes = true;

errorExit:

if (fTmp) 
	fclose (fTmp);
if (fSrc) 
	fclose (fSrc);
return funcRes;
}

//------------------------------------------------------------------------
// CHogManager::Ok()
//
// Saves file to a temporary file called dle_temp.rdl so editor can load it
//------------------------------------------------------------------------

void CHogManager::OnOK () 
{
LBFiles ()->GetText (LBFiles ()->GetCurSel (), m_pszSubFile);
char *pszExt = strrchr ((char *) m_pszSubFile, '.');
if (pszExt && _strcmpi (pszExt,".rdl") && _strcmpi (pszExt,".rl2")) {
#if 1
	ErrorMsg ("DLE-XP cannot process this file. To change the file,\n\n"
				 "export it and process it with the appropriate application.\n"
				 "To incorporate the changes in the HOG file,\n"
				 "import the modified file back into the HOG file.");
	return;
	}
#else //0
	if (QueryMsg ("Would you like to view this file\n"
					  "from an associated windows program?\n\n"
					  "Note: Any changes to this file will affect\n"
					  "the exported file and will have to be imported\n"
					  "back into the HOG file when finished.") == IDOK) {
		SizeListBox->GetString (message, index);
		long size = atol(message);
		OffsetListBox->GetString(message,index);
		long offset = atol(message) + sizeof (level_header);

		// Set all structure members to zero.
		OPENFILENAME ofn;
		memset(&ofn, 0, sizeof (OPENFILENAME));
		ofn.lStructSize = sizeof (OPENFILENAME);
		ofn.hwndOwner = HWindow;
		ofn.lpstrFilter = "All Files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = pszExt;
		strcpy(szTmp, m_pszSubFile);
		ofn.lpstrFile= szTmp;
		ofn.nMaxFile = sizeof (szTmp);
		ofn.Flags =   OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
		if (GetSaveFileName(&ofn)) {
		Export (m_pszName, szTmp, offset, size);
		HINSTANCE hinst = FindExecutable(szTmp, m_startFolder , message);
		if (hinst > (HINSTANCE)32)
			ShellExecute(HWindow, 0, message, szTmp, m_startFolder , SW_SHOWNORMAL);
		else {
			ErrorMsg ("No application is \"associated\" with this extention.\n\n"
						 "Hint: You can use File Manager to associate this fTmp's\n"
						 "extension with an application. File Manager can be found\n"
						 "in your windows directory under the szName WINFILE.EXE");
				}
			}
		}
	return;
	}
#endif //1

LoadLevel ();
CDialog::OnOK ();
}

//------------------------------------------------------------------------
// CHogManager::RenameMsg()
//------------------------------------------------------------------------

void CHogManager::OnRename ()
{
	char buf[20];
	INT32 index = LBFiles ()->GetCurSel ();

if (index < 0)
	return;
LBFiles ()->GetText (index, buf);
CInputDialog dlg (this, "Rename file", "Enter new name:",(char *) buf, sizeof (buf));
if (dlg.DoModal () != IDOK)
	return;
if (FindFilename (buf) >= 0) {
	ErrorMsg ("A file with that name already exists\nin the HOG file.");
	return;
	}
FILE *hogfile;
fopen_s (&hogfile, m_pszFile, "r+b"); // add it to the end
if (!hogfile) {
	ErrorMsg ("Could not open HOG file.");
	return;
	}
long size, offset;
level_header lh;
INT32 fileno = GetFileData (index, &size, &offset);
fseek (hogfile, offset, SEEK_SET);
if (!fread (&lh, sizeof (lh), 1, hogfile))
	ErrorMsg ("Cannot read from HOG file");
else {
	memset(lh.name, 0, sizeof (lh.name));
	buf[12] = NULL;
	_strlwr_s (buf, sizeof (buf));
	strncpy_s (lh.name, sizeof (lh.name), buf, 12);
	fseek (hogfile,offset,SEEK_SET);
	if (!fwrite (&lh, sizeof (lh), 1, hogfile))
		ErrorMsg ("Cannot write to HOG file");
	else {
		// update list box
		DeleteFile ();
		AddFile (lh.name, size, offset, fileno);
/*
		index = LBFiles ()->GetCurSel ();
		LBFiles ()->DeleteString (index);
		INT32 i = LBFiles ()->AddString (lh.name);
		LBFiles ()->SetItemData (i, index);
		LBFiles ()->SetCurSel (index);
*/
		}
	}
fclose(hogfile);
}

//------------------------------------------------------------------------
// CHogManager - ImportMsg
//
// Adds file to the end of the list
//------------------------------------------------------------------------

void CHogManager::OnImport () 
{
	long offset;

	char szFile[256] = "\0"; // buffer for file name
	level_header lh;
//  DWORD index;

//  // make sure there is an item selected
//  index = LBFiles ()->GetCurSel ();
//  if (index < 0) {
//    ErrorMsg ("You must select an item first.");
//  } else {
	// Set all structure members to zero.
#if 0
if (!BrowseForFile (TRUE, (IsD1File ()) ? "rdl" : "rl2", szFile, 
						  "Descent Level|*.rdl|"
						  "Descent 2 Level|*.rl2|"
						  "Texture file|*.pog|"
						  "Robot file|*.hxm|"
						  "Lightmap file|*.lgt|"
						  "Color file|*.clr|"
						  "Palette file|*.pal|"
						  "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
						  theApp.MainFrame ())
	return;
#else
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
if (theApp.IsD1File ()) {
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
#endif

FILE *fDest;
fopen_s (&fDest, m_pszFile, "ab"); // add it to the end
if (!fDest) {
	ErrorMsg ("Could not open destination HOG file for import.");
	return;
	}
FILE *fSrc;
fopen_s (&fSrc, szFile, "rb");
if (!fSrc) {
	ErrorMsg ("Could not open source file for import.");
	fclose (fDest);
	return;
	}
fseek (fDest, 0, SEEK_END);
offset = ftell (fDest);
// write header
lh.size = _filelength (_fileno (fSrc));
_strlwr_s (lh.name, sizeof (lh.name));
fwrite (&lh, sizeof (level_header), 1, fDest);

	// write data (from source to HOG)
while (!feof (fSrc)) {
	size_t n_bytes = fread (dataBuf, 1, sizeof (dataBuf), fSrc);
	if (n_bytes <= 0)
		break;
	fwrite (dataBuf, 1, n_bytes, fDest);
	}
fclose(fSrc);
fclose(fDest);
// update list boxes
AddFile (lh.name, lh.size, offset, LBFiles ()->GetCount ());
/*
INT32 index = LBFiles ()->AddString (lh.name);
AddFileData (index, lh.size, offset);
LBFiles ()->SetCurSel (index);
*/
}

//------------------------------------------------------------------------
// CHogManager - ExportMsg
//
// Exports selected item to a file
//------------------------------------------------------------------------

void CHogManager::OnExport () 
{
	char szFile[256] = "\0"; // buffer for file name
	DWORD index;
	level_header lh;
	long size, offset;

// make sure there is an item selected
index = LBFiles ()->GetCurSel ();
if (index < 0) {
	ErrorMsg ("Please select a file to export.");
	return;
	}
// get item name, size, and offset
LBFiles ()->GetText (index, lh.name);
GetFileData (index, &size, &offset);
lh.size = size;
offset += sizeof (level_header);
strcpy_s (szFile, sizeof (szFile), lh.name);
#if 1
if (!BrowseForFile (FALSE, "", szFile, "All Files|*.*||",
						  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT,
						  theApp.MainFrame ()))
	return;
ExportSubFile (m_pszFile, szFile, offset, lh.size);
#else //0
// Set al structure members to zero.
	OPENFILENAME ofn;
memset(&ofn, 0, sizeof (OPENFILENAME));
ofn.lStructSize = sizeof (OPENFILENAME);
ofn.hwndOwner = GetSafeHwnd ();
ofn.lpstrFilter = "All Files\0*.*\0";
ofn.nFilterIndex = 1;
ofn.lpstrDefExt = "";
strcpy(szFile,lh.name);
ofn.lpstrFile= szFile;
ofn.nMaxFile = sizeof (szFile);
ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
if (GetSaveFileName (&ofn))
	ExportSubFile (m_pszFile, ofn.lpstrFile, offset, lh.size);
#endif
}

//------------------------------------------------------------------------
// CHogManager - DeleteMsg
//
// Deletes an item from a HOG file using the following steps:
//
// 1) Creates a new HOG file which does not contain the file selected
// 2) Deletes original HOG file
// 3) Renames new file to original file's name
//
//------------------------------------------------------------------------

void CHogManager::OnDelete () 
{
	INT32 delete_index;
	long size;
	long offset;
	INT32 fileno;
	FILE *hogFile = 0;
	CListBox * plb = LBFiles ();

// make sure there is an item selected
delete_index = plb->GetCurSel ();
if (delete_index < 0) {
	ErrorMsg ("Please choose a file to delete.");
	return;
	}
#if 1//ndef _DEBUG
if (!bExpertMode) {
	strcpy_s (message, sizeof (message), "Are you sure you want to delete '");
	plb->GetText (delete_index, message + strlen (message));
	strcat_s (message, sizeof (message), "'?\n(This operation will change the HOG hogFile immediately)");
	if (QueryMsg (message) != IDYES)
		return;
	}
#endif
fileno = GetFileData (delete_index, &size, &offset);
INT32 nFiles = plb->GetCount ();
// open hog hogFile for modification
fopen_s (&hogFile, m_pszFile, "r+b");
if (!hogFile) {
	ErrorMsg ("Could not open hog hogFile.");
	return;
	}
DeleteSubFile (hogFile, size + sizeof (struct level_header), offset, nFiles, fileno);
fclose (hogFile);
ReadHogData ();
LBFiles ()->SetCurSel ((delete_index < nFiles - 1) ? delete_index : nFiles - 1);
}

//------------------------------------------------------------------------
// CHogManager - read hog data
//------------------------------------------------------------------------

bool ReadHogData (LPSTR pszFile, CListBox *plb, bool bAllFiles, bool bOnlyLevels, bool bGetFileData) 
{
	struct level_header *level;
	char data [256];
	long position;
	UINT8 index;
	FILE *hog_file;
	INT32 nFiles;

ClearFileList (plb);
fopen_s (&hog_file, pszFile, "rb");
if (!hog_file) {
	sprintf_s (message, sizeof (message), "Unable to open HOG file (%s)",pszFile);
	ErrorMsg (message);
	return false;
	}
fread (data,3,1,hog_file); // verify signature "DHF"
if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') {
	ErrorMsg ("This is not a Descent HOG file");
	return false;
	}
position = 3;
nFiles = 0;
while (!feof (hog_file)) {
	fseek (hog_file, position, SEEK_SET);
	if (fread (data, sizeof (struct level_header), 1, hog_file) != 1) 
		break;
	level = (struct level_header *) data;
	if (level->size < 0) {
		ErrorMsg ("Error reading HOG file");
		fclose (hog_file);
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
		INT32 i = plb->AddString (level->name);
		if (bGetFileData && (0 > AddFileData (plb, i, level->size, position, nFiles))) {
			ErrorMsg ("Too many files in HOG file.");
			fclose (hog_file);
			return false;
			}
		plb->SetCurSel (i);
		nFiles++;
		}
	position += sizeof (struct level_header) + level->size;
	}
fclose (hog_file);
// select first level file
for (index = 0; index < nFiles; index++) {
	message [0] = NULL;
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

//------------------------------------------------------------------------
// CHogManager - read hog data
//------------------------------------------------------------------------

bool FindFileData (LPSTR pszFile, LPSTR pszSubFile, long *nSize, long *nPos, BOOL bVerbose) 
{
	struct level_header *level;
	char data [256];
	long position;
	FILE *hog_file;
	INT32 nFiles;

*nSize = -1;
*nPos = -1;
fopen_s (&hog_file, pszFile, "rb");
if (!hog_file) {
	if (bVerbose) {
		sprintf_s (message, sizeof (message), "Unable to open HOG file (%s)",pszFile);
		ErrorMsg (message);
		}
	return false;
	}
fread(data,3,1,hog_file); // verify signature "DHF"
if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') {
	if (bVerbose)
		ErrorMsg ("This is not a Descent HOG file");
	return false;
	}
position = 3;
nFiles = 0;
while (!feof (hog_file)) {
	fseek (hog_file, position, SEEK_SET);
	if (fread (data, sizeof (struct level_header), 1, hog_file) != 1) 
		break;
	level = (struct level_header *) data;
	if (level->size > 100000000L || level->size < 0) {
		if (bVerbose)
			ErrorMsg ("Error reading HOG file");
		fclose (hog_file);
		return false;
		}
	level->name [sizeof (level->name) - 1] = 0; // null terminate in case its bad
	_strlwr_s (level->name, sizeof (level->name));
	if (!strcmp (pszSubFile, "*"))
		strcpy_s (pszSubFile, 256, level->name);
	if (!_strcmpi (level->name, pszSubFile)) {
		*nSize = level->size;
		*nPos = position;
		fclose (hog_file);
		return true;
		}
	position += sizeof (struct level_header) + level->size;
	}
fclose (hog_file);
return false;
}

//------------------------------------------------------------------------
// extract_from_hog()
//------------------------------------------------------------------------

bool ExportSubFile (const char *pszSrc, const char *pszDest, long offset, long size) 
{
FILE *fSrc;
fopen_s (&fSrc, pszSrc, "rb");
if (!fSrc) {
	ErrorMsg ("Could not open HOG file.");
	return false;
	}
FILE *fDest;
fopen_s (&fDest, pszDest,"wb");
if (!fDest) {
	ErrorMsg ("Could not create export file.");
	fclose (fSrc);
	return false;
	}
// seek to item's offset in HOG file
fseek(fSrc,offset,SEEK_SET);
// create file (from HOG to file)
while (size > 0) {
	size_t n_bytes, n;
	n = (size > sizeof (dataBuf)) ? sizeof (dataBuf) : size_t (size);
	n_bytes = fread (dataBuf, 1, n, fSrc);
	fwrite (dataBuf, 1, n_bytes, fDest);
	size -= long (n_bytes);
	if (n_bytes != n)
		break;
	}
fclose(fDest);
fclose(fSrc);
return (size == 0);
}


//----------------------------------------------------------------------------
// delete_sub_file()
//----------------------------------------------------------------------------

void DeleteSubFile (FILE *file, long size, long offset, INT32 num_entries, INT32 delete_index) 
{
INT32 n_bytes;
// as long as we are not deleting the last item
if (delete_index < num_entries - 1) {
	// get size of chunk to remove from the file, then move everything
	// down by that amount.
	do {
		fseek (file, offset + size, SEEK_SET);
		n_bytes = INT32 (fread (dataBuf, 1, sizeof (dataBuf), file));
		if (n_bytes <= 0)
			break;
		fseek (file, offset, SEEK_SET);
		fwrite (dataBuf, 1, n_bytes, file);
		offset += n_bytes;
		} while (n_bytes > 0);
	}
// set the new size of the file
_chsize (_fileno (file), offset);
}

//--------------------------------------------------------------------------
// strip_extension
//--------------------------------------------------------------------------

void strip_extension(char *str) 
{
char *ext = strrchr(str,'.');
if (ext)
	*ext = '\0';
}

//--------------------------------------------------------------------------
// DeleteLevelSubFiles()
//
// deletes sub-files with same base name from hog
//--------------------------------------------------------------------------

#define MAX_REGNUM 6

void DeleteLevelSubFiles (FILE *file, char *base) 
{
struct region {
	INT32 index;
	INT32 offset;
	INT32 size;
	INT32 files;
	};
	INT32 regnum = 0;
	INT32 delete_index = 0;
	INT32 num_entries = 0;
	region reg [MAX_REGNUM] = {{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0},{-1,0,0,0}};

// figure out regions of the file to delete (3 regions max)
long offset = 3;
long size;
delete_index = -1;
while(!feof (file)) {
	level_header lh = {"",0};
	fseek (file, offset, SEEK_SET); // skip "HOG"
	if (!fread (&lh, sizeof (lh), 1, file)) 
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
	DeleteSubFile (file, reg [regnum].size, reg [regnum].offset, num_entries, reg [regnum].index);
	num_entries -= reg [regnum].files;
	}
fclose (file);
}

#undef MAX_REGNUM

//==========================================================================
// write_sub_file()
//==========================================================================

INT32 WriteSubFile (FILE *fDest, char *szSrc, char *szLevel) 
{
	FILE *fSrc;
	size_t n_bytes;
	level_header lh;

fopen_s (&fSrc, szSrc, "rb");
if (!fSrc) {
	sprintf_s (message, sizeof (message), "Unable to open temporary file:\n%s",szSrc);
	ErrorMsg (message);
	return -1;
	}
// write szLevel (13 chars, null filled)
memset (&lh, 0, sizeof (lh));
strncpy_s (lh.name, sizeof (lh.name), szLevel, 12);
_strlwr_s (lh.name, sizeof (lh.name));
// calculate then write size
fseek (fSrc, 0, SEEK_END);
lh.size = ftell (fSrc);
//fclose (fSrc);
//fSrc = fopen (szSrc,"rb");
fseek (fSrc,0,SEEK_SET);
fwrite (&lh, sizeof (lh), 1, fDest);
// write data
while(!feof (fSrc)) {
	n_bytes = fread (dataBuf,1,sizeof (dataBuf),fSrc);
	if (n_bytes > 0)
		fwrite(dataBuf,1,n_bytes,fDest);
	}
fclose(fSrc);
return lh.size + sizeof (lh);
}

//==========================================================================
// make_hog()
//
// Action - makes a HOG file which includes three files
//             1. the rdl file to include (only one)
//             2. a briefing file (called brief.txb)
//             3. an ending sequence (same name as hog w/ .txb extension)
//          also makes a mission file for this HOG file
//
// Changes - now saves rl2 files
//==========================================================================

INT32 MakeHog (char *rdlFilename, char *hogFilename, char*szSubFile, bool bSaveAs) 
{
	FILE *hogfile, *fTmp;
	char *name_start,*pszNameEnd, *pszExtStart;
	char szBaseName[13];
	char filename[256];
	char szTmp[256], szBase [13];
	INT32 custom_robots = 0;
	INT32 custom_textures = 0;

// create HOG file which contains szTmp.rdl, szTmp.txb, and dlebrief.txb");
strcpy_s (filename, sizeof (filename), hogFilename);
fopen_s (&hogfile, filename, "wb");
if (!hogfile) {
	sprintf_s (message, sizeof (message), "Unable to create HOG file:\n%s",filename);
	ErrorMsg (message);
	return 1;
	}
// write file type
fwrite("DHF",1,3,hogfile); // starts with Descent Hog File
// get base szTmp w/o extension and w/o path
name_start = strrchr(hogFilename,'\\');
if (name_start==NULL)
	name_start = hogFilename;
else
	name_start++; // move to just pass the backslash
strncpy_s (szBaseName, sizeof (szBaseName), name_start,12);
szBaseName[12] = NULL; // make sure it is null terminated
pszNameEnd = strrchr((char *)szBaseName,'.');
if (!pszNameEnd)
	pszNameEnd = szBaseName + strlen ((char *)szBaseName);
memset (pszNameEnd, 0, 12 - INT32 (pszNameEnd - szBaseName));
// write rdl file
if (*szSubFile) {
	for (pszExtStart = szSubFile; *pszExtStart && (*pszExtStart != '.'); pszExtStart++)
		;
	strncpy_s (szBaseName, sizeof (szBaseName), szSubFile, pszExtStart - szSubFile);
	szBaseName [pszExtStart - szSubFile] = '\0';
	}
if (theApp.IsD1File ())
	sprintf_s (szTmp, sizeof (szTmp), "%s.RDL", szBaseName);
else
	sprintf_s (szTmp, sizeof (szTmp), "%s.RL2", szBaseName);
WriteSubFile (hogfile, rdlFilename, szTmp);
_unlink (szTmp);
#if 0
// write palette file into hog (if D2)
if (IsD2File ()) {
	if (strcmp(palette_resource(),"GROUPA_256") == 0) {
		char *start_name = strrchr(descent2_path,'\\');
		if (!start_name) {
			start_name = descent2_path; // point to 1st char if no slash found
			}
		else {
			start_name++;               // point to character after slash
			}
		strncpy(szTmp,start_name,12);
		szTmp[13] = NULL;  // null terminate just in case
		// replace extension with *.256
		if (strlen (szTmp) > 4) {
			strcpy(szTmp + strlen (szTmp) - 4,".256");
			}
		else {
			strcpy(szTmp,"GROUPA.256");
			}
		strupr(szTmp);
		sprintf_s (message, sizeof (message), "%s\\groupa.256",m_startFolder );
		write_sub_file(hogfile, message,szTmp);
		}
	}
#endif

if (theMine->HasCustomLightMap ()) {
	FSplit (hogFilename, szTmp, NULL, NULL);
	strcat_s (szTmp, sizeof (szTmp), "dle_temp.lgt");
	fopen_s (&fTmp, szTmp, "wb");
	if (fTmp) {
		if (!WriteLightMap (fTmp)) {
			fclose (fTmp);
			sprintf_s (szBase, sizeof (szBase), "%s.lgt", szBaseName);
			WriteSubFile (hogfile, szTmp, szBase);
			}
		else
			fclose (fTmp);
		_unlink (szTmp);
		}
	}
if (theMine->HasCustomLightColors ()) {
	FSplit (hogFilename, szTmp, NULL, NULL);
	strcat_s (szTmp, sizeof (szTmp), "dle_temp.clr");
	fopen_s (&fTmp, szTmp, "wb");
	if (fTmp) {
		if (!theMine->WriteColorMap (fTmp)) {
			fclose (fTmp);
			sprintf_s (szBase, sizeof (szBase), "%s.clr", szBaseName);
			WriteSubFile (hogfile, szTmp, szBase);
			}
		else
			fclose (fTmp);
		_unlink (szTmp);
		}
	}
	
if (HasCustomPalette ()) {
	FSplit (hogFilename, szTmp, NULL, NULL);
	strcat_s (szTmp, sizeof (szTmp), "dle_temp.pal");
	fopen_s (&fTmp, szTmp,"wb");
	if (fTmp) {
		if (!WriteCustomPalette (fTmp)) {
			fclose (fTmp);
			sprintf_s (szBase, sizeof (szBase), "%s.pal", szBaseName);
			WriteSubFile (hogfile, szTmp, szBase);
			}
		else
			fclose (fTmp);
		_unlink (szTmp);
		}
	}

// if textures have changed, ask if user wants to create a pog file
if (HasCustomTextures ()) {
	if (bExpertMode ||
		 QueryMsg("This level contains custom textures.\n"
					 "Would you like save these textures into the HOG file?\n\n"
					 "Note: You must use version 1.2 or higher of Descent2 to see\n"
					 "the textures when you play the game.") == IDYES) {
		FSplit (hogFilename, szTmp, NULL, NULL);
		strcat_s (szTmp, sizeof (szTmp), "dle_temp.pog");
		fopen_s (&fTmp, szTmp,"wb");
		if (fTmp) {
			if (!CreatePog (fTmp)) {
				fclose (fTmp);
				sprintf_s (szBase, sizeof (szBase), "%s.pog", szBaseName);
				WriteSubFile (hogfile, szTmp, szBase);
				custom_textures = 1;
				}
			else
				fclose (fTmp);
			_unlink (szTmp);
			}
		}
	}
// if robot info has changed, ask if user wants to create a hxm file
if (theMine->HasCustomRobots ()) {
	if (bExpertMode ||
		 QueryMsg ("This level contains custom robot settings.\n"
					  "Would you like save these changes into the HOG file?\n\n"
					  "Note: You must use version 1.2 or higher of Descent2 for\n"
					  "the changes to take effect.") == IDYES) {
		FSplit (hogFilename, szTmp, NULL, NULL);
		strcat_s (szTmp, sizeof (szTmp), "dle_temp.hxm");
		fopen_s (&fTmp, szTmp, "wb");
		if (fTmp) {
			if (!theMine->WriteHxmFile (fTmp)) {
				sprintf_s (szBase, sizeof (szBase), "%s.hxm", szBaseName);
				WriteSubFile (hogfile, szTmp, szBase);
				custom_robots = 1;
				}
			_unlink (szTmp);
			}
		}
	}
fclose (hogfile);
MakeMissionFile (hogFilename, szSubFile, custom_textures, custom_robots, bSaveAs);
return 0;
}

//==========================================================================
// MENU - Save
//==========================================================================

INT32 SaveToHog (LPSTR szHogFile, LPSTR szSubFile, bool bSaveAs) 
{
	FILE	*fTmp;
	char szTmp [256], subName [256];
	char *psz;

_strlwr_s (szHogFile, 256);
psz = strstr (szHogFile, "new.");
if (!*szSubFile || psz) { 
	CInputDialog dlg (theApp.MainFrame (), "Name mine", "Enter file name:", szSubFile, 9);
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
		strcpy_s (psz, 256 - (psz - szHogFile), szSubFile);
		strcat_s (szHogFile, sizeof (szHogFile), ".hog");
		}
	strcat_s (szSubFile, 256, (theApp.IsD1File ()) ? ".rdl" : ".rl2");
	}
// if this HOG file only contains one rdl/rl2 file total and
// it has the same name as the current level, and it has
// no other files (besides hxm or pog files), then
// allow quick save
FILE *file;
// See if another level with the same name exists
// and see if there are any other files here (ignore hxm and pog files)
INT32 bOtherFilesFound = 0;
INT32 bIdenticalLevelFound = 0;
fopen_s (&file, szHogFile, "rb");
if (!file) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat_s (szTmp, sizeof (szTmp), "dle_temp.rdl");
	theMine->Save (szTmp);
	return MakeHog (szTmp, szHogFile, szSubFile, true);
	}
fseek (file,3,SEEK_SET); // skip "HOG"
while (!feof (file)) {
	level_header lh = {"",0};
	if (!fread (&lh, sizeof (lh), 1, file)) 
		break;
	_strlwr_s (lh.name, sizeof (lh.name));
	lh.name [sizeof (lh.name) - 1] = NULL; // null terminate
	if (!strstr ((char *) lh.name, ".hxm") &&     // not a custom robot file
		 !strstr ((char *) lh.name, ".pog") &&     // not a custom texture file
		 !strstr ((char *) lh.name, ".lgt") &&     // not an texture brightness table file
		 !strstr ((char *) lh.name, ".clr") &&     // not an texture color table file
		 !strstr ((char *) lh.name, ".pal") &&     // not a palette file
		 _strcmpi((char *) lh.name, szSubFile)) // not the same level
		bOtherFilesFound = 1;
	if (!_strcmpi ((char *) lh.name, szSubFile)) // same level
		bIdenticalLevelFound = 1;
	fseek(file, lh.size, SEEK_CUR);
	}
fclose(file);

// if no other files found
// then simply do a quick save
INT32 bQuickSave = 0;
if (!bOtherFilesFound)
	bQuickSave = 1;
else if (bIdenticalLevelFound) {
	if (QueryMsg ("Overwrite old level with same name?") != IDYES)
		return 1;
	}
else {
	// otherwise, if save file was not found,
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
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat_s (szTmp, sizeof (szTmp), "dle_temp.rdl");
	theMine->Save (szTmp);
	return MakeHog (szTmp, szHogFile, szSubFile, bSaveAs);
//	MySetCaption (szHogFile);
	}
char base [256];
// determine base name
FSplit (szSubFile, NULL, base, NULL);
base[8] = NULL;
_strlwr_s (base, sizeof (base));
strip_extension (base);
fopen_s (&file, szHogFile, "r+b"); // reopen file
if (!file) {
	ErrorMsg ("Destination HOG file not found/accessible.");
	return 1;
	}
DeleteLevelSubFiles (file, base);
fclose (file);
// now append sub-files to the end of the HOG file
fopen_s (&file, szHogFile, "ab");
if (!file) {
	ErrorMsg ("Could not open destination HOG file for save.");
	return 1;
	}
fseek (file, 0, SEEK_END);
FSplit (szHogFile, szTmp, NULL, NULL);
strcat_s (szTmp, sizeof (szTmp), "dle_temp.rdl");
theMine->Save (szTmp, true);
WriteSubFile (file, szTmp, szSubFile);

if (theMine->HasCustomLightMap ()) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat_s (szTmp, sizeof (szTmp), "dle_temp.lgt");
	fopen_s (&fTmp, szTmp, "wb");
	bool bOk = false;
	if (fTmp) {
		if (!WriteLightMap (fTmp)) {
			fclose (fTmp);
			sprintf_s (subName, sizeof (subName), "%s.lgt", base);
			WriteSubFile (file, szTmp, subName);
			bOk = true;
			}
		else
			fclose (fTmp);
		_unlink (szTmp);
		}
	if (!bOk)
		ErrorMsg ("Error writing custom light map.");
	}
if (theMine->HasCustomLightColors ()) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat_s (szTmp, sizeof (szTmp), "dle_temp.clr");
	fopen_s (&fTmp, szTmp, "wb");
	if (fTmp) {
		if (!theMine->WriteColorMap (fTmp)) {
			fclose (fTmp);
			sprintf_s (subName, sizeof (subName), "%s.clr", base);
			WriteSubFile (file, szTmp, subName);
			}
		else
			fclose (fTmp);
		_unlink (szTmp);
		}
	}

if (HasCustomTextures ()) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat_s (szTmp, sizeof (szTmp), "dle_temp.hxm");
	fopen_s (&fTmp, szTmp, "wb");
	bool bOk = false;
	if (fTmp) {
		if (!CreatePog (fTmp)) {
			sprintf_s (subName, sizeof (subName), "%s.pog", base);
			WriteSubFile (file, szTmp, subName);
			bOk = true;
			}
		fclose (fTmp);
		_unlink (szTmp);
		}
	if (!bOk)
		ErrorMsg ("Error writing custom textures.");
	}

if (theMine->HasCustomRobots ()) {
	FSplit (szHogFile, szTmp, NULL, NULL);
	strcat_s (szTmp, sizeof (szTmp), "dle_temp.hxm");
	fopen_s (&fTmp, szTmp, "wb");
	bool bOk = false;
	if (fTmp) {
		if (!theMine->WriteHxmFile (fTmp)) {
			sprintf_s (subName, sizeof (subName), "%s.hxm", base);
			WriteSubFile (file, szTmp, subName);
			bOk = true;
			}
		_unlink (szTmp);
		}
	if (!bOk)
		ErrorMsg ("Error writing custom robots.");
	}
fclose(file);
return 0;
}

//==========================================================================
// write_mission_file()
//==========================================================================

static LPSTR szMissionName [] = {"name", "zname", "d2x-name", NULL};
static LPSTR szMissionInfo [] = {"editor", "build_time", "date", "revision", "author", "email", "web_site", "briefing", NULL};
static LPSTR szMissionType [] = {"type", NULL};
static LPSTR szMissionTypes [] = {"anarchy", "normal", NULL};
static LPSTR szMissionFlags [] = {"normal", "anarchy", "robo_anarchy", "coop", "capture_flag", "hoard", NULL};
static LPSTR szCustomFlags [] = {"custom_textures", "custom_robots", "custom_music", NULL};
static LPSTR szAuthorFlags [] = {"multi_author", "want_feedback", NULL};
static LPSTR szNumLevels [] = {"num_levels", NULL};
static LPSTR szNumSecrets [] = {"num_secrets", NULL};
static LPSTR szBool [] = {"no", "yes", NULL};

static LPSTR *szTags [] = {szMissionName, szMissionInfo, szMissionType, szMissionFlags, szCustomFlags, szAuthorFlags, szNumLevels, szNumSecrets};

                         /*--------------------------*/

INT32 atob (LPSTR psz, size_t nSize)
{
_strlwr_s (psz, nSize);
INT32 i;
for (i = 0; i < 2; i++)
	if (!strcmp (psz, szBool [i]))
		return i;
return 0;
}

                         /*--------------------------*/

INT32 ReadMissionFile (char *pszFile) 
{
	FILE	*fMsn;
	char  szMsn [256];
	LPSTR	psz, *ppsz;
	char	szTag [20], szValue [80], szBuf [100];
	INT32	i, j, l;

strcpy_s (szMsn, sizeof (szMsn), pszFile);
char *pExt = strrchr (szMsn, '.');
if (pExt)
	*pExt = '\0';
strcat_s (szMsn, sizeof (szMsn), (theApp.IsD1File ()) ? ".msn" : ".mn2");
fopen_s (&fMsn, szMsn, "rt");
if (!fMsn) {
	DEBUGMSG (" Hog manager: Mission file not found.");
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
		l += INT32 (strlen (szBuf + 1));
		continue;
		}
	else if (!(psz = strchr (szBuf, '=')))	// otherwise need <tag> '=' <value> format
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
				j = INT32 (ppsz - szTags [i]);
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
				for (j = INT32 (strlen (missionData.levelList [i])); --j; )
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

                         /*--------------------------*/

INT32 WriteMissionFile (char *pszFile, INT32 levelVersion, bool bSaveAs) 
{
	FILE	*fMsn;
	char  szMsn [256];
	INT32	i, j;

strcpy_s (szMsn, sizeof (szMsn), pszFile);
char *pExt = strrchr (szMsn, '.');
if (pExt)
	*pExt = '\0';
strcat_s (szMsn, sizeof (szMsn), (theApp.IsD1File ()) ? ".msn" : ".mn2");
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

//==========================================================================
// make_mission_file()
//==========================================================================

INT32 MakeMissionFile (char *pszFile, char *pszSubFile, INT32 bCustomTextures, INT32 bCustomRobots, bool bSaveAs) 
{
	char	szBaseName [256];
	char	szTime [20];

//memset (&missionData, 0, sizeof (missionData));
FSplit (pszSubFile, NULL, szBaseName, NULL);
if (!*missionData.missionName)
	strcpy_s (missionData.missionName, sizeof (missionData.missionName), szBaseName);
if (bSaveAs || !*missionData.missionName)
	do {
		CInputDialog dlg (theApp.MainFrame (), "Mission title", "Enter mission title:", missionData.missionName, sizeof (missionData.missionName));
		if (dlg.DoModal () != IDOK)
			return -1;
	} while (!*missionData.missionName);
missionData.missionType = 1;
missionData.numLevels = 1;
strcpy_s (missionData.levelList [0], sizeof (missionData.levelList [0]), pszSubFile);
if (!strchr (pszSubFile, '.'))
	strcat_s (missionData.levelList [0], sizeof (missionData.levelList [0]), theApp.IsD2File () ? ".rl2" : ".rdl");
missionData.numSecrets = 0;
strcpy_s (missionData.missionInfo [0], sizeof (missionData.levelList [0]), "DLE-XP");
strcpy_s (missionData.missionInfo [2], sizeof (missionData.levelList [2]), DateStr (szTime, sizeof (szTime), true));
if (bSaveAs)
	strcpy_s (missionData.missionInfo [3], sizeof (missionData.missionInfo [3]), "1.0");
missionData.customFlags [0] = bCustomTextures;
missionData.customFlags [1] = bCustomRobots;
return WriteMissionFile (pszFile, theApp.LevelVersion (), bSaveAs);
}

// eof file.cpp