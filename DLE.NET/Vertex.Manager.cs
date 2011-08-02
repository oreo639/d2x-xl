using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    class VertexManager
    {
        #region data

        public MineItemInfo m_info;

        public const int MAX_VERTICES_D1 = 2808; // descent 1 max # of vertices
        public const int MAX_VERTICES_D2 = (GameMine.MAX_SEGMENTS_D2 * 4 + 8); // descent 2 max # of vertices
        public const int VERTEX_LIMIT = (GameMine.SEGMENT_LIMIT * 4 + 8); // descent 2 max # of vertices

        public int MaxVertices { get { return DLE.IsD1File ? MAX_VERTICES_D1 : DLE.IsStdLevel ? MAX_VERTICES_D2 : VERTEX_LIMIT; } }

        public Dictionary<int, Vertex> m_vertices = new Dictionary <int, Vertex> ();
        
        List<int> m_usedKeys = new List<int> ();
        List<int> m_freeKeys = new List<int> ();

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

        public Dictionary<int, Vertex> Vertices { get { return m_vertices; } }

        public Vertex Vertex (int key) { return m_vertices [key]; }

        public int Key (Vertex vertex) { return vertex.Key; }

        public byte Status (int key = 0) { return m_vertices [key].Status; }

        public bool Full () { return m_vertices.Count >= MaxVertices; }

        #endregion

        // ------------------------------------------------------------------------

        VertexManager ()
        {
            m_usedKeys.Capacity = MaxVertices;
            m_freeKeys.Capacity = MaxVertices;
            for (int i = 0; i < MaxVertices; i++)
                m_freeKeys.Add (i);
        }

        // ------------------------------------------------------------------------

        #region code

        public int Add (int [] keys, ushort count = 1, bool bUndo = true)
        {
            if (m_freeKeys.Count < count)
                return 0;
            for (ushort i = 0; i < count; i++)
            {
                keys [i] = m_freeKeys [0];
                m_freeKeys.Remove (keys [i]);
                m_usedKeys.Add (keys [i]);
                m_vertices.Add (keys [i], new Vertex (keys [i]));
            }
            Count += count;
            DLE.undoManager.End ();
            return count;

        }

        // ------------------------------------------------------------------------

        public int Key (Vertex vertex)
        {
            return vertex.Key;
        }


        // ------------------------------------------------------------------------

        void Delete (int key, bool bUndo = true)
        {
            m_vertices.Remove (key);
            m_usedKeys.Remove (key);
            m_freeKeys.Add (key);
        }

        // ------------------------------------------------------------------------

        void Read (FileStream fs)
        {

            using (BinaryReader fp = new BinaryReader (fs))
            {
                foreach (var pair in m_vertices)
                    pair.Value.Read (fp);
            }
        }

        // ------------------------------------------------------------------------

        void Write (FileStream fs)
        {

            using (BinaryWriter fp = new BinaryWriter (fs))
            {
                foreach (var pair in m_vertices)
                    pair.Value.Write (fp);
            }
        }

        // ------------------------------------------------------------------------

        void MarkAll (byte mask = GameMine.MARKED_MASK)
        {
            foreach (var v in m_vertices)
                v.Value.Status |= mask;
        }

        // ------------------------------------------------------------------------

        void UnmarkAll (byte mask = GameMine.MARKED_MASK)
        {
            foreach (var v in m_vertices)
                v.Value.Status &= (byte) ~mask;
        }

        // ------------------------------------------------------------------------

        void Clear ()
        {
            foreach (var v in m_vertices)
                v.Value.Clear ();
        }

        // ------------------------------------------------------------------------

        DoubleVector GetCenter ()
        {
            DoubleVector vMin = new DoubleVector (1e30, 1e30, 1e30);
            DoubleVector vMax = new DoubleVector (-1e30, -1e30, -1e30);
            foreach (var v in m_vertices)
            {
                if (vMin.v.x > v.Value.v.x)
                    vMin.v.x = v.Value.v.x;
                if (vMin.v.y > v.Value.v.y)
                    vMin.v.y = v.Value.v.y;
                if (vMin.v.z > v.Value.v.z)
                    vMin.v.z = v.Value.v.z;
                if (vMax.v.x < v.Value.v.x)
                    vMax.v.x = v.Value.v.x;
                if (vMax.v.y < v.Value.v.y)
                    vMax.v.y = v.Value.v.y;
                if (vMax.v.z < v.Value.v.z)
                    vMax.v.z = v.Value.v.z;
            }
            vMin.Add (vMax);
            vMin.Mul (0.5);
            return vMin;
        }

        // ------------------------------------------------------------------------

        void SetCenter (DoubleVector offset)
        {
            foreach (var v in m_vertices)
            {
                v.Value.Sub (offset);
            }
        }

        // ------------------------------------------------------------------------

        Vertex Find (DoubleVector coord)
        {
            foreach (var v in m_vertices)
                if (v.Value == coord)
                    return m_vertices [v.Key];
            return null;
        }

        // ------------------------------------------------------------------------

        #endregion
    }
}
