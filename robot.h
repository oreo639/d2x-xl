#ifndef __robot_h
#define __robot_h

//describes a list of joint positions
typedef struct tJointList {
public:
	INT16  n_joints;
	INT16  offset;

	void Read (FILE* fp) {
		n_joints = read_INT16 (fp);
		offset = read_INT16 (fp);
		}
	void Write (FILE* fp) {
		 write_INT16 (n_joints, fp);
		 write_INT16 (offset, fp);
		}
} tJointList;

typedef struct tRobotGunInfo {
	CFixVector	points;
	UINT8			subModels;

	void Read (FILE* fp);
	void Write (FILE* fp);
} tRobotGunInfo;

typedef struct tRobotExplInfo {
	INT16			nClip;
	INT16			nSound;

	void Read (FILE* fp);
	void Write (FILE* fp);
} tRobotExplInfo;

typedef struct tRobotContentsInfo {
	INT8			id;			// ID of powerup this robot can contain.
	INT8			count;		// Max number of things this instance can contain.
	INT8			prob;			// Probability that this instance will contain something in N/16
	INT8			type;			// Type of thing contained, robot or powerup, in bitmaps.tbl, !0=robot, 0=powerup

	void Read (FILE* fp);
	void Write (FILE* fp);
} tRobotContentsInfo;

typedef struct tRobotSoundInfo {
	UINT8			see;			// sound robot makes when it first sees the player
	UINT8			attack;		// sound robot makes when it attacks the player
	UINT8			claw;			// sound robot makes as it claws you (m_info.attackType should be 1)
	UINT8			taunt;		// sound robot makes after you die

	void Read (FILE* fp);
	void Write (FILE* fp);
} tRobotSoundInfo;

typedef struct tRobotCombatInfo {
public:
	FIX			fieldOfView;		// compare this value with forward_vector.dot.vector_to_player, if fieldOfView <, then robot can see player
	FIX			firingWait [2];	// time in seconds between shots
	FIX			turnTime;			// time in seconds to rotate 360 degrees in a dimension
	FIX			maxSpeed;			// maximum speed attainable by this robot
	FIX			circleDistance;	// distance at which robot circles player
	INT8			rapidFire;			// number of shots fired rapidly
	INT8			evadeSpeed;			// rate at which robot can evade shots, 0=none, 4=very fast

	void Read (FILE* fp, int nField);
	void Write (FILE* fp, int nField);
} tRobotCombatInfo;

typedef struct tRobotInfo {
	INT32						nModel;		  // which polygon model?
	tRobotGunInfo			guns [MAX_GUNS];
	tRobotExplInfo			expl [2];
	INT8						weaponType [2];
	INT8						n_guns;				// how many different gun positions
	tRobotContentsInfo	contents;
	INT8						kamikaze;			// !0 means commits suicide when hits you, strength thereof. 0 means no.
	INT16						scoreValue;			// Score from this robot.
	INT8						badass;				// Dies with badass explosion, and strength thereof, 0 means NO.
	INT8						drainEnergy;		// Points of energy drained at each collision.
	FIX						lighting;			// should this be here or with polygon model?
	FIX						strength;			// Initial shields of robot
	FIX						mass;					// how heavy is this thing?
	FIX						drag;					// how much drag does it have?
	tRobotCombatInfo		combat [NDL];
	INT8						cloakType;			// 0=never, 1=always, 2=except-when-firing
	INT8						attackType;			// 0=firing, 1=charge (like green guy)
	tRobotSoundInfo		sounds;
	INT8						bossFlag;			// 0 = not boss, 1 = boss.  Is that surprising?
	INT8						companion;			// Companion robot, leads you to things.
	INT8						smartBlobs;			// how many smart blobs are emitted when this guy dies!
	INT8						energyBlobs;		// how many smart blobs are emitted when this guy gets hit by energy weapon!
	INT8						thief;				// !0 means this guy can steal when he collides with you!
	INT8						pursuit;				// !0 means pursues player after he goes around a corner.
														// ..4 = 4/2 pursue up to 4/2 seconds after becoming invisible if up to 4
														// ..segments away
	INT8						lightCast;			// Amount of light cast. 1 is default.  10 is very large.
	INT8						deathRoll;			// 0 = dies without death roll. !0 means does death roll, larger = faster and louder
														// m_info.bossFlag, companion, thief, & pursuit probably should also be bits in the flags byte.
	UINT8						flags;				// misc properties
	UINT8						bCustom;
	UINT8						pad [2];				// alignment
	UINT8						deathRollSound;	// if has deathroll, what sound?
	UINT8						glow;					// apply this light to robot itself. stored as 4:4 FIXed-point
	UINT8						behavior;			// Default behavior.
	UINT8						aim;					// 255 = perfect, less = more likely to miss.  0 != random, would look stupid.
														// ..0=45 degree spread.  Specify in bitmaps.tbl in range 0.0..1.0
	//animation info
	tJointList				animStates [MAX_GUNS+1][N_ANIM_STATES];
	INT32						always_0xabcd;		  // debugging
} tRobotInfo;

class CRobotInfo : public CGameItem {
	public:
		tRobotInfo	m_info;

	virtual CGameItem* Next (void) { return this + 1; }
	virtual INT32 Read (FILE* fp, INT32 version = 0, bool bFlag = false);
	virtual void Write (FILE* fp, INT32 version = 0, bool bFlag = false);
	virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }
};

BOOL HasCustomRobots ();

#endif //__robot_h