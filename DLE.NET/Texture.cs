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
	        byte				xsize[200];
	        byte				line[320],*line_ptr;
	        byte				j;
	        PigTexture		ptexture;
	        PigTextureD2 d2_ptexture;
	        PigHeaderD1		file_header;
	        PigHeaderD2	d2FileHeader;
	        int				offset,dataOffset;
	        short				x,y,w,h,s;
	        byte				byteVal, runcount, runvalue;
	        int				nSize;
	        short				linenum;
	        HRSRC				hFind = 0;
	        HGLOBAL			hGlobal = 0;
	        short				*textureTable;
	        int				rc;
	        short				*texture_ptr;
	        FILE				*fTextures = NULL;
	        char				path [256];
	
        if (m_info.bModified)
	        return 0;
        strcpy_s (path, sizeof (path), (theApp.IsD1File ()) ? descent_path : descent2_path);
        if (!strstr (path, ".pig"))
	        strcat_s (path, sizeof (path), "groupa.pig");

        HINSTANCE hInst = AfxGetInstanceHandle ();

        // do a range check on the texture number
        if ((index > ((theApp.IsD1File ()) ? MAX_D1_TEXTURES : MAX_D2_TEXTURES)) || (index < 0)) {
        DEBUGMSG (" Reading texture: Texture # out of range.");
        rc = 1;
        goto abort;
        }

        // get pointer to texture table from resource fTextures
        hFind = (theApp.IsD1File ()) ?
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
        if (theApp.IsD2File ())
	        fread (&d2FileHeader, sizeof (d2FileHeader), 1, fTextures);
        else
	        fread (&file_header, sizeof (file_header), 1, fTextures);

        // read texture header
        if (theApp.IsD2File ()) {
	        offset = sizeof (PigHeaderD2) + dataOffset +
				        (fix) (textureTable[index]-1) * sizeof (PigTextureD2);
	        fseek (fTextures, offset, SEEK_SET);
	        fread (&d2_ptexture, sizeof (PigTextureD2), 1, fTextures);
	        w = d2_ptexture.xsize + ((d2_ptexture.wh_extra & 0xF) << 8);
	        h = d2_ptexture.ysize + ((d2_ptexture.wh_extra & 0xF0) << 4);
	        }
        else {
	        offset = sizeof (PigHeaderD1) + dataOffset + (fix) (textureTable [index] - 1) * sizeof (ptexture);
	        fseek (fTextures, offset, SEEK_SET);
	        fread (&ptexture, sizeof (PigTexture), 1, fTextures);
	        ptexture.name [sizeof (ptexture.name) - 1] = '\0';
	        // copy d1 texture into d2 texture struct
	        strncpy_s (d2_ptexture.name, sizeof (d2_ptexture.name), ptexture.name, sizeof (ptexture.name));
	        d2_ptexture.dflags = ptexture.dflags;
	        d2_ptexture.xsize = ptexture.xsize;
	        d2_ptexture.ysize = ptexture.ysize;
	        d2_ptexture.flags = ptexture.flags;
	        d2_ptexture.avg_color = ptexture.avg_color;
	        d2_ptexture.offset = ptexture.offset;
	        d2_ptexture.wh_extra = (ptexture.dflags == 128) ? 1 : 0;
	        w = h = 64;
	        }
        s = w * h;

        // seek to data
        if (theApp.IsD2File ()) {
	        offset = sizeof (PigHeaderD2) 
				        + dataOffset
				        + d2FileHeader.nTextures * sizeof (PigTextureD2)
				        + d2_ptexture.offset;
	        }
        else {
	        offset = sizeof (PigHeaderD1) + dataOffset
				        + file_header.nTextures * sizeof (PigTexture)
				        + file_header.nSounds   * sizeof (PIG_SOUND)
				        + d2_ptexture.offset;
	        }

        // allocate data if necessary
        if (m_info.bmDataP && ((m_info.width != w) || (m_info.height != h)))
	        Release ();
        if (m_info.bmDataP == NULL)
	        m_info.bmDataP = new byte [s];
        if (m_info.bmDataP == NULL) {
	        rc = 1;
	        goto abort;
	        }
        m_info.nFormat = 0;
        fseek (fTextures, offset, SEEK_SET);
        if (d2_ptexture.flags & 0x08) {
	        fread (&nSize, 1, sizeof (int), fTextures);
	        fread (xsize, d2_ptexture.ysize, 1, fTextures);
	        linenum = 0;
	        for (y = h - 1; y >= 0; y--) {
		        fread (line, xsize[linenum++], 1, fTextures);
		        line_ptr = line;
			        for (x = 0; x < w;) {
			        byteVal = *line_ptr++;
			        if ((byteVal & 0xe0) == 0xe0) {
				        runcount = byteVal & 0x1f;
				        runvalue = *line_ptr++;
				        for (j = 0; j < runcount; j++) {
					        if (x < w) {
						        m_info.bmDataP [y * w + x] = runvalue;
						        x++;
						        }
					        }
				        }
			        else {
				        m_info.bmDataP [y * w + x] = byteVal;
				        x++;
				        }
			        }
		        }
	        }
        else {
	        for (y=h-1;y>=0;y--) {
        #if 1
		        fread (m_info.bmDataP + y * w, w, 1, fTextures);
        #else
		        fread (line,w,1,fTextures);
		        line_ptr = line;
		        for (x=0;x<w;x++) {
			        byteVal = *line_ptr++;
			        m_info.bmDataP[y*w+x] = byteVal;
			        }
        #endif
		        }
	        }
        fclose (fTextures);
        m_info.width = w;
        m_info.height = h;
        m_info.size = s;
        m_info.bValid = 1;
        return (0);

        abort:
        // free handle
        if (hGlobal) FreeResource (hGlobal);
        return (rc);
        }

    }
}
