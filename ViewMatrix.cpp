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

CViewMatrix::CViewMatrix () : m_nSaved (0)
{
Set (0,0,0,1,1,1,0,0,0);
m_data [0].m_scale = 1;
m_data [1].m_angles[0] = 
m_data [1].m_angles[1] =
m_data [1].m_angles[2] = 0;
}

// -----------------------------------------------------------------------------
// SetViewInfo
// -----------------------------------------------------------------------------

void CViewMatrix::SetViewInfo (double depthPerception, short viewWidth, short viewHeight)
{
m_depthPerception = depthPerception;
m_viewWidth = viewWidth / 2;
m_viewHeight = viewHeight / 2;
}

//--------------------------------------------------------------------------
// Set()
//--------------------------------------------------------------------------

void CViewMatrix::Set (CDoubleVector vMove, CDoubleVector vSize, CDoubleVector vSpin)
{
m_data [0].m_move = vMove;

CDoubleVector sinSpin (sin (vSpin.v.x), sin (vSpin.v.y), sin (vSpin.v.z));
CDoubleVector cosSpin (cos (vSpin.v.x), cos (vSpin.v.y), cos (vSpin.v.z));

m_data [0].m_mat.Set (vSize.v.x * cosSpin.v.z * cosSpin.v.y, 
							 vSize.v.y * sinSpin.v.z * cosSpin.v.y, 
							 -vSize.v.z * sinSpin.v.y,
							 -vSize.v.x * sinSpin.v.z * cosSpin.v.x + vSize.v.x * cosSpin.v.z * sinSpin.v.y * sinSpin.v.x,
							 vSize.v.y * sinSpin.v.z * sinSpin.v.y * sinSpin.v.x + vSize.v.y * cosSpin.v.z * cosSpin.v.x,
							 vSize.v.z * cosSpin.v.y * sinSpin.v.x,
							 vSize.v.x * cosSpin.v.z * sinSpin.v.y * cosSpin.v.x - vSize.v.x * sinSpin.v.z * sinSpin.v.x,
							 vSize.v.y * sinSpin.v.z * sinSpin.v.y * cosSpin.v.x - vSize.v.y * cosSpin.v.z * sinSpin.v.x,
							 vSize.v.z * cosSpin.v.y * cosSpin.v.x);
m_data [0].m_invMat = m_data [0].m_mat.Inverse ();
m_data [0].m_invMove = m_data [0].m_invMat * m_data [0].m_move;
}

//--------------------------------------------------------------------------
// Rotate()
//--------------------------------------------------------------------------

void CViewMatrix::ClampAngle (int i)
{
if (m_data [0].m_angles[i] < 0)
	m_data [0].m_angles[i] += (int) (-m_data [0].m_angles[i] / 360) * 360;
else
	m_data [0].m_angles[i] -= (int) (m_data [0].m_angles[i] / 360) * 360;
}

//--------------------------------------------------------------------------

void CViewMatrix::RotateAngle (int i, double a)
{
m_data [0].m_angles[i] += a;
//ClampAngle (i);
}

//--------------------------------------------------------------------------

bool CViewMatrix::Push (void)
{
if (m_nSaved > 0)
	return false;
++m_nSaved;
memcpy (&m_data [1], &m_data [0], sizeof (CViewData));
return true;
}

//--------------------------------------------------------------------------

bool CViewMatrix::Pop (void)
{
if (m_nSaved < 1)
	return false;
--m_nSaved;
memcpy (&m_data [0], &m_data [1], sizeof (CViewData));
return true;
}

//--------------------------------------------------------------------------

void CViewMatrix::Unrotate (void)
{
#if 0
Rotate ('X', -m_data [1].m_angles[0]);
Rotate ('Y', -m_data [1].m_angles[1]);
Rotate ('Z', -m_data [1].m_angles[2]);
#else
Set (0,0,0,1,1,1,0,0,0);
#endif
Scale (m_data [0].m_scale);
Calculate (m_data [1].m_move.v.x, m_data [1].m_move.v.y, m_data [1].m_move.v.z);
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
			m.Set (cosa, sina, 0.0, -sina, cosa, 0.0, 0.0, 0.0, 1.0);
			RotateAngle (2, angle);
			break;
		}
	r = m_data [0].m_invMat;
	m_data [0].m_invMat = r * m;
	}
}

//--------------------------------------------------------------------------
// Scale()
//--------------------------------------------------------------------------

void CViewMatrix::Scale (double scale) 
{
CDoubleMatrix s (scale, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, scale);
CDoubleMatrix r = m_data [0].m_invMat;
m_data [0].m_invMat = r * s;
m_data [0].m_scale *= scale;
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
m_data [0].m_mat = m_data [0].m_invMat.Inverse ();
m_data [0].m_move.Set (xMove, yMove, zMove);
}

//--------------------------------------------------------------------------
// CalculateInverse()
//--------------------------------------------------------------------------

void CViewMatrix::CalculateInverse (double xMove, double yMove, double zMove) 
{
m_data [0].m_invMat = m_data [0].m_mat.Inverse ();
m_data [0].m_invMove = m_data [0].m_invMat * m_data [0].m_move;
}

//--------------------------------------------------------------------------
// Project()
//--------------------------------------------------------------------------

void CViewMatrix::Project (CDoubleVector& vertex, APOINT& apoint) 
{
	CDoubleVector	r, v = vertex;

v += m_data [0].m_move;
r = m_data [0].m_mat * v;
double scale = 5.0;
if ((m_depthPerception < 10000) && (r.v.z > -m_depthPerception)) 
	scale *= m_depthPerception / (r.v.z + m_depthPerception);
r *= CDoubleVector (scale, scale, 1.0);
apoint.x = ((long) (Round (r.v.x) + m_viewWidth) % 32767);
apoint.y = ((long) (m_viewHeight - Round (r.v.y)) % 32767);
apoint.z = (long) Round (r.v.z * 10000); // 5 digits precision
}

//--------------------------------------------------------------------------
//			     unset_point()
//--------------------------------------------------------------------------

void CViewMatrix::Unproject (CVertex& vertex, APOINT& apoint) 
{
CDoubleVector r (double (apoint.x - x_center), double (y_center - apoint.y), double (apoint.z) / 10000.0);
double scale = 0.2;
if ((m_depthPerception < 10000) && (r.v.z > -m_depthPerception)) 
	scale *= (r.v.z + depthPerception) / depthPerception;
r *= CDoubleVector (scale, scale, 1.0);
CDoubleVector v = m_data [0].m_invMat * r;
v -= m_data [0].m_move;
vertex = v;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// eof