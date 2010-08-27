#ifndef __io_h
#define __io_h

double ReadDouble (FILE *fp);
int ReadInt32 (FILE *fp);
short ReadInt16 (FILE *fp);
char ReadInt8 (FILE *fp);

#define ReadFix(_fp)		(fix) ReadInt32 (_fp)
#define ReadFixAng(_fp)	(fixang) ReadInt16 (_fp)

int WriteInt32 (int value, FILE *fp);
short WriteInt16 (short value, FILE *fp);
char WriteInt8 (char value, FILE *fp);

#define WriteFix(_value, _fp)		WriteInt32 ((int) (_value), _fp)
#define WriteFixAng(_value, _fp)	WriteInt32 ((short) (_value), _fp)

double WriteDouble (double value, FILE *fp);

//#define read_matrix(_m, fp) (_m)->Read (fp)
//#define read_vector(_v, fp) (_v)->Read (fp)
//#define read_angvec(_a, fp) (_a)->Read (fp)
//#define write_matrix(_m, fp) (_m)->Write (fp)
//#define write_vector(_v, fp) (_v)->Write (fp)
//#define write_angvec(_a, fp) (_a)->Write (fp)

LPSTR FSplit (LPSTR fullName, LPSTR pathName, LPSTR fileName, LPSTR extName);
char *TimeStr (char *pszTime, int nDestSize);
char *DateStr (char *pszTime, int nDestSize, bool bMonthNames = false);
char *DateTimeStr (char *pszTime, int nDestSize, bool bMonthNames = false);

#endif //__io_h