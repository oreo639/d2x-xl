#ifndef __undoman_h
#define __undoman_h

#include "mine.h"

#define MAX_UNDOS		100000

//------------------------------------------------------------------------------

typedef struct tUndoBuffer {
	tUndoBuffer	*prevBuf;
	tUndoBuffer	*nextBuf;
	CMineData	undoBuffer;
} tUndoBuffer;

//------------------------------------------------------------------------------

class CUndoItem {
	public:
		CGameItem*	m_item;
		CGameItem*	m_parent;
		uint			m_nId; // used by undo manager

		inline uint& Id (void) { return m_nId; }

		inline CGameItem* Parent (void) { return m_parent; }

		inline CGameItem* Item (void) { return m_item; }

		inline void Undo (void) { m_item->Undo (); }

		inline void Redo (void) { m_item->Redo (); }

		void Setup (CGameItem* item, CGameItem* parent, eEditType editType, int nBackupId);

	};

//------------------------------------------------------------------------------

typedef enum {
	udVertices = 1,
	udSegments = 2,
	udRobotMakers = 4,
	udEquipMakers = 8,
	udWalls = 16,
	udDoors = 32,
	udTriggers = 64,
	udObjects = 128,
	udRobots = 256,
	udVariableLights = 1024,
	udStaticLight = 2048,
	udDynamicLight = 4096
} undoData;


class CUndoData {
public:
	CVertex*				m_vertices;
	int					m_nVertices;
	CSegment*			m_segments;
	int					m_nSegments;
	CMatCenter*			m_matCenters [2];
	int					m_nMatCenters [2];
	CWall*				m_walls;
	int					m_nWalls;
	CDoor*				m_doors;
	int					m_nDoors;
	CTrigger*			m_triggers;
	int					m_nTriggers;
	CGameObject*		m_objects;
	int					m_nObjects;
	CRobotInfo*			m_robotInfo;
	int					m_nRobots;
	CLightDeltaIndex*	m_deltaIndices;
	int					m_nDeltaIndices;
	CLightDeltaValue*	m_deltaValues;
	int					m_nDeltaValues;
	CVariableLight*	m_variableLights;
	int					m_nVariableLights;
	CFaceColor*			m_faceColors;
	int					m_nFaceColors;
	CTextureColor*		m_textureColors;
	int					m_nTextureColors;
	CVertexColor*		m_vertexColors;
	int					m_nVertexColors;
};

//------------------------------------------------------------------------------

class CUndoManager
{
	private:
		CUndoItem	m_buffer [MAX_UNDOS];
		int			m_nHead;
		int			m_nTail;
		int			m_nCurrent;
		//CUndoItem*	m_head;
		//CUndoItem*	m_tail;
		//CUndoItem*	m_current;
		int			m_maxSize;
		int			m_size;
		int			m_delay;
		int			m_enabled;
		int			m_nModified;
		int			m_nId;

	public:
		inline CUndoItem* Head (void) { return (m_nHead < 0) ? null : &m_buffer [m_nHead]; }

		inline CUndoItem* Tail (void) { return (m_nTail < 0) ? null : &m_buffer [m_nTail]; }

		inline CUndoItem* Current (void) { return (m_nCurrent < 0) ? null : &m_buffer [m_nCurrent]; }

		int Backup (CGameItem* parent, eEditType editType);

		inline int Id (void) { return m_nId; }

		bool Update (bool bForce = false);

		bool Undo ();

		bool Redo ();

		void Truncate ();

		void Reset ();

		bool Revert ();

		void Delay (bool bDelay);

		int Count ();

		int Enable (int bEnable);

		int SetMaxSize (int maxSize);

		inline int GetMaxSize (void) { return m_maxSize; }

		inline void Lock () { Delay (true); }

		inline void Unlock () { Delay (false); }

		void SetModified (bool bModified);

		void Begin (void);

		void End (void);

		void Unroll (void);

		CUndoManager (int maxSize = 100);

		~CUndoManager ();

private:
		void Append (void);
};

extern CUndoManager undoManager;

//------------------------------------------------------------------------------

#endif //__undoman_h
