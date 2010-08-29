// palette.h

int Luminance (int r, int g, int b);
int HasCustomPalette (void);
void FreeCustomPalette (void);
int ReadCustomPalette (CFileManager& fp, long fSize);
int WriteCustomPalette (CFileManager& fp);
byte * PalettePtr (void);
LPCTSTR PaletteResource ();
void FreePaletteResource ();
