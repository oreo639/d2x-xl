// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

// ----------------------------------------------------------------------------- 

void CSegmentManager::SetIndex (void)
{
int j = 0;
for (CSegmentIterator si; si; si++)
	si->Index () = j++;
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::ReadSegments (CFileManager& fp, int nFileVersion)
{
if (m_segmentInfo.Restore (fp)) {
	int nLevelType = DLE.IsD2XLevel () ? 2 : DLE.IsD2File () ? 1 : 0;
	int nLevelVersion = DLE.LevelVersion ();
	int i;

	for (i = 0; i < Count (); i++) {
		if (i < MAX_SEGMENTS)
			m_segments [i].Read (fp, nLevelType, nLevelVersion);
		else {
			CSegment s;
			s.Read (fp);
			}
		}
	if (Count () > MAX_SEGMENTS)
		Count () = MAX_SEGMENTS;
	if (!DLE.IsD2File ())
		return;
	for (i = 0; i < Count (); i++)   
		m_segments [i].ReadExtras (fp, nLevelType, nLevelVersion, true);
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::WriteSegments (CFileManager& fp, int nFileVersion)
{
if (m_segmentInfo.Setup (fp)) {
	m_segmentInfo.offset = fp.Tell ();

	int nLevelType = DLE.IsD2XLevel () ? 2 : DLE.IsD2File () ? 1 : 0;
	int nLevelVersion = DLE.LevelVersion ();

	for (int i = 0; i < Count (); i++)
		m_segments [i].Write (fp, nLevelType, nFileVersion);
	if (!DLE.IsD2File ())
		return;
	for (int i = 0; i < Count (); i++)
		m_segments [i].WriteExtras (fp, nLevelType, true);
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::ReadMatCens (CFileManager& fp, int nFileVersion, int nClass)
{
if (m_matCenInfo [nClass].Restore (fp)) {
	for (int i = 0; i < MatCenCount (nClass); i++) {
		if (i < MAX_MATCENS)
			m_matCens [nClass][i].Read (fp, nFileVersion);
		else {
			CMatCenter m;
			m.Read (fp, nFileVersion);
			}
		}
	if (MatCenCount (nClass) > MAX_MATCENS)
		MatCenCount (nClass) = MAX_MATCENS;
	}
}

// ----------------------------------------------------------------------------- 

void CSegmentManager::WriteMatCens (CFileManager& fp, int nFileVersion, int nClass)
{
if (m_matCenInfo [nClass].Setup (fp)) {
	m_matCenInfo [nClass].size = (DLE.IsD1File () || (nClass == 0)) ? 16 : 20; 
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