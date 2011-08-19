using System.Xml;

namespace DLE.NET
{
    public static class DLEExtensions
    {
        public static XmlNode Add (this XmlElement node, XmlDocument doc, XmlElement parent, string key, string value)
        {
            XmlElement attr = doc.CreateElement (key);
            if (attr == null)
                return null;
            attr.InnerText = value;
            return node.AppendChild (attr);
        }
    }
}
