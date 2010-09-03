#include "cfile.h"
#include "MineInfo.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CMineFileInfo::Read (CFileManager& fp) 
{
signature = fp.ReadInt16 ();
version = fp.ReadInt16 ();
size = fp.ReadInt32 ();
}

// -----------------------------------------------------------------------------

void CMineFileInfo::Write (CFileManager& fp) 
{
fp.Write (signature);
fp.Write (version);
fp.Write (size);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CPlayerItemInfo::Read (CFileManager& fp) 
{
offset = fp.ReadInt32 ();
size  = fp.ReadInt32 ();
}

// -----------------------------------------------------------------------------

void CPlayerItemInfo::Write (CFileManager& fp) 
{
fp.Write (offset);
fp.Write (size);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CMineItemInfo::Read (CFileManager& fp) 
{
offset = fp.ReadInt32 ();
count = fp.ReadInt32 ();
size  = fp.ReadInt32 ();
}

// -----------------------------------------------------------------------------

void CMineItemInfo::Write (CFileManager& fp) 
{
fp.Write (offset);
fp.Write (count);
fp.Write (size);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CMineInfo::Read (CFileManager& fp) 
{
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
botGen.Read (fp);
lightDeltaIndices.Read (fp);
lightDeltaValues.Read (fp);
if (fileInfo.size > 143)
	equipGen.Read (fp);
}

// -----------------------------------------------------------------------------

void CMineInfo::Write (CFileManager& fp) 
{
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
botGen.Write (fp);
lightDeltaIndices.Write (fp);
lightDeltaValues.Write (fp);
if (fileInfo.size > 143)
	equipGen.Write (fp);
}

// -----------------------------------------------------------------------------

#endif // __mineinfo_h

