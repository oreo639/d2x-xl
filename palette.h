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
		byte*				m_custom;
		byte*				m_default;	
		tBMIInfo			m_bmi;
		CPalette*		m_render;
		LPLOGPALETTE	m_dlcLog;

	public:
		void Load (void);
		int LoadCustom (CFileManager& fp, long size);
		int SaveCustom (CFileManager& fp);
		void FreeCustom (void);
		void FreeCurrent (void);
		void FreeRender (void);
		inline void Release (void) {
			FreeCustom ();
			FreeCurrent ();
			FreeRender ();
			}
		inline void Reload (void) {
			Release ();
			Current ();
			}

		inline byte* Custom () { return m_custom; }
		inline byte* Default () { return m_default; }
		inline CPalette* Render () { 
			if (!m_render)
				SetupRender (Current ());
			return m_render; 
			}
		byte* Current (void);
		BITMAPINFO* BMI (void) { return Current () ? (BITMAPINFO*) &m_bmi : null; }

	private:
		const char* Resource (void);
		byte FadeValue (byte c, int f);
		void SetupBMI (byte* palette);
		short SetupRender (byte* palette);
		byte* LoadDefault (void);
		void Encode (byte* palette);
		void Decode (byte* palette);

		CPaletteManager () : m_custom(null), m_default(null) {}
		~CPaletteManager () { Release (); }
};

extern CPaletteManager paletteManager;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif //__palette_h