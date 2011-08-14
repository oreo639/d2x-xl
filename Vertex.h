#ifndef __vertex_h
#define __vertex_h

// -----------------------------------------------------------------------------

class CVertex : public CDoubleVector, public CGameItem {
	private:
		byte m_status;

	public:
		void Read (CFileManager* fp = 0, bool bFlag = false);

		void Write (CFileManager* fp = 0, bool bFlag = false);

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

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);

		virtual void Undo (void);

		virtual void Redo (void);

		inline void Mark (byte mask = MARKED_MASK) { m_status |= mask; }

		inline void Unmark (byte mask = MARKED_MASK) { m_status &= ~mask; }

		inline bool IsMarked (byte mask = MARKED_MASK) { return (m_status & mask) != 0; }


		// c'tors
		CVertex () : CGameItem (itVertex), m_status(0) {}
		
		CVertex (double x, double y, double z) : CDoubleVector (x, y, z) { m_status = 0; }
		
		CVertex (tDoubleVector& _v) : CDoubleVector (_v) { m_status = 0; }
		
		CVertex (CDoubleVector _v) : CDoubleVector (_v) { m_status = 0; }
	};

// -----------------------------------------------------------------------------

#endif //__vertex_h