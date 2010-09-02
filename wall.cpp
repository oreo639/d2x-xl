// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>

#include "define.h"
#include "types.h"
#include "cfile.h"
#include "wall.h"
#include "TextureManager.h"
#include "UndoManager.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Note: if nClip == -1, then it is overriden for blastable and auto door
//       if nTexture == -1, then it is overriden for illusion Walls ()
//       if nClip == -2, then texture is applied to nOvlTex instead
//------------------------------------------------------------------------------

void CWall::Setup (short nSegment, short nSide, ushort nWall, byte type, char nClip, short nTexture, bool bRedefine) 
{
undoManager.SetModified (TRUE);
undoManager.Lock ();
// define new wallP
m_nSegment = nSegment;
m_nSide = nSide;
m_info.type = type;
if (!bRedefine) {
	m_info.nTrigger = NO_TRIGGER;
	m_info.linkedWall = -1; //GetOppositeWall (nOppWall, nSegment, nSide) ? nOppWall : -1;
	}
switch (type) {
	case WALL_BLASTABLE:
		m_info.nClip = (nClip == -1) ?  6 : nClip;
		m_info.hps = WALL_HPS;
		// define door textures based on clip number
		SetTextures (nTexture);
		break;

	case WALL_DOOR:
		m_info.nClip = (nClip == -1) ? 1 : nClip;
		m_info.hps = 0;
		// define door textures based on clip number
		SetTextures (nTexture);
		break;

	case WALL_CLOSED:
	case WALL_ILLUSION:
		m_info.nClip = -1;
		m_info.hps = 0;
		// define texture to be energy
		if (nTexture == -1)
			segmentManager.SetTextures (nSegment, nSide, theMine->IsD1File () ? 328 : 353, 0); // energy
		else if (nClip == -2)
			segmentManager.SetTextures (nSegment, nSide, 0, nTexture);
		else
			segmentManager.SetTextures (nSegment, nSide, nTexture, 0);
		break;

	case WALL_OVERLAY: // d2 only
		m_info.nClip = -1;
		m_info.hps = 0;
		// define box01a
		segmentManager.SetTextures (nSegment, nSide, -1, 414);
		break;

	case WALL_CLOAKED:
		m_info.cloakValue = 17;
		break;

	case WALL_TRANSPARENT:
		m_info.cloakValue = 0;
		break;

	default:
		m_info.nClip = -1;
		m_info.hps = 0;
		segmentManager.SetTextures (nSegment, nSide, nTexture, 0);
		break;
	}
m_info.flags = 0;
m_info.state = 0;
m_info.keys = 0;
m_info.controllingTrigger = 0;

// set uvls of new texture
segmentManager.GetSegment (nSegment)->SetUV (nSide, 0, 0);
undoManager.Unlock ();
}

//--------------------------------------------------------------------------
// SetWallTextures()
//--------------------------------------------------------------------------

void CWall::SetTextures (short nTexture) 
{
	static short wallTexturesD1 [N_WALL_TEXTURES_D1][2] = {
		{371,0},{0,376},{0,0},  {0,387},{0,399},{413,0},{419,0},{0,424},  {0,0},{436,0},
		{0,444},{0,459},{0,472},{486,0},{492,0},{500,0},{508,0},{515,0},{521,0},{529,0},
		{536,0},{543,0},{0,550},{563,0},{570,0},{577,0}
		};

	static short wallTexturesD2 [N_WALL_TEXTURES_D2][2] = {
		{435,0},{0,440},{0,0},{0,451},{0,463},{477,0},{483,0},{0,488},{0,0},  {500,0},
		{0,508},{0,523},{0,536},{550,0},{556,0},{564,0},{572,0},{579,0},{585,0},{593,0},
		{600,0},{608,0},{0,615},{628,0},{635,0},{642,0},{0,649},{664,0},{0,672},{0,687},
		{0,702},{717,0},{725,0},{731,0},{738,0},{745,0},{754,0},{763,0},{772,0},{780,0},
		{0,790},{806,0},{817,0},{827,0},{838,0},{849,0},{858,0},{863,0},{0,871},{0,886},
		{901,0}
		};

CSide *sideP = GetSide ();
char nClip = m_info.nClip;

undoManager.SetModified (TRUE);
undoManager.Lock ();
if (IsDoor ()) {
	if (theMine->IsD1File ())
		sideP->SetTextures (wallTexturesD1 [nClip][0], wallTexturesD1 [nClip][1]);
	else
		sideP->SetTextures (wallTexturesD2 [nClip][0], wallTexturesD2 [nClip][1]);
		}
else if (nTexture >= 0) {
	sideP->SetTextures (nTexture, 0);
	}
else
	return;
undoManager.Unlock ();
//DLE.MineView ()->Refresh ();
}

// ------------------------------------------------------------------------

void CWall::Read (CFileManager& fp, int version, bool bFlag)
{
m_nSegment = fp.ReadInt32 ();
m_nSide = fp.ReadInt32 (); 
m_info.hps = fp.ReadInt32 ();
m_info.linkedWall = fp.ReadInt32 ();
m_info.type = fp.ReadByte ();
m_info.flags = ushort ((version < 37) ? fp.ReadSByte () : fp.ReadInt16 ());         
m_info.state = fp.ReadByte ();         
m_info.nTrigger = fp.ReadByte ();       
m_info.nClip = fp.ReadByte ();      
m_info.keys = fp.ReadByte ();          
m_info.controllingTrigger = fp.ReadSByte ();
m_info.cloakValue = fp.ReadSByte ();
}

// ------------------------------------------------------------------------

void CWall::Write (CFileManager& fp, int version, bool bFlag)
{
fp.WriteInt32 ((int) m_nSegment);
fp.WriteInt32 ((int) m_nSide); 
fp.Write (m_info.hps);
fp.Write (m_info.linkedWall);
fp.Write (m_info.type);
if (version < 37) 
	fp.WriteSByte ((sbyte) m_info.flags);
else
	fp.Write (m_info.flags);         
fp.Write (m_info.state);         
fp.Write (m_info.nTrigger);       
fp.Write (m_info.nClip);      
fp.Write (m_info.keys);          
fp.Write (m_info.controllingTrigger);
fp.Write (m_info.cloakValue);
}

// ------------------------------------------------------------------------

CSide* CWall::GetSide (void)
{
return segmentManager.GetSide (m_nSegment, m_nSide);
}

// ------------------------------------------------------------------------

CTrigger* CWall::GetTrigger (void)
{
return triggerManager.GetTrigger (m_info.nTrigger);
}

// ------------------------------------------------------------------------

bool CWall::IsDoor (void)
{
return (m_info.type == WALL_BLASTABLE) || (m_info.type == WALL_DOOR);
}

//------------------------------------------------------------------------------

int CWall::SetClip (short nTexture)
{
	char *ps, *pszName = textureManager.Name (nTexture);

if (!strcmp (pszName, "wall01 - anim"))
	return m_info.nClip = 0;
if (ps = strstr (pszName, "door")) {
	int nDoor = atol (ps + 4);
	for (int i = 1; i < NUM_OF_CLIPS_D2; i++)
		if (nDoor == doorClipTable [i]) {
			m_info.nClip = animClipTable [i];
			undoManager.SetModified (true);
			//DLE.MineView ()->Refresh ();
			return i;
			}
	}
return -1;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void CActiveDoor::Read (CFileManager& fp, int version, bool bFlag)
{
m_info.nParts = fp.ReadInt32 ();
m_info.nFrontWall [0] = fp.ReadInt16 ();
m_info.nFrontWall [1] = fp.ReadInt16 ();
m_info.nBackWall [0] = fp.ReadInt16 (); 
m_info.nBackWall [1] = fp.ReadInt16 (); 
m_info.time = fp.ReadInt32 ();		  
}

// ------------------------------------------------------------------------

void CActiveDoor::Write (CFileManager& fp, int version, bool bFlag)
{
fp.Write (m_info.nParts);
fp.Write (m_info.nFrontWall [0]);
fp.Write (m_info.nFrontWall [1]);
fp.Write (m_info.nBackWall [0]); 
fp.Write (m_info.nBackWall [1]); 
fp.Write (m_info.time);		  
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

//eof wall.cpp