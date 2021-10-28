#ifndef _CSTACK_H
#define _CSTACK_H

#include "carray.h"

//-----------------------------------------------------------------------------

template < class DATA_T > 
class CStack : public CArray< DATA_T > {
	protected:
		uint32_t	m_tos;
		uint32_t	m_growth;

	public:
		CStack () { Init (); }
		CStack (uint32_t nLength) { 
			Init (); 
			Create (nLength);
			}
		~CStack() { Destroy (); }

		inline void Reset (void) { m_tos = 0; }

		inline void Init (void) { 
			m_growth = 0;
			Reset ();
			CArray<DATA_T>::Init ();
			}

		inline bool Grow (const uint32_t i = 1) {
			if ((m_tos + i > this->m_data.length) && (!(m_growth && this->Resize (this->m_data.length + m_growth)))) {
#if DBG
				ArrayError ("invalid stack operation\n");
#endif
				return false;
				}
//#pragma omp critical
			m_tos += i;
			return true;
			}

		inline bool Push (const DATA_T elem) { 
			if (!Grow ())
				return false;
//#pragma omp critical
			this->m_data.buffer [m_tos - 1] = elem;
			return true;
			}
	
		inline void Shrink (uint32_t i = 1) {
//#pragma omp critical
			if (i >= m_tos)
				m_tos = 0;
			else
				m_tos -= i;
			}

		inline DATA_T& Pop (void) {
//#pragma omp critical
			Shrink ();
			return this->m_data.buffer [m_tos];
			}

		inline void Truncate (uint32_t i = 1) {
			if (i < m_tos)
				m_tos = i;
			}

		inline uint32_t Find (DATA_T& elem) {
			for (uint32_t i = 0; i < m_tos; i++)
				if (this->m_data.buffer [i] == elem)
					return i;
			return m_tos;
			}

		inline uint32_t ToS (void) { return m_tos; }

		inline DATA_T* Top (void) { return (this->m_data.buffer && m_tos) ? this->m_data.buffer + m_tos - 1 : NULL; }

		inline bool Delete (uint32_t i) {
			if (i >= m_tos) {
#if DBG
				ArrayError ("invalid stack access\n");
#endif
				return false;
				}
//#pragma omp critical
			if (i < --m_tos)
				memcpy (this->m_data.buffer + i, this->m_data.buffer + i + 1, sizeof (DATA_T) * (m_tos - i));
			return true;
			}

		inline bool DeleteElement (DATA_T& elem) { return Delete (Find (elem));	}

		inline DATA_T& Pull (DATA_T& elem, uint32_t i) {
//#pragma omp critical
			if (i < m_tos) {
				elem = this->m_data.buffer [i];
				Delete (i);
				}
			return elem;
			}

		inline DATA_T Pull (uint32_t i) {
			DATA_T	v;
			return Pull (v, i);
			}

		inline void Destroy (void) { 
			CArray<DATA_T>::Destroy ();
			m_tos = 0;
			}

		inline DATA_T *Create (uint32_t length, const char* pszName = NULL) {
			Destroy ();
			return CArray<DATA_T>::Create (length, pszName);
			}

		inline uint32_t Growth (void) { return m_growth; }

		inline void SetGrowth (uint32_t growth) { m_growth = growth; }

		inline void SortAscending (int32_t left = 0, int32_t right = -1) { 
			if (this->m_data.buffer)
				CQuickSort<DATA_T>::SortAscending (this->m_data.buffer, left, (right >= 0) ? right : m_tos - 1); 
				}

		inline void SortDescending (int32_t left = 0, int32_t right = -1) {
			if (this->m_data.buffer)
				CQuickSort<DATA_T>::SortDescending (this->m_data.buffer, left, (right >= 0) ? right : m_tos - 1);
			}
#ifdef _WIN32
		inline void SortAscending (comparator compare, int32_t left = 0, int32_t right = -1) {
			if (this->m_data.buffer)
				CQuickSort<DATA_T>::SortAscending (this->m_data.buffer, left, (right >= 0) ? right : m_tos - 1, compare);
			}

		inline void SortDescending (comparator compare, int32_t left = 0, int32_t right = -1) {
			if (this->m_data.buffer)
				CQuickSort<DATA_T>::SortDescending (this->m_data.buffer, left, (right >= 0) ? right : m_tos - 1, compare);
			}
#endif

		inline int32_t BinSearch (DATA_T key, int32_t left = 0, int32_t right = -1) {
			return this->m_data.buffer ? CQuickSort<DATA_T>::BinSearch (this->m_data.buffer, left, (right >= 0) ? right : m_tos - 1, key) : -1;
			}

	};

//-----------------------------------------------------------------------------

#endif //_CSTACK_H
