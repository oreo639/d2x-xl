#ifndef __vertex_h
#define __vertex_h

// -----------------------------------------------------------------------------

class CVertex : public CDoubleVector, public CGameItem {
	private:
		byte m_status;

	public:
		void Read (CFileManager& fp, int version = 0, bool bFlag = false);

		void Write (CFileManager& fp, int version = 0, bool bFlag = false);

		inline const CVertex& operator= (const CVertex& other) { 
			v = other.v, m_status = other.m_status; 
			return *this;
			}
		inline const CVertex& operator= (const CDoubleVector& other) { 
			v = other.v; 
			return *this;
			}

		inline byte& Status (void) { return m_status; }

		virtual void Clear (void);

		virtual CGameItem* Clone (eEditType editType = opModify);

		virtual void Backup (eEditType editType);

		virtual void Copy (CGameItem* destP);

		// c'tors
		CVertex () : CGameItem (itVertex), m_status(0) {}
		
		CVertex (double x, double y, double z) : CDoubleVector (x, y, z) { m_status = 0; }
		
		CVertex (tDoubleVector& _v) : CDoubleVector (_v) { m_status = 0; }
		
		CVertex (CDoubleVector _v) : CDoubleVector (_v) { m_status = 0; }
	};

// -----------------------------------------------------------------------------

#endif //__vertex_h