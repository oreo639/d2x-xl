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

//==========================================================================
// CMine - CMine
//==========================================================================

CMine::CMine()
{
	VertCount () = 0;
	SegCount () = 0;
	m_levelVersion = 7;
	m_fileType = RL2_FILE;
	m_dlcLogPalette = 0;
	m_currentPalette = NULL;
	FlickerLightCount () = 0;
	Current () = &Current1 ();
	*m_szBlockFile = '\0';
	GameInfo ().objects.count = 0;
	GameInfo ().walls.count = 0;
	GameInfo ().doors.count = 0;
	GameInfo ().triggers.count = 0;
	GameInfo ().control.count = 0;
	GameInfo ().botgen.count = 0;
	GameInfo ().equipgen.count = 0;
	GameInfo ().lightDeltaIndices.count = 0;
	GameInfo ().lightDeltaValues.count = 0;
	m_nNoLightDeltas = 2;
	m_lightRenderDepth = MAX_LIGHT_DEPTH;
	m_deltaLightRenderDepth = MAX_LIGHT_DEPTH;
	memset (RobotInfo (), 0, sizeof (ROBOT_INFO));
//	LoadPalette ();
	m_bSortObjects = TRUE;
	m_bVertigo = false;
	m_pHxmExtraData = NULL;
	m_nHxmExtraDataSize = 0;
	m_bUseTexColors = false;
	LoadDefaultLightAndColor ();
	
//	strcpy (descent2_path, "d:\\games\\descent\\d2\\");
}

//==========================================================================
// CMine - ~CMine
//==========================================================================

CMine::~CMine()
{
Default ();
}

//==========================================================================
// CMine - init_data()
//==========================================================================

void CMine::Reset ()
{
/*
  INT16 i;
	FIX maxx, maxy, maxz, minx, miny, minz;

	// set max and min points
	maxx = minx = maxy = miny = maxz = minz = 0;
	for (i = 0; i < VertCount (); i++) {
		maxx = max(maxx, vertices [i].x);
		minx = min(minx, vertices [i].x);
		maxy = max(maxy, vertices [i].y);
		miny = min(miny, vertices [i].y);
		maxz = max(maxz, vertices [i].z);
		minz = min(minz, vertices [i].z);
	}
	max_x = (UINT16)(maxx / F1_0);
	max_y = (UINT16)(maxy / F1_0);
	max_z = (UINT16)(maxz / F1_0);
	min_x = (UINT16)(minx / F1_0);
	min_y = (UINT16)(miny / F1_0);
	min_z = (UINT16)(minz / F1_0);
#if 0
	sprintf_s (message, sizeof (message),  "max(%d, %d, %d), min(%d, %d, %d)", max_x, max_y, max_z, min_x, min_y, min_z);
	DEBUGMSG(message);
#endif
	gx0 = 0;
	gy0 = 0;
	gz0 = 0;
	spinx = M_PI/4.f;
	spiny = M_PI/4.f;
	spinz = 0.0;
	movex = - (max_x + min_x)/2.f;
	movey = - (max_y + min_y)/2.f;
	movez = - (max_z + min_z)/2.f;
	INT32 factor;
	INT32 max_all;
	max_all = max(max(max_x - min_x, max_y - min_y), max_z - min_z)/20;
	if (max_all < 2)      factor = 14;
	else if (max_all < 4) factor = 10;
	else if (max_all < 8) factor = 8;
	else if (max_all < 12) factor = 5;
	else if (max_all < 16) factor = 3;
	else if (max_all < 32) factor = 2;
	else factor = 1;
	sizex = .1f * (double)pow(1.2, factor);
	sizey = sizex;
	sizez = sizex;
*/
	Current1 ().nSegment = DEFAULT_SEGMENT;
	Current1 ().nPoint = DEFAULT_POINT;
	Current1 ().nLine = DEFAULT_LINE;
	Current1 ().nSide = DEFAULT_SIDE;
	Current1 ().nObject = DEFAULT_OBJECT;
	Current2 ().nSegment = DEFAULT_SEGMENT;
	Current2 ().nPoint = DEFAULT_POINT;
	Current2 ().nLine = DEFAULT_LINE;
	Current2 ().nSide = DEFAULT_SIDE;
	Current2 ().nObject = DEFAULT_OBJECT;
	theApp.ResetUndoBuffer ();
}

void CMine::ConvertWallNum (UINT16 wNumOld, UINT16 wNumNew)
{
CSegment *segP = Segments ();
CSide *sideP;
INT32 i, j;

for (i = SegCount (); i; i--, segP++)
	for (j = 0, sideP = segP->sides; j < 6; j++, sideP++)
		if (sideP->nWall >= wNumOld)
			sideP->nWall = wNumNew;
}


//==========================================================================
// CMine - load()
//
// ACTION -  loads a level (.RDL) file from disk
//
//==========================================================================

INT16 CMine::Load (const char *filename_passed, bool bLoadFromHog)
{
char filename [256];
INT16 check_err;
bool bNewMine = false;

// first disable curve generator
m_bSplineActive = FALSE;

memset (LightColors (), 0, sizeof (MineData ().lightColors));
memset (VertexColors (), 0, sizeof (MineData ().sideColors));
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
check_err = FixIndexValues();
if (check_err != 0) {
	sprintf_s (message, sizeof (message),  "File contains corrupted data. Would you like to load anyway? Error Code %#04x", check_err);
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

	FILE* loadFile = 0;
	INT32 sig = 0;
	INT32 minedata_offset = 0;
	INT32 gamedata_offset = 0;
	INT32 mine_err, game_err = 0;
	INT32	return_code = 0;
	char	palette_name [256];
	char*	ps;
	INT16 nLights = 0;

m_changesMade = 0;
fopen_s (&loadFile, filename, "rb");

if (!loadFile) {
	sprintf_s (message, sizeof (message),  "Error %d: Can't open file \"%s\".", GetLastError (), filename);
	ErrorMsg (message);
	return -1;
	}
	//  strcpy(gamesave_current_filename, filename);
if (LoadMineSigAndType (loadFile))
	return -1;
// read mine data offset
minedata_offset = read_INT32 (loadFile);
// read game data offset
gamedata_offset = read_INT32 (loadFile);

// don't bother reading  hostagetext_offset since
// for Descent 1 files since we dont use it anyway
// hostagetext_offset = read_INT32(loadFile);

if (IsD2File ()) {
	if (LevelVersion () >= 8) {
		read_INT16(loadFile);
		read_INT16(loadFile);
		read_INT16(loadFile);
		read_INT8(loadFile);
		}
	}

// read palette name *.256
	if (IsD2File ()) {
		// read palette file name
		INT32 i;
		for (i = 0; i < 15; i++) {
			palette_name [i] = fgetc(loadFile);
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
			char *path = strrchr(descent2_path, '\\');
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

#if 1
if (return_code = LoadPalette ())
	goto load_end;
#else
// set palette
palette = PalettePtr ();
ASSERT(palette);
if (!palette) 
	goto load_end;

// redefine logical palette entries if memory for it is allocated
m_dlcLogPalette = (LPLOGPALETTE) malloc (sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * 256);
if (m_dlcLogPalette) {
	m_dlcLogPalette->palVersion = 0x300;
	m_dlcLogPalette->palNumEntries = 256;
	INT32 i;
#if 0
	for (i = 0; i < 256; ++i) {
		m_dlcLogPalette->palPalEntry [i].peRed = *palette++;
		m_dlcLogPalette->palPalEntry [i].peGreen = *palette++;
		m_dlcLogPalette->palPalEntry [i].peBlue = *palette++;
		m_dlcLogPalette->palPalEntry [i].peFlags = PC_RESERVED; palette++;
		}
#else
	for (i = 0; i < 256;++i) {
		m_dlcLogPalette->palPalEntry [i].peRed = palette [i*3 + 0] << 2;
		m_dlcLogPalette->palPalEntry [i].peGreen = palette [i*3 + 1] << 2;
		m_dlcLogPalette->palPalEntry [i].peBlue = palette [i*3 + 2] << 2;
		m_dlcLogPalette->palPalEntry [i].peFlags = PC_RESERVED;
		}
#endif
	// now recreate the global Palette
	if (m_currentPalette) {
		delete m_currentPalette;
		m_currentPalette = new CPalette ();
		m_currentPalette->CreatePalette (m_dlcLogPalette);
//			::DeleteObject(m_currentPalette);
//			m_currentPalette = CreatePalette(m_dlcLogPalette);
		}
	}
FreeResource(hGlobal);
#endif //0

// read descent 2 reactor information
nLights = 0;
if (IsD2File ()) {
	ReactorTime () = read_INT32(loadFile); // base control center explosion time
	ReactorStrength () = read_INT32(loadFile); // reactor strength

#if 0
	sprintf_s (message, sizeof (message),  "Reactor time=%ld, Reactor strength=%ld, Secret Cube #=%ld",
		ReactorTime (), ReactorStrength (), SecretCubeNum ());
	INFOMSG(message);
#endif

	if (LevelVersion () > 6) {
		nLights = (INT16)read_INT32(loadFile);
		if (nLights > 0 && FlickerLightCount () <= MAX_FLICKERING_LIGHTS) {
			fread(FlickeringLights (), sizeof (CFlickeringLight), nLights, loadFile);
			} 
		else {
			if (nLights != 0) {
				ErrorMsg ("Error reading flickering lights");
				nLights = 0;
				}
			}
		}
	// NOTE: d2 v1.1 has two levels at version 7 (b2 and f4),
	//       both have 0 flickering lights
	//    sprintf_s (message, sizeof (message),  "%d flickering lights", nflicks);
	//    DEBUGMSG(message);

	// read secret cube number
	SecretCubeNum () = read_INT32(loadFile);
	// read secret cube orientation?
	read_matrix(&SecretOrient (), loadFile);
	}

m_disableDrawing = TRUE;

fseek(loadFile, minedata_offset, SEEK_SET);
mine_err = LoadMineDataCompiled(loadFile, bNewMine);
FlickerLightCount () = nLights;

if (mine_err != 0) {
	ErrorMsg ("Error loading mine data");
	fclose(loadFile);
	return(2);
}

fseek(loadFile, gamedata_offset, SEEK_SET);
game_err = LoadGameData(loadFile, bNewMine);

if (game_err != 0) {
	ErrorMsg ("Error loading game data");
	// reset "howmany"
	GameInfo ().objects.count = 0;
	GameInfo ().walls.count = 0;
	GameInfo ().doors.count = 0;
	GameInfo ().triggers.count = 0;
	GameInfo ().control.count = 0;
	GameInfo ().botgen.count = 0;
	GameInfo ().equipgen.count = 0;
	GameInfo ().lightDeltaIndices.count = 0;
	GameInfo ().lightDeltaValues.count = 0;
	fclose(loadFile);
	return(3);
	}

goto load_pog;

load_end:
	return_code = 0;
load_pog:

	fclose(loadFile);
if (!bLoadFromHog && (IsD2File ())) {
	ps = strstr (filename, ".");
	if (ps)
		strcpy_s (ps, 256 - (ps - filename), ".pog");
	else
		strcat_s (filename, 256, ".pog");
	fopen_s (&loadFile, filename, "rb");
	if (loadFile) {
		ReadPog (loadFile);
		fclose (loadFile);
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
	fopen_s (&loadFile, filename, "rb");
	if (loadFile) {
		ReadHxmFile (loadFile, -1);
		fclose (loadFile);
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
BOOL bCustom = memcmp (MineData ().texColors, dataP, sizeof (MineData ().texColors)) != 0;
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
if (i > sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]))
	i = sizeof (MineData ().texColors) / sizeof (MineData ().texColors [0]);
for (CColor *pc = MineData ().texColors; i; i--, pc++) {
	pc->index = *dataP++;
	pc->color.r = (double) *((INT32 *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (INT32);
	pc->color.g = (double) *((INT32 *) dataP) / (double) 0x7fffffff;
	dataP += sizeof (INT32);
	pc->color.b = (double) *((INT32 *) dataP) / (double) 0x7fffffff;
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
// Create()
//
// Action: makes a new level from the resource file
// ------------------------------------------------------------------------

INT16 CMine::CreateNewLevel ()
{
HGLOBAL hGlobal;
UINT32 nResSize;
UINT8 *data = LoadDataResource (MAKEINTRESOURCE ((IsD1File ()) ? IDR_NEW_RDL : IDR_NEW_RL2), hGlobal, nResSize);
if (!data)
	return 0;
// copy data to a file

FSplit ((m_fileType== RDL_FILE) ? descent_path : levels_path, m_startFolder , NULL, NULL);
sprintf_s (message, sizeof (message),  (m_fileType== RDL_FILE) ? "%sNEW.RDL" : "%sNEW.RL2", m_startFolder );
memcpy (RobotInfo (), DefRobotInfo (), sizeof (ROBOT_INFO) * N_robot_types);
texture_resource = (IsD1File ()) ? D1_TEXTURE_STRING_TABLE : D2_TEXTURE_STRING_TABLE;
FILE *file;
fopen_s (&file, message, "wb");
if (file) {
	size_t nBytes = fwrite(data, sizeof (UINT8), (UINT16)nResSize, file);
	fclose (file);
	FreeResource (hGlobal);
	if (nBytes != nResSize)
		return 1;
	NumObjTriggers () = 0;
	return 0;
	} 
else {
	FreeResource (hGlobal);
	return 2;
	}
}

// ------------------------------------------------------------------------
// FixIndexValues()
//
// Action - This routine attempts to fix array index values to prevent
//          the editor from crashing when the level is drawn.
//
// Returns - 0 if no error is detected
// ------------------------------------------------------------------------
INT16 CMine::FixIndexValues()
{
	INT16 	nSegment, nSide, nVertex;
	UINT16	nWall;
	INT16 	check_err;

	check_err = 0;
	CSegment *segP = Segments ();
	for(nSegment = 0; nSegment < SegCount (); nSegment++, segP++) {
		for(nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
			// check wall numbers
			CSide& side = segP->sides [nSide];
			if (side.nWall >= GameInfo ().walls.count &&
				side.nWall != NO_WALL (this)) {
				side.nWall = NO_WALL (this);
				check_err |= (1 << 0);
			}
			// check children
			if (segP->children [nSide] < - 2 || segP->children [nSide] >(INT16)SegCount ()) {
				segP->children [nSide] =-1;
				check_err |= (1 << 1);
			}
		}
		// check verts
		for(nVertex = 0; nVertex < MAX_VERTICES_PER_SEGMENT; nVertex++) {
			if (segP->verts [nVertex] < 0 || segP->verts [nVertex] >= VertCount ()) {
				segP->verts [nVertex] = 0;  // this will cause a bad looking picture
				check_err |= (1 << 2);      // but it will prevent a crash
			}
		}
	}
	CWall *wallP = Walls ();
	for (nWall = 0; nWall < GameInfo ().walls.count; nWall++, wallP++) {
		// check nSegment
		if (wallP->m_nSegment < 0 || wallP->m_nSegment > SegCount ()) {
			wallP->m_nSegment = 0;
			check_err |= (1 << 3);
		}
		// check nSide
		if (wallP->m_nSide < 0 || wallP->m_nSide > 5) {
			wallP->m_nSide = 0;
			check_err |= (1 << 4);
		}
	}
	return check_err;
}

// ------------------------------------------------------------------------
// CMine::Default()
// ------------------------------------------------------------------------
void CMine::Default()
{

	ClearMineData();

	if (m_pHxmExtraData) {
		free (m_pHxmExtraData);
		m_pHxmExtraData = NULL;
		m_nHxmExtraDataSize = 0;
		}

	CSegment& segP = *Segments ();
	CFixVector *vert = Vertices ();

	segP.sides [0].nWall = NO_WALL (this);
	segP.sides [0].nBaseTex = 0;
	segP.sides [0].nOvlTex = 0;
	segP.sides [0].uvls [0].u = 0;
	segP.sides [0].uvls [0].v = 0;
	segP.sides [0].uvls [0].l = 0x8000U;
	segP.sides [0].uvls [1].u = 0;
	segP.sides [0].uvls [1].v = 2047;
	segP.sides [0].uvls [1].l = 511;
	segP.sides [0].uvls [2].u = - 2048;
	segP.sides [0].uvls [2].v = 2047;
	segP.sides [0].uvls [2].l = 3833;
	segP.sides [0].uvls [3].u = - 2048;
	segP.sides [0].uvls [3].v = 0;
	segP.sides [0].uvls [3].l = 0x8000U;

	segP.sides [1].nWall = NO_WALL (this);
	segP.sides [1].nBaseTex = 263;
	segP.sides [1].nOvlTex = 264;
	segP.sides [1].uvls [0].u = 0;
	segP.sides [1].uvls [0].v = 0;
	segP.sides [1].uvls [0].l = 0x8000U;
	segP.sides [1].uvls [1].u = 0;
	segP.sides [1].uvls [1].v = 2047;
	segP.sides [1].uvls [1].l = 0x8000U;
	segP.sides [1].uvls [2].u = - 2048;
	segP.sides [1].uvls [2].v = 2047;
	segP.sides [1].uvls [2].l = 0x8000U;
	segP.sides [1].uvls [3].u = - 2048;
	segP.sides [1].uvls [3].v = 0;
	segP.sides [1].uvls [3].l = 0x8000U;

	segP.sides [2].nWall = NO_WALL (this);
	segP.sides [2].nBaseTex = 0;
	segP.sides [2].nOvlTex = 0;
	segP.sides [2].uvls [0].u = 0;
	segP.sides [2].uvls [0].v = 0;
	segP.sides [2].uvls [0].l = 0x8000U;
	segP.sides [2].uvls [1].u = 0;
	segP.sides [2].uvls [1].v = 2047;
	segP.sides [2].uvls [1].l = 3836;
	segP.sides [2].uvls [2].u = - 2048;
	segP.sides [2].uvls [2].v = 2047;
	segP.sides [2].uvls [2].l = 5126;
	segP.sides [2].uvls [3].u = - 2048;
	segP.sides [2].uvls [3].v = 0;
	segP.sides [2].uvls [3].l = 0x8000U;

	segP.sides [3].nWall = NO_WALL (this);
	segP.sides [3].nBaseTex = 270;
	segP.sides [3].nOvlTex = 0;
	segP.sides [3].uvls [0].u = 0;
	segP.sides [3].uvls [0].v = 0;
	segP.sides [3].uvls [0].l = 11678;
	segP.sides [3].uvls [1].u = 0;
	segP.sides [3].uvls [1].v = 2047;
	segP.sides [3].uvls [1].l = 12001;
	segP.sides [3].uvls [2].u = - 2048;
	segP.sides [3].uvls [2].v = 2047;
	segP.sides [3].uvls [2].l = 12001;
	segP.sides [3].uvls [3].u = - 2048;
	segP.sides [3].uvls [3].v = 0;
	segP.sides [3].uvls [3].l = 11678;

	segP.sides [4].nWall = NO_WALL (this);
	segP.sides [4].nBaseTex = 0;
	segP.sides [4].nOvlTex = 0;
	segP.sides [4].uvls [0].u = 0;
	segP.sides [4].uvls [0].v = 0;
	segP.sides [4].uvls [0].l = 0x8000U;
	segP.sides [4].uvls [1].u = 0;
	segP.sides [4].uvls [1].v = 2043;
	segP.sides [4].uvls [1].l = 0x8000U;
	segP.sides [4].uvls [2].u = - 2044;
	segP.sides [4].uvls [2].v = 2045;
	segP.sides [4].uvls [2].l = 0x8000U;
	segP.sides [4].uvls [3].u = - 2043;
	segP.sides [4].uvls [3].v = 0;
	segP.sides [4].uvls [3].l = 0x8000U;

	segP.sides [5].nWall = NO_WALL (this);
	segP.sides [5].nBaseTex = 0;
	segP.sides [5].nOvlTex = 0;
	segP.sides [5].uvls [0].u = 0;
	segP.sides [5].uvls [0].v = 0;
	segP.sides [5].uvls [0].l = 24576;
	segP.sides [5].uvls [1].u = 0;
	segP.sides [5].uvls [1].v = 2048;
	segP.sides [5].uvls [1].l = 24576;
	segP.sides [5].uvls [2].u = - 2048;
	segP.sides [5].uvls [2].v = 2048;
	segP.sides [5].uvls [2].l = 24576;
	segP.sides [5].uvls [3].u = - 2048;
	segP.sides [5].uvls [3].v = 0;
	segP.sides [5].uvls [3].l = 24576;

	segP.children [0] =-1;
	segP.children [1] =-1;
	segP.children [2] =-1;
	segP.children [3] =-1;
	segP.children [4] =-1;
	segP.children [5] =-1;

	segP.verts [0] = 0;
	segP.verts [1] = 1;
	segP.verts [2] = 2;
	segP.verts [3] = 3;
	segP.verts [4] = 4;
	segP.verts [5] = 5;
	segP.verts [6] = 6;
	segP.verts [7] = 7;

	segP.function = 0;
	segP.nMatCen =-1;
	segP.value =-1;
	segP.s2_flags = 0;
	segP.static_light = 263152L;
	segP.child_bitmask = 0;
	segP.wall_bitmask = 0;
	segP.nIndex = 0;
	segP.map_bitmask = 0;

	vert [0].x = 10*F1_0;
	vert [0].y = 10*F1_0;
	vert [0].z = - 10*F1_0;
	vert [1].x = 10*F1_0;
	vert [1].y = - 10*F1_0;
	vert [1].z = - 10*F1_0;
	vert [2].x = - 10*F1_0;
	vert [2].y = - 10*F1_0;
	vert [2].z = - 10*F1_0;
	vert [3].x = - 10*F1_0;
	vert [3].y = 10*F1_0;
	vert [3].z = - 10*F1_0;
	vert [4].x = 10*F1_0;
	vert [4].y = 10*F1_0;
	vert [4].z = 10*F1_0;
	vert [5].x = 10*F1_0;
	vert [5].y = - 10*F1_0;
	vert [5].z = 10*F1_0;
	vert [6].x = - 10*F1_0;
	vert [6].y = - 10*F1_0;
	vert [6].z = 10*F1_0;
	vert [7].x = - 10*F1_0;
	vert [7].y = 10*F1_0;
	vert [7].z = 10*F1_0;

	SegCount () = 1;
	VertCount () = 8;
}

// ------------------------------------------------------------------------
// ClearMineData()
// ------------------------------------------------------------------------

void CMine::ClearMineData() 
{
	INT16 i;

	// initialize Segments ()
CSegment *segP = Segments ();
for (i = 0; i < MAX_SEGMENTS (this); i++, segP++)
	segP->wall_bitmask &= ~MARKED_MASK;
SegCount () = 0;

// initialize vertices
for (i = 0; i < MAX_VERTICES (this); i++) {
	Vertices (i)->Unmark ();
}
VertCount () = 0;

FlickerLightCount () = 0;

// reset "howmany"
GameInfo ().objects.count = 0;
GameInfo ().walls.count = 0;
GameInfo ().doors.count = 0;
GameInfo ().triggers.count = 0;
GameInfo ().control.count = 0;
GameInfo ().botgen.count = 0;
GameInfo ().equipgen.count = 0;
GameInfo ().lightDeltaIndices.count = 0;
GameInfo ().lightDeltaValues.count = 0;
}

// ------------------------------------------------------------------------
// LoadMineDataCompiled()
//
// ACTION - Reads a mine data portion of RDL file.
// ------------------------------------------------------------------------

INT16 CMine::LoadMineDataCompiled (FILE *fp, bool bNewMine)
{
	INT16    i, nSegment, nSide; /** was INT32 */
	UINT8    version;
	UINT16   temp_UINT16;
	UINT8    bit_mask;
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
ClearMineData ();
VertCount () = n_vertices;
SegCount () = n_segments;

// read all vertices
for (i = 0; i < VertCount (); i++)
	Vertices (i)->Read (fp);

if (n_vertices != VertCount ()) {
	fseek(fp, sizeof (CFixVector)*(n_vertices - VertCount ()), SEEK_CUR);
	}

// unmark all vertices while we are here...
for (i = 0; i < VertCount (); i++) {
	Vertices (i)->Unmark ();
	}

// read segment information
for (nSegment = 0; nSegment < SegCount (); nSegment++)   {
	INT16   bit; /** was INT32 */
	CSegment *segP = Segments (nSegment);
	if (IsD2XLevel ()) {
		fread(&segP->owner, sizeof (UINT8), 1, fp);
		fread(&segP->group, sizeof (INT8), 1, fp);
		}
	else {
		segP->owner = -1;
		segP->group = -1;
		}
	// read in child mask (1 byte)
	fread(&bit_mask, sizeof (UINT8), 1, fp);
	segP->child_bitmask = bit_mask;

	// read 0 to 6 children (0 to 12 bytes)
	for (bit = 0; bit < MAX_SIDES_PER_SEGMENT; bit++) {
		if (bit_mask & (1 << bit)) {
			fread(&segP->children [bit], sizeof (INT16), 1, fp);
		} else {
			segP->children [bit] =-1;
		}
	}

	// read vertex numbers (16 bytes)
	fread(segP->verts, sizeof (INT16), MAX_VERTICES_PER_SEGMENT, fp);

	if (IsD1File ()) {
		// read special info (0 to 4 bytes)
		if (bit_mask & (1 << MAX_SIDES_PER_SEGMENT)) {
			fread(&segP->function, sizeof(UINT8), 1, fp);
			fread(&segP->nMatCen, sizeof(INT8), 1, fp);
			fread(&segP->value, sizeof(INT8), 1, fp);
			fread(&segP->s2_flags, sizeof(UINT8), 1, fp);
		} else {
			segP->owner = -1;
			segP->group = -1;
			segP->function = 0;
			segP->nMatCen =-1;
			segP->value = 0;
		}
		segP->s2_flags = 0;  // d1 doesn't use this number, so zero it

		// read static light (2 bytes)
		fread(&temp_UINT16, sizeof (temp_UINT16), 1, fp);
		segP->static_light = ((FIX)temp_UINT16) << 4;
	}

	// read the wall bit mask
	fread(&bit_mask, sizeof (UINT8), 1, fp);

	// read in wall numbers (0 to 6 bytes)
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++) {
		if (bit_mask & (1 << nSide)) {
			if (LevelVersion () < 13) {
				UINT8	nWall;
				fread(&nWall, sizeof (UINT8), 1, fp);
				segP->sides [nSide].nWall = nWall;
				}
			else {
				UINT16	nWall;
				fread(&nWall, sizeof (UINT16), 1, fp);
				segP->sides [nSide].nWall = nWall;
				}
			} 
		else {
			segP->sides [nSide].nWall = NO_WALL (this);
			}
		}

	// read in textures and uvls (0 to 60 bytes)
	for (nSide = 0; nSide < MAX_SIDES_PER_SEGMENT; nSide++)   {
		if ((segP->children [nSide]==-1) || (bit_mask & (1 << nSide))) {
			//  read in texture 1 number
			fread(&temp_UINT16, sizeof (UINT16), 1, fp);
			segP->sides [nSide].nBaseTex = temp_UINT16 & 0x7fff;
			//   read in texture 2 number
			if (!(temp_UINT16 & 0x8000)) {
				segP->sides [nSide].nOvlTex = 0;
			} else {
				fread(&temp_UINT16, sizeof (UINT16), 1, fp);
				segP->sides [nSide].nOvlTex = temp_UINT16;
				temp_UINT16 &= 0x1fff;
				if ((temp_UINT16 == 0) ||(temp_UINT16 >= MAX_TEXTURES (this)))
					segP->sides [nSide].nOvlTex = 0;
			}

			//   read CUVL numbers
			for (i = 0; i < 4; i++)   {
				fread(&segP->sides [nSide].uvls [i].u, sizeof (INT16), 1, fp);
				fread(&segP->sides [nSide].uvls [i].v, sizeof (INT16), 1, fp);
				fread(&segP->sides [nSide].uvls [i].l, sizeof (INT16), 1, fp);
			}
		} else {
			segP->sides [nSide].nBaseTex = 0;
			segP->sides [nSide].nOvlTex = 0;
			for (i = 0; i < 4; i++)   {
				segP->sides [nSide].uvls [i].u = 0;
				segP->sides [nSide].uvls [i].v = 0;
				segP->sides [nSide].uvls [i].l = 0;
				}
			}
		}
	}

	if (IsD2File ()) {
		CSegment *segP = Segments ();
		for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++) {
			// read special info (8 bytes)
			fread(&segP->function, sizeof(UINT8), 1, fp);
			fread(&segP->nMatCen, sizeof(INT8), 1, fp);
			fread(&segP->value, sizeof(INT8), 1, fp);
			fread(&segP->s2_flags, sizeof(UINT8), 1, fp);
			if (LevelVersion () <= 20)
				segP->Upgrade ();
			else {
				fread(&segP->props, sizeof(UINT8), 1, fp);
				fread(segP->damage, sizeof(INT16), 2, fp);
				}
			fread(&segP->static_light, sizeof(FIX), 1, fp);
			if ((segP->function == SEGMENT_FUNC_ROBOTMAKER) && (segP->nMatCen == -1)) {
				segP->function = SEGMENT_FUNC_NONE;
				segP->value = 0;
				segP->child_bitmask &= ~(1 << MAX_SIDES_PER_SEGMENT);
				}
			}
		}
	if (LevelVersion () == 9) {
#if 1
		LoadColors (LightColors (), SegCount () * 6, 9, 14, fp);
		LoadColors (LightColors (), SegCount () * 6, 9, 14, fp);
		LoadColors (VertexColors (), VertCount (), 9, 15, fp);
#else
		fread (LightColors (), sizeof (CColor), SegCount () * 6, fp); //skip obsolete side colors 
		fread (LightColors (), sizeof (CColor), SegCount () * 6, fp);
		fread (VertexColors (), sizeof (CColor), VertCount (), fp);
#endif
		}
	else if (LevelVersion () > 9) {
		LoadColors (VertexColors (), VertCount (), 9, 15, fp);
		LoadColors (LightColors (), SegCount () * 6, 9, 14, fp);
		LoadColors (TexColors (), MAX_D2_TEXTURES, 10, 16, fp);
		}
if (GameInfo ().objects.count > MAX_OBJECTS (this)) {
	sprintf_s (message, sizeof (message),  "Warning: Max number of objects for this level version exceeded (%ld/%d)", 
			  GameInfo ().objects.count, MAX_OBJECTS2);
	ErrorMsg (message);
	}
return 0;
}

// ------------------------------------------------------------------------
// LoadGameData()
//
// ACTION - Loads the player, object, wall, door, trigger, and
//          materialogrifizationator data from an RDL file.
// ------------------------------------------------------------------------

INT16 CMine::LoadGameData(FILE *loadfile, bool bNewMine) 
{
	INT32 i;
	INT32 start_offset;

	start_offset = ftell(loadfile);

	// Set default values
	GameInfo ().objects.count = 0;
	GameInfo ().walls.count = 0;
	GameInfo ().doors.count = 0;
	GameInfo ().triggers.count = 0;
	GameInfo ().control.count = 0;
	GameInfo ().botgen.count = 0;
	GameInfo ().equipgen.count = 0;
	GameInfo ().lightDeltaIndices.count = 0;
	GameInfo ().lightDeltaValues.count = 0;

	GameInfo ().objects.offset =-1;
	GameInfo ().walls.offset =-1;
	GameInfo ().doors.offset =-1;
	GameInfo ().triggers.offset =-1;
	GameInfo ().control.offset =-1;
	GameInfo ().botgen.offset =-1;
	GameInfo ().equipgen.offset =-1;
	GameInfo ().lightDeltaIndices.offset =-1;
	GameInfo ().lightDeltaValues.offset =-1;

	//==================== = READ FILE INFO========================

	// Read in game_top_fileinfo to get size of saved fileinfo.
	if (fseek(loadfile, start_offset, SEEK_SET)) {
		ErrorMsg ("Error seeking in mine.cpp");
		return -1;
	}
	if (fread(&game_top_fileinfo, sizeof (game_top_fileinfo), 1, loadfile) != 1) {
		ErrorMsg ("Error reading game info in mine.cpp");
		return -1;
	}
	// Check signature
	if (game_top_fileinfo.fileinfo_signature != 0x6705) {
		ErrorMsg ("Game data signature incorrect");
		return -1;
	}
	// Check version number
	//  if (game_top_fileinfo.fileinfo_version < GAME_COMPATIBLE_VERSION)
	//    return -1;

	// Now, Read in the fileinfo
	if (fseek(loadfile, start_offset, SEEK_SET)) {
		ErrorMsg ("Error seeking to game info in mine.cpp");
		return -1;
	}
	if (fread(&GameInfo (), (INT16)game_top_fileinfo.fileinfo_size, 1, loadfile)!= 1) {
		ErrorMsg ("Error reading game info from mine.cpp");
		return -1;
	}
	if (GameInfo ().fileinfo_version < 14) 
		m_currentLevelName [0] = 0;
	else {  /*load mine filename */
		char *p;
		for (p = m_currentLevelName; ; p++) {
			*p = fgetc(loadfile);
			if (*p== '\n') *p = 0;
			if (*p== 0) break;
			}
		}

#if 0
	if (GameInfo ().fileinfo_version >= 19) {  /*load pof names */
		names.offset = ftell(loadfile);
		fread(&n_save_pof_names, 2, 1, loadfile);
		n_total_names = (GameInfo ().player.offset - names.offset)/13;
		if (n_total_names > 100) {
			sprintf_s (message, sizeof (message),  "Exceeded maximum number of total pof names (100 max)");
			ErrorMsg (message);
			n_total_names = 100;
			}
		fread(save_pof_names, n_total_names, 13, loadfile);
	}
#endif

	//==================== = READ PLAYER INFO==========================
	//  object_next_signature = 0;
	// 116 bytes of data "GUILE", "BIG_RED", or "RACER_X" + NULL + junk
	// use this area to store version of DMB

	//==================== = READ OBJECT INFO==========================
	// note: same for D1 and D2
	//  gamesave_num_org_robots = 0;
	//  gamesave_num_players = 0;

	if (GameInfo ().objects.offset > -1) {
		if (fseek(loadfile, GameInfo ().objects.offset, SEEK_SET))
			ErrorMsg ("Error seeking to objects.");
		else if (GameInfo ().objects.count > MAX_OBJECTS2) {
			sprintf_s (message, sizeof (message),  "Error: Max number of objects (%ld/%d) exceeded", 
					  GameInfo ().objects.count, MAX_OBJECTS2);
			ErrorMsg (message);
			GameInfo ().objects.count = MAX_OBJECTS (this);
			}
		else {
			CGameObject *objP = Objects ();
			for (i = 0; i < GameInfo ().objects.count; i++, objP++) {
				objP->Read (loadfile, GameInfo ().fileinfo_version);
				//      objP->signature = object_next_signature++;
				//    verify_object(objP);
			}
		}
	}

	//==================== = READ WALL INFO============================
	// note: Wall size will automatically strip last two items

	if ((GameInfo ().walls.offset > -1) && !fseek(loadfile, GameInfo ().walls.offset, SEEK_SET)) {
		if (GameInfo ().walls.count > MAX_WALLS (this)) {
			sprintf_s (message, sizeof (message),  "Error: Max number of walls (%d/%d) exceeded", GameInfo ().walls.count, MAX_WALLS (this));
			ErrorMsg (message);
			GameInfo ().walls.count = MAX_WALLS (this);
			}
		else if (GameInfo ().fileinfo_version < 20)
			ErrorMsg ("Wall version < 20, walls not loaded");
		else if (GameInfo ().walls.count) {
			for (i = 0; i < GameInfo ().walls.count; i++) {
				if (!Walls (i)->Read (loadfile, GameInfo ().fileinfo_version)) {
					ErrorMsg ("Error reading walls from mine.cpp");
					break;
					}
				}
			}
		}

	//==================== = READ DOOR INFO============================
	// note: not used for D1 or D2 since doors.count is always 0
	if ((GameInfo ().doors.offset > -1) && !fseek(loadfile, GameInfo ().doors.offset, SEEK_SET)) {
		if (GameInfo ().doors.count > MAX_DOORS) {
			sprintf_s (message, sizeof (message),  "Error: Max number of doors (%ld/%d) exceeded", GameInfo ().doors.count, MAX_DOORS);
			ErrorMsg (message);
			GameInfo ().doors.count = MAX_DOORS;
			}
		else if (GameInfo ().fileinfo_version < 20)
			ErrorMsg ("Door version < 20, doors not loaded");
		else if (sizeof (*ActiveDoors (i)) != GameInfo ().doors.size)
			ErrorMsg ("Error: Door size incorrect");
		else if (GameInfo ().doors.count) {
			for (i = 0; i < GameInfo ().doors.count; i++) {
				if (!ActiveDoors ()->Read (loadfile, GameInfo ().fileinfo_version))
					ErrorMsg ("Error reading doors.");
				}
			}
		}

	//==================== READ TRIGGER INFO==========================
	// note: order different for D2 levels but size is the same
	if (GameInfo ().triggers.offset > -1) {
		if (GameInfo ().triggers.count > MAX_TRIGGERS (this)) {
			sprintf_s (message, sizeof (message),  "Error: Max number of triggers (%ld/%d) exceeded",
				GameInfo ().triggers.count, MAX_TRIGGERS (this));
			ErrorMsg (message);
			GameInfo ().triggers.count = MAX_TRIGGERS (this);
		}
		if (!fseek(loadfile, GameInfo ().triggers.offset, SEEK_SET)) 
			for (i = 0; i < GameInfo ().triggers.count; i++)
				Triggers (i)->Read (loadfile, GameInfo ().fileinfo_version, false);
		INT32 bObjTriggersOk = 1;
		if (GameInfo ().fileinfo_version >= 33) {
			INT32 i = ftell (loadfile);
			if (fread (&NumObjTriggers (), sizeof (INT32), 1, loadfile) != 1) {
				ErrorMsg ("Error reading object triggers from mine.");
				bObjTriggersOk = 0;
				}
			else {
				for (i = 0; i < NumObjTriggers (); i++)
					ObjTriggers (i)->Read (loadfile, GameInfo ().fileinfo_version, true);
				if (GameInfo ().fileinfo_version >= 40) {
					for (i = 0; i < NumObjTriggers (); i++)
						ObjTriggers (i)->nObject = read_INT16 (loadfile);
					}
				else {
					for (i = 0; i < NumObjTriggers (); i++) {
						read_INT16 (loadfile);
						read_INT16 (loadfile);
						ObjTriggers (i)->nObject = read_INT16 (loadfile);
						}
					if (GameInfo ().fileinfo_version < 36)
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
			memset (ObjTriggers (), 0, sizeof (CTrigger) * MAX_OBJ_TRIGGERS);
			}
	}

	//================ READ CONTROL CENTER TRIGGER INFO============== =
	// note: same for D1 and D2
	if (GameInfo ().control.offset > -1) {
		if (GameInfo ().control.count > MAX_CONTROL_CENTER_TRIGGERS) {
			sprintf_s (message, sizeof (message),  "Error: Max number of control center Triggers () (%ld, %d) exceeded",
				GameInfo ().control.count, MAX_CONTROL_CENTER_TRIGGERS);
			ErrorMsg (message);
			GameInfo ().control.count = MAX_CONTROL_CENTER_TRIGGERS;
		}
		if (!fseek(loadfile, GameInfo ().control.offset, SEEK_SET))  {
			for (i = 0; i < GameInfo ().control.count; i++)
				if (!ReactorTriggers (i)->Read (loadfile, GameInfo ().fileinfo_version)) {
					ErrorMsg ("Error reading control center triggers from mine.cpp");
//					break;
				}
		}
	}

	//================ READ MATERIALIZATION CENTER INFO============== =
	// note: added robot_flags2 for Descent 2
	if (GameInfo ().botgen.offset > -1) {
		if (GameInfo ().botgen.count > MAX_NUM_MATCENS (this)) {
			sprintf_s (message, sizeof (message),  "Error: Max number of robot centers (%ld/%d) exceeded",
				GameInfo ().botgen.count, MAX_NUM_MATCENS (this));
			ErrorMsg (message);
			GameInfo ().botgen.count = MAX_NUM_MATCENS (this);
		}
		if (!fseek(loadfile, GameInfo ().botgen.offset, SEEK_SET))  {
			for (i = 0; i < GameInfo ().botgen.count; i++) {
				if (!BotGens (i)->Read (loadfile, GameInfo ().fileinfo_version)) {
					ErrorMsg ("Error reading botgens from mine.cpp");
					break;
					}
				}
			}
		}

	//================ READ EQUIPMENT CENTER INFO============== =
	// note: added robot_flags2 for Descent 2
	if (GameInfo ().equipgen.offset > -1) {
		if (GameInfo ().equipgen.count > MAX_NUM_MATCENS (this)) {
			sprintf_s (message, sizeof (message),  "Error: Max number of robot centers (%ld/%d) exceeded",
				GameInfo ().equipgen.count, MAX_NUM_MATCENS (this));
			ErrorMsg (message);
			GameInfo ().equipgen.count = MAX_NUM_MATCENS (this);
		}
		if (!fseek(loadfile, GameInfo ().equipgen.offset, SEEK_SET))  {
			for (i = 0; i < GameInfo ().equipgen.count; i++) {
				if (IsD2File ()) {
					if (fread(EquipGens (i), (INT16)GameInfo ().equipgen.size, 1, loadfile)!= 1) {
						ErrorMsg ("Error reading equipgens from mine.cpp");
						break;
						}
					}
				}
			}
		}

	//================ READ DELTA LIGHT INFO============== =
	// note: D2 only
	if (IsD2File ()) {
		//    sprintf_s (message, sizeof (message),  "Number of delta light indices = %ld", GameInfo ().lightDeltaIndices.count);
		//    DEBUGMSG(message);
		if (GameInfo ().lightDeltaIndices.count > MAX_LIGHT_DELTA_INDICES (this)) {
			sprintf_s (message, sizeof (message),  "Error: Max number of delta light indices (%ld/%d) exceeded",
				GameInfo ().lightDeltaIndices.count, MAX_LIGHT_DELTA_INDICES (this));
			ErrorMsg (message);
			GameInfo ().lightDeltaIndices.count = MAX_LIGHT_DELTA_INDICES (this);
			}
		if (GameInfo ().lightDeltaIndices.offset > -1 && GameInfo ().lightDeltaIndices.count > 0) {
			if (!fseek(loadfile, GameInfo ().lightDeltaIndices.offset, SEEK_SET)) {
				bool bD2X = (LevelVersion () >= 15) && (GameInfo ().fileinfo_version >= 34);
				for (i = 0; i < GameInfo ().lightDeltaIndices.count; i++) {
					if (!LightDeltaIndex (i)->Read (loadfile, bD2X)) 
					ErrorMsg ("Error reading delta light indices from mine.cpp");
					}
				}
			}
		}

	//==================== READ DELTA LIGHTS==================== =
	// note: D2 only
	if (IsD2File ()) {
		//    sprintf_s (message, sizeof (message),  "Number of delta light values = %ld", GameInfo ().lightDeltaValues.count);
		//    DEBUGMSG(message);
		if (GameInfo ().lightDeltaValues.count > MAX_LIGHT_DELTA_VALUES (this)) {
			sprintf_s (message, sizeof (message),  "Error: Max number of delta light values (%ld/%d) exceeded",
				GameInfo ().lightDeltaValues.count, MAX_LIGHT_DELTA_VALUES (this));
			ErrorMsg (message);
			GameInfo ().lightDeltaValues.count = MAX_LIGHT_DELTA_VALUES (this);
			}
		if (GameInfo ().lightDeltaValues.offset > -1 && GameInfo ().lightDeltaIndices.count > 0) {
			if (!fseek(loadfile, GameInfo ().lightDeltaValues.offset, SEEK_SET)) {
				CLightDeltaValue *dl, temp_dl;
				dl = LightDeltaValues ();
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
		}
	return 0;
}

// ------------------------------------------------------------------------
// ReadObject()
// ------------------------------------------------------------------------
// CMine - save()
//
// ACTION -  saves a level (.RDL) file to disk
// ------------------------------------------------------------------------

INT16 CMine::Save (const char * filename_passed, bool bSaveToHog)
{
#if 1 //DEMO == 0
#ifdef _DEBUG
	CMine *mine = theApp.GetMine ();
#endif
	FILE * save_file;
	char filename [256];
	INT32 minedata_offset, gamedata_offset, hostagetext_offset;
	INT32 mine_err, game_err;

	//  if (disable_saves) {
	//    ErrorMsg ("Saves disabled, contact Bryan Aamot for your security number.");
	//  }
	strcpy_s (filename, sizeof (filename), filename_passed);

	//  if (disable_saves) return 0;
	fopen_s (&save_file, filename, "wb");
	if (!save_file) {
		//    sprintf_s (message, sizeof (message),  "Can't open save file < %s>", filename);
		//    show_message(str, RED, 1, 1);
		return(1);
	}

	m_changesMade = 0;

	// write file signature
	write_INT32 ('P'*0x1000000L + 'L'*0x10000L + 'V'*0x100 + 'L', save_file); // signature

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
	write_INT32 (LevelVersion (), save_file);

	write_INT32 (0, save_file); // minedata_offset (temporary)
	write_INT32 (0, save_file); // gamedata_offset (temporary)


	if (IsD2File ()) {
		if (LevelVersion () >= 8) {
			write_INT16(rand(), save_file);
			write_INT16(rand(), save_file);
			write_INT16(rand(), save_file);
			write_INT8((INT8)rand(), save_file);
		}
	}

	if (m_fileType== RDL_FILE) {
		write_INT32 (0, save_file); // hostagetext_offset (temporary)
	} else {

		// save palette name
		char *name = strrchr(descent2_path, '\\');
		if (!name) {
			name = descent2_path; // point to 1st char if no slash found
		} else {
			name++;               // point to character after slash
		}
		char palette_name [15];
		strncpy_s (palette_name, sizeof (palette_name), name, 12);
		palette_name [13] = NULL;  // null terminate just in case
		// replace extension with *.256
		if (strlen ((char *)palette_name) > 4) {
			strcpy_s (&palette_name [strlen ((char *) palette_name) - 4], 5, ".256");
		} else {
			strcpy_s (palette_name, sizeof (palette_name), "GROUPA.256");
		}
		_strupr_s (palette_name, sizeof (palette_name));
		strcat_s (palette_name, sizeof (palette_name), "\n"); // add a return to the end
		fwrite (palette_name, strlen ((char *)palette_name), 1, save_file);
	}

	// write reactor info
	if (IsD2File ()) {
		// read descent 2 reactor information
		write_INT32 (ReactorTime (), save_file);
		write_INT32 (ReactorStrength (), save_file);

		// flickering light new for version 7
		write_INT32 (FlickerLightCount (), save_file);
		if (FlickerLightCount () > MAX_FLICKERING_LIGHTS) {
			FlickerLightCount () = MAX_FLICKERING_LIGHTS;
		}
		if (FlickerLightCount () > 0) {
			fwrite(FlickeringLights (), sizeof (CFlickeringLight), FlickerLightCount (), save_file);
		}

		// write secret cube number
		write_INT32 (SecretCubeNum (), save_file);

		// write secret cube orientation?
		write_matrix(&SecretOrient (), save_file);

	}

	// save mine data
	minedata_offset = ftell(save_file);
	mine_err = SaveMineDataCompiled(save_file);

	if (mine_err== -1) {
		fclose(save_file);
		ErrorMsg ("Error saving mine data");
		return(2);
	}

	// save game data
	gamedata_offset = ftell(save_file);
	game_err = SaveGameData(save_file);

	if (game_err== -1) {
		fclose(save_file);
		ErrorMsg ("Error saving game data");
		return(3);
	}

	// save hostage data
	hostagetext_offset = ftell(save_file);
	// leave hostage text empty

	// now and go back to beginning of file and save offsets
	fseek(save_file, 2*sizeof (INT32), SEEK_SET);
	write_INT32 (minedata_offset, save_file);    // gamedata_offset
	write_INT32 (gamedata_offset, save_file);    // gamedata_offset
	if (m_fileType== RDL_FILE) {
		write_INT32 (hostagetext_offset, save_file); // hostagetext_offset
	}

	fclose(save_file);
if (HasCustomTextures () && !bSaveToHog) {
	char* ps = strstr (filename, ".");
	if (ps)
		strcpy_s (ps, sizeof (filename) - (ps - filename), ".pog");
	else
		strcat_s (filename, sizeof (filename), ".pog");
	fopen_s (&save_file, filename, "wb");
	if (save_file) {
		CreatePog (save_file);
		fclose (save_file);
		}
	}
if (HasCustomRobots () && !bSaveToHog) {
	char* ps = strstr (filename, ".");
	if (ps)
		strcpy_s (ps, sizeof (filename) - (ps - filename), ".hxm");
	else
		strcat_s (filename, sizeof (filename), ".hxm");
	fopen_s (&save_file, filename, "wb");
	if (save_file)
		WriteHxmFile (save_file);
	}
#endif //DEMO
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
INT16 CMine::SaveMineDataCompiled(FILE *save_file)
{
	int	i;
// write version (1 byte)
write_INT8 (COMPILED_MINE_VERSION, save_file);

// write no. of vertices (2 bytes)
write_INT16 (VertCount (), save_file);

// write number of Segments () (2 bytes)
write_INT16 (SegCount (), save_file);

// write all vertices
for (int i = 0; i VertCount (); i++)
	Vertices (i)->Write (save_file);

// write segment information
for (nSegment = 0; nSegment < SegCount (); nSegment++)  
	Segments (nSegment)->Write (save_file, IsD2XLevel () ? 2 : IsD2File () ? 1 : 0, LevelVersion());

// for Descent 2, save special info here
if (IsD2File ()) {
  for (nSegment = 0; nSegment < SegCount (); nSegment++)  
	  Segments (nSegment)->WriteExtras (save_file, IsD2XLevel () ? 2 : 1, true);
  }

if (IsD2XLevel ()) {
	SaveColors (VertexColors (), VertCount (), save_file);
	SaveColors (LightColors (), SegCount () * 6, save_file);
	SaveColors (TexColors (), MAX_D2_TEXTURES, save_file);
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
	INT32 start_offset, end_offset;

	start_offset = ftell(savefile);

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
		GameInfo ().fileinfo_signature = 0x6705;
		GameInfo ().fileinfo_version = 25;
		GameInfo ().fileinfo_size = 119;
		GameInfo ().level = 0;
		}
	else {
		GameInfo ().fileinfo_signature = 0x6705;
		GameInfo ().fileinfo_version = (LevelVersion () < 13) ? 31 : 40;
		GameInfo ().fileinfo_size = (LevelVersion () < 13) ? 143 : sizeof (GameInfo ()); // same as sizeof (GameInfo ())
		GameInfo ().level = 0;
	}

	fwrite(&GameInfo (), (INT16)GameInfo ().fileinfo_size, 1, savefile);
	if (GameInfo ().fileinfo_version >= 14) {  /*save mine filename */
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
		Objects (i)->Write (savefile, GameInfo ().fileinfo_version);

	//==================== = WRITE WALL INFO============================
	// note: Wall size will automatically strip last two items
	//       when saving D1 level
	GameInfo ().walls.offset = ftell(savefile);
	if (GameInfo ().fileinfo_version >= 20) {
		for (i = 0; i < GameInfo ().walls.count; i++)
			Walls (i)->Write (savefile, GameInfo ().fileinfo_version);
	}

	//==================== = WRITE DOOR INFO============================
	// note: not used for D1 or D2 since doors.count is always 0
	GameInfo ().doors.offset = ftell(savefile);
	if (GameInfo ().fileinfo_version >= 20)
//	for (i = 0; i < GameInfo ().doors.count; i++)
			fwrite(ActiveDoors (i), TotalSize (GameInfo ().doors), 1, savefile);

	//==================== WRITE TRIGGER INFO==========================
	// note: order different for D2 levels but size is the same
	GameInfo ().triggers.offset = ftell(savefile);
	for (i = 0; i < GameInfo ().triggers.count; i++)
		Triggers (i)->Write (savefile, GameInfo ().fileinfo_version, false);
	if (LevelVersion () >= 12) {
		write_INT32 (NumObjTriggers (), savefile);
		if (NumObjTriggers ()) {
			SortObjTriggers ();
			for (i = 0; i < NumObjTriggers (); i++)
				ObjTriggers (i)->Write (savefile, GameInfo ().fileinfo_version, true);
			for (i = 0; i < NumObjTriggers (); i++)
				write_INT16 (ObjTriggers (i)->nObject, savefile);
			}
		}

	//================ WRITE CONTROL CENTER TRIGGER INFO============== =
	// note: same for D1 and D2
	GameInfo ().control.offset = ftell(savefile);
	//for (i = 0; i < GameInfo ().control.count; i++)
		fwrite(ReactorTriggers (), TotalSize (GameInfo ().control), 1, savefile);

	//================ WRITE MATERIALIZATION CENTERS INFO============== =
	// note: added robot_flags2 for Descent 2
	GameInfo ().botgen.offset = ftell(savefile);
	if (IsD2File ())
		fwrite(BotGens (), TotalSize (GameInfo ().botgen), 1, savefile);
	else {
		for (i = 0; i < GameInfo ().botgen.count; i++) {
			write_INT32 (BotGens (i)->objFlags[0], savefile);
			// skip robot_flags2
			write_FIX  (BotGens (i)->hitPoints, savefile);
			write_FIX  (BotGens (i)->interval, savefile);
			write_INT16(BotGens (i)->nSegment, savefile);
			write_INT16(BotGens (i)->nFuelCen, savefile);
		}
	}

	//================ WRITE EQUIPMENT CENTERS INFO============== =
	// note: added robot_flags2 for Descent 2
	GameInfo ().equipgen.offset = ftell(savefile);
	if (IsD2File ())
		fwrite(EquipGens (), TotalSize (GameInfo ().equipgen), 1, savefile);

	//============== CALCULATE DELTA LIGHT DATA============ =
	if (IsD2File ())
		UpdateDeltaLights ();

	//================ WRITE DELTA LIGHT INFO============== =
	// note: D2 only
	GameInfo ().lightDeltaIndices.offset = ftell(savefile);
	if ((LevelVersion () >= 15) && (GameInfo ().fileinfo_version >= 34))
		SortDLIndex (0, GameInfo ().lightDeltaIndices.count - 1);
	if (IsD2File ())
		fwrite(LightDeltaIndex (), TotalSize (GameInfo ().lightDeltaIndices), 1, savefile);

	//================ = WRITE DELTA LIGHTS==================
	// note: D2 only
	GameInfo ().lightDeltaValues.offset = ftell(savefile);
	if (IsD2File ()) {
		CLightDeltaValue *dl, temp_dl;
		dl = LightDeltaValues ();
		for (i = 0; i < GameInfo ().lightDeltaValues.count; i++) {
			memcpy(&temp_dl, dl, (INT16)(GameInfo ().lightDeltaValues.size));
			fwrite(&temp_dl, (INT16)(GameInfo ().lightDeltaValues.size), 1, savefile);
			dl++;
		}
	}

	end_offset = ftell(savefile);

	//==================== = UPDATE FILE INFO OFFSETS====================== =
	fseek(savefile, start_offset, SEEK_SET);
	fwrite(&GameInfo (), (INT16)GameInfo ().fileinfo_size, 1, savefile);

	//============ = LEAVE ROUTINE AT LAST WRITTEN OFFSET================== = */
	fseek(savefile, end_offset, SEEK_SET);
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
	CSegment *segP = Segments ();
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
// --------------------------------------------------------------------------
void CMine::CalcOrthoVector(CFixVector &result, INT16 nSegment, INT16 nSide)
{
	struct dvector a, b, c;
	double length;
	INT16 vertnum1, vertnum2;
    // calculate orthogonal vector from lines which intersect point 0
    //
    //       |x  y  z |
    // AxB = |ax ay az|= x(aybz - azby), y(azbx - axbz), z(axby - aybx)
    //       |bx by bz|

    vertnum1 =Segments (nSegment)->verts [side_vert [nSide] [0]];
    vertnum2 =Segments (nSegment)->verts [side_vert [nSide] [1]];
	 CFixVector *v1 = Vertices (vertnum1);
	 CFixVector *v2 = Vertices (vertnum2);
    a.x = (double)(v2->x - v1->x);
    a.y = (double)(v2->y - v1->y);
    a.z = (double)(v2->z - v1->z);
    vertnum1 =Segments (nSegment)->verts [side_vert [nSide] [0]];
    vertnum2 =Segments (nSegment)->verts [side_vert [nSide] [3]];
	 v1 = Vertices (vertnum1);
	 v2 = Vertices (vertnum2);
    b.x = (double)(v2->x - v1->x);
    b.y = (double)(v2->y - v1->y);
    b.z = (double)(v2->z - v1->z);

    c.x = a.y*b.z - a.z*b.y;
    c.y = a.z*b.x - a.x*b.z;
    c.z = a.x*b.y - a.y*b.x;

    // normalize the vector
    length = sqrt(c.x*c.x + c.y*c.y + c.z*c.z);
    if (length>0) {
		c.x /= length;
		c.y /= length;
		c.z /= length;
    }

    result.x = (long) dround_off (-c.x * 0x10000L, 1.0);
    result.y = (long) dround_off (-c.y * 0x10000L, 1.0);
    result.z = (long) dround_off (-c.z * 0x10000L, 1.0);
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
void CMine::CalcCenter(CFixVector &center, INT16 nSegment, INT16 nSide)
{
	INT32 i;

	center.x = center.y = center.z = 0;
	CFixVector *v;
	CSegment *segP = Segments (nSegment);
	for (i = 0; i < 4; i++) {
		v = Vertices (segP->verts [side_vert [nSide][i]]);
		center.x += (v->x) >> 2;
		center.y += (v->y) >> 2;
		center.z += (v->z) >> 2;
	}
}

CMine* GetMine (CMine* m) { return m ? m : theApp.GetMine (); }
//eof mine.cpp