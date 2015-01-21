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
		CDynamicArray <CTexture*> m_overrides [2]; // Items can be blank if there is no custom texture
		CDynamicArray <bool>      m_bModified [2]; // Tracks whether textures have changed since last save
		CDynamicArray <CTexture*> m_previous [2];  // Previous texture as of save, to allow reverts

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
		bool*				m_bUsed [2];
#if EXTRA_TEXTURES
		CExtraTexture*	m_extra;
#endif
		CTexture			m_arrow;
		CTexture			m_icons [ICON_COUNT];

	public:
		// Looks up textures by level texture ID. Pointers returned by this function should not be
		// stored persistently as they may change.
		inline const CTexture* Textures (int nTexture, int nVersion = -1) { 
			int nVersionResolved = (nVersion < 0) ? Version () : nVersion;
			if (!m_textures [nVersionResolved].Buffer ())
				return null;
			if (nTexture < 0)
				return AllTextures (-nTexture - 1, nVersionResolved);
			return AllTextures (LevelTexToAllTex (nTexture), nVersionResolved);
			}

		// Looks up textures by global texture ID. Pointers returned by this function should not be
		// stored persistently as they may change.
		inline const CTexture* AllTextures (uint nTexAll, int nVersion = -1) {
			int nVersionResolved = (nVersion < 0) ? Version () : nVersion;
			if (!m_textures [nVersionResolved].Buffer ())
				return null;
			if (nTexAll >= (uint) AllTextureCount (nVersionResolved))
				return null;
			if (m_overrides [nVersionResolved][nTexAll] != null)
				return m_overrides [nVersionResolved][nTexAll];
			return &m_textures [nVersionResolved][nTexAll];
			}

		inline uint LevelTexToAllTex (int nTexLevel, int nVersion = -1) {
			return m_index [(nVersion < 0) ? Version () : nVersion][nTexLevel] - 1;
			}

		bool FindLevelTex (uint nTexAll, int *pnTexLevel, int nVersion = -1);

		inline CPigTexture* Info (int nVersion, uint nTexAll) {
			if (!m_info [nVersion])
				return null;
			return &m_info [nVersion][nTexAll];
			}

		// Location at which texture bitmap data is stored in the PIG
		inline uint TexDataOffset (int nVersion = -1) {
			return m_nOffsets [(nVersion < 0) ? Version () : nVersion];
			}

		int MaxTextures (int nVersion = -1);

		int AllTextureCount (int nVersion = -1);
		
		bool LoadTextures (int nVersion = -1, bool bClearExisting = true);
		
		bool Check (int nTexture);
		
		void Load (ushort nBaseTex, ushort nOvlTex);

		void ReadMod (char* pszFolder);
	
		void LoadMod (void);

		CTexture* OverrideTexture (uint nTexAll, const CTexture* newTexture = null, int nVersion = -1);

		void RevertTexture (uint nTexAll, int nVersion = -1);
		
		void ReleaseTextures (void);
		
#if EXTRA_TEXTURES
		void ReleaseExtras (void);
#endif

		bool HasCustomTextures (void);
		
		int CountCustomTextures (void);

		int CountModifiedTextures (void);
		
		void UnTagUsedTextures (void);

		void TagUsedTextures (void);

		inline bool IsTextureUsed (int nTexLevel, int nVersion = -1) {
			if (nTexLevel < 0)
				return false;
			return m_bUsed [(nVersion < 0) ? Version () : nVersion][nTexLevel];
			}
		
		void RemoveCustomTextures (bool bUnusedOnly = true);

		void CommitTextureChanges (void);

		void UndoTextureChanges (void);
		
		CFileManager* OpenPigFile (int nVersion);
		
#if EXTRA_TEXTURES
		CTexture* AddExtra (ushort nIndex);
#endif

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

		inline CTexture& Arrow (void) { return m_arrow; }

		inline CTexture& Icon (int i) { return m_icons [i]; }

		inline int HaveArrow (void) { return m_arrow.Buffer () != null; }

		inline bool IsLava (short nTexture) { return (strstr (Name (-1, nTexture), "lava") != null); }

		bool IsAnimated (UINT16 texture);

		int ScrollSpeed (UINT16 texture, int *x, int *y);

		int ScrollDirection (UINT16 texture);

		inline double Scale (int nVersion, short nTexture) {
			return m_textures [nVersion].Buffer ()
					 ? Textures (nTexture, nVersion)->Scale (nTexture)
					 : 1.0;
			}

		int UsedCount (uint nTexAll);

		inline int UsedCount (const CTexture *pTexture) { return pTexture ? UsedCount (pTexture->IdAll ()) : 0; }

		inline const CTexture* BaseTextures (uint nTexAll, int nVersion = -1) {
			return &m_textures [(nVersion < 0) ? Version () : nVersion][nTexAll];
			}

		int ReadPog (CFileManager& fp, long nFileSize);

		int CreatePog (CFileManager& fp);

		int WriteCustomTexture (CFileManager& fp, const CTexture *texP);

		void Setup (void);

		void Destroy (void);

		bool Reload (int nVersion, bool bForce = true);

		bool ChangePigFile (const char *pszPigPath, int nVersion = -1);

		bool Available (int nVersion = -1);

		int InitShaders (void);

		int DeployShader (int nType, CFaceListEntry* fle);

		void CreateGLTextures (int nVersion = -1);

		CBGRA* SharedBuffer (void) { return m_bmBuf; }

		size_t SharedBufferSize (void) { return sizeof (m_bmBuf); }

		CTextureManager() { 
#if EXTRA_TEXTURES
			m_extra = null;
#endif
			m_paletteName [0][0] = 0; 
			m_bAvailable [0] = m_bAvailable [1] = false;
			}
	
		~CTextureManager() { Destroy (); }

	private:
		int LoadIndex (int nVersion);

		void LoadNames (int nVersion);

		bool LoadInfo (int nVersion);

		void ReleaseTextures (int nVersion);

		void Create (int nVersion);

		void Destroy (int nVersion);

		uint WriteCustomTextureHeader (CFileManager& fp, const CTexture *texP, uint nId, uint nOffset);
	};

extern CTextureManager textureManager;

//------------------------------------------------------------------------

#endif //__texturemanager_h