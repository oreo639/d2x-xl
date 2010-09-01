using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public interface IGameItem
    {
        int Key { get; set; }

        void Read (BinaryReader fp, int version = 0, bool bFlag = false);
        void Write (BinaryWriter fp, int version = 0, bool bFlag = false);
        void Clear ();

    }
}
