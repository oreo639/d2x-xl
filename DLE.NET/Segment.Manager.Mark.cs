using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public partial class SegmentManager
    {
        // -----------------------------------------------------------------------------

        public void MarkAll (byte mask)
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

        public bool IsMarked (SideKey key)
        {
        DLE.Current.Get (key);
        Segment seg = Segments [key.m_nSegment];
        for (int i = 0; i < 4; i++) 
        {
	        if ((DLE.Vertices [seg.m_verts [GameTables.sideVertTable [key.m_nSide,i]]].Status & GameMine.MARKED_MASK) == 0)
		        return false;
	        }
        return true;
        }

        // -----------------------------------------------------------------------------

        public bool IsMarked (short nSegment)
        {
        Segment seg = Segments [nSegment];
        for (int i = 0;  i < 8; i++)
	        if ((DLE.Vertices [seg.m_verts [i]].Status & GameMine.MARKED_MASK) == 0)
		        return false;
        return true;
        }

        // -----------------------------------------------------------------------------

        public short MarkedCount (bool bCheck = false)
        {
        short nCount = 0;
        for (int i = 0; i < Count; i++)
        {
            if (Segments [i].IsMarked ())
            {
                if (bCheck)
                    return 1;
                ++nCount;
            }
        }
        return nCount; 
        }

        // -----------------------------------------------------------------------------

        public bool HaveMarkedSides
        {
            get 
            {
                SideKey key = new SideKey ();

                for (int i = 0; i < Count; i++)
                {
                    Segment seg = Segments [i];
	                short nSegment = (short) seg.Key;
	                for (short nSide = 0; nSide < 6; nSide++)
                    {
                        key.Set (nSegment, nSide);
		                if (IsMarked (key))
			                return true;
	                }
                }
                return false;
            }
        }

        // -----------------------------------------------------------------------------

        public void Mark (short nSegment)
        {
            Segment seg = Segments [nSegment];

            seg.Mark (); /* flip marked bit */

            // update vertices's marked status
            // ..first clear all marked verts
            // ..then mark all verts for marked Segment ()
            for (int i = 0; i < Count; i++)
            {
                seg = Segments [i];
                if (seg.IsMarked ())
                {
                    for (short nVertex = 0; nVertex < 8; nVertex++)
                        seg.Vertex (nVertex).Status |= GameMine.MARKED_MASK;
                }
            }
        }

        // -----------------------------------------------------------------------------
        // mark all segments which have all 8 verts marked

        public void UpdateMarked ()
        {
            for (int i = 0; i < Count; i++)
            {
                Segment seg = Segments [i];
                ushort[] verts = seg.m_verts;
	        if (((DLE.Vertices [verts [0]].Status & GameMine.MARKED_MASK) != 0) &&
		        ((DLE.Vertices [verts [1]].Status & GameMine.MARKED_MASK) != 0)  &&
		        ((DLE.Vertices [verts [2]].Status & GameMine.MARKED_MASK) != 0)  &&
		        ((DLE.Vertices [verts [3]].Status & GameMine.MARKED_MASK) != 0)  &&
		        ((DLE.Vertices [verts [4]].Status & GameMine.MARKED_MASK) != 0)  &&
		        ((DLE.Vertices [verts [5]].Status & GameMine.MARKED_MASK) != 0)  &&
		        ((DLE.Vertices [verts [6]].Status & GameMine.MARKED_MASK) != 0)  &&
                ((DLE.Vertices [verts [7]].Status & GameMine.MARKED_MASK) != 0))
		        seg.Mark (); 
	        else
		        seg.Unmark (); 
	        }
        }

        // -----------------------------------------------------------------------------

        public void MarkAll (byte mask) 
        {
            for (int i = 0; i < Count; i++)
            {
                Segment seg = Segments [i];
                seg.Mark (mask);
                for (short j = 0; j < 8; j++)
                    seg.Vertex (j).Mark (mask);
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
                    seg.Vertex (j).Unmark (mask);
            }
            DLE.MineView.Refresh (); 
        }

        // -----------------------------------------------------------------------------

        public void MarkSelected ()
        {
	        bool	bSegMark = false; 
	        Segment seg = DLE.Current.Segment; 
	        int i, nPoints;
            int [] p = new int [8];

        switch (DLE.MineView.SelectMode) 
        {
	        case MineView.SelectModes.Point:
		        nPoints = 1; 
		        p [0] = seg.m_verts [GameTables.sideVertTable [DLE.Current.m_nSide,DLE.Current.m_nPoint]]; 
		        break; 

	        case MineView.SelectModes.Line:
		        nPoints = 2; 
		        p [0] = seg.m_verts [GameTables.sideVertTable [DLE.Current.m_nSide,DLE.Current.m_nPoint]]; 
		        p [1] = seg.m_verts [GameTables.sideVertTable [DLE.Current.m_nSide,(DLE.Current.m_nPoint + 1) & 3]]; 
		        break; 

	        case MineView.SelectModes.Side:
		        nPoints = 4; 
		        for (i = 0; i < nPoints; i++)
			        p [i] = seg.m_verts [GameTables.sideVertTable [DLE.Current.m_nSide,i]]; 
		        break; 

	        case MineView.SelectModes.Segment:
		        nPoints = 8; 
		        for (i = 0; i < nPoints; i++)
			        p [i] = seg.m_verts [i]; 
		        break; 

	        default:
                nPoints = 0;
		        bSegMark = true;
                break;
	        }

        if (bSegMark)
	        Mark (DLE.Current.m_nSegment); 
        else {
	        // set i to nPoints if all verts are marked
	        for (i = 0; i < nPoints; i++)
                if ((DLE.Vertices [p [i]].Status & GameMine.MARKED_MASK) == 0) 
			        break; 
		        // if all verts are marked, then unmark them
	        if (i == nPoints) {
		        for (i = 0; i < nPoints; i++)
                    DLE.Vertices [p [i]].Unmark (); 
		        }
	        else {
		        // otherwise mark all the points
		        for (i = 0; i < nPoints; i++)
                    DLE.Vertices [p [i]].Mark (); 
		        }
	        UpdateMarked (); 
	        }
        DLE.MineView.Refresh (); 
        }

        // -----------------------------------------------------------------------------

    }
}
