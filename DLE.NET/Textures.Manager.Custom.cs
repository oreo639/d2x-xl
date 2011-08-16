using System;
using System.IO;
using System.Runtime.InteropServices;

namespace DLE.NET
{
    public partial class TextureManager
    {

    //------------------------------------------------------------------------------

    int ReadPog (BinaryReader fp, long nFileSize) 
    {
        PigHeader pigFileInfo = new PigHeader (1);
        PigTexture pigTexInfo = new PigTexture (1);

        ushort [] xlatTbl = null;
        uint nSize;
        uint offset, hdrOffset, bmpOffset, hdrSize, xlatSize;
        short nTexture;
        ushort nUnknownTextures, nMissingTextures;
        bool bExtraTexture;
        Texture tex;
        int fileType = DLE.FileType;

        // make sure this is descent 2 fp
        if (DLE.IsD1File)
        {
            DLE.InfoMsg (" Descent 1 does not support custom textures.");
            return 1;
        }

        uint startOffset = (uint)fp.BaseStream.Position;
        // read file header
        pigFileInfo.Read (fp);
        if (pigFileInfo.nId != 0x474f5044L)
        {  // 'DPOG'
            DLE.ErrorMsg (@"Invalid pog file - reading from hog file");
            return 1;
        }
        //Release ();
        DLE.DebugMsg (string.Format (@"Pog manager: Reading {0:d} custom textures", pigFileInfo.nTextures));
        xlatTbl = new ushort [pigFileInfo.nTextures];
        if (xlatTbl == null)
            return 5;
        xlatSize = pigFileInfo.nTextures * sizeof (ushort);
        offset = (uint)fp.BaseStream.Position;
        for (int i = 0; i < xlatSize; i++)
            xlatTbl [i] = fp.ReadUInt16 ();
        // loop for each custom texture
        nUnknownTextures = 0;
        nMissingTextures = 0;
        hdrOffset = offset + xlatSize;
        hdrSize = xlatSize + pigFileInfo.nTextures * pigTexInfo.Size;
        bmpOffset = offset + hdrSize;

        //DLE.MainFrame.InitProgress (pigFileInfo.nTextures);

        for (int i = 0; i < pigFileInfo.nTextures; i++)
        {
            //DLE.MainFrame.Progress.StepIt ();
            // read texture index
            ushort nIndex = xlatTbl [i];
            // look it up in the list of textures
            for (nTexture = 0; nTexture < MAX_TEXTURES_D2; nTexture++)
                if (m_index [1] [nTexture] == nIndex)
                    break;
            bExtraTexture = (nTexture >= MAX_TEXTURES_D2);
            // get texture data offset from texture header
            fp.BaseStream.Position = (long)(hdrOffset + i * pigTexInfo.Size);
            pigTexInfo.Read (fp);
            nSize = (uint)pigTexInfo.width * (uint)pigTexInfo.height;
            if ((long)(hdrSize + pigTexInfo.offset + nSize) >= nFileSize)
            {
                nMissingTextures++;
                continue;
            }
            if (bExtraTexture)
            {
                tex = AddExtra (nIndex);
                if (tex == null)
                {
                    nUnknownTextures++;
                    continue;
                }
                nTexture = 0;
            }
            else
            {
                tex = Textures [nTexture];
                tex.Release ();
            }
            //if (!(tex.m_data = new CBGRA [nSize]))
            //	continue;
            tex.m_nFormat = (byte)(((pigTexInfo.flags & 0x80) != 0) ? 1 : 0);
            //tex.m_width = pigTexInfo.width;
            //tex.m_height = pigTexInfo.height;
            if (!tex.Allocate (nSize))
                continue;
            //tex.m_size = nSize;
            //tex.m_bValid = 1;
            fp.BaseStream.Position = (long)(bmpOffset + pigTexInfo.offset);
            tex.Load (fp, pigTexInfo);
            if (!bExtraTexture)
                tex.m_bCustom = true;
        }
        if (nUnknownTextures > 0)
        {
            DLE.DebugMsg (string.Format (@"Pog manager: {0:d} unknown textures found.", nUnknownTextures));
        }
        if (nMissingTextures > 0)
        {
            DLE.DebugMsg (string.Format (@"Pog manager: {0:d} textures missing (Pog file may be damaged).", nMissingTextures));
        }

        //DLE.MainFrame.Progress.DestroyWindow ();
        return 0;
    }

    //------------------------------------------------------------------------------

    uint WriteCustomTextureHeader (BinaryWriter fp, Texture tex, int nId = -1, uint nOffset = 0)
    {
        PigTexture pigTexInfo = new PigTexture (1);
        uint pos = 0xFFFFFFFF;

        if (nId >= 0)
        {
            tex.m_id = nId;
            tex.m_offset = nOffset;
        }
        else
        {
            pos = (uint)fp.BaseStream.Position;
            nId = tex.m_id;
            fp.BaseStream.Position = (long)(nOffset = tex.m_offset);
        }

        pigTexInfo.name = string.Format (@"POG{0:0000}", nId);
        pigTexInfo.Setup (1, (ushort)tex.m_width, (ushort)tex.m_height, (byte)((tex.m_nFormat != 0) ? 0x80 : 0), nOffset);

        // check for transparency and super transparency
        if (tex.m_nFormat == 0)
            if (tex.Buffer != null)
            {
                for (uint j = 0, h = tex.Size; j < h; j++)
                {
                    if (tex.Index [j] == 255)
                        pigTexInfo.flags |= (byte)global::DLE.NET.Texture.Flags.TRANSPARENT;
                    if (tex.Index [j] == 254)
                        pigTexInfo.flags |= (byte)global::DLE.NET.Texture.Flags.SUPER_TRANSPARENT;
                }
            }
        pigTexInfo.Write (fp);
        if (pos != 0xFFFFFFFF)
            fp.BaseStream.Position = (long)pos;
        return (uint)(nOffset + ((tex.m_nFormat != 0) ? tex.Size * Marshal.SizeOf (tex.Buffer [0]) : tex.Size));
    }

    //------------------------------------------------------------------------------

    int WriteCustomTexture (BinaryWriter fp, Texture tex)
    {
        if (tex.m_nFormat != 0)
        {
            uint bufP = tex.m_width * (tex.m_height - 1);
            for (uint i = tex.m_height; i > 0; i--)
            {
                for (uint j = tex.m_width; j > 0; j--, bufP++)
                {
                    BGRA color = tex.Buffer [bufP];
                    fp.Write (color.r);
                    fp.Write (color.g);
                    fp.Write (color.b);
                    fp.Write (color.a);
                }
                bufP -= 2 * tex.m_width;
            }
        }
        else
        {
            uint w = tex.m_width;
            uint h = tex.m_height;
            byte [] bmIndex = new byte [w * h];
            if (bmIndex == null)
            { // write as TGA
                tex.m_nFormat = 1;
                return (WriteCustomTexture (fp, tex) == 0) ? 0 : -1;
            }
            tex.ComputeIndex (bmIndex);
            ;
            for (uint indexP = w * h /*point to last row of bitmap*/; h > 0; h--)
            {
                indexP -= w;
                for (uint j = w; j > 0; j--, indexP++)
                    fp.Write (bmIndex [indexP]);
            }
        }
        return 1;
    }

    //------------------------------------------------------------------------------

    public int CreatePog (BinaryWriter fp) 
    {
	    PigHeader		pigFileInfo  = new PigHeader (1);
	    uint			textureCount = 0, nOffset = 0;
	    int				nVersion = DLE.FileType;
	    int				nId, i, h = DLE.Textures.MaxTextures;
	    ExtraTexture	extraTex;
	    Texture		    tex;

    if (DLE.IsD1File) {
	    DLE.ErrorMsg (@"Descent 1 does not support custom textures.");
	    return 1;
	    }

    textureCount = m_nTextures [1];

    string message = string.Format (DLE.StartFolder + "\\dle_temp.pog");

    // write file  header
    pigFileInfo.nId = (uint) 0x474f5044L; /* 'DPOG' */
    pigFileInfo.nVersion = 1;
    pigFileInfo.nTextures = 0;
    for (i = 0; i < h; i++)
	    if (Textures [i].m_bCustom)
		    pigFileInfo.nTextures++;
    for (extraTex = m_extra; extraTex; extraTex = extraTex.m_next)
	    pigFileInfo.nTextures++;
    pigFileInfo.Write (fp);

    // write list of textures
    for (i = 0; i < h; i++)
    {
        if (Textures [i].m_bCustom)
            fp.Write (m_index [1] [i]);
    }

    for (extraTex = m_extra; extraTex; extraTex = extraTex.m_next)
	    fp.Write (extraTex.m_index);

    // write texture headers
    nId = 0;
    for (i = 0; i < h; i++)
    {
        if (Textures [i].m_bCustom)
            nOffset = WriteCustomTextureHeader (fp, tex, nId++, nOffset);
        for (extraTex = m_extra; extraTex; extraTex = extraTex.m_next)
            nOffset = WriteCustomTextureHeader (fp, extraTex, nId++, nOffset);
    }


    DLE.DebugMsg (string.Format (@"Pog manager: Saving {0} custom textures", pigFileInfo.nTextures));

    for (i = 0; i < h; i++)
    {
        if (Textures [i].m_bCustom)
        {
            if (0 > WriteCustomTexture (fp, tex))
                WriteCustomTextureHeader (fp, tex); // need to rewrite to reflect changed texture type in header data
        }
    }

    for (extraTex = m_extra; extraTex; extraTex = extraTex.m_next)
	    if (0 > WriteCustomTexture (fp, extraTex))
		    WriteCustomTextureHeader (fp, tex); // need to rewrite to reflect changed texture type in header data

    return 0;
    }

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------

    }
}
