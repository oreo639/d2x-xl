
#include "mine.h"
#include "dle-xp.h"

CUndoManager undoManager;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

#if DETAIL_BACKUP

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

#endif

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
if (m_triggers [0].Cleanup ()) bEmpty = false;
if (m_triggers [1].Cleanup ()) bEmpty = false;
if (m_reactorTriggers.Cleanup ()) bEmpty = false;
if (m_reactorData.Cleanup ()) bEmpty = false;
if (m_objects.Cleanup ()) bEmpty = false;
if (m_secretExit.Cleanup ()) bEmpty = false;
if (m_robotInfo.Cleanup ()) bEmpty = false;
if (m_deltaIndices.Cleanup ()) bEmpty = false;
if (m_deltaValues.Cleanup ()) bEmpty = false;
if (m_variableLights.Cleanup ()) bEmpty = false;
if (m_faceColors.Cleanup ()) bEmpty = false;
if (m_textureColors.Cleanup ()) bEmpty = false;
if (m_vertexColors.Cleanup ()) bEmpty = false;
return bEmpty;
}

//------------------------------------------------------------------------------

void CUndoData::Destroy (void) 
{
m_vertices.Destroy ();
m_segments.Destroy ();
m_robotMakers.Destroy ();
m_equipMakers.Destroy ();
m_walls.Destroy ();
m_doors.Destroy ();
m_triggers [0].Destroy ();
m_triggers [1].Destroy ();
m_reactorTriggers.Destroy ();
m_objects.Destroy ();
m_robotInfo.Destroy ();
m_deltaIndices.Destroy ();
m_deltaValues.Destroy ();
m_variableLights.Destroy ();
m_faceColors.Destroy ();
m_textureColors.Destroy ();
m_vertexColors.Destroy ();
m_secretExit.Destroy ();
m_reactorData.Destroy ();
}

//------------------------------------------------------------------------------

void CUndoData::Backup (int dataFlags) 
{
if (dataFlags & udVertices) 
	m_vertices.Backup (vertexManager.Vertex (0), &vertexManager.Count ());

if (dataFlags & udSegments)
	m_segments.Backup (segmentManager.Segment (0), &segmentManager.Count ());

if (dataFlags & udMatCenters) {
	m_robotMakers.Backup (segmentManager.RobotMaker (0), &segmentManager.RobotMakerCount ());
	m_equipMakers.Backup (segmentManager.EquipMaker (0), &segmentManager.EquipMakerCount ());
	}

if (dataFlags & udWalls) 
	m_walls.Backup (wallManager.Wall (0), &wallManager.WallCount ());

if (dataFlags & udTriggers) {
	m_triggers [0].Backup (triggerManager.Trigger (0, 0), &triggerManager.Count (0));
	m_triggers [1].Backup (triggerManager.Trigger (0, 1), &triggerManager.Count (1));
	m_reactorTriggers.Backup (triggerManager.ReactorTrigger (0), &triggerManager.ReactorTriggerCount ());
	m_reactorData.Backup (&triggerManager.ReactorData ());
	}	

if (dataFlags & udObjects) {
	m_objects.Backup (objectManager.Object (0), &objectManager.Count ());
	m_secretExit.Backup (&objectManager.SecretExit ());
	}

if (dataFlags & udRobots) 
	m_robotInfo.Backup (robotManager.RobotInfo (0), &robotManager.Count ());

if (dataFlags & udVariableLights) 
	m_variableLights.Backup (lightManager.VariableLight (0), &lightManager.Count ());

if (dataFlags & udStaticLight) {
	 m_faceColors.Backup (lightManager.FaceColor (0), segmentManager.Count () * 6);
	 m_textureColors.Backup (lightManager.TexColor (0), MAX_TEXTURES_D2);
	 m_vertexColors.Backup (lightManager.VertexColor (0), vertexManager.Count ());
	}

if (dataFlags & udDynamicLight) {
	m_deltaIndices.Backup (lightManager.LightDeltaIndex (0), &lightManager.DeltaIndexCount ());
	m_deltaValues.Backup (lightManager.LightDeltaValue (0), &lightManager.DeltaValueCount ());
	}
}

//------------------------------------------------------------------------------

void CUndoData::Restore (void) 
{
m_vertices.Restore ();
m_segments.Restore ();
m_robotMakers.Restore ();
m_equipMakers.Restore ();
m_walls.Restore ();
m_triggers [0].Restore ();
m_reactorData.Restore ();
m_triggers [1].Restore ();
m_objects.Restore ();
m_secretExit.Restore ();
m_robotInfo.Restore ();
m_variableLights.Restore ();
m_faceColors.Restore ();
m_textureColors.Restore ();
m_vertexColors.Restore ();
m_deltaIndices.Restore ();
m_deltaValues.Restore ();
}

//------------------------------------------------------------------------------

void CUndoData::Reset (void) 
{
m_vertices.Reset ();
m_segments.Reset ();
m_robotMakers.Reset ();
m_equipMakers.Reset ();
m_walls.Reset ();
m_triggers [0].Reset ();
m_reactorData.Reset ();
m_triggers [1].Reset ();
m_objects.Reset ();
m_secretExit.Reset ();
m_robotInfo.Reset ();
m_variableLights.Reset ();
m_faceColors.Reset ();
m_textureColors.Reset ();
m_vertexColors.Reset ();
m_deltaIndices.Reset ();
m_deltaValues.Reset ();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

CUndoManager::CUndoManager (int maxSize)
{
m_nHead = m_nTail = m_nCurrent = -1;
m_size = 0;
m_enabled = 1;
m_nMaxSize = maxSize;
m_nModified = 0;
m_mode = 0;
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

int CUndoManager::SetMaxSize (int maxSize)
{
if (maxSize < 1)
	maxSize = 0;
else if (maxSize > MAX_UNDOS)
	maxSize = MAX_UNDOS;
Enable (maxSize > 0);
int nExcess = Count () - maxSize;
if (nExcess > 0) {
	m_nCurrent -= nExcess;
	Truncate ();
	}
m_nHead.Setup (maxSize);
m_nTail.Setup (maxSize);
m_nCurrent.Setup (maxSize);
return m_nMaxSize = maxSize;
}

//------------------------------------------------------------------------------

bool CUndoManager::Undo (void)
{
if (!m_enabled)
	return false;
if (m_nCurrent == m_nHead)
	return false;
// need a backup of the current state when starting to undo
if ((m_mode == 0) && (m_nCurrent == m_nTail + 1)) {
	m_current.Destroy ();
	m_current.Backup (udAll);
	}

m_mode = 1;
--m_nCurrent;
Current ()->Restore ();
return true;
}

//------------------------------------------------------------------------------

bool CUndoManager::Redo (void)
{
if (!m_enabled)
	return false;
CBufPtr nEnd = m_nTail + 1;
if (m_nCurrent == nEnd)
	return false;
if (m_mode == 0)
	return false;

m_mode = 2;
if (++m_nCurrent == nEnd)
	m_current.Restore ();
else 
	Current ()->Restore ();
return true;
}

//------------------------------------------------------------------------------

void CUndoManager::Truncate (void)
{
if (m_nCurrent == m_nTail + 1)
	return; // at end of undo list already
do {
	m_buffer [*m_nTail].Destroy ();
	if (m_nTail == m_nCurrent)
		break;
	m_nTail = Reverse (m_nTail);
	}
if (m_nCurrent == m_nHead) {
	m_nHead = m_nTail = m_nCurrent = -1;
	m_nId = 0;
	}
else {
	m_nTail = Reverse (m_nTail);
	if (--m_nTail < 0)
		m_nTail = m_nMaxSize - 1;
	m_nCurrent = m_nTail;
	m_nId = Current ()->Id ();
	}
}

//------------------------------------------------------------------------------

void CUndoManager::Append (void)
{
if (m_nHead = -1)
	m_nHead = m_nTail = 0;
else {
	Truncate ();
	if (++m_nTail == m_nHead) {	// buffer full
		int nId = Head ()->Id ();
		do { // remove all items with same backup id from buffer start
			m_nHead = ++m_nHead % m_nMaxSize;
#if DETAIL_BACKUP
			delete Head ()->m_item;
#else
			Head ()->Destroy ();
#endif
			} while (Head ()->Id () == nId);
		}
	}
m_nCurrent = m_nTail + 1;
}

//------------------------------------------------------------------------------

int CUndoManager::Backup (CGameItem* parent, eEditType editType) 
{ 
#if DETAIL_BACKUP
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
#else
return Id ();
#endif
}

//------------------------------------------------------------------------------

void CUndoManager::Backup (void)
{
if (!m_current.Cleanup ()) {
	SetModified (true);
	Append ();
	*Tail () = m_current;
	m_current.Reset ();
	Id ()++;
	}
}

//------------------------------------------------------------------------------

int CUndoManager::Count (void)
{
return (m_nTail > m_nHead) ? (m_nTail - m_nHead + 1) : m_nTail + m_nHead - m_nMaxSize;
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

void CUndoManager::Begin (int dataFlags) 
{
if (0 == m_nModified++) 
	Update ();
if (m_mode != 0) {
	m_mode = 0;
	m_current.Destroy ();
	}
m_current.Backup (dataFlags);
Lock ();
}

//------------------------------------------------------------------------------

void CUndoManager::End (void) 
{
if (m_nModified > 0) {
	Unlock ();
	if (--m_nModified == 0)
		Backup ();
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
