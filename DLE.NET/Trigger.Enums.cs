
namespace DLE.NET
{
    partial class Trigger
    {
        public enum Flags : ushort
        {
        CONTROL_DOORS = 1,	// Control Trigger 
        SHIELD_DAMAGE = 2,	// Shield Damage Trigger 
        ENERGY_DRAIN = 4,	// Energy Drain Trigger 
        EXIT = 8,	        // End of level Trigger 
        ON = 16,	        // Whether Trigger is active 
        ONE_SHOT = 32,	    // If Trigger can only be triggered once 
        MATCEN = 64,	    // Trigger for materialization centers 
        ILLUSION_OFF = 128,	// Switch Illusion OFF trigger 
        SECRET_EXIT = 256,	// Exit to secret level 
        ILLUSION_ON = 512,	// Switch Illusion ON trigger 
        OPEN_WALL = 1024,
        CLOSE_WALL = 2048,
        MAKE_ILLUSIONARY = 4096
        }

        public enum Types : byte
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

        public enum Properties : ushort
        {
            // Descent 1
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
            MAKE_ILLUSIONARY = 4096,

            // Descent 2
            NO_MESSAGE = 1, // Don't show a message when triggered
            SINGLE_USE = 2, // Only trigger once
            DISABLED = 4, // Set after one-shot fires
            PERMANENT = 8, // indestructable switch for repeated operation
            ALTERNATE = 16, // switch will assume the opposite function after operation
            SET_ORIENT = 32, // switch will assume the opposite function after operation
            SILENT = 64,
            AUTOPLAY = 128
        }
    }
}
4