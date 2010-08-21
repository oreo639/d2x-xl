#ifndef __mine_h
#define __mine_h

#include "carray.h"
#include "types.h"
#include "segment.h"

#define MAX_LIGHT_DEPTH 6

// external data
extern UINT8 side_vert[6][4];
extern UINT8 opp_side[6];
extern UINT8 opp_side_vert[6][4];
extern UINT8 line_vert[12][2];
extern UINT8 side_line[6][4];
extern UINT8 connect_points[8][3];
extern INT8 point_sides[8][3];
extern INT8 point_corners[8][3];
extern TEXTURE_LIGHT d1_texture_light[NUM_LIGHTS_D1];
extern TEXTURE_LIGHT d2_texture_light[NUM_LIGHTS_D2];
extern CUVL default_uvls[4];

// Copyright (C) 1997 Bryan Aamot
//**************************************************************************
// CLASS - Level
//**************************************************************************

#if 0
typedef CStaticArray< ROBOT_INFO, MAX_ROBOT_TYPES > robotInfoList;
typedef CStaticArray< CVertex, MAX_VERTICES3 > vertexList;
typedef CStaticArray< CSegment, MAX_SEGMENTS3 > segmentList;
typedef CStaticArray< typedef CStaticArray< CColor, 6 >, MAX_SEGMENTS3 > lightColorList;
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
#else
typedef ROBOT_INFO robotInfoList [MAX_ROBOT_TYPES];
typedef CVertex vertexList [MAX_VERTICES3];
typedef CSegment segmentList [MAX_SEGMENTS3];
typedef CColor lightColorList [MAX_SEGMENTS3][6];
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
#endif

typedef struct tMineData {
	CGameInfo					gameInfo;
	
	INT32							m_reactor_time;
	INT32							m_reactor_strength;
	INT32							m_secret_cubenum;
	CFixMatrix					m_secret_orient;
	
	// robot data
	//ROBOT_INFO				robotInfo [MAX_ROBOT_TYPES];
	robotInfoList				robotInfo;
	
	// structure data
	UINT16						numVertices;
	//CVertex					vertices [MAX_VERTICES3];
	vertexList					vertices;
	
	UINT16						numSegments;
	//CSegment					segments [MAX_SEGMENTS3];
	segmentList					segments;
	//CColor 					lightColors [MAX_SEGMENTS3][6];
	lightColorList				lightColors;
	//CColor						texColors [MAX_D2_TEXTURES];
	texColorList				texColors;
	//CColor						vertexColors [MAX_VERTICES3];
	vertexColorList			vertexColors;
	
	//CWall						walls[MAX_WALLS3];
	wallList						walls;
	//CActiveDoor				activeDoors[MAX_DOORS];
	activeDoorList				activeDoors;
	//CTrigger					triggers[MAX_TRIGGERS2];
	triggerList					triggers;
	//CTrigger					objTriggers[MAX_OBJ_TRIGGERS];
	objTriggerList				objTriggers;
	INT32							numObjTriggers;
	//CReactorTrigger			reactorTriggers[MAX_REACTOR_TRIGGERS];
	reactorTriggerList		reactorTriggers;
	//CRobotMaker				robotMakers[MAX_NUM_MATCENS2];
	robotMakerList				robotMakers;
	//CRobotMaker				equipMakers[MAX_NUM_MATCENS2];
	robotMakerList				equipMakers;
	
	// object data
	//CGameObject				objects[MAX_OBJECTS2];
	objectList					objects;
	
	// light data
	//CLightDeltaIndex		lightDeltaIndices [MAX_LIGHT_DELTA_INDICES_D2X];
	lightDeltaIndexList		lightDeltaIndices;
	//CLightDeltaValue		lightDeltaValues [MAX_LIGHT_DELTA_VALUES_D2X];
	lightDeltaValueList		lightDeltaValues;
	
	// flickering light
	INT16							m_nFlickeringLights;
	//CFlickeringLight		flickeringLights[MAX_FLICKERING_LIGHTS];
	flickeringLightList		flickeringLights;

	CSelection					current1;
	CSelection					current2;
	CSelection					*current;

} MINE_DATA;

class CMine {
public:
	
	// level info
	INT32							m_fileType;
	INT32							m_levelVersion;
	char							m_currentLevelName [256];	
	CGameFileInfo				gameFileInfo;
	MINE_DATA					m_mineData;
	//ROBOT_INFO				m_defaultRobotInfo [MAX_ROBOT_TYPES];
	robotInfoList				m_defaultRobotInfo;
	// textures and palettes
//	HGLOBAL						texture_handle[MAX_D2_TEXTURES];
	HPALETTE						m_paletteHandle;
	CPalette*					m_currentPalette;
	LPLOGPALETTE				m_dlcLogPalette;
	
	// strings
	char				message[256];
	char				m_startFolder [256];
//	char				descent_path[256];
//	char				descent2_path[256];
//	char				levels_path[256];
	
	// selection
	
	// flags
	INT16				m_selectMode;
	INT32				m_disableDrawing;
	INT32				m_changesMade;
	
	bool				m_bSplineActive;
	BOOL				m_bSortObjects;
	INT32				m_nMaxSplines;
	INT32				m_nNoLightDeltas;
	INT32				m_lightRenderDepth;
	INT32				m_deltaLightRenderDepth;

	char				m_szBlockFile [256];
	INT16				m_splineLength1,
						m_splineLength2;
	bool				m_bVertigo;
	char*				m_pHxmExtraData;
	INT32				m_nHxmExtraDataSize;
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

	inline CVertex *Vertices (INT32 i)
		{ return MineData ().vertices + i; }
	inline UINT8& VertStatus (INT32 i = 0)
		{ return Vertices (i)->m_status; }
	inline CSegment *Segments (INT32 i)
		{ return MineData ().segments + i; }
	inline CColor *VertexColors (INT32 i)
		{ return &(MineData ().vertexColors [i]); }
	inline CWall *Walls (INT32 i)
		{ return MineData ().walls + i; }
	inline CTrigger *Triggers (INT32 i)
		{ return MineData ().triggers + i; }
	inline INT32 &NumTriggers ()
		{ return GameInfo ().triggers.count; }
	inline CTrigger *ObjTriggers (INT32 i)
		{ return MineData ().objTriggers + i; }
	inline INT32& NumObjTriggers ()
		{ return MineData ().numObjTriggers; }
	inline CGameObject *Objects (INT32 i)
		{ return MineData ().objects + i; }
	inline CRobotMaker *BotGens (INT32 i)
		{ return MineData ().robotMakers + i; }
	inline CRobotMaker *EquipGens (INT32 i)
		{ return MineData ().equipMakers + i; }
	inline CReactorTrigger *ReactorTriggers (INT32 i)
		{ return MineData ().reactorTriggers + i; }
	inline CActiveDoor *ActiveDoors (INT32 i)
		{ return MineData ().activeDoors + i; }
	inline ROBOT_INFO *RobotInfo (INT32 i)
		{ return MineData ().robotInfo + i; }
	inline ROBOT_INFO *DefRobotInfo (INT32 i)
		{ return m_defaultRobotInfo + i; }

	inline CGameInfo& GameInfo ()
		{ return MineData ().gameInfo; }
	inline UINT16& SegCount ()
		{ return MineData ().numSegments; }
	inline INT32& ObjCount ()
		{ return GameInfo ().objects.count; }
	inline UINT16& VertCount ()
		{ return MineData ().numVertices; }
	inline CLightDeltaIndex *LightDeltaIndex (INT32 i = 0)
		{ return MineData ().lightDeltaIndices + i; }
	inline CLightDeltaValue *LightDeltaValues (INT32 i = 0)
		{ return MineData ().lightDeltaValues + i; }
	inline CFlickeringLight *FlickeringLights (INT32 i = 0)
		{ return MineData ().flickeringLights + i; }
	inline INT16& FlickerLightCount ()
		{ return MineData ().m_nFlickeringLights; }
	long TotalSize (CGameItemInfo& gii)
		{ return (long) gii.size * (long) gii.count; }
	inline INT32& ReactorTime ()
		{ return MineData ().m_reactor_time; }
	inline INT32& ReactorStrength ()
		{ return MineData ().m_reactor_strength; }
	inline INT32& SecretCubeNum ()
		{ return MineData ().m_secret_cubenum; }
	inline CFixMatrix& SecretOrient ()
		{ return MineData ().m_secret_orient; }
	inline CSelection* &Current ()
		{ return MineData ().current; }
	inline CSelection& Current1 ()
		{ return MineData ().current1; }
	inline CSelection& Current2 ()
		{ return MineData ().current2; }
	inline CSelection *Other (void)
		{ return (Current () == &Current2 ()) ? &Current1 () : &Current2 (); }
	inline CColor *TexColors (INT32 i = 0)
		{ return MineData ().texColors + (i & 0x3fff); }
	inline bool& UseTexColors (void)
		{ return m_bUseTexColors; }
	inline void SetTexColor (INT16 nBaseTex, CColor *pc)	{
		if (UseTexColors () && (IsLight (nBaseTex) != -1))
			*TexColors (nBaseTex) = *pc;
		}
	inline CColor *GetTexColor (INT16 nBaseTex, bool bIsTranspWall = false)	
		{ return UseTexColors () && (bIsTranspWall || (IsLight (nBaseTex) != -1)) ? TexColors (nBaseTex) : NULL; }
	CColor *LightColor (INT32 i = 0, INT32 j = 0, bool bUseTexColors = true);
	inline lightColorList& LightColors ()
		{ return MineData ().lightColors; }
	inline CColor *LightColors (INT32 i, INT32 j = 0)
		{ return MineData ().lightColors [i] + j; }
	inline CColor *CurrLightColor ()
		{ return LightColor (Current ()->nSegment, Current ()->nSide); }

	inline INT32 LevelVersion (void) { return m_levelVersion; }
	inline void SetLevelVersion (INT32 levelVersion) { m_levelVersion = levelVersion; }
	inline bool IsD2XLevel (void) { return LevelVersion () >= 9; }
	inline bool IsStdLevel (void) { return LevelVersion () < 9; }
	inline bool LevelOutdated (void) { return LevelVersion () < LEVEL_VERSION; }
	inline void UpdateLevelVersion (void) { SetLevelVersion (LEVEL_VERSION); }
		
	inline INT32 FileType (void) { return m_fileType; }
	inline void SetFileType (INT32 fileType) { m_fileType = fileType; }
	inline bool IsD1File (void) { return m_fileType == RDL_FILE; }
	inline bool IsD2File (void) { return m_fileType != RDL_FILE; }

	UINT8 *LoadDataResource (LPCTSTR pszRes, HGLOBAL& hGlobal, UINT32& nResSize);
	INT16 LoadDefaultLightAndColor (void);
	BOOL HasCustomLightMap (void);
	BOOL HasCustomLightColors (void);

	INT16 Load(const char *filename = NULL, bool bLoadFromHog = false);
	INT16 Save(const char *filename, bool bSaveToHog = false);
	INT32 WriteColorMap (FILE *fColorMap);
	INT32 ReadColorMap (FILE *fColorMap);
	void  Default();
	inline LPSTR LevelName (void)
		{ return m_currentLevelName; }
	inline INT32 LevelNameSize (void)
		{ return sizeof m_currentLevelName; }
	inline bool	SplineActive (void)
		{ return m_bSplineActive; }
	inline void SetSplineActive (bool bSplineActive)
		{ m_bSplineActive = bSplineActive; }
	void  DeleteSegment(INT16 delete_segnum = -1);
	void  DeleteSegmentWalls (INT16 nSegment);
	void	MakeObject (CGameObject *objP, INT8 type, INT16 nSegment);
	void	SetObjectData (INT8 type);
	bool	CopyObject (UINT8 new_type, INT16 nSegment = -1);
	void  DeleteObject(INT16 objectNumber = -1);
	void  DeleteUnusedVertices();
	void  DeleteVertex(INT16 nDeletedVert);

	void InitSegment (INT16 segNum);
	bool SplitSegment ();
	bool  AddSegment();
	bool  LinkSegments(INT16 segnum1,INT16 sidenum1, INT16 segnum2,INT16 sidenum2, FIX margin);
	void  LinkSides(INT16 segnum1,INT16 sidenum1,INT16 segnum2,INT16 sidenum2, tVertMatch match[4]);
	void	CalcSegCenter(CFixVector& pos,INT16 nSegment);
	inline CSegment *CurrSeg ()
		{ return Segments () + Current ()->nSegment; }
	inline CWall *SideWall (INT32 i = 0, INT32 j = 0)
		{ INT32 w = Segments (i)->sides [j].nWall; return (w < 0) ? NULL : Walls (w); }
	inline CWall *CurrWall ()
		{ INT32 w = CurrSide ()->nWall; return (w < 0) ? NULL : Walls (w); }
	inline CSide *CurrSide ()
		{ return CurrSeg ()->sides + Current ()->nSide; }
	inline INT16 CurrVert ()
		{ return CurrSeg ()->verts [side_vert [Current ()->nSide][Current ()->nPoint]]; }
	inline CGameObject *CurrObj ()
		{ return Objects () + Current ()->nObject; }
	void Mark ();
	void MarkAll ();
	void UnmarkAll ();
	void MarkSegment (INT16 nSegment);
	void UpdateMarkedCubes ();
	bool SideIsMarked (INT16 nSegment, INT16 nSide);
	bool SegmentIsMarked (INT16 nSegment);

	bool IsPointOfSide (CSegment *segP, INT32 nSide, INT32 pointnum);
	bool IsLineOfSide (CSegment *segP, INT32 nSide, INT32 linenum);

	void JoinSegments(INT32 automatic = 0);
	void JoinLines();
	void JoinPoints();
	void SplitSegments(INT32 solidify = 0, INT32 nSide = -1);
	void SplitLines();
	void SplitPoints();

	void CalcOrthoVector (CFixVector& result,INT16 nSegment,INT16 nSide);
	void CalcCenter (CFixVector& center,INT16 nSegment,INT16 nSide);
	double CalcLength (CFixVector* center1, CFixVector* center2);

	INT32 IsLight(INT32 nBaseTex);
	INT32 IsWall (INT16 nSegment = -1, INT16 nSide = -1);
	bool IsLava (INT32 nBaseTex);
	bool IsBlastableLight (INT32 nBaseTex);
	bool IsFlickeringLight (INT16 nSegment, INT16 nSide);
	bool CalcDeltaLights (double fLightScale, INT32 force, INT32 recursion_depth);
	void CalcDeltaLightData (double fLightScale = 1.0, INT32 force = 1);
	INT32 FindDeltaLight (INT16 nSegment, INT16 nSide, INT16 *pi = NULL);
	UINT8 LightWeight(INT16 nBaseTex);
	INT16 GetFlickeringLight(INT16 nSegment = -1, INT16 nSide = -1);
	INT16 AddFlickeringLight( INT16 nSegment = -1, INT16 nSide = -1, UINT32 mask = 0xAAAAAAAA, FIX time = 0x10000 / 4);
	bool DeleteFlickeringLight(INT16 nSegment = -1, INT16 nSide = -1);
	INT32 IsExplodingLight(INT32 nBaseTex);
	bool VisibleWall (UINT16 nWall);
	void SetCubeLight (double fLight, bool bAll = false, bool bDynCubeLights = false);
	void ScaleCornerLight (double fLight, bool bAll = false);
	void CalcAverageCornerLight (bool bAll = false);
	void AutoAdjustLight (double fBrightness, bool bAll = false, bool bCopyTexLights = false);
	void BlendColors (CColor *psc, CColor *pdc, double srcBr, double destBr);
	void Illuminate (INT16 nSrcSide, INT16 nSrcSeg, UINT32 brightness, 
						  double fLightScale, bool bAll = false, bool bCopyTexLights = false);
	bool CalcSideLights (INT32 nSegment, INT32 nSide, CFixVector& source_center, 
								CFixVector* source_corner, CFixVector& A, double *effect,
								double fLightScale, bool bIgnoreAngle);

	void FixChildren();
	void SetLinesToDraw ();

	INT16	MarkedSegmentCount (bool bCheck = false);
	bool	GotMarkedSegments (void)
		{ return MarkedSegmentCount (true) > 0; }
	bool CMine::GotMarkedSides ();

	inline void SetSelectMode (INT16 mode)
		{ m_selectMode = mode; }
	INT32 ScrollSpeed (UINT16 texture,INT32 *x,INT32 *y);
	INT32 AlignTextures (INT16 start_segment, INT16 start_side, INT16 only_child, BOOL bAlign1st, BOOL bAlign2nd, char bAlignedSides = 0);

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
	bool SizeItem (INT32 inc); 
	bool MovePoints (INT32 pt0, INT32 pt1); 
	bool SizeLine (CSegment *segP,INT32 point0,INT32 point1,INT32 inc); 
	bool MoveOn (char axis,INT32 inc); 
	bool SpinSelection(double angle); 
	void RotateVmsVector (CFixVector* vector, double angle, char axis); 
	void RotateVmsMatrix (CFixMatrix *matrix, double angle, char axis); 
	void RotateVertex (CFixVector* vertex, CFixVector* orgin, CFixVector* normal, double angle); 
	void SetUV (INT16 segment, INT16 side, INT16 x, INT16 y, double angle);
	void LoadSideTextures (INT16 segNum, INT16 sideNum);

	CWall *AddWall (INT16 nSegment, INT16 nSide, INT16 type, UINT16 flags, UINT8 keys, INT8 nClip, INT16 nTexture);
	CWall *GetWall (INT16 nSegment = -1, INT16 nSide = -1);
	void DeleteWall (UINT16 nWall = -1);
	CWall *FindWall (INT16 nSegment = -1, INT16 nSide = -1);
	void DefineWall (INT16 nSegment, INT16 nSide, UINT16 nWall,
						  UINT8 type, INT8 nClip, INT16 nTexture,
						  bool bRedefine);
	void SetWallTextures (UINT16 nWall, INT16 nTexture = 0);
	// trigger stuff
	void InitTrigger (CTrigger *t, INT16 type, INT16 flags);
	CTrigger *AddTrigger (UINT16 nWall, INT16 type, BOOL bAutoAddWall = FALSE);
	void DeleteTrigger (INT16 nTrigger = -1);
	bool DeleteTriggerTarget (CTrigger *trigger, INT16 nSegment, INT16 nSide, bool bAutoDeleteTrigger = true);
	void DeleteTriggerTargets (INT16 nSegment, INT16 nSide);
	INT32 DeleteTargetFromTrigger (CTrigger *trigger, INT16 linknum, bool bAutoDeleteTrigger = true);
	INT32 DeleteTargetFromTrigger (INT16 nTrigger, INT16 linknum, bool bAutoDeleteTrigger = true);
	INT16 FindTriggerWall (INT16 *nTrigger, INT16 nSegment = -1, INT16 nSide = -1);
	INT16 FindTriggerWall (INT16 nTrigger);
	INT16 FindTriggerObject (INT16 *nTrigger);
	INT16 FindTriggerTarget (INT16 nTrigger, INT16 nSegment, INT16 nSide);
	CTrigger *AddObjTrigger (INT16 objnum, INT16 type);
	bool ObjTriggerIsInList (INT16 nTrigger);
	void DeleteObjTrigger (INT16 objnum);
	void DeleteObjTriggers (INT16 objnum);
	INT16 FindObjTriggerTarget (INT16 nTrigger, INT16 nSegment, INT16 nSide);
	INT16 FindObjBySig (INT16 nSignature);

	void DrawObject (CWnd *pWnd, INT32 type, INT32 id);
	void ConvertWallNum (UINT16 wNumOld, UINT16 wNumNew);

	bool GetOppositeSide (INT16& nOppSeg, INT16& nOppSide, INT16 nSegment = -1, INT16 nSide = -1);
	bool GetOppositeWall (INT16 &nOppWall, INT16 nSegment = -1, INT16 nSide = -1);
	CSide *OppSide ();
	bool SetTexture (INT16 nSegment, INT16 nSide, INT16 nTexture, INT16 tmapnum2);
	void CopyOtherCube ();
	bool WallClipFromTexture (INT16 nSegment, INT16 nSide);
	void CheckForDoor (INT16 nSegment, INT16 nSide);
	void RenumberBotGens ();
	void RenumberEquipGens ();

	bool SetDefaultTexture (INT16 nTexture = -1, INT16 walltype = -1);
	bool DefineSegment (INT16 nSegment, UINT8 type, INT16 nTexture, INT16 walltype = -1);
	void UndefineSegment (INT16 nSegment);
	bool GetTriggerResources (UINT16& nWall);
	bool AutoAddTrigger (INT16 wall_type, UINT16 wall_flags, UINT16 trigger_type);
	bool AddDoorTrigger (INT16 wall_type, UINT16 wall_flags, UINT16 trigger_type);
	bool AddOpenDoorTrigger(); 
	bool AddRobotMakerTrigger (); 
	bool AddShieldTrigger(); 
	bool AddEnergyTrigger(); 
	bool AddUnlockTrigger(); 
	bool AddExit (INT16 type); 
	bool AddNormalExit(); 
	bool AddSecretExit(); 
	bool AddDoor (UINT8 type, UINT8 flags, UINT8 keys, INT8 nClip, INT16 nTexture); 
	bool AddAutoDoor (INT8 nClip = -1, INT16 nTexture = -1); 
	bool AddPrisonDoor (); 
	bool AddGuideBotDoor(); 
	bool AddFuelCell (); 
	bool AddIllusionaryWall (); 
	bool AddForceField (); 
	bool AddFan ();
	bool AddWaterFall ();
	bool AddLavaFall(); 
	bool AddGrate(); 
	bool AddReactor (INT16 nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddRobotMaker (INT16 nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddEquipMaker (INT16 nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddFuelCenter (INT16 nSegment = -1, UINT8 nType = SEGMENT_FUNC_FUELCEN, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddGoalCube (INT16 nSegment, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture);
	bool AddTeamCube (INT16 nSegment, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture);
	bool AddSpeedBoostCube (INT16 nSegment, bool bCreate);
	bool AddSkyboxCube (INT16 nSegment, bool bCreate);
	void AutoLinkExitToReactor ();
	INT32 FuelCenterCount (void);
	inline INT32& RobotMakerCount (void) 
		{ return GameInfo ().botgen.count; }
	inline INT32& EquipMakerCount (void) 
		{ return GameInfo ().equipgen.count; }
	inline INT32& WallCount (void) 
		{ return GameInfo ().walls.count; }
	inline INT32& TriggerCount (void) 
		{ return GameInfo ().triggers.count; }
	inline INT32& ObjectCount (void) 
		{ return GameInfo ().objects.count; }

	inline CSegment *OtherSeg (void)
		{ return Segments () + Other ()->nSegment; }
	inline CSide *OtherSide (void)
		{ return OtherSeg ()->sides + Other ()->nSide; }
	inline void SetCurrent (INT16 nSegment = -1, INT16 nSide = -1, INT16 nLine = -1, INT16 nPoint = -1) {
		if (nSegment >= 0) Current ()->nSegment = nSegment;
		if (nSide >= 0) Current ()->nSide = nSide;
		if (nLine >= 0) Current ()->nLine = nLine;
		if (nPoint >= 0) Current ()->nPoint = nPoint;
		}
	inline void GetCurrent (INT16 &nSegment, INT16& nSide) {
		if (nSegment < 0) nSegment = Current ()->nSegment;
		if (nSide < 0) nSide = Current ()->nSide;
		}

	void InitRobotData();
	INT32 WriteHxmFile (FILE *fp);
	INT32 ReadHxmFile (FILE *fp, long fSize);

	INT16 ReadSegmentInfo (FILE *file);
	void WriteSegmentInfo (FILE *file, INT16 /*nSegment*/);
	void CutBlock ();
	void CopyBlock (char *pszBlkFile = NULL);
	void PasteBlock (); 
	INT32 ReadBlock (char *name,INT32 option); 
	void QuickPasteBlock  ();
	void DeleteBlock ();

	inline void wrap (INT16 *x, INT16 delta,INT16 min,INT16 max) {
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
	void UntwistSegment (INT16 nSegment,INT16 nSide);
	INT32 MatchingSide (INT32 j);
	INT16 LoadPalette (void);

	void SortObjects ();
	void RenumberTriggerTargetObjs (void);
	void RenumberObjTriggers (void);
	void QSortObjects (INT16 left, INT16 right);
	INT32 QCmpObjects (CGameObject *pi, CGameObject *pm);
	INT32 QCmpObjTriggers (CTrigger *pi, CTrigger *pm);
	void QSortObjTriggers (INT16 left, INT16 right);
	void SortObjTriggers (void);
	bool IsCustomRobot (INT32 i);
	BOOL HasCustomRobots();
	INT16 LoadMineSigAndType (FILE* fp);

private:
	INT32 FindClip (CWall *wallP, INT16 nTexture);
	INT16 CreateNewLevel ();
	void DefineVertices(INT16 new_verts[4]);
	void UnlinkChild(INT16 parent_segnum,INT16 nSide);
	INT16 FixIndexValues();
	void ResetSide (INT16 nSegment,INT16 nSide);

	INT32 ReadHamFile(char *fname = NULL, INT32 type = NORMAL_HAM);
	void ReadPigTextureTable();
	void ReadRobotResource(INT32 robot_number);
	void ReadColor (CColor *pc, FILE *load_file);
	void SaveColor (CColor *pc, FILE *save_file);
	void LoadColors (CColor *pc, INT32 nColors, INT32 nFirstVersion, INT32 nNewVersion, FILE *fp);
	void SaveColors (CColor *pc, INT32 nColors, FILE *fp);
	INT32 LoadGameItem (FILE* fp, CGameItemInfo info, CGameItem* items, int nMinVersion,int nMaxCount, char *pszItem, bool bFlag = false);
	INT16 LoadMineDataCompiled (FILE *load_file, bool bNewMine);
	INT16 LoadMine (char *filename, bool bLoadFromHog, bool bNewMine);
	INT16 LoadGameData(FILE *loadfile, bool bNewMine);
	INT16 SaveMineDataCompiled(FILE *save_file);
	INT16 SaveGameData(FILE *savefile);
	void ClearMineData();
	void UpdateDeltaLights ();
	double dround_off(double value, double round);
	void SetSegmentChildNum(CSegment *pRoot, INT16 nSegment,INT16 recursion_level);
	void SetSegmentChildNum (CSegment *pRoot, INT16 nSegment, INT16 recursion_level, INT16* visited);
	void UnlinkSeg (CSegment *pSegment, CSegment *pRoot);
	void LinkSeg (CSegment *pSegment, CSegment *pRoot);
	void SortDLIndex (INT32 left, INT32 right);
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

#endif //__mine_h