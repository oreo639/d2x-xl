
#include "Mine.h"
#include "dle-xp.h"

CVertexManager vertexManager;

// ----------------------------------------------------------------------------- 

bool CVertexManager::Full (void) 
{ 
return Count () >= MAX_VERTICES; 
}

// ----------------------------------------------------------------------------- 

ushort CVertexManager::Add (ushort* nVertices, ushort count, bool bUndo) 
{ 

#if USE_FREELIST

ushort nVertex;
for (ushort i = 0; i < count; i++) {
	if (m_free.Empty ())
		return i;
	nVertex = --m_free;
	Vertex (nVertex)->Clear ();
	if (bUndo)
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

void CVertexManager::Delete (ushort nDelVert, bool bUndo)
{
#if USE_FREELIST

if (bUndo)
	Vertex (nDelVert)->Backup (opDelete);
m_free += (int) nDelVert;
Count ()--;

#else //USE_FREELIST

undoManager.Begin (udVertices);
if (nDelVert < --Count ()) {
	Vertex (nDelVert)->v = Vertex (Count ())->v;
	Vertex (nDelVert)->Status () = 0;
	segmentManager.UpdateVertices (Count (), nDelVert);
	}
undoManager.End ();

#endif //USE_FREELIST
}

// ----------------------------------------------------------------------------- 

void CVertexManager::DeleteUnused (void)
{
UnmarkAll (NEW_MASK);
// mark all used verts
segmentManager.MarkAll (NEW_MASK);
undoManager.Begin (udVertices);
for (CVertexIterator vi (-Count ()); vi; ) {
	--vi;
	if (!(vi->Status () & NEW_MASK))
		Delete (vi.Index ()); 
	}
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

void CVertexManager::Read (CFileManager* fp)
{
for (int i = 0; i < Count (); i++)
	m_vertices [i].Read (fp);
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Write (CFileManager* fp)
{
m_info.offset = fp->Tell ();
for (CVertexIterator vi; vi; vi++)
	vi->Write (fp);
}

// ----------------------------------------------------------------------------- 

void CVertexManager::MarkAll (byte mask)
{
for (CVertexIterator vi; vi; vi++)
	vi->Status () |= mask; 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::UnmarkAll (byte mask)
{
for (CVertexIterator vi; vi; vi++)
	vi->Status () &= ~mask; 
}

// ----------------------------------------------------------------------------- 

void CVertexManager::Clear (void)
{
for (CVertexIterator vi; vi; vi++)
	vi->Clear ();
}

// ----------------------------------------------------------------------------- 
