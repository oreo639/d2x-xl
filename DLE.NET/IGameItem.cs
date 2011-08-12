using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace DLEdotNET
{
    public enum ItemType : uint
    {
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
        itDeltaLightIndex,
        itTextureColor,
        itFaceColor,
        itVertexColor
    }

    public enum EditType : uint
    {
        opNone, opAdd, opDelete, opModify, opMove
    }

    public interface IGameItem //: IComparable, ICloneable
    {
        int Key { get; set; }
        //uint Id { get; set; }
        //int Index { get; set; }
        //uint ItemType { get; set; }
        //uint EditType { get; set; }
        //IGameItem Parent { get; set; }
        //IGameItem Link { get; set; }
        //bool Used { get; }

        void Read (BinaryReader fp, int version = 0, bool bFlag = false);
        void Write (BinaryWriter fp, int version = 0, bool bFlag = false);
        void Clear ();
        //new IGameItem Clone ();
        //IGameItem Copy (IGameItem dest);

        //public void Undo ();
        //public void Redo ();
    }
}
