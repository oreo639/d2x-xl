// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

extern short nDbgSeg, nDbgSide;

// ------------------------------------------------------------------------

void CSegmentManager::SetIndex (void)
{
int j = 0;
for (CSegmentIterator si; si; si++)
	si->Index () = j++;
}

// ------------------------------------------------------------------------

void CSegmentManager::ReadSegments (CFileManager* fp)
{
if (m_segmentInfo.Restore (fp)) {
	int i;

	for (i = 0; i < Count (); i++) {
#ifdef _DEBUG
		if (i == nDbgSeg)
			nDbgSeg = nDbgSeg;
#endif
		if (i < MAX_SEGMENTS)
			m_segments [i].Read (fp);
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
		m_segments [i].ReadExtras (fp, true);
	}
}

// ------------------------------------------------------------------------

void CSegmentManager::WriteSegments (CFileManager* fp)
{
if (m_segmentInfo.Setup (fp)) {
	m_segmentInfo.offset = fp->Tell ();

	for (int i = 0; i < Count (); i++)
		m_segments [i].Write (fp);
	if (!DLE.IsD2File ())
		return;
	for (int i = 0; i < Count (); i++)
		m_segments [i].WriteExtras (fp, true);
	}
}

// ------------------------------------------------------------------------

void CSegmentManager::ReadMatCens (CFileManager* fp, int nClass)
{
if (m_matCenInfo [nClass].Restore (fp)) {
	for (int i = 0; i < MatCenCount (nClass); i++) {
		if (i < MAX_MATCENS)
			m_matCens [nClass][i].Read (fp);
		else {
			CMatCenter m;
			m.Read (fp);
			}
		}
	if (MatCenCount (nClass) > MAX_MATCENS)
		MatCenCount (nClass) = MAX_MATCENS;
	}
}

// ------------------------------------------------------------------------

void CSegmentManager::WriteMatCens (CFileManager* fp, int nClass)
{
if (m_matCenInfo [nClass].Setup (fp)) {
	m_matCenInfo [nClass].size = (DLE.IsD1File () && (nClass == 0)) ? 16 : 20; 
	m_matCenInfo [nClass].offset = fp->Tell ();
	for (int i = 0; i < MatCenCount (nClass); i++)
		m_matCens [nClass][i].Write (fp);
	}
}

// ------------------------------------------------------------------------

void CSegmentManager::ReadRobotMakers (CFileManager* fp)
{
ReadMatCens (fp, 0);
}

// ------------------------------------------------------------------------

void CSegmentManager::WriteRobotMakers (CFileManager* fp)
{
WriteMatCens (fp, 0);
}

// ------------------------------------------------------------------------

void CSegmentManager::ReadEquipMakers (CFileManager* fp)
{
ReadMatCens (fp, 1);
}

// ------------------------------------------------------------------------

void CSegmentManager::WriteEquipMakers (CFileManager* fp)
{
WriteMatCens (fp, 1);
}

// ------------------------------------------------------------------------

void CSegmentManager::Clear (void)
{
for (int i = 0; i < Count (); i++)
	m_segments [i].Clear ();
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
//eof segmentmanager.cpp