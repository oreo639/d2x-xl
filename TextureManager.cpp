
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
#if EXTRA_TEXTURES
m_extra = null;
#endif
m_paletteName [0][0] = 
m_paletteName [1][0] = '\0';
Create (0);
Create (1);
m_arrow.LoadFromResource (IDR_ARROW_TEXTURE);
for (int i = 0; i < ICON_COUNT; i++)
	m_icons [i].LoadFromResource (IDR_SMOKE_ICON + i);
}

//------------------------------------------------------------------------

void CTextureManager::Create (int nVersion)
{
m_pigFiles [nVersion][0] = 0;
m_names [nVersion] = null;
LoadNames (nVersion);
m_header [nVersion] = CPigHeader (nVersion);
LoadIndex (nVersion);
m_bUsed [nVersion] = new bool [MaxTextures (nVersion)];
for (int i = 0; i < MaxTextures (nVersion); i++)
	m_bUsed [nVersion][i] = false;
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
			delete [] m_names [nVersion][j];
	delete [] m_names [nVersion];
	m_names [nVersion] = null;
	}
if (m_index [nVersion]) {
	delete [] m_index [nVersion];
	m_index [nVersion] = null;
	}
if (m_bUsed [nVersion]) {
	delete [] m_bUsed [nVersion];
	m_bUsed [nVersion] = null;
	}
if (m_info [nVersion]) {
	delete [] m_info [nVersion];
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

void CTextureManager::ReleaseTextures (int nVersion) 
{
// free any m_textures that have been buffered
CTexture* texP = &m_textures [nVersion][0];
if (texP != null) {
	for (int i = 0, h = m_header [nVersion].nTextures; i < h; i++, texP++) {
#ifdef _DEBUG
		if (i == nDbgTexture)
			nDbgTexture = nDbgTexture;
#endif
		if (m_overrides [nVersion][i]) {
			delete m_overrides [nVersion][i];
			m_overrides [nVersion][i] = null;
			}
		if (m_previous [nVersion][i]) {
			if (m_previous [nVersion][i] != texP)
				delete m_previous [nVersion][i];
			m_previous [nVersion][i] = null;
			}
		texP->Clear ();
		}
	}
#if EXTRA_TEXTURES
ReleaseExtras ();
#endif
}

//------------------------------------------------------------------------

void CTextureManager::ReleaseTextures (void) 
{
// free any m_textures that have been buffered
for (int i = 0; i < 2; i++) 
	ReleaseTextures (i);
#if EXTRA_TEXTURES
ReleaseExtras ();
#endif
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

int CTextureManager::AllTextureCount (int nVersion)
{
return m_header [(nVersion < 0) ? Version () : nVersion].nTextures;
}

//------------------------------------------------------------------------

bool CTextureManager::FindLevelTex (uint nTexAll, int *pnTexLevel, int nVersion)
{
	int nVersionResolved = (nVersion < 0) ? Version () : nVersion;

// Search the index for this texture ID
for (int i = 0; i < MaxTextures (nVersionResolved); i++) {
	if (m_index [nVersionResolved][i] - 1 == nTexAll) {
		*pnTexLevel = i;
		return true;
		}
	}

return false;
}

//------------------------------------------------------------------------

bool CTextureManager::HasCustomTextures (void) 
{
for (int i = 0; i < AllTextureCount (); i++)
	if (AllTextures ((uint)i)->IsCustom ())
		return true;
return false;
}

//------------------------------------------------------------------------

int CTextureManager::CountCustomTextures (void) 
{
	int			count = 0;

for (int i = 0; i < AllTextureCount (); i++)
	if (AllTextures ((uint)i)->IsCustom ())
		count++;
return count;
}

//------------------------------------------------------------------------

int CTextureManager::CountModifiedTextures (void)
{
	int count = 0;

for (int i = 0; i < AllTextureCount (); i++)
	if (m_bModified [Version ()][i])
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
	delete [] m_info [nVersion];
	m_info [nVersion] = null;
	}
CFileManager* fp = OpenPigFile (nVersion);
if (fp == null)
	return false;
m_header [nVersion].Read (*fp);
// Some sanity checks in case the user opened a corrupt/wrong .PIG - reduces chance of crash
if ((nVersion == 1 && m_header [nVersion].nId != 0x47495050) || (m_header [nVersion].nTextures > 0x00100000)) {
	m_header [nVersion] = CPigHeader (nVersion);
	fp->Close ();
	delete fp;
	return false;
	}
else {
	m_info [nVersion] = new CPigTexture [m_header [nVersion].nTextures];
	for (int i = 0; i < m_header [nVersion].nTextures; i++)
		m_info [nVersion][i].Read (fp, nVersion);
	m_nOffsets [nVersion] = fp->Tell ();
	if (nVersion == 0)
		m_nOffsets [0] += m_header [0].nSounds * sizeof (PIG_SOUND);
	}
fp->Close ();
delete fp;
return true;
}

//------------------------------------------------------------------------------

bool CTextureManager::LoadTextures (int nVersion, bool bClearExisting)
{
if (nVersion < 0) {
	nVersion = Version ();
	strcpy_s (m_pigFiles [nVersion], sizeof (m_pigFiles [nVersion]), descentFolder [nVersion]);
	strcpy_s (m_paletteName [nVersion], sizeof (m_paletteName [nVersion]), paletteManager.Name ());
	LoadInfo (nVersion);
	}
if (!m_textures [nVersion].Buffer ()) {
	m_textures [nVersion].Create (m_header [nVersion].nTextures);
	m_overrides [nVersion].Create (m_header [nVersion].nTextures);
	m_overrides [nVersion].Clear (0);
	m_bModified [nVersion].Create (m_header [nVersion].nTextures);
	m_bModified [nVersion].Clear (0);
	m_previous [nVersion].Create (m_header [nVersion].nTextures);
	m_previous [nVersion].Clear (0);
	}
else if (bClearExisting) {
	ReleaseTextures (nVersion);
	m_textures [nVersion].Resize (m_header [nVersion].nTextures);
	m_overrides [nVersion].Resize (m_header [nVersion].nTextures);
	m_bModified [nVersion].Resize (m_header [nVersion].nTextures);
	m_previous [nVersion].Resize (m_header [nVersion].nTextures);
	}
CFileManager* fp = OpenPigFile (nVersion);
if (fp == null)
	return false;
paletteManager.Reload (m_paletteName [1]);

CTexture* textures = m_textures [nVersion].Buffer ();
for (int i = 0, j = m_header [nVersion].nTextures; i < j; i++)
	textures [i].LoadFromPig (*fp, i, nVersion);
for (int i = 0; i < m_header [nVersion].nTextures; i++) {
	if (textures [i].IsAnimated () && textures [i].FrameNum () == 0) {
		textures [i].CalculateFrameCount ();
		}
	}
fp->Close ();
delete fp;
m_bAvailable [nVersion] = true;
return true;
}

//------------------------------------------------------------------------------

CTexture* CTextureManager::OverrideTexture (uint nTexAll, const CTexture* newTexture, int nVersion)
{
	int nVersionResolved = (nVersion < 0) ? Version () : nVersion;
	CTexture* texture = new CTexture ();

if (newTexture != null)
	texture->Copy (*newTexture);
else
	texture->Copy (*AllTextures (nTexAll, nVersionResolved));
texture->SetCustom ();

// We want to retain a copy of the texture as it was at load-time - custom or stock -
// so we know what to revert to if requested
if (m_overrides [nVersionResolved][nTexAll] != null && m_previous [nVersionResolved][nTexAll] != null)
	delete m_overrides [nVersionResolved][nTexAll];
else if (m_overrides [nVersionResolved][nTexAll] != null)
	m_previous [nVersionResolved][nTexAll] = m_overrides [nVersionResolved][nTexAll];
else
	m_previous [nVersionResolved][nTexAll] = &m_textures [nVersionResolved][nTexAll];

m_overrides [nVersionResolved][nTexAll] = texture;
m_bModified [nVersionResolved][nTexAll] = true;
return m_overrides [nVersionResolved][nTexAll];
}

//------------------------------------------------------------------------------

void CTextureManager::RevertTexture (uint nTexAll, int nVersion)
{
	int nVersionResolved = (nVersion < 0) ? Version () : nVersion;

if (m_overrides [nVersionResolved][nTexAll] == null)
	return;
else if (m_previous [nVersionResolved][nTexAll] != null) {
	delete m_overrides [nVersionResolved][nTexAll];
	m_overrides [nVersionResolved][nTexAll] = null;
	if (m_previous [nVersionResolved][nTexAll] == &m_textures [nVersionResolved][nTexAll]) {
		// Revert operation counts as an undo, so unset modified
		m_previous [nVersionResolved][nTexAll] = null;
		m_bModified [nVersionResolved][nTexAll] = false;
		}
	}
else {
	m_previous [nVersionResolved][nTexAll] = m_overrides [nVersionResolved][nTexAll];
	m_overrides [nVersionResolved][nTexAll] = null;
	m_bModified [nVersionResolved][nTexAll] = true;
	}
}

//------------------------------------------------------------------------

void CTextureManager::UnTagUsedTextures (void)
{
if (!textureManager.Available ())
	return;

	int nVersion = DLE.IsD1File () ? 0 : 1;
	int i, h = MaxTextures (nVersion);

for (i = 0; i < h; i++)
	m_bUsed [nVersion][i] = false;
}

//------------------------------------------------------------------------

void CTextureManager::TagUsedTextures (void)
{
if (!textureManager.Available ())
	return;

	CSegment* segP = segmentManager.Segment (0);
	int nVersion = DLE.IsD1File () ? 0 : 1;
	int i, j, h = MaxTextures (nVersion);

UnTagUsedTextures ();

for (i = segmentManager.Count (); i; i--, segP++) {
	CSide* sideP = segP->m_sides;
	for (j = 6; j; j--, sideP++) {
		if (((short) sideP->m_info.nChild < 0) || (sideP->m_info.nWall != NO_WALL)) {
			if (sideP->BaseTex () >= 0 && sideP->BaseTex () < MaxTextures (nVersion))
				m_bUsed [nVersion][sideP->BaseTex ()] = true;
			if (sideP->OvlTex (0) > 0 && sideP->OvlTex (0) < MaxTextures (nVersion))
				m_bUsed [nVersion][sideP->OvlTex (0)] = true;
			}
		}
	}

for (i = 0; i < h; i++) {
	const CTexture* texP = Textures (i);
	if (texP->IsAnimated () && m_bUsed [nVersion][texP->GetParent ()->IdLevel()])
		m_bUsed [nVersion][i] = true;
	}
}

//------------------------------------------------------------------------

int CTextureManager::UsedCount (uint nTexAll)
{
if (!textureManager.Available ())
	return 0;

int nTexLevel;
if (!FindLevelTex (nTexAll, &nTexLevel))
	return 0;

const CTexture *texP = Textures (nTexLevel);

if (texP->IsAssignableFrame ()) {
	// This is a frame of an animated texture - we need to find the base before
	// we can get the used count
	bool bFoundParent = false;
	for (int i = nTexLevel - 1; i > 0; i--) {
		texP = Textures (i);
		if (!texP->IsAssignableFrame ()) {
			bFoundParent = true;
			break;
			}
		}
	if (!bFoundParent) {
		// No parent. Weird and not supposed to happen.
		return 0;
		}
	}

int usedCount = 0;
CSegment* segP = segmentManager.Segment (0);
for (int i = 0; i < segmentManager.Count (); i++, segP++) {
	CSide* sideP = segP->m_sides;
	for (int j = 0; j < MAX_SIDES_PER_SEGMENT; j++, sideP++) {
		if (((short) sideP->m_info.nChild < 0) || (sideP->m_info.nWall != NO_WALL)) {
			// We aren't double-counting if the texture was used for both base AND overlay.
			// Index 0 on an overlay texture doesn't count as an instance either.
			if (sideP->BaseTex () == texP->IdLevel () || (texP->IdLevel () > 0 && sideP->OvlTex (0) == texP->IdLevel ()))
				usedCount++;
			}
		}
	}

return usedCount;
}

//------------------------------------------------------------------------
// remove unused custom m_textures

void CTextureManager::RemoveCustomTextures (bool bUnusedOnly)
{
	int nCustom = CountCustomTextures ();

if (nCustom) {
	if (bUnusedOnly)
		TagUsedTextures ();
	else
		UnTagUsedTextures ();

	int nRemoved = 0;
	for (int i = 0; i < m_header [Version ()].nTextures; i++) {
		if (AllTextures (i)->IsCustom ()) {
			if (!bUnusedOnly || !IsTextureUsed (AllTextures (i)->IdLevel ())) {
				RevertTexture (i);
				nRemoved++;
				}
			}
		}

	sprintf_s (message, sizeof (message), "%d custom textures %s removed", nRemoved, (nRemoved == 1) ? "was" : "were");
	if (nRemoved)
		undoManager.SetModified (true);
	INFOMSG (message);
	}
}

//------------------------------------------------------------------------

void CTextureManager::CommitTextureChanges (void)
{
for (int i = 0; i < AllTextureCount (); i++)
	if (m_bModified [Version ()][i]) {
		m_previous [Version ()][i] = null;
		m_bModified [Version ()][i] = false;
		}
}

//------------------------------------------------------------------------

void CTextureManager::UndoTextureChanges (void)
{
	int nReverted = 0;

for (int i = 0; i < AllTextureCount (); i++)
	if (m_bModified [Version ()][i]) {
		if (m_previous [Version ()][i] == &m_textures [Version ()][i]) {
			// Revert to default texture
			delete m_overrides [Version ()][i];
			m_overrides [Version ()][i] = null;
			}
		else if (m_overrides [Version ()][i] != m_previous [Version ()][i]) {
			delete m_overrides [Version ()][i];
			m_overrides [Version ()][i] = m_previous [Version ()][i];
			}
		m_previous [Version ()][i] = null;
		m_bModified [Version ()][i] = false;
		nReverted++;
		}

if (nReverted > 0) {
	sprintf_s (message, sizeof (message), "%d modified texture%s reverted", nReverted, (nReverted == 1) ? " was" : "s were");
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
// Scrolling is the direction the "view" moves over the texture, not
// the direction the texture appears to be moving, so we need to invert.
// Textures are bottom-up so y needs to be flipped an additional time.
// +y = down, -y = up, +x = left, -x = right
if (sy > 0) {
	if (sx < 0)
		return 3;
	if (sx > 0)
		return 5;
	return 4;
	}
else if (sy < 0) {
	if (sx < 0)
		return 1;
	if (sx > 0)
		return 7;
	return 0;
	}
else {
	if (sx < 0)
		return 2;
	if (sx > 0)
		return 6;
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

void CTextureManager::CreateGLTextures (int nVersion)
{
	int nVersionResolved = (nVersion < 0) ? Version () : nVersion;

if (!Available (nVersionResolved))
	return;

for (int i = 0; i < AllTextureCount (); i++)
	m_textures [nVersionResolved][i].GLCreate (false);
m_arrow.GLCreate (false);
for (int i = 0; i < ICON_COUNT; i++)
	m_icons [i].GLCreate (false);
}

//------------------------------------------------------------------------

bool CTextureManager::ChangePigFile (const char *pszPigPath, int nVersion)
{
	int nVersionResolved = (nVersion < 0) ? Version () : nVersion;
	char szPigName [256] = {0};
	// Save these, they will change
	CBGR transparentColor = *paletteManager.Current (255);
	CBGR superTransparentColor = *paletteManager.Current (254);

// Set PIG path
strcpy_s (descentFolder [nVersionResolved], sizeof (descentFolder [nVersionResolved]), pszPigPath);
CFileManager::SplitPath (pszPigPath, null, szPigName, null);
strcat_s (szPigName, sizeof (szPigName), ".pig");
paletteManager.SetName (szPigName);

// Reload palette and textures - but keep overrides
for (int i = 0; i < m_header [nVersionResolved].nTextures; i++)
	m_textures [nVersionResolved][i].Clear (); // Force reload
if (!LoadInfo (nVersionResolved) || !LoadTextures (nVersionResolved, false)) {
	sprintf_s (message, sizeof (message), "Couldn't find texture data file (%s).\n\n"
		"Please select the proper folder in the settings dialog.\n", descentFolder [nVersion]);
	return false;
	}

// Change palettes for loaded custom textures - mark all as changed via OverrideTexture to allow saving to work correctly
// TGA format textures don't need any changes
for (uint nTexAll = 0, j = m_header [nVersionResolved].nTextures; nTexAll < j; nTexAll++) {
	if (m_overrides [nVersionResolved][nTexAll] != null && m_overrides [nVersionResolved][nTexAll]->Format () != TGA) {
		CTexture *pNewTexture = OverrideTexture (nTexAll, null, nVersionResolved);
		for (uint i = 0; i < pNewTexture->Size (); i++) {
			// Even BMP format textures are stored in RGBA format in memory, which makes
			// this pretty simple - just need to watch out for transparent color keys
			ubyte newIndex;
			if (*pNewTexture->Buffer (i) == transparentColor)
				newIndex = 255;
			else if (*pNewTexture->Buffer (i) == superTransparentColor)
				newIndex = 254;
			else
				newIndex = paletteManager.ClosestColor (*pNewTexture->Buffer (i), false);
			ubyte alpha = (newIndex < 254) ? 255 : 0;
			*pNewTexture->Buffer (i) = *paletteManager.Current (newIndex);
			pNewTexture->Buffer (i)->a = alpha;
			}
		pNewTexture->CommitBufferChanges ();
		}
	}

DLE.TextureView ()->Setup ();
DLE.TextureView ()->Refresh ();
DLE.MineView ()->ResetView (true);
return true;
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