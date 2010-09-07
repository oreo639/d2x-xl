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
		eEditType	m_editType;
		int			m_nBackupId; // used by undo manager

		inline int& BackupId (void) { return m_nBackupId; }

		inline CGameItem* Parent (void) { return m_parent; }

		inline CGameItem* Item (void) { return m_item; }

		inline void Undo (void) { m_item->Undo (); }

		inline void Redo (void) { m_item->Redo (); }

		void Setup (CGameItem* parent, eEditType editType, int nBackupId);

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

		bool SetModified (bool bModified);

		void ResetModified (bool bRevert);

		CUndoManager (int maxSize = 100);

		~CUndoManager ();

private:
		void Append (void);
};

extern CUndoManager undoManager;

//------------------------------------------------------------------------------

#endif //__undoman_h
