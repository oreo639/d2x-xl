#include "stdafx.h"

#include "define.h"
#include "types.h"
#include "io.h"

//------------------------------------------------------------------------
// ReadInt32 ()
//
// ACTION - Reads a 32 bit word from a file.
//------------------------------------------------------------------------

double ReadDouble (FILE *fp) 
{
double value = 0;
fread (&value, sizeof (double), 1, fp);
return value;
}

//------------------------------------------------------------------------
// ReadInt32 ()
//
// ACTION - Reads a 32 bit word from a file.
//------------------------------------------------------------------------

int ReadInt32 (FILE *fp) 
{
int value = 0;
fread (&value, sizeof (int), 1, fp);
return value;
}

//------------------------------------------------------------------------
// ReadUInt32 ()
//
// ACTION - Reads a 32 bit word from a file.
//------------------------------------------------------------------------

uint ReadUInt32 (FILE *fp) 
{
uint value = 0;
fread (&value, sizeof (uint), 1, fp);
return value;
}

//------------------------------------------------------------------------
// ReadInt16 ()
//
// ACTION - Reads a 16 bit word from a file.
//------------------------------------------------------------------------

short ReadInt16 (FILE *fp) 
{
short value = 0;
fread (&value, sizeof (short), 1, fp);
return value;
}

//------------------------------------------------------------------------
// ReadUInt16 ()
//
// ACTION - Reads a 16 bit word from a file.
//------------------------------------------------------------------------

ushort ReadUInt16 (FILE *fp) 
{
ushort value = 0;
fread (&value, sizeof (ushort), 1, fp);
return value;
}

//------------------------------------------------------------------------
// ReadInt8 ()
//
// ACTION - Reads a 8 bit word from a file.
//------------------------------------------------------------------------

char ReadInt8 (FILE *fp) 
{
char value = 0;
fread (&value, sizeof (char), 1, fp);
return value;
}

//------------------------------------------------------------------------
// ReadInt8 ()
//
// ACTION - Reads a 8 bit word from a file.
//------------------------------------------------------------------------

byte ReadUInt8 (FILE *fp) 
{
byte value = 0;
fread (&value, sizeof (byte), 1, fp);
return value;
}

//------------------------------------------------------------------------
// ReadBytes ()
//
// ACTION - Reads a number of 8 bit bytes from a file.
//------------------------------------------------------------------------

byte* ReadBytes (char* buffer, uint length, FILE *fp) 
{
fread (buffer, sizeof (byte), length, fp);
return buffer;
}

//------------------------------------------------------------------------
// WriteInt32 ()
//
// ACTION - Writes a 32 bit word to a file.
//------------------------------------------------------------------------

int WriteInt32 (int value, FILE *fp) 
{
fwrite (&value, sizeof (int), 1, fp);
return (value);
}

//------------------------------------------------------------------------
// WriteInt16 ()
//
// ACTION - Writes a 16 bit word to a file.
//------------------------------------------------------------------------

short WriteInt16 (short value, FILE *fp) 
{
fwrite (&value, sizeof (short), 1, fp);
return (value);
}

//------------------------------------------------------------------------
// WriteInt8 ()
//
// ACTION - Writes a 8 bit word to a file.
//------------------------------------------------------------------------

char WriteInt8 (char value, FILE *fp) 
{
fwrite (&value, sizeof (char), 1, fp);
return (value);
}

//------------------------------------------------------------------------
// WriteInt32 ()
//
// ACTION - Writes a 32 bit word to a file.
//------------------------------------------------------------------------

int WriteUInt32 (uint value, FILE *fp) 
{
fwrite (&value, sizeof (uint), 1, fp);
return (value);
}

//------------------------------------------------------------------------
// WriteInt16 ()
//
// ACTION - Writes a 16 bit word to a file.
//------------------------------------------------------------------------

short WriteuInt16 (ushort value, FILE *fp) 
{
fwrite (&value, sizeof (ushort), 1, fp);
return (value);
}

//------------------------------------------------------------------------
// WriteInt8 ()
//
// ACTION - Writes a 8 bit word to a file.
//------------------------------------------------------------------------

char WriteuInt8 (byte value, FILE *fp) 
{
fwrite (&value, sizeof (byte), 1, fp);
return (value);
}

//------------------------------------------------------------------------
// WriteDouble ()
//
// ACTION - Writes a double to a file.
//------------------------------------------------------------------------

double WriteDouble (double value, FILE *fp) 
{
fwrite (&value, sizeof (double), 1, fp);
return (value);
}

//------------------------------------------------------------------------
// ReadBytes ()
//
// ACTION - Reads a number of 8 bit bytes from a file.
//------------------------------------------------------------------------

byte* WriteBytes (char* buffer, uint length, FILE *fp) 
{
fwrite (buffer, sizeof (byte), length, fp);
return buffer;
}

                        /*---------------------------*/

static char *CopyIoName (char *dest, char *src, ushort srcLen, ushort destSize)
{
if (dest) {
   if (srcLen > --destSize)
      srcLen = destSize;
   strncpy_s (dest, 256, src, srcLen);
   dest [srcLen] = '\0';
	}
return dest;
}

                        /*---------------------------*/

char *FSplit (char *fullName, char *pathName, char *fileName, char *extName)
{
   char *s;
   char	fn [256];
	int	l;

l = int (strlen (fullName));
memcpy (fn, fullName, l + 1);
if (pathName)
   *pathName = 0;
for (s = fn + l; (s != fn); --s) {
   if ( (*s == ':') || (*s == '\\')) {
      CopyIoName (pathName, fn, (ushort) (s - fn) + 1, 256);
      memmove (fn, s + 1, strlen (s));
      break;
      }
   }

for (s = fn + strlen (fn); (s != fn); --s)
   if ( (*s == '.') || (*s == ':') || (*s == '\\'))
      break;
if (*s == '.') {
   CopyIoName (extName, s, (ushort) strlen (s), 256);
   if (fileName) {
      *s = 0;
      CopyIoName (fileName, fn, (ushort) strlen (fn), 256);
      }
   }
else {
   CopyIoName (fileName, fn, (ushort) strlen (fn), 256);
   if (extName)
      *extName = 0;
   }
return pathName ? pathName : fileName ? fileName : extName;
}

/*----------------------------------------------------------------------------+
|                                                                             |
+----------------------------------------------------------------------------*/

#include <stdio.h>

static char *szMonths [] = {"jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec"};

struct tm *GetTimeDate (struct tm *td)
{
   time_t t;
   static struct tm h;

time (&t);
localtime_s (&h, &t);
h.tm_mon++;
if (!td)
   return &h;
*td = h;
td->tm_year += (td->tm_year < 80 ? 2000 : 1900);
return td;
}


char *TimeStr (char *pszTime, int nDestSize)
{
	struct tm td;

GetTimeDate (&td);
sprintf_s (pszTime, nDestSize, "%d:%02d.%02d", td.tm_hour, td.tm_min, td.tm_sec);
return pszTime;
} 


char *DateStr (char *pszTime, int nDestSize, bool bMonthNames)
{
	struct tm td;

GetTimeDate (&td);
if (bMonthNames)
	sprintf_s (pszTime, nDestSize, "%d %s %d", td.tm_mday, szMonths [td.tm_mon - 1], td.tm_year);
else
	sprintf_s (pszTime, nDestSize, "%d/%d/%d", td.tm_mon, td.tm_mday, td.tm_year);
return pszTime;
} 


char *TimeDateStr (char *pszTime, int nDestSize, bool bMonthNames)
{
	struct tm td;

GetTimeDate (&td);
if (bMonthNames)
	sprintf_s (pszTime, nDestSize, "%d %s %d %d:%02d",
		       td.tm_mday, szMonths [td.tm_mon - 1], td.tm_year,
			    td.tm_hour, td.tm_min);
else
	sprintf_s (pszTime, nDestSize, "%d/%d/%d %d:%02d",
		       td.tm_mon, td.tm_mday, td.tm_year,
			    td.tm_hour, td.tm_min);
return pszTime;
} 


