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

#include "VectorTypes.h"

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


typedef struct tFileInfo {
	FILE		*file;
	char		*filename;
	int		size;
	int		libOffset;
	int		rawPosition;
} tFileInfo;

class CFileManager {
	private:
		tFileInfo	m_cf;

	public:
		CFileManager () { Init (); }
		~CFileManager () { Close (); };
		void Init (void);
		int Open (const char *filename, const char *mode);
		int Length (void);							// Returns actual size of file...
		size_t Read (void *buf, size_t elsize, size_t nelem);
		int Close (void);
		int Size (const char *hogname, const char *folder);
		int Seek (size_t offset, int whence = SEEK_SET);
		int Tell (void);
		char *GetS (char *buf, size_t n);
		int EoF (void);
		int Error (void);
		int Write (const void *buf, int elsize, int nelem);
		int GetC (void);
		int PutC (int c);
		int PutS (const char *str);

		inline int Size (void) { return m_cf.size; }

		// prototypes for reading basic types from fp
		int ReadInt32 (void);
		uint ReadUInt32 (void);
		short ReadInt16 (void);
		ushort ReadUInt16 (void);
		byte ReadByte (void);
		sbyte ReadSByte (void);
		char ReadChar (void);
		fix ReadFix (void);
		fixang ReadFixAng (void);
		float ReadFloat (void);
		double ReadDouble (void);
		void* ReadBytes (void* buffer, int bufLen);
		void ReadString (char* buffer, int bufLen);

		int ReadVector (tFixVector& v);
		int ReadVector (tDoubleVector& v);
		int ReadVector (tAngleVector& v);
		int ReadMatrix (CFixMatrix& v);
		int ReadMatrix (CDoubleMatrix& m);

		inline int Read (CFixVector& v) { return ReadVector (v.v); }
		inline int Read (CDoubleVector& v) { return ReadVector (v.v); }
		inline int Read (CAngleVector& v) { return ReadVector (v.v); }
		inline int Read (CFixMatrix& m) { return ReadMatrix (m); }
		inline int Read (CDoubleMatrix& m) { return ReadMatrix (m); }


		int WriteInt32 (int v);
		int WriteUInt32 (uint v);
		int WriteFix (fix v);
		int WriteInt16 (short v);
		int WriteUInt16 (ushort v);
		int WriteByte (byte v);
		int WriteSByte (sbyte v);
		int WriteChar (char v);
		int WriteFixAng (fixang v);
		int WriteFloat (float v);
		int WriteDouble (double v);

		inline int Write (int& v) { return WriteInt32 (v); }
		inline int Write (uint& v) { return WriteUInt32 (v); }
//		inline int Write (fix& v) { return WriteFix (v); }
		inline int Write (short& v) { return WriteInt16 (v); }
		inline int Write (ushort& v) { return WriteUInt16 (v); }
		inline int Write (byte& v) { return WriteByte (v); }
		inline int Write (sbyte& v) { return WriteSByte (v); }
		inline int Write (char& v) { return WriteChar (v); }
//		inline int Write (fixang& v) { return WriteFixAng (v); }
		inline int Write (float& v) { return WriteFloat (v); }
		inline int Write (double& v) { return WriteDouble (v); }

		void* WriteBytes (void* buffer, int length);
		int WriteString (const char *buf);

		void WriteVector (const tAngleVector& v);
		void WriteVector (const tFixVector& v);
		void WriteVector (const tDoubleVector& v);
		inline void Write (const CAngleVector& v) { WriteVector (v.v); }
		inline void Write (const CFixVector& v) { WriteVector (v.v); }
		inline void Write (const CDoubleVector& v) { WriteVector (v.v); }

		void WriteMatrix (const CFixMatrix& m);
		void WriteMatrix (const CDoubleMatrix& m);
		inline void Write (const CFixMatrix& m) { WriteMatrix (m); }
		inline void Write (const CDoubleMatrix& m) { WriteMatrix (m); }

		int Copy (const char *pszSrc, const char *pszDest);
		byte* ReadData (const char *filename);
		time_t Date (const char *filename);

		static int Exist (const char *filename);	// Returns true if file exists on disk (1) or in hog (2).
		static int Delete (const char *filename);
		static int Rename (const char *oldname, const char *newname, const char *folder);
		static int MkDir (const char *pathname);
		static FILE* GetFileHandle (const char *filename, const char *mode);
		static void SplitPath (const char *szFullPath, char *szFolder, char *szFile, char *szExt);
		static void ChangeFilenameExtension (char *dest, const char *src, const char *new_ext);

		inline FILE*& File () { return m_cf.file; }
		inline char* Name () { return m_cf.filename; }
	};


#ifdef _WIN32
char *UnicodeToAsc (char *str, const wchar_t *w_str);
wchar_t *AscToUnicode (wchar_t *w_str, const char *str);
#endif

#ifdef _WIN32_WCE
# define errno -1
# define strerror (x) "Unknown Error"
#endif

#endif
