
#ifndef __SLL_H
#define __SLL_H

#include "define.h"
#include "mine.h"

// -----------------------------------------------------------------------------
// Single Sided Linked List

template < class _T, class _K >
class CSLL {
	public:
		friend class CSLLIterator <_T>;
	private:
		template < class _U >
		class CNode {
			CNode*	m_link;
			_U			m_data;
			}

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
				if (*listP == key)
					return &listP->m_data;
				}
			if (!(listP = new _T (key))
				return null;
			if (tailP)
				tailP->m_link = listP;
			else
				m_sides = listP;
			return &listP->m_data;
			}
	};

template < class _T >
class CSLLIterator {
	private:
		CSLL<_T>&			m_sll;
		CSLL::CNode<_T>*	m_current;

	public:
		CSLLIterator (CSLL<_T>& sll) : m_sll (sll) {}

		_T* Begin (void) { 
			m_current = &m_sll.m_root;
			return m_current->m_data; 
			}

		_T* End (void) { return null; }

		CSLLIterator& operator++() { m_current = m_current->m_link; }
		
		_T* operator*() { return &m_current->m_data; }

	};

// -----------------------------------------------------------------------------

#endif //__side_h