using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class UVL
    {
        short u, v, l;

        public void Read (BinaryReader fp)
        {
            u = fp.ReadInt16 ();
            v = fp.ReadInt16 ();
            l = fp.ReadInt16 ();
        }

        public void Write (BinaryWriter fp)
        {
            fp.Write (u);
            fp.Write (v);
            fp.Write (l);
        }

        public override void Clear ()
        {
            u = v = l = 0;
        }
    }
}