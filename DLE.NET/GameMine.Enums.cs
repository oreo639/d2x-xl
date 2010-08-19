namespace DLE.NET
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
    }
}
