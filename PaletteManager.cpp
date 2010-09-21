// palette.cpp

#include "stdafx.h"
#include "dle-xp-res.h"

#include <string.h>

#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "FileManager.h"
#include "PaletteManager.h"

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

void CPaletteManager::SetupBMI (COLORREF* palette) 
{
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
uint* rgb = (uint*) &m_bmi.colors [0];
for (int i = 0; i < 256; i++, palette += 3)
	rgb [i] = ((uint) *palette);
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
if (m_fadeTable) {
	delete m_fadeTable;
	m_fadeTable = null;
	}
}

//------------------------------------------------------------------------

void CPaletteManager::FreeDefault (void)
{
if (m_default) {
	delete m_default;
	m_default = null;
	}
}

//------------------------------------------------------------------------

void CPaletteManager::FreeRender (void)
{
if (m_render) {
	delete m_render;
	m_render = null;
	}
if (m_dlcLog) {
	delete m_dlcLog;
	m_dlcLog = null;
	}
if (m_colorMap) {
	delete m_colorMap;
	m_colorMap = null;
	}
}

//------------------------------------------------------------------------

void CPaletteManager::Decode (COLORREF* dest, byte* src)
{
for (int i = 0, j = 0; i < 256; i++, j += 3)
	dest [i] = RGB (src [j] * 4, src [j + 1] * 4, src [j + 2] * 4);
}

//------------------------------------------------------------------------

void CPaletteManager::Encode (byte* dest, COLORREF* src)
{
for (int i = 0, j = 0; i < 256; i++) {
	dest [j++] = GetRValue (src [i]);
	dest [j++] = GetGValue (src [i]);
	dest [j++] = GetBValue (src [i]);
	}
}

//------------------------------------------------------------------------

int CPaletteManager::LoadCustom (CFileManager& fp, long size)
{
FreeCustom ();

m_custom = new COLORREF [256];
m_fadeTable = new byte [34 * 256];
byte* buffer = new byte [37 * 256];
if ((m_custom == null) || (m_fadeTable == null) || (buffer == null)) {
	FreeCustom ();
	return 0;
	}

int h = (int) fp.Read (buffer, 37 * 256, 1);
if (h == 37 * 256)
	return 1;

if (h != 3 * 256) {
	paletteManager.FreeCustom ();
	return 0;
	}

for (int i = 0; i < 256; i++) {
	byte c = buffer [i];
	for (int j = 0; j < 34; j++)
		m_fadeTable [j * 256 + i] = FadeValue (c, j + 1);
	}

Decode (m_custom, buffer);
//SetupRender (m_custom);
//SetupBMI (m_custom);
delete[] buffer;
return 1;
}

//------------------------------------------------------------------------

int CPaletteManager::SaveCustom (CFileManager& fp)
{
return fp.Write (m_custom, 37 * 256, 1) == 1;
}

//------------------------------------------------------------------------

short CPaletteManager::SetupRender (COLORREF* palette)
{
//FreeRender ();
//if (!(m_dlcLog = (LPLOGPALETTE) new byte [sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * 256]))
//	return 1;
//m_dlcLog->palVersion = 0x300;
//m_dlcLog->palNumEntries = 256;
//uint* rgb = (uint*) &m_dlcLog->palPalEntry [0];
//for (int i = 0; i < 256; i++, palette += 3)
//	rgb [i] = ((uint) (palette [0]) << 2) + ((uint) (palette [1]) << 10) + ((uint) (palette [2]) << 18);
//if (!(m_render = new CPalette ()))
//	return 1;
//m_render->CreatePalette (m_dlcLog);
//m_colorMap = new PALETTEENTRY [256];
//m_render->GetPaletteEntries (0, 256, m_colorMap);
//return m_render == null;
return 1;
}

//------------------------------------------------------------------------

COLORREF* CPaletteManager::LoadDefault (void)
{
CResource res;
if (!res.Load (Resource ()))
	return null;
m_default = new COLORREF [res.Size ()];
Decode (m_default, res.Data ());
SetupRender (m_default);
SetupBMI (m_default);
return m_default;
}

//------------------------------------------------------------------------

COLORREF* CPaletteManager::Current (int i)
{
if (m_custom)
	return m_custom + i;
if (m_default)
	return m_default + i;
LoadDefault ();
return (m_default == null) ? null : m_default + i;
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
if (theMine && DLE.IsD1File ())
	return MAKEINTRESOURCE (IDR_PALETTE_256);
CFileManager::SplitPath (descentPath [1], null, szFile, null);
for (ppe = palExt; *(ppe->szFile); ppe++)
	if (!_stricmp (ppe->szFile, szFile))
		return MAKEINTRESOURCE (ppe->nIdPal);
return MAKEINTRESOURCE (IDR_GROUPA_256);
}

//------------------------------------------------------------------------

void CPaletteManager::LoadName (CFileManager& fp)
{
int i;
for (i = 0; i < 15; i++) {
	m_name [i] = fp.ReadChar ();
	if (m_name [i] == 0x0a) {
		m_name [i] = 0;
		break;
		}
	}
// replace extension with .pig
if (i < 4) 
	strcpy_s (m_name, sizeof (m_name), "groupa.pig");
else {
	int l = (int) strlen (m_name) - 4;
	strcpy_s (m_name + l, sizeof (m_name) - l, ".pig");
	}
}

//------------------------------------------------------------------------
