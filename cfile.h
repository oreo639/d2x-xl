#ifndef _CFILE_H
#define _CFILE_H

#include <stdio.h>
#include <sys/types.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#ifndef _WIN32_WCE
#include <errno.h>
#endif

#include "types.h"

// the maximum length of a filename
#define FILENAME_LEN			1024

class CFilename {
	public:
		char m_buffer [FILENAME_LEN + 1];
	public:
		CFilename () { m_buffer [0] = '\0'; }
		inline CFilename& operator= (CFilename& other) { 
			memcpy (m_buffer, other.m_buffer, sizeof (m_buffer)); 
			return *this;
			}
		inline CFilename& operator= (const char* other) { 
			strncpy_s (m_buffer, sizeof (m_buffer), other, sizeof (m_buffer)); 
			return *this;
			}
		inline bool operator== (CFilename& other)
			{ return !strcmp (m_buffer, other.m_buffer); }
		inline bool operator< (CFilename& other)
			{ return strcmp (m_buffer, other.m_buffer) < 0; }
		inline bool operator> (CFilename& other)
			{ return strcmp (m_buffer, other.m_buffer) > 0; }
		inline operator const char*()
			{ return reinterpret_cast<char*> (&m_buffer [0]); }
	};


typedef struct CFILE {
	FILE		*file;
	char		*filename;
	INT32		size;
	INT32		libOffset;
	INT32		rawPosition;
} CFILE;

class CFile {
	private:
		CFILE	m_cf;

	public:
		CFile () { Init (); }
		~CFile () { Close (); };
		void Init (void);
		INT32 Open (const char *filename, const char *folder, const char *mode);
		INT32 Length (void);							// Returns actual size of file...
		size_t Read (void *buf, size_t elsize, size_t nelem);
		INT32 Close (void);
		INT32 Size (const char *hogname, const char *folder);
		INT32 Seek (size_t offset, INT32 where);
		INT32 Tell (void);
		char *GetS (char *buf, size_t n);
		INT32 EoF (void);
		INT32 Error (void);
		INT32 Write (const void *buf, INT32 elsize, INT32 nelem);
		INT32 GetC (void);
		INT32 PutC (INT32 c);
		INT32 PutS (const char *str);

		inline INT32 Size (void) { return m_cf.size; }

		// prototypes for reading basic types from fp
		INT32 ReadInt (void);
		UINT32 ReadUInt (void);
		INT16 ReadShort (void);
		UINT16 ReadUShort (void);
		INT8 ReadByte (void);
		UINT8 ReadUByte (void);
		FIX ReadFix (void);
		FIXANG ReadFixAng (void);
		void ReadVector (CFixVector& v);
		void ReadAngVec (CAngleVector& v);
		void ReadMatrix (CFixMatrix& v);
		float ReadFloat (void);
		double ReadDouble (void);
		void ReadString (char *buf, INT32 n);
		char *ReadData (const char *filename, const char *folder);

		INT32 WriteInt (INT32 i);
		INT32 WriteFix (FIX x);
		INT32 WriteShort (INT16 s);
		INT32 WriteByte (INT8 u);
		INT32 WriteFixAng (FIXANG a);
		INT32 WriteFloat (float f);
		INT32 WriteDouble (double d);
		void WriteAngVec (const CAngleVector& v);
		void WriteVector (const CFixVector& v);
		void WriteMatrix (const CFixMatrix& m);
		INT32 WriteString (const char *buf);

		INT32 Copy (const char *pszSrc, const char *pszDest);
		INT32 Extract (const char *filename, const char *folder, const char *szDest);
		time_t Date (const char *filename, const char *folder);

		static INT32 Exist (const char *filename, const char *folder);	// Returns true if file exists on disk (1) or in hog (2).
		static INT32 Delete (const char *filename, const char* folder);
		static INT32 Rename (const char *oldname, const char *newname, const char *folder);
		static INT32 MkDir (const char *pathname);
		static FILE *GetFileHandle (const char *filename, const char *folder, const char *mode);
		static void SplitPath (const char *szFullPath, char *szFolder, char *szFile, char *szExt);
		static void ChangeFilenameExtension (char *dest, const char *src, const char *new_ext);

		inline FILE*& File () { return m_cf.file; }
		inline char* Name () { return m_cf.filename; }
	};


typedef struct tGameFolders {
	char szHomeDir [FILENAME_LEN];
#if defined (__unix__) || defined (__macosx__)
	char szSharePath [FILENAME_LEN];
#endif
	char szAltHogDir [FILENAME_LEN];
	char szCacheDir [FILENAME_LEN];
	char szConfigDir [FILENAME_LEN];
	char szDataDir [FILENAME_LEN];
	char szDemoDir [FILENAME_LEN];
	char szDownloadDir [FILENAME_LEN];
	char szGameDir [FILENAME_LEN];
	char szMissionDir [FILENAME_LEN];
	char szMissionDirs [2][FILENAME_LEN];
	char szMissionDownloadDir [FILENAME_LEN];
	char szModDir [2][FILENAME_LEN];
	char szModelDir [3][FILENAME_LEN];
	char szModelCacheDir [3][FILENAME_LEN];
	char szModName [FILENAME_LEN];
	char szMovieDir [FILENAME_LEN];
	char szMsnSubDir [FILENAME_LEN];
	char szMusicDir [FILENAME_LEN];
	char szProfDir [FILENAME_LEN];
	char szSaveDir [FILENAME_LEN];
	char szScrShotDir [FILENAME_LEN];
	char szShaderDir [FILENAME_LEN];
	char szSoundDir [7][FILENAME_LEN];
	char szTextureCacheDir [4][FILENAME_LEN];
	char szTextureDir [4][FILENAME_LEN];
	char szWallpaperDir [FILENAME_LEN];
	INT32 bAltHogDirInited;
} tGameFolders;

INT32 GetAppFolder (const char *szRootDir, char *szFolder, const char *szName, const char *szFilter);
char *GameDataFilename (char *pszFilename, const char *pszExt, INT32 nLevel, INT32 nType);
void MakeTexSubFolders (char* pszParentFolder);
void MakeModFolders (const char* pszMission);
void ResetModFolders (void);

#ifdef _WIN32
char *UnicodeToAsc (char *str, const wchar_t *w_str);
wchar_t *AscToUnicode (wchar_t *w_str, const char *str);
#endif

extern INT32 nCFileError;
extern tGameFolders	gameFolders;

#ifdef _WIN32_WCE
# define errno -1
# define strerror (x) "Unknown Error"
#endif

#endif
