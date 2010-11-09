#ifndef __objman_h
#define __objman_h

#include "define.h"
#include "FileManager.h"
#include "carray.h"
#include "Selection.h"
#include "MineInfo.h"

//------------------------------------------------------------------------

#define MAX_OBJECTS ((theMine == null) ? MAX_OBJECTS_D2 : DLE.IsStdLevel () ? MAX_OBJECTS_D1 : MAX_OBJECTS_D2)

#define MAX_PLAYERS ((theMine == null) ? MAX_PLAYERS_D2 : DLE.IsStdLevel () ? MAX_PLAYERS_D2 : MAX_PLAYERS_D2X)

//------------------------------------------------------------------------

class CSecretExit {
	public:
		int nSegment;
		CDoubleMatrix orient;

	inline const CSecretExit& operator= (const CSecretExit& other) { 
		nSegment = other.nSegment;
		orient = other.orient;
		return *this;
		}

	inline const bool operator== (const CSecretExit& other) { return (nSegment == other.nSegment) && (orient == other.orient); }

	inline const bool operator!= (const CSecretExit& other) { return (nSegment != other.nSegment) || (orient != other.orient); }
};

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
		CSecretExit		m_secretExit;
		int				m_bSortObjects;

	public:
		inline void ResetInfo (void) { m_info.Reset (); }

		inline objectList& ObjectList (void) { return m_objects; }

		inline CSecretExit& SecretExit (void) { return m_secretExit; }

		inline int& SecretSegment (void) { return m_secretExit.nSegment; }

		inline CDoubleMatrix& SecretOrient (void) { return m_secretExit.orient; }

		inline CGameObject *Object (int i)  { return &m_objects [i]; }

		inline int& Count (void) { return m_info.count; }

		void Delete (short nDelObj = -1, bool bUndo = true);

		CGameObject* FindBySeg (short nSegment, short i = 0);

		CGameObject* FindBySig (short nSignature);

		CGameObject* FindRobot (short nId, short i = 0);

		void Sort (short left, short right);

		inline short Index (CGameObject* objP) { return (objP == null) ? -1 : objP - &m_objects [0]; }

		void SetIndex (void);
	
		bool HaveResources (void);

		int& SortObjects (void) { return m_bSortObjects; }

		inline short Add (bool bUndo = true);

		bool Create (byte newType, short nSegment = -1);

		void Move (CGameObject * objP = null);

		inline void ReadInfo (CFileManager* fp) { m_info.Read (fp); }

		inline void WriteInfo (CFileManager* fp) { m_info.Write (fp); }

		void Read (CFileManager* fp);

		void Write (CFileManager* fp);

		void Clear (void);

		void Sort (void);

		void UpdateSegments (short nOldSeg, short nNewSeg);

	private:
		int Compare (CGameObject& pi, CGameObject& pm);
};

extern CObjectManager objectManager;

//------------------------------------------------------------------------

#endif //__objman_h