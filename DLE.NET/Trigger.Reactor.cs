using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLEdotNET
{
    public class ReactorTrigger : TriggerTargets, IGameItem 
    {
        public int Key { get; set; }

        // ------------------------------------------------------------------------

		public void Read (BinaryReader fp = null, int version = 0, bool bFlag = false)
        {
            Count = fp.ReadInt16 ();
            base.Read (fp);
        }

        // ------------------------------------------------------------------------

        public void Write (BinaryWriter fp = null, int version = 0, bool bFlag = false)
        {
            fp.Write (Count);
            base.Write (fp);
        }

        // ------------------------------------------------------------------------

        public new void Clear () 
        { 
            base.Clear (); 
        }

        // ------------------------------------------------------------------------

    }
}
