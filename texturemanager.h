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
	char** names [2];
	textureList textures [2];
	ushort* index [2];
	uint nOffsets [2];
	CPigTexture* info [2];
	CPigHeader header [2];
	byte bmBuf [512 * 512 * 32 * 4];	// max texture size: 512x512, RGBA, 32 frames

	inline CTexture* Textures (int nVersion, int nTexture = 0) { return &textures [nVersion][nTexture]; }

	int MaxTextures (int nVersion = -1);
	void LoadTextures (int nVersion);
	CPigTexture& LoadInfo (CFileManager& fp, int nVersion, short nTexture);
	bool Check (int nTexture);
	void Load (ushort nBaseTex, ushort nOvlTex);
	int Define (short nBaseTex, short nOvlTex, CTexture* pDestTex, int x0, int y0);
	void Release (bool bDeleteAll = false, bool bDeleteUnused = false);
	bool HasCustomTextures (void);
	int CountCustomTextures (void);
	void MarkUsedTextures (void);
	void RemoveUnusedTextures (void);
	CFileManager* OpenPigFile (int nVersion);

	inline bool HaveInfo (int nVersion) { return info [nVersion] != null; }
	int Version (void);
	inline char* Name (short nTexture) { 
		char* p = names [Version ()][nTexture]; 
		return p ? p : "";
		}
	CTexture* Texture (short nTexture) { return &textures [Version ()][nTexture]; }

	CTextureManager() {}
	
	void Setup (void);
	void Destroy (void);

	~CTextureManager() { Destroy (); }

private:
	int LoadIndex (int nVersion);
	void LoadNames (int nVersion);
	void Release (int nVersion, bool bDeleteAll, bool bDeleteUnused);
	void Destroy (int nVersion);

};

extern CTextureManager textureManager;

//------------------------------------------------------------------------

#endif //__texturemanager_h