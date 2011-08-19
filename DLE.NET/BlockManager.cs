using System.IO;
using System.Collections.Generic;
using System.Xml;
using System;

namespace DLE.NET
{
    public class BlockManager
    {
        // ------------------------------------------------------------------------

        string m_filename = "";
        List<Segment> m_oldSegments = new List<Segment> ();
        List<Segment> m_newSegments = new List<Segment> ();
        short [] m_xlatSegNum = new short [GameMine.SEGMENT_LIMIT];

        // ------------------------------------------------------------------------

        int CurrentPoint (int i)
        {
            return (DLE.Current.m_nPoint + i) % 4;
        }

        // ------------------------------------------------------------------------

        static readonly string BLOCKOP_HINT =
            @"The block of cubes will be saved relative to the current segment.\n
            Later, when you paste the block, it will be placed relative to\n
            the current segment at that time.  You can change the current side\n
            and the current point to affect the relative direction and\n
            rotation of the block.\n
            \n
            Would you like to proceed?";

        // ------------------------------------------------------------------------

        void SetupTransformation (DoubleMatrix m, DoubleVector o)
        {
        ushort[] verts = DLE.Current.Segment.m_verts;
        o = DLE.Vertices.Vertex (verts [GameTables.sideVertTable [DLE.Current.m_nSide,CurrentPoint(0)]]);
        // set x'
        m.rVec.Set (DLE.Vertices [verts [GameTables.sideVertTable [DLE.Current.m_nSide, CurrentPoint (1)]]]);
        m.rVec.Sub (o);
        // calculate y'
        Vertex v = new Vertex (DLE.Vertices [verts [GameTables.sideVertTable [DLE.Current.m_nSide,CurrentPoint(3)]]]);
        v.Sub (o);
        m.uVec = DoubleVector.CrossProduct (m.rVec, v);
        m.fVec = DoubleVector.CrossProduct (m.rVec, m.uVec);
        m.rVec.Normalize ();
        m.uVec.Normalize ();
        m.fVec.Normalize ();
        }

        // ------------------------------------------------------------------------
        // Read ()
        //
        // ACTION - Reads a segment's information in text form from a file.  Adds
        //          new vertices if non-identical one does not exist.  Aborts if
        //	    MAX_VERTICES is hit.
        //
        // Change - Now reads verts relative to current side
        // ------------------------------------------------------------------------

        short Read (XmlDocument block) 
        {
            int i, j, scanRes;
            int origVertCount;
            DoubleMatrix m = new DoubleMatrix ();
            DoubleVector xAxis = new DoubleVector (), yAxis = new DoubleVector (), zAxis = new DoubleVector (), origin = new DoubleVector ();
            short nNewSegs = 0, nNewWalls = 0, nNewTriggers = 0, nNewObjects = 0;
            int byteBuf; // needed for scanning byte values
            List<Trigger> newTriggers = new List<Trigger> ();
            XmlNode rootNode, segNode;
	
        m_oldSegments = m_newSegments = null;
        // remember number of vertices for later
        origVertCount = DLE.Vertices.Count;

        // set origin
        SetupTransformation (m, origin);
        // now take the determinant
        xAxis.Set (m.uVec.v.y * m.fVec.v.z - m.fVec.v.y * m.uVec.v.z, 
			       m.fVec.v.y * m.rVec.v.z - m.rVec.v.y * m.fVec.v.z, 
			       m.rVec.v.y * m.uVec.v.z - m.uVec.v.y * m.rVec.v.z);
        yAxis.Set (m.fVec.v.x * m.uVec.v.z - m.uVec.v.x * m.fVec.v.z, 
			       m.rVec.v.x * m.fVec.v.z - m.fVec.v.x * m.rVec.v.z,
			       m.uVec.v.x * m.rVec.v.z - m.rVec.v.x * m.uVec.v.z);
        zAxis.Set (m.uVec.v.x * m.fVec.v.y - m.fVec.v.x * m.uVec.v.y,
			       m.fVec.v.x * m.rVec.v.y - m.rVec.v.x * m.fVec.v.y,
			       m.rVec.v.x * m.uVec.v.y - m.uVec.v.x * m.rVec.v.y);

        nNewSegs = 0;
        for (i = 0; i < m_xlatSegNum.Length; i++)
            m_xlatSegNum [i] = -1;

        for (i = 0, j = DLE.Segments.Count; i < j; i++) 
	        m_oldSegments.Add (DLE.Segments [i]);

        DLE.Backup.Begin (UndoData.Flags.udAll);

        rootNode = block.SelectSingleNode ("Segments");
        while ((segNode = rootNode.SelectSingleNode (string.Format (@"Segment{0}", nNewSegs + 1))) != null)
        {
	        //DLE.MainFrame.Progress.SetPos (fp.BaseStream.Position);
            // abort if there are not at least 8 vertices free
	        if (GameMine.MAX_VERTICES - DLE.Vertices.Count < 8)
            {
		        DLE.Backup.End ();
		        DLE.ErrorMsg ("No more free vertices");
		        return nNewSegs;
		        }
	        short nSegment = DLE.Segments.Add ();
	        if (nSegment < 0) 
            {
		        DLE.Backup.End ();
		        DLE.ErrorMsg ("No more free segments");
		        return nNewSegs;
		        }
	        Segment seg = DLE.Segments [nSegment];
            m_newSegments.Add (seg);
	        seg.m_owner = -1;
	        seg.m_group = -1;
            seg.Key = Convert.ToInt32 (segNode.Attributes ["Key"].InnerText);
	        m_xlatSegNum [seg.Key] = nSegment;
	        // invert segment number so its children can be children can be fixed later
	        seg.Key = -seg.Key - 1;

	        // read in side information 
	        for (short nSide = 0; nSide < 6; nSide++) {
    	        Side side = seg.m_sides [nSide];
                if (side.ReadXML (segNode, nSide) < 0)
                    return 0;
                Wall w = side.Wall;
                if (w != null)
                {
                    w.m_nSegment = nSegment;
                    Trigger t = w.Trigger;
                    if (t != null)
                    {
                        newTriggers.Add (t);
                        ++nNewTriggers;
                    }


	        for (i = 0; i < 6; i++)
		        seg.SetChild ((short) i, children [i]);
	        // read in vertices
	        byte bShared = 0;
	        for (i = 0; i < 8; i++) {
		        int x, y, z, test;
		        scanRes = fscanf_s (fp, "  vms_vector %d %d %d %d\n", test, x, y, z);
		        if (test != i) {
			        DLE.Backup.End ();
			        DLE.ErrorMsg ("Invalid vertex number read");
			        return (0);
			        }
		        // each vertex relative to the origin has a x', y', and z' component
		        // adjust vertices relative to origin
                DoubleVector v = new DoubleVector (FixConverter.X2D (x), FixConverter.X2D (y), FixConverter.X2D (z));
		        v.Set (v ^ xAxis, v ^ yAxis, v ^ zAxis);
		        v.Add (origin);
		        // add a new vertex
		        // if this is the same as another vertex, then use that vertex number instead
		        Vertex vert = DLE.Vertices.Find (v);
		        ushort nVertex;
		        if (vert != null) {
			        nVertex = seg.m_verts [i] = (ushort) vert.Key;
			        bShared |= (byte) (1 << i);
			        }
		        // else make a new vertex
		        else  {
			        DLE.Vertices.Add (out nVertex);
			        DLE.Vertices [nVertex].Status  |= GameMine.NEW_MASK;
			        seg.m_verts [i] = nVertex;
			        DLE.Vertices [nVertex].Set (v);
			        }
		        DLE.Vertices [nVertex].Status |= GameMine.MARKED_MASK;
		        }

	        // mark vertices
	        scanRes = fscanf_s (fp, "  static_light %d\n", seg.m_staticLight);
	        if (DLE.ExtBlkFmt) {
		        scanRes = fscanf_s (fp, "  special %d\n", seg.m_function);
		        scanRes = fscanf_s (fp, "  matcen_num %d\n", byteBuf);
		        seg.m_nMatCen = (sbyte) byteBuf;
		        scanRes = fscanf_s (fp, "  value %d\n", byteBuf);
		        seg.m_value = (sbyte) byteBuf;
		        scanRes = fscanf_s (fp, "  child_bitmask %d\n", byteBuf);
		        seg.m_childFlags = (byte) byteBuf;
		        scanRes = fscanf_s (fp, "  wall_bitmask %d\n", byteBuf);
		        seg.m_wallFlags = (byte) byteBuf;
		        switch (seg.m_function) {
			        case Segment.Functions.FUELCEN:
				        if (DLE.Segments.CreateFuelCenter (nSegment, Segment.Functions.FUELCEN, false, false) == 0)
					        seg.m_function = 0;
				        break;
			        case Segment.Functions.REPAIRCEN:
				        if (DLE.Segments.CreateFuelCenter (nSegment, Segment.Functions.REPAIRCEN, false, false) == 0)
					        seg.m_function = 0;
				        break;
			        case Segment.Functions.ROBOTMAKER:
				        if (!DLE.Segments.CreateRobotMaker (nSegment, false, false))
					        seg.m_function = 0;
				        break;
			        case Segment.Functions.EQUIPMAKER:
				        if (!DLE.Segments.CreateEquipMaker (nSegment, false, false))
					        seg.m_function = 0;
				        break;
			        case Segment.Functions.REACTOR:
				        if (!DLE.Segments.CreateReactor (nSegment, false, false))
					        seg.m_function = 0;
				        break;
			        default:
				        break;
			        }
		        }
	        else {
		        seg.m_function = 0;
		        seg.m_nMatCen = -1;
		        seg.m_value = -1;
		        }
	        seg.Mark (GameMine.MARKED_MASK); // no other bits
	        // calculate childFlags
	        seg.m_childFlags = 0;
	        for (i = 0; i < 6; i++) {
		        if (seg.GetChild ((short) i) >= 0)
			        seg.m_childFlags |= (byte) (1 << i);
		        }
	        nNewSegs++;
	        }


        foreach (Trigger trig in newTriggers)
        {
	        for (j = 0; j < trig.Count; j++) {
		        if (trig [j].m_nSegment >= 0)
			        trig [j].m_nSegment = m_xlatSegNum [trig [j].m_nSegment];
		        else if (trig.Count == 1) {
			        DLE.Triggers.Delete ((short) trig.Key);
			        i--;
			        }
		        else {
			        trig.Delete (j);
			        }
		        }
	        }

        DLE.Backup.End ();
        DLE.DebugMsg (string.Format (@"Block tool: {0} blocks, {1} walls, {2} triggers pasted.", nNewSegs, nNewWalls, nNewTriggers));
        return nNewSegs;
        }

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

    }
}
