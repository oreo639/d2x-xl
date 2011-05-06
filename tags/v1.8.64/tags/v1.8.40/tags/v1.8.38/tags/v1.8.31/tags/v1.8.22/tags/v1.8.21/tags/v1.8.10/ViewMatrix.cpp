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

CViewMatrix::CViewMatrix () 
{
Set (0,0,0,1,1,1,0,0,0);
m_scale [0] = 1;
m_angles [1][0] = 
m_angles [1][1] =
m_angles [1][2] = 0;
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

//--------------------------------------------------------------------------
// Rotate()
//--------------------------------------------------------------------------

void CViewMatrix::ClampAngle (int i)
{
if (m_angles [0][i] < 0)
	m_angles [0][i] += (int) (-m_angles [0][i] / 360) * 360;
else
	m_angles [0][i] -= (int) (m_angles [0][i] / 360) * 360;
}

//--------------------------------------------------------------------------

void CViewMatrix::RotateAngle (int i, double a)
{
m_angles [0][i] += a;
//ClampAngle (i);
}

//--------------------------------------------------------------------------

void CViewMatrix::Push (void)
{
m_mat [1] = m_mat [0];
m_invMat [1] = m_invMat [0];
m_move [1] = m_move [0];
m_invMove [1] = m_invMove [0];
memcpy (m_angles [0], m_angles [1], sizeof (m_angles [1]));
m_scale [1] = m_scale [0];
}

//--------------------------------------------------------------------------

void CViewMatrix::Pop (void)
{
m_mat [0] = m_mat [1];
m_invMat [0] = m_invMat [1];
m_move [0] = m_move [1];
m_invMove [0] = m_invMove [1];
memcpy (m_angles [1], m_angles [0], sizeof (m_angles [1]));
m_scale [0] = m_scale [1];
}

//--------------------------------------------------------------------------

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
m_scale [0] *= scale;
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
// Project()
//--------------------------------------------------------------------------

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

//--------------------------------------------------------------------------
//			     unset_point()
//--------------------------------------------------------------------------

void CViewMatrix::Unproject (CVertex& vertex, APOINT& apoint) 
{
CDoubleVector v (double (apoint.x - x_center), double (y_center - apoint.y), double (apoint.z));
double scale = (v.v.z + depthPerception) / depthPerception / 5.0;
v *= CDoubleVector (scale, scale, 1.0);
CDoubleVector r = m_invMat [0] * v;
r -= m_move [0];
vertex = r;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// eof