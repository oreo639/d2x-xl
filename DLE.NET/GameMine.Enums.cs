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

        enum MarkModes : uint
        {
            NEW_MASK = 0x20, // used on vert_status                
            DELETED_MASK = 0x40, // used on wall_bitmask & vert_status 
            MARKED_MASK = 0x80 // used on wall_bitmask & vert_status 
        }

        enum FileType : uint
        {
            RDL = 0,
            RL2 = 1
        }

        enum AIBehavior : uint
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
    }
}
