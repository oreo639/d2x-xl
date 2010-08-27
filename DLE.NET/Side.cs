using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class Side : GameItem
    {
            public short nChild;
            public ushort nWall;		// (was short) Index into Walls array, which wall (probably door) is on this side 
            public ushort nBaseTex;	    // Index into array of textures specified in bitmaps.bin 
            public ushort nOvlTex;		// Index, as above, texture which gets overlaid on nBaseTex 
            public UVL[] uvls = new UVL [4];  // CUVL coordinates at each point 

        public void Setup ()
        {
        }

        public void LoadTextures ()
        {
        }

        public bool SetTexture (short nBaseTex, short nOvlTex)
        {
            return false;
        }

        public Wall Wall ()
        {
            return null;
        }

        public override void Read (BinaryReader fp, int version = 0, bool bFlag = false)
        {
            nWall = fp.ReadUInt16 ();
            nBaseTex = fp.ReadUInt16 ();
            if ((nBaseTex & 0x8000) == 0)
                nOvlTex = 0;
            else
            {
                nOvlTex = fp.ReadUInt16 ();
                if ((nOvlTex & 0x1FFF) == 0)
                    nOvlTex = 0;
            }
            nBaseTex &= 0x1FFF;
            for (int i = 0; i < uvls.Length; i++)
                uvls [i].Read (fp);
        }

        public override void Write (BinaryWriter fp, int version = 0, bool bFlag = false)
        {
            if (nOvlTex == 0)
                fp.Write (nBaseTex);
            else
            {
                fp.Write (nBaseTex | (ushort) 0x8000);
                fp.Write (nOvlTex);
            }
            for (int i = 0; i < uvls.Length; i++)
                uvls [i].Write (fp);
        }

        public override void Clear ()
        {
            nChild = 0;
            nWall = 0;
            nBaseTex = 0;
            nOvlTex = 0;
            for (int i = 0; i < uvls.Length; i++)
                uvls [i].Clear ();
        }
    }
}
