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

CWall *CMine::AddWall (INT16 nSegment,INT16 nSide,
								INT16 type, UINT16 flags, UINT8 keys,
								INT8 nClip, INT16 nTexture) 
{
GetCurrent (nSegment, nSide);

UINT16 nWall;
CSegment *segP = Segments (nSegment);

// if wall is an overlay, make sure there is no child
if (type < 0)
	type = (segP->m_info.children [nSide] == -1) ? WALL_OVERLAY : WALL_OPEN;
if (type == WALL_OVERLAY) {
	if (segP->m_info.children [nSide] != -1) {
		ErrorMsg ("Switches can only be put on solid sides.");
		return NULL;
		}
	}
else {
	// otherwise make sure there is a child
	if (segP->m_info.children [nSide] < 0) {
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
DefineWall (nSegment, nSide, nWall, (UINT8) type, nClip, nTexture, false);
Walls (nWall)->flags = flags;
Walls (nWall)->keys = keys;
// update number of Walls () in mine
GameInfo ().walls.count++;
theApp.UnlockUndo ();
theApp.MineView ()->Refresh ();
return Walls (nWall);
}

//--------------------------------------------------------------------------

bool CMine::GetOppositeWall (INT16& nOppWall, INT16 nSegment, INT16 nSide)
{
	INT16 nOppSeg, nOppSide;

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

void CMine::DefineWall (INT16 nSegment, INT16 nSide, UINT16 nWall,
								UINT8 type, INT8 nClip, INT16 nTexture,
								bool bRedefine) 
{
GetCurrent (nSegment, nSide);

	INT32 i;
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
		wallP->m_info.cloak_value = 17;
		break;

	case WALL_TRANSPARENT:
		wallP->m_info.cloak_value = 0;
		break;

	default:
		wallP->m_info.nClip = -1;
		wallP->m_info.hps = 0;
		SetTexture (nSegment, nSide, nTexture, 0);
		break;
	}
wallP->flags = 0;
wallP->state = 0;
wallP->keys = 0;
//  wallP->pad = 0;
wallP->controlling_trigger = 0;

// set uvls of new texture
UINT32	scale = (UINT32) theMine->Textures (m_fileType, nTexture)->Scale (nTexture);
for (i = 0;i<4;i++) {
	sideP->m_info.uvls [i].u = default_uvls [i].u / scale;
	sideP->m_info.uvls [i].v = default_uvls [i].v / scale;
	sideP->m_info.uvls [i].l = default_uvls [i].l;
	}
SetUV (nSegment, nSide, 0, 0, 0);
theApp.UnlockUndo ();
}

//--------------------------------------------------------------------------
// SetWallTextures()
//
// 1/27/97 - added wall01 and door08
//--------------------------------------------------------------------------

void CMine::SetWallTextures (UINT16 nWall, INT16 nTexture) 
{
static INT16 wall_texture [N_WALL_TEXTURES] [2] = {
	{371,0},{0,376},{0,0},  {0,387},{0,399},{413,0},{419,0},{0,424},  {0,0},{436,0},
	{0,444},{0,459},{0,472},{486,0},{492,0},{500,0},{508,0},{515,0},{521,0},{529,0},
	{536,0},{543,0},{0,550},{563,0},{570,0},{577,0}
	};
static INT16 d2_wall_texture [D2_N_WALL_TEXTURES] [2] = {
	{435,0},{0,440},{0,0},{0,451},{0,463},{477,0},{483,0},{0,488},{0,0},  {500,0},
	{0,508},{0,523},{0,536},{550,0},{556,0},{564,0},{572,0},{579,0},{585,0},{593,0},
	{600,0},{608,0},{0,615},{628,0},{635,0},{642,0},{0,649},{664,0},{0,672},{0,687},
	{0,702},{717,0},{725,0},{731,0},{738,0},{745,0},{754,0},{763,0},{772,0},{780,0},
	{0,790},{806,0},{817,0},{827,0},{838,0},{849,0},{858,0},{863,0},{0,871},{0,886},
	{901,0}
	};

CWall *wallP = Walls (nWall);
CSide *sideP = Segments (wallP->m_nSegment)->m_sides + (INT16) wallP->m_nSide;
INT8 nClip = wallP->nClip;

theApp.SetModified (TRUE);
theApp.LockUndo ();
if ((wallP->type == WALL_DOOR) || (wallP->type == WALL_BLASTABLE))
	if (IsD1File ()) {
		sideP->m_info.nBaseTex = wall_texture [nClip] [0];
		sideP->m_info.nOvlTex = wall_texture [nClip] [1];
		} 
	else {
		sideP->m_info.nBaseTex = d2_wall_texture [nClip] [0];
		sideP->m_info.nOvlTex = d2_wall_texture [nClip] [1];
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

void CMine::DeleteWall (UINT16 nWall) 
{
	INT16 nTrigger;
	INT16 nSegment, nSide, nOppSeg, nOppSide;
	CSegment *segP;
	CSide *sideP;

if (nWall < 0)
	nWall = CurrSide ()->nWall;
if (nWall >= GameInfo ().walls.count)
	return;
// if trigger exists, remove it as well
nTrigger = Walls (nWall)->nTrigger;
theApp.SetModified (TRUE);
theApp.LockUndo ();
if ((nTrigger > -1) && (nTrigger < GameInfo ().triggers.count))
	DeleteTrigger (nTrigger); 
// remove references to the deleted wall
if (GetOppositeSide (nOppSeg, nOppSide, Walls (nWall)->m_nSegment, Walls (nWall)->m_nSide)) {
	INT16 nOppWall = Segments (nOppSeg)->m_sides [nOppSide].m_info.nWall;
	if ((nOppWall >= 0) && (nOppWall < GameInfo ().walls.count))
		Walls (nOppWall)->linkedWall = -1;
	}
// update all Segments () that point to Walls () higher than deleted one
// and unlink all Segments () that point to deleted wall
for (nSegment = 0, segP = Segments (0); nSegment < SegCount (); nSegment++, segP++)
	for (nSide = 0, sideP = segP->m_sides; nSide < 6; nSide++, sideP++)
		if (sideP->nWall >= GameInfo ().walls.count)
			sideP->nWall = NO_WALL;
		else if (sideP->nWall > nWall)
			sideP->nWall--;
		else if (sideP->nWall == nWall) {
			sideP->nWall = NO_WALL;
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

CWall *CMine::FindWall (INT16 nSegment, INT16 nSide)
{
GetCurrent (nSegment, nSide);
CWall *wallP;
INT32 nWall;

for (wallP = Walls (0), nWall = 0; nWall < GameInfo ().walls.count; nWall++, wallP++)
	if ((wallP->m_nSegment == nSegment) && (wallP->m_nSide == nSide))
		return wallP;
return NULL;
}

                        /*--------------------------*/

INT32 CMine::FindClip (CWall *wallP, INT16 nTexture)
{
	HINSTANCE hInst = AfxGetApp ()->m_hInstance;
	char szName [80], *ps;

LoadString (hInst, texture_resource + nTexture, szName, sizeof (szName));
if (!strcmp (szName, "wall01 - anim"))
	return wallP->nClip = 0;
if (ps = strstr (szName, "door")) {
	INT32 i, nDoor = atol (ps + 4);
	for (i = 1; i < D2_NUM_OF_CLIPS; i++)
		if (nDoor == clip_door_number [i]) {
			wallP->nClip = clipList [i];
			theApp.SetModified (TRUE);
			theApp.MineView ()->Refresh ();
			return i;
			}
	}
return -1;
}

                        /*--------------------------*/

CWall *CMine::GetWall (INT16 nSegment, INT16 nSide)
{
GetCurrent (nSegment, nSide);
UINT16 nWall = Segments (nSegment)->m_sides [nSide].m_info.nWall;
return (nWall < GameInfo ().walls.count) ? Walls (nWall) : NULL;
}

                        /*--------------------------*/

bool CMine::WallClipFromTexture (INT16 nSegment, INT16 nSide)
{
CWall *wallP = FindWall (nSegment, nSide);

if (!wallP || ((wallP->type != WALL_DOOR) && (wallP->type != WALL_BLASTABLE)))
	return true;

CSide *sideP = Segments (0) [nSegment].sides + nSide;

if (FindClip (wallP, sideP->m_info.nOvlTex) >= 0)
	return true;
if (FindClip (wallP, sideP->m_info.nBaseTex) >= 0)
	return true;
return false;
}

//------------------------------------------------------------------------
// CheckForDoor()
//------------------------------------------------------------------------

void CMine::CheckForDoor (INT16 nSegment, INT16 nSide) 
{
GetCurrent (nSegment, nSide);
// put up a warning if changing a door's texture
UINT16 nWall = Segments (nSegment)->m_sides [nSide].m_info.nWall;

if (!bExpertMode &&
    (nWall < GameInfo ().walls.count) &&
	 ((Walls (nWall)->type == WALL_BLASTABLE) || (Walls (nWall)->type == WALL_DOOR)))
		ErrorMsg ("Changing the texture of a door only affects\n"
					"how the door will look before it is opened.\n"
					"You can use this trick to hide a door\n"
					"until it is used for the first time.\n\n"
					"Hint: To change the door animation,\n"
					"select \"Wall edit...\" from the Tools\n"
					"menu and change the clip number.");
}

// ------------------------------------------------------------------------

INT32 CWall::Read (FILE* fp, INT32 version, bool bFlag)
{
m_nSegment = read_INT32 (fp);
m_nSide = read_INT32 (fp); 
hps = read_FIX (fp);
linkedWall = read_INT32 (fp);
type = UINT8 (read_INT8 (fp));
flags = UINT16 ((version < 37) ? read_INT8 (fp) : read_INT16 (fp));         
state = UINT8 (read_INT8 (fp));         
nTrigger = UINT8 (read_INT8 (fp));       
nClip = UINT8 (read_INT8 (fp));      
keys = UINT8 (read_INT8 (fp));          
controlling_trigger = read_INT8 (fp);
cloak_value = read_INT8 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CWall::Write (FILE* fp, INT32 version, bool bFlag)
{
write_INT32 (m_nSegment, fp);
write_INT32 (m_nSide, fp); 
write_FIX (hps, fp);
write_INT32 (linkedWall, fp);
write_INT8 (type, fp);
if (version < 37) 
	write_INT8 (INT8 (flags), fp);
else
	write_INT16 (flags, fp);         
write_INT8 (state, fp);         
write_INT8 (nTrigger, fp);       
write_INT8 (nClip, fp);      
write_INT8 (keys, fp);          
write_INT8 (controlling_trigger, fp);
write_INT8 (cloak_value, fp);
}


// ------------------------------------------------------------------------

INT32 CActiveDoor::Read (FILE *fp, INT32 version, bool bFlag)
{
n_parts = read_INT32 (fp);
nFrontWall [0] = read_INT16 (fp);
nFrontWall [1] = read_INT16 (fp);
nBackWall [0] = read_INT16 (fp); 
nBackWall [1] = read_INT16 (fp); 
time = read_INT32 (fp);		  
return 1;
}

// ------------------------------------------------------------------------

void CActiveDoor::Write (FILE *fp, INT32 version, bool bFlag)
{
write_INT32 (n_parts, fp);
write_INT16 (nFrontWall[0], fp);
write_INT16 (nFrontWall[1], fp);
write_INT16 (nBackWall[0], fp); 
write_INT16 (nBackWall[1], fp); 
write_INT32 (time, fp);		  
}

// ------------------------------------------------------------------------


//eof wall.cpp