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

//------------------------------------------------------------------------------

void COGL::CreateDrawBuffer (int nType)
{
if (!m_states.bRender2TextureOk)
	return;
if (!gameStates.render.bRenderIndirect && (nType >= 0))
	return;
if (DrawBuffer ()->Handle ())
	return;
PrintLog ("creating draw buffer\n");
DrawBuffer ()->Create (m_states.nCurWidth, m_states.nCurHeight, nType);
}

//------------------------------------------------------------------------------

void COGL::DestroyDrawBuffer (void)
{
#	if 1
	static int bSemaphore = 0;

if (bSemaphore)
	return;
bSemaphore++;
#	endif
if (m_states.bRender2TextureOk && DrawBuffer () && DrawBuffer ()->Handle ()) {
	SetDrawBuffer (GL_BACK, 0);
	DrawBuffer ()->Destroy ();
	}
#	if 1
bSemaphore--;
#	endif
}

//------------------------------------------------------------------------------

void COGL::DestroyDrawBuffers (void)
{
for (int i = m_data.drawBuffers.Length () - 1; i > 0; i--) {
	if (m_data.drawBuffers [i].Handle ()) {
		SelectDrawBuffer (i);
		DestroyDrawBuffer ();
		}
	}
}

//------------------------------------------------------------------------------

void COGL::SetDrawBuffer (int nBuffer, int bFBO)
{
#if 1
	static int bSemaphore = 0;

if (bSemaphore)
	return;
bSemaphore++;
#endif

if (bFBO && (nBuffer == GL_BACK) && m_states.bRender2TextureOk && DrawBuffer ()->Handle ()) {
	if (!DrawBuffer ()->Active ()) {
		if (DrawBuffer ()->Enable ()) {
			glDrawBuffer (GL_COLOR_ATTACHMENT0_EXT);
			}
		else {
			DestroyDrawBuffers ();
			SelectDrawBuffer (0);
			glDrawBuffer (GL_BACK);
			}
		}
	}
else {
	if (DrawBuffer ()->Active ()) {
		DrawBuffer ()->Disable ();
		}
	glDrawBuffer (m_states.nDrawBuffer = nBuffer);
	}

#if 1
bSemaphore--;
#endif
}

//------------------------------------------------------------------------------

void COGL::SetReadBuffer (int nBuffer, int bFBO)
{
if (bFBO && (nBuffer == GL_BACK) && m_states.bRender2TextureOk && DrawBuffer ()->Handle ()) {
	if (DrawBuffer ()->Active () || DrawBuffer ()->Enable ())
		glReadBuffer (GL_COLOR_ATTACHMENT0_EXT);
	else
		glReadBuffer (GL_BACK);
	}
else {
	if (DrawBuffer ()->Active ())
		DrawBuffer ()->Disable ();
	glReadBuffer (nBuffer);
	}
}

//------------------------------------------------------------------------------

int COGL::SelectDrawBuffer (int nBuffer) 
{ 
//if (gameStates.render.nShadowMap > 0)
//	nBuffer = gameStates.render.nShadowMap + 5;
int nPrevBuffer = (m_states.nCamera < 0) 
						? m_states.nCamera 
						: (m_data.drawBufferP && m_data.drawBufferP->Active ()) 
							? int (m_data.drawBufferP - m_data.drawBuffers.Buffer ()) 
							: 0x7FFFFFFF;
if (nBuffer != nPrevBuffer) {
	if (m_data.drawBufferP)
		m_data.drawBufferP->Disable (false);
	if (nBuffer >= 0) {
		m_states.nCamera = 0;
		m_data.drawBufferP = m_data.GetDrawBuffer (nBuffer); 
		CreateDrawBuffer ((nBuffer < 2) ? 1 : (nBuffer < 3) ? -1 : -2);
		}
	else {
		m_states.nCamera = nBuffer;
		m_data.drawBufferP = &cameraManager.Camera (-nBuffer - 1)->FrameBuffer ();
		}
	}
m_data.drawBufferP->Enable (false);
return nPrevBuffer;
}

//------------------------------------------------------------------------------

void COGL::ChooseDrawBuffer (void)
{
if (gameStates.render.bBriefing || gameStates.render.nWindow) {
	gameStates.render.bRenderIndirect = 0;
	SetDrawBuffer (GL_BACK, 0);
	}
else if (gameStates.render.cameras.bActive) {
	SelectDrawBuffer (-cameraManager.Current () - 1);
	gameStates.render.bRenderIndirect = 0;
	}
else {
	int i = Enhance3D ();
	if (i < 0) {
		ogl.ClearError (0);
		SetDrawBuffer ((m_data.xStereoSeparation < 0) ? GL_BACK_LEFT : GL_BACK_RIGHT, 0);
		if (ogl.ClearError (0))
			gameOpts->render.stereo.nGlasses = 0;
		}	
	else {
#if 0
		gameStates.render.bRenderIndirect = (ogl.m_states.bRender2TextureOk > 0); 
		if (gameStates.render.bRenderIndirect) 
			SelectDrawBuffer (m_data.xStereoSeparation > 0);
		else
			SetDrawBuffer (GL_BACK, 0);
#else
		gameStates.render.bRenderIndirect = (postProcessManager.Effects () != NULL) 
														|| (m_data.xStereoSeparation && (i > 0)) 
														|| (gameStates.render.textures.bHaveShadowMapShader && (EGI_FLAG (bShadows, 0, 1, 0) != 0));
		if (gameStates.render.bRenderIndirect) 
			SelectDrawBuffer ((i > 0) && (m_data.xStereoSeparation > 0));
		else
			SetDrawBuffer (GL_BACK, 0);
#endif
		}
	}
}

//------------------------------------------------------------------------------

void COGL::SelectGlowBuffer (void) 
{ 
SelectDrawBuffer (2);
SetDrawBuffer (GL_BACK, 1);
//glClear (GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT*/);
}

//------------------------------------------------------------------------------

void COGL::SelectBlurBuffer (int nBuffer) 
{ 
SelectDrawBuffer (nBuffer + 3);
SetDrawBuffer (GL_BACK, 1);
}

//------------------------------------------------------------------------------

