
using System.IO;
namespace DLE.NET
{
    // ------------------------------------------------------------------------

    #region Trigger
    public partial class Trigger : TriggerTargets, IGameItem
    {
        public int Key { get; set; }

        //------------------------------------------------------------------------------

        Type m_type;
        Properties m_props;
        short m_nObject;
        int m_value;
        int m_time;
        ushort m_nIndex;

        // ------------------------------------------------------------------------

        public SideKey this [int i]
        {
            get
            {
                return m_targets [i];
            }
            set
            {
                m_targets [i] = value;
            }
        }

        //------------------------------------------------------------------------------

        public Trigger (int key = 0)
        {
            Key = key;
        }

        //------------------------------------------------------------------------------

        void Setup (short type, short props)
        {
        m_type = (Type) type;
        m_props = (Properties) props;
        if (m_type == Type.SPEEDBOOST)
	        m_value = 10;
        else if ((m_type == Type.CHANGE_TEXTURE) || (m_type == Type.MASTER))
	        m_value = 0;
        else if ((m_type == Type.MESSAGE) || (m_type == Type.SOUND))
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
	       m_type = (Type) fp.ReadByte ();
	       m_props = (Properties) (bObjTrigger ? fp.ReadUInt16 () : fp.ReadByte ());
	       Count = fp.ReadByte ();
	       fp.ReadByte (); // skip byte
	       m_value = fp.ReadInt32 ();
	        if ((DLE.LevelVersion < 21) && (m_type == Type.EXIT))
		       m_value = 0;
	        if ((version < 39) && (m_type == Type.MASTER))
		       m_value = 0;
	       m_time = fp.ReadInt32 ();
	        }
        else {
	       m_type = (Type) fp.ReadByte ();
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

        //------------------------------------------------------------------------

    }
    #endregion
}
