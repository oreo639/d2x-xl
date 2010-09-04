#ifndef __mine_h
#define __mine_h

#include "define.h"
#include "global.h"
#include "cfile.h"
#include "carray.h"
#include "Types.h"
#include "Selection.h"
#include "Textures.h"
#include "MineInfo.h"
#include "TriggerManager.h"
#include "WallManager.h"
#include "SegmentManager.h"
#include "VertexManager.h"
#include "RobotManager.h"
#include "ObjectManager.h"
#include "TunnelMaker.h"
#include "BlockManager.h"
#include "LightManager.h"
#include "TextureManager.h"
#include "HogManager.h"
#include "UndoManager.h"
#include "ResourceManager.h"

#ifdef _DEBUG

#define CLEAR(_b) (_b) [0].Reset((_b).Length (void))
#define ASSIGN(_a,_b) (_a) = (_b)
#define DATA(_b) (_b).Buffer (void)

#else

#define CLEAR(_b)	(_b)->Reset (sizeof (_b) / sizeof (_b [0]))
#define ASSIGN(_a,_b) memcpy (_a, _b, sizeof (_a))
#define DATA(_b) (_b)

#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CMine {
	public:
		// level info
		int				m_fileType;
		int				m_levelVersion;
		char				m_currentLevelName [256];	
		CMineData		m_data;

		HPALETTE			m_paletteHandle;
		
		// strings
		char				message [256];
		char				m_startFolder [256];
		short				m_selectMode;
		int				m_disableDrawing;
		int				m_changesMade;
		int				m_nNoLightDeltas;
		int				m_lightRenderDepth;
		int				m_deltaLightRenderDepth;
		char				m_szBlockFile [256];
		bool				m_bVertigo;
		char*				m_pHxmExtraData;
		int				m_nHxmExtraDataSize;
	// Constructor/Desctuctor
	public:
		CMine(void);
		~CMine(void);
		void Initialize (void);
		void Reset (void);
		void Default (void);
		
	public:
		inline CMineData& Data (void)
			{ return m_data; }

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

		//inline textureList& Textures (void)
		//	{ return textures; }



		//inline CTexture* Textures (int i, int j = 0)
		//	{ return &textureManager.textures [i][j]; }

		inline CMineInfo& Info (void)
			{ return Data ().mineInfo; }
		inline CMineFileInfo& FileInfo (void)
			{ return Data ().mineInfo.fileInfo; }

		long TotalSize (CMineItemInfo& gii) { return (int) gii.size * (int) gii.count; }

		inline int& ReactorTime (void) { return Data ().m_reactorTime; }

		inline int& ReactorStrength (void) { return Data ().m_reactorStrength; }

		inline int& SecretSegment (void) { return Data ().m_secretSegNum; }

		inline CDoubleMatrix& SecretOrient (void) { return Data ().m_secretOrient; }

		short Load (const char *filename = null, bool bLoadFromHog = false);

		short Save (const char *filename, bool bSaveToHog = false);

		inline LPSTR LevelName (void) { return m_currentLevelName; }

		inline int LevelNameSize (void) { return sizeof m_currentLevelName; }

		void Mark (void);
		void MarkAll (void);
		void UnmarkAll (void);


		void FixChildren(void);

		inline void SetSelectMode (short mode) { m_selectMode = mode; }

		int ScrollSpeed (ushort texture,int *x,int *y);

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
		bool GetTriggerResources (ushort& nWall);

		int FuelCenterCount (void);
		inline int& RobotMakerCount (void) 
			{ return Info ().botGen.count; }
		inline int& EquipMakerCount (void) 
			{ return Info ().equipGen.count; }
		inline int& WallCount (void) 
			{ return Info ().walls.count; }
		inline int& TriggerCount (void) 
			{ return Info ().triggers.count; }
		inline int& ObjectCount (void) 
			{ return Info ().objects.count; }


		short ReadSegmentInfo (CFileManager& fp);
		void WriteSegmentInfo (CFileManager& fp);
		void CutBlock (void);
		void CopyBlock (char *pszBlkFile = null);
		void PasteBlock (void); 
		int ReadBlock (char *name,int option); 
		void QuickPasteBlock  (void);
		void DeleteBlock (void);

		inline void wrap (short *x, short delta,short min,short max) {
			*x += delta;
			if (*x > max)
				*x = min;
			else if (*x < min)
				*x = max;
			}

		void TunnelGenerator (void);
		void IncreaseSpline (void);
		void DecreaseSpline (void);
		void CalcSpline (void);
		void UntwistSegment (short nSegment,short nSide);
		int MatchingSide (int j);

		short LoadMineSigAndType (CFileManager& fp);
		void LoadPaletteName (CFileManager& fp, bool bNewMine = false);

	private:
		short CreateNewLevel (void);
		void DefineVertices(short newVerts [4]);
		void UnlinkChild(short parent_segnum,short nSide);
		short FixIndexValues (void);

		void ClearGameItem (CGameItem* items, int nCount);
		int LoadGameItem (CFileManager& fp, CMineItemInfo info, CGameItem* items, int nMinVersion,int nMaxCount, char *pszItem, bool bFlag = false);
		int SaveGameItem (CFileManager& fp, CMineItemInfo& info, CGameItem* items, bool bFlag = false);
		short LoadMineDataCompiled (CFileManager& fp, bool bNewMine);
		short LoadMine (char *filename, bool bLoadFromHog, bool bNewMine);
		short LoadGameData (CFileManager& loadfile, bool bNewMine);
		short SaveMineDataCompiled (CFileManager& fp);
		short SaveGameData (CFileManager& savefile);
		void ClearMineData (void);
		void UpdateDeltaLights (void);
		void SortDLIndex (int left, int right);
	};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define MAX_LIGHT_DELTA_INDICES ((theMine == null) ? MAX_LIGHT_DELTA_INDICES_STD : (theMine->IsD1File () || theMine->IsStdLevel ()) ? MAX_LIGHT_DELTA_INDICES_STD : MAX_LIGHT_DELTA_INDICES_D2X)
#define MAX_LIGHT_DELTA_VALUES ((theMine == null) ? MAX_LIGHT_DELTA_VALUES_STD : (theMine->IsD1File () || theMine->IsStdLevel ()) ? MAX_LIGHT_DELTA_VALUES_STD : MAX_LIGHT_DELTA_VALUES_D2X)

extern CMine* theMine;

#define CHECKMINE			if ((theMine == null)) return;
#define CHECKMINEV(_v)	if ((theMine == null)) return (_v);
#define CHECKMINEF		CHECKMINE(FALSE);

#endif //__mine_h