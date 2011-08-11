using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class UVL
    {
        public short u, v;
        public ushort l;

        //------------------------------------------------------------------------------

        public void Read (BinaryReader fp)
        {
            u = fp.ReadInt16 ();
            v = fp.ReadInt16 ();
            l = fp.ReadUInt16 ();
        }

        //------------------------------------------------------------------------------

        public void Write (BinaryWriter fp)
        {
            fp.Write (u);
            fp.Write (v);
            fp.Write (l);
        }

        //------------------------------------------------------------------------------

        public void Clear ()
        {
            u = v = 0;
            l = 0;
        }

        //------------------------------------------------------------------------------

        public UVL (short u = 0, short v = 0, ushort l = 0)
        {
            this.u = u;
            this.v = v;
            this.l = l;
        }

        //------------------------------------------------------------------------------

    }
}