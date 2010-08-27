// palette.h

int Luminance (int r, int g, int b);
int HasCustomPalette (void);
void FreeCustomPalette (void);
int ReadCustomPalette (FILE *fp, long fSize);
int WriteCustomPalette (FILE *fp);
byte * PalettePtr (void);
LPCTSTR PaletteResource ();
void FreePaletteResource ();
