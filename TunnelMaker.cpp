// Copyright (C) 1997 Bryan Aamot

#include "mine.h"
#include "dle-xp.h"

//------------------------------------------------------------------------------

CTunnelMaker tunnelMaker;

//------------------------------------------------------------------------------

#define CURRENT_POINT(a) ((current->m_nPoint + (a))&0x03)

//------------------------------------------------------------------------------

char szTunnelMakerError [] = "You must exit tunnel creation before preforming this function";

//------------------------------------------------------------------------------

long CTunnelMaker::Faculty (int n) 
{
long i = 1;

for (int j = n; j >= 2; j--) 
	i *= j;
return i;
}

//------------------------------------------------------------------------------
//   Coeff(n,i) - returns n!/(i!*(n-i)!)
//------------------------------------------------------------------------------

double CTunnelMaker::Coeff (int n, int i) 
{
return ((double)Faculty (n) / ((double)Faculty (i) * (double) Faculty (n-i)));
}

//------------------------------------------------------------------------------
//   Blend(i,n,u) - returns a weighted coefficient for each point in a nSegment
//------------------------------------------------------------------------------

double CTunnelMaker::Blend (int i, int n, double u) 
{
double partial = Coeff (n, i) * pow (u, i) * pow (1 - u, n - i);
return partial;
}

//------------------------------------------------------------------------------
//   BezierFcn(pt,u,n,p [][]) - sets (x,y,z) for u=#/segs based on m_points p
//------------------------------------------------------------------------------

CVertex CTunnelMaker::BezierFcn (double u, int npts, CVertex* p) 
{
	CVertex v;

for (int i = 0; i < npts; i++) {
	double b = Blend (i, npts - 1, u);
	v += p [i] * b;
	}
return v;
}

//------------------------------------------------------------------------------
// UntwistSegment ()
//
// Action - swaps vertices of opposing side if segment is twisted
//------------------------------------------------------------------------------

void CTunnelMaker::UntwistSegment (short nSegment, short nSide) 
{
  double		len, minLen = 1e10;
  short		index,j;
  CSegment*	segP;
  short		verts [4];

segP = segmentManager.Segment (nSegment);
// calculate length from point 0 to opp_points
CVertex* vert0 = vertexManager.Vertex (segP->m_info.verts [sideVertTable [nSide][0]]);
for (j = 0; j < 4; j++) {
	len = Distance (*vert0, *vertexManager.Vertex (segP->m_info.verts [oppSideVertTable [nSide][j]]));
	if (len < minLen) {
		minLen = len;
		index = j;
		}
	}
// swap verts if index != 0
if (index != 0) {
	for (j = 0; j < 4; j++)
		verts [j] = segP->m_info.verts [oppSideVertTable [nSide][(j + index) % 4]];
	for (j = 0; j < 4; j++)
		segP->m_info.verts [oppSideVertTable [nSide][j]] = verts [j];
	}
}

//------------------------------------------------------------------------------
// SpinPoint () - spin on y-axis then z-axis
//------------------------------------------------------------------------------

void CTunnelMaker::SpinPoint (CVertex* point, double ySpin, double zSpin) 
{
double tx = point->v.x * cos (ySpin) - point->v.z * sin (ySpin);
double ty = point->v.y;
double tz = point->v.x * sin (ySpin) + point->v.z * cos (ySpin);
point->Set (tx * cos (zSpin) + ty * sin (zSpin), ty * cos (zSpin) - tx * sin (zSpin), tz);
}

//------------------------------------------------------------------------------
// SpinBackPoint () - spin on z-axis then y-axis
//------------------------------------------------------------------------------

void CTunnelMaker::SpinBackPoint (CVertex* point, double ySpin, double zSpin) 
{
double tx = point->v.x * cos (-zSpin) + point->v.y * sin (-zSpin);
double ty = -point->v.x * sin (-zSpin) + point->v.y * cos (-zSpin);
double tz = point->v.z;
point->Set (tx * cos (-ySpin) - tz * sin (-ySpin), ty, tx * sin (-ySpin) + tz * cos (-ySpin));
}

//------------------------------------------------------------------------------
// MatchingSide ()
//
// Action - Returns matching side depending on the current m_points
//------------------------------------------------------------------------------

int CTunnelMaker::MatchingSide (int j) 
{
  static int ret [4][4] = {{3,2,1,0},{2,1,0,3},{1,0,3,2},{0,3,2,1}};
  int offset;

offset = (4 + selections [0].m_nPoint - selections [1].m_nPoint) % 4;
return ret [offset][j];
}

//------------------------------------------------------------------------------
//
// Action - Spins m_points which lie in the y-z plane orthagonal to a normal
//          Uses normal as center for translating m_points.
//
// Changes - Chooses axis to normalizes on based on "normal" direction
//------------------------------------------------------------------------------

CDoubleVector CTunnelMaker::RectPoints (double angle, double radius, CVertex* origin, CVertex* normal) 
{
  double			ySpin,zSpin;
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

void CTunnelMaker::PolarPoints (double *angle, double *radius, CVertex* vertex, CVertex* orgin, CVertex* normal) 
{
double x1,y1,z1,y2,z2;
// translate coordanites to orgin
double vx = vertex->v.x - orgin->v.x;
double vy = vertex->v.y - orgin->v.y;
double vz = vertex->v.z - orgin->v.z;
double nx = normal->v.x - orgin->v.x;
double ny = normal->v.y - orgin->v.y;
double nz = normal->v.z - orgin->v.z;

// calculate angles to normalize direction
// spin on z axis to get into the x-z plane
double zSpin = atan3 (ny,nx);
x1 = nx * cos (zSpin) + ny * sin (zSpin);
z1 = nz;
// spin on y to get on the x axis
double ySpin = -atan3 (z1,x1);
// spin vertex (spin on z then y)
x1 = vx * cos (zSpin) + vy * sin (zSpin);
y1 = -vx * sin (zSpin) + vy * cos (zSpin);
z1 = vz;
//  x2 =   x1 * cos (ySpin) - z1 * sin (ySpin);
y2 = y1;
z2 = x1 * sin (ySpin) + z1 * cos (ySpin);
// convert to polar
*radius = sqrt(y2*y2 + z2*z2);  // ignore any x offset
*angle = atan3 (z2,y2);
}

//------------------------------------------------------------------------------
// define segment vert numbers

void CTunnelMaker::SetupVertices (void)
{
ushort nVertex = 0;
for (short i = 0; i < m_nLength [0]; i++) {
	CSegment* segP = segmentManager.Segment (m_nSegments [i]);
	for (short j = 0; j < 4; j++, nVertex++) {
		if (i == 0) { // 1st segment
			segP->m_info.verts [sideVertTable [m_info [0].m_nSide][j]] = m_nVertices [nVertex];
			segP->m_info.verts [oppSideVertTable [m_info [0].m_nSide][j]] = m_info [0].Segment ()->m_info.verts [sideVertTable [m_info [0].m_nSide][j]];
			}
		else if (i == m_nLength [0] - 1) { // last segment
			segP->m_info.verts [sideVertTable [m_info [0].m_nSide][j]] = m_info [1].Segment ()->m_info.verts [sideVertTable [m_info [1].m_nSide][MatchingSide (j)]];
			segP->m_info.verts [oppSideVertTable [m_info [0].m_nSide][j]] = m_nVertices [nVertex - 4];
			}
		else {
			segP->m_info.verts [sideVertTable [m_info [0].m_nSide][j]] = m_nVertices [nVertex];
			segP->m_info.verts [oppSideVertTable [m_info [0].m_nSide][j]] = m_nVertices [nVertex - 4];
			}
		}
	}
// int twisted segments
for (short i = 0; i < m_nLength [0]; i++)
	UntwistSegment (m_nSegments [i], m_info [0].m_nSide);
}

//------------------------------------------------------------------------------

void CTunnelMaker::Realize (void)
{
ushort nVertex = 0;
for (short nSegment = 0; nSegment < m_nLength [0]; nSegment++) {
	CSegment* segP = segmentManager.Segment (m_nSegments [nSegment]);
	// copy current segment
	*segP = *segmentManager.Segment (current->m_nSegment);
	segP->m_info.bTunnel = 0;
	if (nSegment == 0) {
		segP->SetChild (oppSideTable [m_info [0].m_nSide], m_info [0].m_nSegment);
		segP->SetChild (m_info [0].m_nSide, m_nSegments [1]);
		m_info [0].Segment ()->SetChild (m_info [0].m_nSide, m_nSegments [0]);
		} 
	else if (nSegment == m_nLength [0] - 1) {
		segP->SetChild (oppSideTable [m_info [0].m_nSide], m_nSegments [nSegment - 1]); // previous tunnel segment
		segP->SetChild (m_info [0].m_nSide, m_info [1].m_nSegment);
		m_info [1].Segment ()->SetChild (m_info [1].m_nSide, m_nSegments [nSegment]);
		}
	else  {
		segP->SetChild (oppSideTable [m_info [0].m_nSide], m_nSegments [nSegment - 1]); // previous tunnel segment
		segP->SetChild (m_info [0].m_nSide, m_nSegments [nSegment + 1]); // next tunnel segment
		}
	// define child bitmask, special, matcen, value, and wall bitmask
	}
SetupVertices ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Remove (int l)
{
undoManager.Lock ();
for (int i = l; i > 0; )
	segmentManager.Remove (m_nSegments [--i]);
for (int i = (l - 1) * 4; i > 0; )
	vertexManager.Delete (m_nVertices [--i]);
undoManager.Unlock ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Destroy (void)
{
if (m_bActive) {
	Remove (m_nLength [0]);
	m_nLength [0] = 0;
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

//  undoManager.UpdateBuffer(0);

double		length;
int			i;

if (!m_bActive) {
	m_nMaxSegments = MAX_SEGMENTS - segmentManager.Count ();
	if (m_nMaxSegments > MAX_TUNNEL_SEGMENTS)
		m_nMaxSegments = MAX_TUNNEL_SEGMENTS;
	else if (m_nMaxSegments < 3) {
//	if ((vertexManager.Count () + 3 /*MAX_TUNNEL_SEGMENTS*/ * 4 > MAX_VERTICES) ||
//		 (segmentManager.Count () + 3 /*MAX_TUNNEL_SEGMENTS*/ > MAX_SEGMENTS)) {
		ErrorMsg ("Insufficient number of free vertices and/or segments\nto use the segment generator.");
		return;
		}
	// make sure there are no children on either segment/side
	other = &selections [!current->Index ()];
	if ((current->Segment ()->Child (current->m_nSide) != -1) ||
		 (other->Segment ()->Child (other->m_nSide) != -1)) {
		ErrorMsg ("Starting and/or ending point of segment\n"
					 "already have segment(s) attached.\n\n"
					 "Hint: Put the current segment and the alternate segment\n"
					 "on sides which do not have cubes attached.");
		return;
		}

	dynamic_cast<CSideKey&> (m_info [0]) = dynamic_cast<CSideKey&> (selections [0]);
	dynamic_cast<CSideKey&> (m_info [1]) = dynamic_cast<CSideKey&> (selections [1]);
	// define 4 data m_points for nSegment to work from
	// center of current segment
	m_points [0] = segmentManager.CalcSideCenter (m_info [0]);
	// center of other segment
	m_points [3] = segmentManager.CalcSideCenter (m_info [1]);
	// calculate length between cubes
	length = Distance (m_points [0], m_points [3]);
	if (length < 50) {
		ErrorMsg ("End points of segment are too close.\n\n"
					 "Hint: Select two sides which are further apart\n"
					 "using the spacebar and left/right arrow keys,\n"
					 "then try again.");
		return;
		}
	// base nSegment length on distance between cubes
	for (i = 0; i < 2; i++) {
		m_info [i].m_length = (length / 3);
		if (m_info [i].m_length < MIN_TUNNEL_LENGTH)
			m_info [i].m_length = MIN_TUNNEL_LENGTH;
		else if (m_info [i].m_length > MAX_TUNNEL_LENGTH)
			m_info [i].m_length = MAX_TUNNEL_LENGTH;
		}
	if (!bExpertMode)
		ErrorMsg ("Place the current segment on one of the segment end points.\n\n"
				    "Use the ']' and '[' keys to adjust the length of the red\n"
				    "segment segment.\n\n"
				    "Press 'P' to rotate the point connections.\n\n"
				    "Press 'G' or select Tools/Tunnel Generator when you are finished.");

	m_nLength [0] = m_nLength [1] = 0;

	m_bActive = true;
	}
else {
	// ask if user wants to keep the new nSegment
	if (Query2Msg ("Do you want to keep this tunnel?", MB_YESNO) != IDYES) 
		Destroy ();
	else {
		Destroy ();
		undoManager.Begin (udSegments | udVertices);
		ComputeTunnel ();
		Realize ();
		undoManager.End ();
		}
	}
segmentManager.SetLinesToDraw ();
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Stretch (void) 
{

//undoManager.UpdateBuffer(0);

if (current->m_nSegment == m_info [0].m_nSegment)
	if (m_info [0].m_length < (MAX_TUNNEL_LENGTH - TUNNEL_INTERVAL))
		m_info [0].m_length += TUNNEL_INTERVAL;
if (current->m_nSegment == m_info [1].m_nSegment)
	if (m_info [1].m_length < (MAX_TUNNEL_LENGTH - TUNNEL_INTERVAL))
		m_info [1].m_length += TUNNEL_INTERVAL;
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Shrink (void) 
{

//  undoManager.UpdateBuffer(0);

if (current->m_nSegment == m_info [0].m_nSegment)
	if (m_info [0].m_length > (MIN_TUNNEL_LENGTH + TUNNEL_INTERVAL))
		m_info [0].m_length -= TUNNEL_INTERVAL;
if (current->m_nSegment == m_info [1].m_nSegment)
	if (m_info [1].m_length > (MIN_TUNNEL_LENGTH + TUNNEL_INTERVAL))
		m_info [1].m_length -= TUNNEL_INTERVAL;
DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------
// ComputeTunnel ()
//
// Note: this routine is called by dmb.cpp - update_display() everytime
//       the display is redrawn.
//------------------------------------------------------------------------------

void CTunnelMaker::ComputeTunnel (void) 
{
  double length;
  int i,j;
  CSegment *segP;
  CVertex vertex;
  double theta [2][4],radius [2][4]; // polor coordinates of sides
  double deltaAngle [4];
  CVertex relSidePoints [2][4]; // side m_points reletave to center of side 1
  CVertex relPoints [4]; // 4 m_points of nSegment reletave to 1st point
  CVertex relTunnelPoints [MAX_TUNNEL_SEGMENTS];
  double y,z;
  double ySpin,zSpin;

// center of both cubes
m_points [0] = segmentManager.CalcSideCenter (m_info [0]);
m_points [3] = segmentManager.CalcSideCenter (m_info [1]);
// point orthogonal to center of current segment
m_points [1] = segmentManager.CalcSideNormal (m_info [0]);
m_points [1] *= m_info [0].m_length;
m_points [1] += m_points [0];
// point orthogonal to center of other segment
m_points [2] = segmentManager.CalcSideNormal (m_info [1]);
m_points [2] *= m_info [1].m_length;
m_points [2] += m_points [3];

// calculate number of segments (min=1)
length = Distance (*m_points, m_points [3]);
m_nLength [1] = m_nLength [0];
m_nLength [0] = (int) ((fabs (m_info [0].m_length) + fabs (m_info [1].m_length)) / 20 + length / 40.0);
m_nLength [0] = min (m_nLength [0], m_nMaxSegments - 1);

if (m_nLength [1] != m_nLength [0]) {
	if (m_nLength [1] > 0)
		Remove (m_nLength [1]);
	for (i = 0; i < m_nLength [0]; i++) {
		m_nSegments [i] = segmentManager.Add ();
		segmentManager.Segment (m_nSegments [i])->m_info.bTunnel = 1;
		//segmentManager.Segment (m_nSegments [i])->Setup ();
		}
	vertexManager.Add (&m_nVertices [0], (m_nLength [0] - 1) * 4);
	}

// calculate nSegment m_points
for (i = 0; i <= m_nLength [0]; i++) 
	m_tunnelPoints [i] = BezierFcn ((double) i / (double) m_nLength [0], 4, m_points);

// make all m_points reletave to first face (translation)
for (i = 0; i < 4; i++) 
	relPoints [i] = m_points [i] - m_points [0];

for (i = 0; i < 2; i++) {
	segP = m_info [i].Segment ();
	for (j = 0; j < 4; j++) 
		relSidePoints [i][j] = *vertexManager.Vertex (segP->m_info.verts [sideVertTable [m_info [i].m_nSide][j]]) - m_points [0];
	}

for (i = 0; i < m_nLength [0]; i++)
	relTunnelPoints [i] = m_tunnelPoints [i] - m_points [0];

// determine y-spin and z-spin to put 1st orthogonal vector onto the x-axis
ySpin = -atan3 (relPoints [1].v.z, relPoints [1].v.x); // to y-z plane
zSpin = atan3 (relPoints [1].v.y, relPoints [1].v.x * cos (ySpin) - relPoints [1].v.z * sin (ySpin)); // to x axis

// spin all m_points reletave to first face (rotation)
for (i = 0; i < 4; i++) {
	SpinPoint (relPoints + i, ySpin, zSpin);
	for (j = 0; j < 2; j++) 
		SpinPoint (relSidePoints [j] + i, ySpin, zSpin);
	}

for (i = 0; i < m_nLength [0]; i++) 
	SpinPoint (relTunnelPoints + i, ySpin, zSpin);

// determine polar coordinates of the 1st side (simply y,z coords)
for (i = 0; i < 4; i++) {
	theta [0][i] = atan3 (relSidePoints [0][i].v.z,relSidePoints [0][i].v.y);
	y = relSidePoints [0][i].v.y;
	z = relSidePoints [0][i].v.z;
	radius [0][i] =_hypot (y, z);
	}

// determine polar coordinates of the 2nd side by rotating to x-axis first
for (i = 0; i < 4; i++) {
	// flip orthoginal vector to point into segment
	vertex = (relPoints [3] * 2) - relPoints [2];
	//vertex.Set (relPoints [3].v.x - relPoints [2].v.x + relPoints [3].v.x,
	//				relPoints [3].v.y - relPoints [2].v.y + relPoints [3].v.y,
	//				relPoints [3].v.z - relPoints [2].v.z + relPoints [3].v.z);
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
for (i = 0; i < m_nLength [0] - 1; i++) {
	for (j = 0; j < 4; j++) {
		CVertex* vertP = vertexManager.Vertex (m_nVertices [nVertex++]);
		double h = (double) i / (double) m_nLength [0];
		double angle  = h * deltaAngle [j] + theta [0][j];
		double length = h * radius [1][MatchingSide (j)] + (((double) m_nLength [0] - (double) i) / (double) m_nLength [0]) * radius [0][j];
		*vertP = RectPoints (angle, length, &relTunnelPoints [i], &relTunnelPoints [i+1]);
		// spin vertices
		SpinBackPoint (vertP, ySpin, zSpin);
		// translate point back
		*vertP += m_points [0];
		}
	}
SetupVertices ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::Draw (CDC* pDC, CPen* redPen, CPen* bluePen, CViewMatrix& view) 
{
if (!m_bActive)
	return;

//  SelectObject(hdc, hrgnAll);
pDC->SelectObject (redPen);
pDC->SelectObject ((HBRUSH)GetStockObject (NULL_BRUSH));
tunnelMaker.ComputeTunnel ();
tLongVector point;
view.Project (m_points [1], point);
if (IN_RANGE (point.x, x_max) && IN_RANGE (point.y, y_max)) {
	view.Project (m_points [0], point);
	if (IN_RANGE (point.x, x_max) && IN_RANGE (point.y, y_max)) {
		pDC->MoveTo (point.x, point.y);
		view.Project (m_points [1], point);
		pDC->LineTo (point.x, point.y);
		pDC->Ellipse (point.x - 4, point.y - 4, point.x+4,  point.y + 4);
		}
	}
view.Project (m_points [2], point);
if (IN_RANGE (point.x, x_max) && IN_RANGE (point.y, y_max)){
	view.Project (m_points [3], point);
	if (IN_RANGE (point.x, x_max) && IN_RANGE (point.y, y_max)){
		pDC->MoveTo (point.x, point.y);
		view.Project (m_points [2], point);
		pDC->LineTo (point.x, point.y);
		pDC->Ellipse (point.x - 4, point.y - 4, point.x+4,  point.y + 4);
		}
	}
pDC->SelectObject (bluePen);
CMineView* viewP = DLE.MineView ();
int h = (tunnelMaker.Length () - 1) * 4;
for (int i = 0; i < h; i++)
	view.Project (*vertexManager.Vertex (m_nVertices [i]), viewP->m_viewPoints [m_nVertices [i]]);
h = tunnelMaker.Length ();
for (int i = 0; i < h; i++)
	viewP->DrawSegmentQuick (segmentManager.Segment (m_nSegments [i]), false, 1);
}

//------------------------------------------------------------------------------
//eof tunnelgen.cpp