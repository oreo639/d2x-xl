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
extern HPEN hPenRed,hPenDkRed,hPenGreen,hPenDkGreen,hPenBlue,hPenCyan,hPenDkCyan,
	 hPenMagenta,hPenYellow,hPenDkYellow,hPenOrange,hPenGold,hPenGray,
	 hPenLtGray;
extern HRGN hrgnBackground,hrgnLowerBar,hrgnTopBar,hrgnAll;
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
#define OP_GLOW       8	/* glow value for next poly            */

#define MAX_INTERP_COLORS 100
#define MAX_POINTS_PER_POLY 25

//-----------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------
#define W(p)   (*((INT16 *)(p)))
#define WP(p)  ((INT16 *)(p))
#define VP(p)  ((CFixVector* )(p))
#define calcNormal(a,b)
#define glNormal3fv(a)
#define glColor3ub(a,b,c)
#define glBegin(a)
#define glVertex3i(x,y,z)
#define glEnd()
#define glTexCoord2fv(a)

#define UINTW INT32

//-----------------------------------------------------------------------
// local prototypes
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// globals
//-----------------------------------------------------------------------
static MODEL gModel;
static UINT8 *gModelData;
static CGameObject * gpObject;
static CFixVector gOffset;
static INT16 glow_num = -1;
static double normal[3];	// Storage for calculated surface normal
static POLY *panel;
static INT32 pt,pt0,n_points;
static INT32 last_object_type = -1;
static INT32 last_object_id = -1;
static APOINT poly_xy[MAX_POLY_MODEL_POINTS];

//-----------------------------------------------------------------------
// FIX MultiplyFix()
//-----------------------------------------------------------------------

FIX MultiplyFix(FIX a, FIX b) {
  return (FIX) ((double(a) * double(b))/F1_0);
}

//-----------------------------------------------------------------------
// SetupModel()
//
// Action - sets the global handle used when drawing polygon models
//-----------------------------------------------------------------------

INT32 CMineView::SetupModel(CMine *mine, CGameObject *objP) 
{
  gOffset.x = 0;
  gOffset.y = 0;
  gOffset.z = 0;
  gModel.n_points = 0;
  glow_num = -1;
  gpObject = objP;
  INT32 rc = 1; // set return code to fail
  FILE *file = NULL;
  char filename[256];

  // allocate memory if not already allocated
if (gModel.polys == NULL)
	gModel.polys = (POLY *) malloc (MAX_POLYS*sizeof (POLY));
if (!gModel.polys) {
	DEBUGMSG ("SetupModel: Couldn't allocate model polygon.");
	goto abort;
	}
if (gModelData == NULL)
		gModelData = (UINT8 *) malloc (MAX_POLY_MODEL_SIZE);
if (!gModelData) {
	DEBUGMSG ("SetupModel: Couldn't allocate model data.");
	goto abort;
	}

// read model data if necessary
if (last_object_type != objP->type || last_object_id != objP->id) {
	gModelData[0] = OP_EOF;
	strcpy_s(filename, sizeof (filename), descent2_path);
	char *slash = strrchr (filename,'\\');
	if (slash)
		*(slash+1) = NULL;
	else
		filename[0] = NULL;
	if ((objP->type == OBJ_ROBOT) && (objP->id >= N_D2_ROBOT_TYPES)) {
		char *psz = strstr (filename, "data");
		if (psz)
			*psz = '\0';
		}
	strcat_s (filename, sizeof (filename), ((objP->type == OBJ_ROBOT) && (objP->id >= N_D2_ROBOT_TYPES)) 
				 ? "data\\d2x-xl.hog" 
	          : (objP->type == OBJ_CAMBOT) 
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
	if (ReadModelData(file,objP)) {
#if 0		
		DEBUGMSG ("SetupModel: Couldn't read model data.");
#endif		
		goto abort;
		}
	last_object_type = objP->type;
	last_object_id = objP->id;
	}
rc = 0;

abort:

if (file) 
	fclose(file);
return rc;
}

//-----------------------------------------------------------------------
// DrawModel()
//-----------------------------------------------------------------------

void CMineView::DrawModel() 
{
  InterpModelData(gModelData);
}

//-----------------------------------------------------------------------
// set_model_points()
//
// Rotates, translates, then sets screen points (xy) for 3d model points
//-----------------------------------------------------------------------

void CMineView::SetModelPoints(INT32 start, INT32 end) 
{
  CGameObject *objP = gpObject;
  CFixVector pt;
  INT32 i;
  for (i=start;i<end;i++) {
	FIX x0 = gModel.points[i].x;
	FIX y0 = gModel.points[i].y;
	FIX z0 = gModel.points[i].z;

	// rotate point using Objects () rotation matrix
	pt.x = (MultiplyFix(objP->orient.rvec.x,x0)
			+ MultiplyFix(objP->orient.uvec.x,y0)
			+ MultiplyFix(objP->orient.fvec.x,z0));
	pt.y = (MultiplyFix(objP->orient.rvec.y,x0)
			+ MultiplyFix(objP->orient.uvec.y,y0)
			+ MultiplyFix(objP->orient.fvec.y,z0));
	pt.z = (MultiplyFix(objP->orient.rvec.z,x0)
			+ MultiplyFix(objP->orient.uvec.z,y0)
			+ MultiplyFix(objP->orient.fvec.z,z0));

	// set point to be in world coordinates
	pt.x += objP->pos.x;
	pt.y += objP->pos.y;
	pt.z += objP->pos.z;

	// now that points are relative to mine, set screen xy points (poly_xy)
	m_matrix.SetPoint(&pt,poly_xy + i);
  }
}


//-----------------------------------------------------------------------
// draw_poly
//
// Action - Draws a polygon if it is facing the outward.
//          Used global device context handle set by SetupModel()
//-----------------------------------------------------------------------
void CMineView::DrawPoly(POLY *p) {
  INT32 i,j;
  if (m_matrix.CheckNormal(gpObject, &p->offset,&p->normal)) {
#if 1
	POINT aPoints[MAX_POLY_POINTS];
	for (i=0;i<p->n_verts;i++) {
	  j = p->verts[i];
	  aPoints[i].x = poly_xy[j].x;
	  aPoints[i].y = poly_xy[j].y;
	}
	m_pDC->Polygon(aPoints, p->n_verts);
#else
	for (i=0;i<p->n_verts;i++) {
	  j = p->verts[i];
	  if (i==0) {
		m_pDC->MoveTo(poly_xy[j].x,poly_xy[j].y);
	  } else {
		m_pDC->LineTo(poly_xy[j].x+1,poly_xy[j].y+1);
		m_pDC->LineTo(poly_xy[j].x+1,poly_xy[j].y-1);
		m_pDC->LineTo(poly_xy[j].x-1,poly_xy[j].y-1);
		m_pDC->LineTo(poly_xy[j].x-1,poly_xy[j].y+1);
	  }
	}
	j = p->verts[0];
	m_pDC->LineTo(poly_xy[j].x,poly_xy[j].y);
#endif
  }
}

//-----------------------------------------------------------------------
//
// Calls the CGameObject interpreter to render an CGameObject.
// The CGameObject renderer is really a seperate pipeline. returns true if drew
//
//-----------------------------------------------------------------------

void CMineView::InterpModelData(UINT8 *p) 
{
	assert(p);
	assert(gModel.polys);

	while (W(p) != OP_EOF) {
		switch (W(p)) {
			// Point Definitions with Start Offset:
			// 2  UINT16      n_points       number of points
			// 4  UINT16      start_point    starting point
			// 6  UINT16      unknown
			// 8  CFixVector  pts[n_points]  x,y,z data
			case OP_DEFP_START: {
				pt0 = W(p+4);
				n_points = W(p+2);
				gModel.n_points += n_points;
				assert(W(p+6)==0);
				assert(gModel.n_points < MAX_POLY_MODEL_POINTS);
				assert(pt0+n_points < MAX_POLY_MODEL_POINTS);
				for (pt=0;pt< n_points;pt++) {
					gModel.points[pt+pt0].x = VP(p+8)[pt].x + gOffset.x;
					gModel.points[pt+pt0].y = VP(p+8)[pt].y + gOffset.y;
					gModel.points[pt+pt0].z = VP(p+8)[pt].z + gOffset.z;
				}
				SetModelPoints(pt0,pt0+n_points);
				p += W(p+2)*sizeof (CFixVector) + 8;
				break;
			}
			// Flat Shaded Polygon:
			// 2  UINT16     n_verts
			// 4  CFixVector vector1
			// 16 CFixVector vector2
			// 28 UINT16     color
			// 30 UINT16     verts[n_verts]
			case OP_FLATPOLY: {
				panel = &gModel.polys[gModel.n_polys];
				panel->n_verts = W(p+2);
				panel->offset   = *VP(p+4);
				panel->normal   = *VP(p+16);
				for (pt=0;pt<panel->n_verts;pt++) {
					panel->verts[pt] = WP(p+30)[pt];
				}
				assert(panel->n_verts>=MIN_POLY_POINTS);
				assert(panel->n_verts<=MAX_POLY_POINTS);
				assert(gModel.n_polys < MAX_POLYS);
				DrawPoly(panel);
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
				panel = &gModel.polys[gModel.n_polys];
				panel->n_verts  = W(p+2);
				assert(panel->n_verts>=MIN_POLY_POINTS);
				assert(panel->n_verts<=MAX_POLY_POINTS);
				assert(gModel.n_polys < MAX_POLYS);
				panel->offset   = *VP(p+4);
				panel->normal   = *VP(p+16);
				panel->color    = -1;
				panel->nBaseTex = W(p+28);
				panel->glow_num = glow_num;
				for (pt=0;pt<panel->n_verts;pt++) {
					panel->verts[pt] = WP(p+30)[pt];
				}
				p += 30 + ((panel->n_verts&~1)+1)*2;
				DrawPoly(panel);
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
				/* = W(p+2); */
				/* = W(p+4); */
				/* = W(p+16); */
				assert(W(p+2)==0);
				assert(W(p+28)>0);
				assert(W(p+30)>0);
				if ( m_matrix.CheckNormal(gpObject, VP(p+4),VP(p+16)) ) {
				  InterpModelData(p + W(p+28));
				  InterpModelData(p + W(p+30));
				} else {
				  InterpModelData(p + W(p+30));
				  InterpModelData(p + W(p+28));
				}
				p += 32;
				break;
			}
			// Call a Sub Object
			// 2  UINT16     n_anims
			// 4  CFixVector offset
			// 16 UINT16     model offset
			case OP_SUBCALL: {
				assert(W(p+16)>0);
				/* = VP(p+4) */
				gOffset.x += VP(p+4)->x;
				gOffset.y += VP(p+4)->y;
				gOffset.z += VP(p+4)->z;
				InterpModelData(p + W(p+16));
				gOffset.x -= VP(p+4)->x;
				gOffset.y -= VP(p+4)->y;
				gOffset.z -= VP(p+4)->z;
				p += 20;
				break;
			}
			// Glow Number for Next Poly
			// 2 UINTW  Glow_Value
			case OP_GLOW: {
				glow_num = W(p+2);
				p += 4;
				break;
			}
			default: {
				assert(0);
			}
		}
	}
	return;
}

//--------------------------------------------------------------------------
// read_UINTW()
//
// Reads a UINT32 number and converts it UINTW (16 or 32 bit)
//--------------------------------------------------------------------------
UINTW read_UINTW(FILE *fp) {
  UINT32 value;
  fread(&value, sizeof (UINT32), 1, fp);
  assert(value <= 0xffff);
  return (UINTW)value;
}

//-----------------------------------------------------------------------
// ReadModelData();
//-----------------------------------------------------------------------

#define FREAD(b)	fread (&b, sizeof (b), 1, file)

void CMineView::ReadPolyModel (POLYMODEL& polyModel, FILE *file) 
{
FREAD (polyModel.n_models);
FREAD (polyModel.model_data_size);
fseek (file, sizeof (INT32), SEEK_CUR);
polyModel.model_data = NULL;
FREAD (polyModel.submodel_ptrs);
FREAD (polyModel.submodel_offsets);
FREAD (polyModel.submodel_norms);	  // norm for sep plane
FREAD (polyModel.submodel_pnts);	  // point on sep plane
FREAD (polyModel.submodel_rads);	  // radius for each submodel
FREAD (polyModel.submodel_parents);  // what is parent for each submodel
FREAD (polyModel.submodel_mins);
FREAD (polyModel.submodel_maxs);
FREAD (polyModel.mins);
FREAD (polyModel.maxs);				  // min, max for whole model
FREAD (polyModel.rad);
FREAD (polyModel.n_textures);
FREAD (polyModel.first_texture);
FREAD (polyModel.simpler_model);			  // alternate model with less detail (0 if none, model_num+1 else)
assert(polyModel.model_data_size <= MAX_POLY_MODEL_SIZE);
}

//-----------------------------------------------------------------------

INT32 CMineView::ReadModelData(FILE *file, CGameObject *objP) 
{
	UINT32     id;
	UINT32     i,n;
	UINT16     model_data_size[MAX_POLYGON_MODELS];
	POLYMODEL  polyModel;
	POLYMODEL  save_model;
	ROBOT_INFO robot_info;
	UINT32     model_num;

	switch (objP->type) {
		case OBJ_PLAYER:
		case OBJ_COOP:
			model_num = D2_PLAYER_CLIP_NUMBER;
			break;
		case OBJ_WEAPON:
			model_num = MINE_CLIP_NUMBER;
			break;
		case OBJ_CNTRLCEN:
			switch(objP->id) {
				case 1:  model_num = 95;  break;
				case 2:  model_num = 97;  break;
				case 3:  model_num = 99;  break;
				case 4:  model_num = 101; break;
				case 5:  model_num = 103; break;
				case 6:  model_num = 105; break;
				default: model_num = 97;  break; // level 1's reactor
			}
			break;
			// if it is a robot, then read the id from the HAM file
	}

	if ((objP->type == OBJ_CAMBOT) || ((objP->type == OBJ_ROBOT) && (objP->id >= N_D2_ROBOT_TYPES))) {
		struct level_header level;
		char data[3];
		long position;

		fread(data,3,1,file); // verify signature "DHF"
		if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') return 1;
		position = 3;
		while(!feof(file)) {
			fseek(file,position,SEEK_SET);
			if (fread(&level,sizeof (struct level_header),1,file) != 1) return 1;
			if (level.size > 10000000L || level.size < 0) return 1;
			if (strcmp(level.name,"d2x.ham") == 0) {
				id = read_INT32(file);	  					   // read id
				if (id != 0x5848414DL) return 1;
				read_UINTW(file);                              // read version
				n  = read_UINTW(file);                         // n_weapon_types
				fseek(file,n * sizeof (WEAPON_INFO),SEEK_CUR);  // weapon_info
				n  = read_UINTW(file);                         // n_robot_types
				if (objP->type == OBJ_ROBOT) {
					for (i=0;i<n;i++) {
						if (i == (UINT32) (objP->id - N_D2_ROBOT_TYPES)) {
							fread(&robot_info,sizeof (ROBOT_INFO),1,file);// read robot info
							model_num = robot_info.model_num;
						} else {
							fseek(file,sizeof (ROBOT_INFO),SEEK_CUR);   // skip robot info
						}
					}
				} else {
					fseek(file,n * sizeof (ROBOT_INFO),SEEK_CUR);
				}
				n  = read_UINTW(file);                         // n_robot_joints
				fseek(file,n * sizeof (JOINTPOS),SEEK_CUR);     // robot_joints
				break;
			}
			position += sizeof (struct level_header) + level.size;
		}
		n = read_UINTW(file);                          // n_polyModels
		assert(n<=MAX_POLYGON_MODELS);
		for (i = 0; i < n; i++) {
			ReadPolyModel (polyModel, file);
			model_data_size[i] = (UINT16)polyModel.model_data_size;
			if (i==(UINT32) (model_num - N_D2_POLYGON_MODELS))
				memcpy(&save_model,&polyModel,sizeof (POLYMODEL));
		}
		for (i=0;i<n;i++) {
			if (i==(UINT32) (model_num - N_D2_POLYGON_MODELS)) {
				fread(gModelData, model_data_size[i],1,file);
				break; // were done!
			} else {
				fseek(file , model_data_size[i],SEEK_CUR);
			}
		}
	} else {
		id = read_INT32(file);	  					   // read id
		if (id != 0x214d4148L) {
//			printf("Not a HAM file");
			return 1;
		}
		read_UINTW(file);                              // read version
		n  = read_UINTW(file);                         // n_tmap_info
		fseek(file,n * sizeof (UINT16),SEEK_CUR); // bitmap_indicies
		fseek(file,n * sizeof (TMAP_INFO),SEEK_CUR);    // tmap_info
		n = read_UINTW(file);                          // n_sounds
		fseek(file,n * sizeof (UINT8),SEEK_CUR);        // sounds
		fseek(file,n * sizeof (UINT8),SEEK_CUR);        // alt_sounds
		n = read_UINTW(file);                          // n_vclips
		fseek(file,n * sizeof (VCLIP),SEEK_CUR);        // video clips
		n = read_UINTW(file);                          // n_eclips
		fseek(file,n * sizeof (ECLIP),SEEK_CUR);        // effect clips
		n = read_UINTW(file);                          // n_wclips
		fseek(file,n * sizeof (WCLIP),SEEK_CUR);        // weapon clips
		n = read_UINTW(file);                          // n_robots
		if (objP->type == OBJ_ROBOT) {
			for (i=0;i<n;i++) {
				if (i == (UINT32) objP->id) {
					fread(&robot_info,sizeof (ROBOT_INFO),1,file);// read robot info
					model_num = robot_info.model_num;
				} else {
					fseek(file,sizeof (ROBOT_INFO),SEEK_CUR);   // skip robot info
				}
			}
		} else {
			fseek(file,n * sizeof (ROBOT_INFO),SEEK_CUR);
		}
		n = read_UINTW(file);                          // n_robot_joints
		fseek(file,n * sizeof (JOINTPOS),SEEK_CUR);     // robot joints
		n = read_UINTW(file);                          // n_weapon
		fseek(file,n * sizeof (WEAPON_INFO),SEEK_CUR);  // weapon info
		n = read_UINTW(file);                          // n_powerups
		fseek(file,n * sizeof (POWERUP_TYPE_INFO),SEEK_CUR); // powerup info
		n = read_UINTW(file);                          // n_polyModels
		assert(n<=MAX_POLYGON_MODELS);
		for (i=0;i<n;i++) {
			ReadPolyModel (polyModel, file);
			model_data_size[i] = (UINT16)polyModel.model_data_size;
			if (i==(UINT32) model_num) {
				memcpy(&save_model,&polyModel,sizeof (POLYMODEL));
			}
		}
		for (i=0;i<n;i++) {
			if (i==(UINT32) model_num) {
				fread(gModelData, model_data_size[i],1,file);
				break; // were done!
			} else {
				fseek(file , model_data_size[i],SEEK_CUR);
			}
		}
	}
	return 0;
}
