#ifndef __customtextures_h
#define __customtextures_h

#include "textures.h"

//------------------------------------------------------------------------

class CExtraTexture : public CTexture {
public:
	CExtraTexture*	m_next;
	ushort			m_index;
};

extern CExtraTexture* extraTextures;

//------------------------------------------------------------------------

int ReadPog (CFileManager& fp, uint nFileSize = 0xFFFFFFFF);
int CreatePog (CFileManager& fp);

//------------------------------------------------------------------------

#endif //__customtextures_h