// DialHeap.cpp
// Heap management for Dijkstra Address Calculation Sort shortest path determination

#include "error.h"
#include "dialheap.h"
#include "error.h"
#include "descent.h"

#define SPARSE_RESET 1
#define FAST_SCAN 1

//-----------------------------------------------------------------------------

bool CDialHeap::Create (int16_t nNodes)
{
ENTER (0, 0);
Destroy ();
m_nNodes = nNodes;
if (!(m_index.Create (65536) && m_dirtyIndex.Create (65536) && m_cost.Create (nNodes) && m_dirtyCost.Create (nNodes) && m_links.Create (nNodes) && m_pred.Create (nNodes) && m_edge.Create (nNodes))) {
	Destroy ();
	RETVAL (false)
	}
m_index.Clear (0xFF);
m_cost.Clear (0xFF);
RETVAL (true)
}

//-----------------------------------------------------------------------------

void CDialHeap::Destroy (void)
{
ENTER (0, 0);
m_index.Destroy ();
m_dirtyIndex.Destroy ();
m_cost.Destroy ();
m_dirtyCost.Destroy ();
m_links.Destroy ();
m_pred.Destroy ();
m_edge.Destroy ();
m_route.Destroy ();
m_nNodes = 0;
RETURN
}

//-----------------------------------------------------------------------------

void CDialHeap::Reset (void)
{
ENTER (0, 0);
#if SPARSE_RESET
#	if 0 //DBG
for (uint32_t i = 0, j = m_dirtyIndex.ToS (); i < j; i++)
	m_index [m_dirtyIndex [i]] = -1;
for (uint32_t i = 0, j = m_dirtyCost.ToS (); i < j; i++)
	m_cost [m_dirtyCost [i]] = 0xFFFFFFFF;
#	else
int16_t* pIndex = m_index.Buffer ();
uint16_t* dirtyIndexP = m_dirtyIndex.Buffer ();
uint32_t i;
for (i = m_dirtyIndex.ToS (); i; i--, dirtyIndexP++)
	pIndex [*dirtyIndexP] = -1;

uint32_t* costP = m_cost.Buffer ();
uint32_t* dirtyCostP = m_dirtyCost.Buffer ();
for (i = m_dirtyCost.ToS (); i; i--, dirtyCostP++)
	costP [*dirtyCostP] = 0xFFFFFFFF;
#	endif
m_dirtyIndex.Reset ();
m_dirtyCost.Reset ();
#else
m_index.Clear (0xFF);
#endif
m_cost.Clear (0xFF);
m_nIndex = 0;
RETURN
}

//-----------------------------------------------------------------------------

void CDialHeap::Setup (int16_t nNode)
{
ENTER (0, 0);
Reset ();
Push (nNode, -1, -1, 0);
RETURN
}

//-----------------------------------------------------------------------------

bool CDialHeap::Push (int16_t nNode, int16_t nPredNode, int16_t nEdge, uint32_t nNewCost)
{
ENTER (0, 0);
if (!m_index.Buffer ()) { // -> Bug!
	PrintLog (0, "Dial heap has not been initialized!\n");
	RETVAL (false)
	}

	uint32_t nOldCost = m_cost [nNode] & ~0x80000000;

if (nNewCost >= nOldCost)
	RETVAL (false)

#if DBG
if (!nNewCost)
	BRP;
#endif

	uint16_t nIndex = uint16_t (nNewCost & 0xFFFF);

if (nOldCost < 0x7FFFFFFF) {	// node already in heap with higher cost, so unlink
	int32_t h = uint16_t (nOldCost & 0xFFFF);
	for (int32_t i = m_index [h], j = -1; i >= 0; j = i, i = m_links [i]) {
		if (i == nNode) {
			if (j < 0)
				m_index [h] = m_links [i];
			else
				m_links [j] = m_links [i];
			break;
			}
		}
	}
#if SPARSE_RESET
else
	m_dirtyCost.Push (nNode);
if (0 > m_index [nIndex])
	m_dirtyIndex.Push (nIndex);
#endif
m_links [nNode] = m_index [nIndex];
m_cost [nNode] = nNewCost;
m_index [nIndex] = nNode;
m_pred [nNode] = nPredNode;
m_edge [nNode] = nEdge;
RETVAL (true)
}

//-----------------------------------------------------------------------------

int32_t CDialHeap::Scan (int32_t nStart, int32_t nLength)
{
ENTER (0, 0);
if (!m_index.Buffer ()) { // -> Bug!
	PrintLog (0, "Dial heap has not been initialized!\n");
	RETVAL (-1)
	}

int16_t *pBuffer = m_index.Buffer (nStart);
for (; nLength; nLength--, pBuffer++)
	if (*pBuffer >= 0)
		RETVAL (int32_t (pBuffer - m_index.Buffer ()))
RETVAL (-1)
}

//-----------------------------------------------------------------------------

int16_t CDialHeap::Pop (uint32_t& nCost)
{
ENTER (0, 0);
#if FAST_SCAN
int32_t i = Scan (m_nIndex, m_index.Length () - m_nIndex); // scan beginning at m_nIndex to the end of the buffer
if (i < 0)
	i = Scan (0, m_nIndex); // wrap around and scan from the end of the buffer to m_nIndex
if (i < 0)
	RETVAL (-1)
m_nIndex = uint16_t (i);
int16_t nNode = m_index [m_nIndex];
#if DBG
if (nNode < 0) // bug; should never happen
	RETVAL (-1) 
#endif
m_index [m_nIndex] = m_links [nNode];
nCost = m_cost [nNode];
m_cost [nNode] |= 0x80000000;
RETVAL (nNode)
#else
	int16_t	nNode;

for (int32_t i = 65536; i; i--) {
	if (0 <= (nNode = m_index [m_nIndex])) {
		m_index [m_nIndex] = m_links [nNode];
		nCost = m_cost [nNode];
		m_cost [nNode] |= 0x80000000;
		RETVAL (nNode)
		}
	m_nIndex++;
	}
RETVAL (-1)
#endif
}

//-----------------------------------------------------------------------------

int16_t CDialHeap::RouteLength (int16_t nNode)
{
ENTER (0, 0);
if (!m_pred.Buffer ()) { // -> Bug!
	PrintLog (0, "Dial heap has not been initialized!\n");
	RETVAL (0)
	}

	int16_t	h = nNode, i = 1;

#if 1
for (; i; i++) {
	h = m_pred [h];
	if (h >= m_nNodes) {
		PrintLog (0, "router heap corrupted (invalid segment %d)\n", h);
		RETVAL (-1);
		}
	//if (nNode == 2136)
	//	PrintLog (0, "RouteLength [%d] = %d\n", i, h);
	if (h < 0)
		RETVAL (i);
	}
#else
while (0 <= (h = m_pred [h]))
	i++;
#endif
RETVAL (i)
}

//-----------------------------------------------------------------------------

int16_t CDialHeap::BuildRoute (int16_t nNode, int32_t bReverse, tPathNode* route)
{
ENTER (0, 0);
if (!m_pred.Buffer () || !m_edge.Buffer ()) { // -> Bug!
	PrintLog (0, "Dial heap has not been initialized!\n");
	RETVAL (0)
	}

	int16_t	h = RouteLength (nNode);

if (!route) {
	if (!m_route.Buffer ())
		m_route.Create (m_nNodes);
	route = m_route.Buffer ();
	}

if (bReverse) {
	for (int32_t i = 0; i < h; i++) {
		route [i].nNode = nNode;
		route [i].nEdge = m_edge [nNode];
		nNode = m_pred [nNode];
		}
	}
else {
	for (int32_t i = h - 1; i >= 0; i--) {
		route [i].nNode = nNode;
		route [i].nEdge = m_edge [nNode];
		nNode = m_pred [nNode];
		}
	}
RETVAL (h)
}

//-----------------------------------------------------------------------------

