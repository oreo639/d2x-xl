// palette.h

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
		byte*		m_custom;
		byte*		m_default;	
		tBMIInfo	m_bmi;

	public:
		CPaletteManager () : m_custom(null), m_default(null) {}
		void Load (void);
		int LoadCustom (CFileManager& fp, long size);
		int SaveCustom (CFileManager& fp);
		void FreeCustom (void);
		void FreeCurrent (void);
		inline void Release (void) {
			FreeCustom ();
			FreeCurrent ();
			}
		inline void Reload (void) {
			Release ();
			Current ();
			}

		inline byte* Custom () { return m_custom; }
		inline byte* Default () { return m_default; }
		byte* Current (void);
		BITMAPINFO* BMI (void) { return Current () ? (BITMAPINFO*) &m_bmi : null; }

	private:
		const char* Resource (void);
		byte FadeValue (byte c, int f);
		void SetupBMI (byte* palette);
		byte* LoadDefault (void);
		void Encode (byte* palette);
		void Decode (byte* palette);
};

extern CPaletteManager paletteManager;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------
