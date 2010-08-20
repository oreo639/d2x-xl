/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <string.h>
#if defined (_WIN32_WCE) || defined (_WIN32)
#	include <windows.h>
#	include <sys/stat.h>
#else
#	include <sys/stat.h>
#	include <errno.h>
#endif

#include "cfile.h"
//#include "findfile.h"

#define INTEL_SHORT
#define INTEL_INT
#define INTEL_FLOAT
#define INTEL_DOUBLE

INT32 nFileError = 0;

// ----------------------------------------------------------------------------

void SplitPath (const char *szFullPath, char *szFolder, char *szFile, char *szExt)
{
	INT32	h = 0, i, j, l = (INT32) strlen (szFullPath) - 1;

i = l;
#ifdef _WIN32
while ((i >= 0) && (szFullPath [i] != '/') && (szFullPath [i] != '\\') && (szFullPath [i] != ':'))
#else
while ((i >= 0) && (szFullPath [i] != '/'))
#endif
	i--;
i++;
j = l - 1;
while ((j > i) && (szFullPath [j] != '.'))
	j--;
if (szFolder) {
	if (i > 0)
		strncpy_s (szFolder, FILENAME_LEN, szFullPath, i);
	szFolder [i] = '\0';
	}
if (szFile) {
	if (!j || (j > i))
		strncpy_s (szFile, FILENAME_LEN, szFullPath + i, h = (j ? j : l + 1) - i);
	szFile [h] = '\0';
	}
if (szExt) {
	if (j && (j < l))
		strncpy_s (szExt, FILENAME_LEN, szFullPath + j, l - j);
	szExt [l - j] = '\0';
	}
}

//------------------------------------------------------------------------------

void CFile::ChangeFilenameExtension (char *dest, const char *src, const char *newExt)
{
	INT32 i;

strcpy_s (dest, FILENAME_LEN, src);
if (newExt [0] == '.')
	newExt++;
for (i = 1; i < (INT32) strlen (dest); i++)
	if ((dest [i] == '.') || (dest [i] == ' ') || (dest [i] == 0))
		break;
if (i < FILENAME_LEN - 5) {
	dest [i] = '.';
	dest [i+1] = newExt [0];
	dest [i+2] = newExt [1];
	dest [i+3] = newExt [2];
	dest [i+4] = 0;
	}
}

// ----------------------------------------------------------------------------

FILE *CFile::GetFileHandle (const char *filename, const char *folder, const char *mode) 
{
	FILE	*fp;
	char	fn [FILENAME_LEN];
	const char *pfn;

if (!*filename || (strlen (filename) + (folder ? strlen (folder) : 0) >= FILENAME_LEN)) {
	return NULL;
	}
if ((*filename != '/') && (strstr (filename, "./") != filename) && folder && *folder) {
sprintf_s (fn, FILENAME_LEN, "%s/%s", folder, filename);
   pfn = fn;
	}
 else
 	pfn = filename;
 
if (fopen_s (&fp, pfn, mode)) {
	fclose (fp);
	fp = NULL;
	}
//if (!fp) PrintLog ("CFGetFileHandle (): error opening %s\n", pfn);
return fp;
}

// ----------------------------------------------------------------------------
// CFile::EoF () Tests for end-of-file on a stream
//
// returns a nonzero value after the first read operation that attempts to read
// past the end of the file. It returns 0 if the current position is not end of file.
// There is no error return.

INT32 CFile::EoF (void)
{
#if DBG
if (!m_cf.file)
	return 1;
#endif
return (m_cf.rawPosition >= m_cf.size);
}

// ----------------------------------------------------------------------------

INT32 CFile::Error (void)
{
return nFileError;
}

// ----------------------------------------------------------------------------

INT32 CFile::Exist (const char *filename, const char *folder) 
{
	FILE	*fp;
	char	*pfn = const_cast<char*> (filename);

if ((fp = GetFileHandle (pfn, folder, "rb"))) { // Check for non-hogP file first...
	fclose (fp);
	return 1;
	}
return 0;
}

// ----------------------------------------------------------------------------
// Deletes a file.
INT32 CFile::Delete (const char *filename, const char* folder)
{
	char	fn [FILENAME_LEN];

sprintf_s (fn, FILENAME_LEN, "%s%s%s", folder, *folder ? "/" : "", filename);
#ifndef _WIN32_WCE
	return remove (fn);
#else
	return !DeleteFile (fn);
#endif
}

// ----------------------------------------------------------------------------
// Rename a file.
INT32 CFile::Rename (const char *oldname, const char *newname, const char *folder)
{
	char	fno [FILENAME_LEN], fnn [FILENAME_LEN];

sprintf_s (fno, FILENAME_LEN, "%s%s%s", folder, *folder ? "/" : "", oldname);
sprintf_s (fnn, FILENAME_LEN, "%s%s%s", folder, *folder ? "/" : "", newname);
#ifndef _WIN32_WCE
	return rename (fno, fnn);
#else
	return !MoveFile (fno, fnn);
#endif
}

// ----------------------------------------------------------------------------
// Make a directory.
INT32 CFile::MkDir (const char *pathname)
{
#if defined (_WIN32_WCE) || defined (_WIN32)
return !CreateDirectory (pathname, NULL);
#else
return mkdir (pathname, 0755);
#endif
}

// ----------------------------------------------------------------------------

static long ffilelength(FILE *file)
{
	long old_pos, size;

if ((old_pos = ftell(file)) == -1 ||
	 fseek(file, 0, SEEK_END) == -1 ||
	 (size = ftell(file)) == -1 ||
	 fseek(file, old_pos, SEEK_SET) == -1)
	return -1L;
return size;
}

// ----------------------------------------------------------------------------

INT32 CFile::Open (const char *filename, const char *folder, const char *mode) 
{
	INT32	length = -1;
	FILE	*fp = NULL;

m_cf.file = NULL;
if (!(filename && *filename))
	return 0;
fp = GetFileHandle (filename, folder, mode);		// Check for non-hogP file first...
if (!fp) 
	return 0;
m_cf.file = fp;
m_cf.rawPosition = 0;
m_cf.size = (length < 0) ? ffilelength (fp) : length;
m_cf.libOffset = (length < 0) ? 0 : ftell (fp);
m_cf.filename = const_cast<char*> (filename);
return 1;
}

// ----------------------------------------------------------------------------

void CFile::Init (void) 
{
memset (&m_cf, 0, sizeof (m_cf)); 
m_cf.rawPosition = -1; 
}

// ----------------------------------------------------------------------------

INT32 CFile::Length (void) 
{
return m_cf.size;
}

// ----------------------------------------------------------------------------
// Write () writes to the file
//
// returns:   number of full elements actually written
//
//
INT32 CFile::Write (const void *buf, INT32 nElemSize, INT32 nElemCount)
{
	INT32 nWritten;

if (!m_cf.file)
	return 0;
if (!(nElemSize * nElemCount))
	return 0;
nWritten = (INT32) fwrite (buf, nElemSize, nElemCount, m_cf.file);
m_cf.rawPosition = ftell (m_cf.file);
if (Error ())
	return 0;
return nWritten;
}

// ----------------------------------------------------------------------------
// CFile::PutC () writes a character to a file
//
// returns:   success ==> returns character written
//            error   ==> EOF
//
INT32 CFile::PutC (INT32 c)
{
	INT32 char_written;

char_written = fputc (c, m_cf.file);
m_cf.rawPosition = ftell (m_cf.file);
return char_written;
}

// ----------------------------------------------------------------------------

INT32 CFile::GetC (void) 
{
	INT32 c;

if (m_cf.rawPosition >= m_cf.size) 
	return EOF;
c = getc (m_cf.file);
if (c != EOF)
	m_cf.rawPosition = ftell (m_cf.file) - m_cf.libOffset;
return c;
}

// ----------------------------------------------------------------------------
// CFile::PutS () writes a string to a file
//
// returns:   success ==> non-negative value
//            error   ==> EOF
//
INT32 CFile::PutS (const char *str)
{
	INT32 ret;

ret = fputs (str, m_cf.file);
m_cf.rawPosition = ftell (m_cf.file);
return ret;
}

// ----------------------------------------------------------------------------

char * CFile::GetS (char * buf, size_t n) 
{
	char * t = buf;
	size_t i;
	INT32 c;

for (i = 0; i < n - 1; i++) {
	do {
		if (m_cf.rawPosition >= m_cf.size) {
			*buf = 0;
			return (buf == t) ? NULL : t;
			}
		c = GetC ();
		if (c == 0 || c == 10)       // Unix line ending
			break;
		if (c == 13) {      // Mac or DOS line ending
			INT32 c1 = GetC ();
			Seek ( -1, SEEK_CUR);
			if (c1 == 10) // DOS line ending
				continue;
			else            // Mac line ending
				break;
			}
		} while (c == 13);
 	if (c == 0 || c == 10 || c == 13)  // because cr-lf is a bad thing on the mac
 		break;   // and anyway -- 0xod is CR on mac, not 0x0a
	*buf++ = c;
	if (c == '\n')
		break;
	}
*buf++ = 0;
return  t;
}

// ----------------------------------------------------------------------------

size_t CFile::Read (void *buf, size_t elSize, size_t nElems) 
{
UINT32 i, size = (INT32) (elSize * nElems);

if (!m_cf.file || (m_cf.size < 1) || !size) 
	return 0;
i = (INT32) fread (buf, 1, size, m_cf.file);
m_cf.rawPosition += i;
return i / elSize;
}


// ----------------------------------------------------------------------------

INT32 CFile::Tell (void) 
{
return m_cf.rawPosition;
}

// ----------------------------------------------------------------------------

INT32 CFile::Seek (size_t offset, INT32 whence) 
{
if (!m_cf.size)
	return -1;

	INT32 destPos;

switch (whence) {
	case SEEK_SET:
		destPos = offset;
		break;
	case SEEK_CUR:
		destPos = m_cf.rawPosition + offset;
		break;
	case SEEK_END:
		destPos = m_cf.size + offset;
		break;
	default:
		return 1;
	}
INT32 c = fseek (m_cf.file, m_cf.libOffset + destPos, SEEK_SET);
m_cf.rawPosition = ftell (m_cf.file) - m_cf.libOffset;
return c;
}

// ----------------------------------------------------------------------------

INT32 CFile::Close (void)
{
	INT32 result;

if (!m_cf.file)
	return 0;
result = fclose (m_cf.file);
m_cf.file = NULL;
m_cf.size = 0;
m_cf.rawPosition = -1;
return result;
}

// ----------------------------------------------------------------------------
// routines to read basic data types from CFile::ILE's.  Put here to
// simplify mac/pc reading from cfiles.

INT32 CFile::ReadInt (void)
{
	INT32 i;

Read (&i, sizeof (i), 1);
//Error ("Error reading INT32 in CFile::ReadInt ()");
return INTEL_INT (i);
}

// ----------------------------------------------------------------------------

UINT32 CFile::ReadUInt (void)
{
	UINT32 i;

Read (&i, sizeof (i), 1);
//Error ("Error reading INT32 in CFile::ReadInt ()");
return INTEL_INT (i);
}

// ----------------------------------------------------------------------------

INT16 CFile::ReadShort (void)
{
	INT16 s;

Read (&s, sizeof (s), 1);
//Error ("Error reading INT16 in CFile::ReadShort ()");
return INTEL_SHORT (s);
}

// ----------------------------------------------------------------------------

UINT16 CFile::ReadUShort (void)
{
	UINT16 s;

Read (&s, sizeof (s), 1);
//Error ("Error reading INT16 in CFile::ReadShort ()");
return INTEL_SHORT (s);
}

// ----------------------------------------------------------------------------

INT8 CFile::ReadByte (void)
{
	INT8 b;

if (Read (&b, sizeof (b), 1) != 1)
	return nFileError;
//Error ("Error reading byte in CFile::ReadByte ()");
return b;
}

// ----------------------------------------------------------------------------

UINT8 CFile::ReadUByte (void)
{
	UINT8 b;

if (Read (&b, sizeof (b), 1) != 1)
	return nFileError;
//Error ("Error reading byte in CFile::ReadByte ()");
return b;
}

// ----------------------------------------------------------------------------

float CFile::ReadFloat (void)
{
	float f;

Read (&f, sizeof (f), 1) ;
//Error ("Error reading float in CFile::ReadFloat ()");
return INTEL_FLOAT (f);
}

// ----------------------------------------------------------------------------
//Read and return a double (64 bits)
//Throws an exception of nType (nFileError *) if the OS returns an error on read
double CFile::ReadDouble (void)
{
	double d;

Read (&d, sizeof (d), 1);
return INTEL_DOUBLE (d);
}

// ----------------------------------------------------------------------------

FIX CFile::ReadFix (void)
{
	FIX f;

Read (&f, sizeof (f), 1) ;
//Error ("Error reading FIX in CFile::ReadFix ()");
return (FIX) INTEL_INT ((INT32) f);
return f;
}

// ----------------------------------------------------------------------------

FIXANG CFile::ReadFixAng (void)
{
	FIXANG f;

Read (&f, 2, 1);
//Error ("Error reading FIXANG in CFile::ReadFixAng ()");
return (FIXANG) INTEL_SHORT ((INT32) f);
}

// ----------------------------------------------------------------------------

void CFile::ReadVector (CFixVector& v) 
{
v.x = ReadFix ();
v.y = ReadFix ();
v.z = ReadFix ();
}

// ----------------------------------------------------------------------------

void CFile::ReadAngVec (CAngleVector& v)
{
v.p = ReadFixAng ();
v.b = ReadFixAng ();
v.h = ReadFixAng ();
}

// ----------------------------------------------------------------------------

void CFile::ReadMatrix (CFixMatrix& m)
{
ReadVector (m.rvec);
ReadVector (m.uvec);
ReadVector (m.fvec);
}


// ----------------------------------------------------------------------------

void CFile::ReadString (char *buf, INT32 n)
{
	char c;

do {
	c = (char) ReadByte ();
	if (n > 0) {
		*buf++ = c;
		n--;
		}
	} while (c != 0);
}

// ----------------------------------------------------------------------------
// equivalent write functions of above read functions follow

INT32 CFile::WriteInt (INT32 i)
{
i = INTEL_INT (i);
return Write (&i, sizeof (i), 1);
}

// ----------------------------------------------------------------------------

INT32 CFile::WriteShort (INT16 s)
{
s = INTEL_SHORT (s);
return Write(&s, sizeof (s), 1);
}

// ----------------------------------------------------------------------------

INT32 CFile::WriteByte (INT8 b)
{
return Write (&b, sizeof (b), 1);
}

// ----------------------------------------------------------------------------

INT32 CFile::WriteFloat (float f)
{
f = INTEL_FLOAT (f);
return Write (&f, sizeof (f), 1);
}

// ----------------------------------------------------------------------------
//Read and return a double (64 bits)
//Throws an exception of nType (nFileError *) if the OS returns an error on read
INT32 CFile::WriteDouble (double d)
{
d = INTEL_DOUBLE (d);
return Write (&d, sizeof (d), 1);
}

// ----------------------------------------------------------------------------

INT32 CFile::WriteFix (FIX x)
{
x = INTEL_INT (x);
return Write (&x, sizeof (x), 1);
}

// ----------------------------------------------------------------------------

INT32 CFile::WriteFixAng (FIXANG a)
{
a = INTEL_SHORT (a);
return Write (&a, sizeof (a), 1);
}

// ----------------------------------------------------------------------------

void CFile::WriteVector (const CFixVector& v)
{
WriteFix (v.x);
WriteFix (v.y);
WriteFix (v.z);
}

// ----------------------------------------------------------------------------

void CFile::WriteAngVec (const CAngleVector& v)
{
WriteFixAng (v.p);
WriteFixAng (v.b);
WriteFixAng (v.h);
}

// ----------------------------------------------------------------------------

void CFile::WriteMatrix (const CFixMatrix& m)
{
WriteVector (m.rvec);
WriteVector (m.uvec);
WriteVector (m.fvec);
}


// ----------------------------------------------------------------------------

INT32 CFile::WriteString (const char *buf)
{
if (buf && *buf && Write (buf, (INT32) strlen (buf), 1))
	return (INT32) WriteByte (0);   // write out NULL termination
return 0;
}

// ----------------------------------------------------------------------------

INT32 CFile::Extract (const char *filename, const char *folder, const char *szDestName)
{
	FILE		*fp;
	char		szDest [FILENAME_LEN], fn [FILENAME_LEN];
	static	char buf [4096];
	INT32		h, l;

if (!Open (filename, folder, "rb"))
	return 0;
strcpy_s (fn, FILENAME_LEN, filename);
if (*szDestName) {
	if (*szDestName == '.') {
		char *psz = strchr (fn, '.');
		if (psz)
			strcpy_s (psz, FILENAME_LEN, szDestName);
		else
			strcat_s (fn, FILENAME_LEN, szDestName);
		}
	else
		strcpy_s (fn, FILENAME_LEN, szDestName);
	}
sprintf_s (szDest, FILENAME_LEN, "%s%s%s", gameFolders.szCacheDir, *gameFolders.szCacheDir ? "/" : "", fn);
if (fopen_s (&fp, szDest, "wb")) {
	Close ();
	return 0;
	}
for (h = sizeof (buf), l = m_cf.size; l; l -= h) {
	if (h > l)
		h = l;
	Read (buf, h, 1);
	fwrite (buf, h, 1, fp);
	}
Close ();
fclose (fp);
return 1;
}

//	-----------------------------------------------------------------------------------
//	Imagine if C had a function to copy a file...

#define COPY_BUF_SIZE 65536

INT32 CFile::Copy (const char *pszSrc, const char *pszDest)
{
	INT8	buf [COPY_BUF_SIZE];
	CFile	cf;

if (!cf.Open (pszDest, gameFolders.szSaveDir, "wb"))
	return -1;
if (!Open (pszSrc, gameFolders.szSaveDir, "rb"))
	return -2;
while (!EoF ()) {
	INT32 bytes_read = (INT32) Read (buf, 1, COPY_BUF_SIZE);
	if (Error ()) {
		cf.Close ();
		return -2;
	}
	cf.Write (buf, 1, bytes_read);
	if (cf.Error ()) {
		cf.Close ();
		return -2;
	}
	}
if (Close ()) {
	cf.Close ();
	return -3;
	}
if (cf.Close ())
	return -4;
return 0;
}

// ----------------------------------------------------------------------------

char *CFile::ReadData (const char *filename, const char *folder)
{
	char		*pData = NULL;
	size_t	nSize;

if (!Open (filename, folder, "rb"))
	return NULL;
nSize = Length ();
if (!(pData = new char [nSize]))
	return NULL;
if (!Read (pData, nSize, 1)) {
	delete[] pData;
	pData = NULL;
	}
Close ();
return pData;
}

// ----------------------------------------------------------------------------

void CFile::SplitPath (const char *szFullPath, char *szFolder, char *szFile, char *szExt)
{
	INT32	h = 0, i, j, l = (INT32) strlen (szFullPath) - 1;

i = l;
#ifdef _WIN32
while ((i >= 0) && (szFullPath [i] != '/') && (szFullPath [i] != '\\') && (szFullPath [i] != ':'))
#else
while ((i >= 0) && (szFullPath [i] != '/'))
#endif
	i--;
i++;
j = l;
while ((j > i) && (szFullPath [j] != '.'))
	j--;
if (szExt) {
	if (szFullPath [j] != '.')
		*szExt = '\0';
	else {
		strncpy_s (szExt, FILENAME_LEN, szFullPath + j, l - j + 1);
		szExt [l - j + 1] = '\0';
		}
	}
if (szFolder) {
	if (i > 0)
		strncpy_s (szFolder, FILENAME_LEN, szFullPath, i);
	szFolder [i] = '\0';
	}
if (szFile) {
	if (!j || (j > i))
		strncpy_s (szFile, FILENAME_LEN, szFullPath + i, h = (j ? j : l + 1) - i);
	szFile [h] = '\0';
	}
}

// ----------------------------------------------------------------------------

time_t CFile::Date (const char *filename, const char *folder)
{
	struct stat statbuf;

//	sprintf (fn, "%s/%s", folder, hogname);
if (!Open (filename, folder, "rb"))
	return -1;
#ifdef _WIN32
fstat (_fileno (m_cf.file), &statbuf);
#else
fstat (fileno (m_cf.file), &statbuf);
#endif
Close ();
return statbuf.st_mtime;
}

// ----------------------------------------------------------------------------
