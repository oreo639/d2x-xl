#ifndef __texturemanager_h
#define __texturemanager_h

#include "textures.h"
#include "carray.h"

//------------------------------------------------------------------------

#define MAX_TEXTURES ((theMine == null) ? MAX_TEXTURES_D2 : DLE.IsD1File () ? MAX_TEXTURES_D1 : MAX_TEXTURES_D2)

//------------------------------------------------------------------------

#if _DEBUG

typedef CDynamicArray< CTexture > textureList;

#else

typedef CTexture* textureList;

#endif

class CTextureManager {
	public:
		uint m_nTextures [2];
		char** m_names [2];
		char m_paletteName [15];
		textureList m_textures [2];
		ushort* m_index [2];
		uint m_nOffsets [2];
		CPigTexture* m_info [2];
		CPigHeader m_header [2];
		byte m_bmBuf [512 * 512 * 32 * 4];	// max texture size: 512x512, RGBA, 32 frames
		char m_pigFiles [2][256];
		CExtraTexture*	m_extra;

		inline CTexture* Textures (int nVersion, int nTexture = 0) { return &m_textures [nVersion][nTexture]; }

		inline CPigTexture& Info (int nVersion, int nTexture = 0) { return m_info [nVersion][m_index [nVersion][nTexture] - 1]; }

		int MaxTextures (int nVersion = -1);
		
		void LoadTextures (int nVersion = -1, bool bCleanup = false);
		
		bool Check (int nTexture);
		
		void Load (ushort nBaseTex, ushort nOvlTex);
		
		int Define (short nBaseTex, short nOvlTex, CTexture* pDestTex, int x0, int y0);
		
		void Release (bool bDeleteAll = false, bool bDeleteUnused = false);
		
		bool HasCustomTextures (void);
		
		int CountCustomTextures (void);
		
		void MarkUsedTextures (void);
		
		void RemoveUnusedTextures (void);
		
		CFileManager* OpenPigFile (int nVersion);
		
		CTexture* AddExtra (ushort nIndex);

		inline bool HaveInfo (int nVersion) { return m_info [nVersion] != null; }
		
		int Version (void);
		
		inline char* Name (short nTexture) { 
			char* p = m_names [Version ()][nTexture]; 
			return p ? p : "";
			}
		
		CTexture* Texture (short nTexture) { return &m_textures [Version ()][nTexture]; }

		inline bool IsLava (short nTexture) { return (strstr (Name (nTexture), "lava") != null); }

		int ScrollSpeed (UINT16 texture, int *x, int *y);

		CTextureManager() {}
	
		void Setup (void);
		void Destroy (void);

		~CTextureManager() { Destroy (); }

	private:
		int LoadIndex (int nVersion);
		void LoadNames (int nVersion);
		void LoadInfo (int nVersion);
		void Release (int nVersion, bool bDeleteAll, bool bDeleteUnused);
		void Create (int nVersion);
		void Destroy (int nVersion);

	};

extern CTextureManager textureManager;

//------------------------------------------------------------------------

#endif //__texturemanager_h