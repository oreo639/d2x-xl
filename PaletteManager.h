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
		COLORREF*		m_custom;
		COLORREF*		m_default;	
		tBMIInfo			m_bmi;
		CPalette*		m_render;
		LPLOGPALETTE	m_dlcLog;
		PALETTEENTRY*	m_colorMap;

	public:
		void Load (void);

		void LoadName (CFileManager& fp);

		int LoadCustom (CFileManager& fp, long size);

		int SaveCustom (CFileManager& fp);

		void FreeCustom (void);

		void FreeDefault (void);

		void FreeRender (void);

		inline void Release (void) {
			FreeCustom ();
			FreeDefault ();
			FreeRender ();
			}
		inline void Reload (void) {
			Release ();
			Current ();
			}

		inline char* Name (void) { return m_name; }

		inline COLORREF* Custom (void) { return m_custom; }

		inline COLORREF* Default (void) { return m_default; }

		inline CPalette* Render (void) { return m_render; }

		inline PALETTEENTRY* ColorMap (void) { return m_colorMap; }

		COLORREF* Current (int i = 0);

		BITMAPINFO* BMI (void) { return Current () ? (BITMAPINFO*) &m_bmi : null; }

		CPaletteManager () : m_custom(null), m_default(null) { *m_name = '\0'; }

		~CPaletteManager () { Release (); }

	private:
		const char* Resource (void);

		byte FadeValue (byte c, int f);

		void SetupBMI (byte* palette);

		short SetupRender (byte* palette);

		byte* LoadDefault (void);

		void Decode (COLORREF* dest, byte* src, int len);

		void Encode (byte* dest, COLORREF* src);

};

extern CPaletteManager paletteManager;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif //__palette_h