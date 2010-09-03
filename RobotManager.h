#ifndef __botman_h
#define __botman_h

#include "Robot.h"
#include "PolyModel.h"

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
		inline robotInfoList& RobotInfo (void) { return m_robotInfo [0]; }

		inline robotInfoList& DefRobotInfo (void) { return m_robotInfo [1]; }

		inline CRobotInfo* GetRobotInfo (int i, int j = 0) { return &m_robotInfo [j][i]; }

		inline CRobotInfo* GetDefRobotInfo (int i) { return GetRobotInfo (i, 1); }

		void Init (void); 

		bool IsCustomRobot (int nId);

		bool HasCustomRobots (void);

		void LoadResource (int nRobot); 

		int ReadHAM (char *pszFile, int type); 

		int WriteHXM (CFileManager& fp); 

		int ReadHXM (CFileManager& fp, long size); 
	};

#endif //__botman_h