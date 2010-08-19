using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    partial class GameObject
    {
        enum ObjType : byte
        {
            WALL = 0,  // A wall... not really an object, but used for collisions 
            FIREBALL = 1,  // a fireball, part of an explosion 
            ROBOT = 2,  // an evil enemy 
            HOSTAGE = 3,  // a hostage you need to rescue 
            PLAYER = 4,  // the player on the console 
            WEAPON = 5,  // a laser, missile, etc 
            CAMERA = 6,  // a camera to slew around with 
            POWERUP = 7,  // a powerup you can pick up 
            DEBRIS = 8,  // a piece of robot 
            CNTRLCEN = 9,  // the control center 
            FLARE = 10,  // a flare 
            CLUTTER = 11,  // misc objects 
            GHOST = 12,  // what the player turns into when dead 
            LIGHT = 13,  // a light source, & not much else 
            COOP = 14,  // a cooperative player object. 
            MARKER = 15,
            CAMBOT = 16,
            MONSTERBALL = 17,
            SMOKE = 18,
            EXPLOSION = 19,
            EFFECT = 20,
            COUNT = 21,
            NONE = 255  // unused object 
        }

        enum EffectId : byte
        {
            PARTICLES = 0,
            LIGHTNING = 1,
            SOUND = 2
        }

        enum ControlType : byte
        {
            NONE = 0,  // doesn't move (or change movement) 
            AI = 1,  // driven by AI 
            EXPLOSION = 2,  //explosion sequencer 
            FLYING = 4,  //the player is flying 
            SLEW = 5,  //slewing 
            FLYTHROUGH = 6,  //the flythrough system 
            WEAPON = 9,  //laser, etc. 
            REPAIRCEN = 10,  //under the control of the repair center 
            MORPH = 11,  //this object is being morphed 
            DEBRIS = 12,  //this is a piece of debris 
            POWERUP = 13,  //animating powerup blob 
            LIGHT = 14,  //doesn't actually do anything 
            REMOTE = 15,  //controlled by another net player 
            CNTRLCEN = 16  //the control center/main reactor  
        }
    }
}
