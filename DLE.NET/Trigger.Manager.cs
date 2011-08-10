using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public partial class TriggerManager
    {
        // ------------------------------------------------------------------------

        public MineItemInfo m_info = new MineItemInfo ();

        Trigger [] m_triggers = new Trigger [GameMine.MAX_TRIGGERS];

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

        public Trigger [] Triggers { get { return m_triggers; } }

        // ------------------------------------------------------------------------

        public TriggerManager ()
        {
            Count = 0;
            for (int i = 0; i < GameMine.MAX_TRIGGERS; i++)
                Triggers [i] = new Trigger (i);
        }

        // ------------------------------------------------------------------------

        public Trigger this [int i]
        {
            get 
            { 
                return (i < 0) ? null : Triggers [i]; 
            }
            set 
            { 
                Triggers [i] = value; 
            }
        }

        // ------------------------------------------------------------------------

    }
}
