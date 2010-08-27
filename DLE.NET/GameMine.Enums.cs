namespace DLE.NET
{
    public partial class GameMine
    {
        public enum MacroModes : uint
        {
            MACRO_OFF = 0,
            MACRO_RECORD = 1,
            MACRO_PLAY = 2
        }

        public enum SelectModes : uint
        {
            POINT_MODE = 0,
            LINE_MODE = 1,
            SIDE_MODE = 2,
            CUBE_MODE = 3,
            OBJEMODE = 4,
            BLOCK_MODE = 5,
            N_SELEMODES = 6
        }

        public enum EditModes : uint
        {
            EDIT_OFF = 0,
            EDIT_MOVE = 1,
            N_EDIT_MODES = 2
        }

        public enum MarkModes : byte
        {
            NEW_MASK = 0x20, // used on vert_status                
            DELETED_MASK = 0x40, // used on wall_bitmask & vert_status 
            MARKED_MASK = 0x80 // used on wall_bitmask & vert_status 
        }

        public enum ShowMode : uint
        {
            SHOW_LINES_POINTS = 1,
            SHOW_LINES_PARTIAL = 2,
            SHOW_LINES_ALL = 4,
            SHOW_LINES_NEARBY = 8,
            SHOW_FILLED_POLYGONS = 16
        }

        public enum GameFileType : uint
        {
            RDL = 0,
            RL2 = 1
        }

        public enum WallSide : byte
        {
            LEFT = 0,
            TOP = 1,
            RIGHT = 2,
            BOTTOM = 3,
            BACK = 4,
            FRONT = 5

        }

        public enum PowerupMask : byte
        {
            POWERUP_WEAPON_MASK = 1,
            POWERUP_KEY_MASK = 2,
            POWERUP_POWERUP_MASK = 4,
            POWERUP_UNKNOWN_MASK = 255 // show the type if any other mask is on
        }

        public enum AIBehavior : byte
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

        public enum SegInsMode : uint
        {
            ORTHOGONAL = 0,
            EXTEND = 1,
            MIRROR = 2,
            N_CUBE_MODES = 3
        }


        public enum TextureFlag : int
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
