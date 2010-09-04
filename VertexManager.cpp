#include "VertexManager.h"

// ----------------------------------------------------------------------------- 

ushort Add (void) 
{ 
return (Count () < MAX_VERTICES) ? Count ()++ : 65535; 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Delete (ushort nDelVert)
{
undoManager.SetModified (true); 
// fill in gap in vertex array and status
if (nDelVert < --m_nCount) {
	m_vertices [nDelVert] = m_vertices [m_nCount];
	segmentManager.UpdateVertex (m_nCount, nDelVert);
	}
}

// ----------------------------------------------------------------------------- 

void CVertexManager::DeleteUnusedVertices (void)
{
for (ushort nVertex = 0; nVertex < VertCount (); nVertex++)
	vertexManager.Status (nVertex) &= ~NEW_MASK; 
// mark all used verts
CSegment *segP = Segments (0);
for (short nSegment = 0; nSegment < SegCount (); nSegment++, segP++)
	for (short point = 0; point < 8; point++)
		vertexManager.Status (segP->m_info.verts [point]) |= NEW_MASK; 
for (ushort nVertex = VertCount () - 1; nVertex >= 0; nVertex--)
	if (!(vertexManager.Status (nVertex) & NEW_MASK))
		DeleteVertex(nVertex); 
}

// ----------------------------------------------------------------------------- 

void Read (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
Count () = info.count;
for (int = 0; i < Count (); i++)
	m_vertices [i].Read (fp, info, nFileVersion);
}

// ----------------------------------------------------------------------------- 

void Write (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
info.count () = count;
info.offset = fp.Tell ();
for (int = 0; i < Count (); i++)
	m_vertices [i].Write (fp, info, nFileVersion);
}

// ----------------------------------------------------------------------------- 

void VertexManager::Unmark (void)
{
for (ushort nVertex = 0; nVertex < MAX_VERTICES; nVertex++)
	vertexManager.Status (nVertex) &= ~MARKED_MASK; 
}

// ----------------------------------------------------------------------------- 

void Clear (void)
{
for (int = 0; i < Count (); i++)
	m_vertices [i].Clear ();
}

// ----------------------------------------------------------------------------- 
