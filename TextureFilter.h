// Copyright (C) 1997 Bryan Aamot

#ifndef __texturefilter_h
#define __texturefilter_h

#include "types.h"
#include "segment.h"
#include "object.h"

//------------------------------------------------------------------------

#define	TEX_GRAY_ROCK			1
#define	TEX_BROWN_ROCK			(1L << 1)
#define	TEX_RED_ROCK			(1L << 2)
#define	TEX_GREEN_ROCK			(1L << 3)
#define	TEX_YELLOW_ROCK		(1L << 4)
#define	TEX_BLUE_ROCK			(1L << 5)
#define	TEX_ICE					(1L << 6)
#define	TEX_STONES				(1L << 7)
#define	TEX_GRASS				(1L << 8)
#define	TEX_SAND					(1L << 9)
#define	TEX_LAVA					(1L << 10)
#define	TEX_WATER				(1L << 11)
#define	TEX_STEEL				(1L << 12)
#define	TEX_CONCRETE			(1L << 13)
#define	TEX_BRICK				(1L << 14)
#define	TEX_TARMAC				(1L << 15)
#define	TEX_WALL					(1L << 16)
#define	TEX_FLOOR				(1L << 17)
#define	TEX_CEILING				(1L << 18)
#define	TEX_GRATE				(1L << 19)
#define	TEX_FAN					(1L << 20)
#define	TEX_LIGHT				(1L << 21)
#define	TEX_ENERGY				(1L << 22)
#define	TEX_FORCEFIELD			(1L << 23)
#define	TEX_SIGN					(1L << 24)
#define	TEX_SWITCH				(1L << 25)
#define	TEX_TECH					(1L << 26)
#define	TEX_DOOR					(1L << 27)
#define	TEX_LABEL				(1L << 28)
#define	TEX_MONITOR				(1L << 29)
#define	TEX_STRIPES				(1L << 30)
#define	TEX_MOVE					(1L << 31)

#define  TEX_ROCK					(TEX_GRAY_ROCK | TEX_BROWN_ROCK | TEX_RED_ROCK | TEX_YELLOW_ROCK | TEX_GREEN_ROCK | TEX_BLUE_ROCK)
#define  TEX_NATURE				(TEX_ICE | TEX_STONES | TEX_GRASS | TEX_SAND | TEX_LAVA | TEX_WATER)
#define	TEX_BUILDING			(TEX_STEEL | TEX_CONCRETE | TEX_BRICK | TEX_TARMAC | TEX_WALL | TEX_FLOOR | TEX_CEILING)
#define	TEX_OTHER				(TEX_FAN | TEX_GRATE | TEX_DOOR)
#define	TEX_TECHMAT				(TEX_SWITCH | TEX_TECH | TEX_ENERGY | TEX_FORCEFIELD | TEX_LIGHT)
#define	TEX_SIGNS				(TEX_SIGN | TEX_LABEL | TEX_MONITOR | TEX_STRIPES)

#define TEXFILTER_SIZE_D1	154
#define TEXFILTER_SIZE_D2	174

extern tTexFilter texFiltersD2 [TEXFILTER_SIZE_D2];
extern tTexFilter texFiltersD1 [TEXFILTER_SIZE_D1];

#define TEX_FILTER_SIZE	(DLE.IsD1File () ? TEXFILTER_SIZE_D1 : TEXFILTER_SIZE_D2)
#define TEXTURE_FILTERS (DLE.IsD1File () ? texFiltersD1 : texFiltersD2)

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tTexRange {
	short	nMin, nMax;
} tTexRange;

typedef struct tTexFilterInfo {
	tTexRange	m_range;
	uint			m_nFilter1;
	uint			m_nFilter2;
} tTexFilterInfo;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CTextureFilter {
public:
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif //__texturefilter_h4