#ifndef __selection_h
#define __selection_h

#include "TriggerManager.h"
#include "WallManager.h"
#include "SegmentManager.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CSelection {
public:
	CSelection() :
		m_nSegment(0),
		m_nSide(DEFAULT_SIDE),
		m_nLine(DEFAULT_LINE),
		m_nPoint(DEFAULT_POINT),
		m_nObject(DEFAULT_OBJECT)
	{}

	short m_nSegment;
	short m_nSide;
	short m_nLine;
	short m_nPoint;
	short m_nObject;

	Get (short& nSegment, short& nSide);
	Segment (void);
	Side (void);
	Wall (void);
	Object (void);
};

extern CSelection current, other;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif //__selection_h
