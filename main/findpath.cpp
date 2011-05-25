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

#include "descent.h"
#include "error.h"
#include "segment.h"
#include "segmath.h"
#include "findpath.h"
#include "byteswap.h"

#define USE_DACS 0
#define USE_FCD_CACHE 1
#define BIDIRECTIONAL_SCAN 1
#define MULTITHREADED_SCAN 0

#if DBG
#	define DBG_SCAN 0
#else
#	define DBG_SCAN 0
#endif

#if USE_DACS
#	include "dialheap.h"
#endif

CSimpleBiDirRouter simpleRouter;
CDACSUniDirRouter dacsRouter;

// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------

#define	MIN_CACHE_FCD_DIST	 (I2X (80))	//	Must be this far apart for cache lookup to succeed.  Recognizes small changes in distance matter at small distances.

void CFCDCache::Flush (void)
{
m_nIndex = 0;
for (int i = 0; i < MAX_FCD_CACHE; i++)
	m_cache [i].seg0 = -1;
}

// -----------------------------------------------------------------------------------

void CFCDCache::Add (int seg0, int seg1, int nPathLen, fix dist)
{
if (dist > MIN_CACHE_FCD_DIST) {
	m_cache [m_nIndex].seg0 = seg0;
	m_cache [m_nIndex].seg1 = seg1;
	m_cache [m_nIndex].pathLen = nPathLen;
	m_cache [m_nIndex].dist = dist;
	if (++m_nIndex >= MAX_FCD_CACHE)
		m_nIndex = 0;
	SetPathLength (nPathLen);
	}
else {
	//	If it's in the cache, remove it.
	for (int i = 0; i < MAX_FCD_CACHE; i++) {
		if ((m_cache [i].seg0 == seg0) && (m_cache [i].seg1 == seg1)) {
			m_cache [m_nIndex].seg0 = -1;
			break;
			}
		}
	}
}

// -----------------------------------------------------------------------------------

fix CFCDCache::Dist (short seg0, short seg1)
{
	tFCDCacheData*	pc = m_cache.Buffer ();

for (int i = int (m_cache.Length ()); i; i--, pc++) {
	if ((pc->seg0 == seg0) && (pc->seg1 == seg1)) {
		SetPathLength (pc->pathLen);
		return pc->dist;
		}
	}
return -1;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int CScanInfo::Setup (CSimpleHeap* heap, int nWidFlag, int nMaxDepth)
{
m_nLinkSeg = 0;
m_bScanning = 3;
m_heap = heap;
m_widFlag = nWidFlag;
m_maxDepth = (m_maxDepth < 0) ? gameData.segs.nSegments : nMaxDepth;
if (!++m_bFlag)
	m_bFlag = 1;
return m_bFlag;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CSimpleHeap::Setup (short nStartSeg, short nDestSeg, uint flag, int dir)
{
m_path [nStartSeg].m_bVisited = flag;
m_path [nStartSeg].m_nDepth = 0;
m_path [nStartSeg].m_nPred = -1;
m_nStartSeg =
m_queue [0] = nStartSeg;
m_nDestSeg = nDestSeg;
m_nTail = 0;
m_nHead = 1;
m_nDir = dir;
m_nLinkSeg = 0;
}

//	-----------------------------------------------------------------------------

short CSimpleHeap::Expand (CScanInfo& scanInfo)
{
if (m_nTail >= m_nHead)
	return m_nLinkSeg = -1;
short nPredSeg = m_queue [m_nTail++];

#if DBG_SCAN
if (nPredSeg == nDbgSeg)
	nDbgSeg = nDbgSeg;
#endif

short m_nDepth = m_path [nPredSeg].m_nDepth + 1;
if (m_nDepth > scanInfo.m_maxDepth)
	return m_nLinkSeg = scanInfo.Scanning (m_nDir) ? 0 : -1;

CSegment* segP = SEGMENTS + nPredSeg;
for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	short nSuccSeg = segP->m_children [nSide];
	if (nSuccSeg < 0)
		continue;
	if (m_nDir) {
		CSegment* otherSegP = SEGMENTS + nSuccSeg;
		short nOtherSide = SEGMENTS [nPredSeg].ConnectedSide (otherSegP);
		if ((nOtherSide == -1) || !(otherSegP->IsDoorWay (nOtherSide, NULL) & scanInfo.m_widFlag))
			continue;
		}
	else {
		if (!(segP->IsDoorWay (nSide, NULL) & scanInfo.m_widFlag))
			continue;
		}

#if DBG_SCAN
	if (nSuccSeg == nDbgSeg)
		nDbgSeg = nDbgSeg;
#endif
	CPathNode& pathNode = m_path [nSuccSeg];
	if (pathNode.m_bVisited == scanInfo.m_bFlag)
		continue;
	pathNode.m_nPred = nPredSeg;
	if (Match (nSuccSeg, scanInfo))
		return m_nLinkSeg = nSuccSeg + 1;
	pathNode.m_bVisited = scanInfo.m_bFlag;
	pathNode.m_nDepth = m_nDepth;
	m_queue [m_nHead++] = nSuccSeg;
	}
return 0;
}

// -----------------------------------------------------------------------------

bool CSimpleUniDirHeap::Match (short nSegment, CScanInfo& scanInfo)
{
return (nSegment == m_nDestSeg);
}

// -----------------------------------------------------------------------------

bool CSimpleBiDirHeap::Match (short nSegment, CScanInfo& scanInfo)
{
return (scanInfo.m_heap [!m_nDir].m_path [nSegment].m_bVisited == m_bFlag);
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------

//static SDL_mutex* semaphore = NULL;

#if MULTITHREADED_SCAN

int _CDECL_ ExpandSegmentMT (void* nThreadP)
{
	int nDir = *((int *) nThreadP);

while (!(ExpandSegment (nDir) || m_heap [!nDir].nLinkSeg))
	;
return 1;
}

#endif

// -----------------------------------------------------------------------------
//	Determine whether seg0 and seg1 are reachable in a way that allows sound to pass.
//	Search up to a maximum m_nDepth of m_maxDepth.
//	Return the distance.

fix CRouter::PathLength (CFixVector& p0, short nStartSeg, CFixVector& p1, short nDestSeg, int nMaxDepth, int nWidFlag, int nCacheType)
{
#if 0 //DBG
//if (!m_cacheType) 
	{
	m_cache [m_cacheType].SetPathLength (10000);
	return -1;
	}
#endif

m_p0 = p0;
m_p1 = p1;

m_nStartSeg = nStartSeg;
if (m_nStartSeg < 0) {
	m_nStartSeg = FindSegByPos (m_p0, 0, 1, 0);
	if (m_nStartSeg < 0)
		return -1;
	}
m_nDestSeg = nDestSeg;
if (m_nDestSeg < 0) {
	m_nDestSeg = FindSegByPos (m_p0, 0, 1, 0);
	if (m_nDestSeg < 0)
		return -1;
	}

// same segment?
m_cacheType = nCacheType;
if ((m_cacheType >= 0) && (m_nStartSeg == m_nDestSeg)) {
	m_cache [m_cacheType].SetPathLength (0);
	return CFixVector::Dist (m_p0, m_p1);
	}

// adjacent segments?
short nSide = SEGMENTS [m_nStartSeg].ConnectedSide (SEGMENTS + m_nDestSeg);
if ((nSide != -1) && (SEGMENTS [m_nDestSeg].IsDoorWay (nSide, NULL) & m_widFlag)) {
	m_cache [m_cacheType].SetPathLength (1);
	return CFixVector::Dist (m_p0, m_p1);
	}

#if USE_FCD_CACHE
if (m_cacheType >= 0) {
	fix xDist = m_cache [m_cacheType].Dist (m_nStartSeg, m_nDestSeg);
	if (xDist >= 0)
		return xDist;
	}
#endif

m_maxDepth = nMaxDepth;

fix distance = Scan ();

if ((distance < 0) && (m_cacheType >= 0))
	m_cache [m_cacheType].Add (m_nStartSeg, m_nDestSeg, 10000, I2X (10000));

return distance;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

fix CSimpleRouter::Scan (void)
{
FindPath ();

return -1;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// uni-directional scanner

fix CSimpleUniDirRouter::BuildPath (void)
{
	fix	xDist;
	int	nLength = 0; 
	short	nPredSeg, nSuccSeg = m_nDestSeg;

nPredSeg = --m_scanInfo.m_nLinkSeg;
xDist = CFixVector::Dist (m_p1, SEGMENTS [nPredSeg].Center ());
for (;;) {
	nPredSeg = m_heap.m_path [nSuccSeg].m_nPred;
	if (nPredSeg == m_heap.m_nStartSeg)
		break;
	nLength++;
	xDist += CFixVector::Dist (SEGMENTS [nPredSeg].Center (), SEGMENTS [nSuccSeg].Center ());
	nSuccSeg = nPredSeg;
	}
xDist += CFixVector::Dist (m_p0, SEGMENTS [nSuccSeg].Center ());
if (m_cacheType >= 0) 
	m_cache [m_cacheType].Add (m_heap.m_nStartSeg, m_heap.m_nDestSeg, nLength + 3, xDist);
return xDist;
}

// -----------------------------------------------------------------------------

fix CSimpleUniDirRouter::FindPath (void)
{
m_scanInfo.Setup (&m_heap, m_widFlag, m_maxDepth);
m_heap.Setup (m_nStartSeg, m_nDestSeg, 0, m_scanInfo.m_bFlag);

for (;;) {
	if (0 > (m_scanInfo.m_nLinkSeg = m_heap.Expand (m_scanInfo)))
		return -1;
	if (m_scanInfo.m_nLinkSeg > 0) // destination segment reached
		return BuildPath ();
	}	
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

fix CSimpleBiDirRouter::BuildPath (void)
{
	fix	xDist = 0;
	int	nLength = 0; 
	short	nPredSeg, nSuccSeg;

--m_scanInfo.m_nLinkSeg;
for (int nDir = 0; nDir < 2; nDir++) {
	CSimpleHeap& heap = m_heap [nDir];
	nSuccSeg = m_scanInfo.m_nLinkSeg;
	for (;;) {
		nPredSeg = heap.m_path [nSuccSeg].m_nPred;
		if (nPredSeg == heap.m_nStartSeg) {
			xDist += CFixVector::Dist (nDir ? m_p1 : m_p0, SEGMENTS [nSuccSeg].Center ());
			break;
			}
		nLength++;
		if (nLength > 2 * m_scanInfo.m_maxDepth + 2)
			return -0x7FFFFFFF;
		xDist += CFixVector::Dist (SEGMENTS [nPredSeg].Center (), SEGMENTS [nSuccSeg].Center ());
		nSuccSeg = nPredSeg;
		}
	}
if (m_cacheType >= 0) 
	m_cache [m_cacheType].Add (m_heap [0].m_nStartSeg, m_heap [0].m_nDestSeg, nLength + 3, xDist);
return xDist;
}

// -----------------------------------------------------------------------------

fix CSimpleBiDirRouter::FindPath (void)
{
m_scanInfo.Setup (m_heap, m_widFlag, m_maxDepth);

m_heap [0].Setup (m_nStartSeg, m_nDestSeg, 0, m_scanInfo.m_bFlag);
m_heap [1].Setup (m_nDestSeg, m_nStartSeg, 1, m_scanInfo.m_bFlag);

#if MULTITHREADED_SCAN
if (gameStates.app.nThreads > 1) {
	SDL_Thread* threads [2];
	int nThreadIds [2] = {0, 1};
	threads [0] = SDL_CreateThread (ExpandSegmentMT, nThreadIds);
	threads [1] = SDL_CreateThread (ExpandSegmentMT, nThreadIds + 1);
	SDL_WaitThread (threads [0], NULL);
	SDL_WaitThread (threads [1], NULL);

	if (((0 < (m_scanInfo.m_nLinkSeg = m_heap [0].nLinkSeg)) && (m_scanInfo.m_nLinkSeg != m_heap [0].m_nDestSeg + 1)) || 
		 ((0 < (m_scanInfo.m_nLinkSeg = m_heap [1].nLinkSeg)) && (m_scanInfo.m_nLinkSeg != m_heap [1].m_nDestSeg + 1)))
		return BuildPathBiDir (m_p0, m_p1, m_cacheType);
	}
else 
#endif
	{
	for (;;) {
		for (int nDir = 0; nDir < 2; nDir++) {
			if (!(m_scanInfo.m_nLinkSeg = m_heap [nDir].Expand (m_scanInfo))) // nLinkSeg == 0 -> keep expanding
				continue;
			if (m_scanInfo.m_nLinkSeg < 0)
				return -1;
			// destination segment reached
			return BuildPath ();
			}
		}	
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

fix CDACSUniDirRouter::BuildPath (short nSegment)
{
int j = m_heap.BuildRoute (nSegment) - 2;
short* route = m_heap.Route ();
fix xDist = 0;
for (int i = 1; i < j; i++)
	xDist += CFixVector::Dist (SEGMENTS [route [i]].Center (), SEGMENTS [route [i + 1]].Center ());
xDist += CFixVector::Dist (m_p0, SEGMENTS [route [1]].Center ()) + CFixVector::Dist (m_p1, SEGMENTS [route [j]].Center ());
if (m_cacheType >= 0) 
	m_cache [m_cacheType].Add (m_nStartSeg, m_nDestSeg, j + 2, xDist);
return xDist;
}

// -----------------------------------------------------------------------------

fix CDACSUniDirRouter::FindPath (void)
{
	ushort		nDist;
	short			nSegment, nSide;
	CSegment*	segP;

m_heap.Setup (m_nStartSeg);
#if DBG
int nExpanded = 0;
#endif
for (;;) {
	nSegment = m_heap.Pop (nDist);
	if (nSegment < 0)
		return -1;
	if (nSegment == m_nDestSeg)
		return BuildPath (nSegment);
#if DBG
	nExpanded++;
#endif
	segP = SEGMENTS + nSegment;
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		if ((segP->m_children [nSide] >= 0) && (segP->IsDoorWay (nSide, NULL) & m_widFlag))
			m_heap.Push (segP->m_children [nSide], nSegment, nDist + segP->m_childDists [nSide]);
		}
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

int CDACSBiDirRouter::Expand (int nDir)
{
	ushort nDist;

short nSegment = m_heap [nDir].Pop (nDist);
if ((nSegment < 0) || (nSegment == m_nDestSeg))
	return nSegment;
if (m_heap [!nDir].Popped (nSegment))
	return nSegment;
CSegment* segP = SEGMENTS + nSegment;
for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	if ((segP->m_children [nSide] >= 0) && (segP->IsDoorWay (nSide, NULL) & m_widFlag))
		m_heap [nDir].Push (segP->m_children [nSide], nSegment, nDist + segP->m_childDists [nSide]);
	}
return nSegment;
}

// -----------------------------------------------------------------------------

fix CDACSBiDirRouter::BuildPath (short nSegment)
{
	int j = -2;

if (m_nSegments [0] >= 0)
	j += m_heap [0].BuildRoute (nSegment, 0, m_route) - 1;
if (m_nSegments [1] >= 0)
	j += m_heap [1].BuildRoute (nSegment, 1, m_route + j);
fix xDist = 0;
for (int i = 1; i < j; i++)
	xDist += CFixVector::Dist (SEGMENTS [m_route [i]].Center (), SEGMENTS [m_route [i + 1]].Center ());
xDist += CFixVector::Dist (m_p0, SEGMENTS [m_route [1]].Center ()) + CFixVector::Dist (m_p1, SEGMENTS [m_route [j]].Center ());
if (m_cacheType >= 0) 
	m_cache [m_cacheType].Add (m_nStartSeg, m_nDestSeg, j + 2, xDist);
return xDist;
}

// -----------------------------------------------------------------------------

fix CDACSBiDirRouter::FindPath (void)
{
	short	nSegment;

m_heap [0].Setup (m_nSegments [0] = m_nStartSeg);
m_heap [1].Setup (m_nSegments [1] = m_nDestSeg);

for (;;) {
	if (m_nSegments [0] >= 0) {
		m_nSegments [0] = Expand (0);
		}
	if (m_nSegments [1] >= 0) {
		m_nSegments [1] = Expand (1);
		}
	if ((m_nSegments [0] < 0) && (m_nSegments [1] < 0))
	if (m_nSegments [0] == m_nDestSeg) {
		nSegment = m_nSegments [0];
		m_nSegments [1] = -1;
		}
	else if (m_nSegments [1] == m_nStartSeg) {
		nSegment = m_nSegments [1];
		m_nSegments [0] = -1;
		}
	else if ((m_nSegments [1] >= 0) && m_heap [0].Popped (m_nSegments [1]))
		nSegment = m_nSegments [1];
	else if ((m_nSegments [0] >= 0) && m_heap [1].Popped (m_nSegments [0]))
		nSegment = m_nSegments [0];
	else
		continue;
	return BuildPath (nSegment);
	}
}

//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//	-----------------------------------------------------------------------------
//eof
