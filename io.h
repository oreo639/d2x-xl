#ifndef __io_h
#define __io_h

DOUBLE read_INT32(FILE *fp);
INT32 read_INT32(FILE *fp);
INT16 read_INT16(FILE *fp);
INT8 read_INT8(FILE *fp);
FIX read_FIX(FILE *fp);
FIXANG read_FIXANG(FILE *fp);
tFixMatrix *read_matrix(tFixMatrix *matrix,FILE *fp);
tFixVector *read_vector(tFixVector *vector,FILE *fp);
tAngleVector *read_angvec(tAngleVector *vector,FILE *fp);
INT32 write_INT32 (INT32 value,FILE *fp);
INT16 write_INT16(INT16 value,FILE *fp);
INT8 write_INT8(INT8 value,FILE *fp);
FIX write_FIX(FIX value,FILE *fp);
FIXANG write_FIXANG(FIXANG value,FILE *fp);
tFixMatrix *write_matrix(tFixMatrix *matrix,FILE *fp);
tFixVector *write_vector(tFixVector *vector,FILE *fp);
tAngleVector *write_angvec(tAngleVector *vector,FILE *fp);
LPSTR FSplit (LPSTR fullName, LPSTR pathName, LPSTR fileName, LPSTR extName);
char *TimeStr (char *pszTime, INT32 nDestSize);
char *DateStr (char *pszTime, INT32 nDestSize, bool bMonthNames = false);
char *DateTimeStr (char *pszTime, INT32 nDestSize, bool bMonthNames = false);

#endif //__io_h