#include "TriggerManager.h"
#include "WallManager.h"
#include "SegmentManager.h"
#include "selection.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

CSelection current, other;

// -----------------------------------------------------------------------------

void CSelection::Setup (short nSegment = -1, short nSide = -1, short nLine = -1, short nPoint = -1) 
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

CSelection::Get (short& nSegment, short& nSide)
{
if (nSegment < 0)
	nSegment = m_nSegment;
if (nSide < 0)
	nSide = m_nSide;
}

// -----------------------------------------------------------------------------

CSelection::Segment (void)
{
return segmentManager.GetSegment (m_nSegment);
}

// -----------------------------------------------------------------------------

CSelection::Side (void)
{
return segmentManager.GetSide (m_nSegment, m_nSide);
}

// -----------------------------------------------------------------------------

CSelection::Wall (void)
{
return segmentManager.GetWall (m_nSegment, m_nSide);
}

// -----------------------------------------------------------------------------

CSelection::Object (void)
{
return objectManager.GetObject (m_nObject);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

