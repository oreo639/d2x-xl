using System;

namespace DLE.NET
{
    public partial class SegmentManager
    {
        // ------------------------------------------------------------------------

        public void Textures (SideKey key, out short nBaseTex, out short nOvlTex)
        {
        DLE.Current.Get (key);
        Side (key).GetTextures (out nBaseTex, out nOvlTex);
        }

        // ------------------------------------------------------------------------

        public bool SetTextures (SideKey key, ushort nBaseTex, ushort nOvlTex)
        {
	        bool bChange = false;

        DLE.Backup.Begin (UndoData.Flags.udSegments); 
        DLE.Current.Get (key); 
        Side side = Side (key);
        bChange = side.SetTextures ((ushort)nBaseTex, (ushort)nOvlTex);
        if (!bChange) {
	        DLE.Backup.End ();
	        return false;
	        }
        if ((DLE.Lights.IsLight (side.m_nBaseTex) == -1) && (DLE.Lights.IsLight (side.m_nOvlTex & 0x3fff) == -1))
	        DLE.Lights.DeleteVariableLight (key); 
        if (!DLE.Walls.ClipFromTexture (key))
	        DLE.Walls.CheckForDoor (key); 
        DLE.Backup.End (); 
        //sprintf_s (message, sizeof (message), "side has textures %d, %d", side.m_nBaseTex & 0x1fff, side.m_nOvlTex & 0x1fff); 
        //DLE.InfoMsg (message); 
        return true;
        }

        // ------------------------------------------------------------------------

        static short [,] sideChildTable = new short [6, 4] {
		        {4, 3, 5, 1}, //{5, 1, 4, 3}, 
		        {2, 4, 0, 5}, //{5, 0, 4, 2}, 
		        {5, 3, 4, 1}, //{5, 3, 4, 1}, 
		        {0, 4, 2, 5}, //{5, 0, 4, 2}, 
		        {2, 3, 0, 1}, //{2, 3, 0, 1}, 
		        {0, 3, 2, 1} //{2, 3, 0, 1}
		        };

        public int AlignTextures (short nStartSeg, short nStartSide, short nOnlyChildSeg, bool bAlign1st, bool bAlign2nd, sbyte bAlignedSides = 0)
        {
	        Segment	    seg = Segments [nStartSeg]; 
	        Segment	    childSeg; 
	        Side		side = seg.m_sides [nStartSide]; 
	        Side		childSide; 

	        int			return_code = -1; 
	        short		i; 
	        short		nSide, nChildSeg, nLine; 
	        ushort		point0, point1, vert0, vert1; 
	        short		nChildSide, nChildLine; 
	        ushort		nChildPoint0, nChildPoint1, nChildVert0, nChildVert1; 
	        short		u0, v0; 
	        double		sAngle, cAngle, angle, length; 

        DLE.Backup.Begin (UndoData.Flags.udSegments);
        for (nLine = 0; nLine < 4; nLine++) {
	        // find vert numbers for the line's two end points
	        point0 = GameTables.lineVertTable [GameTables.sideLineTable [nStartSide,nLine],0]; 
	        point1 = GameTables.lineVertTable [GameTables.sideLineTable [nStartSide,nLine],1]; 
	        vert0 = seg.m_verts [point0]; 
	        vert1 = seg.m_verts [point1]; 
	        // check child for this line
	        if (nStartSeg == nOnlyChildSeg) {
		        nSide = nStartSide;
		        nChildSeg = nStartSeg;
		        }
	        else {
		        nSide = sideChildTable [nStartSide,nLine]; 
		        nChildSeg = seg.GetChild (nSide); 
		        }
	        if ((nChildSeg < 0) || ((nOnlyChildSeg != -1) && (nChildSeg != nOnlyChildSeg)))
		        continue;
	        childSeg = Segments [nChildSeg]; 
	        // figure out which side of child shares two points w/ nStartSide
	        for (nChildSide = 0; nChildSide < 6; nChildSide++) {
		        if ((nStartSeg == nOnlyChildSeg) && (nChildSide == nStartSide))
			        continue;
		        if ((bAlignedSides & (sbyte) (1 << nChildSide)) != 0)
			        continue;
		        // ignore children of different textures (or no texture)
		        if (!IsWall (new SideKey (nChildSeg, nChildSide)))
			        continue;
		        if (childSeg.m_sides [nChildSide].m_nBaseTex != side.m_nBaseTex)
			        continue;
		        for (nChildLine = 0; nChildLine < 4; nChildLine++) {
			        // find vert numbers for the line's two end points
                    nChildPoint0 = GameTables.lineVertTable [GameTables.sideLineTable [nChildSide,nChildLine],0];
                    nChildPoint1 = GameTables.lineVertTable [GameTables.sideLineTable [nChildSide,nChildLine],1]; 
			        nChildVert0  = childSeg.m_verts [nChildPoint0]; 
			        nChildVert1  = childSeg.m_verts [nChildPoint1]; 
			        // if points of child's line== corresponding points of parent
			        if (!((nChildVert0 == vert1 && nChildVert1 == vert0) ||
					        (nChildVert0 == vert0 && nChildVert1 == vert1)))
				        continue;
			        // now we know the child's side & line which touches the parent
			        // child:  nChildSeg, nChildSide, nChildLine, nChildPoint0, nChildPoint1
			        // parent: nStartSeg, nStartSide, nLine, point0, point1
			        childSide = childSeg.m_sides [nChildSide]; 
			        if (bAlign1st) {
				        // now translate all the childs (u, v) coords so child_point1 is at zero
				        u0 = childSide.m_uvls [(nChildLine + 1) % 4].u; 
				        v0 = childSide.m_uvls [(nChildLine + 1) % 4].v; 
				        for (i = 0; i < 4; i++) {
					        childSide.m_uvls [i].u -= u0; 
					        childSide.m_uvls [i].v -= v0; 
					        }
				        // find difference between parent point0 and child point1
                        u0 = (short) (childSide.m_uvls [(nChildLine + 1) % 4].u - side.m_uvls [nLine].u);
                        v0 = (short) (childSide.m_uvls [(nChildLine + 1) % 4].v - side.m_uvls [nLine].v); 
				        // find the angle formed by the two lines
				        sAngle = Math.Atan2((double) (side.m_uvls [(nLine + 1) % 4].v - side.m_uvls [nLine].v), 
									        (double) (side.m_uvls [(nLine + 1) % 4].u - side.m_uvls [nLine].u)); 
				        cAngle = Math.Atan2((double) (childSide.m_uvls [nChildLine].v - childSide.m_uvls [(nChildLine + 1) % 4].v), 
									        (double) (childSide.m_uvls [nChildLine].u - childSide.m_uvls [(nChildLine + 1) % 4].u)); 
				        // now rotate childs (u, v) coords around child_point1 (cangle - sangle)
				        for (i = 0; i < 4; i++) {
					        angle = Math.Atan2((double) childSide.m_uvls [i].v, (double) childSide.m_uvls [i].u); 
					        length = Math.Sqrt((double) childSide.m_uvls [i].u * (double) childSide.m_uvls [i].u +
									          (double) childSide.m_uvls [i].v * (double) childSide.m_uvls [i].v); 
					        angle -= (cAngle - sAngle); 
					        childSide.m_uvls [i].u = (short)(length * Math.Cos (angle)); 
					        childSide.m_uvls [i].v = (short)(length * Math.Sin (angle)); 
					        }
				        // now translate all the childs (u, v) coords to parent point0
				        for (i = 0; i < 4; i++) {
					        childSide.m_uvls [i].u -= u0; 
					        childSide.m_uvls [i].v -= v0; 
					        }
				        // modulo points by 0x800 (== 64 pixels)
				        u0 = (short) (childSide.m_uvls [0].u / 0x800); 
				        v0 = (short) (childSide.m_uvls [0].v / 0x800); 
				        for (i = 0; i < 4; i++) {
					        childSide.m_uvls [i].u -= (short) (u0 * 0x800); 
					        childSide.m_uvls [i].v -= (short) (v0 * 0x800); 
					        }
				        if (nOnlyChildSeg != -1)
					        return_code = nChildSide; 
				        }
			        if (bAlign2nd && (side.m_nOvlTex != 0) && (childSide.m_nOvlTex != 0)) {
                        int r;
                        int h = side.m_nOvlTex & 0xC000;
                        if (h == 0xC000)
						    r = 1;
					    else if (h == 0x8000)
						    r = 2;
					    else if (h == 0x4000)
						    r = 3;
						else
                            r = 0;
                        angle = Math.Atan2 ((double)childSide.m_uvls [0].v, (double)childSide.m_uvls [0].u);
                        h = (int)(MathExtensions.Degrees (Math.Abs (angle)) / 90 + 0.5); 
        //				h +=(nChildLine + nLine + 2) % 4; //(nChildLine > nLine) ? nChildLine - nLine : nLine - nChildLine;
				        h = (h + r) % 4;
                        unchecked
                        {
                            childSide.m_nOvlTex &= (ushort)~0xC000;
                        }
				        if (h == 1)
						    childSide.m_nOvlTex |= 0xC000;
				        else if (h == 2)
						    childSide.m_nOvlTex |= 0x8000;
				        else if (h == 3)
						    childSide.m_nOvlTex |= 0x4000;
				        }
			        break;
			        }
		        }
	        }
        DLE.Backup.End ();
        return return_code; 
        }

        // ------------------------------------------------------------------------
    }
}