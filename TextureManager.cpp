
#include "mine.h"
#include "dle-xp.h"

CTextureManager textureManager;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

void CTextureManager::Setup (void)
{
m_extra = null;
m_paletteName [0] = 0;
Create (0);
Create (1);
}

//------------------------------------------------------------------------

void CTextureManager::Create (int nVersion)
{
m_pigFiles [nVersion][0] = 0;
m_names [nVersion] = null;
LoadNames (nVersion);
m_header [nVersion] = CPigHeader (nVersion);
LoadIndex (nVersion);
#if _DEBUG
m_textures [nVersion].Create (MaxTextures (nVersion));
#else
m_textures [nVersion] = new CTexture [MaxTextures (nVersion)];
#endif
}

//------------------------------------------------------------------------

void CTextureManager::Destroy (int nVersion)
{
Release (nVersion, true, false);
#if _DEBUG
m_textures [nVersion].Destroy ();
#else
if (m_textures [nVersion]) {
	delete m_textures [nVersion];
	m_textures [nVersion] = null;
	}
#endif
if (m_names [nVersion]) {
	for (int j = 0, h = MaxTextures (nVersion); j < h; j++)
		if (m_names [nVersion][j])
			delete m_names [nVersion][j];
	delete m_names [nVersion];
	m_names [nVersion] = null;
	}
if (m_index [nVersion]) {
	delete m_index [nVersion];
	m_index [nVersion] = null;
	}
if (m_info [nVersion]) {
	delete m_info [nVersion];
	m_info [nVersion] = null;
	}
}

//------------------------------------------------------------------------

void CTextureManager::Destroy (void)
{
for (int i = 0; i < 2; i++)
	Destroy (i);
}

//------------------------------------------------------------------------

int CTextureManager::Version (void) 
{ 
return DLE.IsD1File () ? 0 : 1; 
}

//------------------------------------------------------------------------

void CTextureManager::Release (int nVersion, bool bDeleteAll, bool bDeleteUnused) 
{
// free any m_textures that have been buffered
#if _DEBUG
CTexture* texP = m_textures [nVersion].Buffer ();
#else
CTexture* texP = m_textures [nVersion];
#endif
if (texP != null) {
	for (int i = 0, h = MaxTextures (nVersion); i < h; i++, texP++) {
		if (bDeleteUnused) {
			if (texP->m_info.bCustom && !texP->m_info.bUsed)
				texP->Release ();
			}
		else {
			if (bDeleteAll || texP->m_info.bCustom)
				texP->Release ();
			}
		}		
	}
}

//------------------------------------------------------------------------

void CTextureManager::Release (bool bDeleteAll, bool bDeleteUnused) 
{
// free any m_textures that have been buffered
for (int i = 0; i < 2; i++) 
	Release (i, bDeleteAll, bDeleteUnused);
for (CExtraTexture* p = m_extra; p != null; ) {
	m_extra = p->m_next;
	delete p;
	}
}

//------------------------------------------------------------------------

CFileManager* CTextureManager::OpenPigFile (int nVersion)
{
	CFileManager* fp = new CFileManager;
	char	filename [256], appFolder [256];

if ((strchr (descentPath [nVersion], ':') == null) && (descentPath [nVersion][0] != '\\')) {
	::GetModuleFileName (0, appFolder, sizeof (appFolder));
	CFileManager::SplitPath (appFolder, filename, null, null);
	strcat_s (filename, sizeof (filename), descentPath [nVersion]);
	}
else
	strcpy_s (filename, sizeof (filename), descentPath [nVersion]);
if (!strstr (filename, ".pig"))
	strcat_s (filename, sizeof (filename), "groupa.pig");
if (fp->Open (filename, "rb")) {
	delete fp;
	m_bAvailable [nVersion] = false;
	return null;
	}
uint nOffset = fp->ReadUInt32 ();
if (nOffset == 0x47495050) // 'PPIG' Descent 2 type
	nOffset = 0;
else if (nOffset < 0x10000)
	nOffset = 0;
fp->Seek (nOffset, SEEK_SET);
return fp;
}

//------------------------------------------------------------------------

int CTextureManager::MaxTextures (int nVersion)
{
return ((nVersion < 0) ? DLE.IsD2File () : nVersion) ? MAX_TEXTURES_D2 : MAX_TEXTURES_D1;
}

//------------------------------------------------------------------------

bool CTextureManager::HasCustomTextures (void) 
{
CTexture* texP = &m_textures [DLE.FileType ()][0];

for (int i = MaxTextures (DLE.FileType ()); i; i--, texP++)
	if (texP->m_info.bCustom)
		return true;
return false;
}

//------------------------------------------------------------------------

int CTextureManager::CountCustomTextures (void) 
{
	int			count = 0;
	CTexture*	texP = &m_textures [DLE.FileType ()][0];

for (int i = MaxTextures (); i; i--, texP++)
	if (texP->m_info.bCustom)
		count++;
return count;
}

//------------------------------------------------------------------------------

bool CTextureManager::Check (int nTexture)
{
if ((nTexture >= 0) && (nTexture < MaxTextures ()))
	return true;
sprintf_s (message, sizeof (message), "Reading texture: Texture #%d out of range.", nTexture);
DEBUGMSG (message);
return false;
}

//------------------------------------------------------------------------------

void CTextureManager::Load (ushort nBaseTex, ushort nOvlTex)
{
//if (Check (nBaseTex)) {
//   m_textures [(int)DLE.FileType ()] [nBaseTex].Load (nBaseTex);
//   if (Check (nOvlTex & 0x1FFF) && ((nOvlTex & 0x1FFF) != 0)) {
//       Check ((ushort) (nOvlTex & 0x1FFF));
//       m_textures [(int)DLE.FileType ()] [(ushort) (nOvlTex & 0x1FFF)].Load ((ushort) (nOvlTex & 0x1FFF));
//		}
//   }
}

//------------------------------------------------------------------------------

void CTextureManager::LoadNames (int nVersion)
{
if (nVersion < 0)
	nVersion = DLE.IsD1File () ? 0 : 1;
	int nResource = (nVersion == 0) ? TEXTURE_STRING_TABLE_D1 : TEXTURE_STRING_TABLE_D2;
	CStringResource res;
	int j = MaxTextures (nVersion);

m_names [nVersion] = new char* [j];
for (int i = 0; i < j; i++) {
	res.Clear ();
	res.Load (nResource + i);
	int l = res.Length () + 1;
	m_names [nVersion][i] = new char [l];
	memcpy_s (m_names [nVersion][i], l, res.Value (), l);
	}
}

//------------------------------------------------------------------------------

int CTextureManager::LoadIndex (int nVersion)
{
	CResource res;	

ushort* indexP = (ushort *) res.Load (nVersion ? IDR_TEXTURE2_DAT : IDR_TEXTURE_DAT);
if (!indexP) {
	DEBUGMSG (" Reading texture: Could not load texture index.");
	return 1;
	}
// first long is number of m_textures
m_nTextures [nVersion] = *((uint*) indexP);
indexP += 2;
m_index [nVersion] = new ushort [m_nTextures [nVersion]];
if (m_index [nVersion] == null) {
	DEBUGMSG (" Reading texture: Could not allocate texture index.");
	return 3;
	}
for (uint i = 0; i < m_nTextures [nVersion]; i++)
	m_index [nVersion][i] = (*indexP++);
return 0;
}

//------------------------------------------------------------------------------

bool CTextureManager::LoadInfo (int nVersion)
{
if (m_info [nVersion] != null) {
	delete m_info [nVersion];
	m_info [nVersion] = null;
	}
CFileManager* fp = OpenPigFile (nVersion);
if (fp == null)
	return false;
m_header [nVersion].Read (*fp);
m_info [nVersion] = new CPigTexture [m_header [nVersion].nTextures];
for (int i = 0; i < m_header [nVersion].nTextures; i++)
	m_info [nVersion][i].Read (fp, nVersion);
m_nOffsets [nVersion] = fp->Tell ();
if (nVersion == 0)
	m_nOffsets [0] += m_header [0].nSounds * sizeof (PIG_SOUND);
fp->Close ();
delete fp;
return true;
}

//------------------------------------------------------------------------------

bool CTextureManager::LoadTextures (int nVersion, bool bCleanup)
{
if (nVersion < 0) {
	nVersion = Version ();
	//if (!bCleanup 
	//	 && (strcmp (m_pigFiles [nVersion], descentPath [nVersion]) == 0)
	//	 && (_stricmp (m_paletteName, paletteManager.Name ()) == 0))
	//	return;
	strcpy_s (m_pigFiles [nVersion], sizeof (m_pigFiles [nVersion]), descentPath [nVersion]);
	strcpy_s (m_paletteName, sizeof (m_paletteName), paletteManager.Name ());
	LoadInfo (nVersion);
	}
if (!bCleanup)
	Release (nVersion, true, false);
CFileManager* fp = OpenPigFile (nVersion);
if (fp == null)
	return false;
for (int i = 0, j = MaxTextures (nVersion); i < j; i++)
	m_textures [nVersion][i].Load (*fp, i, nVersion);
fp->Close ();
delete fp;
m_bAvailable [nVersion] = true;
return true;
}

//------------------------------------------------------------------------------

CTexture* CTextureManager::AddExtra (ushort nIndex)
{
	CExtraTexture* extraTexP = new CExtraTexture;
if (!extraTexP)
	return null;
extraTexP->m_next = m_extra;
m_extra = extraTexP;
extraTexP->m_index = nIndex;
return extraTexP;
}

//------------------------------------------------------------------------------

inline CBGRA& CTextureManager::Blend (CBGRA& dest, CBGRA& src)
{
if (paletteManager.SuperTransp (src))
	dest.a = 0;
else if (src.a > 0) {
	if (src.a == 255)
		dest = src;
	else {
		byte b = 255 - src.a;
		dest.r = (byte) (((int) dest.r * b + (int) src.r * src.a) / 255);
		dest.g = (byte) (((int) dest.g * b + (int) src.g * src.a) / 255);
		dest.b = (byte) (((int) dest.b * b + (int) src.b * src.a) / 255);
		}
	}
return dest;
}

//------------------------------------------------------------------------------
// textureManager.Define ()
//
// ACTION - Defines data with texture.  If bitmap_handle != 0,
//          then data is copied from the global data.  Otherwise,
//          the pig file defines bmBuf and then global memory
//          is allocated and the data is copied to the global memory.
//          The next time that texture is used, the handle will be defined.
//------------------------------------------------------------------------------

int CTextureManager::Define (short nBaseTex, short nOvlTex, CTexture* destTexP, int x0, int y0) 
{
	typedef struct tFrac {
		int	c, d;
	} tFrac;

	short			nTextures [2], mode, w, h;
	int			i, x, y, y1, offs, s;
	tFrac			scale, scale2;
	//int			rc; // return code
	CTexture*	texP [2];
	CBGRA*		bmDataP = destTexP->Buffer ();
	int			fileType = DLE.FileType ();

nTextures [0] = nBaseTex;
nTextures [1] = nOvlTex & 0x3fff;
mode = nOvlTex & 0xC000;
for (i = 0; i < 2; i++) {
#if 0	
	ASSERT (m_textures [i] < MAX_TEXTURES);
#endif
	if ((nTextures [i] < 0) || (nTextures [i] >= MAX_TEXTURES))
		nTextures [i] = 0;
	// buffer m_textures if not already buffered
	texP [i] = &m_textures [fileType][nTextures [i]];
	//if (!(texP [i]->m_data && texP [i]->m_info.bValid))
	//	if (rc = texP [i]->Load (nTextures [i]))
	//		return rc;
	}
	
	// Define bmDataP based on texture numbers and rotation
destTexP->m_info.width = texP [0]->m_info.width;
destTexP->m_info.height = texP [0]->m_info.height;
destTexP->m_info.bValid = 1;
destTexP->m_override = null;

CBGRA* srcDataP = texP [0]->Buffer ();
if (srcDataP != null) {
	// if not rotated, then copy directly
	if (x0 == 0 && y0 == 0) {
#if 1
		if (texP [1]->Buffer () == null)
			destTexP->m_override = srcDataP;
		else
#endif
			memcpy (bmDataP, srcDataP, texP [0]->BufSize ());
		}
	else {
		// otherwise, copy bit by bit
		w = texP [0]->m_info.width;
		int l1 = y0 * w + x0;
		int l2 = (texP [0]->Size () - l1) * sizeof (CBGRA);
		memcpy (bmDataP, srcDataP + l1, l2 * sizeof (CBGRA));
		memcpy (bmDataP + l2, srcDataP, l1 * sizeof (CBGRA));
		CBGRA* destDataP = bmDataP;
		h = w;//texP [0]->m_info.height;
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				int i = (((y - y0 + h) % h) * w) + ((x - x0 + w) % w);
				*destDataP++ = srcDataP [i];
				}
			}
		}
	}

// Overlay texture 2 if present

if (nTextures [1] == 0)
	return 0;
srcDataP = texP [1]->Buffer ();
if (srcDataP == null)
	return 0;

if (texP [0]->m_info.width == texP [1]->m_info.width)
	scale.c = scale.d = 1;
else if (texP [0]->m_info.width < texP [1]->m_info.width) {
	scale.c = texP [1]->m_info.width / texP [0]->m_info.width;
	scale.d = 1;
	}
else {
	scale.d = texP [0]->m_info.width / texP [1]->m_info.width;
	scale.c = 1;
	}
scale2.c = scale.c * scale.c;
scale2.d = scale.d * scale.d;
offs = 0;
w = texP [1]->m_info.width / scale.c * scale.d;
h = w;//texP [1]->m_info.height / scale.c * scale.d;
s = (texP [1]->m_info.width * texP [1]->m_info.width)/*texP [1]->m_info.size*/ / scale2.c * scale2.d;
if (!(x0 || y0)) {
	CBGRA * destDataP, * destData2;
	if (mode == 0x0000) {
		destDataP = bmDataP;
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++, destDataP++) {
				Blend (*destDataP, srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
		}
	else if (mode == (short) 0x4000) {
		destDataP = bmDataP + h - 1;
		for (y = 0; y < h; y++, destDataP--) {
			for (x = 0, destData2 = destDataP; x < w; x++, destData2 += w) {
				Blend (*destData2, srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
		}
	else if (mode == (short) 0x8000) {
		destDataP = bmDataP + s - 1;
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++, destDataP--) {
				Blend (*destDataP, srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
		}
	else if (mode == (short) 0xC000) {
		destDataP = bmDataP + (h - 1) * w;
		for (y = 0; y < h; y++, destDataP++) {
			for (x = 0, destData2 = destDataP; x < w; x++, destData2 -= w) {
				Blend (*destData2, srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
		}
	} 
else {
	if (mode == 0x0000) {
		for (y = 0; y < h; y++) {
			y1 = ((y + y0) % h) * w;
			for (x = 0; x < w; x++) {
				Blend (bmDataP [y1 + (x + x0) % w], srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
		}
	else if (mode == (short) 0x4000) {
		for (y = h - 1; y >= 0; y--) {
			for (x = 0; x < w; x++) {
				Blend (bmDataP [((x + y0) % h) * w + (y + x0) % w], srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
		}
	else if (mode == (short) 0x8000) {
		for (y = h - 1; y >= 0; y--) {
			y1 = ((y + y0) % h) * w;
			int i = (y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d;
			for (x = w - 1; x >= 0; x--) {
				Blend (bmDataP [y1 + (x + x0) % w], srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
		}
	else if (mode == (short) 0xC000) {
		for (y = 0; y < h; y++) {
			for (x = w - 1; x >= 0; x--) {
				Blend (bmDataP [((x + y0) % h) * w + (y + x0) % w], srcDataP [i]);
				}
			}
		}
	}
return 0;
}

//------------------------------------------------------------------------

void CTextureManager::MarkUsedTextures (void)
{
	CSegment* segP = segmentManager.Segment (0);
	int nVersion = DLE.IsD1File () ? 0 : 1;
	int i, j, h = MaxTextures (nVersion);

for (i = 0; i < h; i++)
	m_textures [nVersion][i].m_info.bUsed = false;

for (i = segmentManager.Count (); i; i--, segP++) {
#ifdef _DEBUG
	if (segP - segmentManager.Segment (0) == 394)
		segP = segP;
#endif
	CSide* sideP = segP->m_sides;
	for (j = 6; j; j--, sideP++) {
		if (((short) sideP->m_info.nChild < 0) || (sideP->m_info.nWall != NO_WALL)) {
			m_textures [nVersion][sideP->m_info.nBaseTex & 0x1FFF].m_info.bUsed = true;
			if ((sideP->m_info.nOvlTex & 0x1FFF) != 0)
				m_textures [nVersion][sideP->m_info.nOvlTex & 0x1FFF].m_info.bUsed = true;
			}
		}
	}

CTexture * texP, * parentTexP = null;

for (i = 0; i < h; i++) {
	texP = &m_textures [nVersion][i];
	if (!texP->m_info.bFrame)
		parentTexP = texP;
	else if (texP->m_info.bCustom && !texP->m_info.bUsed)
		texP->m_info.bUsed = (parentTexP != null) && parentTexP->m_info.bUsed;
	}
}

//------------------------------------------------------------------------
// remove unused custom m_textures

void CTextureManager::RemoveUnusedTextures (void)
{
	int nCustom = CountCustomTextures ();

if (nCustom) {
	MarkUsedTextures ();
	Release (Version (), false, true);
	LoadTextures (-1, true);
	int nRemoved = nCustom - CountCustomTextures ();
	sprintf_s (message, sizeof (message), "%d custom textures %s removed", nRemoved, (nRemoved == 1) ? "was" : "were");
	if (nRemoved)
		undoManager.SetModified (true);
	INFOMSG (message);
	}
}

//------------------------------------------------------------------------

int CTextureManager::ScrollSpeed (UINT16 texture, int *x, int *y)
{
if (DLE.FileType () == RDL_FILE)
	return 0;
*x = 0; 
*y = 0; 
switch (texture) {
	case 399: *x = - 2; break; 
	case 400: *y = - 8; break; 
	case 402: *x = - 4; break; 
	case 405: *y = - 2; break; 
	case 406: *y = - 4; break; 
	case 407: *y = - 2; break; 
	case 348: *x = - 2; *y = - 2; break; 
	case 349: *x = - 2; *y = - 2; break; 
	case 350: *x = + 2; *y = - 2; break; 
	case 401: *y = - 8; break; 
	case 408: *y = - 2; break; 
	default:
		return 0; 
	}
return 1; 
}

//------------------------------------------------------------------------

bool CTextureManager::Reload (int nVersion) 
{ 
if (LoadInfo (nVersion) && LoadTextures (nVersion))
	return true;
sprintf_s (message, sizeof (message), "Couldn't find texture data file (%s).\n\nPlease select the proper folder in the settings dialog.\n", descentPath [nVersion]);
//ErrorMsg (message);
return false;
}

//------------------------------------------------------------------------

bool CTextureManager::Available (void)
{
return m_bAvailable [Version ()];
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//eof m_textures.cpp