#ifndef __Quaternion_h
#define __Quaternion_h

#include <math.h>
#include "Vector.h"
#include "Matrix.h"

class CQuaternion {
	public:
		double x, y, z, w;

	CQuaternion () : x (0.0), y (0.0), z (0.0), w (0.0) {}

	CQuaternion (double x, double y, double z, double w) : x (x), y (y), z (z), w (w) {}

	CQuaternion& operator= (CQuaternion other) {
		x = other.x, y = other.y, z = other.z, w = other.w; 
		return *this;
		}

	CQuaternion GetConjugate (void) { return CQuaternion (-x, -y, -z, w); };

	void Normalize (void);

	CQuaternion CQuaternion::operator* (CQuaternion other);

	CDoubleVector CQuaternion::operator* (CDoubleVector v);

	void FromAxisAngle (CDoubleVector axis, double angle);

	void FromEuler (double pitch, double yaw, double roll);

	void FromMatrix (CDoubleMatrix& m);

	CDoubleMatrix ToMatrix (void);

	void ToAxisAngle (CDoubleVector& axis, double& angle);
	};

#endif //__Quaternion_h
