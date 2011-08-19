
using System.IO;
using System;
using System.Xml;
namespace DLE.NET
{
    // ------------------------------------------------------------------------

    #region Trigger
    public partial class Trigger : TriggerTargets, IGameItem, IComparable<Trigger>
    {
        public int Key { get; set; }

        //------------------------------------------------------------------------------

        Flags m_flags = 0;
        Types m_type = 0;
        Properties m_props = 0;
        short m_nObject = -1;
        int m_value = 0;
        int m_time = 0;
        ushort m_nIndex = 0;

        //------------------------------------------------------------------------------

        public Flags Flag 
        { 
            get { return m_flags; } 
            set { m_flags = value; } 
        }
        
        public Types Type 
        { 
            get { return m_type; } 
            set { m_type = value; } 
        }
        
        public Properties Props 
        { 
            get { return m_props; } 
            set { m_props = value; } 
        }
        
        public short Object 
        { 
            get { return m_nObject; }
            set { m_nObject = value; }
        }

        public int Value 
        { 
            get { return m_value; } 
            set { m_value = value; } 
        }
        
        public int Time 
        { 
            get { return m_time; } 
            set { m_time = value; }
        }

        public ushort Index 
        { 
            get { return m_nIndex; } 
            set { m_nIndex = value; } 
        }

        //------------------------------------------------------------------------------

        public Trigger ()
        {
            Key = 0;
        }

        //------------------------------------------------------------------------------

        public Trigger (int key)
        {
            Key = key;
        }

        //------------------------------------------------------------------------------

        public Trigger (Trigger other)
        {
            Flag = other.Flag;
            Type = other.Type;
            Props = other.Props;
            Object = other.Object;
            Value = other.Value;
            Time = other.Time;
            Index = other.Index;
        }

        //------------------------------------------------------------------------------

        void Setup (short type, short props)
        {
        m_type = (Types) type;
        m_props = (Properties) props;
        if (m_type == Types.SPEEDBOOST)
	        m_value = 10;
        else if ((m_type == Types.CHANGE_TEXTURE) || (m_type == Types.MASTER))
	        m_value = 0;
        else if ((m_type == Types.MESSAGE) || (m_type == Types.SOUND))
	        m_value = 1;
        else 	
	        m_value = FixConverter.I2X (5); // 5% shield or energy damage
        m_time = -1;
        base.Clear ();
        }

        //------------------------------------------------------------------------------

	    public static bool operator< (Trigger t1, Trigger t2) 
        {
		    return (t1.m_nObject < t2.m_nObject) || ((t1.m_nObject == t2.m_nObject) && (t1.m_type < t2.m_type)); 
		}

        //------------------------------------------------------------------------------

	    public static bool operator> (Trigger t1, Trigger t2) 
        {
		    return (t1.m_nObject > t2.m_nObject) || ((t1.m_nObject == t2.m_nObject) && (t1.m_type > t2.m_type)); 
		}

        // ------------------------------------------------------------------------

        public void Read (BinaryReader fp, int version, bool bObjTrigger)
        {
        if (DLE.IsD2File) {
	       m_type = (Types) fp.ReadByte ();
	       m_props = (Properties) (bObjTrigger ? fp.ReadUInt16 () : fp.ReadByte ());
	       Count = fp.ReadByte ();
	       fp.ReadByte (); // skip byte
	       m_value = fp.ReadInt32 ();
	        if ((DLE.LevelVersion < 21) && (m_type == Types.EXIT))
		       m_value = 0;
	        if ((version < 39) && (m_type == Types.MASTER))
		       m_value = 0;
	       m_time = fp.ReadInt32 ();
	        }
        else {
	       m_type = (Types) fp.ReadByte ();
	       m_props = (Properties) fp.ReadInt16 ();
	       m_value = fp.ReadInt32 ();
	       m_time = fp.ReadInt32 ();
	       fp.ReadByte (); //skip 8 bit value "link_num"
	       Count = fp.ReadInt16 ();
	        if (Count < 0)
		        Count = 0;
	        else if (Count > MAX_TARGETS)
		        Count = MAX_TARGETS;
	        }
        base.Read (fp);
        }

        // ------------------------------------------------------------------------

        public void Write (BinaryWriter fp, int version, bool bObjTrigger)
        {
        if (DLE.IsD2File) {
	        fp.Write ((byte) m_type);
	        if (bObjTrigger)
		        fp.Write ((ushort) m_props);
	        else
		        fp.Write ((sbyte) m_props);
	        fp.Write ((sbyte) Count);
	        fp.Write ((byte) 0);
	        fp.Write (m_value);
	        fp.Write (m_time);
	        }
        else {
	        fp.Write ((byte) m_type);
	        fp.Write ((ushort) m_props);
	        fp.Write (m_value);
	        fp.Write (m_time);
	        fp.Write ((sbyte) Count);
	        fp.Write (Count);
	        }
        base.Write (fp);
        }

        // ------------------------------------------------------------------------

        public bool IsExit (bool bSecret)
        {
            return DLE.IsD1File
                     ? (m_flags & (Flags.EXIT | Flags.SECRET_EXIT)) != 0
                     : (m_type == Types.EXIT) || (bSecret && (m_type == Types.SECRET_EXIT));
        }

        //------------------------------------------------------------------------

        public int CompareTo (object obj)
        {
            if (obj == null)
                return 0;
            Trigger other = obj as Trigger;
            if (other == null)
                return 0;
            return CompareTo (other);
        }

        //------------------------------------------------------------------------

        public int CompareTo (Trigger other)
        {
            short i = this.Object;
            short m = other.Object;

            if (i < m)
                return -1;
            if (i > m)
                return 1;
            i = (short)this.Type;
            m = (short)other.Type;
            return (i < m) ? -1 : (i > m) ? 1 : 0;
        }

        //------------------------------------------------------------------------

        public int ReadXML (XmlNode parent, int id)
        {
            if (id >= GameMine.MAX_TRIGGERS)
                return -1;
            XmlNode node = parent.SelectSingleNode (string.Format (@"Trigger{0}", id));
            if (node == null)
                return 0;
            Clear ();
            Type = (Types)node.ToByte ("Type");
            Flag = (Flags)node.ToUShort ("Flag");
            Value = node.ToInt ("Value");
            Time = node.ToInt ("Time");
            int i = base.ReadXML (node);
            if (i != 1)
                return i;
            return DLE.Triggers.HaveResources () ? 1 : 0;
        }

        //------------------------------------------------------------------------

        public int WriteXML (XmlDocument doc, XmlElement parent, int id)
        {
            XmlElement node = doc.CreateElement (string.Format (@"Trigger{0}", id));
            parent.AppendChild (node);

            node.Add (doc, parent, "Type", Type.ToString ());
            node.Add (doc, parent, "Flag", Flag.ToString ());
            node.Add (doc, parent, "Value", Value.ToString ());
            node.Add (doc, parent, "Time", Time.ToString ());
            base.WriteXML (doc, node, 0);
            return 1;
        }

        //------------------------------------------------------------------------

    }
    #endregion
}
