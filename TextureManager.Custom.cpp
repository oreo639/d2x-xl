#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "FileManager.h"
#include "dle-xp-res.h"
#include "global.h"
#include "PaletteManager.h"
#include "TextureManager.h"
#include "HogManager.h"
#include "dle-xp.h"

extern short nDbgTexture;

//-----------------------------------------------------------------------------------
// ReadPog ()
//-----------------------------------------------------------------------------------

int CTextureManager::ReadPog (CFileManager& fp, long nFileSize) 
{
	CPigHeader		pigFileInfo (1);
	CPigTexture		pigTexInfo (1);

	uint*				textureCount = 0;
	ushort*			xlatTbl = null;
	uint				nSize;
	uint				offset, hdrOffset, bmpOffset, hdrSize, xlatSize;
	ushort			nCustomTextures, nUnknownTextures, nMissingTextures;
	CTexture*		texP;
	int				fileType = DLE.FileType ();
#if EXTRA_TEXTURES
	short				nTexture;
	bool				bExtraTexture;
#endif
// make sure this is descent 2 fp
if (DLE.IsD1File ()) {
	INFOMSG (" Descent 1 does not support custom textures.");
	return 1;
	}

uint startOffset = fp.Tell ();
// read file header
pigFileInfo.Read (fp);
if (pigFileInfo.nId != 0x474f5044L) {  // 'DPOG'
	ErrorMsg ("Invalid pog file - reading from hog file");
	return 1;
	}
//Release ();
sprintf_s (message, sizeof (message), " Pog manager: Reading %d custom textures", pigFileInfo.nTextures);
DEBUGMSG (message);
if (!(xlatTbl = new ushort [pigFileInfo.nTextures]))
	return 5;
xlatSize = pigFileInfo.nTextures * sizeof (ushort);
offset = fp.Tell ();
fp.Read (xlatTbl, xlatSize, 1);
// loop for each custom texture
nCustomTextures = 0;
nUnknownTextures = 0;
nMissingTextures = 0;
hdrOffset = offset + xlatSize;
hdrSize = xlatSize + pigFileInfo.nTextures * sizeof (PIG_TEXTURE_D2);
bmpOffset = offset + hdrSize;

DLE.MainFrame ()->InitProgress (pigFileInfo.nTextures);

for (int i = 0; i < pigFileInfo.nTextures; i++) {
	DLE.MainFrame ()->Progress ().StepIt ();
#ifdef _DEBUG
	if (i == nDbgTexture)
		nDbgTexture = nDbgTexture;
#endif
	// read texture index
	ushort nIndex = xlatTbl [i];
#if EXTRA_TEXTURES
	// look it up in the list of textures
	ushort* index = &m_index [1][0];
	for (nTexture = 0; nTexture < MAX_TEXTURES_D2; nTexture++)
		if (index [nTexture] == nIndex)
			break;
	bExtraTexture = (nTexture >= MAX_TEXTURES_D2);
#endif
	// get texture data offset from texture header
	fp.Seek (hdrOffset + i * sizeof (PIG_TEXTURE_D2), SEEK_SET);
	pigTexInfo.Read (&fp);
	nSize = (uint) pigTexInfo.width * (uint) pigTexInfo.height;
	if ((long) (hdrSize + pigTexInfo.offset /*+ nSize*/) >= nFileSize) {
		nMissingTextures++;
		continue;
		}
#if EXTRA_TEXTURES
	if (bExtraTexture) {
		texP = AddExtra (nIndex);
		if (!texP) {
			nUnknownTextures++;
			continue;
			}
		nTexture = 0;
		}
	else 
#endif
		{
#if EXTRA_TEXTURES
		texP = Textures (1, nTexture);
#else
		texP = Textures (1, -nIndex);
#endif
		texP->Release ();
		}
	//if (!(texP->m_data = new CBGRA [nSize]))
	//	continue;
	texP->m_info.nFormat = (pigTexInfo.flags & 0x80) != 0;
	//texP->m_info.width = pigTexInfo.width;
	//texP->m_info.height = pigTexInfo.height;
	if (!texP->Allocate (nSize)) 
		continue;
	//texP->m_info.size = nSize;
	//texP->m_info.bValid = 1;
	fp.Seek (bmpOffset + pigTexInfo.offset, SEEK_SET);
	texP->Load (fp, pigTexInfo);
#if EXTRA_TEXTURES
	if (!bExtraTexture)
#endif
		texP->m_info.bCustom = 1;
		nCustomTextures++;
	}
if (nUnknownTextures) {
	sprintf_s (message, sizeof (message), " Pog manager: %d unknown textures found.", nUnknownTextures);
	DEBUGMSG (message);
	}
if (nMissingTextures) {
	sprintf_s (message, sizeof (message), " Pog manager: %d textures missing (Pog file probably damaged).", nMissingTextures);
	DEBUGMSG (message);
	}

DLE.MainFrame ()->Progress ().DestroyWindow ();

if (xlatTbl)
	delete xlatTbl;
return 0;
}

//-----------------------------------------------------------------------------------

uint CTextureManager::WriteCustomTextureHeader (CFileManager& fp, CTexture *texP, int nId, uint nOffset)
{
	CPigTexture pigTexInfo (1);
	ubyte *srcP;
	uint pos = 0xFFFFFFFF;

if (nId >= 0) {
	texP->m_info.nId = nId;
	texP->m_info.offset = nOffset;
	}
else {
	pos = fp.Tell ();
	nId = texP->m_info.nId;
	fp.Seek (nOffset = texP->m_info.offset);
	}

if (*texP->m_info.szName)
	strncpy_s (pigTexInfo.name, texP->m_info.szName, sizeof (pigTexInfo.name));
else {
	char name [9];
	sprintf_s (name, sizeof (name), "POG%04d", nId);
	memcpy (pigTexInfo.name, name, sizeof (pigTexInfo.name));
	}
#if 1
pigTexInfo.Setup (1, texP->m_info.width, texP->m_info.height, texP->m_info.nFormat ? 0x80 : 0, nOffset);
#else
pigTexInfo.dflags = 0;
pigTexInfo.flags = texP->m_info.nFormat ? 0x80 : 0;
pigTexInfo.width =  % 256;
if ((pigTexInfo.flags & 0x80) && (texP->m_info.width > 256)) {
	pigTexInfo.whExtra = (texP->m_info.width >> 8);
	pigTexInfo.height = texP->m_info.height / texP->m_info.width;
	}
else {
	pigTexInfo.height = texP->m_info.height % 256;
	pigTexInfo.whExtra = (texP->m_info.width >> 8) | ((texP->m_info.height >> 4) & 0xF0);
	}
pigTexInfo.avgColor = 0;
pigTexInfo.offset = nOffset;
#endif

// check for transparency and super transparency
if (!texP->m_info.nFormat)
	try {
	if (srcP = (ubyte *) texP->ToBitmap (false)) {
		for (uint j = 0, h = texP->Size (0); j < h; j++, srcP++) {
			if (*srcP == 255) 
				pigTexInfo.flags |= BM_FLAG_TRANSPARENT;
			if (*srcP == 254) 
				pigTexInfo.flags |= BM_FLAG_SUPER_TRANSPARENT;
			if ((pigTexInfo.flags & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT)) == (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT))
				break;
			}
		}
	}
	catch(...) {
#if DBG_ARRAYS
		ArrayError ("invalid buffer size\n");
#endif
		}
pigTexInfo.Write (&fp);
if (pos != 0xFFFFFFFF)
	fp.Seek (pos);
return nOffset + (texP->m_info.nFormat ? texP->Size () * sizeof (CBGRA) : texP->Size (0));
}

//-----------------------------------------------------------------------------------

int CTextureManager::WriteCustomTexture (CFileManager& fp, CTexture *texP)
{
if (texP->m_info.nFormat && DLE.IsD2XLevel ()) {
	tRGBA rgba [16384];
	uint h = 0;
	CBGRA* bufP = texP->Buffer (texP->m_info.width * (texP->m_info.height - 1));
	for (uint i = texP->m_info.height; i; i--) {
		for (uint j = texP->m_info.width; j; j--, bufP++) {
			rgba [h].r = bufP->r;
			rgba [h].g = bufP->g;
			rgba [h].b = bufP->b;
			rgba [h].a = bufP->a;
			if (++h == sizeofa (rgba)) {
				fp.Write (rgba, sizeof (tRGBA), h);
				h = 0;
				}
			}
		bufP -= 2 * texP->m_info.width;
		}
	if (h > 0)
		fp.Write (rgba, sizeof (tRGBA), h);
	}
else {
	ubyte* palIndex = texP->ToBitmap (texP->m_info.nFormat != 0);
	if (palIndex == null) { // write as TGA
		if (DLE.IsD2XLevel ())
			return -1;
		texP->m_info.nFormat = 1;
		return (WriteCustomTexture (fp, texP) == 0) ? 0 : -1;
		}
	texP->m_info.nFormat = 0;
	fp.Write (palIndex, 1, texP->Size (0));
	delete palIndex;
	}
return 1;
}

//-----------------------------------------------------------------------------------
// CreatePog ()
//
// Action - Creates a POG fp from all the changed textures
//
// Format:
//   Pig Header
//   Texture Indicies (N total * ushort)
//   Texture Header 0
//     ...
//   Texture Header N
//   Texture 0
//     ...
//   Texture N
//
// where N is the number of textures.
//
// Uncommpressed Texture data:
//   ubyte  data[dataSize];    // raw data
//
// Commpressed Texture data:
//   uint totalSize;      // including this long word and everything below
//   ubyte  lineSizes[64];  // size of each line (in bytes)
//   ubyte  data[dataSize]; // run length encoded (RLE) data
//
// RLE definition:
//
//   If upper 3 bits of ubyte are set, lower 5 bytes represents how many times
//   to repeat the following ubyte.  The last ubyte of the data must be set
//   to 0xE0, which means 0 bytes to follow.  Note: The last ubyte may be
//   omitted since the xsize array defines the length of each line???
//
//-----------------------------------------------------------------------------------

int CTextureManager::CreatePog (CFileManager& fp) 
{
if (!textureManager.Available ())
	return 1;

	CPigHeader		pigFileInfo (1);
	uint				textureCount = 0, nOffset = 0;
	int				nVersion = DLE.FileType ();
	int				nId, i, h = m_header [nVersion].nTextures;
	CTexture*		texP;
#if EXTRA_TEXTURES
	CExtraTexture*	extraTexP;
#endif

if (DLE.IsD1File ()) {
	ErrorMsg ("Descent 1 does not support custom textures.");
	return 0;
	}

paletteManager.ResetCLUT ();
textureCount = m_nTextures [1];

sprintf_s (message, sizeof (message), "%s\\dle_temp.pog", DLE.AppFolder ());

// write file  header
pigFileInfo.nId = 0x474f5044L; /* 'DPOG' */
pigFileInfo.nVersion = 1;
pigFileInfo.nTextures = 0;

for (i = 0; i < h; i++) {
	texP = Texture (-i - 1);
	if (texP->m_info.bCustom == 1)
		pigFileInfo.nTextures++;
	}
#if EXTRA_TEXTURES
for (extraTexP = m_extra; extraTexP; extraTexP = extraTexP->m_next)
	pigFileInfo.nTextures++;
#endif
pigFileInfo.Write (fp);

// write list of textures
for (i = 0; i < h; i++) {
	texP = Texture (-i - 1);
	if (texP->m_info.bCustom == 1)
#if EXTRA_TEXTURES
		fp.Write (m_index [1][i]);
#else
		fp.WriteUInt16 (ushort (i + 1));
#endif
	}
#if EXTRA_TEXTURES
for (extraTexP = m_extra; extraTexP; extraTexP = extraTexP->m_next)
	fp.Write (extraTexP->m_index);
#endif
// write texture headers
nId = 0;
for (i = 0; i < h; i++) {
	texP = Texture (-i - 1);
	if (texP->m_info.bCustom == 1)
		nOffset = WriteCustomTextureHeader (fp, texP, nId++, nOffset);
	}
#if EXTRA_TEXTURES
for (extraTexP = m_extra; extraTexP; extraTexP = extraTexP->m_next)
	nOffset = WriteCustomTextureHeader (fp, extraTexP, nId++, nOffset);
#endif
sprintf_s (message, sizeof (message)," Pog manager: Saving %d custom textures", pigFileInfo.nTextures);
DEBUGMSG (message);

for (i = 0; i < h; i++) {
	texP = Texture (-i - 1);
	if (texP->m_info.bCustom == 1)
		if (0 > WriteCustomTexture (fp, texP))
			WriteCustomTextureHeader (fp, texP); // need to rewrite to reflect changed texture type in header data
	}
#if EXTRA_TEXTURES
for (extraTexP = m_extra; extraTexP; extraTexP = extraTexP->m_next)
	if (0 > WriteCustomTexture (fp, extraTexP))
		WriteCustomTextureHeader (fp, extraTexP); // need to rewrite to reflect changed texture type in header data
#endif
return 1;
}

//------------------------------------------------------------------------------

void CTextureManager::ReadMod (char* pszFolder)
{
	char szFile [256];
	int  h = MaxTextures ();

for (int i = 0; i < h; i++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	CTexture* texP = Texture (i);
#ifdef _DEBUG
	if (i == nDbgTexture)
		nDbgTexture = nDbgTexture;
#endif
	if (texP->m_info.bCustom == 1)
		continue;
	sprintf (szFile, "%s\\%s.tga", pszFolder, texP->m_info.szName);
	texP->m_info.bCustom = texP->LoadTGA (szFile) ? 2 : 0;
	}
}

//------------------------------------------------------------------------------

void CTextureManager::LoadMod (void)
{
if (DLE.MakeModFolders ("textures")) {
	DLE.MainFrame ()->InitProgress (2 * MaxTextures ());
	// first read the level specific textures
	ReadMod (DLE.m_modFolders [0]);
	// then read the mission wide textures
	ReadMod (DLE.m_modFolders [1]);
	DLE.MainFrame ()->Progress ().DestroyWindow ();
	}
}

//------------------------------------------------------------------------------
//eof texturemanager.custom.cpp