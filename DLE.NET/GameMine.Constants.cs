namespace DLE.NET
{
  public partial class GameMine
    {
        const int F0_0 = 0;
        const int F1_0 = 0x10000;
        const int F2_0 = 0x20000;
        const int F3_0 = 0x30000;
        const int F10_0 = 0xa0000;
        const int F0_1 = 0x199a;
        const int F0_5 = 0x8000;

        // physics constants
        const int HOSTAGE_SIZE = 0x50000;
        const int PLAYER_SIZE = 0x46c35;
        const int REACTOR_SIZE = 0xC14ED;
        const int WEAPON_SIZE = 148158;

        const int WEAPON_SHIELD = 20 * F1_0;
        const int DEFAULT_SHIELD = 0x640000;
        const int REACTOR_SHIELD = 0xC82000;

        const int DEFAULT_LIGHTING = F0_5;      // (F1_0/2) 
        const int TRIGGER_DEFAULT = 2 * F1_0;
        const int WALL_HPS = 100 * F1_0; // Normal wall's hp 
        const int WALL_DOOR_INTERVAL = 5 * F1_0;	 // How many seconds a door is open 

        // level database constants
        const int N_D2_ROBOT_TYPES = 66;
        const int N_D2_ROBOT_JOINTS = 1145;
        const int N_D2_POLYGON_MODELS = 166;
        const int N_D2_OBJBITMAPS = 422;
        const int N_D2_OBJBITMAPPTRS = 502;

        const int MAX_ROBOT_TYPES = 85;
        const int MAX_ROBOT_JOINTS = 1250;
        const int MAX_POLYGON_MODELS = 200;
        const int MAX_OBJ_BITMAPS = 600;
        const int MAX_WEAPON_TYPES = 65;

        const int MAX_SEGMENTS1 = 800;  // descent 1 max # of cubes
        const int MAX_SEGMENTS2 = 900;  // descent 2 max # of cubes
        const int MAX_SEGMENTS3 = 8000; // D2X-XL max # of cubes

        const int MAX_VERTICES1 = 2808; // descent 1 max # of vertices
        const int MAX_VERTICES2 = (MAX_SEGMENTS2 * 4 + 8); // descent 2 max # of vertices
        const int MAX_VERTICES3 = (MAX_SEGMENTS3 * 4 + 8); // descent 2 max # of vertices

        const int MAX_OBJECTS1 = 350;
        const int MAX_OBJECTS2 = 2000;

        const int MAX_WALLS1 = 175; // Maximum number of walls for Descent 1
        const int MAX_WALLS2 = 255; // Maximum number of walls for Descent 2
        const int MAX_WALLS3 = 2047; // Maximum number of walls for Descent 2

        const int MAX_TRIGGERS1 = 100;
        const int MAX_TRIGGERS2 = 254;

        const int MAX_OBJ_TRIGGERS = 254;
        const int MAX_TRIGGER_FLAGS = 12;
        const int NO_TRIGGER = 255;

        const int MAX_PLAYERS_D2 = 8;
        const int MAX_PLAYERS_D2X = 16;
        const int MAX_COOP_PLAYERS = 3;

        const int MAX_DOORS = 1;
        const int MAX_CONTROL_CENTER_TRIGGERS = 10;
        const int MAX_NUM_MATCENS1 = 20;
        const int MAX_NUM_MATCENS2 = 100;
        const int MAX_WALL_SWITCHES = 50;
        const int MAX_WALL_LINKS = 100;
        const int MAX_NUM_FUELCENS1 = 70;
        const int MAX_NUM_REPAIRCENS1 = 70;
        const int MAX_NUM_FUELCENS2 = 500;
        const int MAX_NUM_REPAIRCENS2 = 500;
        const int MAX_WALL_ANIMS1 = 30;  // Maximum different types of doors Descent 1
        const int MAX_WALL_ANIMS2 = 60;  // Maximum different types of doors Descent 2
        const int MAX_CLIP_FRAMES1 = 20; // Descent 1
        const int MAX_CLIP_FRAMES2 = 50; // Descent 2
        const int MAX_STUCK_OBJECTS = 32;
        const int MAX_SIDES_PER_SEGMENT = 6;
        const int MAX_VERTICES_PER_SEGMENT = 8;
        const int MAX_AI_FLAGS = 11; // This MUST cause word (4 bytes) alignment in ai_static, allowing for one byte mode 
        const int MAX_SUBMODELS = 10; // I guessed at this value (BAA) 

        const int VCLIP_MAX_FRAMES = 30;
        const int MAX_GUNS = 8;  //should be multiple of 4 for ubyte array
        const int NDL = 5;  // Guessed at this value (B. Aamot 9/14/96)
        const int N_ANIM_STATES = 5;

        const int MAX_D1_AI_OPTIONS = 6;
        const int MAX_D2_AI_OPTIONS = 9;

        const int MAX_MACROS = 100;

        const int MAX_D1_TEXTURES = 584;
        const int MAX_D2_TEXTURES = 910;

        const int MAX_D1_WALL_FLAGS = 5;
        const int MAX_D2_WALL_FLAGS = 9;
        const int MAX_WALL_FLAGS = 9;
        const int NUM_LIGHTS_D1 = 48;
        const int NUM_LIGHTS_D2 = 85;

        const int ASPECT_TOP = 3;
        const int ASPECT_BOT = 5;
        const float ANGLE_RATE = 0.01f;
        const int MOVE_RATE = 5;

        const int DEFAULT_SEGMENT = 0;
        const int DEFAULT_SIDE = 4;
        const int DEFAULT_LINE = 0; // line of the current side (0..3) 
        const int DEFAULT_POINT = 0; // point of the current side (0..3) 
        const int DEFAULT_OBJECT = 0;

        const int MAX_POLY = 6;

        const int MAX_OBJECT_NUMBER = 12;
        const int MAX_CONTAINS_NUMBER = 2;
        const int HOSTAGE_CLIP_NUMBER = 33;
        const int VCLIP_BIG_EXPLOSION = 0;
        const int VCLIP_SMALL_EXPLOSION = 2;

        const int D1_PLAYER_CLIP_NUMBER = 43;
        const int D2_PLAYER_CLIP_NUMBER = 108;

        const int D1_COOP_CLIP_NUMBER = 44;
        const int D2_COOP_CLIP_NUMBER = 108;

        const int D1_REACTOR_CLIP_NUMBER = 39;
        const int D2_REACTOR_CLIP_NUMBER = 97;

        const int COMPILED_MINE_VERSION = 0;

        const int ROBOT_IDS1 = 24;
        const int MAX_ROBOT_IDS_TOTAL = 78;
        const int MAX_POWERUP_IDS1 = 26;
        const int MAX_POWERUP_IDS2 = 50;
        const int MAX_POWERUP_IDS_D2 = 48;
        const int MAX_TRIGGER_TARGETS = 10;

        const int MAX_DL_INDICES_D2 = 500;
        const int MAX_DELTA_LIGHTS_D2 = 10000;
        const int MAX_DL_INDICES_D2X = 3000;
        const int MAX_DELTA_LIGHTS_D2X = 50000;
        const int DL_SCALE = 2048;	// Divide light to allow 3 bits integer, 5 bits fraction.

        // pseudo constants
        public int MAX_SEGMENTS() { return IsD1File() ? MAX_SEGMENTS1 : IsStdLevel() ? MAX_SEGMENTS2 : MAX_SEGMENTS3; }
        public int MAX_VERTICES() { return IsD1File() ? MAX_VERTICES1 : IsStdLevel() ? MAX_VERTICES2 : MAX_VERTICES3; }
        public int MAX_OBJECTS() { return IsStdLevel() ? MAX_OBJECTS1 : MAX_OBJECTS2; }
        public int MAX_WALLS() { return IsD1File() ? MAX_WALLS1 : (LevelVersion < 12) ? MAX_WALLS2 : MAX_WALLS3; }
        public int NO_WALL() { return MAX_WALLS(); }
        public int MAX_TRIGGERS() { return (IsD1File() || (LevelVersion < 12)) ? MAX_TRIGGERS1 : MAX_TRIGGERS2; }
        public int MAX_PLAYERS() { return IsStdLevel() ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X; }
        public int MAX_NUM_MATCENS() { return (IsD1File() || (LevelVersion < 12)) ? MAX_NUM_MATCENS1 : MAX_NUM_MATCENS2; }
        public int MAX_NUM_FUELCENS() { return (IsD1File() || (LevelVersion < 12)) ? MAX_NUM_FUELCENS1 : MAX_NUM_FUELCENS2; }
        public int MAX_NUM_REPAIRCENS() { return (IsD1File() || (LevelVersion < 12)) ? MAX_NUM_REPAIRCENS1 : MAX_NUM_REPAIRCENS2; }
        public int MAX_TEXTURES() { return IsD1File() ? MAX_D1_TEXTURES : MAX_D2_TEXTURES; }
        public int PLAYER_CLIP_NUMBER() { return IsD1File() ? D1_PLAYER_CLIP_NUMBER : D2_PLAYER_CLIP_NUMBER; }
        public int COOP_CLIP_NUMBER() { return IsD1File() ? D1_COOP_CLIP_NUMBER : D2_COOP_CLIP_NUMBER; }
        public int REACTOR_CLIP_NUMBER() { return IsD1File() ? D1_REACTOR_CLIP_NUMBER : D2_REACTOR_CLIP_NUMBER; }
        public int ROBOT_IDS2() { return (LevelVersion == 7) ? N_D2_ROBOT_TYPES : MAX_ROBOT_IDS_TOTAL; }
        public int MAX_POWERUP_IDS() { return IsD1File() ? MAX_POWERUP_IDS1 : MAX_POWERUP_IDS2; }
        public int MAX_DL_INDICES () { return (IsD1File () || IsStdLevel ()) ? MAX_DL_INDICES_D2 : MAX_DL_INDICES_D2X; }
        public int MAX_DELTA_LIGHTS() { return (IsD1File() || IsStdLevel()) ? MAX_DELTA_LIGHTS_D2 : MAX_DELTA_LIGHTS_D2X; }
    }
}
