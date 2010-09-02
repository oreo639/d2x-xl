#ifndef __trigman_h
#define __trigman_h

#include "define.h"

#ifdef USE_DYN_ARRAYS

typedef CStaticArray< CTrigger, MAX_TRIGGERS_D2 > triggerList;
typedef CStaticArray< CTrigger, MAX_OBJ_TRIGGERS > objTriggerList;

#else

typedef CTrigger triggerList [MAX_TRIGGERS_D2];
typedef CTrigger objTriggerList [MAX_OBJ_TRIGGERS];

#endif

class CTriggerManager {
};

#endif //__trigman_h
