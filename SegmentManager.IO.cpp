// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

void CSegmentManager::ReadSegments (CFileManager& fp, int nFileVersion)
{
if (m_segmentInfo.offset >= 0) {
	fp.Seek (m_info.offset);

	int nLevelType = theMine->IsD2XLevel () ? 2 : theMine->IsD2File () ? 1 : 0;
	int nLevelVersion = theMine->LevelVersion ();
	int i;

	for (i = 0; i < Count (); i++)
		m_segments [i].Read (fp, nLevelType, nLevelVersion);
	if (!theMine->IsD2File ())
		return;
	for (i = 0, segP = Segments (0); i < Count (); i++)   
		m_segments [i].ReadExtras (fp, nLevelVersion, true);
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::WriteSegments (CFileManager& fp, int nFileVersion)
{
if (Count () == 0)
	m_segmentInfo.offset = -1;
else {
	m_segmentInfo.offset = fp.Tell ();
	for (int i = 0; i < Count (); i++)
		m_segments [i].Write (fp, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::ReadMatCens (CFileManager& fp, int nFileVersion, int nClass)
{
if (m_matCenInfo [nClass].offset >= 0) {
	for (int i = 0; i < MatCenCount (nClass); i++)
		m_matCens [nClass][i].Read (fp, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::WriteMatCens (CFileManager& fp, int nFileVersion, int nClass)
{
if (m_matCenInfo [nClass].count == 0)
	m_matCenInfo [nClass].offset = -1;
else {
	m_matCenInfo [nClass].offset = fp.Tell ();
	for (int i = 0; i < MatCenCount (nClass); i++)
		m_matCens [nClass][i].Write (fp, nFileVersion);
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::ReadRobotMakers (CFileManager& fp, int nFileVersion)
{
ReadMatCens (fp, nFileVersion, 1);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::WriteRobotMakers (CFileManager& fp, int nFileVersion)
{
WriteMatCens (fp, nFileVersion, 1);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::ReadEquipMakers (CFileManager& fp, int nFileVersion)
{
ReadMatCens (fp, nFileVersion, 1);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::WriteEquipMakers (CFileManager& fp, int nFileVersion)
{
WriteMatCens (fp, nFileVersion, 1);
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::Clear (void)
{
for (int i = 0; i < Count (); i++)
	m_segments [i].Clear ();
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp