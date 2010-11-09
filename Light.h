#ifndef __light_h
#define __light_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "Vector.h"
#include "FileManager.h"
#include "carray.h"

#define DEFAULT_LIGHT_RENDER_DEPTH 6

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tTextureLight {
  int    nBaseTex;
  long   light;
} tTextureLight;

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

		void Read (CFileManager* fp, bool bFlag = false);

		void Write (CFileManager* fp, bool bFlag = false);

		virtual void Clear (void) { 
			memset (&m_info, 0, sizeof (m_info)); 
			CSideKey::Clear ();
			}

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);
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

		void Read (CFileManager* fp, bool bD2X);

		void Write (CFileManager* fp, bool bD2X);

		virtual void Clear (void) { 
			memset (&m_info, 0, sizeof (m_info)); 
			CSideKey::Clear ();
			}

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef struct tVariableLight {
	uint mask;    // bits with 1 = on, 0 = off
	int timer;		 // always set to 0
	int delay;      // time for each bit in mask (short seconds)
} tVariableLight;

// -----------------------------------------------------------------------------

class CVariableLight : public CSideKey, public CGameItem {
public:
	tVariableLight m_info;

	void Read (CFileManager* fp);
	
	void Write (CFileManager* fp);
	
	void Clear (void);
	
	void Setup (CSideKey key, short time, short mask);

	virtual CGameItem* Copy (CGameItem* destP);

	virtual CGameItem* Clone (void);
	
	virtual void Backup (eEditType editType);

	virtual void Undo (void);

	virtual void Redo (void);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __light_h

