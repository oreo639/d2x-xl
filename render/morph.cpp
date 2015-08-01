/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "descent.h"
#include "texmap.h"
#include "error.h"
#include "light.h"
#include "newdemo.h"
#include "interp.h"

//-------------------------------------------------------------
//returns ptr to data for this CObject, or NULL if none
tMorphInfo *MorphFindData (CObject *pObj)
{
if (gameData.demoData.nState == ND_STATE_PLAYBACK) {
	gameData.renderData.morph.objects [0].pObj = pObj;
	return &gameData.renderData.morph.objects [0];
	}

for (int32_t i = 0; i < MAX_MORPH_OBJECTS; i++)
	if (gameData.renderData.morph.objects [i].pObj == pObj)
		return gameData.renderData.morph.objects + i;
return NULL;
}

//-------------------------------------------------------------
//takes pModel, fills in min& max
void MorphFindModelBounds (CPolyModel* pModel, int32_t nSubModel, CFixVector& minv, CFixVector& maxv)
{
	uint16_t nVerts;
	CFixVector *vp;
	uint16_t *data, nType;

	data = reinterpret_cast<uint16_t*> (pModel->Data () + pModel->SubModels ().ptrs [nSubModel]);
	nType = *data++;
	Assert (nType == 7 || nType == 1);
	nVerts = *data++;
	if (nType==7)
		data+=2;		//skip start & pad
	vp = reinterpret_cast<CFixVector*> (data);
	minv = maxv = *vp++;
	nVerts--;
	while (nVerts--) {
		if ((*vp).v.coord.x > maxv.v.coord.x)
			maxv.v.coord.x = (*vp).v.coord.x;
		else if((*vp).v.coord.x < minv.v.coord.x)
			minv.v.coord.x = (*vp).v.coord.x;
		if ((*vp).v.coord.y > maxv.v.coord.y)
			maxv.v.coord.y = (*vp).v.coord.y;
		else if ((*vp).v.coord.y < minv.v.coord.y)
			minv.v.coord.y = (*vp).v.coord.y;
		if ((*vp).v.coord.z > maxv.v.coord.z)
			maxv.v.coord.z = (*vp).v.coord.z;
		else if ((*vp).v.coord.z < minv.v.coord.z)
			minv.v.coord.z = (*vp).v.coord.z;
		vp++;
	}
}

//-------------------------------------------------------------

int32_t MorphInitPoints (CPolyModel* pModel, CFixVector *vBoxSize, int32_t nSubModel, tMorphInfo *pMorphInfo)
{
	uint16_t		nVerts;
	CFixVector	*vp, v;
	uint16_t		*data, nType;
	int32_t			i;

//printf ("initing %d ", nSubModel);
data = reinterpret_cast<uint16_t*> (pModel->Data () + pModel->SubModels ().ptrs [nSubModel]);
nType = *data++;
#if DBG
//Assert (nType == 7 || nType == 1);
if (nType != 7 && nType != 1)
	return 0;
#endif
nVerts = *data++;
pMorphInfo->nMorphingPoints [nSubModel] = 0;
if (nType==7) {
	i = *data++;		//get start point number
	data++;				//skip pad
	}
else
	i = 0;				//start at zero
Assert (i+nVerts < MAX_VECS);
pMorphInfo->submodelStartPoints [nSubModel] = i;
vp = reinterpret_cast<CFixVector*> (data);
v = *vp;
while (nVerts--) {
	fix k, dist;

	if (vBoxSize) {
		fix t;
		k = 0x7fffffff;
		if (v.v.coord.x && X2I ((*vBoxSize).v.coord.x) < abs (v.v.coord.x)/2 && (t = FixDiv ((*vBoxSize).v.coord.x, abs (v.v.coord.x))) < k)
			k = t;
		if (v.v.coord.y && X2I ((*vBoxSize).v.coord.y) < abs (v.v.coord.y)/2 && (t = FixDiv ((*vBoxSize).v.coord.y, abs (v.v.coord.y))) < k)
			k = t;
		if (v.v.coord.z && X2I ((*vBoxSize).v.coord.z) < abs (v.v.coord.z)/2 && (t = FixDiv ((*vBoxSize).v.coord.z, abs (v.v.coord.z))) < k)
			k = t;
		if (k == 0x7fffffff)
			k = 0;
		}
	else
		k = 0;
	pMorphInfo->vecs[i] = *vp * k;
	dist = CFixVector::NormalizedDir(pMorphInfo->deltas[i], *vp, pMorphInfo->vecs[i]);
	pMorphInfo->times [i] = FixDiv (dist, gameData.renderData.morph.xRate);
	if (pMorphInfo->times [i] != 0)
		pMorphInfo->nMorphingPoints [nSubModel]++;
		pMorphInfo->deltas [i] *= gameData.renderData.morph.xRate;
	vp++;
	i++;
	}
return 1;
////printf ("npoints = %d\n", nMorphingPoints [nSubModel]);
}

//-------------------------------------------------------------

int32_t MorphUpdatePoints (CPolyModel* pModel, int32_t nSubModel, tMorphInfo *pMorphInfo)
{
	uint16_t nVerts;
	CFixVector *vp;
	uint16_t *data, nType;
	int32_t i;

	////printf ("updating %d ", nSubModel);
data = reinterpret_cast<uint16_t*> (pModel->Data () + pModel->SubModels ().ptrs [nSubModel]);
nType = *data++;
#if DBG
//Assert (nType == 7 || nType == 1);
if (nType != 7 && nType != 1)
	return 0;
#endif
nVerts = *data++;
if (nType == 7) {
	i = *data++;		//get start point number
	data++;				//skip pad
	}
else
	i = 0;				//start at zero
vp = reinterpret_cast<CFixVector*> (data);
while (nVerts--) {
	if (pMorphInfo->times [i]) {		//not done yet
		if ((pMorphInfo->times [i] -= gameData.timeData.xFrame) <= 0) {
			 pMorphInfo->vecs [i] = *vp;
			 pMorphInfo->times [i] = 0;
			 pMorphInfo->nMorphingPoints [nSubModel]--;
			}
		else
			pMorphInfo->vecs[i] += pMorphInfo->deltas[i] * gameData.timeData.xFrame;
		}
	vp++;
	i++;
	}
return 1;
////printf ("npoints = %d\n", nMorphingPoints [nSubModel]);
}


//-------------------------------------------------------------
//process the morphing CObject for one frame
void CObject::DoMorphFrame (void)
{
	int32_t			i, t;
	CPolyModel	*pModel;
	tMorphInfo	*pMorphInfo;

if (!(pMorphInfo = MorphFindData (this))) {	//maybe loaded half-morphed from disk
	Die ();	//so kill it
	return;
	}
pModel = gameData.modelData.polyModels [0] + pMorphInfo->pObj->ModelId ();
G3CheckAndSwap (reinterpret_cast<void*> (pModel->Data ()));
for (i = 0; i < pModel->ModelCount (); i++)
	if (pMorphInfo->submodelActive [i] == 1) {
		if (!MorphUpdatePoints (pModel, i, pMorphInfo))
			pMorphInfo->submodelActive [i] = 0;
      else if (pMorphInfo->nMorphingPoints [i] == 0) {		//maybe start submodel
			pMorphInfo->submodelActive [i] = 2;		//not animating, just visible
			pMorphInfo->nSubmodelsActive--;		//this one done animating
			for (t = 0; t < pModel->ModelCount (); t++)
				if (pModel->SubModels ().parents [t] == i) 		//start this one
					if ((pMorphInfo->submodelActive [t] = MorphInitPoints (pModel, NULL, t, pMorphInfo)))
						pMorphInfo->nSubmodelsActive++;
			}
		}
if (!pMorphInfo->nSubmodelsActive) {			//done morphing!
	CObject* pObj = pMorphInfo->pObj;
	pObj->info.controlType = pMorphInfo->saveControlType;
	pObj->info.movementType = pMorphInfo->saveMovementType;
	pObj->info.renderType = RT_POLYOBJ;
	pObj->mType.physInfo = pMorphInfo->savePhysInfo;
	pMorphInfo->pObj = NULL;
	}
}

//-------------------------------------------------------------

CFixVector morph_rotvel = CFixVector::Create(0x4000, 0x2000, 0x1000);

void MorphInit ()
{
	int32_t i;

for (i = 0; i < MAX_MORPH_OBJECTS; i++)
	gameData.renderData.morph.objects [i].pObj = NULL;
}


//-------------------------------------------------------------
//make the CObject morph
void CObject::MorphStart (void)
{
	CPolyModel* pModel;
	CFixVector pmmin, pmmax;
	CFixVector vBoxSize;
	int32_t i;
	tMorphInfo *pMorphInfo = &gameData.renderData.morph.objects [0];

for (i = 0; i < MAX_MORPH_OBJECTS; i++, pMorphInfo++)
	if (pMorphInfo->pObj == NULL ||
		 pMorphInfo->pObj->info.nType == OBJ_NONE  ||
		 pMorphInfo->pObj->info.nSignature != pMorphInfo->nSignature)
		break;

if (i == MAX_MORPH_OBJECTS)		//no free slots
	return;

Assert (info.renderType == RT_POLYOBJ);
pMorphInfo->pObj = this;
pMorphInfo->nSignature = info.nSignature;
pMorphInfo->saveControlType = info.controlType;
pMorphInfo->saveMovementType = info.movementType;
pMorphInfo->savePhysInfo = mType.physInfo;
Assert (info.controlType == CT_AI);		//morph OBJECTS are also AI gameData.objPs.objPects
info.controlType = CT_MORPH;
info.renderType = RT_MORPH;
info.movementType = MT_PHYSICS;		//RT_NONE;
mType.physInfo.rotVel = morph_rotvel;
pModel = gameData.modelData.polyModels [0] + ModelId ();
if (!pModel->Data ())
	return;
G3CheckAndSwap (reinterpret_cast<void*> (pModel->Data ()));
MorphFindModelBounds (pModel, 0, pmmin, pmmax);
vBoxSize.v.coord.x = Max (-pmmin.v.coord.x, pmmax.v.coord.x) / 2;
vBoxSize.v.coord.y = Max (-pmmin.v.coord.y, pmmax.v.coord.y) / 2;
vBoxSize.v.coord.z = Max (-pmmin.v.coord.z, pmmax.v.coord.z) / 2;
for (i = 0; i < MAX_VECS; i++)		//clear all points
	pMorphInfo->times [i] = 0;
for (i = 1; i < MAX_SUBMODELS; i++)		//clear all parts
	pMorphInfo->submodelActive [i] = 0;
pMorphInfo->submodelActive [0] = 1;		//1 means visible & animating
pMorphInfo->nSubmodelsActive = 1;
//now, project points onto surface of box
MorphInitPoints (pModel, &vBoxSize, 0, pMorphInfo);
}

//-------------------------------------------------------------

void MorphDrawModel (CPolyModel* pModel, int32_t nSubModel, CAngleVector *animAngles, fix light, tMorphInfo *pMorphInfo, int32_t nModel)
{
	int32_t h, i, j, m, n;
	int32_t sortList [2 * MAX_SUBMODELS];
	//first, sort the submodels

n = h = pModel->ModelCount ();
m = h - 1;
sortList [m] = nSubModel;
for (i = 0; i < h; i++) {
	if (pMorphInfo->submodelActive [i] && pModel->SubModels ().parents [i] == nSubModel) {
		if (!G3CheckNormalFacing (pModel->SubModels ().pnts [i], pModel->SubModels ().norms [i]))
			sortList [n++] = i;
		else
			sortList [--m] = i;
		}
	}

//now draw everything
for (i = m; i < n; i++) {
	m = sortList [i];
	if (m == nSubModel) {
		h = pModel->TextureCount ();
		for (j = 0; j < h; j++) {
			gameData.modelData.textureIndex [j] = gameData.pigData.tex.objBmIndex [gameData.pigData.tex.pObjBmIndex [pModel->FirstTexture () + j]];
			gameData.modelData.textures [j] = gameData.pigData.tex.bitmaps [0] + /*gameData.pigData.tex.objBmIndex [gameData.pigData.tex.pObjBmIndex [pModel->FirstTexture ()+j]]*/gameData.modelData.textureIndex [j].index;
			}

		// Make sure the textures for this CObject are paged in..
		gameData.pigData.tex.bPageFlushed = 0;
		for (j = 0; j < h; j++)
			LoadTexture (gameData.modelData.textureIndex [j].index, 0, 0);
		// Hmmm.. cache got flushed in the middle of paging all these in,
		// so we need to reread them all in.
		if (gameData.pigData.tex.bPageFlushed) {
			gameData.pigData.tex.bPageFlushed = 0;
			for (int32_t j = 0; j < h; j++)
				LoadTexture (gameData.modelData.textureIndex [j].index, 0, 0);
			}
			// Make sure that they can all fit in memory.
		Assert (gameData.pigData.tex.bPageFlushed == 0);

		G3DrawMorphingModel (
			pModel->Data () + pModel->SubModels ().ptrs [nSubModel],
			gameData.modelData.textures,
			animAngles, NULL, light,
			pMorphInfo->vecs + pMorphInfo->submodelStartPoints [nSubModel], nModel);
		}
	else {
		CFixMatrix orient;
		orient = CFixMatrix::Create (animAngles [m]);
		transformation.Begin (pModel->SubModels ().offsets [m], orient, __FILE__, __LINE__);
		MorphDrawModel (pModel, m, animAngles, light, pMorphInfo, nModel);
		transformation.End (__FILE__, __LINE__);
		}
	}
}

//-------------------------------------------------------------

void CObject::MorphDraw (void)
{
//	int32_t save_light;
	CPolyModel* pModel;
	fix light;
	tMorphInfo *pMorphInfo;

pMorphInfo = MorphFindData (this);
Assert (pMorphInfo != NULL);
Assert (ModelId () < gameData.modelData.nPolyModels);
pModel = gameData.modelData.polyModels [0] + ModelId ();
if (!pModel->Data ())
	return;
light = ComputeObjectLight (this, NULL);
transformation.Begin (info.position.vPos, info.position.mOrient, __FILE__, __LINE__);
G3SetModelPoints (gameData.modelData.polyModelPoints.Buffer ());
gameData.renderData.pVertex = gameData.modelData.fPolyModelVerts.Buffer ();
MorphDrawModel (pModel, 0, rType.polyObjInfo.animAngles, light, pMorphInfo, ModelId ());
gameData.renderData.pVertex = NULL;
transformation.End (__FILE__, __LINE__);

if (gameData.demoData.nState == ND_STATE_RECORDING)
	NDRecordMorphFrame (pMorphInfo);
}

//-------------------------------------------------------------
//eof
