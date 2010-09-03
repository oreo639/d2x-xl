#ifndef __types_h
#define __types_h

#include "define.h"
#include "cfile.h"

# pragma pack(push, packing)
# pragma pack(1)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CGameItem {
public:
	virtual CGameItem* Next (void) { return this + 1; }
	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false) = 0;
	virtual void Write (CFileManager& fp, int version = 0, bool bFlag = false) = 0;
	virtual void Clear (void) = 0;
	void Reset (int count) { 
		CGameItem* i = this;
		while (count--) {
			Clear ();
			i = i->Next ();
			}
		}
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

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

	void Read (CFileManager& fp) {
		m_nSegment = fp.ReadInt16 ();
		m_nSide = fp.ReadInt16 ();
		}

	void Write (CFileManager& fp) {
		fp.Write (m_nSegment);
		fp.Write (m_nSide);
		}

	void Clear (void) {
		m_nSegment = m_nSide = -1;
		}
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct rgbColor {
	double	r, g, b;
} rgbColor;

// -----------------------------------------------------------------------------

typedef struct tColor {
	byte		index;
	rgbColor	color;
} tColor;

// -----------------------------------------------------------------------------

class CColor : public CGameItem {
public:
	tColor	m_info;

	virtual CGameItem* Next (void) { return this + 1; }
	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Write (CFileManager& fp, int version = 0, bool bFlag = false);

	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct {
  ushort index;
} BITMAP_INDEX;

typedef struct {
  byte	flags;		    //values defined above
  byte	pad[3];	    //keep alignment
  int		lighting;	    //how much light this casts
  int		damage;	    //how much damage being against this does (for lava)
  short	eclip_num;	    //the eclip that changes this, or -1
  short	destroyed;	    //bitmap to show when destroyed, or -1
  short	slide_u, slide_v;   //slide rates of texture, stored in 8:8 int
} TMAP_INFO;

typedef struct VCLIP {
  int		play_time;  //total time (in seconds) of clip
  int	num_frames;
  int		frame_time; //time (in seconds) of each frame
  int	flags;
  short	sound_num;
  ushort	frames[VCLIP_MAX_FRAMES];
  int		light_value;
} VCLIP;

typedef struct ECLIP {
  VCLIP   vc;			   //imbedded vclip
  int		 time_left;		   //for sequencing
  int	 frame_count;		   //for sequencing
  short	 changing_wall_texture;	   //Which element of Textures array to replace.
  short	 changing_object_texture;  //Which element of ObjBitmapPtrs array to replace.
  int	 flags;			   //see above
  int	 critClip;		   //use this clip instead of above one when mine critical
  int	 dest_bm_num;		//use this bitmap when monitor destroyed
  int	 dest_vclip;		//what vclip to play when exploding
  int	 dest_eclip;		//what eclip to play when exploding
  int	 destSize;		//3d size of explosion
  int	 sound_num;		//what sound this makes
  int	 nSegment,nSide;	//what segP & side, for one-shot clips
} ECLIP;

typedef struct WCLIP {
  int		 play_time;
  short	 num_frames;
  short	 frames[MAX_CLIP_FRAMES_D2];
  short	 open_sound;
  short	 close_sound;
  short	 flags;
  char	 filename[13];
  char	 pad;
} WCLIP;

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

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct level_header {
  char name[13];
  int size;
};

struct sub {
  INT64 offset;
  char name[13];
  int size;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// New stuff, 10/14/95: For shooting out lights and monitors.
// Light cast upon vert_light vertices in nSegment:nSide by some light

typedef struct tLightDeltaValue {
	byte vertLight [4];
} tLightDeltaValue;

class CLightDeltaValue : public CSideKey, public CGameItem {
public:
	tLightDeltaValue m_info;

	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Write (CFileManager& fp, int version = 0, bool bFlag = false);
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

	virtual void Read (CFileManager& fp, int version, bool bD2X);
	virtual void Write (CFileManager& fp, int version, bool bD2X);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
	virtual CGameItem* Next (void) { return this + 1; }
};

//extern CLightDeltaIndex    Dl_indices[MAX_LIGHT_DELTA_INDICES];
//extern CLightDeltaValue CLightDeltaValues[MAX_LIGHT_DELTA_VALUES];
//extern int	     Num_static_lights;

typedef struct tVariableLight {
	uint mask;    // bits with 1 = on, 0 = off
	int timer;		 // always set to 0
	int delay;      // time for each bit in mask (int seconds)
} tVariableLight;

class CVariableLight : public CSideKey {
public:
	tVariableLight m_info;

	void Read (CFileManager& fp) {
		CSideKey::Read (fp);
		m_info.mask = fp.ReadInt32 ();
		m_info.timer = fp.ReadInt32 ();
		m_info.delay = fp.ReadInt32 ();
		}

	void Write (CFileManager& fp) {
		CSideKey::Write (fp);
		fp.Write (m_info.mask);
		fp.Write (m_info.timer);
		fp.Write (m_info.delay);
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

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CVertex;
class CSegment;
class CSide;
class CWall;
class CTrigger;
class CObject;
class CSegmentManager;
class CWallManager;
class CTriggerManager;
class CMine;

extern CMine* theMine;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __types_h

