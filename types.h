#ifndef DMB_TYPES_H
#define DMB_TYPES_H

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

// Copyright (C) 1997 Bryan Aamot
/* define a signed types */
typedef signed char INT8;
typedef signed short INT16;
//typedef signed long INT32;
//typedef double INT64;

/* define unsigned types */
typedef unsigned char UINT8;
typedef unsigned short UINT16;
//typedef unsigned long UINT32;
//typedef double UINT64;

/* floating point types */
//typedef double FLOAT;
typedef double DOUBLE;

/* standard types */
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long LONGWORD;

/* special types */
typedef long FIX;	/*16 bits INT32, 16 bits frac */
typedef INT16 FIXANG;	/*angles */

#include "io.h"
#include "VectorTypes.h"

#if 0
class CStatusMask {
public:
	UINT8	m_status;

	inline UINT8 Status () { return m_status; }
	inline void Set (UINT8 mask = 0) { m_status = mask; }
	inline void Mark (UINT8 flags = MARKED_MASK) { m_status |= flags; }
	inline void Unmark (UINT8 flags = MARKED_MASK) { m_status &= ~flags; }
	inline bool Marked (UINT8 flags = MARKED_MASK) { return (m_status & flags) != 0; }

	inline UINT8& operator&= (UINT8 mask) { 
		m_status &= mask;
		return m_status;
		}

	inline UINT8& operator|= (UINT8 mask) { 
		m_status |= mask;
		return m_status;
		}

	inline UINT8& operator^= (UINT8 mask) { 
		m_status ^= mask;
		return m_status;
		}

	inline UINT8& operator= (UINT8 mask) { 
		m_status = mask;
		return m_status;
		}

	inline const UINT8 operator& (UINT8 other) const { return m_status & other; }
	inline const UINT8 operator| (UINT8 other) const { return m_status | other; }
	inline const UINT8 operator^ (UINT8 other) const { return m_status ^ other; }

	inline CStatusMask& operator&= (CStatusMask& other) { 
		m_status &= other.m_status;
		return *this;
		}

	inline CStatusMask& operator|= (CStatusMask& other) { 
		m_status |= other.m_status;
		return *this;
		}

	inline CStatusMask& operator^= (CStatusMask& other) { 
		m_status ^= other.m_status;
		return *this;
		}

	inline CStatusMask& operator= (CStatusMask& other) { 
		m_status = other.m_status;
		return *this;
		}

	inline const UINT8 operator& (CStatusMask& other) const { return m_status & other.m_status; }
	inline const UINT8 operator| (CStatusMask& other) const { return m_status | other.m_status; }
	inline const UINT8 operator^ (CStatusMask& other) const { return m_status ^ other.m_status; }

	inline INT32 Read (FILE* fp) { 
		m_status = UINT8 (read_INT8 (fp));
		return 1;
		}

	inline void Write (FILE* fp) {
		write_INT8 (INT8 (m_status), fp);
	}
};
#endif

class CGameItem {
public:
	virtual CGameItem* Next (void) { return this + 1; }
	virtual INT32 Read (FILE* fp, INT32 version = 0, bool bFlag = false) = 0;
	virtual void Write (FILE* fp, INT32 version = 0, bool bFlag = false) = 0;
	virtual void Clear (void) = 0;
	void Reset (int count) { 
		CGameItem* i = this;
		while (count--) {
			Clear ();
			i = i->Next ();
			}
		}
};

class CVertex : public CDoubleVector, public CGameItem {
public:
	UINT8 m_status;
	CVertex () : m_status(0) {}
	CVertex (DOUBLE x, DOUBLE y, DOUBLE z) : CDoubleVector (x, y, z) { m_status = 0; }
	CVertex (tDoubleVector& _v) : CDoubleVector (_v) { m_status = 0; }
	//CVertex (CDoubleVector& _v) : CDoubleVector (_v) { m_status = 0; }

	virtual CGameItem* Next (void) { return this + 1; }
	virtual INT32 Read (FILE* fp, INT32 version = 0, bool bFlag = false) { return v.Read (fp); }
	virtual void Write (FILE* fp, INT32 version = 0, bool bFlag = false) { v.Write (fp); }
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

typedef struct {
  UINT16 index;
} BITMAP_INDEX;

typedef struct {
  UINT8	flags;		    //values defined above
  UINT8	pad[3];	    //keep alignment
  FIX		lighting;	    //how much light this casts
  FIX		damage;	    //how much damage being against this does (for lava)
  INT16	eclip_num;	    //the eclip that changes this, or -1
  INT16	destroyed;	    //bitmap to show when destroyed, or -1
  INT16	slide_u, slide_v;   //slide rates of texture, stored in 8:8 FIX
} TMAP_INFO;

typedef struct VCLIP {
  FIX		play_time;  //total time (in seconds) of clip
  INT32	num_frames;
  FIX		frame_time; //time (in seconds) of each frame
  INT32	flags;
  INT16	sound_num;
  UINT16	frames[VCLIP_MAX_FRAMES];
  FIX		light_value;
} VCLIP;

typedef struct ECLIP {
  VCLIP   vc;			   //imbedded vclip
  FIX		 time_left;		   //for sequencing
  INT32	 frame_count;		   //for sequencing
  INT16	 changing_wall_texture;	   //Which element of Textures array to replace.
  INT16	 changing_object_texture;  //Which element of ObjBitmapPtrs array to replace.
  INT32	 flags;			   //see above
  INT32	 critClip;		   //use this clip instead of above one when mine critical
  INT32	 dest_bm_num;		//use this bitmap when monitor destroyed
  INT32	 dest_vclip;		//what vclip to play when exploding
  INT32	 dest_eclip;		//what eclip to play when exploding
  FIX	 destSize;		//3d size of explosion
  INT32	 sound_num;		//what sound this makes
  INT32	 nSegment,nSide;	//what segP & side, for one-shot clips
} ECLIP;

typedef struct WCLIP {
  FIX		 play_time;
  INT16	 num_frames;
  INT16	 frames[MAX_CLIP_FRAMES2];
  INT16	 open_sound;
  INT16	 close_sound;
  INT16	 flags;
  char	 filename[13];
  char	 pad;
} WCLIP;

typedef struct {
  INT32    nBaseTex;
  long   light;
} TEXTURE_LIGHT;

typedef struct {
  INT16 x,y,z;
} APOINT;

typedef struct {
  INT16 nSegment;
  INT16 nSide;
  INT16 nLine;
  INT16 nPoint;
  INT16 nObject;
} SEGMENT;

class CSelection {
public:
	CSelection() :
		nSegment(0),
		nSide(DEFAULT_SIDE),
		nLine(DEFAULT_LINE),
		nPoint(DEFAULT_POINT),
		nObject(DEFAULT_OBJECT)
	{}

	INT16 nSegment;
	INT16 nSide;
	INT16 nLine;
	INT16 nPoint;
	INT16 nObject;
};

struct level_header {
  char name[13];
  INT32 size;
};

struct sub {
  INT64 offset;
  char name[13];
  INT32 size;
};

class CGameFileInfo {
public:
	UINT16  signature;
	UINT16  version;
	INT32   size;

	INT32 Read (FILE* fp) {
		signature = read_INT16 (fp);
		version = read_INT16 (fp);
		size = read_INT32 (fp);
		return 1;
		}

	void Write (FILE* fp) {
		write_INT16 (signature, fp);
		write_INT16 (version, fp);
		write_INT32 (size, fp);
		}
};

class CPlayerItemInfo {
public:
	INT32	 offset;
	INT32  size;

	CPlayerItemInfo () { offset = -1, size = 0; }
	INT32 Read (FILE* fp) {
		offset = read_INT32 (fp);
		size  = read_INT32 (fp);
		return 1;
		}

	void Write (FILE* fp) {
		write_INT32 (offset, fp);
		write_INT32 (size, fp);
		}
};

class CGameItemInfo {
public:
	INT32	 offset;
	INT32	 count;
	INT32  size;

	CGameItemInfo () { Reset (); }

	void Reset (void) { offset = -1, count = size = 0; } 

	INT32 Read (FILE* fp) {
		offset = read_INT32 (fp);
		count = read_INT32 (fp);
		size  = read_INT32 (fp);
		return 1;
		}

	void Write (FILE* fp) {
		write_INT32 (offset, fp);
		write_INT32 (count, fp);
		write_INT32 (size, fp);
		}
};

class CGameInfo {
public:
	CGameFileInfo		fileinfo;
	char					mine_filename[15];
	INT32					level;
	CPlayerItemInfo	player;
	CGameItemInfo		objects;
	CGameItemInfo		walls;
	CGameItemInfo		doors;
	CGameItemInfo		triggers;
	CGameItemInfo		links;
	CGameItemInfo		control;
	CGameItemInfo		botgen;
	CGameItemInfo		lightDeltaIndices;
	CGameItemInfo		lightDeltaValues;
	CGameItemInfo		equipgen;
};


class CSideKey {
public:
	INT16	m_nSegment;
	INT16	m_nSide;

	CSideKey(INT16 nSegment = 0, INT16 nSide = 0) : m_nSegment(nSegment), m_nSide(nSide) {}
	inline bool operator == (CSideKey& other) { return (m_nSegment == other.m_nSegment) && (m_nSide == other.m_nSide); }
	inline bool operator != (CSideKey& other) { return (m_nSegment != other.m_nSegment) || (m_nSide != other.m_nSide); }
	inline bool operator < (CSideKey& other) { return (m_nSegment < other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide < other.m_nSide)); }
	inline bool operator <= (CSideKey& other) { return (m_nSegment < other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide <= other.m_nSide)); }
	inline bool operator > (CSideKey& other) { return (m_nSegment > other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide > other.m_nSide)); }
	inline bool operator >= (CSideKey& other) { return (m_nSegment > other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide >= other.m_nSide)); }

	INT32 Read (FILE* fp) {
		m_nSegment = read_INT16 (fp);
		m_nSide = read_INT16 (fp);
		return 1;
		}
	void Write (FILE* fp) {
		write_INT16 (m_nSegment, fp);
		write_INT16 (m_nSide, fp);
		}

	void Clear (void) {
		m_nSegment = m_nSide = -1;
		}
};

typedef struct tWall {
	FIX		hps;            // "Hit points" of the wall. 
	INT32		linkedWall;		 // number of linked wall
	UINT8		type;           // What kind of special wall.
	UINT16	flags;          // Flags for the wall.    
	UINT8		state;          // Opening, closing, etc.
	UINT8		nTrigger;       // Which trigger is associated with the wall.
	INT8		nClip;          // Which  animation associated with the wall. 
	UINT8		keys;           // which keys are required
	// the following two Descent2 bytes replace the "INT16 pad" of Descent1
	INT8		controllingTrigger; // which trigger causes something to happen here.
	// Not like "trigger" above, which is the trigger on this wall.
	//	Note: This gets stuffed at load time in gamemine.c.  
	// Don't try to use it in the editor.  You will be sorry!
	INT8		cloakValue;	// if this wall is cloaked, the fade value
} tWall;

class CWall : public CSideKey, public CGameItem {
public:
	tWall		m_info;
 

	INT32 Read (FILE* fp, INT32 version = 0, bool bFlag = false);
	void Write (FILE* fp, INT32 version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

typedef struct tActiveDoor {
	INT32		n_parts;	   // for linked walls
  INT16		nFrontWall[2]; // front wall numbers for this door
  INT16		nBackWall[2];  // back wall numbers for this door
  FIX			time;		   // how long been opening, closing, waiting
} tActiveDoor;

class CActiveDoor : public CGameItem {
public:
	tActiveDoor	m_info;

	virtual INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

typedef struct tCloakingWall {
	INT16		nFrontWall;		// front wall numbers for this door
	INT16		nBackWall; 		// back wall numbers for this door
	FIX		front_ls[4]; 	// front wall saved light values
	FIX		back_ls[4];		// back wall saved light values
	FIX		time;				// how long been cloaking or decloaking
} tCloakingWall;

class CCloakingWall : public CGameItem {    // NEW for Descent 2
public:
	tCloakingWall m_info;

	virtual INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

/*
typedef struct {
  FIX	play_time;
  INT16	num_frames;
  INT16	frames[MAX_CLIP_FRAMES];
  INT16	open_sound;
  INT16	close_sound;
  INT16	flags;
  char	filename[13];
  char	pad;
} wclip;
*/

//extern char	Wall_names[7][10]; // New for Descent 2

typedef struct tTriggerTargets {
} tTriggerTargets;

class CTriggerTargets {
public:
	INT16		m_count;
	CSideKey	m_targets [MAX_TRIGGER_TARGETS];

	CTriggerTargets () { m_count = 0; }

	inline CSideKey& operator[](UINT32 i) { return m_targets [i]; }

	inline INT16 Add (CSideKey key) {
		if (m_count < sizeof (m_targets) / sizeof (m_targets [0]))
			m_targets [m_count] = key;
		return m_count++;
		}
	inline INT16 Add (INT16 nSegment, INT16 nSide) { return Add (CSideKey (nSegment, nSide)); }

	inline INT16 Delete (int i = -1) {
		if (i < 0)
			i = m_count - 1;
		if ((m_count > 0) && (i < --m_count)) {
			int l = m_count - i;
			if (l)
				memcpy (m_targets + i, m_targets + i + 1, l * sizeof (m_targets [0]));
			m_targets [m_count] = CSideKey (0,0);
			}
		return m_count;
		}	

	inline INT16 Pop (void) { return Delete (m_count - 1); }

	inline int Find (CSideKey key) { 
		for (int i = 0; i < m_count; i++)
			if (m_targets [i] == key)
				return i;
		return -1;
		}
	inline int Find (INT16 nSegment, INT16 nSide) { return Find (CSideKey (nSegment, nSide)); }
	inline INT16& Segment (UINT32 i) { return m_targets [i].m_nSegment; }
	inline INT16& Side (UINT32 i) { return m_targets [i].m_nSide; }
	void Clear (void) { 
		m_count = 0;
		for (int i = 0; i < MAX_TRIGGER_TARGETS; i++)
			m_targets [i].Clear ();
		}
	inline INT32 ReadTargets (FILE* fp) {
		int i;
		for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
			m_targets [i].m_nSegment = read_INT16(fp);
		for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
			m_targets [i].m_nSide = read_INT16(fp);
		return 1;
		}
	inline void WriteTargets (FILE* fp) {
		int i;
		for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
			write_INT16 (m_targets [i].m_nSegment, fp);
		for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
			write_INT16 (m_targets [i].m_nSide, fp);
		}

};

typedef struct tTrigger {
	UINT8		type;
	UINT16	flags;
	INT16		nObject;
	FIX		value;
	FIX		time;
	UINT16	nIndex;
} tTrigger;

class CTrigger : public CTriggerTargets, public CGameItem {
public:
	struct tTrigger m_info;
	//inline CSideKey& operator[](UINT32 i) { return targets [i]; }

	virtual CGameItem* Next (void) { return this + 1; }
	virtual INT32 Read (FILE *fp, INT32 version, bool bObjTrigger);
	virtual void Write (FILE *fp, INT32 version, bool bObjTrigger);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CTriggerTargets::Clear ();
		}
	void CTrigger::Setup (INT16 type, INT16 flags);
	inline const bool operator< (const CTrigger& other) {
		return (m_info.nObject < other.m_info.nObject) || ((m_info.nObject == other.m_info.nObject) && (m_info.type < other.m_info.type)); 
		}
	inline const bool operator> (const CTrigger& other) {
		return (m_info.nObject > other.m_info.nObject) || ((m_info.nObject == other.m_info.nObject) && (m_info.type > other.m_info.type)); 
		}
};

// New stuff, 10/14/95: For shooting out lights and monitors.
// Light cast upon vert_light vertices in nSegment:nSide by some light

typedef struct tLightDeltaValue {
	UINT8 vertLight [4];
} tLightDeltaValue;

class CLightDeltaValue : public CSideKey, public CGameItem {
public:
	tLightDeltaValue m_info;

	virtual INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
	virtual CGameItem* Next (void) { return this + 1; }
};

// Light at nSegment:nSide casts light on count sides beginning at index (in array CLightDeltaValues)
typedef struct tLightDeltaIndex {
	UINT16 count;
	UINT16 index;
} tLightDeltaIndex;

class CLightDeltaIndex : public CSideKey, public CGameItem {
public:
	tLightDeltaIndex m_info;

	virtual INT32 Read (FILE *fp, INT32 version, bool bD2X);
	virtual void Write (FILE *fp, INT32 version, bool bD2X);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
	virtual CGameItem* Next (void) { return this + 1; }
};

//extern CLightDeltaIndex    Dl_indices[MAX_LIGHT_DELTA_INDICES];
//extern CLightDeltaValue CLightDeltaValues[MAX_LIGHT_DELTA_VALUES];
//extern INT32	     Num_static_lights;


class CReactorTrigger : public CTriggerTargets, public CGameItem {
public:
	INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Clear (void) { CTriggerTargets::Clear (); }
	virtual CGameItem* Next (void) { return this + 1; }
};

typedef struct tRobotMaker {
	INT32  objFlags [2]; /* Up to 32 different Descent 1 robots */
	//  INT32  robot_flags2;// Additional 32 robots for Descent 2
	FIX    hitPoints;  /* How hard it is to destroy this particular matcen */
	FIX    interval;    /* Interval between materializations */
	INT16  nSegment;      /* Segment this is attached to. */
	INT16  nFuelCen; /* Index in fuelcen array. */
} tRobotMaker;

class CRobotMaker : public CGameItem {
public:
	tRobotMaker	m_info;

	INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};


/* pig file types */
typedef struct {
  INT32 number_of_textures;
  INT32 number_of_sounds;
} PIG_HEADER;

typedef struct {
  char name[8];
  UINT8 dflags; /* this is only important for large bitmaps like the cockpit */
  UINT8 xsize;
  UINT8 ysize;
  UINT8 flags;
  UINT8 avg_color;
  UINT32 offset;
} PIG_TEXTURE;

typedef struct {
  INT32 signature;
  INT32 version;
  INT32 textureCount;
} D2_PIG_HEADER;

typedef struct {
  char name[8];
  UINT8 dflags;  // bits 0-5 anim frame num, bit 6 abm flag
  UINT8 xsize;   // low 8 bits here, 4 more bits in pad
  UINT8 ysize;   // low 8 bits here, 4 more bits in pad
  UINT8 wh_extra;     // bits 0-3 xsize, bits 4-7 ysize
  UINT8 flags;   // see BM_FLAG_XXX in define.h
  UINT8 avg_color;   // average color
  UINT32 offset;
} D2_PIG_TEXTURE;

typedef struct {
  UINT8 unknown[20];
} PIG_SOUND;

typedef struct {
  char name[8];
  INT16 number;
} TEXTURE;

typedef struct tFlickeringLight {
	UINT32 mask;    // bits with 1 = on, 0 = off
	FIX timer;		 // always set to 0
	FIX delay;      // time for each bit in mask (INT32 seconds)
} tFlickeringLight;

class CFlickeringLight : public CSideKey {
public:
	tFlickeringLight m_info;

	INT32 Read (FILE* fp) {
		CSideKey::Read (fp);
		m_info.mask = read_INT32 (fp);
		m_info.timer = read_FIX (fp);
		m_info.delay = read_FIX (fp);
		return 1;
		}
	void Write (FILE* fp) {
		CSideKey::Write (fp);
		write_INT32 (m_info.mask, fp);
		write_FIX (m_info.timer, fp);
		write_FIX (m_info.delay, fp);
		}
	void Clear (void) {
		memset (&m_info, 0, sizeof (m_info));
		CSideKey::Clear ();
		}
};

typedef struct {
	INT16	ticks;
	INT16	impulse;
} LIGHT_TIMER;

typedef struct {
	bool	bIsOn;
	bool	bWasOn;
} LIGHT_STATUS;

#define MAX_LEVELS	1000

typedef struct {
	char	missionName [80];
	char  missionInfo [8][80];
	INT32	authorFlags [2];
	INT32	missionType;
	INT32	missionFlags [6];
	INT32	customFlags [3];
	char	levelList [MAX_LEVELS][17];	//18 == ########.###,####'\0' == levlname.ext,secr
	char	comment [4000];
	INT32   numLevels;
	INT32	numSecrets;
} MISSION_DATA;

typedef struct tVertMatch {
		INT16		b;
		INT16		i;
		double	d;
	} tVertMatch; 

#endif // DMB_TYPES_H

