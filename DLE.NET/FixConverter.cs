﻿using System;
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

        public static int I2X (int n)
        {
            return (n * 65536);
        }

        public static int X2I (int n)
        {
            return (n / 65536);
        }

        public static int D2X (double n)
        {
            return ((int)Round (n * 65536.0));
        }

        public static double X2D (int n)
        {
            return ((double)n / 65536.0);
        }

        public static int Mul (int n, int m)
        {
            return (int) ((double) n * (double) m / 65536.0);
        }

        public static int Div (int n, int m)
        {
            return (int) ((double) n / (double) m * 65536.0);
        }

        public static int Sqr (int n)
        {
            return Mul (n, n);
        }

        public static int Sqrt (int n)
        {
            return D2X (Math.Sqrt (X2D (n)));
        }
    }
}
