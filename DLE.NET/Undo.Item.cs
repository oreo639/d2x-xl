using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLEdotNET
{
    public class UndoItem<T>
    {
		Dictionary<int, T>  m_backup = null;
		Dictionary<int, T>  m_source = null;
		int	                m_nId = -1; // used by undo manager

        bool Empty { get { return m_backup == null; } }

        // ------------------------------------------------------------------------

		public UndoItem () {}

        ~UndoItem () { Destroy (); }
        
        // ------------------------------------------------------------------------

		public int Create (Dictionary<int, T> source) {
			m_backup = new Dictionary<int, T> (source);
			if (m_backup == null)
				return 0;
			m_source = source;
			return 1;
			}

        // ------------------------------------------------------------------------

	    public bool Backup (Dictionary<int, T> source) 
        {
			return Create (source) != 0;
		}


        // ------------------------------------------------------------------------

		public bool Restore () 
        {
			if (m_backup == null)
				return false;
            m_source = new Dictionary<int, T> (m_backup);
			return true;
		}	

        // ------------------------------------------------------------------------

		public void Destroy () 
        {
			Reset ();
		}

        // ------------------------------------------------------------------------

        public void Reset () 
        {
			m_backup = null;
			m_source = null;
		}

        // ------------------------------------------------------------------------

        public bool Diff () 
        {
			if (m_backup == null)
				return false;
			if (m_source == null)
				return false;
            if (m_backup.Count != m_source.Count)
                return false;
            for (int i = 0; i < m_backup.Count; i++) 
            {
                IComparable i1 = (IComparable) m_backup [i];
                IComparable i2 = (IComparable) m_source [i];
                if (i1.CompareTo (i2) != 0)
                    return false;
            }
            return true;
		}

        // ------------------------------------------------------------------------

        public bool Cleanup () 
        {
			if (!Diff ())
				Destroy ();
			return !Empty;
		}

        // ------------------------------------------------------------------------

    }
}
