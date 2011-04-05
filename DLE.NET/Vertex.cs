﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using DLE.NET.GameMine;

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

        public int Key { get; set; }
        public uint Id  { get; set; }
        public int Index { get; set; }
        public IGameItem.ItemType ItemType { get; set; }
        public IGameItem.EditType EditType { get; set; }
        public IGameItem Parent { get; set; }
        public IGameItem Link { get; set; }
        public bool Used { get { return Index >= 0; } }

        public void Read (System.IO.BinaryReader fp, int version = 0, bool bFlag = false)
        {
            throw new NotImplementedException ();
        }

        public void Write (System.IO.BinaryWriter fp, int version = 0, bool bFlag = false)
        {
            throw new NotImplementedException ();
        }

        public void Clear ()
        {
            base.Clear ();
            Status = 0;
        }

        public IGameItem Clone ()
        {
            return new Vertex (this);
        }

        public IGameItem Copy (IGameItem dest)
        {
            Vertex v = dest as Vertex;
            v = this;
            return dest;
        }

        public int CompareTo (Vertex other)
        {
            int i = base.CompareTo (other);
            if (i != 0)
                return i;
            return (Status < other.Status) ? -1 : (Status > other.Status) ? 1 : 0;
        }

        #endregion

        #region members

        public void Mark (byte mask = GameMine.MARKED_MASK) { Status |= mask; }
        public void Unark (byte mask = GameMine.MARKED_MASK) { Status &= (byte) ~ (int) mask; }
        public bool IsMarked (byte mask = GameMine.MARKED_MASK) { return (Status & mask) != 0; }

        #endregion
    }
}
