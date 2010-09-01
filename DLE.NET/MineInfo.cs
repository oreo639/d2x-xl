using System.IO;
using System;

namespace DLE.NET
{
    public class MineFileInfo 
    {
	    public ushort signature;
	    public ushort version;
	    public int size;

        public void Read (BinaryReader fp) 
        {
	        signature = fp.ReadUInt16 ();
		    version = fp.ReadUInt16 ();
		    size = fp.ReadInt32 ();
	    }

        public void Write (BinaryWriter fp) 
        {
		    fp.Write (signature);
		    fp.Write (version);
		    fp.Write (size);
	    }
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class PlayerItemInfo 
    {
	    public int offset;
	    public int size;

	    public PlayerItemInfo () 
        { 
            offset = -1;
            size = 0; 
        }

        public void Read (BinaryReader fp) 
        {
		    offset = fp.ReadInt32 ();
		    size  = fp.ReadInt32 ();
		}

        public void Write (BinaryWriter fp) 
        {
		    fp.Write (offset);
		    fp.Write (size);
		}
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class MineItemInfo 
    {
        public int offset;
	    public int count;
	    public int size;

        public MineItemInfo () 
        { 
            Reset (); 
        }

        public void Reset () 
        { 
            offset = -1;
            count = size = 0; 
        }

        public void Read (BinaryReader fp) 
            {
		    offset = fp.ReadInt32 ();
		    count = fp.ReadInt32 ();
		    size  = fp.ReadInt32 ();
		    }

        public void Write (BinaryWriter fp) 
        {
		    fp.Write (offset);
		    fp.Write (count);
		    fp.Write (size);
		}
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

    public class MineInfo
    {
        public byte[] mineFilename = new byte [15];
        public int level;
        MineFileInfo fileInfo = new MineFileInfo ();
        PlayerItemInfo player = new PlayerItemInfo ();
        MineItemInfo[] items = new MineItemInfo [10];

        MineItemInfo objects { get { return items [0]; } }
        MineItemInfo walls  { get { return items [1]; } }
        MineItemInfo doors { get { return items [2]; } }
        MineItemInfo triggers { get { return items [3]; } }
        MineItemInfo links { get { return items [4]; } }
        MineItemInfo reactor { get { return items [5]; } }
        MineItemInfo botgen { get { return items [6]; } }
        MineItemInfo lightDeltaIndices { get { return items [7]; } }
        MineItemInfo lightDeltaValues { get { return items [8]; } }
        MineItemInfo equipgen { get { return items [9]; } }

	    void Read (BinaryReader fp) 
        {
		    fileInfo.Read (fp);
		    mineFilename = fp.ReadBytes (15);
		    level = fp.ReadInt32 ();
		    player.Read (fp);
            int h = items.Length;
		    if (fileInfo.size <= 143)
                h--;
            for (int i = 0; i < h; i++)
                items [i].Read (fp);
		    }

	    void Write (BinaryWriter fp) 
        {
		    fileInfo.Write (fp);
		    fp.Write (mineFilename);
		    fp.Write (level);
		    player.Write (fp);
            int h = items.Length;
		    if (fileInfo.size <= 143)
                h--;
            for (int i = 0; i < h; i++)
                items [i].Write (fp);
	    }
    }

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

}
