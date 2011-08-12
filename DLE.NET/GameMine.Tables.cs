﻿namespace DLEdotNET
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

         public static UVL [] defaultUVLs = new UVL [4] {
	        new UVL ((short) 0x0000, (short)0x0000, (ushort)GameMine.DEFAULT_LIGHTING),
	        new UVL ((short) 0x0000, (short)0x0800, (ushort)GameMine.DEFAULT_LIGHTING),
	        new UVL ((short)-0x0800, (short)0x0800, (ushort)GameMine.DEFAULT_LIGHTING),
	        new UVL ((short)-0x0800, (short)0x0000, (ushort)GameMine.DEFAULT_LIGHTING)
        };

        //------------------------------------------------------------------------------

  }
}