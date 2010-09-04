#ifndef __trigman_h
#define __trigman_h

#include "define.h"
#include "cfile.h"
#include "carray.h"
#include "trigger.h"
#include "MineInfo.h"

//------------------------------------------------------------------------

#define MAX_TRIGGERS_D1			100
#define MAX_TRIGGERS_D2			254
#define MAX_OBJ_TRIGGERS		254
#define MAX_REACTOR_TRIGGERS	10
#define TRIGGER_LIMIT			MAX_TRIGGERS_D2
#define MAX_TRIGGER_FLAGS		12
#define NO_TRIGGER				(TRIGGER_LIMIT + 1)

#define MAX_TRIGGERS	((theMine == null) ? MAX_TRIGGERS_D2 : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_TRIGGERS_D1 : MAX_TRIGGERS_D2)

//------------------------------------------------------------------------

#ifdef _DEBUG

typedef CStaticArray< CTrigger, TRIGGER_LIMIT > triggerList;
typedef CStaticArray< CTrigger, TRIGGER_LIMIT > objTriggerList;
typedef CStaticArray< CReactorTrigger, MAX_REACTOR_TRIGGERS > reactorTriggerList;

#else

typedef CTrigger triggerList [MAX_TRIGGERS_D2];
typedef CTrigger objTriggerList [MAX_OBJ_TRIGGERS];
typedef CReactorTrigger reactorTriggerList [MAX_REACTOR_TRIGGERS];

#endif

//------------------------------------------------------------------------

class CTriggerManager {
	private:
		triggerList				m_triggers [2];
		reactorTriggerList	m_reactorTriggers;
		CMineItemInfo			m_info [2];
		short						m_nReactorTriggers;

	public:
		inline triggerList& TriggerList (int i) { return m_triggers [i]; }

		inline triggerList& GeoTriggerList (void) { return TriggerList (1); }

		inline objTriggerList& ObjTriggerList (void) { return TriggerList (1); }

		inline CTrigger* Trigger (int i, int nClass = 0) { return &m_triggers [nClass][i]; }

		inline int& Count (int nClass) { return m_info [nClass].count; }

		inline int& NumGeoTriggers (void) { return Count (0); }

		inline int& NumObjTriggers (void) { return Count (1); }

		inline CTrigger* GeoTrigger (int i) { return Trigger (i, 0); }

		inline CTrigger* ObjTrigger (int i) { return Trigger (i, 1); }

		short Index (CTrigger* trigP) { return (short) (trigP - &m_triggers [0][0]); }

		void SetIndex (void);

		inline reactorTriggerList& ReactorTriggers (void) { return m_reactorTriggers; }

		inline CReactorTrigger* ReactorTriggers (int i) { return &m_reactorTriggers [i]; }

		void SortObjTriggers (void);

		void RenumberObjTriggers (void);

		void RenumberTargetObjs (void);

		void DeleteTarget (CSideKey key);

		void DeleteTarget (CSideKey key, short nClass);

		inline void DeleteTarget (short nSegment, short nSide) { DeleteTarget (CSideKey (nSegment, nSide)); }

		short Add (void);

		inline CTrigger* Create (short nItem, short type, bool bAddWall = false) {
			return (nItem < 0) ? AddToObject (-nItem - 1, type) : AddToWall (nItem, type, bAddWall);
			}

		inline void Delete (short nDelTrigger) {
			if (nDelTrigger < 0)
				DeleteFromObject (-nDelTrigger - 1);
			else
				DeleteFromWall (nDelTrigger);
			}

		void DeleteObjTriggers (short nObject);

		inline void DeleteTargets (short nSegment, short nSide) { DeleteTargets (GeoTriggerList (), Count (0), nSegment, nSide); }

		bool AutoAddTrigger (short wallType, ushort wallFlags, ushort triggerType);
		bool AddDoorTrigger (short wallType, ushort wallFlags, ushort triggerType);
		bool AddOpenDoor (void); 
		bool AddRobotMaker (void); 
		bool AddShieldDrain (void); 
		bool AddEnergyDrain (void); 
		bool AddUnlock (void); 
		bool AddExit (short type); 
		bool AddNormalExit (void); 
		bool AddSecretExit (void); 

		short FindBySide (short& nTrigger, CSideKey key);

		CTrigger* FindByTarget (CSideKey key, short i = 0);

		void UpdateReactor (void);

		bool HaveResources (void);

		void Read (CFileManager& fp, int nFileVersion);
		void Write (CFileManager& fp, int nFileVersion);
		void Clear (void);

	private:
		int CmpObjTriggers (CTrigger& pi, CTrigger& pm);
		void SortObjTriggers (short left, short right);
		CTrigger* AddToWall (short nWall, short type, bool bAddWall);
		CTrigger* AddToObject (short nObject, short type);
		void DeleteFromWall (short nDelTrigger);
		void DeleteFromObject (short nDelTrigger);
		void DeleteTargets (triggerList triggers, short nTriggers, short nSegment, short nSide);
		void Clear (int nType);
};

extern CTriggerManager triggerManager;

//------------------------------------------------------------------------

#endif //__trigman_h
