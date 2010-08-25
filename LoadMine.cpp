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

INT16 CMine::Load (const char *filename_passed, bool bLoadFromHog)
{
if (!theMine)
	return 0;

char filename [256];
INT16 checkErr;
bool bNewMine = false;

// first disable curve generator
m_bSplineActive = FALSE;

//CLEAR (LightColors ());
//CLEAR (VertexColors ());

// if no file passed, define a new level w/ 1 object
FreeCustomPalette ();
if (filename_passed && *filename_passed)
	strcpy_s (filename, sizeof (filename), filename_passed);
else if (!CreateNewLevel ()) {
	CreateLightMap ();
	FSplit ((m_fileType== RDL_FILE) ? descent_path : levels_path, m_startFolder , NULL, NULL);
	sprintf_s (filename, sizeof (filename), (m_fileType== RDL_FILE) ? "%sNEW.RDL" : "%sNEW.RL2", m_startFolder );
	bLoadFromHog = false;
	bNewMine = true;
	}

m_disableDrawing = TRUE;
if (!bLoadFromHog)
	FreeTextureHandles ();

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
			FSplit ((m_fileType== RDL_FILE) ? descent_path : levels_path, m_startFolder , NULL, NULL);
			sprintf_s (filename, sizeof (filename), (IsD1File ()) ? "%sNEW.RDL" : "%sNEW.RL2", m_startFolder );
			bLoadFromHog = false;
			bNewMine = true;
			}
		m_disableDrawing = TRUE;
		FreeTextureHandles ();
		LoadMine(filename, bLoadFromHog, bNewMine);
		m_disableDrawing = FALSE;
		return 1;
		}
	}
m_disableDrawing = FALSE;
return 0;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

INT16 CMine::LoadPalette (void)
{
HINSTANCE hInst = AfxGetInstanceHandle();
// set palette
// make global palette
UINT8 *palette = PalettePtr ();
ASSERT(palette);
if (!palette)
	return 1;
// redefine logical palette entries if memory for it is allocated
m_dlcLogPalette = (LPLOGPALETTE) malloc (sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * 256);
if (!m_dlcLogPalette) {
	FreePaletteResource ();
	return 1;
	}
m_dlcLogPalette->palVersion = 0x300;
m_dlcLogPalette->palNumEntries = 256;
INT32 i;
for (i = 0; i < 256; ++i) {
	m_dlcLogPalette->palPalEntry [i].peRed = palette [i*3 + 0] << 2;
	m_dlcLogPalette->palPalEntry [i].peGreen = palette [i*3 + 1] << 2;
	m_dlcLogPalette->palPalEntry [i].peBlue = palette [i*3 + 2] << 2;
	m_dlcLogPalette->palPalEntry [i].peFlags = PC_RESERVED;
}
// now recreate the global Palette
if (m_currentPalette)
	delete m_currentPalette;
m_currentPalette = new CPalette ();
m_currentPalette->CreatePalette (m_dlcLogPalette);
FreePaletteResource ();
return 0;
}

// ------------------------------------------------------------------------

INT16 CMine::LoadMineSigAndType (FILE* fp)
{
INT32 sig = read_INT32 (fp);
if (sig != 'P'*0x1000000L + 'L'*0x10000L + 'V'*0x100 + 'L') {
	ErrorMsg ("Signature value incorrect.");
	fclose (fp);
	return 1;
	}

// read version
SetLevelVersion (read_INT32 (fp));
if (LevelVersion () == 1) {
	SetFileType (RDL_FILE);
	texture_resource = D1_TEXTURE_STRING_TABLE;
	}
else if ((LevelVersion () >= 6L) && (LevelVersion () <= 21L)) {
	SetFileType (RL2_FILE);
	texture_resource = D2_TEXTURE_STRING_TABLE;
	}
else {
	sprintf_s (message, sizeof (message),  "Version %d unknown. Cannot load this level.", LevelVersion ());
	ErrorMsg (message);
	fclose (fp);
	return 1;
	}
return 0;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

INT16 CMine::LoadMine (char *filename, bool bLoadFromHog, bool bNewMine)
{
	HINSTANCE hInst = AfxGetInstanceHandle();
	UINT8* palette = 0;

	FILE* fp = 0;
	INT32 sig = 0;
	INT32 minedataOffset = 0;
	INT32 gamedataOffset = 0;
	INT32 mineErr, gameErr = 0;
	INT32	return_code = 0;
	char	palette_name [256];
	char*	ps;

m_changesMade = 0;
fopen_s (&fp, filename, "rb");

if (!fp) {
	sprintf_s (message, sizeof (message),  "Error %d: Can't open file \"%s\".", GetLastError (), filename);
	ErrorMsg (message);
	return -1;
	}
	//  strcpy(gamesave_current_filename, filename);
if (LoadMineSigAndType (fp))
	return -1;
ClearMineData ();
// read mine data offset
minedataOffset = read_INT32 (fp);
// read game data offset
gamedataOffset = read_INT32 (fp);

// don't bother reading  hostagetextOffset since
// for Descent 1 files since we dont use it anyway
// hostagetextOffset = read_INT32(fp);

// read palette name *.256
if (IsD2File ()) {
	if (LevelVersion () >= 8) {
		read_INT16 (fp);
		read_INT16 (fp);
		read_INT16 (fp);
		read_INT8 (fp);
		}
	// read palette file name
	INT32 i;
	for (i = 0; i < 15; i++) {
		palette_name [i] = fgetc(fp);
		if (palette_name [i]== 0x0a) {
			palette_name [i] = NULL;
			break;
			}
		}
	// replace extension with .pig
	if (i >= 4) {
		palette_name [strlen ((char *)palette_name) - 4] = NULL;
		strcat_s (palette_name, sizeof (palette_name), ".PIG");
		}
	// try to find new pig file in same directory as Current () pig file
	// 1) cut off old name
	if (!bNewMine) {
		if (descent2_path [0] != NULL) {
			char *path = strrchr (descent2_path, '\\');
			if (!path) {
				descent2_path [0] = NULL;
				} 
			else {
				path++;  // leave slash
				*path = NULL;
				}
			// paste on new *.pig name
			strcat_s (descent2_path, sizeof (descent2_path), palette_name);
			_strlwr_s (descent2_path, sizeof (descent2_path));
			}
		}
	}

if (return_code = LoadPalette ())
	goto load_end;

// read descent 2 reactor information
if (IsD2File ()) {
	ReactorTime () = read_INT32 (fp); // base control center explosion time
	ReactorStrength () = read_INT32 (fp); // reactor strength
	if (LevelVersion () > 6) {
		FlickerLightCount () = INT16 (read_INT32 (fp));
		if ((FlickerLightCount () > 0) && (FlickerLightCount () <= MAX_FLICKERING_LIGHTS)) {
			for (INT32 i = 0; i < FlickerLightCount (); i++)
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
	SecretCubeNum () = read_INT32(fp);
	// read secret cube orientation?
	read_matrix(&SecretOrient (), fp);
	}

m_disableDrawing = TRUE;

fseek (fp, minedataOffset, SEEK_SET);
mineErr = LoadMineDataCompiled (fp, bNewMine);
int fPos = ftell (fp);

if (mineErr != 0) {
	ErrorMsg ("Error loading mine data");
	fclose(fp);
	return(2);
}

fseek(fp, gamedataOffset, SEEK_SET);
gameErr = LoadGameData(fp, bNewMine);

if (gameErr != 0) {
	ErrorMsg ("Error loading game data");
	// reset "howmany"
	GameInfo ().objects.Reset ();
	GameInfo ().walls.Reset ();
	GameInfo ().doors.Reset ();
	GameInfo ().triggers.Reset ();
	GameInfo ().control.Reset ();
	GameInfo ().botgen.Reset ();
	GameInfo ().equipgen.Reset ();
	GameInfo ().lightDeltaIndices.Reset ();
	GameInfo ().lightDeltaValues.Reset ();
	fclose(fp);
	return(3);
	}

goto load_pog;

load_end:
	return_code = 0;
load_pog:

	fclose(fp);
if (!bLoadFromHog && (IsD2File ())) {
	ps = strstr (filename, ".");
	if (ps)
		strcpy_s (ps, 256 - (ps - filename), ".pog");
	else
		strcat_s (filename, 256, ".pog");
	fopen_s (&fp, filename, "rb");
	if (fp) {
		ReadPog (fp);
		fclose (fp);
		}
	ReadHamFile ();
#if 0
	char szHamFile [256];
	FSplit (descent2_path, szHamFile, NULL, NULL);
	strcat (szHamFile, "hoard.ham");
	ReadHamFile (szHamFile, EXTENDED_HAM);
#endif
#if 1
	if (IsD2File ()) {
		char szHogFile [256], szHamFile [256], *p;
		long nSize, nPos;

		FSplit (descent2_path, szHogFile, NULL, NULL);
		if (p = strstr (szHogFile, "data"))
			*p = '\0';
		strcat_s (szHogFile, sizeof (szHogFile), "missions\\hog");
		if (FindFileData (szHogFile, "ham", &nSize, &nPos, FALSE)) {
			FSplit (descent2_path, szHamFile, NULL, NULL);
			if (p = strstr (szHamFile, "data"))
				*p = '\0';
			strcat_s (szHamFile, sizeof (szHamFile), "missions\\ham");
			if (ExportSubFile (szHogFile, szHamFile, nPos + sizeof (struct level_header), nSize)) {
				m_bVertigo = ReadHamFile (szHamFile, EXTENDED_HAM) == 0;
				_unlink (szHamFile);
				}
			}
		}
#endif
	ps = strstr (filename, ".");
	if (ps)
		strcpy_s (filename, 256 - (ps - filename), ".hxm");
	else
		strcat_s (filename, 256, ".hxm");
	fopen_s (&fp, filename, "rb");
	if (fp) {
		ReadHxmFile (fp, -1);
		fclose (fp);
		}
	}
SortObjects ();
return return_code;
}

// ------------------------------------------------------------------------

UINT8 *CMine::LoadDataResource (LPCTSTR pszRes, HGLOBAL& hGlobal, UINT32& nResSize)
{
HINSTANCE hInst = AfxGetInstanceHandle ();
HRSRC hRes = FindResource (hInst, pszRes, "RC_DATA");
if (!hRes)
	return NULL;
if (!(hGlobal = LoadResource (hInst, hRes)))
	return NULL;
nResSize = SizeofResource (hInst, hRes);
return (UINT8 *) LockResource (hGlobal);
}

// ------------------------------------------------------------------------

BOOL CMine::HasCustomLightMap (void)
{
HGLOBAL hGlobal = 0;
UINT32 nSize = 0;
UINT8 *dataP = LoadDataResource (MAKEINTRESOURCE ((IsD1File ()) ? IDR_LIGHT_D1 : IDR_LIGHT_D2), hGlobal, nSize);
if (!dataP)
	return 0;
BOOL bCustom = memcmp (lightMap, dataP, sizeof (lightMap)) != 0;
FreeResource (hGlobal);
return bCustom;
}

// ------------------------------------------------------------------------

BOOL CMine::HasCustomLightColors (void)
{
HGLOBAL hGlobal = 0;
UINT32 nSize = 0;
UINT8 *dataP = LoadDataResource (MAKEINTRESOURCE ((IsD1File ()) ? IDR_COLOR_D1 : IDR_COLOR_D2), hGlobal, nSize);
if (!dataP)
	return 0;
BOOL bCustom = memcmp (DATA (MineData ().texColors), dataP, sizeof (MineData ().texColors)) != 0;
FreeResource (hGlobal);
return bCustom;
}

// ------------------------------------------------------------------------

INT16 CMine::LoadDefaultLightAndColor (void)
{
HGLOBAL hGlobal = 0;
UINT32 nSize = 0;
UINT8 *dataP = LoadDataResource (MAKEINTRESOURCE ((IsD1File ()) ? IDR_COLOR_D1 : IDR_COLOR_D2), hGlobal, nSize);
if (!dataP)
	return 0;
INT32 i = nSize / (3 * sizeof (INT32) + sizeof (UINT8));
#if USE_DYN_ARRAYS
if (i > (int) MineData ().texColors.Length ())
	i = (int) MineData ().texColors.Length ();
#else
if (i > sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]))
	i = sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]);
#endif
for (CColor *colorP = DATA (MineData ().texColors); i; i--, colorP++) {
	colorP->m_info.index = *dataP++;
	colorP->m_info.color.r = (double) *((INT32 *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (INT32);
	colorP->m_info.color.g = (double) *((INT32 *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (INT32);
	colorP->m_info.color.b = (double) *((INT32 *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (INT32);
	}
FreeResource (hGlobal);
dataP = LoadDataResource (MAKEINTRESOURCE ((IsD1File ()) ? IDR_LIGHT_D1 : IDR_LIGHT_D2), hGlobal, nSize);
if (!dataP)
	return 0;
memcpy (lightMap, dataP, min (nSize, sizeof (lightMap)));
FreeResource (hGlobal);
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
INT16 CMine::FixIndexValues(void)
{
	INT16 	nSegment, nSide, nVertex;
	UINT16	nWall;
	INT16 	checkErr;

	checkErr = 0;
	CSegment *segP = Segments (0);
	for(nSegment = 0; nSegment < SegCount (); nSegment++, segP++) {
		for(nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
			// check wall numbers
			CSide& side = segP->m_sides [nSide];
			if (side.m_info.nWall >= GameInfo ().walls.count && side.m_info.nWall != NO_WALL) {
				side.m_info.nWall = NO_WALL;
				checkErr |= (1 << 0);
			}
			// check children
			if (segP->m_info.children [nSide] < - 2 || segP->m_info.children [nSide] >(INT16)SegCount ()) {
				segP->m_info.children [nSide] =-1;
				checkErr |= (1 << 1);
			}
		}
		// check verts
		for(nVertex = 0; nVertex < MAX_VERTICES_PER_SEGMENT; nVertex++) {
			if (segP->m_info.verts [nVertex] < 0 || segP->m_info.verts [nVertex] >= VertCount ()) {
				segP->m_info.verts [nVertex] = 0;  // this will cause a bad looking picture [0]
				checkErr |= (1 << 2);      // but it will prevent a crash
			}
		}
	}
	CWall *wallP = Walls (0);
	for (nWall = 0; nWall < GameInfo ().walls.count; nWall++, wallP++) {
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

INT16 CMine::LoadMineDataCompiled (FILE *fp, bool bNewMine)
{
	INT32    i; 
	UINT8    version;
	UINT16   n_vertices;
	UINT16   n_segments;

// read version (1 byte)
version = UINT8 (read_INT8 (fp));
// read number of vertices (2 bytes)
n_vertices = UINT16 (read_INT16 (fp));
if (n_vertices > MAX_VERTICES3) {
	sprintf_s (message, sizeof (message),  "Too many vertices (%d)", n_vertices);
	ErrorMsg (message);
	return(1);
	}
if (((IsD1File ()) && (n_vertices > MAX_VERTICES1)) ||
	 ((IsD2File ()) && (IsStdLevel ()) && (n_vertices > MAX_VERTICES2)))
	ErrorMsg ("Warning: Too many vertices for this level version");

// read number of Segments () (2 bytes)
n_segments = UINT16 (read_INT16 (fp));
if (n_segments > MAX_SEGMENTS3) {
	sprintf_s (message, sizeof (message), "Too many Segments (%d)", n_segments);
	ErrorMsg (message);
	return(2);
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
size_t fPos = ftell (fp);
for (i = 0; i < VertCount (); i++, vertP++) {
	vertP->Read (fp);
	vertP->m_status = 0;
	}

if (n_vertices != VertCount ()) 
	fseek(fp, sizeof (CFixVector) * (n_vertices - VertCount ()), SEEK_CUR);

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
	fread (LightColors (), sizeof (CColor), SegCount () * 6, fp); //skip obsolete side colors 
	fread (LightColors (), sizeof (CColor), SegCount () * 6, fp);
	fread (VertexColors (), sizeof (CColor), VertCount (), fp);
#endif
	}
else if (LevelVersion () > 9) {
	LoadColors (VertexColors (0), VertCount (), 9, 15, fp);
	LoadColors (LightColors (0), SegCount () * 6, 9, 14, fp);
	LoadColors (TexColors (0), MAX_D2_TEXTURES, 10, 16, fp);
	}
if (GameInfo ().objects.count > MAX_OBJECTS) {
	sprintf_s (message, sizeof (message),  "Warning: Max number of objects for this level version exceeded (%ld/%d)", 
			  GameInfo ().objects.count, MAX_OBJECTS2);
	ErrorMsg (message);
	}
return 0;
}

// ------------------------------------------------------------------------

INT32 CMine::LoadGameItem (FILE* fp, CGameItemInfo info, CGameItem* items, int nMinVersion, int nMaxCount, char *pszItem, bool bFlag)
{
if ((info.offset < 0) || (info.count < 1))
	return 0;
if (fseek (fp, info.offset, SEEK_SET))
	return -1;
if (info.count > nMaxCount) {
	sprintf_s (message, sizeof (message),  "Error: Too many %s (%d/%d)", pszItem, info.count, nMaxCount);
	ErrorMsg (message);
	return -1;
	}
if (GameInfo ().fileinfo.version < nMinVersion) {
	sprintf_s (message, sizeof (message), "%s version < %d, walls not loaded", pszItem, nMinVersion);
	ErrorMsg (message);
	return 0;
	}

for (int i = 0; i < info.count; i++) {
	if (!items->Read (fp, GameInfo ().fileinfo.version, bFlag)) {
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

INT16 CMine::LoadGameData(FILE *loadfile, bool bNewMine) 
{
	INT32 startOffset;

startOffset = ftell(loadfile);

// Set default values
GameInfo ().objects.Reset ();
GameInfo ().walls.Reset ();
GameInfo ().doors.Reset ();
GameInfo ().triggers.Reset ();
GameInfo ().control.Reset ();
GameInfo ().botgen.Reset ();
GameInfo ().equipgen.Reset ();
GameInfo ().lightDeltaIndices.Reset ();
GameInfo ().lightDeltaValues.Reset ();

//==================== = READ FILE INFO========================

// Read in gameFileInfo to get size of saved fileinfo.
if (fseek(loadfile, startOffset, SEEK_SET)) {
	ErrorMsg ("Error seeking in mine.cpp");
	return -1;
	}
if (fread(&gameFileInfo, sizeof (gameFileInfo), 1, loadfile) != 1) {
	ErrorMsg ("Error reading game info in mine.cpp");
	return -1;
	}
// Check signature
if (gameFileInfo.signature != 0x6705) {
	ErrorMsg ("Game data signature incorrect");
	return -1;
	}
// Check version number
//  if (gameFileInfo.version < GAME_COMPATIBLE_VERSION)
//    return -1;

// Now, Read in the fileinfo
if (fseek(loadfile, startOffset, SEEK_SET)) {
	ErrorMsg ("Error seeking to game info in mine.cpp");
	return -1;
	}
if (fread(&GameInfo (), (INT16)gameFileInfo.size, 1, loadfile)!= 1) {
	ErrorMsg ("Error reading game info from mine.cpp");
	return -1;
	}
if (GameInfo ().fileinfo.version < 14) 
	m_currentLevelName [0] = 0;
else {  /*load mine filename */
	char *p;
	for (p = m_currentLevelName; ; p++) {
		*p = fgetc(loadfile);
		if (*p== '\n') *p = 0;
		if (*p== 0) break;
		}
	}

//===================== READ OBJECT INFO=======================

#if 1
	if (0 > LoadGameItem (loadfile, GameInfo ().objects, Objects (0), -1, MAX_OBJECTS, "Objects"))
		return -1;
#else
	if (GameInfo ().objects.offset > -1) {
		if (fseek(loadfile, GameInfo ().objects.offset, SEEK_SET))
			ErrorMsg ("Error seeking to objects.");
		else if (GameInfo ().objects.count > MAX_OBJECTS) {
			sprintf_s (message, sizeof (message),  "Error: Max number of objects (%ld/%d) exceeded", 
					  GameInfo ().objects.count, MAX_OBJECTS);
			ErrorMsg (message);
			GameInfo ().objects.count = MAX_OBJECTS;
			}
		else {
			for (int i = 0; i < GameInfo ().objects.count; i++) {
				Objects (i)->Read (loadfile, GameInfo ().fileinfo.version);
				//      objP->m_info.signature = object_next_signature++;
				//    verify_object(objP);
				}
			}
		}
#endif
	//===================== READ WALL INFO============================
	// note: Wall size will automatically strip last two items

#if 1
	if (0 > LoadGameItem (loadfile, GameInfo ().walls, Walls (0), 20, MAX_WALLS, "Walls"))
		return -1;
#else
	if ((GameInfo ().walls.offset > -1) && !fseek(loadfile, GameInfo ().walls.offset, SEEK_SET)) {
		if (GameInfo ().walls.count > MAX_WALLS) {
			sprintf_s (message, sizeof (message),  "Error: Max number of walls (%d/%d) exceeded", GameInfo ().walls.count, MAX_WALLS);
			ErrorMsg (message);
			GameInfo ().walls.count = MAX_WALLS;
			}
		else if (GameInfo ().fileinfo.version < 20)
			ErrorMsg ("Wall version < 20, walls not loaded");
		else if (GameInfo ().walls.count) {
			for (i = 0; i < GameInfo ().walls.count; i++) {
				if (!Walls (i)->Read (loadfile, GameInfo ().fileinfo.version)) {
					ErrorMsg ("Error reading walls from mine.cpp");
					break;
					}
				}
			}
		}
#endif
	//===================== READ DOOR INFO============================
	// note: not used for D1 or D2 since doors.count is always 0
#if 1
	if (0 > LoadGameItem (loadfile, GameInfo ().doors, ActiveDoors (0), 20, MAX_DOORS, "Doors"))
		return -1;
#else
	if ((GameInfo ().doors.offset > -1) && !fseek(loadfile, GameInfo ().doors.offset, SEEK_SET)) {
		if (GameInfo ().doors.count > MAX_DOORS) {
			sprintf_s (message, sizeof (message),  "Error: Max number of doors (%ld/%d) exceeded", GameInfo ().doors.count, MAX_DOORS);
			ErrorMsg (message);
			GameInfo ().doors.count = MAX_DOORS;
			}
		else if (GameInfo ().fileinfo.version < 20)
			ErrorMsg ("Door version < 20, doors not loaded");
		else if (sizeof (*ActiveDoors (i)) != GameInfo ().doors.size)
			ErrorMsg ("Error: Door size incorrect");
		else if (GameInfo ().doors.count) {
			for (i = 0; i < GameInfo ().doors.count; i++) {
				if (!ActiveDoors ()->Read (loadfile, GameInfo ().fileinfo.version))
					ErrorMsg ("Error reading doors.");
				}
			}
		}
#endif
	//==================== READ TRIGGER INFO==========================
	// note: order different for D2 levels but size is the same
#if 1
	if (0 > LoadGameItem (loadfile, GameInfo ().triggers, Triggers (0), -1, MAX_TRIGGERS, "Triggers"))
		return -1;
	if (GameInfo ().triggers.offset > -1) {
#else
	if (GameInfo ().triggers.offset > -1) {
		if (GameInfo ().triggers.count > MAX_TRIGGERS) {
			sprintf_s (message, sizeof (message),  "Error: Max number of triggers (%ld/%d) exceeded",
				GameInfo ().triggers.count, MAX_TRIGGERS);
			ErrorMsg (message);
			GameInfo ().triggers.count = MAX_TRIGGERS;
			}
		if (!fseek(loadfile, GameInfo ().triggers.offset, SEEK_SET)) 
			for (i = 0; i < GameInfo ().triggers.count; i++)
				Triggers (i)->Read (loadfile, GameInfo ().fileinfo.version, false);
		}
#endif
		INT32 bObjTriggersOk = 1;
		if (GameInfo ().fileinfo.version >= 33) {
			INT32 i = ftell (loadfile);
			if (fread (&NumObjTriggers (), sizeof (INT32), 1, loadfile) != 1) {
				ErrorMsg ("Error reading object triggers from mine.");
				bObjTriggersOk = 0;
				}
			else {
				for (i = 0; i < NumObjTriggers (); i++)
					ObjTriggers (i)->Read (loadfile, GameInfo ().fileinfo.version, true);
				if (GameInfo ().fileinfo.version >= 40) {
					for (i = 0; i < NumObjTriggers (); i++)
						ObjTriggers (i)->m_info.nObject = read_INT16 (loadfile);
					}
				else {
					for (i = 0; i < NumObjTriggers (); i++) {
						read_INT16 (loadfile);
						read_INT16 (loadfile);
						ObjTriggers (i)->m_info.nObject = read_INT16 (loadfile);
						}
					if (GameInfo ().fileinfo.version < 36)
						fseek (loadfile, 700 * sizeof (INT16), SEEK_CUR);
					else
						fseek (loadfile, 2 * sizeof (INT16) * read_INT16 (loadfile), SEEK_CUR);
					}
				}
			}
		if (bObjTriggersOk && NumObjTriggers ())
			SortObjTriggers ();
		else {
			NumObjTriggers () = 0;
			CLEAR (ObjTriggers ());
			}
		}

	//================ READ CONTROL CENTER TRIGGER INFO============== =
	// note: same for D1 and D2
#if 1
	if (0 > LoadGameItem (loadfile, GameInfo ().control, ReactorTriggers (0), -1, MAX_REACTOR_TRIGGERS, "Reactor triggers"))
		return -1;
#else
	if (GameInfo ().control.offset > -1) {
		if (GameInfo ().control.count > MAX_REACTOR_TRIGGERS) {
			sprintf_s (message, sizeof (message),  "Error: Max number of control center Triggers () (%ld, %d) exceeded",
				GameInfo ().control.count, MAX_REACTOR_TRIGGERS);
			ErrorMsg (message);
			GameInfo ().control.count = MAX_REACTOR_TRIGGERS;
		}
		if (!fseek(loadfile, GameInfo ().control.offset, SEEK_SET))  {
			for (i = 0; i < GameInfo ().control.count; i++)
				if (!ReactorTriggers (i)->Read (loadfile, GameInfo ().fileinfo.version)) {
					ErrorMsg ("Error reading control center triggers from mine.cpp");
				}
			}
		}
#endif
	//================ READ MATERIALIZATION CENTER INFO============== =
	// note: added robot_flags2 for Descent 2
#if 1
	if (0 > LoadGameItem (loadfile, GameInfo ().botgen, BotGens (0), -1, MAX_ROBOT_MAKERS, "Robot makers"))
		return -1;
#else
	if (GameInfo ().botgen.offset > -1) {
		if (GameInfo ().botgen.count > MAX_ROBOT_MAKERS) {
			sprintf_s (message, sizeof (message),  "Error: Max number of robot centers (%ld/%d) exceeded",
				GameInfo ().botgen.count, MAX_ROBOT_MAKERS);
			ErrorMsg (message);
			GameInfo ().botgen.count = MAX_ROBOT_MAKERS;
		}
		if (!fseek(loadfile, GameInfo ().botgen.offset, SEEK_SET))  {
			for (i = 0; i < GameInfo ().botgen.count; i++) {
				if (!BotGens (i)->Read (loadfile, GameInfo ().fileinfo.version)) {
					ErrorMsg ("Error reading botgens from mine.cpp");
					break;
					}
				}
			}
		}
#endif
	//================ READ EQUIPMENT CENTER INFO============== =
	// note: added robot_flags2 for Descent 2
#if 1
	if (0 > LoadGameItem (loadfile, GameInfo ().equipgen, EquipGens (0), -1, MAX_ROBOT_MAKERS, "Equipment makers"))
		return -1;
#else
	if (GameInfo ().equipgen.offset > -1) {
		if (GameInfo ().equipgen.count > MAX_ROBOT_MAKERS) {
			sprintf_s (message, sizeof (message),  "Error: Max number of robot centers (%ld/%d) exceeded",
				GameInfo ().equipgen.count, MAX_ROBOT_MAKERS);
			ErrorMsg (message);
			GameInfo ().equipgen.count = MAX_ROBOT_MAKERS;
		}
		if (!fseek(loadfile, GameInfo ().equipgen.offset, SEEK_SET))  {
			for (i = 0; i < GameInfo ().equipgen.count; i++) {
				if (IsD2File ()) {
					if (!EquipGens (i)->Read (loadfile, GameInfo ().fileinfo.version)) {
						ErrorMsg ("Error reading equipgens from mine.cpp");
						break;
						}
					}
				}
			}
		}
#endif
	//================ READ DELTA LIGHT INFO============== =
	// note: D2 only
	if (IsD2File ()) {
		//    sprintf_s (message, sizeof (message),  "Number of delta light indices = %ld", GameInfo ().lightDeltaIndices.count);
		//    DEBUGMSG(message);
#if 1
		if (0 > LoadGameItem (loadfile, GameInfo ().lightDeltaIndices, LightDeltaIndex (0), -1, MAX_LIGHT_DELTA_INDICES, "Light delta indices", (LevelVersion () >= 15) && (GameInfo ().fileinfo.version >= 34)))
			return -1;
#else
		if (GameInfo ().lightDeltaIndices.offset > -1 && GameInfo ().lightDeltaIndices.count > 0) {
			if (GameInfo ().lightDeltaIndices.count > MAX_LIGHT_DELTA_INDICES) {
				sprintf_s (message, sizeof (message),  "Error: Max number of delta light indices (%ld/%d) exceeded",
					GameInfo ().lightDeltaIndices.count, MAX_LIGHT_DELTA_INDICES);
				ErrorMsg (message);
				GameInfo ().lightDeltaIndices.count = MAX_LIGHT_DELTA_INDICES;
				}
			else if (!fseek(loadfile, GameInfo ().lightDeltaIndices.offset, SEEK_SET)) {
				bool bD2X = (LevelVersion () >= 15) && (GameInfo ().fileinfo.version >= 34);
				for (i = 0; i < GameInfo ().lightDeltaIndices.count; i++) {
					if (!LightDeltaIndex (i)->Read (loadfile, 0, bD2X)) 
					ErrorMsg ("Error reading delta light indices from mine.cpp");
					}
				}
			}
#endif
		}
	//==================== READ DELTA LIGHTS==================== =
	// note: D2 only
	if (IsD2File ()) {
		//    sprintf_s (message, sizeof (message),  "Number of delta light values = %ld", GameInfo ().lightDeltaValues.count);
		//    DEBUGMSG(message);
#if 1
		if (0 > LoadGameItem (loadfile, GameInfo ().lightDeltaValues, LightDeltaValues (0), -1, MAX_LIGHT_DELTA_VALUES, "Light delta values"))
			return -1;
#else
		if (GameInfo ().lightDeltaValues.offset > -1 && GameInfo ().lightDeltaIndices.count > 0) {
			if (GameInfo ().lightDeltaValues.count > MAX_LIGHT_DELTA_VALUES) {
				sprintf_s (message, sizeof (message),  "Error: Max number of delta light values (%ld/%d) exceeded",
					GameInfo ().lightDeltaValues.count, MAX_LIGHT_DELTA_VALUES);
				ErrorMsg (message);
				GameInfo ().lightDeltaValues.count = MAX_LIGHT_DELTA_VALUES;
				}
			else if (!fseek(loadfile, GameInfo ().lightDeltaValues.offset, SEEK_SET)) {
				for (i = 0; i < GameInfo ().lightDeltaValues.count; i++) {
					if (!LightDeltaValues (i)->Read (loadfile)) {
						ErrorMsg ("Error reading delta light values from mine.cpp");
						break;
					}
					memcpy(dl, &temp_dl, (INT32)(GameInfo ().lightDeltaValues.size));
					dl++;
					}
				}
			}
#endif
		}

return 0;
}

// --------------------------------------------------------------------------
//eof mine.cpp