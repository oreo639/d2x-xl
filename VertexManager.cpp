
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
UnmarkAll (NEW_MASK);
// mark all used verts
segmentManager.MarkAll (NEW_MASK);
for (CVertexIterator vi; vi; vi++)
	if (!(vi->Status () & NEW_MASK))
		Delete (vi.Index ()); 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::SetIndex (void)
{
int j = 0;
for (CVertexIterator vi; vi; vi++)
	vi->m_nIndex = j++;
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
for (CVertexIterator vi; vi; vi++)
	vi->Write (fp, nFileVersion);
}

// ----------------------------------------------------------------------------- 

void CVertexManager::MarkAll (void)
{
for (CVertexIterator vi; vi; vi++)
	vi->Status () |= MARKED_MASK; 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::UnmarkAll (void)
{
for (CVertexIterator vi; vi; vi++)
	vi->Status () &= ~MARKED_MASK; 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Clear (void)
{
for (CVertexIterator vi; vi; vi++)
	vi->Clear ();
}

// ----------------------------------------------------------------------------- 
