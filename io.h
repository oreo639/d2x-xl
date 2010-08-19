#ifndef __io_h
#define __io_h

INT32 read_INT32(FILE *load_file);
INT16 read_INT16(FILE *load_file);
INT8 read_INT8(FILE *load_file);
FIX read_FIX(FILE *load_file);
FIXANG read_FIXANG(FILE *load_file);
tFixMatrix *read_matrix(tFixMatrix *matrix,FILE *load_file);
tFixVector *read_vector(tFixVector *vector,FILE *load_file);
tAngleVector *read_angvec(tAngleVector *vector,FILE *load_file);
INT32 write_INT32 (INT32 value,FILE *save_file);
INT16 write_INT16(INT16 value,FILE *save_file);
INT8 write_INT8(INT8 value,FILE *save_file);
FIX write_FIX(FIX value,FILE *save_file);
FIXANG write_FIXANG(FIXANG value,FILE *save_file);
tFixMatrix *write_matrix(tFixMatrix *matrix,FILE *save_file);
tFixVector *write_vector(tFixVector *vector,FILE *save_file);
tAngleVector *write_angvec(tAngleVector *vector,FILE *save_file);
LPSTR FSplit (LPSTR fullName, LPSTR pathName, LPSTR fileName, LPSTR extName);
char *TimeStr (char *pszTime, INT32 nDestSize);
char *DateStr (char *pszTime, INT32 nDestSize, bool bMonthNames = false);
char *DateTimeStr (char *pszTime, INT32 nDestSize, bool bMonthNames = false);

#endif //__io_h