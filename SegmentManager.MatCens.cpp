// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberMatCens (byte nFunction, short nClass) 
{
int h, i = 0, nErrors = 0; 

m_matCens [nClass].SortAscending (0, MatCenCount (nClass) - 1);
for (h = i = 0; i < MatCenCount (nClass); i++) {
	short nSegment = m_matCens [nClass][i].m_info.nSegment; 
	if (nSegment < 0) 
		++nErrors;
	else {
		CSegment* segP = Segment (nSegment); 
		if ((segP->m_info.function == nFunction) && (segP->m_info.nMatCen < 0)) 
			segP->m_info.nMatCen = h++; 
		else {
			m_matCens [nClass][i].m_info.nSegment = -1; 
			++nErrors;
			}
		}
	}

if (nErrors) { // not all matcens assigned to a segment - try to find a segment that doesn't have a matcen
	CSegment* segP = Segment (0);
	for (int i = Count (); i; i--, segP++) {
		if ((segP->m_info.function == nFunction) && (segP->m_info.nMatCen < 0)) {
			for (int j = 0; j < MatCenCount (nClass); j++) {
				if (m_matCens [nClass][j].m_info.nSegment < 0) {
					segP->m_info.function = nFunction;
					segP->m_info.nMatCen = j;
					m_matCens [nClass][j].m_info.nSegment = Count () - i;
					nErrors--;
					break;
					}
				}
			}
		}
	}

if (nErrors) { // delete remaining unassigned matcens
	nErrors = 0;
	for (int i = 0, j = 0; j < MatCenCount (nClass); j++) {
		if (m_matCens [nClass][i].m_info.nSegment >= 0)
			Segment (m_matCens [nClass][i].m_info.nSegment)->m_info.nMatCen = i++;
		else {
			if (i < j)
				m_matCens [nClass][i] = m_matCens [nClass][j];
			++nErrors;
			}
		}
	MatCenCount (nClass) -= nErrors;
	}
}

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberRobotMakers (void) 
{
RenumberMatCens (SEGMENT_FUNC_ROBOTMAKER, 0);
}

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberEquipMakers (void) 
{
RenumberMatCens (SEGMENT_FUNC_EQUIPMAKER, 1);
}

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberFuelCenters (void)
{
undoManager.Begin (udSegments);
CSegment* segP = Segment (0);
for (int h = 0, i = Count (); i; i--, segP++)
	segP->m_info.nMatCen = -1;
RenumberRobotMakers ();
RenumberEquipMakers ();
segP = Segment (0);
for (int h = 0, i = Count (); i; i--, segP++)
	if ((segP->m_info.function == SEGMENT_FUNC_FUELCEN) ||
		 (segP->m_info.function == SEGMENT_FUNC_REPAIRCEN))
		segP->m_info.value = h++; 
	else if ((segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) ||
				(segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER)) {
		if (segP->m_info.nMatCen >= 0) {
			segP->m_info.value = h++; 
			m_matCens [segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER][segP->m_info.nMatCen].m_info.nFuelCen = segP->m_info.value;
			}
		else {
			segP->m_info.value = -1; 
			segP->m_info.function = SEGMENT_FUNC_NONE;
			}
		}
	else
		segP->m_info.value = -1; 
undoManager.End ();
}

// ----------------------------------------------------------------------------- 

CSegment* CSegmentManager::FindRobotMaker (short i)
{
	CSegment* segP = Segment (i);

for (; i < Count (); i++, segP++)
	if (segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER)
		return segP;
return null;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//eof segmentmanager.cpp