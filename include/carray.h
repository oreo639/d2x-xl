#ifndef _CARRAY_H
#define _CARRAY_H

#ifdef HAVE_CONFIG_H
#include <conf.h>
#endif

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifndef DBG
#	ifdef _DEBUG
#		define DBG 1
#	else
#		define DBG 0
#	endif
#endif

#define DBG_ARRAYS	0 //DBG

#include "pstypes.h"
#include "cquicksort.h"
#include "cfile.h"
#include "u_mem.h"

void ArrayError (const char* pszMsg);

//-----------------------------------------------------------------------------

template < class DATA_T > 
class CArray : public CQuickSort < DATA_T > {

	class CArrayData {
		public:
			char		szName [256];
			DATA_T	*	buffer;
			DATA_T		none;
			uint32_t	length;
			uint32_t	pos;
			int32_t		nMode;
			bool		bWrap;

		public:
			inline uint32_t Length (void) { return length; }
		};

	protected:
		CArrayData		m_data;

	public:
		class Iterator {
			private:
				DATA_T*			m_start;
				DATA_T*			m_end;
				DATA_T*			m_p;
				CArray<DATA_T>&	m_a;
			public:
				Iterator () : m_start (nullptr), m_end (nullptr), m_p (nullptr) {}
				Iterator (CArray<DATA_T>& a) : m_start (nullptr), m_end (nullptr), m_p (nullptr), m_a (a) {}
				operator bool() const { return m_p != nullptr; }
				DATA_T* operator*() const { return m_p; }
				Iterator& operator++() { 
					if (m_p) {
						if (m_p < m_end)
							m_p++;
						else
							m_p = nullptr;
						}
					return *this;
					}
				Iterator& operator--() { 
					if (m_p) {
						if (m_p > m_end)
							m_p--;
						else
							m_p = nullptr;
						}
					return *this;
					}
				DATA_T* Start (void) {
					m_p = m_start = m_a.Start (); m_end = m_a.End ();
					return m_p;
					}
				DATA_T* End (void) {
					m_p = m_start = m_a.End (); m_end = m_a.Start ();
					return m_p;
					}
			};

		// ----------------------------------------

		explicit CArray () { 
			Init (); 
			}
		
		explicit CArray (const uint32_t nLength) { 
			Init (); 
			Create (nLength);
			}
		
		CArray (CArray&& other) {
			Move (other);
		}

		CArray(CArray const& other) {
			Init (); 
			Copy (other);
			}
		
		~CArray() { Destroy (); }
		
		// ----------------------------------------

		void Init (void) noexcept { 
			*m_data.szName = 0;
			m_data.buffer = nullptr; 
			m_data.length = 0;
			m_data.pos = 0;
			m_data.nMode = 0;
			m_data.bWrap = false;
			memset (&m_data.none, 0, uint32_t (sizeof(m_data.none)));
			}

		// ----------------------------------------

		void Clear (uint8_t filler = 0, uint32_t count = 0xffffffff) noexcept {
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
				memset (m_data.buffer, filler, sizeof (DATA_T) * ((count < m_data.length) ? count : m_data.length)); 
			}

		// ----------------------------------------

		inline bool IsIndex (uint32_t i) { return (m_data.buffer != nullptr) && (i < m_data.length); }
		
		// ----------------------------------------

		inline bool IsElement (DATA_T* elem, bool bDiligent = false) {
			if (!m_data.buffer || (elem < m_data.buffer) || (elem >= m_data.buffer + m_data.length))
				return false;	// no buffer or element out of buffer
			if (bDiligent) {
				uint32_t i = static_cast<uint32_t> (reinterpret_cast<uint8_t*> (elem) - reinterpret_cast<uint8_t*> (m_data.buffer));
				if (i % sizeof (DATA_T))	
					return false;	// elem in the buffer, but not properly aligned
				}
			return true;
			}

		// ----------------------------------------

#if DBG_ARRAYS
		inline int32_t Index (DATA_T* elem) { 
			if (IsElement (elem))
				return static_cast<int32_t> (elem - m_data.buffer); 
			ArrayError ("invalid array index\n");
			return -1;
			}
#else
		inline uint32_t Index (DATA_T* elem) noexcept { return uint32_t (elem - m_data.buffer); }
#endif

		// ----------------------------------------

#if DBG_ARRAYS
		inline DATA_T* Pointer (uint32_t i) { 
			if (!m_data.buffer || (i >= m_data.length)) {
				ArrayError ("invalid array handle or index\n");
				return nullptr;
				}
			return m_data.buffer + i; 
			}
#else
		inline DATA_T* Pointer (uint32_t i) { return m_data.buffer + i; }
#endif

		// ----------------------------------------

		inline void SetName (const char* pszName) noexcept {
#if DBG_ARRAYS
			if (strlen (pszName) > 255)
				ArrayError ("invalid array name\n");
#endif
			strncpy (m_data.szName, pszName, 256); 
			m_data.szName [(uint32_t) sizeof(m_data.szName) - 1] = '\0';
			}

		// ----------------------------------------

		inline const char* GetName (void) { return m_data.szName; }

		// ----------------------------------------

		void Destroy (void) noexcept {
			if (m_data.buffer) {
				if (!m_data.nMode) {
#if DBG_MALLOC
					UnregisterMemBlock (m_data.buffer);
					bool b = TrackMemory (false);
#endif
					try {
						delete[] m_data.buffer;
						}
					catch(...) {
#if DBG_ARRAYS
						ArrayError ("invalid buffer pointer\n");
#endif
						}
#if DBG_MALLOC
					TrackMemory (b);
#endif
#if DBG_ARRAYS
					m_data.buffer = reinterpret_cast<DATA_T *> (nullptr); 
#endif
					}
				Init ();
				}
			}
			
		// ----------------------------------------

		DATA_T *Create (uint32_t length, const char* pszName = nullptr) {
			if (pszName)
				SetName (pszName);
			if (m_data.length != length) {
				Destroy ();
#if DBG_MALLOC
				bool b = TrackMemory (false);
#endif
				try {
					if ((m_data.buffer = NEW DATA_T [length]))
						m_data.length = length;
					}
				catch(...) {
#if DBG_ARRAYS
					ArrayError ("invalid buffer size\n");
#endif
					m_data.buffer = nullptr;
					}
#if DBG_MALLOC
				TrackMemory (b);
				RegisterMemBlock (m_data.buffer, length * sizeof (DATA_T), m_data.szName, 0);
#endif
				}
			return m_data.buffer;
			}
			
		// ----------------------------------------

		inline DATA_T* Buffer (uint32_t i = 0) const { return m_data.buffer + i; }
		
		// ----------------------------------------

		void SetBuffer (DATA_T *buffer, int32_t nMode = 0, uint32_t length = 0xffffffff) {
			if (m_data.buffer != buffer) {
				if (!(m_data.buffer = buffer))
					Init ();
				else {
					m_data.length = length;
					m_data.nMode = nMode;
					}
				}
			}
			
		// ----------------------------------------

		DATA_T* Resize (uint32_t length, bool bCopy = true) {
			if (m_data.nMode == 2)
				return m_data.buffer;
			if (!m_data.buffer)
				return Create (length);
			DATA_T* p;
#if DBG_MALLOC
			bool b = TrackMemory (false);
#endif
			try {
				p = NEW DATA_T [length];
				}
			catch(...) {
#if DBG_ARRAYS
				ArrayError ("invalid buffer size\n");
#endif
				p = nullptr;
				}
			if (!p) {
#if DBG_MALLOC
				TrackMemory (b);
#endif
				return m_data.buffer;
				}
#if DBG_MALLOC
				TrackMemory (true);
				RegisterMemBlock (p, length * sizeof (DATA_T), m_data.szName, 0);
#endif
			if (bCopy) {
				memcpy (p, m_data.buffer, ((length > m_data.length) ? m_data.length : length) * sizeof (DATA_T)); 
				Clear (); // hack to avoid d'tors
				}
			m_data.length = length;
			m_data.pos %= length;
#if DBG_MALLOC
			if (!UnregisterMemBlock (m_data.buffer))
				ArrayError ("invalid buffer pointer\n");
			TrackMemory (false);
#endif
			try {
				delete[] m_data.buffer;
				}
			catch (...) {
#if DBG_ARRAYS
				ArrayError ("invalid buffer pointer\n");
#endif
				}
#if DBG_MALLOC
			TrackMemory (b);
#endif
			return m_data.buffer = p;
			}

		// ----------------------------------------

		inline uint32_t Length (void) const { return m_data.length; }

		// ----------------------------------------

		inline DATA_T* Current (void) { return m_data.buffer ? m_data.buffer + m_data.pos : nullptr; }

		// ----------------------------------------

		inline uint32_t Size (void) { return m_data.length * sizeof (DATA_T); }
#if DBG_ARRAYS
		inline DATA_T& operator[] (uint32_t i) { 
			if (m_data.buffer && (i < m_data.length))
				return m_data.buffer [i];
			if (i == m_data.length)
				return m_data.none; 
			else {
				ArrayError ("invalid array handle or index\n");
				return m_data.none; 
				}
			}
#else
		inline DATA_T& operator[] (const uint32_t i) { return m_data.buffer [i]; }
#endif

		// ----------------------------------------

		inline DATA_T& operator* () const { return m_data.buffer; }

		// ----------------------------------------

		//inline DATA_T& operator= (CArray<DATA_T> const& source) { return _Copy (source); }

		// ----------------------------------------

		inline CArray<DATA_T>& operator= (CArray<DATA_T> const& source) { return Copy (source); }

		// ----------------------------------------

		inline CArray<DATA_T> operator= (CArray<DATA_T>&& source) { return Move (source); }

		// ----------------------------------------

		inline DATA_T& operator= (DATA_T* source) {
#if DBG_ARRAYS
			if (!m_data.buffer) 
				return m_data.none;
#endif
			memcpy (m_data.buffer, source, m_data.length * sizeof (DATA_T)); 
			return m_data.buffer [0];
			}

		// ----------------------------------------
#if 0
		DATA_T& Copy (CArray const& source, uint32_t offset = 0) { 
#	if DBG_ARRAYS
			if (!source.m_data.buffer) 
				return m_data.none;
#	endif
			if (((static_cast<int32_t> (m_data.length)) >= 0) && (static_cast<int32_t> (source.m_data.length) > 0)) {
				if (!*GetName ())
					SetName (source.m_data.szName);
				if ((m_data.buffer && (m_data.length >= source.m_data.length + offset)) || Resize (source.m_data.length + offset, false)) {
					memcpy (m_data.buffer + offset, source.m_data.buffer, ((m_data.length - offset < source.m_data.length) ? m_data.length - offset : source.m_data.length) * sizeof (DATA_T)); 
					}
				}
			return m_data.buffer [0];
			}
#endif
		// ----------------------------------------

		inline CArray& Copy (CArray const& source, uint32_t offset = 0) {
#if DBG_ARRAYS
			if (!source.Buffer())
				return m_data.none;
#endif
			if (source.Length () > 0) {
				if (!*GetName ())
					SetName (source.m_data.szName);
				_Copy (source.Buffer (), source.Length (), offset);
			}
			return *this;
		}

		// ----------------------------------------

		DATA_T& _Copy (DATA_T const* source, uint32_t const length, uint32_t offset = 0) {
			if ((m_data.buffer && (m_data.length >= length + offset)) || Resize (length + offset, false))
				memcpy (m_data.buffer + offset, source, ((m_data.length - offset < length) ? m_data.length - offset : length) * sizeof (DATA_T));
			return m_data.buffer [0];
		}

		// ----------------------------------------

		CArray<DATA_T>& Move (CArray<DATA_T>& source) {
			Destroy ();
			m_data.buffer = source.m_data.buffer;
			m_data.length = source.m_data.length;
			m_data.pos = source.m_data.pos;
			m_data.mode = source.m_data.mode;
			m_data.wrap = source.m_data.wrap;
			source.m_data.buffer = nullptr;
			source.Destroy ();
			return *this;
		}

		// ----------------------------------------

		inline DATA_T operator+ (CArray<DATA_T>& source) { 
			CArray<DATA_T> a (*this);
			a += source;
			return a;
			}

		// ----------------------------------------

		inline DATA_T& operator+= (CArray<DATA_T>& source) { 
			uint32_t offset = m_data.length;
			if (m_data.buffer) 
				Resize (m_data.length + source.m_data.length);
			return Copy (source, offset);
			}

		// ----------------------------------------

		inline bool operator== (CArray<DATA_T>& other) { 
			return (m_data.length == other.m_data.length) && !(m_data.length && memcmp (m_data.buffer, other.m_data.buffer)); 
			}

		// ----------------------------------------

		inline bool operator!= (CArray<DATA_T>& other) { 
			return (m_data.length != other.m_data.length) || (m_data.length && memcmp (m_data.buffer, other.m_data.buffer)); 
			}

		// ----------------------------------------

		inline DATA_T* Start (void) { return m_data.buffer; }

		// ----------------------------------------

		inline DATA_T* End (void) { return (m_data.buffer && m_data.length) ? m_data.buffer + m_data.length - 1 : nullptr; }

		// ----------------------------------------

		inline DATA_T* operator++ (void) { 
			if (!m_data.buffer)
				return nullptr;
			if (m_data.pos < m_data.length - 1)
				m_data.pos++;
			else if (m_data.bWrap) 
				m_data.pos = 0;
			else
				return nullptr;
			return m_data.buffer + m_data.pos;
			}

		// ----------------------------------------

		inline DATA_T* operator-- (void) { 
			if (!m_data.buffer)
				return nullptr;
			if (m_data.pos > 0)
				m_data.pos--;
			else if (m_data.bWrap)
				m_data.pos = m_data.length - 1;
			else
				return nullptr;
			return m_data.buffer + m_data.pos;
			}

#if DBG_ARRAYS

		// ----------------------------------------

		inline DATA_T* operator+ (uint32_t i) { 
			if (m_data.buffer && (i < m_data.length))
				return m_data.buffer + i;
			if (i == m_data.length)
				return nullptr;
			else {
				ArrayError ("invalid array handle or index\n");
				return  nullptr; 
				}
			}

#else

		inline DATA_T* operator+ (uint32_t i) { return m_data.buffer ? m_data.buffer + i : nullptr; }

#endif

		// ----------------------------------------

		inline DATA_T* operator- (uint32_t i) { return m_data.buffer ? m_data.buffer - i : nullptr; }

		// ----------------------------------------

		CArray<DATA_T>& ShareBuffer (CArray<DATA_T>& child) {
			memcpy (&child.m_data, &m_data, sizeof(m_data));
			if (!child.m_data.nMode)
				child.m_data.nMode = 1;
			return child;
			}

		// ----------------------------------------

		inline bool operator! () { return m_data.buffer == nullptr; }

		// ----------------------------------------

		inline uint32_t Pos (void) { return m_data.pos; }

		// ----------------------------------------

		inline void Pos (uint32_t pos) { m_data.pos = pos % m_data.length; }

		// ----------------------------------------

		size_t Read (CFile& cf, uint32_t nCount = 0, uint32_t nOffset = 0, int32_t bCompressed = 0) { 
			if (!m_data.buffer)
				return -1;
			if (nOffset >= m_data.length)
				return -1;
			if (!nCount)
				nCount = m_data.length - nOffset;
			else if (nCount > m_data.length - nOffset)
				nCount = m_data.length - nOffset;
			return cf.Read (m_data.buffer + nOffset, sizeof (DATA_T), nCount, bCompressed);
			}

		// ----------------------------------------

		size_t Write (CFile& cf, uint32_t nCount = 0, uint32_t nOffset = 0, int32_t bCompressed = 0) { 
			if (!m_data.buffer)
				return -1;
			if (nOffset >= m_data.length)
				return -1;
			if (!nCount)
				nCount = m_data.length - nOffset;
			else if (nCount > m_data.length - nOffset)
				nCount = m_data.length - nOffset;
			return cf.Write (m_data.buffer + nOffset, sizeof (DATA_T), nCount, bCompressed);
			}

		// ----------------------------------------

		inline void SetWrap (bool bWrap) { m_data.bWrap = bWrap; }

		// ----------------------------------------

		inline void SortAscending (int32_t left = 0, int32_t right = -1) { 
			if (m_data.buffer) 
				CQuickSort<DATA_T>::SortAscending (m_data.buffer, left, (right >= 0) ? right : m_data.length - 1); 
				}

		// ----------------------------------------

		inline void SortDescending (int32_t left = 0, int32_t right = -1) {
			if (m_data.buffer) 
				CQuickSort<DATA_T>::SortDescending (m_data.buffer, left, (right >= 0) ? right : m_data.length - 1);
			}

		// ----------------------------------------

#ifdef _WIN32
		inline void SortAscending (comparator compare, int32_t left = 0, int32_t right = -1) {
			if (m_data.buffer) 
				CQuickSort<DATA_T>::SortAscending (m_data.buffer, left, (right >= 0) ? right : m_data.length - 1, compare);
			}

		// ----------------------------------------

		inline void SortDescending (comparator compare, int32_t left = 0, int32_t right = -1) {
			if (m_data.buffer) 
				CQuickSort<DATA_T>::SortDescending (m_data.buffer, left, (right >= 0) ? right : m_data.length - 1, compare);
			}
#endif

		// ----------------------------------------

		inline int32_t BinSearch (DATA_T key, int32_t left = 0, int32_t right = -1) {
			return m_data.buffer ? CQuickSort<DATA_T>::BinSearch (m_data.buffer, left, (right >= 0) ? right : m_data.length - 1, key) : -1;
			}
	};

//-----------------------------------------------------------------------------

inline int32_t operator- (char* v, CArray<char>& a) noexcept { return a.Index (v); }
inline int32_t operator- (uint8_t* v, CArray<uint8_t>& a) noexcept { return a.Index (v); }
inline int32_t operator- (int16_t* v, CArray<int16_t>& a) noexcept { return a.Index (v); }
inline int32_t operator- (uint16_t* v, CArray<uint16_t>& a) noexcept { return a.Index (v); }
inline int32_t operator- (int32_t* v, CArray<int32_t>& a) noexcept { return a.Index (v); }
inline int32_t operator- (uint32_t* v, CArray<uint32_t>& a) noexcept { return a.Index (v); }

//-----------------------------------------------------------------------------

class CCharArray : public CArray<char> {
	public:
		inline char* operator= (const char* source) { 
			uint32_t l = uint32_t (strlen (source) + 1);
			if ((l > this->m_data.length) && !this->Resize (this->m_data.length + l))
				return nullptr;
			memcpy (this->m_data.buffer, source, l);
			return this->m_data.buffer;
		}
};

//-----------------------------------------------------------------------------

class CByteArray : public CArray<uint8_t> {};
class CShortArray : public CArray<int16_t> {};
class CUShortArray : public CArray<uint16_t> {};
class CIntArray : public CArray<int32_t> {};
class CUIntArray : public CArray<uint32_t> {};
class CFloatArray : public CArray<float> {};

//-----------------------------------------------------------------------------

template < class DATA_T, uint32_t length > 
class CStaticArray : public CArray < DATA_T > {

	template < class DATA_T, uint32_t _length > 
	class CStaticArrayData {
		public:
			DATA_T		buffer [_length];
			};

	protected:
		CStaticArrayData< DATA_T, length > m_data;

	public:
		CStaticArray () { Create (length); }

		DATA_T *Create (uint32_t _length) { 
			this->SetBuffer (m_data.buffer, 2, _length); 
			return m_data.buffer;
			}
		void Destroy (void) { }
	};

//-----------------------------------------------------------------------------


#endif //_CARRAY_H
