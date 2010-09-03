#ifndef __trigman_h
#define __trigman_h

#include "define.h"
#include "cfile.h"
#include "carray.h"
#include "trigger.h"
#include "MineInfo.h"

#define MAX_TRIGGERS_D1			100
#define MAX_TRIGGERS_D2			254
#define MAX_OBJ_TRIGGERS		254
#define MAX_REACTOR_TRIGGERS	10
#define TRIGGER_LIMIT			MAX_TRIGGERS_D2
#define MAX_TRIGGER_FLAGS		12
#define NO_TRIGGER				(TRIGGER_LIMIT + 1)

#define MAX_TRIGGERS	((theMine == null) ? MAX_TRIGGERS_D2 : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_TRIGGERS_D1 : MAX_TRIGGERS_D2)

#ifdef _DEBUG

typedef CStaticArray< CTrigger, TRIGGER_LIMIT > triggerList;
typedef CStaticArray< CTrigger, TRIGGER_LIMIT > objTriggerList;
typedef CStaticArray< CReactorTrigger, MAX_REACTOR_TRIGGERS > reactorTriggerList;

#else

typedef CTrigger triggerList [MAX_TRIGGERS_D2];
typedef CTrigger objTriggerList [MAX_OBJ_TRIGGERS];
typedef CReactorTrigger reactorTriggerList [MAX_REACTOR_TRIGGERS];

#endif

class CTriggerManager {
	public:
		triggerList				m_triggers [2];
		reactorTriggerList	m_reactorTriggers;
		short						m_nCount [2];
		short						m_nReactorTriggers;

	public:
		inline bool IsValid (short i, short j) { return (i >= 0) && (i < j); }

		inline triggerList& Triggers (void) { return m_triggers [0]; }

		inline objTriggerList& ObjTriggers (void) { return m_triggers [1]; }

		inline CTrigger* GetTrigger (int i, int nClass = 0) { return IsValid (i, m_nCount [nClass]) ? &m_triggers [nClass][i] : null; }

		inline short& Count (int nClass) { return m_nCount [nClass]; }

		inline short& NumTriggers (void) { return Count (0); }

		inline CTrigger* GetObjTrigger (int i) { return GetTrigger (i, 1); }

		inline short& NumObjTriggers (void) { return Count (1); }

		inline reactorTriggerList& ReactorTriggers (void) { return m_reactorTriggers; }

		inline CReactorTrigger* ReactorTriggers (int i) { return &m_reactorTriggers [i]; }

		void SortObjTriggers (void);

		void DeleteTarget (CSideKey key);

		void DeleteTarget (CSideKey key, short nClass);

		inline void DeleteTarget (short nSegment, short nSide) { DeleteTarget (CSideKey (nSegment, nSide)); }

		inline CTrigger* Add (short nItem, short type, bool bAddWall = false) {
			return (nItem < 0) ? AddToObject (-nItem - 1, type) : AddToWall (nItem, type, bAddWall);
			}

		inline void Delete (short nDelTrigger) {
			if (nDelTrigger < 0)
				DeleteFromObject (-nDelTrigger - 1);
			else
				DeleteFromWall (nDelTrigger);
			}

		void DeleteObjTriggers (short nObject);

		inline void DeleteTargets (short nSegment, short nSide) { DeleteTargets (Triggers (), Count (0), nSegment, nSide); }

		bool AutoAddTrigger (short wallType, ushort wallFlags, ushort triggerType);
		bool AddDoorTrigger (short wallType, ushort wallFlags, ushort triggerType);
		bool AddOpenDoor (void); 
		bool AddRobotMaker (void); 
		bool AddShield (void); 
		bool AddEnergy (void); 
		bool AddUnlock (void); 
		bool AddExit (short type); 
		bool AddNormalExit (void); 
		bool AddSecretExit (void); 

		short FindBySide (short& nTrigger, short nSegment, short nSide);
		CTrigger* FindByTarget (short nSegment, short nSide, short nTrigger = 0);
		short FindObjTarget (short nTrigger, short nSegment, short nSide);

		void UpdateReactor (void);

		void Read (CFileManager& fp, CMineItemInfo& info, int nFileVersion);
		void Write (CFileManager& fp, CMineItemInfo& info, int nFileVersion);
		void Clear (void);

	private:
		int CmpObjTriggers (CTrigger& pi, CTrigger& pm);
		void SortObjTriggers (short left, short right);
		CTrigger* AddToWall (short nWall, short type, bool bAddWall);
		CTrigger* AddToObject (short nObject, short type);
		void DeleteFromWall (short nDelTrigger);
		void DeleteFromObject (short nDelTrigger);
		void DeleteTargets (triggerList triggers, short nTriggers, short nSegment, short nSide);

};

extern CTriggerManager triggerManager;

#endif //__trigman_h
