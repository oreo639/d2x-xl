// palette.cpp

#include "stdafx.h"
#include "dle-xp-res.h"

#include <string.h>

#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "cfile.h"
#include "palette.h"

byte *pCustomPalette = null;
HGLOBAL hPalette;

//------------------------------------------------------------------------

int Luminance (int r, int g, int b)
{
	int minColor, maxColor;

if (r < g) {
	minColor = (r < b) ? r : b;
	maxColor = (g > b) ? g : b;
	}
else {
	minColor = (g < b) ? g : b;
	maxColor = (r > b) ? r : b;
	}
return (minColor + maxColor) / 2;
}

//------------------------------------------------------------------------

byte FadeValue (byte c, int f)
{
return (byte) (((int) c * f) / 34);
}

//------------------------------------------------------------------------

int HasCustomPalette (void)
{
return pCustomPalette != null;
}

//------------------------------------------------------------------------

void FreeCustomPalette (void)
{
if (pCustomPalette) {
	free (pCustomPalette);
	pCustomPalette = null;
	}
}

//------------------------------------------------------------------------

int ReadCustomPalette (CFileManager& fp, long fSize)
{
FreeCustomPalette ();

byte *pCustomPalette = (byte *) malloc (37 * 256);

if (!pCustomPalette)
	return 0;

int h = (int) fp.Read (pCustomPalette, 37 * 256, 1);
if (h == 37 * 256)
	return 1;

if (h != 3 * 256) {
	free (pCustomPalette);
	return 0;
	}

byte *pFade = pCustomPalette + 3 * 256;

int i, j;
for (i = 0; i < 256; i++) {
	byte	c = pCustomPalette [i];
	for (j = 0; j < 34; j++)
		pFade [j * 256 + i] = FadeValue (c, j + 1);
	}
return 1;
}

//------------------------------------------------------------------------

int WriteCustomPalette (CFileManager& fp)
{
return fp.Write (pCustomPalette, 37 * 256, 1) == 1;
}

//------------------------------------------------------------------------

byte* PalettePtr (CResource& res)
{
return pCustomPalette ? pCustomPalette : res.Load (PaletteResource ());
}

//------------------------------------------------------------------------
// PaletteResource()
//
// Action - returns the name of the palette resource.  Neat part about
//          this function is that the strings are automatically stored
//          in the local heap so the string is static.
//
//------------------------------------------------------------------------

const char* PaletteResource (void) 
{
	typedef struct tPalExt {
		char	szFile [256];
		int	nIdPal;
	} tPalExt;

	static tPalExt palExt [] = {
		{"groupa", IDR_GROUPA_256}, 
		{"alien1", IDR_ALIEN1_256}, 
		{"alien2", IDR_ALIEN2_256}, 
		{"fire", IDR_FIRE_256}, 
		{"water", IDR_WATER_256}, 
		{"ice", IDR_ICE_256},
		{"", 0}
	};
	tPalExt	*ppe;
	char		szFile [256];

int id = IDR_GROUPA_256;
if (theMine && theMine->IsD1File ())
	return MAKEINTRESOURCE (IDR_PALETTE_256);
CFileManager fp;
fp.SplitPath (descent2_path, null, szFile, null);
for (ppe = palExt; *(ppe->szFile); ppe++)
	if (!_stricmp (ppe->szFile, szFile))
		return MAKEINTRESOURCE (ppe->nIdPal);
return MAKEINTRESOURCE (IDR_GROUPA_256);
}
