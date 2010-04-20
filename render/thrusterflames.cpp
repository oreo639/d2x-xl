#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <string.h>	// for memset
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "descent.h"
#include "network.h"
#include "interp.h"
#include "ogl_lib.h"
#include "hiresmodels.h"
#include "renderlib.h"
#include "transprender.h"
#include "thrusterflames.h"
#include "addon_bitmaps.h"

#ifndef fabsf
#	define fabsf(_f)	(float) fabs (_f)
#endif

CThrusterFlames thrusterFlames;

static tTexCoord2f tcCap [2][4] = {
	{{{0.0f,0.0f}},{{0.5f,0.0f}},{{0.5f,0.49609375f}},{{0.0f,0.49609375f}}},
	{{{0.5f,0.0f}},{{1.0f,0.0f}},{{1.0f,0.49609375f}},{{0.5f,0.49609375f}}}
	};

// -----------------------------------------------------------------------------

static CFloatVector	vRingVerts [RING_SEGS] = {
	CFloatVector::Create (-0.5f, -0.5f, 0.0f, 1.0f),
	CFloatVector::Create (-0.6533f, -0.2706f, 0.0f, 1.0f),
	CFloatVector::Create (-0.7071f, 0.0f, 0.0f, 1.0f),
	CFloatVector::Create (-0.6533f, 0.2706f, 0.0f, 1.0f),
	CFloatVector::Create (-0.5f, 0.5f, 0.0f, 1.0f),
	CFloatVector::Create (-0.2706f, 0.6533f, 0.0f, 1.0f),
	CFloatVector::Create (0.0f, 0.7071f, 0.0f, 1.0f),
	CFloatVector::Create (0.2706f, 0.6533f, 0.0f, 1.0f),
	CFloatVector::Create (0.5f, 0.5f, 0.0f, 1.0f),
	CFloatVector::Create (0.6533f, 0.2706f, 0.0f, 1.0f),
	CFloatVector::Create (0.7071f, 0.0f, 0.0f, 1.0f),
	CFloatVector::Create (0.6533f, -0.2706f, 0.0f, 1.0f),
	CFloatVector::Create (0.5f, -0.5f, 0.0f, 1.0f),
	CFloatVector::Create (0.2706f, -0.6533f, 0.0f, 1.0f),
	CFloatVector::Create (0.0f, -0.7071f, 0.0f, 1.0f),
	CFloatVector::Create (-0.2706f, -0.6533f, 0.0f, 1.0f)
};

static int nStripIdx [] = {0,15,1,14,2,13,3,12,4,11,5,10,6,9,7,8};

void CThrusterFlames::Create (void)
{
if (!m_bHaveFlame) {
		CFloatVector*	pv;
		int				i, j, m, n;
		double			phi, sinPhi;
		float				z = 0,
							fScale = 2.0f / 3.0f,
							fStep [2] = {1.0f / 4.0f, 1.0f / 3.0f};

	// first part with increasing diameter
	pv = &m_vFlame [0][0];
	for (i = 0, phi = 0; i < 5; i++, phi += Pi / 10, z -= fStep [0]) {
		sinPhi = (1 + sin (phi) / 2) * fScale;
		for (j = 0; j < RING_SEGS; j++, pv++) {
			*pv = vRingVerts [j] * float (sinPhi);
			(*pv) [Z] = z;
			}
		}
	// second part with decreasing diameter
	m = n = THRUSTER_SEGS - i + 1;
	for (phi = Pi / 2; i < THRUSTER_SEGS; i++, phi += Pi / 12, z -= fStep [1], m--) {
		sinPhi = (1 + sin (phi) / 2) * fScale /** m / n*/;
		for (j = 0; j < RING_SEGS; j++, pv++) {
			*pv = vRingVerts [j] * float (sinPhi);
			(*pv) [Z] = z;
			}
		}

	tTexCoord2f	tTexCoord2fl, tTexCoord2flStep = {{0.5f / RING_SEGS, 0.45f / THRUSTER_SEGS}};
	tTexCoord2f* flameTexCoord = m_flameTexCoord [m_bPlayer];
	float uOffset = m_bPlayer ? 0.5f : 0.0f;

	int nVerts = 0;
	for (i = 0; i < THRUSTER_SEGS - 1; i++) {
		for (j = 0; j <= RING_SEGS; j++) {
			m = j % RING_SEGS;
			tTexCoord2fl.v.u = uOffset + j * tTexCoord2flStep.v.u;
			for (n = 0; n < 2; n++) {
				tTexCoord2fl.v.v = 0.5f + tTexCoord2flStep.v.v * (i + n);
				flameTexCoord [nVerts] = tTexCoord2fl;
				m_flameVerts [nVerts++] = m_vFlame [i + n][m];
				}
			}
		}
	m_bHaveFlame = true;
	}
}

// -----------------------------------------------------------------------------

void CThrusterFlames::CalcPosOnShip (CObject *objP, CFixVector *vPos)
{
	tObjTransformation	*pPos = OBJPOS (objP);

if (gameOpts->render.bHiresModels [0]) {
	vPos [0] = pPos->vPos + pPos->mOrient.FVec () * (-objP->info.xSize);
	vPos [0] += pPos->mOrient.RVec () * (-(8 * objP->info.xSize / 44));
	vPos [1] = vPos [0] + pPos->mOrient.RVec () * (8 * objP->info.xSize / 22);
	}
else {
	vPos [0] = pPos->vPos + pPos->mOrient.FVec () * (-objP->info.xSize / 10 * 9);
	if (gameStates.app.bFixModels)
		vPos [0] += pPos->mOrient.UVec () * (objP->info.xSize / 40);
	else
		vPos [0] += pPos->mOrient.UVec () * (-objP->info.xSize / 20);
	vPos [1] = vPos [0];
	vPos [0] += pPos->mOrient.RVec () * (-8 * objP->info.xSize / 49);
	vPos [1] += pPos->mOrient.RVec () * (8 * objP->info.xSize / 49);
	}
}

// -----------------------------------------------------------------------------

int CThrusterFlames::CalcPos (CObject *objP, tThrusterInfo* tiP, int bAfterburnerBlob)
{
if (!tiP)
	tiP = &m_ti;

	tThrusterInfo	ti = *tiP;
	int				i, bMissile = IS_MISSILE (objP);

m_pt = NULL;
ti.pp = NULL;
ti.mtP = gameData.models.thrusters + objP->rType.polyObjInfo.nModel;
m_nThrusters = ti.mtP->nCount;
if (gameOpts->render.bHiresModels [0] && (objP->info.nType == OBJ_PLAYER) && !GetASEModel (objP->rType.polyObjInfo.nModel)) {
	if (!m_bSpectate) {
		m_pt = gameData.render.thrusters + objP->info.nId;
		ti.pp = m_pt->path.GetPoint ();
		}
	ti.fSize = (ti.fLength + 1) / 2;
	m_nThrusters = 2;
	CalcPosOnShip (objP, ti.vPos);
	ti.mtP = NULL;
	}
else if (bAfterburnerBlob || (bMissile && !m_nThrusters)) {
		tHitbox	*phb = &gameData.models.hitboxes [objP->rType.polyObjInfo.nModel].hitboxes [0];
		fix		nObjRad = (phb->vMax [Z] - phb->vMin [Z]) / 2;

	if (bAfterburnerBlob)
		nObjRad *= 2;
	if (objP->info.nId == EARTHSHAKER_ID)
		ti.fSize = 1.0f;
	else if ((objP->info.nId == MEGAMSL_ID) || (objP->info.nId == EARTHSHAKER_MEGA_ID))
		ti.fSize = 0.8f;
	else if (objP->info.nId == SMARTMSL_ID)
		ti.fSize = 0.6f;
	else
		ti.fSize = 0.5f;
	m_nThrusters = 1;
	if (EGI_FLAG (bThrusterFlames, 1, 1, 0) == 2)
		ti.fLength /= 2;
	if (!gameData.models.vScale.IsZero ())
		ti.vPos [0] *= gameData.models.vScale;
	*ti.vPos = objP->info.position.vPos + objP->info.position.mOrient.FVec () * (-nObjRad);
	ti.mtP = NULL;
	}
else if ((objP->info.nType == OBJ_PLAYER) ||
			((objP->info.nType == OBJ_ROBOT) && !objP->cType.aiInfo.CLOAKED) ||
			bMissile) {
	CFixMatrix	m, *viewP;
	if (!m_bSpectate && (objP->info.nType == OBJ_PLAYER)) {
		m_pt = gameData.render.thrusters + objP->info.nId;
		ti.pp = m_pt->path.GetPoint ();
		}
	if (!m_nThrusters) {
		if (objP->info.nType != OBJ_PLAYER)
			return 0;
		if (!m_bSpectate) {
			m_pt = gameData.render.thrusters + objP->info.nId;
			ti.pp = m_pt->path.GetPoint ();
			}
		ti.fSize = (ti.fLength + 1) / 2;
		m_nThrusters = 2;
		CalcPosOnShip (objP, ti.vPos);
		}
	else {
		tObjTransformation *posP = OBJPOS (objP);
		if (SPECTATOR (objP)) {
			viewP = &m;
			m = posP->mOrient.Transpose ();
			}
		else
			viewP = objP->View ();
		for (i = 0; i < m_nThrusters; i++) {
			ti.vPos [i] = *viewP * ti.mtP->vPos [i];
			if (!gameData.models.vScale.IsZero ())
				ti.vPos [i] *= gameData.models.vScale;
			ti.vPos [i] += posP->vPos;
			ti.vDir [i] = *viewP * ti.mtP->vDir [i];
			}
		ti.fSize = ti.mtP->fSize;
		if (bMissile)
			m_nThrusters = 1;
		}
	}
else
	return 0;
*tiP = ti;
return m_nThrusters;
}

// -----------------------------------------------------------------------------

void CThrusterFlames::Render2D (CFixVector& vPos, CFixVector &vDir, float fSize, float fLength, tRgbaColorf *colorP)
{
if (gameOpts->render.stereo.nGlasses && (ogl.StereoSeparation () >= 0))
	return;

	static tTexCoord2f tcTrail [2][4] = {
		{{{0.25f,0.49609375f}},{{0.25f,0.0f}},{{0.5f,0.0f}},{{0.5f,0.49609375f}}},
		{{{0.75f,0.49609375f}},{{0.75f,0.0f}},{{1.0f,0.0f}},{{1.0f,0.49609375f}}}
		};
	static CFloatVector	vEye;

	CFloatVector	v, vPosf, vNormf, vTrail [4], vCap [4], vTrailTip, vDirf;
	float		c = 1/*0.7f + 0.03f * fPulse*/, dotTrail, dotCap;

#if DBG
if (fSize > 5.0f)
	fSize = fSize;
#endif
vDirf.Assign (vDir);
vPosf.Assign (vPos);
vDirf *= fLength;
vTrailTip = vPosf - vDirf;
vEye.Assign (gameData.render.mine.viewer.vPos);
vNormf = CFloatVector::Normal (vTrailTip, vPosf, vEye) * fSize;
vCap [0] = vPosf + vNormf;
vCap [2] = vPosf - vNormf;
vNormf *= 0.7071f;
vTrail [0] = vPosf + vNormf;
vTrail [1] = vPosf - vNormf;
vTrail [2] = vTrail [1] - vDirf;
vTrail [3] = vTrail [0] - vDirf;
vNormf = CFloatVector::Normal (vTrail [0], vTrail [1], vTrailTip) * fSize;
vCap [1] = vPosf + vNormf;
vCap [3] = vPosf - vNormf;
vPosf -= vEye;
CFloatVector::Normalize (vPosf);
v = vTrailTip - vEye;
CFloatVector::Normalize (v);
dotTrail = CFloatVector::Dot (vPosf, v);
v = *vCap - vEye;
CFloatVector::Normalize (v);
dotCap = CFloatVector::Dot (vPosf, v);
transparencyRenderer.AddLightTrail (thruster.Bitmap (), vCap, tcCap [m_bPlayer], (dotTrail < dotCap) ? vTrail : NULL, tcTrail [m_bPlayer], colorP);
}

// -----------------------------------------------------------------------------

void CThrusterFlames::RenderCap (void)
{
if (thruster.Load ()) {
	CFloatVector	verts [4];
	float				z = (m_vFlame [0][0][Z] + m_vFlame [1][0][Z]) / 2.0f * m_ti.fLength;
	float				scale = m_ti.fSize * 1.6666667f;

	// choose 4 vertices from the widest ring of the flame
	for (int i = 0; i < 4; i++) {
		verts [i] = m_vFlame [5][4 * i];
		verts [i] [X] *= scale;
		verts [i] [Y] *= scale;
		verts [i] [Z] = z;
		}
	glColor3f (1,1,1);
	ogl.RenderQuad (thruster.Bitmap (), verts, 3, tcCap [m_bPlayer]);
	ogl.SetTexturing (true);
	thruster.Bitmap ()->Bind (1);
	}
}

// -----------------------------------------------------------------------------

void CThrusterFlames::Render3D (void)
{
glColor3f (1,1,1);
ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
OglTexCoordPointer (2, GL_FLOAT, 0, &m_flameTexCoord [m_bPlayer]);
OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), m_flameVerts);
glPushMatrix ();
glScalef (m_ti.fSize, m_ti.fSize, m_ti.fLength);
OglDrawArrays (GL_QUAD_STRIP, 0, FLAME_VERT_COUNT);
glPopMatrix ();
}

// -----------------------------------------------------------------------------

bool CThrusterFlames::Setup (CObject *objP)
{
if (gameStates.app.bNostalgia)
	return false;
if (!gameOpts->render.effects.bEnabled)
	return false;
if (SHOW_SHADOWS && (gameStates.render.nShadowPass != 1))
	return false;
if (!(m_nStyle = EGI_FLAG (bThrusterFlames, 1, 1, 0)))
	return false;
if ((objP->info.nType == OBJ_PLAYER) && (gameData.multiplayer.players [objP->info.nId].flags & PLAYER_FLAGS_CLOAKED))
	return false;

bool bFallback;

if (thruster.Load ()) 
	bFallback = false;
else {
	m_nStyle ^= 3;	// fall back to other style
	if (!thruster.Load ()) {
		extraGameInfo [IsMultiGame].bThrusterFlames = 0;	// didn't work either
		return false;
		}
	bFallback = true;
	}

if (m_nStyle == 2) {
	ogl.SetTexturing (true);
	thruster.Bitmap ()->SetTranspType (-1);
	if (thruster.Bitmap ()->Bind (1)) {
		extraGameInfo [IsMultiGame].bThrusterFlames = bFallback ? 0 : 1;
		return false;
		}
	thruster.Texture ()->Wrap (GL_CLAMP);
	}

float fSpeed = X2F (objP->mType.physInfo.velocity.Mag ());
if (m_pt)
	m_pt->fSpeed = fSpeed;
m_bSpectate = SPECTATOR (objP);

m_ti.pp = NULL;
if (!CalcPos (objP))
	return false;
m_ti.fLength = fSpeed / 60.0f + 0.5f;
if (m_ti.fLength < m_ti.fSize / 2)
	m_ti.fLength = m_ti.fSize / 2;
m_ti.fLength += float (rand () % 100) / 1000.0f;

m_bPlayer = (objP->info.nType == OBJ_PLAYER);

if (m_nThrusters > 1) {
	CFixVector vRot [2];
	transformation.Rotate (vRot [0], m_ti.vPos [0], 0);
	transformation.Rotate (vRot [1], m_ti.vPos [1], 0);
	if (vRot [0][Z] < vRot [1][Z]) {
		::Swap (m_ti.vPos [0], m_ti.vPos [1]);
		if (objP->info.nType == OBJ_ROBOT)
			::Swap (m_ti.vDir [0], m_ti.vDir [1]);
		}
	}
return true;
}

// -----------------------------------------------------------------------------

void CThrusterFlames::Render (CObject *objP)
{
if (!Setup (objP))
	return;

int bStencil = ogl.StencilOff ();

if (m_nStyle == 1) {	//2D
		static tRgbaColorf	tcColor = {0.75f, 0.75f, 0.75f, 1.0f};
		static CFloatVector	vEye = CFloatVector::ZERO;

	if (!gameData.models.vScale.IsZero ())
		m_ti.fSize *= X2F (gameData.models.vScale [Z]);
	m_ti.fLength *= 4 * m_ti.fSize;
	m_ti.fSize *= ((objP->info.nType == OBJ_PLAYER) && HaveHiresModel (objP->rType.polyObjInfo.nModel)) ? 1.2f : 1.5f;
	for (int i = 0; i < m_nThrusters; i++)
		Render2D (m_ti.vPos [i], m_ti.vDir [i], m_ti.fSize, m_ti.fLength, &tcColor);
	}
else { //3D
	Create ();
	ogl.ResetClientStates (1);
	ogl.SetFaceCulling (false);
	ogl.SetBlendMode (1);
	ogl.SetTransform (1);
	ogl.SetDepthWrite (false);

	m_ti.fLength /= 2;

	for (int i = 0; i < m_nThrusters; i++) {
		transformation.Begin (m_ti.vPos [i], (m_ti.pp && !m_bSpectate) ? m_ti.pp->mOrient : objP->info.position.mOrient);
		// render a cap for the thruster flame at its base
		RenderCap ();
		Render3D ();
		transformation.End ();
		}

	ogl.SetTransform (0);
	ogl.SetBlendMode (0);
	ogl.SetFaceCulling (true);
	OglCullFace (0);
	ogl.SetDepthWrite (true);
	}
ogl.StencilOn (bStencil);
}

//------------------------------------------------------------------------------
//eof
