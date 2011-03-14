// matrix.h
#ifndef __viewmatrix_h
#define __viewmatrix_h

#include "matrix.h"

class CViewData {
	public:
		CDoubleMatrix	m_mat;
		CDoubleMatrix	m_invMat;
		CDoubleVector	m_move;
		CDoubleVector	m_invMove;

		double m_scale;
		double m_angles [3];
	};

class CViewMatrix
{
	public:
		CViewData	m_data [2];
		int			m_nSaved;

	private:
		double m_depthPerception;
		short m_viewWidth;
		short m_viewHeight;

		void ClampAngle (int i);
		void RotateAngle (int i, double a);

	public:
		CViewMatrix ();
		void Set (CDoubleVector vMove, CDoubleVector vSize, CDoubleVector vSpin);
		inline void Set (
			double xMove, double yMove, double zMove, 
			double xSize, double ySize, double zSize,
			double xSpin, double ySpin, double zSpin)
		{ Set (CDoubleVector (xMove, yMove, zMove), CDoubleVector (xSize, ySize, zSize), CDoubleVector (xSpin, ySpin, zSpin)); }

		void SetViewInfo (double depthPerception, short viewWidth, short viewHeight);

		void Rotate (char axis, double angle);
		void Scale (double scale);
		//void Multiply (double A[4][4], double B[4][4]);
		void Calculate (double xMove, double yMove, double zMove);
		void CalculateInverse (double xMove, double yMove, double zMove);
		void Project (CDoubleVector& vert, tLongVector& apoint);
		void Unproject (CVertex& vert, tLongVector& apoint);
		inline double Aspect (void) { return (double) m_viewHeight / (double) m_viewWidth; }
		inline double Scale (void) { return m_data [0].m_scale; }
		bool Push (void);
		bool Pop (void);
		void Unrotate (void);
		//int CheckNormal (CGameObject *objP, CVertex& a, CVertex& b);
		//int CheckNormal (CGameObject *objP, CFixVector& a, CFixVector& b);
	};

#endif //__viewmatrix_h