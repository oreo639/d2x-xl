
#include "mine.h"
#include "dle-xp.h"

CUndoManager undoManager;

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
for (int i = m_nCurrent; i != m_nTail; i = ++i % sizeof (m_buffer)) {
	delete m_buffer [i].m_item;
m_nTail = m_nCurrent;
}

//------------------------------------------------------------------------------

bool CUndoManager::Update (bool bForce)
{
	tUndoBuffer	*p;

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

void CUndoManager::Undo (CGameItem* itemP)
{
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
	m_nCurrent = Current ()->Prev ();
	} while ((m_nCurrent != m_nHead) && (Current ()->BackupId () == nId));
return true;
}

//------------------------------------------------------------------------------

void CUndoManager::Redo (CGameItem* itemP)
{
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
	m_nCurrent = Current ()->Next ();
	} while ((m_nCurrent != m_nTail) && (Current ()->BackupId () == nId));
if (m_nCurrent)
return true;
}

//------------------------------------------------------------------------------

bool CUndoManager::Revert (void)
{
if (!m_enabled || m_delay || !m_nHead)
	return false;
m_nCurrent = m_nTail;
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
CGameItem* itemP = parent->Clone ();
if (itemP != null) {
	Append ();
	SetModified (true);
	itemP->m_parent = parent; 
	itemP->m_nIndex = parent->m_nIndex;
	itemP->m_itemType = itemType;
	itemP->m_editType = editType;
	itemP->m_nBackup = Id ();
	Truncate ();
	m_nTail = m_nCurrent;
	itemP->Link (Tail ()); 
	}
return Id ();
}

//------------------------------------------------------------------------------

int CUndoManager::Count (void)
{
	tUndoBuffer	*p;
	int			i;

if (!m_enabled)
	return 0;
if (!(p = m_nHead))
	return 0;
for (i = 1; p != m_nCurrent; p = p->nextBuf)
	i++;
return i;
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
