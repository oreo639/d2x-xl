#ifndef _CQUICKSORT_H
#define _CQUICKSORT_H

#include "pstypes.h"

//-----------------------------------------------------------------------------

template < class DATA_T > 
class CQuickSort {
	public:
		typedef int32_t (*comparator) (const DATA_T*, const DATA_T*);

		void SortAscending (DATA_T* buffer, int32_t left, int32_t right);
		void SortDescending (DATA_T* buffer, int32_t left, int32_t right);
		void SortAscending (DATA_T* buffer, int32_t left, int32_t right, comparator compare);
		void SortDescending (DATA_T* buffer, int32_t left, int32_t right, comparator compare);
		inline void Swap (DATA_T* left, DATA_T* right);
		int32_t BinSearch (DATA_T* buffer, int32_t l, int32_t r, DATA_T key);
	};

//-----------------------------------------------------------------------------

#include "cquicksort.cpp"

#endif //_CQUICKSORT_H
