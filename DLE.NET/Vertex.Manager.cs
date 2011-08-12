using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public partial class VertexManager
    {
        #region data

        public MineItemInfo m_info = new MineItemInfo ();

        public const int MAX_VERTICES_D1 = 2808; // descent 1 max # of vertices
        public const int MAX_VERTICES_D2 = (GameMine.MAX_SEGMENTS_D2 * 4 + 8); // descent 2 max # of vertices
        public const int VERTEX_LIMIT = (GameMine.SEGMENT_LIMIT * 4 + 8); // descent 2 max # of vertices

        public int MaxVertices { get { return DLE.IsD1File ? MAX_VERTICES_D1 : DLE.IsStdLevel ? MAX_VERTICES_D2 : VERTEX_LIMIT; } }

        Vertex [] m_vertices = new Vertex [GameMine.MAX_VERTICES];

        #endregion

        // ------------------------------------------------------------------------

        #region helper functions

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

        public Vertex [] Vertices { get { return m_vertices; } }

        public Vertex Vertex (int i) { return m_vertices [i]; }

        public byte Status (int i = 0) { return m_vertices [i].Status; }

        public void SetStatus (int i = 0, byte status = 0) { m_vertices [i].Status = status; }

        public bool Full { get { return Count >= GameMine.MAX_VERTICES; } }

        #endregion

        // ------------------------------------------------------------------------

        #region code

        public VertexManager ()
        {
            Count = 0;
            for (int i = 0; i < GameMine.MAX_VERTICES; i++)
                Vertices [i] = new Vertex (i);
        }

        // ------------------------------------------------------------------------

        public Vertex this [int i]
        {
            get { return Vertices [i]; }
            set { Vertices [i] = value; }
        }

        // ------------------------------------------------------------------------

        public int Add (ushort [] vertices, ushort count = 1, bool bUndo = true)
        {
            if (Count + count > GameMine.MAX_VERTICES)
                return 0;
            for (ushort i = 0; i < count; i++)
            {
                vertices [i] = (ushort) (Count + i);
            }
            DLE.Backup.Begin ();
            Count += count;
            DLE.Backup.End ();
            return count;

        }

        // ------------------------------------------------------------------------

        public void Delete (int i, bool bUndo = true)
        {
            DLE.Backup.Begin ();
            Count--;
            if (i < Count)
            {
                Vertices [i].Copy (Vertices [Count]);
                DLE.Segments.UpdateVertices ((ushort)Count, (ushort)i);
            }
            DLE.Backup.End ();
        }

        // ------------------------------------------------------------------------

        public void Read (FileStream fs)
        {
            using (BinaryReader fp = new BinaryReader (fs))
            {
                for (int i = 0; i < Count; i++)
                    Vertices [i].Read (fp);
            }
        }

        // ------------------------------------------------------------------------

        public void Write (FileStream fs)
        {
            using (BinaryWriter fp = new BinaryWriter (fs))
            {
                for (int i = 0; i < Count; i++)
                    Vertices [i].Write (fp);
            }
        }

        // ------------------------------------------------------------------------

        public void MarkAll (byte mask = GameMine.MARKED_MASK)
        {
            for (int i = 0; i < Count; i++)
                Vertices [i].Status |= mask;
        }

        // ------------------------------------------------------------------------

        public void UnmarkAll (byte mask = GameMine.MARKED_MASK)
        {
            for (int i = 0; i < Count; i++)
                Vertices [i].Status &= (byte) ~mask;
        }

        // ------------------------------------------------------------------------

        public void Clear ()
        {
            for (int i = 0; i < Count; i++)
                Vertices [i].Clear ();
        }

        // ------------------------------------------------------------------------

        public DoubleVector GetCenter ()
        {
            DoubleVector vMin = new DoubleVector (1e30, 1e30, 1e30);
            DoubleVector vMax = new DoubleVector (-1e30, -1e30, -1e30);
            for (int i = 0; i < Count; i++)
            {
                Vertex v = Vertices [i];
                if (vMin.v.x > v.v.x)
                    vMin.v.x = v.v.x;
                if (vMin.v.y > v.v.y)
                    vMin.v.y = v.v.y;
                if (vMin.v.z > v.v.z)
                    vMin.v.z = v.v.z;
                if (vMax.v.x < v.v.x)
                    vMax.v.x = v.v.x;
                if (vMax.v.y < v.v.y)
                    vMax.v.y = v.v.y;
                if (vMax.v.z < v.v.z)
                    vMax.v.z = v.v.z;
            }
            vMin.Add (vMax);
            vMin.Mul (0.5);
            return vMin;
        }

        // ------------------------------------------------------------------------

        public void SetCenter (DoubleVector offset)
        {
            for (int i = 0; i < Count; i++)
                Vertices [i].Sub (offset);
        }

        // ------------------------------------------------------------------------

        public Vertex Find (DoubleVector coord)
        {
            for (int i = 0; i < Count; i++)
                if (Vertices [i] == coord)
                    return Vertices [i];
            return null;
        }

        // ------------------------------------------------------------------------

        #endregion
    }
}
