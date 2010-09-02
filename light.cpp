// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "light.h"
#include "global.h"
#include "texturemanager.h"

// external globals
extern int bEnableDeltaShading; // uvls.cpp

long lightMap [MAX_TEXTURES_D2];
long defLightMap [MAX_TEXTURES_D2];

//--------------------------------------------------------------------------
// light_weight()
//--------------------------------------------------------------------------

int FindLight (int nTexture, TEXTURE_LIGHT *pTexLights, int nLights)
{
	int	l = 0;
	int	r = nLights - 1;
	int	m, t;

while (l <= r) {
	m = (l + r) / 2;
	t = pTexLights [m].nBaseTex;
	if (nTexture > t)
		l = m + 1;
	else if (nTexture < t)
		r = m - 1;
	else
		return m;
	}
return -1;
}

//--------------------------------------------------------------------------

void CreateLightMap (void)
{
#if 1
theMine->LoadDefaultLightAndColor ();
#else
	TEXTURE_LIGHT	*pTexLights = (DLE.IsD1File ()) ? d1_texture_light : d2_texture_light;
	int				i = ((DLE.IsD1File ()) ? sizeof (d1_texture_light) : sizeof (d2_texture_light)) / sizeof (TEXTURE_LIGHT);

memset (lightMap, 0, sizeof (lightMap));
while (i) {
	--i;
	CBRK (pTexLights [i].m_info.nBaseTex == 0);
	lightMap [pTexLights [i].m_info.nBaseTex] = ((pTexLights [i].light * 100 + MAX_BRIGHTNESS / 2) / MAX_BRIGHTNESS) * (MAX_BRIGHTNESS / 100);
	}
memcpy (defLightMap, lightMap, sizeof (defLightMap));
#endif
}

//--------------------------------------------------------------------------

#if 0

BOOL HasCustomLightMap (void)
{
#ifdef _DEBUG
	int	i;

i = memcmp (lightMap, defLightMap, sizeof (lightMap));
if (i != 0) {
	for (i = 0; i < MAX_TEXTURES_D2; i++)
		if (lightMap [i] != defLightMap [i])
			return TRUE;
	}
return FALSE;
#else
return memcmp (lightMap, defLightMap, sizeof (lightMap)) != 0;
#endif
}

#endif

//--------------------------------------------------------------------------

int ReadLightMap (CFileManager& fp, uint nSize)
{
return fp.Read (lightMap, nSize, 1) != 1;
}

//--------------------------------------------------------------------------

int WriteLightMap (CFileManager& fp)
{
return fp.Write (lightMap, sizeof (lightMap), 1) != 1;
}

//--------------------------------------------------------------------------
// light_weight()
//--------------------------------------------------------------------------

byte CMine::LightWeight(short nBaseTex) 
{
#if 1
return (byte) ((lightMap [nBaseTex] - 1) / 0x0200L);
#else
  int i;

#	if 1
if (DLE.IsD1File ()) {
	i = FindLight (nBaseTex, d1_texture_light, NUM_LIGHTS_D1);
	if (i >= 0)
		return (byte) ((d1_texture_light [i].light - 1) / 0x0200L);
	}
else {
	i = FindLight (nBaseTex, d2_texture_light, NUM_LIGHTS_D2);
	if (i >= 0)
		return (byte) ((d2_texture_light [i].light - 1) / 0x0200L);
	}
return 0;
#	else
	byte result;

if (nBaseTex >= 0 && nBaseTex < MAX_TEXTURES) {
	if (DLE.IsD1File ()) {
		for (i=0;i<NUM_LIGHTS_D1;i++)
			if (nBaseTex <= d1_texture_light[i].m_info.nBaseTex) 
				break;
		if (nBaseTex == d1_texture_light[i].m_info.nBaseTex)
			result = (byte)((d1_texture_light[i].light - 1) / 0x0200L);
		}
	else {
		for (i=0;i<NUM_LIGHTS_D2;i++)
			if (nBaseTex <= d2_texture_light[i].m_info.nBaseTex) 
				break;
		if (nBaseTex == d2_texture_light[i].m_info.nBaseTex)
			result = (byte)((d2_texture_light[i].light - 1) / 0x0200L);
		}
	}
return (result);
#	endif
#endif
}

//------------------------------------------------------------------------
// GetFlickeringLight() - returns flicker light number
//
// returns -1 if not exists
//------------------------------------------------------------------------

short CMine::GetFlickeringLight (short nSegment, short nSide) 
{
current.Get (nSegment, nSide);
CFlickeringLight *flP = FlickeringLights (0);
int i;
for (i = FlickerLightCount (); i; i--, flP++)
	if ((flP->m_nSegment == nSegment) && (flP->m_nSide == nSide))
		break;
if (i > 0)
	return FlickerLightCount () - i;
return -1;
}



bool CMine::IsFlickeringLight (short nSegment, short nSide)
{
return GetFlickeringLight (nSegment, nSide) >= 0;
}

//------------------------------------------------------------------------
// add_flickering_light()
//
// returns index to newly created flickering light
//------------------------------------------------------------------------

short CMine::AddFlickeringLight (short nSegment, short nSide, uint mask,int time) 
{
current.Get (nSegment, nSide);
if (GetFlickeringLight (nSegment,nSide) != -1) {
	if (!bExpertMode)
		ErrorMsg ("There is already a flickering light on this side");
	return -1;
	}
// we are adding a new flickering light
if (FlickerLightCount () >= MAX_FLICKERING_LIGHTS) {
	if (!bExpertMode) {
		sprintf_s (message, sizeof (message),
					  "Maximum number of flickering lights (%d) have already been added",
					  MAX_FLICKERING_LIGHTS);
		ErrorMsg (message);
		}
	return -1;
	}
short nBaseTex = current.Side ()->m_info.nBaseTex & 0x1fff;
short nOvlTex = current.Side ()->m_info.nOvlTex & 0x1fff;
if ((IsLight (nBaseTex) == -1) && (IsLight (nOvlTex) == -1)) {
	if (!bExpertMode)
		ErrorMsg ("Blinking lights can only be added to a side\n"
					"that has a Texture with \" - light\" at the\n"
					"end of its name.");
	return -1;
	}
DLE.SetModified (TRUE);
CFlickeringLight *flP = FlickeringLights (FlickerLightCount ());
flP->m_nSegment = nSegment;
flP->m_nSide = nSide;
flP->m_info.delay = time;
flP->m_info.timer = time;
flP->m_info.mask = mask;
return ++FlickerLightCount ();
}

//------------------------------------------------------------------------
// delete_flickering_light()
//
// removes a flickering light if it exists
//------------------------------------------------------------------------

bool CMine::DeleteFlickeringLight (short nSegment, short nSide) 
{
if (nSegment < 0)
	nSegment = Current ()->nSegment;
if (nSide < 0)
	nSide = Current ()->nSide;
short index = GetFlickeringLight (nSegment, nSide);
if (index == -1) {
//ErrorMsg ("There is no flickering light on this side.");
	return false;
	}
DLE.SetModified (TRUE);
if (index < --FlickerLightCount ())
// put last light in place of deleted light
	memcpy (FlickeringLights (index), FlickeringLights (FlickerLightCount ()), sizeof (CFlickeringLight));
return true;
}

//------------------------------------------------------------------------
// IsLight()
//
// searches for a light in the list of lights
// returns index if found, -1 if not in the list
//------------------------------------------------------------------------

int CMine::IsLight (int nBaseTex) 
{
#if 1
return (lightMap [nBaseTex & 0x3fff] > 0) ? 0 : -1;
#else
#	if 1
return (IsD1File ()) ?
	FindLight (nBaseTex, d1_texture_light, NUM_LIGHTS_D1) : 
	FindLight (nBaseTex, d2_texture_light, NUM_LIGHTS_D2);
#	else
	int retval = -1;
	int i;
	TEXTURE_LIGHT *t;
	if (IsD1File ()) {
		i = NUM_LIGHTS_D1;
		t = d1_texture_light;
		}
	else {
		i = NUM_LIGHTS_D2;
		t = d2_texture_light;
		}
	for (; i; i--, t++)
		if (nBaseTex <= t->m_info.nBaseTex)
			break;
	if (nBaseTex == t->m_info.nBaseTex)
		return t - ((IsD1File ()) ? d1_texture_light : d2_texture_light);
	return -1;
#	if 0
	for (i=0;i<NUM_LIGHTS_D1;i++) {
		if (nBaseTex <= d1_texture_light[i].m_info.nBaseTex) break;
	if (nBaseTex > 0) {
		if (IsD1File ()) {
			for (i=0;i<NUM_LIGHTS_D1;i++) {
				if (nBaseTex <= d1_texture_light[i].m_info.nBaseTex) break;
			}
			if (nBaseTex == d1_texture_light[i].m_info.nBaseTex) {
				retval = i;
			}
		} else {
			for (i=0;i<NUM_LIGHTS_D2;i++) {
				if (nBaseTex <= d2_texture_light[i].m_info.nBaseTex) break;
			}
			if (nBaseTex == d2_texture_light[i].m_info.nBaseTex) {
				retval = i;
			}
		}
	}
	return retval;
#		endif
#	endif
#endif
}

//------------------------------------------------------------------------
// is_exploding_light()
//
//291 ceil024-l   292 ceil024b-f
//293 ceil025-l   294 ceil025b-f
//296 ceil028-l   297 ceil028b-f
//298 ceil029-l   299 ceil029b-f
//------------------------------------------------------------------------

int CMine::IsExplodingLight(int nBaseTex) 
{
	switch (nBaseTex) {
	case 291:
	case 292:
	case 293:
	case 294:
	case 296:
	case 297:
	case 298:
	case 299:
		return (1);
	}
	return(0);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CMine::IsBlastableLight (int nBaseTex) 
{
nBaseTex &= 0x3fff;
if (IsExplodingLight (nBaseTex))
	return true;
if (IsD1File ())
	return false;
for (short *p = d2_blastable_lights; *p >= 0; p++)
	if (*p == nBaseTex)
		return true;
#if 0
if (IsLight (nBaseTex))
	for (p = d2_switches; *p >= 0; p++)
		if (*p == nBaseTex)
			return true;
#endif
return false;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CMine::VisibleWall (ushort nWall)
{
if (nWall == NO_WALL)
	return false;
CWall	*wallP = Walls (nWall);
return (wallP->m_info.type != WALL_OPEN);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::SetCubeLight (double fLight, bool bAll, bool bDynCubeLights)
{
	long nLight = (int) (fLight * 65536); //24.0 * 327.68);
	CSegment *segP;
	int	h, i, j, l, c, nSegment;

DLE.SetModified (TRUE);
fLight /= 100.0;
for (nSegment = SegCount (), segP = Segments (0); nSegment; nSegment--, segP++) {
	if (bAll || (segP->m_info.wallFlags & MARKED_MASK)) {
		if (!bDynCubeLights)
			segP->m_info.staticLight = nLight;
		else {
			l = 0;
			c = 0;
			for (j = 0; j < 6; j++) {
				for (i = 0; i < 4; i++) {
					h = (ushort) segP->m_sides [j].m_info.uvls [i].l;
					if (h || ((segP->GetChild (j) == -1) && !VisibleWall (segP->m_sides [j].m_info.nWall))) {
						l += h;
						c++;
						}
					}
				}
			segP->m_info.staticLight = (int) (c ? fLight * ((double) l / (double) c) * 2 : nLight);
			}
		}
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::ScaleCornerLight (double fLight, bool bAll) 
{
	int segNum, segCount = SegCount ();
	double scale;

DLE.SetModified (TRUE);
scale = fLight / 100.0; // 100.0% = normal
//#pragma omp parallel 
	{
	//#pragma omp for
	for (segNum = 0; segNum < segCount; segNum++) {
		CSegment* segP = Segments (segNum);
		if (bAll || (segP->m_info.wallFlags & MARKED_MASK)) {
			for (int j = 0; j < 6; j++) {
				for (int i = 0; i < 4; i++) {
					double l = ((double) ((ushort) segP->m_sides [j].m_info.uvls [i].l)) * scale;
					l = min (l, 0x8000);
					l = max (l, 0);
					segP->m_sides [j].m_info.uvls [i].l = (ushort) l;
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

typedef struct tAvgCornerLight {
	ushort	light;
	byte		count;
} tAvgCornerLight;

void CMine::CalcAverageCornerLight (bool bAll)
{
  int nSegment, segCount = SegCount (), wallCount = MineInfo ().walls.count;
  tAvgCornerLight* max_brightness = new tAvgCornerLight [VertCount ()];

memset (max_brightness, 0, VertCount () * sizeof (tAvgCornerLight));

// smooth corner light by averaging all corners which share a vertex
DLE.SetModified (TRUE);
#pragma omp parallel 
	{
#pragma omp for
	for (nSegment = 0; nSegment < segCount; nSegment++) {
		CSegment *segP = Segments (nSegment);
		for (int pt = 0; pt < 8; pt++) {
			int nVertex = segP->m_info.verts [pt];
			if (bAll || (VertStatus (nSegment) & MARKED_MASK)) {
				for (int i = 0; i < 3; i++) {
					int nSide = pointSideTable [pt][i];
					if ((segP->GetChild (nSide) < 0) || (segP->m_sides [nSide].m_info.nWall < wallCount)) {
						int uvnum = pointCornerTable [pt][i];
						if (max_brightness [nVertex].light < ushort (segP->m_sides [nSide].m_info.uvls [uvnum].l))
							max_brightness [nVertex].light = ushort (segP->m_sides [nSide].m_info.uvls [uvnum].l);
						max_brightness [nVertex].count++;
						}
					}
				}
			}
		}
			//	max_brightness = min(max_brightness,0x8000L);
#pragma omp for
	for (nSegment = 0; nSegment < segCount; nSegment++) {
		CSegment *segP = Segments (nSegment);
		for (int pt = 0; pt < 8; pt++) {
			int nVertex = segP->m_info.verts [pt];
			if ((max_brightness [nVertex].count > 0) && (bAll || (VertStatus (nSegment) & MARKED_MASK))) {
				for (int i = 0; i < 3; i++) {
					int nSide = pointSideTable [pt][i];
					if ((segP->GetChild (nSide) < 0) || (segP->m_sides [nSide].m_info.nWall < wallCount)) {
						int uvnum = pointCornerTable [pt][i];
						segP->m_sides [nSide].m_info.uvls [uvnum].l = max_brightness [nVertex].light /*/ max_brightness [nVertex].count*/;
						}
					}
				}
			}
		}
	}
delete[] max_brightness;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::AutoAdjustLight (double fLightScale, bool bAll, bool bCopyTexLights) 
{
	int			nSegment;
	int			nTexture;
	int			nSide;
	uint		brightness;
	CSegment*	segP;
	CSide*		sideP;

// clear all lighting on marked cubes
DLE.SetModified (TRUE);
DLE.LockUndo ();
if (bAll)
	CLEAR (VertexColors ());
for (nSegment = SegCount (), segP = Segments (0); nSegment; nSegment--, segP++)
	if (bAll || (segP->m_info.wallFlags & MARKED_MASK))
		for (nSide=0, sideP = segP->m_sides;nSide < MAX_SIDES_PER_SEGMENT; nSide++, sideP++) {
			int i;
			for (i = 0; i < 4; i++) {
				sideP->m_info.uvls [i].l = 0;
				if (!bAll)
					VertexColors (segP->m_info.verts [sideVertTable [nSide][i]])->Clear ();
				}
			}

// Calculate cube side corner light values
// for each marked side in the level
// (range: 0 = min, 0x8000 = max)
for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++) {
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++) {
		if (!(bAll || SideIsMarked (nSegment, nSide)))
			continue;
		if ((segP->GetChild (nSide) >= 0) && !VisibleWall (sideP->m_info.nWall))
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
DLE.UnlockUndo ();
}

//--------------------------------------------------------------------------
// set_illumination()
//--------------------------------------------------------------------------
// blend two colors depending on each colors brightness
// psc: source color (light source)
// pdc: destination color (vertex/side color)
// srcBr: source brightness (remaining brightness at current vertex/side)
// destBr: vertex/side brightness

void CMine::BlendColors (CColor *srcColorP, CColor *destColorP, double srcBrightness, double destBrightness)
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

//--------------------------------------------------------------------------

void CMine::IlluminateSide (CSegment* segP, short nSide, uint brightness, CColor* lightColorP, double* effect, double fLightScale)
{
CUVL*		uvlP = segP->m_sides [nSide].m_info.uvls;
uint	vertBrightness, lightBrightness;
byte*	sideVerts = sideVertTable [nSide];

DLE.SetModified (TRUE);
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

//--------------------------------------------------------------------------

#ifdef _DEBUG
static int qqq1 = -1, qqq2 = 0;
#endif

void CMine::Illuminate (short nSourceSeg, short nSourceSide, uint brightness, double fLightScale, bool bAll, bool bCopyTexLights) 
{
	CSegment*	segP = Segments (0);
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
#if 1//def _OPENMP
short* visited = new short [SegCount ()];
memset (visited, 0xff, SegCount () * sizeof (*visited));
SetSegmentChildNum (null, nSourceSeg, m_lightRenderDepth, visited);	//mark all children that are at most lightRenderDepth segments away
visited [nSourceSeg] = m_lightRenderDepth;
#else
int i;
for (i = SegCount (); i; )
	Segments (--i)->m_info.nIndex = -1;
SetSegmentChildNum (null, nSourceSeg, m_lightRenderDepth);
segP->m_info.nIndex = m_lightRenderDepth;
#endif

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
bool bWall = false; //FindWall (nSourceSeg, nSourceSide) != null;
// loop on child Segments ()
int nChildSeg;
int nSegCount = SegCount ();
#pragma omp parallel 
	{
#pragma omp for private (effect)
	for (nChildSeg = 0; nChildSeg < nSegCount; nChildSeg++) {
		CSegment* childSegP = Segments (nChildSeg);
		// skip if this is too far away
#if 1//def _OPENMP
		if (visited [nChildSeg] < 0)
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
			if (childSegP->GetChild (nChildSide) >= 0) {
				ushort nWall = childSegP->m_sides [nChildSide].m_info.nWall;
				// .. but there is no wall ..
				if (nWall >= MineInfo ().walls.count)
					continue;
					// .. or its not a door ..
				if (Walls (nWall)->m_info.type == WALL_OPEN)
					continue;
				}

	//		CBRK (segColorP->m_info.index > 0);
			// if the child side is the same as the source side, then set light and continue
			if ((nChildSide == nSourceSide) && (nChildSeg == nSourceSeg)) {
#if 1
				IlluminateSide (childSegP, nChildSide, brightness, lightColorP, null, fLightScale);
#else
				CUVL*		uvlP = childSegP->m_sides [nChildSide].m_info.uvls;
				uint	vertBrightness, lightBrightness;

				DLE.SetModified (TRUE);
				for (int j = 0; j < 4; j++, uvlP++) {
					CColor* vertColorP = VertexColors (childSegP->m_info.verts [sideVertTable [nChildSide][j]]);
					vertBrightness = (ushort) uvlP->l;
					lightBrightness = (uint) (brightness * fLightScale);
					BlendColors (lightColorP, vertColorP, lightBrightness, vertBrightness);
					vertBrightness += lightBrightness;
					vertBrightness = min (0x8000, vertBrightness);
					uvlP->l = (ushort) vertBrightness;
					}
#endif
				continue;
				}

			// calculate vector between center of source segment and center of child
	//		CBRK (nChildSeg == 1 && nChildSide == 2);
			if (CalcSideLights (nChildSeg, nChildSide, sourceCenter, sourceCorners, A, effect, fLightScale, bWall)) {
#if 1
				IlluminateSide (childSegP, nChildSide, brightness, lightColorP, effect, fLightScale);
#else
					uint	vertBrightness, lightBrightness;	//vertex brightness, light brightness
					CUVL		*uvlP = childSegP->m_sides [nChildSide].m_info.uvls;

				DLE.SetModified (TRUE);
				for (int j = 0; j < 4; j++, uvlP++) {
					CColor* vertColorP = VertexColors (childSegP->m_info.verts [sideVertTable [nChildSide][j]]);
					vertBrightness = (ushort) uvlP->l;
					lightBrightness = (ushort) (brightness * effect [j] / 32);
					BlendColors (lightColorP, vertColorP, lightBrightness, vertBrightness);
					vertBrightness += lightBrightness;
					vertBrightness = min (0x8000, vertBrightness);
					uvlP->l = (ushort) vertBrightness;
					}
#endif
				}
			}
		}
	}
#if 1//def _OPENMP
delete[] visited;
#endif
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
// to some flickering lights not being visible in Descent:
// For each recursion step, the delta light calculation will be executed
// for all children of sides with flickering lights with exactly that
// distance (recursionDepth) to their parent. In case of too many delta 
// light values for the full lighting depth at least the lighting of the
// Segments () close to flickering lights will be computed properly.
//------------------------------------------------------------------------

void CMine::CalcDeltaLightData(double fLightScale, int force) 
{
DLE.SetModified (TRUE);
int recursion_depth;
for (recursion_depth = m_deltaLightRenderDepth; recursion_depth; recursion_depth--)
	if (CalcDeltaLights (fLightScale, force, recursion_depth))
		break;
}



bool CMine::IsLava (int nBaseTex)
{
return (strstr (textureManager.Name ((short) nBaseTex), "lava") != null);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

int CMine::FindDeltaLight (short nSegment, short nSide, short *pi)
{
	int	i = pi ? *pi : 0;
	int	j	= (int)MineInfo ().lightDeltaIndices.count++;
	CLightDeltaIndex	*dliP = LightDeltaIndex (0);

if ((LevelVersion () >= 15) && (MineInfo ().fileInfo.version >= 34)) {
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

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CMine::CalcDeltaLights (double fLightScale, int force, int recursion_depth) 
{
	// initialize totals
	int		nSegCount = SegCount ();
	int		nSourceSeg;
	int		nErrors = 0;
	double	effect[4];

MineInfo ().lightDeltaValues.count = 0;
MineInfo ().lightDeltaIndices.count = 0;
bool bWall, bD2XLights = (LevelVersion () >= 15) && (MineInfo ().fileInfo.version >= 34);

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
		for (int nSourceSide = 0; nSourceSide < 6; nSourceSide++) {
			short nTexture = srcSegP->m_sides [nSourceSide].m_info.nBaseTex & 0x3fff;
			short tmapnum2 = srcSegP->m_sides [nSourceSide].m_info.nOvlTex & 0x3fff;
			short nTrigger;
			bool bl1 = (bool) (IsLight (nTexture) != -1);
			bool bl2 = (bool) (IsLight (tmapnum2) != -1);
			if (!(bl1 || bl2))
				continue;	// no lights on this side
			bool bCalcDeltas = false;
			// if the current side is a wall and has a light and is the target of a trigger
			// than can make the wall appear/disappear, calculate delta lights for it
			if ((bWall = (FindWall (nSourceSeg, nSourceSide) != null)) &&
				 ((nTrigger = FindTriggerTarget (0, nSourceSeg, nSourceSide)) >= 0)) {
				char trigtype = Triggers (nTrigger)->m_info.type;
				bCalcDeltas =
					(trigtype == TT_ILLUSION_OFF) ||
					(trigtype == TT_ILLUSION_ON) ||
					(trigtype == TT_CLOSE_WALL) ||
					(trigtype == TT_OPEN_WALL) ||
					(trigtype == TT_LIGHT_OFF) ||
					(trigtype == TT_LIGHT_ON);
					 
				}
			if (!bCalcDeltas)
				bCalcDeltas = IsFlickeringLight (nSourceSeg, nSourceSide);
			if (!bCalcDeltas) {
				bool bb1 = IsBlastableLight (nTexture);
				bool bb2 = IsBlastableLight (tmapnum2);
				if (bb1 == bb2)
					bCalcDeltas = bb1;	// both lights blastable or not
				else if (!(bb1 ? bl2 : bl1))	// i.e. one light blastable and the other texture not a non-blastable light 
					bCalcDeltas = true;
				}
			if (!bCalcDeltas) {	//check if light is target of a "light on/off" trigger
				int nTrigger = FindTriggerTarget (0, nSourceSeg, nSourceSide);
				if ((nTrigger >= 0) && (Triggers (nTrigger)->m_info.type >= TT_LIGHT_OFF))
					bCalcDeltas = true;
				}
			if (!bCalcDeltas)
				continue;

			short srcwall = srcSegP->m_sides [nSourceSide].m_info.nWall;
			if ((srcSegP->GetChild (nSourceSide) != -1) &&
				 ((srcwall >= MineInfo ().walls.count) || (Walls (srcwall)->m_info.type == WALL_OPEN)))
				continue;

			if (MineInfo ().lightDeltaIndices.count >= MAX_LIGHT_DELTA_INDICES) {
//#pragma omp critical
				{
				if (++nErrors == 1) {
					char szMsg [256];
					sprintf_s (szMsg, sizeof (szMsg), " Light tool: Too many dynamic lights at render depth %d", recursion_depth);
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
			lightDeltaIndexCount = int (MineInfo ().lightDeltaIndices.count++);
			}
			CLightDeltaIndex *dliP = LightDeltaIndex (lightDeltaIndexCount);
			if (bD2XLights) {
				dliP->m_nSegment = nSourceSeg;
				dliP->m_nSide = nSourceSide;
				dliP->m_info.count = 0; // will be incremented below
				}
			else {
				dliP->m_nSegment = nSourceSeg;
				dliP->m_nSide = nSourceSide;
				dliP->m_info.count = 0; // will be incremented below
				}
			dliP->m_info.index = (short)MineInfo ().lightDeltaValues.count;

			// find orthogonal angle of source segment
			A = -CalcSideNormal (nSourceSeg,nSourceSide);

			// calculate the center of the source segment
			sourceCenter = CalcSideCenter (nSourceSeg, nSourceSide);

			// mark those Segments () within N children of current cube
			//(note: this is done once per light instead of once per segment
			//       even though some Segments () have multiple lights.
			//       This actually reduces the number of calls since most
			//       Segments () do not have lights)
	#if 1
			short* visited = new short [nSegCount];
			memset (visited, 0xff, nSegCount * sizeof (*visited));
			SetSegmentChildNum (srcSegP, nSourceSeg, recursion_depth, visited);
			visited [nSourceSeg] = recursion_depth;
	#else
			int h;
			for (h = 0; h < SegCount (); h++)
				Segments (h)->m_info.nIndex = -1;
			SetSegmentChildNum (srcSegP, nSourceSeg, recursion_depth);
			srcSegP->m_info.nIndex = recursion_depth;
	#endif

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
	#if 1
				if (visited [nChildSeg] < 0)
					continue;
				CSegment* childSegP = Segments (nChildSeg);
	#else
				CSegment* childSegP = Segments (nChildSeg);
				if (childSegP->m_info.nIndex < 0)
					continue;
	#endif
				// loop on child sides
				for (int nChildSide = 0; nChildSide < 6; nChildSide++) {
					// if texture has a child..
					if (childSegP->GetChild (nChildSide) >= 0) {
						ushort nWall = childSegP->m_sides[nChildSide].m_info.nWall;
						// .. if there is no wall ..
						if (nWall >= MineInfo ().walls.count)
							continue;
						// .. or its not a door ..
						if (Walls (nWall)->m_info.type == WALL_OPEN) 
							continue; // don't put light because there is no texture here
						}
					// don't affect non-flickering light emitting textures (e.g. lava)
					nTexture = childSegP->m_sides [nChildSide].m_info.nBaseTex;
					tmapnum2 = childSegP->m_sides [nChildSide].m_info.nOvlTex & 0x3fff;
					if (m_nNoLightDeltas == 1) {
						if (((IsLight (nTexture) >= 0) || (IsLight (tmapnum2) >= 0))
							 && !IsFlickeringLight (nChildSeg, nChildSide))
							continue;
						}
					else if ((m_nNoLightDeltas == 2) && (IsLava (nTexture) || IsLava (tmapnum2)))
						continue;
					// if the child side is the same as the source side, then set light and continue
					if (nChildSide == nSourceSide && nChildSeg == nSourceSeg) {
						if ((MineInfo ().lightDeltaValues.count >= MAX_LIGHT_DELTA_VALUES) || (dliP->m_info.count == (bD2XLights ? 8191 : 255))) {
//#pragma omp critical
							{
							if (++nErrors == 1) {
								char szMsg [256];
								sprintf_s (szMsg, sizeof (szMsg), " Light tool: Too many dynamic lights at render depth %d", recursion_depth);
								DEBUGMSG (szMsg);
								}
							}
							continue;
							}
						CLightDeltaValue* dl;
	//#pragma omp critical
						{
						dl = LightDeltaValues (MineInfo ().lightDeltaValues.count++);
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
							DLE.SetModified (TRUE);
							if ((MineInfo ().lightDeltaValues.count >= MAX_LIGHT_DELTA_VALUES) || (bD2XLights ? dliP->m_info.count == 8191 : dliP->m_info.count == 255)) {
//#pragma omp critical
								{
								if (++nErrors == 1) {
									char szMsg [256];
									sprintf_s (szMsg, sizeof (szMsg), " Light tool: Too many dynamic lights at render depth %d", recursion_depth);
									DEBUGMSG (szMsg);
									}
								}
								continue;
								}
							CLightDeltaValue *dl;
//#pragma omp critical
							{
							dl = LightDeltaValues (MineInfo ().lightDeltaValues.count++);
							}
							dl->m_nSegment = nChildSeg;
							dl->m_nSide = nChildSide;
							int iCorner;
							for (iCorner = 0; iCorner < 4; iCorner++)
								dl->m_info.vertLight [iCorner] = (byte) min(32, effect [iCorner]);
							if (bD2XLights)
								dliP->m_info.count++;
							else
								dliP->m_info.count++;
							}
						}
					}
	//			}
#if 1
			delete[] visited;
#endif
			}
		}
	}
return (nErrors == 0);
}

//--------------------------------------------------------------------------
// SetSegmentChildNum()
//
// Action - Sets nIndex to child number from parent
//--------------------------------------------------------------------------

void CMine::UnlinkSeg (CSegment *pSegment, CSegment *pRoot)
{
#if 0
	short	prevSeg = pSegment->prevSeg;
	short	nextSeg = pSegment->nextSeg;
	short	thisSeg = pSegment - Segments ();

CBRK ((prevSeg >= 0) && (Segments (prevSeg)->nextSeg < 0));
if (prevSeg >= 0) {
	Segments () [prevSeg].nextSeg = nextSeg;
	pSegment->prevSeg = -1;
	}
if (nextSeg >= 0) {
	Segments () [nextSeg].prevSeg = prevSeg;
	pSegment->nextSeg = -1;
	}
#endif
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::LinkSeg (CSegment *pSegment, CSegment *pRoot)
{
#if 0
	short	prevSeg = pRoot->prevSeg;
	short	nextSeg = pRoot->nextSeg;
	short thisSeg = pSegment - Segments ();
	short rootSeg = pRoot - Segments ();

if (prevSeg < 0) {
	pRoot->nextSeg = thisSeg;
	}
else {
	Segments () [prevSeg].nextSeg = thisSeg;
	pSegment->prevSeg = prevSeg;
	}
pRoot->prevSeg = thisSeg;
pSegment->nextSeg = rootSeg;
pSegment->rootSeg = rootSeg;
#endif
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::SetSegmentChildNum (CSegment *pRoot, short nSegment, short recursion_level) 
{
	short			nSide, child, nImprove = 0;
	ushort		nWall;
	CSegment	*segP = Segments (nSegment);
	CSegment	*prevSeg = null;
	bool			bMarkChildren = false;

// mark each child if child number is lower
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	// Skip if this is a door
	nWall = segP->m_sides [nSide].m_info.nWall;
	// .. if there is a wall and its a door
	if ((nWall < MineInfo ().walls.count) && (Walls (nWall)->m_info.type == WALL_DOOR))
		continue;
	// mark segment if it has a child
	child = segP->GetChild (nSide);
	if ((child > -1) && (child < SegCount ()) && (recursion_level > segP->m_info.nIndex)) {
		if (segP->m_info.nIndex >= 0)
			++nImprove;
/*
		if (pRoot) {
			UnlinkSeg (segP, pRoot);
			LinkSeg (segP, pRoot);
			}
*/
		segP->m_info.nIndex = recursion_level;
		bMarkChildren = true;
		break;
		}
	}
//return if segment has no children or max recursion depth is reached
if (!bMarkChildren || (recursion_level == 1))
	return;

// check each side of this segment for more children
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	// skip if there is a wall and its a door
	nWall = segP->m_sides [nSide].m_info.nWall;
	if ((nWall < MineInfo ().walls.count) && (Walls (nWall)->m_info.type == WALL_DOOR))
		continue;
	// check child
	child = segP->GetChild (nSide);
	if ((child > -1) && (child < SegCount ()))
		SetSegmentChildNum (pRoot, child, recursion_level - 1);
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::SetSegmentChildNum (CSegment *pRoot, short nSegment, short recursion_level, short* visited) 
{
	short			nSide, child, nImprove = 0;
	ushort		nWall;
	CSegment		*segP = Segments (nSegment);
	CSegment		*prevSeg = null;
	bool			bMarkChildren = false;

// mark each child if child number is lower
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	// Skip if this is a door
	nWall = segP->m_sides [nSide].m_info.nWall;
	// .. if there is a wall and its a door
	if ((nWall < MineInfo ().walls.count) && (Walls (nWall)->m_info.type == WALL_DOOR))
		continue;
	// mark segment if it has a child
	child = segP->GetChild (nSide);
	if ((child > -1) && (child < SegCount ()) && (recursion_level > visited [nSegment])) {
		if (visited [nSegment] >= 0)
			++nImprove;
/*
		if (pRoot) {
			UnlinkSeg (segP, pRoot);
			LinkSeg (segP, pRoot);
			}
*/
		visited [nSegment] = recursion_level;
		bMarkChildren = true;
		break;
		}
	}
//return if segment has no children or max recursion depth is reached
if (!bMarkChildren || (recursion_level == 1))
	return;

// check each side of this segment for more children
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	// skip if there is a wall and its a door
	nWall = segP->m_sides [nSide].m_info.nWall;
	if ((nWall < MineInfo ().walls.count) && (Walls (nWall)->m_info.type == WALL_DOOR))
		continue;
	// check child
	child = segP->GetChild (nSide);
	if ((child > -1) && (child < SegCount ()))
		SetSegmentChildNum (pRoot, child, recursion_level - 1, visited);
	}
}

//--------------------------------------------------------------------------

bool CMine::CalcSideLights (int nSegment, int nSide, 
									 CVertex& sourceCenter, CVertex* sourceCorners, CVertex& vertex, 
									 double* effect, double fLightScale, bool bIgnoreAngle)
{
	CSegment *segP = Segments (nSegment);
// calculate vector between center of source segment and center of child
CVertex center = CalcSideCenter (nSegment, nSide);
CVertex A = vertex;
CVertex B = center - sourceCenter;

// calculate angle between vectors (use dot product equation)
if (!bIgnoreAngle) {
	double ratio, angle;
	double A_dot_B = A ^ B;
	double mag_A = A.Mag ();
	double mag_B = B.Mag ();
	if (mag_A == 0.0 || mag_B == 0.0)
		return false;
	ratio = A_dot_B / (mag_A * mag_B);
	ratio = ((double) ((int) (ratio * 1000.0))) / 1000.0;
	if (ratio < -1.0 || ratio > 1.0) 
		return false;
	angle = acos (ratio);  // force a failure
	// if angle is less than 180 degrees
	// then we found a match
	if (angle >= Radians (180.0))
		return false;
	}
for (int j = 0; j < 4; j++) {
	CVertex* corner = Vertices (segP->m_info.verts [sideVertTable [nSide][j]]);
	double length = 20.0 * m_lightRenderDepth;
	for (int i = 0; i < 4; i++)
		length = min (length, Distance (sourceCorners [i], *corner));
	length /= 10.0 * m_lightRenderDepth / 6.0; // divide by 1/2 a cubes length so opposite side
	// light is recuded by 1/4
	effect [j] = fLightScale;
	if (length > 1.0)//if (length < 20.0 * m_lightRenderDepth) // (roughly 4 standard cube lengths)
		effect [j] /= (length * length);
//	else
//		effect [j] = 0;
	}
// if any of the effects are > 0, then increment the
// light for that side
return (effect [0] != 0 || effect [1] != 0 || effect [2] != 0 || effect [3] != 0);
}

//--------------------------------------------------------------------------

CColor *CMine::LightColor (int i, int j, bool bUseTexColors) 
{ 
if (bUseTexColors && UseTexColors ()) {
	CWall *wallP = SideWall (i, j);
	//if (!wallP || (wallP->m_info.type != WALL_TRANSPARENT)) 
		{	//always use a side color for transp. walls
		CColor *pc;
		short t = Segments (i)->m_sides [j].m_info.nOvlTex & 0x3fff;
		if ((t > 0) && (pc = GetTexColor (t)))
			return pc;
		if (pc = GetTexColor (Segments (i)->m_sides [j].m_info.nBaseTex, wallP && (wallP->m_info.type == WALL_TRANSPARENT)))
			return pc;
		}
	}	
return LightColors (i, j); 
}

//--------------------------------------------------------------------------
//eof light.cpp