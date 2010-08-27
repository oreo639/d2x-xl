using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public class TextureManager
    {
        public const int MAX_D1_TEXTURES = 584;
        public const int MAX_D2_TEXTURES = 910;

        public int MAX_TEXTURES () { return DLE.IsD1File () ? MAX_D1_TEXTURES : MAX_D2_TEXTURES; }

        byte [] bmBuffer = new byte [512 * 512 * 4 * 32];
        Texture [][] textures = new Texture [2][] { new Texture [MAX_D1_TEXTURES], new Texture [MAX_D2_TEXTURES] };

        //------------------------------------------------------------------------------

        public void LoadTextures (ushort nBaseTex, ushort nOvlTex)
        {
            textures [(int) DLE.theMine.FileType][nBaseTex].Load (nBaseTex);
            if ((nOvlTex & 0x1FFF) != 0)
                textures [(int)DLE.theMine.FileType] [nOvlTex].Load (nBaseTex);
        }
    }
}
