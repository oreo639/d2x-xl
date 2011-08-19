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
            int i, j;
            int origVertCount;
            DoubleMatrix m = new DoubleMatrix ();
            DoubleVector xAxis = new DoubleVector (), yAxis = new DoubleVector (), zAxis = new DoubleVector (), origin = new DoubleVector ();
            short nNewSegs = 0, nNewWalls = 0, nNewTriggers = 0;
            List<Trigger> newTriggers = new List<Trigger> ();
            XmlNode rootNode;

            m_oldSegments.Clear ();
            m_newSegments.Clear ();
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

            Segment s = new Segment ();
            while (true)
            {
                //DLE.MainFrame.Progress.SetPos (fp.BaseStream.Position);
                // abort if there are not at least 8 vertices free
                // check available vertices
                if (GameMine.MAX_VERTICES - DLE.Vertices.Count < 8)
                {
                    DLE.Backup.End ();
                    DLE.ErrorMsg ("No more free vertices");
                    break;
                }

                // try to allocate a segment
                short nSegment = DLE.Segments.Add ();
                if (nSegment < 0)
                {
                    DLE.Backup.End ();
                    DLE.ErrorMsg ("No more free segments");
                    break;
                }

                // read the segment data (including side and any wall and trigger data)
                Segment seg = DLE.Segments [nSegment];
                if (seg.ReadXML (rootNode, nNewSegs, nSegment, xAxis, yAxis, zAxis, origin) < 1)
                {
                    DLE.Segments.Remove (nSegment);
                    break;
                }

                m_newSegments.Add (seg);
                m_xlatSegNum [-seg.Key - 1] = nSegment;

                for (short nSide = 0; nSide < 6; nSide++)
                {
                    Side side = seg.m_sides [nSide];
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
                    }
                    nNewSegs++;
                }


                foreach (Trigger trig in newTriggers)
                {
                    for (j = 0; j < trig.Count; j++)
                    {
                        if (trig [j].m_nSegment >= 0)
                            trig [j].m_nSegment = m_xlatSegNum [trig [j].m_nSegment];
                        else if (trig.Count == 1)
                        {
                            DLE.Triggers.Delete ((short)trig.Key);
                            i--;
                        }
                        else
                        {
                            trig.Delete (j);
                        }
                    }
                }
            }

            DLE.Backup.End ();
            DLE.DebugMsg (string.Format (@"Block tool: {0} blocks, {1} walls, {2} triggers pasted.", nNewSegs, nNewWalls, nNewTriggers));
            return nNewSegs;
        }

        // ------------------------------------------------------------------------

        public void Write () 
        {
	        Vertex			origin = new Vertex ();
	        DoubleMatrix	m = new DoubleMatrix ();
	        DoubleVector	v = new DoubleVector ();

            XmlDocument doc = new XmlDocument ();
            XmlElement root = doc.CreateElement ("Block");

        // set origin
        SetupTransformation (m, origin);

        for (int nSegment = 0; nSegment < DLE.Segments.Count; nSegment++) 
        {
	        //DLE.MainFrame ().Progress ().StepIt ();
	        Segment seg = DLE.Segments [nSegment];
	        if (seg.IsMarked ()) {
                seg.WriteXML (doc, root, nSegment);
		        fprintf (fp.File (), "segment %d\n", nSegment);
		        for (short nSide = 0; nSide < 6; nSide++) 
                {
                    Side side = seg.m_sides [nSide];
                    fprintf (fp.File (), "  side %d\n", i);
			        fprintf (fp.File (), "    tmap_num %d\n",side.m_nBaseTex);
			        fprintf (fp.File (), "    tmap_num2 %d\n",side.m_nOvlTex);
			        for (int i = 0; i < 4; i++) {
				        fprintf (fp.File (), "    uvls %d %d %d\n",
							        side.m_uvls [i].u, side.m_uvls [i].v, side.m_uvls [i].l);
				        }
			        if (DLE.ExtBlkFmt) {
				        fprintf (fp.File (), "    wall %d\n", side.m_nWall);
				        if (side.m_nWall != GameMine.NO_WALL) {
					        Wall wall = side.Wall;
					        fprintf (fp.File (), "        segment %d\n", wall.m_nSegment);
					        fprintf (fp.File (), "        side %d\n", wall.m_nSide);
					        fprintf (fp.File (), "        hps %d\n", wall.m_hps);
					        fprintf (fp.File (), "        type %d\n", wall.Type);
					        fprintf (fp.File (), "        flags %d\n", wall.m_flags);
					        fprintf (fp.File (), "        state %d\n", wall.m_state);
					        fprintf (fp.File (), "        clip %d\n", wall.m_nClip);
					        fprintf (fp.File (), "        keys %d\n", wall.m_keys);
					        fprintf (fp.File (), "        cloak %d\n", wall.m_cloakValue);
					        if (wall.m_nTrigger == GameMine.NO_TRIGGER)
						        fprintf (fp.File (), "        trigger %u\n", GameMine.NO_TRIGGER);
					        else {
						        Trigger trig = wall.Trigger;
						        int iTarget, count = 0;
						        // count trigger targets in marked area
						        for (iTarget = 0; iTarget < trig.Count; iTarget++)
							        if (DLE.Segments [trig [iTarget].m_nSegment].IsMarked ())
								        count++;
						        fprintf (fp.File (), "        trigger %d\n", wall.m_nTrigger);
						        fprintf (fp.File (), "			    type %d\n", trig.Type);
						        fprintf (fp.File (), "			    flags %d\n", trig.Flag);
						        fprintf (fp.File (), "			    value %d\n", trig.Value);
						        fprintf (fp.File (), "			    timer %d\n", trig.Time);
						        fprintf (fp.File (), "			    count %d\n", count);
						        for (iTarget = 0; iTarget < trig.Count; iTarget++)
							        if (DLE.Segments [trig [iTarget].m_nSegment].IsMarked ()) {
								        fprintf (fp.File (), "			        segment %d\n", trig [iTarget].m_nSegment);
								        fprintf (fp.File (), "			        side %d\n", trig [iTarget].m_nSide);
								        }
						        }
					        }
				        }
			        }
		        fprintf (fp.File (), "  children");
		        for (i = 0; i < 6; i++) {
			        short nChild = seg.GetChild (i);
			        fprintf (fp.File (), " %d", ((nChild < 0) || !DLE.Segments [nChild].IsMarked ()) ? -1 : nChild);
			        }
		        fprintf (fp.File (), "\n");
		        // save vertices
		        for (i = 0; i < 8; i++) {
			        // each vertex relative to the origin has a x', y', and z' component
			        // which is a constant (k) times the axis
			        // k = (B*A)/(A*A) where B is the vertex relative to the origin
			        //                       A is the axis unit vertex (always 1)
			        ushort nVertex = seg.m_verts [i];
			        v.Set (DLE.Vertices [nVertex]);
                    v.Sub (origin);
                    fprintf (fp.File (), "  vms_vector %d %d %d %d\n", i, FixConverter.D2X (v ^ m.rVec), FixConverter.D2X (v ^ m.uVec), FixConverter.D2X (v ^ m.fVec));
			        }
		        fprintf (fp.File (), "  static_light %d\n",seg.m_staticLight);
		        if (DLE.ExtBlkFmt) {
			        fprintf (fp.File (), "  special %d\n", seg.m_function);
			        fprintf (fp.File (), "  matcen_num %d\n", seg.m_nMatCen);
			        fprintf (fp.File (), "  value %d\n", seg.m_value);
			        fprintf (fp.File (), "  child_bitmask %d\n", seg.m_childFlags);
			        fprintf (fp.File (), "  wall_bitmask %d\n", seg.m_wallFlags);
			        }
		        }
	        }
        }
        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

    }
}
