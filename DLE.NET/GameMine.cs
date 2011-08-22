
namespace DLE.NET
{
    public partial class GameMine
    {
        ushort m_nSegments;
        ushort m_nWalls;
        ushort m_nTriggers;

        public ushort SegCount 
        { 
            get { return m_nSegments; } 
            set { m_nSegments = value; }  
        }

        public Segment [] Segments = new Segment [SEGMENT_LIMIT];
        public Wall [] Walls = new Wall [WALL_LIMIT];
        public Trigger [] Triggers = new Trigger [TRIGGER_LIMIT];
        public Vertex [] Vertices = new Vertex [VERTEX_LIMIT];

        public MineInfo Info = new MineInfo ();

        //------------------------------------------------------------------------------

        public int LevelVersion { get; set; }
        public GameFileType FileType { get; set; }

        //------------------------------------------------------------------------------

        public GameMine ()
        {
            LevelVersion = 7;
        }
        
        //------------------------------------------------------------------------------

        public static short Wrap (short value, short delta, short min, short max) 
        {
        value += delta;
        if (value > max)
            return min;
        else if (value < min)
            return max;
        else
            return value;
        }

        //------------------------------------------------------------------------------

        //------------------------------------------------------------------------------

        //------------------------------------------------------------------------------

    }
}
