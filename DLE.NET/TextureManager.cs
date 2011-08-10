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

    public class TextureManager
    {
        public const int MAX_TEXTURES_D1 = 584;
        public const int MAX_TEXTURES_D2 = 910;

        public int MaxTextures { get { return DLE.IsD1File ? MAX_TEXTURES_D1 : MAX_TEXTURES_D2; } }
        public int Version { get { return DLE.IsD1File ? 0 : 1; } }
        public long Offset { get { return m_nOffsets [Version]; } }

        public Texture [] Textures { get { return m_textures [Version]; } }
        public PigTexture [] Infos { get { return m_info [Version]; } }
        public String [] Names { get { return m_names [Version]; } }

        public Texture [][] m_textures = new Texture [2][] { new Texture [MAX_TEXTURES_D1], new Texture [MAX_TEXTURES_D2] };
        public ushort [][] m_index = new ushort [2][];
        public String [][] m_names = new String [2][];
        public PigHeader [] m_header;
        public PigTexture [][] m_info = new PigTexture [2][] { null, null };
        public String [] m_pigFiles = new String [2] { "", "" };
        public uint[] m_nTextures = new uint [2] { 0, 0 };

        long [] m_nOffsets = new long [2] { 0, 0 };
        byte [] m_bmBuffer = new byte [512 * 512 * 4 * 32];
        List<Texture> m_extra = new List<Texture> ();
        String m_paletteName = "";

        //------------------------------------------------------------------------------

        public TextureManager ()
        {
            LoadIndex (0);
            LoadIndex (1);
        }

        //------------------------------------------------------------------------------

        public void Setup ()
        {
            Create (0);
            Create (1);
        }

        //------------------------------------------------------------------------------

        public void Create (int nVersion)
        {
            m_pigFiles [nVersion] = "";
            m_names [nVersion] = null;
            LoadNames (nVersion);
            m_header [nVersion] = new PigHeader (nVersion);
            LoadIndex (nVersion);
            m_textures [nVersion] = new Texture [MaxTextures];
        }

        //------------------------------------------------------------------------------

        public void Destroy (int nVersion)
        {
        }

        //------------------------------------------------------------------------------

        public Texture Texture (int nTexture)
        {
            return Textures [nTexture];
        }

        //------------------------------------------------------------------------------

        public String Name (int nTexture)
        {
            return Names [nTexture];
        }

        //------------------------------------------------------------------------------

        public PigTexture Info (int nTexture)
        {
            return Infos [nTexture];
        }

        //------------------------------------------------------------------------

        void Setup (int nVersion)
        {
            m_pigFiles [nVersion] = "";
            m_header [nVersion] = new PigHeader (nVersion);
            LoadNames (nVersion);
            LoadIndex (nVersion);
        }

        //------------------------------------------------------------------------------

        void LoadNames (int nVersion)
        {
            using (MemoryStream resource = new MemoryStream ((nVersion == 0) ? Properties.Resources.tnames : Properties.Resources.tnames2))
            {
                using (StreamReader reader = new StreamReader (resource))
                {
                    int j = MaxTextures;

                    m_names [nVersion] = new String [j];
                    for (int i = 0; i < j; i++)
                        m_names [nVersion] [i] = reader.ReadLine ();
                }
	        }
        }

        //------------------------------------------------------------------------------

        void LoadIndex (int nVersion)
        {
            using (MemoryStream resource = new MemoryStream ((nVersion == 0) ? Properties.Resources.texture : Properties.Resources.texture2))
            {
                using (BinaryReader reader = new BinaryReader (resource))
                {
                    m_nTextures [nVersion] = reader.ReadUInt32 ();
                    m_index [nVersion] = new ushort [m_nTextures [nVersion]];
                    for (int i = 0; i < m_index.Length; i++)
                        m_index [nVersion][i] = reader.ReadUInt16 ();
                }
            }
        }

        //------------------------------------------------------------------------------

        public PigTexture LoadInfo (FileStream fs, BinaryReader fp, short nTexture)
        {
            int nVersion = Version;
            if (m_info [nVersion] == null)
            {
                m_header [nVersion].Read (fp);
                m_info [nVersion] = new PigTexture [m_header [nVersion].nTextures];
                for (int i = 0; i < m_header [nVersion].nTextures; i++)
                    m_info [nVersion][i].Read (fp);
                m_nOffsets [nVersion] = fs.Position;
            }
            return m_info [nVersion][m_index [nVersion][nTexture] - 1];
        }

        //------------------------------------------------------------------------

        FileStream OpenPigFile ()
        {
            String filename  = DLE.descentPath [Version];
            if (!filename.Contains (".pig"))
                filename += "groupa.pig";
            FileStream fs = File.OpenRead (filename);
            using (BinaryReader fp = new BinaryReader (fs)) 
                {
                uint nOffset = fp.ReadUInt32 ();
                if (nOffset == 0x47495050) // 'PPIG' Descent 2 type
	                nOffset = 0;
                else if (nOffset < 0x10000)
	                nOffset = 0;
                fs.Seek (nOffset, SeekOrigin.Begin);
                }
            return fs;
        }

        //------------------------------------------------------------------------------

        void LoadTextures (int nVersion)
        {
        if (nVersion < 0) {
	        nVersion = Version;
	        m_pigFiles [nVersion] = DLE.descentPath [nVersion];
            m_paletteName = DLE.Palettes.Name;
	        m_info [nVersion] = null;
	        }
        Release (nVersion, true, false);
        FileStream fs = OpenPigFile ();
        for (short i = 0, j = (short) MaxTextures; i < j; i++)
	        m_textures [nVersion][i].Load (fs, i, nVersion);
        fs.Close ();
        }

        //------------------------------------------------------------------------------

        Texture AddExtra (ushort nIndex)
        {
	        Texture tex = new Texture();
            if (tex != null)
            {
                m_extra.Add (tex);
                tex.m_nIndex = nIndex;
            }
        return tex;
        }

        //------------------------------------------------------------------------

        void Release (int nVersion, bool bDeleteAll, bool bDeleteUnused)
        {
            // free any m_textures that have been buffered
            if (m_textures [nVersion] != null)
            {
                foreach (Texture tex in m_textures [nVersion])
                {
                    if (bDeleteUnused)
                    {
                        if (tex.m_bCustom && !tex.m_bUsed)
                            tex.Release ();
                    }
                    else
                    {
                        if (bDeleteAll || tex.m_bCustom)
                            tex.Release ();
                    }
                }
            }
        }

        //------------------------------------------------------------------------

        void Release (bool bDeleteAll, bool bDeleteUnused) 
        {
        // free any m_textures that have been buffered
        for (int i = 0; i < 2; i++) 
	        Release (i, bDeleteAll, bDeleteUnused);
        m_extra = new List<Texture> ();
        }


        //------------------------------------------------------------------------------

        void Blend (ref BGRA dest, BGRA src)
        {
        if (DLE.Palettes.SuperTransp (src))
	        dest.r =
	        dest.g =
	        dest.b =
	        dest.a = 0;
        else if (src.a > 0) {
	        if (src.a == 255)
		        dest = src;
	        else {
		        int b = 255 - src.a;
		        dest.r = (byte) (((int) dest.r * b + (int) src.r * src.a) / 255);
		        dest.g = (byte) (((int) dest.g * b + (int) src.g * src.a) / 255);
		        dest.b = (byte) (((int) dest.b * b + (int) src.b * src.a) / 255);
		        }
	        }
        }

        //-------------------------------------------------------------------------

        private struct tFrac
        {
            public int c, d;
        }

        int BlendTextures (short nBaseTex, short nOvlTex, Texture destTexP, int x0, int y0) 
        {

            short [] nTextures = new short [2];
            Texture [] texP = new Texture [2];
            BGRA [] bmBufP = destTexP.Buffer;
            BGRA [] srcDataP;
            int mode;
            int w, h, i, j, x, y, z, y1, s;
            tFrac scale, scale2;

            nTextures [0] = nBaseTex;
            nTextures [1] = (short) (nOvlTex & 0x3fff);
            mode = (int) (nOvlTex & 0xC000);

            for (i = 0; i < 2; i++) 
            {
                if ((nTextures [i] < 0) || (nTextures [i] >= MaxTextures))
                    nTextures [i] = 0;
                texP [i] = Texture (nTextures [i]);
            }
	
	        // Define bmBufP based on texture numbers and rotation
            destTexP.m_width = texP [0].m_width;
            destTexP.m_height = texP [0].m_height;
            destTexP.m_size = texP [0].m_size;
            destTexP.m_bValid = true;
            srcDataP = texP [0].Buffer;
            // if not rotated, then copy directly
            if (x0 == 0 && y0 == 0)
            {
                Buffer.BlockCopy (bmBufP, 0, srcDataP, 0, srcDataP.Length * 4);
            }
            else
            {
                // otherwise, copy bit by bit
                w = (int)texP [0].m_width;
                int l1 = y0 * w + x0;
                int l2 = (int)texP [0].m_size - l1;
                Buffer.BlockCopy (bmBufP, 0, srcDataP, l1, l2);
                Buffer.BlockCopy (bmBufP, l2, srcDataP, 0, l2);
                h = w;//texP [0].m_height;
                i = 0;
                for (y = 0; y < h; y++)
                    for (x = 0; x < w; x++)
                    {
                        z = (((y - y0 + h) % h) * w) + ((x - x0 + w) % w);
                        bmBufP [i] = srcDataP [z];
                    }
            }

            // Overlay texture 2 if present

            if (nTextures [1] == 0)
                return 0;
            srcDataP = texP [1].Buffer;
            if (texP [0].m_width == texP [1].m_width)
                scale.c = scale.d = 1;
            else if (texP [0].m_width < texP [1].m_width) 
            {
                scale.c = (int) (texP [1].m_width / texP [0].m_width);
                scale.d = 1;
            }
            else 
            {
                scale.d = (int) (texP [0].m_width / texP [1].m_width);
                scale.c = 1;
            }
            scale2.c = scale.c * scale.c;
            scale2.d = scale.d * scale.d;

            w = (int) texP [1].m_width / scale.c * scale.d;
            h = w;//texP [1].m_height / scale.c * scale.d;
            s = (int) (texP [1].m_width * texP [1].m_width)/*texP [1].m_size*/ / scale2.c * scale2.d;
            if ((x0 == 0) && (y0 == 0)) 
            {
                if (mode == 0) 
                {
                    i = 0;
                    for (y = 0; y < h; y++)
                    {
                        for (x = 0; x < w; x++, i++) 
                        {
                            bmBufP [i].Blend (srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
                        }
                    }
                }
                else if (mode == 0x4000) 
                {
                    i = h - 1;
                    for (y = 0; y < h; y++, i--)
                    {
                        j = i;
                        for (x = 0; x < w; x++, j += w) 
                        {
                            bmBufP [j].Blend (srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
                        }
                    }
                }
                else if (mode == 0x8000) 
                {
                    i = s - 1;
                    for (y = 0; y < h; y++)
                    {
                        for (x = 0; x < w; x++, i--) 
                        {
                            bmBufP [i].Blend (srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
                        }
                    }
                }
                else if (mode == 0xC000) 
                {
                    i = (h - 1) * w;
                    for (y = 0; y < h; y++, i++)
                    {
                        j = i;
                        for (x = 0; x < w; x++, i -= w) 
                        {
                            bmBufP [j].Blend (srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
                        }
                    }
                }
            } 
            else 
            {
                if (mode == 0x0000) 
                {
                    for (y = 0; y < h; y++) 
                    {
                        y1 = ((y + y0) % h) * w;
                        for (x = 0; x < w; x++) 
                        {
                            bmBufP [y1 + (x + x0) % w].Blend (srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
                        }
                    }
                }
                else if (mode == 0x4000) 
                {
                    for (y = h - 1; y >= 0; y--)
                    {
                        for (x = 0; x < w; x++) 
                        {
                            bmBufP [((x + y0) % h) * w + (y + x0) % w].Blend (srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
                        }
                    }
                }
                else if (mode == 0x8000) 
                {
                    for (y = h - 1; y >= 0; y--) 
                    {
                        y1 = ((y + y0) % h) * w;
                        for (x = w - 1; x >= 0; x--) 
                        {
                            bmBufP [y1 + (x + x0) % w].Blend (srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
                        }
                    }
                }
                else if (mode == 0xC000) 
                {
                    for (y = 0; y < h; y++)
                    {
                        for (x = w - 1; x >= 0; x--) 
                        {
                            bmBufP [((x + y0) % h) * w + (y + x0) % w].Blend (srcDataP [(y * scale.c / scale.d) * (w * scale.c / scale.d) + x * scale.c / scale.d]);
                        }
                    }
                }
            }
        return 0;
        }

        //------------------------------------------------------------------------

        bool HasCustom
        {
            get
            {
                Texture[] texs = Textures;
                for (int i = 0; i < MaxTextures; i++)
                    if (texs [i].m_bCustom)
                        return true;
                return false;
            }
        }

        //------------------------------------------------------------------------

        ushort CustomCount
        {
            get
            {
                Texture [] texs = Textures;
                ushort count = 0;
                for (int i = 0; i < MaxTextures; i++)
                    if (texs [i].m_bCustom)
                        count++;
                return count;
            }
        }

        //------------------------------------------------------------------------

        void MarkUsed ()
        {
	        int nVersion = DLE.IsD1File ? 0 : 1;
	        int i, j, h = MaxTextures;
            Segment [] segs = DLE.Mine.Segments;
            Texture [] texs = m_textures [Version];

            for (i = 0; i < h; i++)
	            texs [i].m_bUsed = false;

            for (i = 0; i < h; i++)
            {
                Side[] sides = segs [i].m_sides;
	            for (j = 0; j < 6; j++) 
                {
		            if ((sides [i].m_nChild < 0) || (sides [i].m_nWall != DLE.Mine.NO_WALL)) 
                    {
			            m_textures [nVersion][sides [i].m_nBaseTex & 0x1FFF].m_bUsed = true;
			            if ((sides [i].m_nOvlTex & 0x1FFF) != 0)
				            m_textures [nVersion][sides [i].m_nOvlTex & 0x1FFF].m_bUsed = true;
			        }
		        }
	        }

            Texture parentTex = null;

            for (i = 0; i < h; i++) 
            {
	            if (!texs [i].m_bFrame)
                    parentTex = texs [i];
                else if (texs [i].m_bCustom && !texs [i].m_bUsed)
                    texs [i].m_bUsed = (parentTex != null) && parentTex.m_bUsed;
	        }
        }

        //------------------------------------------------------------------------
        // remove unused custom m_textures

        void RemoveUnused ()
        {
	        int nCustom = CustomCount;

        if (nCustom > 0) {
	        MarkUsed ();
	        Release (Version, false, true);
	        int nRemoved = nCustom - CustomCount;
            String message = nRemoved + " custom textures " + ((nRemoved == 1) ? "was" : "were") + " removed";
            //INFOMSG (message);
            if (nRemoved > 0)
		        DLE.Modified = true;
	        }
        }

        //------------------------------------------------------------------------------

    }
}
