using System;
using System.IO;
using System.Runtime.InteropServices;

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

	    public void Write (BinaryWriter fp) 
        {
            if (nVersion == 1)
            {
                fp.Write (nId);
                fp.Write (nVersion);
                fp.Write (nTextures);
            }
            else
            {
                fp.Write (nTextures);
                fp.Write (nSounds);
            }
        }
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class PigTexture
    {
        public ushort whExtra;     // bits 0-3 width, bits 4-7 height

        public string name;
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

        public void Setup (int nVersion, ushort w = 0, ushort h = 0, byte f = 0, uint o = 0)
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

            name = System.Text.Encoding.ASCII.GetString (fp.ReadBytes (8));
            dflags = fp.ReadByte ();
            width = (ushort)fp.ReadByte ();
            height = (ushort)fp.ReadByte ();
            if (Version == 1)
                whExtra = (ushort)fp.ReadByte ();     // bits 0-3 width, bits 4-7 height
            else
            {
                name = "";
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

        public uint BufSize ()
        {
            return (uint)width * (uint)height;
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
        public enum Flags : byte
        {
            TRANSPARENT = 1,
            SUPER_TRANSPARENT = 2,
            NO_LIGHTING = 4,
            RLE = 8,
            PAGED_OUT = 16,
            RLE_BIG = 32
        }

        //----------------------------------------------------------------------------

        [StructLayout (LayoutKind.Explicit)]
        public struct BitmapData
        {
            [FieldOffset (0)]
            public BGRA [] m_color;
            [FieldOffset (0)]
            public byte [] m_index;
        }

        public BitmapData m_bmData = new BitmapData ();
        public BGRA [] m_override;
        public uint m_width = 0, m_height = 0, m_size = 0, m_offset = 0;
        public int m_id = -1;
        public bool m_bCustom = false, m_bExtData = false, m_bFrame = false, m_bUsed = false, m_bValid = false;
        public byte m_nFormat = 0;	// 0: Bitmap, 1: TGA (RGB)
        public ushort m_nIndex = 0;

        public BGRA [] Buffer 
        { 
            get { return m_bmData.m_color; } 
            set { m_bmData.m_color = value; } 
        }
        public byte [] Index { get { return m_bmData.m_index; } }
        public uint Width { get { return m_width; } }
        public uint Height { get { return m_height; } }
        public uint Size { get { return Width * Height; } }

        //----------------------------------------------------------------------------

        public Texture (BGRA[] data = null)
        {
            Clear ();
            Buffer = data;
            m_bExtData = (data != null);
        }

        //-----------------------------------------------------------------------------

        public bool Allocate (uint nSize)
        {
            if ((Buffer != null) && ((m_width * m_height != nSize)))
                Release ();
            if (Buffer == null)
                Buffer = new BGRA [nSize];
            return (Buffer != null);
        }

        //------------------------------------------------------------------------------

        public void Release ()
        {
            if (!m_bExtData)
                Buffer = null;
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
            if ((Buffer != null) && (m_width * m_height != nSize))
	            Release ();
            if (Buffer == null)
            {
                try
                {
                    Buffer = new BGRA [m_size];
                }
                catch (InsufficientMemoryException)
                {
                    throw (new InsufficientMemoryException ("Reading texture: Not enough memory for texture #" + nTexture + "."));
                }
            }
            m_nFormat = 0;
        }

        //------------------------------------------------------------------------------

        public void ComputeIndex (byte [] bmIndex)
        {
	        BGR[] palette = DLE.Palettes.Current ();

	        for (uint y = 0; y < (int) Height; y++) {
		        uint i = y * Width;
		        uint k = Size - Width - i;
		        for (uint x = 0; x < Width; x++) 
                {
			        bmIndex [k + x] = DLE.Palettes.ClosestColor (Buffer [i + x]);
		        }
	        }
        }

        //------------------------------------------------------------------------------

        public void Load (BinaryReader fp, PigTexture info)
        {
            byte [] rowSize = new byte [4096];
            byte [] rowData = new byte [4096];
            BGR [] palette = DLE.Palettes.Current ();
            byte palIndex, alpha;

            if (m_nFormat != 0)
            {
                for (uint i = 0; i < m_size; i++)
                {
                    Buffer [i].r = fp.ReadByte ();
                    Buffer [i].g = fp.ReadByte ();
                    Buffer [i].b = fp.ReadByte ();
                    Buffer [i].a = fp.ReadByte ();
                }
                //texP->m_info.bValid = TGA2Bitmap (texP->m_info.bmData, texP->m_info.bmData, (int) pigTexInfo.width, (int) pigTexInfo.height);
            }
            else if (((int)info.flags & 0x08) != 0)
            {
                uint nSize = fp.ReadUInt32 ();
                rowSize = fp.ReadBytes (info.height);
                byte runLength;
                ushort nLine = 0;
                for (int y = info.height - 1; y >= 0; y--)
                {
                    rowData = fp.ReadBytes (rowSize [nLine++]);
                    for (int x = 0, i = 0; x < info.width; )
                    {
                        palIndex = rowData [i];
                        if ((palIndex & 0xe0) == 0xe0)
                        {
                            runLength = (byte)(palIndex & 0x1f);
                            palIndex = rowData [i++];
                            alpha = (byte)((palIndex < 254) ? 255 : 0);
                            for (int j = 0; j < runLength; j++)
                            {
                                if (x < info.width)
                                {
                                    Buffer [y * info.width + x].Assign (palette [palIndex], alpha);
                                    x++;
                                }
                            }
                        }
                        else
                        {
                            Buffer [y * info.width + x].Assign (palette [palIndex], (byte) ((palIndex < 254) ? 255 : 0));
                            x++;
                        }
                    }
                }
            }
            else
            {
	            for (int y = info.height - 1; y >= 0; y--) 
                    {
		            for (int x = 0; x < info.width; x++) 
                    {
			            int h = y * info.width + x;
			            palIndex = fp.ReadByte ();
			            Buffer [h].Assign (palette [palIndex]);
                        if (palIndex < 254)
                            Buffer [h].a = 0; // transparent
			        }
                }
            }
        }

        //------------------------------------------------------------------------

        public int Load (FileStream fs, short nTexture, int nVersion) 
        {
            if (m_bCustom)
	            return 0;

            if (nVersion < 0)
	            nVersion = DLE.IsD1File ? 0 : 1;

            PigTexture info = DLE.Textures.Info (nTexture);
            uint nSize = info.BufSize ();
            if ((Buffer != null) && ((m_width * m_height == nSize)))
	            return 0; // already loaded
            if (!Allocate (nSize))
	            return 1;
            using (BinaryReader fp = new BinaryReader (fs))
            {
                fs.Seek (DLE.Textures.Offset + info.offset, SeekOrigin.Begin);
                Load (fp, info);
                m_bFrame = DLE.Textures.Name (nTexture).Contains ("frame");
                return 0;
            }
        }

        //------------------------------------------------------------------------------

        public double Scale (short nTexture)
        {
            return (m_width > 0) ? m_width / 64.0 : 1.0;
        }

        //------------------------------------------------------------------------------

    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class ExtraTexture : Texture
    {
        public ExtraTexture m_next;
        public ushort m_index;
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

}