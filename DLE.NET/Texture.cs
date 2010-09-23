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

    public class PigHeader
    {
        public uint nId;
        public uint nVersion;
        public uint nTextures;
        public uint nSounds;

        public int Version { get; private set; }
        public uint Size { get { return (uint)((Version == 1) ? 2 : 3) * sizeof (uint); } }

        public PigHeader (int version = 1)
        {
            Version = version;
        }

        public void Read (BinaryReader fp)
        {
            if (Version == 1)
            {
                nId = fp.ReadUInt32 ();
                nVersion = fp.ReadUInt32 ();
                nTextures = fp.ReadUInt32 ();
            }
            else
            {
                nTextures = fp.ReadUInt32 ();
                nSounds = fp.ReadUInt32 ();
            }
        }
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class PigTexture
    {
        public ushort whExtra;     // bits 0-3 width, bits 4-7 height

        public byte [] name;
        public byte dflags;      // this is only important for large bitmaps like the cockpit
        public ushort width;
        public ushort height;
        public byte flags;
        public byte avgColor;
        public uint offset;

        public int Version { get; private set; }
        public uint Size { get { return (uint)((Version == 1) ? 18 : 17); } }

        public PigTexture (int version = 1)
        {
            Version = version;
        }

        void Setup (int nVersion, ushort w = 0, ushort h = 0, byte f = 0, uint o = 0)
        {
            Version = nVersion;
            width = w;
            height = h;
            flags = f;
            offset = o;
            dflags = 0;
            avgColor = 0;
        }

        void Decode ()
        {
            width += (ushort)((whExtra % 16) * 256);
            if (((flags & 0x80) != 0) && (width > 256))
                height *= width;
            else
                height += (ushort)((whExtra / 16) * 256);
        }

        void Encode ()
        {
            if (((flags & 0x80) != 0) && (width > 256))
            {
                whExtra = (ushort)(width / 256);
                height /= width;
            }
            else
            {
                whExtra = (ushort)((width / 256) + ((height / 256) * 16));
                height %= 256;
            }
            width %= 256;
        }

        public void Read (BinaryReader fp, int nVersion = -1)
        {
            if (nVersion >= 0)
                Version = nVersion;
            name = fp.ReadBytes (name.Length);
            dflags = fp.ReadByte ();
            width = (ushort)fp.ReadByte ();
            height = (ushort)fp.ReadByte ();
            if (Version == 1)
                whExtra = (ushort)fp.ReadByte ();     // bits 0-3 width, bits 4-7 height
            else
            {
                name [7] = 0;
                whExtra = 0;
            }
            flags = fp.ReadByte ();
            avgColor = fp.ReadByte ();
            offset = fp.ReadUInt32 ();
            Decode ();
        }

        public void Write (BinaryWriter fp)
        {
            Encode ();
            fp.Write (name);
            fp.Write (dflags);
            fp.Write ((byte)width);
            fp.Write ((byte)height);
            if (Version == 1)
                fp.Write ((byte)whExtra);
            fp.Write (flags);
            fp.Write (avgColor);
            fp.Write (offset);
        }

        public int BufSize ()
        {
            return (int)width * (int)height;
        }
    }

    //------------------------------------------------------------------------------

    unsafe struct PigSoundD1
    {
        fixed byte unknown [20];

        public const uint Size = 20;
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class Texture
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

        public void Clear ()
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

        void Allocate (uint nSize, ushort nTexture)
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

        double Scale (short nTexture)
        {
            return (m_width > 0) ? m_width / 64.0 : 1.0;
        }

        //------------------------------------------------------------------------------

    }
}