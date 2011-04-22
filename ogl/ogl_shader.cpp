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
#include "particles.h"
#include "../effects/glow.h"
#include "sphere.h"
#include "rendermine.h"
#include "gpgpu_lighting.h"

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

if (!(fp = fopen (fn, "rb")))
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

void CShaderManager::PrintLog (GLhandleARB handle, int bProgram)
{
   GLint nLogLen = 0;
   GLint charsWritten = 0;
   char *infoLog;

#ifdef GL_VERSION_20
if (bProgram) {
	glGetProgramiv (GLuint (intptr_t (handle)), GL_INFO_LOG_LENGTH, &nLogLen);
	if ((nLogLen > 0) && (infoLog = new char [nLogLen])) {
		glGetProgramInfoLog (GLuint (intptr_t (handle)), nLogLen, &charsWritten, infoLog);
		if (*infoLog)
			::PrintLog ("\n%s\n\n", infoLog);
		delete[] infoLog;
		}
	}
else {
	glGetShaderiv (GLuint (intptr_t (handle)), GL_INFO_LOG_LENGTH, &nLogLen);
	if ((nLogLen > 0) && (infoLog = new char [nLogLen])) {
		glGetShaderInfoLog (GLuint (intptr_t (handle)), nLogLen, &charsWritten, infoLog);
		if (*infoLog)
			::PrintLog ("\n%s\n\n", infoLog);
		delete[] infoLog;
		}
	}
#else
glGetObjectParameterivARB (GLuint (intptr_t (handle)), GL_OBJECT_INFO_LOG_LENGTH_ARB, &nLogLen);
if ((nLogLen > 0) && (infoLog = new char [nLogLen])) {
	glGetInfoLogARB (GLuint (intptr_t (handle)), nLogLen, &charsWritten, infoLog);
	if (*infoLog)
		::PrintLog ("\n%s\n\n", infoLog);
	delete[] infoLog;
	}
#endif
}

//------------------------------------------------------------------------------

GLhandleARB	genShaderProg = 0;

int CShaderManager::Create (int nShader)
{
if ((nShader < 0) || (nShader >= int (m_shaders.ToS ())))
	return 0;
if (m_shaders [nShader].program || (m_shaders [nShader].program = glCreateProgramObjectARB ()))
	return 1;
::PrintLog ("   Couldn't create shader program CObject\n");
return 0;
}

//------------------------------------------------------------------------------

void CShaderManager::Dispose (GLhandleARB& shaderProg)
{
if (shaderProg) {
	glDeleteObjectARB (shaderProg);
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
		glDeleteObjectARB (shader.shaders [i]);
		shader.shaders [i] = 0;
		}
	if (!(shader.shaders [i] = glCreateShaderObjectARB (nShaderTypes [i])))
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
	glGetObjectParameterivARB (shader.shaders [i], GL_OBJECT_COMPILE_STATUS_ARB, &bCompiled [i]);
	if (!bCompiled [i])
		break;
	glAttachObjectARB (shader.program, shader.shaders [i]);
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
			glDeleteObjectARB (shader.shaders [i]);
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
	int	i = 0;
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
glGetObjectParameterivARB (shader.program, GL_OBJECT_LINK_STATUS_ARB, &bLinked);
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
			glDeleteObjectARB (shader.shaders [j]);
			shader.shaders [j] = 0;
			}
		}
	if (shader.program) {
		glDeleteObjectARB (shader.program);
		shader.program = 0;
		}
	}
}

//------------------------------------------------------------------------------

intptr_t CShaderManager::Deploy (int nShader)
{
if (!ogl.m_states.bShadersOk)
	return 0;
if (nShader >= int (m_shaders.ToS ()))
	return 0;
GLhandleARB shaderProg = (nShader < 0) ? 0 : m_shaders [nShader].program;
if (m_nCurrent == nShader)
	return -intptr_t (shaderProg); // < 0 => program already bound
m_nCurrent = nShader;
glUseProgramObjectARB (shaderProg);
gameData.render.nShaderChanges++;
return intptr_t (shaderProg);
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
#if GPGPU_VERTEX_LIGHTING
::PrintLog ("   initializing vertex lighting shader programs\n");
gpgpuLighting.InitShader ();
#endif
::PrintLog ("   initializing glare shader programs\n");
glareRenderer.InitShader ();
::PrintLog ("   initializing particle shader programs\n");
particleManager.InitShader ();
::PrintLog ("   initializing blur shader programs\n");
glowRenderer.InitShader ();
::PrintLog ("   initializing gray scale shader programs\n");
InitGrayScaleShader ();
::PrintLog ("   initializing enhanced 3D shader programs\n");
ogl.InitEnhanced3DShader ();
ogl.InitShadowMapShader ();
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
else
	ogl.m_states.bShadersOk = 1;
PrintLog (ogl.m_states.bShadersOk ? "Shaders are available\n" : "No shaders available\n");
}

//------------------------------------------------------------------------------
//eof
