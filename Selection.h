#ifndef __selection_h
#define __selection_h

#include "Vertex.h"
#include "Trigger.h"
#include "Wall.h"
#include "Segment.h"
#include "GameObject.h"

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

	void Get (CSideKey& key);
	void Get (short& nSegment, short& nSide);
	CSegment* Segment (void);
	short Child (void);
	CSegment* ChildSeg (void);
	CSide* Side (void);
	CWall* Wall (void);
	CGameObject* Object (void);
	CColor* CurrLightColor (void);
};

extern CSelection selections [2];
extern CSelection& current;
extern CSelection& other;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif //__selection_h
