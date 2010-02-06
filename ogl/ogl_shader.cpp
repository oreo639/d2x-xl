/*
 *
 * Graphics support functions for OpenGL.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef _WIN32
#	include <windows.h>
#	include <stddef.h>
#	include <io.h>
#endif
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#ifdef __macosx__
# include <stdlib.h>
# include <SDL/SDL.h>
#else
# include <SDL.h>
#endif

#include "descent.h"
#include "error.h"
#include "u_mem.h"
#include "light.h"
#include "dynlight.h"
#include "lightmap.h"
#include "texmerge.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_shader.h"
#include "ogl_fastrender.h"
#include "glare.h"
#include "sphere.h"
#include "rendermine.h"
#include "gpgpu_lighting.h"

#ifndef GL_VERSION_20
PFNGLCREATESHADEROBJECTARBPROC	glCreateShaderObject = NULL;
PFNGLSHADERSOURCEARBPROC			glShaderSource = NULL;
PFNGLCOMPILESHADERARBPROC			glCompileShader = NULL;
PFNGLCREATEPROGRAMOBJECTARBPROC	glCreateProgramObject = NULL;
PFNGLATTACHOBJECTARBPROC			glAttachObject = NULL;
PFNGLLINKPROGRAMARBPROC				glLinkProgram = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC		glUseProgramObject = NULL;
PFNGLDELETEOBJECTARBPROC			glDeleteObject = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC	glGetObjectParameteriv = NULL;
PFNGLGETINFOLOGARBPROC				glGetInfoLog = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC	glGetUniformLocation = NULL;
PFNGLUNIFORM4FARBPROC				glUniform4f = NULL;
PFNGLUNIFORM3FARBPROC				glUniform3f = NULL;
PFNGLUNIFORM1FARBPROC				glUniform1f = NULL;
PFNGLUNIFORM4FVARBPROC				glUniform4fv = NULL;
PFNGLUNIFORM3FVARBPROC				glUniform3fv = NULL;
PFNGLUNIFORM2FVARBPROC				glUniform2fv = NULL;
PFNGLUNIFORM1FVARBPROC				glUniform1fv = NULL;
PFNGLUNIFORM1IARBPROC				glUniform1i = NULL;
#endif

//------------------------------------------------------------------------------

#if 1

const char *progVS [] = {
	"void TexMergeVS ();" \
	"void main (void) {TexMergeVS ();}"
,
	"void LightingVS ();" \
	"void main (void) {LightingVS ();}"
,
	"void LightingVS ();" \
	"void TexMergeVS ();" \
	"void main (void) {TexMergeVS (); LightingVS ();}"
	};

const char *progFS [] = {
	"void TexMergeFS ();" \
	"void main (void) {TexMergeFS ();}"
,
	"void LightingFS ();" \
	"void main (void) {LightingFS ();}"
,
	"void LightingFS ();" \
	"void TexMergeFS ();" \
	"void main (void) {TexMergeFS (); LightingFS ();}"
	};

GLhandleARB mainVS = 0;
GLhandleARB mainFS = 0;

#endif

CShaderManager shaderManager;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CShaderManager::CShaderManager ()
{
Init ();
}

//------------------------------------------------------------------------------

CShaderManager::~CShaderManager ()
{
Destroy (true);
}

//------------------------------------------------------------------------------

void CShaderManager::Init (void)
{
m_shaders.Create (100);
m_shaders.SetGrowth (100);
m_shaders.Clear ();
m_nCurrent = -1;
}

//------------------------------------------------------------------------------

void CShaderManager::Destroy (bool bAll)
{
for (int i = 0; i < int (m_shaders.ToS ()); i++) {
	tShaderData& shader = m_shaders [i];
	Delete (i);
	if (bAll)
		Reset (i);
	}
if (bAll)
	m_shaders.Destroy ();
}

//------------------------------------------------------------------------------
// load a shader program's source code from a file

char* CShaderManager::Load (const char* fileName) //, char* Shadersource)
{
	FILE*	fp;
	char*	bufP = NULL;
	int 	fSize;
#ifdef _WIN32
	int	f;
#endif
	char 	fn [FILENAME_LEN];

if (!(fileName && *fileName))
	return NULL;	// no fileName

sprintf (fn, "%s%s%s", gameFolders.szShaderDir, *gameFolders.szShaderDir ? "/" : "", fileName);
#ifdef _WIN32
if (0 > (f = _open (fn, _O_RDONLY)))
	return NULL;	// couldn't open file
fSize = _lseek (f, 0, SEEK_END);
_close (f);
if (fSize <= 0)
	return NULL;	// empty file or seek error
#endif

if (!(fp = fopen (fn, "rt")))
	return NULL;	// couldn't open file

#ifndef _WIN32
fseek (fp, 0, SEEK_END);
fSize = ftell (fp);
if (fSize <= 0) {
	fclose (fp);
	return NULL;	// empty file or seek error
	}
#endif

if (!(bufP = new char [fSize + 1])) {
	fclose (fp);
	return NULL;	// out of memory
	}
fSize = (int) fread (bufP, sizeof (char), fSize, fp);
bufP [fSize] = '\0';
fclose (fp);
return bufP;
}

//------------------------------------------------------------------------------

#ifdef __macosx__
#	define	_HANDLE	(uint) handle
#else
#	define	_HANDLE	handle
#endif

void CShaderManager::PrintLog (GLhandleARB handle, int bProgram)
{
   GLint nLogLen = 0;
   GLint charsWritten = 0;
   char *infoLog;

#ifdef GL_VERSION_20
if (bProgram) {
	glGetProgramiv (_HANDLE, GL_INFO_LOG_LENGTH, &nLogLen);
	if ((nLogLen > 0) && (infoLog = new char [nLogLen])) {
		glGetProgramInfoLog (_HANDLE, nLogLen, &charsWritten, infoLog);
		if (*infoLog)
			PrintLog ("\n%s\n\n", infoLog);
		delete[] infoLog;
		}
	}
else {
	glGetShaderiv (_HANDLE, GL_INFO_LOG_LENGTH, &nLogLen);
	if ((nLogLen > 0) && (infoLog = new char [nLogLen])) {
		glGetShaderInfoLog (_HANDLE, nLogLen, &charsWritten, infoLog);
		if (*infoLog)
			PrintLog ("\n%s\n\n", infoLog);
		delete[] infoLog;
		}
	}
#else
glGetObjectParameteriv (_HANDLE, GL_OBJECT_INFO_LOG_LENGTH_ARB, &nLogLen);
if ((nLogLen > 0) && (infoLog = new char [nLogLen])) {
	glGetInfoLog (_HANDLE, nLogLen, &charsWritten, infoLog);
	if (*infoLog)
		::PrintLog ("\n%s\n\n", infoLog);
	delete[] infoLog;
	}
#endif
}

#undef _HANDLE

//------------------------------------------------------------------------------

GLhandleARB	genShaderProg = 0;

int CShaderManager::Create (int nShader)
{
if ((nShader < 0) || (nShader >= int (m_shaders.ToS ())))
	return 0;
if (m_shaders [nShader].program || (m_shaders [nShader].program = glCreateProgramObject ()))
	return 1;
::PrintLog ("   Couldn't create shader program CObject\n");
return 0;
}

//------------------------------------------------------------------------------

void CShaderManager::Dispose (GLhandleARB& shaderProg)
{
if (shaderProg) {
	glDeleteObject (shaderProg);
	shaderProg = 0;
	}
}

//------------------------------------------------------------------------------

int CShaderManager::Alloc (int& nShader)
{
if ((nShader >= 0) && (nShader < int (m_shaders.ToS ())) && (m_shaders [nShader].refP == &nShader))
	return nShader;
if (!m_shaders.Grow ())
	return nShader = -1;
nShader = m_shaders.ToS () - 1;
memset (&m_shaders [nShader], 0, sizeof (m_shaders [nShader]));
m_shaders [nShader].refP = &nShader;
return nShader;
}

//------------------------------------------------------------------------------

int CShaderManager::Compile (int nShader, const char* pszFragShader, const char* pszVertShader, bool bFromFile)
{
	GLint		bCompiled [2] = {0,0};
	bool		bError = false;
	int		i;

	static GLint nShaderTypes [2] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};

if (!ogl.m_states.bShadersOk)
	return 0;
if ((nShader < 0) || (nShader >= int (m_shaders.ToS ())))
	return 0;
tShaderData& shader = m_shaders [nShader];

for (i = 0; i < 2; i++) {
	if (shader.shaders [i]) {
		glDeleteObject (shader.shaders [i]);
		shader.shaders [i] = 0;
		}
	if (!(shader.shaders [i] = glCreateShaderObject (nShaderTypes [i])))
		break;
#if DBG_SHADERS
	if (bFromFile) {
		shader.shaders [i] = LoadShader (i ? pszFragShader : pszVertShader);
		if (!shader.shaders [i])
			return 0;
		}
#endif
	glShaderSource (shader.shaders [i], 1, i ? reinterpret_cast<const GLcharARB **> (&pszFragShader) : reinterpret_cast<const GLcharARB **> (&pszVertShader), NULL);
#if DBG_SHADERS
	if (bFromFile) {
		if (i)
			delete[] pszFragShader;
		else
			delete[] pszVertShader;
		}
#endif
	glCompileShader (shader.shaders [i]);
	glGetObjectParameteriv (shader.shaders [i], GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled [i]);
	if (!bCompiled [i])
		break;
	glAttachObject (shader.program, shader.shaders [i]);
	}

for (i = 0; i < 2; i++) {
	if (!bCompiled [i]) {
		bError = true;
		if (i)
			::PrintLog ("   Couldn't compile fragment shader\n   \"%s\"\n", pszFragShader);
		else
			::PrintLog ("   Couldn't compile vertex shader\n   \"%s\"\n", pszVertShader);
		if (shader.shaders [i]) {
			PrintLog (shader.shaders [i], 0);
			glDeleteObject (shader.shaders [i]);
			shader.shaders [i] = 0;
			}
		}
	}
if (!bError)
	return 1;
m_shaders.Pop ();
return 0;
}

//------------------------------------------------------------------------------

int CShaderManager::Link (int nShader)
{
if (!ogl.m_states.bShadersOk)
	return 0;
if ((nShader < 0) || (nShader >= int (m_shaders.ToS ())))
	return 0;
tShaderData& shader = m_shaders [nShader];

if (!shader.program) {
	if (!Create (nShader))
		return 0;
	int	i;
	if (gameOpts->ogl.bGlTexMerge)
		i |= 1;
	if (gameStates.render.nLightingMethod)
		i |= 2;
	if (!i)
		return 0;
	if (!Compile (nShader, progFS [i - 1], progVS [i - 1], 0)) {
		Dispose (shader.program);
		return 0;
		}
	}

glLinkProgram (shader.program);
GLint	bLinked;
glGetObjectParameteriv (shader.program, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
if (bLinked)
	return 1;
::PrintLog ("   Couldn't link shader programs\n");
PrintLog (shader.program, 1);
Dispose (shader.program);
return 0;
}

//------------------------------------------------------------------------------

int CShaderManager::Build (int& nShader, const char* pszFragShader, const char* pszVertShader, bool bFromFile)
{
if ((nShader < 0) || (nShader >= int (m_shaders.ToS ()))) {
	if (Alloc (nShader) < 0)
		return 0;
	if (!Create (nShader))
		return 0;
	}
else {
	if (m_shaders [nShader].program)
		return 1;
	}
if (!Compile (nShader, pszFragShader, pszVertShader, bFromFile))
	return 0;
if (!Link (nShader))
	return 0;
return 1;
}

//------------------------------------------------------------------------------

void CShaderManager::Reset (int nShader)
{
if ((nShader >= 0) && (nShader < int (m_shaders.ToS ()))) {
	*m_shaders [nShader].refP = -1;
	m_shaders [nShader].refP = NULL;
	}
}

//------------------------------------------------------------------------------

void CShaderManager::Delete (int nShader)
{
if ((nShader >= 0) && (nShader < int (m_shaders.ToS ()))) {
	tShaderData& shader = m_shaders [nShader];
	for (int j = 0; j < 2; j++) {
		if (shader.shaders [j]) {
			glDeleteObject (shader.shaders [j]);
			shader.shaders [j] = 0;
			}
		}
	if (shader.program) {
		glDeleteObject (shader.program);
		shader.program = 0;
		}
	}
}

//------------------------------------------------------------------------------

int CShaderManager::Deploy (int nShader)
{
if (!ogl.m_states.bShadersOk)
	return 0;
if (nShader >= int (m_shaders.ToS ()))
	return 0;
GLhandleARB shaderProg = (nShader < 0) ? 0 : m_shaders [nShader].program;
if (m_nCurrent == nShader)
	return -int (shaderProg);
m_nCurrent = nShader;
glUseProgramObject (shaderProg);
gameData.render.nShaderChanges++;
return int (shaderProg);
}

//------------------------------------------------------------------------------

void CShaderManager::Setup (void)
{
	GLint	nTMUs;

if (!(gameOpts->render.bUseShaders && ogl.m_states.bShadersOk))
	return;
Destroy ();
Init ();
::PrintLog ("initializing shader programs\n");
glGetIntegerv (GL_MAX_TEXTURE_UNITS, &nTMUs);
ogl.m_states.bShadersOk = (nTMUs >= 4);
if (!ogl.m_states.bShadersOk) {
	::PrintLog ("GPU has too few texture units (%d)\n", nTMUs);
	ogl.m_states.bLowMemory = 0;
	ogl.m_states.bHaveTexCompression = 0;
	return;
	}
gameStates.render.bLightmapsOk = (nTMUs >= 4);
::PrintLog ("   initializing texture merging shader programs\n");
InitTexMergeShaders ();
ogl.m_data.nHeadlights = 0;
::PrintLog ("   initializing lighting shader programs\n");
InitHeadlightShaders (1);
::PrintLog ("   initializing vertex lighting shader programs\n");
gpgpuLighting.InitShader ();
::PrintLog ("   initializing glare shader programs\n");
glareRenderer.InitShader ();
::PrintLog ("   initializing gray scale shader programs\n");
InitGrayScaleShader ();
::PrintLog ("   initializing enhanced 3D shader programs\n");
ogl.InitEnhanced3DShader ();
ResetPerPixelLightingShaders ();
InitPerPixelLightingShaders ();
ResetLightmapShaders ();
InitLightmapShaders ();
ResetSphereShaders ();
#if 0
Link (Alloc ());
#endif
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void COGL::InitShaders (void)
{
shaderManager.Setup ();
}

//------------------------------------------------------------------------------

void COGL::SetupShaders (void)
{
PrintLog ("Checking shaders ...\n");
ogl.m_states.bShadersOk = 0;
if (!gameOpts->render.bUseShaders)
	PrintLog ("   Shaders have been disabled in d2x.ini\n");
else if (!ogl.m_states.bMultiTexturingOk)
	PrintLog ("   Multi-texturing not supported by the OpenGL driver\n");
else if (!pszOglExtensions)
	PrintLog ("   Required Extensions not supported by the OpenGL driver\n");
else if (!strstr (pszOglExtensions, "GL_ARB_shading_language_100"))
	PrintLog ("   Shading language not supported by the OpenGL driver\n");
else if (!strstr (pszOglExtensions, "GL_ARB_shader_objects"))
	PrintLog ("   Shader objects not supported by the OpenGL driver\n");
else {
#ifndef GL_VERSION_20
	if (!(glCreateProgramObject = (PFNGLCREATEPROGRAMOBJECTARBPROC) wglGetProcAddress ("glCreateProgramObjectARB")))
		PrintLog ("   glCreateProgramObject not supported by the OpenGL driver\n");
	else if (!(glDeleteObject = (PFNGLDELETEOBJECTARBPROC) wglGetProcAddress ("glDeleteObjectARB")))
		PrintLog ("   glDeleteObject not supported by the OpenGL driver\n");
	else if (!(glUseProgramObject = (PFNGLUSEPROGRAMOBJECTARBPROC) wglGetProcAddress ("glUseProgramObjectARB")))
		PrintLog ("   glUseProgramObject not supported by the OpenGL driver\n");
	else if (!(glCreateShaderObject = (PFNGLCREATESHADEROBJECTARBPROC) wglGetProcAddress ("glCreateShaderObjectARB")))
		PrintLog ("   glCreateShaderObject not supported by the OpenGL driver\n");
	else if (!(glShaderSource = (PFNGLSHADERSOURCEARBPROC) wglGetProcAddress ("glShaderSourceARB")))
		PrintLog ("   glShaderSource not supported by the OpenGL driver\n");
	else if (!(glCompileShader = (PFNGLCOMPILESHADERARBPROC) wglGetProcAddress ("glCompileShaderARB")))
		PrintLog ("   glCompileShader not supported by the OpenGL driver\n");
	else if (!(glGetObjectParameteriv = (PFNGLGETOBJECTPARAMETERIVARBPROC) wglGetProcAddress ("glGetObjectParameterivARB")))
		PrintLog ("   glGetObjectParameteriv not supported by the OpenGL driver\n");
	else if (!(glAttachObject = (PFNGLATTACHOBJECTARBPROC) wglGetProcAddress ("glAttachObjectARB")))
		PrintLog ("   glAttachObject not supported by the OpenGL driver\n");
	else if (!(glGetInfoLog = (PFNGLGETINFOLOGARBPROC) wglGetProcAddress ("glGetInfoLogARB")))
		PrintLog ("   glGetInfoLog not supported by the OpenGL driver\n");
	else if (!(glLinkProgram = (PFNGLLINKPROGRAMARBPROC) wglGetProcAddress ("glLinkProgramARB")))
		PrintLog ("   glLinkProgram not supported by the OpenGL driver\n");
	else if (!(glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONARBPROC) wglGetProcAddress ("glGetUniformLocationARB")))
		PrintLog ("   glGetUniformLocation not supported by the OpenGL driver\n");
	else if (!(glUniform4f = (PFNGLUNIFORM4FARBPROC) wglGetProcAddress ("glUniform4fARB")))
		PrintLog ("   glUniform4f not supported by the OpenGL driver\n");
	else if (!(glUniform3f = (PFNGLUNIFORM3FARBPROC) wglGetProcAddress ("glUniform3fARB")))
		PrintLog ("   glUniform3f not supported by the OpenGL driver\n");
	else if (!(glUniform1f = (PFNGLUNIFORM1FARBPROC) wglGetProcAddress ("glUniform1fARB")))
		PrintLog ("   glUniform1f not supported by the OpenGL driver\n");
	else if (!(glUniform4fv = (PFNGLUNIFORM4FVARBPROC) wglGetProcAddress ("glUniform4fvARB")))
		PrintLog ("   glUniform4fv not supported by the OpenGL driver\n");
	else if (!(glUniform3fv = (PFNGLUNIFORM3FVARBPROC) wglGetProcAddress ("glUniform3fvARB")))
		PrintLog ("   glUniform3fv not supported by the OpenGL driver\n");
	else if (!(glUniform2fv = (PFNGLUNIFORM3FVARBPROC) wglGetProcAddress ("glUniform2fvARB")))
		PrintLog ("   glUniform2fv not supported by the OpenGL driver\n");
	else if (!(glUniform1fv = (PFNGLUNIFORM1FVARBPROC) wglGetProcAddress ("glUniform1fvARB")))
		PrintLog ("   glUniform1fv not supported by the OpenGL driver\n");
	else if (!(glUniform1i = (PFNGLUNIFORM1IARBPROC) wglGetProcAddress ("glUniform1iARB")))
		PrintLog ("   glUniform1i not supported by the OpenGL driver\n");
	else
		ogl.m_states.bShadersOk = 1;
#else
	ogl.m_states.bShadersOk = 1;
#endif
	}
PrintLog (ogl.m_states.bShadersOk ? "Shaders are available\n" : "No shaders available\n");
}

//------------------------------------------------------------------------------
//eof
