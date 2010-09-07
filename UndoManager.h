#ifndef __undoman_h
#define __undoman_h

#include "mine.h"

#define MAX_UNDOS		100

//------------------------------------------------------------------------------

typedef struct tUndoBuffer {
	tUndoBuffer	*prevBuf;
	tUndoBuffer	*nextBuf;
	CMineData	undoBuffer;
} tUndoBuffer;

//------------------------------------------------------------------------------

class CUndoItem {
	public:
		eEditType	m_editType;
		CGameItem*	m_item;
		CGameItem*	m_parent;
		CGameItem*	m_prev;
		CGameItem*	m_next;

		inline int& BackupId (void) { return m_nBackupId; }

		inline CGameItem* &Parent (void) { return m_parent; }

		inline CGameItem* &Prev (void) { return m_prev; }

		inline CGameItem* &Next (void) { return m_next; }

		inline void Link (CGameItem* pred) {
			m_prev = pred;
			if (pred != null) {
				m_next = pred->m_next;
				pred->m_next = m_prev;
				}
			}

		inline void Unlink (void) {
			if (m_prev != null)
				m_prev->m_next = m_next;
			if (m_next != null)
				m_next->m_prev = m_prev;
			m_prev = m_next = null;
			}
	};

//------------------------------------------------------------------------------

class CUndoManager
{
	private:
		CUndoItem	m_buffer [100000];
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
		inline CGameItem* Head (void) { return (m_nHead < 0) ? null : &m_buffer [m_head]; }

		inline CGameItem* Tail (void) { return (m_nTail < 0) ? null : &m_buffer [m_nTail]; }

		inline CGameItem* Current (void) { return (m_nCurrent < 0) ? null : &m_buffer [m_nCurrent]; }

		int Backup (CGameItem* parent, eItemType itemType, eEditType editType);

		inline int Id (void) { return m_nId; }

		bool Update (bool bForce = false);

		bool Undo ();

		bool Redo ();

		void Truncate ();

		void Reset ();

		bool Revert ();

		void Delay (bool bDelay);

		int UndoCount ();

		int Enable (int bEnable);

		int SetMaxSize (int maxSize);

		inline int GetMaxSize (void) { return m_maxSize; }

		inline void Lock () { Delay (true); }

		inline void Unlock () { Delay (false); }

		bool SetModified (bool bModified);

		void ResetModified (bool bRevert);

		CUndoManager (int maxSize = 100);

		~CUndoManager ();
};

extern CUndoManager undoManager;

//------------------------------------------------------------------------------

#endif //__undoman_h
