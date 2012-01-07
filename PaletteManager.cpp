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

void CPaletteManager::SetupBMI (CBGR* palette) 
{
m_bmi.header.biSize = sizeof (BITMAPINFOHEADER);
m_bmi.header.biWidth = 64;
m_bmi.header.biHeight = 64;
m_bmi.header.biPlanes = 1;
m_bmi.header.biBitCount = 8;
m_bmi.header.biCompression = BI_RGB;
m_bmi.header.biSizeImage = 0;
m_bmi.header.biXPelsPerMeter = 0;
m_bmi.header.biYPelsPerMeter = 0;
m_bmi.header.biClrUsed = 256;
m_bmi.header.biClrImportant = 0;
uint* rgb = (uint*) &m_bmi.colors [0];
for (int i = 0; i < 256; i++) {
	m_bmi.colors [i].rgbRed = palette [i].r;
	m_bmi.colors [i].rgbGreen = palette [i].g;
	m_bmi.colors [i].rgbBlue = palette [i].b;
	}
}

//------------------------------------------------------------------------

inline byte CPaletteManager::FadeValue (byte c, int f)
{
return (byte) (((int) c * f) / 34);
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

void CPaletteManager::Decode (CBGR* dest)
{
for (int i = 0, j = 0; i < 256; i++) {
	dest [i].r = m_rawData [j++] * 4;
	dest [i].g = m_rawData [j++] * 4;
	dest [i].b = m_rawData [j++] * 4;
	}
m_superTransp = dest [254];
}

//------------------------------------------------------------------------

void CPaletteManager::Encode (CBGR* src)
{
for (int i = 0, j = 0; i < 256; i++) {
	m_rawData [j++] = src [i].r / 4;
	m_rawData [j++] = src [i].g / 4;
	m_rawData [j++] = src [i].b / 4;
	}
}

//------------------------------------------------------------------------

void CPaletteManager::CreateFadeTable (void)
{
for (int i = 0; i < 256; i++) {
	byte c = m_rawData [i];
	for (int j = 0; j < 34; j++)
		m_fadeTables [j * 256 + i] = FadeValue (c, j + 1);
	}
}

//------------------------------------------------------------------------

int CPaletteManager::LoadCustom (CFileManager* fp, long size)
{
if (fp->Read (m_rawData, 1, sizeof (m_rawData)) != sizeof (m_rawData))
	return 0;
CreateFadeTable ();
Decode (m_custom);
m_bHaveCustom = true;
//SetupRender (m_custom);
//SetupBMI (m_custom);
return 1;
}

//------------------------------------------------------------------------

int CPaletteManager::SaveCustom (CFileManager* fp)
{
return fp->Write (m_rawData, 1, sizeof (m_rawData)) == sizeof (m_rawData);
}

//------------------------------------------------------------------------

short CPaletteManager::SetupRender (CBGR* palette)
{
FreeRender ();
if (!(m_dlcLog = (LPLOGPALETTE) new byte [sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * 256]))
	return 1;
m_dlcLog->palVersion = 0x300;
m_dlcLog->palNumEntries = 256;
PALETTEENTRY* rgb = &m_dlcLog->palPalEntry [0];
for (int i = 0; i < 256; i++) {
	rgb [i].peRed = palette [i].r;
	rgb [i].peGreen = palette [i].g;
	rgb [i].peBlue = palette [i].b;
	}
if (!(m_render = new CPalette ()))
	return 1;
m_render->CreatePalette (m_dlcLog);
m_colorMap = new PALETTEENTRY [256];
m_render->GetPaletteEntries (0, 256, m_colorMap);
return m_render == null;
return 1;
}

//------------------------------------------------------------------------

CBGR* CPaletteManager::LoadDefault (void)
{
CResource res;
if (!res.Load (SelectResource ()))
	return null;
memcpy (m_rawData, res.Data (), sizeof (m_rawData));
Decode (m_default);
CreateFadeTable ();
SetupRender (m_default);
SetupBMI (m_default);
m_bHaveDefault = true;
return m_default;
}

//------------------------------------------------------------------------

CBGR* CPaletteManager::Current (int i)
{
if (m_bHaveCustom)
	return m_custom + i;
if (m_bHaveDefault)
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

const char* CPaletteManager::SelectResource (void) 
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

if (theMine && DLE.IsD1File ())
	return MAKEINTRESOURCE (IDR_PALETTE_256);
CFileManager::SplitPath (descentPath [1], null, szFile, null);
for (ppe = palExt; *(ppe->szFile); ppe++)
	if (!_stricmp (ppe->szFile, szFile))
		return MAKEINTRESOURCE (ppe->nIdPal);
return MAKEINTRESOURCE (IDR_GROUPA_256);
}

//------------------------------------------------------------------------

void CPaletteManager::LoadName (CFileManager* fp)
{
int i;
for (i = 0; i < 15; i++) {
	m_name [i] = fp->ReadChar ();
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

//------------------------------------------------------------------------------

byte CPaletteManager::ClosestColor (CBGR& color)
{
	CBGR* palette = Current ();
	uint delta, closestDelta = 0x7fffffff;
	byte closestIndex = 0;

for (int i = 0; i < 256; i++) {
	delta = color.Delta (palette[i]);
	if (delta < closestDelta) {
		if (delta == 0)
			return i;
		closestIndex = i;
		closestDelta = delta;
		}
	}
return closestIndex;
}

//------------------------------------------------------------------------------

