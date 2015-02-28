
#include "mine.h"
#include "dle-xp.h"
#include "TextureManager.h"

CTextureManager textureManager;

extern short nDbgTexture;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

int CAnimationClipInfo::LoadAnimationFrames (CFileManager& fp, int nMaxFrames, bool bIndices) 
{
m_frames.Read (fp);
int l = (int) m_frames.Length ();
if (bIndices)
	for (int i = 0; i < l; i++)
		m_frames [i] = -m_frames [i];
if (l < nMaxFrames)
	fp.Seek ((nMaxFrames - l) * sizeof (short), SEEK_CUR);
return m_frames.Buffer () ? (int) m_frames.Length () : -1;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

static int LoadAnimationClip (CAnimationClipInfo& aci, CFileManager& fp, int nMaxFrames, bool bIndices)
{
aci.m_nPlayTime = fp.ReadInt32 ();
int nFrames = fp.ReadInt32 ();
if (nFrames > nMaxFrames)
	return -1;
if (nFrames > 0)
	aci.m_frames.Create (nFrames);
aci.m_nFrameTime = int (X2F (fp.ReadInt32 ()) * 1000);
aci.m_nFrameTime = int (X2F (aci.m_nPlayTime) / float (nFrames) * 1000);
fp.Seek (sizeof (int) + sizeof (short), SEEK_CUR);
if (nFrames < 0) {
	fp.Seek (nMaxFrames * sizeof (short), SEEK_CUR);
	aci.m_nTexture = -1;
	}
else {
	aci.LoadAnimationFrames (fp, nMaxFrames, bIndices);
	aci.m_nTexture = aci.Frame (0);
	}
fp.ReadInt32 ();
return aci.m_frames.Length ();
}

//------------------------------------------------------------------------

static int LoadAnimationData (CAnimationClipList& animations, CFileManager& fp, int nFrames)
{
	int nClips = fp.ReadInt32 ();

for (int i = 0; i < nClips; i++) {
	CAnimationClipInfo* aciP = animations.Append ();
	if (!aciP)
		return -1;
	if (0 > LoadAnimationClip (*aciP, fp, nFrames, false))
		return -1;
	}
return nClips;
}

//------------------------------------------------------------------------

static int LoadEffectData (CAnimationClipInfo& aci, CFileManager& fp, int nMaxFrames)
{
if (0 > LoadAnimationClip (aci, fp, nMaxFrames, true))
	return -1;
fp.Seek (8, SEEK_CUR);
aci.m_nTexture = fp.ReadInt16 (); // nChangingWallTexture
fp.Seek (38, SEEK_CUR);
return 1;
}

//------------------------------------------------------------------------

static int LoadEffectClips (CAnimationClipList& animations, CFileManager& fp, int nMaxFrames)
{
	int nClips = fp.ReadInt32 ();

for (int i = 0; i < nClips; i++) {
	CAnimationClipInfo* aciP = animations.Append ();
	if (!aciP)
		return -1;
	if (0 > LoadEffectData (*aciP, fp, nMaxFrames))
		return -1;
	}
return nClips;
}

// -----------------------------------------------------------------------------------

int LoadWallEffectData (CAnimationClipInfo& aci, CFileManager& fp, short nMaxFrames)
{
aci.m_nPlayTime = fp.ReadInt32 ();
short nFrames = fp.ReadInt16 ();
if (nFrames > nMaxFrames)
	return -1;
if (nFrames < 0) {
	fp.Seek (nMaxFrames * sizeof (short), SEEK_CUR);
	aci.m_nTexture = -1;
	}
else {
	aci.m_frames.Create (nFrames);
	nFrames = aci.LoadAnimationFrames (fp, nMaxFrames, false);
	aci.m_nFrameTime = int (X2F (nFrames ? aci.m_nPlayTime / nFrames : 0) * 1000);
	aci.m_nTexture = aci.Frame (0);
	}
fp.Seek (20, SEEK_CUR);
aci.m_bBidirectional = true;
return aci.m_frames.Length ();
}

//------------------------------------------------------------------------

static int LoadWallEffectClips (CAnimationClipList& animations, CFileManager& fp, int nMaxFrames)
{
	int nClips = fp.ReadInt32 ();

for (int i = 0; i < nClips; i++) {
	CAnimationClipInfo* aciP = animations.Append ();
	if (!aciP)
		return -1;
	if (0 > LoadWallEffectData (*aciP, fp, nMaxFrames))
		return -1;
	}
return nClips;
}

//------------------------------------------------------------------------

int CAnimationClipLoaderD1::LoadAnimationClips (CAnimationClipList& animations, CFileManager& fp)
{
return 0;
}

//------------------------------------------------------------------------

int CAnimationClipLoaderD2::LoadAnimationClips (CAnimationClipList& animations, CFileManager& fp)
{
return LoadAnimationData (animations, fp, MAX_ANIMATION_FRAMES);
}

//------------------------------------------------------------------------

int CEffectClipLoaderD1::LoadAnimationClips (CAnimationClipList& animations, CFileManager& fp)
{
return LoadEffectClips (animations, fp, MAX_ANIMATION_FRAMES);
}

//------------------------------------------------------------------------

int CEffectClipLoaderD2::LoadAnimationClips (CAnimationClipList& animations, CFileManager& fp)
{
return LoadEffectClips (animations, fp, MAX_ANIMATION_FRAMES);
}

//------------------------------------------------------------------------

int CWallEffectClipLoaderD1::LoadAnimationClips (CAnimationClipList& animations, CFileManager& fp)
{
return LoadWallEffectClips (animations, fp, MAX_WALL_EFFECT_FRAMES_D1);
}

//------------------------------------------------------------------------

int CWallEffectClipLoaderD2::LoadAnimationClips (CAnimationClipList& animations, CFileManager& fp)
{
return LoadWallEffectClips (animations, fp, MAX_WALL_EFFECT_FRAMES_D2);
}

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

int CTextureManager::GlobalTextureCount (int nVersion)
{
return m_header [(nVersion < 0) ? Version () : nVersion].nTextures;
}

//------------------------------------------------------------------------

int CTextureManager::TexIdFromIndex (uint nIndex, int nVersion)
{
	int nVersionResolved = (nVersion < 0) ? Version () : nVersion;

// Search the index for this texture ID
for (uint i = 0; i < m_nTextures [nVersionResolved]; i++) {
	if (m_index [nVersionResolved][i] - 1 == nIndex) {
		return (int) i;
		}
	}

return -1;
}

//------------------------------------------------------------------------

bool CTextureManager::HasCustomTextures (void) 
{
for (int i = 0; i < GlobalTextureCount (); i++)
	if (TextureByIndex ((uint)i)->IsCustom ())
		return true;
return false;
}

//------------------------------------------------------------------------

int CTextureManager::CountCustomTextures (void) 
{
	int count = 0;

for (int i = 0; i < GlobalTextureCount (); i++)
	if (TextureByIndex ((uint) i)->IsCustom ())
		count++;
return count;
}

//------------------------------------------------------------------------

int CTextureManager::CountModifiedTextures (void)
{
	int count = 0;

for (int i = 0; i < GlobalTextureCount (); i++)
	if (m_bModified [Version ()] [i])
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

int CTextureManager::LoadAnimationClips (CAnimationClipList& animations, CFileManager& fp, CAnimationClipLoader* loader)
{
if (loader) {
	int nAnimations = loader->LoadAnimationClips (animations, fp);
	delete loader;
	return nAnimations;
	}
return -1;
}


//------------------------------------------------------------------------------

bool CTextureManager::SkipToAnimationData (CFileManager& fp)
{
if (Version ()) {
	if (fp.ReadInt32 () != 0x214d4148L) { // HAM Id
		fp.Close ();
		return false;
		}
	fp.ReadInt32 (); // HAM version
	int n = fp.ReadUInt32 (); // # of textures                   
	fp.Seek (n * sizeof (ushort), SEEK_CUR);	 
	fp.Seek (n * sizeof (tTextureEffectInfo), SEEK_CUR); 
	n = fp.ReadUInt32 (); // # of sounds     
	fp.Seek (n * sizeof (ubyte), SEEK_CUR);     
	fp.Seek (n * sizeof (ubyte), SEEK_CUR);     
	}
else {
	fp.ReadUInt32 ();                         
	int n = MAX_TEXTURES_D1;
	fp.Seek (n * sizeof (ushort), SEEK_CUR);	 
	fp.Seek (n * sizeof (tTextureEffectInfoD1), SEEK_CUR); 
	n = MAX_SOUNDS_D1;
	fp.Seek (n * sizeof (ubyte), SEEK_CUR);     
	fp.Seek (n * sizeof (ubyte), SEEK_CUR);     
	}
return true;
}

//------------------------------------------------------------------------------

bool CTextureManager::OpenAnimationFile (CFileManager& fp)
{
	char filename [256];

strcpy_s (filename, sizeof (filename), descentFolder [Version ()]);
char *slash = strrchr (filename, '\\');
if (slash)
	*(slash+1) = '\0';
else
	filename [0] = '\0';
strcat (filename, Version () ? "descent2.ham" : "descent.pig");
if (!fp.Open (filename, "rb"))
	return false;
return SkipToAnimationData (fp);
}

//------------------------------------------------------------------------------

void CTextureManager::LoadAnimationData (int nVersion)
{
	CFileManager fp;

if (!OpenAnimationFile (fp)) {
	fp.Close ();
	return;
	}

if (nVersion < 0)
	nVersion = Version ();

CAnimationLoaderFactory	alf;

for (int nType = 0; nType < 3; nType++)
	LoadAnimationClips (m_animationClips [nVersion], fp, alf.GetAnimationLoader (nVersion, nType));

m_animationIndex [nVersion].Clear ();
CSLLIterator <CAnimationClipInfo, CAnimationClipInfo> iter (m_animationClips [Version ()]);
CTexture* textures = m_textures [nVersion].Buffer ();

short nMaxTexture = -1;
for (iter.Begin (); *iter != iter.End (); iter++) {
	CAnimationClipInfo& aic = **iter;
	ubyte nFrames = (ubyte) aic.FrameCount ();
	for (ubyte i = 0; i < nFrames; i++) {
		short nTexture = aic.Frame (i);
		if (nMaxTexture < nTexture)
			nMaxTexture = nTexture;
		}	
	}

m_animationIndex [nVersion].Create (nMaxTexture + 1);
m_animationIndex [nVersion].Clear ();

for (iter.Begin (); *iter != iter.End (); iter++) {
	CAnimationClipInfo& aic = **iter;
	if ((aic.Key () > -1) && aic.FrameCount ()) {
		m_animationIndex [nVersion][aic.m_nTexture] = *iter;
		textures [aic.m_nTexture].SetFrameCount (aic.FrameCount ());
		}
	}
fp.Close ();
}

//------------------------------------------------------------------------------

CAnimationClipInfo* CTextureManager::FindAnimation (short nTexture)
{
CSLLIterator <CAnimationClipInfo, CAnimationClipInfo> iter (m_animationClips [Version ()]);
for (iter.Begin (); *iter != iter.End (); iter++)
	if (**iter == nTexture)
		return *iter;
return null;
}

//------------------------------------------------------------------------------

short CTextureManager::PrevAnimationFrame (short nTexture)
{
CAnimationClipInfo* aicP = FindAnimation (nTexture);

if (!aicP)
	return -1;
size_t i = aicP->Find (nTexture) - 1;
return short ((i < 0) ? aicP->FrameCount () - 1 : i);
}

//------------------------------------------------------------------------------

short CTextureManager::NextAnimationFrame (short nTexture)
{
CAnimationClipInfo* aicP = FindAnimation (nTexture);

if (!aicP)
	return -1;
size_t i = aicP->Find (nTexture) + 1;
return short ((i >= aicP->FrameCount ()) ? 0 : i);
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
LoadAnimationData (nVersion);
#if 0
for (int i = 0; i < m_header [nVersion].nTextures; i++) {
	if (textures [i].IsAnimated () && textures [i].FrameNum () == 0) {
		textures [i].CalculateFrameCount ();
		}
	}
#endif
fp->Close ();
delete fp;
m_bAvailable [nVersion] = true;
return true;
}

//------------------------------------------------------------------------------

CTexture* CTextureManager::OverrideTexture (uint nIndex, const CTexture* newTexture, int nVersion)
{
	int nVersionResolved = (nVersion < 0) ? Version () : nVersion;
	CTexture* texture = new CTexture ();

if (newTexture != null)
	texture->Copy (*newTexture);
else
	texture->Copy (*TextureByIndex (nIndex, nVersionResolved));
texture->SetCustom ();

// We want to retain a copy of the texture as it was at load-time - custom or stock -
// so we know what to revert to if requested
if (m_overrides [nVersionResolved][nIndex] != null && m_previous [nVersionResolved][nIndex] != null)
	delete m_overrides [nVersionResolved][nIndex];
else if (m_overrides [nVersionResolved][nIndex] != null)
	m_previous [nVersionResolved][nIndex] = m_overrides [nVersionResolved][nIndex];
else
	m_previous [nVersionResolved][nIndex] = &m_textures [nVersionResolved][nIndex];

m_overrides [nVersionResolved][nIndex] = texture;
m_bModified [nVersionResolved][nIndex] = true;
return m_overrides [nVersionResolved][nIndex];
}

//------------------------------------------------------------------------------

void CTextureManager::RevertTexture (uint nIndex, int nVersion)
{
	int nVersionResolved = (nVersion < 0) ? Version () : nVersion;

if (m_overrides [nVersionResolved][nIndex] == null)
	return;
else if (m_previous [nVersionResolved][nIndex] != null) {
	delete m_overrides [nVersionResolved][nIndex];
	m_overrides [nVersionResolved][nIndex] = null;
	if (m_previous [nVersionResolved][nIndex] == &m_textures [nVersionResolved][nIndex]) {
		// Revert operation counts as an undo, so unset modified
		m_previous [nVersionResolved][nIndex] = null;
		m_bModified [nVersionResolved][nIndex] = false;
		}
	}
else {
	m_previous [nVersionResolved][nIndex] = m_overrides [nVersionResolved][nIndex];
	m_overrides [nVersionResolved][nIndex] = null;
	m_bModified [nVersionResolved][nIndex] = true;
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

int CTextureManager::UsedCount (uint nIndex)
{
if (!textureManager.Available ())
	return 0;

int nTexture = TexIdFromIndex (nIndex);
if (nTexture < 0)
	return 0;

const CTexture *texP = Textures (nTexture);

if (texP->IsAssignableFrame ()) {
	// This is a frame of an animated texture - we need to find the base before
	// we can get the used count
	bool bFoundParent = false;
	for (int i = nTexture - 1; i > 0; i--) {
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
		if (TextureByIndex (i)->IsCustom ()) {
			if (!bUnusedOnly || !IsTextureUsed (TextureByIndex (i)->IdLevel ())) {
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
for (int i = 0; i < GlobalTextureCount (); i++)
	if (m_bModified [Version ()][i]) {
		m_previous [Version ()][i] = null;
		m_bModified [Version ()][i] = false;
		}
}

//------------------------------------------------------------------------

void CTextureManager::UndoTextureChanges (void)
{
	int nReverted = 0;

for (int i = 0; i < GlobalTextureCount (); i++)
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

for (int i = 0; i < GlobalTextureCount (); i++)
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
for (uint nIndex = 0, j = m_header [nVersionResolved].nTextures; nIndex < j; nIndex++) {
	if (m_overrides [nVersionResolved][nIndex] != null && m_overrides [nVersionResolved][nIndex]->Format () != TGA) {
		CTexture *pNewTexture = OverrideTexture (nIndex, null, nVersionResolved);
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