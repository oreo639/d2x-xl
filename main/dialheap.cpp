// DialHeap.cpp
// Heap management for Dijkstra Address Calculation Sort shortest path determination

#include "dialheap.h"

//-----------------------------------------------------------------------------

bool CDialHeap::Create (short nNodes)
{
Destroy ();
m_nNodes = nNodes;
if (!(m_index.Create (65536) && m_cost.Create (nNodes) && m_links.Create (nNodes) && m_pred.Create (nNodes) && m_edge.Create (nNodes)))
	return false;
return true;
}

//-----------------------------------------------------------------------------

void CDialHeap::Destroy (void)
{
m_index.Destroy ();
m_cost.Destroy ();
m_links.Destroy ();
m_pred.Destroy ();
m_edge.Destroy ();
m_route.Destroy ();
}

//-----------------------------------------------------------------------------

void CDialHeap::Reset (void)
{
m_index.Clear (0xFF);
m_cost.Clear (0xFF);
m_nIndex = 0;
}

//-----------------------------------------------------------------------------

void CDialHeap::Setup (short nNode)
{
Reset ();
Push (nNode, -1, -1, 0);
}

//-----------------------------------------------------------------------------

bool CDialHeap::Push (short nNode, short nPredNode, short nEdge, uint nNewCost)
{
	uint nOldCost = m_cost [nNode];

if (nNewCost >= nOldCost)
	return false;

#if DBG
if (!nNewCost)
	nNewCost = nNewCost;
#endif

	ushort nIndex = ushort (nNewCost & 0xFFFF);

if (nOldCost < 0xFFFF) {	// node already in heap with higher m_cost, so unlink
	int h = ushort (nOldCost & 0xFFFF);
	for (int i = m_index [h], j = -1; i >= 0; j = i, i = m_links [i]) {
		if (i == nNode) {
			if (j < 0)
				m_index [h] = m_links [i];
			else
				m_links [j] = m_links [i];
			break;
			}
		}
	}
m_links [nNode] = m_index [nIndex];
m_index [nIndex] = nNode;
m_cost [nNode] = nNewCost;
m_pred [nNode] = nPredNode;
m_edge [nNode] = nEdge;
return true;
}

//-----------------------------------------------------------------------------

short CDialHeap::Pop (uint& nCost)
{
	short	nNode;

for (int i = 65536; i; i--) {
	if (0 <= (nNode = m_index [m_nIndex])) {
		m_index [m_nIndex] = m_links [nNode];
		nCost = m_cost [nNode];
		m_cost [nNode] = 0;
		return nNode;
		}
	m_nIndex++;
	}
return -1;
}

//-----------------------------------------------------------------------------

short CDialHeap::RouteLength (short nNode)
{
	short	h = nNode, i = 0;

while (0 <= (h = m_pred [h]))
	i++;
return i + 1;
}

//-----------------------------------------------------------------------------

short CDialHeap::BuildRoute (short nNode, int bReverse, tPathNode* route)
{
	short	h = RouteLength (nNode);

if (!route) {
	if (!m_route.Buffer ())
		m_route.Create (m_nNodes);
	route = m_route.Buffer ();
	}

if (bReverse) {
	for (int i = 0; i < h; i++) {
		route [i].nNode = nNode;
		route [i].nEdge = m_edge [nNode];
		nNode = m_pred [nNode];
		}
	}
else {
	for (int i = h - 1; i >= 0; i--) {
		route [i].nNode = nNode;
		route [i].nEdge = m_edge [nNode];
		nNode = m_pred [nNode];
		}
	}
return h;
}

//-----------------------------------------------------------------------------

