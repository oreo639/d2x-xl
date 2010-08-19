//using DLE.NET.GameMine.Types;

public struct FixVector 
{
    int x, y, z;
}

public struct AngleVector 
{
    short p, b, h;
}

public struct FixMatrix 
{
    FixVector rvec, uvec, fvec;
} 

struct DoubleVector 
{
    double x,y,z;
}

struct APOINT {
    short x, y, z;
} 

namespace DLE.NET
{
    public partial class GameMine
    {


        public struct TMAP_INFO {
          byte	    flags;		//values defined above
          fixed byte pad [3];	    //keep alignment
          int	    lighting;	//how much light this casts
          int	    damage;	    //how much damage being against this does (for lava)
          short	    eclip_num;	//the eclip that changes this, or -1
          short	    destroyed;	//bitmap to show when destroyed, or -1
          short	    slide_u, slide_v;   //slide rates of texture, stored in 8:8 int
        } 

        struct VCLIP {
            int	play_time;  //total time (in seconds) of clip
            int	    num_frames;
            int	    frame_time; //time (in seconds) of each frame
            int	    flags;
            short	 sound_num;
            fixed ushort frames [VCLIP_MAX_FRAMES];
            int	    light_value;
        } 

        struct ECLIP {
            VCLIP   vc;			        //embedded vclip
            int     time_left;		    //for sequencing
            int	    frame_count;		//for sequencing
            short	changing_wall_texture;   //Which element of Textures array to replace.
            short	changing_object_texture; //Which element of ObjBitmapPtrs array to replace.
            int	    flags;			    //see above
            int	    crit_clip;		    //use this clip instead of above one when mine critical
            int	    dest_bm_num;		//use this bitmap when monitor destroyed
            int	    dest_vclip;		    //what vclip to play when exploding
            int	    dest_eclip;		    //what eclip to play when exploding
            int	    dest_size;		    //3d size of explosion
            int	    sound_num;		    //what sound this makes
            int	    segnum, sidenum;	//what seg & side, for one-shot clips
        } 

        struct WCLIP {
            int	        play_time;
            short	    num_frames;
            fixed short	frames [MAX_CLIP_FRAMES2];
            short	    open_sound;
            short	    close_sound;
            short	    flags;
            fixed char  filename [13];
            char	    pad;
        } ;

        //describes a list of joint positions
        struct JOINTLIST {
          short  n_joints;
          short  offset;
        } 

        struct ROBOT_INFO {
            int	      	model_num;		  // which polygon model?
            fixed FixVector	gun_points [MAX_GUNS];	  // where each gun model is
            fixed byte	gun_submodels [MAX_GUNS];  // which submodel is each gun in?
            short 	    exp1_vclip_num;
            short		exp1_sound_num;
            short 	    exp2_vclip_num;
            short		exp2_sound_num;
            char		weapon_type;
            char		weapon_type2;		// Secondary weapon number, -1 means none, otherwise gun #0 fires this weapon.
            char		n_guns;			    // how many different gun positions
            char		contains_id;		// ID of powerup this robot can contain.
            char		contains_count;		// Max number of things this instance can contain.
            char		contains_prob;		// Probability that this instance will contain something in N/16
            char		contains_type;		// Type of thing contained, robot or powerup, in bitmaps.tbl, !0=robot, 0=powerup
            char		kamikaze;		    // !0 means commits suicide when hits you, strength thereof. 0 means no.
            short		score_value;		// Score from this robot.
            char		badass;			    // Dies with badass explosion, and strength thereof, 0 means NO.
            char		energy_drain;		// Points of energy drained at each collision.
            int		    lighting;		    // should this be here or with polygon model?
            int		    strength;		    // Initial shields of robot
            int		    mass;			    // how heavy is this thing?
            int		    drag;			    // how much drag does it have?
            fixed int   field_of_view [NDL];// compare this value with forward_vector.dot.vector_to_player,
            fixed int	firing_wait [NDL];	// time in seconds between shots
            fixed int	firing_wait2 [NDL];	// time in seconds between shots
            fixed int	turn_time [NDL];	// time in seconds to rotate 360 degrees in a dimension
            fixed int	max_speed [NDL];	// maximum speed attainable by this robot
            fixed int	circle_distance [NDL]; // distance at which robot circles player
            fixed char	rapidfire_count [NDL]; // number of shots fired rapidly
            fixed char	evade_speed [NDL];	// rate at which robot can evade shots, 0=none, 4=very fast
            char		cloak_type;		    // 0=never, 1=always, 2=except-when-firing
            char		attack_type;		// 0=firing, 1=charge (like green guy)
            byte		see_sound;		    // sound robot makes when it first sees the player
            byte		attack_sound;		// sound robot makes when it attacks the player
            byte		claw_sound;		    // sound robot makes as it claws you (attack_type should be 1)
            byte		taunt_sound;		// sound robot makes after you die
            char		boss_flag;		    // 0 = not boss, 1 = boss.  Is that surprising?
            char		companion;		    // Companion robot, leads you to things.
            char		smart_blobs;		// how many smart blobs are emitted when this guy dies!
            char		energy_blobs;		// how many smart blobs are emitted when this guy gets hit by energy weapon!
            char		thief;			    // !0 means this guy can steal when he collides with you!
            char		pursuit;		    // !0 means pursues player after he goes around a corner.
					                        // ..4 = 4/2 pursue up to 4/2 seconds after becoming invisible if up to 4
					                        // ..segments away
            char		lightcast;		    // Amount of light cast. 1 is default.  10 is very large.
            char		death_roll;		    // 0 = dies without death roll. !0 means does death roll, larger = faster and louder
            byte		flags;			    // misc properties
            fixed byte	pad [3];	        // alignment
            byte		deathroll_sound;	// if has deathroll, what sound?
            byte		glow;			    // apply this light to robot itself. stored as 4:4 FIXed-point
            byte		behavior;		    // Default behavior.
            byte		aim;			    // 255 = perfect, less = more likely to miss.  0 != random, would look stupid.
					                        // ..0=45 degree spread.  Specify in bitmaps.tbl in range 0.0..1.0
            //animation info
            fixed JOINTLIST anim_states[(MAX_GUNS+1) * N_ANIM_STATES];

            int		    always_0xabcd;		// debugging

        } 

        struct JOINTPOS {
          short jointnum;
          AngleVector angles;
        } ;

        struct WEAPON_INFO {
            char	render_type;		// How to draw 0=laser, 1=blob, 2=object
            char	persistent;		    // 0 = dies when it hits something, 1 = continues (eg, fusion cannon)
            short	model_num;		    // Model num if rendertype==2.
            short	model_num_inner;	// Model num of inner part if rendertype==2.
            char	flash_vclip;		// What vclip to use for muzzle flash
            char	robot_hit_vclip;	// What vclip for impact with robot
            short	flash_sound;		// What sound to play when fired
            char	wall_hit_vclip;		// What vclip for impact with wall
            char	fire_count;		    // Number of bursts fired from EACH GUN per firing.
			                            // ..For weapons which fire from both sides, 3*fire_count shots will be fired.
            short	robot_hit_sound;	// What sound for impact with robot
            char	ammo_usage;		    // How many units of ammunition it uses.
            char	weapon_vclip;		// Vclip to render for the weapon, itself.
            short	wall_hit_sound;		// What sound for impact with wall
            char	destructible;		// If !0, this weapon can be destroyed by another weapon.
            char	matter;			    // Flag: set if this object is matter (as opposed to energy)
            char	bounce;			    // 1==always bounces, 2=bounces twice
            char	homing_flag;		// Set if this weapon can home in on a target.
            byte	speedvar;		    // allowed variance in speed below average, /128: 64 = 50% meaning if speed = 100, can be 50..100
            byte	flags;			    // see values above
            char	flash;			    // Flash effect
            char	afterburner_size;	// Size of blobs in F1_0/16 units, specify in bitmaps.tbl as floating point. Player afterburner size = 2.5.
            char	children;		    // ID of weapon to drop if this contains children.  -1 means no children.
            int	    energy_usage;		// How much fuel is consumed to fire this weapon.
            int	    fire_wait;		    // Time until this weapon can be fired again.
            int	    multi_damage_scale;	// Scale damage by this amount when applying to player in multiplayer. F1_0 means no change.
            ushort  bitmap;		        // Pointer to bitmap if rendertype==0 or 1.
            int	    blob_size;		    // Size of blob if blob type
            int	    flash_size;		    // How big to draw the flash
            int	    impact_size;	    // How big of an impact
            fixed int  strength[NDL];   // How much damage it can inflict
            fixed int  speed[NDL];		// How fast it can move, difficulty level based.
            int	    mass;			    // How much mass it has
            int	    drag;			    // How much drag it has
            int	    thrust;			    // How much thrust it has
            int	    po_len_to_width_ratio;	// For polyobjects, the ratio of len/width. (10 maybe?)
            int	    light;			    // Amount of light this weapon casts.
            int	    lifetime;		    // Lifetime in seconds of this weapon.
            int	    damage_radius;		// Radius of damage caused by weapon, used for missiles (not lasers) to apply to damage to things it did not hit
            ushort  picture;	        // a picture of the weapon for the cockpit
            ushort	hires_picture;	    // a hires picture of the above
        } 

        struct POWERUP_TYPE_INFO {
          int	vclip_num;
          int	hit_sound;
          int	size;			// 3d size of longest dimension
          int	light;			// amount of light cast by this powerup, set in bitmaps.tbl
        } 

        //used to describe a polygon model
        struct POLYMODEL {
            int			    n_models;
            int 			model_data_size;
            byte[]			model_data;
            fixed int 		submodel_ptrs[MAX_SUBMODELS];
            fixed FixVector submodel_offsets[MAX_SUBMODELS];
            fixed FixVector submodel_norms[MAX_SUBMODELS];	  // norm for sep plane
            fixed FixVector submodel_pnts[MAX_SUBMODELS];	  // point on sep plane
            fixed int 		submodel_rads[MAX_SUBMODELS];	  // radius for each submodel
            fixed byte 		submodel_parents[MAX_SUBMODELS];  // what is parent for each submodel
            fixed FixVector submodel_mins[MAX_SUBMODELS];
            fixed FixVector submodel_maxs[MAX_SUBMODELS];
            fixed FixVector mins, maxs;			  // min, max for whole model
            int				rad;
            byte			n_textures;
            ushort			first_texture;
            byte			simpler_model;			  // alternate model with less detail (0 if none, model_num+1 else)
        } 

        struct TEXTURE_LIGHT {
          int    nBaseTex;
          long   light;
        } 

        struct CUBE {
          short nSegment;
          short nSide;
          short nLine;
          short nPoint;
          short nObject;
        } 

        class Selection 
        {
	        short nSegment;
	        short nSide;
	        short nLine;
	        short nPoint;
	        short nObject;
            public Selection()
            {
		        nSegment = 0;
		        nSide = DEFAULT_SIDE;
		        nLine = DEFAULT_LINE;
		        nPoint = DEFAULT_POINT;
		        nObject = DEFAULT_OBJECT;
	        }
        }

        struct LEVEL_HEADER {
          char name[13];
          int size;
        }

        struct SUBFILE 
        {
            long    offset;
            fixed char name[13];
            int     size;
        }

        struct game_top_info 
        {
            ushort  fileinfo_signature;
            ushort  fileinfo_version;
            int     fileinfo_size;
        }     

        struct player_item_info 
        {
	        int	 offset;
	        int  size;
        } 

        struct game_item_info 
        {
	        int	 offset;
	        int	 count;
	        int  size;
        } 

        struct game_info 
        {
            ushort              fileinfo_signature;
            ushort              fileinfo_version;
            int                 fileinfo_size;
            fixed char          mine_filename[15];
            int                 level;
            player_item_info    player;
            game_item_info	    objects;
            game_item_info	    walls;
            game_item_info	    doors;
            game_item_info	    triggers;
            game_item_info	    links;
            game_item_info	    control;
            game_item_info	    botgen;
            game_item_info	    dl_indices;
            game_item_info	    delta_lights;
            game_item_info	    equipgen;
        } 


        struct active_door {
          int	 n_parts;	   // for linked walls
          short	 front_wallnum[2]; // front wall numbers for this door
          short	 back_wallnum[2];  // back wall numbers for this door
          int    time;		   // how long been opening, closing, waiting
        } active_door;

        struct cloaking_wall {    // NEW for Descent 2
          short front_wallnum;	  // front wall numbers for this door
          short	back_wallnum; 	  // back wall numbers for this door
          int	front_ls[4]; 	  // front wall saved light values
          int	back_ls[4];	  // back wall saved light values
          int	time;		  // how long been cloaking or decloaking
        } cloaking_wall;

        //
        struct WCLIP 
            {
            int	play_time;
            short	num_frames;
            short	frames[MAX_CLIP_FRAMES];
            short	open_sound;
            short	close_sound;
            short	flags;
            char	filename[13];
            char	pad;
        } w
        

        //extern char	Wall_names[7][10]; // New for Descent 2

        class CDTrigger {
        public:
          byte  type;
          ushort flags;
          short  nObject;
          char   num_links;
          int    value;
          int    time;
          fixed short  seg[MAX_TRIGGER_TARGETS];
          fixed short  side[MAX_TRIGGER_TARGETS];
          ushort	nIndex;
        }

        // New stuff, 10/14/95: For shooting out lights and monitors.
        // Light cast upon vert_light vertices in segnum:sidenum by some light
        struct DELTA_LIGHT 
            {
          short segnum;
          char  sidenum;
          char  dummy;
          fixed byte vert_light[4];
            }

        // Light at segnum:sidenum casts light on count sides beginning at index (in array Delta_lights)
        struct DL_INDEX_D2X 
        {
            short segnum;
            ushort sidenum :3;
            ushort count :13;
            ushort index;
        } 

        struct DL_INDEX_STD
        {
          short segnum;
          byte sidenum;
          byte count;
          ushort index;
        }

            [StructLayout(LayoutKind.Explicit)] 
        struct DL_INDEX {
	        [FieldOffset(0)] DL_INDEX_STD	d2;
	        [FieldOffset(0)] DL_INDEX_D2X	d2x;
        } dl_index;

        //extern dl_index    Dl_indices[MAX_DL_INDICES];
        //extern delta_light Delta_lights[MAX_DELTA_LIGHTS];
        //extern int	     Num_static_lights;


        struct control_center_trigger {
          short num_links;
          fixed short seg[MAX_TRIGGER_TARGETS];
          fixed short side[MAX_TRIGGER_TARGETS];
        } control_center_trigger;

        struct matcen_info 
        {
          fixed int  objFlags [2]; // Up to 32 different Descent 1 robots 
        //  int  robot_flags2;// Additional 32 robots for Descent 2
          int    hit_points;  // How hard it is to destroy this particular matcen 
          int    interval;    // Interval between materializations 
          short  segnum;      // Segment this is attached to. 
          short  fuelcen_num; // Index in fuelcen array. 
        }


        // pig file types 
        struct PIG_HEADER
        {
          int number_of_textures;
          int number_of_sounds;
        } 

        struct PIG_TEXTURE
        {
          char name[8];
          byte dflags; // this is only important for large bitmaps like the cockpit 
          byte xsize;
          byte ysize;
          byte flags;
          byte avg_color;
          uint offset;
        } 

        struct D2_PIG_HEADER
        {
          int signature;
          int version;
          int num_textures;
        } 

        struct D2_PIG_TEXTURE
        {
          char name[8];
          byte dflags;  // bits 0-5 anim frame num, bit 6 abm flag
          byte xsize;   // low 8 bits here, 4 more bits in pad
          byte ysize;   // low 8 bits here, 4 more bits in pad
          byte wh_extra;     // bits 0-3 xsize, bits 4-7 ysize
          byte flags;   // see BM_FLAG_XXX in define.h
          byte avg_color;   // average color
          uint offset;
        } 

        struct PIG_SOUND
        {
          fixed byte unknown[20];
        } 

        struct TEXTURE
        {
          char name[8];
          short number;
        } 

        struct FLICKERING_LIGHT
        {
          short segnum,sidenum;  // cube with light on it
          uint mask;           // bits with 1 = on, 0 = off
          int timer;		 // always set to 0
          int delay;             // time for each bit in mask (int seconds)
        }

        struct LIGHT_TIMER
        {
	        short	ticks;
	        short	impulse;
        } 

        struct LIGHT_STATUS
        {
	        bool	bIsOn;
	        bool	bWasOn;
        } 

        const int MAX_LEVELS = 1000;

        struct MISSION_DATA
        {
	        fixed char	missionName [80];
	        fixed char  missionInfo [8][80];
	        fixed int	authorFlags [2];
	        int	missionType;
	        fixed int	missionFlags [6];
	        fixed int	customFlags [3];
	        fixed char	levelList [MAX_LEVELS][17];	//18 == ########.###,####'\0' == levlname.ext,secr
	        fixed char	comment [4000];
	        int   numLevels;
	        int	numSecrets;
        } 

        struct tVertMatch 
            {
		    short		b;
		    short		i;
		    double	d;
	        }  

    }
}