
#include "mine.h"
#include "dle-xp.h"

CUndoManager undoManager;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CUndoItem::Setup (CGameItem* item, CGameItem* parent, eEditType editType, int nBackupId)
{
m_parent = parent; 
m_item = item;
m_parent->SetParent (m_item);
m_item->SetParent (m_parent);
m_item->Id () = Id (); // copy these to parent to make subsequent cloning simpler
m_parent->EditType () = m_item->EditType () = editType;
m_nBackupId = nBackupId;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

bool CUndoData::Cleanup (void) 
{
	bool bEmpty = true;

if (m_vertices.Cleanup ()) bEmpty = false;
if (m_segments.Cleanup ()) bEmpty = false;
if (m_robotMakers.Cleanup ()) bEmpty = false;
if (m_equipMakers.Cleanup ()) bEmpty = false;
if (m_walls.Cleanup ()) bEmpty = false;
if (m_doors.Cleanup ()) bEmpty = false;
if (m_triggers.Cleanup ()) bEmpty = false;
if (m_objects.Cleanup ()) bEmpty = false;
if (m_robotInfo.Cleanup ()) bEmpty = false;
if (m_deltaLightIndices.Cleanup ()) bEmpty = false;
if (m_deltaLightValues.Cleanup ()) bEmpty = false;
if (m_variableLights.Cleanup ()) bEmpty = false;
if (m_faceColors.Cleanup ()) bEmpty = false;
if (m_textureColors.Cleanup ()) bEmpty = false;
if (m_vertexColors.Cleanup ()) bEmpty = false;
return bEmpty;
}

//------------------------------------------------------------------------------

bool CUndoData::Destroy (void) 
{
m_vertices.Destroy ();
m_segments.Destroy ();
m_robotMakers.Destroy ();
m_equipMakers.Destroy ();
m_walls.Destroy ();
m_doors.Destroy ();
m_triggers.Destroy ();
m_objects.Destroy ();
m_robotInfo.Destroy ();
m_deltaLightIndices.Destroy ();
m_deltaLightValues.Destroy ();
m_variableLights.Destroy ();
m_faceColors.Destroy ();
m_textureColors.Destroy ();
m_vertexColors.Destroy ();
return false;
}

//------------------------------------------------------------------------------

void CUndoData::Backup (undoData dataFlags) 
{
if (dataFlags & udVertices) 
	m_vertices.Backup (vertexManager.Vertex (0), vertexManager.Count ());

if (dataFlags & udSegments) 
	!m_segments.Backup (segmentManager.Segment (0), segmentManager.Count ());

if (dataFlags & udMatCens) {
	m_robotMakers.Backup (segmentManager.RobotMaker (0), segmentManager.RobotMakerCount ());
	m_equipMakers.Backup (segmentManager.EquipMaker (0), segmentManager.EquipMakerCount ());
	}

if (dataFlags & udWalls) 
	!m_walls.Backup (wallManager.Wall (0), wallManager.WallCount ());

if (dataFlags & udTriggers) 
	!m_triggers.Backup (triggerManager.Trigger (0), triggerManager.Count ());

if (dataFlags & udObjects) 
	!m_objects.Backup (objectManager.Objects (0), objectManager.Count ());

if (dataFlags & udRobots) 
	!m_robotInfo.Backup (robotManager.RobotInfo (0), robotManager.Count ());

if (dataFlags & udVariableLights) 
	!m_variableLights.Backup (lightManager.VariableLight (0), lightManager.Count ());

if (dataFlags & udStaticLight) {
	 m_faceColors.Backup (lightManager.FaceColor (0), segmentManager.m_nSegments * 6);
	 m_textureColors.Backup (lightManager.TexColor (0), MAX_TEXTURES_D2);
	 m_vertexColors.Backup (lightManager.VertexColor (0), vertexManager.Count ());
	}

if (dataFlags & udDynamicLight) {
	m_deltaIndices.Backup (lightManager.LightDeltaIndex (0), lightManager.DeltaIndexCount ());
	m_deltaValues.Backup (lightManager.LightDeltaValue (0), lightManager.DeltaValueCount ());
	}
}

//------------------------------------------------------------------------------

void CUndoData::Restore (undoData dataFlags) 
{
if (dataFlags & udVertices) 
	m_vertices.Restore ();

if (dataFlags & udSegments) 
	!m_segments.Restore ();

if (dataFlags & udMatCens) {
	m_robotMakers.Restore ();
	m_equipMakers.Restore ();
	}

if (dataFlags & udWalls) 
	!m_walls.Restore ();

if (dataFlags & udTriggers) 
	!m_triggers.Restore ();

if (dataFlags & udObjects) 
	!m_objects.Restore ();

if (dataFlags & udRobots) 
	!m_robotInfo.Restore ();

if (dataFlags & udVariableLights) 
	!m_variableLights.Restore ();

if (dataFlags & udStaticLight) {
	 m_faceColors.Restore ();
	 m_textureColors.Restore ();
	 m_vertexColors.Restore ();
	}

if (dataFlags & udDynamicLight) {
	m_deltaIndices.Restore ();
	m_deltaValues.Restore ();
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CUndoManager::CUndoManager (int maxSize)
{
m_nHead = m_nTail = m_nCurrent = -1;
m_size = 0;
m_enabled = 1;
m_maxSize = maxSize;
m_nModified = 0;
}

//------------------------------------------------------------------------------

CUndoManager::~CUndoManager ()
{
Reset ();
}

//------------------------------------------------------------------------------

void CUndoManager::Reset ()
{
m_nCurrent = m_nHead;
Truncate ();
}

//------------------------------------------------------------------------------

int CUndoManager::Enable (int bEnable)
{
	int b = m_enabled;

m_enabled = bEnable;
return b;
}

//------------------------------------------------------------------------------

void CUndoManager::Truncate (void)
{
for (;;) {
	m_buffer [m_nTail].Destroy ();
	if (m_nTail == m_nCurrent)
		break;
	if (--m_nTail < 0)
		m_nTail = sizeof (m_buffer) - 1;
	}
if (m_nCurrent == m_nHead) {
	m_nHead = m_nTail = m_nCurrent = -1;
	m_nId = 0;
	}
else {
	if (--m_nTail < 0)
		m_nTail = sizeof (m_buffer) - 1;
	m_nCurrent = m_nTail;
	m_nId = Current ()->Id ();
	}
}

//------------------------------------------------------------------------------

int CUndoManager::SetMaxSize (int maxSize)
{
if (maxSize < 1)
	maxSize = 0;
else if (maxSize > MAX_UNDOS)
	maxSize = MAX_UNDOS;
Enable (maxSize > 0);
while (m_size > maxSize)
	Truncate ();
return m_maxSize = maxSize;
}

//------------------------------------------------------------------------------

bool CUndoManager::Undo (void)
{
if (!m_enabled)
	return false;
if (m_nCurrent == m_nHead)
	return false;
int nId = Current ()->Id ();
do {
	Current ()->Restore ();
	if (--m_nCurrent < 0)
		m_nCurrent = sizeof (m_buffer) - 1;
	} while ((m_nCurrent != m_nHead) && (Current ()->Id () == nId));
return true;
}

//------------------------------------------------------------------------------

bool CUndoManager::Redo (void)
{
if (!m_enabled)
	return false;
if (m_nCurrent == m_nTail)
	return false;
int nId = Current ()->Id ();
do {
	Current ()->Restore ();
	m_nCurrent = ++m_nCurrent % sizeof (m_buffer);
	} while ((m_nCurrent != m_nTail) && (Current ()->Id () == nId));
return true;
}

//------------------------------------------------------------------------------

void CUndoManager::Append (void)
{
if (m_nHead = -1)
	m_nHead = m_nTail = m_nCurrent = 0;
else {
	Truncate ();
	m_nTail = ++m_nTail % sizeof (m_buffer);
	if (m_nTail == m_nHead) {	// buffer full
		int nId = Head ()->Id ();
		do { // remove all items with same backup id from buffer start
			m_nHead = ++m_nHead % sizeof (m_buffer);
#if DETAIL_BACKUP
			delete Head ()->m_item;
#else
			Head ()->Destroy ();
#endif
			} while (Head ()->Id () == nId);
		}
	}
}

//------------------------------------------------------------------------------

int CUndoManager::Backup (CGameItem* parent, eEditType editType) 
{ 
#if USE_FREELIST
SetModified (true);
if (parent->Id () == Id ()) { // item backup up in this batch already, so update the backup
	CGameItem* backup = parent->Parent ();
	parent->Copy (backup);
	backup->SetParent (parent);
	return parent->Id ();
	}
else { // create a new backup
	CGameItem* item = parent->Clone ();
	if (item != null) {
		Append ();
		SetModified (true);
		Tail ()->Setup (item, parent, editType, Id ());
		}
	return Id ();
	}
#endif
}

//------------------------------------------------------------------------------

void CUndoManager::Backup (void)
{
if (m_current.Cleanup ()) 
	Id ()--;
else {
	SetModified (true);
	Append ();
	*Tail () = m_current;
	m_current.Reset ();
	}
}

//------------------------------------------------------------------------------

int CUndoManager::Count (void)
{
return (m_nTail > m_nHead) ? (m_nTail - m_nHead + 1) : m_nTail + m_nHead - sizeof (m_buffer);
}

//------------------------------------------------------------------------------

bool CUndoManager::Update (bool bForce)
{
if (!m_enabled || m_delay)
	return false;
if (m_nCurrent != m_nTail) {
	m_nCurrent = m_nTail;
	m_nId++;
	}
return true;
}

//------------------------------------------------------------------------------

void CUndoManager::Delay (bool bDelay)
{
if (bDelay)
	m_delay++;
else if (m_delay > 0)
	m_delay--;
}

//------------------------------------------------------------------------------

void CUndoManager::SetModified (bool bModified) 
{
DLE.GetDocument ()->SetModifiedFlag (bModified);
}  

//------------------------------------------------------------------------------

void CUndoManager::Begin (undoFlags dataFlags) 
{
if (0 == m_nModified++) 
	Update ();
m_current.Backup (dataFlags);
Lock ();
}

//------------------------------------------------------------------------------

void CUndoManager::End (void) 
{
if (m_nModified > 0) {
	Unlock ();
	if (--m_nModified == 0)
		Update ();
	}
}

//------------------------------------------------------------------------------

bool CUndoManager::Revert (void)
{
if (!m_enabled || m_delay || !m_nHead)
	return false;
Undo ();
Truncate ();
return true;
}

//------------------------------------------------------------------------------

void CUndoManager::Unroll (void) 
{
if (--m_nModified == 0) {
	SetModified (false);
	Revert ();
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
