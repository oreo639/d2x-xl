
namespace DLE.NET
{
    #region TargetList
    public class TargetList
    {
        public const int MAX_TARGETS = 10;

        short m_count { get; private set; }
        SideKey [] m_targets = new SideKey [MAX_TARGETS];

        public short Add (SideKey key)
        {
            if (m_count < m_targets.Length)
                m_targets [m_count] = key;
            return m_count++;
        }

        public short Add (short nSegment, short nSide)
        {
            return Add (new SideKey (nSegment, nSide));
        }

        short Delete (int i = -1)
        {
            if (i < 0)
                i = m_count - 1;
            if ((m_count > 0) && (i < --m_count))
            {
                int l = m_count - i;
                for (int j = 0; j < l; j++)
                    m_targets [j] = m_targets [j + 1];
                m_targets [m_count] = new SideKey (0, 0);
            }
            return m_count;
        }
    }
    #endregion

    #region Trigger
    public partial class Trigger : TargetList
    {
        byte type;
        ushort flags;
        short nObject;
        char num_links;
        int value;
        int time;
        ushort nIndex;
    }
    #endregion
}
