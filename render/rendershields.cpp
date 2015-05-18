#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <string.h>	// for memset
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "descent.h"
#include "newdemo.h"
#include "network.h"
#include "interp.h"
#include "ogl_lib.h"
#include "rendermine.h"
#include "transprender.h"
#include "glare.h"
#include "sphere.h"
#include "marker.h"
#include "fireball.h"
#include "objsmoke.h"
#include "objrender.h"
#include "objeffects.h"
#include "hiresmodels.h"
#include "hitbox.h"

#ifndef fabsf
#	define fabsf(_f)	(float) fabs (_f)
#endif

//------------------------------------------------------------------------------

void TransformHitboxf (CObject *pObj, CFloatVector *vertList, int32_t iSubObj)
{

	CFloatVector		hv;
	tHitbox*				phb = gameData.models.hitboxes [pObj->ModelId ()].hitboxes + iSubObj;
	CFixVector			vMin = phb->vMin;
	CFixVector			vMax = phb->vMax;
	int32_t				i;

for (i = 0; i < 8; i++) {
	hv.v.coord.x = X2F (hitBoxOffsets [i].v.coord.x ? vMin.v.coord.x : vMax.v.coord.x);
	hv.v.coord.y = X2F (hitBoxOffsets [i].v.coord.y ? vMin.v.coord.y : vMax.v.coord.y);
	hv.v.coord.z = X2F (hitBoxOffsets [i].v.coord.z ? vMin.v.coord.z : vMax.v.coord.z);
	transformation.Transform (vertList [i], hv, 0);
	}
}

//------------------------------------------------------------------------------

#if RENDER_HITBOX

static int32_t nDbgBox = -1, nDbgBoxFace = -1;

void RenderHitbox (CObject *pObj, float red, float green, float blue, float alpha)
{
if (pObj->rType.polyObjInfo.nModel < 0)
	return;

	CFloatVector	dir;
	tHitbox*			pmhb = gameData.models.hitboxes [pObj->ModelId ()].hitboxes.Buffer ();
	tCloakInfo		ci = {0, FADE_LEVELS, 0, 0, 0, 0, 0};
	int32_t			i, j, iBox, nBoxes, bHit = 0;
	float				fFade;

if (!SHOW_OBJ_FX)
	return;
if (pObj->info.nType == OBJ_PLAYER) {
	if (!EGI_FLAG (nShieldEffect, 0, 1, gameOpts->render.effects.bShields))
		return;
	if (PLAY (pObj->info.nId).flags & PLAYER_FLAGS_CLOAKED) {
		if (!GetCloakInfo (pObj, 0, 0, &ci))
			return;
		fFade = (float) ci.nFadeValue / (float) FADE_LEVELS;
		red *= fFade;
		green *= fFade;
		blue *= fFade;
		}

	}
else if (pObj->info.nType == OBJ_ROBOT) {
	if (!(gameOpts->render.effects.bEnabled && gameOpts->render.effects.bShields > 1))
		return;
	if (pObj->cType.aiInfo.CLOAKED) {
		if (!GetCloakInfo (pObj, 0, 0, &ci))
			return;
		fFade = (float) ci.nFadeValue / (float) FADE_LEVELS;
		red *= fFade;
		green *= fFade;
		blue *= fFade;
		}
	}
if (!CollisionModel ()) {
	DrawShieldSphere (pObj, red, green, blue, alpha, 1);
	return;
	}
else if (CollisionModel () == 1) {
	iBox =
	nBoxes = 0;
	}
else {
	iBox = 1;
	nBoxes = gameData.models.hitboxes [pObj->ModelId ()].nHitboxes;
	}
ogl.SetDepthMode (GL_LEQUAL);
ogl.SetBlending (true);
ogl.SetBlendMode (OGL_BLEND_ALPHA);
ogl.SetTexturing (false);
ogl.SetDepthWrite (false);

tHitbox* hb = TransformHitboxes (pObj, &pObj->info.position.vPos);

for (; iBox <= nBoxes; iBox++) {
#if 0
	if (iBox)
		transformation.Begin (pmhb [iBox].vOffset, CAngleVector::ZERO);
	TransformHitboxf (pObj, vertList, iBox);
#endif
	if ((pObj->info.nType == OBJ_PLAYER) && (iBox == nDbgBox))
		glColor4f (1.0f, 0, 0, alpha);
	else
		glColor4f (red, green, blue, alpha / 2);
	glBegin (GL_QUADS);
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++) {
			dir.Assign (hb [iBox].box.faces [i].v [j]);
			transformation.Transform (dir, dir, 0);
			glVertex3fv (reinterpret_cast<GLfloat*> (&dir));
			}
		}
	glEnd ();
	glLineWidth (3);
	if ((pObj->info.nType == OBJ_PLAYER) && (iBox == nDbgBox))
		glColor4f (1.0f, 0, 0, alpha);
	else
		glColor4f (red, green, blue, alpha);
	for (i = 0; i < 6; i++) {
		CFixVector coord;
		coord.SetZero ();
		if ((pObj->info.nType == OBJ_PLAYER) && (iBox == nDbgBox))
			glColor4f (1.0f, 0, 0, alpha);
		else
			glColor4f (red, green, blue, alpha);
		glBegin (GL_LINE_LOOP);
		for (j = 0; j < 4; j++) {
			coord += hb [iBox].box.faces [i].v [j];
			dir.Assign (hb [iBox].box.faces [i].v [j]);
			transformation.Transform (dir, dir, 0);
			glVertex3fv (reinterpret_cast<GLfloat*> (&dir));
			}
		glEnd ();
		glBegin (GL_LINES);
		coord /= I2X (4);
		dir.Assign (coord);
		glColor4f (1.0f, 0.5f, 0.0f, alpha);
		transformation.Transform (dir, dir, 0);
		glVertex3fv (reinterpret_cast<GLfloat*> (&dir));
		dir.Assign (coord + hb [iBox].box.faces [i].n [1] * I2X (2));
		transformation.Transform (dir, dir, 0);
		glVertex3fv (reinterpret_cast<GLfloat*> (&dir));
		glEnd ();
		}
	glLineWidth (1);
	}
float r = X2F (CFixVector::Dist (pmhb->vMin, pmhb->vMax) / 2);
#if 0 //DBG //display collision point
if (gameStates.app.nSDLTicks [0] - gameData.models.hitboxes [pObj->ModelId ()].tHit < 500) {
	CObject	o;

	o.info.position.vPos = gameData.models.hitboxes [pObj->ModelId ()].vHit;
	o.info.position.mOrient = pObj->info.position.mOrient;
	o.SetSize (I2X (2));
	int32_t nModel = pObj->rType.polyObjInfo.nModel;
	pObj->rType.polyObjInfo.nModel = -1;
	pObj->rType.polyObjInfo.nModel = nModel;
	//SetupRenderView (0, NULL);
	DrawShieldSphere (&o, 1, 0, 0, 0.33f, 1);
	}
#endif
ogl.SetDepthWrite (true);
ogl.SetDepthMode (GL_LESS);
}

#endif

// -----------------------------------------------------------------------------

void RenderPlayerShield (CObject *pObj)
{
	int32_t		bStencil, dt = 0, i = pObj->info.nId, nColor = 0;
	float			alpha, scale = 1;
	tCloakInfo	ci;

	static CFloatVector shieldColors [3] = {{{{0, 0.5f, 1, 1}}}, {{{1, 0.5f, 0, 1}}}, {{{1, 0.8f, 0.6f, 1}}}};

if (!SHOW_OBJ_FX)
	return;
if (SHOW_SHADOWS &&
	 (FAST_SHADOWS ? (gameStates.render.nShadowPass != 1) : (gameStates.render.nShadowPass != 3)))
	return;

bool bAppearing = pObj->Appearing ();
if (bAppearing || EGI_FLAG (nShieldEffect, 0, 1, gameOpts->render.effects.bShields)) {
	if (PLAYER (i).flags & PLAYER_FLAGS_CLOAKED) {
		if (!GetCloakInfo (pObj, 0, 0, &ci))
			return;
		scale = (float) ci.nFadeValue / (float) FADE_LEVELS;
		scale *= scale;
		}
	bStencil = ogl.StencilOff ();
	CPulseData *pPulse = gameData.render.shield->SetPulse (gameData.multiplayer.spherePulse + i);
	if (gameData.multiplayer.bWasHit [i]) {
		if (gameData.multiplayer.bWasHit [i] < 0) {
			gameData.multiplayer.bWasHit [i] = 1;
			gameData.multiplayer.nLastHitTime [i] = gameStates.app.nSDLTicks [0];
			SetupSpherePulse (gameData.multiplayer.spherePulse + i, 0.1f, 0.5f);
			dt = 0;
			}
		else if ((dt = gameStates.app.nSDLTicks [0] - gameData.multiplayer.nLastHitTime [i]) >= 300) {
			gameData.multiplayer.bWasHit [i] = 0;
			SetupSpherePulse (gameData.multiplayer.spherePulse + i, 0.02f, 0.4f);
			}
		}
#if !RENDER_HITBOX
#	if DBG == 0
	if (!bAppearing && (gameOpts->render.effects.bOnlyShieldHits && !gameData.multiplayer.bWasHit [i]))
		return;
#	endif
#endif
	if (bAppearing)
		nColor = 1;
	else if (PLAYER (i).flags & PLAYER_FLAGS_INVULNERABLE)
		nColor = 2;
	else if (gameData.multiplayer.bWasHit [i])
		nColor = 0;
	else
		nColor = 0;
	if (gameData.multiplayer.bWasHit [i]) {
		alpha = ((!bAppearing && gameOpts->render.effects.bOnlyShieldHits) ? (float) cos (sqrt ((double) dt / float (SHIELD_EFFECT_TIME)) * PI / 2) : 1);
		scale *= alpha;
		}
	else if (PLAYER (i).flags & PLAYER_FLAGS_INVULNERABLE)
		alpha = 1;
	else {
		alpha = X2F (PLAYER (i).Shield ()) / X2F (PLAYER (i).MaxShield ());
		scale *= alpha;
		if (gameData.multiplayer.spherePulse [i].fSpeed == 0.0f)
			SetupSpherePulse (gameData.multiplayer.spherePulse + i, 0.02f, 0.5f);
		}
#if RENDER_HITBOX
	RenderHitbox (pObj, shieldColors [nColor].Red () * scale, shieldColors [nColor].Green () * scale, shieldColors [nColor].Blue () * scale, alpha);
#else
	DrawShieldSphere (pObj, shieldColors [nColor].Red () * scale, shieldColors [nColor].Green () * scale, shieldColors [nColor].Blue () * scale, alpha, 1);
#endif
	gameData.render.shield->SetPulse (pPulse);
	ogl.StencilOn (bStencil);
	}
}

// -----------------------------------------------------------------------------

void RenderRobotShield (CObject *pObj)
{
	static CFloatVector shieldColors [3] = {{{{0.75f, 0, 0.75f, 1}}}, {{{0, 0.5f, 1}}}, {{{1, 0.1f, 0.25f, 1}}}};
	static CPulseData shieldPulse;

#if RENDER_HITBOX
RenderHitbox (pObj, 0.5f, 0.0f, 0.6f, 0.4f);
#else
#endif
#if 1
	float			scale = 1;
	tCloakInfo	ci;
	fix			dt;

if (!(gameOpts->render.effects.bEnabled && gameOpts->render.effects.bShields > 1))
	return;
if ((pObj->info.nType == OBJ_ROBOT) && pObj->cType.aiInfo.CLOAKED) {
	if (!GetCloakInfo (pObj, 0, 0, &ci))
		return;
	scale = (float) ci.nFadeValue / (float) FADE_LEVELS;
	scale *= scale;
	}
CPulseData *pPulse = gameData.render.shield->SetPulse (&shieldPulse);
dt = gameStates.app.nSDLTicks [0] - pObj->TimeLastHit ();
if (dt < SHIELD_EFFECT_TIME) {
	scale *= gameOpts->render.effects.bOnlyShieldHits ? float (cos (sqrt (float (dt) / float (SHIELD_EFFECT_TIME)) * PI / 2)) : 1;
	DrawShieldSphere (pObj, shieldColors [2].Red () * scale, shieldColors [2].Green () * scale, shieldColors [2].Blue () * scale, 0.5f * scale, 1);
	}
else if (!gameOpts->render.effects.bOnlyShieldHits) {
	if ((pObj->info.nType != OBJ_ROBOT) || pObj->IsGuideBot ())
		DrawShieldSphere (pObj, 0.0f, 0.5f * scale, 1.0f * scale, pObj->Damage () / 2 * scale, 1);
	else
		DrawShieldSphere (pObj, 0.75f * scale, 0.0f, 0.75f * scale, pObj->Damage () / 2 * scale, 1);
	}
#endif
}

//------------------------------------------------------------------------------
//eof
