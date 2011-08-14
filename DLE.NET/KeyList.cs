using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    class KeyList
    {
        List<int> m_usedKeys = new List<int> ();
        List<int> m_freeKeys = new List<int> ();

        int m_maxKey = 0;

        // ------------------------------------------------------------------------

        public KeyList (int maxKey = 0) 
        {
            Resize (maxKey);
        }

        // ------------------------------------------------------------------------

        public int Resize (int maxKey)
        {
            if (m_maxKey >= maxKey)
                return 0;
            int i;
            for (i = m_maxKey; i < maxKey; i++)
                m_freeKeys.Add (i);
            i = maxKey - m_maxKey;
            m_maxKey = maxKey;
            return i;
        }

        // ------------------------------------------------------------------------

        public int Key
        {
            get 
            {
                if (m_freeKeys.Count == 0)
                    return -1;
                int i = m_freeKeys [0];
                m_freeKeys.Remove (i);
                m_usedKeys.Add (i);
                return i;
            }
            set
            {
                m_freeKeys.Add (value);
            }
        }

        // ------------------------------------------------------------------------

    }
}
