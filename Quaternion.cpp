
#include "stdafx.h"

#include <math.h>
#include "Quaternion.h"

// -----------------------------------------------------------------------------

void CQuaternion::Normalize (void)
{
	// Don't normalize if we don't have to
double mag2 = w * w + x * x + y * y + z * z;
if ((fabs (mag2) > 1e-30) && (fabs (mag2 - 1.0) > 1e-30)) {
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
vecQuat.w = 0.0;
 
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

static inline double Sign (double x) {return (x >= 0.0f) ? +1.0f : -1.0f;}

void CQuaternion::FromMatrix (CDoubleMatrix& m)
{
x = ( m.m.rVec.v.x + m.m.uVec.v.y + m.m.fVec.v.z + 1.0f) / 4.0f;
y = ( m.m.rVec.v.x - m.m.uVec.v.y - m.m.fVec.v.z + 1.0f) / 4.0f;
z = (-m.m.rVec.v.x + m.m.uVec.v.y - m.m.fVec.v.z + 1.0f) / 4.0f;
w = (-m.m.rVec.v.x - m.m.uVec.v.y + m.m.fVec.v.z + 1.0f) / 4.0f;
if (x < 0.0f) x = 0.0f;
if (y < 0.0f) y = 0.0f;
if (z < 0.0f) z = 0.0f;
if (w < 0.0f) w = 0.0f;
x = sqrt (x);
y = sqrt (y);
z = sqrt (z);
w = sqrt (w);
if (x >= y && x >= z && x >= w) {
    //x *= +1.0f;
    y *= Sign (m.m.fVec.v.y - m.m.uVec.v.z);
    z *= Sign (m.m.rVec.v.z - m.m.fVec.v.x);
    w *= Sign (m.m.uVec.v.x - m.m.rVec.v.y);
	}
else if (y >= x && y >= z && y >= w) {
    x *= Sign (m.m.fVec.v.y - m.m.uVec.v.z);
    //y *= +1.0f;
    z *= Sign (m.m.uVec.v.x + m.m.rVec.v.y);
    w *= Sign (m.m.rVec.v.z + m.m.fVec.v.x);
	} 
else if (z >= x && z >= y && z >= w) {
    x *= Sign (m.m.rVec.v.z - m.m.fVec.v.x);
    y *= Sign (m.m.uVec.v.x + m.m.rVec.v.y);
    //z *= +1.0f;
    w *= Sign (m.m.fVec.v.y + m.m.uVec.v.z);
	} 
else if (w >= x && w >= y && w >= z) {
    x *= Sign (m.m.uVec.v.x - m.m.rVec.v.y);
    y *= Sign (m.m.fVec.v.x + m.m.rVec.v.z);
    z *= Sign (m.m.fVec.v.y + m.m.uVec.v.z);
    //w *= +1.0f;
	}
Normalize ();
}

// -----------------------------------------------------------------------------

CDoubleMatrix CQuaternion::GetMatrix (void)
{
double x2 = x * x;
double y2 = y * y;
double z2 = z * z;
double xy = x * y;
double xz = x * z;
double yz = y * z;
double wx = w * x;
double wy = w * y;
double wz = w * z;
 
// This calculation would be a lot more complicated for non-unit length quaternions
// Note: The constructor of Matrix4 expects the Matrix in column-major format like expected by
//   OpenGL
return CDoubleMatrix (1.0 - 2.0 * (y2 + z2), 2.0 * (xy + wz), 2.0 * (xz - wy), 
							 2.0 * (xy - wz), 1.0 - 2.0 * (x2 + z2), 2.0 * (yz + wx), 
							 2.0 * (xz + wy), 2.0 * (yz - wx), 1.0 - 2.0 * (x2 + y2));
}

// -----------------------------------------------------------------------------

void CQuaternion::GetAxisAngle (CDoubleVector& axis, double& angle)
{
double scale = sqrt (x * x + y * y + z * z);
axis.v.x = x / scale;
axis.v.y = y / scale;
axis.v.z = z / scale;
angle = acos (w) * 2.0;
}

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------


