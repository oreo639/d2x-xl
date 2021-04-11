#ifdef HAVE_CONFIG_H 
#	include <conf.h> 
#endif 

#include "descent.h"
#include "text.h"
#include "error.h"
#include "u_mem.h"
#include "interp.h"
#include "menu.h"
#include "hiresmodels.h"

// ----------------------------------------------------------------------------

tReplacementModel replacementModels [] = {
	// player ship
	{"pyrogl", "pyrogl.pol", 108, 0, 0, -1}, 
	{"pyrogl", "pyrogl.pol", 109, 0, 0, -1}, 
	{"pyrogl", "pyrogl.pol", 110, 0, 0, -1}, 
	//{NULL, NULL, 110, 0, 0, -1}, 	//filename NULL means this is an additional model number to be used with the last listed oof filename
	// robots
	{"smallhulk", NULL, 0, 1, 0, -1}, 
	{"mediumlifter", NULL, 2, 1, 0, -1}, 
	{"spider", NULL, 4, 1, 0, -1}, 
	{"class1drone", NULL, 6, 1, 0, -1}, 
	{"class2drone", NULL, 8, 1, 0, -1}, 
	{"class1driller-cloaked", NULL, 10, 1, 0, -1}, 
	{"class2supervisor", NULL, 14, 1, 0, -1}, 
	{"secondarylifter", NULL, 15, 1, 0, -1}, 
	{"class1heavydriller", NULL, 17, 1, 0, -1}, 
	{"class3gopher", NULL, 19, 1, 0, -1}, 
	{"class1platform", NULL, 20, 1, 0, -1}, 
	{"class2platform", NULL, 21, 1, 0, -1}, 
	{"smallspider", NULL, 23, 1, 0, -1}, 
	{"fusionhulk", NULL, 25, 1, 0, -1}, 
	{"superhulk", NULL, 26, 1, 0, -1}, 
	{"d1boss1", NULL, 28, 1, 0, -1}, 
	{NULL, NULL, 50, 1, 0, -1}, 
	{"cloakedlifter", NULL, 29, 1, 0, -1}, 
	{"class1driller", NULL, 31, 1, 0, -1}, 
	{"mediumhulk", NULL, 33, 1, 0, -1}, 
	{"advancedlifter", NULL, 35, 1, 0, -1}, 
	{"ptmcdefense", NULL, 37, 1, 0, -1}, 
	{"d1boss2", NULL, 39, 1, 0, -1}, 
	{"bper", NULL, 40, 1, 0, -1}, 
	{"smelter", NULL, 41, 1, 0, -1}, 
	{"icespider", NULL, 43, 1, 0, -1}, 
	{"bulkdestroyer", NULL, 44, 1, 0, -1}, 
	{"trnracer", NULL, 45, 1, 0, -1}, 
	{"foxattackbot", NULL, 46, 1, 0, -1}, 
	{"sidarm", NULL, 47, 1, 0, -1}, 
	{"redfattyboss", NULL, 49, 1, 0, -1}, 
	{"guidebot", NULL, 51, 1, 0, -1}, 
	{"eviltwin", NULL, 54, 1, 0, -1}, 
	{"itsc", NULL, 55, 1, 0, -1}, 
	{"itdroid", NULL, 56, 1, 0, -1}, 
	{"pest", NULL, 58, 1, 0, -1}, 
	{"pig", NULL, 60, 1, 0, -1}, 
	{"d-claw", NULL, 62, 1, 0, -1}, 
	{"redhornet", NULL, 64, 1, 0, -1}, 
	{"thief", NULL, 65, 1, 0, -1}, 
	{"seeker", NULL, 67, 1, 0, -1}, 
	{"e-bandit", NULL, 68, 1, 0, -1}, 
	{"fireboss", NULL, 69, 1, 0, -1}, 
	{"waterboss", NULL, 70, 1, 0, -1}, 
	{"boarshead", NULL, 71, 1, 0, -1}, 
	{"greenspider", NULL, 72, 1, 0, -1}, 
	{"omegabot", NULL, 73, 1, 0, -1}, 
	{"louguard", NULL, 75, 1, 0, -1}, 
	{"alienboss1", NULL, 76, 1, 0, -1}, 
	{"redfattyjunior", NULL, 77, 1, 0, -1}, 
	{"d-claw-cloaked", NULL, 78, 1, 0, -1}, 
	{NULL, NULL, 79, 1, 0, -1}, 
	{"smelter-cloaked", NULL, 80, 1, 0, -1}, 
	{"smelterclone-cloaked", NULL, 81, 1, 0, -1}, 
	{"omegaspawn", NULL, 82, 1, 0, -1}, 
	{"smelterclone", NULL, 83, 1, 0, -1}, 
	{"omegaclone", NULL, 85, 1, 0, -1}, 
	{"bperclone", NULL, 86, 1, 0, -1}, 
	{"spiderclone", NULL, 87, 1, 0, -1}, 
	{"spiderspawn", NULL, 88, 1, 0, -1}, 
	{"iceboss", NULL, 89, 1, 0, -1}, 
	{"spiderspawnclone", NULL, 90, 1, 0, -1}, 
	{"alienboss2", NULL, 91, 1, 0, -1}, 
	// Vertigo robots
	{"compactlifter", NULL, 166, 1, 0, -1}, 
	{"fervid99", NULL, 167, 1, 0, -1}, 
	{"fiddler", NULL, 168, 1, 0, -1}, 
	{"class2heavydriller", NULL, 169, 1, 0, -1}, 
	{"smelter2", NULL, 170, 1, 0, -1}, 
	{"max", NULL, 171, 1, 0, -1}, 
	{"sniperng", NULL, 172, 1, 0, -1}, 
	{"logikill", NULL, 173, 1, 0, -1}, 
	{"canary", NULL, 174, 1, 0, -1}, 
	{"vertigoboss", NULL, 175, 1, 0, -1}, 
	{"redguard", NULL, 176, 1, 0, -1}, 
	{"spike", NULL, 177, 1, 0, -1}, 
	//reactors
	{"reactorD1", NULL, 93, 1, 0, -1},
	{"reactorD1-destr", NULL, 94, 1, 0, -1},
	{"reactorD2-1", NULL, 95, 1, 0, -1},
	{"reactorD2-1-destr", NULL, 96, 1, 0, -1},
	{"reactorD2-2", NULL, 97, 1, 0, -1},
	{"reactorD2-2-destr", NULL, 98, 1, 0, -1},
	{"reactorD2-3", NULL, 99, 1, 0, -1},
	{"reactorD2-3-destr", NULL, 100, 1, 0, -1},
	{"reactorD2-4", NULL, 101, 1, 0, -1},
	{"reactorD2-4-destr", NULL, 102, 1, 0, -1},
	{"reactorD2-5", NULL, 103, 1, 0, -1},
	{"reactorD2-5-destr", NULL, 104, 1, 0, -1},
	{"reactorD2-6", NULL, 105, 1, 0, -1},
	{"reactorD2-6-destr", NULL, 106, 1, 0, -1},
	//gun shots
	{"laser1-inner", NULL, 114, 1, 0, LASER_ID},
	{"laser1-outer", NULL, 111, 1, 0, LASER_ID},
	{"laser2-inner", NULL, 118, 1, 0, LASER_ID + 1},
	{"laser2-outer", NULL, 115, 1, 0, LASER_ID + 1},
	{"laser3-inner", NULL, 122, 1, 0, LASER_ID + 2},
	{"laser3-outer", NULL, 119, 1, 0, LASER_ID + 2},
	{"laser4-inner", NULL, 126, 1, 0, LASER_ID + 3},
	{"laser4-outer", NULL, 123, 1, 0, LASER_ID + 3},
	{"laser5-inner", NULL, 146, 1, 0, SUPERLASER_ID},
	{"laser5-outer", NULL, 143, 1, 0, SUPERLASER_ID},
	{"laser6-inner", NULL, 150, 1, 0, SUPERLASER_ID + 1},
	{"laser6-outer", NULL, 147, 1, 0, SUPERLASER_ID + 1},
	{"botlas1-inner", NULL, 139, 1, 0, ROBOT_RED_LASER_ID},
	{"botlas1-outer", NULL, 138, 1, 0, ROBOT_RED_LASER_ID},
	{"botlas2-inner", NULL, 130, 1, 0, ROBOT_BLUE_LASER_ID},
	{"botlas2-outer", NULL, 129, 1, 0, ROBOT_BLUE_LASER_ID},
	{"botlas4-inner", NULL, 141, 1, 0, ROBOT_GREEN_LASER_ID},
	{"botlas4-outer", NULL, 140, 1, 0, ROBOT_GREEN_LASER_ID},
	{"botlas6-inner", NULL, 157, 1, 0, ROBOT_WHITE_LASER_ID},
	{"botlas6-outer", NULL, 156, 1, 0, ROBOT_WHITE_LASER_ID},
	{"lightvulcan", NULL, 155, 1, 0, ROBOT_VULCAN_ID},
	{"fusion-outer", NULL, 131, 1, 0, FUSION_ID},
	{"fusion-inner", NULL, 132, 1, 0, FUSION_ID},
	{"flare", NULL, 128, 1, 0, FLARE_ID},
	//marker
	{"marker", NULL, 107, 1, 0, -1},
	//mine
	{"smallmine", NULL, 159, 1, 0, -1},
	//missiles
	{"concussion", NULL, 127, 0, 1, -1}, 
	{NULL, NULL, 137, 0, 1, -1}, 
	{"homer", NULL, 133, 0, 1, -1}, 
	{NULL, NULL, 136, 0, 1, -1}, 
	{"smartmsl", NULL, 134, 0, 1, -1}, 
	{NULL, NULL, 162, 0, 1, -1}, 
	{"mega", NULL, 135, 0, 1, -1}, 
	{NULL, NULL, 142, 0, 1, -1}, 
	{"flashmsl", NULL, 151, 0, 1, -1}, 
	{NULL, NULL, 158, 0, 1, -1}, 
	{NULL, NULL, 165, 0, 1, -1}, 
	{"guided", NULL, 152, 0, 1, -1}, 
	{"mercury", NULL, 153, 0, 1, -1}, 
	{NULL, NULL, 161, 0, 1, -1}, 
	{"eshaker", NULL, 154, 0, 1, -1}, 
	{NULL, NULL, 163, 0, 1, -1}, 
	{"shakrsub", NULL, 160, 0, 1, -1},
	//powerups
	{"pminepack", "pminpack.pol", MAX_POLYGON_MODELS - 1, 0, 1, -1},
	{"proxmine", "proxmine.pol", MAX_POLYGON_MODELS - 2, 0, 1, -1},
	{"sminepack", "sminpack.pol", MAX_POLYGON_MODELS - 3, 0, 1, -1},
	{"smartmine", "smrtmine.pol", MAX_POLYGON_MODELS - 4, 0, 1, -1},
	{"concussion4", "concpack.pol", MAX_POLYGON_MODELS - 5, 0, 1, -1},
	{"homer4", "homrpack.pol", MAX_POLYGON_MODELS - 6, 0, 1, -1},
	{"flash4", "flshpack.pol", MAX_POLYGON_MODELS - 7, 0, 1, -1},
	{"guided4", "guidpack.pol", MAX_POLYGON_MODELS - 8, 0, 1, -1},
	{"mercury4", "mercpack.pol", MAX_POLYGON_MODELS - 9, 0, 1, -1},
	{"laser", NULL, MAX_POLYGON_MODELS - 10, 0, 1, -1},
	{"vulcan", NULL, MAX_POLYGON_MODELS - 11, 0, 1, -1},
	{"spreadfire", NULL, MAX_POLYGON_MODELS - 12, 0, 1, -1},
	{"plasma", NULL, MAX_POLYGON_MODELS - 13, 0, 1, -1},
	{"fusion", NULL, MAX_POLYGON_MODELS - 14, 0, 1, -1},
	{"superlaser", NULL, MAX_POLYGON_MODELS - 15, 0, 1, -1},
	{"gauss", NULL, MAX_POLYGON_MODELS - 16, 0, 1, -1},
	{"helix", NULL, MAX_POLYGON_MODELS - 17, 0, 1, -1},
	{"phoenix", NULL, MAX_POLYGON_MODELS - 18, 0, 1, -1},
	{"omega", NULL, MAX_POLYGON_MODELS - 19, 0, 1, -1},
	{"quadlaser", NULL, MAX_POLYGON_MODELS - 20, 0, 1, -1},
	{"afterburner", NULL, MAX_POLYGON_MODELS - 21, 0, 1, -1},
	{"headlight", NULL, MAX_POLYGON_MODELS - 22, 0, 1, -1},
	{"ammorack", NULL, MAX_POLYGON_MODELS - 23, 0, 1, -1},
	{"converter", NULL, MAX_POLYGON_MODELS - 24, 0, 1, -1},
	{"fullmap", NULL, MAX_POLYGON_MODELS - 25, 0, 1, -1},
	{"cloak", NULL, MAX_POLYGON_MODELS - 26, 0, 1, -1},
	{"invul", NULL, MAX_POLYGON_MODELS - 27, 0, 1, -1},
	{"extralife", NULL, MAX_POLYGON_MODELS - 28, 0, 1, -1},
	{"bluekey", NULL, MAX_POLYGON_MODELS - 29, 0, 1, -1},
	{"redkey", NULL, MAX_POLYGON_MODELS - 30, 0, 1, -1},
	{"goldkey", NULL, MAX_POLYGON_MODELS - 31, 0, 1, -1},
	{"vulcanammo", NULL, MAX_POLYGON_MODELS - 32, 0, 1, -1},
	{"slowmotion", NULL, MAX_POLYGON_MODELS - 33, 0, 1, -1},
	{"bullettime", NULL, MAX_POLYGON_MODELS - 34, 0, 1, -1},
	{"hostage", NULL, HOSTAGE_MODEL, 0, 1, -1},
	{"bullet", NULL, BULLET_MODEL, 0, 1, -1}
#if 0 //DBG
	,
	{"cockpit", NULL, COCKPIT_MODEL, 0, 1, -1}
#endif
	};

// ----------------------------------------------------------------------------

int32_t ReplacementModelCount (void)
{
return sizeofa (replacementModels);
}

// ----------------------------------------------------------------------------

void InitReplacementModels (void)
{
for (int32_t i = 0; i < 2; i++) {
	gameData.modelData.modelToOOF [i].Clear ();
	gameData.modelData.modelToASE [i].Clear ();
	}
gameData.modelData.modelToPOL.Clear ();
}

// ----------------------------------------------------------------------------

int16_t LoadLoresModel (int16_t i)
{
	CFile			cf;
	CPolyModel*	pModel;
	int16_t			nModel, j = sizeofa (replacementModels);
	char			szModel [FILENAME_LEN];

sprintf (szModel, "model%d.pol", replacementModels [i].nModel);
if (!(replacementModels [i].pszLores && 
	  (cf.Open (replacementModels [i].pszLores, gameFolders.game.szData [0], "rb", 0) ||
	   cf.Open (szModel, gameFolders.game.szData [0], "rb", 0))))
	return ++i;
nModel = replacementModels [i].nModel;
#if DBG
if (nModel == nDbgModel)
	BRP;
#endif
pModel = ((gameStates.app.bFixModels && gameStates.app.bAltModels) ? gameData.modelData.polyModels [2] : gameData.modelData.polyModels [0]) + nModel;
pModel->Destroy ();
if (!pModel->Read (1, 0, cf)) {
	cf.Close ();
	return ++i;
	}
pModel->ReadData (gameData.modelData.polyModels [1] + nModel, cf);
cf.Close ();
pModel->SetRad (pModel->Size (), 1);
do {
	gameData.modelData.modelToPOL [nModel] = pModel;
	} while ((++i < j) && !replacementModels [i].pszHires);
gameData.modelData.nLoresModels++;
return i;
}

// ----------------------------------------------------------------------------

int16_t LoadOOFModel (OOF::CModel *po, int16_t i, int32_t bCustom)
{
	int16_t nModel = replacementModels [i].nModel;

if (gameData.modelData.modelToOOF [bCustom != 0][nModel])
	return i + 1;

	int16_t	j = sizeofa (replacementModels);
	char	szModel [2][FILENAME_LEN];

sprintf (szModel [0], "model%d.oof", replacementModels [i].nModel);
if (replacementModels [i].pszHires)
	sprintf (szModel [1], "%s.oof", replacementModels [i].pszHires);
else
	szModel [1][0] = '\0';
#if DBG
if (nModel == nDbgModel)
	BRP;
#endif
if (!(po->Read (szModel [1], nModel, replacementModels [i].bFlipV, bCustom) || 
	   po->Read (szModel [0], nModel, replacementModels [i].bFlipV, bCustom)))
	return 0;
do {
	gameData.modelData.modelToOOF [bCustom != 0][replacementModels [i].nModel] = po;
	} while ((++i < j) && !replacementModels [i].pszHires);
gameData.modelData.nHiresModels++;
if (bCustom)
	gameOpts->render.bHiresModels [0] = 1;
return i;
}

// ----------------------------------------------------------------------------

int16_t LoadASEModel (ASE::CModel *pa, int16_t i, int32_t bCustom)
{
	int16_t nModel = replacementModels [i].nModel;

#if DBG
if (nModel == nDbgModel)
	BRP;
#endif
if (gameData.modelData.modelToASE [bCustom != 0][nModel])
	return i + 1;

	int16_t	j = sizeofa (replacementModels);
	char	szModel [2][FILENAME_LEN];

sprintf (szModel [0], "model%d.ase", nModel);
if (replacementModels [i].pszHires)
	sprintf (szModel [1], "%s.ase", replacementModels [i].pszHires);
else
	szModel [1][0] = '\0';
#if 0//DBG
while (!ASE_ReadFile (szModel [1], pa, replacementModels [i].nType, bCustom))
	;
#endif
if (!(pa->Read (szModel [1], nModel, bCustom) || 
	   pa->Read (szModel [0], nModel, bCustom)))
	return 0;
do {
	gameData.modelData.modelToASE [bCustom != 0][replacementModels [i].nModel] = pa;
	} while ((++i < j) && !replacementModels [i].pszHires);
gameData.modelData.nHiresModels++;
if (bCustom)
	gameOpts->render.bHiresModels [0] = 1;
return i;
}

// ----------------------------------------------------------------------------

int16_t LoadHiresModel (int16_t nModel, int32_t bCustom, const char* filename)
{
	int16_t	j = sizeofa (replacementModels);

#if DBG
if ((nDbgModel >= 0) && (replacementModels [nModel].nModel == nDbgModel))
	BRP;
#endif
if (filename)
	replacementModels [nModel].pszHires = filename;
bool bLog = (replacementModels [nModel].pszHires != NULL) && (gameStates.app.nLogLevel > 1);
if (bLog)
	PrintLog (1, "Loading model %d (%s)\n", replacementModels [nModel].nModel, replacementModels [nModel].pszHires);
#if DBG
if (replacementModels [nModel].nModel == nDbgModel)
	BRP;
#endif
if ((j = LoadASEModel (gameData.modelData.aseModels [bCustom != 0] + gameData.modelData.nHiresModels, nModel, bCustom))) {
	if (bLog)
		PrintLog (-1);
	return j;
	}
#if 1
if (bCustom && (j = LoadOOFModel (gameData.modelData.oofModels [bCustom != 0] + gameData.modelData.nHiresModels, nModel, bCustom))) {
	if (bLog)
		PrintLog (-1);
	return j;
	}
#endif
if (bLog)
	PrintLog (-1);
return bCustom ? ++nModel : LoadLoresModel (nModel);
}

//------------------------------------------------------------------------------

static int32_t loadIdx;
static int32_t loadOp = 0;

static int32_t LoadModelsPoll (CMenu& menu, int32_t& key, int32_t nCurItem, int32_t nState)
{
if (nState)
	return nCurItem;

//paletteManager.ResumeEffect ();
if (loadOp == 0) {
	loadIdx = LoadHiresModel (loadIdx, 0);
	if (loadIdx >= (int32_t) sizeofa (replacementModels)) {
		loadOp = 1;
		loadIdx = 0;
		}
	}
else if (loadOp == 1) {
	loadIdx = LoadLoresModel (loadIdx);
	if (loadIdx >= (int32_t) sizeofa (replacementModels)) {
		key = -2;
		//paletteManager.ResumeEffect ();
		return nCurItem;
		}
	}
menu [0].Value ()++;
menu [0].Rebuild ();
key = 0;
//paletteManager.ResumeEffect ();
return nCurItem;
}

//------------------------------------------------------------------------------

int32_t ModelsGaugeSize (void)
{
	int32_t h, i, j;

for (h = i = 0, j = sizeofa (replacementModels); i < j; i++)
	if (replacementModels [i].pszHires) {
		if (gameOpts->render.bHiresModels [0])
			h++;
		if (replacementModels [i].pszLores)
			h++;
		}
return h = (gameOpts->render.bHiresModels [0] + 1) * (sizeofa (replacementModels) - 1);
}

//------------------------------------------------------------------------------

void LoadModelsGauge (void)
{
loadIdx = 0;
loadOp = gameOpts->render.bHiresModels [0] ? 0 : 1;
ProgressBar (TXT_LOADING_MODELS, 1, 0, ModelsGaugeSize (), LoadModelsPoll); 
}

// ----------------------------------------------------------------------------

void LoadHiresModels (int32_t bCustom)
{
if (!bCustom) {
	InitReplacementModels ();
	gameData.modelData.nHiresModels = 0;
	}
if (gameStates.app.bNostalgia)
	gameOpts->render.bHiresModels [0] = 0;
else /*if (gameOpts->render.bHiresModels [0])*/ {
	gameOpts->render.bHiresModels [0] = gameOpts->render.bHiresModels [1];
	if (!bCustom && gameStates.app.bProgressBars && gameOpts->menus.nStyle)
		LoadModelsGauge ();
	else {
		int16_t	i = 0, j = sizeofa (replacementModels);
		if (!bCustom)
			messageBox.Show (TXT_LOADING_MODELS);
		if (bCustom || gameOpts->render.bHiresModels [0]) {
			while (i < j)
				i = LoadHiresModel (i, bCustom);
			i = 0;
			}
		if (!bCustom) {
			while (i < j)
				i = LoadLoresModel (i);
			}
		}
	messageBox.Clear ();
	}
}

// ----------------------------------------------------------------------------

void FreeHiresModel (int32_t nModel)
{
for (int32_t i = 0, l = gameData.modelData.nHiresModels; i < l; i++) {
	for (int32_t j = 0; j < 2; j++) {
		if (gameData.modelData.aseModels [j][i].m_nModel == nModel) {
			if (gameData.modelData.modelToASE [j][nModel]) {
				gameData.modelData.modelToASE [j][nModel] = NULL;
				gameData.modelData.aseModels [j][i].Destroy ();
				gameData.modelData.nHiresModels--;
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------

void FreeHiresModels (int32_t bCustom)
{
	int32_t	h, i, j, l;

for (i = 0, l = gameData.modelData.nHiresModels; i < l; i++)
	for (j = bCustom; j < 2; j++) {
		if (0 <= (h = gameData.modelData.oofModels [j][i].m_nModel)) {
#if DBG
			if (h == nDbgModel)
				BRP;
#endif
			if (gameData.modelData.modelToOOF [j][h]) {
				gameData.modelData.modelToOOF [j][h] = NULL;
				gameData.modelData.oofModels [j][i].Destroy ();
				gameData.modelData.nHiresModels--;
				}
			}
		if (0 <= (h = gameData.modelData.aseModels [j][i].m_nModel)) {
#if DBG
			if (h == nDbgModel)
				BRP;
#endif
			if (gameData.modelData.modelToASE [j][h]) {
				gameData.modelData.modelToASE [j][h] = NULL;
				gameData.modelData.aseModels [j][i].Destroy ();
				gameData.modelData.nHiresModels--;
				}
			}
		}
for (j = bCustom; j < 2; j++) {
	gameData.modelData.modelToOOF [j].Clear ();
	gameData.modelData.modelToASE [j].Clear ();
	}
}

// ----------------------------------------------------------------------------
// eof
