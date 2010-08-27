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

        // ------------------------------------------------------------------------

        int Read (BinaryReader fp, bool bTextured)
        {
            if (bTextured) 
            {
                nBaseTex = fp.ReadUInt16 ();
	            if ((nBaseTex & 0x8000) != 0) 
                {
                    nOvlTex = fp.ReadUInt16 ();
		            if ((nOvlTex & 0x1FFF) == 0)
			            nOvlTex = 0;
		        }
	            else
		            nOvlTex = 0;
	            nBaseTex &= 0x1FFF;
	            for (int i = 0; i < 4; i++)
		            uvls [i].Read (fp);
	        }
            else 
            {
	            nBaseTex = 0;
	            nOvlTex = 0;
	            for (int i = 0; i < 4; i++)
		            uvls [i].Clear ();
	        }
            return 1;
        }

        // ------------------------------------------------------------------------

        void Write (BinaryWriter fp)
        {
            if (nOvlTex == 0)
	            fp.Write (nBaseTex);
            else 
            {
	            fp.Write ((ushort) (nBaseTex | 0x8000));
	            fp.Write (nOvlTex);
	        }
            for (int i = 0; i < 4; i++)
	            uvls [i].Write (fp);
        }

        // ------------------------------------------------------------------------

        void Setup ()
        {
        nWall = DLE.theMine.NO_WALL (); 
        nBaseTex =
        nOvlTex = 0; 
        for (int i = 0; i < 4; i++)
	        uvls [i].l = (ushort) GameMine.DEFAULT_LIGHTING; 
        }

        // ------------------------------------------------------------------------ 

        void LoadTextures ()
        {
        DLE.textureManager.LoadTextures (nBaseTex, nOvlTex);
        }

        // ------------------------------------------------------------------------

        bool SetTexture (ushort nNewBaseTex, ushort nNewOvlTex)
        {
	        bool bChange = false;

        if (nOvlTex == nBaseTex)
           nOvlTex = 0; 
        if ((nNewBaseTex >= 0) && (nNewBaseTex != nBaseTex)) {
	        nBaseTex = nNewBaseTex; 
	        if (nNewBaseTex == (nNewOvlTex & 0x3fff)) {
		        nOvlTex = 0; 
		        }
	        bChange = true; 
	        }
        if (nNewOvlTex >= 0) {
	        if (nNewOvlTex == nBaseTex)
		        nOvlTex = 0; 
	        else if (nNewOvlTex != 0) {
		        nOvlTex &= (ushort) 0xD000;	//preserve light settings
		        nOvlTex |= nNewOvlTex; 
		        }
	        else
		        nOvlTex = 0; 
	        bChange = true; 
	        }
        if (bChange)
	        LoadTextures ();
        return bChange;
        }

        // ------------------------------------------------------------------------

        Wall Wall (ushort nWall)
        { 
        return (nWall == DLE.theMine.NO_WALL ()) ? null : DLE.theMine.walls [nWall]; 
        }

        // ------------------------------------------------------------------------

    }
}
