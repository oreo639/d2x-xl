// Segment.h

#ifndef __segment_h
#define __segment_h

#include "define.h"
#include "Types.h"
#include "cfile.h"

// -----------------------------------------------------------------------------
// define points for a given side 

extern byte sideVertTable[6][4];
extern byte oppSideTable[6];
extern byte oppSideVertTable[6][4];
extern byte lineVertTable[12][2];
extern byte sideLineTable[6][4];
extern byte connectPointTable[8][3];
extern byte pointSideTable[8][3];
extern byte pointCornerTable[8][3];

// -----------------------------------------------------------------------------
//  Returns true if nSegment references a child, else returns false. 
//  Note that -1 means no connection, -2 means a connection to the outside world. 
#define  IS_CHILD(nSegment) (nSegment > -1)

#define SEGMENT_TYPE_NONE				0
#define SEGMENT_TYPE_FUELCEN			1
#define SEGMENT_TYPE_REPAIRCEN		2
#define SEGMENT_TYPE_CONTROLCEN		3
#define SEGMENT_TYPE_ROBOTMAKER		4
#define MAX_SEGMENT_TYPES1				5

#define SEGMENT_TYPE_GOAL_BLUE		5 // Descent 2 only
#define SEGMENT_TYPE_GOAL_RED			6 // Descent 2 only
#define SEGMENT_TYPE_WATER				7
#define SEGMENT_TYPE_LAVA				8
#define SEGMENT_TYPE_TEAM_BLUE		9
#define SEGMENT_TYPE_TEAM_RED			10
#define SEGMENT_TYPE_SPEEDBOOST		11
#define SEGMENT_TYPE_BLOCKED			12
#define SEGMENT_TYPE_NODAMAGE			13
#define SEGMENT_TYPE_SKYBOX			14
#define SEGMENT_TYPE_EQUIPMAKER		15	// matcen for powerups
#define SEGMENT_TYPE_OUTDOORS			16
#define MAX_SEGMENT_TYPES2				17 // Descent 2 only

#define SEGMENT_FUNC_NONE				0
#define SEGMENT_FUNC_FUELCEN			1
#define SEGMENT_FUNC_REPAIRCEN		2
#define SEGMENT_FUNC_CONTROLCEN		3
#define SEGMENT_FUNC_ROBOTMAKER		4
#define SEGMENT_FUNC_GOAL_BLUE		5
#define SEGMENT_FUNC_GOAL_RED			6
#define SEGMENT_FUNC_TEAM_BLUE		7
#define SEGMENT_FUNC_TEAM_RED			8
#define SEGMENT_FUNC_SPEEDBOOST		9
#define SEGMENT_FUNC_SKYBOX			10
#define SEGMENT_FUNC_EQUIPMAKER		11
#define MAX_SEGMENT_FUNCTIONS			12

#define SEGMENT_PROP_NONE				0
#define SEGMENT_PROP_WATER				1
#define SEGMENT_PROP_LAVA				2
#define SEGMENT_PROP_BLOCKED			4
#define SEGMENT_PROP_NODAMAGE			8
#define SEGMENT_PROP_OUTDOORS			16

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CVertex : public CDoubleVector, public CGameItem {
public:
	byte m_status;
	CVertex () : m_status(0) {}
	CVertex (double x, double y, double z) : CDoubleVector (x, y, z) { m_status = 0; }
	CVertex (tDoubleVector& _v) : CDoubleVector (_v) { m_status = 0; }
	CVertex (CDoubleVector _v) : CDoubleVector (_v) { m_status = 0; }

	virtual CGameItem* Next (void) { return this + 1; }
	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false) { fp.ReadVector (v); }
	virtual void Write (CFileManager& fp, int version = 0, bool bFlag = false) { fp.WriteVector (v); }
	virtual void Clear (void) { 
		m_status = 0;
		this->CDoubleVector::Clear ();
		}

	inline const CVertex& operator= (const CVertex& other) { 
		v = other.v, m_status = other.m_status; 
		return *this;
		}
	inline const CVertex& operator= (const CDoubleVector& other) { 
		v = other.v; 
		return *this;
		}
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tUVL {
public:
	short u, v, l; 
} tUVL;

class CUVL : public tUVL {
public:
	inline void Read (CFileManager& fp) {
		u = fp.ReadInt16 ();
		v = fp.ReadInt16 ();
		l = fp.ReadInt16 ();
		}
	inline void Write (CFileManager& fp) {
		fp.Write (u);
		fp.Write (v);
		fp.Write (l);
		}

	inline void Clear (void) { u = v = l = 0; }
	inline void Set (short _u, short _v, short _l) { u = _u, v = _v, l = _l; }
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tSide {
	short		nChild;
	ushort	nWall;		// (was short) Index into Walls array, which wall (probably door) is on this side 
	short		nBaseTex;	// Index into array of textures specified in bitmaps.bin 
	short		nOvlTex;		// Index, as above, texture which gets overlaid on nBaseTex 
	CUVL		uvls [4];   // CUVL coordinates at each point 
} tSide;

// -----------------------------------------------------------------------------

class CSide {
public:
	tSide m_info;

	void Read (CFileManager& fp, bool bTextured);
	void Write (CFileManager& fp);
	void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	void Setup (void);
	void LoadTextures (void);
	void GetTextures (short &nBaseTex, short &nOvlTex);
	bool SetTextures (short nBaseTex, short nOvlTex);
	void CSide::InitUVL (void);
	inline void SetWall (short nWall) { m_info.nWall = nWall; }
	CWall* GetWall (void);
	CTrigger* GetTrigger (void);
	bool CSide::IsVisible (void);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tSegment {
	short		verts [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
	byte		function;			// special property of a segment (such as damaging, trigger, etc.) 
	byte		props;
	char		nMatCen;				// which center segment is associated with, high bit set 
	char		value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
	byte		s2Flags;			// New for Descent 2
	short		damage [2];
	int		staticLight;		// average static light in segment 
	byte		childFlags;			// bit0 to 5: children, bit6: unused, bit7: special 
	byte		wallFlags;			// bit0 to 5: door/walls, bit6: deleted, bit7: marked segment 
	short		nIndex;				// used for cut & paste to help link children 
	short		mapBitmask;		// which lines are drawn when displaying wireframe 
	char		owner;
	char		group;
} tSegment;

// -----------------------------------------------------------------------------

class CSegment : public CGameItem {
public:
	tSegment	m_info;
	CSide		m_sides [MAX_SIDES_PER_SEGMENT];		// 6 sides 

public:
	void Upgrade (void);
	void Read (CFileManager& fp, int nLevelType, int nLevelVersion);
	void ReadExtras (CFileManager& fp, int nLevelType, int nLevelVersion, bool bExtras);
	void Write (CFileManager& fp, int nLevelType, int nLevelVersion);
	void WriteExtras (CFileManager& fp, int nLevelType, bool bExtras);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		for (int i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
			m_sides [i].Clear ();
		}
	void Setup (void);
	void SetUV (short nSide, short x, short y);

	virtual CGameItem* Next (void) { return this + 1; }
	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false) {};
	virtual void Write (CFileManager& fp, int version = 0, bool bFlag = false) {};
	inline CSide* GetSide (short i) { return ((i < 0) || (i > 5)) ? null : &m_sides [i]; }
	inline short GetChild (short nSide) { return m_sides [nSide].m_info.nChild; }
	short SetChild (short nSide, short nSegment);

private:
	byte ReadWalls (CFileManager& fp, int nLevelVersion);
	byte WriteWalls (CFileManager& fp, int nLevelVersion);

};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tRobotMaker {
	int  objFlags [2]; /* Up to 32 different Descent 1 robots */
	//  int  robot_flags2;// Additional 32 robots for Descent 2
	int    hitPoints;  /* How hard it is to destroy this particular matcen */
	int    interval;    /* Interval between materializations */
	short  nSegment;      /* Segment this is attached to. */
	short  nFuelCen; /* Index in fuelcen array. */
} tRobotMaker;

// -----------------------------------------------------------------------------

class CRobotMaker : public CGameItem {
public:
	tRobotMaker	m_info;

	void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	void Write (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif //__segment_h