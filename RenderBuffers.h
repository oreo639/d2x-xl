#ifndef __RENDERBUFFERS_H
#define __RENDERBUFFERS_H

#include "glew.h"

GLuint CreateDepthTexture (int nTMU = GL_TEXTURE1, int nType = 0, int nWidth, int hHeight);
void DestroyDepthTexture (void);
bool CopyDepthTexture (GLuint hDepthBuffer, int nTMU = GL_TEXTURE1);
GLuint CreateColorTexture (int nTMU = GL_TEXTURE1, int nWidth, int nHeight, bool bFBO);
void DestroyColorTexture (void);
bool CopyColorTexture (GLuint hColorBuffer, int nTMU, int nWidth, int nHeight)

#endif //__RENDERBUFFERS_H
