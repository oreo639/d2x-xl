#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <string.h>	// for memset
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "descent.h"
#include "error.h"
#include "timer.h"
#include "u_mem.h"
#include "interp.h"
#include "lightning.h"
#include "network.h"
#include "rendermine.h"
#include "objeffects.h"
#include "objrender.h"
#include "objsmoke.h"
#include "automap.h"
#include "shrapnel.h"
#include "addon_bitmaps.h"

CShrapnelManager shrapnelManager;

// -----------------------------------------------------------------------------

#define SHRAPNEL_MAX_PARTS			225
#define SHRAPNEL_PART_LIFE			-1500
#define SHRAPNEL_PART_SPEED		10

static float fShrapnelScale [5] = {0, 5.0f, 10.0f, 15.0f, 20.0f};

void CShrapnel::Create (CObject* parentObjP, CObject* objP, float fScale)
{
	static CFloatVector color = {{{0.95f, 0.95f, 0.95f, 2.0f}}};

m_info.vDir = CFixVector::Random ();
m_info.vPos = objP->info.position.vPos + m_info.vDir * (parentObjP->info.xSize / 4 + Rand (parentObjP->info.xSize / 2));
m_info.nTurn = 1;
m_info.xSpeed = 3 * (I2X (1) / 20 + Rand (I2X (1) / 20)) / 4;
m_info.xLife =
m_info.xTTL = I2X (3) / 2 + rand ();
m_info.tUpdate = gameStates.app.nSDLTicks [0];
m_info.nSmoke = 
	particleManager.Create (&m_info.vPos, NULL, NULL, objP->info.nSegment, 1, -SHRAPNEL_MAX_PARTS * Max (gameOpts->render.particles.nQuality, 2),
								   -PARTICLE_SIZE (1, fScale, 1), 
									/*-1, 1,*/ 
									SHRAPNEL_PART_LIFE, SHRAPNEL_PART_SPEED, SIMPLE_SMOKE_PARTICLES, 0x7fffffff, &color, 1, -1);
if (objP->info.xLifeLeft < m_info.xLife)
	objP->SetLife (m_info.xLife);
}

// -----------------------------------------------------------------------------

void CShrapnel::Destroy (void)
{
if (m_info.nSmoke >= 0) {
	particleManager.SetLife (m_info.nSmoke, 0);
	m_info.nSmoke = 0;
	}
m_info.xTTL = -1;
}

// -----------------------------------------------------------------------------

void CShrapnel::Move (void)
{
	fix			xSpeed = FixDiv (m_info.xSpeed, I2X (25) / 1000);
	CFixVector	vOffs;
	time_t		nTicks;

if ((nTicks = gameStates.app.nSDLTicks [0] - m_info.tUpdate) < 25)
	return;
xSpeed = (fix) (xSpeed / gameStates.gameplay.slowmo [0].fSpeed);
for (; nTicks >= 25; nTicks -= 25) {
	if (--(m_info.nTurn))
		vOffs = m_info.vOffs;
	else {
		m_info.nTurn = ((m_info.xTTL > I2X (1) / 2) ? 2 : 4) + Rand (4);
		vOffs = m_info.vDir;
		vOffs.v.coord.x = FixMul (vOffs.v.coord.x, 2 * RandShort ());
		vOffs.v.coord.y = FixMul (vOffs.v.coord.y, 2 * RandShort ());
		vOffs.v.coord.z = FixMul (vOffs.v.coord.z, 2 * RandShort ());
		CFixVector::Normalize (vOffs);
		m_info.vOffs = vOffs;
		}
	vOffs *= xSpeed;
	m_info.vPos += vOffs;
	}
if (m_info.nSmoke >= 0)
	particleManager.SetPos (m_info.nSmoke, &m_info.vPos, NULL, -1);
m_info.tUpdate = gameStates.app.nSDLTicks [0] - nTicks;
}

// -----------------------------------------------------------------------------

void CShrapnel::Draw (void)
{
if (m_info.xTTL > 0) {
	explBlast.Bitmap ()->SetColor ();
#if 0
	fix xSize = I2X (1) / 2 + Rand (I2X (1) / 4);
	ogl.RenderSprite (explBlast.Bitmap (), m_info.vPos, xSize, xSize, X2F (m_info.xTTL) / X2F (m_info.xLife) / 2, 0, 0);
#endif
	}
}

// -----------------------------------------------------------------------------

int32_t CShrapnel::Update (void)
{
if (m_info.xTTL <= 0)
	return -1;	//dead
Move ();
if (0 < (m_info.xTTL -= (fix) (MSEC2X (gameStates.app.tick40fps.nTime) / gameStates.gameplay.slowmo [0].fSpeed)))
	return 1;	//alive
Destroy ();
return 0; //kill
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

uint32_t CShrapnelCloud::Update (void)
{
	int32_t i;

#if USE_OPENMP //> 1
#	pragma omp parallel for 
#endif
	for (i = 0; i < int32_t (m_tos); i++)
		m_data.buffer [i].Update ();

for (i = int32_t (m_tos) - 1; i >= 0; i--)
	if (m_data.buffer [i].TTL () < 0)
		Delete (i);

if (m_tos)
	return m_tos;
Destroy ();
return 0;
}

// -----------------------------------------------------------------------------

void CShrapnelCloud::Draw (void)
{
if (explBlast.Load ())
#if USE_OPENMP //> 1
#	pragma omp parallel for
#endif
	for (int32_t i = 0; i < int32_t (m_tos); i++)
		m_data.buffer [i].Draw ();
}

// -----------------------------------------------------------------------------

int32_t CShrapnelCloud::Create (CObject* parentObjP, CObject* objP)
{
	float		fScale = float (sqrt (X2F (parentObjP->info.xSize))) * 1.25f;
	int32_t	i, 
				h = (int32_t) FRound (fScale * fShrapnelScale [gameOpts->render.effects.nShrapnels]);

objP->UpdateLife (0);
objP->cType.explInfo.nSpawnTime = -1;
objP->cType.explInfo.nDeleteObj = -1;
objP->cType.explInfo.nDeleteTime = -1;
#if DBG
h += h / 2;
#else
h = 5 * h / 4 + Rand (h / 2);
#endif
if (!CStack<CShrapnel>::Create (h))
	return 0;
if (!Grow (h))
	return 0;
fScale = 7.0f / fScale;
#if USE_OPENMP //> 1
#	pragma omp parallel for 
#endif
for (i = 0; i < h; i++) 
	m_data.buffer [i].Create (parentObjP, objP, 3.0f);
objP->SetLife (objP->LifeLeft  () * 2);
objP->cType.explInfo.nSpawnTime = -1;
objP->cType.explInfo.nDeleteObj = -1;
objP->cType.explInfo.nDeleteTime = -1;
return 1;
}

// -----------------------------------------------------------------------------

void CShrapnelCloud::Destroy (void)
{
#if USE_OPENMP //> 1
#	pragma omp parallel for 
#endif
for (int32_t i = 0; i < int32_t (m_tos); i++)
	m_data.buffer [i].Destroy ();
CStack<CShrapnel>::Destroy ();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

bool CShrapnelManager::Init (void)
{
return CArray<CShrapnelCloud>::Create (LEVEL_OBJECTS) != NULL;
}

// -----------------------------------------------------------------------------

void CShrapnelManager::Reset (void)
{
CArray<CShrapnelCloud>::Destroy ();
}

// -----------------------------------------------------------------------------

int32_t CShrapnelManager::Create (CObject* objP)
{
#if 1
return 0;
#else
if (!SHOW_SMOKE)
	return 0;
if (!gameOpts->render.effects.nShrapnels)
	return 0;
if (objP->info.nFlags & OF_ARMAGEDDON)
	return 0;
if ((objP->info.nType != OBJ_PLAYER) && (objP->info.nType != OBJ_ROBOT) && (objP->info.nType != OBJ_DEBRIS))
	return 0;
int16_t nObject = CreateFireball (0, objP->info.nSegment, objP->info.position.vPos, 1, RT_SHRAPNELS);
if (0 > nObject)
	return 0;
return m_data.buffer [nObject].Create (objP, OBJECTS + nObject);
#endif
}

// -----------------------------------------------------------------------------

void CShrapnelManager::Draw (CObject* objP)
{
m_data.buffer [objP->Index ()].Draw ();
}

// -----------------------------------------------------------------------------

int32_t CShrapnelManager::Update (CObject* objP)
{
if ((objP->LifeLeft () <= 0) || !m_data.buffer [objP->Index ()].Update ())
	Destroy (objP);
return 0;
}

// -----------------------------------------------------------------------------

void CShrapnelManager::Destroy (CObject* objP)
{
if ((objP->info.nType != OBJ_FIREBALL) || (objP->info.renderType != RT_SHRAPNELS))
	return;
m_data.buffer [objP->Index ()].Destroy ();
objP->SetLifeLeft (-1);
}

//------------------------------------------------------------------------------

void CShrapnelManager::DoFrame (void)
{
	CObject	*objP;
	int32_t		i;

if (!SHOW_SMOKE)
	return;
FORALL_STATIC_OBJS (objP)
	if (objP->info.renderType == RT_SHRAPNELS)
		Update (objP);
FORALL_ACTOR_OBJS (objP) {
	i = objP->Index ();
	if (gameData.objs.bWantEffect [i] & SHRAPNEL_SMOKE) {
		gameData.objs.bWantEffect [i] &= ~SHRAPNEL_SMOKE;
		Create (objP);
		}
	}
}

//------------------------------------------------------------------------------
