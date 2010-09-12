// Copyright (C) 1997 Bryan Aamot
#include "mine.h"
#include "dle-xp.h"

// external globals
extern int bEnableDeltaShading; // uvls.cpp

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
// Mark each segment that is within a given range (in segments to traverse) to a root segment

class CVicinity {
	private:
		short* m_depth;
		short m_null;

	public:
		CVicinity () {
			m_null = -1;
			m_depth = new short [segmentManager.Count ()];
			if (m_depth != null)
				memset (m_depth, 0, segmentManager.Count () * sizeof (m_depth [0]));
			}

		~CVicinity () {
			if (m_depth != null) {
				delete[] m_depth;
				m_depth = null;
				}
			}

		inline short Depth (short i) { return (m_depth == null) ? -1 : m_depth [i]; }

		const short& operator[] (int i) { return m_depth ? m_depth [i] : m_null; } 

		void Compute (short nSegment, short nDepth) {
			if (nSegment < 0)
				return;

			if (m_depth == null)
				return;

			if (nDepth <= m_depth [nSegment])
				return;
			m_depth [nSegment] = nDepth;

			if (nDepth > 0) {
				// check each side of this segment for more children
				CSegment* segP = segmentManager.Segment (nSegment);
				for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
					// skip if there is a wall and its a door
					CWall* wallP = segmentManager.Wall (CSideKey (nSegment, nSide));
					if ((wallP == null) && (wallP->Type () != WALL_DOOR))
						Compute (segP->Child (nSide), nDepth - 1);
					}
				}
			}
};

//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
// light_weight()
//---------------------------------------------------------------------------------

byte CLightManager::LightWeight (short nBaseTex) 
{
return (byte) ((m_lightMap [nBaseTex] - 1) / 0x0200L);
}

//---------------------------------------------------------------------------------

void CLightManager::ScaleCornerLight (double fLight, bool bAll) 
{
	double scale;

undoManager.Begin (udSegments);
scale = fLight / 100.0; // 100.0% = normal
//#pragma omp parallel 
	{
	//#pragma omp for
	for (CSegmentIterator si; si; si++) {
		CSegment* segP = &(*si);
		if (bAll || (segP->m_info.wallFlags & MARKED_MASK)) {
			segP->Backup ();
			CSide* sideP = segP->m_sides;
			for (int j = 0; j < 6; j++) {
				CUVL* uvlP = sideP->m_info.uvls;
				for (int i = 0; i < 4; i++) {
					double l = ((double) ((ushort) uvlP [i].l)) * scale;
					l = min (l, 0x8000);
					l = max (l, 0);
					uvlP [i].l = (ushort) l;
					}
				}
			}
		}
	}
undoManager.End ();
}

//---------------------------------------------------------------------------------

typedef struct tAvgCornerLight {
	ushort	light;
	byte		count;
} tAvgCornerLight;

void CLightManager::CalcAverageCornerLight (bool bAll)
{
  tAvgCornerLight* maxBrightness = new tAvgCornerLight [vertexManager.Count ()];

memset (maxBrightness, 0, vertexManager.Count () * sizeof (tAvgCornerLight));

undoManager.Begin (udSegments);
// smooth corner light by averaging all corners which share a vertex
//#pragma omp parallel 
	{
//#pragma omp for
	for (CSegmentIterator si; si; si++) {
		CSegment *segP = &(*si);
		for (int nPoint = 0; nPoint < 8; nPoint++) {
			int nVertex = segP->m_info.verts [nPoint];
			if (bAll || (vertexManager.Status (nVertex) & MARKED_MASK)) {
				for (int i = 0; i < 3; i++) {
					CSide* sideP = &segP->m_sides [pointSideTable [nPoint][i]];
					if (sideP->IsVisible ()) {
						int nCorner = pointCornerTable [nPoint][i];
						if (maxBrightness [nVertex].light < (ushort) sideP->m_info.uvls [nCorner].l)
							maxBrightness [nVertex].light = (ushort) sideP->m_info.uvls [nCorner].l;
						maxBrightness [nVertex].count++;
						}
					}
				}
			}
		}
			//	maxBrightness = min(maxBrightness,0x8000L);
//#pragma omp for
	for (CSegmentIterator si; si; si++) {
		CSegment *segP = &(*si);
		bool bUndo = false;
		for (int nPoint = 0; nPoint < 8; nPoint++) {
			int nVertex = segP->m_info.verts [nPoint];
			if ((maxBrightness [nVertex].count > 0) && (bAll || (vertexManager.Status (nVertex) & MARKED_MASK))) {
				for (int i = 0; i < 3; i++) {
					CSide* sideP = &segP->m_sides [pointSideTable [nPoint][i]];
					if (sideP->IsVisible ()) {
						if (!bUndo) {
							segP->Backup ();
							bUndo = true;
							}
						sideP->m_info.uvls [pointCornerTable [nPoint][i]].l = maxBrightness [nVertex].light;
						}
					}
				}
			}
		}
	} // omp parallel

undoManager.End ();
delete[] maxBrightness;
}

//---------------------------------------------------------------------------------

void CLightManager::ComputeStaticLight (double fLightScale, bool bAll, bool bCopyTexLights) 
{
// clear all lighting on marked cubes
m_fLightScale = fLightScale;
undoManager.Begin (udSegments | udStaticLight);
if (bAll)
	CLEAR (VertexColors ());
else {
	for (CSegmentIterator si; si; si++) {
		CSegment *segP = &(*si);
		if (segP->m_info.wallFlags & MARKED_MASK) {
			segP->Backup ();
			CSide* sideP = segP->m_sides;
			for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++, sideP++) {
				for (int i = 0; i < 4; i++) {
					sideP->m_info.uvls [i].l = 0;
					if (!bAll)
						VertexColor (segP->m_info.verts [sideVertTable [nSide][i]])->Clear ();
					}
				}
			}
		}
	}

// Calculate cube side corner light values
// for each marked side in the level
// (range: 0 = min, 0x8000 = max)
for (CSegmentIterator si; si; si++) {
	CSegment *segP = &(*si);
	short nSegment = (short) si.Index ();
	CSide* sideP = segP->m_sides;
	for (short nSide = 0; nSide < 6; nSide++, sideP++) {
		if (!(bAll || segmentManager.IsMarked (CSideKey (nSegment, nSide))))
			continue;
		if (!segmentManager.IsWall (CSideKey (nSegment, nSide)))
			continue;
		if (bCopyTexLights)
			FaceColor (nSegment, nSide)->Clear ();
		uint brightness = 0;
		int nTexture = sideP->m_info.nBaseTex;
		if ((nTexture >= 0) && (nTexture < MAX_TEXTURES))
			brightness = max (brightness, LightWeight (nTexture));
		nTexture = sideP->m_info.nOvlTex & 0x3fff;
		if ((nTexture > 0) && (nTexture < MAX_TEXTURES))
			brightness = max (brightness, LightWeight (nTexture));
		if (brightness > 0)
			GatherLight (nSegment, nSide, (uint) (brightness * 2 * m_fLightScale), bAll, bCopyTexLights);
		}
	}
undoManager.End ();
}

//---------------------------------------------------------------------------------
// set_illumination()
//---------------------------------------------------------------------------------
// blend two colors depending on each colors brightness
// psc: source color (light source)
// pdc: destination color (vertex/side color)
// srcBr: source brightness (remaining brightness at current vertex/side)
// destBr: vertex/side brightness

void CLightManager::BlendColors (CColor* srcColorP, CColor* destColorP, double srcBrightness, double destBrightness)
{
if (destBrightness)
	destBrightness /= 65536.0;
else {
	if (srcBrightness)
		*destColorP = *srcColorP;
	return;
	}
if (srcBrightness)
	srcBrightness /= 65536.0;
else
	return;
//#pragma omp critical
	{
	if (destColorP->m_info.index) {
		destColorP->m_info.color.r += srcColorP->m_info.color.r * srcBrightness;
		destColorP->m_info.color.g += srcColorP->m_info.color.g * srcBrightness;
		destColorP->m_info.color.b += srcColorP->m_info.color.b * srcBrightness;
		double cMax = destColorP->m_info.color.r;
			if (cMax < destColorP->m_info.color.g)
				cMax = destColorP->m_info.color.g;
			if (cMax < destColorP->m_info.color. b)
				cMax = destColorP->m_info.color. b;
			if (cMax > 1) {
				destColorP->m_info.color.r /= cMax;
				destColorP->m_info.color.g /= cMax;
				destColorP->m_info.color.b /= cMax;
				}
		}
	else {
		if (destBrightness) {
			destColorP->m_info.index = 1;
			destColorP->m_info.color.r = srcColorP->m_info.color.r * destBrightness;
			destColorP->m_info.color.g = srcColorP->m_info.color.g * destBrightness;
			destColorP->m_info.color.b = srcColorP->m_info.color.b * destBrightness;
			}
		else
			*destColorP = *srcColorP;
		}
	}
}

//---------------------------------------------------------------------------------

void CLightManager::GatherFaceLight (CSegment* segP, short nSide, uint brightness, CColor* lightColorP)
{
CUVL*	uvlP = segP->m_sides [nSide].m_info.uvls;
uint	vertBrightness, lightBrightness;
byte*	sideVerts = sideVertTable [nSide];

for (int i = 0; i < 4; i++, uvlP++) {
	CColor* vertColorP = VertexColor (segP->m_info.verts [sideVerts [i]]);
	vertBrightness = (ushort) uvlP->l;
	lightBrightness = (uint) (brightness * (m_cornerLights ? m_cornerLights [i] : m_fLightScale));
	BlendColors (lightColorP, vertColorP, lightBrightness, vertBrightness);
	vertBrightness += lightBrightness;
	vertBrightness = min (0x8000, vertBrightness);
	uvlP->l = (ushort) vertBrightness;
	}
}

//---------------------------------------------------------------------------------
// Walk through all segments up to a given max. distance from the source segment
// and add the light of any light sources in these segments to the the source side's
// color.

void CLightManager::GatherLight (short nSourceSeg, short nSourceSide, uint brightness, bool bAll, bool bCopyTexLights) 
{
	CSegment*	segP = segmentManager.Segment (0);
	// find orthogonal angle of source segment
	CVertex A = -segmentManager.CalcSideNormal (CSideKey (nSourceSeg, nSourceSide));
	// remember to flip the sign since we want it to point inward
	// calculate the center of the source segment
	CVertex sourceCenter = segmentManager.CalcSideCenter (CSideKey (nSourceSeg, nSourceSide));
	// mark those segmentManager.Segment () within N children of current cube

// set child numbers
//segmentManager.Segment ()[nSourceSeg].nIndex = m_renderDepth;

segP = segmentManager.Segment (nSourceSeg);
CVicinity vicinity;
vicinity.Compute (nSourceSeg, m_renderDepth);	//mark all children that are at most lightRenderDepth segments away

CColor *lightColorP = LightColor (CSideKey (nSourceSeg, nSourceSide));
if (!lightColorP->m_info.index) {
	lightColorP->m_info.index = 255;
	lightColorP->m_info.color.r =
	lightColorP->m_info.color.g =
	lightColorP->m_info.color.b = 1.0;
	}
if (UseTexColors () && bCopyTexLights) {
	CColor* colorP = LightColor (CSideKey (nSourceSeg, nSourceSide), false);
	*lightColorP = *colorP;
	}
bool bWall = false; 

//#pragma omp parallel 
	{
//#pragma omp for private (m_cornerLights)
	for (CSegmentIterator si; si; si++) {
		// skip if this is too far away
		int nChildSeg = si.Index ();
		if (vicinity [nChildSeg] <= 0)
			continue;
		CSegment *childSegP = &(*si);
		// setup source corner vertex for length calculation later
		CVertex sourceCorners [4];
		byte* sideVerts = sideVertTable [nSourceSide];
		for (int j = 0; j < 4; j++)
			sourceCorners [j] = *vertexManager.Vertex (segP->m_info.verts [sideVerts [j]]);
		// loop on child sides
		for (int nChildSide = 0; nChildSide < 6; nChildSide++) {
			// if side has a child..
			if (!(bAll || segmentManager.IsMarked (CSideKey (nChildSeg, nChildSide))))
				continue;
			if (childSegP->Child (nChildSide) >= 0) {
				CWall* wallP = childSegP->Side (nChildSide)->Wall ();
				if ((wallP == null) || !wallP->IsDoor ())
					continue;
				}

			// if the child side is the same as the source side, then set light and continue
			if ((nChildSide == nSourceSide) && (nChildSeg == nSourceSeg)) {
				GatherFaceLight (childSegP, nChildSide, brightness, lightColorP);
				continue;
				}

			// calculate vector between center of source segment and center of child
			if (CalcCornerLights (nChildSeg, nChildSide, sourceCenter, sourceCorners, A, bWall)) 
				GatherFaceLight (childSegP, nChildSide, brightness, lightColorP);
			}
		}
	}
}

//------------------------------------------------------------------------
// ComputeVariableLight()
//
// Action - Calculates lightDeltaIndices and CLightDeltaValues arrays.
//
//	Delta lights are only created if the segment has a nOvlTex
//	which matches one of the entries in the broken[] table.
//
//	For a given side, we calculate the distance and angle to all
//	nearby sides.  A nearby side is a side which is on a cube
//	which is a child or sub-child of the current cube.  Light does
//	not shine through wallManager.Wall ().
//
//	We use the following equation to calculate
//	how much the light effects a side:
//              if (angle < 120 degrees)
//		  m_cornerLights = k / distance
//
//	where,
//
// 		angle = acos (A dot B / |A|*|B|)
//		A = vector orthogonal to light source side
//		B = vector from center of light source side to effected side.
//		distance is from center of light souce to effected side's corner
//		k = 0x20 (maximum value for CLightDeltaValue)
//
//	Delta light values are stored for each corner of the effected side.
//	(0x00 = no m_cornerLights, 0x20 is maximum m_cornerLights)
//
// A little int to take care of too many delta light values leading
// to some variable lights not being visible in Descent:
// For each recursion step, the delta light calculation will be executed
// for all children of sides with variable lights with exactly that
// distance (recursionDepth) to their parent. In case of too many delta 
// light values for the full lighting depth at least the lighting of the
// segmentManager.Segment () close to variable lights will be computed properly.
//------------------------------------------------------------------------

void CLightManager::ComputeVariableLight (double fLightScale, int force) 
{
m_fLightScale = fLightScale;
undoManager.Begin (udDynamicLight);
for (int nDepth = m_deltaRenderDepth; nDepth; nDepth--)
	if (CalcLightDeltas (force, nDepth))
		break;
undoManager.End ();
}

//---------------------------------------------------------------------------------

int CLightManager::FindLightDelta (short nSegment, short nSide, short *pi)
{
	int	i = pi ? *pi : 0;
	int	j = DeltaIndexCount ()++;
	CLightDeltaIndex	*dliP = LightDeltaIndex (0);

if ((DLE.LevelVersion () >= 15) && (theMine->Info ().fileInfo.version >= 34)) {
	for (; i < j; i++, dliP++)
		if ((dliP->m_nSegment == nSegment) && (dliP->m_nSide = (byte) nSide))
			return i;
	}
else {
	for (; i < j; i++, dliP++)
		if ((dliP->m_nSegment == nSegment) && (dliP->m_nSide = (byte) nSide))
			return i;
	}
return -1;
}

//---------------------------------------------------------------------------------

bool CLightManager::CalcLightDeltas (int force, int nDepth) 
{
	// initialize totals
	int nErrors = 0;

DeltaValueCount () = 0;
DeltaIndexCount () = 0;

bool bD2XLights = (DLE.LevelVersion () >= 15) && (theMine->Info ().fileInfo.version >= 34);

m_fLightScale = 1.0; ///= 100.0;
//#pragma omp parallel
	{
//#pragma omp for private (m_cornerLights)
	for (CSegmentIterator si; si; si++) {
		if (nErrors)
			continue;
		CSegment* srcSegP = &(*si);
		// skip if not marked unless we are automatically saving
		if  (!(srcSegP->m_info.wallFlags & MARKED_MASK) && !force) 
			continue;
		// loop on all sides
		short nSourceSeg = (short) si.Index ();
		CSide* sideP = srcSegP->m_sides;
		for (int nSourceSide = 0; nSourceSide < 6; nSourceSide++, sideP++) {
			short nBaseTex, nOvlTex;
			sideP->GetTextures (nBaseTex, nOvlTex);
			bool bl1 = (IsLight (nBaseTex) != -1);
			bool bl2 = (IsLight (nOvlTex) != -1);
			if (!(bl1 || bl2))
				continue;	// no lights on this side
			bool bCalcDeltas = false;
			// if the current side is a wall and has a light and is the target of a trigger
			// than can make the wall appear/disappear, calculate delta lights for it
			CSideKey key (nSourceSeg, nSourceSide);
			CWall* wallP = segmentManager.Wall (key);
			bCalcDeltas = (wallP != null) && wallP->IsVariable ();
			if (!bCalcDeltas)
				bCalcDeltas = IsVariableLight (key);
			if (!bCalcDeltas) {
				bool bb1 = IsBlastableLight (nBaseTex);
				bool bb2 = IsBlastableLight (nOvlTex);
				if (bb1 == bb2)
					bCalcDeltas = bb1;	// both lights blastable or not
				else if (!(bb1 ? bl2 : bl1))	// i.e. one light blastable and the other texture not a non-blastable light 
					bCalcDeltas = true;
				}
			if (!bCalcDeltas) {	//check if light is target of a "light on/off" trigger
				CTrigger* trigP = triggerManager.FindByTarget (key);
				if ((trigP != null) && (trigP->Type () >= TT_LIGHT_OFF))
					bCalcDeltas = true;
				}
			if (!bCalcDeltas)
				continue;
			if ((wallP != null) && !wallP->IsVisible ())
				continue;

			if (DeltaIndexCount () >= MAX_LIGHT_DELTA_INDICES) {
//#pragma omp critical
				{
				if (++nErrors == 1) {
					char szMsg [256];
					sprintf_s (szMsg, sizeof (szMsg), " Light tool: Too many dynamic lights at render depth %d", nDepth);
					DEBUGMSG (szMsg);
					}
				}
				continue;
				}

			CVertex	A, sourceCenter;
			int		lightDeltaIndexCount;

			// get index number and increment total number of lightDeltaIndices
	//#pragma omp critical
			{
			lightDeltaIndexCount = int (DeltaIndexCount ()++);
			}
			CLightDeltaIndex *dliP = LightDeltaIndex (lightDeltaIndexCount);
			dliP->m_nSegment = nSourceSeg;
			dliP->m_nSide = nSourceSide;
			dliP->m_info.count = 0; // will be incremented below
			dliP->m_info.index = (short)DeltaValueCount ();

			// find orthogonal angle of source segment
			A = -segmentManager.CalcSideNormal (CSideKey (nSourceSeg, nSourceSide));

			// calculate the center of the source segment
			sourceCenter = segmentManager.CalcSideCenter (CSideKey (nSourceSeg, nSourceSide));

			// mark those segmentManager.Segment () within N children of current cube
			//(note: this is done once per light instead of once per segment
			//       even though some segmentManager.Segment () have multiple lights.
			//       This actually reduces the number of calls since most
			//       segmentManager.Segment () do not have lights)
			CVicinity vicinity;
			vicinity.Compute (nSourceSeg, nDepth);

			// setup source corner vertex for length calculation later
			CVertex sourceCorners [4];
			for (int j = 0; j < 4; j++) {
				byte nVertex = sideVertTable [nSourceSide][j];
				int h = srcSegP->m_info.verts [nVertex];
				sourceCorners [j] = *vertexManager.Vertex (h);
				}

			// loop on child segmentManager.Segment ()
			for (CSegmentIterator sj; sj; sj++) {
				if (nErrors)
					continue;
				int nChildSeg = sj.Index ();
				if (vicinity [nChildSeg] == 0)
					continue;
				CSegment *childSegP = &(*sj);
				// loop on child sides
				for (int nChildSide = 0; nChildSide < 6; nChildSide++) {
					// if texture has a child..
					if (childSegP->Child (nChildSide) >= 0) {
						CWall* wallP = childSegP->m_sides [nChildSide].Wall ();
						// .. if there is no wall ..
						if (wallP == null)
							continue;
						// .. or its not a door ..
						if (wallP->Type () == WALL_OPEN) 
							continue; // don't put light because there is no texture here
						}
					// don't affect non-variable light emitting textures (e.g. lava)
					short nBaseTex = childSegP->m_sides [nChildSide].m_info.nBaseTex;
					short nOvlTex = childSegP->m_sides [nChildSide].m_info.nOvlTex & 0x1fff;
					if (m_nNoLightDeltas == 1) {
						if (((IsLight (nBaseTex) >= 0) || (IsLight (nOvlTex) >= 0))
							 && !IsVariableLight (CSideKey (nChildSeg, nChildSide)))
							continue;
						}
					else if ((m_nNoLightDeltas == 2) && (textureManager.IsLava (nBaseTex) || textureManager.IsLava (nOvlTex)))
						continue;
					// if the child side is the same as the source side, then set light and continue
					if (nChildSide == nSourceSide && nChildSeg == nSourceSeg) {
						if ((DeltaValueCount () >= MAX_LIGHT_DELTA_VALUES) || (dliP->m_info.count == (bD2XLights ? 8191 : 255))) {
//#pragma omp critical
							{
							if (++nErrors == 1) {
								char szMsg [256];
								sprintf_s (szMsg, sizeof (szMsg), " Light tool: Too many dynamic lights at render depth %d", nDepth);
								DEBUGMSG (szMsg);
								}
							}
							continue;
							}
						CLightDeltaValue* dl;
	//#pragma omp critical
						{
						dl = LightDeltaValue (DeltaValueCount ()++);
						}
						dl->m_nSegment = nChildSeg;
						dl->m_nSide = nChildSide;
						dl->m_info.vertLight [0] =
						dl->m_info.vertLight [1] =
						dl->m_info.vertLight [2] =
						dl->m_info.vertLight [3] = (byte) min (32, 32 * m_fLightScale);
						dliP->m_info.count++;
						continue;
						}

					// calculate vector between center of source segment and center of child
						if (CalcCornerLights (nChildSeg, nChildSide, sourceCenter, sourceCorners, A, wallP != null)) {
							if ((DeltaValueCount () >= MAX_LIGHT_DELTA_VALUES) || (bD2XLights ? dliP->m_info.count == 8191 : dliP->m_info.count == 255)) {
//#pragma omp critical
								{
								if (++nErrors == 1) {
									char szMsg [256];
									sprintf_s (szMsg, sizeof (szMsg), " Light tool: Too many dynamic lights at render depth %d", nDepth);
									DEBUGMSG (szMsg);
									}
								}
								continue;
								}
//#pragma omp critical
							CLightDeltaValue* dl = LightDeltaValue (DeltaValueCount ()++);
							dl->m_nSegment = nChildSeg;
							dl->m_nSide = nChildSide;
							for (int nCorner = 0; nCorner < 4; nCorner++)
								dl->m_info.vertLight [nCorner] = (byte) min(32, m_cornerLights [nCorner]);
							if (bD2XLights)
								dliP->m_info.count++;
							else
								dliP->m_info.count++;
							}
						}
					}
	//			}
			}
		}
	}
return (nErrors == 0);
}

//---------------------------------------------------------------------------------

bool CLightManager::CalcCornerLights (int nSegment, int nSide, CVertex& sourceCenter, CVertex* sourceCorners, CVertex& vertex, bool bIgnoreAngle)
{
	CSegment *segP = segmentManager.Segment (nSegment);
// calculate vector between center of source segment and center of child
CVertex center = segmentManager.CalcSideCenter (CSideKey (nSegment, nSide));
CVertex A = vertex;
CVertex B = center - sourceCenter;

// calculate angle between vectors (use dot product equation)
if (!bIgnoreAngle) {
	double mag_A = A.Mag ();
	double mag_B = B.Mag ();
	if ((mag_A < 0.000001) || (mag_B < 0.000001))
		return false;
	double A_dot_B = A ^ B;
	double ratio = A_dot_B / (mag_A * mag_B);
	ratio = ((double) ((int) (ratio * 1000.0))) / 1000.0;
	if (ratio < -1.0 || ratio > 1.0) 
		return false;
	double angle = acos (ratio);  
	// if angle is less than 180 degrees then we found a match
	if (angle >= Radians (180.0))
		return false;
	}
for (int j = 0; j < 4; j++) {
	CVertex* corner = vertexManager.Vertex (segP->m_info.verts [sideVertTable [nSide][j]]);
	double length = 20.0 * m_renderDepth;
	for (int i = 0; i < 4; i++)
		length = min (length, Distance (sourceCorners [i], *corner));
	length /= 10.0 * m_renderDepth / 6.0; // divide by 1/2 a cubes length so opposite side
	m_cornerLights [j] = m_fLightScale;
	if (length > 1.0) 
		m_cornerLights [j] /= (length * length);
	}
// if any of the effects are > 0, then increment the light for that side
return (m_cornerLights [0] != 0.0) || (m_cornerLights [1] != 0.0) || (m_cornerLights [2] != 0.0) || (m_cornerLights [3] != 0.0);
}

// -----------------------------------------------------------------------------

void CLightManager::SetSegmentLight (double fLight, bool bAll, bool bDynSegLights)
{
	long nLight = (int) (fLight * 65536); //24.0 * 327.68);

undoManager.Begin (udSegments);
fLight /= 100.0;
CSegment *segP = segmentManager.Segment (0);
for (CSegmentIterator si; si; si++) {
	CSegment* segP = &(*si);
	if (bAll || (segP->m_info.wallFlags & MARKED_MASK)) {
		if (!bDynSegLights)
			segP->m_info.staticLight = nLight;
		else {
			int l = 0;
			int c = 0;
			CSide* sideP = segP->m_sides;
			for (short nSide = 0; nSide < 6; nSide++) {
				for (short nCorner = 0; nCorner < 4; nCorner++) {
					ushort h = (ushort) sideP [nSide].m_info.uvls [nCorner].l;
					if (h || !sideP->IsVisible ()) {
						l += h;
						c++;
						}
					}
				}
			segP->Backup ();
			segP->m_info.staticLight = (int) (c ? fLight * ((double) l / (double) c) * 2 : nLight);
			}
		}
	}
undoManager.End ();
}

//---------------------------------------------------------------------------------
//eof Illumination.cpp
