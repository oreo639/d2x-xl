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
static tRenderModel renderModel;
static UINT8* renderModelData;
static CGameObject * gpObject;
static CFixVector gOffset;
static INT16 glow_num = -1;
static double normal[3];	// Storage for calculated surface normal
static tRenderModel *panel;
static INT32 pt, pt0, n_points;
static INT32 last_object_type = -1;
static INT32 last_object_id = -1;
static APOINT poly_xy[MAX_POLY_MODEL_POINTS];

//-----------------------------------------------------------------------
// FIX MultiplyFix ()
//-----------------------------------------------------------------------

FIX MultiplyFix (FIX a, FIX b) {
  return (FIX) ((double (a) * double (b))/F1_0);
}

//-----------------------------------------------------------------------
// SetupModel ()
//
// Action - sets the global handle used when drawing polygon models
//-----------------------------------------------------------------------

INT32 CMineView::SetupModel (CGameObject *objP) 
{
  gOffset.Clear ();
  renderModel.n_points = 0;
  glow_num = -1;
  gpObject = objP;
  INT32 rc = 1; // set return code to fail
  FILE *file = NULL;
  char filename[256];

  // allocate memory if not already allocated
if (renderModel.polys == NULL)
	renderModel.polys = (tRenderModel *) malloc (MAX_POLYS*sizeof (tRenderModel));
if (!renderModel.polys) {
	DEBUGMSG ("SetupModel: Couldn't allocate model polygon.");
	goto abort;
	}
if (renderModelData == NULL)
		renderModelData = (UINT8 *) malloc (MAX_POLY_MODEL_SIZE);
if (!renderModelData) {
	DEBUGMSG ("SetupModel: Couldn't allocate model data.");
	goto abort;
	}

// read model data if necessary
if (last_object_type != objP->m_info.type || last_object_id != objP->m_info.id) {
	renderModelData[0] = OP_EOF;
	strcpy_s (filename, sizeof (filename), descent2_path);
	char *slash = strrchr (filename, '\\');
	if (slash)
		* (slash+1) = NULL;
	else
		filename[0] = NULL;
	if ((objP->m_info.type == OBJ_ROBOT) && (objP->m_info.id >= N_D2_ROBOT_TYPES)) {
		char *psz = strstr (filename, "data");
		if (psz)
			*psz = '\0';
		}
	strcat_s (filename, sizeof (filename), ((objP->m_info.type == OBJ_ROBOT) && (objP->m_info.id >= N_D2_ROBOT_TYPES)) 
				 ? "data\\d2x-xl.hog" 
	          : (objP->m_info.type == OBJ_CAMBOT) 
				   ? "cambot.hxm" 
					: "descent2.ham");
	fopen_s (&file, filename, "rb");
	if (!file) {
#if 0		
		sprintf_s (message, sizeof (message), "SetupModel: Couldn't open model file <%s>.", filename); 
		DEBUGMSG (message);
#endif		
		goto abort;
		}
	if (ReadModelData (file, objP)) {
#if 0		
		DEBUGMSG ("SetupModel: Couldn't read model data.");
#endif		
		goto abort;
		}
	last_object_type = objP->m_info.type;
	last_object_id = objP->m_info.id;
	}
rc = 0;

abort:

if (file) 
	fclose (file);
return rc;
}

//-----------------------------------------------------------------------
// DrawModel ()
//-----------------------------------------------------------------------

void CMineView::DrawModel () 
{
  InterpModelData (renderModelData);
}

//-----------------------------------------------------------------------
// set_model_points ()
//
// Rotates, translates, then sets screen points (xy) for 3d model points
//-----------------------------------------------------------------------

void CMineView::SetModelPoints (INT32 start, INT32 end) 
{
CGameObject *objP = gpObject;
CVertex		pt;

for (INT32 i = start; i < end; i++) {
	FIX x0 = renderModel.points[i].v.x;
	FIX y0 = renderModel.points[i].v.y;
	FIX z0 = renderModel.points[i].v.z;

	// rotate point using Objects () rotation matrix
	pt = objP->m_info.orient * renderModel.points [i];
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

void CMineView::DrawPoly (tRenderModel *p) 
{
  INT32 i, j;

if (m_view.CheckNormal (gpObject, &p->offset, &p->normal)) {
	POINT aPoints [MAX_POLY_POINTS];
	for (i = 0; i < p->n_verts; i++) {
		j = p->verts[i];
		aPoints [i].x = poly_xy [j].x;
		aPoints [i].y = poly_xy [j].y;
		}
	m_pDC->Polygon (aPoints, p->n_verts);
	}
}

//-----------------------------------------------------------------------
//
// Calls the CGameObject interpreter to render an CGameObject.
// The CGameObject renderer is really a seperate pipeline. returns true if drew
//
//-----------------------------------------------------------------------

void CMineView::InterpModelData (UINT8 *p) 
{
	assert (p);
	assert (renderModel.polys);

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
				renderModel.n_points += n_points;
				assert (W (p+6)==0);
				assert (renderModel.n_points < MAX_POLY_MODEL_POINTS);
				assert (pt0+n_points < MAX_POLY_MODEL_POINTS);
				for (pt=0;pt< n_points;pt++) {
					renderModel.points [pt+pt0] = VP (p+8)[pt] + gOffset;
				}
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
				panel = &renderModel.polys[renderModel.n_polys];
				panel->n_verts = W (p+2);
				panel->offset = *VP (p+4);
				panel->normal = *VP (p+16);
				for (pt=0;pt<panel->n_verts;pt++) {
					panel->verts[pt] = WP (p+30)[pt];
				}
				assert (panel->n_verts>=MIN_POLY_POINTS);
				assert (panel->n_verts<=MAX_POLY_POINTS);
				assert (renderModel.n_polys < MAX_POLYS);
				DrawPoly (panel);
				p += 30 + ((panel->n_verts&~1)+1)*2;
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
				panel = &renderModel.polys[renderModel.n_polys];
				panel->n_verts  = W (p+2);
				assert (panel->n_verts>=MIN_POLY_POINTS);
				assert (panel->n_verts<=MAX_POLY_POINTS);
				assert (renderModel.n_polys < MAX_POLYS);
				panel->offset   = *VP (p+4);
				panel->normal   = *VP (p+16);
				panel->color    = -1;
				panel->nBaseTex = W (p+28);
				panel->glow_num = glow_num;
				for (pt=0;pt<panel->n_verts;pt++) {
					panel->verts[pt] = WP (p+30)[pt];
				}
				p += 30 + ((panel->n_verts&~1)+1)*2;
				DrawPoly (panel);
				p += panel->n_verts * 12;
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
				assert (W (p+2)==0);
				assert (W (p+28)>0);
				assert (W (p+30)>0);
				if ( m_view.CheckNormal (gpObject, VP (p+4), VP (p+16)) ) {
				  InterpModelData (p + W (p+28));
				  InterpModelData (p + W (p+30));
				} else {
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
				assert (W (p+16)>0);
				/* = VP (p+4) */
				gOffset += *VP (p+4);
				InterpModelData (p + W (p+16));
				gOffset -= *VP (p+4);
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
			default: {
				assert (0);
			}
		}
	}
	return;
}

//--------------------------------------------------------------------------
// read_UINTW ()
//
// Reads a UINT32 number and converts it UINTW (16 or 32 bit)
//--------------------------------------------------------------------------
UINTW read_UINTW (FILE *fp) {
  UINT32 value;
  fread (&value, sizeof (UINT32), 1, fp);
  assert (value <= 0xffff);
  return (UINTW)value;
}

//-----------------------------------------------------------------------
// ReadModelData ();
//-----------------------------------------------------------------------

#define FREAD(b)	fread (&b, sizeof (b), 1, file)

void CPolyModel::Read (FILE* fp) 
{
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
	m_info.subModels [i].parent = read_UINT8 (fp);
for (int i = 0; i < MAX_SUBMODELS; i++)
	m_info.subModels [i].vMin.Read (fp);
for (int i = 0; i < MAX_SUBMODELS; i++)
	m_info.subModels [i].vMax.Read (fp);
m_info.vMin.Read (fp);
m_info.vMax.Read (fp);
m_info.rad = read_FIX (fp);
m_info.textureCount = read_UINT8 (fp);
m_info.firstTexture = read_UINT16 (fp);
m_info.simplerModel = read_UINT8 (fp);
}

//-----------------------------------------------------------------------

static void ReadRenderModelData (FILE* fp, int nModel)
{
int skip = 0;
for (int i = 0; i < n; i++) {
	if (i == nModel) {
		if (skip)
			fseek (fp, skip, SEEK_CUR);
		fread (renderModelData, modelDataSize [i], 1, fp);
		break; // were done!
		}
	else
		skip += modelDataSize [i];
	}
}

//-----------------------------------------------------------------------

INT32 CMineView::ReadModelData (FILE *fp, CGameObject *objP) 
{
	UINT32		id;
	UINT32		i, n, skip;
	UINT16		modelDataSize [MAX_POLYGON_MODELS];
	tPolyModel  curModel;
	tPolyModel  saveModel;
	CRobotInfo	robotInfo;
	UINT32		nModel;

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
	}

if ((objP->m_info.type == OBJ_CAMBOT) || ((objP->m_info.type == OBJ_ROBOT) && (objP->m_info.id >= N_D2_ROBOT_TYPES))) {
	struct level_header level;
	char data[3];
	long position;

	fread (data, 3, 1, fp); // verify signature "DHF"
	if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') return 1;
	position = 3;
	while (!feof (fp)) {
		fseek (fp, position, SEEK_SET);
		if (fread (&level, sizeof (struct level_header), 1, fp) != 1) return 1;
		if (level.size > 10000000L || level.size < 0) return 1;
		if (strcmp (level.name, "d2x.ham") == 0) {
			id = read_INT32 (fp);	  					   // read id
			if (id != 0x5848414DL) return 1;
			read_UINTW (fp);                              // read version
			n  = read_UINTW (fp);                         // n_weapon_types
			fseek (fp, n * sizeof (WEAPON_INFO), SEEK_CUR);  // weapon_info
			n  = read_UINTW (fp);                         // n_robot_types
			if (objP->m_info.type == OBJ_ROBOT) {
				for (i = 0; i < n; i++) {
					if (i == (UINT32) (objP->m_info.id - N_D2_ROBOT_TYPES)) {
						robotInfo.Read (fp);
						nModel = robotInfo.m_info.nModel;
						}
					else
						fseek (fp, sizeof (tRobotInfo), SEEK_CUR);   // skip robot info
					}
				}
			else
				fseek (fp, n * sizeof (tRobotInfo), SEEK_CUR);
			n  = read_UINTW (fp);                         // n_robot_joints
			fseek (fp, n * sizeof (JOINTPOS), SEEK_CUR);     // robot_joints
			break;
			}
		position += sizeof (struct level_header) + level.size;
		}
	n = read_UINTW (fp);                          // n_curModels
	assert (n <= MAX_POLYGON_MODELS);
	for (i = 0; i < n; i++) {
		ReadPolyModel (curModel, fp);
		modelDataSize [i] = (UINT16)curModel.m_info.dataSize;
		if (i == (UINT32) (nModel - N_D2_POLYGON_MODELS))
			memcpy (&saveModel, &curModel, sizeof (tPolyModel));
		}
	ReadRenderModelData (fp, nModel - N_D2_POLYGON_MODELS);
	}
else {
	id = read_INT32 (fp);	  					   // read id
	if (id != 0x214d4148L) {
//			printf ("Not a HAM fp");
		return 1;
		}
	read_UINTW (fp);                              // read version
	n  = read_UINTW (fp);                         // n_tmap_info
	fseek (fp, n * sizeof (UINT16), SEEK_CUR); // bitmap_indicies
	fseek (fp, n * sizeof (TMAP_INFO), SEEK_CUR);    // tmap_info
	n = read_UINTW (fp);                          // n_sounds
	fseek (fp, n * sizeof (UINT8), SEEK_CUR);        // sounds
	fseek (fp, n * sizeof (UINT8), SEEK_CUR);        // alt_sounds
	n = read_UINTW (fp);                          // n_vclips
	fseek (fp, n * sizeof (VCLIP), SEEK_CUR);        // video clips
	n = read_UINTW (fp);                          // n_eclips
	fseek (fp, n * sizeof (ECLIP), SEEK_CUR);        // effect clips
	n = read_UINTW (fp);                          // n_wclips
	fseek (fp, n * sizeof (WCLIP), SEEK_CUR);        // weapon clips
	n = read_UINTW (fp);                          // n_robots
	if (objP->m_info.type == OBJ_ROBOT) {
		for (i = 0; i < n; i++) {
			if (i == (UINT32) objP->m_info.id) {
				robotInfo.Read (fp);
				nModel = robotInfo.m_info.nModel;
				}
			else {
				fseek (fp, sizeof (tRobotInfo), SEEK_CUR);   // skip robot info
				}
			}
		}
	else {
		fseek (fp, n * sizeof (tRobotInfo), SEEK_CUR);
		}
	n = read_UINTW (fp);                          // n_robot_joints
	fseek (fp, n * sizeof (JOINTPOS), SEEK_CUR);     // robot joints
	n = read_UINTW (fp);                          // n_weapon
	fseek (fp, n * sizeof (WEAPON_INFO), SEEK_CUR);  // weapon info
	n = read_UINTW (fp);                          // n_powerups
	fseek (fp, n * sizeof (POWERUP_TYPE_INFO), SEEK_CUR); // powerup info
	n = read_UINTW (fp);                          // n_curModels
	assert (n<=MAX_POLYGON_MODELS);
	for (i=0;i<n;i++) {
		ReadPolyModel (curModel, fp);
		modelDataSize[i] = (UINT16)curModel.modelDataSize;
		if (i== (UINT32) nModel) {
			memcpy (&saveModel, &curModel, sizeof (tPolyModel));
			}
		}
	ReadRenderModelData (fp, nModel);
	for (i = 0; i < n; i++) {
		if (i == (UINT32) nModel) {
			fread (renderModelData, modelDataSize[i], 1, fp);
			break; // were done!
			}
		else {
			fseek (fp , modelDataSize [i], SEEK_CUR);
		}
	}
}
return 0;
}
