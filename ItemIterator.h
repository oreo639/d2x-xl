#ifndef __itemiterator_h
#define __itemiterator_h

#include "freelist.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template < class _T >
class CGameItemIterator {
	protected:
		_T*	m_buffer;
		_T*	m_value;
		int	m_count;
		int	m_index;
		_T		m_null;

	public:
		CGameItemIterator (_T* buffer, int count) : m_index(0), m_buffer(buffer), m_count(count) {}

#if USE_FREELIST

		//prefix increment
		inline _T& operator++ () { 
			do {
				m_value = &m_buffer [m_index++];
				} while (!end () && (dynamic_cast<CGameItem&> (*m_value).Index () < 0));
			--m_count;
			return *m_value; 
			}

		// postfix increment
		inline _T& operator++ (int) { 
			_T* m_value;
			do {
				m_value = &m_buffer [m_index++];
				} while (!end () && (dynamic_cast<CGameItem&> (*m_value).Index () < 0));
			--m_count;
			return *m_value; 
			}

		// prefix decrement
		inline _T& operator-- () { 
			do {
				m_value = m_buffer [--m_index];
				} while (!end () && (dynamic_cast<CGameItem&> (*m_value).Index () < 0));
			--m_count;
			return *m_value; 
			}

		// postfix decrement
		inline _T& operator-- (int) { 
			do {
				m_value = &m_buffer [m_index--];
				} while (!end () && (dynamic_cast<CGameItem&> (*m_value).Index () < 0));
			--m_count;
			return *m_value; 
			}

#else //USE_FREELIST

		//prefix increment
		inline _T& operator++ () { 
			if (m_index >= m_count - 1)
				return m_null;
			--m_count;
			return m_buffer [++m_index];
			}

		// postfix increment
		inline _T& operator++ (int) { 
			if (m_index >= m_count)
				return m_null;
			--m_count;
			return m_buffer [m_index++];
			}

		// prefix decrement
		inline _T& operator-- () { 
			if (m_index <= 0)
				return m_null;
			--m_count;
			return m_buffer [--m_index];
			}

		// postfix decrement
		inline _T& operator-- (int) { 
			if (m_index < 0)
				return m_null;
			--m_count;
			return m_buffer [m_index--];
			}

#endif // USE_FREELIST

		inline CGameItemIterator& operator= (int i) { 
			m_index = i;
			return *this;
			}

		inline const bool end (void) { return m_count == 0; }

		inline operator bool() { return !end (); }

		inline const bool operator== (int i) { return m_index == i; }

		inline const bool operator!= (int i) { return m_index != i; }

		inline _T* operator-> () { return end () ? null : &m_buffer [m_index]; }

		inline _T& operator* () { return end () ? m_null : m_buffer [m_index]; }

		inline const int Index (void) { return (int) (m_value - m_buffer); }
	};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __itemiterator_h

