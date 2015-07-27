
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

//------------------------------------------------------------------------------

int CParticleSystem::Create (CFixVector *vPos, CFixVector *vDir, CFixMatrix *mOrient,
									  short nSegment, int nMaxEmitters, int nMaxParts,
									  float fScale, /*int nDensity, int nPartsPerPos,*/ int nLife, int nSpeed, char nType,
									  int nObject, CFloatVector *colorP, int bBlowUpParts, char nSide)
{
	int			i;
	CFixVector	vEmittingFace [4];

if (nSide >= 0)
	SEGMENTS [nSegment].GetCornerVertices (nSide, vEmittingFace);
nMaxParts = MAX_PARTICLES (nMaxParts, gameOpts->render.particles.nDens [0]);
if (!m_emitters.Create (nMaxEmitters)) {
	return 0;
	}
if ((m_nObject = nObject) < 0x70000000) {
	m_nSignature = OBJECTS [nObject].info.nSignature;
	m_nObjType = OBJECTS [nObject].info.nType;
	m_nObjId = OBJECTS [nObject].info.nId;
	}
m_nEmitters = 0;
m_nLife = nLife;
m_nSpeed = nSpeed;
m_nBirth = gameStates.app.nSDLTicks [0];
m_nMaxEmitters = nMaxEmitters;
for (i = 0; i < nMaxEmitters; i++)
	if (m_emitters [i].Create (vPos, vDir, mOrient, nSegment, nObject, nMaxParts, fScale, /*nDensity, nPartsPerPos,*/ 
										nLife, nSpeed, nType, colorP, gameStates.app.nSDLTicks [0], bBlowUpParts, (nSide < 0) ? NULL : vEmittingFace))
		m_nEmitters++;
	else {
		particleManager.Destroy (m_nId);
		return -1;
		}
m_nType = nType;
m_bValid = 1;
return 1;
}

//	-----------------------------------------------------------------------------

void CParticleSystem::Init (int nId)
{
m_nId = nId;
m_nObject = -1;
m_nObjType = -1;
m_nObjId = -1;
m_nSignature = -1;
m_bValid = 0;
m_bDestroy = false;
}

//------------------------------------------------------------------------------

void CParticleSystem::Destroy (void)
{
m_bValid = 0;
m_bDestroy = false;
if (m_emitters.Buffer ()) {
	m_emitters.Destroy ();
	if ((m_nObject >= 0) && (m_nObject < 0x70000000))
		particleManager.SetObjectSystem (m_nObject, -1);
	m_nObject = -1;
	m_nObjType = -1;
	m_nObjId = -1;
	m_nSignature = -1;
	}
}

//------------------------------------------------------------------------------

int CParticleSystem::Render (int nThread)
{
if (m_bValid < 1)
	return 0;

	int	h = 0;
	CParticleEmitter* emitterP = m_emitters.Buffer ();

if (emitterP) {
	if (!particleImageManager.Load (m_nType))
		return 0;
	if ((m_nObject >= 0) && (m_nObject < 0x70000000) &&
		 ((OBJECTS [m_nObject].info.nType == OBJ_NONE) ||
		  (OBJECTS [m_nObject].info.nSignature != m_nSignature) ||
		  (particleManager.GetObjectSystem (m_nObject) < 0)))
		SetLife (0);
	for (int i = m_nEmitters; i; i--, emitterP++)
		h += emitterP->Render (nThread);
	}
#if DBG
if (!h)
	return 0;
#endif
return h;
}

//------------------------------------------------------------------------------

void CParticleSystem::SetPos (CFixVector *vPos, CFixMatrix *mOrient, short nSegment)
{
if (m_bValid && m_emitters.Buffer ())
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetPos (vPos, mOrient, nSegment);
}

//------------------------------------------------------------------------------

void CParticleSystem::SetDensity (int nMaxParts/*, int nDensity*/)
{
if (m_bValid && m_emitters.Buffer ()) {
	nMaxParts = MAX_PARTICLES (nMaxParts, gameOpts->render.particles.nDens [0]);
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetDensity (nMaxParts/*, nDensity*/);
	}
}

//------------------------------------------------------------------------------

void CParticleSystem::SetScale (float fScale)
{
if (m_bValid && m_emitters.Buffer ())
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetScale (fScale);
}

//------------------------------------------------------------------------------

void CParticleSystem::SetLife (int nLife)
{
if (m_bValid && m_emitters.Buffer () && (m_nLife != nLife)) {
	m_nLife = nLife;
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetLife (nLife);
	}
}

//------------------------------------------------------------------------------

void CParticleSystem::SetBlowUp (int bBlowUpParts)
{
if (m_bValid && m_emitters.Buffer ()) {
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetBlowUp (bBlowUpParts);
	}
}

//------------------------------------------------------------------------------

void CParticleSystem::SetBrightness (int nBrightness)
{
if (m_bValid && m_emitters.Buffer ())
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetBrightness (nBrightness);
}

//------------------------------------------------------------------------------

void CParticleSystem::SetFadeType (int nFadeType)
{
if (m_bValid && m_emitters.Buffer ())
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetFadeType (nFadeType);
}

//------------------------------------------------------------------------------

void CParticleSystem::SetType (int nType)
{
if (m_bValid && m_emitters.Buffer () && (m_nType != nType)) {
	m_nType = nType;
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetType (nType);
	}
}

//------------------------------------------------------------------------------

void CParticleSystem::SetSpeed (int nSpeed)
{
if (m_bValid && m_emitters.Buffer () && (m_nSpeed != nSpeed)) {
	m_nSpeed = nSpeed;
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetSpeed (nSpeed);
	}
}

//------------------------------------------------------------------------------

void CParticleSystem::SetDir (CFixVector *vDir)
{
if (m_bValid && m_emitters.Buffer ())
	for (int i = 0; i < m_nEmitters; i++)
		m_emitters [i].SetDir (vDir);
}

//------------------------------------------------------------------------------

int CParticleSystem::RemoveEmitter (int i)
{
if (m_bValid && m_emitters.Buffer () && (i < m_nEmitters)) {
	m_emitters [i].Destroy ();
	if (i < --m_nEmitters)
		m_emitters [i] = m_emitters [m_nEmitters];
	}
return m_nEmitters;
}

//------------------------------------------------------------------------------

int CParticleSystem::Update (int nThread)
{
if (m_bValid < 1)
	return 0;

	CParticleEmitter*				emitterP;
	CArray<CParticleEmitter*>	emitters;
	int								nEmitters = 0;

if ((m_nObject == 0x7fffffff) && (m_nType <= SMOKE_PARTICLES) &&
	 (gameStates.app.nSDLTicks [0] - m_nBirth > (MAX_SHRAPNEL_LIFE / I2X (1)) * 1000))
	SetLife (0);

if ((emitterP = m_emitters.Buffer ()) && emitters.Create (m_nEmitters)) {
	bool bKill = (m_nObject < 0) || ((m_nObject < 0x70000000) &&
					 ((OBJECTS [m_nObject].info.nSignature != m_nSignature) || (OBJECTS [m_nObject].info.nType == OBJ_NONE)));
	while (nEmitters < m_nEmitters) {
		if (!emitterP)
			return 0;
		if (emitterP->IsDead (gameStates.app.nSDLTicks [0])) {
			if (!RemoveEmitter (nEmitters)) {
				m_bDestroy = true;
				break;
				}
			}
		else {
			if (bKill)
				emitterP->SetLife (0);
			emitterP->Update (gameStates.app.nSDLTicks [0], nThread);
			nEmitters++, emitterP++;
			}
		}
	}
return nEmitters;
}

//------------------------------------------------------------------------------
//eof
