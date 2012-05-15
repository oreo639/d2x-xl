// Copyright (c) 1997 Bryan Aamot
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <string.h>

#include "mine.h"
#include "dle-xp.h"
#include "ASEModel.h"
#include "RenderModel.h"
#include "ModelManager.h"
#include "glew.h"

CModelManager modelManager;

int nDbgModel = -1;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int CModelManager::Model (void)
{
if (theMine == null)
	return -1;

	uint nModel;

switch (m_object->m_info.type) {
	case OBJ_PLAYER:
	case OBJ_COOP:
		nModel = D2_PLAYER_CLIP_NUMBER;
		break;
	case OBJ_WEAPON:
		nModel = MINE_CLIP_NUMBER;
		break;
	case OBJ_REACTOR:
		switch (m_object->m_info.id) {
			case 1:  nModel = 95;  break;
			case 2:  nModel = 97;  break;
			case 3:  nModel = 99;  break;
			case 4:  nModel = 101; break;
			case 5:  nModel = 103; break;
			case 6:  nModel = 105; break;
			default: nModel = 97;  break; // level 1's reactor
		}
		break;
	case OBJ_ROBOT:
		nModel = robotManager.RobotInfo (m_object->m_info.id)->Info ().nModel;
		break;
	default:
		return -1;
	}
return nModel;
}

//------------------------------------------------------------------------------

int CModelManager::Setup (CGameObject *objP, CRenderer* renderer, CDC* pDC) 
{
m_renderer = renderer;
m_viewMatrix = renderer->ViewMatrix ();
m_pDC = pDC;

m_offset.Clear ();
m_data.nPoints = 0;
m_nGlow = -1;
modelManager.m_object = objP;
m_nModel = Model ();

if (m_nModel < 0)
	return 0;

if (m_polyModels [0][m_nModel].m_info.renderData || m_polyModels [1][m_nModel].m_info.renderData || (m_renderModels [m_nModel].m_nModel >= 0))
	return 1;

char filename[256];

strcpy_s (filename, sizeof (filename), descentFolder [1]);
char *slash = strrchr (filename, '\\');
if (slash)
	*(slash+1) = '\0';
else
	filename[0] = '\0';

int bCustom = robotManager.RobotInfo (objP->Id ())->Info ().bCustom;
int bVertigo = objP->Id () > N_ROBOT_TYPES_D2;

if (bCustom) {
	char *psz = strstr (filename, "data");
	if (psz)
		*psz = '\0';
	}
if (bVertigo)
	strcpy_s (filename, sizeof (filename), "..\\missions\\d2x.hog");
else
	strcat_s (filename, sizeof (filename), "descent2.ham");
if (!ReadModelData (filename, bVertigo ? "d2x.ham" : "", bCustom || bVertigo))
	return 0;
return 1;
}

//------------------------------------------------------------------------------

void CModelManager::Reset (void)
{
CPolyModel* modelP = &m_polyModels [1][0];
for (int i = 0; i < MAX_POLYGON_MODELS; i++, modelP++) {
	m_renderModels [i].Destroy ();
	m_aseModels [i].Destroy ();
	m_oofModels [i].Destroy ();
	modelP->Release ();
	modelP->Clear ();
	}
}

//------------------------------------------------------------------------------

int CModelManager::ReadModelData (char* filename, char *szSubFile, bool bCustom) 
{
	CFileManager fp;

if (!fp.Open (filename, "rb"))
	return 0;

	uint	id;
	uint	i, n;

if (bCustom) {
	if (0 > hogManager->ReadSignature (&fp))
		return 0;

	CLevelHeader lh;
	long position;

	position = 3;
	while (!fp.EoF ()) {
		fp.Seek (position, SEEK_SET);
		if (!lh.Read (&fp)) {
			fp.Close ();
			return 0;
			}
		if (!strcmp (lh.Name (), szSubFile)) {
			id = fp.ReadInt32 ();	  					   
			if (id != 0x5848414DL) {
				fp.Close ();
				return 0;
				}
			fp.ReadInt32 ();											
			n = fp.ReadInt32 ();										
			fp.Seek (n * sizeof (WEAPON_INFO), SEEK_CUR);	
			n = fp.ReadInt32 ();										
			for (i = 0; i < n; i++)
				robotManager.RobotInfo (N_ROBOT_TYPES_D2 + i)->Read (&fp);
			n = fp.ReadInt32 ();                         
			fp.Seek (n * sizeof (JOINTPOS), SEEK_CUR);     
			break;
			}
		position += lh.Size () + lh.FileSize ();
		}
	n = fp.ReadInt32 ();                          
	for (i = 0; i < n; i++) 
		m_polyModels [0][N_POLYGON_MODELS_D2 + i].Read (&fp);
	for (i = 0; i < n; i++) 
		m_polyModels [0][N_POLYGON_MODELS_D2 + i].Read (&fp, true);
	}
else {
	id = fp.ReadInt32 ();	  					   
	if (id != 0x214d4148L) {
		fp.Close ();
		return 0;
		}
	fp.ReadInt32 ();                              
	n = fp.ReadUInt32 ();                         
	fp.Seek (n * sizeof (ushort), SEEK_CUR);	 
	fp.Seek (n * sizeof (TMAP_INFO), SEEK_CUR); 
	n = fp.ReadUInt32 ();                          
	fp.Seek (n * sizeof (ubyte), SEEK_CUR);     
	fp.Seek (n * sizeof (ubyte), SEEK_CUR);     
	n = fp.ReadUInt32 ();                          
#if 1
	fp.Read (m_animations, sizeof (tAnimationInfo), n);
#else
	fp.Seek (n * sizeof (tAnimationInfo), SEEK_CUR);     
#endif
	n = fp.ReadUInt32 ();                          
	fp.Seek (n * sizeof (tEffectAnimationInfo), SEEK_CUR);     
	n = fp.ReadUInt32 ();                          
	fp.Seek (n * sizeof (tWallAnimationInfo), SEEK_CUR);    
	n = fp.ReadUInt32 ();                          
	for (i = 0; i < n; i++) 
		robotManager.RobotInfo (i)->Read (&fp);
	n = fp.ReadUInt32 ();                         
	fp.Seek (n * sizeof (JOINTPOS), SEEK_CUR);     
	n = fp.ReadUInt32 ();                         
	fp.Seek (n * sizeof (WEAPON_INFO), SEEK_CUR);  
	n = fp.ReadUInt32 ();                         
	fp.Seek (n * sizeof (POWERUP_TYPE_INFO), SEEK_CUR); 
	n = fp.ReadUInt32 ();                          
	for (i = 0; i < n; i++) 
		m_polyModels [0][i].Read (&fp);
	for (i = 0; i < n; i++) 
		m_polyModels [0][i].Read (&fp, true);
	fp.Seek (2 * n * sizeof (int), SEEK_CUR);
	n = fp.ReadUInt32 ();                          
	fp.Seek (2 * n * sizeof (short), SEEK_CUR);
	n = fp.ReadUInt32 ();                         
	fp.Read (m_textureIndex [0], sizeof (ushort), n);
	fp.Read (m_modelTextureIndex [0], sizeof (ushort), n);
	}
fp.Close ();
return 1;
}

//------------------------------------------------------------------------------

void CModelManager::ReadCustomModelData (ubyte* buffer, long bufSize)
{
CMemoryFile mf;
if (mf.Open (buffer, bufSize)) {
	uint n, i, j;
	
	memcpy (m_textureIndex [1], m_textureIndex [0], sizeof (m_textureIndex [1]));
	memcpy (m_modelTextureIndex [1], m_modelTextureIndex [0], sizeof (m_modelTextureIndex [1]));
	n = mf.ReadUInt32 ();                         
	mf.Seek (n * (sizeof (int) + sizeof (JOINTPOS)), SEEK_CUR);     
	n = mf.ReadUInt32 ();                         
	for (j = 0; j < n; j++) {
		i = mf.ReadUInt32 ();
#ifdef _DEBUG
		if (i == nDbgModel)
			nDbgModel = nDbgModel;
#endif
		m_polyModels [1][i].Read (&mf);
		m_polyModels [1][i].Read (&mf, true);
		if ((m_polyModels [0][i].m_info.dataSize != m_polyModels [1][i].m_info.dataSize) ||
			memcmp (m_polyModels [0][i].m_info.renderData, m_polyModels [1][i].m_info.renderData, m_polyModels [0][i].m_info.dataSize))
			m_polyModels [1][i].m_bCustom = 1;
		else {
			m_polyModels [0][i].Release ();
			m_polyModels [0][i].Clear ();
			}
		mf.Seek (2 * sizeof (int), SEEK_CUR);
		}
	n = mf.ReadUInt32 ();                         
	for (j = 0; j < n; j++) {
		i = mf.ReadInt32 ();
		m_textureIndex [1][i] = mf.ReadUInt16 ();
		}
	n = mf.ReadUInt32 ();                         
	for (j = 0; j < n; j++) {
		i = mf.ReadInt32 ();
		m_modelTextureIndex [1][i] = mf.ReadUInt16 ();
		}
	}
}

//------------------------------------------------------------------------------

void CModelManager::ReadMod (char* pszFolder)
{
for (int i = 0; i < MAX_POLYGON_MODELS; i++) {
	DLE.MainFrame ()->Progress ().StepIt ();
#ifdef _DEBUG
	if (i == nDbgModel)
		nDbgModel = nDbgModel;
#endif
	if (/*m_polyModels [1][i].m_info.renderData ||*/ (m_aseModels [i].m_nModel >= 0) || (m_oofModels [i].m_nModel >= 0))
		continue; // already have a custom model
	if (m_aseModels [i].Read (pszFolder, i))
		m_renderModels [i].BuildFromASE (m_aseModels [i]);
	else if (m_oofModels [i].Read (pszFolder, i))
		m_renderModels [i].BuildFromOOF (m_oofModels [i]);
	}
}

//------------------------------------------------------------------------------

void CModelManager::LoadMod (void)
{
if (DLE.MakeModFolders ("models")) {
	DLE.MainFrame ()->InitProgress (2 * MAX_POLYGON_MODELS);
	// first read the level specific textures
	ReadMod (DLE.m_modFolders [0]);
	// then read the mission wide textures
	ReadMod (DLE.m_modFolders [1]);
	DLE.MainFrame ()->Progress ().DestroyWindow ();
	}
}

//------------------------------------------------------------------------------
// set_model_points ()
//
// Rotates, translates, then sets screen points (xy) for 3d model points
//------------------------------------------------------------------------------

void CModelManager::SetPoints (int start, int end) 
{
for (int i = start; i < end; i++) {
	CVertex& v = m_screenPoly [i];
	// rotate point using Objects () rotation matrix
	m_object->Transform (v, m_data.points [i]);
	// now that points are relative to set screen xy points (modelManager.m_screenPoly)
	v.Transform (m_viewMatrix);
	v.Project (m_viewMatrix);
	if (m_screenRect [0].x > v.m_screen.x)
		m_screenRect [0].x = v.m_screen.x;
	if (m_screenRect [0].y > v.m_screen.y)
		m_screenRect [0].y = v.m_screen.y;
	if (m_screenRect [1].x < v.m_screen.x)
		m_screenRect [1].x = v.m_screen.x;
	if (m_screenRect [1].y < v.m_screen.y)
		m_screenRect [1].y = v.m_screen.y;
	}
}

//------------------------------------------------------------------------------

void CModelManager::Polygon (ushort* index, int nVertices, tTexCoord2d* texCoords, rgbColord* color, short nTexture)
{ 
if (m_renderer->Type ()) {
	m_renderer->TexturedPolygon (nTexture, texCoords, color, m_screenPoly, nVertices, index);
	}
else {
	CPoint points [MAX_POLYMODEL_POLY_POINTS];
	for (int i = 0; i < nVertices; i++) {
		int j = index [i];
		points [i].x = m_screenPoly [j].m_screen.x;
		points [i].y = m_screenPoly [j].m_screen.y;
		}
	m_renderer->Polygon (points, nVertices); 
	}
}

//------------------------------------------------------------------------------

void CModelManager::Draw (void) 
{ 
if (m_nModel >= 0) {
#ifdef _DEBUG
	if (m_nModel == nDbgModel)
		nDbgModel = nDbgModel;
#endif
	m_screenRect [0].x = m_screenRect [0].y = 0x7FFFFFFF;
	m_screenRect [1].x = m_screenRect [1].y = -0x7FFFFFFF;
	if (m_renderer->Type ()) {
		m_renderer->BeginRender ();

		/*if (m_polyModels [1][m_nModel].Draw (m_viewMatrix, m_pDC, 0))
			;
		else*/ if (m_renderModels [m_nModel].Render (m_viewMatrix, m_object))
			;
		else
			m_polyModels [0][m_nModel].Draw (m_viewMatrix, m_pDC, 0);
		m_renderer->EndRender ();
		}
	else {
		for (int nStage = 0; nStage < 2; nStage++) {
			m_renderer->BeginRender (nStage > 0);
			m_polyModels [0][m_nModel].Draw (m_viewMatrix, m_pDC, nStage ? 1 : -1);
			m_renderer->EndRender ();
			}
		}
	}
}

//------------------------------------------------------------------------------

void CModelRenderPoly::Draw (void) 
{
//if (modelManager.CheckNormal (offset, normal))
	modelManager.Polygon (index, nVerts, texCoords, &color, nTexture);
}

//------------------------------------------------------------------------------
//eof ModelManager.cpp

