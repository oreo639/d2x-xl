using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public partial class Segment : GameItem
    {

        public const int MAX_SIDES_PER_SEGMENT = 6;
        public const int MAX_VERTICES_PER_SEGMENT = 8;

        public short [] m_verts = new short [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
        public byte m_function;			// special property of a segment (such as damaging, trigger, etc.) 
        public byte m_props;
        public sbyte m_nMatCen;			// which center segment is associated with, high bit set 
        public sbyte m_value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
        public byte m_s2Flags;			// New for Descent 2
        public short [] m_damage = new short [2];
        public int m_staticLight;		// average static light in segment 
        public byte m_childFlags;			// bit0 to 5: children, bit6: unused, bit7: special 
        public byte m_wallFlags;			// bit0 to 5: door/walls, bit6: deleted, bit7: marked segment 
        public short m_nIndex;				// used for cut & paste to help link children 
        public short m_mapBitmask;		    // which lines are drawn when displaying wireframe 
        public sbyte m_owner;
        public sbyte m_group;
        public Side [] m_sides = new Side [MAX_SIDES_PER_SEGMENT];

        //------------------------------------------------------------------------------

        public override void Read (BinaryReader fp, int version = 0, bool bFlag = false)
        {
            for (int i = 0; i < m_verts.Length; i++)
                m_verts [i] = fp.ReadInt16 ();
            m_function = fp.ReadByte ();
            m_props = fp.ReadByte ();
            m_nMatCen = fp.ReadSByte ();
            m_value = fp.ReadSByte ();
            m_damage [0] = fp.ReadInt16 ();
            m_damage [1] = fp.ReadInt16 ();
            m_staticLight = fp.ReadInt32 ();
            m_childFlags = fp.ReadByte ();
            m_wallFlags = fp.ReadByte ();
            m_mapBitmask = fp.ReadByte ();
            m_owner = fp.ReadSByte ();
            m_group = fp.ReadSByte ();
        }

        //------------------------------------------------------------------------------

        public override void Write (BinaryWriter fp, int version = 0, bool bFlag = false)
        {
            fp.Write (m_function);
            fp.Write (m_props);
            fp.Write (m_nMatCen);
            fp.Write (m_value);
        }

        //------------------------------------------------------------------------------

        public override void Clear ()
        {
            m_function = 0;
            m_props = 0;
            m_nMatCen = -1;
            m_value = 0;
            m_s2Flags = 0;
            m_damage [0] = m_damage [1] = 0;
            m_staticLight = 0;
            m_childFlags = 0;
            m_wallFlags = 0;
            m_nIndex = 0;
            m_mapBitmask = 0;
            m_owner = -1;
            m_group = -1;
            for (int i = 0; i < m_sides.Length; i++)
                m_sides [i].Clear ();
        }

        //------------------------------------------------------------------------------

        public short GetChild (short nSide)
        {
            return m_sides [nSide].m_nChild;
        }

        //------------------------------------------------------------------------------

        public short SetChild (short nSide, short nSegment)
        {
            m_sides [nSide].m_nChild = nSegment;
            if (nSegment == -1)
                m_childFlags &= (byte) ~(1 << nSide);
            else
                m_childFlags |= (byte) (1 << nSide);
            return nSegment;
        }

        //------------------------------------------------------------------------------

    }
}
