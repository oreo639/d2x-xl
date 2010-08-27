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
int Read (FILE* fp, bool bTextured)
{
if (bTextured) {
	m_info.nBaseTex = ReadInt16 (fp);
	if (m_info.nBaseTex & 0x8000) {
		m_info.nOvlTex = ReadInt16 (fp);
		if ((m_info.nOvlTex & 0x1FFF) == 0)
			m_info.nOvlTex = 0;
		}
	else
		m_info.nOvlTex = 0;
	m_info.nBaseTex &= 0x1FFF;
	for (int i = 0; i < 4; i++)
		m_info.uvls [i].Read (fp);
	}
else {
	m_info.nBaseTex = 0;
	m_info.nOvlTex = 0;
	for (int i = 0; i < 4; i++)
		m_info.uvls [i].Clear ();
	}
return 1;
}

// ------------------------------------------------------------------------

void Write (FILE* fp)
{
if (m_info.nOvlTex == 0)
	WriteInt16 (m_info.nBaseTex, fp);
else {
	WriteInt16 (m_info.nBaseTex | 0x8000, fp);
	WriteInt16 (m_info.nOvlTex, fp);
	}
for (int i = 0; i < 4; i++)
	m_info.uvls [i].Write (fp);
}

// ------------------------------------------------------------------------

void Setup ()
{
m_info.nWall = NO_WALL; 
m_info.nBaseTex =
m_info.nOvlTex = 0; 
for (int i = 0; i < 4; i++)
	m_info.uvls [i].l = (ushort) DEFAULT_LIGHTING; 
}

// ------------------------------------------------------------------------ 

void LoadTextures ()
{
theMine->Textures (theMine->m_fileType, m_info.nBaseTex)->Read (m_info.nBaseTex);
if ((m_info.nOvlTex & 0x3fff) > 0)
	theMine->Textures (theMine->m_fileType, m_info.nOvlTex & 0x3fff)->Read (m_info.nOvlTex & 0x3fff);
}

// ------------------------------------------------------------------------

bool SetTexture (short nBaseTex, short nOvlTex)
{
	bool bChange = false;

if (nOvlTex == nBaseTex)
   nOvlTex = 0; 
if ((nBaseTex >= 0) && (nBaseTex != m_info.nBaseTex)) {
	m_info.nBaseTex = nBaseTex; 
	if (nBaseTex == (m_info.nOvlTex & 0x3fff)) {
		m_info.nOvlTex = 0; 
		}
	bChange = true; 
	}
if (nOvlTex >= 0) {
	if (nOvlTex == m_info.nBaseTex)
		nOvlTex = 0; 
	if (nOvlTex) {
		m_info.nOvlTex &= ~(0x3fff);	//preserve light settings
		m_info.nOvlTex |= nOvlTex; 
		}
	else
		m_info.nOvlTex = 0; 
	bChange = true; 
	}
if (bChange)
	LoadTextures ();
return bChange;
}

// ------------------------------------------------------------------------

CWall* Wall ()
{ 
return (m_info.nWall == NO_WALL) ? NULL : theMine->Walls (m_info.nWall); 
}

    }
}
