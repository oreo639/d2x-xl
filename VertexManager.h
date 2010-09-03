#ifndef __vertman_h
#define __vertman_h

#include "define.h"
#include "cfile.h"
#include "carray.h"
#include "Types.h"
#include "Selection.h"
#include "SegmentManager.h"

#define MAX_VERTICES_D1		2808 // descent 1 max # of vertices
#define MAX_VERTICES_D2		(MAX_SEGMENTS_D2 * 4 + 8) // descent 2 max # of vertices
#define VERTEX_LIMIT			(SEGMENT_LIMIT * 4 + 8) // descent 2 max # of vertices

#define MAX_VERTICES ((theMine == null) ? MAX_VERTICES_D2 : theMine->IsD1File () ? MAX_VERTICES_D1 : theMine->IsStdLevel () ? MAX_VERTICES_D2 : VERTEX_LIMIT)

#ifdef _DEBUG

typedef CStaticArray< CVertex, VERTEX_LIMIT > vertexList;

#else

typedef CVertex vertexList [VERTEX_LIMIT];

#endif



class CVertexManager {
	public:
		vertexList	m_vertices;
		ushort		m_nCount;

	public:
		inline bool IsValid (short i, short j) { return (i >= 0) && (i < j); }

		// Segment and side getters
		// Vertex getters
		inline vertexList& Vertices (void)
			{ return m_vertices; }

		inline CVertex *GetVertex (int i) { &m_vertices [i]; }

		inline byte& Status (int i = 0)
			{ return GetVertex (i)->m_status; }

		inline ushort& Count (void)
			{ return m_nCount; }

		void DeleteUnused (void);
		void Delete (short nDelVert);

		void Read (CFileManager& fp, CMineItemInfo& info, int nFileVersion);
		void Write (CFileManager& fp, CMineItemInfo& info, int nFileVersion);
		void Clear (void);
	};

extern CVertexManager vertexManager;

#endif //__vertman_h