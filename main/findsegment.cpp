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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>	//	for memset ()

#include "u_mem.h"
#include "descent.h"
#include "error.h"
#include "mono.h"
#include "segmath.h"
#include "byteswap.h"
#include "light.h"
#include "segment.h"

// -------------------------------------------------------------------------------
//this was converted from GetSegMasks ()...it fills in an array of 6
//elements for the distace behind each CSide, or zero if not behind
//only gets centerMask, and assumes zero rad

uint8_t CSegment::GetSideDists (const CFixVector& pRef, fix* xSideDists, int32_t bBehind)
{
	uint8_t		mask = 0;

for (int32_t nSide = 0; nSide < SEGMENT_SIDE_COUNT; nSide++)
	if (m_sides [nSide].FaceCount ())
		mask |= m_sides [nSide].Dist (pRef, xSideDists [nSide], bBehind, 1 << nSide);
return mask;
}

// -------------------------------------------------------------------------------

uint8_t CSegment::GetSideDistsf (const CFloatVector& pRef, float* fSideDists, int32_t bBehind)
{
	uint8_t		mask = 0;

for (int32_t nSide = 0; nSide < SEGMENT_SIDE_COUNT; nSide++)
	if (m_sides [nSide].FaceCount ())
		mask |= m_sides [nSide].Distf (pRef, fSideDists [nSide], bBehind, 1 << nSide);
return mask;
}

// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
//figure out what seg the given point is in, tracing through segments
//returns CSegment number, or -1 if can't find segment

static int32_t TraceSegs (const CFixVector& vPos, int32_t nCurSeg, int32_t nTraceDepth, uint16_t* bVisited, uint16_t bFlag, fix xTolerance = 0)
{
	CSegment*	pSeg;
	fix			xSideDists [6], xMaxDist;
	int32_t			centerMask, nMaxSide, nSide, bit, nMatchSeg = -1;

if (nTraceDepth >= gameData.segData.nSegments) //gameData.segData.nSegments)
	return -1;
if (bVisited [nCurSeg] == bFlag)
	return -1;
bVisited [nCurSeg] = bFlag;
pSeg = SEGMENT (nCurSeg);
if (!(centerMask = pSeg->GetSideDists (vPos, xSideDists, 1)))		//we're in the old segment
	return nCurSeg;		
for (;;) {
	nMaxSide = -1;
	xMaxDist = 0; // find only sides we're behind as seen from inside the current segment
	for (nSide = 0, bit = 1; nSide < SEGMENT_SIDE_COUNT; nSide++, bit <<= 1)
		if (pSeg->Side (nSide)->FaceCount () && (centerMask & bit) && (xTolerance || (pSeg->m_children [nSide] > -1)) && (xSideDists [nSide] < xMaxDist)) {
			if (xTolerance && (xTolerance >= -xSideDists [nSide]) && (xTolerance >= pSeg->Side (nSide)->DistToPoint (vPos))) 
				return nCurSeg;
			if (pSeg->m_children [nSide] >= 0) {
				xMaxDist = xSideDists [nSide];
				nMaxSide = nSide;
				}
			}
	if (nMaxSide == -1)
		break;
	xSideDists [nMaxSide] = 0;
	if (0 <= (nMatchSeg = TraceSegs (vPos, pSeg->m_children [nMaxSide], nTraceDepth + 1, bVisited, bFlag, xTolerance)))	//trace into adjacent CSegment
		break;
	}
return nMatchSeg;		//we haven't found a CSegment
}

// -------------------------------------------------------------------------------

static int32_t TraceSegsf (const CFloatVector& vPos, int32_t nCurSeg, int32_t nTraceDepth, uint16_t* bVisited, uint16_t bFlag, float fTolerance)
{
	CSegment*		pSeg;
	float				fSideDists [6], fMaxDist;
	int32_t				centerMask, nMaxSide, nSide, bit, nMatchSeg = -1;

if (nTraceDepth >= gameData.segData.nSegments)
	return -1;
if (bVisited [nCurSeg] == bFlag)
	return -1;
bVisited [nCurSeg] = bFlag;
pSeg = SEGMENT (nCurSeg);
if (!(centerMask = pSeg->GetSideDistsf (vPos, fSideDists, 1)))		//we're in the old CSegment
	return nCurSeg;		
for (;;) {
	nMaxSide = -1;
	fMaxDist = 0; // find only sides we're behind as seen from inside the current segment
	for (nSide = 0, bit = 1; nSide < SEGMENT_SIDE_COUNT; nSide++, bit <<= 1)
		if (pSeg->Side (nSide)->FaceCount () && (centerMask & bit) && (fSideDists [nSide] < fMaxDist)) {
			if ((fTolerance >= -fSideDists [nSide])  && (fTolerance >= pSeg->Side (nSide)->DistToPointf (vPos))) {
#if DBG
				SEGMENT (nCurSeg)->GetSideDistsf (vPos, fSideDists, 1);
#endif
				return nCurSeg;
				}
			if (pSeg->m_children [nSide] >= 0) {
				fMaxDist = fSideDists [nSide];
				nMaxSide = nSide;
				}
			}
	if (nMaxSide == -1)
		break;
	fSideDists [nMaxSide] = 0;
	if (0 <= (nMatchSeg = TraceSegsf (vPos, pSeg->m_children [nMaxSide], nTraceDepth + 1, bVisited, bFlag, fTolerance)))	//trace into adjacent CSegment
		break;
	}
return nMatchSeg;		//we haven't found a segment
}

// -------------------------------------------------------------------------------

int32_t PointInSeg (CSegment* pSeg, CFixVector vPos)
{
if (!pSeg->m_nShape) {
	fix d = CFixVector::Dist (vPos, pSeg->Center ());
	if (d <= pSeg->m_rads [0])
		return 1;
	if (d > pSeg->m_rads [1])
		return 0;
	}
return (pSeg->Masks (vPos, 0).m_center == 0);
}

// -------------------------------------------------------------------------------

int32_t FindSegByPosExhaustive (const CFixVector& vPos, int32_t bSkyBox, int32_t nStartSeg)
{
if ((nStartSeg >= 0) && (PointInSeg (SEGMENT (nStartSeg), vPos)))
	return nStartSeg;

	int32_t			i;
	int16_t*		segListP = NULL;
	CSegment*	pSeg;

if (gameData.segData.HaveSegmentGrid (bSkyBox)) {
	for (i = gameData.segData.GetSegList (vPos, segListP, bSkyBox); i; i--, segListP++) {
#if DBG
		if ((*segListP < 0) || (*segListP >= gameData.segData.nSegments))
			continue;
#endif
		if (PointInSeg (SEGMENT (*segListP), vPos))
			return *segListP;
		}
	}
else if (bSkyBox) {
	for (i = gameData.segData.skybox.GetSegList (segListP); i; i--, segListP++) {
		pSeg = SEGMENT (*segListP);
		if ((pSeg->m_function == SEGMENT_FUNC_SKYBOX) && PointInSeg (pSeg, vPos))
			return *segListP;
		}
	}
else {
	pSeg = SEGMENTS.Buffer ();
	for (i = 0; i <= gameData.segData.nLastSegment; i++) {
		pSeg = SEGMENT (i);
		if ((pSeg->m_function != SEGMENT_FUNC_SKYBOX) && PointInSeg (pSeg, vPos))
			return i;
		}
	}
return -1;
}

// -------------------------------------------------------------------------------
//Find segment containing point vPos.

int32_t FindSegByPos (const CFixVector& vPos, int32_t nStartSeg, int32_t bExhaustive, int32_t bSkyBox, fix xTolerance, int32_t nThread)
{
	static uint16_t bVisited [MAX_THREADS][MAX_SEGMENTS_D2X]; 
	static uint16_t bFlags [MAX_THREADS] = {0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
	int32_t nSegment = -1;

if (nStartSeg >= 0) {
	if (PointInSeg (SEGMENT (nStartSeg), vPos))
		return nStartSeg;
	if (!xTolerance && bExhaustive && gameData.segData.HaveSegmentGrid (bSkyBox)) 
		return FindSegByPosExhaustive (vPos, SEGMENT (nStartSeg)->m_function == SEGMENT_FUNC_SKYBOX, nStartSeg);
	if (SEGMENT (nStartSeg)->m_function == SEGMENT_FUNC_SKYBOX) {
		if (0 <= (nSegment = TraceSegs (vPos, nStartSeg, 1, bVisited [nThread], bFlags [nThread], xTolerance))) 
			return nSegment;
		}
	if (!++bFlags [nThread]) {
		memset (bVisited [nThread], 0, sizeofa (bVisited [nThread]));
		bFlags [nThread] = 1;
		}
	if (0 <= (nSegment = TraceSegs (vPos, nStartSeg, 0, bVisited [nThread], bFlags [nThread], xTolerance))) 
		return nSegment;
	}

//couldn't find via attached segs, so search all segs
if (!bExhaustive)
	return -1;

if (bSkyBox < 0) {
	if (0 <= (nSegment = FindSegByPosExhaustive (vPos, 0)))
		return nSegment;
	bSkyBox = 1;
	}
return FindSegByPosExhaustive (vPos, bSkyBox);
}

// -------------------------------------------------------------------------------

int16_t FindClosestSeg (CFixVector& vPos)
{
	CSegment*	pSeg = SEGMENT (0);
	int16_t			nSegment, nClosestSeg = -1;
	fix			nDist, nMinDist = 0x7fffffff;

for (nSegment = 0; nSegment < gameData.segData.nSegments; nSegment++, pSeg++) {
	nDist = CFixVector::Dist (vPos, pSeg->Center ()) - pSeg->MinRad ();
	if (nDist < nMinDist) {
		nMinDist = nDist;
		nClosestSeg = nSegment;
		}
	}
return nClosestSeg;
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	Do a bfs from nSegment, marking slots in markedSegs if the segment is reachable.

#define	AMBIENT_SEGMENT_DEPTH 5

void AmbientMarkBfs (int16_t nSegment, int8_t* markedSegs, int32_t nDepth)
{
	int16_t	i, child;

if (nDepth < 0)
	return;
markedSegs [nSegment] = 1;
CSegment* pSeg = SEGMENT (nSegment);
for (i = 0; i < SEGMENT_SIDE_COUNT; i++) {
	child = pSeg->m_children [i];
	if (IS_CHILD (child) && (pSeg->IsPassable (i, NULL) & WID_TRANSPARENT_FLAG) && !markedSegs [child])
		AmbientMarkBfs (child, markedSegs, nDepth - 1);
	}
}

//	-----------------------------------------------------------------------------
//	Indicate all segments which are within audible range of falling water or lava,
//	and so should hear ambient gurgles.
void SetAmbientSoundFlagsCommon (int32_t tmi_bit, int32_t s2f_bit)
{
	int16_t		i, j;

	static int8_t	markedSegs [MAX_SEGMENTS_D2X];

	//	Now, all segments containing ambient lava or water sound makers are flagged.
	//	Additionally flag all segments which are within range of them.
memset (markedSegs, 0, sizeof (markedSegs));
for (i = 0; i <= gameData.segData.nLastSegment; i++) {
	SEGMENT (i)->m_flags &= ~s2f_bit;
	}

//	Mark all segments which are sources of the sound.
CSegment	*pSeg = SEGMENTS.Buffer ();
for (i = 0; i <= gameData.segData.nLastSegment; i++, pSeg++) {
	CSide	*pSide = pSeg->m_sides;
	for (j = 0; j < SEGMENT_SIDE_COUNT; j++, pSide++) {
		if (!pSide->FaceCount ())
			continue;
		if ((gameData.pig.tex.pTexMapInfo [pSide->m_nBaseTex].flags & tmi_bit) ||
			 (gameData.pig.tex.pTexMapInfo [pSide->m_nOvlTex].flags & tmi_bit)) {
			if (!IS_CHILD (pSeg->m_children [j]) || IS_WALL (pSide->m_nWall)) {
				pSeg->m_flags |= s2f_bit;
				markedSegs [i] = 1;		//	Say it's itself that it is close enough to to hear something.
				}
			}
		}
	}
//	Next mark all segments within N segments of a source.
pSeg = SEGMENTS.Buffer ();
for (i = 0; i <= gameData.segData.nLastSegment; i++, pSeg++) {
	if (pSeg->m_flags & s2f_bit)
		AmbientMarkBfs (i, markedSegs, AMBIENT_SEGMENT_DEPTH);
	}
//	Now, flip bits in all segments which can hear the ambient sound.
for (i = 0; i <= gameData.segData.nLastSegment; i++)
	if (markedSegs [i])
		SEGMENT (i)->m_flags |= s2f_bit;
}

//	-----------------------------------------------------------------------------
//	Indicate all segments which are within audible range of falling water or lava,
//	and so should hear ambient gurgles.

void SetAmbientSoundFlags (void)
{
SetAmbientSoundFlagsCommon (TMI_VOLATILE, S2F_AMBIENT_LAVA);
SetAmbientSoundFlagsCommon (TMI_WATER, S2F_AMBIENT_WATER);
}

//	-----------------------------------------------------------------------------
//eof
