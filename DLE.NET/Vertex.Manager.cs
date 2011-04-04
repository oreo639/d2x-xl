using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    class VertexManager
    {
        public MineItemInfo m_info;

        public const uint MAX_VERTICES_D1 = 2808; // descent 1 max # of vertices
        public const uint MAX_VERTICES_D2 = (GameMine.MAX_SEGMENTS_D2 * 4 + 8); // descent 2 max # of vertices
        public const uint VERTEX_LIMIT = (GameMine.SEGMENT_LIMIT * 4 + 8); // descent 2 max # of vertices

        public uint MaxVertices { get { return DLE.IsD1File ? MAX_VERTICES_D1 : DLE.IsStdLevel ? MAX_VERTICES_D2 : VERTEX_LIMIT; } }

        public Vertex [] m_vertices = new Vertex [VERTEX_LIMIT];

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

        public byte Status (int i = 0) { return Vertex (i).Status; }

        public Vertex Vertex (int i) { return m_vertices [i]; }
    }
}
