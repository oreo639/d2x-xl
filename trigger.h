#ifndef __trigger_h
#define __trigger_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "cfile.h"


//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CTriggerTargets {
public:
	short		m_count;
	CSideKey	m_targets [MAX_TRIGGER_TARGETS];

	CTriggerTargets () { m_count = 0; }
	void Clear (void);
	int Read (CFileManager& fp);
	void Write (CFileManager& fp);

	short Add (CSideKey key);
	short Delete (int i = -1);
	int Find (CSideKey key);

	inline short Add (short nSegment, short nSide) { return Add (CSideKey (nSegment, nSide)); }
	inline short Pop (void) { return Delete (m_count - 1); }
	inline int Find (short nSegment, short nSide) { return Find (CSideKey (nSegment, nSide)); }
	inline short& Segment (uint i) { return m_targets [i].m_nSegment; }
	inline short& Side (uint i) { return m_targets [i].m_nSide; }

	inline CSideKey& operator[](uint i) { return m_targets [i]; }
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

typedef struct tTrigger {
	byte		type;
	ushort	flags;
	short		nObject;
	fix		value;
	fix		time;
	ushort	nIndex;
} tTrigger;

//------------------------------------------------------------------------

class CTrigger : public CTriggerTargets, public CGameItem {
public:
	struct tTrigger m_info;
	//inline CSideKey& operator[](uint i) { return targets [i]; }

	virtual CGameItem* Next (void) { return this + 1; }
	virtual int Read (CFileManager& fp, int version, bool bObjTrigger);
	virtual void Write (CFileManager& fp, int version, bool bObjTrigger);
	virtual void Clear (void) { 
		memset (&m_info, 0, sizeof (m_info)); 
		CTriggerTargets::Clear ();
		}
	void CTrigger::Setup (short type, short flags);
	inline const bool operator< (const CTrigger& other) {
		return (m_info.nObject < other.m_info.nObject) || ((m_info.nObject == other.m_info.nObject) && (m_info.type < other.m_info.type)); 
		}
	inline const bool operator> (const CTrigger& other) {
		return (m_info.nObject > other.m_info.nObject) || ((m_info.nObject == other.m_info.nObject) && (m_info.type > other.m_info.type)); 
		}
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

class CReactorTrigger : public CTriggerTargets, public CGameItem {
public:
	int Read (CFileManager& fp, int version = 0, bool bFlag = false);
	void Write (CFileManager& fp, int version = 0, bool bFlag = false);
	virtual void Clear (void) { CTriggerTargets::Clear (); }
	virtual CGameItem* Next (void) { return this + 1; }
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif // __trigger_h

