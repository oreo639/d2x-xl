
#include "mine.h"

// -----------------------------------------------------------------------------

bool CGameItem::HaveBackup (void)
{
return (m_backup != null) && (m_backup.Id () == undoManager.Id ());
}

// -----------------------------------------------------------------------------

#endif // __types_h

