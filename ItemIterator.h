#ifndef __itemiterator_h
#define __itemiterator_h

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template < class _T >
class CGameItemIterator {
	protected:
		int	m_nIndex;
		_T*	m_buffer;
		int	m_length;
		_T		m_null;

	public:
		CGameItemIterator (_T* buffer, int length) : m_nIndex(0), m_buffer(buffer), m_length(length) {}

		//prefix increment
		inline _T& operator++ () { 
			do {
				++m_nIndex;
				} while (!end () && (dynamic_cast<CGameItem&> (m_buffer [m_nIndex]).m_nIndex < 0));
			return *value; 
			}

		// postfix increment
		inline _T& operator++ (int) { 
			_T* value;
			do {
				value = &m_buffer [m_nIndex++];
				} while (!end () && (dynamic_cast<CGameItem&> (*value).m_nIndex < 0));
			return *value; 
			}

		// prefix decrement
		inline _T& operator-- () { 
			do {
				--m_nIndex;
				} while (!end () && (dynamic_cast<CGameItem&> (m_buffer [m_nIndex]).m_nIndex < 0));
			return m_buffer [m_nIndex]; 
			}

		// postfix decrement
		inline _T& operator-- (int) { 
			_T* value;
			do {
				value = &m_buffer [m_nIndex--];
				} while (!end () && (dynamic_cast<CGameItem&> (*value).m_nIndex < 0));
			return *value; 
			}

		inline CGameItemIterator& operator= (int i) { 
			m_nIndex = i;
			return *this;
			}

		inline const bool start (void) { return m_nIndex == 0; }

		inline const bool end (void) { return m_nIndex == m_length; }

		inline operator bool() { return !end (); }

		inline const bool operator== (int i) { return m_nIndex == i; }

		inline const bool operator!= (int i) { return m_nIndex != i; }

		inline _T* operator-> () { return end () ? null : &m_buffer [m_nIndex]; }

		inline _T& operator* () { return end () ? m_null : m_buffer [m_nIndex]; }
	};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __itemiterator_h

