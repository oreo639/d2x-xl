using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public class Wall : SideKey
    {
        public enum Type : byte
        {
            NORMAL = 0, // Normal wall 
            BLASTABLE = 1, // Removable (by shooting) wall 
            DOOR = 2, // Door  
            ILLUSION = 3, // Wall that appears to be there, but you can fly thru 
            OPEN = 4, // Just an open side. (Trigger) 
            CLOSED = 5, // Wall.  Used for transparent walls. 
            OVERLAY = 6, // Goes over an actual solid side.  For triggers (Descent 2)
            CLOAKED = 7, // Can see it, and see through it  (Descent 2)
            TRANSPARENT = 8
        }

        public enum Flag : ushort
        {
            BLASTED = 1, // Blasted out wall. 
            DOOR_OPENED = 2, // Open door.  
            RENDER_ADDITIVE = 4,
            DOOR_LOCKED = 8, // Door is locked. 
            DOOR_AUTO = 16, // Door automatically closes after time. 
            ILLUSION_OFF = 32, // Illusionary wall is shut off. 
            WALL_SWITCH = 64, // This wall is openable by a wall switch (Descent 2)
            BUDDY_PROOF = 128, // Buddy assumes he cannot get through this wall (Descent 2)
            IGNORE_MARKER = 256
        }

        public enum ClipFlag : byte
        {
            EXPLODES = 1, //door explodes when opening
            BLASTABLE = 2, //this is a blastable wall
            TMAP1 = 4, //this uses primary tmap, not tmap2
            HIDDEN = 8 //this uses primary tmap, not tmap2
        }

        public enum DoorState : byte
        {
            CLOSED = 0, // Door is closed 
            OPENING = 1, // Door is opening. 
            WAITING = 2, // Waiting to close 
            CLOSING = 3, // Door is closing 
            CLOAKING = 5, // Wall is going from closed -> open (Descent 2)
            DECLOAKING = 6	// Wall is going from open -> closed (Descent 2)
        }

        public enum KeyType : byte
        {
            NONE = 1,
            BLUE = 2,
            RED = 4,
            GOLD = 8
        }

        public enum VisType : byte
        {
            FLY = 1,
            RENDER = 2,
            RENDPAST = 4,
            EXTERNAL = 8,
            CLOAKED = 16 // Descent 2
        }

        public enum RenderType : byte  // WALL_IS_DOORWAY return values F/R/RP 
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
        public Type m_type;             // What kind of special wall. 
        public ushort m_flags;          // Flags for the wall.     
        public byte m_state;            // Opening, closing, etc. 
        public byte m_trigger;          // Which trigger is associated with the wall. 
        public char m_nClip;            // Which  animation associated with the wall.  
        public byte m_keys;             // which keys are required 
        public char m_controllingTrigger; // trigger targetting this wall. A bit pointless since a wall can be targetted by several triggers
        public char m_cloakValue;	    // if this wall is cloaked, the fade value
    }
}
