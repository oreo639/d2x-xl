// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberMatCens (byte nFunction, short nClass) 
{
int h, i = 0; 
for (h = i = 0; i < MatCenCount (nClass); i++) {
	short nSegment = m_matCens [nClass][i].m_info.nSegment; 
	if (nSegment >= 0) {
		CSegment* segP = Segment (nSegment); 
		segP->m_info.value = i; 
		if (segP->m_info.function == nFunction)
			segP->m_info.nMatCen = h++; 
		}
	}
// number "value"
CSegment* segP = Segment (0);
for (h = i = 0; i < Count (); i++, segP++)
	if (segP->m_info.function == SEGMENT_FUNC_NONE)
		segP->m_info.value = 0; 
	else
		segP->m_info.value = h++; 
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