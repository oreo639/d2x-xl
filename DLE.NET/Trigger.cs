
namespace DLE.NET
{
    public partial class Trigger
    {
        byte    type;
        ushort  flags;
        short   nObject;
        char    num_links;
        int     value;
        int     time;
        fixed short seg [GameMine.MAX_TRIGGER_TARGETS];
        fixed short side [GameMine.MAX_TRIGGER_TARGETS];
        ushort  nIndex;
    }
}
