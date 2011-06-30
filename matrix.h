#ifndef __matrix_h
#define __matrix_h

#include <afxwin.h>
#include <math.h>

#include "define.h"
#include "Vector.h"

class CFixMatrix;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CFixMatrix {
public:
	CFixVector rVec, uVec, fVec;
#if 0
	inline void Read (CFileManager* fp) { 
		rVec.Read (fp);
		uVec.Read (fp);
		fVec.Read (fp);
	}

	inline void Write (CFileManager* fp) { 
		rVec.Write (fp);
		uVec.Write (fp);
		fVec.Write (fp);
	}
#endif
	CFixMatrix ();
	CFixMatrix (int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3);
	CFixMatrix (CFixMatrix& m) : rVec(m.rVec), uVec(m.uVec), fVec(m.fVec) {}
	CFixMatrix (CFixVector& r, CFixVector& u, CFixVector& f) : rVec (r), uVec (u), fVec (f) {}
	CFixMatrix (int sinp, int cosp, int sinb, int cosb, int sinh, int cosh);
	//computes a matrix from a Set of three angles.  returns ptr to matrix
	CFixMatrix (CAngleVector& a);
	//computes a matrix from a forward vector and an angle
	CFixMatrix (CFixVector *v, short a);

	CFixMatrix& CFixMatrix::Set (int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3);
	CFixMatrix& Set (int sinp, int cosp, int sinb, int cosb, int sinh, int cosh);

	CFixMatrix& Invert (CFixMatrix& m);
	CFixMatrix Mul (const CFixMatrix& m);
	CFixMatrix& Scale (CFixVector& scale);

	const CFixVector operator* (const CFixVector& v);
	inline const CFixMatrix operator* (const CFixMatrix& other) { return Mul (other); }
	inline const CFixMatrix& operator= (const CFixMatrix& other) { 
		rVec = other.rVec, uVec = other.uVec, fVec = other.fVec;
		return *this;
		}

	const int Det (void);
	const CFixMatrix Inverse (void);
	void CFixMatrix::Rotate (double angle, char axis);
	inline const CFixMatrix Transpose (void);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CFixMatrix& Transpose (CFixMatrix& dest, CFixMatrix& src);

inline CFixMatrix& Transpose (CFixMatrix& m)
{
Swap (m.rVec.v.y, m.uVec.v.x);
Swap (m.rVec.v.z, m.fVec.v.x);
Swap (m.uVec.v.z, m.fVec.v.y);
return m;
}

// -----------------------------------------------------------------------------

inline const CFixMatrix CFixMatrix::Transpose (void)
{
CFixMatrix m;
::Transpose (m, *this);
return m;
}

// -----------------------------------------------------------------------------

inline const CFixVector CFixMatrix::operator* (const CFixVector& v)
{
return CFixVector (v ^ rVec, v ^ uVec, v ^ fVec);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CDoubleMatrix {
public:
	CDoubleVector rVec, uVec, fVec;
#if 0
	inline void Read (CFileManager* fp) { 
		rVec.Read (fp);
		uVec.Read (fp);
		fVec.Read (fp);
		}

	inline void Write (CFileManager* fp) { 
		rVec.Write (fp);
		uVec.Write (fp);
		fVec.Write (fp);
		}
#endif
	CDoubleMatrix::CDoubleMatrix () { Clear (); }
	CDoubleMatrix::CDoubleMatrix (double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3);
	CDoubleMatrix (CDoubleMatrix& m) : rVec(m.rVec), uVec(m.uVec), fVec(m.fVec) {}
	CDoubleMatrix (CDoubleVector r, CDoubleVector u, CDoubleVector f) : rVec (r), uVec (u), fVec (f) {}
	CDoubleMatrix (double sinp, double cosp, double sinb, double cosb, double sinh, double cosh);
	//computes a matrix from a Set of three angles.  returns ptr to matrix
	CDoubleMatrix (CAngleVector& a);
	//computes a matrix from a forward vector and an angle
	CDoubleMatrix (CDoubleVector *v, short a);

	void Clear (void);
	CDoubleMatrix& Set (double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3);
	CDoubleMatrix& Set (double sinp, double cosp, double sinb, double cosb, double sinh, double cosh);

	CDoubleMatrix& Invert (CDoubleMatrix& m);
	CDoubleMatrix Mul (const CDoubleMatrix& m);
	CDoubleMatrix& Scale (CDoubleVector& scale);

	const CDoubleVector operator* (const CDoubleVector& v);
	inline const CDoubleMatrix operator* (const CDoubleMatrix& other) { return Mul (other); }
	inline const CDoubleMatrix& operator= (const CDoubleMatrix& other) { 
		rVec = other.rVec, uVec = other.uVec, fVec = other.fVec;
		return *this;
		}

	inline const bool operator== (const CDoubleMatrix& other) {
		return (rVec == other.rVec) && (uVec == other.uVec) && (fVec == other.fVec);
		}

	inline const bool operator!= (const CDoubleMatrix& other) {
		return (rVec != other.rVec) || (uVec != other.uVec) || (fVec != other.fVec);
		}

	const double Det (void);
	const CDoubleMatrix Inverse (void);
	const CDoubleMatrix Adjoint (void);
	inline const CDoubleMatrix Transpose (void);
	void Rotate (double angle, char axis);
	void Scale (double scale);
	void Square2Quad (tLongVector a [4]);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CDoubleMatrix& Transpose (CDoubleMatrix& dest, CDoubleMatrix& src);

inline CDoubleMatrix& Transpose (CDoubleMatrix& m)
{
Swap (m.rVec.v.y, m.uVec.v.x);
Swap (m.rVec.v.z, m.fVec.v.x);
Swap (m.uVec.v.z, m.fVec.v.y);
return m;
}

// -----------------------------------------------------------------------------

inline const CDoubleMatrix CDoubleMatrix::Transpose (void)
{
CDoubleMatrix m;
::Transpose (m, *this);
return m;
}

// -----------------------------------------------------------------------------

inline const CDoubleVector CDoubleMatrix::operator* (const CDoubleVector& v)
{
return CDoubleVector (v ^ rVec, v ^ uVec, v ^ fVec);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __matrix_h

