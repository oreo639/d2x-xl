#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "FileManager.h"
#include "dle-xp-res.h"
#include "global.h"
#include "PaletteManager.h"
#include "TextureManager.h"
#include "dle-xp.h"

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
	short				nTexture;
	ushort			nUnknownTextures, nMissingTextures;
	bool				bExtraTexture;
	CTexture*		texP;
	int				fileType = DLE.FileType ();

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
nUnknownTextures = 0;
nMissingTextures = 0;
hdrOffset = offset + xlatSize;
hdrSize = xlatSize + pigFileInfo.nTextures * sizeof (PIG_TEXTURE_D2);
bmpOffset = offset + hdrSize;

DLE.MainFrame ()->InitProgress (pigFileInfo.nTextures);

for (int i = 0; i < pigFileInfo.nTextures; i++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	// read texture index
	ushort nIndex = xlatTbl [i];
	// look it up in the list of textures
	for (nTexture = 0; nTexture < MAX_TEXTURES_D2; nTexture++)
		if (m_index [1][nTexture] == nIndex)
			break;
	bExtraTexture = (nTexture >= MAX_TEXTURES_D2);
	// get texture data offset from texture header
	fp.Seek (hdrOffset + i * sizeof (PIG_TEXTURE_D2), SEEK_SET);
	pigTexInfo.Read (&fp);
	nSize = (uint) pigTexInfo.width * (uint) pigTexInfo.height;
	if ((long) (hdrSize + pigTexInfo.offset + nSize) >= nFileSize) {
		nMissingTextures++;
		continue;
		}
	if (bExtraTexture) {
			texP = AddExtra (nIndex);
		if (!texP) {
			nUnknownTextures++;
			continue;
			}
		nTexture = 0;
		}
	else {
		texP = Textures (1, nTexture);
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
	if (!bExtraTexture)
		texP->m_info.bCustom = true;
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
	byte *srcP;
	uint pos = 0xFFFFFFFF;

if (nId >= 0) {
	texP->m_info.id = nId;
	texP->m_info.offset = nOffset;
	}
else {
	pos = fp.Tell ();
	nId = texP->m_info.id;
	fp.Seek (nOffset = texP->m_info.offset);
	}

char name [9];
sprintf_s (name, sizeof (name), "POG%04d", nId);
memcpy (pigTexInfo.name, name, sizeof (pigTexInfo.name));
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
	if (srcP = (byte *) texP->Buffer ()) {
		for (uint j = 0, h = texP->Size (); j < h; j++, srcP++) {
			if (*srcP == 255) 
				pigTexInfo.flags |= BM_FLAG_TRANSPARENT;
			if (*srcP == 254) 
				pigTexInfo.flags |= BM_FLAG_SUPER_TRANSPARENT;
			}
	}
pigTexInfo.Write (&fp);
if (pos != 0xFFFFFFFF)
	fp.Seek (pos);
return nOffset + (texP->m_info.nFormat ? texP->Size () * sizeof (CBGRA) : texP->Size ());
}

//-----------------------------------------------------------------------------------

int CTextureManager::WriteCustomTexture (CFileManager& fp, CTexture *texP)
{
if (texP->m_info.nFormat) {
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
	ushort w = texP->m_info.width;
	ushort h = texP->m_info.height;
	byte* bmIndex = new byte [w * h];
	if (bmIndex == null) { // write as TGA
		texP->m_info.nFormat = 1;
		return (WriteCustomTexture (fp, texP) == 0) ? 0 : -1;
		}
	texP->ComputeIndex (bmIndex);
	for (bmIndex += w * h /*point to last row of bitmap*/; h > 0; h--) {
		bmIndex -= w;
		fp.Write (bmIndex, 1, w);
		}
	delete bmIndex;
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
//   byte  data[dataSize];    // raw data
//
// Commpressed Texture data:
//   uint totalSize;      // including this long word and everything below
//   byte  lineSizes[64];  // size of each line (in bytes)
//   byte  data[dataSize]; // run length encoded (RLE) data
//
// RLE definition:
//
//   If upper 3 bits of byte are set, lower 5 bytes represents how many times
//   to repeat the following byte.  The last byte of the data must be set
//   to 0xE0, which means 0 bytes to follow.  Note: The last byte may be
//   omitted since the xsize array defines the length of each line???
//
//-----------------------------------------------------------------------------------

int CTextureManager::CreatePog (CFileManager& fp) 
{
	CPigHeader		pigFileInfo (1);
	uint				textureCount = 0, nOffset = 0;
	int				nVersion = DLE.FileType ();
	int				nId, i, h = MaxTextures (nVersion);
	CExtraTexture*	extraTexP;
	CTexture*		texP;

if (DLE.IsD1File ()) {
	ErrorMsg ("Descent 1 does not support custom textures.");
	return 1;
	}

paletteManager.ResetCLUT ();
textureCount = m_nTextures [1];

sprintf_s (message, sizeof (message), "%s\\dle_temp.pog", m_startFolder);

// write file  header
pigFileInfo.nId = 0x474f5044L; /* 'DPOG' */
pigFileInfo.nVersion = 1;
pigFileInfo.nTextures = 0;
for (i = 0, texP = Textures (nVersion); i < h; i++, texP++)
	if (texP->m_info.bCustom)
		pigFileInfo.nTextures++;
for (extraTexP = m_extra; extraTexP; extraTexP = extraTexP->m_next)
	pigFileInfo.nTextures++;
pigFileInfo.Write (fp);

// write list of textures
for (i = 0, texP = Textures (nVersion); i < h; i++, texP++)
	if (texP->m_info.bCustom)
		fp.Write (m_index [1][i]);

for (extraTexP = m_extra; extraTexP; extraTexP = extraTexP->m_next)
	fp.Write (extraTexP->m_index);

// write texture headers
nId = 0;
for (i = 0, texP = Textures (nVersion); i < h; i++, texP++)
	if (texP->m_info.bCustom)
		nOffset = WriteCustomTextureHeader (fp, texP, nId++, nOffset);
for (extraTexP = m_extra; extraTexP; extraTexP = extraTexP->m_next)
	nOffset = WriteCustomTextureHeader (fp, extraTexP, nId++, nOffset);

sprintf_s (message, sizeof (message)," Pog manager: Saving %d custom textures", pigFileInfo.nTextures);
DEBUGMSG (message);

for (i = 0, texP = Textures (nVersion); i < h; i++, texP++)
	if (texP->m_info.bCustom)
		if (0 > WriteCustomTexture (fp, texP))
			WriteCustomTextureHeader (fp, texP); // need to rewrite to reflect changed texture type in header data

for (extraTexP = m_extra; extraTexP; extraTexP = extraTexP->m_next)
	if (0 > WriteCustomTexture (fp, extraTexP))
		WriteCustomTextureHeader (fp, extraTexP); // need to rewrite to reflect changed texture type in header data

return 0;
}

//------------------------------------------------------------------------------

//eof texturemanager.custom.cpp