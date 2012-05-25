
#ifndef __SLL_H
#define __SLL_H

#include "define.h"

// -----------------------------------------------------------------------------
// Single Sided Linked List

template < class _T, class _K >
class CSLL {
	template < class _U >
	class CNode {
		CNode*	m_link;
		_U			m_data;
		}

	CNode<_T>*	m_root;

	CSLL () : m_root (null) {}
	~CSLL () {
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

// -----------------------------------------------------------------------------

#endif //__side_h