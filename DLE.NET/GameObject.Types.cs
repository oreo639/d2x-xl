using System;
using System.Runtime.InteropServices;

namespace DLE.NET
{
    public partial class GameObject
    {
        struct PHYSICS_INFO {
            FixVector  velocity;   //velocity vector of this object 
            FixVector  thrust;     //constant force applied to this object 
            int        mass;       //the mass of this object 
            int        drag;       //how fast this slows down 
            int        brakes;     //how much brakes applied 
            FixVector  rotvel;     //rotational velecity (angles) 
            FixVector  rotthrust;  //rotational acceleration 
            short      turnroll;   //rotation caused by turn banking 
            ushort     flags;      //misc physics flags 
        } 

        //stuctures for different kinds of simulation 

        struct LASER_INFO {
            short   parent_type;     // The type of the parent of this object 
            short   parent_num;      // The object's parent's number 
            int     parent_signature;// The object's parent's signature... 
            int     creation_time;   //  Absolute time of creation. 
            char    last_hitobj;     //  For persistent weapons (survive object collision), object it most recently hit. 
            char    track_goal;      //  Object this object is tracking. 
            int     multiplier;      //  Power if this is a fusion bolt (or other super weapon to be added). 
        } 

        struct EXPLOSION_INFO {
            int     spawn_time;     // when lifeleft is < this, spawn another 
            int     delete_time;    // when to delete object 
            char    delete_objnum;  // and what object to delete 
            char    attach_parent;  // explosion is attached to this object 
            char    prev_attach;    // previous explosion in attach list 
            char    next_attach;    // next explosion in attach list 
        } 

        struct LIGHT_INFO {
            int     intensity;    //how bright the light is 
        } 

        struct POWERUP_INFO {
            int     count;      //how many/much we pick up (vulcan cannon only?) 
        } 

        struct VCLIP_INFO {
            int       vclip_num;
            int       frametime;
            char      framenum;
        } 

        //structures for different kinds of rendering 

        struct POLYOBJ_INFO {
            int     model_num;        //which polygon model 
            fixed   AngleVector anim_angles[GameMine.MAX_SUBMODELS];  //angles for each subobject 
            int     subobj_flags;     //specify which subobjs to draw 
            int     tmap_override;    //if this is not -1, map all face to this 
            char    alt_textures;     //if not -1, use these textures instead 
        }

        struct AI_STATIC {
            byte    behavior;            //  
            fixed char   flags[GameMine.MAX_AI_FLAGS]; // various flags, meaning defined by constants 
            short   hide_segment;        //  Segment to go to for hiding. 
            short   hide_index;          //  Index in Path_seg_points 
            short   path_length;         //  Length of hide path. 
            short   cur_path_index;      //  Current index in path. 
            short   follow_path_start_seg;  //  Start segment for robot which follows path. 
            short   follow_path_end_seg;    //  End segment for robot which follows path. 
            int     danger_laser_signature;
            short  danger_laser_num;
        } 

        struct PARTICLE_INFO {
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

        struct LIGHTNING_INFO {
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

        //movement info, determined by MOVEMENT_TYPE 
        [StructLayout(LayoutKind.Explicit)] 
        struct MType 
        {
            [FieldOffset(0)] PHYSICS_INFO phys_info; // a physics object 
            [FieldOffset(0)] FixVector   spin_rate; // for spinning objects 
        } 

        //control info, determined by CONTROL_TYPE 
        [StructLayout(LayoutKind.Explicit)] 
        struct CType 
        {
            [FieldOffset(0)] LASER_INFO     laser_info;
            [FieldOffset(0)] EXPLOSION_INFO expl_info;   //NOTE: debris uses this also 
            [FieldOffset(0)] AI_STATIC      ai_info;
            [FieldOffset(0)] LIGHT_INFO     light_info;  //why put this here?  Didn't know what else to do with it. 
            [FieldOffset(0)] POWERUP_INFO   powerup_info;
        } 

        //render info, determined by RENDER_TYPE 
        [StructLayout(LayoutKind.Explicit)] 
        struct RType 
        {
            [FieldOffset(0)] POLYOBJ_INFO	pobj_info;     //polygon model 
            [FieldOffset(0)] VCLIP_INFO		vclip_info;    //vclip 
            [FieldOffset(0)] PARTICLE_INFO	particleInfo;
            [FieldOffset(0)] LIGHTNING_INFO	lightningInfo;
            [FieldOffset(0)] tSoundInfo		soundInfo;
        }
    }
}
