using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using DLE.NET.GameMine;

namespace DLE.NET
{
    public interface IGameItem
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

        public int Key { get; set; }
        public uint Id { get; set; }
        public int Index { get; set; }
        public ItemType ItemType { get; set; }
        public EditType EditType { get; set; }
        public IGameItem Parent { get; set; }
        public IGameItem Link { get; set; }
        public bool Used { get; }

        public void Read (BinaryReader fp, int version = 0, bool bFlag = false);
        public void Write (BinaryWriter fp, int version = 0, bool bFlag = false);
        public void Clear ();
        public IGameItem Clone ();
        public IGameItem Copy (IGameItem dest);

        //public void Undo ();
        //public void Redo ();
    }
}
