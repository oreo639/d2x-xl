#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <math.h>
#include <stdlib.h>
#include "error.h"

#include "descent.h"
#include "interp.h"
#include "gr.h"
#include "u_mem.h"
#include "hiresmodels.h"
#include "buildmodel.h"
#include "rendermodel.h"

using namespace RenderModel;

//------------------------------------------------------------------------------

void CModel::CountASEModelItems (ASE::CModel *pa)
{
m_nFaces = pa->m_nFaces;
m_nSubModels = pa->m_nSubModels;
m_nVerts = pa->m_nVerts;
m_nFaceVerts = pa->m_nFaces * 3;
}

//------------------------------------------------------------------------------

void CModel::GetASEModelItems (int32_t nModel, ASE::CModel *pa, float fScale)
{
	ASE::CSubModel*	psa;
	ASE::CFace*			pfa;
	CSubModel*			psm;
	CFace*				pmf = m_faces.Buffer ();
	CVertex*				pmv = m_faceVerts.Buffer ();
	CBitmap*				pBm;
	int32_t				h, i, nFaces, iFace, nVerts = 0, nIndex = 0;
	int32_t				bTextured;

for (psa = pa->m_subModels; psa; psa = psa->m_next) {
	psm = m_subModels + psa->m_nSubModel;
#if DBG
	strcpy (psm->m_szName, psa->m_szName);
#endif
	psm->m_nSubModel = psa->m_nSubModel;
	psm->m_nParent = psa->m_nParent;
	psm->m_faces = pmf;
	psm->m_nFaces = nFaces = psa->m_nFaces;
	psm->m_bGlow = psa->m_bGlow;
	psm->m_bFlare = psa->m_bFlare;
	psm->m_bBillboard = psa->m_bBillboard;
	psm->m_bRender = psa->m_bRender;
	psm->m_bThruster = psa->m_bThruster;
	psm->m_bWeapon = psa->m_bWeapon;
	psm->m_bHeadlight = psa->m_bHeadlight;
	psm->m_bBombMount = psa->m_bBombMount;
	psm->m_nGun = psa->m_nGun;
	psm->m_nBomb = psa->m_nBomb;
	psm->m_nMissile = psa->m_nMissile;
	psm->m_nType = psa->m_nType;
	psm->m_nWeaponPos = psa->m_nWeaponPos;
	psm->m_nGunPoint = psa->m_nGunPoint;
	psm->m_bBullets = (psa->m_nBullets > 0);
	psm->m_nVertexIndex [0] = nIndex;
	psm->m_nVertices = psa->m_nVerts;
	psm->m_iFrame = 0;
	psm->m_tFrame = 0;
	psm->m_nFrames = (psa->m_bBarrel || psa->m_bThruster) ? 32 : 0;
	psm->m_vOffset.Assign (psa->m_vOffset);
	psm->InitMinMax ();
	for (pfa = psa->m_faces.Buffer (), iFace = 0; iFace < nFaces; iFace++, pfa++, pmf++) {
		pmf->m_nIndex = nIndex;
#if 1
		i = psa->m_nBitmap;
#else
		i = pfa->m_nBitmap;
#endif
		pBm = pa->m_textures.m_bitmaps + i;
		bTextured = !pBm->Flat ();
		pmf->m_nBitmap = bTextured ? i : -1;
		pmf->m_nVerts = 3;
		pmf->m_nId = iFace;
		for (i = 0; i < 3; i++, pmv++) {
			h = pfa->m_nVerts [i];
			if ((pmv->m_bTextured = bTextured))
				pmv->m_baseColor.Red () =
				pmv->m_baseColor.Green () =
				pmv->m_baseColor.Blue () = 1;
			else 
				pBm->GetAvgColor (&pmv->m_baseColor);
			pmv->m_baseColor.Alpha () = 1;
			pmv->m_renderColor = pmv->m_baseColor;
			pmv->m_normal = psa->m_vertices [h].m_normal;
			pmv->m_vertex = psa->m_vertices [h].m_vertex * fScale;
			if (psa->m_texCoord.Buffer ())
				pmv->m_texCoord = psa->m_texCoord [pfa->m_nTexCoord [i]];
			h += nVerts;
			m_vertices [h] = pmv->m_vertex;
			m_vertexOwner [h].m_nOwner = (uint16_t) psa->m_nSubModel;
			m_vertexOwner [h].m_nVertex = (uint16_t) h;
			m_vertNorms [h] = pmv->m_normal;
			pmv->m_nIndex = h;
			psm->SetMinMax (&pmv->m_vertex);
			nIndex++;
			}
#if 1
		if (psm->m_bThruster) {
			CFloatVector3 n = CFloatVector3::Normal ((pmv - 3)->m_vertex, (pmv - 2)->m_vertex, (pmv - 1)->m_vertex);
			pmf->m_vNormal.Assign (n);
			}
		else
#endif
			pmf->m_vNormal.Assign (pfa->m_vNormal);
		}
	nVerts += psa->m_nVerts;
	}
}

//------------------------------------------------------------------------------

int32_t CModel::BuildFromASE (CObject *pObj, int32_t nModel)
{
	ASE::CModel*	pa = gameData.modelData.modelToASE [1][nModel];
	int32_t			i, j;

if (!pa) {
	pa = gameData.modelData.modelToASE [0][nModel];
	if (!pa)
		return 0;
	}
#if DBG
HUDMessage (0, "optimizing model");
#endif
if (gameStates.app.nLogLevel > 1)
	PrintLog (1, "optimizing ASE model %d\n", nModel);
CountASEModelItems (pa);
if (!Create ()) {
	if (gameStates.app.nLogLevel > 1)
		PrintLog (-1);
	return 0;
	}
GetASEModelItems (nModel, pa, 1.0f); //(nModel == 108) || (nModel == 110)) ? 1.145f : 1.0f);
m_nModel = nModel;
m_textures.SetName ("ASE::CModel::m_textures");
m_textures = pa->m_textures.m_bitmaps;
m_nTextures = pa->m_textures.m_nBitmaps;
for (i = 0; i < m_nTextures; i++) {
	sprintf (m_textures [i].Name (), "ASE model %d texture %d", nModel, i);
	pa->m_textures.m_bitmaps [i].ShareBuffer (m_textures [i]);
	}
memset (m_teamTextures, 0xFF, sizeof (m_teamTextures));
for (i = 0; i < m_nTextures; i++)
	if ((j = (int32_t) m_textures [i].Team ()))
		m_teamTextures [j - 1] = i;
m_nType = 2;
gameData.modelData.polyModels [0][nModel].SetRad (Size (pObj, 1), 1);
Setup (1, 1);
#if 1
SetGunPoints (pObj, 1);
#endif
if (gameStates.app.nLogLevel > 1)
	PrintLog (-1);
return -1;
}

//------------------------------------------------------------------------------
//eof
