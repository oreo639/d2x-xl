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

//------------------------------------------------------------------------
// SortObjects ()
//------------------------------------------------------------------------

static short sortObjType [MAX_OBJECT_TYPES] = {7, 8, 5, 4, 0, 2, 9, 3, 10, 6, 11, 12, 13, 14, 1, 16, 15, 17, 18, 19, 20};


int CMine::QCmpObjects (CGameObject *pi, CGameObject *pm)
{
	short ti = sortObjType [pi->m_info.type];
	short tm = sortObjType [pm->m_info.type];
if (ti < tm)
	return -1;
if (ti > tm)
	return 1;
return (pi->m_info.id < pm->m_info.id) ? -1 : (pi->m_info.id > pm->m_info.id) ? 1 : 0;
}


short CMine::FindObjBySig (short signature)
{
	CGameObject*	objP = Objects (0);

for (short i = ObjCount (); i; i--, objP++)
	if (objP->m_info.signature == signature)
		return short (objP - Objects (0));
return -1;
}


void CMine::RenumberTriggerTargetObjs (void)
{
	CTrigger*	trigP = Triggers (0);

for (int i = TriggerCount (); i; i--, trigP++) {
	for (int j = 0; j < trigP->m_count; ) {
		if (trigP->Side (j) >= 0) 
			j++;
		else {
			int h = FindObjBySig (trigP->Segment (j));
			if (h >= 0)
				trigP->Side (j++) = h;
			else if (j < --trigP->m_count) {
				trigP [j] = trigP [trigP->m_count];
				}
			}
		}
	}
}


void CMine::RenumberObjTriggers (void)
{
	CTrigger*	trigP = ObjTriggers (0);
	int			i;

for (i = NumObjTriggers (); i; i--, trigP++)
	trigP->m_info.nObject = FindObjBySig (trigP->m_info.nObject);
i = NumObjTriggers ();
while (i) {
	if (ObjTriggers (--i)->m_info.nObject < 0)
		DeleteObjTrigger (i);
	}
SortObjTriggers ();
}


void CMine::QSortObjects (short left, short right)
{
	CGameObject	median = *Objects ((left + right) / 2);
	short	l = left, r = right;

do {
	while (QCmpObjects (Objects (l), &median) < 0)
		l++;
	while (QCmpObjects (Objects (r), &median) > 0)
		r--;
	if (l <= r) {
		if (l < r) {
			CGameObject o = *Objects (l);
			*Objects (l) = *Objects (r);
			*Objects (r) = o;
			if (Current ()->nObject == l)
				Current ()->nObject = r;
			else if (Current ()->nObject == r)
				Current ()->nObject = l;
			}
		l++;
		r--;
		}
	}
while (l < r);
if (l < right)
	QSortObjects (l, right);
if (left < r)
	QSortObjects (left, r);
}


void CMine::SortObjects ()
{
	int	i, j;

if (m_bSortObjects && ( (i = GameInfo ().objects.count) > 1)) {
	for (j = 0; j < i; j++)
		Objects (j)->m_info.signature = j;
	QSortObjects (0, i - 1);
	RenumberObjTriggers ();
	RenumberTriggerTargetObjs ();
	}
}

//------------------------------------------------------------------------
// make_object ()
//
// Action - Defines a standard object (currently assumed to be a player)
//------------------------------------------------------------------------

void CMine::MakeObject (CGameObject *objP, char type, short nSegment) 
{
  CVertex	location;

theApp.SetModified (TRUE);
theApp.LockUndo ();
CalcSegCenter (location,nSegment);
objP->Clear ();
objP->m_info.signature = 0;
objP->m_info.type = type;
objP->m_info.id = (type == OBJ_WEAPON) ? SMALLMINE_ID : 0;
objP->m_info.controlType = CT_NONE; /* player 0 only */
objP->m_info.movementType = MT_PHYSICS;
objP->m_info.renderType = RT_POLYOBJ;
objP->m_info.flags	= 0;
objP->m_info.nSegment = Current ()->nSegment;
objP->m_location.pos = location;
objP->m_location.orient.rVec.Set (F1_0, 0, 0);
objP->m_location.orient.uVec.Set (0, F1_0, 0);
objP->m_location.orient.fVec.Set (0, 0, F1_0);
objP->m_info.size = PLAYER_SIZE;
objP->m_info.shields = DEFAULT_SHIELD;
objP->rType.polyModelInfo.nModel = PLAYER_CLIP_NUMBER;
objP->rType.polyModelInfo.tmap_override = -1;
objP->m_info.contents.type = 0;
objP->m_info.contents.id = 0;
objP->m_info.contents.count = 0;
theApp.UnlockUndo ();
return;
}

//------------------------------------------------------------------------
// set_object_data ()
//
// Action - Sets control type, movement type, render type
// 	    size, and shields (also nModel & texture if robot)
//------------------------------------------------------------------------

void CMine::SetObjectData (char type) 
{
  CGameObject *objP;
  int  id;

theApp.SetModified (TRUE);
theApp.LockUndo ();
objP = Objects (0) + Current ()->nObject;
id = objP->m_info.id;
memset (&objP->mType, 0, sizeof (objP->mType));
memset (&objP->cType, 0, sizeof (objP->cType));
memset (&objP->rType, 0, sizeof (objP->rType));
switch (type) {
	case OBJ_ROBOT: // an evil enemy
	  objP->m_info.controlType  = CT_AI;
	  objP->m_info.movementType = MT_PHYSICS;
	  objP->m_info.renderType   = RT_POLYOBJ;
	  objP->m_info.size          = robotSize[id];
	  objP->m_info.shields       = robot_shield[id];
	  objP->rType.polyModelInfo.nModel = robotClip[id];
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  objP->cType.aiInfo.behavior = AIB_NORMAL;
	  break;

	case OBJ_HOSTAGE  : // a hostage you need to rescue
	  objP->m_info.controlType = CT_POWERUP;
	  objP->m_info.movementType = MT_NONE;
	  objP->m_info.renderType   = RT_HOSTAGE;
	  objP->rType.vClipInfo.vclip_num = HOSTAGE_CLIP_NUMBER;
	  objP->m_info.size          = PLAYER_SIZE;
	  objP->m_info.shields       = DEFAULT_SHIELD;
	  break;

	case OBJ_PLAYER   : // the player on the console
	  if (objP->m_info.id == 0) {
		objP->m_info.controlType = CT_NONE; /* player 0 only */
	  } else {
		objP->m_info.controlType = CT_SLEW; /* all other players */
	  }
	  objP->m_info.movementType = MT_PHYSICS;
	  objP->m_info.renderType   = RT_POLYOBJ;
	  objP->m_info.size          = PLAYER_SIZE;
	  objP->m_info.shields       = DEFAULT_SHIELD;
	  objP->rType.polyModelInfo.nModel = PLAYER_CLIP_NUMBER;
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to normal
	  break;

	case OBJ_WEAPON   : // a poly-type weapon
	  objP->m_info.controlType  = CT_WEAPON;
	  objP->m_info.movementType = MT_PHYSICS;
	  objP->m_info.renderType   = RT_POLYOBJ;
	  objP->m_info.size          = WEAPON_SIZE;
	  objP->m_info.shields       = WEAPON_SHIELD;
	  objP->rType.polyModelInfo.nModel = MINE_CLIP_NUMBER;
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to normal
	  objP->mType.physInfo.mass      = 65536L;
	  objP->mType.physInfo.drag      = 2162;
	  objP->mType.physInfo.rotvel.x  = 0;
	  objP->mType.physInfo.rotvel.y  = 46482L;  // don't know exactly what to put here
	  objP->mType.physInfo.rotvel.z  = 0;
	  objP->mType.physInfo.flags     = 260;
	  objP->cType.laserInfo.parent_type      = 5;
	  objP->cType.laserInfo.parent_num       = 146; // don't know exactly what to put here
	  objP->cType.laserInfo.parent_signature = 146; // don't know exactly what to put here
	  break;

	case OBJ_POWERUP  : // a powerup you can pick up
	  objP->m_info.controlType  = CT_POWERUP;
	  objP->m_info.movementType = MT_NONE;
	  objP->m_info.renderType   = RT_POWERUP;
	  objP->rType.vClipInfo.vclip_num = powerupClip[id];
	  objP->m_info.size          = powerupSize[id];
	  objP->m_info.shields       = DEFAULT_SHIELD;
	  break;

	case OBJ_CNTRLCEN : // the reactor
	  objP->m_info.controlType = CT_CNTRLCEN;
	  objP->m_info.movementType = MT_NONE;
	  objP->m_info.renderType   = RT_POLYOBJ;
	  objP->m_info.size          = REACTOR_SIZE;
	  objP->m_info.shields       = REACTOR_SHIELD;
	  if (IsD1File ())
			objP->rType.polyModelInfo.nModel = REACTOR_CLIP_NUMBER;
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
		objP->rType.polyModelInfo.nModel = model;
	  }
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  break;

	case OBJ_COOP     : // a cooperative player object
	  objP->m_info.controlType = CT_NONE;
	  objP->m_info.movementType = MT_PHYSICS;
	  objP->m_info.renderType   = RT_POLYOBJ;
	  objP->m_info.size          = PLAYER_SIZE;
	  objP->m_info.shields       = DEFAULT_SHIELD;
	  objP->rType.polyModelInfo.nModel = COOP_CLIP_NUMBER;
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  break;

	case OBJ_CAMBOT:
	case OBJ_SMOKE:
	case OBJ_MONSTERBALL:
	  objP->m_info.controlType  = CT_AI;
	  objP->m_info.movementType = MT_NONE;
	  objP->m_info.renderType   = RT_POLYOBJ;
	  objP->m_info.size          = robotSize[0];
	  objP->m_info.shields       = DEFAULT_SHIELD;
	  objP->rType.polyModelInfo.nModel = robotClip [0];
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  objP->cType.aiInfo.behavior = AIB_STILL;
	  break;

	case OBJ_EXPLOSION:
	  objP->m_info.controlType  = CT_POWERUP;
	  objP->m_info.movementType = MT_NONE;
	  objP->m_info.renderType   = RT_POWERUP;
	  objP->m_info.size          = robotSize[0];
	  objP->m_info.shields       = DEFAULT_SHIELD;
	  objP->rType.vClipInfo.vclip_num = VCLIP_BIG_EXPLOSION;
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  objP->cType.aiInfo.behavior = AIB_STILL;
	  break;

	case OBJ_EFFECT:
	  objP->m_info.controlType  = CT_NONE;
	  objP->m_info.movementType = MT_NONE;
	  objP->m_info.renderType   = RT_NONE;
	  objP->m_info.size          = f1_0;
	  objP->m_info.shields       = DEFAULT_SHIELD;

  }
	theApp.UnlockUndo ();
}

//------------------------------------------------------------------------
// copy_object ()
//
// Action - Copies the current object to a new object
//          If object is a player or coop, it chooses the next available id
//
// Parameters - char new_type = new type of object
//
// Returns - TRUE upon success
//
//------------------------------------------------------------------------

bool CMine::CopyObject (byte new_type, short nSegment) 
{
	short objnum,id;
	short ids [MAX_PLAYERS_D2X + MAX_COOP_PLAYERS] = {0,0,0,0,0,0,0,0,0,0,0};
	CGameObject *objP,*current_obj;
	byte type;
	short i,count;

if (GameInfo ().objects.count >= MAX_OBJECTS) {
	ErrorMsg ("The maximum number of objects has already been reached.");
	return false;
	}
// If OBJ_NONE used, then make a copy of the current object type
// otherwise use the type passed as the parameter "new_type"
//--------------------------------------------------------------

type = (new_type == OBJ_NONE) ? CurrObj ()->m_info.type : new_type;

// Make sure it is ok to add another object of this type
// Set the id if it's a player or coop
//------------------------------------------------------
if (type == OBJ_PLAYER || type == OBJ_COOP) {
	objP = Objects (0);
	for (objnum = GameInfo ().objects.count; objnum; objnum--, objP++)
		if (objP->m_info.type == type) {
			id = objP->m_info.id;
			if (id >= 0 && id < (MAX_PLAYERS + MAX_COOP_PLAYERS))
				ids[id]++;
			}
	if (type == OBJ_PLAYER) {
		for (id = 0; (id <= MAX_PLAYERS) && ids[id]; id++)
				;// loop until 1st id with 0
		if (id > MAX_PLAYERS) {
				char szMsg [80];

			sprintf_s (szMsg, sizeof (szMsg), "There are already %d players in the mine", MAX_PLAYERS);
			ErrorMsg (szMsg);
			return FALSE;
			}
		}
	else {
		for (id = MAX_PLAYERS; (id < MAX_PLAYERS + MAX_COOP_PLAYERS) && ids[id]; id++)
			;// loop until 1st id with 0
		if (id > MAX_PLAYERS + MAX_COOP_PLAYERS) {
				char szMsg [80];

			sprintf_s (szMsg, sizeof (szMsg), "There are already %d cooperative players in the mine", MAX_COOP_PLAYERS);
			ErrorMsg (szMsg);
			return FALSE;
			}
		}
	}

// Now we can add the object
// Make a new object
theApp.SetModified (TRUE);
theApp.LockUndo ();
if (GameInfo ().objects.count == 0) {
	MakeObject (Objects (0), OBJ_PLAYER, (nSegment < 0) ? Current ()->nSegment : nSegment);
	GameInfo ().objects.count = 1;
	objnum = 0;
	}
else {
	// Make a copy of the current object
	objnum = GameInfo ().objects.count++;
	objP = Objects (objnum);
	current_obj = CurrObj ();
	memcpy (objP, current_obj, sizeof (CGameObject));
	}
objP->m_info.flags = 0;                                      // new: 1/27/97
objP->m_info.nSegment = Current ()->nSegment;
// set object position in the center of the cube for now
CalcSegCenter (objP->m_location.pos, Current ()->nSegment);
objP->m_location.lastPos = objP->m_location.pos;
Current ()->nObject = objnum;
// bump position over if this is not the first object in the cube
count = 0;
for (i = 0; i < GameInfo ().objects.count - 1; i++)
	if (Objects (i)->m_info.nSegment == Current ()->nSegment)
		count++;
objP->m_location.pos.v.y += count * 2 * F1_0;
objP->m_location.lastPos.v.y += count * 2 * F1_0;
// set the id if new object is a player or a coop
if (type == OBJ_PLAYER || type == OBJ_COOP)
	objP->m_info.id = (char) id;
// set object data if new object being added
if (new_type != OBJ_NONE) {
	objP->m_info.type = new_type;
	SetObjectData (objP->m_info.type);
	}
// set the contents to zero
objP->m_info.contents.type = 0;
objP->m_info.contents.id = 0;
objP->m_info.contents.count = 0;
SortObjects ();
theApp.MineView ()->Refresh (false);
theApp.ToolView ()->ObjectTool ()->Refresh ();
theApp.UnlockUndo ();
return TRUE;
}

//------------------------------------------------------------------------
// DeleteObject ()
//------------------------------------------------------------------------

void CMine::DeleteObject (short nDelObj)
{
if (GameInfo ().objects.count == 0) {
	if (!bExpertMode)
		ErrorMsg ("There are no Objects () in the mine.");
	return;
	}
if (GameInfo ().objects.count == 1) {
	if (!bExpertMode)
		ErrorMsg ("Cannot delete the last object.");
	return;
	}
if (nDelObj < 0)
	nDelObj = Current ()->nObject;
if (nDelObj == GameInfo ().objects.count) {
	if (!bExpertMode)
		ErrorMsg ("Cannot delete the secret return.");
	return;
	}
theApp.SetModified (TRUE);
theApp.LockUndo ();
DeleteObjTriggers (nDelObj);
int i, j = GameInfo ().objects.count;
for (i = nDelObj; i < j; i++)
	Objects (i)->m_info.signature = i;
if (nDelObj < --j)
	memcpy (Objects () + nDelObj, Objects () + nDelObj + 1, (GameInfo ().objects.count - nDelObj) * sizeof (CGameObject));
GameInfo ().objects.count = j;
RenumberObjTriggers ();
RenumberTriggerTargetObjs ();
if (Current1 ().nObject >= j)
	Current1 ().nObject = j - 1;
if (Current2 ().nObject >= j)
	Current2 ().nObject = j - 1;
theApp.UnlockUndo ();
}

//------------------------------------------------------------------------
/// CObjectTool - DrawObject
//------------------------------------------------------------------------

void CMine::DrawObject (CWnd *pWnd, int type, int id)
{
	int powerup_lookup[48] = {
		 0, 1, 2, 3, 4, 5, 6,-1,-1,-1,
		 7, 8, 9,10,11,12,13,14,15,16,
		17,18,19,20,-1,21,-1,-1,22,23,
		24,25,26,27,28,29,30,31,32,33,
		34,35,36,37,38,39,40,41
		};
	int object_number;

// figure out object number based on object type and id
object_number = -1; // assume that we can't find the object
switch (type) {
	case OBJ_PLAYER:
		object_number = 0;
		break;
	case OBJ_COOP:
		object_number = 0;
		break;
	case OBJ_ROBOT:
		object_number = (id < 66) ? 1 + id : 118 + (id - 66);
		break;
	case OBJ_CNTRLCEN:
	if (IsD1File ())
		object_number = 67;
	else
		switch (id) {
			case 1: object_number = 68; break;
			case 2: object_number = 69; break;
			case 3: object_number = 70; break;
			case 4: object_number = 71; break;
			case 5: object_number = 72; break;
			case 6: object_number = 73; break;
			default: object_number = 69; break; // level 1's reactor
			}
		break;
	case OBJ_WEAPON:
		object_number = 74;
		break;
	case OBJ_HOSTAGE:
		object_number = 75;
		break;
	case OBJ_POWERUP:
		if ( (id >= 0) && (id < MAX_POWERUP_IDS) && (powerup_lookup[id] >= 0))
			object_number = 76 + powerup_lookup[id];
		break;
	default:
		object_number = -1; // undefined
}

CDC *pDC = pWnd->GetDC ();
CRect rc;
pWnd->GetClientRect (rc);
pDC->FillSolidRect (&rc, IMG_BKCOLOR);
if ( (object_number >= 0) && (object_number <= 129)) {
	sprintf_s (message, sizeof (message),"OBJ_%03d_BMP", object_number);
	HINSTANCE hInst = AfxGetApp ()->m_hInstance;
	HRSRC hFind = FindResource (hInst, message, RT_BITMAP);
	HGLOBAL hGlobal = LoadResource (hInst, hFind);
	char *pRes = (char *)LockResource (hGlobal);
	BITMAPINFO *bmi = (BITMAPINFO *)pRes;
	if (bmi) {	//if not, there is a problem in the resource file
		int ncolors = (int)bmi->bmiHeader.biClrUsed;
		if (ncolors == 0)
			ncolors = 1 << (bmi->bmiHeader.biBitCount); // 256 colors for 8-bit data
		char *pImage = pRes + sizeof (BITMAPINFOHEADER) + ncolors * 4;
		int width = (int)bmi->bmiHeader.biWidth;
		int height = (int)bmi->bmiHeader.biHeight;
		int xoffset = (64 - width) / 2;
		int yoffset = (64 - height) / 2;
		SetDIBitsToDevice (pDC->m_hDC, xoffset,yoffset,width,height,0,0,
								0, height,pImage, bmi, DIB_RGB_COLORS);
		}
	FreeResource (hGlobal);
	}
pWnd->ReleaseDC (pDC);
pWnd->InvalidateRect (NULL, TRUE);
pWnd->UpdateWindow ();
}

// ------------------------------------------------------------------------

int CObjPhysicsInfo::Read (FILE *fp, int version)
{
velocity.Read (fp);
thrust.Read (fp);
mass = ReadFix (fp);
drag = ReadFix (fp);
brakes = ReadFix (fp);
rotvel.Read (fp);
rotthrust.Read (fp);
turnroll = ReadFixAng (fp);
flags = ReadInt16 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CObjPhysicsInfo::Write (FILE *fp, int version)
{
velocity.Write (fp);
thrust.Write (fp);
WriteFix (mass, fp);
WriteFix (drag, fp);
WriteFix (brakes, fp);
rotvel.Write (fp);
rotthrust.Write (fp);
WriteFixAng (turnroll, fp);
WriteInt16 (flags, fp);
}

// ------------------------------------------------------------------------

int CObjAIInfo::Read (FILE *fp, int version)
{
behavior = ReadInt8 (fp);
for (int i = 0; i < MAX_AI_FLAGS; i++)
	flags [i] = ReadInt8 (fp);
hide_segment = ReadInt16 (fp);
hide_index = ReadInt16 (fp);
path_length = ReadInt16 (fp);
cur_path_index = ReadInt16 (fp);
if (theApp.IsD1File ()) {
	follow_path_start_seg = ReadInt16 (fp);
	follow_path_end_seg = ReadInt16 (fp);
	}
return 1;
}

// ------------------------------------------------------------------------

void CObjAIInfo::Write (FILE *fp, int version)
{
WriteInt8 (behavior, fp);
for (int i = 0; i < MAX_AI_FLAGS; i++)
	WriteInt8 (flags [i], fp);
WriteInt16 (hide_segment, fp);
WriteInt16 (hide_index, fp);
WriteInt16 (path_length, fp);
WriteInt16 (cur_path_index, fp);
if (theApp.IsD1File ()) {
	WriteInt16 (follow_path_start_seg, fp);
	WriteInt16 (follow_path_end_seg, fp);
	}
}

// ------------------------------------------------------------------------

int CObjExplosionInfo::Read (FILE *fp, int version)
{
spawn_time = ReadFix (fp);
delete_time = ReadFix (fp);
delete_objnum = (byte)ReadInt16 (fp);
next_attach = 
prev_attach = 
attach_parent =-1;
return 1;
}

// ------------------------------------------------------------------------

void CObjExplosionInfo::Write (FILE *fp, int version)
{
WriteFix (spawn_time, fp);
WriteFix (delete_time, fp);
WriteInt16 (delete_objnum, fp);
}

// ------------------------------------------------------------------------

int CObjLaserInfo::Read (FILE *fp, int version)
{
parent_type = ReadInt16 (fp);
parent_num = ReadInt16 (fp);
parent_signature = ReadInt32 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CObjLaserInfo::Write (FILE *fp, int version)
{
WriteInt16 (parent_type, fp);
WriteInt16 (parent_num, fp);
WriteInt32 (parent_signature, fp);
}

// ------------------------------------------------------------------------

int CObjPowerupInfo::Read (FILE *fp, int version)
{
count = (version >= 25) ? ReadInt32 (fp) : 1;
return 1;
}

// ------------------------------------------------------------------------

void CObjPowerupInfo::Write (FILE *fp, int version)
{
if (version >= 25) 
	WriteInt32 (count, fp);
}

// ------------------------------------------------------------------------

int CObjLightInfo::Read (FILE *fp, int version)
{
intensity = ReadFix (fp);
return 1;
}

// ------------------------------------------------------------------------

void CObjLightInfo::Write (FILE *fp, int version)
{
WriteFix (intensity, fp);
}

// ------------------------------------------------------------------------

int CObjPolyModelInfo::Read (FILE *fp, int version)
{
nModel = ReadInt32 (fp);
for (int i = 0; i < MAX_SUBMODELS; i++)
	anim_angles [i].Read (fp);
subobj_flags = ReadInt32 (fp);
tmap_override = ReadInt32 (fp);
alt_textures = 0;
return 1;
}

// ------------------------------------------------------------------------

void CObjPolyModelInfo::Write (FILE *fp, int version)
{
WriteInt32 (nModel, fp);
for (int i = 0; i < MAX_SUBMODELS; i++)
	anim_angles [i].Write (fp);
WriteInt32 (subobj_flags, fp);
WriteInt32 (tmap_override, fp);
}

// ------------------------------------------------------------------------

int CObjVClipInfo::Read (FILE *fp, int version)
{
vclip_num = ReadInt32 (fp);
frametime = ReadFix (fp);
framenum = ReadInt8 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CObjVClipInfo::Write (FILE *fp, int version)
{
WriteInt32 (vclip_num, fp);
WriteFix (frametime, fp);
WriteInt8 (framenum, fp);
}

// ------------------------------------------------------------------------

int CSmokeInfo::Read (FILE *fp, int version)
{
nLife = ReadInt32 (fp);
nSize [0] = ReadInt32 (fp);
nParts = ReadInt32 (fp);
nSpeed = ReadInt32 (fp);
nDrift = ReadInt32 (fp);
nBrightness = ReadInt32 (fp);
for (int i = 0; i < 4; i++)
	color [i] = ReadInt8 (fp);
nSide = ReadInt8 (fp);
nType = (version < 18) ? 0 : ReadInt8 (fp);
bEnabled = (version < 19) ? 1 : ReadInt8 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CSmokeInfo::Write (FILE *fp, int version)
{
WriteInt32 (nLife, fp);
WriteInt32 (nSize [0], fp);
WriteInt32 (nParts, fp);
WriteInt32 (nSpeed, fp);
WriteInt32 (nDrift, fp);
WriteInt32 (nBrightness, fp);
for (int i = 0; i < 4; i++)
	WriteInt8 (color [i], fp);
WriteInt8 (nSide, fp);
WriteInt8 (nSide, fp);
WriteInt8 (bEnabled, fp);
}

// ------------------------------------------------------------------------

int CLightningInfo::Read (FILE *fp, int version)
{
nLife = ReadInt32 (fp);
nDelay = ReadInt32 (fp);
nLength = ReadInt32 (fp);
nAmplitude = ReadInt32 (fp);
nOffset = ReadInt32 (fp);
nLightnings = ReadInt16 (fp);
nId = ReadInt16 (fp);
nTarget = ReadInt16 (fp);
nNodes = ReadInt16 (fp);
nChildren = ReadInt16 (fp);
nSteps = ReadInt16 (fp);
nAngle = ReadInt8 (fp);
nStyle = ReadInt8 (fp);
nSmoothe = ReadInt8 (fp);
bClamp = ReadInt8 (fp);
bPlasma = ReadInt8 (fp);
bSound = ReadInt8 (fp);
bRandom = ReadInt8 (fp);
bInPlane = ReadInt8 (fp);
for (int i = 0; i < 4; i++)
	color [i] = ReadInt8 (fp);
bEnabled = (version < 19) ? 1 : ReadInt8 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CLightningInfo::Write (FILE *fp, int version)
{
WriteInt32 (nLife, fp);
WriteInt32 (nDelay, fp);
WriteInt32 (nLength, fp);
WriteInt32 (nAmplitude, fp);
WriteInt32 (nOffset, fp);
WriteInt16 (nLightnings, fp);
WriteInt16 (nId, fp);
WriteInt16 (nTarget, fp);
WriteInt16 (nNodes, fp);
WriteInt16 (nChildren, fp);
WriteInt16 (nSteps, fp);
WriteInt8 (nAngle, fp);
WriteInt8 (nStyle, fp);
WriteInt8 (nSmoothe, fp);
WriteInt8 (bClamp, fp);
WriteInt8 (bPlasma, fp);
WriteInt8 (bSound, fp);
WriteInt8 (bRandom, fp);
WriteInt8 (bInPlane, fp);
for (int i = 0; i < 4; i++)
	WriteInt8 (color [i], fp);
WriteInt8 (bEnabled, fp);
}

// ------------------------------------------------------------------------

int CSoundInfo::Read (FILE *fp, int version)
{
fread (szFilename, 1, sizeof (szFilename), fp);
nVolume = ReadInt32 (fp);
bEnabled = (version < 19) ? 1 : ReadInt8 (fp);
return 1;
}
// ------------------------------------------------------------------------

void CSoundInfo::Write (FILE *fp, int version)
{
fwrite (szFilename, 1, sizeof (szFilename), fp);
WriteInt32 (nVolume, fp);
WriteInt8 (bEnabled, fp);
}
// ------------------------------------------------------------------------

int CGameObject::Read (FILE *fp, int version, bool bFlag) 
{
m_info.type = ReadInt8 (fp);
m_info.id = ReadInt8 (fp);
m_info.controlType = ReadInt8 (fp);
m_info.movementType = ReadInt8 (fp);
m_info.renderType = ReadInt8 (fp);
m_info.flags = ReadInt8 (fp);
m_info.multiplayer = (version > 37) ? ReadInt8 (fp) : 0;
m_info.nSegment = ReadInt16 (fp);
m_location.pos.Read (fp);
m_location.orient.Read (fp);
m_info.size = ReadFix (fp);
m_info.shields = ReadFix (fp);
m_location.lastPos.Read (fp);
m_info.contents.type = ReadInt8 (fp);
m_info.contents.id = ReadInt8 (fp);
m_info.contents.count = ReadInt8 (fp);

switch (m_info.movementType) {
	case MT_PHYSICS:
		mType.physInfo.Read (fp, version);
		break;
	case MT_SPINNING:
		mType.spinRate.Read (fp);
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
		rType.vClipInfo.Read (fp, theApp.LevelVersion ());
		break;
	case RT_LASER:
		break;
	case RT_SMOKE:
		rType.smokeInfo.Read (fp, theApp.LevelVersion ());
		break;
	case RT_LIGHTNING:
		rType.lightningInfo.Read (fp, theApp.LevelVersion ());
		break;
	case RT_SOUND:
		rType.soundInfo.Read (fp, theApp.LevelVersion ());
		break;
	default:
	break;
	}

return 1;
}

// ------------------------------------------------------------------------
// WriteObject ()
// ------------------------------------------------------------------------

void CGameObject::Write (FILE *fp, int version, bool bFlag)
{
if (theMine->IsStdLevel () && (m_info.type >= OBJ_CAMBOT))
	return;	// not a d2x-xl level, but a d2x-xl object

WriteInt8 (m_info.type, fp);
WriteInt8 (m_info.id, fp);
WriteInt8 (m_info.controlType, fp);
WriteInt8 (m_info.movementType, fp);
WriteInt8 (m_info.renderType, fp);
WriteInt8 (m_info.flags, fp);
WriteInt8 (m_info.multiplayer, fp);
WriteInt16 (m_info.nSegment, fp);
m_location.pos.Write (fp);
m_location.orient.Write (fp);
WriteFix (m_info.size, fp);
WriteFix (m_info.shields, fp);
m_location.lastPos. Write (fp);
WriteInt8 (m_info.contents.type, fp);
WriteInt8 (m_info.contents.id, fp);
WriteInt8 (m_info.contents.count, fp);

switch (m_info.movementType) {
	case MT_PHYSICS:
		mType.physInfo.Write (fp, version);
		break;
	case MT_SPINNING:
		mType.spinRate.Write (fp);
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