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
	private:
		short m_nEdge;
		short m_nPoint;
		short m_nObject;
		short	m_nIndex;

	public:
		CSelection() :
			CSideKey (0, DEFAULT_SIDE),
			m_nEdge (DEFAULT_LINE),
			m_nPoint (DEFAULT_POINT),
			m_nObject (DEFAULT_OBJECT)
		{}

		void Get (CSideKey& key);

		void Get (short& nSegment, short& nSide);

		CSegment* Segment (void);

		inline short SegmentId (void) { return m_nSegment; }

		inline short SetSegmentId (short nSegment) { return m_nSegment = nSegment; }

		short ChildId (void);

		CSegment* ChildSeg (void);

		CSide* Side (void);

		inline short SideId (void) { return m_nSide; }

		inline short SetSideId (short nSide) { return m_nSide = nSide; }

		CSide* OppositeSide (void);

		CWall* Wall (void);

		CTrigger* Trigger (void);

		CGameObject* Object (void);

		inline short ObjectId (void) { return m_nObject; }

		inline short SetObjectId (short nObject) { return m_nObject = nObject; }

		CVertex* Vertex (void);

		inline ushort VertexId (void) { return Segment ()->VertexId (m_nSide, m_nPoint); }

		inline short Point (void) { return m_nPoint; }

		inline short SetPoint (short nPoint) { return m_nEdge = m_nPoint = nPoint; }

		inline short Edge (void) { return m_nEdge; }

		inline short SetEdge (short nEdge) { return m_nPoint = m_nEdge = nEdge; }

		CColor* LightColor (void);

		void Setup (short nSegment = -1, short nSide = -1, short nLine = -1, short nPoint = -1);

		void Reset (void) {
			m_nSegment = 0;
			m_nSide = DEFAULT_SIDE;
			m_nEdge = DEFAULT_LINE,
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
