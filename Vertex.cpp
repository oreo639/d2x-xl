
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
CVertex* cloneP = new CVertex;	// only make a copy if modified
if (cloneP != null) 
	*cloneP = *this;
return cloneP;
}

// -----------------------------------------------------------------------------

void CVertex::Backup (eEditType editType)
{
if (HaveBackup ())
	return false;
m_nBackup = undoManager.Backup (this, itSegment, opModify);
return true;
}

// -----------------------------------------------------------------------------

void CVertex::Save (void)
{
if (!Backup ()) {
	*dynamic_cast<CVertex*> (m_backup) = *this;
	m_backup->Id () = undoManager.Id ();
	}
}

// -----------------------------------------------------------------------------

void CVertex::Undo (void)
{
switch (EditType ()) {
	case opAdd:
		vertexManager.Remove (vertexManager.Index (this), false);
		break;
	case opDelete:
		vertexManager.Add (false);
		// fall through
	case opModify:
		*Parent () = *this;
		break;
	}
}

// -----------------------------------------------------------------------------

void CVertex::Redo (void)
{
switch (EditType ()) {
	case opDelete:
		vertexManager.Remove (vertexManager.Index (this));
		break;
	case opAdd:
		vertexManager.Add (false);
		// fall through
	case opModify:
		*Parent () = *this;
		break;
	}
}

// -----------------------------------------------------------------------------

void CVertex::Clear (void)
{
m_status = 0;
CDoubleVector::Clear ();
}

// -----------------------------------------------------------------------------
