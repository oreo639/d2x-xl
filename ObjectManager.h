#ifndef __objman_h
#define __objman_h

#include "define.h"
#include "cfile.h"
#include "carray.h"
#include "Selection.h"
#include "MineInfo.h"

//------------------------------------------------------------------------

#define MAX_OBJECTS ((theMine == null) ? MAX_OBJECTS_D2 : theMine->IsStdLevel () ? MAX_OBJECTS_D1 : MAX_OBJECTS_D2)

#define MAX_PLAYERS ((theMine == null) ? MAX_PLAYERS_D2 : theMine->IsStdLevel () ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X)

//------------------------------------------------------------------------

#ifdef _DEBUG

typedef CStaticArray< CGameObject, MAX_OBJECTS_D2 > objectList;

#else

typedef CGameObject objectList [MAX_OBJECTS_D2];

#endif

class CObjectManager {
	private:
		objectList		m_objects;
		CMineItemInfo	m_info;
		bool				m_bSortObjects;

	public:
		inline void Reset (void) { m_info.Reset (); }

		inline objectList& ObjectList (void) { return m_objects; }

		inline CGameObject *Object (int i)  { return &m_objects [i]; }

		inline int& Count (void) { return m_info.count; }

		void Delete (short nDelObj);

		CGameObject* FindBySeg (short nSegment, short i = 0);

		CGameObject* FindBySig (short nSignature);

		CGameObject* FindRobot (short nId, short i = 0);

		void Sort (short left, short right);

		inline short Index (CGameObject* objP) { return (objP == null) ? -1 : objP - &m_objects [0]; }

		void SetIndex (void);
	
		bool HaveResources (void);

		inline short Add (void);

		bool Create (byte newType, short nSegment);

		inline void ReadInfo (CFileManager& fp) { m_info.Read (fp); }

		inline void WriteInfo (CFileManager& fp) { m_info.Write (fp); }

		void Read (CFileManager& fp, int nFileVersion);

		void Write (CFileManager& fp, int nFileVersion);

		void Clear (void);

		bool& SortObjects (void) { return m_bSortObjects; }

	private:
		int Compare (CGameObject& pi, CGameObject& pm);

		void Sort (void);
};

extern CObjectManager objectManager;

//------------------------------------------------------------------------

#endif //__objman_h