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

        const int MAX_PLAYERS_D2 = 8;
        const int MAX_PLAYERS_D2X = 16;
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
        const int MAX_D1_WALL_FLAGS = 5;
        const int MAX_D2_WALL_FLAGS = 9;
        const int MAX_WALL_FLAGS = 9;
        const int NUM_LIGHTS_D1 = 48;
        const int NUM_LIGHTS_D2 = 85;

        const int ASPECT_TOP = 3;
        const int ASPECT_BOT = 5;
        const float ANGLE_RATE = 0.01f;
        const int MOVE_RATE = 5;

        const int MAX_POLY = 6;

        public int MAX_PLAYERS()
        {
            return (m_levelVersion < 9) ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X;
        }
    }
}
