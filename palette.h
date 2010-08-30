// palette.h

#include "resourcemanager.h"

int Luminance (int r, int g, int b);
int HasCustomPalette (void);
void FreeCustomPalette (void);
void FreeCurrentPalette (void);
int LoadCustomPalette (CFileManager& fp, long fSize);
int WriteCustomPalette (CFileManager& fp);
byte* PalettePtr (void);
const char* PaletteResource (void);
