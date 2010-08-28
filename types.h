#ifndef DMB_TYPES_H
#define DMB_TYPES_H

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

/* define unsigned types */
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef int fix;			// 16 bits int, 16 bits frac 
typedef short fixang;	// angles 

#include "io.h"
#include "VectorTypes.h"

#if 0
class CStatusMask {
public:
	byte	m_status;

	inline byte Status () { return m_status; }
	inline void Set (byte mask = 0) { m_status = mask; }
	inline void Mark (byte flags = MARKED_MASK) { m_status |= flags; }
	inline void Unmark (byte flags = MARKED_MASK) { m_status &= ~flags; }
	inline bool Marked (byte flags = MARKED_MASK) { return (m_status & flags) != 0; }

	inline byte& operator&= (byte mask) { 
		m_status &= mask;
		return m_status;
		}

	inline byte& operator|= (byte mask) { 
		m_status |= mask;
		return m_status;
		}

	inline byte& operator^= (byte mask) { 
		m_status ^= mask;
		return m_status;
		}

	inline byte& operator= (byte mask) { 
		m_status = mask;
		return m_status;
		}

	inline const byte operator& (byte other) const { return m_status & other; }
	inline const byte operator| (byte other) const { return m_status | other; }
	inline const byte operator^ (byte other) const { return m_status ^ other; }

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

	inline const byte operator& (CStatusMask& other) const { return m_status & other.m_status; }
	inline const byte operator| (CStatusMask& other) const { return m_status | other.m_status; }
	inline const byte operator^ (CStatusMask& other) const { return m_status ^ other.m_status; }

	inline int Read (FILE* fp) { 
		m_status = byte (ReadInt8 (fp));
		return 1;
		}

	inline void Write (FILE* fp) {
		WriteInt8 (char (m_status), fp);
	}
};
#endif

class CGameItem {
public:
	virtual CGameItem* Next (void) { return this + 1; }
	virtual int Read (FILE* fp, int version = 0, bool bFlag = false) = 0;
	virtual void Write (FILE* fp, int version = 0, bool bFlag = false) = 0;
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
	byte m_status;
	CVertex () : m_status(0) {}
	CVertex (double x, double y, double z) : CDoubleVector (x, y, z) { m_status = 0; }
	CVertex (tDoubleVector& _v) : CDoubleVector (_v) { m_status = 0; }
	CVertex (CDoubleVector _v) : CDoubleVector (_v) { m_status = 0; }

	virtual CGameItem* Next (void) { return this + 1; }
	virtual int Read (FILE* fp, int version = 0, bool bFlag = false) { return v.Read (fp); }
	virtual void Write (FILE* fp, int version = 0, bool bFlag = false) { v.Write (fp); }
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
  ushort index;
} BITMAP_INDEX;

typedef struct {
  byte	flags;		    //values defined above
  byte	pad[3];	    //keep alignment
  fix		lighting;	    //how much light this casts
  fix		damage;	    //how much damage being against this does (for lava)
  short	eclip_num;	    //the eclip that changes this, or -1
  short	destroyed;	    //bitmap to show when destroyed, or -1
  short	slide_u, slide_v;   //slide rates of texture, stored in 8:8 fix
} TMAP_INFO;

typedef struct VCLIP {
  fix		play_time;  //total time (in seconds) of clip
  int	num_frames;
  fix		frame_time; //time (in seconds) of each frame
  int	flags;
  short	sound_num;
  ushort	frames[VCLIP_MAX_FRAMES];
  fix		light_value;
} VCLIP;

typedef struct ECLIP {
  VCLIP   vc;			   //imbedded vclip
  fix		 time_left;		   //for sequencing
  int	 frame_count;		   //for sequencing
  short	 changing_wall_texture;	   //Which element of Textures array to replace.
  short	 changing_object_texture;  //Which element of ObjBitmapPtrs array to replace.
  int	 flags;			   //see above
  int	 critClip;		   //use this clip instead of above one when mine critical
  int	 dest_bm_num;		//use this bitmap when monitor destroyed
  int	 dest_vclip;		//what vclip to play when exploding
  int	 dest_eclip;		//what eclip to play when exploding
  fix	 destSize;		//3d size of explosion
  int	 sound_num;		//what sound this makes
  int	 nSegment,nSide;	//what segP & side, for one-shot clips
} ECLIP;

typedef struct WCLIP {
  fix		 play_time;
  short	 num_frames;
  short	 frames[MAX_CLIP_FRAMES2];
  short	 open_sound;
  short	 close_sound;
  short	 flags;
  char	 filename[13];
  char	 pad;
} WCLIP;

typedef struct {
  int    nBaseTex;
  long   light;
} TEXTURE_LIGHT;

typedef struct {
  short x,y,z;
} APOINT;

typedef struct {
  short nSegment;
  short nSide;
  short nLine;
  short nPoint;
  short nObject;
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

	short nSegment;
	short nSide;
	short nLine;
	short nPoint;
	short nObject;
};

struct level_header {
  char name[13];
  int size;
};

struct sub {
  INT64 offset;
  char name[13];
  int size;
};

class CGameFileInfo {
public:
	ushort  signature;
	ushort  version;
	int   size;

	int Read (FILE* fp) {
		signature = ReadInt16 (fp);
		version = ReadInt16 (fp);
		size = ReadInt32 (fp);
		return 1;
		}

	void Write (FILE* fp) {
		WriteInt16 (signature, fp);
		WriteInt16 (version, fp);
		WriteInt32 (size, fp);
		}
};

class CPlayerItemInfo {
public:
	int	 offset;
	int  size;

	CPlayerItemInfo () { offset = -1, size = 0; }
	int Read (FILE* fp) {
		offset = ReadInt32 (fp);
		size  = ReadInt32 (fp);
		return 1;
		}

	void Write (FILE* fp) {
		WriteInt32 (offset, fp);
		WriteInt32 (size, fp);
		}
};

class CGameItemInfo {
public:
	int	 offset;
	int	 count;
	int  size;

	CGameItemInfo () { Reset (); }

	void Reset (void) { offset = -1, count = size = 0; } 

	int Read (FILE* fp) {
		offset = ReadInt32 (fp);
		count = ReadInt32 (fp);
		size  = ReadInt32 (fp);
		return 1;
		}

	void Write (FILE* fp) {
		WriteInt32 (offset, fp);
		WriteInt32 (count, fp);
		WriteInt32 (size, fp);
		}
};

class CGameInfo {
public:
	CGameFileInfo		fileinfo;
	char					mine_filename[15];
	int					level;
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
	short	m_nSegment;
	short	m_nSide;

	CSideKey(short nSegment = 0, short nSide = 0) : m_nSegment(nSegment), m_nSide(nSide) {}
	inline bool operator == (CSideKey& other) { return (m_nSegment == other.m_nSegment) && (m_nSide == other.m_nSide); }
	inline bool operator != (CSideKey& other) { return (m_nSegment != other.m_nSegment) || (m_nSide != other.m_nSide); }
	inline bool operator < (CSideKey& other) { return (m_nSegment < other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide < other.m_nSide)); }
	inline bool operator <= (CSideKey& other) { return (m_nSegment < other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide <= other.m_nSide)); }
	inline bool operator > (CSideKey& other) { return (m_nSegment > other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide > other.m_nSide)); }
	inline bool operator >= (CSideKey& other) { return (m_nSegment > other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide >= other.m_nSide)); }

	int Read (FILE* fp) {
		m_nSegment = ReadInt16 (fp);
		m_nSide = ReadInt16 (fp);
		return 1;
		}
	void Write (FILE* fp) {
		WriteInt16 (m_nSegment, fp);
		WriteInt16 (m_nSide, fp);
		}

	void Clear (void) {
		m_nSegment = m_nSide = -1;
		}
};

typedef struct tWall {
	fix		hps;            // "Hit points" of the wall. 
	int		linkedWall;		 // number of linked wall
	byte		type;           // What kind of special wall.
	ushort	flags;          // Flags for the wall.    
	byte		state;          // Opening, closing, etc.
	byte		nTrigger;       // Which trigger is associated with the wall.
	char		nClip;          // Which  animation associated with the wall. 
	byte		keys;           // which keys are required
	// the following two Descent2 bytes replace the "short pad" of Descent1
	char		controllingTrigger; // which trigger causes something to happen here.
	// Not like "trigger" above, which is the trigger on this wall.
	//	Note: This gets stuffed at load time in gamemine.c.  
	// Don't try to use it in the editor.  You will be sorry!
	char		cloakValue;	// if this wall is cloaked, the fade value
} tWall;

class CWall : public CSideKey, public CGameItem {
public:
	tWall		m_info;
 

	int Read (FILE* fp, int version = 0, bool bFlag = false);
	void Write (FILE* fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

typedef struct tActiveDoor {
	int		n_parts;	   // for linked walls
  short		nFrontWall[2]; // front wall numbers for this door
  short		nBackWall[2];  // back wall numbers for this door
  fix			time;		   // how long been opening, closing, waiting
} tActiveDoor;

class CActiveDoor : public CGameItem {
public:
	tActiveDoor	m_info;

	virtual int Read (FILE *fp, int version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

typedef struct tCloakingWall {
	short		nFrontWall;		// front wall numbers for this door
	short		nBackWall; 		// back wall numbers for this door
	fix		front_ls[4]; 	// front wall saved light values
	fix		back_ls[4];		// back wall saved light values
	fix		time;				// how long been cloaking or decloaking
} tCloakingWall;

class CCloakingWall : public CGameItem {    // NEW for Descent 2
public:
	tCloakingWall m_info;

	virtual int Read (FILE *fp, int version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

/*
typedef struct {
  fix	play_time;
  short	num_frames;
  short	frames[MAX_CLIP_FRAMES];
  short	open_sound;
  short	close_sound;
  short	flags;
  char	filename[13];
  char	pad;
} wclip;
*/

//extern char	Wall_names[7][10]; // New for Descent 2

typedef struct tTriggerTargets {
} tTriggerTargets;

class CTriggerTargets {
public:
	short		m_count;
	CSideKey	m_targets [MAX_TRIGGER_TARGETS];

	CTriggerTargets () { m_count = 0; }

	inline CSideKey& operator[](uint i) { return m_targets [i]; }

	inline short Add (CSideKey key) {
		if (m_count < sizeof (m_targets) / sizeof (m_targets [0]))
			m_targets [m_count] = key;
		return m_count++;
		}
	inline short Add (short nSegment, short nSide) { return Add (CSideKey (nSegment, nSide)); }

	inline short Delete (int i = -1) {
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

	inline short Pop (void) { return Delete (m_count - 1); }

	inline int Find (CSideKey key) { 
		for (int i = 0; i < m_count; i++)
			if (m_targets [i] == key)
				return i;
		return -1;
		}
	inline int Find (short nSegment, short nSide) { return Find (CSideKey (nSegment, nSide)); }
	inline short& Segment (uint i) { return m_targets [i].m_nSegment; }
	inline short& Side (uint i) { return m_targets [i].m_nSide; }
	void Clear (void) { 
		m_count = 0;
		for (int i = 0; i < MAX_TRIGGER_TARGETS; i++)
			m_targets [i].Clear ();
		}
	inline int ReadTargets (FILE* fp) {
		int i;
		for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
			m_targets [i].m_nSegment = ReadInt16(fp);
		for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
			m_targets [i].m_nSide = ReadInt16(fp);
		return 1;
		}
	inline void WriteTargets (FILE* fp) {
		int i;
		for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
			WriteInt16 (m_targets [i].m_nSegment, fp);
		for (i = 0; i < MAX_TRIGGER_TARGETS; i++)
			WriteInt16 (m_targets [i].m_nSide, fp);
		}

};

typedef struct tTrigger {
	byte		type;
	ushort	flags;
	short		nObject;
	fix		value;
	fix		time;
	ushort	nIndex;
} tTrigger;

class CTrigger : public CTriggerTargets, public CGameItem {
public:
	struct tTrigger m_info;
	//inline CSideKey& operator[](uint i) { return targets [i]; }

	virtual CGameItem* Next (void) { return this + 1; }
	virtual int Read (FILE *fp, int version, bool bObjTrigger);
	virtual void Write (FILE *fp, int version, bool bObjTrigger);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CTriggerTargets::Clear ();
		}
	void CTrigger::Setup (short type, short flags);
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
	byte vertLight [4];
} tLightDeltaValue;

class CLightDeltaValue : public CSideKey, public CGameItem {
public:
	tLightDeltaValue m_info;

	virtual int Read (FILE *fp, int version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
	virtual CGameItem* Next (void) { return this + 1; }
};

// Light at nSegment:nSide casts light on count sides beginning at index (in array CLightDeltaValues)
typedef struct tLightDeltaIndex {
	ushort count;
	ushort index;
} tLightDeltaIndex;

class CLightDeltaIndex : public CSideKey, public CGameItem {
public:
	tLightDeltaIndex m_info;

	virtual int Read (FILE *fp, int version, bool bD2X);
	virtual void Write (FILE *fp, int version, bool bD2X);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
	virtual CGameItem* Next (void) { return this + 1; }
};

//extern CLightDeltaIndex    Dl_indices[MAX_LIGHT_DELTA_INDICES];
//extern CLightDeltaValue CLightDeltaValues[MAX_LIGHT_DELTA_VALUES];
//extern int	     Num_static_lights;


class CReactorTrigger : public CTriggerTargets, public CGameItem {
public:
	int Read (FILE *fp, int version = 0, bool bFlag = false);
	void Write (FILE *fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { CTriggerTargets::Clear (); }
	virtual CGameItem* Next (void) { return this + 1; }
};

typedef struct tRobotMaker {
	int  objFlags [2]; /* Up to 32 different Descent 1 robots */
	//  int  robot_flags2;// Additional 32 robots for Descent 2
	fix    hitPoints;  /* How hard it is to destroy this particular matcen */
	fix    interval;    /* Interval between materializations */
	short  nSegment;      /* Segment this is attached to. */
	short  nFuelCen; /* Index in fuelcen array. */
} tRobotMaker;

class CRobotMaker : public CGameItem {
public:
	tRobotMaker	m_info;

	int Read (FILE *fp, int version = 0, bool bFlag = false);
	void Write (FILE *fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};


typedef struct tFlickeringLight {
	uint mask;    // bits with 1 = on, 0 = off
	fix timer;		 // always set to 0
	fix delay;      // time for each bit in mask (int seconds)
} tFlickeringLight;

class CFlickeringLight : public CSideKey {
public:
	tFlickeringLight m_info;

	int Read (FILE* fp) {
		CSideKey::Read (fp);
		m_info.mask = ReadInt32 (fp);
		m_info.timer = ReadFix (fp);
		m_info.delay = ReadFix (fp);
		return 1;
		}
	void Write (FILE* fp) {
		CSideKey::Write (fp);
		WriteInt32 (m_info.mask, fp);
		WriteFix (m_info.timer, fp);
		WriteFix (m_info.delay, fp);
		}
	void Clear (void) {
		memset (&m_info, 0, sizeof (m_info));
		CSideKey::Clear ();
		}
};

typedef struct {
	short	ticks;
	short	impulse;
} LIGHT_TIMER;

typedef struct {
	bool	bIsOn;
	bool	bWasOn;
} LIGHT_STATUS;

#define MAX_LEVELS	1000

typedef struct {
	char	missionName [80];
	char  missionInfo [8][80];
	int	authorFlags [2];
	int	missionType;
	int	missionFlags [6];
	int	customFlags [3];
	char	levelList [MAX_LEVELS][17];	//18 == ########.###,####'\0' == levlname.ext,secr
	char	comment [4000];
	int   numLevels;
	int	numSecrets;
} MISSION_DATA;

typedef struct tVertMatch {
		short		b;
		short		i;
		double	d;
	} tVertMatch; 

#endif // DMB_TYPES_H

