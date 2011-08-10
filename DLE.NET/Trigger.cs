
using System.IO;
namespace DLE.NET
{
    // ------------------------------------------------------------------------

    #region TargetList
    public class TargetList : SideKey
    {
        public const short MAX_TARGETS = 10;

        short m_count;

        public short Count 
        {
            get
            {
                return m_count;
            }
            protected set
            {
               m_count = (value < 0) ? (short) 0 : (value > Trigger.MAX_TARGETS) ? Trigger.MAX_TARGETS : value;
            }
        }
        public SideKey [] m_targets = new SideKey [MAX_TARGETS];

        // ------------------------------------------------------------------------

        public TargetList ()
        {
            for (int i = 0; i < MAX_TARGETS; i++)
                m_targets [i] = new SideKey ();
            Clear ();
        }

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

        // ------------------------------------------------------------------------

        public new void Clear () 
        { 
		    Count = 0;
		    for (int i = 0; i < MAX_TARGETS; i++)
			    m_targets [i].Clear ();
            base.Clear ();
		}

        // ------------------------------------------------------------------------
        // add a new trigger target
        public short Add (SideKey key)
        {
            if (Count < m_targets.Length)
                m_targets [Count] = key;
            return Count++;
        }

        // ------------------------------------------------------------------------
        // add a new trigger target
        public short Add (short nSegment, short nSide)
        {
            return Add (new SideKey (nSegment, nSide));
        }

        // ------------------------------------------------------------------------
        // delete trigger target no. i
        public short Delete (int i = -1)
        {
            if ((Count > 0) && (i < --Count))
            {
                if (i < 0)
                    i = Count - 1;
                int l = Count - i;
                for (int j = 0; j < l; j++)
                    m_targets [j] = m_targets [j + 1];
                m_targets [Count] = new SideKey ();
            }
            return Count;
        }

        // ------------------------------------------------------------------------

        public short Pop () 
        { 
            return Delete (Count - 1); 
        }

        // ------------------------------------------------------------------------

        public int Find (SideKey key) 
        { 
		    for (int i = 0; i < Count; i++)
			    if (m_targets [i] == key)
				    return i;
		return -1;
		}

        // ------------------------------------------------------------------------

        public int Find (short nSegment, short nSide) 
        { 
            return Find (new SideKey (nSegment, nSide)); 
        }

        // ------------------------------------------------------------------------

        public SideKey Target (uint i)
        {
            return m_targets [i];
        }

        // ------------------------------------------------------------------------

	    public int Read (BinaryReader fp) 
        {
		    int i;
		    for (i = 0; i < MAX_TARGETS; i++)
			    m_targets [i].m_nSegment = fp.ReadInt16 ();
		    for (i = 0; i < MAX_TARGETS; i++)
			    m_targets [i].m_nSide = fp.ReadInt16 ();
		    return 1;
		}

        // ------------------------------------------------------------------------

        public void Write (BinaryWriter fp) 
        {
		    int i;
		    for (i = 0; i < MAX_TARGETS; i++)
			    fp.Write (m_targets [i].m_nSegment);
		    for (i = 0; i < MAX_TARGETS; i++)
			    fp.Write (m_targets [i].m_nSide);
		}

        // ------------------------------------------------------------------------
    }
    #endregion

    // ------------------------------------------------------------------------

    #region Trigger
    public partial class Trigger : TargetList, IGameItem
    {
        public int Key { get; set; }

        //------------------------------------------------------------------------------

        Type m_type;
        Properties m_props;
        short m_nObject;
        int m_value;
        int m_time;
        ushort m_nIndex;

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
