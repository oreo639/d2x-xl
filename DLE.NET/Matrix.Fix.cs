using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public class FixMatrix
    {
        public FixVector rVec = new FixVector ();
        public FixVector uVec = new FixVector ();
        public FixVector fVec = new FixVector ();

        // ------------------------------------------------------------------------

        #region c'tors

        FixMatrix ()
        {
            Clear ();
        }

        // ------------------------------------------------------------------------

        FixMatrix (int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3)
        {
            Set (x1, y1, z1, x2, y2, z2, x3, y3, z3);
        }

        // ------------------------------------------------------------------------

        FixMatrix (FixVector r, FixVector u, FixVector f)
        {
            rVec.Set (r);
            uVec.Set (u);
            fVec.Set (f);
        }

        // ------------------------------------------------------------------------

        FixMatrix (FixMatrix m)
        {
            rVec.Set (m.rVec);
            uVec.Set (m.uVec);
            fVec.Set (m.fVec);
        }

        // ------------------------------------------------------------------------

        FixMatrix (int sinp, int cosp, int sinb, int cosb, int sinh, int cosh)
        {
            Set (sinp, cosp, sinb, cosb, sinh, cosh);
        }

        // ------------------------------------------------------------------------

        FixMatrix (AngleVector a)
        {
        }

        // ------------------------------------------------------------------------

        #endregion

        // ------------------------------------------------------------------------

        #region initializers

        void Clear ()
        {
            rVec.Set (FixConverter.I2X (1), 0, 0);
            uVec.Set (0, FixConverter.I2X (1), 0);
            fVec.Set (0, 0, FixConverter.I2X (1));
        }

        // ------------------------------------------------------------------------

        void Set (int x1, int y1, int z1, int x2, int y2, int z2, int x3, int y3, int z3)
        {
            rVec.Set (x1, y1, z1);
            uVec.Set (x2, y2, z2);
            fVec.Set (x3, y3, z3);
        }

        // ------------------------------------------------------------------------

        FixMatrix Set (int sinp, int cosp, int sinb, int cosb, int sinh, int cosh)
        {
            double sbsh = sinb * sinh;
            double cbch = cosb * cosh;
            double cbsh = cosb * sinh;
            double sbch = sinb * cosh;
            rVec.v.x = (int)(cbch + sinp * sbsh);
            uVec.v.z = (int)(sbsh + sinp * cbch);
            uVec.v.x = (int)(sinp * cbsh - sbch);
            rVec.v.z = (int)(sinp * sbch - cbsh);
            fVec.v.x = (int)(sinh * cosp);
            rVec.v.y = (int)(sinb * cosp);
            uVec.v.y = (int)(cosb * cosp);
            fVec.v.z = (int)(cosh * cosp);
            fVec.v.y = (int)-sinp;
            return this;
        }

        #endregion

        // ------------------------------------------------------------------------

        #region operations

        FixMatrix Mul (FixMatrix other) 
        {
	        FixVector v = new FixVector ();
	        FixMatrix m = new FixMatrix ();

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

        int Det () 
        {
            int det = FixConverter.Mul (rVec.v.x, FixConverter.Mul (uVec.v.y, fVec.v.z) - FixConverter.Mul (uVec.v.z, fVec.v.y));
            det += FixConverter.Mul (rVec.v.y, FixConverter.Mul (uVec.v.z, fVec.v.x) - FixConverter.Mul (uVec.v.x, fVec.v.z));
            det += FixConverter.Mul (rVec.v.z, FixConverter.Mul (uVec.v.x, fVec.v.y) - FixConverter.Mul (uVec.v.y, fVec.v.x));
            return det;
        }

        // ------------------------------------------------------------------------

        FixMatrix Inverse () 
        {
	        FixMatrix m = new FixMatrix ();

            int det = Det ();
            if (det != 0)
            {
                m.rVec.v.x = FixConverter.Div (FixConverter.Mul (uVec.v.y, fVec.v.z) - FixConverter.Mul (uVec.v.z, fVec.v.y), det);
                m.rVec.v.y = FixConverter.Div (FixConverter.Mul (rVec.v.z, fVec.v.y) - FixConverter.Mul (rVec.v.y, fVec.v.z), det);
                m.rVec.v.z = FixConverter.Div (FixConverter.Mul (rVec.v.y, uVec.v.z) - FixConverter.Mul (rVec.v.z, uVec.v.y), det);
                m.uVec.v.x = FixConverter.Div (FixConverter.Mul (uVec.v.z, fVec.v.x) - FixConverter.Mul (uVec.v.x, fVec.v.z), det);
                m.uVec.v.y = FixConverter.Div (FixConverter.Mul (rVec.v.x, fVec.v.z) - FixConverter.Mul (rVec.v.z, fVec.v.x), det);
                m.uVec.v.z = FixConverter.Div (FixConverter.Mul (rVec.v.z, uVec.v.x) - FixConverter.Mul (rVec.v.x, uVec.v.z), det);
                m.fVec.v.x = FixConverter.Div (FixConverter.Mul (uVec.v.x, fVec.v.y) - FixConverter.Mul (uVec.v.y, fVec.v.x), det);
                m.fVec.v.y = FixConverter.Div (FixConverter.Mul (rVec.v.y, fVec.v.x) - FixConverter.Mul (rVec.v.x, fVec.v.y), det);
                m.fVec.v.z = FixConverter.Div (FixConverter.Mul (rVec.v.x, uVec.v.y) - FixConverter.Mul (rVec.v.y, uVec.v.x), det);
            }
            return m;
        }

        // ------------------------------------------------------------------------

        FixMatrix Adjoint () 
        {
	        FixMatrix m = new FixMatrix ();

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

        FixMatrix Transpose (FixMatrix dest, FixMatrix src)
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

        void Rotate (int angle, char axis) 
        {
        double cosX = Math.Cos (angle);
        double sinX = Math.Sin (angle);

        FixMatrix mRot = new FixMatrix ();

        switch (axis) {
	        case 'x':
		        // spin x
		        //	1	0	0
		        //	0	cos	sin
		        //	0	-sin	cos
		        //
		        mRot.uVec.Set ((int) (uVec.v.x * cosX + fVec.v.x * sinX), 
					           (int) (uVec.v.y * cosX + fVec.v.y * sinX),
						       (int) (uVec.v.z * cosX + fVec.v.z * sinX));
		        mRot.fVec.Set ((int) (fVec.v.x * cosX - uVec.v.x * sinX),
							   (int) (fVec.v.y * cosX - uVec.v.y * sinX),
							   (int) (fVec.v.z * cosX - uVec.v.z * sinX));
		        uVec = mRot.uVec;
		        fVec = mRot.fVec;
		        break;

	        case 'y':
		        // spin y
		        //	cos	0	-sin
		        //	0	1	0
		        //	sin	0	cos
		        //
		        mRot.rVec.Set ((int) (rVec.v.x * cosX - fVec.v.x * sinX), 
							   (int) (rVec.v.y * cosX - fVec.v.y * sinX), 
							   (int) (rVec.v.z * cosX - fVec.v.z * sinX));
		        mRot.fVec.Set ((int) (rVec.v.x * sinX + fVec.v.x * cosX), 
							   (int) (rVec.v.y * sinX + fVec.v.y * cosX),
							   (int) (rVec.v.z * sinX + fVec.v.z * cosX));
		        rVec = mRot.rVec;
		        fVec = mRot.fVec;
		        break;

	        case 'z':
		        // spin z
		        //	cos	sin	0
		        //	-sin	cos	0
		        //	0	0	1
		        mRot.rVec.Set ((int) (rVec.v.x * cosX + uVec.v.x * sinX),
							   (int) (rVec.v.y * cosX + uVec.v.y * sinX),
							   (int) (rVec.v.z * cosX + uVec.v.z * sinX));
		        mRot.uVec.Set ((int) (uVec.v.x * cosX - rVec.v.x * sinX),
							   (int) (uVec.v.y * cosX - rVec.v.y * sinX),
							   (int) (uVec.v.z * cosX - rVec.v.z * sinX));
		        rVec = mRot.rVec;
		        uVec = mRot.uVec;
		        break;
	        }
        }

        // ------------------------------------------------------------------------

        #endregion

    }
}
