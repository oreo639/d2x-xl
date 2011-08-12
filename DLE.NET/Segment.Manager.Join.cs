using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLEdotNET
{
    public partial class SegmentManager
    {
        // ------------------------------------------------------------------------

        struct VertMatch 
        {
		    public short b;
            public short i;
            public double d;
	    } 

        // ------------------------------------------------------------------------
        // LinkSegments()
        //
        //  Action - checks 2 Segments [) and 2 sides to see if the vertices are identical
        //           If they are, then the segment sides are linked and the vertices
        //           are removed (nSide1 is the extra side).
        //
        //  Change - no longer links if segment already has a child
        //           no longer links Segments [) if vert numbers are not in the right order
        //
        // ------------------------------------------------------------------------

        static int [,] matches = new int [4, 4] { { 0, 3, 2, 1 }, { 1, 0, 3, 2 }, { 2, 1, 0, 3 }, { 3, 2, 1, 0 } };

        public bool Link (short nSegment1, short nSide1, short nSegment2, short nSide2, double margin)
        {
	        short		i, j; 
	        Vertex []	v1 = new Vertex [4], v2 = new Vertex [4]; 
	        VertMatch[]	match = new VertMatch [4]; 

        Segment seg1 = Segments [nSegment1]; 
        Segment seg2 = Segments [nSegment2]; 

        // don't link to a segment which already has a child
        if (seg1.GetChild (nSide1) !=-1 || seg2.GetChild (nSide2) != -1)
	        return false; 

        // copy vertices for comparison later (makes code more readable)
        for (i = 0; i < 4; i++) 
        {
	        v1 [i] = DLE.Vertices [seg1.m_verts [GameTables.sideVertTable [nSide1,i]]];
	        v2 [i] = DLE.Vertices [seg2.m_verts [GameTables.sideVertTable [nSide2,i]]];
	        match [i].i = -1; 
        }

        // check to see if all 4 vertices match exactly one of each of the 4 other segment's vertices
        for (i = 0; i < 4; i++)
	        for (j = 0; j < 4; j++)
		        if ((Math.Abs (v1 [i].v.x - v2 [j].v.x) < margin &&
			        Math.Abs (v1 [i].v.y - v2 [j].v.y) < margin &&
			         Math.Abs (v1 [i].v.z - v2 [j].v.z) < margin))
			        if (match [j].i != -1) // if this vertex already matched another vertex then abort
				        return false; 
			        else
				        match [j].i = i;  // remember which vertex it matched
        if (match [0].i == -1)
	        return false;

        for (i = 1; i < 4; i++)
	        if (match [i].i != matches [match [0].i,i])
		        return false;
        // make sure verts match in the correct order
        // if not failed and match found for each
        LinkSides (nSegment1, nSide1, nSegment2, nSide2, match); 
        return true; 
        }

        // ------------------------------------------------------------------------

        void LinkSides (short nSegment1, short nSide1, short nSegment2, short nSide2, VertMatch [] match) 
        {
	        Segment	seg1 = Segments [nSegment1]; 
	        Segment	seg2 = Segments [nSegment2]; 
	        ushort	nVertex, oldVertex, newVertex; 
	        int		i; 

        DLE.Backup.Begin (UndoData.Flags.udSegments);
        seg1.SetChild (nSide1, nSegment2); 
        seg1.m_sides [nSide1].m_nBaseTex = 0; 
        seg1.m_sides [nSide1].m_nOvlTex = 0; 
        for (i = 0; i < 4; i++) 
	        seg1.m_sides [nSide1].m_uvls [i].Clear (); 
        seg2.SetChild (nSide2, nSegment1); 
        seg2.m_sides [nSide2].m_nBaseTex = 0; 
        seg2.m_sides [nSide2].m_nOvlTex = 0; 
        for (i = 0; i < 4; i++) 
	        seg2.m_sides [nSide2].m_uvls [i].Clear (); 

        // merge vertices
        for (i = 0; i < 4; i++) {
	        oldVertex = seg1.m_verts [GameTables.sideVertTable [nSide1,i]]; 
	        newVertex = seg2.m_verts [GameTables.sideVertTable [nSide2,match [i].i]]; 

	        // if either vert was marked, then mark the new vert
	        DLE.Vertices [newVertex].Status |= (byte) (DLE.Vertices [oldVertex].Status & GameMine.MARKED_MASK); 

	        // update all Segments [) that use this vertex
	        if (oldVertex != newVertex) {
		        for (int j = 0; i < Count; j++) 
                {
			        Segment seg = Segments [i];
			        for (nVertex = 0; nVertex < 8; nVertex++)
				        if (seg.m_verts [nVertex] == oldVertex)
					        seg.m_verts [nVertex] = newVertex; 
			        }
		        // then delete the vertex
		        DLE.Vertices.Delete (oldVertex); 
		        }
	        }
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

    }
}
