#ifndef __mine_h
#define __mine_h

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
extern uvl default_uvls[4];

// Copyright (C) 1997 Bryan Aamot
//**************************************************************************
// CLASS - Level
//**************************************************************************

typedef struct tMineData {
	game_info					game_fileinfo;
	
	INT32							m_reactor_time;
	INT32							m_reactor_strength;
	INT32							m_secret_cubenum;
	tFixMatrix					m_secret_orient;
	
	// robot data
	ROBOT_INFO					Robot_info[MAX_ROBOT_TYPES];
	
	// structure data
	UINT16						numVertices;
	tFixVector					vertices[MAX_VERTICES3];
	
	UINT16						numSegments;
	CDSegment					segments[MAX_SEGMENTS3];
	CDColor 						lightColors [MAX_SEGMENTS3][6];
	CDColor						texColors [MAX_D2_TEXTURES];
	CDColor 						sideColors [MAX_SEGMENTS3][6];
	CDColor						vertexColors [MAX_VERTICES3];
	
	UINT8							vert_status[MAX_VERTICES3];
	
	CDWall						walls[MAX_WALLS3];
	active_door					active_doors[MAX_DOORS];
	CDTrigger					triggers[MAX_TRIGGERS2];
	CDTrigger					objTriggers[MAX_OBJ_TRIGGERS];
	INT32							numObjTriggers;
	control_center_trigger	control_center_triggers[MAX_CONTROL_CENTER_TRIGGERS];
	matcen_info					robot_centers[MAX_NUM_MATCENS2];
	matcen_info					equip_centers[MAX_NUM_MATCENS2];
	
	// object data
	CDObject						objects[MAX_OBJECTS2];
	
	// light data
	dl_index						dl_indices[MAX_DL_INDICES_D2X];
	delta_light					delta_lights[MAX_DELTA_LIGHTS_D2X];
	
	// flickering light
	INT16							m_nFlickeringLights;
	FLICKERING_LIGHT			flickering_lights[MAX_FLICKERING_LIGHTS];

	CDSelection					current1;
	CDSelection					current2;
	CDSelection					*current;

} MINE_DATA;

class CMine {
public:
	
	// level info
	INT32							m_fileType;
	INT32							m_levelVersion;
	char							m_currentLevelName [256];	
	game_top_info				game_top_fileinfo;
	MINE_DATA					m_mineData;
	ROBOT_INFO					m_defaultRobotInfo [MAX_ROBOT_TYPES];
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
	void Reset ();
	
public:
	inline MINE_DATA& MineData ()
		{ return m_mineData; }
	inline tFixVector *Vertices (INT32 i = 0)
		{ return MineData ().vertices + i; }
	inline UINT8 *VertStatus (INT32 i = 0)
		{ return MineData ().vert_status + i; }
	inline CDSegment *Segments (INT32 i = 0)
		{ return MineData ().segments + i; }
	inline CDColor *VertexColors (INT32 i = 0)
		{ return &(MineData ().vertexColors [i]); }
	inline CDWall *Walls (INT32 i = 0)
		{ return MineData ().walls + i; }
	inline CDTrigger *Triggers (INT32 i = 0)
		{ return MineData ().triggers + i; }
	inline INT32 &NumTriggers ()
		{ return GameInfo ().triggers.count; }
	inline CDTrigger *ObjTriggers (INT32 i = 0)
		{ return MineData ().objTriggers + i; }
	inline INT32& NumObjTriggers ()
		{ return MineData ().numObjTriggers; }
	inline CDObject *Objects (INT32 i = 0)
		{ return MineData ().objects + i; }
	inline matcen_info *BotGens (INT32 i = 0)
		{ return MineData ().robot_centers + i; }
	inline matcen_info *EquipGens (INT32 i = 0)
		{ return MineData ().equip_centers + i; }
	inline control_center_trigger *CCTriggers (INT32 i = 0)
		{ return MineData ().control_center_triggers + i; }
	inline active_door *ActiveDoors (INT32 i = 0)
		{ return MineData ().active_doors + i; }
	inline ROBOT_INFO *RobotInfo (INT32 i = 0)
		{ return MineData ().Robot_info + i; }
	inline ROBOT_INFO *DefRobotInfo (INT32 i = 0)
		{ return m_defaultRobotInfo + i; }
	inline game_info& GameInfo ()
		{ return MineData ().game_fileinfo; }
	inline UINT16& SegCount ()
		{ return MineData ().numSegments; }
	inline INT32& ObjCount ()
		{ return GameInfo ().objects.count; }
	inline UINT16& VertCount ()
		{ return MineData ().numVertices; }
	inline dl_index *DLIndex (INT32 i = 0)
		{ return MineData ().dl_indices + i; }
	inline delta_light *DeltaLights (INT32 i = 0)
		{ return MineData ().delta_lights + i; }
	inline FLICKERING_LIGHT *FlickeringLights (INT32 i = 0)
		{ return MineData ().flickering_lights + i; }
	inline INT16& FlickerLightCount ()
		{ return MineData ().m_nFlickeringLights; }
	long TotalSize (game_item_info& gii)
		{ return (long) gii.size * (long) gii.count; }
	inline INT32& ReactorTime ()
		{ return MineData ().m_reactor_time; }
	inline INT32& ReactorStrength ()
		{ return MineData ().m_reactor_strength; }
	inline INT32& SecretCubeNum ()
		{ return MineData ().m_secret_cubenum; }
	inline tFixMatrix& SecretOrient ()
		{ return MineData ().m_secret_orient; }
	inline CDSelection* &Current ()
		{ return MineData ().current; }
	inline CDSelection& Current1 ()
		{ return MineData ().current1; }
	inline CDSelection& Current2 ()
		{ return MineData ().current2; }
	inline CDSelection *Other (void)
		{ return (Current () == &Current2 ()) ? &Current1 () : &Current2 (); }
	inline CDColor *TexColors (INT32 i = 0)
		{ return MineData ().texColors + (i & 0x3fff); }
	inline bool& UseTexColors (void)
		{ return m_bUseTexColors; }
	inline void SetTexColor (INT16 nBaseTex, CDColor *pc)	{
		if (UseTexColors () && (IsLight (nBaseTex) != -1))
			*TexColors (nBaseTex) = *pc;
		}
	inline CDColor *GetTexColor (INT16 nBaseTex, bool bIsTranspWall = false)	
		{ return UseTexColors () && (bIsTranspWall || (IsLight (nBaseTex) != -1)) ? TexColors (nBaseTex) : NULL; }
	CDColor *LightColor (INT32 i = 0, INT32 j = 0, bool bUseTexColors = true);
	inline CDColor *LightColors (INT32 i = 0, INT32 j = 0)
		{ return MineData ().lightColors [i] + j; }
	inline CDColor *CurrLightColor ()
		{ return LightColor (Current ()->segment, Current ()->side); }

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
	void  DeleteSegmentWalls (INT16 segnum);
	void	MakeObject (CDObject *obj, INT8 type, INT16 segnum);
	void	SetObjectData (INT8 type);
	bool	CopyObject (UINT8 new_type, INT16 segnum = -1);
	void  DeleteObject(INT16 objectNumber = -1);
	void  DeleteUnusedVertices();
	void  DeleteVertex(INT16 deleted_vertnum);

	void InitSegment (INT16 segNum);
	bool SplitSegment ();
	bool  AddSegment();
	bool  LinkSegments(INT16 segnum1,INT16 sidenum1, INT16 segnum2,INT16 sidenum2, FIX margin);
	void  LinkSides(INT16 segnum1,INT16 sidenum1,INT16 segnum2,INT16 sidenum2, tVertMatch match[4]);
	void	CalcSegCenter(tFixVector &pos,INT16 segnum);
	inline CDSegment *CurrSeg ()
		{ return Segments () + Current ()->segment; }
	inline CDWall *SideWall (INT32 i = 0, INT32 j = 0)
		{ INT32 w = Segments (i)->sides [j].nWall; return (w < 0) ? NULL : Walls (w); }
	inline CDWall *CurrWall ()
		{ INT32 w = CurrSide ()->nWall; return (w < 0) ? NULL : Walls (w); }
	inline CDSide *CurrSide ()
		{ return CurrSeg ()->sides + Current ()->side; }
	inline INT16 CurrVert ()
		{ return CurrSeg ()->verts [side_vert [Current ()->side][Current ()->point]]; }
	inline CDObject *CurrObj ()
		{ return Objects () + Current ()->object; }
	void Mark ();
	void MarkAll ();
	void UnmarkAll ();
	void MarkSegment (INT16 segnum);
	void UpdateMarkedCubes ();
	bool SideIsMarked (INT16 segnum, INT16 sidenum);
	bool SegmentIsMarked (INT16 segnum);

	bool IsPointOfSide (CDSegment *seg, INT32 sidenum, INT32 pointnum);
	bool IsLineOfSide (CDSegment *seg, INT32 sidenum, INT32 linenum);

	void JoinSegments(INT32 automatic = 0);
	void JoinLines();
	void JoinPoints();
	void SplitSegments(INT32 solidify = 0, INT32 sidenum = -1);
	void SplitLines();
	void SplitPoints();

	void CalcOrthoVector (tFixVector &result,INT16 segnum,INT16 sidenum);
	void CalcCenter (tFixVector &center,INT16 segnum,INT16 sidenum);
	double CalcLength (tFixVector *center1, tFixVector *center2);

	INT32 IsLight(INT32 nBaseTex);
	INT32 IsWall (INT16 segnum = -1, INT16 sidenum = -1);
	bool IsLava (INT32 nBaseTex);
	bool IsBlastableLight (INT32 nBaseTex);
	bool IsFlickeringLight (INT16 segnum, INT16 sidenum);
	bool CalcDeltaLights (double fLightScale, INT32 force, INT32 recursion_depth);
	void CalcDeltaLightData (double fLightScale = 1.0, INT32 force = 1);
	INT32 FindDeltaLight (INT16 segnum, INT16 sidenum, INT16 *pi = NULL);
	UINT8 LightWeight(INT16 nBaseTex);
	INT16 GetFlickeringLight(INT16 segnum = -1, INT16 sidenum = -1);
	INT16 AddFlickeringLight( INT16 segnum = -1, INT16 sidenum = -1, UINT32 mask = 0xAAAAAAAA, FIX time = 0x10000 / 4);
	bool DeleteFlickeringLight(INT16 segnum = -1, INT16 sidenum = -1);
	INT32 IsExplodingLight(INT32 nBaseTex);
	bool VisibleWall (UINT16 nWall);
	void SetCubeLight (double fLight, bool bAll = false, bool bDynCubeLights = false);
	void ScaleCornerLight (double fLight, bool bAll = false);
	void CalcAverageCornerLight (bool bAll = false);
	void AutoAdjustLight (double fBrightness, bool bAll = false, bool bCopyTexLights = false);
	void BlendColors (CDColor *psc, CDColor *pdc, double srcBr, double destBr);
	void Illuminate (INT16 source_segnum, INT16 source_sidenum, UINT32 brightness, 
						  double fLightScale, bool bAll = false, bool bCopyTexLights = false);
	bool CalcSideLights (INT32 segnum, INT32 sidenum, tFixVector& source_center, 
								tFixVector *source_corner, tFixVector& A, double *effect,
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
	bool SizeLine (CDSegment *seg,INT32 point0,INT32 point1,INT32 inc); 
	bool MoveOn (char axis,INT32 inc); 
	bool SpinSelection(double angle); 
	void RotateVmsVector(tFixVector *vector,double angle,char axis); 
	void RotateVmsMatrix(tFixMatrix *matrix,double angle,char axis); 
	void RotateVertex(tFixVector *vertex, tFixVector *orgin, tFixVector *normal, double angle); 
	void SetUV (INT16 segment, INT16 side, INT16 x, INT16 y, double angle);
	void LoadSideTextures (INT16 segNum, INT16 sideNum);

	CDWall *AddWall (INT16 segnum, INT16 sidenum, INT16 type, UINT16 flags, UINT8 keys, INT8 clipnum, INT16 nTexture);
	CDWall *GetWall (INT16 segnum = -1, INT16 sidenum = -1);
	void DeleteWall (UINT16 wallnum = -1);
	CDWall *FindWall (INT16 segnum = -1, INT16 sidenum = -1);
	void DefineWall (INT16 segnum, INT16 sidenum, UINT16 wallnum,
						  UINT8 type, INT8 clipnum, INT16 nTexture,
						  bool bRedefine);
	void SetWallTextures (UINT16 wallnum, INT16 nTexture = 0);
	// trigger stuff
	void InitTrigger (CDTrigger *t, INT16 type, INT16 flags);
	CDTrigger *AddTrigger (UINT16 wallnum, INT16 type, BOOL bAutoAddWall = FALSE);
	void DeleteTrigger (INT16 trignum = -1);
	bool DeleteTriggerTarget (CDTrigger *trigger, INT16 segnum, INT16 sidenum, bool bAutoDeleteTrigger = true);
	void DeleteTriggerTargets (INT16 segnum, INT16 sidenum);
	INT32 DeleteTargetFromTrigger (CDTrigger *trigger, INT16 linknum, bool bAutoDeleteTrigger = true);
	INT32 DeleteTargetFromTrigger (INT16 trignum, INT16 linknum, bool bAutoDeleteTrigger = true);
	INT16 FindTriggerWall (INT16 *trignum, INT16 segnum = -1, INT16 sidenum = -1);
	INT16 FindTriggerWall (INT16 trignum);
	INT16 FindTriggerObject (INT16 *trignum);
	INT16 FindTriggerTarget (INT16 trignum, INT16 segnum, INT16 sidenum);
	CDTrigger *AddObjTrigger (INT16 objnum, INT16 type);
	bool ObjTriggerIsInList (INT16 nTrigger);
	void DeleteObjTrigger (INT16 objnum);
	void DeleteObjTriggers (INT16 objnum);
	INT16 FindObjTriggerTarget (INT16 trignum, INT16 segnum, INT16 sidenum);
	INT16 FindObjBySig (INT16 nSignature);

	void DrawObject (CWnd *pWnd, INT32 type, INT32 id);
	void ConvertWallNum (UINT16 wNumOld, UINT16 wNumNew);

	bool GetOppositeSide (INT16& opp_segnum, INT16& opp_sidenum, INT16 segnum = -1, INT16 sidenum = -1);
	bool GetOppositeWall (INT16 &opp_wallnum, INT16 segnum = -1, INT16 sidenum = -1);
	CDSide *OppSide ();
	bool SetTexture (INT16 segnum, INT16 sidenum, INT16 nTexture, INT16 tmapnum2);
	void CopyOtherCube ();
	bool WallClipFromTexture (INT16 segnum, INT16 sidenum);
	void CheckForDoor (INT16 segnum, INT16 sidenum);
	void RenumberBotGens ();
	void RenumberEquipGens ();

	bool SetDefaultTexture (INT16 nTexture = -1, INT16 walltype = -1);
	bool DefineSegment (INT16 segnum, UINT8 type, INT16 nTexture, INT16 walltype = -1);
	void UndefineSegment (INT16 segnum);
	bool GetTriggerResources (UINT16& wallnum);
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
	bool AddDoor (UINT8 type, UINT8 flags, UINT8 keys, INT8 clipnum, INT16 nTexture); 
	bool AddAutoDoor (INT8 clipnum = -1, INT16 nTexture = -1); 
	bool AddPrisonDoor (); 
	bool AddGuideBotDoor(); 
	bool AddFuelCell (); 
	bool AddIllusionaryWall (); 
	bool AddForceField (); 
	bool AddFan ();
	bool AddWaterFall ();
	bool AddLavaFall(); 
	bool AddGrate(); 
	bool AddReactor (INT16 segnum = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddRobotMaker (INT16 segnum = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddEquipMaker (INT16 segnum = -1, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddFuelCenter (INT16 segnum = -1, UINT8 nType = SEGMENT_FUNC_FUELCEN, bool bCreate = true, bool bSetDefTextures = true); 
	bool AddGoalCube (INT16 segnum, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture);
	bool AddTeamCube (INT16 segnum, bool bCreate, bool bSetDefTextures, UINT8 nType, INT16 nTexture);
	bool AddSpeedBoostCube (INT16 segnum, bool bCreate);
	bool AddSkyboxCube (INT16 segnum, bool bCreate);
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

	inline CDSegment *OtherSeg (void)
		{ return Segments () + Other ()->segment; }
	inline CDSide *OtherSide (void)
		{ return OtherSeg ()->sides + Other ()->side; }
	inline void SetCurrent (INT16 nSegment = -1, INT16 nSide = -1, INT16 nLine = -1, INT16 nPoint = -1) {
		if (nSegment >= 0) Current ()->segment = nSegment;
		if (nSide >= 0) Current ()->side = nSide;
		if (nLine >= 0) Current ()->line = nLine;
		if (nPoint >= 0) Current ()->point = nPoint;
		}
	inline void GetCurrent (INT16 &nSegment, INT16& nSide) {
		if (nSegment < 0) nSegment = Current ()->segment;
		if (nSide < 0) nSide = Current ()->side;
		}

	void InitRobotData();
	INT32 WriteHxmFile (FILE *fp);
	INT32 ReadHxmFile (FILE *fp, long fSize);

	INT16 ReadSegmentInfo (FILE *file);
	void WriteSegmentInfo (FILE *file, INT16 /*segnum*/);
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
	void UntwistSegment (INT16 segnum,INT16 sidenum);
	INT32 MatchingSide (INT32 j);
	INT16 LoadPalette (void);

	void SortObjects ();
	void RenumberTriggerTargetObjs (void);
	void RenumberObjTriggers (void);
	void QSortObjects (INT16 left, INT16 right);
	INT32 QCmpObjects (CDObject *pi, CDObject *pm);
	INT32 QCmpObjTriggers (CDTrigger *pi, CDTrigger *pm);
	void QSortObjTriggers (INT16 left, INT16 right);
	void SortObjTriggers (void);
	bool IsCustomRobot (INT32 i);
	BOOL HasCustomRobots();
	INT16 LoadMineSigAndType (FILE* fp);

private:
	INT32 FindClip (CDWall *wall, INT16 nTexture);
	INT16 CreateNewLevel ();
	void DefineVertices(INT16 new_verts[4]);
	void UnlinkChild(INT16 parent_segnum,INT16 sidenum);
	INT16 FixIndexValues();
	void ResetSide (INT16 segnum,INT16 sidenum);

	INT32 ReadHamFile(char *fname = NULL, INT32 type = NORMAL_HAM);
	void ReadPigTextureTable();
	void ReadRobotResource(INT32 robot_number);
	void ReadColor (CDColor *pc, FILE *load_file);
	void SaveColor (CDColor *pc, FILE *save_file);
	void ReadTrigger (CDTrigger *t, FILE *fp, bool bObjTrigger);
	void WriteTrigger (CDTrigger *t, FILE *fp, bool bObjTrigger);
	void LoadColors (CDColor *pc, INT32 nColors, INT32 nFirstVersion, INT32 nNewVersion, FILE *fp);
	void SaveColors (CDColor *pc, INT32 nColors, FILE *fp);
	INT16 LoadMineDataCompiled (FILE *load_file, bool bNewMine);
	INT16 LoadMine (char *filename, bool bLoadFromHog, bool bNewMine);
	INT16 LoadGameData(FILE *loadfile, bool bNewMine);
	INT16 SaveMineDataCompiled(FILE *save_file);
	INT16 SaveGameData(FILE *savefile);
	void ReadObject(CDObject *obj,FILE *f,INT32 version);
	void WriteObject(CDObject *obj,FILE *f,INT32 version);
	INT32 ReadWall (CDWall* wallP, FILE* fp, INT32 version);
	void WriteWall (CDWall* wallP, FILE* fp, INT32 version);
	void ClearMineData();
	void UpdateDeltaLights ();
	double dround_off(double value, double round);
	void SetSegmentChildNum(CDSegment *pRoot, INT16 segnum,INT16 recursion_level);
	void SetSegmentChildNum (CDSegment *pRoot, INT16 segnum, INT16 recursion_level, INT16* visited);
	void UnlinkSeg (CDSegment *pSegment, CDSegment *pRoot);
	void LinkSeg (CDSegment *pSegment, CDSegment *pRoot);
	void SortDLIndex (INT32 left, INT32 right);
	};

CMine* GetMine (CMine* m);

#define GET_MINE(m) (m = GetMine(m))

inline INT32 MAX_SEGMENTS (CMine* m = NULL) { return !GET_MINE (m) ? 0 : m->IsD1File () ? MAX_SEGMENTS1  : m->IsStdLevel () ? MAX_SEGMENTS2 : MAX_SEGMENTS3; }
inline INT32 MAX_VERTICES (CMine* m = NULL) { return !GET_MINE (m) ? 0 : m->IsD1File () ? MAX_VERTICES1 : m->IsStdLevel () ? MAX_VERTICES2 : MAX_VERTICES3; }
inline INT32 MAX_WALLS (CMine* m = NULL) { return !GET_MINE (m) ? 0 : m->IsD1File () ? MAX_WALLS1 : (m->LevelVersion () < 12) ? MAX_WALLS2 : MAX_WALLS3; }
inline INT32 MAX_TEXTURES (CMine* m = NULL) { return !GET_MINE (m) ? 0 : m->IsD1File () ? MAX_D1_TEXTURES : MAX_D2_TEXTURES; }
inline INT32 MAX_TRIGGERS (CMine* m = NULL) { return !GET_MINE (m) ? 0 : (m->IsD1File () || (m->LevelVersion () < 12)) ? MAX_TRIGGERS1 : MAX_TRIGGERS2; }
inline INT32 MAX_OBJECTS (CMine* m = NULL) { return !GET_MINE (m) ? 0 : m->IsStdLevel () ? MAX_OBJECTS1 : MAX_OBJECTS2; }
inline INT32 MAX_NUM_FUELCENS (CMine* m = NULL) { return !GET_MINE (m) ? 0 : (m->IsD1File () || (m->LevelVersion () < 12)) ? MAX_NUM_FUELCENS1 : MAX_NUM_FUELCENS2; }
inline INT32 MAX_NUM_REPAIRCENS (CMine* m = NULL) { return !GET_MINE (m) ? 0 : (m->IsD1File () || (m->LevelVersion () < 12)) ? MAX_NUM_REPAIRCENS1 : MAX_NUM_REPAIRCENS2; }
inline INT32 MAX_PLAYERS (CMine* m = NULL) { return !GET_MINE (m) ? 0 : m->IsStdLevel () ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X; }
inline INT32 ROBOT_IDS2 (CMine* m = NULL) { return !GET_MINE (m) ? 0 : (m->LevelVersion () == 7) ? N_D2_ROBOT_TYPES : MAX_ROBOT_IDS_TOTAL; }
inline INT32 MAX_NUM_MATCENS (CMine* m = NULL) { return !GET_MINE (m) ? 0 : (m->IsD1File () || (m->LevelVersion () < 12)) ? MAX_NUM_MATCENS1 : MAX_NUM_MATCENS2; }
inline INT32 MAX_DL_INDICES (CMine* m = NULL) { return !GET_MINE (m) ? 0 : (m->IsD1File () || m->IsStdLevel ()) ? MAX_DL_INDICES_D2 : MAX_DL_INDICES_D2X; }
inline INT32 MAX_DELTA_LIGHTS (CMine* m = NULL) { return !GET_MINE (m) ? 0 : (m->IsD1File () || m->IsStdLevel ()) ? MAX_DELTA_LIGHTS_D2 : MAX_DELTA_LIGHTS_D2X; }

#define NO_WALL(m) MAX_WALLS(m)

#endif //__mine_h