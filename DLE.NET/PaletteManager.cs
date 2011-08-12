using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.IO;
using System.Runtime.InteropServices;
using System.Drawing.Imaging;
using System.Resources;

namespace DLEdotNET
{
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    [StructLayout (LayoutKind.Sequential, Pack = 1)]
    public struct RGBA
    {
        public byte r, g, b, a;
    }

    //------------------------------------------------------------------------------

    [StructLayout (LayoutKind.Sequential, Pack = 1)]
    public struct BGR 
    {
	    public byte b, g, r;

	    public BGR (byte red = 0, byte green = 0, byte blue = 0)
		{
            r = red;
            g = green;
            b = blue;
        }

        public bool Same (BGR other)
        {
            return (r == other.r) && (g == other.g) && (b == other.b);
        }

        public bool Different (BGR other)
        {
            return (r != other.r) || (g != other.g) || (b != other.b);
        }

        int Sqr (int i)
        {
        return i * i;
        }

        public uint Delta (BGR other)
        {
            return (uint)(Sqr ((int)r - (int)other.r) + Sqr ((int)g - (int)other.g) + Sqr ((int)b - (int)other.b));
        }

        public uint ColorRef () { return (uint)r * 65536 + (uint)g * 256 + (uint)b; }
    }
    
    //------------------------------------------------------------------------------

    [StructLayout (LayoutKind.Sequential, Pack = 1)]
    public struct BGRA 
    {
	    public byte b, g, r, a;

	    public void Assign (RGBA other) 
        {
		    r = other.r;
		    g = other.g;
		    b = other.b;
		    a = other.a;
		}

        public void Assign (BGR other, byte alpha = 255) 
        {
		    r = other.r;
		    g = other.g;
		    b = other.b;
		    a = alpha;
		}

        public void Blend (BGRA other)
        {
            if (DLE.Palettes.SuperTransp (other))
                r =
                g =
                b =
                a = 0;
            else if (a > 0)
            {
                if (a == 255)
                    this = other;
                else
                {
                    int s = 255 - other.a;
                    r = (byte)(((int)r * s + (int)other.r * other.a) / 255);
                    g = (byte)(((int)g * s + (int)other.g * other.a) / 255);
                    b = (byte)(((int)b * s + (int)other.b * other.a) / 255);
                }
            }
        }

        public bool Same (BGRA other)
        {
            return (r == other.r) && (g == other.g) && (b == other.b) && (a == other.a);
        }

        public bool Same (BGR other)
        {
            return (r == other.r) && (g == other.g) && (b == other.b);
        }

        int Sqr (int i)
        {
            return i * i;
        }

        public uint Delta (BGR other)
        {
            return (uint)(Sqr ((int)r - (int)other.r) + Sqr ((int)g - (int)other.g) + Sqr ((int)b - (int)other.b));
        }

        public BGRA (byte red = 0, byte green = 0, byte blue = 0, byte alpha = 255)
		{
            r = red;
            g = green;
            b = blue;
            a = alpha;
        }
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class PaletteManager
    {

        //------------------------------------------------------------------------

        [StructLayout (LayoutKind.Sequential, Pack=1)]
        public struct RGBQUAD
        {
            public byte rgbBlue;
            public byte rgbGreen;
            public byte rgbRed;
            public byte rgbReserved;
        }        

        //------------------------------------------------------------------------

        public struct tBMIInfo 
        {
	        public BitmapInfoHeader header;
            [MarshalAs (UnmanagedType.ByValArray, SizeConst=256)]
	        public RGBQUAD[] colors;
        }

        //------------------------------------------------------------------------

        public String Name { get {return m_name; } }
        public BGR [] Custom { get { return m_bHaveCustom ? m_custom : null; } }
        public BGR [] Default { get { return m_bHaveDefault ? m_default : null; } }

        BGR m_superTransp;
        BGR[] m_custom = new BGR [256];
        BGR[] m_default = new BGR [256];
        byte[] m_rawData = new byte [37 * 256];
        //tBMIInfo m_bmi;
        //ColorPalette m_render;
        //LPLOGPALETTE m_dlcLog;
        bool m_bHaveDefault = false;
        bool m_bHaveCustom = false;
        String m_name = "groupa.pig";
        String [] m_palResNames = new String [6] { "groupa.256", "alien1.256", "alien2.256", "fire.256", "water.256", "ice.256" };

        const int paletteSize = 37 * 256;

   		public bool SuperTransp (BGRA color) { return color.Same (m_superTransp); }
                
        //------------------------------------------------------------------------

        int Luminance (int r, int g, int b)
        {
	        int minColor, maxColor;

        if (r < g) 
        {
	        minColor = (r < b) ? r : b;
	        maxColor = (g > b) ? g : b;
	    }
        else 
        {
	        minColor = (g < b) ? g : b;
	        maxColor = (r > b) ? r : b;
	    }
        return (minColor + maxColor) / 2;
        }

        //------------------------------------------------------------------------

        void SetupBMI (byte[] palette) 
        {
            //m_bmi.header.Size = sizeof (BitmapInfoHeader);
            //m_bmi.header.Width = 64;
            //m_bmi.header.Height = 64;
            //m_bmi.header.Planes = 1;
            //m_bmi.header.BitCount = 8;
            //m_bmi.header.Compression = BI_RGB;
            //m_bmi.header.SizeImage = 0;
            //m_bmi.header.XPelsPerMeter = 0;
            //m_bmi.header.YPelsPerMeter = 0;
            //m_bmi.header.ClrUsed = 0;
            //m_bmi.header.ClrImportant = 0;
            //for (int i = 0, j = 0; i < 256; i++)
            //{
            //    m_bmi.colors [i].rgbBlue = (byte) (palette [j++] * 4); 
            //    m_bmi.colors [i].rgbGreen = (byte) (palette [j++] * 4); 
            //    m_bmi.colors [i].rgbRed = (byte) (palette [j++] * 4); 
            //}
        }

        //------------------------------------------------------------------------

        void FreeCustom ()
        {
            m_bHaveCustom = false;
        }

        //------------------------------------------------------------------------

        void FreeDefault ()
        {
            m_bHaveDefault = false;
        }

        //------------------------------------------------------------------------

        void FreeRender ()
        {
            //m_render = null;
            //m_dlcLog = null;
            //m_colorMap = null;
        }

        //------------------------------------------------------------------------

        void Decode (BGR[] dest)
        {
            for (int i = 0, j = 0; i < 256; i++) 
            {
	            dest [i].r = (byte) (m_rawData [j++] * 4);
	            dest [i].g = (byte) (m_rawData [j++] * 4);
	            dest [i].b = (byte) (m_rawData [j++] * 4);
	        }
            m_superTransp = dest [254];
        }

        //------------------------------------------------------------------------

        void Encode (BGR[] src)
        {
            for (int i = 0, j = 0; i < 256; i++) 
            {
	            m_rawData [j++] = (byte) (src [i].r / 4);
	            m_rawData [j++] = (byte) (src [i].g / 4);
	            m_rawData [j++] = (byte) (src [i].b / 4);
	        }
        }

        //------------------------------------------------------------------------

        public int LoadCustom (BinaryReader fp, long size)
        {
        FreeCustom ();
        if (fp.Read (m_rawData, 0, paletteSize) != paletteSize)
	        return 0;
        Decode (m_custom);
        m_bHaveCustom = true;
        return 1;
        }

        //------------------------------------------------------------------------

        short SetupRender (byte[] palette)
        {
        FreeRender ();
        //if (!(m_dlcLog = (LPLOGPALETTE) new byte [sizeof (LOGPALETTE) + sizeof (PALETTEENTRY) * 256]))
        //    return 1;
        //m_dlcLog->palVersion = 0x300;
        //m_dlcLog->palNumEntries = 256;
        //uint* rgb = (uint*) &m_dlcLog->palPalEntry [0];
        //for (int i = 0; i < 256; i++, palette += 3)
        //    rgb [i] = ((uint) (palette [0]) << 2) + ((uint) (palette [1]) << 10) + ((uint) (palette [2]) << 18);
        //if (!(m_render = new Palette ()))
        //    return 1;
        //m_render->CreatePalette (m_dlcLog);
        //m_colorMap = new tRGBA [256];
        //m_render->GetPaletteEntries (0, 256, m_colorMap);
        //return m_render == null;
        return 0;
        }

        //------------------------------------------------------------------------

        BGR[] LoadDefault ()
        {
        Buffer.BlockCopy (m_rawData, 0, SelectResource (), 0, paletteSize);
         Decode (m_default);
        //SetupRender (m_default);
        //SetupBMI (m_default);
        return m_default;
        }

        //------------------------------------------------------------------------

        public BGR[] Current ()
        {
        if (m_bHaveCustom)
	        return m_custom;
        if (m_bHaveDefault)
	        return m_default;
        return LoadDefault ();
        }

        //------------------------------------------------------------------------
        // PaletteResource()
        //
        // Action - returns the name of the palette resource.  Neat part about
        //          this function is that the strings are automatically stored
        //          in the local heap so the string is static.
        //
        //------------------------------------------------------------------------

        byte[] SelectResource () 
        {

        if (DLE.IsD1File)
	        return Properties.Resources.palette;

        ResourceManager resMan = new ResourceManager ("DLEdotNET.Properties.Resources", typeof (Properties.Resources).Assembly);

        String filename = Path.GetFileName (DLE.descentPath [1]);
        foreach (String s in m_palResNames)
            if (s == filename)
            {
                return (byte[]) resMan.GetObject (s);
            }
        return (byte []) resMan.GetObject ("groupa.pig");
        }

        //------------------------------------------------------------------------

        void LoadName (BinaryReader fp)
        {
            m_name = "";

            int i;
            for (i = 0; i < 15; i++) {
	            m_name += fp.ReadChar ();
	            if (m_name [i] == 0x0a)
		            break;
	        }
            // replace extension with .pig
            if (i < 4) 
                m_name = "groupa.pig";
            else 
            {
                int l = m_name.Length - 4;
                m_name = m_name.Substring (1, l - 4) + ".pig";
	        }
        }

        //------------------------------------------------------------------------

        public byte ClosestColor (BGRA color)
        {
	        BGR[] palette = Current ();
	        uint delta, closestDelta = 0x7fffffff;
	        int i, closestIndex = 0;

        for (i = 0; i < 256; i++) {
            delta = color.Delta (palette [i]);
	        if (delta < closestDelta) {
		        if (delta == 0)
			        return (byte) i;
		        closestIndex = i;
		        closestDelta = delta;
		        }
	        }
        return (byte) closestIndex;
        }

        //------------------------------------------------------------------------
    }
}
