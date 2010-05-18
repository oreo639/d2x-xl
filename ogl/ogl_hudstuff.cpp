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
#include "maths.h"
#include "mouse.h"
#include "input.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_texture.h"
#include "ogl_bitmap.h"
#include "ogl_render.h"
#include "ogl_hudstuff.h"
#include "renderlib.h"
#include "cockpit.h"
#include "addon_bitmaps.h"

//------------------------------------------------------------------------------

void OglDrawMouseIndicator (void)
{
	float 	scale = float (screen.Width ()) / float (screen.Height ());

	static tSinCosf sinCos30 [30];
	static tSinCosf sinCos12 [12];
	static int bInitSinCos = 1;

	static tCanvasColor color = {-1, 1, {255, 255, 255, 96}};

	float	r, w, h;

if (!gameOpts->input.mouse.nDeadzone)
	return;
if (bInitSinCos) {
	ComputeSinCosTable (sizeofa (sinCos30), sinCos30);
	ComputeSinCosTable (sizeofa (sinCos12), sinCos12);
	bInitSinCos = 0;
	}
#if 0
if (mouseIndList)
glCallList (mouseIndList);
else {
	glNewList (mouseIndList, GL_COMPILE_AND_EXECUTE);
#endif
	if (joyMouse.Load ()) {
		color.color.alpha = 255;
		joyMouse.Bitmap ()->RenderScaled (mouseData.x - 8, mouseData.y - 8, 16, 16, I2X (1), 0, &color);
		color.color.alpha = 96;
		}
	else {
		glPushMatrix ();
		glTranslatef ((float) (mouseData.x) / (float) screen.Width (), 1.0f - (float) (mouseData.y) / (float) screen.Height (), 0);
		glScalef (scale / 320.0f, scale / 200.0f, scale);//the positions are based upon the standard reticle at 320x200 res.
		ogl.SetTexturing (false);
		ogl.SetLineSmooth (true);
		glColor4f (1.0f, 0.8f, 0.0f, 0.9f);
		glLineWidth (3);
		OglDrawEllipse (12, GL_LINE_LOOP, 1.5f, 0, 1.5f * float (screen.Height ()) / float (screen.Width ()), 0, sinCos12);
		glPopMatrix ();
		}
	if (deadzone.Load ()) {
		r = float (controls.CalcDeadzone (0, gameOpts->input.mouse.nDeadzone));
		w = r / float (screen.Width ());
		h = r / float (screen.Height ());
		ogl.SetBlendMode (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPushMatrix ();
		glTranslatef (0.5f, 0.5f, 0);
		glColor4f (1.0f, 1.0f, 1.0f, 0.8f / (float) gameOpts->input.mouse.nDeadzone);
		CFloatVector vPosf;
		vPosf [X] =
		vPosf [Y] = 0;
		tTexCoord2f texCoord [4] = {{{0,0}},{{1,0}},{{1,1}},{{0,1}}};
		deadzone.Bitmap ()->SetTexCoord (texCoord);
		ogl.RenderQuad (deadzone.Bitmap (), vPosf, w, h);
		ogl.BindTexture (0);
		ogl.SetTexturing (false);
		glPopMatrix ();
		}
	else {
		deadzone.Load ();
		glPushMatrix ();
		glTranslatef (0.5f, 0.5f, 0);
		glScalef (scale / 320.0f, scale / 200.0f, scale);	//the positions are based upon the standard reticle at 320x200 res.
		ogl.SetBlendMode (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f (1.0f, 0.8f, 0.0f, 1.0f / (3.0f + 0.5f * gameOpts->input.mouse.nDeadzone));
		glLineWidth (4); //(GLfloat) (4 + 2 * gameOpts->input.mouse.nDeadzone));
		r = float (controls.CalcDeadzone (0, gameOpts->input.mouse.nDeadzone)) / 4;
		ogl.SetTexturing (false);
		OglDrawEllipse (30, GL_LINES, r, 0, r * float (screen.Height ()) / float (screen.Width ()), 0, sinCos30);
		glPopMatrix ();
		}
	ogl.SetLineSmooth (false);
	glLineWidth (1);
#if 0
	glEndList ();
   }
#endif
}

//------------------------------------------------------------------------------
//eof
