#ifndef __trigman_h
#define __trigman_h

#include "define.h"

#define MAX_TRIGGERS_D1			100
#define MAX_TRIGGERS_D2			254
#define MAX_OBJ_TRIGGERS		254
#define MAX_REACTOR_TRIGGERS	10
#define TRIGGER_LIMIT			MAX_TRIGGERS_D2
#define MAX_TRIGGER_FLAGS		12
#define NO_TRIGGER				(TRIGGER_LIMIT + 1)

#define MAX_TRIGGERS	((theMine == null) ? MAX_TRIGGERS_D2 : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_TRIGGERS_D1 : MAX_TRIGGERS_D2)

#ifdef USE_DYN_ARRAYS

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
		short						m_nTriggers [2];
		reactorTriggerList	m_reactorTriggers;
		short						m_nReactorTriggers;

	public:
		inline bool IsValid (short i, j) { return (i >= 0) && (i < j); }

		inline triggerList& Triggers (void) { return m_triggers [0]; }

		inline objTriggerList& ObjTriggers (void) { return m_triggers [1]; }

		inline CTrigger *Triggers (int i, int j = 0) { return IsValid (i, m_nTriggers [j]) ? &m_triggers [j][i] : null; }

		inline int &NumTriggers (int i = 0) { return m_nTriggers [i]; }

		inline CTrigger *ObjTriggers (int i) { return Triggers (i, 1); }

		inline int& NumObjTriggers (void) { return NumTriggers (1); }

		inline reactorTriggerList& ReactorTriggers (void) { return m_reactorTriggers; }

		inline CReactorTrigger* ReactorTriggers (int i) { return &m_reactorTriggers [i]; }

		void CTriggerManager::SortObjTriggers (void);

	private:
		int CTriggerManager::CmpObjTriggers (CTrigger *pi, CTrigger *pm);
		void CTriggerManager::SortObjTriggers (short left, short right);

};

extern CTriggerManager triggerManager;

#endif //__trigman_h
