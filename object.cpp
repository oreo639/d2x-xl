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

static INT16 sortObjType [MAX_OBJECT_TYPES] = {7, 8, 5, 4, 0, 2, 9, 3, 10, 6, 11, 12, 13, 14, 1, 16, 15, 17, 18, 19, 20};


INT32 CMine::QCmpObjects (CGameObject *pi, CGameObject *pm)
{
	INT16 ti = sortObjType [pi->type];
	INT16 tm = sortObjType [pm->type];
if (ti < tm)
	return -1;
if (ti > tm)
	return 1;
return (pi->id < pm->id) ? -1 : (pi->id > pm->id) ? 1 : 0;
}


INT16 CMine::FindObjBySig (INT16 signature)
{
	CGameObject*	objP = Objects (0);

for (INT16 i = ObjCount (); i; i--, objP++)
	if (objP->signature == signature)
		return INT16 (objP - Objects (0));
return -1;
}


void CMine::RenumberTriggerTargetObjs (void)
{
	CTrigger*	trigP = Triggers (0);

for (INT32 i = TriggerCount (); i; i--, trigP++) {
	for (INT32 j = 0; j < trigP->m_count; ) {
		if (trigP->Side (j) >= 0) 
			j++;
		else {
			INT32 h = FindObjBySig (trigP->Segment (j));
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
	INT32			i;

for (i = NumObjTriggers (); i; i--, trigP++)
	trigP->nObject = FindObjBySig (trigP->nObject);
i = NumObjTriggers ();
while (i) {
	if (ObjTriggers (--i)->nObject < 0)
		DeleteObjTrigger (i);
	}
SortObjTriggers ();
}


void CMine::QSortObjects (INT16 left, INT16 right)
{
	CGameObject	median = *Objects ( (left + right) / 2);
	INT16	l = left, r = right;

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
	INT32	i, j;

if (m_bSortObjects && ( (i = GameInfo ().objects.count) > 1)) {
	for (j = 0; j < i; j++)
		Objects (j)->signature = j;
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

void CMine::MakeObject (CGameObject *objP, INT8 type, INT16 nSegment) 
{
  CFixVector location;

	theApp.SetModified (TRUE);
	theApp.LockUndo ();
  CalcSegCenter (location,nSegment);
  memset (objP,0,sizeof (CGameObject));
  objP->signature = 0;
  objP->type = type;
  if (type==OBJ_WEAPON) {
	objP->id = SMALLMINE_ID;
  } else {
	objP->id = 0;
  }
  objP->control_type = CT_NONE; /* player 0 only */
  objP->movement_type = MT_PHYSICS;
  objP->render_type   = RT_POLYOBJ;
  objP->flags         = 0;
  objP->nSegment      = Current ()->nSegment;
  objP->pos				 = location;
  objP->orient.rVec.Set (F1_0, 0, 0);
  objP->orient.uVec.Set (0, F1_0, 0);
  objP->orient.fVec.Set (0, 0, F1_0);
  objP->size          = PLAYER_SIZE;
  objP->shields       = DEFAULT_SHIELD;
  objP->rType.polyModelInfo.model_num = PLAYER_CLIP_NUMBER;
  objP->rType.polyModelInfo.tmap_override = -1;
  objP->contains_type = 0;
  objP->contains_id = 0;
  objP->contains_count = 0;
	theApp.UnlockUndo ();
  return;
}

//------------------------------------------------------------------------
// set_object_data ()
//
// Action - Sets control type, movement type, render type
// 	    size, and shields (also model_num & texture if robot)
//------------------------------------------------------------------------

void CMine::SetObjectData (INT8 type) 
{
  CGameObject *objP;
  INT32  id;

theApp.SetModified (TRUE);
theApp.LockUndo ();
objP = Objects (0) + Current ()->nObject;
id = objP->id;
memset (&objP->mType, 0, sizeof (objP->mType));
memset (&objP->cType, 0, sizeof (objP->cType));
memset (&objP->rType, 0, sizeof (objP->rType));
switch (type) {
	case OBJ_ROBOT: // an evil enemy
	  objP->control_type  = CT_AI;
	  objP->movement_type = MT_PHYSICS;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = robot_size[id];
	  objP->shields       = robot_shield[id];
	  objP->rType.polyModelInfo.model_num = robot_clip[id];
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  objP->cType.aiInfo.behavior = AIB_NORMAL;
	  break;

	case OBJ_HOSTAGE  : // a hostage you need to rescue
	  objP->control_type = CT_POWERUP;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_HOSTAGE;
	  objP->rType.vClipInfo.vclip_num = HOSTAGE_CLIP_NUMBER;
	  objP->size          = PLAYER_SIZE;
	  objP->shields       = DEFAULT_SHIELD;
	  break;

	case OBJ_PLAYER   : // the player on the console
	  if (objP->id == 0) {
		objP->control_type = CT_NONE; /* player 0 only */
	  } else {
		objP->control_type = CT_SLEW; /* all other players */
	  }
	  objP->movement_type = MT_PHYSICS;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = PLAYER_SIZE;
	  objP->shields       = DEFAULT_SHIELD;
	  objP->rType.polyModelInfo.model_num = PLAYER_CLIP_NUMBER;
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to normal
	  break;

	case OBJ_WEAPON   : // a poly-type weapon
	  objP->control_type  = CT_WEAPON;
	  objP->movement_type = MT_PHYSICS;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = WEAPON_SIZE;
	  objP->shields       = WEAPON_SHIELD;
	  objP->rType.polyModelInfo.model_num = MINE_CLIP_NUMBER;
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
	  objP->control_type  = CT_POWERUP;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_POWERUP;
	  objP->rType.vClipInfo.vclip_num = powerup_clip[id];
	  objP->size          = powerup_size[id];
	  objP->shields       = DEFAULT_SHIELD;
	  break;

	case OBJ_CNTRLCEN : // the reactor
	  objP->control_type = CT_CNTRLCEN;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = REACTOR_SIZE;
	  objP->shields       = REACTOR_SHIELD;
	  if (IsD1File ())
			objP->rType.polyModelInfo.model_num = REACTOR_CLIP_NUMBER;
	  else {
		INT32 model;
		switch (id) {
		  case 1:  model = 95;  break;
		  case 2:  model = 97;  break;
		  case 3:  model = 99;  break;
		  case 4:  model = 101; break;
		  case 5:  model = 103; break;
		  case 6:  model = 105; break;
		  default: model = 97;  break; // level 1's reactor
		}
		objP->rType.polyModelInfo.model_num = model;
	  }
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  break;

	case OBJ_COOP     : // a cooperative player object
	  objP->control_type = CT_NONE;
	  objP->movement_type = MT_PHYSICS;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = PLAYER_SIZE;
	  objP->shields       = DEFAULT_SHIELD;
	  objP->rType.polyModelInfo.model_num = COOP_CLIP_NUMBER;
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  break;

	case OBJ_CAMBOT:
	case OBJ_SMOKE:
	case OBJ_MONSTERBALL:
	  objP->control_type  = CT_AI;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = robot_size[0];
	  objP->shields       = DEFAULT_SHIELD;
	  objP->rType.polyModelInfo.model_num = robot_clip [0];
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  objP->cType.aiInfo.behavior = AIB_STILL;
	  break;

	case OBJ_EXPLOSION:
	  objP->control_type  = CT_POWERUP;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_POWERUP;
	  objP->size          = robot_size[0];
	  objP->shields       = DEFAULT_SHIELD;
	  objP->rType.vClipInfo.vclip_num = VCLIP_BIG_EXPLOSION;
	  objP->rType.polyModelInfo.tmap_override = -1; // set texture to none
	  objP->cType.aiInfo.behavior = AIB_STILL;
	  break;

	case OBJ_EFFECT:
	  objP->control_type  = CT_NONE;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_NONE;
	  objP->size          = f1_0;
	  objP->shields       = DEFAULT_SHIELD;

  }
	theApp.UnlockUndo ();
}

//------------------------------------------------------------------------
// copy_object ()
//
// Action - Copies the current object to a new object
//          If object is a player or coop, it chooses the next available id
//
// Parameters - INT8 new_type = new type of object
//
// Returns - TRUE upon success
//
//------------------------------------------------------------------------

bool CMine::CopyObject (UINT8 new_type, INT16 nSegment) 
{
	INT16 objnum,id;
	INT16 ids [MAX_PLAYERS_D2X + MAX_COOP_PLAYERS] = {0,0,0,0,0,0,0,0,0,0,0};
	CGameObject *objP,*current_obj;
	UINT8 type;
	INT16 i,count;

if (GameInfo ().objects.count >= MAX_OBJECTS) {
	ErrorMsg ("The maximum number of objects has already been reached.");
	return false;
	}
// If OBJ_NONE used, then make a copy of the current object type
// otherwise use the type passed as the parameter "new_type"
//--------------------------------------------------------------

type = (new_type == OBJ_NONE) ? CurrObj ()->type : new_type;

// Make sure it is ok to add another object of this type
// Set the id if it's a player or coop
//------------------------------------------------------
if (type == OBJ_PLAYER || type == OBJ_COOP) {
	objP = Objects (0);
	for (objnum = GameInfo ().objects.count; objnum; objnum--, objP++)
		if (objP->type == type) {
			id = objP->id;
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
objP->flags = 0;                                      // new: 1/27/97
objP->nSegment = Current ()->nSegment;
// set object position in the center of the cube for now
CalcSegCenter (objP->pos,Current ()->nSegment);
objP->last_pos = objP->pos;
Current ()->nObject = objnum;
// bump position over if this is not the first object in the cube
count = 0;
for (i = 0; i < GameInfo ().objects.count - 1; i++)
	if (Objects (i)->nSegment == Current ()->nSegment)
		count++;
objP->pos.v.y += count * 2 * F1_0;
objP->last_pos.v.y += count * 2 * F1_0;
// set the id if new object is a player or a coop
if (type == OBJ_PLAYER || type == OBJ_COOP)
	objP->id = (INT8) id;
// set object data if new object being added
if (new_type != OBJ_NONE) {
	objP->type = new_type;
	SetObjectData (objP->type);
	}
// set the contents to zero
objP->contains_type = 0;
objP->contains_id = 0;
objP->contains_count = 0;
SortObjects ();
theApp.MineView ()->Refresh (false);
theApp.ToolView ()->ObjectTool ()->Refresh ();
theApp.UnlockUndo ();
return TRUE;
}

//------------------------------------------------------------------------
// DeleteObject ()
//------------------------------------------------------------------------

void CMine::DeleteObject (INT16 nDelObj)
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
INT32 i, j = GameInfo ().objects.count;
for (i = nDelObj; i < j; i++)
	Objects (i)->signature = i;
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

void CMine::DrawObject (CWnd *pWnd, INT32 type, INT32 id)
{
	INT32 powerup_lookup[48] = {
		 0, 1, 2, 3, 4, 5, 6,-1,-1,-1,
		 7, 8, 9,10,11,12,13,14,15,16,
		17,18,19,20,-1,21,-1,-1,22,23,
		24,25,26,27,28,29,30,31,32,33,
		34,35,36,37,38,39,40,41
		};
	INT32 object_number;

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
		INT32 ncolors = (INT32)bmi->bmiHeader.biClrUsed;
		if (ncolors == 0)
			ncolors = 1 << (bmi->bmiHeader.biBitCount); // 256 colors for 8-bit data
		char *pImage = pRes + sizeof (BITMAPINFOHEADER) + ncolors * 4;
		INT32 width = (INT32)bmi->bmiHeader.biWidth;
		INT32 height = (INT32)bmi->bmiHeader.biHeight;
		INT32 xoffset = (64 - width) / 2;
		INT32 yoffset = (64 - height) / 2;
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

INT32 CObjPhysicsInfo::Read (FILE *fp, INT32 version)
{
read_vector (&velocity, fp);
read_vector (&thrust, fp);
mass = read_FIX (fp);
drag = read_FIX (fp);
brakes = read_FIX (fp);
read_vector (&rotvel, fp);
read_vector (&rotthrust, fp);
turnroll = read_FIXANG (fp);
flags = read_INT16 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CObjPhysicsInfo::Write (FILE *fp, INT32 version)
{
velocity.Write (fp);
thrust.Write (fp);
write_FIX (mass, fp);
write_FIX (drag, fp);
write_FIX (brakes, fp);
rotvel.Write (fp);
rotthrust.Write (fp);
write_FIXANG (turnroll, fp);
write_INT16 (flags, fp);
}

// ------------------------------------------------------------------------

INT32 CObjAIInfo::Read (FILE *fp, INT32 version)
{
behavior = read_INT8 (fp);
for (int i = 0; i < MAX_AI_FLAGS; i++)
	flags [i] = read_INT8 (fp);
hide_segment = read_INT16 (fp);
hide_index = read_INT16 (fp);
path_length = read_INT16 (fp);
cur_path_index = read_INT16 (fp);
if (theApp.IsD1File ()) {
	follow_path_start_seg = read_INT16 (fp);
	follow_path_end_seg = read_INT16 (fp);
	}
return 1;
}

// ------------------------------------------------------------------------

void CObjAIInfo::Write (FILE *fp, INT32 version)
{
write_INT8 (behavior, fp);
for (int i = 0; i < MAX_AI_FLAGS; i++)
	write_INT8 (flags [i], fp);
write_INT16 (hide_segment, fp);
write_INT16 (hide_index, fp);
write_INT16 (path_length, fp);
write_INT16 (cur_path_index, fp);
if (theApp.IsD1File ()) {
	write_INT16 (follow_path_start_seg, fp);
	write_INT16 (follow_path_end_seg, fp);
	}
}

// ------------------------------------------------------------------------

INT32 CObjExplosionInfo::Read (FILE *fp, INT32 version)
{
spawn_time = read_FIX (fp);
delete_time = read_FIX (fp);
delete_objnum = (UINT8)read_INT16 (fp);
next_attach = 
prev_attach = 
attach_parent =-1;
return 1;
}

// ------------------------------------------------------------------------

void CObjExplosionInfo::Write (FILE *fp, INT32 version)
{
write_FIX (spawn_time, fp);
write_FIX (delete_time, fp);
write_INT16 (delete_objnum, fp);
}

// ------------------------------------------------------------------------

INT32 CObjLaserInfo::Read (FILE *fp, INT32 version)
{
parent_type = read_INT16 (fp);
parent_num = read_INT16 (fp);
parent_signature = read_INT32 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CObjLaserInfo::Write (FILE *fp, INT32 version)
{
write_INT16 (parent_type, fp);
write_INT16 (parent_num, fp);
write_INT32 (parent_signature, fp);
}

// ------------------------------------------------------------------------

INT32 CObjPowerupInfo::Read (FILE *fp, INT32 version)
{
count = (version >= 25) ? read_INT32 (fp) : 1;
return 1;
}

// ------------------------------------------------------------------------

void CObjPowerupInfo::Write (FILE *fp, INT32 version)
{
if (version >= 25) 
	write_INT32 (count, fp);
}

// ------------------------------------------------------------------------

INT32 CObjLightInfo::Read (FILE *fp, INT32 version)
{
intensity = read_FIX (fp);
return 1;
}

// ------------------------------------------------------------------------

void CObjLightInfo::Write (FILE *fp, INT32 version)
{
write_FIX (intensity, fp);
}

// ------------------------------------------------------------------------

INT32 CObjPolyModelInfo::Read (FILE *fp, INT32 version)
{
model_num = read_INT32 (fp);
for (int i = 0; i < MAX_SUBMODELS; i++)
	anim_angles [i].Read (fp);
subobj_flags = read_INT32 (fp);
tmap_override = read_INT32 (fp);
alt_textures = 0;
return 1;
}

// ------------------------------------------------------------------------

void CObjPolyModelInfo::Write (FILE *fp, INT32 version)
{
write_INT32 (model_num, fp);
for (int i = 0; i < MAX_SUBMODELS; i++)
	anim_angles [i].Write (fp);
write_INT32 (subobj_flags, fp);
write_INT32 (tmap_override, fp);
}

// ------------------------------------------------------------------------

INT32 CObjVClipInfo::Read (FILE *fp, INT32 version)
{
vclip_num = read_INT32 (fp);
frametime = read_FIX (fp);
framenum = read_INT8 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CObjVClipInfo::Write (FILE *fp, INT32 version)
{
write_INT32 (vclip_num, fp);
write_FIX (frametime, fp);
write_INT8 (framenum, fp);
}

// ------------------------------------------------------------------------

INT32 CSmokeInfo::Read (FILE *fp, INT32 version)
{
nLife = read_INT32 (fp);
nSize [0] = read_INT32 (fp);
nParts = read_INT32 (fp);
nSpeed = read_INT32 (fp);
nDrift = read_INT32 (fp);
nBrightness = read_INT32 (fp);
for (int i = 0; i < 4; i++)
	color [i] = read_INT8 (fp);
nSide = read_INT8 (fp);
nType = (version < 18) ? 0 : read_INT8 (fp);
bEnabled = (version < 19) ? 1 : read_INT8 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CSmokeInfo::Write (FILE *fp, INT32 version)
{
write_INT32 (nLife, fp);
write_INT32 (nSize [0], fp);
write_INT32 (nParts, fp);
write_INT32 (nSpeed, fp);
write_INT32 (nDrift, fp);
write_INT32 (nBrightness, fp);
for (int i = 0; i < 4; i++)
	write_INT8 (color [i], fp);
write_INT8 (nSide, fp);
write_INT8 (nSide, fp);
write_INT8 (bEnabled, fp);
}

// ------------------------------------------------------------------------

INT32 CLightningInfo::Read (FILE *fp, INT32 version)
{
nLife = read_INT32 (fp);
nDelay = read_INT32 (fp);
nLength = read_INT32 (fp);
nAmplitude = read_INT32 (fp);
nOffset = read_INT32 (fp);
nLightnings = read_INT16 (fp);
nId = read_INT16 (fp);
nTarget = read_INT16 (fp);
nNodes = read_INT16 (fp);
nChildren = read_INT16 (fp);
nSteps = read_INT16 (fp);
nAngle = read_INT8 (fp);
nStyle = read_INT8 (fp);
nSmoothe = read_INT8 (fp);
bClamp = read_INT8 (fp);
bPlasma = read_INT8 (fp);
bSound = read_INT8 (fp);
bRandom = read_INT8 (fp);
bInPlane = read_INT8 (fp);
for (int i = 0; i < 4; i++)
	color [i] = read_INT8 (fp);
bEnabled = (version < 19) ? 1 : read_INT8 (fp);
return 1;
}

// ------------------------------------------------------------------------

void CLightningInfo::Write (FILE *fp, INT32 version)
{
write_INT32 (nLife, fp);
write_INT32 (nDelay, fp);
write_INT32 (nLength, fp);
write_INT32 (nAmplitude, fp);
write_INT32 (nOffset, fp);
write_INT16 (nLightnings, fp);
write_INT16 (nId, fp);
write_INT16 (nTarget, fp);
write_INT16 (nNodes, fp);
write_INT16 (nChildren, fp);
write_INT16 (nSteps, fp);
write_INT8 (nAngle, fp);
write_INT8 (nStyle, fp);
write_INT8 (nSmoothe, fp);
write_INT8 (bClamp, fp);
write_INT8 (bPlasma, fp);
write_INT8 (bSound, fp);
write_INT8 (bRandom, fp);
write_INT8 (bInPlane, fp);
for (int i = 0; i < 4; i++)
	write_INT8 (color [i], fp);
write_INT8 (bEnabled, fp);
}

// ------------------------------------------------------------------------

INT32 CSoundInfo::Read (FILE *fp, INT32 version)
{
fread (szFilename, 1, sizeof (szFilename), fp);
nVolume = read_INT32 (fp);
bEnabled = (version < 19) ? 1 : read_INT8 (fp);
return 1;
}
// ------------------------------------------------------------------------

void CSoundInfo::Write (FILE *fp, INT32 version)
{
fwrite (szFilename, 1, sizeof (szFilename), fp);
write_INT32 (nVolume, fp);
write_INT8 (bEnabled, fp);
}
// ------------------------------------------------------------------------

INT32 CGameObject::Read (FILE *fp, INT32 version, bool bFlag) 
{
type = read_INT8 (fp);
id = read_INT8 (fp);
control_type = read_INT8 (fp);
movement_type = read_INT8 (fp);
render_type = read_INT8 (fp);
flags = read_INT8 (fp);
multiplayer = (version > 37) ? read_INT8 (fp) : 0;
nSegment = read_INT16 (fp);
read_vector (&pos, fp);
read_matrix (&orient, fp);
size = read_FIX (fp);
shields = read_FIX (fp);
read_vector (&last_pos, fp);
contains_type = read_INT8 (fp);
contains_id = read_INT8 (fp);
contains_count = read_INT8 (fp);

switch (movement_type) {
	case MT_PHYSICS:
		mType.physInfo.Read (fp, version);
		break;
	case MT_SPINNING:
		read_vector (&mType.spinRate, fp);
		break;
	case MT_NONE:
		break;
	default:
		break;
	}

switch (control_type) {
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

switch (render_type) {
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

void CGameObject::Write (FILE *fp, INT32 version, bool bFlag)
{
if (theMine->IsStdLevel () && (type >= OBJ_CAMBOT))
	return;	// not a d2x-xl level, but a d2x-xl object

write_INT8 (type, fp);
write_INT8 (id, fp);
write_INT8 (control_type, fp);
write_INT8 (movement_type, fp);
write_INT8 (render_type, fp);
write_INT8 (flags, fp);
write_INT8 (multiplayer, fp);
write_INT16 (nSegment, fp);
write_vector (&pos, fp);
write_matrix (&orient, fp);
write_FIX (size, fp);
write_FIX (shields, fp);
write_vector (&last_pos, fp);
write_INT8 (contains_type, fp);
write_INT8 (contains_id, fp);
write_INT8 (contains_count, fp);

switch (movement_type) {
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

switch (control_type) {
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

switch (render_type) {
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