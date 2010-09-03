#ifndef __segman_h
#define __segman_h

#include "define.h"
#include "cfile.h"
#include "carray.h"
#include "Types.h"
#include "Selection.h"
#include "MineInfo.h"

#define MAX_SEGMENTS_D1		800  // descent 1 max # of cubes
#define MAX_SEGMENTS_D2		900  // descent 2 max # of cubes
#define SEGMENT_LIMIT		8000 // D2X-XL max # of cubes

#define MAX_SEGMENTS ((theMine == null) ? MAX_SEGMENTS_D2 : theMine->IsD1File () ? MAX_SEGMENTS_D1  : theMine->IsStdLevel () ? MAX_SEGMENTS_D2 : SEGMENT_LIMIT)

#ifdef _DEBUG

typedef CStaticArray< CSegment, SEGMENT_LIMIT > segmentList;

#else

typedef CSegment segmentList [SEGMENT_LIMIT];

#endif



class CSegmentManager {
	public:
		segmentList	m_segments;
		ushort		m_nCount;
		int			m_nAddMode;

	public:
		inline bool IsValid (short i, short j) { return (i >= 0) && (i < j); }

		// Segment and side getters
		inline segmentList& Segments (void)
			{ return m_segments; }

		inline ushort& Count ()
			{ return m_nCount; }

		inline CSegment *GetSegment (int i) { return &m_segments [i]; }

		inline CSide* GetSide (CSideKey key) {
			current.Get (key);
			return &GetSegment (key.m_nSegment)->m_sides [key.m_nSide];
			}

		inline CSide* GetSide (short nSegment = -1, short nSide = -1) {
			current.Get (nSegment, nSide);
			return &GetSegment (nSegment)->m_sides [nSide];
			}

		CSide* GetOppositeSide (short nSegment = -1, short nSide = -1);

		bool GetOppositeSide (CSideKey& key, short nSegment = -1, short nSide = -1);

		inline CWall* GetWall (short nSegment = -1, short nSide = -1) {
			return GetSide (nSegment, nSide)->GetWall ();
			}

		inline CTrigger* GetTrigger (short nSegment = -1, short nSide = -1) {
			return GetSide (nSegment, nSide)->GetTrigger ();
			}

		inline CWall* GetOppositeWall (short nSegment = -1, short nSide = -1) {
			CSide* sideP = GetSide (nSegment, nSide);
			return (sideP == null) ? null : GetSide (nSegment, nSide)->GetWall ();
			}

		void GetTextures (short nSegment, short nSide, short& nBaseTex, short& nOvlTex);

		inline int SetAddMode (int nMode) { return m_nAddMode = nMode; }
		inline int GetAddMode (void) { return m_nAddMode; }

		// Operations
		bool Add (void);
		void Delete (short nDelSeg = -1);

		bool AddReactor (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 
		bool AddRobotMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 
		bool AddEquipMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 
		bool AddFuelCenter (short nSegment = -1, byte nType = SEGMENT_FUNC_FUELCEN, bool bCreate = true, bool bSetDefTextures = true); 
		bool AddGoal (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture);
		bool AddTeam (short nSegment, bool bCreate, bool bSetDefTextures, byte nType, short nTexture);
		bool AddSpeedBoost (short nSegment, bool bCreate);
		bool AddSkybox (short nSegment, bool bCreate);

		void Init (short nSegment);
		bool Split (void);
		bool Link (short nSegment1, short nSide1, short nSegment2, short nSide2, double margin);
		void CalcCenter (CVertex& pos, short nSegment);

		bool IsPointOfSide (CSegment *segP, int nSide, int nPoint);
		bool IsLineOfSide (CSegment *segP, int nSide, int nLine);

		void JoinSegments (int automatic = 0);
		void SplitSegments (int solidify = 0, int nSide = -1);
		void JoinLines (void);
		void JoinPoints (void);
		void SplitLines (void);
		void SplitPoints (void);

		CDoubleVector CalcSideNormal (short nSegment = -1, short nSide = -1);
		CDoubleVector CalcSideCenter (short nSegment = -1, short nSide = -1);
		//double CalcLength (CFixVector* center1, CFixVector* center2);

		void FixChildren (void);
		void UpdateWall (short nOldWall, short nNewWall);
		void UpdateVertex (short nOldVert, short nNewVert);

		void Mark (short nSegment);
		void UpdateMarked (void);
		bool IsMarked (short nSegment);
		bool IsMarked (short nSegment, short nSide);
		short	MarkedCount (bool bCheck = false);
		bool HaveMarkedSegments (void) { return MarkedCount (true) > 0; }
		bool HaveMarkedSides (void);

		int AlignTextures (short nStartSeg, short nStartSide, short onlyChild, bool bAlign1st, bool bAlign2nd, char bAlignedSides = 0);

		CSide* OppSide (void);
		bool SetTextures (short nSegment, short nSide, short nBaseTex, short nOvlTex);
		void CopyOtherSegment (void);
		void RenumberBotGens (void);
		void RenumberEquipGens (void);

		bool SetDefaultTexture (short nTexture = -1, short wallType = -1);
		bool DefineSegment (short nSegment, byte type, short nTexture, short wallType = -1);
		void UndefineSegment (short nSegment);

		void SetLight (double fLight, bool bAll = false, bool bDynSegLights = false);

		short ReadSegmentInfo (CFileManager& fp);
		void WriteSegmentInfo (CFileManager& fp, short);
		void CutBlock ();
		void CopyBlock (char *filename = null);
		void PasteBlock (); 
		int ReadBlock (char *name,int option); 
		void QuickPasteBlock ();
		void DeleteBlock ();

		inline void Wrap (short *x, short delta,short min,short max) {
			*x += delta;
			if (*x > max)
				*x = min;
			else if (*x < min)
				*x = max;
			}

		void SetLight (double fLight, bool bAll, bool bDynSegLights);

		void Read (CFileManager& fp, CMineItemInfo& info, int nFileVersion);
		void Write (CFileManager& fp, CMineItemInfo& info, int nFileVersion);

	private:
		void UnlinkChild (short nParentSeg, short nSide);
		void ResetSide  (short nSegment, short nSide);

		void SetSegmentChildNum (CSegment *rootSegP, short nSegment, short recursionLevel);
		void SetSegmentChildNum (CSegment *rootSegP, short nSegment, short recursionLevel, short* visited);
		void UnlinkSeg (CSegment *segP, CSegment *rootSegP);
		void LinkSeg (CSegment *segP, CSegment *rootSegP);
		void DeleteWalls (short nSegment);

		};

extern CSegmentManager segmentManager;

#endif //__segman_h