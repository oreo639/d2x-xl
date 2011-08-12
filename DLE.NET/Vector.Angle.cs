using System.IO;

namespace DLE.NET
{
    struct tAngleVector
    {
        public short p, b, h;

        public void Read (BinaryReader fp)
        {
            p = fp.ReadInt16 ();
            b = fp.ReadInt16 ();
            h = fp.ReadInt16 ();
        }

        public void Write (BinaryWriter fp)
        {
            fp.Write (p);
            fp.Write (b);
            fp.Write (h);
        }
    }

    class AngleVector
    {
        public tAngleVector v;

        public AngleVector () { v.p = 0; v.b = 0; v.h = 0; }

        public AngleVector (short p, short b, short h)
        {
            v.p = p;
            v.b = b;
            v.h = h;
        }

        public AngleVector (tAngleVector a)
        {
            v.p = a.p;
            v.b = a.b;
            v.h = a.h;
        }

        public void Set (short p, short b, short h)
        {
            v.p = p;
            v.b = b;
            v.h = h;
        }

        public void Set (tAngleVector other)
        {
            v.p = other.p;
            v.b = other.b;
            v.h = other.h;
        }

        public void Set (AngleVector other)
        {
            v.p = other.v.p;
            v.b = other.v.b;
            v.h = other.v.h;
        }

        public void Clear ()
        {
            Set (0, 0, 0);
        }
    }
}
