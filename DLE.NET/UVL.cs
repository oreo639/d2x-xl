﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class UVL
    {
        public ushort u, v, l;

        //------------------------------------------------------------------------------

        public void Read (BinaryReader fp)
        {
            u = fp.ReadUInt16 ();
            v = fp.ReadUInt16 ();
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
            u = v = l = 0;
        }

        //------------------------------------------------------------------------------

    }
}