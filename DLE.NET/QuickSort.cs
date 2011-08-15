using System;

namespace DLE.NET
{
    public class QuickSort <T> where T:IComparable
    {

        // ------------------------------------------------------------------------

        T [] data;

        // ------------------------------------------------------------------------

        public QuickSort (T [] data)
        {
            this.data = data;
        }

        // ------------------------------------------------------------------------

        public void Sort (int left, int right)
        {
            int l = left, r = right;
            T m = data [(left + right) / 2];
            do
            {
                while (data [l].CompareTo (m) <= 0)
                    l++;
                while (data [r].CompareTo (m) >= 0)
                    r--;
                if (l <= r)
                {
                    if (l < r)
                    {
                        T temp = data [l];
                        data [l] = data [r];
                        data [r] = temp;
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
    }

    // ------------------------------------------------------------------------

}
