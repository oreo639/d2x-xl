/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "descent.h"
#include "pstypes.h"
#include "strutil.h"
#include "text.h"
#include "gr.h"
#include "ogl_defs.h"
#include "loadgamedata.h"
#include "u_mem.h"
#include "mono.h"
#include "error.h"
#include "object.h"
#include "vclip.h"
#include "effects.h"
#include "polymodel.h"
#include "wall.h"
#include "textures.h"
#include "game.h"
#include "multi.h"
#include "iff.h"
#include "cfile.h"
#include "powerup.h"
#include "sounds.h"
#include "piggy.h"
#include "aistruct.h"
#include "robot.h"
#include "weapon.h"
#include "cockpit.h"
#include "player.h"
#include "endlevel.h"
#include "reactor.h"
#include "makesig.h"
#include "interp.h"
#include "light.h"
#include "byteswap.h"
#include "network.h"

#define PRINT_WEAPON_INFO	0

//-----------------------------------------------------------------------------

CPlayerShip defaultShipProps;
#if 0
 = {
	108, 58, 262144, 2162, 511180, 0, 0, I2X (1) / 2, 9175,
 {CFixVector::Create(146013, -59748, 35756),
	 CFixVector::Create(-147477, -59892, 34430),
 	 CFixVector::Create(222008, -118473, 148201),
	 CFixVector::Create(-223479, -118213, 148302),
	 CFixVector::Create(153026, -185, -91405),
	 CFixVector::Create(-156840, -185, -91405),
	 CFixVector::Create(1608, -87663, 184978),
	 CFixVector::Create(-1608, -87663, -190825)}};
#endif

//-----------------------------------------------------------------------------

CWeaponInfo defaultWeaponInfoD2 [] = {
   {2,0,111,114,11,59,13,1,1,11,0,0,28,1,0,0,0,128,0,0,0,-1,32112,16384,65536,{0},0,65536,196608,{589824,589824,589824,589824,589824},{7864320,7864320,7864320,7864320,7864320},32768,0,0,642252,49152,655360,0,{253},{254}},
   {2,0,115,118,15,21,14,30,1,11,0,0,28,1,0,0,0,128,0,0,0,-1,32112,16384,65536,{0},0,72089,196608,{638976,638976,638976,638976,589824},{8192000,8192000,8192000,8192000,8192000},32768,0,0,642252,65536,655360,0,{0},{0}},
   {2,0,119,122,14,23,15,31,1,11,0,0,28,1,0,0,0,128,0,0,0,-1,32112,16384,65536,{0},0,75366,196608,{688128,688128,688128,688128,655360},{8519680,8519680,8519680,8519680,8519680},32768,0,0,642252,81920,655360,0,{0},{0}},
   {2,0,123,126,12,22,16,32,1,11,0,0,28,1,0,0,0,128,0,0,0,-1,32112,16384,65536,{0},0,78643,196608,{737280,737280,737280,737280,720896},{8847360,8847360,8847360,8847360,8847360},32768,0,0,642252,114688,655360,0,{0},{0}},
   {3,0,-1,-1,14,23,18,23,1,11,0,55,28,1,0,0,0,128,0,0,0,-1,32768,16384,65536,{0},65536,131072,163840,{131072,196608,262144,327680,393216},{983040,1966080,2949120,3932160,5570560},65536,0,0,655360,65536,655360,0,{0},{0}},
   {3,0,-1,-1,11,59,19,59,1,11,0,56,28,1,0,0,0,128,0,0,0,-1,32768,16384,65536,{0},65536,163840,163840,{196608,262144,393216,458752,524288},{983040,1966080,2949120,3932160,5570560},65536,0,0,655360,65536,655360,0,{0},{0}},
   {3,0,-1,-1,11,59,22,1,1,11,0,64,28,1,0,0,0,128,0,0,0,-1,32768,16384,65536,{0},65536,196608,196608,{262144,393216,589824,720896,851968},{983040,1966080,2949120,3932160,5570560},65536,0,0,655360,65536,655360,0,{0},{0}},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,{0},0,0,0,{0,0,0,0,0},{0,0,0,0,0},0,0,0,0,0,0,0,{0},{0}},
   {2,0,127,-1,11,5,130,5,1,11,1,0,11,0,1,0,0,128,0,0,0,-1,0,32768,65536,{0},0,327680,458752,{1966080,1966080,1966080,1966080,1966080},{5898240,5898240,5898240,5898240,5898240},65536,0,0,655360,262144,655360,1966080,{255},{256}},
   {2,0,128,-1,11,59,23,-1,1,11,0,0,-1,1,0,0,0,128,0,0,0,-1,65536,75366,65536,{0},0,32768,65536,{65536,65536,65536,65536,65536},{7864320,7864320,7864320,7864320,7864320},6553,0,0,327680,196608,327680,0,{0},{0}},
   {2,0,129,130,14,23,15,31,1,11,0,0,11,1,0,0,0,128,0,0,0,-1,32768,16384,65536,{0},0,196608,196608,{131072,196608,196608,262144,393216},{1966080,2621440,3276800,3932160,5242880},6553,0,0,642252,65536,655360,0,{0},{0}},
   {-1,0,-1,-1,11,1,115,1,1,11,1,0,-1,0,1,0,0,128,0,0,0,-1,0,3276,65536,{259},32768,131072,163840,{262144,262144,262144,262144,262144},{32768000,32768000,32768000,32768000,32768000},655,0,0,655360,0,196608,0,{257},{258}},
   {1,0,-1,-1,14,23,18,23,1,11,0,0,11,1,0,0,0,128,0,0,0,-1,32768,13107,65536,{262},32768,131072,163840,{589824,589824,589824,589824,589824},{8519680,8519680,8519680,8519680,8519680},13107,0,0,655360,65536,655360,0,{260},{261}},
   {3,0,-1,-1,12,23,25,32,1,11,0,54,11,1,0,0,0,128,0,0,0,-1,32768,9830,65536,{0},85196,131072,229376,{720896,720896,720896,720896,720896},{9830400,9830400,9830400,9830400,9830400},655,0,0,655360,131072,655360,0,{263},{264}},
   {2,1,131,132,15,65,24,65,1,11,0,0,11,1,0,0,0,128,0,0,0,-1,32768,65536,32768,{0},0,393216,327680,{1966080,1966080,1966080,1966080,1966080},{9830400,9830400,9830400,9830400,9830400},32768,0,0,65536,327680,655360,0,{265},{266}},
   {2,0,133,-1,11,5,130,5,1,11,1,0,11,0,1,0,1,128,0,0,0,-1,0,32768,65536,{0},655360,327680,458752,{2621440,2621440,2621440,2621440,2621440},{5898240,5898240,5898240,5898240,5898240},65536,0,0,65536,262144,655360,1310720,{267},{268}},
   {3,0,-1,-1,11,5,26,5,1,11,1,53,11,1,1,1,0,128,0,0,0,-1,0,13107,65536,{0},196608,327680,458752,{1966080,1966080,1966080,1966080,1966080},{0,0,0,0,0},65536,2162,0,655360,0,2293760,2621440,{269},{270}},
   {2,0,134,-1,11,5,130,5,1,11,1,0,11,0,1,0,0,128,0,0,0,19,0,32768,65536,{0},655360,327680,458752,{1638400,1638400,1638400,1638400,1638400},{5570560,5570560,5570560,5570560,5570560},65536,0,0,65536,524288,196608,1310720,{271},{272}},
   {2,0,135,-1,11,63,132,5,1,231,1,0,231,0,1,0,1,128,0,0,0,-1,0,65536,65536,{0},0,327680,1966080,{13041664,13041664,13041664,13041664,13041664},{5570560,5570560,5570560,5570560,5570560},65536,0,0,655360,524288,655360,5242880,{274},{275}},
   {3,0,-1,-1,11,5,12,5,1,11,1,54,11,0,0,0,1,128,0,0,0,-1,0,98304,65536,{0},98304,327680,458752,{2293760,2293760,2293760,2293760,2293760},{5898240,5898240,5898240,5898240,5898240},65536,0,0,65536,65536,786432,0,{0},{0}},
   {1,0,-1,-1,14,23,18,23,1,11,0,0,11,1,0,0,0,128,0,0,0,-1,65536,26214,65536,{262},32768,131072,163840,{65536,262144,393216,524288,655360},{5242880,5570560,5898240,6553600,7208960},13107,0,0,655360,32768,655360,0,{260},{261}},
   {2,0,136,-1,11,5,130,5,1,11,1,0,11,0,1,0,1,128,0,0,0,-1,0,32768,65536,{0},655360,327680,458752,{655360,983040,1310720,1638400,1966080},{3932160,3932160,3932160,3932160,5242880},65536,0,0,65536,262144,655360,2621440,{269},{268}},
   {2,0,137,-1,11,5,130,5,1,11,1,0,11,0,1,0,0,128,0,0,0,-1,0,32768,65536,{0},0,327680,458752,{524288,786432,1179648,1507328,1966080},{2621440,3276800,3932160,4587520,6225920},65536,0,0,655360,262144,655360,2621440,{255},{256}},
   {1,0,-1,-1,14,23,18,23,1,11,0,0,-1,1,0,0,0,128,0,0,0,-1,65536,13107,65536,{262},32768,131072,163840,{786432,786432,786432,786432,786432},{13107200,13107200,13107200,13107200,13107200},13107,0,0,655360,0,655360,0,{260},{261}},
   {2,0,138,139,11,59,15,1,1,11,0,0,28,1,0,0,0,128,0,0,0,-1,32768,16384,65536,{0},0,196608,196608,{65536,131072,196608,262144,393216},{1966080,2621440,3276800,3932160,5242880},6553,0,0,642252,65536,655360,0,{0},{0}},
   {2,0,140,141,12,22,15,32,1,11,0,0,28,1,0,0,0,128,0,0,0,-1,32768,16384,65536,{0},0,196608,196608,{131072,196608,196608,262144,393216},{2490368,3145728,3801088,4456448,5111808},6553,0,0,642252,65536,655360,0,{0},{0}},
   {3,0,-1,-1,12,23,18,32,1,11,0,54,11,1,0,0,0,128,0,0,0,-1,39321,9830,65536,{0},85196,131072,229376,{196608,327680,458752,524288,655360},{5242880,5898240,6553600,7864320,9830400},655,0,0,655360,0,655360,0,{263},{264}},
   {3,0,-1,-1,11,59,19,59,1,11,0,56,28,1,0,0,0,128,0,0,0,-1,32768,16384,65536,{0},65536,163840,163840,{327680,393216,524288,786432,1179648},{983040,1966080,2949120,3932160,5242880},131072,0,0,655360,65536,655360,0,{0},{0}},
   {2,0,142,-1,11,63,132,5,1,11,1,0,11,0,1,0,1,128,0,0,0,-1,0,32768,65536,{0},0,327680,1966080,{3276800,4587520,5898240,7208960,7864320},{4259840,4915200,5570560,5570560,5570560},65536,0,0,655360,524288,655360,5242880,{274},{275}},
   {3,0,-1,-1,11,5,12,5,1,11,1,54,11,0,0,0,1,128,0,0,0,-1,0,98304,65536,{0},98304,327680,458752,{262144,393216,589824,720896,851968},{2621440,3276800,3932160,4587520,5898240},65536,0,0,65536,65536,524288,0,{0},{0}},
   {2,0,143,146,85,59,44,87,1,11,0,0,28,1,0,0,0,128,0,0,0,-1,49152,16384,65536,{0},0,81920,196608,{786432,786432,786432,786432,786432},{8847360,8847360,8847360,8847360,8847360},32768,0,0,642252,114688,655360,0,{277},{278}},
   {2,0,147,150,86,59,45,88,1,11,0,0,28,1,0,0,0,128,0,0,0,-1,49152,16384,65536,{0},0,81920,196608,{819200,819200,819200,819200,786432},{8847360,8847360,8847360,8847360,8847360},39321,0,0,642252,114688,655360,0,{0},{0}},
   {-1,0,-1,-1,11,95,230,95,1,11,2,0,-1,0,1,0,0,128,0,0,0,-1,0,9830,65536,{259},32768,131072,229376,{1638400,1638400,1638400,1638400,1638400},{45875200,45875200,45875200,45875200,45875200},13107,0,0,655360,0,196608,655360,{279},{280}},
   {3,0,-1,-1,12,22,37,22,1,11,0,98,11,1,0,0,0,128,0,0,0,-1,49152,9830,49152,{0},32768,131072,163840,{720896,720896,720896,720896,720896},{8519680,8519680,8519680,8519680,8519680},13107,0,0,655360,98304,655360,0,{281},{282}},
   {3,0,-1,-1,11,23,38,59,1,11,0,68,11,1,0,2,0,128,0,0,0,-1,65536,7864,39321,{0},147456,131072,229376,{786432,786432,786432,786432,786432},{9175040,9175040,9175040,9175040,9175040},655,0,0,655360,131072,655360,0,{283},{284}},
   {3,0,-1,-1,86,105,77,105,1,11,0,103,11,1,0,0,0,128,0,0,0,-1,0,65536,32768,{0},58982,81920,262144,{458752,458752,458752,458752,458752},{65536000,65536000,65536000,65536000,65536000},655,0,0,655360,65536,655,0,{285},{286}},
   {2,0,151,-1,11,5,130,5,1,76,1,0,76,0,1,0,0,128,0,16,0,-1,0,39321,65536,{0},0,327680,458752,{589824,589824,589824,589824,589824},{7208960,7208960,7208960,7208960,7208960},65536,0,0,655360,262144,1966080,5242880,{287},{288}},
   {2,0,152,-1,11,5,130,5,1,11,1,0,11,0,1,1,1,128,0,0,0,-1,0,19660,65536,{0},655360,327680,458752,{3604480,3604480,3604480,3604480,3604480},{3932160,3932160,3932160,3932160,3932160},65536,0,0,65536,262144,1310720,2621440,{290},{291}},
   {3,0,-1,-1,11,5,26,5,1,11,1,80,11,1,1,1,0,128,0,0,0,47,0,13107,65536,{0},196608,327680,458752,{1966080,1966080,1966080,1966080,1966080},{0,0,0,0,0},65536,2162,0,655360,0,7864320,655,{293},{294}},
   {2,0,153,-1,11,106,133,5,1,11,1,0,11,0,1,0,0,128,0,0,16,-1,0,49152,65536,{0},0,327680,327680,{3276800,3276800,3276800,3276800,3276800},{26214400,26214400,26214400,26214400,26214400},78643,0,0,655360,262144,655360,1966080,{295},{296}},
   {2,0,154,-1,11,104,250,5,1,231,1,0,231,0,1,0,0,128,0,0,32,54,0,98304,65536,{0},0,327680,3276800,{14417920,14417920,14417920,14417920,14417920},{7864320,7864320,7864320,7864320,7864320},65536,0,0,655360,524288,655360,5242880,{298},{299}},
   {2,0,155,-1,11,1,115,1,1,11,1,0,-1,0,1,0,0,128,0,0,0,-1,0,6553,65536,{0},32768,131072,163840,{65536,65536,131072,131072,196608},{4587520,5898240,7864320,9830400,11141120},655,0,0,642252,0,196608,0,{257},{0}},
   {3,0,-1,-1,11,23,15,59,1,11,0,97,28,1,0,0,0,128,0,0,0,-1,32768,16384,65536,{0},32768,196608,131072,{131072,196608,262144,327680,393216},{983040,1638400,2949120,3932160,5242880},6553,0,0,642252,65536,655360,0,{0},{0}},
   {2,0,156,157,86,22,15,32,1,11,0,0,28,1,0,0,0,128,0,0,0,-1,32768,16384,65536,{0},0,91750,196608,{196608,262144,327680,458752,524288},{1638400,2293760,3604480,5242880,5898240},6553,0,0,642252,65536,655360,0,{0},{0}},
   {3,0,-1,-1,11,23,38,59,1,11,0,68,11,1,0,2,0,128,0,0,0,-1,32768,6553,65536,{0},147456,131072,229376,{262144,327680,393216,524288,589824},{2293760,2949120,4259840,5242880,5898240},655,0,0,655360,131072,655360,0,{283},{0}},
   {3,0,-1,-1,11,23,38,59,1,11,0,68,11,1,0,2,0,128,0,0,0,-1,32768,6553,65536,{0},147456,131072,229376,{327680,458752,589824,720896,786432},{4587520,5898240,7208960,9175040,10485760},655,0,0,655360,131072,655360,0,{283},{0}},
   {3,0,-1,-1,14,23,18,23,1,11,0,67,11,1,0,0,0,128,0,0,0,-1,65536,26214,65536,{0},32768,131072,163840,{196608,327680,458752,589824,720896},{4259840,5242880,6553600,7208960,7864320},13107,0,0,655360,0,655360,0,{281},{0}},
   {3,0,-1,-1,11,5,12,2,1,11,1,92,11,0,0,0,1,128,0,0,0,-1,0,98304,65536,{0},131072,327680,196608,{1638400,1638400,1638400,1638400,1638400},{5898240,5898240,5898240,5898240,5898240},65536,0,0,65536,65536,786432,0,{0},{0}},
   {3,0,-1,-1,12,23,18,32,1,11,0,92,11,1,0,0,0,128,0,0,0,-1,39321,9830,65536,{0},85196,131072,229376,{196608,327680,458752,524288,589824},{3276800,5242880,6553600,7864320,9830400},655,0,0,655360,0,655360,0,{263},{0}},
   {3,0,-1,-1,11,5,12,2,1,11,1,92,11,0,0,0,1,128,0,0,0,-1,0,98304,65536,{0},131072,327680,196608,{786432,851968,917504,983040,1048576},{2293760,3604480,4259840,4915200,5242880},65536,0,0,65536,65536,786432,0,{0},{0}},
   {2,0,158,-1,11,5,130,5,1,11,1,0,11,0,1,0,0,128,0,10,0,-1,0,32768,65536,{0},0,327680,458752,{131072,262144,393216,458752,524288},{2293760,3604480,4259840,5570560,6225920},65536,0,0,655360,262144,655360,3276800,{255},{0}},
   {2,0,159,-1,11,5,26,5,1,11,1,0,11,1,1,1,0,128,1,0,0,-1,0,13107,65536,{0},0,327680,458752,{655360,983040,1310720,1638400,1966080},{0,0,0,0,0},65536,2162,0,65536,0,2293760,2621440,{0},{0}},
   {-1,0,-1,-1,11,95,230,95,1,11,2,0,-1,0,1,0,0,128,0,0,0,-1,0,13107,65536,{259},32768,131072,229376,{393216,524288,655360,851968,983040},{6553600,16384000,26214400,26214400,26214400},13107,0,0,655360,0,196608,458752,{279},{0}},
   {3,0,-1,-1,11,5,26,5,1,11,1,80,11,1,0,1,0,128,0,0,0,49,0,13107,65536,{0},196608,327680,458752,{327680,655360,983040,1310720,1966080},{0,0,0,0,0},65536,2162,0,655360,0,4915200,655360,{293},{0}},
   {2,0,160,-1,11,104,132,5,1,231,1,0,231,0,1,0,1,128,0,0,32,-1,0,65536,65536,{0},0,327680,1966080,{6553600,6553600,6553600,6553600,6553600},{18677760,18677760,18677760,18677760,18677760},65536,0,0,655360,524288,655360,5242880,{274},{0}},
   {2,0,161,-1,11,5,130,5,1,11,1,0,11,0,1,0,0,128,0,0,16,-1,0,49152,65536,{0},0,327680,655360,{983040,1310720,1638400,1966080,2293760},{3276800,4587520,6553600,9830400,13107200},78643,0,0,655360,262144,655360,1638400,{255},{0}},
   {3,0,-1,-1,11,5,12,2,1,11,1,92,11,0,0,0,1,128,0,0,0,-1,0,98304,65536,{0},131072,327680,196608,{983040,1310720,1638400,1966080,2293760},{2621440,3276800,3932160,4587520,5898240},65536,0,0,65536,65536,786432,0,{0},{0}},
   {2,0,162,-1,11,5,130,5,1,11,1,0,11,0,1,0,0,128,0,0,0,29,0,32768,65536,{0},655360,327680,458752,{589824,851968,1179648,1507328,1966080},{2949120,3932160,5242880,5898240,6225920},65536,0,0,65536,524288,262144,1310720,{271},{272}},
   {2,0,163,-1,11,104,132,5,1,231,1,0,231,0,1,0,0,128,0,0,32,59,0,98304,65536,{0},0,327680,3276800,{3932160,4587520,5570560,6553600,7208960},{3276800,4587520,5898240,7208960,7864320},65536,0,0,655360,524288,655360,5242880,{298},{299}},
   {2,0,164,-1,11,104,132,5,1,231,1,0,231,0,1,0,1,128,0,0,32,-1,0,65536,65536,{0},0,327680,1966080,{589824,983040,1310720,1769472,1966080},{2621440,3932160,5242880,6553600,7864320},65536,0,0,655360,524288,655360,3276800,{274},{0}},
   {3,0,-1,-1,86,105,38,105,1,11,0,103,11,1,0,0,1,128,0,0,0,-1,327680,65536,65536,{0},58982,81920,262144,{327680,458752,655360,786432,917504},{6553600,9830400,11141120,12451840,13762560},655,0,0,655360,131072,376832,0,{285},{286}},
   {2,0,165,-1,11,5,130,5,1,11,1,0,11,0,1,0,1,128,0,10,0,-1,0,32768,65536,{0},0,327680,458752,{458752,589824,655360,720896,786432},{3604480,4915200,5570560,6225920,6881280},65536,0,0,655360,262144,655360,5242880,{255},{0}}
};

//-----------------------------------------------------------------------------

ubyte Sounds [2][MAX_SOUNDS];
ubyte AltSounds [2][MAX_SOUNDS];

//right now there's only one CPlayerData ship, but we can have another by
//adding an array and setting the pointer to the active ship.

//---------------- Variables for CObject textures ----------------

#if 0 //def FAST_FILE_IO /*disabled for a reason!*/
#define ReadTMapInfoN(ti, n, fp) cf.Read (ti, sizeof (tTexMapInfo), n, fp)
#else
/*
 * reads n tTexMapInfo structs from a CFile
 */
int ReadTMapInfoN (CArray<tTexMapInfo>& ti, int n, CFile& cf)
{
	int i;

for (i = 0;i < n;i++) {
	ti [i].flags = cf.ReadByte ();
	ti [i].pad [0] = cf.ReadByte ();
	ti [i].pad [1] = cf.ReadByte ();
	ti [i].pad [2] = cf.ReadByte ();
	ti [i].lighting = cf.ReadFix ();
	ti [i].damage = cf.ReadFix ();
	ti [i].nEffectClip = cf.ReadShort ();
	ti [i].destroyed = cf.ReadShort ();
	ti [i].slide_u = cf.ReadShort ();
	ti [i].slide_v = cf.ReadShort ();
	}
return i;
}
#endif

//------------------------------------------------------------------------------

int ReadTMapInfoND1 (tTexMapInfo *ti, int n, CFile& cf)
{
	int i;

for (i = 0;i < n;i++) {
	cf.Seek (13, SEEK_CUR);// skip filename
	ti [i].flags = cf.ReadByte ();
	ti [i].lighting = cf.ReadFix ();
	ti [i].damage = cf.ReadFix ();
	ti [i].nEffectClip = cf.ReadInt ();
	}
return i;
}

//------------------------------------------------------------------------------
// Read data from piggy.
// This is called when the editor is OUT.
// If editor is in, bm_init_use_table () is called.
int BMInit (void)
{
if (!PiggyInit ())				// This calls BMReadAll
	Error ("Cannot open pig and/or ham file");
/*---*/PrintLog (1, "Initializing endlevel data\n");
InitEndLevel ();		//this is in bm_init_use_tbl (), so I gues it goes here
PrintLog (-1);
return 0;
}

//------------------------------------------------------------------------------

void BMSetAfterburnerSizes (void)
{
	sbyte	nSize = gameData.weapons.info [MERCURYMSL_ID].nAfterburnerSize;

//gameData.weapons.info [VULCAN_ID].nAfterburnerSize = 
//gameData.weapons.info [GAUSS_ID].nAfterburnerSize = nSize / 8;
gameData.weapons.info [CONCUSSION_ID].nAfterburnerSize =
gameData.weapons.info [HOMINGMSL_ID].nAfterburnerSize =
gameData.weapons.info [ROBOT_CONCUSSION_ID].nAfterburnerSize =
gameData.weapons.info [FLASHMSL_ID].nAfterburnerSize =
gameData.weapons.info [GUIDEDMSL_ID].nAfterburnerSize =
gameData.weapons.info [ROBOT_FLASHMSL_ID].nAfterburnerSize =
gameData.weapons.info [ROBOT_MEGA_FLASHMSL_ID].nAfterburnerSize =
gameData.weapons.info [ROBOT_MERCURYMSL_ID].nAfterburnerSize = nSize;
gameData.weapons.info [ROBOT_HOMINGMSL_ID].nAfterburnerSize =
gameData.weapons.info [SMARTMSL_ID].nAfterburnerSize = 2 * nSize;
gameData.weapons.info [MEGAMSL_ID].nAfterburnerSize =
gameData.weapons.info [ROBOT_MEGAMSL_ID].nAfterburnerSize =
gameData.weapons.info [ROBOT_SHAKER_MEGA_ID].nAfterburnerSize =
gameData.weapons.info [EARTHSHAKER_MEGA_ID].nAfterburnerSize = 3 * nSize;
gameData.weapons.info [EARTHSHAKER_ID].nAfterburnerSize =
gameData.weapons.info [ROBOT_EARTHSHAKER_ID].nAfterburnerSize = 4 * nSize;
}

//------------------------------------------------------------------------------

void QSortTextureIndex (short *pti, short left, short right)
{
	short	l = left,
			r = right,
			m = pti [(left + right) / 2],
			h;

do {
	while (pti [l] < m)
		l++;
	while (pti [r] > m)
		r--;
	if (l <= r) {
		if (l < r) {
			h = pti [l];
			pti [l] = pti [r];
			pti [r] = h;
			}
		l++;
		r--;
		}
	} while (l <= r);
if (l < right)
	QSortTextureIndex (pti, l, right);
if (left < r)
	QSortTextureIndex (pti, left, r);
}

//------------------------------------------------------------------------------

void BuildTextureIndex (int i, int n)
{
	short				*pti = gameData.pig.tex.textureIndex [i].Buffer ();
	tBitmapIndex	*pbi = gameData.pig.tex.bmIndex [i].Buffer ();

gameData.pig.tex.textureIndex [i].Clear (0xff);
for (i = 0; i < n; i++)
	pti [pbi [i].index] = i;
//QSortTextureIndex (pti, 0, n - 1);
}

//------------------------------------------------------------------------------

void BMReadAll (CFile& cf, bool bDefault)
{
	int i, t;

gameData.pig.tex.nTextures [0] = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d texture indices\n", gameData.pig.tex.nTextures [0]);
ReadBitmapIndices (gameData.pig.tex.bmIndex [0], gameData.pig.tex.nTextures [0], cf);
BuildTextureIndex (0, gameData.pig.tex.nTextures [0]);
ReadTMapInfoN (gameData.pig.tex.tMapInfo [0], gameData.pig.tex.nTextures [0], cf);
PrintLog (-1);

t = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d sound indices\n", t);
cf.Read (Sounds [0], sizeof (ubyte), t);
cf.Read (AltSounds [0], sizeof (ubyte), t);
PrintLog (-1);

gameData.effects.nClips [0] = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d animation clips\n", gameData.effects.nClips [0]);
ReadVideoClips (gameData.effects.vClips [0], gameData.effects.nClips [0], cf);
PrintLog (-1);

gameData.effects.nEffects [0] = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d animation descriptions\n", gameData.effects.nEffects [0]);
ReadEffectClips (gameData.effects.effects [0], gameData.effects.nEffects [0], cf);
// red glow texture animates way too fast
gameData.effects.effects [0][32].vClipInfo.xTotalTime *= 10;
gameData.effects.effects [0][32].vClipInfo.xFrameTime *= 10;
gameData.walls.nAnims [0] = cf.ReadInt ();
PrintLog (-1);
/*---*/PrintLog (1, "Loading %d CWall animations\n", gameData.walls.nAnims [0]);
ReadWallClips (gameData.walls.anims [0], gameData.walls.nAnims [0], cf);
PrintLog (-1);

gameData.bots.nTypes [0] = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d robot descriptions\n", gameData.bots.nTypes [0]);
ReadRobotInfos (gameData.bots.info [0], gameData.bots.nTypes [0], cf);
gameData.bots.nDefaultTypes = gameData.bots.nTypes [0];
gameData.bots.defaultInfo = gameData.bots.info [0];
PrintLog (-1);

gameData.bots.nJoints = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d robot joint descriptions\n", gameData.bots.nJoints);
ReadJointPositions (gameData.bots.joints, gameData.bots.nJoints, cf);
gameData.bots.nDefaultJoints = gameData.bots.nJoints;
gameData.bots.defaultJoints = gameData.bots.joints;
PrintLog (-1);

gameData.weapons.nTypes [0] = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d weapon descriptions\n", gameData.weapons.nTypes [0]);
ReadWeaponInfos (0, gameData.weapons.nTypes [0], cf, gameData.pig.tex.nHamFileVersion, bDefault);
gameData.weapons.info [48].light = I2X (1); // fix light for BPer shots and smart missile blobs - don't make them too bright though
BMSetAfterburnerSizes ();
PrintLog (-1);

gameData.objs.pwrUp.nTypes = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d powerup descriptions\n", gameData.objs.pwrUp.nTypes);
ReadPowerupTypeInfos (gameData.objs.pwrUp.info.Buffer (), gameData.objs.pwrUp.nTypes, cf);
PrintLog (-1);

gameData.models.nPolyModels = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d CPolyModel descriptions\n", gameData.models.nPolyModels);
ReadPolyModels (gameData.models.polyModels [0], gameData.models.nPolyModels, cf);
if (bDefault) {
	gameData.models.nDefPolyModels = gameData.models.nPolyModels;
	memcpy (gameData.models.polyModels [1].Buffer (), gameData.models.polyModels [0].Buffer (), gameData.models.nPolyModels * sizeof (CPolyModel));
	}
PrintLog (-1);

/*---*/PrintLog (1, "Loading poly model data\n");
for (i = 0; i < gameData.models.nPolyModels; i++) {
	gameData.models.polyModels [0][i].SetBuffer (NULL);
	if (bDefault)
		gameData.models.polyModels [1][i].SetBuffer (NULL);
	gameData.models.polyModels [0][i].SetCustom (!bDefault);
	gameData.models.polyModels [0][i].ReadData (bDefault ? gameData.models.polyModels [1] + i : NULL, cf);
	}

for (i = 0; i < gameData.models.nPolyModels; i++)
	gameData.models.nDyingModels [i] = cf.ReadInt ();
for (i = 0; i < gameData.models.nPolyModels; i++)
	gameData.models.nDeadModels [i] = cf.ReadInt ();
PrintLog (-1);

t = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d cockpit gauges\n", t);
ReadBitmapIndices (gameData.cockpit.gauges [1], t, cf);
ReadBitmapIndices (gameData.cockpit.gauges [0], t, cf);
PrintLog (-1);

gameData.pig.tex.nObjBitmaps = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d CObject bitmap indices\n", gameData.pig.tex.nObjBitmaps);
ReadBitmapIndices (gameData.pig.tex.objBmIndex, gameData.pig.tex.nObjBitmaps, cf);
gameData.pig.tex.defaultObjBmIndex = gameData.pig.tex.objBmIndex;
for (i = 0; i < gameData.pig.tex.nObjBitmaps; i++)
	gameData.pig.tex.objBmIndexP [i] = cf.ReadShort ();
PrintLog (-1);

/*---*/PrintLog (1, "Loading CPlayerData ship description\n");
PlayerShipRead (&gameData.pig.ship.only, cf);
PrintLog (-1);

gameData.models.nCockpits = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d cockpit bitmaps\n", gameData.models.nCockpits);
ReadBitmapIndices (gameData.pig.tex.cockpitBmIndex, gameData.models.nCockpits, cf);
gameData.pig.tex.nFirstMultiBitmap = cf.ReadInt ();
PrintLog (-1);

gameData.reactor.nReactors = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d reactor descriptions\n", gameData.reactor.nReactors);
ReadReactors (cf);
PrintLog (-1);

gameData.models.nMarkerModel = cf.ReadInt ();
if (gameData.pig.tex.nHamFileVersion < 3) {
	gameData.endLevel.exit.nModel = cf.ReadInt ();
	gameData.endLevel.exit.nDestroyedModel = cf.ReadInt ();
	}
else
	gameData.endLevel.exit.nModel = 
	gameData.endLevel.exit.nDestroyedModel = gameData.models.nPolyModels;
}

//------------------------------------------------------------------------------

void BMReadWeaponInfoD1N (CFile& cf, int i)
{
	CD1WeaponInfo* wiP = gameData.weapons.infoD1 + i;

wiP->renderType = cf.ReadByte ();
wiP->nModel = cf.ReadByte ();
wiP->nInnerModel = cf.ReadByte ();
wiP->persistent = cf.ReadByte ();
wiP->nFlashVClip = cf.ReadByte ();
wiP->flashSound = cf.ReadShort ();
wiP->nRobotHitVClip = cf.ReadByte ();
wiP->nRobotHitSound = cf.ReadShort ();
wiP->nWallHitVClip = cf.ReadByte ();
wiP->nWallHitSound = cf.ReadShort ();
wiP->fireCount = cf.ReadByte ();
wiP->nAmmoUsage = cf.ReadByte ();
wiP->nVClipIndex = cf.ReadByte ();
wiP->destructible = cf.ReadByte ();
wiP->matter = cf.ReadByte ();
wiP->bounce = cf.ReadByte ();
wiP->homingFlag = cf.ReadByte ();
wiP->dum1 = cf.ReadByte (); 
wiP->dum2 = cf.ReadByte ();
wiP->dum3 = cf.ReadByte ();
wiP->xEnergyUsage = cf.ReadFix ();
wiP->xFireWait = cf.ReadFix ();
wiP->bitmap.index = cf.ReadShort ();
wiP->blob_size = cf.ReadFix ();
wiP->xFlashSize = cf.ReadFix ();
wiP->xImpactSize = cf.ReadFix ();
for (i = 0; i < NDL; i++)
	wiP->strength [i] = cf.ReadFix ();
for (i = 0; i < NDL; i++)
	wiP->speed [i] = cf.ReadFix ();
wiP->mass = cf.ReadFix ();
wiP->drag = cf.ReadFix ();
wiP->thrust = cf.ReadFix ();
wiP->poLenToWidthRatio = cf.ReadFix ();
wiP->light = cf.ReadFix ();
wiP->lifetime = cf.ReadFix ();
wiP->xDamageRadius = cf.ReadFix ();
wiP->picture.index = cf.ReadShort ();

#if PRINT_WEAPON_INFO
PrintLog (0, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,{",
	wiP->renderType,
	wiP->nModel,
	wiP->nInnerModel,
	wiP->persistent,
	wiP->nFlashVClip,
	wiP->flashSound,
	wiP->nRobotHitVClip,
	wiP->nRobotHitSound,
	wiP->nWallHitVClip,
	wiP->nWallHitSound,
	wiP->fireCount,
	wiP->nAmmoUsage,
	wiP->nVClipIndex,
	wiP->destructible,
	wiP->matter,
	wiP->bounce,
	wiP->homingFlag,
	wiP->dum1, 
	wiP->dum2,
	wiP->dum3,
	wiP->xEnergyUsage,
	wiP->xFireWait,
	wiP->bitmap.index,
	wiP->blob_size,
	wiP->xFlashSize,
	wiP->xImpactSize);
for (i = 0; i < NDL; i++)
	PrintLog (0, "%s%d", i ? "," : "", wiP->strength [i]);
PrintLog (1, "},{");
for (i = 0; i < NDL; i++)
	PrintLog (0, "%s%d", i ? "," : "", wiP->speed [i]);
PrintLog (0, "},%d,%d,%d,%d,%d,%d,%d,{%d}},\n",
	wiP->mass,
	wiP->drag,
	wiP->thrust,
	wiP->poLenToWidthRatio,
	wiP->light,
	wiP->lifetime,
	wiP->xDamageRadius,
	wiP->picture.index);
#endif
}

//------------------------------------------------------------------------------
// the following values are needed for compiler settings causing struct members 
// not to be byte aligned. If D2X-XL is compiled with such settings, the size of
// the Descent data structures in memory is bigger than on disk. This needs to
// be compensated when reading such data structures from disk, or needing to skip
// them in the disk files.

#define D1_ROBOT_INFO_SIZE			486
#define D1_WEAPON_INFO_SIZE		115
#define JOINTPOS_SIZE				8
#define JOINTLIST_SIZE				4
#define POWERUP_TYPE_INFO_SIZE	16
#define POLYMODEL_SIZE				734
#define PLAYER_SHIP_SIZE			132
#define MODEL_DATA_SIZE_OFFS		4


typedef struct tD1TextureHeader {
	char	name [8];
	ubyte frame; //bits 0-5 anim frame num, bit 6 abm flag
	ubyte xsize; //low 8 bits here, 4 more bits in size2
	ubyte ysize; //low 8 bits here, 4 more bits in size2
	ubyte flag; //see BM_FLAG_XXX
	ubyte ave_color; //palette index of average color
	uint	offset; //relative to end of directory
} tD1TextureHeader;

typedef struct tD1SoundHeader {
	char name [8];
	int length; //size in bytes
	int data_length; //actually the same as above
	int offset; //relative to end of directory
} tD1SoundHeader;


void BMReadGameDataD1 (CFile& cf)
{
	int				h, i, j, v10DataOffset;
#if 1
	D1_tmap_info	t;
	//D1Robot_info	r;
#endif
	tWallClip		*pw;	
	tTexMapInfo		*pt;
	tRobotInfo		*pr;
	CPolyModel		model;
	ubyte				tmpSounds [D1_MAX_SOUNDS];

v10DataOffset = cf.ReadInt ();
cf.Read (&gameData.pig.tex.nTextures [1], sizeof (int), 1);
j = (gameData.pig.tex.nTextures [1] == 70) ? 70 : D1_MAX_TEXTURES;
/*---*/PrintLog (1, "Loading %d texture indices\n", j);
//cf.Read (gameData.pig.tex.bmIndex [1], sizeof (tBitmapIndex), D1_MAX_TEXTURES);
ReadBitmapIndices (gameData.pig.tex.bmIndex [1], D1_MAX_TEXTURES, cf);
BuildTextureIndex (1, D1_MAX_TEXTURES);
PrintLog (-1);

/*---*/PrintLog (1, "Loading %d texture descriptions\n", j);
for (i = 0, pt = &gameData.pig.tex.tMapInfo [1][0]; i < j; i++, pt++) {
#if DBG
	cf.Read (t.filename, sizeof (t.filename), 1);
#else
	cf.Seek (sizeof (t.filename), SEEK_CUR);
#endif
	pt->flags = (ubyte) cf.ReadByte ();
	pt->lighting = cf.ReadFix ();
	pt->damage = cf.ReadFix ();
	pt->nEffectClip = cf.ReadInt ();
	pt->slide_u = 
	pt->slide_v = 0;
	pt->destroyed = -1;
	}
PrintLog (-1);

cf.Read (Sounds [1], sizeof (ubyte), D1_MAX_SOUNDS);
cf.Read (AltSounds [1], sizeof (ubyte), D1_MAX_SOUNDS);

/*---*/PrintLog (1, "Initializing %d sounds\n", D1_MAX_SOUNDS);
if (gameOpts->sound.bUseD1Sounds) {
	memcpy (Sounds [1] + D1_MAX_SOUNDS, Sounds [0] + D1_MAX_SOUNDS, MAX_SOUNDS - D1_MAX_SOUNDS);
	memcpy (AltSounds [1] + D1_MAX_SOUNDS, AltSounds [0] + D1_MAX_SOUNDS, MAX_SOUNDS - D1_MAX_SOUNDS);
	}
else {
	memcpy (Sounds [1], Sounds [0], MAX_SOUNDS);
	memcpy (AltSounds [1], AltSounds [0], MAX_SOUNDS);
	}
for (i = 0; i < D1_MAX_SOUNDS; i++) {
	if (Sounds [1][i] == 255)
		Sounds [1][i] = Sounds [0][i];
	if (AltSounds [1][i] == 255)
		AltSounds [1][i] = AltSounds [0][i];
	}
PrintLog (-1);

gameData.effects.nClips [1] = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d animation clips\n", gameData.effects.nClips [1]);
ReadVideoClips (gameData.effects.vClips [1], D1_VCLIP_MAXNUM, cf);
PrintLog (-1);

gameData.effects.nEffects [1] = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d animation descriptions\n", gameData.effects.nClips [1]);
ReadEffectClips (gameData.effects.effects [1], D1_MAX_EFFECTS, cf);
PrintLog (-1);

gameData.walls.nAnims [1] = cf.ReadInt ();
/*---*/PrintLog (1, "Loading %d wall animations\n", gameData.walls.nAnims [1]);
for (i = 0, pw = &gameData.walls.anims [1][0]; i < D1_MAX_WALL_ANIMS; i++, pw++) {
	//cf.Read (&w, sizeof (w), 1);
	pw->xTotalTime = cf.ReadFix ();
	pw->nFrameCount = cf.ReadShort ();
	for (j = 0; j < D1_MAX_CLIP_FRAMES; j++)
		pw->frames [j] = cf.ReadShort ();
	pw->openSound = cf.ReadShort ();
	pw->closeSound = cf.ReadShort ();
	pw->flags = cf.ReadShort ();
	cf.Read (pw->filename, sizeof (pw->filename), 1);
	pw->pad = (char) cf.ReadByte ();
	}
PrintLog (-1);

/*---*/PrintLog (1, "Loading %d robot descriptions\n", gameData.bots.nTypes [1]);
cf.Read (&gameData.bots.nTypes [1], sizeof (int), 1);
PrintLog (-1);
gameData.bots.info [1] = gameData.bots.info [0];
if (!gameOpts->sound.bUseD1Sounds) {
	PrintLog (-1);
	return;
	}

for (i = 0, pr = &gameData.bots.info [1][0]; i < D1_MAX_ROBOT_TYPES; i++, pr++) {
	//cf.Read (&r, sizeof (r), 1);
	cf.Seek (
		sizeof (int) * 3 + 
		(sizeof (CFixVector) + sizeof (ubyte)) * MAX_GUNS + 
		sizeof (short) * 5 +
		sizeof (sbyte) * 7 +
		sizeof (fix) * 4 +
		sizeof (fix) * 7 * NDL +
		sizeof (sbyte) * 2 * NDL,
		SEEK_CUR);

	pr->seeSound = (ubyte) cf.ReadByte ();
	pr->attackSound = (ubyte) cf.ReadByte ();
	pr->clawSound = (ubyte) cf.ReadByte ();
	cf.Seek (
		JOINTLIST_SIZE * (MAX_GUNS + 1) * N_ANIM_STATES +
		sizeof (int),
		SEEK_CUR);
	pr->always_0xabcd = 0xabcd;   
	}         

cf.Seek (
	sizeof (int) +
	JOINTPOS_SIZE * D1_MAX_ROBOT_JOINTS +
	sizeof (int) +
	D1_WEAPON_INFO_SIZE * D1_MAX_WEAPON_TYPES +
	sizeof (int) +
	POWERUP_TYPE_INFO_SIZE * MAX_POWERUP_TYPES_D1,
	SEEK_CUR);

i = cf.ReadInt ();
/*---*/PrintLog (1, "Acquiring model data size of %d polymodels\n", i);
for (h = 0; i; i--) {
	cf.Seek (MODEL_DATA_SIZE_OFFS, SEEK_CUR);
	model.SetDataSize (cf.ReadInt ());
	h += model.DataSize ();
	cf.Seek (POLYMODEL_SIZE - MODEL_DATA_SIZE_OFFS - sizeof (int), SEEK_CUR);
	}
cf.Seek (
	h +
	sizeof (tBitmapIndex) * D1_MAX_GAUGE_BMS +
	sizeof (int) * 2 * D1_MAX_POLYGON_MODELS +
	sizeof (tBitmapIndex) * D1_MAX_OBJ_BITMAPS +
	sizeof (ushort) * D1_MAX_OBJ_BITMAPS +
	PLAYER_SHIP_SIZE +
	sizeof (int) +
	sizeof (tBitmapIndex) * D1_N_COCKPIT_BITMAPS,
	SEEK_CUR);
PrintLog (-1);

/*---*/PrintLog (1, "Loading sound data\n", i);
cf.Read (tmpSounds, sizeof (ubyte), D1_MAX_SOUNDS);
PrintLog (-1);

//for (i = 0, pr = &gameData.bots.info [1][0]; i < gameData.bots.nTypes [1]; i++, pr++) 
pr = gameData.bots.info [1] + 17;
/*---*/PrintLog (1, "Initializing sound data\n", i);
for (i = 0; i < D1_MAX_SOUNDS; i++) {
	if (Sounds [1][i] == tmpSounds [pr->seeSound])
		pr->seeSound = i;
	if (Sounds [1][i] == tmpSounds [pr->attackSound])
		pr->attackSound = i;
	if (Sounds [1][i] == tmpSounds [pr->clawSound])
		pr->clawSound = i;
	}
pr = gameData.bots.info [1] + 23;
for (i = 0; i < D1_MAX_SOUNDS; i++) {
	if (Sounds [1][i] == tmpSounds [pr->seeSound])
		pr->seeSound = i;
	if (Sounds [1][i] == tmpSounds [pr->attackSound])
		pr->attackSound = i;
	if (Sounds [1][i] == tmpSounds [pr->clawSound])
		pr->clawSound = i;
	}
cf.Read (tmpSounds, sizeof (ubyte), D1_MAX_SOUNDS);
//	for (i = 0, pr = &gameData.bots.info [1][0]; i < gameData.bots.nTypes [1]; i++, pr++) {
pr = gameData.bots.info [1] + 17;
for (i = 0; i < D1_MAX_SOUNDS; i++) {
	if (AltSounds [1][i] == tmpSounds [pr->seeSound])
		pr->seeSound = i;
	if (AltSounds [1][i] == tmpSounds [pr->attackSound])
		pr->attackSound = i;
	if (AltSounds [1][i] == tmpSounds [pr->clawSound])
		pr->clawSound = i;
	}
pr = gameData.bots.info [1] + 23;
for (i = 0; i < D1_MAX_SOUNDS; i++) {
	if (AltSounds [1][i] == tmpSounds [pr->seeSound])
		pr->seeSound = i;
	if (AltSounds [1][i] == tmpSounds [pr->attackSound])
		pr->attackSound = i;
	if (AltSounds [1][i] == tmpSounds [pr->clawSound])
		pr->clawSound = i;
	}
PrintLog (-1);

#if 0
cf.Seek (v10DataOffset, SEEK_SET);
i = cf.ReadInt ();
j = cf.ReadInt ();
cf.Seek (i * sizeof (tD1TextureHeader), SEEK_CUR);
gameStates.app.bD1Mission = 1;
for (i = 0; i < j; i++) {
	cf.Read (&gameData.pig.sound.sounds [1][i].szName, sizeof (gameData.pig.sound.sounds [1][i].szName), 1);
	cf.Seek (sizeof (tD1SoundHeader) - sizeof (gameData.pig.sound.sounds [1][i].szName), SEEK_CUR);
	}
#endif
}

//------------------------------------------------------------------------------

void BMReadWeaponInfoD1 (CFile& cf)
{
cf.Seek (
	sizeof (int) +
	sizeof (int) +
	sizeof (tBitmapIndex) * D1_MAX_TEXTURES +
	sizeof (D1_tmap_info) * D1_MAX_TEXTURES +
	sizeof (ubyte) * D1_MAX_SOUNDS +
	sizeof (ubyte) * D1_MAX_SOUNDS +
	sizeof (int) +
	sizeof (tVideoClip) * D1_VCLIP_MAXNUM +
	sizeof (int) +
	sizeof (D1_eclip) * D1_MAX_EFFECTS +
	sizeof (int) +
	sizeof (tD1WallClip) * D1_MAX_WALL_ANIMS +
	sizeof (int) +
	sizeof (D1Robot_info) * D1_MAX_ROBOT_TYPES +
	sizeof (int) +
	sizeof (tJointPos) * D1_MAX_ROBOT_JOINTS,
	SEEK_CUR);
gameData.weapons.nTypes [1] = cf.ReadInt ();
#if PRINT_WEAPON_INFO
PrintLog (1, "\nCD1WeaponInfo defaultWeaponInfosD1 [] = {\n");
#endif
for (int i = 0; i < gameData.weapons.nTypes [1]; i++)
	BMReadWeaponInfoD1N (cf, i);
#if PRINT_WEAPON_INFO
PrintLog (-1, "};\n\n");
#endif
}

//------------------------------------------------------------------------------

void LoadTextureBrightness (const char *pszLevel, int *brightnessP)
{
	CFile		cf;
	char		szFile [FILENAME_LEN];
	int		i, *pb;

if (!brightnessP)
	brightnessP = gameData.pig.tex.brightness.Buffer ();
CFile::ChangeFilenameExtension (szFile, pszLevel, ".lgt");
if (cf.Open (szFile, gameFolders.szDataDir [0], "rb", 0) &&
	 (cf.Read (brightnessP, sizeof (*brightnessP) * MAX_WALL_TEXTURES, 1) == 1)) {
	for (i = MAX_WALL_TEXTURES, pb = gameData.pig.tex.brightness.Buffer (); i; i--, pb++)
		*pb = INTEL_INT (*pb);
	cf.Close ();
	}
}

//-----------------------------------------------------------------------------

void InitDefaultShipProps (void)
{
defaultShipProps.nModel = 108;
defaultShipProps.nExplVClip = 58;
defaultShipProps.mass = 262144;
defaultShipProps.drag = 2162;
defaultShipProps.maxThrust = 511180;
defaultShipProps.reverseThrust = 0;
defaultShipProps.brakes = 0;
defaultShipProps.wiggle = I2X (1) / 2;
defaultShipProps.maxRotThrust = 9175;
defaultShipProps.gunPoints [0].Create (146013, -59748, 35756);
defaultShipProps.gunPoints [0].Create (-147477, -59892, 34430);
defaultShipProps.gunPoints [0].Create (222008, -118473, 148201);
defaultShipProps.gunPoints [0].Create (-223479, -118213, 148302);
defaultShipProps.gunPoints [0].Create (153026, -185, -91405);
defaultShipProps.gunPoints [0].Create (-156840, -185, -91405);
defaultShipProps.gunPoints [0].Create (1608, -87663, 184978);
defaultShipProps.gunPoints [0].Create (-1608, -87663, -190825);
}

//-----------------------------------------------------------------------------

void SetDefaultShipProps (void)
{
gameData.pig.ship.player->brakes = defaultShipProps.brakes;
gameData.pig.ship.player->drag = defaultShipProps.drag;
gameData.pig.ship.player->mass = defaultShipProps.mass;
gameData.pig.ship.player->maxThrust = defaultShipProps.maxThrust;
gameData.pig.ship.player->maxRotThrust = defaultShipProps.maxRotThrust;
gameData.pig.ship.player->reverseThrust = defaultShipProps.reverseThrust;
gameData.pig.ship.player->brakes = defaultShipProps.brakes;
gameData.pig.ship.player->wiggle = defaultShipProps.wiggle;
}

//-----------------------------------------------------------------------------

void SetDefaultWeaponProps (void)
{
if (!EGI_FLAG (bAllowCustomWeapons, 0, 0, 1)) {
	for (int i = 0; i < int (sizeofa (defaultWeaponInfoD2)); i++)
		gameData.weapons.info [i] = defaultWeaponInfoD2 [i];
	}
}

//------------------------------------------------------------------------------
//eof
