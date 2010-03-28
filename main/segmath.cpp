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
#include "gameseg.h"
#include "byteswap.h"
#include "light.h"
#include "segment.h"
#include "dialheap.h"

// How far a point can be from a plane, and still be "in" the plane

// -----------------------------------------------------------------------------------
// Fill in array with four absolute point numbers for a given CSide
void GetCorners (int nSegment, int nSide, ushort* vertIndex)
{
	int*		sv = sideVertIndex [nSide];
	short*	vp = SEGMENTS [nSegment].m_verts;

vertIndex [0] = vp [sv [0]];
vertIndex [1] = vp [sv [1]];
vertIndex [2] = vp [sv [2]];
vertIndex [3] = vp [sv [3]];
}

// ------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
//this was converted from GetSegMasks ()...it fills in an array of 6
//elements for the distace behind each CSide, or zero if not behind
//only gets centerMask, and assumes zero rad
ubyte CSegment::GetSideDists (const CFixVector& refP, fix* xSideDists, int bBehind)
{
	ubyte		mask = 0;

for (int nSide = 0; nSide < 6; nSide++)
	mask |= m_sides [nSide].Dist (refP, xSideDists [nSide], bBehind, 1 << nSide);
return mask;
}

// -------------------------------------------------------------------------------

ubyte CSegment::GetSideDistsf (const CFloatVector& refP, float* fSideDists, int bBehind)
{
	ubyte		mask = 0;

for (int nSide = 0; nSide < 6; nSide++)
	mask |= m_sides [nSide].Distf (refP, fSideDists [nSide], bBehind, 1 << nSide);
return mask;
}

// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
//	Used to become a constant based on editor, but I wanted to be able to set
//	this for omega blob FindSegByPos calls.  Would be better to pass a paremeter
//	to the routine...--MK, 01/17/96
int	bDoingLightingHack=0;

//figure out what seg the given point is in, tracing through segments
//returns CSegment number, or -1 if can't find CSegment
static int TraceSegs (const CFixVector& vPos, int nCurSeg, int nTraceDepth, char* bVisited, char bFlag, fix xTolerance = 0)
{
	CSegment*	segP;
	fix			xSideDists [6], xMaxDist;
	int			centerMask, nMaxSide, nSide, bit, nMatchSeg = -1;

if (nTraceDepth >= gameData.segs.nSegments) //gameData.segs.nSegments)
	return -1;
if (bVisited [nCurSeg] == bFlag)
	return -1;
bVisited [nCurSeg] = bFlag;
segP = SEGMENTS + nCurSeg;
if (!(centerMask = segP->GetSideDists (vPos, xSideDists, 1)))		//we're in the old segment
	return nCurSeg;		
for (;;) {
	nMaxSide = -1;
	xMaxDist = 0; // find only sides we're behind as seen from inside the current segment
	for (nSide = 0, bit = 1; nSide < 6; nSide ++, bit <<= 1)
		if ((centerMask & bit) && (xTolerance || (segP->m_children [nSide] > -1)) && (xSideDists [nSide] < xMaxDist)) {
			if (xTolerance && (xTolerance >= -xSideDists [nSide]) && (xTolerance >= segP->Side (nSide)->DistToPoint (vPos))) 
				return nCurSeg;
			if (segP->m_children [nSide] >= 0) {
				xMaxDist = xSideDists [nSide];
				nMaxSide = nSide;
				}
			}
	if (nMaxSide == -1)
		break;
	xSideDists [nMaxSide] = 0;
	if (0 <= (nMatchSeg = TraceSegs (vPos, segP->m_children [nMaxSide], nTraceDepth + 1, bVisited, bFlag, xTolerance)))	//trace into adjacent CSegment
		break;
	}
return nMatchSeg;		//we haven't found a CSegment
}

// -------------------------------------------------------------------------------

static int TraceSegsf (const CFloatVector& vPos, int nCurSeg, int nTraceDepth, char* bVisited, char bFlag, float fTolerance)
{
	CSegment*		segP;
	float				fSideDists [6], fMaxDist;
	int				centerMask, nMaxSide, nSide, bit, nMatchSeg = -1;

if (nTraceDepth >= gameData.segs.nSegments)
	return -1;
if (bVisited [nCurSeg] == bFlag)
	return -1;
bVisited [nCurSeg] = bFlag;
segP = SEGMENTS + nCurSeg;
if (!(centerMask = segP->GetSideDistsf (vPos, fSideDists, 1)))		//we're in the old CSegment
	return nCurSeg;		
for (;;) {
	nMaxSide = -1;
	fMaxDist = 0; // find only sides we're behind as seen from inside the current segment
	for (nSide = 0, bit = 1; nSide < 6; nSide ++, bit <<= 1)
		if ((centerMask & bit) && (fSideDists [nSide] < fMaxDist)) {
			if ((fTolerance >= -fSideDists [nSide])  && (fTolerance >= segP->Side (nSide)->DistToPointf (vPos))) {
#if DBG
				SEGMENTS [nCurSeg].GetSideDistsf (vPos, fSideDists, 1);
#endif
				return nCurSeg;
				}
			if (segP->m_children [nSide] >= 0) {
				fMaxDist = fSideDists [nSide];
				nMaxSide = nSide;
				}
			}
	if (nMaxSide == -1)
		break;
	fSideDists [nMaxSide] = 0;
	if (0 <= (nMatchSeg = TraceSegsf (vPos, segP->m_children [nMaxSide], nTraceDepth + 1, bVisited, bFlag, fTolerance)))	//trace into adjacent CSegment
		break;
	}
return nMatchSeg;		//we haven't found a CSegment
}

int	nExhaustiveCount=0, nExhaustiveFailedCount=0;

// -------------------------------------------------------------------------------

static inline int PointInSeg (CSegment* segP, CFixVector vPos)
{
fix d = CFixVector::Dist (vPos, segP->Center ());
if (d <= segP->m_rads [0])
	return 1;
if (d > segP->m_rads [1])
	return 0;
return (segP->Masks (vPos, 0).m_center == 0);
}

// -------------------------------------------------------------------------------

static int FindSegByPosExhaustive (const CFixVector& vPos, int bSkyBox)
{
	int			i;
	short*		segListP;
	CSegment*	segP;

if (gameData.segs.HaveGrid (bSkyBox)) {
	for (i = gameData.segs.GetSegList (vPos, segListP, bSkyBox); i; i--, segListP++) {
		if (PointInSeg (SEGMENTS + *segListP, vPos))
			return *segListP;
		}
	}
else if (bSkyBox) {
	for (i = gameData.segs.skybox.GetSegList (segListP); i; i--, segListP++) {
		segP = SEGMENTS + *segListP;
		if ((segP->m_nType == SEGMENT_IS_SKYBOX) && PointInSeg (segP, vPos))
			return *segListP;
		}
	}
else {
	segP = SEGMENTS.Buffer ();
	for (i = 0; i <= gameData.segs.nLastSegment; i++) {
		segP = SEGMENTS + i;
		if ((segP->m_nType != SEGMENT_IS_SKYBOX) && PointInSeg (segP, vPos))
			return i;
		}
	}
return -1;
}

// -------------------------------------------------------------------------------
//Find segment containing point vPos.

int FindSegByPos (const CFixVector& vPos, int nSegment, int bExhaustive, int bSkyBox, fix xTolerance, int nThread)
{
	static char	bVisited [MAX_THREADS][MAX_SEGMENTS_D2X]; 
	static char	bFlags [MAX_THREADS] = {-1, -1, -1, -1};

//allow nSegment == -1, meaning we have no idea what CSegment point is in
Assert ((nSegment <= gameData.segs.nLastSegment) && (nSegment >= -1));
if (nSegment != -1) {
	if (bFlags [nThread] < 0) {
		memset (bVisited [nThread], 0, gameData.segs.nSegments);
		bFlags [nThread] = 1;
		}
	else
		bFlags [nThread] = !bFlags [nThread];
	if (0 <= (nSegment = TraceSegs (vPos, nSegment, 0, bVisited [nThread], bFlags [nThread], xTolerance))) 
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

short FindClosestSeg (CFixVector& vPos)
{
	CSegment*	segP = SEGMENTS + 0;
	short			nSegment, nClosestSeg = -1;
	fix			nDist, nMinDist = 0x7fffffff;

for (nSegment = 0; nSegment < gameData.segs.nSegments; nSegment++, segP++) {
	nDist = CFixVector::Dist (vPos, segP->Center ()) - segP->MinRad ();
	if (nDist < nMinDist) {
		nMinDist = nDist;
		nClosestSeg = nSegment;
		}
	}
return nClosestSeg;
}

//--repair-- //	------------------------------------------------------------------------------
//--repair-- void clsd_repair_center (int nSegment)
//--repair-- {
//--repair-- 	int	nSide;
//--repair--
//--repair-- 	//	--- Set repair center bit for all repair center segments.
//--repair-- 	if (SEGMENTS [nSegment].m_nType == SEGMENT_IS_REPAIRCEN) {
//--repair-- 		Lsegments [nSegment].specialType |= SS_REPAIR_CENTER;
//--repair-- 		Lsegments [nSegment].special_segment = nSegment;
//--repair-- 	}
//--repair--
//--repair-- 	//	--- Set repair center bit for all segments adjacent to a repair center.
//--repair-- 	for (nSide=0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
//--repair-- 		int	s = SEGMENTS [nSegment].m_children [nSide];
//--repair--
//--repair-- 		if ((s != -1) && (SEGMENTS [s].m_nType == SEGMENT_IS_REPAIRCEN)) {
//--repair-- 			Lsegments [nSegment].specialType |= SS_REPAIR_CENTER;
//--repair-- 			Lsegments [nSegment].special_segment = s;
//--repair-- 		}
//--repair-- 	}
//--repair-- }

//--repair-- //	------------------------------------------------------------------------------
//--repair-- //	--- Set destination points for all Materialization centers.
//--repair-- void clsd_materialization_center (int nSegment)
//--repair-- {
//--repair-- 	if (SEGMENTS [nSegment].m_nType == SEGMENT_IS_ROBOTMAKER) {
//--repair--
//--repair-- 	}
//--repair-- }
//--repair--
//--repair-- int	Lsegment_highest_segment_index, Lsegment_highest_vertex_index;
//--repair--
//--repair-- //	------------------------------------------------------------------------------
//--repair-- //	Create data specific to mine which doesn't get written to disk.
//--repair-- //	gameData.segs.nLastSegment and gameData.objs.nLastObject [0] must be valid.
//--repair-- //	07/21:	set repair center bit
//--repair-- void create_local_segment_data (void)
//--repair-- {
//--repair-- 	int	nSegment;
//--repair--
//--repair-- 	//	--- Initialize all Lsegments.
//--repair-- 	for (nSegment=0; nSegment <= gameData.segs.nLastSegment; nSegment++) {
//--repair-- 		Lsegments [nSegment].specialType = 0;
//--repair-- 		Lsegments [nSegment].special_segment = -1;
//--repair-- 	}
//--repair--
//--repair-- 	for (nSegment=0; nSegment <= gameData.segs.nLastSegment; nSegment++) {
//--repair--
//--repair-- 		clsd_repair_center (nSegment);
//--repair-- 		clsd_materialization_center (nSegment);
//--repair--
//--repair-- 	}
//--repair--
//--repair-- 	//	Set check variables.
//--repair-- 	//	In main game loop, make sure these are valid, else Lsegments is not valid.
//--repair-- 	Lsegment_highest_segment_index = gameData.segs.nLastSegment;
//--repair-- 	Lsegment_highest_vertex_index = gameData.segs.nLastVertex;
//--repair-- }
//--repair--
//--repair-- //	------------------------------------------------------------------------------------------
//--repair-- //	Sort of makes sure create_local_segment_data has been called for the currently executing mine.
//--repair-- //	It is not failsafe, as pos [Y]u will see if pos [Y]u look at the code.
//--repair-- //	Returns 1 if Lsegments appears valid, 0 if not.
//--repair-- int check_lsegments_validity (void)
//--repair-- {
//--repair-- 	return ((Lsegment_highest_segment_index == gameData.segs.nLastSegment) && (Lsegment_highest_vertex_index == gameData.segs.nLastVertex);
//--repair-- }

#define	MAX_LOC_POINT_SEGS	64

#define	MIN_CACHE_FCD_DIST	 (I2X (80))	//	Must be this far apart for cache lookup to succeed.  Recognizes small changes in distance matter at small distances.
//	----------------------------------------------------------------------------------------------------------

void FlushFCDCache (void)
{
	int	i;

gameData.fcd.nIndex = 0;
for (i = 0; i < MAX_FCD_CACHE; i++)
	gameData.fcd.cache [i].seg0 = -1;
}

//	----------------------------------------------------------------------------------------------------------

void AddToFCDCache (int seg0, int seg1, int nDepth, fix dist)
{
if (dist > MIN_CACHE_FCD_DIST) {
	gameData.fcd.cache [gameData.fcd.nIndex].seg0 = seg0;
	gameData.fcd.cache [gameData.fcd.nIndex].seg1 = seg1;
	gameData.fcd.cache [gameData.fcd.nIndex].csd = nDepth;
	gameData.fcd.cache [gameData.fcd.nIndex].dist = dist;
	if (++gameData.fcd.nIndex >= MAX_FCD_CACHE)
		gameData.fcd.nIndex = 0;
	}
else {
	//	If it's in the cache, remove it.
	for (int i = 0; i<MAX_FCD_CACHE; i++) {
		if ((gameData.fcd.cache [i].seg0 == seg0) && (gameData.fcd.cache [i].seg1 == seg1)) {
			gameData.fcd.cache [gameData.fcd.nIndex].seg0 = -1;
			break;
			}
		}
	}
}

//	----------------------------------------------------------------------------------------------------------
//	Determine whether seg0 and seg1 are reachable in a way that allows sound to pass.
//	Search up to a maximum nDepth of nMaxDepth.
//	Return the distance.

fix FindConnectedDistance (CFixVector& p0, short nStartSeg, CFixVector& p1, short nDestSeg, int nMaxDepth, int widFlag, int bUseCache)
{
// same segment?
if (nStartSeg == nDestSeg) {
	gameData.fcd.nConnSegDist = 0;
	return CFixVector::Dist (p0, p1);
	}

// adjacent segments?
short nSide = SEGMENTS [nStartSeg].ConnectedSide (SEGMENTS + nDestSeg);
if ((nSide != -1) && (SEGMENTS [nDestSeg].IsDoorWay (nSide, NULL) & widFlag)) {
	gameData.fcd.nConnSegDist = 1;
	return CFixVector::Dist (p0, p1);
	}

#if 1

	CDialHeap	heap;
	short			nSegment;
	ushort		nDist;
	CSegment*	segP;

heap.Create (gameData.segs.nSegments);
heap.Setup (nStartSeg);
while (0 <= (nSegment = heap.Pop (nDist))) {
	if (nSegment == nDestSeg) {
		gameData.fcd.nConnSegDist = heap.BuildRoute (nDestSeg);
		int j = gameData.fcd.nConnSegDist - 2;
		short* route = heap.Route ();
		fix xDist = 0;
		for (int i = 1; i < j; i++)
			xDist += CFixVector::Dist (SEGMENTS [route [i]].Center (), SEGMENTS [route [i + 1]].Center ());
		xDist += CFixVector::Dist (p0, SEGMENTS [route [1]].Center ()) + CFixVector::Dist (p1, SEGMENTS [route [j]].Center ());
		return xDist;
		}
	else {
		segP = SEGMENTS + nSegment;
		for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
			if (segP->IsDoorWay (nSide, NULL) & widFlag)
				heap.Push (segP->m_children [nSide], nSegment, nDist + segP->m_childDists [nSide]);
			}
		}
	}
gameData.fcd.nConnSegDist = gameData.segs.nSegments + 1;
return -1;

#else

	short				nCurSeg, nParentSeg, nThisSeg;
	int				qTail = 0, qHead = 0;
	int				i, nCurDepth, nPoints;
	segQueueEntry	segmentQ [MAX_SEGMENTS_D2X];
	short				nDepth [MAX_SEGMENTS_D2X];
	tPointSeg		routeSegs [MAX_LOC_POINT_SEGS];
	fix				xDist;
	CSegment			*segP;
	tFCDCacheData	*pc;

	static sbyte	bVisited [MAX_SEGMENTS_D2X];
	static sbyte	bFlag = -1;

	//	If > this, will overrun routeSegs buffer
if (nMaxDepth > MAX_LOC_POINT_SEGS - 2) {
#if TRACE
	console.printf (1, "Warning: In FindConnectedDistance, nMaxDepth = %i, limited to %i\n", nMaxDepth, MAX_LOC_POINT_SEGS-2);
#endif
	nMaxDepth = MAX_LOC_POINT_SEGS - 2;
	}
//	Periodically flush cache.
if ((gameData.time.xGame - gameData.fcd.xLastFlushTime > I2X (2)) ||
	 (gameData.time.xGame < gameData.fcd.xLastFlushTime)) {
	FlushFCDCache ();
	gameData.fcd.xLastFlushTime = gameData.time.xGame;
	}

//	Can't quickly get distance, so see if in gameData.fcd.cache.
if (bUseCache) {
	for (i = gameData.fcd.cache.Length (), pc = gameData.fcd.cache.Buffer (); i; i--, pc++)
		if ((pc->seg0 == nStartSeg) && (pc->seg1 == nDestSeg)) {
			gameData.fcd.nConnSegDist = pc->csd;
			return pc->xDist;
			}
	}

if (bFlag < 0) {
	memset (bVisited, 0, gameData.segs.nSegments);
	bFlag = 1;
	}
else 
	bFlag = !bFlag;
memset (nDepth, 0, sizeof (nDepth [0]) * gameData.segs.nSegments);

nPoints = 0;
nCurSeg = nStartSeg;
bVisited [nCurSeg] = 1;
nCurDepth = 0;

while (nCurSeg != nDestSeg) {
	segP = SEGMENTS + nCurSeg;

	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		if (segP->IsDoorWay (nSide, NULL) & widFlag) {
			nThisSeg = segP->m_children [nSide];
			Assert ((nThisSeg >= 0) && (nThisSeg < LEVEL_SEGMENTS));
			Assert ((qTail >= 0) && (qTail < LEVEL_SEGMENTS));
			if (bVisited [nThisSeg] = bFlag) {
				segmentQ [qTail].start = nCurSeg;
				segmentQ [qTail].end = nThisSeg;
				bVisited [nThisSeg] = bFlag;
				nDepth [qTail++] = nCurDepth+1;
				if (nMaxDepth != -1) {
					if (nDepth [qTail - 1] == nMaxDepth) {
						gameData.fcd.nConnSegDist = 1000;
						AddToFCDCache (nStartSeg, nDestSeg, gameData.fcd.nConnSegDist, I2X (1000));
						return -1;
						}
					}
				else if (nThisSeg == nDestSeg) {
					goto fcd_done1;
				}
			}
		}
	}	//	for (nSide...

	if (qHead >= qTail) {
		gameData.fcd.nConnSegDist = 1000;
		AddToFCDCache (nStartSeg, nDestSeg, gameData.fcd.nConnSegDist, I2X (1000));
		return -1;
		}
	Assert ((qHead >= 0) && (qHead < LEVEL_SEGMENTS));
	nCurSeg = segmentQ [qHead].end;
	nCurDepth = nDepth [qHead];
	qHead++;

fcd_done1: ;
	}	//	while (nCurSeg ...

//	Set qTail to the CSegment which ends at the goal.
while (segmentQ [--qTail].end != nDestSeg)
	if (qTail < 0) {
		gameData.fcd.nConnSegDist = 1000;
		AddToFCDCache (nStartSeg, nDestSeg, gameData.fcd.nConnSegDist, I2X (1000));
		return -1;
		}

while (qTail >= 0) {
	nThisSeg = segmentQ [qTail].end;
	nParentSeg = segmentQ [qTail].start;
	routeSegs [nPoints].nSegment = nThisSeg;
	routeSegs [nPoints].point = SEGMENTS [nThisSeg].Center ();
	nPoints++;
	if (nParentSeg == nStartSeg)
		break;
	while (segmentQ [--qTail].end != nParentSeg)
		Assert (qTail >= 0);
	}
routeSegs [nPoints].nSegment = nStartSeg;
routeSegs [nPoints].point = SEGMENTS [nStartSeg].Center ();
nPoints++;
if (nPoints == 1) {
	gameData.fcd.nConnSegDist = nPoints;
	return CFixVector::Dist (p0, p1);
	}
else {
	xDist = CFixVector::Dist (p1, routeSegs [1].point);
	xDist += CFixVector::Dist (p0, routeSegs [nPoints-2].point);
	for (i = 1; i < nPoints - 2; i++) {
		xDist += CFixVector::Dist(routeSegs [i].point, routeSegs [i+1].point);
		}
	}
gameData.fcd.nConnSegDist = nPoints;
AddToFCDCache (nStartSeg, nDestSeg, nPoints, xDist);
return xDist;

#endif

}

// -------------------------------------------------------------------------------

sbyte convert_to_byte (fix f)
{
	if (f >= 0x00010000)
		return MATRIX_MAX;
	else if (f <= -0x00010000)
		return -MATRIX_MAX;
	else
		return (sbyte) (f >> MATRIX_PRECISION);
}

#define VEL_PRECISION 12

// -------------------------------------------------------------------------------
//	Create a tShortPos struct from an CObject.
//	Extract the matrix into byte values.
//	Create a position relative to vertex 0 with 1/256 Normal "fix" precision.
//	Stuff CSegment in a short.
void CreateShortPos (tShortPos *spp, CObject *objP, int swap_bytes)
{
	// int	nSegment;
	CFixMatrix orient = objP->info.position.mOrient;
	sbyte   *segP = spp->orient;
	CFixVector *pv;

	*segP++ = convert_to_byte(orient.RVec ()[X]);
	*segP++ = convert_to_byte(orient.UVec ()[X]);
	*segP++ = convert_to_byte(orient.FVec ()[X]);
	*segP++ = convert_to_byte(orient.RVec ()[Y]);
	*segP++ = convert_to_byte(orient.UVec ()[Y]);
	*segP++ = convert_to_byte(orient.FVec ()[Y]);
	*segP++ = convert_to_byte(orient.RVec ()[Z]);
	*segP++ = convert_to_byte(orient.UVec ()[Z]);
	*segP++ = convert_to_byte(orient.FVec ()[Z]);

	pv = gameData.segs.vertices + SEGMENTS [objP->info.nSegment].m_verts [0];
	spp->pos [X] = (short) ((objP->info.position.vPos [X] - (*pv)[X]) >> RELPOS_PRECISION);
	spp->pos [Y] = (short) ((objP->info.position.vPos [Y] - (*pv)[Y]) >> RELPOS_PRECISION);
	spp->pos [Z] = (short) ((objP->info.position.vPos [Z] - (*pv)[Z]) >> RELPOS_PRECISION);

	spp->nSegment = objP->info.nSegment;

 	spp->vel [X] = (short) ((objP->mType.physInfo.velocity[X]) >> VEL_PRECISION);
	spp->vel [Y] = (short) ((objP->mType.physInfo.velocity[Y]) >> VEL_PRECISION);
	spp->vel [Z] = (short) ((objP->mType.physInfo.velocity[Z]) >> VEL_PRECISION);

// swap the short values for the big-endian machines.

	if (swap_bytes) {
		spp->pos [X] = INTEL_SHORT (spp->pos [X]);
		spp->pos [Y] = INTEL_SHORT (spp->pos [Y]);
		spp->pos [Z] = INTEL_SHORT (spp->pos [Z]);
		spp->nSegment = INTEL_SHORT (spp->nSegment);
		spp->vel [X] = INTEL_SHORT (spp->vel [X]);
		spp->vel [Y] = INTEL_SHORT (spp->vel [Y]);
		spp->vel [Z] = INTEL_SHORT (spp->vel [Z]);
	}
}

// -------------------------------------------------------------------------------

void ExtractShortPos (CObject *objP, tShortPos *spp, int swap_bytes)
{
	int	nSegment;
	sbyte   *segP;
	CFixVector *pv;

	segP = spp->orient;

	objP->info.position.mOrient.RVec ()[X] = *segP++ << MATRIX_PRECISION;
	objP->info.position.mOrient.UVec ()[X] = *segP++ << MATRIX_PRECISION;
	objP->info.position.mOrient.FVec ()[X] = *segP++ << MATRIX_PRECISION;
	objP->info.position.mOrient.RVec ()[Y] = *segP++ << MATRIX_PRECISION;
	objP->info.position.mOrient.UVec ()[Y] = *segP++ << MATRIX_PRECISION;
	objP->info.position.mOrient.FVec ()[Y] = *segP++ << MATRIX_PRECISION;
	objP->info.position.mOrient.RVec ()[Z] = *segP++ << MATRIX_PRECISION;
	objP->info.position.mOrient.UVec ()[Z] = *segP++ << MATRIX_PRECISION;
	objP->info.position.mOrient.FVec ()[Z] = *segP++ << MATRIX_PRECISION;

	if (swap_bytes) {
		spp->pos [X] = INTEL_SHORT (spp->pos [X]);
		spp->pos [Y] = INTEL_SHORT (spp->pos [Y]);
		spp->pos [Z] = INTEL_SHORT (spp->pos [Z]);
		spp->nSegment = INTEL_SHORT (spp->nSegment);
		spp->vel [X] = INTEL_SHORT (spp->vel [X]);
		spp->vel [Y] = INTEL_SHORT (spp->vel [Y]);
		spp->vel [Z] = INTEL_SHORT (spp->vel [Z]);
	}

	nSegment = spp->nSegment;

	Assert ((nSegment >= 0) && (nSegment <= gameData.segs.nLastSegment));

	pv = gameData.segs.vertices + SEGMENTS [nSegment].m_verts [0];
	objP->info.position.vPos [X] = (spp->pos [X] << RELPOS_PRECISION) + (*pv)[X];
	objP->info.position.vPos [Y] = (spp->pos [Y] << RELPOS_PRECISION) + (*pv)[Y];
	objP->info.position.vPos [Z] = (spp->pos [Z] << RELPOS_PRECISION) + (*pv)[Z];

	objP->mType.physInfo.velocity[X] = (spp->vel [X] << VEL_PRECISION);
	objP->mType.physInfo.velocity[Y] = (spp->vel [Y] << VEL_PRECISION);
	objP->mType.physInfo.velocity[Z] = (spp->vel [Z] << VEL_PRECISION);

	objP->RelinkToSeg (nSegment);

}

//--unused-- void test_shortpos (void)
//--unused-- {
//--unused-- 	tShortPos	spp;
//--unused--
//--unused-- 	CreateShortPos (&spp, &OBJECTS [0]);
//--unused-- 	ExtractShortPos (&OBJECTS [0], &spp);
//--unused--
//--unused-- }

//	-----------------------------------------------------------------------------
//	Segment validation functions.
//	Moved from editor to game so we can compute surface normals at load time.
// -------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------
//	Extract a vector from a CSegment.  The vector goes from the start face to the end face.
//	The point on each face is the average of the four points forming the face.
void extract_vector_from_segment (CSegment *segP, CFixVector *vp, int start, int end)
{
	int			i;
	CFixVector	vs, ve;

	vs.SetZero ();
	ve.SetZero ();

	for (i=0; i<4; i++) {
		vs += gameData.segs.vertices [segP->m_verts [sideVertIndex [start][i]]];
		ve += gameData.segs.vertices [segP->m_verts [sideVertIndex [end][i]]];
	}

	*vp = ve - vs;
	*vp *= (I2X (1)/4);

}

// -------------------------------------------------------------------------------
//create a matrix that describes the orientation of the given CSegment
void ExtractOrientFromSegment (CFixMatrix *m, CSegment *seg)
{
	CFixVector fVec, uVec;

	extract_vector_from_segment (seg, &fVec, WFRONT, WBACK);
	extract_vector_from_segment (seg, &uVec, WBOTTOM, WTOP);

	//vector to matrix does normalizations and orthogonalizations
	*m = CFixMatrix::CreateFU(fVec, uVec);
//	*m = CFixMatrix::CreateFU(fVec, &uVec, NULL);
}

// -------------------------------------------------------------------------------
//	Return v0, v1, v2 = 3 vertices with smallest numbers.  If *bFlip set, then negate Normal after computation.
//	Note, pos [Y]u cannot just compute the Normal by treating the points in the opposite direction as this introduces
//	small differences between normals which should merely be opposites of each other.
short GetVertsForNormal (short v0, short v1, short v2, short v3, short* vSorted)
{
	int		i, j;
	ushort	index [4] = {0, 1, 2, 3};

//	index is a list that shows how things got scrambled so we know if our Normal is pointing backwards
vSorted [0] = v0;
vSorted [1] = v1;
vSorted [2] = v2;
vSorted [3] = v3;
// bubble sort vSorted in reverse order (largest first)
for (i = 1; i < 4; i++)
	for (j = 0; j < i; j++)
		if (vSorted [j] > vSorted [i]) {
			Swap (vSorted [i], vSorted [j]);
			Swap (index [i], index [j]);
			}

Assert ((vSorted [0] < vSorted [1]) && (vSorted [1] < vSorted [2]) && (vSorted [2] < vSorted [3]));
//	Now, if for any index [i] & index [i+1]: index [i+1] = (index [i]+3)%4, then must flip Normal
return (((index [0] + 3) % 4) == index [1]) || (((index [1] + 3) % 4) == index [2]);
}

// -------------------------------------------------------------------------------

void AddToVertexNormal (int nVertex, CFixVector& vNormal)
{
	g3sNormal	*pn = &gameData.segs.points [nVertex].p3_normal;

#if DBG
if (nVertex == nDbgVertex)
	nDbgVertex = nDbgVertex;
#endif
pn->nFaces++;
pn->vNormal += vNormal;
}

// -------------------------------------------------------------------------------
//	Set up all segments.
//	gameData.segs.nLastSegment must be set.
//	For all used segments (number <= gameData.segs.nLastSegment), nSegment field must be != -1.

void SetupSegments (void)
{
gameOpts->render.nMathFormat = 0;
gameData.segs.points.Clear ();
for (int i = 0; i <= gameData.segs.nLastSegment; i++)
	SEGMENTS [i].Setup ();
ComputeVertexNormals ();
gameOpts->render.nMathFormat = gameOpts->render.nDefMathFormat;
}

//	-----------------------------------------------------------------------------
//	Set the segment depth of all segments from nStartSeg in *segbuf.
//	Returns maximum nDepth value.
int SetSegmentDepths (int nStartSeg, ushort *pDepthBuf)
{
	ubyte		bVisited [MAX_SEGMENTS_D2X];
	short		queue [MAX_SEGMENTS_D2X];
	int		head = 0;
	int		tail = 0;
	int		nDepth = 1;
	int		nSegment, nSide, nChild;
	ushort	nParentDepth = 0;
	short*	childP;

	head = 0;
	tail = 0;

if ((nStartSeg < 0) || (nStartSeg >= gameData.segs.nSegments))
	return 1;
if (pDepthBuf [nStartSeg] == 0)
	return 1;
queue [tail++] = nStartSeg;
memset (bVisited, 0, sizeof (*bVisited) * gameData.segs.nSegments);
bVisited [nStartSeg] = 1;
pDepthBuf [nStartSeg] = nDepth++;
if (nDepth == 0)
	nDepth = 0x7fff;
while (head < tail) {
	nSegment = queue [head++];
#if DBG
	if (nSegment == nDbgSeg)
		nDbgSeg = nDbgSeg;
#endif
	nParentDepth = pDepthBuf [nSegment];
	childP = SEGMENTS [nSegment].m_children;
	for (nSide = MAX_SIDES_PER_SEGMENT; nSide; nSide--, childP++) {
		if (0 > (nChild = *childP))
			continue;
#if DBG
		if (nChild >= gameData.segs.nSegments) {
			Error ("Invalid segment in SetSegmentDepths()\nsegment=%d, side=%d, child=%d",
					 nSegment, nSide, nChild);
			return 1;
			}
#endif
#if DBG
		if (nChild == nDbgSeg)
			nDbgSeg = nDbgSeg;
#endif
		if (!pDepthBuf [nChild])
			continue;
		if (bVisited [nChild])
			continue;
		bVisited [nChild] = 1;
		pDepthBuf [nChild] = nParentDepth + 1;
		queue [tail++] = nChild;
		}
	}
return (nParentDepth + 1) * gameStates.render.bViewDist;
}


//	-----------------------------------------------------------------------------

fix FindConnectedDistanceSegments (short seg0, short seg1, int nDepth, int widFlag)
{
	CFixVector	p0, p1;

p0 = SEGMENTS [seg0].Center ();
p1 = SEGMENTS [seg1].Center ();
return FindConnectedDistance (p0, seg0, p1, seg1, nDepth, widFlag, 0);
}

#define	AMBIENT_SEGMENT_DEPTH		5

//	-----------------------------------------------------------------------------
//	Do a bfs from nSegment, marking slots in markedSegs if the segment is reachable.
void AmbientMarkBfs (short nSegment, sbyte* markedSegs, int nDepth)
{
	short	i, child;

if (nDepth < 0)
	return;
markedSegs [nSegment] = 1;
for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	child = SEGMENTS [nSegment].m_children [i];
	if (IS_CHILD (child) && (SEGMENTS [nSegment].IsDoorWay (i, NULL) & WID_RENDPAST_FLAG) && !markedSegs [child])
		AmbientMarkBfs (child, markedSegs, nDepth - 1);
	}
}

//	-----------------------------------------------------------------------------
//	Indicate all segments which are within audible range of falling water or lava,
//	and so should hear ambient gurgles.
void SetAmbientSoundFlagsCommon (int tmi_bit, int s2f_bit)
{
	short		i, j;

	static sbyte	markedSegs [MAX_SEGMENTS_D2X];

	//	Now, all segments containing ambient lava or water sound makers are flagged.
	//	Additionally flag all segments which are within range of them.
memset (markedSegs, 0, sizeof (markedSegs));
for (i = 0; i <= gameData.segs.nLastSegment; i++) {
	SEGMENTS [i].m_flags &= ~s2f_bit;
	}

//	Mark all segments which are sources of the sound.
CSegment	*segP = SEGMENTS.Buffer ();
for (i = 0; i <= gameData.segs.nLastSegment; i++, segP++) {
	CSide	*sideP = segP->m_sides;
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++, sideP++) {
		if ((gameData.pig.tex.tMapInfoP [sideP->m_nBaseTex].flags & tmi_bit) ||
			 (gameData.pig.tex.tMapInfoP [sideP->m_nOvlTex].flags & tmi_bit)) {
			if (!IS_CHILD (segP->m_children [j]) || IS_WALL (sideP->m_nWall)) {
				segP->m_flags |= s2f_bit;
				markedSegs [i] = 1;		//	Say it's itself that it is close enough to to hear something.
				}
			}
		}
	}
//	Next mark all segments within N segments of a source.
segP = SEGMENTS.Buffer ();
for (i = 0; i <= gameData.segs.nLastSegment; i++, segP++) {
	if (segP->m_flags & s2f_bit)
		AmbientMarkBfs (i, markedSegs, AMBIENT_SEGMENT_DEPTH);
	}
//	Now, flip bits in all segments which can hear the ambient sound.
for (i = 0; i <= gameData.segs.nLastSegment; i++)
	if (markedSegs [i])
		SEGMENTS [i].m_flags |= s2f_bit;
}

//	-----------------------------------------------------------------------------
//	Indicate all segments which are within audible range of falling water or lava,
//	and so should hear ambient gurgles.

void SetAmbientSoundFlags (void)
{
SetAmbientSoundFlagsCommon (TMI_VOLATILE, S2F_AMBIENT_LAVA);
SetAmbientSoundFlagsCommon (TMI_WATER, S2F_AMBIENT_WATER);
}

// -------------------------------------------------------------------------------

float FaceSize (short nSegment, ubyte nSide)
{
	CSegment*	segP = SEGMENTS + nSegment;
	int*			s2v = sideVertIndex [nSide];

	short			v0 = segP->m_verts [s2v [0]];
	short			v1 = segP->m_verts [s2v [1]];
	short			v2 = segP->m_verts [s2v [2]];
	short			v3 = segP->m_verts [s2v [3]];

return TriangleSize (gameData.segs.vertices [v0], gameData.segs.vertices [v1], gameData.segs.vertices [v2]) +
		 TriangleSize (gameData.segs.vertices [v0], gameData.segs.vertices [v2], gameData.segs.vertices [v3]);
}

// -------------------------------------------------------------------------------

void ComputeVertexNormals (void)
{
	int		h, i;
	g3sPoint	*pp;

for (i = gameData.segs.nVertices, pp = gameData.segs.points.Buffer (); i; i--, pp++) {
	if (1 < (h = pp->p3_normal.nFaces)) {
		pp->p3_normal.vNormal /= (float) h;
		/*
		pp->p3_normal.vNormal[Y] /= h;
		pp->p3_normal.vNormal[Z] /= h;
		*/
		CFloatVector::Normalize (pp->p3_normal.vNormal);
		}
	pp->p3_normal.nFaces = 1;
	}
}

// -------------------------------------------------------------------------------

void ResetVertexNormals (void)
{
	int		i;
	g3sPoint	*pp;

for (i = gameData.segs.nVertices, pp = gameData.segs.points.Buffer (); i; i--, pp++) {
	pp->p3_normal.vNormal.SetZero ();
	pp->p3_normal.nFaces = 0;
	}
}

//	-----------------------------------------------------------------------------
//eof
