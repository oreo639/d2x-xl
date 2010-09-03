#ifndef __vertex_h
#define __vertex_h

// -----------------------------------------------------------------------------

class CVertex : public CDoubleVector, public CGameItem {
public:
	byte m_status;
	CVertex () : m_status(0) {}
	CVertex (double x, double y, double z) : CDoubleVector (x, y, z) { m_status = 0; }
	CVertex (tDoubleVector& _v) : CDoubleVector (_v) { m_status = 0; }
	CVertex (CDoubleVector _v) : CDoubleVector (_v) { m_status = 0; }

	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false) { fp.ReadVector (v); }

	virtual void Write (CFileManager& fp, int version = 0, bool bFlag = false) { fp.WriteVector (v); }

	void WriteText (CFileManager& fp) {
		}
	
for (i = 0; i < 8; i++) {
	// each vertex relative to the origin has a x', y', and z' component
	// which is a constant (k) times the axis
	// k = (B*A)/(A*A) where B is the vertex relative to the origin
	//                       A is the axis unit vector (always 1)
	CVertex& v = *GetVertex (m_info.verts [i]) - origin;
	fprintf (fp.File (), "  vms_vector %d %ld %ld %ld\n", i, D2X (v ^ m.rVec), D2X (v ^ m.uVec), D2X (v ^ m.fVec));
	}

	virtual void Clear (void) { 
		m_status = 0;
		this->CDoubleVector::Clear ();
		}

	inline const CVertex& operator= (const CVertex& other) { 
		v = other.v, m_status = other.m_status; 
		return *this;
		}
	inline const CVertex& operator= (const CDoubleVector& other) { 
		v = other.v; 
		return *this;
		}
};

// -----------------------------------------------------------------------------

#endif //__vertex_h