#ifndef __botman_h
#define __botman_h

#include "Robot.h"
#include "PolyModel.h"

//------------------------------------------------------------------------

#define MAX_MATCENS ((theMine == null) ? MAX_NUM_MATCENS_D2 : (theMine->IsD1File () || (theMine->LevelVersion () < 12)) ? MAX_NUM_MATCENS_D1 : MAX_NUM_MATCENS_D2)

#define ROBOT_IDS2 ((theMine == null) ? MAX_ROBOT_IDS_TOTAL : (theMine->LevelVersion () == 7) ? N_ROBOT_TYPES_D2 : MAX_ROBOT_IDS_TOTAL)

//------------------------------------------------------------------------

#ifdef _DEBUG

typedef CStaticArray< CRobotInfo, MAX_ROBOT_TYPES > robotInfoList;

#else

typedef CRobotInfo robotInfoList [MAX_ROBOT_TYPES];

#endif

class CRobotManager {
	private:
		robotInfoList	m_robotInfo [2];

		byte*				m_hxmExtraData;
		int				m_hxmExtraDataSize;
		int				m_nRobotTypes;

	public:
		inline robotInfoList& RobotInfoList (void) { return m_robotInfo [0]; }

		inline robotInfoList& DefRobotInfoList (void) { return m_robotInfo [1]; }

		inline CRobotInfo* RobotInfo (int i, int j = 0) { return &m_robotInfo [j][i]; }

		inline CRobotInfo* DefRobotInfo (int i) { return RobotInfo (i, 1); }

		void Init (void); 

		bool IsCustomRobot (int nId);

		bool HasCustomRobots (void);

		void LoadResource (int nRobot); 

		int ReadHAM (char *pszFile, int type); 

		int WriteHXM (CFileManager& fp); 

		int ReadHXM (CFileManager& fp, long size); 
	};

extern CRobotManager robotManager;

//------------------------------------------------------------------------

#endif //__botman_h