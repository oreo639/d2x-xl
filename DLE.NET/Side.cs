using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class Side : GameItem
    {
        public short m_nChild;
        public ushort m_nWall;		        // (was short) Index into Walls array, which wall (probably door) is on this side 
        public ushort m_nBaseTex;	        // Index into array of textures specified in bitmaps.bin 
        public ushort m_nOvlTex;		    // Index, as above, texture which gets overlaid on nBaseTex 
        public UVL[] uvls = new UVL [4];    // CUVL coordinates at each point 

        // ------------------------------------------------------------------------

        void Setup ()
        {
            m_nWall = DLE.theMine.NO_WALL ();
            m_nBaseTex =
            m_nOvlTex = 0;
            for (int i = 0; i < 4; i++)
                uvls [i].l = (ushort)GameMine.DEFAULT_LIGHTING;
        }

        // ------------------------------------------------------------------------

        public override void Read (BinaryReader fp, int version = 0, bool bFlag = false)
        {
            m_nWall = fp.ReadUInt16 ();
            m_nBaseTex = fp.ReadUInt16 ();
            if ((m_nBaseTex & 0x8000) == 0)
                m_nOvlTex = 0;
            else
            {
                m_nOvlTex = fp.ReadUInt16 ();
                if ((m_nOvlTex & 0x1FFF) == 0)
                    m_nOvlTex = 0;
            }
            m_nBaseTex &= 0x1FFF;
            for (int i = 0; i < uvls.Length; i++)
                uvls [i].Read (fp);
        }

        // ------------------------------------------------------------------------

        public override void Write (BinaryWriter fp, int version = 0, bool bFlag = false)
        {
            if (m_nOvlTex == 0)
                fp.Write (m_nBaseTex);
            else
            {
                fp.Write (m_nBaseTex | (ushort) 0x8000);
                fp.Write (m_nOvlTex);
            }
            for (int i = 0; i < uvls.Length; i++)
                uvls [i].Write (fp);
        }

        // ------------------------------------------------------------------------

        public override void Clear ()
        {
            m_nChild = 0;
            m_nWall = 0;
            m_nBaseTex = 0;
            m_nOvlTex = 0;
            for (int i = 0; i < uvls.Length; i++)
                uvls [i].Clear ();
        }

        // ------------------------------------------------------------------------

        int Read (BinaryReader fp, bool bTextured)
        {
            if (bTextured) 
            {
                m_nBaseTex = fp.ReadUInt16 ();
	            if ((m_nBaseTex & 0x8000) != 0) 
                {
                    m_nOvlTex = fp.ReadUInt16 ();
		            if ((m_nOvlTex & 0x1FFF) == 0)
			            m_nOvlTex = 0;
		        }
	            else
		            m_nOvlTex = 0;
	            m_nBaseTex &= 0x1FFF;
	            for (int i = 0; i < 4; i++)
		            uvls [i].Read (fp);
	        }
            else 
            {
	            m_nBaseTex = 0;
	            m_nOvlTex = 0;
	            for (int i = 0; i < 4; i++)
		            uvls [i].Clear ();
	        }
            return 1;
        }

        // ------------------------------------------------------------------------

        void Write (BinaryWriter fp)
        {
            if (m_nOvlTex == 0)
	            fp.Write (m_nBaseTex);
            else 
            {
	            fp.Write ((ushort) (m_nBaseTex | 0x8000));
	            fp.Write (m_nOvlTex);
	        }
            for (int i = 0; i < 4; i++)
	            uvls [i].Write (fp);
        }

        // ------------------------------------------------------------------------

        bool SetTexture (ushort nNewBaseTex, ushort nNewOvlTex)
        {
	        bool bChange = false;

        if (m_nOvlTex == m_nBaseTex)
           m_nOvlTex = 0; 
        if ((nNewBaseTex >= 0) && (nNewBaseTex != m_nBaseTex)) {
	        m_nBaseTex = nNewBaseTex; 
	        if (nNewBaseTex == (nNewOvlTex & 0x3fff)) {
		        m_nOvlTex = 0; 
		        }
	        bChange = true; 
	        }
        if (nNewOvlTex >= 0) {
	        if (nNewOvlTex == m_nBaseTex)
		        m_nOvlTex = 0; 
	        else if (nNewOvlTex != 0) {
		        m_nOvlTex &= (ushort) 0xD000;	//preserve light settings
		        m_nOvlTex |= nNewOvlTex; 
		        }
	        else
		        m_nOvlTex = 0; 
	        bChange = true; 
	        }
        //if (bChange)
        //    LoadTextures ();
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
