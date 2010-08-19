#ifndef __textures_h
#define __textures_h

extern UINT8 bmBuf [2048 * 2048 * 4];

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

class CDTexture
{
public:
	UINT8		*m_pDataBM;
	tRGBA		*m_pDataTGA;
	UINT32	m_width, m_height, m_size;
	BOOLEAN	m_bModified, m_bExtData, m_bValid;
	UINT8		m_nFormat;	// 0: Bitmap, 1: TGA (RGB)

	CDTexture(UINT8 *pData = NULL) 
		: m_pDataBM (pData) 
		{ m_pDataTGA = NULL, m_nFormat = 0, m_bValid = m_bModified = FALSE, m_bExtData = (pData != NULL); }
	~CDTexture() {
		if (!m_bExtData) {
			delete m_pDataBM;
			if (m_pDataTGA)
				delete m_pDataTGA;
			}
		}

	INT32 Read (INT16 index);
	double Scale (INT16 index = -1);
};


INT32 DefineTexture(INT16 nBaseTex,INT16 nOvlTex, CDTexture *pDestTx, INT32 x0, INT32 y0);
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

extern CDTexture pTextures [2][MAX_D2_TEXTURES];

#endif //__textures_h