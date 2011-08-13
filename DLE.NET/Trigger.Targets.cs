using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class TriggerTargets : SideKey
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
                m_count = (value < 0) ? (short)0 : (value > Trigger.MAX_TARGETS) ? Trigger.MAX_TARGETS : value;
            }
        }
        public SideKey [] m_targets = new SideKey [MAX_TARGETS];

        // -----------------------------------------------------------------------------

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

        public TriggerTargets ()
        {
            for (int i = 0; i < MAX_TARGETS; i++)
                m_targets [i] = new SideKey ();
            Clear ();
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

        public void Update (short nOldSeg, short nNewSeg)
        {
            for (int i = 0; i < m_count; i++)
                if (m_targets [i].m_nSegment == nOldSeg)
                    m_targets [i].m_nSegment = nNewSeg;
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
                SideKey temp = m_targets [i];
                for (; i < Count; i++)
                    m_targets [i] = m_targets [i + 1];
                m_targets [Count] = temp;
            }
            return Count;
        }

        // ------------------------------------------------------------------------

        public short Delete (SideKey key)
        {
            int i = -1;

            if (key.m_nSegment < 0)
            {
                // delete all sides of segment (-key.m_nSegment - 1) from the target list
                key.m_nSegment = (short)(-key.m_nSegment - 1);
                for (int j = Count - 1; j >= 0; j--)
                {
                    if (m_targets [j].m_nSegment == key.m_nSegment)
                    {
                        Delete (j);
                        if (i < 0)
                            i = j;
                    }
                }
            }
            else
            {
                i = Find (key);
                if (i >= 0)
                    Delete (i);
            }
            return (short)i;
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

        public SideKey [] Targets { get { return m_targets; } }

        public SideKey Target (uint i = 0)
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
}
