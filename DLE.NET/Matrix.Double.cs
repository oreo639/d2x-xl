using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public class DoubleMatrix
    {

        public DoubleVector rVec = new DoubleVector ();
        public DoubleVector uVec = new DoubleVector ();
        public DoubleVector fVec = new DoubleVector ();

        // ------------------------------------------------------------------------

        #region c'tors

        public DoubleMatrix ()
        {
            Clear ();
        }

        // ------------------------------------------------------------------------

        public DoubleMatrix (double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
        {
            Set (x1, y1, z1, x2, y2, z2, x3, y3, z3);
        }

        // ------------------------------------------------------------------------

        public DoubleMatrix (DoubleVector r, DoubleVector u, DoubleVector f)
        {
            rVec.Set (r);
            uVec.Set (u);
            fVec.Set (f);
        }

        // ------------------------------------------------------------------------

        public DoubleMatrix (DoubleMatrix m)
        {
            rVec.Set (m.rVec);
            uVec.Set (m.uVec);
            fVec.Set (m.fVec);
        }

        // ------------------------------------------------------------------------

        public DoubleMatrix (double sinp, double cosp, double sinb, double cosb, double sinh, double cosh)
        {
            Set (sinp, cosp, sinb, cosb, sinh, cosh);
        }

        // ------------------------------------------------------------------------

        public DoubleMatrix (AngleVector a)
        {
        }

        // ------------------------------------------------------------------------

        #endregion

        // ------------------------------------------------------------------------

        #region initializers

        public void Clear ()
        {
            rVec.Set (1.0, 0.0, 0.0);
            uVec.Set (0.0, 1.0, 0.0);
            fVec.Set (0.0, 0.0, 1.0);
        }

        // ------------------------------------------------------------------------

        public void Set (DoubleMatrix other)
        {
            rVec.Set (other.rVec);
            uVec.Set (other.uVec);
            fVec.Set (other.fVec);
        }

        // ------------------------------------------------------------------------

        public void Set (double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
        {
            rVec.Set (x1, y1, z1);
            uVec.Set (x2, y2, z2);
            fVec.Set (x3, y3, z3);
        }

        // ------------------------------------------------------------------------

        public DoubleMatrix Set (double sinp, double cosp, double sinb, double cosb, double sinh, double cosh)
        {
        double sbsh = sinb * sinh;
        double cbch = cosb * cosh;
        double cbsh = cosb * sinh;
        double sbch = sinb * cosh;

        rVec.Set (cbch + sinp * sbsh, sinb * cosp, sinp * sbch - cbsh);
        uVec.Set (sinp * cbsh - sbch, cosb * cosp, sbsh + sinp * cbch);
        fVec.Set (sinh * cosp, cosh * cosp, -sinp);
        return this;
        }

        #endregion

        // ------------------------------------------------------------------------

        #region operations

        public DoubleMatrix Mul (DoubleMatrix other) 
        {
	        DoubleVector v = new DoubleVector ();
	        DoubleMatrix m = new DoubleMatrix ();

            v.Set (other.rVec.v.x, other.uVec.v.x, other.fVec.v.x);
            m.rVec.v.x = v ^ rVec;
            m.uVec.v.x = v ^ uVec;
            m.fVec.v.x = v ^ fVec;
            v.Set (other.rVec.v.y, other.uVec.v.y, other.fVec.v.y);
            m.rVec.v.y = v ^ rVec;
            m.uVec.v.y = v ^ uVec;
            m.fVec.v.y = v ^ fVec;
            v.Set (other.rVec.v.z, other.uVec.v.z, other.fVec.v.z);
            m.rVec.v.z = v ^ rVec;
            m.uVec.v.z = v ^ uVec;
            m.fVec.v.z = v ^ fVec;
            return m;
        }

        // ------------------------------------------------------------------------

        public double Det () 
        {
            return rVec.v.x * (fVec.v.y * uVec.v.z - uVec.v.y * fVec.v.z) +
		           uVec.v.x * (rVec.v.y * fVec.v.z - fVec.v.y * rVec.v.z) +
		           fVec.v.x * (uVec.v.y * rVec.v.z - rVec.v.y * uVec.v.z);
        }

        // ------------------------------------------------------------------------

        public DoubleMatrix Inverse () 
        {
	        DoubleMatrix m = new DoubleMatrix ();

            double det = Det ();
            if (det != 0.0) {
	            m.rVec.v.x = (fVec.v.y * uVec.v.z - uVec.v.y * fVec.v.z) / det;
	            m.rVec.v.y = (rVec.v.y * fVec.v.z - fVec.v.y * rVec.v.z) / det;
	            m.rVec.v.z = (uVec.v.y * rVec.v.z - rVec.v.y * uVec.v.z) / det;

	            m.uVec.v.x = (uVec.v.x * fVec.v.z - fVec.v.x * uVec.v.z) / det;
	            m.uVec.v.y = (fVec.v.x * rVec.v.z - rVec.v.x * fVec.v.z) / det;
	            m.uVec.v.z = (rVec.v.x * uVec.v.z - uVec.v.x * rVec.v.z) / det;

	            m.fVec.v.x = (fVec.v.x * uVec.v.y - uVec.v.x * fVec.v.y) / det;
	            m.fVec.v.y = (rVec.v.x * fVec.v.y - fVec.v.x * rVec.v.y) / det;
	            m.fVec.v.z = (uVec.v.x * rVec.v.y - rVec.v.x * uVec.v.y) / det;
	            }
            return m;
        }

        // ------------------------------------------------------------------------

        public DoubleMatrix Adjoint () 
        {
	        DoubleMatrix m = new DoubleMatrix ();

            m.rVec.v.x = uVec.v.y * fVec.v.z - uVec.v.z * fVec.v.y;
            m.rVec.v.y = rVec.v.z * fVec.v.y - rVec.v.y * fVec.v.z;
            m.rVec.v.z = rVec.v.y * uVec.v.z - rVec.v.z * uVec.v.y;
            m.uVec.v.x = uVec.v.z * fVec.v.x - uVec.v.x * fVec.v.z;
            m.uVec.v.y = rVec.v.x * fVec.v.z - rVec.v.z * fVec.v.x;
            m.uVec.v.z = rVec.v.z * uVec.v.x - rVec.v.x * uVec.v.z;
            m.fVec.v.x = uVec.v.x * fVec.v.y - uVec.v.y * fVec.v.x;
            m.fVec.v.y = rVec.v.y * fVec.v.x - rVec.v.x * fVec.v.y;
            m.fVec.v.z = rVec.v.x * uVec.v.y - rVec.v.y * uVec.v.x;
            return m;
        }

        // ------------------------------------------------------------------------

        public DoubleMatrix Transpose (DoubleMatrix dest, DoubleMatrix src)
        {
            dest.rVec.v.x = src.rVec.v.x;
            dest.uVec.v.x = src.rVec.v.y;
            dest.fVec.v.x = src.rVec.v.z;
            dest.rVec.v.y = src.uVec.v.x;
            dest.uVec.v.y = src.uVec.v.y;
            dest.fVec.v.y = src.uVec.v.z;
            dest.rVec.v.z = src.fVec.v.x;
            dest.uVec.v.z = src.fVec.v.y;
            dest.fVec.v.z = src.fVec.v.z;
            return dest;
        }

        // ------------------------------------------------------------------------

        public void Rotate (double angle, char axis) 
        {
        double cosX = Math.Cos (angle);
        double sinX = Math.Sin (angle);

        DoubleMatrix mRot = new DoubleMatrix ();

        switch (axis) {
	        case 'x':
		        // spin x
		        //	1	0	0
		        //	0	cos	sin
		        //	0	-sin	cos
		        //
		        mRot.uVec.Set (uVec.v.x * cosX + fVec.v.x * sinX, 
					           uVec.v.y * cosX + fVec.v.y * sinX,
						       uVec.v.z * cosX + fVec.v.z * sinX);
		        mRot.fVec.Set (fVec.v.x * cosX - uVec.v.x * sinX,
							   fVec.v.y * cosX - uVec.v.y * sinX,
							   fVec.v.z * cosX - uVec.v.z * sinX);
		        uVec = mRot.uVec;
		        fVec = mRot.fVec;
		        break;

	        case 'y':
		        // spin y
		        //	cos	0	-sin
		        //	0	1	0
		        //	sin	0	cos
		        //
		        mRot.rVec.Set (rVec.v.x * cosX - fVec.v.x * sinX, 
							   rVec.v.y * cosX - fVec.v.y * sinX, 
							   rVec.v.z * cosX - fVec.v.z * sinX);
		        mRot.fVec.Set (rVec.v.x * sinX + fVec.v.x * cosX, 
							   rVec.v.y * sinX + fVec.v.y * cosX,
							   rVec.v.z * sinX + fVec.v.z * cosX);
		        rVec = mRot.rVec;
		        fVec = mRot.fVec;
		        break;

	        case 'z':
		        // spin z
		        //	cos	sin	0
		        //	-sin	cos	0
		        //	0	0	1
		        mRot.rVec.Set (rVec.v.x * cosX + uVec.v.x * sinX,
							   rVec.v.y * cosX + uVec.v.y * sinX,
							   rVec.v.z * cosX + uVec.v.z * sinX);
		        mRot.uVec.Set (uVec.v.x * cosX - rVec.v.x * sinX,
							   uVec.v.y * cosX - rVec.v.y * sinX,
							   uVec.v.z * cosX - rVec.v.z * sinX);
		        rVec = mRot.rVec;
		        uVec = mRot.uVec;
		        break;
	        }
        }

        // ------------------------------------------------------------------------

        public void Square2Quad (LongVector [] a) 
        {
            // infer "unity square" to "quad" prespective transformation
            // see page 55-56 of Digital Image Warping by George Wolberg (3rd edition) 
            double dx1 = a [1].x - a [2].x;
            double dx2 = a [3].x - a [2].x;
            double dx3 = a [0].x - a [1].x + a [2].x - a [3].x;
            double dy1 = a [1].y - a [2].y;
            double dy2 = a [3].y - a [2].y;
            double dy3 = a [0].y - a [1].y + a [2].y - a [3].y;
            double w = (dx1 * dy2 - dx2 * dy1);
            if (w == 0.0) 
	            w = 1.0;

            rVec.v.z = (dx3 * dy2 - dx2 * dy3) / w;
            uVec.v.z = (dx1 * dy3 - dx3 * dy1) / w;
            rVec.v.x = a [1].x - a [0].x + rVec.v.z * a [1].x;
            uVec.v.x = a [3].x - a [0].x + uVec.v.z * a [3].x;
            fVec.v.x = a [0].x;
            rVec.v.y = a [1].y - a [0].y + rVec.v.z * a [1].y;
            uVec.v.y = a [3].y - a [0].y + uVec.v.z * a [3].y;
            fVec.v.y = a [0].y;
            fVec.v.z = 1;
        }

        // ------------------------------------------------------------------------

        public void Scale (double scale) 
        {
            Mul (new DoubleMatrix (scale, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, 1.0));
        }

        #endregion

        // ------------------------------------------------------------------------

        #region operators

        // ------------------------------------------------------------------------

        public static DoubleMatrix operator * (DoubleMatrix a, DoubleMatrix b)
        {
            return a.Mul (b);
        }

        public static bool operator == (DoubleMatrix v1, DoubleMatrix v2)
        {
            return (v1.rVec == v2.rVec) && (v1.uVec == v2.uVec) && (v1.fVec == v2.fVec);
        }

        public static bool operator != (DoubleMatrix v1, DoubleMatrix v2)
        {
            return (v1.rVec != v2.rVec) || (v1.uVec != v2.uVec) || (v1.fVec != v2.fVec);
        }

        #endregion

        // ------------------------------------------------------------------------

        #region Equality functions

        public override bool Equals (System.Object obj)
        {
            // If parameter is null return false.
            if (obj == null)
            {
                return false;
            }

            // If parameter cannot be cast to Point return false.
            DoubleMatrix other = obj as DoubleMatrix;
            if ((System.Object)other == null)
            {
                return false;
            }

            // Return true if the fields match:
            return (rVec == other.rVec) && (uVec == other.uVec) && (fVec == other.fVec);
        }

        // ------------------------------------------------------------------------

        public bool Equals (DoubleMatrix other)
        {
            // If parameter is null return false:
            if ((object)other == null)
            {
                return false;
            }

            // Return true if the fields match:
            return (rVec == other.rVec) && (uVec == other.uVec) && (fVec == other.fVec);
        }

        // ------------------------------------------------------------------------

        public override int GetHashCode ()
        {
            return rVec.GetHashCode () ^ uVec.GetHashCode () ^ fVec.GetHashCode ();
        }

        #endregion

        // ------------------------------------------------------------------------

    }
}
