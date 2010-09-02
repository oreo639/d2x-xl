#ifndef __trigman_h
#define __trigman_h

#include "define.h"

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
		inline triggerList& Triggers (void)
			{ return m_triggers [0]; }
		inline objTriggerList& ObjTriggers (void)
			{ return m_triggers [1]; }
		inline CTrigger *Triggers (int i)
			{ return &m_triggers [0][i]; }
		inline int &NumTriggers ()
			{ return m_nTriggers [0]; }
		inline CTrigger *ObjTriggers (int i)
			{ return &m_triggers [1][i]; }
		inline int& NumObjTriggers ()
			{ return m_nTriggers [1]; }
		inline reactorTriggerList& ReactorTriggers (void)
			{ return m_reactorTriggers; }
		inline CReactorTrigger* ReactorTriggers (int i)
			{ return &m_reactorTriggers [i]; }
};

#endif //__trigman_h
