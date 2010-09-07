// -----------------------------------------------------------------------------

void Read (CFileManager& fp, int version, bool bFlag) 
{ 
m_status = 0;
fp.ReadVector (v); 
}

// -----------------------------------------------------------------------------

void Write (CFileManager& fp, int version, bool bFlag) 
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
CVertex* cloneP;
if (editType == opModified)
	cloneP = new CGameItem (itSegment);
else {
	cloneP = new CVertex;	// only make a copy if modified
	if (cloneP == null)
		return null;
		*cloneP = *this;
		}
	}
return cloneP;
}

// -----------------------------------------------------------------------------

void CVertex::Backup (eEditType editType = opModify)
{
if (m_nBackup != undoManager.Id ())
	m_nBackup = undoManager.Backup (this, itSegment, opModify);
}

// -----------------------------------------------------------------------------
