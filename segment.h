// Segment.h

#ifndef __segment_h
#define __segment_h

#include "define.h"
#include "Types.h"
#include "cfile.h"

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
	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false) { return fp.ReadVector (v); }
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
	bool SetTexture (short nBaseTex, short nOvlTex);
	CWall* GetWall (void);
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
	inline short Child (short nSide) { return m_sides [nSide].m_info.nChild; }
	inline short SetChild (short nSide, short nSegment) {
		m_sides [nSide].m_info.nChild = nSegment;
		if (nSegment == -1)
			m_info.childFlags &= ~(1 << nSide);
		else
			m_info.childFlags |= (1 << nSide);
		return nSegment;
		}

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