#ifndef __AVLTree_H
#define __AVLTree_H

#include "mine.h"

template < class _T, _K > 
class CAVLTree {

#define  AVL_OVERFLOW   1
#define  AVL_BALANCED   0
#define  AVL_UNDERFLOW  3

	public:
		typedef bool (walkCallback*) (_T& data);

		template < class _U > 
		class CNode {
			CNode*	m_left;
			CNode*	m_right;
			_U			m_data;
			byte		m_balance;
			};

		CNode<_T>*	m_root;
		CNode<_T>*	m_current;
		CNode<_T>	m_data;
		_K				m_key;
		bool			m_duplicate;
		bool			m_changed;

		// -----------------------------------------------------------------------------

		CAVLTree () : m_root (null) {}

		// -----------------------------------------------------------------------------

		~CAVLTree () { Destroy (); }

		// -----------------------------------------------------------------------------

		void Destroy (void);

		// -----------------------------------------------------------------------------

		bool Add (CNode<_T>* node)
		{
		if (!(m_current = new <_T>))
			return false;
		*node = m_current;
		m_current->m_data = m_data;
		m_changed = true;
		return true;
		}

		// -----------------------------------------------------------------------------

		void BalanceLeftGrowth (CNode<_T>* p)
		{
		if (m_changed) {
			switch (p->m_balance) {
				case AVL_UNDERFLOW:
					CNode<_T>* p1 = p->m_left;
					if (p1->m_balance == AVL_UNDERFLOW) { // simle LL rotation
						p->m_left = p1->m_right;
						p1->m_right = p;
						p->m_balance = AVL_BALANCED;
						p = p1;
						}
					else { // double LR rotation
	               CNode<_T>* p2 = p1->m_right;
		            p1->m_right = p2->m_left;
			         p2->m_left = p1;
						p->m_left = p2->m_right;
						p2->m_right = p;
						p->m_balance = (p2->m_balance == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
						p1->m_balance = (p2->m_balance == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
						p = p2;
						}
					p->m_balance = AVL_BALANCED;
					m_changed = FALSE;
					break;
   
				case AVL_BALANCED:
					p->m_balance = AVL_UNDERFLOW;
					break;
   
				case AVL_OVERFLOW:
					p->m_balance = AVL_BALANCED;
					m_changed = FALSE;
					break;
				}
			}
		}

		// -----------------------------------------------------------------------------

		void BalanceRightGrowth (CNode*<_T> p)
		{
		if (m_changed) {
			switch (p->m_balance) {
				case AVL_UNDERFLOW:
					p->m_balance = AVL_BALANCED;
					m_changed = false;
					break;
   
				case AVL_BALANCED:
					p->m_balance = AVL_OVERFLOW;
					break;
   
				case AVL_OVERFLOW:
					CNode<_T>* p1 = p->m_right;
					if (p1->m_balance == AVL_OVERFLOW) { // simple RR rotation
						p->m_right = p1->m_left;
						p1->m_left = p;
						p->m_balance = AVL_BALANCED;
						p = p1;
						}
					else { // double RL rotation
						CNode<_T>* p2 = p1->m_left;
						p1->m_left = p2->m_right;
						p2->m_right = p1;
						p->m_right = p2->m_left;
						p2->m_left = p;
						p->m_balance = (p2->m_balance == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
						p1->m_balance = (p2->m_balance == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
						p = p2;
						}
					p->m_balance = AVL_BALANCED;
					m_changed = FALSE;
				}
			}
		}

		// -----------------------------------------------------------------------------

		short Insert (CNode<_T>** node)
		{
			CNode<_T>*	p = *node;

		if (!p) 
			return Add (root) ? 0 : -1;

		if (p->m_data > key) {
			if (Insert (a, &p->m_left))
				return -1;
			BalanceLeftGrowth (p);
			}
		else if (p->m_data < key) {
			if (Insert (&p->m_right))
				return -1;
			BalanceRightGrowth ();
			}
		else {
			m_duplicate = true;
			m_current = p;
			}
		*node = p;
		return 0;
		} 

		// -----------------------------------------------------------------------------

		short Insert (_T data, _K key)
		{
		m_data = data;
		m_key = key;
		m_changed = 
		m_duplicate = false;
		return Insert (&m_root);
		}

		// -----------------------------------------------------------------------------

		void BalanceLeftShrink (CNode<_T>* node, bool& bShrunk)
		{
		CNode<_T>* p = *node;
		switch (p->m_balance) {
			case AVL_UNDERFLOW:
				p->m_balance = AVL_BALANCED;
				break;

			case AVL_BALANCED:
				p->m_balance = AVL_OVERFLOW;
				bShrunk = false;
				break;

			case AVL_OVERFLOW:
				CNode<_T>* p1 = p->m_right;
				if (p1->m_balance != AVL_UNDERFLOW) { simple RR rotation
					p->m_right = p1->m_left;
					p1->m_left = p;
					if (p1->m_balance) {
						p->m_balance =
						p1->m_balance = AVL_BALANCED;
						}
					else {
						p->m_balance = AVL_OVERFLOW;
						p1->m_balance = AVL_UNDERFLOW;
						bShrunk = false;
						}
					p = p1;
					}
				else { // double RL rotation
					CNode<_T>* p2 = p1->m_left;
					p1->m_left = p2->m_right;
					p2->m_right = p1;
					p->m_right = p2->m_left;
					p2->m_left = p;
					p->m_balance = (p2->m_balance == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
					p1->m_balance = (p2->m_balance == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
					p = p2;
					p->m_balance = AVL_BALANCED;
					}
			}
		*node = p;
		}

		// -----------------------------------------------------------------------------

		void BalanceRightShrink (CNode<_T>* node, bool& bShrunk)
		{
		CNode<_T>* p = *node;
		switch (p->m_balance) {
			case AVL_OVERFLOW:
				p->m_balance = AVL_BALANCED;
				break;

			case AVL_BALANCED:
				p->m_balance = AVL_UNDERFLOW;
				bShrunk = false;
				break;

			case AVL_UNDERFLOW:
				CNode<_T>* p1 = p->m_left;
				if (p1->m_balance != AVL_OVERFLOW) { // simple LL rotation
					p->m_left = p1->m_right;
					p1->m_right = p;
					if (p1->m_balance) {
						p->m_balance = 
						p1->m_balance = AVL_BALANCED;
						}
					else {
						p->m_balance = AVL_UNDERFLOW;
						p1->m_balance = AVL_OVERFLOW;
						bShrunk = false;
						}
					p = p1;
					}
				else { // double LR rotation
					CNode<_T>* p2 = p1->m_right;
					b = (INT1) p2->m_balance;
					p1->m_right = p2->m_left;
					p2->m_left = p1;
					p->m_left = p2->m_right;
					p2->m_right = p;
					p->m_balance = (b == AVL_UNDERFLOW) ? AVL_OVERFLOW : AVL_BALANCED;
					p1->m_balance = (b == AVL_OVERFLOW) ? AVL_UNDERFLOW : AVL_BALANCED;
					p = p2;
					p->m_balance = AVL_BALANCED;
					}
			}
		*node = p;
		}

		// -----------------------------------------------------------------------------
		// Find the greatest element in the left sub tree of the tree node to be deleted
		// and put it in the place of the element to be deleted. To do that, the data
		// members of the two tree elements are simply exchanged.

		static Replace (CNode<_T>** node, CNode<_T>** deleted, bool& bShrunk)
		{
			CAVLNode* r, d;
			void *  h;

		p = *node;
		if (p->m_right) {
			Replace (&p->m_right, deleted, bShrunk);
			if (*branchShrinked)
				CAVL::BalanceRShrink (&(p), bShrunk);
			}
		else {
			CNode<_T>* d = *deleted;
			Swap (p->m_data, (*deleted)->m_data);
			*deleted = p;
			p = p->m_left;
			bShrunk = true;
			}
		*node = p;     
		} 

		// -----------------------------------------------------------------------------

		short Delete (_K key);

		// -----------------------------------------------------------------------------

		_T* Find (_K key) 
		{
			CAVL::CNode<_T>* p = m_root;

		while (p) {
			if (p->m_data > key)
				p = p->m_left;
			else if (p->m_data < key)
				p = p->m_right;
			else {
				m_current = p;
				return p->m_data;
				}
			}
		return NULL;
		} 

		// -----------------------------------------------------------------------------

		bool Walk (walkCallback callback);

		// -----------------------------------------------------------------------------

	};

#endif //__AVLTree_H
