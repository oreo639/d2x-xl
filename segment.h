// Segment.h

#ifndef __segment_h
#define __segment_h

#include "define.h"
#include "Types.h"
#include "side.h"
#include "FileManager.h"

// -----------------------------------------------------------------------------
// define points for a given side 

extern byte sideVertTable [6][4];
extern byte oppSideTable [6];
extern byte oppSideVertTable [6][4];
extern byte lineVertTable [12][2];
extern byte sideLineTable [6][4];
extern byte connectPointTable [8][3];
extern byte pointSideTable [8][3];
extern byte pointCornerTable [8][3];

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
#define SEGMENT_FUNC_REACTOR			3
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

typedef struct tSegment {
	ushort	verts [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
	byte		function;			// general function (robot maker, energy center, ... - mutually exclusive)
	byte		props;				// special property of a segment (such as damaging, trigger, etc.)
	char		nMatCen;				// which center segment is associated with, high bit set 
	char		value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
	byte		flags;				// internally used
	short		damage [2];
	int		staticLight;		// average static light in segment 
	byte		childFlags;			// bit0 to 5: children, bit6: unused, bit7: special 
	byte		wallFlags;			// bit0 to 5: door/walls, bit6: deleted, bit7: marked segment 
	short		mapBitmask;			// which lines are drawn when displaying wireframe 
	byte		bTunnel;				// part of a tunnel?
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

	void Read (CFileManager* fp);
	
	void Write (CFileManager* fp);
	
	void ReadExtras (CFileManager* fp, bool bExtras);
	
	void WriteExtras (CFileManager* fp, bool bExtras);
	
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		for (int i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
			m_sides [i].Clear ();
		}

	inline tSegment& Info (void) { return m_info; }

	void Setup (void);

	void SetUV (short nSide, short x, short y);

	//void Read (CFileManager* fp, bool bFlag = false) {}
	//void Write (CFileManager* fp, bool bFlag = false) {}

	inline CSide _const_ * Side (short i) _const_ { return ((i < 0) || (i > 5)) ? null : &m_sides [i]; }

	CVertex _const_ * Vertex (short nVertex) _const_;

	inline short Child (short nSide) _const_ { return m_sides [nSide].m_info.nChild; }

	short SetChild (short nSide, short nSegment);

	bool HasChild (short nChild) { return (m_info.childFlags & (1 << nChild)) != 0; }

	void Reset (short nSide = -1);

	void UpdateChildren (short nOldChild, short nNewChild);

	virtual CGameItem* Clone (void);

	virtual void Backup (eEditType editType = opModify);

	virtual CGameItem* Copy (CGameItem* destP);

	virtual void Redo (void);

	virtual void Undo (void);

	inline CUVL _const_ * Uvls (short nSide) _const_ { return Side (nSide)->Uvls (); }

	inline void Mark (byte mask = MARKED_MASK) { m_info.flags |= mask; }

	inline void Unmark (byte mask = MARKED_MASK) { m_info.flags &= ~mask; }

	inline bool IsMarked (byte mask = MARKED_MASK) { return (m_info.flags & mask) != 0; }

	inline bool IsD2X (void) { return (m_info.function >= SEGMENT_FUNC_TEAM_BLUE) || (m_info.props != 0); }

	inline CSide _const_ & operator[] (uint i) _const_ { return m_sides [i]; }

	CSegment () : CGameItem (itSegment) {}

private:
	byte ReadWalls (CFileManager* fp);

	byte WriteWalls (CFileManager* fp);

};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tMatCenter {
	int  objFlags [2]; /* Up to 32 different Descent 1 robots */
	//  int  robot_flags2;// Additional 32 robots for Descent 2
	int    hitPoints;  /* How hard it is to destroy this particular matcen */
	int    interval;    /* Interval between materializations */
	short  nSegment;      /* Segment this is attached to. */
	short  nFuelCen; /* Index in fuelcen array. */
} tMatCenter;

// -----------------------------------------------------------------------------

class CMatCenter : public CGameItem {
public:
	tMatCenter	m_info;

	void Read (CFileManager* fp, bool bFlag = false);
	
	void Write (CFileManager* fp, bool bFlag = false);

	void Setup (short nSegment, short nIndex, int nFlags) {
		Clear ();
		m_info.nSegment = nSegment;
		m_info.objFlags [0] = nFlags;
		m_info.nFuelCen = nIndex;
		};

	inline tMatCenter& Info (void) { return m_info; }

	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }

	virtual CGameItem* Clone (void);

	virtual void Backup (eEditType editType = opModify);

	virtual CGameItem* Copy (CGameItem* destP);

	inline bool operator< (CMatCenter& other) { return m_info.nSegment < other.m_info.nSegment; }

	inline bool operator> (CMatCenter& other) { return m_info.nSegment > other.m_info.nSegment; }

	inline CMatCenter& operator= (CMatCenter& other) { 
		m_info = other.m_info; 
		return *this;
		}

	//virtual void Undo (void);

	//virtual void Redo (void);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif //__segment_h