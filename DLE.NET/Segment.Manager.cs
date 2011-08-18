
namespace DLE.NET
{
    public partial class SegmentManager
    {
        // ------------------------------------------------------------------------

        public MineItemInfo [] m_matCenInfo = new MineItemInfo [2] { new MineItemInfo (), new MineItemInfo () };

        GameArray<Segment> m_segments = new GameArray<Segment> (GameMine.MAX_SEGMENTS);

        // ------------------------------------------------------------------------

        public MineItemInfo Info { get { return m_segments.Info; } }

        public int Count
        {
            get { return Info.count; }
            set { Info.count = value; }
        }

        public int FileOffset
        {
            get { return m_segments.FileOffset; }
            set { m_segments.FileOffset = value; }
        }

        public Segment [] Segments { get { return m_segments.Data; } }

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

        public Side Side (SideKey key)
        {
            return Segments [key.m_nSegment].m_sides [key.m_nSide];
        }

        // ------------------------------------------------------------------------

        public Wall Wall (SideKey key = null)
        {
            if (key == null)
                key = DLE.Current as SideKey;
            else
                DLE.Current.Get (key);
            return Side (key).Wall;
        }

        // ------------------------------------------------------------------------

        public Vertex CalcCenter (short nSegment)
        {
            Vertex center = new Vertex (0.0, 0.0, 0.0);
            Segment seg = Segments [nSegment];
            for (int i = 0; i < 8; i++)
                center.Add (seg.Vertex (i));
            return center;

        }

        // ------------------------------------------------------------------------

        public DoubleVector CalcSideCenter (SideKey key)
        {
        DLE.Current.Get (key);
	    Segment seg = Segments [key.m_nSegment];
	    DoubleVector v = new DoubleVector ();
        for (int i = 0; i < 4; i++)
	        v.Add (seg.Vertex (GameTables.sideVertTable [key.m_nSide,i]));
        v /= 4.0;
        return v;
        }

        // ------------------------------------------------------------------------

        public DoubleVector CalcSideNormal (SideKey key)
        {
            DLE.Current.Get (key);
            Segment seg = Segments [key.m_nSegment];
            DoubleVector v = new DoubleVector ();
            return -DoubleVector.Normal (seg.Vertex (GameTables.sideVertTable [key.m_nSegment,0]),
                                         seg.Vertex (GameTables.sideVertTable [key.m_nSegment,1]),
                                         seg.Vertex (GameTables.sideVertTable [key.m_nSegment,3]));
        }

        // ------------------------------------------------------------------------

        public Side OppositeSide (SideKey key, SideKey opp)
        {
            if (key == null)
                key = DLE.Current as SideKey;
            else
                DLE.Current.Get (key);
            short nChildSeg = Segments [key.m_nSegment].GetChild (key.m_nSide);
            if (nChildSeg < 0 || nChildSeg >= Count)
                return null;
            for (short nChildSide = 0; nChildSide < 6; nChildSide++)
            {
                if (Segments [nChildSeg].GetChild (nChildSide) == key.m_nSegment)
                {
                    opp.m_nSegment = nChildSeg;
                    opp.m_nSide = nChildSide;
                    return Side (opp);
                }
            }
            return null;
        }

        // ------------------------------------------------------------------------

        public bool IsWall (SideKey key)
        {
            DLE.Current.Get (key);
            return (Segments [key.m_nSegment].GetChild (key.m_nSide) == -1) || (Wall (key) != null);
        }

        // ------------------------------------------------------------------------

        public Wall OppositeWall (SideKey key)
        {
            DLE.Current.Get (key);
            Side side = Side (key);
            return (side == null) ? null : side.Wall;
        }

        // ------------------------------------------------------------------------

        public void DeleteWalls (short nSegment)
        {
            Side [] sides = Segments [nSegment].m_sides;
            for (int i = 0; i < 6; i++)
                DLE.Walls.Delete ((short) sides [i].m_nWall);
        }

        // ------------------------------------------------------------------------
        // recompute vertex indices of all segments after a vertex has been deleted
        // and the last valid vertex has been moved to the deleted vertex' place:
        // Each vertex index == nOldIndex will be set to nNewIndex

        public void UpdateVertices (ushort nOldIndex, ushort nNewIndex)
        {
            for (int i = 0; i < Count; i++)
            {
                ushort [] verts = m_segments [i].m_verts;
                for (int j = 0; j < 8; j++)
                    if (verts [j] == nOldIndex)
                        verts [j] = nNewIndex;
            }
        }

        // ------------------------------------------------------------------------

        void UpdateWalls (short nOldWall, short nNewWall)
        {
        Segment seg = Segments [0];
        for (int i = 0; i < Count; i++) {
	        Side [] sides = Segments [i].m_sides;
	        for (int j = 0; j < 6; j++)
                if (sides [i].m_nWall >= (ushort) nOldWall)
			        sides [i].m_nWall = (ushort) nNewWall;
	        }
        }

        // ------------------------------------------------------------------------

        void ResetSide (short nSegment, short nSide)
        {
        if (nSegment < 0 || nSegment >= Count) 
	        return; 
        DLE.Backup.Begin (UndoData.Flags.udSegments);
        Segments [nSegment].Reset (nSide);
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public void SetLinesToDraw ()
        {
        for (short nSegment = 0; nSegment < Count; nSegment++) {
            Segment seg = Segments [nSegment];
	        seg.m_mapBitmask |= 0xFFF; 
	        // if segment nSide has a child, clear bit for drawing line
	        for (short nSide = 0; nSide < 6; nSide++) {
		        if (seg.GetChild (nSide) > -1) { // -1 = no child,  - 2 = outside of world
			        seg.m_mapBitmask &= (short) ~(1 << (GameTables.sideLineTable [nSide,0])); 
			        seg.m_mapBitmask &= (short) ~(1 << (GameTables.sideLineTable [nSide,1])); 
			        seg.m_mapBitmask &= (short) ~(1 << (GameTables.sideLineTable [nSide,2])); 
			        seg.m_mapBitmask &= (short) ~(1 << (GameTables.sideLineTable [nSide,3])); 
			        }
		        }
	        }
        }

        // ------------------------------------------------------------------------

        public void CopyOtherSegment ()
        {
	        bool bChange = false;

        if (DLE.Current.m_nSegment == DLE.Other.m_nSegment)
	        return; 
        short nSegment = DLE.Current.m_nSegment; 
        Segment otherSeg = DLE.Other.Segment; 
        DLE.Backup.Begin (UndoData.Flags.udSegments);
        for (short nSide = 0; nSide < 6; nSide++)
            if (SetTextures (new SideKey (nSegment, nSide), otherSeg.m_sides [nSide].m_nBaseTex, otherSeg.m_sides [nSide].m_nOvlTex))
		        bChange = true;
        DLE.Backup.End ();
        if (bChange)
	        DLE.MineView.Refresh (); 
        }

        // ------------------------------------------------------------------------

        int Fix ()
        {
            int errFlags = 0;

            for (int i = 0; i < Count; i++)
            {
                Segment seg = Segments [i];
                for (short nSide = 0; nSide < 6; nSide++)
                {
                    Side side = seg.m_sides [nSide];
                    if ((side.m_nWall != GameMine.NO_WALL) && (side.m_nWall >= DLE.Walls.Count))
                    {
                        side.m_nWall = GameMine.NO_WALL;
                        errFlags |= 1;
                    }
                    if ((seg.GetChild (nSide) < -2) || (seg.GetChild (nSide) > Count))
                    {
                        seg.SetChild (nSide, -1);
                        errFlags |= 2;
                    }
                }
                for (short nVertex = 0; nVertex < 8; nVertex++)
                {
                    if ((seg.m_verts [nVertex] < 0) || (seg.m_verts [nVertex] >= DLE.Vertices.Count))
                    {
                        seg.m_verts [nVertex] = 0;  // this will cause a bad looking picture
                        errFlags |= 4;
                    }
                }
            }
            return errFlags;
        }

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

    }
}