#ifndef __textures_h
#define __textures_h

#include "Types.h"

extern byte bmBuf [512 * 512 * 32 * 4];

typedef struct {
    byte  identSize;          // size of ID field that follows 18 byte header (0 usually)
    byte  colorMapType;      // type of colour map 0=none, 1=has palette
    byte  imageType;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    short colorMapStart;     // first colour map entry in palette
    short colorMapLength;    // number of colours in palette
    byte  colorMapBits;      // number of bits per palette entry 15,16,24,32

    short xStart;             // image x origin
    short yStart;             // image y origin
    short width;              // image width in pixels
    short height;             // image height in pixels
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
	byte*	bmDataP;
	tRGBA*	tgaDataP;
	uint	width, height, size;
	bool		bModified, bExtData, bValid;
	byte		nFormat;	// 0: Bitmap, 1: TGA (RGB)
} tTexture;

class CTexture : public CGameItem {
public:
	tTexture	m_info;

	CTexture (byte *dataP = NULL) {
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

	int Read (short index);
	double Scale (short index = -1);

	virtual CGameItem* Next (void) { return this + 1; }
	virtual int Read (FILE* fp, int version = 0, bool bFlag = false) { return 1; };
	virtual void Write (FILE* fp, int version = 0, bool bFlag = false) {};
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};


int DefineTexture(short nBaseTex,short nOvlTex, CTexture *pDestTx, int x0, int y0);
void RgbFromIndex (int nIndex, PALETTEENTRY *pRGB);
BITMAPINFO *MakeBitmap(void);
BOOL HasCustomTextures ();
int CountCustomTextures ();
void FreeTextureHandles(bool bDeleteModified = true);
int ReadPog(FILE *file, uint nFileSize = 0xFFFFFFFF);
int CreatePog (FILE *file);
bool PaintTexture (CWnd *pWnd, int bkColor = -1, 
						 int nSegment = -1, int nSide = -1, int texture1 = -1, int texture2 = 0,
						 int xOffset = 0, int yOffset = 0);
bool TGA2Bitmap (tRGBA *pTGA, byte *pBM, int nWidth, int nHeight);

#endif //__textures_h