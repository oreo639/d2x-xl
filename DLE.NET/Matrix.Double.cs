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

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

    }
}
