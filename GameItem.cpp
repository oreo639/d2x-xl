
#include "mine.h"

// -----------------------------------------------------------------------------

bool CGameItem::HaveBackup (void)
{
return (m_parent != null) && (m_parent.Id () == undoManager.Id ());
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __types_h

