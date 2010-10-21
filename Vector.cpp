// Copyright (C) 1997 Bryan Aamot

#include "stdafx.h"

#include <math.h>
#include "define.h"
#include "types.h"
#include "mine.h"
#include "global.h"
#include "matrix.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
#if 0
int CFixVector::Read (CFileManager* fp) 
{ 
x = fp->ReadInt32 ();
y = fp->ReadInt32 ();
z = fp->ReadInt32 ();
return 1;
}

void CFixVector::Write (CFileManager* fp) 
{ 
WriteInt32 (x, fp);
WriteInt32 (y, fp);
WriteInt32 (z, fp);
}
#endif
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CFixVector::CFixVector (CDoubleVector& _v) { 
	v.x = D2X (_v.v.x), v.y = D2X (_v.v.y), v.z = D2X (_v.v.z); 
}

// -----------------------------------------------------------------------------
//  Rotates a vertex around a center point perpendicular to direction vector.
// -----------------------------------------------------------------------------

void CFixVector::Rotate (CFixVector& origin, CFixVector& normal, double angle) 
{

  double				zSpin, ySpin, h;
  CDoubleVector	v0, v1, v2, v3, vn;

  // translate coordanites to origin
v0 = CDoubleVector (*this - origin);
vn = CDoubleVector (normal - origin);

// calculate angles to normalize direction
// spin on z axis to get into the x-z plane
zSpin = (vn.v.y == vn.v.x) ? PI/4 : atan2 (vn.v.y, vn.v.x);
h = vn.v.x * cos (zSpin) + vn.v.y * sin (zSpin);
// spin on y to get on the x axis
ySpin = (vn.v.z == h) ? PI/4 : -atan2(vn.v.z, h);

// normalize vertex (spin on z then y)
v1.Set (v0.v.x * cos (zSpin) + v0.v.y * sin (zSpin), v0.v.y * cos (zSpin) - v0.v.x * sin (zSpin), v0.v.z);
v2.Set (v1.v.x * cos (ySpin) - v1.v.z * sin (ySpin), v1.v.y, v1.v.x * sin (ySpin) + v1.v.z * cos (ySpin));
v3.Set (v2.v.x, v2.v.y * cos (angle) + v2.v.z * sin (angle), v2.v.z * cos (angle) - v2.v.y * sin (angle));
// spin back in negative direction (y first then z)
ySpin = -ySpin;
v2.Set (v3.v.x * cos (ySpin) - v3.v.z * sin (ySpin), v3.v.y, v3.v.x * sin (ySpin) + v3.v.z * cos (ySpin));
zSpin = -zSpin;
v1.Set (v2.v.x * cos (zSpin) + v2.v.y * sin (zSpin), v2.v.y * cos (zSpin) - v2.v.x * sin (zSpin), v2.v.z);

v1 += origin;
*this = CFixVector (v1);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CDoubleVector::CDoubleVector (CFixVector _v) { 
	v.x = X2D (_v.v.x), v.y = X2D (_v.v.y), v.z = X2D (_v.v.z); 
}

// -----------------------------------------------------------------------------
//  Rotates a vertex around a center point perpendicular to direction vector.
// -----------------------------------------------------------------------------

void CDoubleVector::Rotate (CDoubleVector& origin, CDoubleVector& normal, double angle) 
{

  double				zSpin, ySpin, h;
  CDoubleVector	v0, v1, v2, v3, vn;

  // translate coordanites to origin
v0 = CDoubleVector (*this - origin);
vn = CDoubleVector (normal - origin);

// calculate angles to normalize direction
// spin on z axis to get into the x-z plane
zSpin = (vn.v.y == vn.v.x) ? PI/4 : atan2 (vn.v.y, vn.v.x);
h = vn.v.x * cos (zSpin) + vn.v.y * sin (zSpin);
// spin on y to get on the x axis
ySpin = (vn.v.z == h) ? PI/4 : -atan2(vn.v.z, h);

// normalize vertex (spin on z then y)
v1.Set (v0.v.x * cos (zSpin) + v0.v.y * sin (zSpin), v0.v.y * cos (zSpin) - v0.v.x * sin (zSpin), v0.v.z);
v2.Set (v1.v.x * cos (ySpin) - v1.v.z * sin (ySpin), v1.v.y, v1.v.x * sin (ySpin) + v1.v.z * cos (ySpin));
v3.Set (v2.v.x, v2.v.y * cos (angle) + v2.v.z * sin (angle), v2.v.z * cos (angle) - v2.v.y * sin (angle));
// spin back in negative direction (y first then z)
ySpin = -ySpin;
v2.Set (v3.v.x * cos (ySpin) - v3.v.z * sin (ySpin), v3.v.y, v3.v.x * sin (ySpin) + v3.v.z * cos (ySpin));
zSpin = -zSpin;
v1.Set (v2.v.x * cos (zSpin) + v2.v.y * sin (zSpin), v2.v.y * cos (zSpin) - v2.v.x * sin (zSpin), v2.v.z);

v1 += origin;
*this = v1;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// eof