
#ifndef __SLL_H
#define __SLL_H

#include "define.h"
#include "types.h"

// -----------------------------------------------------------------------------
// Singly Linked List

template < class _T, class _K >
class CSLLIterator;

template < class _T, class _K >
class CSLL {
	public:
		friend class CSLLIterator<_T, _K>;


		template < class _U >
		class CNode {
			public:
				CNode*	m_succ;
				_U			m_data;

			CNode () : m_succ (null) {}

			CNode* GetSucc (void) { return m_succ; }

			CNode* SetSucc (CNode* succ) { return m_succ = succ; }
			};


		CNode<_T>*	m_head;
		CNode<_T>*	m_tail;
		uint			m_length;

	private:
		CNode<_T>* Find (_K key, CNode<_T>*& predP) {
			predP = null;
			for (CNode<_T>* nodeP = m_head; nodeP; nodeP = nodeP->m_succ) {
				if (nodeP->m_data == key)
					return nodeP;
				predP = nodeP;
				}
			return null;
			}

	public:
		CSLL () : m_head (null), m_tail (null), m_length (0) {}

		~CSLL () { Destroy (); }

		inline CNode<_T>* Head (void) { return m_head; }

		inline CNode<_T>* Tail (void) { return m_tail; }

		inline uint Length (void) { return m_length; }

		void Destroy (void) {
			CNode<_T>* linkP;
			for (CNode<_T>* nodeP = m_head; nodeP; nodeP = linkP) {
				linkP = nodeP->GetSucc ();
				delete nodeP;
				}
			m_head = null;
			m_tail = null;
			m_length = 0;
			}

		_T* Find (_K key) {
			for (CNode<_T>* nodeP = m_head; nodeP; nodeP = nodeP->GetSucc ()) {
				if (nodeP->m_data == key)
					return &nodeP->m_data;
				}
			return null;
			}

		bool Remove (_K key) {
			CNode<_T>* predP, * nodeP = Find (key, predP);
			if (!nodeP)
				return false;
			if (predP)
				predP->SetSucc (nodeP->GetSucc ());
			if (m_head == nodeP)
				m_head = nodeP->GetSucc ();
			if (m_tail == nodeP)
				m_tail = predP;
			nodeP->SetSucc (null);
			delete nodeP;
			--m_length;
			return true;
			}

		_T* Add (_T data) {
			if (!(nodeP = new CNode<_T>))
				return null;
			nodeP->SetSucc (m_head);
			m_head = nodeP;
			if (!m_tail)
				m_tail = nodeP;
			++m_length;
			nodeP->m_data = data;
			return &nodeP->m_data;
			}

		_T* Append (_T data) {
			if (!(nodeP = new CNode<_T>))
				return null;
			if (m_tail)
				m_tail->SetSucc (nodeP);
			else
				m_head = nodeP;
			m_tail = nodeP;
			++m_length;
			nodeP->m_data = data;
			return &nodeP->m_data;
			}

	inline CSLL<_T, _K>& operator= (CSLL<_T, _K>& other) {
		m_head = other.Head ();
		m_tail = other.Tail ();
		m_length = other.Length ();
		return *this;
		}
	};

// -----------------------------------------------------------------------------

template < class _T, class _K >
class CSLLIterator {
	private:
		CSLL<_T, _K>&	m_sll;
		typename CSLL<_T, _K>::CNode<_T>*	m_current;

	public:
		CSLLIterator (CSLL<_T, _K>& sll) : m_sll (sll) {}

		inline _T* Begin (void) { 
			m_current = m_sll.Head ();
			return m_current ? &m_current->m_data : null; 
			}

		inline _T* End (void) { return null; }

		inline CSLLIterator& operator++() { m_current = m_current->GetSucc (); }
		
		inline CSLLIterator& operator++(int) { 
			m_current = m_current->GetSucc (); 
			return *this;
			}
		
		inline _T* operator->() { return m_current ? &m_current->m_data : null; }
		
		inline _T* operator*() { return m_current ? &m_current->m_data : null; }
	};

// -----------------------------------------------------------------------------

#endif //__SLL_H