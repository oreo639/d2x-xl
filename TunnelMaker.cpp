

#include "mine.h"
#include "dle-xp.h"

//------------------------------------------------------------------------------

CTunnelMaker tunnelMaker;

//------------------------------------------------------------------------------

#define CURRENT_POINT(a) ((current->Point () + (a))&0x03)

//------------------------------------------------------------------------------

char szTunnelMakerError [] = "You must exit tunnel creation before preforming this function";

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

void CTunnelBase::Setup (void)
{
if ((m_nSegment >= 0) && (m_nSide >= 0)) {
	m_point = segmentManager.CalcSideCenter (*this);
	m_normal = segmentManager.CalcSideNormal (*this);
	for (int i = 0; i < 4; i++)
		m_vertices [i] = *Segment ()->Vertex (m_nSide, i);
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTunnelSegment::Setup (CCubicBezier* bezier, CTunnelBase base [2])
{
m_bezier = bezier;
memcpy (m_base, base, sizeof (m_base));
m_nPathLength = 0;
}

//------------------------------------------------------------------------------

void CTunnelSegment::Remove (int i)
{
while (--i >= 0) {
	segmentManager.Remove (m_elements [i].m_nSegment);
	for (int j = 0; j < 4; j++)
		vertexManager.Delete (m_elements [i].m_nVertices [j]);
	}
}

//------------------------------------------------------------------------------

void CTunnelSegment::Destroy (void)
{
Remove (m_nPathLength);
m_nPathLength = 0;
m_elements.Destroy ();
}

//------------------------------------------------------------------------------
// SpinPoint () - spin on y-axis then z-axis
//------------------------------------------------------------------------------

void CTunnelSegment::SpinPoint (CVertex* point, double ySpin, double zSpin) 
{
double tx = point->v.x * cos (ySpin) - point->v.z * sin (ySpin);
double ty = point->v.y;
double tz = point->v.x * sin (ySpin) + point->v.z * cos (ySpin);
point->Set (tx * cos (zSpin) + ty * sin (zSpin), ty * cos (zSpin) - tx * sin (zSpin), tz);
}

//------------------------------------------------------------------------------
// SpinBackPoint () - spin on z-axis then y-axis
//------------------------------------------------------------------------------

void CTunnelSegment::SpinBackPoint (CVertex* point, double ySpin, double zSpin) 
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

int CTunnelSegment::MatchingSide (int j) 
{
  static int ret [4][4] = {{3,2,1,0},{2,1,0,3},{1,0,3,2},{0,3,2,1}};
  int offset;

offset = (4 + selections [0].Point () - selections [1].Point ()) % 4;
return ret [offset][j];
}

//------------------------------------------------------------------------------

void CTunnelSegment::UntwistSegment (short nSegment, short nSide) 
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

void CTunnelSegment::PolarPoints (double* angle, double* radius, CVertex* vertex, CVertex* origin, CVertex* normal) 
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

CDoubleVector CTunnelSegment::RectPoints (double angle, double radius, CVertex* origin, CVertex* normal) 
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
// define segment vert numbers

void CTunnelSegment::SetupVertices (void)
{
ubyte oppVertexIndex [4];
for (short i = 0; i < m_nPathLength; i++) {
	CSegment* segP = segmentManager.Segment (m_elements [i].m_nSegment);
	segP->CreateOppVertexIndex (m_base [0].m_nSide, oppVertexIndex);
	for (short j = 0; j < 4; j++) {
		if (i == 0) { // 1st segment
			segP->SetVertexId (m_base [0].m_nSide, j, m_elements [i].m_nVertices [j]);
			segP->SetVertexId (oppVertexIndex [j], m_base [0].Segment ()->VertexId (m_base [0].m_nSide, j));
			}
		else if (i == m_nPathLength - 1) { // last segment
			segP->SetVertexId (m_base [0].m_nSide, j, m_base [1].Segment ()->VertexId (m_base [1].m_nSide, MatchingSide (j)));
			segP->SetVertexId (oppVertexIndex [j], m_elements [i - 1].m_nVertices [j]);
			}
		else {
			segP->SetVertexId (m_base [0].m_nSide, j, m_elements [i].m_nVertices [j]);
			segP->SetVertexId  (oppVertexIndex [j], m_elements [i - 1].m_nVertices [j]);
			}
		}
	}
// twisted segments
for (short i = 0; i < m_nPathLength; i++)
	UntwistSegment (m_elements [i].m_nSegment, m_base [0].m_nSide);
}

//------------------------------------------------------------------------------

void CTunnelSegment::Compute (short nPathLength) 
{
  int			i, j;
  CSegment*	segP;
  CVertex	vertex;
  double		theta [2][4], radius [2][4]; // polor coordinates of sides
  double		deltaAngle [4];
  CVertex	relSidePoints [2][4]; // side points reletave to center of side 1
  CVertex	relPoints [4]; // 4 points of segment reletave to 1st point
  double		y, z;
  double		ySpin, zSpin;

if (m_nPathLength != nPathLength) { // recompute
	if (m_nPathLength > 0)
		Remove (m_nPathLength);
	if ((nPathLength > m_nPathLength) && !m_elements.Resize (nPathLength, false))
		return;
	m_nPathLength = nPathLength;
	for (i = 0; i < m_nPathLength; i++) {
		m_elements [i].m_nSegment = segmentManager.Add ();
		segmentManager.Segment (m_elements [i].m_nSegment)->m_info.bTunnel = 1;
		vertexManager.Add (&m_elements [i].m_nVertices [0], 4);
		}
	}

// calculate nSegment m_bezierPoints
for (i = 0; i <= m_nPathLength; i++) 
	m_elements [i].m_node = m_bezier->Compute ((double) i / (double) m_nPathLength);

// make all points relative to first face (translation)
for (i = 0; i < 4; i++) 
	relPoints [i] = m_bezier->GetPoint (i) - m_bezier->GetPoint (0);

for (i = 0; i < 2; i++) {
	segP = segmentManager.Segment (m_base [i]);
	for (j = 0; j < 4; j++) 
		relSidePoints [i][j] = *segP->Vertex (m_base [i].m_nSide, j) - m_bezier->GetPoint (0);
	}

for (i = 0; i < m_nPathLength; i++)
	m_elements [i].m_relNode = m_elements [i].m_node - m_bezier->GetPoint (0);

// determine y-spin and z-spin to put 1st orthogonal vector onto the x-axis
ySpin = -atan3 (relPoints [1].v.z, relPoints [1].v.x); // to y-z plane
zSpin = atan3 (relPoints [1].v.y, relPoints [1].v.x * cos (ySpin) - relPoints [1].v.z * sin (ySpin)); // to x axis

// spin all m_bezierPoints relative to first face (rotation)
for (i = 0; i < 4; i++) {
	SpinPoint (relPoints + i, ySpin, zSpin);
	for (j = 0; j < 2; j++) 
		SpinPoint (relSidePoints [j] + i, ySpin, zSpin);
	}

for (i = 0; i < m_nPathLength; i++) 
	SpinPoint (&m_elements [i].m_relNode, ySpin, zSpin);

// determine polar coordinates of the 1st side (simply y,z coords)
for (i = 0; i < 4; i++) {
	theta [0][i] = atan3 (relSidePoints [0][i].v.z, relSidePoints [0][i].v.y);
	y = relSidePoints [0][i].v.y;
	z = relSidePoints [0][i].v.z;
	radius [0][i] =_hypot (y, z);
	}

// determine polar coordinates of the 2nd side by rotating to x-axis first
for (i = 0; i < 4; i++) {
	// flip orthoginal vector to point into segment
	vertex = (relPoints [3] * 2) - relPoints [2];
	PolarPoints (&theta [1][i], &radius [1][i], &relSidePoints [1][i], &relPoints [3], &vertex);
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
for (i = 0; i < m_nPathLength - 1; i++) {
	for (j = 0; j < 4; j++) {
		CVertex* vertP = vertexManager.Vertex (m_elements [i].m_nVertices [j]);
		double h = (double) i / (double) m_nPathLength;
		double angle  = h * deltaAngle [j] + theta [0][j];
		double length = h * radius [1][MatchingSide (j)] + (((double) m_nPathLength - (double) i) / (double) m_nPathLength) * radius [0][j];
		*vertP = RectPoints (angle, length, &m_elements [i].m_relNode, &m_elements [i + 1].m_relNode);
		// spin vertices
		SpinBackPoint (vertP, ySpin, zSpin);
		// translate point back
		*vertP += m_bezier->GetPoint (0);
		}
	}
SetupVertices ();
}

//------------------------------------------------------------------------------

void CTunnelSegment::Realize (void)
{
ushort nVertex = 0;
for (short i = 0; i < m_nPathLength; i++) {
	CSegment* segP = segmentManager.Segment (m_elements [i].m_nSegment);
	// copy current segment
	*segP = *segmentManager.Segment (current->SegmentId ());
	for (int j = 0; j < 6; j++)
		segP->SetChild (j, -1);
	segP->m_info.bTunnel = 0;
	if (i == 0) {
		segP->SetChild (oppSideTable [m_base [0].m_nSide], m_base [0].m_nSegment);
		segP->SetChild (m_base [0].m_nSide, m_elements [1].m_nSegment);
		m_base [0].Segment ()->SetChild (m_base [0].m_nSide, m_elements [0].m_nSegment);
		} 
	else if (i == m_nPathLength - 1) {
		segP->SetChild (oppSideTable [m_base [0].m_nSide], m_elements [i - 1].m_nSegment); // previous tunnel segment
		segP->SetChild (m_base [0].m_nSide, m_base [1].m_nSegment);
		m_base [1].Segment ()->SetChild (m_base [1].m_nSide, m_elements [i].m_nSegment);
		}
	else  {
		segP->SetChild (oppSideTable [m_base [0].m_nSide], m_elements [i - 1].m_nSegment); // previous tunnel segment
		segP->SetChild (m_base [0].m_nSide, m_elements [i + 1].m_nSegment); // next tunnel segment
		}
	// define child bitmask, special, matcen, value, and wall bitmask
	}
SetupVertices ();
}

//------------------------------------------------------------------------------

void CTunnelSegment::Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* viewMatrix) 
{
CDC* pDC = renderer.DC ();

renderer.BeginRender ();
for (int i = 0; i < 4; i++) {
	m_bezier->Transform (viewMatrix);
	m_bezier->Project (viewMatrix);
	}
for (int i = 0; i < m_nPathLength; i++) {
	for (int j = 0; j < 4; j++) {
		CVertex&v = vertexManager [m_elements [i].m_nVertices [j]];
		v.Transform (viewMatrix);
		v.Project (viewMatrix);
		}
	}
renderer.EndRender ();

renderer.BeginRender (true);
renderer.SelectObject ((HBRUSH)GetStockObject (NULL_BRUSH));
renderer.SelectPen (penRed + 1);

CMineView* mineView = DLE.MineView ();
if (m_bezier->GetPoint (1).InRange (mineView->ViewMax ().x, mineView->ViewMax ().y, renderer.Type ())) {
	if (m_bezier->GetPoint (0).InRange (mineView->ViewMax ().x, mineView->ViewMax ().y, renderer.Type ())) {
		renderer.MoveTo (m_bezier->GetPoint (0).m_screen.x, m_bezier->GetPoint (0).m_screen.y);
		renderer.LineTo (m_bezier->GetPoint (1).m_screen.x, m_bezier->GetPoint (1).m_screen.y);
		renderer.Ellipse (m_bezier->GetPoint (1), 4, 4);
		}
	}
if (m_bezier->GetPoint (2).InRange (mineView->ViewMax ().x, mineView->ViewMax ().y, renderer.Type ())) {
	if (m_bezier->GetPoint (3).InRange (mineView->ViewMax ().x, mineView->ViewMax ().y, renderer.Type ())) {
		renderer.MoveTo (m_bezier->GetPoint (3).m_screen.x, m_bezier->GetPoint (3).m_screen.y);
		renderer.LineTo (m_bezier->GetPoint (2).m_screen.x, m_bezier->GetPoint (2).m_screen.y);
		renderer.Ellipse (m_bezier->GetPoint (2), 4, 4);
		}
	}
renderer.SelectPen (penBlue + 1);
for (int i = 0; i < m_nPathLength; i++)
	mineView->DrawSegmentWireFrame (segmentManager.Segment (m_elements [i].m_nSegment), false, false, 1);
renderer.EndRender ();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CTunnelMaker::Destroy (void)
{
if (m_bActive) {
	for (uint i = 0; i < m_tunnel.Length (); i++)
		m_tunnel [i].Destroy ();
	m_bActive = false;
	}
DLE.MineView ()->Refresh (false);
}

//------------------------------------------------------------------------------
// MENU - Tunnel Generator
//
// Action - This is like a super "join" feature which uses tunnels to
//          connect the cubes.  The number of cubes is determined
//          automatically.
//------------------------------------------------------------------------------

void CTunnelMaker::Create (void) 
{
if (!m_bActive) {
	short nMaxSegments = SEGMENT_LIMIT - segmentManager.Count ();
	if (nMaxSegments > MAX_TUNNEL_SEGMENTS)
		nMaxSegments = MAX_TUNNEL_SEGMENTS;
	else if (nMaxSegments < 3) {
		ErrorMsg ("Insufficient number of free vertices and/or segments\nto use the segment generator.");
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

	m_tunnel.Create (1);
	Setup ();
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
		Destroy ();
		undoManager.Begin (udSegments | udVertices);
		m_tunnel [0].Compute (PathLength ());
		m_tunnel [0].Realize ();
		undoManager.End ();
		}
	}
segmentManager.SetLinesToDraw ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Setup (void)
{
dynamic_cast<CSideKey&> (m_base [0]) = dynamic_cast<CSideKey&> (selections [0]);
dynamic_cast<CSideKey&> (m_base [1]) = dynamic_cast<CSideKey&> (selections [1]);
m_base [0].Setup ();
m_base [1].Setup ();
double length = Distance (segmentManager.CalcSideCenter (m_base [0]), segmentManager.CalcSideCenter (m_base [1]));
if (length < 50) {
	ErrorMsg ("End points of segment are too close.\n\n"
					"Hint: Select two sides which are further apart\n"
					"using the spacebar and left/right arrow keys,\n"
					"then try again.");
	return;
	}
// base nSegment length on distance between cubes
length *= 0.5;
if (length < MIN_TUNNEL_LENGTH)
	length = MIN_TUNNEL_LENGTH;
else if (length > MAX_TUNNEL_LENGTH)
	length = MAX_TUNNEL_LENGTH;

// setup intermediate points for a cubic bezier curve
m_nGranularity = 0;
m_bezier.SetLength (length, 0);
m_bezier.SetLength (length, 1);
m_bezier.SetPoint (m_base [0].GetPoint (), 0);
m_bezier.SetPoint (m_base [0].GetPoint () + m_base [0].GetNormal () * m_bezier.GetLength (0), 1);
m_bezier.SetPoint (m_base [1].GetPoint () + m_base [1].GetNormal () * m_bezier.GetLength (1), 2);
m_bezier.SetPoint (m_base [1].GetPoint (), 3);
m_tunnel [0].Setup (&m_bezier, m_base);
}

//------------------------------------------------------------------------------

void CTunnelMaker::Update (void) 
{ 
if ((*((CSideKey*) current) != *((CSideKey*) &m_base [0]) && *((CSideKey*) current) != *((CSideKey*) &m_base [1])) ||
    (*((CSideKey*) other) != *((CSideKey*) &m_base [0]) && *((CSideKey*) other) != *((CSideKey*) &m_base [1])))
	Setup ();
}

//------------------------------------------------------------------------------

short CTunnelMaker::PathLength (void)
{
m_nPathLength = short (m_bezier.Length () / 20.0 + Distance (m_base [0].GetPoint (), m_base [1].GetPoint ()) / 20.0) + m_nGranularity;
if (m_nPathLength > MaxSegments () - 1)
	m_nPathLength = MaxSegments () - 1;
else if (m_nPathLength < 3)
	m_nPathLength = 3;
return m_nPathLength;
}

//------------------------------------------------------------------------------

void CTunnelMaker::ComputeTunnel (void)
{
m_tunnel [0].Compute (PathLength ());
}

//------------------------------------------------------------------------------

void CTunnelMaker::Draw (CRenderer& renderer, CPen* redPen, CPen* bluePen, CViewMatrix* viewMatrix)
{
if (m_bActive) {
	Update ();
	if (PathLength () > 0) {
		m_tunnel [0].Compute (m_nPathLength);
		m_tunnel [0].Draw (renderer, redPen, bluePen, viewMatrix);
		}
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
	if (m_bezier.GetLength (0) > (MAX_TUNNEL_LENGTH - TUNNEL_INTERVAL))
		return;
	m_bezier.SetLength (m_bezier.GetLength (0) + TUNNEL_INTERVAL, 0);
	m_bezier.SetPoint (m_base [0].GetPoint () + m_base [0].GetNormal () * m_bezier.GetLength (0), 1);
	}
else if (current->SegmentId () == m_base [1].m_nSegment) {
	if (m_bezier.GetLength (1) > (MAX_TUNNEL_LENGTH - TUNNEL_INTERVAL))
		return;
	m_bezier.SetLength (m_bezier.GetLength (1) + TUNNEL_INTERVAL, 1);
	m_bezier.SetPoint (m_base [1].GetPoint () + m_base [1].GetNormal () * m_bezier.GetLength (1), 2);
	}
else
	return;
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Shrink (void) 
{
if (current->SegmentId () == m_base [0].m_nSegment) {
	if (m_bezier.GetLength (0) < (MIN_TUNNEL_LENGTH + TUNNEL_INTERVAL)) 
		return;
	m_bezier.SetLength (m_bezier.GetLength (0) - TUNNEL_INTERVAL, 0);
	m_bezier.SetPoint (m_base [0].GetPoint () + m_base [0].GetNormal () * m_bezier.GetLength (0), 1);
	}
else if (current->SegmentId () == m_base [1].m_nSegment) {
	if (m_bezier.GetLength (1) < (MIN_TUNNEL_LENGTH + TUNNEL_INTERVAL))
		return;
	m_bezier.SetLength (m_bezier.GetLength (1) - TUNNEL_INTERVAL, 1);
	m_bezier.SetPoint (m_base [1].GetPoint () + m_base [1].GetNormal () * m_bezier.GetLength (1), 2);
	}
else
	return;
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------
//eof TunnelMaker.cpp