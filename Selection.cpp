
#include "selection.h"
#include "TriggerManager.h"
#include "WallManager.h"
#include "SegmentManager.h"
#include "ObjectManager.h"
#include "VertexManager.h"
#include "LightManager.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CSelection selections [2];
CSelection* current = &selections [0];
CSelection* other = &selections [1];

// -----------------------------------------------------------------------------

void CSelection::Setup (short nSegment, short nSide, short nLine, short nPoint) 
{
if (nSegment >= 0) 
	m_nSegment = nSegment;
if (nSide >= 0) 
	m_nSide = nSide;
if (nLine >= 0) 
	m_nLine = nLine;
if (nPoint >= 0) 
	m_nPoint = nPoint;
}

// -----------------------------------------------------------------------------

void CSelection::Get (CSideKey& key)
{
if (key.m_nSegment < 0)
	key.m_nSegment = m_nSegment;
if (key.m_nSide < 0)
	key.m_nSide = m_nSide;
}

// -----------------------------------------------------------------------------

void CSelection::Get (short& nSegment, short& nSide)
{
if (nSegment < 0)
	nSegment = m_nSegment;
if (nSide < 0)
	nSide = m_nSide;
}

// -----------------------------------------------------------------------------

CSegment* CSelection::Segment (void)
{
return segmentManager.Segment (m_nSegment);
}

// -----------------------------------------------------------------------------

short CSelection::Child (void)
{
return Segment ()->Child (m_nSide);
}

// -----------------------------------------------------------------------------

CSegment* CSelection::ChildSeg (void)
{
short nChild = Child ();
return (nChild < 0) ? null : segmentManager.Segment (nChild);
}

// -----------------------------------------------------------------------------

CSide* CSelection::Side (void)
{
return segmentManager.Side (*this);
}

// -----------------------------------------------------------------------------

CWall* CSelection::Wall (void)
{
return segmentManager.Wall (*this);
}

// -----------------------------------------------------------------------------

CTrigger* CSelection::Trigger (void)
{
CWall* wallP = Wall ();
return (wallP == null) ? null : wallP->Trigger ();
}

// -----------------------------------------------------------------------------

CGameObject* CSelection::Object (void)
{
return objectManager.Object (m_nObject);
}

// -----------------------------------------------------------------------------

CVertex* CSelection::Vertex (void)
{
return Segment ()->Vertex (sideVertTable [m_nSide][m_nPoint]); 
}

// -----------------------------------------------------------------------------

CColor* CSelection::LightColor (void)
{ 
return lightManager.LightColor (*this); 
}

// -----------------------------------------------------------------------------

void CSelection::Fix (short nSegment)
{
if (m_nSegment != nSegment)
	return;
short nChild;
if ((nChild = Child ()) > -1) {
	m_nSegment = nChild;
	return;
	}
if ((nChild = Segment ()->Child (oppSideTable [m_nSide])) > -1) {
	m_nSegment = nChild;
	return;
	}
for (short nSide = 0; nSide < 6; nSide++)
	if ((nChild = Segment ()->Child (nSide)) >= 0) {
		m_nSegment = nChild;
		return;
		}
if (segmentManager.Count () == 1)
	return;
if (m_nSegment >= segmentManager.Count () - 1)
	m_nSegment--;
else
	m_nSegment++;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

