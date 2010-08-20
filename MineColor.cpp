// Copyright (c) 1998 Bryan Aamot, Brainware

#include "stdafx.h"
#include "dle-xp-res.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "io.h"
#include "textures.h"
#include "palette.h"
#include "dle-xp.h"
#include "robot.h"
#include "hogmanager.h"
#include "light.h"

#ifdef ALLOCATE_POLYMODELS
#undef ALLOCATE_POLYMODELS
#endif
#define ALLOCATE_POLYMODELS 0

#define ENABLE_TEXT_DUMP 0

// ------------------------------------------------------------------------

void CColor::Read (FILE *fp, int nLevelVersion)
{
index = read_INT8 (fp);
if (nLevelVersion < nNewVersion) {
	color.r = read_DOUBLE (fp);
	color.g = read_DOUBLE (fp);
	color.b = read_DOUBLE (fp);
	}
else {
	color.r = double (read_INT32 (fp)) / double (0x7fffffff);
	color.g = double (read_INT32 (fp)) / double (0x7fffffff);
	color.b = double (read_INT32 (fp)) / double (0x7fffffff);
	}
return 1;
}

// ------------------------------------------------------------------------

void CColor::Write (FILE *fp)
{
write_INT8 (index, fp);
write_INT32 (INT32 (color.r * 0x7fffffff + 0.5), fp);
write_INT32 (INT32 (color.g * 0x7fffffff + 0.5), fp);
write_INT32 (INT32 (color.b * 0x7fffffff + 0.5), fp);
}

// ------------------------------------------------------------------------

void CMine::LoadColors (CColor *pc, INT32 nColors, INT32 nFirstVersion, INT32 nNewVersion, FILE *fp)
{
if (LevelVersion () > nFirstVersion) { 
	for (; nColors; nColors--, pc++)
		pc->Read (fp, LevelVersion ());
	}
}

// ------------------------------------------------------------------------

void CMine::SaveColors (CColor *pc, INT32 nColors, FILE *fp)
{
for (; nColors; nColors--, pc++)
	pc->Write (fp);
}

//--------------------------------------------------------------------------

INT32 CMine::ReadColorMap (FILE *fColorMap)
{
LoadColors (TexColors (), MAX_D2_TEXTURES, 0, 0, fColorMap);
return 0;
}

//--------------------------------------------------------------------------

INT32 CMine::WriteColorMap (FILE *fColorMap)
{
SaveColors (TexColors (), MAX_D2_TEXTURES, fColorMap);
return 0;
}

//--------------------------------------------------------------------------
//eof mine.cpp