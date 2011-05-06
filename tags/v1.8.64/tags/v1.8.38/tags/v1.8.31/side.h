
#ifndef __side_h
#define __side_h

#include "define.h"
#include "Types.h"
#include "FileManager.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tSide {
	ushort	nChild;
	ushort	nWall;		// (was short) Index into Walls array, which wall (probably door) is on this side 
	short		nBaseTex;	// Index into array of textures specified in bitmaps.bin 
	short		nOvlTex;		// Index, as above, texture which gets overlaid on nBaseTex 
	CUVL		uvls [4];   // CUVL coordinates at each point 
} tSide;

// -----------------------------------------------------------------------------

class CSide {
public:
	tSide m_info;

	inline tSide& Info (void) { return m_info; }

	void Read (CFileManager* fp, bool bTextured);

	void Write (CFileManager* fp);

	void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	
	void Setup (void);
	
	void LoadTextures (void);
	
	void GetTextures (short &nBaseTex, short &nOvlTex) _const_;
	
	bool SetTextures (short nBaseTex, short nOvlTex);
	
	void InitUVL (short nTexture);
	
	inline void SetWall (short nWall) { m_info.nWall = nWall; }

	inline short BaseTex (void) _const_ { return m_info.nBaseTex; }
	
	inline short OvlTex (void) _const_ { return m_info.nOvlTex; }

	inline CUVL _const_ * Uvls (void) _const_ { return &m_info.uvls [0]; }
	
	CSegment _const_ * Child (void) _const_;

	CWall _const_ * Wall (void) _const_;
	
	CTrigger _const_ * Trigger (void) _const_;
	
	bool UpdateChild (short nOldChild, short nNewChild);

	bool IsVisible (void);

	void Reset (void);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif //__side_h