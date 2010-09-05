#ifndef __freelist_h
#define __freelist_h


# pragma pack(push, packing)
# pragma pack(1)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

template <class _T>
class CFreeList {
	protected:
		int*	m_freeList;
		int	m_size;
		int	m_freeItems;
		_T*	m_buffer;

	public:
		CFreeList () : m_buffer(null), m_freeList(null), m_size (0), m_freeItems(0) {}

		~CFreeList () { Destroy (); }
		
		bool Create (_T* buffer, int size) {
			m_freeList = new int [size];
			if (m_freeList == null)
				return false;
			m_buffer = buffer;
			m_size = size;
			Reset ();
			return true;
			}

		void Reset (void) {
			if (m_freeList != null) {
				m_freeItems = 0;
				for (int i = m_size; m_freeItems < m_size; m_freeItems++) {
					m_freeList [m_freeItems] = --i;
					m_buffer [m_freeItems].m_bUsed = false;
					}
				}
			}

		void Destroy (void) {
			if (m_freeList) {
				delete[] m_freeList;
				m_freeList = null;
				}
			m_size = 0;
			}

		inline int Free (void) { return m_freeItems; }

		inline bool Empty (void) { return m_freeItems == 0; }

		inline int Get (void) { 
			if (m_freeItems <= 0) 
				return -1;
			int i = m_freeList [--m_freeItems];
			m_buffer [i].m_bUsed = true;
			return i;
			}

		inline void Put (int i) { 
			if (m_freeItems < m_size) {
				m_buffer [i].m_bUsed = false;
				m_freeList [m_freeItems++] = i;
				}
			}

		inline const int operator-- () { return Get (); }

		inline const int operator-- (int) { return Get (); }

		inline const void operator+= (int i) { Put (i); }
	};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __freelist_h

