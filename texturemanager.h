#ifndef __texturemanager_h
#define __texturemanager_h

#include "textures.h"
#include "carray.h"

//------------------------------------------------------------------------

#if USE_DYN_ARRAYS

typedef CDynamicArray< CTexture > textureList;

#else

typedef CTexture* textureList;

#endif

class CTextureManager {
public:
	uint nTextures [2];
	textureList textures [2];
	ushort* index [2];
	uint nOffsets [2];
	CPigTexture* info [2];
	CPigHeader header [2];
	byte bmBuf [512 * 512 * 32 * 4];	// max texture size: 512x512, RGBA, 32 frames

	inline CTexture* Textures (int nVersion, int nTexture = 0) { return &textures [nVersion][nTexture]; }

	int MaxTextures (int nVersion = -1);
	int LoadIndex (int nVersion);
	void LoadTextures (int nVersion);
	CPigTexture& LoadInfo (CFileManager& fp, int nVersion, short nTexture);
	bool Check (int nTexture);
	void Load (ushort nBaseTex, ushort nOvlTex);
	int Define (short nBaseTex, short nOvlTex, CTexture* pDestTex, int x0, int y0);
	void Release (bool bDeleteAll = true, bool bDeleteUnused = false);
	bool HasCustomTextures (void);
	int CountCustomTextures (void);
	void MarkUsedTextures (void);
	void RemoveUnusedTextures (void);
	CFileManager* OpenPigFile (int nVersion);

	inline bool HaveInfo (int nVersion) { return info [nVersion] != null; }

	CTextureManager() {}
	
	void Setup (void);

	~CTextureManager() {
		for (int i = 0; i < 2; i++) {
#if USE_DYN_ARRAYS
			//textures [i].Destroy ();
#else
			if (textures [i])
				delete textures [i];
#endif
			if (index [i])
				delete index [i];
			if (info [i])
				delete info [i];
			}
		}
};

extern CTextureManager textureManager;

//------------------------------------------------------------------------

#endif //__texturemanager_h