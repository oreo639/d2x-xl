#ifndef _CARRAY_H
#define _CARRAY_H

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <string.h>
#include <stdlib.h>

#ifndef DBG
#	ifdef _DEBUG
#		define DBG 1
#	else
#		define DBG 0
#	endif
#endif

#define DBG_ARRAYS	DBG

#include "cquicksort.h"
#include "cfile.h"

void ArrayError (const char* pszMsg);

//-----------------------------------------------------------------------------

template < class _T > 
class CDynamicArray : public CQuickSort < _T > {

	template < class _U > 
	class CArrayData {
		public:
			_U*		buffer;
			_U			null;
			UINT32	length;
			UINT32	pos;
			INT32		nMode;
			bool		bWrap;
			};

	protected:
		CArrayData<_T>	m_data;

	public:
		template < class _V >
		class Iterator {
			private:
				_V*			m_start;
				_V*			m_end;
				_V*			m_p;
				CDynamicArray<_V>&	m_a;
			public:
				Iterator () : m_p (NULL) {}
				Iterator (CDynamicArray<_V>& a) : m_a (a), m_p (NULL) {}
				operator bool() const { return m_p != NULL; }
				_V* operator*() const { return m_p; }
				Iterator& operator++() { 
					if (m_p) {
						if (m_p < m_end)
							m_p++;
						else
							m_p = NULL;
						}
					return *this;
					}
				Iterator& operator--() { 
					if (m_p) {
						if (m_p > m_end)
							m_p--;
						else
							m_p = NULL;
						}
					return *this;
					}
				_V* Start (void) { m_p = m_start = m_a.Start (); m_end = m_a.End (); }
				_V* End (void) { m_p = m_start = m_a.End (); m_end = m_a.Start (); }
			};

		CDynamicArray () { 
			Init (); 
			}
		
		CDynamicArray (UINT32 nLength) { 
			Init (); 
			Create (nLength);
			}
		
		~CDynamicArray() { Destroy (); }
		
		void Init (void) { 
			m_data.buffer = reinterpret_cast<_T *> (NULL); 
			m_data.length = 0;
			m_data.pos = 0;
			m_data.nMode = 0;
			m_data.bWrap = false;
			memset (&m_data.null, 0, sizeof (m_data.null));
			}

		void Clear (UINT8 filler = 0, UINT32 count = 0xffffffff) { 
#if DBG_ARRAYS
			if ((count != 0xffffffff) && (count > 1000000)) {
				count = count;
				ArrayError ("array overflow\n");
				}
			if ((count == 0xffffffff) && (m_data.length > 512 * 512 * 16 * 4)) {
				count = count;
				ArrayError ("array overflow\n");
				}
#endif
			if (m_data.buffer) 
				memset (m_data.buffer, filler, sizeof (_T) * ((count < m_data.length) ? count : m_data.length)); 
			}
		
		inline bool IsElement (_T* elem, bool bDiligent = false) {
			if (!m_data.buffer || (elem < m_data.buffer) || (elem >= m_data.buffer + m_data.length))
				return false;	// no buffer or element out of buffer
			if (bDiligent) {
				UINT32 i = static_cast<UINT32> (reinterpret_cast<UINT8*> (elem) - reinterpret_cast<UINT8*> (m_data.buffer));
				if (i % sizeof (_T))	
					return false;	// elem in the buffer, but not properly aligned
				}
			return true;
			}

#if DBG_ARRAYS
		inline INT32 Index (_T* elem) { 
			if (IsElement (elem))
				return static_cast<INT32> (elem - m_data.buffer); 
			ArrayError ("invalid array index\n");
			return -1;
			}
#else
		inline UINT32 Index (_T* elem) { return UINT32 (elem - m_data.buffer); }
#endif

#if DBG_ARRAYS
		inline _T* Pointer (UINT32 i) { 
			if (!m_data.buffer || (i >= m_data.length)) {
				ArrayError ("invalid array handle or index\n");
				return NULL;
				}
			return m_data.buffer + i; 
			}
#else
		inline _T* Pointer (UINT32 i) { return m_data.buffer + i; }
#endif

		void Destroy (void) { 
			if (m_data.buffer) {
				if (!m_data.nMode) {
					delete[] m_data.buffer;
#if DBG_ARRAYS
					m_data.buffer = reinterpret_cast<_T *> (NULL); 
#endif
					}
				Init ();
				}
			}
			
		_T *Create (UINT32 length) {
			if (m_data.length != length) {
				Destroy ();
				try {
					if ((m_data.buffer = new _T [length]))
						m_data.length = length;
					}
				catch(...) {
#if DBG_ARRAYS
					ArrayError ("invalid buffer size\n");
#endif
					m_data.buffer = NULL;
					}
				}
			return m_data.buffer;
			}
			
		inline _T* Buffer (UINT32 i = 0) { return m_data.buffer + i; }
		
		void SetBuffer (_T *buffer, INT32 nMode = 0, UINT32 length = 0xffffffff) {
			if (m_data.buffer != buffer) {
				if (!(m_data.buffer = buffer))
					Init ();
				else {
					m_data.length = length;
					m_data.nMode = nMode;
					}
				}
			}
			
		_T* Resize (UINT32 length, bool bCopy = true) {
			if (m_data.nMode == 2)
				return m_data.buffer;
			if (!m_data.buffer)
				return Create (length);
			_T* p;
			try {
			p = new _T [length];
				}
			catch(...) {
#if DBG_ARRAYS
				ArrayError ("invalid buffer size\n");
#endif
				p = NULL;
				}
			if (!p)
				return m_data.buffer;
			if (bCopy) {
				memcpy (p, m_data.buffer, ((length > m_data.length) ? m_data.length : length) * sizeof (_T)); 
				Clear (); // hack to avoid d'tors
				}
			m_data.length = length;
			m_data.pos %= length;
			delete[] m_data.buffer;
			return m_data.buffer = p;
			}

		inline UINT32 Length (void) { return m_data.length; }

		inline _T* Current (void) { return m_data.buffer ? m_data.buffer + m_data.pos : NULL; }

		inline size_t Size (void) { return m_data.length * sizeof (_T); }
#if DBG_ARRAYS
		inline _T& operator[] (UINT32 i) { 
			if (m_data.buffer && (i < m_data.length))
				return m_data.buffer [i];
			if (i == m_data.length)
				return m_data.null; 
			else {
				ArrayError ("invalid array handle or index\n");
				return m_data.null; 
				}
			}
#else
		inline _T& operator[] (UINT32 i) { return m_data.buffer [i]; }
#endif

		inline _T& operator= (CDynamicArray<_T>& source) { return Copy (source); }

		inline _T& operator= (_T* source) { 
			if (m_data.buffer) 
				memcpy (m_data.buffer, source, m_data.length * sizeof (_T)); 
			return m_data.buffer [0];
			}

		_T& Copy (CDynamicArray<_T>& source, UINT32 offset = 0) { 
			if (((static_cast<INT32> (m_data.length)) >= 0) && (static_cast<INT32> (source.m_data.length) > 0)) {
				if ((m_data.buffer && (m_data.length >= source.m_data.length + offset)) || Resize (source.m_data.length + offset, false)) {
					memcpy (m_data.buffer + offset, source.m_data.buffer, ((m_data.length - offset < source.m_data.length) ? m_data.length - offset : source.m_data.length) * sizeof (_T)); 
					}
				}
			return m_data.buffer [0];
			}

		inline _T& operator+ (CDynamicArray<_T>& source) { 
			UINT32 offset = m_data.length;
			if (m_data.buffer) 
				Resize (m_data.length + source.m_data.length);
			return Copy (source, offset);
			}

		inline bool operator== (CDynamicArray<_T>& other) { 
			return (m_data.length == other.m_data.length) && !(m_data.length && memcmp (m_data.buffer, other.m_data.buffer)); 
			}

		inline bool operator!= (CDynamicArray<_T>& other) { 
			return (m_data.length != other.m_data.length) || (m_data.length && memcmp (m_data.buffer, other.m_data.buffer)); 
			}

		inline _T* Start (void) { return m_data.buffer; }

		inline _T* End (void) { return (m_data.buffer && m_data.length) ? m_data.buffer + m_data.length - 1 : NULL; }

		inline _T* operator++ (void) { 
			if (!m_data.buffer)
				return NULL;
			if (m_data.pos < m_data.length - 1)
				m_data.pos++;
			else if (m_data.bWrap) 
				m_data.pos = 0;
			else
				return NULL;
			return m_data.buffer + m_data.pos;
			}

		inline _T* operator-- (void) { 
			if (!m_data.buffer)
				return NULL;
			if (m_data.pos > 0)
				m_data.pos--;
			else if (m_data.bWrap)
				m_data.pos = m_data.length - 1;
			else
				return NULL;
			return m_data.buffer + m_data.pos;
			}

#if DBG_ARRAYS

		inline _T* operator+ (UINT32 i) { 
			if (m_data.buffer && (i < m_data.length))
				return m_data.buffer + i;
			if (i == m_data.length)
				return NULL;
			else {
				ArrayError ("invalid array handle or index\n");
				return  NULL; 
				}
			}

#else

		inline _T* operator+ (UINT32 i) { return m_data.buffer ? m_data.buffer + i : NULL; }

#endif

		inline _T* operator- (UINT32 i) { return m_data.buffer ? m_data.buffer - i : NULL; }

		CDynamicArray<_T>& ShareBuffer (CDynamicArray<_T>& child) {
			memcpy (&child.m_data, &m_data, sizeof (m_data));
			if (!child.m_data.nMode)
				child.m_data.nMode = 1;
			return child;
			}

		inline bool operator! () { return m_data.buffer == NULL; }

		inline UINT32 Pos (void) { return m_data.pos; }

		inline void Pos (UINT32 pos) { m_data.pos = pos % m_data.length; }

		size_t Read (FILE* fp, UINT32 nCount = 0, UINT32 nOffset = 0) { 
			if (!m_data.buffer)
				return -1;
			if (nOffset >= m_data.length)
				return -1;
			if (!nCount)
				nCount = m_data.length - nOffset;
			else if (nCount > m_data.length - nOffset)
				nCount = m_data.length - nOffset;
			return INT32 (fread (m_data.buffer + nOffset, sizeof (_T), nCount));
			}

		size_t Write (FILE* fp, UINT32 nCount = 0, UINT32 nOffset = 0) { 
			if (!m_data.buffer)
				return -1;
			if (nOffset >= m_data.length)
				return -1;
			if (!nCount)
				nCount = m_data.length - nOffset;
			else if (nCount > m_data.length - nOffset)
				nCount = m_data.length - nOffset;
			return INT32 (fwrite (m_data.buffer + nOffset, sizeof (_T), nCount));
			}

		inline void SetWrap (bool bWrap) { m_data.bWrap = bWrap; }

		inline void SortAscending (INT32 left = 0, INT32 right = -1) { 
			if (m_data.buffer) 
				CQuickSort<_T>::SortAscending (m_data.buffer, left, (right >= 0) ? right : m_data.length - 1); 
				}

		inline void SortDescending (INT32 left = 0, INT32 right = -1) {
			if (m_data.buffer) 
				CQuickSort<_T>::SortDescending (m_data.buffer, left, (right >= 0) ? right : m_data.length - 1);
			}

		inline size_t Find (_T key) {
			for (UINT32 i = 0; i < m_data.length; i++)
				if (key == m_data.buffer [i])
					return 0;
			return -1;
			}

		inline size_t FindSorted (_T key) {
			size_t i = 0, j = size_t (m_data.length);
			_T t;
			do {
				m = (i + j) / 2;
				t = m_data.buffer [m];
				if (key < t)
					r = m - 1;
				else if (key > t)
					l = m + 1;
				else
					return m;
			} while (i <= j);
			return -1;
			}

#ifdef _WIN32
		inline void SortAscending (comparator compare, INT32 left = 0, INT32 right = -1) {
			if (m_data.buffer) 
				CQuickSort<_T>::SortAscending (m_data.buffer, left, (right >= 0) ? right : m_data.length - 1, compare);
			}

		inline void SortDescending (comparator compare, INT32 left = 0, INT32 right = -1) {
			if (m_data.buffer) 
				CQuickSort<_T>::SortDescending (m_data.buffer, left, (right >= 0) ? right : m_data.length - 1, compare);
			}
#endif
	};

//-----------------------------------------------------------------------------

inline INT32 operator- (char* v, CDynamicArray<char>& a) { return a.Index (v); }
inline INT32 operator- (UINT8* v, CDynamicArray<UINT8>& a) { return a.Index (v); }
inline INT32 operator- (INT16* v, CDynamicArray<INT16>& a) { return a.Index (v); }
inline INT32 operator- (UINT16* v, CDynamicArray<UINT16>& a) { return a.Index (v); }
inline INT32 operator- (INT32* v, CDynamicArray<INT32>& a) { return a.Index (v); }
inline INT32 operator- (UINT32* v, CDynamicArray<UINT32>& a) { return a.Index (v); }

class CCharArray : public CDynamicArray<char> {
	public:
		inline char* operator= (const char* source) { 
			UINT32 l = UINT32 (strlen (source) + 1);
			if ((l > this->m_data.length) && !this->Resize (this->m_data.length + l))
				return NULL;
			memcpy (this->m_data.buffer, source, l);
			return this->m_data.buffer;
		}
};

//class CByteArray : public CDynamicArray<UINT8> {};
//class CShortArray : public CDynamicArray<INT16> {};
//class CUShortArray : public CDynamicArray<UINT16> {};
//class CIntArray : public CDynamicArray<INT32> {};
//class CUIntArray : public CDynamicArray<UINT32> {};
//class CFloatArray : public CDynamicArray<float> {};

//-----------------------------------------------------------------------------

template < class _T, UINT32 length > 
class CStaticArray : public CDynamicArray < _T > {

	template < class _U, UINT32 _length > 
	class CStaticArrayData {
		public:
			_U		buffer [_length];
			};

	protected:
		CStaticArrayData< _T, length > m_staticData;

	public:
		CStaticArray () { Create (length); }

		_T *Create (UINT32 _length) { 
			this->SetBuffer (m_staticData.buffer, 2, _length); 
			return m_data.buffer;
			}
		void Destroy (void) { }
	};

//-----------------------------------------------------------------------------


#endif //_CARRAY_H
