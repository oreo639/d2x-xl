using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public partial class SegmentManager
    {
        // -----------------------------------------------------------------------------

        public void UnmarkAll (byte mask)
        {
            for (int i = 0; i < Count; i++)
            {
                Segment seg = Segments [i];
                seg.Unmark (mask);
                for (short j = 0; j < 8; j++)
                    seg.Vertex (j).Status |= mask;
            }
            DLE.MineView.Refresh ();
        }

        // -----------------------------------------------------------------------------

        public void UnmarkAll (byte mask) 
        {
        for (int i = 0; i < Count; i++)
        {
	        Segment seg = Segments [i];
	        seg.Unmark (mask); 
	        for (short j = 0; j < 8; j++)
		        seg.Vertex (j).Status &= (byte) ~mask;
	        }
        DLE.MineView.Refresh (); 
        }

        // -----------------------------------------------------------------------------

    }
}
