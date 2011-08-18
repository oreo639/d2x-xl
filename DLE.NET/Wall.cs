using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class Wall : SideKey, IGameItem
    {
        public int Key { get; set; }

        //------------------------------------------------------------------------------

        static byte [] animClipTable = new byte [GameMine.NUM_OF_CLIPS_D2] 
            {
	         0,  1,  3,  4,  5,  6,  7,  9, 10, 11, 
	        12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
	        22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
	        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 
	        42, 43, 44, 45, 46, 47, 48, 49, 50
	        };

        static byte [] doorClipTable = new byte [GameMine.NUM_OF_CLIPS_D2] 
            {
	         1,  1,  4,  5, 10, 24,  8, 11, 13, 12, 
            14, 17, 18, 19, 20, 21, 22, 23, 25, 26, 
            28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 
            38, 39, 40, 41, 42, 43, 44, 45, 47, 48, 
            49, 50, 51, 52, 53, 54, 55, 56, 57
	        };

	    static ushort [] wallTexturesD1 = new ushort [GameMine.N_WALL_TEXTURES_D1 * 2] {
		    371,0,0,376,0,0,  0,387,0,399,413,0,419,0,0,424,  0,0,436,0,
		    0,444,0,459,0,472,486,0,492,0,500,0,508,0,515,0,521,0,529,0,
		    536,0,543,0,0,550,563,0,570,0,577,0
		    };

	    static ushort [] wallTexturesD2 = new ushort [GameMine.N_WALL_TEXTURES_D2 * 2] {
		    435,0,0,440,0,0,0,451,0,463,477,0,483,0,0,488,0,0,  500,0,
		    0,508,0,523,0,536,550,0,556,0,564,0,572,0,579,0,585,0,593,0,
		    600,0,608,0,0,615,628,0,635,0,642,0,0,649,664,0,0,672,0,687,
		    0,702,717,0,725,0,731,0,738,0,745,0,754,0,763,0,772,0,780,0,
		    0,790,806,0,817,0,827,0,838,0,849,0,858,0,863,0,0,871,0,886,
		    901,0
		    };



        public const int HITPOINTS = 100 * FixConverter.scale;
        public const int DOOR_INTERVAL = 5 * FixConverter.scale;

        //------------------------------------------------------------------------------

        public enum Types : byte
        {
            NORMAL = 0, // Normal wall 
            BLASTABLE = 1, // Removable (by shooting) wall 
            DOOR = 2, // Door  
            ILLUSION = 3, // Wall that appears to be there, but you can fly thru 
            OPEN = 4, // Just an open side. (Trigger) 
            CLOSED = 5, // Wall.  Used for transparent walls. 
            OVERLAY = 6, // Goes over an actual solid side.  For triggers (Descent 2)
            CLOAKED = 7, // Can see it, and see through it  (Descent 2)
            COLORED = 8
        }

        public enum Flags : ushort
        {
            BLASTED = 1, // Blasted out wall. 
            DOOR_OPENED = 2, // Open door.  
            RENDER_ADDITIVE = 4,
            DOOR_LOCKED = 8, // Door is locked. 
            DOOR_AUTO = 16, // Door automatically closes after time. 
            ILLUSION_OFF = 32, // Illusionary wall is shut off. 
            SWITCH = 64, // This wall is openable by a wall switch (Descent 2)
            BUDDY_PROOF = 128, // Buddy assumes he cannot get through this wall (Descent 2)
            IGNORE_MARKER = 256
        }

        public enum ClipFlags : byte
        {
            EXPLODES = 1, //door explodes when opening
            BLASTABLE = 2, //this is a blastable wall
            TMAP1 = 4, //this uses primary tmap, not tmap2
            HIDDEN = 8 //this uses primary tmap, not tmap2
        }

        public enum DoorStates : byte
        {
            CLOSED = 0, // Door is closed 
            OPENING = 1, // Door is opening. 
            WAITING = 2, // Waiting to close 
            CLOSING = 3, // Door is closing 
            CLOAKING = 5, // Wall is going from closed -> open (Descent 2)
            DECLOAKING = 6	// Wall is going from open -> closed (Descent 2)
        }

        public enum KeyTypes : byte
        {
            NONE = 1,
            BLUE = 2,
            RED = 4,
            GOLD = 8
        }

        public enum VisTypes : byte
        {
            FLY = 1,
            RENDER = 2,
            RENDPAST = 4,
            EXTERNAL = 8,
            CLOAKED = 16 // Descent 2
        }

        public enum RenderTypes : byte  // Types.IS_DOORWAY return values F/R/RP 
        {
            SOLID = 2,	// 0/1/0 wall	 
            ILLUSORY = 3,	// 1/1/0 illusory wall 
            NONE = 5,	// 1/0/1 no wall, can fly through 
            TRANSPARENT = 6,	// 0/1/1 transparent wall 
            TRANSILLUSORY = 7,	// 1/1/1 transparent illusory wall 
            EXTERNAL = 8	// 0/0/0/1 don't see it, dont fly through it 
        }

        public int m_hps;               // "Hit points" of the wall.  
        public int m_linkedWall;        // number of linked wall 
        public Types m_type;             // What kind of special wall. 
        public ushort m_flags;          // Flags for the wall.     
        public byte m_state;            // Opening, closing, etc. 
        public byte m_nTrigger;         // Which trigger is associated with the wall. 
        public sbyte m_nClip;            // Which  animation associated with the wall.  
        public KeyTypes m_keys;             // which keys are required 
        public sbyte m_controllingTrigger; // trigger targetting this wall. A bit pointless since a wall can be targetted by several triggers
        public sbyte m_cloakValue;	    // if this wall is cloaked, the fade value

        // ------------------------------------------------------------------------

        public Wall (int key = 0)
        {
            Key = key;
        }

        // ------------------------------------------------------------------------

        public Side Side { get { return DLE.Segments.Side (this); } }

        public Trigger Trigger { get { return DLE.Triggers.WallTriggers [m_nTrigger]; } }

        public Types Type { get { return m_type; } }

        // ------------------------------------------------------------------------

		public bool IsTransparent 
        { 
            get { return m_type == Types.COLORED; }
        }

		public bool IsCloaked 
        {
            get { return m_type == Types.CLOAKED; }
        }

		public bool IsIllusion 
        {
            get { return m_type == Types.ILLUSION; }
        }

		public bool IsClosed 
        {
            get { return m_type == Types.CLOSED; }
        }

        public byte Alpha
        {
            get
            {
                if (IsTransparent)
                    return (byte) ((m_hps == 0) ? 128 : FixConverter.X2I (255 * m_hps));
                if (IsCloaked || IsIllusion || IsClosed)
                    return (byte) (255 * (31 - m_cloakValue % 32) / 31);
                return 255;
            }
        }
		
        // ------------------------------------------------------------------------

        public void Setup (SideKey key, ushort nWall, Types type, sbyte nClip, ushort nTexture, bool bRedefine) 
        {
        DLE.Backup.Begin (UndoData.Flags.udWalls);
        // define new wallP
        SideKey sideKey = this as SideKey;
        sideKey = key;
        //m_nSegment = nSegment;
        //m_nSide = nSide;
        m_type = (Types) type;
        if (!bRedefine) {
	        m_nTrigger = GameMine.NO_TRIGGER;
	        m_linkedWall = -1; //OppositeWall (nOppWall, nSegment, nSide) ? nOppWall : -1;
	        }
        switch (m_type) {
	        case Types.BLASTABLE:
                m_nClip = (nClip == (sbyte)-1) ? (sbyte) 6 : nClip;
		        m_hps = HITPOINTS;
		        // define door textures based on clip number
		        SetTextures (nTexture);
		        break;

            case Types.DOOR:
		        m_nClip = (nClip == (sbyte) -1) ? (sbyte) 1 : nClip;
		        m_hps = 0;
		        // define door textures based on clip number
		        SetTextures (nTexture);
		        break;

            case Types.CLOSED:
            case Types.ILLUSION:
		        m_nClip = -1;
		        m_hps = 0;
		        // define texture to be energy
		        if ((short) nTexture == -1)
			        DLE.Segments.SetTextures (key, (ushort) (DLE.IsD1File ? 328 : 353), 0); // energy
		        else if (nClip == -2)
			        DLE.Segments.SetTextures (key, 0, nTexture);
		        else
			        DLE.Segments.SetTextures (key, nTexture, 0);
		        break;

            case Types.OVERLAY: // d2 only
		        m_nClip = -1;
		        m_hps = 0;
		        // define box01a
                unchecked
                {
                    DLE.Segments.SetTextures (key, (ushort)-1, 414);
                }
		        break;

            case Types.CLOAKED:
		        m_cloakValue = 17;
		        break;

	        case Types.COLORED:
		        m_cloakValue = 0;
		        break;

	        default:
		        m_nClip = -1;
		        m_hps = 0;
		        DLE.Segments.SetTextures (key, nTexture, 0);
		        break;
	        }
        m_flags = 0;
        m_state = 0;
        m_keys = 0;
        m_controllingTrigger = 0;

        // set uvls of new texture
        DLE.Segments [key.m_nSegment].SetUV (key.m_nSide, 0, 0);
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        void SetTextures (ushort nTexture) 
        {
            Side side = Side;
            sbyte nClip = m_nClip;

            DLE.Backup.Begin (UndoData.Flags.udWalls);
            if (IsDoor)
            {
                if (DLE.IsD1File)
                    side.SetTextures (wallTexturesD1 [2 * (int)nClip], wallTexturesD1 [2 * (int)nClip + 1]);
                else
                    side.SetTextures (wallTexturesD2 [2 * (int)nClip], wallTexturesD2 [2 * (int)nClip + 1]);
            }
            else if (nTexture >= 0)
            {
                side.SetTextures (nTexture, 0);
            }
            else
                return;
            DLE.Backup.End ();
            DLE.MineView.Refresh ();
        }

        // ------------------------------------------------------------------------

        public void Read (BinaryReader fp, int version, bool bFlag)
        {
            m_nSegment = (short) fp.ReadInt32 ();
            m_nSide = (short) fp.ReadInt32 (); 
            m_hps = fp.ReadInt32 ();
            m_linkedWall = fp.ReadInt32 ();
            m_type = (Types) fp.ReadByte ();
            m_flags = (version < 37) ?  (ushort) fp.ReadByte () : fp.ReadUInt16 ();         
            m_state = fp.ReadByte ();         
            m_nTrigger = fp.ReadByte ();       
            m_nClip = fp.ReadSByte ();      
            m_keys = (KeyTypes) fp.ReadByte ();          
            m_controllingTrigger = fp.ReadSByte ();
            m_cloakValue = fp.ReadSByte ();
        }

        // ------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int version, bool bFlag)
        {
            fp.Write ((int) m_nSegment);
            fp.Write ((int) m_nSide); 
            fp.Write (m_hps);
            fp.Write (m_linkedWall);
            fp.Write ((byte) m_type);
            if (version < 37) 
	            fp.Write ((sbyte) m_flags);
            else
	            fp.Write (m_flags);         
            fp.Write (m_state);         
            fp.Write (m_nTrigger);       
            fp.Write (m_nClip);      
            fp.Write ((byte) m_keys);          
            fp.Write (m_controllingTrigger);
            fp.Write (m_cloakValue);
        }
        
        // ------------------------------------------------------------------------

        public new void Clear ()
        {
            m_nSegment = 0;
            m_nSide = 0;
            m_hps = 0;
            m_linkedWall = 0;
            m_type = 0;
            m_flags = 0;
            m_state = 0;
            m_nTrigger = 0;
            m_nClip = 0;
            m_keys = 0;
            m_controllingTrigger = 0;
            m_cloakValue = 0;
            base.Clear ();
        }

        // ------------------------------------------------------------------------

        public void SetTrigger (short nTrigger)
        {
            m_nTrigger = (byte) nTrigger;
        }

        //------------------------------------------------------------------------------

        public bool IsDoor
        {
            get { return (m_type == Types.BLASTABLE) || (m_type == Types.DOOR); }
        }

        //------------------------------------------------------------------------------

        public bool IsVisible
        {
            get { return (m_type != Types.OPEN); }
        }

        //------------------------------------------------------------------------------

        public bool IsVariable
        {
            get
            {
                Trigger trig = Trigger;
                if (trig == null)
                    return false;
                Trigger.Types trigType = trig.Type;
                return (trigType == Trigger.Types.ILLUSION_OFF) ||
                       (trigType == Trigger.Types.ILLUSION_ON) ||
                       (trigType == Trigger.Types.CLOSE_WALL) ||
                       (trigType == Trigger.Types.OPEN_WALL) ||
                       (trigType == Trigger.Types.LIGHT_OFF) ||
                       (trigType == Trigger.Types.LIGHT_ON);
            }
        }

        // ------------------------------------------------------------------------

        public int SetClip (short nTexture)
        {
	        string texName = DLE.Textures.Name (nTexture);

            if (string.Compare (texName, @"wall01 - anim", true) != 0)
	        return m_nClip = 0;
            int i = texName.IndexOf (@"door");
            if (i >= 0) {
                int nDoor = int.Parse (texName.Substring (i));
	            for (i = 1; i < GameMine.NUM_OF_CLIPS_D2; i++)
		            if (nDoor == doorClipTable [i]) {
			            m_nClip = (sbyte) animClipTable [i];
			            DLE.MineView.Refresh ();
			            return i;
			            }
	            }
            return -1;
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

    }
}
