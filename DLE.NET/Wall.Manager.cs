
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

        public bool Full
        {
            get { return Count >= GameMine.MAX_WALLS; }
        }

        // ------------------------------------------------------------------------

        public bool HaveResources (SideKey key = null)
        {
        Wall wall = DLE.Segments.Wall (key);
        if (wall != null) {
	        DLE.ErrorMsg ("There is already a wall on this side");
	        return false;
	        }
        if (Full) {
	        DLE.ErrorMsg ("Maximum number of walls reached");
	        return false;
	        }
        return true;
        }

        // ------------------------------------------------------------------------

        public ushort Add () 
        { 
        if (!HaveResources ())
	        return GameMine.NO_WALL;
        return (ushort) Count++;
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

        public Wall Create (SideKey key, Wall.Types type, ushort flags, Wall.KeyTypes keys, sbyte nClip, short nTexture) 
        {
        if (!HaveResources (key))
	        return null;

        DLE.Current.Get (key);

        Segment seg = key.Segment;
        Side side = key.Side;

        // if wall is an overlay, make sure there is no child
        short nChild = seg.GetChild (key.m_nSide);
        if (type < 0)
	        type = (nChild == -1) ? Wall.Types.OVERLAY : Wall.Types.OPEN;

        if (type == Wall.Types.OVERLAY) {
	        if (nChild != -1) {
		        DLE.ErrorMsg (@"Switches can only be put on solid sides.");
		        return null;
		        }
	        }
        else {
	        // otherwise make sure there is a child
	        if (nChild == -1) {
		        DLE.ErrorMsg (@"This side must be attached to an other segment before a wall can be added.");
		        return null;
		        }
	        }

        ushort nWall = (ushort) Count;

        // link wall to segment/side
        DLE.Backup.Begin (UndoData.Flags.udSegments | UndoData.Flags.udWalls);
        side.SetWall ((short) nWall);
        Wall wall = Walls [nWall];
        wall.Setup (key, nWall, type, nClip, (ushort) nTexture, false);
        wall.Key = nWall;
        wall.m_flags = flags;
        wall.m_keys = keys;
        // update number of Walls () in mine
        Count++;
        DLE.Backup.End ();
        //DLE.MineView ().Refresh ();
        return wall;
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

            DLE.Backup.Begin (UndoData.Flags.udSegments | UndoData.Flags.udWalls | UndoData.Flags.udTriggers);
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

        public Wall OppositeWall (short nSegment, short nSide)
        {
        Side side = DLE.Segments.OppositeSide (null, new SideKey (nSegment, nSide));
        return (side == null) ? null : side.Wall;
        }

        // ------------------------------------------------------------------------

        public void UpdateTrigger (short nOldTrigger, short nNewTrigger)
        {
	        Wall wall = FindByTrigger (nOldTrigger);

        if (wall != null) {
	        DLE.Backup.Begin (UndoData.Flags.udWalls);
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

        public void CheckForDoor (SideKey key) 
        {
        if (DLE.ExpertMode)
	        return;

        DLE.Current.Get (key);
        Wall wall = DLE.Segments.Wall (key);

        if (wall == null)
         return;
        if (!wall.IsDoor)
	        return;

        DLE.ErrorMsg (@"Changing the texture of a door only affects\n
			           how the door will look before it is opened.\n
			           You can use this trick to hide a door\n
			           until it is used for the first time.\n\n
			           Hint: To change the door animation,\n
			           select 'Wall edit...' from the Tools\n
			           menu and change the clip number.");
        }

        // ------------------------------------------------------------------------

        public bool ClipFromTexture (SideKey key)
        {
        Wall wall = DLE.Segments.Wall (key);

        if ((wall == null) || !wall.IsDoor)
	        return true;

        short nBaseTex, nOvlTex;

        DLE.Segments.Textures (key, out nBaseTex, out nOvlTex);

        return (wall.SetClip (nOvlTex) >= 0) || (wall.SetClip (nBaseTex) >= 0);
        }

        // ------------------------------------------------------------------------

    }
}
