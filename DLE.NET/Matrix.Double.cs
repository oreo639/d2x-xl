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

        DoubleMatrix ()
        {
            Clear ();
        }

        // ------------------------------------------------------------------------

        DoubleMatrix (double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
        {
            Set (x1, y1, z1, x2, y2, z2, x3, y3, z3);
        }

        // ------------------------------------------------------------------------

        DoubleMatrix (DoubleVector r, DoubleVector u, DoubleVector f)
        {
            rVec.Set (r);
            uVec.Set (u);
            fVec.Set (f);
        }

        // ------------------------------------------------------------------------

        DoubleMatrix (DoubleMatrix m)
        {
            rVec.Set (m.rVec);
            uVec.Set (m.uVec);
            fVec.Set (m.fVec);
        }

        // ------------------------------------------------------------------------

        DoubleMatrix (double sinp, double cosp, double sinb, double cosb, double sinh, double cosh)
        {
            Set (sinp, cosp, sinb, cosb, sinh, cosh);
        }

        // ------------------------------------------------------------------------

        DoubleMatrix (AngleVector a)
        {
        }

        // ------------------------------------------------------------------------

        #endregion

        // ------------------------------------------------------------------------

        #region initializers

        void Clear ()
        {
            rVec.Set (1.0, 0.0, 0.0);
            uVec.Set (0.0, 1.0, 0.0);
            fVec.Set (0.0, 0.0, 1.0);
        }

        // ------------------------------------------------------------------------

        void Set (double x1, double y1, double z1, double x2, double y2, double z2, double x3, double y3, double z3)
        {
            rVec.Set (x1, y1, z1);
            uVec.Set (x2, y2, z2);
            fVec.Set (x3, y3, z3);
        }

        // ------------------------------------------------------------------------

        DoubleMatrix Set (double sinp, double cosp, double sinb, double cosb, double sinh, double cosh)
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

        DoubleMatrix Mul (DoubleMatrix other) 
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

        double Det () 
        {
            return rVec.v.x * (fVec.v.y * uVec.v.z - uVec.v.y * fVec.v.z) +
		           uVec.v.x * (rVec.v.y * fVec.v.z - fVec.v.y * rVec.v.z) +
		           fVec.v.x * (uVec.v.y * rVec.v.z - rVec.v.y * uVec.v.z);
        }

        // ------------------------------------------------------------------------

        DoubleMatrix Inverse () 
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

        DoubleMatrix Adjoint () 
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

        DoubleMatrix Transpose (DoubleMatrix dest, DoubleMatrix src)
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

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        #endregion

    }
}
