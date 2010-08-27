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

    struct tRGBA
    {
        byte r, g, b, a;
    }


    struct tBGRA
    {
        byte b, g, r, a;
    }

    struct tABGR
    {
        byte a, b, g, r;
    }

    struct tBGR
    {
        byte r, g, b;
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class PigHeaderD1
    {
        public uint nTextures;
        public uint nSounds;

        public const uint Size = 2 * sizeof (int);

        public void Read (BinaryReader fp)
        {
            nTextures = fp.ReadUInt32 ();
            nSounds = fp.ReadUInt32 ();
        }
    }

    public struct PigTexture
    {
        public byte [] name = new byte [8];
        public byte dflags;      // this is only important for large bitmaps like the cockpit
        public ushort width;
        public ushort height;
        public byte flags;
        public byte avgColor;
        public uint offset;
    }

    public class PigTextureD1
    {
        public PigTexture m_info;

        public const uint Size = 14;

        public void Read (BinaryReader fp)
        {
            m_info.name = fp.ReadBytes (m_info.name.Length);
            m_info.dflags = fp.ReadByte ();
            m_info.width = (ushort)fp.ReadByte ();
            m_info.height = (ushort)fp.ReadByte ();
            m_info.flags = fp.ReadByte ();
            m_info.avgColor = fp.ReadByte ();
            m_info.offset = fp.ReadUInt32 ();
        }
    }

    //------------------------------------------------------------------------------

    public class PigHeaderD2
    {
        public uint signature;
        public uint version;
        public uint nTextures;

        public const uint Size = 3 * sizeof (int);

        public void Read (BinaryReader fp)
        {
            signature = fp.ReadUInt32 ();
            version = fp.ReadUInt32 ();
            nTextures = fp.ReadUInt32 ();
        }
    }

    public class PigTextureD2 : PigTextureD1
    {
        public PigTexture m_info;
        public ushort whExtra;     // bits 0-3 width, bits 4-7 height

        public const uint Size = 15;

        public override void Read (BinaryReader fp)
        {
            m_info.name = fp.ReadBytes (m_info.name.Length);
            m_info.dflags = fp.ReadByte ();
            m_info.width = (ushort)fp.ReadByte ();
            m_info.height = (ushort)fp.ReadByte ();
            whExtra = (ushort)fp.ReadByte ();     // bits 0-3 width, bits 4-7 height
            m_info.width += (ushort)((whExtra % 16) * 256);
            m_info.height += (ushort)((whExtra / 16) * 256);
            m_info.flags = fp.ReadByte ();
            m_info.avgColor = fp.ReadByte ();
            m_info.offset = fp.ReadUInt32 ();
        }
    }

    class PigSoundD1
    {
        fixed byte unknown [20];

        public const uint Size = 20;
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class Texture : GameItem
    {
        public byte [] bmData;
        public tRGBA [] tgaData;
        public uint width = 0, height = 0, size = 0;
        public bool bModified = false, bExtData = false, bValid = false;
        public byte nFormat = 0;	// 0: Bitmap, 1: TGA (RGB)

        //------------------------------------------------------------------------------

        public Texture (byte [] dataP = null)
        {
            Clear ();
            bmData = dataP;
            bExtData = (dataP != null);
        }

        //------------------------------------------------------------------------------

        void Release ()
        {
            bmData = null;
            tgaData = null;
        }

        //------------------------------------------------------------------------------

        virtual override void Read (BinaryReader fp, int version = 0, bool bFlag = false) { }
        virtual override void Write (BinaryWriter fp, int version = 0, bool bFlag = false) { }

        //------------------------------------------------------------------------------

        virtual override void Clear ()
        {
            Release ();
            width = height = size = 0;
            bModified = bExtData = bValid = false;
            nFormat = 0;
        }

        //------------------------------------------------------------------------------

        ushort TextureIndex (byte [] textureTable, short index)
        {
            return (ushort)(textureTable [index + 2] + (int)textureTable [index + 3] * 256);
        }

        //------------------------------------------------------------------------------

        new int Read (short index) 
        {
	        byte[]			rowSize = new byte [4096];
	        byte[]			rowData = new byte [4096];
	        PigTextureD1	pigTexD1 = new PigTextureD1();
	        PigTextureD2    pigTexD2 = new PigTextureD2();
	        PigHeaderD1		fileHeaderD1 = new PigHeaderD1();
	        PigHeaderD2	    fileHeaderD2 = new PigHeaderD2();
	        uint			offset,dataOffset;
	        byte			byteVal, runLength, runValue;
	        uint			nSize;
	        short			nLine;
	        int				rc;
            String          folder;
	
        if ((index > ((DLE.IsD1File ()) ? TextureManager.MAX_D1_TEXTURES : TextureManager.MAX_D2_TEXTURES)) || (index < 0))
            throw (new ArgumentException ("Reading texture: Texture #" + index + " out of range."));
        if (bModified)
	        return 0;
        folder = DLE.settings.gameFolders [DLE.IsD1File () ? 0 : 1];
        if (!folder.Contains (".pig"))
            folder += "groupa.pig";

        byte[] textureTable = (DLE.IsD1File() ? Properties.Resources.texture : Properties.Resources.texture2);

        using (FileStream fs = File.OpenRead (folder))
            using (BinaryReader fp = new BinaryReader (fs))
            {
                dataOffset = fp.ReadUInt32 ();
                if (dataOffset == 0x47495050L) /* 'PPIG' Descent 2 type */
	                dataOffset = 0;
                else if (dataOffset < 0x10000L)
    	            dataOffset = 0;
                fs.Seek (dataOffset, SeekOrigin.Begin);
                if (DLE.IsD2File ())
                {
                    fileHeaderD2.Read (fp);
    	            offset = PigHeaderD2.Size + dataOffset + (uint) (TextureIndex (textureTable, index) - 1) * PigTextureD2.Size;
                    fs.Seek (offset, SeekOrigin.Begin);
                    pigTexD2.Read (fp);
                }
                else
                {
                    fileHeaderD1.Read (fp);
    	            offset = PigHeaderD1.Size + dataOffset + (uint) (TextureIndex (textureTable, index) - 1) * PigTextureD1.Size;
                    fs.Seek (offset, SeekOrigin.Begin);
                    pigTexD1.Read (fp);
                    pigTexD1.m_info.name [7] = 0;
	                // copy d1 texture into d2 texture struct
	                pigTexD2.m_info = pigTexD1.m_info;
	                pigTexD2.whExtra = (ushort) ((pigTexD1.m_info.dflags == 128) ? 1 : 0);
                }

                nSize = (uint) (pigTexD2.m_info.width * pigTexD2.m_info.height);
                if ((bmData != null) && ((width != pigTexD2.m_info.width) || (height != pigTexD2.m_info.height)))
	                Release ();
                if (bmData == null)
	                bmData = new byte [size];
                if (bmData == null)
                    throw (new InsufficientMemoryException ("Reading texture: Not enough memory for texture #" + index + "."));
                nFormat = 0;

	            offset = dataOffset + pigTexD2.m_info.offset +
                            (DLE.IsD2File () 
                            ? PigHeaderD2.Size + fileHeaderD2.nTextures * PigTextureD2.Size 
                            : PigHeaderD1.Size + fileHeaderD1.nTextures * PigTextureD1.Size + fileHeaderD1.nSounds * PigSoundD1.Size);
                fs.Seek (offset, SeekOrigin.Begin);
                if (((int) pigTexD2.m_info.flags & 0x08) == 0) 
                {
	                for (int y = pigTexD2.m_info.height - 1; y >= 0; y--) 
                        Array.Copy (fp.ReadBytes (pigTexD2.m_info.width), 0, bmData, (long) (y * pigTexD2.m_info.width), pigTexD2.m_info.width);
	            }
                else 
                {
                    nSize = fp.ReadUInt32 ();
                    rowSize = fp.ReadBytes (pigTexD2.m_info.height);
	                nLine = 0;
	                for (int y = pigTexD2.m_info.height - 1; y >= 0; y--)
                    {
                        rowData = fp.ReadBytes (rowSize [nLine++]);
		                for (int x = 0, i = 0; x < pigTexD2.m_info.width; ) 
                        {
			                byteVal = rowData [i];
			                if ((byteVal & 0xe0) == 0xe0) 
                            {
				                runLength = (byte) (byteVal & 0x1f);
				                runValue = rowData [i++];
				                for (int j = 0; j < runLength; j++) 
                                {
					                if (x < pigTexD2.m_info.width) 
                                    {
						                bmData [y * pigTexD2.m_info.width + x] = runValue;
						                x++;
						            }
					            }
				            }
			                else 
                            {
				                bmData [y * pigTexD2.m_info.width + x] = byteVal;
				                x++;
				            }
			            }
		            }
	            }
            }
            return 1;
        }
    }
}