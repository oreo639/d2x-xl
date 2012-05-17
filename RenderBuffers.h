#ifndef __RENDERBUFFERS_H
#define __RENDERBUFFERS_H

#include "glew.h"

GLuint CreateDepthTexture (int nTMU, int bFBO, int nType = 0, int nWidth = -1, int hHeight = -1);
void DestroyDepthTexture (int bFBO);
GLuint CopyDepthTexture (int nId, int nTMU = GL_TEXTURE1);
GLuint CreateColorTexture (int nTMU, int bFBO);
void DestroyColorTexture (void);
GLuint CopyColorTexture (void);

#endif //__RENDERBUFFERS_H
