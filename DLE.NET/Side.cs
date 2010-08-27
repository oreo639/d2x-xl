using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class Side : GameItem
    {
	    ushort	nWall;		// (was short) Index into Walls array, which wall (probably door) is on this side 
	    short	nBaseTex;	    // Index into array of textures specified in bitmaps.bin 
	    short	nOvlTex;		// Index, as above, texture which gets overlaid on nBaseTex 
	    UVL[]	uvls = new UVL[4];     // CUVL coordinates at each point 

    	void Setup ()
        {
        }

	    void LoadTextures ()
        {
        }

	    bool SetTexture (short nBaseTex, short nOvlTex)
        {
            return false;
        }
	
        Wall Wall ()
        {
            return null;
        }

        public override int Read (Stream fp, int version = 0, bool bFlag = false)
        {
            return 1;
        }

        public override void Write (Stream fp, int version = 0, bool bFlag = false)
        {
        }

        public override void Clear ()
        {
        }
    }
}
