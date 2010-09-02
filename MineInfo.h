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
	int   size;

	int Read (CFileManager& fp) {
		signature = fp.ReadInt16 ();
		version = fp.ReadInt16 ();
		size = fp.ReadInt32 ();
		return 1;
		}

	void Write (CFileManager& fp) {
		fp.Write (signature);
		fp.Write (version);
		fp.Write (size);
		}
};

class CPlayerItemInfo {
public:
	int	 offset;
	int  size;

	CPlayerItemInfo () { offset = -1, size = 0; }
	int Read (CFileManager& fp) {
		offset = fp.ReadInt32 ();
		size  = fp.ReadInt32 ();
		return 1;
		}

	void Write (CFileManager& fp) {
		fp.Write (offset);
		fp.Write (size);
		}
};

class CMineItemInfo {
public:
	int	offset;
	int	count;
	int	size;

	CMineItemInfo () { Reset (); }

	void Reset (void) { offset = -1, count = size = 0; } 

	int Read (CFileManager& fp) {
		offset = fp.ReadInt32 ();
		count = fp.ReadInt32 ();
		size  = fp.ReadInt32 ();
		return 1;
		}

	void Write (CFileManager& fp) {
		fp.Write (offset);
		fp.Write (count);
		fp.Write (size);
		}
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

	int Read (CFileManager& fp) {
		fileInfo.Read (fp);
		fp.Read (mineFilename, 1, sizeof (mineFilename));
		level = fp.ReadInt32 ();
		player.Read (fp);
		objects.Read (fp);
		walls.Read (fp);
		doors.Read (fp);
		triggers.Read (fp);
		links.Read (fp);
		control.Read (fp);
		botgen.Read (fp);
		lightDeltaIndices.Read (fp);
		lightDeltaValues.Read (fp);
		if (fileInfo.size > 143)
			equipgen.Read (fp);
		return !fp.EoF ();
		}

	void Write (CFileManager& fp) {
		fileInfo.Write (fp);
		fp.Write (mineFilename, 1, sizeof (mineFilename));
		fp.Write (level);
		player.Write (fp);
		objects.Write (fp);
		walls.Write (fp);
		doors.Write (fp);
		triggers.Write (fp);
		links.Write (fp);
		control.Write (fp);
		botgen.Write (fp);
		lightDeltaIndices.Write (fp);
		lightDeltaValues.Write (fp);
		if (fileInfo.size > 143)
			equipgen.Write (fp);
		}
};

#endif // __mineinfo_h

