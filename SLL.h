
#ifndef __SLL_H
#define __SLL_H

#include "define.h"
#include "types.h"

// -----------------------------------------------------------------------------
// Single Sided Linked List

template < class _T, class _K >
class CSLLIterator;

template < class _T, class _K >
class CSLL {
	public:
		friend class CSLLIterator<_T, _K>;

		template < class _U >
		class CNode {
			public:
				CNode*	m_link;
				_U			m_data;

			CNode () : m_link (null) {}
			};

		CNode<_T>*	m_root;

	public:
		CSLL () : m_root (null) {}

		~CSLL () { Destroy (); }

		void Destroy (void) {
			CNode<_T>* linkP;
			for (CNode<_T>* listP = m_root; listP; listP = linkP) {
				linkP = listP->m_link;
				delete listP;
				}
			m_root = null;
			}

		_T* Insert (_T data, _K key) {
			CNode<_T>* tailP = null;
			CNode<_T>* listP = m_root;
			for (; listP; listP = listP->m_link) {
				tailP = listP;
				if (listP->m_data == key)
					return &listP->m_data;
				}
			if (!(listP = new CNode<_T>))
				return null;
			if (tailP)
				tailP->m_link = listP;
			else
				m_root = listP;
			listP->m_data = data;
			return &listP->m_data;
			}

	inline CSLL<_T, _K>& operator= (CSLL<_T, _K>& other) {
		m_root = other.m_root;
		return *this;
		}
	};

template < class _T, class _K >
class CSLLIterator {
	private:
		CSLL<_T, _K>&				m_sll;
		typename CSLL<_T, _K>::CNode<_T>*	m_current;

	public:
		CSLLIterator (CSLL<_T, _K>& sll) : m_sll (sll) {}

		inline _T* Begin (void) { 
			m_current = m_sll.m_root;
			return &m_current->m_data; 
			}

		inline _T* End (void) { return null; }

		inline CSLLIterator& operator++() { m_current = m_current->m_link; }
		
		inline CSLLIterator& operator++(int) { 
			m_current = m_current->m_link; 
			return *this;
			}
		
		inline _T* operator->() { return m_current ? &m_current->m_data : null; }
		
		inline _T* operator*() { return m_current ? &m_current->m_data : null; }
	};

// -----------------------------------------------------------------------------

#endif //__side_h