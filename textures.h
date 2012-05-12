#ifndef __textures_h
#define __textures_h

#include "Define.h"
#include "Types.h"
#include "carray.h"
#include "glew.h"

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
	ubyte dflags; /* this is only important for large bitmaps like the cockpit */
	ubyte width;
	ubyte height;
	ubyte flags;
	ubyte avgColor;
	uint offset;
} PIG_TEXTURE_D1;

typedef struct {
	int nId;
	int nVersion;
	int nTextures;
} PIG_HEADER_D2;

typedef struct {
	char name[8];
	ubyte dflags;  // bits 0-5 anim frame num, bit 6 abm flag
	ubyte width;   // low 8 bits here, 4 more bits in pad
	ubyte height;   // low 8 bits here, 4 more bits in pad
	ubyte whExtra;     // bits 0-3 xsize, bits 4-7 ysize
	ubyte flags;   // see BM_FLAG_XXX in define.h
	ubyte avgColor;   // average color
	uint offset;
} PIG_TEXTURE_D2;

typedef struct {
	ubyte unknown[20];
} PIG_SOUND;

typedef struct {
	char name[8];
	short number;
} TEXTURE;

//------------------------------------------------------------------------

static inline int Pow2ize (int v) 
{
#if 1
for (int i = 2; i <= 0x10000000; i *= 2)
	if (v <= i)
		return i;
#endif
return v;
}

static inline int Pow2Dim (int w, int h)
{
return (w < h) ? Pow2ize (h) : Pow2ize (w);
}

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

	CPigHeader (int nVersion = 1) : nId (0), nVersion (0), nTextures (0), nSounds (0) {
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

typedef struct tPigTextureHeader {
	char		name[8];
	ubyte		dflags;   // bits 0-5 anim frame num, bit 6 abm flag
	ushort	width;    // low 8 bits here, 4 more bits in pad
	ushort	height;   // low 8 bits here, 4 more bits in pad
	ubyte		whExtra;  // bits 0-3 xsize, bits 4-7 ysize
	ubyte		flags;    // see BM_FLAG_XXX in define.h
	ubyte		avgColor; // average color
	uint		offset;
	} tPigTextureHeader;

class CPigTexture : public tPigTextureHeader {
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

	void Setup (int nVersion, ushort w = 0, ushort h = 0, ubyte f = 0, uint o = 0) {
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
		dflags = fp->ReadUByte ();
		width = (ushort) fp->ReadUByte ();
		height = (ushort) fp->ReadUByte ();
		if (m_nVersion == 1)
			whExtra = fp->ReadUByte ();
		else {
			name [7] = 0;
			whExtra = 0;
			}
		flags = fp->ReadUByte ();
		avgColor = fp->ReadUByte ();
		offset = fp->ReadUInt32 ();
		Decode ();
		}

	void Write (CFileManager* fp) {
		Encode ();
		fp->WriteBytes (name, sizeof (name));
		fp->Write (dflags);
		fp->WriteByte ((ubyte) width);
		fp->WriteByte ((ubyte) height);
		if (m_nVersion == 1)
			fp->Write (whExtra);
		fp->Write (flags);
		fp->Write (avgColor);
		fp->Write (offset);
		}

	inline int Dim (void) { 
		int w = Pow2ize (width);
		int h = Pow2ize (height); 
		return (w < h) ? h : w;
		}

	inline int BufSize (void) { 
		int d = Pow2Dim (width, height);
		return d * d;
		}
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct {
    ubyte  identSize;          // size of ID field that follows 18 ubyte header (0 usually)
    ubyte  colorMapType;      // type of colour map 0=none, 1=has palette
    ubyte  imageType;          // type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    short colorMapStart;     // first colour map entry in palette
    short colorMapLength;    // number of colours in palette
    ubyte  colorMapBits;      // number of bits per palette entry 15,16,24,32

    short xStart;             // image x origin
    short yStart;             // image y origin
    short width;              // image width in pixels
    short height;             // image height in pixels
    ubyte  bits;               // image bits per pixel 8,16,24,32
    ubyte  descriptor;         // image descriptor bits (vh flip bits)
} tTgaHeader;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tRGBA {
	ubyte r, g, b, a;
} tRGBA;


typedef struct tBGRA {
	ubyte b, g, r, a;
} tBGRA;


typedef struct tABGR {
	ubyte a, b, g, r;
} tABGR;


class CBGR {
	public:
		ubyte b, g, r;

		CBGR (ubyte red = 0, ubyte green = 0, ubyte blue = 0)
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
		ubyte a;

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

		CBGRA (ubyte red = 0, ubyte green = 0, ubyte blue = 0, ubyte alpha = 255)
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
	uint			width, height, offset, bufSize;
	uint			xOffset, yOffset;
	int			nId; 
	bool			bCustom, bExtData, bFrame, bUsed, bValid, bTransparent, bFlat;
	ubyte			nFormat;	// 0: Bitmap, 1: TGA (RGB)
	char			szName [9];
	rgbaColorf	averageColor;
	GLuint		glHandle;
} tTexture;

class CTexture {
	public:
		tTexture	m_info;
		CBGRA*	m_data;
		CBGRA*	m_override;
		short		m_nTexture;

		CTexture (CBGRA* dataP = null) : m_override (null), m_data (dataP) {
			Clear ();
			m_info.bExtData = (dataP != null);
			}

		void Release (void);

		~CTexture() { Release (); }

		bool Allocate (int nSize);

		int Load (CFileManager& fp, short nTexture, int nVersion = -1);

		void Load (CFileManager& fp, CPigTexture& info);

		void Load (int nId);

		bool LoadTGA (CFileManager& fp);

		bool LoadTGA (char* pszFile);

		int Reload (void);

		double Scale (short index = -1);

		int Shrink (int xFactor, int yFactor);

		ubyte* ToBitmap (bool bShrink);

		void Read (CFileManager& fp, int version = 0, bool bFlag = false) { };

		void Write (CFileManager& fp, int version = 0, bool bFlag = false) {};

		virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }

		void ComputeIndex (ubyte* bmIndex);

		inline bool Transparent (void) { return m_info.bTransparent; }

		inline uint Width (int bScale = 1) { return (m_info.nFormat || !bScale) ? m_info.width : Pow2Dim (m_info.width, m_info.height); }

		inline uint Height (int bScale = 1) { return (m_info.nFormat || !bScale) ? m_info.height : Pow2Dim (m_info.width, m_info.height); }

		inline uint Size (void) { return Width () * Height (); }

		inline uint BufSize (void) { return Width () * Height () * sizeof (CBGRA); }

		inline CBGRA* Buffer (uint i = 0) { return (m_override == null) ? &m_data [i] : &m_override [i]; }

		GLuint GLBind (GLuint nTMU, GLuint nMode);

		void GLRelease (void);

		void DrawLine (POINT pt0, POINT pt1, CBGRA color);

		void DrawAnimDirArrows (short nTexture);

		inline bool Copy (CTexture& src) {
			if (!Allocate (src.Size ()))
				return false;
			memcpy (Buffer (), src.Buffer (), src.BufSize ());
			memcpy (&m_info, &src.m_info, sizeof (m_info));
			m_info.glHandle = 0;
			m_info.bValid = true;
			return true;
			}

		rgbaColorf& AverageColor (rgbaColorf& color);

		inline void SetFlat (bool bFlat) { m_info.bFlat = bFlat; }

		inline bool Flat (void) { return m_info.bFlat; } // texture or just color?

		inline void SetAverageColor (float r, float g, float b, float a = 1.0f) { 
			m_info.averageColor.r = r, m_info.averageColor.g = g, m_info.averageColor.b = b, m_info.averageColor.r = a; }

		inline void SetAverageColor (rgbaColorf& color) { m_info.averageColor = color; }

		inline rgbaColorf& GetAverageColor (rgbaColorf& color) { return color = m_info.averageColor; }

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

bool TGA2Bitmap (tRGBA *pTGA, ubyte *pBM, int nWidth, int nHeight);

//------------------------------------------------------------------------

#endif //__textures_h