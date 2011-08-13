using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public class SecretExit
    {

		public int nSegment = 0;
		public DoubleMatrix orient = new DoubleMatrix ();

        // ------------------------------------------------------------------------

        public SecretExit Set (SecretExit other) 
        { 
		    nSegment = other.nSegment;
		    orient.Set (other.orient);
		    return this;
		}

        // ------------------------------------------------------------------------

        public static bool operator == (SecretExit a, SecretExit b) 
        { 
            return (a.nSegment == b.nSegment) && (a.orient == b.orient); 
        }

    	public static bool operator!= (SecretExit a, SecretExit b) 
        { 
            return (a.nSegment != b.nSegment) || (a.orient != b.orient); 
        }

        // ------------------------------------------------------------------------

        public bool Equals (SecretExit other)
        {
            // If parameter is null return false:
            if ((object)other == null)
            {
                return false;
            }

            // Return true if the fields match:
            return (nSegment == other.nSegment) && (orient == other.orient); 
        }

        // ------------------------------------------------------------------------

        public override int GetHashCode ()
        {
            return nSegment ^ orient.GetHashCode ();
        }

        // ------------------------------------------------------------------------

    }
}
