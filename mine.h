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

#define USE_DYN_ARRAYS _DEBUG

#if USE_DYN_ARRAYS

typedef CStaticArray< CRobotInfo, MAX_ROBOT_TYPES > robotInfoList;
typedef CStaticArray< CVertex, MAX_VERTICES3 > vertexList;
typedef CStaticArray< CSegment, MAX_SEGMENTS3 > segmentList;
typedef CStaticArray< CColor, MAX_SEGMENTS3 * 6 > lightColorList;
typedef CStaticArray< CColor, MAX_D2_TEXTURES > texColorList;
typedef CStaticArray< CColor, MAX_VERTICES3 > vertexColorList;
typedef CStaticArray< CWall, MAX_WALLS3 > wallList;
typedef CStaticArray< CActiveDoor, MAX_DOORS > activeDoorList;
typedef CStaticArray< CTrigger, MAX_TRIGGERS2 > triggerList;
typedef CStaticArray< CTrigger, MAX_OBJ_TRIGGERS > objTriggerList;
typedef CStaticArray< CReactorTrigger, MAX_REACTOR_TRIGGERS > reactorTriggerList;
typedef CStaticArray< CRobotMaker, MAX_NUM_MATCENS2 > robotMakerList;
typedef CStaticArray< CGameObject, MAX_OBJECTS2 > objectList;
typedef CStaticArray< CLightDeltaIndex, MAX_LIGHT_DELTA_INDICES_D2X > lightDeltaIndexList;
typedef CStaticArray< CLightDeltaValue, MAX_LIGHT_DELTA_VALUES_D2X > lightDeltaValueList;
typedef CStaticArray< CFlickeringLight, MAX_FLICKERING_LIGHTS > flickeringLightList;
typedef CStaticArray< CStaticArray< CTexture, MAX_D2_TEXTURES>, 2> textureList;

#define CLEAR(_b) (_b) [0].Reset((_b).Length ())
#define ASSIGN(_a,_b) (_a) = (_b)
#define DATA(_b) (_b).Buffer ()

#else

typedef CRobotInfo robotInfoList [MAX_ROBOT_TYPES];
typedef CVertex vertexList [MAX_VERTICES3];
typedef CSegment segmentList [MAX_SEGMENTS3];
typedef CColor lightColorList [MAX_SEGMENTS3 * 6];
typedef CColor texColorList [MAX_D2_TEXTURES];
typedef CColor vertexColorList [MAX_VERTICES3];
typedef CWall wallList [MAX_WALLS3];
typedef CActiveDoor activeDoorList [MAX_DOORS];
typedef CTrigger triggerList [MAX_TRIGGERS2];
typedef CTrigger objTriggerList [MAX_OBJ_TRIGGERS];
typedef CReactorTrigger reactorTriggerList [MAX_REACTOR_TRIGGERS];
typedef CRobotMaker robotMakerList [MAX_NUM_MATCENS2];
typedef CGameObject objectList [MAX_OBJECTS2];
typedef CLightDeltaIndex lightDeltaIndexList [MAX_LIGHT_DELTA_INDICES_D2X];
typedef CLightDeltaValue lightDeltaValueList [MAX_LIGHT_DELTA_VALUES_D2X];
typedef CFlickeringLight flickeringLightList [MAX_FLICKERING_LIGHTS];
typedef CTexture textureList [2][MAX_D2_TEXTURES];

#define CLEAR(_b)	(_b)->Reset (sizeof (_b) / sizeof (_b [0]))
#define ASSIGN(_a,_b) memcpy (_a, _b, sizeof (_a))
#define DATA(_b) (_b)

#endif

typedef struct tMineData {
	CGameInfo					gameInfo;
	
	int							m_reactor_time;
	int							m_reactor_strength;
	int							m_secret_cubenum;
	CDoubleMatrix				m_secret_orient;
	
	// robot data
	robotInfoList				robotInfo;
	
	// structure data
	ushort						numVertices;
	vertexList					vertices;
	ushort						numSegments;
	segmentList					segments;
	lightColorList				lightColors;
	texColorList				texColors;
	vertexColorList			vertexColors;
	wallList						walls;
	activeDoorList				activeDoors;
	triggerList					triggers;
	objTriggerList				objTriggers;
	int							numObjTriggers;
	reactorTriggerList		reactorTriggers;
	robotMakerList				robotMakers;
	robotMakerList				equipMakers;
	objectList					objects;
	lightDeltaIndexList		lightDeltaIndices;
	lightDeltaValueList		lightDeltaValues;
	short							m_nFlickeringLights;
	flickeringLightList		flickeringLights;

	//CRobotInfo				robotInfo [MAX_ROBOT_TYPES];
	//CVertex					vertices [MAX_VERTICES3];
	//CSegment					segments [MAX_SEGMENTS3];
	//CColor 					lightColors [MAX_SEGMENTS3][6];
	//CColor						texColors [MAX_D2_TEXTURES];
	//CColor						vertexColors [MAX_VERTICES3];
	//CWall						walls[MAX_WALLS3];
	//CActiveDoor				activeDoors[MAX_DOORS];
	//CTrigger					triggers[MAX_TRIGGERS2];
	//CTrigger					objTriggers[MAX_OBJ_TRIGGERS];
	//CReactorTrigger			reactorTriggers[MAX_REACTOR_TRIGGERS];
	//CRobotMaker				robotMakers[MAX_NUM_MATCENS2];
	//CRobotMaker				equipMakers[MAX_NUM_MATCENS2];
	//CGameObject				objects[MAX_OBJECTS2];
	//CLightDeltaIndex		lightDeltaIndices [MAX_LIGHT_DELTA_INDICES_D2X];
	//CLightDeltaValue		lightDeltaValues [MAX_LIGHT_DELTA_VALUES_D2X];
	//CFlickeringLight		flickeringLights[MAX_FLICKERING_LIGHTS];

	CSelection					current1;
	CSelection					current2;
	CSelection					*current;

} MINE_DATA;

class CMine {
public:
	
	// level info
	int				m_fileType;
	int				m_levelVersion;
	char				m_currentLevelName [256];	
	CGameFileInfo	gameFileInfo;
	MINE_DATA		m_mineData;
	robotInfoList	m_defaultRobotInfo;
	HPALETTE			m_paletteHandle;
	CPalette*		m_currentPalette;
	LPLOGPALETTE	m_dlcLogPalette;
	textureList		textures;
	
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
		{ return MineData ().vertices; }
	inline segmentList& Segments (void)
		{ return MineData ().segments; }
	inline vertexColorList& VertexColors (void)
		{ return MineData ().vertexColors; }
	inline wallList& Walls (void)
		{ return MineData ().walls; }
	inline triggerList& Triggers (void)
		{ return MineData ().triggers; }
	inline objTriggerList& ObjTriggers (void)
		{ return MineData ().objTriggers; }
	inline objectList& Objects (void)
		{ return MineData ().objects; }
	inline robotMakerList& BotGens (void)
		{ return MineData ().robotMakers; }
	inline robotMakerList& EquipGens (void)
		{ return MineData ().equipMakers; }
	inline reactorTriggerList& ReactorTriggers (void)
		{ return MineData ().reactorTriggers; }
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
	inline textureList& Textures ()
		{ return textures; }

	inline CVertex *Vertices (int i)
		{ return MineData ().vertices + i; }
	inline byte& VertStatus (int i = 0)
		{ return Vertices (i)->m_status; }
	inline CSegment *Segments (int i)
		{ return MineData ().segments + i; }
	inline CColor *VertexColors (int i)
		{ return &(MineData ().vertexColors [i]); }
	inline CWall *Walls (int i)
		{ return MineData ().walls + i; }
	inline CTrigger *Triggers (int i)
		{ return MineData ().triggers + i; }
	inline int &NumTriggers ()
		{ return GameInfo ().triggers.count; }
	inline CTrigger *ObjTriggers (int i)
		{ return MineData ().objTriggers + i; }
	inline int& NumObjTriggers ()
		{ return MineData ().numObjTriggers; }
	inline CGameObject *Objects (int i)
		{ return MineData ().objects + i; }
	inline CRobotMaker *BotGens (int i)
		{ return MineData ().robotMakers + i; }
	inline CRobotMaker *EquipGens (int i)
		{ return MineData ().equipMakers + i; }
	inline CReactorTrigger *ReactorTriggers (int i)
		{ return MineData ().reactorTriggers + i; }
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
	inline CTexture* Textures (int i, int j = 0)
		{ return &textures [i][j]; }

	inline CGameInfo& GameInfo ()
		{ return MineData ().gameInfo; }
	inline ushort& SegCount ()
		{ return MineData ().numSegments; }
	inline int& ObjCount ()
		{ return GameInfo ().objects.count; }
	inline ushort& VertCount ()
		{ return MineData ().numVertices; }
	inline short& FlickerLightCount ()
		{ return MineData ().m_nFlickeringLights; }
	long TotalSize (CGameItemInfo& gii)
		{ return (fix) gii.size * (fix) gii.count; }
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
		{ return UseTexColors () && (bIsTranspWall || (IsLight (nBaseTex) != -1)) ? TexColors (nBaseTex) : NULL; }
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
	BOOL HasCustomLightMap (void);
	BOOL HasCustomLightColors (void);

	short Load(const char *filename = NULL, bool bLoadFromHog = false);
	short Save(const char *filename, bool bSaveToHog = false);
	int WriteColorMap (FILE *fColorMap);
	int ReadColorMap (FILE *fColorMap);
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
	inline CSegment *CurrSeg ()
		{ return Segments () + Current ()->nSegment; }
	inline CWall *SideWall (int i = 0, int j = 0)
		{ int w = Segments (i)->m_sides [j].m_info.nWall; return (w < 0) ? NULL : Walls (w); }
	inline CWall *CurrWall ()
		{ int w = CurrSide ()->m_info.nWall; return (w < 0) ? NULL : Walls (w); }
	inline CSide *CurrSide ()
		{ return CurrSeg ()->m_sides + Current ()->nSide; }
	inline short CurrVert ()
		{ return CurrSeg ()->m_info.verts [sideVertTable [Current ()->nSide][Current ()->nPoint]]; }
	inline CGameObject *CurrObj ()
		{ return Objects () + Current ()->nObject; }
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

	int IsLight(int nBaseTex);
	int IsWall (short nSegment = -1, short nSide = -1);
	bool IsLava (int nBaseTex);
	bool IsBlastableLight (int nBaseTex);
	bool IsFlickeringLight (short nSegment, short nSide);
	bool CalcDeltaLights (double fLightScale, int force, int recursion_depth);
	void CalcDeltaLightData (double fLightScale = 1.0, int force = 1);
	int FindDeltaLight (short nSegment, short nSide, short *pi = NULL);
	byte LightWeight(short nBaseTex);
	short GetFlickeringLight (short nSegment = -1, short nSide = -1);
	short AddFlickeringLight (short nSegment = -1, short nSide = -1, uint mask = 0xAAAAAAAA, fix time = 0x10000 / 4);
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
	bool CMine::GotMarkedSides ();

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

	CWall *AddWall (short nSegment, short nSide, short type, ushort flags, byte keys, char nClip, short nTexture);
	CWall *GetWall (short nSegment = -1, short nSide = -1);
	void DeleteWall (ushort nWall = -1);
	CWall *FindWall (short nSegment = -1, short nSide = -1);
	void DefineWall (short nSegment, short nSide, ushort nWall,
						  byte type, char nClip, short nTexture,
						  bool bRedefine);
	void SetWallTextures (ushort nWall, short nTexture = 0);
	// trigger stuff
	void InitTrigger (CTrigger *t, short type, short flags);
	CTrigger *AddTrigger (ushort nWall, short type, BOOL bAutoAddWall = FALSE);
	void DeleteTrigger (short nTrigger = -1);
	bool DeleteTriggerTarget (CTrigger *trigger, short nSegment, short nSide, bool bAutoDeleteTrigger = true);
	void DeleteTriggerTargets (short nSegment, short nSide);
	int DeleteTargetFromTrigger (CTrigger *trigger, short linknum, bool bAutoDeleteTrigger = true);
	int DeleteTargetFromTrigger (short nTrigger, short linknum, bool bAutoDeleteTrigger = true);
	short FindTriggerWall (short *nTrigger, short nSegment = -1, short nSide = -1);
	short FindTriggerWall (short nTrigger);
	short FindTriggerObject (short *nTrigger);
	short FindTriggerTarget (short nTrigger, short nSegment, short nSide);
	CTrigger *AddObjTrigger (short objnum, short type);
	bool ObjTriggerIsInList (short nTrigger);
	void DeleteObjTrigger (short objnum);
	void DeleteObjTriggers (short objnum);
	short FindObjTriggerTarget (short nTrigger, short nSegment, short nSide);
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
	bool AutoAddTrigger (short wall_type, ushort wall_flags, ushort trigger_type);
	bool AddDoorTrigger (short wall_type, ushort wall_flags, ushort trigger_type);
	bool AddOpenDoorTrigger(); 
	bool AddRobotMakerTrigger (); 
	bool AddShieldTrigger(); 
	bool AddEnergyTrigger(); 
	bool AddUnlockTrigger(); 
	bool AddExit (short type); 
	bool AddNormalExit(); 
	bool AddSecretExit(); 
	bool AddDoor (byte type, byte flags, byte keys, char nClip, short nTexture); 
	bool AddAutoDoor (char nClip = -1, short nTexture = -1); 
	bool AddPrisonDoor (); 
	bool AddGuideBotDoor(); 
	bool AddFuelCell (); 
	bool AddIllusionaryWall (); 
	bool AddForceField (); 
	bool AddFan ();
	bool AddWaterFall ();
	bool AddLavaFall(); 
	bool AddGrate(); 
	bool AddReactor (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddRobotMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddEquipMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddFuelCenter (short nSegment = -1, byte nType = SEGMENT_FUNC_FUELCEN, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddGoalCube (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture);
	bool AddTeamCube (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture);
	bool AddSpeedBoostCube (short nSegment, bool bCreate);
	bool AddSkyboxCube (short nSegment, bool bCreate);
	void AutoLinkExitToReactor ();
	int FuelCenterCount (void);
	inline int& RobotMakerCount (void) 
		{ return GameInfo ().botgen.count; }
	inline int& EquipMakerCount (void) 
		{ return GameInfo ().equipgen.count; }
	inline int& WallCount (void) 
		{ return GameInfo ().walls.count; }
	inline int& TriggerCount (void) 
		{ return GameInfo ().triggers.count; }
	inline int& ObjectCount (void) 
		{ return GameInfo ().objects.count; }

	inline CSegment *OtherSeg (void)
		{ return Segments () + Other ()->nSegment; }
	inline CSide *OtherSide (void)
		{ return OtherSeg ()->m_sides + Other ()->nSide; }
	inline void SetCurrent (short nSegment = -1, short nSide = -1, short nLine = -1, short nPoint = -1) {
		if (nSegment >= 0) Current ()->nSegment = nSegment;
		if (nSide >= 0) Current ()->nSide = nSide;
		if (nLine >= 0) Current ()->nLine = nLine;
		if (nPoint >= 0) Current ()->nPoint = nPoint;
		}
	inline void GetCurrent (short &nSegment, short& nSide) {
		if (nSegment < 0) nSegment = Current ()->nSegment;
		if (nSide < 0) nSide = Current ()->nSide;
		}

	void InitRobotData();
	int WriteHxmFile (FILE *fp);
	int ReadHxmFile (FILE *fp, long fSize);

	short ReadSegmentInfo (FILE *file);
	void WriteSegmentInfo (FILE *file, short /*nSegment*/);
	void CutBlock ();
	void CopyBlock (char *pszBlkFile = NULL);
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
	short LoadPalette (void);

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
	short LoadMineSigAndType (FILE* fp);

private:
	int FindClip (CWall *wallP, short nTexture);
	short CreateNewLevel ();
	void DefineVertices(short new_verts[4]);
	void UnlinkChild(short parent_segnum,short nSide);
	short FixIndexValues();
	void ResetSide (short nSegment,short nSide);

	int ReadHamFile(char *fname = NULL, int type = NORMAL_HAM);
	void ReadPigTextureTable();
	void ReadRobotResource(int robot_number);
	void ReadColor (CColor *pc, FILE *load_file);
	void SaveColor (CColor *pc, FILE *save_file);
	void LoadColors (CColor *pc, int nColors, int nFirstVersion, int nNewVersion, FILE *fp);
	void SaveColors (CColor *pc, int nColors, FILE *fp);
	void ClearGameItem (CGameItem* items, int nCount);
	int LoadGameItem (FILE* fp, CGameItemInfo info, CGameItem* items, int nMinVersion,int nMaxCount, char *pszItem, bool bFlag = false);
	int SaveGameItem (FILE* fp, CGameItemInfo& info, CGameItem* items, bool bFlag = false);
	short LoadMineDataCompiled (FILE *load_file, bool bNewMine);
	short LoadMine (char *filename, bool bLoadFromHog, bool bNewMine);
	short LoadGameData(FILE *loadfile, bool bNewMine);
	short SaveMineDataCompiled(FILE *save_file);
	short SaveGameData(FILE *savefile);
	void ClearMineData();
	void UpdateDeltaLights ();
	void SetSegmentChildNum(CSegment *pRoot, short nSegment,short recursion_level);
	void SetSegmentChildNum (CSegment *pRoot, short nSegment, short recursion_level, short* visited);
	void UnlinkSeg (CSegment *pSegment, CSegment *pRoot);
	void LinkSeg (CSegment *pSegment, CSegment *pRoot);
	void SortDLIndex (int left, int right);
	};

#define MAX_SEGMENTS (!theMine ? MAX_SEGMENTS2 : theMine->IsD1File () ? MAX_SEGMENTS1  : theMine->IsStdLevel () ? MAX_SEGMENTS2 : MAX_SEGMENTS3)
#define MAX_VERTICES (!theMine ? MAX_VERTICES2 : theMine->IsD1File () ? MAX_VERTICES1 : theMine->IsStdLevel () ? MAX_VERTICES2 : MAX_VERTICES3)
#define MAX_WALLS (!theMine ? MAX_WALLS2 : theMine->IsD1File () ? MAX_WALLS1 : (theMine->LevelVersion () < 12) ? MAX_WALLS2 : MAX_WALLS3)
#define MAX_TEXTURES (!theMine ? MAX_D2_TEXTURES : theMine->IsD1File () ? MAX_D1_TEXTURES : MAX_D2_TEXTURES)
#define MAX_TRIGGERS (!theMine ? MAX_TRIGGERS2 : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_TRIGGERS1 : MAX_TRIGGERS2)
#define MAX_OBJECTS (!theMine ? MAX_OBJECTS2 : theMine->IsStdLevel () ? MAX_OBJECTS1 : MAX_OBJECTS2)
#define MAX_NUM_FUELCENS (!theMine ? MAX_NUM_FUELCENS2 : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_NUM_FUELCENS1 : MAX_NUM_FUELCENS2)
#define MAX_NUM_REPAIRCENS (!theMine ? MAX_NUM_REPAIRCENS2 : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_NUM_REPAIRCENS1 : MAX_NUM_REPAIRCENS2)
#define MAX_PLAYERS (!theMine ? MAX_PLAYERS_D2 : theMine->IsStdLevel () ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X)
#define ROBOT_IDS2 (!theMine ? MAX_ROBOT_IDS_TOTAL : (theMine->LevelVersion () == 7) ? N_D2_ROBOT_TYPES : MAX_ROBOT_IDS_TOTAL)
#define MAX_ROBOT_MAKERS (!theMine ? MAX_NUM_MATCENS2 : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_NUM_MATCENS1 : MAX_NUM_MATCENS2)
#define MAX_LIGHT_DELTA_INDICES (!theMine ? MAX_LIGHT_DELTA_INDICES_STD : (theMine->IsD1File () || theMine->IsStdLevel ()) ? MAX_LIGHT_DELTA_INDICES_STD : MAX_LIGHT_DELTA_INDICES_D2X)
#define MAX_LIGHT_DELTA_VALUES (!theMine ? MAX_LIGHT_DELTA_VALUES_STD : (theMine->IsD1File () || theMine->IsStdLevel ()) ? MAX_LIGHT_DELTA_VALUES_STD : MAX_LIGHT_DELTA_VALUES_D2X)

#define NO_WALL MAX_WALLS

extern CMine* theMine;

#define CHECKMINE			if (!theMine) return;
#define CHECKMINEV(_v)	if (!theMine) return (_v);
#define CHECKMINEF		CHECKMINE(FALSE);

#endif //__mine_h