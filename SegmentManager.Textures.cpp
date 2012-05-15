// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

extern short nDbgSeg, nDbgSide;
extern int nDbgVertex;

// -----------------------------------------------------------------------------

void CSegmentManager::Textures (CSideKey key, short& nBaseTex, short& nOvlTex)
{
current->Get (key);
Side (key)->GetTextures (nBaseTex, nOvlTex);
}

// -----------------------------------------------------------------------------

bool CSegmentManager::SetTextures (CSideKey key, int nBaseTex, int nOvlTex)
{
	bool bChange = false;

undoManager.Begin (udSegments); 
current->Get (key); 
CSide *sideP = Side (key); 
bChange = sideP->SetTextures (nBaseTex, nOvlTex);
if (!bChange) {
	undoManager.End ();
	return false;
	}
if ((lightManager.IsLight (sideP->BaseTex ()) == -1) && (lightManager.IsLight (sideP->OvlTex (0)) == -1))
	lightManager.DeleteVariableLight (key); 
if (!wallManager.ClipFromTexture (key))
	wallManager.CheckForDoor (key); 
undoManager.End (); 
sprintf_s (message, sizeof (message), "side has textures %d, %d", sideP->BaseTex () & 0x1fff, sideP->OvlTex (0)); 
INFOMSG (message); 
return true;
}

// ------------------------------------------------------------------------ 

int CSegmentManager::AlignTextures (short nStartSeg, short nStartSide, short nChildSeg, short nChildSide, int bAlign1st, int bAlign2nd)
{
if ((nStartSeg < 0) || (nStartSeg >= Count ()))
	return -1;
if ((nStartSide < 0) || (nStartSide >= 6))
	return -1;
if ((nChildSeg < 0) || (nChildSeg >= Count ()))
	return -1;
if ((nChildSide < 0) || (nChildSide >= 6))
	return -1;

#ifdef _DEBUG
if ((nStartSeg == nDbgSeg) && ((nDbgSide < 0) || (nStartSide == nDbgSide)))
	nDbgSeg = nDbgSeg;
#endif

CSegment*	segP = Segment (nStartSeg); 
CSide*		sideP = segP->m_sides + nStartSide; 

if (sideP->Shape () > SIDE_SHAPE_TRIANGLE)
	return -1;

CSegment*	childSegP = Segment (nChildSeg); 
CSide*		childSideP = childSegP->m_sides + nChildSide; 
short			nEdgeCount = sideP->VertexCount ();
short			nChildEdgeCount = childSideP->VertexCount ();

short			i; 
short			nEdge, nChildEdge; 

// ignore children with no texture
if (!IsWall (CSideKey (nChildSeg, nChildSide)))
	return -1;

// find matching lines first
for (nEdge = 0; nEdge < nEdgeCount; nEdge++) {
	// find vert numbers for the line's two end points
	ushort vert0 = segP->VertexId (nStartSide, nEdge); 
	ushort vert1 = segP->VertexId (nStartSide, nEdge + 1); // the index (here: nEdge + 1) gets automatically wrapped by this function

	for (nChildEdge = 0; nChildEdge < nChildEdgeCount; nChildEdge++) {
		// find vert numbers for the line's two end points
		ushort nChildVert0 = childSegP->VertexId (nChildSide, nChildEdge);
		ushort nChildVert1 = childSegP->VertexId (nChildSide, nChildEdge + 1);
		// if points of child's line == corresponding points of parent
		if (((nChildVert0 == vert1) && (nChildVert1 == vert0)) || ((nChildVert0 == vert0) && (nChildVert1 == vert1)))
			break;
		}
	if (nChildEdge != nChildEdgeCount)
		break;
	}
if ((nEdge == nEdgeCount) || (nChildEdge == nChildEdgeCount))
	return -1;

// now we know the child's side & line which touches the parent
// child:  nChildSeg, nChildSide, nChildEdge, nChildPoint0, nChildPoint1
// parent: nStartSeg, nStartSide, nEdge, point0, point1
// we can start undoManager now - since we know we are making changes now and won't abort inside this block
undoManager.Begin (udSegments);

double angle;

if (bAlign1st) {
	CUVL uv;
	// now translate all the childs (u, v) coords so the reference point is at zero
	uv = childSideP->m_info.uvls [(nChildEdge + 1) % nChildEdgeCount]; 
	for (i = 0; i < nChildEdgeCount; i++)
		childSideP->m_info.uvls [i] -= uv; 
	// find difference between parent point0 and child point1
	uv = childSideP->m_info.uvls [(nChildEdge + 1) % nChildEdgeCount] - sideP->m_info.uvls [nEdge]; 
	// find the angle formed by the two lines
	double sAngle = atan3 (sideP->m_info.uvls [(nEdge + 1) % nEdgeCount].v - sideP->m_info.uvls [nEdge].v, 
								  sideP->m_info.uvls [(nEdge + 1) % nEdgeCount].u - sideP->m_info.uvls [nEdge].u); 
	double cAngle = atan3 (childSideP->m_info.uvls [nChildEdge].v - childSideP->m_info.uvls [(nChildEdge + 1) % nChildEdgeCount].v, 
					 			  childSideP->m_info.uvls [nChildEdge].u - childSideP->m_info.uvls [(nChildEdge + 1) % nChildEdgeCount].u); 
	// now rotate childs (u, v) coords around child_point1 (cAngle - sAngle)
	angle = cAngle - sAngle;
	for (i = 0; i < nChildEdgeCount; i++) 
		childSideP->m_info.uvls [i].Rotate (angle); 
	// now translate all the childs (u, v) coords to parent point0
	for (i = 0; i < nChildEdgeCount; i++)
		childSideP->m_info.uvls [i] -= uv; 
	}
if (bAlign2nd && sideP->OvlTex (0) && childSideP->OvlTex (0)) {
	int r, h = sideP->OvlAlignment ();
	if (h == 3)
		r = 1;
	else if (h == 2)
		r = 2;
	else if (h == 1)
		r = 3;
	else
		r = 0;
	if (angle < 0.0)
		angle += 2 * PI;
	h = (int) Round (Degrees (angle) / 90.0); 
	h = (h + r) % nEdgeCount;
	childSideP->m_info.nOvlTex &= TEXTURE_MASK;
	if (h == 1)
		childSideP->m_info.nOvlTex |= 0xC000;
	else if (h == 2)
		childSideP->m_info.nOvlTex |= 0x8000;
	else if (h == 3)
		childSideP->m_info.nOvlTex |= 0x4000;
	}

undoManager.End ();
return nChildSide; 
}

// -----------------------------------------------------------------------------

void CSegmentManager::UpdateTexCoords (ushort nVertexId, bool bMove, short nIgnoreSegment, short nIgnoreSide)
{
CSegment* segP = Segment (0);
for (short nSegment = 0; 0 <= (nSegment = FindByVertex (nVertexId, nSegment)); nSegment++, segP++) {
	if (nIgnoreSegment >= 0) {
		if ((nIgnoreSide < 0) && (nSegment == nIgnoreSegment))
			continue;
		if ((nIgnoreSegment == 0x7FFF) && segP->IsMarked ())
			continue;
		}
	segP->UpdateTexCoords (nVertexId, bMove, (nSegment == nIgnoreSegment) ? nIgnoreSide : -1);
	}
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.textures.cpp