// palette.h

#include "resourcemanager.h"

int Luminance (int r, int g, int b);
int HasCustomPalette (void);
void FreeCustomPalette (void);
int ReadCustomPalette (CFileManager& fp, long fSize);
int WriteCustomPalette (CFileManager& fp);
byte* PalettePtr (CResource& res);
LPCTSTR PaletteResource (void);
