// Copyright (c) 1998 Bryan Aamot, Brainware

#include "stdafx.h"
#include "dle-xp-res.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "io.h"
#include "textures.h"
#include "palette.h"
#include "dle-xp.h"
#include "robot.h"
#include "hogmanager.h"
#include "light.h"

#ifdef ALLOCATE_POLYMODELS
#undef ALLOCATE_POLYMODELS
#endif
#define ALLOCATE_POLYMODELS 0

#define ENABLE_TEXT_DUMP 0

// ------------------------------------------------------------------------
// ReadObject()
// ------------------------------------------------------------------------
// CMine - save()
//
// ACTION -  saves a level (.RDL) file to disk
// ------------------------------------------------------------------------

INT16 CMine::Save (const char * filename_passed, bool bSaveToHog)
{
	FILE*	fp;
	char	filename [256];
	INT32 minedataOffset, gamedataOffset, hostagetextOffset;
	INT32 mineErr, gameErr;
	INT32	i;

strcpy_s (filename, sizeof (filename), filename_passed);
if (fopen_s (&fp, filename, "wb")) 
	return(1);

m_changesMade = 0;

// write file signature
write_INT32 ('P'*0x1000000L + 'L'*0x10000L + 'V'*0x100 + 'L', fp); // signature

// always save as version 7 or greater if its a D2 level
// otherwise, blinking lights will not work.
if (LevelVersion () < 7 && IsD2File ()) {
	SetLevelVersion (7);
}
if ((IsD2XLevel ()) && (LevelOutdated ())) {
	UpdateLevelVersion ();
	//if (LevelVersion () < 15)
		ConvertWallNum (MAX_WALLS2 + 1, MAX_WALLS3 + 1);
	}

// write version
write_INT32 (LevelVersion (), fp);
write_INT32 (0, fp); // minedataOffset (temporary)
write_INT32 (0, fp); // gamedataOffset (temporary)

if (IsD2File ()&& (LevelVersion () >= 8)) {
	write_INT16(rand(), fp);
	write_INT16(rand(), fp);
	write_INT16(rand(), fp);
	write_INT8((INT8)rand(), fp);
	}

if (m_fileType== RDL_FILE)
	write_INT32 (0, fp); // hostagetextOffset (temporary)
else {
	// save palette name
	char *name = strrchr(descent2_path, '\\');
	if (!name) 
		name = descent2_path; // point to 1st char if no slash found
	else
		name++;               // point to character after slash
	char palette_name [15];
	strncpy_s (palette_name, sizeof (palette_name), name, 12);
	palette_name [13] = NULL;  // null terminate just in case
	// replace extension with *.256
	if (strlen ((char *)palette_name) > 4)
		strcpy_s (&palette_name [strlen ((char *) palette_name) - 4], 5, ".256");
	else
		strcpy_s (palette_name, sizeof (palette_name), "GROUPA.256");
	_strupr_s (palette_name, sizeof (palette_name));
	strcat_s (palette_name, sizeof (palette_name), "\n"); // add a return to the end
	fwrite (palette_name, strlen ((char *)palette_name), 1, fp);
	}

// write reactor info
if (IsD2File ()) {
	// read descent 2 reactor information
	write_INT32 (ReactorTime (), fp);
	write_INT32 (ReactorStrength (), fp);
	// flickering light new for version 7
	if (FlickerLightCount () > MAX_FLICKERING_LIGHTS) 
		FlickerLightCount () = MAX_FLICKERING_LIGHTS;
	write_INT32 (FlickerLightCount (), fp);
	for (i = 0; i < FlickerLightCount (); i++)
		FlickeringLights (i)->Write (fp);

	// write secret cube number
	write_INT32 (SecretCubeNum (), fp);
	// write secret cube orientation?
	write_matrix(&SecretOrient (), fp);
	}
// save mine data
minedataOffset = ftell(fp);
if (0 > (mineErr = SaveMineDataCompiled (fp))) {
	fclose(fp);
	ErrorMsg ("Error saving mine data");
	return(2);
	}

// save game data
gamedataOffset = ftell(fp);
if (0 > (gameErr = SaveGameData(fp))) {
	fclose(fp);
	ErrorMsg ("Error saving game data");
	return(3);
	}

// save hostage data
hostagetextOffset = ftell(fp);
// leave hostage text empty

// now and go back to beginning of file and save offsets
fseek(fp, 2*sizeof (INT32), SEEK_SET);
write_INT32 (minedataOffset, fp);    // gamedataOffset
write_INT32 (gamedataOffset, fp);    // gamedataOffset
if (m_fileType== RDL_FILE) 
	write_INT32 (hostagetextOffset, fp); // hostagetextOffset
fclose(fp);

if (HasCustomTextures () && !bSaveToHog) {
	char* ps = strstr (filename, ".");
	if (ps)
		strcpy_s (ps, sizeof (filename) - (ps - filename), ".pog");
	else
		strcat_s (filename, sizeof (filename), ".pog");
	fopen_s (&fp, filename, "wb");
	if (fp) {
		CreatePog (fp);
		fclose (fp);
		}
	}

if (HasCustomRobots () && !bSaveToHog) {
	char* ps = strstr (filename, ".");
	if (ps)
		strcpy_s (ps, sizeof (filename) - (ps - filename), ".hxm");
	else
		strcat_s (filename, sizeof (filename), ".hxm");
	fopen_s (&fp, filename, "wb");
	if (fp)
		WriteHxmFile (fp);
	}

return 0;
}

// ------------------------------------------------------------------------

void CMine::SortDLIndex (INT32 left, INT32 right)
{
	INT32	l = left,
			r = right,
			m = (left + right) / 2;
	INT16	mSeg = LightDeltaIndex (m)->m_nSegment, 
			mSide = LightDeltaIndex (m)->m_nSide;
	CSideKey mKey = CSideKey (mSeg, mSide);
	CLightDeltaIndex	*pl, *pr;

do {
	pl = LightDeltaIndex (l);
	//while ((pl->m_nSegment < mSeg) || ((pl->m_nSegment == mSeg) && (pl->nSide < mSide))) {
	while (*pl < mKey) {
		pl++;
		l++;
		}
	pr = LightDeltaIndex (r);
	//while ((pr->nSegment > mSeg) || ((pr->nSegment == mSeg) && (pr->nSide > mSide))) {
	while (*pr > mKey) {
		pr--;
		r--;
		}
	if (l <= r) {
		if (l < r) {
			CLightDeltaIndex	h = *pl;
			*pl = *pr;
			*pr = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (right > l)
   SortDLIndex (l, right);
if (r > left)
   SortDLIndex (left, r);
}

// ------------------------------------------------------------------------
// SaveMineDataCompiled()
//
// ACTION - Writes a mine data portion of RDL file.
// ------------------------------------------------------------------------
INT16 CMine::SaveMineDataCompiled(FILE *fp)
{
	int	i;
// write version (1 byte)
write_INT8 (COMPILED_MINE_VERSION, fp);

// write no. of vertices (2 bytes)
write_INT16 (VertCount (), fp);

// write number of Segments () (2 bytes)
write_INT16 (SegCount (), fp);

// write all vertices
for (int i = 0; i < VertCount (); i++)
	Vertices (i)->Write (fp);

// write segment information
for (i = 0; i < SegCount (); i++)  
	Segments (i)->Write (fp, IsD2XLevel () ? 2 : IsD2File () ? 1 : 0, LevelVersion());

// for Descent 2, save special info here
if (IsD2File ()) {
  for (i = 0; i < SegCount (); i++)  
	  Segments (i)->WriteExtras (fp, IsD2XLevel () ? 2 : 1, true);
  }

if (IsD2XLevel ()) {
	SaveColors (VertexColors (0), VertCount (), fp);
	SaveColors (LightColors (0), SegCount () * 6, fp);
	SaveColors (TexColors (0), MAX_D2_TEXTURES, fp);
	}
return 0;
}

// ------------------------------------------------------------------------
// SaveGameData()
//
//  ACTION - Saves the player, object, wall, door, trigger, and
//           materialogrifizationator data from an RDL file.
// ------------------------------------------------------------------------
INT16 CMine::SaveGameData(FILE *savefile)
{
#if 1 //!DEMO
	HINSTANCE hInst = AfxGetInstanceHandle();

	INT32 i;
	INT32 startOffset, endOffset;

	startOffset = ftell(savefile);

	//==================== = WRITE FILE INFO========================

	// Do not assume the "sizeof" values are the same as what was read when level was loaded.
	// Also be careful no to use sizeof () because the editor's internal size may not match
	// the size which is used by the game engine.
	GameInfo ().objects.size = 0x108;                         // 248 = sizeof (object)
	GameInfo ().walls.size = 24;                            // 24 = sizeof (wall)
	GameInfo ().doors.size = 16;                            // 16 = sizeof (CActiveDoor)
	GameInfo ().triggers.size = (m_fileType== RDL_FILE) ? 54:52; // 54 = sizeof (trigger)
	GameInfo ().control.size = 42;                            // 42 = sizeof (CReactorTrigger)
	GameInfo ().botgen.size = (m_fileType== RDL_FILE) ? 16:20; // 20 = sizeof (CRobotMaker)
	GameInfo ().equipgen.size = 20; // 20 = sizeof (CRobotMaker)
	GameInfo ().lightDeltaIndices.size = 6;                             // 6 = sizeof (CLightDeltaIndex)
	GameInfo ().lightDeltaValues.size = 8;                             // 8 = sizeof (CLightDeltaValue)

	// the offsets will be calculated as we go then rewritten at the end
	//  GameInfo ().doors.offset =-1;
	//  GameInfo ().player.offset =-1;
	//  GameInfo ().objects.offset =-1;
	//  GameInfo ().walls.offset =-1;
	//  GameInfo ().triggers.offset =-1;
	//  GameInfo ().control.offset =-1;
	//  GameInfo ().matcen.offset =-1;
	//  GameInfo ().lightDeltaIndices.offset =-1;
	//  GameInfo ().lightDeltaValues.offset =-1;

	// these numbers (.howmany) are updated by the editor
	//  GameInfo ().objects.count = 0;
	//  GameInfo ().walls.count = 0;
	//  GameInfo ().doors.count = 0;
	//  GameInfo ().triggers.count = 0;
	//  GameInfo ().control.count = 0;
	//  GameInfo ().matcen.count = 0;
	//  GameInfo ().lightDeltaIndices.count = 0; // D2
	//  GameInfo ().lightDeltaValues.count = 0; // D2

	if (m_fileType== RDL_FILE) {
		GameInfo ().fileinfo.signature = 0x6705;
		GameInfo ().fileinfo.version = 25;
		GameInfo ().fileinfo.size = 119;
		GameInfo ().level = 0;
		}
	else {
		GameInfo ().fileinfo.signature = 0x6705;
		GameInfo ().fileinfo.version = (LevelVersion () < 13) ? 31 : 40;
		GameInfo ().fileinfo.size = (LevelVersion () < 13) ? 143 : sizeof (GameInfo ()); // same as sizeof (GameInfo ())
		GameInfo ().level = 0;
	}

	fwrite(&GameInfo (), (INT16)GameInfo ().fileinfo.size, 1, savefile);
	if (GameInfo ().fileinfo.version >= 14) {  /*save mine filename */
		fwrite(m_currentLevelName, sizeof (char), strlen (m_currentLevelName), savefile);
	}
	if (IsD2File ()) {
		fwrite("\n", 1, 1, savefile); // write an end - of - line
	} else {
		fwrite("", 1, 1, savefile);   // write a null
	}

	// write pof names from resource file
	HRSRC     hRes;
	HGLOBAL   hGlobal;
	UINT8 *save_pof_names;
	INT16 n_save_pof_names, n_pofs;

	if (IsD2File ()) {
		n_save_pof_names = 166;
		if (!(hRes = FindResource(hInst, MAKEINTRESOURCE(IDR_POF_NAMES2), "RC_DATA")))
			return 1;
		fwrite(&n_save_pof_names, 2, 1, savefile);   // write # of POF names
		}
	else {
		n_save_pof_names = 78;
		if (!(hRes = FindResource(hInst, MAKEINTRESOURCE(IDR_POF_NAMES1), "RC_DATA")))
			return 1;
		n_pofs = 25;   // Don't know exactly what this value is for or why it is 25?
		fwrite(&n_pofs, 2, 1, savefile);
		}
	hGlobal = LoadResource(hInst, hRes);
	ASSERT(hGlobal);

	save_pof_names = (UINT8 *) LockResource(hGlobal);
	ASSERT(save_pof_names);

	fwrite(save_pof_names, n_save_pof_names, 13, savefile); // 13 characters each
	FreeResource(hGlobal);

	//==================== = WRITE PLAYER INFO==========================
	GameInfo ().player.offset = ftell(savefile);
	char* str = "Made with Descent Level Editor XP 32\0\0\0\0\0\0\0";
	fwrite(str, strlen (str) + 1, 1, savefile);

	//==================== = WRITE OBJECT INFO==========================
	// note: same for D1 and D2
	GameInfo ().objects.offset = ftell(savefile);
	for (i = 0; i < GameInfo ().objects.count; i++)
		Objects (i)->Write (savefile, GameInfo ().fileinfo.version);

	//==================== = WRITE WALL INFO============================
	// note: Wall size will automatically strip last two items
	//       when saving D1 level
	GameInfo ().walls.offset = ftell(savefile);
	if (GameInfo ().fileinfo.version >= 20) {
		for (i = 0; i < GameInfo ().walls.count; i++)
			Walls (i)->Write (savefile, GameInfo ().fileinfo.version);
	}

	//==================== = WRITE DOOR INFO============================
	// note: not used for D1 or D2 since doors.count is always 0
	GameInfo ().doors.offset = ftell(savefile);
	if (GameInfo ().fileinfo.version >= 20)
//	for (i = 0; i < GameInfo ().doors.count; i++)
		for (i = 0; i < GameInfo ().doors.count; i++)
			ActiveDoors (i)->Write (savefile, GameInfo ().fileinfo.version);
			//fwrite(ActiveDoors (i), TotalSize (GameInfo ().doors), 1, savefile);

	//==================== WRITE TRIGGER INFO==========================
	// note: order different for D2 levels but size is the same
	GameInfo ().triggers.offset = ftell(savefile);
	for (i = 0; i < GameInfo ().triggers.count; i++)
		Triggers (i)->Write (savefile, GameInfo ().fileinfo.version, false);
	if (LevelVersion () >= 12) {
		write_INT32 (NumObjTriggers (), savefile);
		if (NumObjTriggers ()) {
			SortObjTriggers ();
			for (i = 0; i < NumObjTriggers (); i++)
				ObjTriggers (i)->Write (savefile, GameInfo ().fileinfo.version, true);
			for (i = 0; i < NumObjTriggers (); i++)
				write_INT16 (ObjTriggers (i)->nObject, savefile);
			}
		}

	//================ WRITE CONTROL CENTER TRIGGER INFO============== =
	// note: same for D1 and D2
	GameInfo ().control.offset = ftell(savefile);
	for (i = 0; i < GameInfo ().control.count; i++)
		ReactorTriggers (i)->Write (savefile, GameInfo ().fileinfo.version);
		//fwrite(ReactorTriggers (), TotalSize (GameInfo ().control), 1, savefile);

	//================ WRITE MATERIALIZATION CENTERS INFO============== =
	// note: added robot_flags2 for Descent 2
	GameInfo ().botgen.offset = ftell(savefile);
	//if (IsD2File ())
	//	fwrite(BotGens (), TotalSize (GameInfo ().botgen), 1, savefile);
	//else 
		for (i = 0; i < GameInfo ().botgen.count; i++) {
			BotGens (i)->Write (savefile, GameInfo ().fileinfo.version);
			//write_INT32 (BotGens (i)->objFlags[0], savefile);
			//// skip robot_flags2
			//write_FIX  (BotGens (i)->hitPoints, savefile);
			//write_FIX  (BotGens (i)->interval, savefile);
			//write_INT16(BotGens (i)->nSegment, savefile);
			//write_INT16(BotGens (i)->nFuelCen, savefile);
	}

	//================ WRITE EQUIPMENT CENTERS INFO============== =
	// note: added robot_flags2 for Descent 2
	GameInfo ().equipgen.offset = ftell(savefile);
	if (IsD2File ())
		for (i = 0; i < GameInfo ().botgen.count; i++) 
			EquipGens (i)->Write (savefile, GameInfo ().fileinfo.version);
		//fwrite(EquipGens (), TotalSize (GameInfo ().equipgen), 1, savefile);

	//============== CALCULATE DELTA LIGHT DATA============ =
	if (IsD2File ())
		UpdateDeltaLights ();

	//================ WRITE DELTA LIGHT INFO============== =
	// note: D2 only
	GameInfo ().lightDeltaIndices.offset = ftell(savefile);
	if ((LevelVersion () >= 15) && (GameInfo ().fileinfo.version >= 34))
		SortDLIndex (0, GameInfo ().lightDeltaIndices.count - 1);
	if (IsD2File ())
		for (i = 0; i < GameInfo ().lightDeltaIndices.count; i++) 
			LightDeltaIndex (i)->Write (savefile, GameInfo ().fileinfo.version, (LevelVersion () >= 15) && (GameInfo ().fileinfo.version >= 34));
		//fwrite(LightDeltaIndex (), TotalSize (GameInfo ().lightDeltaIndices), 1, savefile);

	//================ = WRITE DELTA LIGHTS==================
	// note: D2 only
	GameInfo ().lightDeltaValues.offset = ftell(savefile);
	if (IsD2File ()) {
		//CLightDeltaValue *dl, temp_dl;
		//dl = LightDeltaValues ();
		for (i = 0; i < GameInfo ().lightDeltaValues.count; i++) {
			LightDeltaValues (i)->Write (savefile, GameInfo ().fileinfo.version);
			//memcpy(&temp_dl, dl, (INT16)(GameInfo ().lightDeltaValues.size));
			//fwrite(&temp_dl, (INT16)(GameInfo ().lightDeltaValues.size), 1, savefile);
			//dl++;
		}
	}

	endOffset = ftell(savefile);

	//==================== = UPDATE FILE INFO OFFSETS====================== =
	fseek(savefile, startOffset, SEEK_SET);
	fwrite(&GameInfo (), (INT16)GameInfo ().fileinfo.size, 1, savefile);

	//============ = LEAVE ROUTINE AT LAST WRITTEN OFFSET================== = */
	fseek(savefile, endOffset, SEEK_SET);
#endif //DEMO
	return(0);
}


// ------------------------------------------------------------------------
// UpdateDeltaLights ()
// ------------------------------------------------------------------------
void CMine::UpdateDeltaLights ()
{
return;
	bool found = FALSE;
	CSegment *segP = Segments (0);
	INT32 nSegment;
	for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++) {
		INT32 nSide;
		for (nSide = 0; nSide < 6; nSide++) {
			INT16 tmapnum2 = segP->sides [nSide].nOvlTex & 0x1fff;
			if (IsLight(tmapnum2) != -1) {
				found = TRUE;
				break;
			}
		}
		if (found) break;
	}
	if (found) {
		if (QueryMsg("Would you like to update the delta light values?\n\n"
			"Note: These values are used for blinking, \n"
			"exploding, and trigger controlled lights.\n")== IDYES) {
			CalcDeltaLightData(1.0, 1);
		}
	}
}

// --------------------------------------------------------------------------
//eof mine.cpp