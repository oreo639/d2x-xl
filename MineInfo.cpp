
#include "mine.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CMineFileInfo::Read (CFileManager* fp) 
{
signature = fp->ReadInt16 ();
version = fp->ReadInt16 ();
size = fp->ReadInt32 ();
}

// -----------------------------------------------------------------------------

void CMineFileInfo::Write (CFileManager* fp) 
{
fp->Write (signature);
fp->Write (version);
fp->Write (size);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CPlayerItemInfo::Read (CFileManager* fp) 
{
offset = fp->ReadInt32 ();
size  = fp->ReadInt32 ();
}

// -----------------------------------------------------------------------------

void CPlayerItemInfo::Write (CFileManager* fp) 
{
fp->Write (offset);
fp->Write (size);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CMineItemInfo::Read (CFileManager* fp) 
{
offset = fp->ReadInt32 ();
count = fp->ReadInt32 ();
size  = fp->ReadInt32 ();
}

// -----------------------------------------------------------------------------

void CMineItemInfo::Write (CFileManager* fp) 
{
if (count == 0)
	offset = -1;
fp->Write (offset);
fp->Write (count);
fp->Write (size);
}

// -----------------------------------------------------------------------------

bool CMineItemInfo::Setup (CFileManager* fp) 
{
offset = (count == 0) ? -1 : fp->Tell ();
return offset >= 0;
}

// -----------------------------------------------------------------------------

bool CMineItemInfo::Restore (CFileManager* fp) 
{
if (offset < 0) {
	count = 0;
	return false;
	}
fp->Seek (offset);
return true;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CMineInfo::Read (CFileManager* fp, bool bIsD2XLevel) 
{
fileInfo.Read (fp);
fp->Read (mineFilename, 1, sizeof (mineFilename));
level = fp->ReadInt32 ();
player.Read (fp);
objectManager.ReadInfo (fp);
wallManager.ReadInfo (fp);
triggerManager.ReadInfo (fp);
CMineItemInfo links; // unused
links.Read (fp);
triggerManager.ReadReactorInfo (fp);
segmentManager.ReadRobotMakerInfo (fp);
if (fileInfo.version >= 29)
	lightManager.ReadLightDeltaInfo (fp);
if (bIsD2XLevel && (theMine->LevelVersion () > 15))
	segmentManager.ReadEquipMakerInfo (fp);
}

// -----------------------------------------------------------------------------

void CMineInfo::Write (CFileManager* fp, bool bIsD2XLevel) 
{
	long startPos = fp->Tell ();

fileInfo.Write (fp);
fp->Write (mineFilename, 1, sizeof (mineFilename));
fp->Write (level);
player.Write (fp);
objectManager.WriteInfo (fp);
wallManager.WriteInfo (fp);
triggerManager.WriteInfo (fp);
CMineItemInfo links; // unused
links.Write (fp);
triggerManager.WriteReactorInfo (fp);
segmentManager.WriteRobotMakerInfo (fp);
if (fileInfo.version >= 29)
	lightManager.WriteLightDeltaInfo (fp);
if (bIsD2XLevel)
	segmentManager.WriteEquipMakerInfo (fp);
if (fileInfo.size < 0) {
	fileInfo.size = fp->Tell () - startPos;
	long endPos = fp->Tell ();
	fp->Seek (startPos);
	fileInfo.Write (fp);
	fp->Seek (endPos);
	}
}

// -----------------------------------------------------------------------------

