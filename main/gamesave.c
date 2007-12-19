/* $Id: gamesave.c,v 1.21 2003/06/16 07:15:59 btb Exp $ */
/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#ifdef RCS
char gamesave_rcsid [] = "$Id: gamesave.c,v 1.21 2003/06/16 07:15:59 btb Exp $";
#endif

#include <stdio.h>
#include <string.h>

#include "pstypes.h"
#include "strutil.h"
#include "mono.h"
#include "key.h"
#include "gr.h"
#include "palette.h"
#include "newmenu.h"

#include "inferno.h"
#ifdef EDITOR
#include "editor/editor.h"
#endif
#include "error.h"
#include "object.h"
#include "game.h"
#include "screens.h"
#include "wall.h"
#include "gamemine.h"
#include "robot.h"
#include "fireball.h"

#include "cfile.h"
#include "bm.h"
#include "menu.h"
#include "switch.h"
#include "fuelcen.h"
#include "cntrlcen.h"
#include "powerup.h"
#include "weapon.h"
#include "newdemo.h"
#include "loadgame.h"
#include "automap.h"
#include "polyobj.h"
#include "text.h"
#include "gamefont.h"
#include "gamesave.h"
#include "gamepal.h"
#include "laser.h"
#include "byteswap.h"
#include "multi.h"
#include "makesig.h"
#include "gameseg.h"
#include "light.h"

#define GAME_VERSION            32
#define GAME_COMPATIBLE_VERSION 22

//version 28->29  add delta light support
//version 27->28  controlcen id now is reactor number, not model number
//version 28->29  ??
//version 29->30  changed tTrigger structure
//version 30->31  changed tTrigger structure some more
//version 31->32  change tSegment structure, make it 512 bytes w/o editor, add gameData.segs.segment2s array.

#define MENU_CURSOR_X_MIN       MENU_X
#define MENU_CURSOR_X_MAX       MENU_X+6

tGameFileInfo	gameFileInfo;
game_top_fileinfo	gameTopFileInfo;

//  LINT: adding function prototypes
void ReadObject(tObject *objP, CFILE *f, int version);
#ifdef EDITOR
void writeObject(tObject *objP, FILE *f);
void DoLoadSaveLevels(int save);
#endif
#ifdef _DEBUG
void dump_mine_info(void);
#endif

#ifdef EDITOR
extern char mine_filename [];
extern int save_mine_data_compiled(FILE * SaveFile);
//--unused-- #else
//--unused-- char mine_filename [128];
#endif

int nGameSavePlayers = 0;
int nGameSaveOrgRobots = 0;
int nSavePOFNames = 0;
char szSavePOFNames [MAX_POLYGON_MODELS][SHORT_FILENAME_LEN];

//--unused-- grsBitmap * Gamesave_saved_bitmap = NULL;

//------------------------------------------------------------------------------
#ifdef EDITOR
// Return true if this level has a name of the form "level??"
// Note that a pathspec can appear at the beginning of the filename.
int IsRealLevel(char *filename)
{
	int len = (int) strlen(filename);

if (len < 6)
	return 0;
return !strnicmp(&filename [len-11], "level", 5);
}
#endif

//------------------------------------------------------------------------------
//--unused-- vmsAngVec zero_angles={0,0,0};

#define VmAngVecZero(v) do {(v)->p=(v)->b=(v)->h=0;} while (0)

void CheckAndFixMatrix(vmsMatrix *m);

void VerifyObject(tObject * objP)
{
objP->lifeleft = IMMORTAL_TIME;		//all loaded tObject are immortal, for now
if (objP->nType == OBJ_ROBOT) {
	nGameSaveOrgRobots++;
	// Make sure valid id...
	if (objP->id >= gameData.bots.nTypes [gameStates.app.bD1Data])
		objP->id %= gameData.bots.nTypes [0];
	// Make sure model number & size are correct...
	if (objP->renderType == RT_POLYOBJ) {
		Assert(ROBOTINFO (objP->id).nModel != -1);
			//if you fail this assert, it means that a robot in this level
			//hasn't been loaded, possibly because he's marked as
			//non-shareware.  To see what robot number, print objP->id.
		Assert(ROBOTINFO (objP->id).always_0xabcd == 0xabcd);
			//if you fail this assert, it means that the robot_ai for
			//a robot in this level hasn't been loaded, possibly because
			//it's marked as non-shareware.  To see what robot number,
			//print objP->id.
		objP->rType.polyObjInfo.nModel = ROBOTINFO (objP->id).nModel;
		objP->size = gameData.models.polyModels [objP->rType.polyObjInfo.nModel].rad;
		}
	if (objP->id == 65)						//special "reactor" robots
		objP->movementType = MT_NONE;
	if (objP->movementType == MT_PHYSICS) {
		objP->mType.physInfo.mass = ROBOTINFO (objP->id).mass;
		objP->mType.physInfo.drag = ROBOTINFO (objP->id).drag;
		}
	}
else {		//Robots taken care of above
	if (objP->renderType == RT_POLYOBJ) {
		int i;
		char *name = szSavePOFNames [objP->rType.polyObjInfo.nModel];
		for (i = 0; i < gameData.models.nPolyModels; i++)
			if (!stricmp (Pof_names [i], name)) {		//found it!
				objP->rType.polyObjInfo.nModel = i;
				break;
				}
		}
	}
if (objP->nType == OBJ_POWERUP) {
	if (objP->id >= gameData.objs.pwrUp.nTypes) {
		objP->id = 0;
		Assert(objP->renderType != RT_POLYOBJ);
		}
	objP->controlType = CT_POWERUP;
	objP->size = gameData.objs.pwrUp.info [objP->id].size;
	objP->cType.powerupInfo.creationTime = 0;
	if (gameData.app.nGameMode & GM_NETWORK) {
	  if (MultiPowerupIs4Pack(objP->id)) {
			gameData.multiplayer.powerupsInMine [objP->id-1]+=4;
	 		gameData.multiplayer.maxPowerupsAllowed [objP->id-1]+=4;
			}
		gameData.multiplayer.powerupsInMine [objP->id]++;
		gameData.multiplayer.maxPowerupsAllowed [objP->id]++;
#if TRACE
		con_printf (CONDBG,"PowerupLimiter: ID=%d\n",objP->id);
		if (objP->id>MAX_POWERUP_TYPES)
			con_printf (1,"POWERUP: Overwriting array bounds!\n");
#endif
		}
	}
if (objP->nType == OBJ_WEAPON)	{
	if (objP->id >= gameData.weapons.nTypes [0])	{
		objP->id = 0;
		Assert(objP->renderType != RT_POLYOBJ);
		}
	if (objP->id == SMALLMINE_ID) {		//make sure pmines have correct values
		objP->mType.physInfo.mass = gameData.weapons.info [objP->id].mass;
		objP->mType.physInfo.drag = gameData.weapons.info [objP->id].drag;
		objP->mType.physInfo.flags |= PF_FREE_SPINNING;
		// Make sure model number & size are correct...	
		Assert(objP->renderType == RT_POLYOBJ);
		objP->rType.polyObjInfo.nModel = gameData.weapons.info [objP->id].nModel;
		objP->size = gameData.models.polyModels [objP->rType.polyObjInfo.nModel].rad;
		}
	}
if (objP->nType == OBJ_CNTRLCEN) {
	objP->renderType = RT_POLYOBJ;
	objP->controlType = CT_CNTRLCEN;
	if (gameData.segs.nLevelVersion <= 1) { // descent 1 reactor
		objP->id = 0;                         // used to be only one kind of reactor
		objP->rType.polyObjInfo.nModel = gameData.reactor.props [0].nModel;// descent 1 reactor
		}
#ifdef EDITOR
	{
	int i;
	// Check, and set, strength of reactor
	for (i = 0; i < gameData.objs.types.nCount; i++)
		if ((gameData.objs.types.nType [i] == OL_CONTROL_CENTER) && 
			 (gameData.objs.types.nType.nId [i] == objP->id)) {
			objP->shields = gameData.objs.types.nType.nStrength [i];
			break;	
			}
		Assert(i < gameData.objs.types.nCount);		//make sure we found it
		}
#endif
	}
if (objP->nType == OBJ_PLAYER) {
	if (objP == gameData.objs.console)	
		InitPlayerObject();
	else
		if (objP->renderType == RT_POLYOBJ)	//recover from Matt's pof file matchup bug
			objP->rType.polyObjInfo.nModel = gameData.pig.ship.player->nModel;
	//Make sure orient matrix is orthogonal
	gameOpts->render.nMathFormat = 0;
	CheckAndFixMatrix(&objP->position.mOrient);
	gameOpts->render.nMathFormat = gameOpts->render.nDefMathFormat;
	objP->id = nGameSavePlayers++;
	}
if (objP->nType == OBJ_HOSTAGE) {
	objP->renderType = RT_HOSTAGE;
	objP->controlType = CT_POWERUP;
	}
}

//------------------------------------------------------------------------------
//static gs_skip(int len,CFILE *file)
//{
//
//	CFSeek (file,len,SEEK_CUR);
//}

#ifdef EDITOR
static void gs_write_int(int i,FILE *file)
{
	if (fwrite(&i, sizeof(i), 1, file) != 1)
		Error("Error reading int in gamesave.c");

}

static void gs_write_fix(fix f,FILE *file)
{
	if (fwrite(&f, sizeof(f), 1, file) != 1)
		Error("Error reading fix in gamesave.c");

}

static void gs_write_short(short s,FILE *file)
{
	if (fwrite(&s, sizeof(s), 1, file) != 1)
		Error("Error reading short in gamesave.c");

}

static void gs_write_fixang(fixang f,FILE *file)
{
	if (fwrite(&f, sizeof(f), 1, file) != 1)
		Error("Error reading fixang in gamesave.c");

}

static void gs_write_byte(byte b,FILE *file)
{
	if (fwrite(&b, sizeof(b), 1, file) != 1)
		Error("Error reading byte in gamesave.c");

}

static void gr_write_vector(vmsVector *v,FILE *file)
{
	gs_write_fix(v->x,file);
	gs_write_fix(v->y,file);
	gs_write_fix(v->z,file);
}

static void gs_write_matrix(vmsMatrix *m,FILE *file)
{
	gr_write_vector(&m->rVec,file);
	gr_write_vector(&m->uVec,file);
	gr_write_vector(&m->fVec,file);
}

static void gs_write_angvec(vmsAngVec *v,FILE *file)
{
	gs_write_fixang(v->p,file);
	gs_write_fixang(v->b,file);
	gs_write_fixang(v->h,file);
}

#endif

//------------------------------------------------------------------------------

extern int MultiPowerupIs4Pack(int);
//reads one tObject of the given version from the given file
void ReadObject(tObject *objP,CFILE *f,int version)
{
	int	i;

objP->nType = CFReadByte(f);
objP->id = CFReadByte(f);
objP->controlType = CFReadByte(f);
objP->movementType = CFReadByte(f);
objP->renderType = CFReadByte(f);
objP->flags = CFReadByte(f);
objP->nSegment = CFReadShort(f);
objP->attachedObj = -1;
CFReadVector(&objP->position.vPos,f);
CFReadMatrix(&objP->position.mOrient,f);
objP->size = CFReadFix(f);
objP->shields = CFReadFix(f);
CFReadVector(&objP->vLastPos,f);
objP->containsType = CFReadByte(f);
objP->containsId = CFReadByte(f);
objP->containsCount = CFReadByte(f);
switch (objP->movementType) {
	case MT_PHYSICS:
		CFReadVector(&objP->mType.physInfo.velocity,f);
		CFReadVector(&objP->mType.physInfo.thrust,f);
		objP->mType.physInfo.mass		= CFReadFix(f);
		objP->mType.physInfo.drag		= CFReadFix(f);
		objP->mType.physInfo.brakes	= CFReadFix(f);
		CFReadVector(&objP->mType.physInfo.rotVel,f);
		CFReadVector(&objP->mType.physInfo.rotThrust,f);
		objP->mType.physInfo.turnRoll	= CFReadFixAng(f);
		objP->mType.physInfo.flags		= CFReadShort(f);
		break;

	case MT_SPINNING:
		CFReadVector(&objP->mType.spinRate,f);
		break;

	case MT_NONE:
		break;

	default:
		Int3();
	}

switch (objP->controlType) {
	case CT_AI: 
		objP->cType.aiInfo.behavior = CFReadByte(f);
		for (i=0;i<MAX_AI_FLAGS;i++)
			objP->cType.aiInfo.flags [i] = CFReadByte(f);
		objP->cType.aiInfo.nHideSegment = CFReadShort(f);
		objP->cType.aiInfo.nHideIndex = CFReadShort(f);
		objP->cType.aiInfo.nPathLength = CFReadShort(f);
		objP->cType.aiInfo.nCurPathIndex = (char) CFReadShort(f);
		if (version <= 25) {
			CFReadShort(f);	//				objP->cType.aiInfo.follow_path_start_seg	= 
			CFReadShort(f);	//				objP->cType.aiInfo.follow_path_end_seg		= 
			}
		break;

	case CT_EXPLOSION:
		objP->cType.explInfo.nSpawnTime		= CFReadFix(f);
		objP->cType.explInfo.nDeleteTime		= CFReadFix(f);
		objP->cType.explInfo.nDeleteObj	= CFReadShort(f);
		objP->cType.explInfo.nNextAttach = objP->cType.explInfo.nPrevAttach = objP->cType.explInfo.nAttachParent = -1;
		break;

	case CT_WEAPON: //do I really need to read these?  Are they even saved to disk?
		objP->cType.laserInfo.parentType		= CFReadShort(f);
		objP->cType.laserInfo.nParentObj		= CFReadShort(f);
		objP->cType.laserInfo.nParentSig	= CFReadInt(f);
		break;

	case CT_LIGHT:
		objP->cType.lightInfo.intensity = CFReadFix(f);
		break;

	case CT_POWERUP:
		if (version >= 25)
			objP->cType.powerupInfo.count = CFReadInt(f);
		else
			objP->cType.powerupInfo.count = 1;
		if (objP->id == POW_VULCAN)
			objP->cType.powerupInfo.count = VULCAN_WEAPON_AMMO_AMOUNT;
		if (objP->id == POW_GAUSS)
			objP->cType.powerupInfo.count = VULCAN_WEAPON_AMMO_AMOUNT;
		if (objP->id == POW_OMEGA)
			objP->cType.powerupInfo.count = MAX_OMEGA_CHARGE;
		break;

	case CT_NONE:
	case CT_FLYING:
	case CT_DEBRIS:
		break;

	case CT_SLEW:		//the tPlayer is generally saved as slew
		break;

	case CT_CNTRLCEN:
		break;

	case CT_MORPH:
	case CT_FLYTHROUGH:
	case CT_REPAIRCEN:
		default:
		Int3();
	}

switch (objP->renderType) {
	case RT_NONE:
		break;

	case RT_MORPH:
	case RT_POLYOBJ: {
		int i,tmo;
		objP->rType.polyObjInfo.nModel = CFReadInt(f);
		for (i=0;i<MAX_SUBMODELS;i++)
			CFReadAngVec(&objP->rType.polyObjInfo.animAngles [i],f);
		objP->rType.polyObjInfo.nSubObjFlags = CFReadInt(f);
		tmo = CFReadInt(f);
#ifndef EDITOR
		objP->rType.polyObjInfo.nTexOverride = tmo;
#else
		if (tmo==-1)
			objP->rType.polyObjInfo.nTexOverride = -1;
		else {
			int xlated_tmo = tmap_xlate_table [tmo];
			if (xlated_tmo < 0)	{
#if TRACE
				con_printf (CONDBG, "Couldn't find texture for demo tObject, nModel = %d\n", objP->rType.polyObjInfo.nModel);
#endif
				Int3();
				xlated_tmo = 0;
				}
			objP->rType.polyObjInfo.nTexOverride	= xlated_tmo;
			}
#endif
		objP->rType.polyObjInfo.nAltTextures	= 0;
		break;
		}

	case RT_WEAPON_VCLIP:
	case RT_HOSTAGE:
	case RT_POWERUP:
	case RT_FIREBALL:
		objP->rType.vClipInfo.nClipIndex	= CFReadInt(f);
		objP->rType.vClipInfo.xFrameTime	= CFReadFix(f);
		objP->rType.vClipInfo.nCurFrame	= CFReadByte(f);
		break;

	case RT_THRUSTER:
	case RT_LASER:
		break;

	case RT_SMOKE:
		objP->rType.smokeInfo.nLife = CFReadInt (f);
		objP->rType.smokeInfo.nSize [0] = CFReadInt (f);
		objP->rType.smokeInfo.nParts = CFReadInt (f);
		objP->rType.smokeInfo.nSpeed = CFReadInt (f);
		objP->rType.smokeInfo.nDrift = CFReadInt (f);
		objP->rType.smokeInfo.nBrightness = CFReadInt (f);
		objP->rType.smokeInfo.color.red = CFReadByte (f);
		objP->rType.smokeInfo.color.green = CFReadByte (f);
		objP->rType.smokeInfo.color.blue = CFReadByte (f);
		objP->rType.smokeInfo.color.alpha = CFReadByte (f);
		objP->rType.smokeInfo.nSide = CFReadByte (f);
		break;

	case RT_LIGHTNING:
		objP->rType.lightningInfo.nLife = CFReadInt (f);
		objP->rType.lightningInfo.nDelay = CFReadInt (f);
		objP->rType.lightningInfo.nLength = CFReadInt (f);
		objP->rType.lightningInfo.nAmplitude = CFReadInt (f);
		objP->rType.lightningInfo.nOffset = CFReadInt (f);
		objP->rType.lightningInfo.nLightnings = CFReadShort (f);
		objP->rType.lightningInfo.nId = CFReadShort (f);
		objP->rType.lightningInfo.nTarget = CFReadShort (f);
		objP->rType.lightningInfo.nNodes = CFReadShort (f);
		objP->rType.lightningInfo.nChildren = CFReadShort (f);
		objP->rType.lightningInfo.nSteps = CFReadShort (f);
		objP->rType.lightningInfo.nAngle = CFReadByte (f);
		objP->rType.lightningInfo.nStyle = CFReadByte (f);
		objP->rType.lightningInfo.nSmoothe = CFReadByte (f);
		objP->rType.lightningInfo.bClamp = CFReadByte (f);
		objP->rType.lightningInfo.bPlasma = CFReadByte (f);
		objP->rType.lightningInfo.bSound = CFReadByte (f);
		objP->rType.lightningInfo.bRandom = CFReadByte (f);
		objP->rType.lightningInfo.bInPlane = CFReadByte (f);
		objP->rType.lightningInfo.color.red = CFReadByte (f);
		objP->rType.lightningInfo.color.green = CFReadByte (f);
		objP->rType.lightningInfo.color.blue = CFReadByte (f);
		objP->rType.lightningInfo.color.alpha = CFReadByte (f);
		break;

	default:
		Int3();
	}
}

//------------------------------------------------------------------------------
#ifdef EDITOR

//writes one tObject to the given file
void writeObject(tObject *objP,FILE *f)
{
	gs_write_byte(objP->nType,f);
	gs_write_byte(objP->id,f);

	gs_write_byte(objP->controlType,f);
	gs_write_byte(objP->movementType,f);
	gs_write_byte(objP->renderType,f);
	gs_write_byte(objP->flags,f);

	gs_write_short(objP->nSegment,f);

	gr_write_vector(&objP->position.vPos,f);
	gs_write_matrix(&objP->position.mOrient,f);

	gs_write_fix(objP->size,f);
	gs_write_fix(objP->shields,f);

	gr_write_vector(&objP->vLastPos,f);

	gs_write_byte(objP->containsType,f);
	gs_write_byte(objP->containsId,f);
	gs_write_byte(objP->containsCount,f);

	switch (objP->movementType) {

		case MT_PHYSICS:

	 		gr_write_vector(&objP->mType.physInfo.velocity,f);
			gr_write_vector(&objP->mType.physInfo.thrust,f);

			gs_write_fix(objP->mType.physInfo.mass,f);
			gs_write_fix(objP->mType.physInfo.drag,f);
			gs_write_fix(objP->mType.physInfo.brakes,f);

			gr_write_vector(&objP->mType.physInfo.rotVel,f);
			gr_write_vector(&objP->mType.physInfo.rotThrust,f);

			gs_write_fixang(objP->mType.physInfo.turnRoll,f);
			gs_write_short(objP->mType.physInfo.flags,f);

			break;

		case MT_SPINNING:

			gr_write_vector(&objP->mType.spinRate,f);
			break;

		case MT_NONE:
			break;

		default:
			Int3();
	}

	switch (objP->controlType) {

		case CT_AI: {
			int i;

			gs_write_byte(objP->cType.aiInfo.behavior,f);

			for (i=0;i<MAX_AI_FLAGS;i++)
				gs_write_byte(objP->cType.aiInfo.flags [i],f);

			gs_write_short(objP->cType.aiInfo.nHideSegment,f);
			gs_write_short(objP->cType.aiInfo.nHideIndex,f);
			gs_write_short(objP->cType.aiInfo.nPathLength,f);
			gs_write_short(objP->cType.aiInfo.nCurPathIndex,f);

			// -- unused! mk, 08/13/95 -- gs_write_short(objP->cType.aiInfo.follow_path_start_seg,f);
			// -- unused! mk, 08/13/95 -- gs_write_short(objP->cType.aiInfo.follow_path_end_seg,f);

			break;
		}

		case CT_EXPLOSION:

			gs_write_fix(objP->cType.explInfo.nSpawnTime,f);
			gs_write_fix(objP->cType.explInfo.nDeleteTime,f);
			gs_write_short(objP->cType.explInfo.nDeleteObj,f);

			break;

		case CT_WEAPON:

			//do I really need to write these gameData.objs.objects?

			gs_write_short(objP->cType.laserInfo.parentType,f);
			gs_write_short(objP->cType.laserInfo.nParentObj,f);
			gs_write_int(objP->cType.laserInfo.nParentSig,f);

			break;

		case CT_LIGHT:

			gs_write_fix(objP->cType.lightInfo.intensity,f);
			break;

		case CT_POWERUP:

			gs_write_int(objP->cType.powerupInfo.count,f);
			break;

		case CT_NONE:
		case CT_FLYING:
		case CT_DEBRIS:
			break;

		case CT_SLEW:		//the tPlayer is generally saved as slew
			break;

		case CT_CNTRLCEN:
			break;			//control center tObject.

		case CT_MORPH:
		case CT_REPAIRCEN:
		case CT_FLYTHROUGH:
		default:
			Int3();

	}

	switch (objP->renderType) {

		case RT_NONE:
			break;

		case RT_MORPH:
		case RT_POLYOBJ: {
			int i;

			gs_write_int(objP->rType.polyObjInfo.nModel,f);

			for (i=0;i<MAX_SUBMODELS;i++)
				gs_write_angvec(&objP->rType.polyObjInfo.animAngles [i],f);

			gs_write_int(objP->rType.polyObjInfo.nSubObjFlags,f);

			gs_write_int(objP->rType.polyObjInfo.nTexOverride,f);

			break;
		}

		case RT_WEAPON_VCLIP:
		case RT_HOSTAGE:
		case RT_POWERUP:
		case RT_FIREBALL:

			gs_write_int(objP->rType.vClipInfo.nClipIndex,f);
			gs_write_fix(objP->rType.vClipInfo.xFrameTime,f);
			gs_write_byte(objP->rType.vClipInfo.nCurFrame,f);

			break;

		case RT_THRUSTER:
		case RT_LASER:
			break;

		default:
			Int3();

	}

}
#endif

extern int RemoveTriggerNum (int trigger_num);

// -----------------------------------------------------------------------------

#define	DLIndex(_i)	(gameData.render.lights.deltaIndices + (_i))

void SortDLIndexD2X (int left, int right)
{
	int	l = left,
			r = right,
			m = (left + right) / 2;
	short	mSeg = DLIndex (m)->d2x.nSegment, 
			mSide = DLIndex (m)->d2x.nSide;
	tLightDeltaIndex	*pl, *pr;

do {
	pl = DLIndex (l);
	while ((pl->d2x.nSegment < mSeg) || ((pl->d2x.nSegment == mSeg) && (pl->d2x.nSide < mSide))) {
		pl++;
		l++;
		}
	pr = DLIndex (r);
	while ((pr->d2x.nSegment > mSeg) || ((pr->d2x.nSegment == mSeg) && (pr->d2x.nSide > mSide))) {
		pr--;
		r--;
		}
	if (l <= r) {
		if (l < r) {
			tLightDeltaIndex	h = *pl;
			*pl = *pr;
			*pr = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (right > l)
   SortDLIndexD2X (l, right);
if (r > left)
   SortDLIndexD2X (left, r);
}

// -----------------------------------------------------------------------------

void SortDLIndexD2 (int left, int right)
{
	int	l = left,
			r = right,
			m = (left + right) / 2;
	short	mSeg = DLIndex (m)->d2.nSegment, 
			mSide = DLIndex (m)->d2.nSide;
	tLightDeltaIndex	*pl, *pr;

do {
	pl = DLIndex (l);
	while ((pl->d2.nSegment < mSeg) || ((pl->d2.nSegment == mSeg) && (pl->d2.nSide < mSide))) {
		pl++;
		l++;
		}
	pr = DLIndex (r);
	while ((pr->d2.nSegment > mSeg) || ((pr->d2.nSegment == mSeg) && (pr->d2.nSide > mSide))) {
		pr--;
		r--;
		}
	if (l <= r) {
		if (l < r) {
			tLightDeltaIndex	h = *pl;
			*pl = *pr;
			*pr = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (right > l)
   SortDLIndexD2 (l, right);
if (r > left)
   SortDLIndexD2 (left, r);
}

// -----------------------------------------------------------------------------

void SortDLIndex (void)
{
if (gameStates.render.bD2XLights)
	SortDLIndexD2X (0, gameFileInfo.lightDeltaIndices.count - 1);
else
	SortDLIndexD2 (0, gameFileInfo.lightDeltaIndices.count - 1);
}

// -----------------------------------------------------------------------------

static void InitGameFileInfo (void)
{
gameFileInfo.level				=	-1;
gameFileInfo.player.offset		=	-1;
gameFileInfo.player.size		=	sizeof(tPlayer);
gameFileInfo.objects.offset	=	-1;
gameFileInfo.objects.count		=	0;
gameFileInfo.objects.size		=	sizeof(tObject);  
gameFileInfo.walls.offset		=	-1;
gameFileInfo.walls.count		=	0;
gameFileInfo.walls.size			=	sizeof(tWall);  
gameFileInfo.doors.offset		=	-1;
gameFileInfo.doors.count		=	0;
gameFileInfo.doors.size			=	sizeof(tActiveDoor);  
gameFileInfo.triggers.offset	=	-1;
gameFileInfo.triggers.count	=	0;
gameFileInfo.triggers.size		=	sizeof(tTrigger);  
gameFileInfo.control.offset	=	-1;
gameFileInfo.control.count		=	0;
gameFileInfo.control.size		=	sizeof(tReactorTriggers);
gameFileInfo.botGen.offset		=	-1;
gameFileInfo.botGen.count		=	0;
gameFileInfo.botGen.size		=	sizeof(tMatCenInfo);
gameFileInfo.equipGen.offset	=	-1;
gameFileInfo.equipGen.count	=	0;
gameFileInfo.equipGen.size		=	sizeof(tMatCenInfo);
gameFileInfo.lightDeltaIndices.offset = -1;
gameFileInfo.lightDeltaIndices.count =	0;
gameFileInfo.lightDeltaIndices.size =	sizeof(tLightDeltaIndex);
gameFileInfo.lightDeltas.offset	=	-1;
gameFileInfo.lightDeltas.count	=	0;
gameFileInfo.lightDeltas.size		=	sizeof(tLightDelta);
}

// -----------------------------------------------------------------------------

static int ReadGameFileInfo (CFILE *fp, int nStartOffset)
{
gameTopFileInfo.fileinfo_signature = CFReadShort (fp);
gameTopFileInfo.fileinfo_version = CFReadShort (fp);
gameTopFileInfo.fileinfo_sizeof = CFReadInt (fp);
// Check signature
if (gameTopFileInfo.fileinfo_signature != 0x6705)
	return -1;
// Check version number
if (gameTopFileInfo.fileinfo_version < GAME_COMPATIBLE_VERSION)
	return -1;
// Now, Read in the fileinfo
if (CFSeek (fp, nStartOffset, SEEK_SET)) 
	Error ("Error seeking to gameFileInfo in gamesave.c");
gameFileInfo.fileinfo_signature = CFReadShort (fp);
gameFileInfo.fileinfo_version = CFReadShort (fp);
gameFileInfo.fileinfo_sizeof = CFReadInt (fp);
CFRead (gameFileInfo.mine_filename, sizeof (char), 15, fp);
gameFileInfo.level = CFReadInt (fp);
gameFileInfo.player.offset = CFReadInt (fp);				// Player info
gameFileInfo.player.size = CFReadInt (fp);
gameFileInfo.objects.offset = CFReadInt (fp);				// Object info
gameFileInfo.objects.count = CFReadInt (fp);    
gameFileInfo.objects.size = CFReadInt (fp);  
gameFileInfo.walls.offset = CFReadInt (fp);
gameFileInfo.walls.count = CFReadInt (fp);
gameFileInfo.walls.size = CFReadInt (fp);
gameFileInfo.doors.offset = CFReadInt (fp);
gameFileInfo.doors.count = CFReadInt (fp);
gameFileInfo.doors.size = CFReadInt (fp);
gameFileInfo.triggers.offset = CFReadInt (fp);
gameFileInfo.triggers.count = CFReadInt (fp);
gameFileInfo.triggers.size = CFReadInt (fp);
gameFileInfo.links.offset = CFReadInt (fp);
gameFileInfo.links.count = CFReadInt (fp);
gameFileInfo.links.size = CFReadInt (fp);
gameFileInfo.control.offset = CFReadInt (fp);
gameFileInfo.control.count = CFReadInt (fp);
gameFileInfo.control.size = CFReadInt (fp);
gameFileInfo.botGen.offset = CFReadInt (fp);
gameFileInfo.botGen.count = CFReadInt (fp);
gameFileInfo.botGen.size = CFReadInt (fp);
if (gameTopFileInfo.fileinfo_version >= 29) {
	gameFileInfo.lightDeltaIndices.offset = CFReadInt (fp);
	gameFileInfo.lightDeltaIndices.count = CFReadInt (fp);
	gameFileInfo.lightDeltaIndices.size = CFReadInt (fp);

	gameFileInfo.lightDeltas.offset = CFReadInt (fp);
	gameFileInfo.lightDeltas.count = CFReadInt (fp);
	gameFileInfo.lightDeltas.size = CFReadInt (fp);
	}
if (gameData.segs.nLevelVersion >= 17) {
	gameFileInfo.equipGen.offset = CFReadInt (fp);
	gameFileInfo.equipGen.count = CFReadInt (fp);
	gameFileInfo.equipGen.size = CFReadInt (fp);
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadLevelInfo (CFILE *fp)
{
if (gameTopFileInfo.fileinfo_version >= 31) { //load mine filename
	// read newline-terminated string, not sure what version this changed.
	CFGetS (gameData.missions.szCurrentLevel, sizeof (gameData.missions.szCurrentLevel), fp);

	if (gameData.missions.szCurrentLevel [strlen (gameData.missions.szCurrentLevel) - 1] == '\n')
		gameData.missions.szCurrentLevel [strlen (gameData.missions.szCurrentLevel) - 1] = 0;
}
else if (gameTopFileInfo.fileinfo_version >= 14) { //load mine filename
	// read null-terminated string
	char *p = gameData.missions.szCurrentLevel;
	//must do read one char at a time, since no CFGetS()
	do {
		*p = CFGetC (fp);
		} while (*p++);
}
else
	gameData.missions.szCurrentLevel [0] = 0;
if (gameTopFileInfo.fileinfo_version >= 19) {	//load pof names
	nSavePOFNames = CFReadShort (fp);
	if ((nSavePOFNames != 0x614d) && (nSavePOFNames != 0x5547)) { // "Ma"de w/DMB beta/"GU"ILE
		if (nSavePOFNames >= MAX_POLYGON_MODELS)
			return -1;
		CFRead (szSavePOFNames, nSavePOFNames, SHORT_FILENAME_LEN, fp);
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadObjectInfo (CFILE *fp)
{
	int	i;

if (gameFileInfo.objects.offset > -1) {
	tObject	*objP = gameData.objs.objects;
	if (CFSeek (fp, gameFileInfo.objects.offset, SEEK_SET)) {
		Error ("Error seeking to object data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.objects.count; i++, objP++) {
		ReadObject (objP, fp, gameTopFileInfo.fileinfo_version);
		objP->nSignature = gameData.objs.nNextSignature++;
		VerifyObject (objP);
		gameData.objs.init [i] = *objP;
		}
	}
for (i = 0; i < MAX_OBJECTS - 1; i++)
	gameData.objs.dropInfo [i].nNextPowerup = i + 1;
gameData.objs.dropInfo [i].nNextPowerup = -1;
gameData.objs.nFirstDropped =
gameData.objs.nLastDropped = -1;
gameData.objs.nFreeDropped = 0;
return 0;
}

// -----------------------------------------------------------------------------

static int ReadWallInfo (CFILE *fp)
{
if (gameFileInfo.walls.offset > -1) {
	int	i;
	if (CFSeek (fp, gameFileInfo.walls.offset, SEEK_SET)) {
		Error ("Error seeking to wall data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i <gameFileInfo.walls.count; i++) {
		if (gameTopFileInfo.fileinfo_version >= 20)
			ReadWall(&gameData.walls.walls [i], fp); // v20 walls and up.
		else if (gameTopFileInfo.fileinfo_version >= 17) {
			tWallV19 w;

			ReadWallV19(&w, fp);
			gameData.walls.walls [i].nSegment	   = w.nSegment;
			gameData.walls.walls [i].nSide			= w.nSide;
			gameData.walls.walls [i].nLinkedWall	= w.nLinkedWall;
			gameData.walls.walls [i].nType			= w.nType;
			gameData.walls.walls [i].flags			= w.flags;
			gameData.walls.walls [i].hps				= w.hps;
			gameData.walls.walls [i].nTrigger		= w.nTrigger;
			gameData.walls.walls [i].nClip			= w.nClip;
			gameData.walls.walls [i].keys				= w.keys;
			gameData.walls.walls [i].state			= WALL_DOOR_CLOSED;
			}
		else {
			tWallV16 w;

			ReadWallV16(&w, fp);
			gameData.walls.walls [i].nSegment = gameData.walls.walls [i].nSide = gameData.walls.walls [i].nLinkedWall = -1;
			gameData.walls.walls [i].nType		= w.nType;
			gameData.walls.walls [i].flags		= w.flags;
			gameData.walls.walls [i].hps			= w.hps;
			gameData.walls.walls [i].nTrigger	= w.nTrigger;
			gameData.walls.walls [i].nClip		= w.nClip;
			gameData.walls.walls [i].keys			= w.keys;
			}
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadDoorInfo (CFILE *fp)
{
if (gameFileInfo.doors.offset > -1) {
	int	i;
	if (CFSeek (fp, gameFileInfo.doors.offset, SEEK_SET))	{
		Error ("Error seeking to door data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.doors.count; i++) {
		if (gameTopFileInfo.fileinfo_version >= 20)
			ReadActiveDoor (&gameData.walls.activeDoors [i], fp); // version 20 and up
		else {
			v19_door d;
			int p;
			short nConnSeg, nConnSide;

			ReadActiveDoorV19(&d, fp);
			gameData.walls.activeDoors [i].nPartCount = d.nPartCount;
			for (p = 0; p < d.nPartCount; p++) {
				nConnSeg = gameData.segs.segments [d.seg [p]].children [d.nSide [p]];
				nConnSide = FindConnectedSide(gameData.segs.segments + d.seg [p], gameData.segs.segments + nConnSeg);
				gameData.walls.activeDoors [i].nFrontWall [p] = WallNumI (d.seg [p], d.nSide [p]);
				gameData.walls.activeDoors [i].nBackWall [p] = WallNumI (nConnSeg, nConnSide);
				}
			}
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadTriggerInfo (CFILE *fp)
{
	int		h, i, j;
	tTrigger	*trigP;

if (gameFileInfo.triggers.offset > -1) {
#if TRACE
	con_printf(CONDBG, "   loading tTrigger data ...\n");
#endif
	if (CFSeek (fp, gameFileInfo.triggers.offset, SEEK_SET)) {
		Error ("Error seeking to trigger data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0, trigP = gameData.trigs.triggers; i < gameFileInfo.triggers.count; i++, trigP++) {
		if (gameTopFileInfo.fileinfo_version >= 31) 
			TriggerRead (trigP, fp, 0);
		else {
			tTriggerV30 trig;
			int t, nType = 0, flags = 0;
			if (gameTopFileInfo.fileinfo_version == 30)
				V30TriggerRead (&trig, fp);
			else {
				tTriggerV29 trig29;
				V29TriggerRead (&trig29, fp);
				trig.flags = trig29.flags;
				trig.nLinks	= (char) trig29.nLinks;
				trig.value = trig29.value;
				trig.time = trig29.time;
				for (t = 0; t < trig.nLinks; t++) {
					trig.nSegment [t] = trig29.nSegment [t];
					trig.nSide [t] = trig29.nSide [t];
					}
				}
			//Assert(trig.flags & TRIGGER_ON);
			trig.flags &= ~TRIGGER_ON;
			if (trig.flags & TRIGGER_CONTROL_DOORS)
				nType = TT_OPEN_DOOR;
			else if (trig.flags & TRIGGER_SHIELD_DAMAGE)
				nType = TT_SHIELD_DAMAGE;
			else if (trig.flags & TRIGGER_ENERGY_DRAIN)
				nType = TT_ENERGY_DRAIN;
			else if (trig.flags & TRIGGER_EXIT)
				nType = TT_EXIT;
			else if (trig.flags & TRIGGER_MATCEN)
				nType = TT_MATCEN;
			else if (trig.flags & TRIGGER_ILLUSION_OFF)
				nType = TT_ILLUSION_OFF;
			else if (trig.flags & TRIGGER_SECRET_EXIT)
				nType = TT_SECRET_EXIT;
			else if (trig.flags & TRIGGER_ILLUSION_ON)
				nType = TT_ILLUSION_ON;
			else if (trig.flags & TRIGGER_UNLOCK_DOORS)
				nType = TT_UNLOCK_DOOR;
			else if (trig.flags & TRIGGER_OPEN_WALL)
				nType = TT_OPEN_WALL;
			else if (trig.flags & TRIGGER_CLOSE_WALL)
				nType = TT_CLOSE_WALL;
			else if (trig.flags & TRIGGER_ILLUSORY_WALL)
				nType = TT_ILLUSORY_WALL;
			else
				Int3();
			if (trig.flags & TRIGGER_ONE_SHOT)
				flags = TF_ONE_SHOT;

			trigP->nType = nType;
			trigP->flags = flags;
			trigP->nLinks = trig.nLinks;
			trigP->nLinks = trig.nLinks;
			trigP->value = trig.value;
			trigP->time = trig.time;
			for (t = 0; t < trig.nLinks; t++) {
				trigP->nSegment [t] = trig.nSegment [t];
				trigP->nSide [t] = trig.nSide [t];
				}
			}
		if (trigP->nLinks < 0)
			trigP->nLinks = 0;
		else if (trigP->nLinks > MAX_TRIGGER_TARGETS)
			trigP->nLinks = MAX_TRIGGER_TARGETS;
		for (h = trigP->nLinks, j = 0; j < h; ) {
			if ((trigP->nSegment [j] >= 0) && (trigP->nSegment [j] < gameData.segs.nSegments) &&
				 (trigP->nSide [j] >= 0) || (trigP->nSide [j] < 6))
				j++;
			else if (--h) {
				trigP->nSegment [j] = trigP->nSegment [h];
				trigP->nSide [j] = trigP->nSide [h];
				}
			}
		trigP->nLinks = h;
		}
	if (gameTopFileInfo.fileinfo_version >= 33) {
		gameData.trigs.nObjTriggers = CFReadInt (fp);
		if (gameData.trigs.nObjTriggers) {
			for (i = 0; i < gameData.trigs.nObjTriggers; i++)
				TriggerRead (gameData.trigs.objTriggers + i, fp, 1);
			for (i = 0; i < gameData.trigs.nObjTriggers; i++) {
				gameData.trigs.objTriggerRefs [i].prev = CFReadShort (fp);
				gameData.trigs.objTriggerRefs [i].next = CFReadShort (fp);
				gameData.trigs.objTriggerRefs [i].nObject = CFReadShort (fp);
				}
			}
		if (gameTopFileInfo.fileinfo_version < 36) {
			for (i = 0; i < 700; i++)
				gameData.trigs.firstObjTrigger [i] = CFReadShort (fp);
			}
		else {
			memset (gameData.trigs.firstObjTrigger, 0xff, sizeof (gameData.trigs.firstObjTrigger));
			for (i = CFReadShort (fp); i; i--) {
				j = CFReadShort (fp);
				gameData.trigs.firstObjTrigger [j] = CFReadShort (fp);
				}
			}
		}
	else {
		gameData.trigs.nObjTriggers = 0;
		memset (gameData.trigs.objTriggers, 0, sizeof (tTrigger) * MAX_OBJ_TRIGGERS);
		memset (gameData.trigs.objTriggerRefs, 0xff, sizeof (tObjTriggerRef) * MAX_OBJ_TRIGGERS);
		memset (gameData.trigs.firstObjTrigger, 0xff, sizeof (gameData.trigs.firstObjTrigger));
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadReactorInfo (CFILE *fp)
{
if (gameFileInfo.control.offset > -1) {
#if TRACE
	con_printf(CONDBG, "   loading reactor data ...\n");
#endif
	if (CFSeek (fp, gameFileInfo.control.offset, SEEK_SET)) {
		Error ("Error seeking to reactor data\n(file damaged or invalid)");
		return -1;
		}
	ControlCenterTriggersReadN (&gameData.reactor.triggers, gameFileInfo.control.count, fp);
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadBotGenInfo (CFILE *fp)
{
if (gameFileInfo.botGen.offset > -1) {
	int	i, j;

	if (CFSeek (fp, gameFileInfo.botGen.offset, SEEK_SET)) {
		Error ("Error seeking to robot generator data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.botGen.count; i++) {
		if (gameTopFileInfo.fileinfo_version < 27) {
			old_tMatCenInfo m;

			OldMatCenInfoRead (&m, fp);

			gameData.matCens.botGens [i].objFlags [0] = m.objFlags;
			gameData.matCens.botGens [i].objFlags [1] = 0;
			gameData.matCens.botGens [i].xHitPoints = m.xHitPoints;
			gameData.matCens.botGens [i].xInterval = m.xInterval;
			gameData.matCens.botGens [i].nSegment = m.nSegment;
			gameData.matCens.botGens [i].nFuelCen = m.nFuelCen;
		}
		else
			MatCenInfoRead (gameData.matCens.botGens + i, fp);

		//	Set links in gameData.matCens.botGens to gameData.matCens.fuelCenters array
		for (j = 0; j <= gameData.segs.nLastSegment; j++)
			if ((gameData.segs.segment2s [j].special == SEGMENT_IS_ROBOTMAKER) &&
					(gameData.segs.segment2s [j].nMatCen == i)) {
				gameData.matCens.botGens [i].nFuelCen = gameData.segs.segment2s [j].value;
				break;
				}
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadEquipGenInfo (CFILE *fp)
{
if (gameFileInfo.equipGen.offset > -1) {
	int	i, j;

	if (CFSeek (fp, gameFileInfo.equipGen.offset, SEEK_SET)) {
		Error ("Error seeking to equipment generator data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.equipGen.count; i++) {
		MatCenInfoRead (gameData.matCens.equipGens + i, fp);
		//	Set links in gameData.matCens.botGens to gameData.matCens.fuelCenters array
		for (j = 0; j <= gameData.segs.nLastSegment; j++)
			if ((gameData.segs.segment2s [j].special == SEGMENT_IS_EQUIPMAKER) &&
					(gameData.segs.segment2s [j].nMatCen == i))
				gameData.matCens.equipGens [i].nFuelCen = gameData.segs.segment2s [j].value;
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static int ReadLightDeltaIndexInfo (CFILE *fp)
{
if (gameFileInfo.lightDeltaIndices.offset > -1) {
	int	i;

	if (CFSeek (fp, gameFileInfo.lightDeltaIndices.offset, SEEK_SET)) {
		Error ("Error seeking to light delta index data\n(file damaged or invalid)");
		return -1;
		}
	gameData.render.lights.nStatic = gameFileInfo.lightDeltaIndices.count;
	if (gameTopFileInfo.fileinfo_version < 29) {
#if TRACE
		con_printf (CONDBG, "Warning: Old mine version.  Not reading gameData.render.lights.deltaIndices info.\n");
#endif
		Int3();	//shouldn't be here!!!
		return 0;
		}
	else {
		for (i = 0; i < gameFileInfo.lightDeltaIndices.count; i++) {
			//LogErr ("reading DL index %d\n", i);
			ReadLightDeltaIndex (gameData.render.lights.deltaIndices + i, fp);
			}
		}
	}
SortDLIndex ();
return 0;
}

// -----------------------------------------------------------------------------

static int ReadLightDeltaInfo (CFILE *fp)
{
if (gameFileInfo.lightDeltas.offset > -1) {
	int	i;

#if TRACE
	con_printf(CONDBG, "   loading light data ...\n");
#endif
	if (CFSeek (fp, gameFileInfo.lightDeltas.offset, SEEK_SET)) {
		Error ("Error seeking to light delta data\n(file damaged or invalid)");
		return -1;
		}
	for (i = 0; i < gameFileInfo.lightDeltas.count; i++) {
		if (gameTopFileInfo.fileinfo_version >= 29) 
			ReadLightDelta (gameData.render.lights.deltas + i, fp);
		else {
#if TRACE
			con_printf (CONDBG, "Warning: Old mine version.  Not reading delta light info.\n");
#endif
			}
		}
	}
return 0;
}

// -----------------------------------------------------------------------------

static void CheckAndLinkObjects (void)
{
	int	i;

for (i = 0; i < gameFileInfo.objects.count; i++) {
	gameData.objs.objects [i].next = gameData.objs.objects [i].prev = -1;
	if (gameData.objs.objects [i].nType != OBJ_NONE) {
		int objsegnum = gameData.objs.objects [i].nSegment;
		if ((objsegnum < 0) || (objsegnum > gameData.segs.nLastSegment))	
			gameData.objs.objects [i].nType = OBJ_NONE;
		else {
			gameData.objs.objects [i].nSegment = -1;	
			LinkObject (i, objsegnum);
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Make sure non-transparent doors are set correctly.
static void CheckAndFixDoors (void)
{
	int	i, j;
	tSide	*sideP;

for (i = 0; i < gameData.segs.nSegments; i++) {
	sideP = gameData.segs.segments [i].sides;
	for (j = 0; j < MAX_SIDES_PER_SEGMENT; j++, sideP++) {
		short nWall = WallNumS (sideP);
		tWall  *w;
		if (!IS_WALL (nWall))
			continue;
		w = gameData.walls.walls + nWall;
		if (w->nClip == -1)
			continue;
		if (gameData.walls.pAnims [w->nClip].flags & WCF_TMAP1) {
			sideP->nBaseTex = gameData.walls.pAnims [w->nClip].frames [0];
			sideP->nOvlTex = 0;
			}
		}
	}
}

// -----------------------------------------------------------------------------
//go through all walls, killing references to invalid triggers
static void CheckAndFixWalls (void)
{
	int	i;
	short	nSegment, nSide, nWall;

for (i = 0; i < gameData.walls.nWalls; i++)
	if (gameData.walls.walls [i].nTrigger >= gameData.trigs.nTriggers) {
#if TRACE
		con_printf (CONDBG,"Removing reference to invalid tTrigger %d from tWall %d\n",gameData.walls.walls [i].nTrigger,i);
#endif
		gameData.walls.walls [i].nTrigger = NO_TRIGGER;	//kill tTrigger
		}
if (gameTopFileInfo.fileinfo_version < 17) {
	for (nSegment = 0; nSegment <= gameData.segs.nLastSegment; nSegment++)
		for (nSide = 0; nSide < 6; nSide++)
			if (IS_WALL (nWall = WallNumI (nSegment, nSide))) {
				gameData.walls.walls [nWall].nSegment = nSegment;
				gameData.walls.walls [nWall].nSide = nSide;
				}
	}

#ifdef _DEBUG
for (nSide = 0; nSide < 6; nSide++) {
	nWall = WallNumI (gameData.segs.nLastSegment, nSide);
	if (IS_WALL (nWall) &&
			((gameData.walls.walls [nWall].nSegment != gameData.segs.nLastSegment) || 
			(gameData.walls.walls [nWall].nSide != nSide))) {
			Int3();	//	Error.  Bogus walls in this segment.
		}
	}
#endif
}

// -----------------------------------------------------------------------------
//go through all triggers, killing unused ones
static void CheckAndFixTriggers (void)
{
	int	i, j;
	short	nSegment, nSide, nWall;

for (i = 0; i < gameData.trigs.nTriggers; ) {
	//	Find which tWall this tTrigger is connected to.
	for (j = 0; j < gameData.walls.nWalls; j++)
		if (gameData.walls.walls [j].nTrigger == i)
			break;
#ifdef EDITOR
	if (j == gameData.walls.nWalls) {
#if TRACE
		con_printf (CONDBG,"Removing unreferenced tTrigger %d\n",i);
#endif
		RemoveTriggerNum (i);
		}
	else
#endif
		i++;
	}

for (i = 0; i < gameData.walls.nWalls; i++)
	gameData.walls.walls [i].controllingTrigger = -1;

//	MK, 10/17/95: Make walls point back at the triggers that control them.
//	Go through all triggers, stuffing controllingTrigger field in gameData.walls.walls.

for (i = 0; i < gameData.trigs.nTriggers; i++) {
	for (j = 0; j < gameData.trigs.triggers [i].nLinks; j++) {
		nSegment = gameData.trigs.triggers [i].nSegment [j];
		nSide = gameData.trigs.triggers [i].nSide [j];
		nWall = WallNumI (nSegment, nSide);
		//check to see that if a tTrigger requires a tWall that it has one,
		//and if it requires a botGen that it has one
		if (gameData.trigs.triggers [i].nType == TT_MATCEN) {
			if (gameData.segs.segment2s [nSegment].special != SEGMENT_IS_ROBOTMAKER)
				Int3();		//botGen tTrigger doesn'i point to botGen
			}
		else if ((gameData.trigs.triggers [i].nType != TT_LIGHT_OFF) && 
					(gameData.trigs.triggers [i].nType != TT_LIGHT_ON)) { //light triggers don't require walls
			if (IS_WALL (nWall))
				gameData.walls.walls [nWall].controllingTrigger = i;
			else {
				Int3();	//	This is illegal.  This ttrigger requires a tWall
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Load game 
// Loads all the relevant data for a level.
// If level != -1, it loads the filename with extension changed to .min
// Otherwise it loads the appropriate level mine.
// returns 0=everything ok, 1=old version, -1=error
int LoadMineDataCompiled (CFILE *fp, int bFileInfo)
{
	int 	nStartOffset;

nStartOffset = CFTell (fp);
InitGameFileInfo ();
if (ReadGameFileInfo (fp, nStartOffset))
	return -1;
if (ReadLevelInfo (fp))
	return -1;
gameStates.render.bD2XLights = gameStates.app.bD2XLevel && (gameTopFileInfo.fileinfo_version >= 34);
if (bFileInfo)
	return 0;

gameData.objs.nNextSignature = 0;
gameData.render.lights.nStatic = 0;
nGameSaveOrgRobots = 0;
nGameSavePlayers = 0;
if (ReadObjectInfo (fp))
	return -1;
if (ReadWallInfo (fp))
	return -1;
if (ReadDoorInfo (fp))
	return -1;
if (ReadTriggerInfo (fp))
	return -1;
if (ReadReactorInfo (fp))
	return -1;
if (ReadBotGenInfo (fp))
	return -1;
if (ReadEquipGenInfo (fp))
	return -1;
if (ReadLightDeltaIndexInfo (fp))
	return -1;
if (ReadLightDeltaInfo (fp))
	return -1;
ClearLightSubtracted ();
ResetObjects (gameFileInfo.objects.count);
CheckAndLinkObjects ();
ClearTransientObjects (1);		//1 means clear proximity bombs
CheckAndFixDoors ();
gameData.walls.nWalls = gameFileInfo.walls.count;
ResetWalls();
gameData.walls.nOpenDoors = gameFileInfo.doors.count;
gameData.trigs.nTriggers = gameFileInfo.triggers.count;
CheckAndFixWalls ();
CheckAndFixTriggers ();
gameData.matCens.nBotCenters = gameFileInfo.botGen.count;
//fix old tWall structs


//create_local_segment_data();

FixObjectSegs();

#ifdef _DEBUG
dump_mine_info();
#endif

if ((gameTopFileInfo.fileinfo_version < GAME_VERSION) && 
	 ((gameTopFileInfo.fileinfo_version != 25) || (GAME_VERSION != 26)))
	return 1;		//means old version
return 0;
}

// ----------------------------------------------------------------------------

int CheckSegmentConnections(void);

extern void	SetAmbientSoundFlags(void);


#define LEVEL_FILE_VERSION      8
//1 -> 2  add palette name
//2 -> 3  add control center explosion time
//3 -> 4  add reactor strength
//4 -> 5  killed hostage text stuff
//5 -> 6  added gameData.segs.secret.nReturnSegment and gameData.segs.secret.returnOrient
//6 -> 7  added flickering lights
//7 -> 8  made version 8 to be not compatible with D2 1.0 & 1.1

#ifdef _DEBUG
char *Level_being_loaded=NULL;
#endif

int no_oldLevel_file_error=0;

//loads a level (.LVL) file from disk
//returns 0 if success, else error code
int LoadLevelSub(char * filename_passed)
{
#ifdef EDITOR
	int use_compiledLevel=1;
#endif
	CFILE * fp;
	char filename [128];
	int sig, minedata_offset, gamedata_offset;
	int mine_err, game_err;
	//int i;
SetDataVersion (-1);
gameData.segs.bHaveSlideSegs = 0;
if (gameData.app.nGameMode & GM_NETWORK) {
	memset (gameData.multiplayer.maxPowerupsAllowed, 0, sizeof (gameData.multiplayer.maxPowerupsAllowed));
	memset (gameData.multiplayer.powerupsInMine, 0, sizeof (gameData.multiplayer.powerupsInMine));
	}
#ifdef _DEBUG
Level_being_loaded = filename_passed;
#endif
strcpy(filename,filename_passed);
#ifdef EDITOR
	//if we have the editor, try the LVL first, no matter what was passed.
	//if we don't have an LVL, try RDL  
	//if we don't have the editor, we just use what was passed

	ChangeFilenameExtension(filename,filename_passed,".lvl");
	use_compiledLevel = 0;

	if (!CFExist(filename))	{
		ChangeFilenameExtension(filename,filename,".rl2");
		use_compiledLevel = 1;
	}	
#endif

fp = CFOpen (filename, "", "rb", gameStates.app.bD1Mission);
if (!fp)	{
	#ifdef EDITOR
#if TRACE
		con_printf (CONDBG,"Can't open level file <%s>\n", filename);
#endif
		return 1;
	#else
//			Warning ("Can't open file <%s>\n",filename);
		return 1;
	#endif
}

strcpy(gameData.segs.szLevelFilename, filename);

//	#ifdef NEWDEMO
//	if (gameData.demo.nState == ND_STATE_RECORDING)
//		NDRecordStartDemo();
//	#endif

sig = CFReadInt (fp);
gameData.segs.nLevelVersion = CFReadInt (fp);
gameStates.app.bD2XLevel = (gameData.segs.nLevelVersion >= 10);
#if TRACE
con_printf (CONDBG, "gameData.segs.nLevelVersion = %d\n", gameData.segs.nLevelVersion);
#endif
minedata_offset = CFReadInt (fp);
gamedata_offset = CFReadInt (fp);

Assert(sig == MAKE_SIG('P','L','V','L'));
if (gameData.segs.nLevelVersion >= 8) {    //read dummy data
	CFReadInt (fp);
	CFReadShort (fp);
	CFReadByte(fp);
}

if (gameData.segs.nLevelVersion < 5)
	CFReadInt (fp);       //was hostagetext_offset

if (gameData.segs.nLevelVersion > 1) {
	CFGetS(szCurrentLevelPalette,sizeof(szCurrentLevelPalette),fp);
	if (szCurrentLevelPalette [strlen(szCurrentLevelPalette)-1] == '\n')
		szCurrentLevelPalette [strlen(szCurrentLevelPalette)-1] = 0;
}
if (gameData.segs.nLevelVersion <= 1 || szCurrentLevelPalette [0]==0) // descent 1 level
	strcpy(szCurrentLevelPalette, DEFAULT_LEVEL_PALETTE); //D1_PALETTE

if (gameData.segs.nLevelVersion >= 3)
	gameStates.app.nBaseCtrlCenExplTime = CFReadInt (fp);
else
	gameStates.app.nBaseCtrlCenExplTime = DEFAULT_CONTROL_CENTER_EXPLOSION_TIME;

if (gameData.segs.nLevelVersion >= 4)
	gameData.reactor.nStrength = CFReadInt (fp);
else
	gameData.reactor.nStrength = -1;  //use old defaults

if (gameData.segs.nLevelVersion >= 7) {
	int i;

#if TRACE
con_printf(CONDBG, "   loading dynamic lights ...\n");
#endif
gameData.render.lights.flicker.nLights = CFReadInt (fp);
Assert((gameData.render.lights.flicker.nLights >= 0) && (gameData.render.lights.flicker.nLights < MAX_FLICKERING_LIGHTS));
for (i = 0; i < gameData.render.lights.flicker.nLights; i++)
	ReadVariableLight(&gameData.render.lights.flicker.lights [i], fp);
}
else
	gameData.render.lights.flicker.nLights = 0;

if (gameData.segs.nLevelVersion < 6) {
	gameData.segs.secret.nReturnSegment = 0;
	gameData.segs.secret.returnOrient.rVec.p.y =
	gameData.segs.secret.returnOrient.rVec.p.z = 
	gameData.segs.secret.returnOrient.fVec.p.x =
	gameData.segs.secret.returnOrient.fVec.p.z =
	gameData.segs.secret.returnOrient.uVec.p.x =
	gameData.segs.secret.returnOrient.uVec.p.y = 0;
	gameData.segs.secret.returnOrient.rVec.p.x =
	gameData.segs.secret.returnOrient.fVec.p.y =
	gameData.segs.secret.returnOrient.uVec.p.z = F1_0;
} else {
	gameData.segs.secret.nReturnSegment = CFReadInt (fp);
	gameData.segs.secret.returnOrient.rVec.p.x = CFReadInt (fp);
	gameData.segs.secret.returnOrient.rVec.p.y = CFReadInt (fp);
	gameData.segs.secret.returnOrient.rVec.p.z = CFReadInt (fp);
	gameData.segs.secret.returnOrient.fVec.p.x = CFReadInt (fp);
	gameData.segs.secret.returnOrient.fVec.p.y = CFReadInt (fp);
	gameData.segs.secret.returnOrient.fVec.p.z = CFReadInt (fp);
	gameData.segs.secret.returnOrient.uVec.p.x = CFReadInt (fp);
	gameData.segs.secret.returnOrient.uVec.p.y = CFReadInt (fp);
	gameData.segs.secret.returnOrient.uVec.p.z = CFReadInt (fp);
}

#ifdef EDITOR
if (!use_compiledLevel) {
	CFSeek (fp,minedata_offset, SEEK_SET);
	mine_err = load_mine_data(fp);
#if 0 // get from d1src if needed
	// Compress all uv coordinates in mine, improves texmap precision. --MK, 02/19/96
	compress_uv_coordinates_all();
#endif
	}
else
#endif
	{
	//NOTE LINK TO ABOVE!!
	CFSeek (fp, gamedata_offset, SEEK_SET);
	game_err = LoadMineDataCompiled (fp, 1);
	CFSeek (fp, minedata_offset, SEEK_SET);
	mine_err = LoadMineSegmentsCompiled( fp);
	}
if (mine_err == -1) {   //error!!
	CFClose(fp);
	return 2;
}
CFSeek (fp, gamedata_offset, SEEK_SET);
game_err = LoadMineDataCompiled (fp, 0);
if (game_err == -1) {   //error!!
	CFClose(fp);
	return 3;
	}
CFClose(fp);
SetAmbientSoundFlags ();
#ifdef EDITOR
write_game_text_file(filename);
if (Errors_in_mine) {
	if (IsRealLevel(filename)) {
		char  ErrorMessage [200];

		sprintf(ErrorMessage, TXT_MINE_ERRORS, Errors_in_mine, Level_being_loaded);
		StopTime();
		GrPaletteStepLoad (NULL);
		ExecMessageBox(NULL, 1, TXT_CONTINUE, ErrorMessage);
		StartTime();
	} else {
#if TRACE
		con_printf (1, TXT_MINE_ERRORS, Errors_in_mine, Level_being_loaded);
#endif
	}
}
#endif

#ifdef EDITOR
//If an old version, ask the use if he wants to save as new version
if (!no_oldLevel_file_error && (gameStates.app.nFunctionMode == FMODE_EDITOR) && 
    (((LEVEL_FILE_VERSION > 3) && gameData.segs.nLevelVersion < LEVEL_FILE_VERSION) || mine_err == 1 || game_err == 1)) {
	char  ErrorMessage [200];

	sprintf(ErrorMessage,
				"You just loaded a old version\n"
				"level.  Would you like to save\n"
				"it as a current version level?");

	StopTime();
	GrPaletteStepLoad (NULL);
	if (ExecMessageBox(NULL, 2, "Don't Save", "Save", ErrorMessage)==1)
		SaveLevel(filename);
	StartTime();
}
#endif

#ifdef EDITOR
if (gameStates.app.nFunctionMode == FMODE_EDITOR)
	editor_status("Loaded NEW mine %s, \"%s\"",filename,gameData.missions.szCurrentLevel);
#endif

#ifdef EDITOR
if (CheckSegmentConnections())
	ExecMessageBox("ERROR", 1, "Ok", 
			"Connectivity errors detected in\n"
			"mine.  See monochrome screen for\n"
			"details, and contact Matt or Mike.");
#endif

return 0;
}

#ifdef EDITOR
void GetLevelName()
{
//NO_UI!!!	UI_WINDOW 				*NameWindow = NULL;
//NO_UI!!!	UI_GADGET_INPUTBOX	*NameText;
//NO_UI!!!	UI_GADGET_BUTTON 		*QuitButton;
//NO_UI!!!
//NO_UI!!!	// Open a window with a quit button
//NO_UI!!!	NameWindow = ui_open_window(20, 20, 300, 110, WIN_DIALOG);
//NO_UI!!!	QuitButton = ui_add_gadget_button(NameWindow, 150-24, 60, 48, 40, "Done", NULL);
//NO_UI!!!
//NO_UI!!!	ui_wprintf_at(NameWindow, 10, 12,"Please enter a name for this mine:");
//NO_UI!!!	NameText = ui_add_gadget_inputbox(NameWindow, 10, 30, LEVEL_NAME_LEN, LEVEL_NAME_LEN, gameData.missions.szCurrentLevel);
//NO_UI!!!
//NO_UI!!!	NameWindow->keyboard_focus_gadget = (UI_GADGET *)NameText;
//NO_UI!!!	QuitButton->hotkey = KEY_ENTER;
//NO_UI!!!
//NO_UI!!!	ui_gadget_calc_keys(NameWindow);
//NO_UI!!!
//NO_UI!!!	while (!QuitButton->pressed && last_keypress!=KEY_ENTER) {
//NO_UI!!!		ui_mega_process();
//NO_UI!!!		ui_window_do_gadgets(NameWindow);
//NO_UI!!!	}
//NO_UI!!!
//NO_UI!!!	strcpy(gameData.missions.szCurrentLevel, NameText->text);
//NO_UI!!!
//NO_UI!!!	if (NameWindow!=NULL)	{
//NO_UI!!!		ui_close_window(NameWindow);
//NO_UI!!!		NameWindow = NULL;
//NO_UI!!!	}
//NO_UI!!!

	tMenuItem m [2];

	memset (m, 0, sizeof (m));
	m [0].nType = NM_TYPE_TEXT; 
	m [0].text = "Please enter a name for this mine:";
	m [1].nType = NM_TYPE_INPUT; 
	m [1].text = gameData.missions.szCurrentLevel; 
	m [1].text_len = LEVEL_NAME_LEN;

	ExecMenu(NULL, "Enter mine name", 2, m, NULL);

}
#endif


#ifdef EDITOR

int	Errors_in_mine;

// -----------------------------------------------------------------------------

int CountDeltaLightRecords(void)
{
	int	i;
	int	total = 0;

	for (i=0; i<gameData.render.lights.nStatic; i++) {
		total += gameData.render.lights.deltaIndices [i].count;
	}

	return total;

}

// -----------------------------------------------------------------------------
// Save game
int SaveGameData(FILE * SaveFile)
{
	int  player.offset, tObject.offset, walls.offset, doors.offset, triggers.offset, control.offset, botGen.offset; //, links.offset;
	int	gameData.render.lights.deltaIndices.offset, deltaLight.offset;
	int start_offset,end_offset;

	start_offset = ftell(SaveFile);

	//===================== SAVE FILE INFO ========================

	gameFileInfo.fileinfo_signature =	0x6705;
	gameFileInfo.fileinfo_version	=	GAME_VERSION;
	gameFileInfo.level					=  gameData.missions.nCurrentLevel;
	gameFileInfo.fileinfo_sizeof		=	sizeof(gameFileInfo);
	gameFileInfo.player.offset		=	-1;
	gameFileInfo.player.size		=	sizeof(tPlayer);
	gameFileInfo.objects.offset		=	-1;
	gameFileInfo.objects.count		=	gameData.objs.nLastObject+1;
	gameFileInfo.objects.size		=	sizeof(tObject);
	gameFileInfo.walls.offset			=	-1;
	gameFileInfo.walls.count		=	gameData.walls.nWalls;
	gameFileInfo.walls.size			=	sizeof(tWall);
	gameFileInfo.doors.offset			=	-1;
	gameFileInfo.doors.count		=	gameData.walls.nOpenDoors;
	gameFileInfo.doors.size			=	sizeof(tActiveDoor);
	gameFileInfo.triggers.offset		=	-1;
	gameFileInfo.triggers.count	=	gameData.trigs.nTriggers;
	gameFileInfo.triggers.size		=	sizeof(tTrigger);
	gameFileInfo.control.offset		=	-1;
	gameFileInfo.control.count		=  1;
	gameFileInfo.control.size		=  sizeof(tReactorTriggers);
 	gameFileInfo.botGen.offset		=	-1;
	gameFileInfo.botGen.count		=	gameData.matCens.nBotCenters;
	gameFileInfo.botGen.size		=	sizeof(tMatCenInfo);

 	gameFileInfo.lightDeltaIndices.offset		=	-1;
	gameFileInfo.lightDeltaIndices.count		=	gameData.render.lights.nStatic;
	gameFileInfo.lightDeltaIndices.size		=	sizeof(tLightDeltaIndex);

 	gameFileInfo.lightDeltas.offset		=	-1;
	gameFileInfo.lightDeltas.count	=	CountDeltaLightRecords();
	gameFileInfo.lightDeltas.size		=	sizeof(tLightDelta);

	// Write the fileinfo
	fwrite(&gameFileInfo, sizeof(gameFileInfo), 1, SaveFile);

	// Write the mine name
	fprintf(SaveFile,"%s\n",gameData.missions.szCurrentLevel);

	fwrite(&gameData.models.nPolyModels,2,1,SaveFile);
	fwrite(Pof_names,gameData.models.nPolyModels,sizeof(*Pof_names),SaveFile);

	//==================== SAVE PLAYER INFO ===========================

	player.offset = ftell(SaveFile);
	fwrite(&LOCALPLAYER, sizeof(tPlayer), 1, SaveFile);

	//==================== SAVE OBJECT INFO ===========================

	tObject.offset = ftell(SaveFile);
	//fwrite(&gameData.objs.objects, sizeof(tObject), gameFileInfo.objects.count, SaveFile);
	{
		int i;
		for (i=0;i<gameFileInfo.objects.count;i++)
			writeObject(&gameData.objs.objects [i],SaveFile);
	}

	//==================== SAVE WALL INFO =============================

	walls.offset = ftell(SaveFile);
	fwrite(gameData.walls.walls, sizeof(tWall), gameFileInfo.walls.count, SaveFile);

	//==================== SAVE DOOR INFO =============================

	doors.offset = ftell(SaveFile);
	fwrite(gameData.walls.activeDoors, sizeof(tActiveDoor), gameFileInfo.doors.count, SaveFile);

	//==================== SAVE TRIGGER INFO =============================

	triggers.offset = ftell(SaveFile);
	fwrite(gameData.trigs.triggers, sizeof(tTrigger), gameFileInfo.triggers.count, SaveFile);

	//================ SAVE CONTROL CENTER TRIGGER INFO ===============

	control.offset = ftell(SaveFile);
	fwrite(&gameData.reactor.triggers, sizeof(tReactorTriggers), 1, SaveFile);


	//================ SAVE MATERIALIZATION CENTER TRIGGER INFO ===============

	botGen.offset = ftell(SaveFile);
	fwrite(gameData.matCens.botGens, sizeof(tMatCenInfo), gameFileInfo.botGen.count, SaveFile);

	//================ SAVE DELTA LIGHT INFO ===============
	gameData.render.lights.deltaIndices.offset = ftell(SaveFile);
	fwrite(gameData.render.lights.deltaIndices, sizeof(tLightDeltaIndex), gameFileInfo.lightDeltaIndices.count, SaveFile);

	deltaLight.offset = ftell(SaveFile);
	fwrite(gameData.render.lights.deltas, sizeof(tLightDelta), gameFileInfo.lightDeltas.count, SaveFile);

	//============= REWRITE FILE INFO, TO SAVE OFFSETS ===============

	// Update the offset fields
	gameFileInfo.player.offset		=	player.offset;
	gameFileInfo.objects.offset		=	tObject.offset;
	gameFileInfo.walls.offset			=	walls.offset;
	gameFileInfo.doors.offset			=	doors.offset;
	gameFileInfo.triggers.offset		=	triggers.offset;
	gameFileInfo.control.offset		=	control.offset;
	gameFileInfo.botGen.offset		=	botGen.offset;
	gameFileInfo.lightDeltaIndices.offset	=	gameData.render.lights.deltaIndices.offset;
	gameFileInfo.lightDeltas.offset	=	deltaLight.offset;


	end_offset = ftell(SaveFile);

	// Write the fileinfo
	fseek( SaveFile, start_offset, SEEK_SET);  // Move to TOF
	fwrite(&gameFileInfo, sizeof(gameFileInfo), 1, SaveFile);

	// Go back to end of data
	fseek(SaveFile,end_offset, SEEK_SET);

	return 0;
}

int save_mine_data(FILE * SaveFile);

// -----------------------------------------------------------------------------
// Save game
int saveLevel_sub(char * filename, int compiled_version)
{
	FILE * SaveFile;
	char temp_filename [128];
	int sig = MAKE_SIG('P','L','V','L'),version=LEVEL_FILE_VERSION;
	int minedata_offset=0,gamedata_offset=0;

	if (!compiled_version)	{
		write_game_text_file(filename);

		if (Errors_in_mine) {
			if (IsRealLevel(filename)) {
				char  ErrorMessage [200];

				sprintf(ErrorMessage, TXT_MINE_ERRORS2, Errors_in_mine);
				StopTime();
				GrPaletteStepLoad (NULL);
	 
				if (ExecMessageBox(NULL, 2, TXT_CANCEL_SAVE, TXT_DO_SAVE, ErrorMessage)!=1)	{
					StartTime();
					return 1;
				}
				StartTime();
			}
		}
		ChangeFilenameExtension(temp_filename,filename,".LVL");
	}
	else
	{
		// macs are using the regular hog/rl2 files for shareware
	}

	SaveFile = fopen(temp_filename, "wb");
	if (!SaveFile)
	{
		char ErrorMessage [256];

		char fname [20];
		_splitpath(temp_filename, NULL, NULL, fname, NULL);

		sprintf(ErrorMessage, \
			"ERROR: Cannot write to '%s'.\nYou probably need to check out a locked\nversion of the file. You should save\nthis under a different filename, and then\ncheck out a locked copy by typing\n\'co -l %s.lvl'\nat the DOS prompt.\n" 
			, temp_filename, fname);
		StopTime();
		GrPaletteStepLoad (NULL);
		ExecMessageBox(NULL, 1, "Ok", ErrorMessage);
		StartTime();
		return 1;
	}

	if (gameData.missions.szCurrentLevel [0] == 0)
		strcpy(gameData.missions.szCurrentLevel,"Untitled");

	ClearTransientObjects(1);		//1 means clear proximity bombs

	compressObjects();		//after this, gameData.objs.nLastObject == num gameData.objs.objects

	//make sure tPlayer is in a tSegment
	if (UpdateObjectSeg(&gameData.objs.objects [gameData.multiplayer.players [0].nObject]) == 0) {
		if (gameData.objs.console->nSegment > gameData.segs.nLastSegment)
			gameData.objs.console->nSegment = 0;
		COMPUTE_SEGMENT_CENTER(&gameData.objs.console->position.vPos,&(gameData.segs.segments [gameData.objs.console->nSegment]);
	}
 
	FixObjectSegs();

	//Write the header

	gs_write_int(sig,SaveFile);
	gs_write_int(version,SaveFile);

	//save placeholders
	gs_write_int(minedata_offset,SaveFile);
	gs_write_int(gamedata_offset,SaveFile);

	//Now write the damn data

	//write the version 8 data (to make file unreadable by 1.0 & 1.1)
	gs_write_int(gameData.time.xGame,SaveFile);
	gs_write_short(gameData.app.nFrameCount,SaveFile);
	gs_write_byte(gameData.time.xFrame,SaveFile);

	// Write the palette file name
	fprintf(SaveFile,"%s\n",szCurrentLevelPalette);

	gs_write_int(gameStates.app.nBaseCtrlCenExplTime,SaveFile);
	gs_write_int(gameData.reactor.nStrength,SaveFile);

	gs_write_int(gameData.render.lights.flicker.nLights,SaveFile);
	fwrite(gameData.render.lights.flicker.lights,sizeof(*gameData.render.lights.flicker.lights),gameData.render.lights.flicker.nLights,SaveFile);

	gs_write_int(gameData.segs.secret.nReturnSegment, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.rVec.p.x, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.rVec.p.y, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.rVec.p.z, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.fVec.p.x, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.fVec.p.y, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.fVec.p.z, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.uVec.p.x, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.uVec.p.y, SaveFile);
	gs_write_int(gameData.segs.secret.returnOrient.uVec.p.z, SaveFile);

	minedata_offset = ftell(SaveFile);
	if (!compiled_version)
		save_mine_data(SaveFile);
	else
		save_mine_data_compiled(SaveFile);
	gamedata_offset = ftell(SaveFile);
	SaveGameData(SaveFile);

	fseek(SaveFile,sizeof(sig)+sizeof(version), SEEK_SET);
	gs_write_int(minedata_offset,SaveFile);
	gs_write_int(gamedata_offset,SaveFile);

	//==================== CLOSE THE FILE =============================
	fclose(SaveFile);

	if (!compiled_version)	{
		if (gameStates.app.nFunctionMode == FMODE_EDITOR)
			editor_status("Saved mine %s, \"%s\"",filename,gameData.missions.szCurrentLevel);
	}

	return 0;

}

// -----------------------------------------------------------------------------

#if 0 //dunno - 3rd party stuff?
extern void compress_uv_coordinates_all(void);
#endif

int SaveLevel(char * filename)
{
	int r1;

	// Save normal version...
	r1 = saveLevel_sub(filename, 0);

	// Save compiled version...
	saveLevel_sub(filename, 1);

	return r1;
}

#endif	//EDITOR

#ifdef _DEBUG
void dump_mine_info(void)
{
	int	nSegment, nSide;
	fix	min_u, max_u, min_v, max_v, min_l, max_l, max_sl;

	min_u = F1_0*1000;
	min_v = min_u;
	min_l = min_u;

	max_u = -min_u;
	max_v = max_u;
	max_l = max_u;

	max_sl = 0;

	for (nSegment=0; nSegment<=gameData.segs.nLastSegment; nSegment++) {
		for (nSide=0; nSide<MAX_SIDES_PER_SEGMENT; nSide++) {
			int	vertnum;
			tSide	*sideP = &gameData.segs.segments [nSegment].sides [nSide];

			if (gameData.segs.segment2s [nSegment].xAvgSegLight > max_sl)
				max_sl = gameData.segs.segment2s [nSegment].xAvgSegLight;

			for (vertnum=0; vertnum<4; vertnum++) {
				if (sideP->uvls [vertnum].u < min_u)
					min_u = sideP->uvls [vertnum].u;
				else if (sideP->uvls [vertnum].u > max_u)
					max_u = sideP->uvls [vertnum].u;

				if (sideP->uvls [vertnum].v < min_v)
					min_v = sideP->uvls [vertnum].v;
				else if (sideP->uvls [vertnum].v > max_v)
					max_v = sideP->uvls [vertnum].v;

				if (sideP->uvls [vertnum].l < min_l)
					min_l = sideP->uvls [vertnum].l;
				else if (sideP->uvls [vertnum].l > max_l)
					max_l = sideP->uvls [vertnum].l;
			}

		}
	}
}

#endif

#ifdef EDITOR

// -----------------------------------------------------------------------------
//read in every level in mission and save out compiled version 
void save_all_compiledLevels(void)
{
DoLoadSaveLevels(1);
}

// -----------------------------------------------------------------------------
//read in every level in mission
void LoadAllLevels(void)
{
DoLoadSaveLevels(0);
}

// -----------------------------------------------------------------------------

void DoLoadSaveLevels(int save)
{
	int level_num;

	if (! SafetyCheck())
		return;

	no_oldLevel_file_error=1;

	for (level_num=1;level_num<=gameData.missions.nLastLevel;level_num++) {
		LoadLevelSub(gameData.missions.szLevelNames [level_num-1]);
		LoadPalette(szCurrentLevelPalette,1,1,0);		//don't change screen
		if (save)
			saveLevel_sub(gameData.missions.szLevelNames [level_num-1],1);
	}

	for (level_num = -1; level_num >= gameData.missions.nLastSecretLevel; level_num--) {
		LoadLevelSub(gameData.missions.szSecretLevelNames [-level_num-1]);
		LoadPalette(szCurrentLevelPalette,1,1,0);		//don't change screen
		if (save)
			saveLevel_sub (gameData.missions.szSecretLevelNames [-level_num-1],1);
	}

	no_oldLevel_file_error=0;

}

#endif
