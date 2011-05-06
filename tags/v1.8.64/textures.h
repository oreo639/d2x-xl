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

	void Read (CFileManager* fp, int nVersion = -1) {
		if (nVersion >= 0)
			m_nVersion = nVersion;
		fp->ReadBytes (name, sizeof (name));
		dflags = fp->ReadByte ();
		width = (ushort) fp->ReadByte ();
		height = (ushort) fp->ReadByte ();
		if (m_nVersion == 1)
			whExtra = fp->ReadByte ();
		else {
			name [7] = 0;
			whExtra = 0;
			}
		flags = fp->ReadByte ();
		avgColor = fp->ReadByte ();
		offset = fp->ReadUInt32 ();
		Decode ();
		}

	void Write (CFileManager* fp) {
		Encode ();
		fp->WriteBytes (name, sizeof (name));
		fp->Write (dflags);
		fp->WriteByte ((byte) width);
		fp->WriteByte ((byte) height);
		if (m_nVersion == 1)
			fp->Write (whExtra);
		fp->Write (flags);
		fp->Write (avgColor);
		fp->Write (offset);
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
		byte b, g, r;

		CBGR (byte red = 0, byte green = 0, byte blue = 0)
			: r (red), g (green), b (blue)
			{}

		inline COLORREF ColorRef (void) { return RGB (r, g, b); }

		inline uint Delta (CBGR& other) {
			return (uint) (Sqr ((int) r - (int) other.r) + Sqr ((int) g - (int) other.g) + Sqr ((int) b - (int) other.b));
			}

		inline const bool operator== (const CBGR& other) const { return (r == other.r) && (g == other.g) && (b == other.b); }

	private:
		inline int Sqr (int i) { return i * i; }
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

		CBGRA& operator= (const tBGRA& other) {
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

		CBGRA& operator= (COLORREF color) {
			r = GetRValue (color);
			g = GetGValue (color);
			b = GetBValue (color);
			a = 255;
			return *this;
			}

		CBGRA& operator= (const RGBQUAD& color) {
			r = color.rgbRed;
			g = color.rgbGreen;
			b = color.rgbBlue;
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

class CUV {
	public:
		int u, v;
};

typedef struct tTexture {
	uint		width, height, offset;
	int		id; 
	bool		bCustom, bExtData, bFrame, bUsed, bValid, bTransparent;
	byte		nFormat;	// 0: Bitmap, 1: TGA (RGB)
} tTexture;

class CTexture {
	public:
		tTexture	m_info;
		CBGRA*	m_data;
		CBGRA*	m_override;

		CTexture (CBGRA* dataP = null) : m_override (null), m_data (dataP) {
			Clear ();
			m_info.bExtData = (dataP != null);
			}

		void Release (void);

		~CTexture() { Release (); }

		bool Allocate (int nSize);

		int Load (CFileManager& fp, short nTexture, int nVersion = -1);

		void Load (CFileManager& fp, CPigTexture& info);

		int Reload (void);

		double Scale (short index = -1);

		void Read (CFileManager& fp, int version = 0, bool bFlag = false) { };

		void Write (CFileManager& fp, int version = 0, bool bFlag = false) {};

		virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }

		void ComputeIndex (byte* bmIndex);

		inline bool Transparent (void) { return m_info.bTransparent; }

		inline uint Width (void) { return m_info.width; }

		inline uint Height (void) { return m_info.height; }

		inline uint Size (void) { return Width () * Height (); }

		inline uint BufSize (void) { return Width () * Height () * sizeof (CBGRA); }

		inline CBGRA* Buffer (uint i = 0) { return (m_override == null) ? &m_data [i] : &m_override [i]; }

		inline bool Copy (CTexture& src) {
			if (!Allocate (src.Size ()))
				return false;
			memcpy (Buffer (), src.Buffer (), src.BufSize ());
			memcpy (&m_info, &src.m_info, sizeof (m_info));
			m_info.bValid = true;
			return true;
			}

		CBGRA& operator[] (uint i) { return *Buffer (i); }

		CBGRA& operator[] (CUV uv) { return *Buffer ((Height () - uv.v - 1) * Width () + uv.u); }
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

//int textureManager.BlendTextures (short nBaseTex,short nOvlTex, CTexture *pDestTx, int x0, int y0);
void RgbFromIndex (int nIndex, PALETTEENTRY& rgb);

bool PaintTexture (CWnd *wndP, int bkColor = -1, 
						 int nSegment = -1, int nSide = -1, int texture1 = -1, int texture2 = 0,
						 int xOffset = 0, int yOffset = 0);

bool TGA2Bitmap (tRGBA *pTGA, byte *pBM, int nWidth, int nHeight);

//------------------------------------------------------------------------

#endif //__textures_h