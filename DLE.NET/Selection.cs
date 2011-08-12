using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public partial class Selection : SideKey
    {

        // ------------------------------------------------------------------------

        short m_nLine = GameMine.DEFAULT_LINE;
        short m_nPoint = GameMine.DEFAULT_POINT;
        short m_nObject = GameMine.DEFAULT_OBJECT;

        // ------------------------------------------------------------------------

        public Selection ()
            : base (0, GameMine.DEFAULT_SIDE)
        {
        }

        // ------------------------------------------------------------------------

        public void Setup (short nSegment, short nSide, short nLine, short nPoint) 
        {
            if (nSegment >= 0) 
	            m_nSegment = nSegment;
            if (nSide >= 0) 
	            m_nSide = nSide;
            if (nLine >= 0) 
	            m_nLine = nLine;
            if (nPoint >= 0) 
	            m_nPoint = nPoint;
        }

        // ------------------------------------------------------------------------

        public void Get (SideKey key)
        {
        if (key.m_nSegment < 0)
	        key.m_nSegment = m_nSegment;
        if (key.m_nSide < 0)
	        key.m_nSide = m_nSide;
        }

        // ------------------------------------------------------------------------

        public void Get (ref short nSegment, ref short nSide)
            {
            if (nSegment < 0)
	            nSegment = m_nSegment;
            if (nSide < 0)
	            nSide = m_nSide;
        }

        // ------------------------------------------------------------------------

        public new Segment Segment 
        { 
            get 
            { 
                return DLE.Segments [m_nSegment]; 
            } 
        }

        // ------------------------------------------------------------------------

        public Segment ChildSeg
        {
            get
            {
                short nChild = Child;
                return (nChild < 0) ? null : DLE.Segments [nChild];
            }
        }

        // ------------------------------------------------------------------------

        public short Child 
        { 
            get 
            { 
                return Segment.GetChild (m_nSide); 
            } 
        }

        // ------------------------------------------------------------------------

        public Side Side
        {
            get 
            {
                return DLE.Segments.Side (this);
            }
        }

        // ------------------------------------------------------------------------

        public Wall Wall
        {
            get 
            {
                return DLE.Segments.Wall (this);
            }
        }

        // ------------------------------------------------------------------------

        Trigger Trigger ()
        {
            Wall wall = Wall;
            return (wall == null) ? null : wall.Trigger;
        }

        // ------------------------------------------------------------------------

        GameObject GameObject
        {
            get
            {
                return DLE.Objects [m_nObject];
            }
        }

        // ------------------------------------------------------------------------

        //Color LightColor
        //{
        //    get
        //    {
        //        return DLE.Lights [this];
        //    }
        //}

        // ------------------------------------------------------------------------

        void Fix (short nSegment)
        {
            if (m_nSegment != nSegment)
                return;
            short nChild = Child;
            if (nChild > -1)
            {
                m_nSegment = nChild;
                return;
            }
            if ((nChild = Segment.GetChild (GameTables.oppSideTable [m_nSide])) > -1)
            {
                m_nSegment = nChild;
                return;
            }
            for (short nSide = 0; nSide < 6; nSide++)
                if ((nChild = Segment.GetChild (nSide)) >= 0)
                {
                    m_nSegment = nChild;
                    return;
                }
            if (DLE.Segments.Count == 1)
                return;
            if (m_nSegment >= DLE.Segments.Count - 1)
                m_nSegment--;
            else
                m_nSegment++;
        }

        // ------------------------------------------------------------------------

    }
}
