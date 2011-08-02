using System;
using System.IO;
using System.Collections.Generic;

namespace DLE.NET
{
    public partial class SegmentManager
    {
        // ------------------------------------------------------------------------

        public Dictionary<int, Segment> m_segments = new Dictionary<int, Segment> ();

        KeyList m_keys = new KeyList (GameMine.SEGMENT_LIMIT);

        // ------------------------------------------------------------------------

        public Dictionary<int, Segment> Segments { get { return m_segments; } }

        // ------------------------------------------------------------------------


    }
}