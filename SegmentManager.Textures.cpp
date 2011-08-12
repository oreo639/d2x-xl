// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

// -----------------------------------------------------------------------------

void CSegmentManager::Textures (CSideKey key, short& nBaseTex, short& nOvlTex)
{
current->Get (key);
Side (key)->GetTextures (nBaseTex, nOvlTex);
}

// -----------------------------------------------------------------------------

bool CSegmentManager::SetTextures (CSideKey key, short nBaseTex, short nOvlTex)
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
if ((lightManager.IsLight (sideP->m_info.nBaseTex) == -1) && (lightManager.IsLight (sideP->m_info.nOvlTex & 0x3fff) == -1))
	lightManager.DeleteVariableLight (key); 
if (!wallManager.ClipFromTexture (key))
	wallManager.CheckForDoor (key); 
undoManager.End (); 
sprintf_s (message, sizeof (message), "side has textures %d, %d", sideP->m_info.nBaseTex & 0x1fff, sideP->m_info.nOvlTex & 0x1fff); 
INFOMSG (message); 
return true;
}

// ------------------------------------------------------------------------ 

int CSegmentManager::AlignTextures (short nStartSeg, short nStartSide, short nOnlyChildSeg, int bAlign1st, int bAlign2nd, char bAlignedSides)
{
	CSegment*	segP = Segment (nStartSeg); 
	CSegment*	childSegP; 
	CSide*		sideP = segP->m_sides + nStartSide; 
	CSide*		childSideP; 

	int			return_code = -1; 
	short			i; 
	short			nSide, nChildSeg, nLine; 
	short			point0, point1, vert0, vert1; 
	short			nChildSide, nChildLine; 
	short			nChildPoint0, nChildPoint1, nChildVert0, nChildVert1; 
	short			u0, v0; 
	double		sAngle, cAngle, angle, length; 

	static int sideChildTable [6][4] = {
		{4, 3, 5, 1}, //{5, 1, 4, 3}, 
		{2, 4, 0, 5}, //{5, 0, 4, 2}, 
		{5, 3, 4, 1}, //{5, 3, 4, 1}, 
		{0, 4, 2, 5}, //{5, 0, 4, 2}, 
		{2, 3, 0, 1}, //{2, 3, 0, 1}, 
		{0, 3, 2, 1} //{2, 3, 0, 1}
		}; 

undoManager.Begin (udSegments);
for (nLine = 0; nLine < 4; nLine++) {
	// find vert numbers for the line's two end points
	point0 = lineVertTable [sideLineTable [nStartSide][nLine]][0]; 
	point1 = lineVertTable [sideLineTable [nStartSide][nLine]][1]; 
	vert0  = segP->m_info.verts [point0]; 
	vert1  = segP->m_info.verts [point1]; 
	// check child for this line
	if (nStartSeg == nOnlyChildSeg) {
		nSide = nStartSide;
		nChildSeg = nStartSeg;
		}
	else {
		nSide = sideChildTable [nStartSide][nLine]; 
		nChildSeg = segP->Child (nSide); 
		}
	if ((nChildSeg < 0) || ((nOnlyChildSeg != -1) && (nChildSeg != nOnlyChildSeg)))
		continue;
	childSegP = Segment (nChildSeg); 
	// figure out which side of child shares two points w/ nStartSide
	for (nChildSide = 0; nChildSide < 6; nChildSide++) {
		if ((nStartSeg == nOnlyChildSeg) && (nChildSide == nStartSide))
			continue;
		if (bAlignedSides & (1 << nChildSide))
			continue;
		// ignore children of different textures (or no texture)
		if (!IsWall (CSideKey (nChildSeg, nChildSide)))
			continue;
		if (childSegP->m_sides [nChildSide].m_info.nBaseTex != sideP->m_info.nBaseTex)
			continue;
		for (nChildLine = 0; nChildLine < 4; nChildLine++) {
			// find vert numbers for the line's two end points
			nChildPoint0 = lineVertTable [sideLineTable [nChildSide][nChildLine]][0]; 
			nChildPoint1 = lineVertTable [sideLineTable [nChildSide][nChildLine]][1]; 
			nChildVert0  = childSegP->m_info.verts [nChildPoint0]; 
			nChildVert1  = childSegP->m_info.verts [nChildPoint1]; 
			// if points of child's line== corresponding points of parent
			if (!((nChildVert0 == vert1 && nChildVert1 == vert0) ||
					(nChildVert0 == vert0 && nChildVert1 == vert1)))
				continue;
			// now we know the child's side & line which touches the parent
			// child:  nChildSeg, nChildSide, nChildLine, nChildPoint0, nChildPoint1
			// parent: nStartSeg, nStartSide, nLine, point0, point1
			childSideP = childSegP->m_sides + nChildSide; 
			if (bAlign1st) {
				// now translate all the childs (u, v) coords so child_point1 is at zero
				u0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].u; 
				v0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].v; 
				for (i = 0; i < 4; i++) {
					childSideP->m_info.uvls [i].u -= u0; 
					childSideP->m_info.uvls [i].v -= v0; 
					}
				// find difference between parent point0 and child point1
				u0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].u - sideP->m_info.uvls [nLine].u; 
				v0 = childSideP->m_info.uvls [(nChildLine + 1) % 4].v - sideP->m_info.uvls [nLine].v; 
				// find the angle formed by the two lines
				sAngle = atan3(sideP->m_info.uvls [(nLine + 1) % 4].v - sideP->m_info.uvls [nLine].v, 
									sideP->m_info.uvls [(nLine + 1) % 4].u - sideP->m_info.uvls [nLine].u); 
				cAngle = atan3(childSideP->m_info.uvls [nChildLine].v - childSideP->m_info.uvls [(nChildLine + 1) % 4].v, 
									childSideP->m_info.uvls [nChildLine].u - childSideP->m_info.uvls [(nChildLine + 1) % 4].u); 
				// now rotate childs (u, v) coords around child_point1 (cAngle - sAngle)
				for (i = 0; i < 4; i++) {
					angle = atan3(childSideP->m_info.uvls [i].v, childSideP->m_info.uvls [i].u); 
					length = sqrt((double) childSideP->m_info.uvls [i].u * (double) childSideP->m_info.uvls [i].u +
									  (double) childSideP->m_info.uvls [i].v * (double) childSideP->m_info.uvls [i].v); 
					angle -= (cAngle - sAngle); 
					childSideP->m_info.uvls [i].u = (short)(length * cos (angle)); 
					childSideP->m_info.uvls [i].v = (short)(length * sin (angle)); 
					}
				// now translate all the childs (u, v) coords to parent point0
				for (i = 0; i < 4; i++) {
					childSideP->m_info.uvls [i].u -= u0; 
					childSideP->m_info.uvls [i].v -= v0; 
					}
				// modulo points by 0x800 (== 64 pixels)
				u0 = childSideP->m_info.uvls [0].u / 0x800; 
				v0 = childSideP->m_info.uvls [0].v / 0x800; 
				for (i = 0; i < 4; i++) {
					childSideP->m_info.uvls [i].u -= u0*0x800; 
					childSideP->m_info.uvls [i].v -= v0*0x800; 
					}
				if (nOnlyChildSeg != -1)
					return_code = nChildSide; 
				}
			if (bAlign2nd && sideP->m_info.nOvlTex && childSideP->m_info.nOvlTex) {
				int r, h = sideP->m_info.nOvlTex & 0xC000;
				if (h == 0xC000)
					r = 1;
				else if (h == 0x8000)
					r = 2;
				else if (h == 0x4000)
					r = 3;
				else
					r = 0;
				angle = atan3(childSideP->m_info.uvls [0].v, childSideP->m_info.uvls [0].u); 
				h = (int) (Degrees (fabs (angle)) / 90 + 0.5); 
//				h +=(nChildLine + nLine + 2) % 4; //(nChildLine > nLine) ? nChildLine - nLine : nLine - nChildLine;
				h = (h + r) % 4;
				childSideP->m_info.nOvlTex &= ~0xC000;
				if (h == 1)
					childSideP->m_info.nOvlTex |= 0xC000;
				else if (h == 2)
					childSideP->m_info.nOvlTex |= 0x8000;
				else if (h == 3)
					childSideP->m_info.nOvlTex |= 0x4000;
				}
			break;
			}
		}
	}
undoManager.End ();
return return_code; 
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp