using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    partial class Segment : GameItem
    {

        public const int MAX_SIDES_PER_SEGMENT = 6;
        public const int MAX_VERTICES_PER_SEGMENT = 8;

        short[]     verts = new short [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
        byte		function;			// special property of a segment (such as damaging, trigger, etc.) 
        byte		props;
        sbyte		nMatCen;			// which center segment is associated with, high bit set 
        sbyte		value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
        byte		s2Flags;			// New for Descent 2
        short[]	    damage = new short [2];
        int		    staticLight;		// average static light in segment 
        byte		childFlags;			// bit0 to 5: children, bit6: unused, bit7: special 
        byte		wallFlags;			// bit0 to 5: door/walls, bit6: deleted, bit7: marked segment 
        short		nIndex;				// used for cut & paste to help link children 
        short		mapBitmask;		    // which lines are drawn when displaying wireframe 
        sbyte		owner;
        sbyte		group;
        Side[]      sides = new Side [MAX_SIDES_PER_SEGMENT];

        //------------------------------------------------------------------------------

        public override void Read (Stream fp, int version = 0, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        public override void Write (Stream fp, int version = 0, bool bFlag = false)
        {
        }

        //------------------------------------------------------------------------------

        public override void Clear ()
        {
            function = 0;
            props = 0;
            nMatCen = -1;
            value = 0;
            s2Flags = 0;
            damage [0] = damage [1] = 0;
            staticLight = 0;
            childFlags = 0;
            wallFlags = 0;
            nIndex = 0;
            mapBitmask = 0;
            owner = -1;
            group = -1;
            for (int i = 0; i < sides.Length; i++)
                sides [i].Clear ();
        }

        //------------------------------------------------------------------------------

        public short Child (short nSide)
        {
            return sides [nSide].nChild;
        }

        //------------------------------------------------------------------------------

        public short SetChild (short nSide, short nSegment)
        {
            sides [nSide].nChild = nSegment;
            if (nSegment == -1)
                childFlags &= (byte) ~(1 << nSide);
            else
                childFlags |= (byte) (1 << nSide);
            return nSegment;
        }
    }
}
