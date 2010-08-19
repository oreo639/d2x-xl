﻿namespace DLE.NET
{
    public partial class GameMine
    {
        enum MacroModes : uint
        {
            MACRO_OFF = 0,
            MACRO_RECORD = 1,
            MACRO_PLAY = 2
        }

        enum SelectModes : uint
        {
            POINT_MODE = 0,
            LINE_MODE = 1,
            SIDE_MODE = 2,
            CUBE_MODE = 3,
            OBJECT_MODE = 4,
            BLOCK_MODE = 5,
            N_SELECT_MODES = 6
        }

        enum EditModes : uint
        {
            EDIT_OFF = 0,
            EDIT_MOVE = 1,
            N_EDIT_MODES = 2
        }

        enum MarkModes : byte
        {
            NEW_MASK = 0x20, // used on vert_status                
            DELETED_MASK = 0x40, // used on wall_bitmask & vert_status 
            MARKED_MASK = 0x80 // used on wall_bitmask & vert_status 
        }

        enum ShowMode : uint
        {
            SHOW_LINES_POINTS = 1,
            SHOW_LINES_PARTIAL = 2,
            SHOW_LINES_ALL = 4,
            SHOW_LINES_NEARBY = 8,
            SHOW_FILLED_POLYGONS = 16
        }

        enum GameFileType : uint
        {
            RDL = 0,
            RL2 = 1
        }

        enum WallSide : byte
        {
            LEFT = 0,
            TOP = 1,
            RIGHT = 2,
            BOTTOM = 3,
            BACK = 4,
            FRONT = 5

        }

        enum PowerupMask : byte
        {
            POWERUP_WEAPON_MASK = 1,
            POWERUP_KEY_MASK = 2,
            POWERUP_POWERUP_MASK = 4,
            POWERUP_UNKNOWN_MASK = 255 // show the type if any other mask is on
        }

        enum AIBehavior : byte
        {
            AIB_STILL = 0x80,
            MIN_BEHAVIOR = AIB_STILL,
            AIB_NORMAL = 0x81,
            AIB_GET_BEHIND = 0x82,
            AIB_RUN_FROM = 0x83,
            AIB_SNIPE = 0x84,
            AIB_STATION = 0x85,
            AIB_FOLLOW_PATH = 0x86,
            MAX_BEHAVIOR = AIB_FOLLOW_PATH
        }

        enum SegInsMode : uint
        {
            ORTHOGONAL = 0,
            EXTEND = 1,
            MIRROR = 2,
            N_CUBE_MODES = 3
        }

        enum TriggerTypeD1 : byte
        {
            CONTROL_DOORS = 1,	// Control Trigger 
            SHIELD_DAMAGE = 2,	// Shield Damage Trigger 
            ENERGY_DRAIN = 4,	// Energy Drain Trigger 
            EXIT = 8,	// End of level Trigger 
            ON = 16,	// Whether Trigger is active 
            ONE_SHOT = 32,	// If Trigger can only be triggered once 
            MATCEN = 64,	// Trigger for materialization centers 
            ILLUSION_OFF = 128,	// Switch Illusion OFF trigger 
            SECRET_EXIT = 256,	// Exit to secret level 
            ILLUSION_ON = 512,	// Switch Illusion ON trigger 
            OPEN_WALL = 1024,
            CLOSE_WALL = 2048,
            MAKE_ILLUSIONARY = 4096
        }

        enum TriggerTypeD2 : byte
        {
            OPEN_DOOR = 0,  // Open a door
            CLOSE_DOOR = 1,  // Close a door
            MATCEN = 2,  // Activate a matcen
            EXIT = 3,  // End the level
            SECRET_EXIT = 4,  // Go to secret level
            ILLUSION_OFF = 5,  // Turn an illusion off
            ILLUSION_ON = 6,  // Turn an illusion on
            UNLOCK_DOOR = 7,  // Unlock a door
            LOCK_DOOR = 8,  // Lock a door
            OPEN_WALL = 9,  // Makes a wall open
            CLOSE_WALL = 10,  // = Makes a wall closed
            ILLUSORY_WALL = 11,  // Makes a wall illusory
            LIGHT_OFF = 12,  // Turn a light off
            LIGHT_ON = 13,  // Turn s light on
            TELEPORT = 14,
            SPEEDBOOST = 15,
            CAMERA = 16,
            SHIELD_DAMAGE_D2 = 17,
            ENERGY_DRAIN_D2 = 18,
            CHANGE_TEXTURE = 19,
            SMOKE_LIFE = 20,
            SMOKE_SPEED = 21,
            SMOKE_DENS = 22,
            SMOKE_SIZE = 23,
            SMOKE_DRIFT = 24,
            COUNTDOWN = 25,
            SPAWN_BOT = 26,
            SMOKE_BRIGHTNESS = 27,
            SET_SPAWN = 28,
            MESSAGE = 29,
            SOUND = 30,
            MASTER = 31,
            ENABLE_TRIGGER = 32,
            DISABLE_TRIGGER = 33,
            NUM_TRIGGER_TYPES = 34,
            SHIELD_DAMAGE = 100,    // added to support d1 shield damage
            ENERGY_DRAIN = 101    // added to support d1 energy drain
        }

        enum TriggerFlagD1 : uint
        {
            CONTROL_DOORS = 1, // Control Trigger 
            SHIELD_DAMAGE = 2, // Shield Damage Trigger 
            ENERGY_DRAIN = 4, // Energy Drain Trigger 
            EXIT = 8, // End of level Trigger 
            ON = 16, // Whether Trigger is active 
            ONE_SHOT = 32, // If Trigger can only be triggered once 
            MATCEN = 64, // Trigger for materialization centers 
            ILLUSION_OFF = 128, // Switch Illusion OFF trigger 
            SECRET_EXIT = 256, // Exit to secret level 
            ILLUSION_ON = 512, // Switch Illusion ON trigger 
            OPEN_WALL = 1024,
            CLOSE_WALL = 2048,
            MAKE_ILLUSIONARY = 4096
            }

        enum TriggerFlagD2 : byte
        {
            NO_MESSAGE = 1, // Don't show a message when triggered
            ONE_SHOT = 2, // Only trigger once
            DISABLED = 4, // Set after one-shot fires
            PERMANENT = 8, // indestructable switch for repeated operation
            ALTERNATE = 16, // switch will assume the opposite function after operation
            SET_ORIENT = 32, // switch will assume the opposite function after operation
            SILENT = 64,
            AUTOPLAY = 128
        }

        enum TextureFlag : int
        {
            TRANSPARENT = 1,
            SUPER_TRANSPARENT = 2,
            NO_LIGHTING = 4,
            RLE = 8,
            PAGED_OUT = 16,
            RLE_BIG = 32
        }

        enum SegType : byte
        {
            NONE = 0,
            FUELCEN = 1,
            REPAIRCEN = 2,
            CONTROLCEN = 3,
            ROBOTMAKER = 4,
            COUNT_D1 = 5,

            GOAL_BLUE = 5, // Descent 2 only
            GOAL_RED = 6, // Descent 2 only
            WATER = 7,
            LAVA = 8,
            TEAM_BLUE = 9,
            TEAM_RED = 10,
            SPEEDBOOST = 11,
            BLOCKED = 12,
            NODAMAGE = 13,
            SKYBOX = 14,
            EQUIPMAKER = 15, // matcen for powerups
            OUTDOORS = 16,
            COUNT_D2 = 17 // Descent 2 only
        }

        enum SegFunc : byte
        {
            NONE = 0,
            FUELCEN = 1,
            REPAIRCEN = 2,
            CONTROLCEN = 3,
            ROBOTMAKER = 4,
            GOAL_BLUE = 5,
            GOAL_RED = 6,
            TEAM_BLUE = 7,
            TEAM_RED = 8,
            SPEEDBOOST = 9,
            SKYBOX = 10,
            EQUIPMAKER = 11,
            COUNT = 12
        }

        enum SegProp : byte
        {
            NONE = 0,
            WATER = 1,
            LAVA = 2,
            BLOCKED = 4,
            NODAMAGE = 8,
            OUTDOORS = 16
        }

        enum WallType : byte
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

        enum WallFlag : ushort
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

        enum WallClipFlag : byte
        {
            EXPLODES = 1, //door explodes when opening
            BLASTABLE = 2, //this is a blastable wall
            TMAP1 = 4, //this uses primary tmap, not tmap2
            HIDDEN = 8 //this uses primary tmap, not tmap2
        }

        enum DoorState : byte
        {
            CLOSED = 0, // Door is closed 
            OPENING = 1, // Door is opening. 
            WAITING = 2, // Waiting to close 
            CLOSING = 3, // Door is closing 
            CLOAKING = 5, // Wall is going from closed -> open (Descent 2)
            DECLOAKING = 6	// Wall is going from open -> closed (Descent 2)
        }

        enum KeyType : byte
        {
            NONE = 1,
            BLUE = 2,
            RED = 4,
            GOLD = 8
        }

        enum WallVisType : byte
        {
            FLY = 1,
            RENDER = 2,
            RENDPAST = 4,
            EXTERNAL = 8,
            CLOAKED = 16 // Descent 2
        }

        enum WallRenderType : byte  // WALL_IS_DOORWAY return values F/R/RP 
        {
            SOLID = 2,	// 0/1/0 wall	 
            ILLUSORY = 3,	// 1/1/0 illusory wall 
            NONE = 5,	// 1/0/1 no wall, can fly through 
            TRANSPARENT = 6,	// 0/1/1 transparent wall 
            TRANSILLUSORY = 7,	// 1/1/1 transparent illusory wall 
            EXTERNAL = 8	// 0/0/0/1 don't see it, dont fly through it 
        }

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
    }
}
