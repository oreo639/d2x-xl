#ifndef __textures_h
#define __textures_h

#include "Types.h"

extern UINT8 bmBuf [512 * 512 * 32 * 4];

typedef struct {
    byte  identSize;          // size of ID field that follows 18 byte header (0 usually)
    byte  colorMapType;      // type of colour map 0=none, 1=has palette
    byte  imageType;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    INT16 colorMapStart;     // first colour map entry in palette
    INT16 colorMapLength;    // number of colours in palette
    byte  colorMapBits;      // number of bits per palette entry 15,16,24,32

    INT16 xStart;             // image x origin
    INT16 yStart;             // image y origin
    INT16 width;              // image width in pixels
    INT16 height;             // image height in pixels
    byte  bits;               // image bits per pixel 8,16,24,32
    byte  descriptor;         // image descriptor bits (vh flip bits)
} tTgaHeader;


typedef struct tRGBA {
	unsigned char	r, g, b, a;
} tRGBA;


typedef struct tBGRA {
	unsigned char	b, g, r, a;
} tBGRA;

typedef struct tABGR {
	unsigned char	a, b, g, r;
} tABGR;

typedef struct tBGR {
	unsigned char	r, g, b;
} tBGR;

typedef struct tTexture {
	UINT8*	bmDataP;
	tRGBA*	tgaDataP;
	UINT32	width, height, size;
	bool		bModified, bExtData, bValid;
	UINT8		nFormat;	// 0: Bitmap, 1: TGA (RGB)
} tTexture;

class CTexture : public CGameItem {
public:
	tTexture	m_info;

	CTexture (UINT8 *dataP = NULL) {
		Clear ();
		m_info.bmDataP = dataP;
		m_info.bExtData = dataP != NULL;
		if (dataP != NULL)
			m_info.bExtData = true;
		}

	inline void Release (void) {
		if (!m_info.bExtData) {
			if (m_info.bmDataP)
				delete m_info.bmDataP;
			if (m_info.tgaDataP)
				delete m_info.tgaDataP;
			}
		Clear ();
		}

	~CTexture() { Release (); }

	INT32 Read (INT16 index);
	double Scale (INT16 index = -1);

	virtual CGameItem* Next (void) { return this + 1; }
	virtual INT32 Read (FILE* fp, INT32 version = 0, bool bFlag = false) { return 1; };
	virtual void Write (FILE* fp, INT32 version = 0, bool bFlag = false) {};
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};


INT32 DefineTexture(INT16 nBaseTex,INT16 nOvlTex, CTexture *pDestTx, INT32 x0, INT32 y0);
void RgbFromIndex (INT32 nIndex, PALETTEENTRY *pRGB);
BITMAPINFO *MakeBitmap(void);
BOOL HasCustomTextures ();
INT32 CountCustomTextures ();
void FreeTextureHandles(bool bDeleteModified = true);
INT32 ReadPog(FILE *file, UINT32 nFileSize = 0xFFFFFFFF);
INT32 CreatePog (FILE *file);
bool PaintTexture (CWnd *pWnd, INT32 bkColor = -1, 
						 INT32 nSegment = -1, INT32 nSide = -1, INT32 texture1 = -1, INT32 texture2 = 0,
						 INT32 xOffset = 0, INT32 yOffset = 0);
bool TGA2Bitmap (tRGBA *pTGA, UINT8 *pBM, INT32 nWidth, INT32 nHeight);

#endif //__textures_h