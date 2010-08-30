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

byte* m_custom = null;
byte* m_current = null;

HGLOBAL hPalette;

CPaletteManager paletteManager;

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

void CPaletteManager::SetupBMI (void) 
{
byte* palette = Current ();
if (!palette)
	return;
m_bmi.header.biSize          = sizeof (BITMAPINFOHEADER);
m_bmi.header.biWidth         = 64;
m_bmi.header.biHeight        = 64;
m_bmi.header.biPlanes        = 1;
m_bmi.header.biBitCount      = 8;
m_bmi.header.biCompression   = BI_RGB;
m_bmi.header.biSizeImage     = 0;
m_bmi.header.biXPelsPerMeter = 0;
m_bmi.header.biYPelsPerMeter = 0;
m_bmi.header.biClrUsed       = 0;
m_bmi.header.biClrImportant  = 0;

uint* rgb = (uint*) m_bmi.colors;
for (int i = 256; i; i--, palette += 3, rgb++)
	*rgb = ((uint) (palette [0]) << 18) + ((uint) (palette [1]) << 10) + ((uint) (palette [2]) << 2);
}

//------------------------------------------------------------------------

byte CPaletteManager::FadeValue (byte c, int f)
{
return (byte) (((int) c * f) / 34);
}

//------------------------------------------------------------------------

void CPaletteManager::FreeCustom (void)
{
if (m_custom) {
	delete m_custom;
	m_custom = null;
	}
}

//------------------------------------------------------------------------

void CPaletteManager::FreeCurrent (void)
{
if (m_current) {
	delete m_current;
	m_current = null;
	}
}

//------------------------------------------------------------------------

int CPaletteManager::LoadCustom (CFileManager& fp, long size)
{
FreeCustom ();
if (!(m_custom = new byte [37 * 256]))
	return null;

int h = (int) fp.Read (m_custom, 37 * 256, 1);
if (h == 37 * 256)
	return 1;

if (h != 3 * 256) {
	paletteManager.FreeCustom ();
	return 0;
	}
byte *fadeP = m_custom + 3 * 256;
for (int i = 0; i < 256; i++) {
	byte c = m_custom [i];
	for (int j = 0; j < 34; j++)
		fadeP [j * 256 + i] = FadeValue (c, j + 1);
	}
SetupBMI ();
return 1;
}

//------------------------------------------------------------------------

int CPaletteManager::SaveCustom (CFileManager& fp)
{
return fp.Write (m_custom, 37 * 256, 1) == 1;
}

//------------------------------------------------------------------------

byte* CPaletteManager::LoadDefault (void)
{
CResource res;
if (!res.Load (PaletteResource ()))
	return null;
m_default = new byte [res.Size()];
memcpy (m_default, res.Data (), res.Size ());
SetupBMI ();
return m_default;
}

//------------------------------------------------------------------------

byte* CPaletteManager::Current (void)
{
if (m_custom)
	return m_custom;
if (m_default)
	return m_default;
return LoadDefault ();
}

//------------------------------------------------------------------------
// PaletteResource()
//
// Action - returns the name of the palette resource.  Neat part about
//          this function is that the strings are automatically stored
//          in the local heap so the string is static.
//
//------------------------------------------------------------------------

const char* CPaletteManager::Resource (void) 
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
CFileManager::SplitPath (descent2_path, null, szFile, null);
for (ppe = palExt; *(ppe->szFile); ppe++)
	if (!_stricmp (ppe->szFile, szFile))
		return MAKEINTRESOURCE (ppe->nIdPal);
return MAKEINTRESOURCE (IDR_GROUPA_256);
}

//------------------------------------------------------------------------
