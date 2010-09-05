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
	short m_nIndex;
	bool m_bUsed;

	CGameItem () : m_nIndex(-1), m_bUsed(false) {}

	virtual void Clear (void) = 0;

};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template < class _T >
class CGameItemIterator {
	protected:
		int	m_index;
		_T*	m_buffer;
		int	m_bufSize;

	public:
		CGameItemIterator (_T* buffer, int bufSize) : m_index(0), m_buffer(buffer), m_bufSize(bufSize) {}

		// post-increment
		inline _T& operator++() { 
			_T& value;
			do {
				value = m_buffer [m_index];
				m_index++;
				} while (!end () || dynamic_cast<CGameItem&> (value).m_bUsed);
			return *value; 
			}

		// pre-decrement
		inline _T& operator--() { 
			do {
				m_index--;
				} while (!end () || dynamic_cast<CGameItem&> (m_buffer [m_index]).m_bUsed);
			return m_buffer [m_index]; 
			}

		inline CGameItemIterator& operator= (int i) { 
			m_index = i;
			return *this;
			}

		inline operator bool() const { return !end (); }

		inline const bool operator== (int i) { return m_index == i; }

		inline const bool operator!= (int i) { return m_index != i; }

		inline const bool start (void) { return m_index == 0; }

		inline const bool end (void) { return m_index == m_bufSize; }
	};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CSideKey {
public:
	short	m_nSegment;
	short	m_nSide;

	CSideKey (short nSegment = -1, short nSide = -1) : m_nSegment(nSegment), m_nSide(nSide) {}

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


	void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	void Write (CFileManager& fp, int version = 0, bool bFlag = false);

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

