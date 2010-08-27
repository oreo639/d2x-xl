using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLE.NET
{
    public class UVL
    {
        short u, v, l;

	int Read (Stream fp) {
		u = read_INT16 (fp);
		v = read_INT16 (fp);
		l = read_INT16 (fp);
		return 1;
		}

	void Write (Stream fp) {
		write_INT16 (u, fp);
		write_INT16 (v, fp);
		write_INT16 (l, fp);
		}

	public override void Clear () 
    { 
        u = v = l = 0; 
    }
};
    }
}
