using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class RGBColor 
    {
        protected double r = 0.0, g = 0.0, b = 0.0;

        public RGBColor (double red = 0, double green = 0, double blue = 0)
        {
            r = red;
            g = green;
            b = blue;
        }

        public void Set (double red, double green, double blue)
        {
            r = red;
            g = green;
            b = blue;
        }

        public void Clear ()
        {
            r = g = b = 0.0;
        }

    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    public class ColorRef : RGBColor
    {
        protected byte index = 0;

        public ColorRef (double red = 0, double green = 0, double blue = 0, byte index = 0)
            : base (red, green, blue)
        {
            this.index = index;
        }

        new public void Clear ()
        {
            base.Clear ();
            index = 0;
        }

    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    public class GameColor : ColorRef, IGameItem
    {
        public int Key { get; set; }

        //-------------------------------------------------------------------------

        public void Read (BinaryReader fp, int nVersion, bool bD2X = false)
        {
            index = fp.ReadByte ();
            Set ((double)fp.ReadInt32 () / (double)0x7fffffff, (double)fp.ReadInt32 () / (double)0x7fffffff, (double)fp.ReadInt32 () / (double)0x7fffffff);
        }

        //-------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int nVersion, bool bD2X = false)
        {
            fp.Write (index);
            fp.Write ((int) (r * (double) 0x7fffffff));
            fp.Write ((int) (g * (double) 0x7fffffff));
            fp.Write ((int) (b * (double) 0x7fffffff));
        }

        //-------------------------------------------------------------------------

        public byte Red { get { return (byte)(r * 255); } }
        public byte Green { get { return (byte)(g * 255); } }
        public byte Blue { get { return (byte)(b * 255); } }

        public byte Index
        {
            get { return index; }
            set { index = value; }
        }

        //-------------------------------------------------------------------------

    }
}
