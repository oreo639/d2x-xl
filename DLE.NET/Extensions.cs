using System.Xml;
using System;

namespace DLE.NET
{
    public static class DLEExtensions
    {
        // ------------------------------------------------------------------------

        public static XmlNode Add (this XmlElement node, XmlDocument doc, XmlElement parent, string key, string value)
        {
            XmlElement attr = doc.CreateElement (key);
            if (attr == null)
                return null;
            attr.InnerText = value;
            return node.AppendChild (attr);
        }

        // ------------------------------------------------------------------------

        public static byte ToByte (this XmlNode node, string key)
        {
            return Convert.ToByte (node.SelectSingleNode (key).InnerText);
        }

        public static sbyte ToSByte (this XmlNode node, string key)
        {
            return Convert.ToSByte (node.SelectSingleNode (key).InnerText);
        }

        public static short ToShort (this XmlNode node, string key)
        {
            return (short) Convert.ToInt16 (node.SelectSingleNode (key).InnerText);
        }

        public static ushort ToUShort (this XmlNode node, string key)
        {
            return (ushort) Convert.ToInt16 (node.SelectSingleNode (key).InnerText);
        }

        public static int ToInt (this XmlNode node, string key)
        {
            return (int)Convert.ToInt32 (node.SelectSingleNode (key).InnerText);
        }

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

    }
}
