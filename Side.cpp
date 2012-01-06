
#include "types.h"
#include "mine.h"
#include "dle-xp-res.h"
#include "dle-xp.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CSide::Read (CFileManager* fp, bool bTextured)
{
if (bTextured) {
	m_info.nBaseTex = fp->ReadInt16 ();
	if (m_info.nBaseTex & 0x8000) {
		m_info.nOvlTex = fp->ReadInt16 ();
		if ((m_info.nOvlTex & 0x1FFF) == 0)
			m_info.nOvlTex = 0;
		}
	else
		m_info.nOvlTex = 0;
	m_info.nBaseTex &= 0x1FFF;
	for (int i = 0; i < 4; i++)
		m_info.uvls [i].Read (fp);
	}
else {
	m_info.nBaseTex = 0;
	m_info.nOvlTex = 0;
	for (int i = 0; i < 4; i++)
		m_info.uvls [i].Clear ();
	}
}

// -----------------------------------------------------------------------------

void CSide::Write (CFileManager* fp)
{
if (m_info.nOvlTex == 0)
	fp->Write (m_info.nBaseTex);
else {
	fp->WriteInt16 (m_info.nBaseTex | 0x8000);
	fp->Write (m_info.nOvlTex);
	}
for (int i = 0; i < 4; i++)
	m_info.uvls [i].Write (fp);
}

// -----------------------------------------------------------------------------

void CSide::Setup (void)
{
m_info.nWall = NO_WALL; 
m_info.nBaseTex =
m_info.nOvlTex = 0; 
for (int i = 0; i < 4; i++)
	m_info.uvls [i].l = (ushort) DEFAULT_LIGHTING; 
}

// ----------------------------------------------------------------------------- 

void CSide::LoadTextures (void)
{
textureManager.Load (m_info.nBaseTex, m_info.nOvlTex);
}

// -----------------------------------------------------------------------------

bool CSide::SetTextures (short nBaseTex, short nOvlTex)
{
	bool bChange = false;

if (nOvlTex == nBaseTex)
   nOvlTex = 0; 
if ((nBaseTex >= 0) && (nBaseTex != m_info.nBaseTex)) {
	m_info.nBaseTex = nBaseTex; 
	if (nBaseTex == (m_info.nOvlTex & 0x3fff)) {
		m_info.nOvlTex = 0; 
		}
	bChange = true; 
	}
if (nOvlTex >= 0) {
	if (nOvlTex == m_info.nBaseTex)
		m_info.nOvlTex = 0; 
	else if (nOvlTex) {
		m_info.nOvlTex &= ~(0x3fff);	//preserve light settings
		m_info.nOvlTex |= nOvlTex; 
		}
	else
		m_info.nOvlTex = 0; 
	bChange = true; 
	}
if (bChange)
	LoadTextures ();
return bChange;
}

// -----------------------------------------------------------------------------

void CSide::GetTextures (short &nBaseTex, short &nOvlTex) _const_
{
nBaseTex = m_info.nBaseTex;
nOvlTex = m_info.nOvlTex & 0x1FFF;
}

// -----------------------------------------------------------------------------

void CSide::InitUVL (short nTexture)
{
uint scale = (uint) textureManager.Textures (DLE.FileType (), nTexture)->Scale (nTexture);
for (int i = 0; i < 4; i++) {
	m_info.uvls [i].u = defaultUVLs [i].u / scale;
	m_info.uvls [i].v = defaultUVLs [i].v / scale;
	m_info.uvls [i].l = defaultUVLs [i].l;
	}
}

// -----------------------------------------------------------------------------

CSegment _const_ * CSide::Child (void) _const_
{ 
return ((short) m_info.nChild < 0) ? null: segmentManager.Segment (m_info.nChild); 
}

// -----------------------------------------------------------------------------

CWall _const_ * CSide::Wall (void) _const_
{ 
return wallManager.Wall (m_info.nWall); 
}

// -----------------------------------------------------------------------------

CTrigger _const_ * CSide::Trigger (void) _const_
{ 
CWall* wallP = wallManager.Wall (m_info.nWall); 
return (wallP == null) ? null : wallP->Trigger ();
}

// -----------------------------------------------------------------------------

bool CSide::IsVisible (void)
{
return ((short) m_info.nChild == -1) || wallManager.IsVisible (m_info.nWall);
}

// -----------------------------------------------------------------------------

bool CSide::UpdateChild (short nOldChild, short nNewChild)
{
if (m_info.nChild != nOldChild)
	return false;
m_info.nChild = nNewChild;
return true;
}

// -----------------------------------------------------------------------------

void CSide::Reset (void)
{
m_info.nBaseTex = 0; 
m_info.nOvlTex = 0; 
m_info.nWall = NO_WALL;
CUVL* uvls = m_info.uvls;
double scale = textureManager.Textures (DLE.FileType (), m_info.nBaseTex)->Scale (m_info.nBaseTex);
for (int i = 0; i < 4; i++, uvls++) {
	uvls->u = (short) (defaultUVLs [i].u / scale); 
	uvls->v = (short) (defaultUVLs [i].v / scale); 
	uvls->l = (ushort) DEFAULT_LIGHTING; 
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//eof side.cpp