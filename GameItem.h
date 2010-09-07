#ifndef __gameitem_h
#define __gameitem_h

#include "define.h"
#include "FileManager.h"
#include "crc.h"

# pragma pack(push, packing)
# pragma pack(1)

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

typedef enum {
	itUndefined,
	itVertex,
	itSegment,
	itMatCenter,
	itWall,
	itDoor,
	itTrigger,
	itObject,
	itRobot,
	itVariableLight,
	itDeltaLightValue,
	itDeltaLightIndex
	} eItemType;

typedef enum {
	opNone, opAdd, opDelete, opModify
} eEditType;

class CGameItem {
	protected:
		int			m_nIndex;
		int			m_nId;
		eItemType	m_itemType;
		eEditType	m_editType;
		CGameItem*	m_backup;

	public:
		CGameItem (eItemType itemType = itUndefined) : m_nIndex (-1), m_itemType (itemType), m_editType (opNone), m_backup (null) {}

		inline bool Used (void) { return m_nIndex >= 0; }

		inline int& Index (void) { return m_nIndex; }

		inline int& Id (void) { return m_nId; }

		virtual void Clear (void) {}

		virtual void Backup (eEditType editType = opModify) { return false; }

		virtual void Save (void) {}

		virtual void Undo (void) {}

		virtual void Redo (void) {}

		inline CGameItem* Backup (void) { return m_backup; }

		virtual CGameItem* Clone (eEditType editType) { return null; }

		bool HaveBackup (void);
	};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

#endif // __gameitem_h

