using System;
using System.Runtime.InteropServices;

namespace DLEdotNET
{
    public partial class GameObject
    {
        //------------------------------------------------------------------------------

        public class PHYSICS_INFO
        {
            public FixVector velocity;     //velocity vector of this object 
            public FixVector thrust;       //constant force applied to this object 
            public int mass;               //the mass of this object 
            public int drag;               //how fast this slows down 
            public int brakes;             //how much brakes applied 
            public FixVector rotvel;       //rotational velecity (angles) 
            public FixVector rotthrust;    //rotational acceleration 
            public short turnroll;         //rotation caused by turn banking 
            public ushort flags;           //misc physics flags 
        }

        //------------------------------------------------------------------------------

        public class LASER_INFO
        {
            public short parent_type;      // The type of the parent of this object 
            public short parent_num;       // The object's parent's number 
            public int parent_signature;   // The object's parent's signature... 
            public int creation_time;      //  Absolute time of creation. 
            public char last_hitobj;       //  For persistent weapons (survive object collision), object it most recently hit. 
            public char track_goal;        //  Object this object is tracking. 
            public int multiplier;         //  Power if this is a fusion bolt (or other super weapon to be added). 
        }

        //------------------------------------------------------------------------------

        public class EXPLOSION_INFO
        {
            public int spawn_time;         // when lifeleft is < this, spawn another 
            public int delete_time;        // when to delete object 
            public char delete_objnum;     // and what object to delete 
            public char attach_parent;     // explosion is attached to this object 
            public char prev_attach;       // previous explosion in attach list 
            public char next_attach;       // next explosion in attach list 
        }

        //------------------------------------------------------------------------------

        public class LightInfo
        {
            public int intensity;          //how bright the light is 
        }

        //------------------------------------------------------------------------------

        public class PowerupInfo
        {
            public int count;              //how many/much we pick up (vulcan cannon only?) 
        }

        //------------------------------------------------------------------------------

        public class VClipInfo
        {
            public int vclip_num;
            public int frametime;
            public char framenum;
        }

        //------------------------------------------------------------------------------

        public class PolyModelInfo
        {
            public int nModel;          //which polygon model 
            public AngleVector [] animAngles = new AngleVector [GameMine.MAX_SUBMODELS];  //angles for each subobject 
            public int nSubObjFlags;       //specify which subobjs to draw 
            public int nTextureOverride;      //if this is not -1, map all face to this 
            public sbyte altTextures;      //if not -1, use these textures instead 
        }

        //------------------------------------------------------------------------------

        public class AIInfo
        {
            public byte behavior;
            public char [] flags = new char [GameMine.MAX_AI_FLAGS]; // various flags, meaning defined by constants 
            public short hide_segment;             //  Segment to go to for hiding. 
            public short hide_index;               //  Index in Path_seg_points 
            public short path_length;              //  Length of hide path. 
            public short cur_path_index;           //  Current index in path. 
            public short follow_path_start_seg;    //  Start segment for robot which follows path. 
            public short follow_path_end_seg;      //  End segment for robot which follows path. 
            public int danger_laser_signature;
            public short danger_laser_num;
        }

        //------------------------------------------------------------------------------

        public class ParticleInfo
        {
            public int nLife;
            public int [] nSize = new int [2];
            public int nParts;
            public int nSpeed;
            public int nDrift;
            public int nBrightness;
            public byte [] color = new byte [4];
            public char nSide;
            public char nType;
            public char bEnabled;
        }

        //------------------------------------------------------------------------------

        public class LightningInfo
        {
            public int nLife;
            public int nDelay;
            public int nLength;
            public int nAmplitude;
            public int nOffset;
            public short nBolts;
            public short nId;
            public short nTarget;
            public short nNodes;
            public short nChildren;
            public short nSteps;
            public char nAngle;
            public char nStyle;
            public char nSmoothe;
            public char bClamp;
            public char bPlasma;
            public char bSound;
            public char bRandom;
            public char bInPlane;
            public char bEnabled;
            public byte [] color;
        }


        //------------------------------------------------------------------------------

        public class SoundInfo
        {
            public int nVolume;
            public string filename;
            public char bEnabled;
        }

        //------------------------------------------------------------------------------

        [StructLayout (LayoutKind.Explicit)]
        public class MType
        {
            [FieldOffset (0)]
            public PHYSICS_INFO physInfo; // a physics object 
            [FieldOffset (0)]
            public FixVector spinRate; // for spinning objects 
        }

        //------------------------------------------------------------------------------

        [StructLayout (LayoutKind.Explicit)]
        public class CType
        {
            [FieldOffset (0)]
            public LASER_INFO laserInfo;
            [FieldOffset (0)]
            public EXPLOSION_INFO explInfo;   //NOTE: debris uses this also 
            [FieldOffset (0)]
            public AIInfo aiInfo;
            [FieldOffset (0)]
            public LightInfo lightInfo;  //why put this here?  Didn't know what else to do with it. 
            [FieldOffset (0)]
            public PowerupInfo powerup_info;
        }

        //------------------------------------------------------------------------------

        [StructLayout (LayoutKind.Explicit)]
        public class RType
        {
            [FieldOffset (0)]
            public PolyModelInfo polyModelInfo;     //polygon model 
            [FieldOffset (0)]
            public VClipInfo vClipInfo;    //vclip 
            [FieldOffset (0)]
            public ParticleInfo particleInfo;
            [FieldOffset (0)]
            public LightningInfo lightningInfo;
            [FieldOffset (0)]
            public SoundInfo soundInfo;
        }

        //------------------------------------------------------------------------------

    }
}
