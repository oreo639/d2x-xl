using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    public class GameArray <T> where T : IGameItem, IComparable, new()
    {
        // ------------------------------------------------------------------------

        T [] m_data;
        int m_length = 0;
        MineItemInfo m_info = new MineItemInfo ();

        // ------------------------------------------------------------------------

        public T [] Data { get { return m_data; } }

        public MineItemInfo Info { get { return m_info; } }

        public int Count { get { return Info.count; } }

        // ------------------------------------------------------------------------

        public GameArray () { }

        // ------------------------------------------------------------------------

        public bool Create (int length)
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

        public void QuickSort ()
        {
            if (Info.count > 1)
                Sort (0, Info.count - 1);
        }

        // ------------------------------------------------------------------------

        public void Sort (int left, int right)
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
                Sort (left, r);
            if (l < right)
                Sort (l, right);
        }

        // ------------------------------------------------------------------------

    }
}
