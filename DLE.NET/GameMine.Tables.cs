namespace DLE.NET
{
    public static class GameTables
    {
        //------------------------------------------------------------------------------
        // define points for a given side 
        public static byte[,] sideVertTable = new byte [6,4] 
        {
	        {7,6,2,3},
	        {0,4,7,3},
	        {0,1,5,4},
	        {2,6,5,1},
	        {4,5,6,7},
	        {3,2,1,0} 
        };

        //------------------------------------------------------------------------------
        // define oppisite side of a given side 
        public static byte[] oppSideTable = new byte[6] {2,3,0,1,5,4};

        //------------------------------------------------------------------------------
        // define points for the oppisite side of a given side 
        public static byte[,] oppSideVertTable = new byte [6,4] 
        {
	        {4,5,1,0},
	        {1,5,6,2},
	        {3,2,6,7},
	        {3,7,4,0},
	        {0,1,2,3},
	        {7,6,5,4} 
        };

        //------------------------------------------------------------------------------
        // define 2 points for a given line 
        public static byte[,] lineVertTable = new byte [12,2] 
        {
	        {0,1},
	        {1,2},
	        {2,3},
	        {3,0},
	        {0,4},
	        {4,5},
	        {5,6},
	        {6,7},
	        {7,4},
	        {1,5},
	        {2,6},
	        {3,7}
        };

        //------------------------------------------------------------------------------
        // define lines for a given side 
        public static byte[,] sideLineTable = new byte [6,4] 
        {
	        {7,10,2,11},//{2,11,7,10},
	        {4,8,11,3},//{3,11,8,4},
	        {0,9,5,4},//{0,9,5,4},
	        {10,6,9,1},//{1,10,6,9},
	        {5,6,7,8},//{5,6,7,8},
	        {2,1,0,3} //{0,1,2,3}
        };

        //------------------------------------------------------------------------------
        // define 3 points which connect to a certain point 
        public static byte[,] connectPointTable = new byte [8,3]
        {
	        {1,3,4},
	        {2,0,5},
	        {3,1,6},
	        {0,2,7},
	        {7,5,0},
	        {4,6,1},
	        {5,7,2},
	        {6,4,3}
        };

        //------------------------------------------------------------------------------
        // side numbers for each point (3 sides touch each point)
        public static byte[,] pointSideTable = new byte [8,3] 
        {
            {1,2,5},
            {2,3,5},
            {0,3,5},
            {0,1,5},
            {1,2,4},
            {2,3,4},
            {0,3,4},
            {0,1,4}
        };

        // CUVL corner for pointSideTable above
        public static byte[,] pointCornerTable = new byte [8,3]
        {
            {0,0,3},
            {1,3,2},
            {2,0,1},
            {3,3,0},
            {1,3,0},
            {2,2,1},
            {1,1,2},
            {0,2,3} 
        };

        //------------------------------------------------------------------------------

        public static byte [] animClipTable = new byte [GameMine.NUM_OF_CLIPS_D2] 
            {
	         0,  1,  3,  4,  5,  6,  7,  9, 10, 11, 
	        12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
	        22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
	        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 
	        42, 43, 44, 45, 46, 47, 48, 49, 50
	        };

        public static byte[] doorClipTable = new byte [GameMine.NUM_OF_CLIPS_D2] 
            {
	         1,  1,  4,  5, 10, 24,  8, 11, 13, 12, 
            14, 17, 18, 19, 20, 21, 22, 23, 25, 26, 
            28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 
            38, 39, 40, 41, 42, 43, 44, 45, 47, 48, 
            49, 50, 51, 52, 53, 54, 55, 56, 57
	        };

        //------------------------------------------------------------------------------

         public static UVL [] defaultUVLs = new UVL [4] {
	        new UVL ((short) 0x0000, (short)0x0000, (ushort)GameMine.DEFAULT_LIGHTING),
	        new UVL ((short) 0x0000, (short)0x0800, (ushort)GameMine.DEFAULT_LIGHTING),
	        new UVL ((short)-0x0800, (short)0x0800, (ushort)GameMine.DEFAULT_LIGHTING),
	        new UVL ((short)-0x0800, (short)0x0000, (ushort)GameMine.DEFAULT_LIGHTING)
        };

        //------------------------------------------------------------------------------

  }
}