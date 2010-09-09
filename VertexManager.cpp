
#include "Mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

ushort CVertexManager::Add (ushort* nVertices, ushort count) 
{ 

#if USE_FREELIST

ushort nVertex;
for (ushort i = 0; i < count; i++) {
	if (m_free.Empty ())
		return i;
	nVertex = --m_free;
	Vertex (nVertex)->Clear ();
	Vertex (nVertex)->Backup (opAdd);
	nVertices [i] = nVertex;
	Count ()++;
	}
return count; 

#else //USE_FREELIST

if (Count () + count > MAX_VERTICES)
	return 0;
for (ushort i = 0; i < count; i++)
	nVertices [i] = Count () + i;
undoManager.Begin (udVertices);
Count () += count;
undoManager.End ();
return count;

#endif //USE_FREELIST
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Delete (ushort nDelVert)
{
#if USE_FREELIST

Vertex (nDelVert)->Backup (opDelete);
m_free += (int) nDelVert;
Count ()--;

#else //USE_FREELIST

undoManager.Begin (udVertices);
if (nDelVert < --Count ()) {
	*Vertex (nDelVert) = *Vertex (Count ());
	segmentManager.UpdateVertices (Count (), nDelVert);
undoManager.End ();
return Count ();

#endif //USE_FREELIST
}

// ----------------------------------------------------------------------------- 

void CVertexManager::DeleteUnused (void)
{
UnmarkAll (NEW_MASK);
// mark all used verts
segmentManager.MarkAll (NEW_MASK);
undoManager.Begin (udVertices);
for (CVertexIterator vi; vi; vi++)
	if (!(vi->Status () & NEW_MASK))
		Delete (vi.Index ()); 
undoManager.End ();
}

// ----------------------------------------------------------------------------- 

CVertex* CVertexManager::Find (CDoubleVector v)
{
for (CVertexIterator vi; vi; vi++)
	if (*vi == v)
		return &(*vi);
return null;
}

// ----------------------------------------------------------------------------- 

void CVertexManager::SetIndex (void)
{
int j = 0;
for (CVertexIterator vi; vi; vi++)
	vi->Index () = j++;
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

void CVertexManager::MarkAll (byte mask)
{
for (CVertexIterator vi; vi; vi++)
	vi->Status () |= MARKED_MASK; 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::UnmarkAll (byte mask)
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
