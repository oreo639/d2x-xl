using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public partial class WallManager
    {
        // ------------------------------------------------------------------------

        public MineItemInfo m_info;

        Wall [] m_walls = new Wall [GameMine.MAX_WALLS];

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

        public Wall [] Walls { get { return m_walls; } }

        // ------------------------------------------------------------------------

        public WallManager ()
        {
            Count = 0;
            for (int i = 0; i < GameMine.MAX_WALLS; i++)
                Walls [i] = new Wall (i);
        }

        // ------------------------------------------------------------------------

        public Wall this [int i]
        {
            get { return Walls [i]; }
            set { Walls [i] = value; }
        }

        // ------------------------------------------------------------------------

    }
}
