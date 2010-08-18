namespace DLE.NET
{
    public partial class GameMine
    {
        enum MacroModes : uint
        {
            MACRO_OFF      = 0,
            MACRO_RECORD   = 1,
            MACRO_PLAY     = 2
        }

        enum SelectModes : uint
        {
            POINT_MODE     = 0,
            LINE_MODE      = 1,
            SIDE_MODE      = 2,
            CUBE_MODE      = 3,
            OBJECT_MODE    = 4,
            BLOCK_MODE     = 5,
            N_SELECT_MODES = 6
        }
    }
}
