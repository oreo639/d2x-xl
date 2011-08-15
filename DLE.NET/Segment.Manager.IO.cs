using System.IO;

namespace DLE.NET
{
    public partial class SegmentManager
    {
        // ------------------------------------------------------------------------

        public void SetIndex ()
        {
            for (int i = 0; i < Count; i++)
                Segments [i].Key = i;
        }

        // ------------------------------------------------------------------------

        public void ReadSegments (BinaryReader fp)
        {
        if (m_segmentInfo.Restore (fp)) {
	        int i;
            Segment s = new Segment ();

	        for (i = 0; i < Count; i++) {
		        if (i < GameMine.MAX_SEGMENTS)
			        Segments [i].Read (fp);
		        else
			        s.Read (fp); // skip
		        }
	        if (Count > GameMine.MAX_SEGMENTS)
                Count = GameMine.MAX_SEGMENTS;
	        if (!DLE.IsD2File)
		        return;
	        for (i = 0; i < Count; i++)   
		        Segments [i].ReadExtras (fp, true);
	        }
        }

        // ------------------------------------------------------------------------

        void WriteSegments (BinaryWriter fp)
        {
        if (m_segmentInfo.Setup (fp)) {
	        m_segmentInfo.offset = (int) fp.BaseStream.Position;

	        for (int i = 0; i < Count; i++)
		        Segments [i].Write (fp);
	        if (!DLE.IsD2File)
		        return;
	        for (int i = 0; i < Count; i++)
		        Segments [i].WriteExtras (fp, true);
	        }
        }

        // ------------------------------------------------------------------------

        void ReadMatCens (BinaryReader fp, short nClass)
        {
        if (m_matCenInfo [nClass].Restore (fp)) {
	        MatCenter m = new MatCenter ();
	        for (int i = 0; i < MatCenCount (nClass); i++) {
		        if (i < GameMine.MAX_MATCENS)
			        MatCenters [nClass][i].Read (fp);
		        else 
			        m.Read (fp);
		        }
	        if (MatCenCount (nClass) > GameMine.MAX_MATCENS)
		        SetMatCenCount (nClass, (int)GameMine.MAX_MATCENS);
	        }
        }

        // ------------------------------------------------------------------------

        void WriteMatCens (BinaryWriter fp, short nClass)
        {
        if (m_matCenInfo [nClass].Setup (fp)) {
	        m_matCenInfo [nClass].size = (DLE.IsD1File && (nClass == 0)) ? 16 : 20; 
	        m_matCenInfo [nClass].offset = (int) fp.BaseStream.Position;
	        for (int i = 0; i < MatCenCount (nClass); i++)
		        MatCenters [nClass][i].Write (fp);
	        }
        }

        // ------------------------------------------------------------------------

        public void ReadRobotMakers (BinaryReader fp)
        {
        ReadMatCens (fp, 0);
        }

        // ------------------------------------------------------------------------

        public void WriteRobotMakers (BinaryWriter fp)
        {
        WriteMatCens (fp, 0);
        }

        // ------------------------------------------------------------------------

        public void ReadEquipMakers (BinaryReader fp)
        {
        ReadMatCens (fp, 1);
        }

        // ------------------------------------------------------------------------

        public void WriteEquipMakers (BinaryWriter fp)
        {
        WriteMatCens (fp, 1);
        }

        // ------------------------------------------------------------------------

        void Clear ()
        {
        for (int i = 0; i < Count; i++)
	        Segments [i].Clear ();
        }

        // ------------------------------------------------------------------------

    }
}
