#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "io.h"
#include "dle-xp-res.h"
#include "global.h"
#include "palette.h"
#include "customtextures.h"
#include "dle-xp.h"

CResourceManager resourceManager;

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

byte* CResource::Load (const char* szName, const char* szCategory) 
{
Unload ();
HINSTANCE hInst = AfxGetApp ()->m_hInstance;
HRSRC hFind = FindResource (hInst, "ROBOT_HXM", "RC_DATA");
if (!hFind)
	return null;
m_handle = LoadResource (hInst, hFind);
if (!m_handle)
	return null;
byte* bufP = (byte *) LockResource (m_handle);
if (!bufP)
	return null;
m_size = SizeofResource (hInst, m_handle);
return bufP;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//eof textures.cpp