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
		int	m_count;
		_T		m_null;

	public:
		CGameItemIterator (_T* buffer, int count) : m_nIndex(0), m_buffer(buffer), m_count(count) {}

		//prefix increment
		inline _T& operator++ () { 
			do {
				++m_nIndex;
				} while (!end () && (dynamic_cast<CGameItem&> (m_buffer [m_nIndex]).m_nIndex < 0));
			--m_count;
			return *value; 
			}

		// postfix increment
		inline _T& operator++ (int) { 
			_T* value;
			do {
				value = &m_buffer [m_nIndex++];
				} while (!end () && (dynamic_cast<CGameItem&> (*value).m_nIndex < 0));
			--m_count;
			return *value; 
			}

		// prefix decrement
		inline _T& operator-- () { 
			do {
				--m_nIndex;
				} while (!end () && (dynamic_cast<CGameItem&> (m_buffer [m_nIndex]).m_nIndex < 0));
			--m_count;
			return m_buffer [m_nIndex]; 
			}

		// postfix decrement
		inline _T& operator-- (int) { 
			_T* value;
			do {
				value = &m_buffer [m_nIndex--];
				} while (!end () && (dynamic_cast<CGameItem&> (*value).m_nIndex < 0));
			--m_count;
			return *value; 
			}

		inline CGameItemIterator& operator= (int i) { 
			m_nIndex = i;
			return *this;
			}

		inline const bool end (void) { return m_count == 0; }

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

