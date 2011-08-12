using System.IO;

namespace DLEdotNET
{
    public class Door : IGameItem
    {
        public int Key { get; set; }

        int m_nParts = 0;
        short [] m_nFrontWall = new short [2] {0,0};
        short [] m_nBackWall = new short [2] {0,0};
        int m_time = 0; 

        //------------------------------------------------------------------------------

        public void Read (BinaryReader fp, int version, bool bFlag)
        {
            m_nParts = fp.ReadInt32 ();
            m_nFrontWall [0] = fp.ReadInt16 ();
            m_nFrontWall [1] = fp.ReadInt16 ();
            m_nBackWall [0] = fp.ReadInt16 (); 
            m_nBackWall [1] = fp.ReadInt16 (); 
            m_time = fp.ReadInt32 ();		  
        }

        //------------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int version, bool bFlag)
        {
            fp.Write (m_nParts);
            fp.Write (m_nFrontWall [0]);
            fp.Write (m_nFrontWall [1]);
            fp.Write (m_nBackWall [0]);
            fp.Write (m_nBackWall [1]);
            fp.Write (m_time);
        }

        //------------------------------------------------------------------------------

        public new void Clear ()
        {
            m_nParts = 0;
            m_nFrontWall [0] = m_nFrontWall [1] = m_nBackWall [0] = m_nBackWall [1] = 0;
            m_time = 0;
        }

        //------------------------------------------------------------------------------

    }
}
