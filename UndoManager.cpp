
#include "mine.h"
#include "dle-xp.h"

CUndoManager undoManager;

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

void CUndoItem::Setup (CGameItem* parent, eEditType editType, int nBackupId)
{
m_parent = parent; 
m_editType = editType;
m_nBackupId = nBackupId;
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
m_nTail = m_nCurrent;
}

//------------------------------------------------------------------------------

bool CUndoManager::Update (bool bForce)
{
if (!m_enabled || m_delay)
	return false;
m_nCurrent = m_nTail;
return true;
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
int nId = Current ()->BackupId ();
do {
	Current ()->Undo ();
	if (--m_nCurrent < 0)
		m_nCurrent = sizeof (m_buffer) - 1;
	} while ((m_nCurrent != m_nHead) && (Current ()->BackupId () == nId));
return true;
}

//------------------------------------------------------------------------------

bool CUndoManager::Redo (void)
{
if (!m_enabled)
	return false;
if (m_nCurrent == m_nTail)
	return false;
int nId = Current ()->BackupId ();
do {
	Current ()->Redo ();
	m_nCurrent = ++m_nCurrent % sizeof (m_buffer);
	} while ((m_nCurrent != m_nTail) && (Current ()->BackupId () == nId));
return true;
}

//------------------------------------------------------------------------------

bool CUndoManager::Revert (void)
{
if (!m_enabled || m_delay || !m_nHead)
	return false;
Truncate ();
return true;
}

//------------------------------------------------------------------------------

void CUndoManager::Delay (bool bDelay)
{
if (bDelay) {
	if (m_delay++ == 0)
		m_nId++;
	}
else if (m_delay)
	m_delay--;
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
		int nId = Head ()->BackupId ();
		do { // remove all items with same backup id from buffer start
			m_nHead = ++m_nHead % sizeof (m_buffer);
			delete Head ()->m_item;
			} while (Head ()->BackupId () == nId);
		}
	}
}

//------------------------------------------------------------------------------

int CUndoManager::Backup (CGameItem* parent, eItemType itemType, eEditType editType) 
{ 
CGameItem* itemP = parent->Clone (editType);
if (itemP != null) {
	Append ();
	SetModified (true);
	Tail ()->Setup (parent, editType, Id ());
	}
return Id ();
}

//------------------------------------------------------------------------------

int CUndoManager::Count (void)
{
return (m_nTail > m_nHead) ? (m_nTail - m_nHead + 1) : m_nTail + m_nHead - sizeof (m_buffer);
}

//------------------------------------------------------------------------------

bool CUndoManager::SetModified (bool bModified) 
{
DLE.GetDocument ()->SetModifiedFlag (bModified);
if (bModified) {
	m_nModified++;
	return Update ();
	}
m_nModified = 0;
return false;
}  

//------------------------------------------------------------------------------

void CUndoManager::ResetModified (bool bRevert) 
{
if (m_nModified) {
	if (!--m_nModified)
		SetModified (FALSE);
	Unlock ();
	if (bRevert)
		Revert ();
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
