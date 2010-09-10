// Copyright (c) 1997 Bryan Aamot
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <string.h>

#include "mine.h"
#include "dle-xp.h"

CRenderModel renderModel;

//------------------------------------------------------------------------------
// CONSTANTS
//------------------------------------------------------------------------------

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

//------------------------------------------------------------------------------
// MACROS
//------------------------------------------------------------------------------

#define W(p)   (*((short *) (p)))
#define WP(p)  ((short *) (p))
#define VP(p)  ((CFixVector*) (p))

#define calcNormal(a, b)
#define glNormal3fv(a)
#define glColor3ub(a, b, c)
#define glBegin(a)
#define glVertex3i(x, y, z)
#define glEnd()
#define glTexCoord2fv(a)

#define UINTW int

//------------------------------------------------------------------------------

CModelRenderer modelRenderer;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CPolyModel* CModelRenderer::Model (void)
{
if (theMine == null)
	return null;

	uint nModel;

switch (m_object->m_info.type) {
	case OBJ_PLAYER:
	case OBJ_COOP:
		nModel = D2_PLAYER_CLIP_NUMBER;
		break;
	case OBJ_WEAPON:
		nModel = MINE_CLIP_NUMBER;
		break;
	case OBJ_CNTRLCEN:
		switch (m_object->m_info.id) {
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
		nModel = robotManager.RobotInfo ((m_object->m_info.id >= N_ROBOT_TYPES_D2) ? m_object->m_info.id /*- N_ROBOT_TYPES_D2*/ : m_object->m_info.id)->Info ().nModel;
		break;
	default:
		return null;
	}
return m_polyModels + nModel;
}

//------------------------------------------------------------------------------

int CModelRenderer::Setup (CGameObject *objP, CViewMatrix* view, CDC* pDC) 
{
m_offset.Clear ();
m_data.nPoints = 0;
m_nGlow = -1;
modelRenderer.m_object = objP;
m_model = Model ();

if (m_model == null)
	return 1;
if (m_model->m_info.renderData)
	return 0;

m_view = view;
m_pDC = pDC;

char filename[256];

strcpy_s (filename, sizeof (filename), descentPath [1]);
char *slash = strrchr (filename, '\\');
if (slash)
	*(slash+1) = '\0';
else
	filename[0] = '\0';

bool bCustom = (objP->Type () == OBJ_ROBOT) && (objP->Id () >= N_ROBOT_TYPES_D2);

if (bCustom) {
	char *psz = strstr (filename, "data");
	if (psz)
		*psz = '\0';
	}
strcat_s (filename, sizeof (filename), bCustom ? "missions\\d2x.hog" : "descent2.ham");
if (ReadModelData (filename, bCustom ? "d2x.ham" : "", bCustom))
	m_model = null;
return m_model == null;
}

//------------------------------------------------------------------------------

int CModelRenderer::ReadModelData (char* filename, char *szSubFile, bool bCustom) 
{
	CFileManager fp;

if (fp.Open (filename, "rb"))
	return 1;

	uint	id;
	uint	i, n;

if (bCustom) {
	struct level_header level;
	char data[3];
	long position;

	fp.Read (data, 3, 1); // verify signature "DHF"
	if (data[0] != 'D' || data[1] != 'H' || data[2] != 'F') {
		fp.Close ();
		return 1;
		}
	position = 3;
	while (!fp.EoF ()) {
		fp.Seek (position, SEEK_SET);
		if ((fp.Read (&level, sizeof (struct level_header), 1) != 1) ||
			 (level.size > 10000000L || level.size < 0)) {
				fp.Close ();
				return 1;
				}
		if (!strcmp (level.name, szSubFile)) {
			id = fp.ReadInt32 ();	  					   // read id
			if (id != 0x5848414DL) {
				fp.Close ();
				return 1;
				}
			fp.ReadUInt16 ();                              // read version
			n = fp.ReadInt16 ();                         // n_weapon_types
			fp.Seek (n * sizeof (WEAPON_INFO), SEEK_CUR);  // weapon_info
			n = fp.ReadInt16 ();                         // n_robot_types
			for (i = 0; i < n; i++)
				robotManager.RobotInfo (N_ROBOT_TYPES_D2 + i)->Read (fp);
			n  = fp.ReadInt16 ();                         // n_robot_joints
			fp.Seek (n * sizeof (JOINTPOS), SEEK_CUR);     // robot_joints
			break;
			}
		position += sizeof (struct level_header) + level.size;
		}
	n = fp.ReadInt16 ();                          // n_curModels
	assert (n <= MAX_POLYGON_MODELS);
	for (i = 0; i < n; i++) 
		m_polyModels [N_POLYGON_MODELS_D2 + i].Read (fp);
	for (i = 0; i < n; i++) 
		m_polyModels [N_POLYGON_MODELS_D2 + i].Read (fp, true);
	}
else {
	id = fp.ReadInt32 ();	  					   // read id
	if (id != 0x214d4148L) {
		fp.Close ();
		return 1;
		}
	fp.ReadInt16 ();                              // read version
	n  = fp.ReadInt16 ();                         // n_tmap_info
	fp.Seek (n * sizeof (ushort), SEEK_CUR);	 // bitmap_indicies
	fp.Seek (n * sizeof (TMAP_INFO), SEEK_CUR); // tmap_info
	n = fp.ReadInt16 ();                          // n_sounds
	fp.Seek (n * sizeof (byte), SEEK_CUR);     // sounds
	fp.Seek (n * sizeof (byte), SEEK_CUR);     // alt_sounds
	n = fp.ReadInt16 ();                          // n_vclips
	fp.Seek (n * sizeof (VCLIP), SEEK_CUR);     // video clips
	n = fp.ReadInt16 ();                          // n_eclips
	fp.Seek (n * sizeof (ECLIP), SEEK_CUR);     // effect clips
	n = fp.ReadInt16 ();                          // n_wclips
	fp.Seek (n * sizeof (WCLIP), SEEK_CUR);     // weapon clips
	n = fp.Tell ();
	n = fp.ReadInt16 ();                          // n_robots
	for (i = 0; i < n; i++) 
		robotManager.RobotInfo (i)->Read (fp);
	n = fp.ReadInt16 ();                          // n_robot_joints
	fp.Seek (n * sizeof (JOINTPOS), SEEK_CUR);     // robot joints
	n = fp.ReadInt16 ();                          // n_weapon
	fp.Seek (n * sizeof (WEAPON_INFO), SEEK_CUR);  // weapon info
	n = fp.ReadInt16 ();                          // n_powerups
	fp.Seek (n * sizeof (POWERUP_TYPE_INFO), SEEK_CUR); // powerup info
	n = fp.ReadInt16 ();                          // n_curModels
	assert (n <= MAX_POLYGON_MODELS);
	for (i = 0; i < n; i++) 
		m_polyModels [i].Read (fp);
	for (i = 0; i < n; i++) 
		m_polyModels [i].Read (fp, true);
	}
fp.Close ();
return 0;
}

//------------------------------------------------------------------------------

//static tModelRenderData modelRenderer.m_data;
//static CVertex modelRenderer.m_offset;
//static short nGlow = -1;
//static CDoubleVector normal;	// Storage for calculated surface normal
//static CModelRenderPoly* modelRenderer.m_face;
//static int m_pt, m_pt0, modelRenderer.m_nPolyPoints;
//static int modelRenderer.m_lastObjType = -1;
//static int modelRenderer.m_lastObjId = -1;
//static APOINT modelRenderer.m_screenPoly [MAX_POLYMODEL_POINTS];

//------------------------------------------------------------------------------
// draw_poly
//
// Action - Draws a polygon if it is facing the outward.
//          Used global device context handle set by SetupModel ()
//------------------------------------------------------------------------------

void CModelRenderPoly::Draw (void) 
{
if (modelRenderer.CheckNormal (offset, normal)) {
	POINT aPoints [MAX_POLYMODEL_POLY_POINTS];
	for (int i = 0; i < nVerts; i++) {
		int j = verts [i];
		aPoints [i].x = modelRenderer.m_screenPoly [j].x;
		aPoints [i].y = modelRenderer.m_screenPoly [j].y;
		}
	modelRenderer.m_pDC->Polygon (aPoints, nVerts);
	}
}

//------------------------------------------------------------------------------
// set_model_points ()
//
// Rotates, translates, then sets screen points (xy) for 3d model points
//------------------------------------------------------------------------------

void CPolyModel::SetPoints (int start, int end) 
{
	CVertex	m_pt;

for (int i = start; i < end; i++) {
	// rotate point using Objects () rotation matrix
	m_pt = modelRenderer.m_object->m_location.orient * modelRenderer.m_data.points [i];
	// set point to be in world coordinates
	m_pt += modelRenderer.m_object->m_location.pos;
	// now that points are relative to set screen xy points (modelRenderer.m_screenPoly)
	modelRenderer.m_view->Project (m_pt, modelRenderer.m_screenPoly [i]);
	}
}

//------------------------------------------------------------------------------
//
// Calls the CGameObject interpreter to render an CGameObject.
// The CGameObject renderer is really a seperate pipeline. returns true if drew
//
//------------------------------------------------------------------------------

void CPolyModel::Render (byte *p) 
{
while (W (p) != OP_EOF) {
	switch (W (p)) {
		// Point Definitions with Start Offset:
		// 2  ushort      modelRenderer.m_nPolyPoints       number of points
		// 4  ushort      start_point    starting point
		// 6  ushort      unknown
		// 8  CFixVector  pts[modelRenderer.m_nPolyPoints]  x, y, z data
		case OP_DEFP_START: {
			m_pt0 = W (p+4);
			modelRenderer.m_nPolyPoints = W (p+2);
			modelRenderer.m_data.nPoints += modelRenderer.m_nPolyPoints;
			assert (W (p+6)==0);
			assert (modelRenderer.m_data.nPoints < MAX_POLYMODEL_POINTS);
			assert (m_pt0 + modelRenderer.m_nPolyPoints < MAX_POLYMODEL_POINTS);
			for (m_pt = 0; m_pt < modelRenderer.m_nPolyPoints; m_pt++) 
				modelRenderer.m_data.points [m_pt+m_pt0] = VP (p+8)[m_pt] + modelRenderer.m_offset;
			SetPoints (m_pt0, m_pt0 + modelRenderer.m_nPolyPoints);
			p += W (p+2)*sizeof (CFixVector) + 8;
			break;
			}
		// Flat Shaded Polygon:
		// 2  ushort     nVerts
		// 4  CFixVector vector1
		// 16 CFixVector vector2
		// 28 ushort     color
		// 30 ushort     verts[nVerts]
		case OP_FLATPOLY: {
			modelRenderer.m_face = &modelRenderer.m_data.polys [modelRenderer.m_data.nPolys];
			modelRenderer.m_face->nVerts = W (p+2);
			modelRenderer.m_face->offset = *VP (p+4);
			modelRenderer.m_face->normal = *VP (p+16);
			for (m_pt = 0; m_pt < modelRenderer.m_face->nVerts; m_pt++) 
				modelRenderer.m_face->verts[m_pt] = WP (p+30)[m_pt];
			modelRenderer.m_face->Draw ();
			p += 30 + ((modelRenderer.m_face->nVerts&~1)+1)*2;
			break;
			}
		// Texture Mapped Polygon:
		// 2  ushort     nVerts
		// 4  CFixVector vector1
		// 16 CFixVector vector2
		// 28 ushort     nBaseTex
		// 30 ushort     verts[nVerts]
		// -- UVL        uvls[nVerts]
		case OP_TMAPPOLY: {
			modelRenderer.m_face = &modelRenderer.m_data.polys [modelRenderer.m_data.nPolys];
			modelRenderer.m_face->nVerts  = W (p+2);
			assert (modelRenderer.m_face->nVerts >= MIN_POLYMODEL_POLY_POINTS);
			assert (modelRenderer.m_face->nVerts <= MAX_POLYMODEL_POLY_POINTS);
			assert (modelRenderer.m_data.nPolys < MAX_POLYMODEL_POLYS);
			modelRenderer.m_face->offset   = *VP (p+4);
			modelRenderer.m_face->normal   = *VP (p+16);
			modelRenderer.m_face->color    = -1;
			modelRenderer.m_face->nBaseTex = W (p+28);
			modelRenderer.m_face->nGlow = modelRenderer.m_nGlow;
			for (m_pt = 0; m_pt < modelRenderer.m_face->nVerts; m_pt++) 
				modelRenderer.m_face->verts[m_pt] = WP (p+30)[m_pt];
			p += 30 + ((modelRenderer.m_face->nVerts&~1)+1)*2;
			modelRenderer.m_face->Draw ();
			p += modelRenderer.m_face->nVerts * 12;
			break;
			}
		// Sort by Normal
		// 2  ushort      unknown
		// 4  CFixVector  Front Model normal
		// 16 CFixVector  Back Model normal
		// 28 ushort      Front Model Offset
		// 30 ushort      Back Model Offset
		case OP_SORTNORM: {
			/* = W (p+2); */
			/* = W (p+4); */
			/* = W (p+16); */
			if (modelRenderer.CheckNormal (*VP (p+4), *VP (p+16))) {
			  Render (p + W (p+28));
			  Render (p + W (p+30));
				}
			else {
			  Render (p + W (p+30));
			  Render (p + W (p+28));
				}
			p += 32;
			break;
			}
		// Call a Sub Object
		// 2  ushort     n_anims
		// 4  CFixVector offset
		// 16 ushort     model offset
		case OP_SUBCALL: {
			modelRenderer.m_offset += *VP (p+4);
			Render (p + W (p+16));
			modelRenderer.m_offset -= *VP (p+4);
			p += 20;
			break;
			}
		// Glow Number for Next Poly
		// 2 UINTW  Glow_Value
		case OP_GLOW: {
			modelRenderer.m_nGlow = W (p+2);
			p += 4;
			break;
			}
		default: 
			assert (0);
		}
	}
return;
}

//------------------------------------------------------------------------------
// ReadModelData ();
//------------------------------------------------------------------------------

void CPolyModel::Read (CFileManager& fp, bool bRenderData) 
{
if (bRenderData) {
	Release ();
	if ((m_info.renderData = new byte [m_info.dataSize]))
		fp.Read (m_info.renderData, m_info.dataSize, 1);
	}
else {
	m_info.nModels = fp.ReadInt32 ();
	m_info.dataSize = fp.ReadInt32 ();
	fp.ReadInt32 ();
	m_info.renderData = null;
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].ptr = fp.ReadInt32 ();
	for (int i = 0; i < MAX_SUBMODELS; i++)
		fp.Read (m_info.subModels [i].offset);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		fp.Read (m_info.subModels [i].norm);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		fp.Read (m_info.subModels [i].pnt);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].rad = fp.ReadInt32 ();
	for (int i = 0; i < MAX_SUBMODELS; i++)
		m_info.subModels [i].parent = (byte) fp.ReadSByte ();
	for (int i = 0; i < MAX_SUBMODELS; i++)
		fp.Read (m_info.subModels [i].vMin);
	for (int i = 0; i < MAX_SUBMODELS; i++)
		fp.Read (m_info.subModels [i].vMax);
	fp.Read (m_info.vMin);
	fp.Read (m_info.vMax);
	m_info.rad = fp.ReadInt32 ();
	m_info.textureCount = fp.ReadByte ();
	m_info.firstTexture = fp.ReadUInt16 ();
	m_info.simplerModel = fp.ReadByte ();
	}
}

//------------------------------------------------------------------------------
//eof polymodel.cpp

