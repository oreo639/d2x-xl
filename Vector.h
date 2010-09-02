#ifndef __vectortypes_h
#define __vectortypes_h

#include <math.h>
#include "define.h"
#include "VectorTypes.h"
//#include "cfile.h"

struct tAngleVector;
class CAngleVector;
struct tFixVector;
class CFixVector;
struct tDoubleVector;
class CDoubleVector;
class CFixMatrix;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

inline double Round (double value, double round = 1.0) { return (value >= 0) ? value + round / 2.0 : value - round / 2.0; }

// -----------------------------------------------------------------------------

#define X2D(_v)	((double) _v / 65536.0)
#define D2X(_v)	((int) Round (_v * 65536.0))
#define X2I(_v)	(_v / 65536)
#define I2X(_v)	((int) _v * 65536.0)

// -----------------------------------------------------------------------------

inline int FixMul (int n, int m)
{
return (int) ((double) n * (double) m / 65536.0);
}

inline int FixDiv (int n, int m)
{
return (int) ((double) n / (double) m * 65536.0);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tAngleVector {
public:
	short p, b, h;
#if 0
inline void Read (CFileManager& fp) { 
	p = fp.ReadFixAng ();
	b = fp.ReadFixAng ();
	h = fp.ReadFixAng ();
	}

inline void Write (CFileManager& fp) { 
	WriteFixAng (p, fp);
	WriteFixAng (b, fp);
	WriteFixAng (h, fp);
	}
#endif
} tAngleVector;

class CAngleVector {
public:
	tAngleVector	v;

	CAngleVector () { v.p = 0, v.b = 0, v.h = 0; }
	CAngleVector (short p, short b, short h) { v.p = p, v.b = b, v.h = h; }
	CAngleVector (tAngleVector& _v) { v.p = _v.p, v.b = _v.b, v.h = _v.h; }
	CAngleVector (CAngleVector& _v) { v.p = _v.v.p, v.b = _v.v.b, v.h = _v.v.h; }
	void Set (short p, short b, short h) { v.p = p, v.b = b, v.h = h; }
	void Clear (void) { Set (0,0,0); }
#if 0
	inline void Read (CFileManager& fp) { v.Read (fp); }
	inline void Write (CFileManager& fp) { v.Write (fp); }
#endif
	inline const CAngleVector& operator= (CAngleVector& other) { 
		v.p = other.v.p, v.b = other.v.b, other.v.h = other.v.h; 
		return *this;
		}
	inline const CAngleVector& operator= (tAngleVector& other) { 
		v.p = other.p, v.b = other.b, v.h = other.h; 
		return *this;
		}
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#if FIX_IS_DOUBLE

#define tFixVector	tDoubleVector	
#define CFixVector	CDoubleVector	

#else

struct tFixVector {
public:
	int x, y, z;
#if 0
inline void Read (CFileManager& fp) { 
	x = fp.ReadInt32 ();
	y = fp.ReadInt32 ();
	z = fp.ReadInt32 ();
	}

inline void Write (CFileManager& fp) { 
	WriteFix (x, fp);
	WriteFix (y, fp);
	WriteFix (z, fp);
	}
#endif
};

class CFixVector {
public:
	tFixVector	v;
	//double x, y, z;
	CFixVector ()  { v.x = 0, v.y = 0, v.z = 0; }
	CFixVector (int x, int y, int z) { v.x = x, v.y = y, v.z = z; }
	CFixVector (tFixVector& _v) { v.x = _v.x, v.y = _v.y, v.z = _v.z; }
	CFixVector (CDoubleVector& _v); 
	void Set (int x, int y, int z) { v.x = x, v.y = y, v.z = z; }
	void Clear (void) { Set (0,0,0); }

#if 0
inline void Read (CFileManager& fp) { v.Read (fp); }
inline void Write (CFileManager& fp) { v.Write (fp); }
#endif

inline const bool operator== (const CFixVector other);
inline int& CFixVector::operator[] (const size_t i);
inline const CFixVector& operator= (const tFixVector& other);
inline const CFixVector& operator= (const CFixVector& other);
inline const CFixVector& operator= (const tDoubleVector& other);
inline const CFixVector& operator= (const CDoubleVector& other);
inline const CFixVector& operator+= (const CFixVector other);
inline const CFixVector& operator-= (const CFixVector other);
inline const CFixVector& operator*= (const int n);
inline const CFixVector& operator/= (const int n);
inline const CFixVector& operator*= (const double n);
inline const CFixVector& operator/= (const double n);
inline const CFixVector operator+ (const CFixVector& other) const;
inline const CFixVector operator- (const CFixVector& other) const;
inline const CFixVector operator- (void) const;
inline const CFixVector operator>> (const int n);
inline const CFixVector operator<< (const int n);
inline const CFixVector& operator>>= (const int n);
inline const CFixVector& operator<<= (const int n);
inline const CFixVector operator* (CFixVector other) const;
inline const CFixVector operator/ (CFixVector other) const;
inline const CFixVector operator* (int n) const;
inline const CFixVector operator/ (int n) const;
inline const int operator^ (const CFixVector& other) const;

inline const int Mag (void);
inline const CFixVector& Normalize (void) { *this /= Mag (); return *this; }
void Rotate (CFixVector& origin, CFixVector& normal, double angle);
};

#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct tDoubleVector {
public:
	double x, y, z;
#if 0
inline void Read (CFileManager& fp) { 
	x = X2D (fp.ReadInt32 ());
	y = X2D (fp.ReadInt32 ());
	z = X2D (fp.ReadInt32 ());
	}

inline void Write (CFileManager& fp) { 
	WriteFix (D2X (x), fp);
	WriteFix (D2X (y), fp);
	WriteFix (D2X (z), fp);
	}
#endif
};

class CDoubleVector {
	public:
		tDoubleVector	v;
		//double x, y, z;
		CDoubleVector ()  { v.x = 0, v.y = 0, v.z = 0; }
		CDoubleVector (double x, double y, double z) { v.x = x, v.y = y, v.z = z; }
		CDoubleVector (tDoubleVector& _v) { v.x = _v.x, v.y = _v.y, v.z = _v.z; }
		CDoubleVector (CFixVector _v);
		void Set (double x, double y, double z) { v.x = x, v.y = y, v.z = z; }
		void Clear (void) { Set (0,0,0); }
#if 0
	inline void Read (CFileManager& fp) { v.Read (fp); }
	inline void Write (CFileManager& fp) { v.Write (fp); }
#endif
	inline const bool operator== (const CDoubleVector other);
	inline double& CDoubleVector::operator[] (const size_t i);
	inline const CDoubleVector& operator= (const tDoubleVector& other);
	inline const CDoubleVector& operator= (const CDoubleVector& other);
	inline const CDoubleVector& operator= (const tFixVector& other);
	inline const CDoubleVector& operator= (const CFixVector& other);
	inline const CDoubleVector& operator+= (const CDoubleVector& other);
	inline const CDoubleVector& operator-= (const CDoubleVector& other);
	inline const CDoubleVector& operator*= (const double n);
	inline const CDoubleVector& operator/= (const double n);
	inline const CDoubleVector& operator*= (const CDoubleVector& other);
	inline const CDoubleVector& operator/= (const CDoubleVector& other);
	inline const CDoubleVector operator+ (const CDoubleVector& other) const;
	inline const CDoubleVector operator- (const CDoubleVector& other) const;
	inline const CDoubleVector operator- (void) const;
	inline const CDoubleVector operator* (const CDoubleVector& other) const;
	inline const CDoubleVector operator/ (const CDoubleVector& other) const;
	inline const CDoubleVector operator* (double n);
	inline const CDoubleVector operator/ (double n);
	inline const double operator^ (const CDoubleVector& other) const;

	inline const double Mag (void) { return sqrt (v.x * v.x + v.y * v.y + v.z * v.z); }
	inline const CDoubleVector& Normalize (void) { *this /= Mag (); return *this; }
	void CDoubleVector::Rotate (CDoubleVector& origin, CDoubleVector& normal, double angle);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

inline const CFixVector& CFixVector::operator= (const tFixVector& other) { 
	v.x = other.x, v.y = other.y, v.z = other.z; 
	return *this;
	}

inline const bool CFixVector::operator== (const CFixVector other) {
	return (v.x == other.v.x) && (v.y == other.v.y) && (v.z == other.v.z);
}

inline int& CFixVector::operator[] (const size_t i) { return ((int*) &v) [i]; }

inline const CFixVector& CFixVector::operator= (const CFixVector& other) { 
	v.x = other.v.x, v.y = other.v.y, v.z = other.v.z; 
	return *this;
	}

inline const CFixVector& CFixVector::operator= (const tDoubleVector& other) { 
	v.x = D2X (other.x), v.y = D2X (other.y), v.z = D2X (other.z); 
	return *this;
	}

inline const CFixVector& CFixVector::operator= (const CDoubleVector& other) { 
	v.x = D2X (other.v.x), v.y = D2X (other.v.y), v.z = D2X (other.v.z); 
	return *this;
	}

inline const CFixVector& CFixVector::operator+= (const CFixVector other) {
	v.x += other.v.x, v.y += other.v.y, v.z += other.v.z; 
	return *this;
	}

inline const CFixVector& CFixVector::operator-= (const CFixVector other) {
	v.x -= other.v.x, v.y -= other.v.y, v.z -= other.v.z; 
	return *this;
	}

inline const CFixVector CFixVector::operator+ (const CFixVector& other) const {
	return CFixVector (v.x + other.v.x, v.y + other.v.y, v.z + other.v.z);
	}

inline const CFixVector CFixVector::operator- (const CFixVector& other) const {
	return CFixVector (v.x - other.v.x, v.y - other.v.y, v.z - other.v.z);
	}

inline const CFixVector CFixVector::operator- (void) const {
	return CFixVector (-v.x, -v.y, -v.z);
	}

inline const CFixVector& CFixVector::operator*= (const int n) {
	v.x = FixMul (v.x, n), v.y = FixMul (v.y, n), v.z = FixMul (v.z, n);
	return *this;
	}

inline const CFixVector& CFixVector::operator/= (const int n) {
	v.x = FixDiv (v.x, n), v.y = FixDiv (v.y, n), v.z = FixDiv (v.z, n);
	return *this;
	}

inline const CFixVector& CFixVector::operator*= (const double n) {
	v.x = (int) Round ((double) v.x * n),
	v.y = (int) Round ((double) v.y * n),
	v.z = (int) Round ((double) v.z * n);
	return *this;
	}

inline const CFixVector& CFixVector::operator/= (const double n) {
	v.x = (int) Round ((double) v.x / n),
	v.y = (int) Round ((double) v.y / n),
	v.z = (int) Round ((double) v.z / n);
	return *this;
	}

inline const CFixVector CFixVector::operator* (const int n) const {
	return CFixVector (FixMul (v.x, n), FixMul (v.y, n), FixMul (v.z, n));
	}

inline const CFixVector CFixVector::operator/ (const int n) const {
	return CFixVector (FixDiv (v.x , n), FixDiv (v.y , n), FixDiv (v.z , n));
	}

inline const CFixVector CFixVector::operator* (CFixVector other) const {
	return CFixVector (FixMul (v.x, other.v.x), FixMul (v.y, other.v.y), FixMul (v.z, other.v.z));
	}

inline const CFixVector CFixVector::operator/ (CFixVector other) const {
	return CFixVector (FixDiv (v.x, other.v.x), FixDiv (v.y, other.v.y), FixDiv (v.z, other.v.z));
	}

inline const int CFixVector::operator^ (const CFixVector& other) const {
	return X2D (FixMul (v.x, other.v.x) + FixMul (v.y, other.v.y) + FixMul (v.z, other.v.z));
	}

inline const int CFixVector::Mag (void) { return D2X (CDoubleVector (*this).Mag ()); }

// -----------------------------------------------------------------------------

inline const bool CDoubleVector::operator== (const CDoubleVector other) {
	return (v.x == other.v.x) && (v.y == other.v.y) && (v.z == other.v.z);
}

inline double& CDoubleVector::operator[] (const size_t i) { return ((double*) &v) [i]; }

inline const CDoubleVector& CDoubleVector::operator= (const tDoubleVector& other) { 
	v.x = other.x, v.y = other.y, v.z = other.z; 
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator= (const CDoubleVector& other) { 
	v.x = other.v.x, v.y = other.v.y, v.z = other.v.z; 
	return *this;
	}

#if !FIX_IS_DOUBLE
inline const CDoubleVector& CDoubleVector::operator= (const tFixVector& other) { 
	v.x = X2D (other.x), v.y = X2D (other.y), v.z = X2D (other.z); 
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator= (const CFixVector& other) { 
	v.x = X2D (other.v.x), v.y = X2D (other.v.y), v.z = X2D (other.v.z); 
	return *this;
	}
#endif

inline const CDoubleVector& CDoubleVector::operator+= (const CDoubleVector& other) {
	v.x += other.v.x, v.y += other.v.y, v.z += other.v.z; 
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator-= (const CDoubleVector& other) {
	v.x -= other.v.x, v.y -= other.v.y, v.z -= other.v.z; 
	return *this;
	}

inline const CDoubleVector CDoubleVector::operator+ (const CDoubleVector& other) const {
	return CDoubleVector (v.x + other.v.x, v.y + other.v.y, v.z + other.v.z);
	}

inline const CDoubleVector CDoubleVector::operator- (const CDoubleVector& other) const {
	return CDoubleVector (v.x - other.v.x, v.y - other.v.y, v.z - other.v.z);
	}

inline const CDoubleVector CDoubleVector::operator- (void) const {
	return CDoubleVector (-v.x, -v.y, -v.z);
	}

inline const CDoubleVector& CDoubleVector::operator*= (const double n) {
	v.x *= n, v.y *= n, v.z *= n;
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator/= (const double n) {
	v.x /= n, v.y /= n, v.z /= n;
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator*= (const CDoubleVector& other) {
	v.x *= other.v.x, v.y *= other.v.y, v.z *= other.v.z;
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator/= (const CDoubleVector& other) {
	v.x /= other.v.x, v.y /= other.v.y, v.z /= other.v.z;
	return *this;
	}

inline const CDoubleVector CDoubleVector::operator/ (const double n) {
	return CDoubleVector (v.x / n, v.y / n, v.z / n);
	}

inline const CDoubleVector CDoubleVector::operator* (const double n) {
	return CDoubleVector (v.x * n, v.y * n, v.z * n);
	}

inline const CDoubleVector CDoubleVector::operator* (const CDoubleVector& other) const {
	return CDoubleVector (v.x * other.v.x, v.y * other.v.y, v.z * other.v.z);
	}

inline const CDoubleVector CDoubleVector::operator/ (const CDoubleVector& other) const {
	return CDoubleVector (v.x / other.v.x, v.y / other.v.y, v.z / other.v.z);
	}

inline const double CDoubleVector::operator^ (const CDoubleVector& other) const {
	return double (v.x) * double (other.v.x) + double (v.y) * double (other.v.y) + double (v.z) * double (other.v.z);
	}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

static inline const CDoubleVector CrossProduct (const CDoubleVector& v0, const CDoubleVector& v1) {
	return CDoubleVector (v0.v.y * v1.v.z - v0.v.z * v1.v.y, v0.v.z * v1.v.x - v0.v.x * v1.v.z, v0.v.x * v1.v.y - v0.v.y * v1.v.x);
	}

static inline CDoubleVector Perpendicular (const CDoubleVector& p0, const CDoubleVector& p1, const CDoubleVector& p2) {
	return CrossProduct (p1 - p0, p2 - p1);
	}

static inline const CDoubleVector Normalize (CDoubleVector v) { 
	double m = v.Mag ();
	return (m != 0.0) ? v / m : CDoubleVector (0.0, 0.0, 0.0); 
	}

static inline const CDoubleVector Normal (const CDoubleVector& p0, const CDoubleVector& p1, const CDoubleVector& p2) {
	return Normalize (CrossProduct (p1 - p0, p2 - p0));
	}

static inline const double Normal (CDoubleVector& normal, const CDoubleVector& p0, const CDoubleVector& p1, const CDoubleVector& p2) {
	normal = CrossProduct (p1 - p0, p2 - p0);
	double m = normal.Mag ();
	if (m > 0.0)
		normal *= m;
	return m;
	}

#if !FIX_IS_DOUBLE
static inline int Dot (const CFixVector& v0, const CFixVector& v1) {
	return int ((double (v0.v.x) * double (v1.v.x) + double (v0.v.y) * double (v1.v.y) + double (v0.v.z) * double (v1.v.z)) / 65536.0);
	}

static inline CFixVector Min (const CFixVector& v0, const CFixVector& v1) {
	return CFixVector(min (v0.v.x, v1.v.x), min (v0.v.y, v1.v.y), min (v0.v.z, v1.v.z));
	}

static inline CFixVector Max (const CFixVector& v0, const CFixVector& v1) {
	return CFixVector(max (v0.v.x, v1.v.x), max (v0.v.y, v1.v.y), max (v0.v.z, v1.v.z));
	}

static inline double Distance (const CFixVector& p0, const CFixVector& p1) {
	CFixVector v = p0 - p1;
	return D2X (CDoubleVector (v).Mag ());
	}

static inline CFixVector Average (const CFixVector& p0, const CFixVector& p1) {
	CFixVector v = p0 + p1;
	v /= 2.0;
	return v;
	}
#endif

static inline double Dot (const CDoubleVector& v0, const CDoubleVector& v1) {
	return double (v0.v.x) * double (v1.v.x) + double (v0.v.y) * double (v1.v.y) + double (v0.v.z) * double (v1.v.z);
	}

static inline CDoubleVector Min (const CDoubleVector& v0, const CDoubleVector& v1) {
	return CDoubleVector(min (v0.v.x, v1.v.x), min (v0.v.y, v1.v.y), min (v0.v.z, v1.v.z));
	}

static inline CDoubleVector Max (const CDoubleVector& v0, const CDoubleVector& v1) {
	return CDoubleVector(max (v0.v.x, v1.v.x), max (v0.v.y, v1.v.y), max (v0.v.z, v1.v.z));
	}

static inline double Distance (const CDoubleVector& p0, const CDoubleVector& p1) {
	CDoubleVector v = p0 - p1;
	return v.Mag ();
	}

static inline CDoubleVector Average (const CDoubleVector& p0, const CDoubleVector& p1) {
	CDoubleVector v = p0 + p1;
	v /= double (2);
	return v;
	}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#if FIX_IS_DOUBLE

#define CFixMatrix	CDoubleMatrix

#else

class CFixMatrix {
public:
	CFixVector rVec, uVec, fVec;
#if 0
	inline void Read (CFileManager& fp) { 
		rVec.Read (fp);
		uVec.Read (fp);
		fVec.Read (fp);
	}

	inline void Write (CFileManager& fp) { 
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

#endif

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CDoubleMatrix {
public:
	CDoubleVector rVec, uVec, fVec;
#if 0
	inline void Read (CFileManager& fp) { 
		rVec.Read (fp);
		uVec.Read (fp);
		fVec.Read (fp);
		}

	inline void Write (CFileManager& fp) { 
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

	const double Det (void);
	const CDoubleMatrix Inverse (void);
	const CDoubleMatrix Adjoint (void);
	inline const CDoubleMatrix Transpose (void);
	void Rotate (double angle, char axis);
	void Scale (double scale);
	void Square2Quad (POINT a [4]);
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

#endif // __vectortypes_h

