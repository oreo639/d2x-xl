
using System;
namespace DLE.NET
{
    public class TextureFilter
    {

        //------------------------------------------------------------------------------

        void SETBIT (byte[] b, int i) { b [i >> 3] |= (byte) (1 << (i & 7)); }

        byte GETBIT (byte[] b, int i) { return (byte) (b [i >> 3] & (1 << (i & 7))); }

        //------------------------------------------------------------------------------

        public enum Type : int
        {
		    GRAY_ROCK = 1,
		    BROWN_ROCK = (1 << 1),
		    RED_ROCK = (1 << 2),
		    GREEN_ROCK = (1 << 3),
		    YELLOW_ROCK = (1 << 4),
		    BLUE_ROCK = (1 << 5),
		    ICE = (1 << 6),
		    STONES = (1 << 7),
		    GRASS = (1 << 8),
		    SAND = (1 << 9),
		    LAVA = (1 << 10),
		    WATER = (1 << 11),
		    STEEL = (1 << 12),
		    CONCRETE = (1 << 13),
		    BRICK = (1 << 14),
		    TARMAC = (1 << 15),
       	    WALL = (1 << 16),
		    FLOOR = (1 << 17),
		    CEILING = (1 << 18),
		    GRATE = (1 << 19),
		    FAN = (1 << 20),
		    LIGHT = (1 << 21),
		    ENERGY = (1 << 22),
		    FORCEFIELD = (1 << 23),
		    SIGN = (1 << 24),
		    SWITCH = (1 << 25),
		    TECH = (1 << 26),
		    DOOR = (1 << 27),
		    LABEL = (1 << 28),
		    MONITOR = (1 << 29),
		    STRIPES = (1 << 30),
		    MOVING = (1 << 31)
        }

        //------------------------------------------------------------------------------

        public class FilterInfo
        {
            public short m_nMin, m_nMax;
            public Type m_nFilter1, m_nFilter2;

            public FilterInfo (short nMin, short nMax, Type nFilter1, Type nFilter2 = 0)
            {
                m_nMin = nMin;
                m_nMax = nMax;
                m_nFilter1 = nFilter1;
                m_nFilter2 = nFilter2;
            }

            public uint TEX_ROCK { get { return (uint) TextureFilter.Type.GRAY_ROCK | (uint) TextureFilter.Type.BROWN_ROCK | (uint) TextureFilter.Type.RED_ROCK | (uint) TextureFilter.Type.YELLOW_ROCK | (uint) TextureFilter.Type.GREEN_ROCK | (uint) TextureFilter.Type.BLUE_ROCK; } }
            public uint TEX_NATURE { get { return (uint) TextureFilter.Type.ICE | (uint) TextureFilter.Type.STONES | (uint) TextureFilter.Type.GRASS | (uint) TextureFilter.Type.SAND | (uint) TextureFilter.Type.LAVA | (uint) TextureFilter.Type.WATER; } }
            public uint TEX_BUILDING { get { return (uint) TextureFilter.Type.STEEL | (uint) TextureFilter.Type.CONCRETE | (uint) TextureFilter.Type.BRICK | (uint) TextureFilter.Type.TARMAC | (uint) TextureFilter.Type.WALL | (uint) TextureFilter.Type.FLOOR | (uint) TextureFilter.Type.CEILING; } }
            public uint TEX_OTHER { get { return (uint) TextureFilter.Type.FAN | (uint) TextureFilter.Type.GRATE | (uint) TextureFilter.Type.DOOR; } }
            public uint TEX_TECHMAT { get { return (uint) TextureFilter.Type.SWITCH | (uint) TextureFilter.Type.TECH | (uint) TextureFilter.Type.ENERGY | (uint) TextureFilter.Type.FORCEFIELD | (uint) TextureFilter.Type.LIGHT; } }
            public uint TEX_SIGNS { get { return (uint) TextureFilter.Type.SIGN | (uint) TextureFilter.Type.LABEL | (uint) TextureFilter.Type.MONITOR | (uint) TextureFilter.Type.STRIPES; } }
        }

        //------------------------------------------------------------------------------

        const int TEXFILTER_SIZE_D1 = 154;
        const int TEXFILTER_SIZE_D2 = 174;

        int TEXFILTER_SIZE { get { return DLE.IsD1File ? TEXFILTER_SIZE_D1 : TEXFILTER_SIZE_D2; } }
        FilterInfo[] TEXTURE_FILTERS { get { return DLE.IsD1File ? texFiltersD1 : texFiltersD2; } }

        //------------------------------------------------------------------------------

        FilterInfo[] texFiltersD1 = new FilterInfo [TEXFILTER_SIZE_D1]  
        {
	        new FilterInfo (0, 5, Type.GRAY_ROCK),
	        new FilterInfo (6, 6, Type.BLUE_ROCK),
	        new FilterInfo (7, 7, Type.YELLOW_ROCK),
	        new FilterInfo (8, 8, Type.GRAY_ROCK),
	        new FilterInfo (9, 10, Type.RED_ROCK),
	        new FilterInfo (11, 11, Type.BLUE_ROCK),
	        new FilterInfo (12, 12, Type.GRAY_ROCK, Type.STONES),
	        new FilterInfo (13, 13, Type.GRAY_ROCK),
	        new FilterInfo (14, 14, Type.TARMAC),
	        new FilterInfo (15, 15, Type.GRAY_ROCK),
	        new FilterInfo (16, 16, Type.BLUE_ROCK),
	        new FilterInfo (17, 17, Type.RED_ROCK),
	        new FilterInfo (18, 18, Type.BLUE_ROCK, Type.GRAY_ROCK),
	        new FilterInfo (19, 19, Type.RED_ROCK),
	        new FilterInfo (20, 21, Type.STEEL),
	        new FilterInfo (22, 30, Type.RED_ROCK),
	        new FilterInfo (31, 31, Type.BROWN_ROCK),
	        new FilterInfo (32, 32, Type.RED_ROCK, Type.STONES),
	        new FilterInfo (33, 38, Type.RED_ROCK),
	        new FilterInfo (39, 39, Type.BRICK),
	        new FilterInfo (40, 40, Type.GRAY_ROCK),
	        new FilterInfo (41, 41, Type.FLOOR),
	        new FilterInfo (42, 42, Type.GRAY_ROCK),
	        new FilterInfo (43, 43, Type.RED_ROCK),
	        new FilterInfo (44, 44, Type.BLUE_ROCK),
	        new FilterInfo (45, 45, Type.RED_ROCK, Type.LAVA),
	        new FilterInfo (46, 47, Type.RED_ROCK),
	        new FilterInfo (48, 49, Type.GRAY_ROCK),
	        new FilterInfo (50, 50, Type.RED_ROCK),
	        new FilterInfo (51, 53, Type.BROWN_ROCK),
	        new FilterInfo (54, 55, Type.RED_ROCK),
	        new FilterInfo (56, 56, Type.BROWN_ROCK),
	        new FilterInfo (57, 57, Type.RED_ROCK),
	        new FilterInfo (58, 58, Type.GRAY_ROCK, Type.STONES),
	        new FilterInfo (59, 59, Type.ICE),
	        new FilterInfo (60, 60, Type.BROWN_ROCK, Type.STONES),
	        new FilterInfo (61, 61, Type.GRAY_ROCK),
	        new FilterInfo (62, 66, Type.BROWN_ROCK),
	        new FilterInfo (67, 69, Type.GRAY_ROCK),
	        new FilterInfo (70, 73, Type.GREEN_ROCK),
	        new FilterInfo (74, 75, Type.RED_ROCK),
	        new FilterInfo (76, 77, Type.BLUE_ROCK),
	        new FilterInfo (78, 78, Type.BROWN_ROCK),
	        new FilterInfo (79, 79, Type.BLUE_ROCK, Type.ICE),
	        new FilterInfo (80, 81, Type.BROWN_ROCK),
	        new FilterInfo (82, 82, Type.GRAY_ROCK),
	        new FilterInfo (83, 83, Type.BLUE_ROCK),
	        new FilterInfo (84, 85, Type.GRAY_ROCK),
	        new FilterInfo (86, 87, Type.BLUE_ROCK),
	        new FilterInfo (88, 89, Type.BROWN_ROCK),
	        new FilterInfo (90, 90, Type.BLUE_ROCK),
	        new FilterInfo (91, 91, Type.BROWN_ROCK),
	        new FilterInfo (92, 96, Type.RED_ROCK),
	        new FilterInfo (97, 97, Type.BROWN_ROCK),
	        new FilterInfo (98, 98, Type.GREEN_ROCK),
	        new FilterInfo (99, 99, Type.BROWN_ROCK),
	        new FilterInfo (100, 100, Type.GRAY_ROCK),
	        new FilterInfo (101, 105, Type.BROWN_ROCK),
	        new FilterInfo (106, 109, Type.GREEN_ROCK),
	        new FilterInfo (110, 110, Type.BLUE_ROCK, Type.ICE),
	        new FilterInfo (111, 116, Type.GREEN_ROCK, Type.GRASS),
	        new FilterInfo (117, 118, Type.BROWN_ROCK),
	        new FilterInfo (119, 120, Type.CONCRETE),
	        new FilterInfo (121, 122, Type.BROWN_ROCK),
	        new FilterInfo (123, 123, Type.GREEN_ROCK, Type.GRASS),
	        new FilterInfo (124, 125, Type.BROWN_ROCK),
	        new FilterInfo (126, 126, Type.BROWN_ROCK, Type.BLUE_ROCK),
	        new FilterInfo (127, 127, Type.BLUE_ROCK),
	        new FilterInfo (128, 141, Type.RED_ROCK),
	        new FilterInfo (142, 143, Type.RED_ROCK, Type.LAVA),
	        new FilterInfo (144, 144, Type.BLUE_ROCK, Type.ICE),
	        new FilterInfo (145, 145, Type.GREEN_ROCK),
	        new FilterInfo (146, 146, Type.BLUE_ROCK, Type.ICE),
	        new FilterInfo (147, 149, Type.BROWN_ROCK),
	        new FilterInfo (150, 151, Type.RED_ROCK),
	        new FilterInfo (152, 153, Type.GREEN_ROCK, Type.STONES),
	        new FilterInfo (154, 154, Type.GRAY_ROCK, Type.LAVA),
	        new FilterInfo (155, 155, Type.BROWN_ROCK, Type.LAVA),
	        new FilterInfo (156, 157, Type.STEEL),
	        new FilterInfo (158, 158, Type.CONCRETE, Type.WALL),
	        new FilterInfo (159, 159, Type.TECH, Type.WALL),
	        new FilterInfo (160, 160, Type.CONCRETE, Type.WALL),
	        new FilterInfo (161, 161, Type.TECH, Type.WALL),
	        new FilterInfo (162, 163, Type.CONCRETE, Type.WALL),
	        new FilterInfo (164, 164, Type.CONCRETE, Type.SIGN | Type.WALL),
	        new FilterInfo (165, 171, Type.CONCRETE, Type.WALL),
	        new FilterInfo (172, 173, Type.CONCRETE, Type.SIGN | Type.WALL),
	        new FilterInfo (174, 185, Type.CONCRETE, Type.WALL),
	        new FilterInfo (186, 190, Type.STEEL),
	        new FilterInfo (191, 196, Type.CONCRETE, Type.WALL),
	        new FilterInfo (197, 197, Type.STEEL),
	        new FilterInfo (198, 199, Type.CONCRETE, Type.WALL),
	        new FilterInfo (200, 200, Type.CONCRETE, Type.WALL | Type.GRATE),
	        new FilterInfo (201, 207, Type.CONCRETE, Type.WALL),
	        new FilterInfo (208, 208, Type.CONCRETE, Type.WALL | Type.SIGN),
	        new FilterInfo (209, 211, Type.CONCRETE, Type.WALL),
	        new FilterInfo (212, 214, Type.CONCRETE, Type.WALL | Type.LIGHT),
	        new FilterInfo (215, 217, Type.CONCRETE, Type.WALL),
	        new FilterInfo (218, 218, Type.CONCRETE, Type.WALL | Type.GRATE),
	        new FilterInfo (219, 219, Type.CONCRETE, Type.WALL),
	        new FilterInfo (220, 220, Type.CONCRETE, Type.WALL | Type.GRATE),
	        new FilterInfo (221, 222, Type.CONCRETE, Type.WALL | Type.LIGHT),
	        new FilterInfo (223, 230, Type.TECH),
	        new FilterInfo (231, 234, Type.CONCRETE, Type.WALL),
	        new FilterInfo (235, 237, Type.CONCRETE, Type.WALL | Type.GRATE),
	        new FilterInfo (238, 238, Type.CONCRETE, Type.WALL),
	        new FilterInfo (239, 239, Type.STEEL, Type.GRATE),
	        new FilterInfo (240, 243, Type.TECH),
	        new FilterInfo (244, 244, Type.CONCRETE, Type.WALL),
	        new FilterInfo (245, 245, Type.GRATE, Type.STEEL),
	        new FilterInfo (246, 246, Type.GRATE),
	        new FilterInfo (247, 249, Type.GRATE, Type.STEEL),
	        new FilterInfo (250, 253, Type.LIGHT),
	        new FilterInfo (254, 255, Type.GRATE),
	        new FilterInfo (256, 260, Type.CONCRETE, Type.WALL),
	        new FilterInfo (261, 262, Type.CEILING, Type.STEEL),
	        new FilterInfo (263, 263, Type.FLOOR, Type.STEEL),
	        new FilterInfo (264, 269, Type.LIGHT),
	        new FilterInfo (270, 277, Type.FLOOR),
	        new FilterInfo (278, 289, Type.LIGHT),
	        new FilterInfo (290, 291, Type.FLOOR),
	        new FilterInfo (292, 294, Type.LIGHT),
	        new FilterInfo (295, 296, Type.GRATE, Type.TECH),
	        new FilterInfo (297, 297, Type.GRATE),
	        new FilterInfo (298, 298, Type.SIGN),
	        new FilterInfo (299, 300, Type.TECH, Type.SIGN),
	        new FilterInfo (301, 301, Type.ENERGY),
	        new FilterInfo (302, 308, Type.SIGN, Type.LABEL),
	        new FilterInfo (309, 309, Type.GRATE),
	        new FilterInfo (310, 312, Type.SIGN, Type.LABEL),
	        new FilterInfo (313, 314, Type.SIGN, Type.STRIPES),
	        new FilterInfo (315, 315, Type.TECH),
	        new FilterInfo (316, 316, Type.RED_ROCK, Type.TECH),
	        new FilterInfo (317, 317, Type.SIGN, Type.LABEL),
	        new FilterInfo (318, 318, Type.SIGN, Type.STRIPES),
	        new FilterInfo (319, 321, Type.SIGN, Type.LABEL),
	        new FilterInfo (322, 322, Type.ENERGY),
	        new FilterInfo (323, 323, Type.SIGN, Type.LABEL),
	        new FilterInfo (324, 324, Type.GRATE),
	        new FilterInfo (325, 325, Type.FAN),
	        new FilterInfo (326, 326, Type.TECH, Type.SIGN),
	        new FilterInfo (327, 327, Type.SIGN, Type.LIGHT | Type.MOVING),
	        new FilterInfo (328, 328, Type.ENERGY, Type.LIGHT),
	        new FilterInfo (329, 329, Type.FAN),
	        new FilterInfo (330, 331, Type.SIGN, Type.LIGHT | Type.MOVING),
	        new FilterInfo (332, 332, Type.GREEN_ROCK, Type.TECH),
	        new FilterInfo (333, 333, Type.RED_ROCK, Type.TECH),
	        new FilterInfo (334, 337, Type.CONCRETE, Type.TECH),
	        new FilterInfo (338, 339, Type.TECH),
	        new FilterInfo (340, 340, Type.TARMAC),
	        new FilterInfo (341, 354, Type.SIGN, Type.LIGHT | Type.MONITOR),
	        new FilterInfo (355, 356, Type.LAVA, Type.LIGHT),
	        new FilterInfo (357, 370, Type.SIGN, Type.LIGHT | Type.MONITOR),
	        new FilterInfo (371, 577, Type.DOOR)
        };

        //------------------------------------------------------------------------------

        FilterInfo[] texFiltersD2 = new FilterInfo [TEXFILTER_SIZE_D2]  {
	        new FilterInfo (0, 0, Type.GRAY_ROCK, Type.CONCRETE),
	        new FilterInfo (1, 5, Type.GRAY_ROCK, 0),
	        new FilterInfo (6, 6, Type.BLUE_ROCK, 0),
	        new FilterInfo (7, 7, Type.RED_ROCK, 0),
	        new FilterInfo (8, 14, Type.GRAY_ROCK, 0),
	        new FilterInfo (15, 15, Type.BROWN_ROCK, 0),
	        new FilterInfo (16, 16, Type.RED_ROCK, Type.STONES),
	        new FilterInfo (17, 21, Type.RED_ROCK, 0),
	        new FilterInfo (22, 23, Type.GRAY_ROCK, 0),
	        new FilterInfo (24, 24, Type.RED_ROCK, 0),
	        new FilterInfo (25, 25, Type.RED_ROCK, Type.LAVA),
	        new FilterInfo (26, 27, Type.RED_ROCK, 0),
	        new FilterInfo (28, 28, Type.GRAY_ROCK, 0),
	        new FilterInfo (29, 31, Type.BROWN_ROCK, 0),
	        new FilterInfo (32, 32, Type.RED_ROCK, 0),
	        new FilterInfo (33, 33, Type.BROWN_ROCK, 0),
	        new FilterInfo (34, 34, Type.GRAY_ROCK, Type.STONES),
	        new FilterInfo (35, 35, Type.ICE, 0),
	        new FilterInfo (36, 36, Type.BROWN_ROCK, Type.STONES),
	        new FilterInfo (37, 37, Type.GRAY_ROCK, 0),
	        new FilterInfo (38, 42, Type.BROWN_ROCK, 0),
	        new FilterInfo (43, 43, Type.GRAY_ROCK, 0),
	        new FilterInfo (44, 44, Type.RED_ROCK, 0),
	        new FilterInfo (45, 45, Type.TARMAC, 0),
	        new FilterInfo (46, 49, Type.GREEN_ROCK, 0),
	        new FilterInfo (50, 51, Type.RED_ROCK, 0),
	        new FilterInfo (52, 53, Type.BLUE_ROCK, 0),
	        new FilterInfo (54, 54, Type.BROWN_ROCK, 0),
	        new FilterInfo (55, 55, Type.BLUE_ROCK, Type.ICE),
	        new FilterInfo (56, 58, Type.BROWN_ROCK, 0),
	        new FilterInfo (59, 59, Type.BLUE_ROCK, 0),
	        new FilterInfo (60, 61, Type.GRAY_ROCK, 0),
	        new FilterInfo (62, 63, Type.BLUE_ROCK, 0),
	        new FilterInfo (64, 64, Type.BROWN_ROCK, 0),
	        new FilterInfo (65, 65, Type.BLUE_ROCK, 0),
	        new FilterInfo (66, 66, Type.BROWN_ROCK, 0),
	        new FilterInfo (67, 71, Type.RED_ROCK, 0),
	        new FilterInfo (72, 72, Type.BROWN_ROCK, 0),
	        new FilterInfo (73, 73, Type.GREEN_ROCK, 0),
	        new FilterInfo (74, 74, Type.BROWN_ROCK, 0),
	        new FilterInfo (75, 75, Type.GRAY_ROCK, 0),
	        new FilterInfo (76, 80, Type.BROWN_ROCK, 0),
	        new FilterInfo (81, 81, Type.GREEN_ROCK, Type.ICE),
	        new FilterInfo (82, 82, Type.GREEN_ROCK, 0),
	        new FilterInfo (83, 83, Type.GREEN_ROCK, Type.ICE),
	        new FilterInfo (84, 84, Type.GREEN_ROCK, 0),
	        new FilterInfo (85, 85, Type.GREEN_ROCK, Type.ICE),
	        new FilterInfo (85, 85, Type.GREEN_ROCK | Type.BLUE_ROCK, Type.ICE),
	        new FilterInfo (86, 91, Type.GREEN_ROCK, Type.GRASS),
	        new FilterInfo (92, 93, Type.BROWN_ROCK, 0),
	        new FilterInfo (94, 95, Type.CONCRETE, 0),
	        new FilterInfo (96, 97, Type.BROWN_ROCK, 0),
	        new FilterInfo (98, 98, Type.GREEN_ROCK, Type.GRASS),
	        new FilterInfo (99, 100, Type.BROWN_ROCK, Type.RED_ROCK),
	        new FilterInfo (101, 101, Type.BROWN_ROCK | Type.BLUE_ROCK, Type.ICE),
	        new FilterInfo (102, 102, Type.BLUE_ROCK, 0),
	        new FilterInfo (103, 103, Type.BROWN_ROCK, 0),
	        new FilterInfo (104, 105, Type.RED_ROCK, Type.LAVA),
	        new FilterInfo (106, 106, Type.RED_ROCK, 0),
	        new FilterInfo (107, 111, Type.BROWN_ROCK, 0),
	        new FilterInfo (112, 114, Type.RED_ROCK, 0),
	        new FilterInfo (115, 116, Type.RED_ROCK, Type.LAVA),
	        new FilterInfo (117, 117, Type.BLUE_ROCK, Type.ICE),
	        new FilterInfo (118, 118, Type.GREEN_ROCK, 0),
	        new FilterInfo (119, 119, Type.BLUE_ROCK, 0),
	        new FilterInfo (120, 121, Type.BROWN_ROCK, 0),
	        new FilterInfo (122, 123, Type.BROWN_ROCK | Type.RED_ROCK, 0),
	        new FilterInfo (124, 125, Type.GREEN_ROCK, Type.GRASS),
	        new FilterInfo (126, 127, Type.RED_ROCK, 0),
	        new FilterInfo (128, 138, Type.ICE, 0),
	        new FilterInfo (139, 139, Type.ICE, 0),
	        new FilterInfo (140, 142, Type.BROWN_ROCK, 0),
	        new FilterInfo (143, 145, Type.SAND, 0),
	        new FilterInfo (146, 147, Type.RED_ROCK, 0),
	        new FilterInfo (148, 148, Type.BROWN_ROCK, 0),
	        new FilterInfo (149, 151, Type.BROWN_ROCK, Type.SAND),
	        new FilterInfo (152, 152, Type.RED_ROCK, 0),
	        new FilterInfo (153, 154, Type.BROWN_ROCK, Type.SAND),
	        new FilterInfo (155, 168, Type.RED_ROCK, 0),
	        new FilterInfo (169, 171, Type.GREEN_ROCK, Type.GRASS),
	        new FilterInfo (172, 172, Type.BLUE_ROCK, Type.ICE),
	        new FilterInfo (173, 176, Type.ICE, 0),
	        new FilterInfo (177, 177, Type.BROWN_ROCK),
	        new FilterInfo (178, 183, Type.ICE),
	        new FilterInfo (184, 184, Type.GREEN_ROCK | Type.GRASS),
	        new FilterInfo (185, 190, Type.ICE, 0),
	        new FilterInfo (191, 191, Type.CONCRETE, 0),
	        new FilterInfo (192, 193, Type.ICE, 0),
	        new FilterInfo (194, 194, Type.GREEN_ROCK, Type.ICE),
	        new FilterInfo (195, 195, Type.ICE, 0),
	        new FilterInfo (196, 196, Type.GREEN_ROCK, Type.ICE),
	        new FilterInfo (197, 197, Type.BROWN_ROCK, 0),
	        new FilterInfo (198, 198, Type.GRAY_ROCK, 0),
	        new FilterInfo (199, 199, Type.BROWN_ROCK, 0),
	        new FilterInfo (200, 201, Type.STEEL, 0),
	        new FilterInfo (202, 217, Type.CONCRETE, Type.WALL),
	        new FilterInfo (204, 204, Type.CONCRETE, Type.SIGN | Type.WALL),
	        new FilterInfo (205, 206, Type.CONCRETE, Type.WALL),
	        new FilterInfo (207, 208, Type.CONCRETE, Type.SIGN | Type.WALL),
	        new FilterInfo (209, 217, Type.CONCRETE, Type.WALL),
	        new FilterInfo (218, 222, Type.STEEL, 0),
	        new FilterInfo (223, 225, Type.CONCRETE, Type.WALL),
	        new FilterInfo (226, 226, Type.STEEL, 0),
	        new FilterInfo (227, 232, Type.CONCRETE, Type.WALL),
	        new FilterInfo (233, 233, Type.CONCRETE, Type.SIGN | Type.WALL),
	        new FilterInfo (234, 234, Type.CONCRETE, Type.WALL),
	        new FilterInfo (235, 237, Type.CONCRETE, Type.LIGHT | Type.WALL),
	        new FilterInfo (238, 240, Type.CONCRETE, Type.WALL),
	        new FilterInfo (241, 241, Type.CONCRETE, Type.GRATE | Type.WALL),
	        new FilterInfo (242, 242, Type.CONCRETE, Type.WALL),
	        new FilterInfo (243, 244, Type.CONCRETE, Type.LIGHT | Type.WALL),
	        new FilterInfo (245, 246, Type.TECH, 0),
	        new FilterInfo (247, 248, Type.GRATE, Type.STEEL),
	        new FilterInfo (249, 252, Type.CONCRETE, Type.WALL),
	        new FilterInfo (253, 255, Type.CONCRETE, Type.GRATE | Type.WALL),
	        new FilterInfo (256, 256, Type.CONCRETE, Type.WALL),
	        new FilterInfo (257, 257, Type.GRATE, Type.STEEL),
	        new FilterInfo (258, 258, Type.CONCRETE, Type.WALL),
	        new FilterInfo (259, 262, Type.GRATE, Type.STEEL),
	        new FilterInfo (263, 264, Type.GRATE, 0),
	        new FilterInfo (265, 265, Type.CONCRETE, 0),
	        new FilterInfo (266, 266, Type.FLOOR, 0),
	        new FilterInfo (267, 269, Type.GRATE, 0),
	        new FilterInfo (270, 272, Type.TECH, 0),
	        new FilterInfo (273, 274, Type.STEEL, Type.CEILING),
	        new FilterInfo (275, 279, Type.LIGHT, 0),
	        new FilterInfo (280, 287, Type.FLOOR, 0),
	        new FilterInfo (288, 302, Type.LIGHT, 0),
	        new FilterInfo (303, 304, Type.FLOOR, 0),
	        new FilterInfo (305, 307, Type.LIGHT, 0),
	        new FilterInfo (308, 310, Type.GRATE, 0),
	        new FilterInfo (311, 312, Type.SIGN, Type.TECH | Type.STRIPES),
	        new FilterInfo (313, 313, Type.ENERGY, 0),
	        new FilterInfo (314, 320, Type.SIGN, Type.LABEL),
	        new FilterInfo (321, 321, Type.GRATE, 0),
	        new FilterInfo (322, 324, Type.SIGN, Type.LABEL),
	        new FilterInfo (325, 326, Type.SIGN, Type.STRIPES),
	        new FilterInfo (327, 327, Type.RED_ROCK, Type.TECH),
	        new FilterInfo (328, 328, Type.SIGN, Type.LABEL),
	        new FilterInfo (329, 329, Type.SIGN, Type.STRIPES),
	        new FilterInfo (330, 332, Type.SIGN, Type.LABEL),
	        new FilterInfo (333, 333, Type.ENERGY, 0),
	        new FilterInfo (334, 334, Type.SIGN, Type.LABEL),
	        new FilterInfo (335, 335, Type.GRATE, 0),
	        new FilterInfo (336, 336, Type.FAN, 0),
	        new FilterInfo (337, 337, Type.SIGN, Type.TECH | Type.STRIPES),
	        new FilterInfo (338, 339, Type.SIGN, Type.STRIPES),
	        new FilterInfo (340, 347, Type.LIGHT, 0),
	        new FilterInfo (348, 349, Type.LIGHT, Type.STEEL | Type.MOVING),
	        new FilterInfo (350, 350, Type.GRATE, Type.STEEL | Type.MOVING),
	        new FilterInfo (351, 351, Type.SIGN, Type.LABEL),
	        new FilterInfo (352, 352, Type.SIGN, Type.LIGHT | Type.MOVING),
	        new FilterInfo (353, 353, Type.ENERGY, Type.LIGHT),
	        new FilterInfo (354, 354, Type.FAN, 0),
	        new FilterInfo (355, 355, Type.GREEN_ROCK, Type.TECH),
	        new FilterInfo (356, 359, Type.CONCRETE , Type.TECH | Type.WALL | Type.LIGHT),
	        new FilterInfo (360, 361, Type.LIGHT, Type.TECH),
	        new FilterInfo (362, 362, Type.TARMAC, 0),
	        new FilterInfo (363, 377, Type.SIGN, Type.LIGHT | Type.MONITOR),
	        new FilterInfo (378, 378, Type.LAVA, 0),
	        new FilterInfo (379, 398, Type.SIGN, Type.LIGHT),
	        new FilterInfo (399, 403, Type.WATER, 0),
	        new FilterInfo (404, 409, Type.LAVA, 0),
	        new FilterInfo (410, 412, Type.LIGHT, 0),
	        new FilterInfo (413, 419, Type.SWITCH, 0),
	        new FilterInfo (420, 420, Type.FORCEFIELD, Type.LIGHT),
	        new FilterInfo (423, 424, Type.SIGN, Type.LIGHT | Type.STRIPES),
	        new FilterInfo (425, 425, Type.SIGN, Type.LIGHT),
	        new FilterInfo (426, 426, Type.LIGHT, Type.TECH),
	        new FilterInfo (427, 429, Type.LIGHT, 0),
	        new FilterInfo (430, 431, Type.SIGN, Type.LIGHT),
	        new FilterInfo (432, 432, Type.FORCEFIELD, Type.LIGHT),
	        new FilterInfo (433, 434, Type.LIGHT, Type.TECH),
	        new FilterInfo (435, 901, Type.DOOR, 0)
	    };

        //------------------------------------------------------------------------------

		short [] m_mapTexToView = new short [TextureManager.MAX_TEXTURES_D2];
        short [] m_mapViewToTex = new short [TextureManager.MAX_TEXTURES_D2];
		short [] m_nTextures = new short [2] { 0, 0 };
		Type m_nFilter;
		FilterInfo[] m_filter;

        //------------------------------------------------------------------------------

        Type Filter 
        { 
            get { return m_nFilter; }
            set { m_nFilter |= (Type)value; }
        }

        //------------------------------------------------------------------------------

        void SetFilter () 
        { 
        m_filter = TEXTURE_FILTERS; 
        }

        //------------------------------------------------------------------------

        short FilterIndex (short nTexture)
        {
	        short	m, l = 0, r = (short) (m_filter.Length - 1);

            do
            {
	            m = (byte) ((l + r) / 2);
	            if (nTexture < m_filter [m].m_nMin)
                    r = (byte) (m - 1);
	            else if (nTexture > m_filter [m].m_nMax)
                    l = (byte) (m + 1);
	            else
		            return m;
            }
            while (l <= r);
            return -1;
        }

        //------------------------------------------------------------------------

        uint GetFilter (short nTexture)
        {
	        short	m = FilterIndex (nTexture);

        return (m < 0) ? 0 : (uint) m_filter [m].m_nFilter1;
        }

        //------------------------------------------------------------------------

        int CompareFilters (short nTexture, short mTexture, Type mf, Type mf2)
        {
	        short	n = FilterIndex (nTexture);
	        Type	nf = m_filter [n].m_nFilter1,
			        nf2 = m_filter [n].m_nFilter2;

        //CBRK (((nf == Type.DOOR) && (mf == Type.SAND)) || ((mf == Type.DOOR) && (nf == Type.SAND)));
        if (nf < mf)
	        return -1;
        else if (nf > mf)
	        return 1;
        else if (nf2 < mf2)
	        return -1;
        else if (nf2 > mf2)
	        return 1;
        else
	        return (nTexture < mTexture) ? -1 : (nTexture > mTexture) ? 1 : 0;
        }

        //------------------------------------------------------------------------

        void SortMap (short left, short right)
        {
	        short		mTexture = m_mapViewToTex [(left + right) / 2];
	        short		m = FilterIndex (mTexture);
	        Type		mf, mf2;
	        short		h, l = left, r = right;

        mf = m_filter [m].m_nFilter1;
        mf2 = m_filter [m].m_nFilter2;
        do {
	        while (CompareFilters (m_mapViewToTex [l], mTexture, mf, mf2) < 0)
		        l++;
	        while (CompareFilters (m_mapViewToTex [r], mTexture, mf, mf2) > 0)
		        r--;
	        if (l <= r) {
		        if (l < r) {
			        h = m_mapViewToTex [l];
			        m_mapViewToTex [l] = m_mapViewToTex [r];
			        m_mapViewToTex [r] = h;
			        }
		        l++;
		        r--;
		        }
	        }
        while (l < r);
        if (l < right)
	        SortMap (l, right);
        if (left < r)
	        SortMap (left, r);
        }

        //------------------------------------------------------------------------

        void CreateMap ()
        {
        SortMap (0, (short) (m_nTextures [1] - 1));
        for (short i = 0; i < m_nTextures [1]; i++)
	        m_mapTexToView [m_mapViewToTex [i]] = i;
        }

        //------------------------------------------------------------------------

        void Setup () 
        {
            int nTextures, nFrames = 0;
            short i;

            nTextures = DLE.Textures.MaxTextures;

            // calculate total number of textures
        m_nTextures [1] = 0;
        for (i = 0; i < nTextures; i++) {
	        if (DLE.Textures.Texture (i).m_bFrame)
		        ++nFrames;
	        else
		        m_mapViewToTex [m_nTextures [1]++] = i;
	        }

        // allocate memory for texture list
        SetFilter ();
        CreateMap ();
        }

        //------------------------------------------------------------------------
        // TextureIndex()
        //
        // looks up texture index number for nBaseTex
        // returns 0 to m_nTextures-1 on success
        // returns -1 on failure
        //------------------------------------------------------------------------

        short TextureIndex (short nTexture) 
        {
        return ((nTexture < 0) || (nTexture >= (short) m_mapTexToView.Length)) ? (short) 0 : m_mapTexToView [nTexture];
        }

        //------------------------------------------------------------------------
        // FilterTexture()
        //
        // Determines which textures to display based on which have been used
        //------------------------------------------------------------------------

        void Process (byte[] filter, bool bShowAll) 
        {
        SetFilter ();
        if (bShowAll) {
	        if (m_nFilter == (Type) (-1))
                Buffer.SetByte (filter, 0, 255);
	        else {
                Buffer.SetByte (filter, 0, 0);
		        m_nTextures [0] = 0;
		        Type f = m_nFilter & ~Type.MOVING;
		        for (int i = 0; i < m_nTextures [1]; i++) {
			        short t = m_mapViewToTex [i];
			        short j = FilterIndex (t);
			        if (((m_filter [j].m_nFilter1 | m_filter [j].m_nFilter2) & f) != 0) {
				        SETBIT (filter, i);
				        m_nTextures [0]++;
				        }
			        }
		        }
	        }
        else {
	        ushort nSegment,nSide;
            Segment [] segs = DLE.Mine.Segments;

            Buffer.SetByte (filter, 0, 0);
            m_nTextures [0] = 0;
	        for (nSegment = 0; nSegment < DLE.Mine.SegCount; nSegment++)
            {
                Side[] sides = segs [nSegment].m_sides;
                for (nSide = 0; nSide < 6; nSide++) {
			        if (sides [nSide].IsTextured)
                    {
				        short t = (short) sides [nSide].m_nBaseTex;
				        short i = TextureIndex (t);
				        short j = FilterIndex (t);
				        if ((i >= 0) && (GETBIT (filter, i) == 0) && 
					        (((m_filter [j].m_nFilter1 | m_filter [j].m_nFilter2) & m_nFilter) != 0)) {
					        SETBIT (filter, i);
					        m_nTextures [0]++;
					        }
				        t = (short) (segs [i].m_sides [nSide].m_nOvlTex & 0x3fff);
				        i = TextureIndex (t);
				        j = FilterIndex (t);
				        if ((t > 0) && (GETBIT (filter, i) != 0)) {
					        SETBIT (filter, i);
					        m_nTextures [0]++;
					        }
				        }
			        }
                }
	        }
        }

        //------------------------------------------------------------------------------

    }
}
