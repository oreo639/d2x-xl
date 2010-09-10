#ifndef __selection_h
#define __selection_h

#include "Types.h"
#include "Vertex.h"
#include "Trigger.h"
#include "Wall.h"
#include "Segment.h"
#include "GameObject.h"
#include "Light.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CSelection : public CSideKey {
public:
	CSelection() :
		CSideKey (0, DEFAULT_SIDE),
		m_nLine(DEFAULT_LINE),
		m_nPoint(DEFAULT_POINT),
		m_nObject(DEFAULT_OBJECT)
	{}

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
	CVertex* Vertex (void);
	CColor* LightColor (void);
	void Setup (short nSegment = -1, short nSide = -1, short nLine = -1, short nPoint = -1);
};

extern CSelection selections [2];
extern CSelection& current;
extern CSelection& other;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif //__selection_h
