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

// external globals
extern INT32 bEnableDeltaShading; // uvls.cpp

long lightMap [MAX_D2_TEXTURES];
long defLightMap [MAX_D2_TEXTURES];

//--------------------------------------------------------------------------
// light_weight()
//--------------------------------------------------------------------------

INT32 FindLight (INT32 nTexture, TEXTURE_LIGHT *pTexLights, INT32 nLights)
{
	INT32	l = 0;
	INT32	r = nLights - 1;
	INT32	m, t;

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
	TEXTURE_LIGHT	*pTexLights = (theApp.IsD1File ()) ? d1_texture_light : d2_texture_light;
	INT32				i = ((theApp.IsD1File ()) ? sizeof (d1_texture_light) : sizeof (d2_texture_light)) / sizeof (TEXTURE_LIGHT);

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
	INT32	i;

i = memcmp (lightMap, defLightMap, sizeof (lightMap));
if (i != 0) {
	for (i = 0; i < MAX_D2_TEXTURES; i++)
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

INT32 ReadLightMap (FILE *fLightMap, UINT32 nSize)
{
return fread (lightMap, nSize, 1, fLightMap) != 1;
}

//--------------------------------------------------------------------------

INT32 WriteLightMap (FILE *fLightMap)
{
return fwrite (lightMap, sizeof (lightMap), 1, fLightMap) != 1;
}

//--------------------------------------------------------------------------
// light_weight()
//--------------------------------------------------------------------------

UINT8 CMine::LightWeight(INT16 nBaseTex) 
{
#if 1
return (UINT8) ((lightMap [nBaseTex] - 1) / 0x0200L);
#else
  INT32 i;

#	if 1
if (theApp.IsD1File ()) {
	i = FindLight (nBaseTex, d1_texture_light, NUM_LIGHTS_D1);
	if (i >= 0)
		return (UINT8) ((d1_texture_light [i].light - 1) / 0x0200L);
	}
else {
	i = FindLight (nBaseTex, d2_texture_light, NUM_LIGHTS_D2);
	if (i >= 0)
		return (UINT8) ((d2_texture_light [i].light - 1) / 0x0200L);
	}
return 0;
#	else
	UINT8 result;

if (nBaseTex >= 0 && nBaseTex < MAX_TEXTURES) {
	if (theApp.IsD1File ()) {
		for (i=0;i<NUM_LIGHTS_D1;i++)
			if (nBaseTex <= d1_texture_light[i].m_info.nBaseTex) 
				break;
		if (nBaseTex == d1_texture_light[i].m_info.nBaseTex)
			result = (UINT8)((d1_texture_light[i].light - 1) / 0x0200L);
		}
	else {
		for (i=0;i<NUM_LIGHTS_D2;i++)
			if (nBaseTex <= d2_texture_light[i].m_info.nBaseTex) 
				break;
		if (nBaseTex == d2_texture_light[i].m_info.nBaseTex)
			result = (UINT8)((d2_texture_light[i].light - 1) / 0x0200L);
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

INT16 CMine::GetFlickeringLight (INT16 nSegment, INT16 nSide) 
{
GetCurrent (nSegment, nSide);
CFlickeringLight *pfl = FlickeringLights (0);
INT32 i;
for (i = FlickerLightCount (); i; i--, pfl++)
	if ((pfl->m_nSegment == nSegment) && (pfl->m_nSide == nSide))
		break;
if (i > 0)
	return FlickerLightCount () - i;
return -1;
}



bool CMine::IsFlickeringLight (INT16 nSegment, INT16 nSide)
{
return GetFlickeringLight (nSegment, nSide) >= 0;
}

//------------------------------------------------------------------------
// add_flickering_light()
//
// returns index to newly created flickering light
//------------------------------------------------------------------------

INT16 CMine::AddFlickeringLight (INT16 nSegment, INT16 nSide, UINT32 mask,FIX time) 
{
GetCurrent (nSegment, nSide);
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
INT16 nBaseTex = CurrSide ()->m_info.nBaseTex & 0x1fff;
INT16 nOvlTex = CurrSide ()->m_info.nOvlTex & 0x1fff;
if ((IsLight (nBaseTex) == -1) && (IsLight (nOvlTex) == -1)) {
	if (!bExpertMode)
		ErrorMsg ("Blinking lights can only be added to a side\n"
					"that has a Texture with \" - light\" at the\n"
					"end of its name.");
	return -1;
	}
theApp.SetModified (TRUE);
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

bool CMine::DeleteFlickeringLight(INT16 nSegment, INT16 nSide) 
{
if (nSegment < 0)
	nSegment = Current ()->nSegment;
if (nSide < 0)
	nSide = Current ()->nSide;
INT16 index = GetFlickeringLight (nSegment, nSide);
if (index == -1) {
//ErrorMsg ("There is no flickering light on this side.");
	return false;
	}
theApp.SetModified (TRUE);
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

INT32 CMine::IsLight (INT32 nBaseTex) 
{
#if 1
return (lightMap [nBaseTex & 0x3fff] > 0) ? 0 : -1;
#else
#	if 1
return (IsD1File ()) ?
	FindLight (nBaseTex, d1_texture_light, NUM_LIGHTS_D1) : 
	FindLight (nBaseTex, d2_texture_light, NUM_LIGHTS_D2);
#	else
	INT32 retval = -1;
	INT32 i;
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

INT32 CMine::IsExplodingLight(INT32 nBaseTex) 
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

bool CMine::IsBlastableLight (INT32 nBaseTex) 
{
nBaseTex &= 0x3fff;
if (IsExplodingLight (nBaseTex))
	return true;
if (IsD1File ())
	return false;
for (INT16 *p = d2_blastable_lights; *p >= 0; p++)
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

bool CMine::VisibleWall (UINT16 nWall)
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
	long nLight = (FIX) (fLight * 65536); //24.0 * 327.68);
	CSegment *segP;
	INT32	h, i, j, l, c, nSegment;

theApp.SetModified (TRUE);
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
					h = (UINT16) segP->m_sides [j].m_info.uvls [i].l;
					if (h || ((segP->m_info.children [j] == -1) && !VisibleWall (segP->m_sides [j].m_info.nWall))) {
						l += h;
						c++;
						}
					}
				}
			segP->m_info.staticLight = (FIX) (c ? fLight * ((double) l / (double) c) * 2 : nLight);
			}
		}
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::ScaleCornerLight (double fLight, bool bAll) 
{
	INT32 segNum, segCount = SegCount ();
	double scale;

theApp.SetModified (TRUE);
scale = fLight / 100.0; // 100.0% = normal
//#pragma omp parallel 
	{
	//#pragma omp for
	for (segNum = 0; segNum < segCount; segNum++) {
		CSegment* segP = Segments (segNum);
		if (bAll || (segP->m_info.wallFlags & MARKED_MASK)) {
			for (INT32 j = 0; j < 6; j++) {
				for (INT32 i = 0; i < 4; i++) {
					double l = ((double) ((UINT16) segP->m_sides [j].m_info.uvls [i].l)) * scale;
					l = min (l, 0x8000);
					l = max (l, 0);
					segP->m_sides [j].m_info.uvls [i].l = (UINT16) l;
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

typedef struct tAvgCornerLight {
	UINT16	light;
	UINT8		count;
} tAvgCornerLight;

void CMine::CalcAverageCornerLight (bool bAll)
{
#if 0

  INT32 nSegment,pt,i,nVertex,count,nSide,uvnum;
  UINT16 max_brightness;

// smooth corner light by averaging all corners which share a vertex
theApp.SetModified (TRUE);
for (nVertex = 0; nVertex < VertCount (); nVertex++) {
	if (bAll || (Vertices (i)->Marked ())) {
		max_brightness = 0;
		count = 0;
		// find all Segments () which share this point
		CSegment *segP = Segments (0);
		for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++) {
			for (pt = 0; pt < 8; pt++) {
				if (segP->m_info.verts[pt] == nVertex) {
					// add all the uvls for this point
					for (i = 0; i < 3; i++) {
						nSide = pointSideTable[pt][i];
						uvnum = pointCornerTable[pt][i];
						if ((segP->m_info.children[nSide] < 0) || 
							 (segP->m_sides[nSide].m_info.nWall < GameInfo ().walls.count)) {
#if 1
							max_brightness = max(max_brightness,(UINT16)segP->m_sides[nSide].m_info.uvls[uvnum].l);
#else
							if (max_brightness < (UINT16)segP->m_sides[nSide].m_info.uvls[uvnum].l)
								max_brightness = (UINT16)segP->m_sides[nSide].m_info.uvls[uvnum].l;
#endif
							count++;
							}
						}
					}
				}
			}
		// now go back and set these light values
		if (count > 0) {
			theApp.SetModified (TRUE);
			//	max_brightness = min(max_brightness,0x8000L);
			CSegment *segP = Segments (0);
			for (nSegment=0;nSegment<SegCount ();nSegment++, segP++) {
				for (pt=0;pt<8;pt++) {
					if (segP->m_info.verts[pt] == nVertex) {
						for (i=0;i<3;i++) {
							nSide = pointSideTable[pt][i];
							uvnum = pointCornerTable[pt][i];
							if ((segP->m_info.children[nSide] < 0) || 
								 (segP->m_sides[nSide].m_info.nWall < GameInfo ().walls.count)) {
								segP->m_sides[nSide].m_info.uvls[uvnum].l = max_brightness;
								}
							}
						}
					}
				}
			}
		}
	}

#else

  INT32 nSegment, segCount = SegCount (), wallCount = GameInfo ().walls.count;
  tAvgCornerLight* max_brightness = new tAvgCornerLight [VertCount ()];

memset (max_brightness, 0, VertCount () * sizeof (tAvgCornerLight));

// smooth corner light by averaging all corners which share a vertex
theApp.SetModified (TRUE);
#pragma omp parallel 
	{
#pragma omp for
	for (nSegment = 0; nSegment < segCount; nSegment++) {
		CSegment *segP = Segments (nSegment);
		for (INT32 pt = 0; pt < 8; pt++) {
			INT32 nVertex = segP->m_info.verts [pt];
			if (bAll || (VertStatus (nSegment) & MARKED_MASK)) {
				for (INT32 i = 0; i < 3; i++) {
					INT32 nSide = pointSideTable [pt][i];
					if ((segP->m_info.children [nSide] < 0) || (segP->m_sides [nSide].m_info.nWall < wallCount)) {
						INT32 uvnum = pointCornerTable [pt][i];
						if (max_brightness [nVertex].light < UINT16 (segP->m_sides [nSide].m_info.uvls [uvnum].l))
							max_brightness [nVertex].light = UINT16 (segP->m_sides [nSide].m_info.uvls [uvnum].l);
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
		for (INT32 pt = 0; pt < 8; pt++) {
			INT32 nVertex = segP->m_info.verts [pt];
			if ((max_brightness [nVertex].count > 0) && (bAll || (VertStatus (nSegment) & MARKED_MASK))) {
				for (INT32 i = 0; i < 3; i++) {
					INT32 nSide = pointSideTable [pt][i];
					if ((segP->m_info.children [nSide] < 0) || (segP->m_sides [nSide].m_info.nWall < wallCount)) {
						INT32 uvnum = pointCornerTable [pt][i];
						segP->m_sides [nSide].m_info.uvls [uvnum].l = max_brightness [nVertex].light /*/ max_brightness [nVertex].count*/;
						}
					}
				}
			}
		}
	}
delete[] max_brightness;

#endif
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::AutoAdjustLight (double fLightScale, bool bAll, bool bCopyTexLights) 
{
	INT32			nSegment;
	INT32			texture_num;
	INT32			nSide;
	UINT32		brightness;
	CSegment	*segP;
	CSide		*sideP;

// clear all lighting on marked cubes
theApp.SetModified (TRUE);
theApp.LockUndo ();
if (bAll)
	CLEAR (VertexColors ());
for (nSegment = SegCount (), segP = Segments (0); nSegment; nSegment--, segP++)
	if (bAll || (segP->m_info.wallFlags & MARKED_MASK))
		for (nSide=0, sideP = segP->m_sides;nSide < MAX_SIDES_PER_SEGMENT; nSide++, sideP++) {
			INT32 i;
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
		if ((segP->m_info.children [nSide] >= 0) && !VisibleWall (sideP->m_info.nWall))
			continue;
		if (bCopyTexLights)
			memset (LightColor (nSegment, nSide, false), 0, sizeof (CColor));
		brightness = 0;
		texture_num = sideP->m_info.nBaseTex;
		if ((texture_num >= 0) && (texture_num < MAX_TEXTURES))
			brightness = max (brightness, LightWeight (texture_num));
		texture_num = sideP->m_info.nOvlTex & 0x3fff;
		if ((texture_num > 0) && (texture_num < MAX_TEXTURES))
			brightness = max (brightness, LightWeight (texture_num));
		if (brightness > 0)
			Illuminate (nSegment, nSide, (UINT32) (brightness * 2 * fLightScale), 1.0, bAll, bCopyTexLights);
		}
	}
theApp.UnlockUndo ();
}

//--------------------------------------------------------------------------
// set_illumination()
//--------------------------------------------------------------------------
// blend two colors depending on each colors brightness
// psc: source color (light source)
// pdc: destination color (vertex/side color)
// srcBr: source brightness (remaining brightness at current vertex/side)
// destBr: vertex/side brightness

void CMine::BlendColors (CColor *psc, CColor *pdc, double srcBr, double destBr)
{
if (destBr)
	destBr /= 65536.0;
else {
	if (srcBr)
		*pdc = *psc;
	return;
	}
if (srcBr)
	srcBr /= 65536.0;
else
	return;
//#pragma omp critical
	{
	if (pdc->index) {
		pdc->color.r += psc->color.r * srcBr;
		pdc->color.g += psc->color.g * srcBr;
		pdc->color.b += psc->color.b * srcBr;
		double cMax = pdc->color.r;
			if (cMax < pdc->color.g)
				cMax = pdc->color.g;
			if (cMax < pdc->color. b)
				cMax = pdc->color. b;
			if (cMax > 1) {
				pdc->color.r /= cMax;
				pdc->color.g /= cMax;
				pdc->color.b /= cMax;
				}
		}
	else {
		if (destBr) {
			pdc->index = 1;
			pdc->color.r = psc->color.r * destBr;
			pdc->color.g = psc->color.g * destBr;
			pdc->color.b = psc->color.b * destBr;
			}
		else
			*pdc = *psc;
		}
	}
}

//--------------------------------------------------------------------------

#ifdef _DEBUG
static INT32 qqq1 = -1, qqq2 = 0;
#endif

void CMine::Illuminate (
	INT16 nSourceSeg, 
	INT16 nSourceSide, 
	UINT32 brightness, 
	double fLightScale, 
	bool bAll, 
	bool bCopyTexLights) 
{
	CSegment*		segP = Segments (0);
	double			effect[4];
	// find orthogonal angle of source segment
	CFixVector		A;

//fLightScale /= 100.0;
A = -CalcSideNormal (nSourceSeg, nSourceSide);
// remember to flip the sign since we want it to point inward
// calculate the center of the source segment
CFixVector source_center;
source_center = CalcSideCenter (nSourceSeg, nSourceSide);
// mark those Segments () within N children of current cube

// set child numbers
//Segments ()[nSourceSeg].nIndex = m_lightRenderDepth;

segP = Segments (nSourceSeg);
#if 1//def _OPENMP
INT16* visited = new INT16 [SegCount ()];
memset (visited, 0xff, SegCount () * sizeof (*visited));
SetSegmentChildNum (NULL, nSourceSeg, m_lightRenderDepth, visited);	//mark all children that are at most lightRenderDepth segments away
visited [nSourceSeg] = m_lightRenderDepth;
#else
INT32 i;
for (i = SegCount (); i; )
	Segments (--i)->m_info.nIndex = -1;
SetSegmentChildNum (NULL, nSourceSeg, m_lightRenderDepth);
segP->m_info.nIndex = m_lightRenderDepth;
#endif

CColor *plc = LightColor (nSourceSeg, nSourceSide);
if (!plc->index) {
	plc->index = 255;
	plc->color.r =
	plc->color.g =
	plc->color.b = 1.0;
	}
if (UseTexColors () && bCopyTexLights) {
	CColor	*psc = LightColor (nSourceSeg, nSourceSide, false);
	*psc = *plc;
	}
bool bWall = false; //FindWall (nSourceSeg, nSourceSide) != NULL;
// loop on child Segments ()
INT32 nChildSeg;
INT32 nSegCount = SegCount ();
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
		CFixVector source_corner[4];
		INT32 j;
		for (j = 0; j < 4; j++) {
			INT32 nVertex = sideVertTable [nSourceSide][j];
			INT32 h = segP->m_info.verts [nVertex];
			source_corner[j] = *Vertices (h);
			}
		// loop on child sides
		INT32 nChildSide;
		for (nChildSide = 0; nChildSide < 6; nChildSide++) {
			// if side has a child..
			if (!(bAll || SideIsMarked (nChildSeg, nChildSide)))
				continue;
			if (childSegP->m_info.children [nChildSide] >= 0) {
				UINT16 nWall = childSegP->m_sides [nChildSide].m_info.nWall;
				// .. but there is no wall ..
				if (nWall >= GameInfo ().walls.count)
					continue;
					// .. or its not a door ..
				if (Walls (nWall)->m_info.type == WALL_OPEN)
					continue;
				}

	//		CBRK (psc->index > 0);
			// if the child side is the same as the source side, then set light and continue
			if (nChildSide == nSourceSide && nChildSeg == nSourceSeg) {
				CUVL*		uvlP = childSegP->m_sides [nChildSide].m_info.uvls;
				UINT32	vBr, lBr;

				theApp.SetModified (TRUE);
				for (INT32 j = 0; j < 4; j++, uvlP++) {
					CColor *pvc = VertexColors (childSegP->m_info.verts [sideVertTable [nChildSide][j]]);
					vBr = (UINT16) uvlP->l;
					lBr = (UINT32) (brightness * fLightScale);
					BlendColors (plc, pvc, lBr, vBr);
					vBr += lBr;
					vBr = min (0x8000, vBr);
					uvlP->l = (UINT16) vBr;
					}
				continue;
				}

			// calculate vector between center of source segment and center of child
	//		CBRK (nChildSeg == 1 && nChildSide == 2);
			if (CalcSideLights (nChildSeg, nChildSide, source_center, source_corner, A, effect, fLightScale, bWall)) {
					UINT32	vBr, lBr;	//vertex brightness, light brightness
					CUVL		*uvlP = childSegP->m_sides [nChildSide].m_info.uvls;

				theApp.SetModified (TRUE);
				for (INT32 j = 0; j < 4; j++, uvlP++) {
					CColor *pvc = VertexColors (childSegP->m_info.verts [sideVertTable [nChildSide][j]]);
					vBr = (UINT16) uvlP->l;
					lBr = (UINT16) (brightness * effect [j] / 32);
					BlendColors (plc, pvc, lBr, vBr);
					vBr += lBr;
					vBr = min (0x8000, vBr);
					uvlP->l = (UINT16) vBr;
					}
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
// A little fix to take care of too many delta light values leading
// to some flickering lights not being visible in Descent:
// For each recursion step, the delta light calculation will be executed
// for all children of sides with flickering lights with exactly that
// distance (recursionDepth) to their parent. In case of too many delta 
// light values for the full lighting depth at least the lighting of the
// Segments () close to flickering lights will be computed properly.
//------------------------------------------------------------------------

void CMine::CalcDeltaLightData(double fLightScale, INT32 force) 
{
theApp.SetModified (TRUE);
INT32 recursion_depth;
for (recursion_depth = m_deltaLightRenderDepth; recursion_depth; recursion_depth--)
	if (CalcDeltaLights (fLightScale, force, recursion_depth))
		break;
}



bool CMine::IsLava (INT32 nBaseTex)
{
  HINSTANCE hInst = AfxGetApp()->m_hInstance;
  char		name [20];

LoadString(hInst,texture_resource + nBaseTex, name, sizeof (name));
return (strstr((char*)name,"lava") != NULL);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

INT32 CMine::FindDeltaLight (INT16 nSegment, INT16 nSide, INT16 *pi)
{
	INT32	i = pi ? *pi : 0;
	INT32	j	= (INT32)GameInfo ().lightDeltaIndices.count++;
	CLightDeltaIndex	*dliP = LightDeltaIndex (0);

if ((LevelVersion () >= 15) && (GameInfo ().fileinfo.version >= 34)) {
	for (; i < j; i++, dliP++)
		if ((dliP->m_nSegment == nSegment) && (dliP->m_nSide = (UINT8) nSide))
			return i;
	}
else {
	for (; i < j; i++, dliP++)
		if ((dliP->m_nSegment == nSegment) && (dliP->m_nSide = (UINT8) nSide))
			return i;
	}
return -1;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

bool CMine::CalcDeltaLights (double fLightScale, INT32 force, INT32 recursion_depth) 
{
	// initialize totals
	INT32		nSegCount = SegCount ();
	INT32		nSourceSeg;
	INT32		nErrors = 0;
	double	effect[4];

GameInfo ().lightDeltaValues.count = 0;
GameInfo ().lightDeltaIndices.count = 0;
bool bWall, bD2XLights = (LevelVersion () >= 15) && (GameInfo ().fileinfo.version >= 34);

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
		for (INT32 nSourceSide = 0; nSourceSide < 6; nSourceSide++) {
			INT16 nTexture = srcSegP->m_sides [nSourceSide].m_info.nBaseTex & 0x3fff;
			INT16 tmapnum2 = srcSegP->m_sides [nSourceSide].m_info.nOvlTex & 0x3fff;
			INT16 nTrigger;
			bool bl1 = (bool) (IsLight (nTexture) != -1);
			bool bl2 = (bool) (IsLight (tmapnum2) != -1);
			if (!(bl1 || bl2))
				continue;	// no lights on this side
			bool bCalcDeltas = false;
			// if the current side is a wall and has a light and is the target of a trigger
			// than can make the wall appear/disappear, calculate delta lights for it
			if ((bWall = (FindWall (nSourceSeg, nSourceSide) != NULL)) &&
				 ((nTrigger = FindTriggerTarget (0, nSourceSeg, nSourceSide)) >= 0)) {
				INT8 trigtype = Triggers (nTrigger)->m_info.type;
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
				INT32 nTrigger = FindTriggerTarget (0, nSourceSeg, nSourceSide);
				if ((nTrigger >= 0) && (Triggers (nTrigger)->m_info.type >= TT_LIGHT_OFF))
					bCalcDeltas = true;
				}
			if (!bCalcDeltas)
				continue;

			INT16 srcwall = srcSegP->m_sides [nSourceSide].m_info.nWall;
			if ((srcSegP->m_info.children [nSourceSide] != -1) &&
				 ((srcwall >= GameInfo ().walls.count) || (Walls (srcwall)->m_info.type == WALL_OPEN)))
				continue;

			if (GameInfo ().lightDeltaIndices.count >= MAX_LIGHT_DELTA_INDICES) {
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

			CFixVector	A, source_center;
			INT32		lightDeltaIndexCount;

			// get index number and increment total number of lightDeltaIndices
	//#pragma omp critical
			{
			lightDeltaIndexCount = INT32 (GameInfo ().lightDeltaIndices.count++);
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
			dliP->m_info.index = (INT16)GameInfo ().lightDeltaValues.count;

			// find orthogonal angle of source segment
			A = -CalcSideNormal(nSourceSeg,nSourceSide);

			// calculate the center of the source segment
			source_center = CalcSideCenter (nSourceSeg, nSourceSide);

			// mark those Segments () within N children of current cube
			//(note: this is done once per light instead of once per segment
			//       even though some Segments () have multiple lights.
			//       This actually reduces the number of calls since most
			//       Segments () do not have lights)
	#if 1
			INT16* visited = new INT16 [nSegCount];
			memset (visited, 0xff, nSegCount * sizeof (*visited));
			SetSegmentChildNum (srcSegP, nSourceSeg, recursion_depth, visited);
			visited [nSourceSeg] = recursion_depth;
	#else
			INT32 h;
			for (h = 0; h < SegCount (); h++)
				Segments (h)->m_info.nIndex = -1;
			SetSegmentChildNum (srcSegP, nSourceSeg, recursion_depth);
			srcSegP->m_info.nIndex = recursion_depth;
	#endif

			// setup source corner vertex for length calculation later
			CFixVector source_corner[4];
			for (INT32 j = 0; j < 4; j++) {
				UINT8 nVertex = sideVertTable [nSourceSide][j];
				INT32 h = srcSegP->m_info.verts [nVertex];
				source_corner[j] = *Vertices (h);
				}

			// loop on child Segments ()
			INT32 nChildSeg;
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
				for (INT32 nChildSide = 0; nChildSide < 6; nChildSide++) {
					// if texture has a child..
					if (childSegP->m_info.children[nChildSide] >= 0) {
						UINT16 nWall = childSegP->m_sides[nChildSide].m_info.nWall;
						// .. if there is no wall ..
						if (nWall >= GameInfo ().walls.count)
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
						if ((GameInfo ().lightDeltaValues.count >= MAX_LIGHT_DELTA_VALUES) || (dliP->m_info.count == (bD2XLights ? 8191 : 255))) {
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
						dl = LightDeltaValues (GameInfo ().lightDeltaValues.count++);
						}
						dl->m_nSegment = nChildSeg;
						dl->m_nSide = nChildSide;
						dl->m_info.vertLight [0] =
						dl->m_info.vertLight [1] =
						dl->m_info.vertLight [2] =
						dl->m_info.vertLight [3] = (UINT8) min (32, 32 * fLightScale);
						dliP->m_info.count++;
						continue;
						}

					// calculate vector between center of source segment and center of child
						if (CalcSideLights (nChildSeg, nChildSide, source_center, source_corner, A, effect, fLightScale, bWall)) {
							theApp.SetModified (TRUE);
							if ((GameInfo ().lightDeltaValues.count >= MAX_LIGHT_DELTA_VALUES) || (bD2XLights ? dliP->m_info.count == 8191 : dliP->m_info.count == 255)) {
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
							dl = LightDeltaValues (GameInfo ().lightDeltaValues.count++);
							}
							dl->m_nSegment = nChildSeg;
							dl->m_nSide = nChildSide;
							INT32 iCorner;
							for (iCorner = 0; iCorner < 4; iCorner++)
								dl->m_info.vertLight [iCorner] = (UINT8) min(32, effect [iCorner]);
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
	INT16	prevSeg = pSegment->prevSeg;
	INT16	nextSeg = pSegment->nextSeg;
	INT16	thisSeg = pSegment - Segments ();

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
	INT16	prevSeg = pRoot->prevSeg;
	INT16	nextSeg = pRoot->nextSeg;
	INT16 thisSeg = pSegment - Segments ();
	INT16 rootSeg = pRoot - Segments ();

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

void CMine::SetSegmentChildNum (CSegment *pRoot, INT16 nSegment, INT16 recursion_level) 
{
	INT16			nSide, child, nImprove = 0;
	UINT16		nWall;
	CSegment	*segP = Segments (0) + nSegment;
	CSegment	*prevSeg = NULL;
	bool			bMarkChildren = false;

// mark each child if child number is lower
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	// Skip if this is a door
	nWall = segP->m_sides [nSide].m_info.nWall;
	// .. if there is a wall and its a door
	if ((nWall < GameInfo ().walls.count) && (Walls (nWall)->m_info.type == WALL_DOOR))
		continue;
	// mark segment if it has a child
	child = segP->m_info.children [nSide];
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
	if ((nWall < GameInfo ().walls.count) && (Walls (nWall)->m_info.type == WALL_DOOR))
		continue;
	// check child
	child = segP->m_info.children [nSide];
	if ((child > -1) && (child < SegCount ()))
		SetSegmentChildNum (pRoot, child, recursion_level - 1);
	}
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void CMine::SetSegmentChildNum (CSegment *pRoot, INT16 nSegment, INT16 recursion_level, INT16* visited) 
{
	INT16			nSide, child, nImprove = 0;
	UINT16		nWall;
	CSegment	*segP = Segments (0) + nSegment;
	CSegment	*prevSeg = NULL;
	bool			bMarkChildren = false;

// mark each child if child number is lower
for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
	// Skip if this is a door
	nWall = segP->m_sides [nSide].m_info.nWall;
	// .. if there is a wall and its a door
	if ((nWall < GameInfo ().walls.count) && (Walls (nWall)->m_info.type == WALL_DOOR))
		continue;
	// mark segment if it has a child
	child = segP->m_info.children [nSide];
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
	if ((nWall < GameInfo ().walls.count) && (Walls (nWall)->m_info.type == WALL_DOOR))
		continue;
	// check child
	child = segP->m_info.children [nSide];
	if ((child > -1) && (child < SegCount ()))
		SetSegmentChildNum (pRoot, child, recursion_level - 1, visited);
	}
}

//--------------------------------------------------------------------------

bool CMine::CalcSideLights (INT32 nSegment, INT32 nSide, CFixVector& source_center, 
									 CFixVector* source_corner, CFixVector& vertex, double *effect,
									 double fLightScale, bool bIgnoreAngle)
{
	CSegment *segP = Segments (nSegment);
// calculate vector between center of source segment and center of child
CDoubleVector center = CalcSideCenter (nSegment, nSide);
CDoubleVector A = vertex;
CDoubleVector B = center - source_center;

// calculate angle between vectors (use dot product equation)
if (!bIgnoreAngle) {
	double ratio, angle;
	double A_dot_B = A ^ B;
	double mag_A = A.Mag ();
	double mag_B = B.Mag ();
	if (mag_A == 0.0 || mag_B == 0.0)
		angle = (200.0 * M_PI)/180.0; // force a failure
	else {
		ratio = A_dot_B/(mag_A * mag_B);
		ratio = ((double)((INT32)(ratio*1000.0))) / 1000.0;
		if (ratio < -1.0 || ratio > (double)1.0)
			angle = (199.0 * M_PI)/180.0;  // force a failure
		else
			angle = acos (ratio);
		}
	// if angle is less than 110 degrees
	// then we found a match
	if (angle >= (180.0 * M_PI)/180.0)
		return false;
	}
INT32 i, j;
for (j = 0; j < 4; j++) {
	CFixVector corner;
	INT32 nVertex = sideVertTable[nSide][j];
	INT32 h = segP->m_info.verts[nVertex];
	corner = *Vertices (h);
	double length = 20.0 * m_lightRenderDepth;
	for (i = 0; i < 4; i++)
		length = min (length, CalcLength (source_corner + i, &corner) / F1_0);
	length /= 10.0 * m_lightRenderDepth / 6.0; // divide by 1/2 a cubes length so opposite side
	// light is recuded by 1/4
	effect [j] = 32;
	if (length > 1.0)//if (length < 20.0 * m_lightRenderDepth) // (roughly 4 standard cube lengths)
		effect [j] /= (length * length);
	effect [j] *= fLightScale;
//	else
//		effect [j] = 0;
	}
// if any of the effects are > 0, then increment the
// light for that side
return (effect [0] != 0 || effect [1] != 0 || effect [2] != 0 || effect [3] != 0);
}

//--------------------------------------------------------------------------

CColor *CMine::LightColor (INT32 i, INT32 j, bool bUseTexColors) 
{ 
if (bUseTexColors && UseTexColors ()) {
	CWall *pWall = SideWall (i, j);
	//if (!pWall || (pWall->m_info.type != WALL_TRANSPARENT)) 
		{	//always use a side color for transp. walls
		CColor *pc;
		INT16 t = Segments (i)->m_sides [j].m_info.nOvlTex & 0x3fff;
		if ((t > 0) && (pc = GetTexColor (t)))
			return pc;
		if (pc = GetTexColor (Segments (i)->m_sides [j].m_info.nBaseTex, pWall && (pWall->m_info.type == WALL_TRANSPARENT)))
			return pc;
		}
	}	
return &MineData ().lightColors [i][j]; 
}

//--------------------------------------------------------------------------
//eof light.cpp