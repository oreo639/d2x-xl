// matrix.h
#ifndef __matrix_h
#define __matrix_h

#include "global.h"

class CViewMatrix
{
public:
	CDoubleMatrix	m_mat [2];
	CDoubleMatrix	m_invMat [2];
	CDoubleVector	m_move [2];
	CDoubleVector	m_invMove [2];

private:
	double m_depthPerception;
	double m_scale [2];
	double m_angles [2][3];
	INT16 m_viewWidth;
	INT16 m_viewHeight;

	void ClampAngle (INT32 i);
	void RotateAngle (INT32 i, double a);

public:
	CViewMatrix ();
	void Set (CDoubleVector vMove, CDoubleVector vSize, CDoubleVector vSpin);
	inline void Set (
		double xMove, double yMove, double zMove, 
		double xSize, double ySize, double zSize,
		double xSpin, double ySpin, double zSpin)
	{ Set (CDoubleVector (xMove, yMove, zMove), CDoubleVector (xSize, ySize, zSize), CDoubleVector (xSpin, ySpin, zSpin)); }

	void SetViewInfo (double depthPerception, INT16 viewWidth, INT16 viewHeight);

	void Rotate (char axis, double angle);
	void Scale (double scale);
	//void Multiply (double A[4][4], double B[4][4]);
	void Calculate (double xMove, double yMove, double zMove);
	void CalculateInverse (double xMove, double yMove, double zMove);
	void Project (CVertex& vert, APOINT& apoint);
	void Unproject (CVertex& vert, APOINT& apoint);
	INT32 CheckNormal (CGameObject *objP, CVertex& a, CVertex& b);
	inline double Aspect (void) { return (double) m_viewHeight / (double) m_viewWidth; }
	inline double Scale (void) { return m_scale [0]; }
	void Push (void);
	void Pop (void);
	void Unrotate (void);
};

#endif //__matrix_h