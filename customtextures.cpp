#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "cfile.h"
#include "dle-xp-res.h"
#include "global.h"
#include "palette.h"
#include "customtextures.h"
#include "texturemanager.h"
#include "dle-xp.h"

//------------------------------------------------------------------------

inline int Sqr (int i)
{
return i * i;
}


inline uint ColorDelta (int r, int g, int b, PALETTEENTRY* palette)
{
return (uint) (Sqr (r - (int) palette->peRed) + Sqr (g - (int) palette->peGreen) + Sqr (b - (int) palette->peBlue));
}

//------------------------------------------------------------------------

inline byte ClosestColor (int r, int g, int b, PALETTEENTRY* palette)
{
	uint	i, delta, closestDelta = 0x7fffffff, closestIndex = 0;

for (i = 0; (i < 256) && closestDelta; i++) {
	delta = ColorDelta (r, g, b, palette++);
	if (delta < closestDelta) {
		if (delta == 0)
			return i;
		closestIndex = i;
		closestDelta = delta;
		}
	}
return (byte) closestIndex;
}

//------------------------------------------------------------------------

bool TGA2Bitmap (tRGBA *pTGA, byte *bmP, int nWidth, int nHeight)
{
	tRGBA			rgba = {0,0,0,0};
	byte			*bConverted = null;

int nSize = nWidth * nHeight;	//only convert the 1st frame of animated TGAs
int h = nSize, i = 0, k, x, y;

for (i = y = 0, k = nSize; y < nHeight; y++, i += nWidth) {
	k -= nWidth;
#pragma omp parallel 
{
#	pragma omp for private (x)
	for (x = 0; x < nWidth; x++) {
		rgba = pTGA [i + x];
		bmP [k + x] = ClosestColor ((int) rgba.r, (int) rgba.g, (int) rgba.b, paletteManager.ColorMap ()); //paletteManager.Current ());
		}
	}
}
return true;
}

//------------------------------------------------------------------------------

int RLEExpand (CPigTexture& pigTexInfo, byte *texBuf)
{
#	define RLE_CODE			0xE0
#	define NOT_RLE_CODE		31
#	define IS_RLE_CODE(x)	(((x) & RLE_CODE) == RLE_CODE)


	byte	*expandBuf, *pSrc, *pDest;
	byte	c, h;
	int	i, j, l, bBigRLE;
	ushort nLineSize;

int bufSize = pigTexInfo.BufSize ();
if (!(pigTexInfo.flags & BM_FLAG_RLE))
	return (int) bufSize;
if (!(expandBuf = (byte *) malloc (bufSize)))
	return -1;

bBigRLE = (pigTexInfo.flags & BM_FLAG_RLE_BIG) != 0;
if (bBigRLE)
	pSrc = texBuf + 4 + 2 * pigTexInfo.height;
else
	pSrc = texBuf + 4 + pigTexInfo.height;
pDest = expandBuf;
for (i = 0; i < pigTexInfo.height; i++, pSrc += nLineSize) {
	if (bBigRLE)
		nLineSize = *((ushort *) (texBuf + 4 + 2 * i));
	else
		nLineSize = texBuf [4 + i];
	for (j = 0; j < nLineSize; j++) {
		h = pSrc [j];
		if (!IS_RLE_CODE (h)) {
			c = h; // translate
			if (pDest - expandBuf >= bufSize) {
				free (expandBuf);
				return -1;
				}	
			*pDest++ = c;
			}
		else if (l = (h & NOT_RLE_CODE)) {
			c = pSrc [++j];
			if ((pDest - expandBuf) + l > bufSize) {
				free (expandBuf);
				return -1;
				}	
			memset (pDest, c, l);
			pDest += l;
			}
		}
	}
l = (int) (pDest - expandBuf);
memcpy (texBuf, expandBuf, l);
pigTexInfo.flags &= ~(BM_FLAG_RLE | BM_FLAG_RLE_BIG);
free (expandBuf);
return l;
}

//-----------------------------------------------------------------------------
// ReadPog ()
//-----------------------------------------------------------------------------

int ReadPog (CFileManager& fp, uint nFileSize) 
{
	CPigHeader		pigFileInfo (1);
	CPigTexture		pigTexInfo (1);

	uint*				textureCount = 0;
	ushort*			xlatTbl = null;
	uint				nSize;
	uint				offset, hdrOffset, bmpOffset, hdrSize, xlatSize;
	short				nTexture;
	int				row;
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
//textureManager.Release ();
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
	for (nTexture = 0; nTexture < MAX_D2_TEXTURES; nTexture++)
		if (textureManager.m_index [1][nTexture] == nIndex)
			break;
	bExtraTexture = (nTexture >= MAX_D2_TEXTURES);
	// get texture data offset from texture header
	fp.Seek (hdrOffset + i * sizeof (PIG_TEXTURE_D2), SEEK_SET);
	pigTexInfo.Read (fp);
	nSize = (uint) pigTexInfo.width * (uint) pigTexInfo.height;
	if (hdrSize + pigTexInfo.offset + nSize >= nFileSize) {
		nMissingTextures++;
		continue;
		}
	if (bExtraTexture) {
			texP = textureManager.AddExtra (nIndex);
		if (!texP) {
			nUnknownTextures++;
			continue;
			}
		nTexture = 0;
		}
	else {
		texP = textureManager.Textures (1, nTexture);
		texP->Release ();
		}
// allocate memory for texture if not already
	if (!(texP->m_info.bmDataP = new byte [nSize]))
		continue;
	if (pigTexInfo.flags & 0x80) {
		if (!(texP->m_info.tgaDataP = new tRGBA [nSize])) {
			texP->Release ();
			continue;
			}
		texP->m_info.nFormat = 1;
		}
	else
		texP->m_info.nFormat = 0;
	texP->m_info.width = pigTexInfo.width;
	texP->m_info.height = pigTexInfo.height;
	texP->m_info.size = nSize;
	texP->m_info.bValid = 1;
	// read texture into memory (assume non-compressed)
	fp.Seek (bmpOffset + pigTexInfo.offset, SEEK_SET);
	if (texP->m_info.nFormat) {
		fp.Read (texP->m_info.tgaDataP, sizeof (tRGBA), texP->m_info.size);
		texP->m_info.bValid = TGA2Bitmap (texP->m_info.tgaDataP, texP->m_info.bmDataP, (int) pigTexInfo.width, (int) pigTexInfo.height);
		}
	else {
		if (pigTexInfo.flags & BM_FLAG_RLE) {
			fp.Read (texP->m_info.bmDataP, nSize, 1);
			RLEExpand (pigTexInfo, texP->m_info.bmDataP);
			}
		else {
			byte *bufP = texP->m_info.bmDataP + pigTexInfo.width * (pigTexInfo.height - 1); // point to last row of bitmap
			for (row = 0; row < pigTexInfo.height; row++) {
				fp.Read (bufP, pigTexInfo.width, 1);
				bufP -= pigTexInfo.width;
				}
			}
		}
	if (!bExtraTexture)
		texP->m_info.bCustom = TRUE;
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

//-----------------------------------------------------------------------------

uint WritePogTextureHeader (CFileManager& fp, CTexture *texP, int nTexture, uint nOffset)
{
	CPigTexture pigTexInfo (1);
	byte *pSrc;

sprintf_s (pigTexInfo.name, sizeof (pigTexInfo.name), "POG%04d", nTexture);
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

nOffset += texP->m_info.nFormat ? texP->m_info.size * 4 : texP->m_info.size;

// check for transparency and super transparency
if (!texP->m_info.nFormat)
	if (pSrc = (byte *) texP->m_info.bmDataP) {
		for (uint j = 0; j < texP->m_info.size; j++, pSrc++) {
			if (*pSrc == 255) 
				pigTexInfo.flags |= BM_FLAG_TRANSPARENT;
			if (*pSrc == 254) 
				pigTexInfo.flags |= BM_FLAG_SUPER_TRANSPARENT;
			}
	}
pigTexInfo.Write (fp);
return nOffset;
}

//-----------------------------------------------------------------------------

bool WritePogTexture (CFileManager& fp, CTexture *texP)
{
	byte	*bufP = texP->m_info.nFormat ? (byte*) texP->m_info.tgaDataP : texP->m_info.bmDataP;

if (texP->m_info.nFormat) {
	fp.Write (bufP, sizeof (tRGBA), texP->m_info.size);
	}
else {
	ushort w = texP->m_info.width;
	ushort h = texP->m_info.height;
	bufP += w * (h - 1); // point to last row of bitmap
	for (int row = 0; row < h; row++) {
		fp.Write (bufP, 1, w);
		bufP -= w;
		}
	}
return true;
}

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

int CreatePog (CFileManager& fp) 
{
	CPigHeader		pigFileInfo (1);
	uint				textureCount = 0, nOffset = 0;
	int				nVersion = DLE.FileType ();
	int				nExtra, i, h = textureManager.MaxTextures (nVersion);
	CExtraTexture*	extraTexP;
	CTexture*		texP;

if (DLE.IsD1File ()) {
	ErrorMsg ("Descent 1 does not support custom textures.");
	return 1;
	}

textureCount = textureManager.nTextures [1];

sprintf_s (message, sizeof (message),"%s\\dle_temp.pog",m_startFolder );

// write file  header
pigFileInfo.nId = 0x474f5044L; /* 'DPOG' */
pigFileInfo.nVersion = 1;
pigFileInfo.nTextures = 0;
for (i = 0, texP = textureManager.Textures (nVersion); i < h; i++, texP++)
	if (texP->m_info.bCustom)
		pigFileInfo.nTextures++;
for (extraTexP = extraTextures; extraTexP; extraTexP = extraTexP->m_next)
	pigFileInfo.nTextures++;
pigFileInfo.Write (fp);

// write list of textures
for (i = 0, texP = textureManager.Textures (nVersion); i < h; i++, texP++)
	if (texP->m_info.bCustom)
		fp.Write (textureManager.m_index [1][i]);

for (extraTexP = extraTextures; extraTexP; extraTexP = extraTexP->m_next)
	fp.Write (extraTexP->m_index);

// write texture headers
nExtra = 0;
for (i = 0, texP = textureManager.Textures (nVersion); i < h; i++, texP++)
	if (texP->m_info.bCustom)
		nOffset = WritePogTextureHeader (fp, texP, nExtra++, nOffset);
for (extraTexP = extraTextures; extraTexP; extraTexP = extraTexP->m_next)
	nOffset = WritePogTextureHeader (fp, extraTexP, nExtra++, nOffset);

sprintf_s (message, sizeof (message)," Pog manager: Saving %d custom textures", pigFileInfo.nTextures);
DEBUGMSG (message);

for (i = 0, texP = textureManager.Textures (nVersion); i < h; i++, texP++)
	if (texP->m_info.bCustom)
		WritePogTexture (fp, texP);
for (extraTexP = extraTextures; extraTexP; extraTexP = extraTexP->m_next)
	WritePogTexture (fp, extraTexP);

return 0;
}

//------------------------------------------------------------------------

//eof textures.cpp