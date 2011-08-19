using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace DLE.NET
{
    public struct UVL
    {
        public short u, v;
        public ushort l;

        //------------------------------------------------------------------------------

        public void Read (BinaryReader fp)
        {
            u = fp.ReadInt16 ();
            v = fp.ReadInt16 ();
            l = fp.ReadUInt16 ();
        }

        //------------------------------------------------------------------------------

        public void Write (BinaryWriter fp)
        {
            fp.Write (u);
            fp.Write (v);
            fp.Write (l);
        }

        //------------------------------------------------------------------------------

        public void Clear ()
        {
            u = v = 0;
            l = 0;
        }

        //------------------------------------------------------------------------------

        public UVL (short u = 0, short v = 0, ushort l = 0)
        {
            this.u = u;
            this.v = v;
            this.l = l;
        }

        //------------------------------------------------------------------------------

        public int ReadXML (XmlNode parent, int id)
        {
            XmlNode node = parent.SelectSingleNode (string.Format (@"UVL{0}", id));
            if (node == null)
                return 0;
            u = Convert.ToInt16 (node.Attributes ["U"]);
            v = Convert.ToInt16 (node.Attributes ["V"]);
            l = Convert.ToUInt16 (node.Attributes ["L"]);
            return 1;
        }

        //------------------------------------------------------------------------------

        public int WriteXML (XmlDocument doc, XmlElement parent, int id)
        {
            XmlElement node = doc.CreateElement (string.Format (@"UVL{0}", id));
            parent.AppendChild (node);
            node.SetAttribute ("U", u.ToString ());
            node.SetAttribute ("V", u.ToString ());
            node.SetAttribute ("L", u.ToString ());
            return 1;
        }


        //------------------------------------------------------------------------------

    }
}