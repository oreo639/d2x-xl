using System.IO;
using System;

namespace DLE.NET
{
    public struct tFixVector
    {
        public int x, y, z;

        public void Read (BinaryReader fp)
        {
            x = fp.ReadInt32 ();
            y = fp.ReadInt32 ();
            z = fp.ReadInt32 ();
        }

        public void Write (BinaryWriter fp)
        {
            fp.Write (x);
            fp.Write (y);
            fp.Write (z);
        }
    }

    public class FixVector
    {
        public tFixVector v;

        #region Initializers
        public FixVector ()
        {
            v.x = 0;
            v.y = 0;
            v.z = 0;
        }

        public FixVector (int x, int y, int z)
        {
            v.x = x;
            v.y = y;
            v.z = z;
        }

        public FixVector (tFixVector other)
        {
            v.x = other.x;
            v.y = other.y;
            v.z = other.z;
        }

        public FixVector (DoubleVector other)
        {
            v.x = FixConverter.D2X (other.v.x);
            v.y = FixConverter.D2X (other.v.y);
            v.z = FixConverter.D2X (other.v.z);
        }

        public void Set (int x, int y, int z)
        {
            v.x = x;
            v.y = y;
            v.z = z;
        }

        public void Set (FixVector other)
        {
            v.x = other.v.x;
            v.y = other.v.y;
            v.z = other.v.z;
        }

        public void Clear ()
        {
            Set (0, 0, 0);
        }
        #endregion

        #region File i/o
        public void Read (BinaryReader fp)
        {
            v.Read (fp);
        }

        public void Write (BinaryWriter fp)
        {
            v.Write (fp);
        }
        #endregion

        #region Operators
        public static bool operator == (FixVector v1, FixVector v2) 
        {
	        return (v1.v.x == v2.v.x) && (v1.v.y == v2.v.y) && (v1.v.z == v2.v.z);
        }

        public static bool operator != (FixVector v1, FixVector v2)
        {
            return (v1.v.x != v2.v.x) || (v1.v.y != v2.v.y) || (v1.v.z != v2.v.z);
        }

        public static FixVector operator + (FixVector v1, FixVector v2) 
        {
	        return new FixVector (v1.v.x + v2.v.x, v1.v.y + v2.v.y, v1.v.z + v2.v.z);
	    }

        public static FixVector operator- (FixVector v1, FixVector v2) 
        {
	        return new FixVector (v1.v.x - v2.v.x, v1.v.y - v2.v.y, v1.v.z - v2.v.z);
	    }

        public static FixVector operator- (FixVector v) 
        {
	        return new FixVector (-v.v.x, -v.v.y, -v.v.z);
	    }

        public static FixVector operator* (FixVector v, int n) 
        {
            return new FixVector (FixConverter.Mul (v.v.x, n), FixConverter.Mul (v.v.y, n), FixConverter.Mul (v.v.z, n));
        }

        public static FixVector operator/ (FixVector v, int n) 
        {
            return new FixVector (FixConverter.Div (v.v.x, n), FixConverter.Div (v.v.y, n), FixConverter.Div (v.v.z, n));
        }

        public static FixVector operator* (FixVector v1, FixVector v2) 
        {
	        return new FixVector (FixConverter.Mul (v1.v.x, v2.v.x), FixConverter.Mul (v1.v.y, v2.v.y), FixConverter.Mul (v1.v.z, v2.v.z));
	    }

        public static FixVector operator/ (FixVector v1, FixVector v2) 
        {
            return new FixVector (FixConverter.Div (v1.v.x, v2.v.x), FixConverter.Div (v1.v.y, v2.v.y), FixConverter.Div (v1.v.z, v2.v.z));
        }

        // dot product
        public static int operator^ (FixVector v1, FixVector v2) 
        {
	        return v1.v.x * v2.v.x + v1.v.y * v2.v.y + v1.v.z * v2.v.z;
	    }
        #endregion
        // ------------------------------------------------------------------------
        #region Unary operations
        public FixVector Neg (FixVector other)
        {
            v.x = -v.x;
            v.y = -v.z;
            v.z = -v.z;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public FixVector Add (FixVector other)
        {
            v.x += other.v.x;
            v.y += other.v.z;
            v.z += other.v.z;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public FixVector Sub (FixVector other)
        {
            v.x -= other.v.x;
            v.y -= other.v.z;
            v.z -= other.v.z;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public FixVector Mul (FixVector other)
        {
            v.x = FixConverter.Mul (v.x, other.v.x);
            v.y = FixConverter.Mul (v.y, other.v.y);
            v.z = FixConverter.Mul (v.z, other.v.z);
            return this;
        }
        
        // ------------------------------------------------------------------------

        public FixVector Div (FixVector other)
        {
            v.x = FixConverter.Div (v.x, other.v.x);
            v.y = FixConverter.Div (v.y, other.v.y);
            v.z = FixConverter.Div (v.z, other.v.z);
            return this;
        }
        
        // ------------------------------------------------------------------------

        public FixVector Add (int value)
        {
            v.x += value;
            v.y += value;
            v.z += value;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public FixVector Sub (int value)
        {
            v.x -= value;
            v.y -= value;
            v.z -= value;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public FixVector Mul (int value)
        {
            v.x = FixConverter.Mul (v.x, value);
            v.y = FixConverter.Mul (v.y, value);
            v.z = FixConverter.Mul (v.z, value);
            return this;
        }
        
        // ------------------------------------------------------------------------

        public FixVector Div (int value)
        {
            v.x = FixConverter.Div (v.x, value);
            v.y = FixConverter.Div (v.y, value);
            v.z = FixConverter.Div (v.z, value);
            return this;
        }
        #endregion
        // ------------------------------------------------------------------------
        #region Math
        public int Mag () 
        {
            return FixConverter.Sqrt (FixConverter.Sqr (v.x) + FixConverter.Sqr (v.y) + FixConverter.Sqr (v.z));
        }

        // ------------------------------------------------------------------------

        public FixVector Normalize () 
        { 
            return Div (Mag ());
        }

        // ------------------------------------------------------------------------

        void Rotate (FixVector origin, FixVector normal, int angle) 
        {
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
            FixVector p = obj as FixVector;
            if ((System.Object) p == null)
            {
                return false;
            }

            // Return true if the fields match:
            return (v.x == p.v.x) && (v.y == p.v.y) && (v.z == p.v.z);
        }

        // ------------------------------------------------------------------------

        public bool Equals (FixVector p)
        {
            // If parameter is null return false:
            if ((object)p == null)
            {
                return false;
            }

            // Return true if the fields match:
            return (v.x == p.v.x) && (v.y == p.v.y) && (v.z == p.v.z);
        }

        // ------------------------------------------------------------------------

        public override int GetHashCode ()
        {
            return FixConverter.D2X (v.x) ^ FixConverter.D2X (v.y) ^ FixConverter.D2X (v.z);
        }
        #endregion
        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        public static int Dot (FixVector v0, FixVector v1)
        {
            return (int)((((double)v0.v.x * (double)v1.v.x + (double)v0.v.y * (double)v1.v.y + (double)v0.v.z * (double)v1.v.z)) / 65536.0);
        }

        // ------------------------------------------------------------------------

        public static FixVector Min (FixVector v0, FixVector v1)
        {
            return new FixVector (Math.Min (v0.v.x, v1.v.x), Math.Min (v0.v.y, v1.v.y), Math.Min (v0.v.z, v1.v.z));
        }

        // ------------------------------------------------------------------------

        public static FixVector Max (FixVector v0, FixVector v1)
        {
            return new FixVector (Math.Max (v0.v.x, v1.v.x), Math.Max (v0.v.y, v1.v.y), Math.Max (v0.v.z, v1.v.z));
        }

        // ------------------------------------------------------------------------

        public static double Distance (FixVector p0, FixVector p1)
        {
            FixVector v = p0 - p1;
            return FixConverter.D2X (new DoubleVector (v).Mag ());
        }

        // ------------------------------------------------------------------------

        public static FixVector Average (FixVector p0, FixVector p1)
        {
            FixVector v = p0 + p1;
            v /= FixConverter.I2X (2);
            return v;
        }

        // ------------------------------------------------------------------------

    }
}
