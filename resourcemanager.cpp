#include <assert.h>

#include "stdafx.h"
#include "textures.h"
#include "cfile.h"
#include "dle-xp-res.h"
#include "global.h"
#include "palette.h"
#include "customtextures.h"
#include "dle-xp.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

byte* CResource::Load (const char* szName, const char* szCategory) 
{
Unload ();
HINSTANCE hInst = AfxGetInstanceHandle (); //AfxGetApp ()->m_hInstance;
HRSRC hRes = FindResource (hInst, szName, "RC_DATA");
if (!hRes)
	return null;
m_handle = LoadResource (hInst, hRes);
if (!m_handle)
	return null;
byte* bufP = (byte *) LockResource (m_handle);
if (!bufP)
	return null;
m_size = SizeofResource (hInst, hRes);
return bufP;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

//eof textures.cpp