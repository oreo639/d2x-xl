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

        public void Read (BinaryReader fp, int nVersion, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int nVersion, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        public new void Clear ()
        {
            for (int i = 0; i < 4; i++)
                m_vertLight [i] = 0;
        }

        //------------------------------------------------------------------------------

        public byte this [int i]
        {
            get { return m_vertLight [i]; }
            set { m_vertLight [i] = value; }
        }

        //------------------------------------------------------------------------------

    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class LightDeltaIndex : SideKey, IGameItem
    {
        public int Key { get; set; }

        ushort m_count = 0;
        ushort m_index = 0;

        //------------------------------------------------------------------------------

        public void Read (BinaryReader fp, int nVersion, bool bD2X = false)
        {
        }

        //------------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int nVersion, bool bD2X = false)
        {
        }

        //------------------------------------------------------------------------------

        new public void Clear ()
        {
            m_count = m_index = 0;
        }

        //------------------------------------------------------------------------------

    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class VariableLight : SideKey, IGameItem, IComparable<VariableLight>
    {
        public int Key { get; set; }

        uint m_mask;
        int m_timer;
        int m_delay;

        //------------------------------------------------------------------------------

        public void Read (BinaryReader fp, int nVersion, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int nVersion, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        public new void Clear ()
        {
            m_mask = 0;
            m_timer = m_delay = 0;
        }

        //------------------------------------------------------------------------------

        public void Setup (SideKey key, int time, uint mask)
        {
            m_mask = mask;
            m_timer = time;
        }

        //------------------------------------------------------------------------

        public int CompareTo (object obj)
        {
            VariableLight other = obj as VariableLight;
            int i = this.Key;
            int m = other.Key;

            return (i < m) ? -1 : (i > m) ? 1 : 0;
        }

        //------------------------------------------------------------------------------

    }
}
