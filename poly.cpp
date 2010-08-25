// Copyright (c) 1997 Bryan Aamot
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include "stdafx.h"
#include "dle-xp.h"
#include "dlcDoc.h"
#include "mineview.h"
#include "define.h"
#include "types.h"
#include "global.h"
#include "poly.h"
#include "io.h"
/*
extern HPEN hPenRed, hPenDkRed, hPenGreen, hPenDkGreen, hPenBlue, hPenCyan, hPenDkCyan, 
	 hPenMagenta, hPenYellow, hPenDkYellow, hPenOrange, hPenGold, hPenGray, 
	 hPenLtGray;
extern HRGN hrgnBackground, hrgnLowerBar, hrgnTopBar, hrgnAll;
*/
//-----------------------------------------------------------------------
// CONSTANTS
//-----------------------------------------------------------------------
#define OP_EOF        0	/* eof                                 */
#define OP_DEFPOINTS  1	/* defpoints                (not used) */
#define OP_FLATPOLY   2	/* flat-shaded polygon                 */
#define OP_TMAPPOLY   3	/* texture-mapped polygon              */
#define OP_SORTNORM   4	/* sort by normal                      */
#define OP_RODBM      5	/* rod bitmap               (not used) */
#define OP_SUBCALL    6	/* call a subobject                    */
#define OP_DEFP_START 7	/* defpoints with start                */
#define OP_GLOW       8	/* m_info.glow value for next poly            */

#define MAX_INTERP_COLORS 100
#define MAX_POINTS_PER_POLY 25

//-----------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------
#define W(p)   (*((INT16 *) (p)))
#define WP(p)  ((INT16 *) (p))
#define VP(p)  ((CFixVector*) (p))
#define calcNormal(a, b)
#define glNormal3fv(a)
#define glColor3ub(a, b, c)
#define glBegin(a)
#define glVertex3i(x, y, z)
#define glEnd()
#define glTexCoord2fv(a)

#define UINTW INT32

//-----------------------------------------------------------------------
// local prototypes
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// globals
//-----------------------------------------------------------------------
static CGameObject* renderObject;
static CPolyModel* renderModel;
static tModelRenderData modelRenderData;
static CVertex renderOffset;
static INT16 glow_num = -1;
static CDoubleVector normal;	// Storage for calculated surface normal
static tModelRenderPoly* faceP;
static INT32 pt, pt0, n_points;
static INT32 lastObjectType = -1;
static INT32 lastObjectId = -1;
static APOINT poly_xy [MAX_POLYMODEL_POINTS];

//-----------------------------------------------------------------------
// FIX MultiplyFix ()
//-----------------------------------------------------------------------

FIX MultiplyFix (FIX a, FIX b) {
  return (FIX) ((double (a) * double (b))/F1_0);
}

//-----------------------------------------------------------------------
// DrawModel ()
//-----------------------------------------------------------------------

void CMineView::DrawModel () 
{
renderModel.Draw ();
InterpModelData (renderModelData);
}

//-----------------------------------------------------------------------
// set_model_points ()
//
// Rotates, translates, then sets screen points (xy) for 3d model points
//-----------------------------------------------------------------------

void CMineView::SetModelPoints (INT32 start, INT32 end) 
{
CGameObject *objP = renderObject;
CVertex		pt;

for (INT32 i = start; i < end; i++) {
	//FIX x0 = modelRenderData.points[i].v.x;
	//FIX y0 = modelRenderData.points[i].v.y;
	//FIX z0 = modelRenderData.points[i].v.z;

	// rotate point using Objects () rotation matrix
	pt = objP->m_info.orient * modelRenderData.points [i];
	// set point to be in world coordinates
	pt += objP->m_info.pos;
	// now that points are relative to set screen xy points (poly_xy)
	m_view.Project (pt, poly_xy [i]);
	}
}


//-----------------------------------------------------------------------
// draw_poly
//
// Action - Draws a polygon if it is facing the outward.
//          Used global device context handle set by SetupModel ()
//-----------------------------------------------------------------------

void CMineView::DrawPoly (tModelRenderPoly* p) 
{
if (m_view.CheckNormal (renderObject, &p->m_info.offset, &p->m_info.normal)) 
	p->Draw (m_pDC);
}

//-----------------------------------------------------------------------
//
// Calls the CGameObject interpreter to render an CGameObject.
// The CGameObject renderer is really a seperate pipeline. returns true if drew
//
//-----------------------------------------------------------------------

void CPolyModel::Render (UINT8 *p) 
{
while (W (p) != OP_EOF) {
	switch (W (p)) {
		// Point Definitions with Start Offset:
		// 2  UINT16      n_points       number of points
		// 4  UINT16      start_point    starting point
		// 6  UINT16      unknown
		// 8  CFixVector  pts[n_points]  x, y, z data
		case OP_DEFP_START: {
			pt0 = W (p+4);
			n_points = W (p+2);
			modelRenderData.n_points += n_points;
			assert (W (p+6)==0);
			assert (modelRenderData.n_points < MAX_POLYMODEL_POINTS);
			assert (pt0+n_points < MAX_POLYMODEL_POINTS);
			for (pt = 0;pt < n_points; pt++) 
				modelRenderData.points [pt+pt0] = VP (p+8)[pt] + renderOffset;
			SetModelPoints (pt0, pt0+n_points);
			p += W (p+2)*sizeof (CFixVector) + 8;
			break;
			}
		// Flat Shaded Polygon:
		// 2  UINT16     n_verts
		// 4  CFixVector vector1
		// 16 CFixVector vector2
		// 28 UINT16     color
		// 30 UINT16     verts[n_verts]
		case OP_FLATPOLY: {
			faceP = &modelRenderData.polys[modelRenderData.n_polys];
			faceP->n_verts = W (p+2);
			faceP->offset = *VP (p+4);
			faceP->normal = *VP (p+16);
			for (pt = 0; pt < faceP->n_verts; pt++) 
				faceP->verts[pt] = WP (p+30)[pt];
			DrawPoly (faceP);
			p += 30 + ((faceP->n_verts&~1)+1)*2;
			break;
			}
		// Texture Mapped Polygon:
		// 2  UINT16     n_verts
		// 4  CFixVector vector1
		// 16 CFixVector vector2
		// 28 UINT16     nBaseTex
		// 30 UINT16     verts[n_verts]
		// -- UVL        uvls[n_verts]
		case OP_TMAPPOLY: {
			faceP = &modelRenderData.polys[modelRenderData.n_polys];
			faceP->n_verts  = W (p+2);
			assert (faceP->n_verts>=MIN_POLYMODEL_POLY_POINTS);
			assert (faceP->n_verts<=MAX_POLYMODEL_POLY_POINTS);
			assert (modelRenderData.n_polys < MAX_POLYMODEL_POLYS);
			faceP->offset   = *VP (p+4);
			faceP->normal   = *VP (p+16);
			faceP->color    = -1;
			faceP->nBaseTex = W (p+28);
			faceP->glow_num = glow_num;
			for (pt = 0; pt < faceP->n_verts; pt++) 
				faceP->verts[pt] = WP (p+30)[pt];
			p += 30 + ((faceP->n_verts&~1)+1)*2;
			DrawPoly (faceP);
			p += faceP->n_verts * 12;
			break;
			}
		// Sort by Normal
		// 2  UINT16      unknown
		// 4  CFixVector  Front Model normal
		// 16 CFixVector  Back Model normal
		// 28 UINT16      Front Model Offset
		// 30 UINT16      Back Model Offset
		case OP_SORTNORM: {
			/* = W (p+2); */
			/* = W (p+4); */
			/* = W (p+16); */
			if ( m_view.CheckNormal (renderObject, VP (p+4), VP (p+16)) ) {
			  InterpModelData (p + W (p+28));
			  InterpModelData (p + W (p+30));
				}
			else {
			  InterpModelData (p + W (p+30));
			  InterpModelData (p + W (p+28));
				}
			p += 32;
			break;
			}
		// Call a Sub Object
		// 2  UINT16     n_anims
		// 4  CFixVector offset
		// 16 UINT16     model offset
		case OP_SUBCALL: {
			renderOffset += *VP (p+4);
			InterpModelData (p + W (p+16));
			renderOffset -= *VP (p+4);
			p += 20;
			break;
			}
		// Glow Number for Next Poly
		// 2 UINTW  Glow_Value
		case OP_GLOW: {
			glow_num = W (p+2);
			p += 4;
			break;
			}
		default: 
			assert (0);
		}
	}
return;
}

//--------------------------------------------------------------------------
// read_UINTW ()
//
// Reads a UINT32 number and converts it UINTW (16 or 32 bit)
//--------------------------------------------------------------------------

UINTW read_UINTW (FILE *fp) 
{
UINT32 value;
fread (&value, sizeof (UINT32), 1, fp);
assert (value <= 0xffff);
return (UINTW)value;
}

//-----------------------------------------------------------------------
// ReadModelData ();
//-----------------------------------------------------------------------

#define FREAD(b)	fread (&b, sizeof (b), 1, file)

INT32 CPolyModel::Read (FILE* fp, bool bRenderData) 
{
if (bRenderData) {
	Release ();
	if (!(m_info.bRenderData = new UINT8 [m_info.dataSize]))
		return 0;
	return fread (m_info.renderData, m_info.dataSize, 1, fp) == 1;
	}
else {
	m_info.nModels = read_INT32 (fp);
	m_info.dataSize = read_INT32 (fp);
	m_info.data = NULL;
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].ptr = read_INT32 (fp);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].offset.Read (fp);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].norm.Read (fp);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].pnt.Read (fp);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].rad = read_FIX (fp);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].parent = (UINT8) read_INT8 (fp);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].vMin.Read (fp);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].vMax.Read (fp);
	m_info.vMin.Read (fp);
	m_info.vMax.Read (fp);
	m_info.rad = read_FIX (fp);
	m_info.textureCount = (UINT8) read_INT8 (fp);
	m_info.firstTexture = (UINT16) read_INT16 (fp);
	m_info.simplerModel = (UINT8) read_INT8 (fp);
	}
return 1;
}

//-----------------------------------------------------------------------

static bool ReadRenderModelData (FILE* fp, int nModels, int nId, bool bReadAll)
{
int skip = 0;
for (int i = 0; i < nModels; i++) {
	if (i == nId) {
		if (skip)
			fseek (fp, skip, SEEK_CUR);
		if (bReadAll)
			skip = 0;
		else
			return fread (renderModelData, modelDataSize [i], 1, fp) == 1;
		}
	else
		skip += modelDataSize [i];
	}
if (skip)
	fseek (fp, skip, SEEK_CUR);
return bReadAll;
}

//-----------------------------------------------------------------------

static int ReadRobotInfo (FILE* fp, int nRobots, int nId)
{
int nModel = -1;
int skip = 0;
for (i = 0; i < n; i++) {
	if (i == (UINT32) (objP->m_info.id - N_D2_ROBOT_TYPES)) {
		if (skip)
			fseek (fp, skip, SEEK_CUR);
		robotInfo.Read (fp);
		nModel = robotInfo.m_info.nModel;
		}
	else
		skip += sizeof (tRobotInfo);
	}
if (skip)
	fseek (fp, skip, SEEK_CUR);
return nModel;
}

//-----------------------------------------------------------------------

INT32 CMineView::ReadModelData (char* filename, bool bCustom = false) 
{
	FILE*		fp;

if (fopen_s (&file, filename, "rb"))
	return 1;


	UINT32	id;
	UINT32	i, n;

if (bCustom) {
	struct level_header level;
	char data[3];
	long position;

	fread (data, 3, 1, fp); // verify signature "DHF"
	if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') return 1;
	position = 3;
	while (!feof (fp)) {
		fseek (fp, position, SEEK_SET);
		if ((fread (&level, sizeof (struct level_header), 1, fp) != 1) ||
			 (level.size > 10000000L || level.size < 0)) {
				fclose (fp);
				return 1;
				}
		if (!strcmp (level.name, "d2x.ham")) {
			id = read_INT32 (fp);	  					   // read id
			if (id != 0x5848414DL) {
				fclose (fp);
				return 1;
				}
			read_UINTW (fp);                              // read version
			n  = read_UINTW (fp);                         // n_weapon_types
			fseek (fp, n * sizeof (WEAPON_INFO), SEEK_CUR);  // weapon_info
			n  = read_UINTW (fp);                         // n_robot_types
			for (i = 0; i < n; i++)
				RobotInfo (N_D2_ROBOT_TYPES + i)->Read (fp);
			n  = read_UINTW (fp);                         // n_robot_joints
			fseek (fp, n * sizeof (JOINTPOS), SEEK_CUR);     // robot_joints
			break;
			}
		position += sizeof (struct level_header) + level.size;
		}
	n = read_UINTW (fp);                          // n_curModels
	assert (n <= MAX_POLYGON_MODELS);
	for (i = 0; i < n; i++) 
		polyModels [N_D2_ROBOT_TYPES + i].Read (fp);
	for (i = 0; i < n; i++) 
		polyModels [N_D2_ROBOT_TYPES + i].Read (fp, true);
	}
else {
	id = read_INT32 (fp);	  					   // read id
	if (id != 0x214d4148L) {
		fclose (fp);
		return 1;
		}
	read_UINTW (fp);                              // read version
	n  = read_UINTW (fp);                         // n_tmap_info
	fseek (fp, n * sizeof (UINT16), SEEK_CUR);	 // bitmap_indicies
	fseek (fp, n * sizeof (TMAP_INFO), SEEK_CUR); // tmap_info
	n = read_UINTW (fp);                          // n_sounds
	fseek (fp, n * sizeof (UINT8), SEEK_CUR);     // sounds
	fseek (fp, n * sizeof (UINT8), SEEK_CUR);     // alt_sounds
	n = read_UINTW (fp);                          // n_vclips
	fseek (fp, n * sizeof (VCLIP), SEEK_CUR);     // video clips
	n = read_UINTW (fp);                          // n_eclips
	fseek (fp, n * sizeof (ECLIP), SEEK_CUR);     // effect clips
	n = read_UINTW (fp);                          // n_wclips
	fseek (fp, n * sizeof (WCLIP), SEEK_CUR);     // weapon clips
	n = read_UINTW (fp);                          // n_robots
	for (i = 0; i < n; i++) 
		RobotInfo (i)->Read (fp);
	n = read_UINTW (fp);                          // n_robot_joints
	fseek (fp, n * sizeof (JOINTPOS), SEEK_CUR);     // robot joints
	n = read_UINTW (fp);                          // n_weapon
	fseek (fp, n * sizeof (WEAPON_INFO), SEEK_CUR);  // weapon info
	n = read_UINTW (fp);                          // n_powerups
	fseek (fp, n * sizeof (POWERUP_TYPE_INFO), SEEK_CUR); // powerup info
	n = read_UINTW (fp);                          // n_curModels
	assert (n <= MAX_POLYGON_MODELS);
	for (i = 0; i < n; i++) 
		polyModels [i].Read (fp);
	for (i = 0; i < n; i++) 
		polyModels [i].Read (fp, true);
	}
fclose (fp);
return 0;
}

//-----------------------------------------------------------------------

CPolyModel* CMineView::RenderModel (CGameObject* objP)
{
	UINT32 nModel;

switch (objP->m_info.type) {
	case OBJ_PLAYER:
	case OBJ_COOP:
		nModel = D2_PLAYER_CLIP_NUMBER;
		break;
	case OBJ_WEAPON:
		nModel = MINE_CLIP_NUMBER;
		break;
	case OBJ_CNTRLCEN:
		switch (objP->m_info.id) {
			case 1:  nModel = 95;  break;
			case 2:  nModel = 97;  break;
			case 3:  nModel = 99;  break;
			case 4:  nModel = 101; break;
			case 5:  nModel = 103; break;
			case 6:  nModel = 105; break;
			default: nModel = 97;  break; // level 1's reactor
		}
		break;
	case OBJ_ROBOT:
		nModel = RobotInfo ((objP->m_info.id >= N_D2_ROBOT_TYPES) ? objP->m_info.id - N_D2_ROBOT_TYPES : objP->m_info.id)->m_info.nModel;
	default:
		return NULL;
	}
return polyModels + nModel;
}

//-----------------------------------------------------------------------
// SetupModel ()
//
// Action - sets the global handle used when drawing polygon models
//-----------------------------------------------------------------------

INT32 CMineView::SetupModel (CGameObject *objP) 
{
renderOffset.Clear ();
modelRenderData.n_points = 0;
glow_num = -1;

if (!renderModel = RenderModel (renderObject = objP))
	return 1;
if (renderModel->m_data.renderData)
	return 0;

strcpy_s (filename, sizeof (filename), descent2_path);
char *slash = strrchr (filename, '\\');
if (slash)
	*(slash+1) = '\0';
else
	filename[0] = '\0';

bool bCustom = (objP->m_info.type == OBJ_ROBOT) && (objP->m_info.id >= N_D2_ROBOT_TYPES);

if (bCustom) {
	char *psz = strstr (filename, "data");
	if (psz)
		*psz = '\0';
	}
strcat_s (filename, sizeof (filename), ((objP->m_info.type == OBJ_ROBOT) && (objP->m_info.id >= N_D2_ROBOT_TYPES)) 
			 ? "data\\d2x-xl.hog" 
          : (objP->m_info.type == OBJ_CAMBOT) 
			   ? "cambot.hxm" 
				: "descent2.ham");
if (ReadModelData (filename, bCustom))
	renderModel = NULL;
return renderModel == NULL;
}

//-----------------------------------------------------------------------

void tModelRenderPoly::Draw (CDC* pDC) 
{
  INT32 i, j;

POINT aPoints [MAX_POLYMODEL_POLY_POINTS];
for (i = 0; i < m_info.n_verts; i++) {
	j = m_info.verts [i];
	aPoints [i].x = poly_xy [j].x;
	aPoints [i].y = poly_xy [j].y;
	}
pDC->Polygon (aPoints, m_info.n_verts);
}

//-----------------------------------------------------------------------

