#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "FileManager.h"
#include "dle-xp-res.h"
#include "global.h"
#include "PaletteManager.h"
#include "dle-xp.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

byte* CResource::Load (const char* szName, const char* szCategory) 
{
Unload ();
HINSTANCE hInst = AfxGetInstanceHandle (); //AfxGetApp ()->m_hInstance;
HRSRC hRes = FindResource (hInst, szName, szCategory);
if (!hRes)
	return null;
m_handle = LoadResource (hInst, hRes);
if (!m_handle)
	return null;
m_data = (byte *) LockResource (m_handle);
if (!m_data)
	return null;
m_size = SizeofResource (hInst, hRes);
return m_data;
}

//------------------------------------------------------------------------

char* CStringResource::Load (int nResource)
{
	size_t l = strlen (m_value);

LoadString (m_hInst, nResource, m_value + l, sizeof (m_value) - l);
return m_value;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//eof textures.cpp