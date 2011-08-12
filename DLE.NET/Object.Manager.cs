using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLEdotNET
{
    public partial class ObjectManager
    {
        // ------------------------------------------------------------------------

        public MineItemInfo m_info = new MineItemInfo ();

        GameObject [] m_objects = new GameObject [GameMine.MAX_OBJECTS];

        // ------------------------------------------------------------------------

        public int Count
        {
            get { return m_info.count; }
            set { m_info.count = value; }
        }

        public int FileOffset
        {
            get { return m_info.offset; }
            set { m_info.offset = value; }
        }

        // ------------------------------------------------------------------------

        public GameObject [] Objects { get { return m_objects; } }

        // ------------------------------------------------------------------------

        public ObjectManager ()
        {
            Count = 0;
            for (int i = 0; i < GameMine.MAX_OBJECTS; i++)
                Objects [i] = new GameObject (i);
        }

        // ------------------------------------------------------------------------

        public GameObject this [int i]
        {
            get { return (i < 0) ? null : Objects [i]; }
            set { Objects [i] = value; }
        }

        // ------------------------------------------------------------------------

        public void UpdateSegments (short nOldSeg, short nNewSeg)
        {
        DLE.Backup.Begin (UndoData.Flags.udObjects);
        for (int i = 0; i < Count; i++)
	        if (Objects [i].m_nSegment == nOldSeg)
		        Objects [i].m_nSegment = nNewSeg;
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

    }
}
