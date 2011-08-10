using System;
using System.IO;
using System.Collections.Generic;

namespace DLE.NET
{
    public partial class SegmentManager
    {
        // ------------------------------------------------------------------------

        public MineItemInfo m_info;

        Segment [] m_segments = new Segment [GameMine.MAX_SEGMENTS];

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

        public Segment [] Segments { get { return m_segments; } }

        // ------------------------------------------------------------------------

        public SegmentManager ()
        {
            Count = 0;
            for (int i = 0; i < GameMine.MAX_SEGMENTS; i++)
                m_segments [i] = new Segment (i);
        }

        // ------------------------------------------------------------------------

        public Segment this [int i]
        {
            get { return Segments [i]; }
            set { Segments [i] = value; }
        }

        // ------------------------------------------------------------------------

        public Vertex CalcCenter (Vertex center, short nSegment)
        {
            center.Clear ();
            Segment seg = Segments [nSegment];
            for (int i = 0; i < 8; i++)
                center.Add (seg.Vertex (i));
            return center;

        }

        // ------------------------------------------------------------------------
        // recompute vertex indices of all segments after a vertex has been deleted
        // and the last valid vertex has been moved to the deleted vertex' place:
        // Each vertex index == nOldIndex will be set to nNewIndex

        public void UpdateVertices (short nOldIndex, short nNewIndex)
        {
            for (int i = 0; i < Count; i++)
            {
                short [] verts = m_segments [i].m_verts;
                for (int j = 0; j < 8; j++)
                    if (verts [j] == nOldIndex)
                        verts [j] = nNewIndex;
            }
        }

        // ------------------------------------------------------------------------

    }
}