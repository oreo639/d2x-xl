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
#include "config.h"
#include "maths.h"
#include "crypt.h"
#include "strutil.h"
#include "segmath.h"
#include "light.h"
#include "dynlight.h"
#include "lightmap.h"
#include "network.h"
#include "gr.h"
#include "glow.h"
#include "gamefont.h"
#include "screens.h"
#include "interp.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_texcache.h"
#include "ogl_color.h"
#include "ogl_shader.h"
#include "ogl_render.h"
#include "findfile.h"
#include "rendermine.h"
#include "sphere.h"
#include "glare.h"
#include "menu.h"
#include "menubackground.h"
#include "cockpit.h"
#include "renderframe.h"
#include "automap.h"
#include "gpgpu_lighting.h"
#include "postprocessing.h"

//#define _WIN32_WINNT		0x0600

int enhance3DShaderProg [2][2][3] = {{{-1,-1,-1},{-1,-1,-1}},{{-1,-1,-1},{-1,-1,-1}}};
int duboisShaderProg = -1;

int nScreenDists [10] = {1, 2, 5, 10, 15, 20, 30, 50, 70, 100};
float nDeghostThresholds [4][2] = {{1.0f, 1.0f}, {0.8f, 0.8f}, {0.7f, 0.7f}, {0.6f, 0.6f}};

extern tTexCoord2f quadTexCoord [4];
extern float quadVerts [4][2];

//-----------------------------------------------------------------------------------

void COGL::FlushStereoBuffers (int nEffects)
{
int j = Enhance3D ();

if (j == -2) {
	// todo: add barrel distortion shader
	SetDrawBuffer (GL_BACK, 0);
	OglDrawArrays (GL_QUADS, 0, 4);
	}
else if (j == -1) {
	SetDrawBuffer ((m_data.xStereoSeparation < 0) ? GL_BACK_LEFT : GL_BACK_RIGHT, 0);
	OglDrawArrays (GL_QUADS, 0, 4);
	}
else { // merge left and right anaglyph buffers
	if (m_data.xStereoSeparation > 0) {
		static float gain [4] = {1.0, 4.0, 2.0, 1.0};
		int h = gameOpts->render.stereo.bDeghost;
		int i = (gameOpts->render.stereo.bColorGain > 0);
		if (nEffects & 1) // additional effect and/or shadow map rendering
			SelectDrawBuffer (2); // use as temporary render buffer
		else
			SetDrawBuffer (GL_BACK, 0);
		if ((j > 0) && (j <= 3) && ((h < 4) || (j == 2))) {
			GLhandleARB shaderProg = GLhandleARB (shaderManager.Deploy ((h == 4) ? duboisShaderProg : enhance3DShaderProg [h > 0][i][--j]));
			if (shaderProg) {
				shaderManager.Rebuild (shaderProg);
				ogl.EnableClientStates (1, 0, 0, GL_TEXTURE1);
				ogl.BindTexture (DrawBuffer (1)->ColorBuffer ());
				OglTexCoordPointer (2, GL_FLOAT, 0, quadTexCoord);
				OglVertexPointer (2, GL_FLOAT, 0, quadVerts);

				glUniform1i (glGetUniformLocation (shaderProg, "leftFrame"), gameOpts->render.stereo.bFlipFrames);
				glUniform1i (glGetUniformLocation (shaderProg, "rightFrame"), !gameOpts->render.stereo.bFlipFrames);
				if (h < 4) {
					if (h)
						glUniform2fv (glGetUniformLocation (shaderProg, "strength"), 1, reinterpret_cast<GLfloat*> (&nDeghostThresholds [gameOpts->render.stereo.bDeghost]));
					if (i)
						glUniform1f (glGetUniformLocation (shaderProg, "gain"), gain [gameOpts->render.stereo.bColorGain]);
					}
				ClearError (0);
				}
			}
		OglDrawArrays (GL_QUADS, 0, 4);
		}
	}	
}

//-----------------------------------------------------------------------------------

const char* enhance3DFS [2][2][3] = {
	{
		{
	#if 0
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"void main() {\r\n" \
		"gl_FragColor = vec4 (texture2D (leftFrame, gl_TexCoord [0].xy).xy, dot (texture2D (rightFrame, gl_TexCoord [0].xy).rgb, vec3 (0.15, 0.15, 0.7)), 1.0);\r\n" \
		"}",
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"gl_FragColor = vec4 (clamp (1.0, dot (texture2D (leftFrame, gl_TexCoord [0].xy).rgb, vec3 (1.0, 0.15, 0.15))), texture2D (rightFrame, gl_TexCoord [0].xy).yz, 0.0, 1.0);\r\n" \
		"}",
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"void main() {\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"gl_FragColor = vec4 (cl.r, dot (texture2D (rightFrame, gl_TexCoord [0].xy).rgb, vec3 (0.15, 0.7, 0.15)), cl.b, 1.0);\r\n" \
		"}"
	#else
		// amber/blue
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"void main() {\r\n" \
		"vec3 c = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float s = min (1.0 - c.b, 0.3) / max (0.000001, c.r + c.g);\r\n" \
		"gl_FragColor = vec4 (texture2D (leftFrame, gl_TexCoord [0].xy).xy, dot (c, vec3 (c.r * s, c.g * s, 1.0)), 1.0);\r\n" \
		"}",
		// red/cyan
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"void main() {\r\n" \
		"vec3 c = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float s = min (1.0 - c.r, 0.3) / max (0.000001, c.g + c.b);\r\n" \
		"gl_FragColor = vec4 (dot (c, vec3 (1.0, c.g * s, c.b * s)), texture2D (rightFrame, gl_TexCoord [0].xy).yz, 1.0);\r\n" \
		"}",
		// green/magenta
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"void main() {\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"vec3 cr = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float s = min (1.0 - cl.g, 0.3) / max (0.000001, cl.r + cl.b);\r\n" \
		"gl_FragColor = vec4 (cr.r, dot (cl, vec3 (cl.r * s, 1.0, cl.b * s)), cr.b, 1.0);\r\n" \
		"}"
#endif
		},
		{
		// amber/blue
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"uniform float gain;\r\n" \
		"void main() {\r\n" \
		"vec3 cr = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float dr = (1.0 - cr.b) /  max (0.000001, cr.r + cr.g) / gain;\r\n" \
		"float dl = 1.0 + cl.b /  max (0.000001, cl.r + cl.g) / gain;\r\n" \
		"gl_FragColor = vec4 (cl.r * dl, cl.g * dl, dot (cr, vec3 (dr * cr.r, dr * cr.g, 1.0)), 1.0);\r\n" \
		"}",
		// red/cyan
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"uniform float gain;\r\n" \
		"void main() {\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"vec3 cr = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float dl = (1.0 - cl.r) / max (0.000001, cl.g + cl.b) / gain;\r\n" \
		"float dr = 1.0 + cr.r /  max (0.000001, cr.g + cr.b) / gain;\r\n" \
		"gl_FragColor = vec4 (dot (cl, vec3 (1.0, dl * cl.g, dl * cl.b)), cr.g * dr, cr.b * dr, 1.0);\r\n" \
		"}",
		// green/magenta
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"uniform float gain;\r\n" \
		"void main() {\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"vec3 cr = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float dl = (1.0 - cl.g) / max (0.000001, cl.r + cl.b) / gain;\r\n" \
		"float dr = 1.0 + cr.g /  max (0.000001, cr.r + cr.b) / gain;\r\n" \
		"gl_FragColor = vec4 (cr.r * dr, dot (cl, vec3 (dl * cl.r, 1.0, dl * cl.b)), cr.b * dr, 1.0);\r\n" \
		"}"
		},
	},
	// with de-ghosting
	{
		// no color adjustment
		{
		// amber/blue
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"uniform vec2 strength;\r\n" \
		"void main() {\r\n" \
		"vec3 c = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float s = min (1.0 - c.b, 0.3) / max (0.000001, c.r + c.g);\r\n" \
		"vec3 h = vec3 (texture2D (leftFrame, gl_TexCoord [0].xy).xy, dot (c, vec3 (c.r * s, c.g * s, 1.0)));\r\n" \
		"vec3 l = h * vec3 (0.3, 0.59, 0.11);\r\n" \
		"float t = (l.r + l.g + l.b);\r\n" \
		"s = strength [0] / max (strength [0], (l.g + l.r) / t);\r\n" \
		"t = strength [1] / max (strength [1], l.b / t);\r\n" \
		"gl_FragColor = vec4 (h.r * s, h.g * s, h.b * t, 1.0);\r\n" \
		"}",
		// red/cyan
		// de-ghosting by reducing red or cyan brightness if they exceed a certain threshold compared to the pixels total brightness
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"uniform vec2 strength;\r\n" \
		"void main() {\r\n" \
		"vec3 c = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float s = min (1.0 - c.r, 0.3) / max (0.000001, c.g + c.b);\r\n" \
		"vec3 h = vec3 (dot (c, vec3 (1.0, c.g * s, c.b * s)), texture2D (rightFrame, gl_TexCoord [0].xy).yz);\r\n" \
		"vec3 l = h * vec3 (0.3, 0.59, 0.11);\r\n" \
		"float t = (l.r + l.g + l.b);\r\n" \
		"s = strength [0] / max (strength [0], (l.g + l.b) / t);\r\n" \
		"t = strength [1] / max (strength [1], l.r / t);\r\n" \
		"gl_FragColor = vec4 (h.r * t, h.g * s, h.b * s, 1.0);\r\n" \
		"}",
		// green/magenta
		// de-ghosting by reducing red or cyan brightness if they exceed a certain threshold compared to the pixels total brightness
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"uniform vec2 strength;\r\n" \
		"void main() {\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"vec3 cr = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float s = min (1.0 - cl.g, 0.3) / max (0.000001, cl.r + cl.b);\r\n" \
		"vec3 h = vec3 (cr.r, dot (cl, vec3 (cl.r * s, 1.0, cl.b * s)), cr.b);\r\n" \
		"vec3 l = h * vec3 (0.3, 0.59, 0.11);\r\n" \
		"float t = (l.r + l.g + l.b);\r\n" \
		"s = strength [0] / max (strength [0], (l.r + l.b) / t);\r\n" \
		"t = strength [1] / max (strength [1], l.g / t);\r\n" \
		"gl_FragColor = vec4 (h.r * s, h.g * t, h.b * s, 1.0);\r\n" \
		"}"
		},
		// color adjustment
		{
		// amber/blue
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"uniform vec2 strength;\r\n" \
		"uniform float gain;\r\n" \
		"void main() {\r\n" \
		"vec3 cr = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float dr = (1.0 - cr.b) / max (0.000001, cr.r + cr.g) / gain;\r\n" \
		"float dl = 1.0 + cl.b /  max (0.000001, cl.r + cl.g) / gain;\r\n" \
		"vec3 h = vec3 (cl.r * dl, cl.g * dl, dot (cr, vec3 (dr * cr.r, dr * cr.g, 1.0)));\r\n" \
		"vec3 l = h * vec3 (0.3, 0.59, 0.11);\r\n" \
		"float t = (l.r + l.g + l.b);\r\n" \
		"float s = strength [0] / max (strength [0], (l.g + l.r) / t);\r\n" \
		"t = strength [1] / max (strength [1], l.b / t);\r\n" \
		"gl_FragColor = vec4 (h.r * s, h.g * s, h.b * t, 1.0);\r\n" \
		"}",
		// red/cyan
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"uniform vec2 strength;\r\n" \
		"uniform float gain;\r\n" \
		"void main() {\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"vec3 cr = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float dl = (1.0 - cl.r) / max (0.000001, cl.g + cl.b) / gain;\r\n" \
		"float dr = 1.0 + cr.r /  max (0.000001, cr.g + cr.b) / gain;\r\n" \
		"vec3 h = vec3 (dot (cl, vec3 (1.0, dl * cl.g, dl * cl.b)), cr.g * dr, cr.b * dr);\r\n" \
		"vec3 l = h * vec3 (0.3, 0.59, 0.11);\r\n" \
		"float t = (l.r + l.g + l.b);\r\n" \
		"float s = strength [0] / max (strength [0], (l.g + l.b) / t);\r\n" \
		"t = strength [1] / max (strength [1], l.r / t);\r\n" \
		"gl_FragColor = vec4 (h.r * t, h.g * s, h.b * s, 1.0);\r\n" \
		"}",
		// red/cyan
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"uniform vec2 strength;\r\n" \
		"uniform float gain;\r\n" \
		"void main() {\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"vec3 cr = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"float dl = (1.0 - cl.g) / max (0.000001, cl.r + cl.b) / gain;\r\n" \
		"float dr = 1.0 + cr.g /  max (0.000001, cr.r + cr.b) / gain;\r\n" \
		"vec3 h = vec3 (cr.r * dr, dot (cl, vec3 (dl * cl.r, 1.0, dl * cl.b)), cr.b * dr);\r\n" \
		"vec3 l = h * vec3 (0.3, 0.59, 0.11);\r\n" \
		"float t = (l.r + l.g + l.b);\r\n" \
		"float s = strength [0] / max (strength [0], (l.r + l.b) / t);\r\n" \
		"t = strength [1] / max (strength [1], l.g / t);\r\n" \
		"gl_FragColor = vec4 (h.r * s, h.g * t, h.b * s, 1.0);\r\n" \
		"}"
		}
	}
};

const char* duboisFS = 
#if 0
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"vec3 rl = vec3 ( 0.4561000,  0.5004840,  0.17638100);\r\n" \
		"vec3 gl = vec3 (-0.0400822, -0.0378246, -0.01575890);\r\n" \
		"vec3 bl = vec3 (-0.0152161, -0.0205971, -0.00546856);\r\n" \
		"vec3 rr = vec3 (-0.0434706, -0.0879388, -0.00155529);\r\n" \
		"vec3 gr = vec3 ( 0.3784760,  0.7336400, -0.01845030);\r\n" \
		"vec3 br = vec3 (-0.0721527, -0.1129610,  1.22640000);\r\n" \
		"void main() {\r\n" \
		"vec3 cl = texture2D (leftFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"vec3 cr = texture2D (rightFrame, gl_TexCoord [0].xy).rgb;\r\n" \
		"gl_FragColor = vec4 (clamp (dot (cl, rl) + dot (cr, rr), 0.0, 1.0),\r\n" \
		"                     clamp (dot (cl, gl) + dot (cr, gr), 0.0, 1.0),\r\n" \
		"                     clamp (dot (cl, bl) + dot (cr, br), 0.0, 1.0), 1.0);\r\n" \
		"}";
#else
		"uniform sampler2D leftFrame, rightFrame;\r\n" \
		"mat3 lScale = mat3 ( 0.4561000,  0.5004840,  0.17638100,\r\n" \
		"                    -0.0400822, -0.0378246, -0.01575890,\r\n" \
		"                    -0.0152161, -0.0205971, -0.00546856);\r\n" \
		"mat3 rScale = mat3 (-0.0434706, -0.0879388, -0.00155529,\r\n" \
		"                     0.3784760,  0.7336400, -0.01845030,\r\n" \
		"                    -0.0721527, -0.1129610,  1.22640000);\r\n" \
		"void main() {\r\n" \
		"gl_FragColor = vec4 (clamp (texture2D (leftFrame, gl_TexCoord [0].xy).rgb * lScale + texture2D (rightFrame, gl_TexCoord [0].xy).rgb * rScale,\r\n" \
		"                            vec3 (0.0, 0.0, 0.0), vec3 (1.0, 1.0, 1.0)), 1.0);\r\n" \
		"}";
#endif

const char* enhance3DVS = 
	"void main(void){" \
	"gl_TexCoord [0]=gl_MultiTexCoord0;"\
	"gl_Position=ftransform();"\
	"gl_FrontColor=gl_Color;}"
	;

//------------------------------------------------------------------------------

void COGL::InitEnhanced3DShader (void)
{
if (gameOpts->render.bUseShaders && m_features.bShaders.Available ()) {
	PrintLog (0, "building dubois optimization programs\n");
	gameStates.render.textures.bHaveEnhanced3DShader = (0 <= shaderManager.Build (duboisShaderProg, duboisFS, enhance3DVS));
	if (!gameStates.render.textures.bHaveEnhanced3DShader) {
		DeleteEnhanced3DShader ();
		gameOpts->render.stereo.nGlasses = GLASSES_NONE;
		return;
		}
	PrintLog (0, "building enhanced 3D shader programs\n");
	for (int h = 0; h < 2; h++) {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 3; j++) {
				gameStates.render.textures.bHaveEnhanced3DShader = (0 <= shaderManager.Build (enhance3DShaderProg [h][i][j], enhance3DFS [h][i][j], enhance3DVS));
				if (!gameStates.render.textures.bHaveEnhanced3DShader) {
					DeleteEnhanced3DShader ();
					gameOpts->render.stereo.nGlasses = GLASSES_NONE;
					return;
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------

void COGL::DeleteEnhanced3DShader (void)
{
if (duboisShaderProg >= 0) {
	shaderManager.Delete (duboisShaderProg);
	for (int h = 0; h < 2; h++) {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 3; j++) {
				shaderManager.Delete (enhance3DShaderProg [h][i][j]);
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
