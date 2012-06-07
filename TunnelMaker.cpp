

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
#if 0
while (angle < -PI)
	angle += 2.0 * PI;
while (angle > PI)
	angle -= 2.0 * PI;
#endif
return angle;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
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

void CTunnelBase::Setup (int nSelection, double sign)
{
if (nSelection >= 0) {
	if (m_nSelection < 0)
		m_nSelection = nSelection;
	*((CSideKey*) this) = selections [m_nSelection];
	}
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
//m_rotation.m.rVec *= -sign;
m_rotation.m.uVec = CrossProduct (m_rotation.m.fVec, m_rotation.m.rVec);
m_rotation.m.uVec.Normalize ();
}

//------------------------------------------------------------------------------
// Determine whether the tunnel needs to be updated
// For the start side, only update when the current edge or a vertex of the start side have changed
// For the end side, also update when the segment and/or side have changed

int CTunnelBase::Update (bool bStart)
{
	CSelection* selection;
	bool			bNewSide = CSideKey (*this) != CSideKey (selections [m_nSelection]);

if (!bStart && bNewSide) {
	*((CSelection*) this) = selections [m_nSelection];
	return m_bUpdate = -1;
	}
selection = &selections [m_nSelection];
if (!(bStart && bNewSide) && (Edge () != selection->Edge ())) {
	m_nEdge = selection->Edge ();
	return m_bUpdate = 1;
	}
CSegment* segP = segmentManager.Segment (m_nSegment);
for (int i = 0; i < 4; i++)
	if (m_vertices [i] != *segP->Vertex (m_nSide, i))
		return m_bUpdate = 1;
return m_bUpdate = 0;
}

//------------------------------------------------------------------------------
// create a tunnel segment (tunnel 'slice') and correlate each element's vertices
// with its corresponding vertices

bool CTunnelSegment::Create (CTunnelPath& path, short nSegments, short nVertices)
{
Release ();
if (!(m_elements.Resize (nSegments) && m_nVertices.Resize (nVertices)))
	return false;
if (!vertexManager.Add (&m_nVertices [0], nVertices))
	return false;
for (short i = 0; i < nSegments; i++) {
	CTunnelElement& element = m_elements [i];
	if (0 > (element.m_nSegment = segmentManager.Add ()))
		return false;
	segmentManager.Segment (element.m_nSegment)->m_info.bTunnel = 1;
	short* vertexIndex = path.m_startSides [i].m_nVertexIndex;
	for (short j = 0; j < 4; j++) 
		element.m_nVertices [j] = m_nVertices [vertexIndex [j]];
	}
return true;
}

//------------------------------------------------------------------------------

void CTunnelSegment::Release (void)
{
if (!(m_nVertices.Buffer () && (m_elements.Buffer ())))
	return;
for (int i = (int) m_elements.Length (); --i >= 0; ) {
	segmentManager.Remove (m_elements [i].m_nSegment);
	m_elements [i].m_nSegment = SEGMENT_LIMIT + 1;
	}
for (int i = (int) m_nVertices.Length (); --i >= 0; ) {
	vertexManager.Delete (m_nVertices [i]);
	m_nVertices [i] = MAX_VERTEX + 1;
	}
}

//------------------------------------------------------------------------------

void CTunnelSegment::Draw (void)
{
CMineView* mineView = DLE.MineView ();
mineView->Renderer ().BeginRender (false);
#ifdef _DEBUG
if (mineView->GetRenderer ()) {
	glLineStipple (1, 0x0c3f);  // dot dash
	glEnable (GL_LINE_STIPPLE);
	}
#endif
for (int i = (int) m_elements.Length (); --i >= 0; ) 
	mineView->DrawSegmentWireFrame (segmentManager.Segment (m_elements [i].m_nSegment), false, false, 1);
mineView->Renderer ().EndRender ();
#ifdef _DEBUG
if (mineView->GetRenderer ()) 
	glDisable (GL_LINE_STIPPLE);
#endif
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTunnel::Setup (CTunnelBase base [2])
{
Destroy ();
memcpy (m_base, base, sizeof (m_base));
}

//------------------------------------------------------------------------------

void CTunnel::Release (void)
{
if (m_segments.Buffer ())
	for (int i = m_nSteps; i >= 0; i--)
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
// Walk through all tunnel segments and assign the tunnel vertices to their 
// corresponding segments. Each segment's tunnel vertex ids have been computed
// when preparing the tunnel path and have been stored in CTunnelElement::m_nVertices.

void CTunnel::AssignVertices (void)
{
for (uint nElement = 0, nElements = m_segments [0].m_elements.Length (); nElement < nElements; nElement++) {
	CTunnelElement * e0, * e1 = &m_segments [0].m_elements [nElement];
	for (short nSegment = 1; nSegment <= m_nSteps; nSegment++) {
		e0 = e1;
		e1 = &m_segments [nSegment].m_elements [nElement];
		CSegment* segP = segmentManager.Segment (e1->m_nSegment);
		for (short nVertex = 0; nVertex < 4; nVertex++) {
			segP->SetVertexId (m_base [0].m_nSide, nVertex, e1->m_nVertices [nVertex]);
			segP->SetVertexId (m_base [0].m_oppVertexIndex [nVertex], e0->m_nVertices [nVertex]);
			}
		}
	}
}

//------------------------------------------------------------------------------

bool CTunnel::Create (CTunnelPath& path) 
{
	short nSegments = (short) path.m_startSides.Length ();
	short nVertices = (short) path.m_nStartVertices.Length ();

if (m_nSteps != path.Steps ()) { // allocate sufficient memory for required segments and vertices
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

// Compute all tunnel vertices by rotating the base vertices using each path node's orientation (== rotation matrix)
// The rotation is relative to the base coordinate system (identity matrix), but the vertices are relative to the 
// start point and start rotation, so each vertex has to be un-translated and un-rotated before rotating and translating
// it with the current path node's orientation matrix and position.
CMineView* mineView = DLE.MineView ();
CViewMatrix* viewMatrix = mineView->ViewMatrix ();
mineView->Renderer ().BeginRender (false);
for (int nSegment = 0; nSegment <= m_nSteps; nSegment++) {
	CDoubleMatrix& rotation = path [nSegment].m_rotation;
	CDoubleVector& translation = path [nSegment].m_vertex;
	for (uint nVertex = 0, l = path.m_nStartVertices.Length (); nVertex < l; nVertex++) {
		CVertex v = vertexManager [path.m_nStartVertices [nVertex]];
		v -= path.m_base [0].m_point; // un-translate (make relative to tunnel start)
		v = path.m_base [0].m_rotation * v; // un-rotate
		v = rotation * v; // rotate (
		CDoubleVector a = rotation.Angles ();
		v += translation;
		v.Transform (viewMatrix);
		v.Project (viewMatrix);
#ifdef _DEBUG
		v.Tag ();
#else
		v.UnTag ();
#endif
		vertexManager [m_segments [nSegment].m_nVertices [nVertex]] = v;
#ifdef _DEBUG
		v = v;
#endif
		}
	}
mineView->Renderer ().EndRender ();
AssignVertices ();
return true;
}

//------------------------------------------------------------------------------
// Connect all tunnel segments with all adjacent other tunnel segments and base segments.
// No segments will be connected with the tunnel's end segment to avoid geometry distortions.

void CTunnel::Realize (CTunnelPath& path)
{
ushort nVertex = 0;
short nElements = (short) m_segments [0].m_elements.Length ();
for (short nSegment = 0; nSegment < m_nSteps; nSegment++) {
	short nStartSeg = path.m_startSides [nSegment].m_nSegment;
	short nStartSide = m_base [0].m_nSide;
	CSegment* startSegP = segmentManager.Segment (nStartSeg);

	for (short iElement = 0; iElement < nElements; iElement++) {
		CTunnelElement& e0 = m_segments [nSegment].m_elements [iElement];
		CSegment* segP = segmentManager.Segment (e0.m_nSegment);
		*segP = *startSegP;
		for (int j = 0; j < 6; j++)
			segP->SetChild (j, -1);
		segP->m_info.bTunnel = 0;
		if (nSegment == 0) {
			segP->SetChild (oppSideTable [nStartSide], nStartSeg);
			segP->SetChild (nStartSide, m_segments [nSegment + 1].m_elements [iElement].m_nSegment);
			startSegP->SetChild (nStartSide, e0.m_nSegment);
			} 
#if 0
		else if (nSegment == m_nSteps - 1) {
			segP->SetChild (oppSideTable [m_base [0].m_nSide], m_segments [nSegment - 1].m_elements [0].m_nSegment); // previous tunnel segment
			segP->SetChild (m_base [0].m_nSide, m_base [1].m_nSegment);
			m_base [1].Segment ()->SetChild (m_base [1].m_nSide, e0.m_nSegment);
			}
#endif
		else {
			segP->SetChild (oppSideTable [nStartSide], m_segments [nSegment - 1].m_elements [iElement].m_nSegment); // previous tunnel segment
			segP->SetChild (nStartSide, m_segments [nSegment + 1].m_elements [iElement].m_nSegment); // next tunnel segment
			}
		for (short jElement = 0; jElement < nElements; jElement++) {
			if (jElement == iElement)
				continue;
			short nChildSeg = m_segments [nSegment].m_elements [jElement].m_nSegment;
			short nChildSide, nSide = segP->CommonSides (nChildSeg, nChildSide);
			if (nSide < 0)
				continue;
			segP->SetChild (nSide, nChildSeg);
			segmentManager.Segment (nChildSeg)->SetChild (e0.m_nSegment, nSide);
			}
		}
	}
AssignVertices ();
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

bool CTunnelPath::Setup (CTunnelBase base [2], bool bFull)
{
memcpy (m_base, base, sizeof (m_base));

double length = Distance (m_base [0].m_point, m_base [1].m_point);
if (length < 40.0)
	return false;
length *= 0.5;
if (length < MIN_TUNNEL_LENGTH)
	length = MIN_TUNNEL_LENGTH;
else if (length > MAX_TUNNEL_LENGTH)
	length = MAX_TUNNEL_LENGTH;

if (bFull) {
	CDoubleMatrix identity;

	// collect all tagged sides that don't have child segments, are directly or indirectly 
	// connected to the start side and are at an angle of <= 22.5° to the start side
	CTagTunnelStart tagger;
	bool bTagged = current->Side ()->IsTagged ();
	if (!bTagged)
		current->Side ()->Tag ();
	if (!tagger.Setup (segmentManager.TaggedSideCount (), TUNNEL_MASK))
		return false;
	int nSides = tagger.Run ();
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
	}
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
m_nodes [0].m_rotation = m_base [0].m_rotation;
m_nodes [m_nSteps].m_rotation = m_base [1].m_rotation;

m_deltaAngle = ClampAngle (m_base [1].m_rotation.Angles ().v.z - m_base [0].m_rotation.Angles ().v.z);
// Compute each path node's rotation matrix from the previous node's rotation matrix
// First rotate the r and u vectors by the difference angles of the preceding and the current nodes' rotation matrices' z axis
// To do that, compute the angle using the dot product and the rotation vector from the two z axii perpendicular vector
// and rotate using a quaternion
// Then rotate the r and u vectors around the z axis by the z angle difference
CTunnelPathNode * n0, * n1 = &m_nodes [0];
CQuaternion q;

for (int i = 1; i < m_nSteps; i++) {
	n0 = n1;
	n1 = &m_nodes [i];
	if (i < m_nSteps) // last matrix is the end side's matrix - use it's forward vector
		n1->m_rotation.m.fVec = m_nodes [i + 1].m_vertex - n1->m_vertex; //n0->m_vertex;

	// rotate the previous matrix around the perpendicular of the previous and the current forward vector
	// to orient it properly for the current path node
	double dot = Dot (n1->m_rotation.m.fVec.Normalize (), n0->m_rotation.m.fVec); // angle of current and previous forward vectors
	if (fabs (dot) > 1e-6) {
		CDoubleVector axis = CrossProduct (n1->m_rotation.m.fVec, -n0->m_rotation.m.fVec); // get rotation axis between the two forward vectors
		q.FromAxisAngle (axis, acos (dot));
		n1->m_rotation.m.rVec = q * n0->m_rotation.m.rVec; // rotate right and up vectors accordingly
		n1->m_rotation.m.uVec = q * n0->m_rotation.m.uVec;
		n1->m_rotation.m.rVec.Normalize ();
		n1->m_rotation.m.uVec.Normalize ();
		}
	// twist the current matrix around the forward vector 
	n1->m_angle = m_deltaAngle * Length (i) / l;
	q.FromAxisAngle (n1->m_rotation.m.fVec, n0->m_angle - n1->m_angle);
	n1->m_rotation.m.rVec = q * n1->m_rotation.m.rVec;
	n1->m_rotation.m.uVec = q * n1->m_rotation.m.uVec;
	n1->m_rotation.m.rVec.Normalize ();
	n1->m_rotation.m.uVec.Normalize ();

#if 0
	if (Dot (n1->m_rotation.m.rVec, n0->m_rotation.m.rVec) < 0.0) {
		n1->m_rotation.m.rVec.Negate ();
		n1->m_rotation.m.uVec.Negate ();
		}
#endif
	}
for (int i = 0; i <= m_nSteps; i++) 
	m_nodes [i].m_rotation = m_nodes [i].m_rotation.Inverse ();
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
	m_tunnel.Release ();
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

	if (!Setup (true)) {
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
			m_tunnel.Realize (m_path);
		undoManager.End ();
		Destroy ();
		}
	}
segmentManager.SetLinesToDraw ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

bool CTunnelMaker::Setup (bool bFull)
{
if (bFull) {
	m_base [0].m_nSelection = -1;
	m_base [0].Setup (current != &selections [0], -1.0);
	m_base [1].m_nSelection = -1;
	m_base [1].Setup (current == &selections [0], 1.0);
	}
else {
	m_base [0].Setup (-1, -1.0);
	if (m_base [1].m_bUpdate > 0) 
		m_base [1].Setup (-1, 1.0);
	else if (m_base [1].m_bUpdate < 0) {
		m_base [1].m_nSelection = -1;
		m_base [1].Setup (current != &selections [0], 1.0);
		}
	}
m_nGranularity = 0;

if (m_path.Setup (m_base, bFull)) {
	m_tunnel.Setup (m_base);
	return true;
	}
ErrorMsg ("End points of segments are too close.\n\n"
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
if (m_base [0].Update (true) > 0)
	return Setup (false);
int i = m_base [1].Update (false);
if (i)
	return Setup (i < 0);
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