namespace DLE.NET
{
  public partial class GameMine
    {
        const uint F0_0 = 0;
        const uint F1_0 = 0x10000;
        const uint F2_0 = 0x20000;
        const uint F3_0 = 0x30000;
        const uint F10_0 = 0xa0000;
        const uint F0_1 = 0x199a;
        const uint F0_5 = 0x8000;

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
        public int MAX_SEGMENTS() { return (m_fileType == FileType.RDL) ? MAX_SEGMENTS1 : (m_levelVersion < 9) ? MAX_SEGMENTS2 : MAX_SEGMENTS3; }

        const int MAX_VERTICES1 = 2808; // descent 1 max # of vertices
        const int MAX_VERTICES2 = (MAX_SEGMENTS2 * 4 + 8); // descent 2 max # of vertices
        const int MAX_VERTICES3 = (MAX_SEGMENTS3 * 4 + 8); // descent 2 max # of vertices
        public int MAX_VERTICES() { return (m_fileType == FileType.RDL) ? MAX_VERTICES1 : (m_levelVersion < 9) ? MAX_VERTICES2 : MAX_VERTICES3; }

        const int MAX_OBJECTS1 = 350;
        const int MAX_OBJECTS2 = 2000;
        public int MAX_OBJECTS() { return (m_levelVersion < 9) ? MAX_OBJECTS1 : MAX_OBJECTS2; }

        const int MAX_WALLS1 = 175; // Maximum number of walls for Descent 1
        const int MAX_WALLS2 = 255; // Maximum number of walls for Descent 2
        const int MAX_WALLS3 = 2047; // Maximum number of walls for Descent 2
        public int MAX_WALLS() { return (m_fileType == FileType.RDL) ? MAX_WALLS1 : (m_levelVersion < 12) ? MAX_WALLS2 : MAX_WALLS3; }
        public int NO_WALL() { return MAX_WALLS(); }

        const int MAX_TRIGGERS1 = 100;
        const int MAX_TRIGGERS2 = 254;
        public int MAX_TRIGGERS() { return ((m_fileType == FileType.RDL) || (m_levelVersion < 12)) ? MAX_TRIGGERS1 : MAX_TRIGGERS2; }

        const int MAX_OBJ_TRIGGERS = 254;
        const int MAX_TRIGGER_FLAGS = 12;
        const int NO_TRIGGER = 255;

        const int MAX_PLAYERS_D2 = 8;
        const int MAX_PLAYERS_D2X = 16;
        public int MAX_PLAYERS() { return (m_levelVersion < 9) ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X; }
        const int MAX_COOP_PLAYERS = 3;

        const int VCLIP_MAX_FRAMES = 30;
        const int MAX_GUNS = 8;  //should be multiple of 4 for ubyte array
        const int NDL = 5;  // Guessed at this value (B. Aamot 9/14/96)
        const int N_ANIM_STATES = 5;

        const int HOSTAGE_SIZE = 0x50000;
        const int PLAYER_SIZE = 0x46c35;
        const int REACTOR_SIZE = 0xC14ED;
        const int WEAPON_SIZE = 148158;

        const int WEAPON_SHIELD = 0x140000;
        const int DEFAULT_SHIELD = 0x640000;
        const int REACTOR_SHIELD = 0xC82000;

        const int MAX_D1_AI_OPTIONS = 6;
        const int MAX_D2_AI_OPTIONS = 9;

        const int MAX_MACROS = 100;

        const int MAX_D1_TEXTURES = 584;
        const int MAX_D2_TEXTURES = 910;
        public int MAX_TEXTURES() { return (m_fileType == FileType.RDL) ? MAX_D1_TEXTURES : MAX_D2_TEXTURES; }

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
        public int PLAYER_CLIP_NUMBER () { return (m_fileType == FileType.RDL) ? D1_PLAYER_CLIP_NUMBER : D2_PLAYER_CLIP_NUMBER; }

        const int D1_COOP_CLIP_NUMBER = 44;
        const int D2_COOP_CLIP_NUMBER = 108;
        public int COOP_CLIP_NUMBER () { return (m_fileType == FileType.RDL) ? D1_COOP_CLIP_NUMBER : D2_COOP_CLIP_NUMBER; }

        const int D1_REACTOR_CLIP_NUMBER = 39;
        const int D2_REACTOR_CLIP_NUMBER = 97;
        public int REACTOR_CLIP_NUMBER() { return (m_fileType == FileType.RDL) ? D1_REACTOR_CLIP_NUMBER : D2_REACTOR_CLIP_NUMBER; }


    }
}
