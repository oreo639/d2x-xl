using System;
using System.IO;

namespace DLE.NET
{
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------

    public struct ObjectContents
    {
        public byte type;
        public byte id;
        public byte count;
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------

    public class ObjectLocation
    {
        public Vertex pos = new Vertex ();
        public Vertex lastPos = new Vertex ();
        public DoubleMatrix orient = new DoubleMatrix ();

        public void Clear ()
        {
            pos.Clear ();
            lastPos.Clear ();
            orient.Clear ();
        }

        public void Add (DoubleVector v)
        {
            pos.Add (v);
            lastPos.Add (v);
        }

        public void Sub (DoubleVector v)
        {
            pos.Sub (v);
            lastPos.Sub (v);
        }

        public ObjectLocation ()
        {
            Clear ();
        }
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------

    public partial class GameObject : IGameItem, IComparable<GameObject>
    {
        public int Key { get; set; }

         // ------------------------------------------------------------------------

        static int [] powerupSize = new int [GameMine.MAX_POWERUP_IDS_D2] 
        {
	        0x28000, 0x30000, 0x28000, 0x40000, 0x30000, 0x30000, 0x30000, 0x30000, 
	        0x30000, 0x30000, 0x28000, 0x30000, 0x30000, 0x40000, 0x40000, 0x40000, 
	        0x40000, 0x30000, 0x28000, 0x30000, 0x28000, 0x30000, 0x1cccc, 0x20000, 
	        0x30000, 0x29999, 0x30000, 0x30000, 0x40000, 0x40000, 0x40000, 0x40000, 
	        0x40000, 0x40000, 0x40000, 0x40000, 0x48000, 0x30000, 0x28000, 0x28000, 
	        0x30000, 0x30000, 0x40000, 0x40000, 0x40000, 0x40000, 0x38000, 0x38000
        };

        // ------------------------------------------------------------------------

        static int [] robotSize = new int [GameMine.MAX_ROBOT_IDS_TOTAL] 
        {
	        399147, 368925, 454202, 316909, 328097, 345407, 399147, 293412, 
	        300998, 308541, 246493, 283415, 283415, 227232, 200000, 598958, 
	        399147, 1597221, 290318, 345407, 323879, 339488, 294037, 1443273, 
	        378417, 408340, 586422, 302295, 524232, 405281, 736493, 892216, 
	        400000, 204718, 400000, 400000, 354534, 236192, 373267, 351215, 
	        429512, 169251, 310419, 378181, 381597, 1101683, 853047, 423359, 
	        402717, 289744, 187426, 361065, 994374, 758384, 429512, 408340, 
	        289744, 408340, 289744, 400000, 402717, 169251, 1312272, 169251, 
	        905234, 1014749, 
	        374114, 318151, 377386, 492146, 257003, 403683, // vertigo robots
	        342424, 322628, 332831, 1217722, 907806, 378960 // vertigo robots
        };

        // ------------------------------------------------------------------------

        static int [] robotShield = new int [GameMine.MAX_ROBOT_IDS_TOTAL]
        {
	        6553600, 6553600, 6553600, 1638400, 2293760, 6553600, 9830400, 16384000, 
	        2293760, 16384000, 2293760, 2293760, 2000000, 9830400, 1310720, 26214400, 
	        21299200, 131072000, 6553600, 3276800, 3276800, 4587520, 4587520, 327680000, 
	        5570560, 5242880, 9830400, 2949120, 6553600, 6553600, 7864320, 196608000, 
	        5000000, 45875200, 5000000, 5000000, 5242880, 786432, 1966080, 4587520, 
	        9830400, 1310720, 29491200, 9830400, 11796480, 262144000, 262144000, 13107200, 
	        7208960, 655360, 983040, 11141120, 294912000, 32768000, 7864320, 3932160, 
	        4587520, 5242880, 4587520, 5000000, 7208960, 1310720, 196608000, 1310720, 
	        294912000, 19660800, 
	        6553600, 6553600, 6553600, 10485760, 4587520, 16384000, // vertigo robots
	        6553600, 7864320, 7864320, 180224000, 360448000, 9830400 // vertigo robots
        };

        // ------------------------------------------------------------------------

        static byte [] robotClip = new byte [GameMine.MAX_ROBOT_IDS_TOTAL] 
        {
	        0x00, 0x02, 0x03, 0x05, 0x07, 0x09, 0x0b, 0x0d, 
	        0x0e, 0x10, 0x12, 0x13, 0x14, 0x15, 0x16, 0x18, 
	        0x19, 0x1b, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 
	        40, 41, 43, 44, 45, 46, 47, 49, 
	        50, 51, 52, 53, 55, 56, 58, 60, 	// 50,  52,  53,  and 86 were guessed at but seem to work ok
	        62, 64, 65, 67, 68, 69, 70, 71, 
	        72, 73, 74, 75, 76, 77, 78, 80, 
	        82, 83, 85, 86, 87, 88, 89, 90, 
	        91, 92, 
	        0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1 // vertigo clip numbers
        };

        // ------------------------------------------------------------------------
        // note1: 0 == not used,
        // note2: 100 and 101 are flags but will appear as shields
        //        in non multiplayer level
        static byte [] powerupClip = new byte [GameMine.MAX_POWERUP_IDS_D2] 
        {
	        36, 18, 19, 20, 24, 26, 25,  0,
	         0,  0, 34, 35, 51, 37, 38, 39,
	        40, 41, 42, 43, 44, 45, 46, 47,
	         0, 49,  0,  0, 69, 70, 71, 72,
	        77, 73, 74, 75, 76, 83, 78, 89,
	        79, 90, 91, 81,102, 82,100,101
        };

        // ------------------------------------------------------------------------

        const int POWERUP_WEAPON_MASK = 0x01;
        const int POWERUP_KEY_MASK = 0x02;
        const int POWERUP_POWERUP_MASK = 0x04;
        const int POWERUP_UNKNOWN_MASK = 0xff; // show the type if any other mask is on

        static byte [] powerupTypeTable = new byte [GameMine.MAX_POWERUP_IDS_D2 + 2] 
        {
		        POWERUP_POWERUP_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_KEY_MASK,
		        POWERUP_KEY_MASK,
		        POWERUP_KEY_MASK,
		        POWERUP_UNKNOWN_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_UNKNOWN_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_UNKNOWN_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_UNKNOWN_MASK,
		        POWERUP_UNKNOWN_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_WEAPON_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_POWERUP_MASK,
		        POWERUP_POWERUP_MASK
        };

         // ------------------------------------------------------------------------

        public enum Types : byte
        {
            NONE = 255, // unused object 
            WALL = 0, // A wall... not really an object, but used for collisions 
            FIREBALL = 1, // a fireball, part of an explosion 
            ROBOT = 2, // an evil enemy 
            HOSTAGE = 3, // a hostage you need to rescue 
            PLAYER = 4, // the player on the console 
            WEAPON = 5, // a laser, missile, etc 
            CAMERA = 6, // a camera to slew around with 
            POWERUP = 7, // a powerup you can pick up 
            DEBRIS = 8, // a piece of robot 
            CNTRLCEN = 9, // the control center 
            FLARE = 10, // a flare 
            CLUTTER = 11, // misc objects 
            GHOST = 12, // what the player turns into when dead 
            LIGHT = 13, // a light source, & not much else 
            COOP = 14, // a cooperative player object. 
            MARKER = 15,
            CAMBOT = 16,
            MONSTERBALL = 17,
            SMOKE = 18,
            EXPLOSION = 19,
            EFFECT = 20,
            COUNT = 21
        };

         // ------------------------------------------------------------------------

        public enum Ids : byte
        {
            SMALLMINE = 51
        };

         // ------------------------------------------------------------------------

        public enum ControlTypes : byte
        {
            NONE = 0, // doesn't move (or change movement) 
            AI = 1, // driven by AI 
            EXPLOSION = 2, //explosion sequencer 
            FLYING = 4, //the player is flying 
            SLEW = 5, //slewing 
            FLYTHROUGH = 6, //the flythrough system 
            WEAPON = 9, //laser, etc. 
            REPAIRCEN = 10, //under the control of the repair center 
            MORPH = 11, //this object is being morphed 
            DEBRIS = 12, //this is a piece of debris 
            POWERUP = 13, //animating powerup blob 
            LIGHT = 14, //doesn't actually do anything 
            REMOTE = 15, //controlled by another net player 
            CNTRLCEN = 16 //the control center/main reactor  
        };  

        // ------------------------------------------------------------------------
        //Movement types 

        public enum MovementTypes : byte
        {
            NONE = 0, //doesn't move 
            PHYSICS = 1, //moves by physics 
            SPINNING = 3 //this object doesn't move, just sits and spins 
        }; 

        // ------------------------------------------------------------------------
        //Render types 

        public enum RenderTypes : byte
        {
            NONE = 0, //does not render 
            POLYOBJ = 1, //a polygon model 
            FIREBALL = 2, //a fireball 
            LASER = 3, //a laser 
            HOSTAGE = 4, //a hostage 
            POWERUP = 5, //a powerup 
            MORPH = 6, //a robot being morphed 
            WEAPON_VCLIP = 7, //a weapon that renders as a vclip 
            THRUSTER = 8,	 // like afterburner, but doesn't cast light
            EXPLBLAST = 9,	 // white explosion light blast
            SHRAPNELS = 10,	 // white explosion light blast
            SMOKE = 11,
            LIGHTNING = 12,
            SOUND = 13
        };

        // ------------------------------------------------------------------------
        //misc object flags 

        public enum Flags : byte
        {
            EXPLODING = 1, //this object is exploding 
            SHOULD_BE_DEAD = 2, //this object should be dead, so next time we can, we should delete this object. 
            DESTROYED = 4, //this has been killed, and is showing the dead version 
            SILENT = 8, //this makes no sound when it hits a wall. Added by MK for weapons, if you extend it to other types, do it completely! 
            ATTACHED = 16, //this object is a fireball attached to another object 
            HARMLESS = 32 //this object does no damage.  Added to make quad lasers do 1.5 damage as normal lasers. 
        };

        // ------------------------------------------------------------------------
        //physics flags 

        public enum PhysicsFlags : byte
        {
            TURNROLL = 1, // roll when turning 
            LEVELLING = 2, // level object with closest side 
            BOUNCE = 4, // bounce (not slide) when hit will 
            WIGGLE = 8, // wiggle while flying 
            STICK = 16, // object sticks (stops moving) when hits wall 
            PERSISTENT = 32, // object keeps going even after it hits another object (eg, fusion cannon) 
            USES_THRUST = 64, // this object uses its thrust 
        };

        // ------------------------------------------------------------------------

        public short m_signature; // reduced size to save memory 
        public Types m_type; // what type of object this is... robot, weapon, hostage, powerup, fireball 
        public Ids m_id; // which form of object...which powerup, robot, etc. 
        public ControlTypes m_controlType; // how this object is controlled 
        public MovementTypes m_movementType; // how this object moves 
        public RenderTypes m_renderType; //  how this object renders 
        public Flags m_flags; // misc flags 
        public byte m_multiplayer; // object only available in multiplayer games 
        public short m_nSegment; // segment number containing object 
        public FixVector m_pos; // absolute x,y,z coordinate of center of object 
        public FixMatrix m_orient; // orientation of object in world 
        public int m_size; // 3d size of object - for collision detection 
        public int m_shields; // Starts at maximum, when <0, object dies.. 
        public FixVector m_lastPos; // where object was last frame 
        public ObjectLocation m_location = new ObjectLocation ();
        public ObjectContents m_contents = new ObjectContents { };

        public MType m_mType = new MType ();
        public CType m_cType = new CType ();
        public RType m_rType = new RType ();

         // ------------------------------------------------------------------------

        public GameObject ()
        {
            Key = 0;
        }

        public GameObject (int key)
        {
            Key = key;
        }

        //------------------------------------------------------------------------

        public int CompareTo (object obj)
        {
            if (obj == null)
                return 0;
            GameObject other = obj as GameObject;
            if (other == null)
                return 0;
            return CompareTo (other);
        }

        //------------------------------------------------------------------------

        public int CompareTo (GameObject other)
        {
            Types i = this.m_type;
            Types m = other.m_type;
            return (i < m) ? -1 : (i > m) ? 1 : 0;
        }

        //------------------------------------------------------------------------------

        public void Read (BinaryReader fp, int nVersion, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int nVersion, bool bFlag = false)
        {
        }

        // ------------------------------------------------------------------------

        public void Clear ()
        {
        }

        // ------------------------------------------------------------------------

        void Create (Types type, short nSegment) 
        {
          Vertex location = new Vertex ();

        DLE.Backup.Begin (UndoData.Flags.udObjects);
        DLE.Segments.CalcCenter (location, nSegment);
        m_signature = 0;
        m_type = type;
        m_id = (type == Types.WEAPON) ? Ids.SMALLMINE : 0;
        m_controlType = ControlTypes.NONE; /* player 0 only */
        m_movementType = MovementTypes.PHYSICS;
        m_renderType = RenderTypes.POLYOBJ;
        m_flags = 0;
        m_nSegment = DLE.Current.m_nSegment;
        m_location.pos = location;
        m_location.orient.rVec.Set (FixConverter.I2X (1), 0, 0);
        m_location.orient.uVec.Set (0, FixConverter.I2X (1), 0);
        m_location.orient.fVec.Set (0, 0, FixConverter.I2X (1));
        m_size = GameMine.PLAYER_SIZE;
        m_shields = GameMine.DEFAULT_SHIELD;
        m_rType.polyModelInfo.nModel = GameMine.PLAYER_CLIP_NUMBER;
        m_rType.polyModelInfo.nTextureOverride = -1;
        m_contents.type = 0;
        m_contents.id = 0;
        m_contents.count = 0;
        DLE.Backup.End ();
        return;
        }

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

         // ------------------------------------------------------------------------

    }
}
