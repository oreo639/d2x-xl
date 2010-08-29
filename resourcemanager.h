#ifndef __resourcemanager_h
#define __resourcemanager_h

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CResource {
private:
	HGLOBAL		m_handle;
	size_t		m_size;

public:
	byte* Load (const char* szName, const char* szCategory = "RC_DATA");

	inline byte* Load (const int nId, const char* szCategory = "RC_DATA") { return Load (MAKEINTRESOURCE (nId), szCategory); }

	inline void Unload (void) {
		if (m_hResource) {
			FreeResource (m_hResource);
			m_hResource = 0;
			}
		}

	inline size_t Size (void) { return m_size; }

	CResource () : m_handle (0), m_size(0) {}

	~CResource () { Unload (); }

};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif //__resourcemanager_h