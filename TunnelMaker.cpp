// Copyright (C) 1997 Bryan Aamot

#include "mine.h"

#define CURRENT_POINT(a) ((current.m_nPoint + (a))&0x03)

CTunnelMaker tunnelMaker;

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
//   Blend(i,n,u) - returns a weighted coefficient for each point in a spline
//------------------------------------------------------------------------------

double CTunnelMaker::Blend (int i, int n, double u) 
{
double partial = Coeff (n, i) * pow (u, i) * pow (1 - u, n - i);
return partial;
}

//------------------------------------------------------------------------------
//   BezierFcn(pt,u,n,p [][]) - sets (x,y,z) for u=#/segs based on points p
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
// Action - swaps vertices of opposing side if cube is twisted
//------------------------------------------------------------------------------

void CTunnelMaker::UntwistSegment (short nSegment,short nSide) 
{
  double		len, minLen = 1e10;
  short		index,j;
  CSegment*	segP;
  short		verts [4];

segP = segmentManager.GetSegment (nSegment);
// calculate length from point 0 to opp_points
CVertex* vert0 = vertexManager.GetVertex (segP->m_info.verts [sideVertTable [nSide][0]]);
for (j = 0; j < 4; j++) {
	len = Distance (*vert0, *vertexManager.GetVertex (segP->m_info.verts [oppSideVertTable [nSide][j]]));
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
// Action - Returns matching side depending on the current points
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
// Action - Spins points which lie in the y-z plane orthagonal to a normal
//          Uses normal as center for translating points.
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
// MENU - Tunnel Generator
//
// Action - This is like a super "join" feature which uses splines to
//          connect the cubes.  The number of cubes is determined
//          automatically.
//------------------------------------------------------------------------------

void CTunnelMaker::Create (void) 
{

//  undoManager.UpdateBuffer(0);

double		length;
int			i, j, nVertex, spline;
CSegment*	segP;

if (!m_bActive) {
	m_nMaxSplines = MAX_SEGMENTS - segmentManager.Count ();
	if (m_nMaxSplines > MAX_SPLINES)
		m_nMaxSplines = MAX_SPLINES;
	else if (m_nMaxSplines < 3) {
//	if ((vertexManager.Count () + 3 /*MAX_SPLINES*/ * 4 > MAX_VERTICES) ||
//		 (segmentManager.Count () + 3 /*MAX_SPLINES*/ > MAX_SEGMENTS)) {
		ErrorMsg ("Insufficient number of free vertices and/or segments\n"
					"to use the tunnel generator.");
		return;
		}
	// make sure there are no children on either segment/side
	if ((segmentManager.GetSegment (selections [0].m_nSegment)->GetChild (selections [0].m_nSide) != -1) ||
		 (segmentManager.GetSegment (selections [1].m_nSegment)->GetChild (selections [1].m_nSide) != -1)) {
		ErrorMsg ("Starting and/or ending point of spline\n"
					"already have cube(s) attached.\n\n"
					"Hint: Put the current cube and the alternate cube\n"
					"on sides which do not have cubes attached.");
		return;
		}
	nSplineSeg1 = selections [0].m_nSegment;
	nSplineSeg2 = selections [1].m_nSegment;
	nSplineSide1 = selections [0].m_nSide;
	nSplineSide2 = selections [1].m_nSide;
	// define 4 data points for spline to work from
	// center of current cube
	points [0] = segmentManager.CalcSideCenter (nSplineSeg1, nSplineSide1);
	// center of other cube
	points [3] = segmentManager.CalcSideCenter (nSplineSeg2, nSplineSide2);
	// calculate length between cubes
	length = Distance (*points, points [3]);
	// base spline length on distance between cubes
	m_splineLength1 = (length / 3);
	if (m_splineLength1 < MIN_SPLINE_LENGTH)
		m_splineLength1 = MIN_SPLINE_LENGTH;
	if (m_splineLength1 > MAX_SPLINE_LENGTH)
		m_splineLength1 = MAX_SPLINE_LENGTH;
	m_splineLength2 = (length / 3);
	if (m_splineLength2 < MIN_SPLINE_LENGTH)
		m_splineLength2 = MIN_SPLINE_LENGTH;
	if (m_splineLength2 > MAX_SPLINE_LENGTH)
		m_splineLength2 = MAX_SPLINE_LENGTH;
	if (length < 50) {
		ErrorMsg ("End points of tunnel are too close.\n\n"
					"Hint: Select two sides which are further apart\n"
					"using the spacebar and left/right arrow keys,\n"
					"then try again.");
		return;
		}
	if (!bExpertMode)
		ErrorMsg ("Place the current cube on one of the tunnel end points.\n\n"
				  "Use the ']' and '[' keys to adjust the length of the red\n"
				  "spline segment.\n\n"
				  "Press 'P' to rotate the point connections.\n\n"
				  "Press 'G' or select Tools/Tunnel Generator when you are finished.");
	m_bActive = true;
	}
else {
	m_bActive = false;
	// ask if user wants to keep the new spline
	if (Query2Msg ("Do you want to keep this tunnel?", MB_YESNO) != IDYES) {
		//DLE.MineView ()->Refresh (false);
		return;
		}
	undoManager.SetModified (true);
	undoManager.Lock ();
	for (spline = 0; spline < n_splines; spline++) {
		segP = segmentManager.GetSegment (segmentManager.Count ());
		// copy current segment
		*segP = *segmentManager.GetSegment (current.m_nSegment);
		// use last "n_spline" segments
		nVertex = (MAX_VERTICES - 1) - spline * 4;
		for (j = 0; j < 4; j++) {
			if (vertexManager.Count () >= MAX_VERTICES)
				DEBUGMSG (" Tunnel generator: Vertex count out of range.")
			else if ((nVertex - j < 0) || (nVertex - j >= MAX_VERTICES))
				DEBUGMSG (" Tunnel generator: Vertex number out of range.")
			else
				*vertexManager.GetVertex (vertexManager.Count ()) = *vertexManager.GetVertex (nVertex - j);
/*
			vertices [vertexManager.Count ()].x = vertices [nVertex-j].x;
			vertices [vertexManager.Count ()].y = vertices [nVertex-j].y;
			vertices [vertexManager.Count ()].z = vertices [nVertex-j].z;
*/
			if (spline == 0) {         // 1st segment
				segP->m_info.verts [sideVertTable [nSplineSide1][j]] = vertexManager.Count ();
				segP->m_info.verts [oppSideVertTable [nSplineSide1][j]] = segmentManager.GetSegment (nSplineSeg1)->m_info.verts [sideVertTable [nSplineSide1][j]];
				vertexManager.Status (vertexManager.Count ()++) = 0;
				}
			else if(spline < n_splines - 1) { // center segments
				segP->m_info.verts [sideVertTable [nSplineSide1][j]] = vertexManager.Count ();
				segP->m_info.verts [oppSideVertTable [nSplineSide1][j]] = vertexManager.Count () - 4;
				vertexManager.Status (vertexManager.Count ()++) = 0;
				}
			else {          // last segment
				segP->m_info.verts [sideVertTable [nSplineSide1][j]] = segmentManager.GetSegment (nSplineSeg2)->m_info.verts [sideVertTable [nSplineSide2][MatchingSide (j)]];
				segP->m_info.verts [oppSideVertTable [nSplineSide1][j]] = vertexManager.Count () - 4 + j;
				}
			}
		// int twisted segments
		UntwistSegment (segmentManager.Count (), nSplineSide1);
		// define children and sides (textures and nWall)
		for (j = 0; j < 6; j++) {
			segP->SetChild (j, -1);
			segP->m_sides [j].m_info.nBaseTex = 0;
			segP->m_sides [j].m_info.nOvlTex = 0;
			segP->m_sides [j].m_info.nWall = NO_WALL;
			for (i = 0; i < 4; i++) {
//	    segP->m_sides [j].uvls [i].u = defaultUVLs [i].u;
//	    segP->m_sides [j].uvls [i].v = defaultUVLs [i].v;
				segP->m_sides [j].m_info.uvls [i].l = (ushort) DEFAULT_LIGHTING;
				}
			segP->SetUV (j,0,0);
			}
		if (spline == 0) {
			segP->SetChild (oppSideTable [nSplineSide1], nSplineSeg1);
			segP->SetChild (nSplineSide1, segmentManager.Count () + 1);
			segmentManager.GetSegment (nSplineSeg1)->SetChild (nSplineSide1, segmentManager.Count ());
			} 
		else if (spline < n_splines-1) {
			segP->SetChild (oppSideTable [nSplineSide1], segmentManager.Count () - 1);
			segP->SetChild (nSplineSide1, segmentManager.Count () + 1);
			}
		else {
			segP->SetChild (oppSideTable [nSplineSide1], segmentManager.Count () - 1);
			segP->SetChild (nSplineSide1, nSplineSeg2);
			segmentManager.GetSegment (nSplineSeg2)->SetChild (nSplineSide2, segmentManager.Count ());
			}
		// define child bitmask, special, matcen, value, and wall bitmask
		segP->m_info.owner = -1;
		segP->m_info.group = -1;
		segP->m_info.function = 0;
		segP->m_info.nMatCen = -1;
		segP->m_info.value = -1;
		segP->m_info.wallFlags = 0; // make sure segment is not marked
		segmentManager.Count ()++;
		}
	undoManager.Unlock ();
	}
segmentManager.SetLinesToDraw ();
//DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::IncreaseSpline (void) 
{

//undoManager.UpdateBuffer(0);

if (current.m_nSegment == nSplineSeg1)
	if (m_splineLength1 < (MAX_SPLINE_LENGTH-SPLINE_INTERVAL))
		m_splineLength1 += SPLINE_INTERVAL;
if (current.m_nSegment == nSplineSeg2)
	if (m_splineLength2 < (MAX_SPLINE_LENGTH-SPLINE_INTERVAL))
		m_splineLength2 += SPLINE_INTERVAL;
//DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------

void CTunnelMaker::DecreaseSpline (void) 
{

//  undoManager.UpdateBuffer(0);

if (current.m_nSegment == nSplineSeg1)
	if (m_splineLength1 > (MIN_SPLINE_LENGTH+SPLINE_INTERVAL))
		m_splineLength1 -= SPLINE_INTERVAL;
if (current.m_nSegment == nSplineSeg2)
	if (m_splineLength2 > (MIN_SPLINE_LENGTH+SPLINE_INTERVAL))
		m_splineLength2 -= SPLINE_INTERVAL;
//DLE.MineView ()->Refresh ();
}

//------------------------------------------------------------------------------
// ComputeSpline ()
//
// Note: this routine is called by dmb.cpp - update_display() everytime
//       the display is redrawn.
//------------------------------------------------------------------------------

void CTunnelMaker::ComputeSpline (void) 
{
  double length;
  double angle;
  int i,j;
  CSegment *segP;
  ushort nVertex;
  CVertex vertex;
  double theta [2][4],radius [2][4]; // polor coordinates of sides
  double delta_angle [4];
  CVertex rel_side_pts [2][4]; // side points reletave to center of side 1
  CVertex rel_pts [4]; // 4 points of spline reletave to 1st point
  CVertex rel_spline_pts [MAX_SPLINES];
  double y,z;
  double ySpin,zSpin;

// center of both cubes
points [0] = segmentManager.CalcSideCenter (nSplineSeg1, nSplineSide1);
points [3] = segmentManager.CalcSideCenter (nSplineSeg2, nSplineSide2);
// point orthogonal to center of current cube
points [1] = segmentManager.CalcSideNormal (nSplineSeg1, nSplineSide1);
points [1] *= m_splineLength1;
points [1] += points [0];
// point orthogonal to center of other cube
points [2] = segmentManager.CalcSideNormal (nSplineSeg2, nSplineSide2);
points [2] *= m_splineLength2;
points [2] += points [3];

// calculate number of segments (min=1)
length = Distance (*points, points [3]);
n_splines = (int) ((fabs (m_splineLength1) + fabs (m_splineLength2)) / 20 + length / 40.0);
n_splines = min (n_splines, m_nMaxSplines - 1);

// calculate spline points
for (i = 0; i <= n_splines; i++) 
	splinePoints [i] = BezierFcn ((double) i / (double) n_splines, 4, points);

// make all points reletave to first face (translation)
for (i = 0; i < 4; i++) 
	rel_pts [i] = points [i] - points [0];
segP = segmentManager.GetSegment (nSplineSeg1);
for (i = 0; i < 4; i++) 
	rel_side_pts [0][i] = *vertexManager.GetVertex (segP->m_info.verts [sideVertTable [nSplineSide1][i]]) - points [0];
segP = segmentManager.GetSegment (nSplineSeg2);
for (i = 0; i < 4; i++)
	rel_side_pts [1][i] = *vertexManager.GetVertex (segP->m_info.verts [sideVertTable [nSplineSide2][i]]) - points [0];
for (i = 0; i < n_splines; i++) {
	rel_spline_pts [i] = splinePoints [i] - points [0];
}

// determine y-spin and z-spin to put 1st orthogonal vector onto the x-axis
ySpin = -atan3 (rel_pts [1].v.z, rel_pts [1].v.x); // to y-z plane
zSpin = atan3 (rel_pts [1].v.y, rel_pts [1].v.x * cos (ySpin)-rel_pts [1].v.z * sin (ySpin)); // to x axis

// spin all points reletave to first face (rotation)
for (i = 0; i < 4; i++) {
	SpinPoint (rel_pts + i,ySpin,zSpin);
	for (j = 0; j < 2; j++) 
		SpinPoint (rel_side_pts [j] + i,ySpin,zSpin);
	}

for (i = 0; i < n_splines; i++) 
	SpinPoint (rel_spline_pts + i,ySpin,zSpin);

// determine polar coordinates of the 1st side (simply y,z coords)
for (i = 0; i < 4; i++) {
	theta [0][i] = atan3 (rel_side_pts [0][i].v.z,rel_side_pts [0][i].v.y);
	y = (float) rel_side_pts [0][i].v.y;
	z = (float) rel_side_pts [0][i].v.z;
	radius [0][i] = sqrt(y*y + z*z);
	}

// determine polar coordinates of the 2nd side by rotating to x-axis first
for (i = 0; i < 4; i++) {
	// flip orthoginal vector to point into cube
	vertex.Set (rel_pts [3].v.x - rel_pts [2].v.x + rel_pts [3].v.x,
					rel_pts [3].v.y - rel_pts [2].v.y + rel_pts [3].v.y,
					rel_pts [3].v.z - rel_pts [2].v.z + rel_pts [3].v.z);
	PolarPoints (&theta [1][i], &radius [1][i], &rel_side_pts [1][i], &rel_pts [3], &vertex);
	}

// figure out the angle differences to be in range (-pi to pi)
for (j = 0; j < 4; j++) {
	delta_angle [j] = theta [1][MatchingSide (j)] - theta [0][j];
	if (delta_angle [j] < M_PI) 
		delta_angle [j] += 2*M_PI;
	if (delta_angle [j] > M_PI) 
		delta_angle [j] -= 2*M_PI;
	}

// make sure delta angles do not cross PI & -PI
for (i = 1; i < 4; i++) {
	if (delta_angle [i] > delta_angle [0] + M_PI) 
		delta_angle [i] -= 2*M_PI;
	if (delta_angle [i] < delta_angle [0] - M_PI) 
		delta_angle [i] += 2*M_PI;
	}

#if 0
sprintf_s (message, sizeof (message), "theta [0] = %d,%d,%d,%d\n"
"theta [1] = %d,%d,%d,%d\n"
"radius [0] = %d,%d,%d,%d\n"
"radius [1] = %d,%d,%d,%d\n"
"delta_angles = %d,%d,%d,%d\n"
"ySpin = %d\n"
"zSpin = %d",
(int)(theta [0][0]*180/M_PI),(int)(theta [0][1]*180/M_PI),(int)(theta [0][2]*180/M_PI),(int)(theta [0][3]*180/M_PI),
(int)(theta [1][0]*180/M_PI),(int)(theta [1][1]*180/M_PI),(int)(theta [1][2]*180/M_PI),(int)(theta [1][3]*180/M_PI),
(int)(radius [0][0]*180/M_PI),(int)(radius [0][1]*180/M_PI),(int)(radius [0][2]*180/M_PI),(int)(radius [0][3]*180/M_PI),
(int)(radius [1][0]*180/M_PI),(int)(radius [1][1]*180/M_PI),(int)(radius [1][2]*180/M_PI),(int)(radius [1][3]*180/M_PI),
(int)(delta_angle [0]*180/M_PI),(int)(delta_angle [1]*180/M_PI),(int)(delta_angle [2]*180/M_PI),(int)(delta_angle [3]*180/M_PI),
(int)(ySpin*180/M_PI),(int)(zSpin*180/M_PI)
);
DebugMsg(message);
#endif

// calculate segment vertices as weighted average between the two sides
// then spin vertices in the direction of the segment vector
for (i = 0; i < n_splines; i++) {
	nVertex = (MAX_VERTICES-1) - (i * 4);
	for (j = 0; j < 4; j++) {
		CVertex* vert = vertexManager.GetVertex (nVertex - j);
		angle  = ((double) i / (double) n_splines) * delta_angle [j] + theta [0][j];
		length = ((double) i / (double) n_splines) * radius [1][MatchingSide (j)] + (((double) n_splines- (double) i) / (double) n_splines) * radius [0][j];
		*vert = RectPoints (angle, length, &rel_spline_pts [i], &rel_spline_pts [i+1]);
		// spin vertices
		SpinBackPoint (vert, ySpin, zSpin);
		// translate point back
		*vert += points [0];
		}
	}

  // define segment vert numbers
for (i = 0; i < n_splines; i++) {
	// use last "n_spline" segments
	segP = segmentManager.GetSegment (MAX_SEGMENTS - 1 - i);
	nVertex = MAX_VERTICES - 1 - i * 4;
	for (j = 0; j < 4; j++) {
		if (i == 0) {         // 1st segment
			segP->m_info.verts [sideVertTable [nSplineSide1][j]] = nVertex - j;
			segP->m_info.verts [oppSideVertTable [nSplineSide1][j]] = segmentManager.GetSegment (nSplineSeg1)->m_info.verts [sideVertTable [nSplineSide1][j]];
			}
		else {
			if(i < n_splines - 1) { // center segments
				segP->m_info.verts [sideVertTable [nSplineSide1][j]] = nVertex - j;
				segP->m_info.verts [oppSideVertTable [nSplineSide1][j]] = nVertex + 4 - j;
				} 
			else {          // last segment
				segP->m_info.verts [sideVertTable [nSplineSide1][j]] = segmentManager.GetSegment (nSplineSeg2)->m_info.verts [sideVertTable [nSplineSide2][MatchingSide (j)]];
				segP->m_info.verts [oppSideVertTable [nSplineSide1][j]] = nVertex + 4 - j;
				}
			}
		}
	}

  // int twisted segments
for (i = 0; i < n_splines; i++)
	UntwistSegment ((MAX_SEGMENTS - 1) - i, nSplineSide1);
}

//------------------------------------------------------------------------------
//eof tunnelgen.cpp