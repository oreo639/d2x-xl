using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public static class MathExtensions
    {
        public static double Radians (double a)
        {
            return a * Math.PI / 180.0;
        }

        public static double Degrees (double a)
        {
            return a * 180.0 / Math.PI;
        }
    }
}
