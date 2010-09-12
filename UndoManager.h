#ifndef __undoman_h
#define __undoman_h

#include "mine.h"

#define DLE_MAX_UNDOS			500
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
				memcpy (m_backup, source, m_length * sizeof (_T));
				}
			m_source = source;
			return 1;
			}

	public:
		bool Backup (_T* source, int* length) {
			int i = Create (source, *length);
			if (i == 0)
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
			if (m_backup == null)
				return false;
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
	udMatCenters = 4,
	udWalls = 8,
	udDoors = 16,
	udTriggers = 32,
	udObjects = 64,
	udRobots = 128,
	udVariableLights = 256,
	udStaticLight = 512,
	udDynamicLight = 1024,
	udLight = 0x700,
	udAll = 0x7FF
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
	CSelection							m_selections [2];

	bool m_bSelections;
	uint m_nId;

	inline uint& Id (void) { return m_nId; }

	void Backup (int dataFlags);

	bool Cleanup (void);

	void Restore (void);

	void Destroy (void);

	void Reset (void);

	CUndoData () : m_nId (0), m_bSelections (false) { }

	~CUndoData () { Destroy (); }
	};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class CBufPtr {
	public:
		int	m_size;
		int	m_index;

		void Setup (int size) { 
			if (m_index >= (m_size = size))
				m_index = m_size - 1;
			}

		const CBufPtr& operator= (CBufPtr& other) {
			m_index = other.m_index;
			m_size = other.m_size;
			return *this;
			}

		const CBufPtr& operator= (const int i) { 
			m_index = i; 
			return *this;
			}

		const bool operator== (const int i) { return m_index == i; }

		const bool operator== (const CBufPtr& other) { return m_index == other.m_index; }

		const bool operator!= (const int i) { return m_index != i; }

		const bool operator!= (const CBufPtr& other) { return m_index != other.m_index; }

		const bool operator< (const int i) { return m_index < i; }

		const bool operator> (const int i) { return m_index > i; }

		const bool operator< (const CBufPtr& other) { return m_index < other.m_index; }

		const bool operator> (const CBufPtr& other) { return m_index > other.m_index; }

		const int operator++ (int) { 
			int i = m_index;
			m_index = ++m_index % m_size; 
			return i;
			}

		const int operator++ () { return m_index = ++m_index % m_size; }

		const int operator-- (int) { 
			int i = m_index;
			m_index = ((m_index == 0) ? m_size : m_index) - 1; 
			return i;
			}

		const int operator-- () { return m_index = ((m_index == 0) ? m_size : m_index) - 1; }

		const int operator-= (const int i) { 
			m_index -= i; 
			if (m_index < 0)
				m_index += m_size;
			return m_index;
			}

		const int operator+= (const int i) { 
			m_index += i; 
			if (m_index >= m_size)
				m_index -= m_size;
			return m_index;
			}

		const int operator- (const int i) { 
			int h = m_index - i; 
			if (h < 0)
				h += m_size;
			return h;
			}

		const int operator+ (const int i) { 
			int h = m_index + i; 
			if (h >= m_size)
				h -= m_size;
			return h;
			}

		const int operator* (void) { return m_index; }

		CBufPtr (int size = DLE_MAX_UNDOS, int index = -1) : m_size (size), m_index (index) {}
	};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class CUndoManager
{
	private:
		CUndoData	m_buffer [DLE_MAX_UNDOS];
		CBufPtr		m_nHead;
		CBufPtr		m_nTail;
		CBufPtr		m_nCurrent;
		CUndoData	m_current;
		//CUndoItem*	m_head;
		//CUndoItem*	m_tail;
		//CUndoItem*	m_current;
		int			m_nMaxSize;
		int			m_size;
		int			m_delay;
		int			m_enabled;
		int			m_mode;
		int			m_nModified;
		uint			m_nId;

	public:
		inline CUndoData* Head (void) { return (m_nHead < 0) ? null : &m_buffer [*m_nHead]; }

		inline CUndoData* Tail (void) { return (m_nTail < 0) ? null : &m_buffer [*m_nTail]; }

		inline CUndoData* Current (void) { return (m_nCurrent < 0) ? null : &m_buffer [*m_nCurrent]; }

		int Backup (CGameItem* parent, eEditType editType);

		void Backup (void);

		inline uint& Id (void) { return m_nId; }

		bool Undo ();

		bool Redo ();

		void Truncate ();

		void Reset ();

		bool Revert ();

		int Count (void);

		int Enable (int bEnable);

		int SetMaxSize (int maxSize);

		inline int MaxSize (void) { return m_nMaxSize; }

		void SetModified (bool bModified);

		void Begin (int dataFlags);

		void End (void);

		void Unroll (void);

		CUndoManager (int maxSize = 100);

		~CUndoManager ();

private:
		void Append (void);

		inline int Advance (int i) { return (i == m_nMaxSize - 1) ? 0 : i + 1; }

		inline int Reverse (int i) { return (i == 0) ? m_nMaxSize - 1 : i - 1; }
};

extern CUndoManager undoManager;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#endif //__undoman_h
