using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    class BufPtr
    {
    //    public int	m_size;
    //    public int	m_index;

    //    #region functions
        
    //    BufPtr (int size = UndoManager.DLE_MAX_UNDOS, int index = -1) 
    //    {
    //        m_size = size;
    //        m_index = index;
    //    }

    //    void Setup (int size)
    //    { 
    //        if (m_index >= (m_size = size))
    //            m_index = m_size - 1;
    //    }

    //    public BufPtr Copy (BufPtr j) 
    //    {
    //        m_index = j.m_index;
    //        m_size = j.m_size;
    //        return this;
    //    }

    //    public BufPtr Copy (int i) 
    //    { 
    //        m_index = i; 
    //        return this;
    //    }

    //    #endregion

    //    #region operators

    //    public static implicit operator int (BufPtr i)
    //    {
    //        return i.m_index;
    //    }
        
    //    public static bool operator == (int i, int j) { return i == j; }

    //    public static bool operator == (BufPtr i, BufPtr j) { return i.m_index == j.m_index; }

    //    public static bool operator != (int i, int j) { return i != j; }

    //    public static bool operator != (BufPtr i, BufPtr j) { return i.m_index != j.m_index; }

    //    public static bool operator < (int i, int j) { return i < j; }

    //    public static bool operator > (int i, int j) { return i > j; }

    //    public static bool operator < (BufPtr i, BufPtr j) { return i.m_index < j.m_index; }

    //    public static bool operator > (BufPtr i, BufPtr j) { return i.m_index > j.m_index; }

    //    public static int operator ++ (BufPtr i) 
    //    { 
    //        return i.m_index = ++i.m_index % i.m_size; 
    //    }

    //    public static int operator -- (BufPtr i) 
    //    { 
    //        int h = i.m_index;
    //        i.m_index = ((i.m_index == 0) ? i.m_size : i.m_index) - 1; 
    //        return h;
    //    }

    //    public static int operator -- (BufPtr i)
    //    { 
    //        return i.m_index = ((i.m_index == 0) ? i.m_size : i.m_index) - 1; 
    //    }

    //    public static int operator - (BufPtr i, int j) 
    //    { 
    //        int h = i.m_index - j; 
    //        if (h < 0)
    //            h += i.m_size;
    //        return h;
    //    }

    //    public int operator + (BufPtr i, int j) 
    //    { 
    //        return (i.m_index + j) % i.m_size; 
    //    }

    //#endregion

    }
}
