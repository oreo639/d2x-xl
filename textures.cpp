#include "stdafx.h"
#include "textures.h"
#include "io.h"
#include "dle-xp-res.h"
#include "global.h"
#include "palette.h"
#include "dle-xp.h"

struct tExtraTexture;

typedef struct tExtraTexture	*pExtraTexture;

struct tExtraTexture {
	pExtraTexture	pNext;
	CTexture		texture;
	UINT16			texture_index;
} tExtraTexture;

pExtraTexture	extraTextures = NULL;

UINT8 bmBuf [512 * 512 * 32 * 4];

//------------------------------------------------------------------------

inline INT32 Sqr (INT32 i)
{
return i * i;
}


inline INT32 ColorDelta (unsigned char r, unsigned char g, unsigned char b, PALETTEENTRY *sysPal, INT32 j)
{
sysPal += j;
return 
	Sqr (r - sysPal->peRed) + 
	Sqr (g - sysPal->peGreen) + 
	Sqr (b - sysPal->peBlue);
}

//------------------------------------------------------------------------

inline UINT32 ClosestColor (unsigned char r, unsigned char g, unsigned char b, PALETTEENTRY *sysPal)
{
	UINT32	k, delta, closestDelta = 0x7fffffff, closestIndex = 0;

for (k = 0; (k < 256) && closestDelta; k++) {
	delta = ColorDelta (r, g, b, sysPal, k);
	if (delta < closestDelta) {
		closestIndex = k;
		closestDelta = delta;
		}
	}
return closestIndex;
}

//------------------------------------------------------------------------

bool TGA2Bitmap (tRGBA *pTGA, UINT8 *pBM, INT32 nWidth, INT32 nHeight)
{
	tRGBA			rgba = {0,0,0,0};
	UINT8			*bConverted = NULL;

PALETTEENTRY *sysPal = (PALETTEENTRY *) malloc (256 * sizeof (PALETTEENTRY));
if (!sysPal) {
	ErrorMsg ("Not enough memory for palette.");
	return false;
	}
theMine->m_currentPalette->GetPaletteEntries (0, 256, sysPal);

INT32 nSize = nWidth * nHeight;	//only convert the 1st frame of animated TGAs
INT32 h = nSize, i = 0, k, x, y;

#if 1
for (i = y = 0, k = nSize; y < nHeight; y++, i += nWidth) {
	k -= nWidth;
#pragma omp parallel 
{
#	pragma omp for private (x)
	for (x = 0; x < nWidth; x++) {
		rgba = pTGA [i + x];
		pBM [k + x] = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
		}
	}
}
#else
if (bConverted = (UINT8 *) malloc ((nSize + 7) / 8))
	memset (bConverted, 0, (nSize + 7) / 8);

nextPixel:

if (bConverted) {
	for (k = nSize; (i < h) && nSize; i++) {
		rgba = pTGA [i];
#	if 0
		if (bConverted [i >> 3] & (1 << (i & 7)))
			continue;
		closestIndex = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
#	else
//		CBRK ((rgba.r == 252) && (rgba.b == 252) && (rgba.b == 252));
#		if 0
		pBM [--k] = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
#		else
		k = (nHeight - (i / nHeight) - 1);
		k *= nWidth;
		k += i % nWidth;
		pBM [k] = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
//		pBM [(nHeight - (i / nHeight) - 1) * nWidth + i % nWidth] = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
#		endif
		continue;
#	endif
		for (j = i, pRGBA = pTGA + j; (j < h) && nSize; j++, pRGBA++) {
			if (bConverted [j >> 3] & (1 << (j & 7)))
				continue;
			if ((j == i) || ((pRGBA->r == rgba.r) && (pRGBA->g == rgba.g) && (pRGBA->b == rgba.b))) {
				pBM [j] = closestIndex;
				bConverted [j >> 3] |= (1 << (j & 7));
				nSize--;
				}
			}
		}
	}
else {
	for (; (i < h) && nSize; i++) {
		hRgba = rgba;
		rgba = pTGA [i];
		// look if this color already converted
		for (j = i - 1, pRGBA = pTGA + j; j >= 0; j--, pRGBA--)
			if ((pRGBA->r == rgba.r) && (pRGBA->g == rgba.g) && (pRGBA->b == rgba.b)) {
				i++;
				goto nextPixel;
				}
		closestIndex = ClosestColor (rgba.r, rgba.g, rgba.b, sysPal);
		for (j = i, pRGBA = pTGA + j; (j < h) && nSize; j++, pRGBA++) {
			if ((pRGBA->r == rgba.r) && (pRGBA->g == rgba.g) && (pRGBA->b == rgba.b)) {
				pBM [j] = closestIndex;
				nSize--;
				}
			}
		}
	}
if (bConverted)
	free (bConverted);
#endif
free (sysPal);
return true;
}

//------------------------------------------------------------------------
// DefineTexture ()
//
// ACTION - Defines data with texture.  If bitmap_handle != 0,
//          then data is copied from the global data.  Otherwise,
//          the pig file defines bmBuf and then global memory
//          is allocated and the data is copied to the global memory.
//          The next time that texture is used, the handle will be defined.
//------------------------------------------------------------------------

INT32 DefineTexture (INT16 nBaseTex,INT16 nOvlTex, CTexture *pDestTx, INT32 x0, INT32 y0) 
{
	typedef struct tFrac {
		INT32	c, d;
	} tFrac;

	UINT8			*ptr;
	INT16			textures [2], mode, w, h;
	INT32			i, x, y, y1, offs, s;
	tFrac			scale, scale2;
	INT32			rc; // return code
	CTexture*	texP [2];
	UINT8			*bmBuf = pDestTx->m_info.bmDataP;
	UINT8			c;
	INT32			fileType = theApp.FileType ();

	
	textures [0] = nBaseTex;
	textures [1] = nOvlTex & 0x3fff;
	mode     = nOvlTex & 0xC000;
	for (i = 0; i < 2; i++) {
#if 0	
	ASSERT (textures [i] < MAX_TEXTURES);
#endif
if ((textures [i] < 0) || (textures [i] >= MAX_TEXTURES))
	textures [i] = 0;
	// buffer textures if not already buffered
	texP [i] = theMine->Textures (fileType, textures [i]);
	if (!(texP [i]->m_info.bmDataP && texP [i]->m_info.bValid))
		if (rc = texP [i]->Read (textures [i]))
			return (rc);
	}
	
	// Define bmBuf based on texture numbers and rotation
pDestTx->m_info.width = texP [0]->m_info.width;
pDestTx->m_info.height = texP [0]->m_info.height;
pDestTx->m_info.size = texP [0]->m_info.size;
pDestTx->m_info.bValid = 1;
ptr = texP [0]->m_info.bmDataP;
//CBRK (textures [0] == 220);
if (ptr) {
	// if not rotated, then copy directly
	if (x0 == 0 && y0 == 0) 
		memcpy (bmBuf, ptr, texP [0]->m_info.size);
	else {
		// otherwise, copy bit by bit
		w = texP [0]->m_info.width;
#if 1
		INT32	l1 = y0 * w + x0;
		INT32	l2 = texP [0]->m_info.size - l1;
		memcpy (bmBuf, ptr + l1, l2);
		memcpy (bmBuf + l2, ptr, l1);
#else
		UINT8		*dest = bmBuf;
		h = w;//texP [0]->m_info.height;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				*dest++ = ptr [(((y - y0 + h) % h) * w) + ((x - x0 + w) % w)];
#endif
		}
	}

// Overlay texture 2 if present

if (textures [1] == 0)
	return 0;
if (!(ptr = texP [1]->m_info.bmDataP))
	return 0;
if (texP [0]->m_info.width == texP [1]->m_info.width)
	scale.c = scale.d = 1;
else if (texP [0]->m_info.width < texP [1]->m_info.width) {
	scale.c = texP [1]->m_info.width / texP [0]->m_info.width;
	scale.d = 1;
	}
else if (texP [0]->m_info.width > texP [1]->m_info.width) {
	scale.d = texP [0]->m_info.width / texP [1]->m_info.width;
	scale.c = 1;
	}
scale2.c = scale.c * scale.c;
scale2.d = scale.d * scale.d;
offs = 0;
w = texP [1]->m_info.width / scale.c * scale.d;
h = w;//texP [1]->m_info.height / scale.c * scale.d;
s = (texP [1]->m_info.width * texP [1]->m_info.width)/*texP [1]->m_info.size*/ / scale2.c * scale2.d;
if (!(x0 || y0)) {
	UINT8 *dest, *dest2;
	if (mode == 0x0000) {
		dest = bmBuf;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++, dest++) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest = c;
				}
		}
	else if (mode == (INT16) 0x4000) {
		dest = bmBuf + h - 1;
		for (y = 0; y < h; y++, dest--)
			for (x = 0, dest2 = dest; x < w; x++, dest2 += w) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest2 = c;
				}
		}
	else if (mode == (INT16) 0x8000) {
		dest = bmBuf + s - 1;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++, dest--) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest = c;
				}
		}
	else if (mode == (INT16) 0xC000) {
		dest = bmBuf + (h - 1) * w;
		for (y = 0; y < h; y++, dest++)
			for (x = 0, dest2 = dest; x < w; x++, dest2 -= w) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					*dest2 = c;
				}
		}
	} 
else {
	if (mode == 0x0000) {
		for (y = 0; y < h; y++) {
			y1 = ((y + y0) % h) * w;
			for (x = 0; x < w; x++) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBuf [y1 + (x + x0) % w] = c;
				}
			}
		}
	else if (mode == (INT16) 0x4000) {
		for (y = h - 1; y >= 0; y--)
			for (x = 0; x < w; x++) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBuf [((x + y0) % h) * w + (y + x0) % w] = c;
				}
		}
	else if (mode == (INT16) 0x8000) {
		for (y = h - 1; y >= 0; y--) {
			y1 = ((y + y0) % h) * w;
			for (x = w - 1; x >= 0; x--) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBuf [y1 + (x + x0) % w] = c;
				}
			}
		}
	else if (mode == (INT16) 0xC000) {
		for (y = 0; y < h; y++)
			for (x = w - 1; x >= 0; x--) {
				c = ptr [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d];
				if (c != 255)
					bmBuf [((x + y0) % h) * w + (y + x0) % w] = c;
				}
			}
	}
return 0;
}

//------------------------------------------------------------------------
// ReadTextureFromFile ()
//
// ACTION - defines a bitmap from a descent PIG fTextures.
//
// INPUTS - index:	no. of texture within pig file
//          fTextures:	pointer to pig file stream
//			mode:	bit16/15=position of orgin
//                  00=upper left   01=upper right
//                  10=lower right  11=lower left
//
// Changes - Y axis flipped to since DIBs have zero in lower left corner
//------------------------------------------------------------------------

INT32 CTexture::Read (INT16 index) 
{
	UINT8				xsize[200];
	UINT8				line[320],*line_ptr;
	UINT8				j;
	PIG_TEXTURE		ptexture;
	D2_PIG_TEXTURE d2_ptexture;
	PIG_HEADER		file_header;
	D2_PIG_HEADER	d2_file_header;
	INT32				offset,dataOffset;
	INT16				x,y,w,h,s;
	UINT8				byte,runcount,runvalue;
	INT32				nSize;
	INT16				linenum;
	HRSRC				hFind = 0;
	HGLOBAL			hGlobal = 0;
	INT16				*texture_table;
	INT32				rc;
	INT16				*texture_ptr;
	FILE				*fTextures = NULL;
	char				path [256];
	
if (m_info.bModified)
	return 0;
strcpy_s (path, sizeof (path), (theApp.IsD1File ()) ? descent_path : descent2_path);
if (!strstr (path, ".pig"))
	strcat_s (path, sizeof (path), "groupa.pig");

HINSTANCE hInst = AfxGetInstanceHandle ();

// do a range check on the texture number
if ((index > ((theApp.IsD1File ()) ? MAX_D1_TEXTURES : MAX_D2_TEXTURES)) || (index < 0)) {
DEBUGMSG (" Reading texture: Texture # out of range.");
rc = 1;
goto abort;
}

// get pointer to texture table from resource fTextures
hFind = (theApp.IsD1File ()) ?
	FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE_DAT), "RC_DATA") : 
	FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE2_DAT), "RC_DATA");
if (!hFind) {
	DEBUGMSG (" Reading texture: Texture resource not found.");
	rc = 1;
	goto abort;
	}
hGlobal = LoadResource (hInst, hFind);
if (!hGlobal) {
	DEBUGMSG (" Reading texture: Could not load texture resource.");
	rc = 2;
	goto abort;
	}
// first long is number of textures
texture_ptr = (INT16 *)LockResource (hGlobal);
texture_table = texture_ptr + 2;

fopen_s (&fTextures, path, "rb");
if (!fTextures) {
	DEBUGMSG (" Reading texture: Texture file not found.");
	rc = 1;
	goto abort;
	}

// read fTextures header
fseek (fTextures,0,SEEK_SET);
dataOffset = read_INT32 (fTextures);
if (dataOffset == 0x47495050L) /* 'PPIG' Descent 2 type */
	dataOffset = 0;
else if (dataOffset < 0x10000L)
	dataOffset = 0;
fseek (fTextures,dataOffset,SEEK_SET);
if (theApp.IsD2File ())
	fread (&d2_file_header, sizeof (d2_file_header), 1, fTextures);
else
	fread (&file_header, sizeof (file_header), 1, fTextures);

// read texture header
if (theApp.IsD2File ()) {
	offset = sizeof (D2_PIG_HEADER) + dataOffset +
				(FIX) (texture_table[index]-1) * sizeof (D2_PIG_TEXTURE);
	fseek (fTextures,offset,SEEK_SET);
	fread (&d2_ptexture, sizeof (D2_PIG_TEXTURE), 1, fTextures);
	w = d2_ptexture.xsize + ((d2_ptexture.wh_extra & 0xF) << 8);
	h = d2_ptexture.ysize + ((d2_ptexture.wh_extra & 0xF0) << 4);
	}
else {
	offset = sizeof (PIG_HEADER) + dataOffset + 
				(FIX) (texture_table[index]-1) * sizeof (ptexture);
	fseek (fTextures,offset,SEEK_SET);
	fread (&ptexture, sizeof (PIG_TEXTURE), 1, fTextures);
	ptexture.name [sizeof (ptexture.name) - 1] = '\0';
	// copy d1 texture into d2 texture struct
	strncpy_s (d2_ptexture.name, sizeof (d2_ptexture.name), ptexture.name, sizeof (ptexture.name));
	d2_ptexture.dflags = ptexture.dflags;
	d2_ptexture.xsize = ptexture.xsize;
	d2_ptexture.ysize = ptexture.ysize;
	d2_ptexture.flags = ptexture.flags;
	d2_ptexture.avg_color = ptexture.avg_color;
	d2_ptexture.offset = ptexture.offset;
	d2_ptexture.wh_extra = (ptexture.dflags == 128) ? 1 : 0;
	w = h = 64;
	}
s = w * h;

// seek to data
if (theApp.IsD2File ()) {
	offset = sizeof (D2_PIG_HEADER) + dataOffset
	+ d2_file_header.num_textures * sizeof (D2_PIG_TEXTURE)
	+ d2_ptexture.offset;
	}
else {
	offset =  sizeof (PIG_HEADER) + dataOffset
	+ file_header.number_of_textures * sizeof (PIG_TEXTURE)
	+ file_header.number_of_sounds   * sizeof (PIG_SOUND)
	+ d2_ptexture.offset;
	}

// allocate data if necessary
if (m_info.bmDataP && ((m_info.width != w) || (m_info.height != h))) {
	delete m_info.bmDataP;
	m_info.bmDataP = NULL;
	}
if (m_info.tgaDataP) {
	delete m_info.tgaDataP;
	m_info.tgaDataP = NULL;
	}
if (m_info.bmDataP == NULL)
	m_info.bmDataP = new UINT8 [s];
if (m_info.bmDataP == NULL) {
	rc = 1;
	goto abort;
	}
m_info.nFormat = 0;
fseek (fTextures,offset,SEEK_SET);
if (d2_ptexture.flags & 0x08) {
	fread (&nSize,1,sizeof (INT32),fTextures);
	fread (xsize,d2_ptexture.ysize,1,fTextures);
	linenum = 0;
	for (y=h-1;y>=0;y--) {
		fread (line,xsize[linenum++],1,fTextures);
		line_ptr = line;
			for (x=0;x<w;) {
			byte = *line_ptr++;
			if ((byte & 0xe0) == 0xe0) {
				runcount = byte & 0x1f;
				runvalue = *line_ptr++;
				for (j=0;j<runcount;j++) {
					if (x<w) {
						m_info.bmDataP[y*w+x] = runvalue;
						x++;
						}
					}
				}
			else {
				m_info.bmDataP[y*w+x] = byte;
				x++;
				}
			}
		}
	}
else {
	for (y=h-1;y>=0;y--) {
#if 1
		fread (m_info.bmDataP + y * w, w, 1, fTextures);
#else
		fread (line,w,1,fTextures);
		line_ptr = line;
		for (x=0;x<w;x++) {
			byte = *line_ptr++;
			m_info.bmDataP[y*w+x] = byte;
			}
#endif
		}
	}
fclose (fTextures);
m_info.width = w;
m_info.height = h;
m_info.size = s;
m_info.bValid = 1;
return (0);

abort:
// free handle
if (hGlobal) FreeResource (hGlobal);
return (rc);
}

//------------------------------------------------------------------------

double CTexture::Scale (INT16 index)
{
if (!m_info.width)
	if (index < 0)
		return 1.0;
	else
		Read (index);
return m_info.width ? (double) m_info.width / 64.0 : 1.0;
}

//------------------------------------------------------------------------------

INT32 RLEExpand (D2_PIG_TEXTURE *bmP, UINT8 *texBuf)
{
#	define RLE_CODE			0xE0
#	define NOT_RLE_CODE		31
#	define IS_RLE_CODE(x)	(((x) & RLE_CODE) == RLE_CODE)


	UINT8	*expandBuf, *pSrc, *pDest;
	UINT8	c, h;
	INT32	i, j, l, bufSize, bBigRLE;
	UINT16 nLineSize;

bufSize = bmP->xsize * bmP->ysize;
if (!(bmP->flags & BM_FLAG_RLE))
	return bufSize;
if (!(expandBuf = (UINT8 *) malloc (bufSize)))
	return -1;

bBigRLE = (bmP->flags & BM_FLAG_RLE_BIG) != 0;
if (bBigRLE)
	pSrc = texBuf + 4 + 2 * bmP->ysize;
else
	pSrc = texBuf + 4 + bmP->ysize;
pDest = expandBuf;
for (i = 0; i < bmP->ysize; i++, pSrc += nLineSize) {
	if (bBigRLE)
		nLineSize = *((UINT16 *) (texBuf + 4 + 2 * i));
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
l = (INT32) (pDest - expandBuf);
memcpy (texBuf, expandBuf, l);
bmP->flags &= ~(BM_FLAG_RLE | BM_FLAG_RLE_BIG);
free (expandBuf);
return l;
}

//-----------------------------------------------------------------------------
// ReadPog ()
//-----------------------------------------------------------------------------

INT32 ReadPog (FILE *fTextures, UINT32 nFileSize) 
{
	D2_PIG_HEADER	d2_file_header;
	D2_PIG_TEXTURE d2texture;

	HRSRC			hFind = 0;
	HGLOBAL		hGlobal = 0;
	UINT32		*n_textures = 0;
	UINT16		*texture_table;
	UINT16		nBaseTex;
	UINT16		*xlatTbl = NULL;
	INT32			tWidth, tHeight, tSize;
	UINT32		offset, hdrOffset, bmpOffset, hdrSize, xlatTblSize;
	INT32			rc; // return code;
	INT32			tnum;
	UINT8			*ptr;
	INT32			row;
	UINT16		nUnknownTextures, nMissingTextures;
	bool			bExtraTexture;
	CTexture	*texP;
	INT32			fileType = theApp.FileType ();

// make sure this is descent 2 fTextures
if (theApp.IsD1File ()) {
	INFOMSG (" Descent 1 does not support custom textures.");
	rc = 1;
	goto abort;
	}

//--------------------------------------------------------------------
// first open the texture table
//--------------------------------------------------------------------
// get pointer to texture table from resource fTextures
//	hFind	 = FindResource (hInst,"TEXTURE_DATA2", "RC_DATA");
hFind = FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE2_DAT), "RC_DATA");
if (!hFind) {
	DEBUGMSG (" POG manager: Texture resource not found.");
	rc = 2;
	goto abort;
	}
hGlobal = LoadResource (hInst, hFind);
if (!hGlobal) {
	DEBUGMSG (" POG manager: Could not load texture resource from pog file.");
	rc = 3;
	goto abort;
	}
n_textures = (UINT32 *)LockResource (hGlobal);
texture_table = (UINT16 *) (n_textures + 1);     // first long is number of textures

// read file header
fread (&d2_file_header, sizeof (D2_PIG_HEADER), 1, fTextures);
if (d2_file_header.signature != 0x474f5044L) {  // 'DPOG'
	ErrorMsg ("Invalid pog file - reading from hog file");
	rc = 4;
	goto abort;
	}
FreeTextureHandles ();
sprintf_s (message, sizeof (message), " Pog manager: Reading %d custom textures",d2_file_header.num_textures);
DEBUGMSG (message);
xlatTbl = new UINT16 [d2_file_header.num_textures];
if (!xlatTbl) {
	rc = 5;
	goto abort;
	}
xlatTblSize = d2_file_header.num_textures * sizeof (UINT16);
offset = ftell (fTextures);
fread (xlatTbl, xlatTblSize, 1, fTextures);
// loop for each custom texture
nUnknownTextures = 0;
nMissingTextures = 0;
hdrOffset = offset + xlatTblSize;
hdrSize = xlatTblSize + d2_file_header.num_textures * sizeof (D2_PIG_TEXTURE);
bmpOffset = offset + hdrSize;
for (tnum = 0; tnum < d2_file_header.num_textures; tnum++) {
	// read texture index
	UINT16 texture_index = xlatTbl [tnum];
	// look it up in the list of textures
	for (nBaseTex = 0; nBaseTex < MAX_D2_TEXTURES; nBaseTex++)
		if (texture_table [nBaseTex] == texture_index)
			break;
	bExtraTexture = (nBaseTex >= MAX_D2_TEXTURES);
	// get texture data offset from texture header
#if 1
	fseek (fTextures, hdrOffset + tnum * sizeof (D2_PIG_TEXTURE), SEEK_SET);
#endif
	if (fread (&d2texture, sizeof (D2_PIG_TEXTURE), 1, fTextures) != 1)
		break;
	tWidth = d2texture.xsize + ((d2texture.wh_extra & 0xF) << 8);
	if ((d2texture.flags & 0x80) && (tWidth > 256))
		tHeight = (INT32) d2texture.ysize * tWidth;
	else
		tHeight = d2texture.ysize + ((d2texture.wh_extra & 0xF0) << 4);
//	if (tWidth != 512 || tHeight != 512)
//		continue;
	tSize = tWidth * tHeight;
	if (hdrSize + d2texture.offset + tSize >= nFileSize) {
		nMissingTextures++;
		continue;
		}
	if (bExtraTexture) {
		pExtraTexture	pxTx = (pExtraTexture) malloc (sizeof (*pxTx));
		if (!pxTx) {
			nUnknownTextures++;
			continue;
			}
		pxTx->pNext = extraTextures;
		extraTextures = pxTx;
		pxTx->texture_index = texture_index;
		pxTx->texture.m_info.bmDataP = NULL;
		texP = &(pxTx->texture);
		nBaseTex = 0;
		}
	else
		texP = theMine->Textures (fileType);
// allocate memory for texture if not already
	ptr = (UINT8*) malloc (tWidth * tHeight);
	if (ptr) {
		if (texP [nBaseTex].m_info.bmDataP)
			delete texP [nBaseTex].m_info.bmDataP;
		texP [nBaseTex].m_info.bmDataP = ptr;
		if (d2texture.flags & 0x80) {
			ptr = (UINT8*) malloc (tSize * sizeof (tRGBA));
			if (ptr) {
				texP [nBaseTex].m_info.tgaDataP = (tRGBA *) ptr;
				texP [nBaseTex].m_info.nFormat = 1;
				}
			else {
				delete texP [nBaseTex].m_info.bmDataP;
				continue;
				}
			}
		else
			texP [nBaseTex].m_info.nFormat = 0;
		texP [nBaseTex].m_info.width = tWidth;
		texP [nBaseTex].m_info.height = tHeight;
		texP [nBaseTex].m_info.size = tSize;
		texP [nBaseTex].m_info.bValid = 1;
		// read texture into memory (assume non-compressed)
#if 1
		fseek (fTextures, bmpOffset + d2texture.offset, SEEK_SET);
#endif
		if (texP [nBaseTex].m_info.nFormat) {
			fread (ptr, texP [nBaseTex].m_info.size * sizeof (tRGBA), 1, fTextures);
			texP [nBaseTex].m_info.bValid = 
				TGA2Bitmap (texP [nBaseTex].m_info.tgaDataP, texP [nBaseTex].m_info.bmDataP, (INT32) tWidth, (INT32) tHeight);
			}
		else {
			if (d2texture.flags & BM_FLAG_RLE) {
				fread (ptr, tSize, 1, fTextures);
				RLEExpand (&d2texture, ptr);
				}
			else {
				UINT8 *p = ptr + tWidth * (tHeight - 1); // point to last row of bitmap
				for (row = 0; row < tHeight; row++) {
					fread (p, tWidth, 1, fTextures);
					p -= tWidth;
					}
				}
			}
		if (!bExtraTexture)
			texP [nBaseTex].m_info.bModified = TRUE;
		}
	}
if (nUnknownTextures) {
	sprintf_s (message, sizeof (message), " Pog manager: %d unknown textures found.", nUnknownTextures);
	DEBUGMSG (message);
	}
if (nMissingTextures) {
	sprintf_s (message, sizeof (message), " Pog manager: %d textures missing (Pog file probably damaged).", nMissingTextures);
	DEBUGMSG (message);
	}

rc = 0;

abort:

if (xlatTbl)
	delete xlatTbl;
if (n_textures) 
	GlobalUnlock (hGlobal);  // no need to unlock it but what the heck
if (hGlobal) 
	FreeResource (hGlobal);
return rc;
}

//-----------------------------------------------------------------------------

void WritePogTextureHeader (FILE *pDestPigFile, CTexture *pTexture, INT32 nTexture, UINT32& nOffset)
{
	D2_PIG_TEXTURE d2texture;
	UINT8 *pSrc;

sprintf_s (d2texture.name, sizeof (d2texture.name), "new%04d", nTexture);
d2texture.dflags = 0;
d2texture.flags = pTexture->m_info.nFormat ? 0x80 : 0;
d2texture.xsize = pTexture->m_info.width % 256;
if ((d2texture.flags & 0x80) && (pTexture->m_info.width > 256)) {
	d2texture.wh_extra = (pTexture->m_info.width >> 8);
	d2texture.ysize = pTexture->m_info.height / pTexture->m_info.width;
	}
else {
	d2texture.ysize = pTexture->m_info.height % 256;
	d2texture.wh_extra = (pTexture->m_info.width >> 8) | ((pTexture->m_info.height >> 4) & 0xF0);
	}
d2texture.avg_color = 0;
d2texture.offset = nOffset;
nOffset += (d2texture.flags & 0x80) ? pTexture->m_info.size * 4: pTexture->m_info.size;

// check for transparency and super transparency
if (!pTexture->m_info.nFormat)
	if (pSrc = (UINT8 *) pTexture->m_info.bmDataP) {
		UINT32 j;
		for (j = 0; j < pTexture->m_info.size; j++, pSrc++) {
			if (*pSrc == 255) 
				d2texture.flags |= BM_FLAG_TRANSPARENT;
			if (*pSrc == 254) 
				d2texture.flags |= BM_FLAG_SUPER_TRANSPARENT;
			}
	}
fwrite (&d2texture, sizeof (D2_PIG_TEXTURE), 1, pDestPigFile);
}

//-----------------------------------------------------------------------------

bool WritePogTexture (FILE *pDestPigFile, CTexture *pTexture)
{
	UINT8		*pSrc = pTexture->m_info.nFormat ? (UINT8*) pTexture->m_info.tgaDataP : pTexture->m_info.bmDataP;

if (!pSrc) {
	DEBUGMSG (" POG manager: Couldn't lock texture data.");
	return false;
	}
if (pTexture->m_info.nFormat) {
#if 0
	INT32 w = pTexture->m_info.width;
	INT32 h = pTexture->m_info.height;
	tBGRA	bgra;

	pSrc += w * (h - 1) * 4;
	INT32 i, j;
	for (i = h; i; i--) {
		for (j = w; j; j--) {
			bgra.b = *pSrc++;
			bgra.g = *pSrc++;
			bgra.r = *pSrc++;
			bgra.a = *pSrc++;
			fwrite (&bgra, sizeof (bgra), 1, pDestPigFile);
			}
		pSrc -= 2 * w * 4;
		}
#else
	fwrite (pSrc, pTexture->m_info.width * pTexture->m_info.height * sizeof (tRGBA), 1, pDestPigFile);
#endif
	}
else {
	UINT16 w = pTexture->m_info.width;
	UINT16 h = pTexture->m_info.height;
	pSrc += w * (h - 1); // point to last row of bitmap
	INT32 row;
	for (row = 0; row < h; row++) {
		fwrite (pSrc, w, 1, pDestPigFile);
		pSrc -= w;
		}
	}
return true;
}

//-----------------------------------------------------------------------------
// CreatePog ()
//
// Action - Creates a POG fTextures from all the changed textures
//
// Format:
//   Pig Header
//   Texture Indicies (N total * UINT16)
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
//   UINT8  data[dataSize];    // raw data
//
// Commpressed Texture data:
//   UINT32 totalSize;      // including this long word and everything below
//   UINT8  lineSizes[64];  // size of each line (in bytes)
//   UINT8  data[dataSize]; // run length encoded (RLE) data
//
// RLE definition:
//
//   If upper 3 bits of byte are set, lower 5 bytes represents how many times
//   to repeat the following byte.  The last byte of the data must be set
//   to 0xE0, which means 0 bytes to follow.  Note: The last byte may be
//   omitted since the xsize array defines the length of each line???
//
//-----------------------------------------------------------------------------
INT32 CreatePog (FILE *outPigFile) 
{
	INT32 rc; // return code;
	D2_PIG_HEADER d2_file_header;
	HRSRC hFind = 0;
	HGLOBAL hGlobal = 0;
	UINT32 *n_textures = 0, nOffset = 0;
	UINT16 *texture_table;
	INT32 i;
	INT32 num;
	pExtraTexture	pxTx;
	CTexture* texP;
	INT32	fileType = theApp.FileType ();

if (theApp.IsD1File ()) {
	ErrorMsg ("Descent 1 does not support custom textures.");
	rc = 4;
	goto abort;
	}

//--------------------------------------------------------------------
// first open the texture table
//--------------------------------------------------------------------
// get pointer to texture table from resource fTextures
hFind   = FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE2_DAT), "RC_DATA");
if (!hFind) {
	DEBUGMSG (" Reading texture: Texture resource not found.");
	rc = 5;
	goto abort;
	}
hGlobal = LoadResource (hInst, hFind);
if (!hGlobal) {
	DEBUGMSG (" Pog manager: Could not load texture resource.");
	rc = 6;
	goto abort;
	}
n_textures = (UINT32 *)LockResource (hGlobal);
texture_table = (UINT16 *) (n_textures + 1);     // first long is number of textures

sprintf_s (message, sizeof (message),"%s\\dle_temp.pog",m_startFolder );

// write file  header
d2_file_header.signature    = 0x474f5044L; /* 'DPOG' */
d2_file_header.version      = 0x00000001L;
d2_file_header.num_textures = 0;
for (i = 0, texP = theMine->Textures (fileType); i < MAX_D2_TEXTURES; i++, texP++)
	if (texP->m_info.bModified)
		d2_file_header.num_textures++;
for (pxTx = extraTextures; pxTx; pxTx = pxTx->pNext)
	d2_file_header.num_textures++;
fwrite (&d2_file_header, sizeof (D2_PIG_HEADER), 1, outPigFile);

// write list of textures
for (i = 0, texP = theMine->Textures (fileType); i < MAX_D2_TEXTURES; i++, texP++)
	if (texP->m_info.bModified)
		fwrite (texture_table + i, sizeof (UINT16), 1, outPigFile);

for (pxTx = extraTextures; pxTx; pxTx = pxTx->pNext)
	fwrite (&pxTx->texture_index, sizeof (UINT16), 1, outPigFile);

// write texture headers
num = 0;
for (i = 0, texP = theMine->Textures (fileType); i < MAX_D2_TEXTURES; i++, texP++)
	if (texP->m_info.bModified)
		WritePogTextureHeader (outPigFile, texP, num++, nOffset);
for (pxTx = extraTextures; pxTx; pxTx = pxTx->pNext, num++)
	WritePogTextureHeader (outPigFile, &pxTx->texture, num, nOffset);

sprintf_s (message, sizeof (message)," Pog manager: Saving %d custom textures",d2_file_header.num_textures);
DEBUGMSG (message);

//-----------------------------------------
// write textures (non-compressed)
//-----------------------------------------
rc = 8;
for (i = 0, texP = theMine->Textures (fileType); i < MAX_D2_TEXTURES; i++, texP++)
	if (texP->m_info.bModified && !WritePogTexture (outPigFile, texP))
		goto abort;
for (pxTx = extraTextures; pxTx; pxTx = pxTx->pNext)
	if (!WritePogTexture (outPigFile, &pxTx->texture))
		goto abort;

rc = 0; // return success

abort:
if (outPigFile) 
	fclose (outPigFile);
if (n_textures) 
	GlobalUnlock (hGlobal);  // no need to unlock it but what the heck
if (hGlobal) 
	FreeResource (hGlobal);
return rc;
}

//------------------------------------------------------------------------
// Free Texture Handles
//------------------------------------------------------------------------

void FreeTextureHandles (bool bDeleteModified) 
{
  // free any textures that have been buffered
	INT32 i, j;
	INT32 fileType = theMine->FileType ();

for (i = 0; i < 2; i++) {
	CTexture* texP = theMine->Textures (i);
	for (j = MAX_D2_TEXTURES; j; j--, texP++) {
		if (!bDeleteModified && texP->m_info.bModified)
			continue;
		if (texP->m_info.bmDataP) {
			free (texP->m_info.bmDataP);
			texP->m_info.bmDataP = NULL;
			}
		if (texP->m_info.tgaDataP) {
			free (texP->m_info.tgaDataP);
			texP->m_info.tgaDataP = NULL;
			}
		texP->m_info.bModified = FALSE;
		texP->m_info.nFormat = 0;
		}
	}
pExtraTexture	p;
while (extraTextures) {
	p = extraTextures;
	extraTextures = p->pNext;
	free (p);
	}
}



BOOL HasCustomTextures () 
{
CTexture* texP = theMine->Textures (theApp.FileType ());

for (INT32 i = MAX_D2_TEXTURES; i; i--, texP++)
	if (texP->m_info.bModified)
		return TRUE;
return FALSE;
}



INT32 CountCustomTextures () 
{
	INT32			count = 0;
	CTexture*	texP = theMine->Textures (theApp.FileType ());

for (INT32 i = MAX_D2_TEXTURES; i; i--, texP++)
	if (texP->m_info.bModified)
		count++;
return count;
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

void RgbFromIndex (INT32 nIndex, PALETTEENTRY *pRGB)
{
if (pRGB) {
	HRSRC hFind = FindResource (hInst, PaletteResource (), "RC_DATA");
	if (hFind) {
		HGLOBAL hPalette = LoadResource (hInst, hFind);
		UINT8 *palette = ((UINT8 *) LockResource (hPalette)) + 3 * nIndex;
		pRGB->peRed = (*palette++)<<2;
		pRGB->peGreen = (*palette++)<<2;
		pRGB->peBlue = (*palette)<<2;
		pRGB->peFlags = 0;
		}
	}
}

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------


BITMAPINFO *MakeBitmap (void) 
{
	BITMAPINFO *bmi;
	BITMAPINFOHEADER *bi;
	INT16 i;
	HRSRC hFind;
	HGLOBAL hPalette;
	UINT8 *palette;

	typedef struct tagMyBMI {
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD bmiColors[256];
		} MyBMI;

static MyBMI my_bmi;

	// point bitmap info to structure
	bmi = (BITMAPINFO *)&my_bmi;

	// point the info to the bitmap header structure
	bi = &bmi->bmiHeader;

	// define bit map info header elements
	bi->biSize          = sizeof (BITMAPINFOHEADER);
	bi->biWidth         = 64;
	bi->biHeight        = 64;
	bi->biPlanes        = 1;
	bi->biBitCount      = 8;
	bi->biCompression   = BI_RGB;
	bi->biSizeImage     = 0;
	bi->biXPelsPerMeter = 0;
	bi->biYPelsPerMeter = 0;
	bi->biClrUsed       = 0;
	bi->biClrImportant  = 0;

	// define d1 palette from resource
	hFind = FindResource (hInst, PaletteResource (), "RC_DATA");
	if (!hFind) {
		DEBUGMSG (" Bitmap creation: Palette resource not found.");
		return NULL;
		}
	hPalette = LoadResource (hInst, hFind);
	palette = (UINT8 *) LockResource (hPalette);
#if 0
	for (i=0;i<256;i++) {
	  bmi->bmiColors[i].rgbRed      = *palette++;
	  bmi->bmiColors[i].rgbGreen    = *palette++;
	  bmi->bmiColors[i].rgbBlue     = *palette++;
	  bmi->bmiColors[i].rgbReserved = 0;palette++;
	}
#else
	for (i=0;i<256;i++) {
	  bmi->bmiColors[i].rgbRed    = (*palette++)<<2;
	  bmi->bmiColors[i].rgbGreen  = (*palette++)<<2;
	  bmi->bmiColors[i].rgbBlue   = (*palette++)<<2;
	  bmi->bmiColors[i].rgbReserved = 0;
	}
#endif
	FreeResource (hPalette);
  return bmi;
}
// -------------------------------------------------------------------------- 
// -------------------------------------------------------------------------- 

bool PaintTexture (CWnd *pWnd, INT32 bkColor, INT32 nSegment, INT32 nSide, INT32 nBaseTex, INT32 nOvlTex, INT32 xOffset, INT32 yOffset)
{
if (!theMine) 
	return false;

	static INT32 dataOffset [2] = {0, 0};

	CDC			*pDC = pWnd->GetDC ();

if (!pDC)
	return false;

	HINSTANCE	hInst = AfxGetApp ()->m_hInstance;
	CBitmap		bmTexture;
	FILE			*fTextures = NULL;
	char			szFile [256];
	BITMAP		bm;
	CDC			memDC;
	CSegment*	segP;
	CSide*		sideP;
	INT16			nWall;
	bool			bShowTexture = true;
	char			*path = (theApp.IsD1File ()) ? descent_path : descent2_path;

CRect	rc;
pWnd->GetClientRect (rc);

if (nBaseTex < 0) {
	segP = (nSegment < 0) ? theMine->CurrSeg () : theMine->Segments (nSegment);
	sideP = (nSide < 0) ? theMine->CurrSide () : segP->m_sides + nSide;
	INT32 nSide = theMine->Current ()->nSide;
	nBaseTex = sideP->m_info.nBaseTex;
	nOvlTex = sideP->m_info.nOvlTex & 0x1fff;
	if (segP->m_info.children [nSide] == -1)
		bShowTexture = TRUE;
	else {
		nWall = sideP->m_info.nWall;
		bShowTexture = (nWall < theMine->GameInfo ().walls.count);
		}
	}
if ((nBaseTex < 0) || (nBaseTex >= MAX_TEXTURES + 10))
	bShowTexture = false;
if ((nOvlTex < 0) || (nOvlTex >= MAX_TEXTURES))	// this allows to suppress bitmap display by 
	bShowTexture = false;									// passing 0 for texture 1 and -1 for nOvlTex

if (bShowTexture) {
	// check pig file
	if (dataOffset [theApp.IsD1File ()] == 0) {
		strcpy_s (szFile, sizeof (szFile), (theApp.IsD1File ()) ? descent_path : descent2_path);
		if (fopen_s (&fTextures, szFile, "rb"))
			dataOffset [theApp.IsD1File ()] = -1;  // pig file not found
		else {
			fseek (fTextures, 0, SEEK_SET);
			dataOffset [theApp.IsD1File ()] = read_INT32 (fTextures);  // determine type of pig file
			fclose (fTextures);
			}
		}
	if (dataOffset [theApp.IsD1File ()] > 0x10000L) {  // pig file type is v1.4a or descent 2 type
		CTexture	tex (bmBuf, true);
		if (DefineTexture (nBaseTex, nOvlTex, &tex, xOffset, yOffset))
			DEBUGMSG (" Texture renderer: Texture not found (DefineTexture failed)");
		CPalette *pOldPalette = pDC->SelectPalette (theMine->m_currentPalette, FALSE);
		pDC->RealizePalette ();
		INT32 caps = pDC->GetDeviceCaps (RASTERCAPS);
		if (caps & RC_DIBTODEV) {
			BITMAPINFO *bmi = MakeBitmap ();
			if (!bmi)
				return false;
			bmi->bmiHeader.biWidth = 
			bmi->bmiHeader.biHeight = tex.m_info.width;
			StretchDIBits (pDC->m_hDC, 0, 0, rc.Width (), rc.Height (), 0, 0, tex.m_info.width, tex.m_info.width,
					        	 (void *) bmBuf, bmi, DIB_RGB_COLORS, SRCCOPY);
			}
		else {
			double scale = tex.Scale ();
			UINT32 x, y;
			for (x = 0; x < tex.m_info.width; x = (INT32) (x + scale))
				for (y = 0; y < tex.m_info.width; y = (INT32) (y + scale))
					pDC->SetPixel ((INT16) (x / scale), (INT16) (y / scale), PALETTEINDEX (bmBuf [y*tex.m_info.width+x]));
			}
		pDC->SelectPalette (pOldPalette, FALSE);
		}
	else {
		HGDIOBJ hgdiobj1;
		bmTexture.LoadBitmap ((dataOffset < 0) ? "NO_PIG_BITMAP" : "WRONG_PIG_BITMAP");
		bmTexture.GetObject (sizeof (BITMAP), &bm);
		memDC.CreateCompatibleDC (pDC);
		hgdiobj1 = memDC.SelectObject (bmTexture);
		pDC->StretchBlt (0, 0, rc.Width (), rc.Height (), &memDC, 0, 0, 64, 64, SRCCOPY);
		memDC.SelectObject (hgdiobj1);
		DeleteObject (bmTexture);
//		DeleteDC (memDC.m_hDC);
		}
	}
else if (bkColor < 0) {
	HGDIOBJ hgdiobj;
	// set bitmap
	bmTexture.LoadBitmap ("NO_TEXTURE_BITMAP");
	bmTexture.GetObject (sizeof (BITMAP), &bm);
	memDC.CreateCompatibleDC (pDC);
	hgdiobj = memDC.SelectObject (bmTexture);
	pDC->StretchBlt (0, 0, rc.Width (), rc.Height (), &memDC, 0, 0, 64, 64, SRCCOPY);
	memDC.SelectObject (hgdiobj);
	DeleteObject (bmTexture);
//	DeleteDC (memDC.m_hDC);
	}
else if (bkColor >= 0)
	pDC->FillSolidRect (&rc, (COLORREF) bkColor);
pWnd->ReleaseDC (pDC);
pWnd->InvalidateRect (NULL, TRUE);
pWnd->UpdateWindow ();
return bShowTexture;
}
//eof textures.cpp