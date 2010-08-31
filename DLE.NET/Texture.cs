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
        public byte [] m_bmData;
        public tRGBA [] m_tgaData;
        public uint m_width = 0, m_height = 0, m_size = 0;
        public bool m_bCustom = false, m_bExtData = false, m_bFrame = false, m_bUsed = false, m_bValid = false;
        public byte m_nFormat = 0;	// 0: Bitmap, 1: TGA (RGB)
        public ushort m_nIndex = 0;

        //------------------------------------------------------------------------------

        public Texture (byte [] dataP = null)
        {
            Clear ();
            m_bmData = dataP;
            m_bExtData = (dataP != null);
        }

        //------------------------------------------------------------------------------

        public void Release ()
        {
            m_bmData = null;
            m_tgaData = null;
        }

        //------------------------------------------------------------------------------

        public override void Read (BinaryReader fp, int version = 0, bool bFlag = false) { }
        public override void Write (BinaryWriter fp, int version = 0, bool bFlag = false) { }

        //------------------------------------------------------------------------------

        public override void Clear ()
        {
            Release ();
            m_width = m_height = m_size = 0;
            m_bCustom = m_bExtData = m_bValid = false;
            m_nFormat = 0;
        }

        //------------------------------------------------------------------------------

        ushort TextureIndex (byte [] textureIndex, short index)
        {
            return (ushort)(textureIndex [index + 2] + (int)textureIndex [index + 3] * 256);
        }

        //------------------------------------------------------------------------------

        void Allocate(uint nSize, ushort nTexture)
        {
            if ((m_bmData != null) && (m_width * m_height != nSize))
	            Release ();
            if (m_bmData == null)
            {
                try
                {
                    m_bmData = new byte [m_size];
                }
                catch (InsufficientMemoryException)
                {
                    throw (new InsufficientMemoryException ("Reading texture: Not enough memory for texture #" + nTexture + "."));
                }
            }
            m_nFormat = 0;
        }

        //------------------------------------------------------------------------------

        void Load (BinaryReader fp, PigTexture info)
        {
            byte [] rowSize = new byte [4096];
            byte [] rowData = new byte [4096];

            if (((int)info.flags & 0x08) == 0)
            {
                for (int y = info.height - 1; y >= 0; y--)
                    Array.Copy (fp.ReadBytes (info.width), 0, m_bmData, (long)(y * info.width), info.width);
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
                                    m_bmData [y * info.width + x] = runValue;
                                    x++;
                                }
                            }
                        }
                        else
                        {
                            m_bmData [y * info.width + x] = byteVal;
                            x++;
                        }
                    }
                }
            }
        }

        //------------------------------------------------------------------------

        bool Allocate (int nSize, int nTexture)
        {
            if ((m_bmData != null) && ((m_width * m_height != nSize)))
	            Release ();
            if (m_bmData == null)
	            m_bmData = new byte [nSize];
            return (m_bmData != null);
        }

        //------------------------------------------------------------------------

        public int Load (FileStream fs, short nTexture, int nVersion) 
        {
            if (m_bCustom)
	            return 0;

            if (nVersion < 0)
	            nVersion = DLE.IsD1File ? 0 : 1;

            PigTexture info = DLE.textureManager.Info (nTexture);
            int nSize = info.BufSize ();
            if ((m_bmData != null) && ((m_width * m_height == nSize)))
	            return 0; // already loaded
            if (!Allocate (nSize, nTexture))
	            return 1;
            using (BinaryReader fp = new BinaryReader (fs))
            {
                fs.Seek (DLE.textureManager.Offset + info.offset, SeekOrigin.Begin);
                Load (fp, info);
                m_bFrame = DLE.textureManager.Name (nTexture).Contains ("frame");
                return 0;
            }
        }

        //------------------------------------------------------------------------------

    }
}