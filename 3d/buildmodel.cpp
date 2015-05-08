#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <math.h>
#include <stdlib.h>
#include "error.h"

#include "descent.h"
#include "interp.h"
#include "shadows.h"
#include "hitbox.h"
#include "globvars.h"
#include "gr.h"
#include "byteswap.h"
#include "u_mem.h"
#include "console.h"
#include "ogl_defs.h"
#include "ogl_lib.h"
#include "ogl_color.h"
#include "network.h"
#include "rendermine.h"
#include "segmath.h"
#include "light.h"
#include "dynlight.h"
#include "lightning.h"
#include "renderthreads.h"
#include "hiresmodels.h"
#include "buildmodel.h"
#include "cquicksort.h"

using namespace RenderModel;

#define DIRECT_VBO			1

#if POLYGONAL_OUTLINE
extern bool bPolygonalOutline;
#endif

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int32_t CContourInfo::IsFacingPoint (CFloatVector3& v)
{
	CFloatVector3	h = v - m_vCenterf [1];

return (h * m_vNormalf [1]) > 0;
}

//------------------------------------------------------------------------------

int32_t CContourInfo::IsFacingViewer (void)
{
return CFloatVector3::Dot (m_vCenterf [1], m_vNormalf [1]) >= 0;
}

//------------------------------------------------------------------------------

void CContourInfo::RotateNormal (void)
{
transformation.Rotate (m_vNormalf [1], m_vNormalf [0]);
}

//------------------------------------------------------------------------------

void CContourInfo::TransformCenter (void)
{
transformation.Transform (m_vCenterf [1], m_vCenterf [0]);
}

//------------------------------------------------------------------------------

int32_t CContourInfo::IsLit (CFloatVector3& vLight)
{
return m_bFacingLight = IsFacingPoint (vLight);
}

//------------------------------------------------------------------------------

int32_t CContourInfo::IsFront (void)
{
return m_bFrontFace = IsFacingViewer ();
}

//------------------------------------------------------------------------------

void CContourInfo::Transform (void)
{
if (!m_bTransformed) {
	TransformCenter ();
	RotateNormal ();
	m_bFacingViewer = IsFacingViewer ();
	m_bTransformed = 1;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CModelEdge::Transform (void)
{
for (int32_t i = 0; i < 2; i++) {
	transformation.Rotate (m_faces [i].m_vNormal [1], m_faces [i].m_vNormal [0]);
	transformation.Transform (m_faces [i].m_vCenter [1], m_faces [i].m_vCenter [0]);
	}
CMeshEdge::Transform ();
}

//------------------------------------------------------------------------------

int32_t CModelEdge::IsFacingViewer (int16_t nFace)
{
#if 1
//return CFloatVector::Dot (m_vertices [1][nFace]/*m_faces [nFace].m_vCenter [1]*/, m_faces [nFace].m_vNormal [1]) < 0.0f;
float dot = CFloatVector::Dot (m_faces [nFace].m_vCenter [1], m_faces [nFace].m_vNormal [1]);
return dot < 0.0f;
#else
CFloatVector v = m_faces [nFace].m_vCenter [1];
CFloatVector::Normalize (v);
return CFloatVector::Dot (v, m_faces [nFace].m_vNormal [1]) < 0.0f;
#endif
}

//------------------------------------------------------------------------------

int32_t CModelEdge::Visibility (void)
{
return IsFacingViewer (0) | ((m_nFaces == 1) ? 0 : (IsFacingViewer (1) << 1));
}

//------------------------------------------------------------------------------

int32_t CModelEdge::IsContour (void)
{
return IsFacingViewer (0) != IsFacingViewer (1);
}

//------------------------------------------------------------------------------
// -1: edge invisible
// 0: contour
// 1: both faces visible

int32_t CModelEdge::Type (void)
{
#if 0
if (m_nFaces < 2)
	return -1;
#endif
int32_t h = Visibility ();
return (h == 0) ? -1 : (h != 3) ? 0 : (m_fDot < 0.925f) ? (m_fScale < 1.0f) ? 2 : 1 : -1;
}

//------------------------------------------------------------------------------

int32_t CModelEdge::Partial (void)
{
return m_fDot > 0.75f; // ~ 41.5�
}

//------------------------------------------------------------------------------

CFloatVector& CModelEdge::Normal (int32_t nFace)
{
return m_faces [nFace].m_vNormal [0];
}

//------------------------------------------------------------------------------

CFloatVector& CModelEdge::Vertex (int32_t i)
{
#if POLYGONAL_OUTLINE
if (bPolygonalOutline)
	return m_vertices [1][i];
else	// only required if not transforming model outlines via OpenGL when rendering
#endif
	return m_vertices [0][i];
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

GLenum FaceWinding (CFloatVector3 *v0, CFloatVector3 *v1, CFloatVector3 *v2)
{
return (((*v1).v.coord.x - (*v0).v.coord.x) * ((*v2).v.coord.y - (*v1).v.coord.y) < 0) ? GL_CW : GL_CCW;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CModel::Init (void)
{
memset (m_teamTextures, 0, sizeof (m_teamTextures));
memset (m_nGunSubModels, 0xFF, sizeof (m_nGunSubModels));
m_nModel = -1;
m_fScale = 0;
m_nType = 0; //-1: custom mode, 0: default model, 1: alternative model, 2: hires model
m_nFaces = 0;
m_iFace = 0;
m_nVerts = 0;
m_nFaceVerts = 0;
m_iFaceVert = 0;
m_nSubModels = 0;
m_nTextures = 0;
m_iSubModel = 0;
m_nEdges = 0;
m_bHasTransparency = 0;
m_bValid = 0;
m_bRendered = 0;
m_bBullets = 0;
m_vBullets.SetZero ();
m_vboDataHandle = 0;
m_vboIndexHandle = 0;
}

//------------------------------------------------------------------------------

bool CModel::Create (void)
{
m_vertices.Create (m_nVerts);
m_vertexOwner.Create (m_nVerts);
m_color.Create (m_nVerts);
m_vertBuf [0].Create (m_nFaceVerts);
m_faceVerts.Create (m_nFaceVerts);
m_vertNorms.Create (m_nFaceVerts);
m_subModels.Create (m_nSubModels);
m_faces.Create (m_nFaces);
m_index [0].Create (m_nFaceVerts);
m_sortedVerts.Create (m_nFaceVerts);
m_edges.Create (m_nFaceVerts);

m_vertices.Clear (0);
m_color.Clear (0xff);
m_vertBuf [0].Clear (0);
m_faceVerts.Clear (0);
m_vertNorms.Clear (0);
m_subModels.Clear (0);
m_faces.Clear (0);
m_index [0].Clear (0);
m_sortedVerts.Clear (0);

for (uint16_t i = 0; i < m_nSubModels; i++)
	m_subModels [i].m_nSubModel = i;

if (ogl.m_features.bVertexBufferObjects) {
	int32_t i;
	ogl.ClearError (0);
	glGenBuffersARB (1, &m_vboDataHandle);
	if ((i = glGetError ())) {
#	if DBG
		HUDMessage (0, "glGenBuffersARB failed (%d)", i);
#	endif
		ogl.m_features.bVertexBufferObjects = 0;
		Destroy ();
		return false;
		}
#if !DIRECT_VBO
	glBindBufferARB (GL_ARRAY_BUFFER_ARB, m_vboDataHandle);
	if ((i = glGetError ())) {
#	if DBG
		HUDMessage (0, "glBindBufferARB failed (%d)", i);
#	endif
		ogl.m_features.bVertexBufferObjects = 0;
		Destroy ();
		return false;
		}
	glBufferDataARB (GL_ARRAY_BUFFER, m_nFaceVerts * sizeof (CRenderVertex), NULL, GL_STATIC_DRAW_ARB);
	m_vertBuf [1].SetBuffer (reinterpret_cast<CRenderVertex*> (glMapBufferARB (GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB)), 1, m_nFaceVerts);
#endif
	m_vboIndexHandle = 0;
	glGenBuffersARB (1, &m_vboIndexHandle);
	if (m_vboIndexHandle) {
#if !DIRECT_VBO
		glBindBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, m_vboIndexHandle);
		glBufferDataARB (GL_ELEMENT_ARRAY_BUFFER_ARB, m_nFaceVerts * sizeof (int16_t), NULL, GL_STATIC_DRAW_ARB);
		m_index [1].SetBuffer (reinterpret_cast<int16_t*> (glMapBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB)), 1, m_nFaceVerts);
#endif
		}
	}
return true;
}

//------------------------------------------------------------------------------

void CModel::Destroy (void)
{
#if DBG
if (m_nModel > -1)
	BRP;
if ((nDbgModel >= 0) && (m_nModel == nDbgModel))
	BRP;
#endif
if (ogl.m_features.bVertexBufferObjects && m_vboDataHandle)
	glDeleteBuffersARB (1, &m_vboDataHandle);
if (ogl.m_features.bVertexBufferObjects && m_vboIndexHandle)
	glDeleteBuffersARB (1, &m_vboIndexHandle);
#if DBG
if (m_textures.Buffer ())
#endif
	m_textures.Clear (); // careful - m_textures just contains a copy of textures allocated elsewhere
m_textures.Destroy ();
m_sortedVerts.Destroy ();
m_index [0].Destroy ();
m_faces.Destroy ();
m_subModels.Destroy ();
m_vertNorms.Destroy ();
m_faceVerts.Destroy ();
m_vertBuf [0].Destroy ();
m_vertBuf [1].SetBuffer (0);	//avoid trying to delete memory allocated by the graphics driver
m_color.Destroy ();
m_vertices.Destroy ();
m_vertexOwner.Destroy ();

Init ();
}

//------------------------------------------------------------------------------

int32_t CFace::Compare (const CFace* pf, const CFace* pm)
{
if (pf == pm)
	return 0;
if (pf->m_bBillboard < pm->m_bBillboard)
	return -1;
if (pf->m_bBillboard > pm->m_bBillboard)
	return 1;
if (pf->m_textureP && (pf->m_nBitmap >= 0) && (pm->m_nBitmap >= 0)) {
	if (pf->m_textureP->BPP () < pm->m_textureP->BPP ())
		return -1;
	if (pf->m_textureP->BPP () > pm->m_textureP->BPP ())
		return -1;
	}
if (pf->m_nBitmap < pm->m_nBitmap)
	return -1;
if (pf->m_nBitmap > pm->m_nBitmap)
	return 1;
if (pf->m_nVerts < pm->m_nVerts)
	return -1;
if (pf->m_nVerts > pm->m_nVerts)
	return 1;
return 0;
}

//------------------------------------------------------------------------------

const bool CFace::operator!= (CFace& other)
{
if (m_textureP && (m_nBitmap >= 0) && (other.m_nBitmap >= 0)) {
	if (m_textureP->BPP () < other.m_textureP->BPP ())
		return true;
	if (m_textureP->BPP () > other.m_textureP->BPP ())
		return true;
	}
if (m_nBitmap < other.m_nBitmap)
	return true;
if (m_nBitmap > other.m_nBitmap)
	return true;
if (m_nVerts < other.m_nVerts)
	return true;
if (m_nVerts > other.m_nVerts)
	return true;
return false;
}

//------------------------------------------------------------------------------

void CFace::SetTexture (CBitmap* textureP)
{
m_textureP = (textureP && (m_nBitmap >= 0)) ? textureP + m_nBitmap : NULL;
}

//------------------------------------------------------------------------------
// Before this function is executed, m_nIndex points into the CRenderModel::m_faceVerts
// After it is executed, it points into CRenderModel::m_sortedVerts
int32_t CFace::GatherVertices (CArray<CVertex>& source, CArray<CVertex>& dest, int32_t nIndex)
{
#if DBG
if (uint32_t (m_nIndex + m_nVerts) > source.Length ())
	return 0;
if (uint32_t (nIndex + m_nVerts) > dest.Length ())
	return 0;
#endif
memcpy (dest + nIndex, source + m_nIndex, m_nVerts * sizeof (CVertex));
m_nIndex = nIndex;	//set this face's index of its first vertex in the model's vertex buffer
return m_nVerts;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CSubModel::InitMinMax (void)
{
m_vMin = CFloatVector3::Create ((float) 1e30, (float) 1e30, (float) 1e30);
m_vMax = CFloatVector3::Create ((float) -1e30, (float) -1e30, (float) -1e30);
}

//------------------------------------------------------------------------------

void CSubModel::SetMinMax (CFloatVector3 *vertexP)
{
	CFloatVector3	v = *vertexP;

#if 0
dir.dir.coord.x += X2F (psm->m_vOffset.dir.coord.x);
dir.dir.coord.y += X2F (psm->m_vOffset.dir.coord.y);
dir.dir.coord.z += X2F (psm->m_vOffset.dir.coord.z);
#endif
if (m_vMin.v.coord.x > v.v.coord.x)
	m_vMin.v.coord.x = v.v.coord.x;
if (m_vMin.v.coord.y > v.v.coord.y)
	m_vMin.v.coord.y = v.v.coord.y;
if (m_vMin.v.coord.z > v.v.coord.z)
	m_vMin.v.coord.z = v.v.coord.z;
if (m_vMax.v.coord.x < v.v.coord.x)
	m_vMax.v.coord.x = v.v.coord.x;
if (m_vMax.v.coord.y < v.v.coord.y)
	m_vMax.v.coord.y = v.v.coord.y;
if (m_vMax.v.coord.z < v.v.coord.z)
	m_vMax.v.coord.z = v.v.coord.z;
}

//------------------------------------------------------------------------------

void CSubModel::SortFaces (CBitmap* textureP)
{
	CQuickSort<CFace>	qs;

for (int32_t i = 0; i < m_nFaces; i++)
	m_faces [i].SetTexture (textureP);
if (m_nFaces > 1)
	qs.SortAscending (m_faces, 0, static_cast<uint32_t> (m_nFaces - 1), &RenderModel::CFace::Compare);
}

//------------------------------------------------------------------------------

void CSubModel::GatherVertices (CArray<CVertex>& source, CArray<CVertex>& dest)
{
	int32_t nIndex = m_nVertexIndex [0];	//this submodel's vertices start at m_nIndex in the model's vertex buffer

// copy vertices face by face
for (int32_t i = 0; i < m_nFaces; i++)
	nIndex += m_faces [i].GatherVertices (source, dest, nIndex);
}

//------------------------------------------------------------------------------

void CSubModel::GatherContourEdges (CModel* modelP)
{
	CModelEdge* edgeP = m_edges.Buffer ();

modelP->m_contourEdges.Reset ();
for (uint16_t i = 0; i < m_nEdges; i++, edgeP++)
	if (edgeP->IsContour ())
		modelP->m_contourEdges.Push (i);
}

//------------------------------------------------------------------------------

void CSubModel::GatherLitFaces (CModel* modelP, CFloatVector3& vLight)
{
modelP->m_litFaces.Reset ();
for (uint16_t i = 0; i < m_nFaces; i++)
	if (m_faces [i].IsLit (vLight))
		modelP->m_litFaces.Push (m_faces + i);
}

//------------------------------------------------------------------------------

#define DIST_TOLERANCE	0.001f
#define MATCH_METHOD		0

#define EDGE_LIST			m_edges	// modelP->m_edges
#define EDGE_COUNT		m_nEdges	// modelP->m_nEdges

int32_t CSubModel::FindEdge (CModel* modelP, uint16_t v1, uint16_t v2)
{
	CModelEdge		*edgeP = EDGE_LIST.Buffer ();
#if MATCH_METHOD
	CFloatVector	v [2], d;
	float				l;
	
v [0].Assign (modelP->m_vertices [v1]);
v [1].Assign (modelP->m_vertices [v2]);
#endif

for (int32_t i = 0; i < EDGE_COUNT; i++, edgeP++) {
#if !MATCH_METHOD
	if ((edgeP->m_nFaces < 2) && (edgeP->m_nVertices [0] == v1) && (edgeP->m_nVertices [1] == v2))
		return i;
#elif MATCH_METHOD == 1
	if (((edgeP->m_vertices [0][0] == v [0]) && (edgeP->m_vertices [0][1] == v [1])) ||
		 ((edgeP->m_vertices [0][0] == v [1]) && (edgeP->m_vertices [0][1] == v [0])))
		return i;
#else
	d = edgeP->m_vertices [0][0] - v [0];
	if ((l = d.Mag ()) < DIST_TOLERANCE) {
		d = edgeP->m_vertices [0][1] - v [1];
		if ((l = d.Mag ()) < DIST_TOLERANCE)
			return i;
		}
	d = edgeP->m_vertices [0][0] - v [1];
	if ((l = d.Mag ()) < DIST_TOLERANCE) {
		d = edgeP->m_vertices [0][1] - v [0];
		if ((l = d.Mag ()) < DIST_TOLERANCE)
			return i;
		}
#endif
	}
return -1;
}

//------------------------------------------------------------------------------

int32_t CSubModel::AddEdge (CModel *modelP, CFace* faceP, uint16_t v1, uint16_t v2)
{
if (v1 > v2)
	Swap (v1, v2);
int32_t i = FindEdge (modelP, v1, v2);

CModelEdge *edgeP;

if (i >= 0) {
	edgeP = EDGE_LIST + i;
	if (edgeP->m_faces [0].m_nItem != faceP->m_nSubModel)
		BRP;
	if (edgeP->m_nFaces < 1)
		i = FindEdge (modelP, v1, v2);
	edgeP->m_nFaces = 2;
	edgeP->m_faces [1].m_nItem = faceP->m_nSubModel;
	edgeP->m_faces [1].m_vNormal [0].Assign (faceP->m_vNormalf [0]);
	edgeP->m_faces [1].m_vCenter [0].Assign (faceP->m_vCenterf [0]);
	edgeP->Setup ();
	}
else {
	edgeP = EDGE_LIST + EDGE_COUNT++;
	edgeP->m_nFaces = 1;
	edgeP->m_nVertices [0] = v1;
	edgeP->m_nVertices [1] = v2;
	edgeP->m_vertices [0][0].Assign (modelP->m_vertices [v1]);
	edgeP->m_vertices [0][1].Assign (modelP->m_vertices [v2]);
	if (faceP->m_nSubModel < 0)
		BRP;
	edgeP->m_faces [0].m_nItem = faceP->m_nSubModel;
	edgeP->m_faces [0].m_vNormal [0].Assign (faceP->m_vNormalf [0]);
	edgeP->m_faces [0].m_vCenter [0].Assign (faceP->m_vCenterf [0]);
	}
return edgeP->m_nFaces == 2;
}

//------------------------------------------------------------------------------

bool CSubModel::BuildEdgeList (CModel* modelP)
{
	CFace*			faceP = m_faces;
	CFloatVector3	vCenter;
	int32_t			nComplete = 0;

vCenter.Assign (m_vCenter);
m_nEdges = 0;

#if 1
for (uint16_t i = 0; i < m_nFaces; i++, faceP++)
	m_nEdges += faceP->m_nVerts;
if (!(m_edges.Create (m_nEdges)))
	return false;
m_nEdges = 0;
#endif

faceP = m_faces;
for (uint16_t i = 0; i < m_nFaces; i++, faceP++) {
	faceP->m_vCenterf [0].SetZero ();
	for (uint16_t j = 0; j < faceP->m_nVerts; j++)
		faceP->m_vCenterf [0] += modelP->m_faceVerts [faceP->m_nIndex + j].m_vertex;
	faceP->m_vCenterf [0] /= float (faceP->m_nVerts);
	faceP->m_faceWinding = FaceWinding (&modelP->m_faceVerts [faceP->m_nIndex].m_vertex, 
													&modelP->m_faceVerts [faceP->m_nIndex + 1].m_vertex, 
													&modelP->m_faceVerts [faceP->m_nIndex + 2].m_vertex);
#if 0
	faceP->m_vNormalf [0].Assign (faceP->m_vNormal);
#else
	faceP->m_vNormalf [0] = CFloatVector3::Normal (modelP->m_faceVerts [faceP->m_nIndex].m_vertex, 
																  modelP->m_faceVerts [faceP->m_nIndex + 1].m_vertex, 
																  modelP->m_faceVerts [faceP->m_nIndex + 2].m_vertex);
#	if 0
#		if 0
	CFloatVector3 v = faceP->m_vCenterf [0];
	v -= vCenter;
	CFloatVector3::Normalize (v);
	if (CFloatVector3::Dot (v, faceP->m_vNormalf [0]) < 0.0f)
		faceP->m_vNormalf [0].Neg ();
#	else
	if (faceP->m_faceWinding == GL_CW)
		faceP->m_vNormalf [0].Neg ();
#		endif
#	endif
	faceP->m_vNormal.Assign (faceP->m_vNormalf [0]);
#endif

	for (uint16_t j = 0; j < faceP->m_nVerts; j++)
		if (AddEdge (modelP, faceP, modelP->m_faceVerts [faceP->m_nIndex + j].m_nIndex, modelP->m_faceVerts [faceP->m_nIndex + (j + 1) % faceP->m_nVerts].m_nIndex))
			++nComplete;
	}

#if 0 //DBG
CModelEdge *edgeP = m_edges.Buffer ();
int32_t nIncomplete = 0;
for (int32_t i = m_nEdges; i; i--, edgeP++) 
	if (edgeP->m_nFaces < 2) 
		nIncomplete++;
#endif

return 1; //m_edges.Resize (m_nEdges) != NULL;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CModel::Setup (int32_t bHires, int32_t bSort)
{
	CSubModel*		psm;
	CFace*			pfi, * pfj;
	CVertex*			pmv;
	CFloatVector3*	pv, * pn;
	tTexCoord2f*	pt;
	CFloatVector*	pc;
	CBitmap*			textureP = bHires ? m_textures.Buffer () : NULL;
	int32_t			i, j;
	uint16_t			nId;

m_vertexOwner.SortAscending ();
m_fScale = 1;
for (i = 0, j = m_nFaceVerts; i < j; i++)
	m_index [0][i] = i;
//sort each submodel's faces
for (i = 0; i < m_nSubModels; i++) {
	psm = &m_subModels [i];
	if (psm->m_nFaces) {
		psm->BuildEdgeList (this);
		if (bSort) {
			psm->SortFaces (textureP);
			psm->GatherVertices (m_faceVerts, m_sortedVerts);
			}
		pfi = psm->m_faces;
		pfi->SetTexture (textureP);
		for (nId = 0, j = psm->m_nFaces - 1; j; j--) {
			pfi->m_nId = nId;
			pfj = pfi++;
			pfi->SetTexture (textureP);
			if (*pfi != *pfj)
				nId++;
#if G3_ALLOW_TRANSPARENCY
			if (textureP && (textureP [pfi->nBitmap].props.flags & BM_FLAG_TRANSPARENT))
			m_bHasTransparency = 1;
#endif
		}
		pfi->m_nId = nId;
		}
	}

#if 0// DBG
CModelEdge *edgeP = m_edges.Buffer ();
int32_t nIncomplete = 0;
for (int32_t i = m_nEdges; i; i--, edgeP++) 
	if (edgeP->m_nFaces < 2) 
		nIncomplete++;
#endif

m_vbVerts.SetBuffer (reinterpret_cast<CFloatVector3*> (m_vertBuf [0].Buffer ()), 1, m_vertBuf [0].Length ());
m_vbNormals.SetBuffer (m_vbVerts.Buffer () + m_nFaceVerts, true, m_vertBuf [0].Length ());
m_vbColor.SetBuffer (reinterpret_cast<CFloatVector*> (m_vbNormals.Buffer () + m_nFaceVerts), 1, m_vertBuf [0].Length ());
m_vbTexCoord.SetBuffer (reinterpret_cast<tTexCoord2f*> (m_vbColor.Buffer () + m_nFaceVerts), 1, m_vertBuf [0].Length ());
pv = m_vbVerts.Buffer ();
pn = m_vbNormals.Buffer ();
pt = m_vbTexCoord.Buffer ();
pc = m_vbColor.Buffer ();
pmv = bSort ? m_sortedVerts.Buffer () : m_faceVerts.Buffer ();
for (i = 0, j = m_nFaceVerts; i < j; i++, pmv++) {
	pv [i] = pmv->m_vertex;
	pn [i] = pmv->m_normal;
	pc [i] = pmv->m_baseColor;
	pt [i] = pmv->m_texCoord;
	}
#if DIRECT_VBO
if (m_vertBuf [0].Buffer ()) { // points to graphics driver buffer for VBO based rendering
	glBindBufferARB (GL_ARRAY_BUFFER_ARB, m_vboDataHandle);
	ogl.EnableClientState (GL_VERTEX_ARRAY);
	glBufferDataARB (GL_ARRAY_BUFFER, m_nFaceVerts * sizeof (CRenderVertex), reinterpret_cast<void*> (m_vertBuf [0].Buffer ()), GL_STATIC_DRAW_ARB);
#else
if (m_vertBuf [1].Buffer ()) { // points to graphics driver buffer for VBO based rendering
	memcpy (m_vertBuf [1].Buffer (), m_vertBuf [0].Buffer (), m_vertBuf [1].Size ());
	glUnmapBufferARB (GL_ARRAY_BUFFER_ARB);
	m_vertBuf [1].SetBuffer (NULL);
#endif
	}
#if DIRECT_VBO
if (m_index [0].Buffer ()) { // points to graphics driver buffer for VBO based rendering
	glBindBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, m_vboIndexHandle);
	ogl.EnableClientState (GL_VERTEX_ARRAY);
	glBufferDataARB (GL_ELEMENT_ARRAY_BUFFER_ARB, m_nFaceVerts * sizeof (int16_t), reinterpret_cast<void*> (m_index [0].Buffer ()), GL_STATIC_DRAW_ARB);
#else
if (m_index [1].Buffer ()) { // points to graphics driver buffer for VBO based rendering
	memcpy (m_index [1].Buffer (), m_index [0].Buffer (), m_index [1].Size ());
	glUnmapBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB);
	m_index [1].SetBuffer (NULL);
#endif
	}
if (bSort)
	memcpy (m_faceVerts.Buffer (), m_sortedVerts.Buffer (), m_faceVerts.Size ());
else
	memcpy (m_sortedVerts.Buffer (), m_faceVerts.Buffer (), m_sortedVerts.Size ());
m_bValid = 1;
if (ogl.m_features.bVertexBufferObjects) {
	glBindBufferARB (GL_ARRAY_BUFFER_ARB, 0);
	glBindBufferARB (GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	}
m_sortedVerts.Destroy ();

#if DBG
if (m_nModel == nDbgModel)
	BRP;
#endif

gameStates.render.EnableCartoonStyle ();
for (i = 0; i < (int32_t) m_textures.Length (); i++)
	m_textures [i].PrepareTexture (1, 0);
gameStates.render.DisableCartoonStyle ();
}

//------------------------------------------------------------------------------

int32_t CModel::Shift (CObject *objP, int32_t bHires, CFloatVector3 *vOffsetfP)
{
#if 1
return 0;
#else
	CRenderSubModel*	psm;
	int32_t					i;
	CFloatVector3		vOffsetf;
	CVertex*				pmv;

if (objP->info.nType == OBJ_PLAYER) {
	if (IsMultiGame && !IsCoopGame)
		return 0;
	}
else if ((objP->info.nType != OBJ_ROBOT) && (objP->info.nType != OBJ_HOSTAGE) && (objP->info.nType != OBJ_POWERUP))
	return 0;
if (vOffsetfP)
	vOffsetf = *vOffsetfP;
else
	vOffsetf.Assign (gameData.models.offsets [m_nModel]);
if (!(vOffsetf.dir.coord.x || vOffsetf.dir.coord.y || vOffsetf.dir.coord.z))
	return 0;
if (vOffsetfP) {
	for (i = m_nFaceVerts, pmv = m_faceVerts; i; i--, pmv++) {
		pmv->m_vertex.dir.coord.x += vOffsetf.dir.coord.x;
		pmv->m_vertex.dir.coord.y += vOffsetf.dir.coord.y;
		pmv->m_vertex.dir.coord.z += vOffsetf.dir.coord.z;
		}
	}
else {
	for (i = m_nSubModels, psm = m_subModels; i; i--, psm++) {
		psm->m_vMin.dir.coord.x += vOffsetf.dir.coord.x;
		psm->m_vMin.dir.coord.y += vOffsetf.dir.coord.y;
		psm->m_vMin.dir.coord.z += vOffsetf.dir.coord.z;
		psm->m_vMax.dir.coord.x += vOffsetf.dir.coord.x;
		psm->m_vMax.dir.coord.y += vOffsetf.dir.coord.y;
		psm->m_vMax.dir.coord.z += vOffsetf.dir.coord.z;
		}
	}
return 1;
#endif
}

//------------------------------------------------------------------------------

void CSubModel::Size (CModel* pm, CObject* objP, CFixVector* vOffset)
{
	tHitbox		*phb = (m_nHitbox < 0) ? NULL : gameData.models.hitboxes [pm->m_nModel].hitboxes + m_nHitbox;
	CFixVector	vMin, vMax, vOffs;
	int32_t		i, j;
	CSubModel	*psm;

if (vOffset)
	vOffs = *vOffset + m_vOffset;	//compute absolute offset (i.e. including offsets of all parent submodels)
else
	vOffs = m_vOffset;
if (phb)
	phb->vOffset = vOffs;
vMin.v.coord.x = F2X (m_vMin.v.coord.x);
vMin.v.coord.y = F2X (m_vMin.v.coord.y);
vMin.v.coord.z = F2X (m_vMin.v.coord.z);
vMax.v.coord.x = F2X (m_vMax.v.coord.x);
vMax.v.coord.y = F2X (m_vMax.v.coord.y);
vMax.v.coord.z = F2X (m_vMax.v.coord.z);
m_vCenter = CFixVector::Avg (vMin, vMax);
if (m_bBullets) {
	pm->m_bBullets = 1;
	pm->m_vBullets = m_vCenter;
	}
m_nRad = CFixVector::Dist (vMin, vMax) / 2;
for (i = 0, j = pm->m_nSubModels, psm = pm->m_subModels.Buffer (); i < j; i++, psm++)
	if (psm->m_nParent == m_nSubModel)
		psm->Size (pm, objP, &vOffs);
}

//------------------------------------------------------------------------------

int32_t CModel::CmpVerts (const CFloatVector3* pv, const CFloatVector3* pm)
{
	float h;

h = pv->v.coord.x - pm->v.coord.x;
if (h < 0)
	return -1;
if (h > 0)
	return 1;
h = pv->v.coord.y - pm->v.coord.y;
if (h < 0)
	return -1;
if (h > 0)
	return 1;
h = pv->v.coord.z - pm->v.coord.z;
if (h < 0)
	return -1;
if (h > 0)
	return 1;
return 0;
}

//------------------------------------------------------------------------------
// remove duplicates

uint16_t CModel::FilterVertices (CArray<CFloatVector3>& vertices, uint16_t nVertices)
{
	CFloatVector3	*pi, *pj;

for (pi = vertices.Buffer (), pj = pi + 1, --nVertices; nVertices; nVertices--, pj++)
	if (CmpVerts (pi, pj))
		*++pi = *pj;
return (uint16_t) (pi - vertices) + 1;
}

//------------------------------------------------------------------------------

fix CModel::Radius (CObject *objP)
{
	CSubModel*					psm;
	CFace*						pmf;
	CVertex*						pmv;
	CArray<CFloatVector3>	vertices;
	CFloatVector3				vCenter, vOffset, v, vMin, vMax;
	float							fRad = 0, r;
	uint16_t						h, i, j, k;

tModelSphere *sP = gameData.models.spheres + m_nModel;
if (m_nType >= 0) {
	if ((m_nSubModels == sP->nSubModels) && (m_nFaces == sP->nFaces) && (m_nFaceVerts == sP->nFaceVerts)) {
		gameData.models.offsets [m_nModel] = sP->vOffsets [m_nType];
		return sP->xRads [m_nType];
		}
	}
//first get the biggest distance between any two model vertices
if (vertices.Create (m_nFaceVerts)) {
		CFloatVector3	*pv, *pvi, *pvj;

	for (i = 0, h = m_nSubModels, psm = m_subModels.Buffer (), pv = vertices.Buffer (); i < h; i++, psm++) {
		if (psm->m_nHitbox > 0) {
			vOffset.Assign (gameData.models.hitboxes [m_nModel].hitboxes [psm->m_nHitbox].vOffset);
			for (j = psm->m_nFaces, pmf = psm->m_faces; j; j--, pmf++) {
				for (k = pmf->m_nVerts, pmv = m_faceVerts + pmf->m_nIndex; k; k--, pmv++, pv++)
					*pv = pmv->m_vertex + vOffset;
				}
			}
		}
	h = (uint16_t) (pv - vertices.Buffer ()) - 1;

	CQuickSort<CFloatVector3>	qs;
	qs.SortAscending (vertices.Buffer (), 0, h, &RenderModel::CModel::CmpVerts);

	//G3SortModelVerts (vertices, 0, h);
	h = FilterVertices (vertices, h);
	for (i = 0, pvi = vertices.Buffer (); i < h - 1; i++, pvi++)
		for (j = i + 1, pvj = vertices + j; j < h; j++, pvj++)
			if (fRad < (r = CFloatVector3::Dist (*pvi, *pvj))) {
				fRad = r;
				vMin = *pvi;
				vMax = *pvj;
				}
	fRad /= 2;
	// then move the tentatively computed model center around so that all vertices are enclosed in the sphere
	// around the center with the radius computed above
	vCenter.Assign (gameData.models.offsets [m_nModel]);
	for (i = h, pv = vertices.Buffer (); i; i--, pv++) {
		v = *pv - vCenter;
		r = v.Mag();
		if (fRad < r)
			vCenter += v * ((r - fRad) / r);
		}

	for (i = h, pv = vertices.Buffer (); i; i--, pv++)
		if (fRad < (r = CFloatVector3::Dist (*pv, vCenter)))
			fRad = r;

	vertices.Destroy ();

	gameData.models.offsets [m_nModel].Assign (vCenter);
	if (m_nType >= 0) {
		sP->nSubModels = m_nSubModels;
		sP->nFaces = m_nFaces;
		sP->nFaceVerts = m_nFaceVerts;
		sP->vOffsets [m_nType] = gameData.models.offsets [m_nModel];
		sP->xRads [m_nType] = F2X (fRad);
		}
	}
else {
	// then move the tentatively computed model center around so that all vertices are enclosed in the sphere
	// around the center with the radius computed above
	vCenter.Assign (gameData.models.offsets [m_nModel]);
	for (i = 0, h = m_nSubModels, psm = m_subModels.Buffer (); i < h; i++, psm++) {
		if (psm->m_nHitbox > 0) {
			vOffset.Assign (gameData.models.hitboxes [m_nModel].hitboxes [psm->m_nHitbox].vOffset);
			for (j = psm->m_nFaces, pmf = psm->m_faces; j; j--, pmf++) {
				for (k = pmf->m_nVerts, pmv = m_faceVerts + pmf->m_nIndex; k; k--, pmv++) {
					v = pmv->m_vertex + vOffset;
					if (fRad < (r = CFloatVector3::Dist (v, vCenter)))
						fRad = r;
					}
				}
			}
		}
	gameData.models.offsets [m_nModel].Assign (vCenter);
	}
return F2X (fRad);
}

//------------------------------------------------------------------------------

fix CModel::Size (CObject *objP, int32_t bHires)
{
	CSubModel*		psm;
	int32_t			i, j;
	tHitbox*			phb = &gameData.models.hitboxes [m_nModel].hitboxes [0];
	CFixVector		hv;
	CFloatVector3	vOffset;
	double			dx, dy, dz;

#if DBG
if (m_nModel == nDbgModel)
	BRP;
#endif
psm = m_subModels.Buffer ();
vOffset = psm->m_vMin;

j = 1;
if (m_nSubModels == 1) {
	psm->m_nHitbox = 1;
	gameData.models.hitboxes [m_nModel].nHitboxes = 1;
	}
else {
	for (i = m_nSubModels; i; i--, psm++)
		psm->m_nHitbox = (psm->m_bGlow || psm->m_bThruster || psm->m_bWeapon || (psm->m_nGunPoint >= 0)) ? -1 : j++;
	gameData.models.hitboxes [m_nModel].nHitboxes = j - 1;
	}
do {
	// initialize
	for (i = 0; i <= MAX_HITBOXES; i++) {
		phb [i].vMin.v.coord.x = phb [i].vMin.v.coord.y = phb [i].vMin.v.coord.z = 0x7fffffff;
		phb [i].vMax.v.coord.x = phb [i].vMax.v.coord.y = phb [i].vMax.v.coord.z = -0x7fffffff;
		phb [i].vOffset.v.coord.x = phb [i].vOffset.v.coord.y = phb [i].vOffset.v.coord.z = 0;
		}
	// walk through all submodels, getting their sizes
	if (bHires) {
		for (i = 0; i < m_nSubModels; i++)
			if (m_subModels [i].m_nParent == -1)
				m_subModels [i].Size (this, objP, NULL);
		}
	else
		m_subModels [0].Size (this, objP, NULL);
	// determine min and max size
	for (i = 0, psm = m_subModels.Buffer (); i < m_nSubModels; i++, psm++) {
		if (0 < (j = psm->m_nHitbox)) {
			phb [j].vMin.Assign(psm->m_vMin);
			phb [j].vMax.Assign (psm->m_vMax);
			dx = (phb [j].vMax.v.coord.x - phb [j].vMin.v.coord.x) / 2;
			dy = (phb [j].vMax.v.coord.y - phb [j].vMin.v.coord.y) / 2;
			dz = (phb [j].vMax.v.coord.z - phb [j].vMin.v.coord.z) / 2;
			phb [j].vSize.v.coord.x = (fix) dx;
			phb [j].vSize.v.coord.y = (fix) dy;
			phb [j].vSize.v.coord.z = (fix) dz;
			hv = phb [j].vMin + phb [j].vOffset;
			if (phb [0].vMin.v.coord.x > hv.v.coord.x)
				phb [0].vMin.v.coord.x = hv.v.coord.x;
			if (phb [0].vMin.v.coord.y > hv.v.coord.y)
				phb [0].vMin.v.coord.y = hv.v.coord.y;
			if (phb [0].vMin.v.coord.z > hv.v.coord.z)
				phb [0].vMin.v.coord.z = hv.v.coord.z;
			hv = phb [j].vMax + phb [j].vOffset;
			if (phb [0].vMax.v.coord.x < hv.v.coord.x)
				phb [0].vMax.v.coord.x = hv.v.coord.x;
			if (phb [0].vMax.v.coord.y < hv.v.coord.y)
				phb [0].vMax.v.coord.y = hv.v.coord.y;
			if (phb [0].vMax.v.coord.z < hv.v.coord.z)
				phb [0].vMax.v.coord.z = hv.v.coord.z;
			}
		}
	if (IsMultiGame)
		gameData.models.offsets [m_nModel].v.coord.x =
		gameData.models.offsets [m_nModel].v.coord.y =
		gameData.models.offsets [m_nModel].v.coord.z = 0;
	else {
		gameData.models.offsets [m_nModel].v.coord.x = (phb [0].vMin.v.coord.x + phb [0].vMax.v.coord.x) / -2;
		gameData.models.offsets [m_nModel].v.coord.y = (phb [0].vMin.v.coord.y + phb [0].vMax.v.coord.y) / -2;
		gameData.models.offsets [m_nModel].v.coord.z = (phb [0].vMin.v.coord.z + phb [0].vMax.v.coord.z) / -2;
		}
	} while (Shift (objP, bHires, NULL));

psm = m_subModels.Buffer ();
vOffset = psm->m_vMin - vOffset;
gameData.models.offsets [m_nModel].Assign (vOffset);
#if DBG
if (m_nModel == nDbgModel)
	BRP;
#endif
#if 1
//VmVecInc (&psm [0].vOffset, gameData.models.offsets + m_nModel);
#else
G3ShiftModel (objP, m_nModel, bHires, &vOffset);
#endif
phb [0].vSize = phb [0].vMax - phb [0].vMin;
gameData.models.offsets [m_nModel] = CFixVector::Avg (phb [0].vMax, phb [0].vMin);
//phb [0].vOffset = gameData.models.offsets [m_nModel];
for (i = 0; i <= j; i++)
	ComputeHitbox (m_nModel, i);
return Radius (objP);
}

//------------------------------------------------------------------------------

int32_t CModel::MinMax (tHitbox *phb)
{
	CSubModel*	psm;
	int32_t		i;

for (i = m_nSubModels, psm = m_subModels.Buffer (); i; i--, psm++) {
	if (!psm->m_bThruster && (psm->m_nGunPoint < 0)) {
		phb->vMin.v.coord.x = F2X (psm->m_vMin.v.coord.x);
		phb->vMin.v.coord.y = F2X (psm->m_vMin.v.coord.y);
		phb->vMin.v.coord.z = F2X (psm->m_vMin.v.coord.z);
		phb->vMax.v.coord.x = F2X (psm->m_vMax.v.coord.x);
		phb->vMax.v.coord.y = F2X (psm->m_vMax.v.coord.y);
		phb->vMax.v.coord.z = F2X (psm->m_vMax.v.coord.z);
		phb++;
		}
	}
return m_nSubModels;
}

//------------------------------------------------------------------------------

void CModel::SetShipGunPoints (OOF::CModel *po)
{
	static int16_t nGunSubModels [] = {6, 7, 5, 4, 9, 10, 3, 3};

	CSubModel*		psm;
	OOF::CPoint*	pp = po->m_gunPoints.Buffer ();

po->m_gunPoints.Resize (N_PLAYER_GUNS);
for (uint32_t i = 0; i < po->m_gunPoints.Length (); i++, pp++) {
	if (nGunSubModels [i] >= m_nSubModels)
		continue;
	m_nGunSubModels [i] = nGunSubModels [i];
	psm = m_subModels + nGunSubModels [i];
	pp->m_vPos.v.coord.x = (psm->m_vMax.v.coord.x + psm->m_vMin.v.coord.x) / 2;
	if (3 == (pp->m_nParent = nGunSubModels [i])) {
		pp->m_vPos.v.coord.y = (psm->m_vMax.v.coord.y + 3 * psm->m_vMin.v.coord.y) / 4;
		pp->m_vPos.v.coord.z = 7 * (psm->m_vMax.v.coord.z + psm->m_vMin.v.coord.z) / 8;
		}
	else {
		pp->m_vPos.v.coord.y = (psm->m_vMax.v.coord.y + psm->m_vMin.v.coord.y) / 2;
		if (i < 4)
      	pp->m_vPos.v.coord.z = psm->m_vMax.v.coord.z;
		else
			pp->m_vPos.v.coord.z = (psm->m_vMax.v.coord.z + psm->m_vMin.v.coord.z) / 2;
		}
	}
}

//------------------------------------------------------------------------------

void CModel::SetRobotGunPoints (OOF::CModel *po)
{
	CSubModel*		psm;
	OOF::CPoint*	pp;
	int32_t			i, j = po->m_gunPoints.Length ();

for (i = 0, pp = po->m_gunPoints.Buffer (); i < j; i++, pp++) {
	m_nGunSubModels [i] = pp->m_nParent;
	psm = m_subModels + pp->m_nParent;
	pp->m_vPos.v.coord.x = (psm->m_vMax.v.coord.x + psm->m_vMin.v.coord.x) / 2;
	pp->m_vPos.v.coord.y = (psm->m_vMax.v.coord.y + psm->m_vMin.v.coord.y) / 2;
  	pp->m_vPos.v.coord.z = psm->m_vMax.v.coord.z;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

int32_t NearestGunPoint (CFixVector *vGunPoints, CFixVector *vGunPoint, int32_t nGuns, int32_t *nUsedGuns)
{
	fix			xDist, xMinDist = 0x7fffffff;
	int32_t		h = 0, i;
	CFixVector	vi, v0 = *vGunPoint;

v0.v.coord.z = 0;
for (i = 0; i < nGuns; i++) {
	if (nUsedGuns [i])
		continue;
	vi = vGunPoints [i];
	vi.v.coord.z = 0;
	xDist = CFixVector::Dist(vi, v0);
	if (xMinDist > xDist) {
		xMinDist = xDist;
		h = i;
		}
	}
nUsedGuns [h] = 1;
return h;
}

//------------------------------------------------------------------------------

fix G3ModelRad (CObject *objP, int32_t m_nModel, int32_t bHires)
{
return gameData.models.renderModels [bHires][m_nModel].Radius (objP);
}

//------------------------------------------------------------------------------

fix G3ModelSize (CObject *objP, int32_t m_nModel, int32_t bHires)
{
return gameData.models.renderModels [bHires][m_nModel].Size (objP, bHires);
}

//------------------------------------------------------------------------------

void G3SetShipGunPoints (OOF::CModel *po, CModel* pm)
{
pm->SetShipGunPoints (po);
}

//------------------------------------------------------------------------------

void G3SetRobotGunPoints (OOF::CModel *po, CModel* pm)
{
pm->SetRobotGunPoints (po);
}

//------------------------------------------------------------------------------

void CModel::SetGunPoints (CObject *objP, int32_t bASE)
{
	CFixVector*	vGunPoints;
	int32_t		nParent, h, i, j;

if (bASE) {
	CSubModel	*psm = m_subModels.Buffer ();

	vGunPoints = gameData.models.gunInfo [m_nModel].vGunPoints;
	for (i = 0, j = 0; i < m_nSubModels; i++, psm++) {
		h = psm->m_nGunPoint;
		if ((h >= 0) && (h < MAX_GUNS)) {
			j++;
			m_nGunSubModels [h] = i;
			vGunPoints [h] = psm->m_vCenter;
			}
		}
	gameData.models.gunInfo [m_nModel].nGuns = j;
	}
else {
		OOF::CPoint		*pp;
		OOF::CSubModel	*pso;
		OOF::CModel		*po = gameData.models.modelToOOF [1][m_nModel];

	if (!po)
		po = gameData.models.modelToOOF [0][m_nModel];
	pp = po->m_gunPoints.Buffer ();
	if (objP->info.nType == OBJ_PLAYER)
		SetShipGunPoints (po);
	else if (objP->info.nType == OBJ_ROBOT)
		SetRobotGunPoints (po);
	else {
		gameData.models.gunInfo [m_nModel].nGuns = 0;
		return;
		}
	if ((j = po->m_gunPoints.Length ())) {
		if (j > MAX_GUNS)
			j = MAX_GUNS;
		gameData.models.gunInfo [m_nModel].nGuns = j;
		vGunPoints = gameData.models.gunInfo [m_nModel].vGunPoints;
		for (i = 0; i < j; i++, pp++, vGunPoints++) {
			(*vGunPoints).v.coord.x = F2X (pp->m_vPos.v.coord.x);
			(*vGunPoints).v.coord.y = F2X (pp->m_vPos.v.coord.y);
			(*vGunPoints).v.coord.z = F2X (pp->m_vPos.v.coord.z);
			for (nParent = pp->m_nParent; nParent >= 0; nParent = pso->m_nParent) {
				pso = po->m_subModels + nParent;
				(*vGunPoints) += m_subModels [nParent].m_vOffset;
				}
			}
		}
	}
}

//------------------------------------------------------------------------------

int32_t G3BuildModel (CObject* objP, int32_t nModel, CPolyModel* pp, CArray<CBitmap*>& modelBitmaps, CFloatVector* pObjColor, int32_t bHires)
{
	RenderModel::CModel*	pm = gameData.models.renderModels [bHires] + nModel;

if (pm->m_bValid > 0)
	return 1;
if (pm->m_bValid < 0)
	return 0;
pm->m_bRendered = 0;
#if DBG
if (nModel == nDbgModel)
	BRP;
#endif
pm->Init ();
return bHires
		 ? GetASEModel (nModel)
			? pm->BuildFromASE (objP, nModel)
			: pm->BuildFromOOF (objP, nModel)
		 : pm->BuildFromPOF (objP, nModel, pp, modelBitmaps, pObjColor);
}

//------------------------------------------------------------------------------

int32_t G3ModelMinMax (int32_t m_nModel, tHitbox *phb)
{
	RenderModel::CModel*		pm;

if (!((pm = gameData.models.renderModels [1] + m_nModel) ||
	   (pm = gameData.models.renderModels [0] + m_nModel)))
	return 0;
return pm->MinMax (phb);
}

//------------------------------------------------------------------------------
//eof
