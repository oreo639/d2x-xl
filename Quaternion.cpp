
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

CQuaternion CQuaternion::operator* (CQuaternion other)
{
	// the constructor takes its arguments as (x, y, z, w)
return CQuaternion (w * other.x + x * other.w + y * other.z - z * other.y,
                    w * other.y + y * other.w + z * other.x - x * other.z,
                    w * other.z + z * other.w + x * other.y - y * other.x,
                    w * other.w - x * other.x - y * other.y - z * other.z);
}

// -----------------------------------------------------------------------------

// Multiplying a quaternion q with a vector v applies the q-rotation to v
CDoubleVector CQuaternion::operator* (CDoubleVector v)
{
v.Normalize ();
 
CQuaternion vecQuat, resQuat;
vecQuat.x = v.v.x;
vecQuat.y = v.v.y;
vecQuat.z = v.v.z;
vecQuat.w = 0.0;
 
resQuat = vecQuat * GetConjugate ();
resQuat = *this * resQuat;
 
return (CDoubleVector (resQuat.x, resQuat.y, resQuat.z));
}

// -----------------------------------------------------------------------------

CQuaternion& CQuaternion::FromAxisAngle (CDoubleVector axis, double angle)
{
axis.Normalize ();
angle *= 0.5;
double sina = sin (angle);
x = axis.v.x * sina;
y = axis.v.y * sina;
z = axis.v.z * sina;
w = cos (angle);
Normalize ();
return *this;
}

// -----------------------------------------------------------------------------

CQuaternion& CQuaternion::FromEuler (double pitch, double yaw, double roll)
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
return *this;
}

// -----------------------------------------------------------------------------

static inline double Sign (double x) {return (x >= 0.0f) ? +1.0f : -1.0f;}

CQuaternion& CQuaternion::FromMatrix (CDoubleMatrix& m)
{
#if 1

	double s, t = m.m.rVec.v.x + m.m.uVec.v.y + m.m.fVec.v.z + 1.0;
	
if (t > 0.0) {
	s = 0.5 / sqrt (t);
   w = 0.25 / s;
   x = (m.m.fVec.v.y - m.m.uVec.v.z) * s;
   y = (m.m.rVec.v.z - m.m.fVec.v.x) * s;
   z = (m.m.uVec.v.x - m.m.rVec.v.y) * s;
	}
else {
	int column = (m.m.rVec.v.x >= m.m.uVec.v.y) ? (m.m.rVec.v.x >= m.m.fVec.v.z) ? 0 : 2 : (m.m.uVec.v.y >= m.m.fVec.v.z) ? 1 : 2;
	if (column == 0) {
      s = sqrt (1.0 + m.m.rVec.v.x - m.m.uVec.v.y - m.m.fVec.v.z) * 2;
      x = 0.5 / s;
      y = (m.m.rVec.v.y + m.m.uVec.v.x) / s;
      z = (m.m.rVec.v.z + m.m.fVec.v.y) / s;
      w = (m.m.uVec.v.z + m.m.fVec.v.z) / s;
		}
	else if (column == 1) {
      s = sqrt (1.0 + m.m.uVec.v.y - m.m.rVec.v.x - m.m.fVec.v.z) * 2;
      x = (m.m.rVec.v.y + m.m.uVec.v.x) / s;
      y = 0.5 / s;
      z = (m.m.uVec.v.z + m.m.fVec.v.z) / s;
      w = (m.m.rVec.v.z + m.m.fVec.v.y) / s;
		}
	else {
      s = sqrt (1.0 + m.m.fVec.v.z - m.m.rVec.v.x - m.m.uVec.v.y) * 2;
      x = (m.m.rVec.v.z + m.m.fVec.v.y) / s;
      y = (m.m.uVec.v.z + m.m.fVec.v.z) / s;
      z = 0.5 / s;
      w = (m.m.rVec.v.y + m.m.uVec.v.x) / s;
		}
	}

#else

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

#endif

Normalize ();
return *this;
}

// -----------------------------------------------------------------------------

CDoubleMatrix& CQuaternion::ToMatrix (CDoubleMatrix& m)
{
double x2 = x * x * 2.0;
double y2 = y * y * 2.0;
double z2 = z * z * 2.0;
double xy = x * y * 2.0;
double xz = x * z * 2.0;
double yz = y * z * 2.0;
double xw = x * w * 2.0;
double yw = y * w * 2.0;
double zw = z * w * 2.0;
 
// This calculation would be a lot more complicated for non-unit length quaternions
// Note: The constructor of Matrix4 expects the Matrix in column-major format like expected by
//   OpenGL
m.Set (1.0 - y2 - z2, xy - zw, xz + yw, 
		 xy + zw, 1.0 - x2 - z2, yz - xw, 
		 xz - yw, yz + xw, 1.0 - x2 - y2);
return m;
}

// -----------------------------------------------------------------------------

void CQuaternion::ToAxisAngle (CDoubleVector& axis, double& angle)
{
#if 1

Normalize ();

angle = acos (w) * 2.0 * PI;
double sina = sqrt (1.0 - w * w);
if (fabs (sina) < 1e-10)
	sina = 1.0;
axis.v.x = x / sina;
axis.v.y = y / sina;
axis.v.z = z / sina;

#else

double scale = sqrt (x * x + y * y + z * z);
axis.v.x = x / scale;
axis.v.y = y / scale;
axis.v.z = z / scale;
angle = acos (w) * 2.0;

#endif
}

// -----------------------------------------------------------------------------
#if 0 // usage example
void Camera::movex(float xmmod)
{
	pos += rotation * Vector3(xmmod, 0.0f, 0.0f);
}
 
void Camera::movey(float ymmod)
{
	pos.y -= ymmod;
}
 
void Camera::movez(float zmmod)
{
	pos += rotation * Vector3(0.0f, 0.0f, -zmmod);
}
 
void Camera::rotatex(float xrmod)
{
	Quaternion nrot(Vector3(1.0f, 0.0f, 0.0f), xrmod * PIOVER180);
	rotation = rotation * nrot;
}
 
void Camera::rotatey(float yrmod)
{
	Quaternion nrot(Vector3(0.0f, 1.0f, 0.0f), yrmod * PIOVER180);
	rotation = nrot * rotation;
}
 
void Camera::tick(float seconds)
{
	if (xrot != 0.0f) rotatex(xrot * seconds * rotspeed);
	if (yrot != 0.0f) rotatey(yrot * seconds * rotspeed);
 
	if (xmov != 0.0f) movex(xmov * seconds * movespeed);
	if (ymov != 0.0f) movey(ymov * seconds * movespeed);
	if (zmov != 0.0f) movez(zmov * seconds * movespeed);
}
#endif
// -----------------------------------------------------------------------------
