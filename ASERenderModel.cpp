#include <math.h>
#include <stdlib.h>

#include "mine.h"
#include "dle-xp.h"
#include "rendermodel.h"
#include "buildmodel.h"

using namespace RenderModel;

//------------------------------------------------------------------------------

void CModel::CountASEModelItems (ASE::CModel& aseModel)
{
m_nFaces = aseModel.m_nFaces;
m_nSubModels = aseModel.m_nSubModels;
m_nVerts = aseModel.m_nVerts;
m_nFaceVerts = aseModel.m_nFaces * 3;
}

//------------------------------------------------------------------------------

void CModel::GetASEModelItems (int nModel, ASE::CModel& aseModel, float fScale)
{
	ASE::CSubModel*	aseSubModelP;
	ASE::CFace*			aseFaceP;
	CSubModel*			renderSubModelP;
	CFace*				renderFaceP = m_faces.Buffer ();
	CVertex*				renderVertexP = m_faceVerts.Buffer ();
	int					h, i, nFaces, iFace, nVerts = 0, nIndex = 0;
	bool					bTextured;

for (aseSubModelP = aseModel.m_subModels; aseSubModelP; aseSubModelP = aseSubModelP->m_next) {
	renderSubModelP = m_subModels + aseSubModelP->m_nSubModel;
#if DBG
	strcpy (renderSubModelP->m_szName, aseSubModelP->m_szName);
#endif
	renderSubModelP->m_nSubModel = aseSubModelP->m_nSubModel;
	renderSubModelP->m_nParent = aseSubModelP->m_nParent;
	renderSubModelP->m_faces = renderFaceP;
	renderSubModelP->m_nFaces = nFaces = aseSubModelP->m_nFaces;
	renderSubModelP->m_bGlow = aseSubModelP->m_bGlow;
	renderSubModelP->m_bFlare = aseSubModelP->m_bFlare;
	renderSubModelP->m_bBillboard = aseSubModelP->m_bBillboard;
	renderSubModelP->m_bRender = aseSubModelP->m_bRender;
	renderSubModelP->m_bThruster = aseSubModelP->m_bThruster;
	renderSubModelP->m_bWeapon = aseSubModelP->m_bWeapon;
	renderSubModelP->m_bHeadlight = aseSubModelP->m_bHeadlight;
	renderSubModelP->m_bBombMount = aseSubModelP->m_bBombMount;
	renderSubModelP->m_nGun = aseSubModelP->m_nGun;
	renderSubModelP->m_nBomb = aseSubModelP->m_nBomb;
	renderSubModelP->m_nMissile = aseSubModelP->m_nMissile;
	renderSubModelP->m_nType = aseSubModelP->m_nType;
	renderSubModelP->m_nWeaponPos = aseSubModelP->m_nWeaponPos;
	renderSubModelP->m_nGunPoint = aseSubModelP->m_nGunPoint;
	renderSubModelP->m_bBullets = (aseSubModelP->m_nBullets > 0);
	renderSubModelP->m_nIndex = nIndex;
	renderSubModelP->m_iFrame = 0;
	renderSubModelP->m_tFrame = 0;
	renderSubModelP->m_nFrames = (aseSubModelP->m_bBarrel || aseSubModelP->m_bThruster) ? 32 : 0;
	renderSubModelP->m_vOffset = aseSubModelP->m_vOffset;
	renderSubModelP->InitMinMax ();
	for (aseFaceP = aseSubModelP->m_faces.Buffer (), iFace = 0; iFace < nFaces; iFace++, aseFaceP++, renderFaceP++) {
		renderFaceP->m_nIndex = nIndex;
		i = aseSubModelP->m_nTexture;
		CTexture& tex = aseModel.m_textures.Texture (i);
		bTextured = !tex.Flat ();
		renderFaceP->m_nTexture = bTextured ? i : -1;
		renderFaceP->m_nVerts = 3;
		renderFaceP->m_nId = iFace;
		for (i = 0; i < 3; i++, renderVertexP++) {
			h = aseFaceP->m_nVerts [i];
			if ((renderVertexP->m_bTextured = bTextured))
				renderVertexP->m_baseColor.r =
				renderVertexP->m_baseColor.g =
				renderVertexP->m_baseColor.b = 1.0f;
			else 
				tex.GetAverageColor (renderVertexP->m_baseColor);
			renderVertexP->m_baseColor.a = 1.0f;
			renderVertexP->m_renderColor = renderVertexP->m_baseColor;
			renderVertexP->m_normal = aseSubModelP->m_vertices [h].m_normal;
			renderVertexP->m_vertex = aseSubModelP->m_vertices [h].m_vertex * fScale;
			if (aseSubModelP->m_texCoord.Buffer ())
				renderVertexP->m_texCoord = aseSubModelP->m_texCoord [aseFaceP->m_nTexCoord [i]];
			h += nVerts;
			m_vertices [h] = renderVertexP->m_vertex;
			m_vertNorms [h] = renderVertexP->m_normal;
			renderVertexP->m_nIndex = h;
			renderSubModelP->SetMinMax (&renderVertexP->m_vertex);
			nIndex++;
			}
#if 1
		if (renderSubModelP->m_bThruster) {
			CFloatVector n = Normal ((renderVertexP - 3)->m_vertex, (renderVertexP - 2)->m_vertex, (renderVertexP - 1)->m_vertex);
			renderFaceP->m_vNormal = n;
			}
		else
#endif
			renderFaceP->m_vNormal = aseFaceP->m_vNormal;
		}
	nVerts += aseSubModelP->m_nVerts;
	}
}

//------------------------------------------------------------------------------

int CModel::BuildFromASE (ASE::CModel& aseModel)
{
if (aseModel.m_nModel < 0)
	return 0;
CountASEModelItems (aseModel);
if (!Create ()) 
	return 0;
GetASEModelItems (m_nModel = aseModel.m_nModel, aseModel, 1.0f); 
m_textures = aseModel.m_textures.Textures ();
m_nTextures = aseModel.m_textures.Count ();
memset (m_teamTextures, 0xFF, sizeof (m_teamTextures));
for (int i = 0; i < m_nTextures; i++) {
	int j = (int) aseModel.m_textures.Team (i);
	if (j)
		m_teamTextures [j - 1] = i;
	}
m_nType = 2;
Setup (1, 0);
return -1;
}

//------------------------------------------------------------------------------
//eof
