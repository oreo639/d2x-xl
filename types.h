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

class CAngleVector {
public:
	FIXANG p, b, h;
	//CAngleVector (FIXANG p, FIXANG b, FIXANG h) : p(p), b(b), h(h) {}
	//CAngleVector (CAngleVector& a) : p(a.p), b(a.b), h(a.h) {}

inline INT32 Read (FILE* fp) { 
	p = read_FIXANG (fp);
	b = read_FIXANG (fp);
	h = read_FIXANG (fp);
	return 1;
	}

void Write (FILE* fp) { 
	write_FIXANG (p, fp);
	write_FIXANG (b, fp);
	write_FIXANG (h, fp);
	}
};

class CFixVector {
public:
	FIX x, y, z;
	//CFixVector () : x(0), y(0), z(0) {}
	//CFixVector (FIX x, FIX y, FIX z) : x(x), y(y), z(z) {}
	//CFixVector (CFixVector& v) : x(v.x), y(v.y), z(v.z) {}

inline INT32 Read (FILE* fp) { 
	x = read_FIX (fp);
	y = read_FIX (fp);
	z = read_FIX (fp);
	return 1;
	}

void Write (FILE* fp) { 
	write_FIX (x, fp);
	write_FIX (y, fp);
	write_FIX (z, fp);
	}
};

class CFixMatrix {
public:
	CFixVector rvec, uvec, fvec;

	inline INT32 Read (FILE* fp) { 
		rvec.Read (fp);
		uvec.Read (fp);
		fvec.Read (fp);
		return 1;
	}

	inline void Write (FILE* fp) { 
		rvec.Write (fp);
		uvec.Write (fp);
		fvec.Write (fp);
	}
};

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

class CVertex : public CFixVector {
public:
	UINT8 m_status;
	CVertex () : m_status(0) {}
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

typedef struct {
  FIX		play_time;  //total time (in seconds) of clip
  INT32	num_frames;
  FIX		frame_time; //time (in seconds) of each frame
  INT32	flags;
  INT16	sound_num;
  UINT16	frames[VCLIP_MAX_FRAMES];
  FIX		light_value;
} VCLIP;

typedef struct {
  VCLIP   vc;			   //imbedded vclip
  FIX		 time_left;		   //for sequencing
  INT32	 frame_count;		   //for sequencing
  INT16	 changing_wall_texture;	   //Which element of Textures array to replace.
  INT16	 changing_object_texture;  //Which element of ObjBitmapPtrs array to replace.
  INT32	 flags;			   //see above
  INT32	 crit_clip;		   //use this clip instead of above one when mine critical
  INT32	 dest_bm_num;		//use this bitmap when monitor destroyed
  INT32	 dest_vclip;		//what vclip to play when exploding
  INT32	 dest_eclip;		//what eclip to play when exploding
  FIX	 dest_size;		//3d size of explosion
  INT32	 sound_num;		//what sound this makes
  INT32	 nSegment,nSide;	//what segP & side, for one-shot clips
} ECLIP;

typedef struct {
  FIX		 play_time;
  INT16	 num_frames;
  INT16	 frames[MAX_CLIP_FRAMES2];
  INT16	 open_sound;
  INT16	 close_sound;
  INT16	 flags;
  char	 filename[13];
  char	 pad;
} WCLIP;

//describes a list of joint positions
typedef struct {
  INT16  n_joints;
  INT16  offset;
} JOINTLIST;

typedef struct {
  INT32	      model_num;		  // which polygon model?
  CFixVector	gun_points[MAX_GUNS];	  // where each gun model is
  UINT8			gun_submodels[MAX_GUNS];  // which submodel is each gun in?

  INT16 		exp1_vclip_num;
  INT16		exp1_sound_num;

  INT16 		exp2_vclip_num;
  INT16		exp2_sound_num;

  INT8		weapon_type;
  INT8		weapon_type2;		  // Secondary weapon number, -1 means none, otherwise gun #0 fires this weapon.
  INT8		n_guns;			  // how many different gun positions
  INT8		contains_id;		  // ID of powerup this robot can contain.

  INT8		contains_count;		  // Max number of things this instance can contain.
  INT8		contains_prob;		  // Probability that this instance will contain something in N/16
  INT8		contains_type;		  // Type of thing contained, robot or powerup, in bitmaps.tbl, !0=robot, 0=powerup
  INT8		kamikaze;		  // !0 means commits suicide when hits you, strength thereof. 0 means no.

  INT16		score_value;		  // Score from this robot.
  INT8		badass;			  // Dies with badass explosion, and strength thereof, 0 means NO.
  INT8		energy_drain;		  // Points of energy drained at each collision.

  FIX		lighting;		  // should this be here or with polygon model?
  FIX		strength;		  // Initial shields of robot

  FIX		mass;			  // how heavy is this thing?
  FIX		drag;			  // how much drag does it have?

  FIX		field_of_view[NDL];	  // compare this value with forward_vector.dot.vector_to_player,
					  // ..if field_of_view <, then robot can see player
  FIX		firing_wait[NDL];	  // time in seconds between shots
  FIX		firing_wait2[NDL];	  // time in seconds between shots
  FIX		turn_time[NDL];		  // time in seconds to rotate 360 degrees in a dimension
// -- unused, mk, 05/25/95	FIX		fire_power[NDL];   // damage done by a hit from this robot
// -- unused, mk, 05/25/95	FIX		shield[NDL];	   // shield strength of this robot
  FIX		max_speed[NDL];		  // maximum speed attainable by this robot
  FIX		circle_distance[NDL];	  // distance at which robot circles player

  INT8		rapidfire_count[NDL];	  // number of shots fired rapidly
  INT8		evade_speed[NDL];	  // rate at which robot can evade shots, 0=none, 4=very fast
  INT8		cloak_type;		  // 0=never, 1=always, 2=except-when-firing
  INT8		attack_type;		  // 0=firing, 1=charge (like green guy)

  UINT8		see_sound;		  // sound robot makes when it first sees the player
  UINT8		attack_sound;		  // sound robot makes when it attacks the player
  UINT8		claw_sound;		  // sound robot makes as it claws you (attack_type should be 1)
  UINT8		taunt_sound;		  // sound robot makes after you die

  INT8		boss_flag;		  // 0 = not boss, 1 = boss.  Is that surprising?
  INT8		companion;		  // Companion robot, leads you to things.
  INT8		smart_blobs;		  // how many smart blobs are emitted when this guy dies!
  INT8		energy_blobs;		  // how many smart blobs are emitted when this guy gets hit by energy weapon!

  INT8		thief;			  // !0 means this guy can steal when he collides with you!
  INT8		pursuit;		  // !0 means pursues player after he goes around a corner.
					  // ..4 = 4/2 pursue up to 4/2 seconds after becoming invisible if up to 4
					  // ..segments away
  INT8		lightcast;		  // Amount of light cast. 1 is default.  10 is very large.
  INT8		death_roll;		  // 0 = dies without death roll. !0 means does death roll, larger = faster
					  // ..and louder

  //boss_flag, companion, thief, & pursuit probably should also be bits in the flags byte.
  UINT8		flags;			  // misc properties
  UINT8		pad[3];			  // alignment

  UINT8		deathroll_sound;	  // if has deathroll, what sound?
  UINT8		glow;			  // apply this light to robot itself. stored as 4:4 FIXed-point
  UINT8		behavior;		  // Default behavior.
  UINT8		aim;			  // 255 = perfect, less = more likely to miss.  0 != random, would look stupid.
					  // ..0=45 degree spread.  Specify in bitmaps.tbl in range 0.0..1.0

  //animation info
  JOINTLIST anim_states[MAX_GUNS+1][N_ANIM_STATES];

  INT32		always_0xabcd;		  // debugging

} ROBOT_INFO;

typedef struct {
  INT16 jointnum;
  CAngleVector angles;
} JOINTPOS;

typedef struct {
  INT8	render_type;		// How to draw 0=laser, 1=blob, 2=object
  INT8	persistent;		// 0 = dies when it hits something, 1 = continues (eg, fusion cannon)
  INT16	model_num;		// Model num if rendertype==2.
  INT16	model_num_inner;	// Model num of inner part if rendertype==2.

  INT8	flash_vclip;		// What vclip to use for muzzle flash
  INT8	robot_hit_vclip;	// What vclip for impact with robot
  INT16	flash_sound;		// What sound to play when fired

  INT8	wall_hit_vclip;		// What vclip for impact with wall
  INT8	fire_count;		// Number of bursts fired from EACH GUN per firing.
				// ..For weapons which fire from both sides, 3*fire_count shots will be fired.
  INT16	robot_hit_sound;	// What sound for impact with robot

  INT8	ammo_usage;		// How many units of ammunition it uses.
  INT8	weapon_vclip;		// Vclip to render for the weapon, itself.
  INT16	wall_hit_sound;		// What sound for impact with wall

  INT8	destroyable;		// If !0, this weapon can be destroyed by another weapon.
  INT8	matter;			// Flag: set if this object is matter (as opposed to energy)
  INT8	bounce;			// 1==always bounces, 2=bounces twice
  INT8	homing_flag;		// Set if this weapon can home in on a target.

  UINT8	speedvar;		// allowed variance in speed below average, /128: 64 = 50% meaning if speed = 100,
				// ..can be 50..100

  UINT8	flags;			// see values above

  INT8	flash;			// Flash effect
  INT8	afterburner_size;	// Size of blobs in F1_0/16 units, specify in bitmaps.tbl as floating point.
				// ..Player afterburner size = 2.5.

  INT8	children;		// ID of weapon to drop if this contains children.  -1 means no children.

  FIX	energy_usage;		// How much fuel is consumed to fire this weapon.
  FIX	fire_wait;		// Time until this weapon can be fired again.

  FIX	multi_damage_scale;	// Scale damage by this amount when applying to player in multiplayer.
				// ..F1_0 means no change.

  UINT16 bitmap;		// Pointer to bitmap if rendertype==0 or 1.

  FIX	blob_size;		// Size of blob if blob type
  FIX	flash_size;		// How big to draw the flash
  FIX	impact_size;		// How big of an impact
  FIX	strength[NDL];		// How much damage it can inflict
  FIX	speed[NDL];		// How fast it can move, difficulty level based.
  FIX	mass;			// How much mass it has
  FIX	drag;			// How much drag it has
  FIX	thrust;			// How much thrust it has
  FIX	po_len_to_width_ratio;	// For polyobjects, the ratio of len/width. (10 maybe?)
  FIX	light;			// Amount of light this weapon casts.
  FIX	lifetime;		// Lifetime in seconds of this weapon.
  FIX	damage_radius;		// Radius of damage caused by weapon, used for missiles (not lasers) to apply
				// ..to damage to things it did not hit
//-- unused--	FIX	damage_force;	 // Force of damage caused by weapon, used for missiles (not lasers) to
// ..apply to damage to things it did not hit.
// damage_force was a real mess.  Wasn't Difficulty_level based, and was being applied instead of weapon's
// ..actual strength.  Now use 2*strength instead. --MK, 01/19/95
  UINT16	picture;	// a picture of the weapon for the cockpit
  UINT16	hires_picture;	// a hires picture of the above
} WEAPON_INFO;

typedef struct {
  INT32	vclip_num;
  INT32	hit_sound;
  FIX	size;			// 3d size of longest dimension
  FIX	light;			// amount of light cast by this powerup, set in bitmaps.tbl
} POWERUP_TYPE_INFO;

//used to describe a polygon model
typedef struct {
  INT32			n_models;
  INT32 			model_data_size;
  UINT8*			model_data;
  INT32 			submodel_ptrs[MAX_SUBMODELS];
  CFixVector 	submodel_offsets[MAX_SUBMODELS];
  CFixVector 	submodel_norms[MAX_SUBMODELS];	  // norm for sep plane
  CFixVector 	submodel_pnts[MAX_SUBMODELS];	  // point on sep plane
  FIX 			submodel_rads[MAX_SUBMODELS];	  // radius for each submodel
  UINT8 			submodel_parents[MAX_SUBMODELS];  // what is parent for each submodel
  CFixVector 	submodel_mins[MAX_SUBMODELS];
  CFixVector   submodel_maxs[MAX_SUBMODELS];
  CFixVector 	mins, maxs;			  // min, max for whole model
  FIX				rad;
  UINT8			n_textures;
  UINT16			first_texture;
  UINT8			simpler_model;			  // alternate model with less detail (0 if none, model_num+1 else)
//  CFixVector min, max;
} POLYMODEL;

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

struct dvector {
  double x,y,z;
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

class CObjPhysicsInfo {
public:
	CFixVector velocity;   /*velocity vector of this object */
	CFixVector thrust;     /*constant force applied to this object */
	FIX        mass;       /*the mass of this object */
	FIX        drag;       /*how fast this slows down */
	FIX        brakes;     /*how much brakes applied */
	CFixVector rotvel;     /*rotational velecity (angles) */
	CFixVector rotthrust;  /*rotational acceleration */
	FIXANG     turnroll;   /*rotation caused by turn banking */
	UINT16     flags;      /*misc physics flags */

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

/*stuctures for different kinds of simulation */

class CObjLaserInfo {
public:
	INT16 parent_type;     /* The type of the parent of this object */
	INT16 parent_num;      /* The object's parent's number */
	INT32 parent_signature;/* The object's parent's signature... */
	FIX   creation_time;   /*  Absolute time of creation. */
	INT8  last_hitobj;     /*  For persistent weapons (survive object collision), object it most recently hit. */
	INT8  track_goal;      /*  Object this object is tracking. */
	FIX   multiplier;      /*  Power if this is a fusion bolt (or other super weapon to be added). */

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

class CObjExplosionInfo {
public:
	FIX   spawn_time;     /* when lifeleft is < this, spawn another */
	FIX   delete_time;    /* when to delete object */
	INT8  delete_objnum;  /* and what object to delete */
	INT8  attach_parent;  /* explosion is attached to this object */
	INT8  prev_attach;    /* previous explosion in attach list */
	INT8  next_attach;    /* next explosion in attach list */

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

class CObjLightInfo {
public:
  FIX  intensity;    /*how bright the light is */

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

class CObjPowerupInfo {
public:
	INT32  count;      /*how many/much we pick up (vulcan cannon only?) */

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

class CObjVClipInfo {
public:
	INT32 vclip_num;
	FIX	frametime;
	INT8	framenum;

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

/*structures for different kinds of rendering */

class CObjPolyModelInfo {
public:
	INT32      model_num;        /*which polygon model */
	CAngleVector anim_angles[MAX_SUBMODELS];  /*angles for each subobject */
	INT32      subobj_flags;     /*specify which subobjs to draw */
	INT32      tmap_override;    /*if this is not -1, map all face to this */
	INT8       alt_textures;     /*if not -1, use these textures instead */

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

class CObjAIInfo {
public:
	UINT8  behavior;            /*  */
	INT8   flags[MAX_AI_FLAGS]; /* various flags, meaning defined by constants */
	INT16  hide_segment;        /*  Segment to go to for hiding. */
	INT16  hide_index;          /*  Index in Path_seg_points */
	INT16  path_length;         /*  Length of hide path. */
	INT16  cur_path_index;      /*  Current index in path. */

	INT16  follow_path_start_seg;  /*  Start segment for robot which follows path. */
	INT16  follow_path_end_seg;    /*  End segment for robot which follows path. */

	INT32  danger_laser_signature;
	INT16  danger_laser_num;

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

class CSmokeInfo {
public:
	INT32			nLife;
	INT32			nSize [2];
	INT32			nParts;
	INT32			nSpeed;
	INT32			nDrift;
	INT32			nBrightness;
	UINT8			color [4];
	char			nSide;
	char			nType;
	char			bEnabled;

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

class CLightningInfo {
public:
	INT32			nLife;
	INT32			nDelay;
	INT32			nLength;
	INT32			nAmplitude;
	INT32			nOffset;
	INT16			nLightnings;
	INT16			nId;
	INT16			nTarget;
	INT16			nNodes;
	INT16			nChildren;
	INT16			nSteps;
	char			nAngle;
	char			nStyle;
	char			nSmoothe;
	char			bClamp;
	char			bPlasma;
	char			bSound;
	char			bRandom;
	char			bInPlane;
	char			bEnabled;
	UINT8			color [4];

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

class CSoundInfo {
public:
	INT32			nVolume;
	char			szFilename [40];
	char			bEnabled;

	INT32 Read (FILE* fp, INT32 version);
	void Write (FILE* fp, INT32 version);
};

class CGameItem {
public:
	virtual INT32 Read (FILE* fp, INT32 version = 0, bool bFlag = false) = 0;
	virtual void Write (FILE* fp, INT32 version = 0, bool bFlag = false) = 0;
	virtual CGameItem* Next (void) { return this + 1; }
};

class CGameObject : public CGameItem {
public:
	INT16			signature;     // reduced size to save memory 
	INT8			type;          // what type of object this is... robot, weapon, hostage, powerup, fireball 
	INT8			id;            // which form of object...which powerup, robot, etc. 
	UINT8			control_type;  // how this object is controlled 
	UINT8			movement_type; // how this object moves 
	UINT8			render_type;   //  how this object renders 
	UINT8			flags;         // misc flags 
	UINT8			multiplayer;   // object only available in multiplayer games 
	INT16			nSegment;      // segment number containing object 
	CFixVector	pos;           // absolute x,y,z coordinate of center of object 
	CFixMatrix	orient;        // orientation of object in world 
	FIX			size;          // 3d size of object - for collision detection 
	FIX			shields;       // Starts at maximum, when <0, object dies.. 
	CFixVector	last_pos;      // where object was last frame 
	INT8			contains_type; //  Type of object this object contains (eg, spider contains powerup) 
	INT8			contains_id;   //  ID of object this object contains (eg, id = blue type = key) 
	INT8			contains_count;// number of objects of type:id this object contains 

	//movement info, determined by MOVEMENT_TYPE 
	union {
		CObjPhysicsInfo	physInfo; // a physics object 
		CFixVector			spinRate; // for spinning objects 
		} mType;

	//control info, determined by CONTROL_TYPE 
	union {
		CObjLaserInfo     laserInfo;
		CObjExplosionInfo explInfo;   //NOTE: debris uses this also 
		CObjAIInfo			aiInfo;
		CObjLightInfo     lightInfo;  //why put this here?  Didn't know what else to do with it. 
		CObjPowerupInfo   powerupInfo;
		} cType;

	//render info, determined by RENDER_TYPE 
	union {
		CObjPolyModelInfo	polyModelInfo;     //polygon model 
		CObjVClipInfo		vClipInfo;    //vclip 
		CSmokeInfo			smokeInfo;
		CLightningInfo		lightningInfo;
		CSoundInfo			soundInfo;
		} rType;

	//CGameObject () { memset (this, 0, sizeof (*this)); }

	virtual INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual CGameItem* Next (void) { return this + 1; }
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
};

class CWall : public CSideKey, public CGameItem {
public:
	FIX		hps;            /* "Hit points" of the wall.  */
	INT32		linkedWall;		 /* number of linked wall */
	UINT8		type;           /* What kind of special wall. */
	UINT16	flags;          /* Flags for the wall.     */
	UINT8		state;          /* Opening, closing, etc. */
	UINT8		nTrigger;       /* Which trigger is associated with the wall. */
	INT8		nClip;          /* Which  animation associated with the wall.  */
	UINT8		keys;           /* which keys are required */
 
 // the following two Descent2 bytes replace the "INT16 pad" of Descent1
	INT8		controlling_trigger; // which trigger causes something to happen here.
		// Not like "trigger" above, which is the trigger on this wall.
		//	Note: This gets stuffed at load time in gamemine.c.  
		// Don't try to use it in the editor.  You will be sorry!
	INT8		cloak_value;	// if this wall is cloaked, the fade value

	INT32 Read (FILE* fp, INT32 version = 0, bool bFlag = false);
	void Write (FILE* fp, INT32 version = 0, bool bFlag = false);
	virtual CGameItem* Next (void) { return this + 1; }
};

class CActiveDoor : public CGameItem {
public:
  INT32		n_parts;	   // for linked walls
  INT16		nFrontWall[2]; // front wall numbers for this door
  INT16		nBackWall[2];  // back wall numbers for this door
  FIX			time;		   // how long been opening, closing, waiting

	virtual INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual CGameItem* Next (void) { return this + 1; }
};

class CCloakingWall : public CGameItem {    // NEW for Descent 2
public:
	INT16		nFrontWall;	  // front wall numbers for this door
	INT16		nBackWall; 	  // back wall numbers for this door
	FIX		front_ls[4]; 	  // front wall saved light values
	FIX		back_ls[4];	  // back wall saved light values
	FIX		time;		  // how long been cloaking or decloaking

	virtual INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
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

class CTriggerTargets {
public:
	INT16		m_count;
	CSideKey	m_targets [MAX_TRIGGER_TARGETS];

	CTriggerTargets () : m_count (0) {}

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

};

class CTrigger : public CTriggerTargets, public CGameItem {
public:
	UINT8		type;
	UINT16	flags;
	INT16		nObject;
	FIX		value;
	FIX		time;
	UINT16	nIndex;

	//inline CSideKey& operator[](UINT32 i) { return targets [i]; }

	virtual INT32 Read (FILE *fp, INT32 version, bool bObjTrigger);
	virtual void Write (FILE *fp, INT32 version, bool bObjTrigger);
	virtual CGameItem* Next (void) { return this + 1; }
};

// New stuff, 10/14/95: For shooting out lights and monitors.
// Light cast upon vert_light vertices in nSegment:nSide by some light
class CLightDeltaValue : public CSideKey, public CGameItem {
public:
	UINT8 vert_light [4];

	virtual INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual CGameItem* Next (void) { return this + 1; }
};

// Light at nSegment:nSide casts light on count sides beginning at index (in array CLightDeltaValues)
class CLightDeltaIndex : public CSideKey, public CGameItem {
public:
	UINT16 count;
	UINT16 index;

	virtual INT32 Read (FILE *fp, INT32 version, bool bD2X);
	virtual void Write (FILE *fp, INT32 version, bool bD2X);
	virtual CGameItem* Next (void) { return this + 1; }
};

//extern CLightDeltaIndex    Dl_indices[MAX_LIGHT_DELTA_INDICES];
//extern CLightDeltaValue CLightDeltaValues[MAX_LIGHT_DELTA_VALUES];
//extern INT32	     Num_static_lights;


class CReactorTrigger : public CTriggerTargets, public CGameItem {
public:
	INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
	virtual CGameItem* Next (void) { return this + 1; }
};

class CRobotMaker : public CGameItem {
public:
	INT32  objFlags [2]; /* Up to 32 different Descent 1 robots */
	//  INT32  robot_flags2;// Additional 32 robots for Descent 2
	FIX    hitPoints;  /* How hard it is to destroy this particular matcen */
	FIX    interval;    /* Interval between materializations */
	INT16  nSegment;      /* Segment this is attached to. */
	INT16  nFuelCen; /* Index in fuelcen array. */

	INT32 Read (FILE *fp, INT32 version = 0, bool bFlag = false);
	void Write (FILE *fp, INT32 version = 0, bool bFlag = false);
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
  INT32 num_textures;
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

class CFlickeringLight : public CSideKey {
public:
	UINT32 mask;    // bits with 1 = on, 0 = off
	FIX timer;		 // always set to 0
	FIX delay;      // time for each bit in mask (INT32 seconds)
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

