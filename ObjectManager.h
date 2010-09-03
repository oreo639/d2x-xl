#ifndef __objman_h
#define __objman_h

#include "define.h"
#include "cfile.h"
#include "carray.h"
#include "Selection.h"

#ifdef _DEBUG

typedef CStaticArray< CGameObject, MAX_OBJECTS_D2 > objectList;

#else

typedef CGameObject objectList [MAX_OBJECTS_D2];

#endif

class CObjectManager {
	public:
		objectList	m_objects;
		short			m_nCount;

		inline bool IsValid (short i) { return (i >= 0) && (i < m_nCount); }

		inline objectList& Objects (void) { return m_objects; }
		inline CGameObject *GetObject (int i) 
			{ return &m_objects [i]; }

		inline short& ObjCount () { return m_nCount; }

		short FindObjBySig (short nSignature);
		void InitRobotData (void);
		int ReadHxmFile (CFileManager& fp, long fSize);
		int WriteHxmFile (CFileManager& fp);
		void SortObjects ();
		void Sort (short left, short right);
		int CmpObjects (CGameObject *pi, CGameObject *pm);
		bool IsCustomRobot (int i);
		bool HasCustomRobots (void);
		inline short Index (CGameObject* objP) { return (objP == null) ? -1 : objP - m_objects; }

		void Read (CFileManager& fp, CMineItemInfo& info, int nFileVersion);
		void Write (CFileManager& fp, CMineItemInfo& info, int nFileVersion);
		void Clear (void);
};

extern CObjectManager objectManager;

#endif //__objman_h