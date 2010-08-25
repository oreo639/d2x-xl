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

void tRobotGunInfo::Read (FILE* fp) {
	points.Read (fp);
	subModels = UINT8 (read_INT8 (fp));
	}

void tRobotGunInfo::Write (FILE* fp) {
	points.Write (fp);
	write_INT8 ((INT8) subModels, fp);
	}

//------------------------------------------------------------------------

void tRobotExplInfo::Read (FILE* fp) {
	nClip = read_INT16 (fp);
	nSound = read_INT16 (fp);
	}

void tRobotExplInfo::Write (FILE* fp) {
	write_INT16 (nClip , fp);
	write_INT16 (nSound, fp);
	}

//------------------------------------------------------------------------

void tRobotContentsInfo::Read (FILE* fp) {
	id = read_INT8 (fp);
	count = read_INT8 (fp);
	prob = read_INT8 (fp);
	type = read_INT8 (fp);
	}

void tRobotContentsInfo::Write (FILE* fp) {
	write_INT8 (id, fp);
	write_INT8 (count, fp);
	write_INT8 (prob, fp);
	write_INT8 (type, fp);
	}

//------------------------------------------------------------------------

void tRobotSoundInfo::Read (FILE* fp) {
	see = (UINT8) read_INT8 (fp);
	attack = (UINT8) read_INT8 (fp);
	claw = (UINT8) read_INT8 (fp);
	taunt = (UINT8) read_INT8 (fp);
	}

void tRobotSoundInfo::Write (FILE* fp) {
	write_INT8 ((INT8) see, fp);
	write_INT8 ((INT8) attack, fp);
	write_INT8 ((INT8) claw, fp);
	write_INT8 ((INT8) taunt, fp);
	}

//------------------------------------------------------------------------

void tRobotCombatInfo::Read (FILE* fp, int nField) {
	switch (nField) {
		case 0:
			fieldOfView = read_FIX (fp);
			break;
		case 1:
			firingWait [0] = read_FIX (fp);
			break;
		case 2:
			firingWait [1] = read_FIX (fp);
			break;
		case 3:
			turnTime = read_FIX (fp);
			break;
		case 4:
			maxSpeed = read_FIX (fp);
			break;
		case 5:
			circleDistance = read_FIX (fp);
			break;
		case 6:
			rapidFire = read_INT8 (fp);
			break;
		case 7:
			evadeSpeed = read_INT8 (fp);
			break;
		}
	}

void tRobotCombatInfo::Write (FILE* fp, int nField) {
	switch (nField) {
		case 0:
			write_FIX (fieldOfView, fp);
			break;
		case 1:
			write_FIX (firingWait [0], fp);
			break;
		case 2:
			write_FIX (firingWait [1], fp);
			break;
		case 3:
			write_FIX (turnTime, fp);
			break;
		case 4:
			write_FIX (maxSpeed, fp);
			break;
		case 5:
			write_FIX (circleDistance, fp);
			break;
		case 6:
			write_INT8 (rapidFire, fp);
			break;
		case 7:
			write_INT8 (evadeSpeed, fp);
			break;
		}
	}

//------------------------------------------------------------------------

INT32 CRobotInfo::Read (FILE* fp, INT32 version, bool bFlag) 
{ 
	int i, j;

m_info.nModel = read_INT32 (fp);
for (i = 0; i < MAX_GUNS; i++)
	m_info.guns [i].Read (fp);
for (i = 0; i < 2; i++)
	m_info.expl [i].Read (fp);
for (i = 0; i < 2; i++)
	m_info.weaponType [i] = read_INT8 (fp);
m_info.n_guns = read_INT8 (fp);
m_info.contents.Read (fp);
m_info.kamikaze = read_INT8 (fp);
m_info.scoreValue = read_INT16 (fp);
m_info.badass = read_INT8 (fp);
m_info.drainEnergy = read_INT8 (fp);
m_info.lighting = read_FIX (fp);
m_info.strength = read_FIX (fp);
m_info.mass = read_FIX (fp);
m_info.drag = read_FIX (fp);
for (i = 0; i < 2; i++)
	m_info.weaponType [i] = read_INT8 (fp);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Read (fp, 0);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Read (fp, 1);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Read (fp, 2);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Read (fp, 3);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Read (fp, 4);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Read (fp, 5);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Read (fp, 6);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Read (fp, 7);
m_info.cloakType = read_INT8 (fp);
m_info.attackType = read_INT8 (fp);
m_info.sounds.Read (fp);
m_info.bossFlag = read_INT8 (fp);
m_info.companion = read_INT8 (fp);
m_info.smartBlobs = read_INT8 (fp);
m_info.energyBlobs = read_INT8 (fp);
m_info.thief = read_INT8 (fp);
m_info.pursuit = read_INT8 (fp);
m_info.lightCast = read_INT8 (fp);
m_info.deathRoll = read_INT8 (fp);
m_info.flags = (UINT8) read_INT8 (fp);
m_info.bCustom = read_INT8 (fp); 
m_info.pad [0] = read_INT8 (fp); 
m_info.pad [1] = read_INT8 (fp); 
m_info.deathRollSound = (UINT8) read_INT8 (fp);
m_info.glow = (UINT8) read_INT8 (fp);
m_info.behavior = (UINT8) read_INT8 (fp);
m_info.aim = (UINT8) read_INT8 (fp);
for (i = 0; i <= MAX_GUNS; i++)
	for (j = 0; j < N_ANIM_STATES; j++)
		m_info.animStates [i][j].Read (fp);
m_info.always_0xabcd = read_INT32 (fp);
return 1; 
}

//------------------------------------------------------------------------

void CRobotInfo::Write (FILE* fp, INT32 version, bool bFlag) 
{
	int i, j;

write_INT32 (m_info.nModel, fp);
for (i = 0; i < MAX_GUNS; i++)
	m_info.guns [i].Write (fp);
for (i = 0; i < 2; i++)
	m_info.expl [i].Write (fp);
for (i = 0; i < 2; i++)
	write_INT8 (m_info.weaponType [i], fp);
write_INT8 (m_info.n_guns, fp);
m_info.contents.Write (fp);
write_INT8 (m_info.kamikaze, fp);
write_INT16 (m_info.scoreValue, fp);
write_INT8 (m_info.badass, fp);
write_INT8 (m_info.drainEnergy, fp);
write_FIX (m_info.lighting, fp);
write_FIX (m_info.strength, fp);
write_FIX (m_info.mass, fp);
write_FIX (m_info.drag, fp);
for (i = 0; i < 2; i++)
	write_INT8 (m_info.weaponType [i], fp);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Write (fp, 0);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Write (fp, 1);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Write (fp, 2);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Write (fp, 3);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Write (fp, 4);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Write (fp, 5);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Write (fp, 6);
for (i = 0; i < NDL; i++)
	m_info.combat [i].Write (fp, 7);
write_INT8 (m_info.cloakType, fp);
write_INT8 (m_info.attackType, fp);
m_info.sounds.Write (fp);
write_INT8 (m_info.bossFlag, fp);
write_INT8 (m_info.companion, fp);
write_INT8 (m_info.smartBlobs, fp);
write_INT8 (m_info.energyBlobs, fp);
write_INT8 (m_info.thief, fp);
write_INT8 (m_info.pursuit, fp);
write_INT8 (m_info.lightCast, fp);
write_INT8 (m_info.deathRoll, fp);
write_INT8 (m_info.flags, fp);
write_INT8 (m_info.bCustom, fp); // skip
write_INT8 (m_info.pad [1], fp); // skip
write_INT8 (m_info.pad [2], fp); // skip
(UINT8) write_INT8 (m_info.deathRollSound, fp);
(UINT8) write_INT8 (m_info.glow, fp);
(UINT8) write_INT8 (m_info.behavior, fp);
(UINT8) write_INT8 (m_info.aim, fp);
for (i = 0; i <= MAX_GUNS; i++)
	for (j = 0; j < N_ANIM_STATES; j++)
		m_info.animStates [i][j].Write (fp);
write_INT32 (m_info.always_0xabcd, fp);
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

#define MAKESIG(_sig)	(UINT32) *((INT32 *) &(_sig))

INT32 CMine::ReadHamFile(char *pszFile, INT32 type) 
{
  FILE*			fp;
  INT16			t, t0;
  UINT32			id;
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
	id = read_INT32(fp); // "HAM!"
	if (id != MAKESIG (d2HamSig)) {//0x214d4148L) {
		sprintf_s (message, sizeof (message), "Not a D2 HAM file (%s)", pszFile);
		ErrorMsg (message);
		goto abort;
		}
	read_INT32(fp); // version (0x00000007)
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof (UINT16)*t,SEEK_CUR);
	fseek(fp,sizeof (TMAP_INFO)*t,SEEK_CUR);
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof (UINT8)*t,SEEK_CUR);
	fseek(fp,sizeof (UINT8)*t,SEEK_CUR);
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof (VCLIP)*t,SEEK_CUR);
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof (ECLIP)*t,SEEK_CUR);
	t = (INT16) read_INT32(fp);
	fseek(fp,sizeof (WCLIP)*t,SEEK_CUR);
	}
else if (type == EXTENDED_HAM)  {
	id = read_INT32(fp); // "HAM!"
	if (id != MAKESIG (d2xHamSig)) {//0x214d4148L) {
		sprintf_s (message, sizeof (message), "Not a D2X HAM file (%s)", pszFile);
		ErrorMsg (message);
		goto abort;
		}
	read_INT32(fp); //skip version
	t = read_INT32(fp); //skip weapon count
	fseek (fp, t * sizeof (WEAPON_INFO), SEEK_CUR); //skip weapon info
	}

  // read robot information
  //------------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_ROBOT_TYPES;
  N_robot_types = t0 + t;
  if (N_robot_types > MAX_ROBOT_TYPES) {
    sprintf_s (message, sizeof (message), "Too many robots (%d) in <%s>.  Max is %d.",t,pszFile,MAX_ROBOT_TYPES-N_D2_ROBOT_TYPES);
    ErrorMsg (message);
	 N_robot_types = MAX_ROBOT_TYPES;
	 t = N_robot_types - t0;
//    goto abort;
  }
  RobotInfo (t0)->Read (fp);
  *DefRobotInfo (t0) = *RobotInfo (t0);
  //memcpy (DefRobotInfo (t0), RobotInfo (t0), sizeof (tRobotInfo) * t);

  // skip joints weapons, and powerups
  //----------------------------------
  t = (INT16) read_INT32(fp);
  fseek(fp,sizeof (JOINTPOS)*t,SEEK_CUR);
  if (type == NORMAL_HAM) {
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof (WEAPON_INFO)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof (POWERUP_TYPE_INFO)*t,SEEK_CUR);
  }

  // read poly model data and write it to a file
  //---------------------------------------------
  t = (INT16) read_INT32(fp);
  if (t > MAX_POLYGON_MODELS) {
    sprintf_s (message, sizeof (message), "Too many polygon models (%d) in <%s>.  Max is %d.",t,pszFile,MAX_POLYGON_MODELS-N_D2_POLYGON_MODELS);
    ErrorMsg (message);
    goto abort;
  }
#if 0
  INT16 i, j;
  FILE *fp;
  fopen_s (&fp, "d:\\bc\\dlc2data\\poly.dat", "wt");
  if (fp) {
    for (i = 0; i < t; i++ ) {
      fread(&pm,  sizeof (tPolyModel),  1,  fp );
      fprintf (fp, "n_models        = %ld\n", pmm_info.nModels);
      fprintf (fp, "model_dataSize = %ld\n", pm.m_info.modelDataSize);
      for (j = 0; j < pmm_info.nModels; j++) {
			fprintf (fp, "submodel_ptrs[%d]    = %#08lx\n", j, pm.m_info.submodel [i].ptr [j]);
      for (j = 0; j < pmm_info.nModels; j++) {
			fprintf (fp, "submodel_offsets[%d] = %#08lx %#08lx %#08lx\n", j, 
						pm.m_info.submodel [i].offset [j].v.x, pm.m_info.submodel [i].offset [j].v.y, pm.m_info.submodel [i].offset [j].v.z);
      for (j = 0; j < pmm_info.nModels; j++) {
			fprintf (fp, "submodel_norms[%d]   = %#08lx %#08lx %#08lx\n", j, 
						pm.m_info.submodel [i].norm [j].v.x, pm.m_info.submodel [i].norm [j].v.y, pm.m_info.submodel [i].norm [j].v.z);
      for (j = 0; j < pmm_info.nModels; j++) {
			fprintf (fp, "submodel_pnts[%d]    = %#08lx %#08lx %#08lx\n", j, 
						pm.m_info.submodel [i].pnt [j].v.x, pm.m_info.submodel [i].pnt [j].v.y, pm.m_info.submodel [i].pnt [j].v.z);
      for (j = 0; j < pmm_info.nModels; j++) {
			fprintf (fp, "submodel_rads[%d]    = %#08lx\n", j, pm.m_info.submodel [i].rad [j]);
      for (j = 0; j < pmm_info.nModels; j++) {
			fprintf (fp, "submodel_parents[%d] = %d\n", j, pm.m_info.submodel [i].parent [j]);
      for (j = 0; j < pmm_info.nModels; j++) {
			fprintf (fp, "submodel_mins[%d]    = %#08lx %#08lx %#08lx\n", j, 
						pm.m_info.submodel [i].min [j].v.x, pm.m_info.submodel [i].min [j].v.y, pm.m_info.submodel [i].min [j].v.z);
      for (j = 0; j < pmm_info.nModels; j++) {
			fprintf (fp, "submodel_maxs[%d]    = %#08lx %#08lx %#08lx\n", j, 
						pm.m_info.submodel [i].max [j].v.x, pm.m_info.submodel [i].max [j].v.y, pm.m_info.submodel [i].max [j].v.z);
			}
      fprintf (fp, "mins            = %#08lx %#08lx %#08lx\n", pm.mins.v.x, pm.mins.v.y, pm.mins.v.z);
      fprintf (fp, "maxs            = %#08lx %#08lx %#08lx\n", pm.maxs.v.x, pm.maxs.v.y, pm.maxs.v.z);
      fprintf (fp, "rad             = %ld\n", pm.rad);
      fprintf (fp, "textureCount      = %d\n", pm.textureCount);
      fprintf (fp, "first_texture   = %d\n", pm.first_texture);
      fprintf (fp, "simpler_model   = %d\n\n", pm.simpler_model);
    }
    fclose(fp);
  }
#endif
#if ALLOCATE_tPolyModelS
  // read joint information
  //-----------------------
  t = (INT16) read_INT32(fp);
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
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof (WEAPON_INFO)*t,SEEK_CUR);
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof (POWERUP_TYPE_INFO)*t,SEEK_CUR);
  }

  // read poly model data
  //---------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_POLYGON_MODELS;
  N_polygon_models = t0 + t;
  if (N_polygon_models > MAX_POLYGON_MODELS) {
    sprintf_s (message, sizeof (message), "Too many polygon models (%d) in <%s>.  Max is %d.",t,pszFile,MAX_POLYGON_MODELS-N_D2_POLYGON_MODELS);
    ErrorMsg (message);
    goto abort;
  }

  INT16 i;

  for (i=t0; i<t0+t; i++ ) {
    // free poly data memory if already allocated
    if (Polygon_models[i]->model_data != NULL ) {
      free((void *)Polygon_models[i]->model_data);
    }
    fread(Polygon_models[i], sizeof (tPolyModel), 1, fp );
    fread(&Polygon_model, sizeof (tPolyModel), 1, fp );
  }
  for (i=t0; i<t0+t; i++ ) {
    Polygon_models[i]->model_data = (UINT8 *) malloc((INT32)Polygon_models[i]->model_dataSize);

    if (Polygon_models[i]->model_data == NULL ) {
      ErrorMsg ("Could not allocate memory for polymodel data");
      goto abort;
    }
    fread( Polygon_models[i]->model_data, sizeof (UINT8), (INT16)Polygon_models[i]->model_dataSize, fp );
//    g3_init_polygon_model(Polygon_models[i].model_data);
  }

  // extended hog writes over normal hogs dying models instead of adding new ones
  fread( &Dying_modelnums[t0], sizeof (INT32), t, fp );
  fread( &Dead_modelnums[t0], sizeof (INT32), t, fp );

  // skip gague data
  //----------------
  if (type == NORMAL_HAM) {
    t = (INT16) read_INT32(fp);
    fseek(fp,sizeof (UINT16)*t,SEEK_CUR); // lores gague
    fseek(fp,sizeof (UINT16)*t,SEEK_CUR); // hires gague
  }

  // read object bitmap data
  //------------------------
  // NOTE: this overwrites D2 object bitmap indices instead of
  // adding more bitmap texture indicies.  I believe that D2
  // writes all 600 indicies even though it doesn't use all
  // of them.
  //----------------------------------------------------------
  t = (INT16) read_INT32(fp);
  t0 = (type == NORMAL_HAM) ? 0: N_D2_OBJBITMAPS;
  if (type == NORMAL_HAM) {
    N_object_bitmaps  = t0 + t;  // only update this if we are reading Descent2.ham file
  }
  if (t+t0 > MAX_OBJ_BITMAPS) {
    sprintf_s (message, sizeof (message), "Too many object bitmaps (%d) in <%s>.  Max is %d.",t,pszFile,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPS);
    ErrorMsg (message);
    goto abort;
  }
  fread( &ObjBitmaps[t0], sizeof (UINT16), t, fp );

  if (type == EXTENDED_HAM) {
    t = (INT16) read_INT32(fp);
    t0 = (type == NORMAL_HAM) ? 0: N_D2_OBJBITMAPPTRS;
    if (t+t0 > MAX_OBJ_BITMAPS) {
      sprintf_s (message, sizeof (message), "Too many object bitmaps pointer (%d) in <%s>.  Max is %d.",t,pszFile,MAX_OBJ_BITMAPS-N_D2_OBJBITMAPPTRS);
      ErrorMsg (message);
      goto abort;
    }
  }
  fread(&ObjBitmapPtrs[t0], sizeof (UINT16), t, fp );
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

INT32 CMine::ReadHxmFile(FILE *fp, long fSize) 
{
	UINT16 t,i,j;
	CRobotInfo rInfo;
	long p;

if (!fp) {
	ErrorMsg ("Invalid file handle for reading HXM data.");
	goto abort;
	}

p = ftell (fp);
if (fSize < 0)
	fSize = _filelength (_fileno (fp));
UINT32 id;
id = read_INT32(fp); // "HXM!"
if (id != 0x21584d48L) {
	ErrorMsg ("Not a HXM file");
	goto abort;
	}
if (m_pHxmExtraData) {
	free (m_pHxmExtraData);
	m_pHxmExtraData = NULL;
	m_nHxmExtraDataSize = 0;
	}
read_INT32(fp); // version (0x00000001)

// read robot information
//------------------------
t = (UINT16) read_INT32(fp);
for (j=0;j<t;j++) {
	i = (UINT16)read_INT32(fp);
	if (i>=N_robot_types) {
		sprintf_s (message, sizeof (message), "Robots number (%d) out of range.  Range = [0..%d].",i,N_robot_types-1);
		ErrorMsg (message);
		goto abort;
		}
	fread(&rInfo, sizeof (tRobotInfo), 1, fp );
	// compare this to existing data
	if (memcmp(&rInfo,RobotInfo (i),sizeof (tRobotInfo)) != 0) {
		memcpy(RobotInfo (i),&rInfo,sizeof (tRobotInfo));
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

INT32 CMine::WriteHxmFile(FILE *fp) 
{
UINT16 t,i;

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
UINT32 id;
id = 0x21584d48L;    // "HXM!"
write_INT32 (id,fp);
write_INT32 (1,fp);   // version 1
#endif

// write robot information
//------------------------
write_INT32 (t,fp); // number of robot info structs stored
for (i = 0; i < N_robot_types; i++) {
	if (RobotInfo (i)->m_info.bCustom) {
		write_INT32 ((UINT32) i, fp);
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
write_INT32 (0,fp);  //number of joints
write_INT32 (0,fp);  //number of polygon models
write_INT32 (0,fp);  //number of objbitmaps
write_INT32 (0,fp);  //number of objbitmaps
write_INT32 (0,fp);  //number of objbitmapptrs
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
void CMine::ReadRobotResource(INT32 robot_number) 
{
  UINT16 i,j,t;
  UINT8 *ptr;
  HRSRC hFind = FindResource( hInst,"ROBOT_HXM", "RC_DATA");
  HINSTANCE hInst = AfxGetApp ()->m_hInstance;
  HGLOBAL hResource = LoadResource( hInst, hFind);
  if (!hResource) {
    ErrorMsg ("Could not find robot resource data");
    return;
  }
  ptr = (UINT8 *)LockResource(hResource);
  if (!ptr) {
    ErrorMsg ("Could not lock robot resource data");
    return;
  }
  t = (UINT16)(*((UINT32 *)ptr));
  N_robot_types = min(t,MAX_ROBOT_TYPES);
  ptr += sizeof (UINT32);
  for (j=0;j<t;j++) {
    i = (UINT16)(*((UINT32 *)ptr));
    if (i>MAX_ROBOT_TYPES) break;
    ptr += sizeof (UINT32);
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

bool CMine::IsCustomRobot (INT32 i)
{
	bool				bFound = false;
	CSegment*		segP;
	CGameObject*	objP;
	INT32				j;

if (!RobotInfo (i)->m_info.bCustom) //changed?
	return false;
	// check if actually different from defaults
UINT8 bCustom = DefRobotInfo (i)->m_info.bCustom;
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
				INT32 matcen = segP->m_info.nMatCen;
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
INT32 i;
for (i = 0; i < (INT32) N_robot_types; i++)
	if (IsCustomRobot (i))
		return TRUE;
return (m_nHxmExtraDataSize > 0);
}

//eof robot.cpp


