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
        int nTextures;
        int nSounds;

        //------------------------------------------------------------------------------

        public void Read (BinaryReader fp)
        {
            nTextures = fp.ReadInt32 ();
            nSounds = fp.ReadInt32 ();
        }
    } 

    public class PigTextureD1
    {
        byte[] name = new byte[8];
        byte dflags;      // this is only important for large bitmaps like the cockpit
        byte width;
        byte height;
        byte flags;
        byte avgColor;
        uint offset;

        public void Read (BinaryReader fp)
        {
            name = fp.ReadBytes (name.Length);
            dflags = fp.ReadByte();
            width = fp.ReadByte();
            height = fp.ReadByte();
            flags = fp.ReadByte();
            avgColor = fp.ReadByte();
            offset = fp.ReadUInt32();
        }
    } 

    //------------------------------------------------------------------------------

    public class PigHeaderD2
    {
        int signature;
        int version;
        int nTextures;

        public void Read (BinaryReader fp)
        {
            signature = fp.ReadInt32 ();
            version = fp.ReadInt32 ();
            nTextures = fp.ReadInt32 ();
        }
    } 

    public class PigTextureD2
    {
        byte[] name = new byte[8];
        byte dflags;      // bits 0-5 anim frame num, bit 6 abm flag
        ushort width;       // low 8 bits here, 4 more bits in pad
        ushort height;      // low 8 bits here, 4 more bits in pad
        byte flags;       // see BM_FLAG_XXX in define.h
        byte avgColor;    // average color
        uint offset;

        public void Read (BinaryReader fp)
        {
            name = fp.ReadBytes (name.Length);
            dflags = fp.ReadByte();
            width = (ushort) fp.ReadByte();
            height = (ushort) fp.ReadByte();
            ushort whExtra = (ushort) fp.ReadByte ();     // bits 0-3 width, bits 4-7 height
            width += (ushort) ((whExtra % 16) * 256);
            height += (ushort) ((whExtra / 16) * 256);
            flags = fp.ReadByte();
            avgColor = fp.ReadByte();
            offset = fp.ReadUInt32();
        }
    }

    public class PigSoundD2
    {
        byte[] unknown = new byte[20];
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
	        int				offset,dataOffset;
	        short			x,y,w,h,s;
	        byte			byteVal, runcount, runvalue;
	        int				nSize;
	        short			nLine;
            //HRSRC			hFind = 0;
            //HGLOBAL			hGlobal = 0;
            //short			*textureTable;
	        int				rc;
	        short			linePtr, texPtr;
            Stream          fTextures;
            String          folder;
	
        if ((index > ((DLE.IsD1File ()) ? TextureManager.MAX_D1_TEXTURES : TextureManager.MAX_D2_TEXTURES)) || (index < 0))
            throw (new ArgumentException ("Reading texture: Texture #" + i + " out of range.");
        if (bModified)
	        return 0;
        folder = DLE.settings.gameFolders [DLE.IsD1File () ? 0 : 1];
        if (!folder.Contains (".pig"))
            folder += "groupa.pig";

            = DLE.Properties.Resources.Data.

        HINSTANCE hInst = AfxGetInstanceHandle ();

        // do a range check on the texture number
        DEBUGMSG ();
        rc = 1;
        goto abort;
        }

        // get pointer to texture table from resource fTextures
        hFind = (DLE.IsD1File ()) ?
	        FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE_DAT), "RC_DATA") : 
	        FindResource (hInst,MAKEINTRESOURCE (IDR_TEXTURE2_DAT), "RC_DATA");
        if (!hFind) {
	        DEBUGMSG (" Reading texture: Texture resource not found.");
	        rc = 1;
	        goto abort;
	        }
        hGlobal = LoadResource (hInst, hFind);
        if (!hGlobal) {
	        DEBUGMSG (" Reading texture: Could not load texture resource.");
	        rc = 2;
	        goto abort;
	        }
        // first long is number of textures
        texture_ptr = (short *)LockResource (hGlobal);
        textureTable = texture_ptr + 2;

        if (fopen_s (&fTextures, path, "rb")) {
	        DEBUGMSG (" Reading texture: Texture file not found.");
	        rc = 1;
	        goto abort;
	        }

        // read fTextures header
        fseek (fTextures, 0, SEEK_SET);
        dataOffset = ReadInt32 (fTextures);
        if (dataOffset == 0x47495050L) /* 'PPIG' Descent 2 type */
	        dataOffset = 0;
        else if (dataOffset < 0x10000L)
	        dataOffset = 0;
        fseek (fTextures, dataOffset, SEEK_SET);
        if (DLE.IsD2File ())
	        fread (&fileHeaderD2, sizeof (fileHeaderD2), 1, fTextures);
        else
	        fread (&fileHeaderD1, sizeof (fileHeaderD1), 1, fTextures);

        // read texture header
        if (DLE.IsD2File ()) {
	        offset = sizeof (PigHeaderD2) + dataOffset +
				        (fix) (textureTable[index]-1) * sizeof (PigTextureD2);
	        fseek (fTextures, offset, SEEK_SET);
	        fread (&pigTexD2, sizeof (PigTextureD2), 1, fTextures);
	        w = pigTexD2.xsize + ((pigTexD2.wh_extra & 0xF) << 8);
	        h = pigTexD2.ysize + ((pigTexD2.wh_extra & 0xF0) << 4);
	        }
        else {
	        offset = sizeof (PigHeaderD1) + dataOffset + (fix) (textureTable [index] - 1) * sizeof (ptexture);
	        fseek (fTextures, offset, SEEK_SET);
	        fread (&ptexture, sizeof (PigTexture), 1, fTextures);
	        ptexture.name [sizeof (ptexture.name) - 1] = '\0';
	        // copy d1 texture into d2 texture struct
	        strncpy_s (pigTexD2.name, sizeof (pigTexD2.name), ptexture.name, sizeof (ptexture.name));
	        pigTexD2.dflags = ptexture.dflags;
	        pigTexD2.xsize = ptexture.xsize;
	        pigTexD2.ysize = ptexture.ysize;
	        pigTexD2.flags = ptexture.flags;
	        pigTexD2.avg_color = ptexture.avg_color;
	        pigTexD2.offset = ptexture.offset;
	        pigTexD2.wh_extra = (ptexture.dflags == 128) ? 1 : 0;
	        w = h = 64;
	        }
        s = w * h;

        // seek to data
        if (DLE.IsD2File ()) {
	        offset = sizeof (PigHeaderD2) 
				        + dataOffset
				        + fileHeaderD2.nTextures * sizeof (PigTextureD2)
				        + pigTexD2.offset;
	        }
        else {
	        offset = sizeof (PigHeaderD1) + dataOffset
				        + fileHeaderD1.nTextures * sizeof (PigTexture)
				        + fileHeaderD1.nSounds   * sizeof (PIG_SOUND)
				        + pigTexD2.offset;
	        }

        // allocate data if necessary
        if (bmDataP && ((width != w) || (height != h)))
	        Release ();
        if (bmDataP == NULL)
	        bmDataP = new byte [s];
        if (bmDataP == NULL) {
	        rc = 1;
	        goto abort;
	        }
        nFormat = 0;
        fseek (fTextures, offset, SEEK_SET);
        if (pigTexD2.flags & 0x08) {
	        fread (&nSize, 1, sizeof (int), fTextures);
	        fread (xsize, pigTexD2.ysize, 1, fTextures);
	        nLine = 0;
	        for (y = h - 1; y >= 0; y--) {
		        fread (line, xsize[nLine++], 1, fTextures);
		        line_ptr = line;
			        for (x = 0; x < w;) {
			        byteVal = *line_ptr++;
			        if ((byteVal & 0xe0) == 0xe0) {
				        runcount = byteVal & 0x1f;
				        runvalue = *line_ptr++;
				        for (j = 0; j < runcount; j++) {
					        if (x < w) {
						        bmDataP [y * w + x] = runvalue;
						        x++;
						        }
					        }
				        }
			        else {
				        bmDataP [y * w + x] = byteVal;
				        x++;
				        }
			        }
		        }
	        }
        else {
	        for (y=h-1;y>=0;y--) {
        #if 1
		        fread (bmDataP + y * w, w, 1, fTextures);
        #else
		        fread (line,w,1,fTextures);
		        line_ptr = line;
		        for (x=0;x<w;x++) {
			        byteVal = *line_ptr++;
			        bmDataP[y*w+x] = byteVal;
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
