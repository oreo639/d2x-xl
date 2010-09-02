#ifndef __undoman_h
#define __undoman_h

//------------------------------------------------------------------------------

typedef struct tUndoBuffer {
	tUndoBuffer	*prevBuf;
	tUndoBuffer	*nextBuf;
	MINE_DATA	undoBuffer;
} tUndoBuffer;

//------------------------------------------------------------------------------

class CUndoManager
{
	public:
		tUndoBuffer	*m_head;
		tUndoBuffer	*m_tail;
		tUndoBuffer	*m_current;
		int			m_maxSize;
		int			m_size;
		int			m_delay;
		int			m_enabled;
		int			m_nModified;

		CUndoList (int maxSize = 100);
		~CUndoList ();
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
};

extern CUndoManager undoManager;

//------------------------------------------------------------------------------

#endif //__undoman_h
