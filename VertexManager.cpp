// ----------------------------------------------------------------------------- 

void CVertexManager::Delete (short nDelVert)
{
	short nVertex, nSegment; 

undoManager.SetModified (true); 
// fill in gap in vertex array and status
if (nDelVert < --m_nCount) {
	m_vertices [nDelVert] = m_vertices [m_nCount];
	segmentManager.UpdateVertex (m_nCount, nDelVert);
	}
}

// ----------------------------------------------------------------------------- 
// DeleteUnusedVertices()
//
// ACTION - Deletes unused vertices
// ----------------------------------------------------------------------------- 

void CVertexManager::DeleteUnusedVertices (void)
{
	short nVertex, nSegment, point; 

for (nVertex = 0; nVertex < VertCount (); nVertex++)
	vertexManager.Status (nVertex) &= ~NEW_MASK; 
// mark all used verts
CSegment *segP = Segments (0);
for (nSegment = 0; nSegment < SegCount (); nSegment++, segP++)
	for (point = 0; point < 8; point++)
		vertexManager.Status (segP->m_info.verts [point]) |= NEW_MASK; 
for (nVertex = VertCount () - 1; nVertex >= 0; nVertex--)
	if (!(vertexManager.Status (nVertex) & NEW_MASK))
		DeleteVertex(nVertex); 
}

// ----------------------------------------------------------------------------- 
