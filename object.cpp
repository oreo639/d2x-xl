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
	CGameObject*	objP = Objects ();

for (INT16 i = ObjCount (); i; i--, objP++)
	if (objP->signature == signature)
		return INT16 (objP - Objects ());
return -1;
}


void CMine::RenumberTriggerTargetObjs (void)
{
	CTrigger*	trigP = Triggers ();

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
	CTrigger*	trigP = ObjTriggers ();
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
	CGameObject	median = *Objects ((left + right) / 2);
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

if (m_bSortObjects && ((i = GameInfo ().objects.count) > 1)) {
	for (j = 0; j < i; j++)
		Objects (j)->signature = j;
	QSortObjects (0, i - 1);
	RenumberObjTriggers ();
	RenumberTriggerTargetObjs ();
	}
}

//------------------------------------------------------------------------
// make_object()
//
// Action - Defines a standard object (currently assumed to be a player)
//------------------------------------------------------------------------

void CMine::MakeObject (CGameObject *objP, INT8 type, INT16 nSegment) 
{
  tFixVector location;

	theApp.SetModified (TRUE);
	theApp.LockUndo ();
  CalcSegCenter (location,nSegment);
  memset(objP,0,sizeof (CGameObject));
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
  objP->nSegment        = Current ()->nSegment;
  objP->pos.x         = location.x;
  objP->pos.y         = location.y;
  objP->pos.z         = location.z;
  objP->orient.rvec.x = F1_0;
  objP->orient.rvec.y = 0;
  objP->orient.rvec.z = 0;
  objP->orient.uvec.x = 0;
  objP->orient.uvec.y = F1_0;
  objP->orient.uvec.z = 0;
  objP->orient.fvec.x = 0;
  objP->orient.fvec.y = 0;
  objP->orient.fvec.z = F1_0;
  objP->size          = PLAYER_SIZE;
  objP->shields       = DEFAULT_SHIELD;
  objP->rtype.pobj_info.model_num = PLAYER_CLIP_NUMBER;
  objP->rtype.pobj_info.tmap_override = -1;
  objP->contains_type = 0;
  objP->contains_id = 0;
  objP->contains_count = 0;
	theApp.UnlockUndo ();
  return;
}

//------------------------------------------------------------------------
// set_object_data()
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
objP = Objects () + Current ()->nObject;
id = objP->id;
memset(&objP->mtype, 0, sizeof (objP->mtype));
memset(&objP->ctype, 0, sizeof (objP->ctype));
memset(&objP->rtype, 0, sizeof (objP->rtype));
switch (type) {
	case OBJ_ROBOT    : // an evil enemy
	  objP->control_type  = CT_AI;
	  objP->movement_type = MT_PHYSICS;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = robot_size[id];
	  objP->shields       = robot_shield[id];
	  objP->rtype.pobj_info.model_num = robot_clip[id];
	  objP->rtype.pobj_info.tmap_override = -1; // set texture to none
	  objP->ctype.ai_info.behavior = AIB_NORMAL;
	  break;

	case OBJ_HOSTAGE  : // a hostage you need to rescue
	  objP->control_type = CT_POWERUP;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_HOSTAGE;
	  objP->rtype.vclip_info.vclip_num = HOSTAGE_CLIP_NUMBER;
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
	  objP->rtype.pobj_info.model_num = PLAYER_CLIP_NUMBER;
	  objP->rtype.pobj_info.tmap_override = -1; // set texture to normal
	  break;

	case OBJ_WEAPON   : // a poly-type weapon
	  objP->control_type  = CT_WEAPON;
	  objP->movement_type = MT_PHYSICS;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = WEAPON_SIZE;
	  objP->shields       = WEAPON_SHIELD;
	  objP->rtype.pobj_info.model_num = MINE_CLIP_NUMBER;
	  objP->rtype.pobj_info.tmap_override = -1; // set texture to normal
	  objP->mtype.phys_info.mass      = 65536L;
	  objP->mtype.phys_info.drag      = 2162;
	  objP->mtype.phys_info.rotvel.x  = 0;
	  objP->mtype.phys_info.rotvel.y  = 46482L;  // don't know exactly what to put here
	  objP->mtype.phys_info.rotvel.z  = 0;
	  objP->mtype.phys_info.flags     = 260;
	  objP->ctype.laser_info.parent_type      = 5;
	  objP->ctype.laser_info.parent_num       = 146; // don't know exactly what to put here
	  objP->ctype.laser_info.parent_signature = 146; // don't know exactly what to put here
	  break;

	case OBJ_POWERUP  : // a powerup you can pick up
	  objP->control_type  = CT_POWERUP;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_POWERUP;
	  objP->rtype.vclip_info.vclip_num = powerup_clip[id];
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
			objP->rtype.pobj_info.model_num = REACTOR_CLIP_NUMBER;
	  else {
		INT32 model;
		switch(id) {
		  case 1:  model = 95;  break;
		  case 2:  model = 97;  break;
		  case 3:  model = 99;  break;
		  case 4:  model = 101; break;
		  case 5:  model = 103; break;
		  case 6:  model = 105; break;
		  default: model = 97;  break; // level 1's reactor
		}
		objP->rtype.pobj_info.model_num = model;
	  }
	  objP->rtype.pobj_info.tmap_override = -1; // set texture to none
	  break;

	case OBJ_COOP     : // a cooperative player object
	  objP->control_type = CT_NONE;
	  objP->movement_type = MT_PHYSICS;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = PLAYER_SIZE;
	  objP->shields       = DEFAULT_SHIELD;
	  objP->rtype.pobj_info.model_num = COOP_CLIP_NUMBER;
	  objP->rtype.pobj_info.tmap_override = -1; // set texture to none
	  break;

	case OBJ_CAMBOT:
	case OBJ_SMOKE:
	case OBJ_MONSTERBALL:
	  objP->control_type  = CT_AI;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_POLYOBJ;
	  objP->size          = robot_size[0];
	  objP->shields       = DEFAULT_SHIELD;
	  objP->rtype.pobj_info.model_num = robot_clip [0];
	  objP->rtype.pobj_info.tmap_override = -1; // set texture to none
	  objP->ctype.ai_info.behavior = AIB_STILL;
	  break;

	case OBJ_EXPLOSION:
	  objP->control_type  = CT_POWERUP;
	  objP->movement_type = MT_NONE;
	  objP->render_type   = RT_POWERUP;
	  objP->size          = robot_size[0];
	  objP->shields       = DEFAULT_SHIELD;
	  objP->rtype.vclip_info.vclip_num = VCLIP_BIG_EXPLOSION;
	  objP->rtype.pobj_info.tmap_override = -1; // set texture to none
	  objP->ctype.ai_info.behavior = AIB_STILL;
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
// copy_object()
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

if (GameInfo ().objects.count >= MAX_OBJECTS (this)) {
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
	objP = Objects ();
	for (objnum = GameInfo ().objects.count; objnum; objnum--, objP++)
		if (objP->type == type) {
			id = objP->id;
			if (id >= 0 && id < (MAX_PLAYERS (this) + MAX_COOP_PLAYERS))
				ids[id]++;
			}
	if (type == OBJ_PLAYER) {
		for (id = 0; (id <= MAX_PLAYERS (this)) && ids[id]; id++)
				;// loop until 1st id with 0
		if (id > MAX_PLAYERS (this)) {
				char szMsg [80];

			sprintf_s (szMsg, sizeof (szMsg), "There are already %d players in the mine", MAX_PLAYERS (this));
			ErrorMsg (szMsg);
			return FALSE;
			}
		}
	else {
		for (id = MAX_PLAYERS (this); (id < MAX_PLAYERS (this) + MAX_COOP_PLAYERS) && ids[id]; id++)
			;// loop until 1st id with 0
		if (id > MAX_PLAYERS (this) + MAX_COOP_PLAYERS) {
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
	MakeObject (Objects (), OBJ_PLAYER, (nSegment < 0) ? Current ()->nSegment : nSegment);
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
objP->last_pos.x = objP->pos.x;
objP->last_pos.y = objP->pos.y;
objP->last_pos.z = objP->pos.z;
Current ()->nObject = objnum;
// bump position over if this is not the first object in the cube
count = 0;
for (i = 0; i < GameInfo ().objects.count - 1; i++)
	if (Objects (i)->nSegment == Current ()->nSegment)
		count++;
objP->pos.y += count*2*F1_0;
objP->last_pos.y += count*2*F1_0;
// set the id if new object is a player or a coop
if (type == OBJ_PLAYER || type == OBJ_COOP)
	objP->id = (INT8) id;
// set object data if new object being added
if (new_type != OBJ_NONE) {
	objP->type = new_type;
	SetObjectData(objP->type);
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
// DeleteObject()
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
		switch(id) {
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
		if ((id >= 0) && (id < MAX_POWERUP_IDS) && (powerup_lookup[id] >= 0))
			object_number = 76 + powerup_lookup[id];
		break;
	default:
		object_number = -1; // undefined
}

CDC *pDC = pWnd->GetDC ();
CRect rc;
pWnd->GetClientRect (rc);
pDC->FillSolidRect (&rc, IMG_BKCOLOR);
if ((object_number >= 0) && (object_number <= 129)) {
	sprintf_s (message, sizeof (message),"OBJ_%03d_BMP", object_number);
	HINSTANCE hInst = AfxGetApp ()->m_hInstance;
	HRSRC hFind = FindResource (hInst, message, RT_BITMAP);
	HGLOBAL hGlobal = LoadResource (hInst, hFind);
	char *pRes = (char *)LockResource(hGlobal);
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
		SetDIBitsToDevice(pDC->m_hDC, xoffset,yoffset,width,height,0,0,
								0, height,pImage, bmi, DIB_RGB_COLORS);
		}
	FreeResource (hGlobal);
	}
pWnd->ReleaseDC (pDC);
pWnd->InvalidateRect (NULL, TRUE);
pWnd->UpdateWindow ();
}

// ------------------------------------------------------------------------

void CGameObject::Read (FILE *fp, INT32 version) 
{
	INT32 i, levelVersion = theApp.LevelVersion ();

	type = read_INT8(fp);
	id = read_INT8(fp);
	control_type = read_INT8(fp);
	movement_type = read_INT8(fp);
	render_type = read_INT8(fp);
	flags = read_INT8(fp);
	if (version > 37)
		multiplayer = read_INT8(fp);
	else
		multiplayer = 0;
	nSegment = read_INT16(fp);
	read_vector(&pos, fp);
	read_matrix(&orient, fp);
	size = read_FIX(fp);
	shields = read_FIX(fp);
	read_vector(&last_pos, fp);
	contains_type = read_INT8(fp);
	contains_id = read_INT8(fp);
	contains_count = read_INT8(fp);

	switch (movement_type) {
    case MT_PHYSICS:
		read_vector(&mtype.phys_info.velocity, fp);
		read_vector(&mtype.phys_info.thrust, fp);
		mtype.phys_info.mass = read_FIX(fp);
		mtype.phys_info.drag = read_FIX(fp);
		mtype.phys_info.brakes = read_FIX(fp);
		read_vector(&mtype.phys_info.rotvel, fp);
		read_vector(&mtype.phys_info.rotthrust, fp);
		mtype.phys_info.turnroll = read_FIXANG(fp);
		mtype.phys_info.flags = read_INT16(fp);
		break;

    case MT_SPINNING:
		read_vector(&mtype.spin_rate, fp);
		break;

    case MT_NONE:
		break;

    default:
		break;
	}

	switch (control_type) {
    case CT_AI: {
		INT16 i;
		ctype.ai_info.behavior = read_INT8(fp);
		for (i = 0; i < MAX_AI_FLAGS; i++) {
			ctype.ai_info.flags [i] = read_INT8(fp);
		}
		ctype.ai_info.hide_segment = read_INT16(fp);
		ctype.ai_info.hide_index = read_INT16(fp);
		ctype.ai_info.path_length = read_INT16(fp);
		ctype.ai_info.cur_path_index = read_INT16(fp);
		if (theApp.IsD1File ()) {
			ctype.ai_info.follow_path_start_seg = read_INT16(fp);
			ctype.ai_info.follow_path_end_seg = read_INT16(fp);
		}
		break;
				}
    case CT_EXPLOSION:
		ctype.expl_info.spawn_time = read_FIX(fp);
		ctype.expl_info.delete_time = read_FIX(fp);
		ctype.expl_info.delete_objnum = (UINT8)read_INT16(fp);
		ctype.expl_info.next_attach = ctype.expl_info.prev_attach = ctype.expl_info.attach_parent =-1;
		break;

    case CT_WEAPON:
		ctype.laser_info.parent_type = read_INT16(fp);
		ctype.laser_info.parent_num = read_INT16(fp);
		ctype.laser_info.parent_signature = read_INT32(fp);
		break;

    case CT_LIGHT:
		ctype.light_info.intensity = read_FIX(fp);
		break;

    case CT_POWERUP:
		if (version >= 25) {
			ctype.powerup_info.count = read_INT32(fp);
		} else {
			ctype.powerup_info.count = 1;
			//      if (id== POW_VULCAN_WEAPON)
			//          ctype.powerup_info.count = VULCAN_WEAPON_AMMO_AMOUNT;
		}
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
    case RT_POLYOBJ: {
		INT16 i;
		INT32 tmo;
		rtype.pobj_info.model_num = read_INT32(fp);
		for (i = 0; i < MAX_SUBMODELS; i++) {
			read_angvec(&rtype.pobj_info.anim_angles [i], fp);
		}
		rtype.pobj_info.subobj_flags = read_INT32(fp);
		tmo = read_INT32(fp);
		rtype.pobj_info.tmap_override = tmo;
		rtype.pobj_info.alt_textures = 0;
		break;
					 }

    case RT_WEAPON_VCLIP:
    case RT_HOSTAGE:
    case RT_POWERUP:
    case RT_FIREBALL:
		rtype.vclip_info.vclip_num = read_INT32(fp);
		rtype.vclip_info.frametime = read_FIX(fp);
		rtype.vclip_info.framenum = read_INT8(fp);

		break;

    case RT_LASER:
		break;

	case RT_SMOKE:
		rtype.smokeInfo.nLife = read_INT32 (fp);
		rtype.smokeInfo.nSize [0] = read_INT32 (fp);
		rtype.smokeInfo.nParts = read_INT32 (fp);
		rtype.smokeInfo.nSpeed = read_INT32 (fp);
		rtype.smokeInfo.nDrift = read_INT32 (fp);
		rtype.smokeInfo.nBrightness = read_INT32 (fp);
		for (i = 0; i < 4; i++)
			rtype.smokeInfo.color [i] = read_INT8 (fp);
		rtype.smokeInfo.nSide = read_INT8 (fp);
		rtype.smokeInfo.nType = (levelVersion < 18) ? 0 : read_INT8 (fp);
		rtype.smokeInfo.bEnabled = (levelVersion < 19) ? 1 : read_INT8 (fp);
		break;

	case RT_LIGHTNING:
		rtype.lightningInfo.nLife = read_INT32 (fp);
		rtype.lightningInfo.nDelay = read_INT32 (fp);
		rtype.lightningInfo.nLength = read_INT32 (fp);
		rtype.lightningInfo.nAmplitude = read_INT32 (fp);
		rtype.lightningInfo.nOffset = read_INT32 (fp);
		rtype.lightningInfo.nLightnings = read_INT16 (fp);
		rtype.lightningInfo.nId = read_INT16 (fp);
		rtype.lightningInfo.nTarget = read_INT16 (fp);
		rtype.lightningInfo.nNodes = read_INT16 (fp);
		rtype.lightningInfo.nChildren = read_INT16 (fp);
		rtype.lightningInfo.nSteps = read_INT16 (fp);
		rtype.lightningInfo.nAngle = read_INT8 (fp);
		rtype.lightningInfo.nStyle = read_INT8 (fp);
		rtype.lightningInfo.nSmoothe = read_INT8 (fp);
		rtype.lightningInfo.bClamp = read_INT8 (fp);
		rtype.lightningInfo.bPlasma = read_INT8 (fp);
		rtype.lightningInfo.bSound = read_INT8 (fp);
		rtype.lightningInfo.bRandom = read_INT8 (fp);
		rtype.lightningInfo.bInPlane = read_INT8 (fp);
		for (i = 0; i < 4; i++)
			rtype.lightningInfo.color [i] = read_INT8 (fp);
		rtype.lightningInfo.bEnabled = (levelVersion < 19) ? 1 : read_INT8 (fp);
		break;

	case RT_SOUND:
		fread (rtype.soundInfo.szFilename, 1, sizeof (rtype.soundInfo.szFilename), fp);
		rtype.soundInfo.nVolume = read_INT32 (fp);
		rtype.soundInfo.bEnabled = (levelVersion < 19) ? 1 : read_INT8 (fp);
		break;

	default:
		break;
	}
}

// ------------------------------------------------------------------------
// WriteObject()
// ------------------------------------------------------------------------

void CGameObject::Write (FILE *fp, INT32 version)
{
if (theApp.GetMine()->IsStdLevel () && (type >= OBJ_CAMBOT))
	return;	// not a d2x-xl level, but a d2x-xl object

	INT32 i;
	write_INT8(type, fp);
	write_INT8(id, fp);
	write_INT8(control_type, fp);
	write_INT8(movement_type, fp);
	write_INT8(render_type, fp);
	write_INT8(flags, fp);
	if (version > 36)
		write_INT8(multiplayer, fp);
	write_INT16(nSegment, fp);
	write_vector(&pos, fp);
	write_matrix(&orient, fp);
	write_FIX(size, fp);
	write_FIX(shields, fp);
	write_vector(&last_pos, fp);
	write_INT8(contains_type, fp);
	write_INT8(contains_id, fp);
	write_INT8(contains_count, fp);

	switch (movement_type) {
    case MT_PHYSICS:
		write_vector(&mtype.phys_info.velocity, fp);
		write_vector(&mtype.phys_info.thrust, fp);
		write_FIX(mtype.phys_info.mass, fp);
		write_FIX(mtype.phys_info.drag, fp);
		write_FIX(mtype.phys_info.brakes, fp);
		write_vector(&mtype.phys_info.rotvel, fp);
		write_vector(&mtype.phys_info.rotthrust, fp);
		write_FIXANG(mtype.phys_info.turnroll, fp);
		write_INT16(mtype.phys_info.flags, fp);
		break;

    case MT_SPINNING:
		write_vector(&mtype.spin_rate, fp);
		break;

    case MT_NONE:
		break;

    default:
		break;
	}

	switch (control_type) {
	case CT_AI: {
		INT16 i;
		write_INT8(ctype.ai_info.behavior, fp);
		for (i = 0; i < MAX_AI_FLAGS; i++)
			write_INT8(ctype.ai_info.flags [i], fp);
		write_INT16(ctype.ai_info.hide_segment, fp);
		write_INT16(ctype.ai_info.hide_index, fp);
		write_INT16(ctype.ai_info.path_length, fp);
		write_INT16(ctype.ai_info.cur_path_index, fp);
		if (theApp.GetMine()->IsD1File ()) {
			write_INT16(ctype.ai_info.follow_path_start_seg, fp);
			write_INT16(ctype.ai_info.follow_path_end_seg, fp);
		}
		break;
				}
    case CT_EXPLOSION:
		write_FIX(ctype.expl_info.spawn_time, fp);
		write_FIX(ctype.expl_info.delete_time, fp);
		write_INT16(ctype.expl_info.delete_objnum, fp);
		break;

    case CT_WEAPON:
		write_INT16(ctype.laser_info.parent_type, fp);
		write_INT16(ctype.laser_info.parent_num, fp);
		write_INT32 (ctype.laser_info.parent_signature, fp);
		break;

    case CT_LIGHT:
		write_FIX(ctype.light_info.intensity, fp);
		break;

    case CT_POWERUP:
		if (version >= 25) {
			write_INT32 (ctype.powerup_info.count, fp);
		}
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
    case RT_POLYOBJ: {
		INT16 i;
		INT32 tmo;

		write_INT32 (rtype.pobj_info.model_num, fp);
		for (i = 0; i < MAX_SUBMODELS; i++) {
			write_angvec(&rtype.pobj_info.anim_angles [i], fp);
		}
		write_INT32 (rtype.pobj_info.subobj_flags, fp);
		tmo = rtype.pobj_info.tmap_override;
		write_INT32 (tmo, fp);
		break;
					 }
	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
		write_INT32 (rtype.vclip_info.vclip_num, fp);
		write_FIX(rtype.vclip_info.frametime, fp);
		write_INT8(rtype.vclip_info.framenum, fp);
		break;

	case RT_LASER:
		break;

	case RT_SMOKE:
		write_INT32 (rtype.smokeInfo.nLife, fp);
		write_INT32 (rtype.smokeInfo.nSize [0], fp);
		write_INT32 (rtype.smokeInfo.nParts, fp);
		write_INT32 (rtype.smokeInfo.nSpeed, fp);
		write_INT32 (rtype.smokeInfo.nDrift, fp);
		write_INT32 (rtype.smokeInfo.nBrightness, fp);
		for (i = 0; i < 4; i++)
			write_INT8 (rtype.smokeInfo.color [i], fp);
		write_INT8 (rtype.smokeInfo.nSide, fp);
		write_INT8 (rtype.smokeInfo.nType, fp);
		write_INT8 (rtype.smokeInfo.bEnabled, fp);
		break;

	case RT_LIGHTNING:
		write_INT32 (rtype.lightningInfo.nLife, fp);
		write_INT32 (rtype.lightningInfo.nDelay, fp);
		write_INT32 (rtype.lightningInfo.nLength, fp);
		write_INT32 (rtype.lightningInfo.nAmplitude, fp);
		write_INT32 (rtype.lightningInfo.nOffset, fp);
		write_INT16 (rtype.lightningInfo.nLightnings, fp);
		write_INT16 (rtype.lightningInfo.nId, fp);
		write_INT16 (rtype.lightningInfo.nTarget, fp);
		write_INT16 (rtype.lightningInfo.nNodes, fp);
		write_INT16 (rtype.lightningInfo.nChildren, fp);
		write_INT16 (rtype.lightningInfo.nSteps, fp);
		write_INT8 (rtype.lightningInfo.nAngle, fp);
		write_INT8 (rtype.lightningInfo.nStyle, fp);
		write_INT8 (rtype.lightningInfo.nSmoothe, fp);
		write_INT8 (rtype.lightningInfo.bClamp, fp);
		write_INT8 (rtype.lightningInfo.bPlasma, fp);
		write_INT8 (rtype.lightningInfo.bSound, fp);
		write_INT8 (rtype.lightningInfo.bRandom, fp);
		write_INT8 (rtype.lightningInfo.bInPlane, fp);
		for (i = 0; i < 4; i++)
			write_INT8 (rtype.lightningInfo.color [i], fp);
		write_INT8 (rtype.lightningInfo.bEnabled, fp);
		break;

	case RT_SOUND:
		fwrite (rtype.soundInfo.szFilename, 1, sizeof (rtype.soundInfo.szFilename), fp);
		write_INT32 (rtype.soundInfo.nVolume, fp);
		write_INT8 (rtype.soundInfo.bEnabled, fp);
		break;

	default:
		break;

	}
}

// ------------------------------------------------------------------------
// eof object.cpp