using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public partial class ObjectManager
    {
        // ------------------------------------------------------------------------

        public MineItemInfo m_info = new MineItemInfo ();

        GameObject [] m_objects = new GameObject [GameMine.MAX_OBJECTS];
        SecretExit m_secretExit = new SecretExit ();

        public bool m_bSortObjects = true;

        // ------------------------------------------------------------------------

        public int Count
        {
            get { return m_info.count; }
            set { m_info.count = value; }
        }

        public int FileOffset
        {
            get { return m_info.offset; }
            set { m_info.offset = value; }
        }

        public int SecretSegment
        {
            get { return m_secretExit.nSegment; }
            set { m_secretExit.nSegment = value; }
        }

        public DoubleMatrix SecretOrient
        {
            get { return m_secretExit.orient; }
            set { m_secretExit.orient.Set (value); }
        }

        // ------------------------------------------------------------------------

        public GameObject [] Objects { get { return m_objects; } }

        // ------------------------------------------------------------------------

        public ObjectManager ()
        {
            Count = 0;
            for (int i = 0; i < GameMine.MAX_OBJECTS; i++)
                Objects [i] = new GameObject (i);
        }

        // ------------------------------------------------------------------------

        public GameObject this [int i]
        {
            get { return (i < 0) ? null : Objects [i]; }
            set { Objects [i] = value; }
        }

        // ------------------------------------------------------------------------

        public void UpdateSegments (short nOldSeg, short nNewSeg)
        {
        DLE.Backup.Begin (UndoData.Flags.udObjects);
        for (int i = 0; i < Count; i++)
	        if (Objects [i].m_nSegment == nOldSeg)
		        Objects [i].m_nSegment = nNewSeg;
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public void SetIndex ()
        {
        for (int i = 0; i < Count; i++)
	        Objects [i].Key = i;
        }

        // ------------------------------------------------------------------------

        void Delete (short nDelObj = -1, bool bUndo = true)
        {
        if (Count == 0) {
	        if (!DLE.ExpertMode)
		        DLE.ErrorMsg (@"There are no Object () in the mine.");
	        return;
	        }
        if (Count == 1) {
	        if (!DLE.ExpertMode)
		        DLE.ErrorMsg (@"Cannot delete the last object.");
	        return;
	        }
        if (nDelObj < 0)
	        nDelObj = DLE.Current.m_nObject;
        if (nDelObj == Count) {
	        if (!DLE.ExpertMode)
		        DLE.ErrorMsg (@"Cannot delete the secret return.");
	        return;
	        }
        DLE.Backup.Begin (UndoData.Flags.udObjects);
        if (nDelObj < 0)
	        nDelObj = DLE.Current.m_nObject;
        DLE.Triggers.DeleteObjTriggers (nDelObj);
        if (nDelObj < --Count) {
	        for (int i = nDelObj + 1; i < Count; i++)
		        Objects [i].m_signature = (short) i;
            GameObject temp = Objects [nDelObj];
            for (int i = nDelObj; i < Count; i++)
            {
                Objects [i] = Objects [i + 1];
                Objects [i].Key = i;
            }
            Objects [Count] = temp;
            Objects [Count].Key = Count;
	        SetIndex ();
	        DLE.Triggers.RenumberObjTriggers ();
            DLE.Triggers.RenumberTargetObjs ();
	        }
        if (DLE.Current.m_nObject >= Count)
            DLE.Current.m_nObject = (short) (Count - 1);
        if (DLE.Other.m_nObject >= Count)
            DLE.Other.m_nObject = (short)(Count - 1);
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public void DeleteSegmentObjects (short nSegment)
        {
            for (short i = (short)Count; i >= 0; i--)
            {
                if (Objects [i].m_nSegment == nSegment)
                   Delete (i);
            }
        }

        // ------------------------------------------------------------------------

        public GameObject FindBySig (short nSignature)
        {
            for (short i = (short) (Count - 1); i >= 0; i--)
	            if (Objects [i].m_signature == nSignature)
		            return Objects [i];
            return null;
        }

        // ------------------------------------------------------------------------

        public short Index (GameObject obj)
        {
            return (short) ((obj == null) ? -1 : obj.Key);
        }

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

    }
}
