
#include "mine.h"
#include "dle-xp.h"

CUndoManager undoManager;

//------------------------------------------------------------------------------

CUndoManager::CUndoManager (int maxSize)
{
m_head = m_tail = m_current = null;
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
m_current = m_head;
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

void CUndoManager::Truncate ()
{
	tUndoBuffer	*p;

if (!m_current)
	return;
m_tail = m_current->prevBuf;
while (p = m_current) {
	m_current = m_current->nextBuf;
	delete p;
	m_size--;
	}
if (m_current = m_tail)
	m_tail->nextBuf = null;
else
	m_head = null;
}

//------------------------------------------------------------------------------

bool CUndoManager::Update (bool bForce)
{
	tUndoBuffer	*p;

if (!m_enabled || m_delay)
	return false;
if (!bForce && m_head &&
	 !memcmp (&m_current->undoBuffer, &theMine->Data (), sizeof (CMineData)))
	return true;
if (m_current != m_tail) {
	if (m_current)
		m_current = m_current->nextBuf;
	else
		m_current = m_head;
	Truncate ();
	}
if ((m_size < m_maxSize) && (p = new tUndoBuffer)) {
	m_size++;
	p->prevBuf = m_tail;
	if (m_tail)
		m_tail->nextBuf = p;
	else
		m_head = p;
	m_tail = p;
	}
else if (m_head) {
	if (m_head != m_tail) {
		m_head->prevBuf = m_tail;
		m_tail->nextBuf = m_head;
		m_tail = m_head;
		m_head = m_head->nextBuf;
		m_head->prevBuf = null;
		}
	}
else
	return false;
m_tail->nextBuf = null;
memcpy (&m_tail->undoBuffer, &theMine->Data (), sizeof (CMineData));
m_current = m_tail;
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

bool CUndoManager::Undo ()
{
if (!m_enabled)
	return false;
if (m_current == m_tail)
	Update ();
if (!m_current)
	return false;
if (m_current != m_head)
	m_current = m_current->prevBuf;
memcpy (&theMine->Data (), &m_current->undoBuffer, sizeof (CMineData));
return true;
}

//------------------------------------------------------------------------------

bool CUndoManager::Redo ()
{
if (!m_enabled)
	return false;
if (m_current == m_tail)
	return false;
if (m_current)
	m_current = m_current->nextBuf;
else
	m_current = m_head;
memcpy (&theMine->Data (), &m_current->undoBuffer, sizeof (CMineData));
if (m_current == m_tail)
	Truncate ();
return true;
}

//------------------------------------------------------------------------------

bool CUndoManager::Revert ()
{
if (!m_enabled || m_delay || !m_head)
	return false;
m_current = m_tail;
Truncate ();
return true;
}

//------------------------------------------------------------------------------

void CUndoManager::Delay (bool bDelay)
{
if (bDelay)
	m_delay++;
else if (m_delay)
	m_delay--;
}

//------------------------------------------------------------------------------

int CUndoManager::Backup (CGameItem* parent, eItemTyper itemType, eEditType editType) 
{ 
CGameItem* backup = parent->Clone ();
if (backupP != null) {
	backup->m_parent = parent;
	backup->m_itemType = itemType;
	backup->m_editType = editType;
	backup->m_nBackup = Id ();
	backup->Link (Tail ()); 
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
if (!(p = m_head))
	return 0;
for (i = 1; p != m_current; p = p->nextBuf)
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
