using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public partial class WallManager
    {
        // ------------------------------------------------------------------------

        public MineItemInfo m_info = new MineItemInfo ();

        Wall [] m_walls = new Wall [GameMine.MAX_WALLS];

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

        // ------------------------------------------------------------------------

        public Wall [] Walls { get { return m_walls; } }

        // ------------------------------------------------------------------------

        public WallManager ()
        {
            Count = 0;
            for (int i = 0; i < GameMine.MAX_WALLS; i++)
                Walls [i] = new Wall (i);
        }

        // ------------------------------------------------------------------------

        public Wall this [int i]
        {
            get { return (i < 0) ? null : Walls [i]; }
            set { Walls [i] = value; }
        }


        // ------------------------------------------------------------------------

        void Remove (short nDelWall)
        {
            if (nDelWall < --Count)
            {
                Wall temp = Walls [nDelWall];
                Walls [nDelWall] = Walls [Count];
                Walls [Count] = temp;
                Walls [nDelWall].Key = nDelWall;
                Walls [Count].Key = Count;
            }
    	DLE.Segments.Side (Walls [nDelWall]).SetWall (nDelWall);
	}

        // ------------------------------------------------------------------------

        public void Delete (short nDelWall)
        {
            if (nDelWall == GameMine.NO_WALL)
                return;
            Wall delWall = (nDelWall < 0) ? null : Walls [nDelWall];
            if (delWall == null)
            {
                delWall = DLE.Current.Wall;
                if (delWall == null)
                    return;
            }
            nDelWall = (short) delWall.Key;

            DLE.Backup.Begin ((int) (UndoData.UndoFlag.udSegments | UndoData.UndoFlag.udWalls | UndoData.UndoFlag.udTriggers));
            // if trigger exists, remove it as well
            DLE.Triggers.DeleteFromWall (delWall.m_nTrigger);
            // remove references to the deleted wall
            Wall oppWall = DLE.Segments.OppositeWall (delWall);
            if (oppWall != null)
                oppWall.m_linkedWall = -1;

            DLE.Triggers.DeleteTargets (delWall);
            DLE.Segments.Side (delWall).SetWall ((short) GameMine.NO_WALL);
            Remove (nDelWall);

            DLE.Backup.End ();
            DLE.Triggers.UpdateReactor ();

        }

        // ------------------------------------------------------------------------

        public void UpdateTrigger (short nOldTrigger, short nNewTrigger)
        {
	        Wall wall = FindByTrigger (nOldTrigger);

        if (wall != null) {
	        DLE.Backup.Begin ((int) UndoData.UndoFlag.udWalls);
	        wall.SetTrigger (nNewTrigger);
	        DLE.Backup.End ();
	        }
        }

        // ------------------------------------------------------------------------

        public Wall FindBySide (SideKey key, int i = 0)
        {
        for (; i < Count; i++)
	        if (Walls [i] == key)
		        return Walls [i];
        return null;
        }

        // ------------------------------------------------------------------------

        public Wall FindByTrigger (short nTrigger, int i = 0)
        {
        for (; i < Count; i++)
	        if (Walls [i].m_nTrigger == nTrigger)
		        return Walls [i];
        return null;
        }

        // ------------------------------------------------------------------------

    }
}
