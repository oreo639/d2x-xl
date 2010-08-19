// palette.h

INT32 Luminance (INT32 r, INT32 g, INT32 b);
INT32 HasCustomPalette (void);
void FreeCustomPalette (void);
INT32 ReadCustomPalette (FILE *fp, long fSize);
INT32 WriteCustomPalette (FILE *fp);
UINT8 * PalettePtr (void);
LPCTSTR PaletteResource ();
void FreePaletteResource ();
