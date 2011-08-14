using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public class SideKey
    {
        public short m_nSegment;
        public short m_nSide;
        public Segment m_segment = null;

        // ------------------------------------------------------------------------

        public Segment Segment { get { return m_segment; } }

        // ------------------------------------------------------------------------

        public short SegNum 
        {
            get 
            { 
                return m_nSegment; 
            }
            set
            {
                m_nSegment = value;
                m_segment = (m_nSegment < 0) ? null : DLE.Mine.Segments [m_nSegment];
            }
        }

        // ------------------------------------------------------------------------

        public SideKey (short nSegment = -1, short nSide = -1) 
        { 
            m_nSegment = nSegment; 
            m_nSide = nSide;
        }

        // ------------------------------------------------------------------------

        public void Set (short nSegment, short nSide)
        {
            m_nSegment = nSegment;
            m_nSide = nSide;
        }

        // ------------------------------------------------------------------------

        public void Clear ()
        {
            m_nSegment = -1;
            m_nSide = -1;
        }

        // ------------------------------------------------------------------------

        #region operators

        // ------------------------------------------------------------------------

        public static bool operator < (SideKey k1, SideKey k2)
        {
            return (k1.m_nSegment < k2.m_nSegment) || ((k1.m_nSegment == k2.m_nSegment) && (k1.m_nSide < k2.m_nSide));
        }

        // ------------------------------------------------------------------------

        public static bool operator > (SideKey k1, SideKey k2)
        {
            return (k1.m_nSegment > k2.m_nSegment) || ((k1.m_nSegment == k2.m_nSegment) && (k1.m_nSide > k2.m_nSide));
        }

        // ------------------------------------------------------------------------

        public static bool operator == (SideKey k1, SideKey k2) 
        { 
            return (k1.m_nSegment == k2.m_nSegment) && (k1.m_nSide == k2.m_nSide); 
        }

        // ------------------------------------------------------------------------

        public static bool operator != (SideKey k1, SideKey k2) 
        { 
            return (k1.m_nSegment != k2.m_nSegment) || (k1.m_nSide != k2.m_nSide); 
        }

        // ------------------------------------------------------------------------

        public override bool Equals (System.Object obj)
        {
            // If parameter is null return false.
            if (obj == null)
            {
                return false;
            }

            // If parameter cannot be cast to Point return false.
            SideKey p = obj as SideKey;
            if ((System.Object)p == null)
            {
                return false;
            }

            // Return true if the fields match:
            return (m_nSegment == p.m_nSegment) && (m_nSide == p.m_nSide);
        }

        // ------------------------------------------------------------------------

        public bool Equals (SideKey p)
        {
            // If parameter is null return false:
            if ((object)p == null)
            {
                return false;
            }

            // Return true if the fields match:
            return (m_nSegment == p.m_nSegment) && (m_nSide == p.m_nSide);
        }

        // ------------------------------------------------------------------------

        public override int GetHashCode ()
        {
            return m_nSegment ^ m_nSide;
        }

        #endregion

        // ------------------------------------------------------------------------

    }
}
