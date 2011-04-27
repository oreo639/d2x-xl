#ifndef _TGA_H
#define _TGA_H

#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

typedef struct {
    char  identSize;         // size of ID field that follows 18 char header (0 usually)
    char  colorMapType;      // nType of colour map 0=none, 1=has palette
    char  imageType;         // nType of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

    short colorMapStart;     // first colour map entry in palette
    short colorMapLength;    // number of colours in palette
    char  colorMapBits;      // number of bits per palette entry 15,16,24,32

    ushort xStart;            // image x origin
    ushort yStart;            // image y origin
    ushort width;             // image width in pixels
    ushort height;            // image height in pixels
    char   bits;              // image bits per pixel 8,16,24,32
    char   descriptor;        // image descriptor bits (vh flip bits)
} __pack__ tTGAHeader;


class CTGAHeader {
	public:
		tTGAHeader	m_data;

	public:
		CTGAHeader () { Reset (); }
		void Reset (void) { memset (&m_data, 0, sizeof (m_data)); }
		void Setup (const tTGAHeader* headerP) { 
			if (headerP) 
				m_data = *headerP; 
			else
				Reset ();
			}
		
		int Read (CFile& cf, CBitmap* bmP);
		int Write (CFile& cf, CBitmap *bmP);

		inline tTGAHeader& Data (void) { return m_data; }
		inline ushort Width (void) { return m_data.width; }
		inline ushort Height (void) { return m_data.height; }
		inline char Bits (void) { return m_data.bits; }
	};

class CModelTextures {
	public:
		int						m_nBitmaps;
		CArray<CCharArray>	m_names;
		CArray<CBitmap>		m_bitmaps;
		CArray<ubyte>			m_nTeam;

	public:
		CModelTextures () { Init (); } 
		void Init (void) { m_nBitmaps = 0; }
		int Bind (int bCustom);
		void Release (void);
		int Read (int bCustom);
		int ReadBitmap (int i, int bCustom);
		bool Create (int nBitmaps);
		void Destroy (void);
};


class CTGA {
	protected:
		CFile			m_cf;
		CTGAHeader	m_header;
		CBitmap*		m_bmP;

	public:
		CTGA (CBitmap* bmP = NULL)
			: m_bmP (bmP)
			{
			}

		~CTGA () { m_bmP = NULL; }

		void Setup (CBitmap* bmP = NULL, const tTGAHeader* headerP = NULL) 
			{ 
			if (bmP)
				m_bmP = bmP;
			m_header.Setup (headerP);
			}

		int Shrink (int xFactor, int yFactor, int bRealloc);
		int ReadData (CFile& cf, int alpha, double brightness, int bGrayScale, int bReverse);
		int WriteData (void);
		int Load (int alpha, double brightness, int bGrayScale);
		int Read (const char* pszFile, const char* pszFolder, int alpha = -1, double brightness = 1.0, int bGrayScale = 0);
		int Write (void);
		CBitmap* CreateAndRead (char* pszFile);
		int Save (const char *pszFile, const char *pszFolder);
		double Brightness (void);
		void ChangeBrightness (double dScale, int bInverse, int nOffset, int bSkipAlpha);
		int Interpolate (int nScale);
		int MakeSquare (void);
		int Compress (void);
		void ConvertToRGB (void);
		void PreMultiplyAlpha (float fScale = 1.0f);
		CBitmap* ReadModelTexture (const char *pszFile, int bCustom);

		inline tTGAHeader& Header (void) { return m_header.Data (); }

	private:
		void SetProperties (int alpha, int bGrayScale, double brightness, bool bSwapRB = true);
		int ReadImage (const char* pszFile, const char* pszFolder, int alpha, double brightness, int bGrayScale);

	};


#endif //_TGA_H
