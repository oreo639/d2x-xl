#ifndef __vertman_h
#define __vertman_h

#include "define.h"
#include "cfile.h"
#include "carray.h"
#include "Types.h"
#include "Selection.h"
#include "SegmentManager.h"
#include "FreeList.h"

//------------------------------------------------------------------------

#define MAX_VERTICES_D1		2808 // descent 1 max # of vertices
#define MAX_VERTICES_D2		(MAX_SEGMENTS_D2 * 4 + 8) // descent 2 max # of vertices
#define VERTEX_LIMIT			(SEGMENT_LIMIT * 4 + 8) // descent 2 max # of vertices

#define MAX_VERTICES ((theMine == null) ? MAX_VERTICES_D2 : theMine->IsD1File () ? MAX_VERTICES_D1 : theMine->IsStdLevel () ? MAX_VERTICES_D2 : VERTEX_LIMIT)

//------------------------------------------------------------------------

#ifdef _DEBUG

typedef CStaticArray< CVertex, VERTEX_LIMIT > vertexList;

#else

typedef CVertex vertexList [VERTEX_LIMIT];

#endif

//------------------------------------------------------------------------

class CVertexManager : CFreeList {
	public:
		vertexList		m_vertices;
		CMineItemInfo	m_info;

	public:
		// Segment and side getters
		// Vertex getters
		void ResetInfo (void) { m_info.Reset (); }

		inline vertexList& Vertices (void) { return m_vertices; }

		inline CVertex *Vertex (int i) { return &m_vertices [i]; }

		inline byte& Status (int i = 0) { return Vertex (i)->m_status; }

		inline int& Count (void) { return m_info.count; }

		inline int& FileOffset (void) { return m_info.offset; }

		inline ushort Index (CVertex* vertP) { return (ushort) (vertP - &m_vertices [0]); }

		ushort Add (ushort count = 1);

		void Delete (ushort nDelVert);

		void DeleteUnused (void);

		void Read (CFileManager& fp, int nFileVersion);

		void Write (CFileManager& fp, int nFileVersion);

		void Clear (void);

		void Unmark (void);

		VertexManager () : CFreeList (VERTEX_LIMIT) {
			ResetInfo ();
			Clear ();
			}
	};

extern CVertexManager vertexManager;

//------------------------------------------------------------------------

#endif //__vertman_h