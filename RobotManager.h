#ifndef __botman_h
#define __botman_h

#ifdef _DEBUG

typedef CStaticArray< CRobotInfo, MAX_ROBOT_TYPES > robotInfoList;

#else

typedef CRobotInfo robotInfoList [MAX_ROBOT_TYPES];

#endif

class CRobotManager {
	public:
		robotInfoList	m_robotInfo;
		byte*	m_pHxmExtraData;
		int	m_nHxmExtraDataSize;
		int	m_nRobotTypes;

		void Init (void); 
		void LoadResource (int nRobot); 
		int ReadHAM (char *pszFile, int type); 
		int WriteHXM (CFileManager& fp); 
		int ReadHXM (CFileManager& fp, long size); 

	};

#endif //__botman_h