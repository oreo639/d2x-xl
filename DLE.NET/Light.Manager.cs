using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

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

        int m_nCount = 0;

        // ------------------------------------------------------------------------

        public VariableLight[] VariableLights { get { return m_variableLights; } }

        // ------------------------------------------------------------------------

        public int Count
        {
            get { return m_nCount; }
            set { m_nCount = value; }
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

        // ------------------------------------------------------------------------

        int [] m_lightMap = new int [TextureManager.MAX_TEXTURES_D2];

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
	        DLE.Backup.Begin ((int) UndoData.UndoFlag.udVariableLights);
	        if (index < --Count) {
                VariableLight temp = VariableLights [index];
                VariableLights [index] = VariableLights [Count];
                VariableLights [Count] = temp;
                }
	        DLE.Backup.End ();
	        }
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
