
namespace DLE.NET
{
    public partial class TriggerManager
    {
        // ------------------------------------------------------------------------

        public MineItemInfo m_reactorInfo = new MineItemInfo ();

        GameArray<Trigger> [] m_triggers = new GameArray<Trigger> [2] { new GameArray<Trigger> (GameMine.MAX_TRIGGERS), new GameArray<Trigger> (GameMine.MAX_TRIGGERS) };
        ReactorTrigger [] m_reactorTriggers = new ReactorTrigger [GameMine.MAX_REACTOR_TRIGGERS];

        // ------------------------------------------------------------------------

        public Trigger [] WallTriggers { get { return m_triggers [0].Data; } }

        public Trigger [] ObjTriggers { get { return m_triggers [1].Data; } }

        public ReactorTrigger [] ReactorTriggers { get { return m_reactorTriggers; } }

        // ------------------------------------------------------------------------

        public int Count (int nClass)
        {
            return m_triggers [nClass].Count;
        }

        public int WallTriggerCount
        {
            get { return m_triggers [0].Count; }
            set { m_triggers [0].Count = value; }
        }

        public int ObjTriggerCount
        {
            get { return m_triggers [1].Count; }
            set { m_triggers [1].Count = value; }
        }

        public int ReactorTriggerCount
        {
            get { return m_reactorInfo.count; }
            set { m_reactorInfo.count = value; }
        }

        public int FileOffset
        {
            get { return m_triggers [0].FileOffset; }
            set { m_triggers [0].FileOffset = value; }
        }

        // ------------------------------------------------------------------------

        public TriggerManager ()
        {
            WallTriggerCount = 0;
            for (int i = 0; i < GameMine.MAX_TRIGGERS; i++)
            {
                WallTriggers [i] = new Trigger (i);
                ObjTriggers [i] = new Trigger (i);
            }
        }

        // ------------------------------------------------------------------------

        public Trigger this [int i, int j]
        {
            get 
            { 
                return (j < 0) ? null : m_triggers [i][j]; 
            }
            set 
            { 
                m_triggers [i][j] = value; 
            }
        }

        // ------------------------------------------------------------------------

        public bool Full 
        {
            get { return Count (0) >= GameMine.MAX_TRIGGERS; }
        }

        // ------------------------------------------------------------------------

        public bool HaveResources ()
        {
        if (!DLE.Walls.HaveResources ())
	        return false;
        if (Full) 
        {
	        DLE.ErrorMsg ("Maximum number of triggers reached");
	        return false;
	        }
        return true;
        }

        // ------------------------------------------------------------------------

        public void DeleteFromWall (short nDelTrigger) 
        {
        if (nDelTrigger == GameMine.NO_TRIGGER)
	        return;

        if (nDelTrigger < 0) {
	        Wall wall = DLE.Current.Wall;
	        if (wall == null)
		        return;
	        nDelTrigger = wall.m_nTrigger;
	        }

        Trigger delTrig = WallTriggers [nDelTrigger];

        if (delTrig == null)
	        return;

        DLE.Backup.Begin (UndoData.Flags.udTriggers);
        DLE.Walls.UpdateTrigger (nDelTrigger, GameMine.NO_TRIGGER);

        if (nDelTrigger < --WallTriggerCount) {
	        delTrig = WallTriggers [WallTriggerCount];
	        Wall wall = DLE.Walls.FindByTrigger ((short) WallTriggerCount);
	        if (wall != null) {
		        DLE.Backup.Begin (UndoData.Flags.udWalls);
		        wall.m_nTrigger = (byte) nDelTrigger;
		        DLE.Backup.End ();
		        }
	        }

        DLE.Backup.End ();
        DLE.MineView.Refresh ();
        UpdateReactor ();
        }

        // ------------------------------------------------------------------------

        public void DeleteTargets (SideKey key) 
        {
            DLE.Backup.Begin (UndoData.Flags.udTriggers);
            int i;
            for (i = 0; i < WallTriggerCount; i++)
                WallTriggers [i].Delete (key);

        for (i = ObjTriggerCount; i > 0; )
	        if (ObjTriggers [--i].Delete (key) == 0) // no targets left
		        DeleteFromObject ((short) i);
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        void DeleteFromObject (short nDelTrigger) 
        {
            if ((nDelTrigger < 0) || (nDelTrigger >= ObjTriggerCount))
	            return;
            DLE.Backup.Begin (UndoData.Flags.udTriggers);
            if (nDelTrigger < --ObjTriggerCount)
                m_triggers [1].Swap (nDelTrigger, ObjTriggerCount);
            DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public void UpdateTargets (short nOldSeg, short nNewSeg) 
        {
            for (int i = 0; i < WallTriggerCount; i++)
                WallTriggers [i].Update (nOldSeg, nNewSeg);
        }

        // ------------------------------------------------------------------------

        public void UpdateReactor () 
        {
            ReactorTrigger reactorTrigger = ReactorTriggers [0];	// only one reactor trigger per level

            DLE.Backup.Begin (UndoData.Flags.udTriggers);
            // remove items from list that do not point to a wall
            for (short nTarget = 0; nTarget < ReactorTriggerCount; nTarget++) {
	            if (DLE.Walls.FindBySide (reactorTrigger [nTarget]) == null)
		            reactorTrigger.Delete (nTarget);
	            }
            // add any exits to target list that are not already in it

            for (int i = 0; i < DLE.Walls.Count; i++)
            {
                Wall wall = DLE.Walls [i];
                Trigger trig = wall.Trigger;
	            if (trig == null)
		            continue;
	            bool bExit = trig.IsExit (false);
	            bool bFound = (reactorTrigger.Find (wall) >= 0);
	            if (bFound == bExit)
		            continue;
	            if (bExit)
		            reactorTrigger.Add (wall);
	            else 
		            reactorTrigger.Delete (wall);
	            }
            DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public void DeleteObjTriggers (short nObject) 
        {
        DLE.Backup.Begin (UndoData.Flags.udTriggers);
        for (short i = (short) (ObjTriggerCount - 1); i >= 0; i--)
	        if (ObjTriggers [i].Object == nObject)
		        DeleteFromObject (i);
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public void RenumberObjTriggers ()
        {
	            int	i;

            DLE.Backup.Begin (UndoData.Flags.udTriggers);
            for (i = ObjTriggerCount - 1; i >= 0; i--)
                ObjTriggers [i].Object = DLE.Objects.Index (DLE.Objects.FindBySig (ObjTriggers [i].Object));
            for (i = ObjTriggerCount - 1; i >= 0; i--)
            {
	            if (ObjTriggers [i].Object < 0)
		            DeleteFromObject ((short) i);
	        }
            SortObjTriggers ();
            DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public void RenumberTargetObjs ()
        {
        DLE.Backup.Begin (UndoData.Flags.udTriggers);
        for (int i = ObjTriggerCount - 1; i >= 0; i--) 
        {
            Trigger trig = ObjTriggers [i];
	        SideKey[] targets = ObjTriggers [i].Targets;
            int h = 0;
	        for (int j = 0; j < trig.Count; j++) {
		        if (targets [h].m_nSide >= 0) // trigger target is geometry
			        h++;
		        else {
			        GameObject obj = DLE.Objects.FindBySig (targets [h].m_nSegment);
			        if (obj == null)
				        trig.Delete (j--);
			        else
				        targets [h++].SegNum = DLE.Objects.Index (obj);
			        }
		        }
	        }
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        void SortObjTriggers ()
        {
	        int	h = ObjTriggerCount;

        if (h > 1) {
	        DLE.Backup.Begin (UndoData.Flags.udTriggers);
            m_triggers [1].Sort ();
	        DLE.Backup.End ();
	        }
        }

        // ------------------------------------------------------------------------

    }
}
