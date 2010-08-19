using System;
using System.Runtime.InteropServices;

namespace DLE.NET
{
    public partial class GameObject
    {
        struct physics_info {
            GameMine.FixVector  velocity;   //velocity vector of this object 
            GameMine.FixVector  thrust;     //constant force applied to this object 
            int                 mass;       //the mass of this object 
            int                 drag;       //how fast this slows down 
            int                 brakes;     //how much brakes applied 
            GameMine.FixVector  rotvel;     //rotational velecity (angles) 
            GameMine.FixVector  rotthrust;  //rotational acceleration 
            short               turnroll;   //rotation caused by turn banking 
            ushort              flags;      //misc physics flags 
        } 

        //stuctures for different kinds of simulation 

        struct laser_info {
            short   parent_type;     // The type of the parent of this object 
            short   parent_num;      // The object's parent's number 
            int     parent_signature;// The object's parent's signature... 
            int     creation_time;   //  Absolute time of creation. 
            char    last_hitobj;     //  For persistent weapons (survive object collision), object it most recently hit. 
            char    track_goal;      //  Object this object is tracking. 
            int     multiplier;      //  Power if this is a fusion bolt (or other super weapon to be added). 
        } 

        struct explosion_info {
            int     spawn_time;     // when lifeleft is < this, spawn another 
            int     delete_time;    // when to delete object 
            char    delete_objnum;  // and what object to delete 
            char    attach_parent;  // explosion is attached to this object 
            char    prev_attach;    // previous explosion in attach list 
            char    next_attach;    // next explosion in attach list 
        } 

        struct light_info {
            int     intensity;    //how bright the light is 
        } 

        struct powerup_info {
            int     count;      //how many/much we pick up (vulcan cannon only?) 
        } 

        struct vclip_info {
            int       vclip_num;
            int       frametime;
            char      framenum;
        } 

        //structures for different kinds of rendering 

        struct polyobj_info {
            int     model_num;        //which polygon model 
            fixed   tAngleVector anim_angles[MAX_SUBMODELS];  //angles for each subobject 
            int     subobj_flags;     //specify which subobjs to draw 
            int     tmap_override;    //if this is not -1, map all face to this 
            char    alt_textures;     //if not -1, use these textures instead 
        } polyobj_info;

        struct ai_static {
            byte    behavior;            //  
            fixed char   flags[MAX_AI_FLAGS]; // various flags, meaning defined by constants 
            short   hide_segment;        //  Segment to go to for hiding. 
            short   hide_index;          //  Index in Path_seg_points 
            short   path_length;         //  Length of hide path. 
            short   cur_path_index;      //  Current index in path. 
            short   follow_path_start_seg;  //  Start segment for robot which follows path. 
            short   follow_path_end_seg;    //  End segment for robot which follows path. 
            int     danger_laser_signature;
            short  danger_laser_num;
        } 

        struct tSmokeInfo {
	        int			nLife;
	        fixed int	nSize [2];
	        int			nParts;
	        int			nSpeed;
	        int			nDrift;
	        int			nBrightness;
	        fixed byte	color [4];
	        char		nSide;
	        char		nType;
	        char		bEnabled;
        } 

        struct tLightningInfo {
	        int			nLife;
	        int			nDelay;
	        int			nLength;
	        int			nAmplitude;
	        int			nOffset;
	        short		nBolts;
	        short		nId;
	        short		nTarget;
	        short		nNodes;
	        short		nChildren;
	        short		nSteps;
	        char		nAngle;
	        char		nStyle;
	        char		nSmoothe;
	        char		bClamp;
	        char		bPlasma;
	        char		bSound;
	        char		bRandom;
	        char		bInPlane;
	        char		bEnabled;
	        fixed byte	color [4];
        } 


        struct tSoundInfo {
	        int			nVolume;
	        fixed char	szFilename [40];
	        char		bEnabled;
        } 

        class CDObject 
            {
            short       signature;     // reduced size to save memory 
            char        type;          // what type of object this is... robot, weapon, hostage, powerup, fireball 
            char        id;            // which form of object...which powerup, robot, etc. 
            byte        control_type;  // how this object is controlled 
            byte        movement_type; // how this object moves 
            byte        render_type;   //  how this object renders 
            byte        flags;         // misc flags 
            byte	    multiplayer;   // object only available in multiplayer games 
            short       segnum;        // segment number containing object 
            FixVector   pos;           // absolute x,y,z coordinate of center of object 
            FixMatrix   orient;        // orientation of object in world 
            int         size;          // 3d size of object - for collision detection 
            int         shields;       // Starts at maximum, when <0, object dies.. 
            FixVector   last_pos;      // where object was last frame 
            char		contains_type; //  Type of object this object contains (eg, spider contains powerup) 
            char		contains_id;   //  ID of object this object contains (eg, id = blue type = key) 
            char		contains_count;// number of objects of type:id this object contains 
  
          //movement info, determined by MOVEMENT_TYPE 
            [StructLayout(LayoutKind.Explicit)] 
            struct MType 
            {
                [FieldOffset(0)] physics_info phys_info; // a physics object 
                [FieldOffset(0)] FixVector   spin_rate; // for spinning objects 
            } 

          //control info, determined by CONTROL_TYPE 
            [StructLayout(LayoutKind.Explicit)] 
            struct CType 
            {
                [FieldOffset(0)] laser_info     laser_info;
                [FieldOffset(0)] explosion_info expl_info;   //NOTE: debris uses this also 
                [FieldOffset(0)] ai_static      ai_info;
                [FieldOffset(0)] light_info     light_info;  //why put this here?  Didn't know what else to do with it. 
                [FieldOffset(0)] powerup_info   powerup_info;
            } 

          //render info, determined by RENDER_TYPE 
            [StructLayout(LayoutKind.Explicit)] 
            struct RType {
                [FieldOffset(0)] polyobj_info	pobj_info;     //polygon model 
                [FieldOffset(0)] vclip_info		vclip_info;    //vclip 
                [FieldOffset(0)] tSmokeInfo		smokeInfo;
                [FieldOffset(0)] tLightningInfo	lightningInfo;
                [FieldOffset(0)] tSoundInfo		soundInfo;
            } 

            MType mtype;
            CType ctype;
            RType rtype;
        };
    }
}
