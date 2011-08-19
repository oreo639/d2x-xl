using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace DLE.NET
{
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------

    public class Side : IGameItem
    {
        public int Key { get; set; }

        //------------------------------------------------------------------------------

        public short m_nChild;
        public ushort m_nWall;		        // (was short) Index into Walls array, which wall (probably door) is on this side 
        public ushort m_nBaseTex;	        // Index into array of textures specified in bitmaps.bin 
        public ushort m_nOvlTex;		    // Index, as above, texture which gets overlaid on nBaseTex 
        public UVL[] m_uvls = new UVL [4];    // UVL coordinates at each point 

        // ------------------------------------------------------------------------

        public Segment Child { get { return (m_nChild < 0) ? null : DLE.Segments [(int) m_nChild]; } }

        public Wall Wall 
        { 
            get { return (m_nWall == GameMine.NO_WALL) ? null : DLE.Walls [(int)m_nWall]; } 
        }

        public Trigger Trigger
        {
            get 
            {
                Wall wall = Wall;
                return (wall == null) ? null : Wall.Trigger;
            }
        }

        public ushort BaseTex
        {
            get { return m_nBaseTex; }
            set { m_nBaseTex = value; }
        }

        public ushort OvlTex
        {
            get { return m_nOvlTex; }
            set { m_nOvlTex = value; }
        }

        // ------------------------------------------------------------------------

        public void Setup ()
        {
            m_nWall = GameMine.NO_WALL;
            m_nBaseTex =
            m_nOvlTex = 0;
            for (int i = 0; i < 4; i++)
            {
                m_uvls [i] = new UVL ();
                m_uvls [i].l = (ushort)GameMine.DEFAULT_LIGHTING;
            }
        }

        // ------------------------------------------------------------------------

        public void Read (BinaryReader fp, int version = 0, bool bFlag = false)
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
            for (int i = 0; i < m_uvls.Length; i++)
                m_uvls [i].Read (fp);
        }

        // ------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int version = 0, bool bFlag = false)
        {
            if (m_nOvlTex == 0)
                fp.Write (m_nBaseTex);
            else
            {
                fp.Write (m_nBaseTex | (ushort) 0x8000);
                fp.Write (m_nOvlTex);
            }
            for (int i = 0; i < m_uvls.Length; i++)
                m_uvls [i].Write (fp);
        }

        // ------------------------------------------------------------------------

        public void Clear ()
        {
            m_nChild = 0;
            m_nWall = GameMine.NO_WALL;
            m_nBaseTex = 0;
            m_nOvlTex = 0;
            ClearUVL ();
        }

        // ------------------------------------------------------------------------

        public void ClearUVL ()
        {
            for (int i = 0; i < m_uvls.Length; i++)
                m_uvls [i].Clear ();
        }

        // ------------------------------------------------------------------------

        public int Read (BinaryReader fp, bool bTextured)
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
		            m_uvls [i].Read (fp);
	        }
            else 
            {
	            m_nBaseTex = 0;
	            m_nOvlTex = 0;
	            for (int i = 0; i < 4; i++)
		            m_uvls [i].Clear ();
	        }
            return 1;
        }

        // ------------------------------------------------------------------------

        public void Write (BinaryWriter fp)
        {
            if (m_nOvlTex == 0)
	            fp.Write (m_nBaseTex);
            else 
            {
	            fp.Write ((ushort) (m_nBaseTex | 0x8000));
	            fp.Write (m_nOvlTex);
	        }
            for (int i = 0; i < 4; i++)
	            m_uvls [i].Write (fp);
        }

        // ------------------------------------------------------------------------

        public void GetTextures (out short nBaseTex, out short nOvlTex)
        {
        nBaseTex = (short) m_nBaseTex;
        nOvlTex = (short) (m_nOvlTex & 0x1FFF);
        }

        // ------------------------------------------------------------------------

        public bool SetTextures (ushort nNewBaseTex, ushort nNewOvlTex)
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
		        m_nOvlTex &= (ushort) 0xE000;	//preserve light settings
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

        public void SetWall (short nWall)
        {
            m_nWall = (ushort) nWall;
        }

        // ------------------------------------------------------------------------

        public bool IsTextured
        {
            get { return (m_nChild == -1) || (m_nWall < DLE.Mine.Info.walls.count && DLE.Mine.Walls [m_nWall].m_type != Wall.Types.OPEN); }
        }

        // ------------------------------------------------------------------------

        public bool UpdateChild (short nOldChild, short nNewChild)
        {
        if (m_nChild != nOldChild)
	        return false;
        m_nChild = nNewChild;
        return true;
        }

        // ------------------------------------------------------------------------

        public void Reset ()
        {
        m_nBaseTex = 0; 
        m_nOvlTex = 0; 
        m_nWall = GameMine.NO_WALL;
        double scale = DLE.Textures [DLE.FileType, m_nBaseTex].Scale ((short) m_nBaseTex);
        for (int i = 0; i < 4; i++) {
	        m_uvls [i].u = (short) (GameTables.defaultUVLs [i].u / scale); 
	        m_uvls [i].v = (short) (GameTables.defaultUVLs [i].v / scale); 
	        m_uvls [i].l = (ushort) GameMine.DEFAULT_LIGHTING; 
	        }
        }

        // ------------------------------------------------------------------------

        public int ReadXML (XmlNode parent, int id)
        {
            XmlNode node = parent.SelectSingleNode (string.Format (@"Side{0}", id));
            if (node == null)
                return -1;
            Clear ();
            short key = Convert.ToInt16 (node.SelectSingleNode ("Key").InnerText);
            if (key != id)
            {
                DLE.Backup.End ();
                DLE.ErrorMsg ("Invalid side number read");
                return -1;
            }
            m_nWall = GameMine.NO_WALL;
            m_nBaseTex = node.ToUShort ("BaseTex");
            m_nOvlTex = node.ToUShort ("OvlTex");
            for (int i = 0; i < 4; i++)
                m_uvls [i].ReadXML (node, i);
            if (!DLE.ExtBlkFmt)
                return 1;
            m_nWall = node.ToUShort ("Wall");
            if (m_nWall != GameMine.NO_WALL)
            {
                Wall w = new Wall ();
                w.ReadXML (node, m_nWall);
                m_nWall = DLE.Walls.Add ();
                DLE.Walls [m_nWall] = w;
            }
            return 1;
        }

        // ------------------------------------------------------------------------

        public int WriteXML (XmlDocument doc, XmlElement parent, int id)
        {
            XmlElement node = doc.CreateElement (string.Format (@"Side{0}", id));
            parent.AppendChild (node);

            node.Add (doc, parent, "Key", Key.ToString ());
            node.Add (doc, parent, "BaseTex", m_nBaseTex.ToString ());
            node.Add (doc, parent, "OvlTex", m_nOvlTex.ToString ());

            for (int i = 0; i < 4; i++)
                m_uvls [i].WriteXML (doc, node, i);

            if (DLE.ExtBlkFmt)
            {
                node.Add (doc, parent, "Wall", m_nWall.ToString ());

                if (m_nWall != GameMine.NO_WALL)
                    Wall.WriteXML (doc, node, m_nWall);
            }
            return 1;
        }

        // ------------------------------------------------------------------------

    }
}
