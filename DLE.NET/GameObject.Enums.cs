using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    partial class GameObject
    {
        enum Type : byte
        {
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
            COUNT = 21,
            NONE = 255 // unused object 
        }

        enum EffectId : byte
        {
            PARTICLES = 0,
            LIGHTNING = 1,
            SOUND = 2
        }

        enum ControlType : byte
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
        }

        enum MovementType : byte
        {
            NONE = 0, //doesn't move 
            PHYSICS = 1, //moves by physics 
            SPINNING = 3 //this object doesn't move, just sits and spins 
        }

        enum RenderType : byte
        {
            NONE = 0, //does not render 
            POLYOBJ = 1, //a polygon model 
            FIREBALL = 2, //a fireball 
            LASER = 3, //a laser 
            HOSTAGE = 4, //a hostage 
            POWERUP = 5, //a powerup 
            MORPH = 6, //a robot being morphed 
            WEAPON_VCLIP = 7, //a weapon that renders as a vclip 
            THRUSTER = 8, // like afterburner, but doesn't cast light
            EXPLBLAST = 9, // white explosion light blast
            SHRAPNELS = 10, // white explosion light blast
            SMOKE = 11,
            LIGHTNING = 12,
            SOUND = 13
        }

        enum State
        {
            EXPLODING = 1, // this object is exploding 
            SHOULD_BE_DEAD = 2, // this object should be dead, so next time we can, we should delete this object. 
            DESTROYED = 4, // this has been killed, and is showing the dead version 
            SILENT = 8, // this makes no sound when it hits a wall.  
            ATTACHED = 16, // this object is a fireball attached to another object 
            HARMLESS = 32, // this object does no damage.  Added to make quad lasers do 1.5 damage as normal lasers. 
        }

        enum PhysicsFlag : byte
        {
            TURNROLL = 1, // roll when turning 
            LEVELLING = 2, // level object with closest side 
            BOUNCE = 4, // bounce (not slide) when hit will 
            WIGGLE = 8, // wiggle while flying 
            STICK = 16, // object sticks (stops moving) when hits wall 
            PERSISTENT = 32, // object keeps going even after it hits another object (eg, fusion cannon) 
            USES_THRUST = 64 // this object uses its thrust 
        }
    }
}
