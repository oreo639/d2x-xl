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

CFixMatrix::CFixMatrix ()
{
rVec.Set (F1_0, 0, 0);
uVec.Set (0, F1_0, 0);
fVec.Set (0, 0, F1_0);
}

// -----------------------------------------------------------------------------

CFixMatrix::CFixMatrix (FIX x1, FIX y1, FIX z1, FIX x2, FIX y2, FIX z2, FIX x3, FIX y3, FIX z3)
{
Set (x1, y1, z1, x2, y2, z2, x3, y3, z3);
}

// -----------------------------------------------------------------------------

CFixMatrix& CFixMatrix::Set (FIX x1, FIX y1, FIX z1, FIX x2, FIX y2, FIX z2, FIX x3, FIX y3, FIX z3)
{
rVec.Set (x1, y1, z1);
uVec.Set (x2, y2, z2);
fVec.Set (x3, y3, z3);
return *this;
}

// -----------------------------------------------------------------------------

CFixMatrix::CFixMatrix (FIX sinp, FIX cosp, FIX sinb, FIX cosb, FIX sinh, FIX cosh)
{
Set (sinp, cosp, sinb, cosb, sinh, cosh);
}

// -----------------------------------------------------------------------------

CFixMatrix& CFixMatrix::Set (FIX sinp, FIX cosp, FIX sinb, FIX cosb, FIX sinh, FIX cosh)
{
double sbsh = sinb * sinh;
double cbch = cosb * cosh;
double cbsh = cosb * sinh;
double sbch = sinb * cosh;
rVec.v.x = (FIX) ( cbch + sinp * sbsh);
uVec.v.z = (FIX) ( sbsh + sinp * cbch);
uVec.v.x = (FIX) ( sinp * cbsh - sbch);
rVec.v.z = (FIX) ( sinp * sbch - cbsh);
fVec.v.x = (FIX) ( sinh * cosp);		
rVec.v.y = (FIX) ( sinb * cosp);		
uVec.v.y = (FIX) ( cosb * cosp);		
fVec.v.z = (FIX) ( cosh * cosp);		
fVec.v.y = (FIX) -sinp;				
return *this;
}

// -----------------------------------------------------------------------------

CFixMatrix CFixMatrix::Mul (const CFixMatrix& other) 
{
	CFixVector v;
	CFixMatrix m;

v.Set (rVec.v.x, uVec.v.x, fVec.v.x);
m.rVec.v.x = v ^ other.rVec;
m.uVec.v.x = v ^ other.uVec;
m.fVec.v.x = v ^ other.fVec;
v.Set (rVec.v.y, uVec.v.y, fVec.v.y);
m.rVec.v.y = v ^ other.rVec;
m.uVec.v.y = v ^ other.uVec;
m.fVec.v.y = v ^ other.fVec;
v.Set (rVec.v.z, uVec.v.z, fVec.v.z);
m.rVec.v.z = v ^ other.rVec;
m.uVec.v.z = v ^ other.uVec;
m.fVec.v.z = v ^ other.fVec;
return m;
}

// -----------------------------------------------------------------------------

const FIX CFixMatrix::Det (void) 
{
FIX det = FixMul (rVec.v.x, FixMul (uVec.v.y, fVec.v.z) - FixMul (uVec.v.z, fVec.v.y));
det += FixMul (rVec.v.y, FixMul (uVec.v.z, fVec.v.x) - FixMul (uVec.v.x, fVec.v.z));
det += FixMul (rVec.v.z, FixMul (uVec.v.x, fVec.v.y) - FixMul (uVec.v.y, fVec.v.x));
return det;
}

// -----------------------------------------------------------------------------

const CFixMatrix CFixMatrix::Inverse (void) 
{
	CFixMatrix m;

FIX det = Det ();
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
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CDoubleMatrix::CDoubleMatrix ()
{
rVec.Set (1.0, 0.0, 0.0);
uVec.Set (0.0, 1.0, 0.0);
fVec.Set (0.0, 0.0, 1.0);
}

// -----------------------------------------------------------------------------

CDoubleMatrix::CDoubleMatrix (DOUBLE x1, DOUBLE y1, DOUBLE z1, DOUBLE x2, DOUBLE y2, DOUBLE z2, DOUBLE x3, DOUBLE y3, DOUBLE z3)
{
Set (x1, y1, z1, x2, y2, z2, x3, y3, z3);
}

// -----------------------------------------------------------------------------

CDoubleMatrix& CDoubleMatrix::Set (DOUBLE x1, DOUBLE y1, DOUBLE z1, DOUBLE x2, DOUBLE y2, DOUBLE z2, DOUBLE x3, DOUBLE y3, DOUBLE z3)
{
rVec.Set (x1, y1, z1);
uVec.Set (x2, y2, z2);
fVec.Set (x3, y3, z3);
return *this;
}

// -----------------------------------------------------------------------------

CDoubleMatrix::CDoubleMatrix (DOUBLE sinp, DOUBLE cosp, DOUBLE sinb, DOUBLE cosb, DOUBLE sinh, DOUBLE cosh)
{
Set (sinp, cosp, sinb, cosb, sinh, cosh);
}

// -----------------------------------------------------------------------------

CDoubleMatrix& CDoubleMatrix::Set (DOUBLE sinp, DOUBLE cosp, DOUBLE sinb, DOUBLE cosb, DOUBLE sinh, DOUBLE cosh)
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

v.Set (rVec.v.x, uVec.v.x, fVec.v.x);
m.rVec.v.x = v ^ other.rVec;
m.uVec.v.x = v ^ other.uVec;
m.fVec.v.x = v ^ other.fVec;
v.Set (rVec.v.y, uVec.v.y, fVec.v.y);
m.rVec.v.y = v ^ other.rVec;
m.uVec.v.y = v ^ other.uVec;
m.fVec.v.y = v ^ other.fVec;
v.Set (rVec.v.z, uVec.v.z, fVec.v.z);
m.rVec.v.z = v ^ other.rVec;
m.uVec.v.z = v ^ other.uVec;
m.fVec.v.z = v ^ other.fVec;
return m;
}

// -----------------------------------------------------------------------------

const double CDoubleMatrix::Det (void) 
{
double det = rVec.v.x * (uVec.v.y * fVec.v.z - uVec.v.z * fVec.v.y);
det += rVec.v.y * (uVec.v.z * fVec.v.x - uVec.v.x * fVec.v.z);
det += rVec.v.z * (uVec.v.x * fVec.v.y - uVec.v.y * fVec.v.x);
return det;
}

// -----------------------------------------------------------------------------

const CDoubleMatrix CDoubleMatrix::Inverse (void) 
{
	CDoubleMatrix m;

double det = Det ();
if (det != 0.0) {
	m.rVec.v.x = (uVec.v.y * fVec.v.z - uVec.v.z * fVec.v.y) / det;
	m.rVec.v.y = (rVec.v.z * fVec.v.y - rVec.v.y * fVec.v.z) / det;
	m.rVec.v.z = (rVec.v.y * uVec.v.z - rVec.v.z * uVec.v.y) / det;
	m.uVec.v.x = (uVec.v.z * fVec.v.x - uVec.v.x * fVec.v.z) / det;
	m.uVec.v.y = (rVec.v.x * fVec.v.z - rVec.v.z * fVec.v.x) / det;
	m.uVec.v.z = (rVec.v.z * uVec.v.x - rVec.v.x * uVec.v.z) / det;
	m.fVec.v.x = (uVec.v.x * fVec.v.y - uVec.v.y * fVec.v.x) / det;
	m.fVec.v.y = (rVec.v.y * fVec.v.x - rVec.v.x * fVec.v.y) / det;
	m.fVec.v.z = (rVec.v.x * uVec.v.y - rVec.v.y * uVec.v.x) / det;
	}
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
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CViewMatrix::CViewMatrix () 
{
Set (0,0,0,1,1,1,0,0,0);
m_scale = 1;
m_angleSave [0] = 
m_angleSave [1] =
m_angleSave [2] = 0;
}

// -----------------------------------------------------------------------------
// SetViewInfo
// -----------------------------------------------------------------------------

void CViewMatrix::SetViewInfo (double depthPerception, INT16 viewWidth, INT16 viewHeight)
{
m_depthPerception = depthPerception;
m_viewWidth = viewWidth / 2;
m_viewHeight = viewHeight / 2;
}

//--------------------------------------------------------------------------
// Set()
//--------------------------------------------------------------------------

void CViewMatrix::Set (
	double xMove, double yMove, double zMove, 
	double xSize, double ySize, double zSize,
	double xSpin, double ySpin, double zSpin) 
{
m_move [0].Set (xMove, yMove, zMove);

CDoubleVector sinSpin (sin (xSpin), sin (ySpin), sin (zSpin));
CDoubleVector cosSpin (cos (xSpin), cos (ySpin), cos (zSpin));

m_mat [0].Set (xSize * cosSpin.v.z * cosSpin.v.y, 
			  ySize * sinSpin.v.z * cosSpin.v.y, 
			  -zSize * sinSpin.v.y,
			  -xSize * sinSpin.v.z * cosSpin.v.x + xSize * cosSpin.v.z * sinSpin.v.y * sinSpin.v.x,
			  ySize * sinSpin.v.z * sinSpin.v.y * sinSpin.v.x + ySize * cosSpin.v.z * cosSpin.v.x,
			  zSize * cosSpin.v.y * sinSpin.v.x,
			  xSize * cosSpin.v.z * sinSpin.v.y * cosSpin.v.x - xSize * sinSpin.v.z * sinSpin.v.x,
			  ySize * sinSpin.v.z * sinSpin.v.y * cosSpin.v.x - ySize * cosSpin.v.z * sinSpin.v.x,
			  zSize * cosSpin.v.y * cosSpin.v.x);
m_invMat [0] = m_mat [0].Inverse ();
m_invMove [0] = m_invMat [0] * m_move [0];
}

//--------------------------------------------------------------------------
// Rotate()
//--------------------------------------------------------------------------

void CViewMatrix::ClampAngle (INT32 i)
{
if (m_angleSave [i] < 0)
	m_angleSave [i] += (INT32) (-m_angleSave [i] / 360) * 360;
else
	m_angleSave [i] -= (INT32) (m_angleSave [i] / 360) * 360;
}

//--------------------------------------------------------------------------

void CViewMatrix::RotateAngle (INT32 i, double a)
{
m_angles [i] += a;
//ClampAngle (i);
}

//--------------------------------------------------------------------------

void CViewMatrix::Push (void)
{
m_mat [1] = m_mat [0];
m_invMat [1] = m_invMat [0];
m_move [1] = m_move [0];
m_invMove [1] = m_invMove [0];
memcpy (m_angleSave, m_angleSave, sizeof (m_angleSave));
m_scaleSave = m_scale;
}

//--------------------------------------------------------------------------

void CViewMatrix::Pop (void)
{
m_mat [0] = m_mat [1];
m_invMat [0] = m_invMat [1];
m_move [0] = m_move [1];
m_invMove [0] = m_invMove [1];
memcpy (m_angleSave, m_angleSave, sizeof (m_angleSave));
m_scale = m_scaleSave;
}

//--------------------------------------------------------------------------

void CViewMatrix::Unrotate (void)
{
#if 0
Rotate ('X', -m_angleSave [0]);
Rotate ('Y', -m_angleSave [1]);
Rotate ('Z', -m_angleSave [2]);
#else
Set (0,0,0,1,1,1,0,0,0);
#endif
Scale (m_scale);
Calculate (m_move [1].v.x, m_move [1].v.y, m_move [1].v.z);
}

//--------------------------------------------------------------------------

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
			m.Set (cosa, sina, 0.0, -sina, cosa, 0.0, 0.0, 0.0, 0.0);
			RotateAngle (2, angle);
			break;
		}
	r = m_invMat [0];
	m_invMat [0] = r * m;
	}
}

//--------------------------------------------------------------------------
// Scale()
//--------------------------------------------------------------------------

void CViewMatrix::Scale (double scale) 
{
CDoubleMatrix s (scale, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, scale);
CDoubleMatrix r = m_invMat [0];
m_invMat [0] = r * s;
m_scale *= scale;
}

//--------------------------------------------------------------------------
// Multiply()
//--------------------------------------------------------------------------

//void CViewMatrix::Multiply(double A[4][4], double B[4][4]) 
//{
//	double T[4][4];
//	
//	T[1][1] = A[1][1]*B[1][1]+A[1][2]*B[2][1]+A[1][3]*B[3][1];
//	T[2][1] = A[2][1]*B[1][1]+A[2][2]*B[2][1]+A[2][3]*B[3][1];
//	T[3][1] = A[3][1]*B[1][1]+A[3][2]*B[2][1]+A[3][3]*B[3][1];
//	
//	T[1][2] = A[1][1]*B[1][2]+A[1][2]*B[2][2]+A[1][3]*B[3][2];
//	T[2][2] = A[2][1]*B[1][2]+A[2][2]*B[2][2]+A[2][3]*B[3][2];
//	T[3][2] = A[3][1]*B[1][2]+A[3][2]*B[2][2]+A[3][3]*B[3][2];
//	
//	T[1][3] = A[1][1]*B[1][3]+A[1][2]*B[2][3]+A[1][3]*B[3][3];
//	T[2][3] = A[2][1]*B[1][3]+A[2][2]*B[2][3]+A[2][3]*B[3][3];
//	T[3][3] = A[3][1]*B[1][3]+A[3][2]*B[2][3]+A[3][3]*B[3][3];
//	
//	A[1][1] = T[1][1];
//	A[1][2] = T[1][2];
//	A[1][3] = T[1][3];
//	A[2][1] = T[2][1];
//	A[2][2] = T[2][2];
//	A[2][3] = T[2][3];
//	A[3][1] = T[3][1];
//	A[3][2] = T[3][2];
//	A[3][3] = T[3][3];
//}

//--------------------------------------------------------------------------
// Calculate()
//--------------------------------------------------------------------------

void CViewMatrix::Calculate (double xMove, double yMove, double zMove) 
{
m_mat [0] = m_invMat [0].Inverse ();
m_move [0].Set (xMove, yMove, zMove);
}

//--------------------------------------------------------------------------
// CalculateInverse()
//--------------------------------------------------------------------------

void CViewMatrix::CalculateInverse (double xMove, double yMove, double zMove) 
{
m_invMat [0] = m_mat [0].Inverse ();
m_invMove [0] = m_invMat [0] * m_move [0];
}

//--------------------------------------------------------------------------
// SetPoint()
//--------------------------------------------------------------------------

void CViewMatrix::SetPoint (CFixVector* vertex, APOINT *apoint) 
{
	CDoubleVector	v (*vertex), r;

v += m_move [0];
r = m_mat [0] * v;
double scale = 5.0;
if ((m_depthPerception < 10000) && (r.v.z > - m_depthPerception)) 
	scale *= m_depthPerception / (r.v.z + m_depthPerception);
r *= CDoubleVector (scale, scale, 1.0);
apoint->x = (INT16) ((FIX) (r.v.x + m_viewWidth) % 32767);
apoint->y = (INT16) ((FIX) (m_viewHeight - r.v.y) % 32767);
apoint->z = (INT16) r.v.z;
}

//--------------------------------------------------------------------------
//			     unset_point()
//--------------------------------------------------------------------------

void CViewMatrix::UnsetPoint (CFixVector* vertex, APOINT *apoint) 
{
CDoubleVector v (double (apoint->x - x_center), double (y_center - apoint->y), double (apoint->z));
double scale = (v.v.z + depth_perception) / depth_perception / 5.0;
v *= CDoubleVector (scale, scale, 1.0);
CDoubleVector r = m_invMat [0] * v;
r -= m_move [0];
*vertex = r;
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

INT32 CViewMatrix::CheckNormal (CGameObject *objP, CFixVector* a, CFixVector* b) 
{
CFixVector _a = objP->orient * *a;
CFixVector _b = objP->orient * *b;
_a += objP->pos;
_a += m_move [0];
_b += _a;
return Dot (m_mat [0].fVec, CDoubleVector (_a)) > Dot (m_mat [0].fVec, CDoubleVector (_b));
}

// -----------------------------------------------------------------------------
// eof