
namespace DLE.NET
{
    public partial class GameMine
    {
        public Segment [] segments = new Segment [SEGMENT_LIMIT];
        public Wall [] walls = new Wall [WALL_LIMIT];

        //------------------------------------------------------------------------------

        public int LevelVersion { get; set; }
        public GameFileType FileType { get; set; }

        //------------------------------------------------------------------------------

    }
}
