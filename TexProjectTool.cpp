// TexProjectTool.cpp
//

#include "stdafx.h"
#include "dle-xp.h"
#include "toolview.h"

#define PROJECTION_MODE_PLANAR   1
#define PROJECTION_MODE_CYLINDER 2

#define PREVIEWUVLS_EXPAND_INCREMENT 256

void CTextureTool::UpdateProjectWnd (void)
{
	CToolDlg::EnableControls (IDC_TEXPROJECT_ORIGINX, IDC_TEXPROJECT_COPIEDONLY, m_bProjectToolActive);
}

void CTextureTool::StartProjectionTool (int nProjectionMode)
{
	UpdateData (TRUE);
	m_nProjectionMode = nProjectionMode;

	if(m_bProjectToolActive) {
		switch(m_nProjectionMode) {
		case PROJECTION_MODE_PLANAR:
			m_projectScaleU = m_projectScaleV;
			break;

		case PROJECTION_MODE_CYLINDER:
			m_projectScaleU = CalcAverageRadius() * 0.4;
			break;

		default:
			break;
		}
	} else {
		m_bProjectToolActive = TRUE;

		switch(m_nProjectionMode) {
		case PROJECTION_MODE_PLANAR:
			ResetProjectOffset();
			ResetProjectDirection();
			m_projectScaleU = 1;
			m_projectScaleV = 1;
			break;

		case PROJECTION_MODE_CYLINDER:
			ResetProjectOffset();
			ResetProjectDirection();
			// 4 repeats for every 10 units radius - this scales exactly to a square cross-section.
			// May distort a little for more sides; substantially for triangles but these are rare.
			m_projectScaleU = CalcAverageRadius() * 0.4;
			m_projectScaleV = 1;
			break;

		default:
			break;
		}
	}

	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
	UpdateData (FALSE);
	UpdateProjectWnd ();
}

void CTextureTool::EndProjectionTool (BOOL bApply)
{
	if (bApply) {
		Project ();
		DLE.MineView ()->Refresh (false);
	} else if (m_bProjectPreview) {
		DLE.MineView ()->Refresh (false);
	}

	// Clear UI and disable the controls
	m_bProjectToolActive = FALSE;
	m_projectRotP = 0;
	m_projectRotB = 0;
	m_projectRotH = 0;
	m_projectScaleU = 0;
	m_projectScaleV = 0;
	UpdateData (FALSE);
	UpdateProjectWnd ();
}

void CTextureTool::ResetProjectOffset ()
{
	m_vCenter = current->Segment ()->ComputeCenter ();
}

void CTextureTool::ResetProjectDirection ()
{
	CSegment* segP = current->Segment ();
	CSide* sideP = current->Side ();
	short nSide = current->SideId ();
	short nEdge = current->Edge ();
	ubyte v1 = sideP->VertexIdIndex (nEdge);
	ubyte v2 = sideP->VertexIdIndex (nEdge + 1);
	sideP->ComputeNormals (segP->m_info.vertexIds, m_vCenter);
	segP->ComputeCenter (nSide);

	CDoubleVector fVec = sideP->Normal ();
	fVec.Negate ();
	CDoubleVector uVec = vertexManager [segP->m_info.vertexIds [v1]] - vertexManager [segP->m_info.vertexIds [v2]];
	uVec.Normalize ();
	CDoubleVector rVec = CrossProduct (uVec, fVec);

	m_projectOrient = CDoubleMatrix (rVec, uVec, fVec);
	m_projectOrient.CopyTo (m_projectRotP, m_projectRotB, m_projectRotH);
	m_projectRotP = Degrees (m_projectRotP);
	m_projectRotB = Degrees (m_projectRotB);
	m_projectRotH = Degrees (m_projectRotH);
	ClampProjectDirection ();
}

void CTextureTool::ClampProjectDirection ()
{
	// Keep display roughly in the -179 to 180 range
	if (m_projectRotP + 180 < 0.01)
		m_projectRotP += 360;
	else if (m_projectRotP > 180.01)
		m_projectRotP -= 360;
	if (m_projectRotB + 180 < 0.01)
		m_projectRotB += 360;
	else if (m_projectRotB > 180.01)
		m_projectRotB -= 360;
	if (m_projectRotH + 180 < 0.01)
		m_projectRotH += 360;
	else if (m_projectRotH > 180.01)
		m_projectRotH -= 360;
}

double CTextureTool::CalcAverageRadius ()
{
	double sumDistance = 0;
	ushort count = 0;

	#pragma omp parallel for if (segmentManager.Count () > 15) reduction (+ : sumDistance, count)
	for (int i = 0; i < segmentManager.Count (); i++) {
		for (short j = 0; j < MAX_SIDES_PER_SEGMENT; j++) {
			if (!ShouldProjectFace (i, j))
				continue;
			CDoubleVector vSideCenter = segmentManager.Segment (i)->ComputeCenter (j);
			sumDistance += Distance (vSideCenter, ProjectPointOnLine (&m_vCenter, &m_projectOrient.U (), &vSideCenter));
			count++;
		}
	}

	if (!count)
		return 0;
	return sumDistance / count;
}

void CTextureTool::Project (CDynamicArray< CPreviewUVL > *previewUVLs)
{
	uint nModifiedUVLs = 0;
	if (!previewUVLs)
		undoManager.Begin (__FUNCTION__, udSegments, true);
	else
		previewUVLs->Create (PREVIEWUVLS_EXPAND_INCREMENT);

	#pragma omp parallel for if (segmentManager.Count () > 15 && !previewUVLs)
	for (int i = 0; i < segmentManager.Count (); i++) {
		#pragma omp parallel for if (segmentManager.Count () > 15 && !previewUVLs)
		for (short j = 0; j < MAX_SIDES_PER_SEGMENT; j++) {
			if (!ShouldProjectFace (i, j))
				continue;

			uint nThisUVL = nModifiedUVLs++;
			if (previewUVLs && nModifiedUVLs >= previewUVLs->Length ()) {
				previewUVLs->Resize (nModifiedUVLs + PREVIEWUVLS_EXPAND_INCREMENT);
			}

			CSegment* segP = segmentManager.Segment (i);
			CSide *sideP = segP->Side (j);
			CUVL vTranslate;
			double maxAngle = 0, minAngle = (2 * M_PI);
			CUVL uvlModified [4];

			for (short k = 0; k < sideP->VertexCount (); k++) {
				// Copy UVL from face
				uvlModified [k] = *sideP->Uvls (k);

				switch (m_nProjectionMode) {
				case PROJECTION_MODE_PLANAR:
					uvlModified [k].u = Dot (*segP->Vertex (j, k) - m_vCenter, m_projectOrient.R ()) / 20.0 - 0.5;
					uvlModified [k].v = Dot (*segP->Vertex (j, k) - m_vCenter, m_projectOrient.U ()) / 20.0 - 0.5;
					break;
				case PROJECTION_MODE_CYLINDER:
					{
						CDoubleVector vIntersection = ProjectPointOnPlane (&m_vCenter, &m_projectOrient.U (), segP->Vertex (j, k));
						vIntersection = m_projectOrient * (vIntersection - m_vCenter);
						double angle = (vIntersection.v.x || vIntersection.v.z) ? atan3 (vIntersection.v.x, vIntersection.v.z) : 0;
						maxAngle = max (maxAngle, angle);
						minAngle = min (minAngle, angle);
						uvlModified [k].u = angle / (2 * M_PI);
					}
					uvlModified [k].v = Dot (*segP->Vertex (j, k) - m_vCenter, m_projectOrient.U ()) / 20.0 - 0.5;
					break;
				default:
					break;
				}
				// V tends to be reversed because textures are top-down
				uvlModified [k].u *= m_projectScaleU;
				uvlModified [k].v *= -m_projectScaleV;
			}
			if (m_nProjectionMode == PROJECTION_MODE_CYLINDER) {
				for (short k = 0; k < sideP->VertexCount (); k++) {
					// Check if the texture wrapped and if so, bump the lower-angle UVs up
					if ((maxAngle - minAngle) > M_PI && uvlModified [k].u < 0) {
						uvlModified [k].u += m_projectScaleU;
					}

					// Translate textures so the first vertex is in the -20 to 20 range
					if (k == 0) {
						vTranslate = CUVL (fmod (uvlModified [k].u, 1) - uvlModified [k].u,
							fmod (uvlModified [k].v, 1) - uvlModified [k].v);
					}
					uvlModified [k] += vTranslate;
				}
			}

			// Write modified UVLs back to face or to preview
			if (previewUVLs) {
				(*previewUVLs) [nThisUVL] = CPreviewUVL (i, j);
				for (short k = 0; k < sideP->VertexCount (); k++) {
					(*previewUVLs) [nThisUVL].m_uvlOld [k] = *sideP->Uvls (k);
					(*previewUVLs) [nThisUVL].m_uvlPreview [k] = uvlModified [k];
				}
			} else {
				for (short k = 0; k < sideP->VertexCount (); k++)
					*sideP->Uvls (k) = uvlModified [k];
			}
		}
	}

	if (!previewUVLs)
		undoManager.End (__FUNCTION__);
	else if (nModifiedUVLs > 0)
		previewUVLs->Resize (nModifiedUVLs);
	else
		previewUVLs->Destroy ();
}

bool CTextureTool::ShouldProjectFace (short nSegment, short nSide)
{
	CSide *sideP = segmentManager.Segment (nSegment)->Side (nSide);
	if (sideP->Shape () > SIDE_SHAPE_TRIANGLE)
		return false;
	// If any vertices are tagged, use tagged faces
	if (vertexManager.HasTaggedVertices ()) {
		if (!sideP->IsTagged ())
			return false;
	}
	// Find faces that match the saved texture(s) if requested
	if (m_bProjectCopiedOnly && m_bUse1st && (sideP->BaseTex () != m_saveTexture [0]))
		return false;
	if (m_bProjectCopiedOnly && m_bUse2nd && (sideP->OvlTex (0) != m_saveTexture [1]))
		return false;
	return true;
}

void CTextureTool::DrawProjectGuides (CRenderer& renderer, CViewMatrix* viewMatrix)
{
if(!m_bProjectToolActive || !m_nProjectionMode)
	return;
CDC* pDC = renderer.DC ();

CVertex vCenter = m_vCenter;
CVertex projectAxes [3] = { m_projectOrient.R (), m_projectOrient.U (), m_projectOrient.F () };
CVertex planeVertices [4] = { CVertex (-5, -5, 0), CVertex (-5, 5, 0), CVertex (5, 5, 0), CVertex (5, -5, 0) };
CPoint planePoints [4];
CDoubleVector vScale (m_projectScaleU, m_projectScaleV, 1);
int nCylinderLines = (int)ceil (abs (m_projectScaleU));
CVertex *cylinderLineVertices [2];
CVertex cylinderEndVertices [2][60];
CPoint cylinderEndPoints [2][60];

renderer.BeginRender ();
vCenter.Transform (viewMatrix);
vCenter.Project (viewMatrix);
for (int i = 0; i < 3; i++) {
	projectAxes [i].Normalize ();
	projectAxes [i] *= 5.0;
	projectAxes [i] += vCenter;
	projectAxes [i].Transform (viewMatrix);
	projectAxes [i].Project (viewMatrix);
	}
CDoubleMatrix inverse = m_projectOrient.Inverse ();
switch (m_nProjectionMode) {
	case PROJECTION_MODE_PLANAR:
		for (int i = 0; i < 4; i++) {
			planeVertices [i] /= vScale;
			planeVertices [i] = inverse * planeVertices [i];
			planeVertices [i] += vCenter;
			planeVertices [i].Transform (viewMatrix);
			planeVertices [i].Project (viewMatrix);
			planePoints [i] = CPoint (planeVertices [i].m_screen.x, planeVertices [i].m_screen.y);
			}
		break;
	case PROJECTION_MODE_CYLINDER:
		for (int i = 0; i < 2; i++) {
			cylinderLineVertices [i] = new CVertex [nCylinderLines];
			for (int j = 0; j < nCylinderLines; j++) {
				cylinderLineVertices [i][j] = CVertex (0, pow ((double)-1, i) * 5 / m_projectScaleV, 5);
				cylinderLineVertices [i][j].Rotate (CDoubleVector (0, 1, 0), j * 2 * M_PI / m_projectScaleU);
				cylinderLineVertices [i][j] = inverse * cylinderLineVertices [i][j];
				cylinderLineVertices [i][j] += vCenter;
				cylinderLineVertices [i][j].Transform (viewMatrix);
				cylinderLineVertices [i][j].Project (viewMatrix);
				}
			for (int j = 0; j < 60; j++) {
				cylinderEndVertices [i][j] = CVertex (0, pow ((double)-1, i) * 5 / m_projectScaleV, 5);
				cylinderEndVertices [i][j].Rotate (CDoubleVector (0, 1, 0), j * 2 * M_PI / 60);
				cylinderEndVertices [i][j] = inverse * cylinderEndVertices [i][j];
				cylinderEndVertices [i][j] += vCenter;
				cylinderEndVertices [i][j].Transform (viewMatrix);
				cylinderEndVertices [i][j].Project (viewMatrix);
				cylinderEndPoints [i][j] = CPoint (cylinderEndVertices [i][j].m_screen.x,
					cylinderEndVertices [i][j].m_screen.y);
				}
			}
		break;
	default:
		break;
}
renderer.EndRender ();

renderer.BeginRender (true);
renderer.SelectObject ((HBRUSH)GetStockObject (NULL_BRUSH));
static ePenColor pens [3] = { penRed, penMedGreen, penMedBlue };

renderer.Ellipse (vCenter, 4, 4);
for (int i = 0; i < 3; i++) {
	renderer.SelectPen (pens [i] + 1);
	renderer.MoveTo (vCenter.m_screen.x, vCenter.m_screen.y);
	renderer.LineTo (projectAxes [i].m_screen.x, projectAxes [i].m_screen.y);
	}
renderer.SelectPen (penDkGreen + 1);
switch (m_nProjectionMode) {
	case PROJECTION_MODE_PLANAR:
		renderer.Polygon (planePoints, 4);
		break;
	case PROJECTION_MODE_CYLINDER:
		renderer.Polygon (cylinderEndPoints [0], 60);
		renderer.Polygon (cylinderEndPoints [1], 60);
		for (int i = 0; i < nCylinderLines; i++) {
			renderer.MoveTo (cylinderLineVertices [0][i].m_screen.x, cylinderLineVertices [0][i].m_screen.y);
			renderer.LineTo (cylinderLineVertices [1][i].m_screen.x, cylinderLineVertices [1][i].m_screen.y);
			}
		delete [] cylinderLineVertices [0];
		delete [] cylinderLineVertices [1];
		break;
	default:
		break;
}
renderer.EndRender ();
}

void CTextureTool::OnProjectPlanar ()
{
	StartProjectionTool (PROJECTION_MODE_PLANAR);
}

void CTextureTool::OnProjectCylinder ()
{
	StartProjectionTool (PROJECTION_MODE_CYLINDER);
}

void CTextureTool::OnProjectApply ()
{
	EndProjectionTool (TRUE);
}

void CTextureTool::OnProjectCancel ()
{
	EndProjectionTool (FALSE);
}

void CTextureTool::OnProjectOffsetXUp ()
{
	UpdateData (TRUE);
	m_vCenter.v.x += (DLE.MineView ()->MineMoveRate ());
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectOffsetXDown ()
{
	UpdateData (TRUE);
	m_vCenter.v.x -= (DLE.MineView ()->MineMoveRate ());
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectOffsetYUp ()
{
	UpdateData (TRUE);
	m_vCenter.v.y += (DLE.MineView ()->MineMoveRate ());
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectOffsetYDown ()
{
	UpdateData (TRUE);
	m_vCenter.v.y -= (DLE.MineView ()->MineMoveRate ());
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectOffsetZUp ()
{
	UpdateData (TRUE);
	m_vCenter.v.z += (DLE.MineView ()->MineMoveRate ());
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectOffsetZDown ()
{
	UpdateData (TRUE);
	m_vCenter.v.z -= (DLE.MineView ()->MineMoveRate ());
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectRotPUp ()
{
	UpdateData (TRUE);
	m_projectRotP += Degrees (theMine->RotateRate ());
	ClampProjectDirection ();
	m_projectOrient.Set (sin (Radians (m_projectRotP)), cos (Radians (m_projectRotP)),
		sin (Radians (m_projectRotB)), cos (Radians (m_projectRotB)),
		sin (Radians (m_projectRotH)), cos (Radians (m_projectRotH)));
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectRotPDown ()
{
	UpdateData (TRUE);
	m_projectRotP -= Degrees (theMine->RotateRate ());
	ClampProjectDirection ();
	m_projectOrient.Set (sin (Radians (m_projectRotP)), cos (Radians (m_projectRotP)),
		sin (Radians (m_projectRotB)), cos (Radians (m_projectRotB)),
		sin (Radians (m_projectRotH)), cos (Radians (m_projectRotH)));
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectRotBUp ()
{
	UpdateData (TRUE);
	m_projectRotB += Degrees (theMine->RotateRate ());
	ClampProjectDirection ();
	m_projectOrient.Set (sin (Radians (m_projectRotP)), cos (Radians (m_projectRotP)),
		sin (Radians (m_projectRotB)), cos (Radians (m_projectRotB)),
		sin (Radians (m_projectRotH)), cos (Radians (m_projectRotH)));
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectRotBDown ()
{
	UpdateData (TRUE);
	m_projectRotB -= Degrees (theMine->RotateRate ());
	ClampProjectDirection ();
	m_projectOrient.Set (sin (Radians (m_projectRotP)), cos (Radians (m_projectRotP)),
		sin (Radians (m_projectRotB)), cos (Radians (m_projectRotB)),
		sin (Radians (m_projectRotH)), cos (Radians (m_projectRotH)));
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectRotHUp ()
{
	UpdateData (TRUE);
	m_projectRotH += Degrees (theMine->RotateRate ());
	ClampProjectDirection ();
	m_projectOrient.Set (sin (Radians (m_projectRotP)), cos (Radians (m_projectRotP)),
		sin (Radians (m_projectRotB)), cos (Radians (m_projectRotB)),
		sin (Radians (m_projectRotH)), cos (Radians (m_projectRotH)));
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectRotHDown ()
{
	UpdateData (TRUE);
	m_projectRotH -= Degrees (theMine->RotateRate ());
	ClampProjectDirection ();
	m_projectOrient.Set (sin (Radians (m_projectRotP)), cos (Radians (m_projectRotP)),
		sin (Radians (m_projectRotB)), cos (Radians (m_projectRotB)),
		sin (Radians (m_projectRotH)), cos (Radians (m_projectRotH)));
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectScaleUUp ()
{
	UpdateData (TRUE);
	m_projectScaleU += (DLE.MineView ()->MineMoveRate () / 20.0);
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectScaleUDown ()
{
	UpdateData (TRUE);
	m_projectScaleU -= (DLE.MineView ()->MineMoveRate () / 20.0);
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectScaleVUp ()
{
	UpdateData (TRUE);
	m_projectScaleV += (DLE.MineView ()->MineMoveRate () / 20.0);
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectScaleVDown ()
{
	UpdateData (TRUE);
	m_projectScaleV -= (DLE.MineView ()->MineMoveRate () / 20.0);
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectOffsetX ()
{
	UpdateData (TRUE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectOffsetY ()
{
	UpdateData (TRUE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectOffsetZ ()
{
	UpdateData (TRUE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectRotP ()
{
	UpdateData (TRUE);
	ClampProjectDirection ();
	m_projectOrient.Set (sin (Radians (m_projectRotP)), cos (Radians (m_projectRotP)),
		sin (Radians (m_projectRotB)), cos (Radians (m_projectRotB)),
		sin (Radians (m_projectRotH)), cos (Radians (m_projectRotH)));
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectRotB ()
{
	UpdateData (TRUE);
	ClampProjectDirection ();
	m_projectOrient.Set (sin (Radians (m_projectRotP)), cos (Radians (m_projectRotP)),
		sin (Radians (m_projectRotB)), cos (Radians (m_projectRotB)),
		sin (Radians (m_projectRotH)), cos (Radians (m_projectRotH)));
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectRotH ()
{
	UpdateData (TRUE);
	ClampProjectDirection ();
	m_projectOrient.Set (sin (Radians (m_projectRotP)), cos (Radians (m_projectRotP)),
		sin (Radians (m_projectRotB)), cos (Radians (m_projectRotB)),
		sin (Radians (m_projectRotH)), cos (Radians (m_projectRotH)));
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectScaleU ()
{
	UpdateData (TRUE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectScaleV ()
{
	UpdateData (TRUE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectResetOffset ()
{
	UpdateData (TRUE);
	ResetProjectOffset();
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectResetDirection ()
{
	UpdateData (TRUE);
	ResetProjectDirection();
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectResetScaling ()
{
	UpdateData (TRUE);
	switch(m_nProjectionMode) {
	case PROJECTION_MODE_PLANAR:
		m_projectScaleU = 1;
		m_projectScaleV = 1;
		break;

	case PROJECTION_MODE_CYLINDER:
		m_projectScaleU = CalcAverageRadius() * 0.4;
		m_projectScaleV = 1;
		break;

	default:
		break;
	}
	UpdateData (FALSE);
	if (m_bProjectPreview)
		DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectPreview ()
{
	m_bProjectPreview = !m_bProjectPreview;
	DLE.MineView ()->Refresh (false);
}

void CTextureTool::OnProjectCopiedOnly ()
{
	m_bProjectCopiedOnly = !m_bProjectCopiedOnly;
	DLE.MineView ()->Refresh (false);
}

//eof TexProjectTool.cpp