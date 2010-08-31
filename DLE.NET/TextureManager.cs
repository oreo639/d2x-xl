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

    public class PigHeader
    {
        public uint nId;
        public uint nVersion;
        public uint nTextures;
        public uint nSounds;

        public int Version { get; private set; }
        public uint Size { get { return (uint) ((Version == 1) ? 2 : 3) * sizeof (uint); } }

        public PigHeader(int version = 1)
        {
            Version = version;
        }

        public void Read (BinaryReader fp)
        {
            if (Version == 1)
            {
                nId = fp.ReadUInt32 ();
                nVersion = fp.ReadUInt32 ();
                nTextures = fp.ReadUInt32 ();
            }
            else
            {
                nTextures = fp.ReadUInt32 ();
                nSounds = fp.ReadUInt32 ();
            }
        }
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class PigTexture
    {
        public ushort whExtra;     // bits 0-3 width, bits 4-7 height

        public byte [] name;
        public byte dflags;      // this is only important for large bitmaps like the cockpit
        public ushort width;
        public ushort height;
        public byte flags;
        public byte avgColor;
        public uint offset;

        public int Version { get; private set; }
        public uint Size { get { return (uint) ((Version == 1) ? 18 : 17); } }

        public PigTexture (int version = 1)
        {
            Version = version;
        }

	    void Setup (int nVersion, ushort w = 0, ushort h = 0, byte f = 0, uint o = 0) 
        {
		    Version = nVersion;
		    width = w;
		    height = h;
		    flags = f;
		    offset = o;
		    dflags = 0;
		    avgColor = 0;
		}

	    void Decode () 
        {
		    width += (ushort) ((whExtra % 16) * 256);
		    if (((flags & 0x80) != 0) && (width > 256))
			    height *= width;
		    else
			    height += (ushort) ((whExtra / 16) * 256);
		    }

	    void Encode () 
        {
		    if (((flags & 0x80) != 0) && (width > 256)) {
			    whExtra = (ushort) (width / 256);
			    height /= width;
			    }
		    else {
			    whExtra = (ushort) ((width / 256) + ((height / 256) * 16));
			    height %= 256;
			    }
            width %= 256;
        }

        public void Read (BinaryReader fp, int nVersion = -1)
        {
            if (nVersion >= 0)
                Version = nVersion;
            name = fp.ReadBytes (name.Length);
            dflags = fp.ReadByte ();
            width = (ushort)fp.ReadByte ();
            height = (ushort)fp.ReadByte ();
            if (Version == 1)
                whExtra = (ushort)fp.ReadByte ();     // bits 0-3 width, bits 4-7 height
            else
            {
                name [7] = 0;
                whExtra = 0;
            }
            flags = fp.ReadByte ();
            avgColor = fp.ReadByte ();
            offset = fp.ReadUInt32 ();
            Decode ();
        }

        public void Write (BinaryWriter fp)
        {
            Encode ();
            fp.Write (name);
            fp.Write (dflags);
            fp.Write ((byte)width);
            fp.Write ((byte)height);
            if (Version == 1)
                fp.Write ((byte) whExtra);
            fp.Write (flags);
            fp.Write (avgColor);
            fp.Write (offset);
        }

        public int BufSize ()
        {
            return (int)width * (int)height;
        }
    }

    //------------------------------------------------------------------------------

    unsafe struct PigSoundD1
    {
        fixed byte unknown [20];

        public const uint Size = 20;
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class TextureManager
    {
        public const int MAX_TEXTURES_D1 = 584;
        public const int MAX_TEXTURES_D2 = 910;

        public int MaxTextures { get { return DLE.IsD1File () ? MAX_TEXTURES_D1 : MAX_TEXTURES_D2; } }
        public int Version { get { return DLE.IsD1File () ? 0 : 1; } }
        public long Offset { get { return m_nOffsets [Version]; } }

        public Texture [][] m_textures = new Texture [2][] { new Texture [MAX_TEXTURES_D1], new Texture [MAX_TEXTURES_D2] };
        public ushort [][] m_index = new ushort [2][];
        public String [][] m_names = new String [2][];
        public PigHeader [] m_header;
        public PigTexture [][] m_info = new PigTexture [2][] { null, null };
        public String [] m_pigFiles = new String [2] { "", "" };

        long [] m_nOffsets = new long [2] { 0, 0 };
        byte [] m_bmBuffer = new byte [512 * 512 * 4 * 32];
        Texture m_extra = null;

        //------------------------------------------------------------------------------

        public TextureManager()
        {
            LoadIndex (0);
            LoadIndex (1);
        }

        //------------------------------------------------------------------------------

        public Texture Texture (int nTexture)
        {
            return m_textures [Version] [nTexture];
        }

        //------------------------------------------------------------------------------

        public String Name (int nTexture)
        {
            return m_names [Version] [nTexture];
        }

        //------------------------------------------------------------------------------

        public PigTexture Info (int nTexture)
        {
            return m_info [Version] [nTexture];
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
                    m_index [nVersion] = new ushort [resource.Length / 2 - 2];
                    // skip four bytes
                    reader.ReadUInt16 ();
                    reader.ReadUInt16 ();
                    for (int i = 0; i < m_index.Length; i++)
                    {
                        m_index [nVersion][i] = reader.ReadUInt16 ();
                    }
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
	        if (m_pigFiles [nVersion] != DLE.descentPath [nVersion])
		        return;
	        m_pigFiles [nVersion] = DLE.descentPath [nVersion];
	        m_info [nVersion] = null;
	        }
        Release (nVersion, true, false);
        FileStream fs = OpenPigFile ();
        for (short i = 0, j = (short) MaxTextures; i < j; i++)
	        m_textures [nVersion][i].Load (fs, i, nVersion);
        fs.Close ();
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
                        if (tex.bCustom && !tex.bUsed)
                            tex.Release ();
                    }
                    else
                    {
                        if (bDeleteAll || tex.bCustom)
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
        m_extra.Release ();
        }

        //------------------------------------------------------------------------------

    }
}
