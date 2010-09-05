#ifndef __freelist_h
#define __freelist_h


# pragma pack(push, packing)
# pragma pack(1)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CFreeList {
	protected:
		int*	m_freeList;
		int	m_size;
		int	m_freeItems;

	public:
		CFreeList () : m_freeList(null), m_size (0), m_freeItems(0) { Create (size); }

		~CFreeList () { Destroy () };
		
		bool Create (int size) {
			m_freeList = new int [size];
			if (m_freeList == null)
				return false;
			m_size = size;
			Reset ();
			return true;
			}

		void Reset (void) {
			if (m_freeList != null) {
				m_freeItems = 0;
				for (int i = m_size; m_freeItems < m_size; m_freeItems++)
					m_freeList [m_freeItems] = --i;
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

		inline int Get (void) { return (m_freeItems > 0) ? m_freeList [--m_freeItems] : -1; }

		inline void Put (int i) { 
			if (m_freeItems < m_size)
				m_freeList [m_freeItems++] = i;
			}
	};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __freelist_h

