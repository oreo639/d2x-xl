
#include "define.h"
#include "Types.h"
#include "Vector.h"
#include "Vertex.h"
#include "FileManager.h"
#include "UndoManager.h"

// -----------------------------------------------------------------------------

void CVertex::Read (CFileManager& fp, int version, bool bFlag) 
{ 
m_status = 0;
fp.ReadVector (v); 
}

// -----------------------------------------------------------------------------

void CVertex::Write (CFileManager& fp, int version, bool bFlag) 
{ 
fp.WriteVector (v); 
}

// -----------------------------------------------------------------------------
// make a copy of this vertex for the undo manager
// if vertex was modified, make a copy of the current vertex
// if vertex was added or deleted, just make a new CGameItem instance and 
// mark the operation there

CGameItem* CVertex::Clone (eEditType editType)
{
if (editType == opModify)
	return new CGameItem (itVertex);
CVertex* cloneP = new CVertex;	// only make a copy if modified
if (cloneP == null) {
	return null;
	*cloneP = *this;
	}
return cloneP;
}

// -----------------------------------------------------------------------------

void CVertex::Backup (eEditType editType)
{
if (m_nBackup != undoManager.Id ())
	m_nBackup = undoManager.Backup (this, itSegment, opModify);
}

// -----------------------------------------------------------------------------

void CVertex::Clear (void)
{
m_status = 0;
CDoubleVector::Clear ();
}

// -----------------------------------------------------------------------------
