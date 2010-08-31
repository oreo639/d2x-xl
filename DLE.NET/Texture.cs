using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public struct tRGBA
    {
        byte r, g, b, a;
    }


    public struct tBGRA
    {
        byte b, g, r, a;
    }

    public struct tABGR
    {
        byte a, b, g, r;
    }

    public struct tBGR
    {
        byte r, g, b;
    }

    
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class Texture : GameItem
    {
        public byte [] bmData;
        public tRGBA [] tgaData;
        public uint width = 0, height = 0, size = 0;
        public bool bCustom = false, bExtData = false, bFrame = false, bUsed = false, bValid = false;
        public byte nFormat = 0;	// 0: Bitmap, 1: TGA (RGB)

        //------------------------------------------------------------------------------

        public Texture (byte [] dataP = null)
        {
            Clear ();
            bmData = dataP;
            bExtData = (dataP != null);
        }

        //------------------------------------------------------------------------------

        public void Release ()
        {
            bmData = null;
            tgaData = null;
        }

        //------------------------------------------------------------------------------

        public override void Read (BinaryReader fp, int version = 0, bool bFlag = false) { }
        public override void Write (BinaryWriter fp, int version = 0, bool bFlag = false) { }

        //------------------------------------------------------------------------------

        public override void Clear ()
        {
            Release ();
            width = height = size = 0;
            bCustom = bExtData = bValid = false;
            nFormat = 0;
        }

        //------------------------------------------------------------------------------

        ushort TextureIndex (byte [] textureIndex, short index)
        {
            return (ushort)(textureIndex [index + 2] + (int)textureIndex [index + 3] * 256);
        }

        //------------------------------------------------------------------------------

        void Allocate(uint nSize, ushort nTexture)
        {
            if ((bmData != null) && (width * height != nSize))
	            Release ();
            if (bmData == null)
            {
                try
                {
                    bmData = new byte [size];
                }
                catch (InsufficientMemoryException)
                {
                    throw (new InsufficientMemoryException ("Reading texture: Not enough memory for texture #" + nTexture + "."));
                }
            }
            nFormat = 0;
        }

        //------------------------------------------------------------------------------

        void Load (BinaryReader fp, PigTexture info)
        {
            byte [] rowSize = new byte [4096];
            byte [] rowData = new byte [4096];

            if (((int)info.flags & 0x08) == 0)
            {
                for (int y = info.height - 1; y >= 0; y--)
                    Array.Copy (fp.ReadBytes (info.width), 0, bmData, (long)(y * info.width), info.width);
            }
            else
            {
                uint nSize = fp.ReadUInt32 ();
                rowSize = fp.ReadBytes (info.height);
                byte byteVal, runLength, runValue;
                ushort nLine = 0;
                for (int y = info.height - 1; y >= 0; y--)
                {
                    rowData = fp.ReadBytes (rowSize [nLine++]);
                    for (int x = 0, i = 0; x < info.width; )
                    {
                        byteVal = rowData [i];
                        if ((byteVal & 0xe0) == 0xe0)
                        {
                            runLength = (byte)(byteVal & 0x1f);
                            runValue = rowData [i++];
                            for (int j = 0; j < runLength; j++)
                            {
                                if (x < info.width)
                                {
                                    bmData [y * info.width + x] = runValue;
                                    x++;
                                }
                            }
                        }
                        else
                        {
                            bmData [y * info.width + x] = byteVal;
                            x++;
                        }
                    }
                }
            }
        }

        //------------------------------------------------------------------------

        bool Allocate (int nSize, int nTexture)
        {
            if ((bmData != null) && ((width * height != nSize)))
	            Release ();
            if (bmData == null)
	            bmData = new byte [nSize];
            return (bmData != null);
        }

        //------------------------------------------------------------------------

        public int Load (FileStream fs, short nTexture, int nVersion) 
        {
            if (bCustom)
	            return 0;

            if (nVersion < 0)
	            nVersion = DLE.IsD1File () ? 0 : 1;

            PigTexture info = DLE.textureManager.Info (nTexture);
            int nSize = info.BufSize ();
            if ((bmData != null) && ((width * height == nSize)))
	            return 0; // already loaded
            if (!Allocate (nSize, nTexture))
	            return 1;
            using (BinaryReader fp = new BinaryReader (fs))
            {
                fs.Seek (DLE.textureManager.Offset + info.offset, SeekOrigin.Begin);
                Load (fp, info);
                bFrame = DLE.textureManager.Name (nTexture).Contains ("frame");
                return 0;
            }
        }

        //------------------------------------------------------------------------------

    }
}