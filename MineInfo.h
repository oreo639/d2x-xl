#ifndef __mineinfo_h
#define __mineinfo_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

// -----------------------------------------------------------------------------

class CMineFileInfo {
public:
	ushort  signature;
	ushort  version;
	int		size;

	void Read (CFileManager& fp);
	void Write (CFileManager& fp);
};

// -----------------------------------------------------------------------------

class CPlayerItemInfo {
public:
	int	 offset;
	int  size;

	CPlayerItemInfo () { offset = -1, size = 0; }
	void Read (CFileManager& fp);
	void Write (CFileManager& fp);
};

// -----------------------------------------------------------------------------

class CMineItemInfo {
public:
	int	offset;
	int	count;
	int	size;

	CMineItemInfo () { Reset (); }

	void Reset (void) { offset = -1, count = size = 0; } 
	void Read (CFileManager& fp);
	void Write (CFileManager& fp);
};

// -----------------------------------------------------------------------------

class CMineInfo {
public:
	CMineFileInfo		fileInfo;
	char					mineFilename[15];
	int					level;
	CPlayerItemInfo	player;
	CMineItemInfo		objects;
	CMineItemInfo		walls;
	CMineItemInfo		doors;
	CMineItemInfo		triggers;
	CMineItemInfo		links;
	CMineItemInfo		control;
	CMineItemInfo		botGen;
	CMineItemInfo		lightDeltaIndices;
	CMineItemInfo		lightDeltaValues;
	CMineItemInfo		equipGen;

	void Read (CFileManager& fp);
	void Write (CFileManager& fp);
};

// -----------------------------------------------------------------------------

class CMineData {
	public:
		CMineInfo					mineInfo;
		int							m_reactorTime;
		int							m_reactorStrength;
		int							m_secretSegNum;
		CDoubleMatrix				m_secretOrient;
		
		// robot data
		// structure data
};

// -----------------------------------------------------------------------------

#endif // __mineinfo_h

