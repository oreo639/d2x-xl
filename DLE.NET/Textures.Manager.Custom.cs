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
	    PigHeader		pigFileInfo = new PigHeader (1);
	    PigTexture		pigTexInfo = new PigTexture (1);

	    ushort[]		xlatTbl = null;
	    uint			nSize;
	    uint			offset, hdrOffset, bmpOffset, hdrSize, xlatSize;
	    short			nTexture;
	    ushort			nUnknownTextures, nMissingTextures;
	    bool			bExtraTexture;
	    Texture	    	tex;
	    int				fileType = DLE.FileType;

    // make sure this is descent 2 fp
    if (DLE.IsD1File) {
	    DLE.InfoMsg (" Descent 1 does not support custom textures.");
	    return 1;
	    }

    uint startOffset = (uint)fp.BaseStream.Position;
    // read file header
    pigFileInfo.Read (fp);
    if (pigFileInfo.nId != 0x474f5044L) {  // 'DPOG'
	    DLE.ErrorMsg (@"Invalid pog file - reading from hog file");
	    return 1;
	    }
    //Release ();
    DLE.DebugMsg (string.Format (@"Pog manager: Reading {0:d} custom textures", pigFileInfo.nTextures));
    xlatTbl = new ushort [pigFileInfo.nTextures];
    if (xlatTbl == null)
	    return 5;
    xlatSize = pigFileInfo.nTextures * sizeof (ushort);
    offset = (uint) fp.BaseStream.Position;
    for (int i = 0; i < xlatSize; i++)
        xlatTbl [i] = fp.ReadUInt16 ();
    // loop for each custom texture
    nUnknownTextures = 0;
    nMissingTextures = 0;
    hdrOffset = offset + xlatSize;
    hdrSize = xlatSize + pigFileInfo.nTextures * pigTexInfo.Size;
    bmpOffset = offset + hdrSize;

    //DLE.MainFrame.InitProgress (pigFileInfo.nTextures);

    for (int i = 0; i < pigFileInfo.nTextures; i++) {
	    //DLE.MainFrame.Progress.StepIt ();
	    // read texture index
	    ushort nIndex = xlatTbl [i];
	    // look it up in the list of textures
	    for (nTexture = 0; nTexture < MAX_TEXTURES_D2; nTexture++)
		    if (m_index [1][nTexture] == nIndex)
			    break;
	    bExtraTexture = (nTexture >= MAX_TEXTURES_D2);
	    // get texture data offset from texture header
	    fp.BaseStream.Position = (long) (hdrOffset + i * pigTexInfo.Size);
	    pigTexInfo.Read (fp);
	    nSize = (uint) pigTexInfo.width * (uint) pigTexInfo.height;
	    if ((long) (hdrSize + pigTexInfo.offset + nSize) >= nFileSize) {
		    nMissingTextures++;
		    continue;
		    }
	    if (bExtraTexture) {
			    tex = AddExtra (nIndex);
		    if (tex == null) {
			    nUnknownTextures++;
			    continue;
			    }
		    nTexture = 0;
		    }
	    else {
		    tex = Textures [nTexture];
		    tex.Release ();
		    }
	    //if (!(tex.m_data = new CBGRA [nSize]))
	    //	continue;
	    tex.m_nFormat = (byte) (((pigTexInfo.flags & 0x80) != 0) ? 1 : 0);
	    //tex.m_width = pigTexInfo.width;
	    //tex.m_height = pigTexInfo.height;
	    if (!tex.Allocate (nSize)) 
		    continue;
	    //tex.m_size = nSize;
	    //tex.m_bValid = 1;
	    fp.BaseStream.Position = (long) (bmpOffset + pigTexInfo.offset);
	    tex.Load (fp, pigTexInfo);
	    if (!bExtraTexture)
		    tex.m_bCustom = true;
	    }
    if (nUnknownTextures > 0) {
	    DLE.DebugMsg (string.Format (@"Pog manager: {0:d} unknown textures found.", nUnknownTextures));
	    }
    if (nMissingTextures > 0) {
	    DLE.DebugMsg (string.Format (@"Pog manager: {0:d} textures missing (Pog file may be damaged).", nMissingTextures));
	    }

    //DLE.MainFrame.Progress.DestroyWindow ();
    return 0;
    }

    //------------------------------------------------------------------------------

    uint WriteCustomTextureHeader (BinaryWriter fp, Texture tex, int nId, uint nOffset)
    {
	    PigTexture pigTexInfo = new PigTexture (1);
	    uint pos = 0xFFFFFFFF;

    if (nId >= 0) {
	    tex.m_id = nId;
	    tex.m_offset = nOffset;
	    }
    else {
	    pos = (uint) fp.BaseStream.Position;
	    nId = tex.m_id;
	    fp.BaseStream.Position = (long) (nOffset = tex.m_offset);
	    }

    pigTexInfo.name = string.Format (@"POG{0:0000}", nId);
    pigTexInfo.Setup (1, (ushort)tex.m_width, (ushort)tex.m_height, (byte) ((tex.m_nFormat != 0) ? 0x80 : 0), nOffset);

    // check for transparency and super transparency
    if (tex.m_nFormat == 0)
	    if (tex.Buffer != null) 
        {
		    for (uint j = 0, h = tex.Size; j < h; j++) 
            {
			    if (tex.Index [j] == 255) 
				    pigTexInfo.flags |= (byte) global::DLE.NET.Texture.Flags.TRANSPARENT;
                if (tex.Index [j] == 254)
                    pigTexInfo.flags |= (byte)global::DLE.NET.Texture.Flags.SUPER_TRANSPARENT;
			}
	    }
    pigTexInfo.Write (fp);
    if (pos != 0xFFFFFFFF)
	    fp.BaseStream.Position = (long) pos;
    return (uint) (nOffset + ((tex.m_nFormat != 0) ? tex.Size * Marshal.SizeOf (tex.Buffer [0]) : tex.Size));
    }

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------

    }
}
