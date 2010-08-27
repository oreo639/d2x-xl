// Segment.h

#ifndef __segment_h
#define __segment_h

#include "define.h"

typedef struct tUVL {
public:
	short u, v, l; 
} tUVL;

class CUVL : public tUVL {
public:
	inline int Read (FILE* fp) {
		u = ReadInt16 (fp);
		v = ReadInt16 (fp);
		l = ReadInt16 (fp);
		return 1;
		}
	inline void Write (FILE* fp) {
		WriteInt16 (u, fp);
		WriteInt16 (v, fp);
		WriteInt16 (l, fp);
		}

	inline void Clear (void) { u = v = l = 0; }
	inline void Set (short _u, short _v, short _l) { u = _u, v = _v, l = _l; }
};

typedef struct rgbColor {
	double	r, g, b;
} rgbColor;

typedef struct tColor {
	byte		index;
	rgbColor	color;
} tColor;

class CColor : public CGameItem {
public:
	tColor	m_info;

	virtual CGameItem* Next (void) { return this + 1; }
	virtual int Read (FILE* fp, int version = 0, bool bFlag = false);
	virtual void Write (FILE* fp, int version = 0, bool bFlag = false);

	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};

typedef struct tSide {
	short		nChild;
	ushort	nWall;		// (was short) Index into Walls array, which wall (probably door) is on this side 
	short		nBaseTex;	// Index into array of textures specified in bitmaps.bin 
	short		nOvlTex;		// Index, as above, texture which gets overlaid on nBaseTex 
	CUVL		uvls [4];   // CUVL coordinates at each point 
} tSide;

class CSide {
public:
	tSide m_info;

	int Read (FILE* fp, bool bTextured);
	void Write (FILE* fp);
	void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	void Setup (void);
	void LoadTextures (void);
	bool SetTexture (short nBaseTex, short nOvlTex);
	CWall* Wall (void);
};

typedef struct tSegment {
	short		verts [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
	byte		function;			// special property of a segment (such as damaging, trigger, etc.) 
	byte		props;
	char		nMatCen;				// which center segment is associated with, high bit set 
	char		value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
	byte		s2Flags;			// New for Descent 2
	short		damage [2];
	fix		staticLight;		// average static light in segment 
	byte		childFlags;			// bit0 to 5: children, bit6: unused, bit7: special 
	byte		wallFlags;			// bit0 to 5: door/walls, bit6: deleted, bit7: marked segment 
	short		nIndex;				// used for cut & paste to help link children 
	short		mapBitmask;		// which lines are drawn when displaying wireframe 
	char		owner;
	char		group;
} tSegment;

class CSegment : public CGameItem {
public:
	tSegment	m_info;
	CSide		m_sides [MAX_SIDES_PER_SEGMENT];		// 6 sides 

public:
	void Upgrade (void);
	int Read (FILE* fp, int nLevelType, int nLevelVersion);
	void ReadExtras (FILE* fp, int nLevelType, int nLevelVersion, bool bExtras);
	void Write (FILE* fp, int nLevelType, int nLevelVersion);
	void WriteExtras (FILE* fp, int nLevelType, bool bExtras);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		for (int i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
			m_sides [i].Clear ();
		}
	void Setup (void);
	void SetUV (short nSide, short x, short y);

	virtual CGameItem* Next (void) { return this + 1; }
	virtual int Read (FILE* fp, int version = 0, bool bFlag = false) { return 1; };
	virtual void Write (FILE* fp, int version = 0, bool bFlag = false) {};
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
	byte ReadWalls (FILE* fp, int nLevelVersion);
	byte WriteWalls (FILE* fp, int nLevelVersion);

};

#endif //__segment_h