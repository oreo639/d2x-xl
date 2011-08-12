using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public struct tDoubleVector 
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

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------

    public class DoubleVector : IComparable<DoubleVector>
    {
        public tDoubleVector v;

        #region Initializers
        public DoubleVector ()
        {
            v.x = 0;
            v.y = 0;
            v.z = 0;
        }

        public DoubleVector (double x, double y, double z)
        {
            v.x = x;
            v.y = y;
            v.z = z;
        }

        public DoubleVector (DoubleVector other)
        {
            v.x = other.v.x;
            v.y = other.v.y;
            v.z = other.v.z;
        }

        public DoubleVector (tDoubleVector other)
        {
            v.x = other.x;
            v.y = other.y;
            v.z = other.z;
        }

        public DoubleVector (FixVector other)
        {
            v.x = FixConverter.X2D (other.v.x);
            v.y = FixConverter.X2D (other.v.y);
            v.z = FixConverter.X2D (other.v.z);
        }

        public void Set (double x, double y, double z)
        {
            v.x = x;
            v.y = y;
            v.z = z;
        }

        public void Set (DoubleVector other)
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
        public static bool operator == (DoubleVector v1, DoubleVector v2) 
        {
	        return (v1.v.x == v2.v.x) && (v1.v.y == v2.v.y) && (v1.v.z == v2.v.z);
        }

        public static bool operator != (DoubleVector v1, DoubleVector v2)
        {
            return (v1.v.x != v2.v.x) || (v1.v.y != v2.v.y) || (v1.v.z != v2.v.z);
        }

        public static DoubleVector operator + (DoubleVector v1, DoubleVector v2) 
        {
	        return new DoubleVector (v1.v.x + v2.v.x, v1.v.y + v2.v.y, v1.v.z + v2.v.z);
	    }

        public static DoubleVector operator- (DoubleVector v1, DoubleVector v2) 
        {
	        return new DoubleVector (v1.v.x - v2.v.x, v1.v.y - v2.v.y, v1.v.z - v2.v.z);
	    }

        public static DoubleVector operator- (DoubleVector v) 
        {
	        return new DoubleVector (-v.v.x, -v.v.y, -v.v.z);
	    }

        public static DoubleVector operator* (DoubleVector v, double n) 
        {
            return new DoubleVector (v.v.x * n, v.v.y * n, v.v.z * n);
        }

        public static DoubleVector operator/ (DoubleVector v, double n) 
        {
            return new DoubleVector (v.v.x / n, v.v.y / n, v.v.z / n);
        }

        public static DoubleVector operator* (DoubleVector v1, DoubleVector v2) 
        {
	        return new DoubleVector (v1.v.x * v2.v.x, v1.v.y * v2.v.y, v1.v.z * v2.v.z);
	    }

        public static DoubleVector operator/ (DoubleVector v1, DoubleVector v2) 
        {
	        return new DoubleVector (v1.v.x / v2.v.x, v1.v.y / v2.v.y, v1.v.z / v2.v.z);
	    }

        // dot product
        public static double operator^ (DoubleVector v1, DoubleVector v2) 
        {
	        return v1.v.x * v2.v.x + v1.v.y * v2.v.y + v1.v.z * v2.v.z;
	    }

        #endregion

        #region interface functions

        public int CompareTo (DoubleVector other)
        {
        if (v.x < other.v.x)
            return -1;
        if (v.x > other.v.x)
            return 1;
        if (v.y < other.v.y)
            return -1;
        if (v.y > other.v.y)
            return 1;
        if (v.z < other.v.z)
            return -1;
        if (v.z > other.v.z)
            return 1;
        return 0;
        }

        public DoubleVector Clone ()
        {
            return new DoubleVector (this);
        }

        public DoubleVector Copy (DoubleVector dest)
        {
            dest.v = this.v;
            return dest;
        }

        #endregion
        // ------------------------------------------------------------------------
        #region Unary operations
        public DoubleVector Neg (DoubleVector other)
        {
            v.x = -v.x;
            v.y = -v.z;
            v.z = -v.z;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public DoubleVector Add (DoubleVector other)
        {
            v.x += other.v.x;
            v.y += other.v.z;
            v.z += other.v.z;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public DoubleVector Sub (DoubleVector other)
        {
            v.x -= other.v.x;
            v.y -= other.v.z;
            v.z -= other.v.z;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public DoubleVector Mul (DoubleVector other)
        {
            v.x *= other.v.x;
            v.y *= other.v.y;
            v.z *= other.v.z;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public DoubleVector Div (DoubleVector other)
        {
            v.x /= other.v.x;
            v.y /= other.v.y;
            v.z /= other.v.z;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public DoubleVector Add (double value)
        {
            v.x += value;
            v.y += value;
            v.z += value;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public DoubleVector Sub (double value)
        {
            v.x -= value;
            v.y -= value;
            v.z -= value;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public DoubleVector Mul (double value)
        {
            v.x *= value;
            v.y *= value;
            v.z *= value;
            return this;
        }
        
        // ------------------------------------------------------------------------

        public DoubleVector Div (double value)
        {
            v.x /= value;
            v.y /= value;
            v.z /= value;
            return this;
        }
        #endregion
        // ------------------------------------------------------------------------
        #region Math
        public double Mag () 
        { 
            return Math.Sqrt (v.x * v.x + v.y * v.y + v.z * v.z); 
        }

        // ------------------------------------------------------------------------

        void Rotate (DoubleVector origin, DoubleVector normal, double angle) 
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
            DoubleVector other = obj as DoubleVector;
            if ((System.Object) other == null)
            {
                return false;
            }

            // Return true if the fields match:
            return (v.x == other.v.x) && (v.y == other.v.y) && (v.z == other.v.z);
        }

        // ------------------------------------------------------------------------

        public bool Equals (DoubleVector other)
        {
            // If parameter is null return false:
            if ((object)other == null)
            {
                return false;
            }

            // Return true if the fields match:
            return (v.x == other.v.x) && (v.y == other.v.y) && (v.z == other.v.z);
        }

        // ------------------------------------------------------------------------

        public override int GetHashCode ()
        {
            return FixConverter.D2X (v.x) ^ FixConverter.D2X (v.y) ^ FixConverter.D2X (v.z);
        }
        #endregion

        // ------------------------------------------------------------------------

        public static DoubleVector CrossProduct (DoubleVector v0, DoubleVector v1) 
        {
    	    return new DoubleVector (v0.v.y * v1.v.z - v0.v.z * v1.v.y, v0.v.z * v1.v.x - v0.v.x * v1.v.z, v0.v.x * v1.v.y - v0.v.y * v1.v.x);
	    }

        // ------------------------------------------------------------------------

        public static DoubleVector Perpendicular (DoubleVector p0, DoubleVector p1, DoubleVector p2) 
        {
	        return CrossProduct (p1 - p0, p2 - p1);
	    }

        // ------------------------------------------------------------------------

        public static DoubleVector Normalize (DoubleVector v) 
        { 
	        double m = v.Mag ();
            if (m == 0.0)
                v.Set (0.0, 0.0, 0.0);
            else
                v /= m;
            return v;
	    }

        // ------------------------------------------------------------------------

        public DoubleVector Normalize ()
        {
            double m = Mag ();
            if (m == 0.0)
                Set (0.0, 0.0, 0.0);
            else
            {
                DoubleVector v = this as DoubleVector;
                v /= m;
            }
            return this;
        }

        // ------------------------------------------------------------------------

        public static DoubleVector Normal (DoubleVector p0, DoubleVector p1, DoubleVector p2) 
        {
	        return Normalize (CrossProduct (p1 - p0, p2 - p0));
	    }

        // ------------------------------------------------------------------------

        public static double Normal (DoubleVector normal, DoubleVector p0, DoubleVector p1, DoubleVector p2) 
        {
	        normal = CrossProduct (p1 - p0, p2 - p0);
	        double m = normal.Mag ();
	        if (m > 0.0)
		        normal *= m;
	        return m;
	    }

        // ------------------------------------------------------------------------

        public static double Dot (DoubleVector v0, DoubleVector v1) 
        {
    	    return (double) v0.v.x * (double) v1.v.x + (double) v0.v.y * (double) v1.v.y + (double) v0.v.z * (double) v1.v.z;
	    }

        // ------------------------------------------------------------------------

        public static DoubleVector Min (DoubleVector v0, DoubleVector v1) 
        {
	        return new DoubleVector (Math.Min (v0.v.x, v1.v.x), Math.Min (v0.v.y, v1.v.y), Math.Min (v0.v.z, v1.v.z));
	    }

        // ------------------------------------------------------------------------

        public static DoubleVector Max (DoubleVector v0, DoubleVector v1) 
        {
	        return new DoubleVector (Math.Max (v0.v.x, v1.v.x), Math.Max (v0.v.y, v1.v.y), Math.Max (v0.v.z, v1.v.z));
	    }

        // ------------------------------------------------------------------------

        public static double Distance (DoubleVector p0, DoubleVector p1) 
        {
	        DoubleVector v = p0 - p1;
	        return v.Mag ();
	    }

        // ------------------------------------------------------------------------

        public static DoubleVector Average (DoubleVector p0, DoubleVector p1) 
        {
    	    DoubleVector v = p0 + p1;
	        v /= 2.0;
	        return v;
	    }


        // ------------------------------------------------------------------------

    }
}
