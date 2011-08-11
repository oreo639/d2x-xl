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

        Trigger [][] m_triggers = new Trigger [2][] {new Trigger [GameMine.MAX_TRIGGERS], new Trigger [GameMine.MAX_TRIGGERS]};

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

        public int FileOffset
        {
            get { return m_info [0].offset; }
            set { m_info [0].offset = value; }
        }

        // ------------------------------------------------------------------------

        public Trigger [] WallTriggers { get { return m_triggers [0]; } }

        public Trigger [] ObjTriggers { get { return m_triggers [1]; } }

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
        //DLE.MineView.Refresh ();
        UpdateReactor ();
        }

        // ------------------------------------------------------------------------

        void DeleteTargets (SideKey key) 
        {
            DLE.Backup.Begin ((int) UndoData.UndoFlag.udTriggers);
            int i;
            for (i = 0; i < WallTriggerCount; i++)
                WallTriggers [i].Delete (key);

        for (i = ObjTriggerCount; i > 0; )
	        if (ObjTriggers [--i].Delete (key) == 0) // no targets left
		        DeleteFromObject (i);
        DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public void UpdateTargets (short nOldSeg, short nNewSeg) 
        {
            for (int i = 0; i < WallTriggerCount; i++)
                WallTriggers [i].Update (nOldSeg, nNewSeg);
        }

        // ------------------------------------------------------------------------

        void UpdateReactor (void) 
        {
          ReactorTrigger reactorTrigger = ReactorTriggers [0];	// only one reactor trigger per level

        undoManager.Begin (udTriggers);
        // remove items from list that do not point to a wall
        for (short nTarget = 0; nTarget < reactorTrigger->Count (); nTarget++) {
	        if (!wallManager.FindBySide ((*reactorTrigger) [nTarget]))
		        reactorTrigger->Delete (nTarget);
	        }
        // add any exits to target list that are not already in it
        for (CWallIterator wi; wi; wi++) {
	        CWall* wallP = &(*wi);
	        CTrigger* trigP = wallP->Trigger ();
	        if (trigP == null)
		        continue;
	        bool bExit = trigP->IsExit (false);
	        bool bFound = (reactorTrigger->Find (*wallP) >= 0);
	        if (bFound == bExit)
		        continue;
	        if (bExit)
		        reactorTrigger->Add (*wallP);
	        else 
		        reactorTrigger->Delete (*wallP);
	        }
        undoManager.End ();
        }

        // ------------------------------------------------------------------------

    }
}
