#ifndef __segman_h
#define __segman_h

#include "define.h"
#include "FileManager.h"
#include "carray.h"
#include "Types.h"
#include "Selection.h"
#include "MineInfo.h"
#include "FreeList.h"
#include "SLL.h"
#include "AVLTree.h"

// -----------------------------------------------------------------------------

#define MAX_SEGMENTS_D1		800  // descent 1 max # of cubes
#define MAX_SEGMENTS_D2		900  // descent 2 max # of cubes
#define SEGMENT_LIMIT		20000 // D2X-XL max # of cubes

#define MAX_SEGMENTS ((theMine == null) ? MAX_SEGMENTS_D2 : DLE.IsD1File () ? MAX_SEGMENTS_D1  : DLE.IsStdLevel () ? MAX_SEGMENTS_D2 : SEGMENT_LIMIT)

#define MAX_NUM_MATCENS_D1			20
#define MAX_NUM_MATCENS_D2			100

#define MAX_NUM_RECHARGERS_D2		70
#define MAX_NUM_RECHARGERS_D2X	500

#define MAX_NUM_RECHARGERS ((theMine == null) ? MAX_NUM_RECHARGERS_D2X : (DLE.IsD1File () || (DLE.LevelVersion () < 12)) ? MAX_NUM_RECHARGERS_D2 : MAX_NUM_RECHARGERS_D2X)

// -----------------------------------------------------------------------------

#ifdef _DEBUG

typedef CStaticArray< CSegment, SEGMENT_LIMIT > segmentList;
typedef CStaticArray< CObjectProducer, MAX_NUM_MATCENS_D2 > robotMakerList;

#else

typedef CSegment segmentList [SEGMENT_LIMIT];
typedef CStaticArray<CObjectProducer, MAX_NUM_MATCENS_D2> robotMakerList;

#endif

// -----------------------------------------------------------------------------

typedef struct tVertMatch {
		short		v;
		double	d;
	} tVertMatch; 

// -----------------------------------------------------------------------------

class CEdgeTreeNode {
	public:
		uint								m_nKey;
		CSLL<CSideKey, CSideKey>	m_sides;

	explicit CEdgeTreeNode (uint nKey = 0) : m_nKey (nKey) {}
	~CEdgeTreeNode () {}
	void Destroy (void) { m_sides.Destroy (); }

	CSideKey* Insert (short nSegment, short nSide) {
		CSideKey	key = CSideKey (nSegment, nSide);
		return m_sides.Insert (key, key);
		}

	inline CEdgeTreeNode& operator= (CEdgeTreeNode& other) {
		m_nKey = other.m_nKey;
		m_sides = other.m_sides;
		return *this;
		}
	inline bool operator< (uint key) { return m_nKey < key; }
	inline bool operator> (uint key) { return m_nKey > key; }
	};

// -----------------------------------------------------------------------------

class CSegmentManager {
	public:
		segmentList				m_segments;
		CMineItemInfo			m_segmentInfo;
		FREELIST(CSegment)
		int						m_nAddMode;
		robotMakerList			m_producers [2];
		CMineItemInfo			m_producerInfo [2];
		bool						m_bCreating;
		CSegment*				m_selectedSegments;
		CSide*					m_selectedSides;

	public:
		inline void ResetInfo (void) {
			m_segmentInfo.Reset ();
			m_producerInfo [0].Reset ();
			m_producerInfo [1].Reset ();
#if USE_FREELST
			m_free.Reset ();
#endif
			}
		// Segment and side getters
		inline segmentList& Segments (void)	{ return m_segments; }

		inline int& Count (void) { return m_segmentInfo.count; }

		inline int& FileOffset (void) { return m_segmentInfo.offset; }

		inline int& ProducerCount (_const_ short nClass) { return m_producerInfo [nClass].count; }

		inline int& RobotMakerCount (void) { return ProducerCount (0); }

		inline int& EquipMakerCount (void) { return ProducerCount (1); }

		int ProducerCount (void);

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

		CSide _const_ * BackSide (CSideKey key, CSideKey& back);

		inline CSide _const_ * BackSide (CSideKey& back) { return BackSide (CSideKey (), back); }
		//CSide* OppositeSide (CSideKey opp, CSideKey& key, short nSegment = -1, short nSide = -1);

		//inline CWall* Wall (short nSegment = -1, short nSide = -1) {
		inline CWall _const_ * Wall (CSideKey key) { 
			current->Get (key);
			CSide _const_ * sideP;
			sideP = Side (key);
			return sideP->Wall (); 
			}

		inline CWall _const_ * Wall (CSideKey* key = null) { return Wall ((key == null) ? *current : *key); }

		bool IsExit (short nSegment);

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

		inline robotMakerList& RobotMakers (void) { return m_producers [0]; }
		
		inline robotMakerList& EquipMakers (void) { return m_producers [1]; }
		
		inline CObjectProducer* Producerter (int i, int nClass) { return &m_producers [nClass][i]; }
		
		inline CObjectProducer* RobotMaker (int i) { return Producerter (i, 0); }
		
		inline CObjectProducer* EquipMaker (int i) { return Producerter (i, 1); }

		inline int SetAddMode (int nMode) { return m_nAddMode = nMode; }

		inline int AddMode (void) { return m_nAddMode; }

		CSegment* FindRobotMaker (short i = 0);

		// Operations
		short Add (void);

		void Remove (short nDelSeg);

		void AddSegments (void);

		short Create (CSideKey key, int addMode = -1);

		void Delete (short nDelSeg = -1, bool bDeleteVerts = true);

		void ResetSide (short nSegment, short nSide);

		bool CreateReactor (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateRobotMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateEquipMaker (short nSegment = -1, bool bCreate = true, bool bSetDefTextures = true); 

		bool CreateGoal (short nSegment, bool bCreate, bool bSetDefTextures, ubyte nType, short nTexture);

		bool CreateTeam (short nSegment, bool bCreate, bool bSetDefTextures, ubyte nType, short nTexture);

		bool CreateSpeedBoost (short nSegment, bool bCreate);

		bool CreateSkybox (short nSegment, bool bCreate);

		short CreateProducer (short nSegment = -1, ubyte nType = SEGMENT_FUNC_PRODUCER, bool bCreate = true, bool bSetDefTextures = true); 

		void Init (short nSegment);

		bool SplitIn7 (void);

		bool SplitIn8 (CSegment* rootSegP);

		bool CreateWedge (void);

		bool CreatePyramid (void);

		void Join (CSideKey key, bool bFind);

		bool Link (short nSegment1, short nSide1, short nSegment2, short nSide2, double margin);

		void LinkSides (short nSegment1, short nSide1, short nSegment2, short nSide2, tVertMatch match [4]);

		CVertex& CalcCenter (CVertex& pos, short nSegment);

		bool IsPointOfSide (CSegment *segP, int nSide, int nPoint);

		bool IsLineOfSide (CSegment *segP, int nSide, int nLine);

		void JoinSegments (int automatic = 0);

		bool SeparateSegments (int solidify = 0, int nSide = -1, bool bVerbose = true);

		void JoinLines (void);

		void JoinPoints (void);

		void SeparateLines (short nLine = -1);

		int SeparatePoints (CSideKey key = CSideKey (-1, -1), int nVertexId = -1, bool bVerbose = true);

		CDoubleVector CalcSideNormal (CSideKey key = CSideKey (-1, -1));

		CDoubleVector CalcSideCenter (CSideKey key = CSideKey (-1, -1));
		//double CalcLength (CFixVector* center1, CFixVector* center2);

		void FixChildren (void);

		void UpdateWalls (short nOldWall, short nNewWall);

		void UpdateVertices (short nOldVert, short nNewVert);

		void Tag (short nSegment);

		void TagAll (ubyte mask = TAGGED_MASK);

		void UnTagAll (ubyte mask = TAGGED_MASK);

		void TagPlane (void);

		void TagSelected (void);

		void UpdateTagged (void);

		bool IsTagged (short nSegment);

		bool IsTagged (CSideKey key = CSideKey (-1, -1));

		short	TaggedCount (bool bSides = false, bool bCheck = false);

		bool HaveTaggedSegments (bool bSides = false) { return TaggedCount (bSides, true) > 0; }

		bool HaveTaggedSides (void);

		int AlignTextures (short nStartSeg, short nStartSide, short nChildSeg, short nChildSide, int bAlign1st, int bAlign2nd);

		CSide* OppSide (void);

		bool SetTextures (CSideKey key, int nBaseTex, int nOvlTex);

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

		inline void ReadRobotMakerInfo (CFileManager* fp) { m_producerInfo [0].Read (fp); }

		inline void WriteRobotMakerInfo (CFileManager* fp) { m_producerInfo [0].Write (fp); }

		inline void ReadEquipMakerInfo (CFileManager* fp) { m_producerInfo [1].Read (fp); }

		inline void WriteEquipMakerInfo (CFileManager* fp) { m_producerInfo [1].Write (fp); }

		void ReadSegments (CFileManager* fp);
		
		void WriteSegments (CFileManager* fp);
		
		void ReadRobotMakers (CFileManager* fp);
		
		void WriteRobotMakers (CFileManager* fp);
		
		void ReadEquipMakers (CFileManager* fp);
		
		void WriteEquipMakers (CFileManager* fp);

		void RenumberProducers (void);

		void Clear (void);

		int Fix (void);

		void Undefine (short nSegment);

		void DeleteD2X (void);

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

		int Overflow (int nSegments = -1);

		short FindByVertex (ushort nVertex, short nSegment);

		void UpdateTexCoords (ushort nVertexId, bool bMove, short nIgnoreSegment = -1, short nIgnoreSide = -1);

		bool VertexInUse (ushort nVertex, short nIgnoreSegment = -1);

		bool CollapseEdge (short nSegment = -1, short nSide = -1, short nEdge = -1, bool bUpdateCoord = true);

		void ComputeNormals (bool bAll, bool bView = false);

		CSegment* GatherSelectableSegments (CRect& viewport, long xMouse, long yMouse, bool bAllowSkyBox);

		CSide* GatherSelectableSides (CRect& viewport, long xMouse, long yMouse, bool bAllowSkyBox, bool bSegments);

		inline CSegment* SelectedSegments (void) { return m_selectedSegments; }
	
		inline CSide* SelectedSides (void) { return m_selectedSides; }
	
		void CSegmentManager::GatherEdges (CAVLTree <CEdgeTreeNode, uint>& edgeTree);

		uint CSegmentManager::VisibleSideCount (void);

	private:
		void UnlinkChild (short nParentSeg, short nSide);

		void UnlinkSeg (CSegment *segP, CSegment *rootSegP);

		void LinkSeg (CSegment *segP, CSegment *rootSegP);

		void DeleteWalls (short nSegment);

		short Create (short nSegment, bool bCreate, ubyte nFunction, short nTexture = -1, char* szError = null);

		bool Define (short nSegment, ubyte nFunction, short nTexture);

		short ComputeVertices (ushort newVerts [4], int addMode = -1);

		short ComputeVerticesOrtho (ushort newVerts [4]);
		
		short ComputeVerticesExtend (ushort newVerts [4]);
		
		short ComputeVerticesMirror (ushort newVerts [4]);

		bool FindNearbySide (CSideKey thisKey, CSideKey& otherKey, short& thisPoint, short& otherPoint, tVertMatch* match);

		void RemoveProducer (CSegment* segP, CObjectProducer* producers, CMineItemInfo& info, int nFunction);

		bool CreateProducer (short nSegment, bool bCreate, ubyte nType, bool bSetDefTextures, CObjectProducer* producers, CMineItemInfo& info, char* szError);

		void RenumberProducers (ubyte nFunction, short nClass);

		void ReadProducers (CFileManager* fp, int nClass);
		
		void WriteProducers (CFileManager* fp, int nClass);

	};

extern CSegmentManager segmentManager;

// -----------------------------------------------------------------------------

class CSegmentIterator : public CGameItemIterator<CSegment> {
	public:
		CSegmentIterator() : CGameItemIterator (const_cast<CSegment*>(segmentManager.Segment (0)), segmentManager.Count ()) {}
	};

// -----------------------------------------------------------------------------

#endif //__segman_h