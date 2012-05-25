#ifndef __texturemanager_h
#define __texturemanager_h

#include "textures.h"
#include "carray.h"

//------------------------------------------------------------------------

#define MAX_TEXTURES ((theMine == null) ? MAX_TEXTURES_D2 : DLE.IsD1File () ? MAX_TEXTURES_D1 : MAX_TEXTURES_D2)

//------------------------------------------------------------------------

#define SMOKE_ICON		0
#define SNOW_ICON			1
#define RAIN_ICON			2
#define BUBBLE_ICON		3
#define FIRE_ICON			4
#define LIGHTNING_ICON	5
#define SOUND_ICON		6
#define WAYPOINT_ICON	7
#define LIGHT_ICON		8
#define CIRCLE_ICON		9
#define ICON_COUNT		10

#if 1 //_DEBUG

typedef CDynamicArray< CTexture > textureList;

#else

typedef CTexture* textureList;

#endif

class CTextureManager {
	private:
		textureList		m_textures [2];

	public:
		uint				m_nTextures [2];
		char**			m_names [2];
		char				m_paletteName [2][256];
		ushort*			m_index [2];
		uint				m_nOffsets [2];
		CPigTexture*	m_info [2];
		CPigHeader		m_header [2];
		CBGRA				m_bmBuf [512 * 512 * 32];	// max texture size: 512x512, RGBA, 32 frames
		char				m_pigFiles [2][256];
		bool				m_bAvailable [2];
		CExtraTexture*	m_extra;
		CTexture			m_arrow;
		CTexture			m_icons [ICON_COUNT];

		inline CTexture* Textures (int nVersion, int nTexture = 0) { 
			if (!m_textures [nVersion].Buffer ())
				return null;
			if (nTexture < 0)
				return &m_textures [1][-nTexture - 1]; 
			return &m_textures [nVersion][m_index [nVersion][nTexture] - 1];
		}

		inline CPigTexture* Info (int nVersion, int nTexture = 0) { 
			if (!m_info [nVersion])
				return null;
			return (nTexture < 0)
					 ? &m_info [nVersion][-nTexture - 1]
					 : &m_info [nVersion][m_index [nVersion][nTexture] - 1]; 
			}

		int MaxTextures (int nVersion = -1);
		
		bool LoadTextures (int nVersion = -1, bool bCleanup = false);
		
		bool Check (int nTexture);
		
		void Load (ushort nBaseTex, ushort nOvlTex);

		void ReadMod (char* pszFolder);
	
		void LoadMod (void);
		
		int BlendTextures (short nBaseTex, short nOvlTex, CTexture* pDestTex, int x0, int y0);
		
		void Release (bool bDeleteAll = false, bool bDeleteUnused = false);
		
		void ReleaseExtras (void);

		bool HasCustomTextures (void);
		
		int CountCustomTextures (void);
		
		void UnTagUsedTextures (void);

		void TagUsedTextures (void);
		
		void RemoveTextures (bool bUnused = true);
		
		CFileManager* OpenPigFile (int nVersion);
		
		CTexture* AddExtra (ushort nIndex);

		inline bool HaveInfo (int nVersion) { return m_info [nVersion] != null; }
		
		int Version (void);
		
		inline char* Name (int nVersion, short nTexture) { 
			if (nVersion < 0)
				nVersion = Version ();
			if (!m_names [nVersion])
				return "";
			char* p = m_names [nVersion][nTexture];
			return p ? p : "";
			}
		
		inline int Index (CTexture* texP, int nVersion = -1) { return int (texP - &m_textures [(nVersion < 0) ? Version () : nVersion][0]); }

		inline CTexture* Texture (short nTexture) { 
			int nVersion = Version ();
			if (!m_textures [nVersion].Buffer ())
				return null;
			if (nTexture < 0)
				return &m_textures [1][-nTexture - 1]; 
			else if (nTexture >= MaxTextures (nVersion))
				nTexture = 0;
			return &m_textures [nVersion][m_index [nVersion][nTexture] - 1];
			}

		inline CTexture& Arrow (void) { return m_arrow; }

		inline CTexture& Icon (int i) { return m_icons [i]; }

		inline int HaveArrow (void) { return m_arrow.Buffer () != null; }

		inline bool IsLava (short nTexture) { return (strstr (Name (-1, nTexture), "lava") != null); }

		bool IsAnimated (UINT16 texture);

		int ScrollSpeed (UINT16 texture, int *x, int *y);

		int ScrollDirection (UINT16 texture);

		inline double Scale (int nVersion, short nTexture) {
			return m_textures [nVersion].Buffer ()
					 ? Textures (nVersion, nTexture)->Scale (nTexture)
					 : 1.0;
			}

		int ReadPog (CFileManager& fp, long nFileSize);

		int CreatePog (CFileManager& fp);

		int WriteCustomTexture (CFileManager& fp, CTexture *texP);

		void Setup (void);

		void Destroy (void);

		bool Reload (int nVersion, bool bForce = true);

		bool Available (int nVersion = -1);

		int InitShaders (void);

		int DeployShader (int nType, CFaceListEntry* fle);

		CTextureManager() : m_extra (null) { 
			m_paletteName [0][0] = 0; 
			m_bAvailable [0] = m_bAvailable [1] = false;
			}
	
		~CTextureManager() { Destroy (); }

	private:
		int LoadIndex (int nVersion);

		void LoadNames (int nVersion);

		bool LoadInfo (int nVersion);

		void Release (int nVersion, bool bDeleteAll, bool bDeleteUnused);

		void Create (int nVersion);

		void Destroy (int nVersion);

		inline CBGRA& Blend (CBGRA& dest, CBGRA& src);

		uint WriteCustomTextureHeader (CFileManager& fp, CTexture *texP, int nId = -1, uint nOffset = 0);
	};

extern CTextureManager textureManager;

//------------------------------------------------------------------------

#endif //__texturemanager_h