
#include "stdafx.h"

#include <math.h>
#include "Quaternion.h"

// -----------------------------------------------------------------------------

void CQuaternion::Normalize (void)
{
	// Don't normalize if we don't have to
double mag2 = w * w + x * x + y * y + z * z;
if ((fabs (mag2) > 1e-30) && (fabs (mag2 - 1.0f) > 1e-30)) {
	double mag = sqrt (mag2);
	w /= mag;
	x /= mag;
	y /= mag;
	z /= mag;
	}
}

// -----------------------------------------------------------------------------

CQuaternion CQuaternion::operator* (CQuaternion rq)
{
	// the constructor takes its arguments as (x, y, z, w)
return CQuaternion (w * rq.x + x * rq.w + y * rq.z - z * rq.y,
                    w * rq.y + y * rq.w + z * rq.x - x * rq.z,
                    w * rq.z + z * rq.w + x * rq.y - y * rq.x,
                    w * rq.w - x * rq.x - y * rq.y - z * rq.z);
}

// -----------------------------------------------------------------------------

// Multiplying a quaternion q with a vector v applies the q-rotation to v
CDoubleVector CQuaternion::operator* (const CDoubleVector &vec)
{
CDoubleVector vn (vec);
vn.Normalize ();
 
CQuaternion vecQuat, resQuat;
vecQuat.x = vn.v.x;
vecQuat.y = vn.v.y;
vecQuat.z = vn.v.z;
vecQuat.w = 0.0f;
 
resQuat = vecQuat * GetConjugate ();
resQuat = *this * resQuat;
 
return (CDoubleVector(resQuat.x, resQuat.y, resQuat.z));
}

// -----------------------------------------------------------------------------

void CQuaternion::FromAxis (const CDoubleVector &v, double angle)
{
double sinAngle;
angle *= 0.5f;
CDoubleVector vn (v);
vn.Normalize ();
 
sinAngle = sin (angle);
 
x = (vn.v.x * sinAngle);
y = (vn.v.y * sinAngle);
z = (vn.v.z * sinAngle);
w = cos (angle);
}

// -----------------------------------------------------------------------------

void CQuaternion::FromEuler (double pitch, double yaw, double roll)
{
	// Basically we create 3 Quaternions, one for pitch, one for yaw, one for roll
	// and multiply those together.
	// the calculation below does the same, just shorter
 
double p = Radians (pitch) * 0.5;
double y = Radians (yaw) * 0.5;
double r = Radians (roll) * 0.5;
 
double sinp = sin (p);
double siny = sin (y);
double sinr = sin (r);
double cosp = cos (p);
double cosy = cos (y);
double cosr = cos (r);
 
x = sinr * cosp * cosy - cosr * sinp * siny;
y = cosr * sinp * cosy + sinr * cosp * siny;
z = cosr * cosp * siny - sinr * sinp * cosy;
w = cosr * cosp * cosy + sinr * sinp * siny;
 
Normalize ();
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------


