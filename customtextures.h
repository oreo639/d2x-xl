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

int ReadPog (FILE *file, uint nFileSize = 0xFFFFFFFF);
int CreatePog (FILE *file);

//------------------------------------------------------------------------

#endif //__customtextures_h