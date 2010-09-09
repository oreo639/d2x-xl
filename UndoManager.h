#ifndef __undoman_h
#define __undoman_h

#include "mine.h"

#define MAX_UNDOS			500
#define DETAIL_BACKUP	0

//------------------------------------------------------------------------------

typedef struct tUndoBuffer {
	tUndoBuffer	*prevBuf;
	tUndoBuffer	*nextBuf;
	CMineData	undoBuffer;
} tUndoBuffer;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#if DETAIL_BACKUP

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

#else //DETAIL_BACKUP

template< class _T >
class CUndoItem {
	private:
		_T*	m_backup;
		_T*	m_source;
		int	m_length;
		int*	m_sourceLength;
		uint	m_nId; // used by undo manager

		int Create (_T* source, int length) {
			if (m_backup != null) 
				return -1;
			if (length > 0) {
				m_length = length;
				m_backup = new _T [length];
				if (m_backup == null) {
					m_length = -1;
					return 0;
					}
				}
			m_source = source;
			memcpy (m_backup, m_source, m_length * sizeof (_T));
			return 1;
			}

	public:
		bool Backup (_T* source, int* length) {
			int i = Create (source, *length);
			if (i < 0)
				return false;
			if (i > 0)
				m_sourceLength = length;
			return true;
			}

		inline bool Backup (_T* source, int length = 1) {
			m_sourceLength = null;
			return Create (source, length) >= 0;
			}

		bool Restore (void) {
			if (m_length < 0)
				return false;
			if (m_backup != null)
				memcpy (m_source, m_backup, m_length * sizeof (_T));
			if (m_sourceLength != null)
				*m_sourceLength = m_length;
			return true;
			}	

		void Destroy (void) {
			if (m_backup != null) {
				delete[] m_backup;
				Reset ();
				}
			}

		void Reset (void) {
			m_backup = null;
			m_source = null;
			m_length = -1;
			m_sourceLength = null;
			}

		bool Diff (void) {
			if (m_length < 0)
				return false;
			if (m_length != *m_sourceLength)
				return true;
			return memcmp (m_backup, m_source, m_length * sizeof (_T)) != 0;
			}

		bool Cleanup (void) {
			if (!Diff ())
				Destroy ();
			return !Empty ();
			}

		inline bool Empty (void) { return m_backup == null; }

		CUndoItem () : m_backup (null), m_source (null), m_length (-1), m_sourceLength (null) {}

		~CUndoItem () { Destroy (); }
	};

#endif //DETAIL_BACKUP

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
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
	udVariableLights = 512,
	udStaticLight = 1024,
	udDynamicLight = 2048,
	udLight = 0xE00,
	udAll = 0xFFF
} eUndoFlags;

//------------------------------------------------------------------------------

class CUndoData {
public:
	CUndoItem<CVertex>				m_vertices;
	CUndoItem<CSegment>				m_segments;
	CUndoItem<CMatCenter>			m_robotMakers;
	CUndoItem<CMatCenter>			m_equipMakers;
	CUndoItem<CWall>					m_walls;
	CUndoItem<CDoor>					m_doors;
	CUndoItem<CTrigger>				m_triggers [2];
	CUndoItem<CReactorTrigger>		m_reactorTriggers;
	CUndoItem<CGameObject>			m_objects;
	CUndoItem<CRobotInfo>			m_robotInfo;
	CUndoItem<CLightDeltaIndex>	m_deltaIndices;
	CUndoItem<CLightDeltaValue>	m_deltaValues;
	CUndoItem<CVariableLight>		m_variableLights;
	CUndoItem<CFaceColor>			m_faceColors;
	CUndoItem<CTextureColor>		m_textureColors;
	CUndoItem<CVertexColor>			m_vertexColors;
	CUndoItem<CSecretExit>			m_secretExit;
	CUndoItem<CReactorData>			m_reactorData;

	uint m_nId;

	inline uint& Id (void) { return m_nId; }

	void Backup (eUndoFlags dataFlags);

	bool Cleanup (void);

	void Restore (void);

	void Destroy ();

	void Reset (void) { memset (this, 0, sizeof (*this)); }

	CUndoData () : m_nId (0) { Reset (); }

	~CUndoData () { Destroy (); }
	};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class CUndoManager
{
	private:
		CUndoData	m_buffer [MAX_UNDOS];
		int			m_nHead;
		int			m_nTail;
		int			m_nCurrent;
		CUndoData	m_current;
		//CUndoItem*	m_head;
		//CUndoItem*	m_tail;
		//CUndoItem*	m_current;
		int			m_maxSize;
		int			m_size;
		int			m_delay;
		int			m_enabled;
		int			m_nModified;
		uint			m_nId;

	public:
		inline CUndoData* Head (void) { return (m_nHead < 0) ? null : &m_buffer [m_nHead]; }

		inline CUndoData* Tail (void) { return (m_nTail < 0) ? null : &m_buffer [m_nTail]; }

		inline CUndoData* Current (void) { return (m_nCurrent < 0) ? null : &m_buffer [m_nCurrent]; }

		int Backup (CGameItem* parent, eEditType editType);

		void Backup (void);

		inline uint& Id (void) { return m_nId; }

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

		void Begin (eUndoFlags dataFlags);

		void End (void);

		void Unroll (void);

		CUndoManager (int maxSize = 100);

		~CUndoManager ();

private:
		void Append (void);
};

extern CUndoManager undoManager;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif //__undoman_h
