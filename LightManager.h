#ifndef __lightman_h
#define __lightman_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "Vector.h"
#include "cfile.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tLightDeltaValue {
	byte vertLight [4];
} tLightDeltaValue;

// -----------------------------------------------------------------------------

class CLightDeltaValue : public CSideKey, public CGameItem {
public:
	tLightDeltaValue m_info;

	virtual void Read (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Write (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
	virtual CGameItem* Next (void) { return this + 1; }
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

// Light at nSegment:nSide casts light on count sides beginning at index (in array CLightDeltaValues)
typedef struct tLightDeltaIndex {
	ushort count;
	ushort index;
} tLightDeltaIndex;

// -----------------------------------------------------------------------------

class CLightDeltaIndex : public CSideKey, public CGameItem {
public:
	tLightDeltaIndex m_info;

	virtual void Read (CFileManager& fp, int version, bool bD2X);
	virtual void Write (CFileManager& fp, int version, bool bD2X);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CSideKey::Clear ();
		}
	virtual CGameItem* Next (void) { return this + 1; }
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tFlickeringLight {
	uint mask;    // bits with 1 = on, 0 = off
	int timer;		 // always set to 0
	int delay;      // time for each bit in mask (int seconds)
} tFlickeringLight;

// -----------------------------------------------------------------------------

class CFlickeringLight : public CSideKey {
public:
	tFlickeringLight m_info;

	void Read (CFileManager& fp) {
		CSideKey::Read (fp);
		m_info.mask = fp.ReadInt32 ();
		m_info.timer = fp.ReadInt32 ();
		m_info.delay = fp.ReadInt32 ();
		}

	void Write (CFileManager& fp) {
		CSideKey::Write (fp);
		fp.Write (m_info.mask);
		fp.Write (m_info.timer);
		fp.Write (m_info.delay);
		}
	void Clear (void) {
		memset (&m_info, 0, sizeof (m_info));
		CSideKey::Clear ();
		}
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __lightman_h

