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

CFloatVector defaultParticleColor = {{{ 1.0f, 1.0f, 1.0f, 1.0f }}};

CFloatVector CParticle::vRot [PARTICLE_POSITIONS];
CFixMatrix CParticle::mRot [2][PARTICLE_POSITIONS];
CFixMatrix CParticle::mSparkOrient;

static int32_t smokeStartAlpha [2][5] = {{160, 128, 96, 64, 32}, {128, 96, 64, 32, 16}};

//------------------------------------------------------------------------------

static inline int32_t RandN (int32_t n) 
{
return n ? int32_t (RandFloat () * float (n)) : 0;
}

//------------------------------------------------------------------------------

inline float sqr (float n) 
{
return n * n;
}

//------------------------------------------------------------------------------
// Create a luminance value for an RGB color
// Greater values have stronger influence

static inline float Luminance (CFloatVector& color)
{
#if 1 
return sqrt ((sqr (Min (color.v.color.r, 1.0f) * 255.0f) + sqr (Min (color.v.color.g, 1.0f) * 255.0f) + sqr (Min (color.v.color.b, 1.0f) * 255.0f)) / 3.0f) / 255.0f;
#else
return pow ((sqr (Min (color.v.color.r, 1.0f) * 255.0f, 3.0f) + sqr (Min (color.v.color.g, 1.0f) * 255.0f, 3.0f) + sqr (Min (color.v.color.b, 1.0f) * 255.0f, 3.0f)) / 3.0f, 1.0f / 3.0f) / 255.0f;
#endif
}

//------------------------------------------------------------------------------

static inline float SmokeStartAlpha (char bBlowUp, char nClass)
{
int32_t alpha = smokeStartAlpha [int32_t (bBlowUp)][gameOpts->render.particles.nAlpha [gameOpts->app.bExpertMode ? int32_t (nClass) : 0]];
return float (3 * alpha / 4 + RandN (alpha / 2)) / 255.0f;
}

//------------------------------------------------------------------------------

inline float ParticleBrightness (CFloatVector *pColor) 
{
#if 0
return (pColor->Red () + pColor->Green () + pColor->Blue ()) / 3.0f;
#else
return pColor ? (pColor->Red () * 3 + pColor->Green () * 5 + pColor->Blue () * 2) / 10.0f : 1.0f;
#endif
}

//------------------------------------------------------------------------------

CFixVector* RandomPointOnTriangle (CFixVector *triangle, CFixVector *vPos) 
{
fix a = 2 * RandShort ();
fix b = 2 * RandShort ();
if (a + b > I2X (1)) {
	a = I2X (1) - a;
	b = I2X (1) - b;
	}
fix c = I2X (1) - a - b;
*vPos = triangle [0] * a;
*vPos += triangle [1] * b;
*vPos += triangle [2] * c;
return vPos;
}

//------------------------------------------------------------------------------

CFixVector* RandomPointOnQuad (CFixVector *quad, CFixVector *vPos) 
{
	int32_t		i;
	CFixVector	v [4];

memcpy (v, quad, sizeof (v));
for (i = 0; i < 4; i++)
	if ((v [i].v.coord.x == 0x7fffffff) && (v [i].v.coord.y == 0x7fffffff) && (v [i].v.coord.z == 0x7fffffff))
		break;
if (i < 3)
	return NULL;
if (i == 3)
	return RandomPointOnTriangle (v, vPos);
if (Rand (2))
	return RandomPointOnTriangle (v, vPos);
v [1] = v [0];
return RandomPointOnTriangle (v + 1, vPos);
}

//------------------------------------------------------------------------------

void CParticle::InitRotation (void)
{
CAngleVector vRotAngs;
vRotAngs.SetZero ();
for (int32_t i = 0; i < PARTICLE_POSITIONS; i++) {
	vRotAngs.v.coord.b = i * (I2X (1) / PARTICLE_POSITIONS);
	CParticle::mRot [0][i] = CFixMatrix::Create (vRotAngs);
	}
}

//------------------------------------------------------------------------------

void CParticle::SetupRotation (void)
{
for (int32_t i = 0; i < PARTICLE_POSITIONS; i++)
	mRot [1][i] = gameData.renderData.mine.viewer.mOrient * mRot [0][i];
}

//------------------------------------------------------------------------------

#define RANDOM_FADE	 (0.95f + RandFloat (20.0f))

static int32_t brightFlags [PARTICLE_TYPES] = {1,1,0,0,0,1,1,0,1,1};

void CParticle::InitColor (CFloatVector* pColor, float fBrightness, char nParticleSystemType)
{
	CFloatVector color;

m_bEmissive = (nParticleSystemType == LIGHT_PARTICLES) 
					? 1
					: (nParticleSystemType == FIRE_PARTICLES) 
						? 2
						//: ((nParticleSystemType <= SMOKE_PARTICLES) || (nParticleSystemType <= WATERFALL_PARTICLES))
						//	? 3
						: 0;
m_color [0] = m_color [1] = color = CFloatVector::Min ((pColor && (m_bEmissive != 2)) ? *pColor : defaultParticleColor, 1.0f);

if (!brightFlags [(int32_t) m_nType]) {
	m_bBright = 0;
	m_nFadeTime = -1;
	if (pColor && (pColor->Alpha () < 0)) {
		uint8_t a = (uint8_t) FRound (-pColor->Alpha () * 255.0f * 0.25f);
		m_color [1].Alpha () = float (3 * a + RandN (2 * a)) / 255.0f;
		}
	}
else {
	m_bBright = (m_nType <= SMOKE_PARTICLES) ? Rand (50) == 0 : 0;
	if (pColor) {
		if (!m_bEmissive /*|| (m_bEmissive == 3)*/) {
			m_color [0].Red () *= RANDOM_FADE;
			m_color [0].Green () *= RANDOM_FADE;
			m_color [0].Blue () *= RANDOM_FADE;
			}
		m_nFadeTime = 0;
		} 
	else {
		pColor = &m_color [0];
		m_color [0].Alpha () = 2.0f;
		}
	if (m_bEmissive /*&& (m_bEmissive < 3)*/)
		; // m_color [0].Alpha () = float (SMOKE_START_ALPHA + 64) / 255.0f;
	else if (nParticleSystemType != GATLING_PARTICLES) {
		if (!pColor)
			m_color [1].Alpha () = SmokeStartAlpha (m_bBlowUp, m_nClass);
		else if (pColor->Alpha () < 0) {
			uint8_t a = (uint8_t) FRound (-pColor->Alpha () * 255.0f * 0.25f);
			m_color [1].Alpha () = float (3 * a + RandN (2 * a)) / 255.0f;
			} 
		else if (char (pColor->Alpha ()) != 2.0f) 
			m_color [1].Alpha () = SmokeStartAlpha (m_bBlowUp, m_nClass);
		else {
			if ((m_bEmissive = (gameOpts->render.particles.nQuality > 2))) {
				m_color [0].Red () = 0.5f + RandFloat (2.0f);
				m_color [0].Green () = m_color [0].Red () * (0.5f + RandFloat (2.0f));
				}
			else {
#if 1
				m_color [0].Red () = 
				m_color [0].Green () = 1.0;
#else
				m_color [0].Red () = 0.9f + RandFloat (10.0f);
				m_color [0].Green () = m_color [0].Red () * (0.9f + RandFloat (10.0f));
#endif
				}
			m_color [0].Blue () = 0.0f;
			m_nFadeTime = 50 + Rand (150);
			m_color [1].Red () *= RANDOM_FADE;
			m_color [1].Green () *= RANDOM_FADE;
			m_color [1].Blue () *= RANDOM_FADE;
			m_nWidth *= 0.75;
			m_nHeight *= 0.75;
			m_color [1].Alpha () = SmokeStartAlpha (m_bBlowUp, m_nClass);
			}
		if (m_bBlowUp && !m_bBright) {
			fBrightness = 1.0f - fBrightness;
			if (m_nFadeTime <= 0)
				m_color [1].Alpha () += fBrightness * fBrightness / 8.0f;
			}
		}
	}

if (nParticleSystemType == SIMPLE_SMOKE_PARTICLES)
	m_color [1].Alpha () /= 3.5f - float (1 + int32_t (gameOpts->render.particles.nQuality > 1)) / 2.0f; //pColor ? 2.0f + (color.Red () + color.Green () + color.Blue ()) / 3.0f : 2.0f;
else if (nParticleSystemType == SMOKE_PARTICLES)
#if 1
	m_color [1].Alpha () /= pColor ? 3.0f - Luminance (color) : 2.5f;
#else
	m_color [1].Alpha () /= pColor ? 3.0f - (color.Red () + color.Green () + color.Blue ()) / 3.0f : 2.5f;
#endif
else if ((nParticleSystemType == BUBBLE_PARTICLES) || (nParticleSystemType == RAIN_PARTICLES) || (nParticleSystemType == SNOW_PARTICLES))
	m_color [1].Alpha () /= 2.0f;
else if (nParticleSystemType == GATLING_PARTICLES)
	m_color [1].Alpha () /= 4.0f;
if (m_bEmissive)
	m_color [0].Alpha () = (m_nFadeTime > 0) ? 0.8f + RandFloat (5.0f) : 1.0f;
else
	m_color [0].Alpha () = m_color [1].Alpha ();
}

//------------------------------------------------------------------------------

int32_t CParticle::InitDrift (CFixVector* vDir, int32_t nSpeed)
{
#if 0
if (nType == FIRE_PARTICLES)
nSpeed = int32_t (sqrt (double (nSpeed)) * float (I2X (1)));
else
#endif
nSpeed *= I2X (1);
if (!vDir) {
	m_vDrift.v.coord.x = nSpeed - RandN (2 * nSpeed);
	m_vDrift.v.coord.y = nSpeed - RandN (2 * nSpeed);
	m_vDrift.v.coord.z = nSpeed - RandN (2 * nSpeed);
	m_vDir.SetZero ();
	m_bHaveDir = 1;
	}
else {
	m_vDir = *vDir;

	if (m_nType == RAIN_PARTICLES)
		m_vDrift = m_vDir;
	else {
#if 1
		m_vDrift.v.coord.x = RandN (I2X (1) / 4) - I2X (1) / 8;
		m_vDrift.v.coord.y = RandN (I2X (1) / 4) - I2X (1) / 8;
		m_vDrift.v.coord.z = RandN (I2X (1) / 4) - I2X (1) / 8;
		m_vDrift += m_vDir;
#else
		CAngleVector a;
		CFixMatrix m;
		a.v.coord.p = RandN (I2X (1) / 4) - I2X (1) / 8;
		a.v.coord.b = RandN (I2X (1) / 4) - I2X (1) / 8;
		a.v.coord.h = RandN (I2X (1) / 4) - I2X (1) / 8;
		m = CFixMatrix::Create (a);
		if (m_nType == WATERFALL_PARTICLES)
			CFixVector::Normalize (m_vDir);
		m_vDrift = m * m_vDir;
#endif
		CFixVector::Normalize (m_vDrift);
		if (m_nType == WATERFALL_PARTICLES) {
			fix dot = CFixVector::Dot (m_vDir, m_vDrift);
			if (dot < I2X (1) / 2)
				return 0;
			}
		float d = float (CFixVector::DeltaAngle (m_vDrift, m_vDir, NULL));
		if (d) {
			d = (float) exp ((I2X (1) / 8) / d);
			nSpeed = (fix) ((float) nSpeed / d);
			}
		}
	m_vDrift *= nSpeed;
	if (m_nType <= FIRE_PARTICLES)
		m_vDir *= (I2X (3) / 4 + I2X (RandN (16)) / 64);
	m_bHaveDir = 1;
	}
return 1;
}

//------------------------------------------------------------------------------

bool CParticle::InitPosition (CFixVector* vPos, CFixVector* vEmittingFace, CFixMatrix *mOrient, bool bPointSource)
{
if (vEmittingFace)
	m_vPos = *RandomPointOnQuad (vEmittingFace, vPos);
else if ((m_nType != BUBBLE_PARTICLES) && (m_nType != RAIN_PARTICLES) && (m_nType != SNOW_PARTICLES))
	m_vPos = *vPos + m_vDrift * (I2X (1) / 64);
else if (!bPointSource) {
	//m_vPos = *vPos + vDrift * (I2X (1) / 32);
	int32_t nSpeed = m_vDrift.Mag () / 16;
	CFixVector v = CFixVector::Avg ((*mOrient).m.dir.r * (nSpeed - RandN (2 * nSpeed)), (*mOrient).m.dir.u * (nSpeed - RandN (2 * nSpeed)));
	m_vPos = *vPos + v + (*mOrient).m.dir.f * (I2X (1) / 2 - RandN (I2X (1)));
	}
m_vStartPos = m_vPos;
return true;
}

//------------------------------------------------------------------------------

void CParticle::InitSize (float nScale, CFixMatrix *mOrient)
{
if (nScale < 0)
	m_nRad = float (-nScale);
else if (gameOpts->render.particles.bSyncSizes)
	m_nRad = float (PARTICLE_SIZE (gameOpts->render.particles.nSize [0], nScale, m_bBlowUp));
else
	m_nRad = float (nScale);
if (!m_nRad)
	m_nRad = 1.0f;

if ((m_nType == BUBBLE_PARTICLES) || (m_nType == SNOW_PARTICLES))
	m_nRad = m_nRad / 20 + float (RandN (int32_t (9 * m_nRad / 20)));
else {
	if (m_nType <= SMOKE_PARTICLES) {
		if (!m_bBlowUp)
			m_nLife = 2 * m_nLife / 3;
		m_nLife = 4 * m_nLife / 5 + RandN (2 * m_nLife / 5);
		m_nRad += float (RandN (int32_t (m_nRad)));
		}
	else if (m_nType == FIRE_PARTICLES) {
		m_nLife = 3 * m_nLife / 4 + RandN (m_nLife / 4);
		m_nRad += float (RandN (int32_t (m_nRad)));
		}
	else
		m_nRad *= 2;
	if (mOrient) {
		static CFixMatrix mRot [9 * 9];
		static char bInit = 1;

		if (bInit) {
			bInit = 0;
			for (int32_t p = 0; p < 9; p++) { 
				for (int32_t h = 0; h < 9; h++) {
					CAngleVector vRot;
					vRot.v.coord.b = 0;
					vRot.v.coord.p = 2048 - (p * 512);
					vRot.v.coord.h = 2048 - (h * 512);
					mRot [p * 9 + h] = CFixMatrix::Create (vRot);
					}
				}
			}
		m_mOrient = *mOrient * mRot [Rand (9) * 9 + Rand (9)];
		}
	}

if (m_bBlowUp) {
	m_nWidth = (m_nType == WATERFALL_PARTICLES) 
				  ? m_nRad * 0.6666667f
				  : m_nRad;
	m_nHeight = m_nRad;
	}
else {
	m_nWidth = (m_nType == WATERFALL_PARTICLES) 
				  ? m_nRad * 0.3333333f
			     : m_nRad/* * 2*/;
	m_nHeight = m_nRad /** 2*/;
	}
m_nWidth /= 65536.0f;
m_nHeight /= 65536.0f;
m_nRad /= 65536.0f;
}

//------------------------------------------------------------------------------

void CParticle::InitAnimation (void)
{
m_nFrames = ParticleImageInfo (m_nType).nFrames;
m_deltaUV = 1.0f / float (m_nFrames);
if (m_nType == BULLET_PARTICLES) {
	m_iFrame = 0;
	m_nRotFrame = 0;
	m_nOrient = 3;
	}
else if ((m_nType == RAIN_PARTICLES) || (m_nType == SNOW_PARTICLES)) {
	m_iFrame = 0;
	m_nRotFrame = 0;
	m_nOrient = 1;
	}
else if (m_nType == BUBBLE_PARTICLES) {
	m_iFrame = Rand (m_nFrames * m_nFrames);
	m_nRotFrame = 0;
	m_nOrient = 0;
	}
else if ((m_nType == LIGHT_PARTICLES) || (m_nType == GATLING_PARTICLES)) {
	m_iFrame = 0;
	m_nRotFrame = 0;
	m_nOrient = 0;
	}
else if (m_nType == FIRE_PARTICLES) {
	m_iFrame = (Rand (10) < 6) ? 0 : 2; // more fire than smoke (60:40)
	if (m_iFrame < 2)
		m_nLife = 9 * m_nLife / 10;
	else
		m_nLife = 10 * m_nLife / 9;
	m_nRotFrame = Rand (PARTICLE_POSITIONS);
	m_nOrient = Rand (4);
	}
else {
	m_iFrame = Rand (m_nFrames * m_nFrames);
	m_nRotFrame = Rand (PARTICLE_POSITIONS);
	m_nOrient = Rand (4);
	}
m_bAnimate = (m_nType != FIRE_PARTICLES) && (gameOpts->render.particles.nQuality > 1) && (m_nFrames > 1);
m_bRotate = ((m_nRenderType <= SMOKE_PARTICLES) || (m_nRenderType == SNOW_PARTICLES)) ? 1 : (m_nRenderType == FIRE_PARTICLES + PARTICLE_TYPES) ? -1 : 0;
}

//------------------------------------------------------------------------------

static int32_t bounceFlags [PARTICLE_TYPES] = {2,2,1,1,1,2,1,2,2,2};

int32_t CParticle::Create (CFixVector *vPos, CFixVector *vDir, CFixMatrix *mOrient,
									int16_t nSegment, int32_t nLife, int32_t nSpeed, char nParticleSystemType,
									char nClass, float nScale, CFloatVector *pColor, int32_t nCurTime,
									int32_t bBlowUp, char nFadeType, float fBrightness,
									CFixVector *vEmittingFace) 
{
	int32_t nType = particleImageManager.GetType (nParticleSystemType);

m_bChecked = 0;
m_bBlowUp = bBlowUp && gameOpts->render.particles.bDisperse;
m_nType = nType;
m_nClass = nClass;
m_nFadeType = nFadeType;
m_nSegment = nSegment;
m_bSkybox = SEGMENT (nSegment)->Function () == SEGMENT_FUNC_SKYBOX;
#if DBG
if (nSegment < 0)
	BRP;
#endif
m_nBounce = bounceFlags [(int32_t) m_nType];
m_bReversed = 0;
m_nUpdated = m_nMoved = nCurTime;
if (nLife < 0)
	nLife = -nLife;
m_nLife = nLife;
m_nDelay = 0; //bStart ? RandN (nLife) : 0;
m_nRenderType = RenderType ();

bool bPointSource = nSpeed < 0;
if (bPointSource)
	nSpeed = -nSpeed;

#if 0

InitColor (pColor, fBrightness, nParticleSystemType);
if (!InitDrift (vDir, nSpeed))
	return 0;
if (!InitPosition (vPos, vEmittingFace, mOrient, bPointSource)) // needs InitDrift() to be executed first!
	return 0;
InitSize (nScale, mOrient);
InitAnimation ();

#else

// init color

	CFloatVector color;

m_bEmissive = (nParticleSystemType == LIGHT_PARTICLES) 
					? 1
					: (nParticleSystemType == FIRE_PARTICLES) 
						? 2
						//: ((nParticleSystemType <= SMOKE_PARTICLES) || (nParticleSystemType <= WATERFALL_PARTICLES))
						//	? 3
						: 0;
m_color [0] = m_color [1] = color = CFloatVector::Min ((pColor && (m_bEmissive != 2)) ? *pColor : defaultParticleColor, 1.0f);

if (!brightFlags [(int32_t) m_nType]) {
	m_bBright = 0;
	m_nFadeTime = -1;
	if (pColor && (pColor->Alpha () < 0)) {
		uint8_t a = (uint8_t) FRound (-pColor->Alpha () * 255.0f * 0.25f);
		m_color [1].Alpha () = float (3 * a + RandN (2 * a)) / 255.0f;
		}
	}
else {
	m_bBright = (m_nType <= SMOKE_PARTICLES) ? Rand (50) == 0 : 0;
	if (pColor) {
		if (!m_bEmissive /*|| (m_bEmissive == 3)*/) {
			m_color [0].Red () *= RANDOM_FADE;
			m_color [0].Green () *= RANDOM_FADE;
			m_color [0].Blue () *= RANDOM_FADE;
			}
		m_nFadeTime = 0;
		} 
	else {
		pColor = &m_color [0];
		m_color [0].Alpha () = 2.0f;
		}
	if (m_bEmissive /*&& (m_bEmissive < 3)*/)
		; // m_color [0].Alpha () = float (SMOKE_START_ALPHA + 64) / 255.0f;
	else if (nParticleSystemType != GATLING_PARTICLES) {
		if (!pColor)
			m_color [1].Alpha () = SmokeStartAlpha (m_bBlowUp, m_nClass);
		else if (pColor->Alpha () < 0) {
			uint8_t a = (uint8_t) FRound (-pColor->Alpha () * 255.0f * 0.25f);
			m_color [1].Alpha () = float (3 * a + RandN (2 * a)) / 255.0f;
			} 
		else if (char (pColor->Alpha ()) != 2.0f) 
			m_color [1].Alpha () = SmokeStartAlpha (m_bBlowUp, m_nClass);
		else {
			if ((m_bEmissive = (gameOpts->render.particles.nQuality > 2))) {
				m_color [0].Red () = 0.5f + RandFloat (2.0f);
				m_color [0].Green () = m_color [0].Red () * (0.5f + RandFloat (2.0f));
				}
			else {
#if 1
				m_color [0].Red () = 
				m_color [0].Green () = 1.0;
#else
				m_color [0].Red () = 0.9f + RandFloat (10.0f);
				m_color [0].Green () = m_color [0].Red () * (0.9f + RandFloat (10.0f));
#endif
				}
			m_color [0].Blue () = 0.0f;
			m_nFadeTime = (nParticleSystemType == SIMPLE_SMOKE_PARTICLES) ? 25 + Rand (75) : 50 + Rand (150);
			m_color [1].Red () *= RANDOM_FADE;
			m_color [1].Green () *= RANDOM_FADE;
			m_color [1].Blue () *= RANDOM_FADE;
			//m_nWidth *= 0.75;
			//m_nHeight *= 0.75;
			m_color [1].Alpha () = SmokeStartAlpha (m_bBlowUp, m_nClass);
			}
		if (m_bBlowUp && !m_bBright) {
			fBrightness = 1.0f - fBrightness;
			if (m_nFadeTime <= 0)
				m_color [1].Alpha () += fBrightness * fBrightness / 8.0f;
			}
		}
	}

if (nParticleSystemType == SIMPLE_SMOKE_PARTICLES)
	m_color [1].Alpha () /= 3.5f - float (1 + int32_t (gameOpts->render.particles.nQuality > 1)) / 2.0f; //pColor ? 2.0f + (color.Red () + color.Green () + color.Blue ()) / 3.0f : 2.0f;
else if (nParticleSystemType == SMOKE_PARTICLES)
#if 1
	m_color [1].Alpha () /= pColor ? 3.0f - Luminance (color) : 2.5f;
#else
	m_color [1].Alpha () /= pColor ? 3.0f - (color.Red () + color.Green () + color.Blue ()) / 3.0f : 2.5f;
#endif
else if ((nParticleSystemType == BUBBLE_PARTICLES) || (nParticleSystemType == RAIN_PARTICLES) || (nParticleSystemType == SNOW_PARTICLES))
	m_color [1].Alpha () /= 2.0f;
else if (nParticleSystemType == GATLING_PARTICLES)
	m_color [1].Alpha () /= 4.0f;
if (m_bEmissive)
	m_color [0].Alpha () = (m_nFadeTime > 0) ? 0.8f + RandFloat (5.0f) : 1.0f;
else
	m_color [0].Alpha () = m_color [1].Alpha ();


// init drift

#if 0
if (nType == FIRE_PARTICLES)
nSpeed = int32_t (sqrt (double (nSpeed)) * float (I2X (1)));
else
#endif
nSpeed *= I2X (1);
if (!vDir) {
	m_vDrift.v.coord.x = nSpeed - RandN (2 * nSpeed);
	m_vDrift.v.coord.y = nSpeed - RandN (2 * nSpeed);
	m_vDrift.v.coord.z = nSpeed - RandN (2 * nSpeed);
	m_vDir.SetZero ();
	m_bHaveDir = 1;
	}
else {
	m_vDir = *vDir;

	if (m_nType == RAIN_PARTICLES)
		m_vDrift = m_vDir;
	else {
#if 1
		m_vDrift.v.coord.x = RandN (I2X (1) / 4) - I2X (1) / 8;
		m_vDrift.v.coord.y = RandN (I2X (1) / 4) - I2X (1) / 8;
		m_vDrift.v.coord.z = RandN (I2X (1) / 4) - I2X (1) / 8;
		m_vDrift += m_vDir;
#else
		CAngleVector a;
		CFixMatrix m;
		a.v.coord.p = RandN (I2X (1) / 4) - I2X (1) / 8;
		a.v.coord.b = RandN (I2X (1) / 4) - I2X (1) / 8;
		a.v.coord.h = RandN (I2X (1) / 4) - I2X (1) / 8;
		m = CFixMatrix::Create (a);
		if (m_nType == WATERFALL_PARTICLES)
			CFixVector::Normalize (m_vDir);
		m_vDrift = m * m_vDir;
#endif
		CFixVector::Normalize (m_vDrift);
		if (m_nType == WATERFALL_PARTICLES) {
			fix dot = CFixVector::Dot (m_vDir, m_vDrift);
			if (dot < I2X (1) / 2)
				return 0;
			}
		float d = float (CFixVector::DeltaAngle (m_vDrift, m_vDir, NULL));
		if (d) {
			d = (float) exp ((I2X (1) / 8) / d);
			nSpeed = (fix) ((float) nSpeed / d);
			}
		}
	m_vDrift *= nSpeed;
	if (m_nType <= FIRE_PARTICLES)
		m_vDir *= (I2X (3) / 4 + I2X (RandN (16)) / 64);
	m_bHaveDir = 1;
	}

// init position

if (vEmittingFace)
	m_vPos = *RandomPointOnQuad (vEmittingFace, vPos);
else if ((m_nType != BUBBLE_PARTICLES) && (m_nType != RAIN_PARTICLES) && (m_nType != SNOW_PARTICLES))
	m_vPos = *vPos + m_vDrift * (I2X (1) / 64);
else if (!bPointSource) {
	//m_vPos = *vPos + vDrift * (I2X (1) / 32);
	int32_t nSpeed = m_vDrift.Mag () / 16;
	CFixVector v = CFixVector::Avg ((*mOrient).m.dir.r * (nSpeed - RandN (2 * nSpeed)), (*mOrient).m.dir.u * (nSpeed - RandN (2 * nSpeed)));
	m_vPos = *vPos + v + (*mOrient).m.dir.f * (I2X (1) / 2 - RandN (I2X (1)));
	}
m_vStartPos = m_vPos;

// init size
if (nScale < 0)
	m_nRad = float (-nScale);
else if (gameOpts->render.particles.bSyncSizes)
	m_nRad = float (PARTICLE_SIZE (gameOpts->render.particles.nSize [0], nScale, m_bBlowUp));
else
	m_nRad = float (nScale);
if (!m_nRad)
	m_nRad = 1.0f;

if ((m_nType == BUBBLE_PARTICLES) || (m_nType == SNOW_PARTICLES))
	m_nRad = m_nRad / 20 + float (RandN (int32_t (9 * m_nRad / 20)));
else {
	if (m_nType <= SMOKE_PARTICLES) {
		if (m_bBlowUp)
			m_nLife = 2 * m_nLife / 3;
		m_nLife = 4 * m_nLife / 5 + RandN (2 * m_nLife / 5);
		m_nRad += float (RandN (int32_t (m_nRad)));
		}
	else if (m_nType == FIRE_PARTICLES) {
		m_nLife = 3 * m_nLife / 4 + RandN (m_nLife / 4);
		m_nRad += float (RandN (int32_t (m_nRad)));
		}
	else
		m_nRad *= 2;
	if (mOrient) {
		static CFixMatrix mRot [9 * 9];
		static char bInit = 1;

		if (bInit) {
			bInit = 0;
			for (int32_t p = 0; p < 9; p++) { 
				for (int32_t h = 0; h < 9; h++) {
					CAngleVector vRot;
					vRot.v.coord.b = 0;
					vRot.v.coord.p = 2048 - (p * 512);
					vRot.v.coord.h = 2048 - (h * 512);
					mRot [p * 9 + h] = CFixMatrix::Create (vRot);
					}
				}
			}
		m_mOrient = *mOrient * mRot [Rand (9) * 9 + Rand (9)];
		}
	}

if (m_bBlowUp) {
	m_nWidth = (m_nType == WATERFALL_PARTICLES) 
				  ? m_nRad * 0.6666667f
				  : m_nRad;
	m_nHeight = m_nRad;
	}
else {
	m_nWidth = (m_nType == WATERFALL_PARTICLES) 
				  ? m_nRad * 0.3333333f
			     : m_nRad /** 2*/;
	m_nHeight = m_nRad /** 2*/;
	}
m_nWidth /= 65536.0f;
m_nHeight /= 65536.0f;
m_nRad /= 65536.0f;

// init animation
m_nFrames = ParticleImageInfo (m_nType).nFrames;
m_deltaUV = 1.0f / float (m_nFrames);
if (m_nType == BULLET_PARTICLES) {
	m_iFrame = 0;
	m_nRotFrame = 0;
	m_nOrient = 3;
	}
else if ((m_nType == RAIN_PARTICLES) || (m_nType == SNOW_PARTICLES)) {
	m_iFrame = 0;
	m_nRotFrame = 0;
	m_nOrient = 1;
	}
else if (m_nType == BUBBLE_PARTICLES) {
	m_iFrame = Rand (m_nFrames * m_nFrames);
	m_nRotFrame = 0;
	m_nOrient = 0;
	}
else if ((m_nType == LIGHT_PARTICLES) || (m_nType == GATLING_PARTICLES)) {
	m_iFrame = 0;
	m_nRotFrame = 0;
	m_nOrient = 0;
	}
else if (m_nType == FIRE_PARTICLES) {
	m_iFrame = (Rand (10) < 6) ? 0 : 2; // more fire than smoke (60:40)
	if (m_iFrame < 2)
		m_nLife = 9 * m_nLife / 10;
	else
		m_nLife = 10 * m_nLife / 9;
	m_nRotFrame = Rand (PARTICLE_POSITIONS);
	m_nOrient = Rand (4);
	}
else {
	m_iFrame = Rand (m_nFrames * m_nFrames);
	m_nRotFrame = Rand (PARTICLE_POSITIONS);
	m_nOrient = Rand (4);
	}
m_bAnimate = (m_nType != FIRE_PARTICLES) && (gameOpts->render.particles.nQuality > 1) && (m_nFrames > 1);
m_bRotate = ((m_nRenderType <= SMOKE_PARTICLES) || (m_nRenderType == SNOW_PARTICLES)) ? 1 : (m_nRenderType == FIRE_PARTICLES + PARTICLE_TYPES) ? -1 : 0;

#endif

m_nTTL = m_nLife; // may have been changed during initialization

UpdateDecay ();
UpdateTexCoord ();
SetupColor (fBrightness);
m_nDelayPosUpdate = -2;
return 1;
}

//------------------------------------------------------------------------------

bool CParticle::IsVisible (int32_t nThread) 
{
#if 0
	return gameData.renderData.mine.Visible (m_nSegment);
#else
if ((m_nSegment < 0) || (m_nSegment >= gameData.segData.nSegments))
	return false;
if (gameData.renderData.mine.Visible (m_nSegment))
	return true;
int16_t* pChild = SEGMENT (m_nSegment)->m_children;
for (int32_t i = 6; i; i--, pChild++) {
	if ((*pChild >= 0) && (gameData.renderData.mine.Visible (*pChild)))
		return true;
	}
#if 1
int32_t nSegment = FindSegByPosExhaustive (m_vPos, m_bSkybox, m_nSegment);
#else
int32_t nSegment = FindSegByPos (m_vPos, m_nSegment, 0, 0, 0, nThread);
#endif
if (nSegment < 0)
	return false;
m_nSegment = nSegment;
return gameData.renderData.mine.Visible (nSegment);
#endif
}

//------------------------------------------------------------------------------

inline int32_t CParticle::ChangeDir (int32_t d) 
{
	int32_t h = d;

if (h)
	h = h / 2 - RandN (h);
return (d * 10 + h) / 10;
}

//------------------------------------------------------------------------------

int32_t nPartSeg [MAX_THREADS] = { -1, -1, -1, -1 }; //, -1, -1, -1, -1};

static int32_t nFaceCount [MAX_THREADS][6];
static int32_t bSidePokesOut [MAX_THREADS][6];
//static int32_t nVert [6];
static CFixVector* wallNorm [MAX_THREADS];

int32_t CParticle::CollideWithWall (int32_t nThread) 
{
if (m_nSegment < 0)
	return 0;

wallNorm [nThread] = NULL;
CSegment* pSeg = SEGMENT (m_nSegment);
int32_t bInit = (m_nSegment != nPartSeg [nThread]);
if (bInit)
	nPartSeg [nThread] = m_nSegment;

CSide* pSide = pSeg->m_sides;
for (int32_t nSide = 0; nSide < SEGMENT_SIDE_COUNT; nSide++, pSide++) {
	if (pSide->m_nShape > SIDE_SHAPE_TRIANGLE)
		continue;
	if (bInit) {
		bSidePokesOut [nThread][nSide] = !pSide->IsPlanar ();
		nFaceCount [nThread][nSide] = pSide->m_nFaces;
		}
	int32_t nFaces = nFaceCount [nThread][nSide];
	int32_t nInFront = 0;
	for (int32_t nFace = 0; nFace < nFaces; nFace++) {
		fix nDist = m_vPos.DistToPlane (pSide->m_normals [nFace], gameData.segData.vertices [pSide->m_nMinVertex [0]]);
		if (nDist > -PLANE_DIST_TOLERANCE)
			nInFront++;
		else
			wallNorm [nThread] = pSide->m_normals + nFace;
		}
	if ((nInFront == 0) || ((nInFront == 1) && (nFaces == 2) && bSidePokesOut [nThread][nSide])) {
		int32_t nChild = pSeg->m_children [nSide];
		if (0 > nChild) {
#if DBG
			if (!wallNorm [nThread])
				BRP;
#endif
			return 1;
			}
		m_nSegment = nChild;
		break;
		}
	}
return 0;
}

//------------------------------------------------------------------------------

void CParticle::UpdateTexCoord (void) 
{
if ((m_nType <= WATERFALL_PARTICLES) && ((m_nType != BUBBLE_PARTICLES) || gameOpts->render.particles.bWobbleBubbles)) {
	m_texCoord.v.u = float (m_iFrame % m_nFrames) * m_deltaUV;
	m_texCoord.v.v = float (m_iFrame / m_nFrames) * m_deltaUV;
	}
}

//------------------------------------------------------------------------------

void CParticle::UpdateColor (float fBrightness, int32_t nThread) 
{
if (m_nType <= SMOKE_PARTICLES) {
	if (m_nFadeTime > 0) {
#if 1
		if (m_nTTL - m_nLife < m_nFadeTime) {
			m_color [0].Red () *= 0.95f;
			m_color [0].Green () *= 0.8f;
			}
		else {
			m_color [0] = m_color [1];
			//m_nWidth *= 1.25;
			//m_nHeight *= 1.25;
			m_bEmissive = false;
			m_nFadeTime = -1;
			}
#else
		if (m_color [0].Green () < m_color [1].Green ()) {
#if SMOKE_SLOWMO
			m_color [0].Green () += 1.0f / 40.0f / (float) gameStates.gameplay.slowmo [0].fSpeed;
#else
			m_color [0].Green () += 1.0f / 40.0f;
#endif
			if (m_color [0].Green () > m_color [1].Green ()) {
				m_color [0].Green () = m_color [1].Green ();
				m_nFadeTime--;
				}
			}
		if (m_color [0].Blue () < m_color [1].Blue ()) {
#if SMOKE_SLOWMO
			m_color [0].Blue () += 1.0f / 20.0f / (float) gameStates.gameplay.slowmo [0].fSpeed;
#else
			m_color [0].Blue () += 1.0f / 20.0f;
#endif
			if (m_color [0].Blue () > m_color [1].Blue ()) {
				m_color [0].Blue () = m_color [1].Blue ();
				m_nFadeTime--;
				}
			}
#endif
		}
	else if (m_nFadeTime == 0) {
		m_color [0].Red () = m_color [1].Red () * RANDOM_FADE;
		m_color [0].Green () = m_color [1].Green () * RANDOM_FADE;
		m_color [0].Blue () = m_color [1].Blue () * RANDOM_FADE;
		m_nFadeTime = -1;
	}
#if SMOKE_LIGHTING //> 1
	if (gameOpts->render.particles.nQuality > 2) {
		if (0 <= (m_nSegment = FindSegByPos (m_vPos, m_nSegment, 0, 0, 0, nThread))) {
			CFaceColor* pColor = lightManager.AvgSgmColor (m_nSegment, NULL, nThread);
			m_color [0].Red () *= pColor->Red ();
			m_color [0].Green () *= pColor->Green ();
			m_color [0].Blue () *= pColor->Blue ();
			}
		}
#endif
	}
SetupColor (fBrightness);
}

//------------------------------------------------------------------------------

fix CParticle::Drag (void) 
{
if ((m_nType == BUBBLE_PARTICLES) || (m_nType == RAIN_PARTICLES) || (m_nType == SNOW_PARTICLES) || (m_nType == FIRE_PARTICLES))
	return I2X (1); // constant speed
if (m_nType == WATERFALL_PARTICLES) {
	float h = 4.0f - 3.0f * m_decay;
	h *= h;
	return F2X (h);
	} 
return (m_decay > 0.9f) ? F2X (sqrt ((1.0f - m_decay) / 0.1f)) : I2X (1); //F2X (m_decay); // decelerate
}

//------------------------------------------------------------------------------

int32_t CParticle::Bounce (int32_t nThread) 
{
if (!gameOpts->render.particles.bCollisions)
	return 1;
if (!CollideWithWall (nThread)) {
	m_nBounce = ((m_nType == BUBBLE_PARTICLES) || (m_nType == RAIN_PARTICLES) || (m_nType == SNOW_PARTICLES) || (m_nType == WATERFALL_PARTICLES)) ? 1 : 2;
	return 1;
	}
if (!--m_nBounce)
	return 0;
fix dot = CFixVector::Dot (m_vDrift, *wallNorm [nThread]);
if (dot < I2X (1) / 100) {
	m_nLife = -1;
	return 0;
	}
m_vDrift += m_vDrift + *wallNorm [nThread] * (-2 * dot);
return 1;
}

//------------------------------------------------------------------------------

int32_t CParticle::UpdateDrift (int32_t nCurTime, int32_t nThread) 
{
fix t = nCurTime - m_nUpdated;
if (m_decay > 0.9f) 
	t = fix (t * sqrt ((1.0f - m_decay) / 0.1f));
m_nUpdated = nCurTime;
m_vPos += m_vDrift * t; // (I2X (t) / 1000);

#if 0 //DBG
CFixVector vDrift = m_vDrift;
CFixVector::Normalize (vDrift);
if (CFixVector::Dot (vDrift, m_vDir) < 0)
	t = t;
#endif

if ((m_nType <= SMOKE_PARTICLES) || (m_nType == FIRE_PARTICLES)) {
	m_vDrift.v.coord.x = ChangeDir (m_vDrift.v.coord.x);
	m_vDrift.v.coord.y = ChangeDir (m_vDrift.v.coord.y);
	m_vDrift.v.coord.z = ChangeDir (m_vDrift.v.coord.z);
	}

if (m_bHaveDir) {
	CFixVector vi = m_vDrift, vj = m_vDir;
	CFixVector::Normalize (vi);
	CFixVector::Normalize (vj);
	fix drag = Drag ();
	if (CFixVector::Dot (vi, vj) < 0)
		drag = -drag;
	m_vPos += m_vDir * drag;
	}

if (nCurTime - m_nMoved < 250) 
	return 1;
m_nMoved = nCurTime;

int32_t nSegment = m_nSegment;

if (m_nSegment < -1)
	m_nSegment++;
if (m_nSegment >= -1) {
#if 1 
	nSegment = FindSegByPosExhaustive (m_vPos, m_bSkybox, m_nSegment);
#else
	nSegment = FindSegByPos (m_vPos, m_nSegment, 1, -1, ((m_nType == BUBBLE_PARTICLES) || (m_nType == RAIN_PARTICLES) || (m_nType == SNOW_PARTICLES)) ? 0 : fix (m_nRad), nThread);
#endif
	if (nSegment < 0)
		m_nSegment = int32_t (--m_nDelayPosUpdate);
	}

if (nSegment < 0) {
	if (m_nType == WATERFALL_PARTICLES) {
		if (!m_bChecked) {
			CFixVector vDir = m_vPos - m_vStartPos;
			if ((CFixVector::Normalize (vDir) >= I2X (1)) && (CFixVector::Dot (vDir, m_vDir) < I2X (1) / 2)) {
				m_nLife = -1;
				return 0;
				}
			m_bChecked = 1;
			m_nLife = 500;
			}
		}
	if (m_nTTL - m_nLife > 500) {
		m_nLife = -1;
		return 0;
		}
	}
else if (m_nType == BUBBLE_PARTICLES) {
	if (!SEGMENT (nSegment)->HasWaterProp ()) {
		m_nLife = -1;
		return 0;
		}
	}
else if ((m_nType == RAIN_PARTICLES) || (m_nType == SNOW_PARTICLES)) {
	if (SEGMENT (nSegment)->HasWaterProp () || SEGMENT (nSegment)->HasLavaProp ()) {
		m_nLife = -1;
		return 0;
		}
#if 1
	if ((m_nSegment >= 0) && (nSegment != m_nSegment) && SEGMENT (m_nSegment)->HasFunction (SEGMENT_FUNC_SKYBOX)) {
		if (m_nTTL - m_nLife > 500) {
			m_nLife = -1;
			return 0;
			}
		nSegment = m_nSegment;
		}
#endif
	}
m_nSegment = nSegment;

if (!Bounce (nThread))
	return 0;

return 1;
}

//------------------------------------------------------------------------------

void CParticle::UpdateDecay (void)
{
if ((m_nType == BUBBLE_PARTICLES) || (m_nType == RAIN_PARTICLES) || (m_nType == SNOW_PARTICLES) || (m_nType == WATERFALL_PARTICLES))
	m_decay = 1.0f;
else if (m_nType == FIRE_PARTICLES) {
#if 1
	m_decay = float (sin (double (m_nLife) / double (m_nTTL) * PI));
#else
	m_decay = float (m_nLife) / float (m_nTTL);
	if (m_decay < 0.4)
	m_decay = float (sin (double (m_decay) * PI * 1.25));
	else if (m_decay > 0.15)
	m_decay = float (sin (double (1.0 - m_decay) * PI * 0.5 / 0.15));
	else
	m_decay = 1.0f;
#endif
	} 
else
	m_decay = float (m_nLife) / float (m_nTTL);
}

//------------------------------------------------------------------------------

int32_t CParticle::Update (int32_t nCurTime, float fBrightness, int32_t nThread) 
{
if ((m_nLife <= 0) /*|| (m_color [0].Alpha () < 0.01f)*/)
	return 0;

fix t = nCurTime - m_nUpdated;

#if !ENABLE_UPDATE 
m_nLife -= t;
m_decay = ((m_nType == BUBBLE_PARTICLES) || (m_nType == WATERFALL_PARTICLES)) ? 1.0f : float (m_nLife) / float (m_nTTL);
#else
UpdateColor (fBrightness, nThread);
if (m_nLife < 0)
	return 1;

if (m_nDelay > 0) {
	m_nUpdated = nCurTime;
	m_nDelay -= t;
	return 1;
	}

if (!UpdateDrift (nCurTime, nThread))
	return 0;
if (m_nLife < 0)
	return 1;

#if SMOKE_SLOWMO
m_nLife -= (int32_t) (t / gameStates.gameplay.slowmo [0].fSpeed);
#else
m_nLife -= t;
#	if 0
if ((m_nType == FIRE_PARTICLES) && !m_bReversed && (m_nLife <= m_nTTL / 4 + RandN (m_nTTL / 4))) {
	m_vDrift = -m_vDrift;
	m_bReversed = 1;
}
#	endif
#	if DBG
if ((m_nLife <= 0) && (m_nType == 2))
	m_nLife = -1;
#	endif
#endif
if (m_nLife < 0)
	m_nLife = 0;
UpdateDecay ();

if ((m_nType <= SMOKE_PARTICLES) && (m_nRad > 0)) {
	if (m_bBlowUp) {
		if (m_nWidth >= m_nRad)
			m_nRad = 0;
		else {
			m_nWidth += m_nRad * 0.1f;
			if (m_nWidth > m_nRad)
				m_nWidth = m_nRad;
			m_nHeight += m_nRad * 0.1f;
			if (m_nHeight > m_nRad)
				m_nHeight = m_nRad;
			m_color [0].Alpha () *= (1.0f + 0.0725f / m_bBlowUp);
			if (m_color [0].Alpha () > 1)
				m_color [0].Alpha () = 1;
			}
		} 
	else {
		if (m_nWidth <= m_nRad)
			m_nRad = 0;
		else {
			m_nRad *= 1.2f;
			m_color [0].Alpha () *= 1.0725f;
			if (m_color [0].Alpha () > 1)
				m_color [0].Alpha () = 1;
			}
		}
	}
#endif
return 1;
}

//------------------------------------------------------------------------------

int32_t CParticle::RenderType (void) 
{
#if 0
return m_nType;
#else
if ((m_nType != FIRE_PARTICLES) || /* (gameOpts->render.particles.nQuality < 2) ||*/ (m_iFrame < m_nFrames))
	return (m_nType == SNOW_PARTICLES) ? SIMPLE_SMOKE_PARTICLES : m_nType;
return PARTICLE_TYPES + m_nType;
#endif
}

//------------------------------------------------------------------------------

int32_t CParticle::Render (float fBrightness) 
{
if (m_nDelay > 0)
	return 0;
if (m_nLife < 0)
	return 0;
if ((m_nType < 0) || (m_nType >= PARTICLE_TYPES))
	return 0;
#if 0 //DBG
if (m_nType == BUBBLE_PARTICLES)
	return 0;
#endif

#if !ENABLE_RENDER
	return 1;
#else

PROF_START

#if LAZY_RENDER_SETUP
bool bFlushed = particleManager.Add (this, fBrightness);
#else
bool bFlushed = false;
Setup (fBrightness, m_iFrame, m_nRotFrame, particleRenderBuffer + particleManager.BufPtr () * 4, 0);
#endif

if (particleManager.Animate ()) {
	if (m_bAnimate && (m_nFrames > 1)) {
		m_iFrame = (m_iFrame + 1) % (m_nFrames * m_nFrames);
		UpdateTexCoord ();
		}
	if (m_bRotate) {
		if (m_bRotate < 0)
			m_nRotFrame = (m_nRotFrame + 1) % PARTICLE_POSITIONS;
		else {
			m_bRotate <<= 1;
			if (m_bRotate == 4) {
				m_bRotate = 1;
				m_nRotFrame = (m_nRotFrame + 1) % PARTICLE_POSITIONS;
				}
			}
		}
	}

PROF_END (ptParticles)
return bFlushed ? -1 : 1;
#endif
}

//------------------------------------------------------------------------------

int32_t CParticle::SetupColor (float fBrightness) 
{
if (m_bBright)
	fBrightness = float (sqrt (fBrightness));

m_renderColor = m_color [0];
if (m_nType <= SMOKE_PARTICLES) {
	fBrightness *= 0.9f + (float) Rand (1000) / 5000.0f;
	m_renderColor.Red () *= fBrightness;
	m_renderColor.Green () *= fBrightness;
	m_renderColor.Blue () *= fBrightness;
	}

float fFade;

if (m_nFadeType == 0) { // default (start fully visible, fade out)
#if 1 
	m_renderColor.Alpha () *= m_decay; // * 0.6f;
#else
	m_renderColor.Alpha () *= float (cos (double (sqr (1.0f - m_decay)) * PI) * 0.5 + 0.5) * 0.6f;
#endif
	}
else if (m_nFadeType == 1) { // quickly fade in, then gently fade out
	m_renderColor.Alpha () *= float (sin (double (sqr (1.0f - m_decay)) * PI * 1.5) * 0.5 + 0.5);
	if (m_decay >= 0.666f)
		return 1;
} else if (m_nFadeType == 2) { // fade in, then gently fade out
	float fPivot = m_nTTL / 4000.0f;
	if (fPivot > 0.25f)
		fPivot = 0.25f;
	float fLived = (m_nTTL - m_nLife) / 1000.0f;
	if (fLived < fPivot)
		fFade = fLived / fPivot;
	else
		fFade = 1.0f - (fLived - fPivot) / (1.0f - fPivot);
	if (fFade < 0.0f)
		fFade = 0.05f;
	else
		fFade = 0.1f + 0.9f * fFade;
	if (fFade > 1.0f)
		fFade = 1.0f;
	m_renderColor.Alpha () *= fFade;
	if (m_decay >= 0.75f)
		return 1;
	}
else if (m_nFadeType == 3) { // fire (additive, blend in)
#if 0
	if (m_decay > 0.5f)
	fFade = 2.0f * (1.0f - m_decay);
	else
	fFade = m_decay * 2.0f;
	if ((m_decay < 0.5f) && (fFade < 0.00333f)) {
		m_nLife = -1;
		return 0;
	}
	m_renderColor.Red () =
	m_renderColor.Green () =
	m_renderColor.Blue () = fFade;
	m_color [0] = m_renderColor;
#endif
	return 1;
	}
else if (m_nFadeType == 4) { // light trail (additive, constant effect)
	m_renderColor.Red () /= 50.0f;
	m_renderColor.Green () /= 50.0f;
	m_renderColor.Blue () /= 50.0f;
	return 1;
	}
if (m_renderColor.Alpha () >= 0.01f) //1.0 / 255.0
	return 1;
m_nLife = -1;
return 0;
}

//------------------------------------------------------------------------------

#if TRANSFORM_PARTICLE_VERTICES

void CParticle::Setup (bool alphaControl, float fBrightness, char nFrame, char nRotFrame, tParticleVertex* pb, int32_t nThread) 
{
	CFloatVector3 vCenter, vOffset;

transformation.Transform (m_vTransPos, m_vPos, gameStates.render.bPerPixelLighting == 2);
vCenter.Assign (m_vTransPos);

if ((m_nType <= SMOKE_PARTICLES) && m_bBlowUp) {
#if DBG
	float fFade;
	if (m_nFadeType == 3)
	fFade = 1.0;
	else if (m_decay > 0.9f)
	fFade = (1.0f - pow (m_decay, 44.0f)) / float (pow (m_decay, 0.25f));
	else
	fFade = 1.0f / float (pow (m_decay, 0.25f));
#else
	float fFade = (m_nFadeType == 3)
						? 1.0f
						: (m_decay > 0.9f) // start from zero size by scaling with pow (m_decay, 44f) which is < 0.01 for m_decay == 0.9f
							? (1.0f - pow (m_decay, 44.0f)) / float (pow (m_decay, 0.3333333f))
							: 1.0f / float (pow (m_decay, 0.3333333f));
#endif
	vOffset.v.coord.x = m_nWidth * fFade;
	vOffset.v.coord.y = m_nHeight * fFade;
	}
else {
	vOffset.v.coord.x = m_nWidth * m_decay;
	vOffset.v.coord.y = m_nHeight * m_decay;
	}
vOffset.v.coord.z = 0;

float h = ParticleImageInfo (m_nType).xBorder;
pb [m_nOrient].texCoord.v.u =
pb [(m_nOrient + 3) % 4].texCoord.v.u = m_texCoord.v.u + h;
pb [(m_nOrient + 1) % 4].texCoord.v.u =
pb [(m_nOrient + 2) % 4].texCoord.v.u = m_texCoord.v.u + m_deltaUV - h;
h = ParticleImageInfo (m_nType).yBorder;
pb [m_nOrient].texCoord.v.v =
pb [(m_nOrient + 1) % 4].texCoord.v.v = m_texCoord.v.v + h;
pb [(m_nOrient + 2) % 4].texCoord.v.v =
pb [(m_nOrient + 3) % 4].texCoord.v.v = m_texCoord.v.v + m_deltaUV - h;

pb [0].color =
pb [1].color =
pb [2].color =
pb [3].color = m_renderColor;

if ((m_nType == BUBBLE_PARTICLES) && gameOpts->render.particles.bWiggleBubbles)
vCenter.v.coord.x += (float) sin (nFrame / 4.0f * PI) / (10 + Rand (6));
if (m_bRotate && gameOpts->render.particles.bRotate) {
	int32_t i = (m_nOrient & 1) ? 63 - m_nRotFrame : m_nRotFrame;
	vOffset.v.coord.x *= vRot [i].v.coord.x;
	vOffset.v.coord.y *= vRot [i].v.coord.y;

	pb [0].vertex.v.coord.x = vCenter.v.coord.x - vOffset.v.coord.x;
	pb [0].vertex.v.coord.y = vCenter.v.coord.y + vOffset.v.coord.y;
	pb [1].vertex.v.coord.x = vCenter.v.coord.x + vOffset.v.coord.y;
	pb [1].vertex.v.coord.y = vCenter.v.coord.y + vOffset.v.coord.x;
	pb [2].vertex.v.coord.x = vCenter.v.coord.x + vOffset.v.coord.x;
	pb [2].vertex.v.coord.y = vCenter.v.coord.y - vOffset.v.coord.y;
	pb [3].vertex.v.coord.x = vCenter.v.coord.x - vOffset.v.coord.y;
	pb [3].vertex.v.coord.y = vCenter.v.coord.y - vOffset.v.coord.x;
	}
else {
	pb [0].vertex.v.coord.x =
	pb [3].vertex.v.coord.x = vCenter.v.coord.x - vOffset.v.coord.x;
	pb [1].vertex.v.coord.x =
	pb [2].vertex.v.coord.x = vCenter.v.coord.x + vOffset.v.coord.x;
	pb [0].vertex.v.coord.y =
	pb [1].vertex.v.coord.y = vCenter.v.coord.y + vOffset.v.coord.y;
	pb [2].vertex.v.coord.y =
	pb [3].vertex.v.coord.y = vCenter.v.coord.y - vOffset.v.coord.y;
	}
pb [0].vertex.v.coord.z =
pb [1].vertex.v.coord.z =
pb [2].vertex.v.coord.z =
pb [3].vertex.v.coord.z = vCenter.v.coord.z;
}

#else // -----------------------------------------------------------------------

void CParticle::Setup (bool alphaControl, float fBrightness, char nFrame, char nRotFrame, tParticleVertex* pb, int32_t nThread) 
{
	CFloatVector3 vCenter, uVec, rVec;
	float fScale;

#if 0
vCenter.Assign (gameData.objData.pConsole->Orientation ().m.dir.f); //m_vPos);
vCenter *= 10.0;
fVec.Assign (gameData.objData.pConsole->Position ()); 
vCenter += fVec;
#else
vCenter.Assign (m_vPos);
#endif

if (m_nType <= SMOKE_PARTICLES) {
#if 1 //DBG
	if (m_nFadeType == 3)
		fScale = 1.0f;
	else {
#if 1
		fScale = m_bBlowUp ? 1.0f / float (sqrt (m_decay)) : 1.0f;
#else
		fScale = m_bBlowUp ? 1.0f / float (pow (m_decay, 1.0f / 3.0f)) : 1.0f;
#endif
		if ((m_bBlowUp >= 0) && (m_decay > 0.9f))
			fScale *= sqrt ((1.0f - m_decay) / 0.1f);
		}
#else
	fScale = (m_nFadeType == 3)
				? 1.0f
				: (m_decay > 0.9f) // start from zero size by scaling with pow (m_decay, 44f) which is < 0.01 for m_decay == 0.9f
					? (1.0f - pow (m_decay, 44.0f)) / float (pow (m_decay, 0.3333333f)) 0.1 - 1.0 + m_decay
					: 1.0f / float (pow (m_decay, 0.3333333f));
#endif
	}
else {
	fScale = m_decay;
	}

pb [0].color = m_renderColor;
if (m_bEmissive && alphaControl)
	pb [0].color.Alpha () = 0.0f;
pb [1].color = pb [2].color = pb [3].color = pb [0].color;

if (m_bEmissive < 0) {
	m_bEmissive = 1;
	m_nType = SPARK_PARTICLES;

	float nRow = m_texCoord.v.u;
	float nCol = m_texCoord.v.v;

	pb [0].texCoord.v.u = 
	pb [1].texCoord.v.u = nCol + 1.0f / 64.0f;
	pb [1].texCoord.v.v = 
	pb [2].texCoord.v.v = nRow;
	pb [2].texCoord.v.u = 
	pb [3].texCoord.v.u = nCol + 7.0f / 64.0f;
	pb [0].texCoord.v.v = 
	pb [3].texCoord.v.v = nRow + 1 / 8.0f;
	pb [0].texCoord.v.l = 
	pb [1].texCoord.v.l = 
	pb [2].texCoord.v.l = 
	pb [3].texCoord.v.l = 0.0f;
	}
else {
	float hx = ParticleImageInfo (m_nType).xBorder;
	pb [(int32_t) m_nOrient].texCoord.v.u = pb [int32_t (m_nOrient + 3) % 4].texCoord.v.u = m_texCoord.v.u + hx;
	pb [int32_t (m_nOrient + 1) % 4].texCoord.v.u = pb [(m_nOrient + 2) % 4].texCoord.v.u = m_texCoord.v.u + m_deltaUV - hx;
	float hy = ParticleImageInfo (m_nType).yBorder;
	pb [(int32_t) m_nOrient].texCoord.v.v = pb [int32_t (m_nOrient + 1) % 4].texCoord.v.v = m_texCoord.v.v + hy;
	pb [int32_t (m_nOrient + 2) % 4].texCoord.v.v = pb [(m_nOrient + 3) % 4].texCoord.v.v = m_texCoord.v.v + m_deltaUV - hy;
	pb [0].texCoord.v.l = pb [1].texCoord.v.l = pb [2].texCoord.v.l = pb [3].texCoord.v.l = (((m_nType == SMOKE_PARTICLES) || (m_nType == WATERFALL_PARTICLES)) ? 1.0f : 2.0f);
	}

if (m_nType == SPARK_PARTICLES) {
	uVec.Assign (mSparkOrient.m.dir.u);
	rVec.Assign (mSparkOrient.m.dir.r);
	}
else if (m_nType == RAIN_PARTICLES) {
	uVec.Assign (m_vDir);
	CFloatVector3::Normalize (uVec);
	uVec.Neg ();
	CFloatVector3 v;
	v.Assign (gameData.renderData.mine.viewer.vPos);
	CFloatVector3 u = uVec - v;
	CFloatVector3 c = vCenter - v;
	CFloatVector3::Normalize (u);
	CFloatVector3::Normalize (c);
	rVec = CFloatVector3::Cross (u, v);
	}
else {
	if ((m_nType == SNOW_PARTICLES) || ((m_nType == BUBBLE_PARTICLES) && gameOpts->render.particles.bWiggleBubbles))
		vCenter.v.coord.x += (float) sin (nFrame / 4.0f * PI) / (10 + Rand (6));
	if (m_bRotate && gameOpts->render.particles.bRotate) {
		CFixMatrix& mOrient = mRot [1][(m_nOrient & 1) ? 63 - m_nRotFrame : m_nRotFrame];
		uVec.Assign (mOrient.m.dir.u);
		rVec.Assign (mOrient.m.dir.r);
		}
	else {
		uVec.Assign (gameData.renderData.mine.viewer.mOrient.m.dir.u);
		rVec.Assign (gameData.renderData.mine.viewer.mOrient.m.dir.r);
		}
	}
uVec *= m_nHeight * fScale;
rVec *= m_nWidth * fScale;
pb [0].vertex = pb [1].vertex = vCenter - rVec;
pb [0].vertex -= uVec;
pb [1].vertex += uVec;
pb [2].vertex = pb [3].vertex = vCenter + rVec;
pb [2].vertex += uVec;
pb [3].vertex -= uVec;
}

#endif

//------------------------------------------------------------------------------
//eof
