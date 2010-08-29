// Copyright (c) 1998 Bryan Aamot, Brainware

#include "stdafx.h"
#include "dle-xp-res.h"

#include < math.h>
#include "define.h"
#include "types.h"
#include "global.h"
#include "mine.h"
#include "matrix.h"
#include "cfile.h"
#include "customtextures.h"
#include "palette.h"
#include "dle-xp.h"
#include "robot.h"
#include "hogmanager.h"
#include "light.h"

#ifdef ALLOCATE_tPolyModelS
#undef ALLOCATE_tPolyModelS
#endif
#define ALLOCATE_tPolyModelS 0

#define ENABLE_TEXT_DUMP 0

// ------------------------------------------------------------------------
// ReadObject()
// ------------------------------------------------------------------------
// CMine - save()
//
// ACTION -  saves a level (.RDL) file to disk
// ------------------------------------------------------------------------

short CMine::Save (const char * szFile, bool bSaveToHog)
{
	CFileManager	fp;
	char				filename [256];
	int				minedataOffset, gamedataOffset, hostageTextOffset;
	int				mineErr, gameErr;
	int				i;

if (fp.Open (filename, "wb"))
	return 1;

m_changesMade = 0;
strcpy_s (filename, sizeof (filename), szFile);
// write file signature
fp.WriteInt32 ('P'*0x1000000L + 'L'*0x10000L + 'V'*0x100 + 'L'); // signature

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
fp.WriteInt32 (LevelVersion ());
fp.WriteInt32 (0); // minedataOffset (temporary)
fp.WriteInt32 (0); // gamedataOffset (temporary)

if (IsD2File ()&& (LevelVersion () >= 8)) {
	fp.WriteInt16 (rand ());
	fp.WriteInt16 (rand ());
	fp.WriteInt16 (rand ());
	fp.WriteSByte ((sbyte) rand ());
	}

if (m_fileType== RDL_FILE)
	fp.WriteInt32 (0); // hostageTextOffset (temporary)
else {
	// save palette name
	char *name = strrchr (descent2_path, '\\');
	if (!name) 
		name = descent2_path; // point to 1st char if no slash found
	else
		name++;               // point to character after slash
	char palette_name [15];
	strncpy_s (palette_name, sizeof (palette_name), name, 12);
	palette_name [13] = null;  // null terminate just in case
	// replace extension with *.256
	if (strlen ((char *)palette_name) > 4)
		strcpy_s (&palette_name [strlen ((char *) palette_name) - 4], 5, ".256");
	else
		strcpy_s (palette_name, sizeof (palette_name), "GROUPA.256");
	_strupr_s (palette_name, sizeof (palette_name));
	strcat_s (palette_name, sizeof (palette_name), "\n"); // add a return to the end
	fp.Write (palette_name, strlen ((char *) palette_name), 1);
	}

// write reactor info
if (IsD2File ()) {
	// read descent 2 reactor information
	fp.Write (ReactorTime ());
	fp.Write (ReactorStrength ());
	// flickering light new for version 7
	if (FlickerLightCount () > MAX_FLICKERING_LIGHTS) 
		FlickerLightCount () = MAX_FLICKERING_LIGHTS;
	fp.WriteInt32 ((int) FlickerLightCount ());
	for (i = 0; i < FlickerLightCount (); i++)
		FlickeringLights (i)->Write (fp);

	// write secret cube number
	fp.Write (SecretCubeNum ());
	// write secret cube orientation?
	fp.Write (SecretOrient ());
	}
// save mine data
minedataOffset = fp.Tell ();
if (0 > (mineErr = SaveMineDataCompiled (fp))) {
	fp.Close ();
	ErrorMsg ("Error saving mine data");
	return(2);
	}

// save game data
gamedataOffset = fp.Tell ();
if (0 > (gameErr = SaveGameData (fp))) {
	fp.Close ();
	ErrorMsg ("Error saving game data");
	return(3);
	}

// save hostage data
hostageTextOffset = fp.Tell ();
// leave hostage text empty

// now and go back to beginning of file and save offsets
fp.Seek (2 * sizeof (int), SEEK_SET);
fp.Write (minedataOffset);    // gamedataOffset
fp.Write (gamedataOffset);    // gamedataOffset
if (m_fileType == RDL_FILE) 
	fp.Write (hostageTextOffset); // hostageTextOffset
fp.Close ();

if (textureManager.HasCustomTextures () && !bSaveToHog) {
	char* ps = strstr (filename, ".");
	if (ps)
		strcpy_s (ps, sizeof (filename) - (ps - filename), ".pog");
	else
		strcat_s (filename, sizeof (filename), ".pog");
	if (!fp.Open (filename, "wb")) {
		CreatePog (fp);
		fp.Close ();
		}
	}

if (HasCustomRobots () && !bSaveToHog) {
	char* ps = strstr (filename, ".");
	if (ps)
		strcpy_s (ps, sizeof (filename) - (ps - filename), ".hxm");
	else
		strcat_s (filename, sizeof (filename), ".hxm");
	if (!fp.Open (filename, "wb"))
		WriteHxmFile (fp);
	}
return 0;
}

// ------------------------------------------------------------------------

void CMine::SortDLIndex (int left, int right)
{
	int	l = left,
			r = right,
			m = (left + right) / 2;
	short	mSeg = LightDeltaIndex (m)->m_nSegment, 
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
	//while ((pr->m_info.nSegment > mSeg) || ((pr->m_info.nSegment == mSeg) && (pr->nSide > mSide))) {
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

int CMine::SaveGameItem (CFileManager& fp, CGameItemInfo& info, CGameItem* items, bool bFlag)
{
info.offset = fp.Tell ();
for (int i = 0; i < info.count; i++) {
	items->Write (fp, GameInfo ().fileinfo.version, bFlag);
	items = items->Next ();
	}
return info.count;
}
									
// ------------------------------------------------------------------------
// SaveMineDataCompiled()
//
// ACTION - Writes a mine data portion of RDL file.
// ------------------------------------------------------------------------

short CMine::SaveMineDataCompiled (CFileManager& fp)
{
	int	i;
// write version (1 byte)
fp.WriteSByte (COMPILED_MINE_VERSION);

// write no. of vertices (2 bytes)
fp.WriteInt16 (VertCount ());

// write number of Segments () (2 bytes)
fp.WriteInt16 (SegCount ());

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
short CMine::SaveGameData(CFileManager& fp)
{
HINSTANCE hInst = AfxGetInstanceHandle();

int i;
int startOffset, endOffset;

startOffset = fp.Tell ();

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

fp.Write (&GameInfo (), (short)GameInfo ().fileinfo.size, 1);
if (GameInfo ().fileinfo.version >= 14) {  /*save mine filename */
	fp.Write (m_currentLevelName, sizeof (char), strlen (m_currentLevelName));
}
if (IsD2File ()) {
	fp.Write ("\n", 1, 1); // write an end - of - line
} else {
	fp.Write ("", 1, 1);   // write a null
}

// write pof names from resource file
byte*				savePofNamesP;
short				nSavePofNames, nPofs;
CResource	res;

if (IsD2File ()) {
	nSavePofNames = 166;
	fwrite (&nSavePofNames, 2, 1);   // write # of POF names
	}
else {
	nSavePofNames = 78;
	nPofs = 25;   // Don't know exactly what this value is for or why it is 25?
	fp.Write (&nPofs, 2, 1);
	}

if (!(savePofNamesP = res.Load (IsD1File () ? IDR_POF_NAMES1 : IDR_POF_NAMES2)))
	return 1;

fp.Write (savePofNamesP, nSavePofNames, 13); // 13 characters each

GameInfo ().player.offset = fp.Tell ();
char* str = "Made with Descent Level Editor XP 32\0\0\0\0\0\0\0";
fp.Write (str, strlen (str) + 1, 1);

SaveGameItem (fp, GameInfo ().objects, DATA (Objects ()));
SaveGameItem (fp, GameInfo ().walls, DATA (Walls ()));
SaveGameItem (fp, GameInfo ().doors, DATA (ActiveDoors ()));
SaveGameItem (fp, GameInfo ().triggers, DATA (Triggers ()));
if (LevelVersion () >= 12) {
	fp.Write (NumObjTriggers ());
	if (NumObjTriggers ()) {
		SortObjTriggers ();
		for (i = 0; i < NumObjTriggers (); i++)
			ObjTriggers (i)->Write (fp, GameInfo ().fileinfo.version, true);
		for (i = 0; i < NumObjTriggers (); i++)
			fp.WriteInt16 (ObjTriggers (i)->m_info.nObject);
		}
	}
SaveGameItem (fp, GameInfo ().control, DATA (ReactorTriggers ()));
SaveGameItem (fp, GameInfo ().botgen, DATA (BotGens ()));
if (IsD2File ()) {
SaveGameItem (fp, GameInfo ().equipgen, DATA (EquipGens ()));
	if ((LevelVersion () >= 15) && (GameInfo ().fileinfo.version >= 34))
		SortDLIndex (0, GameInfo ().lightDeltaIndices.count - 1);
	SaveGameItem (fp, GameInfo ().lightDeltaIndices, DATA (LightDeltaIndex ()));
	SaveGameItem (fp, GameInfo ().lightDeltaValues, DATA (LightDeltaValues ()));
	}

endOffset = fp.Tell ();

//==================== = UPDATE FILE INFO OFFSETS====================== =
fp.Seek (startOffset, SEEK_SET);
fp.Write (&GameInfo (), GameInfo ().fileinfo.size, 1);

//============ = LEAVE ROUTINE AT LAST WRITTEN OFFSET================== = */
fp.Seek (endOffset, SEEK_SET);
return 0;
}

// --------------------------------------------------------------------------
//eof mine.cpp