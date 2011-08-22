
using System.IO;
namespace DLE.NET
{
    public partial class ObjectManager
    {
        // ------------------------------------------------------------------------

        GameArray<GameObject> m_objects = new GameArray<GameObject> (GameMine.MAX_OBJECTS);
        SecretExit m_secretExit = new SecretExit ();

        public bool m_bSortObjects = true;

        // ------------------------------------------------------------------------

        public MineItemInfo Info
        {
            get { return m_objects.Info; }
        }

        public int Count
        {
            get { return m_objects.Count; }
            set { m_objects.Count = value; }
        }

        public int FileOffset
        {
            get { return m_objects.FileOffset; }
            set { m_objects.FileOffset = value; }
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

        public GameObject [] Objects { get { return m_objects.Data; } }

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
		        DLE.ErrorMsg (@"There are no objects in the mine.");
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

        public void Move (GameObject obj, short nSegment = -1)
        {
        DLE.Backup.Begin (UndoData.Flags.udObjects);
        if (obj == null)
	        obj = DLE.Current.GameObject;
        if (Index (obj) == Count)
	        SecretSegment = DLE.Current.m_nSegment;
        else {
            if (nSegment < 0)
                nSegment = DLE.Current.m_nSegment;
	        obj.Position = DLE.Segments.CalcCenter (nSegment);
	        // bump position over if this is not the first object in the segment
	        int i, count = 0;
	        for (i = 0; i < Count; i++)
		        if (Objects [i].m_nSegment == nSegment)
			        count++;
	        i = ((count & 1) != 0) ? -count : count;
	        obj.Position.v.y += 2 * i;
	        obj.m_location.lastPos.v.y += 2 * i;
	        obj.m_nSegment = nSegment;
	        DLE.MineView.Refresh (false);
	        }
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        void Read (BinaryReader fp)
        {
            if (Info.Restore (fp))
            {
                GameObject o = new GameObject ();

                for (short i = 0; i < Count; i++)
                {
                    if (i < GameMine.MAX_OBJECTS)
                    {
                        Objects [i].Read (fp, 0);
                        Objects [i].Key = i;
                    }
                    else
                    {
                        o.Read (fp, 0);
                    }
                }
            }
        }

        // ------------------------------------------------------------------------

        void Write (BinaryWriter fp)
        {
            if (Info.Setup (fp))
            {
                Info.size = 0x108;
                for (short i = 0; i < Count; i++)
                    Objects [i].Write (fp, 0);
            }
        }

        // ------------------------------------------------------------------------

        void Clear ()
        {
        for (short i = 0; i < Count; i++)
	        Objects [i].Clear ();
        }

        // ------------------------------------------------------------------------

        void SetCenter (DoubleVector v)
        {
        for (short i = 0; i < Count; i++)
	        m_objects [i].m_location.Sub (v);
        }

        // ------------------------------------------------------------------------

    }
}
