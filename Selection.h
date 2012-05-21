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
		m_nLine (DEFAULT_LINE),
		m_nPoint (DEFAULT_POINT),
		m_nObject (DEFAULT_OBJECT)
	{}

	short m_nLine;
	short m_nPoint;
	short m_nObject;
	short	m_nIndex;

	void Get (CSideKey& key);

	void Get (short& nSegment, short& nSide);

	CSegment* Segment (void);

	short Child (void);

	CSegment* ChildSeg (void);

	CSide* Side (void);

	CSide* OppositeSide (void);

	CWall* Wall (void);

	CTrigger* Trigger (void);

	CGameObject* Object (void);

	CVertex* Vertex (void);

	inline ushort VertexId (void) { return Segment ()->VertexId (m_nSide, m_nPoint); }

	CColor* LightColor (void);

	void Setup (short nSegment = -1, short nSide = -1, short nLine = -1, short nPoint = -1);

	void Reset (void) {
		m_nSegment = 0;
		m_nSide = DEFAULT_SIDE;
		m_nLine = DEFAULT_LINE,
		m_nPoint = DEFAULT_POINT,
		m_nObject = DEFAULT_OBJECT;
		}

	inline void Setup (CSideKey key) { Setup (key.m_nSegment, key.m_nSide); }

	void Fix (short nSegment);

	inline short& Index (void) { return m_nIndex; }
};

extern CSelection selections [3];
extern CSelection* current;
extern CSelection* other;
extern CSelection* nearest;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif //__selection_h
