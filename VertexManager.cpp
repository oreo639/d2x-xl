
#include "Mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

ushort CVertexManager::Add (ushort* nVertices, ushort count) 
{ 
ushort nVertex;
for (ushort i = 0; i < count; i++) {
	if (m_free.Empty ())
		return i;
	nVertex = --m_free;
	m_vertices [nVertex].Clear ();
	nVertices [i] = nVertex;
	Count ()++;
	}
return count; 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Delete (ushort nDelVert)
{
undoManager.SetModified (true); 
m_free += (int) nDelVert;
Count ()--;
}

// ----------------------------------------------------------------------------- 

void CVertexManager::DeleteUnused (void)
{
for (CVertexIterator i; i; i++)
	i->Status () &= ~NEW_MASK; 
// mark all used verts
CSegment *segP = segmentManager.Segment (0);
for (CSegmentIterator i; i; i++)
	for (short point = 0; point < 8; point++)
		vertexManager.Status (i->m_info.verts [point]) |= NEW_MASK; 
for (CVertexIterator i; i; i++)
	if (!(i->Status () & NEW_MASK))
		Delete (i.Index ()); 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Read (CFileManager& fp, int nFileVersion)
{
ushort nVertex;

for (int i = 0; i < Count (); i++) {
	Add (&nVertex);
	m_vertices [nVertex].Read (fp, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Write (CFileManager& fp, int nFileVersion)
{
m_info.offset = fp.Tell ();
for (CVertexIterator i; i; i++)
	i->Write (fp, nFileVersion);
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Unmark (void)
{
for (CVertexIterator i; i; i++)
	i->Status () &= ~MARKED_MASK; 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Clear (void)
{
for (CVertexIterator i; i; i++)
	i->Clear ();
}

// ----------------------------------------------------------------------------- 
