#ifndef __mine_h
#define __mine_h

#include "carray.h"
#include "types.h"
#include "segment.h"
#include "robot.h"
#include "object.h"
#include "textures.h"
#include "poly.h"

#define MAX_LIGHT_DEPTH 6

// external data
extern byte sideVertTable[6][4];
extern byte oppSideTable[6];
extern byte oppSideVertTable[6][4];
extern byte lineVertTable[12][2];
extern byte sideLineTable[6][4];
extern byte connectPointTable[8][3];
extern char pointSideTable[8][3];
extern char pointCornerTable[8][3];
extern TEXTURE_LIGHT d1_texture_light[NUM_LIGHTS_D1];
extern TEXTURE_LIGHT d2_texture_light[NUM_LIGHTS_D2];

// Copyright (C) 1997 Bryan Aamot
//**************************************************************************
// CLASS - Level
//**************************************************************************

#ifdef USE_DYN_ARRAYS

typedef CStaticArray< CRobotInfo, MAX_ROBOT_TYPES > robotInfoList;
typedef CStaticArray< CColor, SEGMENT_LIMIT * 6 > lightColorList;
typedef CStaticArray< CColor, MAX_TEXTURES_D2 > texColorList;
typedef CStaticArray< CColor, VERTEX_LIMIT > vertexColorList;
typedef CStaticArray< CActiveDoor, DOOR_LIMIT > activeDoorList;
typedef CStaticArray< CTrigger, MAX_TRIGGERS_D2 > triggerList;
typedef CStaticArray< CTrigger, MAX_OBJ_TRIGGERS > objTriggerList;
typedef CStaticArray< CRobotMaker, MAX_NUM_MATCENS_D2 > robotMakerList;
typedef CStaticArray< CGameObject, MAX_OBJECTS_D2 > objectList;
typedef CStaticArray< CLightDeltaIndex, MAX_LIGHT_DELTA_INDICES_D2X > lightDeltaIndexList;
typedef CStaticArray< CLightDeltaValue, MAX_LIGHT_DELTA_VALUES_D2X > lightDeltaValueList;
typedef CStaticArray< CFlickeringLight, MAX_FLICKERING_LIGHTS > flickeringLightList;

#define CLEAR(_b) (_b) [0].Reset((_b).Length ())
#define ASSIGN(_a,_b) (_a) = (_b)
#define DATA(_b) (_b).Buffer ()

#else

typedef CRobotInfo robotInfoList [MAX_ROBOT_TYPES];
typedef CColor lightColorList [SEGMENT_LIMIT * 6];
typedef CColor texColorList [MAX_TEXTURES_D2];
typedef CColor vertexColorList [VERTEX_LIMIT];
typedef CActiveDoor activeDoorList [DOOR_LIMIT];
typedef CRobotMaker robotMakerList [MAX_NUM_MATCENS_D2];
typedef CGameObject objectList [MAX_OBJECTS_D2];
typedef CLightDeltaIndex lightDeltaIndexList [MAX_LIGHT_DELTA_INDICES_D2X];
typedef CLightDeltaValue lightDeltaValueList [MAX_LIGHT_DELTA_VALUES_D2X];
typedef CFlickeringLight flickeringLightList [MAX_FLICKERING_LIGHTS];

#define CLEAR(_b)	(_b)->Reset (sizeof (_b) / sizeof (_b [0]))
#define ASSIGN(_a,_b) memcpy (_a, _b, sizeof (_a))
#define DATA(_b) (_b)

#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CSelection {
public:
	CSelection() :
		nSegment(0),
		nSide(DEFAULT_SIDE),
		nLine(DEFAULT_LINE),
		nPoint(DEFAULT_POINT),
		nObject(DEFAULT_OBJECT)
	{}

	short nSegment;
	short nSide;
	short nLine;
	short nPoint;
	short nObject;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tMineData {
	CGameInfo					gameInfo;
	
	int							m_reactor_time;
	int							m_reactor_strength;
	int							m_secret_cubenum;
	CDoubleMatrix				m_secret_orient;
	
	// robot data
	robotInfoList				robotInfo;
	
	// structure data
	lightColorList				lightColors;
	texColorList				texColors;
	vertexColorList			vertexColors;
	wallList						walls;
	activeDoorList				activeDoors;
	reactorTriggerList		reactorTriggers;
	robotMakerList				robotMakers;
	robotMakerList				equipMakers;
	objectList					objects;
	lightDeltaIndexList		lightDeltaIndices;
	lightDeltaValueList		lightDeltaValues;
	short							m_nFlickeringLights;
	flickeringLightList		flickeringLights;

	//CRobotInfo				robotInfo [MAX_ROBOT_TYPES];
	//CVertex					vertices [VERTEX_LIMIT];
	//CSegment					segments [SEGMENT_LIMIT];
	//CColor 					lightColors [SEGMENT_LIMIT][6];
	//CColor						texColors [MAX_TEXTURES_D2];
	//CColor						vertexColors [VERTEX_LIMIT];
	//CWall						walls[WALL_LIMIT];
	//CActiveDoor				activeDoors[DOOR_LIMIT];
	//CTrigger					triggers[MAX_TRIGGERS_D2];
	//CTrigger					objTriggers[MAX_OBJ_TRIGGERS];
	//CReactorTrigger			reactorTriggers[MAX_REACTOR_TRIGGERS];
	//CRobotMaker				robotMakers[MAX_NUM_MATCENS_D2];
	//CRobotMaker				equipMakers[MAX_NUM_MATCENS_D2];
	//CGameObject				objects[MAX_OBJECTS_D2];
	//CLightDeltaIndex		lightDeltaIndices [MAX_LIGHT_DELTA_INDICES_D2X];
	//CLightDeltaValue		lightDeltaValues [MAX_LIGHT_DELTA_VALUES_D2X];
	//CFlickeringLight		flickeringLights[MAX_FLICKERING_LIGHTS];

	CSelection					current1;
	CSelection					current2;
	CSelection					*current;

} MINE_DATA;

// -----------------------------------------------------------------------------

class CMine {
public:
	// level info
	int				m_fileType;
	int				m_levelVersion;
	char				m_currentLevelName [256];	
	MINE_DATA		m_mineData;
	robotInfoList	m_defaultRobotInfo;
	HPALETTE			m_paletteHandle;
	
	// strings
	char				message[256];
	char				m_startFolder [256];
	short				m_selectMode;
	int				m_disableDrawing;
	int				m_changesMade;
	bool				m_bSplineActive;
	BOOL				m_bSortObjects;
	int				m_nMaxSplines;
	int				m_nNoLightDeltas;
	int				m_lightRenderDepth;
	int				m_deltaLightRenderDepth;
	char				m_szBlockFile [256];
	double			m_splineLength1,
						m_splineLength2;
	bool				m_bVertigo;
	char*				m_pHxmExtraData;
	int				m_nHxmExtraDataSize;
	bool				m_bUseTexColors;
// Constructor/Desctuctor
public:
	CMine();
	~CMine();
	void Initialize (void);
	void Reset (void);
	
public:
	inline MINE_DATA& MineData ()
		{ return m_mineData; }

	inline vertexList& Vertices (void)
		{ return segmentManager.Vertices (); }
	inline CVertex *Vertices (int i)
		{ return Vertices () + i; }
	inline byte& VertStatus (int i = 0)
		{ return Vertices (i)->m_status; }

	inline segmentList& Segments (void)
		{ return segmentManager.Segments (); }
	inline ushort& SegCount ()
		{ return segmentManager.SegCount (); }
	inline CSegment *Segments (int i)
		{ return Segments () + i; }

	inline vertexColorList& VertexColors (void)
		{ return MineData ().vertexColors; }

	inline wallList& Walls (void)
		{ return MineData ().walls; }

	inline triggerList& Triggers (void)
		{ return MineData ().triggers; }
	inline objTriggerList& ObjTriggers (void)
		{ return MineData ().objTriggers; }
	inline reactorTriggerList& ReactorTriggers (void)
		{ return MineData ().reactorTriggers; }

	inline objectList& Objects (void)
		{ return MineData ().objects; }
	inline robotMakerList& BotGens (void)
		{ return MineData ().robotMakers; }
	inline robotMakerList& EquipGens (void)
		{ return MineData ().equipMakers; }
	inline activeDoorList& ActiveDoors (void)
		{ return MineData ().activeDoors; }
	inline robotInfoList& RobotInfo (void)
		{ return MineData ().robotInfo; }
	inline robotInfoList& DefRobotInfo (void)
		{ return m_defaultRobotInfo; }
	inline lightDeltaIndexList& LightDeltaIndex ()
		{ return MineData ().lightDeltaIndices; }
	inline lightDeltaValueList& LightDeltaValues ()
		{ return MineData ().lightDeltaValues; }
	inline flickeringLightList& FlickeringLights ()
		{ return MineData ().flickeringLights; }
	//inline textureList& Textures ()
	//	{ return textures; }

	inline CColor *VertexColors (int i)
		{ return &(MineData ().vertexColors [i]); }

	inline CWall *Walls (int i)
		{ return MineData ().walls + i; }

	inline CTrigger *Triggers (int i)
		{ return MineData ().triggers + i; }
	inline int &NumTriggers ()
		{ return MineInfo ().triggers.count; }
	inline CTrigger *ObjTriggers (int i)
		{ return MineData ().objTriggers + i; }
	inline int& NumObjTriggers ()
		{ return MineData ().numObjTriggers; }
	inline CReactorTrigger *ReactorTriggers (int i)
		{ return MineData ().reactorTriggers + i; }

	inline CGameObject *Objects (int i)
		{ return MineData ().objects + i; }
	inline CRobotMaker *BotGens (int i)
		{ return MineData ().robotMakers + i; }
	inline CRobotMaker *EquipGens (int i)
		{ return MineData ().equipMakers + i; }
	inline CActiveDoor *ActiveDoors (int i)
		{ return MineData ().activeDoors + i; }
	inline CRobotInfo *RobotInfo (int i)
		{ return MineData ().robotInfo + i; }
	inline CRobotInfo *DefRobotInfo (int i)
		{ return m_defaultRobotInfo + i; }
	inline CLightDeltaIndex *LightDeltaIndex (int i)
		{ return MineData ().lightDeltaIndices + i; }
	inline CLightDeltaValue *LightDeltaValues (int i)
		{ return MineData ().lightDeltaValues + i; }
	inline CFlickeringLight *FlickeringLights (int i)
		{ return MineData ().flickeringLights + i; }
	//inline CTexture* Textures (int i, int j = 0)
	//	{ return &textureManager.textures [i][j]; }

	inline CGameInfo& MineInfo ()
		{ return MineData ().gameInfo; }
	inline CMineFileInfo& MineFileInfo ()
		{ return MineData ().gameInfo.fileInfo; }
	inline int& ObjCount ()
		{ return MineInfo ().objects.count; }
	inline ushort& VertCount ()
		{ return segmentManager.VertCount (); }
	inline short& FlickerLightCount ()
		{ return MineData ().m_nFlickeringLights; }
	long TotalSize (CMineItemInfo& gii)
		{ return (int) gii.size * (int) gii.count; }
	inline int& ReactorTime ()
		{ return MineData ().m_reactor_time; }
	inline int& ReactorStrength ()
		{ return MineData ().m_reactor_strength; }
	inline int& SecretCubeNum ()
		{ return MineData ().m_secret_cubenum; }
	inline CDoubleMatrix& SecretOrient ()
		{ return MineData ().m_secret_orient; }
	inline CSelection* &Current ()
		{ return MineData ().current; }
	inline CSelection& Current1 ()
		{ return MineData ().current1; }
	inline CSelection& Current2 ()
		{ return MineData ().current2; }
	inline CSelection *Other (void)
		{ return (Current () == &Current2 ()) ? &Current1 () : &Current2 (); }
	inline CColor *TexColors (int i = 0)
		{ return MineData ().texColors + (i & 0x3fff); }
	inline bool& UseTexColors (void)
		{ return m_bUseTexColors; }
	inline void SetTexColor (short nBaseTex, CColor *pc)	{
		if (UseTexColors () && (IsLight (nBaseTex) != -1))
			*TexColors (nBaseTex) = *pc;
		}
	inline CColor *GetTexColor (short nBaseTex, bool bIsTranspWall = false)	
		{ return UseTexColors () && (bIsTranspWall || (IsLight (nBaseTex) != -1)) ? TexColors (nBaseTex) : null; }
	CColor *LightColor (int i = 0, int j = 0, bool bUseTexColors = true);
	inline lightColorList& LightColors ()
		{ return MineData ().lightColors; }
	inline CColor *LightColors (int i, int j = 0)
		{ return &MineData ().lightColors [i * 6 + j]; }
	inline CColor *CurrLightColor ()
		{ return LightColor (Current ()->nSegment, Current ()->nSide); }

	inline int LevelVersion (void) { return m_levelVersion; }
	inline void SetLevelVersion (int levelVersion) { m_levelVersion = levelVersion; }
	inline bool IsD2XLevel (void) { return LevelVersion () >= 9; }
	inline bool IsStdLevel (void) { return LevelVersion () < 9; }
	inline bool LevelOutdated (void) { return LevelVersion () < LEVEL_VERSION; }
	inline void UpdateLevelVersion (void) { SetLevelVersion (LEVEL_VERSION); }
		
	inline int FileType (void) { return m_fileType; }
	inline void SetFileType (int fileType) { m_fileType = fileType; }
	inline bool IsD1File (void) { return m_fileType == RDL_FILE; }
	inline bool IsD2File (void) { return m_fileType != RDL_FILE; }

	byte *LoadDataResource (LPCTSTR pszRes, HGLOBAL& hGlobal, uint& nResSize);
	short LoadDefaultLightAndColor (void);
	bool HasCustomLightMap (void);
	bool HasCustomLightColors (void);

	short Load(const char *filename = null, bool bLoadFromHog = false);
	short Save(const char *filename, bool bSaveToHog = false);
	int WriteColorMap (CFileManager& fColorMap);
	int ReadColorMap (CFileManager& fColorMap);
	void  Default();
	inline LPSTR LevelName (void)
		{ return m_currentLevelName; }
	inline int LevelNameSize (void)
		{ return sizeof m_currentLevelName; }
	inline bool	SplineActive (void)
		{ return m_bSplineActive; }
	inline void SetSplineActive (bool bSplineActive)
		{ m_bSplineActive = bSplineActive; }
	void  DeleteSegment(short delete_segnum = -1);
	void  DeleteSegmentWalls (short nSegment);
	void	MakeObject (CGameObject *objP, char type, short nSegment);
	void	SetObjectData (char type);
	bool	CopyObject (byte new_type, short nSegment = -1);
	void  DeleteObject(short objectNumber = -1);
	void  DeleteUnusedVertices();
	void  DeleteVertex(short nDeletedVert);

	void InitSegment (short segNum);
	bool SplitSegment ();
	bool  AddSegment();
	bool  LinkSegments(short nSegment1, short nSide1, short nSegment2,short nSide2, double margin);
	void  LinkSides(short nSegment1, short nSide1, short nSegment2, short nSide2, tVertMatch match[4]);
	void	CalcSegCenter (CVertex& pos, short nSegment);
	inline CSegment *current.Segment ()
		{ return Segments (Current ()->nSegment); }
	inline CWall *SideWall (int i = 0, int j = 0)
		{ int w = Segments (i)->m_sides [j].m_info.nWall; return (w < 0) ? null : Walls (w); }
	inline CWall *CurrWall ()
		{ int w = current.Side ()->m_info.nWall; return (w < 0) ? null : Walls (w); }
	inline CSide *current.Side ()
		{ return current.Segment ()->m_sides + Current ()->nSide; }
	inline short CurrVert ()
		{ return current.Segment ()->m_info.verts [sideVertTable [Current ()->nSide][Current ()->nPoint]]; }
	inline CGameObject *CurrObj ()
		{ return Objects (Current ()->nObject); }
	void Mark ();
	void MarkAll ();
	void UnmarkAll ();
	void MarkSegment (short nSegment);
	void UpdateMarkedCubes ();
	bool SideIsMarked (short nSegment, short nSide);
	bool SegmentIsMarked (short nSegment);

	bool IsPointOfSide (CSegment *segP, int nSide, int pointnum);
	bool IsLineOfSide (CSegment *segP, int nSide, int linenum);

	void JoinSegments(int automatic = 0);
	void JoinLines();
	void JoinPoints();
	void SplitSegments(int solidify = 0, int nSide = -1);
	void SplitLines();
	void SplitPoints();

	CDoubleVector CalcSideNormal (short nSegment = -1, short nSide = -1);
	CDoubleVector CalcSideCenter (short nSegment = -1, short nSide = -1);
	//double CalcLength (CFixVector* center1, CFixVector* center2);

	int IsLight (int nBaseTex);
	int IsWall (short nSegment = -1, short nSide = -1);
	bool IsLava (int nBaseTex);
	bool IsBlastableLight (int nBaseTex);
	bool IsFlickeringLight (short nSegment, short nSide);
	bool CalcDeltaLights (double fLightScale, int force, int recursion_depth);
	void CalcDeltaLightData (double fLightScale = 1.0, int force = 1);
	int FindDeltaLight (short nSegment, short nSide, short *pi = null);
	byte LightWeight(short nBaseTex);
	short GetFlickeringLight (short nSegment = -1, short nSide = -1);
	short AddFlickeringLight (short nSegment = -1, short nSide = -1, uint mask = 0xAAAAAAAA, int time = 0x10000 / 4);
	bool DeleteFlickeringLight (short nSegment = -1, short nSide = -1);
	int IsExplodingLight(int nBaseTex);
	bool VisibleWall (ushort nWall);
	void SetCubeLight (double fLight, bool bAll = false, bool bDynCubeLights = false);
	void ScaleCornerLight (double fLight, bool bAll = false);
	void CalcAverageCornerLight (bool bAll = false);
	void AutoAdjustLight (double fBrightness, bool bAll = false, bool bCopyTexLights = false);
	void BlendColors (CColor *psc, CColor *pdc, double srcBr, double destBr);
	void Illuminate (short nSrcSide, short nSrcSeg, uint brightness, 
						  double fLightScale, bool bAll = false, bool bCopyTexLights = false);
	void IlluminateSide (CSegment* segP, short nSide, uint brightness, CColor* lightColorP, double* effect, double fLightScale);
	bool CalcSideLights (int nSegment, int nSide, CVertex& source_center, 
								CVertex* sourceCorner, CVertex& A, double *effect,
								double fLightScale, bool bIgnoreAngle);

	void FixChildren();
	void SetLinesToDraw ();

	short	MarkedSegmentCount (bool bCheck = false);
	bool	GotMarkedSegments (void)
		{ return MarkedSegmentCount (true) > 0; }
	bool GotMarkedSides ();

	inline void SetSelectMode (short mode)
		{ m_selectMode = mode; }
	int ScrollSpeed (ushort texture,int *x,int *y);
	int AlignTextures (short start_segment, short start_side, short only_child, BOOL bAlign1st, BOOL bAlign2nd, char bAlignedSides = 0);

	bool EditGeoFwd (void);
	bool EditGeoBack (void);
	bool EditGeoUp (void);
	bool EditGeoDown (void); 
	bool EditGeoLeft (void); 
	bool EditGeoRight (void); 
	bool EditGeoRotLeft (void); 
	bool EditGeoRotRight (void); 
	bool EditGeoGrow (void); 
	bool EditGeoShrink (void); 
	bool RotateSelection(double angle, bool perpendicular); 
	bool ResizeItem (double delta); 
	bool MovePoints (int pt0, int pt1); 
	bool ResizeLine (CSegment *segP, int point0, int point1, double delta); 
	bool MoveOn (CDoubleVector delta); 
	bool SpinSelection (double angle); 
	//void SetUV (short segment, short side, short x, short y, double angle);
	void LoadSideTextures (short segNum, short sideNum);

	// trigger stuff
	short FindObjBySig (short nSignature);

	void DrawObject (CWnd *pWnd, int type, int id);
	void ConvertWallNum (ushort wNumOld, ushort wNumNew);

	bool GetOppositeSide (short& nOppSeg, short& nOppSide, short nSegment = -1, short nSide = -1);
	bool GetOppositeWall (short &nOppWall, short nSegment = -1, short nSide = -1);
	CSide *OppSide ();
	bool SetTexture (short nSegment, short nSide, short nTexture, short tmapnum2);
	void CopyOtherCube ();
	bool WallClipFromTexture (short nSegment, short nSide);
	void CheckForDoor (short nSegment, short nSide);
	void RenumberBotGens ();
	void RenumberEquipGens ();

	bool SetDefaultTexture (short nTexture = -1, short walltype = -1);
	bool DefineSegment (short nSegment, byte type, short nTexture, short walltype = -1);
	void UndefineSegment (short nSegment);
	bool GetTriggerResources (ushort& nWall);
	int FuelCenterCount (void);
	inline int& RobotMakerCount (void) 
		{ return MineInfo ().botgen.count; }
	inline int& EquipMakerCount (void) 
		{ return MineInfo ().equipgen.count; }
	inline int& WallCount (void) 
		{ return MineInfo ().walls.count; }
	inline int& TriggerCount (void) 
		{ return MineInfo ().triggers.count; }
	inline int& ObjectCount (void) 
		{ return MineInfo ().objects.count; }

	void InitRobotData();
	int WriteHxmFile (CFileManager& fp);
	int ReadHxmFile (CFileManager& fp, long fSize);

	short ReadSegmentInfo (CFileManager& file);
	void WriteSegmentInfo (CFileManager& file, short /*nSegment*/);
	void CutBlock ();
	void CopyBlock (char *pszBlkFile = null);
	void PasteBlock (); 
	int ReadBlock (char *name,int option); 
	void QuickPasteBlock  ();
	void DeleteBlock ();

	inline void wrap (short *x, short delta,short min,short max) {
		*x += delta;
		if (*x > max)
			*x = min;
		else if (*x < min)
			*x = max;
		}

	void TunnelGenerator ();
	void IncreaseSpline ();
	void DecreaseSpline ();
	void CalcSpline ();
	void UntwistSegment (short nSegment,short nSide);
	int MatchingSide (int j);

	void SortObjects ();
	void RenumberTriggerTargetObjs (void);
	void RenumberObjTriggers (void);
	void QSortObjects (short left, short right);
	int QCmpObjects (CGameObject *pi, CGameObject *pm);
	int QCmpObjTriggers (CTrigger *pi, CTrigger *pm);
	void QSortObjTriggers (short left, short right);
	void SortObjTriggers (void);
	bool IsCustomRobot (int i);
	BOOL HasCustomRobots();
	short LoadMineSigAndType (CFileManager& fp);
	void LoadPaletteName (CFileManager& fp, bool bNewMine = false);

private:
	short CreateNewLevel ();
	void DefineVertices(short new_verts[4]);
	void UnlinkChild(short parent_segnum,short nSide);
	short FixIndexValues ();
	void ResetSide (short nSegment,short nSide);

	int ReadHamFile(char *fname = null, int type = NORMAL_HAM);
	void ReadPigTextureTable();
	void ReadRobotResource(int robot_number);
	void ReadColor (CColor *pc, CFileManager& fp);
	void SaveColor (CColor *pc, CFileManager& fp);
	void LoadColors (CColor *pc, int nColors, int nFirstVersion, int nNewVersion, CFileManager& fp);
	void SaveColors (CColor *pc, int nColors, CFileManager& fp);
	void ClearGameItem (CGameItem* items, int nCount);
	int LoadGameItem (CFileManager& fp, CMineItemInfo info, CGameItem* items, int nMinVersion,int nMaxCount, char *pszItem, bool bFlag = false);
	int SaveGameItem (CFileManager& fp, CMineItemInfo& info, CGameItem* items, bool bFlag = false);
	short LoadMineDataCompiled (CFileManager& fp, bool bNewMine);
	short LoadMine (char *filename, bool bLoadFromHog, bool bNewMine);
	short LoadGameData(CFileManager& loadfile, bool bNewMine);
	short SaveMineDataCompiled(CFileManager& fp);
	short SaveGameData(CFileManager& savefile);
	void ClearMineData();
	void UpdateDeltaLights ();
	void SortDLIndex (int left, int right);
	};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define MAX_TEXTURES ((theMine == null) ? MAX_TEXTURES_D2 : theMine->IsD1File () ? MAX_TEXTURES_D1 : MAX_TEXTURES_D2)
#define MAX_OBJECTS ((theMine == null) ? MAX_OBJECTS_D2 : theMine->IsStdLevel () ? MAX_OBJECTS_D1 : MAX_OBJECTS_D2)
#define MAX_NUM_FUELCENS ((theMine == null) ? MAX_NUM_FUELCENS_D2X : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_NUM_FUELCENS_D2 : MAX_NUM_FUELCENS_D2X)
#define MAX_NUM_REPAIRCENS ((theMine == null) ? MAX_NUM_REPAIRCENS_D2X : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_NUM_REPAIRCENS_D2 : MAX_NUM_REPAIRCENS_D2X)
#define MAX_PLAYERS ((theMine == null) ? MAX_PLAYERS_D2 : theMine->IsStdLevel () ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X)
#define ROBOT_IDS2 ((theMine == null) ? MAX_ROBOT_IDS_TOTAL : (theMine->LevelVersion () == 7) ? N_ROBOT_TYPES_D2 : MAX_ROBOT_IDS_TOTAL)
#define MAX_ROBOT_MAKERS ((theMine == null) ? MAX_NUM_MATCENS_D2 : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_NUM_MATCENS_D1 : MAX_NUM_MATCENS_D2)
#define MAX_LIGHT_DELTA_INDICES ((theMine == null) ? MAX_LIGHT_DELTA_INDICES_STD : (theMine->IsD1File () || theMine->IsStdLevel ()) ? MAX_LIGHT_DELTA_INDICES_STD : MAX_LIGHT_DELTA_INDICES_D2X)
#define MAX_LIGHT_DELTA_VALUES ((theMine == null) ? MAX_LIGHT_DELTA_VALUES_STD : (theMine->IsD1File () || theMine->IsStdLevel ()) ? MAX_LIGHT_DELTA_VALUES_STD : MAX_LIGHT_DELTA_VALUES_D2X)

#define NO_WALL MAX_WALLS

extern CMine* theMine;

#define CHECKMINE			if ((theMine == null)) return;
#define CHECKMINEV(_v)	if ((theMine == null)) return (_v);
#define CHECKMINEF		CHECKMINE(FALSE);

#endif //__mine_h