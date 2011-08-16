using System.IO;
namespace DLE.NET
{
    public partial class GameMine
    {
    // -----------------------------------------------------------------------------

    //short LoadLevel (FileStream fs, BinaryReader fp, bool bLoadFromHog)
    //{
    //    CMemoryFile mf;
    //    FileManager df;
    //    bool bCreate = false;

    //if (fp == null) {
    //    if (CreateNewLevel (mf)) 
    //        fp = &mf;
    //    else {
    //        DLE.Lights.CreateLightMap ();
    //        CFileManager::SplitPath (IsD1File () ? descentPath [0] : missionPath, m_startFolder, null, null);
    //        char filename [256];
    //        sprintf_s (filename, sizeof (filename), IsD1File () ? "%new.rdl" : "%snew.rl2", m_startFolder);
    //        if (df.Open (filename, "rb")) {
    //            sprintf_s (message, sizeof (message),  "Error %d: Can't open file \"%s\".", GetLastError (), filename);
    //            ErrorMsg (message);
    //            return -1;
    //            }
    //        fp = &df;
    //        bLoadFromHog = false;
    //        bCreate = true;
    //        }
    //    }
    //DLE.Backup.Lock ();
    //LoadMine (fp, bLoadFromHog, bCreate);
    //DLE.Backup.Unlock ();
    //return bCreate ? 1 : 0;
    //}

    //// -----------------------------------------------------------------------------

    //short Load (char[] filename)
    //{
    //CMemoryFile	fp;
    //return fp.Open (filename, "rb") ? Load (null, false) : Load (&fp, false);
    //}

    //// -----------------------------------------------------------------------------

    //short Load (FileStream fs, BinaryReader fp, bool bLoadFromHog)
    //{
    //    bool bCreate = false;

    //DLE.Backup.Reset ();
    //tunnelMaker.Destroy ();
    //// if no file passed, define a new level w/ 1 object
    //short i = LoadLevel (fs, fp, bLoadFromHog);
    //if (i != 0)
    //    return i < 1;

    //if (LevelIsOutdated ()) {
    //    DLE.Backup.Lock ();
    //    if (LevelVersion () < 15) {
    //        DLE.Segments.UpdateWalls (MAX_WALLS_D2 + 1, WALL_LIMIT + 1);
    //        DLE.Triggers.ObjTriggerCount () = 0;
    //        }
    //    UpdateLevelVersion ();
    //    DLE.Backup.Unlock ();
    //    }

    //int errFlags = FixIndexValues ();
    //if (errFlags == 0)
    //    return 1;
    //sprintf_s (message, sizeof (message),  "File contains corrupted data (error code %#04x). Would you like to load anyway? ", errFlags);
    //if (QueryMsg (message) != IDYES) {
    //    return LoadLevel (null, false) < 1;
    //    }

    //return 0; // failed
    //}

    //// -----------------------------------------------------------------------------

    //short LoadMineSigAndType (CFileManager* fp)
    //{
    //int sig = fp.ReadInt32 ();
    //if (sig != 'P'*0x1000000L + 'L'*0x10000L + 'V'*0x100 + 'L') {
    //    ErrorMsg ("Signature value incorrect.");
    //    fp.Close ();
    //    return 1;
    //    }

    //// read version
    //SetLevelVersion (fp.ReadInt32 ());
    //if (LevelVersion () == 1) {
    //    SetFileType (RDL_FILE);
    //    }
    //else if ((LevelVersion () >= 6L) && (LevelVersion () <= 21L)) {
    //    SetFileType (RL2_FILE);
    //    }
    //else {
    //    sprintf_s (message, sizeof (message),  "Version %d unknown. Cannot load this level.", LevelVersion ());
    //    ErrorMsg (message);
    //    fp.Close ();
    //    return 1;
    //    }
    //return 0;
    //}

    //// -----------------------------------------------------------------------------

    //void LoadPaletteName (CFileManager* fp, bool bCreate)
    //{
    //if (IsD2File ()) {
    //    if (LevelVersion () >= 8) {
    //        fp.ReadInt16 ();
    //        fp.ReadInt16 ();
    //        fp.ReadInt16 ();
    //        fp.ReadSByte ();
    //        }
    //    // read palette file name
    //    DLE.Palettes.LoadName (fp);
    //    // try to find new pig file in same directory as Current () pig file
    //    // 1) cut off old name
    //    if (!bCreate) {
    //        if (descentPath [1][0] != 0) {
    //            char *path = strrchr (descentPath [1], '\\');
    //            if (!path) {
    //                descentPath [1][0] = null;
    //                } 
    //            else {
    //                path++;  // leave slash
    //                *path = null;
    //                }
    //            // paste on new *.pig name
    //            strcat_s (descentPath [1], sizeof (descentPath [1]), DLE.Palettes.Name ());
    //            _strlwr_s (descentPath [1], sizeof (descentPath [1]));
    //            }
    //        }
    //    }
    //else
    //    DLE.Palettes.SetName ("descent.pig");
    //}

    //// -----------------------------------------------------------------------------

    //short LoadMine (CFileManager* fp, bool bLoadFromHog, bool bCreate)
    //{
    //m_changesMade = 0;

    ////	CFileManager fp;
    ////if (fp.Open (filename, "rb")) {
    ////	sprintf_s (message, sizeof (message),  "Error %d: Can't open file \"%s\".", GetLastError (), filename);
    ////	ErrorMsg (message);
    ////	return -1;
    ////	}

    //if (LoadMineSigAndType (fp))
    //    return -1;
    //ClearMineData ();
    //// read mine data offset
    //int mineDataOffset = fp.ReadInt32 ();
    //// read game data offset
    //int gameDataOffset = fp.ReadInt32 ();
    //LoadPaletteName (fp, bCreate);

    //// read descent 2 reactor information
    //if (IsD2File ()) {
    //    ReactorTime () = fp.ReadInt32 (); // base control center explosion time
    //    ReactorStrength () = fp.ReadInt32 (); // reactor strength
    //    DLE.Lights.ReadVariableLights (fp);
    //    // read secret segment number
    //    SecretSegment () = fp.ReadInt32 ();
    //    // read secret segment orientation?
    //    fp.Read (SecretOrient ());
    //    }

    //fp.Seek (mineDataOffset, SEEK_SET);
    //if (LoadMineGeometry (fp, bCreate) != 0) {
    //    ErrorMsg ("Error loading mine data");
    //    fp.Close ();
    //    return(2);
    //    }

    //fp.Seek (gameDataOffset, SEEK_SET);
    //if (LoadGameItems (fp, bCreate) != 0) {
    //    ErrorMsg ("Error loading game data");
    //    // reset "howmany"
    //    DLE.Objects.ResetInfo ();
    //    DLE.Walls.ResetInfo ();
    //    DLE.Triggers.ResetInfo ();
    //    DLE.Segments.ResetInfo ();
    //    DLE.Lights.ResetInfo ();
    //    fp.Close ();
    //    return 3;
    //    }

    //if (!(bLoadFromHog || bCreate)) {
    //    DLE.Palettes.Reload ();
    //    DLE.Textures.Reload (DLE.Textures.Version ());
    //    if (IsD2File ()) {
    //        char filename [256];
    //        strcpy_s (filename, sizeof (filename), fp.Name ());
    //        char* ps = strstr (filename, ".");
    //        if (ps)
    //            strcpy_s (ps, 256 - (ps - filename), ".pog");
    //        else
    //            strcat_s (filename, 256, ".pog");
    //        if (!fp.Open (filename, "rb")) {
    //            DLE.Textures.ReadPog (*fp, fp.Size ());
    //            fp.Close ();
    //            }
    //        robotManager.ReadHAM (null);
    //        if (IsD2File ()) {
    //            char szHogFile [256], szHamFile [256], *p;
    //            long nSize, nOffset;
    //            CFileManager hfp;

    //            CFileManager::SplitPath (descentPath [1], szHogFile, null, null);
    //            if (p = strstr (szHogFile, "data"))
    //                *p = '\0';
    //            strcat_s (szHogFile, sizeof (szHogFile), "missions\\d2x.hog");
    //            if (FindFileData (szHogFile, "d2x.ham", &nSize, &nOffset, FALSE)) {
    //                CFileManager::SplitPath (descentPath [1], szHamFile, null, null);
    //                if (p = strstr (szHamFile, "data"))
    //                    *p = '\0';
    //                strcat_s (szHamFile, sizeof (szHamFile), "missions\\d2x.ham");
    //                if (hfp.Open (szHogFile, "rb"))
    //                    ErrorMsg ("Could not open HOG file.");
    //                else {
    //                    if (0 < hfp.Seek (nOffset + sizeof (struct level_header), SEEK_SET))
    //                        m_bVertigo = robotManager.ReadHAM (&hfp, EXTENDED_HAM) == 0;
    //                    hfp.Close ();
    //                    }
    //                }
    //            }
    //        ps = strstr (filename, ".");
    //        if (ps)
    //            strcpy_s (filename, 256 - (ps - filename), ".hxm");
    //        else
    //            strcat_s (filename, 256, ".hxm");
    //        if (!fp.Open (filename, "rb")) {
    //            robotManager.ReadHXM (*fp, -1);
    //            fp.Close ();
    //            }
    //        }
    //    }
    //DLE.Objects.Sort ();
    //DLE.MainFrame ().SetSelectMode (eSelectSide);
    //current.Reset ();
    //other.Reset ();
    //return 0;
    //}

    //// -----------------------------------------------------------------------------
    //// LoadMineGeometry()
    ////
    //// ACTION - Reads a mine data portion of RDL file.
    //// -----------------------------------------------------------------------------

    //short LoadMineGeometry (CFileManager* fp, bool bCreate)
    //{
    //// read version (1 byte)
    //byte version = fp.ReadByte ();

    //// read number of vertices (2 bytes)
    //ushort nVertices = fp.ReadUInt16 ();
    //if (nVertices > VERTEX_LIMIT) {
    //    sprintf_s (message, sizeof (message),  "Too many vertices (%d)", nVertices);
    //    ErrorMsg (message);
    //    return(1);
    //    }
    //if (IsD1File () ? nVertices > MAX_VERTICES_D1 : IsStdLevel () && (nVertices > MAX_VERTICES_D2))
    //    ErrorMsg ("Warning: Too many vertices for this level version");

    //// read number of Segments () (2 bytes)
    //ushort nSegments = fp.ReadUInt16 ();
    //if (nSegments > SEGMENT_LIMIT) {
    //    sprintf_s (message, sizeof (message), "Too many Segments (%d)", nSegments);
    //    ErrorMsg (message);
    //    return 2;
    //    }
    //if (IsD1File () ? nSegments > MAX_SEGMENTS_D1 : IsStdLevel () && (nSegments > MAX_SEGMENTS_D2))
    //    ErrorMsg ("Warning: Too many Segments for this level version");

    //DLE.Objects.ResetInfo ();
    //DLE.Walls.ResetInfo ();
    //DLE.Triggers.ResetInfo ();
    //DLE.Segments.ResetInfo ();
    //DLE.Lights.ResetInfo ();

    //vertexManager.Count = nVertices;
    //vertexManager.FileOffset () = fp.Tell ();
    //vertexManager.Read (fp);

    //DLE.Segments.Count = nSegments;
    //DLE.Segments.FileOffset () = fp.Tell ();
    //DLE.Segments.ReadSegments (fp);

    //DLE.Lights.ReadColors (*fp);

    //if (DLE.Objects.Count > MAX_OBJECTS) {
    //    sprintf_s (message, sizeof (message),  "Warning: Max number of objects for this level version exceeded (%ld/%d)", 
    //                 DLE.Objects.Count, MAX_OBJECTS);
    //    ErrorMsg (message);
    //    }
    //return 0;
    //}

    //// -----------------------------------------------------------------------------
    //// LoadGameItems()
    ////
    //// ACTION - Loads the player, object, wall, door, trigger, and
    ////          materialogrifizationator data from an RDL file.
    //// -----------------------------------------------------------------------------

    //short LoadGameItems (CFileManager* fp, bool bCreate) 
    //{
    //// Check signature
    //Info ().Read (fp);
    //if (FileInfo ().signature != 0x6705) {
    //    ErrorMsg ("Game data signature incorrect");
    //    return -1;
    //    }
    //if (Info ().fileInfo.version < 14) 
    //    m_currentLevelName [0] = 0;
    //else {  /*load mine filename */
    //    for (char *p = m_currentLevelName; ; p++) {
    //        *p = fp.ReadChar ();
    //        if (*p== '\n')
    //            *p = 0;
    //        if (*p == 0)
    //            break;
    //        }
    //    }

    //DLE.Objects.Read (fp);
    //DLE.Walls.Read (fp);
    //DLE.Triggers.Read (fp);
    //DLE.Triggers.ReadReactor (fp);
    //DLE.Segments.ReadRobotMakers (fp);
    //DLE.Lights.ReadLightDeltas (fp);
    //if (!DLE.IsD1File ())
    //    DLE.Segments.ReadEquipMakers (fp);
    //return 0;
    //}

    // -------------------------------------------------------------------------------
    }
}
