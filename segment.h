// Segment.h

#ifndef __segment_h
#define __segment_h

class CUVL {
public:
	INT16 u, v, l; 

	INT32 Read (FILE* fp);
	void Write (FILE* fp);
} 

class CSide {
public:
	UINT16	nWall;		// (was INT16) Index into Walls array, which wall (probably door) is on this side 
	INT16		nBaseTex;	// Index into array of textures specified in bitmaps.bin 
	INT16		nOvlTex;		// Index, as above, texture which gets overlaid on nBaseTex 
	CUVL		uvls [4];   // uvl coordinates at each point 
	INT32 Read (FILE* fp);
	void Write (FILE* fp);
};

typedef struct rgbColor {
	double	r, g, b;
} rgbColor;

class CColor {
	public:
		UINT8		index;
		rgbColor	color;
};

class CSegment {
public:

	CSide		sides [MAX_SIDES_PER_SEGMENT];		// 6 sides 
	INT16		children [MAX_SIDES_PER_SEGMENT];	// indices of 6 children segments, front, left, top, right, bottom, back 
	INT16		verts [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
	UINT8		function;			// special property of a segment (such as damaging, trigger, etc.) 
	UINT8		props;
	INT8		nMatCen;				// which center segment is associated with, high bit set 
	INT8		value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
	UINT8		s2_flags;			// New for Descent 2
	INT16		damage [2];
	FIX		static_light;		// average static light in segment 
	UINT8		child_bitmask;		// bit0 to 5: children, bit6: unused, bit7: special 
	UINT8		wall_bitmask;		// bit0 to 5: door/walls, bit6: deleted, bit7: marked segment 
	INT16		nIndex;				// used for cut & paste to help link children 
	INT16		map_bitmask;		// which lines are drawn when displaying wireframe 
	INT8		owner;
	INT8		group;

public:
	void Upgrade (void);
	INT32 Read (FILE* fp, bool bD2X);
	void Write (FILE* fp, bool bD2X);
	
};

#endif //__segment_h