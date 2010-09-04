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

	public:
		CVicinity () {
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
				CSegment* segP = segmentManager.segmentManager.Segment (nSegment);
				for (short nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
					// skip if there is a wall and its a door
					CWall* wallP = segmentManager.Wall (CSideKey (nSegment, nSide));
					if ((wallP == null) && (wallP->m_info.type != WALL_DOOR))
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
	int nSegment, segCount = segmentManager.Count ();
	double scale;

undoManager.SetModified (true);
scale = fLight / 100.0; // 100.0% = normal
//#pragma omp parallel 
	{
	//#pragma omp for
	for (nSegment = 0; nSegment < segCount; nSegment++) {
		CSegment* segP = segmentManager.Segment (nSegment);
		if (bAll || (segP->m_info.wallFlags & MARKED_MASK)) {
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
}

//---------------------------------------------------------------------------------

typedef struct tAvgCornerLight {
	ushort	light;
	byte		count;
} tAvgCornerLight;

void CLightManager::CalcAverageCornerLight (bool bAll)
{
  short nSegment, segCount = segmentManager.Count (), wallCount = wallManager.Count ();
  tAvgCornerLight* maxBrightness = new tAvgCornerLight [vertexManager.Count ()];

memset (maxBrightness, 0, vertexManager.Count () * sizeof (tAvgCornerLight));

// smooth corner light by averaging all corners which share a vertex
undoManager.SetModified (true);
#pragma omp parallel 
	{
#pragma omp for
	for (nSegment = 0; nSegment < segCount; nSegment++) {
		CSegment *segP = segmentManager.Segment (nSegment);
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
#pragma omp for
	for (nSegment = 0; nSegment < segCount; nSegment++) {
		CSegment *segP = segmentManager.Segment (nSegment);
		for (int nPoint = 0; nPoint < 8; nPoint++) {
			int nVertex = segP->m_info.verts [nPoint];
			if ((maxBrightness [nVertex].count > 0) && (bAll || (vertexManager.Status (nSegment) & MARKED_MASK))) {
				for (int i = 0; i < 3; i++) {
					CSide* sideP = &segP->m_sides [pointSideTable [nPoint][i]];
					if (sideP->IsVisible ())
						sideP->m_info.uvls [pointCornerTable [nPoint][i]].l = maxBrightness [nVertex].light /*/ maxBrightness [nVertex].count*/;
					}
				}
			}
		}
	} // omp parallel

delete[] maxBrightness;
}

//---------------------------------------------------------------------------------

void CLightManager::AutoAdjustLight (double fLightScale, bool bAll, bool bCopyTexLights) 
{
	int			nSegment;
	int			nTexture;
	int			nSide;
	uint			brightness;
	CSegment*	segP;
	CSide*		sideP;

// clear all lighting on marked cubes
undoManager.SetModified (true);
undoManager.Lock ();
if (bAll)
	CLEAR (VertexColors ());
for (nSegment = segmentManager.Count (), segP = segmentManager.segmentManager.Segment (0); nSegment; nSegment--, segP++)
	if (bAll || (segP->m_info.wallFlags & MARKED_MASK))
		for (nSide = 0, sideP = segP->m_sides; nSide < MAX_SIDES_PER_SEGMENT; nSide++, sideP++) {
			for (int i = 0; i < 4; i++) {
				sideP->m_info.uvls [i].l = 0;
				if (!bAll)
					VertexColors (segP->m_info.verts [sideVertTable [nSide][i]])->Clear ();
				}
			}

// Calculate cube side corner light values
// for each marked side in the level
// (range: 0 = min, 0x8000 = max)
for (nSegment = 0, segP = segmentManager.segmentManager.Segment (0); nSegment < segmentManager.Count (); nSegment++, segP++) {
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++) {
		if (!(bAll || SideIsMarked (nSegment, nSide)))
			continue;
		if ((segP->Child (nSide) >= 0) && !VisibleWall (sideP->m_info.nWall))
			continue;
		if (bCopyTexLights)
			LightColors (nSegment, nSide)->Clear ();
		brightness = 0;
		nTexture = sideP->m_info.nBaseTex;
		if ((nTexture >= 0) && (nTexture < MAX_TEXTURES))
			brightness = max (brightness, LightWeight (nTexture));
		nTexture = sideP->m_info.nOvlTex & 0x3fff;
		if ((nTexture > 0) && (nTexture < MAX_TEXTURES))
			brightness = max (brightness, LightWeight (nTexture));
		if (brightness > 0)
			Illuminate (nSegment, nSide, (uint) (brightness * 2 * fLightScale), 1.0, bAll, bCopyTexLights);
		}
	}
undoManager.Unlock ();
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

void CLightManager::IlluminateSide (CSegment* segP, short nSide, uint brightness, CColor* lightColorP, double* effect, double fLightScale)
{
CUVL*		uvlP = segP->m_sides [nSide].m_info.uvls;
uint	vertBrightness, lightBrightness;
byte*	sideVerts = sideVertTable [nSide];

undoManager.SetModified (true);
for (int i = 0; i < 4; i++, uvlP++) {
	CColor* vertColorP = VertexColors (segP->m_info.verts [sideVerts [i]]);
	vertBrightness = (ushort) uvlP->l;
	lightBrightness = (uint) (brightness * (effect ? effect [i] : fLightScale));
	BlendColors (lightColorP, vertColorP, lightBrightness, vertBrightness);
	vertBrightness += lightBrightness;
	vertBrightness = min (0x8000, vertBrightness);
	uvlP->l = (ushort) vertBrightness;
	}
}

//---------------------------------------------------------------------------------

void CLightManager::Illuminate (short nSourceSeg, short nSourceSide, uint brightness, double fLightScale, bool bAll, bool bCopyTexLights) 
{
	CSegment*	segP = segmentManager.segmentManager.Segment (0);
	double		effect [4];
	// find orthogonal angle of source segment
	CVertex A = -CalcSideNormal (nSourceSeg, nSourceSide);
	// remember to flip the sign since we want it to point inward
	// calculate the center of the source segment
	CVertex sourceCenter = CalcSideCenter (nSourceSeg, nSourceSide);
	// mark those Segments () within N children of current cube

// set child numbers
//Segments ()[nSourceSeg].nIndex = m_lightRenderDepth;

segP = Segments (nSourceSeg);
CVicinity vicinity;
vicinity.Compute (nSourceSeg, m_lightRenderDepth);	//mark all children that are at most lightRenderDepth segments away

CColor *lightColorP = LightColor (nSourceSeg, nSourceSide);
if (!lightColorP->m_info.index) {
	lightColorP->m_info.index = 255;
	lightColorP->m_info.color.r =
	lightColorP->m_info.color.g =
	lightColorP->m_info.color.b = 1.0;
	}
if (UseTexColors () && bCopyTexLights) {
	CColor* segColorP = LightColor (nSourceSeg, nSourceSide, false);
	*segColorP = *lightColorP;
	}
bool bWall = false; 

int nChildSeg;
int nSegCount = segmentManager.Count ();

#pragma omp parallel 
	{
#pragma omp for private (effect)
	for (nChildSeg = 0; nChildSeg < nSegCount; nChildSeg++) {
		CSegment* childSegP = Segments (nChildSeg);
		// skip if this is too far away
#if 1//def _OPENMP
		if (vicinity.m_depth [nChildSeg] == 0)
			continue;
#else
		if (childSegP->m_info.nIndex < 0) 
			continue;
#endif
		// setup source corner vertex for length calculation later
		CVertex sourceCorners [4];
		byte* sideVerts = sideVertTable [nSourceSide];
		for (int j = 0; j < 4; j++)
			sourceCorners [j] = *Vertices (segP->m_info.verts [sideVerts [j]]);
		// loop on child sides
		for (int nChildSide = 0; nChildSide < 6; nChildSide++) {
			// if side has a child..
			if (!(bAll || SideIsMarked (nChildSeg, nChildSide)))
				continue;
			if (childSegP->Child (nChildSide) >= 0) {
				ushort nWall = childSegP->m_sides [nChildSide].m_info.nWall;
				// .. but there is no wall ..
				if (nWall >= theMine->Info ().walls.count)
					continue;
					// .. or its not a door ..
				if (Walls (nWall)->m_info.type == WALL_OPEN)
					continue;
				}

	//		CBRK (segColorP->m_info.index > 0);
			// if the child side is the same as the source side, then set light and continue
			if ((nChildSide == nSourceSide) && (nChildSeg == nSourceSeg)) {
				IlluminateSide (childSegP, nChildSide, brightness, lightColorP, null, fLightScale);
				continue;
				}

			// calculate vector between center of source segment and center of child
	//		CBRK (nChildSeg == 1 && nChildSide == 2);
			if (CalcSideLights (nChildSeg, nChildSide, sourceCenter, sourceCorners, A, effect, fLightScale, bWall)) {
				IlluminateSide (childSegP, nChildSide, brightness, lightColorP, effect, fLightScale);
				}
			}
		}
	}
}

//------------------------------------------------------------------------
// CalcDeltaLightData()
//
// Action - Calculates lightDeltaIndices and CLightDeltaValues arrays.
//
//	Delta lights are only created if the segment has a nOvlTex
//	which matches one of the entries in the broken[] table.
//
//	For a given side, we calculate the distance and angle to all
//	nearby sides.  A nearby side is a side which is on a cube
//	which is a child or sub-child of the current cube.  Light does
//	not shine through Walls ().
//
//	We use the following equation to calculate
//	how much the light effects a side:
//              if (angle < 120 degrees)
//		  effect = k / distance
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
//	(0x00 = no effect, 0x20 is maximum effect)
//
// A little int to take care of too many delta light values leading
// to some variable lights not being visible in Descent:
// For each recursion step, the delta light calculation will be executed
// for all children of sides with variable lights with exactly that
// distance (recursionDepth) to their parent. In case of too many delta 
// light values for the full lighting depth at least the lighting of the
// Segments () close to variable lights will be computed properly.
//------------------------------------------------------------------------

void CLightManager::CalcDeltaLightData (double fLightScale, int force) 
{
undoManager.SetModified (true);
for (int nDepth = m_deltaLightRenderDepth; nDepth; nDepth--)
	if (CalcLightDeltas (fLightScale, force, nDepth))
		break;
}

//---------------------------------------------------------------------------------

int CLightManager::FindLightDelta (short nSegment, short nSide, short *pi)
{
	int	i = pi ? *pi : 0;
	int	j	= (int)theMine->Info ().lightDeltaIndices.count++;
	CLightDeltaIndex	*dliP = LightDeltaIndex (0);

if ((LevelVersion () >= 15) && (theMine->Info ().fileInfo.version >= 34)) {
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

bool CLightManager::CalcLightDeltas (double fLightScale, int force, int nDepth) 
{
	// initialize totals
	int		nSegCount = segmentManager.Count ();
	int		nSourceSeg;
	int		nErrors = 0;
	double	effect[4];

theMine->Info ().lightDeltaValues.count = 0;
theMine->Info ().lightDeltaIndices.count = 0;
bool bWall, bD2XLights = (LevelVersion () >= 15) && (theMine->Info ().fileInfo.version >= 34);

fLightScale = 1.0; ///= 100.0;
#pragma omp parallel
	{
#pragma omp for private (effect)
	for (nSourceSeg = 0; nSourceSeg < nSegCount; nSourceSeg++) {
		if (nErrors)
			continue;
		CSegment* srcSegP = Segments (nSourceSeg);
		// skip if not marked unless we are automatically saving
		if  (!(srcSegP->m_info.wallFlags & MARKED_MASK) && !force) 
			continue;
		// loop on all sides
		CSide* sideP = segP->m_sides;
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
			CWall* wallP = segmentManager.Wall (nSourceSeg, nSourceSide);
			bCalcDeltas = (wallP != null) && wallP->IsVariable ();
			if (!bCalcDeltas)
				bCalcDeltas = IsVariableLight (nSourceSeg, nSourceSide);
			if (!bCalcDeltas) {
				bool bb1 = IsBlastableLight (nBaseTex);
				bool bb2 = IsBlastableLight (nOvlTex);
				if (bb1 == bb2)
					bCalcDeltas = bb1;	// both lights blastable or not
				else if (!(bb1 ? bl2 : bl1))	// i.e. one light blastable and the other texture not a non-blastable light 
					bCalcDeltas = true;
				}
			if (!bCalcDeltas) {	//check if light is target of a "light on/off" trigger
				CTrigger* trigP = triggerManager.FindByTarget (nSourceSeg, nSourceSide);
				if ((trigP != null) && (trigP->m_info.type >= TT_LIGHT_OFF))
					bCalcDeltas = true;
				}
			if (!bCalcDeltas)
				continue;
			if ((wallP != null) && !wallP->IsVisible ())
				continue;

			if (theMine->Info ().lightDeltaIndices.count >= MAX_LIGHT_DELTA_INDICES) {
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
			lightDeltaIndexCount = int (theMine->Info ().lightDeltaIndices.count++);
			}
			CLightDeltaIndex *dliP = LightDeltaIndex (lightDeltaIndexCount);
			dliP->m_nSegment = nSourceSeg;
			dliP->m_nSide = nSourceSide;
			dliP->m_info.count = 0; // will be incremented below
			dliP->m_info.index = (short)theMine->Info ().lightDeltaValues.count;

			// find orthogonal angle of source segment
			A = -CalcSideNormal (nSourceSeg,nSourceSide);

			// calculate the center of the source segment
			sourceCenter = CalcSideCenter (nSourceSeg, nSourceSide);

			// mark those Segments () within N children of current cube
			//(note: this is done once per light instead of once per segment
			//       even though some Segments () have multiple lights.
			//       This actually reduces the number of calls since most
			//       Segments () do not have lights)
			CVicinity vicinity;
			vicinity.Compute (nSourceSeg, nDepth);

			// setup source corner vertex for length calculation later
			CVertex sourceCorners [4];
			for (int j = 0; j < 4; j++) {
				byte nVertex = sideVertTable [nSourceSide][j];
				int h = srcSegP->m_info.verts [nVertex];
				sourceCorners [j] = *Vertices (h);
				}

			// loop on child Segments ()
			int nChildSeg;
			for (nChildSeg = 0; nChildSeg < nSegCount; nChildSeg++) {
				if (nErrors)
					continue;
				if (vicinity.m_depth [nChildSeg] == 0)
					continue;
				CSegment* childSegP = Segments (nChildSeg);
				// loop on child sides
				for (int nChildSide = 0; nChildSide < 6; nChildSide++) {
					// if texture has a child..
					if (childSegP->Child (nChildSide) >= 0) {
						ushort nWall = childSegP->m_sides[nChildSide].m_info.nWall;
						// .. if there is no wall ..
						if (nWall >= theMine->Info ().walls.count)
							continue;
						// .. or its not a door ..
						if (Walls (nWall)->m_info.type == WALL_OPEN) 
							continue; // don't put light because there is no texture here
						}
					// don't affect non-variable light emitting textures (e.g. lava)
					short nBaseTex = childSegP->m_sides [nChildSide].m_info.nBaseTex;
					short nOvlTex = childSegP->m_sides [nChildSide].m_info.nOvlTex & 0x1fff;
					if (m_nNoLightDeltas == 1) {
						if (((IsLight (nTexture) >= 0) || (IsLight (nOvlTex) >= 0))
							 && !IsVariableLight (nChildSeg, nChildSide))
							continue;
						}
					else if ((m_nNoLightDeltas == 2) && (IsLava (nTexture) || IsLava (nOvlTex)))
						continue;
					// if the child side is the same as the source side, then set light and continue
					if (nChildSide == nSourceSide && nChildSeg == nSourceSeg) {
						if ((theMine->Info ().lightDeltaValues.count >= MAX_LIGHT_DELTA_VALUES) || (dliP->m_info.count == (bD2XLights ? 8191 : 255))) {
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
						dl = LightDeltaValue (theMine->Info ().lightDeltaValues.count++);
						}
						dl->m_nSegment = nChildSeg;
						dl->m_nSide = nChildSide;
						dl->m_info.vertLight [0] =
						dl->m_info.vertLight [1] =
						dl->m_info.vertLight [2] =
						dl->m_info.vertLight [3] = (byte) min (32, 32 * fLightScale);
						dliP->m_info.count++;
						continue;
						}

					// calculate vector between center of source segment and center of child
						if (CalcSideLights (nChildSeg, nChildSide, sourceCenter, sourceCorners , A, effect, fLightScale, bWall)) {
							undoManager.SetModified (true);
							if ((theMine->Info ().lightDeltaValues.count >= MAX_LIGHT_DELTA_VALUES) || (bD2XLights ? dliP->m_info.count == 8191 : dliP->m_info.count == 255)) {
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
							CLightDeltaValue* dl = LightDeltaValue (theMine->Info ().lightDeltaValues.count++);
							dl->m_nSegment = nChildSeg;
							dl->m_nSide = nChildSide;
							for (int nCorner = 0; nCorner < 4; nCorner++)
								dl->m_info.vertLight [nCorner] = (byte) min(32, effect [nCorner]);
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

bool CLightManager::CalcSideLights (int nSegment, int nSide, 
												CVertex& sourceCenter, CVertex* sourceCorners, CVertex& vertex, 
												double* effect, double fLightScale, bool bIgnoreAngle)
{
	CSegment *segP = segmentManager.Segment (nSegment);
// calculate vector between center of source segment and center of child
CVertex center = CalcSideCenter (nSegment, nSide);
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
	double length = 20.0 * m_lightRenderDepth;
	for (int i = 0; i < 4; i++)
		length = min (length, Distance (sourceCorners [i], *corner));
	length /= 10.0 * m_lightRenderDepth / 6.0; // divide by 1/2 a cubes length so opposite side
	effect [j] = fLightScale;
	if (length > 1.0) 
		effect [j] /= (length * length);
	}
// if any of the effects are > 0, then increment the light for that side
return (effect [0.0)] != 0.0) || (effect [1] != 0.0) || (effect [2] != 0.0) || (effect [3] != 0.0);
}

//---------------------------------------------------------------------------------
//eof Illumination.cpp
