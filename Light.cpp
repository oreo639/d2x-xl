#include "Mine.h"

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------

void CColor::Read (CFileManager& fp, int version, bool bNewFormat)
{
m_info.index = fp.ReadSByte ();
if (bNewFormat) {
	m_info.color.r = double (fp.ReadInt32 ()) / double (0x7fffffff);
	m_info.color.g = double (fp.ReadInt32 ()) / double (0x7fffffff);
	m_info.color.b = double (fp.ReadInt32 ()) / double (0x7fffffff);
	}
else {
	m_info.color.r = fp.ReadDouble ();
	m_info.color.g = fp.ReadDouble ();
	m_info.color.b = fp.ReadDouble ();
	}
}

// ------------------------------------------------------------------------

void CColor::Write (CFileManager& fp, int version, bool bFlag) 
{
fp.Write (m_info.index);
fp.WriteInt32 ((int) (m_info.color.r * 0x7fffffff + 0.5));
fp.WriteInt32 ((int) (m_info.color.g * 0x7fffffff + 0.5));
fp.WriteInt32 ((int) (m_info.color.b * 0x7fffffff + 0.5));
}

// -----------------------------------------------------------------------------

CGameItem* CColor::Copy (CGameItem* destP)
{
if (destP != null)
	*dynamic_cast<CColor*> (destP) = *this;
return destP;
}

// -----------------------------------------------------------------------------

CGameItem* CColor::Clone (void)
{
return Copy (new CColor);	// only make a copy if modified
}

// -----------------------------------------------------------------------------

void CColor::Backup (eEditType editType)
{
Id () = undoManager.Backup (this, editType);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CVariableLight::Read (CFileManager* fp) 
{
CSideKey::Read (fp);
m_info.mask = fp->ReadUInt32 ();
m_info.timer = fp->ReadInt32 ();
m_info.delay = fp->ReadInt32 ();
}

//------------------------------------------------------------------------------

void CVariableLight::Write (CFileManager* fp) 
{
CSideKey::Write (fp);
fp->Write (m_info.mask);
fp->Write (m_info.timer);
fp->Write (m_info.delay);
}

//------------------------------------------------------------------------------

void CVariableLight::Clear (void) 
{
memset (&m_info, 0, sizeof (m_info));
CSideKey::Clear ();
}

//------------------------------------------------------------------------------

void CVariableLight::Setup (CSideKey key, short time, short mask)
{
m_nSegment = key.m_nSegment;
m_nSide = key.m_nSide;
m_info.delay = m_info.timer = time;
m_info.mask = mask;
}

// -----------------------------------------------------------------------------

CGameItem* CVariableLight::Copy (CGameItem* destP)
{
if (destP != null)
	*dynamic_cast<CVariableLight*> (destP) = *this;
return destP;
}

// -----------------------------------------------------------------------------

CGameItem* CVariableLight::Clone (void)
{
return Copy (new CVariableLight);	// only make a copy if modified
}

// -----------------------------------------------------------------------------

void CVariableLight::Backup (eEditType editType)
{
Id () = undoManager.Backup (this, editType);
}

// -----------------------------------------------------------------------------

void CVariableLight::Undo (void)
{
switch (EditType ()) {
	case opAdd:
		lightManager.DeleteVariableLight (Index (), false);
		break;
	case opDelete:
		SetParent (lightManager.AddVariableLight (-1));
		// fall through
	case opModify:
		if (Parent ())
			*Parent () = *this;
		break;
	case opMove:
		*lightManager.VariableLight (lightManager.Count ()) = *this;
		break;
	}
}

// -----------------------------------------------------------------------------

void CVariableLight::Redo (void)
{
switch (EditType ()) {
	case opDelete:
		lightManager.DeleteVariableLight (Index (), false);
		break;
	case opAdd:
		SetParent (lightManager.AddVariableLight (-1));
		// fall through
	case opModify:
		if (Parent ())
			*Parent () = *this;
		break;
	case opMove:
		*lightManager.VariableLight (Index ()) = *this;
		break;
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CLightDeltaValue::Read (CFileManager* fp, int version, bool bFlag)
{
m_nSegment = fp->ReadInt16 ();
m_nSide = fp->ReadSByte ();
fp->ReadSByte ();
for (int i = 0; i < 4; i++)
	m_info.vertLight [i] = fp->ReadSByte ();
}

// -----------------------------------------------------------------------------

void CLightDeltaValue::Write (CFileManager* fp, int version, bool bFlag)
{
fp->Write (m_nSegment);
fp->WriteSByte ((sbyte) m_nSide);
fp->WriteByte (0);
for (int i = 0; i < 4; i++)
	fp->Write (m_info.vertLight [i]);
}


// -----------------------------------------------------------------------------

CGameItem* CLightDeltaValue::Copy (CGameItem* destP)
{
if (destP != null)
	*dynamic_cast<CLightDeltaValue*> (destP) = *this;
return destP;
}

// -----------------------------------------------------------------------------

CGameItem* CLightDeltaValue::Clone (void)
{
return Copy (new CLightDeltaValue);	// only make a copy if modified
}

// -----------------------------------------------------------------------------

void CLightDeltaValue::Backup (eEditType editType)
{
Id () = undoManager.Backup (this, editType);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void CLightDeltaIndex::Read (CFileManager* fp, int version, bool bD2X)
{
m_nSegment = fp->ReadInt16 ();
if (bD2X) {
	ushort h = fp->ReadInt16 ();
	m_nSide = h & 7;
	m_info.count = (h >> 3) & 0x1FFF;
	}
else {
	m_nSide = fp->ReadSByte ();
	m_info.count = fp->ReadSByte ();
	}
m_info.index = fp->ReadInt16 ();
}

// -----------------------------------------------------------------------------

void CLightDeltaIndex::Write (CFileManager* fp, int version, bool bD2X)
{
fp->Write (m_nSegment);
if (bD2X)
	fp->WriteInt16 ((m_nSide & 7) | (m_info.count << 3));
else {
	fp->WriteSByte ((sbyte) m_nSide);
	fp->WriteSByte ((sbyte) m_info.count);
	}
fp->Write (m_info.index);
}

// -----------------------------------------------------------------------------

CGameItem* CLightDeltaIndex::Copy (CGameItem* destP)
{
if (destP != null)
	*dynamic_cast<CLightDeltaIndex*> (destP) = *this;
return destP;
}

// -----------------------------------------------------------------------------

CGameItem* CLightDeltaIndex::Clone (void)
{
return Copy (new CLightDeltaIndex);	// only make a copy if modified
}

// -----------------------------------------------------------------------------

void CLightDeltaIndex::Backup (eEditType editType)
{
Id () = undoManager.Backup (this, editType);
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
//------------------------------------------------------------------------------
