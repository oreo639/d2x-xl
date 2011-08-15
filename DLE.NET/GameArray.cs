using System;

namespace DLE.NET
{
    public class GameArray <T> where T : IGameItem, IComparable<T>, new()
    {
        // ------------------------------------------------------------------------

        T [] m_data;
        int m_length = 0;
        MineItemInfo m_info = new MineItemInfo ();

        // ------------------------------------------------------------------------

        public T [] Data { get { return m_data; } }

        public MineItemInfo Info { get { return m_info; } }

        public int Count 
        { 
            get { return Info.count; } 
            set { Info.count = value; } 
        }

        public int FileOffset
        {
            get { return Info.offset; }
            set { Info.offset = value; }
        }

        // ------------------------------------------------------------------------

        public T this [int i]
        {
            get { return Data [i]; }
            set { Data [i] = value; }
        }

        // ------------------------------------------------------------------------

        public GameArray (int length = 0) 
        {
            Create (length);
        }

        // ------------------------------------------------------------------------

        public bool Create (int length)
        {
            if (length > 0)
            {
                m_data = new T [length];
                if (m_data == null)
                    return false;
                for (int i = 0; i < length; i++)
                {
                    m_data [i] = new T ();
                    if (m_data [i] == null)
                    {
                        m_data = null;
                        return false;
                    }
                    m_data [i].Key = i;
                }
                m_length = length;
            }
            return true;
        }

        // ------------------------------------------------------------------------

        public void Renumber ()
        {
            for (int i = 0; i < Count; i++)
                m_data [i].Key = i;
        }

        // ------------------------------------------------------------------------

        public void Swap (int i, int j)
        {
            T h = m_data [i];
            m_data [i] = m_data [j];
            m_data [j] = h;
            m_data [i].Key = i;
            m_data [j].Key = j;
        }

        // ------------------------------------------------------------------------

        public void Sort ()
        {
            if (Info.count > 1)
            {
                // using generic QuickSort class inside generic class
                //QuickSort<T> sorter = new QuickSort<T> (Data);
                //sorter.Sort (0, Info.count - 1);
                QuickSort (0, Info.count - 1);
            }
        }

        // ------------------------------------------------------------------------

        public void QuickSort (int left, int right)
        {
            int l = left, r = right;
            T m = m_data [(left + right) / 2];
            do
            {
                while (m_data [l].CompareTo (m) <= 0)
                    l++;
                while (m_data [r].CompareTo (m) >= 0)
                    r--;
                if (l <= r)
                {
                    if (l < r)
                    {
                        T temp = m_data [l];
                        m_data [l] = m_data [r];
                        m_data [r] = temp;
                    }
                    l++;
                    r--;
                }
            } while (l <= r);
            if (left < r)
                QuickSort (left, r);
            if (l < right)
                QuickSort (l, right);
        }

        // ------------------------------------------------------------------------

    }
}
