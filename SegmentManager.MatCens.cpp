// Segment.cpp

#include "mine.h"
#include "dle-xp.h"

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberProducers (ubyte nFunction, short nClass) 
{
int h, i = 0, nErrors = 0; 

m_producers [nClass].SortAscending (0, ProducerCount (nClass) - 1);
for (h = i = 0; i < ProducerCount (nClass); i++) {
	short nSegment = m_producers [nClass][i].m_info.nSegment; 
	if (nSegment < 0) 
		++nErrors;
	else {
		CSegment* segP = Segment (nSegment); 
		if ((segP->m_info.function == nFunction) && (segP->m_info.nProducer < 0)) 
			m_producers [nClass][i].m_info.nProducer = segP->m_info.nProducer = h++; 
		else {
			m_producers [nClass][i].m_info.nSegment = -1; 
			++nErrors;
			}
		}
	}

if (nErrors) { // not all matcens assigned to a segment - try to find a segment that doesn't have a matcen
	CSegment* segP = Segment (0);
	for (int i = Count (); i; i--, segP++) {
		if ((segP->m_info.function == nFunction) && (segP->m_info.nProducer < 0)) {
			for (int j = 0; j < ProducerCount (nClass); j++) {
				if (m_producers [nClass][j].m_info.nSegment < 0) {
					segP->m_info.function = nFunction;
					segP->m_info.nProducer = j;
					m_producers [nClass][j].m_info.nSegment = Count () - i;
					nErrors--;
					break;
					}
				}
			}
		}
	}

if (nErrors) { // delete remaining unassigned matcens
	nErrors = 0;
	for (int i = 0, j = 0; j < ProducerCount (nClass); j++) {
		if (m_producers [nClass][i].m_info.nSegment >= 0)
			Segment (m_producers [nClass][i].m_info.nSegment)->m_info.nProducer = i++;
		else {
			if (i < j)
				m_producers [nClass][i] = m_producers [nClass][j];
			++nErrors;
			}
		}
	ProducerCount (nClass) -= nErrors;
	}
}

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberRobotMakers (void) 
{
RenumberProducers (SEGMENT_FUNC_ROBOTMAKER, 0);
}

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberEquipMakers (void) 
{
RenumberProducers (SEGMENT_FUNC_EQUIPMAKER, 1);
}

// -----------------------------------------------------------------------------

void CSegmentManager::RenumberProducers (void)
{
undoManager.Begin (__FUNCTION__, udSegments);
CSegment* segP = Segment (0);
for (int h = 0, i = Count (); i; i--, segP++)
	segP->m_info.nProducer = -1;
RenumberRobotMakers ();
RenumberEquipMakers ();
segP = Segment (0);
for (int h = 0, i = Count (); i; i--, segP++)
	if ((segP->m_info.function == SEGMENT_FUNC_PRODUCER) ||
		 (segP->m_info.function == SEGMENT_FUNC_REPAIRCEN))
		segP->m_info.value = h++; 
	else if ((segP->m_info.function == SEGMENT_FUNC_ROBOTMAKER) ||
				(segP->m_info.function == SEGMENT_FUNC_EQUIPMAKER)) {
		if (segP->m_info.nProducer >= 0) {
			segP->m_info.value = h++; 
			}
		else {
			segP->m_info.value = -1; 
			segP->m_info.function = SEGMENT_FUNC_NONE;
			}
		}
	else
		segP->m_info.value = -1; 
undoManager.End (__FUNCTION__);
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