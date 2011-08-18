using System;

namespace DLE.NET
{
    public partial class SegmentManager
    {
        // ------------------------------------------------------------------------

        public enum AddModes : byte
        {
            ORTHOGONAL = 0,
            EXTEND = 1,
            MIRROR = 2
        };

        public bool m_bCreating = false;
        public AddModes m_addMode = AddModes.ORTHOGONAL;

        // ------------------------------------------------------------------------

        bool Full { get { return Count >= GameMine.MAX_SEGMENTS; } }

        // ------------------------------------------------------------------------

        public short Add ()
        {
            if (Full)
                return -1;
            return (short) Count++;
        }

        // ------------------------------------------------------------------------

        public void Remove (short nDelSeg)
        {
            if (nDelSeg < --Count)
            {
                // move the last segment in the segment list to the deleted segment's position
                if (DLE.Current.m_nSegment == Count)
                    DLE.Current.m_nSegment = nDelSeg;
                if (DLE.Other.m_nSegment == Count)
                    DLE.Other.m_nSegment = nDelSeg;
                Segment temp = Segments [nDelSeg];
                Segments [nDelSeg] = Segments [Count];
                Segments [nDelSeg].Key = nDelSeg;
                Segments [Count] = temp;
                Segments [Count].Key = Count;
                // update all trigger targets pointing at the moved segment
                DLE.Triggers.UpdateTargets ((short) Count, nDelSeg);
                DLE.Objects.UpdateSegments ((short)Count, nDelSeg);
                // update all walls inside the moved segment
                Side [] side = Segments [nDelSeg].m_sides;
                for (int i = 0; i < 6; i++)
                {
                    Segment seg = side [i].Child;
                    if (seg != null)
                        seg.UpdateChildren ((short) Count, nDelSeg);
                    Wall wall = side [i].Wall;
                    if (wall != null)
                        wall.m_nSegment = nDelSeg;
                }
            }
        }

        // ------------------------------------------------------------------------

        ushort CURRENT_POINT(ushort i)
        {
            return (ushort)((DLE.Current.m_nPoint + i) % 4);
        }

        // ------------------------------------------------------------------------

        void ComputeVertices (ushort[] newVerts)
        {
	        Segment		    curSeg; 
            DoubleVector[]  A = new DoubleVector [8], B = new DoubleVector [8], C = new DoubleVector [8], D = new DoubleVector [8], E = new DoubleVector [8];
	        DoubleVector	a, b, c, d; 
	        double			length; 
	        ushort			nVertex; 
	        ushort			i;
            ushort []       points = new ushort [4]; 
	        DoubleVector	center, oppCenter, newCenter, vNormal; 

            curSeg = DLE.Current.Segment; 
            for (i = 0; i < 4; i++)
	            points [i] = CURRENT_POINT(i);
	            // METHOD 1: orthogonal with right angle on new side and standard segment side
            // TODO:
            //	int add_segment_mode = ORTHOGONAL; 
            if (m_addMode == AddModes.ORTHOGONAL)
		    {
		        center = CalcSideCenter (DLE.Current); 
		        oppCenter = CalcSideCenter (new SideKey (DLE.Current.m_nSegment, GameTables.oppSideTable [DLE.Current.m_nSide])); 
		        vNormal = CalcSideNormal (DLE.Current); 
		        // set the length of the new segment to be one standard segment length
		        // scale the vector
		        vNormal *= 20; 
		        // figure out new center
		        newCenter = center + vNormal; 
		        // new method: extend points 0 and 1 with vNormal, then move point 0 toward point 1.
		        // point 0
                a = vNormal + DLE.Vertices [curSeg.m_verts [GameTables.sideVertTable [DLE.Current.m_nSide, CURRENT_POINT (0)]]]; 
		        // point 1
                b = vNormal + DLE.Vertices [curSeg.m_verts [GameTables.sideVertTable [DLE.Current.m_nSide, CURRENT_POINT (1)]]]; 
		        // center
		        c = DoubleVector.Average (a, b);
		        // vector from center to point0 and its length
		        d = a - c; 
		        length = d.Mag (); 
		        // factor to mul
		        double factor = (length > 0) ? 10.0 / length : 1.0; 
		        // set point 0
		        d *= factor;
		        A [points [0]] = c + d; 
		        // set point 1
		        A [points [1]] = c - d; 
		        // point 2 is orthogonal to the vector 01 and the vNormal vector
		        c = -DoubleVector.CrossProduct (A [points [0]] - A [points [1]], vNormal);
		        c.Normalize ();
		        // normalize the vector
		        A [points [2]] = A [points [1]] + (c * 20); 
		        A [points [3]] = A [points [0]] + (c * 20); 
		        // now center the side along about the newCenter
		        a = (A [0] + A [1] + A [2] + A [3]); 
		        a /= 4;
		        for (i = 0; i < 4; i++)
			        A [i] += (newCenter - a); 
		        // set the new vertices
		        for (i = 0; i < 4; i++) 
                {
			        //nVertex = curSeg.m_verts [sideVertTable [DLE.Current.m_nSide][i]]; 
			        nVertex = newVerts [i];
			        DLE.Vertices [nVertex].Set (A [i]); 
			    }
		    }
	        // METHOD 2: orghogonal with right angle on new side
            else if (m_addMode == AddModes.EXTEND)
		    {
		        center = CalcSideCenter (DLE.Current); 
		        oppCenter = CalcSideCenter (new SideKey (DLE.Current.m_nSegment, GameTables.oppSideTable [DLE.Current.m_nSide])); 
		        vNormal = CalcSideNormal (DLE.Current); 
		        // calculate the length of the new segment
		        vNormal *= DoubleVector.Distance (center, oppCenter); 
		        // set the new vertices
		        for (i = 0; i < 4; i++) 
                {
			        nVertex = newVerts [i];
			        DLE.Vertices [nVertex].Set (DLE.Vertices [curSeg.m_verts [GameTables.sideVertTable [DLE.Current.m_nSide,i]]] + vNormal); 
			    }
            }
            // METHOD 3: mirror relative to plane of side
            else if (m_addMode == AddModes.MIRROR)
	        {
		        // copy side's four points into A
		        short nSide = DLE.Current.m_nSide;
		        for (i = 0; i < 4; i++) 
                {
			        A [i] = DLE.Vertices [curSeg.m_verts [GameTables.sideVertTable [nSide,i]]]; 
			        A [i + 4] = DLE.Vertices [curSeg.m_verts [GameTables.oppSideVertTable [nSide,i]]]; 
			    }

		        // subtract point 0 from all points in A to form B points
		        for (i = 0; i < 8; i++)
			        B [i] = A [i] - A [0]; 

		        // calculate angle to put point 1 in x - y plane by spinning on x - axis
		        // then rotate B points on x - axis to form C points.
		        // check to see if on x - axis already
		        double angle1 = Math.Atan2 (B [1].v.z, B [1].v.y); 
		        for (i = 0; i < 8; i++)
			        C [i].Set (B [i].v.x, B [i].v.y * Math.Cos (angle1) + B [i].v.z * Math.Sin (angle1), B [i].v.z * Math.Cos (angle1) - B [i].v.y * Math.Sin (angle1)); 
		        // calculate angle to put point 1 on x axis by spinning on z - axis
		        // then rotate C points on z - axis to form D points
		        // check to see if on z - axis already
		        double angle2 = Math.Atan2 (C [1].v.y, C [1].v.x); 
		        for (i = 0; i < 8; i++)
			        D [i].Set (C [i].v.x * Math.Cos (angle2) + C [i].v.y * Math.Sin (angle2), C [i].v.y * Math.Cos (angle2) - C [i].v.x * Math.Sin (angle2), C [i].v.z); 

		        // calculate angle to put point 2 in x - y plane by spinning on x - axis
		        // the rotate D points on x - axis to form E points
		        // check to see if on x - axis already
		        double angle3 = Math.Atan2 (D [2].v.z, D [2].v.y); 
		        for (i = 0; i < 8; i++) 
			        E [i].Set (D [i].v.x, D [i].v.y * Math.Cos (angle3) + D [i].v.z * Math.Sin (angle3), D [i].v.z * Math.Cos (angle3) - D [i].v.y * Math.Sin (angle3)); 

		        // now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
		        // mirror new points on z axis
		        for (i = 4; i < 8; i++)
			        E [i].v.z = -E [i].v.z; 
		        // now reverse rotations
		        angle3 = -angle3;
		        for (i = 4; i < 8; i++) 
			        D [i].Set (E [i].v.x, E [i].v.y * Math.Cos (angle3) + E [i].v.z * Math.Sin (angle3), E [i].v.z * Math.Cos (angle3) - E [i].v.y * Math.Sin (angle3)); 
		        angle2 = -angle2;
		        for (i = 4; i < 8; i++) 
			        C [i].Set (D [i].v.x * Math.Cos (angle2) + D [i].v.y * Math.Sin (angle2), D [i].v.y * Math.Cos (angle2) - D [i].v.x * Math.Sin (angle2), D [i].v.z); 
		        angle1 = -angle1;
		        for (i = 4; i < 8; i++) 
			        B [i].Set (C [i].v.x, C [i].v.y * Math.Cos (angle1) + C [i].v.z * Math.Sin (angle1), C [i].v.z * Math.Cos (angle1) - C [i].v.y * Math.Sin (angle1)); 

		        // and translate back
		        nVertex = curSeg.m_verts [GameTables.sideVertTable [DLE.Current.m_nSide,0]]; 
		        for (i = 4; i < 8; i++) 
			        A [i] = B [i] + DLE.Vertices [nVertex]; 

		        for (i = 0; i < 4; i++) 
                {
			        nVertex = newVerts [i];
			        DLE.Vertices [nVertex].Set (A [i + 4]); 
		        }
	        }
        }

        // ------------------------------------------------------------------------

        short Create ()
        {
	        Segment     newSeg, curSeg; 
	        int		    i;
	        short	    nNewSeg, nNewSide, nCurSide = DLE.Current.m_nSide; 
	        ushort[]	newVerts = new ushort [4]; 
	        short	    nSide; 

        if (DLE.TunnelMaker.Active)
	        return -1; 

        if (Full) {
	        DLE.ErrorMsg (@"Cannot add a new segment because\nthe maximum number of segments has been reached."); 
	        return -1;
	        }
        if (DLE.Vertices.Full) {
	        DLE.ErrorMsg (@"Cannot add a new segment because\nthe maximum number of vertices has been reached."); 
	        return -1;
	        }

        curSeg = DLE.Current.Segment; 

        if (curSeg.GetChild (nCurSide) >= 0) {
	        DLE.ErrorMsg (@"Can not add a new segment to a side\nwhich already has a segment attached."); 
	        return -1;
	        }

        DLE.Backup.Begin (UndoData.Flags.udSegments); 
        // get new segment
        m_bCreating = true;
        nNewSeg = Add (); 
        newSeg = Segments [nNewSeg]; 

        DLE.Vertices.Add (newVerts, 4);
        // define vert numbers for common side
        for (i = 0; i < 4; i++) {
	        newSeg.m_verts [GameTables.oppSideVertTable [nCurSide,i]] = curSeg.m_verts [GameTables.sideVertTable [nCurSide,i]]; 
	        // define vert numbers for new side
	        newSeg.m_verts [GameTables.sideVertTable [nCurSide,i]] = newVerts [i]; 
	        }

        ComputeVertices (newVerts); 
        newSeg.Setup ();
        // define children and special child
        newSeg.m_childFlags = (byte) (1 << GameTables.oppSideTable [nCurSide]); // only opposite side connects to current_segment 
        for (i = 0; i < 6; i++) // no remaining children
	        newSeg.SetChild ((short)i, (short)(((newSeg.m_childFlags & (1 << i)) != 0) ? DLE.Current.m_nSegment : -1));

        // define textures
        for (nSide = 0; nSide < 6; nSide++) {
	        if (newSeg.GetChild (nSide) < 0) {
		        // if other segment does not have a child (therefore it has a texture)
		        if ((curSeg.GetChild (nSide) < 0) && (curSeg.m_function == Segment.Functions.NONE)) {
			        newSeg.m_sides [nSide].m_nBaseTex = curSeg.m_sides [nSide].m_nBaseTex; 
			        newSeg.m_sides [nSide].m_nOvlTex = curSeg.m_sides [nSide].m_nOvlTex; 
			        for (i = 0; i < 4; i++) 
				        newSeg.m_sides [nSide].m_uvls [i].l = curSeg.m_sides [nSide].m_uvls [i].l; 
			        } 
		        }
	        else {
		        newSeg.m_sides [nSide].ClearUVL ();
		        }
	        }

        // define static light
        newSeg.m_staticLight = curSeg.m_staticLight; 

        // delete variable light if it exists
        DLE.Lights.DeleteVariableLight (new SideKey (DLE.Current.m_nSegment, nCurSide)); 

        // update current segment
        curSeg.SetChild (nCurSide, nNewSeg); 
        Side side = curSeg.m_sides [nCurSide];
        side.m_nBaseTex = 0; 
        side.m_nOvlTex = 0;
        side.ClearUVL ();
 
        // link the new segment with any touching Segment ()
        Vertex vNewSeg = DLE.Vertices [newSeg.m_verts [0]];
        for (i = 0; i < Count; i++)
        {
            if (i != nNewSeg)
            {
                // first check to see if Segment () are any where near each other
                // use x, y, and z coordinate of first point of each segment for comparison
                Vertex vSeg = DLE.Vertices [Segments [i].m_verts [0]];
                if ((Math.Abs (vNewSeg.v.x - vSeg.v.x) < 160.0) &&
                    (Math.Abs (vNewSeg.v.y - vSeg.v.y) < 160.0) &&
                    (Math.Abs (vNewSeg.v.z - vSeg.v.z) < 160.0))
                {
                    for (nNewSide = 0; nNewSide < 6; nNewSide++)
                        for (nSide = 0; nSide < 6; nSide++)
                            Link (nNewSeg, nNewSide, (short) i, nSide, 3);
                }
            }
        }
        // auto align textures new segment
        for (nNewSide = 0; nNewSide < 6; nNewSide++)
	        AlignTextures (DLE.Current.m_nSegment, nNewSide, nNewSeg, true, true); 
        // set current segment to new segment
        DLE.Current.m_nSegment = nNewSeg; 
        //		SetLinesToDraw(); 
        DLE.MineView.Refresh (false); 
        DLE.ToolView.Refresh (); 
        DLE.Backup.End ();
        m_bCreating = false;
        return nNewSeg; 
        }

        // ------------------------------------------------------------------------

        short Create (short nSegment, bool bCreate, Segment.Functions nFunction, short nTexture = -1, string szError = null)
        {
            if ((szError != null) && DLE.IsD1File)
            {
                if (!DLE.ExpertMode)
                    DLE.ErrorMsg (szError);
                return 0;
            }

            DLE.Backup.Begin (UndoData.Flags.udSegments);
            if (bCreate)
            {
                if (DLE.Current.Child >= 0)
                {
                    DLE.Backup.End ();
                    return -1;
                }
                nSegment = Create ();
                if (nSegment < 0)
                {
                    Remove (nSegment);
                    DLE.Backup.End ();
                    return -1;
                }
            }
            DLE.MineView.DelayRefresh (true);
            m_bCreating = true;
            if (!Define (nSegment, nFunction, -1))
            {
                if (bCreate)
                    Remove (nSegment);
                DLE.Backup.End ();
                DLE.MineView.DelayRefresh (false);
                m_bCreating = false;
                return -1;
            }
            m_bCreating = false;
            DLE.Backup.End ();
            DLE.MineView.DelayRefresh (false);
            DLE.MineView.Refresh ();
            return nSegment;
        }

        // ------------------------------------------------------------------------

        bool SetDefaultTexture (short nTexture)
        {
            if (nTexture < 0)
	            return true;

            short nSegment = DLE.Current.m_nSegment;
            Segment seg = Segments [nSegment];

            double scale = DLE.Textures.Textures [nTexture].Scale (nTexture);

            DLE.Backup.Begin (UndoData.Flags.udSegments);
            seg.m_childFlags |= (1 << 6);
            // set textures
            for (short nSide = 0; nSide < 6; nSide++) 
                {
                Side side = seg.m_sides [nSide];
	            if (seg.GetChild (nSide) == -1) {
		            SetTextures (new SideKey (nSegment, nSide), (ushort) nTexture, 0);
		            for (int i = 0; i < 4; i++) {
			            side.m_uvls [i].u = (short) ((double) GameTables.defaultUVLs [i].u / scale);
			            side.m_uvls [i].v = (short) ((double) GameTables.defaultUVLs [i].v / scale);
			            side.m_uvls [i].l = GameTables.defaultUVLs [i].l;
			            }
		            Segments [nSegment].SetUV (nSide, 0, 0);
		            }
	            }
            DLE.Backup.End ();
            return true;
        }

        // ------------------------------------------------------------------------

        bool Define (short nSegment, Segment.Functions nFunction, short nTexture)
        {
            DLE.Backup.Begin (UndoData.Flags.udSegments);
            Segment seg = (nSegment < 0) ? DLE.Current.Segment : Segments [nSegment];
            Undefine ((short) seg.Key);
            seg.m_function = nFunction;
            seg.m_childFlags |= (1 << 6);
            SetDefaultTexture (nTexture);
            DLE.Backup.End ();
            DLE.MineView.Refresh ();
            return true;
        }

        // ------------------------------------------------------------------------

        void Undefine (short nSegment)
        {
	        Segment seg = (nSegment < 0) ? DLE.Current.Segment : Segments [nSegment];

            nSegment = (short)seg.Key;
            if (seg.m_function == Segment.Functions.ROBOTMAKER)
                RemoveMatCenter (seg, RobotMakers, m_matCenInfo [0], Segment.Functions.ROBOTMAKER);
            else if (seg.m_function == Segment.Functions.EQUIPMAKER)
                RemoveMatCenter (seg, EquipMakers, m_matCenInfo [1], Segment.Functions.EQUIPMAKER);
            else if (seg.m_function == Segment.Functions.FUELCEN)
            { //remove all fuel cell walls
                DLE.Backup.Begin (UndoData.Flags.udSegments);
                for (short nSide = 0; nSide < 6; nSide++)
                {
                    Side side = seg.m_sides [nSide];
                    if (seg.GetChild (nSide) < 0)	// assume no wall if no child segment at the current side
                        continue;
                    Segment childSeg = Segments [seg.GetChild (nSide)];
                    if (childSeg.m_function == Segment.Functions.FUELCEN)	// don't delete if child segment is fuel center
                        continue;
                    // if there is a wall and it's a fuel cell delete it
                    SideKey key = new SideKey (nSegment, nSide);
                    Wall wall = Wall (key);
                    if ((wall != null) && (wall.Type == global::DLE.NET.Wall.Types.ILLUSION) && (side.m_nBaseTex == (DLE.IsD1File ? 322 : 333)))
                        DLE.Walls.Delete ((short)side.m_nWall);
                    // if there is a wall at the opposite side and it's a fuel cell delete it
                    SideKey opp = new SideKey (-1, -1);
                    if (OppositeSide (key, opp) != null)
                    {
                        wall = Wall (opp);
                        if ((wall != null) && (wall.Type == global::DLE.NET.Wall.Types.ILLUSION))
                        {
                            Side oppSide = Side (opp);
                            if (oppSide.m_nBaseTex == (DLE.IsD1File ? 322 : 333))
                                DLE.Walls.Delete ((short)oppSide.m_nWall);
                        }
                    }
                }
                DLE.Backup.End ();
            }
            unchecked
            {
                seg.m_childFlags &= (byte)~(1 << 6);
            }
            seg.m_function = Segment.Functions.NONE;
        }

        // ------------------------------------------------------------------------

        bool CreateMatCen (short nSegment, bool bCreate, Segment.Functions nType, bool bSetDefTextures, 
	                       MatCenter[] matCens, MineItemInfo info, string szError) 
        {
        if (info.count >= GameMine.MAX_MATCENS) {
            DLE.ErrorMsg (szError);
	         return false;
	        }
        DLE.Backup.Begin (UndoData.Flags.udSegments);
        if (0 > (nSegment = Create (nSegment, bCreate, nType)))
	        return false;
        matCens [info.count].Setup (nSegment, (short) info.count, 0);
        Segments [nSegment].m_value = 
        Segments [nSegment].m_nMatCen = (sbyte) (info.count++);
        DLE.Backup.End ();
        return true;
        }

        // ------------------------------------------------------------------------

        void RemoveMatCenter (Segment seg, MatCenter [] matCens, MineItemInfo info, Segment.Functions nFunction)
        {
            if (info.count > 0) 
            {
	            // fill in deleted matcen
	            short nDelMatCen = seg.m_nMatCen;
	            if (nDelMatCen >= 0) 
                {
		            // copy last matCen in list to deleted matCen's position
		            DLE.Backup.Begin (UndoData.Flags.udSegments | UndoData.Flags.udMatCenters);
		            seg.m_nMatCen = -1;
		            seg.m_value = -1;
		            if (nDelMatCen < --info.count) 
                    {
                        MatCenter temp = matCens [nDelMatCen];
                        matCens [nDelMatCen] = matCens [info.count];
                        matCens [info.count] = temp;
                        matCens [nDelMatCen].Key = nDelMatCen;
                        matCens [info.count].Key = info.count;
			            matCens [nDelMatCen].m_nFuelCen = nDelMatCen;
			            // point owner of relocated matCen to that matCen's new position
                        for (int i = 0; i < Count; i++)
                        {
                            Segment s = Segments [i];
				            if ((s.m_function == nFunction) && (s.m_nMatCen == info.count)) {
					            s.m_nMatCen = (sbyte) nDelMatCen;
					            break;
					            }
				            }
			            }
		            // remove matCen from all robot maker triggers targetting it
		            SideKey key = new SideKey ((short) (-seg.Key - 1), 0); 
		            for (int nClass = 0; nClass < 2; nClass++) 
                    {
                        for (int j = 0; j < DLE.Triggers.Count (nClass); j++)
                        {
                            Trigger trig = DLE.Triggers [nClass,j];
				            if (trig.Type == Trigger.Types.MATCEN)
					            trig.Delete (key);
				            }
			            }
		            DLE.Backup.End ();
		            }
	            }
            seg.m_nMatCen = -1;
        }

        // ------------------------------------------------------------------------

        void Delete (short nDelSeg, bool bDeleteVerts)
        {
            if (Count < 2)
                return;
            if (nDelSeg < 0)
                nDelSeg = DLE.Current.m_nSegment;
            if (nDelSeg < 0 || nDelSeg >= Count)
                return;
            if (DLE.TunnelMaker.Active)
                return;

            DLE.Backup.Begin (UndoData.Flags.udSegments);
            Segment delSeg = Segments [nDelSeg];
            DLE.Current.Fix (nDelSeg);
            DLE.Other.Fix (nDelSeg);
            Undefine (nDelSeg);

            // delete any variable lights that use this segment
            SideKey key = new SideKey ();
            for (short nSide = 0; nSide < 6; nSide++)
            {
                key.Set (nDelSeg, nSide);
                DLE.Triggers.DeleteTargets (key);
                DLE.Lights.DeleteVariableLight (key);
            }

            // delete any Walls () within segment (if defined)
            DeleteWalls (nDelSeg);

            // delete any walls from adjacent segments' sides connecting to this segment
            for (short i = 0; i < 6; i++)
            {
                if (delSeg.GetChild (i) >= 0)
                {
                    SideKey opp = new SideKey ();
                    key.Set (nDelSeg, i);
                    if (OppositeSide (key, opp) != null)
                        DLE.Walls.Delete ((short)Side (opp).m_nWall);
                }
            }

            // delete any Objects () within segment
            DLE.Objects.DeleteSegmentObjects (nDelSeg);

            // delete any control segP with this segment
            for (short i = (short)DLE.Triggers.ReactorTriggerCount; i > 0; )
                DLE.Triggers.ReactorTriggers [--i].Delete (new SideKey ((short) (-nDelSeg - 1)));

            // update secret segment number if out of range now
            short nSegment = (short)DLE.Objects.SecretSegment;
            if (nSegment >= Count || nSegment == nDelSeg)
                DLE.Objects.SecretSegment = 0;

            // update segment flags
            delSeg.Unmark ();

            // unlink any children with this segment number
            Texture[] tex = DLE.Textures.Textures;
            for (int i = 0; i < Count; i++)
            {
                Segment seg = Segments [i];
                for (short nChild = 0; nChild < 6; nChild++)
                {
                    if (seg.GetChild (nChild) == nDelSeg)
                    {
                        // remove nChild number and update nChild bitmask
                        seg.SetChild (nChild, -1);

                        // define textures, (u, v) and light
                        Side side = delSeg.m_sides [nChild];
                        SetTextures (new SideKey ((short) i, nChild), side.BaseTex, side.OvlTex);
                        seg.SetUV (nChild, 0, 0);
                        double scale = tex [side.m_nBaseTex].Scale ((short)side.m_nBaseTex);
                        for (short j = 0; j < 4; j++)
                        {
                            //seg.m_sides [nChild].m_uvls [j].u = (short) ((double) defaultUVLs [j].u / scale); 
                            //seg.m_sides [nChild].m_uvls [j].v = (short) ((double) defaultUVLs [j].v / scale); 
                            seg.Uvls (nChild) [j].l = delSeg.Uvls (nChild) [j].l;
                        }
                    }
                }
            }
            Remove (nDelSeg);

            // delete all unused vertices
            DLE.Vertices.DeleteUnused ();

            // make sure current segment numbers are valid
            DLE.MineView.Refresh (false);
            DLE.ToolView.Refresh ();
            DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        int FuelCenterCount
        {
            get
            {
                int nFuelCens = 0;
                for (int i = 0; i < Count; i++)
                {
                    Segment.Functions function = Segments [i].m_function;
                    if ((function == Segment.Functions.FUELCEN) || (function == Segment.Functions.REPAIRCEN))
                        nFuelCens++;
                }
                return nFuelCens;
            }
        }

        // ------------------------------------------------------------------------

        bool CreateEquipMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
        {
            if (!DLE.IsD2XFile)
            {
                DLE.ErrorMsg (@"Equipment makers are only available in D2X-XL levels.");
                return false;
            }
            return CreateMatCen (nSegment, bCreate, Segment.Functions.EQUIPMAKER, bSetDefTextures, EquipMakers, m_matCenInfo [1], @"Maximum number of equipment makers reached");
        }

        // ------------------------------------------------------------------------

        bool CreateRobotMaker (short nSegment, bool bCreate, bool bSetDefTextures) 
        {
            return CreateMatCen (nSegment, bCreate, Segment.Functions.ROBOTMAKER, bSetDefTextures, RobotMakers, m_matCenInfo [0], @"Maximum number of robot makers reached");
        }

        // ------------------------------------------------------------------------

        bool CreateReactor (short nSegment, bool bCreate, bool bSetDefTextures) 
        {
            return 0 <= Create (nSegment, bCreate, Segment.Functions.REACTOR, (short) (bSetDefTextures ? DLE.IsD1File ? 10 : 357 : -1), @"Flag goals are not available in Descent 1.");
        }

        // ------------------------------------------------------------------------

        bool CreateGoal (short nSegment, bool bCreate, bool bSetDefTextures, Segment.Functions nType, short nTexture) 
        {
            return 0 <= Create (nSegment, bCreate, nType, (short) (bSetDefTextures ? nTexture : -1), @"Flag goals are not available in Descent 1.");
        }

        // ------------------------------------------------------------------------

        bool CreateTeam (short nSegment, bool bCreate, bool bSetDefTextures, Segment.Functions nType, short nTexture) 
        {
            return 0 <= Create (nSegment, bCreate, nType, (short) (bSetDefTextures ? nTexture : -1), @"Team start positions are not available in Descent 1.");
        }

        // ------------------------------------------------------------------------

        bool CreateSkybox (short nSegment, bool bCreate) 
        {
            return 0 <= Create (nSegment, bCreate, Segment.Functions.SKYBOX, -1, @"Skyboxes are not available in Descent 1.");
        }

        // ------------------------------------------------------------------------

        bool CreateSpeedBoost (short nSegment, bool bCreate) 
        {
            return 0 <= Create (nSegment, bCreate, Segment.Functions.SPEEDBOOST, -1, @"Speed boost segments are not available in Descent 1.");
        }

        // ------------------------------------------------------------------------

        public short CreateFuelCenter (short nSegment = -1, Segment.Functions nType = Segment.Functions.FUELCEN, bool bCreate = true, bool bSetDefTextures = true) 
        {
        // count number of fuel centers
        int nFuelCen = FuelCenterCount;
        if (nFuelCen >= GameMine.MAX_NUM_RECHARGERS) {
	        DLE.ErrorMsg ("Maximum number of fuel centers reached.");
	        return 0;
	        }

        Segment seg = Segments [0];

        DLE.Backup.Begin (UndoData.Flags.udSegments);
        if (nType == Segment.Functions.REPAIRCEN)
	        nSegment = Create (nSegment, bCreate, nType, (short) (bSetDefTextures ? 433 : -1), @"Repair centers are not available in Descent 1.");
        else {
	        short nLastSeg = DLE.Current.m_nSegment;
	        nSegment = Create (nSegment, bCreate, nType, (short) (bSetDefTextures ? DLE.IsD1File ? 322 : 333 : -1));
	        if (nSegment < 0)
		        return -1;
	        if (bSetDefTextures) { // add energy spark walls to fuel center sides
		        DLE.Current.m_nSegment = nLastSeg;
		        if (DLE.Walls.Create (DLE.Current, global::DLE.NET.Wall.Types.ILLUSION, 0, global::DLE.NET.Wall.KeyTypes.NONE, -1, -1) != null) {
			        SideKey opp = new SideKey ();
			        if (OppositeSide (null, opp) != null)
                        DLE.Walls.Create (opp, global::DLE.NET.Wall.Types.ILLUSION, 0, global::DLE.NET.Wall.KeyTypes.NONE, -1, -1);
			        }
		        DLE.Current.m_nSegment = nSegment;
		        }
	        }
        DLE.Backup.End ();
        return nSegment;
        }

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

    }
}
