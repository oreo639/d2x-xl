#ifndef __mineinfo_h
#define __mineinfo_h

#include "define.h"

# pragma pack(push, packing)
# pragma pack(1)

#include "VectorTypes.h"
#include "cfile.h"

class CMineFileInfo {
public:
	ushort  signature;
	ushort  version;
	int		size;

	void Read (CFileManager& fp);
	void Write (CFileManager& fp);
};

class CPlayerItemInfo {
public:
	int	 offset;
	int  size;

	CPlayerItemInfo () { offset = -1, size = 0; }
	void Read (CFileManager& fp);
	void Write (CFileManager& fp);
};

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

class CGameInfo {
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
	CMineItemInfo		botgen;
	CMineItemInfo		lightDeltaIndices;
	CMineItemInfo		lightDeltaValues;
	CMineItemInfo		equipgen;

	void Read (CFileManager& fp);
	void Write (CFileManager& fp);
};

#endif // __mineinfo_h

