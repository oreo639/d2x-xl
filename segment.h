// Segment.h

#ifndef __segment_h
#define __segment_h

#include "define.h"

typedef struct tUVL {
public:
	INT16 u, v, l; 
} tUVL;

class CUVL : public tUVL {
public:
	inline INT32 Read (FILE* fp) {
		u = read_INT16 (fp);
		v = read_INT16 (fp);
		l = read_INT16 (fp);
		return 1;
		}
	inline void Write (FILE* fp) {
		write_INT16 (u, fp);
		write_INT16 (v, fp);
		write_INT16 (l, fp);
		}

	inline void Clear (void) { u = v = l = 0; }
	inline void Set (INT16 _u, INT16 _v, INT16 _l) { u = _u, v = _v, l = _l; }
};

typedef struct rgbColor {
	double	r, g, b;
} rgbColor;

typedef struct tColor {
	UINT8		index;
	rgbColor	color;
} tColor;

class CColor : public CGameItem {
public:
	tColor	m_info;

	virtual CGameItem* Next (void) { return this + 1; }
	virtual INT32 Read (FILE* fp, INT32 version = 0, bool bFlag = false);
	virtual void Write (FILE* fp, INT32 version = 0, bool bFlag = false);

	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};

typedef struct tSide {
	INT16		nChild;
	UINT16	nWall;		// (was INT16) Index into Walls array, which wall (probably door) is on this side 
	INT16		nBaseTex;	// Index into array of textures specified in bitmaps.bin 
	INT16		nOvlTex;		// Index, as above, texture which gets overlaid on nBaseTex 
	CUVL		uvls [4];   // CUVL coordinates at each point 
} tSide;

class CSide {
public:
	tSide m_info;

	INT32 Read (FILE* fp, bool bTextured);
	void Write (FILE* fp);
	void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	void Setup (void);
	void LoadTextures (void);
	bool SetTexture (INT16 nBaseTex, INT16 nOvlTex);
	CWall* Wall (void);
};

typedef struct tSegment {
	INT16		verts [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
	UINT8		function;			// special property of a segment (such as damaging, trigger, etc.) 
	UINT8		props;
	INT8		nMatCen;				// which center segment is associated with, high bit set 
	INT8		value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
	UINT8		s2_flags;			// New for Descent 2
	INT16		damage [2];
	FIX		staticLight;		// average static light in segment 
	UINT8		childFlags;			// bit0 to 5: children, bit6: unused, bit7: special 
	UINT8		wallFlags;			// bit0 to 5: door/walls, bit6: deleted, bit7: marked segment 
	INT16		nIndex;				// used for cut & paste to help link children 
	INT16		mapBitmask;		// which lines are drawn when displaying wireframe 
	INT8		owner;
	INT8		group;
} tSegment;

class CSegment : public CGameItem {
public:
	tSegment	m_info;
	CSide		m_sides [MAX_SIDES_PER_SEGMENT];		// 6 sides 

public:
	void Upgrade (void);
	INT32 Read (FILE* fp, int nLevelType, int nLevelVersion);
	void ReadExtras (FILE* fp, int nLevelType, int nLevelVersion, bool bExtras);
	void Write (FILE* fp, int nLevelType, int nLevelVersion);
	void WriteExtras (FILE* fp, int nLevelType, bool bExtras);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		for (int i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
			m_sides [i].Clear ();
		}
	void Setup (void);
	void SetUV (INT16 nSide, INT16 x, INT16 y);

	virtual CGameItem* Next (void) { return this + 1; }
	virtual INT32 Read (FILE* fp, INT32 version = 0, bool bFlag = false) { return 1; };
	virtual void Write (FILE* fp, INT32 version = 0, bool bFlag = false) {};
	inline INT16 Child (INT16 nSide) { return m_sides [nSide].m_info.nChild; }
	inline void SetChild (INT16 nSide, INT16 nSegment) {
		m_sides [nSide].m_info.nChild = nSegment;
		if (nSegment == -1)
			m_info.childFlags &= ~(1 << nSide);
		else
			m_info.childFlags |= (1 << nSide);
		}

private:
	UINT8 ReadWalls (FILE* fp, int nLevelVersion);
	UINT8 WriteWalls (FILE* fp, int nLevelVersion);

};

#endif //__segment_h