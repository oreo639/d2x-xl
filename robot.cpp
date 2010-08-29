// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <io.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "dle-xp.h"
#include "mine.h"
#include "global.h"
#include "toolview.h"
#include "io.h"
#include "hogmanager.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

void tRobotGunInfo::Read (CFileManager& fp, int nField) {
	if (nField == 0)
		points.Read (fp);
	else
		subModels = byte (fp.ReadSByte ());
	}

void tRobotGunInfo::Write (CFileManager& fp, int nField) {
	if (nField == 0)
		points.Write (fp);
	else
		WriteInt8 ((char) subModels, fp);
	}

//------------------------------------------------------------------------

void tRobotExplInfo::Read (CFileManager& fp) {
	nClip = fp.ReadInt16 ();
	nSound = fp.ReadInt16 ();
	}

void tRobotExplInfo::Write (CFileManager& fp) {
	WriteInt16 (nClip , fp);
	WriteInt16 (nSound, fp);
	}

//------------------------------------------------------------------------

void tRobotContentsInfo::Read (CFileManager& fp) {
	id = fp.ReadSByte ();
	count = fp.ReadSByte ();
	prob = fp.ReadSByte ();
	type = fp.ReadSByte ();
	}

void tRobotContentsInfo::Write (CFileManager& fp) {
	WriteInt8 (id, fp);
	WriteInt8 (count, fp);
	WriteInt8 (prob, fp);
	WriteInt8 (type, fp);
	}

//------------------------------------------------------------------------

void tRobotSoundInfo::Read (CFileManager& fp) {
	see = (byte) fp.ReadSByte ();
	attack = (byte) fp.ReadSByte ();
	claw = (byte) fp.ReadSByte ();
	taunt = (byte) fp.ReadSByte ();
	}

void tRobotSoundInfo::Write (CFileManager& fp) {
	WriteInt8 ((char) see, fp);
	WriteInt8 ((char) attack, fp);
	WriteInt8 ((char) claw, fp);
	WriteInt8 ((char) taunt, fp);
	}

//------------------------------------------------------------------------

void tRobotCombatInfo::Read (CFileManager& fp, int nField) {
	switch (nField) {
		case 0:
			fieldOfView = fp.ReadFix ();
			break;
		case 1:
			firingWait [0] = fp.ReadFix ();
			break;
		case 2:
			firingWait [1] = fp.ReadFix ();
			break;
		case 3:
			turnTime = fp.ReadFix ();
			break;
		case 4:
			maxSpeed = fp.ReadFix ();
			break;
		case 5:
			circleDistance = fp.ReadFix ();
			break;
		case 6:
			rapidFire = fp.ReadSByte ();
			break;
		case 7:
			evadeSpeed = fp.ReadSByte ();
			break;
		}
	}

void tRobotCombatInfo::Write (CFileManager& fp, int nField) {
	switch (nField) {
		case 0:
			WriteFix (fieldOfView, fp);
			break;
		case 1:
			WriteFix (firingWait [0], fp);
			break;
		case 2:
			WriteFix (firingWait [1], fp);
			break;
		case 3:
			WriteFix (turnTime, fp);
			break;
		case 4:
			WriteFix (maxSpeed, fp);
			break;
		case 5:
			WriteFix (circleDistance, fp);
			break;
		case 6:
			WriteInt8 (rapidFire, fp);
			break;
		case 7:
			WriteInt8 (evadeSpeed, fp);
			break;
		}
	}

//------------------------------------------------------------------------

int CRobotInfo::Read (CFileManager& fp, int version, bool bFlag) 
{ 
	int i, j;

m_info.nModel = fp.ReadInt32 ();
for (j = 0; j < 2; j++)
	for (i = 0; i < MAX_GUNS; i++)
		m_info.guns [i].Read (fp, j);
for (i = 0; i < 2; i++)
	m_info.expl [i].Read (fp);
for (i = 0; i < 2; i++)
	m_info.weaponType [i] = fp.ReadSByte ();
m_info.n_guns = fp.ReadSByte ();
m_info.contents.Read (fp);
m_info.kamikaze = fp.ReadSByte ();
m_info.scoreValue = fp.ReadInt16 ();
m_info.badass = fp.ReadSByte ();
m_info.drainEnergy = fp.ReadSByte ();
m_info.lighting = fp.ReadFix ();
m_info.strength = fp.ReadFix ();
m_info.mass = fp.ReadFix ();
m_info.drag = fp.ReadFix ();
for (j = 0; j < 8; j++)
	for (i = 0; i < NDL; i++)
		m_info.combat [i].Read (fp, j);
m_info.cloakType = fp.ReadSByte ();
m_info.attackType = fp.ReadSByte ();
m_info.sounds.Read (fp);
m_info.bossFlag = fp.ReadSByte ();
m_info.companion = fp.ReadSByte ();
m_info.smartBlobs = fp.ReadSByte ();
m_info.energyBlobs = fp.ReadSByte ();
m_info.thief = fp.ReadSByte ();
m_info.pursuit = fp.ReadSByte ();
m_info.lightCast = fp.ReadSByte ();
m_info.deathRoll = fp.ReadSByte ();
m_info.flags = (byte) fp.ReadSByte ();
m_info.bCustom = fp.ReadSByte (); 
m_info.pad [0] = fp.ReadSByte (); 
m_info.pad [1] = fp.ReadSByte (); 
m_info.deathRollSound = (byte) fp.ReadSByte ();
m_info.glow = (byte) fp.ReadSByte ();
m_info.behavior = (byte) fp.ReadSByte ();
m_info.aim = (byte) fp.ReadSByte ();
for (i = 0; i <= MAX_GUNS; i++)
	for (j = 0; j < N_ANIM_STATES; j++)
		m_info.animStates [i][j].Read (fp);
m_info.always_0xabcd = fp.ReadInt32 ();
return 1; 
}

//------------------------------------------------------------------------

void CRobotInfo::Write (CFileManager& fp, int version, bool bFlag) 
{
	int i, j;

WriteInt32 (m_info.nModel, fp);
for (j = 0; j < 2; j++)
	for (i = 0; i < MAX_GUNS; i++)
		m_info.guns [i].Write (fp, j);
for (i = 0; i < 2; i++)
	m_info.expl [i].Write (fp);
for (i = 0; i < 2; i++)
	WriteInt8 (m_info.weaponType [i], fp);
WriteInt8 (m_info.n_guns, fp);
m_info.contents.Write (fp);
WriteInt8 (m_info.kamikaze, fp);
WriteInt16 (m_info.scoreValue, fp);
WriteInt8 (m_info.badass, fp);
WriteInt8 (m_info.drainEnergy, fp);
WriteFix (m_info.lighting, fp);
WriteFix (m_info.strength, fp);
WriteFix (m_info.mass, fp);
WriteFix (m_info.drag, fp);
for (j = 0; j < 8; j++)
	for (i = 0; i < NDL; i++)
		m_info.combat [i].Write (fp, j);
WriteInt8 (m_info.cloakType, fp);
WriteInt8 (m_info.attackType, fp);
m_info.sounds.Write (fp);
WriteInt8 (m_info.bossFlag, fp);
WriteInt8 (m_info.companion, fp);
WriteInt8 (m_info.smartBlobs, fp);
WriteInt8 (m_info.energyBlobs, fp);
WriteInt8 (m_info.thief, fp);
WriteInt8 (m_info.pursuit, fp);
WriteInt8 (m_info.lightCast, fp);
WriteInt8 (m_info.deathRoll, fp);
WriteInt8 (m_info.flags, fp);
WriteInt8 (m_info.bCustom, fp); // skip
WriteInt8 (m_info.pad [1], fp); // skip
WriteInt8 (m_info.pad [2], fp); // skip
(byte) WriteInt8 (m_info.deathRollSound, fp);
(byte) WriteInt8 (m_info.glow, fp);
(byte) WriteInt8 (m_info.behavior, fp);
(byte) WriteInt8 (m_info.aim, fp);
for (i = 0; i <= MAX_GUNS; i++)
	for (j = 0; j < N_ANIM_STATES; j++)
		m_info.animStates [i][j].Write (fp);
WriteInt32 (m_info.always_0xabcd, fp);
}

//------------------------------------------------------------------------

//------------------------------------------------------------------------
// ReadHamFile()
//
// Actions
//   1)	Reads all robot data from a HAM file
//   2) Memory will be allocated for polymodel data.  It is the
//	responsibility of the caller to free all non-null polymodel
//	data pointer.
//   3) If polymodel data is allocated aready, it will be freed and
//	reallocated.
//
// Parameters
//	fname - handle of file to read
//      type - type of file (0=DESCENT2.HAM, 1=extended robot HAM file)
//
// Globals used
//
//  N_robot_types
//  robotInfo[MAX_ROBOT_TYPES]
//
//  N_robot_joints
//  Robot_joints[MAX_ROBOT_JOINTS]
//
//  N_polygon_models
//  Polygon_models[MAX_POLYGON_MODELS]
//  Dying_modelnums[N_D2_POLYGON_MODELS]
//  Dead_modelnums[N_D2_POLYGON_MODELS]
//
//  N_object_bitmaps
//  ObjBitmaps[MAX_OBJ_BITMAPS]
//  ObjBitmapPtrs[MAX_OBJ_BITMAPS]
//
//
// Assumptions
//   1) Memory was allocated for globals (except polymodel data)
//------------------------------------------------------------------------

#define MAKESIG(_sig)	(uint) *((int *) &(_sig))

int CMine::ReadHamFile(char *pszFile, int type) 
{
  FILE*			fp;
  short			t, t0;
  uint			id;
  CPolyModel	pm;
  char			szFile [256];

  static char d2HamSig [4] = {'H','A','M','!'};
  static char d2xHamSig [4] = {'M','A','H','X'};

if (!pszFile) {
	if (IsD2File ()) {
		FSplit (descent2_path, szFile, NULL, NULL);
		strcat_s (szFile, sizeof (szFile), "descent2.ham");
		}
	else {
		FSplit (descent_path, szFile, NULL, NULL);
		strcat_s (szFile, sizeof (szFile), "descent.ham");
		}
	pszFile = szFile;
	}

  pm.m_info.nModels = 0;
  fopen_s (&fp, pszFile, "rb");
  if (!fp) {
    sprintf_s (message, sizeof (message), " Ham manager: Cannot open robot file <%s>.", pszFile);
    DEBUGMSG (message);
    goto abort;
  }

// The extended HAM only contains part of the normal HAM file.
// Therefore, we have to skip some items if we are reading
// a normal HAM cause we are only interested in reading
// the information which is found in the extended ham
// (the robot information)
if (type == NORMAL_HAM)  {
	id = ReadInt32(fp); // "HAM!"
	if (id != MAKESIG (d2HamSig)) {//0x214d4148L) {
		sprintf_s (message, sizeof (message), "Not a D2 HAM file (%s)", pszFile);
		ErrorMsg (message);
		goto abort;
		}
	ReadInt32(fp); // version (0x00000007)
	t = (short) ReadInt32(fp);
	fseek (fp, sizeof (ushort) * t, SEEK_CUR);
	fseek (fp, sizeof (TMAP_INFO) * t, SEEK_CUR);
	t = (short) ReadInt32(fp);
	fseek (fp, sizeof (byte) * t, SEEK_CUR);
	fseek (fp, sizeof (byte) * t, SEEK_CUR);
	t = (short) ReadInt32(fp);
	fseek (fp, sizeof (VCLIP) * t, SEEK_CUR);
	t = (short) ReadInt32(fp);
	fseek (fp, sizeof (ECLIP) * t, SEEK_CUR);
	t = (short) ReadInt32(fp);
	fseek (fp, sizeof (WCLIP) * t, SEEK_CUR);
	}
else if (type == EXTENDED_HAM)  {
	id = ReadInt32(fp); // "HAM!"
	if (id != MAKESIG (d2xHamSig)) {//0x214d4148L) {
		sprintf_s (message, sizeof (message), "Not a D2X HAM file (%s)", pszFile);
		ErrorMsg (message);
		goto abort;
		}
	ReadInt32(fp); //skip version
	t = ReadInt32(fp); //skip weapon count
	fseek (fp, t * sizeof (WEAPON_INFO), SEEK_CUR); //skip weapon info
	}

	// read robot information
	//------------------------
	t = (short) ReadInt32(fp);
	t0 = (type == NORMAL_HAM) ? 0: N_D2_ROBOT_TYPES;
	N_robot_types = t0 + t;
	if (N_robot_types > MAX_ROBOT_TYPES) {
		sprintf_s (message, sizeof (message), "Too many robots (%d) in <%s>.  Max is %d.",t,pszFile,MAX_ROBOT_TYPES-N_D2_ROBOT_TYPES);
		ErrorMsg (message);
		N_robot_types = MAX_ROBOT_TYPES;
		t = N_robot_types - t0;
		//    goto abort;
		}
	for (; t; t--, t0++) {
		RobotInfo (t0)->Read (fp);
		*DefRobotInfo (t0) = *RobotInfo (t0);
		}
  //memcpy (DefRobotInfo (t0), RobotInfo (t0), sizeof (tRobotInfo) * t);

  // skip joints weapons, and powerups
  //----------------------------------
  t = (short) ReadInt32(fp);
  fseek(fp,sizeof (JOINTPOS)*t,SEEK_CUR);
  if (type == NORMAL_HAM) {
    t = (short) ReadInt32(fp);
    fseek(fp,sizeof (WEAPON_INFO)*t,SEEK_CUR);
    t = (short) ReadInt32(fp);
    fseek(fp,sizeof (POWERUP_TYPE_INFO)*t,SEEK_CUR);
  }

  // read poly model data and write it to a file
  //---------------------------------------------
  t = (short) ReadInt32(fp);
  if (t > MAX_POLYGON_MODELS) {
    sprintf_s (message, sizeof (message), "Too many polygon models (%d) in <%s>.  Max is %d.",t,pszFile,MAX_POLYGON_MODELS-N_D2_POLYGON_MODELS);
    ErrorMsg (message);
    goto abort;
  }
#if ALLOCATE_tPolyModelS
  // read joint information
  //-----------------------
  t = (short) ReadInt32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_ROBOT_JOINTS;
  N_robot_joints = t0 + t;
  if (N_robot_joints > MAX_ROBOT_JOINTS) {
    sprintf_s (message, sizeof (message), "Too many robot joints (%d) in <%s>.  Max is %d.",t,pszFile,MAX_ROBOT_JOINTS-N_D2_ROBOT_JOINTS);
    ErrorMsg (message);
    goto abort;
  }
  fread( &Robot_joints[t0], sizeof (JOINTPOS), t, fp );

  // skip weapon and powerup data
  //-----------------------------
  if (type == NORMAL_HAM) {
    t = (short) ReadInt32(fp);
    fseek(fp,sizeof (WEAPON_INFO)*t,SEEK_CUR);
    t = (short) ReadInt32(fp);
    fseek(fp,sizeof (POWERUP_TYPE_INFO)*t,SEEK_CUR);
  }

  // read poly model data
  //---------------------
  t = (short) ReadInt32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_POLYGON_MODELS;
  N_polygon_models = t0 + t;
  if (N_polygon_models > MAX_POLYGON_MODELS) {
    sprintf_s (message, sizeof (message), "Too many polygon models (%d) in <%s>.  Max is %d.",t,pszFile,MAX_POLYGON_MODELS-N_D2_POLYGON_MODELS);
    ErrorMsg (message);
    goto abort;
  }

  short i;

  for (i=t0; i<t0+t; i++ ) {
    // free poly data memory if already allocated
    if (Polygon_models[i]->model_data != NULL ) {
      free((void *)Polygon_models[i]->model_data);
    }
    fread(Polygon_models[i], sizeof (tPolyModel), 1, fp );
    fread(&Polygon_model, sizeof (tPolyModel), 1, fp );
  }
  for (i=t0; i<t0+t; i++ ) {
    Polygon_models[i]->model_data = (byte *) malloc((int)Polygon_models[i]->model_dataSize);

    if (Polygon_models[i]->model_data == NULL ) {
      ErrorMsg ("Could not allocate memory for polymodel data");
      goto abort;
    }
    fread( Polygon_models[i]->model_data, sizeof (byte), (short)Polygon_models[i]->model_dataSize, fp );
//    g3_init_polygon_model(Polygon_models[i].model_data);
  }

  // extended hog writes over normal hogs dying models instead of adding new ones
  fread( &Dying_modelnums[t0], sizeof (int), t, fp );
  fread( &Dead_modelnums[t0], sizeof (int), t, fp );

  // skip gague data
  //----------------
  if (type == NORMAL_HAM) {
    t = (short) ReadInt32(fp);
    fseek(fp,sizeof (ushort)*t,SEEK_CUR); // lores gague
    fseek(fp,sizeof (ushort)*t,SEEK_CUR); // hires gague
  }

  // read object bitmap data
  //------------------------
  // NOTE: this overwrites D2 object bitmap indices instead of
  // adding more bitmap texture indicies.  I believe that D2
  // writes all 600 indicies even though it doesn't use all
  // of them.
  //----------------------------------------------------------
  t = (short) ReadInt32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_OBJBITMAPS;
  if (type == NORMAL_HAM) {
    N_object_bitmaps  = t0 + t;  // only update this if we are reading Descent2.ham file
  }
  if (t+t0 > MAX_OBJ_BITMAPS) {
    sprintf_s (message, sizeof (message), "Too many object bitmaps (%d) in <%s>.  Max is %d.",t,pszFile,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPS);
    ErrorMsg (message);
    goto abort;
  }
  fread( &ObjBitmaps[t0], sizeof (ushort), t, fp );

  if (type == EXTENDED_HAM) {
    t = (short) ReadInt32(fp);
    t0 = (type == NORMAL_HAM) ? 0: N_D2_OBJBITMAPPTRS;
    if (t+t0 > MAX_OBJ_BITMAPS) {
      sprintf_s (message, sizeof (message), "Too many object bitmaps pointer (%d) in <%s>.  Max is %d.",t,pszFile,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPPTRS);
      ErrorMsg (message);
      goto abort;
    }
  }
  fread(&ObjBitmapPtrs[t0], sizeof (ushort), t, fp );
#endif

  fclose(fp);
  return 0;
abort:
  if (fp) fclose(fp);
  return 1;
}


//------------------------------------------------------------------------
// ReadHxmFile()
//
// Actions
//   1)	Reads all robot data from a HXM file
//   2) Memory will be allocated for polymodel data.  It is the
//	responsibility of the caller to free all non-null polymodel
//	data pointer.
//   3) If polymodel data is allocated aready, it will be freed and
//	reallocated.
//
// Parameters
//	fp - pointer to file (already offset to the correct position)
//
// Globals used
//
//  N_robot_types
//  robotInfo[MAX_ROBOT_TYPES]
//
//  N_robot_joints
//  Robot_joints[MAX_ROBOT_JOINTS]
//
//  N_polygon_models
//  Polygon_models[MAX_POLYGON_MODELS]
//  Dying_modelnums[N_D2_POLYGON_MODELS]
//  Dead_modelnums[N_D2_POLYGON_MODELS]
//
//  N_object_bitmaps
//  ObjBitmaps[MAX_OBJ_BITMAPS]
//  ObjBitmapPtrs[MAX_OBJ_BITMAPS]
//
//
// Assumptions
//   1) Memory was allocated for globals (except polymodel data)
//------------------------------------------------------------------------

int CMine::ReadHxmFile(CFileManager& fp, long fSize) 
{
	ushort t,i,j;
	CRobotInfo rInfo;
	long p;

if (!fp) {
	ErrorMsg ("Invalid file handle for reading HXM data.");
	goto abort;
	}

p = ftell (fp);
if (fSize < 0)
	fSize = _filelength (_fileno (fp));
uint id;
id = ReadInt32(fp); // "HXM!"
if (id != 0x21584d48L) {
	ErrorMsg ("Not a HXM file");
	goto abort;
	}
if (m_pHxmExtraData) {
	free (m_pHxmExtraData);
	m_pHxmExtraData = NULL;
	m_nHxmExtraDataSize = 0;
	}
ReadInt32(fp); // version (0x00000001)

// read robot information
//------------------------
t = (ushort) ReadInt32(fp);
for (j = 0; j < t; j++) {
	i = (ushort) ReadInt32(fp);
	if (i >= N_robot_types) {
		sprintf_s (message, sizeof (message), "Robots number (%d) out of range.  Range = [0..%d].", i, N_robot_types - 1);
		ErrorMsg (message);
		goto abort;
		}
	rInfo.Read (fp);
	// compare this to existing data
	if (memcmp (&rInfo, RobotInfo (i), sizeof (tRobotInfo)) != 0) {
		memcpy (RobotInfo (i), &rInfo, sizeof (tRobotInfo));
		RobotInfo (i)->m_info.bCustom = 1; // mark as custom
		}
	}

m_nHxmExtraDataSize = fSize - ftell (fp) + p;
if (m_nHxmExtraDataSize > 0) {
	if (!(m_pHxmExtraData = (char*) malloc (m_nHxmExtraDataSize))) {
		ErrorMsg ("Couldn't allocate extra data from hxm file.\nThis data will be lost when saving the level!");
		goto abort;
		}
	if (fread (m_pHxmExtraData, m_nHxmExtraDataSize, 1, fp) != 1) {
		ErrorMsg ("Couldn't read extra data from hxm file.\nThis data will be lost when saving the level!");
		goto abort;
		}
	}
return 0;
abort:
//  if (fp) fclose(fp);
return 1;
}

//------------------------------------------------------------------------
// ()
//
// Actions
//   1)	Writes robot info in HXM file format
//
// Parameters
//	fp - pointer to file (already offset to the correct position)
//
// Globals used
//
//  N_robot_types
//  robotInfo[MAX_ROBOT_TYPES]
//------------------------------------------------------------------------

int CMine::WriteHxmFile(CFileManager& fp) 
{
ushort t,i;

for (i = 0,t = 0; i < N_robot_types; i++)
	if (IsCustomRobot (i))
		t++;
if (!(t || m_nHxmExtraDataSize))
	return 0;
if (!fp) {
	ErrorMsg ("Invalid file handle for writing HXM data.");
	goto abort;
	}

#if 1
uint id;
id = 0x21584d48L;    // "HXM!"
WriteInt32 (id,fp);
WriteInt32 (1,fp);   // version 1
#endif

// write robot information
//------------------------
WriteInt32 (t,fp); // number of robot info structs stored
for (i = 0; i < N_robot_types; i++) {
	if (RobotInfo (i)->m_info.bCustom) {
		WriteInt32 ((uint) i, fp);
		RobotInfo (i)->Write (fp);
		}
	}

#if 1
if (m_nHxmExtraDataSize)
	fwrite (m_pHxmExtraData, m_nHxmExtraDataSize, 1, fp);
else
#endif
// write zeros for the rest of the data
//-------------------------------------
{
WriteInt32 (0,fp);  //number of joints
WriteInt32 (0,fp);  //number of polygon models
WriteInt32 (0,fp);  //number of objbitmaps
WriteInt32 (0,fp);  //number of objbitmaps
WriteInt32 (0,fp);  //number of objbitmapptrs
}

if (t) {
	sprintf_s (message, sizeof (message)," Hxm manager: Saving %d custom robots",t);
	DEBUGMSG (message);
	}
fclose(fp);
return 0;

abort:

if (fp) 
	fclose(fp);
return 1;
}

//------------------------------------------------------------------------
// InitRobotData()
//------------------------------------------------------------------------

void CMine::InitRobotData() 
{
  ReadRobotResource(-1);
}


//------------------------------------------------------------------------
// ReadRobotResource() - reads robot.hxm from resource data into robotInfo[]
//
// if robot_number == -1, then it reads all robots
//------------------------------------------------------------------------
void CMine::ReadRobotResource(int robot_number) 
{
  ushort i,j,t;
  byte *ptr;
  HRSRC hFind = FindResource( hInst,"ROBOT_HXM", "RC_DATA");
  HINSTANCE hInst = AfxGetApp ()->m_hInstance;
  HGLOBAL hResource = LoadResource( hInst, hFind);
  if (!hResource) {
    ErrorMsg ("Could not find robot resource data");
    return;
  }
  ptr = (byte *)LockResource(hResource);
  if (!ptr) {
    ErrorMsg ("Could not lock robot resource data");
    return;
  }
  t = (ushort)(*((uint *)ptr));
  N_robot_types = min(t,MAX_ROBOT_TYPES);
  ptr += sizeof (uint);
  for (j=0;j<t;j++) {
    i = (ushort)(*((uint *)ptr));
    if (i>MAX_ROBOT_TYPES) break;
    ptr += sizeof (uint);
    // copy the robot info for one robot, or all robots
    if (j==robot_number || robot_number == -1) {
      memcpy(RobotInfo (i), ptr, sizeof (tRobotInfo));
    }
    ptr += sizeof (tRobotInfo);
  }
  FreeResource(hResource);
}

//--------------------------------------------------------------------------
// has_custom_robots()
//--------------------------------------------------------------------------

bool CMine::IsCustomRobot (int i)
{
	bool				bFound = false;
	CSegment*		segP;
	CGameObject*	objP;
	int				j;

if (!RobotInfo (i)->m_info.bCustom) //changed?
	return false;
	// check if actually different from defaults
byte bCustom = DefRobotInfo (i)->m_info.bCustom;
DefRobotInfo (i)->m_info.bCustom = RobotInfo (i)->m_info.bCustom; //make sure it's equal for the comparison
if (memcmp (RobotInfo (i), DefRobotInfo (i), sizeof (tRobotInfo))) { //they're different
	// find a robot of that type
	for (j = GameInfo ().objects.count, objP = Objects (0); j; j--, objP++)
		if ((objP->m_info.type == OBJ_ROBOT) && (objP->m_info.id == i))
			break;
	if (j) // found one
		bFound = true;
	else { //no robot of that type present
		// find a matcen producing a robot of that type
		for (j = SegCount (), segP = Segments (0); j; j--, segP++)
			if (segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) {
				int matcen = segP->m_info.nMatCen;
				if ((i < 32) 
					 ? BotGens (matcen)->m_info.objFlags [0] & (1L << i) 
					 : BotGens (matcen)->m_info.objFlags [1] & (1L << (i-32)))
					break;
				}
		if (j) // found one
			bFound = true;
		else
			RobotInfo (i)->m_info.bCustom = 0; // no matcens or none producing that robot type
		}
	}
else
	RobotInfo (i)->m_info.bCustom = 0; //same as default
DefRobotInfo (i)->m_info.bCustom = bCustom; //restore
return bFound;
}

//--------------------------------------------------------------------------
// has_custom_robots()
//--------------------------------------------------------------------------

BOOL CMine::HasCustomRobots() 
{
int i;
for (i = 0; i < (int) N_robot_types; i++)
	if (IsCustomRobot (i))
		return TRUE;
return (m_nHxmExtraDataSize > 0);
}

//eof robot.cpp


