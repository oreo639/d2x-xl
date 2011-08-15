using System.IO;
using System;

namespace DLE.NET
{
    public class MatCenter : IGameItem, IComparable
    {
        public int Key { get; set; }

        // ------------------------------------------------------------------------

	    public int[]  m_objFlags = new int [2]; // Up to 32 different Descent 1 robots 
        public int m_hitPoints;// How hard it is to destroy this particular matcen
        public int m_interval; // Interval between materializations 
        public short m_nSegment; // Segment this is attached to. 
        public short m_nFuelCen; // Index in fuelcen array. 

        // ------------------------------------------------------------------------

        public void Read (BinaryReader fp, int version = 0, bool bFlag = false)
        {
        }

        // ------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int version = 0, bool bFlag = false)
        {
        }

        // ------------------------------------------------------------------------

        public void Clear ()
        {
            m_objFlags [0] = m_objFlags [1] = 0;
            m_hitPoints = 0;
            m_interval = 0;
            m_nSegment = 0;
            m_nFuelCen = 0;
        }

        // ------------------------------------------------------------------------

        public MatCenter ()
        {
            Clear ();
        }

        // ------------------------------------------------------------------------

	    public void Setup (short nSegment, short nIndex, int nFlags) 
        {
		    Clear ();
		    m_nSegment = nSegment;
		    m_objFlags [0] = nFlags;
		    m_nFuelCen = nIndex;
		}

        // ------------------------------------------------------------------------

        public static bool operator < (MatCenter m1, MatCenter m2)
        {
            return m1.m_nSegment < m2.m_nSegment;
        }

        public static bool operator > (MatCenter m1, MatCenter m2)
        {
            return m1.m_nSegment > m2.m_nSegment;
        }

        //------------------------------------------------------------------------

        public int CompareTo (object obj)
        {
            MatCenter other = obj as MatCenter;
            short i = this.m_nSegment;
            short m = other.m_nSegment;

            return (i < m) ? -1 : (i > m) ? 1 : 0;
        }

        // ------------------------------------------------------------------------

    }
}
