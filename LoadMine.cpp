// Copyright (c) 1998 Bryan Aamot, Brainware

#include "mine.h"
#include "CustomTextures.h"
#include "dle-xp.h"

// -----------------------------------------------------------------------------

short CMine::Load (const char *szFile, bool bLoadFromHog)
{
if (theMine == null)
	return 0;

	char filename [256];
	bool bNewMine = false;
	CFileManager fp;

undoManager.Reset ();
tunnelMaker.Destroy ();
// if no file passed, define a new level w/ 1 object
if (szFile && *szFile)
	strcpy_s (filename, sizeof (filename), szFile);
else if (!CreateNewLevel ()) {
	lightManager.CreateLightMap ();
	CFileManager::SplitPath ((m_fileType== RDL_FILE) ? descentPath [0] : missionPath, m_startFolder , null, null);
	sprintf_s (filename, sizeof (filename), (m_fileType== RDL_FILE) ? "%sNEW.RDL" : "%sNEW.RL2", m_startFolder );
	bLoadFromHog = false;
	bNewMine = true;
	}

m_disableDrawing = TRUE;

undoManager.Enable (false);
LoadMine (filename, bLoadFromHog, bNewMine);
if (!bNewMine && IsD2XLevel () && LevelIsOutdated ()) {
	if (LevelVersion () < 15) {
		segmentManager.UpdateWalls (MAX_WALLS_D2 + 1, WALL_LIMIT + 1);
		triggerManager.ObjTriggerCount () = 0;
		}
	UpdateLevelVersion ();
	}
//ComputeVariableLight ();
int errFlags = FixIndexValues ();
if (errFlags != 0) {
	sprintf_s (message, sizeof (message),  "File contains corrupted data (error code %#04x). Would you like to load anyway? ", errFlags);
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
		undoManager.Enable (true);
		return 1;
		}
	}
m_disableDrawing = FALSE;
undoManager.Enable (true);
return 0;
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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
		if (descentPath [1][0] != 0) {
			char *path = strrchr (descentPath [1], '\\');
			if (!path) {
				descentPath [1][0] = null;
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

// -----------------------------------------------------------------------------

short CMine::LoadMine (char *filename, bool bLoadFromHog, bool bNewMine)
{
	byte* palette = 0;

	CFileManager fp;
	int	sig = 0;
	int	mineDataOffset = 0, gameDataOffset = 0;
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
mineDataOffset = fp.ReadInt32 ();
// read game data offset
gameDataOffset = fp.ReadInt32 ();
LoadPaletteName (fp, bNewMine);

// read descent 2 reactor information
if (IsD2File ()) {
	ReactorTime () = fp.ReadInt32 (); // base control center explosion time
	ReactorStrength () = fp.ReadInt32 (); // reactor strength
	lightManager.ReadVariableLights (fp);
	// read secret cube number
	SecretSegment () = fp.ReadInt32 ();
	// read secret cube orientation?
	fp.Read (SecretOrient ());
	}

m_disableDrawing = TRUE;

fp.Seek (mineDataOffset, SEEK_SET);
mineErr = LoadMineGeometry (fp, bNewMine);
int fPos = fp.Tell ();

if (mineErr != 0) {
	ErrorMsg ("Error loading mine data");
	fp.Close ();
	return(2);
}

fp.Seek (gameDataOffset, SEEK_SET);
gameErr = LoadGameItems (fp, bNewMine);

if (gameErr != 0) {
	ErrorMsg ("Error loading game data");
	// reset "howmany"
	objectManager.ResetInfo ();
	wallManager.ResetInfo ();
	triggerManager.ResetInfo ();
	segmentManager.ResetInfo ();
	lightManager.ResetInfo ();
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
		robotManager.ReadHAM ();
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
					m_bVertigo = robotManager.ReadHAM (szHamFile, EXTENDED_HAM) == 0;
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
			robotManager.ReadHXM (fp, -1);
			fp.Close ();
			}
		}
	}
objectManager.Sort ();
return return_code;
}

// -----------------------------------------------------------------------------
// LoadMineGeometry()
//
// ACTION - Reads a mine data portion of RDL file.
// -----------------------------------------------------------------------------

short CMine::LoadMineGeometry (CFileManager& fp, bool bNewMine)
{

// read version (1 byte)
byte version = fp.ReadByte ();

// read number of vertices (2 bytes)
ushort nVertices = fp.ReadUInt16 ();
if (nVertices > VERTEX_LIMIT) {
	sprintf_s (message, sizeof (message),  "Too many vertices (%d)", nVertices);
	ErrorMsg (message);
	return(1);
	}
if (IsD1File () ? nVertices > MAX_VERTICES_D1 : IsStdLevel () && (nVertices > MAX_VERTICES_D2))
	ErrorMsg ("Warning: Too many vertices for this level version");

// read number of Segments () (2 bytes)
ushort nSegments = fp.ReadUInt16 ();
if (nSegments > SEGMENT_LIMIT) {
	sprintf_s (message, sizeof (message), "Too many Segments (%d)", nSegments);
	ErrorMsg (message);
	return 2;
	}
if (IsD1File () ? nVertices > MAX_SEGMENTS_D1 : IsStdLevel () && (nVertices > MAX_SEGMENTS_D2))
	ErrorMsg ("Warning: Too many Segments for this level version");

objectManager.ResetInfo ();
wallManager.ResetInfo ();
triggerManager.ResetInfo ();
segmentManager.ResetInfo ();
lightManager.ResetInfo ();

vertexManager.Count () = nVertices;
vertexManager.FileOffset () = fp.Tell ();
vertexManager.Read (fp, FileInfo ().version);

segmentManager.Count () = nSegments;
segmentManager.FileOffset () = fp.Tell ();
segmentManager.ReadSegments (fp, FileInfo ().version);

lightManager.ReadColors (fp);

if (objectManager.Count () > MAX_OBJECTS) {
	sprintf_s (message, sizeof (message),  "Warning: Max number of objects for this level version exceeded (%ld/%d)", 
			     objectManager.Count (), MAX_OBJECTS);
	ErrorMsg (message);
	}
return 0;
}

// -----------------------------------------------------------------------------
// LoadGameItems()
//
// ACTION - Loads the player, object, wall, door, trigger, and
//          materialogrifizationator data from an RDL file.
// -----------------------------------------------------------------------------

short CMine::LoadGameItems (CFileManager& fp, bool bNewMine) 
{
int startOffset = fp.Tell ();

// Read in FileInfo () to get size of saved fileinfo.
if (fp.Seek (startOffset, SEEK_SET)) {
	ErrorMsg ("Error seeking in mine.cpp");
	return -1;
	}
// Check signature
Info ().Read (fp);
if (FileInfo ().signature != 0x6705) {
	ErrorMsg ("Game data signature incorrect");
	return -1;
	}
if (Info ().fileInfo.version < 14) 
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

objectManager.Read (fp, FileInfo ().version);
wallManager.Read (fp, FileInfo ().version);
triggerManager.Read (fp, FileInfo ().version);
segmentManager.ReadRobotMakers (fp, FileInfo ().version);
lightManager.ReadLightDeltas (fp, FileInfo ().version);
segmentManager.ReadEquipMakers (fp, FileInfo ().version);
return 0;
}

// -------------------------------------------------------------------------------
//eof mine.cpp