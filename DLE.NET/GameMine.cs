
namespace DLE.NET
{
    public partial class GameMine
    {
        public Segment [] segments = new Segment [SEGMENT_LIMIT];
        public Wall [] walls = new Wall [WALL_LIMIT];

        //------------------------------------------------------------------------------

        public int LevelVersion { get; set; }
        public GameFileType FileType { get; set; }

        public bool IsD1File () { return FileType == GameFileType.RDL; }
        public bool IsD2File () { return FileType != GameFileType.RDL; }
        public bool IsStdLevel () { return LevelVersion < 9; }
        public bool IsD2XLevel () { return LevelVersion >= 9; }

        //------------------------------------------------------------------------------

    }
}
