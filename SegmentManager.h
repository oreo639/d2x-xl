#ifndef __segman_h
#define __segman_h

#include "define.h"
#include "cfile.h"
#include "carray.h"
#include "Types.h"
#include "Selection.h"
#include "MineInfo.h"
#include "FreeList.h"

//------------------------------------------------------------------------

#define MAX_SEGMENTS_D1		800  // descent 1 max # of cubes
#define MAX_SEGMENTS_D2		900  // descent 2 max # of cubes
#define SEGMENT_LIMIT		8000 // D2X-XL max # of cubes

#define MAX_SEGMENTS ((theMine == null) ? MAX_SEGMENTS_D2 : theMine->IsD1File () ? MAX_SEGMENTS_D1  : theMine->IsStdLevel () ? MAX_SEGMENTS_D2 : SEGMENT_LIMIT)

#define MAX_NUM_MATCENS_D1			20
#define MAX_NUM_MATCENS_D2			100

#define MAX_NUM_RECHARGERS_D2		70
#define MAX_NUM_RECHARGERS_D2X	500

#define MAX_NUM_RECHARGERS ((theMine == null) ? MAX_NUM_RECHARGERS_D2X : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_NUM_RECHARGERS_D2 : MAX_NUM_RECHARGERS_D2X)

//------------------------------------------------------------------------

#ifdef _DEBUG

typedef CStaticArray< CSegment, SEGMENT_LIMIT > segmentList;
typedef CStaticArray< CMatCenter, MAX_NUM_MATCENS_D2 > robotMakerList;

#else

typedef CSegment segmentList [SEGMENT_LIMIT];
typedef CMatCenter robotMakerList [MAX_NUM_MATCENS_D2];

#endif

//------------------------------------------------------------------------

typedef struct tVertMatch {
		short		b;
		short		i;
		double	d;
	} tVertMatch; 

//------------------------------------------------------------------------

class CSegmentManager {
	public:
		segmentList				m_segments;
		CMineItemInfo			m_segmentInfo;
		CFreeList<CSegment>	m_free;
		int						m_nAddMode;
		robotMakerList			m_matCens [2];
		CMineItemInfo			m_matCenInfo [2];

	public:
		inline void ResetInfo (void) {
			m_segmentInfo.Reset ();
			m_matCenInfo [0].Reset ();
			m_matCenInfo [1].Reset ();
			m_free.Reset ();
			}
		// Segment and side getters
		inline segmentList& Segments (void)	{ return m_segments; }

		inline int& Count (void) { return m_segmentInfo.count; }

		inline int& FileOffset (void) { return m_segmentInfo.offset; }

		inline int& MatCenCount (short nClass) { return m_matCenInfo [nClass].count; }

		inline int& RobotMakerCount (void) { return MatCenCount (0); }

		inline int& EquipMakerCount (void) { return MatCenCount (1); }

		inline short Index (CSegment* segP) { return (short) (segP - &m_segments [0]); }

		inline CSegment *Segment (int i) { return &m_segments [i]; }

		inline CSegment *Segment (CSideKey& key) { return Segment (key.m_nSegment); }

		inline CSide* Side (CSideKey key) {
			current.Get (key);
			return &Segment (key.m_nSegment)->m_sides [key.m_nSide];
			}

		inline CSide* Side (void) { return Side (CSideKey ()); }
		//inline CSide* Side (short nSegment = -1, short nSide = -1) {
		//	current.Get (nSegment, nSide);
		//	return &Segment (nSegment)->m_sides [nSide];
		//	}

		//CSide* OppositeSide (short nSegment = -1, short nSide = -1);

		CSide* OppositeSide (CSideKey key, CSideKey& opp);

		inline CSide* OppositeSide (CSideKey& opp) { return OppositeSide (CSideKey (), opp); }
		//CSide* OppositeSide (CSideKey opp, CSideKey& key, short nSegment = -1, short nSide = -1);

		//inline CWall* Wall (short nSegment = -1, short nSide = -1) {
		inline CWall* Wall (CSideKey key) { 
			current.Get (key);
			return Side (key)->Wall (); 
			}

		inline CWall* Wall (void) { return Wall (CSideKey ()); }

		int IsWall (CSideKey key);

		//inline CTrigger* Trigger (short nSegment = -1, short nSide = -1) {
		inline CTrigger* Trigger (CSideKey key) { return Side (key)->Trigger (); }

		inline CTrigger* Trigger (void) { return Trigger(CSideKey ()); }

		//inline CWall* OppositeWall (short nSegment = -1, short nSide = -1) {
		inline CWall* OppositeWall (CSideKey key) {
			CSide* sideP = Side (key);
			return (sideP == null) ? null : Side (key)->Wall ();
			}

		inline CWall* OppositeWall (void) { return OppositeWall (CSideKey ()); }

		void Textures (CSideKey key, short& nBaseTex, short& nOvlTex);

		inline robotMakerList& RobotMakers (void) { return m_matCens [0]; }
		
		inline robotMakerList& EquipMakers (void) { return m_matCens [1]; }
		
		inline CMatCenter* RobotMaker (int i) { return &m_matCens [0][i]; }
		
		inline CMatCenter* EquipMaker (int i) { return &m_matCens [1][i]; }

		inline int SetAddMode (int nMode) { return m_nAddMode = nMode; }
		inline int AddMode (void) { return m_nAddMode; }

		CSegment* FindRobotMaker (short i = 0);

		// Operations
		short Add (void);

		bool Create (void);

		void Delete (short nDelSeg = -1);

		void ResetSide (short nSegment, short nSide);

		bool CreateReactor (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateRobotMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateEquipMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateFuelCenter (short nSegment = -1, byte nType = SEGMENT_FUNC_FUELCEN, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateGoal (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture);

		bool CreateTeam (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture);

		bool CreateSpeedBoost (short nSegment, bool bCreate);

		bool CreateSkybox (short nSegment, bool bCreate);

		void Init (short nSegment);

		bool Split (void);

		void Join (int solidify);

		bool Link (short nSegment1, short nSide1, short nSegment2, short nSide2, double margin);

		void LinkSides (short nSegment1, short nSide1, short nSegment2, short nSide2, tVertMatch match [4]);

		void CalcCenter (CVertex& pos, short nSegment);

		bool IsPointOfSide (CSegment *segP, int nSide, int nPoint);

		bool IsLineOfSide (CSegment *segP, int nSide, int nLine);

		void JoinSegments (int automatic = 0);

		void SplitSegments (int solidify = 0, int nSide = -1);

		void JoinLines (void);

		void JoinPoints (void);

		void SplitLines (void);

		void SplitPoints (void);

		CDoubleVector CalcSideNormal (CSideKey key = CSideKey (-1, -1));

		CDoubleVector CalcSideCenter (CSideKey key = CSideKey (-1, -1));
		//double CalcLength (CFixVector* center1, CFixVector* center2);

		void FixChildren (void);

		void UpdateWalls (short nOldWall, short nNewWall);

		void UpdateVertex (short nOldVert, short nNewVert);

		void Mark (short nSegment);

		void MarkAll (void);

		void UnmarkAll (void);

		void UpdateMarked (void);

		bool IsMarked (short nSegment);

		bool IsMarked (CSideKey key = CSideKey (-1, -1));

		short	MarkedCount (bool bCheck = false);

		bool HaveMarkedSegments (void) { return MarkedCount (true) > 0; }

		bool HaveMarkedSides (void);

		int AlignTextures (short nStartSeg, short nStartSide, short onlyChild, bool bAlign1st, bool bAlign2nd, char bAlignedSides = 0);

		CSide* OppSide (void);

		bool SetTextures (CSideKey key, short nBaseTex, short nOvlTex);

		void CopyOtherSegment (void);

		void RenumberRobotMakers (void);

		void RenumberEquipMakers (void);

		bool SetDefaultTexture (short nTexture);

		bool DefineSegment (short nSegment, byte type, short nTexture, short wallType = -1);

		void UndefineSegment (short nSegment);

		void SetLinesToDraw (void);

		short ReadSegmentInfo (CFileManager& fp);

		void WriteSegmentInfo (CFileManager& fp, short);

		void CutBlock ();

		void CopyBlock (char *filename = null);

		void PasteBlock (); 

		int ReadBlock (char *name,int option); 

		void QuickPasteBlock ();

		void DeleteBlock ();

		void SetIndex (void);

		inline void ReadInfo (CFileManager& fp) { m_segmentInfo.Read (fp); }

		inline void WriteInfo (CFileManager& fp) { m_segmentInfo.Write (fp); }

		inline void ReadRobotMakerInfo (CFileManager& fp) { m_matCenInfo [0].Read (fp); }

		inline void WriteRobotMakerInfo (CFileManager& fp) { m_matCenInfo [0].Write (fp); }

		inline void ReadEquipMakerInfo (CFileManager& fp) { m_matCenInfo [1].Read (fp); }

		inline void WriteEquipMakerInfo (CFileManager& fp) { m_matCenInfo [1].Write (fp); }

		void ReadSegments (CFileManager& fp, int nFileVersion);
		
		void WriteSegments (CFileManager& fp, int nFileVersion);
		
		void ReadRobotMakers (CFileManager& fp, int nFileVersion);
		
		void WriteRobotMakers (CFileManager& fp, int nFileVersion);
		
		void ReadEquipMakers (CFileManager& fp, int nFileVersion);
		
		void WriteEquipMakers (CFileManager& fp, int nFileVersion);

		void Clear (void);

		int Fix (void);

		inline bool Full (void) { return m_free.Empty (); }

		CSegmentManager () { 
			ResetInfo ();
			Clear ();
			m_free.Create (Segment (0), SEGMENT_LIMIT);
			}

	private:
		void UnlinkChild (short nParentSeg, short nSide);

		void UnlinkSeg (CSegment *segP, CSegment *rootSegP);

		void LinkSeg (CSegment *segP, CSegment *rootSegP);

		void DeleteWalls (short nSegment);

		bool Create (short nSegment, bool bCreate, byte nFunction, short nTexture = -1, char* szError = null);

		bool Define (short nSegment, byte nFunction, short nTexture);

		void Undefine (short nSegment);

		void ComputeVertices (ushort newVerts [4]);

		int FuelCenterCount (void);

		void RemoveMatCen (CSegment* segP, CMatCenter* matCens, CMineItemInfo& info);

		bool CreateMatCen (short nSegment, bool bCreate, byte nType, bool bSetDefTextures, CMatCenter* matCens, CMineItemInfo& info, char* szError);

		void RenumberMatCens (byte nFunction, short nClass);

		void ReadMatCens (CFileManager& fp, int nFileVersion, int nClass);
		
		void WriteMatCens (CFileManager& fp, int nFileVersion, int nClass);
	
	};

extern CSegmentManager segmentManager;

//------------------------------------------------------------------------

class CSegmentIterator : public CGameItemIterator<CSegment> {
	public:
		CSegmentIterator() : CGameItemIterator (segmentManager.Segment (0), segmentManager.Count ()) {}
	};

//------------------------------------------------------------------------

#endif //__segman_h