
namespace DLE.NET
{
    public partial class Trigger
    {
        public const int MAX_TARGETS = 10;

        public class Target
        {
            short nSegment;
            short nSide;
        }

        byte    type;
        ushort  flags;
        short   nObject;
        char    num_links;
        int     value;
        int     time;
        Target[] targets = new Target[MAX_TARGETS];
        ushort  nIndex;
    }
}
