namespace DLE.NET
{
    public partial class GameMine
    {
        public const int F0_0 = 0;
        public const int F1_0 = 0x10000;
        public const int F2_0 = 0x20000;
        public const int F3_0 = 0x30000;
        public const int F10_0 = 0xa0000;
        public const int F0_1 = 0x199a;
        public const int F0_5 = 0x8000;

        // physics constants
        public const int HOSTAGE_SIZE = 0x50000;
        public const int PLAYER_SIZE = 0x46c35;
        public const int REACTOR_SIZE = 0xC14ED;
        public const int WEAPON_SIZE = 148158;

        public const int WEAPON_SHIELD = 20 * F1_0;
        public const int DEFAULT_SHIELD = 0x640000;
        public const int REACTOR_SHIELD = 0xC82000;

        public const int DEFAULT_LIGHTING = F0_5;      // (F1_0/2) 
        public const int TRIGGER_DEFAULT = 2 * F1_0;
        public const int WALL_HPS = 100 * F1_0; // Normal wall's hp 
        public const int WALL_DOOR_INTERVAL = 5 * F1_0;	 // How many seconds a door is open 

        // level database constants
        public const int N_ROBOT_TYPES_D2 = 66;
        public const int N_ROBOT_JOINTS_D2 = 1145;
        public const int N_POLYGON_MODELS_D2 = 166;
        public const int N_OBJBITMAPS_D2 = 422;
        public const int N_OBJBITMAPPTRS_D2 = 502;

        public const int MAX_ROBOT_TYPES = 85;
        public const int MAX_ROBOT_JOINTS = 1250;
        public const int MAX_POLYGON_MODELS = 200;
        public const int MAX_OBJ_BITMAPS = 600;
        public const int MAX_WEAPON_TYPES = 65;

        public const int MAX_SEGMENTS_D1 = 800;  // descent 1 max # of cubes
        public const int MAX_SEGMENTS_D2 = 900;  // descent 2 max # of cubes
        public const int SEGMENT_LIMIT = 8000; // D2X-XL max # of cubes

        public const int MAX_VERTICES_D1 = 2808; // descent 1 max # of vertices
        public const int MAX_VERTICES_D2 = (MAX_SEGMENTS_D2 * 4 + 8); // descent 2 max # of vertices
        public const int VERTEX_LIMIT = (SEGMENT_LIMIT * 4 + 8); // descent 2 max # of vertices

        public const int MAX_OBJECTS1 = 350;
        public const int MAX_OBJECTS2 = 2000;

        public const int MAX_WALLS_D1 = 175; // Maximum number of walls for Descent 1
        public const int MAX_WALLS_D2 = 255; // Maximum number of walls for Descent 2
        public const int WALL_LIMIT = 2047; // Maximum number of walls for Descent 2

        public const int MAX_TRIGGERS_D1 = 100;
        public const int MAX_TRIGGERS_D2 = 254;
        public const int TRIGGER_LIMIT = MAX_TRIGGERS_D2;

        public const int MAX_OBJ_TRIGGERS = 254;
        public const int MAX_REACTOR_TRIGGERS = 10;
        public const int MAX_TRIGGER_FLAGS = 12;
        public const int NO_TRIGGER = 255;

        public const int MAX_PLAYERS_D2 = 8;
        public const int MAX_PLAYERS_D2X = 16;
        public const int MAX_COOP_PLAYERS = 3;

        public const int MAX_DOORS = 1;
        public const int MAX_CONTROL_CENTER_TRIGGERS = 10;
        public const int MAX_NUM_MATCENS_D1 = 20;
        public const int MAX_NUM_MATCENS_D2 = 100;
        public const int MAX_NUM_RECHARGERS_D2 = 70;
        public const int MAX_NUM_RECHARGERS_D2X = 500;
        public const int MAX_WALL_SWITCHES = 50;
        public const int MAX_WALL_LINKS = 100;
        public const int MAX_NUM_FUELCENS_D2 = 70;
        public const int MAX_NUM_REPAIRCENS_D2 = 70;
        public const int MAX_NUM_FUELCENS_D2X = 500;
        public const int MAX_NUM_REPAIRCENS_D2X = 500;
        public const int MAX_WALL_ANIMS_D1 = 30;  // Maximum different types of doors Descent 1
        public const int MAX_WALL_ANIMS_D2 = 60;  // Maximum different types of doors Descent 2
        public const int MAX_CLIP_FRAMES_D1 = 20; // Descent 1
        public const int MAX_CLIP_FRAMES_D2 = 50; // Descent 2
        public const int MAX_STUCK_OBJECTS = 32;
        public const int MAX_AI_FLAGS = 11; // This MUST cause word (4 bytes) alignment in ai_static, allowing for one byte mode 
        public const int MAX_SUBMODELS = 10; // I guessed at this value (BAA) 

        public const int N_WALL_TEXTURES_D1 = 26;
        public const int N_WALL_TEXTURES_D2 = 51;
        public const int NUM_OF_CLIPS_D1 = 24;
        public const int NUM_OF_CLIPS_D2 = 49;

        public const int VCLIP_MAX_FRAMES = 30;
        public const int MAX_GUNS = 8;  //should be multiple of 4 for ubyte array
        public const int NDL = 5;  // Guessed at this value (B. Aamot 9/14/96)
        public const int N_ANIM_STATES = 5;

        public const int MAX_AI_OPTIONS_D1 = 6;
        public const int MAX_AI_OPTIONS_D2 = 9;

        public const int MAX_MACROS = 100;

        public const int MAX_WALL_FLAGS_D1 = 5;
        public const int MAX_WALL_FLAGS_D2 = 9;
        public const int WALL_FLAG_LIMIT = 9;

        public const int ASPECT_TOP = 3;
        public const int ASPECT_BOT = 5;
        public const double ANGLE_RATE = 0.01f;
        public const double MOVE_RATE = 5;

        public const int DEFAULT_SEGMENT = 0;
        public const int DEFAULT_SIDE = 4;
        public const int DEFAULT_LINE = 0; // line of the current side (0..3) 
        public const int DEFAULT_POINT = 0; // point of the current side (0..3) 
        public const int DEFAULT_OBJECT = 0;

        public const int MAX_POLY = 6;

        public const int MAX_OBJECT_NUMBER = 12;
        public const int MAX_CONTAINS_NUMBER = 2;
        public const int HOSTAGE_CLIP_NUMBER = 33;
        public const int VCLIP_BIG_EXPLOSION = 0;
        public const int VCLIP_SMALL_EXPLOSION = 2;

        public const int PLAYER_CLIP_NUMBER_D1 = 43;
        public const int PLAYER_CLIP_NUMBER_D2 = 108;

        public const int COOP_CLIP_NUMBER_D1 = 44;
        public const int COOP_CLIP_NUMBER_D2 = 108;

        public const int REACTOR_CLIP_NUMBER_D1 = 39;
        public const int REACTOR_CLIP_NUMBER_D2 = 97;

        public const int COMPILED_MINE_VERSION = 0;

        public const int ROBOT_IDS1 = 24;
        public const int MAX_ROBOT_IDS_TOTAL = 78;
        public const int MAX_POWERUP_IDS1 = 26;
        public const int MAX_POWERUP_IDS2 = 50;
        public const int MAX_POWERUP_IDS_D2 = 48;

        public const int MAX_DL_INDICES_D2 = 500;
        public const int MAX_DELTA_LIGHTS_D2 = 10000;
        public const int MAX_DL_INDICES_D2X = 3000;
        public const int MAX_DELTA_LIGHTS_D2X = 50000;
        public const int DL_SCALE = 2048;	// Divide light to allow 3 bits integer, 5 bits fraction.

        public const int MAX_BRIGHTNESS = 0x20000;

        public const byte MARKED_MASK = 0x80; // used on wallFlags & vertexStatus 
        public const byte DELETED_MASK = 0x40; // used on wallFlags & vertexStatus 
        public const byte NEW_MASK = 0x20; // used on vertexStatus      
        public const byte SHARED_MASK = 0x10;
        
        // pseudo constants
        public static ushort MAX_SEGMENTS { get {return (ushort) (DLE.IsD1File ? MAX_SEGMENTS_D1 : DLE.IsStdLevel ? MAX_SEGMENTS_D2 : SEGMENT_LIMIT); } }
        public static ushort MAX_VERTICES { get { return (ushort)(DLE.IsD1File ? MAX_VERTICES_D1 : DLE.IsStdLevel ? MAX_VERTICES_D2 : VERTEX_LIMIT); } }
        public static ushort MAX_OBJECTS { get { return (ushort)(DLE.IsStdLevel ? MAX_OBJECTS1 : MAX_OBJECTS2); } }
        public static ushort MAX_WALLS { get { return (ushort)(DLE.IsD1File ? MAX_WALLS_D1 : (DLE.LevelVersion < 12) ? MAX_WALLS_D2 : WALL_LIMIT); } }
        public static ushort NO_WALL { get { return MAX_WALLS; } }
        public static ushort MAX_TRIGGERS { get { return (ushort)((DLE.IsD1File || (DLE.LevelVersion < 12)) ? MAX_TRIGGERS_D1 : MAX_TRIGGERS_D2); } }
        public static ushort MAX_PLAYERS { get { return (ushort)(DLE.IsStdLevel ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X); } }
        public static ushort MAX_MATCENS { get { return (ushort)((DLE.IsD1File || (DLE.LevelVersion < 12)) ? MAX_NUM_MATCENS_D1 : MAX_NUM_MATCENS_D2); } }
        public static ushort MAX_FUELCENS { get { return (ushort)((DLE.IsD1File || (DLE.LevelVersion < 12)) ? MAX_NUM_FUELCENS_D2 : MAX_NUM_FUELCENS_D2X); } }
        public static ushort MAX_REPAIRCENS { get { return (ushort)((DLE.IsD1File || (DLE.LevelVersion < 12)) ? MAX_NUM_REPAIRCENS_D2 : MAX_NUM_REPAIRCENS_D2X); } }
        public static ushort MAX_NUM_RECHARGERS { get { return (ushort) ((DLE.theMine == null) ? MAX_NUM_RECHARGERS_D2X : (DLE.IsD1File || (DLE.LevelVersion < 12)) ? MAX_NUM_RECHARGERS_D2 : MAX_NUM_RECHARGERS_D2X); } }

        public static ushort PLAYER_CLIP_NUMBER { get { return (ushort)(DLE.IsD1File ? PLAYER_CLIP_NUMBER_D1 : PLAYER_CLIP_NUMBER_D2); } }
        public static ushort COOP_CLIP_NUMBER { get { return (ushort)(DLE.IsD1File ? COOP_CLIP_NUMBER_D1 : COOP_CLIP_NUMBER_D2); } }
        public static ushort REACTOR_CLIP_NUMBER { get { return (ushort)(DLE.IsD1File ? REACTOR_CLIP_NUMBER_D1 : REACTOR_CLIP_NUMBER_D2); } }
        public static ushort ROBOT_IDS2 { get { return (ushort)((DLE.LevelVersion == 7) ? N_ROBOT_TYPES_D2 : MAX_ROBOT_IDS_TOTAL); } }
        public static ushort MAX_POWERUP_IDS { get { return (ushort)(DLE.IsD1File ? MAX_POWERUP_IDS1 : MAX_POWERUP_IDS2); } }
        public static ushort MAX_DL_INDICES { get { return (ushort)((DLE.IsD1File || DLE.IsStdLevel) ? MAX_DL_INDICES_D2 : MAX_DL_INDICES_D2X); } }
        public static ushort MAX_DELTA_LIGHTS { get { return (ushort)((DLE.IsD1File || DLE.IsStdLevel) ? MAX_DELTA_LIGHTS_D2 : MAX_DELTA_LIGHTS_D2X); } }
    }
}
