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
#	include <sys/stat.h>
#else
#	include <sys/stat.h>
#	include <errno.h>
#endif

#include "FileManager.h"
//#include "findfile.h"

#define INTEL_SHORT
#define INTEL_INT
#define INTEL_FLOAT
#define INTEL_DOUBLE

int nFileError = 0;

// ----------------------------------------------------------------------------

#define ReadScalar(_T) { _T i; Read (&i, sizeof (i), 1); return i; }

#define WriteScalar(i) { return Write (&i, sizeof (i), 1); }

// ----------------------------------------------------------------------------

void SplitPath (const char *szFullPath, char *szFolder, char *szFile, char *szExt)
{
	int	h = 0, i, j, l = (int) strlen (szFullPath) - 1;

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

void CFileManager::ChangeFilenameExtension (char *dest, const char *src, const char *newExt)
{
	int i;

strcpy_s (dest, FILENAME_LEN, src);
if (newExt [0] == '.')
	newExt++;
for (i = 1; i < (int) strlen (dest); i++)
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

FILE* CFileManager::GetFileHandle (const char *filename, const char *mode) 
{
	FILE	*fp;

if (fopen_s (&fp, filename, mode)) {
	fclose (fp);
	fp = null;
	}
return fp;
}

// ----------------------------------------------------------------------------
// CFileManager::EoF () Tests for end-of-file on a stream
//
// returns a nonzero value after the first read operation that attempts to read
// past the end of the file. It returns 0 if the current position is not end of file.
// There is no error return.

int CFileManager::EoF (void)
{
#if DBG
if (!m_info.file)
	return 1;
#endif
Tell ();
return (m_info.position >= m_info.size);
}

// ----------------------------------------------------------------------------

int CFileManager::Error (void)
{
return nFileError;
}

// ----------------------------------------------------------------------------

int CFileManager::Exist (const char *filename) 
{
	FILE *fp;

if ((fp = GetFileHandle (filename, "rb"))) { // Check for non-hogP file first...
	fclose (fp);
	return 1;
	}
return 0;
}

// ----------------------------------------------------------------------------
// Deletes a file.
int CFileManager::Delete (const char *filename)
{
#ifndef _WIN32_WCE
	return remove (filename);
#else
	return !DeleteFile (fn);
#endif
}

// ----------------------------------------------------------------------------
// Rename a file.
int CFileManager::Rename (const char *oldname, const char *newname, const char *folder)
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
int CFileManager::MkDir (const char *pathname)
{
#if defined (_WIN32_WCE) || defined (_WIN32)
return !CreateDirectory (pathname, null);
#else
return mkdir (pathname, 0755);
#endif
}

// ----------------------------------------------------------------------------

static long ffilelength (FILE* fp)
{
	long oldPos, size;

if (((oldPos = ftell (fp)) == -1) ||
	 (fseek (fp, 0, SEEK_END) == -1) ||
	 ((size = ftell (fp)) == -1) ||
	 (fseek (fp, oldPos, SEEK_SET) == -1))
	return -1;
return size;
}

// ----------------------------------------------------------------------------

int CFileManager::Open (const char *filename, const char *mode) 
{
m_info.file = null;
if (!(filename && *filename))
	return 1;
if (fopen_s (&m_info.file, filename, mode))
	return 1;
m_info.position = Tell ();
m_info.size = ffilelength (m_info.file);
m_info.name = const_cast<char*> (filename);
return 0;
}

// ----------------------------------------------------------------------------

void CFileManager::Init (void) 
{
memset (&m_info, 0, sizeof (m_info)); 
m_info.position = -1; 
}

// ----------------------------------------------------------------------------

int CFileManager::Length (void) 
{
return m_info.size;
}

// ----------------------------------------------------------------------------
// Write () writes to the file
//
// returns:   number of full elements actually written
//
//
int CFileManager::Write (const void *buffer, int nElemSize, int nElemCount)
{
	int nWritten;

if (!m_info.file)
	return 0;
if (!(nElemSize * nElemCount))
	return 0;
nWritten = (int) fwrite (buffer, nElemSize, nElemCount, m_info.file);
Tell ();
if (Error ())
	return 0;
return nWritten;
}

// ----------------------------------------------------------------------------
// CFileManager::PutC () writes a character to a file
//
// returns:   success ==> returns character written
//            error   ==> EOF
//
int CFileManager::PutC (int c)
{
	int char_written;

char_written = fputc (c, m_info.file);
Tell ();
return char_written;
}

// ----------------------------------------------------------------------------

int CFileManager::GetC (void) 
{
	int c;

if (m_info.position >= m_info.size) 
	return EOF;
c = getc (m_info.file);
if (c != EOF)
	Tell ();
return c;
}

// ----------------------------------------------------------------------------
// CFileManager::PutS () writes a string to a file
//
// returns:   success ==> non-negative value
//            error   ==> EOF
//
int CFileManager::PutS (const char *str)
{
	int ret;

ret = fputs (str, m_info.file);
Tell ();
return ret;
}

// ----------------------------------------------------------------------------

char * CFileManager::GetS (char * buffer, size_t n) 
{
	char * t = buffer;
	size_t i;
	int c;

for (i = 0; i < n - 1; i++) {
	do {
		if (m_info.position >= m_info.size) {
			*buffer = 0;
			return (buffer == t) ? null : t;
			}
		c = GetC ();
		if (c == 0 || c == 10)       // Unix line ending
			break;
		if (c == 13) {      // Mac or DOS line ending
			int c1 = GetC ();
			Seek (-1, SEEK_CUR);
			if (c1 == 10) // DOS line ending
				continue;
			else            // Mac line ending
				break;
			}
		} while (c == 13);
 	if (c == 0 || c == 10 || c == 13)  // because cr-lf is a bad thing on the mac
 		break;   // and anyway -- 0xod is CR on mac, not 0x0a
	*buffer++ = c;
	if (c == '\n')
		break;
	}
*buffer++ = 0;
return  t;
}

// ----------------------------------------------------------------------------

size_t CFileManager::Read (void *buffer, size_t elSize, size_t nElems) 
{
uint i, size = (int) (elSize * nElems);

if (!m_info.file || (m_info.size < 1) || !size) 
	return 0;
i = (int) fread (buffer, 1, size, m_info.file);
m_info.position += i;
return i / elSize;
}

// ----------------------------------------------------------------------------

int CFileManager::Tell (void) 
{
return SetPos (ftell (m_info.file));
}

// ----------------------------------------------------------------------------

int CFileManager::Seek (size_t offset, int whence) 
{
if (!m_info.file)
	return -1;
if (fseek (m_info.file, (long) offset, whence) != 0)
	return -1;
return Tell ();
}

// ----------------------------------------------------------------------------

int CFileManager::Close (bool bReset)
{
	int result;

if (!m_info.file)
	return 0;
result = fclose (m_info.file);
m_info.file = null;
if (bReset) {
	m_info.size = 0;
	m_info.position = -1;
	}
return result;
}

// ----------------------------------------------------------------------------

int CFileManager::ReadInt32 (void) { ReadScalar (int) }
uint CFileManager::ReadUInt32 (void) { ReadScalar (uint) }
short CFileManager::ReadInt16 (void) { ReadScalar (short) }
ushort CFileManager::ReadUInt16 (void) { ReadScalar (ushort) }
sbyte CFileManager::ReadSByte (void) { ReadScalar (sbyte) }
ubyte CFileManager::ReadUByte (void) { ReadScalar (ubyte) }
char CFileManager::ReadChar (void) { ReadScalar (char) }
float CFileManager::ReadFloat (void) { ReadScalar (float) }
double CFileManager::ReadDouble (void) { ReadScalar (double) }
short CFileManager::ReadFixAng (void) { ReadScalar (short) }

int CFileManager::WriteInt32 (int v) { WriteScalar (v) }
int CFileManager::WriteUInt32 (uint v) { WriteScalar (v) }
int CFileManager::WriteInt16 (short v) { WriteScalar (v) }
int CFileManager::WriteUInt16 (ushort v) { WriteScalar (v) }
int CFileManager::WriteSByte (sbyte v) { WriteScalar (v) }
int CFileManager::WriteByte (ubyte v) { WriteScalar (v) }
int CFileManager::WriteChar (char v) { WriteScalar (v) }
int CFileManager::WriteFloat (float v) { WriteScalar (v) }
int CFileManager::WriteDouble (double v) { WriteScalar (v) }

// ----------------------------------------------------------------------------

int CFileManager::ReadVector (tFixVector& v) 
{
v.x = ReadInt32 ();
v.y = ReadInt32 ();
v.z = ReadInt32 ();
return 1;
}

// ----------------------------------------------------------------------------

int CFileManager::ReadVector (tDoubleVector& v) 
{
v.x = X2D (ReadInt32 ());
v.y = X2D (ReadInt32 ());
v.z = X2D (ReadInt32 ());
return 1;
}

// ----------------------------------------------------------------------------

int CFileManager::ReadVector (tAngleVector& v)
{
v.p = ReadFixAng ();
v.b = ReadFixAng ();
v.h = ReadFixAng ();
return 1;
}

// ----------------------------------------------------------------------------

int CFileManager::ReadMatrix (CFixMatrix& m)
{
ReadVector (m.m.rVec.v);
ReadVector (m.m.uVec.v);
ReadVector (m.m.fVec.v);
return 1;
}

// ----------------------------------------------------------------------------

int CFileManager::ReadMatrix (CDoubleMatrix& m)
{
ReadVector (m.m.rVec.v);
ReadVector (m.m.uVec.v);
ReadVector (m.m.fVec.v);
return 1;
}

// ----------------------------------------------------------------------------

void* CFileManager::ReadBytes (void* buffer, int length)
{
Read (buffer, 1, length);
return buffer;
}

// ----------------------------------------------------------------------------

void CFileManager::ReadString (char *buffer, int length)
{
	char c;

do {
	c = (char) ReadUByte ();
	if (length > 0) {
		*buffer++ = c;
		length--;
		}
	} while (c != 0);
}

// ----------------------------------------------------------------------------

void CFileManager::WriteVector (const tFixVector& v)
{
WriteInt32 (v.x);
WriteInt32 (v.y);
WriteInt32 (v.z);
}

// ----------------------------------------------------------------------------

void CFileManager::WriteVector (const tDoubleVector& v)
{
WriteInt32 (D2X (v.x));
WriteInt32 (D2X (v.y));
WriteInt32 (D2X (v.z));
}

// ----------------------------------------------------------------------------

void CFileManager::WriteVector (const tAngleVector& v)
{
WriteInt16 (v.p);
WriteInt16 (v.b);
WriteInt16 (v.h);
}

// ----------------------------------------------------------------------------

void CFileManager::WriteMatrix (const CFixMatrix& m)
{
WriteVector (m.m.rVec.v);
WriteVector (m.m.uVec.v);
WriteVector (m.m.fVec.v);
}

// ----------------------------------------------------------------------------

void CFileManager::WriteMatrix (const CDoubleMatrix& m)
{
WriteVector (m.m.rVec.v);
WriteVector (m.m.uVec.v);
WriteVector (m.m.fVec.v);
}

// ----------------------------------------------------------------------------

void* CFileManager::WriteBytes (void* buffer, int length)
{
Write (buffer, 1, length);
return buffer;
}

// ----------------------------------------------------------------------------

int CFileManager::WriteString (const char *buffer)
{
if (buffer && *buffer && Write (buffer, (int) strlen (buffer), 1))
	return (int) WriteByte (0);   // write out null termination
return 0;
}

//	-----------------------------------------------------------------------------------
//	Imagine if C had a function to copy a file...

#define COPY_BUF_SIZE 65536

int CFileManager::Copy (const char *pszSrc, const char *pszDest)
{
	INT8	buffer [COPY_BUF_SIZE];
	CFileManager	cf;

if (cf.Open (pszDest, "wb"))
	return -1;
if (Open (pszSrc, "rb"))
	return -2;
while (!EoF ()) {
	int nRead = (int) Read (buffer, 1, COPY_BUF_SIZE);
	if (Error ()) {
		cf.Close ();
		return -2;
	}
	cf.Write (buffer, 1, nRead);
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

ubyte *CFileManager::ReadData (const char *filename)
{
	ubyte*		buffer = null;
	size_t	nSize;

if (!Open (filename, "rb"))
	return null;
nSize = Length ();
if (!(buffer = new ubyte [nSize]))
	return null;
if (!Read (buffer, nSize, 1)) {
	delete[] buffer;
	buffer = null;
	}
Close ();
return buffer;
}

// ----------------------------------------------------------------------------

void CFileManager::SplitPath (const char *szFullPath, char *szFolder, char *szFile, char *szExt)
{
	int	h = 0, i, j, l = (int) strlen (szFullPath) - 1;

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

time_t CFileManager::Date (const char* filename)
{
	struct stat statbuf;

//	sprintf (fn, "%s/%s", folder, hogname);
if (Open (filename, "rb"))
	return -1;
#ifdef _WIN32
fstat (_fileno (m_info.file), &statbuf);
#else
fstat (fileno (m_info.file), &statbuf);
#endif
Close ();
return statbuf.st_mtime;
}

// ----------------------------------------------------------------------------
