using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class LightDeltaValue : SideKey, IGameItem
    {
        public int Key { get; set; }

        byte [] m_vertLight = new byte [4];

        //------------------------------------------------------------------------------

        void Read (BinaryReader fp, int nVersion, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        void Write (BinaryWriter fp, int nVersion, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        void Clear ()
        {
            for (int i = 0; i < 4; i++)
                m_vertLight [i] = 0;
        }

        //------------------------------------------------------------------------------

        byte this [int i]
        {
            get { return m_vertLight [i]; }
            set { m_vertLight [i] = value; }
        }

        //------------------------------------------------------------------------------

    }

    public class LightDeltaIndex : SideKey, IGameItem
    {
        public int Key { get; set; }

        ushort m_count = 0;
        ushort m_index = 0;

        //------------------------------------------------------------------------------

        void Read (BinaryReader fp, int nVersion, bool bD2X = false)
        {
        }

        //------------------------------------------------------------------------------

        void Write (BinaryWriter fp, int nVersion, bool bD2X = false)
        {
        }

        //------------------------------------------------------------------------------

        void Clear ()
        {
            m_count = m_index = 0;
        }

        //------------------------------------------------------------------------------

    }

    public class VariableLight : SideKey, IGameItem
    {
        public int Key { get; set; }

        uint m_mask;
        int m_timer;
        int m_delay;

        //------------------------------------------------------------------------------

        void Read (BinaryReader fp, int nVersion, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        void Write (BinaryWriter fp, int nVersion, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        void Clear ()
        {
            m_mask = 0;
            m_timer = m_delay = 0;
        }

        //------------------------------------------------------------------------------

        void Setup (SideKey key, short time, short mask)
        {
            m_mask = (uint)mask;
            m_timer = (int)time;
        }

        //------------------------------------------------------------------------------

        //------------------------------------------------------------------------------

        //------------------------------------------------------------------------------

    }
}
