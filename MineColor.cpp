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

INT32 CColor::Read (FILE* fp, INT32 version, bool bNewFormat)
{
m_info.index = read_INT8 (fp);
if (bNewFormat) {
	m_info.color.r = double (read_INT32 (fp)) / double (0x7fffffff);
	m_info.color.g = double (read_INT32 (fp)) / double (0x7fffffff);
	m_info.color.b = double (read_INT32 (fp)) / double (0x7fffffff);
	}
else {
	m_info.color.r = read_DOUBLE (fp);
	m_info.color.g = read_DOUBLE (fp);
	m_info.color.b = read_DOUBLE (fp);
	}
return 1;
}

// ------------------------------------------------------------------------

void CColor::Write (FILE* fp, INT32 version, bool bFlag) 
{
write_INT8 (m_info.index, fp);
write_INT32 (INT32 (m_info.color.r * 0x7fffffff + 0.5), fp);
write_INT32 (INT32 (m_info.color.g * 0x7fffffff + 0.5), fp);
write_INT32 (INT32 (m_info.color.b * 0x7fffffff + 0.5), fp);
}

// ------------------------------------------------------------------------

void CMine::LoadColors (CColor *pc, INT32 nColors, INT32 nFirstVersion, INT32 nNewVersion, FILE *fp)
{
	bool bNewFormat = LevelVersion () >= nNewVersion;

if (LevelVersion () > nFirstVersion) { 
	for (; nColors; nColors--, pc++)
		pc->Read (fp, 0, bNewFormat);
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