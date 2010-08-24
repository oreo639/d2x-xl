#ifndef VECTOR_TYPES_H
#define VECTOR_TYPES_H

#include <math.h>
#include "define.h"
#include "VectorTypes.h"

struct tAngleVector;
class CAngleVector;
struct tFixVector;
class CFixVector;
struct tDoubleVector;
class CDoubleVector;
class CFixMatrix;

// --------------------------------------------------------------------------

inline FIX FixMul (FIX n, FIX m)
{
return (FIX) ((double) n * (double) m / 65536.0);
}

inline FIX FixDiv (FIX n, FIX m)
{
return (FIX) ((double) n / (double) m * 65536.0);
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

typedef struct tAngleVector {
public:
	FIXANG p, b, h;

inline INT32 Read (FILE* fp) { 
	p = read_FIXANG (fp);
	b = read_FIXANG (fp);
	h = read_FIXANG (fp);
	return 1;
	}

inline void Write (FILE* fp) { 
	write_FIXANG (p, fp);
	write_FIXANG (b, fp);
	write_FIXANG (h, fp);
	}

} tAngleVector;

class CAngleVector {
public:
	tAngleVector	v;

	CAngleVector () { v.p = 0, v.b = 0, v.h = 0; }
	CAngleVector (FIXANG p, FIXANG b, FIXANG h) { v.p = p, v.b = b, v.h = h; }
	CAngleVector (tAngleVector& _v) { v.p = _v.p, v.b = _v.b, v.h = _v.h; }
	CAngleVector (CAngleVector& _v) { v.p = _v.v.p, v.b = _v.v.b, v.h = _v.v.h; }
	void Set (FIXANG p, FIXANG b, FIXANG h) { v.p = p, v.b = b, v.h = h; }
	void Clear (void) { Set (0,0,0); }

	inline INT32 Read (FILE* fp) { return v.Read (fp); }
	inline void Write (FILE* fp) { v.Write (fp); }

	inline const CAngleVector& operator= (CAngleVector& other) { 
		v.p = other.v.p, v.b = other.v.b, other.v.h = other.v.h; 
		return *this;
		}
	inline const CAngleVector& operator= (tAngleVector& other) { 
		v.p = other.p, v.b = other.b, v.h = other.h; 
		return *this;
		}
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

#if FIX_IS_DOUBLE

#define tFixVector	tDoubleVector	
#define CFixVector	CDoubleVector	

#else

struct tFixVector {
public:
	FIX x, y, z;

inline INT32 Read (FILE* fp) { 
	x = read_FIX (fp);
	y = read_FIX (fp);
	z = read_FIX (fp);
	return 1;
	}

inline void Write (FILE* fp) { 
	write_FIX (x, fp);
	write_FIX (y, fp);
	write_FIX (z, fp);
	}
};

class CFixVector {
public:
	tFixVector	v;
	//DOUBLE x, y, z;
	CFixVector ()  { v.x = 0, v.y = 0, v.z = 0; }
	CFixVector (FIX x, FIX y, FIX z) { v.x = x, v.y = y, v.z = z; }
	CFixVector (tFixVector& _v) { v.x = _v.x, v.y = _v.y, v.z = _v.z; }
	CFixVector (CDoubleVector& _v); 
	void Set (FIX x, FIX y, FIX z) { v.x = x, v.y = y, v.z = z; }
	void Clear (void) { Set (0,0,0); }


inline INT32 Read (FILE* fp) { return v.Read (fp); }
inline void Write (FILE* fp) { v.Write (fp); }

inline const bool operator== (const CFixVector other);
inline FIX& CFixVector::operator[] (const size_t i);
inline const CFixVector& operator= (const tFixVector& other);
inline const CFixVector& operator= (const CFixVector& other);
inline const CFixVector& operator= (const tDoubleVector& other);
inline const CFixVector& operator= (const CDoubleVector& other);
inline const CFixVector& operator+= (const CFixVector other);
inline const CFixVector& operator-= (const CFixVector other);
inline const CFixVector& operator*= (const FIX n);
inline const CFixVector& operator/= (const FIX n);
inline const CFixVector& operator*= (const DOUBLE n);
inline const CFixVector& operator/= (const DOUBLE n);
inline const CFixVector operator+ (const CFixVector& other) const;
inline const CFixVector operator- (const CFixVector& other) const;
inline const CFixVector operator- (void) const;
inline const CFixVector operator>> (const FIX n);
inline const CFixVector operator<< (const FIX n);
inline const CFixVector& operator>>= (const FIX n);
inline const CFixVector& operator<<= (const FIX n);
inline const CFixVector operator* (CFixVector other) const;
inline const CFixVector operator/ (CFixVector other) const;
inline const CFixVector operator* (FIX n) const;
inline const CFixVector operator/ (FIX n) const;
inline const FIX operator^ (const CFixVector& other) const;

inline const FIX Mag (void);
inline const CFixVector& Normalize (void) { *this /= Mag (); return *this; }
};

#endif

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

struct tDoubleVector {
public:
	DOUBLE x, y, z;

inline INT32 Read (FILE* fp) { 
	x = X2D (read_FIX (fp));
	y = X2D (read_FIX (fp));
	z = X2D (read_FIX (fp));
	return 1;
	}

inline void Write (FILE* fp) { 
	write_DOUBLE (D2X (x), fp);
	write_DOUBLE (D2X (y), fp);
	write_DOUBLE (D2X (z), fp);
	}
};

class CDoubleVector {
public:
	tDoubleVector	v;
	//DOUBLE x, y, z;
	CDoubleVector ()  { v.x = 0, v.y = 0, v.z = 0; }
	CDoubleVector (DOUBLE x, DOUBLE y, DOUBLE z) { v.x = x, v.y = y, v.z = z; }
	CDoubleVector (tDoubleVector& _v) { v.x = _v.x, v.y = _v.y, v.z = _v.z; }
#if !FIX_IS_DOUBLE
	CDoubleVector (CFixVector _v);
#endif
	//CDoubleVector (CDoubleVector& _v) { v.x = _v.v.x, v.y = _v.v.y, v.z = _v.v.z; }
	void Set (DOUBLE x, DOUBLE y, DOUBLE z) { v.x = x, v.y = y, v.z = z; }
	void Clear (void) { Set (0,0,0); }

inline INT32 Read (FILE* fp) { return v.Read (fp); }
inline void Write (FILE* fp) { v.Write (fp); }

inline const bool operator== (const CDoubleVector other);
inline DOUBLE& CDoubleVector::operator[] (const size_t i);
inline const CDoubleVector& operator= (const tDoubleVector& other);
inline const CDoubleVector& operator= (const CDoubleVector& other);
#if !FIX_IS_DOUBLE
inline const CDoubleVector& operator= (const tFixVector& other);
inline const CDoubleVector& operator= (const CFixVector& other);
#endif
inline const CDoubleVector& operator+= (const CDoubleVector other);
inline const CDoubleVector& operator-= (const CDoubleVector other);
inline const CDoubleVector& operator*= (const DOUBLE n);
inline const CDoubleVector& operator/= (const DOUBLE n);
inline const CDoubleVector& operator*= (const CDoubleVector other);
inline const CDoubleVector& operator/= (const CDoubleVector other);
inline const CDoubleVector operator+ (const CDoubleVector& other) const;
inline const CDoubleVector operator- (const CDoubleVector& other) const;
inline const CDoubleVector operator- (void) const;
inline const CDoubleVector operator* (CDoubleVector other) const;
inline const CDoubleVector operator/ (CDoubleVector other) const;
inline const CDoubleVector operator* (DOUBLE n);
inline const CDoubleVector operator/ (DOUBLE n);
inline const DOUBLE operator^ (const CDoubleVector& other) const;

inline const DOUBLE Mag (void) { return sqrt (v.x * v.x + v.y * v.y + v.z * v.z); }
inline const CDoubleVector& Normalize (void) { *this /= Mag (); return *this; }
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

#if !FIX_IS_DOUBLE

inline const CFixVector& CFixVector::operator= (const tFixVector& other) { 
	v.x = other.x, v.y = other.y, v.z = other.z; 
	return *this;
	}

inline const bool CFixVector::operator== (const CFixVector other) {
	return (v.x == other.v.x) && (v.y == other.v.y) && (v.z == other.v.z);
}

inline FIX& CFixVector::operator[] (const size_t i) { return ((FIX*) &v) [i]; }

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

inline const CFixVector& CFixVector::operator*= (const FIX n) {
	v.x = FixMul (v.x, n), v.y = FixMul (v.y, n), v.z = FixMul (v.z, n);
	return *this;
	}

inline const CFixVector& CFixVector::operator/= (const FIX n) {
	v.x = FixDiv (v.x, n), v.y = FixDiv (v.y, n), v.z = FixDiv (v.z, n);
	return *this;
	}

inline const CFixVector& CFixVector::operator*= (const DOUBLE n) {
	v.x = (FIX) Round ((double) v.x * n),
	v.y = (FIX) Round ((double) v.y * n),
	v.z = (FIX) Round ((double) v.z * n);
	return *this;
	}

inline const CFixVector& CFixVector::operator/= (const DOUBLE n) {
	v.x = (FIX) Round ((double) v.x / n),
	v.y = (FIX) Round ((double) v.y / n),
	v.z = (FIX) Round ((double) v.z / n);
	return *this;
	}

inline const CFixVector CFixVector::operator* (const FIX n) const {
	return CFixVector (FixMul (v.x, n), FixMul (v.y, n), FixMul (v.z, n));
	}

inline const CFixVector CFixVector::operator/ (const FIX n) const {
	return CFixVector (FixDiv (v.x , n), FixDiv (v.y , n), FixDiv (v.z , n));
	}

inline const CFixVector CFixVector::operator>> (const FIX n) {
	return CFixVector (v.x >> n, v.y >> n, v.z >> n);
	}

inline const CFixVector CFixVector::operator<< (const FIX n) {
	return CFixVector (v.x << n, v.y << n, v.z << n);
	}

inline const CFixVector& CFixVector::operator>>= (const FIX n) {
	v.x >>= n, v.y >>= n, v.z >>= n;
	return *this;
	}

inline const CFixVector& CFixVector::operator<<= (const FIX n) {
	v.x <<= n, v.y <<= n, v.z <<= n;
	return *this;
	}

inline const CFixVector CFixVector::operator* (CFixVector other) const {
	return CFixVector (FixMul (v.x, other.v.x), FixMul (v.y, other.v.y), FixMul (v.z, other.v.z));
	}

inline const CFixVector CFixVector::operator/ (CFixVector other) const {
	return CFixVector (FixDiv (v.x, other.v.x), FixDiv (v.y, other.v.y), FixDiv (v.z, other.v.z));
	}

inline const FIX CFixVector::operator^ (const CFixVector& other) const {
	return FIX ((double (v.x) * double (other.v.x) + double (v.y) * double (other.v.y) + double (v.z) * double (other.v.z)) / 65536.0);
	}

inline const FIX CFixVector::Mag (void) { return D2X (CDoubleVector (*this).Mag ()); }

#endif

// --------------------------------------------------------------------------

inline const bool CDoubleVector::operator== (const CDoubleVector other) {
	return (v.x == other.v.x) && (v.y == other.v.y) && (v.z == other.v.z);
}

inline DOUBLE& CDoubleVector::operator[] (const size_t i) { return ((DOUBLE*) &v) [i]; }

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

inline const CDoubleVector& CDoubleVector::operator+= (const CDoubleVector other) {
	v.x += other.v.x, v.y += other.v.y, v.z += other.v.z; 
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator-= (const CDoubleVector other) {
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

inline const CDoubleVector& CDoubleVector::operator*= (const DOUBLE n) {
	v.x *= n, v.y *= n, v.z *= n;
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator/= (const DOUBLE n) {
	v.x /= n, v.y /= n, v.z /= n;
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator*= (const CDoubleVector other) {
	v.x *= other.v.x, v.y *= other.v.y, v.z *= other.v.z;
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator/= (const CDoubleVector other) {
	v.x /= other.v.x, v.y /= other.v.y, v.z /= other.v.z;
	return *this;
	}

inline const CDoubleVector CDoubleVector::operator/ (const DOUBLE n) {
	return CDoubleVector (v.x / n, v.y / n, v.z / n);
	}

inline const CDoubleVector CDoubleVector::operator* (const DOUBLE n) {
	return CDoubleVector (v.x * n, v.y * n, v.z * n);
	}

inline const CDoubleVector CDoubleVector::operator* (CDoubleVector other) const {
	return CDoubleVector (v.x * other.v.x, v.y * other.v.y, v.z * other.v.z);
	}

inline const CDoubleVector CDoubleVector::operator/ (CDoubleVector other) const {
	return CDoubleVector (v.x / other.v.x, v.y / other.v.y, v.z / other.v.z);
	}

inline const DOUBLE CDoubleVector::operator^ (const CDoubleVector& other) const {
	return double (v.x) * double (other.v.x) + double (v.y) * double (other.v.y) + double (v.z) * double (other.v.z);
	}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

static inline const CDoubleVector CrossProduct (const CDoubleVector& v0, const CDoubleVector& v1) {
	return CDoubleVector (v0.v.y * v1.v.z - v0.v.z * v1.v.y, v0.v.z * v1.v.x - v0.v.x * v1.v.z, v0.v.x * v1.v.y - v0.v.y * v1.v.x);
	}

static inline CDoubleVector Perpendicular (const CDoubleVector& p0, const CDoubleVector& p1, const CDoubleVector& p2) {
	return CrossProduct (p1 - p0, p2 - p1);
	}

static inline const CDoubleVector Normalize (CDoubleVector v) { 
	DOUBLE m = v.Mag ();
	return (m != 0.0) ? v / m : CDoubleVector (0.0, 0.0, 0.0); 
	}

static inline const CDoubleVector Normal (const CDoubleVector& p0, const CDoubleVector& p1, const CDoubleVector& p2) {
	return Normalize (CrossProduct (p1 - p0, p2 - p0));
	}

static inline const DOUBLE Normal (CDoubleVector& normal, const CDoubleVector& p0, const CDoubleVector& p1, const CDoubleVector& p2) {
	normal = CrossProduct (p1 - p0, p2 - p0);
	DOUBLE m = normal.Mag ();
	if (m > 0.0)
		normal *= m;
	return m;
	}

#if !FIX_IS_DOUBLE
static inline FIX Dot (const CFixVector& v0, const CFixVector& v1) {
	return FIX ((double (v0.v.x) * double (v1.v.x) + double (v0.v.y) * double (v1.v.y) + double (v0.v.z) * double (v1.v.z)) / 65536.0);
	}

static inline CFixVector Min (const CFixVector& v0, const CFixVector& v1) {
	return CFixVector(min (v0.v.x, v1.v.x), min (v0.v.y, v1.v.y), min (v0.v.z, v1.v.z));
	}

static inline CFixVector Max (const CFixVector& v0, const CFixVector& v1) {
	return CFixVector(max (v0.v.x, v1.v.x), max (v0.v.y, v1.v.y), max (v0.v.z, v1.v.z));
	}

static inline DOUBLE Distance (const CFixVector& p0, const CFixVector& p1) {
	CFixVector v = p0 - p1;
	return D2X (CDoubleVector (v).Mag ());
	}

static inline CFixVector Average (const CFixVector& p0, const CFixVector& p1) {
	CFixVector v = p0 + p1;
	v /= FIX (2);
	return v;
	}
#endif

static inline DOUBLE Dot (const CDoubleVector& v0, const CDoubleVector& v1) {
	return double (v0.v.x) * double (v1.v.x) + double (v0.v.y) * double (v1.v.y) + double (v0.v.z) * double (v1.v.z);
	}

static inline CDoubleVector Min (const CDoubleVector& v0, const CDoubleVector& v1) {
	return CDoubleVector(min (v0.v.x, v1.v.x), min (v0.v.y, v1.v.y), min (v0.v.z, v1.v.z));
	}

static inline CDoubleVector Max (const CDoubleVector& v0, const CDoubleVector& v1) {
	return CDoubleVector(max (v0.v.x, v1.v.x), max (v0.v.y, v1.v.y), max (v0.v.z, v1.v.z));
	}

static inline DOUBLE Distance (const CDoubleVector& p0, const CDoubleVector& p1) {
	CDoubleVector v = p0 - p1;
	return v.Mag ();
	}

static inline CDoubleVector Average (const CDoubleVector& p0, const CDoubleVector& p1) {
	CDoubleVector v = p0 + p1;
	v /= DOUBLE (2);
	return v;
	}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

#if FIX_IS_DOUBLE

#define CFixMatrix	CDoubleMatrix

#else

class CFixMatrix {
public:
	CFixVector rVec, uVec, fVec;

	inline INT32 Read (FILE* fp) { 
		rVec.Read (fp);
		uVec.Read (fp);
		fVec.Read (fp);
		return 1;
	}

	inline void Write (FILE* fp) { 
		rVec.Write (fp);
		uVec.Write (fp);
		fVec.Write (fp);
	}

	CFixMatrix ();
	CFixMatrix (FIX x1, FIX y1, FIX z1, FIX x2, FIX y2, FIX z2, FIX x3, FIX y3, FIX z3);
	CFixMatrix (CFixMatrix& m) : rVec(m.rVec), uVec(m.uVec), fVec(m.fVec) {}
	CFixMatrix (CFixVector& r, CFixVector& u, CFixVector& f) : rVec (r), uVec (u), fVec (f) {}
	CFixMatrix (FIX sinp, FIX cosp, FIX sinb, FIX cosb, FIX sinh, FIX cosh);
	//computes a matrix from a Set of three angles.  returns ptr to matrix
	CFixMatrix (CAngleVector& a);
	//computes a matrix from a forward vector and an angle
	CFixMatrix (CFixVector *v, FIXANG a);

	CFixMatrix& CFixMatrix::Set (FIX x1, FIX y1, FIX z1, FIX x2, FIX y2, FIX z2, FIX x3, FIX y3, FIX z3);
	CFixMatrix& Set (FIX sinp, FIX cosp, FIX sinb, FIX cosb, FIX sinh, FIX cosh);

	CFixMatrix& Invert (CFixMatrix& m);
	CFixMatrix Mul (const CFixMatrix& m);
	CFixMatrix& Scale (CFixVector& scale);

	const CFixVector operator* (const CFixVector& v);
	inline const CFixMatrix operator* (const CFixMatrix& other) { return Mul (other); }
	inline const CFixMatrix& operator= (const CFixMatrix& other) { 
		rVec = other.rVec, uVec = other.uVec, fVec = other.fVec;
		return *this;
		}

	const FIX Det (void);
	const CFixMatrix Inverse (void);
	inline const CFixMatrix Transpose (void);
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

CFixMatrix& Transpose (CFixMatrix& dest, CFixMatrix& src);

inline CFixMatrix& Transpose (CFixMatrix& m)
{
Swap (m.rVec.v.y, m.uVec.v.x);
Swap (m.rVec.v.z, m.fVec.v.x);
Swap (m.uVec.v.z, m.fVec.v.y);
return m;
}

// --------------------------------------------------------------------------

inline const CFixMatrix CFixMatrix::Transpose (void)
{
CFixMatrix m;
::Transpose (m, *this);
return m;
}

// --------------------------------------------------------------------------

inline const CFixVector CFixMatrix::operator* (const CFixVector& v)
{
return CFixVector (v ^ rVec, v ^ uVec, v ^ fVec);
}

#endif

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class CDoubleMatrix {
public:
	CDoubleVector rVec, uVec, fVec;

	inline INT32 Read (FILE* fp) { 
		rVec.Read (fp);
		uVec.Read (fp);
		fVec.Read (fp);
		return 1;
	}

	inline void Write (FILE* fp) { 
		rVec.Write (fp);
		uVec.Write (fp);
		fVec.Write (fp);
	}

	CDoubleMatrix::CDoubleMatrix ();
	CDoubleMatrix::CDoubleMatrix (DOUBLE x1, DOUBLE y1, DOUBLE z1, DOUBLE x2, DOUBLE y2, DOUBLE z2, DOUBLE x3, DOUBLE y3, DOUBLE z3);
	CDoubleMatrix (CDoubleMatrix& m) : rVec(m.rVec), uVec(m.uVec), fVec(m.fVec) {}
	CDoubleMatrix (CDoubleVector r, CDoubleVector u, CDoubleVector f) : rVec (r), uVec (u), fVec (f) {}
	CDoubleMatrix (DOUBLE sinp, DOUBLE cosp, DOUBLE sinb, DOUBLE cosb, DOUBLE sinh, DOUBLE cosh);
	//computes a matrix from a Set of three angles.  returns ptr to matrix
	CDoubleMatrix (CAngleVector& a);
	//computes a matrix from a forward vector and an angle
	CDoubleMatrix (CDoubleVector *v, FIXANG a);

	CDoubleMatrix& Set (DOUBLE x1, DOUBLE y1, DOUBLE z1, DOUBLE x2, DOUBLE y2, DOUBLE z2, DOUBLE x3, DOUBLE y3, DOUBLE z3);
	CDoubleMatrix& Set (DOUBLE sinp, DOUBLE cosp, DOUBLE sinb, DOUBLE cosb, DOUBLE sinh, DOUBLE cosh);

	CDoubleMatrix& Invert (CDoubleMatrix& m);
	CDoubleMatrix Mul (const CDoubleMatrix& m);
	CDoubleMatrix& Scale (CDoubleVector& scale);

	const CDoubleVector operator* (const CDoubleVector& v);
	inline const CDoubleMatrix operator* (const CDoubleMatrix& other) { return Mul (other); }
	inline const CDoubleMatrix& operator= (const CDoubleMatrix& other) { 
		rVec = other.rVec, uVec = other.uVec, fVec = other.fVec;
		return *this;
		}

	const DOUBLE Det (void);
	const CDoubleMatrix Inverse (void);
	inline const CDoubleMatrix Transpose (void);
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

CDoubleMatrix& Transpose (CDoubleMatrix& dest, CDoubleMatrix& src);

inline CDoubleMatrix& Transpose (CDoubleMatrix& m)
{
Swap (m.rVec.v.y, m.uVec.v.x);
Swap (m.rVec.v.z, m.fVec.v.x);
Swap (m.uVec.v.z, m.fVec.v.y);
return m;
}

// --------------------------------------------------------------------------

inline const CDoubleMatrix CDoubleMatrix::Transpose (void)
{
CDoubleMatrix m;
::Transpose (m, *this);
return m;
}

// --------------------------------------------------------------------------

inline const CDoubleVector CDoubleMatrix::operator* (const CDoubleVector& v)
{
return CDoubleVector (v ^ rVec, v ^ uVec, v ^ fVec);
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

#endif // VECTOR_TYPES_H

