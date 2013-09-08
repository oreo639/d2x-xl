#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifndef _WIN32
#	include <unistd.h>
#endif

#include "descent.h"
#include "u_mem.h"
#include "error.h"
#include "ogl_lib.h"
#include "ogl_shader.h"
#include "ogl_fastrender.h"
#include "rendermine.h"
#include "segmath.h"
#include "objrender.h"
#include "lightmap.h"
#include "lightning.h"
#include "sphere.h"
#include "../effects/glow.h"
#include "glare.h"
#include "thrusterflames.h"
#include "automap.h"
#include "transprender.h"
#include "renderthreads.h"
#include "addon_bitmaps.h"

#define RENDER_TRANSPARENCY  1
#define RENDER_TRANSP_DECALS 1

#define TRANSP_POLYS			1
#define TRANSP_FACES			1
#define TRANSP_OBJECTS		1
#define TRANSP_SPRITES		1
#define TRANSP_SPARKS		1
#define TRANSP_BULLETS		1
#define TRANSP_PARTICLES	1
#define TRANSP_SPHERES		1
#define TRANSP_LIGHTNING	1
#define TRANSP_LIGHTTRAILS 1
#define TRANSP_THRUSTER		1

#define TI_POLY_OFFSET 0
#define TI_POLY_CENTER 1

#if DBG
int nDbgPoly = -1, nDbgItem = -1;
#endif

CTransparencyRenderer transparencyRenderer;

// Lazy reset controls when the entire list of transparent items is cleared.
// This is an optimization for stereoscopic rendering, where all transparent items
// need to be rendered twice. Now some of these items are passed to the 
// transparency renderer with their coordinates already transformed into view
// space, while others are passed with their world space coordinates, and 
// coordinate transformation is done by OpenGL when these items are rendered.
// The further kind of transparent items needs to be passed to the transparency
// renderer everytime the transformation changes, while the latter doesn't
// (since transformation changes are handled via OpenGL when rendering them).
// Smoke particles, which usually appear in greater numbers, belong to the latter
// category, so not having to process them twice for stereoscopic rendering is quite
// a performance improvement. Lazy reset makes sure such transparent items are
// only cleared from the buffer after the complete image (in the case of stereoscopic
// rendering left and right frame) have been rendered.
// To make handling this transparent to the various program parts passing stuff
// to the transparency renderer, the transparency renderer decides which items
// that are passed to it are accepted at what stage of the rendering process.
// LAZY_RESET controls this.

#define LAZY_RESET 1

//------------------------------------------------------------------------------

typedef struct tSparkVertex {
	CFloatVector3		vPos;
	tTexCoord2f	texCoord;
} tSparkVertex;

#define SPARK_BUF_SIZE	1000

typedef struct tSparkBuffer {
	int				nSparks;
	tSparkVertex	vertices [SPARK_BUF_SIZE * 4];
	CParticle		particles [SPARK_BUF_SIZE];
} tSparkBuffer;

tSparkBuffer sparkBuffer;
CEffectArea sparkArea;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CEffectArea& CEffectArea::Add (CFloatVector& pos, float rad) 
{
if (m_rad == 0.0f) {
	m_pos = pos;
	m_rad = rad;
	}
else {
	CFloatVector v = pos - m_pos;
	float d = v.Mag ();
	float h = rad + d - m_rad;
	if (h > 0.0f) {
		h /= 2.0f;
		v /= d;
		v *= h;
		m_pos += v;
		m_rad += h;
		}
	m_pos = CFloatVector::Avg (m_pos, pos);
	}
return *this;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTranspPoly::Render (void)
{
#if TRANSP_POLYS
if (faceP || triP)
	RenderFace ();
else {
	PROF_START
		int bSoftBlend = transparencyRenderer.SoftBlend (SOFT_BLEND_SPRITES);

	ogl.ResetClientStates (1);
	transparencyRenderer.Data ().bmP [1] = transparencyRenderer.Data ().bmP [2] = NULL;
	if (bAdditive & 3)
		glowRenderer.Begin (GLOW_POLYS, 2, false, 1.0f);
	ogl.EnableClientStates (bmP != NULL, nColors == nVertices, 0, GL_TEXTURE0);
	if (transparencyRenderer.LoadTexture (bmP, 0, 0, 0, nWrap)) {
		if (bmP)
			OglTexCoordPointer (2, GL_FLOAT, 0, texCoord);
		if (nColors == nVertices)
			OglColorPointer (4, GL_FLOAT, 0, color);
		else
			glColor4fv (reinterpret_cast<GLfloat*> (color));
		OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), vertices);
		ogl.SetBlendMode (bAdditive);
		if (!(bSoftBlend && glareRenderer.LoadShader (5, bAdditive != 0)))
			shaderManager.Deploy (-1, true);
		//ogl.SetTexturing (false);
		//glLineWidth (5);
		ogl.SetFaceCulling (false);
		ogl.SetupTransform (0);
		OglDrawArrays (nPrimitive, 0, nVertices);
		ogl.ResetTransform (0);
		ogl.SetFaceCulling (true);
		//glLineWidth (1);
		}
	if (bAdditive & 3)
		glowRenderer.Done (GLOW_POLYS);
	#if DBG
	else
		HUDMessage (0, "Couldn't load '%s'", bmP->Name ());
	#endif
	#if TI_POLY_OFFSET
	if (!bmP) {
		glPolygonOffset (0,0);
		glDisable (GL_POLYGON_OFFSET_FILL);
		}
	#endif
	PROF_END(ptRenderFaces)
	}
#endif
}

//------------------------------------------------------------------------------

void CTranspPoly::RenderFace (void)
{
#if TRANSP_FACES
PROF_START
	//CSegFace*		faceP = this->faceP;
	//tFaceTriangle*	triP = this->triP;
	CBitmap*			bmBot = bmP, *bmTop, *bmMask;
	int				bDecal, 
						bLightmaps = transparencyRenderer.Data ().bLightmaps && !gameStates.render.bFullBright,
						bTextured = (faceP->m_info.nSegColor == 0) && (bmBot != NULL), 
						bColored = (nColors == nVertices) && (bTextured || (gameStates.render.bPerPixelLighting != 2)) && !gameStates.render.bFullBright;

#if TI_POLY_OFFSET
if (faceP->m_info.nSegColor) {
	glEnable (GL_POLYGON_OFFSET_FILL);
	glPolygonOffset (1.0, -1.0);
	//glPolygonMode (GL_FRONT, GL_FILL);
	}
#endif

#if DBG
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide))) {
	nDbgSeg = nDbgSeg;
	}
#endif

if (bTextured) {
	if ((bmTop = faceP->bmTop))
		bmTop = bmTop->Override (-1);
	if (bmTop && !(bmTop->Flags () & (BM_FLAG_SUPER_TRANSPARENT | BM_FLAG_TRANSPARENT | BM_FLAG_SEE_THRU))) {
		bmBot = bmTop;
		bmTop = bmMask = NULL;
		bDecal = -1;
		faceP->m_info.nRenderType = gameStates.render.history.nType = 1;
		}
	else {
		bDecal = (bmTop != NULL);
		bmMask = (bDecal && ((bmTop->Flags () & BM_FLAG_SUPER_TRANSPARENT) != 0) && gameStates.render.textures.bHaveMaskShader) ? bmTop->Mask () : NULL;
		}
	}
else {
	bmBot = bmTop = bmMask = NULL;
	bDecal = 0;
	}

int bAdditive = this->bAdditive, nIndex = triP ? triP->nIndex : faceP->m_info.nIndex;
if (bAdditive & 3)
	glowRenderer.Begin (GLOW_FACES, 2, false, 1.0f);
#if 0 //DBG
transparencyRenderer.Data ().bmP [0] = NULL;
ogl.ResetClientStates (bLightmaps);
#else
ogl.ResetClientStates (bTextured + bLightmaps + (bmTop != NULL) + (bmMask != NULL));
if (!bTextured)
	ogl.SetTexturing (false);
#endif

if (bmTop) {
	ogl.EnableClientStates (bTextured, 0, 0, GL_TEXTURE1 + bLightmaps);
#if 0 //DBG
	transparencyRenderer.Data ().bmP [1] = NULL;
#endif
	if (!transparencyRenderer.LoadTexture (bmTop, 0, 1, bLightmaps, nWrap))
		return;
	if (bTextured)
		OglTexCoordPointer (2, GL_FLOAT, 0, FACES.ovlTexCoord + nIndex);
	OglVertexPointer (3, GL_FLOAT, 0, FACES.vertices + nIndex);
	if (bmMask) {
		ogl.EnableClientStates (bTextured, 0, 0, GL_TEXTURE2 + bLightmaps);
		if (!transparencyRenderer.LoadTexture (bmMask, 0, 2, bLightmaps, nWrap))
			return;
		if (bTextured)
			OglTexCoordPointer (2, GL_FLOAT, 0, FACES.ovlTexCoord + nIndex);
		OglVertexPointer (3, GL_FLOAT, 0, FACES.vertices + nIndex);
		}
	else
		transparencyRenderer.Data ().bmP [2] = NULL;
	}
else {
	transparencyRenderer.Data ().bmP [1] = transparencyRenderer.Data ().bmP [2] = NULL;
	}

if (bLightmaps)
	ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0 + bTextured);
else
	ogl.EnableClientStates (bTextured, bColored, 1, GL_TEXTURE0);
if (!transparencyRenderer.LoadTexture (bmBot, 0, 0, bLightmaps, nWrap)) {
	if (bAdditive & 3)
		glowRenderer.Done (GLOW_FACES);
	return;
	}	
if (bTextured)
	OglTexCoordPointer (2, GL_FLOAT, 0, (bDecal < 0) ? FACES.ovlTexCoord + nIndex : FACES.texCoord + nIndex);

if (!bLightmaps) {
	if (bColored)
		OglColorPointer (4, GL_FLOAT, 0, FACES.color + nIndex);
	OglNormalPointer (GL_FLOAT, 0, FACES.normals + nIndex);
	}
OglVertexPointer (3, GL_FLOAT, 0, FACES.vertices + nIndex);

if (bLightmaps) {
	ogl.EnableClientStates (1, bColored, 1, GL_TEXTURE0);
	OglTexCoordPointer (2, GL_FLOAT, 0, FACES.lMapTexCoord + nIndex);
	if (bColored)
		OglColorPointer (4, GL_FLOAT, 0, FACES.color + nIndex);
	OglNormalPointer (GL_FLOAT, 0, FACES.normals + nIndex);
	OglVertexPointer (3, GL_FLOAT, 0, FACES.vertices + nIndex);
	}
#if 0
if (!bTextured) {
	ogl.BindTexture (0);
	ogl.SetTexturing (false);
	transparencyRenderer.Data ().bmP [0] = NULL;
	}
#endif
ogl.SetupTransform (1);
if (!(bColored || bTextured))
	glColor4fv (reinterpret_cast<GLfloat*> (&faceP->m_info.color));
else if (gameStates.render.bFullBright)
	glColor4f (1.0f, 1.0f, 1.0f, color [0].Alpha ());
else
	glColor4fv (reinterpret_cast<GLfloat*> (color));
ogl.SetBlendMode (bAdditive);

#if DBG
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
//else
//	return;
//shaderManager.Deploy (-1);
#endif

if (gameStates.render.bPerPixelLighting && !gameStates.render.bFullBright) {
	if (!faceP->m_info.bColored) {
		SetupGrayScaleShader ((int) faceP->m_info.nRenderType, &faceP->m_info.color);
		OglDrawArrays (nPrimitive, 0, nVertices);
		}
	else {
		if (gameStates.render.bPerPixelLighting == 1) {
			SetupLightmapShader (faceP, bTextured ? int (faceP->m_info.nRenderType) : 0, false);
			OglDrawArrays (nPrimitive, 0, nVertices);
			}
		else {
			ogl.m_states.iLight = 0;
			lightManager.Index (0,0).nActive = -1;
			for (;;) {
				SetupPerPixelLightingShader (faceP, bTextured ? int (faceP->m_info.nRenderType) : 0, false);
				OglDrawArrays (nPrimitive, 0, nVertices);
				if (ogl.m_states.iLight >= ogl.m_states.nLights) 
					break;
				bAdditive = 1;
				ogl.SetBlendMode (OGL_BLEND_ADD);
				ogl.SetDepthWrite (false);
				}
			}
		if (gameStates.render.bHeadlights) {
#	if DBG
			if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
				nDbgSeg = nDbgSeg;
			ogl.SetBlending (true);
#	endif
			lightManager.Headlights ().SetupShader (bTextured ? bmMask ? 3 : bDecal ? 2 : 1 : 0, 1, bTextured ? NULL : &faceP->m_info.color);
			if (bAdditive != 2) {
				bAdditive = 2;
				ogl.SetBlendMode (OGL_BLEND_ADD_WEAK);
				ogl.SetDepthWrite (false);
				}
			OglDrawArrays (nPrimitive, 0, nVertices);
			}
		}
	}
else {
	SetupShader (faceP, bmMask != NULL, bDecal > 0, bmBot != NULL,
				    (nSegment < 0) || !automap.Display () || automap.m_visited [nSegment],
					 bTextured ? NULL : faceP ? &faceP->m_info.color : color);
	OglDrawArrays (nPrimitive, 0, nVertices);
	}
if (bAdditive & 3)
	glowRenderer.Done (GLOW_FACES);
ogl.ResetTransform (faceP != NULL);
gameData.render.nTotalFaces++;

#if TI_POLY_OFFSET
if (faceP->m_info.nSegColor) {
	glPolygonOffset (0,0);
	glDisable (GL_POLYGON_OFFSET_FILL);
	}
#endif
PROF_END(ptRenderFaces)
#endif
}

//------------------------------------------------------------------------------

void CTranspObject::Render (void)
{
#if TRANSP_OBJECTS
shaderManager.Deploy (-1);
ogl.ResetClientStates ();
gameData.models.vScale = vScale;
DrawPolygonObject (objP, 1);
gameData.models.vScale.SetZero ();
transparencyRenderer.ResetBitmaps ();
#endif
}

//------------------------------------------------------------------------------

void CTranspSprite::Render (void)
{
#if TRANSP_SPRITES
	int bSoftBlend = (fSoftRad > 0) && transparencyRenderer.SoftBlend (SOFT_BLEND_SPRITES);
	int bGlow = 0;

if (bAdditive < 0)
	bAdditive = -bAdditive;
else if (glowRenderer.Available (GLOW_SPRITES)) {
	bGlow = 1;
	if (bAdditive == 1) 
		glowRenderer.Begin (GLOW_SPRITES, 2, false, 0.85f);
	else if (bAdditive == 2)
		glowRenderer.Begin (GLOW_SPRITES, 1, false, 0.95f);
	else {
		bGlow = 0;
		if (glowRenderer.End ())
			transparencyRenderer.ResetBitmaps ();
		}
	}

ogl.ResetClientStates (1);
transparencyRenderer.Data ().bmP [1] = transparencyRenderer.Data ().bmP [2] = NULL;
transparencyRenderer.Data ().bUseLightmaps = 0;
ogl.SelectTMU (GL_TEXTURE0, true);
if (transparencyRenderer.LoadTexture (bmP, 0, 0, 0, GL_CLAMP)) {
	ogl.SetTexturing (true);
	transparencyRenderer.ResetBitmaps ();
	if (bColor)
		glColor4fv (reinterpret_cast<GLfloat*> (&color));
	else
		glColor3f (1,1,1);
	ogl.SetBlendMode (bAdditive);
	if (!(bSoftBlend && glareRenderer.LoadShader (fSoftRad, bAdditive != 0)))
		shaderManager.Deploy (-1, true);
	bmP->SetColor ();
	CFloatVector vPosf;
	transformation.Transform (vPosf, position, 0);
	if (!bGlow || glowRenderer.SetViewport (GLOW_SPRITES, *vPosf.XYZ (), X2F (nWidth), X2F (nHeight), true))
		ogl.RenderQuad (bmP, vPosf, X2F (nWidth), X2F (nHeight), 3);
	}
if (bGlow)
	glowRenderer.Done (GLOW_SPRITES);
#endif
}

//------------------------------------------------------------------------------

void CTranspSpark::Render (void)
{
#if TRANSP_SPARKS
	float	nCol = (float) (nFrame / 8);
	float	nRow = (float) (nFrame % 8);

if (!nType)
	nCol += 4;
if (USE_PARTICLE_SHADER) {
	if (sparkBuffer.nSparks >= SPARK_BUF_SIZE) {
		transparencyRenderer.FlushParticleBuffer (-1);
		sparkBuffer.nSparks = 0;
		}

	CParticle& p = sparkBuffer.particles [sparkBuffer.nSparks];

	p.m_nType =
	p.m_nRenderType = SMOKE_PARTICLES;
	p.m_nLife = p.m_nTTL = 1000;
	p.m_decay = 1.0f;
	p.m_nRad = 1.0f;
	p.m_renderColor.Red () = p.m_renderColor.Green () = p.m_renderColor.Blue () = p.m_renderColor.Alpha () = 1.0f;
	p.m_nWidth =
	p.m_nHeight = X2F (nSize);
	p.m_bEmissive = -1;
	p.m_bRotate = 1;
	p.m_nRotFrame = (nRotFrame + nFrame) % 64;
	p.m_vPosf = position;
	p.m_vPos.Assign (position);
	p.m_texCoord.v.v = nCol* 0.125f;
	p.m_texCoord.v.u = nRow* 0.125f; 
	particleManager.Add (&p, 1.0f);
	}
else {
	if (sparkBuffer.nSparks >= SPARK_BUF_SIZE)
		transparencyRenderer.FlushSparkBuffer ();

		tSparkVertex	*vertexP = sparkBuffer.vertices + 4 * sparkBuffer.nSparks;
		CFloatVector	vPos;
		float				nSize = X2F (this->nSize);

	sparkArea.Add (position, nSize);

	transformation.Transform (vPos, position, 0);
	vertexP->vPos.v.coord.x = vPos.v.coord.x - nSize;
	vertexP->vPos.v.coord.y = vPos.v.coord.y + nSize;
	vertexP->vPos.v.coord.z = vPos.v.coord.z;
	vertexP->texCoord.v.u = nCol* 0.125f;
	vertexP->texCoord.v.v = (nRow + 1)* 0.125f;
	vertexP++;
	vertexP->vPos.v.coord.x = vPos.v.coord.x + nSize;
	vertexP->vPos.v.coord.y = vPos.v.coord.y + nSize;
	vertexP->vPos.v.coord.z = vPos.v.coord.z;
	vertexP->texCoord.v.u = (nCol + 1)* 0.125f;
	vertexP->texCoord.v.v = (nRow + 1)* 0.125f;
	vertexP++;
	vertexP->vPos.v.coord.x = vPos.v.coord.x + nSize;
	vertexP->vPos.v.coord.y = vPos.v.coord.y - nSize;
	vertexP->vPos.v.coord.z = vPos.v.coord.z;
	vertexP->texCoord.v.u = (nCol + 1)* 0.125f;
	vertexP->texCoord.v.v = nRow* 0.125f;
	vertexP++;
	vertexP->vPos.v.coord.x = vPos.v.coord.x - nSize;
	vertexP->vPos.v.coord.y = vPos.v.coord.y - nSize;
	vertexP->vPos.v.coord.z = vPos.v.coord.z;
	vertexP->texCoord.v.u = nCol* 0.125f;
	vertexP->texCoord.v.v = nRow* 0.125f;
	}
sparkBuffer.nSparks++;
#endif
}

//------------------------------------------------------------------------------

void CTranspSphere::Render (void)
{
#if TRANSP_SPHERES
ogl.ResetClientStates ();
shaderManager.Deploy (-1, true);
if (nType == riSphereShield) {
	DrawShieldSphere (objP, color.Red (), color.Green (), color.Blue (), color.Alpha (), bAdditive, nSize);
	}
else if (nType == riMonsterball) {
	if (glowRenderer.End ())
		transparencyRenderer.ResetBitmaps ();
	DrawMonsterball (objP, color.Red (), color.Green (), color.Blue (), color.Alpha ());
	}
//shaderManager.Deploy (-1);
ogl.SetDepthWrite (false);
transparencyRenderer.ResetBitmaps ();
#endif
}

//------------------------------------------------------------------------------

void CTranspParticle::RenderBullet (CParticle *bullet)
{
#if TRANSP_BULLETS
	CObject	o;

memset (&o, 0, sizeof (o));
o.info.nType = OBJ_POWERUP;
o.info.position.vPos = bullet->m_vPos;
o.info.position.mOrient = bullet->m_mOrient;
if (0 <= (o.info.nSegment = FindSegByPos (o.info.position.vPos, bullet->m_nSegment, 0, 0))) {
	lightManager.Index (0,0).nActive = 0;
	o.info.renderType = RT_POLYOBJ;
	o.rType.polyObjInfo.nModel = BULLET_MODEL;
	o.rType.polyObjInfo.nTexOverride = -1;
	DrawPolygonObject (&o, 1);
	gameData.models.vScale.SetZero ();
	transparencyRenderer.ResetBitmaps ();
	}
#endif
}

//------------------------------------------------------------------------------

void CTranspParticle::Render (void)
{
#if TRANSP_PARTICLES
if (particle->m_nType == BULLET_PARTICLES)
	RenderBullet (particle);
else {
	if (particle->Render (fBrightness) < 0)
		transparencyRenderer.ResetBitmaps ();
	}
#endif
}

//------------------------------------------------------------------------------

void CTranspLightning::Render (void)
{
#if TRANSP_LIGHTNING
if (transparencyRenderer.Data ().nPrevType != transparencyRenderer.Data ().nCurType) {
	ogl.ResetClientStates ();
	shaderManager.Deploy (-1, true);
	}
lightning->Render (nDepth, 0);
transparencyRenderer.ResetBitmaps ();
#endif
}

//------------------------------------------------------------------------------

void CTranspLightTrail::Render (void)
{
#if TRANSP_LIGHTTRAILS
if (transparencyRenderer.Data ().nPrevType != transparencyRenderer.Data ().nCurType) {
	ogl.ResetClientStates (1);
	transparencyRenderer.Data ().bmP [1] = transparencyRenderer.Data ().bmP [2] = NULL;
	transparencyRenderer.Data ().bUseLightmaps = 0;
	shaderManager.Deploy (-1, true);
	}
glowRenderer.Begin (GLOW_LIGHTTRAILS, 2, false);
ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
if (transparencyRenderer.LoadTexture (bmP, 0, 0, 0, GL_CLAMP)) {
	ogl.SetDepthWrite (false); //true);
	ogl.SetFaceCulling (false);
	ogl.SetBlendMode (OGL_BLEND_ADD);
	glColor4fv (reinterpret_cast<GLfloat*> (&color));
	OglTexCoordPointer (2, GL_FLOAT, 0, texCoord);
	OglVertexPointer (3, GL_FLOAT, sizeof (CFloatVector), vertices);
	ogl.SetupTransform (1);
	OglDrawArrays (GL_QUADS, 0, bTrail ? 8 : 4);
	ogl.ResetTransform (1);
	ogl.SetFaceCulling (true);
	ogl.SetBlendMode (OGL_BLEND_ALPHA);
	}
glowRenderer.Done (GLOW_LIGHTTRAILS);
#endif
}

//------------------------------------------------------------------------------

void CTranspThruster::Render (void)
{
#if TRANSP_THRUSTER
shaderManager.Deploy (-1, true);
ogl.ResetClientStates ();
thrusterFlames.Render (objP, &info, nThruster);
gameData.models.vScale.SetZero ();
transparencyRenderer.ResetBitmaps ();
#endif
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int CTranspItemBuffers::Create (void)
{
	float scale = 1.0f / sqrt (float (gameStates.app.nThreads));

if (depthBuffer.Buffer ())
	return 1;
if (!depthBuffer.Create (uint (ITEM_DEPTHBUFFER_SIZE * scale)))
	return 0;
if (!itemHeap.Create (uint (ITEM_BUFFER_SIZE * scale))) {
	depthBuffer.Destroy ();
	return 0;
	}
nHeapSize = 0;
Clear ();
ResetFreeList ();
return 1;
}

//------------------------------------------------------------------------------

void CTranspItemBuffers::Destroy (void)
{
itemHeap.Destroy ();
depthBuffer.Destroy ();
}

//------------------------------------------------------------------------------

void CTranspItemBuffers::Clear (void)
{
if (depthBuffer.Buffer ())
	depthBuffer.Clear ();
#if DBG
if (itemHeap.Buffer ())
	memset (itemHeap.Buffer (), 0, nHeapSize);
#endif
}

//------------------------------------------------------------------------------

void CTranspItemBuffers::ResetFreeList (void)
{
memset (freeList, 0, sizeof (freeList));
}

//------------------------------------------------------------------------------

inline CTranspItem* CTranspItemBuffers::AllocItem (int nType, int nSize)
{
	CTranspItem* ti = freeList [nType];

if (ti) {
	freeList [nType] = ti->nextItemP;
	return ti;
	}
if (nHeapSize + nSize >= (int) itemHeap.Length ())
	return NULL;
nHeapSize += nSize;
return (CTranspItem*) itemHeap.Buffer (nHeapSize - nSize);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

static tTexCoord2f tcDefault [4] = {{{0,0}},{{1,0}},{{1,1}},{{0,1}}};

//------------------------------------------------------------------------------

int CTransparencyRenderer::ItemCount (int i) 
{ 
	int nCount = 0;

for (int j = 0; j < gameStates.app.nThreads; j++)
	nCount += m_data.buffers [j].nItems [i];
return nCount;
}

//------------------------------------------------------------------------------

inline int CTransparencyRenderer::HeapSize (void)
{
	int nHeapSize = 0;

for (int i = 0; i < gameStates.app.nThreads; i++) 
	nHeapSize += m_data.buffers [i].nHeapSize;
return nHeapSize;
}

//------------------------------------------------------------------------------

inline int CTransparencyRenderer::DepthBuffer (void)
{
for (int i = 0; i < gameStates.app.nThreads; i++) 
	if (m_data.buffers [i].depthBuffer.Buffer ())
		return i;
return -1;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AllocBuffers (void)
{
for (int i = 0; i < gameStates.app.nThreads; i++) 
	if (!m_data.buffers [i].Create ())
		return 0;
return 1;
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::FreeBuffers (void)
{
for (int i = 0; i < gameStates.app.nThreads; i++) 
	m_data.buffers [i].Destroy ();
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::ResetBuffers (void)
{
for (int i = 0; i < gameStates.app.nThreads; i++) 
	m_data.buffers [i].Clear ();
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::ResetFreeList (void)
{
for (int i = 0; i < gameStates.app.nThreads; i++) 
	m_data.buffers [i].ResetFreeList ();
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::Reset (void)
{
for (int i = 0; i < gameStates.app.nThreads; i++) 
	m_data.buffers [i].Reset ();
}

//------------------------------------------------------------------------------

inline CTranspItem* CTransparencyRenderer::AllocItem (int nType, int nSize, int nThread)
{
return m_data.buffers [nThread].AllocItem (nType, nSize);
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::InitBuffer (int zMin, int zMax, int nWindow)
{
#if LAZY_RESET
if (nWindow)
	m_data.bAllowAdd = 1;
else if (gameStates.render.cameras.bActive)
	m_data.bAllowAdd = 1;
else if (gameOpts->render.stereo.nGlasses && (ogl.StereoSeparation () >= 0))
	m_data.bAllowAdd = -1;
else
	m_data.bAllowAdd = 1;
if (m_data.bAllowAdd > 0) 
#endif
	{
	m_data.zMin = 0;
	m_data.zMax = zMax;
#if DBG
	if (zMax < 0)
		zMax = zMax;
#endif
	m_data.zScale = (double) (ITEM_DEPTHBUFFER_SIZE - 1) / (double) (m_data.zMax * sqrt (double (gameStates.app.nThreads)));
	if (m_data.zScale < 0)
		m_data.zScale = 1;
	else if (m_data.zScale > 1)
		m_data.zScale = 1;
	m_data.vViewer [0] = gameData.render.mine.viewer.vPos;
	transformation.Transform (m_data.vViewer [1], m_data.vViewer [0]);
	m_data.vViewerf [0].Assign (m_data.vViewer [0]);
	m_data.vViewerf [1].Assign (m_data.vViewer [1]);
	}
m_data.bRenderGlow =
m_data.bSoftBlend = 0;
m_data.bReady = 1;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::Add (CTranspItem* item, CFixVector vPos, int nOffset, bool bClamp, int bTransformed)
{
//if (gameStates.render.nType == RENDER_TYPE_TRANSPARENCY)
//	return 0;
if (!Ready ())
	return 0;
#if LAZY_RESET
if (!m_data.bAllowAdd)
	return 0;
if ((bTransformed > 0) && (m_data.bAllowAdd < 0))	// already added and still in buffer
	return 0;
#endif

#if RENDER_TRANSPARENCY

int nDepth = Depth (vPos, bTransformed > 0);
	
if (nDepth >= I2X (nOffset))
	nDepth -= I2X (nOffset);

if (nDepth < 0) {
	if (!bClamp)
		return 1;
	nDepth = m_data.zMin;
	}
else if (nDepth > m_data.zMax) {
	if (!bClamp)
		return 1;
	nDepth = m_data.zMax;
	}

// find the first particle to insert the new one *before* and place in pj; pi will be it's predecessor (NULL if to insert at list start)
nOffset = int (double (nDepth) * m_data.zScale);

#if USE_OPENMP > 1
int nThread = omp_get_thread_num ();
#	if DBG
if (nThread > gameStates.app.nThreads)
	return 0;
if (nThread == gameStates.app.nThreads)
	nThread = nThread;
#	endif
#else
int nThread = 0;
#endif

CTranspItemBuffers& buffer = m_data.buffers [nThread];

if (nOffset >= (int) buffer.depthBuffer.Length ()) 
	return 0;
CTranspItem* ph = buffer.AllocItem (item->Type (), item->Size ());
if (!ph) {
	return 0;
	}

memcpy (ph, item, item->Size ());
ph->nItem = buffer.nItems [0]++;
ph->bRendered = 0;
ph->bTransformed = bTransformed;
ph->z = nDepth;
ph->bValid = 1;
CTranspItem** pd = buffer.depthBuffer + nOffset;
#if 0 // sort by depth
CTranspItem* pi;
for (pi = pd->head; pi; pi = pi->nextItemP) {
	if ((pi->z < nDepth) || ((pi->z == nDepth) && (pi->nType < nType)))
		break;
	}
if (pi) {
	ph->nextItemP = pi->nextItemP;
	pi->nextItemP = ph;
	}
else 
#endif
	{
	ph->nextItemP = *pd;
	*pd = ph;
	}
if (buffer.nMinOffs > nOffset)
	buffer.nMinOffs = nOffset;
if (buffer.nMaxOffs < nOffset)
	buffer.nMaxOffs = nOffset;
return 1;
#else // RENDER_TRANSPARENCY
return 0;
#endif // RENDER_TRANSPARENCY
}

//------------------------------------------------------------------------------

#if TI_SPLIT_POLYS

int CTransparencyRenderer::SplitPoly (tTranspPoly *item, int nDepth)
{
	tTranspPoly		split [2];
	CFloatVector		vSplit;
	CFloatVector	color;
	int			i, l, i0, i1, i2, i3, nMinLen = 0x7fff, nMaxLen = 0;
	float			z, zMin, zMax, *coord, *c0, *c1;

split [0] = split [1] = *item;
for (i = i0 = 0; i < split [0].nVertices; i++) {
	l = split [0].sideLength [i];
	if (nMaxLen < l) {
		nMaxLen = l;
		i0 = i;
		}
	if (nMinLen > l)
		nMinLen = l;
	}
if ((nDepth > 1) || !nMaxLen || (nMaxLen < 10) || ((nMaxLen <= 30) && ((split [0].nVertices == 3) || (nMaxLen <= nMinLen / 2 * 3)))) {
	for (i = 0, zMax = 0, zMin = 1e30f; i < split [0].nVertices; i++) {
		z = split [0].vertices [i].dir.coord.z;
		if (zMax < z)
			zMax = z;
		if (zMin > z)
			zMin = z;
		}
#if TI_POLY_CENTER
	return Add (item->bmP ? riTexPoly : riFlatPoly, item, sizeof (*item), F2X (zMax), F2X ((zMax + zMin) / 2));
#else
	return Add (item->bmP ? riTexPoly : riFlatPoly, item, sizeof (*item), F2X (zMax), F2X (zMin));
#endif
	}
if (split [0].nVertices == 3) {
	i1 = (i0 + 1) % 3;
	vSplit = CFloatVector::Avg(split [0].vertices [i0], split [0].vertices [i1]);
	split [0].vertices [i0] =
	split [1].vertices [i1] = vSplit;
	split [0].sideLength [i0] =
	split [1].sideLength [i0] = nMaxLen / 2;
	if (split [0].bmP) {
		split [0].texCoord [i0].dir.u =
		split [1].texCoord [i1].dir.u = (split [0].texCoord [i1].dir.u + split [0].texCoord [i0].dir.u) / 2;
		split [0].texCoord [i0].dir.dir =
		split [1].texCoord [i1].dir.dir = (split [0].texCoord [i1].dir.dir + split [0].texCoord [i0].dir.dir) / 2;
		}
	if (split [0].nColors == 3) {
		for (i = 4, coord = reinterpret_cast<float*> (&color, c0 = reinterpret_cast<float*> (split [0].color + i0), c1 = reinterpret_cast<float*> (split [0].color + i1); i; i--)
			*coord++ = (*c0++ + *c1++) / 2;
		split [0].color [i0] =
		split [1].color [i1] = color;
		}
	}
else {
	i1 = (i0 + 1) % 4;
	i2 = (i0 + 2) % 4;
	i3 = (i1 + 2) % 4;
	vSplit = CFloatVector::Avg(split [0].vertices [i0], split [0].vertices [i1]);
	split [0].vertices [i1] =
	split [1].vertices [i0] = vSplit;
	vSplit = CFloatVector::Avg(split [0].vertices [i2], split [0].vertices [i3]);
	split [0].vertices [i2] =
	split [1].vertices [i3] = vSplit;
	if (split [0].bmP) {
		split [0].texCoord [i1].dir.u =
		split [1].texCoord [i0].dir.u = (split [0].texCoord [i1].dir.u + split [0].texCoord [i0].dir.u) / 2;
		split [0].texCoord [i1].dir.dir =
		split [1].texCoord [i0].dir.dir = (split [0].texCoord [i1].dir.dir + split [0].texCoord [i0].dir.dir) / 2;
		split [0].texCoord [i2].dir.u =
		split [1].texCoord [i3].dir.u = (split [0].texCoord [i3].dir.u + split [0].texCoord [i2].dir.u) / 2;
		split [0].texCoord [i2].dir.dir =
		split [1].texCoord [i3].dir.dir = (split [0].texCoord [i3].dir.dir + split [0].texCoord [i2].dir.dir) / 2;
		}
	if (split [0].nColors == 4) {
		for (i = 4, coord = reinterpret_cast<float*> (&color, c0 = reinterpret_cast<float*> (split [0].color + i0), c1 = reinterpret_cast<float*> (split [0].color + i1); i; i--)
			*coord++ = (*c0++ + *c1++) / 2;
		split [0].color [i1] =
		split [1].color [i0] = color;
		for (i = 4, coord = reinterpret_cast<float*> (&color, c0 = reinterpret_cast<float*> (split [0].color + i2), c1 = reinterpret_cast<float*> (split [0].color + i3); i; i--)
			*coord++ = (*c0++ + *c1++) / 2;
		split [0].color [i2] =
		split [1].color [i3] = color;
		}
	split [0].sideLength [i0] =
	split [1].sideLength [i0] =
	split [0].sideLength [i2] =
	split [1].sideLength [i2] = nMaxLen / 2;
	}
return SplitPoly (split, nDepth + 1) && SplitPoly (split + 1, nDepth + 1);
}

#endif

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddObject (CObject *objP)
{
if (gameStates.render.nShadowMap)
	return 0;

	CTranspObject	item;
//	CFixVector		vPos;

if (objP->info.nType == 255)
	return 0;
item.objP = objP;
item.vScale = gameData.models.vScale;
//transformation.Transform (vPos, OBJPOS (objP)->vPos, 0);
return Add (&item, OBJPOS (objP)->vPos, 0, 0, -1);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddPoly (CSegFace *faceP, tFaceTriangle *triP, CBitmap *bmP,
												CFloatVector *vertices, char nVertices, tTexCoord2f *texCoord, CFloatVector *color,
												CFaceColor* altColor, char nColors, char bDepthMask, int nPrimitive, int nWrap, int bAdditive,
												short nSegment)
{
if (gameStates.render.nShadowMap)
	return 0;

	CTranspPoly		item;
	int				i;
	float				s = gameStates.render.grAlpha;
	//fix			zCenter;

item.faceP = faceP;
item.triP = triP;
item.bmP = bmP;
item.nVertices = nVertices;
item.nPrimitive = nPrimitive;
item.nWrap = nWrap;
item.bDepthMask = bDepthMask;
item.bAdditive = bAdditive;
item.nSegment = nSegment;
memcpy (item.texCoord, texCoord ? texCoord : tcDefault, nVertices * sizeof (tTexCoord2f));
if ((item.nColors = nColors)) {
	if (nColors < nVertices)
		nColors = 1;
	if (color) {
		memcpy (item.color, color, nColors * sizeof (CFloatVector));
		for (i = 0; i < nColors; i++)
			if (bAdditive)
				item.color [i].Alpha () = 1;
			else
				item.color [i].Alpha () *= s;
		}
	else if (altColor) {
		for (i = 0; i < nColors; i++) {
			item.color [i] = altColor [i];
			if (bAdditive)
				item.color [i].Alpha () = 1;
			else
				item.color [i].Alpha () *= s;
			}
		}
	else
		item.nColors = 0;
	}
memcpy (item.vertices, vertices, nVertices * sizeof (CFloatVector));
#if TI_SPLIT_POLYS
if (bDepthMask && m_data.bSplitPolys) {
	for (i = 0; i < nVertices; i++)
		item.sideLength [i] = (short) (CFloatVector::Dist(vertices [i], vertices [(i + 1) % nVertices]) + 0.5f);
	return SplitPoly (&item, 0);
	}
else
#endif
	{
	if (faceP)
		return Add (&item, SEGMENTS [faceP->m_info.nSegment].Side (faceP->m_info.nSide)->Center (), 0, true, false);
	CFloatVector v = item.vertices [0];
	for (i = 1; i < item.nVertices; i++) 
		v += item.vertices [i];
	v /= item.nVertices;
	CFixVector vPos;
	vPos.Assign (v);
	if (bAdditive & 3)
		m_data.bRenderGlow = 1;
	if (gameOpts->SoftBlend (SOFT_BLEND_SPRITES) != 0)
		m_data.bSoftBlend = 1;
	return Add (&item, vPos, 0, true);
	}
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddFaceTris (CSegFace *faceP)
{
if (gameStates.render.nShadowMap)
	return 0;

	tFaceTriangle*	triP;
	CFloatVector	vertices [3];
	int				h, i, j, bAdditive = FaceIsAdditive (faceP);
	CBitmap*			bmP = faceP->m_info.bTextured ? /*faceP->bmTop ? faceP->bmTop :*/ faceP->bmBot : NULL;

if (bmP)
	bmP = bmP->Override (-1);
#if DBG
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	faceP = faceP;
#endif
triP = FACES.tris + faceP->m_info.nTriIndex;
for (h = faceP->m_info.nTris; h; h--, triP++) {
	for (i = 0, j = triP->nIndex; i < 3; i++, j++) {
#if 1
		transformation.Transform (vertices [i], *(reinterpret_cast<CFloatVector*> (FACES.vertices + j)), 0);
#else
		if (automap.Display ())
			transformation.Transform (vertices + i, gameData.segs.fVertices + triP->index [i], 0);
		else
			vertices [i].Assign (RENDERPOINTS [triP->index [i]].m_vertex [1]);
#endif
		}
	if (!AddPoly (faceP, triP, bmP, vertices, 3, FACES.texCoord + triP->nIndex,
					  FACES.color + triP->nIndex,
					  NULL, 3, (bmP != NULL) && !bAdditive, GL_TRIANGLES, GL_REPEAT,
					  bAdditive, faceP->m_info.nSegment))
		return 0;
	}
return 1;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddFaceQuads (CSegFace *faceP)
{
if (gameStates.render.nShadowMap)
	return 0;

	CFloatVector	vertices [4];
	int				i, j, bAdditive = FaceIsAdditive (faceP);
	CBitmap*			bmP = faceP->m_info.bTextured ? /*faceP->bmTop ? faceP->bmTop :*/ faceP->bmBot : NULL;

if (bmP)
	bmP = bmP->Override (-1);
#if DBG
if ((faceP->m_info.nSegment == nDbgSeg) && ((nDbgSide < 0) || (faceP->m_info.nSide == nDbgSide)))
	faceP = faceP;
#endif
for (i = 0, j = faceP->m_info.nIndex; i < 4; i++, j++) {
#if 1
	transformation.Transform (vertices [i], *(reinterpret_cast<CFloatVector*> (FACES.vertices + j)), 0);
#else
	if (automap.Display ())
		transformation.Transform(vertices [i], gameData.segs.fVertices [faceP->m_info.index [i]], 0);
	else
		vertices [i].Assign (RENDERPOINTS [faceP->m_info.index [i]].m_vertex [1]);
#endif
	}
return AddPoly (faceP, NULL, bmP,
					 vertices, 4, FACES.texCoord + faceP->m_info.nIndex,
					 FACES.color + faceP->m_info.nIndex,
					 NULL, 4, (bmP != NULL) && !bAdditive, GL_TRIANGLE_FAN, GL_REPEAT,
					 bAdditive, faceP->m_info.nSegment) > 0;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddSprite (CBitmap *bmP, const CFixVector& position, CFloatVector *color,
												  int nWidth, int nHeight, char nFrame, char bAdditive, float fSoftRad)
{
if (gameStates.render.nShadowMap)
	return 0;

	CTranspSprite	item;

item.bmP = bmP;
if ((item.bColor = (color != NULL)))
	item.color = *color;
item.nWidth = nWidth;
item.nHeight = nHeight;
item.nFrame = nFrame;
item.bAdditive = bAdditive;
item.fSoftRad = fSoftRad;
item.position.Assign (position);
if (bAdditive & 3)
	m_data.bRenderGlow = 1;
if (gameOpts->SoftBlend (SOFT_BLEND_SPRITES) != 0)
	m_data.bSoftBlend = 1;
return Add (&item, position);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddSpark (const CFixVector& position, char nType, int nSize, char nFrame, char nRotFrame, char nOrient)
{
#if 1
if (gameStates.render.nShadowMap)
	return 0;

	CTranspSpark	item;
//	CFixVector		vPos;

item.nSize = nSize;
item.nFrame = nFrame;
item.nRotFrame = nRotFrame;
item.nOrient = nOrient;
item.nType = nType;
item.position.Assign (position);
if (gameOpts->SoftBlend (SOFT_BLEND_SPARKS))
	m_data.bSoftBlend = 1;
return Add (&item, position);
#else
return 0;
#endif
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddSphere (tTranspSphereType nType, float red, float green, float blue, float alpha, 
												  CObject *objP, char bAdditive, fix nSize)
{
if (gameStates.render.nShadowMap)
	return 0;

	CTranspSphere	item;
	//CFixVector		vPos;

item.nType = nType;
item.color.Red () = red;
item.color.Green () = green;
item.color.Blue () = blue;
item.color.Alpha () = alpha;
item.nSize = nSize;
item.bAdditive = bAdditive;
item.objP = objP;
if (nType != riMonsterball)
	m_data.bRenderGlow = 1;
//transformation.Transform (vPos, objP->info.position.vPos, 0);
return Add (&item, objP->info.position.vPos);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddParticle (CParticle *particle, float fBrightness, int nThread)
{
if (gameStates.render.nShadowMap)
	return 0;

	CTranspParticle	item;
//	fix					z;

if ((particle->m_nType < 0) || (particle->m_nType >= PARTICLE_TYPES))
	return 0;
item.particle = particle;
item.fBrightness = fBrightness;
//particle->Transform (gameStates.render.bPerPixelLighting == 2);
if (gameOpts->SoftBlend (SOFT_BLEND_PARTICLES))
	m_data.bSoftBlend = 1;
return Add (&item, particle->m_vPos, 10);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddLightning (CLightning *lightningP, short nDepth)
{
if (gameStates.render.nShadowMap)
	return 0;

	CTranspLightning	item;
	bool					bSwap;
	//CFixVector			vPos;

item.lightning = lightningP;
item.nDepth = nDepth;
#if 0
transformation.Transform (vPos, lightningP->m_vPos, 0);
z = vPos.dir.coord.z;
transformation.Transform (vPos, lightningP->m_vEnd, 0);
if (z < vPos.dir.coord.z)
	z = vPos.dir.coord.z;
#endif
fix d1 = Depth (lightningP->m_vPos, false);
fix d2 = Depth (lightningP->m_vEnd, false);
if ((bSwap = (d1 < d2)))
	::Swap (d1, d2);
if (d2 > m_data.zMax)
	return 0;
if (d1 < m_data.zMin)
	return 0;
m_data.bRenderGlow = 1;
if (!Add (&item, bSwap ? lightningP->m_vEnd : lightningP->m_vPos /*CFixVector::Avg (lightningP->m_vPos, lightningP->m_vEnd)*/, 0, true, 0)) // -1))
	return 0;
return 1;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddLightTrail (CBitmap *bmP, CFloatVector *vThruster, tTexCoord2f *tcThruster, CFloatVector *vFlame, tTexCoord2f *tcFlame, CFloatVector *colorP)
{
if (gameStates.render.nShadowMap)
	return 0;

	CTranspLightTrail	item;
	int					i, j, iMin = 0;
	CFixVector			v;
	float					d, dMin = 1e30f;

item.bmP = bmP;
if ((item.bTrail = (vFlame != NULL))) {
	memcpy (item.vertices, vFlame, 4 * sizeof (CFloatVector));
	memcpy (item.texCoord, tcFlame, 4 * sizeof (tTexCoord2f));
	j = 4;
	}
else
	j = 0;
memcpy (item.vertices + j, vThruster, 4 * sizeof (CFloatVector));
memcpy (item.texCoord + j, tcThruster, 4 * sizeof (tTexCoord2f));
item.color = *colorP;
for (i = 0; i < j; i++) {
	d = Depth (item.vertices [i], true);
	if (dMin > d) {
		dMin = d;
		iMin = i;
		}
	}
v.Assign (item.vertices [iMin]);
m_data.bRenderGlow = 1;
return Add (&item, v, 0, false, 0);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::AddThruster (CObject* objP, tThrusterInfo* infoP, int nThruster)
{
if (gameStates.render.nShadowMap)
	return 0;

	CTranspThruster item;

item.objP = objP;
item.info = *infoP;
item.nThruster = nThruster;
m_data.bRenderGlow = 1;
return Add (&item, infoP->vPos [nThruster], 0, false, 0);
}

//------------------------------------------------------------------------------

inline void CTransparencyRenderer::ResetBitmaps (void)
{
m_data.bmP [0] =
m_data.bmP [1] =
m_data.bmP [2] = NULL;
m_data.bDecal = 0;
m_data.bTextured = 0;
m_data.nFrame = -1;
m_data.bUseLightmaps = 0;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::LoadTexture (CBitmap *bmP, int nFrame, int bDecal, int bLightmaps, int nWrap)
{
if (bmP) {
#if 0
	ogl.SelectTMU (GL_TEXTURE0 + bLightmaps, true);
	ogl.SetTexturing (true);
#endif
	if ((bmP != m_data.bmP [bDecal]) || (nFrame != m_data.nFrame) || (nWrap != m_data.nWrap)) {
		gameData.render.nStateChanges++;
		if (bmP) {
			if (bmP->Bind (1)) {
				ResetBitmaps ();
				return 0;
				}
			if (bDecal != 2)
				bmP = bmP->Override (nFrame);
			bmP->Texture ()->Wrap (nWrap);
			m_data.nWrap = nWrap;
			m_data.nFrame = nFrame;
			}
		else
			ogl.BindTexture (0);
		m_data.bmP [bDecal] = bmP;
		}
	}
else {
	ogl.BindTexture (0);
	ogl.SetTexturing (false);
	ResetBitmaps ();
	}
return 1;
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::FlushSparkBuffer (void)
{
if (DepthBuffer () < 0)
	return;

if (!sparkBuffer.nSparks)
	return;

sparkArea.Reset ();

	int bSoftBlend = SoftBlend (SOFT_BLEND_SPARKS);

ogl.ResetClientStates (1);
m_data.bmP [1] = m_data.bmP [2] = NULL;
m_data.bUseLightmaps = 0;
ogl.EnableClientStates (1, 0, 0, GL_TEXTURE0);
if (LoadTexture (sparks.Bitmap (), 0, 0, 0, GL_CLAMP)) {
	if (!(bSoftBlend && glareRenderer.LoadShader (3, 1)))
		shaderManager.Deploy (-1, true);
	ogl.SetBlendMode (OGL_BLEND_ADD);
	glColor3f (1,1,1);
	OglTexCoordPointer (2, GL_FLOAT, sizeof (tSparkVertex), &sparkBuffer.vertices [0].texCoord);
	OglVertexPointer (3, GL_FLOAT, sizeof (tSparkVertex), &sparkBuffer.vertices [0].vPos);
	OglDrawArrays (GL_QUADS, 0, 4 * sparkBuffer.nSparks);
	ogl.SetBlendMode (OGL_BLEND_ALPHA);
	ogl.SetDepthTest (true);
	sparkBuffer.nSparks = 0;
	}
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::FlushParticleBuffer (int nType)
{
if (DepthBuffer () < 0)
	return;
if (!HeapSize ())
	return;

if ((nType < 0) || ((nType != tiParticle) && (particleManager.LastType () >= 0))) {
	ResetBitmaps ();
	if (sparkBuffer.nSparks && !USE_PARTICLE_SHADER && particleManager.Overlap (sparkArea))
		FlushSparkBuffer ();
	if (particleManager.Flush (-1.0f, true)) {
		if (nType < 0)
			particleManager.CloseBuffer ();
		particleManager.SetLastType (-1);
		m_data.bClientColor = 1;
		sparkBuffer.nSparks = 0;
		ResetBitmaps ();
		}
	}
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::FlushBuffers (int nType, CTranspItem *item)
{
if (DepthBuffer () < 0)
	return;
if (!HeapSize ())
	return;

if (glowRenderer.Available (GLOW_LIGHTNING | GLOW_SHIELDS | GLOW_SPRITES | GLOW_THRUSTERS) && 
	 (nType != tiLightning) && (nType != tiSphere) && (nType != tiSprite) && (nType != tiThruster)) {
	if (glowRenderer.End ())
		ResetBitmaps ();
	}
if (USE_PARTICLE_SHADER) {
	if ((nType != tiSpark) && (nType != tiParticle)) {
		//FlushSparkBuffer ();
		FlushParticleBuffer (nType);
		}
	}	
else {
	if (nType == tiSpark) {
		if (!item || particleManager.Overlap (((CTranspSpark*) item)->position, X2F (((CTranspSpark*) item)->nSize)))
			FlushParticleBuffer (nType);
		}
	else if (nType == tiParticle) {
		if (!item || sparkArea.Overlap (((CTranspParticle*) item)->particle->Posf (), ((CTranspParticle*) item)->particle->Rad ()))
			FlushSparkBuffer ();
		}	
	else {
		FlushSparkBuffer ();
		FlushParticleBuffer (nType);
		}
	}
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::RenderItem (CTranspItem *itemP)
{
if (!itemP->bRendered) {
	//itemP->bRendered = true;
	m_data.nPrevType = m_data.nCurType;
	m_data.nCurType = itemP->Type ();
#if DBG
	if (gameOpts->render.debug.bTextures && gameOpts->render.debug.bWalls)
#endif
	try {
		FlushBuffers (m_data.nCurType, itemP);
		ogl.SetBlendMode (OGL_BLEND_ALPHA);
		ogl.SetDepthWrite (false);
		ogl.SetDepthMode (GL_LEQUAL);
		ogl.SetDepthTest (true);
		itemP->Render ();
		if ((m_data.nCurType != tiSphere) && (m_data.nPrevType == tiSphere)) // spheres somehow mess up the glow renderer; I cannot determine why though
			glowRenderer.End ();
		}
	catch(...) {
		PrintLog (0, "invalid transparent render item (type: %d)\n", m_data.nCurType);
		}
	}
return m_data.nCurType;
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::NeedDepthBuffer (void)
{
if (!gameOpts->render.bUseShaders)
	return 0;
if (!gameOpts->render.effects.bEnabled)
	return 0;
if (!ogl.m_states.bGlowRendering)
	return true;
if (gameStates.render.cameras.bActive && !gameOpts->render.cameras.bHires)
	return 0;
return int (m_data.bRenderGlow || m_data.bSoftBlend);
}

//------------------------------------------------------------------------------

int CTransparencyRenderer::SoftBlend (int nFlag)
{
return m_data.bHaveDepthBuffer && !gameStates.render.cameras.bActive && (gameOpts->SoftBlend (nFlag) != 0);
}

//------------------------------------------------------------------------------

void CTransparencyRenderer::RenderBuffer (CTranspItemBuffers& buffer, bool bCleanup)
{
CTranspItem* currentP = *buffer.bufP, * nextP, * prevP;
if (bCleanup)
	*buffer.bufP = NULL;
prevP = NULL;
do {
#if DBG
	if (currentP->nItem == nDbgItem)
		nDbgItem = nDbgItem;
#endif
	buffer.nItems [0]--;
#if 0
	if (ogl.m_data.xStereoSeparation < 0)
#endif
	RenderItem (currentP);

	nextP = currentP->nextItemP;
	if (bCleanup)
		currentP->nextItemP = NULL;
	else if (currentP->bTransformed) {	// remove items that have transformed coordinates when stereo rendering since these items will be reentered with different coordinates
		int nType = currentP->Type ();
		currentP->nextItemP = buffer.freeList [nType];
		buffer.freeList [nType] = currentP;
		if (prevP)
			prevP->nextItemP = nextP;
		else
			*buffer.bufP = nextP;
		}
	else
		prevP = currentP;
	currentP = nextP;
	} while (currentP);
}

//------------------------------------------------------------------------------

extern int bLog;

void CTransparencyRenderer::Render (int nWindow)
{
#if RENDER_TRANSPARENCY
	int				bStencil;
	bool				bCleanup = !LAZY_RESET || (ogl.StereoSeparation () >= 0) || nWindow;

if (!AllocBuffers ())
	return;
if (!HeapSize ())
	return;
if (gameStates.render.cameras.bActive)
	nWindow = nWindow;
//HUDMessage (0, "transp. render heap size: %d.%03d.%03d", HeapSize () / 1000000, (HeapSize () % 1000000) / 1000, HeapSize () % 1000);

PROF_START
gameStates.render.nType = RENDER_TYPE_TRANSPARENCY;
ogl.ChooseDrawBuffer ();
shaderManager.Deploy (-1);
bStencil = ogl.StencilOff ();
ResetBitmaps ();
m_data.bReady = 0;
m_data.bTextured = -1;
m_data.bUseLightmaps = 0;
m_data.bDecal = 0;
m_data.bLightmaps = lightmapManager.HaveLightmaps ();
m_data.bSplitPolys = (gameStates.render.bPerPixelLighting != 2) && (gameStates.render.bSplitPolys > 0);
m_data.nWrap = 0;
m_data.nFrame = -1;
m_data.bmP [0] =
m_data.bmP [1] = NULL;
if (!glowRenderer.Available (0xFFFFFFFF))
	m_data.bRenderGlow = 0;
if (gameOptions [0].render.nQuality < 3)
	m_data.bSoftBlend = 0;
sparkBuffer.nSparks = 0;
ogl.DisableLighting ();
ogl.ResetClientStates ();
m_data.bHaveParticles = particleImageManager.LoadAll ();
ogl.SetBlendMode (OGL_BLEND_ALPHA);
ogl.SetDepthMode (GL_LEQUAL);
ogl.SetDepthWrite (false);
ogl.SetFaceCulling (true);
ogl.SetViewport (gameData.render.screen.Left (), gameData.render.screen.Top (), gameData.render.screen.Width (), gameData.render.screen.Height ());
m_data.bHaveDepthBuffer = NeedDepthBuffer () && ogl.CopyDepthTexture (1);
particleManager.BeginRender (-1, 1);
//ogl.ChooseDrawBuffer ();
ogl.SetViewport (gameData.render.frame.Left (), gameData.render.frame.Top (), gameData.render.frame.Width (), gameData.render.frame.Height ());
m_data.nCurType = -1;

int h = -1, nBuffers = 0;

for (int i = 0; i < gameStates.app.nThreads; i++)
	if (m_data.buffers [i].nItems [0]) {
		if (h < 0)
			h = i;
		nBuffers++;
		}

if (nBuffers < 2) {
	CTranspItemBuffers& buffer = m_data.buffers [h];

	m_data.buffers [h].nItems [1] = m_data.buffers [h].nItems [0];
	for (buffer.bufP = &buffer.depthBuffer [buffer.nMaxOffs]; buffer.nItems [0] && (buffer.bufP >= buffer.depthBuffer.Buffer ()); buffer.bufP--)
		if (*buffer.bufP)
			RenderBuffer (buffer, bCleanup);
	}
else {
	CTranspItemBuffers* buffers [MAX_THREADS];
	
	nBuffers = 0;
	for (int i = 0; i < gameStates.app.nThreads; i++)
		if (m_data.buffers [i].nItems [0]) {
			buffers [nBuffers] = &m_data.buffers [i];
			buffers [nBuffers]->bufP = &m_data.buffers [i].depthBuffer [buffers [nBuffers]->nMaxOffs];
			m_data.buffers [i].nItems [1] = m_data.buffers [i].nItems [0];
			nBuffers++;
			}

	while (nBuffers > 0) {
		for (int i = 0; i < nBuffers; i++) {
			if (buffers [i]->bufP) {
				if (*buffers [i]->bufP) 
					RenderBuffer (*buffers [i], bCleanup);
				if (!buffers [i]->nItems [0] || (--buffers [i]->bufP < buffers [i]->depthBuffer.Buffer ())) {
					if (i < --nBuffers)
						buffers [i--] = buffers [nBuffers];
					}
				}
			}
		}
	}


ogl.SetBlendMode (OGL_BLEND_ALPHA);
ogl.SetDepthWrite (false);
ogl.SetDepthMode (GL_LEQUAL);
ogl.SetDepthTest (true);
glowRenderer.End ();
FlushBuffers (-1);
particleManager.EndRender ();
shaderManager.Deploy (-1);
ogl.ResetClientStates ();
ogl.SetTexturing (false);
ogl.SetBlendMode (OGL_BLEND_ALPHA);
ogl.SetDepthMode (GL_LEQUAL);
ogl.SetDepthWrite (true);
ogl.StencilOn (bStencil);

if (bCleanup) {
	ResetFreeList ();
	for (int i = 0; i < gameStates.app.nThreads; i++) {
#if 0
		for (int j = 0, l = int (m_data.buffers [i].depthBuffer.Length ()); j < l; j++) {
			if (m_data.buffers [i].depthBuffer [j]) {
				m_data.buffers [i].depthBuffer [j] = NULL;
				}
			}
#endif
		m_data.buffers [i].nItems [0] = 0;
		m_data.buffers [i].nMinOffs = ITEM_DEPTHBUFFER_SIZE;
		m_data.buffers [i].nMaxOffs = 0;
		m_data.buffers [i].nHeapSize = 0;
		}
	}
else {
	for (int i = 0; i < gameStates.app.nThreads; i++)
		m_data.buffers [i].nItems [0] = m_data.buffers [i].nItems [1];
	}

PROF_END(ptTranspPolys)
#endif
}

//------------------------------------------------------------------------------
//eof
