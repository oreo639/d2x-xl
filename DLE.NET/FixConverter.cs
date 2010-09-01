using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public static class FixConverter
    {
        public static double Round (double value, double round = 1.0)
        {
            return (value >= 0) ? value + round / 2.0 : value - round / 2.0;
        }

        public static double X2D (int f)
        {
            return ((double)f / 65536.0);
        }

        public static int D2X (double f)
        {
            return ((int)Round (f * 65536.0));
        }

        public static int Mul (int n, int m)
        {
            return (int) ((double) n * (double) m / 65536.0);
        }

        public static int Div (int n, int m)
        {
            return (int) ((double) n / (double) m * 65536.0);
        }
    }
}
