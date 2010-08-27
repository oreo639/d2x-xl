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

        unsafe struct tSegment 
        {
            fixed short	children [MAX_SIDES_PER_SEGMENT];	// indices of 6 children segments, front, left, top, right, bottom, back 
            fixed short verts [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
            byte		function;			// special property of a segment (such as damaging, trigger, etc.) 
            byte		props;
            char		nMatCen;				// which center segment is associated with, high bit set 
            char		value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
            byte		s2_flags;			// New for Descent 2
            fixed short	damage [2];
            int		    staticLight;		// average static light in segment 
            byte		childFlags;			// bit0 to 5: children, bit6: unused, bit7: special 
            byte		wallFlags;			// bit0 to 5: door/walls, bit6: deleted, bit7: marked segment 
            short		nIndex;				// used for cut & paste to help link children 
            short		map_bitmask;		// which lines are drawn when displaying wireframe 
            char		owner;
            char		group;
        }


        public override int Read (Stream fp, int version = 0, bool bFlag = false)
        {
            return 1;
        }
        
        public override void Write (Stream fp, int version = 0, bool bFlag = false)
        {
        }

        public override void Clear ()
        {
        }
    }
}
