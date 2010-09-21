#ifndef __palette_h
#define __palette_h

#include "resourcemanager.h"

//------------------------------------------------------------------------
int Luminance (int r, int g, int b);

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tBMIInfo {
	BITMAPINFOHEADER header;
	RGBQUAD colors [256];
} tBMIInfo;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CPaletteManager {
	private:
		char				m_name [15];
		byte				m_rawData [37 * 256];
		COLORREF			m_custom [256];
		COLORREF			m_default [256];	
		byte				m_fadeTable [34 * 256];
		tBMIInfo			m_bmi;
		CPalette*		m_render;
		LPLOGPALETTE	m_dlcLog;
		PALETTEENTRY*	m_colorMap;

	public:
		void Load (void);

		void LoadName (CFileManager& fp);

		int LoadCustom (CFileManager& fp, long size);

		int SaveCustom (CFileManager& fp);

		void FreeRender (void);

		inline void Release (void) {
			FreeRender ();
			}
		inline void Reload (void) {
			Release ();
			Current ();
			}

		inline byte* FadeTable (void) { return m_fadeTable; }

		inline char* Name (void) { return m_name; }

		inline COLORREF* Custom (void) { return m_custom; }

		inline COLORREF* Default (void) { return m_default; }

		inline CPalette* Render (void) { return m_render; }

		inline PALETTEENTRY* ColorMap (void) { return m_colorMap; }

		COLORREF* Current (int i = 0);

		BITMAPINFO* BMI (void) { return Current () ? (BITMAPINFO*) &m_bmi : null; }

		CPaletteManager () { *m_name = '\0'; }

		~CPaletteManager () { Release (); }

	private:
		const char* Resource (void);

		byte FadeValue (byte c, int f);

		void SetupBMI (COLORREF* palette);

		short SetupRender (COLORREF* palette);

		COLORREF* LoadDefault (void);

		void Decode (COLORREF* dest);

		void Encode (COLORREF* src);

		void CreateFadeTable (void);
};

extern CPaletteManager paletteManager;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif //__palette_h