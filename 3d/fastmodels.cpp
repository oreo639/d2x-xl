#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <math.h>
#include <stdlib.h>
#include "error.h"

#include "descent.h"
#include "interp.h"
#include "shadows.h"
#include "hitbox.h"
#include "globvars.h"
#include "gr.h"
#include "byteswap.h"
#include "u_mem.h"
#include "console.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_color.h"
#include "network.h"
#include "rendermine.h"
#include "segmath.h"
#include "light.h"
#include "dynlight.h"
#include "lightning.h"
#include "renderthreads.h"
#include "hiresmodels.h"
#include "buildmodel.h"
#include "weapon.h"
#include "headlight.h"

extern tFaceColor tMapColor;

#if DBG_SHADOWS
extern int bShadowTest;
extern int bFrontCap;
extern int bRearCap;
extern int bShadowVolume;
extern int bFrontFaces;
extern int bBackFaces;
extern int bSWCulling;
#endif
extern int bZPass;

#define G3_DRAW_ARRAYS				1
#define G3_DRAW_SUBMODELS			1
#define G3_FAST_MODELS				1
#define G3_DRAW_RANGE_ELEMENTS	1
#define G3_SW_SCALING				0
#define G3_HW_LIGHTING				1
#define G3_USE_VBOS					1 //G3_HW_LIGHTING
#define G3_ALLOW_TRANSPARENCY		1

//------------------------------------------------------------------------------

void G3DynLightModel (CObject *objP, RenderModel::CModel *pm, short iVerts, short nVerts, short iFaceVerts, short nFaceVerts)
{
	CFloatVector				vPos, vVertex;
	CFloatVector3*				pv, * pn;
	RenderModel::CVertex*	pmv;
	tFaceColor*					pc;
	float							fAlpha = gameStates.render.grAlpha;
	int							h, i, 
									bEmissive = (objP->info.nType == OBJ_WEAPON) && 
													gameData.objs.bIsWeapon [objP->info.nId] && 
													!gameData.objs.bIsMissile [objP->info.nId];

if (!gameStates.render.bBrightObject) {
	vPos.Assign (objP->info.position.vPos);
	for (i = iVerts, pv = pm->m_verts + iVerts, pn = pm->m_vertNorms + iVerts, pc = pm->m_color + iVerts;
		  i < nVerts;
		  i++, pv++, pn++, pc++) {
		pc->index = 0;
		vVertex = vPos + *reinterpret_cast<CFloatVector*> (pv);
		G3VertexColor (reinterpret_cast<CFloatVector3*> (pn), vVertex.XYZ (), i, pc, NULL, 1, 0, 0);
		}
	}
for (i = iFaceVerts, h = iFaceVerts, pmv = pm->m_faceVerts + iFaceVerts; i < nFaceVerts; i++, h++, pmv++) {
	if (gameStates.render.bBrightObject || bEmissive) {
		pm->m_vbColor [h] = pmv->m_baseColor;
		pm->m_vbColor [h].alpha = fAlpha;
		}
	else if (pmv->m_bTextured)
		pm->m_vbColor [h] = pm->m_color [pmv->m_nIndex].color;
	else {
		pc = pm->m_color + pmv->m_nIndex;
		pm->m_vbColor [h].red = pmv->m_baseColor.red * pc->color.red;
		pm->m_vbColor [h].green = pmv->m_baseColor.green * pc->color.green;
		pm->m_vbColor [h].blue = pmv->m_baseColor.blue * pc->color.blue;
		pm->m_vbColor [h].alpha = pmv->m_baseColor.alpha;
		}
	}
}

//------------------------------------------------------------------------------

void G3LightModel (CObject *objP, int nModel, fix xModelLight, fix *xGlowValues, int bHires)
{
	RenderModel::CModel*		pm = gameData.models.renderModels [bHires] + nModel;
	RenderModel::CVertex*	pmv;
	RenderModel::CFace*		pmf;
	tRgbaColorf					baseColor, *colorP;
	float							fLight, fAlpha = gameStates.render.grAlpha;
	int							h, i, j, l;
	int							bEmissive = (objP->info.nType == OBJ_MARKER) ||
													((objP->info.nType == OBJ_WEAPON) && 
													 gameData.objs.bIsWeapon [objP->info.nId] && 
													 !gameData.objs.bIsMissile [objP->info.nId]);

#if DBG
if (objP->Index () == nDbgObj)
	objP = objP;
#endif
#if 0
if (xModelLight > I2X (1))
	xModelLight = I2X (1);
#endif
if (SHOW_DYN_LIGHT && (gameOpts->ogl.bObjLighting ||
    (gameOpts->ogl.bLightObjects && (gameOpts->ogl.bLightPowerups || (objP->info.nType == OBJ_PLAYER) || (objP->info.nType == OBJ_ROBOT))))) {
	tiRender.objP = objP;
	tiRender.pm = pm;
	if (!RunRenderThreads (rtPolyModel))
		G3DynLightModel (objP, pm, 0, pm->m_nVerts, 0, pm->m_nFaceVerts);
	}
else {
	if (gameData.objs.color.index)
		baseColor = gameData.objs.color.color;
	else
		baseColor.red = baseColor.green = baseColor.blue = 1;
	baseColor.alpha = fAlpha;
	for (i = pm->m_nFaces, pmf = pm->m_faces.Buffer (); i; i--, pmf++) {
		if (pmf->m_nBitmap >= 0)
			colorP = &baseColor;
		else
			colorP = NULL;
		if (pmf->m_bGlow)
			l = xGlowValues [nGlow];
		else if (bEmissive)
			l = I2X (1);
		else {
			l = -CFixVector::Dot (transformation.m_info.view [0].FVec (), pmf->m_vNormal);
			l = 3 * I2X (1) / 4 + l / 4;
			l = FixMul (l, xModelLight);
			}
		fLight = X2F (l);
		for (j = pmf->m_nVerts, h = pmf->m_nIndex, pmv = pm->m_faceVerts + pmf->m_nIndex; j; j--, h++, pmv++) {
#if G3_DRAW_ARRAYS
			if (colorP) {
				pm->m_vbColor [h].red = colorP->red * fLight;
				pm->m_vbColor [h].green = colorP->green * fLight;
				pm->m_vbColor [h].blue = colorP->blue * fLight;
				pm->m_vbColor [h].alpha = fAlpha;
				}
			else {
				pm->m_vbColor [h].red = pmv->m_baseColor.red * fLight;
				pm->m_vbColor [h].green = pmv->m_baseColor.green * fLight;
				pm->m_vbColor [h].blue = pmv->m_baseColor.blue * fLight;
				pm->m_vbColor [h].alpha = fAlpha;
				}
#else
			if (colorP) {
				pmv->m_renderColor.red = colorP->red * fLight;
				pmv->m_renderColor.green = colorP->green * fLight;
				pmv->m_renderColor.blue = colorP->blue * fLight;
				pmv->m_renderColor.alpha = fAlpha;
				}
			else {
				pmv->m_renderColor.red = pmv->m_baseColor.red * fLight;
				pmv->m_renderColor.green = pmv->m_baseColor.green * fLight;
				pmv->m_renderColor.blue = pmv->m_baseColor.blue * fLight;
				pmv->m_renderColor.alpha = fAlpha;
				}
#endif
			}
		}
	}
}

//------------------------------------------------------------------------------

#if G3_SW_SCALING

void G3ScaleModel (int nModel, int bHires)
{
	RenderModel::CModel			*pm = gameData.models.renderModels [bHires] + nModel;
	CFloatVector			fScale;
	int				i;
	CFloatVector3			*pv;
	RenderModel::CVertex	*pmv;

if (gameData.models.vScale.IsZero ())
	fScale.Create (1,1,1);
else
	fScale.Assign (gameData.models.vScale);
if (pm->m_fScale == fScale)
	return;
fScale /= pm->m_fScale;
for (i = pm->m_nVerts, pv = pm->m_verts; i; i--, pv++) {
	pv->p.x *= fScale;
	pv->p.y *= fScale;
	pv->p.z *= fScale;
	}
for (i = pm->m_nFaceVerts, pmv = pm->m_faceVerts; i; i--, pmv++)
	pmv->m_vertex = pm->m_verts [pmv->m_nIndex];
pm->m_fScale *= fScale;
}

#endif

//------------------------------------------------------------------------------

void G3GetThrusterPos (CObject *objP, short nModel, RenderModel::CFace *pmf, CFixVector *vOffsetP,
							  CFixVector *vNormal, int nRad, int bHires, ubyte nType = 255)
{
	RenderModel::CModel*		pm = gameData.models.renderModels [bHires] + nModel;
	RenderModel::CVertex*	pmv = NULL;
	CFloatVector3				v = CFloatVector3::ZERO, vn, vo, vForward = CFloatVector3::Create(0,0,1);
	CModelThrusters*			mtP = gameData.models.thrusters + nModel;
	int							i, j = 0, nCount = -mtP->nCount;
	float							h, nSize;

if (!objP)
	return;
if (nCount < 0) {
	if (pm->m_bRendered && gameData.models.vScale.IsZero ())
		return;
	nCount = 0;
	}	
else if (nCount >= MAX_THRUSTERS)
	return;
vn.Assign (pmf ? pmf->m_vNormal : *vNormal);
if ((nModel != 108) && (CFloatVector3::Dot (vn, vForward) > -1.0f / 3.0f))
	return;
if (pmf) {
	for (i = 0, j = pmf->m_nVerts, pmv = pm->m_faceVerts + pmf->m_nIndex; i < j; i++)
		v += pmv [i].m_vertex;
	v[X] /= j;
	v[Y] /= j;
	v[Z] /= j;
	}
else
	v.SetZero ();
v[Z] -= 1.0f / 16.0f;
#if 0
transformation.Transform (&v, &v, 0);
#else
#if 1
if (vOffsetP) {
	vo.Assign (*vOffsetP);
	v += vo;
	}
#endif
#endif
if (nCount && (v[X] == mtP->vPos [0][X]) && (v[Y] == mtP->vPos [0][Y]) && (v[Z] == mtP->vPos [0][Z]))
	return;
mtP->vPos [nCount].Assign (v);
if (vOffsetP)
	v -= vo;
mtP->vDir [nCount] = *vNormal;
mtP->vDir [nCount] = -mtP->vDir [nCount];
mtP->nType [nCount] = nType;
//if (!nCount) 
	{
	if (!pmf)
		mtP->fSize [nCount] = X2F (nRad);
	else {
		for (i = 0, nSize = 1000000000; i < j; i++)
			if (nSize > (h = CFloatVector3::Dist (v, pmv [i].m_vertex)))
				nSize = h;
		mtP->fSize [nCount] = nSize;// * 1.25f;
		}
	}
mtP->nCount = -(++nCount);
}

//------------------------------------------------------------------------------

static int bCenterGuns [] = {0, 1, 1, 0, 0, 0, 1, 1, 0, 1};

int G3FilterSubModel (CObject *objP, RenderModel::CSubModel *psm, int nGunId, int nBombId, int nMissileId, int nMissiles)
{
	int nId = objP->info.nId;

if (!psm->m_bRender)
	return 0;
if (psm->m_nGunPoint >= 0)
	return 1;
if (psm->m_bBullets)
	return 1;
#if 1
if (psm->m_bThruster && ((psm->m_bThruster & (REAR_THRUSTER | FRONTAL_THRUSTER)) != (REAR_THRUSTER | FRONTAL_THRUSTER)))
	return 1;
#endif
#if 0
if (psm->m_bFlare)
	return 1;
#endif
if (psm->m_bHeadlight)
	return !HeadlightIsOn (nId);
if (psm->m_bBombMount)
	return (nBombId == 0);
if (psm->m_bWeapon) {
	CPlayerData	*playerP = gameData.multiplayer.players + nId;
	int		bLasers = (nGunId == LASER_INDEX) || (nGunId == SUPER_LASER_INDEX);
	int		bSuperLasers = playerP->laserLevel > MAX_LASER_LEVEL;
	int		bQuadLasers = gameData.multiplayer.weaponStates [gameData.multiplayer.nLocalPlayer].bQuadLasers;
	int		bCenterGun = bCenterGuns [nGunId];
	int		nWingtip = bQuadLasers ? bSuperLasers : 2; //gameOpts->render.ship.nWingtip;

	gameOpts->render.ship.nWingtip = nWingtip;
	if (nWingtip == 0)
		nWingtip = bLasers && bSuperLasers && bQuadLasers;
	else if (nWingtip == 1)
		nWingtip = !bLasers || bSuperLasers;
	
	if (EGI_FLAG (bShowWeapons, 0, 1, 0)) {
		if (psm->m_nGun == nGunId + 1) {
			if (psm->m_nGun == FUSION_INDEX + 1) {
				if ((psm->m_nWeaponPos == 3) && !gameData.multiplayer.weaponStates [gameData.multiplayer.nLocalPlayer].bTripleFusion)
					return 1;
				}
			else if (bLasers) {
				if ((psm->m_nWeaponPos > 2) && !bQuadLasers && (nWingtip != bSuperLasers))
					return 1;
				}
			}
		else if (psm->m_nGun == LASER_INDEX + 1) {
			if (nWingtip)
				return 1;
			return !bCenterGun && (psm->m_nWeaponPos < 3);
			}
		else if (psm->m_nGun == SUPER_LASER_INDEX + 1) {
			if (nWingtip != 1)
				return 1;
			return !bCenterGun && (psm->m_nWeaponPos < 3);
			}
		else if (!psm->m_nGun) {
			if (bLasers && bQuadLasers)
				return 1;
			if (psm->m_nType != gameOpts->render.ship.nWingtip)
				return 1;
			return 0;
			}
		else if (psm->m_nBomb == nBombId)
			return (nId == gameData.multiplayer.nLocalPlayer) && !AllowedToFireMissile (nId, 0);
		else if (psm->m_nMissile == nMissileId) {
			if (psm->m_nWeaponPos > nMissiles)
				return 1;
			else {
				static int nMslPos [] = {-1, 1, 0, 3, 2};
				int nLaunchPos = gameData.multiplayer.weaponStates [nId].nMslLaunchPos;
				return (nId == gameData.multiplayer.nLocalPlayer) && !AllowedToFireMissile (nId, 0) &&
						 (nLaunchPos == (nMslPos [(int) psm->m_nWeaponPos]));
				}
			}
		else
			return 1;
		}
	else {
		if (psm->m_nGun == 1)
			return 0;
		if ((psm->m_nGun < 0) && (psm->m_nMissile == 1))
			return 0;
		return 1;
		}
	}
return 0;
}

//------------------------------------------------------------------------------

static inline int ObjectHasThruster (CObject *objP)
{
return (objP->info.nType == OBJ_PLAYER) ||
		 (objP->info.nType == OBJ_ROBOT) ||
		 ((objP->info.nType == OBJ_WEAPON) && (gameData.objs.bIsMissile [objP->info.nId]));
}

//------------------------------------------------------------------------------

int G3AnimateSubModel (CObject *objP, RenderModel::CSubModel *psm, short nModel)
{
	tFiringData*	fP;
	float				nTimeout, y;
	int				nDelay;

if (!psm->m_nFrames)
	return 0;
if ((psm->m_bThruster & (REAR_THRUSTER | FRONTAL_THRUSTER)) == (REAR_THRUSTER | FRONTAL_THRUSTER)) {
	nTimeout = gameStates.gameplay.slowmo [0].fSpeed;
	glPushMatrix ();
	CFloatVector vCenter;
	vCenter.Assign (psm->m_vCenter);
	glTranslatef (vCenter [X], vCenter [Y], vCenter [Z]);
	glRotatef (-360.0f * 5.0f * float (psm->m_iFrame) / float (psm->m_nFrames), 0, 0, 1);
	glTranslatef (-vCenter [X], -vCenter [Y], -vCenter [Z]);
	if (gameStates.app.nSDLTicks [0] - psm->m_tFrame > nTimeout) {
		psm->m_tFrame = gameStates.app.nSDLTicks [0];
		psm->m_iFrame = ++psm->m_iFrame % psm->m_nFrames;
		}
	}
else {
	nTimeout = 25 * gameStates.gameplay.slowmo [0].fSpeed;
	fP = gameData.multiplayer.weaponStates [objP->info.nId].firing;
	if (gameData.weapons.nPrimary == VULCAN_INDEX)
		nTimeout /= 2;
	if (fP->nStop > 0) {
		nDelay = gameStates.app.nSDLTicks [0] - fP->nStop;
		if (nDelay > GATLING_DELAY)
			return 0;
		nTimeout += (float) nDelay / 10;
		}
	else if (fP->nStart > 0) {
		nDelay = GATLING_DELAY - fP->nDuration;
		if (nDelay > 0)
			nTimeout += (float) nDelay / 10;
		}
	else
		return 0;
	glPushMatrix ();
	y = X2F (psm->m_vCenter [Y]);
	glTranslatef (0, y, 0);
	glRotatef (360 * float (psm->m_iFrame) / float (psm->m_nFrames), 0, 0, 1);
	glTranslatef (0, -y, 0);
	if (gameStates.app.nSDLTicks [0] - psm->m_tFrame > nTimeout) {
		psm->m_tFrame = gameStates.app.nSDLTicks [0];
		psm->m_iFrame = ++psm->m_iFrame % psm->m_nFrames;
		}
	}
return 1;
}

//------------------------------------------------------------------------------

static inline int PlayerColor (int nObject)
{
	int	nColor;

for (nColor = 0; nColor < gameData.multiplayer.nPlayers; nColor++)
	if (gameData.multiplayer.players [nColor].nObject == nObject)
		return (nColor % MAX_PLAYER_COLORS) + 1;
return 1;
}

//------------------------------------------------------------------------------

void G3DrawSubModel (CObject *objP, short nModel, short nSubModel, short nExclusive, CArray<CBitmap*>& modelBitmaps,
						   CAngleVector *animAnglesP, CFixVector *vOffsetP, int bHires, int bUseVBO, int nPass, int bTranspFilter,
							int nGunId, int nBombId, int nMissileId, int nMissiles)
{
	RenderModel::CModel*		pm = gameData.models.renderModels [bHires] + nModel;
	RenderModel::CSubModel*	psm = pm->m_subModels + nSubModel;
	RenderModel::CFace*		pmf;
	CBitmap*						bmP = NULL;
	CAngleVector				va = animAnglesP ? animAnglesP [psm->m_nAngles] : CAngleVector::ZERO;
	CFixVector					vo;
	int							h, i, j, bTranspState, bAnimate, bTextured = gameOpts->render.debug.bTextures && !(gameStates.render.bCloaked /*|| nPass*/),
									bGetThruster = !(nPass || bTranspFilter) && ObjectHasThruster (objP);
	short							nId, nFaceVerts, nVerts, nIndex, nBitmap = -1, nTeamColor;

if (objP->info.nType == OBJ_PLAYER)
	nTeamColor = IsMultiGame ? IsTeamGame ? GetTeam (objP->info.nId) + 1 : PlayerColor (objP->Index ()) : gameOpts->render.ship.nColor;
else
	nTeamColor = 0;
#if 1
if (psm->m_bThruster /*== 1*/) {
	if (!nPass) {
		vo = psm->m_vOffset + psm->m_vCenter;
		G3GetThrusterPos (objP, nModel, NULL, &vo, &psm->m_faces->m_vNormal, psm->m_nRad, bHires, psm->m_bThruster);
		}
	if (!psm->m_nFrames)
		return;
	}
if (G3FilterSubModel (objP, psm, nGunId, nBombId, nMissileId, nMissiles))
	return;
#endif
vo = psm->m_vOffset;
if (!gameData.models.vScale.IsZero ())
	vo *= gameData.models.vScale;
#if 1
if (vOffsetP && (nExclusive < 0)) {
	transformation.Begin (vo, va);
	vo += *vOffsetP;
	}
#endif
bAnimate = G3AnimateSubModel (objP, psm, nModel);
#if G3_DRAW_SUBMODELS
// render any dependent submodels
for (i = 0, j = pm->m_nSubModels, psm = pm->m_subModels.Buffer (); i < j; i++, psm++)
	if (psm->m_nParent == nSubModel)
		G3DrawSubModel (objP, nModel, i, nExclusive, modelBitmaps, animAnglesP, &vo, bHires,
							 bUseVBO, nPass, bTranspFilter, nGunId, nBombId, nMissileId, nMissiles);
#endif
// render the faces
#if 0
psm = pm->m_subModels + nSubModel;
if (psm->m_bBillboard)
#endif
if ((nExclusive < 0) || (nSubModel == nExclusive)) {
#if 0
	if (vOffsetP && (nSubModel == nExclusive))
		transformation.Begin (vOffsetP, NULL);
#endif
	psm = pm->m_subModels + nSubModel;
	if (!bAnimate && psm->m_bBillboard) {
		glMatrixMode (GL_MODELVIEW);
		glPushMatrix ();
		float modelView [16];
		glGetFloatv (GL_MODELVIEW_MATRIX, modelView);
		// undo all rotations
		// beware all scaling is lost as well 
#if 1
		for (int i = 0; i < 3; i++) 
			 for (int j = 0; j < 3; j++)
				modelView [i * 4 + j] = (i == j) ? 1.0f : 0.0f;
#endif
		glLoadMatrixf (modelView);
		}
	ogl.SetTexturing (false);
	if (gameStates.render.bCloaked)
		glColor4f (0, 0, 0, gameStates.render.grAlpha);
	for (i = psm->m_nFaces, pmf = psm->m_faces; i; ) {
		if (bTextured && (nBitmap != pmf->m_nBitmap)) {
			if (0 > (nBitmap = pmf->m_nBitmap))
				ogl.SetTexturing (false);
			else {
				if (!bHires)
					bmP = modelBitmaps [nBitmap];
				else {
					bmP = pm->m_textures + nBitmap;
#if 0
					if (!nPass)
						ogl.SetBlendMode (psm->m_bFlare);
#endif
					if (nTeamColor && bmP->Team () && (0 <= (h = pm->m_teamTextures [nTeamColor % MAX_PLAYER_COLORS]))) {
						nBitmap = h;
						bmP = pm->m_textures + nBitmap;
						}
					}
				ogl.SelectTMU (GL_TEXTURE0, true);
				ogl.SetTexturing (true);
				bmP = bmP->Override (-1);
				if (bmP->Frames ())
					bmP = bmP->CurFrame ();
				if (bmP->Bind (1))
					continue;
				bmP->Texture ()->Wrap (GL_REPEAT);
				}
			}
		nIndex = pmf->m_nIndex;
		if (bHires) {
			bTranspState = !bmP ? 0 : psm->m_bThruster ? 0 : (psm->m_bGlow || psm->m_bFlare) ? 2 : ((bmP->Flags () & BM_FLAG_TRANSPARENT) != 0) ? 1 : 0;
			if (bTranspState != bTranspFilter) {
				if (bTranspState)
					pm->m_bHasTransparency |= bTranspState;
				pmf++;
				i--;
				continue;
				}
			}
		if ((nFaceVerts = pmf->m_nVerts) > 4) {
			if (bGetThruster && pmf->m_bThruster)
				G3GetThrusterPos (objP, nModel, pmf, &vo, &pmf->m_vNormal, 0, bHires);
			nVerts = nFaceVerts;
			pmf++;
			i--;
			}
		else {
			nId = pmf->m_nId;
			nVerts = 0;
			do {
				if (bGetThruster && pmf->m_bThruster)
					G3GetThrusterPos (objP, nModel, pmf, &vo, &pmf->m_vNormal, 0, bHires);
				nVerts += nFaceVerts;
				pmf++;
				i--;
				} while (i && (pmf->m_nId == nId));
			}
#if 1 //!DBG
#ifdef _WIN32
		if (glDrawRangeElements)
#endif
#if DBG
			if (bUseVBO)
#if 1
				glDrawRangeElements (gameOpts->render.debug.bWireFrame ? GL_LINE_LOOP : (nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN,
											0, pm->m_nFaceVerts - 1, nVerts, GL_UNSIGNED_SHORT,
											G3_BUFFER_OFFSET (nIndex * sizeof (short)));
#else
				glDrawElements (gameOpts->render.debug.bWireFrame ? GL_LINE_LOOP : (nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN,
									 nVerts, GL_UNSIGNED_SHORT,
									 G3_BUFFER_OFFSET (nIndex * sizeof (short)));
#endif
			else
				glDrawRangeElements (gameOpts->render.debug.bWireFrame ? GL_LINE_LOOP : (nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN,
											nIndex, nIndex + nVerts - 1, nVerts, GL_UNSIGNED_SHORT,
											pm->m_index [0] + nIndex);
#else
			if (bUseVBO)
				glDrawRangeElements ((nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN,
											0, pm->m_nFaceVerts - 1, nVerts, GL_UNSIGNED_SHORT,
											G3_BUFFER_OFFSET (nIndex * sizeof (short)));
			else
				glDrawRangeElements ((nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN,
											nIndex, nIndex + nVerts - 1, nVerts, GL_UNSIGNED_SHORT,
											pm->m_index [0] + nIndex);
#endif
#ifdef _WIN32
		else
			if (bUseVBO)
				glDrawElements ((nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN,
									 nVerts, GL_UNSIGNED_SHORT, G3_BUFFER_OFFSET (nIndex * sizeof (short)));
			else
				glDrawElements ((nFaceVerts == 3) ? GL_TRIANGLES : (nFaceVerts == 4) ? GL_QUADS : GL_TRIANGLE_FAN,
									 nVerts, GL_UNSIGNED_SHORT, pm->m_index + nIndex);
#endif
#endif
		}
	}
if (bAnimate || psm->m_bBillboard)
	glPopMatrix ();
#if 1
if ((nExclusive < 0) /*|| (nSubModel == nExclusive)*/)
	transformation.End ();
#endif
}

//------------------------------------------------------------------------------

void G3DrawModel (CObject *objP, short nModel, short nSubModel, CArray<CBitmap*>& modelBitmaps,
						CAngleVector *animAnglesP, CFixVector *vOffsetP, int bHires, int bUseVBO, int bTranspFilter,
						int nGunId, int nBombId, int nMissileId, int nMissiles)
{
	RenderModel::CModel*	pm;
	CDynLight*				prl;
	int						nPass, iLight, nLights, nLightRange;
	int						bBright = objP && (objP->info.nType == OBJ_MARKER);
	int						bEmissive = objP && (objP->info.nType == OBJ_WEAPON) && gameData.objs.bIsWeapon [objP->info.nId] && !gameData.objs.bIsMissile [objP->info.nId];
	int						bLighting = SHOW_DYN_LIGHT && gameOpts->ogl.bObjLighting && (bTranspFilter < 2) && !(gameStates.render.bCloaked || bEmissive || bBright);
	GLenum					hLight;
	float						fBrightness, fLightScale = gameData.models.nLightScale ? X2F (gameData.models.nLightScale) : 1.0f;
	CFloatVector			color;
	CDynLightIndex*		sliP = bLighting ? &lightManager.Index (0,0) : NULL;
	CActiveDynLight*		activeLightsP = sliP ? lightManager.Active (0) + sliP->nFirst : NULL;
	tObjTransformation*	posP = OBJPOS (objP);

ogl.SetupTransform (1);
if (bLighting) {
	nLights = sliP->nActive;
	if (nLights > gameStates.render.nMaxLightsPerObject)
		nLights = gameStates.render.nMaxLightsPerObject;
	ogl.EnableLighting (0);
	}
else
	nLights = 1;
ogl.SetBlending (true);
if (bEmissive || (bTranspFilter == 2))
	ogl.SetBlendMode (1);
else if (gameStates.render.bCloaked)
	ogl.SetBlendMode (0);
else if (bTranspFilter) {
	ogl.SetBlendMode (0);
	ogl.SetDepthWrite (false);
	}
else {
	ogl.SetBlendMode (GL_ONE, GL_ZERO);
	ogl.SetDepthWrite (true);
	}

if (!bLighting || (sliP->nLast < 0))
	nLightRange = 0;
else
	nLightRange = sliP->nLast - sliP->nFirst + 1;
for (nPass = 0; ((nLightRange > 0) && (nLights > 0)) || !nPass; nPass++) {
	if (bLighting) {
		if (nPass) {
			ogl.SetBlendMode (2);
			ogl.SetDepthWrite (false);
			}
		for (iLight = 0; (nLightRange > 0) && (iLight < 8) && nLights; activeLightsP++, nLightRange--) {
#if DBG
			if (activeLightsP - lightManager.Active (0) >= MAX_OGL_LIGHTS)
				break;
			if (activeLightsP < lightManager.Active (0))
				break;
#endif
			if (nLights < 0) {
				tFaceColor *psc = lightManager.AvgSgmColor (objP->info.nSegment, NULL, 0);
				CFloatVector3 vPos;
				hLight = GL_LIGHT0 + iLight++;
				glEnable (hLight);
				vPos.Assign (objP->info.position.vPos);
				glLightfv (hLight, GL_POSITION, reinterpret_cast<GLfloat*> (&vPos));
				glLightfv (hLight, GL_DIFFUSE, reinterpret_cast<GLfloat*> (&psc->color));
				glLightfv (hLight, GL_SPECULAR, reinterpret_cast<GLfloat*> (&psc->color));
				glLightf (hLight, GL_CONSTANT_ATTENUATION, 0.1f);
				glLightf (hLight, GL_LINEAR_ATTENUATION, 0.1f);
				glLightf (hLight, GL_QUADRATIC_ATTENUATION, 0.01f);
				nLights = 0;
				}
			else if ((prl = lightManager.GetActive (activeLightsP, 0))) {
#if DBG
				if ((nDbgSeg >= 0) && (prl->info.nObject >= 0) && (OBJECTS [prl->info.nObject].info.nSegment == nDbgSeg))
					nDbgSeg = nDbgSeg;
#endif
				hLight = GL_LIGHT0 + iLight++;
				glEnable (hLight);
	//			sprintf (szLightSources + strlen (szLightSources), "%d ", (prl->nObject >= 0) ? -prl->nObject : prl->nSegment);
				fBrightness = prl->info.fBrightness * fLightScale;
				color = *(reinterpret_cast<CFloatVector*> (&prl->info.color));
				color[R] *= fLightScale;
				color[G] *= fLightScale;
				color[B] *= fLightScale;
				glLightfv (hLight, GL_POSITION, reinterpret_cast<GLfloat*> (prl->render.vPosf));
				glLightfv (hLight, GL_DIFFUSE, reinterpret_cast<GLfloat*> (&color));
				glLightfv (hLight, GL_SPECULAR, reinterpret_cast<GLfloat*> (&color));
				if (prl->info.bSpot) {
#if 0
					prl = prl;
#else
					glLighti (hLight, GL_SPOT_EXPONENT, 12);
					glLighti (hLight, GL_SPOT_CUTOFF, 25);
					glLightfv (hLight, GL_SPOT_DIRECTION, reinterpret_cast<GLfloat*> (&prl->info.vDirf));
#endif
					glLightf (hLight, GL_CONSTANT_ATTENUATION, 0.0f); //0.1f / fBrightness);
					glLightf (hLight, GL_LINEAR_ATTENUATION, 0.05f / fBrightness);
					glLightf (hLight, GL_QUADRATIC_ATTENUATION, 0.005f / fBrightness);
					}
				else {
					glLightf (hLight, GL_CONSTANT_ATTENUATION, 0.0f); //0.1f / fBrightness);
#if 1
					if (X2F (CFixVector::Dist (objP->info.position.vPos, prl->info.vPos)) <= prl->info.fRad) {
						glLightf (hLight, GL_LINEAR_ATTENUATION, 0.01f / fBrightness);
						glLightf (hLight, GL_QUADRATIC_ATTENUATION, 0.001f / fBrightness);
						}
					else 
#endif
						{
						glLightf (hLight, GL_LINEAR_ATTENUATION, 0.05f / fBrightness);
						glLightf (hLight, GL_QUADRATIC_ATTENUATION, 0.005f / fBrightness);
						}
					}
				nLights--;
				}
			}
		for (; iLight < 8; iLight++)
			glDisable (GL_LIGHT0 + iLight);
		}
	transformation.Begin (posP->vPos, posP->mOrient);
	pm = gameData.models.renderModels [bHires] + nModel;
	if (bHires) {
		for (int i = 0; i < pm->m_nSubModels; i++)
			if (pm->m_subModels [i].m_nParent == -1)
				G3DrawSubModel (objP, nModel, i, nSubModel, modelBitmaps, animAnglesP, (nSubModel < 0) ? &pm->m_subModels [0].m_vOffset : vOffsetP,
									 bHires, bUseVBO, nPass, bTranspFilter, nGunId, nBombId, nMissileId, nMissiles);
		}
	else
		G3DrawSubModel (objP, nModel, 0, nSubModel, modelBitmaps, animAnglesP, (nSubModel < 0) ? &pm->m_subModels [0].m_vOffset : vOffsetP,
							 bHires, bUseVBO, nPass, bTranspFilter, nGunId, nBombId, nMissileId, nMissiles);
	//if (nSubModel < 0)
		transformation.End ();
	if (!bLighting)
		break;
	}
#if DBG
if (!nLightRange && nLights)
	nLights = 0;
#endif
if (bLighting) {
	ogl.DisableLighting ();
	ogl.SetBlendMode (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	ogl.SetDepthWrite (true);
	}
ogl.ResetTransform (1);
//HUDMessage (0, "%s", szLightSources);
}

//------------------------------------------------------------------------------

void G3RenderDamageLightning (CObject *objP, short nModel, short nSubModel,
										 CAngleVector *animAnglesP, CFixVector *vOffsetP, int bHires)
{
if (!(SHOW_LIGHTNING && gameOpts->render.lightning.bDamage))
	return;

	RenderModel::CModel*		pm;
	RenderModel::CSubModel*	psm;
	RenderModel::CFace*		pmf;
	const CAngleVector*		va;
	CFixVector					vo;
	int							i, j;

pm = gameData.models.renderModels [bHires] + nModel;
if (pm->m_bValid < 1) {
	if (!bHires)
		return;
	pm = gameData.models.renderModels [0] + nModel;
	if (pm->m_bValid < 1)
		return;
	}
psm = pm->m_subModels + nSubModel;
va = animAnglesP ? animAnglesP + psm->m_nAngles : &CAngleVector::ZERO;
if (!objP || (objP->Damage () > 0.5f))
	return;
// set the translation
vo = psm->m_vOffset;
if (!gameData.models.vScale.IsZero ())
	vo *= gameData.models.vScale;
if (vOffsetP) {
	transformation.Begin (vo, *va);
	vo += *vOffsetP;
	}
// render any dependent submodels
for (i = 0, j = pm->m_nSubModels, psm = pm->m_subModels.Buffer (); i < j; i++, psm++)
	if (psm->m_nParent == nSubModel)
		G3RenderDamageLightning (objP, nModel, i, animAnglesP, &vo, bHires);
// render the lightnings
for (psm = pm->m_subModels + nSubModel, i = psm->m_nFaces, pmf = psm->m_faces; i; i--, pmf++)
	lightningManager.RenderForDamage (objP, NULL, pm->m_faceVerts + pmf->m_nIndex, pmf->m_nVerts);
if (vOffsetP)
	transformation.End ();
}

//------------------------------------------------------------------------------

int G3RenderModel (CObject *objP, short nModel, short nSubModel, CPolyModel* pp, CArray<CBitmap*>& modelBitmaps,
						 CAngleVector *animAnglesP, CFixVector *vOffsetP, fix xModelLight, fix *xGlowValues, tRgbaColorf *pObjColor)
{
	RenderModel::CModel*	pm = gameData.models.renderModels [1] + nModel;
	int						i, 
								bHires = 1, 
								bUseVBO = ogl.m_states.bHaveVBOs && ((gameStates.render.bPerPixelLighting == 2) || gameOpts->ogl.bObjLighting),
								nGunId, nBombId, nMissileId, nMissiles;

if (!objP)
	return 0;
#if DBG
if (nModel == nDbgModel)
	nDbgModel = nModel;
if (objP && (ObjIdx (objP) == nDbgObj))
	nDbgObj = nDbgObj;
if (objP->info.nSegment == nDbgSeg)
	nDbgSeg = nDbgSeg;
#endif

if (pm->m_bValid < 1) {
	if (pm->m_bValid) {
		i = 0;
		bHires = 0;
		}
	else {
		if (IsDefaultModel (nModel)) {
			if (bUseVBO && pm->m_bValid && !(pm->m_vboDataHandle && pm->m_vboIndexHandle))
				pm->m_bValid = 0;
			i = G3BuildModel (objP, nModel, pp, modelBitmaps, pObjColor, 1);
			if (i < 0)	//successfully built new model
				return gameStates.render.bBuildModels;
			}
		else
			i = 0;
		pm->m_bValid = -1;
		}
	pm = gameData.models.renderModels [0] + nModel;
	if (pm->m_bValid < 0)
		return 0;
	if (bUseVBO && pm->m_bValid && !(pm->m_vboDataHandle && pm->m_vboIndexHandle))
		pm->m_bValid = 0;
	if (!(i || pm->m_bValid)) {
		i = G3BuildModel (objP, nModel, pp, modelBitmaps, pObjColor, 0);
		if (i <= 0) {
			if (!i)
				pm->m_bValid = -1;
			return gameStates.render.bBuildModels;
			}
		}
	}

//#pragma omp critical (fastModelRender)
{
PROF_START
if (gameStates.render.bCloaked)
	 ogl.EnableClientStates (0, 0, 0, GL_TEXTURE0);
else
	ogl.EnableClientStates (1, 1, gameOpts->ogl.bObjLighting, GL_TEXTURE0);
if (bUseVBO)
	glBindBufferARB (GL_ARRAY_BUFFER_ARB, pm->m_vboDataHandle);
else {
	pm->m_vbVerts.SetBuffer (reinterpret_cast<CFloatVector3*> (pm->m_vertBuf [0].Buffer ()), 1, pm->m_nFaceVerts);
	pm->m_vbNormals.SetBuffer (pm->m_vbVerts.Buffer () + pm->m_nFaceVerts, 1, pm->m_nFaceVerts);
	pm->m_vbColor.SetBuffer (reinterpret_cast<tRgbaColorf*> (pm->m_vbNormals.Buffer () + pm->m_nFaceVerts), 1, pm->m_nFaceVerts);
	pm->m_vbTexCoord.SetBuffer (reinterpret_cast<tTexCoord2f*> (pm->m_vbColor.Buffer () + pm->m_nFaceVerts), 1, pm->m_nFaceVerts);
	}
#if G3_SW_SCALING
G3ScaleModel (nModel);
#else
#	if 0
if (bHires)
	gameData.models.vScale.SetZero ();
#	endif
#endif
if (!(gameOpts->ogl.bObjLighting || gameStates.render.bCloaked))
	G3LightModel (objP, nModel, xModelLight, xGlowValues, bHires);
if (bUseVBO) {
	if (!gameStates.render.bCloaked) {
		OglNormalPointer (GL_FLOAT, 0, G3_BUFFER_OFFSET (pm->m_nFaceVerts * sizeof (CFloatVector3)));
		OglColorPointer (4, GL_FLOAT, 0, G3_BUFFER_OFFSET (pm->m_nFaceVerts * 2 * sizeof (CFloatVector3)));
		OglTexCoordPointer (2, GL_FLOAT, 0, G3_BUFFER_OFFSET (pm->m_nFaceVerts * ((2 * sizeof (CFloatVector3) + sizeof (tRgbaColorf)))));
		}
	OglVertexPointer (3, GL_FLOAT, 0, G3_BUFFER_OFFSET (0));
	if (pm->m_vboIndexHandle)
		glBindBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, pm->m_vboIndexHandle);
	}
else {
	if (!gameStates.render.bCloaked) {
		OglTexCoordPointer (2, GL_FLOAT, 0, pm->m_vbTexCoord.Buffer ());
		if (gameOpts->ogl.bObjLighting)
			OglNormalPointer (GL_FLOAT, 0, pm->m_vbNormals.Buffer ());
		OglColorPointer (4, GL_FLOAT, 0, pm->m_vbColor.Buffer ());
		}
	OglVertexPointer (3, GL_FLOAT, 0, pm->m_vbVerts.Buffer ());
	}
nGunId = EquippedPlayerGun (objP);
nBombId = EquippedPlayerBomb (objP);
nMissileId = EquippedPlayerMissile (objP, &nMissiles);
if (!bHires && (objP->info.nType == OBJ_POWERUP)) {
	if ((objP->info.nId == POW_SMARTMINE) || (objP->info.nId == POW_PROXMINE))
		gameData.models.vScale.Set (I2X (2), I2X (2), I2X (2));
	else
		gameData.models.vScale.Set (I2X (3) / 2, I2X (3) / 2, I2X (3) / 2);
	}
#if 1 //!DBG
G3DrawModel (objP, nModel, nSubModel, modelBitmaps, animAnglesP, vOffsetP, bHires, bUseVBO, 0, nGunId, nBombId, nMissileId, nMissiles);
pm->m_bRendered = 1;
if (gameData.models.thrusters [nModel].nCount < 0)
	gameData.models.thrusters [nModel].nCount = -gameData.models.thrusters [nModel].nCount;
if ((objP->info.nType != OBJ_DEBRIS) && bHires) {
	if (pm->m_bHasTransparency & 1)
		G3DrawModel (objP, nModel, nSubModel, modelBitmaps, animAnglesP, vOffsetP, bHires, bUseVBO, 1, nGunId, nBombId, nMissileId, nMissiles);
#if 1
	if (pm->m_bHasTransparency & 2) {
		ogl.SetFaceCulling (false);
		G3DrawModel (objP, nModel, nSubModel, modelBitmaps, animAnglesP, vOffsetP, bHires, bUseVBO, 2, nGunId, nBombId, nMissileId, nMissiles);
		ogl.SetFaceCulling (true);
		}
#endif
	}
#endif
ogl.SetTexturing (false);
glBindBufferARB (GL_ARRAY_BUFFER_ARB, 0);
glBindBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
if (gameStates.render.bCloaked)
	ogl.DisableClientStates (0, 0, 0, -1);
else
	ogl.DisableClientStates (1, 1, gameOpts->ogl.bObjLighting, -1);
#if DBG
if (gameOpts->render.debug.bWireFrame)
	glLineWidth (3.0f);
#endif
#if 1 //!DBG
if (objP && ((objP->info.nType == OBJ_PLAYER) || (objP->info.nType == OBJ_ROBOT) || (objP->info.nType == OBJ_REACTOR))) {
	transformation.Begin (objP->info.position.vPos, objP->info.position.mOrient);
	G3RenderDamageLightning (objP, nModel, 0, animAnglesP, NULL, bHires);
	transformation.End ();
	}
#endif
#if DBG
if (gameOpts->render.debug.bWireFrame)
	glLineWidth (1.0f);
#endif
ogl.ClearError (0);
PROF_END(ptRenderObjectsFast)
}
return 1;
}

//------------------------------------------------------------------------------
//eof
