/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#define COMPRESS		1	//do the RLE or not? (for debugging mostly)
#define WRITE_TINY	0	//should we write a TINY chunk?

#define MIN_COMPRESS_WIDTH	65	//don't compress if less than this wide

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "descent.h"
#include "u_mem.h"
#include "iff.h"
#include "cfile.h"
#include "error.h"

//Internal constants and structures for this library

//Type values for bitmaps
#define TYPE_PBM		0
#define TYPE_ILBM		1

//Compression types
#define cmpNone	0
#define cmpByteRun1	1

//Masking types
#define mskNone	0
#define mskHasMask	1
#define mskHasTransparentColor 2

//#define min(a, b) ((a<b)?a:b)

//------------------------------------------------------------------------------

#define MAKE_SIG(a, b, c, d) (((int32_t)(a)<<24)+((int32_t)(b)<<16)+((c)<<8)+(d))

#define form_sig MAKE_SIG('F', 'O', 'R', 'M')
#define ilbm_sig MAKE_SIG('I', 'L', 'B', 'M')
#define body_sig MAKE_SIG('B', 'O', 'D', 'Y')
#define pbm_sig  MAKE_SIG('P', 'B', 'M', ' ')
#define bmhd_sig MAKE_SIG('B', 'M', 'H', 'D')
#define anhd_sig MAKE_SIG('A', 'N', 'H', 'D')
#define cmap_sig MAKE_SIG('C', 'M', 'A', 'P')
#define tiny_sig MAKE_SIG('T', 'I', 'N', 'Y')
#define anim_sig MAKE_SIG('A', 'N', 'I', 'M')
#define dlta_sig MAKE_SIG('D', 'L', 'T', 'A')

//------------------------------------------------------------------------------

#if DBG
//void printsig(int32_t s)
//{
//	char *t=reinterpret_cast<char*> (&s);
//
///*  //printf("%c%c%c%c", *(&s+3), *(&s+2), *(&s+1), s);*/
//	//printf("%c%c%c%c", t[3], t[2], t[1], t[0]);
//}
#endif

//------------------------------------------------------------------------------

int32_t PutSig (int32_t sig, FILE *fp)
{
	char *s = reinterpret_cast<char*> (&sig);

fputc(s[3], fp);
fputc(s[2], fp);
fputc(s[1], fp);
return fputc(s[0], fp);

}

//------------------------------------------------------------------------------

int32_t PutByte (uint8_t c, FILE *fp)
{
return fputc(c, fp);
}

//------------------------------------------------------------------------------

int32_t PutWord (int32_t n, FILE *fp)
{
	uint8_t c0, c1;

c0 = (n & 0xff00) >> 8;
c1 = n & 0xff;
PutByte (c0, fp);
return PutByte (c1, fp);
}

//------------------------------------------------------------------------------

int32_t PutLong(int32_t n, FILE *fp)
{
	int32_t n0, n1;

n0 = (int32_t) ((n & 0xffff0000l) >> 16);
n1 = (int32_t) (n & 0xffff);
PutWord (n0, fp);
return PutWord (n1, fp);
}

//------------------------------------------------------------------------------

int32_t CIFF::GetSig (void)
{
union ciffSig {
	char	szId [4];
	int32_t	nId;
} ciffSig;

if (Pos () >= Len () - 4) 
		return EOF;
#if !(defined(WORDS_BIGENDIAN) || defined(__BIG_ENDIAN__))
	ciffSig.szId [3] = Data () [NextPos ()];
	ciffSig.szId [2] = Data () [NextPos ()];
	ciffSig.szId [1] = Data () [NextPos ()];
	ciffSig.szId [0] = Data () [NextPos ()];
#else
	ciffSig.szId [0] = Data () [NextPos ()];
	ciffSig.szId [1] = Data () [NextPos ()];
	ciffSig.szId [2] = Data () [NextPos ()];
	ciffSig.szId [3] = Data () [NextPos ()];
#endif
return ciffSig.nId;
}

//------------------------------------------------------------------------------

int32_t CIFF::GetBytes (char* dest, int32_t len)
{
	int32_t maxLen = Rem ();

if (len > maxLen)
	len = maxLen;
memcpy (dest, Data () + NextPos (len), len);
return len;
}

//------------------------------------------------------------------------------

char CIFF::GetByte (void)
{
if (Pos() >= Len()) return EOF;
return Data() [NextPos()];
}

//------------------------------------------------------------------------------

int32_t CIFF::GetWord (void)
{
if (Rem () < 2) 
	return EOF;
uint8_t c1 = Data() [NextPos()];
uint8_t c0 = Data() [NextPos()];
if (c0 == 0xff) 
	return EOF;
return ((int32_t) c1 << 8) + c0;
}

//------------------------------------------------------------------------------

int32_t CIFF::GetLong (void)
{
if (Rem () < 4) 
	return EOF;

uint8_t c3 = Data() [NextPos()];
uint8_t c2 = Data() [NextPos()];
uint8_t c1 = Data() [NextPos()];
uint8_t c0 = Data() [NextPos()];
return ((int32_t) c3 << 24) + ((int32_t) c2 << 16) + ((int32_t) c1 << 8) + c0;
}

//------------------------------------------------------------------------------

int32_t CIFF::ParseHeader (int32_t len, tIFFBitmapHeader *bmHeader)
{
bmHeader->w = GetWord ();
bmHeader->h = GetWord ();
bmHeader->x = GetWord ();
bmHeader->y = GetWord ();
bmHeader->nplanes = GetByte ();
bmHeader->masking = GetByte ();
bmHeader->compression = GetByte ();
GetByte ();        /* skip pad */
bmHeader->transparentColor = GetWord ();
bmHeader->xaspect = GetByte ();
bmHeader->yaspect = GetByte ();
bmHeader->pagewidth = GetWord ();
bmHeader->pageheight = GetWord ();
m_transparentColor = (char) bmHeader->transparentColor;
m_hasTransparency = 0;
if (bmHeader->masking == mskHasTransparentColor)
	m_hasTransparency = 1;
else if (bmHeader->masking != mskNone && bmHeader->masking != mskHasMask)
	return IFF_UNKNOWN_MASK;
return IFF_NO_ERROR;
}
//------------------------------------------------------------------------------
//  the buffer pointed to by raw_data is stuffed with a pointer to decompressed pixel data

int32_t CIFF::ParseBody (int32_t len, tIFFBitmapHeader *bmHeader)
{
	uint8_t*		p = bmHeader->raw_data;
	int32_t			width, depth;
	signed char	n;
	int32_t			nn, wid_cnt, end_cnt, plane;
	char			ignore = 0;
	uint8_t*		data_end;
	int32_t			endPos;

#if DBG
	int32_t rowCount=0;
#endif

width = 0;
depth = 0;
endPos = Pos() + len;
if (len&1)
	endPos++;
if (bmHeader->nType == TYPE_PBM) {
	width=bmHeader->w;
	depth=1;
	}
else if (bmHeader->nType == TYPE_ILBM) {
	width = (bmHeader->w+7)/8;
	depth=bmHeader->nplanes;
	}
end_cnt = (width&1)?-1:0;
data_end = p + width*bmHeader->h*depth;
if (bmHeader->compression == cmpNone) {        /* no compression */
	for (int32_t y = bmHeader->h; y; y--) {
#if 1
		p += GetBytes ((char*) p, width * depth);
#else
		for (int32_t x = 0; x < width * depth; x++)
			*p++= Data() [NextPos()];
#endif
		if (bmHeader->masking == mskHasMask)
			SetPos (Pos () + width);				//skip mask!
		if (bmHeader->w & 1) 
			NextPos();
		}
	}
else if (bmHeader->compression == cmpByteRun1)
	for (wid_cnt = width, plane = 0;Pos() < endPos && p < data_end;) {
		uint8_t c;
		if (wid_cnt == end_cnt) {
			wid_cnt = width;
			plane++;
			if ((bmHeader->masking == mskHasMask && plane==depth+1) ||
				 (bmHeader->masking != mskHasMask && plane==depth))
				plane=0;
			}
		Assert(wid_cnt > end_cnt);
	n=Data() [NextPos()];
	if (n >= 0) {                       // copy next n+1 bytes from source, they are not compressed
		nn = (int32_t) n+1;
		wid_cnt -= nn;
		if (wid_cnt==-1) {
			--nn; 
			Assert(width&1);
			}
		if (plane==depth)	//masking row
			SetPos (Pos() + nn);
		else
			while (nn--) 
				*p++= Data() [NextPos()];
			if (wid_cnt==-1) 
				NextPos();
		}
	else if (n>=-127) {             // next -n + 1 bytes are following byte
		c=Data() [NextPos()];
		nn = (int32_t) -n+1;
		wid_cnt -= nn;
		if (wid_cnt==-1) {
			--nn; 
			Assert(width&1);
			}
		if (plane!=depth) {//not masking row
			memset(p, c, nn); 
			p += nn;
			}
		}
#if DBG
	if ((p-bmHeader->raw_data) % width == 0)
		rowCount++;
	Assert((p-bmHeader->raw_data) - (width*rowCount) < width);
#endif
	}
if (p!=data_end)				//if we don't have the whole bitmap...
	return IFF_CORRUPT;		//...the give an error
//Pretend we read the whole chuck, because if we didn't, it's because
//we didn't read the last mask like or the last rle record for padding
//or whatever and it's not important, because we check to make sure
//we got the while bitmap, and that's what really counts.
SetPos(endPos);
if (ignore) 
	ignore++;   // haha, suppress the evil warning message
return IFF_NO_ERROR;
}


//------------------------------------------------------------------------------
//modify passed bitmap
int32_t CIFF::ParseDelta (int32_t len, tIFFBitmapHeader *bmHeader)
{
	uint8_t  *p=bmHeader->raw_data;
	int32_t y;
	int32_t chunk_end = Pos() + len;

	GetByte ();		//longword, seems to be equal to 4.  Don't know what it is

for (y=0;y<bmHeader->h;y++) {
	uint8_t n_items;
	int32_t cnt = bmHeader->w;
	uint8_t code;
	n_items = GetByte ();
	while (n_items--) {
		code = GetByte ();
		if (code==0) {				//repeat
			uint8_t rep = GetByte ();
			uint8_t val = GetByte ();
			cnt -= rep;
			if (cnt==-1)
				rep--;
			while (rep--)
				*p++= val;
			}
		else if (code > 0x80) {	//skip
			cnt -= (code-0x80);
			p += (code-0x80);
			if (cnt==-1)
				p--;
			}
		else {						//literal
			cnt -= code;
			if (cnt==-1)
				code--;
			while (code--)
				*p++= GetByte ();
			if (cnt==-1)
				GetByte ();
			}
		}
	if (cnt == -1) {
		if (!bmHeader->w&1)
			return IFF_CORRUPT;
		}
	else if (cnt)
		return IFF_CORRUPT;
	}
if (Pos() == chunk_end-1)		//pad
	NextPos();
if (Pos() != chunk_end)
	return IFF_CORRUPT;
return IFF_NO_ERROR;
}

//------------------------------------------------------------------------------
//  the buffer pointed to by raw_data is stuffed with a pointer to bitplane pixel data
void CIFF::SkipChunk (int32_t len)
{
	//int32_t c, i;
	int32_t ilen;
	ilen = (len+1) & ~1;

////printf("Skipping %d chunk\n", ilen);
SetPos (Pos() + ilen);
if (Pos() >= Len())
	SetPos(Len());
}

//------------------------------------------------------------------------------
//read an ILBM or PBM file
// Pass pointer to opened file, and to empty bitmap_header structure, and form length
int32_t CIFF::Parse (int32_t formType, tIFFBitmapHeader *bmHeader, int32_t formLen, CBitmap *pPrevBm)
{
	int32_t sig, len;
	//char ignore=0;
	int32_t startPos, endPos;

startPos = Pos();
endPos = startPos - 4 + formLen;
if (formType == pbm_sig)
	bmHeader->nType = TYPE_PBM;
else
	bmHeader->nType = TYPE_ILBM;
while ((Pos() < endPos) && (sig = GetSig ()) != EOF) {
	len = GetLong ();
	if (len == EOF) 
		break;
	switch (sig) {
		case bmhd_sig: {
			int32_t save_w = bmHeader->w, save_h = bmHeader->h;
			int32_t ret = ParseHeader (len, bmHeader);
			if (ret != IFF_NO_ERROR)
				return ret;
			if (bmHeader->raw_data) {
				if (save_w != bmHeader->w  ||  save_h != bmHeader->h)
					return IFF_BM_MISMATCH;
				}
			else {
				bmHeader->raw_data = NEW uint8_t [bmHeader->w * bmHeader->h];
				if (!bmHeader->raw_data)
					return IFF_NO_MEM;
				}
			break;
			}

		case anhd_sig:
			if (!pPrevBm) 
				return IFF_CORRUPT;
			bmHeader->w = pPrevBm->Width ();
			bmHeader->h = pPrevBm->Height ();
			bmHeader->nType = pPrevBm->Mode ();
			bmHeader->raw_data = NEW uint8_t [bmHeader->w * bmHeader->h];
			memcpy (bmHeader->raw_data, pPrevBm->Buffer (), bmHeader->w * bmHeader->h);
			SkipChunk (len);
			break;

		case cmap_sig: {
#if 1
			tPalEntry* p = bmHeader->palette;
			for (int32_t i = len / 3; i; i--, p++) {
				p->r = Data () [NextPos ()] / 4;
				p->g = Data () [NextPos ()] / 4;
				p->b = Data () [NextPos ()] / 4;
				}
#else
			char* p = (char*) &bmHeader->palette;
			len = GetBytes (p, len);
			for (int32_t i = len; i; i--)
				*p++ /= 4;
#endif
			if (len & 1) 
				NextPos();
			break;
			}

		case body_sig: {
			int32_t r;
			if ((r=ParseBody (len, bmHeader))!=IFF_NO_ERROR)
				return r;
			break;
			}

		case dlta_sig: {
			int32_t r;
			if ((r=ParseDelta (len, bmHeader))!=IFF_NO_ERROR)
				return r;
			break;
			}

		default:
			SkipChunk(len);
			break;
		}
	}
if (Pos() != startPos-4+formLen)
	return IFF_CORRUPT;
return IFF_NO_ERROR;    /* ok! */
}

//------------------------------------------------------------------------------
//convert an ILBM file to a PBM file
int32_t CIFF::ConvertToPBM (tIFFBitmapHeader *bmHeader)
{
	int32_t x, y, p;
	uint8_t *new_data, *destptr, *rowptr;
	int32_t bytes_per_row, byteofs;
	uint8_t checkmask, newbyte, setbit;

new_data = NEW uint8_t [bmHeader->w * bmHeader->h];
if (new_data == NULL) 
	return IFF_NO_MEM;
destptr = new_data;
bytes_per_row = 2*((bmHeader->w+15)/16);
for (y=0;y<bmHeader->h;y++) {
	rowptr = &bmHeader->raw_data[y * bytes_per_row * bmHeader->nplanes];
	for (x=0, checkmask=0x80;x<bmHeader->w;x++) {
		byteofs = x >> 3;
		for (p=newbyte=0, setbit=1;p<bmHeader->nplanes;p++) {
			if (rowptr[bytes_per_row * p + byteofs] & checkmask)
				newbyte |= setbit;
			setbit <<= 1;
			}
		*destptr++= newbyte;
		if ((checkmask >>= 1) == 0) checkmask=0x80;
		}
	}
delete[] bmHeader->raw_data;
bmHeader->raw_data = new_data;
bmHeader->nType = TYPE_PBM;
return IFF_NO_ERROR;
}

//------------------------------------------------------------------------------

#define INDEX_TO_15BPP(i) ((int16_t)((((palptr[(i)].r/2)&31)<<10)+(((palptr[(i)].g/2)&31)<<5)+((palptr[(i)].b/2)&31)))

int32_t CIFF::ConvertRgb15 (CBitmap *pBm, tIFFBitmapHeader *bmHeader)
{
	uint16_t *new_data;
	int32_t x, y;
	int32_t newptr = 0;
	tPalEntry *palptr;

palptr = bmHeader->palette;
new_data = NEW uint16_t [pBm->FrameSize () * 2];
if (new_data == NULL)
	return IFF_NO_MEM;
for (y=0; y < pBm->Height (); y++) {
	for (x=0; x<bmHeader->w; x++)
		new_data[newptr++] = INDEX_TO_15BPP(bmHeader->raw_data[y*bmHeader->w+x]);
	}
pBm->DestroyBuffer ();				//get rid of old-style data
pBm->SetBuffer (reinterpret_cast<uint8_t*> (new_data));			//..ccAnd point to new data
pBm->SetRowSize (pBm->RowSize () * 2);				//two bytes per row
return IFF_NO_ERROR;
}

//------------------------------------------------------------------------------
//read in a entire file into a fake file structure
int32_t CIFF::Open (const char *cfname)
{
	CFile cf;
	int32_t	ret;

Data() = NULL;
if (!cf.Open (cfname, gameFolders.game.szData [0], "rb", gameStates.app.bD1Mission))
	return IFF_NO_FILE;
SetLen ((int32_t) cf.Length ());
Data() = NEW uint8_t [Len ()];
if (cf.Read (Data(), 1, Len()) < (size_t) Len())
	ret = IFF_READ_ERROR;
else
	ret = IFF_NO_ERROR;
SetPos (0);
cf.Close();
return ret;
}

//------------------------------------------------------------------------------

void CIFF::Close (void)
{
if (Data ()) {
	delete[] Data ();
	Data () = NULL;
	}
SetPos (0);
SetLen (0);
}

//------------------------------------------------------------------------------
//copy an iff header structure to a CBitmap structure
void CIFF::CopyIffToBitmap (CBitmap *pBm, tIFFBitmapHeader *bmHeader)
{
pBm->DestroyBuffer ();
pBm->SetFlags (0);
pBm->Init (bmHeader->nType, 0, 0, bmHeader->w, bmHeader->h, 1, bmHeader->raw_data);
bmHeader->raw_data = NULL;
}

//------------------------------------------------------------------------------
//if pBm->Buffer () is set, use it (making sure w & h are correct), else
//allocate the memory
int32_t CIFF::ParseBitmap (CBitmap *pBm, int32_t bitmapType, CBitmap *pPrevBm)
{
	tIFFBitmapHeader	bmHeader;
	int32_t					ret, sig, formLen;
	int32_t					formType;

memset (&bmHeader, 0, sizeof (bmHeader));
if ((bmHeader.raw_data = pBm->Buffer ())) {
	bmHeader.w = pBm->Width ();
	bmHeader.h = pBm->Height ();
	}
sig = GetSig ();
if (sig != form_sig)
	return IFF_NOT_IFF;
formLen = GetLong ();
formType = GetSig ();
if (formType == anim_sig)
	ret = IFF_FORM_ANIM;
else if ((formType == pbm_sig) || (formType == ilbm_sig))
	ret = Parse (formType, &bmHeader, formLen, pPrevBm);
else
	ret = IFF_UNKNOWN_FORM;
if (ret != IFF_NO_ERROR) {		//got an error parsing
	if (bmHeader.raw_data) 
		delete[] bmHeader.raw_data;
	return ret;
	}
//If IFF file is ILBM, convert to PPB
if (bmHeader.nType == TYPE_ILBM) {
	ret = ConvertToPBM (&bmHeader);
	if (ret != IFF_NO_ERROR)
		return ret;
	}
//Copy data from tIFFBitmapHeader structure into CBitmap structure
CopyIffToBitmap (pBm, &bmHeader);
if ((bmHeader.masking != mskHasTransparentColor) || (bmHeader.transparentColor < 0))
	pBm->SetPalette (paletteManager.Add (reinterpret_cast<uint8_t*> (&bmHeader.palette)));
else
	pBm->SetPalette (paletteManager.Add (reinterpret_cast<uint8_t*> (&bmHeader.palette), bmHeader.transparentColor, -1), bmHeader.transparentColor, -1);
//Now do post-process if required
if (bitmapType == BM_RGB15)
	ret = ConvertRgb15 (pBm, &bmHeader);
return ret;
}

//------------------------------------------------------------------------------
//returns error codes - see IFF.H.  see GR[HA] for bitmapType
int32_t CIFF::ReadBitmap (const char *cfname, CBitmap *pBm, int32_t bitmapType)
{
#if 1
	char* p, fn [FILENAME_LEN];

strcpy (fn, cfname);

if ((p = strstr (fn, ".bbm")))
	*p = '\0';
if (ReadHiresBitmap (&gameData.endLevelData.terrain.bmInstance, fn, 0x7FFFFFFF, 1) > 0)
	return IFF_NO_ERROR;
#endif
int32_t ret = Open (cfname);		//read in entire file
if (ret == IFF_NO_ERROR) {
	ret = ParseBitmap (pBm, bitmapType, NULL);
	}
Close ();
return ret;
}

//------------------------------------------------------------------------------
//like iff_read_bitmap(), but reads into a bitmap that already exists, 
//without allocating memory for the bitmap.
int32_t CIFF::ReplaceBitmap (const char *cfname, CBitmap *pBm)
{
	int32_t ret;			//return code

ret = Open (cfname);		//read in entire file
if (ret == IFF_NO_ERROR)
	ret = ParseBitmap (pBm, pBm->Mode (), NULL);
Close();
return ret;
}

#define BMHD_SIZE 20

//------------------------------------------------------------------------------

int32_t CIFF::WriteHeader (FILE *fp, tIFFBitmapHeader *bitmap_header)
{
PutSig(bmhd_sig, fp);
PutLong ((int32_t) BMHD_SIZE, fp);
PutWord (bitmap_header->w, fp);
PutWord (bitmap_header->h, fp);
PutWord (bitmap_header->x, fp);
PutWord (bitmap_header->y, fp);
PutByte (bitmap_header->nplanes, fp);
PutByte (bitmap_header->masking, fp);
PutByte (bitmap_header->compression, fp);
PutByte (0, fp);	/* pad */
PutWord (bitmap_header->transparentColor, fp);
PutByte (bitmap_header->xaspect, fp);
PutByte (bitmap_header->yaspect, fp);
PutWord (bitmap_header->pagewidth, fp);
PutWord (bitmap_header->pageheight, fp);
return IFF_NO_ERROR;
}

//------------------------------------------------------------------------------

int32_t CIFF::WritePalette (FILE *fp, tIFFBitmapHeader *bitmap_header)
{
	int32_t	i;

	int32_t n_colors = 1 << bitmap_header->nplanes;

PutSig(cmap_sig, fp);
PutLong(3 * n_colors, fp);
for (i=0; i<256; i++) {
	uint8_t r, g, b;
	r = bitmap_header->palette [i].r * 4 + (bitmap_header->palette [i].r?3:0);
	g = bitmap_header->palette [i].g * 4 + (bitmap_header->palette [i].g?3:0);
	b = bitmap_header->palette [i].b * 4 + (bitmap_header->palette [i].b?3:0);
	fputc(r, fp);
	fputc(g, fp);
	fputc(b, fp);
	}
return IFF_NO_ERROR;
}

//------------------------------------------------------------------------------

int32_t CIFF::RLESpan (uint8_t *dest, uint8_t *src, int32_t len)
{
	int32_t n, lit_cnt, rep_cnt;
	uint8_t last, *cnt_ptr, *dptr;

cnt_ptr = 0;
dptr = dest;
last = src[0]; 
lit_cnt = 1;
for (n = 1; n < len; n++) {
	if (src [n] != last) 
		n--;
	else {
		rep_cnt = 2;
		n++;
		while ((n < len) && (rep_cnt < 128) && (src [n] == last)) {
			n++; 
			rep_cnt++;
			}
		if (rep_cnt > 2 || lit_cnt < 2) {
			if (lit_cnt > 1) {
				*cnt_ptr = lit_cnt-2; 
				--dptr;
				}		//save old lit count
			*dptr++= -(rep_cnt-1);
			*dptr++= last;
			last = src [n];
			lit_cnt = (n < len) ? 1 : 0;
			continue;		//go to next char
			} 
		}
	if (lit_cnt==1) {
		cnt_ptr = dptr++;		//save place for count
		*dptr++=last;			//store first char
		}
	*dptr++= last = src[n];
	if (lit_cnt == 127) {
		*cnt_ptr = lit_cnt;
		lit_cnt = 1;
		last = src[++n];
		}
	else 
		lit_cnt++;
	}

if (lit_cnt==1) {
	*dptr++= 0;
	*dptr++=last;			//store first char
	}
else if (lit_cnt > 1)
	*cnt_ptr = lit_cnt-1;
return (int32_t) (dptr - dest);
}

#define EVEN(a) ((a+1)&0xfffffffel)

//------------------------------------------------------------------------------

//returns length of chunk
int32_t CIFF::WriteBody (FILE *fp, tIFFBitmapHeader *bitmap_header, int32_t bCompression)
{
	int32_t w=bitmap_header->w, h=bitmap_header->h;
	int32_t y, odd=w&1;
	int32_t len = EVEN(w) * h, newlen, total_len=0;
	uint8_t *p=bitmap_header->raw_data, *new_span;
	int32_t save_pos;

PutSig(body_sig, fp);
save_pos = ftell(fp);
PutLong(len, fp);
new_span = NEW uint8_t [bitmap_header->w + (bitmap_header->w/128+2)*2];
if (new_span == NULL) 
	return IFF_NO_MEM;
for (y=bitmap_header->h;y--;) {
	if (bCompression) {
		total_len += newlen = RLESpan (new_span, p, bitmap_header->w+odd);
		fwrite(new_span, newlen, 1, fp);
		}
	else
		fwrite(p, bitmap_header->w+odd, 1, fp);
	p+=bitmap_header->row_size;	//bitmap_header->w;
	}
if (bCompression) {		//write actual data length
	fseek(fp, save_pos, SEEK_SET);
	PutLong(total_len, fp);
	fseek(fp, total_len, SEEK_CUR);
	if (total_len&1) fputc(0, fp);		//pad to even
	}
delete[] new_span;
return ((bCompression) ? (EVEN(total_len)+8) : (len+8));
}


//------------------------------------------------------------------------------

int32_t CIFF::WritePbm (FILE *fp, tIFFBitmapHeader *bitmap_header, int32_t bCompression)			/* writes a pbm iff file */
{
	int32_t ret;
	int32_t raw_size = EVEN(bitmap_header->w) * bitmap_header->h;
	int32_t body_size, tiny_size, pbm_size = 4 + BMHD_SIZE + 8 + EVEN(raw_size) + sizeof (tPalEntry) * int32_t (1 << bitmap_header->nplanes) + 8;

////printf("write_pbm\n");

PutSig(form_sig, fp);
int32_t save_pos = ftell(fp);
PutLong(pbm_size+8, fp);
PutSig(pbm_sig, fp);
ret = WriteHeader (fp, bitmap_header);
if (ret != IFF_NO_ERROR) 
	return ret;
ret = WritePalette(fp, bitmap_header);
if (ret != IFF_NO_ERROR) 
	return ret;
tiny_size = 0;
body_size = WriteBody(fp, bitmap_header, bCompression);
pbm_size = 4 + BMHD_SIZE + body_size + tiny_size + sizeof (tPalEntry) * int32_t (1 << bitmap_header->nplanes) + 8;
fseek(fp, save_pos, SEEK_SET);
PutLong(pbm_size+8, fp);
fseek(fp, pbm_size+8, SEEK_CUR);
return ret;
}

//------------------------------------------------------------------------------
//writes an IFF file from a CBitmap structure. writes palette if not null
//returns error codes - see IFF.H.
int32_t CIFF::WriteBitmap (const char *cfname, CBitmap *pBm, uint8_t *palette)
{
	FILE					*fp;
	tIFFBitmapHeader	bmHeader;
	int32_t					ret;
	int32_t					bCompression;

if (pBm->Mode () == BM_RGB15) return IFF_BAD_BM_TYPE;
#if COMPRESS
	bCompression = (pBm->Width ()>=MIN_COMPRESS_WIDTH);
#else
	bCompression = 0;
#endif
//fill in values in bmHeader
bmHeader.x = bmHeader.y = 0;
bmHeader.w = pBm->Width ();
bmHeader.h = pBm->Height ();
bmHeader.nType = TYPE_PBM;
bmHeader.transparentColor = m_transparentColor;
bmHeader.pagewidth = pBm->Width ();	//I don't think it matters what I write
bmHeader.pageheight = pBm->Height ();
bmHeader.nplanes = 8;
bmHeader.masking = mskNone;
if (m_hasTransparency) {
	 bmHeader.masking |= mskHasTransparentColor;
	}
bmHeader.compression = (bCompression?cmpByteRun1:cmpNone);
bmHeader.xaspect = bmHeader.yaspect = 1;	//I don't think it matters what I write
bmHeader.raw_data = pBm->Buffer ();
bmHeader.row_size = pBm->RowSize ();
if (palette) 
	memcpy(&bmHeader.palette, palette, 256*3);
//open file and write
if (!(fp = fopen(cfname, "wb"))) 
	ret=IFF_NO_FILE;
else
	ret = WritePbm (fp, &bmHeader, bCompression);
fclose(fp);
return ret;
}

//------------------------------------------------------------------------------
//read in many brushes.  fills in array of pointers, and n_bitmaps.
//returns iff error codes
int32_t CIFF::ReadAnimBrush (const char *cfname, CBitmap **bm_list, int32_t max_bitmaps, int32_t *n_bitmaps)
{
	int32_t					ret;			//return code
	int32_t					sig, formLen;
	int32_t					formType;

*n_bitmaps=0;
ret = Open (cfname);		//read in entire file
if (ret != IFF_NO_ERROR) 
	goto done;
sig=GetSig();
formLen = GetLong();
if (sig != form_sig) {
	ret = IFF_NOT_IFF;
	goto done;
	}
formType = GetSig();
if ((formType == pbm_sig) || (formType == ilbm_sig))
	ret = IFF_FORM_BITMAP;
else if (formType == anim_sig) {
	int32_t anim_end = Pos() + formLen - 4;
		while (Pos() < anim_end && *n_bitmaps < max_bitmaps) {
			CBitmap *pPrevBm;
			pPrevBm = *n_bitmaps>0?bm_list[*n_bitmaps-1]:NULL;
			bm_list [*n_bitmaps] = NEW CBitmap;
			bm_list [*n_bitmaps]->SetBuffer (NULL);
			ret = ParseBitmap (bm_list[*n_bitmaps], formType, pPrevBm);
			if (ret != IFF_NO_ERROR)
				goto done;
			(*n_bitmaps)++;
			}
		if (Pos() < anim_end)	//ran out of room
			ret = IFF_TOO_MANY_BMS;
		}
	else
		ret = IFF_UNKNOWN_FORM;

done:

Close();
return ret;
}

//------------------------------------------------------------------------------
//text for error messges
const char error_messages[] = {
	"No error.\0"
	"Not enough mem for loading or processing bitmap.\0"
	"IFF file has unknown FORM nType.\0"
	"Not an IFF file.\0"
	"Cannot open file.\0"
	"Tried to save invalid nType, like BM_RGB15.\0"
	"Bad data in file.\0"
	"ANIM file cannot be loaded with Normal bitmap loader.\0"
	"Normal bitmap file cannot be loaded with anim loader.\0"
	"Array not big enough on anim brush read.\0"
	"Unknown mask nType in bitmap header.\0"
	"Error reading file.\0"
	"No exit found.\0"
};


//------------------------------------------------------------------------------
//function to return pointer to error message
const char *CIFF::ErrorMsg (int32_t nError)
{
const char *p = error_messages;

while (nError--) {
	if (!p) 
		return NULL;
	p += (int32_t) strlen(p)+1;
	}
return p;
}

//------------------------------------------------------------------------------
//eof
