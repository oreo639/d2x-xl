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

class CUndoManager
{
	private:
		CGameItem*	m_head;
		CGameItem*	m_tail;
		CGameItem*	m_current;
		int			m_maxSize;
		int			m_size;
		int			m_delay;
		int			m_enabled;
		int			m_nModified;
		int			m_nId;

	public:
		inline CGameItem* Head (void) { return m_head; }

		inline CGameItem* Tail (void) { return m_tail; }

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
