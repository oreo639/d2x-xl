
#include "mine.h"
#include "dle-xp.h"
#include "TextureManager.h"

CTextureManager textureManager;

extern short nDbgTexture;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

void CTextureManager::Setup (void)
{
m_extra = null;
m_paletteName [0][0] = 
m_paletteName [1][0] = '\0';
Create (0);
Create (1);
m_arrow.Load (IDR_ARROW_TEXTURE);
for (int i = 0; i < ICON_COUNT; i++)
	m_icons [i].Load (IDR_SMOKE_ICON + i);
}

//------------------------------------------------------------------------

void CTextureManager::Create (int nVersion)
{
m_pigFiles [nVersion][0] = 0;
m_names [nVersion] = null;
LoadNames (nVersion);
m_header [nVersion] = CPigHeader (nVersion);
LoadIndex (nVersion);
}

//------------------------------------------------------------------------

void CTextureManager::Destroy (int nVersion)
{
//Release (nVersion, true, false);
#if 1 //_DEBUG
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
CTexture* texP = &m_textures [nVersion][0];
if (texP != null) {
	for (int i = 0, h = m_header [nVersion].nTextures; i < h; i++, texP++) {
#ifdef _DEBUG
		if (i == nDbgTexture)
			nDbgTexture = nDbgTexture;
#endif
		if (bDeleteUnused) {
			if (texP->m_info.bCustom && !texP->m_info.bUsed)
				texP->Release ();
			}
		else {
			if (bDeleteAll || texP->m_info.bCustom) {
				texP->Release ();
				}
			}
		}		
	}
ReleaseExtras ();
}

//------------------------------------------------------------------------

void CTextureManager::ReleaseExtras (void) 
{
for (CExtraTexture* p = m_extra; p != null; ) {
	m_extra = p->m_next;
	delete p;
	p = m_extra;
	}
}

//------------------------------------------------------------------------

void CTextureManager::Release (bool bDeleteAll, bool bDeleteUnused) 
{
// free any m_textures that have been buffered
for (int i = 0; i < 2; i++) 
	Release (i, bDeleteAll, bDeleteUnused);
ReleaseExtras ();
}

//------------------------------------------------------------------------

CFileManager* CTextureManager::OpenPigFile (int nVersion)
{
	CFileManager* fp = new CFileManager;
	char	filename [256], appFolder [256];

if ((strchr (descentFolder [nVersion], ':') == null) && (descentFolder [nVersion][0] != '\\')) {
	::GetModuleFileName (0, appFolder, sizeof (appFolder));
	CFileManager::SplitPath (appFolder, filename, null, null);
	strcat_s (filename, sizeof (filename), descentFolder [nVersion]);
	}
else
	strcpy_s (filename, sizeof (filename), descentFolder [nVersion]);
if (!strstr (filename, ".pig"))
	strcat_s (filename, sizeof (filename), "groupa.pig");
if (!fp->Open (filename, "rb")) {
	delete fp;
	m_bAvailable [nVersion] = false;
	return null;
	}
CFileManager::SplitPath (filename, null, m_paletteName [1], null);
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

if (!texP)
	return false;
for (int i = m_header [DLE.FileType ()].nTextures; i; i--, texP++)
	if (texP->m_info.bCustom)
		return true;
return false;
}

//------------------------------------------------------------------------

int CTextureManager::CountCustomTextures (void) 
{
	int			count = 0;
	CTexture*	texP = &m_textures [DLE.FileType ()][0];

if (!texP)
	return 0;
for (int i = m_header [DLE.FileType ()].nTextures; i; i--, texP++)
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
	int l = (int) res.Length () + 1;
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
	strcpy_s (m_pigFiles [nVersion], sizeof (m_pigFiles [nVersion]), descentFolder [nVersion]);
	strcpy_s (m_paletteName [0], sizeof (m_paletteName [0]), paletteManager.Name ());
	LoadInfo (nVersion);
	}
m_textures [nVersion].Resize (m_header [nVersion].nTextures);
if (!bCleanup)
	Release (nVersion, true, false);
CFileManager* fp = OpenPigFile (nVersion);
if (fp == null)
	return false;
paletteManager.Reload (m_paletteName [1]);

CTexture* textures = m_textures [nVersion].Buffer ();
ushort* index = m_index [nVersion];
for (int i = 0, j = m_header [nVersion].nTextures; i < j; i++)
	textures [i].Load (*fp, i, nVersion);
for (int i = 0, j = MaxTextures (nVersion); i < j; i++) {
	textures [index [i] - 1].m_info.bFrame = (strstr (textureManager.Name (nVersion, i), "frame") != null);
	}
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
	dest.r =
	dest.g =
	dest.b =
	dest.a = 0;
else if (src.a > 0) {
	if (src.a == 255)
		dest = src;
	else {
		ubyte b = 255 - src.a;
		dest.r = (ubyte) (((int) dest.r * b + (int) src.r * src.a) / 255);
		dest.g = (ubyte) (((int) dest.g * b + (int) src.g * src.a) / 255);
		dest.b = (ubyte) (((int) dest.b * b + (int) src.b * src.a) / 255);
		}
	}
return dest;
}

//------------------------------------------------------------------------------
// textureManager.BlendTextures ()
//
// Builds a texture for rendering by blending base and optional overlay texture
// and writes the result into a temporary buffer
//------------------------------------------------------------------------------

int CTextureManager::BlendTextures (short nBaseTex, short nOvlTex, CTexture* destTexP, int x0, int y0) 
{
if (!textureManager.Available ())
	return 1;

	typedef struct tFrac {
		int	c, d;
	} tFrac;

	short			nTextures [2], mode, w, h;
	int			i, offs;
	tFrac			scale;
	float			fScale;
	//int			rc; // return code
	CTexture*	texP [2];
	CBGRA*		bmBufP = destTexP->Buffer ();
	int			fileType = DLE.FileType ();

mode = (nOvlTex >> ALIGNMENT_SHIFT) & 3;
nTextures [0] = nBaseTex;
nTextures [1] = nOvlTex & TEXTURE_MASK;
for (i = 0; i < 2; i++) {
	if ((nTextures [i] < 0) || (nTextures [i] >= MAX_TEXTURES))
		nTextures [i] = 0;
	texP [i] = Texture (nTextures [i]);
	}
	
// Define bmBufP based on texture numbers and rotation
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
			{
			w = texP [0]->Width (0);
			memcpy (bmBufP, srcDataP, w * w * sizeof (CBGRA));
			}
		}
	else {
		// otherwise, copy bit by bit
		w = texP [0]->Width (0);
#if 0
		int l1 = y0 * w + x0;
		int l2 = texP [0]->Size () - l1;
		memcpy (bmBufP, srcDataP + l1, l2 * sizeof (CBGRA));
		memcpy (bmBufP + l2, srcDataP, l1 * sizeof (CBGRA));
#else
		h = w;//texP [0]->m_info.height;
#ifdef NDEBUG
//#pragma omp parallel for 
#endif
		int dx = (x0 + w) % w;
		for (int y = 0; y < h; y++) {
			CBGRA* destDataP = bmBufP + y * w;
			int dy = ((y - y0 + h) % h) * w;
#if 1
			memcpy (bmBufP + y * w, srcDataP + dy + dx, (w - dx) * sizeof (CBGRA));
			if (dx)
				memcpy (bmBufP + y * w + (w - dx), srcDataP + dy + dx, dx * sizeof (CBGRA));
#else
			for (int x = 0; x < w; x++) {
				int i = dy + ((x - x0 + w) % w);
				*destDataP++ = srcDataP [i];
				}
#endif
			}
#endif
		}
	}

// Overlay texture 2 if present

if (nTextures [1] == 0)
	return 0;
srcDataP = texP [1]->Buffer ();
if (srcDataP == null)
	return 0;

if (texP [0]->m_info.width == texP [1]->m_info.width) {
	scale.c = scale.d = 1;
	fScale = 1.0;
	}
else if (texP [0]->m_info.width < texP [1]->m_info.width) {
	scale.c = texP [1]->m_info.width / texP [0]->m_info.width;
	scale.d = 1;
	fScale = float (texP [1]->m_info.width) / float (texP [0]->m_info.width);
	}
else {
	scale.d = texP [0]->m_info.width / texP [1]->m_info.width;
	scale.c = 1;
	fScale = float (texP [1]->m_info.width) / float (texP [0]->m_info.width);
	}
offs = 0;
w = texP [1]->m_info.width / scale.c * scale.d;
h = w;//texP [1]->m_info.height / scale.c * scale.d;

if (!(x0 || y0)) {
	if (mode == 0) {
//#pragma omp parallel for
		for (int y = 0; y < h; y++) {
			CBGRA* destDataP = bmBufP + y * w;
			float offset = float (int (y * fScale) * int (w * fScale));
			for (int x = 0; x < w; x++, destDataP++, offset += fScale) 
				Blend (*destDataP, srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				//Blend (*destDataP, srcDataP [int (offset)]);
			}
		}
	else if (mode == 1) {
#if 0
		CBGRA* destDataP = bmBufP + h - 1;
		for (int y = 0; y < h; y++, destDataP--) {
			CBGRA* destData2 = destDataP;
			for (int x = 0; x < w; x++, destData2 += w) {
				Blend (*destData2, srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
#else
		bmBufP += h - 1;
#pragma omp parallel for
		for (int y = 0; y < h; y++) {
			CBGRA* destDataP = bmBufP - y;
			float offset = float (int (y * fScale) * int (w * fScale));
			for (int x = 0; x < w; x++, destDataP += w, offset += fScale)
				//Blend (*destDataP, srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				Blend (*destDataP, srcDataP [int (offset)]);
			}
#endif
		}
	else if (mode == 2) {
#if 0
		CBGRA* destDataP = bmBufP + h * w;
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				Blend (*(--destDataP), srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
#else
		bmBufP += h * w;
#pragma omp parallel for
		for (int y = 0; y < h; y++) {
			CBGRA* destDataP = bmBufP - y * w;
			float offset = float (int (y * fScale) * int (w * fScale));
			for (int x = 0; x < w; x++, offset += fScale)
				//Blend (*(--destDataP), srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				Blend (*(--destDataP), srcDataP [int (offset)]);
			}
#endif
		}
	else if (mode == 3) {
#if 0
		CBGRA* destDataP = bmBufP + (h - 1) * w;
		for (int y = 0; y < h; y++, destDataP++) {
			CBGRA* destData2 = destDataP;
			for (int x = 0; x < w; x++, destData2 -= w) {
				Blend (*destData2, srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				}
			}
		}
#else
		bmBufP += (h - 1) * w;
#pragma omp parallel for
		for (int y = 0; y < h; y++) {
			CBGRA* destDataP = bmBufP + y;
			float offset = float (int (y * fScale) * int (w * fScale));
			for (int x = 0; x < w; x++, destDataP -= w, offset += fScale)
				//Blend (*destDataP, srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				Blend (*destDataP, srcDataP [int (offset)]);
			}
		}
#endif
	} 
else {
	if (mode == 0) {
#pragma omp parallel for
		for (int y = 0; y < h; y++) {
			int y1 = ((y + y0) % h) * w;
			float offset = float (int (y * fScale) * int (w * fScale));
			for (int x = 0; x < w; x++, offset += fScale)
				//Blend (bmBufP [y1 + (x + x0) % w], srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				Blend (bmBufP [y1 + (x + x0) % w], srcDataP [int (offset)]);
			}
		}
	else if (mode == 1) {
#pragma omp parallel for
		for (int y = h - 1; y >= 0; y--) {
			int y1 = (y + x0) % w;
			float offset = float (int (y * fScale) * int (w * fScale));
			for (int x = 0; x < w; x++, offset += fScale)
				//Blend (bmBufP [y1 + ((x + y0) % h) * w], srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				Blend (bmBufP [((x + y0) % h) * w + (y + x0) % w], srcDataP [int (offset)]);
			}
		}
	else if (mode == 2) {
#pragma omp parallel for
		for (int y = h - 1; y >= 0; y--) {
			int y1 = ((y + y0) % h) * w;
			float offset = float (int (y * fScale) * int (w * fScale));
			for (int x = w - 1; x >= 0; x--, offset += fScale)
				//Blend (bmBufP [y1 + (x + x0) % w], srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
				Blend (bmBufP [y1 + (x + x0) % w], srcDataP [int (offset)]);
			}
		}
	else if (mode == 3) {
#pragma omp parallel for
		for (int y = 0; y < h; y++) {
			int y1 = (y + x0) % w;
			for (int x = w - 1; x >= 0; x--) {
				Blend (bmBufP [y1 + ((x + y0) % h) * w], srcDataP [i]);
				}
			}
		}
	}

return 0;
}

//------------------------------------------------------------------------

void CTextureManager::UnmarkTextures (void)
{
if (!textureManager.Available ())
	return;

	int nVersion = DLE.IsD1File () ? 0 : 1;
	int i, h = MaxTextures (nVersion);

for (i = 0; i < h; i++)
	m_textures [nVersion][m_index [nVersion][i] - 1].m_info.bUsed = false;
}

//------------------------------------------------------------------------

void CTextureManager::MarkUsedTextures (void)
{
if (!textureManager.Available ())
	return;

	CSegment* segP = segmentManager.Segment (0);
	int nVersion = DLE.IsD1File () ? 0 : 1;
	int i, j, h = MaxTextures (nVersion);

UnmarkTextures ();

for (i = segmentManager.Count (); i; i--, segP++) {
	CSide* sideP = segP->m_sides;
	for (j = 6; j; j--, sideP++) {
		if (((short) sideP->m_info.nChild < 0) || (sideP->m_info.nWall != NO_WALL)) {
			Texture (sideP->BaseTex ())->m_info.bUsed = true;
			if ((sideP->OvlTex (0)) != 0)
				Texture (sideP->OvlTex (0))->m_info.bUsed = true;
			}
		}
	}

CTexture * texP, * parentTexP = null;

for (i = 0; i < h; i++) {
	texP = Texture (i);
	if (!texP->m_info.bFrame)
		parentTexP = texP;
	else if (texP->m_info.bCustom && !texP->m_info.bUsed)
		texP->m_info.bUsed = (parentTexP != null) && parentTexP->m_info.bUsed;
	}
}

//------------------------------------------------------------------------
// remove unused custom m_textures

void CTextureManager::RemoveTextures (bool bUnused)
{
	int nCustom = CountCustomTextures ();

if (nCustom) {
	if (bUnused)
		MarkUsedTextures ();
	else
		UnmarkTextures ();
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

bool CTextureManager::IsAnimated (UINT16 texture)
{
if (DLE.FileType () == RDL_FILE)
	return false;
switch (texture) {
	case 399:
	case 400:
	case 402:
	case 405:
	case 406:
	case 407:
	case 348:
	case 349:
	case 350:
	case 401:
	case 408:
		return true;
	default:
		return false;
	}
}

//------------------------------------------------------------------------

int CTextureManager::ScrollSpeed (UINT16 nTexture, int *x, int *y)
{
if (DLE.FileType () == RDL_FILE)
	return 0;
*x = 0; 
*y = 0; 
switch (nTexture) {
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

//------------------------------------------------------------------------------

int CTextureManager::ScrollDirection (UINT16 nTexture)
{
int sx, sy;
if (!textureManager.ScrollSpeed (nTexture, &sx, &sy))
	return -1;
#ifdef _DEBUG
if (nTexture == nDbgTexture)
	nDbgTexture = nDbgTexture;
#endif
if (sy > 0) {
	if (sx > 0)
		return 3;
	if (sx < 0)
		return 5;
	return 4;
	}
else if (sy < 0) {
	if (sx < 0)
		return 7;
	if (sx > 0)
		return 1;
	return 0;
	}
else {
	if (sx > 0)
		return 6;
	if (sx < 0)
		return 2;
	}
return -1;
}

//------------------------------------------------------------------------

bool CTextureManager::Reload (int nVersion, bool bForce) 
{ 
if (!bForce && m_textures [nVersion].Buffer ()) 
	return true;
if (LoadInfo (nVersion) && LoadTextures (nVersion))
	return true;
sprintf_s (message, sizeof (message), "Couldn't find texture data file (%s).\n\nPlease select the proper folder in the settings dialog.\n", descentFolder [nVersion]);
//ErrorMsg (message);
return false;
}

//------------------------------------------------------------------------

bool CTextureManager::Available (int nVersion)
{
return m_bAvailable [(nVersion < 0) ? Version () : nVersion];
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//eof m_textures.cpp