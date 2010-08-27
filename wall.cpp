// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#undef abs
#include <math.h>
#include <mmsystem.h>
#include <stdio.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "mine.h"
#include "dle-xp.h"
#include "global.h"
#include "io.h"

//--------------------------------------------------------------------------
// Mine - add wall
//
// Returns - TRUE on success
//
// Note: nClip & nTexture are used for call to DefineWall only.
//--------------------------------------------------------------------------

CWall *CMine::AddWall (short nSegment,short nSide,
								short type, ushort flags, byte keys,
								char nClip, short nTexture) 
{
GetCurrent (nSegment, nSide);

ushort nWall;
CSegment *segP = Segments (nSegment);

// if wall is an overlay, make sure there is no child
if (type < 0)
	type = (segP->Child (nSide) == -1) ? WALL_OVERLAY : WALL_OPEN;
if (type == WALL_OVERLAY) {
	if (segP->Child (nSide) != -1) {
		ErrorMsg ("Switches can only be put on solid sides.");
		return NULL;
		}
	}
else {
	// otherwise make sure there is a child
	if (segP->Child (nSide) < 0) {
		ErrorMsg ("This side must be attached to an other cube before a wall can be added.");
		return NULL;
		}
	}

if (segP->m_sides [nSide].m_info.nWall < GameInfo ().walls.count) {
	ErrorMsg ("There is already a wall on this side.");
	return NULL;
	}

if ((nWall = GameInfo ().walls.count) >= MAX_WALLS) {
	ErrorMsg ("Maximum number of Walls () reached");
	return NULL;
	}

// link wall to segment/side
theApp.SetModified (TRUE);
theApp.LockUndo ();
segP->m_sides [nSide].m_info.nWall = nWall;
DefineWall (nSegment, nSide, nWall, (byte) type, nClip, nTexture, false);
Walls (nWall)->m_info.flags = flags;
Walls (nWall)->m_info.keys = keys;
// update number of Walls () in mine
GameInfo ().walls.count++;
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return Walls (nWall);
}

//--------------------------------------------------------------------------

bool CMine::GetOppositeWall (short& nOppWall, short nSegment, short nSide)
{
	short nOppSeg, nOppSide;

if (!GetOppositeSide (nOppSeg, nOppSide, nSegment, nSide))
	return false;
nOppWall = Segments (nOppSeg)->m_sides [nOppSide].m_info.nWall;
return true;
}

//--------------------------------------------------------------------------
// DefineWall()
//
// Note: if nClip == -1, then it is overriden for blastable and auto door
//       if nTexture == -1, then it is overriden for illusion Walls ()
//       if nClip == -2, then texture is applied to nOvlTex instead
//--------------------------------------------------------------------------

void CMine::DefineWall (short nSegment, short nSide, ushort nWall,
								byte type, char nClip, short nTexture,
								bool bRedefine) 
{
GetCurrent (nSegment, nSide);

	int i;
	CSegment *segP = Segments (nSegment);
	CSide *sideP = segP->m_sides + nSide;
	CWall *wallP = Walls (nWall);

theApp.SetModified (TRUE);
theApp.LockUndo ();
// define new wallP
wallP->m_nSegment = nSegment;
wallP->m_nSide = nSide;
wallP->m_info.type = type;
if (!bRedefine) {
	wallP->m_info.nTrigger = NO_TRIGGER;
	wallP->m_info.linkedWall = -1; //GetOppositeWall (nOppWall, nSegment, nSide) ? nOppWall : -1;
	}
switch (type) {
	case WALL_BLASTABLE:
		wallP->m_info.nClip = (nClip == -1) ?  6 : nClip;
		wallP->m_info.hps = WALL_HPS;
		// define door textures based on clip number
		SetWallTextures (nWall, nTexture);
		break;

	case WALL_DOOR:
		wallP->m_info.nClip = (nClip == -1) ? 1 : nClip;
		wallP->m_info.hps = 0;
		// define door textures based on clip number
		SetWallTextures (nWall, nTexture);
		break;

	case WALL_CLOSED:
	case WALL_ILLUSION:
		wallP->m_info.nClip = -1;
		wallP->m_info.hps = 0;
		// define texture to be energy
		if (nTexture == -1)
			SetTexture (nSegment, nSide, (IsD1File ()) ? 328 : 353, 0); // energy
		else if (nClip == -2)
			SetTexture (nSegment, nSide, 0, nTexture);
		else
			SetTexture (nSegment, nSide, nTexture, 0);
		break;

	case WALL_OVERLAY: // d2 only
		wallP->m_info.nClip = -1;
		wallP->m_info.hps = 0;
		// define box01a
		SetTexture (nSegment, nSide, -1, 414);
		break;

	case WALL_CLOAKED:
		wallP->m_info.cloakValue = 17;
		break;

	case WALL_TRANSPARENT:
		wallP->m_info.cloakValue = 0;
		break;

	default:
		wallP->m_info.nClip = -1;
		wallP->m_info.hps = 0;
		SetTexture (nSegment, nSide, nTexture, 0);
		break;
	}
wallP->m_info.flags = 0;
wallP->m_info.state = 0;
wallP->m_info.keys = 0;
//  wallP->pad = 0;
wallP->m_info.controllingTrigger = 0;

// set uvls of new texture
uint	scale = (uint) theMine->Textures (m_fileType, nTexture)->Scale (nTexture);
for (i = 0;i<4;i++) {
	sideP->m_info.uvls [i].u = default_uvls [i].u / scale;
	sideP->m_info.uvls [i].v = default_uvls [i].v / scale;
	sideP->m_info.uvls [i].l = default_uvls [i].l;
	}
Segments (nSegment)->SetUV (nSide, 0, 0);
theApp.UnlockUndo ();
}

//--------------------------------------------------------------------------
// SetWallTextures()
//
// 1/27/97 - added wall01 and door08
//--------------------------------------------------------------------------

void CMine::SetWallTextures (ushort nWall, short nTexture) 
{
static short wall_texture [N_WALL_TEXTURES][2] = {
	{371,0},{0,376},{0,0},  {0,387},{0,399},{413,0},{419,0},{0,424},  {0,0},{436,0},
	{0,444},{0,459},{0,472},{486,0},{492,0},{500,0},{508,0},{515,0},{521,0},{529,0},
	{536,0},{543,0},{0,550},{563,0},{570,0},{577,0}
	};
static short d2_wall_texture [D2_N_WALL_TEXTURES][2] = {
	{435,0},{0,440},{0,0},{0,451},{0,463},{477,0},{483,0},{0,488},{0,0},  {500,0},
	{0,508},{0,523},{0,536},{550,0},{556,0},{564,0},{572,0},{579,0},{585,0},{593,0},
	{600,0},{608,0},{0,615},{628,0},{635,0},{642,0},{0,649},{664,0},{0,672},{0,687},
	{0,702},{717,0},{725,0},{731,0},{738,0},{745,0},{754,0},{763,0},{772,0},{780,0},
	{0,790},{806,0},{817,0},{827,0},{838,0},{849,0},{858,0},{863,0},{0,871},{0,886},
	{901,0}
	};

CWall *wallP = Walls (nWall);
CSide *sideP = Segments (wallP->m_nSegment)->m_sides + (short) wallP->m_nSide;
char nClip = wallP->m_info.nClip;

theApp.SetModified (TRUE);
theApp.LockUndo ();
if ((wallP->m_info.type == WALL_DOOR) || (wallP->m_info.type == WALL_BLASTABLE))
	if (IsD1File ()) {
		sideP->m_info.nBaseTex = wall_texture [nClip][0];
		sideP->m_info.nOvlTex = wall_texture [nClip][1];
		} 
	else {
		sideP->m_info.nBaseTex = d2_wall_texture [nClip][0];
		sideP->m_info.nOvlTex = d2_wall_texture [nClip][1];
		}
else if (nTexture >= 0) {
	sideP->m_info.nBaseTex = nTexture;
	sideP->m_info.nOvlTex = 0;
	}
else
	return;
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
}

//--------------------------------------------------------------------------
// Mine - delete wall
//--------------------------------------------------------------------------

void CMine::DeleteWall (ushort nWall) 
{
	short nTrigger;
	short nSegment, nSide, nOppSeg, nOppSide;
	CSegment *segP;
	CSide *sideP;

if (nWall < 0)
	nWall = CurrSide ()->m_info.nWall;
if (nWall >= GameInfo ().walls.count)
	return;
// if trigger exists, remove it as well
nTrigger = Walls (nWall)->m_info.nTrigger;
theApp.SetModified (TRUE);
theApp.LockUndo ();
if ((nTrigger > -1) && (nTrigger < GameInfo ().triggers.count))
	DeleteTrigger (nTrigger); 
// remove references to the deleted wall
if (GetOppositeSide (nOppSeg, nOppSide, Walls (nWall)->m_nSegment, Walls (nWall)->m_nSide)) {
	short nOppWall = Segments (nOppSeg)->m_sides [nOppSide].m_info.nWall;
	if ((nOppWall >= 0) && (nOppWall < GameInfo ().walls.count))
		Walls (nOppWall)->m_info.linkedWall = -1;
	}
// update all Segments () that point to Walls () higher than deleted one
// and unlink all Segments () that point to deleted wall
for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++)
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++)
		if (sideP->m_info.nWall >= GameInfo ().walls.count)
			sideP->m_info.nWall = NO_WALL;
		else if (sideP->m_info.nWall > nWall)
			sideP->m_info.nWall--;
		else if (sideP->m_info.nWall == nWall) {
			sideP->m_info.nWall = NO_WALL;
			DeleteTriggerTargets (nSegment, nSide); //delete this wall from all Triggers () that target it
			}
// move remaining Walls () in place of deleted wall
// for (i = nWall; i < GameInfo ().walls.count - 1; i++)
if (nWall < --GameInfo ().walls.count)
	memcpy (Walls (nWall), Walls (nWall + 1), (GameInfo ().walls.count - nWall) * sizeof (CWall));
// update number of Walls () in mine
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
AutoLinkExitToReactor();
}

                        /*--------------------------*/

CWall *CMine::FindWall (short nSegment, short nSide)
{
GetCurrent (nSegment, nSide);
CWall *wallP;
int nWall;

for (wallP = Walls (0), nWall = 0; nWall < GameInfo ().walls.count; nWall++, wallP++)
	if ((wallP->m_nSegment == nSegment) && (wallP->m_nSide == nSide))
		return wallP;
return NULL;
}

                        /*--------------------------*/

int CMine::FindClip (CWall *wallP, short nTexture)
{
	HINSTANCE hInst = AfxGetApp ()->m_hInstance;
	char szName [80], *ps;

LoadString (hInst, texture_resource + nTexture, szName, sizeof (szName));
if (!strcmp (szName, "wall01 - anim"))
	return wallP->m_info.nClip = 0;
if (ps = strstr (szName, "door")) {
	int i, nDoor = atol (ps + 4);
	for (i = 1; i < D2_NUM_OF_CLIPS; i++)
		if (nDoor == doorClipTable [i]) {
			wallP->m_info.nClip = clipList [i];
			theApp.SetModified (TRUE);
			theApp.MineView ()->Refresh ();
			return i;
			}
	}
return -1;
}

                        /*--------------------------*/

CWall *CMine::GetWall (short nSegment, short nSide)
{
GetCurrent (nSegment, nSide);
ushort nWall = Segments (nSegment)->m_sides [nSide].m_info.nWall;
return (nWall < GameInfo ().walls.count) ? Walls (nWall) : NULL;
}

                        /*--------------------------*/

bool CMine::WallClipFromTexture (short nSegment, short nSide)
{
CWall *wallP = FindWall (nSegment, nSide);

if (!wallP || ((wallP->m_info.type != WALL_DOOR) && (wallP->m_info.type != WALL_BLASTABLE)))
	return true;

CSide *sideP = Segments (0) [nSegment].m_sides + nSide;

if (FindClip (wallP, sideP->m_info.nOvlTex) >= 0)
	return true;
if (FindClip (wallP, sideP->m_info.nBaseTex) >= 0)
	return true;
return false;
}

//------------------------------------------------------------------------
// CheckForDoor()
//------------------------------------------------------------------------

void CMine::CheckForDoor (short nSegment, short nSide) 
{
GetCurrent (nSegment, nSide);
// put up a warning if changing a door's texture
ushort nWall = Segments (nSegment)->m_sides [nSide].m_info.nWall;

if (!bExpertMode &&
    (nWall < GameInfo ().walls.count) &&
	 ((Walls (nWall)->m_info.type == WALL_BLASTABLE) || (Walls (nWall)->m_info.type == WALL_DOOR)))
		ErrorMsg ("Changing the texture of a door only affects\n"
					"how the door will look before it is opened.\n"
					"You can use this trick to hide a door\n"
					"until it is used for the first time.\n\n"
					"Hint: To change the door animation,\n"
					"select \"Wall edit...\" from the Tools\n"
					"menu and change the clip number.");
}

// ------------------------------------------------------------------------

int CWall::Read (FILE* fp, int version, bool bFlag)
{
m_nSegment = ReadInt32 (fp);
m_nSide = ReadInt32 (fp); 
m_info.hps = ReadFix (fp);
m_info.linkedWall = ReadInt32 (fp);
m_info.type = byte (ReadInt8 (fp));
m_info.flags = ushort ((version < 37) ? ReadInt8 (fp) : ReadInt16 (fp));         
m_info.state = byte (ReadInt8 (fp));         
m_info.nTrigger = byte (ReadInt8 (fp));       
m_info.nClip = byte (ReadInt8 (fp));      
m_info.keys = byte (ReadInt8 (fp));          
m_info.controllingTrigger = ReadInt8 (fp);
m_info.cloakValue = ReadInt8 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CWall::Write (FILE* fp, int version, bool bFlag)
{
WriteInt32 (m_nSegment, fp);
WriteInt32 (m_nSide, fp); 
WriteFix (m_info.hps, fp);
WriteInt32 (m_info.linkedWall, fp);
WriteInt8 (m_info.type, fp);
if (version < 37) 
	WriteInt8 (char (m_info.flags), fp);
else
	WriteInt16 (m_info.flags, fp);         
WriteInt8 (m_info.state, fp);         
WriteInt8 (m_info.nTrigger, fp);       
WriteInt8 (m_info.nClip, fp);      
WriteInt8 (m_info.keys, fp);          
WriteInt8 (m_info.controllingTrigger, fp);
WriteInt8 (m_info.cloakValue, fp);
}


// ------------------------------------------------------------------------

int CActiveDoor::Read (FILE *fp, int version, bool bFlag)
{
m_info.n_parts = ReadInt32 (fp);
m_info.nFrontWall [0] = ReadInt16 (fp);
m_info.nFrontWall [1] = ReadInt16 (fp);
m_info.nBackWall [0] = ReadInt16 (fp); 
m_info.nBackWall [1] = ReadInt16 (fp); 
m_info.time = ReadInt32 (fp);		  
return 1;
}

// ------------------------------------------------------------------------

void CActiveDoor::Write (FILE *fp, int version, bool bFlag)
{
WriteInt32 (m_info.n_parts, fp);
WriteInt16 (m_info.nFrontWall[0], fp);
WriteInt16 (m_info.nFrontWall[1], fp);
WriteInt16 (m_info.nBackWall[0], fp); 
WriteInt16 (m_info.nBackWall[1], fp); 
WriteInt32 (m_info.time, fp);		  
}

// ------------------------------------------------------------------------


//eof wall.cpp