using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLEdotNET
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
        }

        //-------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int nVersion, bool bD2X = false)
        {
        }

        //-------------------------------------------------------------------------

        byte Red { get { return (byte) (r * 255); } }
        byte Green { get { return (byte) (g * 255); } }
        byte Blue { get { return (byte)(b * 255); } }

        //-------------------------------------------------------------------------

    }
}
