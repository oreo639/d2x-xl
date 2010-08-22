// Copyright (C) 1997 Bryan Aamot
#include "stdafx.h"
#include <stdio.h>
#include <math.h>
#include "stophere.h"
#include "define.h"
#include "types.h"
#include "mine.h"
#include "dle-xp.h"
#include "global.h"
#include "dlcdoc.h"
#include <signal.h>

#define CURRENT_POINT(a) ((Current ()->nPoint + (a))&0x03)
#define JOIN_DISTANCE (20*20*F1_0)

//-------------------------------------------------------------------------
// _matherr()
//
// defined to prevent crashes
//-------------------------------------------------------------------------
#if 0
char *whyS [] = {
    "argument domain error",
    "argument singularity ",
    "overflow range error ",
    "underflow range error",
    "total loss of significance",
    "partial loss of significance"
};

INT32 _matherr (struct exception *e) {
  sprintf_s (message, sizeof (message),"DMB has detected a math error\n"
		   "%s (%8g,%8g): %s\n\n"
		   "Press OK to continue, or Cancel to close DMB",
		    e->name, e->arg1, e->arg2, whyS [e->type - 1]);
  if (MessageBox(NULL, message,"Descent Level Editor XP - Error",
	    MB_ICONEXCLAMATION|MB_OKCANCEL+MB_TASKMODAL) == IDCANCEL) {
    if (QueryMsg("Are you sure you want to abort DLE-XP?") == IDYES) {
      return 0;
    }
  }
  return 1; // let program continue
}
#endif
//-------------------------------------------------------------------------
//   Faculty (q) - returns n! (n factorial)
//-------------------------------------------------------------------------
long Faculty (INT32 n) {
  long i;
  INT32 j;

  i=1;
  for (j=n;j>=2;j--) {
    i *= j;
  }
  return i;
}

//-------------------------------------------------------------------------
//   Coeff(n,i) - returns n!/(i!*(n-i)!)
//-------------------------------------------------------------------------
double Coeff(INT32 n, INT32 i) {
  return ((double)Faculty (n)/((double)Faculty (i)*(double)Faculty (n-i)));
}

//-------------------------------------------------------------------------
//   Blend(i,n,u) - returns a weighted coefficient for each point in a spline
//-------------------------------------------------------------------------
double Blend(INT32 i, INT32 n, double u) {
  double partial;
  INT32 j;

  partial = Coeff(n,i);
  for(j=1;j<=i;j++) {
    partial *= u;
  }
  for(j=1;j<=(n-i);j++) {
    partial *= (1-u);
  }
  return partial;
}

//-------------------------------------------------------------------------
//   BezierFcn(pt,u,n,p [][]) - sets (x,y,z) for u=#/segs based on points p
//-------------------------------------------------------------------------
CFixVector BezierFcn (double u, INT32 npts, CFixVector* p) 
{
CFixVector	v;

for (INT32 i = 0; i < npts; i++) {
	double b = Blend (i, npts - 1, u);
	v.v.x += FIX (Round (p [i].v.x * b));
	v.v.y += FIX (Round (p [i].v.y * b));
	v.v.z += FIX (Round (p [i].v.z * b));
	}
return v;
}

//--------------------------------------------------------------------------
// UntwistSegment ()
//
// Action - swaps vertices of opposing side if cube is twisted
//--------------------------------------------------------------------------

void CMine::UntwistSegment (INT16 nSegment,INT16 nSide) 
{
  double len, min_len;
  INT16 index,j;
  CSegment *segP;
  INT16 verts [4];

segP = Segments (nSegment);
// calculate length from point 0 to opp_points
for (j=0;j<4;j++) {
	len = CalcLength (Vertices (segP->verts [side_vert [nSide][0]]),
							Vertices (segP->verts [opp_side_vert [nSide][j]]));
	if (j==0) {
		min_len = len;
		index = 0;
		}
	else if (len < min_len) {
		min_len = len;
		index = j;
		}
	}
// swap verts if index != 0
if (index != 0) {
	for (j=0;j<4;j++)
		verts [j] = segP->verts [opp_side_vert [nSide][(j+index)%4]];
	for (j=0;j<4;j++)
		segP->verts [opp_side_vert [nSide][j]] = verts [j];
	}
}

//--------------------------------------------------------------------------
// SpinPoint () - spin on y-axis then z-axis
//--------------------------------------------------------------------------

void SpinPoint (CFixVector* point, double y_spin, double z_spin) 
{
double tx,ty,tz;
tx = point->v.x * cos (y_spin) - point->v.z * sin (y_spin);
ty = point->v.y;
tz = point->v.x * sin (y_spin) + point->v.z * cos (y_spin);
point->v.x = (FIX) (tx * cos (z_spin) + ty * sin (z_spin));
point->v.y = (FIX) (-tx * sin (z_spin) + ty * cos (z_spin));
point->v.z = (FIX) (tz);
}

//--------------------------------------------------------------------------
// SpinBackPoint () - spin on z-axis then y-axis
//--------------------------------------------------------------------------

void SpinBackPoint (CFixVector* point, double y_spin, double z_spin) 
{
  double tx, ty, tz;

tx = point->v.x * cos (-z_spin) + point->v.y * sin (-z_spin);
ty = -point->v.x * sin (-z_spin) + point->v.y * cos (-z_spin);
tz = point->v.z;
point->v.x = (FIX) (tx * cos (-y_spin) - tz * sin (-y_spin));
point->v.y = (FIX) (ty);
point->v.z = (FIX) (tx * sin (-y_spin) + tz * cos (-y_spin));
}

//--------------------------------------------------------------------------
// MatchingSide ()
//
// Action - Returns matching side depending on the current points
//--------------------------------------------------------------------------

INT32 CMine::MatchingSide (INT32 j) 
{
  INT32 ret [4][4] = {{3,2,1,0},{2,1,0,3},{1,0,3,2},{0,3,2,1}};
  INT32 offset;

offset = (4 + Current1 ().nPoint - Current2 ().nPoint) % 4;
return ret [offset][j];
}

//--------------------------------------------------------------------------
//
// Action - Spins points which lie in the y-z plane orthagonal to a normal
//          Uses normal as center for translating points.
//
// Changes - Chooses axis to normalizes on based on "normal" direction
//--------------------------------------------------------------------------

CFixVector RectPoints (double angle, double radius, CFixVector* origin, CFixVector* normal) 
{
  double				y_spin,z_spin;
  char				spinAxis;
  CFixVector		v = *normal - *origin;
  CDoubleVector	n (v), v1, v2;

  // translate coordanites to orgin
  // choose rotation order
if (fabs(n.v.z) > fabs(n.v.y))
	spinAxis = 'Y';
else 
	spinAxis = 'Z';
spinAxis = 'Y';

// start by defining vertex in rectangular coordinates (xz plane)
v.Set (0, (FIX) (radius * cos (angle)), (FIX) (radius * sin (angle)));

switch(spinAxis) {
 case 'Y':
   // calculate angles to normalize direction
   // spin on y axis to get into the y-z plane
   y_spin = - atan3(n.v.z,n.v.x);
   v1.Set (n.v.x * cos (y_spin) - n.v.z * sin (y_spin), n.v.y, n.v.x * sin (y_spin) + n.v.z * cos (y_spin));
   // spin on z to get on the x axis
   z_spin = atan3(v1.v.y,v1.v.x);
   // spin vertex back in negative direction (z first then y)
   v2.Set ((double) v.v.x * cos (-z_spin) + (double) v.v.y * sin (-z_spin), (double) -v.v.x * sin (-z_spin) + (double) v.v.y * cos (-z_spin), double (v.v.z));
   v1.Set (v2.v.x * cos (-y_spin) - v2.v.z * sin (-y_spin), v2.v.y, v2.v.x * sin (-y_spin) + v2.v.z * cos (-y_spin));
   break;

 case 'Z':
   // calculate angles to normalize direction
   // spin on z axis to get into the x-z plane
   z_spin = atan3(n.v.y,n.v.x);
   v1.Set (n.v.x * cos (z_spin) + n.v.y * sin (z_spin), -n.v.x * sin (z_spin) + n.v.y * cos (z_spin), n.v.z);
   // spin on y to get on the x axis
   y_spin = -atan3(v1.v.z,v1.v.x);
   // spin vertex back in negative direction (y first then z)
   v2.Set ((double) v.v.x * cos (-y_spin) - (double) v.v.z * sin (-y_spin), double (v.v.y), (double) v.v.x * sin (-y_spin) + (double) v.v.z * cos (-y_spin));
   v1.Set (v2.v.x * cos (-z_spin) + v2.v.y * sin (-z_spin), -v2.v.x * sin (-z_spin) + v2.v.y * cos (-z_spin), v2.v.z);
   break;
	}
v = *normal + CFixVector (v1);
return v;
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void PolarPoints (double *angle, double *radius, CFixVector* vertex, CFixVector* orgin, CFixVector* normal) 
{
  double z_spin,y_spin;
  double vx,vy,vz,nx,ny,nz;
  double x1,y1,z1,y2,z2;
  // translate coordanites to orgin
  vx = vertex->v.x - orgin->v.x;
  vy = vertex->v.y - orgin->v.y;
  vz = vertex->v.z - orgin->v.z;
  nx = normal->v.x - orgin->v.x;
  ny = normal->v.y - orgin->v.y;
  nz = normal->v.z - orgin->v.z;

  // calculate angles to normalize direction
  // spin on z axis to get into the x-z plane
z_spin = atan3(ny,nx);
x1 = nx * cos (z_spin) + ny * sin (z_spin);
z1 = nz;

// spin on y to get on the x axis
y_spin = -atan3(z1,x1);
// spin vertex (spin on z then y)
x1 = vx * cos (z_spin) + vy * sin (z_spin);
y1 = -vx * sin (z_spin) + vy * cos (z_spin);
z1 = vz;
//  x2 =   x1 * cos (y_spin) - z1 * sin (y_spin);
y2 = y1;
z2 = x1 * sin (y_spin) + z1 * cos (y_spin);
// convert to polar
*radius = sqrt(y2*y2 + z2*z2);  // ignore any x offset
*angle = atan3(z2,y2);
}

//==========================================================================
// MENU - Tunnel Generator
//
// Action - This is like a super "join" feature which uses splines to
//          connect the cubes.  The number of cubes is determined
//          automatically.
//==========================================================================

void CMine::TunnelGenerator (void) 
{

//  UpdateUndoBuffer(0);

double		length;
INT32			i, j, nVertex, spline;
CSegment*	segP;

if (!m_bSplineActive) {
	m_nMaxSplines = MAX_SEGMENTS - SegCount ();
	if (m_nMaxSplines > MAX_SPLINES)
		m_nMaxSplines = MAX_SPLINES;
	else if (m_nMaxSplines < 3) {
//	if ((VertCount () + 3 /*MAX_SPLINES*/ * 4 > MAX_VERTICES) ||
//		 (SegCount () + 3 /*MAX_SPLINES*/ > MAX_SEGMENTS)) {
		ErrorMsg ("Insufficient number of free vertices and/or segments\n"
					"to use the tunnel generator.");
		return;
		}
	// make sure there are no children on either segment/side
	if ((Segments (Current1 ().nSegment)->children [Current1 ().nSide] != -1) ||
		 (Segments (Current2 ().nSegment)->children [Current2 ().nSide] != -1)) {
		ErrorMsg ("Starting and/or ending point of spline\n"
					"already have cube(s) attached.\n\n"
					"Hint: Put the current cube and the alternate cube\n"
					"on sides which do not have cubes attached.");
		return;
		}
	nSplineSeg1 = Current1 ().nSegment;
	nSplineSeg2 = Current2 ().nSegment;
	nSplineSide1 = Current1 ().nSide;
	nSplineSide2 = Current2 ().nSide;
	// define 4 data points for spline to work from
	// center of current cube
	points [0] = CalcSideCenter (nSplineSeg1, nSplineSide1);
	// center of other cube
	points [3] = CalcSideCenter (nSplineSeg2, nSplineSide2);
	// calculate length between cubes
	length = CalcLength (points, points + 3);
	// base spline length on distance between cubes
	m_splineLength1 = (INT16) (length / (3 * F1_0));
	if (m_splineLength1 < MIN_SPLINE_LENGTH)
		m_splineLength1 = MIN_SPLINE_LENGTH;
	if (m_splineLength1 > MAX_SPLINE_LENGTH)
		m_splineLength1 = MAX_SPLINE_LENGTH;
	m_splineLength2 = (INT16) (length / (3 * F1_0));
	if (m_splineLength2 < MIN_SPLINE_LENGTH)
		m_splineLength2 = MIN_SPLINE_LENGTH;
	if (m_splineLength2 > MAX_SPLINE_LENGTH)
		m_splineLength2 = MAX_SPLINE_LENGTH;
	if (length < 50 * F1_0) {
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
	m_bSplineActive = TRUE;
	}
else {
	m_bSplineActive = FALSE;
	// ask if user wants to keep the new spline
	if (Query2Msg ("Do you want to keep this tunnel?", MB_YESNO) != IDYES) {
		theApp.MineView ()->Refresh (false);
		return;
		}
	theApp.SetModified (TRUE);
	theApp.LockUndo ();
	for (spline = 0; spline < n_splines; spline++) {
		segP = Segments (SegCount ());
		// copy current segment
		*segP = *Segments (Current ()->nSegment);
		// use last "n_spline" segments
		nVertex = (MAX_VERTICES - 1) - spline * 4;
		for (j = 0; j < 4; j++) {
			if (VertCount () >= MAX_VERTICES)
				DEBUGMSG (" Tunnel generator: Vertex count out of range.")
			else if ((nVertex - j < 0) || (nVertex - j >= MAX_VERTICES))
				DEBUGMSG (" Tunnel generator: Vertex number out of range.")
			else
				memcpy (Vertices (VertCount ()), Vertices (nVertex - j), sizeof (*Vertices (0)));
/*
			vertices [VertCount ()].x = vertices [nVertex-j].x;
			vertices [VertCount ()].y = vertices [nVertex-j].y;
			vertices [VertCount ()].z = vertices [nVertex-j].z;
*/
			if (spline == 0) {         // 1st segment
				segP->verts [side_vert [nSplineSide1][j]] = VertCount ();
				segP->verts [opp_side_vert [nSplineSide1][j]] = Segments (nSplineSeg1)->verts [side_vert [nSplineSide1][j]];
				VertStatus (VertCount ()++) = 0;
				}
			else if(spline < n_splines - 1) { // center segments
				segP->verts [side_vert [nSplineSide1][j]] = VertCount ();
				segP->verts [opp_side_vert [nSplineSide1][j]] = VertCount () - 4;
				VertStatus (VertCount ()++) = 0;
				}
			else {          // last segment
				segP->verts [side_vert [nSplineSide1][j]] = Segments (nSplineSeg2)->verts [side_vert [nSplineSide2][MatchingSide (j)]];
				segP->verts [opp_side_vert [nSplineSide1][j]] = VertCount () - 4 + j;
				}
			}
		// fix twisted segments
		UntwistSegment (SegCount (), nSplineSide1);
		// define children and sides (textures and nWall)
		for (j = 0; j < 6; j++) {
			segP->children [j] = -1;
			segP->sides [j].nBaseTex = 0;
			segP->sides [j].nOvlTex = 0;
			segP->sides [j].nWall = NO_WALL;
			for (i = 0; i < 4; i++) {
//	    segP->sides [j].uvls [i].u = default_uvls [i].u;
//	    segP->sides [j].uvls [i].v = default_uvls [i].v;
				segP->sides [j].uvls [i].l = (UINT16) DEFAULT_LIGHTING;
				}
			SetUV (SegCount (),j,0,0,0);
			}
		if (spline==0) {
			segP->children [opp_side [nSplineSide1]] = nSplineSeg1;
			segP->children [nSplineSide1] = SegCount ()+1;
			Segments (nSplineSeg1)->children [nSplineSide1] = SegCount ();
			Segments (nSplineSeg1)->childFlags |= (1<<nSplineSide1);
			} 
		else if (spline<n_splines-1) {
			segP->children [opp_side [nSplineSide1]] = SegCount ()-1;
			segP->children [nSplineSide1] = SegCount ()+1;
			}
		else {
			segP->children [opp_side [nSplineSide1]] = SegCount ()-1;
			segP->children [nSplineSide1] = nSplineSeg2;
			Segments (nSplineSeg2)->children [nSplineSide2] = SegCount ();
			Segments (nSplineSeg2)->childFlags |= (1<<nSplineSide2);
			}
		// define child bitmask, special, matcen, value, and wall bitmask
		segP->childFlags = (1<<nSplineSide1) | (1<<opp_side [nSplineSide1]);
		segP->owner = -1;
		segP->group = -1;
		segP->function = 0;
		segP->nMatCen = -1;
		segP->value = -1;
		segP->wallFlags = 0; // make sure segment is not marked
		SegCount ()++;
		}
	theApp.UnlockUndo ();
	}
SetLinesToDraw ();
theApp.MineView ()->Refresh ();
}

//==========================================================================
// TMainWindow - CM_IncreaseSpline
//==========================================================================

void CMine::IncreaseSpline() 
{

//UpdateUndoBuffer(0);

if (Current ()->nSegment == nSplineSeg1)
	if (m_splineLength1 < (MAX_SPLINE_LENGTH-SPLINE_INTERVAL))
		m_splineLength1 += SPLINE_INTERVAL;
if (Current ()->nSegment == nSplineSeg2)
	if (m_splineLength2 < (MAX_SPLINE_LENGTH-SPLINE_INTERVAL))
		m_splineLength2 += SPLINE_INTERVAL;
theApp.MineView ()->Refresh ();
}

//==========================================================================
// TMainWindow - CM_DecreaseSpline
//==========================================================================

void CMine::DecreaseSpline() 
{

//  UpdateUndoBuffer(0);

if (Current ()->nSegment == nSplineSeg1)
	if (m_splineLength1 > (MIN_SPLINE_LENGTH+SPLINE_INTERVAL))
		m_splineLength1 -= SPLINE_INTERVAL;
if (Current ()->nSegment == nSplineSeg2)
	if (m_splineLength2 > (MIN_SPLINE_LENGTH+SPLINE_INTERVAL))
		m_splineLength2 -= SPLINE_INTERVAL;
theApp.MineView ()->Refresh ();
}

//-------------------------------------------------------------------------
// CalcSpline ()
//
// Note: this routine is called by dmb.cpp - update_display() everytime
//       the display is redrawn.
//-------------------------------------------------------------------------

void CMine::CalcSpline (void) 
{
  double length;
  double angle;
  INT32 i,j;
  CSegment *segP;
  INT16 nVertex;
  CFixVector vertex;
  double theta [2][4],radius [2][4]; // polor coordinates of sides
  double delta_angle [4];
  CFixVector rel_side_pts [2][4]; // side points reletave to center of side 1
  CFixVector rel_pts [4]; // 4 points of spline reletave to 1st point
  CFixVector rel_spline_pts [MAX_SPLINES];
  double y,z;
  double y_spin,z_spin;

  // center of both cubes
  points [0] = CalcSideCenter (nSplineSeg1, nSplineSide1);
  points [3] = CalcSideCenter (nSplineSeg2, nSplineSide2);

  // point orthogonal to center of current cube
  points [1] = CalcSideNormal (nSplineSeg1, nSplineSide1);
  points [1] *= FIX (m_splineLength1);
  points [1] += points [0];


  // point orthogonal to center of other cube
  points [2] = CalcSideNormal (nSplineSeg2, nSplineSide2);
  points [2] *= FIX (m_splineLength2);
  points [2] += points [3];

  // calculate number of segments (min=1)
  length = CalcLength (points,points + 3);
  n_splines = (INT32)(abs (m_splineLength1) + abs (m_splineLength2)) / 20 + (INT32)(length / (40.0 * (double) 0x10000L));
  n_splines = min (n_splines, m_nMaxSplines - 1);

  // calculate spline points
  for (i = 0; i <= n_splines; i++) {
    spline_points [i] = BezierFcn ((double) i / (double) n_splines, 4, points);
  }

  // make all points reletave to first face (translation)
  for (i = 0; i < 4; i++) {
    rel_pts [i] = points [i] - points [0];
  }
  segP = Segments (nSplineSeg1);
  CVertex* vert;
  for (i = 0; i < 4; i++) {
    nVertex = side_vert [nSplineSide1][i];
	 vert = Vertices (segP->verts [nVertex]);
    rel_side_pts [0][i] = *vert - points [0];
  }
  segP = Segments (nSplineSeg2);
  for (i = 0; i < 4; i++) {
    nVertex = side_vert [nSplineSide2][i];
	 vert = Vertices (segP->verts [nVertex]);
    rel_side_pts [1][i] = *vert - points [0];
  }
  for (i=0;i<n_splines;i++) {
    rel_spline_pts [i] = spline_points [i] - points [0];
  }

  // determine y-spin and z-spin to put 1st orthogonal vector onto the x-axis
  y_spin = - atan3(rel_pts [1].v.z, rel_pts [1].v.x                                     ); // to y-z plane
  z_spin =   atan3(rel_pts [1].v.y, rel_pts [1].v.x * cos (y_spin)-rel_pts [1].v.z * sin (y_spin)); // to x axis

  // spin all points reletave to first face (rotation)
  for (i = 0; i < 4; i++) {
    SpinPoint (rel_pts + i,y_spin,z_spin);
    for (j=0;j<2;j++) {
      SpinPoint (rel_side_pts [j] + i,y_spin,z_spin);
    }
  }
  for (i=0;i<n_splines;i++) {
    SpinPoint (rel_spline_pts + i,y_spin,z_spin);
  }

  // determine polar coordinates of the 1st side (simply y,z coords)
  for (i = 0; i < 4; i++) {
    theta [0][i] = atan3(rel_side_pts [0][i].v.z,rel_side_pts [0][i].v.y);
    y = (float) rel_side_pts [0][i].v.y;
    z = (float) rel_side_pts [0][i].v.z;
    radius [0][i] = sqrt(y*y + z*z);
  }

  // determine polar coordinates of the 2nd side by rotating to x-axis first
  for (i = 0; i < 4; i++) {
    // flip orthoginal vector to point into cube
    vertex.Set (-(rel_pts [2].v.x-rel_pts [3].v.x) + rel_pts [3].v.x,
					 -(rel_pts [2].v.y-rel_pts [3].v.y) + rel_pts [3].v.y,
					 -(rel_pts [2].v.z-rel_pts [3].v.z) + rel_pts [3].v.z);
    PolarPoints (&theta [1][i],&radius [1][i],&rel_side_pts [1][i],&rel_pts [3],&vertex);
  }

  // figure out the angle differences to be in range (-pi to pi)
  for (j=0;j<4;j++) {
    delta_angle [j] = theta [1][MatchingSide (j)] - theta [0][j];
    if (delta_angle [j] < M_PI) {
      delta_angle [j] += 2*M_PI;
    }
    if (delta_angle [j] > M_PI) {
      delta_angle [j] -= 2*M_PI;
    }
  }

  // make sure delta angles do not cross PI & -PI
  for (i=1;i<4;i++) {
    if (delta_angle [i] > delta_angle [0] + M_PI) delta_angle [i] -= 2*M_PI;
    if (delta_angle [i] < delta_angle [0] - M_PI) delta_angle [i] += 2*M_PI;
  }

#if 0
  sprintf_s (message, sizeof (message), "theta [0] = %d,%d,%d,%d\n"
                  "theta [1] = %d,%d,%d,%d\n"
		  "radius [0] = %d,%d,%d,%d\n"
		  "radius [1] = %d,%d,%d,%d\n"
		  "delta_angles = %d,%d,%d,%d\n"
		  "y_spin = %d\n"
		  "z_spin = %d",
		   (INT32)(theta [0][0]*180/M_PI),(INT32)(theta [0][1]*180/M_PI),(INT32)(theta [0][2]*180/M_PI),(INT32)(theta [0][3]*180/M_PI),
		   (INT32)(theta [1][0]*180/M_PI),(INT32)(theta [1][1]*180/M_PI),(INT32)(theta [1][2]*180/M_PI),(INT32)(theta [1][3]*180/M_PI),
		   (INT32)(radius [0][0]*180/M_PI),(INT32)(radius [0][1]*180/M_PI),(INT32)(radius [0][2]*180/M_PI),(INT32)(radius [0][3]*180/M_PI),
		   (INT32)(radius [1][0]*180/M_PI),(INT32)(radius [1][1]*180/M_PI),(INT32)(radius [1][2]*180/M_PI),(INT32)(radius [1][3]*180/M_PI),
		   (INT32)(delta_angle [0]*180/M_PI),(INT32)(delta_angle [1]*180/M_PI),(INT32)(delta_angle [2]*180/M_PI),(INT32)(delta_angle [3]*180/M_PI),
		   (INT32)(y_spin*180/M_PI),(INT32)(z_spin*180/M_PI)
		   );
  DebugMsg(message);
#endif

  // calculate segment verticies as weighted average between the two sides
  // then spin verticies in the direction of the segment vector
  for (i=0;i<n_splines;i++) {
    nVertex = (MAX_VERTICES-1)-(i*4);
    for (j=0;j<4;j++) {
	   vert = Vertices (nVertex - j);
      angle  = ((float)i/(float)n_splines) * delta_angle [j] + theta [0][j];
      length = ((float)i/(float)n_splines) * radius [1][MatchingSide (j)] + (((float)n_splines-(float)i)/(float)n_splines) * radius [0][j];
      *vert = RectPoints (angle, length, &rel_spline_pts [i], &rel_spline_pts [i+1]);
      // spin vertices
      SpinBackPoint (vert,y_spin,z_spin);
      // translate point back
      *vert += points [0];
    }
  }

  // define segment vert numbers
for (i=0;i<n_splines;i++) {
	// use last "n_spline" segments
	segP = Segments (MAX_SEGMENTS - 1 - i);
	nVertex = MAX_VERTICES - 1 - i * 4;
	for (j = 0; j < 4; j++) {
		if (i == 0) {         // 1st segment
			segP->verts [side_vert [nSplineSide1][j]] = nVertex - j;
			segP->verts [opp_side_vert [nSplineSide1][j]] = Segments (nSplineSeg1)->verts [side_vert [nSplineSide1][j]];
			}
		else {
			if(i < n_splines - 1) { // center segments
				segP->verts [side_vert [nSplineSide1][j]] = nVertex - j;
				segP->verts [opp_side_vert [nSplineSide1][j]] = nVertex + 4 - j;
				} 
			else {          // last segment
				segP->verts [side_vert [nSplineSide1][j]] = Segments (nSplineSeg2)->verts [side_vert [nSplineSide2][MatchingSide (j)]];
				segP->verts [opp_side_vert [nSplineSide1][j]] = nVertex + 4 - j;
				}
			}
		}
	}

  // fix twisted segments
for (i = 0; i < n_splines; i++)
	UntwistSegment ((MAX_SEGMENTS - 1) - i, nSplineSide1);
}

//eof tunnelgen.cpp