
//particle.h
//simple particle system handler

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#	include <conf.h>
#endif

#ifdef __macosx__
# include <SDL/SDL.h>
#else
# include <SDL.h>
#endif

#include "pstypes.h"
#include "descent.h"
#include "error.h"
#include "u_mem.h"
#include "vecmat.h"
#include "hudmsgs.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_shader.h"
#include "ogl_fastrender.h"
#include "piggy.h"
#include "globvars.h"
#include "segmath.h"
#include "network.h"
#include "light.h"
#include "dynlight.h"
#include "lightmap.h"
#include "renderlib.h"
#include "rendermine.h"
#include "transprender.h"
#include "objsmoke.h"
#include "glare.h"
#include "particles.h"
#include "renderthreads.h"
#include "automap.h"

//-------------------------------------------------------------------------

int hParticleShader = -1;

bool CParticleManager::LoadShader (float dMax [2])
{
ogl.ClearError (0);
if (!gameOpts->render.bUseShaders)
	return false;
if (ogl.m_features.bDepthBlending < 1)
	return false;
if (!ogl.CopyDepthTexture (0, GL_TEXTURE2))
	return false;
//ogl.DrawBuffer ()->FlipBuffers (0, 1); // color buffer 1 becomes render target, color buffer 0 becomes render source (scene texture)
//ogl.DrawBuffer ()->SetDrawBuffers ();
m_shaderProg = GLhandleARB (shaderManager.Deploy (hParticleShader));
if (!m_shaderProg)
	return false;
if (shaderManager.Rebuild (m_shaderProg)) {
	shaderManager.Set ("particleTex", 0);
	shaderManager.Set ("sparkTex", 1);
	shaderManager.Set ("depthTex", 2);
	shaderManager.Set ("windowScale", ogl.m_data.windowScale.vec);
	shaderManager.Set ("dMax", dMax);
	}
ogl.SetDepthTest (false);
ogl.SetAlphaTest (false);
ogl.SetBlendMode (OGL_BLEND_ALPHA_CONTROLLED);
ogl.SelectTMU (GL_TEXTURE0);
return true;
}

//-------------------------------------------------------------------------

void CParticleManager::UnloadShader (void)
{
if (ogl.m_features.bDepthBlending > 0) {
	shaderManager.Deploy (-1);
	//DestroyGlareDepthTexture ();
	ogl.SetTexturing (true);
	ogl.SelectTMU (GL_TEXTURE1);
	ogl.BindTexture (0);
	ogl.SelectTMU (GL_TEXTURE2);
	ogl.BindTexture (0);
	ogl.SelectTMU (GL_TEXTURE0);
	ogl.SetDepthTest (true);
	}
}

//------------------------------------------------------------------------------
// The following shader blends a particle into a scene. The blend mode depends
// on the particle color's alpha value: 0.0 => additive, otherwise alpha
// This shader allows to switch between additive and alpha blending without
// having to flush a particle render batch beforehand
// In order to be able to handle blending in a shader, a framebuffer with 
// two color buffers is used and the scene from the one color buffer is rendered
// into the other color buffer with blend mode replace (GL_ONE, GL_ZERO)

const char *particleFS =
	"uniform sampler2D particleTex, sparkTex, depthTex;\r\n" \
	"uniform vec2 dMax;\r\n" \
	"uniform vec2 windowScale;\r\n" \
	"//#define ZNEAR 1.0\r\n" \
	"//#define ZFAR 5000.0\r\n" \
	"//#define A 5001.0 //(ZNEAR + ZFAR)\r\n" \
	"//#define B 4999.0 //(ZNEAR - ZFAR)\r\n" \
	"//#define C 10000.0 //(2.0 * ZNEAR * ZFAR)\r\n" \
	"//#define D (NDC (Z) * B)\r\n" \
	"// compute Normalized Device Coordinates\r\n" \
	"#define NDC(z) (2.0 * z - 1.0)\r\n" \
	"// compute eye space depth value from window depth\r\n" \
	"#define ZEYE(z) (10000.0 / (5001.0 - NDC (z) * 4999.0)) //(C / (A + D))\r\n" \
	"//#define ZEYE(z) -(ZFAR / ((z) * (ZFAR - ZNEAR) - ZFAR))\r\n" \
	"void main (void) {\r\n" \
	"// compute distance from scene fragment to particle fragment and clamp with 0.0 and max. distance\r\n" \
	"// the bigger the result, the further the particle fragment is behind the corresponding scene fragment\r\n" \
	"bool nType = gl_TexCoord [0].z < 0.5;\r\n" \
	"float dm = dMax [nType];\r\n" \
	"float dz = clamp (ZEYE (gl_FragCoord.z) - ZEYE (texture2D (depthTex, gl_FragCoord.xy * windowScale).r), 0.0, dm);\r\n" \
	"// compute scaling factor [0.0 - 1.0] - the closer distance to max distance, the smaller it gets\r\n" \
	"dz = (dm - dz) / dm;\r\n" \
	"vec4 texColor = texture2D (nType ? sparkTex : particleTex, gl_TexCoord [0].xy);\r\n" \
	"texColor *= gl_Color * dz;\r\n" \
	"if (gl_Color.a == 0.0) //additive\r\n" \
	"   gl_FragColor = vec4 (texColor.rgb, 1.0);\r\n" \
	"else // alpha\r\n" \
	"   gl_FragColor = vec4 (texColor.rgb * texColor.a, 1.0 - texColor.a);\r\n" \
	"}\r\n"
	;

const char *particleVS =
	"void main (void){\r\n" \
	"gl_TexCoord [0] = gl_MultiTexCoord0;\r\n" \
	"gl_Position = ftransform (); //gl_ModelViewProjectionMatrix * gl_Vertex;\r\n" \
	"gl_FrontColor = gl_Color;}\r\n"
	;

//-------------------------------------------------------------------------

void CParticleManager::InitShader (void)
{
if (ogl.m_features.bRenderToTexture && ogl.m_features.bShaders && (ogl.m_features.bDepthBlending > -1)) {
	PrintLog ("building particle blending shader program\n");
	m_shaderProg = 0;
	if (shaderManager.Build (hParticleShader, particleFS, particleVS))
		ogl.m_features.bDepthBlending.Available (1);
	else {
		ogl.ClearError (0);
		ogl.m_features.bDepthBlending.Available (0);
		}
	}
}

//------------------------------------------------------------------------------
//eof
