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
#include "cfile.h"

//------------------------------------------------------------------------
// make_object ()
//
// Action - Defines a standard object (currently assumed to be a player)
//------------------------------------------------------------------------

void CGameObject::Create (byte type, short nSegment) 
{
  CVertex	location;

undoManager.SetModified (true);
undoManager.Lock ();
segmentManager.CalcSegCenter (location, nSegment);
Clear ();
m_info.signature = 0;
m_info.type = type;
m_info.id = (type == OBJ_WEAPON) ? SMALLMINE_ID : 0;
m_info.controlType = CT_NONE; /* player 0 only */
m_info.movementType = MT_PHYSICS;
m_info.renderType = RT_POLYOBJ;
m_info.flags	= 0;
m_info.nSegment = current.m_nSegment;
m_location.pos = location;
m_location.orient.rVec.Set (F1_0, 0, 0);
m_location.orient.uVec.Set (0, F1_0, 0);
m_location.orient.fVec.Set (0, 0, F1_0);
m_info.size = PLAYER_SIZE;
m_info.shields = DEFAULT_SHIELD;
rType.polyModelInfo.nModel = PLAYER_CLIP_NUMBER;
rType.polyModelInfo.tmap_override = -1;
m_info.contents.type = 0;
m_info.contents.id = 0;
m_info.contents.count = 0;
undoManager.Unlock ();
return;
}

//------------------------------------------------------------------------
// set_object_data ()
//
// Action - Sets control type, movement type, render type
// 	    size, and shields (also nModel & texture if robot)
//------------------------------------------------------------------------

void CGameObject::Setup (char type) 
{
  int  id;

undoManager.SetModified (true);
undoManager.Lock ();
id = m_info.id;
memset (&mType, 0, sizeof (mType));
memset (&cType, 0, sizeof (cType));
memset (&rType, 0, sizeof (rType));
switch (type) {
	case OBJ_ROBOT: // an evil enemy
		m_info.controlType = CT_AI;
		m_info.movementType = MT_PHYSICS;
		m_info.renderType = RT_POLYOBJ;
		m_info.size = robotSize[id];
		m_info.shields = robot_shield[id];
		rType.polyModelInfo.nModel = robotClip[id];
		rType.polyModelInfo.tmap_override = -1; // set texture to none
		cType.aiInfo.behavior = AIB_NORMAL;
		break;

	case OBJ_HOSTAGE: // a hostage you need to rescue
		m_info.controlType = CT_POWERUP;
		m_info.movementType = MT_NONE;
		m_info.renderType = RT_HOSTAGE;
		rType.vClipInfo.vclip_num = HOSTAGE_CLIP_NUMBER;
		m_info.size = PLAYER_SIZE;
		m_info.shields = DEFAULT_SHIELD;
		break;

	case OBJ_PLAYER: // the player on the console
		m_info.controlType = (m_info.id == 0) ? CT_NONE : CT_SLEW; /* player 0 only */
		m_info.movementType = MT_PHYSICS;
		m_info.renderType = RT_POLYOBJ;
		m_info.size = PLAYER_SIZE;
		m_info.shields = DEFAULT_SHIELD;
		rType.polyModelInfo.nModel = PLAYER_CLIP_NUMBER;
		rType.polyModelInfo.tmap_override = -1; // set texture to normal
		break;

	case OBJ_WEAPON: // a poly-type weapon
		m_info.controlType = CT_WEAPON;
		m_info.movementType = MT_PHYSICS;
		m_info.renderType = RT_POLYOBJ;
		m_info.size = WEAPON_SIZE;
		m_info.shields = WEAPON_SHIELD;
		rType.polyModelInfo.nModel = MINE_CLIP_NUMBER;
		rType.polyModelInfo.tmap_override = -1; // set texture to normal
		mType.physInfo.mass = 65536L;
		mType.physInfo.drag = 2162;
		mType.physInfo.rotvel.x = 0;
		mType.physInfo.rotvel.y = 46482L;  // don't know exactly what to put here
		mType.physInfo.rotvel.z = 0;
		mType.physInfo.flags = 260;
		cType.laserInfo.parent_type = 5;
		cType.laserInfo.parent_num = 146; // don't know exactly what to put here
		cType.laserInfo.parent_signature = 146; // don't know exactly what to put here
		break;

	case OBJ_POWERUP: // a powerup you can pick up
		m_info.controlType = CT_POWERUP;
		m_info.movementType = MT_NONE;
		m_info.renderType = RT_POWERUP;
		rType.vClipInfo.vclip_num = powerupClip[id];
		m_info.size = powerupSize[id];
		m_info.shields = DEFAULT_SHIELD;
		break;

	case OBJ_CNTRLCEN: // the reactor
		m_info.controlType = CT_CNTRLCEN;
		m_info.movementType = MT_NONE;
		m_info.renderType = RT_POLYOBJ;
		m_info.size = REACTOR_SIZE;
		m_info.shields = REACTOR_SHIELD;
		if (theMine->IsD1File ())
			rType.polyModelInfo.nModel = REACTOR_CLIP_NUMBER;
		else {
			int model;
			switch (id) {
				case 1:  model = 95;  break;
				case 2:  model = 97;  break;
				case 3:  model = 99;  break;
				case 4:  model = 101; break;
				case 5:  model = 103; break;
				case 6:  model = 105; break;
				default: model = 97;  break; // level 1's reactor
				}
			rType.polyModelInfo.nModel = model;
			}
		rType.polyModelInfo.tmap_override = -1; // set texture to none
		break;

	case OBJ_COOP: // a cooperative player object
		m_info.controlType = CT_NONE;
		m_info.movementType = MT_PHYSICS;
		m_info.renderType = RT_POLYOBJ;
		m_info.size = PLAYER_SIZE;
		m_info.shields = DEFAULT_SHIELD;
		rType.polyModelInfo.nModel = COOP_CLIP_NUMBER;
		rType.polyModelInfo.tmap_override = -1; // set texture to none
		break;

	case OBJ_CAMBOT:
	case OBJ_SMOKE:
	case OBJ_MONSTERBALL:
		m_info.controlType = CT_AI;
		m_info.movementType = MT_NONE;
		m_info.renderType = RT_POLYOBJ;
		m_info.size = robotSize[0];
		m_info.shields = DEFAULT_SHIELD;
		rType.polyModelInfo.nModel = robotClip [0];
		rType.polyModelInfo.tmap_override = -1; // set texture to none
		cType.aiInfo.behavior = AIB_STILL;
		break;

	case OBJ_EXPLOSION:
		m_info.controlType = CT_POWERUP;
		m_info.movementType = MT_NONE;
		m_info.renderType = RT_POWERUP;
		m_info.size = robotSize[0];
		m_info.shields = DEFAULT_SHIELD;
		rType.vClipInfo.vclip_num = VCLIP_BIG_EXPLOSION;
		rType.polyModelInfo.tmap_override = -1; // set texture to none
		cType.aiInfo.behavior = AIB_STILL;
		break;

	case OBJ_EFFECT:
		m_info.controlType = CT_NONE;
		m_info.movementType = MT_NONE;
		m_info.renderType = RT_NONE;
		m_info.size = f1_0;
		m_info.shields = DEFAULT_SHIELD;
	}
undoManager.Unlock ();
}

//------------------------------------------------------------------------
/// CObjectTool - DrawObject
//------------------------------------------------------------------------

void CObject::Draw (CWnd* wndP)
{
	static int powerupTable [48] = {
		 0,  1,  2,  3,  4,  5,  6, -1, -1, -1, 
		 7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 
		17, 18, 19, 20, -1, 21, -1, -1, 22, 23, 
		24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 
		34, 35, 36, 37, 38, 39, 40, 41
		};

	int nBitmap = -1;

// figure out object number based on object type and id

switch (m_info.type) {
	case OBJ_PLAYER:
		nBitmap = 0;
		break;
	case OBJ_COOP:
		nBitmap = 0;
		break;
	case OBJ_ROBOT:
		nBitmap = (id < 66) ? 1 + id : 118 + (id - 66);
		break;
	case OBJ_CNTRLCEN:
	if (theMine->IsD1File ())
		nBitmap = 67;
	else
		switch (id) {
			case 1: nBitmap = 68; break;
			case 2: nBitmap = 69; break;
			case 3: nBitmap = 70; break;
			case 4: nBitmap = 71; break;
			case 5: nBitmap = 72; break;
			case 6: nBitmap = 73; break;
			default: nBitmap = 69; break; // level 1's reactor
			}
		break;
	case OBJ_WEAPON:
		nBitmap = 74;
		break;
	case OBJ_HOSTAGE:
		nBitmap = 75;
		break;
	case OBJ_POWERUP:
		if ((m_info.id >= 0) && (m_info.id < MAX_POWERUP_IDS) && (powerupTable [m_info.id] >= 0))
			nBitmap = 76 + powerupTable [m_info.id];
		break;
	default:
		nBitmap = -1; // undefined
}

CDC *pDC = wndP->GetDC ();
CRect rc;
wndP->GetClientRect (rc);
pDC->FillSolidRect (&rc, IMG_BKCOLOR);
if ((nBitmap >= 0) && (nBitmap <= 129)) {
	sprintf_s (message, sizeof (message),"OBJ_%03d_BMP", nBitmap);
	CResource res;
	char *resP = (char*) res.Load (message, RT_BITMAP);
	BITMAPINFO *bmi = (BITMAPINFO *) resP;
	if (bmi) {	//if not, there is a problem in the resource file
		int ncolors = (int) bmi->bmiHeader.biClrUsed;
		if (ncolors == 0)
			ncolors = 1 << (bmi->bmiHeader.biBitCount); // 256 colors for 8-bit data
		char *pImage = resP + sizeof (BITMAPINFOHEADER) + ncolors * 4;
		int width = (int) bmi->bmiHeader.biWidth;
		int height = (int) bmi->bmiHeader.biHeight;
		int xoffset = (64 - width) / 2;
		int yoffset = (64 - height) / 2;
		SetDIBitsToDevice (pDC->m_hDC, xoffset, yoffset, width, height, 0, 0, 0, height,pImage, bmi, DIB_RGB_COLORS);
		}
	}
wndP->ReleaseDC (pDC);
wndP->InvalidateRect (NULL, TRUE);
wndP->UpdateWindow ();
}

// ------------------------------------------------------------------------

int CObjPhysicsInfo::Read (CFileManager& fp, int version)
{
fp.ReadVector (velocity);
fp.ReadVector (thrust);
mass = fp.ReadInt32 ();
drag = fp.ReadInt32 ();
brakes = fp.ReadInt32 ();
fp.ReadVector (rotvel);
fp.ReadVector (rotthrust);
turnroll = fp.ReadFixAng ();
flags = fp.ReadInt16 ();
return 1;
}

// ------------------------------------------------------------------------

void CObjPhysicsInfo::Write (CFileManager& fp, int version)
{
fp.WriteVector (velocity);
fp.WriteVector (thrust);
fp.WriteInt32 (mass);
fp.WriteInt32 (drag);
fp.WriteInt32 (brakes);
fp.WriteVector (rotvel);
fp.WriteVector (rotthrust);
fp.WriteInt16 (turnroll);
fp.WriteInt16 (flags);
}

// ------------------------------------------------------------------------

int CObjAIInfo::Read (CFileManager& fp, int version)
{
behavior = fp.ReadSByte ();
for (int i = 0; i < MAX_AI_FLAGS; i++)
	flags [i] = fp.ReadSByte ();
hide_segment = fp.ReadInt16 ();
hide_index = fp.ReadInt16 ();
path_length = fp.ReadInt16 ();
cur_path_index = fp.ReadInt16 ();
if (DLE.IsD1File ()) {
	follow_path_start_seg = fp.ReadInt16 ();
	follow_path_end_seg = fp.ReadInt16 ();
	}
return 1;
}

// ------------------------------------------------------------------------

void CObjAIInfo::Write (CFileManager& fp, int version)
{
fp.Write (behavior);
for (int i = 0; i < MAX_AI_FLAGS; i++)
	fp.Write (flags [i]);
fp.Write (hide_segment);
fp.Write (hide_index);
fp.Write (path_length);
fp.Write (cur_path_index);
if (DLE.IsD1File ()) {
	fp.Write (follow_path_start_seg);
	fp.Write (follow_path_end_seg);
	}
}

// ------------------------------------------------------------------------

int CObjExplosionInfo::Read (CFileManager& fp, int version)
{
spawn_time = fp.ReadInt32 ();
delete_time = fp.ReadInt32 ();
delete_objnum = (byte)fp.ReadInt16 ();
next_attach = 
prev_attach = 
attach_parent =-1;
return 1;
}

// ------------------------------------------------------------------------

void CObjExplosionInfo::Write (CFileManager& fp, int version)
{
fp.Write (spawn_time);
fp.Write (delete_time);
fp.Write (delete_objnum);
}

// ------------------------------------------------------------------------

int CObjLaserInfo::Read (CFileManager& fp, int version)
{
parent_type = fp.ReadInt16 ();
parent_num = fp.ReadInt16 ();
parent_signature = fp.ReadInt32 ();
return 1;
}

// ------------------------------------------------------------------------

void CObjLaserInfo::Write (CFileManager& fp, int version)
{
fp.Write (parent_type);
fp.Write (parent_num);
fp.Write (parent_signature);
}

// ------------------------------------------------------------------------

int CObjPowerupInfo::Read (CFileManager& fp, int version)
{
count = (version >= 25) ? fp.ReadInt32 () : 1;
return 1;
}

// ------------------------------------------------------------------------

void CObjPowerupInfo::Write (CFileManager& fp, int version)
{
if (version >= 25) 
	fp.Write (count);
}

// ------------------------------------------------------------------------

int CObjLightInfo::Read (CFileManager& fp, int version)
{
intensity = fp.ReadInt32 ();
return 1;
}

// ------------------------------------------------------------------------

void CObjLightInfo::Write (CFileManager& fp, int version)
{
fp.Write (intensity);
}

// ------------------------------------------------------------------------

int CObjPolyModelInfo::Read (CFileManager& fp, int version)
{
nModel = fp.ReadInt32 ();
for (int i = 0; i < MAX_SUBMODELS; i++)
	fp.ReadVector (anim_angles [i]);
subobj_flags = fp.ReadInt32 ();
tmap_override = fp.ReadInt32 ();
alt_textures = 0;
return 1;
}

// ------------------------------------------------------------------------

void CObjPolyModelInfo::Write (CFileManager& fp, int version)
{
fp.Write (nModel);
for (int i = 0; i < MAX_SUBMODELS; i++)
	fp.WriteVector (anim_angles [i]);
fp.Write (subobj_flags);
fp.Write (tmap_override);
}

// ------------------------------------------------------------------------

int CObjVClipInfo::Read (CFileManager& fp, int version)
{
vclip_num = fp.ReadInt32 ();
frametime = fp.ReadInt32 ();
framenum = fp.ReadSByte ();
return 1;
}

// ------------------------------------------------------------------------

void CObjVClipInfo::Write (CFileManager& fp, int version)
{
fp.Write (vclip_num);
fp.Write (frametime);
fp.Write (framenum);
}

// ------------------------------------------------------------------------

int CSmokeInfo::Read (CFileManager& fp, int version)
{
nLife = fp.ReadInt32 ();
nSize [0] = fp.ReadInt32 ();
nParts = fp.ReadInt32 ();
nSpeed = fp.ReadInt32 ();
nDrift = fp.ReadInt32 ();
nBrightness = fp.ReadInt32 ();
for (int i = 0; i < 4; i++)
	color [i] = fp.ReadSByte ();
nSide = fp.ReadSByte ();
nType = (version < 18) ? 0 : fp.ReadSByte ();
bEnabled = (version < 19) ? 1 : fp.ReadSByte ();
return 1;
}

// ------------------------------------------------------------------------

void CSmokeInfo::Write (CFileManager& fp, int version)
{
fp.Write (nLife);
fp.Write (nSize [0]);
fp.Write (nParts);
fp.Write (nSpeed);
fp.Write (nDrift);
fp.Write (nBrightness);
for (int i = 0; i < 4; i++)
	fp.Write (color [i]);
fp.Write (nSide);
fp.Write (nSide);
fp.Write (bEnabled);
}

// ------------------------------------------------------------------------

int CLightningInfo::Read (CFileManager& fp, int version)
{
nLife = fp.ReadInt32 ();
nDelay = fp.ReadInt32 ();
nLength = fp.ReadInt32 ();
nAmplitude = fp.ReadInt32 ();
nOffset = fp.ReadInt32 ();
nLightnings = fp.ReadInt16 ();
nId = fp.ReadInt16 ();
nTarget = fp.ReadInt16 ();
nNodes = fp.ReadInt16 ();
nChildren = fp.ReadInt16 ();
nSteps = fp.ReadInt16 ();
nAngle = fp.ReadSByte ();
nStyle = fp.ReadSByte ();
nSmoothe = fp.ReadSByte ();
bClamp = fp.ReadSByte ();
bPlasma = fp.ReadSByte ();
bSound = fp.ReadSByte ();
bRandom = fp.ReadSByte ();
bInPlane = fp.ReadSByte ();
for (int i = 0; i < 4; i++)
	color [i] = fp.ReadSByte ();
bEnabled = (version < 19) ? 1 : fp.ReadSByte ();
return 1;
}

// ------------------------------------------------------------------------

void CLightningInfo::Write (CFileManager& fp, int version)
{
fp.Write (nLife);
fp.Write (nDelay);
fp.Write (nLength);
fp.Write (nAmplitude);
fp.Write (nOffset);
fp.Write (nLightnings);
fp.Write (nId);
fp.Write (nTarget);
fp.Write (nNodes);
fp.Write (nChildren);
fp.Write (nSteps);
fp.Write (nAngle);
fp.Write (nStyle);
fp.Write (nSmoothe);
fp.Write (bClamp);
fp.Write (bPlasma);
fp.Write (bSound);
fp.Write (bRandom);
fp.Write (bInPlane);
for (int i = 0; i < 4; i++)
	fp.Write (color [i]);
fp.Write (bEnabled);
}

// ------------------------------------------------------------------------

int CSoundInfo::Read (CFileManager& fp, int version)
{
fp.Read (szFilename, 1, sizeof (szFilename));
nVolume = fp.ReadInt32 ();
bEnabled = (version < 19) ? 1 : fp.ReadSByte ();
return 1;
}
// ------------------------------------------------------------------------

void CSoundInfo::Write (CFileManager& fp, int version)
{
fp.Write (szFilename, 1, sizeof (szFilename));
fp.Write (nVolume);
fp.Write (bEnabled);
}

// ------------------------------------------------------------------------

int CGameObject::Read (CFileManager& fp, int version, bool bFlag) 
{
m_info.type = fp.ReadSByte ();
m_info.id = fp.ReadSByte ();
m_info.controlType = fp.ReadSByte ();
m_info.movementType = fp.ReadSByte ();
m_info.renderType = fp.ReadSByte ();
m_info.flags = fp.ReadSByte ();
m_info.multiplayer = (version > 37) ? fp.ReadSByte () : 0;
m_info.nSegment = fp.ReadInt16 ();
m_location.pos.Read (fp);
fp.Read (m_location.orient);
m_info.size = fp.ReadInt32 ();
m_info.shields = fp.ReadInt32 ();
m_location.lastPos.Read (fp);
m_info.contents.type = fp.ReadSByte ();
m_info.contents.id = fp.ReadSByte ();
m_info.contents.count = fp.ReadSByte ();

switch (m_info.movementType) {
	case MT_PHYSICS:
		mType.physInfo.Read (fp, version);
		break;
	case MT_SPINNING:
		fp.ReadVector (mType.spinRate);
		break;
	case MT_NONE:
		break;
	default:
		break;
	}

switch (m_info.controlType) {
	case CT_AI:
		cType.aiInfo.Read (fp, version);
		break;
	case CT_EXPLOSION:
		cType.explInfo.Read (fp, version);
		break;
	case CT_WEAPON:
		cType.laserInfo.Read (fp, version);
		break;
	case CT_LIGHT:
		cType.lightInfo.Read (fp, version);
		break;
	case CT_POWERUP:
		cType.powerupInfo.Read (fp, version);
		break;
	case CT_NONE:
	case CT_FLYING:
	case CT_DEBRIS:
		break;
	case CT_SLEW:    /*the player is generally saved as slew */
		break;
	case CT_CNTRLCEN:
		break;
	case CT_MORPH:
	case CT_FLYTHROUGH:
	case CT_REPAIRCEN:
		default:
		break;
	}

switch (m_info.renderType) {
	case RT_NONE:
		break;
	case RT_MORPH:
	case RT_POLYOBJ: 
		rType.polyModelInfo.Read (fp, version);
		break;
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
		rType.vClipInfo.Read (fp, DLE.LevelVersion ());
		break;
	case RT_LASER:
		break;
	case RT_SMOKE:
		rType.smokeInfo.Read (fp, DLE.LevelVersion ());
		break;
	case RT_LIGHTNING:
		rType.lightningInfo.Read (fp, DLE.LevelVersion ());
		break;
	case RT_SOUND:
		rType.soundInfo.Read (fp, DLE.LevelVersion ());
		break;
	default:
	break;
	}

return 1;
}

// ------------------------------------------------------------------------
// WriteObject ()
// ------------------------------------------------------------------------

void CGameObject::Write (CFileManager& fp, int version, bool bFlag)
{
if (theMine->IsStdLevel () && (m_info.type >= OBJ_CAMBOT))
	return;	// not a d2x-xl level, but a d2x-xl object

fp.Write (m_info.type);
fp.Write (m_info.id);
fp.Write (m_info.controlType);
fp.Write (m_info.movementType);
fp.Write (m_info.renderType);
fp.Write (m_info.flags);
fp.Write (m_info.multiplayer);
fp.Write (m_info.nSegment);
fp.Write (m_location.pos);
fp.Write (m_location.orient);
fp.Write (m_info.size);
fp.Write (m_info.shields);
fp.Write (m_location.lastPos);
fp.Write (m_info.contents.type);
fp.Write (m_info.contents.id);
fp.Write (m_info.contents.count);

switch (m_info.movementType) {
	case MT_PHYSICS:
		mType.physInfo.Write (fp, version);
		break;
	case MT_SPINNING:
		fp.Write (mType.spinRate);
		break;
	case MT_NONE:
		break;
	default:
		break;
	}

switch (m_info.controlType) {
	case CT_AI:
		cType.aiInfo.Write (fp, version);
		break;
	case CT_EXPLOSION:
		cType.explInfo.Write (fp, version);
		break;
	case CT_WEAPON:
		cType.laserInfo.Write (fp, version);
		break;
	case CT_LIGHT:
		cType.lightInfo.Write (fp, version);
		break;
	case CT_POWERUP:
		cType.powerupInfo.Write (fp, version);
		break;
	case CT_NONE:
	case CT_FLYING:
	case CT_DEBRIS:
		break;
	case CT_SLEW:    /*the player is generally saved as slew */
		break;
	case CT_CNTRLCEN:
		break;
	case CT_MORPH:
	case CT_FLYTHROUGH:
	case CT_REPAIRCEN:
		default:
		break;
	}

switch (m_info.renderType) {
	case RT_NONE:
		break;
	case RT_MORPH:
	case RT_POLYOBJ:
		rType.polyModelInfo.Write (fp, version);
	break;
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
		rType.vClipInfo.Write (fp, version);
		break;
	case RT_LASER:
		break;
	case RT_SMOKE:
		rType.smokeInfo.Write (fp, version);
		break;
	case RT_LIGHTNING:
		rType.lightningInfo.Write (fp, version);
		break;
	case RT_SOUND:
		rType.soundInfo.Write (fp, version);
		break;
	default:
		break;
	}
}

// ------------------------------------------------------------------------
// eof object.cpp