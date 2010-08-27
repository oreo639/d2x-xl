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
        public int nTextures;
        public int nSounds;

        public const uint Size = 2 * sizeof (int);

        public void Read (BinaryReader fp)
        {
            nTextures = fp.ReadInt32 ();
            nSounds = fp.ReadInt32 ();
        }
    } 

    public struct PigTexture
    {
        public byte[] name = new byte[8];
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
            m_info.dflags = fp.ReadByte();
            m_info.width = (ushort) fp.ReadByte();
            m_info.height = (ushort) fp.ReadByte();
            m_info.flags = fp.ReadByte();
            m_info.avgColor = fp.ReadByte();
            m_info.offset = fp.ReadUInt32();
        }
    } 

    //------------------------------------------------------------------------------

    public class PigHeaderD2
    {
        public int signature;
        public int version;
        public int nTextures;

        public const uint Size = 3 * sizeof (int);

        public void Read (BinaryReader fp)
        {
            signature = fp.ReadInt32 ();
            version = fp.ReadInt32 ();
            nTextures = fp.ReadInt32 ();
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
            m_info.dflags = fp.ReadByte();
            m_info.width = (ushort) fp.ReadByte();
            m_info.height = (ushort) fp.ReadByte();
            whExtra = (ushort) fp.ReadByte ();     // bits 0-3 width, bits 4-7 height
            m_info.width += (ushort) ((whExtra % 16) * 256);
            m_info.height += (ushort) ((whExtra / 16) * 256);
            m_info.flags = fp.ReadByte();
            m_info.avgColor = fp.ReadByte();
            m_info.offset = fp.ReadUInt32();
        }
    }

    public unsafe struct PigSoundD1
    {
        fixed byte unknown [20];
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

       	public Texture (byte[] dataP = null) 
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

    	virtual override void Read (BinaryReader fp, int version = 0, bool bFlag = false) {}
        virtual override void Write (BinaryWriter fp, int version = 0, bool bFlag = false) {}

        //------------------------------------------------------------------------------

        virtual override void Clear () 
        {
            Release ();
            width = height = size = 0;
            bModified = bExtData = bValid = false;
            nFormat = 0;
        }

        //------------------------------------------------------------------------------

        int Read (short index) 
        {
	        byte[]			xsize = new byte [200];
	        byte[]			line = new byte [320];
	        byte			j;
	        PigTextureD1	pigTexD1;
	        PigTextureD2    pigTexD2;
	        PigHeaderD1		fileHeaderD1;
	        PigHeaderD2	    fileHeaderD2;
	        uint			offset,dataOffset;
	        int 			x, y, w, h, s;
	        byte			byteVal, runcount, runvalue;
	        int				nSize;
	        short			nLine;
	        int				rc;
	        short			linePtr, texPtr;
            String          folder;
	
        if ((index > ((DLE.IsD1File ()) ? TextureManager.MAX_D1_TEXTURES : TextureManager.MAX_D2_TEXTURES)) || (index < 0))
            throw (new ArgumentException ("Reading texture: Texture #" + index + " out of range."));
        if (bModified)
	        return 0;
        folder = DLE.settings.gameFolders [DLE.IsD1File () ? 0 : 1];
        if (!folder.Contains (".pig"))
            folder += "groupa.pig";

        byte[] textureTable = (DLE.IsD1File() ? Properties.Resources.tnames : Properties.Resources.tnames2);

        using (FileStream fs = File.OpenRead (folder))
        {
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
    	        offset = (textureTable [index] - 1) * PigTextureD2.Size;
                fs.Seek (offset, SeekOrigin.Current);
                pigTexD2.Read (fp);
            }
            else
            {
                fileHeaderD1.Read (fp);
    	        offset = (textureTable [index] - 1) * PigTextureD1.Size;
                fs.Seek (offset, SeekOrigin.Current);
                pigTexD1.Read (fp);
                pigTexD1.m_info.name [7] = 0;
	            // copy d1 texture into d2 texture struct
	            pigTexD2.m_info = pigTexD1.m_info;
	            pigTexD2.whExtra = (ushort ((pigTexD1.m_info.dflags == 128) ? 1 : 0);
	            w = h = 64;
            }

         s = pigTexD2.m_info.width * pigTexD2.m_info.height;

        // seek to data
        if (DLE.IsD2File ()) {
	        offset = PigHeaderD2.Size
				        + dataOffset
				        + fileHeaderD2.nTextures * PigTextureD2.Size
				        + pigTexD2.m_info.offset;
	        }
        else {
	        offset = sizeof (PigHeaderD1) + dataOffset
				        + fileHeaderD1.nTextures * pigTextureD1.Size
				        + fileHeaderD1.nSounds * sizeof (PigSoundD1)
				        + pigTexD2.m_info.offset;
	        }

        // allocate data if necessary
        if ((bmData != null) && ((width != pigTexD2.m_info.width) || (height != pigTexD2.m_info.height)))
	        Release ();
        if (bmData == null)
	        bmData = new byte [s];
        if (bmData == null)
            throw (new InsufficientMemoryException ("Reading texture: Not enough memory for texture #" + index + "."));
        nFormat = 0;
        fseek (fTextures, offset, SEEK_SET);
        if (pigTexD2.flags & 0x08) {
	        fread (&nSize, 1, sizeof (int), fTextures);
	        fread (width, pigTexD2.height, 1, fTextures);
	        nLine = 0;
	        for (y = h - 1; y >= 0; y--) {
		        fread (line, width[nLine++], 1, fTextures);
		        line_ptr = line;
			        for (x = 0; x < w;) {
			        byteVal = *line_ptr++;
			        if ((byteVal & 0xe0) == 0xe0) {
				        runcount = byteVal & 0x1f;
				        runvalue = *line_ptr++;
				        for (j = 0; j < runcount; j++) {
					        if (x < w) {
						        bmData [y * w + x] = runvalue;
						        x++;
						        }
					        }
				        }
			        else {
				        bmData [y * w + x] = byteVal;
				        x++;
				        }
			        }
		        }
	        }
        else {
	        for (y=h-1;y>=0;y--) {
        #if 1
		        fread (bmData + y * w, w, 1, fTextures);
        #else
		        fread (line,w,1,fTextures);
		        line_ptr = line;
		        for (x=0;x<w;x++) {
			        byteVal = *line_ptr++;
			        bmData[y*w+x] = byteVal;
			        }
        #endif
		        }
	        }
        fclose (fTextures);
        width = w;
        height = h;
        size = s;
        bValid = 1;
        return (0);

        abort:
        // free handle
        if (hGlobal) FreeResource (hGlobal);
        return (rc);
        }

    }
}
