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

#define NEW_FVI_STUFF 1

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "descent.h"
#include "u_mem.h"
#include "error.h"
#include "rle.h"
#include "segmath.h"
#include "interp.h"
#include "hitbox.h"
#include "network.h"
#include "renderlib.h"
#include "visibility.h"

//	-----------------------------------------------------------------------------

int PixelTranspType (short nTexture, short nOrient, short nFrame, fix u, fix v)
{
	CBitmap *bmP;
	int bmx, bmy, w, h, offs;
	ubyte	c;
#if 0
	tBitmapIndex *bmiP;

bmiP = gameData.pig.tex.bmIndexP + (nTexture);
LoadTexture (*bmiP, gameStates.app.bD1Data);
bmP = BmOverride (gameData.pig.tex.bitmapP + bmiP->index);
#else
bmP = LoadFaceBitmap (nTexture, nFrame);
if (!bmP->Buffer ())
	return 0;
#endif
if (bmP->Flags () & BM_FLAG_RLE)
	bmP = rle_expand_texture (bmP);
w = bmP->Width ();
h = ((bmP->Type () == BM_TYPE_ALT) && bmP->Frames ()) ? w : bmP->Height ();
if (nOrient == 0) {
	bmx = ((unsigned) X2I (u * w)) % w;
	bmy = ((unsigned) X2I (v * h)) % h;
	}
else if (nOrient == 1) {
	bmx = ((unsigned) X2I ((I2X (1) - v) * w)) % w;
	bmy = ((unsigned) X2I (u * h)) % h;
	}
else if (nOrient == 2) {
	bmx = ((unsigned) X2I ((I2X (1) - u) * w)) % w;
	bmy = ((unsigned) X2I ((I2X (1) - v) * h)) % h;
	}
else {
	bmx = ((unsigned) X2I (v * w)) % w;
	bmy = ((unsigned) X2I ((I2X (1) - u) * h)) % h;
	}
offs = bmy * w + bmx;
if (bmP->Flags () & BM_FLAG_TGA) {
	ubyte *p;

	if (bmP->BPP () == 3)	//no alpha -> no transparency
		return 0;
	p = bmP->Buffer () + offs * bmP->BPP ();
	// check super transparency color
#if 1
	if ((p [0] == 120) && (p [1] == 88) && (p [2] == 128))
#else
	if ((gameOpts->ogl.bGlTexMerge) ?
	    (p [3] == 1) : ((p [0] == 120) && (p [1] == 88) && (p [2] == 128)))
#endif
		return -1;
	// check alpha
	if (!p [3])
		return 1;
	}
else {
	c = bmP->Buffer () [offs];
	if (c == SUPER_TRANSP_COLOR)
		return -1;
	if (c == TRANSPARENCY_COLOR)
		return 1;
	}
return 0;
}

//	-----------------------------------------------------------------------------
//check if a particular refP on a CWall is a transparent pixel
//returns 1 if can pass though the CWall, else 0
int CSide::CheckForTranspPixel (CFixVector& intersection, short iFace)
{
	fix	u, v;
	int	nTranspType;

HitPointUV (&u, &v, NULL, intersection, iFace);	//	Don't compute light value.
if (m_nOvlTex) {
	nTranspType = PixelTranspType (m_nOvlTex, m_nOvlOrient, m_nFrame, u, v);
	if (nTranspType < 0)
		return 1;
	if (!nTranspType)
		return 0;
	}
nTranspType = PixelTranspType (m_nBaseTex, 0, m_nFrame, u, v) != 0;
return nTranspType;
}

//------------------------------------------------------------------------------

int CanSeePoint (CObject *objP, CFixVector *vSource, CFixVector *vDest, short nSegment, fix xRad)
{
	CHitQuery	fq (FQ_TRANSWALL, vSource, vDest, -1, 1, xRad, objP ? objP->Index () : -1);
	CHitData		hitData;
if (SPECTATOR (objP))
	fq.startSeg = FindSegByPos (objP->info.position.vPos, objP->info.nSegment, 1, 0);
else
	fq.startSeg = objP ? objP->info.nSegment : nSegment;

int nHitType = FindHitpoint (&fq, &hitData);
return nHitType != HIT_WALL;
}

//	-----------------------------------------------------------------------------
//returns true if viewerP can see CObject

int CanSeeObject (int nObject, int bCheckObjs)
{
	CHitQuery	fq (bCheckObjs ? FQ_CHECK_OBJS | FQ_TRANSWALL : FQ_TRANSWALL,
						 &gameData.objs.viewerP->info.position.vPos,
						 &OBJECTS [nObject].info.position.vPos,
						 gameData.objs.viewerP->info.nSegment,
						 0, 0,
						 gameStates.render.cameras.bActive ? -1 : OBJ_IDX (gameData.objs.viewerP)
						);
	CHitData		hitData;

int nHitType = FindHitpoint (&fq, &hitData);
return bCheckObjs ? (nHitType == HIT_OBJECT) && (hitData.hit.nObject == nObject) : (nHitType != HIT_WALL);
}

//	-----------------------------------------------------------------------------------------------------------
//	Determine if two OBJECTS are on a line of sight.  If so, return true, else return false.
//	Calls fvi.
int ObjectToObjectVisibility (CObject *objP1, CObject *objP2, int transType)
{
	CHitQuery	fq;
	CHitData		hitData;
	int			fate, nTries = 0, bSpectate = SPECTATOR (objP1);

do {
	if (nTries++) {
		fq.flags = transType;
		fq.p0 = bSpectate ? &gameStates.app.playerPos.vPos : &objP1->info.position.vPos;
		fq.p1 = SPECTATOR (objP2) ? &gameStates.app.playerPos.vPos : &objP2->info.position.vPos;
		fq.startSeg	= bSpectate ? FindSegByPos (gameStates.app.playerPos.vPos, gameStates.app.nPlayerSegment, 1, 0) :
						  FindSegByPos (objP1->info.position.vPos, objP1->info.nSegment, 1, 0);
		fq.radP0 =
		fq.radP1 = 0x10;
		fq.thisObjNum = OBJ_IDX (objP1);
		if (fq.startSeg < 0) {
			fate = HIT_BAD_P0;
			return false;
			}
		}
	else
		fq.startSeg	= bSpectate ? gameStates.app.nPlayerSegment : objP1->info.nSegment;
	fate = FindHitpoint (&fq, &hitData);
	}
while ((fate == HIT_BAD_P0) && (nTries < 2));
return (fate == HIT_NONE) || (fate == HIT_BAD_P0);
}

//	-----------------------------------------------------------------------------

int TargetInLineOfFire (void)
{
	int			nType;
	CHitData		hitData;
	CObject*		objP;

	//see if we can see this CPlayerData

CFixVector vEndPos = gameData.objs.viewerP->info.position.vPos + gameData.objs.viewerP->info.position.mOrient.m.dir.f * I2X (2000);
CHitQuery fq (FQ_CHECK_OBJS | FQ_VISIBLE_OBJS | FQ_IGNORE_POWERUPS | FQ_TRANSWALL | FQ_VISIBILITY,
				  &gameData.objs.viewerP->info.position.vPos,
				  &vEndPos,
				  gameData.objs.viewerP->info.nSegment,
				  0, 0,
				  OBJ_IDX (gameData.objs.viewerP)
				 );

int nHitType = FindHitpoint (&fq, &hitData);
if (nHitType != HIT_OBJECT)
	return 0;
objP = OBJECTS + hitData.hit.nObject;
nType = objP->Type ();
if ((nType == OBJ_ROBOT) || (nType == OBJ_REACTOR))
	return 1;
if (nType != OBJ_PLAYER)
	return 0;
if (IsCoopGame)
	return 0;
if (!IsTeamGame)
	return 1;
return GetTeam (gameData.objs.consoleP->info.nId) != GetTeam (objP->info.nId);
}

//	-----------------------------------------------------------------------------
//eof
