#ifndef __types_h
#define __types_h

#include "define.h"
#include "GameItem.h"
#include "FileManager.h"

# pragma pack(push, packing)
# pragma pack(1)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CSideKey {
public:
	short	m_nSegment;
	short	m_nSide;

	CSideKey (short nSegment = -1, short nSide = -1) : m_nSegment(nSegment), m_nSide(nSide) {}

	inline bool operator == (CSideKey& other) { return (m_nSegment == other.m_nSegment) && (m_nSide == other.m_nSide); }
	
	inline bool operator != (CSideKey& other) { return (m_nSegment != other.m_nSegment) || (m_nSide != other.m_nSide); }
	
	inline bool operator < (CSideKey& other) { return (m_nSegment < other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide < other.m_nSide)); }
	
	inline bool operator <= (CSideKey& other) { return (m_nSegment < other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide <= other.m_nSide)); }
	
	inline bool operator > (CSideKey& other) { return (m_nSegment > other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide > other.m_nSide)); }
	
	inline bool operator >= (CSideKey& other) { return (m_nSegment > other.m_nSegment) || ((m_nSegment == other.m_nSegment) && (m_nSide >= other.m_nSide)); }

	void Read (CFileManager* fp) {
		m_nSegment = fp->ReadInt16 ();
		m_nSide = fp->ReadInt16 ();
		}

	void Write (CFileManager* fp) {
		fp->Write (m_nSegment);
		fp->Write (m_nSide);
		}

	void Clear (void) {
		m_nSegment = m_nSide = -1;
		}
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tUVL {
public:
	short u, v, l; 
} tUVL;

class CUVL : public tUVL {
public:
	inline void Read (CFileManager* fp) {
		u = fp->ReadInt16 ();
		v = fp->ReadInt16 ();
		l = fp->ReadInt16 ();
		}

	inline void Write (CFileManager* fp) {
		fp->Write (u);
		fp->Write (v);
		fp->Write (l);
		}

	inline void Clear (void) { u = v = l = 0; }
	inline void Set (short _u, short _v, short _l) { u = _u, v = _v, l = _l; }
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct rgbColor {
	double	r, g, b;
} rgbColor;

// -----------------------------------------------------------------------------

typedef struct tColor {
	byte		index;
	rgbColor	color;
} tColor;

// -----------------------------------------------------------------------------

class CColor : public CGameItem {
	public:
		tColor	m_info;

		void Read (CFileManager& fp, bool bFlag = false);

		void Write (CFileManager& fp, bool bFlag = false);

		virtual void Clear (void) { memset (&m_info, 0, sizeof (m_info)); }

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);

		CColor (eItemType itemType = itUndefined) : CGameItem (itemType) {}

		inline byte Red (void) { return (byte) (m_info.color.r * 255); }

		inline byte Green (void) { return (byte) (m_info.color.g * 255); }

		inline byte Blue (void) { return (byte) (m_info.color.b * 255); }
};


class CFaceColor : public CColor {
	public:
		CFaceColor () : CColor (itFaceColor) {}

		inline const CFaceColor& operator= (const CColor& other) { 
			m_info = other.m_info; 
			return *this;
			}
};

class CTextureColor : public CColor {
	public:
		CTextureColor () : CColor (itTextureColor) {}

		inline const CTextureColor& operator= (const CColor& other) { 
			m_info = other.m_info; 
			return *this;
			}
};

class CVertexColor : public CColor {
	public:
		CVertexColor () : CColor (itVertexColor) {}

		inline const CVertexColor& operator= (const CColor& other) { 
			m_info = other.m_info; 
			return *this;
			}
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

struct level_header {
  char name[13];
  int size;
};

struct sub {
  INT64 offset;
  char name[13];
  int size;
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#define MAX_LEVELS	1000

typedef struct {
	char	missionName [80];
	char  missionInfo [8][80];
	int	authorFlags [2];
	int	missionType;
	int	missionFlags [6];
	int	customFlags [3];
	char	levelList [MAX_LEVELS][17];	//18 == ########.###,####'\0' == levlname.ext,secr
	char	comment [4000];
	int   numLevels;
	int	numSecrets;
} MISSION_DATA;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class CVertex;
class CSegment;
class CSide;
class CWall;
class CTrigger;
class CObject;
class CSegmentManager;
class CWallManager;
class CTriggerManager;
class CMine;

extern CMine* theMine;

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __types_h

