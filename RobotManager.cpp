// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <io.h>

#include "define.h"
#include "global.h"
#include "FileManager.h"
#include "Mine.h"
#include "dle-xp.h"
#include "ResourceManager.h"

CRobotManager robotManager;

//------------------------------------------------------------------------------

#define MAKESIG(_sig)	(uint) *((int *) &(_sig))

int CRobotManager::ReadHAM (CFileManager* fp, int type) 
{
  int				t, t0;
  uint			id;
  CPolyModel	pm;
  char			szFile [256];
  CFileManager	fm;

  static char d2HamSig [4] = {'H','A','M','!'};
  static char d2xHamSig [4] = {'M','A','H','X'};

if (fp == null) {
	fp = &fm;
	if (DLE.IsD2File ()) {
		CFileManager::SplitPath (descentPath [1], szFile, null, null);
		strcat_s (szFile, sizeof (szFile), "descent2.ham");
		}
	else {
		CFileManager::SplitPath (descentPath [0], szFile, null, null);
		strcat_s (szFile, sizeof (szFile), "descent.ham");
		}
	if (fp->Open (szFile, "rb")) {
		sprintf_s (message, sizeof (message), " Ham manager: Cannot open robot file <%s>.", szFile);
		DEBUGMSG (message);
		return 1;
		}
	}

pm.m_info.nModels = 0;

// The extended HAM only contains part of the normal HAM file.
// Therefore, we have to skip some items if we are reading
// a normal HAM cause we are only interested in reading
// the information which is found in the extended ham
// (the robot information)
if (type == NORMAL_HAM)  {
	id = fp->ReadInt32 (); // "HAM!"
	if (id != MAKESIG (d2HamSig)) {//0x214d4148L) {
		sprintf_s (message, sizeof (message), "Not a D2 HAM file (%s)", fp->Name ());
		ErrorMsg (message);
	    return 1;
		}
	fp->ReadInt32 (); // version (0x00000007)
	t = fp->ReadInt32 ();
	fp->Seek (sizeof (ushort) * t, SEEK_CUR);
	fp->Seek (sizeof (TMAP_INFO) * t, SEEK_CUR);
	t = fp->ReadInt32 ();
	fp->Seek (sizeof (byte) * t, SEEK_CUR);
	fp->Seek (sizeof (byte) * t, SEEK_CUR);
	t = fp->ReadInt32 ();
	fp->Seek (sizeof (VCLIP) * t, SEEK_CUR);
	t = fp->ReadInt32 ();
	fp->Seek (sizeof (ECLIP) * t, SEEK_CUR);
	t = fp->ReadInt32 ();
	fp->Seek (sizeof (WCLIP) * t, SEEK_CUR);
	}
else if (type == EXTENDED_HAM)  {
	id = fp->ReadInt32 (); // "HAM!"
	if (id != MAKESIG (d2xHamSig)) {//0x214d4148L) {
		sprintf_s (message, sizeof (message), "Not a D2X HAM file (%s)", fp->Name ());
		ErrorMsg (message);
	    return 1;
		}
	fp->ReadInt32 (); //skip version
	t = fp->ReadInt32 (); //skip weapon count
	fp->Seek (t * sizeof (WEAPON_INFO), SEEK_CUR); //skip weapon info
	}

	// read robot information
t = fp->ReadInt32 ();
t0 = (type == NORMAL_HAM) ? 0: N_ROBOT_TYPES_D2;
m_nRobotTypes = t0 + t;
if (m_nRobotTypes > MAX_ROBOT_TYPES) {
	sprintf_s (message, sizeof (message), "Too many robots (%d) in <%s>.  Max is %d.", 
				  t, fp->Name (), MAX_ROBOT_TYPES - N_ROBOT_TYPES_D2);
	ErrorMsg (message);
	m_nRobotTypes = MAX_ROBOT_TYPES;
	t = m_nRobotTypes - t0;
	}
for (; t; t--, t0++) {
	RobotInfo (t0)->Read (fp);
	*DefRobotInfo (t0) = *RobotInfo (t0);
	}

// skip joints weapons, and powerups
t = fp->ReadInt32 ();
fp->Seek (sizeof (JOINTPOS) * t, SEEK_CUR);
if (type == NORMAL_HAM) {
	t = fp->ReadInt32 ();
	fp->Seek (sizeof (WEAPON_INFO) * t, SEEK_CUR);
	t = fp->ReadInt32 ();
	fp->Seek (sizeof (POWERUP_TYPE_INFO) * t, SEEK_CUR);
	}

  // read poly model data and write it to a file
t = fp->ReadInt32 ();
if (t > MAX_POLYGON_MODELS) {
	sprintf_s (message, sizeof (message), "Too many polygon models (%d) in <%s>.  Max is %d.",
				t, fp->Name (), MAX_POLYGON_MODELS - N_POLYGON_MODELS_D2);
	ErrorMsg (message);
	return 1;
	}
#if ALLOCATE_POLYMODELS
  // read joint information
  t = fp->ReadInt32 ();
  t0 = (type == NORMAL_HAM) ? 0: N_ROBOT_JOINTS_D2;
  N_robot_joints = t0 + t;
  if (N_robot_joints > MAX_ROBOT_JOINTS) {
    sprintf_s (message, sizeof (message), "Too many robot joints (%d) in <%s>.  Max is %d.",
					t, fp->Name (), MAX_ROBOT_JOINTS - N_ROBOT_JOINTS_D2);
    ErrorMsg (message);
    goto abort;
  }
  fread( &Robot_joints[t0], sizeof (JOINTPOS), t );

  // skip weapon and powerup data
  if (type == NORMAL_HAM) {
    t = fp->ReadInt32 ();
    fp->Seek (sizeof (WEAPON_INFO) * t, SEEK_CUR);
    t = fp->ReadInt32 ();
    fp->Seek (sizeof (POWERUP_TYPE_INFO) * t, SEEK_CUR);
  }

  // read poly model data
  t = fp->ReadInt32 ();
  t0 = (type == NORMAL_HAM) ? 0: N_POLYGON_MODELS_D2;
  N_polygon_models = t0 + t;
  if (N_polygon_models > MAX_POLYGON_MODELS) {
    sprintf_s (message, sizeof (message), "Too many polygon models (%d) in <%s>.  Max is %d.",
					t, fp->Name (), MAX_POLYGON_MODELS - N_POLYGON_MODELS_D2);
    ErrorMsg (message);
    goto abort;
  }

  short i;

  for (i=t0; i<t0+t; i++ ) {
    // free poly data memory if already allocated
    if (Polygon_models[i]->model_data != null ) {
      free((void *)Polygon_models[i]->model_data);
    }
    fread(Polygon_models[i], sizeof (tPolyModel), 1 );
    fread(&Polygon_model, sizeof (tPolyModel), 1 );
  }
  for (i=t0; i<t0+t; i++ ) {
    Polygon_models[i]->model_data = (byte *) malloc((int)Polygon_models[i]->model_dataSize);

    if (Polygon_models[i]->model_data == null ) {
      ErrorMsg ("Could not allocate memory for polymodel data");
      goto abort;
    }
    fread( Polygon_models[i]->model_data, sizeof (byte), (short)Polygon_models[i]->model_dataSize );
//    g3_init_polygon_model(Polygon_models[i].model_data);
  }

  // extended hog writes over normal hogs dying models instead of adding new ones
  fread( &Dying_modelnums[t0], sizeof (int), t );
  fread( &Dead_modelnums[t0], sizeof (int), t );

  // skip gague data
  if (type == NORMAL_HAM) {
    t = fp->ReadInt32 ();
    fp->Seek (sizeof (ushort) * t, SEEK_CUR); // lores gague
    fp->Seek (sizeof (ushort) * t, SEEK_CUR); // hires gague
  }

  // read object bitmap data
  // NOTE: this overwrites D2 object bitmap indices instead of adding more bitmap texture indices.  
  // I believe that D2 writes all 600 indices even though it doesn't use all of them.
  t = fp->ReadInt32 ();
  t0 = (type == NORMAL_HAM) ? 0: N_OBJBITMAPS_D2;
  if (type == NORMAL_HAM) {
    N_object_bitmaps  = t0 + t;  // only update this if we are reading Descent2.ham file
  }
  if (t+t0 > MAX_OBJ_BITMAPS) {
    sprintf_s (message, sizeof (message), "Too many object bitmaps (%d) in <%s>.  Max is %d.",
					t, fp->Name (), MAX_OBJ_BITMAPS - N_OBJBITMAPS_D2);
    ErrorMsg (message);
    goto abort;
  }
  fread( &ObjBitmaps[t0], sizeof (ushort), t );

  if (type == EXTENDED_HAM) {
    t = fp->ReadInt32 ();
    t0 = (type == NORMAL_HAM) ? 0: N_OBJBITMAPPTRS_D2;
    if (t+t0 > MAX_OBJ_BITMAPS) {
      sprintf_s (message, sizeof (message), "Too many object bitmaps pointer (%d) in <%s>.  Max is %d.",
					  t, fp->Name (), MAX_OBJ_BITMAPS - N_OBJBITMAPPTRS_D2);
      ErrorMsg (message);
      goto abort;
    }
  }
  fread(&ObjBitmapPtrs[t0], sizeof (ushort), t );
#endif

fp->Close ();
return 0;
}

//------------------------------------------------------------------------------

int CRobotManager::ReadHXM (CFileManager& fp, long size) 
{
	CRobotInfo	rInfo;
	int			t, i, j;
	long			p;

if (!fp.File ()) {
	ErrorMsg ("Invalid file handle for reading HXM data.");
	return 1;
	}

p = fp.Tell ();
if (size < 0)
	size = fp.Length ();
uint id;
id = fp.ReadInt32 (); // "HXM!"
if (id != 0x21584d48L) {
	ErrorMsg ("Not a HXM file");
	return 1;
	}

if (m_hxmExtraData) {
	free (m_hxmExtraData);
	m_hxmExtraData = null;
	m_hxmExtraDataSize = 0;
	}
fp.ReadInt32 (); // version (0x00000001)

// read robot information
t = fp.ReadInt32 ();
for (j = 0; j < t; j++) {
	i = fp.ReadInt32 ();
	if ((i < 0) || (i >= (int) m_nRobotTypes)) {
		sprintf_s (message, sizeof (message), "Robots number (%d) out of range.  Range = [0..%d].", i, m_nRobotTypes - 1);
		ErrorMsg (message);
		return 1;
		}
	rInfo.Read (&fp);
	// compare this to existing data
	if (memcmp (&rInfo, RobotInfo (i), sizeof (tRobotInfo)) != 0) {
		memcpy (RobotInfo (i), &rInfo, sizeof (tRobotInfo));
		RobotInfo (i)->Info ().bCustom = 1; // mark as custom
		}
	}

m_hxmExtraDataSize = size - fp.Tell () + p;
if (m_hxmExtraDataSize > 0) {
	m_hxmExtraData = new byte [m_hxmExtraDataSize];
	if (m_hxmExtraData == null) {
		ErrorMsg ("Couldn't allocate extra data from hxm file.\nThis data will be lost when saving the level!");
		return 1;
		}
	if (fp.Read (m_hxmExtraData, m_hxmExtraDataSize, 1) != 1) {
		ErrorMsg ("Couldn't read extra data from hxm file.\nThis data will be lost when saving the level!");
		return 1;
		}
	}
return 0;
}

//------------------------------------------------------------------------------

int CRobotManager::WriteHXM (CFileManager& fp) 
{
	int i, t;

for (i = 0, t = 0; i < m_nRobotTypes; i++)
	if (IsCustomRobot (i))
		t++;
if ((t == 0) && (m_hxmExtraDataSize == 0))
	return 0;

if (!fp.File ()) {
	ErrorMsg ("Invalid file handle for writing HXM data.");
	return 1;
	}

fp.WriteInt32 (0x21584d48);	// "HXM!"
fp.WriteInt32 (1);   // version 1

// write robot information
fp.Write (t); // number of robot info structs stored
for (i = 0; i < m_nRobotTypes; i++) {
	if (RobotInfo (i)->Info ().bCustom) {
		fp.Write (i);
		RobotInfo (i)->Write (&fp);
		}
	}

if (m_hxmExtraDataSize)
	fp.Write (m_hxmExtraData, m_hxmExtraDataSize, 1);
else {
	// write zeros for the rest of the data
	fp.WriteInt32 (0);  //number of joints
	fp.WriteInt32 (0);  //number of polygon models
	fp.WriteInt32 (0);  //number of objbitmaps
	fp.WriteInt32 (0);  //number of objbitmaps
	fp.WriteInt32 (0);  //number of objbitmapptrs
	}

if (t) {
	sprintf_s (message, sizeof (message)," Hxm manager: Saving %d custom robots",t);
	DEBUGMSG (message);
	}
fp.Close ();
return 0;
}

//------------------------------------------------------------------------------

void CRobotManager::Init (void) 
{
LoadResource (-1);
}

//------------------------------------------------------------------------------

void CRobotManager::LoadResource (int nRobot) 
{
  int			i,	j,	t;
  byte*		bufP;
  CResource	res;

if (!(bufP = res.Load ("ROBOT.HXM"))) {
	ErrorMsg ("Could not lock robot resource data");
	return;
	}
t = (ushort) (*((uint *) bufP));
m_nRobotTypes = min (t, MAX_ROBOT_TYPES);
bufP += sizeof (uint);
for (j = 0; j < t; j++) {
	i = (ushort) (*((uint *) bufP));
	if (i > MAX_ROBOT_TYPES) 
		break;
	bufP += sizeof (uint);
	// copy the robot info for one robot, or all robots
	if ((j == nRobot) || (nRobot == -1)) 
		memcpy (RobotInfo (i), bufP, sizeof (tRobotInfo));
	bufP += sizeof (tRobotInfo);
	}
}

//------------------------------------------------------------------------------

bool CRobotManager::IsCustomRobot (int nId)
{
if (!RobotInfo (nId)->Info ().bCustom) //changed?
	return false;
	// check if actually different from defaults

bool bFound = false;

byte bCustom = DefRobotInfo (nId)->Info ().bCustom;
DefRobotInfo (nId)->Info ().bCustom = RobotInfo (nId)->Info ().bCustom; //make sure it's equal for the comparison
if (!memcmp (RobotInfo (nId), DefRobotInfo (nId), sizeof (tRobotInfo))) 
	RobotInfo (nId)->Info ().bCustom = 0; //same as default
else { //they're different
	// find a robot of that type
	CGameObject* objP = objectManager.FindRobot (nId);
	if (objP != null) // found one
		bFound = true;
	else { //no robot of that type present
		// find a matcen producing a robot of that type
		CSegment* segP;
		for (short i = 0; (segP = segmentManager.FindRobotMaker (i)) != null; i = segmentManager.Index (segP) + 1) {
			int nBotGen = segP->Info ().nMatCen;
			if ((nId < 32) 
				 ? segmentManager.RobotMaker (nBotGen)->Info ().objFlags [0] & (1 << nId) 
				 : segmentManager.RobotMaker (nBotGen)->Info ().objFlags [1] & (1 << (nId - 32)))
				break;
			}
		if (segP != null) // found one
			bFound = true;
		else
			RobotInfo (nId)->Info ().bCustom = 0; // no matcens or none producing that robot type
		}
	}
DefRobotInfo (nId)->Info ().bCustom = bCustom; //restore
return bFound;
}

//------------------------------------------------------------------------------

bool CRobotManager::HasCustomRobots (void) 
{
for (int i = 0; i < (int) m_nRobotTypes; i++)
	if (IsCustomRobot (i))
		return true;
return (m_hxmExtraDataSize > 0);
}

//------------------------------------------------------------------------------
//eof RobotManager.cpp


