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

        byte [] bmBuffer = new byte [512 * 512 * 4 * 32];
        Texture [][] textures = new Texture [2][] { new Texture [MAX_D1_TEXTURES], new Texture [MAX_D2_TEXTURES] };
    }
}
