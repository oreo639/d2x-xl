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
	CResource () : m_handle (0), m_size(0) {}

	~CResource () {
		if (m_hResource) {
			FreeResource (m_hResource);
			m_hResource = 0;
			}
		}

	byte* Load (const char* szName, const char* szCategory);

	inline size_t Size (void) { return m_size; }
};

//------------------------------------------------------------------------

class CDataResource : public CResource {
public:
	inline byte* Load (const char* szName) { return this->CResource::Load (szName, "RC_DATA"); }
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif //__resourcemanager_h