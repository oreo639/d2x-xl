#include "Mine.h"

CLightManager lightManager;

//------------------------------------------------------------------------------

tTextureLight textureLightD1[NUM_LIGHTS_D1] = {
	{250, 0x00b333L}, {251, 0x008000L}, {252, 0x008000L}, {253, 0x008000L},
	{264, 0x01547aL}, {265, 0x014666L}, {268, 0x014666L}, {278, 0x014cccL},
	{279, 0x014cccL}, {280, 0x011999L}, {281, 0x014666L}, {282, 0x011999L},
	{283, 0x0107aeL}, {284, 0x0107aeL}, {285, 0x011999L}, {286, 0x014666L},
	{287, 0x014666L}, {288, 0x014666L}, {289, 0x014666L}, {292, 0x010cccL},
	{293, 0x010000L}, {294, 0x013333L}, {330, 0x010000L}, {333, 0x010000L}, 
	{341, 0x010000L}, {343, 0x010000L}, {345, 0x010000L}, {347, 0x010000L}, 
	{349, 0x010000L}, {351, 0x010000L}, {352, 0x010000L}, {354, 0x010000L}, 
	{355, 0x010000L}, {356, 0x020000L}, {357, 0x020000L}, {358, 0x020000L}, 
	{359, 0x020000L}, {360, 0x020000L}, {361, 0x020000L}, {362, 0x020000L}, 
	{363, 0x020000L}, {364, 0x020000L}, {365, 0x020000L}, {366, 0x020000L}, 
	{367, 0x020000L}, {368, 0x020000L}, {369, 0x020000L}, {370, 0x020000L}
};

tTextureLight textureLightD2[NUM_LIGHTS_D2] = {
	{235, 0x012666L}, {236, 0x00b5c2L}, {237, 0x00b5c2L}, {243, 0x00b5c2L},
	{244, 0x00b5c2L}, {275, 0x01547aL}, {276, 0x014666L}, {278, 0x014666L},
	{288, 0x014cccL}, {289, 0x014cccL}, {290, 0x011999L}, {291, 0x014666L},
	{293, 0x011999L}, {295, 0x0107aeL}, {296, 0x011999L}, {298, 0x014666L},
	{300, 0x014666L}, {301, 0x014666L}, {302, 0x014666L}, {305, 0x010cccL},
	{306, 0x010000L}, {307, 0x013333L}, {340, 0x00b333L}, {341, 0x00b333L},
	{343, 0x004cccL}, {344, 0x003333L}, {345, 0x00b333L}, {346, 0x004cccL},
	{348, 0x003333L}, {349, 0x003333L}, {353, 0x011333L}, {356, 0x00028fL},
	{357, 0x00028fL}, {358, 0x00028fL}, {359, 0x00028fL}, {364, 0x010000L},
	{366, 0x010000L}, {368, 0x010000L}, {370, 0x010000L}, {372, 0x010000L},
	{374, 0x010000L}, {375, 0x010000L}, {377, 0x010000L}, {378, 0x010000L},
	{380, 0x010000L}, {382, 0x010000L}, {383, 0x020000L}, {384, 0x020000L},
	{385, 0x020000L}, {386, 0x020000L}, {387, 0x020000L}, {388, 0x020000L},
	{389, 0x020000L}, {390, 0x020000L}, {391, 0x020000L}, {392, 0x020000L},
	{393, 0x020000L}, {394, 0x020000L}, {395, 0x020000L}, {396, 0x020000L},
	{397, 0x020000L}, {398, 0x020000L}, {404, 0x010000L}, {405, 0x010000L},
	{406, 0x010000L}, {407, 0x010000L}, {408, 0x010000L}, {409, 0x020000L},
	{410, 0x008000L}, {411, 0x008000L}, {412, 0x008000L}, {419, 0x020000L},
	{420, 0x020000L}, {423, 0x010000L}, {424, 0x010000L}, {425, 0x020000L},
	{426, 0x020000L}, {427, 0x008000L}, {428, 0x008000L}, {429, 0x008000L},
	{430, 0x020000L}, {431, 0x020000L}, {432, 0x00e000L}, {433, 0x020000L},
	{434, 0x020000L}
};

short blastableLightsD2 [] = {
	276, 278, 360, 361, 364, 366, 368,
	370, 372, 374, 375, 377, 380, 382, 
	420, 432, 431,  -1
	};

// ------------------------------------------------------------------------

void CColor::Read (CFileManager& fp, int version, bool bNewFormat)
{
m_info.index = fp.ReadSByte ();
if (bNewFormat) {
	m_info.color.r = double (fp.ReadInt32 ()) / double (0x7fffffff);
	m_info.color.g = double (fp.ReadInt32 ()) / double (0x7fffffff);
	m_info.color.b = double (fp.ReadInt32 ()) / double (0x7fffffff);
	}
else {
	m_info.color.r = fp.ReadDouble ();
	m_info.color.g = fp.ReadDouble ();
	m_info.color.b = fp.ReadDouble ();
	}
}

// ------------------------------------------------------------------------

void CColor::Write (CFileManager& fp, int version, bool bFlag) 
{
fp.Write (m_info.index);
fp.WriteInt32 ((int) (m_info.color.r * 0x7fffffff + 0.5));
fp.WriteInt32 ((int) (m_info.color.g * 0x7fffffff + 0.5));
fp.WriteInt32 ((int) (m_info.color.b * 0x7fffffff + 0.5));
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CVariableLight::Read (CFileManager& fp) 
{
CSideKey::Read (fp);
m_info.mask = fp.ReadInt32 ();
m_info.timer = fp.ReadInt32 ();
m_info.delay = fp.ReadInt32 ();
}

//------------------------------------------------------------------------------

void CVariableLight::Write (CFileManager& fp) 
{
CSideKey::Write (fp);
fp.Write (m_info.mask);
fp.Write (m_info.timer);
fp.Write (m_info.delay);
}

//------------------------------------------------------------------------------

void CVariableLight::Clear (void) 
{
memset (&m_info, 0, sizeof (m_info));
CSideKey::Clear ();
}

//------------------------------------------------------------------------------

void CVariableLight::Setup (CSideKey key, short time, short mask)
{
m_nSegment = key.m_nSegment;
m_nSide = key.m_nSide;
m_info.delay = m_info.timer = time;
m_info.mask = mask;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

short CLightManager::VariableLight (CSideKey key) 
{
current.Get (key);
CVariableLight* flP = VariableLight (0);
int i;
for (i = Count (); i; i--, flP++)
	if (*flP == key)
		break;
if (i > 0)
	return Count () - i;
return -1;
}

//------------------------------------------------------------------------------

bool CLightManager::IsVariableLight (CSideKey key)
{
return VariableLight (key) >= 0;
}

//------------------------------------------------------------------------------

short CLightManager::AddVariableLight (CSideKey key, uint mask,int time) 
{
current.Get (key);
if (VariableLight (key) != -1) {
	if (!bExpertMode)
		ErrorMsg ("There is already a variable light on this side");
	return -1;
	}
// we are adding a new variable light
if (Count () >= MAX_VARIABLE_LIGHTS) {
	if (!bExpertMode) {
		sprintf_s (message, sizeof (message),
					  "Maximum number of variable lights (%d) have already been added",
					  MAX_VARIABLE_LIGHTS);
		ErrorMsg (message);
		}
	return -1;
	}
short nBaseTex, nOvlTex;
current.Side ()->GetTextures (nBaseTex, nOvlTex);

if ((IsLight (nBaseTex) == -1) && (IsLight (nOvlTex) == -1)) {
	if (!bExpertMode)
		ErrorMsg ("Blinking lights can only be added to a side\n"
					 "that has a light emitting texture.\n"
					 "Hint: You can use the texture tool's brightness control\n"
					 "to make any texture emit light.");
	return -1;
	}
undoManager.SetModified (true);
VariableLight (m_nCount)->Setup (key, time, mask);
return ++Count ();
}

//------------------------------------------------------------------------------

void CLightManager::DeleteVariableLight (short index) 
{
if (index > -1) {
	undoManager.SetModified (true);
	if (index < --Count ())
		memcpy (VariableLight (index), VariableLight (m_nCount), sizeof (CVariableLight));
	}
}

//------------------------------------------------------------------------------

bool CLightManager::DeleteVariableLight (CSideKey key) 
{
current.Get (key);
short index = VariableLight (key);
if (index == -1)
	return false;
DeleteVariableLight (index);
return true;
}

//------------------------------------------------------------------------------

int CLightManager::IsLight (int nBaseTex) 
{
return (m_lightMap [nBaseTex & 0x1fff] > 0) ? 0 : -1;
}

//------------------------------------------------------------------------------

int CLightManager::IsExplodingLight (int nBaseTex) 
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

//------------------------------------------------------------------------------

bool CLightManager::IsBlastableLight (int nBaseTex) 
{
nBaseTex &= 0x3fff;
if (IsExplodingLight (nBaseTex))
	return true;
if (theMine->IsD1File ())
	return false;
for (short *p = blastableLightsD2; *p >= 0; p++)
	if (*p == nBaseTex)
		return true;
return false;
}

//------------------------------------------------------------------------------

CColor* CLightManager::LightColor (CSideKey key, bool bUseTexColors) 
{ 
current.Get (key);
if (bUseTexColors && UseTexColors ()) {
	short nBaseTex, nOvlTex;
	segmentManager.Textures (key, nBaseTex, nOvlTex);
	CColor *pc;
	if ((nOvlTex > 0) && (pc = &m_texColors [nOvlTex]))
		return pc;
	CWall* wallP = segmentManager.Wall (key);
	if (pc = GetTexColor (nBaseTex, (wallP != null) && (wallP->m_info.type == WALL_TRANSPARENT)))
		return pc;
	}	
return LightColor (key.m_nSegment, key.m_nSide); 
}

//------------------------------------------------------------------------------

int CLightManager::FindLight (int nTexture, tTextureLight* texLightP, int nLights)
{
	int	l = 0;
	int	r = nLights - 1;
	int	m, t;

while (l <= r) {
	m = (l + r) / 2;
	t = texLightP [m].nBaseTex;
	if (nTexture > t)
		l = m + 1;
	else if (nTexture < t)
		r = m - 1;
	else
		return m;
	}
return -1;
}

//------------------------------------------------------------------------------

void CLightManager::CreateLightMap (void)
{
LoadDefaults ();
}

//------------------------------------------------------------------------------

void CLightManager::ReadLightMap (CFileManager& fp, uint nSize)
{
fp.Read (m_lightMap, nSize, 1);
}

//------------------------------------------------------------------------------

void CLightManager::WriteLightMap (CFileManager& fp)
{
fp.Write (m_lightMap, sizeof (m_lightMap), 1);
}

// ------------------------------------------------------------------------

void CLightManager::LoadColors (CColor *pc, int nColors, int nFirstVersion, int nNewVersion, CFileManager& fp)
{
	bool bNewFormat = theMine->LevelVersion () >= nNewVersion;

if (theMine->LevelVersion () > nFirstVersion) { 
	for (; nColors; nColors--, pc++)
		pc->Read (fp, 0, bNewFormat);
	}
}

// ------------------------------------------------------------------------

void CLightManager::SaveColors (CColor *pc, int nColors, CFileManager& fp)
{
for (; nColors; nColors--, pc++)
	pc->Write (fp);
}

//------------------------------------------------------------------------------

void CLightManager::ReadColorMap (CFileManager& fp)
{
LoadColors (&m_texColors [0], MAX_TEXTURES_D2, 0, 0, fp);
}

//------------------------------------------------------------------------------

void CLightManager::WriteColorMap (CFileManager& fp)
{
SaveColors (&m_texColors [0], MAX_TEXTURES_D2, fp);
}

// ------------------------------------------------------------------------

bool CLightManager::HasCustomLightMap (void)
{
CResource res;
byte *dataP;
if (!(dataP = res.Load (theMine->IsD1File () ? IDR_LIGHT_D1 : IDR_LIGHT_D2)))
	return false;
return memcmp (m_lightMap, dataP, sizeof (m_lightMap)) != 0;
}

// ------------------------------------------------------------------------

bool CLightManager::HasCustomLightColors (void)
{
CResource res;
byte *dataP;
if (!(dataP = res.Load (theMine->IsD1File () ? IDR_COLOR_D1 : IDR_COLOR_D2)))
	return false;
return memcmp (&m_texColors [0], dataP, sizeof (m_texColors)) != 0;
}

// ------------------------------------------------------------------------

short CLightManager::LoadDefaults (void)
{
CResource res;
byte *dataP;
if (!(dataP = res.Load (theMine->IsD1File () ? IDR_COLOR_D1 : IDR_COLOR_D2)))
	return false;
int i = res.Size () / (3 * sizeof (int) + sizeof (byte));
#if _DEBUG
if (i > (int)m_texColors.Length ())
	i = (int)m_texColors.Length ();
#else
if (i > sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]))
	i = sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]);
#endif
for (CColor* colorP = &m_texColors [0]; i; i--, colorP++) {
	colorP->m_info.index = *dataP++;
	colorP->m_info.color.r = (double) *((int *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (int);
	colorP->m_info.color.g = (double) *((int *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (int);
	colorP->m_info.color.b = (double) *((int *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (int);
	}

if (!(dataP = res.Load (theMine->IsD1File () ? IDR_LIGHT_D1 : IDR_LIGHT_D2)))
	return false;
memcpy (m_lightMap, dataP, min (res.Size (), sizeof (m_lightMap)));
return 1;
}

// -----------------------------------------------------------------------------

void CLightManager::SetLight (double fLight, bool bAll, bool bDynSegLights)
{
	long nLight = (int) (fLight * 65536); //24.0 * 327.68);

undoManager.SetModified (true);

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
			segP->m_info.staticLight = (int) (c ? fLight * ((double) l / (double) c) * 2 : nLight);
			}
		}
	}
}

// ------------------------------------------------------------------------

void CLightManager::SortDeltaIndex (int left, int right)
{
	int	l = left,
			r = right,
			m = (left + right) / 2;
	short	mSeg = LightDeltaIndex (m)->m_nSegment, 
			mSide = LightDeltaIndex (m)->m_nSide;
	CSideKey mKey = CSideKey (mSeg, mSide);
	CLightDeltaIndex	*pl, *pr;

do {
	pl = LightDeltaIndex (l);
	//while ((pl->m_nSegment < mSeg) || ((pl->m_nSegment == mSeg) && (pl->nSide < mSide))) {
	while (*pl < mKey) {
		pl++;
		l++;
		}
	pr = LightDeltaIndex (r);
	//while ((pr->m_info.nSegment > mSeg) || ((pr->m_info.nSegment == mSeg) && (pr->nSide > mSide))) {
	while (*pr > mKey) {
		pr--;
		r--;
		}
	if (l <= r) {
		if (l < r) {
			CLightDeltaIndex	h = *pl;
			*pl = *pr;
			*pr = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (right > l)
   SortDeltaIndex (l, right);
if (r > left)
   SortDeltaIndex (left, r);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CLightDeltaValue::Read (CFileManager& fp, int version, bool bFlag)
{
m_nSegment = fp.ReadInt16 ();
m_nSide = fp.ReadSByte ();
fp.ReadSByte ();
for (int i = 0; i < 4; i++)
	m_info.vertLight [i] = fp.ReadSByte ();
}

// -----------------------------------------------------------------------------

void CLightDeltaValue::Write (CFileManager& fp, int version, bool bFlag)
{
fp.Write (m_nSegment);
fp.WriteSByte ((sbyte) m_nSide);
fp.WriteByte (0);
for (int i = 0; i < 4; i++)
	fp.Write (m_info.vertLight [i]);
}

// -----------------------------------------------------------------------------

void CLightDeltaIndex::Read (CFileManager& fp, int version, bool bD2X)
{
m_nSegment = fp.ReadInt16 ();
if (bD2X) {
	ushort h = fp.ReadInt16 ();
	m_nSide = h & 7;
	m_info.count = h >> 3;
	}
else {
	m_nSide = fp.ReadSByte ();
	m_info.count = fp.ReadSByte ();
	}
m_info.index = fp.ReadInt16 ();
}

// -----------------------------------------------------------------------------

void CLightDeltaIndex::Write (CFileManager& fp, int version, bool bD2X)
{
fp.Write (m_nSegment);
if (bD2X)
	fp.WriteInt16 ((m_nSide & 7) | (m_info.count << 3));
else {
	fp.WriteSByte ((sbyte) m_nSide);
	fp.WriteSByte ((sbyte) m_info.count);
	}
fp.Write (m_info.index);
}

//------------------------------------------------------------------------------

void CLightManager::ReadColors (CFileManager& fp)
{
if (theMine->LevelVersion () == 9) {
#if 1
	LoadColors (LightColor (0), segmentManager.Count () * 6, 9, 14, fp);
	LoadColors (LightColor (0), segmentManager.Count () * 6, 9, 14, fp);
	LoadColors (VertexColor (0), vertexManager.Count (), 9, 15, fp);
#else
	fp.Read (LightColors (), sizeof (CColor), segmentManager.Count () * 6); //skip obsolete side colors 
	fp.Read (LightColors (), sizeof (CColor), segmentManager.Count () * 6);
	fp.Read (VertexColors (), sizeof (CColor), vertexManager.Count ());
#endif
	}
else if (theMine->LevelVersion () > 9) {
	LoadColors (VertexColor (0), vertexManager.Count (), 9, 15, fp);
	LoadColors (LightColor (0), segmentManager.Count () * 6, 9, 14, fp);
	LoadColors (TexColor (0), MAX_TEXTURES_D2, 10, 16, fp);
	}
}

//------------------------------------------------------------------------------

void CLightManager::WriteColors (CFileManager& fp)
{
SaveColors (VertexColor (0), vertexManager.Count (), fp);
SaveColors (LightColor (0), segmentManager.Count () * 6, fp);
SaveColors (TexColor (0), MAX_TEXTURES_D2, fp);
}

//------------------------------------------------------------------------------

void CLightManager::ReadVariableLights (CFileManager& fp)
{
if (theMine->LevelVersion () > 6) {
	lightManager.Count () = (short) fp.ReadInt32 ();
	for (int i = 0; i < lightManager.Count (); i++) {
		if (i < MAX_VARIABLE_LIGHTS)
			VariableLight (i)->Read (fp);
		else { // skip excess data
			CVariableLight l;
			l.Read (fp);
			}
		}
	}
}

//------------------------------------------------------------------------------

void CLightManager::WriteVariableLights (CFileManager& fp)
{
if (theMine->LevelVersion () > 6) {
	 fp.Write (lightManager.Count ());
	for (int i = 0; i < lightManager.Count (); i++) {
		VariableLight (i)->Write (fp);
		}
	}
}

//------------------------------------------------------------------------------

void CLightManager::ReadLightDeltas (CFileManager& fp, int nFileVersion)
{
if (theMine->IsD2File ()) {

	bool bD2X = (theMine->LevelVersion () >= 15) && (theMine->FileInfo ().version >= 34);
	int i;

	for (i = 0; i < m_deltaIndexInfo.count; i++)
		m_deltaIndex [i].Read (fp, nFileVersion, bD2X);
	for (i = 0; i < m_deltaValueInfo.count; i++)
		m_deltaValues [i].Read (fp, nFileVersion);
	}
}

//------------------------------------------------------------------------------

void CLightManager::WriteLightDeltas (CFileManager& fp, int nFileVersion)
{
if (DeltaIndexCount () > 0) {
	if ((theMine->LevelVersion () >= 15) && (nFileVersion >= 34))
		lightManager.SortDeltaIndex ();

	bool bD2X = (theMine->LevelVersion () >= 15) && (theMine->FileInfo ().version >= 34);
	int i;

	m_deltaIndexInfo.size = 6;
	m_deltaIndexInfo.offset = fp.Tell ();
	for (i = 0; i < m_deltaIndexInfo.count; i++)
		m_deltaIndex [i].Write (fp, nFileVersion, bD2X);

	m_deltaValueInfo.size = 8;
	m_deltaValueInfo.offset = fp.Tell ();
	for (i = 0; i < m_deltaValueInfo.count; i++)
		m_deltaValues [i].Write (fp, nFileVersion);
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
