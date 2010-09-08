
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

CGameItem* CVertex::Copy (CGameItem* destP)
{
if (destP != null)
	*dynamic_cast<CVertex*> (destP) = *this;
return destP;
}

// -----------------------------------------------------------------------------

CGameItem* CVertex::Clone (void)
{
return Copy (new CVertex);	// only make a copy if modified
}

// -----------------------------------------------------------------------------

void CVertex::Backup (eEditType editType)
{
Id () = undoManager.Backup (this, editType);
}

// -----------------------------------------------------------------------------

void CVertex::Undo (void)
{
switch (EditType ()) {
	case opAdd:
		vertexManager.Delete (Index (), true);
		break;
	case opDelete:
		vertexManager.Add (true);
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
		vertexManager.Delete (Index ());
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
