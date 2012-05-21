#include <stdlib.h>

#include "mine.h"
#include "dle-xp.h"
#include "frustum.h"

//------------------------------------------------------------------------------

#define COMPUTE_TYPE 0

static int planeVerts [6][4] = {
	{0,1,2,3},{0,1,5,4},{1,2,6,5},{2,3,7,6},{0,3,7,4},{4,5,6,7}
	};

//static int oppSides [6] = {5, 3, 4, 1, 2, 0};

static int normRefs [6][2] = {{1,5},{4,7},{5,4},{7,4},{4,5},{5,1}};

void CFrustum::Setup (CRect viewport, double fov)
{
double h = double (tan (fov * PI / 360.0));
double w = double (h * double (viewport.Width ()) / double (viewport.Height ()));
double n = double (1.0);
double f = double (50000.0);
double m = f * 0.5f;
double r = f / n;

#define ln -w
#define rn +w
#define tn +h
#define bn -h

#define lf (ln * r)
#define rf (rn * r)
#define tf (tn * r)
#define bf (bn * r)

for (int i = 0; i < 4; i++)
	m_corners [i].Clear ();
m_corners [4].Set (lf, bf, f);
m_corners [5].Set (lf, rf, f);
m_corners [6].Set (rf, tf, f);
m_corners [7].Set (rf, bf, f);
m_centers [0].Set (0.0, 0.0, 0.0);
m_centers [1].Set (lf * 0.5, 0.0, m);
m_centers [1].Set (0.0f, tf * 0.5, m);
m_centers [1].Set (rf * 0.5, 0.0, m);
m_centers [1].Set (0.0, bf * 0.5, m);
m_centers [1].Set (0.0, 0.0, f);


m_centers [0].Clear ();
m_normals [0].Set (0.0, 0.0, 1.0);
for (int i = 1; i < 6; i++) {
	m_normals [i] = Normal (m_corners [planeVerts [i][1]], m_corners [planeVerts [i][2]], m_corners [planeVerts [i][3]]);
	for (int j = 0; j < 4; j++)
		m_centers [i] += m_corners [planeVerts [i][j]];
	m_centers [i] /= 4.0;
	CDoubleVector v = m_corners [normRefs [i][1]] - m_corners [normRefs [i][0]];
	v.Normalize ();
	if (Dot (v, m_normals [i]) < 0)
		m_normals [i].Negate ();
	}
}

//------------------------------------------------------------------------------
// Check whether the frustum intersects with a face defined by side *sideP.

bool CFrustum::Contains (short nSegment, short nSide)
{
	static int lineVerts [12][2] = {
		{0,1}, {1,2}, {2,3}, {3,0}, 
		{4,5}, {5,6}, {6,7}, {7,4},
		{0,4}, {1,5}, {2,6}, {3,7}
	};

	CSegment* segP = segmentManager.Segment (nSegment);
	CSide* sideP = segP->Side (nSide);
	int i, j, nInside = 0, nOutside [4] = {0, 0, 0, 0}, nVertices = sideP->VertexCount ();
	CVertex*	points [4];
	CDoubleVector intersection;

for (i = 0; i < nVertices; i++) {
	points [i] = segP->Vertex (nSide, i);
	}

// check whether all vertices of the face are at the back side of at least one frustum plane,
// or if at least one is at at least on one frustum plane's front sides
for (i = 0; i < 6; i++) {
	int nPtInside = 4;
	int bPtInside = 1;
	CDoubleVector& c = m_centers [i];
	CDoubleVector& n = m_normals [i];
	for (j = 0; j < 4; j++) {
		CDoubleVector v = points [j]->m_view - c;
		Normalize (v);
		if (Dot (n, v) < 0) {
			if (!--nPtInside)
				return false;
			++nOutside [j];
			bPtInside = 0;
			}
		}
	nInside += bPtInside;
	}

if (nInside == 6)
	return true; // face completely contained
for (j = 0; j < 4; j++) 
	if (!nOutside [j])
		return true; // some vertex inside frustum

// check whether the frustum intersects with the face
// to do that, compute the intersections of all frustum edges with the plane(s) spanned up by the face (two planes if face not planar)
// if an edge intersects, check whether the intersection is inside the face
// since the near plane is at 0.0, only 8 edges of 5 planes need to be checked
for (i = 11; i >= 4; i--)
	if (sideP->LineHitsFace (&m_corners [lineVerts [i][0]], &m_corners [lineVerts [i][1]], segP->m_info.vertexIds, 0.0, true))
		return true;
return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//eof

