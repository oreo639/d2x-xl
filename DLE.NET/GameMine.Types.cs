using System;
using System.Runtime.InteropServices;

public class FixVector
{
    int x, y, z;
}

public class AngleVector
{
    short p, b, h;
}

public class FixMatrix
{
    FixVector rvec, uvec, fvec;
}

class DoubleVector
{
    double x, y, z;
}

class APOINT
{
    short x, y, z;
}

namespace DLE.NET
{
    public partial class GameMine
    {
        public class TMAP_INFO
        {
            byte flags;		//values defined above
            byte pad1, pad2, pad3;	    //keep alignment
            int lighting;	//how much light this casts
            int damage;	    //how much damage being against this does (for lava)
            short eclip_num;	//the eclip that changes this, or -1
            short destroyed;	//bitmap to show when destroyed, or -1
            short slide_u, slide_v;   //slide rates of texture, stored in 8:8 int
        }

        public class VCLIP
        {
            int play_time;  //total time (in seconds) of clip
            int num_frames;
            int frame_time; //time (in seconds) of each frame
            int flags;
            short sound_num;
            ushort [] frames = new ushort [VCLIP_MAX_FRAMES];
            int light_value;
        }

        public class ECLIP
        {
            VCLIP vc;			        //embedded vclip
            int time_left;		    //for sequencing
            int frame_count;		//for sequencing
            short changing_wall_texture;   //Which element of Textures array to replace.
            short changing_object_texture; //Which element of ObjBitmapPtrs array to replace.
            int flags;			    //see above
            int crit_clip;		    //use this clip instead of above one when mine critical
            int dest_bm_num;		//use this bitmap when monitor destroyed
            int dest_vclip;		    //what vclip to play when exploding
            int dest_eclip;		    //what eclip to play when exploding
            int dest_size;		    //3d size of explosion
            int sound_num;		    //what sound this makes
            int segnum, sidenum;	//what seg & side, for one-shot clips
        }

        public class WCLIP
        {
            int play_time;
            short num_frames;
            short [] frames = new short [MAX_CLIP_FRAMES_D2];
            short open_sound;
            short close_sound;
            short flags;
            string filename;
            char pad;
        } ;

        //describes a list of joint positions
        public class JOINTLIST
        {
            short n_joints;
            short offset;
        }

        public class ROBOT_GUN_INFO
        {
            FixVector points; // where each gun model is
            byte subModels; // which submodel is each gun in?
        }

        public class ROBOT_COMBAT_INFO
        {
            int field_of_view;      // compare this value with forward_vector.dot.vector_to_player,
            int firing_wait;	    // time in seconds between shots
            int firing_wait2;	    // time in seconds between shots
            int turn_time;	        // time in seconds to rotate 360 degrees in a dimension
            int max_speed;	        // maximum speed attainable by this robot
            int circle_distance;    // distance at which robot circles player
            char rapidfire_count;   // number of shots fired rapidly
            char evade_speed;	    // rate at which robot can evade shots, 0=none, 4=very fast
        }

        public class ROBOT_INFO
        {
            int model_num;		  // which polygon model?
            ROBOT_GUN_INFO [] gunInfo = new ROBOT_GUN_INFO [MAX_GUNS];
            short exp1_vclip_num;
            short exp1_sound_num;
            short exp2_vclip_num;
            short exp2_sound_num;
            char weapon_type;
            char weapon_type2;		// Secondary weapon number, -1 means none, otherwise gun #0 fires this weapon.
            char n_guns;			// how many different gun positions
            char contains_id;		// ID of powerup this robot can contain.
            char contains_count;	// Max number of things this instance can contain.
            char contains_prob;		// Probability that this instance will contain something in N/16
            char contains_type;		// Type of thing contained, robot or powerup, in bitmaps.tbl, !0=robot, 0=powerup
            char kamikaze;		    // !0 means commits suicide when hits you, strength thereof. 0 means no.
            short score_value;		// Score from this robot.
            char badass;			// Dies with badass explosion, and strength thereof, 0 means NO.
            char energy_drain;		// Points of energy drained at each collision.
            int lighting;		    // should this be here or with polygon model?
            int strength;		    // Initial shields of robot
            int mass;			    // how heavy is this thing?
            int drag;			    // how much drag does it have?
            ROBOT_COMBAT_INFO[] combatInfo = new ROBOT_COMBAT_INFO [NDL];
            char cloak_type;		// 0=never, 1=always, 2=except-when-firing
            char attack_type;		// 0=firing, 1=charge (like green guy)
            byte see_sound;		    // sound robot makes when it first sees the player
            byte attack_sound;		// sound robot makes when it attacks the player
            byte claw_sound;		// sound robot makes as it claws you (attack_type should be 1)
            byte taunt_sound;		// sound robot makes after you die
            char boss_flag;		    // 0 = not boss, 1 = boss.  Is that surprising?
            char companion;		    // Companion robot, leads you to things.
            char smart_blobs;		// how many smart blobs are emitted when this guy dies!
            char energy_blobs;		// how many smart blobs are emitted when this guy gets hit by energy weapon!
            char thief;			    // !0 means this guy can steal when he collides with you!
            char pursuit;		    // !0 means pursues player after he goes around a corner.
            // ..4 = 4/2 pursue up to 4/2 seconds after becoming invisible if up to 4
            // ..segments away
            char lightcast;		    // Amount of light cast. 1 is default.  10 is very large.
            char death_roll;		// 0 = dies without death roll. !0 means does death roll, larger = faster and louder
            byte flags;			    // misc properties
            byte pad1, pad2, pad3;  // alignment
            byte deathroll_sound;	// if has deathroll, what sound?
            byte glow;			    // apply this light to robot itself. stored as 4:4 FIXed-point
            byte behavior;		    // Default behavior.
            byte aim;			    // 255 = perfect, less = more likely to miss.  0 != random, would look stupid.
                                    // 0=45 degree spread.  Specify in bitmaps.tbl in range 0.0..1.0
            JOINTLIST [,] anim_states = new JOINTLIST [MAX_GUNS + 1, N_ANIM_STATES]; //animation info
            int always_0xabcd;		// debugging
        }

        class JOINTPOS
        {
            short jointnum;
            AngleVector angles;
        } ;

        class WEAPON_INFO
        {
            char render_type;		// How to draw 0=laser, 1=blob, 2=object
            char persistent;		    // 0 = dies when it hits something, 1 = continues (eg, fusion cannon)
            short model_num;		    // Model num if rendertype==2.
            short model_num_inner;	// Model num of inner part if rendertype==2.
            char flash_vclip;		// What vclip to use for muzzle flash
            char robot_hit_vclip;	// What vclip for impact with robot
            short flash_sound;		// What sound to play when fired
            char wall_hit_vclip;		// What vclip for impact with wall
            char fire_count;		    // Number of bursts fired from EACH GUN per firing.
            // ..For weapons which fire from both sides, 3*fire_count shots will be fired.
            short robot_hit_sound;	// What sound for impact with robot
            char ammo_usage;		    // How many units of ammunition it uses.
            char weapon_vclip;		// Vclip to render for the weapon, itself.
            short wall_hit_sound;		// What sound for impact with wall
            char destructible;		// If !0, this weapon can be destroyed by another weapon.
            char matter;			    // Flag: set if this object is matter (as opposed to energy)
            char bounce;			    // 1==always bounces, 2=bounces twice
            char homing_flag;		// Set if this weapon can home in on a target.
            byte speedvar;		    // allowed variance in speed below average, /128: 64 = 50% meaning if speed = 100, can be 50..100
            byte flags;			    // see values above
            char flash;			    // Flash effect
            char afterburner_size;	// Size of blobs in F1_0/16 units, specify in bitmaps.tbl as floating point. Player afterburner size = 2.5.
            char children;		    // ID of weapon to drop if this contains children.  -1 means no children.
            int energy_usage;		// How much fuel is consumed to fire this weapon.
            int fire_wait;		    // Time until this weapon can be fired again.
            int multi_damage_scale;	// Scale damage by this amount when applying to player in multiplayer. F1_0 means no change.
            ushort bitmap;		        // Pointer to bitmap if rendertype==0 or 1.
            int blob_size;		    // Size of blob if blob type
            int flash_size;		    // How big to draw the flash
            int impact_size;	    // How big of an impact
            int [] strength = new int [NDL];   // How much damage it can inflict
            int [] speed = new int [NDL];		// How fast it can move, difficulty level based.
            int mass;			    // How much mass it has
            int drag;			    // How much drag it has
            int thrust;			    // How much thrust it has
            int po_len_to_width_ratio;	// For polyobjects, the ratio of len/width. (10 maybe?)
            int light;			    // Amount of light this weapon casts.
            int lifetime;		    // Lifetime in seconds of this weapon.
            int damage_radius;		// Radius of damage caused by weapon, used for missiles (not lasers) to apply to damage to things it did not hit
            ushort picture;	        // a picture of the weapon for the cockpit
            ushort hires_picture;	    // a hires picture of the above
        }

        class POWERUP_TYPE_INFO
        {
            int vclip_num;
            int hit_sound;
            int size;			// 3d size of longest dimension
            int light;			// amount of light cast by this powerup, set in bitmaps.tbl
        }

        public class SUBMODEL_INFO
        {
            int ptrs;
            FixVector offsets;
            FixVector norms;	  // norm for sep plane
            FixVector pnts;	  // point on sep plane
            int rads;	  // radius for each submodel
            byte parents;  // what is parent for each submodel
            FixVector mins;
            FixVector maxs;
        }

        //used to describe a polygon model
        class POLYMODEL
        {
            int n_models;
            int model_data_size;
            byte [] model_data;
            SUBMODEL_INFO [] submodel = new SUBMODEL_INFO [MAX_SUBMODELS];
            FixVector mins, maxs;			  // min, max for whole model
            int rad;
            byte n_textures;
            ushort first_texture;
            byte simpler_model;			  // alternate model with less detail (0 if none, model_num+1 else)
        }

        public class TEXTURE_LIGHT
        {
            int nBaseTex;
            long light;
        }

        public class CUBE
        {
            short nSegment;
            short nSide;
            short nLine;
            short nPoint;
            short nObject;
        }

        public class Selection
        {
            short nSegment;
            short nSide;
            short nLine;
            short nPoint;
            short nObject;
            public Selection ()
            {
                nSegment = 0;
                nSide = DEFAULT_SIDE;
                nLine = DEFAULT_LINE;
                nPoint = DEFAULT_POINT;
                nObject = DEFAULT_OBJECT;
            }
        }

        public class LEVEL_HEADER
        {
            string name;
            int size;
        }

        public class SUBFILE
        {
            long offset;
            string name;
            int size;
        }

        public class ACTIVE_DOOR
        {
            int n_parts;	   // for linked walls
            short [] front_wallnum = new short [2]; // front wall numbers for this door
            short [] back_wallnum = new short [2];  // back wall numbers for this door
            int time;		   // how long been opening, closing, waiting
        }

        public class CLOAKING_WALL
        {    // NEW for Descent 2
            short front_wallnum;	  // front wall numbers for this door
            short back_wallnum; 	  // back wall numbers for this door
            int [] front_ls = new int [4]; 	  // front wall saved light values
            int [] back_ls = new int [4];	  // back wall saved light values
            int time;		  // how long been cloaking or decloaking
        }

        // New stuff, 10/14/95: For shooting out lights and monitors.
        // Light cast upon vert_light vertices in segnum:sidenum by some light
        public class LightDeltaValue : SideKey
        {
            short segnum;
            short sidenum;
            byte [] vert_light = new byte [4];
        }

        // Light at segnum:sidenum casts light on count sides beginning at index (in array Delta_lights)
        public class LightDeltaIndex : SideKey
        {
            short segnum;
            ushort sidenum;
            ushort count;
            ushort index;
        }

        //extern dl_index    Dl_indices[MAX_DL_INDICES];
        //extern delta_light Delta_lights[MAX_DELTA_LIGHTS];
        //extern int	     Num_static_lights;


        public class MATCEN_INFO
        {
            int objFlag1;          // Up to 32 different Descent 1 robots 
            int objFlag2;
            //int  robot_flags_d2x;       // Additional 32 robots for Descent 2
            int hit_points;        // How hard it is to destroy this particular matcen 
            int interval;          // Interval between materializations 
            short segnum;            // Segment this is attached to. 
            short fuelcen_num;       // Index in fuelcen array. 
        }


        // pig file types 
        public class PIG_HEADER
        {
            int number_of_textures;
            int number_of_sounds;
        }

        public class PIG_TEXTURE
        {
            string name;
            byte dflags; // this is only important for large bitmaps like the cockpit 
            byte xsize;
            byte ysize;
            byte flags;
            byte avg_color;
            uint offset;
        }

        public class D2_PIG_HEADER
        {
            int signature;
            int version;
            int num_textures;
        }

        public class D2_PIG_TEXTURE
        {
            string name;
            byte dflags;  // bits 0-5 anim frame num, bit 6 abm flag
            byte xsize;   // low 8 bits here, 4 more bits in pad
            byte ysize;   // low 8 bits here, 4 more bits in pad
            byte wh_extra;     // bits 0-3 xsize, bits 4-7 ysize
            byte flags;   // see BM_FLAG_XXX in define.h
            byte avg_color;   // average color
            uint offset;
        }

        public class PIG_SOUND
        {
            byte [] unknown = new byte [20];
        }

        public class TEXTURE
        {
            string name;
            short number;
        }

        public class FLICKERING_LIGHT
        {
            short segnum;
            short sidenum;  // cube with light on it
            uint mask;     // bits with 1 = on, 0 = off
            int timer;	  // always set to 0
            int delay;    // time for each bit in mask (int seconds)
        }

        public class LIGHT_TIMER
        {
            short ticks;
            short impulse;
        }

        public class LIGHT_STATUS
        {
            bool bIsOn;
            bool bWasOn;
        }

        const int MAX_LEVELS = 1000;

        public class MISSION_DATA
        {
            string missionName;
            string [] missionInfo = new string [8];
            int [] authorFlags = new int [2];
            int missionType;
            int [] missionFlags = new int [6];
            int [] customFlags = new int [3];
            string [] levelList = new string [MAX_LEVELS];	//18 == ########.###,####'\0' == levlname.ext,secr
            char [] comment = new char [4000];
            int numLevels;
            int numSecrets;
        }

        public class tVertMatch
        {
            short b;
            short i;
            double d;
        }

    }
}