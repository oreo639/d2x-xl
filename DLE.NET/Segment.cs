﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public partial class Segment : GameItem
    {

        public const int MAX_SIDES_PER_SEGMENT = 6;
        public const int MAX_VERTICES_PER_SEGMENT = 8;

        public short [] m_verts = new short [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
        public byte m_function;			// special property of a segment (such as damaging, trigger, etc.) 
        public byte m_props;
        public sbyte m_nMatCen;			// which center segment is associated with, high bit set 
        public sbyte m_value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
        public byte m_s2Flags;			// New for Descent 2
        public short [] m_damage = new short [2];
        public int m_staticLight;		// average static light in segment 
        public byte m_childFlags;			// bit0 to 5: children, bit6: unused, bit7: special 
        public byte m_wallFlags;			// bit0 to 5: door/walls, bit6: deleted, bit7: marked segment 
        public short m_nIndex;				// used for cut & paste to help link children 
        public short m_mapBitmask;		    // which lines are drawn when displaying wireframe 
        public sbyte m_owner;
        public sbyte m_group;
        public Side [] m_sides = new Side [MAX_SIDES_PER_SEGMENT];

        //------------------------------------------------------------------------------

        Function[] segFuncFromType = new Function[(int) Type.COUNT_D2]
        {
	        Function.NONE,
	        Function.FUELCEN,
	        Function.REPAIRCEN,
	        Function.CONTROLCEN,
	        Function.ROBOTMAKER,
	        Function.GOAL_BLUE,
	        Function.GOAL_RED,
	        Function.NONE,
	        Function.NONE,
	        Function.TEAM_BLUE,
	        Function.TEAM_RED,
	        Function.SPEEDBOOST,
	        Function.NONE,
	        Function.NONE,
	        Function.SKYBOX,
	        Function.EQUIPMAKER,
	        Function.NONE
	    };

        Property [] segPropsFromType = new Property [(int)Type.COUNT_D2] 
        {
	        Property.NONE,
	        Property.NONE,
	        Property.NONE,
	        Property.NONE,
	        Property.NONE,
	        Property.NONE,
	        Property.NONE,
	        Property.WATER,
	        Property.LAVA,
	        Property.NONE,
	        Property.NONE,
	        Property.NONE,
	        Property.BLOCKED,
	        Property.NODAMAGE,
	        Property.BLOCKED,
	        Property.NONE,
	        Property.OUTDOORS
        };

        //------------------------------------------------------------------------------

        public override void Read (BinaryReader fp, int version = 0, bool bFlag = false)
        {
            for (int i = 0; i < m_verts.Length; i++)
                m_verts [i] = fp.ReadInt16 ();
            m_function = fp.ReadByte ();
            m_props = fp.ReadByte ();
            m_nMatCen = fp.ReadSByte ();
            m_value = fp.ReadSByte ();
            m_damage [0] = fp.ReadInt16 ();
            m_damage [1] = fp.ReadInt16 ();
            m_staticLight = fp.ReadInt32 ();
            m_childFlags = fp.ReadByte ();
            m_wallFlags = fp.ReadByte ();
            m_mapBitmask = fp.ReadByte ();
            m_owner = fp.ReadSByte ();
            m_group = fp.ReadSByte ();
        }

        //------------------------------------------------------------------------------

        public override void Write (BinaryWriter fp, int version = 0, bool bFlag = false)
        {
            fp.Write (m_function);
            fp.Write (m_props);
            fp.Write (m_nMatCen);
            fp.Write (m_value);
        }

        //------------------------------------------------------------------------------

        public override void Clear ()
        {
            m_function = 0;
            m_props = 0;
            m_nMatCen = -1;
            m_value = 0;
            m_s2Flags = 0;
            m_damage [0] = m_damage [1] = 0;
            m_staticLight = 0;
            m_childFlags = 0;
            m_wallFlags = 0;
            m_nIndex = 0;
            m_mapBitmask = 0;
            m_owner = -1;
            m_group = -1;
            for (int i = 0; i < m_sides.Length; i++)
                m_sides [i].Clear ();
        }

        //------------------------------------------------------------------------------

        public short GetChild (short nSide)
        {
            return m_sides [nSide].m_nChild;
        }

        //------------------------------------------------------------------------------

        public short SetChild (short nSide, short nSegment)
        {
            m_sides [nSide].m_nChild = nSegment;
            if (nSegment == -1)
                m_childFlags &= (byte) ~(1 << nSide);
            else
                m_childFlags |= (byte) (1 << nSide);
            return nSegment;
        }

        //------------------------------------------------------------------------------

        byte ReadWalls (CFileManager& fp, int nLevelVersion)
        {
	        byte wallFlags = byte (fp.ReadSByte ());
	        int	i;

        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	        if (m_wallFlags &= (1 << i)) 
		        m_sides [i].m_nWall = (nLevelVersion >= 13) ? fp.ReadInt16 () : short (fp.ReadSByte ());
        return wallFlags;
        }

        //------------------------------------------------------------------------------

        void ReadExtras (CFileManager& fp, int nLevelType, int nLevelVersion, bool bExtras)
        {
        if (bExtras) {
	        m_function = fp.ReadSByte ();
	        m_nMatCen = fp.ReadSByte ();
	        m_value = fp.ReadSByte ();
	        fp.ReadSByte ();
	        }
        else {
	        m_function = 0;
	        m_nMatCen = -1;
	        m_value = 0;
	        }
        m_s2Flags = 0;  
        if (nLevelType) {
	        if (nLevelVersion < 20)
		        Upgrade ();
	        else {
		        m_props = fp.ReadSByte ();
		        m_damage [0] = fp.ReadInt16 ();
		        m_damage [1] = fp.ReadInt16 ();
		        }
	        }
        m_staticLight = fp.ReadFix ();
        }

        //------------------------------------------------------------------------------

        int Read (CFileManager& fp, int nLevelType, int nLevelVersion)
        {
	        int	i;

        if (nLevelVersion >= 9) {
	        m_owner = fp.ReadSByte ();
	        m_group = fp.ReadSByte ();
	        }
        else {
	        m_owner = -1;
	        m_group = -1;
	        }
        // read in child mask (1 byte)
        m_childFlags = byte (fp.ReadSByte ());

        // read 0 to 6 children (0 to 12 bytes)
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
	        m_sides [i].m_nChild = (m_childFlags & (1 << i)) ? fp.ReadInt16 () : -1;

        // read vertex numbers (16 bytes)
        for (int i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	        m_verts [i] = fp.ReadInt16 ();

        if (nLevelVersion == 0)
	        ReadExtras (fp, nLevelType, nLevelVersion, (m_childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

        // read the wall bit mask
        m_wallFlags = byte (fp.ReadSByte ());

        // read in wall numbers (0 to 6 bytes)
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	        m_sides [i].m_nWall = (m_wallFlags & (1 << i)) 
										        ? ushort ((nLevelVersion < 13) ? fp.ReadSByte () : fp.ReadInt16 ())
										        : NO_WALL;

        // read in textures and uvls (0 to 60 bytes)
        size_t fPos = fp.Tell ();
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  
	        m_sides [i].Read (fp, (GetChild (i) == -1) || ((m_wallFlags & (1 << i)) != 0));
        return 1;
        }

        //------------------------------------------------------------------------------

        byte WriteWalls (CFileManager& fp, int nLevelVersion)
        {
	        int	i;

        m_wallFlags = 0;
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	        if (m_sides [i].m_nWall < theMine->GameInfo ().walls.count) 
		        m_wallFlags |= (1 << i);
	        }
        fp.Write (m_wallFlags);

        // write wall numbers
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	        if (m_wallFlags & (1 << i)) {
		        if (nLevelVersion >= 13)
			        fp.Write (m_sides [i].m_nWall);
		        else
			        fp.WriteSByte ((sbyte) m_sides [i].m_nWall);
		        }
	        }
        return m_wallFlags;
        }

        //------------------------------------------------------------------------------

        void WriteExtras (CFileManager& fp, int nLevelType, bool bExtras)
        {
        if (bExtras) {
	        fp.Write (m_function);
	        fp.Write (m_nMatCen);
	        fp.Write (m_value);
	        fp.Write (m_s2Flags);
	        }
        if (nLevelType == 2) {
	        fp.Write (m_props);
	        fp.Write (m_damage [0]);
	        fp.Write (m_damage [1]);
	        }
        fp.Write (m_staticLight);
        }

        //------------------------------------------------------------------------------

        void Write (CFileManager& fp, int nLevelType, int nLevelVersion)
        {
	        int	i;

        if (nLevelType == 2) {
	        fp.Write (m_owner);
	        fp.Write (m_group);
	        }

        #if 1
        m_childFlags = 0;
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	        if (GetChild (i) != -1) {
		        m_childFlags |= (1 << i);
		        }
	        }
        if (nLevelType == 0) {
	        if (m_function != 0) { // if this is a special cube
		        m_childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
		        }
	        }
        #endif
        fp.Write (m_childFlags);

        // write children numbers (0 to 6 bytes)
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	        if (m_childFlags & (1 << i)) 
		        fp.WriteInt16 (GetChild (i));

        // write vertex numbers (16 bytes)
        for (i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	        fp.Write (m_verts [i]);

        // write special info (0 to 4 bytes)
        if ((m_function == SEGMENT_FUNC_ROBOTMAKER) && (m_nMatCen == -1)) {
	        m_function = SEGMENT_FUNC_NONE;
	        m_value = 0;
	        m_childFlags &= ~(1 << MAX_SIDES_PER_SEGMENT);
	        }
        if (nLevelType == 0)
	        WriteExtras (fp, nLevelType, (m_childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

        // calculate wall bit mask
        WriteWalls (fp, nLevelVersion);
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  
	        if ((GetChild (i) == -1) || (m_wallFlags & (1 << i))) 
		        m_sides [i].Write (fp);
        }

        //------------------------------------------------------------------------------

        void Setup (void)
        {
	        int i;

        m_owner = -1;
        m_group = -1;
        m_function = 0; 
        m_nMatCen = -1; 
        m_value = -1; 
        m_childFlags = 0;
        m_wallFlags = 0; 
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	        m_sides [i].Setup (); 
	        SetUV (i, 0, 0); 
	        SetChild (i, -1);
	        }
        m_staticLight = 0; 
        }

        // ------------------------------------------------------------------------

        void SetUV (short nSide short x, short y)
        {
	        CDoubleVector	A [4], B [4], C [4], D [4], E [4]; 
	        int				i; 
	        double			angle, sinAngle, cosAngle; 

            // copy side's four points into A
            for (i = 0; i < 4; i++)
	            A [i] = DLE.Mine.Vertices [m_verts [sideVertTable [nSide][i]]] as DoubleVector;

            // subtract point 0 from all points in A to form B points
            for (i = 0; i < 4; i++) 
	            B [i] = A [i] - A [0]; 

            // calculate angle to put point 1 in x - y plane by spinning on x - axis
            // then rotate B points on x - axis to form C points.
            // check to see if on x - axis already
            angle = atan3 (B [1].v.z, B [1].v.y); 
            sinAngle = sin (angle);
            cosAngle = cos (angle);
            for (i = 0; i < 4; i++) 
	            C [i].Set (B [i].v.x, B [i].v.y * cosAngle + B [i].v.z * sinAngle, -B [i].v.y * sinAngle + B [i].v.z * cosAngle); 

            // calculate angle to put point 1 on x axis by spinning on z - axis
            // then rotate C points on z - axis to form D points
            // check to see if on z - axis already
            angle = atan3 (C [1].v.y, C [1].v.x); 
            sinAngle = sin (angle);
            cosAngle = cos (angle);
            for (i = 0; i < 4; i++)
	            D [i].Set (C [i].v.x * cosAngle + C [i].v.y * sinAngle, -C [i].v.x * sinAngle + C [i].v.y * cosAngle, C [i].v.z); 

            // calculate angle to put point 2 in x - y plane by spinning on x - axis
            // the rotate D points on x - axis to form E points
            // check to see if on x - axis already
            angle = atan3 (D [2].v.z, D [2].v.y); 
            for (i = 0; i < 4; i++) 
	            E [i].Set (D [i].v.x, D [i].v.y * cos (angle) + D [i].v.z * sin (angle), -D [i].v.y * sin (angle) + D [i].v.z * cos (angle)); 

            // now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
            // set v to x axis and u to negative u axis to match default (u, v)
            // (remember to scale by dividing by 640)
            UVL[] uvls = m_sides [nSide].m_uvls;
            DLE.SetModified = true; 
            m_sides [nSide].LoadTextures ();
            double scale = 1.0; //textureManager.Textures () [m_fileType][sideP->m_nBaseTex].Scale (sideP->m_nBaseTex);
            for (i = 0; i < 4; i++, uvls++) 
            {
	            uvls->v = (short) ((y + D2X (E [i].v.x / 640)) / scale); 
	            uvls->u = (short) ((x - D2X (E [i].v.y / 640)) / scale); 
	        }
        }

        //------------------------------------------------------------------------------

    }
}
