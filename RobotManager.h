#ifndef __botman_h
#define __botman_h

class CRobotManager {
	public:
		void Init (void); 
		void LoadResource (int nRobot); 
		int ReadHAM (char *pszFile, int type); 
		int WriteHXM (CFileManager& fp); 
		int ReadHXM (CFileManager& fp, long size); 

	};

#endif //__botman_h