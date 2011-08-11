using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public partial class TriggerManager
    {
        // ------------------------------------------------------------------------

        public MineItemInfo [] m_info = new MineItemInfo [2] {new MineItemInfo (), new MineItemInfo ()};
        public MineItemInfo m_reactorInfo = new MineItemInfo ();

        Trigger [][] m_triggers = new Trigger [2][] {new Trigger [GameMine.MAX_TRIGGERS], new Trigger [GameMine.MAX_TRIGGERS]};
        ReactorTrigger [] m_reactorTriggers = new ReactorTrigger [GameMine.MAX_REACTOR_TRIGGERS];

        // ------------------------------------------------------------------------

        public int WallTriggerCount
        {
            get { return m_info [0].count; }
            set { m_info [0].count = value; }
        }

        public int ObjTriggerCount
        {
            get { return m_info [1].count; }
            set { m_info [1].count = value; }
        }

        public int ReactorTriggerCount
        {
            get { return m_reactorInfo.count; }
            set { m_reactorInfo.count = value; }
        }

        public int FileOffset
        {
            get { return m_info [0].offset; }
            set { m_info [0].offset = value; }
        }

        // ------------------------------------------------------------------------

        public Trigger [] WallTriggers { get { return m_triggers [0]; } }

        public Trigger [] ObjTriggers { get { return m_triggers [1]; } }

        public ReactorTrigger [] ReactorTriggers { get { return m_reactorTriggers; } }

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

        DLE.Backup.Begin ((int) UndoData.UndoFlag.udTriggers);
        DLE.Walls.UpdateTrigger (nDelTrigger, GameMine.NO_TRIGGER);

        if (nDelTrigger < --WallTriggerCount) {
	        delTrig = WallTriggers [WallTriggerCount];
	        Wall wall = DLE.Walls.FindByTrigger ((short) WallTriggerCount);
	        if (wall != null) {
		        DLE.Backup.Begin ((int) UndoData.UndoFlag.udWalls);
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
            DLE.Backup.Begin ((int) UndoData.UndoFlag.udTriggers);
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
            DLE.Backup.Begin ((int) UndoData.UndoFlag.udTriggers);
            if (nDelTrigger < --ObjTriggerCount)
            {
                Trigger temp = ObjTriggers [nDelTrigger];
                ObjTriggers [nDelTrigger] = ObjTriggers [ObjTriggerCount];
                ObjTriggers [ObjTriggerCount] = temp;
                ObjTriggers [nDelTrigger].Key = nDelTrigger;
                ObjTriggers [ObjTriggerCount].Key = ObjTriggerCount;
            }
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

            DLE.Backup.Begin ((int) UndoData.UndoFlag.udTriggers);
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

    }
}
