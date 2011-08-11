using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    class ReactorTrigger : TriggerTargets, IGameItem 
    {
        // ------------------------------------------------------------------------

		public void Read (BinaryReader fp = null, bool bFlag = false)
        {
        }

        // ------------------------------------------------------------------------

		void Write (BinaryWriter fp = null, bool bFlag = false)
        {
        }

        // ------------------------------------------------------------------------

		void Clear () 
        { 
            base.Clear (); 
        }

        // ------------------------------------------------------------------------

    }
}
