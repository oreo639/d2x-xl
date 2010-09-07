#ifndef __trigger_h
#define __trigger_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "FileManager.h"
#include "Types.h"

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#define MAX_TRIGGER_TARGETS	10

class CTriggerTargets {
public:
	short		m_count;
	CSideKey	m_targets [MAX_TRIGGER_TARGETS];

	CTriggerTargets () { m_count = 0; }
	void Clear (void);
	void Read (CFileManager& fp);
	void Write (CFileManager& fp);

	short Add (CSideKey key);
	short Delete (short i = -1);
	int Delete (CSideKey key);
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
	int		value;
	int		time;
	short		nObject;
} tTrigger;

//------------------------------------------------------------------------

class CTrigger : public CTriggerTargets, public CGameItem {
	private:
		struct tTrigger m_info;
		//inline CSideKey& operator[](uint i) { return targets [i]; }

	public:
		inline tTrigger& Info (void) { return m_info; }

		void Read (CFileManager& fp, int version, bool bObjTrigger);

		void Write (CFileManager& fp, int version, bool bObjTrigger);

		virtual void Clear (void);

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);

		void Setup (short type, short flags);

		bool IsExit (void);

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

		void Read (CFileManager& fp, int version = 0, bool bFlag = false);

		void Write (CFileManager& fp, int version = 0, bool bFlag = false);

		virtual void Clear (void) { CTriggerTargets::Clear (); }

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------

#endif // __trigger_h

