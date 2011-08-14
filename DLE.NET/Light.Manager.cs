using System.IO;
using System.Runtime.InteropServices;

namespace DLE.NET
{
    public partial class LightManager
    {

        public const int NUM_LIGHTS_D1 = 48;
        public const int NUM_LIGHTS_D2 = 85;

        public const int MAX_LIGHT_DELTA_INDICES_STD = 500;
        public const int MAX_LIGHT_DELTA_INDICES_D2X = 3000;
        public const int MAX_LIGHT_DELTA_VALUES_STD = 10000;
        public const int MAX_LIGHT_DELTA_VALUES_D2X = 50000;
        public const int MAX_VARIABLE_LIGHTS = 100;
        public const int LIGHT_DELTA_SCALE = 2048;	// Divide light to allow 3 bits integer, 5 bits fraction.

        //------------------------------------------------------------------------------

        struct TextureLight 
        {
            public int  nBaseTex;
            public long light;

            public TextureLight (int nBaseTex = 0, long light = 0)
            {
                this.nBaseTex = nBaseTex;
                this.light = light;
            }
        }

        //------------------------------------------------------------------------------

        readonly TextureLight[] textureLightD1 = new TextureLight [NUM_LIGHTS_D1] 
        {
	        new TextureLight (250, 0x00b333L), new TextureLight (251, 0x008000L), new TextureLight (252, 0x008000L), new TextureLight (253, 0x008000L),
	        new TextureLight (264, 0x01547aL), new TextureLight (265, 0x014666L), new TextureLight (268, 0x014666L), new TextureLight (278, 0x014cccL),
	        new TextureLight (279, 0x014cccL), new TextureLight (280, 0x011999L), new TextureLight (281, 0x014666L), new TextureLight (282, 0x011999L),
	        new TextureLight (283, 0x0107aeL), new TextureLight (284, 0x0107aeL), new TextureLight (285, 0x011999L), new TextureLight (286, 0x014666L),
	        new TextureLight (287, 0x014666L), new TextureLight (288, 0x014666L), new TextureLight (289, 0x014666L), new TextureLight (292, 0x010cccL),
	        new TextureLight (293, 0x010000L), new TextureLight (294, 0x013333L), new TextureLight (330, 0x010000L), new TextureLight (333, 0x010000L), 
	        new TextureLight (341, 0x010000L), new TextureLight (343, 0x010000L), new TextureLight (345, 0x010000L), new TextureLight (347, 0x010000L), 
	        new TextureLight (349, 0x010000L), new TextureLight (351, 0x010000L), new TextureLight (352, 0x010000L), new TextureLight (354, 0x010000L), 
	        new TextureLight (355, 0x010000L), new TextureLight (356, 0x020000L), new TextureLight (357, 0x020000L), new TextureLight (358, 0x020000L), 
	        new TextureLight (359, 0x020000L), new TextureLight (360, 0x020000L), new TextureLight (361, 0x020000L), new TextureLight (362, 0x020000L), 
	        new TextureLight (363, 0x020000L), new TextureLight (364, 0x020000L), new TextureLight (365, 0x020000L), new TextureLight (366, 0x020000L), 
	        new TextureLight (367, 0x020000L), new TextureLight (368, 0x020000L), new TextureLight (369, 0x020000L), new TextureLight (370, 0x020000L)
        };

        readonly TextureLight[] textureLightD2 = new TextureLight [NUM_LIGHTS_D2] 
        {
	        new TextureLight (235, 0x012666L), new TextureLight (236, 0x00b5c2L), new TextureLight (237, 0x00b5c2L), new TextureLight (243, 0x00b5c2L),
	        new TextureLight (244, 0x00b5c2L), new TextureLight (275, 0x01547aL), new TextureLight (276, 0x014666L), new TextureLight (278, 0x014666L),
	        new TextureLight (288, 0x014cccL), new TextureLight (289, 0x014cccL), new TextureLight (290, 0x011999L), new TextureLight (291, 0x014666L),
	        new TextureLight (293, 0x011999L), new TextureLight (295, 0x0107aeL), new TextureLight (296, 0x011999L), new TextureLight (298, 0x014666L),
	        new TextureLight (300, 0x014666L), new TextureLight (301, 0x014666L), new TextureLight (302, 0x014666L), new TextureLight (305, 0x010cccL),
	        new TextureLight (306, 0x010000L), new TextureLight (307, 0x013333L), new TextureLight (340, 0x00b333L), new TextureLight (341, 0x00b333L),
	        new TextureLight (343, 0x004cccL), new TextureLight (344, 0x003333L), new TextureLight (345, 0x00b333L), new TextureLight (346, 0x004cccL),
	        new TextureLight (348, 0x003333L), new TextureLight (349, 0x003333L), new TextureLight (353, 0x011333L), new TextureLight (356, 0x00028fL),
	        new TextureLight (357, 0x00028fL), new TextureLight (358, 0x00028fL), new TextureLight (359, 0x00028fL), new TextureLight (364, 0x010000L),
	        new TextureLight (366, 0x010000L), new TextureLight (368, 0x010000L), new TextureLight (370, 0x010000L), new TextureLight (372, 0x010000L),
	        new TextureLight (374, 0x010000L), new TextureLight (375, 0x010000L), new TextureLight (377, 0x010000L), new TextureLight (378, 0x010000L),
	        new TextureLight (380, 0x010000L), new TextureLight (382, 0x010000L), new TextureLight (383, 0x020000L), new TextureLight (384, 0x020000L),
	        new TextureLight (385, 0x020000L), new TextureLight (386, 0x020000L), new TextureLight (387, 0x020000L), new TextureLight (388, 0x020000L),
	        new TextureLight (389, 0x020000L), new TextureLight (390, 0x020000L), new TextureLight (391, 0x020000L), new TextureLight (392, 0x020000L),
	        new TextureLight (393, 0x020000L), new TextureLight (394, 0x020000L), new TextureLight (395, 0x020000L), new TextureLight (396, 0x020000L),
	        new TextureLight (397, 0x020000L), new TextureLight (398, 0x020000L), new TextureLight (404, 0x010000L), new TextureLight (405, 0x010000L),
	        new TextureLight (406, 0x010000L), new TextureLight (407, 0x010000L), new TextureLight (408, 0x010000L), new TextureLight (409, 0x020000L),
	        new TextureLight (410, 0x008000L), new TextureLight (411, 0x008000L), new TextureLight (412, 0x008000L), new TextureLight (419, 0x020000L),
	        new TextureLight (420, 0x020000L), new TextureLight (423, 0x010000L), new TextureLight (424, 0x010000L), new TextureLight (425, 0x020000L),
	        new TextureLight (426, 0x020000L), new TextureLight (427, 0x008000L), new TextureLight (428, 0x008000L), new TextureLight (429, 0x008000L),
	        new TextureLight (430, 0x020000L), new TextureLight (431, 0x020000L), new TextureLight (432, 0x00e000L), new TextureLight (433, 0x020000L),
	        new TextureLight (434, 0x020000L)
        };

        short[] blastableLightsD2 = new short [] 
        {
	        276, 278, 360, 361, 364, 366, 368,
	        370, 372, 374, 375, 377, 380, 382, 
	        420, 432, 431,  -1
	    };

        // ------------------------------------------------------------------------

        public MineItemInfo m_deltaIndexInfo = new MineItemInfo ();
        public MineItemInfo m_deltaValueInfo = new MineItemInfo ();

        VariableLight [] m_variableLights = new VariableLight [MAX_VARIABLE_LIGHTS];
        LightDeltaIndex [] m_deltaIndex = new LightDeltaIndex [MAX_LIGHT_DELTA_INDICES_D2X];
        LightDeltaValue [] m_deltaValues = new LightDeltaValue [MAX_LIGHT_DELTA_VALUES_D2X];

        GameColor [] m_faceColors = new GameColor [GameMine.SEGMENT_LIMIT * 6];
        GameColor [] m_texColors = new GameColor [TextureManager.MAX_TEXTURES_D2];
        GameColor [] m_vertexColors = new GameColor [GameMine.VERTEX_LIMIT];

        int [] m_lightMap = new int [TextureManager.MAX_TEXTURES_D2];

        int m_nCount = 0;
        bool m_bUseTexColors = true;

        // ------------------------------------------------------------------------

        public VariableLight[] VariableLights { get { return m_variableLights; } }

        // ------------------------------------------------------------------------

        public int Count
        {
            get { return m_nCount; }
            set { m_nCount = value; }
        }

        public int DeltaIndexCount
        {
            get { return m_deltaIndexInfo.count; }
            set { m_deltaIndexInfo.count = value; }
        }

        public int DeltaValueCount
        {
            get { return m_deltaValueInfo.count; }
            set { m_deltaValueInfo.count = value; }
        }

        public int DeltaIndexFileOffset
        {
            get { return m_deltaIndexInfo.offset; }
            set { m_deltaIndexInfo.offset = value; }
        }

        public int DeltaValueFileOffset
        {
            get { return m_deltaValueInfo.offset; }
            set { m_deltaValueInfo.offset = value; }
        }

        public bool UseTexColors
        {
            get { return m_bUseTexColors; }
            set { m_bUseTexColors = value; }
        }

        // ------------------------------------------------------------------------

        public LightDeltaIndex [] DeltaIndex { get { return m_deltaIndex; } }
        public LightDeltaValue [] DeltaValue { get { return m_deltaValues; } }

        // ------------------------------------------------------------------------

        public bool IsVariableLight (SideKey key)
        {
            return VariableLight (key) >= 0;
        }

        // ------------------------------------------------------------------------

        public int IsLight (int nBaseTex) 
        {
            return (m_lightMap [nBaseTex & 0x1fff] > 0) ? 0 : -1;
        }

        // ------------------------------------------------------------------------

        public bool IsExplodingLight (int nBaseTex) 
        {
            switch (nBaseTex)
            {
                case 291:
                case 292:
                case 293:
                case 294:
                case 296:
                case 297:
                case 298:
                case 299:
                    return true;
            }
            return false;
        }

        // ------------------------------------------------------------------------

        public bool IsBlastableLight (int nBaseTex) 
        {
            nBaseTex &= 0x3fff;
            if (IsExplodingLight (nBaseTex))
                return true;
            if (DLE.IsD1File)
                return false;
            for (int i = 0; blastableLightsD2 [i] >= 0; i++)
                if (blastableLightsD2 [i] == nBaseTex)
                    return true;
            return false;
        }

        // ------------------------------------------------------------------------

        public short VariableLight (SideKey key) 
        {
            DLE.Current.Get (key);
            VariableLight flP = VariableLights [0];
            int i;
            for (i = 0; i < Count; i++)
                if (VariableLights [i] == key)
                    break;
            if (i > 0)
                return (short) (Count - i);
            return -1;
        }

        // ------------------------------------------------------------------------

        VariableLight AddVariableLight (short index = -1) 
        {
        if (Count >= MAX_VARIABLE_LIGHTS) {
	        if (!DLE.ExpertMode && (index < 0)) {
		        DLE.ErrorMsg (string.Format ("Maximum number of variable lights ({0}) have already been added", MAX_VARIABLE_LIGHTS));
		        }
	        return null;
	        }

        short nBaseTex, nOvlTex;
        DLE.Current.Side.GetTextures (out nBaseTex, out nOvlTex);
        if ((IsLight (nBaseTex) == -1) && (IsLight (nOvlTex) == -1)) {
	        if (!DLE.ExpertMode && (index < 0))
		        DLE.ErrorMsg (@"Blinking lights can only be added to a side\n
					          that has a light emitting texture.\n
					          Hint: You can use the texture tool's brightness control\n
					          to make any texture emit light.");
	        return null;
	        }

        if (index < 0)
	        index = (short)Count;
        Count++;
        return VariableLights [index];
        }

        // ------------------------------------------------------------------------

        short AddVariableLight (SideKey key, uint mask, int time) 
        {
        DLE.Current.Get (key);
        if (VariableLight (key) != -1) {
	        if (!DLE.ExpertMode)
		        DLE.ErrorMsg (@"There is already a variable light on this side");
	        return -1;
	        }
        // we are adding a new variable light
        DLE.Backup.Begin (UndoData.Flags.udVariableLights);
        VariableLight light = AddVariableLight ();
        if (light == null) {
	        DLE.Backup.End ();
	        return -1;
	        }
        light.Setup (key, time, mask);
        DLE.Backup.End ();
        return (short) Count;
        }

        // ------------------------------------------------------------------------

        public bool DeleteVariableLight (SideKey key) 
        {
        DLE.Current.Get (key);
        short index = VariableLight (key);
        if (index == -1)
	        return false;
        DeleteVariableLight (index);
        return true;
        }

        // ------------------------------------------------------------------------

        public void DeleteVariableLight (short index, bool bUndo = false) 
        {
        if (index > -1) {
	        DLE.Backup.Begin (UndoData.Flags.udVariableLights);
	        if (index < --Count) {
                VariableLight temp = VariableLights [index];
                VariableLights [index] = VariableLights [Count];
                VariableLights [Count] = temp;
                }
	        DLE.Backup.End ();
	        }
        }

        // ------------------------------------------------------------------------

        public GameColor GetTexColor (short nTexture, bool bIsTranspWall)	
		{ 
            return UseTexColors && (bIsTranspWall || (IsLight (nTexture) != -1)) ? m_texColors [nTexture] : null; 
        }

        // ------------------------------------------------------------------------

		public GameColor FaceColor (short nSegment, short nSide = 0) 
        { 
            return m_faceColors [nSegment * 6 + nSide]; 
        }

        // ------------------------------------------------------------------------

        public GameColor LightColor (SideKey key, bool bUseTexColors) 
        { 
        DLE.Current.Get (key);
        if (bUseTexColors && UseTexColors) {
	        short nBaseTex, nOvlTex;
	        DLE.Segments.Textures (key, out nBaseTex, out nOvlTex);
	        GameColor color;
	        if (nOvlTex > 0) {
		        color = GetTexColor (nOvlTex, false);
		        if (color != null)
			        return color;
		        }
	        Wall wall = DLE.Segments.Wall (key);
	        color = GetTexColor (nBaseTex, (wall != null) && wall.IsTransparent);
	        if (color != null)
		        return color;
	        }	
        return FaceColor (key.m_nSegment, key.m_nSide); 
        }

        // ------------------------------------------------------------------------

        public int FindLight (int nTexture, TextureLight [] texLight, int nLights)
        {
	        int	l = 0;
	        int	r = nLights - 1;
	        int	m, t;

        while (l <= r) {
	        m = (l + r) / 2;
	        t = texLight [m].nBaseTex;
	        if (nTexture > t)
		        l = m + 1;
	        else if (nTexture < t)
		        r = m - 1;
	        else
		        return m;
	        }
        return -1;
        }

        // ------------------------------------------------------------------------

        public void CreateLightMap ()
        {
            LoadDefaults ();
        }

        // ------------------------------------------------------------------------

        bool HasCustomLightMap
        {
            get
            {
                using (MemoryStream resource = new MemoryStream (DLE.IsD1File ? Properties.Resources.lightMapD1 : Properties.Resources.lightMapD2))
                {
                    using (BinaryReader reader = new BinaryReader (resource))
                    {
                        for (int i = 0; i < m_lightMap.Length; i++)
                        {
                            if (m_lightMap [i] != reader.ReadInt32 ())
                                return false;
                        }
                    }
                }
                return true;
            }
        }

        // ------------------------------------------------------------------------

        bool HasCustomLightColors
        {
            get
            {
                using (MemoryStream resource = new MemoryStream (DLE.IsD1File ? Properties.Resources.texColorsD1 : Properties.Resources.texColorsD2))
                {
                    using (BinaryReader reader = new BinaryReader (resource))
                    {
                        GameColor color = new GameColor ();
                        for (int i = 0; i < m_texColors.Length; i++)
                        {
                            color.Read (reader, 0);
                            if (m_texColors [i] != color)
                                return false;
                        }
                    }
                }
                return true;
            }
        }

        // ------------------------------------------------------------------------

        short LoadDefaults ()
        {
            using (MemoryStream resource = new MemoryStream (DLE.IsD1File ? Properties.Resources.texColorsD1 : Properties.Resources.texColorsD2))
            {
                using (BinaryReader reader = new BinaryReader (resource))
                {
                    for (int i = 0; i < m_texColors.Length; i++)
                        m_texColors [i].Read (reader, 0);
                }
            }
        return 1;
        }

        // ------------------------------------------------------------------------

        void SortDeltaIndex (int left, int right)
        {
	        int	    l = left,
			        r = right,
			        m = (left + right) / 2;
	        short	mSeg = DeltaIndex [m].m_nSegment, 
			        mSide = DeltaIndex [m].m_nSide;
	        SideKey mKey = new SideKey (mSeg, mSide);

        do {
	        while (DeltaIndex [l] < mKey)
		        l++;
	        while (DeltaIndex [r] > mKey) {
		        r--;
		        }
	        if (l <= r) {
                if (l < r)
                {
                    LightDeltaIndex t = DeltaIndex [l];
                    DeltaIndex [l] = DeltaIndex [r];
                    DeltaIndex [r] = t;
                }
		        l++;
		        r--;
		        }
	        } while (l <= r);
        if (right > l)
           SortDeltaIndex (l, right);
        if (r > left)
           SortDeltaIndex (left, r);
        }

        // ------------------------------------------------------------------------

        public void SortDeltaIndex () 
        { 
        for (int i = 0; i < DeltaIndexCount; i++)
	        DeltaIndex [i].Key = i;
        SortDeltaIndex (0, DeltaIndexCount - 1); 
        }

        // ------------------------------------------------------------------------

        void UnsortDeltaIndex (int left, int right)
        {
	        int	l = left,
			    r = right,
			    m = (left + right) / 2;
	        int	mKey = DeltaIndex [m].Key;

        do {
	        while (DeltaIndex [l].Key < mKey)
		        l++;
	        while (DeltaIndex [r].Key > mKey)
		        r--;
	        if (l <= r) {
		        if (l < r)
                {
                    LightDeltaIndex t = DeltaIndex [l];
                    DeltaIndex [l] = DeltaIndex [r];
                    DeltaIndex [r] = t;
                }
                l++;
		        r--;
		        }
	        } while (l <= r);
        if (right > l)
           UnsortDeltaIndex (l, right);
        if (r > left)
           UnsortDeltaIndex (left, r);
        }

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

    }
}
