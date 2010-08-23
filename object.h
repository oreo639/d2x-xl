#ifndef OBJECT_TYPES_H
#define OBJECT_TYPES_H

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
#include "Types.h"
#include "VectorTypes.h"

typedef struct {
  INT16 jointnum;
  CAngleVector angles;
} JOINTPOS;

typedef struct {
  INT8	renderType;		// How to draw 0=laser, 1=blob, 2=object
  INT8	persistent;		// 0 = dies when it hits something, 1 = continues (eg, fusion cannon)
  INT16	nModel;		// Model num if rendertype==2.
  INT16	nInnerModel;	// Model num of inner part if rendertype==2.

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

  INT8	destructible;		// If !0, this weapon can be destroyed by another weapon.
  INT8	matter;			// Flag: set if this object is matter (as opposed to energy)
  INT8	bounce;			// 1==always bounces, 2=bounces twice
  INT8	homing_flag;		// Set if this weapon can home in on a target.

  UINT8	speedvar;		// allowed variance in speed below average, /128: 64 = 50% meaning if speed = 100,
				// ..can be 50..100

  UINT8	flags;				// see values above
  INT8	flash;				// Flash effect
  INT8	afterburnerSize;	// Size of blobs in F1_0/16 units, specify in bitmaps.tbl as floating point. Player afterburner size = 2.5.
  INT8	children;				// ID of weapon to drop if this contains children.  -1 means no children.
  FIX		energy_usage;			// How much fuel is consumed to fire this weapon.
  FIX		fire_wait;				// Time until this weapon can be fired again.
  FIX		multi_damage_scale;	// Scale damage by this amount when applying to player in multiplayer. F1_0 means no change.
  UINT16 bitmap;					// Pointer to bitmap if rendertype==0 or 1.
  FIX		blobSize;				// Size of blob if blob type
  FIX		flashSize;				// How big to draw the flash
  FIX		impactSize;				// How big of an impact
  FIX		strength[NDL];			// How much damage it can inflict
  FIX		speed[NDL];				// How fast it can move, difficulty level based.
  FIX		mass;						// How much mass it has
  FIX		drag;						// How much drag it has
  FIX		thrust;					// How much thrust it has
  FIX		poLenToWidthRatio;	// For polyobjects, the ratio of len/width. (10 maybe?)
  FIX		light;					// Amount of light this weapon casts.
  FIX		lifetime;				// Lifetime in seconds of this weapon.
  FIX		damageRadius;			// Radius of damage caused by weapon, used for missiles (not lasers) to apply to damage to things it did not hit
  UINT16	picture [2];			// a picture and a hires picture of the weapon for the cockpit 
} WEAPON_INFO;

typedef struct {
  INT32	vclip_num;
  INT32	hit_sound;
  FIX		size;				// 3d size of longest dimension
  FIX		light;			// amount of light cast by this powerup, set in bitmaps.tbl
} POWERUP_TYPE_INFO;

//used to describe a polygon model
typedef struct {
  INT32			n_models;
  INT32 			model_dataSize;
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
  UINT8			simpler_model;			  // alternate model with less detail (0 if none, nModel+1 else)
//  CFixVector min, max;
} POLYMODEL;

class CObjPhysicsInfo {
public:
	tFixVector velocity;   /*velocity vector of this object */
	tFixVector thrust;     /*constant force applied to this object */
	FIX        mass;       /*the mass of this object */
	FIX        drag;       /*how fast this slows down */
	FIX        brakes;     /*how much brakes applied */
	tFixVector rotvel;     /*rotational velecity (angles) */
	tFixVector rotthrust;  /*rotational acceleration */
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
	INT32      nModel;        /*which polygon model */
	tAngleVector anim_angles[MAX_SUBMODELS];  /*angles for each subobject */
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

typedef struct tObjContentsInfo {
	INT8			type; //  Type of object this object contains (eg, spider contains powerup) 
	INT8			id;   //  ID of object this object contains (eg, id = blue type = key) 
	INT8			count;// number of objects of type:id this object contains 
} tObjContentsInfo;

typedef struct tGameObject {
	INT16			signature;     // reduced size to save memory 
	INT8			type;          // what type of object this is... robot, weapon, hostage, powerup, fireball 
	INT8			id;            // which form of object...which powerup, robot, etc. 
	UINT8			controlType;  // how this object is controlled 
	UINT8			movementType; // how this object moves 
	UINT8			renderType;   //  how this object renders 
	UINT8			flags;         // misc flags 
	UINT8			multiplayer;   // object only available in multiplayer games 
	INT16			nSegment;      // segment number containing object 
	CFixVector	pos;           // absolute x,y,z coordinate of center of object 
	CFixMatrix	orient;        // orientation of object in world 
	FIX			size;          // 3d size of object - for collision detection 
	FIX			shields;       // Starts at maximum, when <0, object dies.. 
	CFixVector	lastPos;			// where object was last frame 
	tObjContentsInfo contents;
} tGameObject;

class CGameObject : public CGameItem {
public:
	tGameObject	m_info;
	//movement info, determined by MOVEMENT_TYPE 
	union {
		CObjPhysicsInfo	physInfo; // a physics object 
		tFixVector			spinRate; // for spinning objects 
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
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
	virtual CGameItem* Next (void) { return this + 1; }
};

#endif // OBJECT_TYPES_H

