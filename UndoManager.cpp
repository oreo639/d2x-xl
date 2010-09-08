
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

bool CUndoData::Destroy (void) 
{
if (m_vertices != null) {
	delete[] m_vertices;
	m_vertices = null;
	m_nVertices = 0;
	}

if (m_segments != null) {
	delete[] m_segments;
	m_segments = null;
	m_nSegments = 0;
	}

for (int i = 0; i < 2; i++)
	if (m_matCens [i] != null) {
		delete[] m_matCens [i];
		m_matCens [i] = null;
		m_nMatCens [i] = 0;
		}

if (m_walls != null) {
	delete[] m_walls;
	m_walls = null;
	m_nWalls = 0;
	}

if (m_triggers != null) {
	delete[] m_triggers;
	m_triggers = null;
	m_nTriggers = 0;
	}

if (m_objects != null) {
	delete[] m_objects;
	m_objects = null;
	m_nObjects = 0;
	}

if (m_robots != null) {
	delete[] m_robots;
	m_robots = null;
	m_nRobots = 0;
	}

if (m_variableLights != null) {
	delete[] m_variableLights;
	m_variableLights = null;
	m_nVariableLights = 0;
	}

if (m_faceColors != null) {
	delete[] m_faceColors;
	m_faceColors = null;
	m_nFaceColors = 0;
	}

if (m_textureColors != null) {
	delete[] m_textureColors;
	m_textureColors = null;
	m_nTextureColors = 0;
	}

if (m_vertexColors != null) {
	delete[] m_vertexColors;
	m_vertexColors = null;
	m_nVertexColors = 0;
	}

if (m_deltaIndices != null) {
	delete[] m_deltaIndices;
	m_deltaIndices = null;
	m_nDeltaIndices = 0;
	}

if (m_deltaValues != null) {
	delete[] m_deltaValues;
	m_deltaValues = null;
	m_nDeltaValues = 0;
	}

return false;
}

//------------------------------------------------------------------------------

bool CUndoData::Save (undoData data) 
{
if (data & udVertices) {
	m_vertices = new CVertex [m_nVertices = vertexManager.Count ()];
	if (m_vertices == null)
		return Destroy ();
	int i = 0;
	for (CVertexIterator it; it; it++)
		it->Index () = it.Index ();
		m_vertices [i++] = it;
		}
	}

if (data & udSegments) {
	m_segments = new CSegment [m_nSegments = segmentManager.Count ()];
	if (m_segments == null)
		return Destroy ();
	int i = 0;
	for (CSegmentIterator it; it; it++)
		it->Index () = it.Index ();
		m_segments [i++] = it;
		}
	}

if (data & udMatCens) {
	for (int i = 0; i < 2; i++) {
		if ((m_nMatCens [i] = segmentManager.MatCenCount (i)) > 0) {
			m_matCens [i] = new CMatCenter [m_nMatCens [i]];
			if (m_matCens [i] == null)
				return Destroy ();
			memcpy (m_matCens [i], segmentManager.MatCenters (i), m_nMatCens [i] * sizeof (CMatCenter));
			}
		}
	}

if (data & udWalls) {
	m_walls = new CWall [m_nWalls = wallManager.Count ()];
	if (m_walls == null)
		return Destroy ();
	int i = 0;
	for (CWallIterator it; it; it++)
		it->Index () = it.Index ();
		m_walls [i++] = it;
		}
	}

if (data & udTriggers) {
	m_triggers = new CTrigger [m_nTriggers = triggerManager.Count ()];
	if (m_triggers == null)
		return Destroy ();
	int i = 0;
	for (CTriggerIterator it; it; it++)
		it->Index () = it.Index ();
		m_triggers [i++] = it;
		}
	}

if (data & udObjects) {
	m_objects = new CGameObject [m_nObjects = objectManager.Count ()];
	if (m_objects == null)
		return Destroy ();
	memcpy (m_objects, objectManager.Object (0), m_nObjects * sizeof (CGameObject));
	}

if (data & udRobots) {
	m_robots = new CRobotInfo [m_nRobots = robotManager.Count ()];
	if (m_robots == null)
		return Destroy ();
	memcpy (m_robots, robotManager.RobotInfo (0), m_nRobots * sizeof (CRobotInfo));
	}

if (data & udVariableLights) {
	m_variableLights = new CVariableLights [m_nVariableLights = lightManager.Count ()];
	if (m_variableLights == null)
		return Destroy ();
	memcpy (m_variableLights, lightManager.VariableLight (0), m_nVariableLights * sizeof (CVariableLight));
	}

if (data & udStaticLight) {
	 m_faceColors = new CFaceColor [m_nFaceColors = segmentManager.m_nSegments * 6];
	 if (m_faceColors == null)
		 return Destroy ();
	 memcpy (m_faceColors, lightManager.FaceColor (0), m_nFaceColors * sizeof (CFaceColor));

	 m_textureColors = new CTextureColor [m_nTextureColors = MAX_TEXTURES_D2];
	 if (m_textureColors == null)
		 return Destroy ();
	 memcpy (m_textureColors, lightManager.TexColor (0), m_nTextureColors * sizeof (CTextureColor));

	 m_vertexColors = new CVertexColor [m_nVertexColors = vertexManager.Count ()];
	 if (m_vertexColors == null)
		 return Destroy ();
	 memcpy (m_vertexColors, lightManager.TexColor (0), m_nVertexColors * sizeof (CVertexColor));
	}

if (data & udDynamicLight) {
	m_deltaIndices = new CLightDeltaIndex [m_nDeltaIndices = lightManager.DeltaIndexCount ()];
	if (m_deltaIndices == null)
		return Destroy ();
	memcpy (m_deltaIndices, lightManager.LightDeltaIndex (0), m_nDeltaIndices * sizeof (CLightDeltaIndex));

	m_seltaValues = new CLightDeltaValue [m_nDeltaValues = lightManager.DeltaValueCount ()];
	if (m_deltaValues == null)
		return Destroy ();
	memcpy (m_deltaValues, lightManager.LightDeltaValue (0), m_nDeltaValues * sizeof (CLightDeltaValue));
	}
return true;
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
for (int i = m_nCurrent; i != m_nTail; i = ++i % sizeof (m_buffer)) 
	delete m_buffer [i].m_item;
if (m_nCurrent == m_nHead) {
	m_nHead = m_nTail = m_nCurrent = -1;
	m_nId = 0;
	}
else {
	m_nTail = m_nCurrent;
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
	Current ()->Undo ();
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
	Current ()->Redo ();
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
			delete Head ()->m_item;
			} while (Head ()->Id () == nId);
		}
	}
}

//------------------------------------------------------------------------------

int CUndoManager::Backup (CGameItem* parent, eEditType editType) 
{ 
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

void CUndoManager::Begin (void) 
{
if (0 == m_nModified++) {
	SetModified (true);
	Update ();
	}
Lock ();
}

//------------------------------------------------------------------------------

void CUndoManager::Begin (undoData data) 
{
if (0 == m_nModified++) {
	SetModified (true);
	Update ();
	}
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
