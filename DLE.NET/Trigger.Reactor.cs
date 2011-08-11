using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DLE.NET
{
    class ReactorTrigger : TriggerTarget, IGameItem {
    {
class CReactorTrigger : public CTriggerTargets, public CGameItem {
	public:

		void Read (CFileManager* fp = 0, bool bFlag = false);

		void Write (CFileManager* fp = 0, bool bFlag = false);

		virtual void Clear (void) { CTriggerTargets::Clear (); }

		virtual CGameItem* Clone (void);

		virtual void Backup (eEditType editType = opModify);

		virtual CGameItem* Copy (CGameItem* destP);
};

    }
}
