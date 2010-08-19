// palette.cpp

#include "stdafx.h"
#include "dle-xp-res.h"

#include <string.h>

#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "io.h"
#include "palette.h"

UINT8 *pCustomPalette = NULL;
HGLOBAL hPalette;

//------------------------------------------------------------------------

INT32 Luminance (INT32 r, INT32 g, INT32 b)
{
	INT32 minColor, maxColor;

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

UINT8 FadeValue (UINT8 c, INT32 f)
{
return (UINT8) (((INT32) c * f) / 34);
}

//------------------------------------------------------------------------

INT32 HasCustomPalette (void)
{
return pCustomPalette != NULL;
}

//------------------------------------------------------------------------

void FreeCustomPalette (void)
{
if (pCustomPalette) {
	free (pCustomPalette);
	pCustomPalette = NULL;
	}
}

//------------------------------------------------------------------------

INT32 ReadCustomPalette (FILE *fp, long fSize)
{
FreeCustomPalette ();

UINT8 *pCustomPalette = (UINT8 *) malloc (37 * 256);

if (!pCustomPalette)
	return 0;

INT32 h = INT32 (fread (pCustomPalette, 37 * 256, 1, fp));
if (h == 37 * 256)
	return 1;

if (h != 3 * 256) {
	free (pCustomPalette);
	return 0;
	}

UINT8 *pFade = pCustomPalette + 3 * 256;

INT32 i, j;
for (i = 0; i < 256; i++) {
	UINT8	c = pCustomPalette [i];
	for (j = 0; j < 34; j++)
		pFade [j * 256 + i] = FadeValue (c, j + 1);
	}
return 1;
}

//------------------------------------------------------------------------

INT32 WriteCustomPalette (FILE *fp)
{
return fwrite (pCustomPalette, 37 * 256, 1, fp) == 37 * 256;
}

//------------------------------------------------------------------------

UINT8 * PalettePtr (void)
{
if (pCustomPalette)
	return pCustomPalette;
HINSTANCE hInst = AfxGetInstanceHandle();
HRSRC hResource = FindResource (hInst, PaletteResource (), "RC_DATA");
if (!hResource)
	return NULL;
hPalette = LoadResource (hInst, hResource);
if (!hPalette)
	return NULL;
return (UINT8 *) LockResource (hPalette);
}

//------------------------------------------------------------------------

void FreePaletteResource ()
{
if (!pCustomPalette && hPalette) {
	FreeResource (hPalette);
	hPalette = 0;
	}
}

//------------------------------------------------------------------------
// PaletteResource()
//
// Action - returns the name of the palette resource.  Neat part about
//          this function is that the strings are automatically stored
//          in the local heap so the string is static.
//
//------------------------------------------------------------------------

LPCTSTR PaletteResource (void) 
{
	typedef struct tPalExt {
		char	szFile [256];
		INT32	nIdPal;
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

INT32 id = IDR_GROUPA_256;
if (theApp.GetMine ()->IsD1File ())
	return MAKEINTRESOURCE (IDR_PALETTE_256);
FSplit (descent2_path, NULL, szFile, NULL);
for (ppe = palExt; *(ppe->szFile); ppe++)
	if (!_stricmp (ppe->szFile, szFile))
		return MAKEINTRESOURCE (ppe->nIdPal);
return MAKEINTRESOURCE (IDR_GROUPA_256);
}
