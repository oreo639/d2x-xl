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
	uint				offset, hdrOffset, bmpOffset, hdrSize, xlatSize;
	ushort			nCustomTextures, nUnknownTextures, nMissingTextures;
	CTexture*		pTexture;
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

	// get texture data offset from texture header
	fp.Seek (hdrOffset + i * sizeof (PIG_TEXTURE_D2), SEEK_SET);
	pigTexInfo.Read (&fp);
	if ((long) (hdrSize + pigTexInfo.offset) >= nFileSize) {
		nMissingTextures++;
		continue;
		}
	
	fp.Seek (bmpOffset + pigTexInfo.offset, SEEK_SET);
	pTexture = OverrideTexture (nIndex - 1, null, 1);
	pTexture->LoadFromPog (fp, pigTexInfo);
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
// Textures shouldn't be marked modified on first load. Assuming here that POGs are only read while
// opening a level, otherwise we'll have to unmark each texture individually
CommitTextureChanges ();

DLE.MainFrame ()->Progress ().DestroyWindow ();

if (xlatTbl)
	delete xlatTbl;
return 0;
}

//-----------------------------------------------------------------------------------

uint CTextureManager::WriteCustomTextureHeader (CFileManager& fp, const CTexture *pTexture, uint nId, uint nOffset)
{
	CPigTexture pigTexInfo (1);
	ubyte *palIndex, *pSrc;

memset (pigTexInfo.name, 0, sizeof (pigTexInfo.name));
if (*pTexture->Name ())
	// Texture name in pigTexInfo is not null-terminated
	memcpy_s (pigTexInfo.name, sizeof (pigTexInfo.name), pTexture->Name (), strlen (pTexture->Name ()));
else {
	char name [9];
	sprintf_s (name, sizeof (name), "POG%04d", nId);
	memcpy (pigTexInfo.name, name, sizeof (pigTexInfo.name));
	}
#if 1
pigTexInfo.Setup (1, pTexture->Width (), pTexture->Height (), (pTexture->Format () == TGA) ? 0x80 : 0, nOffset);
#else
pigTexInfo.dflags = 0;
pigTexInfo.flags = pTexture->m_info.nFormat ? 0x80 : 0;
pigTexInfo.width =  % 256;
if ((pigTexInfo.flags & 0x80) && (pTexture->m_info.width > 256)) {
	pigTexInfo.whExtra = (pTexture->m_info.width >> 8);
	pigTexInfo.height = pTexture->m_info.height / pTexture->m_info.width;
	}
else {
	pigTexInfo.height = pTexture->m_info.height % 256;
	pigTexInfo.whExtra = (pTexture->m_info.width >> 8) | ((pTexture->m_info.height >> 4) & 0xF0);
	}
pigTexInfo.avgColor = 0;
pigTexInfo.offset = nOffset;
#endif

// check for transparency and super transparency
if (pTexture->Format () == BMP)
	try {
	if (palIndex = pSrc = pTexture->ToBitmap ()) {
		for (uint j = 0, h = pTexture->Size (); j < h; j++, pSrc++) {
			if (*pSrc == 255) 
				pigTexInfo.flags |= BM_FLAG_TRANSPARENT;
			if (*pSrc == 254) 
				pigTexInfo.flags |= BM_FLAG_SUPER_TRANSPARENT;
			if ((pigTexInfo.flags & (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT)) == (BM_FLAG_TRANSPARENT | BM_FLAG_SUPER_TRANSPARENT))
				break;
			}
		delete [] palIndex;
		}
	}
	catch(...) {
#if DBG_ARRAYS
		ArrayError ("invalid buffer size\n");
#endif
		}
pigTexInfo.Write (&fp);
return nOffset + ((pTexture->Format () == TGA) ? pTexture->Size () * sizeof (CBGRA) : pTexture->Size ());
}

//-----------------------------------------------------------------------------------

int CTextureManager::WriteCustomTexture (CFileManager& fp, const CTexture *pTexture)
{
if (pTexture->Format () == TGA && DLE.IsD2XLevel ()) {
	tRGBA rgba [16384];
	uint h = 0;
	const CBGRA* pBuffer = pTexture->Buffer (pTexture->Width () * (pTexture->Height () - 1));
	for (uint i = pTexture->Height (); i; i--) {
		for (uint j = pTexture->Width (); j; j--, pBuffer++) {
			rgba [h].r = pBuffer->r;
			rgba [h].g = pBuffer->g;
			rgba [h].b = pBuffer->b;
			rgba [h].a = pBuffer->a;
			if (++h == sizeofa (rgba)) {
				fp.Write (rgba, sizeof (tRGBA), h);
				h = 0;
				}
			}
		pBuffer -= 2 * pTexture->Width ();
		}
	if (h > 0)
		fp.Write (rgba, sizeof (tRGBA), h);
	}
else {
	ubyte* palIndex = pTexture->ToBitmap ();
	fp.Write (palIndex, 1, pTexture->Size ());
	delete [] palIndex;
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
	const CTexture*	pTexture;

if (DLE.IsD1File ()) {
	ErrorMsg ("Descent 1 does not support custom textures.");
	return 0;
	}

paletteManager.ResetCLUT ();
textureCount = m_nTextures [1];

sprintf_s (message, sizeof (message), "%s\\dle_temp.pog", DLE.AppFolder ());

// write file header
pigFileInfo.nId = 0x474f5044L; /* 'DPOG' */
pigFileInfo.nVersion = 1;
pigFileInfo.nTextures = 0;

for (i = 0; i < h; i++) {
	pTexture = TextureByIndex (i);
	if (pTexture->IsCustom ())
		pigFileInfo.nTextures++;
	}
pigFileInfo.Write (fp);

// write list of textures
for (i = 0; i < h; i++) {
	pTexture = TextureByIndex (i);
	if (pTexture->IsCustom ())
		fp.WriteUInt16 (ushort (i + 1));
	}

// write texture headers
nId = 0;
for (i = 0; i < h; i++) {
	pTexture = TextureByIndex (i);
	if (pTexture->IsCustom ())
		nOffset = WriteCustomTextureHeader (fp, pTexture, nId++, nOffset);
	}

sprintf_s (message, sizeof (message)," Pog manager: Saving %d custom textures", pigFileInfo.nTextures);
DEBUGMSG (message);

for (i = 0; i < h; i++) {
	pTexture = TextureByIndex (i);
	if (pTexture->IsCustom ())
		WriteCustomTexture (fp, pTexture);
	}

return 1;
}

//------------------------------------------------------------------------------

void CTextureManager::ReadMod (char* pszFolder)
{
	char szFile [256];
	int  h = MaxTextures ();

for (int i = 0; i < h; i++) {
	DLE.MainFrame ()->Progress ().StepIt ();
	const CTexture* pTexture = Textures (i);
#ifdef _DEBUG
	if (i == nDbgTexture)
		nDbgTexture = nDbgTexture;
#endif
	if (pTexture->IsCustom ())
		continue;
	sprintf (szFile, "%s\\%s%s.tga", pszFolder, pTexture->Name (), pTexture->IsAnimated () ? "#0" : "");
	CTexture *pNewTexture = new CTexture ();
	if (!pNewTexture)
		ErrorMsg ("Not enough memory for texture.");
	else if (pNewTexture->Copy (*pTexture) && pNewTexture->LoadFromFile (szFile, false)) {
		OverrideTexture (pTexture->Index (), pNewTexture);
		pNewTexture->SetCustom ();
		}
	else
		delete pNewTexture;
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