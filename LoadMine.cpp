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
#include "texturemanager.h"
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

//==========================================================================
// CMine - load()
//
// ACTION -  loads a level (.RDL) file from disk
//
//==========================================================================

short CMine::Load (const char *szFile, bool bLoadFromHog)
{
if (!theMine)
	return 0;

	char filename [256];
	short checkErr;
	bool bNewMine = false;
	CFileManager fp;

// first disable curve generator
m_bSplineActive = FALSE;

//CLEAR (LightColors ());
//CLEAR (VertexColors ());

// if no file passed, define a new level w/ 1 object
if (szFile && *szFile)
	strcpy_s (filename, sizeof (filename), szFile);
else if (!CreateNewLevel ()) {
	CreateLightMap ();
	CFileManager::SplitPath ((m_fileType== RDL_FILE) ? descentPath [0] : missionPath, m_startFolder , null, null);
	sprintf_s (filename, sizeof (filename), (m_fileType== RDL_FILE) ? "%sNEW.RDL" : "%sNEW.RL2", m_startFolder );
	bLoadFromHog = false;
	bNewMine = true;
	}

m_disableDrawing = TRUE;

LoadMine (filename, bLoadFromHog, bNewMine);
if (!bNewMine && (IsD2XLevel ()) && (LevelOutdated ())) {
	if (LevelVersion () < 15) {
		ConvertWallNum (MAX_WALLS2 + 1, MAX_WALLS3 + 1);
		NumObjTriggers () = 0;
		}
	UpdateLevelVersion ();
	}
//CalcDeltaLightData ();
checkErr = FixIndexValues();
if (checkErr != 0) {
	sprintf_s (message, sizeof (message),  "File contains corrupted data. Would you like to load anyway? Error Code %#04x", checkErr);
	if (QueryMsg(message) != IDYES) {
		if (!CreateNewLevel ()) {
			CFileManager::SplitPath ((m_fileType== RDL_FILE) ? descentPath [0] : missionPath, m_startFolder , null, null);
			sprintf_s (filename, sizeof (filename), (IsD1File ()) ? "%sNEW.RDL" : "%sNEW.RL2", m_startFolder );
			bLoadFromHog = false;
			bNewMine = true;
			}
		m_disableDrawing = TRUE;
		LoadMine (filename, bLoadFromHog, bNewMine);
		m_disableDrawing = FALSE;
		return 1;
		}
	}
m_disableDrawing = FALSE;
return 0;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

short CMine::LoadMineSigAndType (CFileManager& fp)
{
int sig = fp.ReadInt32 ();
if (sig != 'P'*0x1000000L + 'L'*0x10000L + 'V'*0x100 + 'L') {
	ErrorMsg ("Signature value incorrect.");
	fp.Close ();
	return 1;
	}

// read version
SetLevelVersion (fp.ReadInt32 ());
if (LevelVersion () == 1) {
	SetFileType (RDL_FILE);
	}
else if ((LevelVersion () >= 6L) && (LevelVersion () <= 21L)) {
	SetFileType (RL2_FILE);
	}
else {
	sprintf_s (message, sizeof (message),  "Version %d unknown. Cannot load this level.", LevelVersion ());
	ErrorMsg (message);
	fp.Close ();
	return 1;
	}
return 0;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void CMine::LoadPaletteName (CFileManager& fp, bool bNewMine)
{
if (IsD2File ()) {
	char	paletteName [256];

	if (LevelVersion () >= 8) {
		fp.ReadInt16 ();
		fp.ReadInt16 ();
		fp.ReadInt16 ();
		fp.ReadSByte ();
		}
	// read palette file name
	int i;
	for (i = 0; i < 15; i++) {
		paletteName [i] = fp.ReadChar ();
		if (paletteName [i]== 0x0a) {
			paletteName [i] = 0;
			break;
			}
		}
	// replace extension with .pig
	if (i >= 4) {
		paletteName [strlen ((char *)paletteName) - 4] = 0;
		strcat_s (paletteName, sizeof (paletteName), ".PIG");
		}
	// try to find new pig file in same directory as Current () pig file
	// 1) cut off old name
	if (!bNewMine) {
		if (descentPath [1] [0] != 0) {
			char *path = strrchr (descentPath [1], '\\');
			if (!path) {
				descentPath [1] [0] = null;
				} 
			else {
				path++;  // leave slash
				*path = null;
				}
			// paste on new *.pig name
			strcat_s (descentPath [1], sizeof (descentPath [1]), paletteName);
			_strlwr_s (descentPath [1], sizeof (descentPath [1]));
			}
		}
	}
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

short CMine::LoadMine (char *filename, bool bLoadFromHog, bool bNewMine)
{
	byte* palette = 0;

	CFileManager fp;
	int	sig = 0;
	int	minedataOffset = 0, gamedataOffset = 0;
	int	mineErr, gameErr = 0;
	int	return_code = 0;
	char*	ps;

m_changesMade = 0;

if (fp.Open (filename, "rb")) {
	sprintf_s (message, sizeof (message),  "Error %d: Can't open file \"%s\".", GetLastError (), filename);
	ErrorMsg (message);
	return -1;
	}
	//  strcpy(gamesave_current_filename, filename);
if (LoadMineSigAndType (fp))
	return -1;
ClearMineData ();
// read mine data offset
minedataOffset = fp.ReadInt32 ();
// read game data offset
gamedataOffset = fp.ReadInt32 ();
LoadPaletteName (fp, bNewMine);

// read descent 2 reactor information
if (IsD2File ()) {
	ReactorTime () = fp.ReadInt32 (); // base control center explosion time
	ReactorStrength () = fp.ReadInt32 (); // reactor strength
	if (LevelVersion () > 6) {
		FlickerLightCount () = short (fp.ReadInt32 ());
		if ((FlickerLightCount () > 0) && (FlickerLightCount () <= MAX_FLICKERING_LIGHTS)) {
			for (int i = 0; i < FlickerLightCount (); i++)
				FlickeringLights (i)->Read (fp);
			} 
		else {
			if (FlickerLightCount () != 0) {
				ErrorMsg ("Error reading flickering lights");
				FlickerLightCount () = 0;
				}
			}
		}
	// NOTE: d2 v1.1 has two levels at version 7 (b2 and f4),
	//       both have 0 flickering lights
	//    sprintf_s (message, sizeof (message),  "%d flickering lights", nflicks);
	//    DEBUGMSG(message);

	// read secret cube number
	SecretCubeNum () = fp.ReadInt32 ();
	// read secret cube orientation?
	fp.Read (SecretOrient ());
	}

m_disableDrawing = TRUE;

fp.Seek (minedataOffset, SEEK_SET);
mineErr = LoadMineDataCompiled (fp, bNewMine);
int fPos = fp.Tell ();

if (mineErr != 0) {
	ErrorMsg ("Error loading mine data");
	fp.Close ();
	return(2);
}

fp.Seek (gamedataOffset, SEEK_SET);
gameErr = LoadGameData (fp, bNewMine);

if (gameErr != 0) {
	ErrorMsg ("Error loading game data");
	// reset "howmany"
	MineInfo ().objects.Reset ();
	MineInfo ().walls.Reset ();
	MineInfo ().doors.Reset ();
	MineInfo ().triggers.Reset ();
	MineInfo ().control.Reset ();
	MineInfo ().botgen.Reset ();
	MineInfo ().equipgen.Reset ();
	MineInfo ().lightDeltaIndices.Reset ();
	MineInfo ().lightDeltaValues.Reset ();
	fp.Close ();
	return 3;
	}

goto load_pog;

return_code = 0;

load_pog:

fp.Close ();
if (!bLoadFromHog) {
	paletteManager.Reload ();
	textureManager.LoadTextures ();
	if (IsD2File ()) {
		ps = strstr (filename, ".");
		if (ps)
			strcpy_s (ps, 256 - (ps - filename), ".pog");
		else
			strcat_s (filename, 256, ".pog");
		if (!fp.Open (filename, "rb")) {
			ReadPog (fp);
			fp.Close ();
			}
		ReadHamFile ();
		if (IsD2File ()) {
			char szHogFile [256], szHamFile [256], *p;
			long nSize, nPos;

			CFileManager::SplitPath (descentPath [1], szHogFile, null, null);
			if (p = strstr (szHogFile, "data"))
				*p = '\0';
			strcat_s (szHogFile, sizeof (szHogFile), "missions\\d2x.hog");
			if (FindFileData (szHogFile, "d2x.ham", &nSize, &nPos, FALSE)) {
				CFileManager::SplitPath (descentPath [1], szHamFile, null, null);
				if (p = strstr (szHamFile, "data"))
					*p = '\0';
				strcat_s (szHamFile, sizeof (szHamFile), "missions\\d2x.ham");
				if (ExportSubFile (szHogFile, szHamFile, nPos + sizeof (struct level_header), nSize)) {
					m_bVertigo = ReadHamFile (szHamFile, EXTENDED_HAM) == 0;
					_unlink (szHamFile);
					}
				}
			}
		ps = strstr (filename, ".");
		if (ps)
			strcpy_s (filename, 256 - (ps - filename), ".hxm");
		else
			strcat_s (filename, 256, ".hxm");
		if (!fp.Open (filename, "rb")) {
			ReadHxmFile (fp, -1);
			fp.Close ();
			}
		}
	}
SortObjects ();
return return_code;
}

// ------------------------------------------------------------------------

bool CMine::HasCustomLightMap (void)
{
CResource res;
byte *dataP;
if (!(dataP = res.Load (IsD1File () ? IDR_LIGHT_D1 : IDR_LIGHT_D2)))
	return false;
return memcmp (lightMap, dataP, sizeof (lightMap)) != 0;
}

// ------------------------------------------------------------------------

bool CMine::HasCustomLightColors (void)
{
CResource res;
byte *dataP;
if (!(dataP = res.Load (IsD1File () ? IDR_COLOR_D1 : IDR_COLOR_D2)))
	return false;
return memcmp (DATA (MineData ().texColors), dataP, sizeof (MineData ().texColors)) != 0;
}

// ------------------------------------------------------------------------

short CMine::LoadDefaultLightAndColor (void)
{
CResource res;
byte *dataP;;
if (!(dataP = res.Load (IsD1File () ? IDR_COLOR_D1 : IDR_COLOR_D2)))
	return false;
int i = res.Size () / (3 * sizeof (int) + sizeof (byte));
#if USE_DYN_ARRAYS
if (i > (int) MineData ().texColors.Length ())
	i = (int) MineData ().texColors.Length ();
#else
if (i > sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]))
	i = sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]);
#endif
for (CColor *colorP = DATA (MineData ().texColors); i; i--, colorP++) {
	colorP->m_info.index = *dataP++;
	colorP->m_info.color.r = (double) *((int *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (int);
	colorP->m_info.color.g = (double) *((int *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (int);
	colorP->m_info.color.b = (double) *((int *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (int);
	}

if (!(dataP = res.Load (IsD1File () ? IDR_LIGHT_D1 : IDR_LIGHT_D2)))
	return false;
memcpy (lightMap, dataP, min (res.Size (), sizeof (lightMap)));
return 1;
}

// ------------------------------------------------------------------------
// FixIndexValues()
//
// Action - This routine attempts to fix array index values to prevent
//          the editor from crashing when the level is drawn.
//
// Returns - 0 if no error is detected
// ------------------------------------------------------------------------

short CMine::FixIndexValues(void)
{
	short 	nSegment, nSide, nVertex;
	ushort	nWall;
	short 	checkErr;

checkErr = 0;
CSegment *segP = Segments (0);
for(nSegment = 0; nSegment < SegCount (); nSegment++, segP++) {
	for(nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		// check wall numbers
		CSide& side = segP->m_sides [nSide];
		if (side.m_info.nWall >= MineInfo ().walls.count && side.m_info.nWall != NO_WALL) {
			side.m_info.nWall = NO_WALL;
			checkErr |= (1 << 0);
			}
		// check children
		if ((segP->Child (nSide) < -2) || (segP->Child (nSide) > (short)SegCount ())) {
			segP->SetChild (nSide, -1);
			checkErr |= (1 << 1);
			}
		}
	// check verts
	for(nVertex = 0; nVertex < MAX_VERTICES_PER_SEGMENT; nVertex++) {
		if ((segP->m_info.verts [nVertex] < 0) || (segP->m_info.verts [nVertex] >= VertCount ())) {
			segP->m_info.verts [nVertex] = 0;  // this will cause a bad looking picture [0]
			checkErr |= (1 << 2);      // but it will prevent a crash
			}
		}
	}
CWall *wallP = Walls (0);
for (nWall = 0; nWall < MineInfo ().walls.count; nWall++, wallP++) {
	// check nSegment
	if (wallP->m_nSegment < 0 || wallP->m_nSegment > SegCount ()) {
		wallP->m_nSegment = 0;
		checkErr |= (1 << 3);
		}
	// check nSide
	if (wallP->m_nSide < 0 || wallP->m_nSide > 5) {
		wallP->m_nSide = 0;
		checkErr |= (1 << 4);
		}
	}
return checkErr;
}

// ------------------------------------------------------------------------
// LoadMineDataCompiled()
//
// ACTION - Reads a mine data portion of RDL file.
// ------------------------------------------------------------------------

short CMine::LoadMineDataCompiled (CFileManager& fp, bool bNewMine)
{
	int		i; 
	byte		version;
	ushort   n_vertices;
	ushort   n_segments;

// read version (1 byte)
version = byte (fp.ReadSByte ());
// read number of vertices (2 bytes)
n_vertices = ushort (fp.ReadInt16 ());
if (n_vertices > MAX_VERTICES3) {
	sprintf_s (message, sizeof (message),  "Too many vertices (%d)", n_vertices);
	ErrorMsg (message);
	return(1);
	}
if (((IsD1File ()) && (n_vertices > MAX_VERTICES1)) ||
	 ((IsD2File ()) && (IsStdLevel ()) && (n_vertices > MAX_VERTICES2)))
	ErrorMsg ("Warning: Too many vertices for this level version");

// read number of Segments () (2 bytes)
n_segments = ushort (fp.ReadInt16 ());
if (n_segments > MAX_SEGMENTS3) {
	sprintf_s (message, sizeof (message), "Too many Segments (%d)", n_segments);
	ErrorMsg (message);
	return 2;
	}
if (((IsD1File ()) && (n_segments > MAX_SEGMENTS1)) ||
	 ((IsD2File ()) && (IsStdLevel ()) && (n_segments > MAX_SEGMENTS2)))
	ErrorMsg ("Warning: Too many Segments for this level version");

// if we are happy with the number of verts and Segments (), then proceed...
//ClearMineData ();
VertCount () = n_vertices;
SegCount () = n_segments;

// read all vertices
CVertex* vertP = Vertices (0);
size_t fPos = fp.Tell ();
for (i = 0; i < VertCount (); i++, vertP++) {
	vertP->Read (fp);
	vertP->m_status = 0;
	}

// read segment information
CSegment *segP;
for (i = 0, segP = Segments (0); i < SegCount (); i++, segP++)   
	segP->Read (fp, IsD2XLevel () ? 2 : IsD2File () ? 1 : 0, LevelVersion());
if (IsD2File ())
	for (i = 0, segP = Segments (0); i < SegCount (); i++, segP++)   
		segP->ReadExtras (fp, IsD2XLevel () ? 2 : 1, LevelVersion(), true);

if (LevelVersion () == 9) {
#if 1
	LoadColors (LightColors (0), SegCount () * 6, 9, 14, fp);
	LoadColors (LightColors (0), SegCount () * 6, 9, 14, fp);
	LoadColors (VertexColors (0), VertCount (), 9, 15, fp);
#else
	fp.Read (LightColors (), sizeof (CColor), SegCount () * 6); //skip obsolete side colors 
	fp.Read (LightColors (), sizeof (CColor), SegCount () * 6);
	fp.Read (VertexColors (), sizeof (CColor), VertCount ());
#endif
	}
else if (LevelVersion () > 9) {
	LoadColors (VertexColors (0), VertCount (), 9, 15, fp);
	LoadColors (LightColors (0), SegCount () * 6, 9, 14, fp);
	LoadColors (TexColors (0), MAX_TEXTURES_D2, 10, 16, fp);
	}
if (MineInfo ().objects.count > MAX_OBJECTS) {
	sprintf_s (message, sizeof (message),  "Warning: Max number of objects for this level version exceeded (%ld/%d)", 
			  MineInfo ().objects.count, MAX_OBJECTS2);
	ErrorMsg (message);
	}
return 0;
}

// ------------------------------------------------------------------------

int CMine::LoadGameItem (CFileManager& fp, CGameItemInfo info, CGameItem* items, int nMinVersion, int nMaxCount, char *pszItem, bool bFlag)
{
if (info.offset < 0)
	return 0;
if (fp.Seek (info.offset, SEEK_SET))
	return -1;
if (info.count < 1)
	return 0;
if (info.count > nMaxCount) {
	sprintf_s (message, sizeof (message),  "Error: Too many %s (%d/%d)", pszItem, info.count, nMaxCount);
	ErrorMsg (message);
	return -1;
	}
if (MineInfo ().fileInfo.version < nMinVersion) {
	sprintf_s (message, sizeof (message), "%s version < %d, walls not loaded", pszItem, nMinVersion);
	ErrorMsg (message);
	return 0;
	}

int nVersion =  MineInfo ().fileInfo.version;
for (int i = 0; i < info.count; i++) {
	if (!items->Read (fp, nVersion, bFlag)) {
		sprintf_s (message, sizeof (message), "Error reading %s", pszItem);
		ErrorMsg (message);
		return -1;
		}
	items = items->Next ();
	}
return info.count;
}

// ------------------------------------------------------------------------
// LoadGameData()
//
// ACTION - Loads the player, object, wall, door, trigger, and
//          materialogrifizationator data from an RDL file.
// ------------------------------------------------------------------------

short CMine::LoadGameData (CFileManager& fp, bool bNewMine) 
{
	int startOffset;

startOffset = fp.Tell ();

// Set default values
MineInfo ().objects.Reset ();
MineInfo ().walls.Reset ();
MineInfo ().doors.Reset ();
MineInfo ().triggers.Reset ();
MineInfo ().control.Reset ();
MineInfo ().botgen.Reset ();
MineInfo ().equipgen.Reset ();
MineInfo ().lightDeltaIndices.Reset ();
MineInfo ().lightDeltaValues.Reset ();

//==================== = READ FILE INFO========================

// Read in MineFileInfo () to get size of saved fileinfo.
if (fp.Seek (startOffset, SEEK_SET)) {
	ErrorMsg ("Error seeking in mine.cpp");
	return -1;
	}
// Check signature
MineInfo ().Read (fp);
if (MineFileInfo ().signature != 0x6705) {
	ErrorMsg ("Game data signature incorrect");
	return -1;
	}
if (MineInfo ().fileInfo.version < 14) 
	m_currentLevelName [0] = 0;
else {  /*load mine filename */
	for (char *p = m_currentLevelName; ; p++) {
		*p = fp.ReadChar ();
		if (*p== '\n')
			*p = 0;
		if (*p == 0)
			break;
		}
	}

if (0 > LoadGameItem (fp, MineInfo ().objects, Objects (0), -1, MAX_OBJECTS, "Objects"))
	return -1;
if (0 > LoadGameItem (fp, MineInfo ().walls, Walls (0), 20, MAX_WALLS, "Walls"))
	return -1;
if (0 > LoadGameItem (fp, MineInfo ().doors, ActiveDoors (0), 20, MAX_DOORS, "Doors"))
	return -1;
if (0 > LoadGameItem (fp, MineInfo ().triggers, Triggers (0), -1, MAX_TRIGGERS, "Triggers"))
	return -1;
if (MineInfo ().triggers.offset > -1) {
	int bObjTriggersOk = 1;
	if (MineInfo ().fileInfo.version >= 33) {
		NumObjTriggers () = fp.ReadInt32 ();
		for (int i = 0; i < NumObjTriggers (); i++)
			ObjTriggers (i)->Read (fp, MineInfo ().fileInfo.version, true);
		if (MineInfo ().fileInfo.version >= 40) {
			for (int i = 0; i < NumObjTriggers (); i++)
				ObjTriggers (i)->m_info.nObject = fp.ReadInt16 ();
			}
		else {
			for (int i = 0; i < NumObjTriggers (); i++) {
				fp.ReadInt16 ();
				fp.ReadInt16 ();
				ObjTriggers (i)->m_info.nObject = fp.ReadInt16 ();
				}
			if (MineInfo ().fileInfo.version < 36)
				fp.Seek (700 * sizeof (short), SEEK_CUR);
			else
				fp.Seek (2 * sizeof (short) * fp.ReadInt16 (), SEEK_CUR);
			}
		}
	if (bObjTriggersOk && NumObjTriggers ())
		SortObjTriggers ();
	else {
		NumObjTriggers () = 0;
		CLEAR (ObjTriggers ());
		}
	}

if (0 > LoadGameItem (fp, MineInfo ().control, ReactorTriggers (0), -1, MAX_REACTOR_TRIGGERS, "Reactor triggers"))
	return -1;
if (0 > LoadGameItem (fp, MineInfo ().botgen, BotGens (0), -1, MAX_ROBOT_MAKERS, "Robot makers"))
	return -1;
if (0 > LoadGameItem (fp, MineInfo ().equipgen, EquipGens (0), -1, MAX_ROBOT_MAKERS, "Equipment makers"))
	return -1;
if (IsD2File ()) {
	if (0 > LoadGameItem (fp, MineInfo ().lightDeltaIndices, LightDeltaIndex (0), -1, MAX_LIGHT_DELTA_INDICES, "Light delta indices", (LevelVersion () >= 15) && (MineInfo ().fileInfo.version >= 34)))
		return -1;
	if (0 > LoadGameItem (fp, MineInfo ().lightDeltaValues, LightDeltaValues (0), -1, MAX_LIGHT_DELTA_VALUES, "Light delta values"))
		return -1;
	}

return 0;
}

// --------------------------------------------------------------------------
//eof mine.cpp