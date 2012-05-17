#include <windows.h>
#include <stddef.h>
#include <string.h>

#include "mine.h"
#include "dle-xp.h"
#include "glew.h"
#include "RenderBuffers.h"
#include "FBO.h"

// -----------------------------------------------------------------------------------

#define GL_DEPTH_STENCIL_EXT			0x84F9
#define GL_UNSIGNED_INT_24_8_EXT		0x84FA
#define GL_DEPTH24_STENCIL8_EXT		0x88F0
#define GL_TEXTURE_STENCIL_SIZE_EXT 0x88F1

GLuint CreateDepthTexture (int nTMU, int nType, int nWidth, int nHeight)
{
	GLuint	hDepthBuffer;

if (nTMU >= GL_TEXTURE0)
	glActiveTexture (nTMU);
glEnable (GL_TEXTURE_2D);
glGenTextures (1, &hDepthBuffer);
if (glGetError ())
	return hDepthBuffer = 0;
glBindTexture (nTMU, hDepthBuffer);
glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY); 	
if (nType == 0) // depth + stencil
	glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8_EXT, nWidth, nHeight, 0, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT_24_8_EXT, NULL);
else
	glTexImage2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, (nWidth > 0) ? nWidth, nHeight, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
if (glGetError ()) {
	glDeleteTextures (1, &hDepthBuffer);
	return hDepthBuffer = 0;
	}
glBindTexture (nTMU, 0);
return hDepthBuffer;
}

// -----------------------------------------------------------------------------------

bool CopyDepthTexture (GLuint hDepthBuffer, int nTMU, int nWidth, int nHeight)
{
	static int nSamples = -1;
	static int nDuration = 0;

	GLenum nError = glGetError ();

glActiveTexture (nTMU);
glEnable (GL_TEXTURE_2D);
glBindTexture (nTMU, hDepthBuffer);
glReadBuffer (GL_BACK);
glCopyTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 0, 0, nWidth, nHeight);
if (glGetError ())
	return false;
#if 0
if (t > 0) {
	if (++nSamples > 0) {
		nDuration += SDL_GetTicks () - t;
		if ((nSamples >= 5) && (nDuration / nSamples > 10)) {
			PrintLog (0, "Disabling depth buffer reads (average read time: %d ms)\n", nDuration / nSamples);
			ogl.m_features.bDepthBlending = -1;
			DestroyDepthTexture (nId);
			hDepthBuffer = 0;
			}
		}
	}
#endif
return true;
}

// -----------------------------------------------------------------------------------

GLuint CreateColorTexture (int nTMU, int nWidth, int nHeight, bool bFBO)
{
	GLuint	hColorBuffer;

if (nTMU > GL_TEXTURE0)
	glActiveTexture (nTMU);
glEnable (GL_TEXTURE_2D);
glGenTextures (1, &hColorBuffer);
if (glGetError ())
	return hColorBuffer = 0;
glBindTexture (nTMU, hColorBuffer);
glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
glTexImage2D (GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0, GL_RGB, GL_UNSIGNED_SHORT, NULL);
if (glGetError ()) {
	glDeleteTextures (1, &hColorBuffer);
	return hColorBuffer = 0;
	}
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
if (!bFBO) {
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
	}
if (glGetError ()) {
	glDeleteTextures (1, &hColorBuffer);
	return hColorBuffer = 0;
	}
return hColorBuffer;
}

// -----------------------------------------------------------------------------------

bool CopyColorTexture (GLuint hColorBuffer, int nTMU, int nWidth, int nHeight)
{
	GLenum nError = glGetError ();

glActiveTexture (GL_TEXTURE1);
glEnable (GL_TEXTURE_2D);
glBindTexture (nTMU, hColorBuffer);
glCopyTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 0, 0, nWidth, nHeight);
return true;
}

// -----------------------------------------------------------------------------------

#if 0

GLuint COGL::CreateStencilTexture (int nTMU, int bFBO)
{
	GLuint	hBuffer;

if (nTMU > 0)
	glActiveTexture (nTMU);
ogl.glEnable (GL_TEXTURE_2D);
GenTextures (1, &hBuffer);
if (glGetError ())
	return hDepthBuffer = 0;
ogl.BindTexture (hBuffer);
glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
glTexImage2D (GL_TEXTURE_2D, 0, GL_STENCIL_COMPONENT8, m_states.nCurWidth, m_states.nCurHeight,
				  0, GL_STENCIL_COMPONENT, GL_UNSIGNED_BYTE, NULL);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
if (glGetError ()) {
	DeleteTextures (1, &hBuffer);
	return hBuffer = 0;
	}
return hBuffer;
}

#endif

//------------------------------------------------------------------------------

