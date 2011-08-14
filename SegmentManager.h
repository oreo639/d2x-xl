#ifndef __segman_h
#define __segman_h

#include "define.h"
#include "FileManager.h"
#include "carray.h"
#include "Types.h"
#include "Selection.h"
#include "MineInfo.h"
#include "FreeList.h"

//------------------------------------------------------------------------

#define MAX_SEGMENTS_D1		800  // descent 1 max # of cubes
#define MAX_SEGMENTS_D2		900  // descent 2 max # of cubes
#define SEGMENT_LIMIT		8000 // D2X-XL max # of cubes

#define MAX_SEGMENTS ((theMine == null) ? MAX_SEGMENTS_D2 : DLE.IsD1File () ? MAX_SEGMENTS_D1  : DLE.IsStdLevel () ? MAX_SEGMENTS_D2 : SEGMENT_LIMIT)

#define MAX_NUM_MATCENS_D1			20
#define MAX_NUM_MATCENS_D2			100

#define MAX_NUM_RECHARGERS_D2		70
#define MAX_NUM_RECHARGERS_D2X	500

#define MAX_NUM_RECHARGERS ((theMine == null) ? MAX_NUM_RECHARGERS_D2X : (DLE.IsD1File () || (DLE.LevelVersion () < 12)) ? MAX_NUM_RECHARGERS_D2 : MAX_NUM_RECHARGERS_D2X)

//------------------------------------------------------------------------

#ifdef _DEBUG

typedef CStaticArray< CSegment, SEGMENT_LIMIT > segmentList;
typedef CStaticArray< CMatCenter, MAX_NUM_MATCENS_D2 > robotMakerList;

#else

typedef CSegment segmentList [SEGMENT_LIMIT];
typedef CStaticArray<CMatCenter, MAX_NUM_MATCENS_D2> robotMakerList;

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
		FREELIST(CSegment)
		int						m_nAddMode;
		robotMakerList			m_matCens [2];
		CMineItemInfo			m_matCenInfo [2];
		bool						m_bCreating;

	public:
		inline void ResetInfo (void) {
			m_segmentInfo.Reset ();
			m_matCenInfo [0].Reset ();
			m_matCenInfo [1].Reset ();
#if USE_FREELST
			m_free.Reset ();
#endif
			}
		// Segment and side getters
		inline segmentList& Segments (void)	{ return m_segments; }

		inline int& Count (void) { return m_segmentInfo.count; }

		inline int& FileOffset (void) { return m_segmentInfo.offset; }

		inline int& MatCenCount (_const_ short nClass) { return m_matCenInfo [nClass].count; }

		inline int& RobotMakerCount (void) { return MatCenCount (0); }

		inline int& EquipMakerCount (void) { return MatCenCount (1); }

		int FuelCenterCount (void);

		inline short Index (CSegment _const_ * segP) { return (short) (segP - &m_segments [0]); }

		inline CSegment _const_ * Segment (_const_ int i) { return &m_segments [i]; }

		inline CSegment _const_ * Segment (CSideKey& key) { return Segment (key.m_nSegment); }

		inline CSegment _const_ & GetSegment (_const_ int i) { return m_segments [i]; }

		inline void PutSegment (_const_ int i, _const_ CSegment& seg) { m_segments [i] = seg; }

		inline CSide _const_ * Side (CSideKey key) {
			current->Get (key);
			return m_segments [key.m_nSegment].m_sides + key.m_nSide;
			}

		inline CSide _const_ * Side (void) { return Side (CSideKey ()); }

		inline CSide _const_ & GetSide (CSideKey key) { return *Side (key); }

		inline void PutSide (CSideKey key, CSide& side) { 
			current->Get (key);
			m_segments [key.m_nSegment].m_sides [key.m_nSide] = side;
			}

		CSide _const_ * OppositeSide (CSideKey key, CSideKey& opp);

		inline CSide _const_ * OppositeSide (CSideKey& opp) { return OppositeSide (CSideKey (), opp); }
		//CSide* OppositeSide (CSideKey opp, CSideKey& key, short nSegment = -1, short nSide = -1);

		//inline CWall* Wall (short nSegment = -1, short nSide = -1) {
		inline CWall _const_ * Wall (CSideKey key) { 
			current->Get (key);
			CSide _const_ * sideP;
			sideP = Side (key);
			return sideP->Wall (); 
			}

		inline CWall _const_ * Wall (CSideKey* key = null) { return Wall ((key == null) ? *current : *key); }

		int IsWall (CSideKey key = CSideKey ());

		//inline CTrigger* Trigger (short nSegment = -1, short nSide = -1) {
		inline CTrigger _const_ * Trigger (CSideKey key) { return Side (key)->Trigger (); }

		inline CTrigger _const_ * Trigger (void) { return Trigger(CSideKey ()); }

		//inline CWall* OppositeWall (short nSegment = -1, short nSide = -1) {
		inline _const_ CWall* OppositeWall (CSideKey key) {
			_const_ CSide* sideP = Side (key);
			return (sideP == null) ? null : sideP->Wall ();
			}

		inline _const_ CWall* OppositeWall (void) { return OppositeWall (CSideKey ()); }

		void Textures (CSideKey key, short& nBaseTex, short& nOvlTex);

		inline robotMakerList& RobotMakers (void) { return m_matCens [0]; }
		
		inline robotMakerList& EquipMakers (void) { return m_matCens [1]; }
		
		inline CMatCenter* MatCenter (int i, int nClass) { return &m_matCens [nClass][i]; }
		
		inline CMatCenter* RobotMaker (int i) { return MatCenter (i, 0); }
		
		inline CMatCenter* EquipMaker (int i) { return MatCenter (i, 1); }

		inline int SetAddMode (int nMode) { return m_nAddMode = nMode; }

		inline int AddMode (void) { return m_nAddMode; }

		CSegment* FindRobotMaker (short i = 0);

		// Operations
		short Add (void);

		void Remove (short nDelSeg);

		short Create (void);

		void Delete (short nDelSeg = -1, bool bDeleteVerts = true);

		void ResetSide (short nSegment, short nSide);

		bool CreateReactor (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateRobotMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateEquipMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateGoal (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture);

		bool CreateTeam (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture);

		bool CreateSpeedBoost (short nSegment, bool bCreate);

		bool CreateSkybox (short nSegment, bool bCreate);

		short CreateFuelCenter (short nSegment = -1, byte nType = SEGMENT_FUNC_FUELCEN, bool bCreate = true, bool bSetDefTextures = true); 

		void Init (short nSegment);

		bool Subdivide (void);

		void Join (CSideKey key, bool bFind);

		bool Link (short nSegment1, short nSide1, short nSegment2, short nSide2, double margin);

		void LinkSides (short nSegment1, short nSide1, short nSegment2, short nSide2, tVertMatch match [4]);

		CVertex& CalcCenter (CVertex& pos, short nSegment);

		bool IsPointOfSide (CSegment *segP, int nSide, int nPoint);

		bool IsLineOfSide (CSegment *segP, int nSide, int nLine);

		void JoinSegments (int automatic = 0);

		void SeparateSegments (int solidify = 0, int nSide = -1);

		void JoinLines (void);

		void JoinPoints (void);

		void SeparateLines (void);

		void SeparatePoints (void);

		CDoubleVector CalcSideNormal (CSideKey key = CSideKey (-1, -1));

		CDoubleVector CalcSideCenter (CSideKey key = CSideKey (-1, -1));
		//double CalcLength (CFixVector* center1, CFixVector* center2);

		void FixChildren (void);

		void UpdateWalls (short nOldWall, short nNewWall);

		void UpdateVertices (short nOldVert, short nNewVert);

		void Mark (short nSegment);

		void MarkAll (byte mask = MARKED_MASK);

		void UnmarkAll (byte mask = MARKED_MASK);

		void MarkSelected (void);

		void UpdateMarked (void);

		bool IsMarked (short nSegment);

		bool IsMarked (CSideKey key = CSideKey (-1, -1));

		short	MarkedCount (bool bCheck = false);

		bool HaveMarkedSegments (void) { return MarkedCount (true) > 0; }

		bool HaveMarkedSides (void);

		int AlignTextures (short nStartSeg, short nStartSide, short onlyChild, int bAlign1st, int bAlign2nd, char bAlignedSides = 0);

		CSide* OppSide (void);

		bool SetTextures (CSideKey key, short nBaseTex, short nOvlTex);

		void CopyOtherSegment (void);

		void RenumberRobotMakers (void);

		void RenumberEquipMakers (void);

		bool SetDefaultTexture (short nTexture);

		void SetLinesToDraw (void);

		short ReadSegmentInfo (CFileManager* fp);

		void WriteSegmentInfo (CFileManager* fp, short);

		void CutBlock ();

		void CopyBlock (char *filename = null);

		void PasteBlock (); 

		int ReadBlock (char *name,int option); 

		void QuickPasteBlock ();

		void DeleteBlock ();

		void SetIndex (void);

		inline void ReadInfo (CFileManager* fp) { m_segmentInfo.Read (fp); }

		inline void WriteInfo (CFileManager* fp) { m_segmentInfo.Write (fp); }

		inline void ReadRobotMakerInfo (CFileManager* fp) { m_matCenInfo [0].Read (fp); }

		inline void WriteRobotMakerInfo (CFileManager* fp) { m_matCenInfo [0].Write (fp); }

		inline void ReadEquipMakerInfo (CFileManager* fp) { m_matCenInfo [1].Read (fp); }

		inline void WriteEquipMakerInfo (CFileManager* fp) { m_matCenInfo [1].Write (fp); }

		void ReadSegments (CFileManager* fp);
		
		void WriteSegments (CFileManager* fp);
		
		void ReadRobotMakers (CFileManager* fp);
		
		void WriteRobotMakers (CFileManager* fp);
		
		void ReadEquipMakers (CFileManager* fp);
		
		void WriteEquipMakers (CFileManager* fp);

		void RenumberFuelCenters (void);

		void Clear (void);

		int Fix (void);

		void Undefine (short nSegment);

#if USE_FREELIST
		inline bool Full (void) { return m_free.Empty (); }
#else
		bool Full (void);
#endif

		CSegmentManager () : m_bCreating (false) { 
			ResetInfo ();
			Clear ();
#if USE_FREELST
			m_free.Create (Segment (0), SEGMENT_LIMIT);
#endif
			}

	private:
		void UnlinkChild (short nParentSeg, short nSide);

		void UnlinkSeg (CSegment *segP, CSegment *rootSegP);

		void LinkSeg (CSegment *segP, CSegment *rootSegP);

		void DeleteWalls (short nSegment);

		short Create (short nSegment, bool bCreate, byte nFunction, short nTexture = -1, char* szError = null);

		bool Define (short nSegment, byte nFunction, short nTexture);

		void ComputeVertices (ushort newVerts [4]);

		void RemoveMatCenter (CSegment* segP, CMatCenter* matCens, CMineItemInfo& info, int nFunction);

		bool CreateMatCen (short nSegment, bool bCreate, byte nType, bool bSetDefTextures, CMatCenter* matCens, CMineItemInfo& info, char* szError);

		void RenumberMatCens (byte nFunction, short nClass);

		void ReadMatCens (CFileManager* fp, int nClass);
		
		void WriteMatCens (CFileManager* fp, int nClass);
	
	};

extern CSegmentManager segmentManager;

//------------------------------------------------------------------------

class CSegmentIterator : public CGameItemIterator<CSegment> {
	public:
		CSegmentIterator() : CGameItemIterator (const_cast<CSegment*>(segmentManager.Segment (0)), segmentManager.Count ()) {}
	};

//------------------------------------------------------------------------

#endif //__segman_h