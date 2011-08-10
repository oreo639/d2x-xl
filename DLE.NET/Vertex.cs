using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class Vertex : DoubleVector, IGameItem, IComparable<Vertex>
    {
        public byte Status { get; set; }

        #region constructors

        public Vertex ()
            : base ()
        {
            Status = 0;
        }

        public Vertex (int key)
            : base ()
        {
            Status = 0;
            Key = key;
        }

        public Vertex (double x, double y, double z)
            : base (x, y, z)
        {
            Status = 0;
        }

        public Vertex (Vertex other)
            : base (other)
        {
            Status = other.Status;
        }

        public Vertex (DoubleVector other)
            : base (other)
        {
            Status = 0;
        }

        #endregion

        #region IGameItem Members

        // ------------------------------------------------------------------------

        public int Key { get; set; }
        public uint ItemType { get; set; }
        public uint EditType { get; set; }
        public IGameItem Parent { get; set; }
        public IGameItem Link { get; set; }

        // ------------------------------------------------------------------------

        public void Read (BinaryReader fp, int version = 0, bool bFlag = false)
        {
            Status = 0;
            base.Read (fp);
        }

        // ------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int version = 0, bool bFlag = false)
        {
            base.Write (fp);
        }

        // ------------------------------------------------------------------------

        new public void Clear ()
        {
            base.Clear ();
            Status = 0;
        }

        // ------------------------------------------------------------------------

        public IGameItem Clone ()
        {
            return new Vertex (this);
        }

        // ------------------------------------------------------------------------

        public IGameItem Copy (IGameItem dest)
        {
            Vertex v = dest as Vertex;
            this.v = v.v;
            this.Status = v.Status;
            return this;
        }

        // ------------------------------------------------------------------------

        public IGameItem CopyTo (IGameItem dest)
        {
            Vertex v = dest as Vertex;
            v.v = this.v;
            v.Status = this.Status;
            return dest;
        }

        // ------------------------------------------------------------------------

        public int CompareTo (Vertex other)
        {
            int i = base.CompareTo (other);
            if (i != 0)
                return i;
            return (Status < other.Status) ? -1 : (Status > other.Status) ? 1 : 0;
        }

        // ------------------------------------------------------------------------

        #endregion

        #region members

        public void Mark (byte mask = GameMine.MARKED_MASK) { Status |= mask; }
        public void Unark (byte mask = GameMine.MARKED_MASK) { Status &= (byte) ~ (int) mask; }
        public bool IsMarked (byte mask = GameMine.MARKED_MASK) { return (Status & mask) != 0; }

        #endregion
    }
}
