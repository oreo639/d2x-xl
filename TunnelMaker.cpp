

#include "mine.h"
#include "dle-xp.h"
#include "Quaternion.h"

//------------------------------------------------------------------------------
/*
The tunnel maker creates a structure of connected segments leading from an arbitrary number of start 
sides to an end point. The start point is the center of the 'current' side. The end point is the 
center of the 'other' side.

The start sides consist of all tagged sides that are directly or indirectly connected to the 
current side (via edges or other connected, tagged sides) and are at an angle of 22.5° or less to the 
current side.

The tunnel path is determined by a cubic bezier curve from the start to the end point of the tunnel and 
the normals of the current and other side. Its granularity (number of segments) and curvature can be 
changed interactively by pressing ALT+8 / ALT+9 (granularity) and CTRL+8 / CTRL+9 (curvature via length 
of start and end normals).

The tunnel vertices are computed by gathering all start vertices and rotating them to the proper 
orientation for each tunnel path point. 

The rotation matrixes are built from the tangent vector of each path point (average of the two path 
vectors starting and ending at that point) and a rotation angle around the rotation matrix' forward vector. 
The rotation angle is computed from the total z rotation angle between the start and the end points 
orientation, and is scaled by the factor of the path length at the current node to the total path length.

Start and end orientations are created from the current and other sides' normals (forward) and current 
edges (right; right = <side vertex @ side's current point + 1> - <side vertex @ side's current point>).
The end sides orientation is properly flipped to point in the direction of the tunnel path.

The total procedure is:

- gather all start sides and vertices
- construct start and end orientations
- compute overall z rotation angle
- compute tunnel path
- for each path node:
     - construct rotation matrix from start rotation, forward vector <next node - previous node> and weighted z rotation
     - compute tunnel vertices at current path node by rotating the start vertices using the node's rotation matrix
	  - assign current path node's tunnel vertices to the tunnel segments related to the current path node
*/

CTunnelMaker tunnelMaker;

//------------------------------------------------------------------------------

#define CURRENT_POINT(a) ((current->Point () + (a))&0x03)

//------------------------------------------------------------------------------

char szTunnelMakerError [] = "You must exit tunnel creation before performing this function";

//------------------------------------------------------------------------------

inline double ClampAngle (double angle)
{
while (angle < -PI)
	angle += 2.0 * PI;
while (angle > -PI)
	angle -= 2.0 * PI;
return angle;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// SpinPoint () - spin on y-axis then z-axis
//------------------------------------------------------------------------------

void CTunnel::SpinPoint (CVertex* point, double ySpin, double zSpin) 
{
double tx = point->v.x * cos (ySpin) - point->v.z * sin (ySpin);
double ty = point->v.y;
double tz = point->v.x * sin (ySpin) + point->v.z * cos (ySpin);
point->Set (tx * cos (zSpin) + ty * sin (zSpin), ty * cos (zSpin) - tx * sin (zSpin), tz);
}

//------------------------------------------------------------------------------
// SpinBackPoint () - spin on z-axis then y-axis
//------------------------------------------------------------------------------

void CTunnel::SpinBackPoint (CVertex* point, double ySpin, double zSpin) 
{
double tx = point->v.x * cos (-zSpin) + point->v.y * sin (-zSpin);
double ty = -point->v.x * sin (-zSpin) + point->v.y * cos (-zSpin);
double tz = point->v.z;
point->Set (tx * cos (-ySpin) - tz * sin (-ySpin), ty, tx * sin (-ySpin) + tz * cos (-ySpin));
}

//------------------------------------------------------------------------------
// MatchingSide ()
//
// Action - Returns matching side depending on the current m_bezierPoints
//------------------------------------------------------------------------------

int CTunnel::MatchingSide (int j) 
{
  static int ret [4][4] = {{3,2,1,0},{2,1,0,3},{1,0,3,2},{0,3,2,1}};
  int offset;

offset = (4 + selections [0].Point () - selections [1].Point ()) % 4;
return ret [offset][j];
}

//------------------------------------------------------------------------------

void CTunnel::PolarPoints (double* angle, double* radius, CVertex* vertex, CVertex* origin, CVertex* normal) 
{
// translate coordinates to origin
#if 0
double vx = vertex->v.x - origin->v.x;
double vy = vertex->v.y - origin->v.y;
double vz = vertex->v.z - origin->v.z;
double nx = normal->v.x - origin->v.x;
double ny = normal->v.y - origin->v.y;
double nz = normal->v.z - origin->v.z;
#endif
CDoubleVector v = *vertex - *origin;
CDoubleVector n = *normal - *origin;
// calculate angles to normalize direction
// spin on z axis to get into the x-z plane
double zSpin = atan3 (n.v.y, n.v.x);
double zSpinSin = sin (zSpin);
double zSpinCos = cos (zSpin);
double x1 = n.v.x * zSpinCos + n.v.y * zSpinSin;
double z1 = n.v.z;
// spin on y to get on the x axis
double ySpin = -atan3 (z1, x1);
// spin vertex (spin on z then y)
x1 = v.v.x * zSpinCos + v.v.y * zSpinSin;
double y1 = -v.v.x * zSpinSin + v.v.y * zSpinCos;
z1 = v.v.z;
double y2 = y1;
double z2 = x1 * sin (ySpin) + z1 * cos (ySpin);
// convert to polar
*radius = sqrt (y2 * y2 + z2 * z2);  // ignore any x offset
*angle = atan3 (z2, y2);
}

//------------------------------------------------------------------------------
// Action - Spins m_bezierPoints which lie in the y-z plane orthagonal to a normal
//          Uses normal as center for translating m_bezierPoints.
//
// Changes - Chooses axis to normalizes on m_based on "normal" direction
//------------------------------------------------------------------------------

CDoubleVector CTunnel::RectPoints (double angle, double radius, CVertex* origin, CVertex* normal) 
{
  double			ySpin, zSpin;
  char			spinAxis;
  CVertex		v, n = *normal - *origin, v1, v2;

  // translate coordanites to orgin
  // choose rotation order
if (fabs(n.v.z) > fabs(n.v.y))
	spinAxis = 'Y';
else 
	spinAxis = 'Z';
spinAxis = 'Y';

// start by defining vertex in rectangular coordinates (xz plane)
v.Set (0, radius * cos (angle), radius * sin (angle));

switch(spinAxis) {
 case 'Y':
   // calculate angles to normalize direction
   // spin on y axis to get into the y-z plane
   ySpin = -atan3 (n.v.z, n.v.x);
   v1.Set (n.v.x * cos (ySpin) - n.v.z * sin (ySpin), n.v.y, n.v.x * sin (ySpin) + n.v.z * cos (ySpin));
   // spin on z to get on the x axis
   zSpin = atan3 (v1.v.y,v1.v.x);
   // spin vertex back in negative direction (z first then y)
	zSpin = -zSpin;
   v2.Set ((double) v.v.x * cos (zSpin) + (double) v.v.y * sin (zSpin), (double) -v.v.x * sin (zSpin) + (double) v.v.y * cos (zSpin), double (v.v.z));
	ySpin = -ySpin;
   v1.Set (v2.v.x * cos (ySpin) - v2.v.z * sin (ySpin), v2.v.y, v2.v.x * sin (ySpin) + v2.v.z * cos (ySpin));
   break;

 case 'Z':
   // calculate angles to normalize direction
   // spin on z axis to get into the x-z plane
   zSpin = atan3 (n.v.y, n.v.x);
   v1.Set (n.v.x * cos (zSpin) + n.v.y * sin (zSpin), -n.v.x * sin (zSpin) + n.v.y * cos (zSpin), n.v.z);
   // spin on y to get on the x axis
   ySpin = -atan3 (v1.v.z,v1.v.x);
   // spin vertex back in negative direction (y first then z)
	ySpin = -ySpin;
   v2.Set ((double) v.v.x * cos (ySpin) - (double) v.v.z * sin (ySpin), double (v.v.y), (double) v.v.x * sin (ySpin) + (double) v.v.z * cos (ySpin));
	zSpin = -zSpin;
   v1.Set (v2.v.x * cos (zSpin) + v2.v.y * sin (zSpin), -v2.v.x * sin (zSpin) + v2.v.y * cos (zSpin), v2.v.z);
   break;
	}
v = *normal + v1;
return v;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

long CCubicBezier::Faculty (int n) 
{
long i = 1;

for (int j = n; j >= 2; j--) 
	i *= j;
return i;
}

//------------------------------------------------------------------------------
//   Coeff(n,i) - returns n!/(i!*(n-i)!)
//------------------------------------------------------------------------------

double CCubicBezier::Coeff (int n, int i) 
{
return ((double) Faculty (n) / ((double) Faculty (i) * (double) Faculty (n-i)));
}

//------------------------------------------------------------------------------
//   Blend(i,n,u) - returns a weighted coefficient for each point in a nSegment
//------------------------------------------------------------------------------

double CCubicBezier::Blend (int i, int n, double u) 
{
double partial = Coeff (n, i) * pow (u, i) * pow (1 - u, n - i);
return partial;
}

//------------------------------------------------------------------------------
//   BezierFcn(pt,u,n,p [][]) - sets (x,y,z) for u=#/segs based on m_bezierPoints p
//------------------------------------------------------------------------------

CDoubleVector CCubicBezier::Compute (double u) 
{
	CDoubleVector v;

for (int i = 0; i < 4; i++) {
	double b = Blend (i, 3, u);
	v += m_points [i] * b;
	}
return v;
}

//------------------------------------------------------------------------------

void CCubicBezier::Transform (CViewMatrix* viewMatrix)
{
for (int i = 0; i < 4; i++)
	m_points [i].Transform (viewMatrix);
}

//------------------------------------------------------------------------------

void CCubicBezier::Project (CViewMatrix* viewMatrix)
{
for (int i = 0; i < 4; i++)
	m_points [i].Project (viewMatrix);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTunnelBase::Setup (double sign)
{
m_sign = sign;
CSegment* segP = segmentManager.Segment (m_nSegment);
CSide* sideP = segP->Side (m_nSide);
segP->ComputeNormals (m_nSide);
m_normal = sideP->Normal () * sign;
segP->CreateOppVertexIndex (m_nSide, m_oppVertexIndex);
m_point = segP->ComputeCenter (m_nSide);
for (int i = 0; i < 4; i++)
	m_vertices [i] = *Segment ()->Vertex (m_nSide, i);
m_rotation.m.fVec = m_normal;
m_rotation.m.rVec = m_vertices [(sideP->m_nPoint + 1) % sideP->VertexCount ()] - m_vertices [sideP->m_nPoint];
m_rotation.m.rVec.Normalize ();
m_rotation.m.rVec *= -sign;
m_rotation.m.uVec = CrossProduct (m_rotation.m.fVec, m_rotation.m.rVec);
m_rotation.m.uVec.Normalize ();
}

//------------------------------------------------------------------------------

bool CTunnelBase::Update (void)
{
	CSelection* selection;

if (*((CSideKey*) this) == *((CSideKey*) current))
	selection = current;
else if (*((CSideKey*) this) == *((CSideKey*) other))
	selection = other;
else
	return true;
if (Edge () != selection->Edge ())
	return true;
for (int i = 0; i < 4; i++)
	if (m_vertices [i] != *selection->Vertex (i))
		return true;
return false;
}

//------------------------------------------------------------------------------
// create a tunnel segment (tunnel 'slice') and correlate each element's vertices
// with its corresponding vertices

bool CTunnelSegment::Create (CTunnelPath& path, short nSegments, short nVertices)
{
if (!m_elements.Resize (nSegments))
	return false;
if (!nVertices) 
	path.m_nStartVertices.ShareBuffer (m_nVertices);
else if (!(m_nVertices.Resize (nVertices) && vertexManager.Add (&m_nVertices [0], nVertices))) 
	return false;

for (short i = 0; i < nSegments; i++) {
	CTunnelElement& element = m_elements [i];
	short* vertexIndex = path.m_startSides [i].m_nVertexIndex;
	for (short j = 0; j < 4; j++) 
		element.m_nVertices [j] = m_nVertices [vertexIndex [j]];
	}

if (nVertices == 0)
	return true;

for (short i = 0; i < nSegments; i++) {
	CTunnelElement& element = m_elements [i];
	if (0 > (element.m_nSegment = segmentManager.Add ()))
		return false;
	segmentManager.Segment (element.m_nSegment)->m_info.bTunnel = 1;
	}
return true;
}

//------------------------------------------------------------------------------

void CTunnelSegment::Release (void)
{
if (!(m_nVertices.Buffer () && (m_elements.Buffer ())))
	return;
for (int i = (int) m_elements.Length (); --i >= 0; ) 
	segmentManager.Remove (m_elements [i].m_nSegment);
for (int i = (int) m_nVertices.Length (); --i >= 0; ) 
	vertexManager.Delete (m_nVertices [i]);
}

//------------------------------------------------------------------------------

void CTunnelSegment::Draw (void)
{
CMineView* mineView = DLE.MineView ();
for (int i = (int) m_elements.Length (); --i >= 0; ) 
	mineView->DrawSegmentWireFrame (segmentManager.Segment (m_elements [i].m_nSegment), false, false, 1);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTunnel::Setup (CTunnelBase base [2])
{
memcpy (m_base, base, sizeof (m_base));
m_nSteps = 0;
}

//------------------------------------------------------------------------------

void CTunnel::Release (void)
{
for (int i = m_nSteps; --i >= 0; )
	m_segments [i].Release ();
}

//------------------------------------------------------------------------------

void CTunnel::Destroy (void)
{
Release ();
m_nSteps = 0;
m_segments.Destroy ();
}

//------------------------------------------------------------------------------

void CTunnel::UntwistSegment (short nSegment, short nSide) 
{
  double		minLen = 1e10;
  short		verts [4];
  short		nOppSide = oppSideTable [nSide];
  ubyte		oppVertexIndex [4];

CSegment* segP = segmentManager.Segment (nSegment);
segP->CreateOppVertexIndex (nSide, oppVertexIndex);
// calculate length from point 0 to opp_points
CVertex* vert0 = segP->Vertex (nSide, 0);
short index = 0;
for (short j = 0; j < 4; j++) {
	double len = Distance (*vert0, *segP->Vertex (nOppSide, oppVertexIndex [j]));
	if (len < minLen) {
		minLen = len;
		index = j;
		}
	}
// swap verts if index != 0
if (index != 0) {
	for (short j = 0; j < 4; j++)
		verts [j] = segP->VertexId (oppVertexIndex [(j + index) % 4]);
	for (short j = 0; j < 4; j++)
		segP->SetVertexId (oppVertexIndex [j], verts [j]);
	}
}

//------------------------------------------------------------------------------
// define segment vert numbers

void CTunnel::SetupVertices (void)
{
for (uint nElement = 0, nElements = m_segments [0].m_elements.Length (); nElement < nElements; nElement++) {
	CTunnelElement * e0, * e1 = &m_segments [0].m_elements [nElement];
	for (short nSegment = 1; nSegment <= m_nSteps; nSegment++) {
		e0 = e1;
		e1 = &m_segments [nSegment].m_elements [nElement];
		CSegment* segP = segmentManager.Segment (e1->m_nSegment);
		for (short nVertex = 0; nVertex < 4; nVertex++) {
#if 0
			if (nSegment == 0) { // 1st segment
				segP->SetVertexId (m_base [0].m_nSide, nVertex, e1->m_nVertices [nVertex]);
				segP->SetVertexId (m_base [0].m_oppVertexIndex [nVertex], m_base [0].Segment ()->VertexId (m_base [0].m_nSide, nElement));
				}
			else if (nSegment == m_nSteps - 1) { // last segment
				segP->SetVertexId (m_base [0].m_nSide, nVertex, m_base [1].Segment ()->VertexId (m_base [1].m_nSide, m_base [1].m_oppVertexIndex [nVertex])); 
				segP->SetVertexId (m_base [0].m_oppVertexIndex [nVertex], e0->m_nVertices [nVertex]);
				}
			else 
#endif
				{
				segP->SetVertexId (m_base [0].m_nSide, nVertex, e1->m_nVertices [nVertex]);
				segP->SetVertexId (m_base [0].m_oppVertexIndex [nVertex], e0->m_nVertices [nVertex]);
				}
			}
		}
	}
// twisted segments
//for (short i = 0; i < m_nSteps; i++)
//	UntwistSegment (m_segments [i].m_nSegment, m_base [0].m_nSide);
}

//------------------------------------------------------------------------------

bool CTunnel::Create (CTunnelPath& path) 
{
	short nSegments = (short) path.m_startSides.Length ();
	short nVertices = (short) path.m_nStartVertices.Length ();

if (m_nSteps != path.Steps ()) { // recompute
	if (m_nSteps > 0)
		Release ();
	if ((path.Steps () > m_nSteps) && !m_segments.Resize (path.Steps () + 1, false))
		return false;
	m_nSteps = path.Steps ();
	for (int i = 0; i <= m_nSteps; i++) {
		if (!m_segments [i].Create (path, nSegments, nVertices))
			return false;
		}
	}

#if 1

for (int i = 0; i <= m_nSteps; i++) {
	CDoubleMatrix& rotation = path [i].m_rotation;
	CDoubleVector& translation = path [i].m_vertex;
	for (uint j = 0, l = path.m_nStartVertices.Length (); j < l; j++) {
		CVertex v = vertexManager [path.m_nStartVertices [j]];
		v -= path.m_base [0].m_point;
		v = path.m_base [0].m_rotation * v;
		v = rotation * v;
		CDoubleVector a = rotation.Angles ();
		v += translation;
		vertexManager [m_segments [i].m_nVertices [j]] = v;
#ifdef _DEBUG
		v = v;
#endif
		}
	}

#else

CVertex relSidePoints [2][4]; // side points relative to center of side 1
CVertex relBezierPoints [4]; // 4 points of segment relative to 1st point
CDoubleVector t = path.Bezier ().GetPoint (0); // translation

for (i = 0; i < 2; i++) {
	segP = segmentManager.Segment (m_base [i]);
	for (j = 0; j < 4; j++) {
		relSidePoints [i][j] = *segP->Vertex (m_base [i].m_nSide, j) - t;
		}
	}

double	theta [2][4], radius [2][4]; // polor coordinates of sides
double	deltaAngle [4];
double	y, z;
double	ySpin, zSpin;

// make all points relative to first face (translation)
for (i = 0; i < 4; i++) 
	relBezierPoints [i] = path.Bezier ().GetPoint (i) - path.Bezier ().GetPoint (0);

// determine y-spin and z-spin to put 1st orthogonal vector onto the x-axis
ySpin = -atan3 (relBezierPoints [1].v.z, relBezierPoints [1].v.x); // to y-z plane
zSpin = atan3 (relBezierPoints [1].v.y, relBezierPoints [1].v.x * cos (ySpin) - relBezierPoints [1].v.z * sin (ySpin)); // to x axis

// spin all m_bezierPoints relative to first face (rotation)
for (i = 0; i < 4; i++) {
	SpinPoint (relBezierPoints + i, ySpin, zSpin);
	for (j = 0; j < 2; j++) 
		SpinPoint (relSidePoints [j] + i, ySpin, zSpin);
	}

for (i = 0; i < m_nSteps; i++) 
	SpinPoint (&path [i], ySpin, zSpin);

// determine polar coordinates of the 1st side (simply y,z coords)
for (i = 0; i < 4; i++) {
	theta [0][i] = atan3 (relSidePoints [0][i].v.z, relSidePoints [0][i].v.y);
	y = relSidePoints [0][i].v.y;
	z = relSidePoints [0][i].v.z;
	radius [0][i] =_hypot (y, z);
	}

// determine polar coordinates of the 2nd side by rotating to x-axis first
for (i = 0; i < 4; i++) {
	// flip orthogonal vector to point into segment
	vertex = (relBezierPoints [3] * 2) - relBezierPoints [2];
	PolarPoints (&theta [1][i], &radius [1][i], &relSidePoints [1][i], &relBezierPoints [3], &vertex);
	}

// figure out the angle differences to be in range (-pi to pi)
for (j = 0; j < 4; j++) {
	deltaAngle [j] = theta [1][MatchingSide (j)] - theta [0][j];
	if (deltaAngle [j] < M_PI) 
		deltaAngle [j] += 2 * M_PI;
	if (deltaAngle [j] > M_PI) 
		deltaAngle [j] -= 2 * M_PI;
	}

// make sure delta angles do not cross PI & -PI
for (i = 1; i < 4; i++) {
	if (deltaAngle [i] > deltaAngle [0] + M_PI) 
		deltaAngle [i] -= 2 * M_PI;
	if (deltaAngle [i] < deltaAngle [0] - M_PI) 
		deltaAngle [i] += 2 * M_PI;
	}

// calculate segment vertices as weighted average between the two sides
// then spin vertices in the direction of the segment vector
ushort nVertex = 0;
for (i = 0; i < m_nSteps - 1; i++) {
	for (j = 0; j < 4; j++) {
		CVertex* vertP = vertexManager.Vertex (m_segments [i].m_nVertices [j]);
		double h = (double) i / (double) m_nSteps;
		double angle  = h * deltaAngle [j] + theta [0][j];
		double length = h * radius [1][MatchingSide (j)] + (((double) m_nSteps - (double) i) / (double) m_nSteps) * radius [0][j];
		*vertP = RectPoints (angle, length, &path [i], &path [i + 1]);
		// spin vertices
		SpinBackPoint (vertP, ySpin, zSpin);
		// translate point back
		*vertP += path.Bezier ().GetPoint (0);
		}
	}

#endif
SetupVertices ();
return true;
}

//------------------------------------------------------------------------------

void CTunnel::Realize (void)
{
ushort nVertex = 0;
for (short i = 0; i < m_nSteps; i++) {
	CSegment* segP = segmentManager.Segment (m_segments [i].m_elements [0].m_nSegment);
	// copy current segment
	*segP = *segmentManager.Segment (current->SegmentId ());
	for (int j = 0; j < 6; j++)
		segP->SetChild (j, -1);
	segP->m_info.bTunnel = 0;
	if (i == 0) {
		segP->SetChild (oppSideTable [m_base [0].m_nSide], m_base [0].m_nSegment);
		segP->SetChild (m_base [0].m_nSide, m_segments [1].m_elements [0].m_nSegment);
		m_base [0].Segment ()->SetChild (m_base [0].m_nSide, m_segments [0].m_elements [0].m_nSegment);
		} 
	else if (i == m_nSteps - 1) {
		segP->SetChild (oppSideTable [m_base [0].m_nSide], m_segments [i - 1].m_elements [0].m_nSegment); // previous tunnel segment
		segP->SetChild (m_base [0].m_nSide, m_base [1].m_nSegment);
		m_base [1].Segment ()->SetChild (m_base [1].m_nSide, m_segments [i].m_elements [0].m_nSegment);
		}
	else  {
		segP->SetChild (oppSideTable [m_base [0].m_nSide], m_segments [i - 1].m_elements [0].m_nSegment); // previous tunnel segment
		segP->SetChild (m_base [0].m_nSide, m_segments [i + 1].m_elements [0].m_nSegment); // next tunnel segment
		}
	// define child bitmask, special, matcen, value, and wall bitmask
	}
SetupVertices ();
}

//------------------------------------------------------------------------------

void CTunnel::Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* viewMatrix) 
{
CDC* pDC = renderer.DC ();

renderer.BeginRender ();
for (int i = 0; i <= m_nSteps; i++) {
	for (int j = 0; j < 4; j++) {
		CVertex&v = vertexManager [m_segments [i].m_nVertices [j]];
		v.Transform (viewMatrix);
		v.Project (viewMatrix);
		}
	}
renderer.EndRender ();

renderer.BeginRender (true);
renderer.SelectObject ((HBRUSH)GetStockObject (NULL_BRUSH));
renderer.SelectPen (penBlue + 1);
CMineView* mineView = DLE.MineView ();
for (int i = 1; i <= m_nSteps; i++)
	m_segments [i].Draw ();
renderer.EndRender ();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Derive a rotation matrix from a source rotation matrix, a z rotation angle and a forward vector.
// First construct arbitrary right and up vectors from the forward vector, then rotate up and right
// around forward by the given angle.

void CTunnelPathNode::CreateRotation (CVertex fVec, CDoubleMatrix& mOrigin, double angle)
{
	CDoubleMatrix	m;
	
m.m.fVec = fVec;
m.m.fVec.Normalize ();
m.m.rVec = CrossProduct (mOrigin.m.uVec, m.m.fVec);
double l = m.m.rVec.Mag (); 
if (l >= 0.1) {
	m.m.rVec /= l;
	m.m.uVec = CrossProduct (m.m.rVec, m.m.fVec);
	m.m.rVec = CrossProduct (m.m.uVec, m.m.fVec);
	}
else {
	m.m.uVec = CrossProduct (mOrigin.m.rVec, m.m.fVec);
	m.m.uVec.Normalize ();
	m.m.rVec = CrossProduct (m.m.uVec, m.m.fVec);
	m.m.uVec = CrossProduct (m.m.rVec, m.m.fVec);
	}
#if 1
if (m.m.fVec != CrossProduct (m.m.rVec, m.m.uVec))
	m.m.rVec.Negate ();
#endif
#if 1
angle -= ClampAngle (m.Angles ().v.z - mOrigin.Angles ().v.z);
#if 0
while (angle < -PI)
	angle += PI;
while (angle > PI)
	angle -= PI;
#endif
#else
CVertex v1 = CrossProduct (m.m.fVec, v0);
double a = Dot (v0, mOrigin.m.rVec) + Dot (v1, mOrigin.m.uVec); 
double b = Dot (v1, mOrigin.m.rVec) - Dot (v0, mOrigin.m.uVec); 
double q = sqrt (a * a + b * b); 
double r, s;
if (q >= 0.001) { 
	r = a / q;  
	s = b / q;  
	}
else {  
	r = 1.0;  
	s = 0.0;  
	}
m.m.rVec = v0 * r + v1 * s;
m.m.uVec = v1 * r - v0 * s;
#endif
// rotate right and up vector around forward vector
#if 1
CDoubleMatrix zr (cos (angle), -sin (angle), 0.0, sin (angle), cos (angle), 0.0, 0.0, 0.0, 1.0);
#endif
m_rotation = m.Mul (zr).Inverse ();
//m_rotation.m.rVec.Rotate (m_rotation.m.fVec, angle);
//m_rotation.m.uVec.Rotate (m_rotation.m.fVec, angle);
//m_rotation = m_rotation.Inverse ();
}
 
//------------------------------------------------------------------------------

void CTunnelPathNode::Draw (CRenderer& renderer, CViewMatrix* viewMatrix) 
{
CDC* pDC = renderer.DC ();

CDoubleMatrix m;
m = m_rotation.Inverse ();
CVertex v [3] = { m.m.rVec, m.m.uVec, m.m.fVec };

renderer.BeginRender ();
m_vertex.Transform (viewMatrix);
m_vertex.Project (viewMatrix);
for (int i = 0; i < 3; i++) {
	v [i] *= 5.0;
	v [i] += m_vertex;
	v [i].Transform (viewMatrix);
	v [i].Project (viewMatrix);
	}
renderer.EndRender ();

renderer.BeginRender (true);
renderer.SelectObject ((HBRUSH)GetStockObject (NULL_BRUSH));
static ePenColor pens [3] = { penOrange, penMedGreen, penMedBlue };

renderer.Ellipse (m_vertex, 4, 4);
for (int i = 0; i < 3; i++) {
	renderer.SelectPen (pens [i] + 1);
	renderer.MoveTo (m_vertex.m_screen.x, m_vertex.m_screen.y);
	renderer.LineTo (v [i].m_screen.x, v [i].m_screen.y);
	}
renderer.EndRender ();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

bool CTunnelPath::Setup (CTunnelBase base [2])
{
memcpy (m_base, base, sizeof (m_base));
double length = Distance (m_base [0].m_point, m_base [1].m_point);
if (length < 50.0)
	return false;
length *= 0.5;
if (length < MIN_TUNNEL_LENGTH)
	length = MIN_TUNNEL_LENGTH;
else if (length > MAX_TUNNEL_LENGTH)
	length = MAX_TUNNEL_LENGTH;

CDoubleMatrix identity;

// collect all tagged sides that don't have child segments, are directly or indirectly connected to the start side and are at an angle of <= 22.5° to the start side
CTagTunnelStart tagger;
bool bTagged = current->Side ()->IsTagged ();
if (!bTagged)
	current->Side ()->Tag ();
int nSides = tagger.Setup (segmentManager.TaggedSideCount (), TUNNEL_MASK);
tagger.Run ();
if (!bTagged)
	current->Side ()->UnTag ();

// copy the collected sides to an array
// gather all vertices of the start sides
// create indices into the start vertex array for every start side's corner
if (!(m_startSides.Create (nSides)))
	return false;

CSLL<ushort,ushort>	startVertices;

for (int nStartSide = 0; nStartSide < nSides; nStartSide++) {
	m_startSides [nStartSide] = tagger.m_sideList [nStartSide].m_child;
	short nSide = m_startSides [nStartSide].m_nSide;
	CSegment* segP = segmentManager.Segment (m_startSides [nStartSide]);
	for (int nVertex = 0, nVertexCount = segP->Side (nSide)->VertexCount (); nVertex < nVertexCount; nVertex++) {
		ushort nId = segP->VertexId (nSide, nVertex);
		int nIndex = startVertices.Index (nId);
		if (nIndex < 0) {
			if (!startVertices.Append (nId))
				return false; // out of memory
			nIndex = startVertices.Length () - 1;
			}
		m_startSides [nStartSide].m_nVertexIndex [nVertex] = nIndex;
		}
	}

if (!(m_nStartVertices.Create (startVertices.Length ())))
	return false;

// copy the start vertices to an array
CSLLIterator<ushort, ushort> iter (startVertices);
ushort j = 0;
for (iter.Begin (); *iter != iter.End (); iter++)
	m_nStartVertices [j++] = **iter;

// setup intermediate points for a cubic bezier curve
m_bezier.SetLength (length, 0);
m_bezier.SetLength (length, 1);
m_bezier.SetPoint (m_base [0].GetPoint (), 0);
m_bezier.SetPoint (m_base [0].GetPoint () + m_base [0].GetNormal () * m_bezier.GetLength (0), 1);
m_bezier.SetPoint (m_base [1].GetPoint () + m_base [1].GetNormal () * m_bezier.GetLength (1), 2);
m_bezier.SetPoint (m_base [1].GetPoint (), 3);
return true;
}

//------------------------------------------------------------------------------

void CTunnelPath::Destroy (void)
{
m_startSides.Destroy ();
m_nStartVertices.Destroy ();
}

//------------------------------------------------------------------------------

bool CTunnelPath::Create (short nSteps)
{
if (m_nSteps != nSteps) { // recompute
	if ((nSteps > m_nSteps) && !m_nodes.Resize (nSteps + 1, false))
		return false;
	m_nSteps = nSteps;
	}

// calculate nSegment m_bezierPoints
for (int i = 0; i <= m_nSteps; i++) 
	m_nodes [i].m_vertex = m_bezier.Compute ((double) i / (double) m_nSteps);
CDoubleVector t = m_nodes [0].m_vertex;

double l = Length ();
m_nodes [0].m_rotation = m_base [0].m_rotation.Inverse ();
m_nodes [m_nSteps].m_rotation = m_base [1].m_rotation.Inverse ();

m_deltaAngle = ClampAngle (m_base [1].m_rotation.Angles ().v.z - m_base [0].m_rotation.Angles ().v.z);
#if 0
while (m_deltaAngle < -PI * 1.5)
	m_deltaAngle += PI * 0.5;
while (m_deltaAngle > PI * 1.5)
	m_deltaAngle -= PI * 0.5;
#endif
for (int i = 1; i <= m_nSteps; i++) 
	m_nodes [i].CreateRotation ((i == 0) ? m_base [0].m_normal : (i == m_nSteps) ? m_base [1].m_normal : m_nodes [i + 1].m_vertex - m_nodes [i - 1].m_vertex, 
											 m_nodes [0].m_rotation, m_deltaAngle * Length (i) / l);
return true;
}

//------------------------------------------------------------------------------

double CTunnelPath::Length (int nSteps)
{
	double length = 0.0;

if (nSteps < 0)
	nSteps = m_nSteps;
for (int i = 1; i <= nSteps; i++) 
	length += Distance (m_nodes [i].m_vertex, m_nodes [i - 1].m_vertex);
return length;
}

//------------------------------------------------------------------------------

void CTunnelPath::Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* viewMatrix) 
{
#ifdef _DEBUG

for (int i = 0; i <= m_nSteps; i++) 
	m_nodes [i].Draw (renderer, viewMatrix);

#else

CDC* pDC = renderer.DC ();

renderer.BeginRender ();
for (int i = 0; i < 4; i++) {
	Bezier ().Transform (viewMatrix);
	Bezier ().Project (viewMatrix);
	}
renderer.EndRender ();

renderer.BeginRender (true);
renderer.SelectObject ((HBRUSH)GetStockObject (NULL_BRUSH));
renderer.SelectPen (penRed + 1);


CMineView* mineView = DLE.MineView ();
if (Bezier ().GetPoint (1).InRange (mineView->ViewMax ().x, mineView->ViewMax ().y, renderer.Type ())) {
	if (Bezier ().GetPoint (0).InRange (mineView->ViewMax ().x, mineView->ViewMax ().y, renderer.Type ())) {
		renderer.MoveTo (Bezier ().GetPoint (0).m_screen.x, Bezier ().GetPoint (0).m_screen.y);
		renderer.LineTo (Bezier ().GetPoint (1).m_screen.x, Bezier ().GetPoint (1).m_screen.y);
		renderer.Ellipse (Bezier ().GetPoint (1), 4, 4);
		}
	}
if (Bezier ().GetPoint (2).InRange (mineView->ViewMax ().x, mineView->ViewMax ().y, renderer.Type ())) {
	if (Bezier ().GetPoint (3).InRange (mineView->ViewMax ().x, mineView->ViewMax ().y, renderer.Type ())) {
		renderer.MoveTo (Bezier ().GetPoint (3).m_screen.x, Bezier ().GetPoint (3).m_screen.y);
		renderer.LineTo (Bezier ().GetPoint (2).m_screen.x, Bezier ().GetPoint (2).m_screen.y);
		renderer.Ellipse (Bezier ().GetPoint (2), 4, 4);
		}
	}
renderer.EndRender ();

#endif
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTunnelMaker::Reset (void)
{
if (m_bActive) {
	m_tunnel.Destroy ();
	m_bActive = false;
	}
DLE.MineView ()->Refresh (false);
}

//------------------------------------------------------------------------------

void CTunnelMaker::Destroy (void) 
{
m_bActive = false;
m_tunnel.Destroy ();
m_path.Destroy ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Run (void) 
{
if (!m_bActive) {
	short nMaxSegments = SEGMENT_LIMIT - segmentManager.Count ();
	if (nMaxSegments > MAX_TUNNEL_SEGMENTS)
		nMaxSegments = MAX_TUNNEL_SEGMENTS;
	else if (nMaxSegments < 3) {
		ErrorMsg ("Insufficient number of free vertices and/or segments\nto use the tunnel generator.");
		return;
		}
	// make sure there are no children on either segment/side
	other = &selections [!current->Index ()];
	if ((current->Segment ()->ChildId (current->SideId ()) != -1) ||
		 (other->Segment ()->ChildId (other->m_nSide) != -1)) {
		ErrorMsg ("Starting and/or ending point of segment\n"
					 "already have segment(s) attached.\n\n"
					 "Hint: Put the current segment and the alternate segment\n"
					 "on sides which do not have cubes attached.");
		return;
		}

	if (!Setup ()) {
		m_bActive = false;
		return;
		}

	undoManager.Lock ();

	if (!DLE.ExpertMode ())
		ErrorMsg ("Place the current segment on one of the segment end points.\n\n"
					 "Use the CTRL+8 and CTRL+9 keys to adjust the length of the red segment.\n\n"
				    "Press 'P' to rotate the point connections.\n\n"
				    "Press 'G' or select Tools/Tunnel Generator when you are finished.");

	m_bActive = true;
	}
else {
	// ask if user wants to keep the new nSegment
	undoManager.Unlock ();
	if (Query2Msg ("Do you want to keep this tunnel?", MB_YESNO) != IDYES) 
		Destroy ();
	else {
		Reset ();
		undoManager.Begin (udSegments | udVertices);
		if (Create ())
			m_tunnel.Realize ();
		undoManager.End ();
		Destroy ();
		}
	}
segmentManager.SetLinesToDraw ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

bool CTunnelMaker::Setup (void)
{
dynamic_cast<CSideKey&> (m_base [0]) = dynamic_cast<CSideKey&> (*current);
dynamic_cast<CSideKey&> (m_base [1]) = dynamic_cast<CSideKey&> (*other);
m_base [0].Setup (-1.0);
m_base [1].Setup (1.0);
m_nGranularity = 0;

if (m_path.Setup (m_base)) {
	m_tunnel.Setup (m_base);
	return true;
	}
ErrorMsg ("End points of segment are too close.\n\n"
				"Hint: Select two sides which are further apart\n"
				"using the spacebar and left/right arrow keys,\n"
				"then try again.");
Destroy ();
return false;
}

//------------------------------------------------------------------------------

bool CTunnelMaker::Update (void) 
{ 
if (!m_bActive)
	return false;
if (current->Segment ()->HasChild (current->SideId ()) || other->Segment ()->HasChild (other->SideId ()))
	return true;
if (m_base [0].Update () || m_base [1].Update ())
	return Setup ();
return true;
}

//------------------------------------------------------------------------------

short CTunnelMaker::PathLength (void)
{
m_nSteps = short (m_path.Bezier ().Length () / 20.0 + Distance (m_base [0].GetPoint (), m_base [1].GetPoint ()) / 20.0) + m_nGranularity;
if (m_nSteps > MaxSegments () - 1)
	m_nSteps = MaxSegments () - 1;
else if (m_nSteps < 3)
	m_nSteps = 3;
return m_nSteps;
}

//------------------------------------------------------------------------------

bool CTunnelMaker::Create (void)
{
if (PathLength () <= 0) 
	return false;
if (m_path.Create (m_nSteps) && m_tunnel.Create (m_path))
	return true;
Destroy ();
return false;
}

//------------------------------------------------------------------------------

void CTunnelMaker::Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* viewMatrix)
{
if (Update () && Create ()) {
	m_path.Draw (renderer, redPen, bluePen, viewMatrix);
	m_tunnel.Draw (renderer, redPen, bluePen, viewMatrix);
	}
}

//------------------------------------------------------------------------------

void CTunnelMaker::Finer (void) 
{
++m_nGranularity;
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Coarser (void) 
{
--m_nGranularity;
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Stretch (void) 
{
if (current->SegmentId () == m_base [0].m_nSegment) {
	if (m_path.Bezier ().GetLength (0) > (MAX_TUNNEL_LENGTH - TUNNEL_INTERVAL))
		return;
	m_path.Bezier ().SetLength (m_path.Bezier ().GetLength (0) + TUNNEL_INTERVAL, 0);
	m_path.Bezier ().SetPoint (m_base [0].GetPoint () + m_base [0].GetNormal () * m_path.Bezier ().GetLength (0), 1);
	}
else if (current->SegmentId () == m_base [1].m_nSegment) {
	if (m_path.Bezier ().GetLength (1) > (MAX_TUNNEL_LENGTH - TUNNEL_INTERVAL))
		return;
	m_path.Bezier ().SetLength (m_path.Bezier ().GetLength (1) + TUNNEL_INTERVAL, 1);
	m_path.Bezier ().SetPoint (m_base [1].GetPoint () + m_base [1].GetNormal () * m_path.Bezier ().GetLength (1), 2);
	}
else
	return;
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Shrink (void) 
{
if (current->SegmentId () == m_base [0].m_nSegment) {
	if (m_path.Bezier ().GetLength (0) < (MIN_TUNNEL_LENGTH + TUNNEL_INTERVAL)) 
		return;
	m_path.Bezier ().SetLength (m_path.Bezier ().GetLength (0) - TUNNEL_INTERVAL, 0);
	m_path.Bezier ().SetPoint (m_base [0].GetPoint () + m_base [0].GetNormal () * m_path.Bezier ().GetLength (0), 1);
	}
else if (current->SegmentId () == m_base [1].m_nSegment) {
	if (m_path.Bezier ().GetLength (1) < (MIN_TUNNEL_LENGTH + TUNNEL_INTERVAL))
		return;
	m_path.Bezier ().SetLength (m_path.Bezier ().GetLength (1) - TUNNEL_INTERVAL, 1);
	m_path.Bezier ().SetPoint (m_base [1].GetPoint () + m_base [1].GetNormal () * m_path.Bezier ().GetLength (1), 2);
	}
else
	return;
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------
//eof TunnelMaker.cpp