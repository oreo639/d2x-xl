using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class DoubleVector
    {
        struct tDoubleVector 
        {
            public double x, y, z;

            public void Read (BinaryReader fp) 
            { 
	            x = FixConverter.X2D (fp.ReadInt32 ());
	            y = FixConverter.X2D (fp.ReadInt32 ());
	            z = FixConverter.X2D (fp.ReadInt32 ());
	        }

            public void Write (BinaryWriter fp) 
            { 
	            fp.Write (FixConverter.D2X (x));
	            fp.Write (FixConverter.D2X (y));
	            fp.Write (FixConverter.D2X (z));
	        }
        }

        public tDoubleVector v;
	    //double x; y; z;
	    DoubleVector ()  { v.x = 0; v.y = 0; v.z = 0; }
	    DoubleVector (double x, double y, double z) { v.x = x; v.y = y; v.z = z; }
	    DoubleVector (tDoubleVector _v) { v.x = _v.x; v.y = _v.y; v.z = _v.z; }
	    DoubleVector (FixVector _v);
	    //DoubleVector (DoubleVector _v) { v.x = _v.v.x; v.y = _v.v.y; v.z = _v.v.z; }

	    void Set (double x, double y, double z) 
        { 
            v.x = x; 
            v.y = y; 
            v.z = z; 
        }

	    void Clear () 
        { 
            Set (0, 0, 0); 
        }

        void Read (BinaryReader fp) 
        { 
            v.Read (fp); 
        }

        void Write (BinaryWrite fp) { v.Write (fp); }
        bool operator== (DoubleVector other) {
	        return (v.x == other.v.x) && (v.y == other.v.y) && (v.z == other.v.z);
}

        double operator[] (size_t i) { return ((double*) &v) [i]; }

        DoubleVector operator= (tDoubleVector& other) 
        { 
	        v.x = other.x; v.y = other.y; v.z = other.z; 
	        return *this;
	    }

        DoubleVector operator= (DoubleVector other) { 
	        v.x = other.v.x; v.y = other.v.y; v.z = other.v.z; 
	        return *this;
	        }

        #if !FIX_IS_DOUBLE
        DoubleVector operator= (tFixVector& other) { 
	        v.x = X2D (other.x); v.y = X2D (other.y); v.z = X2D (other.z); 
	        return *this;
	        }

        DoubleVector operator= (CFixVector& other) { 
	        v.x = X2D (other.v.x); v.y = X2D (other.v.y); v.z = X2D (other.v.z); 
	        return *this;
	        }
        #endif

        DoubleVector operator+= (DoubleVector other) {
	        v.x += other.v.x; v.y += other.v.y; v.z += other.v.z; 
	        return *this;
	        }

        DoubleVector operator-= (DoubleVector other) {
	        v.x -= other.v.x; v.y -= other.v.y; v.z -= other.v.z; 
	        return *this;
	        }

        DoubleVector operator+ (DoubleVector other) {
	        return DoubleVector (v.x + other.v.x; v.y + other.v.y; v.z + other.v.z);
	        }

        DoubleVector operator- (DoubleVector other) {
	        return DoubleVector (v.x - other.v.x; v.y - other.v.y; v.z - other.v.z);
	        }

        DoubleVector operator- () {
	        return DoubleVector (-v.x; -v.y; -v.z);
	        }

        DoubleVector operator*= (double n) {
	        v.x *= n; v.y *= n; v.z *= n;
	        return *this;
	        }

        DoubleVector operator/= (double n) {
	        v.x /= n; v.y /= n; v.z /= n;
	        return *this;
	        }

        DoubleVector operator*= (DoubleVector other) {
	        v.x *= other.v.x; v.y *= other.v.y; v.z *= other.v.z;
	        return *this;
	        }

        DoubleVector operator/= (DoubleVector other) {
	        v.x /= other.v.x; v.y /= other.v.y; v.z /= other.v.z;
	        return *this;
	        }

        DoubleVector operator/ (double n) {
	        return DoubleVector (v.x / n; v.y / n; v.z / n);
	        }

        DoubleVector operator* (double n) {
	        return DoubleVector (v.x * n; v.y * n; v.z * n);
	        }

        DoubleVector operator* (DoubleVector other) {
	        return DoubleVector (v.x * other.v.x; v.y * other.v.y; v.z * other.v.z);
	        }

        DoubleVector operator/ (DoubleVector other) {
	        return DoubleVector (v.x / other.v.x; v.y / other.v.y; v.z / other.v.z);
	        }

        double operator^ (DoubleVector other) {
	        return double (v.x) * double (other.v.x) + double (v.y) * double (other.v.y) + double (v.z) * double (other.v.z);
	        }


    double Mag () { return sqrt (v.x * v.x + v.y * v.y + v.z * v.z); }
    DoubleVector Normalize () { *this /= Mag (); return *this; }
    void Rotate (DoubleVector origin; DoubleVector normal; double angle);
    };

    }
}
