#ifndef __segman_h
#define __segman_h

#include "carray.h"
#include "types.h"
#include "segment.h"
#include "robot.h"
#include "object.h"
#include "textures.h"
#include "poly.h"

#ifdef USE_DYN_ARRAYS

typedef CStaticArray< CSegment, SEGMENT_LIMIT > segmentList;
typedef CStaticArray< CVertex, VERTEX_LIMIT > vertexList;

#else

typedef CSegment segmentList [SEGMENT_LIMIT];
typedef CVertex vertexList [VERTEX_LIMIT];

#endif



class CSegmentManager {
public:
	segmentList	m_segments;
	ushort		m_nSegments;
	vertexList	vertices;
	ushort		m_nVertices;
	int			m_nAddMode;

public:
	inline bool IsValid (short i, short j) { return (i > 0) && (i < j); }

	// Segment and side getters
	inline segmentList& Segments (void)
		{ return m_segments; }

	inline ushort& SegCount ()
		{ return m_nSegments; }

	inline CSegment *GetSegment (int i)
		{ return IsValid (i, m_nSegments) ? &m_segments [i] : null; }

	inline CSide* GetSide (short nSegment = -1, short nSide = -1) {
		GetCurrent (nSegment, nSide);
		return &Segments (nSegment)->m_sides [nSide];
		}

	inline CSide* GetOppositeSide (short nSegment = -1, short nSide = -1) {
		GetCurrent (nSegment, nSide);
		return &Segments (nSegment)->m_sides [nSide];
		}

	void GetTextures (short nSegment, short nSide, short& nBaseTex, short& nOvlTex);

	// Vertex getters
	inline vertexList& Vertices (void)
		{ return m_vertices; }

	inline CVertex *GetVertex (int i)
		{ return IsValid (i, m_nVertices) ? &m_vertices [i] : null; }

	inline byte& VertStatus (int i = 0)
		{ return Vertices (i)->m_status; }

	inline ushort& VertCount ()
		{ return m_nVertices; }

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
	void Mark (short nSegment);
	void UpdateMarked (void);
	bool IsMarked (short nSegment);
	bool IsMarked (short nSegment, short nSide);

	bool IsPointOfSide (CSegment *segP, int nSide, int nPoint);
	bool IsLineOfSide (CSegment *segP, int nSide, int nLine);

	void Join (int automatic = 0);
	void Split (int solidify = 0, int nSide = -1);

	CDoubleVector CalcSideNormal (short nSegment = -1, short nSide = -1);
	CDoubleVector CalcSideCenter (short nSegment = -1, short nSide = -1);
	//double CalcLength (CFixVector* center1, CFixVector* center2);

	void FixChildren (void);

	short	MarkedSegmentCount (bool bCheck = false);
	bool	GotMarkedSegments (void) { return MarkedSegmentCount (true) > 0; }
	bool GotMarkedSides (void);

	int AlignTextures (short start_segment, short start_side, short only_child, BOOL bAlign1st, BOOL bAlign2nd, char bAlignedSides = 0);

	bool GetOppositeSide (short& nOppSeg, short& nOppSide, short nSegment = -1, short nSide = -1);
	CSide *OppSide (void);
	bool SetTexture (short nSegment, short nSide, short nBaseTex, short nOvlTex);
	void CopyOtherSegment (void);
	bool WallClipFromTexture (short nSegment, short nSide);
	void CheckForDoor (short nSegment, short nSide);
	void RenumberBotGens (void);
	void RenumberEquipGens (void);

	bool SetDefaultTexture (short nTexture = -1, short walltype = -1);
	bool DefineSegment (short nSegment, byte type, short nTexture, short walltype = -1);
	void UndefineSegment (short nSegment);
	void AutoLinkExitToReactor ();

	inline CSegment *OtherSeg (void)
		{ return Segments (Other ()->nSegment); }
	inline CSide *OtherSide (void)
		{ return OtherSeg ()->m_sides + Other ()->nSide; }

	short ReadSegmentInfo (CFileManager& file);
	void WriteSegmentInfo (CFileManager& file, short);
	void CutBlock ();
	void CopyBlock (char *pszBlkFile = null);
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


private:
	void UnlinkChild (short nParentSeg, short nSide);
	void ResetSide  (short nSegment, short nSide);

	void SetSegmentChildNum (CSegment *rootSegP, short nSegment, short recursionLevel);
	void SetSegmentChildNum (CSegment *rootSegP, short nSegment, short recursionLevel, short* visited);
	void UnlinkSeg (CSegment *segP, CSegment *rootSegP);
	void LinkSeg (CSegment *segP, CSegment *rootSegP);
	void DeleteWalls (short nSegment);

	};

#endif //__segman_h