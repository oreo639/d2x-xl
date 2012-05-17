#ifndef __hogman_h
#define __hogman_h

#include "dle-xp-res.h"
#include "MemoryFile.h"

#define MAX_HOGFILES	1000

//------------------------------------------------------------------------

class CInputDialog : public CDialog {
	public:
		LPSTR		m_pszTitle;
		LPSTR		m_pszPrompt;
		LPSTR		m_pszBuf;
		size_t	m_nBufSize;

		CInputDialog (CWnd *pParentWnd = null, LPSTR pszTitle = null, LPSTR pszPrompt = null, LPSTR pszBuf = null, size_t nBufSize = 0);
		virtual BOOL OnInitDialog (void);
		virtual void DoDataExchange (CDataExchange * pDX);
		void OnOK (void);
};

//------------------------------------------------------------------------

typedef struct tHogFileData {
	long	m_offs;
	long	m_size;
	int	m_fileno;
} tHogFileData;

class CHogManager : public CDialog {
	public:
		bool				m_bInited;
		char				m_name [256];
		LPSTR				m_pszFile;
		LPSTR				m_pszSubFile;
		int				m_bExtended;
		//int				m_type;
		//int				*m_pType;
		int				m_bShowAll;
		CMemoryFile		m_level;
		//CLevelHeaderFactory	m_headerFactory;
//		tHogFileData	m_fileData;

		CHogManager (CWnd *pParentWnd = null, LPSTR pszFile = null, LPSTR pszSubFile = null);
		void Setup (LPSTR pszFile, LPSTR pszSubFile);
		virtual BOOL OnInitDialog (void);
		virtual void DoDataExchange (CDataExchange * pDX);
		void EndDialog (int nResult);
		int FindFilename (LPSTR pszName);
		void ClearFileList (void);
		int AddFile (LPSTR pszName, long length, long size, long offset, int fileno);
		int DeleteFile (int index = -1);
		int GetFileData (int index = -1, long *size = null, long *offset = null);
		int AddFileData (int index, long size, long offset, int fileno);
		bool LoadLevel (LPSTR pszFile = null, LPSTR pszSubFile = null);
		void OnOK (void);
		void OnCancel (void);
		bool ReadHogData (void);
		void Reset (void);

		long FindSubFile (CFileManager& fp, const char* pszFile, const char* pszSubFile, const char* pszExt);
		int ReadSignature (CFileManager* fp, bool bVerbose = true);
		int ReadData (LPSTR pszFile, CListBox *plb, bool bAllFiles, bool bOnlyLevels, bool bGetFileData = true);

		inline char* MissionName (void) { return m_pszFile; }
		inline char* LevelName (void) { return m_pszSubFile; }

		//inline CBaseLevelHeader* CreateHeader (int nType = -1) { return m_headerFactory.Create ((nType < 0) ? m_nType : nType); }

		afx_msg void OnRename ();
		afx_msg void OnDelete ();
		afx_msg void OnImport ();
		afx_msg void OnExport ();
		afx_msg void OnFilter ();
		afx_msg void OnSetFile ();
		inline CListBox *LBFiles () { return (CListBox *) GetDlgItem (IDC_HOG_FILES); }

	private:
		void Rename (CFileManager& fp, int index, char* szNewName);

	DECLARE_MESSAGE_MAP ()
	};

extern CHogManager* hogManager;

//------------------------------------------------------------------------

bool BrowseForFile (BOOL bOpen, LPSTR pszDefExt, LPSTR pszFile, LPSTR pszFilter, DWORD nFlags = 0, CWnd *pParentWnd = null);
int SaveToHog (LPSTR szHogFile, LPSTR szSubFile, bool bSaveAs);
bool FindFileData (const char* pszFile, char* pszSubFile, CLevelHeader& lh, long& nSize, long& nPos, BOOL bVerbose = TRUE, CFileManager* fp = null);
bool ExportSubFile (const char *pszSrc, const char *pszDest, long offset, long size);
int ReadMissionFile (char *pszFile);
int WriteMissionFile (char *pszFile, int levelVersion, bool bSaveAs = true);
int MakeMissionFile (char *pszFile, char *pszSubFile, int bCustomTextures, int bCustomRobots, bool bSaveAs = true);

//------------------------------------------------------------------------

#endif //__hogman_h
