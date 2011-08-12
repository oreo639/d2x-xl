using System;
using System.IO;

namespace DLE.NET
{
    public partial class Segment : IGameItem
    {

        public int Key { get; set; }

        // ------------------------------------------------------------------------

        public const int MAX_SIDES_PER_SEGMENT = 6;
        public const int MAX_VERTICES_PER_SEGMENT = 8;

        public ushort [] m_verts = new ushort [MAX_VERTICES_PER_SEGMENT];	// vertex ids of 4 front and 4 back vertices 
        public Functions m_function;			// special property of a segment (such as damaging, trigger, etc.) 
        public Properties m_props;
        public sbyte m_nMatCen;			// which center segment is associated with, high bit set 
        public sbyte m_value;				// matcens: bitmask of producable robots, fuelcenters: energy given? --MK, 3/15/95 
        public byte m_flags;			// New for Descent 2
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

        Functions[] segFuncFromType = new Functions[(int) Type.COUNT_D2]
        {
	        Functions.NONE,
	        Functions.FUELCEN,
	        Functions.REPAIRCEN,
	        Functions.CONTROLCEN,
	        Functions.ROBOTMAKER,
	        Functions.GOAL_BLUE,
	        Functions.GOAL_RED,
	        Functions.NONE,
	        Functions.NONE,
	        Functions.TEAM_BLUE,
	        Functions.TEAM_RED,
	        Functions.SPEEDBOOST,
	        Functions.NONE,
	        Functions.NONE,
	        Functions.SKYBOX,
	        Functions.EQUIPMAKER,
	        Functions.NONE
	    };

        Properties [] segPropsFromType = new Properties [(int)Type.COUNT_D2] 
        {
	        Properties.NONE,
	        Properties.NONE,
	        Properties.NONE,
	        Properties.NONE,
	        Properties.NONE,
	        Properties.NONE,
	        Properties.NONE,
	        Properties.WATER,
	        Properties.LAVA,
	        Properties.NONE,
	        Properties.NONE,
	        Properties.NONE,
	        Properties.BLOCKED,
	        Properties.NODAMAGE,
	        Properties.BLOCKED,
	        Properties.NONE,
	        Properties.OUTDOORS
        };
        private int i;

        //------------------------------------------------------------------------------

        public Segment (int key = 0)
        {
            Key = key;
            Setup ();
        }

        //------------------------------------------------------------------------------

        public void Read (BinaryReader fp, int version = 0, bool bFlag = false)
        {
            for (int i = 0; i < m_verts.Length; i++)
                m_verts [i] = fp.ReadUInt16 ();
            m_function = (Functions) fp.ReadByte ();
            m_props = (Properties) fp.ReadByte ();
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

        public void Write (BinaryWriter fp, int version = 0, bool bFlag = false)
        {
            fp.Write ((byte) m_function);
            fp.Write ((byte) m_props);
            fp.Write (m_nMatCen);
            fp.Write (m_value);
        }

        //------------------------------------------------------------------------------

        public void Clear ()
        {
            m_function = 0;
            m_props = 0;
            m_nMatCen = -1;
            m_value = 0;
            m_flags = 0;
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

        void Upgrade ()
        {
        m_props = segPropsFromType [(int) m_function];
        m_function = segFuncFromType [(int) m_function];
        m_damage [0] =
        m_damage [1] = 0;
        }

        // ------------------------------------------------------------------------

        byte ReadWalls (BinaryReader fp, int nLevelVersion)
        {
	        byte wallFlags = fp.ReadByte ();
	        int	i;

        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	        if ((m_wallFlags & (1 << i)) != 0)
		        m_sides [i].m_nWall = (nLevelVersion >= 13) ? fp.ReadUInt16 () : (ushort) fp.ReadByte ();
        return wallFlags;
        }

        //------------------------------------------------------------------------------

        void ReadExtras (BinaryReader fp, int nLevelType, int nLevelVersion, bool bExtras)
        {
        if (bExtras) {
	        m_function = (Functions) fp.ReadByte ();
	        m_nMatCen = fp.ReadSByte ();
	        m_value = fp.ReadSByte ();
	        fp.ReadSByte ();
	        }
        else {
	        m_function = 0;
	        m_nMatCen = -1;
	        m_value = 0;
	        }
        m_flags = 0;  
        if (nLevelType != 0) {
	        if (nLevelVersion < 20)
		        Upgrade ();
	        else {
		        m_props = (Properties) fp.ReadByte ();
		        m_damage [0] = fp.ReadInt16 ();
		        m_damage [1] = fp.ReadInt16 ();
		        }
	        }
        m_staticLight = fp.ReadInt32 ();
        }

        //------------------------------------------------------------------------------

        int Read (BinaryReader fp, int nLevelType, int nLevelVersion)
        {
	        short	i;

        if (nLevelVersion >= 9) {
	        m_owner = fp.ReadSByte ();
	        m_group = fp.ReadSByte ();
	        }
        else {
	        m_owner = -1;
	        m_group = -1;
	        }
        // read in child mask (1 byte)
        m_childFlags = fp.ReadByte ();

        // read 0 to 6 children (0 to 12 bytes)
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)
	        m_sides [i].m_nChild = ((m_childFlags & (1 << i)) != 0) ? fp.ReadInt16 () : (short) -1;

        // read vertex numbers (16 bytes)
        for (i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	        m_verts [i] = fp.ReadInt16 ();

        if (nLevelVersion == 0)
	        ReadExtras (fp, nLevelType, nLevelVersion, (m_childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

        // read the wall bit mask
        m_wallFlags = fp.ReadByte ();

        // read in wall numbers (0 to 6 bytes)
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	        m_sides [i].m_nWall = ((m_wallFlags & (1 << i)) != 0)
						          ? (ushort) ((nLevelVersion < 13) ? fp.ReadSByte () : fp.ReadInt16 ())
								  : GameMine.NO_WALL;

        // read in textures and uvls (0 to 60 bytes)
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  
	        m_sides [i].Read (fp, (GetChild (i) == -1) || ((m_wallFlags & (1 << i)) != 0));
        return 1;
        }

        //------------------------------------------------------------------------------

        byte WriteWalls (BinaryWriter fp, int nLevelVersion)
        {
	        int	i;

        m_wallFlags = 0;
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	        if (m_sides [i].m_nWall < DLE.Mine.Info.walls.count) 
		        m_wallFlags |= (byte) (1 << i);
	        }
        fp.Write (m_wallFlags);

        // write wall numbers
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	        if ((m_wallFlags & (1 << i)) != 0) {
		        if (nLevelVersion >= 13)
			        fp.Write (m_sides [i].m_nWall);
		        else
			        fp.Write ((sbyte) m_sides [i].m_nWall);
		        }
	        }
        return m_wallFlags;
        }

        //------------------------------------------------------------------------------

        void WriteExtras (BinaryWriter fp, int nLevelType, bool bExtras)
        {
        if (bExtras) {
	        fp.Write ((byte) m_function);
	        fp.Write (m_nMatCen);
	        fp.Write (m_value);
	        fp.Write (m_flags);
	        }
        if (nLevelType == 2) {
	        fp.Write ((byte) m_props);
	        fp.Write (m_damage [0]);
	        fp.Write (m_damage [1]);
	        }
        fp.Write (m_staticLight);
        }

        //------------------------------------------------------------------------------

        void Write (BinaryWriter fp, int nLevelType, int nLevelVersion)
        {
	        short i;

        if (nLevelType == 2) {
	        fp.Write (m_owner);
	        fp.Write (m_group);
	        }

        m_childFlags = 0;
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
	        if (GetChild (i) != -1) {
		        m_childFlags |= (byte) (1 << i);
		        }
	        }
        if (nLevelType == 0) {
	        if (m_function != 0) { // if this is a special cube
		        m_childFlags |= (1 << MAX_SIDES_PER_SEGMENT);
		        }
	        }
        fp.Write (m_childFlags);

        // write children numbers (0 to 6 bytes)
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) 
	        if ((m_childFlags & (1 << i)) != 0)
		        fp.Write (GetChild (i));

        // write vertex numbers (16 bytes)
        for (i = 0; i < MAX_VERTICES_PER_SEGMENT; i++)
	        fp.Write (m_verts [i]);

        // write special info (0 to 4 bytes)
        if ((m_function == Functions.ROBOTMAKER) && (m_nMatCen == -1)) {
	        m_function = Functions.NONE;
	        m_value = 0;
	        m_childFlags = (byte) ((int) m_childFlags & ~(1 << MAX_SIDES_PER_SEGMENT));
	        }
        if (nLevelType == 0)
	        WriteExtras (fp, nLevelType, (m_childFlags & (1 << MAX_SIDES_PER_SEGMENT)) != 0);

        // calculate wall bit mask
        WriteWalls (fp, nLevelVersion);
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++)  
	        if ((GetChild (i) == -1) || ((m_wallFlags & (1 << i)) != 0))
		        m_sides [i].Write (fp);
        }

        //------------------------------------------------------------------------------

        public void Setup ()
        {
	        short i;

        m_owner = -1;
        m_group = -1;
        m_function = 0; 
        m_nMatCen = -1; 
        m_value = -1; 
        m_childFlags = 0;
        m_wallFlags = 0; 
        for (i = 0; i < MAX_SIDES_PER_SEGMENT; i++) {
            m_sides [i] = new Side ();
	        m_sides [i].Setup (); 
	        SetUV (i, 0, 0); 
	        SetChild (i, -1);
	        }
        m_staticLight = 0; 
        }

        // ------------------------------------------------------------------------

        public void SetUV (short nSide, short x, short y)
        {
	        DoubleVector[]	A = new DoubleVector [4], 
                            B = new DoubleVector [4], 
                            C = new DoubleVector [4], 
                            D = new DoubleVector [4], 
                            E = new DoubleVector [4]; 
	        int				i; 
	        double			angle, sinAngle, cosAngle; 

            // copy side's four points into A
            for (i = 0; i < 4; i++)
	            A [i] = DLE.Vertices [m_verts [GameTables.sideVertTable [nSide,i]]] as DoubleVector;

            // subtract point 0 from all points in A to form B points
            for (i = 0; i < 4; i++) 
	            B [i] = A [i] - A [0]; 

            // calculate angle to put point 1 in x - y plane by spinning on x - axis
            // then rotate B points on x - axis to form C points.
            // check to see if on x - axis already
            angle = Math.Atan2 (B [1].v.z, B [1].v.y); 
            sinAngle = Math.Sin (angle);
            cosAngle = Math.Cos (angle);
            for (i = 0; i < 4; i++) 
	            C [i] = new DoubleVector (B [i].v.x, B [i].v.y * cosAngle + B [i].v.z * sinAngle, -B [i].v.y * sinAngle + B [i].v.z * cosAngle); 

            // calculate angle to put point 1 on x axis by spinning on z - axis
            // then rotate C points on z - axis to form D points
            // check to see if on z - axis already
            angle = Math.Atan2 (C [1].v.y, C [1].v.x); 
            sinAngle = Math.Sin (angle);
            cosAngle = Math.Cos (angle);
            for (i = 0; i < 4; i++)
                D [i] = new DoubleVector (C [i].v.x * cosAngle + C [i].v.y * sinAngle, -C [i].v.x * sinAngle + C [i].v.y * cosAngle, C [i].v.z); 

            // calculate angle to put point 2 in x - y plane by spinning on x - axis
            // the rotate D points on x - axis to form E points
            // check to see if on x - axis already
            angle = Math.Atan2 (D [2].v.z, D [2].v.y); 
            for (i = 0; i < 4; i++)
                E [i] = new DoubleVector (D [i].v.x, D [i].v.y * Math.Cos (angle) + D [i].v.z * Math.Sin (angle), -D [i].v.y * Math.Sin (angle) + D [i].v.z * Math.Cos (angle)); 

            // now points 0, 1, and 2 are in x - y plane and point 3 is close enough.
            // set v to x axis and u to negative u axis to match default (u, v)
            // (remember to scale by dividing by 640)
            UVL[] uvls = m_sides [nSide].m_uvls;
            DLE.Modified = true; 
            double scale = 1.0; //textureManager.Textures () [m_fileType][sideP->m_nBaseTex].Scale (sideP->m_nBaseTex);
            for (i = 0; i < 4; i++) 
            {
	            uvls [i].v = (short) ((y + FixConverter.D2X (E [i].v.x / 640)) / scale); 
	            uvls [i].u = (short) ((x - FixConverter.D2X (E [i].v.y / 640)) / scale); 
	        }
        }

        //------------------------------------------------------------------------------

        public Vertex Vertex (int i)
        {
            return DLE.Vertices [i];
        }

        //------------------------------------------------------------------------------

        public void UpdateChildren (short nOldChild, short nNewChild)
        {
        for (short nSide = 0; nSide < 6; nSide++)
	        if (m_sides [nSide].UpdateChild (nOldChild, nNewChild))	// no two sides can have the same child
		        return;
        }

        //------------------------------------------------------------------------------

        public void Reset (short nSide)
        {
        m_group = -1;
        m_owner = -1;
        m_function = 0;
        m_value = -1;
        m_nMatCen = -1;
        m_wallFlags = 0;
        m_flags = 0;
        SetChild (nSide, -1); 
        m_childFlags &= (byte) ~(1 << nSide); 
        if (nSide < 0) {
	        for (nSide = 0; nSide < 6; nSide++)
		        m_sides [nSide].Reset ();
	        }	
        else
	        m_sides [nSide].Reset ();
        }

        //------------------------------------------------------------------------------

    }
}
