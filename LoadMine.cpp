// Copyright (c) 1998 Bryan Aamot, Brainware

#include "mine.h"
#include "CustomTextures.h"
#include "dle-xp.h"

// -----------------------------------------------------------------------------

short CMine::LoadLevel (CFileManager* fp, bool bLoadFromHog)
{
	CMemoryFile mf;
	CFileManager df;
	bool bCreate = false;

if (fp == null) {
	if (CreateNewLevel (mf)) 
		fp = &mf;
	else {
		lightManager.CreateLightMap ();
		CFileManager::SplitPath (IsD1File () ? descentPath [0] : missionPath, m_startFolder, null, null);
		char filename [256];
		sprintf_s (filename, sizeof (filename), IsD1File () ? "%new.rdl" : "%snew.rl2", m_startFolder);
		if (df.Open (filename, "rb")) {
			sprintf_s (message, sizeof (message),  "Error %d: Can't open file \"%s\".", GetLastError (), filename);
			ErrorMsg (message);
			return -1;
			}
		fp = &df;
		bLoadFromHog = false;
		bCreate = true;
		}
	}
undoManager.Lock ();
LoadMine (*fp, bLoadFromHog, bCreate);
undoManager.Unlock ();
return bCreate ? 1 : 0;
}

// -----------------------------------------------------------------------------

short CMine::Load (const char* filename)
{
CMemoryFile	fp;
return fp.Open (filename, "rb") ? Load (null, false) : Load (&fp, false);
}

// -----------------------------------------------------------------------------

short CMine::Load (CFileManager* fp, bool bLoadFromHog)
{
	bool bCreate = false;

undoManager.Reset ();
tunnelMaker.Destroy ();
// if no file passed, define a new level w/ 1 object
short i = LoadLevel (fp, bLoadFromHog);
if (i != 0)
	return i < 1;

if (LevelIsOutdated ()) {
	undoManager.Lock ();
	if (LevelVersion () < 15) {
		segmentManager.UpdateWalls (MAX_WALLS_D2 + 1, WALL_LIMIT + 1);
		triggerManager.ObjTriggerCount () = 0;
		}
	UpdateLevelVersion ();
	undoManager.Unlock ();
	}

int errFlags = FixIndexValues ();
if (errFlags == 0)
	return 1;
sprintf_s (message, sizeof (message),  "File contains corrupted data (error code %#04x). Would you like to load anyway? ", errFlags);
if (QueryMsg (message) != IDYES) {
	return LoadLevel (null, false) < 1;
	}

return 0; // failed
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

void CMine::LoadPaletteName (CFileManager& fp, bool bCreate)
{
if (IsD2File ()) {
	if (LevelVersion () >= 8) {
		fp.ReadInt16 ();
		fp.ReadInt16 ();
		fp.ReadInt16 ();
		fp.ReadSByte ();
		}
	// read palette file name
	paletteManager.LoadName (fp);
	// try to find new pig file in same directory as Current () pig file
	// 1) cut off old name
	if (!bCreate) {
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
			strcat_s (descentPath [1], sizeof (descentPath [1]), paletteManager.Name ());
			_strlwr_s (descentPath [1], sizeof (descentPath [1]));
			}
		}
	}
}

// -----------------------------------------------------------------------------

short CMine::LoadMine (CFileManager& fp, bool bLoadFromHog, bool bCreate)
{
m_changesMade = 0;

//	CFileManager fp;
//if (fp.Open (filename, "rb")) {
//	sprintf_s (message, sizeof (message),  "Error %d: Can't open file \"%s\".", GetLastError (), filename);
//	ErrorMsg (message);
//	return -1;
//	}

if (LoadMineSigAndType (fp))
	return -1;
ClearMineData ();
// read mine data offset
int mineDataOffset = fp.ReadInt32 ();
// read game data offset
int gameDataOffset = fp.ReadInt32 ();
LoadPaletteName (fp, bCreate);

// read descent 2 reactor information
if (IsD2File ()) {
	ReactorTime () = fp.ReadInt32 (); // base control center explosion time
	ReactorStrength () = fp.ReadInt32 (); // reactor strength
	lightManager.ReadVariableLights (fp);
	// read secret segment number
	SecretSegment () = fp.ReadInt32 ();
	// read secret segment orientation?
	fp.Read (SecretOrient ());
	}

fp.Seek (mineDataOffset, SEEK_SET);
if (LoadMineGeometry (fp, bCreate) != 0) {
	ErrorMsg ("Error loading mine data");
	fp.Close ();
	return(2);
	}

fp.Seek (gameDataOffset, SEEK_SET);
if (LoadGameItems (fp, bCreate) != 0) {
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

if (!(bLoadFromHog || bCreate)) {
	paletteManager.Reload ();
	textureManager.LoadTextures ();
	if (IsD2File ()) {
		char filename [256];
		strcpy_s (filename, sizeof (filename), fp.Name ());
		char* ps = strstr (filename, ".");
		if (ps)
			strcpy_s (ps, 256 - (ps - filename), ".pog");
		else
			strcat_s (filename, 256, ".pog");
		if (!fp.Open (filename, "rb")) {
			ReadPog (fp, fp.Size ());
			fp.Close ();
			}
		CFileManager fp;
		robotManager.ReadHAM (fp);
		if (IsD2File ()) {
			char szHogFile [256], szHamFile [256], *p;
			long nSize, nOffset;

			CFileManager::SplitPath (descentPath [1], szHogFile, null, null);
			if (p = strstr (szHogFile, "data"))
				*p = '\0';
			strcat_s (szHogFile, sizeof (szHogFile), "missions\\d2x.hog");
			if (FindFileData (szHogFile, "d2x.ham", &nSize, &nOffset, FALSE)) {
				CFileManager::SplitPath (descentPath [1], szHamFile, null, null);
				if (p = strstr (szHamFile, "data"))
					*p = '\0';
				strcat_s (szHamFile, sizeof (szHamFile), "missions\\d2x.ham");
				if (fp.Open (szHogFile, "rb"))
					ErrorMsg ("Could not open HOG file.");
				else {
					if (0 < fp.Seek (nOffset + sizeof (struct level_header), SEEK_SET))
						m_bVertigo = robotManager.ReadHAM (fp, EXTENDED_HAM) == 0;
					fp.Close ();
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
return 0;
}

// -----------------------------------------------------------------------------
// LoadMineGeometry()
//
// ACTION - Reads a mine data portion of RDL file.
// -----------------------------------------------------------------------------

short CMine::LoadMineGeometry (CFileManager& fp, bool bCreate)
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

short CMine::LoadGameItems (CFileManager& fp, bool bCreate) 
{
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
triggerManager.ReadReactor (fp, FileInfo ().version);
segmentManager.ReadRobotMakers (fp, FileInfo ().version);
lightManager.ReadLightDeltas (fp, FileInfo ().version);
segmentManager.ReadEquipMakers (fp, FileInfo ().version);
return 0;
}

// -------------------------------------------------------------------------------
//eof mine.cpp