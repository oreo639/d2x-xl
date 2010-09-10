// Copyright (C) 1997 Bryan Aamot

#include "stdafx.h"

#include <math.h>
#include "define.h"
#include "types.h"
#include "mine.h"
#include "global.h"
#include "matrix.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CFixMatrix::CFixMatrix ()
{
rVec.Set (F1_0, 0, 0);
uVec.Set (0, F1_0, 0);
fVec.Set (0, 0, F1_0);
}

// -----------------------------------------------------------------------------

CFixMatrix::CFixMatrix (int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3)
{
Set (x1, y1, z1, x2, y2, z2, x3, y3, z3);
}

// -----------------------------------------------------------------------------

CFixMatrix& CFixMatrix::Set (int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3)
{
rVec.Set (x1, y1, z1);
uVec.Set (x2, y2, z2);
fVec.Set (x3, y3, z3);
return *this;
}

// -----------------------------------------------------------------------------

CFixMatrix::CFixMatrix (int sinp, int cosp, int sinb, int cosb, int sinh, int cosh)
{
Set (sinp, cosp, sinb, cosb, sinh, cosh);
}

// -----------------------------------------------------------------------------

CFixMatrix& CFixMatrix::Set (int sinp, int cosp, int sinb, int cosb, int sinh, int cosh)
{
double sbsh = sinb * sinh;
double cbch = cosb * cosh;
double cbsh = cosb * sinh;
double sbch = sinb * cosh;
rVec.v.x = (int) (cbch + sinp * sbsh);
uVec.v.z = (int) (sbsh + sinp * cbch);
uVec.v.x = (int) (sinp * cbsh - sbch);
rVec.v.z = (int) (sinp * sbch - cbsh);
fVec.v.x = (int) (sinh * cosp);		
rVec.v.y = (int) (sinb * cosp);		
uVec.v.y = (int) (cosb * cosp);		
fVec.v.z = (int) (cosh * cosp);		
fVec.v.y = (int) -sinp;				
return *this;
}

// -----------------------------------------------------------------------------

CFixMatrix CFixMatrix::Mul (const CFixMatrix& other) 
{
	CFixVector v;
	CFixMatrix m;

v.Set (other.rVec.v.x, other.uVec.v.x, other.fVec.v.x);
m.rVec.v.x = v ^ rVec;
m.uVec.v.x = v ^ uVec;
m.fVec.v.x = v ^ fVec;
v.Set (other.rVec.v.y, other.uVec.v.y, other.fVec.v.y);
m.rVec.v.y = v ^ rVec;
m.uVec.v.y = v ^ uVec;
m.fVec.v.y = v ^ fVec;
v.Set (other.rVec.v.z, other.uVec.v.z, other.fVec.v.z);
m.rVec.v.z = v ^ rVec;
m.uVec.v.z = v ^ uVec;
m.fVec.v.z = v ^ fVec;
return m;
}

// -----------------------------------------------------------------------------

const int CFixMatrix::Det (void) 
{
int det = FixMul (rVec.v.x, FixMul (uVec.v.y, fVec.v.z) - FixMul (uVec.v.z, fVec.v.y));
det += FixMul (rVec.v.y, FixMul (uVec.v.z, fVec.v.x) - FixMul (uVec.v.x, fVec.v.z));
det += FixMul (rVec.v.z, FixMul (uVec.v.x, fVec.v.y) - FixMul (uVec.v.y, fVec.v.x));
return det;
}

// -----------------------------------------------------------------------------

const CFixMatrix CFixMatrix::Inverse (void) 
{
	CFixMatrix m;

int det = Det ();
if (det != 0) {
	m.rVec.v.x = FixDiv (FixMul (uVec.v.y, fVec.v.z) - FixMul (uVec.v.z, fVec.v.y), det);
	m.rVec.v.y = FixDiv (FixMul (rVec.v.z, fVec.v.y) - FixMul (rVec.v.y, fVec.v.z), det);
	m.rVec.v.z = FixDiv (FixMul (rVec.v.y, uVec.v.z) - FixMul (rVec.v.z, uVec.v.y), det);
	m.uVec.v.x = FixDiv (FixMul (uVec.v.z, fVec.v.x) - FixMul (uVec.v.x, fVec.v.z), det);
	m.uVec.v.y = FixDiv (FixMul (rVec.v.x, fVec.v.z) - FixMul (rVec.v.z, fVec.v.x), det);
	m.uVec.v.z = FixDiv (FixMul (rVec.v.z, uVec.v.x) - FixMul (rVec.v.x, uVec.v.z), det);
	m.fVec.v.x = FixDiv (FixMul (uVec.v.x, fVec.v.y) - FixMul (uVec.v.y, fVec.v.x), det);
	m.fVec.v.y = FixDiv (FixMul (rVec.v.y, fVec.v.x) - FixMul (rVec.v.x, fVec.v.y), det);
	m.fVec.v.z = FixDiv (FixMul (rVec.v.x, uVec.v.y) - FixMul (rVec.v.y, uVec.v.x), det);
	}
return m;
}

// -----------------------------------------------------------------------------

CFixMatrix& Transpose (CFixMatrix& dest, CFixMatrix& src)
{
dest.rVec.v.x = src.rVec.v.x;
dest.uVec.v.x = src.rVec.v.y;
dest.fVec.v.x = src.rVec.v.z;
dest.rVec.v.y = src.uVec.v.x;
dest.uVec.v.y = src.uVec.v.y;
dest.fVec.v.y = src.uVec.v.z;
dest.rVec.v.z = src.fVec.v.x;
dest.uVec.v.z = src.fVec.v.y;
dest.fVec.v.z = src.fVec.v.z;
return dest;
}

// -----------------------------------------------------------------------------

void CFixMatrix::Rotate (double angle, char axis) 
{
double cosX = cos (angle);
double sinX = sin (angle);

CFixMatrix mRot;

switch (axis) {
	case 'x':
		// spin x
		//	1	0	0
		//	0	cos	sin
		//	0	-sin	cos
		//
		mRot.uVec.Set ((int) (uVec.v.x * cosX + fVec.v.x * sinX), 
					      (int) (uVec.v.y * cosX + fVec.v.y * sinX),
						   (int) (uVec.v.z * cosX + fVec.v.z * sinX));
		mRot.fVec.Set ((int) (-uVec.v.x * sinX + fVec.v.x * cosX),
							(int) (-uVec.v.y * sinX + fVec.v.y * cosX),
							(int) (-uVec.v.z * sinX + fVec.v.z * cosX));
		uVec = mRot.uVec;
		fVec = mRot.fVec;
		break;
	case 'y':
		// spin y
		//	cos	0	-sin
		//	0	1	0
		//	sin	0	cos
		//
		mRot.rVec.Set ((int) (rVec.v.x * cosX - fVec.v.x * sinX), 
							(int) (rVec.v.y * cosX - fVec.v.y * sinX), 
							(int) (rVec.v.z * cosX - fVec.v.z * sinX));
		mRot.fVec.Set ((int) (rVec.v.x * sinX + fVec.v.x * cosX), 
							(int) (rVec.v.y * sinX + fVec.v.y * cosX),
							(int) (rVec.v.z * sinX + fVec.v.z * cosX));
		rVec = mRot.rVec;
		fVec = mRot.fVec;
		break;
	case 'z':
		// spin z
		//	cos	sin	0
		//	-sin	cos	0
		//	0	0	1
		mRot.rVec.Set ((int) (rVec.v.x * cosX + uVec.v.x * sinX),
							(int) (rVec.v.y * cosX + uVec.v.y * sinX),
							(int) (rVec.v.z * cosX + uVec.v.z * sinX));
		mRot.uVec.Set ((int) (-rVec.v.x * sinX + uVec.v.x * cosX),
							(int) (-rVec.v.y * sinX + uVec.v.y * cosX),
							(int) (-rVec.v.z * sinX + uVec.v.z * cosX));
		rVec = mRot.rVec;
		uVec = mRot.uVec;
		break;
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CDoubleMatrix::Clear (void)
{
rVec.Set (1.0, 0.0, 0.0);
uVec.Set (0.0, 1.0, 0.0);
fVec.Set (0.0, 0.0, 1.0);
}

// -----------------------------------------------------------------------------

CDoubleMatrix::CDoubleMatrix (double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
{
Set (x1, y1, z1, x2, y2, z2, x3, y3, z3);
}

// -----------------------------------------------------------------------------

CDoubleMatrix& CDoubleMatrix::Set (double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
{
rVec.Set (x1, y1, z1);
uVec.Set (x2, y2, z2);
fVec.Set (x3, y3, z3);
return *this;
}

// -----------------------------------------------------------------------------

CDoubleMatrix::CDoubleMatrix (double sinp, double cosp, double sinb, double cosb, double sinh, double cosh)
{
Set (sinp, cosp, sinb, cosb, sinh, cosh);
}

// -----------------------------------------------------------------------------

CDoubleMatrix& CDoubleMatrix::Set (double sinp, double cosp, double sinb, double cosb, double sinh, double cosh)
{
double sbsh = sinb * sinh;
double cbch = cosb * cosh;
double cbsh = cosb * sinh;
double sbch = sinb * cosh;

rVec.Set (cbch + sinp * sbsh, sinb * cosp, sinp * sbch - cbsh);
uVec.Set (sinp * cbsh - sbch, cosb * cosp, sbsh + sinp * cbch);
fVec.Set (sinh * cosp, cosh * cosp, -sinp);
return *this;
}

// -----------------------------------------------------------------------------

CDoubleMatrix CDoubleMatrix::Mul (const CDoubleMatrix& other) 
{
	CDoubleVector v;
	CDoubleMatrix m;

v.Set (other.rVec.v.x, other.uVec.v.x, other.fVec.v.x);
m.rVec.v.x = v ^ rVec;
m.uVec.v.x = v ^ uVec;
m.fVec.v.x = v ^ fVec;
v.Set (other.rVec.v.y, other.uVec.v.y, other.fVec.v.y);
m.rVec.v.y = v ^ rVec;
m.uVec.v.y = v ^ uVec;
m.fVec.v.y = v ^ fVec;
v.Set (other.rVec.v.z, other.uVec.v.z, other.fVec.v.z);
m.rVec.v.z = v ^ rVec;
m.uVec.v.z = v ^ uVec;
m.fVec.v.z = v ^ fVec;
return m;
}

// -----------------------------------------------------------------------------

const double CDoubleMatrix::Det (void) 
{
return rVec.v.x * (fVec.v.y * uVec.v.z - uVec.v.y * fVec.v.z) +
		 uVec.v.x * (rVec.v.y * fVec.v.z - fVec.v.y * rVec.v.z) +
		 fVec.v.x * (uVec.v.y * rVec.v.z - rVec.v.y * uVec.v.z);
}

// -----------------------------------------------------------------------------

const CDoubleMatrix CDoubleMatrix::Inverse (void) 
{
	CDoubleMatrix m;

double det = Det ();
if (det != 0.0) {
	m.rVec.v.x = (fVec.v.y * uVec.v.z - uVec.v.y * fVec.v.z) / det;
	m.rVec.v.y = (rVec.v.y * fVec.v.z - fVec.v.y * rVec.v.z) / det;
	m.rVec.v.z = (uVec.v.y * rVec.v.z - rVec.v.y * uVec.v.z) / det;

	m.uVec.v.x = (uVec.v.x * fVec.v.z - fVec.v.x * uVec.v.z) / det;
	m.uVec.v.y = (fVec.v.x * rVec.v.z - rVec.v.x * fVec.v.z) / det;
	m.uVec.v.z = (rVec.v.x * uVec.v.z - uVec.v.x * rVec.v.z) / det;

	m.fVec.v.x = (fVec.v.x * uVec.v.y - uVec.v.x * fVec.v.y) / det;
	m.fVec.v.y = (rVec.v.x * fVec.v.y - fVec.v.x * rVec.v.y) / det;
	m.fVec.v.z = (uVec.v.x * rVec.v.y - rVec.v.x * uVec.v.y) / det;
	}
return m;
}

// -----------------------------------------------------------------------------

const CDoubleMatrix CDoubleMatrix::Adjoint (void) 
{
	CDoubleMatrix m;

m.rVec.v.x = uVec.v.y * fVec.v.z - uVec.v.z * fVec.v.y;
m.rVec.v.y = rVec.v.z * fVec.v.y - rVec.v.y * fVec.v.z;
m.rVec.v.z = rVec.v.y * uVec.v.z - rVec.v.z * uVec.v.y;
m.uVec.v.x = uVec.v.z * fVec.v.x - uVec.v.x * fVec.v.z;
m.uVec.v.y = rVec.v.x * fVec.v.z - rVec.v.z * fVec.v.x;
m.uVec.v.z = rVec.v.z * uVec.v.x - rVec.v.x * uVec.v.z;
m.fVec.v.x = uVec.v.x * fVec.v.y - uVec.v.y * fVec.v.x;
m.fVec.v.y = rVec.v.y * fVec.v.x - rVec.v.x * fVec.v.y;
m.fVec.v.z = rVec.v.x * uVec.v.y - rVec.v.y * uVec.v.x;
return m;
}

// -----------------------------------------------------------------------------

CDoubleMatrix& Transpose (CDoubleMatrix& dest, CDoubleMatrix& src)
{
dest.rVec.v.x = src.rVec.v.x;
dest.uVec.v.x = src.rVec.v.y;
dest.fVec.v.x = src.rVec.v.z;
dest.rVec.v.y = src.uVec.v.x;
dest.uVec.v.y = src.uVec.v.y;
dest.fVec.v.y = src.uVec.v.z;
dest.rVec.v.z = src.fVec.v.x;
dest.uVec.v.z = src.fVec.v.y;
dest.fVec.v.z = src.fVec.v.z;
return dest;
}

// -----------------------------------------------------------------------------

void CDoubleMatrix::Rotate (double angle, char axis) 
{
double cosX = cos (angle);
double sinX = sin (angle);

CDoubleMatrix mRot;

switch (axis) {
	case 'x':
		// spin x
		//	1	0	0
		//	0	cos	sin
		//	0	-sin	cos
		//
		mRot.uVec.Set (uVec.v.x * cosX + fVec.v.x * sinX, 
					      uVec.v.y * cosX + fVec.v.y * sinX,
						   uVec.v.z * cosX + fVec.v.z * sinX);
		mRot.fVec.Set (-uVec.v.x * sinX + fVec.v.x * cosX,
							-uVec.v.y * sinX + fVec.v.y * cosX,
							-uVec.v.z * sinX + fVec.v.z * cosX);
		uVec = mRot.uVec;
		fVec = mRot.fVec;
		break;
	case 'y':
		// spin y
		//	cos	0	-sin
		//	0	1	0
		//	sin	0	cos
		//
		mRot.rVec.Set (rVec.v.x * cosX - fVec.v.x * sinX, 
							rVec.v.y * cosX - fVec.v.y * sinX, 
							rVec.v.z * cosX - fVec.v.z * sinX);
		mRot.fVec.Set (rVec.v.x * sinX + fVec.v.x * cosX, 
							rVec.v.y * sinX + fVec.v.y * cosX,
							rVec.v.z * sinX + fVec.v.z * cosX);
		rVec = mRot.rVec;
		fVec = mRot.fVec;
		break;
	case 'z':
		// spin z
		//	cos	sin	0
		//	-sin	cos	0
		//	0	0	1
		mRot.rVec.Set ((int) (rVec.v.x * cosX + uVec.v.x * sinX),
							(int) (rVec.v.y * cosX + uVec.v.y * sinX),
							(int) (rVec.v.z * cosX + uVec.v.z * sinX));
		mRot.uVec.Set ((int) (-rVec.v.x * sinX + uVec.v.x * cosX),
							(int) (-rVec.v.y * sinX + uVec.v.y * cosX),
							(int) (-rVec.v.z * sinX + uVec.v.z * cosX));
		rVec = mRot.rVec;
		uVec = mRot.uVec;
		break;
	}
}

//------------------------------------------------------------------------

void CDoubleMatrix::Square2Quad (POINT a [4]) 
{
// infer "unity square" to "quad" prespective transformation
// see page 55-56 of Digital Image Warping by George Wolberg (3rd edition) 
double dx1 = a [1].x - a [2].x;
double dx2 = a [3].x - a [2].x;
double dx3 = a [0].x - a [1].x + a [2].x - a [3].x;
double dy1 = a [1].y - a [2].y;
double dy2 = a [3].y - a [2].y;
double dy3 = a [0].y - a [1].y + a [2].y - a [3].y;
double w = (dx1 * dy2 - dx2 * dy1);
if (w == 0.0) 
	w = 1.0;

rVec.v.z = (dx3 * dy2 - dx2 * dy3) / w;
uVec.v.z = (dx1 * dy3 - dx3 * dy1) / w;
rVec.v.x = a [1].x - a [0].x + rVec.v.z * a [1].x;
uVec.v.x = a [3].x - a [0].x + uVec.v.z * a [3].x;
fVec.v.x = a [0].x;
rVec.v.y = a [1].y - a [0].y + rVec.v.z * a [1].y;
uVec.v.y = a [3].y - a [0].y + uVec.v.z * a [3].y;
fVec.v.y = a [0].y;
fVec.v.z = 1;
}

//------------------------------------------------------------------------------

void CDoubleMatrix::Scale (double scale) 
{
CDoubleMatrix s, m = *this;
s.Set (scale, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, 1.0);
*this = m * s;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CViewMatrix::CViewMatrix () 
{
Set (0,0,0,1,1,1,0,0,0);
m_scale [0] = 1;
m_angles [1][0] = 
m_angles [1][1] =
m_angles [1][2] = 0;
}

//------------------------------------------------------------------------------

void CViewMatrix::SetViewInfo (double depthPerception, short viewWidth, short viewHeight)
{
m_depthPerception = depthPerception;
m_viewWidth = viewWidth / 2;
m_viewHeight = viewHeight / 2;
}

//------------------------------------------------------------------------------

void CViewMatrix::Set (CDoubleVector vMove, CDoubleVector vSize, CDoubleVector vSpin)
{
m_move [0] = vMove;

CDoubleVector sinSpin (sin (vSpin.v.x), sin (vSpin.v.y), sin (vSpin.v.z));
CDoubleVector cosSpin (cos (vSpin.v.x), cos (vSpin.v.y), cos (vSpin.v.z));

m_mat [0].Set (vSize.v.x * cosSpin.v.z * cosSpin.v.y, 
				   vSize.v.y * sinSpin.v.z * cosSpin.v.y, 
				   -vSize.v.z * sinSpin.v.y,
				   -vSize.v.x * sinSpin.v.z * cosSpin.v.x + vSize.v.x * cosSpin.v.z * sinSpin.v.y * sinSpin.v.x,
				   vSize.v.y * sinSpin.v.z * sinSpin.v.y * sinSpin.v.x + vSize.v.y * cosSpin.v.z * cosSpin.v.x,
				   vSize.v.z * cosSpin.v.y * sinSpin.v.x,
				   vSize.v.x * cosSpin.v.z * sinSpin.v.y * cosSpin.v.x - vSize.v.x * sinSpin.v.z * sinSpin.v.x,
				   vSize.v.y * sinSpin.v.z * sinSpin.v.y * cosSpin.v.x - vSize.v.y * cosSpin.v.z * sinSpin.v.x,
				   vSize.v.z * cosSpin.v.y * cosSpin.v.x);
m_invMat [0] = m_mat [0].Inverse ();
m_invMove [0] = m_invMat [0] * m_move [0];
}

//------------------------------------------------------------------------------

void CViewMatrix::ClampAngle (int i)
{
if (m_angles [0][i] < 0)
	m_angles [0][i] += (int) (-m_angles [0][i] / 360) * 360;
else
	m_angles [0][i] -= (int) (m_angles [0][i] / 360) * 360;
}

//------------------------------------------------------------------------------

void CViewMatrix::RotateAngle (int i, double a)
{
m_angles [0][i] += a;
//ClampAngle (i);
}

//------------------------------------------------------------------------------

void CViewMatrix::Push (void)
{
m_mat [1] = m_mat [0];
m_invMat [1] = m_invMat [0];
m_move [1] = m_move [0];
m_invMove [1] = m_invMove [0];
memcpy (m_angles [0], m_angles [1], sizeof (m_angles [1]));
m_scale [1] = m_scale [0];
}

//------------------------------------------------------------------------------

void CViewMatrix::Pop (void)
{
m_mat [0] = m_mat [1];
m_invMat [0] = m_invMat [1];
m_move [0] = m_move [1];
m_invMove [0] = m_invMove [1];
memcpy (m_angles [1], m_angles [0], sizeof (m_angles [1]));
m_scale [0] = m_scale [1];
}

//------------------------------------------------------------------------------

void CViewMatrix::Unrotate (void)
{
#if 0
Rotate ('X', -m_angles [1][0]);
Rotate ('Y', -m_angles [1][1]);
Rotate ('Z', -m_angles [1][2]);
#else
Set (0,0,0,1,1,1,0,0,0);
#endif
Scale (m_scale [0]);
Calculate (m_move [1].v.x, m_move [1].v.y, m_move [1].v.z);
}

//------------------------------------------------------------------------------

void CViewMatrix::Rotate (char axis, double angle) 
{
if (angle) {
	double cosa = (double) cos (angle);
	double sina = (double) sin (angle);
	CDoubleMatrix m, r;
	switch(axis) {
		default:
		case 'X':
			m.Set (1.0, 0.0, 0.0, 0.0, cosa, sina, 0.0, -sina, cosa);
			RotateAngle (0, angle);
			break;
		case 'Y':
			m.Set (cosa, 0.0, -sina, 0.0, 1.0, 0.0, sina, 0.0, cosa);
			RotateAngle (1, angle);
			break;
		case 'Z':
			m.Set (cosa, sina, 0.0, -sina, cosa, 0.0, 0.0, 0.0, 1.0);
			RotateAngle (2, angle);
			break;
		}
	r = m_invMat [0];
	m_invMat [0] = r * m;
	}
}

//------------------------------------------------------------------------------

void CViewMatrix::Scale (double scale) 
{
CDoubleMatrix s (scale, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, scale);
CDoubleMatrix r = m_invMat [0];
m_invMat [0] = r * s;
m_scale [0] *= scale;
}

//------------------------------------------------------------------------------

void CViewMatrix::Calculate (double xMove, double yMove, double zMove) 
{
m_mat [0] = m_invMat [0].Inverse ();
m_move [0].Set (xMove, yMove, zMove);
}

//------------------------------------------------------------------------------

void CViewMatrix::CalculateInverse (double xMove, double yMove, double zMove) 
{
m_invMat [0] = m_mat [0].Inverse ();
m_invMove [0] = m_invMat [0] * m_move [0];
}

//------------------------------------------------------------------------------

void CViewMatrix::Project (CDoubleVector& vertex, APOINT& apoint) 
{
	CDoubleVector	r, v = vertex;

v += m_move [0];
r = m_mat [0] * v;
double scale = 5.0;
if ((m_depthPerception < 10000) && (r.v.z > - m_depthPerception)) 
	scale *= m_depthPerception / (r.v.z + m_depthPerception);
r *= CDoubleVector (scale, scale, 1.0);
apoint.x = (short) ((int) (r.v.x + m_viewWidth) % 32767);
apoint.y = (short) ((int) (m_viewHeight - r.v.y) % 32767);
apoint.z = (short) r.v.z;
}

//------------------------------------------------------------------------------

void CViewMatrix::Unproject (CVertex& vertex, APOINT& apoint) 
{
CDoubleVector v (double (apoint.x - x_center), double (y_center - apoint.y), double (apoint.z));
double scale = (v.v.z + depthPerception) / depthPerception / 5.0;
v *= CDoubleVector (scale, scale, 1.0);
CDoubleVector r = m_invMat [0] * v;
r -= m_move [0];
vertex = r;
}

//------------------------------------------------------------------------------
#if 0
int CViewMatrix::CheckNormal (CGameObject *objP, CVertex& a, CVertex& b) 
{
CVertex _a = objP->Orient () * a;
CVertex _b = objP->Orient () * b;
_a += objP->Position ();
_a += m_move [0];
_b += _a;
return Dot (m_mat [0].fVec, _a) > Dot (m_mat [0].fVec, _b);
}

// -----------------------------------------------------------------------------

int CViewMatrix::CheckNormal (CGameObject *objP, CFixVector& a, CFixVector& b) 
{
CVertex _a = objP->Orient () * CDoubleVector (a);
CVertex _b = objP->Orient () * CDoubleVector (b);
_a += objP->Position ();
_a += m_move [0];
_b += _a;
return Dot (m_mat [0].fVec, _a) > Dot (m_mat [0].fVec, _b);
}
#endif
// -----------------------------------------------------------------------------
// eof