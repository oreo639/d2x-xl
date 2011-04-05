using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    class VertexManager
    {
        #region data

        public MineItemInfo m_info;

        public const uint MAX_VERTICES_D1 = 2808; // descent 1 max # of vertices
        public const uint MAX_VERTICES_D2 = (GameMine.MAX_SEGMENTS_D2 * 4 + 8); // descent 2 max # of vertices
        public const uint VERTEX_LIMIT = (GameMine.SEGMENT_LIMIT * 4 + 8); // descent 2 max # of vertices

        public uint MaxVertices { get { return DLE.IsD1File ? MAX_VERTICES_D1 : DLE.IsStdLevel ? MAX_VERTICES_D2 : VERTEX_LIMIT; } }

        public List<Vertex> m_vertices = new List <Vertex> ();

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

        public List<Vertex> Vertices { get { return m_vertices; } }

        public byte Status (int i = 0) { return Vertex (i).Status; }

        public Vertex Vertex (int i) { return m_vertices [i]; }

        public int Index (Vertex vertex) { return vertex.Index; }

        #endregion

        // ------------------------------------------------------------------------

        #region code

        public int Add (Vertex[] vertices, ushort count = 1, bool bUndo = true)
        {
            if (Count + count > MaxVertices)
                return 0;
            for (ushort i = 0; i < count; i++)
                m_vertices.Add (vertices [i] = new Vertex ());
            Count += count;
            undoManager.End ();
            return count;

        }

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        // ------------------------------------------------------------------------

        #endregion
    }
}
