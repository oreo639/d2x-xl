
namespace DLE.NET
{
    public partial class SegmentManager
    {
        // ------------------------------------------------------------------------

        void RenumberMatCens (Segment.Functions nFunction, short nClass) 
        {
            int h = MatCenCount (nClass), i = 0, nErrors = 0;

            if (h > 1)
            {
                QuickSort<MatCenter> qs = new QuickSort<MatCenter> (MatCenters [nClass]);
                qs.Sort (0, h);
            }

            for (h = i = 0; i < MatCenCount (nClass); i++)
            {
                short nSegment = MatCenters [nClass] [i].m_nSegment;
                if (nSegment < 0)
                    ++nErrors;
                else
                {
                    Segment seg = Segments [nSegment];
                    if ((seg.m_function == nFunction) && (seg.m_nMatCen < 0))
                        seg.m_nMatCen = (sbyte)h++;
                    else
                    {
                        MatCenters [nClass] [i].m_nSegment = -1;
                        ++nErrors;
                    }
                }
            }

            if (nErrors > 0) // not all matcens assigned to a segment - try to find a segment that doesn't have a matcen
            {
                for (i = 0; i < Count; i++)
                {
                    Segment seg = Segments [i];
                    if ((seg.m_function == nFunction) && (seg.m_nMatCen < 0))
                    {
                        for (int j = 0; j < MatCenCount (nClass); j++)
                        {
                            if (MatCenters [nClass] [j].m_nSegment < 0)
                            {
                                seg.m_function = nFunction;
                                seg.m_nMatCen = (sbyte)j;
                                MatCenters [nClass] [j].m_nSegment = (short)(Count - i);
                                nErrors--;
                                break;
                            }
                        }
                    }
                }
            }

            if (nErrors > 0) // delete remaining unassigned matcens
            { 
                nErrors = 0;
                int j;
                for (i = 0, j = 0; j < MatCenCount (nClass); j++)
                {
                    if (MatCenters [nClass] [i].m_nSegment >= 0)
                        Segments [MatCenters [nClass] [i].m_nSegment].m_nMatCen = (sbyte)i++;
                    else
                    {
                        if (i < j)
                            MatCenters [nClass] [i] = MatCenters [nClass] [j];
                        ++nErrors;
                    }
                }
                SetMatCenCount (nClass, MatCenCount (nClass) - nErrors);
            }
        }

        // ------------------------------------------------------------------------

        public void RenumberRobotMakers () 
        {
            RenumberMatCens (Segment.Functions.ROBOTMAKER, 0);
        }

        // ------------------------------------------------------------------------

        public void RenumberEquipMakers () 
        {
            RenumberMatCens (Segment.Functions.EQUIPMAKER, 1);
        }

        // ------------------------------------------------------------------------

        public void RenumberFuelCenters ()
        {
            DLE.Backup.Begin (UndoData.Flags.udSegments);

            for (int i = 0; i < Count; i++)
                Segments [i].m_nMatCen = -1;

            RenumberRobotMakers ();
            RenumberEquipMakers ();

            for (int h = 0, i = 0; i < Count; i++)
            {
                Segment seg = Segments [i];
                if ((seg.m_function == Segment.Functions.FUELCEN) ||
                         (seg.m_function == Segment.Functions.REPAIRCEN))
                    seg.m_value = (sbyte)h++;
                else if ((seg.m_function == Segment.Functions.ROBOTMAKER) ||
                            (seg.m_function == Segment.Functions.EQUIPMAKER))
                {
                    if (seg.m_nMatCen >= 0)
                    {
                        seg.m_value = (sbyte)h++;
                        MatCenters [(seg.m_function == Segment.Functions.EQUIPMAKER) ? 1 : 0] [seg.m_nMatCen].m_nFuelCen = seg.m_value;
                    }
                    else
                    {
                        seg.m_value = -1;
                        seg.m_function = Segment.Functions.NONE;
                    }
                }
                else
                    seg.m_value = -1;
            }
            DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public Segment FindRobotMaker (ref short i)
        {
	        Segment seg = Segments [i];

        for (; i < Count; i++)
	        if (Segments [i].m_function == Segment.Functions.ROBOTMAKER)
		        return Segments [i];
        return null;
        }

        // ------------------------------------------------------------------------

    }
}
