#ifndef __textures_h
#define __textures_h

#include "Define.h"
#include "Types.h"
#include "carray.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

/* pig file types */
typedef struct {
  int nTextures;
  int nSounds;
} PIG_HEADER_D1;

typedef struct {
	char name[8];
	byte dflags; /* this is only important for large bitmaps like the cockpit */
	byte width;
	byte height;
	byte flags;
	byte avgColor;
	uint offset;
} PIG_TEXTURE_D1;

typedef struct {
	int nId;
	int nVersion;
	int nTextures;
} PIG_HEADER_D2;

typedef struct {
	char name[8];
	byte dflags;  // bits 0-5 anim frame num, bit 6 abm flag
	byte width;   // low 8 bits here, 4 more bits in pad
	byte height;   // low 8 bits here, 4 more bits in pad
	byte whExtra;     // bits 0-3 xsize, bits 4-7 ysize
	byte flags;   // see BM_FLAG_XXX in define.h
	byte avgColor;   // average color
	uint offset;
} PIG_TEXTURE_D2;

typedef struct {
	byte unknown[20];
} PIG_SOUND;

typedef struct {
	char name[8];
	short number;
} TEXTURE;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CPigHeader {
public:
	int nId;
	int nVersion;
	int nTextures;
	int nSounds;

	int m_nVersion;

	inline int Size (void) { return m_nVersion ? sizeof (PIG_HEADER_D2) : sizeof (PIG_HEADER_D1); }

	CPigHeader (int nVersion = 1) {
		m_nVersion = nVersion;
		}

	void Read (FILE* fp) {
		if (m_nVersion == 1) {
			nId = ReadInt32 (fp);
			nVersion = ReadInt32 (fp);
			nTextures = ReadInt32 (fp);
			}
		else {
			nTextures = ReadInt32 (fp);
			nSounds = ReadInt32 (fp);
			}
		}
};

//------------------------------------------------------------------------

typedef struct tPigHeader {
	char		name[8];
	byte		dflags;  // bits 0-5 anim frame num, bit 6 abm flag
	ushort	width;   // low 8 bits here, 4 more bits in pad
	ushort	height;   // low 8 bits here, 4 more bits in pad
	byte		whExtra;     // bits 0-3 xsize, bits 4-7 ysize
	byte		flags;   // see BM_FLAG_XXX in define.h
	byte		avgColor;   // average color
	uint		offset;
	} tPigTexture;

class CPigTexture : public tPigTexture {
public:
	int m_nVersion;

	CPigTexture (int nVersion = 1) {
		m_nVersion = nVersion;
		}

	inline int Size (void) { return m_nVersion ? sizeof (PIG_TEXTURE_D2) : sizeof (PIG_TEXTURE_D1); }

	void Setup (int nVersion, ushort w = 0, ushort h = 0, byte f = 0, uint o = 0) {
		m_nVersion = nVersion;
		width = w;
		height = h;
		flags = f;
		offset = o;
		dflags = 0;
		avgColor = 0;
		}

	void Decode (void) {
		width += (ushort) (whExtra % 16) * 256;
		if ((flags & 0x80) && (width > 256))
			height *= width;
		else
			height += (ushort) (whExtra / 16) * 256;
		}

	void Encode (void) {
		if ((flags & 0x80) && (width > 256)) {
			whExtra = width / 256;
			height /= width;
			}
		else {
			whExtra = (width / 256) + ((height / 256) * 16);
			width &= 256;
			height %= 256;
			}
		}

	void Read (FILE* fp) {
		ReadBytes (name, sizeof (name), fp);
		dflags = ReadUInt8 (fp);
		width = (ushort) ReadUInt8 (fp);
		height = (ushort) ReadUInt8 (fp);
		if (m_nVersion == 0)
			name [7] = 0;
		else 
			whExtra = ReadUInt8 (fp);
		flags = ReadUInt8 (fp);
		avgColor = ReadUInt8 (fp);
		offset = ReadUInt32 (fp);
		Decode ();
		}

	void Write (FILE* fp) {
		Encode ();
		WriteBytes (name, sizeof (name), fp);
		WriteUInt8 (dflags, fp);
		WriteUInt8 ((byte) width, fp);
		WriteUInt8 ((byte) height, fp);
		if (m_nVersion == 1)
			WriteUInt8 (whExtra, fp);
		WriteUInt8 (flags, fp);
		WriteUInt8 (avgColor, fp);
		}

	inline int BufSize (void) { return (int) width * (int) height; }
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

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

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tTexture {
	byte*		bmDataP;
	tRGBA*	tgaDataP;
	uint		width, height, size;
	bool		bModified, bExtData, bValid;
	byte		nFormat;	// 0: Bitmap, 1: TGA (RGB)
} tTexture;

class CTexture : public CGameItem {
public:
	tTexture	m_info;

	CTexture (byte *dataP = NULL) {
		Clear ();
		m_info.bmDataP = dataP;
		m_info.bExtData = (dataP != NULL);
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

	bool Allocate (int nSize, int nTexture);
	int Load (short nTexture, int nVersion = -1, FILE* fp = NULL);
	void Load (FILE* fp, CPigTexture& info);
	double Scale (short index = -1);

	virtual CGameItem* Next (void) { return this + 1; }
	virtual int Read (FILE* fp, int version = 0, bool bFlag = false) { return 1; };
	virtual void Write (FILE* fp, int version = 0, bool bFlag = false) {};
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#if USE_DYN_ARRAYS

typedef CDynamicArray< CTexture > textureList;

#else

typedef CTexture* textureList [2];

#endif

class CTextureManager {
public:
	uint nTextures [2];
	textureList textures [2];
	ushort* index [2];
	uint nOffsets [2];
	CPigTexture* info [2];
	CPigHeader header [2];
	byte bmBuf [512 * 512 * 32 * 4];	// max texture size: 512x512, RGBA, 32 frames

	inline CTexture* Textures (int nVersion, int nTexture = 0) { return &textures [nVersion][nTexture]; }

	int MaxTextures (int nVersion = -1);
	int LoadIndex (int nVersion);
	void LoadTextures (int nVersion);
	CPigTexture& LoadInfo (FILE* fp, int nVersion, short nTexture);
	bool Check (int nTexture);
	void Load (ushort nBaseTex, ushort nOvlTex);
	int Define (short nBaseTex, short nOvlTex, CTexture* pDestTex, int x0, int y0);
	void Release (bool bDeleteModified = true);
	bool HasCustomTextures (void);
	int CountCustomTextures (void);
	FILE* OpenPigFile (int nVersion);

	inline bool HaveInfo (int nVersion) { return info [nVersion] != NULL; }

	CTextureManager() {}
	
	void Setup (void);

	~CTextureManager() {
		for (int i = 0; i < 2; i++) {
#if USE_DYN_ARRAYS
			//textures [i].Destroy ();
#else
			if (textures [i])
				delete textures [i];
#endif
			if (index [i])
				delete index [i];
			if (info [i])
				delete info [i];
			}
		}
};

extern CTextureManager textureManager;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//int textureManager.Define (short nBaseTex,short nOvlTex, CTexture *pDestTx, int x0, int y0);
void RgbFromIndex (int nIndex, PALETTEENTRY *pRGB);
BITMAPINFO *MakeBitmap(void);
//BOOL HasCustomTextures ();
//int CountCustomTextures ();
int ReadPog(FILE *file, uint nFileSize = 0xFFFFFFFF);
int CreatePog (FILE *file);
bool PaintTexture (CWnd *pWnd, int bkColor = -1, 
						 int nSegment = -1, int nSide = -1, int texture1 = -1, int texture2 = 0,
						 int xOffset = 0, int yOffset = 0);
bool TGA2Bitmap (tRGBA *pTGA, byte *pBM, int nWidth, int nHeight);

//------------------------------------------------------------------------

#endif //__textures_h