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

tTexCoord2f quadTexCoord [4] = {{{0,0}},{{1,0}},{{1,1}},{{0,1}}};
float quadVerts [4][2] = {{0,0},{1,0},{1,1},{0,1}};

//------------------------------------------------------------------------------

void COGL::SwapBuffers (int bForce, int bClear)
{
if (!gameStates.menus.nInMenu || bForce) {
#	if PROFILING
	if (gameStates.render.bShowProfiler && gameStates.app.bGameRunning && !gameStates.menus.nInMenu && fontManager.Current () && SMALL_FONT) {
		static time_t t0 = -1000;
		time_t t1 = clock ();
		static tProfilerData p;
		static float nFrameCount = 1;
		if (t1 - t0 >= 1000) {
			memcpy (&p, &gameData.profiler, sizeof (p));
			nFrameCount = float (gameData.app.nFrameCount);
			t0 = t1;
			}
		int h = SMALL_FONT->Height () + 3, i = 3;
		fontManager.SetColorRGBi (ORANGE_RGBA, 1, 0, 0);
		float t, s = 0;
		GrPrintF (NULL, 5, h * i++, "frame: %1.2f", float (p.t [ptFrame]) / nFrameCount);
		if (p.t [ptRenderFrame]) {
			GrPrintF (NULL, 5, h * i++, "  scene: %1.2f %c (%1.2f)", 100.0f * float (p.t [ptRenderFrame]) / float (p.t [ptFrame]), '%', float (p.t [ptRenderFrame]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "    mine: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptRenderMine]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptRenderMine]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "    light: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptLighting]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptLighting]) / nFrameCount);
			s += t;
			GrPrintF (NULL, 5, h * i++, "    render: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptRenderPass]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptRenderPass]) / nFrameCount);
			s += t;
			GrPrintF (NULL, 5, h * i++, "      face list: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptFaceList]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptFaceList]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "      faces: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptRenderFaces]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptRenderFaces]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "      objects: %1.2f %c (%1.2f) ", t = 100.0f * float (p.t [ptRenderObjects]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptRenderObjects]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "      transparency: %1.2f %c (%1.2f) ", t = 100.0f * float (p.t [ptTranspPolys]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptTranspPolys]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "      effects: %1.2f %c (%1.2f) ", t = 100.0f * float (p.t [ptEffects]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptEffects]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "        particles: %1.2f %c (%1.2f) ", t = 100.0f * float (p.t [ptParticles]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptParticles]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "      cockpit: %1.2f %c (%1.2f) ", t = 100.0f * float (p.t [ptCockpit]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptCockpit]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "      states: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptRenderStates]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptRenderStates]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "      shaders: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptShaderStates]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptShaderStates]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "    other: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptAux]) / float (p.t [ptRenderFrame]), '%');
			s += t;
			GrPrintF (NULL, 5, h * i++, "      transform: %1.2f %c (%1.2f) ", t = 100.0f * float (p.t [ptTransform]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptTransform]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "      seg list: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptBuildSegList]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptBuildSegList]) / nFrameCount);
			GrPrintF (NULL, 5, h * i++, "      obj list: %1.2f %c (%1.2f)", t = 100.0f * float (p.t [ptBuildObjList]) / float (p.t [ptRenderFrame]), '%', float (p.t [ptBuildObjList]) / nFrameCount);
			}
		GrPrintF (NULL, 5, h * i++, "  total: %1.2f %c", s, '%');
		}
#endif
#if 0
	if (gameStates.app.bGameRunning && !gameStates.menus.nInMenu)
		paletteManager.RenderEffect ();
#endif
	glowRenderer.End ();
	if ((gameStates.render.bRenderIndirect > 0) && !gameStates.menus.nInMenu) {
		FlushDrawBuffer ();
		//SelectDrawBuffer (0);
		gameStates.render.bRenderIndirect = 0;
		Draw2DFrameElements ();
		gameStates.render.bRenderIndirect = 1;
		//SetDrawBuffer (GL_BACK, 1);
		}
#if 0
	if (!gameStates.menus.nInMenu && gameStates.app.bGameRunning) {
		EnableClientStates (1, 0, 0, GL_TEXTURE0);
		OglTexCoordPointer (2, GL_FLOAT, 0, quadTexCoord);
		OglVertexPointer (2, GL_FLOAT, 0, quadVerts);
		BindTexture (m_states.hDepthBuffer [0]);
		SetDrawBuffer (GL_BACK, 0);
		OglDrawArrays (GL_QUADS, 0, 4);
		}
#endif
	SDL_GL_SwapBuffers ();
	if (gameStates.app.bSaveScreenShot)
		SaveScreenShot (NULL, 0);
	SetDrawBuffer (GL_BACK, gameStates.render.bRenderIndirect > 0);
#if 1
	if (gameStates.menus.nInMenu || bClear)
		glClear (GL_COLOR_BUFFER_BIT);
#endif
	}
}

//------------------------------------------------------------------------------

void COGL::FlushEffects (int nEffects)
{
//ogl.EnableClientStates (1, 0, 0, GL_TEXTURE1);
if (nEffects & 5) {
	postProcessManager.Setup ();
	ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
	ogl.BindTexture (DrawBuffer ((nEffects & 2) ? 1 : 0)->ColorBuffer ());
	OglTexCoordPointer (2, GL_FLOAT, 0, quadTexCoord);
	OglVertexPointer (2, GL_FLOAT, 0, quadVerts);
	if ((nEffects & 4) == 0)
		SetDrawBuffer (GL_BACK, 0);
	else {
		SelectDrawBuffer (1);
		//SetDrawBuffer (GL_BACK, 1);
		}
	postProcessManager.Render ();
	}
}

//------------------------------------------------------------------------------

void COGL::FlushDrawBuffer (bool bAdditive)
{
if (HaveDrawBuffer ()) {
	int nEffects = postProcessManager.HaveEffects () 
						+ (int (m_data.xStereoSeparation > 0) << 1)
#if MAX_SHADOWMAPS
						+ (int (EGI_FLAG (bShadows, 0, 1, 0) != 0) << 2)
#endif
						;

	glColor3f (1,1,1);

	EnableClientStates (1, 0, 0, GL_TEXTURE0);
	OglTexCoordPointer (2, GL_FLOAT, 0, quadTexCoord);
	OglVertexPointer (2, GL_FLOAT, 0, quadVerts);
	BindTexture (DrawBuffer (0)->ColorBuffer ());

	if (nEffects) {
		FlushStereoBuffers (nEffects);
		FlushEffects (nEffects);
#if MAX_SHADOWMAPS
		FlushShadowMaps (nEffects);
#endif
		}
	else {
		SetDrawBuffer (GL_BACK, 0);
		OglDrawArrays (GL_QUADS, 0, 4);
		}
	ResetClientStates (0);
	SelectDrawBuffer (0);
	//if (bStereo)
		shaderManager.Deploy (-1);
	}
}

//------------------------------------------------------------------------------
