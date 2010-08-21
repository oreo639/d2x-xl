#ifndef VECTOR_TYPES_H
#define VECTOR_TYPES_H

#include <math.h>

struct tAngleVector;
class CAngleVector;
struct tFixVector;
class CFixVector;
struct tDoubleVector;
class CDoubleVector;
class CFixMatrix;

// --------------------------------------------------------------------------

inline double Round (double value, double round = 1.0) 
{
return (value >= 0) ? value + round / 2.0 : value - round / 2.0;
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

typedef struct tAngleVector {
public:
	FIXANG p, b, h;
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

	inline INT32 Read (FILE* fp) { 
		v.p = read_FIXANG (fp);
		v.b = read_FIXANG (fp);
		v.h = read_FIXANG (fp);
		return 1;
		}

	void Write (FILE* fp) { 
		write_FIXANG (v.p, fp);
		write_FIXANG (v.b, fp);
		write_FIXANG (v.h, fp);
		}

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

struct tFixVector {
public:
	FIX x, y, z;
};

class CFixVector {
public:
	tFixVector	v;
	//FIX x, y, z;
	CFixVector ()  { v.x = 0, v.y = 0, v.z = 0; }
	CFixVector (FIX x, FIX y, FIX z) { v.x = x, v.y = y, v.z = z; }
	CFixVector (tFixVector& _v) { v.x = _v.x, v.y = _v.y, v.z = _v.z; }
	CFixVector (CDoubleVector& _v); 
	void Set (FIX x, FIX y, FIX z) { v.x = x, v.y = y, v.z = z; }
	void Clear (void) { Set (0,0,0); }


inline INT32 Read (FILE* fp) { 
	v.x = read_FIX (fp);
	v.y = read_FIX (fp);
	v.z = read_FIX (fp);
	return 1;
	}

void Write (FILE* fp) { 
	write_FIX (v.x, fp);
	write_FIX (v.y, fp);
	write_FIX (v.z, fp);
	}

inline const CFixVector& operator= (const tFixVector& other);
inline const CFixVector& operator= (const CFixVector& other);
inline const CFixVector& operator= (const tDoubleVector& other);
inline const CFixVector& operator= (const CDoubleVector& other);
inline const CFixVector& operator+= (const CFixVector other);
inline const CFixVector& operator-= (const CFixVector other);
inline const CFixVector& operator*= (const FIX n);
inline const CFixVector& operator/= (const FIX n);
inline const CFixVector operator+ (const CFixVector& other);
inline const CFixVector operator- (const CFixVector& other);
inline const CFixVector operator- (void) const;
inline const CFixVector operator>> (const FIX n);
inline const CFixVector operator<< (const FIX n);
inline const CFixVector& operator>>= (const FIX n);
inline const CFixVector& operator<<= (const FIX n);
inline const CFixVector operator* (CFixVector& other);
inline const CFixVector operator/ (CFixVector& other);
inline const CFixVector operator* (FIX n);
inline const CFixVector operator/ (FIX n);
inline const FIX operator^ (CFixVector& other);
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

struct tDoubleVector {
public:
	DOUBLE x, y, z;
};

class CDoubleVector {
public:
	tDoubleVector	v;
	//DOUBLE x, y, z;
	CDoubleVector ()  { v.x = 0, v.y = 0, v.z = 0; }
	CDoubleVector (DOUBLE x, DOUBLE y, DOUBLE z) { v.x = x, v.y = y, v.z = z; }
	CDoubleVector (tDoubleVector& _v) { v.x = _v.x, v.y = _v.y, v.z = _v.z; }
	CDoubleVector (CFixVector& _v);
	//CDoubleVector (CDoubleVector& _v) { v.x = _v.v.x, v.y = _v.v.y, v.z = _v.v.z; }
	void Set (DOUBLE x, DOUBLE y, DOUBLE z) { v.x = x, v.y = y, v.z = z; }
	void Clear (void) { Set (0,0,0); }


inline INT32 Read (FILE* fp) { 
	v.x = read_DOUBLE (fp);
	v.y = read_DOUBLE (fp);
	v.z = read_DOUBLE (fp);
	return 1;
	}

void Write (FILE* fp) { 
	write_DOUBLE (v.x, fp);
	write_DOUBLE (v.y, fp);
	write_DOUBLE (v.z, fp);
	}

inline const CDoubleVector& operator= (const tDoubleVector& other);
inline const CDoubleVector& operator= (const CDoubleVector& other);
inline const CDoubleVector& operator= (const tFixVector& other);
inline const CDoubleVector& operator= (const CFixVector& other);
inline const CDoubleVector& operator+= (const CDoubleVector other);
inline const CDoubleVector& operator-= (const CDoubleVector other);
inline const CDoubleVector& operator*= (const DOUBLE n);
inline const CDoubleVector& operator/= (const DOUBLE n);
inline const CDoubleVector operator+ (const CDoubleVector& other) const;
inline const CDoubleVector operator- (const CDoubleVector& other) const;
inline const CDoubleVector operator- (void) const;
inline const CDoubleVector operator* (CDoubleVector& other);
inline const CDoubleVector operator/ (CDoubleVector& other);
inline const CDoubleVector operator* (DOUBLE n);
inline const CDoubleVector operator/ (DOUBLE n);
inline const DOUBLE operator^ (CDoubleVector& other);

inline const DOUBLE Mag (void) { return sqrt (v.x * v.x + v.y * v.y + v.z * v.z); }
inline const CDoubleVector& Normalize (void) { *this /= Mag (); return *this; }
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

class CFixMatrix {
public:
	CFixVector rvec, uvec, fvec;

	inline INT32 Read (FILE* fp) { 
		rvec.Read (fp);
		uvec.Read (fp);
		fvec.Read (fp);
		return 1;
	}

	inline void Write (FILE* fp) { 
		rvec.Write (fp);
		uvec.Write (fp);
		fvec.Write (fp);
	}
};

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

CFixVector::CFixVector (CDoubleVector& _v) { 
	v.x = FIX (Round (_v.v.x)), v.y = FIX (Round (_v.v.y)), v.z = FIX (Round (_v.v.z)); 
}

inline const CFixVector& CFixVector::operator= (const tFixVector& other) { 
	v.x = other.x, v.y = other.y, v.z = other.z; 
	return *this;
	}

inline const CFixVector& CFixVector::operator= (const CFixVector& other) { 
	v.x = other.v.x, v.y = other.v.y, v.z = other.v.z; 
	return *this;
	}

inline const CFixVector& CFixVector::operator= (const tDoubleVector& other) { 
	v.x = FIX (other.x), v.y = FIX (other.y), v.z = FIX (other.z); 
	return *this;
	}

inline const CFixVector& CFixVector::operator= (const CDoubleVector& other) { 
	v.x = FIX (other.v.x), v.y = FIX (other.v.y), v.z = FIX (other.v.z); 
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

inline const CFixVector CFixVector::operator+ (const CFixVector& other) {
	return CFixVector (v.x + other.v.x, v.y + other.v.y, v.z + other.v.z);
	}

inline const CFixVector CFixVector::operator- (const CFixVector& other) {
	return CFixVector (v.x - other.v.x, v.y - other.v.y, v.z - other.v.z);
	}

inline const CFixVector CFixVector::operator- (void) const {
	return CFixVector (-v.x, -v.y, -v.z);
	}

inline const CFixVector& CFixVector::operator*= (const FIX n) {
	v.x *= n, v.y *= n, v.z *= n;
	return *this;
	}

inline const CFixVector& CFixVector::operator/= (const FIX n) {
	v.x /= n, v.y /= n, v.z /= n;
	return *this;
	}

inline const CFixVector CFixVector::operator* (const FIX n) {
	return CFixVector (v.x * n, v.y * n, v.z * n);
	}

inline const CFixVector CFixVector::operator/ (const FIX n) {
	return CFixVector (v.x / n, v.y / n, v.z / n);
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

inline const CFixVector CFixVector::operator* (CFixVector& other) {
	return CFixVector (v.x * other.v.x, v.y * other.v.y, v.z * other.v.z);
	}

inline const CFixVector CFixVector::operator/ (CFixVector& other) {
	return CFixVector (v.x / other.v.x, v.y / other.v.y, v.z / other.v.z);
	}

inline const FIX CFixVector::operator^ (CFixVector& other) {
	return FIX ((double (v.x) * double (other.v.x) + double (v.y) * double (other.v.y) + double (v.z) * double (other.v.z)) / 65536.0);
	}

// --------------------------------------------------------------------------

CDoubleVector::CDoubleVector (CFixVector& _v) { 
	v.x = DOUBLE (_v.v.x), v.y = DOUBLE (_v.v.y), v.z = DOUBLE (_v.v.z); 
}

inline const CDoubleVector& CDoubleVector::operator= (const tDoubleVector& other) { 
	v.x = other.x, v.y = other.y, v.z = other.z; 
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator= (const CDoubleVector& other) { 
	v.x = other.v.x, v.y = other.v.y, v.z = other.v.z; 
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator= (const tFixVector& other) { 
	v.x = other.x, v.y = other.y, v.z = other.z; 
	return *this;
	}

inline const CDoubleVector& CDoubleVector::operator= (const CFixVector& other) { 
	v.x = other.v.x, v.y = other.v.y, v.z = other.v.z; 
	return *this;
	}

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

inline const CDoubleVector CDoubleVector::operator/ (const DOUBLE n) {
	return CDoubleVector (v.x / n, v.y / n, v.z / n);
	}

inline const CDoubleVector CDoubleVector::operator* (const DOUBLE n) {
	return CDoubleVector (v.x * n, v.y * n, v.z * n);
	}

inline const CDoubleVector CDoubleVector::operator* (CDoubleVector& other) {
	return CDoubleVector (v.x * other.v.x, v.y * other.v.y, v.z * other.v.z);
	}

inline const CDoubleVector CDoubleVector::operator/ (CDoubleVector& other) {
	return CDoubleVector (v.x / other.v.x, v.y / other.v.y, v.z / other.v.z);
	}

inline const DOUBLE CDoubleVector::operator^ (CDoubleVector& other) {
	return DOUBLE ((double (v.x) * double (other.v.x) + double (v.y) * double (other.v.y) + double (v.z) * double (other.v.z)) / 65536.0);
	}

inline const CDoubleVector CrossProduct (const CDoubleVector& v0, const CDoubleVector& v1) {
	return CDoubleVector (v0.v.y * v1.v.z - v0.v.z * v1.v.y, v0.v.z * v1.v.x - v0.v.x * v1.v.z, v0.v.x * v1.v.y - v0.v.y * v1.v.x);
	}

inline CDoubleVector Perpendicular (const CDoubleVector& p0, const CDoubleVector& p1, const CDoubleVector& p2) {
	return CrossProduct (p1 - p0, p2 - p1);
	}

inline const CDoubleVector Normalize (CDoubleVector v) { 
	return v / v.Mag (); 
	}

inline const CDoubleVector Normal (const CDoubleVector& p0, const CDoubleVector& p1, const CDoubleVector& p2) {
	return Normalize (CrossProduct (p1 - p0, p2 - p1));
	}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

#endif // VECTOR_TYPES_H

