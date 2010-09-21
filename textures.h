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

	void Read (CFileManager& fp) {
		if (m_nVersion == 1) {
			nId = fp.ReadInt32 ();
			nVersion = fp.ReadInt32 ();
			nTextures = fp.ReadInt32 ();
			}
		else {
			nTextures = fp.ReadInt32 ();
			nSounds = fp.ReadInt32 ();
			}
		}

	void Write (CFileManager& fp) {
		if (m_nVersion == 1) {
			 fp.Write (nId);
			 fp.Write (nVersion);
			 fp.Write (nTextures);
			}
		else {
			fp.Write (nTextures);
			fp.Write (nSounds);
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
			height %= 256;
			}
		width %= 256;
		}

	void Setup (int nVersion, ushort w = 0, ushort h = 0, byte f = 0, uint o = 0) {
		m_nVersion = nVersion;
		width = w;
		height = h;
		flags = f;
		offset = o;
		dflags = 0;
		avgColor = 0;
		}

	void Read (CFileManager& fp, int nVersion = -1) {
		if (nVersion >= 0)
			m_nVersion = nVersion;
		fp.ReadBytes (name, sizeof (name));
		dflags = fp.ReadByte ();
		width = (ushort) fp.ReadByte ();
		height = (ushort) fp.ReadByte ();
		if (m_nVersion == 1)
			whExtra = fp.ReadByte ();
		else {
			name [7] = 0;
			whExtra = 0;
			}
		flags = fp.ReadByte ();
		avgColor = fp.ReadByte ();
		offset = fp.ReadUInt32 ();
		Decode ();
		}

	void Write (CFileManager& fp) {
		Encode ();
		fp.WriteBytes (name, sizeof (name));
		fp.Write (dflags);
		fp.WriteByte ((byte) width);
		fp.WriteByte ((byte) height);
		if (m_nVersion == 1)
			fp.Write (whExtra);
		fp.Write (flags);
		fp.Write (avgColor);
		fp.Write (offset);
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
	byte r, g, b, a;
} tRGBA;


typedef struct tBGRA {
	byte b, g, r, a;
} tBGRA;


typedef struct tABGR {
	byte a, b, g, r;
} tABGR;


class CBGR {
	public:
		byte r, g, b;

		CBGR (byte red = 0, byte green = 0, byte blue = 0)
			: r (red), g (green), b (blue)
			{}
};

class CBGRA : public CBGR {
	public:
		byte a;

		CBGRA& operator= (const tRGBA& other) {
			r = other.r;
			g = other.g;
			b = other.b;
			a = other.a;
			return *this;
			}

		CBGRA& operator= (const CBGR& other) {
			r = other.r;
			g = other.g;
			b = other.b;
			a = 255;
			return *this;
			}

		CBGRA (byte red = 0, byte green = 0, byte blue = 0, byte alpha = 255)
			: CBGR (red, green, blue), a (alpha) 
			{}
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tTexture {
	byte*		bmIndex;
	CBGRA*	bmData;
	uint		width, height, size;
	bool		bCustom, bExtData, bFrame, bUsed, bValid;
	byte		nFormat;	// 0: Bitmap, 1: TGA (RGB)
} tTexture;

class CTexture {
	public:
		tTexture	m_info;

		CTexture (CBGRA* dataP = null, byte* indexP = null) {
			Clear ();
			m_info.bmData = dataP;
			m_info.bmIndex = indexP;
			m_info.bExtData = (dataP != null);
			}

		void Release (void);

		~CTexture() { Release (); }

		bool Allocate (int nSize, int nTexture);

		int Load (CFileManager& fp, short nTexture, int nVersion = -1);

		void Load (CFileManager& fp, CPigTexture& info);

		double Scale (short index = -1);

		void Read (CFileManager& fp, int version = 0, bool bFlag = false) { };

		void Write (CFileManager& fp, int version = 0, bool bFlag = false) {};

		virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};

//------------------------------------------------------------------------

class CExtraTexture : public CTexture {
public:
	CExtraTexture*	m_next;
	ushort			m_index;
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//int textureManager.Define (short nBaseTex,short nOvlTex, CTexture *pDestTx, int x0, int y0);
void RgbFromIndex (int nIndex, PALETTEENTRY& rgb);

bool PaintTexture (CWnd *wndP, int bkColor = -1, 
						 int nSegment = -1, int nSide = -1, int texture1 = -1, int texture2 = 0,
						 int xOffset = 0, int yOffset = 0);

bool TGA2Bitmap (tRGBA *pTGA, byte *pBM, int nWidth, int nHeight);

//------------------------------------------------------------------------

#endif //__textures_h