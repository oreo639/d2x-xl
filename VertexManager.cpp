
#include "Mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

ushort CVertexManager::Add (void) 
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

void CVertexManager::DeleteUnused (void)
{
for (ushort nVertex = 0; nVertex < Count (); nVertex++)
	vertexManager.Status (nVertex) &= ~NEW_MASK; 
// mark all used verts
CSegment *segP = segmentManager.Segment (0);
for (short nSegment = 0; nSegment < segmentManager.Count (); nSegment++, segP++)
	for (short point = 0; point < 8; point++)
		vertexManager.Status (segP->m_info.verts [point]) |= NEW_MASK; 
for (ushort nVertex = Count () - 1; nVertex >= 0; nVertex--)
	if (!(vertexManager.Status (nVertex) & NEW_MASK))
		Delete (nVertex); 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Read (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
Count () = info.count;
for (int i = 0; i < Count (); i++)
	m_vertices [i].Read (fp, nFileVersion);
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Write (CFileManager& fp, CMineItemInfo& info, int nFileVersion)
{
info.count = Count ();
info.offset = fp.Tell ();
for (int i = 0; i < Count (); i++)
	m_vertices [i].Write (fp, nFileVersion);
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Unmark (void)
{
for (ushort nVertex = 0; nVertex < MAX_VERTICES; nVertex++)
	vertexManager.Status (nVertex) &= ~MARKED_MASK; 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Clear (void)
{
for (int i = 0; i < Count (); i++)
	m_vertices [i].Clear ();
}

// ----------------------------------------------------------------------------- 
